#ifndef __MCD_AUDIO_AUDIOSOURCECOMPONENT__
#define __MCD_AUDIO_AUDIOSOURCECOMPONENT__

#include "ShareLib.h"
#include "AudioSource.h"
#include "../Core/Entity/Component.h"
#include <vector>

namespace MCD {

class MCD_ABSTRACT_CLASS MCD_AUDIO_API AudioComponent : public Component
{
public:
	sal_override const std::type_info& familyType() const {
		return typeid(AudioComponent);
	}

	virtual void update(float dt) = 0;

protected:
	sal_override void gather();
};	// AudioComponent

typedef IntrusiveWeakPtr<AudioComponent> AudioComponentPtr;

class MCD_AUDIO_API AudioManagerComponent : public ComponentUpdater
{
	friend class AudioComponent;

protected:
	sal_override void begin();
	sal_override void end(float dt);

	std::vector<AudioComponentPtr> mComponents;
};	// AudioManagerComponent

typedef IntrusiveWeakPtr<AudioManagerComponent> AudioManagerComponentPtr;

/*!	
 */
class MCD_AUDIO_API AudioSourceComponent : public AudioComponent
{
public:
	AudioSourceComponent();

// Cloning
	sal_override sal_checkreturn bool cloneable() const { return true; }

	//!	Note that the current playing position will not be applied to the cloned component.
	sal_override sal_maybenull Component* clone() const;

// Operations
	sal_override void update(float dt);

// Attrubutes
	bool play;

	bool loop;

	float time;

	float volume;	//!< From 0 to 1, default is 1
	float groupvolume;

	float totalTime() const;

	bool useTransform;	//!< Enable 3D audio positioning, default is false

	/*!	If set to true, the AudioSourceComponent will destroy itself when the audio playback is finished.
		Default is false.
	 */
	bool destroyAfterFinish;

	/*!	If set to true, the Entity that contain this AudioSourceComponent will destroy when the audio playback is finished.
		Default is false.
	 */
	bool destroyEntityAfterFinish;

	std::string filePath;

	std::string loadOptions;

protected:
	uint64_t toPcm(float time) const;
	float lastTime;
	std::string lastFilePath;

	AudioSource audioSource;
};	// AudioSourceComponent

typedef IntrusiveWeakPtr<AudioSourceComponent> AudioSourceComponentPtr;

}	// namespace MCD

#endif	// __MCD_AUDIO_AUDIOSOURCECOMPONENT__
