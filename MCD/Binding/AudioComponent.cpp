#include "Pch.h"
#include "AudioComponent.h"
#include "Entity.h"
#include "System.h"

using namespace MCD;

namespace script {

namespace types {

static void destroy(MCD::AudioEffectComponent* obj)	{ obj->destroyThis(); }
static void destroy(MCD::AudioSourceComponent* obj)	{ obj->destroyThis(); }

}	// namespace types

SCRIPT_CLASS_REGISTER(AudioComponent)
	.declareClass<AudioComponent, Component>("AudioComponent")
;}

SCRIPT_CLASS_REGISTER(AudioEffectComponent)
	.declareClass<AudioEffectComponent, AudioComponent>("AudioEffectComponent")
	.constructor()
	.method("create", &AudioEffectComponent::create)
;}

static bool audioSourceComponentLoad(
	AudioSourceComponent& self, IResourceManager& resourceManager,
	const char* path, const char* args)
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
	.declareClass<AudioSourceComponent, AudioComponent>("AudioSourceComponent")
	.enableGetset()
	.constructor()
	.wrappedMethod("load", &audioSourceComponentLoad)
	.method("play", &AudioSourceComponent::play)
	.method("pause", &AudioSourceComponent::pause)
	.method("stop", &AudioSourceComponent::stop)
	.wrappedMethod("seek", &audioSourceComponentSeek)
	.method("_getfrequency", &AudioSourceComponent::frequency)
	.wrappedMethod("_gettotalTime", &audioSourceComponentTotalTime)
	.wrappedMethod("_getcurrentTime", &audioSourceComponentCurrentTime)
	.method("_getisPlaying", &AudioSourceComponent::isPlaying)
	.method("_getisPaused", &AudioSourceComponent::isPaused)
	.method("_getgain", &AudioSourceComponent::gain)
	.method("_setgain", &AudioSourceComponent::setGain)
	.method("_geteffect", &AudioSourceComponent::effect)
	.method("_seteffect", &AudioSourceComponent::setEffect)
	.getset("destroyAfterFinish", &AudioSourceComponent::destroyAfterFinish)
	.getset("destroyEntityAfterFinish", &AudioSourceComponent::destroyEntityAfterFinish)
	.runScript("AudioSourceComponent.classString<-\"AudioComponent()\"")	// The default construction stirng for serialization
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
