#include "Pch.h"
#include "Max3dsLoader.h"
#include "../Render/Effect.h"
#include "../Render/Material.h"
#include "../Render/MeshBuilder.h"
#include "../Render/MeshBuilderUtility.h"
#include "../Render/Mesh.h"
#include "../Render/Model.h"
#include "../Render/Texture.h"
#include "../Render/TangentSpaceBuilder.h"
#include "../Core/Math/Mat44.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/Stream.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"
#include <limits>	// For numeric_limits
#include <map>
#include <memory>	// For auto_ptr
#include <set>

// TODO: Remove the dependency on graphics API
#include "../../3Party/glew/glew.h"

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
	std::string mName;

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

		if(texture)
			addProperty(new TextureProperty(texture.get(), 0, GL_LINEAR, GL_LINEAR), 0);

		inited = true;
	}
};	// Max3dsMaterial

/*!	This stream class provides some add on functionality over the std::istream,
	and provide more error checking.
 */
class MyStream
{
public:
	MyStream(std::istream& is, volatile IResourceLoader::LoadingState& loadingState)
		: mIs(is), mLoadingState(loadingState)
	{}

	template<typename T>
	void read(T& t)
	{
		mIs.read((char*)&t, sizeof(t));
		if(mIs.gcount() != sizeof(t))
			mLoadingState = IResourceLoader::Aborted;
	}

	template<typename T>
	void read(StrideArray<T>& t)
	{
		if(t.size == t.stride)
			read(t.data, t.sizeInByte());
		else {
			for(size_t i=0; i<t.size; ++i)
				read(t[i]);
		}
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
		(void)MCD::read(mIs, header.id);
		(void)MCD::read(mIs, header.length);

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
};	// MyStream

}	// namespace

class Max3dsLoader::Impl
{
public:
	Impl(IResourceManager* resourceManager);

	~Impl();

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const;

	void readColor(ColorRGBf& color);

	int16_t readPercentageAsInt();

	size_t readString(std::string& str);

private:
	//! Read all 3DS chunk data and store those information into mModelInfo
	void readChunks(const Path* fileId);

	//! Process the 3DS Max friendy format into real-time API friendly format
	void postProcess();

private:
	MyStream* mStream;
	IResourceManager* mResourceManager;

	struct LoadOptions
	{
		bool includeTangents;
	};	// LoadOptions

	std::auto_ptr<LoadOptions> mLoadOptions;

	//! Represent which face the material is assigned to.
	struct MultiSubObject
	{
		Max3dsMaterial* material;
		std::vector<uint16_t> mFaceIndex;	//!< Index to the face
		std::vector<uint16_t> mIndexIndex;	//!< Transform mFaceIndex to vertex index
		MeshBuilderPtr splittedBuilder;		//!< Assigned from the ModelInfo, after splitting the main builder into smaller one for each material
	};	// MultiSubObject

	struct ModelInfo
	{
		std::string name;
		MeshBuilderPtr meshBuilder;			//!< Contains vertex buffer only
		std::vector<uint32_t> smoothingGroup;	//!< Which smoothing group the face belongs to
		std::list<MultiSubObject> multiSubObject;
	};	// ModelInfo

	std::list<ModelInfo> mModelInfo;

	typedef std::list<Max3dsMaterial*> MaterialList;
	MaterialList mMaterials;

	Max3dsMaterial mDefaultMaterial;

	volatile IResourceLoader::LoadingState mLoadingState;
	mutable Mutex mMutex;
};	// Impl

static const int cIndexAttId = 0, cPositionAttId = 1, cNormalAttId = 2;

Max3dsLoader::Impl::Impl(IResourceManager* resourceManager)
	: mStream(nullptr), mResourceManager(resourceManager), mLoadingState(NotLoaded)
{
	mDefaultMaterial.ambient = ColorRGBf(0.5f);
	mDefaultMaterial.diffuse = ColorRGBf(1);
	mDefaultMaterial.specular = ColorRGBf(0.5f);
	mDefaultMaterial.shininess = 10;
}

Max3dsLoader::Impl::~Impl()
{
	MCD_FOREACH(Max3dsMaterial* material, mMaterials)
		delete material;
	delete mStream;
}

#define ABORTLOADING() { mLoadingState = Aborted; break; }

void Max3dsLoader::Impl::readChunks(const Path* fileId)
{
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
			{
				ModelInfo modelInfo;
				readString(modelInfo.name);

				currentMeshBuilder = new MeshBuilder(false);
				if(!currentMeshBuilder)
					ABORTLOADING();

				modelInfo.meshBuilder = currentMeshBuilder;
				mModelInfo.push_back(modelInfo);
				currentMeshBuilder->declareAttribute(VertexFormat::get("position"), 1);
				currentMeshBuilder->declareAttribute(VertexFormat::get("normal"), 1);
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

				if(!currentMeshBuilder->resizeBuffers(vertexCount, currentMeshBuilder->indexCount()))
					ABORTLOADING();

				StrideArray<Vec3f> posArray = currentMeshBuilder->getAttributeAs<Vec3f>(cPositionAttId);

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

					posArray[i] = v;
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

					StrideArray<Vec3f> vertex = currentMeshBuilder->getAttributeAs<Vec3f>(cPositionAttId);

					for(size_t i=0; i<vertex.size; ++i) {
						Vec4f tmp(vertex[i].x, -vertex[i].z, vertex[i].y, 0);
						// TODO: Use matrix.transform once the function is available
						tmp = (matrix * tmp) + matrix[3];
						vertex[i] = Vec3f(tmp.x, tmp.z, -tmp.y);
					}
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

				if(!currentMeshBuilder->resizeIndexBuffer(faceCount * 3))	// Each triangle has 3 vertex
					ABORTLOADING();

				StrideArray<uint16_t> idx = currentMeshBuilder->getAttributeAs<uint16_t>(cIndexAttId);

				for(size_t i=0; i<faceCount; ++i) {
					// Read 3 indexes at once
					mStream->read(&idx[i * 3], 3 * sizeof(uint16_t));
					uint16_t faceFlags;	// Flag that stores some face information (currently not used)
					mStream->read(faceFlags);
				}

				if(invertWinding) {
					for(size_t i=0; i<idx.size; i+=3)
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

				std::string materialName;
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
				smoothingGroup.resize(currentMeshBuilder->indexCount() / 3);	// Count of index / 3 = numbers of face

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

				int uvId = currentMeshBuilder->declareAttribute(VertexFormat::get("uv0"), 1);
				StrideArray<Vec2f> coord = currentMeshBuilder->getAttributeAs<Vec2f>(uvId);

				if(count != coord.size)
					ABORTLOADING();

				mStream->read(coord);

				// Open gl flipped the texture vertically
				// Reference: http://www.devolution.com/pipermail/sdl/2002-September/049064.html
				for(size_t i=0; i<count; ++i)
					coord[i].y = 1 - coord[i].y;
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
				std::string textureFileName;
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
}

#undef ABORTLOADING

//!	Generate necessary vertex to ensure each vertex will only rest in one smoothing group.
static sal_checkreturn bool splitSmoothingGroupVertex(MeshBuilder& builder, const std::vector<uint32_t>& smoothingGroups)
{
	if(smoothingGroups.empty())
		return true;

	StrideArray<Vec3f> vertex = builder.getAttributeAs<Vec3f>(cPositionAttId);
	StrideArray<uint16_t> idx = builder.getAttributeAs<uint16_t>(cIndexAttId);
	const uint16_t faceCount = uint16_t(idx.size / 3);

	// A map to remember which "index-smoothing group pair" is already processed.
	typedef std::map<uint64_t, uint16_t> NerVertexMap;
	NerVertexMap newVertex;

	// Create a smoothing group to face info
	MCD_ASSERT(smoothingGroups.size() == faceCount);
	std::vector<uint16_t> g2f[32];
	for(uint16_t i=0; i<faceCount; ++i) {
		uint32_t sg = smoothingGroups[i];
		for(size_t j=0; j<32; ++j) {
			if(sg & (1 << j))
				g2f[j].push_back(i);
		}
	}

	// Create a vertex to smoothing group info
	std::vector<uint32_t> v2g(vertex.size, 0);
	for(size_t i=0; i<idx.size; ++i)
		v2g[idx[i]] |= smoothingGroups[i/3];

	// Loop for all vertex
	const size_t startingVertexCount = vertex.size;	// We will append vertex, so backup the current count first.
	for(size_t i=0; i<startingVertexCount; ++i)
	{
		uint32_t sg = v2g[i];

		// If the vertex has only one smoothing group, great! nothing more need to do.
		if(isPowerOf2(sg))
			continue;

		// Foreach smoothing group
		bool firstGroupIgnored = false;
		for(size_t j=0; j<32; ++j) {
			if(!(sg & (1 << j)))
				continue;
			if(!firstGroupIgnored) {
				firstGroupIgnored = true;
				continue;
			}

			// Scan which face index in this smoothing group use this vertex
			for(size_t k=0; k<g2f[j].size(); ++k) {
				const uint16_t faceIdx = g2f[j][k];

				// Scan for each vertex in the j'th smoothing group's k'th face.
				for(uint16_t l=0; l<3; ++l) {
					const uint16_t indexIndex = faceIdx * 3 + l;
					const uint16_t vidx = idx[indexIndex];
					if(vidx != i)
						continue;

					uint64_t key = (uint64_t(vidx) << 32) + sg;
					NerVertexMap::const_iterator itr = newVertex.find(key);
					if(itr != newVertex.end()) {
						idx[indexIndex] = itr->second;
						continue;
					}

					uint16_t srcIdx = vidx;
					uint16_t destIdx = uint16_t(vertex.size);
					newVertex[key] = destIdx;
					idx[indexIndex] = destIdx;

					// Check that adding more vertex will not causing uint16_t overflow
					if(vertex.size + 1 >= std::numeric_limits<uint16_t>::max())
						return false;

					if(!builder.resizeVertexBuffer(uint16_t(vertex.size + 1)))
						return false;

					// Remember to re-assign "vertex" pointer after the builder has resized.
					vertex = builder.getAttributeAs<Vec3f>(cPositionAttId);

					// Perform the vertex data copying.
					if(!MeshBuilderUtility::copyVertexAttributes(
						builder, builder,
						FixStrideArray<uint16_t>(&srcIdx,1), FixStrideArray<uint16_t>(&destIdx,1)
					))
						return false;
				}
			}
		}
	}

	return true;
}

// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=504353&whichpage=1&#3290286
static void computNormal(MeshBuilder& builder, const std::vector<uint32_t>& smoothingGroups)
{
	StrideArray<Vec3f> vertex = builder.getAttributeAs<Vec3f>(cPositionAttId);
	StrideArray<uint16_t> index = builder.getAttributeAs<uint16_t>(cIndexAttId);
	StrideArray<Vec3f> normal = builder.getAttributeAs<Vec3f>(cNormalAttId);

	struct Comparator
	{
		bool operator()(const Vec3f& lhs, const Vec3f& rhs) const {
			return memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
		}
	};

	// Map between vertex to (multiple) vertex index's index.
	// This map aims to find out distinct vertex that having the same position.
	typedef std::map<Vec3f, std::set<uint16_t>, Comparator> Position2Faces;
	Position2Faces position2Faces;

	for(uint16_t i=0; i<index.size; ++i)
		position2Faces[vertex[index[i]]].insert(i);

	// Initialize the normal to zero first
	for(size_t i=0; i<vertex.size; ++i)
		normal[i] = Vec3f::cZero;

	if(smoothingGroups.size() == index.size / 3) {
		// Calculate the face normal for each face
		for(size_t i=0; i<index.size; i+=3) {
			const Vec3f& v1 = vertex[index[i+0]];
			const Vec3f& v2 = vertex[index[i+1]];
			const Vec3f& v3 = vertex[index[i+2]];

			// We need not to normalize this faceNormal, since a vertex's normal
			// should be influenced more by a larger polygon.
			const Vec3f faceNormal = (v3 - v2) ^ (v1 - v2);

			// The smoothing group for this face
			int sg = smoothingGroups[i/3];

			for(size_t j=0; j<3; ++j) {
				MCD_FOREACH(uint16_t f, position2Faces[vertex[index[i+j]]]) {
					if(sg & smoothingGroups[f/3])
						normal[index[f]] += faceNormal;
				}
			}
		}
	}
	else {
		for(size_t i=0; i<index.size; i+=3) {
			const Vec3f& v1 = vertex[index[i+0]];
			const Vec3f& v2 = vertex[index[i+1]];
			const Vec3f& v3 = vertex[index[i+2]];

			// We need not to normalize this faceNormal, since a vertex's normal
			// should be influenced more by a larger polygon.
			const Vec3f faceNormal = (v3 - v2) ^ (v1 - v2);

			for(size_t j=0; j<3; ++j) {
				MCD_FOREACH(uint16_t f, position2Faces[vertex[index[i+j]]])
					normal[index[f]] += faceNormal;
			}
		}
	}

	// Normalize for each vertex normal
	for(size_t i=0; i<vertex.size; ++i)
		normal[i].normalize();
}

void Max3dsLoader::Impl::postProcess()
{
	MCD_FOREACH(const ModelInfo& _model, mModelInfo)
	{
		ModelInfo& model = const_cast<ModelInfo&>(_model);
		MeshBuilder& meshBuilder = *model.meshBuilder;

		// Compute tangent space if needed
		int uvId = meshBuilder.findAttributeId("uv0");
		if(mLoadOptions->includeTangents && uvId != -1)
		{
			TangentSpaceBuilder tsBuilder;
			int tanId = meshBuilder.declareAttribute(VertexFormat::get("tangent"), 2);
			MCD_VERIFY(tsBuilder.compute(meshBuilder, cIndexAttId, cPositionAttId, cNormalAttId, uvId, tanId));
		}

		if(!splitSmoothingGroupVertex(meshBuilder, model.smoothingGroup)) {
			Log::write(Log::Warn, "Failed to split smoothing group, mesh skipped");
			continue;
		}

		// We don't use MeshBuilderUtility::computNormal because there may have
		// multiple vertex that are having the same vertex position (UV degenerated vertex).
//		MeshBuilderUtility::computNormal(meshBuilder, 2);
		computNormal(meshBuilder, model.smoothingGroup);

		if(model.multiSubObject.size() == 0) {	// The model has no multi subobject
			MultiSubObject subObject;
			subObject.splittedBuilder = model.meshBuilder;
			subObject.material = &mDefaultMaterial;
			model.multiSubObject.push_back(subObject);
		}
		else if(model.multiSubObject.size() == 1) {	// Single material, no need to split the mesh.
			model.multiSubObject.begin()->splittedBuilder = model.meshBuilder;
		}
		else
		{
			MeshBuilder** subBuilders = (MeshBuilder**)MCD_STACKALLOCA(sizeof(MeshBuilder*) * model.multiSubObject.size());
			StrideArray<uint16_t>* faceIndices = (StrideArray<uint16_t>*)MCD_STACKALLOCA(sizeof(StrideArray<uint16_t>) * model.multiSubObject.size());

			size_t i = 0;
			MCD_FOREACH(const MultiSubObject& _subObject, model.multiSubObject)
			{
				MultiSubObject& subObject = const_cast<MultiSubObject&>(_subObject);
				subBuilders[i] = new MeshBuilder(false);
				subObject.splittedBuilder = subBuilders[i];

				StrideArray<uint16_t> idx = meshBuilder.getAttributeAs<uint16_t>(cIndexAttId);
				const size_t faceCount = subObject.mFaceIndex.size();

				std::vector<uint16_t>& indexIndex = subObject.mIndexIndex;
				indexIndex.resize(faceCount * 3);
				for(size_t j=0; j<faceCount; ++j) {
					const size_t fIdx = subObject.mFaceIndex[j] * 3;
					indexIndex[j*3 + 0] = idx[fIdx + 0];
					indexIndex[j*3 + 1] = idx[fIdx + 1];
					indexIndex[j*3 + 2] = idx[fIdx + 2];
				}

				faceIndices[i] = StrideArray<uint16_t>((uint16_t*)&indexIndex[0], indexIndex.size());
				++i;
			}

			MeshBuilderUtility::split(model.multiSubObject.size(), meshBuilder, subBuilders, faceIndices);

			MCD_STACKFREE(subBuilders);
			MCD_STACKFREE(faceIndices);
		}
	}
}

IResourceLoader::LoadingState Max3dsLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	using namespace std;

	// Parse the load options from args
	if(mLoadOptions.get() == nullptr)
	{
		mLoadOptions.reset(new LoadOptions);
		mLoadOptions->includeTangents = false;

		if(nullptr != args)
		{
			NvpParser parser(args);
			const char* name, *value;
			while(parser.next(name, value))
			{
				if(strCaseCmp(name, "tangents") == 0 && strCaseCmp(value, "true") == 0)
					mLoadOptions->includeTangents = true;
			}
		}
	}

	{	ScopeLock lock(mMutex);
		mLoadingState = is ? NotLoaded : Aborted;
	}

	if(mLoadingState & Stopped)
		return mLoadingState;

	if(!mStream)
		mStream = new MyStream(*is, mLoadingState);

	readChunks(fileId);

	if(mLoadingState == Aborted)
		return mLoadingState;

	postProcess();

	if(mLoadingState == Aborted)
		return mLoadingState;

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

size_t Max3dsLoader::Impl::readString(std::string& str)
{
	size_t i = 0;
	str.clear();
	const size_t maxLength = 128;	// Max length to prevent infinite loop
	char c = '\0';
	do {
		mStream->read(c);
		str += c;
		++i;
	} while(c != '\0' && i < maxLength);

	// Returns how much is read from the stream
	return i;
}

void Max3dsLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because Max3dsLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.

	Model& model = dynamic_cast<Model&>(resource);

	// TODO: Design a way to set this variable from outside
	Mesh::StorageHint storageHint = Mesh::Static;

	MCD_FOREACH(const ModelInfo& modelInfo, mModelInfo)
	{
		MCD_FOREACH(const MultiSubObject& subObject, modelInfo.multiSubObject)
		{
			if(!subObject.splittedBuilder ||
				subObject.splittedBuilder->vertexCount() == 0 ||
				subObject.splittedBuilder->indexCount() == 0
			)
				continue;

			MeshPtr mesh = new Mesh("");
			if(!mesh->create(*subObject.splittedBuilder, storageHint)) {
				Log::write(Log::Warn, "Failed to commit mesh");
				continue;
			}

			// Assign material
			subObject.material->init();

			std::auto_ptr<Model::MeshAndMaterial> meshMat(new Model::MeshAndMaterial);
			meshMat->mesh = mesh;
			meshMat->effect = new Effect("");
			meshMat->effect->material.reset(subObject.material->clone());
			meshMat->name = modelInfo.name;

			model.mMeshes.pushBack(*meshMat);
			meshMat.release();
		}
	}

	// Remember to reset the LoadOptions
	mLoadOptions.reset();
}

IResourceLoader::LoadingState Max3dsLoader::Impl::getLoadingState() const
{
	ScopeLock lock(mMutex);
	return mLoadingState;
}

Max3dsLoader::Max3dsLoader(IResourceManager* resourceManager)
	: mImpl(*new Impl(resourceManager))
{
}

Max3dsLoader::~Max3dsLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState Max3dsLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("Max3dsLoader::load");
	return mImpl.load(is, fileId, args);
}

void Max3dsLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("Max3dsLoader::commit");
	mImpl.commit(resource);
}

IResourceLoader::LoadingState Max3dsLoader::getLoadingState() const
{
	return mImpl.getLoadingState();
}

}	// namespace MCD
