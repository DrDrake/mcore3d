#ifndef __MCD_BINDING_LAUNCHER__
#define __MCD_BINDING_LAUNCHER__

#include "ScriptComponentManager.h"
#include "PhysicsComponent.h"
#include "../Core/System/Timer.h"
#include "../Core/System/Thread.h"

namespace MCD {

class InputComponent;
class IResourceManager;

class MCD_BINDING_API Launcher
{
public:
	Launcher();
	~Launcher();

	//! A timer to measure the fps
	class FrameTimer : protected DeltaTimer
	{
	public:
		FrameTimer() : DeltaTimer(TimeInterval(1.0/60)) {}

		float frameTime() const { return float(mFrameTime.asSecond()); }
		float accumulateTime() const { return float(mAccumulateTime.asSecond()); }
		float fps() const { return 1.0f / frameTime(); }
		void nextFrame() { mFrameTime = getDelta(); mAccumulateTime = mTimer.get(); }

	protected:
		TimeInterval mFrameTime;
		TimeInterval mAccumulateTime;
	};	// FrameTimer

// Operations:
	/*!	Perform initialization.
		It will take over the ownership of inputComponent.
	 */
	void init(InputComponent& inputComponent);

	/*!	Loads a tree of entity from a file.
		Currently only *.3ds is supported.
	 */
	sal_notnull Entity* loadEntity(const wchar_t* filePath, bool createCollisionMesh);

	sal_override void update(float deltaTime);

// Attributes:
	sal_notnull Entity* rootNode() {
		return mRootNode;
	}

	sal_notnull InputComponent* inputComponent() {
		return mInputComponent;
	}

	sal_notnull IResourceManager* resourceManager() {
		return mResourceManager;
	}

	sal_notnull ThreadedDynamicsWorld* dynamicsWorld() {
		return &mDynamicsWorld;
	}

	sal_notnull FrameTimer* frameTimer() {
		return &mFrameTimer;
	}

	static Launcher* sinleton() {
		return Launcher::mSingleton;
	}

protected:
	static Launcher* mSingleton;

	Entity* mRootNode;
	InputComponent* mInputComponent;

	FrameTimer mFrameTimer;
	IFileSystem& fileSystem;
	IResourceManager* mResourceManager;
	Thread mPhysicsThread;
	ThreadedDynamicsWorld mDynamicsWorld;
	ScriptComponentManager mScriptComponentManager;
};	// Launcher

}	// namespace MCD

#endif	// __MCD_BINDING_LAUNCHER__
