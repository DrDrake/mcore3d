#include "Pch.h"
#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Render/TangentSpaceBuilder.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"

using namespace MCD;

namespace PostProcessingTest
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
		GLuint handle;
		BufferFormat format;
		GLenum attachmentPoint;
		bool isTexture;
	};

public:
	FrameBuffers(GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect);
	~FrameBuffers();

	/*! Adds a new texture buffer */
	void textureBuffer(BufferFormat format);

	GLuint handle() const {return mFBOId;}
	GLuint width() const {return mWidth;}
	GLuint height() const {return mHeight;}

	size_t bufferCnt() const {return mBufferInfos.size();}
	const BufferInfo& bufferInfo(size_t i) const {return mBufferInfos[i];}

	bool begin(size_t n, size_t* bufferIdxs);
	void end();

	bool checkFramebufferStatus();

private:
	std::vector<BufferInfo> mBufferInfos;
	BufferInfo mDepthBufferInfo;
	std::vector<GLenum> mDrawBuffers;
	const GLenum mTexTarget;
	GLuint mWidth;
	GLuint mHeight;
	GLuint mFBOId;

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
	glGenFramebuffersEXT(1, &mFBOId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOId);

	if(DepthBuffer_Offscreen == depthBufType)
	{
		mDepthBufferInfo.attachmentPoint = GL_DEPTH_ATTACHMENT_EXT;
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

	glDeleteFramebuffersEXT(1, &mFBOId);
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

	if(mBufferInfos.empty())
		buf.attachmentPoint = GL_COLOR_ATTACHMENT0_EXT;
	else
		buf.attachmentPoint = mBufferInfos.back().attachmentPoint + 1;

	// attach to frame buffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOId);

	glFramebufferTexture2DEXT
		( GL_FRAMEBUFFER_EXT
		, buf.attachmentPoint
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
		mDrawBuffers[i] = mBufferInfos[bufferIdxs[i]].attachmentPoint;

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBOId);

	glDrawBuffers(n, &mDrawBuffers[0]);

	//return checkFramebufferStatus();
	return true;
}

void FrameBuffers::end()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

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
		BLUR_PASS,
		COPY_PASS,
	};

	enum BUFFERS
	{
		BUFFER0,
		BUFFER1,
	};

	enum
	{
		BLUR_KERNEL_SIZE = 9,
	};

	TestWindow()
		:
		BasicGlWindow(L"title=PostProcessingTest;width=800;height=600;fullscreen=0;FSAA=1"),
		mResourceManager(*createDefaultFileSystem())
	{
		// load normal mapping effect
		mEffect = static_cast<Effect*>(mResourceManager.load(L"Material/postprocessingtest.fx.xml").get());

		mModel = dynamic_cast<Model*>(mResourceManager.load(L"Scene/City/scene.3ds").get());

		// blur kernel
		float t = -BLUR_KERNEL_SIZE / 2.0f;
		float k = 0;
		for(int i = 0; i < BLUR_KERNEL_SIZE; ++i)
		{
			m_hblurOffset[i*2+0] = t;
			m_hblurOffset[i*2+1] = 0;

			m_vblurOffset[i*2+0] = 0;
			m_vblurOffset[i*2+1] = t;

			m_blurKernel[i] = GaussianZeroMean( t, BLUR_KERNEL_SIZE / 2.0f );
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

		mBuffers.reset( new FrameBuffers(width, height, FrameBuffers::DepthBuffer_Offscreen, false) );
		mBuffers->textureBuffer( FrameBuffers::BufferFormat_U8_RGBA );
		mBuffers->textureBuffer( FrameBuffers::BufferFormat_U8_RGBA );
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		Material2* mat = mEffect->material.get();

		if(!mat) return;
		if(!mBuffers.get()) return;
		if(!mModel) return;

		{	// scene pass

			ScopedFBBinding fbBinding(*mBuffers, BUFFER0);
			ScopePassBinding passBinding(*mat, SCENE_PASS);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glPushMatrix();
			glScalef(0.01f, 0.01f, 0.01f);

			mModel->draw();

			glPopMatrix();
		}
			
		{	// horizontal blur pass

			ScopedFBBinding fbBinding(*mBuffers, BUFFER1);
			ScopePassBinding passBinding(*mat, BLUR_PASS);
			ScopedTexBinding texBinding(GL_TEXTURE_2D, mBuffers->bufferInfo(BUFFER0).handle);

			// bind shader uniform
			GLint program;
			glGetIntegerv(GL_CURRENT_PROGRAM, &program);

			if(0 != program)
			{
				glUniform2fv( glGetUniformLocation(program, "g_blurOffset"), BLUR_KERNEL_SIZE, m_hblurOffset.data() );
				glUniform1fv( glGetUniformLocation(program, "g_blurKernel"), BLUR_KERNEL_SIZE, m_blurKernel.data() );
				glUniform2f( glGetUniformLocation(program, "g_InvTexSize"), 1.0f / width(), 1.0f / height() );
			}

			// draw quad
			drawViewportQuad(0, 0, width(), height(), GL_TEXTURE_2D);
		}

		{	// vertical blur pass
			
			ScopedFBBinding fbBinding(*mBuffers, BUFFER0);
			ScopePassBinding passBinding(*mat, BLUR_PASS);
			ScopedTexBinding texBinding(GL_TEXTURE_2D, mBuffers->bufferInfo(BUFFER1).handle);

			// bind shader uniform
			GLint program;
			glGetIntegerv(GL_CURRENT_PROGRAM, &program);

			if(0 != program)
			{
				glUniform2fv( glGetUniformLocation(program, "g_blurOffset"), BLUR_KERNEL_SIZE, m_vblurOffset.data() );
				glUniform1fv( glGetUniformLocation(program, "g_blurKernel"), BLUR_KERNEL_SIZE, m_blurKernel.data() );
				glUniform2f( glGetUniformLocation(program, "g_InvTexSize"), 1.0f / width(), 1.0f / height() );
			}

			// draw quad
			drawViewportQuad(0, 0, width(), height(), GL_TEXTURE_2D);
		}

		{	// copy to screen

			ScopePassBinding passBinding(*mat, COPY_PASS);
			ScopedTexBinding texBinding(GL_TEXTURE_2D, mBuffers->bufferInfo(BUFFER0).handle);

			// draw quad
			drawViewportQuad(0, 0, width(), height(), GL_TEXTURE_2D);
		}
	}

	DefaultResourceManager mResourceManager;
	ModelPtr mModel;
	EffectPtr mEffect;

	Array<float, BLUR_KERNEL_SIZE * 2> m_hblurOffset;
	Array<float, BLUR_KERNEL_SIZE * 2> m_vblurOffset;
	Array<float, BLUR_KERNEL_SIZE> m_blurKernel;

	std::auto_ptr<FrameBuffers> mBuffers;

};	// TestWindow

}	// namespace PostProcessingTest

TEST(PostProcessingTest)
{
	PostProcessingTest::TestWindow window;
	window.update(0.1f);
	window.mainLoop();
}
