#ifndef __MCD_RENDER_MESHBUILDER__
#define __MCD_RENDER_MESHBUILDER__

#include "Mesh.h"

namespace MCD {

template<typename T> class Vec3;
typedef Vec3<float> Vec3f;

/*!	To fill data to Mesh class.
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

	//! 
	void begin(uint format);

	void end();

	/*!
		\note Should \em NOT executed in between begin() and end().
	 */
	void clear();

	//!
	uint format() const {
		return mFormat;
	}

	/*!	Resurve vertex buffer space for faster vertex insertion
		\note Should only executed in between begin() and end().
	 */
	void reserveVertex(size_t count);

	/*!	Resurve index buffer space for faster triangle insertion
		\note Should only executed in between begin() and end().
	 */
	void reserveTriangle(size_t count);

	/*!
		\note Should only executed in between begin() and end().
	 */
	void position(const Vec3f& vertex);

//	void vertexColor3(const Color3& color);
//	void vertexColor4(const Color4& color);

	/*!
		\note Should only executed in between begin() and end().
	 */
	void normal(const Vec3f& normal);

//	void textureCoord(Mesh::DataType textureUnit, const Vec2f& coord);

	/*!
		\note Should only executed in between begin() and end().
	 */
	void textureCoord(Mesh::DataType textureUnit, const Vec3f& coord);

//	void textureCoord(Mesh::DataType textureUnit, const Vec4f& coord);

	/*!	Add a vertex using previous position, normal etc...
		\return The vertex index.
	 */
	uint16_t addVertex();

	/*!
		\return False if any of the index is out of range.
	 */
	bool addTriangle(uint16_t idx1, uint16_t idx2, uint16_t idx3);

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
	sal_checkreturn void* acquireBufferPointer(Mesh::DataType dataType, size_t* sal_out_opt count=nullptr);

	/*!	Release the pointer acquired by acquireBufferPointer().
		\sa acquireBufferPointer
	 */
	void releaseBufferPointer(void* ptr);

protected:
	uint mFormat;		//!< The same meaning as Mesh::mFormat
	BufferImpl& mBuffer;
};	// Mesh

}	// namespace MCD

#endif	// __MCD_RENDER_MESHBUILDER__
