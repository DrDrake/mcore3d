#include "Pch.h"
#include "../RenderWindow.h"

namespace MCD {

RenderWindow::RenderWindow()
{
	MCD_ASSERT(false);
}

RenderWindow::~RenderWindow()
{
	destroy();
}

void* RenderWindow::renderContext()
{
	return nullptr;
}

void RenderWindow::create(const char* options)
{
}

void RenderWindow::create(Handle existingControl, const char* options)
{
}

void RenderWindow::destroy()
{
	Window::destroy();
}

bool RenderWindow::makeActive()
{
	return true;
}

void* RenderWindow::getActiveContext()
{
	return nullptr;
}

void RenderWindow::preUpdate()
{
}

void RenderWindow::postUpdate()
{
}

bool RenderWindow::setVerticalSync(bool flag)
{
	return true;
}

void RenderWindow::onEvent(const Event& eventReceived)
{
	// Let Window (the super class of RenderWindow) to do the remaining job
	Window::onEvent(eventReceived);
}

}	// namespace MCD
