#ifndef __MCD_CORE_MATH_ANIMATIONBLENDTREE__
#define __MCD_CORE_MATH_ANIMATIONBLENDTREE__

#include "AnimationState.h"
#include "ShortestPathMatrix.h"
#include "../System/StringHash.h"
#include "../System/PtrVector.h"
#include <vector>

namespace MCD {

class ResourceManager;

/// Advanced animation control system using a tree structure representation.
///
/// The implementation stores all the nodes in a linear array, with their order
/// sorted as in-order traversal.
///
/// When the user request the final pose of the blend tree, all the nodes are processed
/// in the node array, intermediate results will be calculated on demand and store into
/// a cache. Parent nodes will ask for the cached result of their children such that
/// the result will propagate to the root node.
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
		size_t parent;
		FixString name;
		FixString userData;	///< Currently it is to be used for UI editor persistency
		float duration;		///< Specify the explicit duration of this node (local timeline will freeze after duration), 0 zero for infinity
		float localRefTime;	///< Affect the timeline position of all child nodes (and this node)

		INode() : parent(size_t(-1)), duration(0), localRefTime(0) {}
		virtual ~INode() {}
		virtual INode* clone() const = 0;

		/// Allow the nodes to preform some pre-processing.
		virtual void begin(AnimationBlendTree& tree) {}

		/// Function to let this node know about it's children.
		virtual void collectChild(INode* child, AnimationBlendTree& tree) = 0;

		/// Perform the required calculation, store the result at a cache location and return the cache index.
		virtual int returnPose(AnimationBlendTree& tree) = 0;

		/// Get the animation clip resource if this node has one, usefull during serialization.
		virtual AnimationClipPtr getClipSource() { return nullptr; }

		virtual std::string xmlStart(const AnimationBlendTree&) const = 0;
		virtual std::string xmlEnd() const = 0;

		/// Get the final reference time, by considering the parent node's localRefTime
		float worldRefTime(AnimationBlendTree& tree) const;
	};	// INode

	/// It's a leaf node
	class MCD_CORE_API ClipNode : public INode
	{
	public:
		AnimationState state;
		sal_override INode* clone() const;
		sal_override void collectChild(INode* child, AnimationBlendTree& tree) { MCD_ASSERT(false); }
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override AnimationClipPtr getClipSource() { return state.clip; }
		sal_override std::string xmlStart(const AnimationBlendTree&) const;
		sal_override std::string xmlEnd() const;
	};	// ClipNode

	/// Lerp between two child node
	class MCD_CORE_API LerpNode : public INode
	{
	public:
		LerpNode();
		float t;	///< The lerp parameter
		sal_override INode* clone() const;
		sal_override void collectChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override std::string xmlStart(const AnimationBlendTree&) const;
		sal_override std::string xmlEnd() const;
	protected:
		INode* mNode1, *mNode2;
	};	// LerpNode

	class MCD_CORE_API SubtractiveNode : public INode
	{
	public:
		SubtractiveNode();
		sal_override INode* clone() const;
		sal_override void collectChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override std::string xmlStart(const AnimationBlendTree&) const;
		sal_override std::string xmlEnd() const;
	protected:
		INode* mNode1, *mNode2;
	};	// SubtractiveNode

	/// Perform addition between a full pose and a difference pose
	/// First child as full pose node, second child as difference pose.
	class MCD_CORE_API AdditiveNode : public INode
	{
	public:
		AdditiveNode();
		sal_override INode* clone() const;
		sal_override void collectChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override std::string xmlStart(const AnimationBlendTree&) const;
		sal_override std::string xmlEnd() const;
	protected:
		INode* mNode1, *mNode2;
	};	// AdditiveNode

	/// Switch between the child nodes, with a cross-fade duration option
	class MCD_CORE_API SwitchNode : public INode
	{
	public:
		SwitchNode();
		sal_override INode* clone() const;
		sal_override void collectChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override std::string xmlStart(const AnimationBlendTree&) const;
		sal_override std::string xmlEnd() const;
		void switchTo(int nodeIdx, float timeToSwitch);	///< Perform the switching at a specific world time

		int currentNode() const;	///< The current using node

		float fadeDuration;	///< The duration used during cross-fading

	protected:
		int mCurrentNode, mLastNode;
		float mNodeChangeTime;	///< The world time when the last node change happened
		INode* mNode1, *mNode2;
	};	// SwitchNode

	/// A "smart" switching node
	/// Any nodes as a direct child of the FsmNode will be considered as a state,
	/// user need to defind "edge" between state to represent the possible transition
	/// between states.
	class MCD_CORE_API FsmNode : public INode
	{
	public:
		explicit FsmNode(AnimationBlendTree& tree);
		sal_override INode* clone() const;
		sal_override void begin(AnimationBlendTree& tree);
		sal_override void collectChild(INode* child, AnimationBlendTree& tree);
		sal_override int returnPose(AnimationBlendTree& tree);
		sal_override std::string xmlStart(const AnimationBlendTree&) const;
		sal_override std::string xmlEnd() const;

		/// -1 if not able to switch (no possible path).
		/// 0 if able to start the transition immediatly
		/// 1 if the transition is pending and will take place at a later time (Wait until the current animation or corss-fade to finish)
		int switchTo(int nodeIdx);

		int startingNode;
		int mTargetingNode;

		struct Transition
		{
			/// Transition type
			/// Sync: the current animation must be finished before transiting to the next
			/// ASync: the transition will start immediatly with a cross fading (with adjustable fade duration)
			/// Auto: state A (ended) can transit to state B automatically if there exist an Auto transition between A and B, implies Sync
			enum Type { Sync, ASync, Auto };
			int src, dest;
			Type type;
			float duration;
		};
		typedef std::vector<Transition> Transitions;
		Transitions transitions;

	protected:
		sal_maybenull Transition* findTransitionFor(int src, int dest);

		AnimationBlendTree& mTree;
		ShortestPathMatrix mShortestPath;
		float mFadeDuration;

		int mCurrentNode, mLastNode;
		INode* mNode1, *mNode2;	///< Pointer to the nodes (2 nodes are involved during corss fading)
	};	// FsmNode

// Attributes
	float worldTime;

	/// The nodes in this array should be in-order sorted before use.
	typedef ptr_vector<INode> Nodes;
	Nodes nodes;

// Operations
	/// Returns -1 if the node cannot be found
	int findNodeIndexByName(const char* name);

	/// Returns null if the node cannot be found
	sal_maybenull INode* findNodeByName(const char* name);

	/// Sort the nodes as in-ordered.
	/// Must be invoked once you have changes in the tree structure.
	void inOrderSort();

	/// Get the final animated data out of this blend tree.
	Pose getFinalPose();

	/// Fill the blend tree from an Xml file, a ResourceManager is also needed
	/// in order to load the animation tracks.
	sal_checkreturn bool loadFromXml(const char* xml, ResourceManager& mgr, const char* clipSearchPath=nullptr);

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
