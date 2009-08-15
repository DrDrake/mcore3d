#ifndef __MCD_RENDER_FRAMEBUFFERSET__
#define __MCD_RENDER_FRAMEBUFFERSET__

#include "ShareLib.h"
#include "../Core/System/ResourceManager.h"
#include "../../MCD/Render/BackRenderBuffer.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"

namespace MCD
{

// TODO: Should this class have a name that is consit with class RenderBuffer?
class MCD_RENDER_API FrameBufferSet : private Noncopyable
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
		RenderBufferPtr	bufferPtr;
		bool			isTexture;
		GLenum			format;

        sal_maybenull Texture* texture() const
        {
            if(isTexture)
                return static_cast<TextureRenderBuffer&>(*bufferPtr).texture.get();
            else
                return nullptr;
        }
	};	// BufferInfo

public:
	FrameBufferSet(IResourceManager& resMgr, GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect);

	~FrameBufferSet();

// Attributes
	//! Framebuffer width
	size_t width() const {return mRenderTarget.width();}

	//! Framebuffer height
	size_t height() const {return mRenderTarget.height();}

	/*! OpenGL texture target.
		Used in glBindTexture(buf.target(), texHandle);
	 */
	GLenum target() const {return mTexTarget;}

	//! # of buffers.
	size_t bufferCount() const {return mBufferInfos.size();}

	//!	Info of each buffer.
	const BufferInfo& bufferInfo(size_t i) const {return mBufferInfos[i];}

	//!	Info of depth buffer.
	const BufferInfo& depthBufferInfo() const {return mDepthBufferInfo;}

// Operations
	/*! Adds a new texture buffer
	*/
	sal_checkreturn bool textureBuffer(int format, sal_in_z const wchar_t* texname);

	/*! Begin to use this Framebuffer for rendering
		The device current render target and viewport will be modified.
	*/
	bool begin(size_t n, sal_in_ecount(n) const size_t* bufferIdxs);

	/*! Equals to:
		size_t buffers[] = {bufid0};
		begin(1, buffers);
	*/
	bool begin(size_t bufid0);

	/*! Equals to:
		size_t buffers[] = {bufid0, bufid1};
		begin(2, buffers);
	*/
	bool begin(size_t bufid0, size_t bufid1);

	/*! Equals to:
		size_t buffers[] = {bufid0, bufid1, bufid2};
		begin(3, buffers);
	*/
	bool begin(size_t bufid0, size_t bufid1, size_t bufid2);

	/*! Equals to:
		size_t buffers[] = {bufid0, bufid1, bufid2, bufid3};
		begin(4, buffers);
	*/
	bool begin(size_t bufid0, size_t bufid1, size_t bufid2, size_t bufid3);

	//! End to use this Framebuffer for rendering
	void end();

	//! Checks this Framebuffer's status, returns true if everything is ok.
	sal_checkreturn bool checkFramebufferStatus(bool reportSuccess);

private:
	IResourceManager&		mResMgr;
	std::vector<BufferInfo> mBufferInfos;
	BufferInfo				mDepthBufferInfo;
	std::vector<GLenum>		mDrawBuffers;
	const GLenum			mTexTarget;
	RenderTarget			mRenderTarget;
};	// FrameBufferSet

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

#endif	// __MCD_RENDER_FRAMEBUFFERSET__