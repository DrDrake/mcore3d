#ifndef __MCD_RENDER_EFFECT__
#define __MCD_RENDER_EFFECT__

#include "ShareLib.h"
#include "../Core/System/Resource.h"
#include <memory>

namespace MCD {

class Material;

//! A simple wrapper over the Material to make it a kind of Resource.
class MCD_RENDER_API Effect : public Resource
{
public:
	explicit Effect(const Path& fileId);

	sal_override ~Effect();

	/*!	\note Aware that the material pointer may be null during background loading.
	 */
	std::auto_ptr<Material> material;
};	// Effect

typedef IntrusivePtr<Effect> EffectPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_EFFECT__
