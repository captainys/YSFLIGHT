#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <unistd.h>


#include <ysclass.h>
#include <ysport.h>
#include <fsgui.h>

#include <fssimplewindow.h>

#include "fsconfig.h"
#include "fsoption.h"

#include "fs.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"


#include "ysglfontdata.h"
#include "ysjoyreader.h"


#include "fsmacosx_alert.h"



// fsmacosxwrapper.o >>
extern "C" void FsOpenWindowC(int x0,int y0,int wid,int hei,int useDoubleBuffer,int useAntiAliasing);
extern "C" void FsGetWindowSizeC(int *wid,int *hei);
extern "C" void FsGetWindowClientPositionC(int *x0,int *y0,int *wid,int *hei);
extern "C" void FsPollDeviceC(void);
extern "C" void FsSleepC(int ms);
extern "C" int FsPassedTimeC(void);
extern "C" void FsMouseC(int *lb,int *mb,int *rb,int *mx,int *my);
extern "C" int FsGetMouseEventC(int *lb,int *mb,int *rb,int *mx,int *my);
extern "C" void FsSwapBufferC(void);
extern "C" int FsInkeyC(void);
extern "C" int FsInkeyCharC(void);
extern "C" int FsGetKeyStateC(int fsKeyCode);
extern "C" void FsChangeToProgramDirC(void);

extern "C" int FsIsMainWindowActiveC(void);
extern "C" int FsIsSubWindowActiveC(int subWndId);
extern "C" int FsIsSubWindowOpenC(int subWindowId);
extern "C" int FsCloseSubWindowC(int subWindowId);
extern "C" int FsSelectMainWindowC(void);
extern "C" int FsSelectSubWindowC(int subWindowId);

extern "C" void FsOpenWebSiteC(const char url[]);

extern "C" const char *FsGetLanguageIdC(char buf[]);
// fsmacosxwrapper.o <<


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

// Interval callback >>


extern "C" void FsInvokeIntervalCallBack(void)
{
	if(NULL!=fsIntervalCallBack)
	{
		(*fsIntervalCallBack)(fsIntervalCallBackParameter);
	}
}

// Interval callback <<

void FsBeforeOpenWindow(const class FsOption &,const class FsFlightConfig &)
{
}
void FsAfterOpenWindow(const class FsOption &opt,const class FsFlightConfig &cfg)
{
	if(1==opt.scrnMode)
	{
		FsMaximizeWindow();
	}
	else if(1<opt.scrnMode)
	{
		FsSetFullScreen(0,0,0);
	}

	if(YSTRUE==cfg.useOpenGlAntiAliasing)
	{
		glEnable(GL_MULTISAMPLE);
	}
}

void FsSetOnPaintCallback(class FsOnPaintCallback *)
{
}

void FsHidePartOfScreenForSharewareMessage(void)
{
}

void FsMessageBox(const char msg[],const char title[])
{
	FsMessageDialogC(title,msg,"OK");
}

YSBOOL FsYesNoDialog(const char msg[],const char title[])
{
	return (YSBOOL)FsYesNoDialogC(title,msg,"OK","Cancel");
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

