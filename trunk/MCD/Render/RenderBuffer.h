#ifndef __MCD_RENDER_RENDERBUFFER__
#define __MCD_RENDER_RENDERBUFFER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/IntrusivePtr.h"

namespace MCD {

class RenderTarget;

/*!	Represent a render buffer.
	A render buffer is a region of memory that a RenderTarget can render to.
	There are 2 kinds of render buffer, the first one is the back
	For simplicity, there is no way to un-link a render buffer from the RenderTarget.
	Therefore user have to create and bind the render buffer/target from scratch
	whenever a different configuration is needed.
 */
class MCD_ABSTRACT_CLASS IRenderBuffer
{
public:
	virtual ~IRenderBuffer() {}

	/*!	Link this render buffer to the render target.
		In order to link correctly, the render buffer itself should be
		initialized properly and having the same dimension as the render
		target.
	 */
	virtual sal_checkreturn bool linkTo(RenderTarget& renderTarget) = 0;

	virtual size_t width() const = 0;
	virtual size_t height() const = 0;
};	// IRenderBuffer

/*!	Adds the shared semantics over the IRenderBuffer.
	The RenderBuffer may shared by among different RenderTarget.
 */
class MCD_ABSTRACT_CLASS RenderBuffer : public IRenderBuffer, private Noncopyable
{
protected:
	RenderBuffer() : mRefCount(0) {}

	~RenderBuffer() {}

public:
	friend void intrusivePtrAddRef(RenderBuffer* renderBuffer) {
		++(renderBuffer->mRefCount);
	}

	friend void intrusivePtrRelease(RenderBuffer* renderBuffer)
	{
		if(--(renderBuffer->mRefCount) == 0)
			delete renderBuffer;
	}

protected:
	void addOwnerShipTo(RenderTarget& renderTarget);

	size_t mRefCount;
};	// RenderBuffer

typedef IntrusivePtr<RenderBuffer> RenderBufferPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERBUFFER__
