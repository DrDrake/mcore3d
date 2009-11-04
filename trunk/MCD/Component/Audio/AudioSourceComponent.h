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

/*!	
 */
class MCD_COMPONENT_API AudioSourceComponent : public AudioComponent
{
public:
// Operations
	sal_override void update();

	sal_checkreturn bool load(
		IResourceManager& resourceManager, const Path& fileId,
		sal_in_z_opt const wchar_t* args=nullptr)
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

	AudioSource audioSource;
};	// AudioSourceComponent

typedef WeakPtr<AudioSourceComponent> AudioSourceComponentPtr;

}	// namespace MCD

#endif	// __MCD_COMPONENT_AUDIO_AUDIOSOURCECOMPONENT__