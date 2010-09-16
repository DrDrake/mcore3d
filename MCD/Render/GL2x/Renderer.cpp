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
	: mLastMaterial(nullptr), mEntityItr(nullptr), mCurrentMaterial(nullptr)
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

	// Tell OpenGl use origin of the eye coordinate rather than simply z axis to calculate the specular angle
	// Ref: http://gregs-blog.com/2007/12/21/theres-nothing-wrong-with-opengls-specular-lighting/
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0f);

	// Seems make better specular lighting
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

	{	// Apply camera
		CameraComponentPtr camera = renderTarget.cameraComponent;
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

	mCurrentMaterial = nullptr;

	// Traverse the Entity tree
	for(mEntityItr = EntityPreorderIterator(&entityTree); !mEntityItr.ended();)
	{
		// The input parameter entityTree will never skip
		if(!mEntityItr->enabled && mEntityItr.current() != &entityTree) {
			mEntityItr.skipChildren();
			// TODO: May need mMaterialStack.push(mMaterialStack.top()) ?
			continue;
		}

		Entity* e = mEntityItr.current();

		// Pop material when moving up (towards parent) or leveling in the tree
		for(int depth = mEntityItr.depthChange(); depth <= 0 && mMaterialStack.size() > 0; ++depth)
			mMaterialStack.pop();
		mCurrentMaterial = !mMaterialStack.empty() ? mMaterialStack.top() : nullptr;

		// Preform actions defined by the concret type of RenderableComponent we have found
		if(RenderableComponent* renderable = e->findComponent<RenderableComponent>())
			renderable->render2(this);

		if(!mCurrentMaterial) {
			// Skip if there where no material
			if(mMaterialStack.empty()) {
				mEntityItr.next();
				continue;
			}
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
				Entity* e = quad->entity();
				MCD_ASSUME(e);
				const Mat44f& transform = e->worldTransform();
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
	RenderableComponent::traverseEntities(&entityTree);

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

		if(Entity* e = i.entity) {
			mWorldMatrix = i.worldTransform;
			mWorldViewProjMatrix = mViewProjMatrix * mWorldMatrix;

			IMaterialComponent* mtl = i.material;
			// Only single pass material support the last material optimization
			if(mLastMaterial && (mtl != mLastMaterial/* || passCount > 1*/)) {
				mLastMaterial->postRender(0, this);
				mLastMaterial = nullptr;
			}

			if(mtl != mLastMaterial)
				mtl->preRender(0, this);

			glPushMatrix();
			glMultTransposeMatrixf(e->worldTransform().getPtr());
			i.drawCall->draw(this);
			glPopMatrix();

			//if(passCount == 1)
				mLastMaterial = mtl;
		}
	}

	{	// Reset the last material
		if(mLastMaterial) {
			mLastMaterial->postRender(0, this);
			mLastMaterial = nullptr;
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
	// Trigger a flush whenever the material is changed or too many quad in the buffer
	if(mtl != mCurrentMaterial || mQuadCount > 512)
		flush();
	mCurrentMaterial = mtl;

	const float halfW = 0.5f * width;
	const float halfH = 0.5f * height;

	++mQuadCount;
	Vertex v[4] = {
		{ Vec3f(-halfW,  halfH, 0), Vec2f(uv.x, uv.y) },
		{ Vec3f(-halfW, -halfH, 0), Vec2f(uv.x, uv.w) },
		{ Vec3f( halfW, -halfH, 0), Vec2f(uv.z, uv.w) },
		{ Vec3f( halfW,  halfH, 0), Vec2f(uv.z, uv.y) },
	};

	for(size_t i=0; i<MCD_COUNTOF(v); ++i)
		transform.transformPoint(v[i].position);

	mVertexBuffer.push_back(v[0]);
	mVertexBuffer.push_back(v[1]);
	mVertexBuffer.push_back(v[2]);
	mVertexBuffer.push_back(v[0]);
	mVertexBuffer.push_back(v[2]);
	mVertexBuffer.push_back(v[3]);
}

void QuadRenderer::flush()
{
	MCD_ASSERT(mVertexBuffer.size() == mQuadCount * 6);
	if(!mQuadCount)
		return;

	mRenderer.preRenderMaterial(0, *mCurrentMaterial);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].uv);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), &mVertexBuffer[0].position);
	glDrawArrays(GL_TRIANGLES, 0, mVertexBuffer.size());

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	mVertexBuffer.clear();
	mQuadCount = 0;

	mRenderer.postRenderMaterial(0, *mCurrentMaterial);
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
