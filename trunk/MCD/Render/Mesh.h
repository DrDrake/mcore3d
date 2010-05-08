#ifndef __MCD_RENDER_MESH__
#define __MCD_RENDER_MESH__

#include "VertexFormat.h"
#include "../Core/System/Array.h"
#include "../Core/System/Resource.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

class MeshBuilder;
typedef IntrusivePtr<class Mesh> MeshPtr;

// TODO: Refactor it by referencing the design of SPODMesh in the PowerVR SDK, and see if it fits DirectX too.
/*!	Represent an indexed triangle mesh.
	\note Since we use uint16_t to store index data, therefore a single Mesh instance cannot
		stores more than 65536 vertices.

	\sa http://www.opengl.org/wiki/GlVertexAttribPointer
		http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribPointer.xml
 */
class MCD_RENDER_API Mesh : public Resource
{
public:
	explicit Mesh(const Path& fileId = "");

	//!	Enum values to describe what option is needed in map()
	enum MapOption
	{
		Read	= 1 << 0,
		Write	= 1 << 1,
		Discard	= 1 << 2	//!< The written data is used by the GPU only once.
	};	// MapOption

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

// Attributes
	//! Declare what a single vertex attribute contains.
	struct Attribute
	{
		VertexFormat format;
		uint8_t bufferIndex;	//!< Index to the buffer object array: \em handles
		uint16_t byteOffset;	//!< Byte offset of this attribute in a single vertex
		uint16_t stride;		//!< Byte offset to the next element in the buffer
	};	// Attribute

	//! Maximum number of separated buffer object.
	static const size_t cMaxBufferCount = 8;

	//! Maximum number of attributes, should be >= cMaxBufferCount.
	static const size_t cMaxAttributeCount = 8;

	typedef SharedPtr<uint> HandlePtr;
	typedef Array<HandlePtr, cMaxBufferCount> Handles;

	/*!	Handles to opengl buffer objects.
		The first one should be index buffer.
		\note
			The handles can be shared because SharedPtr is used.
		\note
			The array element should never be null, while it's pointee value
			may have a zero value to indicate empty handle.
	 */
	Handles handles;
	size_t bufferCount;		//!< Number of elements in handles

	//!	Get the size in byte of a particular buffer, it calculates base on an attribute's stride.
	size_t bufferSize(size_t bufferIndex) const;

	typedef Array<Attribute, cMaxAttributeCount> Attributes;
	Attributes attributes;
	size_t attributeCount;	//!< Number of attributes in attributes

	size_t vertexCount;

	size_t indexCount;

	static const int8_t cIndexAttrIdx = 0;
	static const int8_t cPositionAttrIdx = 1;

// Operations
	//! Render the mesh with all associated attributes.
	void draw();

	/*!	Render the mesh without any attributes other then position.
		Only the vertex and index buffer is used, this function is usefull for example
		depth pre-pass or render for picking.
	 */
	void drawFaceOnly();

	//!	Clear all buffers and reseting the Mesh into it's initial state.
	void clear();

	sal_notnull MeshPtr clone(sal_in_z const char* name, StorageHint hint);

	//!	Return -1 if the semnatic cannot be found.
	int findAttributeBySemantic(const StringHash& semantic) const;

	//!	An object for remembering which buffer is already mapped using mapBuffer().
	struct MappedBuffers : public Array<void*, cMaxBufferCount>
	{
		MappedBuffers() { assign(nullptr); }
	};	// MappedBuffers

	/*! Map the corresponding buffer object of a specific attribute.
		\sa mapAttribute()
		\return The pointer to the data for read / write; nullptr if any errors occur.
	 */
	sal_maybenull void* mapBuffer(size_t bufferIdx, MappedBuffers& mapped, MapOption mapOptions=Read);

	sal_maybenull const void* mapBuffer(size_t bufferIdx, MappedBuffers& mapped) const;

	/*!	Map a specific attribute in the buffer.
		Usage:
		\code
		Mesh::MappedBuffers mapped;
		StrideArray<Vec3f> vertex = mesh.mapAttribute<Vec3f>(mesh->positionAttrIdx, mapped);
		StrideArray<uint16_t> index = mesh.mapAttribute<uint16_t>(mesh->indexAttrIdx, mapped);
		// Use the vertex and index array ...
		mesh.unmapBuffers(mapped);
		\endcode
	 */
	template<typename T> StrideArray<T> mapAttribute(size_t attributeIdx, MappedBuffers& mapped, MapOption mapOptions=Read)
	{
		const Attribute& a = attributes[attributeIdx];
		MCD_ASSERT(a.format.sizeInByte() == sizeof(T));
		size_t count = (attributeIdx == cIndexAttrIdx) ? indexCount : vertexCount;
		return StrideArray<T>(reinterpret_cast<T*>(static_cast<char*>(mapBuffer(a.bufferIndex, mapped, mapOptions)) + a.byteOffset), count, a.stride);
	}

	// TODO: Introducing StrideArray2D
	template<typename T> StrideArray<T> mapAttributeUnsafe(size_t attributeIdx, MappedBuffers& mapped, MapOption mapOptions=Read)
	{
		const Attribute& a = attributes[attributeIdx];
		MCD_ASSERT(a.format.sizeInByte() >= sizeof(T));
		size_t count = (attributeIdx == cIndexAttrIdx) ? indexCount : vertexCount;
		return StrideArray<T>(reinterpret_cast<T*>(static_cast<char*>(mapBuffer(a.bufferIndex, mapped, mapOptions)) + a.byteOffset), count, a.stride);
	}

	void unmapBuffers(MappedBuffers& mapped) const;

	/*! Create a Mesh from existing data buffer(s), the Mesh's
		attributes, attributeCount, bufferCount, vertexCount, indexCount
		should be already initialized correctly prior calling this method.
		\param data Array of pointers to the buffer data.
		\note Make sure the array size of \em data match \em bufferCount.
	 */
	sal_checkreturn bool create(const void* const* data, StorageHint storageHint);

	//! Create a Mesh from an initialized MeshBuilder object
	sal_checkreturn bool create(const MeshBuilder& builder, StorageHint storageHint);

protected:
	sal_override ~Mesh();

	class Impl;
	sal_maybenull Impl* mImpl;	//! Optional book keeping object for specific API needs.
};	// Mesh

}	// namespace MCD

#endif	// __MCD_RENDER_MESH__
