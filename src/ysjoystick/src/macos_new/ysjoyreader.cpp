#include <stdio.h>
#include <algorithm>

#include "../ysjoyreader.h"

#include "ysjoyreader_objc.h"


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
	unsigned int gamepadId;	
};

YsJoyReader::YsJoyReader()
{
	platformDependentInfoPtr=new PlatformDependentInfo;
	platformDependentInfoPtr->gamepadId=0;
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
	platformDependentInfoPtr->gamepadId=joyId;
	axis[0].exist=1;
	axis[1].exist=1;
	axis[2].exist=1;
	axis[3].exist=1;
	button[0].exist=1;
	button[1].exist=1;
	button[2].exist=1;
	button[3].exist=1;
	button[4].exist=1;
	button[5].exist=1;
	button[6].exist=1;
	button[7].exist=1;
	hatSwitch[0].exist=1;
	return 0;
}

void YsJoyReader::Read(void)
{
}

int YsJoyReader::WriteCalibInfoFile(FILE *fp) const
{
	return 1;
}

int YsJoyReader::ReadCalibInfoFile(FILE *fp)
{
	return 0;
}

int YsJoyReaderSetUpJoystick(int &nJoystick,YsJoyReader joystick[],int maxNumJoystick)
{
	YsJoyReader_MacOS_InitializeController();
	nJoystick=std::min(4,maxNumJoystick); // Tentatively make it 4
	for(int i=0; i<nJoystick; ++i)
	{
		SetUpInfo info;
		joystick[i].SetUp(i,info);
	}
	return nJoystick;
}


FILE *YsJoyReaderOpenJoystickCalibrationFile(const char mode[])
{
	return NULL;
}

