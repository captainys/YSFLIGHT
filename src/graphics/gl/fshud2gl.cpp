#include <ysclass.h>
#include "ysunitconv.h"

#include "graphics/common/fsopengl.h"
#include "fshud2.h"
#include "fsinstreading.h"

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif



#include "fswirefont.h"


void FsHud2::TakeSpecificResource(void)
{
}

void FsHud2::ReleaseSpecificResource(void)
{
}



void FsHud2::BeginDrawHud(const YsVec3 &viewPos,const YsAtt3 &airAtt)
{
	pointVtxBuf.CleanUp();
	pointColBuf.CleanUp();
	lineVtxBuf.CleanUp();
	lineColBuf.CleanUp();
	triVtxBuf.CleanUp();
	triColBuf.CleanUp();


	glDisable(GL_LIGHTING);
	glColor4ub(hudCol.Ruc(),hudCol.Guc(),hudCol.Buc(),255);

	glDisable(GL_CULL_FACE);  // 2010/12/19 Does this solve missing flaps, throttle, spoiler, etc.?

	glPushMatrix();

	this->viewPos=viewPos;
	this->airAtt=airAtt;

	YsMatrix3x3 airRot;
	double airRotOpenGL[16];
	airRot.Rotate(airAtt);
	airRot.GetOpenGlCompatibleMatrix(airRotOpenGL);

	// Translate
	glTranslated(viewPos.x(),viewPos.y(),viewPos.z());

	// Rotate
	glMultMatrixd(airRotOpenGL);

	glDepthFunc(GL_ALWAYS);
}

void FsHud2::EndDrawHud(void)
{
	glBegin(GL_TRIANGLES);
	for(int i=0; i<triVtxBuf.GetN(); ++i)
	{
		glColor4f(triColBuf[i][0],triColBuf[i][1],triColBuf[i][2],triColBuf[i][3]);
		glVertex3f(triVtxBuf[i][0],triVtxBuf[i][1],triVtxBuf[i][2]);
	}
	glEnd();

	glBegin(GL_LINES);
	for(int i=0; i<lineVtxBuf.GetN(); ++i)
	{
		glColor4f(lineColBuf[i][0],lineColBuf[i][1],lineColBuf[i][2],lineColBuf[i][3]);
		glVertex3f(lineVtxBuf[i][0],lineVtxBuf[i][1],lineVtxBuf[i][2]);
	}
	glEnd();

	glPopMatrix();
}

