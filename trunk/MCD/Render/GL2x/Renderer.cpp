#include "Pch.h"
#include "Renderer.inc"
#include "../Camera.h"
#include "../Light.h"
#include "../Mesh.h"
#include "../RenderTarget.h"
#include "../RenderWindow.h"
#include "../../../3Party/glew/wglew.h"
#include <set>

namespace MCD {

RendererComponent::Impl::Impl()
{
	resetStatistic();
}

void RendererComponent::Impl::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);			// OpenGl use counterclockwise as the default winding
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Tell OpenGl use origin of the eye coordinate rather than simply z axis to calculate the specular angle
	// Ref: http://gregs-blog.com/2007/12/21/theres-nothing-wrong-with-opengls-specular-lighting/
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);

	// Seems make better specular lighting
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

	mCurrentRenderTarget = &renderTarget;

	{	// Apply camera
		CameraComponentPtr camera = renderTarget.cameraComponent;
		if(!camera) return;
		Entity* cameraEntity = camera->entity();
		if(!cameraEntity) return;

		mCurrentCamera = camera.getNotNull();
		camera->frustum.computeProjection(mProjMatrix.data);
		mCameraTransform = cameraEntity->worldTransform();
		mViewMatrix = mCameraTransform.inverse();
		mViewProjMatrix = mProjMatrix * mViewMatrix;

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(mProjMatrix.data);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(mViewMatrix.data);
	}

	// Traverse the Entity tree
	traverseEntities(entityTree);

	// Set up lighting
	static const size_t cMaxHardwareLight = 8;
	for(size_t i=0; i<cMaxHardwareLight; ++i)
	{
		const int iLight = GL_LIGHT0 + i;

		if(i < mLights.size()) {
			const LightComponent* light = mLights[i];
			MCD_ASSUME(light);

			glEnable(iLight);
			const ColorRGBAf ambient(0, 1);
			const ColorRGBAf diffuse(light->color, 1);

			glLightfv(iLight, GL_AMBIENT, ambient.rawPointer());
			glLightfv(iLight, GL_DIFFUSE, diffuse.rawPointer());
			glLightfv(iLight, GL_SPECULAR, diffuse.rawPointer());

			Entity* e = light->entity();
			MCD_ASSUME(e);
			const Vec3f pos = e->worldTransform().translation();
			GLfloat lightPos[] = { pos.x, pos.y, pos.z, 1.0f };
			glLightfv(iLight, GL_POSITION, lightPos);
		}
		else
			glDisable(iLight);
	}

	{	// Disable lighting if there is no LightComponent
		if(mLights.empty())
			glDisable(GL_LIGHTING);
		else
			glEnable(GL_LIGHTING);
	}

	{	// Render opaque items
		processRenderItems(mOpaqueQueue, mStatistic.opaque, mStatistic.materialSwitch);
	}

	{	// Render transparent items
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glDepthMask(GL_FALSE);

		processRenderItems(mTransparentQueue, mStatistic.transparent, mStatistic.materialSwitch);

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}

	// Reset the last material
	if(mLastMaterial) {
		mLastMaterial->postRender(0, this);
		mLastMaterial = nullptr;
	}

	mLights.clear();
	mCurrentCamera = nullptr;
	mOpaqueQueue.destroyAll();
	mTransparentQueue.destroyAll();
}

void RendererComponent::Impl::render(Entity& entityTree)
{
	// Only perform postUpdate() (swap buffers) for each unique window
	typedef std::set<RenderWindow*> UniqueWindows;
	UniqueWindows uniqueWindows;

	// Process the render targets one by one
	for(size_t i=0; i<mRenderTargets.size(); ++i) {
		if(RenderTargetComponent* r = mRenderTargets[i].get()) {
			r->render(*mBackRef);
			uniqueWindows.insert(r->window);
		}
	}
	mRenderTargets.clear();

	for(UniqueWindows::const_iterator i=uniqueWindows.begin(); i!=uniqueWindows.end(); ++i)
		if(RenderWindow* w = *i) w->postUpdate();
}

void RendererComponent::Impl::processRenderItems(RenderItems& items, IDrawCall::Statistic& statistic, size_t& materialSwitch)
{
	for(RenderItemNode* node = items.findMin(); node != nullptr; node = node->next())
	{
		const RenderItem& i = node->mRenderItem;

		if(Entity* e = i.entity) {
			mWorldMatrix = i.worldTransform;
			mWorldViewProjMatrix = mViewProjMatrix * mWorldMatrix;

			glPushMatrix();
			glMultMatrixf(e->worldTransform().data);

			// Deal with 2d axis mode
			if(mCurrentCamera->frustum.projectionType == Frustum::YDown2D)
				glScalef(1, -1, 1);

			IMaterialComponent* mtl = i.material;

			if(mtl != mLastMaterial)
				++materialSwitch;

			// The material class will preform early out if mtl == mLastMaterial
			// NOTE: Must call for every RenderItem because the material is also
			// responsible for setting up the various matrix shader constants.
			if(mtl) {
				mtl->preRender(0, this);
				i.drawCall->draw(this, statistic);
				mtl->postRender(0, this);
			}
			// RenderItems' material can be null, meaning the Renderable will handle
			// the material for itself, for example SpriteComponent
			else {
				i.drawCall->draw(this, statistic);
			}

			mLastMaterial = mtl;

			glPopMatrix();
		}
	}
}

RendererComponent::RendererComponent()
	: mImpl(*new Impl)
{
	mImpl.mBackRef = this;
}

RendererComponent::~RendererComponent()
{
	delete &mImpl;
}

void RendererComponent::render(Entity& entityTree) {
	mImpl.render(entityTree);
}

void RendererComponent::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	mImpl.render(entityTree, renderTarget);
}

static RendererComponent* gCurrentRendererComponent = nullptr;

RendererComponent& RendererComponent::current()
{
	MCD_ASSUME(gCurrentRendererComponent);
	return *gCurrentRendererComponent;
}

void RendererComponent::begin()
{
	gCurrentRendererComponent = this;
	mImpl.resetStatistic();
}

void RendererComponent::end(float dt)
{
	gCurrentRendererComponent = nullptr;
}

}	// namespace MCD
