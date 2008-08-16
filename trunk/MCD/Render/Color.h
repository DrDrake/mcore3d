#ifndef __MCD_RENDER_COLOR__
#define __MCD_RENDER_COLOR__

#include "ShareLib.h"

namespace MCD {

/*!	A structure that represent a color with red, green and blue component.
	\note Each color component use 8 bit to represent, total 24-bits
 */
class MCD_RENDER_API ColorRGB8
{
public:
	static const ColorRGB8 red;
	static const ColorRGB8 green;
	static const ColorRGB8 blue;
	static const ColorRGB8 yellow;

	uint8_t r, g, b;
};	// ColorRGB8

class MCD_RENDER_API ColorRGBf
{
public:
	ColorRGBf() {}

	ColorRGBf(float uniform) : r(uniform), g(uniform), b(uniform) {}

	/*!	Multiple the color with a scaler.
		\note
			May get out of range after the multiplication, use clamp()
			to make it back in range.
	 */
	ColorRGBf& operator*=(float scale)
	{
		r *= scale; g *= scale; b *= scale;
		return *this;
	}

	//! Clamp the color components back into it's range [0, 1]
	void clamp();

	float r, g, b;
};	// ColorRGBf

}	// namespace MCD

#endif	// __MCD_RENDER_COLOR__
