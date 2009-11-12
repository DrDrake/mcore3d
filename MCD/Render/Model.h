#ifndef __MCD_RENDER_MODEL__
#define __MCD_RENDER_MODEL__

#include "Effect.h"
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
	/*!A simple structure for sotring mesh and material as a pair.
		To keep it simple, copying this struct is not implemented.
	 */
	struct MCD_RENDER_API MeshAndMaterial : public LinkListBase::Node<MeshAndMaterial>, MCD::Noncopyable
	{
		MeshAndMaterial();
		~MeshAndMaterial();

		MeshPtr mesh;
		EffectPtr effect;
	};	// MeshAndMaterial

	typedef LinkList<MeshAndMaterial> MeshList;
	MeshList mMeshes;
};	// Model

typedef IntrusivePtr<Model> ModelPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MODEL__
