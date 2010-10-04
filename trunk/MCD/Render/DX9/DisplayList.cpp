#include "Pch.h"
#include "../DisplayList.h"
#include "../DisplayList.inc"
#include "../GpuDataFormat.h"
#include "Renderer.inc"
#include "../../Core/System/Utility.h"
#include <d3d9.h>
#include <D3DX9Shader.h>

namespace MCD {

//! To share the directx vertex declaration among all instances of DisplayListComponent
class SharedVertexDeclaration : public IntrusiveSharedWeakPtrTarget<size_t>
{
public:
	SharedVertexDeclaration(sal_notnull LPDIRECT3DDEVICE9 device);
	~SharedVertexDeclaration();

	LPDIRECT3DVERTEXDECLARATION9 decl;
};	// SharedVertexDeclaration

typedef IntrusivePtr<SharedVertexDeclaration> SharedVertexDeclarationPtr;
typedef IntrusiveSharedWeakPtr<SharedVertexDeclaration> SharedVertexDeclarationWeakPtr;

static SharedVertexDeclarationWeakPtr gDecl;

SharedVertexDeclaration::SharedVertexDeclaration(sal_notnull LPDIRECT3DDEVICE9 device)
{
	// The setting should match the strucutre of DisplayListComponentImplBase::Vertex
	const D3DVERTEXELEMENT9 vertexDecl[4 + 1] = {
		{ 0, 0,					D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, sizeof(float)*3,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, sizeof(float)*6,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, sizeof(float)*9,	D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END(),
	};

	MCD_ASSERT(!gDecl.lock());
	MCD_VERIFY(device->CreateVertexDeclaration(&vertexDecl[0], &decl) ==  D3D_OK);
	MCD_ASSUME(decl);
}

SharedVertexDeclaration::~SharedVertexDeclaration()
{
	MCD_ASSUME(decl);
	decl->Release();
}

class DisplayListComponent::Impl : public DisplayListComponentImplBase
{
public:
	void draw(void* context, Statistic& statistic);
	SharedVertexDeclarationPtr decl;
};	// Impl

//static LPDIRECT3DVERTEXDECLARATION9 gDecl = nullptr;

void DisplayListComponent::Impl::draw(void* context, Statistic& statistic)
{
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);

	if(!decl) {
		decl = gDecl.lock();
		if(!decl) {
			decl = new SharedVertexDeclaration(device);
			gDecl = decl.getNotNull();
		}
	}

	MCD_FOREACH(const Section& section, mSections)
	{
		D3DPRIMITIVETYPE type;
		size_t count = section.vertexCount;

		switch(section.primitive) {
		case Triangles:
			type = D3DPT_TRIANGLELIST;
			count /= 3;
			break;
		case Lines:
			count /= 2;
			type = D3DPT_LINELIST;
			break;
		case LineStrip:
			count -= 1;
			type = D3DPT_LINESTRIP;
			break;
		default:
			continue;
		}

		if(count) {
			device->SetVertexDeclaration(decl->decl);
			MCD_VERIFY(device->DrawPrimitiveUP(type, count, &mVertices[section.offset], sizeof(Vertex)) == D3D_OK);

			++statistic.drawCallCount;
			statistic.primitiveCount += count;
		}
	}
}

DisplayListComponent::DisplayListComponent()
	: mImpl(*new Impl)
{
}

DisplayListComponent::~DisplayListComponent()
{
	delete &mImpl;
}

void DisplayListComponent::clear()
{
	mImpl.clear();
}

void DisplayListComponent::begin(PrimitiveType primitive)
{
	mImpl.begin(primitive);
}

void DisplayListComponent::color(float r, float g, float b, float a)
{
	mImpl.color(r, g, b, a);
}

void DisplayListComponent::texcoord(float u, float v, float w)
{
	mImpl.texcoord(u, v, w);
}

void DisplayListComponent::normal(float x, float y, float z)
{
	mImpl.normal(x, y, z);
}

void DisplayListComponent::vertex(float x, float y, float z)
{
	mImpl.vertex(x, y, z);
}

void DisplayListComponent::end()
{
	mImpl.end();
}

void DisplayListComponent::render(void* context)
{
	Entity* e = entity();
	MCD_ASSUME(e);

	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.submitDrawCall(*this, *e, e->worldTransform());
}

void DisplayListComponent::draw(void* context, Statistic& statistic)
{
	mImpl.draw(context, statistic);
}

}	// namespace MCD
