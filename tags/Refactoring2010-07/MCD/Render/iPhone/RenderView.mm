#import "RenderView.h"
#import "RenderWindow.h"
#import "RenderWindow.inc"
#import <QuartzCore/QuartzCore.h>

#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 30000
#	import <OpenGLES/ES2/gl.h>
#else
#	import <OpenGLES/ES1/glext.h>
#	define glGenFramebuffers glGenFramebuffersOES
#	define glBindFramebuffer glBindFramebufferOES
#	define glGenRenderbuffers glGenRenderbuffersOES
#	define glBindRenderbuffer glBindRenderbufferOES
#	define glFramebufferRenderbuffer glFramebufferRenderbufferOES
#	define glGetRenderbufferParameteriv glGetRenderbufferParameterivOES
#	define glCheckFramebufferStatus glCheckFramebufferStatusOES
#	define GL_FRAMEBUFFER GL_FRAMEBUFFER
#	define GL_RENDERBUFFER GL_RENDERBUFFER
#	define GL_COLOR_ATTACHMENT0 GL_COLOR_ATTACHMENT0_OES
#	define GL_DEPTH_COMPONENT16 GL_DEPTH_COMPONENT16_OES
#	define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_OES
#	define GL_RENDERBUFFER_WIDTH GL_RENDERBUFFER_WIDTH_OES
#	define GL_RENDERBUFFER_HEIGHT GL_RENDERBUFFER_HEIGHT_OES
#	define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_OES
#endif

//#import <OpenGLES/EAGL.h>
//#import <OpenGLES/ES1/gl.h>

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
	glGenFramebuffers(1, &_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

	// Get buffer storage from the layer and create color buffer from it
	glGenRenderbuffers(1, &_colorBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
	[self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorBuffer);

	// Getting the width and height of the render buffer
	GLint width;
	GLint height;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	glViewport(0, 0, width, height);
//	glScissor(0, 0, width, height);

	// Setup the detph buffer
	glGenRenderbuffers(1, &_depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;
	if(status != GL_FRAMEBUFFER_COMPLETE) {
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

	glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);

	if(![context presentRenderbuffer:GL_RENDERBUFFER])
		printf("Failed to swap renderbuffer in %s\n", __FUNCTION__);
}

@end
