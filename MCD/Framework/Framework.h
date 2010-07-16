#ifndef __MCD_FRAMEWORK_FRAMEWORK__
#define __MCD_FRAMEWORK_FRAMEWORK__

#include "ShareLib.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/WeakPtr.h"
#include "../Core/System/WindowEvent.h"

namespace MCD {

class Entity;
class FileSystemCollection;
class Path;
class PrefabLoaderComponent;
class TaskPool;
class Window;
typedef IntrusivePtr<class Prefab> PrefabPtr;
typedef IntrusiveWeakPtr<class RendererComponent> RendererComponentPtr;

/// A framework that integrate various MCore modules.
class MCD_FRAMEWORK_API Framework : private Noncopyable
{
public:
	Framework();

	virtual ~Framework();

// Operations
	///	Create the main window using an already initialized window.
	sal_checkreturn bool initWindow(Window& existingWindow, bool takeOwership=true);

	///	Create the main window with the option string, implies taking ownership.
	sal_checkreturn bool initWindow(sal_in_z const char* args);

	/// Absolute/relative path to a folder or a zip file
	sal_checkreturn bool addFileSystem(sal_in_z const char* path);

	sal_checkreturn bool removeFileSystem(sal_in_z const char* path);

	/// Adds a resource factory to the reousrce manager.
	/// \note A wrapper method to the ResourceManager::addFactory().
	/// \sa ResourceManager::addFactory()
	void addLoaderFactory(sal_in ResourceManager::IFactory* factory);

	/// Load a prefab from a resource path, and put a PrefabLoaderComponent
	/// under the specified Entity.
	/// \return Null if failed.
	PrefabLoaderComponent* loadPrefabTo(sal_in_z const Path& resourcePath, sal_in_opt Entity* location, IResourceManager::BlockingMode blockingMode = IResourceManager::NonBlock, sal_in_z const char* args = nullptr);

	/// Pop an event out of the event queue.
	/// \sa Window::popEvent()
	/// \param event Output parameter to get the event.
	/// \return false if the event queue is empty.
	bool update(Event& e);

// Attributes
	FileSystemCollection& fileSystemCollection();

	ResourceManager& resourceManager();

	TaskPool& taskPool();

	Entity& rootEntity();
	Entity& systemEntity();
	Entity& sceneLayer();
	Entity& guiLayer();

	sal_maybenull Window* window();

	RendererComponentPtr rendererComponent();

protected:
	void processLoadingEvents();

	class Impl;
	Impl& mImpl;
};	// Framework

}	// namespace MCD

#endif	// __MCD_FRAMEWORK_FRAMEWORK__

