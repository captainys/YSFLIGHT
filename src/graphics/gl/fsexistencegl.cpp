#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fs.h"

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include "fsrecord.h"
#include "graphics/common/fsopengl.h"

#include <ysbitmap.h>



//#define YSOGLERRORCHECk
//extern void FsOpenGlShowError(const char tag[]);


void FsAirplane::DrawVapor(double currentTime,double remainTime,int step,YSBOOL transparency) const
{
#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawVapor In");
#endif

	YsGLVertexBuffer vtxBuf;
	YsGLColorBuffer colBuf;
	this->MakeVaporVertexArray(vtxBuf,colBuf,currentTime,remainTime,step);

	glDisable(GL_LIGHTING);
	glColor3d(1.0,1.0,1.0);
	glBegin(GL_LINES);

	for(YSSIZE_T idx=0; idx<vtxBuf.size(); ++idx)
	{
		glColor4fv(colBuf[idx]);
		glVertex3fv(vtxBuf[idx]);
	}

	glEnd();


#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawVapor Out");
#endif
}

void FsAirplane::DrawSingleSmoke(int smkId,double currentTime,double remainTime,FSSMOKETYPE smk,int step,YSBOOL transparency) const
{
#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawSmoke In");
#endif
	YsGLVertexBuffer vtxBuf;
	YsGLNormalBuffer nomBuf;
	YsGLColorBuffer colBuf;
	AddSingleSmokeVertexArray(vtxBuf,nomBuf,colBuf,smkId,currentTime,remainTime,smk,step);

	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);

	glBegin(GL_TRIANGLES);
	for(YSSIZE_T idx=0; idx<vtxBuf.size(); ++idx)
	{
		glColor4fv(colBuf[idx]);
		glNormal3fv(nomBuf[idx]);
		glVertex3fv(vtxBuf[idx]);
	}
	glEnd();

	glDisable(GL_CULL_FACE);

#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawSmoke Out");
#endif
}

void FsField::DrawMapVisual
    (FSENVIRONMENT env,
     const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,const double &elvMin,const double &elvMax,YSBOOL drawPset,
     const double &currentTime,
     YSBOOL /*useOpenGlGroundTexture*/,YSBOOL /*useOpenGlRunwayLightTexture*/) const
{
#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsField::DrawMapVisual In");
#endif


	int wid,hei;
	float pointSize;
	FsGetWindowSize(wid,hei);


	if(env==FSNIGHT)
	{
		YsScenery::lightPointSize3d=0.4f;

		pointSize=(float)wid/1024.0F;  // 1024 pixel -> 2 dots
		pointSize=YsBound(pointSize,1.0F,2.0F);
		glPointSize(pointSize);
	}
	else
	{
		YsScenery::lightPointSize3d=0.1f;

		glPointSize(1.0f);
	}

	if(fld!=NULL)
	{
		YsMatrix4x4 viewTfm;
		viewTfm.RotateXY(-viewAtt.b());
		viewTfm.RotateZY(-viewAtt.p());
		viewTfm.RotateXZ(-viewAtt.h());
		viewTfm.Translate(-viewPos);

		fld->pos=pos;
		fld->att=att;
		fld->DrawMapVisual(viewTfm,YsIdentity4x4(),projMat,elvMin,elvMax,drawPset,currentTime);
	}

	glPointSize(1);

#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsField::DrawMapVisual Out");
#endif
}

