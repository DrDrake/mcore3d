#include "Pch.h"
#include "../../MCD/Render/ChamferBox.h"
#include "../../MCD/Render/BackRenderBuffer.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "../../MCD/Render/Mesh.h"
#include <memory>	// For auto_ptr

using namespace MCD;

TEST(RenderTargetTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=RenderTargetTest;width=800;height=600;"),
			mAngle(0), mLargerSide(0)
		{
			// Setup the chamfer box mesh
			mMesh = new Mesh(L"");
			ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
			chamferBoxBuilder.commit(*mMesh, MeshBuilder::Static);
		}

		// We have to re-create all those off-screen buffers every time
		// the windows is resized.
		sal_override void onResize(size_t width, size_t height)
		{
			// If the window is minized
			if(width == 0 || height == 0)
				return;

			// Setup the render target, with a squared size
			mLargerSide = width > height ? width : height;
			mRenderTarget.reset(new RenderTarget(mLargerSide, mLargerSide));

			// Setup for the color buffer
			TextureRenderBufferPtr textureBuffer = new TextureRenderBuffer(GL_COLOR_ATTACHMENT0_EXT);
			if(!textureBuffer->createTexture(mLargerSide, mLargerSide, GL_TEXTURE_RECTANGLE_ARB, GL_RGB))
				throw std::runtime_error("");
			if(!textureBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");
			mRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			// Setup for the depth buffer
			BackRenderBufferPtr backBuffer = new BackRenderBuffer();
			backBuffer->create(mLargerSide, mLargerSide, GL_DEPTH_COMPONENT24, GL_DEPTH_ATTACHMENT_EXT);
			if(!backBuffer->linkTo(*mRenderTarget))
				throw std::runtime_error("");

			if(!mRenderTarget->checkCompleteness())
				throw std::runtime_error("");

			BasicGlWindow::onResize(width, height);
		}

		void drawToTexture()
		{
			mRenderTarget->bind();

			glEnable(GL_TEXTURE_2D);
			glDisable(GL_TEXTURE_RECTANGLE_ARB);

			// Set the current Viewport to the FBO size
			glViewport(0, 0, mRenderTarget->width(), mRenderTarget->height());

			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_LIGHTING);

			mCamera.applyTransform();

			glTranslatef(0.0f, 0.0f, -2.0f);
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			mMesh->draw();
			mRenderTarget->unbind();
		}

		void drawTextureToCube()
		{
			glEnable(GL_TEXTURE_RECTANGLE_ARB);
			glDisable(GL_TEXTURE_2D);

			glViewport(0, 0, width(), height());

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45, (GLfloat)width()/height(), 0.1f, 1000.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDisable(GL_LIGHTING);

			glTranslatef(0.0f, 0.0f, -5.0f);
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			mRenderTexture->bind();

			glColor3f(1, 1, 1);
			// Since we are using GL_TEXTURE_RECTANGLE_ARB (the default of TextureRenderBuffer,
			// we must use the pixel as the texture coordinate
			drawUnitCube(float(mRenderTarget->width()), float(mRenderTarget->height()));
		}

		sal_override void update(float deltaTime)
		{
			// Some how update() is invoked before the fist onResize() which
			// create the render target, so we make a safety check here
			if(!mRenderTarget.get())
				return;

			drawToTexture();
			drawTextureToCube();
			mAngle += deltaTime * 10;
		}

		float mAngle;
		size_t mLargerSide;	//! The larger side of the window's width or height.
		MCD::MeshPtr mMesh;
		TexturePtr mRenderTexture;
		std::auto_ptr<RenderTarget> mRenderTarget;
	};	// TestWindow

	{
		TestWindow window;
		window.mainLoop();
	}

	CHECK(true);
}
