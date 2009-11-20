#ifndef __MCD_RENDER_SEMANTICMAP__
#define __MCD_RENDER_SEMANTICMAP__

#include "MeshBuilder.h"

namespace MCD {

/*!	The SemanticMap class is a way to unify the semantic definiation for various mesh loaders.
	User can derived this class and supply it to mesh loaders for their customized semantics.

	Implemetation of SemanticMap should make sure the "name" variable of the returning Semantic
	has a static storage.

	The default		implementation of SemanticMap has the following semantics:
	name			elementSize,		elementCount	channelIndex
	"index",		sizeof(uint16_t),	1,				0
	"position",		sizeof(float),		3,				0
	"normal",		sizeof(float),		3,				0
	"tangent",		sizeof(float),		3,				0
	"binormal",		sizeof(float),		3,				0
	"blendWeight",	sizeof(float),		1,				0
	"blendIndex",	sizeof(uint8_t),	1,				0
	"uv[i]",		sizeof(float),		[c],			[i]
	"color[i]",		[s],				[c],			[i]

	where [i], [s], [c] are depends on the input parameter.
 */
class MCD_RENDER_API SemanticMap
{
public:
	typedef MeshBuilder2::Semantic Semantic;
	virtual ~SemanticMap() {}

	virtual Semantic index() const;
	virtual Semantic position() const;
	virtual Semantic normal() const;
	virtual Semantic tangent() const;
	virtual Semantic binormal() const;
	virtual Semantic blendWeight() const;
	virtual Semantic blendIndex() const;

	/*! The uv is a special case which allow multiple channels.
		Due to the const char* ownership constrain, implementation of this class
		should impose a hard limit on the number of channels. An out of bound
		index should return a Semantic with empty string: "" as the name.
	 */
	virtual Semantic uv(size_t index, size_t elementCount) const;

	//! Almost the same as uv(), with additional elementSize (float -> 4, byte -> 1)
	virtual Semantic color(size_t index, size_t elementCount, size_t elementSize) const;

	/*!	Let user to register their own derived Semantic.
		Will take over the ownership of the input and delete previous SemanticMap.
	 */
	static void setSingleton(SemanticMap& map);

	//! It will return exact type: SemanticMap if no setSingleton() is ever called.
	static SemanticMap& getSingleton();

protected:
	SemanticMap();	// Prevent accidental instantiation.
};	// SemanticMap

}	// namespace MCD

#endif	// __MCD_RENDER_SEMANTICMAP__
