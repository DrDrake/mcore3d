#include "Pch.h"

#include "FramebufferSet.h"

#include "../../MCD/Core/System/Log.h"

namespace MCD
{

FrameBufferSet::FrameBufferSet(
	IResourceManager& resMgr
	, GLuint width, GLuint height
	, DepthBufferType depthBufType
	, bool useTexRect
	)
	: mResMgr(resMgr)
	, mRenderTarget(width, height)
	, mTexTarget(useTexRect ? GL_TEXTURE_RECTANGLE_ARB : GL_TEXTURE_2D)
{
	if(DepthBuffer_Offscreen == depthBufType)
	{		
		BackRenderBuffer* bufferPtr = new BackRenderBuffer;
		bufferPtr->create(width, height, GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT_EXT);
		bufferPtr->linkTo(mRenderTarget);

		mDepthBufferInfo.format = GL_DEPTH_COMPONENT24;
		mDepthBufferInfo.isTexture = false;
		mDepthBufferInfo.bufferPtr = bufferPtr;
	}
	else if(DepthBuffer_Texture16 <= depthBufType)
	{
		int format;
		switch(depthBufType)
		{
		case DepthBuffer_Texture16:
			{format = GL_DEPTH_COMPONENT16;} break;
		case DepthBuffer_Texture24:
			{format = GL_DEPTH_COMPONENT24;} break;
		case DepthBuffer_Texture32:
			{format = GL_DEPTH_COMPONENT32;} break;
		default:
			{format = GL_DEPTH_COMPONENT16;} break;
		}

		int dataType, components;
		Texture::dataTypeAndComponents(format, dataType, components);
		
		TextureRenderBuffer* bufferPtr = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);
		// depth texture must use GL_TEXTURE_RECTANGLE_ARB
		if(!bufferPtr->create(width, height, GL_TEXTURE_RECTANGLE_ARB, format, dataType, components))
		{
			Log::format(Log::Error, L"FrameBufferSet: failed to create depth texture:%x", format);
		}
		bufferPtr->linkTo(mRenderTarget);

		mDepthBufferInfo.format = format;
		mDepthBufferInfo.isTexture = true;
		mDepthBufferInfo.bufferPtr = bufferPtr;
	}
}

FrameBufferSet::~FrameBufferSet()
{
}

bool FrameBufferSet::textureBuffer(int format, const wchar_t* texname)
{
	int dataType, components;

	if (!Texture::dataTypeAndComponents(format, dataType, components))
		return false;

	TextureRenderBufferPtr bufferPtr = new TextureRenderBuffer(GLenum(GL_COLOR_ATTACHMENT0_EXT + mBufferInfos.size()));

	//todo: also specific dataType, components
	if(!bufferPtr->create(mRenderTarget.width(), mRenderTarget.height(), mTexTarget, format, dataType, components, texname))
	{
		Log::format(Log::Error, L"FrameBufferSet: failed to create texture buffer:%s %x", texname, format);
		return false;
	}

	bufferPtr->linkTo(mRenderTarget);

    mRenderTarget.bind();
    checkFramebufferStatus(true);
    mRenderTarget.unbind();

	BufferInfo buf;

	buf.isTexture = true;
	buf.format = format;
	buf.bufferPtr = bufferPtr;

	mBufferInfos.push_back(buf);

	mResMgr.cache(bufferPtr->texture);

	return true;
}

bool FrameBufferSet::checkFramebufferStatus(bool reportSuccess)
{
	using namespace std;

    // check FBO status
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
		if(reportSuccess)
			Log::write(Log::Info, L"Framebuffer complete.");
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		Log::write(Log::Error, L"[ERROR] Framebuffer incomplete: Attachment is NOT complete.");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        Log::write(Log::Error, L"[ERROR] Framebuffer incomplete: No image is attached to FBO.");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        Log::write(Log::Error, L"[ERROR] Framebuffer incomplete: Attached images have different dimensions.");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        Log::write(Log::Error, L"[ERROR] Framebuffer incomplete: Color attached images have different internal formats.");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        Log::write(Log::Error, L"[ERROR] Framebuffer incomplete: Draw buffer.");
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        Log::write(Log::Error, L"[ERROR] Framebuffer incomplete: Read buffer.");
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        Log::write(Log::Error, L"[ERROR] Unsupported by FBO implementation.");
        return false;

    default:
        Log::write(Log::Error, L"[ERROR] Unknow error.");
        return false;
    }
}

bool FrameBufferSet::begin(size_t n, const size_t* bufferIdxs)
{
	if(mDrawBuffers.size() < n)
		mDrawBuffers.resize(n);

	for(size_t i = 0; i < n; ++i)
	{
		mDrawBuffers[i] = GLenum(GL_COLOR_ATTACHMENT0_EXT + bufferIdxs[i]);
	}

	mRenderTarget.bind();

	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

	glDrawBuffers(n, &mDrawBuffers[0]);

	glViewport(0, 0, mRenderTarget.width(), mRenderTarget.height());

	return checkFramebufferStatus(false);
}

bool FrameBufferSet::begin(size_t bufid0)
{
	size_t buffers[] = {bufid0};
	return begin(1, buffers);
}

bool FrameBufferSet::begin(size_t bufid0, size_t bufid1)
{
	size_t buffers[] = {bufid0, bufid1};
	return begin(2, buffers);
}

bool FrameBufferSet::begin(size_t bufid0, size_t bufid1, size_t bufid2)
{
	size_t buffers[] = {bufid0, bufid1, bufid2};
	return begin(3, buffers);
}

bool FrameBufferSet::begin(size_t bufid0, size_t bufid1, size_t bufid2, size_t bufid3)
{
	size_t buffers[] = {bufid0, bufid1, bufid2, bufid3};
	return begin(4, buffers);
}

void FrameBufferSet::end()
{
	glPopAttrib();
	mRenderTarget.unbind();
}

void ScreenQuad::draw(int textureTarget, size_t x, size_t y, size_t width, size_t height)
{
	// How to draw fullscreen quad:
	// Reference: http://www.opengl.org/resources/faq/technical/transformations.htm

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glViewport(x, y, width, height);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_MULTISAMPLE);

	float w = (textureTarget == GL_TEXTURE_2D) ? 1.0f : float(width);
	float h = (textureTarget == GL_TEXTURE_2D) ? 1.0f : float(height);
	const float texcoord[4][2] = {
		{0, 0}, {w, 0}, {w, h}, {0, h}
	};

	glBegin(GL_QUADS);
		glTexCoord2fv(texcoord[0]);	glVertex3i(-1, -1, -1);
		glTexCoord2fv(texcoord[1]);	glVertex3i( 1, -1, -1);
		glTexCoord2fv(texcoord[2]);	glVertex3i( 1,  1, -1);
		glTexCoord2fv(texcoord[3]);	glVertex3i(-1,  1, -1);
	glEnd();

	glPopAttrib();
}

}	// namespace MCD