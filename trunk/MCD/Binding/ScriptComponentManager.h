#ifndef __MCD_BINDING_SCRIPTCOMPONENTMANAGER__
#define __MCD_BINDING_SCRIPTCOMPONENTMANAGER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/Timer.h"

namespace MCD {

class Component;
class Entity;
class IFileSystem;
class Path;
class ScriptVM;

/*!	A centralized class to control the ScriptComponent.
	In order to execute the ScriptComponent, this class host a
	Squirrel VM and bind some necessary global script variables and
	functions.
 */
class MCD_BINDING_API ScriptComponentManager : Noncopyable
{
public:
	//! A timer to measure fps and manage the waking up of co-routine
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

public:
	ScriptComponentManager();

	~ScriptComponentManager();

	/*!	Initialize the manager with the file system and the script VM.
		The supplied file system will be used to search for script files.
		\note Will not take ownership of both \em fs and \em vm.
	 */
	sal_checkreturn bool init(ScriptVM& vm, IFileSystem& fs);

	/*!	Set the entity to appear as the root entity in the script VM.
		\note The VM take no ownership of the entity.
		\note Make sure the ScriptComponentManager destroy after the entity.
	 */
	void registerRootEntity(Entity& entity);

	/*!	Load and execute a script file.
		The script file will be search inside \em fileSystem.
		\param pushRetVal Push the return value (if any) onto the VM stack.
		\param vm The specific squirrel VM that calling this function, pass null if the main VM should be used.
		\return False if file not found or compile error or run-time error.
	 */
	sal_checkreturn bool doFile(const Path& filePath, bool pushRetVal, void* vm=nullptr);

	/*!	Run a script and return it as en Entity.
		If any error occurs or the script didn't returning an Entity, null is returned.
		\param scriptKeepOwnership Options that define C++ or the script Vm owns the Entity.
	 */
	sal_maybenull Entity* runScripAsEntity(sal_in_z const char* scriptCode, bool scriptKeepOwnership=false);

	/*!	Run a script and return it a Component.
		If any error occurs or the script didn't returning an Component, null is returned.
		\param scriptKeepOwnership Options that define C++ or the script Vm owns the Component.
	 */
	sal_maybenull Component* runScripAsComponent(sal_in_z const char* scriptCode, bool scriptKeepOwnership=false);

	//! Invoke all active script components' update() function.
	void updateScriptComponents();

	//! Do cleanups
	void shutdown();

	ScriptVM* vm;
	IFileSystem* fileSystem;
	FrameTimer frameTimer;
};	// ScriptComponentManager

}	// namespace script

#endif	// __MCD_BINDING_SCRIPTCOMPONENTMANAGER__
