#ifndef __MCD_RENDER_MODEL__
#define __MCD_RENDER_MODEL__

#include "Material.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/Resource.h"
#include <list>

namespace MCD {

class Mesh;
typedef IntrusivePtr<Mesh> MeshPtr;
class Texture;

class MCD_RENDER_API Model : public Resource, private Noncopyable
{
public:
	explicit Model(const Path& fileId) : Resource(fileId) {}

	void draw();

public:
	struct MeshAndMaterial
	{
		MeshPtr mesh;
		Material material;
	};	// MeshAndMaterial

	typedef std::list<MeshAndMaterial> MeshList;
	MeshList mMeshes;
};	// Model

typedef IntrusivePtr<Model> ModelPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MODEL__
