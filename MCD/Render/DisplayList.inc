#include "../Core/Math/Vec3.h"
#include "../Core/Math/Vec4.h"
#include <vector>

namespace MCD {

class DisplayListComponentImplBase
{
public:
	struct Vertex
	{
		Vec3f position;
		Vec3f normal;
		Vec3f uvw;
		Vec4f color;
	};	// Vertex

	struct Section
	{
		size_t offset;	// Vertex offset
		size_t vertexCount;
		DisplayListComponent::PrimitiveType primitive;
	};	// Section

	std::vector<Vertex> mVertices;
	std::vector<Section> mSections;
	Vertex mCurrent;

	DisplayListComponentImplBase()
	{
		color(0, 0, 0, 0);
		texcoord(0, 0, 0);
		normal(0, 0, 0);
		mCurrent.position = Vec3f(0);
	}

	void begin(DisplayListComponent::PrimitiveType primitive)
	{
		color(1, 1, 1, 1);
		texcoord(0, 0, 0);
		normal(0, 0, 1);

		Section sec;
		sec.vertexCount = 0;
		sec.offset = mVertices.size();
		sec.primitive = primitive;
		mSections.push_back(sec);
	}

	void color(float r, float g, float b, float a)
	{
		mCurrent.color = Vec4f(r, g, b, a);
	}

	void texcoord(float u, float v, float w)
	{
		mCurrent.uvw = Vec3f(u, v, w);
	}

	void normal(float x, float y, float z)
	{
		mCurrent.normal = Vec3f(x, y, z);
	}

	void vertex(float x, float y, float z)
	{
		mCurrent.position = Vec3f(x, y, z);
		mVertices.push_back(mCurrent);
	}

	void end()
	{
		MCD_ASSERT(!mSections.empty());
		Section& sec = mSections.back();
		sec.vertexCount = mVertices.size() - sec.offset;
	}

};	// DisplayListComponentImplBase

}	// namespace MCD
