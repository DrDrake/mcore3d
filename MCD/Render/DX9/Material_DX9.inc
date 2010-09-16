#include "../Material.h"
#include "Helper/ShaderCache.h"

typedef const char* D3DXHANDLE;

namespace MCD {

class MaterialComponent::Impl
{
public:
	Impl();

	~Impl();

	bool createVs();
	bool createPs();

	// Update the world matrix shader constants only.
	void updateWorldTransform(void* context);

	DX9Helper::ShaderCache::Vs mVs;
	DX9Helper::ShaderCache::Ps mPs;

	struct ConstantHandles
	{
		D3DXHANDLE world;
		D3DXHANDLE worldViewProj;
		D3DXHANDLE lights;
		D3DXHANDLE cameraPosition;
		D3DXHANDLE diffuseColor;
		D3DXHANDLE specularColor;
		D3DXHANDLE emissionColor;
		D3DXHANDLE specularExponent;
		D3DXHANDLE opacity;
	};	// ConstantHandles

	ConstantHandles mConstantHandles;
};	// Impl

}	// namespace MCD
