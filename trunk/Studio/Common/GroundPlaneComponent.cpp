#include "stdafx.h"
#include "GroundPlaneComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../3Party/glew/glew.h"

using namespace MCD;

Vec3f projectToUnitViewPort(const Vec3f& p)
{
	GLdouble model[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	GLint viewPort[4] = { 0, 0, 1, 1 };

	GLdouble v[3];
	MCD_VERIFY(gluProject(p[0], p[1], p[2], model, projection, viewPort, &v[0], &v[1], &v[2]) == GL_TRUE);

	return Vec3f(float(v[0]), float(v[1]), float(v[2]));
}

// TODO: The grid line drawing logic is not correct yet.
// An algorithm for determining the best bound and spacing is needed.
void GroundPlaneComponent::render()
{
	Entity* e = entity();
	MCD_ASSUME(e);

	// Get the translation component
	Vec3f trans = e->worldTransform().translation();

	Vec3f p1(trans);
	Vec3f p2(trans + Vec3f::c100);

	// Test a unit lenght vector to see it's projected lenght on the view port.
	p1 = projectToUnitViewPort(p1);
	p2 = projectToUnitViewPort(p2);

	// Use this projected length as a bias to scale our grid
	float len = (p2 - p1).length();
	int cLineCount = int(1.0f / len);
	int factor = int(::log10f(len));
	len *= ::powf(10, -(factor - 1));

//	const float cPixelSizeForUnitLength = 100.0f;
//	e->localTransform.setScale(Vec3f(cPixelSizeForUnitLength / len));

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glDisable(GL_LIGHTING);
	glPushMatrix();
	glMultTransposeMatrixf(entity()->worldTransform().getPtr());
	glScalef(len, len, len);

	glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
	glBegin(GL_LINES);
		const float cInterval = 0.2f;
		for(int i=-cLineCount; i<=cLineCount; ++i) {
			if(i == 0)
				continue;
			glVertex3f(cInterval * i, 0, -cLineCount * cInterval);
			glVertex3f(cInterval * i, 0,  cLineCount * cInterval);
			glVertex3f(-cLineCount * cInterval, 0, cInterval * i);
			glVertex3f( cLineCount * cInterval, 0, cInterval * i);
		}
	glEnd();

	glColor4f(0.1f, 0.1f, 0.1f, 0.5f);
	glLineWidth(1);
	glBegin(GL_LINES);
		glVertex3f(0, 0, -cLineCount * cInterval);
		glVertex3f(0, 0,  cLineCount * cInterval);
		glVertex3f(-cLineCount * cInterval, 0, 0);
		glVertex3f( cLineCount * cInterval, 0, 0);
	glEnd();
	glLineWidth(1);

	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
