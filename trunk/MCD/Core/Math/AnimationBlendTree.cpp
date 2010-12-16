#include "Pch.h"
#include "AnimationBlendTree.h"
#include "Quaternion.h"
#include "../System/Deque.h"
#include "../System/ResourceManager.h"
#include "../System/StrUtility.h"
#include "../System/XmlParser.h"
#include <map>

namespace MCD {

typedef AnimationBlendTree::Pose Pose;
typedef AnimationBlendTree::INode INode;

AnimationBlendTree::AnimationBlendTree()
	: worldTime(0), mTrackCount(0), mPoseBuffer(nullptr)
{
}

AnimationBlendTree::~AnimationBlendTree()
{
	resetPoseBuffer();
}

AnimationBlendTree::AnimationBlendTree(const AnimationBlendTree& rhs)
	: mTrackCount(0), mPoseBuffer(nullptr)
{
	*this = rhs;
}

AnimationBlendTree& AnimationBlendTree::operator=(const AnimationBlendTree& rhs)
{
	resetPoseBuffer();
	for(size_t i=0; i<rhs.nodes.size(); ++i)
		nodes.push_back(rhs.nodes[i].clone());
	return *this;
}

int AnimationBlendTree::allocatePose(size_t trackCount)
{
	if(0 == mTrackCount && 0 != trackCount) {
		resetPoseBuffer();
		mTrackCount = trackCount;
		mPoseBuffer = new AnimationClip::Sample[trackCount * cPoseCacheSize];
	}

	MCD_ASSERT(mTrackCount > 0);

	for(size_t i=0; i<mAllocated.size(); ++i) {
		if(!mAllocated[i]) {
			mAllocated[i] = true;
			return i;
		}
	}

	MCD_ASSERT(false && "Not enough animation pose buffer");
	return -1;
}

Pose AnimationBlendTree::getPose(int idx)
{
	MCD_ASSERT(idx >= 0);
	return Pose(&mPoseBuffer[idx * mTrackCount], mTrackCount);
}

void AnimationBlendTree::releasePose(int idx)
{
	mAllocated[idx] = false;
}

void AnimationBlendTree::resetPoseBuffer()
{
	mTrackCount = 0;
	delete[] mPoseBuffer;
	mPoseBuffer = nullptr;
	mAllocated.assign(false);
}

int AnimationBlendTree::findNodeIndexByName(const char* name)
{
	for(size_t i=0; i<nodes.size(); ++i) {
		if(FixString(name) == nodes[i].name)
			return int(i);
	}
	return -1;
}

INode* AnimationBlendTree::findNodeByName(const char* name)
{
	for(size_t i=0; i<nodes.size(); ++i) {
		if(FixString(name) == nodes[i].name)
			return &nodes[i];
	}
	return nullptr;
}

struct Sorter
{
	Sorter(AnimationBlendTree::Nodes& s) : src(s)
	{
		// Build the children structure
		children.resize(src.size());
		for(size_t i=0; i<src.size(); ++i) {
			std::vector<size_t>& n = children[i];
			for(size_t j=0; j<src.size(); ++j) {
				if(i == src[j].parent)
					n.push_back(j);
			}
		}

		// Find the root index
		size_t rootIdx = size_t(-1);
		for(size_t i=0; i<src.size(); ++i) {
			if(src[i].parent >= src.size() || src[i].parent == i) {
				rootIdx = i;
				break;
			}
		}

		MCD_ASSERT(rootIdx < src.size());

		// Perfor the traverse and the sorted result will store in output
		doit(rootIdx);
	}

	// Traverse recursively
	void doit(size_t srcIdx) {
		std::vector<size_t>& n = children[srcIdx];
		for(size_t i=0; i<n.size(); ++i)
			doit(n[i]);
		output.push_back(&src[srcIdx]);

		for(size_t i=0; i<n.size(); ++i)
			src[n[i]].parent = output.size() - 1;
	}

	AnimationBlendTree::Nodes& src;
	std::vector<std::vector<size_t> > children;
	std::vector<INode*> output;
};	// Sorter

void AnimationBlendTree::inOrderSort()
{
	Sorter sorter(nodes);
	nodes.clear(false);

	for(size_t i=0; i<sorter.output.size(); ++i)
		nodes.push_back(sorter.output[i]);
}

Pose AnimationBlendTree::getFinalPose()
{
	for(size_t i=0; i<nodes.size(); ++i)
		nodes[i].begin(*this);

	const size_t ignoreLastNode = 1;
	for(size_t i=0; i<nodes.size() - ignoreLastNode; ++i) {
		INode& childNode = nodes[i];
		INode& parentNode = nodes[childNode.parent];
		parentNode.collectChild(&childNode, *this);
	}

	const size_t outputIdx = nodes.back().returnPose(*this);
	releasePose(outputIdx);
	return getPose(outputIdx);
}

// Convert any name reference in the XML back to index (after the in-order sort)
typedef void (*fixupFunc)(INode*, int index, void* userData);
struct Fixup { FixString name; fixupFunc func; INode* node; void* userData; };
typedef std::vector<Fixup> IndexFixupTable;

static INode* loadClipNode(XmlParser& parser, ResourceManager& mgr, const char* clipSearchPath)
{
	AnimationBlendTree::ClipNode* n = new AnimationBlendTree::ClipNode;
	n->state.rate = parser.attributeValueAsFloat("rate", 1);

	Path path(parser.attributeValue("src"));
	if(!path.hasRootDirectory() && clipSearchPath)
		path = Path(clipSearchPath)/path;
	n->state.clip = mgr.loadAs<AnimationClip>(path, 1);
	return n;
}

static INode* loadLerpNode(XmlParser& parser)
{
	AnimationBlendTree::LerpNode* n = new AnimationBlendTree::LerpNode;
	n->t = parser.attributeValueAsFloat("t", 0.5f);
	return n;
}

static INode* loadSubtractiveNode(XmlParser& parser)
{
	AnimationBlendTree::SubtractiveNode* n = new AnimationBlendTree::SubtractiveNode;
	return n;
}

static INode* loadAdditiveNode(XmlParser& parser)
{
	AnimationBlendTree::AdditiveNode* n = new AnimationBlendTree::AdditiveNode;
	return n;
}

static void switchNodeFixup(INode* node, int index, void*)
{
	static_cast<AnimationBlendTree::SwitchNode*>(node)->switchTo(index, 0);
}

static INode* loadSwitchNode(XmlParser& parser, IndexFixupTable& fixupTable)
{
	AnimationBlendTree::SwitchNode* n = new AnimationBlendTree::SwitchNode;
	n->fadeDuration = parser.attributeValueAsFloat("fadeDuration", 0);
	FixString current = parser.attributeValue("current");
	Fixup fixup = { current, &switchNodeFixup, n, nullptr };
	fixupTable.push_back(fixup);
	return n;
}

static void fsmNodeFixup(INode* node, int index, void*) {
	static_cast<AnimationBlendTree::FsmNode*>(node)->startingNode = index;
}

static void fsmTransitionSrcFixup(INode* node, int index, void* ud) {
	reinterpret_cast<AnimationBlendTree::FsmNode*>(node)->transitions[int(ud)].src = index;
}

static void fsmTransitionDestFixup(INode* node, int index, void* ud) {
	reinterpret_cast<AnimationBlendTree::FsmNode*>(node)->transitions[int(ud)].dest = index;
}

static INode* loadFsmNode(XmlParser& parser, IndexFixupTable& fixupTable, AnimationBlendTree& tree)
{
	AnimationBlendTree::FsmNode* n = new AnimationBlendTree::FsmNode(tree);
	FixString starting = parser.attributeValue("startingNode");
	Fixup fixup = { starting, &fsmNodeFixup, n, nullptr };
	fixupTable.push_back(fixup);

	bool done = false;
	while(!done)
	{
		typedef XmlParser::Event Event;
		typedef AnimationBlendTree::FsmNode::Transition Transition;
		Event::Enum e = parser.nextEvent();
		switch(e) {
		case Event::BeginElement:
			if(strcmp(parser.elementName(), "transition") == 0) {
				Transition t;
				t.type = (strcmp(parser.attributeValue("type"), "sync") == 0) ? Transition::Sync : Transition::ASync;

				{	FixString srcName = parser.attributeValue("src");
					Fixup fixup = { srcName, &fsmTransitionSrcFixup, n, (void*)n->transitions.size() };
					fixupTable.push_back(fixup);
				}

				{	FixString destName = parser.attributeValue("dest");
					Fixup fixup = { destName, &fsmTransitionDestFixup, n, (void*)n->transitions.size() };
					fixupTable.push_back(fixup);
				}

				n->transitions.push_back(t);
			}
			break;
		case Event::EndElement:
			if(strcmp(parser.elementName(), "transitions") == 0)
				done = true;
			else if(strcmp(parser.elementName(), "fsm") == 0)
				done = true;
			break;
		}
	}

	return n;
}

bool AnimationBlendTree::loadFromXml(const char* xml, ResourceManager& mgr, const char* clipSearchPath)
{
	char* tmp = ::strdup(xml);

	typedef XmlParser::Event Event;
	XmlParser parser;
	parser.parse(tmp);

	// Reset all stateful member first
	nodes.clear();
	mTrackCount = 0;

	std::stack<size_t> parentIdx;

	// Convert any name reference in the XML back to index (after the in-order sort)
	IndexFixupTable indexFixupTable;

	bool done = false;
	while(!done)
	{
		Event::Enum e = parser.nextEvent();
		INode* n = nullptr;

		switch(e)
		{
		case Event::BeginElement:
			if(strcmp(parser.elementName(), "clip") == 0)
				n = loadClipNode(parser, mgr, clipSearchPath);
			else if(strcmp(parser.elementName(), "lerp") == 0)
				n = loadLerpNode(parser);
			else if(strcmp(parser.elementName(), "subtractive") == 0)
				n = loadSubtractiveNode(parser);
			else if(strcmp(parser.elementName(), "additive") == 0)
				n = loadAdditiveNode(parser);
			else if(strcmp(parser.elementName(), "switch") == 0)
				n = loadSwitchNode(parser, indexFixupTable);
			else if(strcmp(parser.elementName(), "fsm") == 0)
				n = loadFsmNode(parser, indexFixupTable, *this);

			if(!n)
				return false;

			n->name = parser.attributeValue("name");
			n->userData = parser.attributeValue("userData");

			n->parent = parentIdx.empty() ? size_t(-1) : parentIdx.top();
			parentIdx.push(nodes.size());
			nodes.push_back(n);
			break;
		case Event::EndElement:
			parentIdx.pop();
			break;
		case Event::Error:
		case Event::EndDocument:
			done = true;
			break;

		default:
			break;
		}
	}

	inOrderSort();
	::free(tmp);

	// Apply the fixup table
	for(IndexFixupTable::const_iterator i=indexFixupTable.begin(); i!=indexFixupTable.end(); ++i) {
		int idx = findNodeIndexByName(i->name.c_str());
		MCD_ASSERT(idx >= 0 && idx < (int)nodes.size());
		(*i->func)(i->node, idx, i->userData);
	}

	return true;
}

static std::string getXmlStr(const AnimationBlendTree& tree, size_t nodeIdx, const std::vector<std::vector<size_t> >& children)
{
	std::string ret = tree.nodes[nodeIdx].xmlStart(tree);

	const std::vector<size_t>& c = children[nodeIdx];
	for(size_t i=0; i<c.size(); ++i)
		ret += getXmlStr(tree, c[i], children);	// Invoke recursively
	ret += tree.nodes[nodeIdx].xmlEnd();

	return ret;
}

std::string AnimationBlendTree::saveToXml() const
{
	if(nodes.empty()) return "";

	Sorter sorter(const_cast<Nodes&>(nodes));
	return getXmlStr(*this, nodes.size()-1, sorter.children);
}

INode* AnimationBlendTree::ClipNode::clone() const
{
	return new ClipNode(*this);
}

int AnimationBlendTree::ClipNode::returnPose(AnimationBlendTree& tree)
{
	state.worldTime = tree.worldTime;
	int i = tree.allocatePose(state.clip->trackCount());
	MCD_ASSERT(i >= 0);
	state.assignTo(tree.getPose(i));
	return i;
}

std::string AnimationBlendTree::ClipNode::xmlStart(const AnimationBlendTree&) const
{
	std::string ret = "<clip";
	if(!name.empty()) ret += formatStr(" name=\"%s\"", name.c_str());
	if(duration != 0) ret += formatStr(" duration=\"%f\"", duration);
	ret += formatStr(" rate=\"%f\"", state.rate);
	ret += formatStr(" src=\"%s\"", state.clip->fileId().c_str());
	if(!userData.empty()) ret += formatStr(" userData=\"%s\"", userData.c_str());
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::ClipNode::xmlEnd() const
{
	return "</clip>";
}

AnimationBlendTree::LerpNode::LerpNode()
	: t(0.5f), mNode1(nullptr), mNode2(nullptr)
{}

INode* AnimationBlendTree::LerpNode::clone() const
{
	return new LerpNode(*this);
}

void AnimationBlendTree::LerpNode::collectChild(AnimationBlendTree::INode* child, AnimationBlendTree& tree)
{
	if(!mNode1) mNode1 = child;
	else mNode2 = child;
}

int AnimationBlendTree::LerpNode::returnPose(AnimationBlendTree& tree)
{
	if(t == 0) return mNode1->returnPose(tree);
	if(t == 1) return mNode2->returnPose(tree);

	int idx1 = mNode1->returnPose(tree);
	int idx2 = mNode2->returnPose(tree);
	Pose pose1 = tree.getPose(idx1);
	Pose pose2 = tree.getPose(idx2);
	MCD_ASSERT(pose1.size == pose2.size);

	for(size_t i=0; i<pose1.size; ++i)
		pose1[i].blend(t, pose1[i], pose2[i]);
	tree.releasePose(idx2);

	return idx1;
}

std::string AnimationBlendTree::LerpNode::xmlStart(const AnimationBlendTree&) const
{
	std::string ret = "<lerp";
	if(!name.empty()) ret += formatStr(" name=\"%s\"", name.c_str());
	if(duration != 0) ret += formatStr(" duration=\"%f\"", duration);
	ret += formatStr(" t=\"%f\"", t);
	if(!userData.empty()) ret += formatStr(" userData=\"%s\"", userData.c_str());
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::LerpNode::xmlEnd() const
{
	return "</lerp>";
}

AnimationBlendTree::SubtractiveNode::SubtractiveNode()
	: mNode1(nullptr), mNode2(nullptr)
{}

INode* AnimationBlendTree::SubtractiveNode::clone() const
{
	return new SubtractiveNode(*this);
}

void AnimationBlendTree::SubtractiveNode::collectChild(AnimationBlendTree::INode* child, AnimationBlendTree& tree)
{
	if(!mNode1) mNode1 = child;
	else mNode2 = child;
}

int AnimationBlendTree::SubtractiveNode::returnPose(AnimationBlendTree& tree)
{
	int idx1 = mNode1->returnPose(tree);	// The targeting node
	int idx2 = mNode2->returnPose(tree);	// The master node
	Pose pose1 = tree.getPose(idx1);
	Pose pose2 = tree.getPose(idx2);
	MCD_ASSERT(pose1.size == pose2.size);

	for(size_t i=0; i<pose1.size; ++i) {
		MCD_ASSERT(pose1[i].flag == pose2[i].flag);
		switch(pose1[i].flag) {
		case AnimationClip::Linear:
		case AnimationClip::Step:
			pose1[i].v = pose1[i].v - pose2[i].v;
			break;
		case AnimationClip::Slerp:
			// target = master * master.inverse() * target
			// target = master * diff where diff = master.inverse() * target
			pose1[i].cast<Quaternionf>() = pose2[i].cast<Quaternionf>().inverse() * pose1[i].cast<Quaternionf>();
			break;
		default: MCD_ASSERT(false);
		}
	}

	tree.releasePose(idx2);

	return idx1;
}

std::string AnimationBlendTree::SubtractiveNode::xmlStart(const AnimationBlendTree&) const
{
	std::string ret = "<subtractive";
	if(!name.empty()) ret += formatStr(" name=\"%s\"", name.c_str());
	if(duration != 0) ret += formatStr(" duration=\"%f\"", duration);
	if(!userData.empty()) ret += formatStr(" userData=\"%s\"", userData.c_str());
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::SubtractiveNode::xmlEnd() const
{
	return "</subtractive>";
}

AnimationBlendTree::AdditiveNode::AdditiveNode()
	: mNode1(nullptr), mNode2(nullptr)
{}

INode* AnimationBlendTree::AdditiveNode::clone() const
{
	return new AdditiveNode(*this);
}

void AnimationBlendTree::AdditiveNode::collectChild(AnimationBlendTree::INode* child, AnimationBlendTree& tree)
{
	if(!mNode1) mNode1 = child;
	else mNode2 = child;
}

int AnimationBlendTree::AdditiveNode::returnPose(AnimationBlendTree& tree)
{
	int idx1 = mNode1->returnPose(tree);
	int idx2 = mNode2->returnPose(tree);
	Pose pose1 = tree.getPose(idx1);	// pose1 is the full pose
	Pose pose2 = tree.getPose(idx2);	// pose2 is the pose diff
	MCD_ASSERT(pose1.size == pose2.size);

	for(size_t i=0; i<pose1.size; ++i) {
		switch(pose1[i].flag) {
		case AnimationClip::Linear:
		case AnimationClip::Step:
			pose1[i].v = pose1[i].v + pose2[i].v;
			break;
		case AnimationClip::Slerp:
			pose1[i].cast<Quaternionf>() = pose1[i].cast<Quaternionf>() * pose2[i].cast<Quaternionf>();
			break;
		default: MCD_ASSERT(false);
		}
	}

	tree.releasePose(idx2);

	return idx1;
}

std::string AnimationBlendTree::AdditiveNode::xmlStart(const AnimationBlendTree&) const
{
	std::string ret = "<additive";
	if(!name.empty()) ret += formatStr(" name=\"%s\"", name.c_str());
	if(duration != 0) ret += formatStr(" duration=\"%f\"", duration);
	if(!userData.empty()) ret += formatStr(" userData=\"%s\"", userData.c_str());
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::AdditiveNode::xmlEnd() const
{
	return "</additive>";
}

AnimationBlendTree::SwitchNode::SwitchNode()
	: mCurrentNode(-1), mLastNode(-1), fadeDuration(0), mNodeChangeTime(0)
	, mNode1(nullptr), mNode2(nullptr)
{}

void AnimationBlendTree::SwitchNode::switchTo(int nodeIdx, float timeToSwitch)
{
	mLastNode = mCurrentNode;
	mCurrentNode = nodeIdx;
	mNodeChangeTime = timeToSwitch;
}

int AnimationBlendTree::SwitchNode::currentNode() const
{
	return mCurrentNode;
}

INode* AnimationBlendTree::SwitchNode::clone() const
{
	return new SwitchNode(*this);
}

void AnimationBlendTree::SwitchNode::collectChild(AnimationBlendTree::INode* child, AnimationBlendTree& tree)
{
	if(!mNode1)
		mNode1 = (mLastNode < 0 || (&tree.nodes[mLastNode]) == child) ? child : nullptr;
	if(!mNode2)
		mNode2 = (mCurrentNode < 0 || (&tree.nodes[mCurrentNode]) == child) ? child : nullptr;
}

int AnimationBlendTree::SwitchNode::returnPose(AnimationBlendTree& tree)
{
	INode* n1 = mNode1, *n2 = mNode2;
	mNode1 = mNode2 = nullptr;

	if(tree.worldTime >= mNodeChangeTime + fadeDuration)
		return n2->returnPose(tree);
	else if(tree.worldTime <= mNodeChangeTime)
		return n1->returnPose(tree);

	const float lerpFactor = (tree.worldTime - mNodeChangeTime) / fadeDuration;
	Pose pose1 = tree.getPose(mLastNode);
	Pose pose2 = tree.getPose(mCurrentNode);
	MCD_ASSERT(pose1.size == pose2.size);
	MCD_ASSERT(lerpFactor >= 0 && lerpFactor <= 1);

	for(size_t i=0; i<pose1.size; ++i)
		pose1[i].blend(lerpFactor, pose1[i], pose2[i]);

	tree.releasePose(mCurrentNode);
	return mLastNode;
}

std::string AnimationBlendTree::SwitchNode::xmlStart(const AnimationBlendTree& tree) const
{
	std::string ret = "<switch";
	if(!name.empty()) ret += formatStr(" name=\"%s\"", name.c_str());
	if(duration != 0) ret += formatStr(" duration=\"%f\"", duration);
	if(fadeDuration > 0) ret += formatStr(" startingNode=\"%f\"", fadeDuration);
	if(mCurrentNode >= 0) ret += formatStr(" current=\"%s\"", tree.nodes[mCurrentNode].name.c_str());
	if(!userData.empty()) ret += formatStr(" userData=\"%s\"", userData.c_str());
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::SwitchNode::xmlEnd() const
{
	return "</switch>";
}

AnimationBlendTree::FsmNode::FsmNode(AnimationBlendTree& tree)
	: mTree(tree), mLastNode(-1)
	, fadeDuration(0.2f)
	, currentNodeStartingTime(0)
	, mNodeFadingTo(-1)
	, mNode1(nullptr), mNode2(nullptr)
{}

INode* AnimationBlendTree::FsmNode::clone() const
{
	return new FsmNode(*this);
}

void AnimationBlendTree::FsmNode::begin(AnimationBlendTree& tree)
{
	const float currentTime = tree.worldTime;
	const float currentNodeDuration = tree.nodes[currentNode].duration;

	// Current node haven't finished yet
	if(currentNodeDuration == 0 || currentNodeStartingTime + currentNodeDuration < currentTime) return;

	// Target node not reached, find the next intermediate node to transit
	if(currentNode != targetingNode) {
		int next = mShortestPath.getNext(currentNode, targetingNode);
		// No possible node to move on
		if(next == -1) return;

		currentNode = next;
		currentNodeStartingTime += currentNodeDuration;
		return;
	}

	// We have reached the specified destination, and the current node reached it's end,
	// find the possible node to move on
}

void AnimationBlendTree::FsmNode::collectChild(AnimationBlendTree::INode* child, AnimationBlendTree& tree)
{
	if(!mNode1)
		mNode1 = (currentNode < 0 || (&tree.nodes[currentNode]) == child) ? child : nullptr;
	if(!mNode2)
		mNode2 = (mNodeFadingTo < 0 || (&tree.nodes[mNodeFadingTo]) == child) ? child : nullptr;
}

int AnimationBlendTree::FsmNode::returnPose(AnimationBlendTree& tree)
{
/*	INode* n1 = mNode1, *n2 = mNode2;
	mNode1 = mNode2 = nullptr;

	if(tree.worldTime >= mNodeChangeTime + fadeDuration)
		return n2->returnPose(tree);
	else if(tree.worldTime <= mNodeChangeTime)
		return n1->returnPose(tree);

	const float lerpFactor = (tree.worldTime - mNodeChangeTime) / fadeDuration;
	Pose pose1 = tree.getPose(mLastNode);
	Pose pose2 = tree.getPose(mCurrentNode);
	MCD_ASSERT(pose1.size == pose2.size);
	MCD_ASSERT(lerpFactor >= 0 && lerpFactor <= 1);

	for(size_t i=0; i<pose1.size; ++i)
		pose1[i].blend(lerpFactor, pose1[i], pose2[i]);

	tree.releasePose(mCurrentNode);
	return mLastNode;*/
	return 0;
}

std::string AnimationBlendTree::FsmNode::xmlStart(const AnimationBlendTree& tree) const
{
	std::string ret = "<fsm";
	if(!name.empty()) ret += formatStr(" name=\"%s\"", name.c_str());
	if(duration != 0) ret += formatStr(" duration=\"%f\"", duration);
	ret += formatStr(" startingNode=\"%s\"", tree.nodes[startingNode].name.c_str());
	ret += ">";

	ret += "<transitions>";
	for(Transitions::const_iterator i=transitions.begin(); i!=transitions.end(); ++i) {
		ret += formatStr("<transition type=\"%s\" src=\"%s\" dest=\"%s\"/>",
			i->type == Transition::Sync ? "sync" : "async",
			tree.nodes[i->src].name.c_str(),
			tree.nodes[i->dest].name.c_str()
		);
	}
	ret += "</transitions>";

	return ret;
}

std::string AnimationBlendTree::FsmNode::xmlEnd() const
{
	return "</fsm>";
}

int AnimationBlendTree::FsmNode::switchTo(int nodeIdx)
{
	AnimationBlendTree::Nodes& nodes = mTree.nodes;
	MCD_ASSERT(nodeIdx > 0 && nodeIdx < (int)nodes.size());
	MCD_ASSERT("You can only swithc to node under the FsmNode" && (&nodes[nodes[nodeIdx].parent] == this));

	// Init the shortest path matrix for the first time
	if(mShortestPath.vertexCount == 0) {
		size_t childCount = 0;
		// Search for all direct children under this node
		for(size_t i=0; i<nodes.size(); ++i) {
			const size_t parent = nodes[i].parent;
			if(parent < nodes.size() && (&nodes[parent]) == this)
				++childCount;
		}
		mShortestPath.resize(childCount);

		// Fill the adjacency matrix
		for(size_t i=0; i<transitions.size(); ++i) {
			// NOTE: Currently we don't have weighted transition, all are having the same length of 1
			mShortestPath.distance(transitions[i].src, transitions[i].dest) = 1;
		}

		mShortestPath.preProcess();
	}

	const int next = mShortestPath.getNext(currentNode, nodeIdx);
	if(-1 == next) return -1;	// No possible path

	// Find the transition info
	Transition* t = findTransitionFor(currentNode, next);
	MCD_ASSUME(t && "Discrepancy between the transition info and the adjacency matrix");

	targetingNode = nodeIdx;

	// If we where performing cross fade
	if(-1 != mNodeFadingTo) return 1;

	if(t->type == Transition::ASync) {
		currentNode = next;
		currentNodeStartingTime = mTree.worldTime;
		return 0;
	}

	return 1;
}

AnimationBlendTree::FsmNode::Transition* AnimationBlendTree::FsmNode::findTransitionFor(int src, int dest)
{
	for(size_t i=0; i<transitions.size(); ++i) {
		if(transitions[i].src == src && transitions[i].dest == dest)
			return &transitions[i];
	}
	return nullptr;
}

}	// namespace MCD
