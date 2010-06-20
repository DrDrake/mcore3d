#include "Pch.h"
#include "Renderer.inc"
#include "../Camera.h"
#include "../Light.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../RenderTargetComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/wglew.h"

namespace MCD {

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
	for(EntityPreorderIterator itr(&entityTree); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		Entity* e = itr.current();

		// Push light into the light list, if any
		if(LightComponent* light = e->findComponent<LightComponent>()) {
			mLights.push_back(light);
		}

		// Pop material when moving up (towards parent) or leveling in the tree
		for(int depth = itr.depthChange(); depth <= 0 && mMaterialStack.size() > 0; ++depth)
			mMaterialStack.pop();

		// Push material
		MaterialComponent* mtl = e->findComponent<MaterialComponent>();
		if(nullptr == mtl) {
			// Skip if there where no material
			if(mMaterialStack.empty()) {
				itr.next();
				continue;
			}
			mtl = mMaterialStack.top();
		}
		mMaterialStack.push(mtl);

		// Push mesh into render queue, if any
		if(MeshComponent2* mesh = e->findComponent<MeshComponent2>()) {
			Vec3f pos = e->worldTransform().translation();
			mViewMatrix.transformPoint(pos);
			const float dist = pos.z;

			RenderItem r = { mesh, mtl };
			if(!mtl->isTransparent())
				mOpaqueQueue.insert(*new RenderItemNode(-dist, r));
			else
				mTransparentQueue.insert(*new RenderItemNode(dist, r));
		}

		itr.next();
	}

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

	{	// Render opaque items
		processRenderItems(mOpaqueQueue);
	}

	{	// Render transparent items
		glEnable(GL_BLEND);
		glColor4f(0, 0, 0, 0.5f);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//		glDisable(GL_CULL_FACE);
//		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//		glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
		glDepthMask(GL_FALSE);
		processRenderItems(mTransparentQueue);
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
	}

	mLights.clear();
	mOpaqueQueue.destroyAll();
	mTransparentQueue.destroyAll();
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
	// TODO: Update only RenderTargetComponent, to bring them into mImpl.mRenderTargets
	RenderableComponent2::traverseEntities(&entityTree);

	// Process the render targets one by one
	MCD_FOREACH(RenderTargetComponentPtr renderTarget, mImpl.mRenderTargets) {
		if(renderTarget) {
//			RenderableComponent2::traverseEntities(renderTarget->entityToRender.get());
			renderTarget->render(*this);
		}
	}
	mImpl.mRenderTargets.clear();
}

void RendererComponent::render(Entity& entityTree, RenderTargetComponent& renderTarget)
{
	mImpl.render(entityTree, renderTarget);
}

}	// namespace MCD