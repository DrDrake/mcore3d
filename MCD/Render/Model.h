#ifndef __MCD_RENDER_MODEL__
#define __MCD_RENDER_MODEL__

#include "Material.h"
#include "Mesh.h"
#include "Renderable.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/Resource.h"
#include "../Core/System/LinkList.h"
#include <list>

namespace MCD {

typedef IntrusivePtr<class Mesh> MeshPtr;
class Texture;

class MCD_RENDER_API Model : public Resource, public IRenderable, private Noncopyable
{
public:
	explicit Model(const Path& fileId) : Resource(fileId) {}

	sal_override void draw();

public:
    struct MeshAndMaterial : public LinkListBase::Node<MeshAndMaterial>
	{
		MeshPtr mesh;
        IMaterial* material;

		~MeshAndMaterial()
		{
			delete material;
		}
	};	// MeshAndMaterial

    typedef LinkList<MeshAndMaterial> MeshList;
	MeshList mMeshes;
};	// Model

typedef IntrusivePtr<Model> ModelPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MODEL__
