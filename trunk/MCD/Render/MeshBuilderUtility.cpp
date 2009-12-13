#include "Pch.h"
#include "MeshBuilderUtility.h"
#include "MeshBuilder.h"
#include "SemanticMap.h"
#include "../Core/Math/Vec3.h"
#include "../Core/System/Log.h"
#include <map>

namespace MCD {

bool MeshBuilderUtility::copyVertexAttributes(
	MeshBuilder2& srcBuilder, MeshBuilder2& destBuilder,
	FixStrideArray<uint16_t> srcIndex, FixStrideArray<uint16_t> destIndex)
{
	const size_t bufferCount = srcBuilder.bufferCount();

	if(bufferCount != destBuilder.bufferCount()) return false;
	if(srcIndex.size != destIndex.size) return false;

	for(size_t i=1; i<bufferCount; ++i)	// We skip the first buffer, which is index buffer
	{
		size_t elementSize1, totalSize1;
		size_t elementSize2, totalSize2;
		const char* p1 = srcBuilder.getBufferPointer(i, &elementSize1, &totalSize1);
		char* p2 = destBuilder.getBufferPointer(i, &elementSize2, &totalSize2);

		if(!p1 || !p2 || elementSize1 != elementSize2) return false;
		const size_t srcVertexCount = totalSize1 / elementSize1;
		const size_t destVertexCount = totalSize2 / elementSize2;

		for(size_t j=0; j<srcIndex.size; ++j) {
			if(srcIndex[j] >= srcVertexCount) return false;
			if(destIndex[j] >= destVertexCount) return false;
			::memcpy(p2 + destIndex[j] * elementSize2, p1 + srcIndex[j] * elementSize1, elementSize1);
		}
	}

	// Assign index to destBuilder
	destBuilder.resizeBuffers(destBuilder.vertexCount(), destIndex.size);
	StrideArray<uint16_t> idx = destBuilder.getAttributeAs<uint16_t>(0);
	for(size_t i=0; i<destIndex.size; ++i)
		idx[i] = destIndex[i];

	return true;
}

/*!	\param srcBuilder Contains a large buffer of vertex attribute, the inside index buffer is ignored.
	\param outBuilders Pointer to an array of MeshBuilder. We will fill both vertex and index data into them.
	\param faceIndices An array of index which those index are indexing the srcBuilder.
 */
void MeshBuilderUtility::split(size_t splitCount, MeshBuilder2& srcBuilder, MeshBuilderIM* outBuilders, StrideArray<uint16_t>* faceIndices)
{
	MCD_ASSERT(srcBuilder.vertexCount() < uint16_t(-1) && "uint16_t(-1) is reserved for error indication");

	// Multiplex the declarations from srcBuilder to outBuilders
	for(size_t i=0; i<splitCount; ++i) {
		outBuilders[i].clear();
		for(size_t j=1; j<srcBuilder.attributeCount(); ++j) {
			MeshBuilder2::Semantic semantic;
			if(srcBuilder.getAttributePointer(j, nullptr, nullptr, &semantic))
				outBuilders[i].declareAttribute(semantic, 1);	// TODO: Get the buffer ID from srcBuilder
		}
	}

	// A map that use old index as key to find the new index.
	std::vector<uint16_t> idxMap, outIdx;
	idxMap.resize(srcBuilder.vertexCount());

	for(size_t i=0; i<splitCount; ++i)
	{
		const StrideArray<uint16_t> srcIdx = faceIndices[i];
		idxMap.assign(idxMap.size(), uint16_t(-1));

		outIdx.resize(srcIdx.size);
		uint16_t uniqueVertexCount = 0;

		// Assign the index map first
		for(size_t j=0; j<srcIdx.size; ++j) {
			uint16_t& val = idxMap[srcIdx[j]];
			if(val == uint16_t(-1))
				val = uniqueVertexCount++;
			outIdx[j] = val;
		}

		outBuilders[i].resizeBuffers(uniqueVertexCount, outIdx.size());

		copyVertexAttributes(
			srcBuilder, outBuilders[i],
			FixStrideArray<uint16_t>(srcIdx.data, srcIdx.size),
			FixStrideArray<uint16_t>(&outIdx[0], outIdx.size())
		);
	}
}

void MeshBuilderUtility::computNormal(MeshBuilder2& builder, size_t whichBufferIdStoreNormal)
{
	const SemanticMap& map = SemanticMap::getSingleton();

	if(!builder.declareAttribute(map.normal(), whichBufferIdStoreNormal))
		Log::write(Log::Warn, L"The mesh already has a normal semantic");

	const size_t indexCount = builder.indexCount();
	const size_t vertexCount = builder.vertexCount();

	const StrideArray<uint16_t> index = builder.getAttributeAs<uint16_t>(builder.findAttributeId(map.index().name));
	const StrideArray<Vec3f> vertex = builder.getAttributeAs<Vec3f>(builder.findAttributeId(map.position().name));
	StrideArray<Vec3f> normal = builder.getAttributeAs<Vec3f>(builder.findAttributeId(map.normal().name));

	// Initialize the normal to zero first
	for(size_t i=0; i<vertexCount; ++i)
		normal[i] = Vec3f::cZero;

	// Calculate the face normal for each face
	for(size_t i=0; i<indexCount; i+=3) {
		uint16_t i0 = index[i+0];
		uint16_t i1 = index[i+1];
		uint16_t i2 = index[i+2];
		const Vec3f& v1 = vertex[i0];
		const Vec3f& v2 = vertex[i1];
		const Vec3f& v3 = vertex[i2];

		// We need not to normalize this faceNormal, since a vertex's normal
		// should be influenced by a larger polygon.
		const Vec3f faceNormal = (v3 - v2) ^ (v1 - v2);

		// Add the face normal to the corresponding vertices
		normal[i0] += faceNormal;
		normal[i1] += faceNormal;
		normal[i2] += faceNormal;
	}

	// Normalize for each vertex normal
	for(size_t i=0; i<vertexCount; ++i)
		normal[i].normalize();
}

}	// namespace MCD
