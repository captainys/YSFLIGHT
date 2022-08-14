#ifndef YSJOYSTICK_IS_INCLUDED
#define YSJOYSTICK_IS_INCLUDED
/* { */


#include <stdio.h>

// #define YSJOYREADER_USE_HAT_CALIBRATION


const int YsJoyReaderMaxNumAxis=6;
const int YsJoyReaderMaxNumButton=32;
const int YsJoyReaderMaxNumHatSwitch=4;


class YsJoyReaderElement
{
public:
	class PlatformDependentInfo;
	int exist;
	int value;
	PlatformDependentInfo *platformDependentInfoPtr;

	YsJoyReaderElement();
	~YsJoyReaderElement();
};

class YsJoyReaderAxis : public YsJoyReaderElement
{
public:
	int min,max;
	int calibCenter,calibMin,calibMax;

	YsJoyReaderAxis();
	double GetCalibratedValue(void) const;

	void CaptureCenter(void);
	void BeginCaptureMinMax(void);
	void CaptureMinMax(void);
	void CenterFromMinMax(void);
};

class YsJoyReaderButton : public YsJoyReaderElement
{
public:
	YsJoyReaderButton();
};

class YsJoyReaderHatSwitch : public YsJoyReaderElement
{
public:
	YsJoyReaderHatSwitch();
	int valueNeutral;
	int value0Deg;
	int value90Deg;
	int value180Deg;
	int value270Deg;
	int GetDiscreteValue(void) const;
};

class YsJoyReader
{
public:
	class PlatformDependentInfo;
	PlatformDependentInfo *platformDependentInfoPtr;

	int joyId;
	char name[256];
	YsJoyReaderAxis axis[YsJoyReaderMaxNumAxis];
	YsJoyReaderButton button[YsJoyReaderMaxNumButton];
	YsJoyReaderHatSwitch hatSwitch[YsJoyReaderMaxNumHatSwitch];

	YsJoyReader();
	~YsJoyReader();

	class SetUpInfo;
	int SetUp(int joyId,const SetUpInfo &info);
	void Read(void);

	int WriteCalibInfoFile(FILE *fp) const;
	int ReadCalibInfoFile(FILE *fp);
};

int YsJoyReaderSetUpJoystick(int &nJoystick,YsJoyReader joystick[],int maxNumJoystick);

FILE *YsJoyReaderOpenJoystickCalibrationFile(const char mode[]);

int YsJoyReaderSaveJoystickCalibrationInfo(int nJoystick,YsJoyReader joystick[]);
int YsJoyReaderLoadJoystickCalibrationInfo(int nJoystick,YsJoyReader joystick[]);

/* } */
#endif
