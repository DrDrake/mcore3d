#include "Pch.h"
#include "AudioSourceComponent.h"
//#include "AudioEffectComponent.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/System/MemoryProfiler.h"

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
		// update() may delete the current Entity.
		itr.next();

		if(audio)
			audio->update();
	}
}

AudioSourceComponent::AudioSourceComponent()
{
	play = true;
	loop = true;
	time = lastTime = 0;
	volumn = 1;
	useTransform = false;
	destroyAfterFinish = false;
	destroyEntityAfterFinish = false;
}

Component* AudioSourceComponent::clone() const
{
	std::auto_ptr<AudioSourceComponent> cloned(new AudioSourceComponent);
	cloned->destroyAfterFinish = this->destroyAfterFinish;
	cloned->destroyEntityAfterFinish = this->destroyEntityAfterFinish;
	return cloned.release();
}

void AudioSourceComponent::update()
{
	if(play)
	{
		if(!audioSource.isPlaying())
			audioSource.play();

		if(filePath != lastFilePath) {	// Check if user want to play another audio file
			// Find the Entity tree root
			if(Entity* e = Entity::currentRoot())
			if(ResourceManagerComponent* c = e->findComponentInChildrenExactType<ResourceManagerComponent>()) {
				(void)audioSource.load(c->resourceManager(), filePath, loadOptions.c_str());
				lastFilePath = filePath;
			}
		}

		if(time != lastTime)	// Check if user has altered the time
			audioSource.seek(toPcm(time));

		audioSource.setGain(volumn);

		if(useTransform) {
			Entity* e = entity();
			MCD_ASSUME(e);
			const Vec3f p = e->worldTransform().translation();
			audioSource.setPosition(p);
		}

		audioSource.update();

		lastTime = time = float(double(audioSource.currentPcm()) / audioSource.frequency());
	}
	else
		audioSource.pause();

	if(audioSource.isPcmPlayToEnd()) {
		if(loop)
			time = 0;
		else {
			play = false;
			if(destroyAfterFinish) destroyThis();
			Entity* e = entity();
			if(destroyEntityAfterFinish) Entity::destroy(e);
		}
	}
}

uint64_t AudioSourceComponent::toPcm(float time) const
{
	return uint64_t(time * audioSource.frequency());
}

/*
void AudioSourceComponent::setEffect(AudioEffectComponent* effect)
{
	if(!effect) {
		if(mEffect)
			mEffect->audioEffect.unbind(audioSource);
	} else
		effect->audioEffect.bind(audioSource);

	mEffect = effect;
}
*/
}	// namespace MCD
