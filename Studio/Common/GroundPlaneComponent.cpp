#include "stdafx.h"
#include "GroundPlaneComponent.h"
#include "../../MCD/Core/Entity/Entity.h"
#include "../../3Party/glew/glew.h"

void GroundPlaneComponent::render()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);

	glDisable(GL_LIGHTING);
	glPushMatrix();
	glMultTransposeMatrixf(entity()->worldTransform().getPtr());

	glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
	glBegin(GL_LINES);
		const float cInterval = 0.2f;
		const int cLineCount = 10;
		for(int i=-cLineCount; i<=cLineCount; ++i) {
			if(i == 0)
				continue;
			glVertex3f(cInterval * i, 0, -cLineCount * cInterval);
			glVertex3f(cInterval * i, 0,  cLineCount * cInterval);
			glVertex3f(-cLineCount * cInterval, 0, cInterval * i);
			glVertex3f( cLineCount * cInterval, 0, cInterval * i);
		}
	glEnd();

	glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
	glLineWidth(2);
	glBegin(GL_LINES);
		glVertex3f(0, 0, -11 * cInterval);
		glVertex3f(0, 0,  11 * cInterval);
		glVertex3f(-11 * cInterval, 0, 0);
		glVertex3f( 11 * cInterval, 0, 0);
	glEnd();
	glLineWidth(1);

	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}
