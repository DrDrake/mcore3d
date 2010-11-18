#ifndef __MCD_CORE_MATH_ANIMATIONBLENDTREE__
#define __MCD_CORE_MATH_ANIMATIONBLENDTREE__

#include "AnimationState.h"
#include "../System/StringHash.h"
#include "../System/PtrVector.h"
#include <vector>

namespace MCD {

class ResourceManager;

/// 
class MCD_CORE_API AnimationBlendTree
{
public:
	AnimationBlendTree();

	~AnimationBlendTree();

	typedef AnimationClip::Pose Pose;

	class INode
	{
	public:
		FixString name;
		size_t parent;
		virtual ~INode() {}
		virtual void processChild(INode* child, AnimationBlendTree& tree) = 0;
		virtual int returnPose(AnimationBlendTree& tree) = 0;
	};	// INode

	/// It's a leaf node
	class MCD_CORE_API ClipNode : public INode
	{
	public:
		AnimationState state;
		sal_override void processChild(INode* child, AnimationBlendTree& tree) { MCD_ASSERT(false); }
		sal_override int returnPose(AnimationBlendTree& tree);
	};	// ClipNode

	/// Lerp between two child node
	class MCD_CORE_API LerpNode : public INode
	{
	public:
		LerpNode();
		float t;	///< The lerp parameter
		sal_override void processChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
	protected:
		int pose1Idx;
	};	// LerpNode

	/// Perform addition between a full pose and a difference pose
	/// First child as full pose node, second child as difference pose.
	class MCD_CORE_API AdditiveNode : public INode
	{
	public:
		AdditiveNode();
		sal_override void processChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
	protected:
		int pose1Idx;
	};	// AdditiveNode

// Attributes
	float worldTime;

	/// The nodes in this array should be in-order sorted before use.
	ptr_vector<INode> nodes;

// Operations
	/// Sort the nodes as in-ordered.
	/// Must be invoked once you have changes in the tree structure.
	void inOrderSort();

	INode* findNodeByName(const char* name);

	/// Get the final animated data out of this blend tree.
	Pose getFinalPose();

	/// Fill the blend tree from an Xml file, a ResourceManager is also needed
	/// in order to load the animation tracks.
	sal_checkreturn bool loadFromXml(const char* xml, ResourceManager& mgr);

protected:
	int allocatePose(size_t trackCount=0);
	Pose getPose(int idx);
	void releasePose(int idx);
	void resetPoseBuffer();

	size_t mTrackCount;	///< All tree nodes should have the same number of animation channel.
	AnimationClip::Sample* mPoseBuffer;
	static const size_t cPoseCacheSize = 8;
	Array<bool, cPoseCacheSize> mAllocated;
};	// AnimationBlendTree

}	// namespace MCD

#endif	// __MCD_CORE_MATH_ANIMATIONBLENDTREE__
