#ifndef __MCD_FBX_MODELIMPORTER__
#define __MCD_FBX_MODELIMPORTER__

#include "../../MCD/Render/Model.h"

namespace MCD
{

class FbxFile;
class ErrorList;

class MCD_FBX_API ModelImporter
{
public:
    ModelImporter();

	void import(FbxFile& fbxfile, Model& outModel);
};

}	// namespace MCD

#endif  // #ifndef __MCD_FBX_MODELIMPORTER__
