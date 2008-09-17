#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Shader.h"
#include "../../MCD/Render/ShaderProgram.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

TEST(SSAOTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=SSAOTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mUseSSAO(true), mAngle(0), mResourceManager(L"./Media/")
		{
		}

		bool init()
		{
			// Load the shaders synchronously
			ShaderPtr vs = dynamic_cast<Shader*>(mResourceManager.load(L"Shader/SSAO/SSAO.glvs", true).get());
			ShaderPtr ps = dynamic_cast<Shader*>(mResourceManager.load(L"Shader/SSAO/SSAOv1.glps", true).get());

			while(true) {
				int result = mResourceManager.processLoadingEvents();
				if(result < 0)
					return false;
				else if(result == 0)
					break;
			}

			if(!vs || !ps)
				return false;

			mShaderProgram.create();
			mShaderProgram.attach(*vs);
			mShaderProgram.attach(*ps);
			if(!mShaderProgram.link()) {
				std::string log;
				mShaderProgram.getLog(log);
				std::cout << log << std::endl;
			}

			return true;
		}

		sal_override void onEvent(const Event& e)
		{
			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F1)
				mUseSSAO = !mUseSSAO;

			BasicGlWindow::onEvent(e);
		}

		sal_override void onResize(size_t width, size_t height)
		{
			// Setup the render target
			mRenderTarget.reset(new RenderTarget(width, height));
			TextureRenderBufferPtr textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT0_EXT);
			textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_RGB);
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mColorRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);
			textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_COMPONENT);
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mDepthRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			if(!mRenderTarget->checkCompleteness())
				throw std::runtime_error("");

			BasicGlWindow::onResize(width, height);
		}

		void drawScene()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glTranslatef(0.0f, 0.0f, -50.0f);

			const float scale = 0.5f;
			glScalef(scale, scale, scale);

			mModel->draw();
		}

		void postProcessing()
		{
			if(!mRenderTarget.get())
				return;

			mRenderTarget->unbind();

			glActiveTexture(GL_TEXTURE0);
			mColorRenderTexture->bind();

			glActiveTexture(GL_TEXTURE1);
			mDepthRenderTexture->bind();

			if(mUseSSAO) {
				mShaderProgram.bind();

				// Setup the uniform variables
				// Reference: http://www.lighthouse3d.com/opengl/glsl/index.php?ogluniform
				{	int texColor = glGetUniformLocation(mShaderProgram.handle(), "texColor");
					glUniform1i(texColor, 0);

					int texDepth = glGetUniformLocation(mShaderProgram.handle(), "texDepth");
					glUniform1i(texDepth, 1);
				}
			}

			drawViewportQuad(0, 0, width(), height(), mColorRenderTexture->type());
			mShaderProgram.unbind();

			glActiveTexture(GL_TEXTURE0);
			glDisable(GL_TEXTURE_RECTANGLE_ARB);
		}

		void load3ds(const wchar_t* fileId)
		{
			mModel = dynamic_cast<Model*>(mResourceManager.load(fileId).get());
		}

		sal_override void update(float deltaTime)
		{
			(void)deltaTime;
			mResourceManager.processLoadingEvents();

			if(mUseSSAO && mRenderTarget.get())
				mRenderTarget->bind();

			drawScene();

			if(mUseSSAO)
				postProcessing();
		}

		bool mUseSSAO;
		float mAngle;

		ModelPtr mModel;
		ShaderProgram mShaderProgram;

		DefaultResourceManager mResourceManager;

		TexturePtr mColorRenderTexture, mDepthRenderTexture;
		std::auto_ptr<RenderTarget> mRenderTarget;
	};	// TestWindow

	{
		TestWindow window;

		if(!window.init()) {
			CHECK(false);
			return;
		}

//		window.load3ds(L"titanic.3DS");
//		window.load3ds(L"titanic2.3DS");
//		window.load3ds(L"spaceship.3DS");
		window.load3ds(L"TextureBoxSphere.3ds");
//		window.load3ds(L"ship^kiy.3ds");
//		window.load3ds(L"Alfa Romeo.3ds");
//		window.load3ds(L"Nissan350Z.3ds");
//		window.load3ds(L"Nathalie aguilera Boing 747.3DS");
//		window.load3ds(L"Dog 1 N280708.3ds");
//		window.load3ds(L"Leon N300708.3DS");
//		window.load3ds(L"Ford N120208.3ds");
//		window.load3ds(L"Wolf359a/Star wars/awing/awing.3DS");
//		window.load3ds(L"F40/F40_L.3DS");
//		window.load3ds(L"F5E/f5e_05.3ds");
//		window.load3ds(L"city/city.3ds");
//		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//		window.load3ds(L"House/house.3ds");
//		window.load3ds(L"FockeWulf 189A/fw189.3ds");
//		window.load3ds(L"MclarenF1/MclarenF1.3ds");
//		window.load3ds(L"benetton/Benetton 2001.3ds");
		window.load3ds(L"3M00696/buelllightning.3DS");

		window.mainLoop();
	}
}
