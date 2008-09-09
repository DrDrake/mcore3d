#ifndef __MCD_RENDERTEST_BASICGLWINDOW__
#define __MCD_RENDERTEST_BASICGLWINDOW__

#include "../../MCD/Render/Camera.h"
#include "../../MCD/Render/GlWindow.h"
#include "../../MCD/Core/Math/Vec2.h"
#include "../../MCD/Core/System/Timer.h"

class MovingCamera : public MCD::Camera
{
public:
	MovingCamera(const MCD::Vec3f& position, const MCD::Vec3f lookAt, const MCD::Vec3f& upVector);

	void setForwardVelocity(float speed);

	//! Move towards right.
	void setRightVelocity(float speed);

	//! Moves upward.
	void setUpVelocity(float speed);

	void setMousePosition(uint x, uint y);

	void setMouseDown(bool isDown);

	void update(float deltaTime);

protected:
	MCD::Vec3f mVelocity;	//!< Represent forward, right and up velocity
	MCD::Vec2<int> mLastMousePos;
	bool mIsMouseDown;
};	// MovingCamera

class BasicGlWindow : public MCD::GlWindow
{
public:
	BasicGlWindow(const wchar_t* options = L"width=800;height=600;FSAA=4");

	sal_override ~BasicGlWindow();

	void mainLoop();

	void preUpdate();

	//! To be overriden by derived class to do the actual update.
	virtual void update(float deltaTime);

	//! To be overriden by derived class to capture the resize event.
	virtual void onResize(size_t width, size_t height);

	void postUpdate();

	bool keepRun() const {
		return mKeepRun;
	}

protected:
	//! To be overriden by derived class to capture window event.
	sal_override void onEvent(const MCD::Event& e);

	void setFieldOfView(float angle);

	void update();

protected:
	bool mKeepRun;
	float mFieldOfView;
	size_t mIteration;
	MovingCamera mCamera;
	MCD::DeltaTimer mTimer;
};	// BasicGlWindow

#endif	// __MCD_RENDERTEST_BASICGLWINDOW__
