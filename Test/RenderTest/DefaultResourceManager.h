#ifndef __MCD_RENDERTEST_DEFAULTRESOURCEMANAGER__
#define __MCD_RENDERTEST_DEFAULTRESOURCEMANAGER__

#include "../../MCD/Core/System/ResourceManager.h"

//! Create a FileSystemCollection that contains RawFileSystem("Media") and ZipFileSystem("Media.zip")
extern MCD::IFileSystem* createDefaultFileSystem();

class DefaultResourceManager : public MCD::ResourceManager
{
public:
	explicit DefaultResourceManager(MCD::IFileSystem& fileSystem);

	~DefaultResourceManager();

	/*!
		\return
			>0 - A resource is sucessfully loaded (partial or full).
			=0 - There are no more event to process at this moment.
			<0 - The resource failed to load.
	 */
	virtual int processLoadingEvents();

	MCD::IFileSystem& fileSystem() const;

protected:
	void setupFactories();

	class Impl;
	Impl& mImpl;
};	// DefaultResourceManager

#endif	// __MCD_RENDERTEST_DEFAULTRESOURCEMANAGER__
