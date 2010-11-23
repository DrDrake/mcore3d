#include "../Material.h"
#include "Helper/ShaderCache.h"

typedef const char* D3DXHANDLE;

namespace MCD {

class MaterialComponent::Impl
{
public:
	Impl();

	~Impl();

	bool createVs(const char* headerCode);
	bool createPs(const char* headerCode);

	/// Create the required shaders according to the current options
	/// which may trigger a new compilation of shader.
	void createShadersIfNeeded();

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
		D3DXHANDLE lighting;
		D3DXHANDLE bumpMapSize;
	};	// ConstantHandles

	ConstantHandles mConstantHandles;

	struct MacroSettings {
		MacroSettings() : initialized(false) {}
		bool initialized;
		bool useVertexColor;
		bool useBumpMap;
	};	// MacroSettings

	MacroSettings mLastMacro;	///< The macro setting for the last frame, to detect shader recreation.
	MaterialComponent* mBackRef;
};	// Impl

}	// namespace MCD
