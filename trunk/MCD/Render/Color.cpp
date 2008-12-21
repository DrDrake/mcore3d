#include "Pch.h"
#include "Color.h"
#include "../Core/Math/BasicFunction.h"

namespace MCD {

const ColorRGB8 ColorRGB8::red = { 255, 0, 0 };
const ColorRGB8 ColorRGB8::green = { 0, 255, 0 };
const ColorRGB8 ColorRGB8::blue = { 0, 0, 255 };
const ColorRGB8 ColorRGB8::yellow = { 255, 255, 0 };

void ColorRGBf::clamp()
{
	r = Mathf::clamp(r, 0, 1);
	g = Mathf::clamp(g, 0, 1);
	b = Mathf::clamp(b, 0, 1);
}

void ColorRGBAf::clamp()
{
	ColorRGBf::clamp();
	a = Mathf::clamp(a, 0, 1);
}

}	// namespace MCD
