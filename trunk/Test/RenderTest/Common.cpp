#include "Pch.h"
#include "Common.h"
#include "../../MCD/Render/Frustum.h"
#include "../../3Party/glew/glew.h"

namespace MCD {

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

}	// namespace MCD
