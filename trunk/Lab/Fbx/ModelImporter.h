#ifndef __MCD_FBX_MODELIMPORTER__
#define __MCD_FBX_MODELIMPORTER__

#include "../../MCD/Render/Model.h"

namespace MCD
{

class FbxFile;
class IResourceManager;

class MCD_FBX_API ModelImporter
{
public:
    ModelImporter(sal_maybenull IResourceManager* resMgr = nullptr);

	void import(FbxFile& fbxfile, Model& outModel, bool editable);

private:
	IResourceManager* mResMgr;
};

}	// namespace MCD

#endif  // #ifndef __MCD_FBX_MODELIMPORTER__
