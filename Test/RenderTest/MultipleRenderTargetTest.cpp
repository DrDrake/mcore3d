#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Shader.h"
#include "../../MCD/Render/ShaderProgram.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"

using namespace MCD;

TEST(MultipleRenderTargetTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=MultipleRenderTargetTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(*createDefaultFileSystem())
		{
		}

		bool initShaderProgram(const wchar_t* vsSource, const wchar_t* psSource, ShaderProgram& shaderProgram)
		{
			// Load the shaders synchronously
			ShaderPtr vs = dynamic_cast<Shader*>(mResourceManager.load(vsSource, true).get());
			ShaderPtr ps = dynamic_cast<Shader*>(mResourceManager.load(psSource, true).get());

			while(true) {
				int result = mResourceManager.processLoadingEvents();
				if(result < 0)
					return false;
				else if(result == 0)
					break;
			}

			if(!vs || !ps)
				return false;

			shaderProgram.create();
			shaderProgram.attach(*vs);
			shaderProgram.attach(*ps);
			if(!shaderProgram.link()) {
				std::string log;
				shaderProgram.getLog(log);
				std::cout << log << std::endl;
			}

			return true;
		}

		sal_override void onResize(size_t width, size_t height)
		{
			{	// We need at least 3 buffers
				GLint maxBuffers;
				glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &maxBuffers);
				if(maxBuffers < 4)
					throw std::runtime_error("");
			}

			// We use a smaller render target size than the actual window size
			size_t targetWidth = width / 2;
			size_t targetHeight = height / 2;

			// Setup the render target
			mRenderTarget.reset(new RenderTarget(targetWidth, targetHeight));
			TextureRenderBufferPtr textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT0_EXT);
			if(!textureBuffer->createTexture(targetWidth, targetHeight, GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mColorRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT1_EXT);
			if(!textureBuffer->createTexture(targetWidth, targetHeight, GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mNormalRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);
			if(!textureBuffer->createTexture(targetWidth, targetHeight, GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_COMPONENT))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mDepthRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			mRenderTarget->bind();
			GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
			glDrawBuffers(2, buffers);

			if(!mRenderTarget->checkCompleteness())
				throw std::runtime_error("");

			BasicGlWindow::onResize(width, height);
		}

		void drawScene()
		{
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_RECTANGLE_ARB);
			glViewport(0, 0, width()/2, height()/2);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glTranslatef(0.0f, 0.0f, -50.0f);

			const float scale = 0.5f;
			glScalef(scale, scale, scale);

			mScenePass.bind();
			mModel->draw();
			mScenePass.unbind();
		}

		void postProcessing()
		{
			if(!mRenderTarget.get())
				return;

			mRenderTarget->unbind();

			glEnable(GL_TEXTURE_RECTANGLE_ARB);
			glDisable(GL_TEXTURE_2D);

			size_t w = width() / 2;
			size_t h = height() / 2;

			mColorRenderTexture->bind();
			drawViewportQuad(0, h, w, h, mColorRenderTexture->type());

			mDepthPass.bind();
			mDepthRenderTexture->bind();
			drawViewportQuad(w, h, w, h, mDepthRenderTexture->type());
			mDepthPass.unbind();

			mNormalRenderTexture->bind();
			drawViewportQuad(w, 0, w, h, mNormalRenderTexture->type());
		}

		void load3ds(const wchar_t* fileId)
		{
			mModel = dynamic_cast<Model*>(mResourceManager.load(fileId).get());
		}

		sal_override void update(float deltaTime)
		{
			(void)deltaTime;
			mResourceManager.processLoadingEvents();

			if(mRenderTarget.get())
				mRenderTarget->bind();

			drawScene();

			postProcessing();
		}

		ModelPtr mModel;
		ShaderProgram mScenePass, mDepthPass;

		DefaultResourceManager mResourceManager;

		TexturePtr mColorRenderTexture, mDepthRenderTexture, mNormalRenderTexture;
		std::auto_ptr<RenderTarget> mRenderTarget;
	};	// TestWindow

	{
		TestWindow window;

		if(!window.initShaderProgram(L"Shader/MRT/Scene.glvs", L"Shader/MRT/Scene.glps", window.mScenePass) ||
		   !window.initShaderProgram(L"Shader/MRT/Depth.glvs", L"Shader/MRT/Depth.glps", window.mDepthPass))
		{
			CHECK(false);
			return;
		}

		window.load3ds(L"TextureBoxSphere.3DS");
		window.load3ds(L"3M00696/buelllightning.3DS");
//		window.load3ds(L"city/city.3ds");

		window.mainLoop();
	}
}
