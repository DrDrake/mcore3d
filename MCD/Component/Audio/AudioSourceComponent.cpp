#include "Pch.h"
#include "AudioSourceComponent.h"
#include "AudioEffectComponent.h"
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

AudioSourceComponent::AudioSourceComponent()
{
	destroyAfterFinish = false;
	destroyEntityAfterFinish = false;
}

void AudioSourceComponent::update()
{
	audioSource.update();

	if(audioSource.isPcmPlayToEnd()) {
		if(destroyAfterFinish) destroyThis();
		if(destroyEntityAfterFinish) delete entity();
	}
}

void AudioSourceComponent::setEffect(AudioEffectComponent* effect)
{
	if(!effect) {
		if(mEffect)
			mEffect->audioEffect.unbind(audioSource);
	} else
		effect->audioEffect.bind(audioSource);

	mEffect = effect;
}

}	// namespace MCD
