#ifndef __MCD_CORE_MATH_SHORTESTPATHMATRIX__
#define __MCD_CORE_MATH_SHORTESTPATHMATRIX__

#include "../System/Platform.h"
#include <vector>

namespace MCD {

/*!	For calculating shortest path.
	The Floyd¡VWarshall algorithm is used.
	Reference:
	http://en.wikipedia.org/wiki/Floyd%E2%80%93Warshall_algorithm
	http://compprog.wordpress.com/2007/11/29/one-source-shortest-path-the-bellman-ford-algorithm/

	Example: A -> B -> C
	Adjacency matrix, (-1 means infinity):
	-1,  1, -1
	-1, -1,  1
	-1, -1, -1

	ShortestPathMatrix m;
	m.resize(3);
	m.distance(0,1) = 1;	// A connect to B
	m.distance(1,2) = 1;	// B connect to C
	m.preProcess();

	int v = m.getNext(0, 2);// Find the path from A to C
	assert(v == 1);			// The intermediate result should be B

	v = m.getNext(v, 2);	// Find the path from B to C
	assert(v == 2);
 */
class ShortestPathMatrix
{
public:
	ShortestPathMatrix() : vertexCount(0) {}

// Operations
	void resize(size_t vertexCount_)
	{
		vertexCount = vertexCount_;
		m.resize(vertexCount * vertexCount * 2, size_t(-1));
	}

	/// Call this before you try to call getNext()
	void preProcess()
	{
		// Assign the direct "next" value from the initial distance information
		const size_t N = vertexCount;
		for(size_t i=0; i<N; ++i) for(size_t j=0; j<N; ++j) {
			if( distance(i,j) != size_t(-1) )
				next(i,j) = j;
		}

		// Perform the search iteration with complexity of O(N^3)
		for(size_t k=0; k<N; ++k) for(size_t i=0; i<N; ++i) for(size_t j=0; j<N; ++j) {
			if( distance(i,k) + distance(k,j) < distance(i,j) ) {
				distance(i,j) = distance(i,k) + distance(k,j);
				next(i,j) = k;
			}
		}
	}

	/// Return the next node inorder to traverse the required path
	/// Return -1 if there is no path in between
	/// int node = 1;			// Starting node
	/// int targetNode = 10;	// The targeting node
	/// while(node != -1 && node != targetNode)
	///		node = ShortestPathMatrix.getNext(node, targetNode);
	int getNext(size_t from, size_t to)
	{
		if(distance(from, to) > 9999) return -1;
		return (int)next(from, to);
	}

// Attributes
	/// Get/set the distance value
	size_t& distance(size_t from, size_t to) { return m[from * vertexCount + to]; }

	size_t vertexCount;

protected:
	size_t& next(size_t i, size_t j) { return m[vertexCount*vertexCount + i*vertexCount + j]; }

	/// First half of the data stores the distance value, second half store the path
	std::vector<size_t> m;
};	// ShortestPathMatrix

}	// namespace MCD

#endif	// __MCD_CORE_MATH_SHORTESTPATHMATRIX__
