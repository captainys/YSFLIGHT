#include <windows.h>
#include <ysclass.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <fsdef.h>
#include <fswindow.h>
#include <fscontrol.h>

static YSBOOL FsFirstJoystickRead=YSTRUE;

static JOYCAPS FsJoyCaps[FsMaxNumJoystick];
static YSBOOL FsJoystickIsPlugged[FsMaxNumJoystick];

static void FsGetJoystickCapabilities(void)
{
	if(FsFirstJoystickRead==YSTRUE)
	{
		int i,j;
		int nJoy;

		FsFirstJoystickRead=YSFALSE;

		for(i=0; i<FsMaxNumJoystick; i++)
		{
			FsJoystickIsPlugged[i]=YSFALSE;
		}

		nJoy=YsSmaller <int> (joyGetNumDevs(),FsMaxNumJoystick);

		for(i=0; i<nJoy; i++)
		{
			for(j=0; j<8; j++)  // Retry
			{
				JOYINFOEX joy;
				joy.dwSize=sizeof(joy);
				joy.dwFlags=JOY_RETURNALL|JOY_RETURNCENTERED;
				if(joyGetPosEx(i,&joy)==JOYERR_NOERROR)  // If it is unplugged, it'll return JOYERR_UNPLUGGED
				{
					FsJoystickIsPlugged[i]=YSTRUE;
					joyGetDevCaps(i,&FsJoyCaps[i],sizeof(JOYCAPS));
					break;
				}
				FsSleep(10);
			}
		}
	}
}

YSBOOL FsIsJoystickAxisAvailable(int joyId,int joyAxs)
{
	FsGetJoystickCapabilities();

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
	else if(0<=joyId && joyId<FsMaxNumJoystick && FsJoystickIsPlugged[joyId]==YSTRUE)
	{
		switch(joyAxs)
		{
		case 0:
		case 1:
			return YSTRUE;
		case 2:
			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASZ)
			{
				return YSTRUE;
			}
			break;
		case 3:
			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASR)
			{
				return YSTRUE;
			}
			break;
		case 4:
			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASU)
			{
				return YSTRUE;
			}
			break;
		case 5:
			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASV)
			{
				return YSTRUE;
			}
			break;
		}
	}
	return YSFALSE;
}

YSRESULT FsPollJoystick(FsJoystick &joy,int joyId)
{
	static unsigned int buttonCheckFlag[FsMaxNumJoyTrig]=
	{
		JOY_BUTTON1,
		JOY_BUTTON2,
		JOY_BUTTON3,
		JOY_BUTTON4,
		JOY_BUTTON5,
		JOY_BUTTON6,
		JOY_BUTTON7,
		JOY_BUTTON8,
		JOY_BUTTON9,
		JOY_BUTTON10,
		JOY_BUTTON11,
		JOY_BUTTON12,
		JOY_BUTTON13,
		JOY_BUTTON14,
		JOY_BUTTON15,
		JOY_BUTTON16,
		JOY_BUTTON17,
		JOY_BUTTON18,
		JOY_BUTTON19,
		JOY_BUTTON20,
		JOY_BUTTON21,
		JOY_BUTTON22,
		JOY_BUTTON23,
		JOY_BUTTON24,
		JOY_BUTTON25,
		JOY_BUTTON26,
		JOY_BUTTON27,
		JOY_BUTTON28,
		JOY_BUTTON29,
		JOY_BUTTON30,
		JOY_BUTTON31,
		JOY_BUTTON32
	};

	FsGetJoystickCapabilities();

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
		FsMouse(lb,mb,rb,mx,my);  // Never turn it to FsGetMouseEvent.  This function must not take event.
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
	else if(0<=joyId && joyId<FsMaxNumJoystick && FsJoystickIsPlugged[joyId]==YSTRUE)
	{
		JOYINFOEX joyInfo;
		joyInfo.dwSize=sizeof(JOYINFOEX);
		joyInfo.dwFlags=JOY_RETURNALL|JOY_RETURNCENTERED;
		if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASPOV)
		{
			joyInfo.dwFlags|=JOY_RETURNPOVCTS;
		}
		if(joyGetPosEx(joyId,&joyInfo)==JOYERR_NOERROR)
		{
			unsigned i;
			for(i=0; i<FsMaxNumJoyTrig && i<FsJoyCaps[joyId].wNumButtons; i++)
			{
				joy.trg[i]=((joyInfo.dwButtons&buttonCheckFlag[i])!=0 ? YSTRUE : YSFALSE);
			}

			joy.axs[0]=(double)(        joyInfo.dwXpos-FsJoyCaps[joyId].wXmin)/
			           (double)(FsJoyCaps[joyId].wXmax-FsJoyCaps[joyId].wXmin);
			joy.axs[0]=YsBound(joy.axs[0],0.0,1.0);

			joy.axs[1]=(double)(        joyInfo.dwYpos-FsJoyCaps[joyId].wYmin)/
			           (double)(FsJoyCaps[joyId].wYmax-FsJoyCaps[joyId].wYmin);
			joy.axs[1]=YsBound(joy.axs[1],0.0,1.0);

			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASZ)
			{
				joy.axs[2]=(double)(        joyInfo.dwZpos-FsJoyCaps[joyId].wZmin)/
				           (double)(FsJoyCaps[joyId].wZmax-FsJoyCaps[joyId].wZmin);
				joy.axs[2]=YsBound(joy.axs[2],0.0,1.0);
			}

			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASR)
			{
				joy.axs[3]=(double)(        joyInfo.dwRpos-FsJoyCaps[joyId].wRmin)/
				           (double)(FsJoyCaps[joyId].wRmax-FsJoyCaps[joyId].wRmin);
				joy.axs[3]=YsBound(joy.axs[3],0.0,1.0);
			}

			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASU)
			{
				joy.axs[4]=(double)(        joyInfo.dwUpos-FsJoyCaps[joyId].wUmin)/
				           (double)(FsJoyCaps[joyId].wUmax-FsJoyCaps[joyId].wUmin);
				joy.axs[4]=YsBound(joy.axs[4],0.0,1.0);
			}

			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASV)
			{
				joy.axs[5]=(double)(        joyInfo.dwVpos-FsJoyCaps[joyId].wVmin)/
				           (double)(FsJoyCaps[joyId].wVmax-FsJoyCaps[joyId].wVmin);
				joy.axs[5]=YsBound(joy.axs[5],0.0,1.0);
			}

			if(FsJoyCaps[joyId].wCaps&JOYCAPS_HASPOV)
			{
				if(0<=joyInfo.dwPOV && joyInfo.dwPOV<36000)  // If POV is neutral, dwPOV is 65535
				{
					joy.pov=YSTRUE;
					joy.povAngle=(double)joyInfo.dwPOV*YsPi/18000.0;
				}
			}

			return YSOK;
		}
	}

	return YSERR;
}
