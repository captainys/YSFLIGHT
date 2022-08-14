#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#include <dlfcn.h>


#include <ysclass.h>
#include <ysport.h>
#include <fsairsound.h>
#include <fsfilename.h>



////////////////////////////////////////////////////////////

extern "C" 
{
void (*FsSoundDllInitialize)(void)=NULL;
void (*FsSoundDllTerminate)(void)=NULL;

void (*FsSoundDllSetMasterSwitch)(YSBOOL sw)=NULL;
void (*FsSoundDllSetEnvironmentalSwitch)(YSBOOL sw)=NULL;
void (*FsSoundDllSetOneTimeSwitch)(YSBOOL sw)=NULL;

void (*FsSoundDllStopAll)(void)=NULL;
void (*FsSoundDllSetVehicleName)(const char vehicleName[])=NULL;
void (*FsSoundDllSetEngine)(FSSND_ENGINETYPE engineType,int numEngine,const double power)=NULL;
void (*FsSoundDllSetMachineGun)(FSSND_MACHINEGUNTYPE machineGunType)=NULL;
void (*FsSoundDllSetAlarm)(FSSND_ALARMTYPE alarmType)=NULL;
void (*FsSoundDllSetOneTime)(FSSND_ONETIMETYPE oneTimeType)=NULL;
void (*FsSoundDllKeepPlaying)(void)=NULL;
}

static void *FsSndDllPtr=NULL;

extern "C"
{
void (*FsVoiceDllInitialize)(void)=NULL;
void (*FsVoiceDllTerminate)(void)=NULL;
void (*FsVoiceDllSetMasterSwitch)(YSBOOL sw)=NULL;
void (*FsVoiceDllStopAll)(void)=NULL;
void (*FsVoiceDllSpeak)(int nVoicePhrase,const struct FsVoicePhrase voicePhrase[])=NULL;
void (*FsVoiceDllKeepSpeaking)(void)=NULL;
}

static void *FsVoiceDllPtr=NULL;

void FsSoundInitialize(void)
{
	if(NULL==FsSndDllPtr)
	{
		const wchar_t *sndDllFn=FsGetSoundDllFile();

		YsString utf8;
		utf8.EncodeUTF8 <wchar_t> (sndDllFn);
		YsPrintf("Loading Sound Plug-In. (%s)\n",utf8.Txt());


		FsSndDllPtr=dlopen(utf8,RTLD_LAZY);
		if(NULL!=FsSndDllPtr)
		{
			YsPrintf("Sound Plug-In Loaded.\n");

			FsSoundDllInitialize=(void (*)(void))dlsym(FsSndDllPtr,"FsSoundDllInitialize");

			FsSoundDllTerminate=(void (*)(void))dlsym(FsSndDllPtr,"FsSoundDllTerminate");

			FsSoundDllSetMasterSwitch=(void (*)(YSBOOL sw))dlsym(FsSndDllPtr,"FsSoundDllSetMasterSwitch");
			FsSoundDllSetEnvironmentalSwitch=(void (*)(YSBOOL sw))dlsym(FsSndDllPtr,"FsSoundDllSetEnvironmentalSwitch");
			FsSoundDllSetOneTimeSwitch=(void (*)(YSBOOL sw))dlsym(FsSndDllPtr,"FsSoundDllSetOneTimeSwitch");

			FsSoundDllStopAll=(void (*)(void))dlsym(FsSndDllPtr,"FsSoundDllStopAll");

			FsSoundDllSetVehicleName=(void (*)(const char vehicleName[]))dlsym(FsSndDllPtr,"FsSoundDllSetVehicleName");
			FsSoundDllSetEngine=(void (*)(FSSND_ENGINETYPE,int,const double))dlsym(FsSndDllPtr,"FsSoundDllSetEngine");

			FsSoundDllSetMachineGun=(void (*)(FSSND_MACHINEGUNTYPE machineGunType))dlsym(FsSndDllPtr,"FsSoundDllSetMachineGun");
			FsSoundDllSetAlarm=(void (*)(FSSND_ALARMTYPE alarmType))dlsym(FsSndDllPtr,"FsSoundDllSetAlarm");
			FsSoundDllSetOneTime=(void (*)(FSSND_ONETIMETYPE oneTimeType))dlsym(FsSndDllPtr,"FsSoundDllSetOneTime");
			FsSoundDllKeepPlaying=(void (*)(void))dlsym(FsSndDllPtr,"FsSoundDllKeepPlaying");

			if(NULL!=FsSoundDllInitialize)
			{
				(*FsSoundDllInitialize)();
			}
		}
	}


	if(NULL==FsVoiceDllPtr)
	{
		const wchar_t *voiceDllFn=FsGetVoiceDllFile();
		YsString cVoiceDllFn;
		YsUnicodeToSystemEncoding(cVoiceDllFn,voiceDllFn);

		YsPrintf("Loading Voice Plug-In. (%s)\n",cVoiceDllFn.Txt());


		FsVoiceDllPtr=dlopen(cVoiceDllFn.Txt(),RTLD_LAZY);
		if(NULL!=FsVoiceDllPtr)
		{
			YsPrintf("Voice Plug-In Loaded.\n");

			FsVoiceDllInitialize=(void (*)(void))dlsym(FsVoiceDllPtr,"FsVoiceDllInitialize");
			FsVoiceDllTerminate=(void (*)(void))dlsym(FsVoiceDllPtr,"FsVoiceDllTerminate");
			FsVoiceDllSetMasterSwitch=(void (*)(YSBOOL))dlsym(FsVoiceDllPtr,"FsVoiceDllSetMasterSwitch");
			FsVoiceDllStopAll=(void (*)(void))dlsym(FsVoiceDllPtr,"FsVoiceDllStopAll");
			FsVoiceDllSpeak=(void (*)(int,const struct FsVoicePhrase []))dlsym(FsVoiceDllPtr,"FsVoiceDllSpeak");
			FsVoiceDllKeepSpeaking=(void (*)(void))dlsym(FsVoiceDllPtr,"FsVoiceDllKeepSpeaking");

			if(NULL!=FsVoiceDllInitialize)
			{
				(*FsVoiceDllInitialize)();
			}
		}
	}
}

void FsSoundSetMasterSwitch(YSBOOL sw)
{
	if(NULL!=FsSoundDllSetMasterSwitch)
	{
		(*FsSoundDllSetMasterSwitch)(sw);
	}
	if(NULL!=FsVoiceDllSetMasterSwitch)
	{
		(*FsVoiceDllSetMasterSwitch)(sw);
	}
}

void FsSoundSetEnvironmentalSwitch(YSBOOL sw)
{
	if(NULL!=FsSoundDllSetEnvironmentalSwitch)
	{
		(*FsSoundDllSetEnvironmentalSwitch)(sw);
	}
}

void FsSoundSetOneTimeSwitch(YSBOOL sw)
{
	if(NULL!=FsSoundDllSetOneTimeSwitch)
	{
		(*FsSoundDllSetOneTimeSwitch)(sw);
	}
}

void FsSoundStopAll(void)
{
	if(NULL!=FsSoundDllStopAll)
	{
		(*FsSoundDllStopAll)();
	}
}

void FsSoundSetVehicleName(const char vehicleName[])
{
	if(NULL!=FsSoundDllSetVehicleName)
	{
		(*FsSoundDllSetVehicleName)(vehicleName);
	}
}

void FsSoundSetEngine(FSSND_ENGINETYPE engineType,int numEngine,const double power)
{
	if(NULL!=FsSoundDllSetEngine)
	{
		(*FsSoundDllSetEngine)(engineType,numEngine,power);
	}
}

void FsSoundSetMachineGun(FSSND_MACHINEGUNTYPE machineGunType)
{
	if(NULL!=FsSoundDllSetMachineGun)
	{
		(*FsSoundDllSetMachineGun)(machineGunType);
	}
}

void FsSoundSetAlarm(FSSND_ALARMTYPE alarmType)
{
	if(NULL!=FsSoundDllSetAlarm)
	{
		(*FsSoundDllSetAlarm)(alarmType);
	}
}

void FsSoundSetOneTime(FSSND_ONETIMETYPE oneTimeType)
{
	if(NULL!=FsSoundDllSetOneTime)
	{
		(*FsSoundDllSetOneTime)(oneTimeType);
	}
}

void FsSoundKeepPlaying(void)
{
	if(NULL!=FsSoundDllKeepPlaying)
	{
		(*FsSoundDllKeepPlaying)();
	}
}

void FsSoundTerminate(void)
{
	FsSoundStopAll();

	if(NULL!=FsSoundDllTerminate)
	{
		(*FsSoundDllTerminate)();
	}

	if(NULL!=FsSndDllPtr)
	{
		dlclose(FsSndDllPtr);
		FsSndDllPtr=NULL;
	}

	FsSoundDllInitialize=NULL;
	FsSoundDllTerminate=NULL;

	FsSoundDllSetMasterSwitch=NULL;
	FsSoundDllSetEnvironmentalSwitch=NULL;
	FsSoundDllSetOneTimeSwitch=NULL;

	FsSoundDllStopAll=NULL;
	FsSoundDllSetVehicleName=NULL;
	FsSoundDllSetEngine=NULL;
	FsSoundDllSetMachineGun=NULL;
	FsSoundDllSetAlarm=NULL;
	FsSoundDllSetOneTime=NULL;
	FsSoundDllKeepPlaying=NULL;



	FsVoiceStopAll();
	if(NULL!=FsVoiceDllTerminate)
	{
		(*FsVoiceDllTerminate)();
	}

	if(NULL!=FsVoiceDllPtr)
	{
		dlclose(FsVoiceDllPtr);
		FsVoiceDllPtr=NULL;
	}

	FsVoiceDllInitialize=NULL;
	FsVoiceDllTerminate=NULL;
	FsVoiceDllSetMasterSwitch=NULL;
	FsVoiceDllStopAll=NULL;
	FsVoiceDllSpeak=NULL;
	FsVoiceDllKeepSpeaking=NULL;
}


////////////////////////////////////////////////////////////


void FsVoiceStopAll(void)
{
	if(NULL!=FsVoiceDllStopAll)
	{
		(*FsVoiceDllStopAll)();
	}
}

void FsVoiceSpeak(int nVoicePhrase,const struct FsVoicePhrase voicePhrase[])
{
	if(NULL!=FsVoiceDllSpeak)
	{
		(*FsVoiceDllSpeak)(nVoicePhrase,voicePhrase);
	}
}

void FsVoiceKeepSpeaking(void)
{
	if(NULL!=FsVoiceDllKeepSpeaking)
	{
		(*FsVoiceDllKeepSpeaking)();
	}
}

