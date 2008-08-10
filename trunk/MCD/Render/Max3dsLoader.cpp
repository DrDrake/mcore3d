#include "Pch.h"
#include "Max3dsLoader.h"
#include "Texture.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

void Material::bind() const
{
	{	GLfloat ambient[] = { mAmbient.r, mAmbient.g, mAmbient.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	}

	{	GLfloat diffuse[] = { mDiffuse.r, mDiffuse.g, mDiffuse.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	}

	{	GLfloat specular[] = { mSpecular.r, mSpecular.g, mSpecular.b, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, mShininess);
	}

	if(mTexture) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		mTexture->bind();
	}
}

void Model::draw()
{
	MCD_FOREACH(const MeshAndMaterial& mesh, mMeshes) {
		mesh.material.bind();
		mesh.mesh->draw();
	}
}

namespace {

// Named enums for the chunk id
// Reference: http://www.flipcode.com/archives/Another_3DS_LoaderViewer_Class.shtml
enum ChunkId
{
	MAIN3DS							= 0x4D4D,
		MAIN_VERS					= 0x0002,
		EDIT3DS						= 0x3D3D,
			MATERIAL				= 0xAFFF,
				MAT_NAME			= 0xA000,
				MAT_AMBIENT			= 0xA010,
				MAT_DIFFUSE			= 0xA020,
				MAT_SPECULAR		= 0xA030,
				SHINY_PERC			= 0xA040,
				SHINY_STR_PERC		= 0xA041,
				TRANS_PERC			= 0xA050,
				TRANS_FOFF_PERC		= 0xA052,
				REF_BLUR_PERC		= 0xA053,
				RENDER_TYPE			= 0xA100,
				SELF_ILUM			= 0xA084,
				MAT_SEFL_ILPCT		= 0xA08A,
				WIRE_THICKNESS		= 0xA087,
				MAT_TEXMAP			= 0xA200,
					MAT_MAPNAME		= 0xA300,
			MESH_VERS				= 0x3D3E,
			OBJECT					= 0x4000,
				TRIG_MESH			= 0x4100,
					VERT_LIST		= 0x4110,
					FACE_DESC		= 0x4120,
						FACE_MAT	= 0x4130,
					TEX_VERTS		= 0x4140,
						SMOOTH_GROUP= 0x4150,
					LOCAL_COORDS	= 0x4160,
			ONE_UNIT				= 0x0100,
		KEYF3DS						= 0xB000,
			FRAMES					= 0xB008,
			MESH_INFO				= 0xB002,
				HIER_POS			= 0xB030,
				HIER_FATHER			= 0xB010,
				PIVOT_PT			= 0xB013,
				TRACK00				= 0xB020,
				TRACK01				= 0xB021,
				TRACK02				= 0xB022,
	COLOR_RGB						= 0x0010,
	COLOR_TRU						= 0x0011,
	COLOR_TRUG						= 0x0012,
	COLOR_RGBG						= 0x0013,
	PERC_INT						= 0x0030,
	COLOPERC_FLOAT					= 0x0031,
};	// ChunkId

struct ChunkHeader
{
	uint16_t id;
	uint32_t length;
};

}	// namespace

class Max3dsLoader::NamedMaterial : public Material
{
public:
	std::wstring mName;
};

class Max3dsLoader::Stream
{
public:
	Stream(std::istream& is) : mIs(is) {}

	template<typename T>
	void read(T& t)
	{
		MCD_ASSERT(!eof());
		mIs.read((char*)&t, sizeof(t));
	}

	template<typename T>
	void read(T& t, size_t size)
	{
		MCD_ASSERT(!eof());
		mIs.read((char*)&t, size);
	}

	void read(ChunkHeader& header)
	{
		read(header.id);
		if(!eof())
			read(header.length);
	}

	void skip(size_t count) {
		mIs.seekg(count, std::ios_base::cur);
	}
	bool eof() const {
		return mIs.eof();
	}

	std::istream& mIs;
};	// Stream

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

Max3dsLoader::Max3dsLoader(std::istream& is_, ResourceManager* resourceManager)
	: mStream(new Stream(is_)), mResourceManager(resourceManager)
{
	using namespace std;

	Stream is(is_);

	size_t i;	// Index variable

	ChunkHeader header;

	MeshBuilder* currentMeshBuilder = nullptr;
	NamedMaterial* currentMaterial = nullptr;

	uint16_t l_qty;	//Number of elements in each chunk

	while(true)
	{
		is.read(header);
		if(is.eof()) break;

		std::wstring str;

		switch(header.id)
		{
			//----------------- MAIN3DS -----------------
			// Description: Main chunk, contains all the other chunks
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case MAIN3DS: 
			break;

			//----------------- EDIT3DS -----------------
			// Description: 3D Editor chunk, objects layout info 
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case EDIT3DS:
			break;
			
			//--------------- EDIT_OBJECT ---------------
			// Description: Object block, info for each object
			// Chunk Lenght: len(object name) + sub chunks
			//-------------------------------------------
			case OBJECT: 
			{	// Currently the object name has no use.
				std::wstring objectName;
				readString(objectName);

				currentMeshBuilder = new MeshBuilder;
				MeshBuilderAndMaterial builderAndMaterial = { currentMeshBuilder, nullptr };
				mMeshBuilders.push_back(builderAndMaterial);
				currentMeshBuilder->enable(Mesh::Position | Mesh::Normal);
			}
				break;

			//--------------- OBJ_TRIMESH ---------------
			// Description: Triangular mesh, contains chunks for 3d mesh info
			// Chunk Lenght: 0 + sub chunks
			//-------------------------------------------
			case TRIG_MESH:
				break;

			//--------------- TRI_VERTEXL ---------------
			// Description: Vertices list
			// Chunk Lenght: 1 x uint16_t (number of vertices) 
			//             + 3 x float (vertex coordinates) x (number of vertices)
			//             + sub chunks
			//-------------------------------------------
			case VERT_LIST:
				is.read(l_qty);
				MCD_ASSERT(!mMeshBuilders.empty());
				MCD_ASSERT(currentMeshBuilder == mMeshBuilders.back().meshBuilder);
				currentMeshBuilder->reserveVertex(l_qty);

				for(i=0; i<l_qty; ++i) {
					Vec3f v;
					MCD_ASSERT(sizeof(v) == 3 * sizeof(float));
					is.read(v);

					// Now we should have all of the vertices read in. Because 3D Studio Max Models with the Z-Axis
					// pointing up (strange and ugly I know!), we need to flip the y values with the z values in our
					// vertices. That way it will be normal, with Y pointing up. Also, because we swap the Y and Z
					// we need to negate the Z to make it come out correctly.
					std::swap(v.y, v.z);
					v.z = -v.z;

					currentMeshBuilder->position(v);
					currentMeshBuilder->normal(Vec3f::cZero);	// We calculate the normal after all faces are known
					currentMeshBuilder->addVertex();
				}
				break;

			//--------------- TRI_FACEL1 ----------------
			// Description: Polygons (faces) list
			// Chunk Lenght: 1 x uint16_t (number of polygons) 
			//             + 3 x uint16_t (polygon points) x (number of polygons)
			//             + sub chunks
			//-------------------------------------------
			case FACE_DESC:
			{
				is.read(l_qty);
				MCD_ASSERT(!mMeshBuilders.empty());
				MCD_ASSERT(currentMeshBuilder == mMeshBuilders.back().meshBuilder);
				currentMeshBuilder->enable(Mesh::Index);
				currentMeshBuilder->reserveTriangle(l_qty);

				for(i=0; i<l_qty; ++i) {
					uint16_t i1, i2, i3;
					is.read(i1);
					is.read(i2);
					is.read(i3);
					uint16_t faceFlags;	// Flag that stores some face information (currently not used)
					is.read(faceFlags);
					currentMeshBuilder->addTriangle(i1, i2, i3);
				}

				size_t vertexCount, indexCount;
				Vec3f* vertex = reinterpret_cast<Vec3f*>(currentMeshBuilder->acquireBufferPointer(Mesh::Position, &vertexCount));
				Vec3f* normal = reinterpret_cast<Vec3f*>(currentMeshBuilder->acquireBufferPointer(Mesh::Normal));
				uint16_t* index = reinterpret_cast<uint16_t*>(currentMeshBuilder->acquireBufferPointer(Mesh::Index, &indexCount));

				computeNormal(vertex, normal, index, vertexCount, indexCount);

				currentMeshBuilder->releaseBufferPointer(index);
				currentMeshBuilder->releaseBufferPointer(normal);
				currentMeshBuilder->releaseBufferPointer(vertex);
			}
				break;

			//---------------- FACE_MAT -----------------
			// Description: Which material the mesh belongs to
			//-------------------------------------------
			case FACE_MAT:
			{
				MCD_ASSERT(!mMeshBuilders.empty());
				MCD_ASSERT(currentMeshBuilder == mMeshBuilders.back().meshBuilder);

				std::wstring materialName;
				size_t readCount = readString(materialName);

				// Loop for all materials to find the one with the same material name
				MCD_FOREACH(NamedMaterial* material, mMaterials) {
					if(material->mName != materialName)
						continue;
					mMeshBuilders.back().material = material;
					break;
				}

				// Read past the rest of the chunk since we don't care about shared vertices
				// You will notice we subtract the bytes already read in this chunk from the total length.
				is.skip(header.length - 6 - readCount);
			}
				break;

			//------------- TRI_MAPPINGCOORS ------------
			// Description: Vertices list
			// Chunk Lenght: 1 x unsigned short (number of mapping points) 
			//             + 2 x float (mapping coordinates) x (number of mapping points)
			//             + sub chunks
			//-------------------------------------------
			case TEX_VERTS:
			{
				is.read(l_qty);
				MCD_ASSERT(!mMeshBuilders.empty());
				MCD_ASSERT(currentMeshBuilder == mMeshBuilders.back().meshBuilder);

				currentMeshBuilder->enable(Mesh::TextureCoord0);
				currentMeshBuilder->textureUnit(Mesh::TextureCoord0);
				currentMeshBuilder->textureCoordSize(2);
				size_t coordCount;
				Vec2f* coord = reinterpret_cast<Vec2f*>(currentMeshBuilder->acquireBufferPointer(Mesh::TextureCoord0, &coordCount));

				MCD_ASSERT(l_qty == coordCount);
				for(i=0; i<l_qty; i++) {
					is.read(coord[i]);
					// Open gl flipped the texture vertically
					// Reference: http://www.devolution.com/pipermail/sdl/2002-September/049064.html
					coord[i].y = 1 - coord[i].y;
				}

				currentMeshBuilder->releaseBufferPointer(coord);
			}
				break;

			case MATERIAL:	// Material Start
				currentMaterial = new NamedMaterial;
				mMaterials.push_back(currentMaterial);
				break;

			case MAT_NAME:	// Material Name
				MCD_ASSUME(currentMaterial);
				readString(currentMaterial->mName);
				break;

			case MAT_AMBIENT:	// Material - Ambient Color
				MCD_ASSUME(currentMaterial);
				readColor(currentMaterial->mAmbient);
				break;

			case MAT_DIFFUSE:	// Material - Diffuse Color
				MCD_ASSUME(currentMaterial);
				readColor(currentMaterial->mDiffuse);
				break;

			case MAT_SPECULAR:	// Material - Spec Color
				MCD_ASSUME(currentMaterial);
				readColor(currentMaterial->mSpecular);
				break;

			case SHINY_PERC:	// Material - Shininess
				currentMaterial->mShininess = uint8_t(readPercInt());
				break;

			case SHINY_STR_PERC:	// Material - Shine Strength
				readPercInt();
				break;

			case TRANS_PERC:	// Material - Transparency
				readPercInt();
				break;

			case MAT_TEXMAP:	// Material - Start of Texture Info
				break;

			case MAT_MAPNAME:	// Material - Texture Name
				readString(str);
				if(mResourceManager)
					currentMaterial->mTexture = dynamic_cast<Texture*>(mResourceManager->load(str, false).get());
				else
					currentMaterial->mTexture = new Texture(str);
				break;

			//----------- Skip unknow chunks ------------
			// We need to skip all the chunks that currently we don't use
			// We use the chunk lenght information to set the file pointer
			// to the same level next chunk
			//-------------------------------------------
			default:
				is.skip(header.length - 6);
		}

		if(is.eof())
			break;
	}
}

Max3dsLoader::~Max3dsLoader()
{
	MCD_FOREACH(const MeshBuilderAndMaterial& builder, mMeshBuilders)
		delete builder.meshBuilder;
	MCD_FOREACH(NamedMaterial* material, mMaterials)
		delete material;
	delete mStream;
}

void Max3dsLoader::readColor(Color& color)
{
	ChunkHeader header;

	uint8_t uintColor[3];

	mStream->read(header);

	MCD_ASSERT(header.id == COLOR_TRU);

	mStream->read(uintColor, 3);
	color.r = float(uintColor[0]) / 256;
	color.g = float(uintColor[1]) / 256;
	color.b = float(uintColor[2]) / 256;
}

uint16_t Max3dsLoader::readPercInt()
{
	ChunkHeader header;
	mStream->read(header);

	MCD_ASSERT(header.id == PERC_INT);

	uint16_t a;
	mStream->read(a);

	return a;
}

size_t Max3dsLoader::readString(std::wstring& str)
{
	size_t i = 0;
	std::string objectName;
	const size_t maxLength = 128;	// Max length to prevent infinite loop
	char c;
	do {
		mStream->read(c);
		objectName += c;
		++i;
	} while(c != '\0' && i < maxLength);

	str = str2WStr(objectName);

	// Returns how much is read from the stream
	return i;
}

void Max3dsLoader::commit(Model& model, MeshBuilder::StorageHint storageHint)
{
	model.mMeshes.clear();
	MCD_FOREACH(const MeshBuilderAndMaterial& i, mMeshBuilders) {
		MeshBuilder* builder = i.meshBuilder;
		MeshPtr mesh = new Mesh(L"");
		Model::MeshAndMaterial meshMat;
		meshMat.mesh = mesh;
		if(i.material)
			meshMat.material = *(i.material);
		model.mMeshes.push_back(meshMat);
		builder->commit(*mesh, storageHint);
	}
}

}	// namespace MCD
