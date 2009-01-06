#ifndef __MCD_RENDER_RENDERABLE__
#define __MCD_RENDER_RENDERABLE__

#include "../Core/System/Platform.h"

namespace MCD {

class MCD_ABSTRACT_CLASS IRenderable
{
public:
	virtual ~IRenderable() {}

	virtual void draw() = 0;
};	// IRenderable

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERABLE__
