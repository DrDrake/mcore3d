#include "Pch.h"
#include "OgreMeshLoader.h"
#include "Effect.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshBuilder.h"
#include "Model.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/Stream.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"	// for FOR_EACH
#include <limits>
#include <string.h>	// for strcmp

namespace MCD {

namespace {

/*!	Named enums for the chunk id.
	The indentation shows the parent/child relationship between the chunks.
	Reference: Ogre source: OgreMeshFileFormat.h
 */
enum ChunkID
{
	M_HEADER									= 0x1000,
	M_MESH										= 0x3000,
		M_SUBMESH								= 0x4000,
			M_SUBMESH_OPERATION					= 0x4010,
			M_SUBMESH_BONE_ASSIGNMENT			= 0x4100,
			M_SUBMESH_TEXTURE_ALIAS				= 0x4200,
		M_GEOMETRY								= 0x5000,	// This chunk may embedded within both M_MESH and M_SUBMESH
			M_GEOMETRY_VERTEX_DECLARATION		= 0x5100,
				M_GEOMETRY_VERTEX_ELEMENT		= 0x5110,	// Repeating section
			M_GEOMETRY_VERTEX_BUFFER			= 0x5200,	// Repeating section
				M_GEOMETRY_VERTEX_BUFFER_DATA	= 0x5210,
		M_MESH_SKELETON_LINK					= 0x6000,
		M_MESH_BONE_ASSIGNMENT					= 0x7000,
		M_MESH_BOUNDS							= 0x9000,
		M_SUBMESH_NAME_TABLE					= 0xA000,
			M_SUBMESH_NAME_TABLE_ELEMENT		= 0xA100,
};	// ChunkID

//!	Vertex element semantics defined in Ogre's OgreHardwareVertexBuffer.h
enum VertexElementSemantic
{
	VES_POSITION = 1,	//!< 3 reals per vertex
	VES_BLEND_WEIGHTS = 2,
	VES_BLEND_INDICES = 3,
	VES_NORMAL = 4,		//!< 3 reals per vertex
	VES_DIFFUSE = 5,
	VES_SPECULAR = 6,
	VES_TEXTURE_COORDINATES = 7,
	VES_BINORMAL = 8,	//!< Binormal (Y axis if normal is Z)
	VES_TANGENT = 9		//!< Tangent (X axis if normal is Z)
};	// VertexElementSemantic

//! Vertex element type defined in Ogre's OgreHardwareVertexBuffer.h
enum VertexElementType
{
	VET_FLOAT1 = 0,
	VET_FLOAT2 = 1,
	VET_FLOAT3 = 2,
	VET_FLOAT4 = 3,
	VET_COLOUR = 4,
	VET_SHORT1 = 5,
	VET_SHORT2 = 6,
	VET_SHORT3 = 7,
	VET_SHORT4 = 8,
	VET_UBYTE4 = 9,
	VET_COLOUR_ARGB = 10,
	VET_COLOUR_ABGR = 11,
	VET_UINT1 = 12
};	// VertexElementType

struct ChunkHeader
{
	uint16_t id;
	uint32_t length;

	static const size_t cSize = 6;

	bool readFrom(std::istream& is)
	{
		id = 0;
		length = 0;

		is.read((char*)&id, sizeof(id));
		is.read((char*)&length, sizeof(length));

		return id != 0 && length != 0;
	}
};	// ChunkHeader

// Following are some data structures that minics the mesh data structure in Ogre,
// once these structure are filled with data, we can transform them into our mesh format.

struct VertexDeclaration
{
	uint16_t source;	//!< Buffer bind source	(vertex buffer index)
	uint16_t type;		//!< VertexElementType (int, float, float3, ...)
	uint16_t semantic;	//!< VertexElementSemantic	(position, normal, ...)
	uint16_t offset;	//!< Start offset in buffer in bytes (byte offset in a single vertex)
	uint16_t index;		//!< Index of the semantic (for colours and texture coords)

	static const size_t cSize = sizeof(uint16_t) * 5;
};	// VertexDeclaration

size_t getChannelCount(const VertexDeclaration& d)
{
	switch(d.type) {
	case VET_FLOAT1: case VET_SHORT1: return 1;
	case VET_FLOAT2: case VET_SHORT2: return 2;
	case VET_FLOAT3: case VET_SHORT3: return 3;
	case VET_FLOAT4: case VET_SHORT4: return 4;
	case VET_COLOUR_ARGB: case VET_COLOUR_ABGR: return 4;
	default: return 0;
	}
}

VertexFormat getSemantic(const VertexDeclaration& d)
{
	switch(d.semantic) {
	case VES_POSITION:				return (d.type == VET_FLOAT3) ? VertexFormat::get("position") : VertexFormat::null();
	case VES_BLEND_WEIGHTS:			return VertexFormat::get("jointWeight");	// TODO: Check that map.blendWeight() return the expected elementSize
	case VES_BLEND_INDICES:			return VertexFormat::get("jointIndex");	// TODO: Check that map.blendIndex() return the expected elementSize
	case VES_NORMAL:				return (d.type == VET_FLOAT3) ? VertexFormat::get("position") : VertexFormat::null();
	case VES_DIFFUSE:				return VertexFormat::get("color0");
	case VES_SPECULAR:				return VertexFormat::get("color1");
	case VES_TEXTURE_COORDINATES:	return VertexFormat::get("uv0");
	case VES_BINORMAL:				return VertexFormat::get("binormal");
	case VES_TANGENT:				return VertexFormat::get("tangent");
	default:						return VertexFormat::null();
	}
}

class Geometry
{
public:
	Geometry(size_t indexCount)
	{
		meshBuilder = new MeshBuilder(false);
		(void)meshBuilder->resizeBuffers(0, indexCount);
	}

	std::string name;
	std::string materialName;
	ResourcePtr material;
	MeshBuilderPtr meshBuilder;
};	// Geometry

sal_checkreturn bool myReadString(std::istream& is, char* buf, size_t bufLen) {
	return MCD::readString(is, buf, bufLen, '\n') > 0;
}

sal_checkreturn bool readUInt16Array(std::istream& is, uint16_t* result, size_t count)
{
	is.read((char*)result, sizeof(*result) * count);
	return is.gcount() == std::streamsize(sizeof(*result) * count);
}

}	// namespace

class OgreMeshLoader::Impl
{
public:
	Impl(IResourceManager* resourceManager)
		: mResourceManager(resourceManager)
		, mVersionHeaderLoaded(false)
		, mCurrentGeometryIdx(-1)
		, mFileId(nullptr)
	{
	}

	bool loadVersionHeader(std::istream& is)
	{
		uint16_t id = 0;
		is.read((char*)&id, sizeof(id));

		if(id != M_HEADER)
			return false;

		char buf[256];
		return mVersionHeaderLoaded = myReadString(is, buf, sizeof(buf));
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const
	{
		return mLoadingState;
	}

private:
	IResourceManager* mResourceManager;
	bool mVersionHeaderLoaded;

	volatile IResourceLoader::LoadingState mLoadingState;

	ptr_vector<Geometry> mGeometry;
	int mCurrentGeometryIdx;	// Can be negative

	const Path* mFileId;
};	// Impl

IResourceLoader::LoadingState OgreMeshLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	// Simplying the error check
	#define ABORT_IF(expression) if(expression) { MCD_ASSERT(false); return mLoadingState = Aborted; }

	ABORT_IF(!is);
	mFileId = fileId;

	if(mLoadingState != Loading)
		mLoadingState = NotLoaded;

	if(mLoadingState & Stopped)
		return mLoadingState;

	if(!mVersionHeaderLoaded) {
		ABORT_IF(!loadVersionHeader(*is));
		mLoadingState = Loading;
	}

	ChunkHeader header;
	if(!header.readFrom(*is))
		return mLoadingState = is->eof() ? Loaded : Aborted;

	switch(header.id)
	{
	case M_MESH:
	{	// Skip the bool
		is->seekg(1, std::ios_base::cur);
	}	break;

	case M_SUBMESH:
	{
		char materialName[256];
		ABORT_IF(!myReadString(*is, materialName, sizeof(materialName)));

		bool useSharedVertices;
		ABORT_IF(!read(*is, useSharedVertices));

		// TODO: Handle shared vertices between sub-meshes
		ABORT_IF(useSharedVertices);

		// TODO: Check the vertex count will not overflow according to indexes32Bit
		uint32_t indexCount;
		is->read((char*)&indexCount, sizeof(indexCount));

		// TODO: Handle 32-bit index
		bool indexes32Bit;
		ABORT_IF(!read(*is, indexes32Bit));

		Geometry* geo = new Geometry(indexCount);
		geo->materialName = materialName;

		{	// Load material, assume the material is at the same path as the .mesh file itself.
			Path adjustedPath = mFileId ? mFileId->getBranchPath() / materialName : materialName;
			geo->material = mResourceManager->load(adjustedPath);
		}

		char* p = geo->meshBuilder->getBufferPointer(0);
		ABORT_IF(!p || !readUInt16Array(*is, (uint16_t*)p, indexCount));

		mGeometry.push_back(geo);
		++mCurrentGeometryIdx;
	}	break;

	case M_GEOMETRY:
	{
		uint32_t vertexCount;
		is->read((char*)&vertexCount, sizeof(vertexCount));

		ABORT_IF(vertexCount >= size_t(std::numeric_limits<uint16_t>::max()));

		MeshBuilder& builder = *mGeometry[mCurrentGeometryIdx].meshBuilder;
		ABORT_IF(!builder.resizeBuffers(uint16_t(vertexCount), builder.indexCount()));
	}	break;

	case M_GEOMETRY_VERTEX_DECLARATION:
		break;

	case M_GEOMETRY_VERTEX_ELEMENT:
	{
		Geometry& geo = mGeometry[mCurrentGeometryIdx];

		VertexDeclaration decl;
		is->read((char*)&decl, VertexDeclaration::cSize);

		geo.meshBuilder->declareAttribute(
			getSemantic(decl),
			decl.source + 1	// One buffer index is already reserved for index buffer
		);

	}	break;

	case M_GEOMETRY_VERTEX_BUFFER:
	{
		Geometry& geo = mGeometry[mCurrentGeometryIdx];

		uint16_t bindIndex;
		ABORT_IF(!readUInt16Array(*is, &bindIndex, 1));

		// TODO: Vertex the vertexSize is valid according to geo.VertexDeclarations
		uint16_t vertexSize;
		ABORT_IF(!readUInt16Array(*is, &vertexSize, 1));

		// Read the chunk M_GEOMETRY_VERTEX_BUFFER_DATA
		ABORT_IF(!header.readFrom(*is) || header.id != M_GEOMETRY_VERTEX_BUFFER_DATA);

		size_t sizeInByte;
		char* buf = geo.meshBuilder->getBufferPointer(bindIndex + 1, nullptr, &sizeInByte);
		ABORT_IF(size_t(vertexSize) * geo.meshBuilder->vertexCount() != sizeInByte);
		is->read(buf, sizeInByte);
	}	break;

	case M_GEOMETRY_VERTEX_BUFFER_DATA:
	{
		ABORT_IF("M_GEOMETRY_VERTEX_BUFFER_DATA should be already processed in M_GEOMETRY_VERTEX_BUFFER");
	}	break;

	case M_SUBMESH_NAME_TABLE:
		break;

	// An optional list of name for the meshes
	case M_SUBMESH_NAME_TABLE_ELEMENT:
	{
		uint16_t idx;
		ABORT_IF(!readUInt16Array(*is, &idx, 1));

		char subMeshName[256];
		ABORT_IF(!myReadString(*is, subMeshName, sizeof(subMeshName)));

		if(idx < mGeometry.size())
			mGeometry[idx].name = subMeshName;
	}	break;

	// Skip uninterested chunks
	default:
		is->seekg(header.length - ChunkHeader::cSize, std::ios_base::cur);
	}

	return mLoadingState;

	#undef ABORT_IF
}

//! A default white material
class WhiteMaterial : public Material
{
public:
	WhiteMaterial(const TexturePtr& texture=nullptr)
	{
		this->mRenderPasses.push_back(new Pass);
		addProperty(
			new StandardProperty(
				ColorRGBAf(1.0f, 1.0f)
				, ColorRGBAf(1.0f, 1.0f)
				, ColorRGBAf(1.0f, 1.0f)
				, ColorProperty::ColorOperation::Replace
				, 100.0f)
			, 0
		);

		if(texture)
			addProperty(new TextureProperty(texture.get(), 0, GL_LINEAR, GL_LINEAR), 0);
	}
};	// WhiteMaterial

void OgreMeshLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because OgreMeshLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.

	Model& model = dynamic_cast<Model&>(resource);

	MCD_FOREACH(const Geometry& geo, mGeometry)
	{
		MeshPtr mesh = new Mesh(geo.name);

		if(!mesh->create(*geo.meshBuilder, Mesh::Static))
			Log::write(Log::Warn, "Failed to commit mesh");

		std::auto_ptr<Model::MeshAndMaterial> meshMat(new Model::MeshAndMaterial);
		meshMat->mesh = mesh;
		meshMat->effect = new Effect("");
		meshMat->name = geo.name;
		meshMat->meshBuilder = geo.meshBuilder;	// TODO: Parse args to decide conserve mesh builder or not.

		// Use the default white material if none can load
		if(!geo.material)
			meshMat->effect->material.reset(new WhiteMaterial);
		else
		{
			if(Texture* texture = dynamic_cast<Texture*>(geo.material.get()))
				meshMat->effect->material.reset(new WhiteMaterial(texture));
			if(Effect* effect = dynamic_cast<Effect*>(geo.material.get()))
				meshMat->effect = effect;
		}

		model.mMeshes.pushBack(*meshMat);
		meshMat.release();
	}
}

OgreMeshLoader::OgreMeshLoader(IResourceManager* resourceManager)
	: mImpl(*new Impl(resourceManager))
{
}

OgreMeshLoader::~OgreMeshLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState OgreMeshLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("OgreMeshLoader::load");
	return mImpl.load(is, fileId, args);
}

void OgreMeshLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("OgreMeshLoader::commit");
	return mImpl.commit(resource);
}

IResourceLoader::LoadingState OgreMeshLoader::getLoadingState() const
{
	return mImpl.getLoadingState();
}

}	// namespace MCD
