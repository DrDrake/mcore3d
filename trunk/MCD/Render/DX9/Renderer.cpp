#include "Pch.h"
#include "../Renderer.h"
#include "../Camera.h"
#include "../../Core/Math/Mat44.h"
#include "../../Core/Entity/Entity.h"

namespace MCD {

class RendererComponent::Impl
{
public:
	void render(Entity& entityTree, CameraComponent2* camera);

	//! mDefaultCamera and mCurrentCamera may differ from each other when rendering using light's view
	CameraComponent2Ptr mDefaultCamera, mCurrentCamera;
	Mat44f mProjectionMatrix, mViewMatrix;
	sal_notnull RendererComponent* mBackRef;
};	// Impl

void RendererComponent::Impl::render(Entity& entityTree, CameraComponent2* camera)
{
	{	// Apply camera
		if(!camera) camera = mDefaultCamera.get();
		mCurrentCamera = camera;
		if(!camera || !camera->entity())
			return;

		camera->frustum.computeProjection(mProjectionMatrix.getPtr());
		mViewMatrix = camera->entity()->worldTransform().inverse();

/*		glMatrixMode(GL_PROJECTION);
		glLoadTransposeMatrixf(mProjectionMatrix.getPtr());
		glMatrixMode(GL_MODELVIEW);
		glMultTransposeMatrixf(mViewMatrix.getPtr());
		glClearColor(0, 0, 0, 1);*/
	}

	// Traverse the Entity tree
	for(EntityPreorderIterator itr(&entityTree); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

//		Entity* e = itr.current();

		// Render mesh if any
//		MeshComponent* mesh = e->findComponent<MeshComponent>();

		itr.next();
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
