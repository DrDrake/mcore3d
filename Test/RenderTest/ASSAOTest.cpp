#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Core/Math/Mat44.h"
#include "../../MCD/Render/Color.h"
#include "../../MCD/Render/Effect.h"
#include "../../MCD/Render/Material.h"
#include "../../MCD/Render/Renderable.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Shader.h"
#include "../../MCD/Render/ShaderProgram.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"

using namespace MCD;

static TexturePtr generateRandomTexture(uint textureSize)
{
	TexturePtr texture = new Texture(L"texture:dither");
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

		const uint8_t contrast = 10;	// The minimum value is 2
		buffer[i].a = (rand() % uint8_t(256*(contrast-1)/contrast)) + (256/contrast);
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

TEST(ASSAOTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=SSAOTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mUseSSAO(true), mBlurSSAO(true), mShowTexture(false),
			mSSAORescale(0.5f), mSSAORadius(0.5f), mBlurPassCount(0),
			mRenderable(nullptr), mResourceManager(*createDefaultFileSystem())
		{
			if( !loadShaderProgram(L"Shader/ASSAO/SSAO.glvs", L"Shader/ASSAO/SSAO.glps", mSSAOPass, mResourceManager) ||
				!loadShaderProgram(L"Shader/ASSAO/Blur.glvs", L"Shader/ASSAO/Blur.glps", mBlurPass, mResourceManager) ||
				!loadShaderProgram(L"Shader/ASSAO/Combine.glvs", L"Shader/ASSAO/Combine.glps", mCombinePass, mResourceManager))
			{
				throw std::runtime_error("Fail to load shader");
			}

			mSceneEffect = static_cast<Effect*>(mResourceManager.load(L"Material/ShowOffTest/ScenePass.fx.xml").get());
//			mSSAOEffect = static_cast<Effect*>(mResourceManager.load(L"Material/ShowOffTest/SSAOPass.fx.xml").get());

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
/*			if(e.Type == Event::KeyReleased && e.Key.Code == Key::F4) {
				mShowTexture = !mShowTexture;
				mScenePass.bind();
				glUniform1i(glGetUniformLocation(mScenePass.handle, "showTexture"), mShowTexture);
				mScenePass.unbind();
			}*/

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

		TexturePtr createRenderTexture(
			RenderTarget& renderTarget, int attachmentType, int textureType,
			size_t width, size_t height, const wchar_t* resourceName
			)
		{
			TextureRenderBufferPtr textureBuffer = new TextureRenderBuffer(attachmentType);
			if(!textureBuffer->createTexture(width, height, GL_TEXTURE_RECTANGLE_ARB, textureType, resourceName))
				return nullptr;
			if(!textureBuffer->linkTo(renderTarget))
				return nullptr;
			TexturePtr ret = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;
			mResourceManager.cache(ret);

			return ret;
		}

		// Set the texture as clamp to border
		void clampBorder(Texture& texture)
		{
			texture.bind();
			float b[] = { 1, 1, 1, 1 };
			glTexParameterfv(texture.type, GL_TEXTURE_BORDER_COLOR, b);
			glTexParameterf(texture.type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameterf(texture.type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		}

		sal_override void onResize(size_t width, size_t height)
		{
			// If the window is minmized
			if(width == 0 || height == 0)
				return;

			// Setup scene's render target, where the scene will output the color, depth and normal
			mSceneRenderTarget.reset(new RenderTarget(width, height));

			mColorRenderTexture = createRenderTexture(*mSceneRenderTarget, GL_COLOR_ATTACHMENT0_EXT, GL_RGB, width, height, L"renderBuffer:color");
			mNormalRenderTexture = createRenderTexture(*mSceneRenderTarget, GL_COLOR_ATTACHMENT1_EXT, GL_RGB, width, height, L"renderBuffer:normal");
			mDepthRenderTexture = createRenderTexture(*mSceneRenderTarget, GL_DEPTH_ATTACHMENT_EXT, GL_DEPTH_COMPONENT, width, height, L"renderBuffer:depth");

			mSceneRenderTarget->bind();
			GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
			glDrawBuffers(2, buffers);
			mSceneRenderTarget->unbind();

			if(!mSceneRenderTarget->checkCompleteness())
				throw std::runtime_error("Fail to create mSceneRenderTarget");

			// Setup SSAO's render target, outputing dithered SSAO result
			size_t width2 = size_t(width * mSSAORescale);
			size_t height2 = size_t(height * mSSAORescale);
			mSSAORenderTarget.reset(new RenderTarget(width2, height2));

			mAccum1 = createRenderTexture(*mSSAORenderTarget, GL_COLOR_ATTACHMENT0_EXT, GL_RGBA, width2, height2, L"renderBuffer:Accum1");
			mAccum2 = createRenderTexture(*mSSAORenderTarget, GL_COLOR_ATTACHMENT1_EXT, GL_RGBA, width2, height2, L"renderBuffer:Accum2");

			if(!mSSAORenderTarget->checkCompleteness())
				throw std::runtime_error("Fail to create mSSAORenderTarget");

			// Setup the clamp mode
			clampBorder(*mDepthRenderTexture);
			clampBorder(*mAccum1);
			clampBorder(*mAccum2);

			// Setup the uniform variables
			// Reference: http://www.lighthouse3d.com/opengl/glsl/index.php?ogluniform
			// TODO: Setup the camera near and far plane.
			{	mSSAOPass.bind();

				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "texDepth"), 0);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "texNormal"), 1);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "texDither"), 2);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "texAccum"), 3);
				glUniform1f(glGetUniformLocation(mSSAOPass.handle, "radius"), mSSAORadius);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "screenWidth"), width);
				glUniform1i(glGetUniformLocation(mSSAOPass.handle, "screenHeight"), height);
				// We need some carefull adjustment in order to get the rescaled screen sampling correct
				glUniform1f(glGetUniformLocation(mSSAOPass.handle, "screenWidth2"),
					(width % 2 == 0 || mSSAORescale == 1.0f ? width : width-1) * mSSAORescale);
				glUniform1f(glGetUniformLocation(mSSAOPass.handle, "screenHeight2"),
					(height % 2 == 0 || mSSAORescale == 1.0f ? height : height-1) * mSSAORescale);
				glUniform1f(glGetUniformLocation(mSSAOPass.handle, "ssaoRescale"), mSSAORescale);

				// Random points INSIDE an unit sphere.
				float randSphere[] = {
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

				for(size_t i=0; i<sizeof(randSphere)/sizeof(float)/3; ++i)
					reinterpret_cast<Vec3f*>(randSphere+i)->normalize();

				glUniform3fv(glGetUniformLocation(mSSAOPass.handle, "ranSphere"), 32, (float*)randSphere);

				mSSAOPass.unbind();

				mBlurPass.bind();
				glUniform1i(glGetUniformLocation(mBlurPass.handle, "texSSAO"), 0);
				glUniform1i(glGetUniformLocation(mBlurPass.handle, "texNormal"), 1);
				glUniform1f(glGetUniformLocation(mBlurPass.handle, "ssaoRescale"), mSSAORescale);
				mBlurPass.unbind();

				mCombinePass.bind();

				glUniform1i(glGetUniformLocation(mCombinePass.handle, "texColor"), 0);
				glUniform1i(glGetUniformLocation(mCombinePass.handle, "texSSAO"), 1);
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

			glTranslatef(0.0f, -50.0f, 100.0f);

			const float scale = 2.5f;
			glScalef(scale, scale, scale);

			Material2* material = nullptr;
			if(mSceneEffect && (material = mSceneEffect->material.get()) != nullptr) {
				material->preRender(0);
				glActiveTexture(GL_TEXTURE0);
				mRenderable->draw();
				material->postRender(0);
			}

			if(mUseSSAO)
				mSceneRenderTarget->unbind();

			// Compute the delta matrix
			// Read more about "The Real-Time Reprojection Cache
			// http://www.cs.umbc.edu/~olano/s2006c03/ch06.pdf
			Mat44f bias = Mat44f::cIdentity;
			bias.setTranslation(Vec3f(0.5f, 0.5f, 0.5f));
			bias.setScale(Vec3f(0.5f, 0.5f, 0.5f));
			Mat44f currentView;
			mCamera.computeTransform(currentView.data);
			mDeltaMatrix = bias * mLastProjectionMatrix * mLastViewMatrix * currentView.inverse();
			mLastViewMatrix = currentView;
			glGetFloatv(GL_PROJECTION_MATRIX, mLastProjectionMatrix.data);
			mLastProjectionMatrix = mLastProjectionMatrix.transpose();
		}

		void postProcessing()
		{
			if(!mUseSSAO)
				return;

			// Generate new random dithering every frame
			mDitherTexture = generateRandomTexture(32);

			// Render the SSAO
			MCD_ASSERT(mSSAORenderTarget.get());
			mSSAORenderTarget->bind();

			glActiveTexture(GL_TEXTURE0);
			mDepthRenderTexture->bind();

			glActiveTexture(GL_TEXTURE1);
			mNormalRenderTexture->bind();

			glActiveTexture(GL_TEXTURE2);
			mDitherTexture->bind();

			static bool flip = false;
			GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
			TexturePtr current, previous;
			size_t currentIdx, previousIdx;
			if(flip) {
				// accum1 = ssao / 100 + accum2 * 99 / 100;
				current = mAccum1;
				previous = mAccum2;
				currentIdx = 0;
				previousIdx = 1;
			} else {
				// accum2 = ssao / 100 + accum1 * 99 / 100;
				current = mAccum2;
				previous = mAccum1;
				currentIdx = 1;
				previousIdx = 0;
			}
			glDrawBuffers(1, buffers + currentIdx);	// Effectively output to current
			glActiveTexture(GL_TEXTURE3);
			previous->bind();
			flip = !flip;

			mSSAOPass.bind();
			glUniformMatrix4fvARB(glGetUniformLocation(mSSAOPass.handle, "deltaMatrix"), 1, true, mDeltaMatrix.data);
			drawViewportQuad(0, 0, mAccum1->width, mAccum1->height, mAccum1->type);
			mSSAOPass.unbind();

			// Blurr the SSAO
			if(mBlurPassCount > 0)
			{
				mBlurPass.bind();
				glActiveTexture(GL_TEXTURE1);
				mNormalRenderTexture->bind();

				for(size_t i=0; i<mBlurPassCount; ++i)
				{
					glDrawBuffers(1, buffers + previousIdx);	// Effectively output to previous
					glActiveTexture(GL_TEXTURE0);
					current->bind();
					glUniform2f(glGetUniformLocation(mBlurPass.handle, "blurDirection"), 1, 0);
					drawViewportQuad(0, 0, previous->width, previous->height, previous->type);

					glDrawBuffers(1, buffers + currentIdx);	// Effectively output to current
					glActiveTexture(GL_TEXTURE0);
					previous->bind();
					glUniform2f(glGetUniformLocation(mBlurPass.handle, "blurDirection"), 0, 1);
					drawViewportQuad(0, 0, current->width, current->height, current->type);
				}

				mBlurPass.unbind();
			}

			mSSAORenderTarget->unbind();

			// Final pass that combine the color and the SSAO
			glActiveTexture(GL_TEXTURE0);
			mColorRenderTexture->bind();

			glActiveTexture(GL_TEXTURE1);
			current->bind();

			mCombinePass.bind();
			drawViewportQuad(0, 0, width(), height(), mAccum1->type);
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

			// NOTE: Don't konw why the light position have to set every frame
			GLfloat lightPos[] = { 200, 200, 200, 1.0f };
			glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

			drawScene();

			postProcessing();
		}

		bool mUseSSAO;
		bool mBlurSSAO;
		bool mShowTexture;
		float mSSAORescale;
		float mSSAORadius;
		size_t mBlurPassCount;

		ResourcePtr mModel;
		IRenderable* mRenderable;
		ShaderProgram mSSAOPass, mBlurPass, mCombinePass;

		DefaultResourceManager mResourceManager;

		EffectPtr mSceneEffect, mSSAOEffect;
		TexturePtr mColorRenderTexture, mNormalRenderTexture, mDepthRenderTexture, mDitherTexture;
		TexturePtr mAccum1, mAccum2;
		std::auto_ptr<RenderTarget> mSceneRenderTarget, mSSAORenderTarget;

		Mat44f mLastViewMatrix;
		Mat44f mLastProjectionMatrix;
		Mat44f mDeltaMatrix;
	};	// TestWindow

	{
		TestWindow window;

//		window.loadModel(L"Stanford/dragon.3DS");
//		window.loadModel(L"TextureBoxSphere.3ds");
//		window.loadModel(L"Scene/House/scene.3ds");
//		window.loadModel(L"Scene/City/scene.3ds");
		window.loadModel(L"Scene/03/scene.3ds");
//		window.loadModel(L"Ship/01/scene.3ds");
//		window.loadModel(L"Scene/National Stadium/scene.pod");
//		window.loadModel(L"Church/sponza/sponza.3ds");
//		window.loadModel(L"3M00696/buelllightning.3DS");
//		window.loadModel(L"Lamborghini Gallardo Polizia/Lamborghini Gallardo Polizia.3DS");

		window.mainLoop();
	}
}
