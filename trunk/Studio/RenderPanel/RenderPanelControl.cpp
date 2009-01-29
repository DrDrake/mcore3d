#include "stdafx.h"
#include "RenderPanelControl.h"

#define _WINDOWS
#include "../../MCD/Render/GlWindow.h"
#include "../../3Party/glew/glew.h"

#pragma comment(lib, "OpenGL32")
#pragma comment(lib, "GLU32")
#pragma comment(lib, "GlAux")
//#pragma comment(lib, "glew")

namespace RenderPanel {

class RenderPanelControlImpl : public MCD::GlWindow
{
};	// RenderPanelControlImpl

RenderPanelControl::RenderPanelControl()
{
	InitializeComponent();
	mImpl = nullptr;
}

RenderPanelControl::~RenderPanelControl()
{
	if(components)
	{
		delete components;
	}

	delete mImpl;
	mImpl = nullptr;	// Finializer may call more than once
}

RenderPanelControl::!RenderPanelControl()
{
	delete mImpl;
	mImpl = nullptr;	// Finializer may call more than once
}

System::Void RenderPanelControl::RenderPanelControl_Load(System::Object^ sender, System::EventArgs^ e)
{
	MCD_ASSERT(mImpl == nullptr);
	mImpl = new RenderPanelControlImpl;
	mImpl->create(Handle.ToPointer(), nullptr);
	mImpl->makeActive();
}

System::Void RenderPanelControl::RenderPanelControl_Resize(System::Object^ sender, System::EventArgs^ e)
{
}

System::Void RenderPanelControl::timer1_Tick(System::Object^ sender, System::EventArgs^ e)
{
	if(!mImpl)
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mImpl->swapBuffers();
}

}
