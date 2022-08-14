#include <stdio.h>

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
	nJoystick=0;
	return nJoystick;
}


FILE *YsJoyReaderOpenJoystickCalibrationFile(const char mode[])
{
	return NULL;
}

