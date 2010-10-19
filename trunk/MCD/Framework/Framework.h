#ifndef __MCD_FRAMEWORK_FRAMEWORK__
#define __MCD_FRAMEWORK_FRAMEWORK__

#include "ShareLib.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/WeakPtr.h"
#include "../Core/System/WindowEvent.h"

namespace MCD {

class Entity;
class BehaviourComponent;
class FileSystemCollection;
class Path;
class PrefabLoaderComponent;
class RenderWindow;
class TaskPool;
typedef IntrusivePtr<class Prefab> PrefabPtr;
typedef IntrusivePtr<class Texture> TexturePtr;
typedef IntrusiveWeakPtr<class InputComponent> InputComponentPtr;
typedef IntrusiveWeakPtr<class RendererComponent> RendererComponentPtr;

/// A framework that integrate various MCore modules.
class MCD_FRAMEWORK_API Framework : private Noncopyable
{
public:
	Framework();

	virtual ~Framework();

// Operations
	///	Create the main window using an already initialized window.
	sal_checkreturn bool initWindow(RenderWindow& existingWindow, bool takeOwership=true);

	///	Create the main window with the option string, implies taking ownership.
	sal_checkreturn bool initWindow(sal_in_z const char* args);

	/// Absolute/relative path to a folder or a zip file
	sal_checkreturn bool addFileSystem(sal_in_z const char* path);

	sal_checkreturn bool removeFileSystem(sal_in_z const char* path);

	/// Adds a resource factory to the reousrce manager.
	/// \note A wrapper method to the ResourceManager::addFactory().
	/// \sa ResourceManager::addFactory()
	void addLoaderFactory(sal_in ResourceManager::IFactory* factory);

	/// Pop an event out of the event queue.
	/// \sa Window::popEvent()
	/// \param event Output parameter to get the event.
	/// \return false if the event queue is empty.
	bool update(Event& e);

// Higher level operations
	TexturePtr loadTexture(sal_in_z const char* path, int blockIteration=-1);

	/// Load a prefab from a resource path, and put a PrefabLoaderComponent under the specified Entity location.
	/// \return Null if failed.
	PrefabLoaderComponent* loadPrefabTo(sal_in_z const char* path, Entity& location, bool blockingLoad=true);

	void registerResourceCallback(sal_in_z const char* path, BehaviourComponent& callback, bool isRecursive, int minLoadIteration=-1);

// Attributes
	FileSystemCollection& fileSystemCollection();

	ResourceManager& resourceManager();

	TaskPool& taskPool();

	Entity& rootEntity();
	Entity& systemEntity();
	Entity& sceneLayer();
	Entity& guiLayer();

	sal_maybenull RenderWindow* window();

	RendererComponentPtr rendererComponent();

	InputComponentPtr inputComponent();

	float dt() const;	//!< Duration of LAST frame
	float fps() const;	//!< Frame per second over the last second

protected:
	class Impl;
	Impl& mImpl;
};	// Framework

}	// namespace MCD

#endif	// __MCD_FRAMEWORK_FRAMEWORK__

