#include "Pch.h"
#include "AudioBindings.h"
#include "AudioSourceComponent.h"
#include "../Core/Binding/CoreBindings.h"
#include "../Core/Binding/Declarator.h"
#include "../Core/Binding/VMCore.h"

namespace MCD {
namespace Binding {

// AudioSourceComponent

SCRIPT_CLASS_DECLAR(AudioSourceComponent);
SCRIPT_CLASS_REGISTER(AudioSourceComponent)
	.declareClass<AudioSourceComponent, Component>("AudioSourceComponent")
	.constructor()
	.var("play", &AudioSourceComponent::play)
	.var("loop", &AudioSourceComponent::loop)
	.var("time", &AudioSourceComponent::time)
	.varGet("totalTime", &AudioSourceComponent::totalTime)
	.var("volume", &AudioSourceComponent::volume)
	.var("useTransform", &AudioSourceComponent::useTransform)
	.var("destroyAfterFinish", &AudioSourceComponent::destroyAfterFinish)
	.var("destroyEntityAfterFinish", &AudioSourceComponent::destroyEntityAfterFinish)
	.var("filePath", &AudioSourceComponent::filePath)
	.var("loadOptions", &AudioSourceComponent::loadOptions)
;}

void registerAudioBinding(VMCore& vm)
{
	Binding::ClassTraits<AudioSourceComponent>::bind(&vm);
}

}	// namespace Binding
}	// namespace MCD
