#include <ysclass.h>
#include <ysport.h>

#include <fssimplewindow.h>

#include <fsgui.h>

#include <fsdef.h>
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"
#include "fsoption.h"
#include "fsconfig.h"

#include "../../gui/fsmenu.h" // It is dirty, but FsMessageBox calls FsKeyMessageBox function.
#include "fscontrol.h"

#include <fsfilename.h>

#include <ysglfontdata.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>

#include <unistd.h>

#include <ysjoyreader.h>



static YSRESULT FsOpenWindow(int x0,int y0,int wid,int hei,const char fontName[],int fontHeight);



void FsSetTopMostWindow(YSBOOL sw)
{
}

YSRESULT FsTaskBarAddIcon(void)
{
  return YSOK;
}

YSRESULT FsTaskBarDeleteIcon(void)
{
  return YSOK;
}

void FsBeforeOpenWindow(const class FsOption &opt,const class FsFlightConfig &)
{
}

void FsAfterOpenWindow(const class FsOption &opt,const class FsFlightConfig &)
{
	if(1==opt.scrnMode)
	{
		FsMaximizeWindow();
	}
	else if(1<opt.scrnMode)
	{
		FsSetFullScreen(0,0,0);
	}
}

void FsSetOnPaintCallback(class FsOnPaintCallback *callback)
{
}

void FsHidePartOfScreenForSharewareMessage(void)
{
}

void FsMessageBox(const char msg[],const char title[])
{
	FsKeyMessageBox(title,msg,"OK",NULL);

//	double nextDrawTime;
//	nextDrawTime=0.0;
//
//	FsPollDevice();
//	while(FsInkey()!=FSKEY_NULL)
//	{
//		FsPollDevice();
//	}
//
//	for(;;)
//	{
//		FsPollDevice();
//		if(FsInkey()!=FSKEY_NULL)
//		{
//			break;
//		}
//
//		if(nextDrawTime<YsTolerance)
//		{
//			int y,l;
//			const char *ptr;
//			char str[256];
//
//			FsClearScreenAndZBuffer(YsBlue());
//			FsSet2DDrawing();
//
//			y=32;
//			l=0;
//			ptr=msg;
//			while((*ptr)!=0)
//			{
//			    if((*ptr)!=0x0a)
//			    {
//			        str[l++]=*ptr;
//			    }
//				ptr++;
//				if((*ptr)=='\n' || (*ptr)==0 || l==254)
//				{
//					str[l]=0;
//					FsDrawString(0,y,str,YsWhite());
//					l=0;
//					y+=20;
//
//				}
//			}
//
//			YsColor blink;
//			blink.SetIntRGB(rand()%256,rand()%256,rand()%256);
//			FsDrawString(0,y,"<< Press Key To Continue >>",blink);
//
//			FsSwapBuffers();
//
//			nextDrawTime=0.2;
//		}
//
//		FsSleep(100);
//		nextDrawTime-=0.1;
//	}
}

YSBOOL FsYesNoDialog(const char msg[],const char title[])
{
	return FsKeyMessageBox(title,msg);
}

////////////////////////////////////////////////////////////
// Below section beals with DirectDraw.

void FsSetFullScreen(int,int,int)
{
	FsMakeFullScreen();
}

void FsSetNormalWindow(void)
{
}


// Above section deals with DirectDraw
////////////////////////////////////////////////////////////


static YSBOOL firstJoyRead=YSTRUE;

static int nJoystick=0;
static YsJoyReader joystick[FsMaxNumJoystick];

static void FsCheckJoyCaps(void)
{
	if(firstJoyRead==YSTRUE)
	{
		YsJoyReaderSetUpJoystick(nJoystick,joystick,FsMaxNumJoystick);
		YsJoyReaderLoadJoystickCalibrationInfo(nJoystick,joystick);
		firstJoyRead=YSFALSE;
	}
}

int FsGetNumYsJoyReader(void)
{
  FsCheckJoyCaps();
	return nJoystick;
}

YsJoyReader *FsGetYsJoyReaderArray(void)
{
  FsCheckJoyCaps();
	return joystick;
}

YSBOOL FsIsJoystickAxisAvailable(int joyId,int joyAxs)
{
	FsCheckJoyCaps();
	FsCheckJoyCaps();
	if(joyId==FsMouseJoyId)
	{
		if(joyAxs==0 || joyAxs==1)
		{
			return YSTRUE;
		}
		else
		{
			return YSFALSE;
		}
	}
	else if(0<=joyId && joyId<nJoystick &&
	        0<=joyAxs && joyAxs<FsMaxNumJoyAxis && joyAxs<YsJoyReaderMaxNumAxis)
	{
		return (0!=joystick[joyId].axis[joyAxs].exist ? YSTRUE : YSFALSE);
	}
	return YSFALSE;
}

YSRESULT FsPollJoystick(FsJoystick &joy,int joyId)
{
	int i;
	for(i=0; i<FsMaxNumJoyAxis; i++)
	{
		joy.axs[i]=-1.0;
	}
	for(i=0; i<FsMaxNumJoyTrig; i++)
	{
		joy.trg[i]=YSFALSE;
	}
	joy.pov=YSFALSE;
	joy.povAngle=0.0;

	if(joyId==FsMouseJoyId)
	{
		int wid,hei,mx,my;
		YSBOOL lb,mb,rb;
		FsMouse(lb,mb,rb,mx,my);
		FsGetWindowSize(wid,hei);
		const int cx=wid/2;
		const int cy=hei/2;
		const int denom=YsSmaller(wid,hei);
		joy.axs[0]=YsBound(0.5+(double)(mx-cx)/(double)denom,0.0,1.0);
		joy.axs[1]=YsBound(0.5+(double)(my-cy)/(double)denom,0.0,1.0);
		joy.trg[0]=lb;
		joy.trg[1]=rb;
		joy.trg[2]=mb;
		return YSOK;
	}
	else if(0<=joyId && joyId<nJoystick)
	{
		int i;
		joystick[joyId].Read();

		for(i=0; i<FsMaxNumJoyAxis && i<YsJoyReaderMaxNumAxis; i++)
		{
			if(0!=joystick[joyId].axis[i].exist)
			{
				joy.axs[i]=(1.0+joystick[joyId].axis[i].GetCalibratedValue())/2.0;
			}
			else
			{
				joy.axs[i]=0.0;
			}
		}

		for(i=0; i<FsMaxNumJoyTrig && i<YsJoyReaderMaxNumButton; i++)
		{
			if(joystick[joyId].button[i].exist!=0)
			{
				joy.trg[i]=(0!=joystick[joyId].button[i].value ? YSTRUE : YSFALSE);
			}
			else
			{
				joy.trg[i]=YSFALSE;
			}
		}

		if(0!=joystick[joyId].hatSwitch[0].exist && 0!=joystick[joyId].hatSwitch[0].GetDiscreteValue())
		{
			int deg;
			deg=(joystick[joyId].hatSwitch[0].value-1)*45;

			joy.pov=YSTRUE;
			joy.povAngle=(double)deg*YsPi/180.0;
		}
	}

	return YSERR;
}

