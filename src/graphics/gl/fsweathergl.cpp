#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include "fs.h"

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif



void FsWeather::DrawCloudLayer(const YsVec3 &cameraPos) const
{
	int i;

	glPushMatrix();

	glEnable(GL_LIGHTING);

	glTranslated(cameraPos.x(),0.0,cameraPos.z());
	glColor3d(0.9,0.9,0.9);

	glBegin(GL_QUADS);
	forYsArray(i,cloudLayer)
	{
		glNormal3d(0.0,-1.0,0.0);
		glVertex3d(-20000.0,cloudLayer[i].y0,-20000.0);
		glVertex3d(-20000.0,cloudLayer[i].y0, 20000.0);
		glVertex3d( 20000.0,cloudLayer[i].y0, 20000.0);
		glVertex3d( 20000.0,cloudLayer[i].y0,-20000.0);

		glNormal3d(0.0,1.0,0.0);
		glVertex3d(-20000.0,cloudLayer[i].y1,-20000.0);
		glVertex3d(-20000.0,cloudLayer[i].y1, 20000.0);
		glVertex3d( 20000.0,cloudLayer[i].y1, 20000.0);
		glVertex3d( 20000.0,cloudLayer[i].y1,-20000.0);
	}
	glEnd();

	glPopMatrix();
}



