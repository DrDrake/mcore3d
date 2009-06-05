#include "Pch.h"

#ifdef NDEBUG
	#pragma comment(lib, "libbulletdynamics")
	#pragma comment(lib, "libbulletcollision")
	#pragma comment(lib, "libbulletmathd")
#else
	#pragma comment(lib, "libbulletdynamicsd")
	#pragma comment(lib, "libbulletcollisiond")
	#pragma comment(lib, "libbulletmathd")
#endif

#include "Dynamics/DynamicsWorld.h"
#include "Dynamics/RigidBodyComponent.h"
