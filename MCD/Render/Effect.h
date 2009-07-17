#ifndef __MCD_RENDER_EFFECT__
#define __MCD_RENDER_EFFECT__

#include "ShareLib.h"
#include "../Core/System/Resource.h"
#include <memory>

namespace MCD {

class Material2;

class MCD_RENDER_API Effect : public Resource
{
public:
	explicit Effect(const Path& fileId);

	sal_override ~Effect();

	std::auto_ptr<Material2> material;
};	// Effect

typedef IntrusivePtr<Effect> EffectPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_EFFECT__
