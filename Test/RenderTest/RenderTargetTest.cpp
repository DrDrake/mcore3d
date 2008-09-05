#include "Pch.h"
#include "ChamferBox.h"
#include "../../MCD/Render/BackRenderBuffer.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"

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
			// Setup the render target, with a squared size
			mLargerSide = width > height ? width : height;
			mRenderTarget.reset(new RenderTarget(mLargerSide, mLargerSide));
			RenderBufferPtr textureBuffer = new TextureRenderBuffer();
			textureBuffer->linkTo(*mRenderTarget);
			mRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			RenderBufferPtr backBuffer = new BackRenderBuffer();
			backBuffer->linkTo(*mRenderTarget);

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

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(mFieldOfView, (GLfloat)mRenderTarget->width()/mRenderTarget->height(), 0.1f, 1000.0f);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			mCamera.applyTransform();

			glTranslatef(0.0f, 0.0f, -2.0f);
			glRotatef(mAngle, 1, 0, 0);
			glRotatef(mAngle, 0, 1, 0);
			glRotatef(mAngle, 0, 0, 1);

			mMesh->draw();
		}

		void drawUnitCube()
		{
			// Face vertice are specified in counterclockwise direction
			static const float vertice[6][4][3] = {
				{ {-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1} },	// Front face	(fix z at 1)
				{ { 1, 1,-1}, { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1} },	// Back face	(fix z at -1)
				{ { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1}, { 1, 1,-1} },	// Right face	(fix x at 1)
				{ {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1} },	// Left face	(fix x at -1)
				{ {-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}, { 1, 1,-1} },	// Top face		(fix y at 1)
				{ { 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1} },	// Bottom face	(fix y at -1)
			};

			// Setting up the texture coordinate
			// Since we are using GL_TEXTURE_RECTANGLE_ARB (the default of TextureRenderBuffer,
			// we must use the pixel as the texture coordinate
			const float w = float(mRenderTarget->width());
			const float h = float(mRenderTarget->height());
			const float tex[4][2] = {
				{0, h}, {0, 0}, {w, 0}, {w, h}
			};

			glColor3f(1, 1, 1);
			glBegin(GL_QUADS);
			for(size_t face=0; face<6; ++face) {
				for(size_t vertex=0; vertex<4; ++vertex) {
					glTexCoord2fv(tex[vertex]);
					glVertex3fv(vertice[face][vertex]);
				}
			}
			glEnd();
		}

		void drawTextureToScene()
		{
			mRenderTarget->unbind();

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
			drawUnitCube();
		}

		sal_override void update(float deltaTime)
		{
			// Some how update() is invoked before the fist onResize() which
			// create the render target, so we make a safety check here
			if(!mRenderTarget.get())
				return;

			drawToTexture();
			drawTextureToScene();
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
