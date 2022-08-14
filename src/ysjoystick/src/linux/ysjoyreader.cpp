#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

#include "../ysjoyreader.h"


struct Joy
{
  unsigned int tm;
  short v;
  unsigned char tp;
  unsigned char n;
};

const int JoyButtonRead=1;
const int JoyAxisRead=2;


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

class YsJoyReader::PlatformDependentInfo
{
public:
	int joyFd;
	int povXAxis,povYAxis,povXValue,povYValue;
};

YsJoyReader::YsJoyReader()
{
	platformDependentInfoPtr=new PlatformDependentInfo;
	platformDependentInfoPtr->povXAxis=-1;
	platformDependentInfoPtr->povYAxis=-1;
	platformDependentInfoPtr->povXValue=0;
	platformDependentInfoPtr->povYValue=0;
}

YsJoyReader::~YsJoyReader()
{
	delete platformDependentInfoPtr;
}

class YsJoyReader::SetUpInfo
{
};

int YsJoyReader::SetUp(int joyId,const SetUpInfo &info)
{
	// Success 1  Fail 0
	int j;
	struct Joy js;

	this->joyId=joyId;

	printf("Checking Joy [%d] Caps.\n",joyId);

	for(j=0; j<YsJoyReaderMaxNumAxis; j++)
	{
		axis[j].exist=0;
		axis[j].value=0.0;
	}
	for(j=0; j<YsJoyReaderMaxNumButton; j++)
	{
		button[j].exist=0;
	}
	hatSwitch[0].exist=1;
	hatSwitch[0].value=0;
	for(j=1; j<YsJoyReaderMaxNumHatSwitch; j++)
	{
		hatSwitch[j].exist=0;
	}



	char fn[256];
	sprintf(fn,"/dev/input/js%d",joyId);
	platformDependentInfoPtr->joyFd=open(fn,O_RDONLY);

	if(platformDependentInfoPtr->joyFd<0)
	{
		sprintf(fn,"/dev/js%d",joyId);
		platformDependentInfoPtr->joyFd=open(fn,O_RDONLY);
	}

	if(platformDependentInfoPtr->joyFd>=0)
	{
		while(1)
		{
			struct pollfd pfd;
			pfd.fd=platformDependentInfoPtr->joyFd;
			pfd.events=POLLIN|POLLPRI;
			if(poll(&pfd,1,0)<=0)
			{
				break;
			}

			if(read(platformDependentInfoPtr->joyFd,&js,sizeof(struct Joy))==sizeof(struct Joy))
			{
				if((js.tp&JoyButtonRead) && js.n<YsJoyReaderMaxNumButton)
				{
					button[js.n].exist=1;
					button[js.n].value=js.v;
				}
				if((js.tp&JoyAxisRead) && js.n<YsJoyReaderMaxNumAxis)
				{
					axis[js.n].exist=1;
					axis[js.n].value=js.v;
					axis[js.n].calibCenter=0;
					axis[js.n].calibMin=-32768;
					axis[js.n].calibMax= 32767;
				}

				int povChange;
				povChange=0;
				if((js.tp&JoyAxisRead) && js.n==platformDependentInfoPtr->povXAxis)
				{
					platformDependentInfoPtr->povXValue=js.v;
					povChange=1;
				}
				if((js.tp&JoyAxisRead) && js.n==platformDependentInfoPtr->povYAxis)
				{
					platformDependentInfoPtr->povYValue=js.v;
					povChange=1;
				}
				if(povChange!=0)
				{
					if(platformDependentInfoPtr->povXAxis==0 && platformDependentInfoPtr->povYAxis==0)
					{
						hatSwitch[0].value=0;
					}
					else if(platformDependentInfoPtr->povXAxis==0 && platformDependentInfoPtr->povYAxis>0)
					{
						hatSwitch[0].value=1;
					}
					else if(platformDependentInfoPtr->povXAxis>0 && platformDependentInfoPtr->povYAxis>0)
					{
						hatSwitch[0].value=2;
					}
					else if(platformDependentInfoPtr->povXAxis>0 && platformDependentInfoPtr->povYAxis==0)
					{
						hatSwitch[0].value=3;
					}
					else if(platformDependentInfoPtr->povXAxis>0 && platformDependentInfoPtr->povYAxis<0)
					{
						hatSwitch[0].value=4;
					}
					else if(platformDependentInfoPtr->povXAxis==0 && platformDependentInfoPtr->povYAxis<0)
					{
						hatSwitch[0].value=5;
					}
					else if(platformDependentInfoPtr->povXAxis<0 && platformDependentInfoPtr->povYAxis<0)
					{
						hatSwitch[0].value=6;
					}
					else if(platformDependentInfoPtr->povXAxis<0 && platformDependentInfoPtr->povYAxis==0)
					{
						hatSwitch[0].value=7;
					}
					else if(platformDependentInfoPtr->povXAxis<0 && platformDependentInfoPtr->povYAxis>0)
					{
						hatSwitch[0].value=8;
					}
				}
			}
		}
		return 1;
	}
	return 0;
}

void YsJoyReader::Read(void)
{
	int j,n;
	struct Joy js;

	if(platformDependentInfoPtr->joyFd<0)
	{
	    return;
	}

	while(1)
	{
		struct pollfd pfd;
		pfd.fd=platformDependentInfoPtr->joyFd;
		pfd.events=POLLIN|POLLPRI;
		if(poll(&pfd,1,0)<=0)
		{
			break;
		}

		if(read(platformDependentInfoPtr->joyFd,&js,sizeof(struct Joy))==sizeof(struct Joy))
		{
			if((js.tp&JoyButtonRead) && js.n<YsJoyReaderMaxNumButton)
			{
				button[js.n].value=js.v;
			}
			else if((js.tp&JoyAxisRead) && js.n<YsJoyReaderMaxNumAxis)
			{
				axis[js.n].value=js.v;
			}

			int povChange;
			povChange=0;
			if((js.tp&JoyAxisRead) && js.n==platformDependentInfoPtr->povXAxis)
			{
				platformDependentInfoPtr->povXValue=js.v;
				povChange=1;
			}
			if((js.tp&JoyAxisRead) && js.n==platformDependentInfoPtr->povYAxis)
			{
				platformDependentInfoPtr->povYValue=js.v;
				povChange=1;
			}
			if(povChange!=0)
			{
				if(platformDependentInfoPtr->povXValue==0 && platformDependentInfoPtr->povYValue==0)
				{
					hatSwitch[0].value=0;
				}
				else if(platformDependentInfoPtr->povXValue==0 && platformDependentInfoPtr->povYValue<0)
				{
					hatSwitch[0].value=1;
				}
				else if(platformDependentInfoPtr->povXValue>0 && platformDependentInfoPtr->povYValue<0)
				{
					hatSwitch[0].value=2;
				}
				else if(platformDependentInfoPtr->povXValue>0 && platformDependentInfoPtr->povYValue==0)
				{
					hatSwitch[0].value=3;
				}
				else if(platformDependentInfoPtr->povXValue>0 && platformDependentInfoPtr->povYValue>0)
				{
					hatSwitch[0].value=4;
				}
				else if(platformDependentInfoPtr->povXValue==0 && platformDependentInfoPtr->povYValue>0)
				{
					hatSwitch[0].value=5;
				}
				else if(platformDependentInfoPtr->povXValue<0 && platformDependentInfoPtr->povYValue>0)
				{
					hatSwitch[0].value=6;
				}
				else if(platformDependentInfoPtr->povXValue<0 && platformDependentInfoPtr->povYValue==0)
				{
					hatSwitch[0].value=7;
				}
				else if(platformDependentInfoPtr->povXValue<0 && platformDependentInfoPtr->povYValue<0)
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
	if(platformDependentInfoPtr->povXAxis>=0 && platformDependentInfoPtr->povYAxis>=0)
	{
		fprintf(fp,"POVAXS %d %d\n",platformDependentInfoPtr->povXAxis,platformDependentInfoPtr->povYAxis);
	}
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
		else if(strncmp(str,"POVAXS",6)==0)
		{
			sscanf(str,"%*s %d %d",&platformDependentInfoPtr->povXAxis,&platformDependentInfoPtr->povYAxis);
		}
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
	FILE *fp;
	char *fullPath,*homeDir;
	homeDir=getenv("HOME");

	int lng;
	lng=strlen(homeDir);
	lng+=strlen(".ysjoycalib");

	fullPath=new char [lng+16];
	sprintf(fullPath,"%s/.ysjoycalib",homeDir);

	fp=fopen(fullPath,mode);
	delete [] fullPath;

	return fp;
}

