function println(str) {
	print(str);
	print("\n");
}

v100 <- Vec3(1, 0, 0);
v010 <- Vec3(0, 1, 0);
v001 <- Vec3(0, 0, 1);
v110 <- Vec3(1, 1, 0);
v123 <- Vec3(1, 2, 3);
v220 <- Vec3(2, 2, 0);

function basicTest()
{
	local v1 = Vec3();
	v1.x = 1.0; v1.y = 2.0; v1.z = 3.0;

	assert(v1.x == 1);
	assert(v1.y == 2);
	assert(v1.z == 3);

	local v2 = Vec3(1, 2, 3);
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
	local p = Vec3(1, 2, 3);
	local v = Vec3(10000.0);
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
