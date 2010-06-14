#import <UIKit/UIKit.h>

namespace MCD
{
	class RenderWindow;
}	// namespace

@class EAGLContext;

/*!	This class wraps our C++ RenderWindow into a UIView subclass such that
	you can add this view into the application.
 */
@interface RenderView : UIView
{
@private
	MCD::RenderWindow* _window;
	unsigned int _frameBuffer;
	unsigned int _colorBuffer;
	unsigned int _depthBuffer;
}

@property(readonly) MCD::RenderWindow* renderWindow;
@property(readonly) EAGLContext* context;

- (id) initWithFrame:(CGRect)frame;
- (void) setAsCurrentContext;
- (void) swapBuffers;

@end
