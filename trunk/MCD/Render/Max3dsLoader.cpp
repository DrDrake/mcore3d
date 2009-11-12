#include "Pch.h"
#include "Max3dsLoader.h"
#include "Material.h"
#include "MeshBuilder.h"
#include "Mesh.h"
#include "EditableMesh.h"
#include "Model.h"
#include "Texture.h"
#include "TangentSpaceBuilder.h"
#include "../Core/Math/Vec2.h"
#include "../Core/Math/Vec3.h"
#include "../Core/Math/Mat44.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"
#include <list>
#include <memory>	// For auto_ptr

namespace MCD {

namespace {

/*!	Named enums for the chunk id.
	The indentation shows the parent/child relationship between the chunks.
	Reference: http://www.flipcode.com/archives/Another_3DS_LoaderViewer_Class.shtml
	http://gpwiki.org/index.php/C:Load3DS
	Official 3Ds file SDK: http://usa.autodesk.com/adsk/servlet/item?siteID=123112&id=7481394
 */
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
					TEX_VERTS		= 0x4140,
					LOCAL_COORDS	= 0x4160,
					FACE_DESC		= 0x4120,
						FACE_MAT	= 0x4130,
						SMOOTH_GROUP= 0x4150,
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
};	// ChunkHeader

class Max3dsMaterial : public Material
{
public:
	std::wstring mName;

// Attributes
	ColorRGBf ambient, diffuse, specular;
	uint8_t shininess;
	TexturePtr texture;
	bool inited;

	Max3dsMaterial() : inited(false) {}

	void init() {

		if(inited)
			return;

		this->mRenderPasses.push_back(new Pass);
		addProperty(
			new StandardProperty(
				ColorRGBAf(ambient, 1.0f)
				, ColorRGBAf(diffuse, 1.0f)
				, ColorRGBAf(specular, 1.0f)
				, ColorProperty::ColorOperation::Replace
				, shininess)
			, 0
			);

		if(texture) {
			addProperty(new TextureProperty(texture.get(), 0, GL_LINEAR, GL_LINEAR), 0);
		}

		inited = true;
	}
};	// Max3dsMaterial

/*!	This stream class provides some add on functionality over the std::istream,
	and provide more error checking.
 */
class Stream
{
public:
	Stream(std::istream& is, volatile IResourceLoader::LoadingState& loadingState)
		: mIs(is), mLoadingState(loadingState)
	{}

	template<typename T>
	void read(T& t)
	{
		mIs.read((char*)&t, sizeof(t));
		if(mIs.gcount() != sizeof(t))
			mLoadingState = IResourceLoader::Aborted;
	}

	void read(void* p, size_t size)
	{
		mIs.read((char*)p, size);
		if(mIs.gcount() != std::streamsize(size))
			mLoadingState = IResourceLoader::Aborted;
	}

	bool read(ChunkHeader& header)
	{
		header.id = 0;
		header.length = 0;

		// We use the raw std::istream to get ride the IResourceLoader::Aborted
		mIs.read((char*)&header.id, sizeof(header.id));
		mIs.read((char*)&header.length, sizeof(header.length));

		if(header.id == 0 || header.length == 0) {
			if(header.id != 0 || header.length != 0)
				mLoadingState = IResourceLoader::Aborted;
			return false;
		}

		return true;
	}

	void skip(size_t count) {
		mIs.seekg(count, std::ios_base::cur);
	}

	bool eof() const {
		return mIs.eof();
	}

	std::istream& mIs;
	volatile IResourceLoader::LoadingState& mLoadingState;
};	// Stream

}	// namespace

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
		// should be influenced by a larger polygon.
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

class Max3dsLoader::Impl
{
public:
	Impl(IResourceManager* resourceManager);

	~Impl();

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const wchar_t* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const;

	void readColor(ColorRGBf& color);

	int16_t readPercentageAsInt();

	size_t readString(std::wstring& str);

private:
	Stream* mStream;
	IResourceManager* mResourceManager;

	struct LoadOptions
	{
		bool includeTangents;
		bool keepMeshBuilders;
	};

	std::auto_ptr<LoadOptions> mLoadOptions;

	//! Represent which face the material is assigned to.
	struct MultiSubObject
	{
		//Material* material;
		Max3dsMaterial* material;
		std::vector<uint16_t> mFaceIndex;	//! Index to the index buffer
	};	// MultiSubObject

	struct ModelInfo
	{
		MeshBuilderPtr meshBuilder;	//! Contains vertex buffer only
		std::vector<uint16_t> index;//! The triangle index
		std::list<MultiSubObject> multiSubObject;
		std::vector<uint32_t> smoothingGroup;
	};	// ModelInfo

	std::list<ModelInfo> mModelInfo;

	typedef std::list<Max3dsMaterial*> MaterialList;
	MaterialList mMaterials;

	volatile IResourceLoader::LoadingState mLoadingState;
	mutable Mutex mMutex;
};	// Impl

Max3dsLoader::Impl::Impl(IResourceManager* resourceManager)
	: mStream(nullptr), mResourceManager(resourceManager), mLoadingState(NotLoaded)
{
}

Max3dsLoader::Impl::~Impl()
{
	MCD_FOREACH(Max3dsMaterial* material, mMaterials)
		delete material;
	delete mStream;
}

#define ABORTLOADING() { mLoadingState = Aborted; break; }

IResourceLoader::LoadingState Max3dsLoader::Impl::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	using namespace std;

	// parse the load options from args
	if(mLoadOptions.get() == nullptr)
	{
		mLoadOptions.reset(new LoadOptions);
		mLoadOptions->includeTangents = false;
		mLoadOptions->keepMeshBuilders = false;

		if(nullptr != args)
		{
			NvpParser parser(args);
			const wchar_t* name, *value;
			while(parser.next(name, value))
			{
				if(wstrCaseCmp(name, L"tangents") == 0 && wstrCaseCmp(value, L"true") == 0)
					mLoadOptions->includeTangents = true;

				if(wstrCaseCmp(name, L"editable") == 0 && wstrCaseCmp(value, L"true") == 0)
					mLoadOptions->keepMeshBuilders = true;
			}
		}
	}

	{	ScopeLock lock(mMutex);
		mLoadingState = is ? NotLoaded : Aborted;
	}

	if(mLoadingState & Stopped)
		return mLoadingState;

	if(!mStream)
		mStream = new Stream(*is, mLoadingState);

	ChunkHeader header;

	MeshBuilderPtr currentMeshBuilder = nullptr;
	Max3dsMaterial* currentMaterial = nullptr;

	// When mirror is used in the 3DS, the triangle winding order need to be inverted.
	// Trunk LOCAL_COORDS comes before FACE_DESC, during the loading of LOCAL_COORDS we got
	// the information to change the clockwise/anti-clockwise triangle winding or not
	// and apply this information during the loading of FACE_DESC
	bool invertWinding = false;

	while(mStream->read(header) && mLoadingState != Aborted)
	{
		switch(header.id)
		{
			//----------------- MAIN3DS -----------------
			// Description: Main chunk, contains all the other chunks
			// Chunk Length: 0 + sub chunks
			//-------------------------------------------
			case MAIN3DS:
			break;

			//----------------- EDIT3DS -----------------
			// Description: 3D Editor chunk, objects layout info
			// Chunk Length: 0 + sub chunks
			//-------------------------------------------
			case EDIT3DS:
			break;

			//--------------- OBJECT ---------------
			// Description: Object block, info for each object
			// Chunk Length: len(object name) + sub chunks
			//-------------------------------------------
			case OBJECT:
			{	// Currently the object name has no use.
				std::wstring objectName;
				readString(objectName);

				currentMeshBuilder = new MeshBuilder(false);
				if(!currentMeshBuilder)
					ABORTLOADING();

				ModelInfo modelInfo;
				modelInfo.meshBuilder = currentMeshBuilder;
				mModelInfo.push_back(modelInfo);
				currentMeshBuilder->enable(Mesh::Position | Mesh::Normal);
			}	break;

			//--------------- TRIG_MESH ---------------
			// Description: Triangular mesh, contains chunks for 3d mesh info
			// Chunk Length: 0 + sub chunks
			//-------------------------------------------
			case TRIG_MESH:
				break;

			//--------------- VERT_LIST ---------------
			// Description: Vertices list
			// Chunk Length: 1 x uint16_t (number of vertices)
			//             + 3 x float (vertex coordinates) x (number of vertices)
			//             + sub chunks
			//-------------------------------------------
			case VERT_LIST:
			{
				uint16_t vertexCount = 0;
				mStream->read(vertexCount);

				if(mModelInfo.empty() || currentMeshBuilder != mModelInfo.back().meshBuilder)
					ABORTLOADING();

				currentMeshBuilder->reserveVertex(vertexCount);

				for(size_t i=0; i<vertexCount; ++i) {
					Vec3f v;
					MCD_ASSERT(sizeof(v) == 3 * sizeof(float));
					mStream->read(v);

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
			}	break;

			case LOCAL_COORDS:
			{
				Mat44f matrix = Mat44f::cIdentity;
				for(size_t i=0; i<4; ++i)
					mStream->read(matrix.data2D[i], sizeof(float) * 3);

				if((invertWinding = (matrix.determinant() < 0)) == true) {
					Mat44f inv = matrix.inverse();

					matrix.m00 = -matrix.m00;
					matrix.m01 = -matrix.m01;
					matrix.m02 = -matrix.m02;
					matrix.m03 = -matrix.m03;

					matrix = (inv * matrix);

					size_t vertexCount = 0;
					Vec3f* vertex = reinterpret_cast<Vec3f*>(currentMeshBuilder->acquireBufferPointer(Mesh::Position, &vertexCount));

					for(size_t i=0; i<vertexCount; ++i) {
						Vec4f tmp(vertex[i].x, -vertex[i].z, vertex[i].y, 0);
						// TODO: Use matrix.transform once the function is available
						tmp = (matrix * tmp) + matrix[3];
						vertex[i] = Vec3f(tmp.x, tmp.z, -tmp.y);
					}

					currentMeshBuilder->releaseBufferPointer(vertex);
				}
			}	break;

			//--------------- FACE_DESC ----------------
			// Description: Polygons (faces) list
			// Chunk Length: 1 x uint16_t (number of polygons)
			//             + 3 x uint16_t (polygon points) x (number of polygons)
			//             + sub chunks
			//-------------------------------------------
			case FACE_DESC:
			{
				uint16_t faceCount = 0;
				mStream->read(faceCount);

				if(mModelInfo.empty())
					ABORTLOADING();

				std::vector<uint16_t>& idx = mModelInfo.back().index;
				idx.resize(faceCount * 3);	// Each triangle has 3 vertex

				for(size_t i=0; i<faceCount; ++i) {
					// Read 3 indexes at once
					mStream->read(&idx[i * 3], 3 * sizeof(uint16_t));
					uint16_t faceFlags;	// Flag that stores some face information (currently not used)
					mStream->read(faceFlags);
				}

				if(invertWinding) {
					for(size_t i=0; i<idx.size(); i+=3)
						std::swap(idx[i], idx[i+2]);
				}
			}	break;

			//---------------- FACE_MAT -----------------
			// Description: Which material the face belongs to
			//-------------------------------------------
			case FACE_MAT:
			{
				if(mModelInfo.empty())
					ABORTLOADING();

				std::wstring materialName;
				readString(materialName);

				// Get the number of faces of the object concerned by this material
				uint16_t faceCount = 0;
				mStream->read(faceCount);

				if(faceCount == 0)
					break;

				{	// Insert a new MultiSubObject into the current mesh
					MultiSubObject object;
					object.material = nullptr;
					mModelInfo.back().multiSubObject.push_back(object);
				}

				// Read the array which describe which faces in the mesh use this material
				MultiSubObject& object = mModelInfo.back().multiSubObject.back();
				object.mFaceIndex.resize(faceCount);
				mStream->read(&object.mFaceIndex[0], faceCount * sizeof(uint16_t));

				// Loop for all materials to find the one with the same material name
				MCD_FOREACH(Max3dsMaterial* material, mMaterials) {
					if(material->mName != materialName)
						continue;
					object.material = material;
					break;
				}

				// We should be able to find a material in mMaterials, otherwise the file should be corrupted.
				if(object.material == nullptr)
					ABORTLOADING();
			}	break;

			case SMOOTH_GROUP:
			{
				if(mModelInfo.empty())
					ABORTLOADING();

				std::vector<uint32_t>& smoothingGroup = mModelInfo.back().smoothingGroup;
				smoothingGroup.resize(mModelInfo.back().index.size() / 3);	// Count of index / 3 = numbers of face

				if(!smoothingGroup.empty())
					mStream->read(&smoothingGroup[0], smoothingGroup.size() * sizeof(uint32_t));
			}	break;

			//------------- TRI_MAPPINGCOORS ------------
			// Description: Vertices list
			// Chunk Length: 1 x unsigned short (number of mapping points)
			//             + 2 x float (mapping coordinates) x (number of mapping points)
			//             + sub chunks
			//-------------------------------------------
			case TEX_VERTS:
			{
				uint16_t count = 0;
				mStream->read(count);

				if(mModelInfo.empty() || currentMeshBuilder != mModelInfo.back().meshBuilder)
					ABORTLOADING();

				currentMeshBuilder->enable(Mesh::TextureCoord0);
				currentMeshBuilder->textureUnit(Mesh::TextureCoord0);
				currentMeshBuilder->textureCoordSize(2);
				size_t coordCount = 0;
				Vec2f* coord = reinterpret_cast<Vec2f*>(currentMeshBuilder->acquireBufferPointer(Mesh::TextureCoord0, &coordCount));

				if(count != coordCount)
					ABORTLOADING();

				mStream->read(coord, sizeof(Vec2f) * count);

				// Open gl flipped the texture vertically
				// Reference: http://www.devolution.com/pipermail/sdl/2002-September/049064.html
				for(size_t i=0; i<count; ++i)
					coord[i].y = 1 - coord[i].y;

				currentMeshBuilder->releaseBufferPointer(coord);
			}	break;

			case MATERIAL:	// Material Start
				currentMaterial = new Max3dsMaterial;
				mMaterials.push_back(currentMaterial);
				break;

			case MAT_NAME:	// Material Name
				if(!currentMaterial) ABORTLOADING();
				readString(currentMaterial->mName);
				break;

			case MAT_AMBIENT:	// Material - Ambient Color
				if(!currentMaterial) ABORTLOADING();
				readColor(currentMaterial->ambient);
				break;

			case MAT_DIFFUSE:	// Material - Diffuse Color
				if(!currentMaterial) ABORTLOADING();
				readColor(currentMaterial->diffuse);
				break;

			case MAT_SPECULAR:	// Material - Spec Color
				if(!currentMaterial) ABORTLOADING();
				readColor(currentMaterial->specular);
				break;

			case SHINY_PERC:	// Material - Shininess (Glossiness)
			{
				if(!currentMaterial) ABORTLOADING();
				uint16_t shininess = Math<int16_t>::clamp(readPercentageAsInt(), 0, 100);
				// Rescale from 0-100 to 0-128 since the maximum accepted value for
				// glMateriali with GL_SHININESS is 128
				currentMaterial->shininess = uint8_t(shininess * 128.f / 100);
			}	break;

			case SHINY_STR_PERC:	// Material - Shine Strength
			{
				if(!currentMaterial) ABORTLOADING();
				uint16_t strength = readPercentageAsInt();
				currentMaterial->specular *= float(strength) / 100;
			}	break;

			case TRANS_PERC:	// Material - Transparency
				readPercentageAsInt();
				break;

			case MAT_TEXMAP:	// Material - Start of Texture Info
				break;

			case MAT_MAPNAME:	// Material - Texture Name
			{
				if(!currentMaterial) ABORTLOADING();
				std::wstring textureFileName;
				readString(textureFileName);

				// We assume the texture is at the same path as the 3ds file itself.
				Path adjustedPath = fileId ? fileId->getBranchPath()/textureFileName : textureFileName;
				if(mResourceManager)
					currentMaterial->texture = dynamic_cast<Texture*>(mResourceManager->load(adjustedPath).get());
				else
					currentMaterial->texture = new Texture(adjustedPath);
			}	break;

			// Skip unknown chunks.
			// We need to skip all the chunks that currently we don't use.
			// We use the chunk length information to set the file pointer to the same level next chunk.
			default:
				mStream->skip(header.length - 6);
		}
	}

	if(mLoadingState == Aborted)
		return mLoadingState;

	// Calculate vertex normal for each mesh
	TangentSpaceBuilder tsBuilder;

	MCD_FOREACH(const ModelInfo& model, mModelInfo) {
		MeshBuilder& meshBuilder = *model.meshBuilder;
		size_t vertexCount, indexCount = model.index.size();

		if(indexCount == 0)
			continue;

		Vec3f* vertex = reinterpret_cast<Vec3f*>(meshBuilder.acquireBufferPointer(Mesh::Position, &vertexCount));
		Vec3f* normal = reinterpret_cast<Vec3f*>(meshBuilder.acquireBufferPointer(Mesh::Normal));

		if(!vertex || !normal)
			continue;

		const uint16_t* index = &model.index[0];

		if(vertex && normal && indexCount > 0) {
			// We have 2 different routines for calculating normals, one with the
			// smoothing group information and one does not.
			if(model.smoothingGroup.empty())
				computeNormal(vertex, normal, index, uint16_t(vertexCount), indexCount);
			else
				computeNormal(vertex, normal, index, &(model.smoothingGroup[0]), uint16_t(vertexCount), indexCount);
		}

#pragma warning(push)
#pragma warning (disable : 6240)
		if((meshBuilder.format() & Mesh::TextureCoord0) && mLoadOptions->includeTangents) {
#pragma warning(pop)
			meshBuilder.enable(Mesh::TextureCoord1);
			meshBuilder.textureUnit(Mesh::TextureCoord1);
			meshBuilder.textureCoordSize(3);

			Vec2f* uv = reinterpret_cast<Vec2f*>(meshBuilder.acquireBufferPointer(Mesh::TextureCoord0));
			Vec3f* tangent = reinterpret_cast<Vec3f*>(meshBuilder.acquireBufferPointer(Mesh::TextureCoord1));

			if(uv && tangent) {
				tsBuilder.compute(indexCount / 3, vertexCount, index, vertex, normal, uv, tangent);

				meshBuilder.releaseBufferPointer(tangent);
				meshBuilder.releaseBufferPointer(uv);
			}
		}

		meshBuilder.releaseBufferPointer(normal);
		meshBuilder.releaseBufferPointer(vertex);
	}

	{	ScopeLock lock(mMutex);
		mLoadingState = Loaded;
	}

	return mLoadingState;
}

void Max3dsLoader::Impl::readColor(ColorRGBf& color)
{
	ChunkHeader header;

	uint8_t uintColor[3];

	mStream->read(header);

	if(header.id != COLOR_TRU)
		mLoadingState = Aborted;

	mStream->read(uintColor, 3);
	color.r = float(uintColor[0]) / 256;
	color.g = float(uintColor[1]) / 256;
	color.b = float(uintColor[2]) / 256;
}

int16_t Max3dsLoader::Impl::readPercentageAsInt()
{
	ChunkHeader header;
	mStream->read(header);

	if(header.id != PERC_INT)
		mLoadingState = Aborted;

	int16_t percentage;
	mStream->read(percentage);

	return Math<int16_t>::clamp(percentage, -100, 100);
}

size_t Max3dsLoader::Impl::readString(std::wstring& str)
{
	size_t i = 0;
	std::string objectName;
	const size_t maxLength = 128;	// Max length to prevent infinite loop
	char c = '\0';
	do {
		mStream->read(c);
		objectName += c;
		++i;
	} while(c != '\0' && i < maxLength);

	str = strToWStr(objectName);

	// Returns how much is read from the stream
	return i;
}

void Max3dsLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because Max3dsLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.

	Model& model = dynamic_cast<Model&>(resource);

	// TODO: Design a way to set this variable from outside
	MeshBuilder::StorageHint storageHint = MeshBuilder::Static;

	MCD_FOREACH(const ModelInfo& modelInfo, mModelInfo)
	{
		// Commit the mesh with the vertex buffer first
		MeshPtr meshWithoutIndex;

		if(mLoadOptions->keepMeshBuilders)
		{
			meshWithoutIndex = new EditableMesh(L"tmp");
			((EditableMesh&)*meshWithoutIndex).builder = modelInfo.meshBuilder;
		}
		else
			meshWithoutIndex = new Mesh(L"tmp");

		modelInfo.meshBuilder->commit(*meshWithoutIndex, storageHint);

		// TODO: Handle empty multiSubObject. That is, those meshes without multi-subobject
		MCD_FOREACH(const MultiSubObject& subObject, modelInfo.multiSubObject)
		{
			// Share all the buffers in meshWithoutIndex into this mesh
			// TODO: Add name to the mesh
			MeshPtr mesh;
			
			if(mLoadOptions->keepMeshBuilders)
				mesh = new EditableMesh(L"", (EditableMesh&)*meshWithoutIndex);
			else
				mesh = new Mesh(L"", *meshWithoutIndex);

			// Let mesh have it's own index buffer
			mesh->setHandlePtr(Mesh::Index, Mesh::HandlePtr(new uint(0)));

			// Setup the index buffer
			MeshBuilder indexBuilder;
			indexBuilder.enable(Mesh::Index);
			size_t faceCount = subObject.mFaceIndex.size();

			if(faceCount > 0 && mLoadOptions->keepMeshBuilders)
				((EditableMesh&)*mesh).builder->enable(Mesh::Index);

			for(size_t i=0; i<faceCount; ++i) {
				const size_t vertexIdx = subObject.mFaceIndex[i] * 3;	// From face index to vertex index
				const uint16_t i1 = modelInfo.index[vertexIdx + 0];
				const uint16_t i2 = modelInfo.index[vertexIdx + 1];
				const uint16_t i3 = modelInfo.index[vertexIdx + 2];
				indexBuilder.addTriangle(i1, i2, i3);

				// make sure the output mesh have indices
				if(mLoadOptions->keepMeshBuilders)
					((EditableMesh&)*mesh).builder->addTriangle(i1, i2, i3);

			}
			indexBuilder.commit(*mesh, Mesh::Index, storageHint);

			// Assign material
			subObject.material->init();

			Model::MeshAndMaterial* meshMat = new Model::MeshAndMaterial;
			meshMat->mesh = mesh;
			meshMat->effect = new Effect(L"");
			meshMat->effect->material.reset(subObject.material->clone());

			model.mMeshes.pushBack(*meshMat);
		}
	}

	// remember to reset the LoadOptions
	mLoadOptions.reset();
}

IResourceLoader::LoadingState Max3dsLoader::Impl::getLoadingState() const
{
	ScopeLock lock(mMutex);
	return mLoadingState;
}

Max3dsLoader::Max3dsLoader(IResourceManager* resourceManager)
{
	mImpl = new Impl(resourceManager);
}

Max3dsLoader::~Max3dsLoader()
{
	delete mImpl;
}

IResourceLoader::LoadingState Max3dsLoader::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	MemoryProfiler::Scope scope("Max3dsLoader::load");
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->load(is, fileId, args);
}

void Max3dsLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("Max3dsLoader::commit");
	MCD_ASSUME(mImpl != nullptr);
	mImpl->commit(resource);
}

IResourceLoader::LoadingState Max3dsLoader::getLoadingState() const
{
	MCD_ASSUME(mImpl != nullptr);
	return mImpl->getLoadingState();
}

}	// namespace MCD
