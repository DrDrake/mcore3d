#include "Pch.h"
#include "../Mesh.h"
#include "Renderer.inc"
#include "../MeshBuilder.h"

#include <d3d9.h>
#include <D3DX9Shader.h>

#ifndef NDEBUG
#	pragma comment(lib, "D3dx9d")
#else
#	pragma comment(lib, "D3dx9")
#endif

namespace MCD {

void Mesh::draw(size_t drawIndexOffset, size_t drawIndexCount) {
	drawFaceOnly(drawIndexOffset, drawIndexCount);
}

void Mesh::drawFaceOnly(size_t drawIndexOffset, size_t drawIndexCount)
{
	if(indexCount == 0 || vertexCount == 0)
		return;

	drawIndexCount = (drawIndexCount == 0) ? this->indexCount : drawIndexCount;

	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	// Bind index buffer
	LPDIRECT3DINDEXBUFFER9* indexBuf = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(
		this->handles[attributes[Mesh::cIndexAttrIdx].bufferIndex].get()
	);
	MCD_ASSUME(indexBuf);
	device->SetIndices(*indexBuf);

	// Bind vertex and other buffers
	for(size_t i=1; i<bufferCount; ++i) {
		LPDIRECT3DVERTEXBUFFER9* vertexBuf = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(
			this->handles[i].get()
		);
		MCD_ASSUME(vertexBuf);
		MCD_VERIFY(device->SetStreamSource(i, *vertexBuf, 0, bufferSize(i)/vertexCount) == D3D_OK);
	}

	// Bind vertex declaration
	LPDIRECT3DVERTEXDECLARATION9& decl = reinterpret_cast<LPDIRECT3DVERTEXDECLARATION9&>(mImpl);
	MCD_VERIFY(device->SetVertexDeclaration(decl) == D3D_OK);

	// Draw the primitives
	MCD_VERIFY(device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, drawIndexOffset, drawIndexCount/3) == D3D_OK);
}

void Mesh::clear()
{
	LPDIRECT3DDEVICE9 device = getDevice();

	if(!device)
		return;

	// Release the buffer that is no longer shared.
	for(size_t i=0; i<handles.size(); ++i) {
		if(bufferCount && handles[i] && handles[i].referenceCount() == 1) {
			if(i == attributes[cIndexAttrIdx].bufferIndex) {
				LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[i].get());
				MCD_ASSUME(handle);
				SAFE_RELEASE(*handle);
			}
			else {
				LPDIRECT3DVERTEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(this->handles[i].get());
				MCD_ASSUME(handle);
				SAFE_RELEASE(*handle);
			}
		}

		handles[i] = new uint(0);
	}

	{	LPDIRECT3DVERTEXDECLARATION9& decl = reinterpret_cast<LPDIRECT3DVERTEXDECLARATION9&>(mImpl);
		SAFE_RELEASE(decl);
	}

	attributeCount = 0;
	bufferCount = 0;
	vertexCount = 0;
	indexCount = 0;
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
		MCD_ASSUME(handle);
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
			MCD_ASSUME(handle);
			MCD_VERIFY(SUCCEEDED((*handle)->Unlock()));
		}
		else {
			LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[i].get());
			MCD_ASSUME(handle);
			MCD_VERIFY(SUCCEEDED((*handle)->Unlock()));
		}

		mapped[i] = nullptr;	// Just feeling set it to null is more safe :)
	}
}

static BYTE toVertexDecl(const StringHash& semantic)
{
	if(semantic == StringHash("position"))
		return D3DDECLUSAGE_POSITION;
	if(semantic == StringHash("normal"))
		return D3DDECLUSAGE_NORMAL;
	if(semantic == StringHash("uv0"))
		return D3DDECLUSAGE_TEXCOORD;
	if(semantic == StringHash("uv1"))
		return D3DDECLUSAGE_TEXCOORD;
	if(semantic == StringHash("tangent"))
		return D3DDECLUSAGE_TANGENT;
	if(semantic == StringHash("binormal"))
		return D3DDECLUSAGE_BINORMAL;
	if(semantic == StringHash("color0"))
		return D3DDECLUSAGE_COLOR;
	if(semantic == StringHash("color1"))
		return D3DDECLUSAGE_COLOR;
	if(semantic == StringHash("jointWeight"))
		return D3DDECLUSAGE_BLENDWEIGHT;
	if(semantic == StringHash("jointIndex"))
		return D3DDECLUSAGE_BLENDINDICES;
	return BYTE(-1);
}

// More about lock usage on:
// http://www.toymaker.info/Games/html/d3d_resources.html
bool Mesh::create(const void* const* data, Mesh::StorageHint storageHint)
{
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	for(size_t i=0; i<bufferCount; ++i)
	{
		const size_t size = bufferSize(i);

		// TODO: Mesh::Stream may suffer from device reset.
		int storageFlag = (storageHint == Mesh::Stream) ? D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY : 0;
		D3DPOOL pool = (storageHint == Mesh::Stream) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

		if(i == Mesh::cIndexAttrIdx) {
			LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[i].get());
			MCD_ASSUME(handle);
			SAFE_RELEASE(*handle);

			if(FAILED(device->CreateIndexBuffer(size, storageFlag, D3DFMT_INDEX16, pool, handle, nullptr)))
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
			MCD_ASSUME(handle);
			SAFE_RELEASE(*handle);

			if(FAILED(device->CreateVertexBuffer(size, storageFlag, 0, pool, handle, nullptr)))
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

	// Create vertex declaration
	// Reference: http://msdn.microsoft.com/en-us/library/ee416419%28VS.85%29.aspx
	LPDIRECT3DVERTEXDECLARATION9& decl = reinterpret_cast<LPDIRECT3DVERTEXDECLARATION9&>(mImpl);
	SAFE_RELEASE(decl);

	D3DVERTEXELEMENT9 vertexDecl[Mesh::cMaxAttributeCount + 1];
	for(size_t i=Mesh::cPositionAttrIdx; i<attributeCount; ++i) {
		const Attribute& a = attributes[i];
		MCD_ASSERT(a.format.gpuFormat.dataType != -1);
		const D3DVERTEXELEMENT9 d = {
			a.bufferIndex, a.byteOffset,
			(BYTE)a.format.gpuFormat.dataType,
			D3DDECLMETHOD_DEFAULT, toVertexDecl(a.format.semantic.hashValue()), a.format.channel
		};
		vertexDecl[i] = d;
	}

	static const D3DVERTEXELEMENT9 gVertexDeclEnd = D3DDECL_END();
	vertexDecl[attributeCount] = gVertexDeclEnd;
	MCD_VERIFY(device->CreateVertexDeclaration(&vertexDecl[Mesh::cPositionAttrIdx], &decl) ==  D3D_OK);

	return true;
}

void MeshComponent::render(void* context)
{
	Entity* e = entity();
	MCD_ASSUME(e);

	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.submitDrawCall(*this, *e, e->worldTransform());
}

}	// namespace MCD
