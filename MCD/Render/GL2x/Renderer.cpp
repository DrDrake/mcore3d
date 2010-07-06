#include "Pch.h"
#include "Renderer.inc"
#include "../Camera.h"
#include "../Light.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../QuadComponent.h"
#include "../RenderTargetComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/wglew.h"

namespace MCD {

RendererComponent::Impl::Impl()
	: mEntityItr(nullptr)
{
	mQuadRenderer.reset(new QuadRenderer(*this));
}

void RendererComponent::Impl::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);			// OpenGl use counterclockwise as the default winding
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_LIGHTING);

	{	// Apply camera
		CameraComponent2Ptr camera = renderTarget.cameraComponent;
		if(!camera) camera = mDefaultCamera.get();
		Entity* cameraEntity = camera->entity();
		if(!cameraEntity) return;

		camera->frustum.computeProjection(mProjMatrix.getPtr());
		mCameraTransform = cameraEntity->worldTransform();
		mViewMatrix = mCameraTransform.inverse();
		mViewProjMatrix = mProjMatrix * mViewMatrix;

		glMatrixMode(GL_PROJECTION);
		glLoadTransposeMatrixf(mProjMatrix.getPtr());
		glMatrixMode(GL_MODELVIEW);
		glLoadTransposeMatrixf(mViewMatrix.getPtr());
	}

	// Traverse the Entity tree
	for(mEntityItr = EntityPreorderIterator(&entityTree); !mEntityItr.ended();)
	{
		// The input parameter entityTree will never skip
		if(!mEntityItr->enabled && mEntityItr.current() != &entityTree) {
			mEntityItr.skipChildren();
			continue;
		}

		Entity* e = mEntityItr.current();

		// Pop material when moving up (towards parent) or leveling in the tree
		mCurrentMaterial = nullptr;
		for(int depth = mEntityItr.depthChange(); depth <= 0 && mMaterialStack.size() > 0; ++depth)
			mMaterialStack.pop();

		// Preform actions defined by the concret type of RenderableComponent2 we have found
		if(RenderableComponent2* renderable = e->findComponent<RenderableComponent2>())
			renderable->render2(this);

		// Skip if there where no material
		if(nullptr == mCurrentMaterial) {
			if(mMaterialStack.empty()) {
				mEntityItr.next();
				continue;
			}
			mCurrentMaterial = mMaterialStack.top();
		}
		mMaterialStack.push(mCurrentMaterial);

		mEntityItr.next();
	}	// traverse entities

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

			const Vec3f pos = light->entity()->worldTransform().translation();
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
		processRenderItems(mOpaqueQueue);
	}

	{	// Render transparent items
		glEnable(GL_BLEND);
//		glDisable(GL_CULL_FACE);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glDepthMask(GL_FALSE);

		processRenderItems(mTransparentQueue);

		{	// Render QuadComponent
			MCD_FOREACH(const QuadMaterialPair& pair, mQuads) {
				QuadComponent* quad = pair.quad;
				const Mat44f& transform = quad->entity()->worldTransform();
				mQuadRenderer->push(transform, quad->width, quad->height, quad->uv, pair.mtl);
			}
			mQuadRenderer->flush();
			mQuads.clear();
		}

		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
	}

	mLights.clear();
	mOpaqueQueue.destroyAll();
	mTransparentQueue.destroyAll();
}

void RendererComponent::Impl::render(Entity& entityTree)
{
	// TODO: Update only RenderTargetComponent, to bring them into mRenderTargets
	RenderableComponent2::traverseEntities(&entityTree);

	// Process the render targets one by one
	for(size_t i=0; i<mRenderTargets.size(); ++i) {
		if(RenderTargetComponent* r1 = mRenderTargets[i].get()) {
			const RenderWindow* w1 = r1 ? r1->window : nullptr;
			const RenderTargetComponent* r2 = (i+1 == mRenderTargets.size()) ? nullptr : mRenderTargets[i+1].get();
			const RenderWindow* w2 = r2 ? r2->window : nullptr;
			const bool swapBuffers = (w1 != w2);
			r1->render(*mBackRef, swapBuffers);
		}
	}
	mRenderTargets.clear();
}

void RendererComponent::Impl::processRenderItems(RenderItems& items)
{
	for(RenderItemNode* node = items.findMin(); node != nullptr; node = node->next()) {
		const RenderItem& i = node->mRenderItem;

		if(Entity* e = i.mesh->entity()) {
			mWorldMatrix = e->worldTransform();
			mWorldViewProjMatrix = mViewProjMatrix * mWorldMatrix;

			i.material->preRender(0, this);
			glPushMatrix();
			glMultTransposeMatrixf(e->worldTransform().getPtr());
			i.mesh->mesh->draw();
			glPopMatrix();
			i.material->postRender(0, this);
		}
	}
}

void RendererComponent::Impl::preRenderMaterial(size_t pass, IMaterialComponent& mtl) {
	mtl.preRender(pass, this);
}

void RendererComponent::Impl::postRenderMaterial(size_t pass, IMaterialComponent& mtl) {
	mtl.postRender(pass, this);
}

void QuadRenderer::push(const Mat44f& transform, float width, float height, const Vec4f& uv, IMaterialComponent* mtl)
{
	mRenderer.preRenderMaterial(0, *mtl);
	glPushMatrix();
	glMultTransposeMatrixf(transform.getPtr());

	const float halfW = 0.5f * width;
	const float halfH = 0.5f * height;
	glBegin(GL_QUADS);
		glTexCoord2f(uv.x, uv.y);
		glVertex3f(-halfW, halfH, 0);
		glTexCoord2f(uv.x, uv.w);
		glVertex3f(-halfW, -halfH, 0);
		glTexCoord2f(uv.z, uv.w);
		glVertex3f(halfW, -halfH, 0);
		glTexCoord2f(uv.z, uv.y);
		glVertex3f(halfW, halfH, 0);
	glEnd();

	glPopMatrix();
	mRenderer.postRenderMaterial(0, *mtl);
}

void QuadRenderer::flush()
{
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

}	// namespace MCD
