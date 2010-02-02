#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Mesh.h"
//#include "../../MCD/Render/EditableMesh.h"
#include "../../MCD/Render/RayMeshIntersect.h"

#include "../../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../../MCD/Component/Render/MeshComponent.h"

#include <fstream>

using namespace MCD;

/*
namespace RayMeshIntersectTest {

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=RayMeshIntersectTest;width=800;height=600;fullscreen=0;FSAA=4"),
		mResourceManager(*createDefaultFileSystem()),
		mAccumTime(0)
	{
		// Override the default loader of *.3ds file
		mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

		// Load scene
		const wchar_t* scenePath = L"Scene/City/scene.3ds";
		EntityPrototypeLoader::addEntityAfterLoad(&mRootNode, mResourceManager, scenePath, 0, 0, L"loadAsEntity=true;editable=true");

		// Disable Lighting
		glDisable(GL_LIGHTING);
	}

	sal_override void update(float deltaTime)
	{
		// Update mAccumTime;
		mAccumTime += deltaTime;

		// Remember to process loading events
		mResourceManager.processLoadingEvents();

		// Finally render
		RenderableComponent::traverseEntities(&mRootNode);

		intersectionTest();
	}

	void intersectionTest()
	{
		// Update the transform
		const float ct = cosf(mAccumTime * 0.5f);
		const float st = sinf(mAccumTime * 0.5f);
		const float r = 12.0f;

		const Vec3f rayOrig( 0, 10, 0 );
		const Vec3f rayTarget( ct * r, -10, st * r );

		SimpleRayMeshIntersect i;

		struct AutoAdvance
		{
			EntityPreorderIterator& itr;
			explicit AutoAdvance(EntityPreorderIterator& _itr) : itr(_itr) {}
			~AutoAdvance() { itr.next(); }
		};

		// Add meshes to intersect object and invoke EditableMesh::beginEditing()
		for(EntityPreorderIterator itr(&mRootNode); !itr.ended();)
		{
			if(!itr->enabled)
			{
				itr.skipChildren();
				continue;
			}

			AutoAdvance autoadv(itr);

			MeshComponent* meshComp = dynamic_cast<MeshComponent*>(itr->findComponent<RenderableComponent>());
			if(!meshComp) continue;

			EditableMesh* mesh = dynamic_cast<EditableMesh*>(meshComp->mesh.get());
			if(!mesh) continue;

			mesh->beginEditing();
			i.addMesh(mesh, itr->worldTransform());
		}

		i.build();

		// Perform testing
		i.begin();
		for(size_t j=0; j<5; ++j) for(size_t k=0; k<5; ++k)
			i.test(rayOrig, (rayTarget - rayOrig + Vec3f(float(j), 0, float(k))).normalizedCopy(), false);
		i.end();

		drawHitResults(i.results());

		// Invoke EditableMesh::endEditing()
		for(EntityPreorderIterator itr(&mRootNode); !itr.ended();)
		{
			if(!itr->enabled)
			{
				itr.skipChildren();
				continue;
			}

			AutoAdvance autoadv(itr);

			MeshComponent* meshComp = dynamic_cast<MeshComponent*>(itr->findComponent<RenderableComponent>());
			if(!meshComp) continue;

			EditableMesh* mesh = dynamic_cast<EditableMesh*>(meshComp->mesh.get());
			if(!mesh) continue;

			mesh->endEditing(false);
		}
	}

	void drawHitResults(LinkList<IRayMeshIntersect::HitResult>& results)
	{
		glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
		
		glDisable(GL_LIGHTING);
		glBegin(GL_LINES);

		// Draw the results
		for(IRayMeshIntersect::HitResult* r = results.begin();
			r != results.end();
			r = r->next() )
		{
			Vec3f rayOrig = r->rayOrig;
			Vec3f rayTarget = r->rayOrig + r->rayDir * 20.0f;

			if(r->hits.isEmpty())
			{
				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex3fv(rayOrig.data);
				glVertex3fv(rayTarget.data);
			}
			else
			{
				glColor3f(1.0f, 0.0f, 0.0f);
				Vec3f hitPos = IRayMeshIntersect::Helper::getHitPosition(r->closest);
				glVertex3fv(rayOrig.data);
				glVertex3fv(hitPos.data);

				glColor3f(0.0f, 1.0f, 0.0f);
				glVertex3fv(rayTarget.data);
				glVertex3fv(hitPos.data);
			}
		}

		// Reset the color to white
		glColor3f(1.0f, 1.0f, 1.0f);

		glEnd();
		glPopAttrib();
	}

	DefaultResourceManager mResourceManager;
	
	float mAccumTime;
	Entity mRootNode;
};	// TestWindow

}	// namespace RayMeshIntersectTest

TEST(RayMeshIntersectTest)
{	
	RayMeshIntersectTest::TestWindow window;
	window.mainLoop();
}
*/