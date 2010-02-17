#include "Pch.h"
#include "MeshLoader.h"
#include "Mesh.h"
#include "SemanticMap.h"
#include "../Core/System/MemoryProfiler.h"
#include <iostream>

namespace MCD {

class MeshLoader::Impl
{
public:
	Impl()
	{
		buffers.assign(nullptr);
	}

	~Impl()
	{
		for(size_t i=0; i<buffers.size(); ++i)
			delete buffers[i];
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const wchar_t* args);

	void commit(Resource& resource);

	IResourceLoader::LoadingState getLoadingState() const
	{
		return mLoadingState;
	}

private:
	Mesh::Attributes attributes;
	uint16_t bufferCount, attributeCount, vertexCount, indexCount;

	typedef Array<void*, Mesh::cMaxBufferCount> Buffers;
	Buffers buffers;
	typedef Array<uint32_t, Mesh::cMaxBufferCount> BufferSizes;
	BufferSizes bufferSizes;

	volatile IResourceLoader::LoadingState mLoadingState;
};	// Impl

IResourceLoader::LoadingState MeshLoader::Impl::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	// Simplying the error check
	#define ABORT_IF(expression) if(expression) { MCD_ASSERT(false); return mLoadingState = Aborted; }

	ABORT_IF(!is);

	if(mLoadingState != Loading)
		mLoadingState = NotLoaded;

	if(mLoadingState & Stopped)
		return mLoadingState;

	// Read the counts
	(*is) >> bufferCount >> attributeCount >> vertexCount >> indexCount;

	SemanticMap& semanticMap = SemanticMap::getSingleton();

	// Write the attribute descriptions
	for(size_t i=0; i<attributeCount; ++i) {
		// NOTE: For simplicity, Mesh::Attribute::semantic is read but ignored.
		is->read((char*)&attributes[i], sizeof(Mesh::Attribute));

		// Reconstruct the static string pointer from SemanticMap
		std::string semanticName;
		(*is) >> semanticName;

		MeshBuilder::Semantic semantic;
		ABORT_IF(!semanticMap.find(semanticName.c_str(), semantic));

		attributes[i].semantic = semantic.name;
	}

	// Read the buffers
	for(size_t i=0; i<bufferCount; ++i) {
		uint32_t size = 0;
		(*is) >> size;
		bufferSizes[i] = size;
		buffers[i] = new char[size];
		is->read((char*)buffers[i], size);
	}

	return mLoadingState;

	#undef ABORT_IF
}

// TODO: Unify the code with Mesh and MeshBuilder
void MeshLoader::Impl::commit(Resource& resource)
{
	// There is no need to do a mutex lock because MeshLoader didn't support progressive loading.
	// Therefore, commit will not be invoked if the load() function itsn't finished.

	Mesh& mesh = dynamic_cast<Mesh&>(resource);
	mesh.clear();

	mesh.bufferCount = bufferCount;
	mesh.attributeCount = attributeCount;
	mesh.vertexCount = vertexCount;
	mesh.indexCount = indexCount;
	mesh.attributes = attributes;

	// Setup the short cut attribute indices
	for(uint8_t i=0; i<attributeCount; ++i) {
		const Mesh::Attribute& a = attributes[i];
		const char* semantic = a.semantic;
		const SemanticMap& semanticMap = SemanticMap::getSingleton();

		if(strcmp(semantic, semanticMap.index().name) == 0)
			mesh.indexAttrIdx = i;
		else if(strcmp(semantic, semanticMap.position().name) == 0)
			mesh.positionAttrIdx = i;
		else if(strcmp(semantic, semanticMap.normal().name) == 0)
			mesh.normalAttrIdx = i;
		else if(strcmp(semantic, semanticMap.uv(0, a.elementCount).name) == 0)
			mesh.uv0AttrIdx = i;
		else if(strcmp(semantic, semanticMap.uv(1, a.elementCount).name) == 0)
			mesh.uv1AttrIdx = i;
		else if(strcmp(semantic, semanticMap.uv(2, a.elementCount).name) == 0)
			mesh.uv2AttrIdx = i;
	}

	// Commit buffer data to the Mesh
	for(size_t i=0; i<bufferCount; ++i)
	{
		uint* handle = mesh.handles[i].get();
		MCD_ASSUME(handle);
		if(!*handle)
			glGenBuffers(1, handle);

		const GLenum verOrIdxBuf = i == 0 ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
		glBindBuffer(verOrIdxBuf, *handle);
		glBufferData(verOrIdxBuf, bufferSizes[i], buffers[i], Mesh::Static);
	}
}

MeshLoader::MeshLoader()
	: mImpl(*new Impl)
{
}

MeshLoader::~MeshLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState MeshLoader::load(std::istream* is, const Path* fileId, const wchar_t* args)
{
	MemoryProfiler::Scope scope("MeshLoader::load");
	return mImpl.load(is, fileId, args);
}

void MeshLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("MeshLoader::commit");
	return mImpl.commit(resource);
}

IResourceLoader::LoadingState MeshLoader::getLoadingState() const
{
	return mImpl.getLoadingState();
}

}	// namespace MCD
