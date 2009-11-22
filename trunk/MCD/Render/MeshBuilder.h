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
	MeshBuilder2 builder;
	int posId = builder.declareAttribute(map.position(), 1);
	int normalId = builder.declareAttribute(map.normal(), 1);
	int uvId = builder.declareAttribute(map.uv(0, 2), 1);

	int blendWeightId = builder.declareAttribute(map.blendWeight(), 2);
	int blendIndexId = builder.declareAttribute(map.blendIndex(), 2);

	// Make 1000 of those declared vertex, and 2000 index.
	if(!resizeBuffers(1000, 2000)) return false;

	// Acquire the buffer pointer and fill up the data yourself.
	ArrayWrapper<uint16_t> posArray = builder.getAttributeAs<uint16_t>(0);
	ArrayWrapper<Vec3f> posArray = builder.getAttributeAs<Vec3f>(posId);
	ArrayWrapper<Vec3f> normalArray = builder.getAttributeAs<Vec3f>(normalId);
	// ...
	\endcode
 */
class MCD_RENDER_API MeshBuilder2 : public IntrusiveSharedObject<AtomicInteger>, private Noncopyable
{
public:
	struct Semantic
	{
		const char* name;		//! Unique string that identify the semantic
		size_t elementSize;		//! Size in byte of individual element in this semanitc, eg. Vec3f -> sizeof(float)
		size_t elementCount;	//! Number of element in this semanitc, eg. Vec3f -> 3
		size_t channelIndex;	//! The channel index for color/texture coordinate
	};	// Semantic

	explicit MeshBuilder2(bool isCreatedOnStack=true);

	sal_override ~MeshBuilder2();

// Operations
	/*!	Call this function to declare the vertex attributes you want.
		\param semantic
			The semantic for this attribute. You can get some preset semantics using SemanticMap::getSingleton().
		\param bufferId
			Feed this param with different value if you want to group different attributes into set of buffers.
			The value zero is reserved for index buffer, otherwise you can supply successive values 1,2,3,etc...
		\return The attribute ID for future reference, -1 if any error occured.

		\note You can invoke declareAttribute() even when there are datas already in the buffers.
	 */
	int declareAttribute(const Semantic& semantic, size_t bufferId=1);

	/*!	Resize the buffers.
		\note Remember only 65536 vertex is supported.
	 */
	sal_checkreturn bool resizeBuffers(uint16_t vertexCount, size_t indexCount);

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
		sal_out_opt Semantic* semantic=nullptr
	);

	sal_maybenull const char* getAttributePointer(
		int attributeId,
		sal_out_opt size_t* count=nullptr,
		sal_out_opt size_t* stride=nullptr,
		sal_out_opt Semantic* semantic=nullptr
	) const;

	/*!	Returns the required attribute as a ArrayWrapper with the correct stride.
		Returns an ArrayWrpper of null if error occurred.
	 */
	template<typename T> ArrayWrapper<T> getAttributeAs(int attributeId)
	{
		size_t count, stride;
		Semantic semantic;
		char* p = getAttributePointer(attributeId, &count, &stride, &semantic);
		if(sizeof(T) == semantic.elementCount * semantic.elementSize)
			return ArrayWrapper<T>(p, count, stride);
		MCD_ASSERT(false);
		return ArrayWrapper<T>(nullptr, 0, 0);
	}

	template<typename T> const ArrayWrapper<T> getAttributeAs(int attributeId) const
	{
		return const_cast<MeshBuilder2*>(this)->getAttributeAs(attributeId);
	}

	sal_maybenull char* getBufferPointer(
		size_t bufferIdx,
		sal_out_opt size_t* sizeInByte=nullptr
	);

	sal_maybenull const char* getBufferPointer(
		size_t bufferIdx,
		sal_out_opt size_t* sizeInByte=nullptr
	) const;

protected:
	class Impl;
	Impl& mImpl;

#ifndef NDEBUG
	mutable bool mIsCreatedOnStack;
	MCD_RENDER_API friend void intrusivePtrAddRef(MeshBuilder2* p);
	MCD_RENDER_API friend void intrusivePtrRelease(MeshBuilder2* p);
#endif
};	// MeshBuilder2

typedef IntrusivePtr<MeshBuilder2> MeshBuilder2Ptr;

/*!	Extension of MeshBuilder2, which support immediate mode mesh construction.
 */
class MCD_RENDER_API MeshBuilderIM : public MeshBuilder2
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
struct ColorRGB8;

template<typename T> class Vec2;
typedef Vec2<float> Vec2f;

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

template<typename T> class Vec4;
typedef Vec4<float> Vec4f;

/*!	Fill data to Mesh class.
	The mesh builder have a similar concept with Opengl display list, where user supply
	the vertex position, color, normal, texture coordinate etc as the current state of the builder.
	Once the state is setup correctly, vertices and index data can be added and they are
	stored inside an internal buffer. With the vertex and index data buffer, you can commit
	those data as many meshes as you want.

	\sa http://miloyip.seezone.net/?p=34#extended

	Example that demonstrate how to construct a single triangle with color, vertex normal
	and 2D texture coordinate information.
	\code
	MeshBuilder builder;
	builder.enable(Mesh::Position | Mesh::Color | Mesh::Normal | Mesh::TextureCoord0 | Mesh::Index);
	builder.textureUnit(Mesh::TextureCoord0);	// Enable texture unit 0
	builder.textureCoordSize(2);	// We use 2D texture

	// A demonstration on how to reserve vertex and polygon for better performance
	builder.reserveVertex(3);
	builder.reserveTriangle(1);

	// All 3 vertices have the same normal [0,0,1]
	builder.normal(Vec3f(0, 0, 1));

	// First vertex
	builder.position(Vec3f(-1, 1, 1));
	builder.color(ColorRGB8::red);
	builder.textureCoord(Vec2f(0, 1));
	uint16_t idx1 = builder.addVertex();

	// Second vertex
	builder.position(Vec3f(-1, -1, 1));
	builder.color(ColorRGB8::green);
	builder.textureCoord(Vec2f(0, 0));
	uint16_t idx2 = builder.addVertex();

	// Third vertex
	builder.position(Vec3f(1, -1, 1));
	builder.color(ColorRGB8::blue);
	builder.textureCoord(Vec2f(1, 0));
	uint16_t idx3 = builder.addVertex();

	// Create a triangle using the 3 vertices
	builder.addTriangle(idx1, idx2, idx3);

	// Commit the data to a mesh
	builder.commit(mesh, MeshBuilder::Static);
	\endcode
 */
class MCD_RENDER_API MeshBuilder : public IntrusiveSharedObject<AtomicInteger>, private Noncopyable
{
	class BufferImpl;

public:
	/*!	Since MeshBuilder can be shared using IntrusivePtr, we should do some error
		checking to see if the MeshBuilder is intesionally shared or not. So if you
		assign a MeshBuilder created on stack to a MeshBuilderPtr, assertion will
		occur. Vise versa, if MeshBuilder is not created on stack, it should be deteled
		though MeshBuilderPtr.
	 */
	explicit MeshBuilder(bool isCreatedOnStack=true);

	~MeshBuilder();

	//!	Enable specific format(s) to be build.
	void enable(uint format);

	//!	Clear the mesh builder to it's empty state.
	void clear();

	//! Get the format(s) that are currently enabled.
	uint format() const {
		return mFormat;
	}

	/*!	Reserve vertex buffer space for faster vertex insertion.
		\note
			Setting up the format and texture coordinate size before this function
			for maximum performance.
	 */
	void reserveVertex(size_t count);

	//!	Reserve index buffer space for faster triangle insertion.
	void reserveTriangle(size_t count);

	//!	Set the current position state.
	void position(const Vec3f& vertex);

	/*!	Set the current color state.
		\note For vertex color, a 24 bit color should be sufficient.
	 */
	void color(const ColorRGB8& color);

	//!	Set the current normal state.
	void normal(const Vec3f& normal);

	//! Set the current texture unit. The support data types are defined in Mesh::DataType.
	void textureUnit(int textureUnit);

	/*!	Set the number of components for the current texture unit.
		\param size Number of components (can be 2 or 3) for the specific texture unit.
		\sa textureCoord()
		\note
			Assertion failure for re-setting the size with the same textureUnit.
			You should call clear() before attempt to change the size.
	 */
	void textureCoordSize(size_t size);

	/*!	Set the current texture coordinate state of the current texture unit.
		\note Must have textureCoordSize(2) invoked prior to this function call.
		\sa textureCoordSize(), textureUnit()
	 */
	void textureCoord(const Vec2f& coord);

	/*!	Set the current texture coordinate state of the current texture unit.
		\note Must have textureCoordSize(3) invoked prior to this function call.
		\sa textureCoordSize(), textureUnit()
	 */
	void textureCoord(const Vec3f& coord);

	/*!	Set the current texture coordinate state of the current texture unit.
		\note Must have textureCoordSize(4) invoked prior to this function call.
		\sa textureCoordSize(), textureUnit()
	 */
	void textureCoord(const Vec4f& coord);

	/*!	Adds a new vertex using current vertex attributes (position, normal etc...).
		\return The vertex index.
	 */
	uint16_t addVertex();

	/*!	Adds a new triangle using the supplied indexes.
		\return False if any of the index is out of range.
	 */
	bool addTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3);

	/*!	Adds a new quad using the supplied indexes.
		\note Two triangles are generated internally.
		\return False if any of the index is out of range.
	 */
	bool addQuad(uint16_t idx1, uint16_t idx2, uint16_t idx3, uint16_t idx4);

	enum StorageHint
	{
		/*! The data stored in the buffer object is unlikely to change and will be used
			possibly many times as a source for drawing. This hint tells the implementation
			to put the data somewhere it¡¦s quick to draw from, but probably not quick to
			update.
		 */
		Static	= 0x88E4,

		/*! The data stored in the buffer object is likely to change frequently but is likely
			to be used as a source for drawing several times in between changes. This hint
			tells the implementation to put the data somewhere it won¡¦t be too painful to
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

	//!	Commit the stored buffer in the builder into a mesh.
	void commit(Mesh& mesh, StorageHint storageHint);

	//!	Commit only specific type(s) of buffer to the mesh.
	void commit(Mesh& mesh, uint format, StorageHint storageHint);

	/*!	Acquire the data pointer from the internal buffer.
		\param dataType One of the data type which is defined in Mesh::DataType.
		\param count Returns the number of elements.
		\note
			Once you acquired the pointer, you need to release it using
			releaseBufferPointer() before adding another vertex or triangle.
		\sa releaseBufferPointer
	 */
	sal_maybenull void* acquireBufferPointer(int dataType, sal_out_opt size_t* count=nullptr);

	/*!	Release the pointer acquired by acquireBufferPointer().
		\note Do nothing if no corresponding pointer is acquired.
		\sa acquireBufferPointer
	 */
	void releaseBufferPointer(sal_maybenull const void* ptr);

protected:
	uint mFormat;		//!< The same meaning as Mesh::mFormat
	BufferImpl& mBuffer;

#ifndef NDEBUG
	mutable bool mIsCreatedOnStack;
	MCD_RENDER_API friend void intrusivePtrAddRef(MeshBuilder* p);
	MCD_RENDER_API friend void intrusivePtrRelease(MeshBuilder* p);
#endif
};	// MeshBuilder

typedef IntrusivePtr<MeshBuilder> MeshBuilderPtr;

/*!	Commit the data in the MeshBuilder2 to a Mesh.
	int attributeMap[] = { 0, Mesh::Index, 1, Mesh::Position, 2, Mesh::Normal }
	Make sure the element count of attributeMap is >= builder.attributeCount() * 2.
 */
MCD_RENDER_API void commitMesh(MeshBuilder2& builder, Mesh& mesh, const int* attributeMap, MeshBuilder::StorageHint storageHint);

}	// namespace MCD

#endif	// __MCD_RENDER_MESHBUILDER__
