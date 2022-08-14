#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sysexits.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hid/IOHIDLib.h>




#include "../ysjoyreader.h"


class YsJoyReaderElement::PlatformDependentInfo
{
public:
	IOHIDElementRef elem;
	int scaledMin,scaledMax;
};

YsJoyReaderElement::YsJoyReaderElement()
{
	exist=0;
	value=0;
	platformDependentInfoPtr=new PlatformDependentInfo;
}

YsJoyReaderElement::~YsJoyReaderElement()
{
	delete platformDependentInfoPtr;
}

YsJoyReaderAxis::YsJoyReaderAxis()
{
	min=0;
	max=0;
	calibCenter=0;
	calibMin=0;
	calibMax=0;

	platformDependentInfoPtr->scaledMin=0;
	platformDependentInfoPtr->scaledMax=0;
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
	else if(value==value90Deg)
	{
		return 3;
	}
	else if(value==value180Deg)
	{
		return 5;
	}
	else if(value270Deg==value)
	{
		return 7;
	}
	else if(value0Deg<value && value<value90Deg)
	{
		return 2;
	}
	else if(value90Deg<value && value<value180Deg)
	{
		return 4;
	}
	else if(value180Deg<value && value<value270Deg)
	{
		return 6;
	}
	else if(value270Deg<value)
	{
		return 8;
	}
	return 0;
}


////////////////////////////////////////////////////////////


class YsJoyReader::PlatformDependentInfo
{
public:
	static IOHIDManagerRef hidManager;
	static CFMutableArrayRef devArray;
	IOHIDDeviceRef hidDev;
};

IOHIDManagerRef YsJoyReader::PlatformDependentInfo::hidManager=NULL;
CFMutableArrayRef YsJoyReader::PlatformDependentInfo::devArray=NULL;

YsJoyReader::YsJoyReader()
{
	platformDependentInfoPtr=new PlatformDependentInfo;
	platformDependentInfoPtr->hidDev=NULL;
	name[0]=0;
}

YsJoyReader::~YsJoyReader()
{
	delete platformDependentInfoPtr;
}

static bool requestIssued=false;

class YsJoyReader::SetUpInfo
{
public:
	IOHIDDeviceRef hidDev;
};

static void AddAxis(YsJoyReader &reader,int axisId,IOHIDElementRef elem,int min,int max,int scaledMin,int scaledMax)
{
	if(0<=axisId && axisId<YsJoyReaderMaxNumAxis)
	{
		reader.axis[axisId].exist=1;
		reader.axis[axisId].min=min;
		reader.axis[axisId].max=max;

		reader.axis[axisId].calibCenter=(min+max)/2;
		reader.axis[axisId].calibMin=min;
		reader.axis[axisId].calibMax=max;

		reader.axis[axisId].platformDependentInfoPtr->elem=elem;
		reader.axis[axisId].platformDependentInfoPtr->scaledMin=scaledMin;
		reader.axis[axisId].platformDependentInfoPtr->scaledMax=scaledMax;

		CFRetain(elem);
	}
}

int YsJoyReader::SetUp(int joyId,const SetUpInfo &info)
{
	// https://stackoverflow.com/questions/58670785/which-api-is-behind-the-privacy-feature-input-monitoring-in-the-security-syste
	if(kIOHIDAccessTypeGranted!=IOHIDCheckAccess(kIOHIDRequestTypeListenEvent))
	{
		if(true!=requestIssued)
		{
			IOHIDRequestAccess(kIOHIDRequestTypeListenEvent);
			requestIssued=true;
		}
		return 0;
	}


	this->joyId=joyId;

	if(info.hidDev!=NULL)
	{
		CFArrayRef elemAry=IOHIDDeviceCopyMatchingElements(info.hidDev,NULL,0);
		int nElem=(int)CFArrayGetCount(elemAry);
		bool isMouse=false,isJoystick=false,isKeyboard=false,isGamePad=false;

		printf("<<Device>>\n");
		// See
		// https://developer.apple.com/library/prerelease/mac/samplecode/HID_Utilities/Listings/IOHIDManager_IOHIDDevice__c.html
		// >>
		// {
		// 	char strBuf[256];
		// 	CFStringRef manufacturer=(CFStringRef)IOHIDDeviceGetProperty(info.hidDev,CFSTR(kIOHIDManufacturerKey));
		// 	if(NULL!=manufacturer)
		// 	{
		// 		CFStringGetCString(manufacturer,strBuf,255,kCFStringEncodingUTF8);
		// 		printf("%s\n",strBuf);
		// 	}
		// 	CFStringRef product=(CFStringRef)IOHIDDeviceGetProperty(info.hidDev,CFSTR(kIOHIDProductKey));
		// 	if(NULL!=manufacturer)
		// 	{
		// 		CFStringGetCString(product,strBuf,255,kCFStringEncodingUTF8);
		// 		printf("%s\n",strBuf);
		// 	}
		// }
		// <<

		// Yes!  This is the correct way of identifying joysticks!   2015/04/03
		isMouse=IOHIDDeviceConformsTo(info.hidDev,kHIDPage_GenericDesktop,kHIDUsage_GD_Mouse);
		isKeyboard=IOHIDDeviceConformsTo(info.hidDev,kHIDPage_GenericDesktop,kHIDUsage_GD_Keyboard);
		isJoystick=IOHIDDeviceConformsTo(info.hidDev,kHIDPage_GenericDesktop,kHIDUsage_GD_Joystick);
		isGamePad=IOHIDDeviceConformsTo(info.hidDev,kHIDPage_GenericDesktop,kHIDUsage_GD_GamePad);

		printf("As Joystick: %d\n",(int)isJoystick);
		printf("As GamePad:  %d\n",(int)isGamePad);
		printf("As Mouse:    %d\n",(int)isMouse);
		printf("As Keyboard: %d\n",(int)isKeyboard);

		CFTypeRef locIdRef=IOHIDDeviceGetProperty(info.hidDev,CFSTR(kIOHIDLocationIDKey));
		if(NULL!=locIdRef && CFNumberGetTypeID()==CFGetTypeID(locIdRef))
		{
			long locId;
			CFNumberGetValue((CFNumberRef)locIdRef,kCFNumberSInt32Type,&locId);
			printf("Location ID=%08x\n",(int)locId);
		}
		else
		{
			printf("Cannot acquire location ID.\n");
		}

		printf("This HID Device has %d elements.\n",nElem);

		int j;
		for(j=0; j<nElem; j++)
		{
			IOHIDElementRef elem=(IOHIDElementRef)CFArrayGetValueAtIndex(elemAry,j);
			IOHIDElementType elemType=IOHIDElementGetType(elem);
			unsigned int usage=IOHIDElementGetUsage(elem);
			unsigned int usagePage=IOHIDElementGetUsagePage(elem);

			// 2015/04/03
			// IOHIDElementGetName seems to return NULL always.
			// CFStringRef elemName=IOHIDElementGetName(elem);
			// if(NULL!=elemName)
			// {
			// 	char nameBuf[256];
			// 	CFStringGetCString(elemName,nameBuf,255,kCFStringEncodingUTF8);
			// 	printf("%s\n",nameBuf);
			// }

			// printf("Element %3d",j);
			switch(elemType)
			{
			default:
				// printf(" Unknown   ");
				break;
			case kIOHIDElementTypeInput_ScanCodes:
				// printf(" ScanCode  ");
				break;
			case kIOHIDElementTypeInput_Misc:
				// printf(" Misc      ");
				break;
			case kIOHIDElementTypeInput_Button:
				// printf(" Button    ");
				break;
			case kIOHIDElementTypeInput_Axis:
				// printf(" Axis      ");
				break;
			case kIOHIDElementTypeOutput:
				// printf(" Output    ");
				break;
			case kIOHIDElementTypeFeature:
				// printf(" Feature   ");
				break;
			case kIOHIDElementTypeCollection:
				// printf(" Collection");
				break;
			}

			// printf("  Usage %3d  UsagePage %3d\n",usage,usagePage);

			// It appears that usage and usagePage should be only looked at if the element type is Collection.
			// Otherwise, Microsoft Keyboard states itself as a keyboard, mouse, joystick, and gamepad at the same time.
			// However, I don't see any official documentation that is stating so.  Instead, IOHIDDeviceConformsTo
			// works ok.
			// if(kHIDPage_GenericDesktop==usagePage)
			// {
			// 	// http://www.opensource.apple.com/source/IOHIDFamily/IOHIDFamily-315.7.16/IOHIDFamily/IOHIDUsageTables.h
			// 	switch(usage)
			// 	{
			// 	default:
			// 		printf("    Unknown Usage.          \n");
			// 		break;
			// 	case kHIDUsage_GD_Mouse:
			// 		printf("    Can function as mouse   \n");
			// 		break;
			// 	case kHIDUsage_GD_Keyboard:
			// 		printf("    Can function as Keyboard\n");
			// 		break;
			// 	case kHIDUsage_GD_Joystick:
			// 		printf("    Can function as Joystick\n");
			// 		break;
			// 	case kHIDUsage_GD_GamePad:
			// 		printf("    Can function as GamePad \n");
			// 		break;
			// 	}
			// }
			// else
			// {
			// 	printf("\n");
			// }
		}

		if(0!=isJoystick || 0!=isGamePad)
		{
			int nAxis=0;
			int nHat=0;

			int j;
			for(j=0; j<nElem; j++)
			{
				IOHIDElementRef elem=(IOHIDElementRef)CFArrayGetValueAtIndex(elemAry,j);
				IOHIDElementType elemType=IOHIDElementGetType(elem);
				unsigned int usage=IOHIDElementGetUsage(elem);
				unsigned int usagePage=IOHIDElementGetUsagePage(elem);
				// The following two returned 0 and 255
				// IOHIDElementGetPhysicalMin(elem);
				// IOHIDElementGetPhysicalMax(elem);
				int min=IOHIDElementGetLogicalMin(elem);
				int max=IOHIDElementGetLogicalMax(elem);
				int scaledMin=min;
				int scaledMax=max;

				if(elemType==kIOHIDElementTypeInput_Misc ||
				   elemType==kIOHIDElementTypeInput_Button ||
				   elemType==kIOHIDElementTypeInput_Axis ||
				   elemType==kIOHIDElementTypeInput_ScanCodes)
				{
					switch(usagePage)
					{
					case kHIDPage_GenericDesktop:
						switch(usage)
						{
						case kHIDUsage_GD_Mouse:
							break;
						case kHIDUsage_GD_Keyboard:
							break;
						case kHIDUsage_GD_Joystick:
							break;
						case kHIDUsage_GD_GamePad:
							break;
						case kHIDUsage_GD_X:
							printf("    This element is for X-Axis (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							AddAxis(*this,nAxis++,elem,min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Y:
							printf("    This element is for Y-Axis (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							AddAxis(*this,nAxis++,elem,min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Z:
							printf("    This element is for Z-Axis (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							AddAxis(*this,nAxis++,elem,min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Rx:
							printf("    This element is for Rx-Axis (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							AddAxis(*this,nAxis++,elem,min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Ry:
							printf("    This element is for Ry-Axis (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							AddAxis(*this,nAxis++,elem,min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Rz:
							printf("    This element is for Rz-Axis (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							AddAxis(*this,nAxis++,elem,min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Slider:
							printf("    This element is for Slider (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							AddAxis(*this,nAxis++,elem,min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Wheel:
							printf("    This element is for Wheel (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							break;
						case kHIDUsage_GD_Hatswitch:
							printf("    This element is for Hatswitch (%d->%d) Scaled(%d->%d)\n",min,max,scaledMin,scaledMax);
							if(nHat<YsJoyReaderMaxNumHatSwitch)
							{
								hatSwitch[nHat].exist=1;
								hatSwitch[nHat].platformDependentInfoPtr->elem=elem;
								CFRetain(elem);
								nHat++;
							}
							break;
						}
						break;
					case kHIDPage_Button:
						printf("    This element is for Button %d\n",usage-1);
						usage--;
						if(usage<YsJoyReaderMaxNumButton)
						{
							button[usage].exist=1;
							button[usage].platformDependentInfoPtr->elem=elem;
							CFRetain(elem);
						}
						break;
					}
				}
			}
			CFRelease(elemAry);
			this->platformDependentInfoPtr->hidDev=info.hidDev;
			return 1;

		}
		CFRelease(elemAry);
	}

	return 0;
}

void YsJoyReader::Read(void)
{
	// https://stackoverflow.com/questions/58670785/which-api-is-behind-the-privacy-feature-input-monitoring-in-the-security-syste
	if(kIOHIDAccessTypeGranted!=IOHIDCheckAccess(kIOHIDRequestTypeListenEvent))
	{
		if(true!=requestIssued)
		{
			IOHIDRequestAccess(kIOHIDRequestTypeListenEvent);
			requestIssued=true;
		}
		return;
	}

	int i;
	IOHIDValueRef valueRef;
	for(i=0; i<YsJoyReaderMaxNumAxis; i++)
	{
		if(axis[i].exist!=0)
		{
			IOHIDDeviceGetValue(platformDependentInfoPtr->hidDev,axis[i].platformDependentInfoPtr->elem,&valueRef);
			axis[i].value=IOHIDValueGetIntegerValue(valueRef);
		}
	}
	for(i=0; i<YsJoyReaderMaxNumButton; i++)
	{
		if(button[i].exist!=0)
		{
			IOHIDDeviceGetValue(platformDependentInfoPtr->hidDev,button[i].platformDependentInfoPtr->elem,&valueRef);
			button[i].value=IOHIDValueGetIntegerValue(valueRef);
		}
	}
	for(i=0; i<YsJoyReaderMaxNumHatSwitch; i++)
	{
		if(hatSwitch[i].exist!=0)
		{
			IOHIDDeviceGetValue(platformDependentInfoPtr->hidDev,hatSwitch[i].platformDependentInfoPtr->elem,&valueRef);

			double scaled=IOHIDValueGetScaledValue(valueRef,kIOHIDValueScaleTypePhysical);
			if(scaled<-0.001 || 359.999<scaled)
			{
				hatSwitch[i].value=0;
			}
			else
			{
				hatSwitch[i].value=1+(int)((scaled+22.5)/45.0);
			}
		}
	}
}

static void ReleaseInterface(YsJoyReader &reader)
{
	if(reader.platformDependentInfoPtr->hidDev!=NULL)
	{
		// Honestly, I don't know what to do.
		//
		// Should I do
		//   CFRelease(platformDependentInfoPtr->hidDev);
		// ?
		//
		// This hidDev was copied from a copy of IOHIDManager's device list.
		// Who owns it?  Why did I have to make a copy?
		// 
		// The Creare Rule implies that I have the ownership.
		// http://developer.apple.com/mac/library/documentation/CoreFoundation/Conceptual/CFMemoryMgmt/Concepts/Ownership.html#//apple_ref/doc/uid/20001148-SW1
		//
		// Then, I suppose I should release it.  Am I right?
		CFRelease(reader.platformDependentInfoPtr->hidDev);
		reader.platformDependentInfoPtr->hidDev=nullptr;
	}
}

void CFSetCopyCallBack(const void *value,void *context)
{
	CFArrayAppendValue((CFMutableArrayRef)context,value);
}

int YsJoyReader::WriteCalibInfoFile(FILE *fp) const
{
	int i;
	fprintf(fp,"BGNJOY %d\n",joyId);
	for(i=0; i<YsJoyReaderMaxNumAxis; i++)
	{
		if(0!=axis[i].exist)
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
	nJoystick=0;

	// https://stackoverflow.com/questions/58670785/which-api-is-behind-the-privacy-feature-input-monitoring-in-the-security-syste
	if(kIOHIDAccessTypeGranted!=IOHIDCheckAccess(kIOHIDRequestTypeListenEvent))
	{
		if(true!=requestIssued)
		{
			IOHIDRequestAccess(kIOHIDRequestTypeListenEvent);
			requestIssued=true;
		}
		return 0;
	}

	if(NULL==YsJoyReader::PlatformDependentInfo::hidManager)
	{
		YsJoyReader::PlatformDependentInfo::hidManager=IOHIDManagerCreate(kCFAllocatorDefault,kIOHIDOptionsTypeNone);
	}

	if(NULL!=YsJoyReader::PlatformDependentInfo::hidManager)
	{
		IOHIDManagerSetDeviceMatching(YsJoyReader::PlatformDependentInfo::hidManager,NULL);  // Just enumrate all devices
		IOHIDManagerScheduleWithRunLoop(YsJoyReader::PlatformDependentInfo::hidManager,CFRunLoopGetMain(),kCFRunLoopDefaultMode);
		IOHIDManagerOpen(YsJoyReader::PlatformDependentInfo::hidManager,kIOHIDOptionsTypeNone);

		CFSetRef copyOfDevices=IOHIDManagerCopyDevices(YsJoyReader::PlatformDependentInfo::hidManager);
		if(NULL!=YsJoyReader::PlatformDependentInfo::devArray)
		{
			CFRelease(YsJoyReader::PlatformDependentInfo::devArray);
			YsJoyReader::PlatformDependentInfo::devArray=nullptr;
		}
		YsJoyReader::PlatformDependentInfo::devArray=CFArrayCreateMutable(kCFAllocatorDefault,0,&kCFTypeArrayCallBacks);
		CFSetApplyFunction(copyOfDevices,CFSetCopyCallBack,(void *)YsJoyReader::PlatformDependentInfo::devArray);

		CFIndex nDev=CFArrayGetCount(YsJoyReader::PlatformDependentInfo::devArray);

		printf("%d devices found\n",(int)nDev);

		CFRelease(copyOfDevices);



		if(0<nDev)
		{
			int i;
			class LocIdHidDevPair
			{
			public:
				IOHIDDeviceRef hidDev;
				long locId;
			};
			LocIdHidDevPair *sorter=new LocIdHidDevPair [nDev];

			for(i=0; i<nDev; i++)
			{
				sorter[i].hidDev=(IOHIDDeviceRef)CFArrayGetValueAtIndex(YsJoyReader::PlatformDependentInfo::devArray,i);
				sorter[i].locId=0;
				IOHIDDeviceScheduleWithRunLoop(sorter[i].hidDev,CFRunLoopGetMain(),kCFRunLoopDefaultMode);

				CFTypeRef locIdRef=IOHIDDeviceGetProperty(sorter[i].hidDev,CFSTR(kIOHIDLocationIDKey));
				if(NULL!=locIdRef && CFNumberGetTypeID()==CFGetTypeID(locIdRef))
				{
					CFNumberGetValue((CFNumberRef)locIdRef,kCFNumberSInt32Type,&sorter[i].locId);
					printf("Location ID=%08x\n",(int)sorter[i].locId);
				}
				else
				{
					printf("Cannot acquire location ID.\n");
				}
			}

			// Cutting corner.  I could use Quick Sort.
			// But, it is unlikely there are thousands of HID devices connected to
			// the computer.  Bubble sort must be good enough.
			int j;
			for(i=0; i<nDev; i++)
			{
				for(j=i+1; j<nDev; j++)
				{
					if(sorter[j].locId<sorter[i].locId)
					{
						LocIdHidDevPair swp;
						swp=sorter[i];
						sorter[i]=sorter[j];
						sorter[j]=swp;
					}
				}
			}

			for(i=0; i<nDev && nJoystick<maxNumJoystick; i++)
			{
				YsJoyReader::SetUpInfo info;
				info.hidDev=sorter[i].hidDev;
				if(joystick[nJoystick].SetUp(nJoystick,info)!=0)
				{
					nJoystick++;
					// CFRelease(platformDependentInfoPtr->hidDev);  // Doesn't it destroy integrity of devArray?
				}
			}

			delete [] sorter;
		}
	}

	return nJoystick;
}


extern "C" FILE *YsJoyReaderOpenJoystickCalibrationFileC(const char mode[]);

FILE *YsJoyReaderOpenJoystickCalibrationFile(const char mode[])
{
  return YsJoyReaderOpenJoystickCalibrationFileC(mode);
}

