#include "Pch.h"

#include "../MathConvertor.h"
#include "CollisionShape.h"

#include "../../3Party/bullet/btBulletCollisionCommon.h"

using namespace MCD;
using namespace MCD::BulletBinding;

CollisionShape::CollisionShape(btCollisionShape* shape)
{
	mShape.reset(shape);
}

SphereShape::SphereShape(float radius)
{
	mShape.reset(new btSphereShape(radius));
}

StaticPlaneShape::StaticPlaneShape(const Vec3f& planeNormal, float planeConstant)
{
	mShape.reset(new btStaticPlaneShape(MathConvertor::ToBullet(planeNormal), planeConstant));
}