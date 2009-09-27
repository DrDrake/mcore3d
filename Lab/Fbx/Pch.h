
#include "../../MCD/Core/System/Platform.h"
#include "../../MCD/Core/System/LinkList.h"
#include "../../MCD/Core/System/Timer.h"
#include "../../MCD/Core/System/Log.h"
#include "../../MCD/Core/System/StrUtility.h"

#include "../../MCD/Core/System/Platform.h"
#include "../../MCD/Core/Math/BasicFunction.h"
#include "../../MCD/Core/Math/Mat33.h"
#include "../../MCD/Core/Math/Mat44.h"
#include "../../MCD/Core/Math/Vec2.h"
#include "../../MCD/Core/Math/Vec3.h"
#include "../../MCD/Core/Math/Quaternion.h"

#include "ShareLib.h"

#include <string>

// please notice that this module assume the FbxSDK 2009.3 is already installed
// and configured in the compiling environment
#define KFBX_PLUGIN
#define KFBX_FBXSDK
#define KFBX_NODLL
//#pragma warning(push)
#pragma warning(disable:4505)
#include <fbxsdk.h>
#include <fbxfilesdk/fbxfilesdk_nsuse.h>
//#pragma warning(pop)

using namespace MCD;