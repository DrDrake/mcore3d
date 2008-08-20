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

Material::Material()
	: mAmbient(0.5), mDiffuse(1), mSpecular(0), mShininess(0)
{
}

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
	else
		glBindTexture(GL_TEXTURE_2D, 0);
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
// Official 3Ds file SDK: http://usa.autodesk.com/adsk/servlet/item?siteID=123112&id=7481394
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
						SMOOTH_GROUP= 0x4150,
					TEX_VERTS		= 0x4140,
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

	void read(void* p, size_t size)
	{
		MCD_ASSERT(!eof());
		mIs.read((char*)p, size);
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

// Compute vertex normals
// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=313015
// Reference: http://www.devmaster.net/forums/showthread.php?t=414
static void computeNormal(const Vec3f* vertex, Vec3f* normal, const uint16_t* index, uint16_t vertexCount, size_t indexCount)
{
	// Calculate the face normal for each face
	for(size_t i=0; i<indexCount; i+=3) {
		uint16_t i0 = index[i+0];
		uint16_t i1 = index[i+1];
		uint16_t i2 = index[i+2];
		Vec3f v1 = vertex[i0];
		Vec3f v2 = vertex[i1];
		Vec3f v3 = vertex[i2];

		// We need not to normalize this faceNormal, since a vertex's normal
		// shoule be influenced by a larger polygon.
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

static void computeNormal(const Vec3f* vertex, Vec3f* normal, const uint16_t* index, const uint32_t* smoothingGroup, uint16_t vertexCount, size_t indexCount)
{
	const size_t triangleCount = indexCount / 3;
	std::vector<Vec3f> faceNormal;
	faceNormal.reserve(triangleCount);

	// Calculate the face normal for each triangle
	for(size_t f = 0; f<indexCount; f+=3) {
		uint16_t i0 = index[f+0];
		uint16_t i1 = index[f+1];
		uint16_t i2 = index[f+2];
		const Vec3f& v1 = vertex[i0];
		const Vec3f& v2 = vertex[i1];
		const Vec3f& v3 = vertex[i2];
		faceNormal.push_back((v3 - v2) ^ (v1 - v2));
	}

	//! Store info on which index is associated with a vertex (a single vertex can associate with a numbers of index)
	typedef std::vector<size_t> Indexes;
	typedef std::vector<Indexes> Vertex2TriangleIndexMapping;
	Vertex2TriangleIndexMapping mapping;
	mapping.resize(vertexCount);

	// Loop for every triangle f
	for(size_t f=0; f<indexCount; f+=3) {
		// Loop for every vertex of f, namely v
		for(size_t v=f; v<f+3u; ++v) {
			uint16_t indexOfV = index[v];
			mapping[indexOfV].push_back(v/3);	// Divided by 3 to make it triangle index
		}
	}

	// Loop for every triangle f
	for(size_t f=0; f<indexCount; f+=3) {
		// Loop for every vertex of f, namely v
		for(size_t v=f; v<f+3u; ++v) {
			// Loop for every triangle f2 in mapping[index of v]
			uint16_t indexOfV = index[v];
			const Indexes& indexes = mapping[indexOfV];
			MCD_FOREACH(uint16_t f2, indexes) {
				// If f2 and f share smoothing groups
				if(smoothingGroup[f/3] | smoothingGroup[f2])
					normal[indexOfV] += faceNormal[f2];
			}
		}
	}

	// Normalize for each vertex normal
	for(uint16_t v=0; v<vertexCount; ++v)
		normal[v].normalize();
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
				ModelInfo modelInfo = { currentMeshBuilder };
				mModelInfo.push_back(modelInfo);
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
				MCD_ASSERT(!mModelInfo.empty());
				MCD_ASSERT(currentMeshBuilder == mModelInfo.back().meshBuilder);
				MCD_ASSUME(currentMeshBuilder);
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
				uint16_t faceCount;
				is.read(faceCount);
				MCD_ASSERT(!mModelInfo.empty());

				std::vector<uint16_t>& idx = mModelInfo.back().index;
				idx.resize(faceCount * 3);	// Each triangle has 3 vertex

				for(i=0; i<faceCount; ++i) {
					// Read 3 indexes at once
					is.read(&idx[i * 3], 3 * sizeof(uint16_t));
					uint16_t faceFlags;	// Flag that stores some face information (currently not used)
					is.read(faceFlags);
				}
			}
				break;

			//---------------- FACE_MAT -----------------
			// Description: Which material the face belongs to
			//-------------------------------------------
			case FACE_MAT:
			{
				MCD_ASSERT(!mModelInfo.empty());
				MCD_ASSERT(currentMeshBuilder == mModelInfo.back().meshBuilder);

				std::wstring materialName;
				readString(materialName);

				// Get the number of faces of the object concerned by this material
				uint16_t faceCount;
				is.read(faceCount);

				if(faceCount == 0)
					break;

				{	// Insert a new MultiSubObject into the current mesh
					MultiSubObject object = { nullptr };
					mModelInfo.back().multiSubObject.push_back(object);
				}

				// Read the array which describle which faces in the mesh use this material
				MultiSubObject& object = mModelInfo.back().multiSubObject.back();
				object.mFaceIndex.resize(faceCount);
				is.read(&object.mFaceIndex[0], faceCount * sizeof(uint16_t));

				// Loop for all materials to find the one with the same material name
				MCD_FOREACH(NamedMaterial* material, mMaterials) {
					if(material->mName != materialName)
						continue;
					object.material = material;
					break;
				}

				MCD_ASSERT(object.material != nullptr);
			}
				break;

			case SMOOTH_GROUP:
			{
				MCD_ASSERT(!mModelInfo.empty());
				MCD_ASSERT(currentMeshBuilder == mModelInfo.back().meshBuilder);
				std::vector<uint32_t>& smoothingGroup = mModelInfo.back().smoothingGroup;
				smoothingGroup.resize(mModelInfo.back().index.size() / 3);	// Count of index / 3 = numbers of face

				is.read(&smoothingGroup[0], smoothingGroup.size() * sizeof(uint32_t));
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
				MCD_ASSERT(!mModelInfo.empty());
				MCD_ASSERT(currentMeshBuilder == mModelInfo.back().meshBuilder);

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

			case SHINY_PERC:	// Material - Shininess (Glossiness)
			{	uint16_t shininess = Math<int16_t>::clamp(readPercentageAsInt(), 0, 100);
				// Rescle from 0-100 to 0-128 since the maximum accepted value for
				// glMateriali with GL_SHININESS is 128
				currentMaterial->mShininess = uint8_t(shininess * 128.f / 100);
			}
				break;

			case SHINY_STR_PERC:	// Material - Shine Strength
			{	uint16_t strength = readPercentageAsInt();
				currentMaterial->mSpecular *= float(strength) / 100;
			}
				break;

			case TRANS_PERC:	// Material - Transparency
				readPercentageAsInt();
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

	// Calculate vertex normal for each mesh
	MCD_FOREACH(const ModelInfo& model, mModelInfo) {
		MeshBuilder* meshBuilder = model.meshBuilder;
		size_t vertexCount, indexCount = model.index.size();

		if(indexCount == 0)
			continue;

		Vec3f* vertex = reinterpret_cast<Vec3f*>(meshBuilder->acquireBufferPointer(Mesh::Position, &vertexCount));
		Vec3f* normal = reinterpret_cast<Vec3f*>(meshBuilder->acquireBufferPointer(Mesh::Normal));
		const uint16_t* index = &model.index[0];

		if(vertex && normal && indexCount > 0) {
			// We have 2 different routines for calculating normals, one with the 
			// smoothing group information and one does not.
			if(model.smoothingGroup.empty())
				computeNormal(vertex, normal, index, uint16_t(vertexCount), indexCount);
			else
				computeNormal(vertex, normal, index, &(model.smoothingGroup[0]), uint16_t(vertexCount), indexCount);
		}

		meshBuilder->releaseBufferPointer(normal);
		meshBuilder->releaseBufferPointer(vertex);
	}
}

Max3dsLoader::~Max3dsLoader()
{
	MCD_FOREACH(const ModelInfo& model, mModelInfo)
		delete model.meshBuilder;
	MCD_FOREACH(NamedMaterial* material, mMaterials)
		delete material;
	delete mStream;
}

void Max3dsLoader::readColor(ColorRGBf& color)
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

int16_t Max3dsLoader::readPercentageAsInt()
{
	ChunkHeader header;
	mStream->read(header);

	MCD_ASSERT(header.id == PERC_INT);

	int16_t percentage;
	mStream->read(percentage);

	return Math<int16_t>::clamp(percentage, -100, 100);
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
	MCD_FOREACH(const ModelInfo& modelInfo, mModelInfo) {
		MeshBuilder* builder = modelInfo.meshBuilder;

		// Commit the mesh with the vertex buffer first
		MeshPtr meshWithoutIndex = new Mesh(L"tmp");
		builder->commit(*meshWithoutIndex, storageHint);

		MCD_FOREACH(const MultiSubObject& subObject, modelInfo.multiSubObject) {
			// Share all the buffers in meshWithoutIndex into this mesh
			// TODO: Add name to the mesh
			MeshPtr mesh = new Mesh(L"", *meshWithoutIndex);

			// Let mesh have it's own index buffer
			mesh->setHandlePtr(Mesh::Index, Mesh::HandlePtr(new uint(0)));

			// Setup the index buffer
			MeshBuilder indexBuilder;
			indexBuilder.enable(Mesh::Index);
			size_t faceCount = subObject.mFaceIndex.size();
			for(size_t i=0; i<faceCount; ++i) {
				const size_t vertexIdx = subObject.mFaceIndex[i] * 3;	// From face index to vertex index
				const uint16_t i1 = modelInfo.index[vertexIdx + 0];
				const uint16_t i2 = modelInfo.index[vertexIdx + 1];
				const uint16_t i3 = modelInfo.index[vertexIdx + 2];
				indexBuilder.addTriangle(i1, i2, i3);
			}
			indexBuilder.commit(*mesh, Mesh::Index, storageHint);

			// Assign material
			Model::MeshAndMaterial meshMat;
			meshMat.material = *subObject.material;

			meshMat.mesh = mesh;
			model.mMeshes.push_back(meshMat);
		}
	}
}

}	// namespace MCD
