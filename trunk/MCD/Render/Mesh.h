#ifndef __MCD_RENDER_MESH__
#define __MCD_RENDER_MESH__

#include "ShareLib.h"
#include "../Core/System/Array.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/Resource.h"
#include <vector>

namespace MCD {

/*!
	\note Since we use uint16_t to store index data, therefore a single Mesh instance cannot
		stores more than 65536 vertice.
 */
class MCD_RENDER_API Mesh : public Resource, Noncopyable
{
public:
	//!	Enum values to describe what the mesh content represent.
	enum DataType
	{
		TextureCoord	= (1 << 3) - 1,	//!< Texture corrdinate
		TextureCoord0	= 1,
		TextureCoord1	= 2,
		TextureCoord2	= 3,
		TextureCoord3	= 4,
		TextureCoord4	= 5,
		TextureCoord5	= 6,
		Position		= 1 << 3,		//!< Vertex position	(3 floats)
		Index			= 1 << 4,		//!< Index				(1 uint16_t)
		Color			= 1 << 5,		//!< Color				(3 uint8_t)
		Normal			= 1 << 6,		//!< Normal				(3 floats)

		MaxTypeEntry	= Normal << 1	//!< For debugging assertion check only
	};

	//! Maximum number of texture coordinate per vertex.
	static const size_t cMaxTextureCoordCount = 6;

	explicit Mesh(const Path& fileId);

	//! Test against the DataType enum using logical &
	uint format() const {
		return mFormat;
	}

	size_t vertexCount() const {
		return mVertexCount;
	}

	size_t indexCount() const {
		return mIndexCount;
	}

	/*!	Get the buffer object handle for the corresponding data type.
		Returns 0 if the data type is not found in this mesh.
	 */
	uint handle(DataType dataType) const;

	//! Get the component count for the corresponding data type.
	uint8_t componentCount(DataType dataType) const;

	//! Bind the corresponding buffer object of a specific data type.
	void bind(DataType dataType);

	void unbind();

	void draw();

	/*!	User can define this class in their scope in order to modify the data member in Texture.
		Various texture loaders use this technique to modify the texture.
	 */
	template<class T> class PrivateAccessor;

protected:
	sal_override ~Mesh();

	/*!	Get a pointer to the handle with the supplied data type.
		Use this function to modify the handles.
		Returns null if the data type is not found in this mesh.
	 */
	uint* getHandlePtr(DataType dataType);

	/*!	Get a pointer to the component with the supplied data type.
		Use this function to modify the component count for the various texture coordinates.
		Returns null if the data type is not found in this mesh or it's not related to texture coordinate.
	 */
	uint8_t* getComponentCountPtr(DataType dataType);

protected:
	/*!	Handles to opengl buffer objects.
		[0] for position
		[1] for index
		[2] for color
		[3] for normal
		[4-10] for texture coordinate[0-7]
	 */
	Array<uint,4 + cMaxTextureCoordCount> mHandles;

	//!	Specifies the number of component for a particular DataType
	Array<uint8_t,4 + cMaxTextureCoordCount> mComponentCount;

	//! Union (using logical &) of different DataType
	uint mFormat;

	size_t mVertexCount;

	size_t mIndexCount;
};	// Mesh

typedef IntrusivePtr<Mesh> MeshPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MESH__
