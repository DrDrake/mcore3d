#include "Pch.h"
#include "../../MCD/Render/MeshBuilder.h"
#include "../../MCD/Core/Math/Vec3.h"

using namespace MCD;

TEST(Basic_MeshLoaderTest)
{
	{	// Simple create and destroy a mesh
		MCD::ResourcePtr mesh = new Mesh(L"");
		mesh = nullptr;
	}

	{	// Simple create and destroy a mesh builder
		MeshBuilder builder;
	}

	{	// Simple begin end with nothing
		MeshBuilder builder;
		builder.begin(Mesh::Position | Mesh::Normal | Mesh::Index);
		builder.end();
	}
}

#include "BasicGlWindow.h"
#include "ChamferBox.h"
#include <fstream>

class Dummy {
public:
	Dummy(std::istream& is) : mIs(is) {}
	template<typename T>
	void read(T& t) {
		MCD_ASSERT(!eof());
		mIs.read((char*)&t, sizeof(t));
	}
	void skip(size_t count) {
//		char buf[256];
//		mIs.read(buf, count);
		mIs.seekg(count, std::ios_base::cur);
	}
	bool eof() const {
		return mIs.eof();
	}

	std::istream& mIs;
};

class Max3dsLoader
{
public:
	Max3dsLoader(std::istream& is_)
	{
		using namespace std;

		Dummy is(is_);
		mMeshBuilder.begin(Mesh::Position | Mesh::Normal | Mesh::Index);

		size_t i;	// Index variable
		uint16_t l_chunk_id;	// Chunk identifier
		uint32_t l_chunk_lenght;	// Chunk lenght

		unsigned char l_char; //Char variable
		uint16_t l_qty;	//Number of elements in each chunk

		uint16_t l_face_flags;	//Flag that stores some face information

		while(true)
		{
			is.read(l_chunk_id);	// Read the chunk header
			if(is.eof()) break;
			printf("ChunkID: %x\n",l_chunk_id);
			is.read(l_chunk_lenght);	// Read the lenght of the chunk
			if(is.eof()) break;
			printf("ChunkLenght: %x\n", l_chunk_lenght);

			switch(l_chunk_id)
			{
				//----------------- MAIN3DS -----------------
				// Description: Main chunk, contains all the other chunks
				// Chunk Lenght: 0 + sub chunks
				//-------------------------------------------
				case 0x4d4d: 
				break;    

				//----------------- EDIT3DS -----------------
				// Description: 3D Editor chunk, objects layout info 
				// Chunk Lenght: 0 + sub chunks
				//-------------------------------------------
				case 0x3d3d:
				break;
				
				//--------------- EDIT_OBJECT ---------------
				// Description: Object block, info for each object
				// Chunk Lenght: len(object name) + sub chunks
				//-------------------------------------------
				case 0x4000: 
					i = 0;
					do {
						is.read(l_char);
//						p_object->name[i]=l_char;
						++i;
					} while(l_char != '\0' && i<20);
				break;

				//--------------- OBJ_TRIMESH ---------------
				// Description: Triangular mesh, contains chunks for 3d mesh info
				// Chunk Lenght: 0 + sub chunks
				//-------------------------------------------
				case 0x4100:
				break;

				//--------------- TRI_VERTEXL ---------------
				// Description: Vertices list
				// Chunk Lenght: 1 x uint16_t (number of vertices) 
				//             + 3 x float (vertex coordinates) x (number of vertices)
				//             + sub chunks
				//-------------------------------------------
				case 0x4110:
					is.read(l_qty);
					mMeshBuilder.reserveVertex(l_qty);
					printf("Number of vertices: %d\n", l_qty);
					for(i=0; i<l_qty; ++i) {
						Vec3f v;
						MCD_ASSERT(sizeof(v) == 3 * sizeof(float));
						is.read(v);
						mMeshBuilder.position(v);
						mMeshBuilder.normal(Vec3f::cZero);	// We calculate the normal after all faces are known
						mMeshBuilder.addVertex();
					}
				break;

				//--------------- TRI_FACEL1 ----------------
				// Description: Polygons (faces) list
				// Chunk ID: 4120 (hex)
				// Chunk Lenght: 1 x uint16_t (number of polygons) 
				//             + 3 x uint16_t (polygon points) x (number of polygons)
				//             + sub chunks
				//-------------------------------------------
				case 0x4120:
					is.read(l_qty);
					mMeshBuilder.reserveTriangle(l_qty);
					printf("Number of polygons: %d\n", l_qty); 
					for(i=0; i<l_qty; ++i) {
						uint16_t i1, i2, i3;
						is.read(i1);
						is.read(i2);
						is.read(i3);
						is.read(l_face_flags);
						mMeshBuilder.addTriangle(i1, i2, i3);
					}
				break;

				//----------- Skip unknow chunks ------------
				// We need to skip all the chunks that currently we don't use
				// We use the chunk lenght information to set the file pointer
				// to the same level next chunk
				//-------------------------------------------
				default:
					is.skip(l_chunk_lenght - 6);
			}

			if(is.eof())
				break;
		}

		// Build the vertex normals
		size_t vertexCount, indexCount;
		Vec3f* vertex = reinterpret_cast<Vec3f*>(mMeshBuilder.acquireBufferPointer(Mesh::Position, &vertexCount));
		Vec3f* normal = reinterpret_cast<Vec3f*>(mMeshBuilder.acquireBufferPointer(Mesh::Normal));
		uint16_t* index = reinterpret_cast<uint16_t*>(mMeshBuilder.acquireBufferPointer(Mesh::Index, &indexCount));

		// Calculate the face normal for each face
		for(size_t i=0; i<indexCount; i+=3) {
			uint16_t i0 = index[i+0];
			uint16_t i1 = index[i+1];
			uint16_t i2 = index[i+2];
			Vec3f v1 = vertex[i0];
			Vec3f v2 = vertex[i1];
			Vec3f v3 = vertex[i2];
			Vec3f faceNormal = (v3 - v2) ^ (v1 - v2);

			// Add the face normal to the corresponding vertices
			normal[i0] += faceNormal;
			normal[i1] += faceNormal;
			normal[i2] += faceNormal;
		}

		// Normalize for each vertex normal
		for(size_t i=0; i<vertexCount; ++i) {
			normal[i].normalize();
		}

		mMeshBuilder.end();
	}

	void commit(Mesh& mesh, MeshBuilder::StorageHint storageHint)
	{
		mMeshBuilder.commit(mesh, storageHint);
	}

	MeshBuilder mMeshBuilder;
};

TEST(Sphere_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L""), mAngle(0)
		{
			mMesh = new Mesh(L"");
		}

		sal_override void update(float deltaTime)
		{
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
//			glScalef(0.05f, 0.05f, 0.05f);
			glTranslatef(0.0f, 0.0f, -10.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			Mesh& mesh = static_cast<Mesh&>(*mMesh);
			mesh.draw();
			glPopMatrix();
		}

		MCD::ResourcePtr mMesh;
		float mAngle;
	};

	{	// Add some vertex with normal and index
		MeshBuilder builder;
		builder.begin(Mesh::Position | Mesh::Normal | Mesh::Index);
			builder.reserveVertex(4);
			builder.reserveTriangle(2);

			builder.normal(Vec3f(0, 0, 1));
			builder.position(Vec3f(-1, 1, 1));
			uint16_t idx1 = builder.addVertex();

			builder.position(Vec3f(1, 1, 1));
			uint16_t idx2 = builder.addVertex();

			builder.position(Vec3f(1, -1, 1));
			uint16_t idx3 = builder.addVertex();

			builder.position(Vec3f(-1, -1, 1));
			uint16_t idx4 = builder.addVertex();

			builder.addTriangle(idx1, idx2, idx3);
			builder.addTriangle(idx3, idx4, idx1);
		builder.end();

		std::ifstream is("spaceship.3DS", std::ios_base::binary);
		CHECK(is.is_open());
		Max3dsLoader loader(is);
		TestWindow window;
		ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);

		builder.commit(static_cast<Mesh&>(*window.mMesh), MeshBuilder::Static);
		chamferBoxBuilder.commit(static_cast<Mesh&>(*window.mMesh), MeshBuilder::Static);

		loader.commit(static_cast<Mesh&>(*window.mMesh), MeshBuilder::Static);

		// Set up and enable light 0
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

		GLfloat lightPos[] = { -50.f, 0.0f, 40.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		window.mainLoop();
	}
}
