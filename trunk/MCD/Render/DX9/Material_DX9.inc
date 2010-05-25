// To be included by Material_DX9.cpp and Renderer.cpp

namespace MCD {

class MaterialComponent::Impl
{
public:
	Impl();

	~Impl();

	bool createVs();
	bool createPs();

	LPDIRECT3DVERTEXSHADER9 mVs;
	LPDIRECT3DPIXELSHADER9 mPs;
	LPD3DXCONSTANTTABLE mVsConstTable;
	LPD3DXCONSTANTTABLE mPsConstTable;

	struct ConstantHandles
	{
		D3DXHANDLE world;
		D3DXHANDLE worldViewProj;
		D3DXHANDLE lights;
	};	// ConstantHandles

	ConstantHandles mConstantHandles;
};	// Impl

}	// namespace MCD
