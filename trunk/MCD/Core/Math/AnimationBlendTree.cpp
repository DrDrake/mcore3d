#include "Pch.h"
#include "AnimationBlendTree.h"
#include "Quaternion.h"
#include "../System/Deque.h"
#include "../System/ResourceManager.h"
#include "../System/StrUtility.h"
#include "../System/XmlParser.h"

namespace MCD {

typedef AnimationBlendTree::Pose Pose;
typedef AnimationBlendTree::INode INode;

AnimationBlendTree::AnimationBlendTree()
	: mTrackCount(0), mPoseBuffer(nullptr)
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

struct Sorter
{
	Sorter(ptr_vector<INode>& s) : src(s)
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

	ptr_vector<INode>& src;
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
	const size_t ignoreLastNode = 1;
	for(size_t i=0; i<nodes.size() - ignoreLastNode; ++i) {
		INode& childNode = nodes[i];
		INode& parentNode = nodes[childNode.parent];
		parentNode.processChild(&childNode, *this);
	}

	const size_t outputIdx = nodes.back().returnPose(*this);
	releasePose(outputIdx);
	return getPose(outputIdx);
}

static INode* loadClipNode(XmlParser& parser, ResourceManager& mgr)
{
	AnimationBlendTree::ClipNode* n = new AnimationBlendTree::ClipNode;
	n->state.rate = parser.attributeValueAsFloat("rate", 1);
	n->state.clip = mgr.loadAs<AnimationClip>(parser.attributeValue("src"), 1);
	return n;
}

static INode* loadLerpNode(XmlParser& parser)
{
	AnimationBlendTree::LerpNode* n = new AnimationBlendTree::LerpNode;
	n->name = parser.attributeValue("name");
	n->t = parser.attributeValueAsFloat("t", 0.5f);
	return n;
}

static INode* loadAdditiveNode(XmlParser& parser)
{
	AnimationBlendTree::AdditiveNode* n = new AnimationBlendTree::AdditiveNode;
	return n;
}

bool AnimationBlendTree::loadFromXml(const char* xml, ResourceManager& mgr)
{
	char* tmp = ::strdup(xml);

	typedef XmlParser::Event Event;
	XmlParser parser;
	parser.parse(tmp);

	std::stack<size_t> parentIdx;

	bool ended = false;
	while(!ended)
	{
		Event::Enum e = parser.nextEvent();
		INode* n = nullptr;

		switch(e)
		{
		case Event::BeginElement:
			if(strcmp(parser.elementName(), "clip") == 0)
				n = loadClipNode(parser, mgr);
			else if(strcmp(parser.elementName(), "lerp") == 0)
				n = loadLerpNode(parser);
			else if(strcmp(parser.elementName(), "additive") == 0)
				n = loadAdditiveNode(parser);

			if(!n)
				return false;

			n->parent = parentIdx.empty() ? size_t(-1) : parentIdx.top();
			parentIdx.push(nodes.size());
			nodes.push_back(n);
			break;
		case Event::EndElement:
			parentIdx.pop();
			break;
		case Event::Error:
		case Event::EndDocument:
			ended = true;
			break;

		default:
			break;
		}
	}

	inOrderSort();
	::free(tmp);

	return true;
}

std::string AnimationBlendTree::saveToXml() const
{
	std::string ret;
	size_t lastParentIdx = size_t(-1);
	for(size_t i=0; i<nodes.size(); ++i) {
		if(i == lastParentIdx)
			ret = nodes[i].xmlStart() + ret + nodes[i].xmlEnd();
		else
			ret = ret + nodes[i].xmlStart() + nodes[i].xmlEnd();
		lastParentIdx = nodes[i].parent;
	}

	return ret;
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

std::string AnimationBlendTree::ClipNode::xmlStart() const
{
	std::string ret = "<clip ";
	ret += "rate=\"" + float2Str(state.rate) + "\"";
	ret += "src=\"" + state.clip->fileId().getString() + "\"";
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::ClipNode::xmlEnd() const
{
	return "</clip>";
}

AnimationBlendTree::LerpNode::LerpNode()
	: t(0.5f), pose1Idx(-1)
{}

INode* AnimationBlendTree::LerpNode::clone() const
{
	return new LerpNode(*this);
}

void AnimationBlendTree::LerpNode::processChild(AnimationBlendTree::INode* child, AnimationBlendTree& tree)
{
	if(pose1Idx < 0) {
		pose1Idx = child->returnPose(tree);
	}
	else {
		int pose2Idx = child->returnPose(tree);
		Pose pose1 = tree.getPose(pose1Idx);
		Pose pose2 = tree.getPose(pose2Idx);
		MCD_ASSERT(pose1.size == pose2.size);

		for(size_t i=0; i<pose1.size; ++i)
			pose1[i].blend(t, pose1[i], pose2[i]);

		tree.releasePose(pose2Idx);
	}
}

int AnimationBlendTree::LerpNode::returnPose(AnimationBlendTree& tree)
{
	return pose1Idx;
}

std::string AnimationBlendTree::LerpNode::xmlStart() const
{
	std::string ret = "<lerp ";
	if(!name.empty()) ret += std::string("name=\"") + name.c_str() + "\"";
	ret += "t=\"" + float2Str(t) + "\"";
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::LerpNode::xmlEnd() const
{
	return "</lerp>";
}

AnimationBlendTree::AdditiveNode::AdditiveNode()
	: pose1Idx(-1)
{}

INode* AnimationBlendTree::AdditiveNode::clone() const
{
	return new AdditiveNode(*this);
}

void AnimationBlendTree::AdditiveNode::processChild(AnimationBlendTree::INode* child, AnimationBlendTree& tree)
{
	if(pose1Idx < 0) {
		pose1Idx = child->returnPose(tree);
	}
	else {
		int pose2Idx = child->returnPose(tree);
		Pose pose1 = tree.getPose(pose1Idx);	// pose1 is the full pose
		Pose pose2 = tree.getPose(pose2Idx);	// pose2 is the pose diff
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

		tree.releasePose(pose2Idx);
	}
}

int AnimationBlendTree::AdditiveNode::returnPose(AnimationBlendTree& tree)
{
	return pose1Idx;
}

std::string AnimationBlendTree::AdditiveNode::xmlStart() const
{
	std::string ret = "<additive";
	ret += ">";
	return ret;
}

std::string AnimationBlendTree::AdditiveNode::xmlEnd() const
{
	return "</additive>";
}

}	// namespace MCD
