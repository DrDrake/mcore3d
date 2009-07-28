dofile("squnit.nut", true);

class TestVec3
{
	v100 = null;
	v010 = null;
	v001 = null;
	v110 = null;
	v123 = null;
	v220 = null;

	function setUp()
	{
		v100 = Vec3(1, 0, 0);
		v010 = Vec3(0, 1, 0);
		v001 = Vec3(0, 0, 1);
		v110 = Vec3(1, 1, 0);
		v123 = Vec3(1, 2, 3);
		v220 = Vec3(2, 2, 0);
	}

	function testXYZ()
	{
		local v = Vec3();
		v.x = 1.0; v.y = 2.0; v.z = 3.0;

		assertEquals(v.x, 1);
		assertEquals(v.y, 2);
		assertEquals(v.z, 3);
	}

	function testConstructXYZ()
	{
		local v = Vec3(1, 2, 3);
		assert(v.isEqual(v123));
	}

	function testConstructScalar()
	{
		local v = Vec3(1);
		assertEquals(v.x, 1);
		assertEquals(v.y, 1);
		assertEquals(v.z, 1);
	}

	function testArthim()
	{
		assert((v100 + v010).isEqual(v110));
		assert((v110 - v010).isEqual(v100));
		assert((v110 * 2.0).isEqual(v220));
	}

	function testDotProduct()
	{
		assertEquals(2, v123 % v010);
		assertEquals(2, v010 % v123);
		assertEquals(14, v123 % v123);
		assertEquals(4, (v010 + v010) % v123);
	}

	function _testPerformance()
	{
		local p = Vec3(1, 2, 3);
		local v = Vec3(10000.0);
		local friction = 0.9;

		local iterationCount = 1000000;
		local start = clock();
		for(local i= iterationCount; i--;) {
			p.addEqual(v);
			v.mulEqual(friction);
		}
		local end = clock();

		print("Iteration per second = " + iterationCount/(end - start));
	}
}	// TestVec3

class TestMat44
{
	ma = null;
	mb = null;
	mc = null;
	md = null;
	identity = null;

	function setUp()
	{
		ma = Mat44(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
		mb = Mat44(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
		mc = Mat44(15);
		md = Mat44(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30);
		identity = Mat44(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	}

	function testElement()
	{
		assertEquals(0, ma.m00);
		assertEquals(1, ma.m01);
		assertEquals(2, ma.m02);
		assertEquals(3, ma.m03);
		assertEquals(4, ma.m10);
		
		ma.m00 = -1.2;
		assertEquals(-1.2, ma.m00);
	}

	function testArthim()
	{
		assert((ma + mb).isEqual(mc));
		assert((mc - mb).isEqual(ma));

		assert(ma.scalarMul(2).isEqual(md));

		{	local result = Mat44(
				 34,  28,  22,  16,
				178, 156, 134, 112,
				322, 284, 246, 208,
				466, 412, 358, 304
			);

			assert((ma * mb).isEqual(result));
		}
	}

	function testTranspose()
	{
		local m = ma.transpose;
		assertEquals(0, m.m00);
		assertEquals(1, m.m10);
		assertEquals(2, m.m20);
		assertEquals(3, m.m30);
		assertEquals(4, m.m01);
		assertEquals(5, m.m11);
		assertEquals(6, m.m21);
		assertEquals(7, m.m31);
	}

	function testDeterminant()
	{
		local m = Mat44(
			1, 2, 1, 1,
			2, 2, 2, 2,
			2, 1, 1, 2,
			2, 1, 0, 1);

		assertEquals(-2, m.determinant);
	}

	function testInverse()
	{
		local a = Mat44(
			1, 2, 1, 1,
			2, 2, 2, 2,
			2, 1, 1, 2,
			2, 1, 0, 1);

		local b = Mat44(
			-1,    1, -1,  1,
			 1, -0.5,  0,  0,
			-1,  1.5, -1,  0,
			 1, -1.5,  2, -1);
			 
		local ai = a.inverse;
		assert(ai.isEqual(b));
		assert(ai.inverse.isEqual(a));
		assert((a * ai).isEqual(identity));
	}
}	// TestMat44

SqUnit().run();
