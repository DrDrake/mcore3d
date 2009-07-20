#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/ShaderProgram.h"
#include "../../MCD/Render/PlaneMeshBuilder.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Physics/CollisionShape.h"
#include "../../MCD/Component/Physics/ThreadedDynamicWorld.h"
#include "../../MCD/Component/Physics/RigidBodyComponent.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Max3dsLoader.h"
#include "../../MCD/Render/ResourceLoaderFactory.h"
#include "../../MCD/Render/Mesh.h"

#include "../../3Party/glew/glew.h"

using namespace MCD;

//#define USE_HARDWARE_INSTANCE

// The Instanced Mesh composes of 2 classes:
// InstancedMesh and InstancedMeshComponent
// InstancedMeshComponent registers per-instance information to InstancedMesh
// InstancedMesh is responsible for uploading per-instance information to GPU, and issue the draw call
class InstancedMesh
{
public:
	InstancedMesh(const MeshPtr& mesh, IResourceManager& resourceManager) : mMesh(mesh), mUniformBufferHandle(0), mHwInstEffect(0)
	{
		CreateBindableUniformBuffer();
		mHwInstEffect = static_cast<Effect*>(resourceManager.load(L"Material/hwinst.fx.xml").get());
	}

	~InstancedMesh()
	{
		glDeleteBuffers(1, &mUniformBufferHandle);
	}

	void update(const Mat44f& viewMat)
	{
		{	// Write the per-instance data into the uniform buffer
			Mat44f worldMatBuf;

			glBindBuffer(GL_UNIFORM_BUFFER_EXT, mUniformBufferHandle);

			for(size_t i = 0; i < mPerInstanceInfo.size(); ++i)
			{
				worldMatBuf = (viewMat * mPerInstanceInfo[i]).transpose();

				glBufferSubData(GL_UNIFORM_BUFFER_EXT, i * sizeof(Mat44f), sizeof(Mat44f), worldMatBuf.getPtr());
			}

			glBindBuffer(GL_UNIFORM_BUFFER_EXT, 0);
		}

		{	// Bind the uniform buffer to shader, then draw the mesh
			Material2* material = nullptr;
			if (mHwInstEffect && (material = mHwInstEffect->material.get()) != nullptr) {
				for(size_t i=0; i<material->getPassCount(); ++i) {
					material->preRender(i);

					{	// Bind the per-instance uniform buffer to the shader
						Material2::Pass& pass = material->mRenderPasses[i];
						for(size_t propIdx = 0; propIdx < pass.mProperty.size(); ++propIdx)
						{
							IMaterialProperty& prop = pass.mProperty[propIdx];

							if (ShaderProperty* sp = dynamic_cast<ShaderProperty*>(&prop))
							{
								uint program = sp->shaderProgram->handle;
								uint location = glGetUniformLocation(program, "transformArray");
								glUniformBufferEXT(program, location, mUniformBufferHandle);
							}
						}
					}

					{	// Issue a single draw call to draw the balls
						mMesh->bind(Mesh::Index);
						glEnableClientState(GL_VERTEX_ARRAY);
						mMesh->bind(Mesh::Position);

						glDrawElementsInstancedEXT(GL_TRIANGLES, mMesh->indexCount(), GL_UNSIGNED_SHORT, 0, mPerInstanceInfo.size());

						glDisableClientState(GL_VERTEX_ARRAY);
					}

					material->postRender(i);
				}
			}
		}
		mPerInstanceInfo.clear();
	}

	// This function accept world transformation only, but this limitation is temporary..
	// It will be extended to accept more infomation(e.g. hw skinning info)
	// But before that, research on size of bindable uniforms and vertex texture should be done first
	void registerPerInstanceInfo(Mat44f info)
	{
		mPerInstanceInfo.push_back(info);
	}

	// Function required for intrusive pointer 
	friend void intrusivePtrAddRef(InstancedMesh* instMesh) {
		++(instMesh->mRefCount);
	}

	friend void intrusivePtrRelease(InstancedMesh* instMesh)
	{
		if(--(instMesh->mRefCount) == 0)
			delete instMesh;
	}
protected:
	AtomicInteger	mRefCount;

	MeshPtr			mMesh;
	GLuint			mUniformBufferHandle;
	EffectPtr		mHwInstEffect;

	typedef std::vector<Mat44f> PerInstanceInfo;
	PerInstanceInfo mPerInstanceInfo;
private:
	void CreateBindableUniformBuffer()
	{
		// Generate and allocate the uniform buffer
		const int MAX_BUFFER_SIZE = 65536; // It works on 8800GTS
		mUniformBufferHandle = 0;

		glGenBuffers(1, &mUniformBufferHandle);
		glBindBuffer(GL_UNIFORM_BUFFER_EXT, mUniformBufferHandle);
		// Allocate more than we need... Since the number of instances varies during runtime,
		// and the cost of allocating a single bindable uniform is in fact constant
		glBufferData(GL_UNIFORM_BUFFER_EXT, MAX_BUFFER_SIZE, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER_EXT, 0);
	}
};

typedef IntrusivePtr<InstancedMesh> InstancedMeshPtr;

// InstancedMeshComponent is a renderable
// When it is visited by Renderable Vistor, it does not render anything, but register the per-instance info to the InstancedMesh
// The InstancedMesh then later render the geometry
class InstancedMeshComponent : public RenderableComponent
{
	InstancedMeshPtr mInstMesh;
public:
	InstancedMeshComponent(InstancedMeshPtr instMesh) : mInstMesh(instMesh)
	{
	}

	sal_override void render()
	{
		renderFaceOnly();
	}

	sal_override void renderFaceOnly()
	{
		Entity* e = entity();
		MCD_ASSUME(e);
		mInstMesh->registerPerInstanceInfo(e->localTransform);
	}
};	// InstancedMeshComponent

TEST(ThreadedPhysicsComponentTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		void processResourceLoadingEvents()
		{
			while(true) {
				ResourceManager::Event e = mResourceManager.popEvent();
				if(e.loader) {
					if(e.loader->getLoadingState() == IResourceLoader::Aborted)
						std::wcout << L"Resource:" << e.resource->fileId().getString() << L" failed to load" << std::endl;
					// Allow one resource to commit for each frame
					e.loader->commit(*e.resource);
				} else
					break;
			}
		}

		TestWindow()
			:
			BasicGlWindow(L"title=ThreadedPhysicsComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			mResourceManager.addFactory(new Max3dsLoaderFactory(mResourceManager));

			// The maximum random displacement added to the balls
			static const float randomness = 0.0f;

			mDynamicsWorld.setGravity(Vec3f(0, -5, 0));

			Vec3f ballInitialPosition(0, 290, 0), ballPosXDelta(10, 0, 0), ballPosYDelta(0, 0, 10);

			// Setup the chamfer box mesh
			MeshPtr ballMesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
			chamferBoxBuilder.commit(*ballMesh, MeshBuilder::Static);

			mBallInstMesh = new InstancedMesh(ballMesh, mResourceManager);

			// Ball count
			int xCount = 32, yCount = 32;

			//  Setup a stack of balls
			for(int x = 0; x < xCount; ++x)
			{
				Vec3f ballPosition(ballInitialPosition + (float)(x - xCount / 2) * ballPosXDelta - (float)(yCount / 2) * ballPosYDelta);
				for(int y = 0; y < yCount; ++y)
				{	// Build entity
					std::auto_ptr<Entity> e(new Entity);
					e->name = L"ChamferBox 1";
					e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));
					// Add some randomness, hehehehe
					
					Vec3f randomOffset((Mathf::random() - 0.5f) * 2, (Mathf::random() - 0.5f) * 2, (Mathf::random() - 0.5f) * 2);
					e->localTransform.setTranslation(ballPosition + randomness * randomOffset);
					ballPosition += ballPosYDelta;

					// Add component
#ifdef USE_HARDWARE_INSTANCE
					InstancedMeshComponent* c = new InstancedMeshComponent(mBallInstMesh);
					e->addComponent(c);
#else
					MeshComponent* c = new MeshComponent;
					c->mesh = ballMesh;
					c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/simple.fx.xml").get());
					e->addComponent(c);
#endif

					// Create the phyiscs component
					RigidBodyComponent* rbc = new RigidBodyComponent(mDynamicsWorld, 0.5f, new SphereShape(1));
					e->addComponent(rbc);

					e->asChildOf(&mRootNode);
					e.release();
				}
			}

			{
				// Use a 3ds mesh as the ground
				mModel = dynamic_cast<Model*>(mResourceManager.load(L"Scene/City/scene.3ds", true).get());
				processResourceLoadingEvents();

				for(Model::MeshAndMaterial* it = mModel->mMeshes.begin(); it != mModel->mMeshes.end(); it = it->next())
				{
					MeshPtr mesh = it->mesh;

					// Setup the ground plane
					std::auto_ptr<Entity> e(new Entity);
					e->name = L"Ground";

					// Create the phyiscs component
					RigidBodyComponent* rbc = new RigidBodyComponent(mDynamicsWorld, 0, new StaticTriMeshShape(mesh));
					e->addComponent(rbc);

					e->asChildOf(&mRootNode);
					e.release();
				}
			}

			// Override camera position to see the huge lattice of balls, hahahaha
			mCamera.position = Vec3f(0, 300, 0);
			mCamera.lookAt = Vec3f(0, -1, 0);
			mCamera.upVector = Vec3f(0, 0, 1);

			// Start the physics thread
			mPhysicsThread.start(mDynamicsWorld, false);
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			mModel->draw();

			RenderableComponent::traverseEntities(&mRootNode);

#ifdef USE_HARDWARE_INSTANCE
			Mat44f viewMat;
			mCamera.computeView(viewMat.getPtr());
			mBallInstMesh->update(viewMat);
#endif

			BehaviourComponent::traverseEntities(&mRootNode);
		}

		virtual ~TestWindow()
		{
			mPhysicsThread.postQuit();
			mPhysicsThread.wait();

			// Make sure the RigidBodyComponent is freed BEFORE the dynamics world...
			while(mRootNode.firstChild())
				delete mRootNode.firstChild();
		}

		Entity mRootNode;
		ModelPtr mModel;
		InstancedMeshPtr mBallInstMesh;
		ThreadedDynamicsWorld mDynamicsWorld;
		DefaultResourceManager mResourceManager;
		Thread mPhysicsThread;
	};	// TestWindow

	{
		TestWindow window;
		window.mainLoop();
	}
}
