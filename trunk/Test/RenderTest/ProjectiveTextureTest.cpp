#include "Pch.h"
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Frustum.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Math/Mat44.h"

using namespace MCD;

TEST(ProjectiveTextureTest)
{
	class TestWindow : public BasicGlWindow
	{
	public:
		TestWindow()
			:
			BasicGlWindow(L"title=ProjectiveTextureTest;width=800;height=600;fullscreen=0;FSAA=4"),
			mAngle(0), mTime(0), mResourceManager(L"./Media/")
		{
			mLightFrustum.create(40, 1, 1, 20);
			mLightCamera = Camera(Vec3f(10, 20, 0), Vec3f(2, 0, 0), Vec3f::c001);
		}

		void load3ds(const wchar_t* fileId)
		{
			mModel = dynamic_cast<Model*>(mResourceManager.load(fileId).get());
		}

		void loadTexture(const wchar_t* fileId)
		{
			mTextureToProject = dynamic_cast<Texture*>(mResourceManager.load(fileId).get());
		}

		void drawFrustum()
		{
			// Calculate the vertex of the frustum.
			Vec3f vertex[8];
			mLightFrustum.computeVertex(vertex);

			Mat44f transform;
			mLightCamera.computeTransform(transform.getPtr());
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
		}

		void projectTexture()
		{
			// Animate the projection frustum in a circular montion.
			mLightCamera.lookAt = Vec3f(sinf(mTime), 0, cosf(mTime)) * 5;

			Mat44f tmp = Mat44f::cIdentity;

			// Scale and translate by one-half to bring the coordinates from [-1, 1]
			// to the texture coordinate [0, 1]
			tmp.setTranslation(Vec3f(0.5f, 0.5f, 0.5f));
			tmp.setScale(Vec3f(0.5f, 0.5f, 0.5f));

			Mat44f projection;
			mLightFrustum.computePerspective(projection.getPtr());

			Mat44f modelView;
			mLightCamera.computeTransform(modelView.getPtr());
			tmp = tmp * projection * modelView;

			// Use another texture unit to avoid conflit with the diffuse texture of the model
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glEnable(GL_TEXTURE_GEN_Q);

			// A post-multiply by the inverse of the current modelview matrix is
			// applied automatically to the eye plane equations we provided.
			glTexGenfv(GL_S, GL_EYE_PLANE, &tmp[0][0]);
			glTexGenfv(GL_T, GL_EYE_PLANE, &tmp[1][0]);
			glTexGenfv(GL_R, GL_EYE_PLANE, &tmp[2][0]);
			glTexGenfv(GL_Q, GL_EYE_PLANE, &tmp[3][0]);

			glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenf(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);

			mTextureToProject->bind();
		}

		sal_override void update(float deltaTime)
		{
			mTime += deltaTime;

			mResourceManager.processLoadingEvents();

			drawFrustum();

			projectTexture();

			const float scale = 0.1f;
			glScalef(scale, scale, scale);

			glActiveTexture(GL_TEXTURE0);
			glEnable(GL_TEXTURE_2D);
			mModel->draw();
		}

		ModelPtr mModel;
		float mAngle;
		float mTime;

		Frustum mLightFrustum;
		Camera mLightCamera;

		DefaultResourceManager mResourceManager;
		TexturePtr mTextureToProject;
	};	// TestWindow

	{
		TestWindow window;

		window.load3ds(L"city/city.3ds");
		window.loadTexture(L"Progressive512x512.jpg");

		window.mainLoop();
	}

	CHECK(true);
}
