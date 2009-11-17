#include "Pch.h"
#include "OgreMeshLoader.h"
#include "Effect.h"
#include "Material.h"
#include "Mesh.h"
#include "MeshBuilder.h"
#include "Model.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"	// for FOR_EACH

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

struct RawVertexBuffer
{
	RawVertexBuffer() : data(nullptr), vertexSize(0) {}
	char* data;
	size_t vertexSize;	//!< Size of a single vertex in byte
};	// RawVertexBuffer

struct Geometry
{
	Geometry() { vertexCount = 0; }

	~Geometry()
	{
		for(size_t i=rawVertexBuffers.size(); i--;)
			delete[] rawVertexBuffers[i].data;
	}

	size_t vertexCount;
	std::vector<VertexDeclaration> VertexDeclarations;
	std::vector<RawVertexBuffer> rawVertexBuffers;
};	// Geometry

// Assuming each ModelInfo owns an unique Geometry.
struct ModelInfo : public Geometry
{
	ModelInfo(size_t indexCount_)
	{
		indexBuffer = new uint16_t[indexCount_];
		indexCount = indexCount_;
	}

	~ModelInfo() { delete[] indexBuffer; }

	std::wstring name;
	std::wstring materialName;
	ResourcePtr material;
	uint16_t* indexBuffer;
	size_t indexCount;
};	// ModelInfo

sal_checkreturn bool readString(std::istream& is, char* buf, size_t bufLen)
{
	is.getline(buf, bufLen, '\n');
	size_t readCnt = is.gcount();
	return readCnt > 0 && readCnt < bufLen;
}

sal_checkreturn bool readBool(std::istream& is, bool& result)
{
	uint8_t tmp;
	is.read((char*)&tmp, 1);
	result = tmp > 0;
	return is.gcount() == 1;
}

sal_checkreturn bool readUInt32Array(std::istream& is, uint32_t* result, size_t count)
{
	is.read((char*)result, sizeof(*result) * count);
	return is.gcount() == std::streamsize(sizeof(*result) * count);
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
		, mCurrentModelInfoIdx(-1)
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
		return mVersionHeaderLoaded = readString(is, buf, sizeof(buf));
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const wchar_t* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const
	{
		ScopeLock lock(mMutex);
		return mLoadingState;
	}

private:
	IResourceManager* mResourceManager;
	bool mVersionHeaderLoaded;

	volatile IResourceLoader::LoadingState mLoadingState;
	mutable Mutex mMutex;

	ptr_vector<ModelInfo> mModelInfo;
	int mCurrentModelInfoIdx;	// Can be negative

	const Path* mFileId;
};	// Impl

IResourceLoader::LoadingState OgreMeshLoader::Impl::load(std::istream* is, const Path* fileId, const wchar_t* args)
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
		ABORT_IF(!readString(*is, materialName, sizeof(materialName)));

		bool useSharedVertices;
		ABORT_IF(!readBool(*is, useSharedVertices));

		// TODO: Handle shared vertices between sub-meshes
		ABORT_IF(useSharedVertices);

		// TODO: Check the vertex count will not overflow according to indexes32Bit
        uint32_t indexCount;
		is->read((char*)&indexCount, sizeof(indexCount));

		// TODO: Handle 32-bit index
        bool indexes32Bit;
		ABORT_IF(!readBool(*is, indexes32Bit));

		ModelInfo* info = new ModelInfo(indexCount);
		info->materialName = strToWStr(materialName);

		{	// Load material, assume the material is at the same path as the .mesh file itself.
			Path adjustedPath = mFileId ? mFileId->getBranchPath() / info->materialName : info->materialName;
			info->material = mResourceManager->load(adjustedPath);
		}

		readUInt16Array(*is, info->indexBuffer, indexCount);
		mModelInfo.push_back(info);
		++mCurrentModelInfoIdx;
	}	break;

	case M_GEOMETRY:
	{
		uint32_t vertexCount;
		is->read((char*)&vertexCount, sizeof(vertexCount));
		mModelInfo[mCurrentModelInfoIdx].vertexCount = vertexCount;
	}	break;

	case M_GEOMETRY_VERTEX_DECLARATION:
		break;

	case M_GEOMETRY_VERTEX_ELEMENT:
	{
		ModelInfo& modelInfo = mModelInfo[mCurrentModelInfoIdx];
		modelInfo.VertexDeclarations.push_back(VertexDeclaration());

		VertexDeclaration& decl = modelInfo.VertexDeclarations.back();
		is->read((char*)&decl, VertexDeclaration::cSize);
	}	break;

	case M_GEOMETRY_VERTEX_BUFFER:
	{
		ModelInfo& modelInfo = mModelInfo[mCurrentModelInfoIdx];

		uint16_t bindIndex;
		ABORT_IF(!readUInt16Array(*is, &bindIndex, 1));

		// TODO: Vertex the vertexSize is valid according to modelInfo.VertexDeclarations
		uint16_t vertexSize;
		ABORT_IF(!readUInt16Array(*is, &vertexSize, 1));

		if(bindIndex >= modelInfo.rawVertexBuffers.size())
			modelInfo.rawVertexBuffers.resize(bindIndex + 1);

		// Read the chunk M_GEOMETRY_VERTEX_BUFFER_DATA
		ABORT_IF(!header.readFrom(*is) || header.id != M_GEOMETRY_VERTEX_BUFFER_DATA);

		const size_t sizeInBytes = vertexSize * modelInfo.vertexCount;
		char* buf = new char[sizeInBytes];
		modelInfo.rawVertexBuffers[bindIndex].data = buf;
		modelInfo.rawVertexBuffers[bindIndex].vertexSize = vertexSize;
		is->read(buf, sizeInBytes);
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
		ABORT_IF(!readString(*is, subMeshName, sizeof(subMeshName)));

		if(idx < mModelInfo.size())
			mModelInfo[idx].name = strToWStr(subMeshName);
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

	MeshBuilder builder;

	MCD_FOREACH(const ModelInfo& modelInfo, mModelInfo)
	{
		MeshPtr mesh = new Mesh(modelInfo.name);
		(void)mesh;

		builder.clear();
		builder.enable(Mesh::Position | Mesh::Index);
		builder.reserveVertex(modelInfo.vertexCount);
		builder.reserveTriangle(modelInfo.indexCount / 3);

		// First pass to enable the possile data types first.
		MCD_FOREACH(const VertexDeclaration& decl, modelInfo.VertexDeclarations)
		{
			switch(decl.semantic)
			{
			case VES_NORMAL:
				builder.enable(Mesh::Normal);
				break;
			case VES_TEXTURE_COORDINATES:
				builder.enable(Mesh::TextureCoord0 + decl.index);
				builder.textureUnit(Mesh::TextureCoord0 + decl.index);
				builder.textureCoordSize(decl.type + VET_FLOAT2);
				break;
			}
		}

		// TODO: This loop is very slow, can be optimized the run-time by using extra memory.
		for(size_t i=0; i<modelInfo.vertexCount; ++i)
		{
			MCD_FOREACH(const VertexDeclaration& decl, modelInfo.VertexDeclarations)
			{
				const RawVertexBuffer& rawVertexBuffer = modelInfo.rawVertexBuffers[decl.source];
				const char* const p = rawVertexBuffer.data + i * rawVertexBuffer.vertexSize + decl.offset;

				switch(decl.semantic)
				{
					case VES_POSITION:
						builder.position(*reinterpret_cast<const Vec3f*>(p));
						break;
					case VES_NORMAL:
						builder.normal(*reinterpret_cast<const Vec3f*>(p));
						break;
					case VES_TEXTURE_COORDINATES:
						builder.textureUnit(Mesh::TextureCoord0 + decl.index);
						switch(decl.type) {
						case VET_FLOAT2:
							builder.textureCoord(*reinterpret_cast<const Vec2f*>(p));
							break;
						case VET_FLOAT3:
							builder.textureCoord(*reinterpret_cast<const Vec3f*>(p));
							break;
						case VET_FLOAT4:
							builder.textureCoord(*reinterpret_cast<const Vec4f*>(p));
							break;
						default:
							// Other texture coordinates are currently ignored
							break;
						}
						break;
					default:
						// Other semantics are currently ignored
						break;
				}
			}
			builder.addVertex();
		}

		for(size_t i=0; i<modelInfo.indexCount; i+=3)
		{
			uint16_t* idx = &modelInfo.indexBuffer[i];
			builder.addTriangle(idx[0], idx[1], idx[2]);
		}

		// TODO: Design a way to set this variable from outside
		builder.commit(*mesh, MeshBuilder::Static);

		Model::MeshAndMaterial* meshMat = new Model::MeshAndMaterial;
		model.mMeshes.pushBack(*meshMat);

		meshMat->mesh = mesh;
		meshMat->effect = new Effect(L"");

		// Use the default white material if none can load
		if(!modelInfo.material)
			meshMat->effect->material.reset(new WhiteMaterial);
		else
		{
			if(Texture* texture = dynamic_cast<Texture*>(modelInfo.material.get()))
				meshMat->effect->material.reset(new WhiteMaterial(texture));
			if(Effect* effect = dynamic_cast<Effect*>(modelInfo.material.get()))
				meshMat->effect = effect;
		}
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

IResourceLoader::LoadingState OgreMeshLoader::load(std::istream* is, const Path* fileId, const wchar_t* args)
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
