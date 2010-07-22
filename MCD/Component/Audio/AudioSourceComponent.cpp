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

		// NOTE: We must iterate to the next entity first, since
		// audio->update() may delete the current Entity.
		itr.next();

		if(audio)
			audio->update();
	}
}

AudioSourceComponent::AudioSourceComponent()
{
	destroyAfterFinish = false;
	destroyEntityAfterFinish = false;
}

Component* AudioSourceComponent::clone() const
{
	std::auto_ptr<AudioSourceComponent> cloned(new AudioSourceComponent);

	if(IResourceManager* mgr = audioSource.resourceManager()) {
		if(!cloned->load(*mgr, audioSource.fileId(), audioSource.loadOptions().c_str()))
			return nullptr;
	}

	cloned->destroyAfterFinish = this->destroyAfterFinish;
	cloned->destroyEntityAfterFinish = this->destroyEntityAfterFinish;
	cloned->setEffect(this->mEffect.get());
	return cloned.release();
}

void AudioSourceComponent::update()
{
	audioSource.update();

	if(audioSource.isPcmPlayToEnd()) {
		if(destroyAfterFinish) {
			destroyThis();
			// NOTE: Must return, since this object is deleted!
			return;
		}
		if(destroyEntityAfterFinish) {
			entity()->destroyThis();
			// NOTE: Must return, since this object is deleted!
			return;
		}
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
