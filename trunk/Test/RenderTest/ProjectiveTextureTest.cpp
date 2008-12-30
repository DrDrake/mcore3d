#include "Pch.h"
#include "Common.h"	// For drawFrustum()
#include "DefaultResourceManager.h"
#include "../../MCD/Render/Frustum.h"
#include "../../MCD/Render/Model.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Core/Math/Mat44.h"

// Reference: http://www.nvidia.com/object/Projective_Texture_Mapping.html
using namespace MCD;

namespace {

class TestWindow : public BasicGlWindow
{
public:
	TestWindow(const wchar_t* options)
		:
		BasicGlWindow(options),
		mAngle(0), mTime(0), mResourceManager(*createDefaultFileSystem())
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
		Mat44f transform;
		mLightCamera.computeTransform(transform.getPtr());
		transform = transform.inverse();

		// TODO: There is no effect on calling glColor3f()
		glColor3f(1, 1, 0);
		glPushMatrix();
		glMultTransposeMatrixf(transform.getPtr());
		::drawFrustum(mLightFrustum);
		glPopMatrix();
		glColor3f(1, 1, 1);
	}

	void projectTexture(bool useShader)
	{
		// Animate the projection frustum in a circular montion.
		mLightCamera.lookAt = Vec3f(sinf(mTime), 0, cosf(mTime)) * 5;

		// Scale and translate by one-half to bring the coordinates from [-1, 1]
		// to the texture coordinate [0, 1]
		Mat44f bias = Mat44f::cIdentity;
		bias.setTranslation(Vec3f(0.5f, 0.5f, 0.5f));
		bias.setScale(Vec3f(0.5f, 0.5f, 0.5f));

		Mat44f projection;
		mLightFrustum.computeProjection(projection.getPtr());

		Mat44f projectionView;
		mLightCamera.computeTransform(projectionView.getPtr());

		Mat44f cameraViewInverse = Mat44f::cIdentity;

		// We use cameraViewInverse to cancel out the gl_ModelViewMatrix
		// in order to get the model matrix ONLY in glsl
		if(useShader) {
			mCamera.computeTransform(cameraViewInverse.getPtr());
			cameraViewInverse = cameraViewInverse.inverse();
		}

		Mat44f textureMatrix = bias * projection * projectionView * cameraViewInverse;

		// Use another texture unit to avoid conflit with the diffuse texture of the model
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

		// The following code block is only relavent for fixed pipeline
		if(!useShader)
		{
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glEnable(GL_TEXTURE_GEN_Q);

			static const GLfloat sPlane[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
			static const GLfloat tPlane[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
			static const GLfloat rPlane[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
			static const GLfloat qPlane[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

			// A post-multiply by the inverse of the current modelview matrix is
			// applied automatically to the eye plane equations we provided.
			glTexGenfv(GL_S, GL_EYE_PLANE, sPlane);
			glTexGenfv(GL_T, GL_EYE_PLANE, tPlane);
			glTexGenfv(GL_R, GL_EYE_PLANE, rPlane);
			glTexGenfv(GL_Q, GL_EYE_PLANE, qPlane);

			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		}

		glMatrixMode(GL_TEXTURE);
		glLoadTransposeMatrixf(textureMatrix.getPtr());
		glMatrixMode(GL_MODELVIEW);
		mTextureToProject->bind();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	}

	void drawScene()
	{
		const float scale = 0.1f;
		glScalef(scale, scale, scale);

		glActiveTexture(GL_TEXTURE0);
		mModel->draw();
	}

	sal_override void update(float deltaTime)
	{
		mTime += deltaTime;

		mResourceManager.processLoadingEvents();

		drawFrustum();

		projectTexture(false);

		drawScene();
	}

	ModelPtr mModel;
	float mAngle;
	float mTime;

	Frustum mLightFrustum;
	Camera mLightCamera;

	DefaultResourceManager mResourceManager;
	TexturePtr mTextureToProject;
};	// TestWindow

}	// namespace

TEST(ProjectiveTextureTest)
{
	TestWindow window(L"title=ProjectiveTextureTest;width=800;height=600;fullscreen=0;FSAA=4");

	window.load3ds(L"city/city.3ds");
	window.loadTexture(L"Progressive512x512.jpg");
	window.mainLoop();

	CHECK(true);
}

// Testing projective texture with shader

#include "../../MCD/Render/Shader.h"
#include "../../MCD/Render/ShaderProgram.h"

namespace {

class ShaderTestWindow : public TestWindow
{
public:
	ShaderTestWindow(const wchar_t* options) : TestWindow(options)
	{
		if(!loadShaderProgram(
			L"Shader/ProjectiveTexture/Scene.glvs", L"Shader/ProjectiveTexture/Scene.glps",
			mProjectiveTextureShader, mResourceManager))
		{
			throw std::runtime_error("Fail to load shader");
		}

		mProjectiveTextureShader.bind();
		int colorTex = glGetUniformLocation(mProjectiveTextureShader.handle, "colorTex");
		glUniform1i(colorTex, 0);

		int projectiveTex = glGetUniformLocation(mProjectiveTextureShader.handle, "projectiveTex");
		glUniform1i(projectiveTex, 1);
		mProjectiveTextureShader.unbind();
	}

	sal_override void update(float deltaTime)
	{
		mTime += deltaTime;

		mResourceManager.processLoadingEvents();

		drawFrustum();

		projectTexture(true);

		mProjectiveTextureShader.bind();
		drawScene();
		mProjectiveTextureShader.unbind();
	}

	ShaderProgram mProjectiveTextureShader;
};	// ShaderTestWindow

}	// namespace

TEST(ProjectiveTextureShaderTest)
{
	ShaderTestWindow window(L"title=ProjectiveTextureShaderTest;width=800;height=600;fullscreen=0;FSAA=4");

	window.load3ds(L"city/city.3ds");
	window.loadTexture(L"Progressive512x512.jpg");
	window.mainLoop();

	CHECK(true);
}
