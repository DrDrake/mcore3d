#ifndef __MCD_RENDER_INSTANCEDMESH__
#define __MCD_RENDER_INSTANCEDMESH__

#include "ShareLib.h"
#include "../Core/System/Atomic.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/IntrusivePtr.h"

namespace MCD {

template<typename T> class Mat44;
typedef Mat44<float> Mat44f;

typedef IntrusivePtr<class Effect> EffectPtr;
typedef IntrusivePtr<class Mesh> MeshPtr;

// TODO: This class is subject to heavy revise.
/*!	The Instanced Mesh composes of 2 classes:
	InstancedMesh and InstancedMeshComponent
	InstancedMeshComponent registers per-instance information to InstancedMesh
	InstancedMesh is responsible for uploading per-instance information to GPU, and issue the draw call
 */
class MCD_RENDER_API InstancedMesh : public IntrusiveSharedObject<AtomicInteger>, Noncopyable
{
public:
	InstancedMesh(const MeshPtr& mesh, const EffectPtr& effect);

	~InstancedMesh();

	void update(const Mat44f& viewMat);

	// This function accept world transformation only, but this limitation is temporary..
	// It will be extended to accept more infomation(e.g. hw skinning info)
	// But before that, research on size of bindable uniforms and vertex texture should be done first
	void registerPerInstanceInfo(const Mat44f& info);

	MeshPtr mesh;
	EffectPtr effect;

private:
	class Impl;
	Impl* mImpl;
};	// InstancedMesh

typedef IntrusivePtr<InstancedMesh> InstancedMeshPtr;

}	// namespace MCD

#endif	// __MCD_RENDER_INSTANCEDMESH__
