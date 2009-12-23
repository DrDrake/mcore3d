#ifndef __MCD_BINDING_LAUNCHER__
#define __MCD_BINDING_LAUNCHER__

#include "Binding.h"	// For definiation of ScriptVM
#include "ScriptComponentManager.h"
#include "../Component/Physics/ThreadedDynamicWorld.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/Timer.h"
#include "../Core/System/Thread.h"
#include "../Core/System/WeakPtr.h"

namespace MCD {

class IResourceManager;
typedef WeakPtr<class InputComponent> InputComponentPtr;

class MCD_BINDING_API Launcher
{
public:
	//! Will not take ownership of fileSystem.
	Launcher(
		IFileSystem& fileSystem,
		IResourceManager& resourceManager,
		bool takeResourceManagerOwnership = true
	);

	~Launcher();

// Operations:
	/*!	Call this to host the remote debugger server.
		It is advised to call this before init()
	 */
	sal_checkreturn bool enableDebugger(size_t port, bool blockUntilConnected);

	/*!	Perform initialization.
		It will take over the ownership of inputComponent.
		If the inputComponent is already attached to an Entity, it will continue to host in that Entity.
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
		Ownership to the Previous root node will be released.
	 */
	void setRootNode(sal_in_opt Entity* e);

	sal_maybenull InputComponent* inputComponent();

	/*!	Assign a new input component.
		Previous input component will keep in it's orginal Entity.
		An Entity will be created if the incomming inputComponent doesn't have one already.
	 */
	void setInputComponent(sal_in_opt InputComponent* inputComponent);

	sal_notnull IResourceManager* resourceManager() {
		return mResourceManager;
	}

	sal_notnull ThreadedDynamicsWorld* dynamicsWorld() {
		return &mDynamicsWorld;
	}

	static Launcher* sinleton() {
		return Launcher::mSingleton;
	}

	ScriptVM vm;

	ScriptComponentManager scriptComponentManager;

protected:
	static Launcher* mSingleton;

	IFileSystem& mFileSystem;
	Entity* mRootNode;
	InputComponentPtr mInputComponent;

	void* mDbgContext;

	DeltaTimer mDeltaTimer;

	IResourceManager* mResourceManager;
	bool mTakeResourceManagerOwnership;
	Thread mPhysicsThread;
	ThreadedDynamicsWorld mDynamicsWorld;
};	// Launcher

//! A default implementation of ResourceManager for Launcher
class MCD_BINDING_API LauncherDefaultResourceManager : public ResourceManager
{
public:
	explicit LauncherDefaultResourceManager(IFileSystem& fileSystem, bool takeFileSystemOwnership=true);

	/*!
		\return
			>0 - A resource is sucessfully loaded (partial or full).
			=0 - There are no more event to process at this moment.
			<0 - The resource failed to load.
	 */
	sal_override int update();
};	// LauncherDefaultResourceManager

}	// namespace MCD

#endif	// __MCD_BINDING_LAUNCHER__
