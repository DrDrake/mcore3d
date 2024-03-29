#ifndef __MCD_COMPONENT_AUDIO_AUDIOSOURCECOMPONENT__
#define __MCD_COMPONENT_AUDIO_AUDIOSOURCECOMPONENT__

#include "../ShareLib.h"
#include "../../Audio/AudioSource.h"
#include "../../Core/Entity/Component.h"

namespace MCD {

class MCD_ABSTRACT_CLASS MCD_COMPONENT_API AudioComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(AudioComponent);
	}

	virtual void update() = 0;

	//! Invoke the AudioComponent::update() in every Entity under the entityNode sub-tree.
	static void traverseEntities(sal_maybenull Entity* entityNode);
};	// AudioComponent

typedef IntrusiveWeakPtr<class AudioEffectComponent> AudioEffectComponentPtr;

/*!	
 */
class MCD_COMPONENT_API AudioSourceComponent : public AudioComponent
{
public:
	AudioSourceComponent();

// Cloning
	//!	Note that the current playing position will not be applied to the cloned component.
	sal_override sal_maybenull Component* clone() const;

// Operations
	sal_override void update();

	sal_checkreturn bool load(
		IResourceManager& resourceManager, const Path& fileId,
		sal_in_z_opt const char* args=nullptr)
	{
		return audioSource.load(resourceManager, fileId, args);
	}

	void play() { audioSource.play(); }

	void pause() { audioSource.pause(); }

	void stop() { audioSource.stop(); }

	sal_checkreturn bool seek(uint64_t pcmOffset) { return audioSource.seek(pcmOffset); }

// Attrubutes
	size_t frequency() const { return audioSource.frequency(); }

	uint64_t totalPcm() const { return audioSource.totalPcm(); }

	uint64_t currentPcm() const { return audioSource.currentPcm(); }

	bool isPlaying() const { return audioSource.isPlaying(); }

	bool isPaused() const { return audioSource.isPaused(); }

	float gain() const { return audioSource.gain(); }

	//! Set the volumn from 0 to 1, default is 1.
	void setGain(float value) { audioSource.setGain(value); }

	AudioEffectComponent* effect() { return mEffect.get(); }

	void setEffect(sal_in_opt AudioEffectComponent* effect);

	AudioSource audioSource;

	/*!	If set to true, the AudioSourceComponent will destroy itself when the audio playback is finished.
		Default is false.
	 */
	bool destroyAfterFinish;

	/*!	If set to true, the Entity that contain this AudioSourceComponent will destroy when the audio playback is finished.
		Default is false.
	 */
	bool destroyEntityAfterFinish;

protected:
	AudioEffectComponentPtr mEffect;
};	// AudioSourceComponent

typedef IntrusiveWeakPtr<AudioSourceComponent> AudioSourceComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENT_AUDIO_AUDIOSOURCECOMPONENT__
