#include "Pch.h"
#include "AudioSourceComponent.h"
#include "../../Core/Entity/Entity.h"
#include "../../Core/System/MemoryProfiler.h"

namespace MCD {

void AudioComponent::traverseEntities(Entity* entityNode)
{
	MemoryProfiler::Scope profiler("AudioComponent::traverseEntities");

	for(EntityPreorderIterator itr(entityNode); !itr.ended();)
	{
		if(!itr->enabled) {
			itr.skipChildren();
			continue;
		}

		AudioComponent* audio = itr->findComponent<AudioComponent>();
		if(audio)
			audio->update();

		itr.next();
	}
}

void AudioSourceComponent::update()
{
	audioSource.update();
}

}	// namespace MCD
