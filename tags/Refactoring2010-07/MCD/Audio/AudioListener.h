#ifndef __MCD_AUDIO_AUDIOLISTENER__
#define __MCD_AUDIO_AUDIOLISTENER__

#include "ShareLib.h"
#include "../Core/Math/Vec3.h"
#include "../Core/System/NonCopyable.h"

namespace MCD {

/*!	Currently it is assumed the application can only have one listener.
 */
class MCD_AUDIO_API AudioListener : Noncopyable
{
public:
// Attributes
	float gain() const;

	/*!	While a source gain can only be 0 to 1, any positive value for
		listener's gain will be fine.
	 */
	void setGain(float value);

	Vec3f position() const;

	void setPosition(const Vec3f& p);

	void orientation(Vec3f& lookAt, Vec3f& up) const;

	void setOrientation(const Vec3f& lookAt, const Vec3f& up);

	Vec3f velocity() const;

	void setVelocity(const Vec3f& v);
};	// AudioListener

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOLISTENER__
