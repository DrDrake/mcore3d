#include "Pch.h"
#include "../DisplayList.h"

namespace MCD {

DisplayListComponent::DisplayListComponent()
	: mImpl(*reinterpret_cast<Impl*>(nullptr))
{
}

DisplayListComponent::~DisplayListComponent()
{
}

void DisplayListComponent::clear()
{
}

void DisplayListComponent::begin(PrimitiveType primitive)
{
}

void DisplayListComponent::color(float r, float g, float b, float a)
{
}

void DisplayListComponent::texcoord(float u, float v, float w)
{
}

void DisplayListComponent::normal(float x, float y, float z)
{
}

void DisplayListComponent::vertex(float x, float y, float z)
{
}

void DisplayListComponent::end()
{
}

void DisplayListComponent::render(void* context)
{
}

void DisplayListComponent::draw(void* context, Statistic& statistic)
{
}

}	// namespace MCD
