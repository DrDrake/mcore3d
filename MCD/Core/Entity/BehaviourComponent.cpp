#include "Pch.h"
#include "BehaviourComponent.h"
#include "Entity.h"

namespace MCD {

static BehaviourUpdaterComponent* gCurrentBehaviourUpdater = nullptr;

void BehaviourComponent::gather()
{
	MCD_ASSUME(gCurrentBehaviourUpdater);
	gCurrentBehaviourUpdater->mComponents.push_back(this);
}

void BehaviourUpdaterComponent::begin()
{
	gCurrentBehaviourUpdater = this;
}

void BehaviourUpdaterComponent::end(float dt)
{
	MCD_FOREACH(const BehaviourComponentPtr c, mComponents)
		if(c) c->update(dt);
	mComponents.clear();
	gCurrentBehaviourUpdater = nullptr;
}

}	// namespace MCD
