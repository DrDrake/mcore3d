#include "Pch.h"
#include "RenderBuffer.h"
#include "RenderTarget.h"

namespace MCD {

void RenderBuffer::addOwnerShipTo(RenderTarget& renderTarget)
{
	renderTarget.mRenderBuffers.push_back(this);
}

}	// namespace MCD
