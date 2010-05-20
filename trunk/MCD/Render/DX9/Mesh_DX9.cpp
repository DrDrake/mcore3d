#include "Pch.h"
#include "../Mesh.h"
#include "../MeshBuilder.h"
#include "../RenderWindow.h"
#include "../../Core/System/Log.h"

#include "../Camera.h"
#include "../../Core/Math/Mat44.h"

#include <d3d9.h>
#include <D3DX9Shader.h>

#ifndef NDEBUG
#	pragma comment(lib, "D3dx9d")
#else
#	pragma comment(lib, "D3dx9")
#endif

namespace MCD {

LPDIRECT3DVERTEXSHADER9 gDefaultVertexShader = nullptr;
LPDIRECT3DPIXELSHADER9 gDefaultPixelShader = nullptr;
LPD3DXCONSTANTTABLE gConstTable = nullptr;

template<typename T> void SAFE_RELEASE(T& p)
{
	if(p) {
		p->Release();
		p = nullptr;
	}
}

static const D3DVERTEXELEMENT9 gVertexDeclEnd = D3DDECL_END();

void Mesh::draw()
{
	if(indexCount == 0)
		return;

	// An array to indicate which attribute is processed by fixed function.
	bool processed[cMaxAttributeCount] = { false };

	processed[cIndexAttrIdx] = true;
	processed[cPositionAttrIdx] = true;

	for(size_t i=2; i<attributes.size(); ++i) {
		const Attribute& a = attributes[i];
		if(a.format.semantic == StringHash("normal")) {
//			LPDIRECT3DVERTEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(this->handles[a.bufferIndex].get());
//			MCD_VERIFY(device->SetStreamSource(a.bufferIndex, *handle, a.byteOffset, a.stride) == D3D_OK);
		}
	}

	drawFaceOnly();
}

void Mesh::drawFaceOnly()
{
	Mat44f mat;
	Camera camera(Vec3f::cZero, Vec3f::c001, Vec3f::c010);
	camera.computeView(mat.getPtr());

	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	// Bind vertex buffers
	const Attribute& a = attributes[Mesh::cPositionAttrIdx];
	LPDIRECT3DVERTEXBUFFER9* vertexBuf = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(
		this->handles[a.bufferIndex].get()
	);
	MCD_VERIFY(device->SetStreamSource(a.bufferIndex, *vertexBuf, a.byteOffset, a.stride) == D3D_OK);

	// Bind index buffer
	LPDIRECT3DINDEXBUFFER9* indexBuf = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(
		this->handles[attributes[Mesh::cIndexAttrIdx].bufferIndex].get()
	);
	device->SetIndices(*indexBuf);

	// Bind vertex declaration
	LPDIRECT3DVERTEXDECLARATION9& decl = reinterpret_cast<LPDIRECT3DVERTEXDECLARATION9&>(mImpl);
	device->SetVertexDeclaration(decl);

	device->SetVertexShader(gDefaultVertexShader);
	device->SetPixelShader(gDefaultPixelShader);

	D3DXHANDLE handle;
	handle = gConstTable->GetConstantByName(nullptr, "mcdWorldViewProj");
	gConstTable->SetMatrix(device, handle,(D3DXMATRIX*)mat.getPtr());

	D3DVIEWPORT9 view_port;
	view_port.X=0;
	view_port.Y=0;
	view_port.Width=800;
	view_port.Height=600;
	view_port.MinZ=0.0f;
	view_port.MaxZ=1.0f;
	device->SetViewport(&view_port);

	camera.frustum.computePerspective(mat.getPtr());
	device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)mat.getPtr());

	// Draw the primitives
	MCD_VERIFY(device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount, 0, indexCount/3) == D3D_OK);
}

void Mesh::clear()
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());

	if(!device)
		return;

	// Release the buffer that is no longer shared.
	for(size_t i=0; i<handles.size(); ++i) {
		if(handles[i] && handles[i].referenceCount() == 1) {
			if(i == attributes[cIndexAttrIdx].bufferIndex) {
				LPDIRECT3DINDEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DINDEXBUFFER9*>(this->handles[i].get());
				SAFE_RELEASE(*handle);
			}
			else {
				LPDIRECT3DVERTEXBUFFER9* handle = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9*>(this->handles[i].get());
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
int VertexFormat::toApiDependentType(ComponentType type, size_t componentCount)
{
	static const int8_t mapping[][4] = {
		{ -1, -1, -1, -1 },
		{ -1, -1, -1, -1 },	// Integer
		{ -1, -1, -1, -1 },	// Unsigned integer
		{ -1, -1, -1, -1 },	// Signed byte
		{ -1, -1, -1, D3DDECLTYPE_UBYTE4 }, // Unsigned byte
		{ -1, D3DDECLTYPE_SHORT2, -1, D3DDECLTYPE_SHORT4 }, // Signed short
		{ -1, -1, -1, -1 },	// Unsigned short
		{ D3DDECLTYPE_FLOAT1, D3DDECLTYPE_FLOAT2, D3DDECLTYPE_FLOAT3, D3DDECLTYPE_FLOAT4 },
		{ -1, -1, -1, -1 },	// Double
		{ -1, -1, -1, -1 }
	};

	return mapping[type - VertexFormat::TYPE_NOT_USED][componentCount - 1];
}

static void createDefaultVS(LPDIRECT3DDEVICE9 device)
{
	static const char cDefaultVertexShader[] =
	"float4x4 mcdWorldViewProj;"
	"struct VS_INPUT { float4 position : POSITION; };"
	"struct VS_OUTPUT { float4 position : POSITION; float4 color : COLOR; };"
	"VS_OUTPUT main(const VS_INPUT v) {"
	"	VS_OUTPUT o;"
	"	o.position = mul(mcdWorldViewProj, v.position);"
	"	o.color = float4(1, 1, 1, 1);"
	"	return o;"
	"}";

	LPD3DXBUFFER vsBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	if(D3D_OK != D3DXCompileShader(
		cDefaultVertexShader, sizeof(cDefaultVertexShader),
		nullptr, nullptr,	// Shader macro and include
		"main", "vs_3_0",
		0,	// flags
		&vsBuf, &errors,
		&gConstTable	// Constant table
	))
	{
		const char* msg = (const char*)errors->GetBufferPointer();
		msg = msg;
		errors->Release();
	}

	device->CreateVertexShader((DWORD*)vsBuf->GetBufferPointer(), &gDefaultVertexShader);
	vsBuf->Release();
}

static void createDefaultPS(LPDIRECT3DDEVICE9 device)
{
	static const char cDefaultPixelShader[] =
	"struct PS_INPUT { float4 Color : COLOR0; };"
	"struct PS_OUTPUT { float4 Color : COLOR; };"
	"PS_OUTPUT main(PS_INPUT In) {"
	"	PS_OUTPUT Out = (PS_OUTPUT) 0;"
	"	Out.Color = In.Color;"
	"	Out.Color.r = 1.0f; Out.Color.g = 0.0f; Out.Color.b = 0.0f;"
	"	return Out;"
	"}";

	LPD3DXBUFFER psBuf = nullptr;
	LPD3DXBUFFER errors = nullptr;

	if(D3D_OK != D3DXCompileShader(
		cDefaultPixelShader, sizeof(cDefaultPixelShader),
		nullptr, nullptr,	// Shader macro and include
		"main", "ps_3_0",
		0,	// flags
		&psBuf, &errors,
		nullptr	// Constant table
	))
	{
		const char* msg = (const char*)errors->GetBufferPointer();
		msg = msg;
		errors->Release();
	}

	device->CreatePixelShader((DWORD*)psBuf->GetBufferPointer(), &gDefaultPixelShader);
	psBuf->Release();
}

static BYTE toVertexDecl(const StringHash& semantic)
{
	if(semantic == StringHash("position"))
		return D3DDECLUSAGE_POSITION;
	if(semantic == StringHash("normal"))
		return D3DDECLUSAGE_NORMAL;
	if(semantic == StringHash("uv0"))
		return D3DDECLUSAGE_TEXCOORD;
	return BYTE(-1);
}

bool Mesh::create(const void* const* data, Mesh::StorageHint storageHint)
{
	LPDIRECT3DDEVICE9 device = reinterpret_cast<LPDIRECT3DDEVICE9>(RenderWindow::getActiveContext());
	MCD_ASSUME(device);

	if(!gDefaultVertexShader)
		createDefaultVS(device);

	if(!gDefaultPixelShader)
		createDefaultPS(device);

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

	// Create vertex declaration
	// Reference: http://msdn.microsoft.com/en-us/library/ee416419%28VS.85%29.aspx
	LPDIRECT3DVERTEXDECLARATION9& decl = reinterpret_cast<LPDIRECT3DVERTEXDECLARATION9&>(mImpl);
	SAFE_RELEASE(decl);

	D3DVERTEXELEMENT9 vertexDecl[Mesh::cMaxAttributeCount + 1];
	for(size_t i=Mesh::cPositionAttrIdx; i<attributeCount; ++i) {
		const Attribute& a = attributes[i];
		const D3DVERTEXELEMENT9 d = {
			a.bufferIndex, a.byteOffset,
			(BYTE)VertexFormat::toApiDependentType(a.format.componentType, a.format.componentCount),
			D3DDECLMETHOD_DEFAULT, toVertexDecl(a.format.semantic.hashValue()), 0
		};
		vertexDecl[i] = d;
	}
	vertexDecl[attributeCount] = gVertexDeclEnd;
	MCD_VERIFY(device->CreateVertexDeclaration(&vertexDecl[Mesh::cPositionAttrIdx], &decl) ==  D3D_OK);

	return true;
}

}	// namespace MCD
