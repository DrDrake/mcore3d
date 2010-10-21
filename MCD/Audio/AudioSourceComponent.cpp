#include "Pch.h"
#include "AudioSourceComponent.h"
//#include "AudioEffectComponent.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/SystemComponent.h"

namespace MCD {

static AudioManagerComponent* gAudioManager = nullptr;

void AudioComponent::gather()
{
	MCD_ASSUME(gAudioManager);
	gAudioManager->mComponents.push_back(this);
}

void AudioManagerComponent::begin()
{
	mComponents.clear();
	gAudioManager = this;
}

void AudioManagerComponent::end(float dt)
{
	MCD_FOREACH(const AudioComponentPtr c, mComponents)
		if(c) c->update(dt);
	gAudioManager = nullptr;
}

AudioSourceComponent::AudioSourceComponent()
{
	play = true;
	loop = true;
	time = lastTime = 0;
	volume = 1;
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

void AudioSourceComponent::update(float)
{
	if(play)
	{
		if(!audioSource.isPlaying() || audioSource.isPaused())
			audioSource.play();

		if(filePath != lastFilePath) {	// Check if user want to play another audio file
			// Find the Entity tree root
			if(ResourceManagerComponent* c = ResourceManagerComponent::fromCurrentEntityRoot()) {
				audioSource.stop();
				(void)audioSource.load(c->resourceManager(), filePath, loadOptions.c_str());
				lastFilePath = filePath;
			}
		}

		if(time != lastTime)	// Check if user has altered the time
			MCD_VERIFY(audioSource.seek(toPcm(time)));

		audioSource.setGain(volume);

		if(useTransform) {
			Entity* e = entity();
			MCD_ASSUME(e);
			const Vec3f p = e->worldTransform().translation();
			audioSource.setPosition(p);
		}

		audioSource.update();

		if(audioSource.frequency() == 0)
			lastTime = time = 0;
		else
			lastTime = time = float(double(audioSource.currentPcm()) / audioSource.frequency());
	}
	else
		audioSource.pause();

	if(audioSource.isPcmPlayToEnd()) {
		time = 0;
		if(!loop) {
			play = false;
			if(destroyAfterFinish) destroyThis();
			Entity* e = entity();
			if(destroyEntityAfterFinish) Entity::destroy(e);
		}
	}
}

float AudioSourceComponent::totalTime() const
{
	return float(audioSource.totalPcm() / audioSource.frequency());
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
