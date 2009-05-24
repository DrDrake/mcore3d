#include "Pch.h"
#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/Components/MeshComponent.h"

using namespace MCD;

namespace MCD
{

class TangentSpaceBuilder
{
public:
	void compute( MeshBuilder& builder, Mesh::DataType uvDataType, Mesh::DataType tangentDataType );

protected:
	static void computeTangentBasis(
		const Vec3f& P1, const Vec3f& P2, const Vec3f& P3, 
		const Vec2f& UV1, const Vec2f& UV2, const Vec2f& UV3,
		Vec3f &tangent, Vec3f &bitangent
		);

	/*! A small utility which acquire the mesh buffer pointer in its constructor;
		and release the pointer in its destructor.
	*/
	class MeshBufferPointer
	{
	public:
		MeshBufferPointer(MeshBuilder& builder, Mesh::DataType dataType)
			: mBuilder(builder)
		{
			mDataPtr = builder.acquireBufferPointer(dataType, &mDataSize);
		}

		~MeshBufferPointer()
		{
			mBuilder.releaseBufferPointer(mDataPtr);
		}

		template<typename T>
		T* as() { return (T*)mDataPtr; }

		size_t size() const { return mDataSize; }

	private:
		MeshBuilder&	mBuilder;
		void*			mDataPtr;
		size_t			mDataSize;
	};
};

void TangentSpaceBuilder::computeTangentBasis(
	const Vec3f& P1, const Vec3f& P2, const Vec3f& P3, 
	const Vec2f& UV1, const Vec2f& UV2, const Vec2f& UV3,
	Vec3f &tangent, Vec3f &bitangent
	)
{
	// reference: http://www.3dkingdoms.com/weekly/weekly.php?a=37
	Vec3f Edge1 = P2 - P1;
	Vec3f Edge2 = P3 - P1;
	Vec2f Edge1uv = UV2 - UV1;
	Vec2f Edge2uv = UV3 - UV1;

	float cp = Edge1uv.y * Edge2uv.x - Edge1uv.x * Edge2uv.y;

	if ( cp != 0.0f )
	{
		float mul = 1.0f / cp;
		tangent   = (Edge1 * -Edge2uv.y + Edge2 * Edge1uv.y) * mul;
		bitangent = (Edge1 * -Edge2uv.x + Edge2 * Edge1uv.x) * mul;

		tangent.normalize();
		bitangent.normalize();
	}
}

void TangentSpaceBuilder::compute(MeshBuilder& builder, Mesh::DataType uvDataType, Mesh::DataType tangentDataType)
{
	const uint cCurrentFormat = builder.format();

	if(0 == (cCurrentFormat & tangentDataType))
		return;

	// acquire mesh buffer pointers
	MeshBufferPointer xyzPtr(builder, Mesh::Position);
	MeshBufferPointer nrmPtr(builder, Mesh::Normal);
	MeshBufferPointer uvPtr(builder, uvDataType);
	MeshBufferPointer tangPtr(builder, tangentDataType);

	// make sure the data
	if(xyzPtr.size() != uvPtr.size() || xyzPtr.size() != tangPtr.size())
		return;

	MeshBufferPointer idxPtr(builder, Mesh::Index);

	// we supports triangles only :-)
	const size_t cFaceCnt	= idxPtr.size() / 3;
	const size_t cVertexCnt = xyzPtr.size() / 3;

	uint16_t const* indexBuf	= idxPtr.as<uint16_t>();
	Vec3f const* xyzBuf			= xyzPtr.as<Vec3f>();
	Vec3f const* nrmBuf			= nrmPtr.as<Vec3f>();
	Vec2f const* uvBuf			= uvPtr.as<Vec2f>();
	Vec3f* tangBuf				= tangPtr.as<Vec3f>();

	// initialize tangents to zero-length vectors
	for(size_t ivert = 0; ivert < cVertexCnt; ++ivert)
		tangBuf[ivert] = Vec3f::cZero;

	// compute tangent for each face and add to each corresponding vertex
	for(size_t iface = 0; iface < cFaceCnt; ++iface)
	{
		const uint16_t v0 = indexBuf[iface*3+0];
		const uint16_t v1 = indexBuf[iface*3+1];
		const uint16_t v2 = indexBuf[iface*3+2];

		const Vec3f& vN0 = nrmBuf[v0];
		const Vec3f& vN1 = nrmBuf[v1];
		const Vec3f& vN2 = nrmBuf[v2];

		Vec3f vT, vB;

		computeTangentBasis
			( xyzBuf[v0], xyzBuf[v1], xyzBuf[v2]
			, uvBuf[v0], uvBuf[v1], uvBuf[v2]
			, vT, vB );

		Vec3f vT0 = (vT - vN0 * vT.dot(vN0));
		Vec3f vT1 = (vT - vN1 * vT.dot(vN1));
		Vec3f vT2 = (vT - vN2 * vT.dot(vN2));

		vT0.normalize();
		vT1.normalize();
		vT2.normalize();

		//Vec3f vB0 = vT0.cross(vN0);
		//Vec3f vB1 = vT1.cross(vN1);
		//Vec3f vB2 = vT2.cross(vN2);
		
		// write smoothed tangents back
		tangBuf[v0] += vT0;
		tangBuf[v1] += vT1;
		tangBuf[v2] += vT2;
	}

	// finally normalize the tangents
	for(size_t ivert = 0; ivert < cVertexCnt; ++ivert)
		tangBuf[ivert].normalize();
}

}	// namespace MCD

TEST(NormalMappingTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=NormalMappingTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
			{	// Setup entity 1
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"ChamferBox 1";
				e->asChildOf(&mRootNode);
				e->localTransform = Mat44f(Mat33f::rotateXYZ(0, Mathf::cPiOver4(), 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(0.4f, 3);
				TangentSpaceBuilder().compute(chamferBoxBuilder, Mesh::TextureCoord0, Mesh::TextureCoord1);

				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/normalmapping.fx.xml").get());
				e->addComponent(c);

				e.release();
			}

			{	// Setup entity 2
				std::auto_ptr<Entity> e(new Entity);
				e->name = L"Sphere 1";
				e->asChildOf(&mRootNode);
				e->localTransform.setTranslation(Vec3f(1, 0, 0));

				// Setup the chamfer box mesh
				MeshPtr mesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(1.0f, 5);
				TangentSpaceBuilder().compute(chamferBoxBuilder, Mesh::TextureCoord0, Mesh::TextureCoord1);
				chamferBoxBuilder.commit(*mesh, MeshBuilder::Static);

				// Add component
				MeshComponent* c = new MeshComponent;
				c->mesh = mesh;
				c->effect = static_cast<Effect*>(mResourceManager.load(L"Material/normalmapping.fx.xml").get());
				e->addComponent(c);

				e.release();
			}
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			glTranslatef(0.0f, 0.0f, -5.0f);

			RenderableComponent::traverseEntities(&mRootNode);
		}

		Entity mRootNode;

		DefaultResourceManager mResourceManager;
	};	// TestWindow

	{
		TestWindow window;
		window.update(0.1f);
		window.mainLoop();
	}
}
