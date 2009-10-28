#include "Pch.h"
#include "AudioEffect.h"
#include "AudioSource.h"
#include "AudioBuffer.h"	// for checkAndPrintError()
#include "../../3Party/OpenAL/al.h"
#include "../../3Party/OpenAL/efx.h"

// Effect objects
static LPALGENFILTERS alGenEffects;
static LPALDELETEEFFECTS alDeleteEffects;
static LPALEFFECTI alEffecti;
static LPALEFFECTF alEffectf;

// Auxiliary slot object
static LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
static LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
static LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;

namespace MCD {

void initAudioEffect()
{
	alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
	alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
	alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
	alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");

	alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
	alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
	alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
}

AudioEffect::AudioEffect()
{
	alGenEffects(1, &handle);
	alGenAuxiliaryEffectSlots(1, &slotHandle);
}

AudioEffect::~AudioEffect()
{
	alDeleteEffects(1, &handle);
	alDeleteAuxiliaryEffectSlots(1, &slotHandle);
}

void AudioEffect::create(sal_in_z_opt const char* effectName)
{
	alEffecti(handle, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
	alEffectf(handle, AL_REVERB_DECAY_TIME, 5.0f);
	alAuxiliaryEffectSloti(slotHandle, AL_EFFECTSLOT_EFFECT, handle);
	checkAndPrintError("");
}

void AudioEffect::bind(AudioSource& source)
{
	alSource3i(source.handle, AL_AUXILIARY_SEND_FILTER, slotHandle, 0, AL_FILTER_NULL);
	checkAndPrintError("");
}

void AudioEffect::setParameters(sal_in_z_opt const char* paramNameValuePair)
{
}

float AudioEffect::getParameterf(sal_in_z_opt const char* paramName)
{
}

}	// namespace MCD
