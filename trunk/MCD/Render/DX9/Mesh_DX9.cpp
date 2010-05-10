#include "Pch.h"
#include "../Mesh.h"
#include "../MeshBuilder.h"
#include "../RenderWindow.h"
#include "../../Core/System/Log.h"
#include <d3d9.h>

namespace MCD {

template<typename T> void SAFE_RELEASE(T& p)
{
	if(p) {
		p->Release();
		p = nullptr;
	}
}

static D3DVERTEXELEMENT9 gVertexDecl[Mesh::cMaxAttributeCount] = 
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{ 1, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL, 0},
	{ 2, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};	// gVertexDecl

static const D3DVERTEXELEMENT9 gVertexDeclEnd = D3DDECL_END();

void Mesh::draw()
{
	if(indexCount == 0)
		return;

	// An array to indicate which attribute is processed by fixed function.
	bool processed[cMaxAttributeCount] = { false };

	processed[cIndexAttrIdx] = true;
	processed[cPositionAttrIdx] = true;

	drawFaceOnly();

}

void Mesh::drawFaceOnly()
{
	gVertexDecl[attributeCount] = gVertexDeclEnd;
	for(size_t i=2; i<attributes.size(); ++i) {
		const Attribute& a = attributes[i];
		if(a.format.semantic == StringHash("normal")) {
		}
	}
}

void Mesh::clear()
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	if(!device)
		return;

	// Release the buffer that is no longer shared.
	for(size_t i=0; i<handles.size(); ++i) {
		if(!handles[i] || handles[i].referenceCount() != 1)
			continue;

		if(i == attributes[cIndexAttrIdx].bufferIndex) {
			LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[i].get());
			SAFE_RELEASE(*handle);
		}
		else {
			LPDIRECT3DVERTEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(this->handles[i].get());
			SAFE_RELEASE(*handle);
		}
	}

	attributeCount = 0;
	bufferCount = 0;
	vertexCount = 0;
	indexCount = 0;
}

MeshPtr Mesh::clone(const char* name, StorageHint hint)
{
	MeshPtr ret = new Mesh(name);

	ret->bufferCount = bufferCount;
	ret->attributes = attributes;
	ret->attributeCount = attributeCount;
	ret->vertexCount = vertexCount;
	ret->indexCount = indexCount;

	MappedBuffers mapped;
	for(size_t i=0; i<bufferCount; ++i) {
		void* data = mapBuffer(i, mapped);
		const GLenum verOrIdxBuf = i == 0 ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
		uint* handle = ret->handles[i].get();
		MCD_ASSUME(handle);
		glGenBuffers(1, handle);
		glBindBuffer(verOrIdxBuf, *handle);
		glBufferData(verOrIdxBuf, bufferSize(i), data, hint);
	}
	unmapBuffers(mapped);

	return ret;
}

void* Mesh::mapBuffer(size_t bufferIdx, MappedBuffers& mapped, MapOption mapOptions)
{
	if(bufferIdx >= bufferCount)
		return nullptr;

	if(mapped[bufferIdx])
		return mapped[bufferIdx];

	const size_t size = bufferSize(bufferIdx);

	int flags = 0;
	if(mapOptions & Discard)
		flags |= D3DLOCK_DISCARD;
	if(mapOptions & Read && !(mapOptions & Write))
		flags |= D3DLOCK_READONLY;
	
	void* ret;
	if(bufferIdx != attributes[cIndexAttrIdx].bufferIndex) {
		LPDIRECT3DVERTEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(this->handles[bufferIdx].get());
		if(FAILED((*handle)->Lock(0, size, &ret, flags)))
			return nullptr;
	}
	else {
		LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[bufferIdx].get());
		if(FAILED((*handle)->Lock(0, size, &ret, flags)))
			return nullptr;
	}

	mapped[bufferIdx] = ret;
	return ret;
}

void Mesh::unmapBuffers(MappedBuffers& mapped) const
{
	for(size_t i=0; i<bufferCount; ++i) {
		if(!mapped[i])
			continue;

		if(i != attributes[cIndexAttrIdx].bufferIndex) {
			LPDIRECT3DVERTEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(this->handles[i].get());
			MCD_VERIFY(SUCCEEDED((*handle)->Unlock()));
		}
		else {
			LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[i].get());
			MCD_VERIFY(SUCCEEDED((*handle)->Unlock()));
		}

		mapped[i] = nullptr;	// Just feeling set it to null is more safe :)
	}
}

// TODO: Put it back to API specific localtion
int VertexFormat::toApiDependentType(ComponentType type)
{
	static const int mapping[] = {
		-1,
		GL_INT, GL_UNSIGNED_INT,
		GL_BYTE, GL_UNSIGNED_BYTE,
		GL_SHORT, GL_UNSIGNED_SHORT,
		GL_FLOAT, GL_DOUBLE,
		-1
	};

	return mapping[type - VertexFormat::TYPE_NOT_USED];
}

bool Mesh::create(const void* const* data, Mesh::StorageHint storageHint)
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());
	MCD_ASSUME(device);

	for(size_t i=0; i<bufferCount; ++i)
	{
		const size_t size = bufferSize(i);
		int storageFlag = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;

		if(i == Mesh::cIndexAttrIdx) {
			LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[i].get());
			SAFE_RELEASE(*handle);

			if(FAILED(device->CreateIndexBuffer(size, storageFlag, D3DFMT_INDEX16, D3DPOOL_DEFAULT, handle, nullptr)))
				return false;
			if(const char* p = reinterpret_cast<const char*>(data[i])) {
				void* p2 = nullptr;
				if(FAILED((*handle)->Lock(0, size, &p2, 0)) || !p2)
					return false;
				memcpy(p2, p, size);
				MCD_VERIFY(SUCCEEDED((*handle)->Unlock()));
			}
		}
		else
		{
			LPDIRECT3DVERTEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(this->handles[i].get());
			SAFE_RELEASE(*handle);

			if(FAILED(device->CreateVertexBuffer(size, storageFlag, 0, D3DPOOL_DEFAULT, handle, nullptr)))
				return false;
			if(const char* p = reinterpret_cast<const char*>(data[i])) {
				void* p2 = nullptr;
				if(FAILED((*handle)->Lock(0, size, &p2, 0)) || !p2)
					return false;
				memcpy(p2, p, size);
				MCD_VERIFY(SUCCEEDED((*handle)->Unlock()));
			}
		}
	}
	return true;
}

}	// namespace MCD
