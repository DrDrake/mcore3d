#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Color.h"
#include "../../MCD/Render/Renderable.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Shader.h"
#include "../../MCD/Render/ShaderProgram.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "../../MCD/Core/System/WindowEvent.h"

using namespace MCD;

static void minizeEnergy(float* data, size_t sampleCount, size_t stride=sizeof(float)*3)
{
	stride /= sizeof(float);
	int iter = 100;
	while(iter--)
	{
		for(size_t i=0; i<sampleCount; ++i)
		{
			Vec3f force;
			Vec3f res(0.0f);
			Vec3f vec;
			float fac;

			Vec3f& sampleI = reinterpret_cast<Vec3f&>(data[i*stride]);
			sampleI.normalize();
			vec = sampleI;
			// Minimize with other samples
			for(size_t j=0; j<sampleCount; ++j)
			{
				Vec3f& sampleJ = reinterpret_cast<Vec3f&>(data[j*stride]);
				force = vec - sampleJ;
				if((fac = force.norm())!= 0.0f )
					res += force / fac;
			}

			sampleI += res * 0.5f;
			sampleI.normalize();
		}
	}

	for(size_t i=0; i<sampleCount; ++i)
	{
		Vec3f& sample = reinterpret_cast<Vec3f&>(data[i*stride]);
		sample *= float(rand()) / RAND_MAX;
	}
}

static TexturePtr generateRandomTexture(uint textureSize)
{
	TexturePtr texture = new Texture(L"");
	texture->width = textureSize;
	texture->height = textureSize;
	texture->type = GL_TEXTURE_2D;
	texture->format = GL_RGBA;
	glGenTextures(1, &texture->handle);


	ColorRGBA8* buffer = new ColorRGBA8[textureSize * textureSize];

	for(size_t i=0; i<textureSize * textureSize; ++i) {
		buffer[i].r = rand() % 256;
		buffer[i].g = rand() % 256;
		buffer[i].b = rand() % 256;
		buffer[i].a = rand() % 256;
	}

	texture->bind();
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, texture->format, textureSize, textureSize,
		0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	delete[] buffer;

	return texture;
}

TEST(SSAOTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=SSAOTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mUseSSAO(true), mShowTexture(false),
			mSSAORescale(0.5f), mSSAORadius(0.5f), mBlurPassCount(2),
			mRenderable(nullptr), mResourceManager(*createDefaultFileSystem())
		{
			if( !loadShaderProgram(L"Shader/SSAO/Scene.glvs", L"Shader/SSAO/Scene.glps", mScenePass, mResourceManager) ||
				!loadShaderProgram(L"Shader/SSAO/SSAO.glvs", L"Shader/SSAO/SSAO.glps", mSSAOPass, mResourceManager) ||
				!loadShaderProgram(L"Shader/SSAO/Blur.glvs", L"Shader/SSAO/Blur.glps", mBlurPass, mResourceManager) ||
				!loadShaderProgram(L"Shader/SSAO/Combine.glvs", L"Shader/SSAO/Combine.glps", mCombinePass, mResourceManager))
			{
				throw std::runtime_error("Fail to load shader");
			}

			// Load the random normal map
			mDitherTexture = generateRandomTexture(32);
		}

		// Update the view frustrum related uniform variables.
		void updateViewFrustum()
		{
			float projectionMatrix[4*4];

			glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

			mSSAOPass.bind();

			glUniform1f(glGetUniformLocation(mSSAOPass.handle, "fov"), fieldOfView() * Mathf::cPi() / 180);

			glUniformMatrix4fvARB(glGetUniformLocation(mSSAOPass.handle, "projection"), 1, false, projectionMatrix);
		}

		sal_override void onEvent(const Event& e)
		{
			// Toggle SSAO
			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F1)
				mUseSSAO = !mUseSSAO;

			// Toggle half-size SSAO rendering
			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F2) {
				mSSAORescale = mSSAORescale == 0.5f ? 1.0f : 0.5f;
				onResize(width(), height());
			}

			// Adjust SSAO blur
			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F3) {
				if(e.Key.Shift) {
					if(mBlurPassCount > 0)
						--mBlurPassCount;
				} else
					++mBlurPassCount;
			}

			// Toggle texturing
			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F4) {
				mShowTexture = !mShowTexture;
				mScenePass.bind();
				glUniform1i(glGetUniformLocation(mScenePass.handle, "showTexture"), mShowTexture);
				mScenePass.unbind();
			}

			// Adjust SSAO radius
			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F5) {
				if(e.Key.Shift)
					mSSAORadius *= 1.05f;
				else
					mSSAORadius /= 1.05f;
				mSSAOPass.bind();
				glUniform1f(glGetUniformLocation(mSSAOPass.handle, "radius"), mSSAORadius);
				mSSAOPass.unbind();
			}

			BasicGlWindow::onEvent(e);

			if(e.Type == Event::MouseWheelMoved)
				updateViewFrustum();
		}

		sal_override void onResize(size_t width, size_t height)
		{
			// If the window is minmized
			if(width == 0 || height == 0)
				return;

			// Setup scene's render target, where the scene will output the color, depth and normal
			mSceneRenderTarget.reset(new RenderTarget(width, height));
			TextureRenderBufferPtr textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT0_EXT);
			if(!textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mSceneRenderTarget))
				throw std::runtime_error("");
			mColorRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT1_EXT);
			if(!textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mSceneRenderTarget))
				throw std::runtime_error("");
			mNormalRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_DEPTH_ATTACHMENT_EXT);
			if(!textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, GL_DEPTH_COMPONENT))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mSceneRenderTarget))
				throw std::runtime_error("");
			mDepthRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			{	// Adjust the clamp mode of the depth texture to be clamp to border
				mDepthRenderTexture->bind();
				float b[] = { 1, 1, 1 };	// Everything outside the border is defined as far away
				glTexParameterfv(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_BORDER_COLOR, b);
				glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			}

			mSceneRenderTarget->bind();
			GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
			glDrawBuffers(2, buffers);
			mSceneRenderTarget->unbind();

			if(!mSceneRenderTarget->checkCompleteness())
				throw std::runtime_error("");

			// Setup SSAO's render target, outputing dithered SSAO result
			mSSAORenderTarget.reset(new RenderTarget(size_t(width * mSSAORescale), size_t(height * mSSAORescale)));
			textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT0_EXT);
			if(!textureBuffer->createTexture(mSSAORenderTarget->width(), mSSAORenderTarget->height(), GL_TEXTURE_RECTANGLE_ARB, GL_RGBA))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mSSAORenderTarget))
				throw std::runtime_error("");
			mSSAORenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT1_EXT);
			if(!textureBuffer->createTexture(mSSAORenderTarget->width(), mSSAORenderTarget->height(), GL_TEXTURE_RECTANGLE_ARB, GL_RGBA))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mSSAORenderTarget))
				throw std::runtime_error("");
			mSSAORenderTexture2 = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			mSSAORenderTarget->bind();
			glDrawBuffers(1, buffers);
			mSSAORenderTarget->unbind();

			if(!mSSAORenderTarget->checkCompleteness())
				throw std::runtime_error("");

			// Setup the uniform variables
			// Reference: http://www.lighthouse3d.com/opengl/glsl/index.php?ogluniform
			// TODO: Setup the camera near and far plane.
			{	mSSAOPass.bind();

				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "texDepth"), 0);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "texNormal"), 1);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "texDither"), 2);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "screenWidth"), width);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "screenHeight"), height);
				glUniform1f(glGetUniformLocation(mSSAOPass.handle, "radius"), mSSAORadius);
				glUniform1f(glGetUniformLocation(mSSAOPass.handle, "ssaoRescale"), mSSAORescale);

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

				minizeEnergy((float*)randSphere, sizeof(randSphere) / (3 * sizeof(float)));
				glUniform3fv(glGetUniformLocation(mSSAOPass.handle, "ranSphere"), 32, (float*)randSphere);

				mSSAOPass.unbind();

				mBlurPass.bind();
				glUniform1i(glGetUniformLocation(mBlurPass.handle, "texSSAO"), 0);
				mBlurPass.unbind();

				mCombinePass.bind();

				glUniform1i(glGetUniformLocation(mCombinePass.handle, "texColor"), 0);
				glUniform1i(glGetUniformLocation(mCombinePass.handle, "texSSAO"), 1);
				glUniform1i(glGetUniformLocation(mCombinePass.handle, "texDepth"), 2);
				glUniform1f(glGetUniformLocation(mCombinePass.handle, "ssaoRescale"), mSSAORescale);

				mCombinePass.unbind();
			}

			BasicGlWindow::onResize(width, height);
		}

		void drawScene()
		{
			MCD_ASSERT(mSceneRenderTarget.get());
			if(mUseSSAO)
				mSceneRenderTarget->bind();

			updateViewFrustum();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glTranslatef(0.0f, 0.0f, -50.0f);

			const float scale = 1.2f;
			glScalef(scale, scale, scale);

			mScenePass.bind();
			glActiveTexture(GL_TEXTURE0);
			glDisable(GL_TEXTURE_RECTANGLE_ARB);
			mRenderable->draw();
			mScenePass.unbind();

			if(mUseSSAO)
				mSceneRenderTarget->unbind();
		}

		void postProcessing()
		{
			if(!mUseSSAO)
				return;

			// Render the SSAO
			MCD_ASSERT(mSSAORenderTarget.get());
			mSSAORenderTarget->bind();
			GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
			glDrawBuffers(1, buffers + 0);	// Effectively output to mSSAORenderTexture

			glActiveTexture(GL_TEXTURE0);
			mDepthRenderTexture->bind();

			glActiveTexture(GL_TEXTURE1);
			mNormalRenderTexture->bind();

			glActiveTexture(GL_TEXTURE2);
			mDitherTexture->bind();

			mSSAOPass.bind();
			drawViewportQuad(0, 0, mSSAORenderTexture->width, mSSAORenderTexture->height, mSSAORenderTexture->type);
			mSSAOPass.unbind();

			// Blurr the SSAO
			if(mBlurPassCount > 0)
			{
				mBlurPass.bind();

				for(size_t i=0; i<mBlurPassCount; ++i)
				{
					glDrawBuffers(1, buffers + 1);	// Effectively output to mSSAORenderTexture2
					glActiveTexture(GL_TEXTURE0);
					mSSAORenderTexture->bind();
					glUniform2f(glGetUniformLocation(mBlurPass.handle, "blurDirection"), 1, 0);
					drawViewportQuad(0, 0, mSSAORenderTexture->width, mSSAORenderTexture->height, mSSAORenderTexture->type);

					glDrawBuffers(1, buffers + 0);	// Effectively output to mSSAORenderTexture
					glActiveTexture(GL_TEXTURE0);
					mSSAORenderTexture2->bind();
					glUniform2f(glGetUniformLocation(mBlurPass.handle, "blurDirection"), 0, 1);
					drawViewportQuad(0, 0, mSSAORenderTexture->width, mSSAORenderTexture->height, mSSAORenderTexture->type);
				}

				mBlurPass.unbind();
			}

			mSSAORenderTarget->unbind();

			// Final pass that combine the color and the SSAO
			glActiveTexture(GL_TEXTURE0);
			mColorRenderTexture->bind();

			glActiveTexture(GL_TEXTURE1);
			mSSAORenderTexture->bind();

			glActiveTexture(GL_TEXTURE2);
			mDepthRenderTexture->bind();

			mCombinePass.bind();
			drawViewportQuad(0, 0, width(), height(), mSSAORenderTexture->type);
			mCombinePass.unbind();
		}

		void loadModel(const wchar_t* fileId)
		{
			mModel = mResourceManager.load(fileId).get();
			mRenderable = dynamic_cast<IRenderable*>(mModel.get());
		}

		sal_override void update(float deltaTime)
		{
			(void)deltaTime;
			mResourceManager.processLoadingEvents();

			if(!mSceneRenderTarget.get() || !mSSAORenderTarget.get())
				return;

			drawScene();

			postProcessing();
		}

		bool mUseSSAO;
		bool mShowTexture;
		float mSSAORescale;
		float mSSAORadius;
		size_t mBlurPassCount;

		ResourcePtr mModel;
		IRenderable* mRenderable;
		ShaderProgram mScenePass, mSSAOPass, mBlurPass, mCombinePass;

		DefaultResourceManager mResourceManager;

		TexturePtr mColorRenderTexture, mNormalRenderTexture, mDepthRenderTexture, mDitherTexture, mSSAORenderTexture, mSSAORenderTexture2;
		std::auto_ptr<RenderTarget> mSceneRenderTarget, mSSAORenderTarget;
	};	// TestWindow

	{
		TestWindow window;

		window.loadModel(L"ANDOX.3DS");
//		window.loadModel(L"TextureBoxSphere.3ds");
//		window.loadModel(L"House/house.3ds");
//		window.loadModel(L"City/city.3ds");
//		window.loadModel(L"Scene/01/scene.3ds");
//		window.loadModel(L"Scene/01/scene.pod");
//		window.loadModel(L"Church/sponza/sponza.3ds");
//		window.loadModel(L"3M00696/buelllightning.3DS");
//		window.loadModel(L"Lamborghini Gallardo Polizia/Lamborghini Gallardo Polizia.3DS");

		window.mainLoop();
	}
}
