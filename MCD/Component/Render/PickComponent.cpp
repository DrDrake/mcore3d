#include "Pch.h"
#include "PickComponent.h"
#include "RenderableComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/Utility.h"
#include "../../../3Party/glew/glew.h"
#include <algorithm>
#include <vector>

namespace MCD {

class PickComponent::Impl
{
public:
	Impl()
	{
		x = y = 0;
		width = height = 1;
	}

	size_t x, y, width, height;

	struct PickResult {
		size_t minDepth, maxDepth;
		EntityPtr entity;
	};
	struct PickResultSort {
		bool operator()(const PickResult& lhs, const PickResult& rhs) const {
			return lhs.minDepth < rhs.minDepth;
		}
	};
	//! Stores weak pointers of entities which were picked.
	std::vector<PickResult> pickedEntities;
};	//Impl

PickComponent::PickComponent()
{
	mImpl = new Impl;
}

PickComponent::~PickComponent()
{
	delete mImpl;
}

void PickComponent::update()
{
	if(!entityToPick)
		return;

	MCD_ASSUME(mImpl);

	// The pick buffer for the opengl to store picking results.
	GLuint pickBuffer[1024];
	glSelectBuffer(sizeof(pickBuffer), pickBuffer);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);	// Fill the stack with one element (or glLoadName will generate an error)

	// Adjust the projection matrix so only a certain rectangular area is
	// used for pick detection
	// Reference: http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/glu/pickmatrix.html
	{	GLint view[4];
		glGetIntegerv(GL_VIEWPORT, view);

		// Get the current projection matrix
		float projectionMatrix[4*4];
		glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPickMatrix(
			mImpl->x,
			view[3] - mImpl->y,	// Invert the y-axis
			mImpl->width,
			mImpl->height,
			view);
		glMultMatrixf(projectionMatrix);
		glMatrixMode(GL_MODELVIEW);
	}

	for(EntityPreorderIterator itr(entityToPick.get()); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		RenderableComponent* renderable = itr->findComponent<RenderableComponent>();

		if(renderable) {
			// Use the entity's pointer as the gl name
			// TODO: 64-bit platform problem
			glLoadName(reinterpret_cast<GLuint>(itr.current()));
			renderable->renderFaceOnly();
		}

		itr.next();
	}

	{	glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	// Actually I am not sure this call is really needed or not.
	glPopName();

	size_t hitCount = glRenderMode(GL_RENDER);

	mImpl->pickedEntities.clear();

	// Loop though the pick buffer to extract the information
	size_t index = 0;
	for(size_t i=0; i<hitCount; ++i) {
		size_t nameCount = pickBuffer[index++];
		size_t minDepth = pickBuffer[index++];
		size_t maxDepth = pickBuffer[index++];

		for(size_t j=0; j<nameCount; ++j) {
			Impl::PickResult result = {
				minDepth, maxDepth,
				EntityPtr(reinterpret_cast<Entity*>(pickBuffer[index++]))
			};
			mImpl->pickedEntities.push_back(result);
		}
		MCD_ASSERT(index <= MCD_COUNTOF(pickBuffer));
	}

	// Sort the entities so that the one that closer to the camera will come first
	std::sort(mImpl->pickedEntities.begin( ), mImpl->pickedEntities.end(), Impl::PickResultSort());
}

void PickComponent::setPickRegion(size_t x, size_t y, size_t width, size_t height)
{
	mImpl->x = x;
	mImpl->y = y;
	mImpl->width = width;
	mImpl->height = height;
}

size_t PickComponent::hitCount() const
{
	MCD_ASSUME(mImpl);
	return mImpl->pickedEntities.size();
}

EntityPtr PickComponent::hitAtIndex(size_t index)
{
	if(index >= hitCount())
		return nullptr;
	return mImpl->pickedEntities[index].entity;
}

void PickComponent::clearResult()
{
	mImpl->pickedEntities.clear();
}

}	// namespace MCD
