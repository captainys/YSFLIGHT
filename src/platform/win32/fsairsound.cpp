#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#define _WINSOCKAPI_
#include <windows.h>
#include <mmsystem.h>


#include <ysclass.h>
#include <fsairsound.h>
#include <fsfilename.h>



void (__cdecl *FsSoundDllInitialize)(HWND hWndMain)=NULL;
void (__cdecl *FsSoundDllTerminate)(void)=NULL;

void (__cdecl *FsSoundDllSetMasterSwitch)(YSBOOL sw)=NULL;
void (__cdecl *FsSoundDllSetEnvironmentalSwitch)(YSBOOL sw)=NULL;
void (__cdecl *FsSoundDllSetOneTimeSwitch)(YSBOOL sw)=NULL;

void (__cdecl *FsSoundDllStopAll)(void)=NULL;
void (__cdecl *FsSoundDllSetVehicleName)(const char vehicleName[])=NULL;
void (__cdecl *FsSoundDllSetEngine)(FSSND_ENGINETYPE engineType,int numEngine,const double power)=NULL;
void (__cdecl *FsSoundDllSetMachineGun)(FSSND_MACHINEGUNTYPE machineGunType)=NULL;
void (__cdecl *FsSoundDllSetAlarm)(FSSND_ALARMTYPE alarmType)=NULL;
void (__cdecl *FsSoundDllSetOneTime)(FSSND_ONETIMETYPE oneTimeType)=NULL;
void (__cdecl *FsSoundDllKeepPlaying)(void)=NULL;

static HMODULE hSndDll=NULL;



void (__cdecl *FsVoiceDllInitialize)(HWND)=NULL;
void (__cdecl *FsVoiceDllTerminate)(void)=NULL;
void (__cdecl *FsVoiceDllSetMasterSwitch)(YSBOOL)=NULL;
void (__cdecl *FsVoiceDllStopAll)(void)=NULL;
void (__cdecl *FsVoiceDllSpeak)(int,const struct FsVoicePhrase [])=NULL;
void (__cdecl *FsVoiceDllKeepSpeaking)(void)=NULL;

static HMODULE hVoiceDll=NULL;



extern HWND FsWin32GetMainWindowHandle(void);


void FsSoundInitialize(void)
{
	if(NULL==hSndDll)
	{
		const wchar_t *sndDllFn=FsGetSoundDllFile();

		YsPrintf("Loading Sound Plug-In.\n");

		hSndDll=LoadLibraryExW(sndDllFn,NULL,0);
		if(NULL!=hSndDll)
		{
			YsPrintf("Sound Plug-In Loaded.\n");

			FsSoundDllInitialize=(void (__cdecl *)(HWND))GetProcAddress(hSndDll,"FsSoundDllInitialize");
			FsSoundDllTerminate=(void (__cdecl *)(void))GetProcAddress(hSndDll,"FsSoundDllTerminate");

			FsSoundDllSetMasterSwitch=(void (__cdecl *)(YSBOOL sw))GetProcAddress(hSndDll,"FsSoundDllSetMasterSwitch");
			FsSoundDllSetEnvironmentalSwitch=(void (__cdecl *)(YSBOOL sw))GetProcAddress(hSndDll,"FsSoundDllSetEnvironmentalSwitch");
			FsSoundDllSetOneTimeSwitch=(void (__cdecl *)(YSBOOL sw))GetProcAddress(hSndDll,"FsSoundDllSetOneTimeSwitch");

			FsSoundDllStopAll=(void (__cdecl *)(void))GetProcAddress(hSndDll,"FsSoundDllStopAll");

			FsSoundDllSetVehicleName=(void (__cdecl *)(const char vehicleName[]))GetProcAddress(hSndDll,"FsSoundDllSetVehicleName");
			FsSoundDllSetEngine=(void (__cdecl *)(FSSND_ENGINETYPE,int,const double))GetProcAddress(hSndDll,"FsSoundDllSetEngine");

			FsSoundDllSetMachineGun=(void (__cdecl *)(FSSND_MACHINEGUNTYPE machineGunType))GetProcAddress(hSndDll,"FsSoundDllSetMachineGun");
			FsSoundDllSetAlarm=(void (__cdecl *)(FSSND_ALARMTYPE alarmType))GetProcAddress(hSndDll,"FsSoundDllSetAlarm");
			FsSoundDllSetOneTime=(void (__cdecl *)(FSSND_ONETIMETYPE oneTimeType))GetProcAddress(hSndDll,"FsSoundDllSetOneTime");
			FsSoundDllKeepPlaying=(void (__cdecl *)(void))GetProcAddress(hSndDll,"FsSoundDllKeepPlaying");

			if(NULL!=FsSoundDllInitialize)
			{
				(*FsSoundDllInitialize)(FsWin32GetMainWindowHandle());
			}
		}
		else
		{
			DWORD err=GetLastError();
			printf("Could not load Sound Plug-In : GetLastError=%d\n",err);
		}
	}

	if(NULL==hVoiceDll)
	{
		const wchar_t *voiceDllFn=FsGetVoiceDllFile();

		YsPrintf("Loading Voice Plug-In.\n");

		hVoiceDll=LoadLibraryExW(voiceDllFn,NULL,0);
		if(NULL!=hVoiceDll)
		{
			YsPrintf("Voice Plug-In Loaded.\n");

			FsVoiceDllInitialize=(void (__cdecl *)(HWND))GetProcAddress(hVoiceDll,"FsVoiceDllInitialize");
			FsVoiceDllTerminate=(void (__cdecl *)(void))GetProcAddress(hVoiceDll,"FsVoiceDllTerminate");
			FsVoiceDllSetMasterSwitch=(void (__cdecl *)(YSBOOL))GetProcAddress(hVoiceDll,"FsVoiceDllSetMasterSwitch");
			FsVoiceDllStopAll=(void (__cdecl *)(void))GetProcAddress(hVoiceDll,"FsVoiceDllStopAll");
			FsVoiceDllSpeak=(void (__cdecl *)(int,const struct FsVoicePhrase []))GetProcAddress(hVoiceDll,"FsVoiceDllSpeak");
			FsVoiceDllKeepSpeaking=(void (__cdecl *)(void))GetProcAddress(hVoiceDll,"FsVoiceDllKeepSpeaking");

			if(NULL!=FsVoiceDllInitialize &&
			   NULL!=FsVoiceDllTerminate &&
			   NULL!=FsVoiceDllSetMasterSwitch &&
			   NULL!=FsVoiceDllStopAll &&
			   NULL!=FsVoiceDllSpeak &&
			   NULL!=FsVoiceDllKeepSpeaking)
			{
				printf("All functions from Voice Dll are ready.\n");
			}

			if(NULL!=FsVoiceDllInitialize)
			{
				(*FsVoiceDllInitialize)(FsWin32GetMainWindowHandle());
			}
		}
		else
		{
			DWORD err=GetLastError();
			printf("Could not load Voice Plug-In : GetLastError=%d\n",err);
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

	if(NULL!=hSndDll)
	{
		FreeLibrary(hSndDll);
		hSndDll=NULL;
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

	if(NULL!=hVoiceDll)
	{
		FreeLibrary(hVoiceDll);
		hVoiceDll=NULL;
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


