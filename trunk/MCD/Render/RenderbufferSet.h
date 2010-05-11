#ifndef __MCD_RENDER_RENDERBUFFERSET__
#define __MCD_RENDER_RENDERBUFFERSET__

#include "ShareLib.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/ResourceManager.h"
#include "../../MCD/Render/BackRenderBuffer.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"

namespace MCD {

class MCD_RENDER_API RenderBufferSet : private Noncopyable, public IntrusiveSharedObject<int>
{
public:
	enum DepthBufferType
	{
		DepthBuffer_None,
		DepthBuffer_Offscreen,
		DepthBuffer_Texture16,
		DepthBuffer_Texture24,
		DepthBuffer_Texture32,
	};

	struct BufferInfo
	{
		RenderBufferPtr bufferPtr;
		bool isTexture;
		int format;

		sal_maybenull Texture* texture() const
		{
			if(isTexture)
				return static_cast<TextureRenderBuffer&>(*bufferPtr).texture.get();
			else
				return nullptr;
		}
	};	// BufferInfo

public:
	RenderBufferSet(IResourceManager& resMgr, GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect);

	~RenderBufferSet();

// Attributes
	//! Framebuffer width
	size_t width() const {return mRenderTarget.width();}

	//! Framebuffer height
	size_t height() const {return mRenderTarget.height();}

	/*! OpenGL texture target.
		Used in glBindTexture(buf.target(), texHandle);
	 */
	int target() const {return mTexTarget;}

	//! # of buffers.
	size_t bufferCount() const {return mBufferInfos.size();}

	//!	Info of each buffer.
	const BufferInfo& bufferInfo(size_t i) const {return mBufferInfos[i];}

	//!	Info of depth buffer.
	const BufferInfo& depthBufferInfo() const {return mDepthBufferInfo;}

// Operations
	/*! Adds a new texture buffer
		@param format A device specific texture format.
	*/
	sal_checkreturn bool textureBuffer(int format, sal_in_z const char* texname);

	/*! Begin to use this Framebuffer for rendering
		The device current render target and viewport will be modified.
		Use checkDeviceStatus() to check the device status.
	*/
	void begin(size_t n, sal_in_ecount(n) const size_t* bufferIdxs);

	/*! Equals to:
		size_t buffers[] = {bufid0};
		begin(1, buffers);
	*/
	void begin(size_t bufid0);

	/*! Equals to:
		size_t buffers[] = {bufid0, bufid1};
		begin(2, buffers);
	*/
	void begin(size_t bufid0, size_t bufid1);

	/*! Equals to:
		size_t buffers[] = {bufid0, bufid1, bufid2};
		begin(3, buffers);
	*/
	void begin(size_t bufid0, size_t bufid1, size_t bufid2);

	/*! Equals to:
		size_t buffers[] = {bufid0, bufid1, bufid2, bufid3};
		begin(4, buffers);
	*/
	void begin(size_t bufid0, size_t bufid1, size_t bufid2, size_t bufid3);

	//! End to use this Framebuffer for rendering
	void end();

	/*! Checks the device's status, returns true if everything is ok.
		@param reportSuccess if true, a success message will be printed to Log().
	*/
	sal_checkreturn bool checkDeviceStatus(bool reportSuccess);

private:
	IResourceManager&		mResMgr;
	std::vector<BufferInfo> mBufferInfos;
	BufferInfo				mDepthBufferInfo;
	std::vector<int>		mDrawBuffers;
	const int			mTexTarget;
	RenderTarget			mRenderTarget;
};	// RenderBufferSet

// TODO: Should this class move to somewhere else?
class MCD_RENDER_API ScreenQuad
{
public:
	/*!	Assume the modelview matrix and projection matrix is set.
		Or vertex shader is being used to handle the transformation.
	*/
	static void draw(int textureTarget, size_t x, size_t y, size_t width, size_t height);
};

}	// namespace MCD

#endif	// __MCD_RENDER_RENDERBUFFERSET__