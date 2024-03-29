#include "Pch.h"
#include "MeshBuilderUtility.h"
#include "MeshBuilder.h"
#include "../Core/Math/Vec3.h"
#include "../Core/System/Log.h"

namespace MCD {

bool MeshBuilderUtility::copyVertexAttributes(
	MeshBuilder& srcBuilder, MeshBuilder& destBuilder,
	FixStrideArray<uint16_t> srcIndex, FixStrideArray<uint16_t> destIndex)
{
	const size_t bufferCount = srcBuilder.bufferCount();

	if(bufferCount != destBuilder.bufferCount()) return false;
	const size_t destVertexCount = destBuilder.vertexCount();

	if(!srcIndex.data) return false;
	if(destIndex.size && srcIndex.size != destIndex.size) return false;
	if(destBuilder.vertexCount() < srcIndex.size && !destBuilder.resizeVertexBuffer(uint16_t(srcIndex.size))) return false;

	for(size_t i=1; i<bufferCount; ++i)	// We skip the first buffer, which is index buffer
	{
		size_t elementSize1, totalSize1;
		size_t elementSize2, totalSize2;
		const char* p1 = srcBuilder.getBufferPointer(i, &elementSize1, &totalSize1);
		char* p2 = destBuilder.getBufferPointer(i, &elementSize2, &totalSize2);

		if(!p1 || !p2 || elementSize1 != elementSize2) return false;
		const size_t srcVertexCount = totalSize1 / elementSize1;

		if(destIndex.data && destIndex.size)
			for(size_t j=0; j<srcIndex.size; ++j) {
				if(srcIndex[j] >= srcVertexCount) return false;
				if(destIndex[j] >= destVertexCount) return false;
				::memcpy(p2 + destIndex[j] * elementSize2, p1 + srcIndex[j] * elementSize1, elementSize1);
			}
		else
			for(size_t j=0; j<srcIndex.size; ++j) {
				if(srcIndex[j] >= srcVertexCount) return false;
				::memcpy(p2 + j * elementSize2, p1 + srcIndex[j] * elementSize1, elementSize1);
			}
	}

	return true;
}

/*!	\param srcBuilder Contains a large buffer of vertex attribute, the inside index buffer is ignored.
	\param outBuilders Pointer to an array of MeshBuilder. We will fill both vertex and index data into them.
	\param faceIndices An array of index which indexing the srcBuilder vertex.
 */
void MeshBuilderUtility::split(size_t splitCount, MeshBuilder& srcBuilder, MeshBuilder** outBuilders, StrideArray<uint16_t>* faceIndices)
{
	if(splitCount == 0)
		return;

	MCD_ASSERT(srcBuilder.vertexCount() < uint16_t(-1) && "uint16_t(-1) is reserved for error indication");

	// Multiplex the declarations from srcBuilder to outBuilders
	for(size_t i=0; i<splitCount; ++i) {
		outBuilders[i]->clear();
		for(size_t j=1; j<srcBuilder.attributeCount(); ++j) {
			size_t bufferId;
			VertexFormat format;
			if(srcBuilder.getAttributePointer(j, nullptr, nullptr, &bufferId, nullptr, &format))
				outBuilders[i]->declareAttribute(format, bufferId);
		}
	}

	// A map that use old index as key to find the new index.
	std::vector<uint16_t> idxMap, uniqueIdx;
	idxMap.resize(srcBuilder.vertexCount());

	for(size_t i=0; i<splitCount; ++i)
	{
		const StrideArray<uint16_t> srcIdx = faceIndices[i];
		idxMap.assign(idxMap.size(), uint16_t(-1));

		uniqueIdx.clear();
		uint16_t uniqueVertexCount = 0;
		MCD_VERIFY(outBuilders[i]->resizeIndexBuffer(srcIdx.size));
		StrideArray<uint16_t> outIdx(reinterpret_cast<uint16_t*>(outBuilders[i]->getBufferPointer(0)), srcIdx.size); 

		// Build up the unique index map
		for(size_t j=0; j<srcIdx.size; ++j) {
			uint16_t& val = idxMap[srcIdx[j]];
			if(val == uint16_t(-1)) {
				val = uniqueVertexCount++;
				uniqueIdx.push_back(srcIdx[j]);
			}
			outIdx[j] = val;
		}

		MCD_ASSERT(uniqueVertexCount <= srcIdx.size);
		MCD_VERIFY(outBuilders[i]->resizeVertexBuffer(uniqueVertexCount));

		MCD_VERIFY(copyVertexAttributes(
			srcBuilder, *outBuilders[i],
			FixStrideArray<uint16_t>(&uniqueIdx[0], uniqueIdx.size())
		));
	}
}

void MeshBuilderUtility::computNormal(MeshBuilder& builder, size_t whichBufferIdStoreNormal)
{
	// In case that the builder doesn't has a normal attribute yet.
	builder.declareAttribute(VertexFormat::get("normal"), whichBufferIdStoreNormal);

	const size_t indexCount = builder.indexCount();
	const size_t vertexCount = builder.vertexCount();

	const StrideArray<uint16_t> index = builder.getAttributeAs<uint16_t>(builder.findAttributeId("index"));
	const StrideArray<Vec3f> vertex = builder.getAttributeAs<Vec3f>(builder.findAttributeId("position"));
	StrideArray<Vec3f> normal = builder.getAttributeAs<Vec3f>(builder.findAttributeId("normal"));

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
		// should be influenced more by a larger polygon.
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
