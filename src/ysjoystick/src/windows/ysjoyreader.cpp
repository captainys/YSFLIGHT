#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>

#include <mmsystem.h>

#include "../ysjoyreader.h"

YsJoyReaderElement::YsJoyReaderElement()
{
	exist=0;
	value=0;
	platformDependentInfoPtr=nullptr;

}

YsJoyReaderElement::~YsJoyReaderElement()
{
}

YsJoyReaderAxis::YsJoyReaderAxis()
{
	min=0;
	max=0;
	calibCenter=32768;
	calibMin=31768;
	calibMax=33768;
}

int YsJoyReaderHatSwitch::GetDiscreteValue(void) const
{
	if(value==valueNeutral)
	{
		return 0;
	}
	else if(value==value0Deg)
	{
		return 1;
	}
	else if(value0Deg<value && value<value90Deg)
	{
		return 2;
	}
	else if(value==value90Deg)
	{
		return 3;
	}
	else if(value90Deg<value && value<value180Deg)
	{
		return 4;
	}
	else if(value==value180Deg)
	{
		return 5;
	}
	else if(value180Deg<value && value<value270Deg)
	{
		return 6;
	}
	else if(value270Deg==value)
	{
		return 7;
	}
	else if(value270Deg<value)
	{
		return 8;
	}
	return 0;
}

YsJoyReader::YsJoyReader()
{
	platformDependentInfoPtr=nullptr;
}
YsJoyReader::~YsJoyReader()
{
}

class YsJoyReader::SetUpInfo
{
};

int YsJoyReader::SetUp(int joyId,const SetUpInfo &info)
{
	// Success 1  Fail 0

	JOYCAPS caps;
	if(joyGetDevCaps(joyId,&caps,sizeof(caps))==JOYERR_NOERROR)
	{
		strncpy(name,caps.szPname,255);
		name[256]=0;
		printf("[%s]\n",name);

		this->joyId=joyId;

		axis[0].exist=1;
		axis[0].min=caps.wXmin;
		axis[0].max=caps.wXmax;

		axis[1].exist=1;
		axis[1].min=caps.wYmin;
		axis[1].max=caps.wYmax;

		if(caps.wCaps&JOYCAPS_HASZ)
		{
			axis[2].exist=1;
			axis[2].min=caps.wZmin;
			axis[2].max=caps.wZmax;
		}
		if(caps.wCaps&JOYCAPS_HASR)
		{
			axis[3].exist=1;
			axis[3].min=caps.wRmin;
			axis[3].max=caps.wRmax;
		}
		if(caps.wCaps&JOYCAPS_HASU)
		{
			axis[4].exist=1;
			axis[4].min=caps.wUmin;
			axis[4].max=caps.wUmax;
		}
		if(caps.wCaps&JOYCAPS_HASV)
		{
			axis[5].exist=1;
			axis[5].min=caps.wVmin;
			axis[5].max=caps.wVmax;
		}
		if(caps.wCaps&JOYCAPS_HASPOV)
		{
			hatSwitch[0].exist=1;
		}

		for(int i=0; i<YsJoyReaderMaxNumAxis; i++)
		{
			if(axis[i].exist!=0)
			{
				axis[i].calibCenter=(axis[i].min+axis[i].max)/2;
				axis[i].calibMin=axis[i].min;
				axis[i].calibMax=axis[i].max;
			}
		}

		for(unsigned int i=0; i<YsJoyReaderMaxNumButton && i<caps.wMaxButtons; i++)
		{
			button[i].exist=1;
		}

		return 1;
	}
	return 0;
}

void YsJoyReader::Read(void)
{
	int i,bit;
	JOYINFOEX joy;

	joy.dwSize=sizeof(joy);
	joy.dwFlags=JOY_RETURNALL /*|JOY_RETURNRAWDATA */;
	if(joyGetPosEx(joyId,&joy)==JOYERR_NOERROR)
	{
		axis[0].value=joy.dwXpos;
		axis[1].value=joy.dwYpos;
		axis[2].value=joy.dwZpos;
		axis[3].value=joy.dwRpos;
		axis[4].value=joy.dwUpos;
		axis[5].value=joy.dwVpos;

		bit=1;
		for(i=0; i<32 && i<YsJoyReaderMaxNumButton; i++)
		{
			button[i].value=((joy.dwButtons&bit)!=0);
			bit<<=1;
		}

		if(hatSwitch[0].exist!=0)
		{
			if(joy.dwPOV==JOY_POVCENTERED)
			{
				hatSwitch[0].value=0;
			}
			else
			{
				if((33750<=joy.dwPOV && joy.dwPOV<35900) || (0<=joy.dwPOV && joy.dwPOV<2250))
				{
					hatSwitch[0].value=1;
				}
				else if(2250<=joy.dwPOV && joy.dwPOV<6750)
				{
					hatSwitch[0].value=2;
				}
				else if(6750<=joy.dwPOV && joy.dwPOV<11250)
				{
					hatSwitch[0].value=3;
				}
				else if(11250<=joy.dwPOV && joy.dwPOV<15750)
				{
					hatSwitch[0].value=4;
				}
				else if(15750<=joy.dwPOV && joy.dwPOV<20250)
				{
					hatSwitch[0].value=5;
				}
				else if(20250<=joy.dwPOV && joy.dwPOV<24750)
				{
					hatSwitch[0].value=6;
				}
				else if(24750<=joy.dwPOV && joy.dwPOV<29250)
				{
					hatSwitch[0].value=7;
				}
				else if(29250<=joy.dwPOV && joy.dwPOV<33750)
				{
					hatSwitch[0].value=8;
				}
			}
		}
	}
}

int YsJoyReader::WriteCalibInfoFile(FILE *fp) const
{
	int i;
	fprintf(fp,"BGNJOY %d\n",joyId);
	for(i=0; i<YsJoyReaderMaxNumAxis; i++)
	{
		if(axis[i].exist!=0)
		{
			fprintf(fp,"AXSINF %d %d %d %d\n",i,axis[i].calibCenter,axis[i].calibMin,axis[i].calibMax);
		}
	}
#ifdef YSJOYREADER_USE_HAT_CALIBRATION
	for(i=0; i<YsJoyReaderMaxNumHatSwitch; i++)
	{
		if(0!=hatSwitch[i].exist)
		{
			fprintf(fp,"HATINF %d %d %d %d %d %d\n",
			    i,
			    hatSwitch[i].valueNeutral,
			    hatSwitch[i].value0Deg,
			    hatSwitch[i].value90Deg,
			    hatSwitch[i].value180Deg,
			    hatSwitch[i].value270Deg);
		}
	}
#endif
	fprintf(fp,"ENDJOY\n");
	return 1;
}

int YsJoyReader::ReadCalibInfoFile(FILE *fp)
{
	char str[256];
	while(fgets(str,255,fp)!=NULL)
	{
		if(strncmp(str,"AXSINF",6)==0)
		{
			int axisId,cen,min,max;
			sscanf(str,"%*s %d %d %d %d",&axisId,&cen,&min,&max);
			if(0<=axisId && axisId<YsJoyReaderMaxNumAxis)
			{
				axis[axisId].calibCenter=cen;
				axis[axisId].calibMin=min;
				axis[axisId].calibMax=max;
			}
		}
#ifdef YSJOYREADER_USE_HAT_CALIBRATION
		else if(strncmp(str,"HATINF",6)==0)
		{
			int hatId;
			int valueNeutral=0,value0Deg=1,value90Deg=3,value180Deg=5,value270Deg=7;
			sscanf(str,"%*s %d %d %d %d %d %d",&hatId,&valueNeutral,&value0Deg,&value90Deg,&value180Deg,&value270Deg);
			if(0<=hatId && hatId<YsJoyReaderMaxNumHatSwitch)
			{
				hatSwitch[hatId].valueNeutral=valueNeutral;
				hatSwitch[hatId].value0Deg=value0Deg;
				hatSwitch[hatId].value90Deg=value90Deg;
				hatSwitch[hatId].value180Deg=value180Deg;
				hatSwitch[hatId].value270Deg=value270Deg;
			}
		}
#endif
		else if(strncmp(str,"ENDJOY",6)==0)
		{
			return 1;
		}
	}
	return 0;
}

int YsJoyReaderSetUpJoystick(int &nJoystick,YsJoyReader joystick[],int maxNumJoystick)
{
	int i;

	nJoystick=0;
	for(i=0; i<maxNumJoystick; i++)
	{
		YsJoyReader::SetUpInfo info;
		if(joystick[nJoystick].SetUp(i,info)!=0)
		{
			nJoystick++;
			if(nJoystick==maxNumJoystick)
			{
				break;
			}
		}
	}

	return nJoystick;
}


FILE *YsJoyReaderOpenJoystickCalibrationFile(const char mode[])
{
	return fopen("ysjoycalib.dat",mode);
}

