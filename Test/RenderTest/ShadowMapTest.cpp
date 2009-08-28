#include "Pch.h"
#include "Common.h"	// For drawFrustum()
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/ProjectiveTexture.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "../../MCD/Core/Math/Mat44.h"
#include <memory>	// For auto_ptr

using namespace MCD;

namespace {

class TestWindow : public BasicGlWindow
{
public:
	TestWindow(const wchar_t* options)
		:
		BasicGlWindow(options),
		mResourceManager(*createDefaultFileSystem())
	{
		mShadowMapProjection.camera.frustum.projectionType = Frustum::Ortho;
		mShadowMapProjection.camera.frustum.create(-100, 100, -100, 100, -100, 600);
		mShadowMapProjection.camera = Camera(Vec3f(200, 200, 200), Vec3f(2, 0, 0), Vec3f::c010);

		mShadowMapSize = 2048;

		mShadowMapRendered = 0;

		if(!setupRenderTarget())
			throw std::runtime_error("Fail to setup render target");
	}

	bool setupRenderTarget()
	{
		// Setup the render target with the desired shadow map size
		mRenderTarget.reset(new RenderTarget(mShadowMapSize, mShadowMapSize));

		// Setup for the depth buffer
		TextureRenderBufferPtr textureBuffer = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);
		if(!textureBuffer->createTexture(mShadowMapSize, mShadowMapSize, GL_TEXTURE_2D, GL_DEPTH_COMPONENT))
			return false;
		if(!textureBuffer->linkTo(*mRenderTarget))
			return false;
		mShadowMapProjection.texture = textureBuffer->texture;

		{	// Setup the texture parameters for the shadow map
			mShadowMapProjection.texture->bind();

			glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

			// NOTE: Not all display card suppport this
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FAIL_VALUE_ARB, 0.5f);

			// Depth map clamp behaviour, making the border with white color so
			// no shadow will be applied outside the light frustum.
			float clampColor[] = { 1, 1, 1, 1 };
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

			mShadowMapProjection.texture->unbind();
		}

		mRenderTarget->bind();
		// We only have the depth texture, no color attachment
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		mRenderTarget->unbind();

		if(!mRenderTarget->checkCompleteness())
			return false;

		return true;
	}

	void drawShadowMap()
	{
		// Render the shadow map only every 10 frames
		++mShadowMapRendered;
		if(mShadowMapRendered % 10 != 0)
			return;

		mRenderTarget->bind();

		glColorMask(0, 0, 0, 0);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(2.0f, 0.0f);
		glShadeModel(GL_FLAT);

		// Set the current Viewport to the shadow map size
		glViewport(0, 0, mShadowMapSize, mShadowMapSize);

		glClear(GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		mShadowMapProjection.camera.applyTransform();
		drawScene();

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		// Restore the original viewport
		glViewport(0, 0, width(), height());

		glColorMask(1, 1, 1, 1);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glShadeModel(GL_SMOOTH);

		mRenderTarget->unbind();
	}

	void load3ds(const wchar_t* fileId)
	{
		mModel = dynamic_cast<Model*>(mResourceManager.load(fileId).get());
	}

	void drawScene()
	{
		const float scale = 0.5f;
		glScalef(scale, scale, scale);

		mModel->draw();
	}

	void debugShadowMap()
	{
		size_t w = 512;//width()/4;
		size_t h = 512;//height()/4;
		glViewport(0, 0, w, h);
		mShadowMapProjection.texture->bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		drawViewportQuad(0, 0, w, h, GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		mShadowMapProjection.texture->unbind();
		glViewport(0, 0, width(), height());
	}

	void drawFrustum()
	{
		Mat44f transform;
		mShadowMapProjection.camera.computeView(transform.getPtr());
		transform = transform.inverse();

		// TODO: There is no effect on calling glColor3f()
		glColor3f(1, 1, 0);
		glPushMatrix();
		glMultTransposeMatrixf(transform.getPtr());
		::drawFrustum(mShadowMapProjection.camera.frustum);
		glPopMatrix();
		glColor3f(1, 1, 1);
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		drawShadowMap();

		drawFrustum();

		mShadowMapProjection.bind(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
		drawScene();
		mShadowMapProjection.unbind();

		debugShadowMap();
	}

	ModelPtr mModel;
	DefaultResourceManager mResourceManager;
	std::auto_ptr<RenderTarget> mRenderTarget;

	size_t mShadowMapSize;
	size_t mShadowMapRendered;
	ProjectiveTexture mShadowMapProjection;
};	// TestWindow

}	// namespace

TEST(ShadowMapTest)
{
	TestWindow window(L"title=ShadowMapTest;width=800;height=600;fullscreen=0;FSAA=4");
	window.load3ds(L"Scene/City/scene.3ds");
	window.mainLoop();
	CHECK(true);
}

// Testing shadow map with shader

#include "../../MCD/Render/Shader.h"
#include "../../MCD/Render/ShaderProgram.h"

namespace {

class ShaderTestWindow : public TestWindow
{
public:
	ShaderTestWindow(const wchar_t* options) : TestWindow(options)
	{
		if(!loadShaderProgram(
			L"Shader/ShadowMap/Scene.glvs", L"Shader/ShadowMap/Scene.glps",
			mShadowMapShader, mResourceManager))
		{
			throw std::runtime_error("Fail to load shader");
		}

        if(!loadShaderProgram(
			L"Shader/ShadowMap/DebugShadowMap.glvs", L"Shader/ShadowMap/DebugShadowMap.glps",
			mDebugShadowMapShader, mResourceManager))
		{
			throw std::runtime_error("Fail to load shader");
		}

		mShadowMapShader.bind();
		int colorTex = glGetUniformLocation(mShadowMapShader.handle, "colorTex");
		glUniform1i(colorTex, 0);

		int shadowTex = glGetUniformLocation(mShadowMapShader.handle, "shadowTex");
		glUniform1i(shadowTex, 1);
		mShadowMapShader.unbind();

        mDebugShadowMapShader.bind();
        shadowTex = glGetUniformLocation(mDebugShadowMapShader.handle, "shadowTex");
		glUniform1i(shadowTex, 0);
		mDebugShadowMapShader.unbind();
	}

	sal_override void update(float deltaTime)
	{
		mResourceManager.processLoadingEvents();

		drawShadowMap();

		drawFrustum();

		Mat44f cameraViewInverse;
		mCamera.computeView(cameraViewInverse.getPtr());
		cameraViewInverse = cameraViewInverse.inverse();
		mShadowMapProjection.bind(GL_TEXTURE1, cameraViewInverse.getPtr());

		glActiveTexture(GL_TEXTURE0);

		mShadowMapShader.bind();
		drawScene();
		mShadowMapShader.unbind();

		mShadowMapProjection.unbind();

        mDebugShadowMapShader.bind();
		debugShadowMap();
        mDebugShadowMapShader.unbind();
	}

	ShaderProgram mShadowMapShader;
    ShaderProgram mDebugShadowMapShader;
};	// ShaderTestWindow

}	// namespace

TEST(ShadowMapShaderTest)
{
	ShaderTestWindow window(L"title=ShadowMapShaderTest;width=800;height=600;fullscreen=0;FSAA=4");
	window.load3ds(L"Scene/City/scene.3ds");
	window.mainLoop();
	CHECK(true);
}
