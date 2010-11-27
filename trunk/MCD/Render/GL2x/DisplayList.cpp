#include "Pch.h"
#include "../DisplayList.h"
#include "../DisplayList.inc"
#include "Renderer.inc"
#include "../GpuDataFormat.h"
#include "../../Core/System/Utility.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

class DisplayListComponent::Impl : public DisplayListComponentImplBase
{
public:
	Impl() : mBufferHandle(0), mBufferSize(0)
	{}

	~Impl()
	{
		if(mBufferHandle) glDeleteBuffers(1, &mBufferHandle);
	}

	void clear();
	void end();

	void bind();
	void draw(void* context, Statistic& statistic);
	void unbind();

	GLuint mBufferHandle;
	size_t mBufferSize;
};	// Impl

void DisplayListComponent::Impl::clear()
{
	DisplayListComponentImplBase::clear();
	mBufferSize = 0;
}

void DisplayListComponent::Impl::end()
{
	DisplayListComponentImplBase::end();

	const size_t requestedVBSize = mVertices.size() * sizeof(Vertex);

	if(!requestedVBSize)
		return;

	if(!mBufferHandle)
		glGenBuffers(1, &mBufferHandle);
	
	glBindBuffer(GL_ARRAY_BUFFER, mBufferHandle);

	// Resize the gl buffer if necessary
	if(mBufferSize <= requestedVBSize) {
		mBufferSize = requestedVBSize;
		glBufferData(GL_ARRAY_BUFFER, mBufferSize, nullptr, GL_STATIC_READ);
		glBufferData(GL_ARRAY_BUFFER, mBufferSize, &mVertices[0], GL_DYNAMIC_DRAW);
	}
	else
		glBufferSubData(GL_ARRAY_BUFFER, 0, mBufferSize, &mVertices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DisplayListComponent::Impl::bind()
{
	static const size_t stride = sizeof(Vertex);
	// NOTE: The name is currently not used
	struct Attr { const char* name; GpuDataFormat fmt; size_t byteOffset; };
	static const Attr attr[] = {
		{ "position", GpuDataFormat::get("floatRGB32"), 0 },
		{ "normal", GpuDataFormat::get("floatRGB32"), 3 * sizeof(float) },
		{ "uv0", GpuDataFormat::get("floatRGB32"), 6 * sizeof(float) },
		{ "color0", GpuDataFormat::get("floatRGBA32"), 9 * sizeof(float) },
	};

	glEnableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, mBufferHandle);
	glNormalPointer(attr[1].fmt.dataType, stride, (GLvoid*)attr[1].byteOffset);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(attr[2].fmt.componentCount, attr[2].fmt.dataType, stride, (GLvoid*)attr[2].byteOffset);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(attr[3].fmt.componentCount, attr[3].fmt.dataType, stride, (GLvoid*)attr[3].byteOffset);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(attr[0].fmt.componentCount, attr[0].fmt.dataType, stride, (GLvoid*)attr[0].byteOffset);
}

void DisplayListComponent::Impl::draw(void* context, Statistic& statistic)
{
	MCD_FOREACH(const Section& section, mSections)
	{
		GLenum type;
		size_t primitiveCount = section.vertexCount;

		switch(section.primitive) {
		case Triangles:
			type = GL_TRIANGLES;
			primitiveCount /= 3;
			break;
		case Lines:
			type = GL_LINES;
			primitiveCount /= 2;
			break;
		case LineStrip:
			type = GL_LINE_STRIP;
			primitiveCount -= 1;
			break;
		default:
			continue;
		}

		bind();
		glDrawArrays(type, section.offset, section.vertexCount);
		unbind();

		++statistic.drawCallCount;
		statistic.primitiveCount += primitiveCount;
	}
}

void DisplayListComponent::Impl::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

DisplayListComponent::DisplayListComponent()
	: mImpl(*new Impl)
{
}

DisplayListComponent::~DisplayListComponent()
{
	delete &mImpl;
}

void DisplayListComponent::clear()
{
	mImpl.clear();
}

void DisplayListComponent::begin(PrimitiveType primitive)
{
	mImpl.begin(primitive);
}

void DisplayListComponent::color(float r, float g, float b, float a)
{
	mImpl.color(r, g, b, a);
}

void DisplayListComponent::texcoord(float u, float v, float w)
{
	mImpl.texcoord(u, v, w);
}

void DisplayListComponent::normal(float x, float y, float z)
{
	mImpl.normal(x, y, z);
}

void DisplayListComponent::vertex(float x, float y, float z)
{
	mImpl.vertex(x, y, z);
}

void DisplayListComponent::end()
{
	mImpl.end();
}

void DisplayListComponent::render(void* context)
{
	Entity* e = entity();
	MCD_ASSUME(e);

	RendererComponent::Impl& renderer = *reinterpret_cast<RendererComponent::Impl*>(context);
	renderer.submitDrawCall(*this, *e, e->worldTransform());
}

void DisplayListComponent::draw(void* context, Statistic& statistic)
{
	mImpl.draw(context, statistic);
}

}	// namespace MCD
