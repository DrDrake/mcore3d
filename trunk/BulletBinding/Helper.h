
// Tempoary hack
btVector3 Vec3fTobtVector3(const MCD::Vec3f v)
{
	return btVector3(v.x, v.y, v.z);
}


MCD::Vec3f btVector3ToVec3f(const btVector3& v)
{
	return MCD::Vec3f(v.x(), v.y(), v.z());
}
