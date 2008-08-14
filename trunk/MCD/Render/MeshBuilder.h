#ifndef __MCD_RENDER_MESHBUILDER__
#define __MCD_RENDER_MESHBUILDER__

#include "Mesh.h"

namespace MCD {

template<typename T> class Vec2;
typedef Vec2<float> Vec2f;

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

/*!	Fill data to Mesh class.
	The mesh builder have a similar concept with Opengl display list, where user supply
	the vertex position, normal, texture coordinate etc as the current state of the builder.
	Once the state is setup correctly, vertices and index data can be added and they are
	stored inside an internal buffer. With the vertex and index data buffer, you can commit
	those data to the mesh you want.

	\sa http://miloyip.seezone.net/?p=34#extended

	\code
	MeshBuilder builder;
	builder.begin();
	builder.end();
	\endcode
 */
class MCD_RENDER_API MeshBuilder :  Noncopyable
{
	class BufferImpl;

public:
	MeshBuilder();
	sal_override ~MeshBuilder();

	//!	Enable specific format(s) to be build.
	void enable(uint format);

	//!	Clear the mesh builder to it's emtpy state.
	void clear();

	//! Get the format(s) that are currently enabled.
	uint format() const {
		return mFormat;
	}

	/*!	Resurve vertex buffer space for faster vertex insertion.
		\note
			Setting up the format and texture coordinate size before this function
			for maximum performace.
	 */
	void reserveVertex(size_t count);

	//!	Resurve index buffer space for faster triangle insertion.
	void reserveTriangle(size_t count);

	//!
	void position(const Vec3f& vertex);

//	void color(const Color3& color);
//	void color(const Color4& color);

	//!
	void normal(const Vec3f& normal);

	//! Set the current texture unit.
	void textureUnit(Mesh::DataType textureUnit);

	/*!	Set the number of components for the current texture unit.
		\param size Number of components (can be 2 or 3) for the speific texture unit.
		\note
			Assertion failure for re-setting the size with the same textureUnit.
			You should call clear() before attempt to change the size.
	 */
	void textureCoordSize(size_t size);

	//!
	void textureCoord(const Vec2f& coord);

	//!
	void textureCoord(const Vec3f& coord);

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
			the implementation that you have time-sensitive data such as animated geom-
			etry that will be used once and then replaced. It is crucial that the data be
			placed somewhere quick to update, even at the expense of faster rendering.
		 */
		Stream	= 0x88E0,
	};

	/*!
		\note Should \em NOT executed in between begin() and end().
	 */
	void commit(Mesh& mesh, StorageHint storageHint);

	/*!	Acquire the data pointer from the internal buffer.
		\param count Returns the number of elements.
		\note
			Once you acquired the pointer, you need to release it using
			releaseBufferPointer() before adding another vertex or triangle.
		\sa releaseBufferPointer
	 */
	sal_checkreturn void* acquireBufferPointer(Mesh::DataType dataType, sal_out_opt size_t* count=nullptr);

	/*!	Release the pointer acquired by acquireBufferPointer().
		\noet Do nothing if no corresponding pointer is acquired.
		\sa acquireBufferPointer
	 */
	void releaseBufferPointer(void* ptr);

protected:
	uint mFormat;		//!< The same meaning as Mesh::mFormat
	BufferImpl& mBuffer;
};	// Mesh

}	// namespace MCD

#endif	// __MCD_RENDER_MESHBUILDER__
