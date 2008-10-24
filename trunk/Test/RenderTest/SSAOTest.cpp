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
			if( !loadShaderProgram(L"Shader/SSAO/Scene.glvs", L"Shader/SSAO/Scene.glps", mScenePass, mResourceManager) ||
				!loadShaderProgram(L"Shader/SSAO/SSAO.glvs", L"Shader/SSAO/SSAO.glps", mSSAOPass, mResourceManager))
			{
				throw std::runtime_error("Fail to load shader");
			}

			// Load the random normal map
			mDitherTexture = dynamic_cast<Texture*>(mResourceManager.load(L"RandomNormals128x128.png", true).get());
		}

		// Update the view frustrum related uniform variables.
		void updateViewFrustum()
		{
			float projectionMatrix[4*4];

			glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

			mSSAOPass.bind();

			glUniform1f(glGetUniformLocation(mSSAOPass.handle(), "fov"), fieldOfView() * Mathf::cPi() / 180);

			glUniformMatrix4fvARB(glGetUniformLocation(mSSAOPass.handle(), "projection"), 1, false, projectionMatrix);
		}

		sal_override void onEvent(const Event& e)
		{
			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F1)
				mUseSSAO = !mUseSSAO;

			BasicGlWindow::onEvent(e);

			if(e.Type == Event::MouseWheelMoved)
				updateViewFrustum();
		}

		sal_override void onResize(size_t width, size_t height)
		{
			// If the window is minized
			if(width == 0 || height == 0)
				return;

			// Setup the render target
			mRenderTarget.reset(new RenderTarget(width, height));
			TextureRenderBufferPtr textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT0_EXT);
			if(!textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mColorRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT1_EXT);
			if(!textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mNormalRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);
			if(!textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_COMPONENT))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mDepthRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			mRenderTarget->bind();
			GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
			glDrawBuffers(2, buffers);

			if(!mRenderTarget->checkCompleteness())
				throw std::runtime_error("");

			mSSAOPass.bind();

			// Setup the uniform variables
			// Reference: http://www.lighthouse3d.com/opengl/glsl/index.php?ogluniform
			// TODO: Setup the camera near and far plane.
			{	int texColor = glGetUniformLocation(mSSAOPass.handle(), "texColor");
				glUniform1i(texColor, 0);

				int texDepth = glGetUniformLocation(mSSAOPass.handle(), "texDepth");
				glUniform1i(texDepth, 1);

				int texNormal = glGetUniformLocation(mSSAOPass.handle(), "texNormal");
				glUniform1i(texNormal, 2);

				int texDither = glGetUniformLocation(mSSAOPass.handle(), "texDither");
				glUniform1i(texDither, 3);

				int screenWidth = glGetUniformLocation(mSSAOPass.handle(), "screenWidth");
				glUniform1i(screenWidth, width);

				int screenHeight = glGetUniformLocation(mSSAOPass.handle(), "screenHeight");
				glUniform1i(screenHeight, height);

				// Random points INSIDE an unit sphere.
				const float randSphere[] = {
					 0.000005f,  0.078660f,  0.451855f,
					 0.175578f, -0.358549f,  0.114750f,
					-0.058036f,  0.000666f,  0.033155f,
					-0.110033f, -0.019899f,  0.308601f,
					 0.680572f,  0.103066f, -0.013045f,
					 0.141117f, -0.225826f, -0.396631f,
					-0.078870f,  0.039488f, -0.021275f,
					-0.382385f, -0.614163f, -0.430833f,

					 0.625820f,  0.257202f,  0.055261f,
					 0.124728f, -0.108621f,  0.066673f,
					-0.389841f,  0.641297f,  0.549976f,
					-0.165321f, -0.197665f,  0.258978f,
					 0.484174f,  0.351062f, -0.410552f,
					 0.077469f, -0.089567f, -0.117051f,
					-0.338855f,  0.026368f, -0.179018f,
					-0.027324f, -0.036386f, -0.122232f,

					 0.289819f,  0.398782f,  0.163619f,
					 0.005096f, -0.018950f,  0.026599f,
					-0.552742f,  0.344206f,  0.060074f,
					-0.149550f, -0.594806f,  0.437477f,
					 0.225526f,  0.200505f, -0.052620f,
					 0.021552f, -0.039765f, -0.040277f,
					-0.117847f,  0.388661f, -0.225310f,
					-0.130309f, -0.393243f, -0.004296f,

					 0.528152f,  0.021974f,  0.033980f,
					 0.332483f, -0.310106f,  0.594333f,
					-0.259461f,  0.437335f,  0.567771f,
					-0.633172f, -0.070639f,  0.613848f,
					 0.346119f,  0.506114f, -0.582868f,
					 0.249371f, -0.139074f, -0.142318f,
					-0.598198f,  0.031839f, -0.484108f,
					-0.005480f, -0.144742f, -0.191251f,
				};
				int ranSphere = glGetUniformLocation(mSSAOPass.handle(), "ranSphere");
				glUniform3fv(ranSphere, 32, (float*)randSphere);
			}

			mSSAOPass.unbind();

			BasicGlWindow::onResize(width, height);
		}

		void drawScene()
		{
			updateViewFrustum();

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

			glActiveTexture(GL_TEXTURE0);
			mColorRenderTexture->bind();

			glActiveTexture(GL_TEXTURE1);
			mDepthRenderTexture->bind();

			glActiveTexture(GL_TEXTURE2);
			mNormalRenderTexture->bind();

			glActiveTexture(GL_TEXTURE3);
			mDitherTexture->bind();

			mSSAOPass.bind();
			drawViewportQuad(0, 0, width(), height(), mColorRenderTexture->type());
			mSSAOPass.unbind();

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
		ShaderProgram mScenePass, mSSAOPass;

		DefaultResourceManager mResourceManager;

		TexturePtr mColorRenderTexture, mNormalRenderTexture, mDepthRenderTexture, mDitherTexture;
		std::auto_ptr<RenderTarget> mRenderTarget;
	};	// TestWindow

	{
		TestWindow window;

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
//		window.load3ds(L"Lamborghini Gallardo Polizia/Lamborghini Gallardo Polizia.3DS");

		window.mainLoop();
	}
}
