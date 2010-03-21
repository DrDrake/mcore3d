#ifndef __MCD_RENDER_MODEL__
#define __MCD_RENDER_MODEL__

#include "ShareLib.h"
#include "Renderable.h"
#include "../Core/System/LinkList.h"
#include "../Core/System/Resource.h"

namespace MCD {

typedef IntrusivePtr<class Mesh> MeshPtr;
typedef IntrusivePtr<class MeshBuilder> MeshBuilderPtr;
typedef IntrusivePtr<class Effect> EffectPtr;

/*!	Basically it's a list of Mesh - Material pairs.
	It make shared ownership on Mesh and exclusive ownership on Materail.
 */
class MCD_RENDER_API Model : public Resource, public IRenderable
{
public:
	explicit Model(const Path& fileId);

// Operations
	sal_override void draw();

// Attributes
	/*!	A simple structure for storing mesh, mesh builder and material as a tuple.
		To keep it simple, copying this struct is not implemented.
	 */
	struct MCD_RENDER_API MeshAndMaterial : public LinkListBase::Node<MeshAndMaterial>, MCD::Noncopyable
	{
		MeshAndMaterial();
		~MeshAndMaterial();

		MeshPtr mesh;
		EffectPtr effect;
		std::string name;	// TODO: To be remove.

		/*! Pointer to the MeshBuilder of this mesh, it is primaryly used for reading the mesh data
			without downloading them from the GPU.
			Please notice that this pointer may be nullptr.
		 */
		MeshBuilderPtr meshBuilder;
	};	// MeshAndMaterial

	typedef LinkList<MeshAndMaterial> MeshList;
	MeshList mMeshes;

protected:
	sal_override ~Model();
};	// Model

typedef IntrusivePtr<Model> ModelPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_MODEL__
