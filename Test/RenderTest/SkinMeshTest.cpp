#include "Pch.h"
#include "../../MCD/Render/Skeleton.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../MCD/Core/Math/Quaternion.h"
#include "../../MCD/Render/Mesh.h"
#include "../../MCD/Render/Renderer.h"
#include "../../MCD/Render/PlaneMeshBuilder.h"
#include "../../MCD/Render/SkinMesh.h"
#include "../../MCD/Framework/Framework.h"

using namespace MCD;

class SkinMeshTestFixture
{
public:
	static const size_t jointCount = 10;
	static const float jointLength;

	SkinMeshTestFixture()
	{
		MCD_VERIFY(framework.initWindow("title=SkinMeshTest;width=800;height=600;fullscreen=0;FSAA=4"));

		createCylinder(2);
		createSkeleton();
		createAnimationClip();
	}

	// Create a cylinder alone the z-axis
	void createCylinder(float radius)
	{
		const size_t segCount = 10;
		const float diameter = radius * Mathf::cPi() * 2;
		const float length = jointCount * jointLength;

		// We make a plan mesh first and then adjust it's vertex to form a cyclinder
		PlaneMeshBuilder builder(diameter, length, segCount, jointCount, false);
		StrideArray<Vec3f> position = builder.getAttributeAs<Vec3f>(builder.posId);
		StrideArray<Vec3f> normal = builder.getAttributeAs<Vec3f>(builder.normalId);

		// Adjust the position and normal
		for(size_t i=0; i<position.size; ++i) {
			// Map the linear range [-diameter/2, diameter/2] into -Pi to Pi degree.
			const float angle = position[i].x / (diameter/2) * Mathf::cPi();
			Mathf::sinCos(angle, normal[i].x, normal[i].y);
			position[i].x = normal[i].x * radius;
			position[i].y = normal[i].y * radius;
			position[i].z += length / 2;
		}

		// Assign the joint index and weight
		// NOTE: All getAttributeAs() should appear after declareAttribute(), otherwise the pointer may become invalid.
		int jointIdxId = builder.declareAttribute(VertexFormat::get("jointIndex"), 2);
		int jointWeightId = builder.declareAttribute(VertexFormat::get("jointWeight"), 2);
		StrideArray<Vec4<uint8_t> > jointIdx = builder.getAttributeAs<Vec4<uint8_t> >(jointIdxId);
		StrideArray<Vec4f> jointWeight = builder.getAttributeAs<Vec4f>(jointWeightId);

		// Re-acquire the position pointer
		position = builder.getAttributeAs<Vec3f>(builder.posId);

		for(size_t i=0; i<jointIdx.size; ++i) {
			// Find the 4 nearest joints
			const int nearestJointIdx = int(position[i].z / jointLength);
			jointIdx[i][0] = uint8_t(Math<int>::clamp(nearestJointIdx - 0, 0, jointCount - 1));
			jointIdx[i][1] = uint8_t(Math<int>::clamp(nearestJointIdx - 1, 0, jointCount - 1));
			jointIdx[i][2] = uint8_t(Math<int>::clamp(nearestJointIdx + 1, 0, jointCount - 1));
			jointIdx[i][3] = uint8_t(Math<int>::clamp(nearestJointIdx + 2, 0, jointCount - 1));

			jointWeight[i][0] = 0.5f;
			jointWeight[i][1] = 0.2f;
			jointWeight[i][2] = 0.2f;
			jointWeight[i][3] = 0.1f;
		}

		basePoseMesh = new Mesh("cylinder");
		MCD_VERIFY(basePoseMesh->create(builder, Mesh::Static));
	}

	Quaternionf randomQuaternion()
	{
		Quaternionf q(Mathf::random() * 2 -1, Mathf::random() * 2 -1, Mathf::random() * 2 -1, Mathf::random() * 2 -1);
		q /= q.length();
		return q;
	}

	void createSkeleton()
	{
		skeleton = new Skeleton("");
		skeleton->init(jointCount);

		skeleton->parents[0] = 0;
		for(size_t i=1; i<jointCount; ++i)
			skeleton->parents[i] = i-1;

		for(size_t i=0; i<jointCount; ++i)
			skeleton->basePose[i] = Mat44f::makeTranslation(Vec3f(0, 0, i * (float)jointLength));

		skeleton->initBasePoseInverse();
	}

	void createAnimationClip()
	{
		clip = new AnimationClip("");

		static const size_t sampleCount = 3;
		static const size_t trackCount = jointCount * 2;

		// Create animation clip
		// Number of tracks = number of joint * attribute count (which is 2 because of translation and rotation)
		std::vector<size_t> tmp(trackCount, sampleCount);
		MCD_VERIFY(clip->init(StrideArray<const size_t>(&tmp[0], trackCount)));
		clip->length = sampleCount * clip->framerate;

		// Setting up the transform for each joint relative to it's parent joint.
		for(size_t i=0; i<clip->trackCount(); ++i) {
			AnimationClip::Keys keys = clip->getKeysForTrack(i);

			// Setup frame position
			for(size_t j=0; j<keys.size; ++j)
				keys[j].pos = float(j) * clip->framerate;

			// Setup translation
			if(i % 2 == 0) {
				clip->tracks[i].flag = AnimationClip::Linear;
				for(size_t j=0; j<keys.size; ++j)
					keys[j].cast<Vec3f>() = Vec3f(0, 0, (float)jointLength);
			}
			// Setup rotation
			else {
				clip->tracks[i].flag = AnimationClip::Slerp;
				for(size_t j=0; j<keys.size; ++j)
					keys[j].cast<Quaternionf>().fromAxisAngle(Vec3f::c100, Mathf::cPi() * j / jointCount);	// Rotate around x-axis anti-clockwise for each key frame
			}
		}
	}

	Entity* createEntity()
	{
		Entity* e = new Entity("");

		SimpleAnimationComponent* anim = e->addComponent(new SimpleAnimationComponent);
		anim->animations.resize(1);
		anim->animations[0].clip = clip;

		SkeletonPose* pose = e->addComponent(new SkeletonPose);
		pose->animation = anim;
		pose->skeleton = skeleton;

		SkinMesh* mesh = e->addComponent(new SkinMesh);
		mesh->basePoseMesh = basePoseMesh;
		mesh->pose = pose;
		return e;
	}

	Framework framework;
	MeshPtr basePoseMesh;
	SkeletonPtr skeleton;
	AnimationClipPtr clip;
};	// SkinMeshTestFixture

const float SkinMeshTestFixture::jointLength = 1.0f;

TEST_FIXTURE(SkinMeshTestFixture, Render)
{
	Entity& root = framework.rootEntity();
	RendererComponent* renderer = root.findComponentInChildrenExactType<RendererComponent>();

	framework.sceneLayer().addFirstChild(createEntity());

	while(true)
	{
		Event e;
		framework.update(e);

		if(e.Type == Event::Closed)
			break;

		renderer->render(root);
	}
}
