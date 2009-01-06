#ifndef __MCD_RENDER_COLOR__
#define __MCD_RENDER_COLOR__

#include "ShareLib.h"

namespace MCD {

/*!	A structure that represent a color with red, green and blue component.
	\note Each color component use 8 bit to represent, total 24-bits
 */
struct MCD_RENDER_API ColorRGB8
{
	static const ColorRGB8 red;
	static const ColorRGB8 green;
	static const ColorRGB8 blue;
	static const ColorRGB8 yellow;

	uint8_t r, g, b;
};	// ColorRGB8

struct MCD_RENDER_API ColorRGBA8
{
	uint8_t r, g, b, a;
};	// ColorRGBA8

class MCD_RENDER_API ColorRGBf
{
public:
	ColorRGBf() {}

	explicit ColorRGBf(float uniform) : r(uniform), g(uniform), b(uniform) {}

// Operations
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

// Attributes
	float* rawPointer() {
		return reinterpret_cast<float*>(this);
	}

	float r, g, b;
};	// ColorRGBf

class MCD_RENDER_API ColorRGBAf : public ColorRGBf
{
public:
	ColorRGBAf() {}

	ColorRGBAf(float uniform, float alpha) : ColorRGBf(uniform), a(alpha) {}

	ColorRGBAf(const ColorRGBf& rgb, float alpha) : ColorRGBf(rgb), a(alpha) {}

// Operations
	//! Clamp the color components back into it's range [0, 1]
	void clamp();

// Attributes
	float* rawPointer() {
		return reinterpret_cast<float*>(this);
	}

	float a;
};	// ColorRGBAf

}	// namespace MCD

#endif	// __MCD_RENDER_COLOR__
