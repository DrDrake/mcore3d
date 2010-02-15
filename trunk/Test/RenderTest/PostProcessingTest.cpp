#include "Pch.h"

#include "DefaultResourceManager.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/RenderBufferSet.h"
#include "../../MCD/Render/ShaderProgram.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/System/Array.h"

using namespace MCD;

namespace PostProcessingTest
{

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

	float GaussianZeroMean(float x, float sd)
	{
		float t = x / sd;
		return expf(-0.5f * t * t);
	}

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

		const int format = GL_RGBA8;
		//int format = GL_RGBA16F_ARB;

		const bool useTexRect = false;

		mBuffersFull.reset( new RenderBufferSet(mResourceManager, width, height, RenderBufferSet::DepthBuffer_Texture24, useTexRect) );
		MCD_VERIFY(mBuffersFull->textureBuffer(format, L"rtt:/full.0.buf"));
		MCD_VERIFY(mBuffersFull->textureBuffer(format, L"rtt:/full.1.buf"));

		GLuint halfWidth = std::max((GLuint)2, GLuint(width / 2));
		GLuint halfHeight = std::max((GLuint)2, GLuint(height / 2));
		mBuffersHalf.reset( new RenderBufferSet(mResourceManager, halfWidth, halfHeight, RenderBufferSet::DepthBuffer_Texture24, useTexRect) );
		MCD_VERIFY(mBuffersHalf->textureBuffer(format, L"rtt:/half.0.buf"));
		MCD_VERIFY(mBuffersHalf->textureBuffer(format, L"rtt:/half.1.buf"));
		
		// load normal mapping effect
		mEffect = static_cast<Effect*>(mResourceManager.load(L"Material/postprocessingtest.fx.xml").get());
	}

	Material::Pass& pass(Material* mat, int pass)
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

		Material* mat = mEffect->material.get();
		if(!mat) return;

		RenderBufferSet& bufHalf = *mBuffersHalf;
		RenderBufferSet& bufFull = *mBuffersFull;

		{	// scene pass
			RenderBufferSet& frameBuf = bufFull;

			frameBuf.begin(BUFFER0);

			drawScene(mat);

			frameBuf.end();
		}

		{	// sun extract pass
			const int cPassId = SUN_EXTRACT_PASS;

			RenderBufferSet& frameBuf = bufHalf;
			frameBuf.begin(BUFFER0);

			pass(mat, cPassId).textureProp(0)->texture = bufFull.bufferInfo(BUFFER0).texture();
			pass(mat, cPassId).preRender();

			// bind shader uniform
			//ShaderProgram& program = *ShaderProgram::current();//*pass(mat, cPassId).shaderProp()->shaderProgram;
			
			// draw quad
			//drawViewportQuad(0, 0, bufHalf.width(), bufHalf.height(), bufHalf.target());
			ScreenQuad::draw(bufHalf.target(), 0, 0, bufHalf.width(), bufHalf.height());

			pass(mat, cPassId).postRender();

			frameBuf.end();
		}

		{	// horizontal blur pass
			const int cPassId = BLUR_PASS;

			RenderBufferSet& frameBuf = bufHalf;
			frameBuf.begin(BUFFER1);

			pass(mat, cPassId).textureProp(0)->texture = bufHalf.bufferInfo(BUFFER0).texture();
			pass(mat, cPassId).preRender();

			// bind shader uniform
			ShaderProgram& program = *ShaderProgram::current();//*pass(mat, cPassId).shaderProp()->shaderProgram;
			program.uniform2fv( "g_blurOffset", BLUR_KERNEL_SIZE, m_hblurOffset.data() );
			program.uniform1fv( "g_blurKernel", BLUR_KERNEL_SIZE, m_blurKernel.data() );
			program.uniform2f ( "g_InvTexSize", 1.0f / bufHalf.width(), 1.0f / bufHalf.height() );

			// draw quad
			//drawViewportQuad(0, 0, bufHalf.width(), bufHalf.height(), bufHalf.target());
			ScreenQuad::draw(bufHalf.target(), 0, 0, bufHalf.width(), bufHalf.height());

			pass(mat, cPassId).postRender();

			frameBuf.end();
		}

		{	// vertical blur pass
			const int cPassId = BLUR_PASS;
			
			RenderBufferSet& frameBuf = bufHalf;
			frameBuf.begin(BUFFER0);

			TextureProperty* texProp = pass(mat, cPassId).textureProp(0);
			if(texProp)
				texProp->texture = bufHalf.bufferInfo(BUFFER1).texture();
			pass(mat, cPassId).preRender();

			// bind shader uniform
			if(ShaderProgram* program = ShaderProgram::current()) {
				program->uniform2fv( "g_blurOffset", BLUR_KERNEL_SIZE, m_vblurOffset.data() );
				program->uniform1fv( "g_blurKernel", BLUR_KERNEL_SIZE, m_blurKernel.data() );
				program->uniform2f ( "g_InvTexSize", 1.0f / bufHalf.width(), 1.0f / bufHalf.height() );
			}

			// draw quad
			//drawViewportQuad(0, 0, bufHalf.width(), bufHalf.height(), bufHalf.target());
			ScreenQuad::draw(bufHalf.target(), 0, 0, bufHalf.width(), bufHalf.height());

			pass(mat, cPassId).postRender();

			frameBuf.end();
		}

		{	// copy to screen
			const int cPassId = COPY_PASS;

			TextureProperty* texProp = pass(mat, cPassId).textureProp(0);
			if(texProp)
				texProp->texture = bufFull.bufferInfo(BUFFER0).texture();
			pass(mat, cPassId).preRender();

			// draw quad
			//drawViewportQuad(0, 0, this->width(), this->height(), bufFull.target());
			ScreenQuad::draw(bufFull.target(), 0, 0, width(), height());

			pass(mat, cPassId).postRender();
		}

		{	// radial glow
			const int cPassId = RADIAL_GLOW_PASS;

			TextureProperty* texProp = pass(mat, cPassId).textureProp(0);
			if(texProp)
				texProp->texture = bufHalf.bufferInfo(BUFFER0).texture();
			pass(mat, cPassId).preRender();
			
			// bind shader uniform
			if(ShaderProgram* program = ShaderProgram::current())
				program->uniform3fv("g_sunPos", 1, m_sunPos.data);

			// draw quad
			// preserve transforms since we need gl_ModelViewProjectionMatrox
			//drawViewportQuad(0, 0, this->width(), this->height(), bufHalf.target(), true);
			ScreenQuad::draw(bufHalf.target(), 0, 0, width(), height());

			pass(mat, cPassId).postRender();
		}

		if(0)
		{	// show the depth buffer
			const int cPassId = DEPTH_TEXTURE_PASS;

			pass(mat, cPassId).textureProp(0)->texture = bufFull.depthBufferInfo().texture();
			pass(mat, cPassId).preRender();
			
			// draw quad
			//drawViewportQuad(0, 0, this->width(), this->height(), GL_TEXTURE_RECTANGLE_ARB);
			ScreenQuad::draw(GL_TEXTURE_RECTANGLE_ARB, 0, 0, width(), height());

			pass(mat, cPassId).postRender();
		}

		//drawSunPos();
	}

	void drawScene(Material* mat)
	{
		{
			//ScopePassBinding bindPass(*mat, SCENE_PASS);
			const int cPassId = SCENE_PASS;
			pass(mat, cPassId).preRender();

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

			pass(mat, cPassId).postRender();
		}

		{
			//ScopePassBinding bindPass(*mat, SKYBOX_PASS);
			const int cPassId = SKYBOX_PASS;
			pass(mat, cPassId).preRender();

			Vec3f frustVertex[8];
			mCamera.frustum.computeVertex(frustVertex);
			
			Mat44f mtx;
			mCamera.computeView(mtx.getPtr());
			mtx = mtx.inverse();

			Vec3f p[4];
			for(size_t i = 0; i < 4; ++i)
			{
				p[i] = frustVertex[i] + (frustVertex[i+4] - frustVertex[i]) * 0.5f;
				mtx.transformPoint(p[i]);
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

			pass(mat, cPassId).postRender();
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

	std::auto_ptr<RenderBufferSet> mBuffersFull;
	std::auto_ptr<RenderBufferSet> mBuffersHalf;

};	// TestWindow

}	// namespace PostProcessingTest

TEST(PostProcessingTest)
{
	PostProcessingTest::TestWindow window;
	window.mainLoop();
}
