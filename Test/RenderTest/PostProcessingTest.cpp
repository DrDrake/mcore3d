#include "Pch.h"
//#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Core/Entity/Entity.h"
//#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "../../MCD/Render/BackRenderBuffer.h"

using namespace MCD;

namespace PostProcessingTest
{
namespace v1
{
class FrameBuffers : private Noncopyable
{
public:
	enum DepthBufferType
	{
		DepthBuffer_None,
		DepthBuffer_Offscreen,
		DepthBuffer_Texture,
	};

	enum Filtering
	{
		Filtering_Point,
		Filtering_Bilinear,
		Filtering_Trilinear,
	};

	static void getGLFiltering(Filtering f, GLenum& outMagFilter, GLenum& outMinFilter);

	enum BufferFormat
	{
		BufferFormat_U8_RGBA,		// unsigned int8 with rgba components
		BufferFormat_F16_RGBA,		// float16 with rgba components
		BufferFormat_F16_RG,		// float16 with rg components
		BufferFormat_F16_R,			// float16 with r component
		BufferFormat_F32_RGBA,		// float32 with rgba components
		BufferFormat_F32_RG,		// float32 with rg components
		BufferFormat_F32_R,			// float32 with r component
	};

	static void getGLBufferFormat(BufferFormat f, GLenum& outInternalFmt, GLenum& outDataType, GLenum& outComponents);

	struct BufferInfo
	{
		//GLuint			handle;
		RenderBufferPtr	bufferPtr;
		bool			isTexture;
		BufferFormat	format;

        Texture* texture() const
        {
            if(isTexture)
                return static_cast<TextureRenderBuffer&>(*bufferPtr).texture.get();
            else
                return nullptr;
        }
	};

public:
	FrameBuffers(GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect);

	~FrameBuffers();

	/*! Adds a new texture buffer */
	void textureBuffer(BufferFormat format);

	/*! Framebuffer object handle */
	//GLuint handle() const {return mFBOHandle;}
	
	/*! Framebuffer width */
	size_t width() const {return mWidth;}

	/*! Framebuffer height */
	size_t height() const {return mHeight;}

	/*! OpenGL texture target.
		Used in glBindTexture(buf.target(), texHandle);
	*/
	GLenum target() const {return mTexTarget;}

	/*! # of buffers. */
	size_t bufferCnt() const {return mBufferInfos.size();}

	/*! Info of each buffer. */
	const BufferInfo& bufferInfo(size_t i) const {return mBufferInfos[i];}

	/*! Begin to use this Framebuffer for rendering
		The device current render target and viewport will be modified.
	*/
	bool begin(size_t n, size_t* bufferIdxs);

	/*! End to use this Framebuffer for rendering */
	void end();

	/*! Checks this Framebuffer's status, returns true if everything is ok. */
	bool checkFramebufferStatus(bool reportSuccess);

private:
	std::vector<BufferInfo> mBufferInfos;
	BufferInfo				mDepthBufferInfo;
	std::vector<GLenum>		mDrawBuffers;
	const GLenum			mTexTarget;
	size_t					mWidth;
	size_t					mHeight;
	//GLuint					mFBOHandle;

	RenderTarget			mRenderTarget;
};

void FrameBuffers::getGLFiltering(Filtering f, GLenum& outMagFilter, GLenum& outMinFilter)
{
	switch(f)
	{
	case Filtering_Point:
		{outMagFilter = GL_POINT; outMinFilter = GL_POINT;} break;
	case Filtering_Bilinear:
		{outMagFilter = GL_LINEAR; outMinFilter = GL_LINEAR;} break;
	case Filtering_Trilinear:
		{outMagFilter = GL_LINEAR; outMinFilter = GL_LINEAR_MIPMAP_LINEAR;} break;
	}
}

void FrameBuffers::getGLBufferFormat(BufferFormat f, GLenum& outInternalFmt, GLenum& outDataType, GLenum& outComponents)
{
	switch(f)
	{
	case BufferFormat_U8_RGBA:		// unsigned int8 with rgba components
		{outInternalFmt = GL_RGBA8; outDataType = GL_UNSIGNED_BYTE; outComponents = GL_RGBA;} break;
	case BufferFormat_F16_RGBA:		// float16 with rgba components
		{outInternalFmt = GL_RGBA16F_ARB; outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_BGRA;} break;
	case BufferFormat_F16_RG:		// float16 with rg components
		{outInternalFmt = GL_LUMINANCE_ALPHA16F_ARB; outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_LUMINANCE_ALPHA;} break;
	case BufferFormat_F16_R:		// float16 with r component
		{outInternalFmt = GL_LUMINANCE16F_ARB; outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_LUMINANCE;} break;
	case BufferFormat_F32_RGBA:	// float32 with rgba components
		{outInternalFmt = GL_RGBA32F_ARB; outDataType = GL_FLOAT; outComponents = GL_BGRA;} break;
	case BufferFormat_F32_RG:		// float32 with rg components
		{outInternalFmt = GL_LUMINANCE_ALPHA32F_ARB; outDataType = GL_FLOAT; outComponents = GL_LUMINANCE_ALPHA;} break;
	case BufferFormat_F32_R:		// float32 with r component
		{outInternalFmt = GL_LUMINANCE32F_ARB; outDataType = GL_FLOAT; outComponents = GL_LUMINANCE;} break;
	}
}

FrameBuffers::FrameBuffers(GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect)
	: mWidth(width), mHeight(height)
	, mRenderTarget(width, height)
	, mTexTarget(useTexRect ? GL_TEXTURE_RECTANGLE_ARB : GL_TEXTURE_2D)
{
	if(DepthBuffer_Offscreen == depthBufType)
	{
		mDepthBufferInfo.isTexture = false;
		BackRenderBuffer* bufferPtr = new BackRenderBuffer;
		bufferPtr->create(mWidth, mHeight, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT_EXT);
		mDepthBufferInfo.bufferPtr = bufferPtr;

		bufferPtr->linkTo(mRenderTarget);
	}
	else if(DepthBuffer_Texture == depthBufType)
	{
		mDepthBufferInfo.isTexture = true;
		TextureRenderBuffer* bufferPtr = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);
		bufferPtr->createTexture(mWidth, mHeight, mTexTarget, GL_DEPTH_COMPONENT);
		mDepthBufferInfo.bufferPtr = bufferPtr;

		bufferPtr->linkTo(mRenderTarget);
	}
}

FrameBuffers::~FrameBuffers()
{
}

void FrameBuffers::textureBuffer(BufferFormat format)
{
	GLenum internalFmt, dataType, components;
	getGLBufferFormat(format, internalFmt, dataType, components);

	TextureRenderBuffer* bufferPtr = new TextureRenderBuffer(GLenum(GL_COLOR_ATTACHMENT0_EXT + mBufferInfos.size()));
	
	//todo: also specific dataType, components
	bufferPtr->createTexture(mWidth, mHeight, mTexTarget, internalFmt);
	bufferPtr->linkTo(mRenderTarget);

    mRenderTarget.bind();
    checkFramebufferStatus(true);
    mRenderTarget.unbind();

	BufferInfo buf;

	buf.isTexture = true;
	buf.format = format;
	buf.bufferPtr = bufferPtr;

	/*
	glGenTextures(1, &buf.handle);
	glBindTexture(mTexTarget, buf.handle);

	glTexParameterf(mTexTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(mTexTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(mTexTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(mTexTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(mTexTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap

	GLenum internalFmt, dataType, components;
	getGLBufferFormat(format, internalFmt, dataType, components);

	glTexImage2D(mTexTarget, 0, internalFmt, mWidth, mHeight, 0, components, dataType, 0);
	glBindTexture(mTexTarget, 0);

	// attach to frame buffer
	//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOHandle);

	glFramebufferTexture2DEXT
		( GL_FRAMEBUFFER_EXT
		, GLenum(GL_COLOR_ATTACHMENT0_EXT + mBufferInfos.size())
		, mTexTarget
		, buf.handle, 0			// attach mip-level 0 to it
		);
	checkFramebufferStatus();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	*/

	mBufferInfos.push_back(buf);
}

bool FrameBuffers::checkFramebufferStatus(bool reportSuccess)
{
	using namespace std;

    // check FBO status
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
		if(reportSuccess)
			std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Unknow error." << std::endl;
        return false;
    }
}

bool FrameBuffers::begin(size_t n, size_t* bufferIdxs)
{
	if(mDrawBuffers.size() < n)
		mDrawBuffers.resize(n);

	for(size_t i = 0; i < n; ++i)
	{
		mDrawBuffers[i] = GLenum(GL_COLOR_ATTACHMENT0_EXT + bufferIdxs[i]);
	}

	//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOHandle);
	mRenderTarget.bind();

	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

	glDrawBuffers(n, &mDrawBuffers[0]);

	glViewport(0, 0, mWidth, mHeight);

	return checkFramebufferStatus(false);
}

void FrameBuffers::end()
{
	glPopAttrib();
	//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	mRenderTarget.unbind();
}

}

namespace v2
{
class FrameBuffers : private Noncopyable
{
public:
	enum DepthBufferType
	{
		DepthBuffer_None,
		DepthBuffer_Offscreen,
		DepthBuffer_Texture,
	};

	enum Filtering
	{
		Filtering_Point,
		Filtering_Bilinear,
		Filtering_Trilinear,
	};

	static void getGLFiltering(Filtering f, GLenum& outMagFilter, GLenum& outMinFilter);

	enum BufferFormat
	{
		BufferFormat_U8_RGBA,		// unsigned int8 with rgba components
		BufferFormat_F16_RGBA,		// float16 with rgba components
		BufferFormat_F16_RG,		// float16 with rg components
		BufferFormat_F16_R,			// float16 with r component
		BufferFormat_F32_RGBA,		// float32 with rgba components
		BufferFormat_F32_RG,		// float32 with rg components
		BufferFormat_F32_R,			// float32 with r component
	};

	static void getGLBufferFormat(BufferFormat f, GLenum& outInternalFmt, GLenum& outDataType, GLenum& outComponents);

	struct BufferInfo
	{
		GLuint			handle;
		BufferFormat	format;
		bool			isTexture;
	};

public:
	FrameBuffers(GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect);

	~FrameBuffers();

	/*! Adds a new texture buffer */
	void textureBuffer(BufferFormat format);

	/*! Framebuffer object handle */
	GLuint handle() const {return mFBOHandle;}
	
	/*! Framebuffer width */
	GLuint width() const {return mWidth;}

	/*! Framebuffer height */
	GLuint height() const {return mHeight;}

	/*! OpenGL texture target.
		Used in glBindTexture(buf.target(), texHandle);
	*/
	GLenum target() const {return mTexTarget;}

	/*! # of buffers. */
	size_t bufferCnt() const {return mBufferInfos.size();}

	/*! Info of each buffer. */
	const BufferInfo& bufferInfo(size_t i) const {return mBufferInfos[i];}

	/*! Begin to use this Framebuffer for rendering
		The device current render target and viewport will be modified.
	*/
	bool begin(size_t n, size_t* bufferIdxs);

	/*! End to use this Framebuffer for rendering */
	void end();

	/*! Checks this Framebuffer's status, returns true if everything is ok. */
	bool checkFramebufferStatus();

private:
	std::vector<BufferInfo> mBufferInfos;
	BufferInfo				mDepthBufferInfo;
	std::vector<GLenum>		mDrawBuffers;
	const GLenum			mTexTarget;
	GLuint					mWidth;
	GLuint					mHeight;
	GLuint					mFBOHandle;


};

void FrameBuffers::getGLFiltering(Filtering f, GLenum& outMagFilter, GLenum& outMinFilter)
{
	switch(f)
	{
	case Filtering_Point:
		{outMagFilter = GL_POINT; outMinFilter = GL_POINT;} break;
	case Filtering_Bilinear:
		{outMagFilter = GL_LINEAR; outMinFilter = GL_LINEAR;} break;
	case Filtering_Trilinear:
		{outMagFilter = GL_LINEAR; outMinFilter = GL_LINEAR_MIPMAP_LINEAR;} break;
	}
}

void FrameBuffers::getGLBufferFormat(BufferFormat f, GLenum& outInternalFmt, GLenum& outDataType, GLenum& outComponents)
{
	switch(f)
	{
	case BufferFormat_U8_RGBA:		// unsigned int8 with rgba components
		{outInternalFmt = GL_RGBA8; outDataType = GL_UNSIGNED_BYTE; outComponents = GL_RGBA;} break;
	case BufferFormat_F16_RGBA:		// float16 with rgba components
		{outInternalFmt = GL_RGBA16F_ARB; outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_BGRA;} break;
	case BufferFormat_F16_RG:		// float16 with rg components
		{outInternalFmt = GL_LUMINANCE_ALPHA16F_ARB; outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_LUMINANCE_ALPHA;} break;
	case BufferFormat_F16_R:		// float16 with r component
		{outInternalFmt = GL_LUMINANCE16F_ARB; outDataType = GL_HALF_FLOAT_ARB; outComponents = GL_LUMINANCE;} break;
	case BufferFormat_F32_RGBA:	// float32 with rgba components
		{outInternalFmt = GL_RGBA32F_ARB; outDataType = GL_FLOAT; outComponents = GL_BGRA;} break;
	case BufferFormat_F32_RG:		// float32 with rg components
		{outInternalFmt = GL_LUMINANCE_ALPHA32F_ARB; outDataType = GL_FLOAT; outComponents = GL_LUMINANCE_ALPHA;} break;
	case BufferFormat_F32_R:		// float32 with r component
		{outInternalFmt = GL_LUMINANCE32F_ARB; outDataType = GL_FLOAT; outComponents = GL_LUMINANCE;} break;
	}
}

FrameBuffers::FrameBuffers(GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect)
	: mWidth(width), mHeight(height)
	, mTexTarget(useTexRect ? GL_TEXTURE_RECTANGLE_ARB : GL_TEXTURE_2D)
{
	glGenFramebuffersEXT(1, &mFBOHandle);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOHandle);

	if(DepthBuffer_Offscreen == depthBufType)
	{
		mDepthBufferInfo.isTexture = false;

		glGenRenderbuffersEXT(1, &mDepthBufferInfo.handle);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthBufferInfo.handle);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDepthBufferInfo.handle);
	}
	else if(DepthBuffer_Texture == depthBufType)
	{
	}

	checkFramebufferStatus();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

FrameBuffers::~FrameBuffers()
{
	const size_t cBufCnt = mBufferInfos.size();
	for(size_t i = 0; i < cBufCnt; ++i)
	{
		glDeleteTextures(1, &mBufferInfos[i].handle);
	}

	glDeleteFramebuffersEXT(1, &mFBOHandle);
}

void FrameBuffers::textureBuffer(BufferFormat format)
{
	BufferInfo buf;

	buf.isTexture = true;
	buf.format = format;

	glGenTextures(1, &buf.handle);
	glBindTexture(mTexTarget, buf.handle);

	glTexParameterf(mTexTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(mTexTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(mTexTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(mTexTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(mTexTarget, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap

	GLenum internalFmt, dataType, components;
	getGLBufferFormat(format, internalFmt, dataType, components);

	glTexImage2D(mTexTarget, 0, internalFmt, mWidth, mHeight, 0, components, dataType, 0);
	glBindTexture(mTexTarget, 0);

	// attach to frame buffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOHandle);

	glFramebufferTexture2DEXT
		( GL_FRAMEBUFFER_EXT
		, GLenum(GL_COLOR_ATTACHMENT0_EXT + mBufferInfos.size())
		, mTexTarget
		, buf.handle, 0			// attach mip-level 0 to it
		);

	checkFramebufferStatus();

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	mBufferInfos.push_back(buf);
}

bool FrameBuffers::checkFramebufferStatus()
{
	using namespace std;

    // check FBO status
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Unknow error." << std::endl;
        return false;
    }
}

bool FrameBuffers::begin(size_t n, size_t* bufferIdxs)
{
	if(mDrawBuffers.size() < n)
		mDrawBuffers.resize(n);

	for(size_t i = 0; i < n; ++i)
	{
		mDrawBuffers[i] = GLenum(GL_COLOR_ATTACHMENT0_EXT + bufferIdxs[i]);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOHandle);

	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

	glDrawBuffers(n, &mDrawBuffers[0]);

	glViewport(0, 0, mWidth, mHeight);

	return true;
}

void FrameBuffers::end()
{
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

}	// namespace v2

typedef v1::FrameBuffers FrameBuffers;

/*! An untility for frame buffer binding.
*/
class ScopedFBBinding
{
public:
	ScopedFBBinding(FrameBuffers& fb, size_t bufId0)
		: mFB(fb)
	{
		size_t bufferIds[] = {bufId0};
		mFB.begin(1, bufferIds);
	}

	ScopedFBBinding(FrameBuffers& fb, size_t bufId0, size_t bufId1)
		: mFB(fb)
	{
		size_t bufferIds[] = {bufId0, bufId1};
		mFB.begin(2, bufferIds);
	}

	ScopedFBBinding(FrameBuffers& fb, size_t bufId0, size_t bufId1, size_t bufId2)
		: mFB(fb)
	{
		size_t bufferIds[] = {bufId0, bufId1, bufId2};
		mFB.begin(3, bufferIds);
	}

	ScopedFBBinding(FrameBuffers& fb, size_t bufId0, size_t bufId1, size_t bufId2, size_t bufId3)
		: mFB(fb)
	{
		size_t bufferIds[] = {bufId0, bufId1, bufId2, bufId3};
		mFB.begin(4, bufferIds);
	}

	~ScopedFBBinding()
	{
		mFB.end();
	}

	FrameBuffers& mFB;

};

/*! An untility for texture binding.
*/
class ScopedTexBinding
{
public:
	ScopedTexBinding(GLenum target, GLuint texId0)
		: mTexUnitCnt(1)
		, mTexTarget(target)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(target, texId0);
	}

	ScopedTexBinding(GLenum target, GLuint texId0, GLuint texId1)
		: mTexUnitCnt(2)
		, mTexTarget(target)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(target, texId0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(target, texId1);
	}

	ScopedTexBinding(GLenum target, GLuint texId0, GLuint texId1, GLuint texId2)
		: mTexUnitCnt(3)
		, mTexTarget(target)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(target, texId0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(target, texId1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(target, texId2);
	}

	ScopedTexBinding(GLenum target, GLuint texId0, GLuint texId1, GLuint texId2, GLuint texId3)
		: mTexUnitCnt(4)
		, mTexTarget(target)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(target, texId0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(target, texId1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(target, texId2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(target, texId3);
	}

	~ScopedTexBinding()
	{
		for(size_t i = 0; i < mTexUnitCnt; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(mTexTarget, 0);
		}

		glActiveTexture(GL_TEXTURE0);
	}

	const GLenum mTexTarget;
	const size_t mTexUnitCnt;
};

/*! An untility for texture binding.
*/
class ScopedTexBinding2
{
public:
    enum {MAX_TEXTURE_UNIT = 4};
    typedef Texture* TextureUnit;

    TextureUnit mTexUnits[MAX_TEXTURE_UNIT];
    size_t      mTexUnitCnt;

	ScopedTexBinding2(TextureUnit tu0)
		: mTexUnitCnt(1)
	{
        glActiveTexture(GL_TEXTURE0); tu0->bind();
		mTexUnits[0] = tu0;
	}

	ScopedTexBinding2(TextureUnit tu0, TextureUnit tu1)
		: mTexUnitCnt(2)
	{
		glActiveTexture(GL_TEXTURE0); tu0->bind();
		mTexUnits[0] = tu0;

        glActiveTexture(GL_TEXTURE1); tu1->bind();
		mTexUnits[1] = tu1;
	}

	ScopedTexBinding2(TextureUnit tu0, TextureUnit tu1, TextureUnit tu2)
		: mTexUnitCnt(3)
	{
		glActiveTexture(GL_TEXTURE0); tu0->bind();
		mTexUnits[0] = tu0;

        glActiveTexture(GL_TEXTURE1); tu1->bind();
		mTexUnits[1] = tu1;

        glActiveTexture(GL_TEXTURE2); tu2->bind();
		mTexUnits[2] = tu2;
	}

	ScopedTexBinding2(TextureUnit tu0, TextureUnit tu1, TextureUnit tu2, TextureUnit tu3)
		: mTexUnitCnt(4)
	{
		glActiveTexture(GL_TEXTURE0); tu0->bind();
		mTexUnits[0] = tu0;

        glActiveTexture(GL_TEXTURE1); tu1->bind();
		mTexUnits[1] = tu1;

        glActiveTexture(GL_TEXTURE2); tu2->bind();
		mTexUnits[2] = tu2;

        glActiveTexture(GL_TEXTURE3); tu3->bind();
		mTexUnits[3] = tu3;
	}

	~ScopedTexBinding2()
	{
		for(size_t i = 0; i < mTexUnitCnt; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            mTexUnits[i]->unbind();
        }

        glActiveTexture(GL_TEXTURE0);
	}
};

/*! An untility for pass binding inside a Material2 object.
*/
class ScopePassBinding
{
public:
	ScopePassBinding(Material2& material, size_t pass)
		: mMaterial(material)
		, mPass(pass)
	{
		mMaterial.preRender(mPass);
	}

	~ScopePassBinding()
	{
		mMaterial.postRender(mPass);
	}

	Material2& mMaterial;
	size_t mPass;
};

inline float GaussianZeroMean(float x, float sd)
{
    float t = x / sd;
    return expf(-0.5f * t * t);
}

/*! Test code
*/
class TestWindow : public BasicGlWindow
{
public:
	enum EffectPasses
	{
		SCENE_PASS,
		SKYBOX_PASS,
		SUN_EXTRACT_PASS,
		BLUR_PASS,
		COPY_PASS,
		RADIAL_MASK_PASS,
		RADIAL_GLOW_PASS,
	};

	enum BUFFERS
	{
		BUFFER0,
		BUFFER1,
	};

	enum
	{
		BLUR_KERNEL_SIZE = 10,
	};

	TestWindow()
		:
		BasicGlWindow(L"title=PostProcessingTest;width=800;height=600;fullscreen=0;FSAA=1"),
		mResourceManager(*createDefaultFileSystem())
	{
		// move slower
		mCameraVelocity = 5.0f;

		// load normal mapping effect
		mEffect = static_cast<Effect*>(mResourceManager.load(L"Material/postprocessingtest.fx.xml").get());

		mModel = dynamic_cast<Model*>(mResourceManager.load(L"Scene/City/scene.3ds").get());

		// sun
		m_sunPos = Vec3f(500.0f, 200.0f,-500.0f);

		// blur kernel
		float t = -BLUR_KERNEL_SIZE / 2.0f - 1.0f;
		float k = 0;
		for(int i = 0; i < BLUR_KERNEL_SIZE; ++i)
		{
			m_hblurOffset[i*2+0] = t;
			m_hblurOffset[i*2+1] = 0;

			m_vblurOffset[i*2+0] = 0;
			m_vblurOffset[i*2+1] = t;

			m_blurKernel[i] = GaussianZeroMean( t, BLUR_KERNEL_SIZE / 2.5f );
			k += m_blurKernel[i];

			t += 1.0f;
		}

		for(int i = 0; i < BLUR_KERNEL_SIZE; ++i)
			m_blurKernel[i] /= k;
	}

	sal_override void onResize(size_t width, size_t height)
	{
		if(0 == width || 0 == height)
			return;

		printf( "onResize %03d x %03d\n", width, height );

		mBuffersFull.reset( new FrameBuffers(width, height, FrameBuffers::DepthBuffer_Offscreen, false) );
		mBuffersFull->textureBuffer( FrameBuffers::BufferFormat_U8_RGBA );
		mBuffersFull->textureBuffer( FrameBuffers::BufferFormat_U8_RGBA );

		GLuint halfWidth = std::max((GLuint)2, GLuint(width / 2));
		GLuint halfHeight = std::max((GLuint)2, GLuint(height / 2));
		mBuffersQuar.reset( new FrameBuffers(halfWidth, halfHeight, FrameBuffers::DepthBuffer_Offscreen, false) );
		mBuffersQuar->textureBuffer( FrameBuffers::BufferFormat_U8_RGBA );
		mBuffersQuar->textureBuffer( FrameBuffers::BufferFormat_U8_RGBA );
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		Material2* mat = mEffect->material.get();

		if(!mat) return;
		if(!mBuffersFull.get()) return;
		if(!mModel) return;

		FrameBuffers& bufQuar = *mBuffersQuar;
		FrameBuffers& bufFull = *mBuffersFull;

		{	// scene pass
			ScopedFBBinding fbBinding(bufFull, BUFFER0);
			drawScene(mat);
		}

		const bool cDrawScene = true;

		if(cDrawScene)
		{
			{
				ScopedFBBinding fbBinding(bufQuar, BUFFER1);
				drawScene(mat);
			}
			{
				ScopedFBBinding fbBinding(bufQuar, BUFFER0);
				ScopePassBinding passBinding(*mat, SUN_EXTRACT_PASS);
				//ScopedTexBinding texBinding(bufQuar.target(), bufQuar.bufferInfo(BUFFER1).handle);
				ScopedTexBinding2 texBinding(bufQuar.bufferInfo(BUFFER1).texture());
				// bind shader uniform
				GLint program;
				glGetIntegerv(GL_CURRENT_PROGRAM, &program);

				if(0 != program)
				{
					glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );
				}

				// draw quad
				drawViewportQuad(0, 0, bufQuar.width(), bufQuar.height(), bufQuar.target());
			}
		}
		else
		{
			ScopedFBBinding fbBinding(bufQuar, BUFFER0);
			ScopePassBinding passBinding(*mat, SUN_EXTRACT_PASS);
			//ScopedTexBinding texBinding(bufFull.target(), bufFull.bufferInfo(BUFFER0).handle);
            ScopedTexBinding2 texBinding(bufFull.bufferInfo(BUFFER0).texture());

			// bind shader uniform
			GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			if(0 != program)
			{
				glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );
			}
			
			// draw quad
			drawViewportQuad(0, 0, bufQuar.width(), bufQuar.height(), bufQuar.target());
		}

		{	// horizontal blur pass
			ScopedFBBinding fbBinding(bufQuar, BUFFER1);
			ScopePassBinding passBinding(*mat, BLUR_PASS);
			//ScopedTexBinding texBinding(bufQuar.target(), bufQuar.bufferInfo(BUFFER0).handle);
            ScopedTexBinding2 texBinding(bufQuar.bufferInfo(BUFFER0).texture());

			// bind shader uniform
			GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			if(0 != program)
			{
				glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );
				glUniform2fv( glGetUniformLocation(program, "g_blurOffset"), BLUR_KERNEL_SIZE, m_hblurOffset.data() );
				glUniform1fv( glGetUniformLocation(program, "g_blurKernel"), BLUR_KERNEL_SIZE, m_blurKernel.data() );
				glUniform2f( glGetUniformLocation(program, "g_InvTexSize"), 1.0f / bufQuar.width(), 1.0f / bufQuar.height() );
			}

			// draw quad
			drawViewportQuad(0, 0, bufQuar.width(), bufQuar.height(), bufQuar.target());
		}

		{	// vertical blur pass
			ScopedFBBinding fbBinding(bufQuar, BUFFER0);
			ScopePassBinding passBinding(*mat, BLUR_PASS);
			//ScopedTexBinding texBinding(bufQuar.target(), bufQuar.bufferInfo(BUFFER1).handle);
            ScopedTexBinding2 texBinding(bufQuar.bufferInfo(BUFFER1).texture());

			// bind shader uniform
			GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			if(0 != program)
			{
				glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );
				glUniform2fv( glGetUniformLocation(program, "g_blurOffset"), BLUR_KERNEL_SIZE, m_vblurOffset.data() );
				glUniform1fv( glGetUniformLocation(program, "g_blurKernel"), BLUR_KERNEL_SIZE, m_blurKernel.data() );
				glUniform2f( glGetUniformLocation(program, "g_InvTexSize"), 1.0f / bufQuar.width(), 1.0f / bufQuar.height() );
			}

			// draw quad
			drawViewportQuad(0, 0, bufQuar.width(), bufQuar.height(), bufQuar.target());
		}

		{	// copy to screen
			ScopePassBinding passBinding(*mat, COPY_PASS);
			//ScopedTexBinding texBinding(bufFull.target(), bufFull.bufferInfo(BUFFER0).handle);
            ScopedTexBinding2 texBinding(bufFull.bufferInfo(BUFFER0).texture());

			GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			if(0 != program)
			{
				glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );
			}

			// draw quad
			drawViewportQuad(0, 0, this->width(), this->height(), bufFull.target());
		}

		{	// radial mask
			ScopedFBBinding fbBinding(bufQuar, BUFFER1);
			ScopePassBinding passBinding(*mat, RADIAL_MASK_PASS);
			//ScopedTexBinding texBinding(bufQuar.target(), bufQuar.bufferInfo(BUFFER0).handle);
            ScopedTexBinding2 texBinding(bufQuar.bufferInfo(BUFFER0).texture());

			// bind shader uniform
			GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			if(0 != program)
			{
				glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );
				glUniform1i( glGetUniformLocation(program, "g_maskSam"), 1 );
				glUniform3fv( glGetUniformLocation(program, "g_sunPos"), 1, m_sunPos.data );
			}

			// draw quad
			// preserve transforms since we need gl_ModelViewProjectionMatrox
			drawViewportQuad(0, 0, bufQuar.width(), bufQuar.height(), bufQuar.target(), true);
		}

		{	// radial glow
			ScopePassBinding passBinding(*mat, RADIAL_GLOW_PASS);
			//ScopedTexBinding texBinding(bufQuar.target(), bufQuar.bufferInfo(BUFFER1).handle);
            ScopedTexBinding2 texBinding(bufQuar.bufferInfo(BUFFER1).texture());

			// bind shader uniform
			GLint program; glGetIntegerv(GL_CURRENT_PROGRAM, &program);
			if(0 != program)
			{
				glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );
				glUniform3fv( glGetUniformLocation(program, "g_sunPos"), 1, m_sunPos.data );
			}

			// draw quad
			// preserve transforms since we need gl_ModelViewProjectionMatrox
			drawViewportQuad(0, 0, this->width(), this->height(), bufQuar.target(), true);
		}

		//drawSunPos();
	}

	void drawScene(Material2* mat)
	{
		{
			ScopePassBinding passBinding(*mat, SCENE_PASS);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// bind shader uniform
			GLint program;
			glGetIntegerv(GL_CURRENT_PROGRAM, &program);

			if(0 != program)
			{
				glUniform3fv( glGetUniformLocation(program, "g_sunPos"), 1, m_sunPos.data );
			}

			glPushMatrix();
			glScalef(0.05f, 0.05f, 0.05f);

			mModel->draw();

			glPopMatrix();
		}

		{
			ScopePassBinding passBinding(*mat, SKYBOX_PASS);

			Vec3f frustVertex[8];
			mCamera.frustum.computeVertex(frustVertex);
			
			Mat44f mat;
			mCamera.computeView(mat.getPtr());
			mat = mat.inverse();

			Vec3f p[4];
			for(size_t i = 0; i < 4; ++i)
			{
				p[i] = frustVertex[i] + (frustVertex[i+4] - frustVertex[i]) * 0.5f;
				mat.transformPoint(p[i]);
			}

			// bind shader uniform
			GLint program;
			glGetIntegerv(GL_CURRENT_PROGRAM, &program);

			if(0 != program)
			{
				glUniform3fv( glGetUniformLocation(program, "g_sunPos"), 1, m_sunPos.data );
			}

			glBegin(GL_TRIANGLES);

			glVertex3fv(p[0].data);
			glVertex3fv(p[1].data);
			glVertex3fv(p[2].data);

			glVertex3fv(p[0].data);
			glVertex3fv(p[2].data);
			glVertex3fv(p[3].data);

			glEnd();
		}
	}
	
	void drawSunPos()
	{
		glPushAttrib(GL_LIGHTING_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

		glBegin(GL_LINES);

		glColor3f(0,1,0);
		glVertex3fv((m_sunPos + Vec3f(-1, 0, 0)).data);
		glVertex3fv((m_sunPos + Vec3f( 1, 0, 0)).data);
		glVertex3fv((m_sunPos + Vec3f( 0,-1, 0)).data);
		glVertex3fv((m_sunPos + Vec3f( 0, 1, 0)).data);
		glVertex3fv((m_sunPos + Vec3f( 0, 0,-1)).data);
		glVertex3fv((m_sunPos + Vec3f( 0, 0, 1)).data);

		glVertex3fv(Vec3f::cZero.data);
		glVertex3fv(m_sunPos.data);

		glEnd();

		glPopAttrib();
	}

	DefaultResourceManager mResourceManager;
	ModelPtr mModel;
	EffectPtr mEffect;

	Vec3f m_sunPos;

	Array<float, BLUR_KERNEL_SIZE * 2> m_hblurOffset;
	Array<float, BLUR_KERNEL_SIZE * 2> m_vblurOffset;
	Array<float, BLUR_KERNEL_SIZE> m_blurKernel;

	std::auto_ptr<FrameBuffers> mBuffersFull;
	std::auto_ptr<FrameBuffers> mBuffersQuar;

};	// TestWindow

}	// namespace PostProcessingTest

TEST(PostProcessingTest)
{
	PostProcessingTest::TestWindow window;
	window.update(0.1f);
	window.mainLoop();
}