#ifndef __MCD_RENDER_DISPLAYLIST__
#define __MCD_RENDER_DISPLAYLIST__

#include "Renderable.h"

namespace MCD {

/*!	Similar to OpenGl display list that compile a list of
	immediate commands into a single efficient draw call.

	\note No sorting is done for the submitted primitives, sort it yourself
		if needed.
 */
class MCD_RENDER_API DisplayListComponent : public RenderableComponent, public IDrawCall
{
public:
	DisplayListComponent();

	sal_override ~DisplayListComponent();

	enum PrimitiveType
	{
		Triangles,
		Lines,
		LineStrip,
	};	// PrimitiveType

// Operations
	void clear();

	void begin(PrimitiveType primitive);
		void color(float r, float g, float b, float a=1);
		void texcoord(float u, float v, float w = 0);
		void normal(float x, float y, float z);
		void vertex(float x, float y, float z);
	void end();

	sal_override void render(sal_in void* context);
	sal_override void draw(sal_in void* context);

protected:
	class Impl;
	Impl& mImpl;
};	// DisplayListComponent

}	// namespace MCD

#endif	// __MCD_RENDER_DISPLAYLIST__
