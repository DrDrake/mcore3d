#include "Pch.h"
#include "RenderBufferSet.h"
#include "../../MCD/Core/System/Log.h"

namespace MCD
{

RenderBufferSet::RenderBufferSet(
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

		if(!bufferPtr->linkTo(mRenderTarget))
			Log::format(Log::Error, "RenderBufferSet::RenderBufferSet() failed to link render target");

		mDepthBufferInfo.format = GL_DEPTH_COMPONENT24;
		mDepthBufferInfo.isTexture = false;
		mDepthBufferInfo.bufferPtr = bufferPtr;
	}
	else if(DepthBuffer_Texture16 <= depthBufType)
	{
		GpuDataFormat format;
		switch(depthBufType)
		{
		case DepthBuffer_Texture24:
			format = GpuDataFormat::get("depth24"); break;
		case DepthBuffer_Texture32:
			format = GpuDataFormat::get("depth32"); break;
		case DepthBuffer_Texture16:
		default:
			format = GpuDataFormat::get("depth16"); break;
		}

		MCD_ASSERT(format.isValid());

		TextureRenderBuffer* bufferPtr = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);

		// Depth texture must use GL_TEXTURE_RECTANGLE_ARB
		if(!bufferPtr->create(width, height, GL_TEXTURE_RECTANGLE_ARB, format))
			Log::format(Log::Error, "RenderBufferSet: failed to create depth texture:%x", format);

		if(!bufferPtr->linkTo(mRenderTarget))
			Log::format(Log::Error, "RenderBufferSet::RenderBufferSet() failed to link render target");

		mDepthBufferInfo.format = format.format;
		mDepthBufferInfo.isTexture = true;
		mDepthBufferInfo.bufferPtr = bufferPtr;
	}
}

RenderBufferSet::~RenderBufferSet()
{
}

bool RenderBufferSet::textureBuffer(const GpuDataFormat& format, const char* texname)
{
	TextureRenderBufferPtr bufferPtr = new TextureRenderBuffer(int(GL_COLOR_ATTACHMENT0_EXT + mBufferInfos.size()));

	if(!bufferPtr->create(mRenderTarget.width(), mRenderTarget.height(), mTexTarget, format, texname))
	{
		Log::format(Log::Error, "RenderBufferSet: failed to create texture buffer:%s %x", texname, format);
		return false;
	}

	if(!bufferPtr->linkTo(mRenderTarget))
		return false;

	{	mRenderTarget.bind();
		const bool ok = checkDeviceStatus(false);
		mRenderTarget.unbind();
		if(!ok)
			return false;
	}

	BufferInfo buf;

	buf.isTexture = true;
	buf.format = format.format;
	buf.bufferPtr = bufferPtr;

	mBufferInfos.push_back(buf);

	mResMgr.cache(bufferPtr->texture);

	return true;
}

bool RenderBufferSet::checkDeviceStatus(bool reportSuccess)
{
	using namespace std;

	// check FBO status
	const int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	switch(status)
	{
	case GL_FRAMEBUFFER_COMPLETE_EXT:
		if(reportSuccess)
			Log::write(Log::Info, "Framebuffer complete.");
		return true;

	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		Log::write(Log::Error, "[ERROR] Framebuffer incomplete: Attachment is NOT complete.");
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
		Log::write(Log::Error, "[ERROR] Framebuffer incomplete: No image is attached to FBO.");
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		Log::write(Log::Error, "[ERROR] Framebuffer incomplete: Attached images have different dimensions.");
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		Log::write(Log::Error, "[ERROR] Framebuffer incomplete: Color attached images have different internal formats.");
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
		Log::write(Log::Error, "[ERROR] Framebuffer incomplete: Draw buffer.");
		return false;

	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
		Log::write(Log::Error, "[ERROR] Framebuffer incomplete: Read buffer.");
		return false;

	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
		Log::write(Log::Error, "[ERROR] Unsupported by FBO implementation.");
		return false;

	default:
		Log::write(Log::Error, "[ERROR] Unknow error.");
		return false;
	}
}

void RenderBufferSet::begin(size_t n, const size_t* bufferIdxs)
{
	if(mDrawBuffers.size() < n)
		mDrawBuffers.resize(n);

	for(size_t i = 0; i < n; ++i)
	{
		mDrawBuffers[i] = int(GL_COLOR_ATTACHMENT0_EXT + bufferIdxs[i]);
	}

	mRenderTarget.bind();

	glPushAttrib(GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

	glDrawBuffers(n, (GLenum*)&mDrawBuffers[0]);

	glViewport(0, 0, mRenderTarget.width(), mRenderTarget.height());
}

void RenderBufferSet::begin(size_t bufid0)
{
	size_t buffers[] = {bufid0};
	begin(1, buffers);
}

void RenderBufferSet::begin(size_t bufid0, size_t bufid1)
{
	size_t buffers[] = {bufid0, bufid1};
	begin(2, buffers);
}

void RenderBufferSet::begin(size_t bufid0, size_t bufid1, size_t bufid2)
{
	size_t buffers[] = {bufid0, bufid1, bufid2};
	begin(3, buffers);
}

void RenderBufferSet::begin(size_t bufid0, size_t bufid1, size_t bufid2, size_t bufid3)
{
	size_t buffers[] = {bufid0, bufid1, bufid2, bufid3};
	begin(4, buffers);
}

void RenderBufferSet::end()
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