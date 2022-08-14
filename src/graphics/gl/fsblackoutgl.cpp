#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fsconfig.h"

#include "fs.h"
#include "graphics/common/fsopengl.h"

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


void FsSimulation::SimDrawBlackout(const ActualViewMode &actualViewMode) const
{
	const FsAirplane *playerPlane;

	playerPlane=GetPlayerAirplane();
	int i,j;
	const double plusGLimit=9.0;
	const double minusGLimit=-5.0;

	if(cfgPtr->blackOut==YSTRUE &&
	   actualViewMode.actualViewMode==FSCOCKPITVIEW &&
	   playerPlane!=NULL &&
	   playerPlane->isPlayingRecord!=YSTRUE &&
	   playerPlane->IsAlive()==YSTRUE &&
	   (playerPlane->Prop().GetG()>plusGLimit ||
	    playerPlane->Prop().GetG()<minusGLimit))
	{
		int vp[4],cenX,cenY,rad;
		double blackness,r,g,b;

		glDisable(GL_LIGHTING);

		if(playerPlane->Prop().GetG()>0.0)
		{
			blackness=(playerPlane->Prop().GetG()-plusGLimit)/6.0;
			r=0.0;
			g=0.0;
			b=0.0;
		}
		else
		{
			blackness=(minusGLimit-playerPlane->Prop().GetG())/6.0;
			r=1.0;
			g=0.0;
			b=0.0;
		}

		glGetIntegerv(GL_VIEWPORT,vp);
		cenX=vp[2]/2;
		cenY=vp[3]/2;
		rad=(int)sqrt((double)(vp[2]*vp[2]+vp[3]*vp[3]));

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0,double(vp[2]),double(vp[3]),0.0,-1.0,1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_DEPTH_TEST);
		glDepthMask(0);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		double circle[]=
		{
			 1.0       , 0.0,
			 1.4142/2.0, 1.4142/2.0,
			 0.0       , 1.0,
			-1.4142/2.0, 1.4142/2.0,
			-1.0       , 0.0,
			-1.4142/2.0,-1.4142/2.0,
			 0.0       ,-1.0,
			 1.4142/2.0,-1.4142/2.0,
			 1.0       , 0.0
		};

		glBegin(GL_QUADS);
		for(i=0; i<5; i++)  // i==0:Inner circle    smaller i->lighter black
		{
			double alpha[2];
			double radius[2];
			alpha[0]=blackness*YsSqr((double)(1+ i   /5.0));
			alpha[1]=blackness*YsSqr((double)(1+(i+1)/5.0));
			radius[0]=(double)(i*rad/5);
			radius[1]=(double)((i+1)*rad/5);

			alpha[0]=YsBound(alpha[0],0.0,1.0);
			alpha[1]=YsBound(alpha[1],0.0,1.0);

			for(j=0; j<8; j++)
			{
				glColor4d(r,g,b,alpha[0]);
				glVertex2d(cenX+circle[ j   *2]*radius[0],cenY+circle[ j   *2+1]*radius[0]);
				glVertex2d(cenX+circle[(j+1)*2]*radius[0],cenY+circle[(j+1)*2+1]*radius[0]);
				glColor4d(r,g,b,alpha[1]);
				glVertex2d(cenX+circle[(j+1)*2]*radius[1],cenY+circle[(j+1)*2+1]*radius[1]);
				glVertex2d(cenX+circle[ j   *2]*radius[1],cenY+circle[ j   *2+1]*radius[1]);
			}
		}
		glEnd();

		//glDisable(GL_BLEND);
		glDepthMask(~0);
		glEnable(GL_DEPTH_TEST);

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		YsString str;
		str.Printf("%.1lfG",playerPlane->Prop().GetG());

		int wid,hei;
		FsGetWindowSize(wid,hei);
		FsDrawString(wid/2-20,hei/2,str,YsRed());
	}
}

