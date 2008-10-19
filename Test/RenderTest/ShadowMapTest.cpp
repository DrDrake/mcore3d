#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/RenderTarget.h"
#include "../../MCD/Render/ProjectiveTexture.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/TextureRenderBuffer.h"
#include "../../MCD/Core/Math/Mat44.h"

using namespace MCD;

TEST(ShadowMapTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=ShadowMapTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mResourceManager(L"./Media/")
		{
			mShadowMapProjection.frustum.create(80, 1, 1, 50);
			mShadowMapProjection.camera = Camera(Vec3f(20, 20, 10), Vec3f(2, 0, 0), Vec3f::c010);

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
			mShadowMapProjection.texture = static_cast<TextureRenderBuffer&>(*textureBuffer).texture;

			{	// Setup the texture parameters for the shadow map
				mShadowMapProjection.texture->bind();

				glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

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
			mShadowMapProjection.frustum.applyProjection();

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
			const float scale = 0.1f;
			glScalef(scale, scale, scale);

			mModel->draw();
		}

		void drawFrustum()
		{
			// Calculate the vertex of the frustum.
			Vec3f vertex[8];
			mShadowMapProjection.frustum.computeVertex(vertex);

			Mat44f transform;
			mShadowMapProjection.camera.computeTransform(transform.getPtr());
			transform = transform.inverse();
	
			for(size_t i=0; i<sizeof(vertex)/sizeof(Vec3f); ++i)
				transform.transformPoint(vertex[i]);

			// TODO: There is no effect on calling glColor3f()
			glColor3f(1, 1, 0);
			glBegin(GL_LINE_LOOP);
				glVertex3fv(vertex[0 + 0].getPtr());
				glVertex3fv(vertex[1 + 0].getPtr());
				glVertex3fv(vertex[2 + 0].getPtr());
				glVertex3fv(vertex[3 + 0].getPtr());
				glVertex3fv(vertex[3 + 4].getPtr());
				glVertex3fv(vertex[2 + 4].getPtr());
				glVertex3fv(vertex[1 + 4].getPtr());
				glVertex3fv(vertex[0 + 4].getPtr());
				glVertex3fv(vertex[0 + 0].getPtr());
				glVertex3fv(vertex[1 + 0].getPtr());
				glVertex3fv(vertex[1 + 4].getPtr());
				glVertex3fv(vertex[0 + 4].getPtr());
				glVertex3fv(vertex[3 + 4].getPtr());
				glVertex3fv(vertex[2 + 4].getPtr());
				glVertex3fv(vertex[2 + 0].getPtr());
				glVertex3fv(vertex[3 + 0].getPtr());
			glEnd();
			glColor3f(1, 1, 1);
		}

		sal_override void update(float deltaTime)
		{
			mResourceManager.processLoadingEvents();

			drawShadowMap();

			drawFrustum();

			mShadowMapProjection.bind(GL_TEXTURE1);

			glActiveTexture(GL_TEXTURE0);
			drawScene();
			mShadowMapProjection.unbind();
		}

		ModelPtr mModel;
		DefaultResourceManager mResourceManager;
		std::auto_ptr<RenderTarget> mRenderTarget;

		size_t mShadowMapSize;
		size_t mShadowMapRendered;
		ProjectiveTexture mShadowMapProjection;
	};	// TestWindow

	{
		TestWindow window;

		window.load3ds(L"city/city.3ds");

		window.mainLoop();
	}

	CHECK(true);
}
