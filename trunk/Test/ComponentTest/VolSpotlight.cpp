#include "Pch.h"
#include "../RenderTest/BasicGlWindow.h"
#include "../RenderTest/DefaultResourceManager.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Component/Render/MeshComponent.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../MCD/Core/Math/BasicFunction.h"
using namespace MCD;

TEST(VolSpotlight)
{
	class TestWindow : public BasicGlWindow
	{
		MeshPtr mMeshSpotlightCone;
		Entity* mCone;
		Effect* mRayTracer;
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=MeshComponentTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			{	// Build the light cone geometry, build a unit cone, and scale it
				mMeshSpotlightCone = new Mesh(L"SpotlightCone");
				MeshBuilder vertexBuilder, indexBuilder;

				// Build the vertex
				vertexBuilder.enable(Mesh::Position | Mesh::TextureCoord0);
				vertexBuilder.textureUnit(Mesh::TextureCoord0);
				vertexBuilder.textureCoordSize(2);

				const unsigned int Seg = 36 * 2;
				const unsigned int Ring = 2; // The current algo support ring = 2 only.
				const float DeltaT = 2.0f * Mathf::cPi() / Seg;
				const float InnerRadius = 0.0f;
				const float OuterRadius = 1;
				const float Height = 1;

				for(unsigned int ring = 0; ring < Ring; ++ring) // Since no lighting will be applied on it, we create two rings only, the top one and the bottom one.
				{
					float h = ring * Height;
					// The center of the top ring is put at the mesh center
					vertexBuilder.position(Vec3f(0, 0, h));
					vertexBuilder.textureCoord(Vec2f(h, 0));
					vertexBuilder.addVertex();

					float radius = (1 - h) * InnerRadius + h * OuterRadius;
					for(unsigned int seg = 0; seg < Seg; ++seg)
					{
						float t = seg * DeltaT;
						float x = cos(t);
						float y = sin(t);
						vertexBuilder.position(Vec3f(radius * x, radius * y, h));
						vertexBuilder.textureCoord(Vec2f(h, t));
						vertexBuilder.addVertex();
					}
				}

				vertexBuilder.commit(*mMeshSpotlightCone, MeshBuilder::Static);

				// Index
				indexBuilder.enable(Mesh::Index);
				unsigned int ringVertexCount = Seg + 1;

				for(unsigned int ring = 0; ring < Ring; ++ring)
				{
					unsigned int ringVertexOffset = ring * ringVertexCount;
					// build the ring first
					for(unsigned int seg = 0; seg < Seg; ++seg)
					{
						if (ring != Ring - 1)
							indexBuilder.addTriangle((uint16_t)(ringVertexOffset),
													 (uint16_t)(ringVertexOffset + 1 + (seg + 1) % Seg),
													 (uint16_t)(ringVertexOffset + 1 + (seg) % Seg));
						else
							indexBuilder.addTriangle((uint16_t)(ringVertexOffset),
													 (uint16_t)(ringVertexOffset + 1 + (seg) % Seg),
													 (uint16_t)(ringVertexOffset + 1 + (seg + 1) % Seg));
					}

					// build the side faces
					if (ring < Ring - 1)
					{
						for(unsigned int seg = 0; seg < Seg; ++seg)
							indexBuilder.addQuad((uint16_t)(ringVertexOffset + 1 + (seg) % Seg),
												 (uint16_t)(ringVertexOffset + 1 + (seg + 1) % Seg),
												 (uint16_t)(ringVertexCount + ringVertexOffset + 1 + (seg + 1) % Seg),
												 (uint16_t)(ringVertexCount + ringVertexOffset + 1 + (seg) % Seg));
					}
				}
				indexBuilder.commit(*mMeshSpotlightCone, Mesh::Index, MeshBuilder::Static);
			}

			{	// Add the light cone into the scene
				
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Cone 1";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(0, 0, 0));
				e->localTransform.setScale(Vec3f(10, 10, 20));

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mMeshSpotlightCone;
				mRayTracer = static_cast<Effect*>(mResourceManager.load(L"Material/VolSpotlight.fx.xml").get()); 
				c->effect = mRayTracer;
				e->addComponent(c);

				mCone = e.release();
			}

			mCamera.position = Vec3f(5.0f, 0.0f, 3.5f);
			mCamera.lookAt = Vec3f(-1, 0, 0);		
		}

		sal_override void update(float deltaTime)
		{
			GLfloat lightPos[] = { 200, 200, 200, 1.0f };
			glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
			mResourceManager.processLoadingEvents();

			Vec4f camLocalPos = mCone->worldTransform().inverse() * Vec4f(mCamera.position, 1);

			{	// Bind the camLocalPos to shader
				Material2* material = nullptr;
				if (mRayTracer && (material = mRayTracer->material.get()) != nullptr)
				{
					for(size_t i = 0; i < material->getPassCount(); ++i)
					{
						// Bind the per-instance uniform buffer to the shader
						material->preRender(i);
						Material2::Pass& pass = material->mRenderPasses[i];
						for(size_t propIdx = 0; propIdx < pass.mProperty.size(); ++propIdx)
						{
							IMaterialProperty& prop = pass.mProperty[propIdx];

							if (ShaderProperty* sp = dynamic_cast<ShaderProperty*>(&prop))
							{
								sp->shaderProgram->uniform3fv("osCamPos", 1, camLocalPos.getPtr());
							}
						}
						material->postRender(i);
					}
				}
			}
			
			glFrontFace(GL_CW);

			RenderableComponent::traverseEntities(&mRootNode);
		}

		Entity mRootNode;

		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.mainLoop();
	}
}
