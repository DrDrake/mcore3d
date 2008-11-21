#ifndef __MCD_RENDERTEST_DEFAULTRESOURCEMANAGER__
#define __MCD_RENDERTEST_DEFAULTRESOURCEMANAGER__

#include "../../MCD/Core/System/ResourceManager.h"

class DefaultResourceManager : public MCD::ResourceManager
{
public:
	explicit DefaultResourceManager(const MCD::Path& rootPath);

	explicit DefaultResourceManager(MCD::IFileSystem& fileSystem);

	/*!	
		\return
			>0 - A resource is sucessfully loaded (partial or full).
			=0 - There are no more event to process at this moment.
			<0 - The resource failed to load.
	 */
	virtual int processLoadingEvents();

protected:
	void setupFactories();
};	// DefaultResourceManager

#endif	// __MCD_RENDERTEST_DEFAULTRESOURCEMANAGER__
