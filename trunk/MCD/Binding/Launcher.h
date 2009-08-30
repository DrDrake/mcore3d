#ifndef __MCD_BINDING_LAUNCHER__
#define __MCD_BINDING_LAUNCHER__

#include "ScriptComponentManager.h"
#include "../Component/Physics/ThreadedDynamicWorld.h"
#include "../Core/System/Timer.h"
#include "../Core/System/Thread.h"
#include "../Core/System/WeakPtr.h"

namespace MCD {

class IResourceManager;
typedef WeakPtr<class InputComponent> InputComponentPtr;

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

protected:
	static Launcher* mSingleton;

	Entity* mRootNode;
	InputComponentPtr mInputComponent;

	FrameTimer mFrameTimer;
	IFileSystem& fileSystem;
	IResourceManager* mResourceManager;
	Thread mPhysicsThread;
	ThreadedDynamicsWorld mDynamicsWorld;

public:
// Operations:
	/*!	Perform initialization.
		It will take over the ownership of inputComponent.
		\param rootNode Optional user supplied Entity root node.
	 */
	sal_checkreturn bool init(InputComponent& inputComponent, Entity* rootNode=nullptr);

	/*!	Loads a tree of entity from a file.
		Since the load will performed asynchrously, this function will return an empty
		Entity immediatly, and more child will be added when load complete.
		Currently only *.3ds is supported.
	 */
	sal_notnull Entity* loadEntity(const wchar_t* filePath, bool createCollisionMesh);

	sal_override void update();

// Attributes:
	sal_notnull Entity* rootNode() {
		return mRootNode;
	}

	/*!	Assign a node as the root node of Launcher.
		Previous root node will be deleted.
	 */
	void setRootNode(sal_in_opt Entity* e);

	sal_maybenull InputComponent* inputComponent();

	/*!	Assign a new input component.
		Previous input component will be deleted.
	 */
	void setInputComponent(sal_in_opt InputComponent* inputComponent);

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

	ScriptComponentManager scriptComponentManager;
};	// Launcher

}	// namespace MCD

#endif	// __MCD_BINDING_LAUNCHER__
