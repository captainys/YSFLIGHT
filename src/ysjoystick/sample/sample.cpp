// Compiled as: g++ -I../macosx sample.cpp ../macosx/ysjoyreader.cpp ../macosx/ysjoyreader-objc.m -framework IOKit -framework Foundation

#include <stdio.h>
#include <ysjoyreader.h>
#include <unistd.h>

const int maxNumJoystick=4;
int numJoystick;
YsJoyReader joystick[maxNumJoystick];

int main(void)
{
	YsJoyReaderSetUpJoystick(numJoystick,joystick,maxNumJoystick);
	YsJoyReaderLoadJoystickCalibrationInfo(numJoystick,joystick);

	while(1)
	{
		int i,j;
		for(i=0; i<numJoystick; i++)
		{
			joystick[i].Read();

			printf("Joy[%d]",i);
			for(j=0; j<YsJoyReaderMaxNumAxis; j++)
			{
				if(joystick[i].axis[j].exist!=0)
				{
					printf(" Ax%d:%+5.2lf",j,joystick[i].axis[j].GetCalibratedValue());
				}
			}
			for(j=0; j<YsJoyReaderMaxNumButton; j++)
			{
				if(joystick[i].button[j].exist!=0)
				{
					printf(" Bt%d:%d",j,joystick[i].button[j].value);
				}
			}
			for(j=0; j<YsJoyReaderMaxNumHatSwitch; j++)
			{
				if(joystick[i].hatSwitch[j].exist!=0)
				{
					printf(" POV%d:%d",j,joystick[i].hatSwitch[j].value);
				}
			}
			printf("\n");
		}

		sleep(1);
	}

	return 0;
}
