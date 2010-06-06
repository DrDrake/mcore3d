#include "Pch.h"
#include "../Renderer.h"
#include "../Camera.h"
#include "../Light.h"
#include "../Material.h"
#include "../Mesh.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/System/Deque.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/wglew.h"

namespace MCD {

struct RenderItem
{
	sal_notnull MeshComponent2* mesh;
	sal_notnull MaterialComponent* material;
};	// RenderItem

typedef std::vector<RenderItem> RenderItems;

class RendererComponent::Impl
{
public:
	void render(Entity& entityTree, CameraComponent2* camera);

	//! mDefaultCamera and mCurrentCamera may differ from each other when rendering using light's view
	CameraComponent2Ptr mDefaultCamera, mCurrentCamera;
	Mat44f mCameraTransform;
	Mat44f mProjMatrix, mViewMatrix, mViewProjMatrix, mWorldMatrix, mWorldViewProjMatrix;
	sal_notnull RendererComponent* mBackRef;
	std::stack<MaterialComponent*> mMaterialStack;

	typedef std::vector<LightComponent*> Lights;
	Lights mLights;

	RenderItems mRenderQueue;
};	// Impl

void RendererComponent::Impl::render(Entity& entityTree, CameraComponent2* camera)
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
		if(!camera) camera = mDefaultCamera.get();
		mCurrentCamera = camera;
		if(!camera) return;
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
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Traverse the Entity tree
	for(EntityPreorderIterator itr(&entityTree); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		Entity* e = itr.current();

		MaterialComponent* mtl = e->findComponent<MaterialComponent>();

		// Pop material when moving up (towards parent) or leveling in the tree
		for(int depth = itr.depthChange(); depth <= 0 && mMaterialStack.size() > 0; ++depth)
			mMaterialStack.pop();

		// Push material
		if(nullptr == mtl)
			mtl = mMaterialStack.top();
		mMaterialStack.push(mtl);

		// Push light into the light list, if any
		if(LightComponent* light = e->findComponent<LightComponent>()) {
			mLights.push_back(light);
		}

		// Push mesh into render queue, if any
		if(MeshComponent2* mesh = e->findComponent<MeshComponent2>()) {
			RenderItem r = { mesh, mtl };
			mRenderQueue.push_back(r);
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

	// Render the items in render queue
	MCD_FOREACH(const RenderItem& i, mRenderQueue) {
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

	mLights.clear();
	mRenderQueue.clear();
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

void RendererComponent::setDefaultCamera(CameraComponent2& camera) {
	mImpl.mDefaultCamera = &camera;
}

CameraComponent2* RendererComponent::defaultCamera() const {
	return mImpl.mDefaultCamera.get();
}

void RendererComponent::render(Entity& entityTree, CameraComponent2* camera) {
	mImpl.render(entityTree, camera);
}

}	// namespace MCD