#include "Pch.h"
#include "../../../MCD/Core/Math/Vec4.h"
#include "../../../MCD/Core/Math/Quaternion.h"
#include "../../../MCD/Core/Math/AnimationBlendTree.h"
#include "../../../MCD/Core/System/ResourceManager.h"
#include "../../../MCD/Core/System/RawFileSystem.h"

using namespace MCD;

class AnimationBlendTreeTestFixture
{
public:
	AnimationBlendTreeTestFixture()
	{
		createClips();
	}

	void createClips()
	{
		const size_t trackCount = 1;
		const size_t keyCount = 2;
		const float clipDuration = 1;

		clip1 = new AnimationClip("clip1.clip");
		clip2 = new AnimationClip("clip2.clip");

		std::vector<size_t> tmp(trackCount, keyCount);

		MCD_VERIFY(clip1->init(StrideArray<const size_t>(&tmp[0], trackCount)));
		MCD_VERIFY(clip2->init(StrideArray<const size_t>(&tmp[0], trackCount)));

		clip1->framerate = 30;
		clip2->framerate = 30;

		clip1->length = clip1->framerate * clipDuration;
		clip2->length = clip2->framerate * clipDuration;

		// Clip 1 will move along x while clip2 move along y
		for(size_t i=0; i<trackCount; ++i) {
			AnimationClip::Keys keys = clip1->getKeysForTrack(i);
			for(size_t j=0; j<keys.size; ++j)
				keys[j].pos = float(j) * clip1->framerate;
			clip1->tracks[i].flag = AnimationClip::Linear;
			for(size_t j=0; j<keys.size; ++j)
				keys[j].cast<Vec3f>() = Vec3f(float(j), 0, 1);
		}

		for(size_t i=0; i<trackCount; ++i) {
			AnimationClip::Keys keys = clip2->getKeysForTrack(i);
			for(size_t j=0; j<keys.size; ++j)
				keys[j].pos = float(j) * clip2->framerate;
			clip2->tracks[i].flag = AnimationClip::Linear;
			for(size_t j=0; j<keys.size; ++j)
				keys[j].cast<Vec3f>() = Vec3f(0, float(j), 2);
		}
	}

	AnimationBlendTree tree;
	AnimationClipPtr clip1, clip2;
};	// AnimationBlendTreeTestFixture

typedef AnimationBlendTree::ClipNode ClipNode;
typedef AnimationBlendTree::LerpNode LerpNode;

TEST_FIXTURE(AnimationBlendTreeTestFixture, Basic)
{
	ClipNode* n1 = new ClipNode;
	n1->state.clip = clip1;
	n1->parent = 2;

	ClipNode* n2 = new ClipNode;
	n2->state.clip = clip2;
	n2->parent = 2;

	LerpNode* n3 = new LerpNode;
	n3->t = 0.5f;

	tree.nodes.push_back(n1);
	tree.nodes.push_back(n2);
	tree.nodes.push_back(n3);

	tree.inOrderSort();

	tree.getFinalPose();
}

TEST_FIXTURE(AnimationBlendTreeTestFixture, Xml)
{
	const char* xml = "\
	<lerp name=\"node1\" t=\"0.5\">\
		<clip rate=\"1.2\" src=\"clip1.clip\" />\
		<clip rate=\"0.9\" src=\"clip2.clip\" />\
	</lerp>";

	ResourceManager mgr(*new RawFileSystem(""));

	mgr.cache(clip1);
	mgr.cache(clip2);

	CHECK(tree.loadFromXml(xml, mgr));

	AnimationBlendTree::Pose pose = tree.getFinalPose();
	CHECK_EQUAL(1.5f, pose[0].v.z);
}
