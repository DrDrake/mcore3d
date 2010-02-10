#include "Pch.h"
#include "BasicGlWindow.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/CpuProfiler.h"
#include "../../MCD/Core/System/WindowEvent.h"
#include "../../MCD/Core/System/MemoryProfiler.h"
#include "../../3Party/glew/glew.h"
#include <iostream>
#include <stdio.h>	// for printf

#if defined(MCD_VC)
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "glew")
#endif

using namespace MCD;

MovingCamera::MovingCamera(const Vec3f& pos, const Vec3f look, const Vec3f& up)
	: Camera(pos, look, up), mVelocity(0), mLastMousePos(-1), mIsMouseDown(false)
{
	MCD_ASSERT(look.length() > 0);
	MCD_ASSERT(up.length() > 0);
}

void MovingCamera::setForwardVelocity(float speed)
{
	mVelocity.x = speed;
}

void MovingCamera::setRightVelocity(float speed)
{
	mVelocity.y = speed;
}

void MovingCamera::setUpVelocity(float speed)
{
	mVelocity.z = speed;
}

void MovingCamera::setMousePosition(uint x, uint y)
{
	Vec2<int> pos = Vec2<int>(x, y);

	// 1 degree per mouse pixel movement
	static const float sensitivity = 0.5f * Mathf::cPi()/180;

	// We won't have the delta value for the first invocation of setMousePosition()
	if(mIsMouseDown && mLastMousePos.x >= 0 && pos != mLastMousePos) {
		Vec2f delta(float(pos.x - mLastMousePos.x), float(pos.y - mLastMousePos.y));
		delta *= sensitivity;
		rotate(Vec3f(0, 1, 0), -delta.x);
		rotate(rightVector(), -delta.y);
	}

	mLastMousePos = Vec2<int>(x, y);
}

void MovingCamera::setMouseDown(bool isDown)
{
	mIsMouseDown = isDown;
}

void MovingCamera::update(float deltaTime)
{
	if(mVelocity.length() > 0) {
		moveForward(mVelocity.x * deltaTime);
		moveRight(mVelocity.y * deltaTime);
		moveUp(mVelocity.z  * deltaTime);
	}
}

BasicGlWindow::BasicGlWindow(const wchar_t* options)
	:
	mKeepRun(true),
	mIteration(0),
	mCamera(Vec3f(0, 0, 1.0f), Vec3f(0, 0, -1), Vec3f(0, 1, 0)),
	mCameraVelocity(10.f),
	mFrameCounter(0), mOneSecondCountDown(1.0f)
{
#ifndef MCD_CYGWIN
	Log::start(&std::wcout);
	Log::setLevel(Log::Level(Log::Error | Log::Warn | Log::Info));
#endif

	create(options);

	// A lot of opengl options to be enabled by default
	glShadeModel(GL_SMOOTH);
	glFrontFace(GL_CCW);			// OpenGl use counterclockwise as the default winding
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	// Automatic normalization (useful when we have uniform scaled the model)
	// Reference: http://www.opengl.org/resources/features/KilgardTechniques/oglpitfall/
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Set up and enable light 0
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

BasicGlWindow::~BasicGlWindow()
{
	Log::stop(false);
}

void BasicGlWindow::mainLoop()
{
	while(keepRun()) {
		// Cleanup the event queue
		Event e;
		popEvent(e, false);

		update();

		if(CpuProfiler::singleton().timeSinceLastReset.asSecond() > 0.5) {
			MemoryProfiler::singleton().reset();
			CpuProfiler::singleton().reset();
		}

		// Call nextFrame() after reset() to prevent division by zero (printing report with zero framecount).
		MemoryProfiler::singleton().nextFrame();
		CpuProfiler::singleton().nextFrame();
	}
}

void BasicGlWindow::preUpdate()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void BasicGlWindow::update(float deltaTime)
{
	(void)deltaTime;
}

void BasicGlWindow::onEvent(const Event& e)
{
	switch(e.Type)
	{
	case Event::Closed:
		mKeepRun = false;
		break;

	case Event::KeyPressed:
		switch(e.Key.Code)
		{
		case Key::Up:
		case Key::W:
			mCamera.setForwardVelocity(mCameraVelocity);
			break;
		case Key::Down:
		case Key::S:
			mCamera.setForwardVelocity(-mCameraVelocity);
			break;
		case Key::Right:
		case Key::D:
			mCamera.setRightVelocity(mCameraVelocity);
			break;
		case Key::Left:
		case Key::A:
			mCamera.setRightVelocity(-mCameraVelocity);
			break;
		case Key::PageUp:
			mCamera.setUpVelocity(mCameraVelocity);
			break;
		case Key::PageDown:
			mCamera.setUpVelocity(-mCameraVelocity);
			break;
		default:
			break;
		}
		break;

	case Event::KeyReleased:
		switch(e.Key.Code)
		{
		case Key::Up:
		case Key::W:
		case Key::Down:
		case Key::S:
			mCamera.setForwardVelocity(0);
			break;
		case Key::Right:
		case Key::D:
		case Key::Left:
		case Key::A:
			mCamera.setRightVelocity(0);
			break;
		case Key::PageUp:
		case Key::PageDown:
			mCamera.setUpVelocity(0);
			break;
		default:
			break;
		}
		break;

	case Event::MouseButtonPressed:
		//setOptions(L"showCursor=0");
		mCamera.setMouseDown(true);
		break;

	case Event::MouseButtonReleased:
		//setOptions(L"showCursor=1");
		mCamera.setMouseDown(false);
		break;

	case Event::MouseMoved:
		mCamera.setMousePosition(e.MouseMove.X, e.MouseMove.Y);
		break;

	case Event::MouseWheelMoved:
	{	float fov = mCamera.frustum.fov();
		fov *= pow(0.9f, e.MouseWheel.Delta);
		if(fov > 160)
			fov = 160;
		mCamera.frustum.setFov(fov);
	}	break;

	case Event::Resized:
		onResize(e.Size.Width, e.Size.Height);
		break;

	default:
		// Just ignore the remaining events
		break;
	}

	GlWindow::onEvent(e);
}

void BasicGlWindow::onResize(size_t width, size_t height)
{
	// Prevents division by zero
	height = (height == 0) ? 1 : height;
	glViewport(0, 0, width, height);
	mCamera.frustum.setAcpectRatio(float(width) / height);
}

void BasicGlWindow::postUpdate()
{
	int a = glGetError();
	if(a != GL_NO_ERROR)
		printf("OpenGl has error 0x%08x\n", a);

	glFlush();
	swapBuffers();
}

void BasicGlWindow::update()
{
	float deltaTime = float(mTimer.getDelta().asSecond());
	mOneSecondCountDown -= deltaTime;
	++mFrameCounter;

	if(mOneSecondCountDown < 0) {
		printf("FPS: %f\r", float(mFrameCounter) / (1-mOneSecondCountDown));
		mOneSecondCountDown = 1 + mOneSecondCountDown;
		mFrameCounter = 0;
	}

	preUpdate();
	mCamera.update(deltaTime);
	mCamera.applyTransform();

	// NOTE: this should be set per-frame after the camera transform since
	// the light position stored in opengl is view-dependent
	GLfloat lightPos[] = { 200, 200, 200, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	update(deltaTime);
	postUpdate();
}
