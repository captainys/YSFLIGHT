#include <stdio.h>
#include <ysclass.h>

#include <ysgl.h>

#include <ysbitmap.h>

#ifndef YSFLIGHT
	#include <fssimplewindow.h>
#else
	#include "fsdef.h"
	#include "fswindow.h"
	#define FSSIMPLEWINDOW_MACRO_ONLY
	#include <fssimplewindow.h>
	#undef FSSIMPLEWINDOW_MACRO_ONLY
#endif


#include <fsgui.h>


#include "ysjoyreader.h"
#include "fsjoycalibdlg.h"


void FsCalibrationDialog::ClearScreen(void) const
{
	glClearColor(0.7f,0.7f,0.7f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void FsCalibrationDialog::ShowJoystickAxis(const YsJoyReader &joy,const int axisType[],int axis,int x,int y) const
{
	const GLfloat black[16]=
	{
		0.0f,0.0f,0.0f,1.0f,
		0.0f,0.0f,0.0f,1.0f,
		0.0f,0.0f,0.0f,1.0f,
		0.0f,0.0f,0.0f,1.0f
	};
	const GLfloat white[16]=
	{
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f
	};
	const GLfloat blue[16]=
	{
		0.0f,0.0f,1.0f,1.0f,
		0.0f,0.0f,1.0f,1.0f,
		0.0f,0.0f,1.0f,1.0f,
		0.0f,0.0f,1.0f,1.0f
	};


	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);

	switch(axisType[axis])
	{
	case FSJOY_AXISTYPE_STICKORYOKE:
		{
			const GLfloat vtx[8]=
			{
				(GLfloat)(x    ),(GLfloat)(y),
				(GLfloat)(x+299),(GLfloat)(y),
				(GLfloat)(x+299),(GLfloat)(y+299),
				(GLfloat)(x    ),(GLfloat)(y+299)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,black);
		}
		{
			const GLfloat vtx[8]=
			{
				(GLfloat)(x+150),(GLfloat)(y),
				(GLfloat)(x+150),(GLfloat)(y+299),
				(GLfloat)(x    ),(GLfloat)(y+150),
				(GLfloat)(x+299),(GLfloat)(y+150)
			};
			const GLfloat col[16]=
			{
				0.25f,0.25f,0.25f,1.0f,
				0.25f,0.25f,0.25f,1.0f,
				0.25f,0.25f,0.25f,1.0f,
				0.25f,0.25f,0.25f,1.0f
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINES,4,vtx,col);
		}
		{
			const GLfloat vtx[8]=
			{
				(GLfloat)(x    ),(GLfloat)(y),
				(GLfloat)(x+299),(GLfloat)(y),
				(GLfloat)(x+299),(GLfloat)(y+299),
				(GLfloat)(x    ),(GLfloat)(y+299)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINE_LOOP,4,vtx,white);
		}

		{
			double calibX,calibY;
			int cursorX,cursorY;
			calibX=joy.axis[axis].GetCalibratedValue();
			calibY=joy.axis[axis+1].GetCalibratedValue();
			cursorX=x+150+(int)(calibX*150.0);
			cursorY=y+150+(int)(calibY*150.0);

			const GLfloat vtx[8]=
			{
				(GLfloat)(cursorX  ),(GLfloat)(cursorY-5),
				(GLfloat)(cursorX  ),(GLfloat)(cursorY+5),
				(GLfloat)(cursorX-5),(GLfloat)(cursorY),
				(GLfloat)(cursorX+5),(GLfloat)(cursorY)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINES,4,vtx,white);
		}
		break;
	case FSJOY_AXISTYPE_QUADRANT:
		{
			const GLfloat vtx[8]=
			{
				(GLfloat)(x   ),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y+299),
				(GLfloat)(x   ),(GLfloat)(y+299)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,black);
		}
		{
			const double calib=joy.axis[axis].GetCalibratedValue();
			const int level=(int)(calib*150.0);

			const GLfloat vtx[8]=
			{
				(GLfloat)(x   ),(GLfloat)(y+299),
				(GLfloat)(x+29),(GLfloat)(y+299),
				(GLfloat)(x+29),(GLfloat)(y+150+level),
				(GLfloat)(x   ),(GLfloat)(y+150+level)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,blue);
		}
		{
			const GLfloat vtx[8]=
			{
				(GLfloat)(x   ),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y+299),
				(GLfloat)(x   ),(GLfloat)(y+299)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,white);
		}
		break;
	case FSJOY_AXISTYPE_AXISWITHCENTER:
		{
			const GLfloat vtx[8]=
			{
				(GLfloat)(x   ),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y+299),
				(GLfloat)(x   ),(GLfloat)(y+299)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,black);
		}
		{
			const double calib=joy.axis[axis].GetCalibratedValue();
			const int level=(int)(calib*150.0);

			const GLfloat vtx[8]=
			{
				(GLfloat)(x   ),(GLfloat)(y+299),
				(GLfloat)(x+29),(GLfloat)(y+299),
				(GLfloat)(x+29),(GLfloat)(y+150+level),
				(GLfloat)(x   ),(GLfloat)(y+150+level)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,blue);
		}
		{
			const GLfloat vtx[8]=
			{
				(GLfloat)(x   ),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y),
				(GLfloat)(x+29),(GLfloat)(y+299),
				(GLfloat)(x   ),(GLfloat)(y+299)
			};
			YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINE_LOOP,4,vtx,white);
		}
		break;
	}
	YsGLSLEndUsePlain2DRenderer(renderer);
}

void FsCalibrationDialog::ShowHatSwitch(const YsJoyReader &joy,int hatId,int x,int y) const
{
	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);


	const double size=60;
	const double pi=3.1415927;
	double cx,cy,rad,ang;

	cx=(double)(x+size/2.0);
	cy=(double)(y+size/2.0);
	rad=(double)(size/2.0);

	int nVtx=0;
	GLfloat vtx[24*2];
	GLfloat col[24*4];

	for(ang=0; ang<360; ang+=15)
	{
		double radian,s,c,u,v;
		radian=(double)ang*pi/180.0;

		s=sin(radian);
		c=cos(radian);

		u=cx+c*rad;
		v=cy+s*rad;

		vtx[nVtx*2  ]=(GLfloat)u;
		vtx[nVtx*2+1]=(GLfloat)v;
		col[nVtx*4  ]=0.0f;
		col[nVtx*4+1]=0.0f;
		col[nVtx*4+2]=0.0f;
		col[nVtx*4+3]=1.0f;
		++nVtx;
	}
	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINE_LOOP,nVtx,vtx,col);

	int value;
	value=joy.hatSwitch[hatId].GetDiscreteValue();
	if(value!=0)
	{
		double radian,s,c;
		radian=(double)(value-1)*45.0*pi/180.0;
		radian=pi/2.0-radian;

		s=sin(radian);
		c=cos(radian);

		cx=cx+c*rad;
		cy=cy-s*rad;
	}

	const GLfloat quadVtx[8]=
	{
		(GLfloat)(cx-4.0),(GLfloat)(cy),
		(GLfloat)(cx    ),(GLfloat)(cy-4.0),
		(GLfloat)(cx+4.0),(GLfloat)(cy),
		(GLfloat)(cx    ),(GLfloat)(cy+4.0)
	};
	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,quadVtx,col);

	YsGLSLEndUsePlain2DRenderer(renderer);
}

