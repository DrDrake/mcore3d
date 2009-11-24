#include "Pch.h"
#include "MeshBuilderUtility.h"
#include "MeshBuilder.h"
#include "SemanticMap.h"
#include "../Core/Math/Vec3.h"

namespace MCD {

void MeshBuilderUtility::split(size_t splitCount, MeshBuilder2& srcBuilder, MeshBuilderIM* outBuilders, std::vector<uint16_t>* faceIndices)
{
	for(size_t i=0; i<splitCount; ++i)
	{
		// Multiplex the declarations from srcBuilder to outBuilders
		outBuilders[i].clear();
		for(size_t j=1; j<srcBuilder.attributeCount(); ++i) {
			MeshBuilder2::Semantic semantic;
			if(srcBuilder.getAttributePointer(i, nullptr, nullptr, &semantic))
				outBuilders[i].declareAttribute(semantic, 1);	// TODO: Get the buffer ID from srcBuilder
		}

//		const std::vector<uint16_t>& indices = faceIndices[i];
//		outBuilders[i].resizeBuffers(srcBuilder.vertexCount(), srcBuilder.indexCount());
	}
}

void MeshBuilderUtility::computNormal(MeshBuilder2& builder, size_t whichBufferIdStoreNormal)
{
	const SemanticMap& map = SemanticMap::getSingleton();
	// TODO: Log warning if builder already declared normal attribute.
	builder.declareAttribute(map.normal(), whichBufferIdStoreNormal);

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
