#include "Pch.h"
#include "AudioComponent.h"
#include "Entity.h"
#include "System.h"

using namespace MCD;

namespace script {

SCRIPT_CLASS_REGISTER(AudioComponent)
	.declareClass<AudioComponent, Component>(xSTRING("AudioComponent"))
;}

SCRIPT_CLASS_REGISTER(AudioEffectComponent)
	.declareClass<AudioEffectComponent, AudioComponent>(xSTRING("AudioEffectComponent"))
	.constructor()
	.method(xSTRING("create"), &AudioEffectComponent::create)
;}

static bool audioSourceComponentLoad(
	AudioSourceComponent& self, IResourceManager& resourceManager,
	const wchar_t* path, const wchar_t* args)
{
	return self.load(resourceManager, path, args);
}
static bool audioSourceComponentSeek(AudioSourceComponent& self, double seconds) {
	return self.seek(seconds * self.frequency());
}
static double audioSourceComponentTotalTime(AudioSourceComponent& self) {
	return double(self.totalPcm()) / self.frequency();
}
static double audioSourceComponentCurrentTime(AudioSourceComponent& self) {
	return double(self.currentPcm()) / self.frequency();
}
SCRIPT_CLASS_REGISTER(AudioSourceComponent)
	.declareClass<AudioSourceComponent, AudioComponent>(xSTRING("AudioSourceComponent"))
	.enableGetset()
	.constructor()
	.wrappedMethod(xSTRING("load"), &audioSourceComponentLoad)
	.method(xSTRING("play"), &AudioSourceComponent::play)
	.method(xSTRING("pause"), &AudioSourceComponent::pause)
	.method(xSTRING("stop"), &AudioSourceComponent::stop)
	.wrappedMethod(xSTRING("seek"), &audioSourceComponentSeek)
	.method(xSTRING("_getfrequency"), &AudioSourceComponent::frequency)
	.wrappedMethod(xSTRING("_gettotalTime"), &audioSourceComponentTotalTime)
	.wrappedMethod(xSTRING("_getcurrentTime"), &audioSourceComponentCurrentTime)
	.method(xSTRING("_getisPlaying"), &AudioSourceComponent::isPlaying)
	.method(xSTRING("_getisPaused"), &AudioSourceComponent::isPaused)
	.method(xSTRING("_geteffect"), &AudioSourceComponent::effect)
	.method(xSTRING("_seteffect"), &AudioSourceComponent::setEffect)
;}

}	// namespace script

namespace MCD {

void registerAudioComponentBinding(script::VMCore* v)
{
	script::ClassTraits<AudioComponent>::bind(v);
	script::ClassTraits<AudioEffectComponent>::bind(v);
	script::ClassTraits<AudioSourceComponent>::bind(v);
}

}	// namespace MCD
