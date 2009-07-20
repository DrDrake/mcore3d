#include "../../../3Party/bullet/btBulletDynamicsCommon.h"
#include <vector>

namespace MCD {

class DynamicsWorld::Impl
{
public:
	Impl();
	~Impl();

	btAxisSweep3* mBroadphase;
	btDefaultCollisionConfiguration* mCollisionConfiguration;
	btCollisionDispatcher* mDispatcher;
	btSequentialImpulseConstraintSolver* mSolver;
	btDynamicsWorld* mDynamicsWorld;
};	// Impl

}	// namespace MCD
