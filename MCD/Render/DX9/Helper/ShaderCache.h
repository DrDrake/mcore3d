#ifndef __MCD_RENDER_DX9_HELPER_SHADERCACHE__
#define __MCD_RENDER_DX9_HELPER_SHADERCACHE__

#include "../../../Core/System/StringHash.h"
#include <map>

typedef struct IDirect3DVertexShader9 *LPDIRECT3DVERTEXSHADER9;
typedef struct IDirect3DPixelShader9 *LPDIRECT3DPIXELSHADER9;
struct ID3DXConstantTable;
typedef ID3DXConstantTable *LPD3DXCONSTANTTABLE;

namespace MCD {

class ResourceManager;

namespace DX9Helper {

/*!	Cache compiled shader
	The shader might be vertex shader or pixel shader, depends on which handle is not null
 */
class ShaderCache : Noncopyable
{
public:
	struct Vs
	{
		sal_maybenull LPDIRECT3DVERTEXSHADER9 vs;
		sal_maybenull LPD3DXCONSTANTTABLE constTable;
		void Release();
		void AddRef();
	};	// Vs

	struct Ps
	{
		sal_maybenull LPDIRECT3DPIXELSHADER9 ps;
		sal_maybenull LPD3DXCONSTANTTABLE constTable;
		void Release();
		void AddRef();
	};	// Ps

	~ShaderCache();

	/// Return Vs with null values if failed.
	/// Must call in main thread
	Vs getVertexShader(const char* sourceCode);
	Ps getPixelShader(const char* sourceCode);

	/// With the help of ResourceManager, these functions can call in any thread
	Vs getVertexShader(const char* sourceCode, ResourceManager& mgr);
	Ps getPixelShader(const char* sourceCode, ResourceManager& mgr);

protected:
	typedef std::map<uint32_t, Vs> VsMap;
	VsMap mVsMap;
	typedef std::map<uint32_t, Ps> PsMap;
	PsMap mPsMap;
};	// ShaderCache

}	// namespace DX9Helper
}	// namespace MCD

#endif	// __MCD_RENDER_DX9_HELPER_SHADERCACHE__
