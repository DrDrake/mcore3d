#include "Pch.h"

#include "Dynamics/DynamicsWorld.h"
#include "Dynamics/RigidBodyComponent.h"

#ifdef NDEBUG

#pragma comment(lib, "libbulletdynamics")
#pragma comment(lib, "libbulletcollision")
#pragma comment(lib, "libbulletmathd")
#pragma comment(lib, "MCDCore")

#else

#pragma comment(lib, "libbulletdynamicsd")
#pragma comment(lib, "libbulletcollisiond")
#pragma comment(lib, "libbulletmathd")
#pragma comment(lib, "MCDCored")

#endif
