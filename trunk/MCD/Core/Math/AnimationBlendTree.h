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
	AnimationBlendTree(const AnimationBlendTree& rhs);
	AnimationBlendTree& operator=(const AnimationBlendTree& rhs);

	typedef AnimationClip::Pose Pose;

	class INode
	{
	public:
		FixString name;
		size_t parent;
		INode() : parent(size_t(-1)) {}
		virtual ~INode() {}
		virtual INode* clone() const = 0;

		virtual void processChild(INode* child, AnimationBlendTree& tree) = 0;
		virtual int returnPose(AnimationBlendTree& tree) = 0;

		/// Get the animation clip resource if this node has one, usefull during serialization.
		virtual AnimationClipPtr getClipSource() { return nullptr; }

		virtual std::string xmlStart() const = 0;
		virtual std::string xmlEnd() const = 0;
	};	// INode

	/// It's a leaf node
	class MCD_CORE_API ClipNode : public INode
	{
	public:
		AnimationState state;
		sal_override INode* clone() const;
		sal_override void processChild(INode* child, AnimationBlendTree& tree) { MCD_ASSERT(false); }
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override AnimationClipPtr getClipSource() { return state.clip; }
		sal_override std::string xmlStart() const;
		sal_override std::string xmlEnd() const;
	};	// ClipNode

	/// Lerp between two child node
	class MCD_CORE_API LerpNode : public INode
	{
	public:
		LerpNode();
		float t;	///< The lerp parameter
		sal_override INode* clone() const;
		sal_override void processChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override std::string xmlStart() const;
		sal_override std::string xmlEnd() const;
	protected:
		int pose1Idx;
	};	// LerpNode

	/// Perform addition between a full pose and a difference pose
	/// First child as full pose node, second child as difference pose.
	class MCD_CORE_API AdditiveNode : public INode
	{
	public:
		AdditiveNode();
		sal_override INode* clone() const;
		sal_override void processChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override std::string xmlStart() const;
		sal_override std::string xmlEnd() const;
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

	std::string saveToXml() const;

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