#include <ysclass.h>

#include "ysunitconv.h"

#include "fs.h"
#include "graphics/common/fsopengl.h"

#include "fsinstpanel.h"
#include "fswirefont.h"


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





////////////////////////////////////////////////////////////

void FsInstrumentPanel::DrawCrossHair3d(void)  // This function is called from BeginDraw3d
{
	glColor3ub(255,255,255);

	glBegin(GL_LINES);
	glVertex3d(-0.01,0.0,0.5);
	glVertex3d(-0.03,0.0,0.5);
	glVertex3d( 0.01,0.0,0.5);
	glVertex3d( 0.03,0.0,0.5);

	glVertex3d(0.0,-0.01,0.5);
	glVertex3d(0.0,-0.03,0.5);
	glVertex3d(0.0, 0.01,0.5);
	glVertex3d(0.0, 0.03,0.5);
	glEnd();
}

void FsInstrumentPanel::EndDraw3d(void)
{
	auto viewPos=viewPosCache;
	auto localViewPos=localViewPosCache;
	auto &prop=*airPropCache;

	glDisable(GL_LIGHTING);

	glDepthFunc(GL_ALWAYS);
	glPushMatrix();


	glTranslated(viewPos.x(),viewPos.y(),viewPos.z());


	YsMatrix3x3 airRot;
	double airRotOpenGL[16];
	airRot.Rotate(prop.GetAttitude());
	airRot.GetOpenGlCompatibleMatrix(airRotOpenGL);

	glMultMatrixd(airRotOpenGL);


	if(drawCrossHair==YSTRUE)
	{
		DrawCrossHair3d();
	}


	YsVec3 instPanelOffset;
	prop.GetInstPanelPos(instPanelOffset);
	instPanelOffset-=localViewPos;

	glTranslated(instPanelOffset.x(),instPanelOffset.y(),instPanelOffset.z());
	glScaled(prop.GetInstPanelScaling(),prop.GetInstPanelScaling(),prop.GetInstPanelScaling());


	const YsAtt3 &instAtt=prop.GetInstPanelAtt();
	glRotated(YsRadToDeg( instAtt.h()),0.0,1.0,0.0);
	glRotated(YsRadToDeg(-instAtt.p()),1.0,0.0,0.0);
	glRotated(YsRadToDeg( instAtt.b()),0.0,0.0,1.0);


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

	glBegin(GL_TRIANGLES);
	for(int i=0; i<ovTriVtxBuf.GetN(); ++i)
	{
		glColor4f(ovTriColBuf[i][0],ovTriColBuf[i][1],ovTriColBuf[i][2],ovTriColBuf[i][3]);
		glVertex3f(ovTriVtxBuf[i][0],ovTriVtxBuf[i][1],ovTriVtxBuf[i][2]);
	}
	glEnd();

	glBegin(GL_LINES);
	for(int i=0; i<ovLineVtxBuf.GetN(); ++i)
	{
		glColor4f(ovLineColBuf[i][0],ovLineColBuf[i][1],ovLineColBuf[i][2],ovLineColBuf[i][3]);
		glVertex3f(ovLineVtxBuf[i][0],ovLineVtxBuf[i][1],ovLineVtxBuf[i][2]);
	}
	glEnd();

	glPopMatrix();
	glDepthFunc(GL_LEQUAL);
}

