#ifndef __MCD_RENDER_MESH__
#define __MCD_RENDER_MESH__

#include "ShareLib.h"
#include "../Core/System/Array.h"
#include "../Core/System/Resource.h"
#include "../Core/System/SharedPtr.h"

namespace MCD {

typedef IntrusivePtr<class Mesh> MeshPtr;

/*!	Represent an indexed triangle mesh.
	\note Since we use uint16_t to store index data, therefore a single Mesh instance cannot
		stores more than 65536 vertices.

	\sa http://www.opengl.org/wiki/GlVertexAttribPointer
		http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribPointer.xml
 */
class MCD_RENDER_API Mesh : public Resource
{
public:
	explicit Mesh(const Path& fileId = L"");

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
		int dataType;			//!< GL_INT, GL_FLOAT etc...
		uint16_t elementSize;	//!< Size in byte of the attribute, ie: Vec3f -> sizeof(float)
		uint8_t elementCount;	//!< Number of components, ie: Vec3f -> 3
		uint8_t bufferIndex;	//!< Index to the buffer object array: \em handles
		uint16_t byteOffset;	//!< Byte offset of this attribute in a single vertex
		uint16_t stride;		//!< Byte offset to the next element in the buffer
		const char* semantic;	//!< Shader uniform name
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

	//!	Get the size in byte of a particular buffer.
	size_t bufferSize(size_t bufferIndex) const;

	typedef Array<Attribute, cMaxAttributeCount> Attributes;
	Attributes attributes;
	size_t attributeCount;	//!< Number of attributes in attributes

	size_t vertexCount;

	size_t indexCount;

	/*!	A short-cut for mapping commonly used semantic to the \em attributes array.
		A value of -1 means no such attribute.
	 */
	int8_t indexAttrIdx, positionAttrIdx, normalAttrIdx;
	int8_t uv0AttrIdx, uv1AttrIdx, uv2AttrIdx;

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

	sal_notnull MeshPtr clone(sal_in_z const wchar_t* name, StorageHint hint);

	//!	Return -1 if the semnatic cannot be found.
	int finidAttributeBySemantic(const char* semantic) const;

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
		MCD_ASSUME(attributeIdx < attributeCount);
		const Attribute& a = attributes[attributeIdx];
		MCD_ASSERT(a.elementSize * a.elementCount == sizeof(T));
		size_t count = (attributeIdx == size_t(indexAttrIdx)) ? indexCount : vertexCount;
		return StrideArray<T>(reinterpret_cast<T*>(static_cast<char*>(mapBuffer(a.bufferIndex, mapped, mapOptions)) + a.byteOffset), count, a.stride);
	}

	void unmapBuffers(MappedBuffers& mapped) const;

protected:
	sal_override ~Mesh();
};	// Mesh

class MeshBuilder;

//!	Commit the data in a MeshBuilder to the Mesh.
MCD_RENDER_API sal_checkreturn bool commitMesh(const MeshBuilder& builder, Mesh& mesh, Mesh::StorageHint storageHint);

}	// namespace MCD

#endif	// __MCD_RENDER_MESH__
