#include "../Material.h"

typedef struct IDirect3DVertexShader9 *LPDIRECT3DVERTEXSHADER9;
typedef struct IDirect3DPixelShader9 *LPDIRECT3DPIXELSHADER9;
struct ID3DXConstantTable;
typedef ID3DXConstantTable *LPD3DXCONSTANTTABLE;
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

	LPDIRECT3DVERTEXSHADER9 mVs;
	LPDIRECT3DPIXELSHADER9 mPs;
	LPD3DXCONSTANTTABLE mVsConstTable;
	LPD3DXCONSTANTTABLE mPsConstTable;

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
