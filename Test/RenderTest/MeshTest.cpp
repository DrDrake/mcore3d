#include "Pch.h"
#include "../../MCD/Render/MeshBuilder.h"

using namespace MCD;

TEST(Basic_MeshBuilderTest)
{
	{	// Simple create and destroy a mesh
		MCD::ResourcePtr mesh = new Mesh(L"");
		mesh = nullptr;
	}

	{	// Simple create and destroy a mesh builder
		MeshBuilder builder;
	}

	{	// Enable with different formats
		MeshBuilder builder;

		builder.enable(Mesh::Position);
		CHECK(builder.format() & Mesh::Position);

		builder.enable(Mesh::Normal);
		CHECK(builder.format() & Mesh::Normal);

		builder.enable(Mesh::Color);
		CHECK(builder.format() & Mesh::Color);

		builder.enable(Mesh::Index);
		CHECK(builder.format() & Mesh::Index);

		for(size_t i=0; i<Mesh::cMaxTextureCoordCount; ++i) {
			builder.enable(Mesh::TextureCoord0 + i);
			Mesh::DataType textureUnit = Mesh::DataType(Mesh::TextureCoord0 + i);
			builder.textureUnit(textureUnit);
			builder.textureCoordSize(2);
			CHECK_EQUAL(uint(textureUnit), (builder.format() & Mesh::TextureCoord));
		}
	}

	{	// Fill some data
		MeshBuilder builder;

		// Mesh::TextureCoord1 also implies Mesh::TextureCoord0
		builder.enable(Mesh::Position | Mesh::Color | Mesh::Normal | Mesh::Index | Mesh::TextureCoord1);

		CHECK(builder.format() & Mesh::Position);
		CHECK(builder.format() & Mesh::Color);
		CHECK(builder.format() & Mesh::Normal);
		CHECK(builder.format() & Mesh::Index);
		CHECK_EQUAL(2u, builder.format() & Mesh::TextureCoord);

		builder.textureUnit(Mesh::TextureCoord0);
		builder.textureCoordSize(2);
		builder.textureUnit(Mesh::TextureCoord1);
		builder.textureCoordSize(3);

		builder.reserveVertex(3);
		builder.reserveTriangle(1);

		// TODO: Implement
//		builder.color();
		builder.normal(Vec3f::c001);

		{	uint16_t index[3];
			for(size_t i=0; i<3; ++i) {
				builder.position(Vec3f(1.23f + i));
				builder.textureUnit(Mesh::TextureCoord0);
				builder.textureCoord(Vec2f::c01 + float(i));
				builder.textureUnit(Mesh::TextureCoord1);
				builder.textureCoord(Vec3f::c100 + float(i));

				index[i] = builder.addVertex();
				CHECK_EQUAL(i, index[i]);
			}

			builder.addTriangle(index[0], index[1], index[2]);
		}

		// Get back the buffer pointers, and their element counts
		{	size_t count = 0;
			float* position = reinterpret_cast<float*>(builder.acquireBufferPointer(Mesh::Position, &count));
			CHECK_EQUAL(3u, count);
			CHECK_EQUAL(1.23f, *position);
			builder.releaseBufferPointer(position);
		}

		{	size_t count = 0;
			uint16_t* index = reinterpret_cast<uint16_t*>(builder.acquireBufferPointer(Mesh::Index, &count));
			CHECK_EQUAL(3u, count);
			for(size_t i=0; i<3; ++i)
				CHECK_EQUAL(i, index[i]);
			builder.releaseBufferPointer(index);
		}

		BasicGlWindow window(L"show=0, width=1, height=1");
		MeshPtr mesh = new Mesh(L"");
		builder.commit(*mesh, MeshBuilder::Static);

		CHECK(mesh->format() == builder.format());
		CHECK_EQUAL(3u, mesh->vertexCount());
		CHECK_EQUAL(3u, mesh->indexCount());
	}
}

TEST(Cube_MeshBuilderTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L""), mAngle(0)
		{
			mMesh = new Mesh(L"");
			MeshBuilder builder;

			builder.enable(Mesh::Position | Mesh::Normal | Mesh::TextureCoord0 | Mesh::Index);
				builder.textureUnit(Mesh::TextureCoord0);
				builder.textureCoordSize(2);

				builder.reserveVertex(4);
				builder.reserveTriangle(2);
				builder.textureUnit(Mesh::TextureCoord0);

				builder.normal(Vec3f(0, 0, 1));
				builder.position(Vec3f(-1, 1, 1));
				builder.textureCoord(Vec2f(0, 1));
				uint16_t idx1 = builder.addVertex();

				builder.position(Vec3f(-1, -1, 1));
				builder.textureCoord(Vec2f(0, 0));
				uint16_t idx2 = builder.addVertex();

				builder.position(Vec3f(1, -1, 1));
				builder.textureCoord(Vec2f(1, 0));
				uint16_t idx3 = builder.addVertex();

				builder.position(Vec3f(1, 1, 1));
				builder.textureCoord(Vec2f(1, 1));
				uint16_t idx4 = builder.addVertex();

				builder.addTriangle(idx1, idx2, idx3);
				builder.addTriangle(idx3, idx4, idx1);
			builder.commit(*mMesh, MeshBuilder::Static);
		}

		sal_override void update(float deltaTime)
		{
			glTranslatef(0.0f, 0.0f, -5.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			glColor3f(1, 0, 0);
			mMesh->draw();
		}

		MCD::MeshPtr mMesh;
		float mAngle;
	};

	TestWindow window;
	window.mainLoop();
}

#include "ChamferBox.h"
#include "../../MCD/Render/JpegLoader.h"
#include "../../MCD/Render/PngLoader.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/StrUtility.h"
#include "../../MCD/Core/System/Utility.h"
#include <fstream>
#include <list>

class Dummy {
public:
	Dummy(std::istream& is) : mIs(is) {}
	template<typename T>
	void read(T& t) {
		MCD_ASSERT(!eof());
		mIs.read((char*)&t, sizeof(t));
	}
	void skip(size_t count) {
		mIs.seekg(count, std::ios_base::cur);
	}
	bool eof() const {
		return mIs.eof();
	}

	std::istream& mIs;
};

class Model : Noncopyable
{
public:
	Model()
	{
	}

	void draw()
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if(!mTextures.empty())
			mTextures.front()->bind();

		MCD_FOREACH(const MeshPtr& mesh, mMeshes) {
			mesh->draw();
		}
	}

public:
	typedef IntrusivePtr<Mesh> MeshPtr;
	typedef std::list<MeshPtr> MeshList;
	MeshList mMeshes;

	typedef IntrusivePtr<Texture> TexturePtr;
	typedef std::list<TexturePtr> TextureList;
	TextureList mTextures;
};	// Model

// Compute vertex normals
// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=313015
// Reference: http://www.devmaster.net/forums/showthread.php?t=414
void computeNormal(Vec3f* vertex, Vec3f* normal, uint16_t* index, size_t vertexCount, size_t indexCount)
{
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
}

/*!
	\sa http://www.flipcode.com/archives/Another_3DS_LoaderViewer_Class.shtml
	\sa http://www.gamedev.net/community/forums/topic.asp?topic_id=382606
 */
class Max3dsLoader
{
public:
	Max3dsLoader(std::istream& is_)
		: mStream(is_)
	{
		using namespace std;

		Dummy is(is_);

		size_t i;	// Index variable
		uint16_t l_chunk_id;	// Chunk identifier
		uint32_t l_chunk_lenght;	// Chunk lenght

//		unsigned char l_char; //Char variable
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
				{	std::wstring objectName;
					readString(objectName);

					MeshBuilder* builder = new MeshBuilder;
					mMeshBuilders.push_back(builder);
					builder->enable(Mesh::Position | Mesh::Normal | Mesh::Index);
				}
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
				{
					is.read(l_qty);
					MCD_ASSERT(!mMeshBuilders.empty());
					MeshBuilder* builder = mMeshBuilders.back();
					builder->reserveVertex(l_qty);
					printf("Number of vertices: %d\n", l_qty);
					for(i=0; i<l_qty; ++i) {
						Vec3f v;
						MCD_ASSERT(sizeof(v) == 3 * sizeof(float));
						is.read(v);

						// Swap y and z poisition
						std::swap(v.y, v.z);
						v.z = -v.z;

						builder->position(v);
						builder->normal(Vec3f::cZero);	// We calculate the normal after all faces are known
						builder->addVertex();
					}
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
				{
					is.read(l_qty);
					MCD_ASSERT(!mMeshBuilders.empty());
					MeshBuilder* builder = mMeshBuilders.back();
					builder->reserveTriangle(l_qty);
					printf("Number of polygons: %d\n", l_qty); 
					for(i=0; i<l_qty; ++i) {
						uint16_t i1, i2, i3;
						is.read(i1);
						is.read(i2);
						is.read(i3);
						is.read(l_face_flags);
						builder->addTriangle(i1, i2, i3);
					}

					size_t vertexCount, indexCount;
					Vec3f* vertex = reinterpret_cast<Vec3f*>(builder->acquireBufferPointer(Mesh::Position, &vertexCount));
					Vec3f* normal = reinterpret_cast<Vec3f*>(builder->acquireBufferPointer(Mesh::Normal));
					uint16_t* index = reinterpret_cast<uint16_t*>(builder->acquireBufferPointer(Mesh::Index, &indexCount));

					computeNormal(vertex, normal, index, vertexCount, indexCount);

					builder->releaseBufferPointer(index);
					builder->releaseBufferPointer(normal);
					builder->releaseBufferPointer(vertex);
				}
				break;

				//------------- TRI_MAPPINGCOORS ------------
				// Description: Vertices list
				// Chunk Lenght: 1 x unsigned short (number of mapping points) 
				//             + 2 x float (mapping coordinates) x (number of mapping points)
				//             + sub chunks
				//-------------------------------------------
				case 0x4140:
				{
					is.read(l_qty);
					MCD_ASSERT(!mMeshBuilders.empty());
					MeshBuilder* builder = mMeshBuilders.back();

					builder->enable(Mesh::TextureCoord0);
					builder->textureUnit(Mesh::TextureCoord0);
					builder->textureCoordSize(2);
					size_t coordCount;
					Vec2f* coord = reinterpret_cast<Vec2f*>(builder->acquireBufferPointer(Mesh::TextureCoord0, &coordCount));

					MCD_ASSERT(l_qty == coordCount);
					for(i=0; i<l_qty; i++) {
						is.read(coord[i]);
						// Open gl flipped the texture vertically
						// Reference: http://www.devolution.com/pipermail/sdl/2002-September/049064.html
						coord[i].y = 1 - coord[i].y;
					}

					builder->releaseBufferPointer(coord);
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
	}

	~Max3dsLoader()
	{
		MCD_FOREACH(MeshBuilder* builder, mMeshBuilders)
			delete builder;
	}

	void readString(std::wstring& str)
	{
		size_t i = 0;
		std::string objectName;
		const size_t maxLength = 128;	// Max length to prevent infinite loop
		char c;
		do {
			mStream.read(c);
			objectName += c;
			++i;
		} while(c != '\0' && i < maxLength);

		str = str2WStr(objectName);
	}

	void commit(Model& model, MeshBuilder::StorageHint storageHint)
	{
		model.mMeshes.clear();
		MCD_FOREACH(MeshBuilder* builder, mMeshBuilders) {
			MeshPtr mesh = new Mesh(L"");
			model.mMeshes.push_back(mesh);
			builder->commit(*mesh, storageHint);
		}
	}

	Dummy mStream;

	std::list<MeshBuilder*> mMeshBuilders;
	MeshBuilder mMeshBuilder;

	typedef IntrusivePtr<Mesh> MeshPtr;
	typedef std::list<MeshPtr> MeshList;
	MeshPtr mCurrentMesh;
	MeshList mMeshes;

	typedef IntrusivePtr<Texture> TexturePtr;
	typedef std::list<TexturePtr> TextureList;
	TextureList mTextures;
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
//			mMesh = new Mesh(L"");
//			Texture* texture = new Texture(L"spaceshiptexture.jpg");
			Texture* texture = new Texture(L"LDoberman.jpg");
			mModel.mTextures.push_back(texture);
			JpegLoader* loader = new JpegLoader;
			RawFileSystem fs(L"./");
			std::auto_ptr<std::istream> is = fs.openRead(texture->fileId());

			while(!(loader->load(is.get()) & IResourceLoader::Stopped));
			loader->commit(*texture);

			delete loader;
		}

		sal_override void update(float deltaTime)
		{
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glTranslatef(0.0f, 0.0f, -50.0f);
			mAngle += deltaTime * 10;
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			const float scale = 0.5f;
			glScalef(scale, scale, scale);

//			Mesh& mesh = static_cast<Mesh&>(*mMesh);
//			mesh.draw();
			mModel.draw();
			glPopMatrix();
		}

//		MCD::ResourcePtr mMesh;
		Model mModel;
		float mAngle;
	};

	{
//		std::ifstream is("spaceship.3DS", std::ios_base::binary);
//		std::ifstream is("Nathalie aguilera Boing 747.3DS", std::ios_base::binary);
		std::ifstream is("Dog 1 N280708.3ds", std::ios_base::binary);
//		std::ifstream is("Leon N300708.3DS", std::ios_base::binary);
//		std::ifstream is("Ford N120208.3ds", std::ios_base::binary);
//		std::ifstream is("musai.3DS", std::ios_base::binary);
		CHECK(is.is_open());
		Max3dsLoader loader(is);
		TestWindow window;
		ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);

//		chamferBoxBuilder.commit(static_cast<Mesh&>(*window.mMesh), MeshBuilder::Static);

		loader.commit(window.mModel, MeshBuilder::Static);

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
