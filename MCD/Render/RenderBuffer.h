#ifndef __MCD_RENDER_RENDERBUFFER__
#define __MCD_RENDER_RENDERBUFFER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/IntrusivePtr.h"

namespace MCD {

class RenderTarget;

class MCD_NOVTABLE IRenderBuffer
{
public:
	virtual ~IRenderBuffer() {}
	virtual bool bind(RenderTarget& renderTarget) = 0;
	virtual size_t width() const = 0;
	virtual size_t height() const = 0;
};	// IRenderBuffer

/*!	Adds the shared smentics over the IRenderBuffer
 */
class MCD_NOVTABLE RenderBuffer : public IRenderBuffer, private Noncopyable
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
