#include <stdio.h>
#include <ysclass.h>

#ifdef WIN32
#include <windows.h>
#endif

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
#include <fsguicommondrawing.h>



#include "ysjoyreader.h"
#include "fsjoycalibdlg.h"


void FsCalibrationDialog::ClearScreen(void) const
{
	glClearColor(0.7f,0.7f,0.7f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void FsCalibrationDialog::ShowJoystickAxis(const YsJoyReader &joy,const int axisType[],int axis,int x,int y) const
{
	YsColor gray;
	gray.SetIntRGB(64,64,64);

	switch(axisType[axis])
	{
	case FSJOY_AXISTYPE_STICKORYOKE:
		FsGuiCommonDrawing::DrawRect(x,y,x+299,y+299,YsBlack(),YSTRUE);
		FsGuiCommonDrawing::DrawLine(x+150,y,x+150,y+299,gray);
		FsGuiCommonDrawing::DrawLine(x,y+150,x+299,y+150,gray);
		FsGuiCommonDrawing::DrawRect(x    ,y,x+299,y+299,YsWhite(),YSFALSE);

		{
			double calibX,calibY;
			int cursorX,cursorY;
			calibX=joy.axis[axis].GetCalibratedValue();
			calibY=joy.axis[axis+1].GetCalibratedValue();
			cursorX=x+150+(int)(calibX*150.0);
			cursorY=y+150+(int)(calibY*150.0);

			FsGuiCommonDrawing::DrawLine(cursorX,cursorY-5,cursorX,cursorY+5,YsWhite());
			FsGuiCommonDrawing::DrawLine(cursorX-5,cursorY,cursorX+5,cursorY,YsWhite());
		}

		break;
	case FSJOY_AXISTYPE_QUADRANT:
		glColor3ub(0,0,0);
		glBegin(GL_QUADS);
		glVertex2i(x    ,y);
		glVertex2i(x+29,y);
		glVertex2i(x+29,y+299);
		glVertex2i(x   ,y+299);
		glEnd();

		{
			const double calib=joy.axis[axis].GetCalibratedValue();
			const int level=(int)(calib*150.0);

			glColor3ub(0,0,255);
			glBegin(GL_QUADS);
			glVertex2i(x   ,y+299);
			glVertex2i(x+29,y+299);
			glVertex2i(x+29,y+150+level);
			glVertex2i(x   ,y+150+level);
			glEnd();
		}

		glColor3ub(255,255,255);
		glBegin(GL_LINE_LOOP);
		glVertex2i(x    ,y);
		glVertex2i(x+29,y);
		glVertex2i(x+29,y+299);
		glVertex2i(x    ,y+299);
		glEnd();

		break;
	case FSJOY_AXISTYPE_AXISWITHCENTER:
		glColor3ub(0,0,0);
		glBegin(GL_QUADS);
		glVertex2i(x    ,y);
		glVertex2i(x+29,y);
		glVertex2i(x+29,y+299);
		glVertex2i(x   ,y+299);
		glEnd();

		{
			const double calib=joy.axis[axis].GetCalibratedValue();
			const int level=(int)(calib*150.0);

			glColor3ub(0,0,255);
			glBegin(GL_QUADS);
			glVertex2i(x   ,y+299);
			glVertex2i(x+29,y+299);
			glVertex2i(x+29,y+150+level);
			glVertex2i(x   ,y+150+level);
			glEnd();
		}

		glColor3ub(255,255,255);
		glBegin(GL_LINE_LOOP);
		glVertex2i(x    ,y);
		glVertex2i(x+29,y);
		glVertex2i(x+29,y+299);
		glVertex2i(x    ,y+299);
		glEnd();

		break;
	}
}

void FsCalibrationDialog::ShowHatSwitch(const YsJoyReader &joy,int hatId,int x,int y) const
{
	const double size=60;
	const double pi=3.1415927;
	double cx,cy,rad,ang;

	cx=(double)(x+size/2.0);
	cy=(double)(y+size/2.0);
	rad=(double)(size/2.0);

	glColor3ub(0,0,0);

	glBegin(GL_LINE_LOOP);
	for(ang=0; ang<360; ang+=15)
	{
		double radian,s,c,u,v;
		radian=(double)ang*pi/180.0;

		s=sin(radian);
		c=cos(radian);

		u=cx+c*rad;
		v=cy+s*rad;

		glVertex2d(u,v);
	}
	glEnd();

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

	glBegin(GL_QUADS);
	glVertex2d(cx-4.0,cy);
	glVertex2d(cx    ,cy-4.0);
	glVertex2d(cx+4.0,cy);
	glVertex2d(cx    ,cy+4.0);
	glEnd();
}

