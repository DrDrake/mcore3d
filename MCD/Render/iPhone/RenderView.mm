#import "RenderView.h"
#import "RenderWindow.h"
#import "RenderWindow.inc"
#import <QuartzCore/QuartzCore.h>

//#import <OpenGLES/EAGL.h>
//#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

#define CHECK_GL_ERROR() ({ GLenum __error = glGetError(); if(__error) printf("OpenGL error 0x%04X in %s\n", __error, __FUNCTION__); (__error ? NO : YES); })

using namespace MCD;

@implementation RenderView

@synthesize renderWindow = _window;

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

// TODO: More error check
- (BOOL) _createSurface
{
	CAEAGLLayer* eaglLayer = (CAEAGLLayer*)[self layer];

	if(![EAGLContext setCurrentContext:self.context])
		return NO;

	// Create the FBO
	glGenFramebuffersOES(1, &_frameBuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, _frameBuffer);

	// Get buffer storage from the layer and create color buffer from it
	glGenRenderbuffersOES(1, &_colorBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _colorBuffer);
	[self.context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:eaglLayer];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, _colorBuffer);

	// Getting the width and height of the render buffer
	GLint width;
	GLint height;
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &width);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &height);
	glViewport(0, 0, width, height);
//	glScissor(0, 0, width, height);

	// Setup the detph buffer
	glGenRenderbuffersOES(1, &_depthBuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _depthBuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, _depthBuffer);

	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) ;
	if(status != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"failed to make complete framebuffer object %x", status);
		return NO;
	}

	CHECK_GL_ERROR();

	return YES;
}

- (id) initWithFrame:(CGRect)frame
{
	if((self = [super initWithFrame:frame])) {
		_window = new RenderWindow;
		_window->create();
		if(![self _createSurface]) {
			[self release];
			return nil;
		}
	}

	return self;
}

- (void) setAsCurrentContext
{
	_window->makeActive();
}

- (void) dealloc
{
	delete _window;
	[super dealloc];
}

- (EAGLContext*) context
{
	return _window ? (EAGLContext*)_window->renderContext() : nil;
}

- (void) swapBuffers
{
	EAGLContext* context = [EAGLContext currentContext];

	if(context != self.context) {
		[EAGLContext setCurrentContext:self.context];
		context = self.context;
	}

	// TODO: Check OpenGl error
	CHECK_GL_ERROR();

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _colorBuffer);

	if(![context presentRenderbuffer:GL_RENDERBUFFER_OES])
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
}

@end
