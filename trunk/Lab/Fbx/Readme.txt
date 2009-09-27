To build the Fbx module:
1. download the http://mcore3d.googlecode.com/files/FbxSdk_2010.0.2.7z
2. Extract the contents (i.e. the 'include' & 'lib' folder) to "$(MCDROOT)\Lab\Fbx\FbxSdk_2010.0.2"

To use the FbxModelLoader simply use the following code:

#include "Fbx/FbxModelLoader.h"

// add the loader to the ResourceManager, do this once only
mResMgr.addFactory(new FbxModelLoaderFactory(mResMgr));

// load the fbx file as Model
MCD::Model* model = dynamic_cast<MCD::Model*>(mResMgr.load("test.fbx").get());