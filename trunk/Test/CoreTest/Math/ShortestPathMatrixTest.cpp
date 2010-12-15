#include "Pch.h"
#include "../../../MCD/Core/Math/ShortestPathMatrix.h"

using namespace MCD;

TEST(ShortestPathMatrixTest)
{
	ShortestPathMatrix m;

	/*	Example: A -> B -> C
		Adjacency matrix, (-1 means infinity):
		-1,  1, -1
		-1, -1,  1
		-1, -1, -1
	 */
	m.resize(3);
	m.distance(0,1) = 1;	// A connect to B
	m.distance(1,2) = 1;	// B connect to C
	m.preProcess();

	CHECK_EQUAL(-1, m.getNext(0, 0));
	CHECK_EQUAL( 1, m.getNext(0, 1));
	CHECK_EQUAL( 1, m.getNext(0, 2));
	CHECK_EQUAL(-1, m.getNext(1, 0));
	CHECK_EQUAL(-1, m.getNext(1, 1));
	CHECK_EQUAL( 2, m.getNext(1, 2));
	CHECK_EQUAL(-1, m.getNext(2, 0));
	CHECK_EQUAL(-1, m.getNext(2, 1));
	CHECK_EQUAL(-1, m.getNext(2, 2));
}
