#ifndef __MCD_RENDER_MODEL__
#define __MCD_RENDER_MODEL__

#include "Material.h"
#include "Renderable.h"
#include "../Core/System/LinkList.h"
#include "../Core/System/Resource.h"

namespace MCD {

typedef IntrusivePtr<class Mesh> MeshPtr;

/*!	Basically it's a list of Mesh - Material pairs.
	It make shared ownership on Mesh and exclusive ownership on Materail.
 */
class MCD_RENDER_API Model : public Resource, public IRenderable
{
public:
	explicit Model(const Path& fileId) : Resource(fileId) {}

	sal_override void draw();

protected:
	sal_override ~Model();

public:
	struct MeshAndMaterial : public LinkListBase::Node<MeshAndMaterial>
	{
		MeshAndMaterial();
		~MeshAndMaterial();

		MeshPtr mesh;
		Material* material;
	};	// MeshAndMaterial

	typedef LinkList<MeshAndMaterial> MeshList;
	MeshList mMeshes;
};	// Model

typedef IntrusivePtr<Model> ModelPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MODEL__
