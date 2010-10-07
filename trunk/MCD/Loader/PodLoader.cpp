#include "Pch.h"
#include "PodLoader.h"
//#include "../Renderer/Component/AnimationComponent.h"
//#include "../Renderer/Component/SkeletonAnimationComponent.h"
#include "../Render/Mesh.h"
//#include "../Render/MeshUtility.h"
#include "../Render/Material.h"
//#include "../Render/MeshComponent.h"
//#include "../Render/SkinMeshComponent.h"
#include "../Render/Texture.h"
#include "../Core/Entity/Entity.h"
#include "../Core/Entity/Prefab.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/PtrVector.h"
#include "../Core/System/StrUtility.h"
#include "../Core/Math/AnimationInstance.h"
#include "../Core/Math/Skeleton.h"
#include "../Core/Math/Quaternion.h"
#include "../../3Party/PowerVR/PVRTModelPOD.h"

#include "../Core/Math/Vec2.h"

#ifdef MCD_VC
#	ifdef NDEBUG
#		pragma comment(lib, "PowerVR")
#	else
#		pragma comment(lib, "PowerVRd")
#	endif
#endif	// MCD_VC

namespace MCD {

class PodLoader::Impl
{
public:
	Impl()
		: mResourceManager(nullptr), mSystemEntities(nullptr)
	{
		initSystemComponents();
	}

	~Impl()
	{
		mPod.Destroy();

		for(size_t i=0; i<mMaterials.size(); ++i)
			mMaterials[i]->destroyThis();
	}

	void initSystemComponents()
	{
		mSystemEntities = Entity::currentRoot();
		if(!mSystemEntities)
			return;

		if(ResourceManagerComponent* c = mSystemEntities->findComponentInChildrenExactType<ResourceManagerComponent>())
			mResourceManager = &c->resourceManager();

//		mAnimationUpdater = mSystemEntities->findComponentInChildrenExactType<AnimationUpdaterComponent>())

		//if(Entity* e = mSystemEntities->findEntityInDescendants("Skeleton animation updater"))
		//	mSkeletonAnimationUpdater = dynamic_cast<SkeletonAnimationUpdaterComponent*>(e->findComponent<BehaviourComponent>());
	}

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	ResourceManager* mResourceManager;
	//AnimationUpdaterComponentPtr mAnimationUpdater;
	//SkeletonAnimationUpdaterComponentPtr mSkeletonAnimationUpdater;

	EntityPtr mSystemEntities;
	Entity mRootEntity;
	CPVRTModelPOD mPod;

	//!	Store a list of Mesh that is pending to commit the buffer (which must done in main thread).
	std::vector<std::pair<MeshPtr, std::vector<void*> > > mMeshes;

	std::vector<TexturePtr> mTextures;
	std::vector<MaterialComponent*> mMaterials;
//	std::vector<std::pair<SkinMeshComponentPtr, MeshPtr> > mSkinMeshToCommit;
};	// Impl

static GpuDataFormat toCpuDataFormat(EPVRTDataType type, size_t count)
{
	if(type == EPODDataFloat && count == 1)
		return GpuDataFormat::get("floatR32");
	else if(type == EPODDataFloat && count == 2)
		return GpuDataFormat::get("floatRG32");
	else if(type == EPODDataFloat && count == 3)
		return GpuDataFormat::get("floatRGB32");
	else if(type == EPODDataFloat && count == 4)
		return GpuDataFormat::get("floatRGBA32");
	else if(type == EPODDataUnsignedShort && count == 1)
		return GpuDataFormat::get("uintR16");
	else if(type == EPODDataUnsignedByte && count == 1)
		return  GpuDataFormat::get("uintR8");
	else if(type == EPODDataUnsignedByte && count == 2)
		return  GpuDataFormat::get("uintRG8");
	else if(type == EPODDataUnsignedByte && count == 3)
		return  GpuDataFormat::get("uintRGB8");
	else if(type == EPODDataUnsignedByte && count == 4)
		return  GpuDataFormat::get("uintRGBA8");

	MCD_ASSERT(false);
	return GpuDataFormat::none();
}

static void assignAttribute(Mesh& mesh, std::vector<void*>& bufferPtrs, const SPODMesh& podMesh, Mesh::Attribute& a, const CPODData& data, const StringHash& semantic, bool isIndex=false)
{
	a.bufferIndex = uint8_t(mesh.bufferCount);
	a.format.semantic = semantic;
	a.format.gpuFormat = toCpuDataFormat(data.eType, data.n);
	a.format.channel = 0;
	a.stride = uint16_t(data.nStride);

	MCD_ASSERT(a.format.gpuFormat.isValid());

	if(isIndex) {
		a.byteOffset = 0;
		mesh.bufferCount++;
		bufferPtrs.push_back(data.pData);
	}
	else {
		a.byteOffset = podMesh.pInterleaved ? uint16_t(uintptr_t(data.pData)) : 0;

		if(!podMesh.pInterleaved) {
			mesh.bufferCount++;
			bufferPtrs.push_back(data.pData);
		}
	}

	mesh.attributeCount++;
}

static void getLocalTransform(const CPVRTModelPOD& pod, const SPODNode& node, Mat44f& ret)
{
	PVRTMat4& mat = reinterpret_cast<PVRTMat4&>(ret);
	PVRTMat4 tmp;

	pod.GetScalingMatrix(mat, node);
	pod.GetRotationMatrix(tmp, node);
	PVRTMatrixMultiply(mat, mat, tmp);
	pod.GetTranslationMatrix(tmp, node);
	PVRTMatrixMultiply(mat, mat, tmp);

	mat = mat.transpose();
}
/*
static StrideArray<Vec3f> getVertexData(const SPODMesh& mesh)
{
	StrideArray<Vec3f> ret(nullptr, mesh.nNumVertex, mesh.sVertex.nStride);
	if(mesh.pInterleaved)
		ret.data = (char*)(mesh.pInterleaved + size_t(mesh.sVertex.pData));
	else
		ret.data = (char*)mesh.sVertex.pData;
	return ret;
}

static StrideArray<uint8_t> getJointIdx(const SPODMesh& mesh)
{
	StrideArray<uint8_t> ret(nullptr, mesh.nNumVertex, mesh.sBoneIdx.nStride);
	if(mesh.pInterleaved)
		ret.data = (char*)(mesh.pInterleaved + size_t(mesh.sBoneIdx.pData));
	else
		ret.data = (char*)mesh.sBoneIdx.pData;
	return ret;
}

static StrideArray<byte_t> getJointWeight(const SPODMesh& mesh)
{
	StrideArray<byte_t> ret(nullptr, mesh.nNumVertex, mesh.sBoneWeight.nStride);
	if(mesh.pInterleaved)
		ret.data = (char*)(mesh.pInterleaved + size_t(mesh.sBoneWeight.pData));
	else
		ret.data = (char*)mesh.sBoneWeight.pData;
	return ret;
}

static bool isSkinMeshNode(const CPVRTModelPOD& pod, size_t i)
{
	if(i >= pod.nNumMesh)
		return false;

	const SPODNode& podNode = pod.pNode[i];
	const SPODMesh& podMesh = pod.pMesh[podNode.nIdx];
	return podMesh.sBoneIdx.n > 0;
}
*/
static void* insertDataToInterleavedBuffer(
	const void* originalData,
	size_t oldStride, size_t newStride, size_t vertexCount,
	size_t insertAtOffset, const void* initialContent=nullptr
)
{
	MCD_ASSUME(newStride > oldStride);
	MCD_ASSUME(insertAtOffset <= oldStride);

	byte_t* newBuf = (byte_t*)::malloc(newStride * vertexCount);

	const size_t deltaSize = newStride - oldStride;
	const byte_t* pSrc = (const byte_t*)originalData;
	byte_t* pDest = newBuf;

	for(size_t i=0; i<vertexCount; ++i, pSrc += oldStride, pDest += newStride) {
		::memcpy(pDest, pSrc, insertAtOffset);
		if(initialContent)
			::memcpy(pDest + insertAtOffset, initialContent, deltaSize);
		::memcpy(pDest + insertAtOffset + deltaSize, pSrc + insertAtOffset, oldStride - insertAtOffset);
	}

	return newBuf;
}

IResourceLoader::LoadingState PodLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	if(!is)
		return Aborted;

	EPVRTError result = mPod.ReadFromStream(*is);

	if(result != PVR_SUCCESS)
		return Aborted;

	// Load the textures ASAP
	for(size_t i=0; i<mPod.nNumTexture; ++i)
	{
		std::string texturePath(mPod.pTexture[i].pszName);

		// We assume the texture is relative to the pod file, if the texture file didn't has a root path.
		Path adjustedPath = fileId ? fileId->getBranchPath()/texturePath : texturePath;
		if(mResourceManager)
			mTextures.push_back(dynamic_cast<Texture*>(mResourceManager->load(adjustedPath).get()));
		else
			mTextures.push_back(new Texture(adjustedPath));
	}

	// Generate materials
	if(mResourceManager) for(size_t i=0; i<mPod.nNumMaterial; ++i)
	{
		const SPODMaterial& material = mPod.pMaterial[i];

		MaterialComponent* m = new MaterialComponent;

		m->opacity = material.fMatOpacity;
		m->specularExponent = material.fMatShininess * 100;
		m->diffuseColor = ColorRGBAf(reinterpret_cast<const ColorRGBf&>(*material.pfMatDiffuse), 1);
		m->specularColor = ColorRGBAf(reinterpret_cast<const ColorRGBf&>(*material.pfMatSpecular), 1);

		if(material.nIdxTexDiffuse >= 0)
			m->diffuseMap = mTextures[material.nIdxTexDiffuse];
//		if(material.nIdxTexSpecularColour >= 0)
//			m->specularMap = mTextures[material.nIdxTexSpecularColour];
//		if(material.nIdxTexEmissive >= 0) {
//			m->emissionMap = mTextures[material.nIdxTexEmissive];
//			m->emissionColor = ColorRGBAf(1, 1, 1, 1);
//		}
//		if(material.nIdxTexBump >= 0)
//			m->normalMap = mTextures[material.nIdxTexBump];

		mMaterials.push_back(m);
	}

	// Generate meshes
	for(size_t i=0; i<mPod.nNumMesh; ++i)
	{
		std::string meshName = fileId->getString() + ":mesh" + MCD::int2Str(i);
		MeshPtr mesh = new Mesh(meshName.c_str());
		const SPODMesh& podMesh = mPod.pMesh[i];
		mMeshes.push_back(std::make_pair(mesh, std::vector<void*>()));
		std::vector<void*>& bufferPtrs = mMeshes[i].second;

		if(podMesh.ePrimitiveType != ePODTriangles)	// We only support triangle
			continue;
		if(podMesh.nNumStrips > 0 || podMesh.pnStripLength)	// Triangle strip is not supported (yet?)
			continue;

		// Adjust every skinned mesh to have 4 joints per vertex, to fullfill the requirenment imposed by directx
		// other wise we need to seperate the joint index and joint weight into a seperate buffer
		if(podMesh.sBoneIdx.n && podMesh.sBoneIdx.n != 4 && podMesh.pInterleaved)
		{
			const size_t oldJointPerVertex = podMesh.sBoneIdx.n;
			static const size_t newJointPerVertex = 4;
			const size_t deltaJoinPerVertex = newJointPerVertex - oldJointPerVertex;
			const size_t oldStride = podMesh.sBoneIdx.nStride;
			const size_t newStride = oldStride + deltaJoinPerVertex * (sizeof(uint8_t) + sizeof(float));
			SPODMesh& mesh_ = const_cast<SPODMesh&>(podMesh);

			// Insert dummy joint weight data
			const float cDummyWeight[newJointPerVertex] = { 0 };
			void* newBuf = insertDataToInterleavedBuffer(
				podMesh.pInterleaved,
				podMesh.sBoneWeight.nStride,
				podMesh.sBoneWeight.nStride + deltaJoinPerVertex * sizeof(float),
				podMesh.nNumVertex,
				size_t(podMesh.sBoneWeight.pData) + oldJointPerVertex * sizeof(float),	// insertAtOffset
				cDummyWeight
			);
			::free(podMesh.pInterleaved);

			if(podMesh.sBoneIdx.pData > podMesh.sBoneWeight.pData)
				mesh_.sBoneIdx.pData += deltaJoinPerVertex * sizeof(float);
			else
				mesh_.sBoneWeight.pData += deltaJoinPerVertex * sizeof(uint8_t);

			// Insert dummy joint index data
			const uint8_t cDummyIdx[newJointPerVertex] = { 0 };
			mesh_.pInterleaved = (unsigned char*)insertDataToInterleavedBuffer(
				newBuf,
				podMesh.sBoneIdx.nStride + deltaJoinPerVertex * sizeof(float),
				newStride,
				podMesh.nNumVertex,
				size_t(podMesh.sBoneIdx.pData) + oldJointPerVertex * sizeof(uint8_t),	// insertAtOffset
				cDummyIdx
			);
			::free(newBuf);

			if(podMesh.sVertex.n) mesh_.sVertex.nStride = newStride;
			if(podMesh.sNormals.n) mesh_.sNormals.nStride = newStride;
			for(size_t j=0; j<podMesh.nNumUVW; ++j)
				mesh_.psUVW[j].nStride = newStride;

			mesh_.sBoneIdx.n = newJointPerVertex;
			mesh_.sBoneIdx.nStride = newStride;
			mesh_.sBoneWeight.n = newJointPerVertex;
			mesh_.sBoneWeight.nStride = newStride;
		}

		// Index
		if(podMesh.sFaces.n) {
			MCD_ASSERT(mesh->attributeCount == Mesh::cIndexAttrIdx);
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sFaces, "index", true);
		}
		else {
			Log::write(Log::Warn, "Empty mesh data in pod");
			continue;
		}

		// Vertex position
		if(podMesh.sVertex.n) {
			MCD_ASSERT(mesh->attributeCount == Mesh::cPositionAttrIdx);
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sVertex, "position");
		}
		else {
			Log::write(Log::Warn, "Empty mesh data in pod");
			continue;
		}

		// Vertex normal
		if(podMesh.sNormals.n) {
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sNormals, "normal");
		}

		// UV channels
		for(size_t j=0; j<podMesh.nNumUVW; ++j) {
			if(j > 9) continue;
			char buf[] = "uv0";
			buf[2] += char(j);
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.psUVW[j], StringHash(buf, sizeof(buf)));
		}

		if(podMesh.sBoneIdx.n) {
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sBoneIdx, "jointIndex");
		}

		if(podMesh.sBoneWeight.n) {
			Mesh::Attribute& a = mesh->attributes[mesh->attributeCount];
			assignAttribute(*mesh, bufferPtrs, podMesh, a, podMesh.sBoneWeight, "jointWeight");
		}

		if(podMesh.pInterleaved) {
			mesh->bufferCount++;
			bufferPtrs.push_back(podMesh.pInterleaved);
		}

		mesh->vertexCount = podMesh.nNumVertex;
		mesh->indexCount = podMesh.nNumFaces * 3;

		// NOTE: The upload of vertex data is postponed until commit() which is run in main thread.
	}

	// Create animation track
/*	AnimationTrackPtr track = new AnimationTrack("");
	track->naturalFramerate = 30;

	// Create skeleton animation
	SkeletonPtr skeleton = new Skeleton("");

	static const size_t cExtraRootNode = 1;

	// Determines the sub-track structure
	std::vector<size_t> subTrackStructure(AnimationComponent::subtrackPerEntity * cExtraRootNode, 1);

	for(size_t i=0; i<mPod.nNumNode; ++i)
	{
		// We share the same animation track if the pod file contains both node and skeleton animation,
		// as node animation needs at least 4 sub-track, there will be redudant tracks when use as skeleton.
		size_t frames[AnimationComponent::subtrackPerEntity];
		for(size_t j=0; j<AnimationComponent::subtrackPerEntity; ++j) frames[j] = 1;

		if(mPod.pNode[i].nAnimFlags & ePODHasPositionAni)
			frames[0] = mPod.nNumFrame;
		if(mPod.pNode[i].nAnimFlags & ePODHasRotationAni)
			frames[1] = mPod.nNumFrame;
		if(mPod.pNode[i].nAnimFlags & ePODHasScaleAni)
			frames[2] = mPod.nNumFrame;

		subTrackStructure.insert(subTrackStructure.end(), &frames[0], &frames[4]);
	}

	if(!subTrackStructure.empty())
	{	// Fill the animation track
		AnimationTrack::ScopedWriteLock lock(*track);
		MCD_VERIFY(track->init(StrideArray<const size_t>(&subTrackStructure[0], subTrackStructure.size())));

		// Assign the position of each frame
		for(size_t i=0; i<subTrackStructure.size(); ++i) {
			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(i);
			for(size_t j=0; j<frames.size; ++j)
				frames[j].pos = float(j);
		}

		// Assign for the extra root node
		{	AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(0);
			reinterpret_cast<Vec3f&>(frames[0]) = Vec3f(0);
			frames = track->getKeyFramesForSubtrack(1);
			reinterpret_cast<Quaternionf&>(frames[0]) = Quaternionf::cIdentity;
			frames = track->getKeyFramesForSubtrack(2);
			reinterpret_cast<Vec3f&>(frames[0]) = Vec3f(1);
			frames = track->getKeyFramesForSubtrack(3);
			reinterpret_cast<Vec4f&>(frames[0]) = Vec4f(1);
		}

		size_t j = cExtraRootNode;
		for(size_t i=0; i<mPod.nNumNode; ++i)
		{
			const SPODNode& podNode = mPod.pNode[i];
			const size_t subTrackOffset = j * AnimationComponent::subtrackPerEntity;

			// Translation
			AnimationTrack::KeyFrames frames = track->getKeyFramesForSubtrack(subTrackOffset + 0);
			for(size_t k=0; k<frames.size; ++k)
				reinterpret_cast<Vec3f&>(frames[k]) = reinterpret_cast<Vec3f&>(podNode.pfAnimPosition[k * 3]);

			// Rotation
			frames = track->getKeyFramesForSubtrack(subTrackOffset + 1);
			track->subtracks[subTrackOffset + 1].flag = AnimationTrack::Slerp;
			for(size_t k=0; k<frames.size; ++k)
				reinterpret_cast<Quaternionf&>(frames[k]) = reinterpret_cast<Quaternionf&>(podNode.pfAnimRotation[k * 4]).inverseUnit();

			// Sclae
			frames = track->getKeyFramesForSubtrack(subTrackOffset + 2);
			for(size_t k=0; k<frames.size; ++k)
				reinterpret_cast<Vec3f&>(frames[k]) = reinterpret_cast<Vec3f&>(podNode.pfAnimScale[k * 7]);	// Don't know why the stride of scale is 7

			// Color
			frames = track->getKeyFramesForSubtrack(subTrackOffset + 3);
			for(size_t k=0; k<frames.size; ++k)
				reinterpret_cast<Vec4f&>(frames[k]) = Vec4f(1);
			++j;	// Move to next node with animation
		}
	}*/

	bool hasSkeleton = false;

/*	if(!subTrackStructure.empty())
	{	// Setup the skeleton
		skeleton->init(subTrackStructure.size() / AnimationComponent::subtrackPerEntity);
		skeleton->basePose.init(subTrackStructure.size() / AnimationComponent::subtrackPerEntity);

		for(size_t i=0; i<mPod.nNumNode; ++i) {
			const SPODNode& podNode = mPod.pNode[i];
			skeleton->names[i + cExtraRootNode] = podNode.pszName;
			skeleton->parents[i + cExtraRootNode] = podNode.nIdxParent < 0 ? 0 : podNode.nIdxParent + cExtraRootNode;
		}

		for(size_t i=0; i<mPod.nNumMesh; ++i)
		{
			const SPODMesh& podMesh = mPod.pMesh[i];
			if(!podMesh.sBoneIdx.pData || !podMesh.sBoneWeight.pData)
				continue;

			std::vector<bool> boneIdxRemapAssigned(podMesh.nNumVertex, false);

			StrideArray<uint8_t> jointIdx = getJointIdx(podMesh);
			StrideArray<byte_t> jointWeight = getJointWeight(podMesh);

			const CPVRTBoneBatches& boneBatches = podMesh.sBoneBatches;
			for(int iBatch = 0; iBatch < boneBatches.nBatchCnt; ++iBatch)
			{
				std::vector<size_t> boneIdxRemap;

				// Go through the bones for the current bone batch
				for(int b=0; b<boneBatches.pnBatchBoneCnt[iBatch]; ++b) {
					// Get the Node of the bone
					int nodeIdx = boneBatches.pnBatches[iBatch * boneBatches.nBatchBoneMax + b];
					boneIdxRemap.push_back(nodeIdx + cExtraRootNode);
					hasSkeleton = true;
				}

				// Transform the original bone index to match our flattened bone patch
				size_t triBegin = podMesh.sBoneBatches.pnBatchOffset[iBatch];	// Begining triangle for this patch
				size_t triEnd = iBatch+1 < boneBatches.nBatchCnt ? boneBatches.pnBatchOffset[iBatch+1] : podMesh.nNumFaces;	// End triangle for this patch

				for(size_t t=triBegin; t<triEnd; ++t) for(size_t iv=0; iv<3; ++iv) {
					const size_t v = ((uint16_t*)(podMesh.sFaces.pData))[t*3 + iv];
					if(boneIdxRemapAssigned[v])
						continue;
					boneIdxRemapAssigned[v] = true;

					uint8_t* jtIdx = &jointIdx[v];
					float* jtWeight = reinterpret_cast<float*>(&jointWeight[v]);

					for(size_t j=0; j<podMesh.sBoneIdx.n; ++j) {
						uint8_t& val = jtIdx[j];
						const size_t newIdx = boneIdxRemap[val];

						MCD_ASSERT(newIdx < 255);
						val = uint8_t(newIdx);
					}

					// Sort the joint as decending joint weight
					for(size_t j=0; j<podMesh.sBoneIdx.n; ++j) for(size_t k=1; k<podMesh.sBoneIdx.n; ++k)
						if(jtWeight[k-1] < jtWeight[k]) {
							std::swap(jtIdx[k-1], jtIdx[k]);
							std::swap(jtWeight[k-1], jtWeight[k]);
						}
				}
			}
		}

		// Since all the get functions of the pod API are in final world transform, we
		// would like the vertex positions are also in the final world coordinate; therefore
		// we bake the mesh's transform into the vertex.
		if(hasSkeleton) for(size_t i=0; i<mPod.nNumMeshNode; ++i)
		{
			const SPODNode& podNode = mPod.pNode[i];
			const SPODMesh& podMesh = mPod.pMesh[podNode.nIdx];

			Mat44f mat;
			getLocalTransform(mPod, podNode, mat);

			StrideArray<Vec3f> vertex = getVertexData(podMesh);
			for(size_t j=0; j<vertex.size; ++j)
				mat.transformPoint(vertex[j]);
		}

		if(!hasSkeleton)
			skeleton = nullptr;
	}

	// Create AnimationComponent if necessary
	AnimationComponentPtr animationComponent;
	//if(mAnimationUpdater && mPod.nNumFrame > 0) {
	if(mPod.nNumFrame > 0) {
		EntityPtr e = new Entity("Animation controller);
		animationComponent = e->addComponent(new AnimationComponent(*mSystemEntities));
		e->asChildOf(&mRootEntity);
	}

	// Create SkeletonAnimationComponent if necessary
	SkeletonAnimationComponentPtr skeletonAnimationComponent;
	//if(mSkeletonAnimationUpdater && skeleton) {
	if(skeleton) {
		EntityPtr e = new Entity("Skeleton animation controller");
		skeletonAnimationComponent = e->addComponent(new SkeletonAnimationComponent(*mSystemEntities));
		e->asChildOf(&mRootEntity);

		skeletonAnimationComponent->skeletonAnimation.skeleton = skeleton;
		MCD_VERIFY(skeletonAnimationComponent->skeletonAnimation.anim.addTrack(*track));
		skeletonAnimationComponent->skeletonAnimation.anim.update();
		skeletonAnimationComponent->skeletonAnimation.applyTo(skeleton->basePose);	// Use the first frame's pose as the base pose
		skeletonAnimationComponent->pose.init(skeleton->basePose.jointCount());
		skeleton->initBasePoseInverse();
	}*/

	// Index of pod node to Entity
	std::vector<EntityPtr> nodeToEntity;
	nodeToEntity.push_back(&mRootEntity);	// SPODNode::nIdxParent gives -1 for no parent

	// Loop for all nodes
	for(size_t i=0; i<mPod.nNumNode; ++i)
	{
		const SPODNode& podNode = mPod.pNode[i];

		if(podNode.nIdx < 0 || podNode.nIdx >= int(mMeshes.size()))
			continue;

		if(podNode.nIdxParent + 1 >= int(nodeToEntity.size()))
			continue;

		EntityPtr e = new Entity(podNode.pszName);
		e->asChildOf(nodeToEntity[podNode.nIdxParent + 1].getNotNull());	// SPODNode::nIdxParent gives -1 for no parent

		if(!hasSkeleton)
			getLocalTransform(mPod, podNode, e->localTransform);

		nodeToEntity.push_back(e);

		// Add mesh component if 1) It's a pod mesh node and 2) The mesh format is supported by our loader.
		// TODO: Sort meshes by material before creating the Entity structure
		if(i < mPod.nNumMeshNode && mMeshes[podNode.nIdx].first->vertexCount > 0)
		{
			// Setup for material
			if(podNode.nIdxMaterial >= 0 && podNode.nIdxMaterial < int(mMaterials.size()))
				e->addComponent(mMaterials[podNode.nIdxMaterial]->clone());

			if(mPod.pMesh[podNode.nIdx].sBoneIdx.n) {
/*				SkinMeshComponentPtr sm = new SkinMeshComponent();
				mSkinMeshToCommit.push_back(std::make_pair(sm, mMeshes[podNode.nIdx].first));
				sm->skeleton = skeleton;
				sm->skeletonAnimation = skeletonAnimationComponent;

				e->addComponent(sm.get());*/
			}
			else {
				Entity* e2 = e->addFirstChild(new Entity);
				MeshComponent* c = e2->addComponent(new MeshComponent);
				c->mesh = mMeshes[podNode.nIdx].first;
			}
		}
	}

	//if(mAnimationUpdater && animationComponent) {
/*	if(animationComponent) {
		// Initialize animation track
		MCD_VERIFY(animationComponent->animationInstance.addTrack(*track));

		// Calculate animation data and link up with the entities
		animationComponent->affectingEntities.push_back(nullptr);
		for(size_t i=cExtraRootNode; i<nodeToEntity.size(); ++i) {
			EntityPtr e = isSkinMeshNode(mPod, i-cExtraRootNode) ? nullptr : nodeToEntity[i];
			animationComponent->affectingEntities.push_back(e);
		}
	}*/

	return Loaded;
}

void PodLoader::Impl::commit(Resource& resource)
{
	// Commit all the mesh buffers
	for(size_t i=0; i<mPod.nNumMesh; ++i) {
		// TODO: Optimization, ignore those mesh in mMeshes with reference count == 1
		Mesh& mesh = *mMeshes[i].first;

		if(mesh.indexCount == 0 || mesh.vertexCount == 0)
			continue;

		MCD_ASSERT(!mPod.pMesh[i].pInterleaved || mesh.bufferCount <= 2);

		const void* data[Mesh::cMaxBufferCount] = { nullptr };
		for(size_t j=0; j<mesh.bufferCount; ++j)
			data[j] = mMeshes[i].second[j];

		MCD_VERIFY(mesh.create(data, Mesh::Static));	// TODO: Way to set the Mesh::StorageHint
//		MeshUtility::computeBoundingBox(mesh);
	}

	// Commit skin mesh
/*	if(mResourceManager) for(size_t i=0; i<mSkinMeshToCommit.size(); ++i) {
		MCD_VERIFY(mSkinMeshToCommit[i].first->init(*mResourceManager, *mSkinMeshToCommit[i].second));
	}*/

	Prefab& prefab = dynamic_cast<Prefab&>(resource);
	prefab.entity.reset(new Entity());

	// Clone all the node in mRootEntity to the target.
	std::auto_ptr<Entity> cloned(mRootEntity.clone());
	for(Entity* i=cloned->firstChild(); i;) {
		Entity* bk = i;
		i = i->nextSibling();
		bk->asChildOf(prefab.entity.get());
	}
}

PodLoader::PodLoader()
	: mImpl(*new Impl)
{
}

PodLoader::~PodLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState PodLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("PodLoader::load");
	return mImpl.load(is, fileId, args);
}

void PodLoader::commit(Resource& resource)
{
	return mImpl.commit(resource);
}

ResourcePtr PodLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "pod") == 0)
		return new Prefab(fileId);
	return nullptr;
}

IResourceLoaderPtr PodLoaderFactory::createLoader()
{
	return new PodLoader;
}

}	// namespace MCD
