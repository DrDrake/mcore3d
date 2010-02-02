#ifndef __MCD_RENDER_MESHBUILDER__
#define __MCD_RENDER_MESHBUILDER__

#include "ShareLib.h"
#include "../Core/System/Array.h"
#include "../Core/System/Atomic.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/NonCopyable.h"

namespace MCD {

/*!	Providing a simple interface for declarating (generic) vertex attributes and building the buffers.
	The buffers are then utilized and commit to Mesh.

	There is a one to one mapping between attribute ID and semantic string, the purpose of attribute ID
	is for performance only.

	Example:
	\code
	// Get the default semantic map
	const SemanticMap& map = SemanticMap::getSingleton();

	// Declare an vertex with position, normal and uv2f within a buffer, and
	// blend weight, blend index in another buffer.
	MeshBuilder builder;
	int posId = builder.declareAttribute(map.position(), 1);
	int normalId = builder.declareAttribute(map.normal(), 1);
	int uvId = builder.declareAttribute(map.uv(0, 2), 1);

	int blendWeightId = builder.declareAttribute(map.blendWeight(), 2);
	int blendIndexId = builder.declareAttribute(map.blendIndex(), 2);

	// Make 1000 of those declared vertex, and 2000 index.
	if(!resizeBuffers(1000, 2000)) return false;

	// Acquire the buffer pointer and fill up the data yourself.
	StrideArray<uint16_t> indexArray = builder.getAttributeAs<uint16_t>(0);
	StrideArray<Vec3f> posArray = builder.getAttributeAs<Vec3f>(posId);
	StrideArray<Vec3f> normalArray = builder.getAttributeAs<Vec3f>(normalId);
	// ...
	\endcode
 */
class MCD_RENDER_API MeshBuilder : public IntrusiveSharedObject<AtomicInteger>, private Noncopyable
{
public:
	enum ElementType
	{
		TYPE_NOT_USED,
		TYPE_INT,		TYPE_UINT,
		TYPE_INT8,		TYPE_UINT8,
		TYPE_INT16,		TYPE_UINT16,
		TYPE_FLOAT,		TYPE_DOUBLE,
		TYPE_NONE	//!< For error indication
	};	// ElementType

	struct Semantic
	{
		const char* name;		//! Unique string that identify the semantic
		ElementType elementType;//! The data type of the element
		size_t elementSize;		//! Size in byte of individual element in this semanitc, eg. Vec3f -> sizeof(float)
		size_t elementCount;	//! Number of element in this semanitc, eg. Vec3f -> 3
		size_t channelIndex;	//! The channel index for color/texture coordinate
	};	// Semantic

	enum StorageHint
	{
		/*! The data stored in the buffer object is unlikely to change and will be used
			possibly many times as a source for drawing. This hint tells the implementation
			to put the data somewhere it' s quick to draw from, but probably not quick to
			update.
		 */
		Static	= 0x88E4,

		/*! The data stored in the buffer object is likely to change frequently but is likely
			to be used as a source for drawing several times in between changes. This hint
			tells the implementation to put the data somewhere it won't be too painful to
			update once in a while.
		 */
		Dynamic	= 0x88E8,

		/*! The data store in the buffer object is likely to change frequently and will be
			used only once (or at least very few times) in between changes. This hint tells
			the implementation that you have time-sensitive data such as animated geometry
			that will be used once and then replaced. It is crucial that the data be
			placed somewhere quick to update, even at the expense of faster rendering.
		 */
		Stream	= 0x88E0,
	};	// StorageHint

	explicit MeshBuilder(bool isCreatedOnStack=true);

	sal_override ~MeshBuilder();

// Operations
	/*!	Call this function to declare the vertex attributes you want.
		\param semantic
			The semantic for this attribute. You can get some preset semantics using SemanticMap::getSingleton().
		\param bufferId
			Feed this param with different value if you want to group different attributes into set of buffers.
			The value zero is reserved for index buffer, otherwise you can supply successive values 1,2,3,etc...
		\return The attribute ID for future reference, -1 if any error occured.

		\note You can invoke declareAttribute() even when there are datas already in the buffers.
		\note Return failure if the semantic is already declared once.
	 */
	int declareAttribute(const Semantic& semantic, size_t bufferId=1);

	/*!	Resize the buffers.
		\note Remember only 65536 vertex is supported.
	 */
	sal_checkreturn bool resizeBuffers(uint16_t vertexCount, size_t indexCount);

	sal_checkreturn bool resizeVertexBuffer(uint16_t vertexCount);

	sal_checkreturn bool resizeIndexBuffer(size_t indexCount);

	//!	Clear the mesh builder to it's initial state.
	void clear();

	//! Clear only the buffers, keeping the declarations.
	void clearBuffers();

// Query
	/*! Number of attribtues defined.
		\note The vertex index is also accounted as an attribute.
	 */
	size_t attributeCount() const;

	uint16_t vertexCount() const;

	size_t indexCount() const;

	size_t bufferCount() const;

	/*!	Search the attribute ID with the given semantic name.
		Returns -1 if non of the semantic can be found.
	 */
	int findAttributeId(const char* semanticName) const;

	/*!	Acquire the data pointer from the internal buffer.
		This function also expose the associated properties of that attribute.
		\param attributedId
			The ID returned from declareAttribute(), or 0 for index buffer
		\param count
			The number of attributes in the buffer, may equal to indexCount() or vertexCount().
		\param stride
			The byte offset between consecutive vertex attributes, which is equal to the size of a vertex.
			Note that stride will not be zero even the data are tightly packed.
		\param offset
			Byte offset of this attribute in it's buffer.
		\param semantic
			The semantic for this attribute, as it was passed to declareAttribute().
			Since all error checking is done in declareAttribute(), the returned semantic
			by this function should be always valid.

		\note If stride == sizeInByte it means a whole buffer is dedicated to that attribute.
	 */
	sal_maybenull char* getAttributePointer(
		int attributeId,
		sal_out_opt size_t* count=nullptr,
		sal_out_opt size_t* stride=nullptr,
		sal_out_opt size_t* bufferId=nullptr,
		sal_out_opt size_t* offset=nullptr,
		sal_out_opt Semantic* semantic=nullptr
	);

	sal_maybenull const char* getAttributePointer(
		int attributeId,
		sal_out_opt size_t* count=nullptr,
		sal_out_opt size_t* stride=nullptr,
		sal_out_opt size_t* bufferId=nullptr,
		sal_out_opt size_t* offset=nullptr,
		sal_out_opt Semantic* semantic=nullptr
	) const;

	/*!	Returns the required attribute as a StrideArray with the correct stride.
		Returns an ArrayWrpper of null if error occurred.
	 */
	template<typename T> StrideArray<T> getAttributeAs(int attributeId)
	{
		size_t count, stride;
		Semantic semantic;
		char* p = getAttributePointer(attributeId, &count, &stride, nullptr, nullptr, &semantic);
		if(p && sizeof(T) == semantic.elementCount * semantic.elementSize)
			return StrideArray<T>(p, count, stride);
		return StrideArray<T>(nullptr, 0, 0);
	}

	template<typename T> const StrideArray<T> getAttributeAs(int attributeId) const
	{
		return const_cast<MeshBuilder*>(this)->getAttributeAs(attributeId);
	}

	sal_maybenull char* getBufferPointer(
		size_t bufferIdx,
		sal_out_opt size_t* elementSize=nullptr,
		sal_out_opt size_t* sizeInByte=nullptr
	);

	sal_maybenull const char* getBufferPointer(
		size_t bufferIdx,
		sal_out_opt size_t* elementSize=nullptr,
		sal_out_opt size_t* sizeInByte=nullptr
	) const;

protected:
	class Impl;
	Impl& mImpl;

#ifndef NDEBUG
	mutable bool mIsCreatedOnStack;
	MCD_RENDER_API friend void intrusivePtrAddRef(MeshBuilder* p);
	MCD_RENDER_API friend void intrusivePtrRelease(MeshBuilder* p);
#endif
};	// MeshBuilder

typedef IntrusivePtr<MeshBuilder> MeshBuilderPtr;

/*!	Extension of MeshBuilder, which support immediate mode mesh construction.
 */
class MCD_RENDER_API MeshBuilderIM : public MeshBuilder
{
public:
	explicit MeshBuilderIM(bool isCreatedOnStack=true);

	sal_override ~MeshBuilderIM();

	/*!	Pre-allocating vertex and index buffer for faster insertion.
		\note This function better be invoked after all vertex declarations are done.
	 */
	sal_checkreturn bool reserveBuffers(uint16_t vertexCount, size_t indexCount);

	/*!	Assign a vertex attribute to the current state.
		\return false if attributeId is invalid.
		\note Beware that you are passing the correct data type to the void* pointer.
	 */
	sal_checkreturn bool vertexAttribute(int attributeId, const void* data);

	/*!	Adds a new vertex using current vertex attributes (position, normal etc...).
		\return The vertex index, uint16_t(-1) if there is error.
	 */
	uint16_t addVertex();

	/*!	Adds a new triangle using the supplied indexes.
		\return False if any of the index is out of range.
		\note No more declareAttribute() can be made after this function is invoked, unless clear() is used.
	 */
	sal_checkreturn bool addTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3);

	/*!	Adds a new quad using the supplied indexes.
		\return False if any of the index is out of range.
		\note Two triangles are generated internally.
		\note No more declareAttribute() can be made after this function is invoked, unless clear() is used.
	 */
	sal_checkreturn bool addQuad(uint16_t idx1, uint16_t idx2, uint16_t idx3, uint16_t idx4);

protected:
	class Impl2;
	Impl2& mImpl2;
};	// MeshBuilderIM

typedef IntrusivePtr<MeshBuilderIM> MeshBuilderIMPtr;

class Mesh;

//!	Commit the data in the MeshBuilder to a Mesh.
MCD_RENDER_API sal_checkreturn bool commitMesh(const MeshBuilder& builder, Mesh& mesh, MeshBuilder::StorageHint storageHint);

}	// namespace MCD

#endif	// __MCD_RENDER_MESHBUILDER__
