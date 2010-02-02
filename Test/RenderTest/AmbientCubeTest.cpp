#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Component/Render/EntityPrototypeLoader.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Component/Render/RenderModifierComponent.h"

#include <fstream>

using namespace MCD;

//--------------------------------------------------------------------------
struct AmbientCube
	: public LinkListBase::Node<AmbientCube>
{
	Vec3f pos;
	float color[18];

	void mix(const AmbientCube& mcube, float w)
	{
		float i_w = (w < 1) ? 1.0f - w : 0;

		for(int i=0; i<18; ++i)
			color[i] = (color[i] * i_w) + (mcube.color[i] * w);
	}

	void setColor(int index, const Vec3f& _color)
	{
		color[index*3+0] = _color.x;
		color[index*3+1] = _color.y;
		color[index*3+2] = _color.z;
	}
};	// AmbientCube

class AmbientCubeSet
{
public:
	LinkList<AmbientCube> mMCubes;

	bool isEmpty() const { return mMCubes.isEmpty(); }
	sal_maybenull AmbientCube* findClosest(const Vec3f&);
};	//AmbientCubeSet

AmbientCube* AmbientCubeSet::findClosest(const Vec3f& targetPos)
{
	if(mMCubes.isEmpty())
		return nullptr;

	// Currently just do a linear search, we will do a faster one later (if I have time :-P).
	AmbientCube* closestMCube = nullptr;
	float closestDist = 0.0f;

	for(AmbientCube* curr = mMCubes.begin(); curr != mMCubes.end(); curr = curr->next())
	{
		float dist = curr->pos.distance(targetPos);

		if(nullptr == closestMCube)
		{
			closestMCube = curr;
			closestDist = dist;
			continue;
		}

		if(dist < closestDist)
		{
			closestMCube = curr;
			closestDist = dist;
		}
	}

	MCD_ASSUME(closestMCube);

	return closestMCube;
}

//--------------------------------------------------------------------------
class AmbientCubeSetLoader
{
public:
	sal_maybenull AmbientCubeSet* load(const wchar_t* filePath)
	{
		std::ifstream fin;
		fin.open(filePath, std::ifstream::in);

		if(!fin.good())
		{
			Log::format( Log::Error, L"AmbientCubeSetLoader: File %s not found", filePath );
			return nullptr;
		}

		if(fin.eof())
		{
			Log::format( Log::Error, L"AmbientCubeSetLoader: File %s is empty", filePath );
			return nullptr;
		}

		std::auto_ptr<AmbientCubeSet> mcubes(new AmbientCubeSet);

		char line[256];
		int lineNo = 1;
		do
		{
			fin.getline(line, 256);

			if(!fin.good() && !fin.eof())
			{
				Log::format( Log::Error, L"AmbientCubeSetLoader: Failed to read line %d", lineNo );
				break;
			}

			if( ::strlen(line) > 0 )
			{
				std::auto_ptr<AmbientCube> mcube(new AmbientCube);

				int read = ::sscanf(line
					, "%f,%f,%f;%f,%f,%f;%f,%f,%f;%f,%f,%f;%f,%f,%f;%f,%f,%f;%f,%f,%f;"
					, &mcube->pos.x, &mcube->pos.y, &mcube->pos.z
					, &mcube->color[ 0], &mcube->color[ 1], &mcube->color[ 2]
					, &mcube->color[ 3], &mcube->color[ 4], &mcube->color[ 5]
					, &mcube->color[ 6], &mcube->color[ 7], &mcube->color[ 8]
					, &mcube->color[ 9], &mcube->color[10], &mcube->color[11]
					, &mcube->color[12], &mcube->color[13], &mcube->color[14]
					, &mcube->color[15], &mcube->color[16], &mcube->color[17]
					);

				if(21 != read)
				{
					// invalid format, just skip it
					Log::format( Log::Error, L"AmbientCubeSetLoader: Invalid format at line %d", lineNo );
					continue;
				}

				// success add to set
				mcubes->mMCubes.pushBack(*mcube.release());
			}

			++lineNo;

		} while(!fin.eof());

		return mcubes.release();
	}
};	// AmbientCubeSetLoader

//--------------------------------------------------------------------------
namespace AmbientCubeTest
{
	class Object
	{
	private:
		MeshComponent* mMeshComponent;
		AmbientCubeSet* mMCubes;
		AmbientCube mAccmMCube;

	public:
		Object(const wchar_t* name, Entity* root
			, const MeshPtr& mesh
			, const EffectPtr& effect
			, AmbientCubeSet* mcubes)
			: mMCubes(mcubes)
		{
			// create entity
			std::auto_ptr<Entity> e(new Entity);
			e->name = name;
			e->asChildOf(root);
			e->localTransform.setScale(Vec3f(0.5f, 0.5f, 0.5f));

			MeshComponent* c = new MeshComponent;
			c->mesh = mesh;
			c->effect = effect;
			e->addComponent(c);
			e->addComponent(new RenderCallback);

			mMeshComponent = c;

			// set the this to entity's user-data
			e->userData.setPtr<UserData>(new UserData(*this));

			e.release();

			// init ambient cube
			mAccmMCube.setColor(0, Vec3f(1,0,0));
			mAccmMCube.setColor(1, Vec3f(0,0,0));
			mAccmMCube.setColor(2, Vec3f(0,1,0));
			mAccmMCube.setColor(3, Vec3f(0,0,0));
			mAccmMCube.setColor(4, Vec3f(0,0,1));
			mAccmMCube.setColor(5, Vec3f(0,0,0));

            //mAccmMCube = (*mMCubes->findClosest(worldTransform().translation()));

		}

		void update(float deltaTime)
		{
			if(nullptr != mMCubes && !mMCubes->isEmpty())
			{
				if(AmbientCube* mcube = mMCubes->findClosest(worldTransform().translation()))
					mAccmMCube.mix(*mcube, deltaTime * 2.0f);
			}
		}

		Mat44f worldTransform() const
		{
			MCD_ASSUME(mMeshComponent);
			Entity* e = mMeshComponent->entity();
			MCD_ASSUME(e);
			return e->worldTransform();
		}

		Mat44f& localTransform()
		{
			MCD_ASSUME(mMeshComponent);
			Entity* e = mMeshComponent->entity();
			MCD_ASSUME(e);
			return e->localTransform;
		}

		struct UserData
		{
			Object& object;
			UserData(Object& _object) : object(_object) {}
		};

		class RenderCallback : public RenderModifierComponent
		{
		public:
			sal_override void preGeomRender()
			{
				Entity* e = entity();
				if(nullptr == e) return;

				UserData* userData = e->userData.getPtr<UserData>();
				if(nullptr == userData) return;

				ShaderProgram* shd = ShaderProgram::current();
				if(nullptr == shd) return;

				Mat44f worldTran = userData->object.worldTransform();
				shd->uniformMatrix4fv("g_WorldMatrix", 1, true, (float*)&worldTran);
				shd->uniform3fv("g_AmbientCube", 6, userData->object.mAccmMCube.color);
			}

			sal_override void postGeomRender()
			{
			}
		};	// ICallback
	};	// Object

	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=AmbientCubeTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem()),
			mAccumTime(0)
		{
			// Override the default loader of *.3ds file
			mResourceManager.addFactory(new EntityPrototypeLoaderFactory(mResourceManager));

			// load scene
			const wchar_t* scenePath = L"Scene/AmbientCubeScene/AmbientCubeScene.3DS";
			EntityPrototypeLoader::addEntityAfterLoad(&mRootNode, mResourceManager, scenePath);

			// load ambient cubes
			const wchar_t* mcubePath = L"Media/Scene/AmbientCubeScene/ambient_cube_scene.mcube";
			AmbientCubeSetLoader mcubeLoader;
			mAmbientCubes.reset( mcubeLoader.load(mcubePath) );

			// load object mesh
			MeshPtr mesh = new Mesh(L"");
			commitMesh(ChamferBoxBuilder(0.5f, 3, true), *mesh, MeshBuilder::Static);

			// load object effect
			EffectPtr effect = dynamic_cast<Effect*>(mResourceManager.load(L"Material/ambientcube.fx.xml").get());
			
			// create objects
			mObj1.reset(new Object(L"ChamferBox 1", &mRootNode, mesh, effect, mAmbientCubes.get()));
			mObj2.reset(new Object(L"ChamferBox 2", &mRootNode, mesh, effect, mAmbientCubes.get()));

			// disable Lighting
			glDisable(GL_LIGHTING);
		}

		sal_override void update(float deltaTime)
		{
			// update mAccumTime;
			mAccumTime += deltaTime;

			// remember to process loading events
			mResourceManager.processLoadingEvents();

			// update the transform
			const float ct = cosf(mAccumTime * 0.5f);
			const float st = sinf(mAccumTime * 0.5f);
			mObj1->localTransform().setTranslation(Vec3f(ct * 4, 2, -1));
			mObj1->update(deltaTime);

			mObj2->localTransform().setTranslation(Vec3f(st * 4, 2, 1));
			mObj2->update(deltaTime);

			// finally render
			render();
		}

		void render()
		{
			RenderableComponent::traverseEntities(&mRootNode);
		}

		DefaultResourceManager mResourceManager;
		
		float mAccumTime;
		Entity mRootNode;
		std::auto_ptr<AmbientCubeSet> mAmbientCubes;
		std::auto_ptr<Object> mObj1;
		std::auto_ptr<Object> mObj2;

	};	// TestWindow

}	// namespace AmbientCubeTest

TEST(AmbientCubeTest)
{	
	AmbientCubeTest::TestWindow window;
	window.mainLoop();
}
