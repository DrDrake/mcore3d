#include "Pch.h"
#include "../../MCD/Core/System/StrUtility.h"
#include "../../MCD/Render/BackRenderBuffer.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "ChamferBox.h"

using namespace MCD;

TEST(RenderTargetTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=RenderTargetTest;width=512;height=512;"), mAngle(0)
		{
			{	// Setup the render target
				mRenderTarget.reset(new RenderTarget(width(), height()));
				RenderBufferPtr textureBuffer = new TextureRenderBuffer();
				textureBuffer->bind(*mRenderTarget);
				mRenderTexture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

				RenderBufferPtr backBuffer = new BackRenderBuffer();
				backBuffer->bind(*mRenderTarget);
			}

			{	// Setup the chamfer box mesh
				mMesh = new Mesh(L"");
				ChamferBoxBuilder chamferBoxBuilder(0.4f, 10);
				chamferBoxBuilder.commit(*mMesh, MeshBuilder::Static);
			}
		}

		void drawToTexture()
		{
			mRenderTarget->bind();

			// Set the current Viewport to the FBO size
			glViewport(0, 0, width(), height());

			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_LIGHTING);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(mFieldOfView, (GLfloat)width()/(GLfloat)height(), 0.1f, 1000.0f);
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

			static const float tex[4][2] = {
				{0, 1}, {0, 0}, {1, 0}, {1, 1}
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

			glViewport(0, 0, (GLsizei)mWidth, (GLsizei)mHeight);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45, (GLfloat)mWidth/(GLfloat)mHeight, 0.1f, 1000.0f);
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
			drawToTexture();
			drawTextureToScene();
			mAngle += deltaTime * 10;
		}

		float mAngle;
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
