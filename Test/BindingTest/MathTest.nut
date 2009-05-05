function println(str) {
	print(str);
	print("\n");
}

v100 <- Vec3.create();
v100.x = 1.0; v100.y = 0.0; v100.z = 0.0;
v010 <- Vec3.create();
v010.x = 0.0; v010.y = 1.0; v010.z = 0.0;
v001 <- Vec3.create();
v001.x = 1.0; v001.y = 0.0; v001.z = 1.0;
v110 <- Vec3.create();
v110.x = 1.0; v110.y = 1.0; v110.z = 0.0;
v123 <- Vec3.create();
v123.x = 1.0; v123.y = 2.0; v123.z = 3.0;
v220 <- Vec3.create();
v220.x = 2.0; v220.y = 2.0; v220.z = 0.0;

function basicTest()
{
	local v1 = Vec3.create();
	v1.x = 1.0; v1.y = 2.0; v1.z = 3.0;

	assert(v1.x == 1);
	assert(v1.y == 2);
	assert(v1.z == 3);

	local v2 = Vec3.create(1, 2, 3);
	assert(v1.isEqual(v2));
}
basicTest();

function arthimTest()
{
	assert((v100 + v010).isEqual(v110));
	assert((v110 - v010).isEqual(v100));
	assert((v110 * 2.0).isEqual(v220));
}
arthimTest();

function dotTest()
{
	assert(2 == v123 % v010);
	assert(2 == v010 % v123);
	assert(14 == v123 % v123);
	assert(4 == (v010 + v010) % v123);
}
dotTest();

function performanceTest()
{
	local p = Vec3.create(1, 2, 3);
	local v = Vec3.create(10000.0);
	local friction = 0.9;

	local iterationCount = 1000;//000;
	local start = clock();
	for(local i= iterationCount; i--;) {
		p += v;
		v *= friction;
	}
	local end = clock();

	println("Iteration per second = " +  iterationCount/(end - start));
}

performanceTest();
