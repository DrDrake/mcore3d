#include "Pch.h"
#include "InstancedMesh.h"
#include "Effect.h"
#include "Material.h"
#include "Mesh.h"
#include "../Core/Math/Mat44.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

class InstancedMesh::Impl
{
public:
	Impl()
	{
		// Generate and allocate the uniform buffer
		const int MAX_BUFFER_SIZE = 65536;	// TODO: Is there any other way?
		glGenBuffers(1, &mUniformBufferHandle);
		glBindBuffer(GL_UNIFORM_BUFFER_EXT, mUniformBufferHandle);

		// Allocate more than we need... Since the number of instances varies during runtime,
		// and the cost of allocating a single bindable uniform is in fact constant
		glBufferData(GL_UNIFORM_BUFFER_EXT, MAX_BUFFER_SIZE, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER_EXT, 0);
	}

	~Impl() {
		glDeleteBuffers(1, &mUniformBufferHandle);
	}

	void update(const Mat44f& viewMat, Mesh& mesh, const Effect& effect)
	{
		{	// Write the per-instance data into the uniform buffer
			mTempBuffer.resize(mPerInstanceInfo.size());

			for(size_t i = 0; i < mPerInstanceInfo.size(); ++i)
				mTempBuffer[i] = (viewMat * mPerInstanceInfo[i]).transpose();

			glBindBuffer(GL_UNIFORM_BUFFER_EXT, mUniformBufferHandle);
			glBufferSubData(GL_UNIFORM_BUFFER_EXT, 0, mTempBuffer.size() * sizeof(Mat44f), &mTempBuffer[0]);
			glBindBuffer(GL_UNIFORM_BUFFER_EXT, 0);
		}

		{	// Bind the uniform buffer to shader, then draw the mesh
			Material* material = nullptr;
			if((material = effect.material.get()) != nullptr)
			{
				for(size_t i=0; i<material->getPassCount(); ++i)
				{
					material->preRender(i);

					{	// Bind the per-instance uniform buffer to the shader
						Material::Pass& pass = material->mRenderPasses[i];
						for(size_t propIdx = 0; propIdx < pass.mProperty.size(); ++propIdx)
						{
							IMaterialProperty& prop = pass.mProperty[propIdx];

							if(ShaderProperty* sp = dynamic_cast<ShaderProperty*>(&prop))
							{
								uint program = sp->shaderProgram->handle;
								uint location = glGetUniformLocation(program, "transformArray");
								glUniformBufferEXT(program, location, mUniformBufferHandle);
							}
						}
					}

					{	// Issue a single draw call to draw the meshes
						mesh.bind(Mesh::Index);
						glEnableClientState(GL_VERTEX_ARRAY);
						mesh.bind(Mesh::Position);

						glDrawElementsInstancedEXT(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_SHORT, 0, mPerInstanceInfo.size());

						glDisableClientState(GL_VERTEX_ARRAY);
					}

					material->postRender(i);
				}
			}
		}

		mPerInstanceInfo.clear();
	}

	GLuint mUniformBufferHandle;
	typedef std::vector<Mat44f> PerInstanceInfo;
	PerInstanceInfo mPerInstanceInfo;
	PerInstanceInfo mTempBuffer;
};	// Impl

InstancedMesh::InstancedMesh(const MeshPtr& mesh, const EffectPtr& effect)
	: mesh(mesh), effect(effect), mImpl(*new Impl)
{
}

InstancedMesh::~InstancedMesh()
{
	delete &mImpl;
}

void InstancedMesh::update(const Mat44f& viewMat)
{
	if(mesh && effect)
		mImpl.update(viewMat, *mesh, *effect);
}

void InstancedMesh::registerPerInstanceInfo(const Mat44f& info)
{
	mImpl.mPerInstanceInfo.push_back(info);
}

}	// namespace MCD
