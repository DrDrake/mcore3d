#include "Pch.h"
#include "../DisplayList.h"
#include "../DisplayList.inc"
#include "../GpuDataFormat.h"
#include "Renderer.inc"
#include "../../Core/System/Utility.h"
#include <d3d9.h>

namespace MCD {

class DisplayListComponent::Impl : public DisplayListComponentImplBase
{
public:
	void draw(void* context);
};	// Impl

// Note that DirectX expects to find the structure fields in a specific order
// http://insanedevelopers.net/2009/03/22/directx-9-tutorials-02-vertex-buffers/
static const int cVertexFVF = (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1);

void DisplayListComponent::Impl::draw(void* context)
{
	LPDIRECT3DDEVICE9 device = getDevice();
	MCD_ASSUME(device);
	device->SetFVF(cVertexFVF);

	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	MCD_VERIFY(device->SetTransform(D3DTS_WORLD, (D3DMATRIX*)renderer.mWorldMatrix.transpose().getPtr()) == D3D_OK);	// TODO: Why transpose?

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

		if(count)
			MCD_VERIFY(device->DrawPrimitiveUP(type, count, &mVertices[section.offset], sizeof(Vertex)) == D3D_OK);
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

void DisplayListComponent::draw(void* context)
{
	mImpl.draw(context);
}

}	// namespace MCD
