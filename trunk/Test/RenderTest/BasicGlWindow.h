#ifndef __MCD_RENDERTEST_BASICGLWINDOW__
#define __MCD_RENDERTEST_BASICGLWINDOW__

#include "../../MCD/Render/GlWindow.h"
#include "../../MCD/Core/System/Timer.h"

class BasicGlWindow : public MCD::GlWindow
{
public:
	BasicGlWindow(const wchar_t* options = L"width=640;height=480;FSAA=4");

	sal_override ~BasicGlWindow();

	void mainLoop();

	void preUpdate();

	//! To be overriden by derived class to do the actual update
	virtual void update(float deltaTime);

	sal_override void onClose();

	void postUpdate();

protected:
	void setFieldOfView(float angle);

	void update();

protected:
	bool mIsClosing;
	uint mWidth, mHeight;
	float mFieldOfView;
	size_t mIteration;
	MCD::DeltaTimer mTimer;
};	// BasicGlWindow

#endif	// __MCD_RENDERTEST_BASICGLWINDOW__
