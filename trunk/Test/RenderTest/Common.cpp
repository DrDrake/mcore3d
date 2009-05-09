#include "Pch.h"
#include "Common.h"
#include "../../MCD/Render/Frustum.h"
#include "../../3Party/glew/glew.h"
#include <stdlib.h> // For rand

using namespace MCD;

void drawViewportQuad(size_t x, size_t y, size_t width, size_t height, int textureType)
{
	// How to draw fullscreen quad:
	// Reference: http://www.opengl.org/resources/faq/technical/transformations.htm

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glViewport(x, y, width, height);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_MULTISAMPLE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	glLoadIdentity();

	float w = textureType == GL_TEXTURE_2D ? 1 : float(width);
	float h = textureType == GL_TEXTURE_2D ? 1 : float(height);
	const float tex[4][2] = {
		{0, 0}, {w, 0}, {w, h}, {0, h}
	};

	glBegin(GL_QUADS);
		glTexCoord2fv(tex[0]);	glVertex3i(-1, -1, -1);
		glTexCoord2fv(tex[1]);	glVertex3i(1, -1, -1);
		glTexCoord2fv(tex[2]);	glVertex3i(1, 1, -1);
		glTexCoord2fv(tex[3]);	glVertex3i(-1, 1, -1);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void drawUnitCube(float textureCoord1, float textureCoord2)
{
	// Face vertice are specified in counterclockwise direction
	static const float vertice[6][4][3] = {
		{ {-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1} },	// Front face	(fix z at 1)
		{ { 1, 1,-1}, { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1} },	// Back face	(fix z at -1)
		{ { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1}, { 1, 1,-1} },	// Right face	(fix x at 1)
		{ {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1} },	// Left face	(fix x at -1)
		{ {-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}, { 1, 1,-1} },	// Top face		(fix y at 1)
		{ { 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1} },	// Bottom face	(fix y at -1)
	};

	const float w = textureCoord1;
	const float h = textureCoord2;
	const float tex[4][2] = {
		{0, h}, {0, 0}, {w, 0}, {w, h}
	};

	glBegin(GL_QUADS);
	for(size_t face=0; face<6; ++face) {
		for(size_t vertex=0; vertex<4; ++vertex) {
			glTexCoord2fv(tex[vertex]);
			glVertex3fv(vertice[face][vertex]);
		}
	}
	glEnd();
}

void drawFrustum(const Frustum& frustum)
{
	// Calculate the vertex of the frustum.
	Vec3f vertex[8];
	frustum.computeVertex(vertex);

	glBegin(GL_LINE_LOOP);
		glVertex3fv(vertex[0 + 0].getPtr());
		glVertex3fv(vertex[1 + 0].getPtr());
		glVertex3fv(vertex[2 + 0].getPtr());
		glVertex3fv(vertex[3 + 0].getPtr());
		glVertex3fv(vertex[3 + 4].getPtr());
		glVertex3fv(vertex[2 + 4].getPtr());
		glVertex3fv(vertex[1 + 4].getPtr());
		glVertex3fv(vertex[0 + 4].getPtr());
		glVertex3fv(vertex[0 + 0].getPtr());
		glVertex3fv(vertex[1 + 0].getPtr());
		glVertex3fv(vertex[1 + 4].getPtr());
		glVertex3fv(vertex[0 + 4].getPtr());
		glVertex3fv(vertex[3 + 4].getPtr());
		glVertex3fv(vertex[2 + 4].getPtr());
		glVertex3fv(vertex[2 + 0].getPtr());
		glVertex3fv(vertex[3 + 0].getPtr());
	glEnd();
}

void minizeEnergy(float* data, size_t sampleCount, size_t stride)
{
	stride /= sizeof(float);
	int iter = 100;
	while(iter--)
	{
		for(size_t i=0; i<sampleCount; ++i)
		{
			Vec3f force;
			Vec3f res(0.0f);
			Vec3f vec;
			float fac;

			Vec3f& sampleI = reinterpret_cast<Vec3f&>(data[i*stride]);
			sampleI.normalize();
			vec = sampleI;
			// Minimize with other samples
			for(size_t j=0; j<sampleCount; ++j)
			{
				Vec3f& sampleJ = reinterpret_cast<Vec3f&>(data[j*stride]);
				force = vec - sampleJ;
				if((fac = force.norm())!= 0.0f )
					res += force / fac;
			}

			sampleI += res * 0.5f;
			sampleI.normalize();
		}
	}

	for(size_t i=0; i<sampleCount; ++i)
	{
		Vec3f& sample = reinterpret_cast<Vec3f&>(data[i*stride]);
		sample *= float(rand()) / RAND_MAX;
	}
}

#include "DefaultResourceManager.h"
#include "../../MCD/Core/System/Path.h"
#include "../../MCD/Core/System/ResourceManager.h"
#include "../../MCD/Render/Shader.h"
#include "../../MCD/Render/ShaderProgram.h"

bool loadShaderProgram(
	const wchar_t* vsSource, const wchar_t* psSource,
	ShaderProgram& shaderProgram,
	DefaultResourceManager& resourceManager)
{
	// Load the shaders synchronously
	ShaderPtr vs = dynamic_cast<Shader*>(resourceManager.load(vsSource, true).get());
	ShaderPtr ps = dynamic_cast<Shader*>(resourceManager.load(psSource, true).get());

	while(true) {
		int result = resourceManager.processLoadingEvents();
		if(result < 0)
			return false;
		else if(result == 0)
			break;
	}

	if(!vs || !ps)
		return false;

	shaderProgram.create();
	shaderProgram.attach(*vs);
	shaderProgram.attach(*ps);
	if(!shaderProgram.link()) {
		std::string log;
		shaderProgram.getLog(log);
		std::cout << log << std::endl;
		return false;
	}

	return true;
}
