#include "Pch.h"
//#include "ChamferBox.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Core/Entity/Entity.h"
//#include "../../MCD/Render/Components/MeshComponent.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "../../MCD/Render/BackRenderBuffer.h"
#include "../../MCD/Render/ShaderProgram.h"

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
		DepthBuffer_Texture16,
		DepthBuffer_Texture24,
		DepthBuffer_Texture32,
	};

	struct BufferInfo
	{
		RenderBufferPtr	bufferPtr;
		bool			isTexture;
		GLenum			format;

        Texture* texture() const
        {
            if(isTexture)
                return static_cast<TextureRenderBuffer&>(*bufferPtr).texture.get();
            else
                return nullptr;
        }
	};

public:
	FrameBuffers(ResourceManager& resMgr, GLuint width, GLuint height, DepthBufferType depthBufType, bool useTexRect);

	~FrameBuffers();

	/*! Adds a new texture buffer
	*/
	bool textureBuffer(int format, const wchar_t* texname);

	/*! Framebuffer width */
	size_t width() const {return mRenderTarget.width();}

	/*! Framebuffer height */
	size_t height() const {return mRenderTarget.height();}

	/*! OpenGL texture target.
		Used in glBindTexture(buf.target(), texHandle);
	*/
	GLenum target() const {return mTexTarget;}

	/*! # of buffers. */
	size_t bufferCnt() const {return mBufferInfos.size();}

	/*! Info of each buffer. */
	const BufferInfo& bufferInfo(size_t i) const {return mBufferInfos[i];}

	/*! Info of depth buffer. */
	const BufferInfo& depthBufferInfo() const {return mDepthBufferInfo;}

	/*! Begin to use this Framebuffer for rendering
		The device current render target and viewport will be modified.
	*/
	bool begin(size_t n, size_t* bufferIdxs);

	/*! End to use this Framebuffer for rendering */
	void end();

	/*! Checks this Framebuffer's status, returns true if everything is ok. */
	bool checkFramebufferStatus(bool reportSuccess);

private:
	ResourceManager&		mResMgr;
	std::vector<BufferInfo> mBufferInfos;
	BufferInfo				mDepthBufferInfo;
	std::vector<GLenum>		mDrawBuffers;
	const GLenum			mTexTarget;
	RenderTarget			mRenderTarget;
};

FrameBuffers::FrameBuffers(
	ResourceManager& resMgr
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
			Log::format(Log::Error, L"FrameBuffers: failed to create depth texture:%x", format);
		}
		bufferPtr->linkTo(mRenderTarget);

		mDepthBufferInfo.format = format;
		mDepthBufferInfo.isTexture = true;
		mDepthBufferInfo.bufferPtr = bufferPtr;
	}
}

FrameBuffers::~FrameBuffers()
{
}

bool FrameBuffers::textureBuffer(int format, const wchar_t* texname)
{
	int dataType, components;

	if (!Texture::dataTypeAndComponents(format, dataType, components))
		return false;

	TextureRenderBufferPtr bufferPtr = new TextureRenderBuffer(GLenum(GL_COLOR_ATTACHMENT0_EXT + mBufferInfos.size()));

	//todo: also specific dataType, components
	if(!bufferPtr->create(mRenderTarget.width(), mRenderTarget.height(), mTexTarget, format, dataType, components, texname))
	{
		Log::format(Log::Error, L"FrameBuffers: failed to create texture buffer:%s %x", texname, format);
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

bool FrameBuffers::checkFramebufferStatus(bool reportSuccess)
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

bool FrameBuffers::begin(size_t n, size_t* bufferIdxs)
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

void FrameBuffers::end()
{
	glPopAttrib();
	mRenderTarget.unbind();
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
		DEPTH_TEXTURE_PASS,
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

		// load model
		mModel = dynamic_cast<Model*>(mResourceManager.load(L"Scene/City/scene.3ds").get());

		// sun
		m_sunPos = Vec3f(-500.0f, 200.0f, -500.0f);

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

		Log::format(Log::Info, L"onResize %03d x %03d\n", width, height);

		int format = GL_RGBA8;
		//int format = GL_RGBA16F_ARB;

		bool useTexRect = false;

		mBuffersFull.reset( new FrameBuffers(mResourceManager, width, height, FrameBuffers::DepthBuffer_Texture24, useTexRect) );
		mBuffersFull->textureBuffer(format, L"rtt:/full.0.buf");
		mBuffersFull->textureBuffer(format, L"rtt:/full.1.buf");

		GLuint halfWidth = std::max((GLuint)2, GLuint(width / 2));
		GLuint halfHeight = std::max((GLuint)2, GLuint(height / 2));
		mBuffersHalf.reset( new FrameBuffers(mResourceManager, halfWidth, halfHeight, FrameBuffers::DepthBuffer_Texture24, useTexRect) );
		mBuffersHalf->textureBuffer(format, L"rtt:/half.0.buf");
		mBuffersHalf->textureBuffer(format, L"rtt:/half.1.buf");
		
		// load normal mapping effect
		mEffect = static_cast<Effect*>(mResourceManager.load(L"Material/postprocessingtest.fx.xml").get());

		Log::write(Log::Info, L"loading effect");
	}

	Material2::Pass& pass(Material2* mat, int pass)
	{
		return mat->mRenderPasses[pass];
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		if(!mEffect) return;
		if(!mModel) return;
		if(!mBuffersFull.get()) return;
		if(!mBuffersHalf.get()) return;

		Material2* mat = mEffect->material.get();
		if(!mat) return;

		FrameBuffers& bufHalf = *mBuffersHalf;
		FrameBuffers& bufFull = *mBuffersFull;

		{	// scene pass
			ScopedFBBinding bindFB(bufFull, BUFFER0);

			drawScene(mat);
		}

		{	// sun extract pass
			const int cPassId = SUN_EXTRACT_PASS;

			ScopedFBBinding bindFB(bufHalf, BUFFER0);

			pass(mat, cPassId).textureProp(0)->texture = bufFull.bufferInfo(BUFFER0).texture();
			
			ScopePassBinding bindPass(*mat, cPassId);

			// bind shader uniform
			//GLint program = pass(mat, cPassId).shaderProp()->shaderProgram->handle;
			
			// draw quad
			drawViewportQuad(0, 0, bufHalf.width(), bufHalf.height(), bufHalf.target());
		}

		{	// horizontal blur pass
			const int cPassId = BLUR_PASS;

			ScopedFBBinding bindFB(bufHalf, BUFFER1);

			pass(mat, cPassId).textureProp(0)->texture = bufHalf.bufferInfo(BUFFER0).texture();

			ScopePassBinding bindPass(*mat, cPassId);

			// bind shader uniform
			GLint program = pass(mat, cPassId).shaderProp()->shaderProgram->handle;
			glUniform2fv( glGetUniformLocation(program, "g_blurOffset"), BLUR_KERNEL_SIZE, m_hblurOffset.data() );
			glUniform1fv( glGetUniformLocation(program, "g_blurKernel"), BLUR_KERNEL_SIZE, m_blurKernel.data() );
			glUniform2f( glGetUniformLocation(program, "g_InvTexSize"), 1.0f / bufHalf.width(), 1.0f / bufHalf.height() );

			// draw quad
			drawViewportQuad(0, 0, bufHalf.width(), bufHalf.height(), bufHalf.target());
		}

		{	// vertical blur pass
			const int cPassId = BLUR_PASS;
			
			ScopedFBBinding bindFB(bufHalf, BUFFER0);

			pass(mat, cPassId).textureProp(0)->texture = bufHalf.bufferInfo(BUFFER1).texture();

			ScopePassBinding bindPass(*mat, cPassId);

			// bind shader uniform
			GLint program = pass(mat, cPassId).shaderProp()->shaderProgram->handle;
			glUniform2fv( glGetUniformLocation(program, "g_blurOffset"), BLUR_KERNEL_SIZE, m_vblurOffset.data() );
			glUniform1fv( glGetUniformLocation(program, "g_blurKernel"), BLUR_KERNEL_SIZE, m_blurKernel.data() );
			glUniform2f( glGetUniformLocation(program, "g_InvTexSize"), 1.0f / bufHalf.width(), 1.0f / bufHalf.height() );

			// draw quad
			drawViewportQuad(0, 0, bufHalf.width(), bufHalf.height(), bufHalf.target());
		}

		{	// copy to screen
			const int cPassId = COPY_PASS;

			pass(mat, cPassId).textureProp(0)->texture = bufFull.bufferInfo(BUFFER0).texture();
			
			ScopePassBinding bindPass(*mat, cPassId);

			//GLint program = pass(mat, cPassId).shaderProp()->shaderProgram->handle;

			// draw quad
			drawViewportQuad(0, 0, this->width(), this->height(), bufFull.target());
		}

		//{	// radial mask
		//  const int cPassId = RADIAL_MASK_PASS;
		//	ScopedFBBinding bindFB(bufHalf, BUFFER1);
		//  pass(mat, cPassId).textureProp(0)->texture = bufHalf.bufferInfo(BUFFER0).texture();
		//	ScopePassBinding bindPass(*mat, cPassId);

		//	// bind shader uniform
		//	GLint program = pass(mat, cPassId).shaderProp()->shaderProgram->handle;
		//	glUniform3fv( glGetUniformLocation(program, "g_sunPos"), 1, m_sunPos.data );

		//	// draw quad
		//	// preserve transforms since we need gl_ModelViewProjectionMatrox
		//	drawViewportQuad(0, 0, bufHalf.width(), bufHalf.height(), bufHalf.target(), true);
		//}

		{	// radial glow
			const int cPassId = RADIAL_GLOW_PASS;

			//pass(mat, cPassId).textureProp(0)->texture = bufHalf.bufferInfo(BUFFER1).texture();
			pass(mat, cPassId).textureProp(0)->texture = bufHalf.bufferInfo(BUFFER0).texture();

			ScopePassBinding bindPass(*mat, cPassId);
			
			// bind shader uniform
			GLint program = pass(mat, cPassId).shaderProp()->shaderProgram->handle;
			glUniform3fv( glGetUniformLocation(program, "g_sunPos"), 1, m_sunPos.data );

			// draw quad
			// preserve transforms since we need gl_ModelViewProjectionMatrox
			drawViewportQuad(0, 0, this->width(), this->height(), bufHalf.target(), true);
		}

		if(0)
		{	// show the depth buffer
			const int cPassId = DEPTH_TEXTURE_PASS;
			pass(mat, cPassId).textureProp(0)->texture = bufFull.depthBufferInfo().texture();
			
			ScopePassBinding bindPass(*mat, cPassId);
			
			// bind shader uniform
			GLint program = pass(mat, cPassId).shaderProp()->shaderProgram->handle;
			glUniform1i( glGetUniformLocation(program, "g_inputSam"), 0 );

			// draw quad
			drawViewportQuad(0, 0, this->width(), this->height(), GL_TEXTURE_RECTANGLE_ARB);
		}

		//drawSunPos();
	}

	void drawScene(Material2* mat)
	{
		{
			ScopePassBinding bindPass(*mat, SCENE_PASS);

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
			ScopePassBinding bindPass(*mat, SKYBOX_PASS);

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
	std::auto_ptr<FrameBuffers> mBuffersHalf;

};	// TestWindow

}	// namespace PostProcessingTest

TEST(PostProcessingTest)
{
	PostProcessingTest::TestWindow window;
	window.update(0.1f);
	window.mainLoop();
}
