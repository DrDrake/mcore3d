#ifndef __MCD_BINDING_SCRIPTCOMPONENTMANAGER__
#define __MCD_BINDING_SCRIPTCOMPONENTMANAGER__

#include "Binding.h"
#include "../Core/System/NonCopyable.h"

namespace MCD {

class Entity;
class IFileSystem;
class Path;

/*!	A centralized class to control the ScriptComponent.
	In order to execute the ScriptComponent, this class host a
	Squirrel VM and bind some necessary global script variables and
	functions.
 */
class MCD_BINDING_API ScriptComponentManager : Noncopyable
{
public:
	//! We will use the supplied file system to load script.
	ScriptComponentManager(IFileSystem& fs);

	/*!	Set the entity to appear as the root entity in the script VM.
		\note The VM take no ownership of the entity.
		\note Make sure the ScriptComponentManager destroy after the entity.
	 */
	void registerRootEntity(Entity& entity);

	/*!	Load and execute a script file.
		The script file will be search inside \em fileSystem.
		\param pushRetVal Push the return value (if any) onto the VM stack.
		\return False if file not found or compile error or run-time error.
	 */
	sal_checkreturn bool doFile(const Path& filePath, bool pushRetVal);

	sal_maybenull Entity* runScripAsEntity(const wchar_t* scriptCode);

	//! Invoke all active script components' update() function.
	void updateScriptComponents();

	//! Do cleanups
	void shutdown();

	ScriptVM vm;
	IFileSystem& fileSystem;
};	// ScriptComponentManager

}	// namespace script

#endif	// __MCD_BINDING_SCRIPTCOMPONENTMANAGER__
