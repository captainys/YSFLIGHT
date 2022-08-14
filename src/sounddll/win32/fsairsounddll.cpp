#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#define _WINSOCKAPI_
#include <windows.h>
#include <mmsystem.h>


#include "../fsairsound.h"


class FsSoundStatus
{
public:
	const char *vehicleName;

	FSSND_ENGINETYPE engineType;
	int numEngine;
	double enginePower;

	FSSND_MACHINEGUNTYPE machineGunType;
	FSSND_ALARMTYPE alarmType;
	FSSND_ONETIMETYPE oneTimeType;

	FsSoundStatus();
	void Initialize(void);
};


FsSoundStatus::FsSoundStatus()
{
	Initialize();
}

void FsSoundStatus::Initialize(void)
{
	vehicleName="";

	engineType=FSSND_ENGINE_SILENT;
	numEngine=0;
	enginePower=0.0;

	machineGunType=FSSND_MACHINEGUN_SILENT;
	alarmType=FSSND_ALARM_SILENT;
	oneTimeType=FSSND_ONETIME_SILENT;
}


////////////////////////////////////////////////////////////


class FsWaveSound
{
protected:
	unsigned nDat;
	unsigned char *dat;
	int headerSize;

#ifdef WIN32
	WAVEFORMATEX fmt;
	WAVEHDR header;
	HWAVEOUT handle;
	YSBOOL isPlaying;
#endif

public:
	FsWaveSound();
	FsWaveSound(char fn[]);
	~FsWaveSound();
	YSRESULT Load(char fn[]);
	YSRESULT Free(void);
	YSRESULT Play(int loop,int volume);
	YSRESULT Stop(void);
	YSBOOL IsPlaying(void);
};


FsWaveSound::FsWaveSound()
{
	nDat=0;
	dat=NULL;
	isPlaying=YSFALSE;
}

FsWaveSound::FsWaveSound(char fn[])
{
	FsWaveSound();
	Load(fn);
}

FsWaveSound::~FsWaveSound()
{
	Free();
}

YSRESULT FsWaveSound::Load(char fn[])
{
	FILE *fp;
	char buf[256];

	fp=fopen(fn,"rb");
	if(fp!=NULL)
	{
		int i;
		unsigned long fileSize;

		fread(buf,4,1,fp);  // Skip "RIFF"
		fread(&fileSize,4,1,fp);

		fread(buf,1,8,fp);
		for(i=0; i<8; i++)
		{
			if('a'<=buf[i] && buf[i]<='z')
			{
				buf[i]=buf[i]+'A'-'a';
			}
		}
		if(strncmp(buf,"WAVEFMT ",8)!=0)
		{
			fclose(fp);
			return YSERR;
		}

		fread(&headerSize,4,1,fp);  // Skip Header Length
		fread(&fmt,1,headerSize,fp);

		fread(buf,1,4,fp);
		if(strncmp(buf,"fact",4)==0)
		{
			// What is fact?
			int factSize;
			fread(&factSize,4,1,fp);
			fread(buf,1,factSize,fp);
			fread(buf,1,4,fp);  // Skip "data"
		}

		fread(&nDat,4,1,fp);

		dat=new unsigned char[nDat];
		if(dat!=NULL)
		{
			fread(dat,1,nDat,fp);
		}

		fclose(fp);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsWaveSound::Free(void)
{
	if(isPlaying==YSTRUE)
	{
		Stop();
	}
	if(nDat>0 && dat!=NULL)
	{
		delete dat;
		nDat=0;
		dat=NULL;
	}
	return YSOK;
}

YSRESULT FsWaveSound::Play(int loop,int volume)
{
	if(nDat>0 && dat!=NULL && isPlaying==YSFALSE)
	{
		MMRESULT res;

#if(WINVER >= 0x0400)  // VC++4.0
		res=waveOutOpen
		    (&handle,WAVE_MAPPER,(LPWAVEFORMATEX)&fmt,NULL,NULL,WAVE_ALLOWSYNC);
		header.lpData=(char *)dat;
#else                  // VC++2.0
		res=waveOutOpen
		    (&handle,WAVE_MAPPER,(LPWAVEFORMAT)&fmt,NULL,NULL,WAVE_ALLOWSYNC);
		header.lpData=dat;
#endif
		if(res==MMSYSERR_NOERROR)
		{
			header.dwBufferLength=nDat;
			header.dwBytesRecorded=nDat;
			header.dwUser=0;
			header.dwFlags=WHDR_BEGINLOOP|WHDR_ENDLOOP;
			header.dwLoops=loop;
			header.lpNext=NULL;

			if(waveOutPrepareHeader(handle,&header,sizeof(header))==MMSYSERR_NOERROR)
			{
				volume=volume*0x10000+volume;
/*
#if(WINVER>=0x0400)  //VC++4.0
				waveOutSetVolume(handle,volume);
#else                //VC++2.0
				unsigned int id;
				waveOutGetID(handle,&id);
				waveOutSetVolume(id,volume);
#endif
*/

				waveOutWrite(handle,&header,sizeof(header));
				isPlaying=YSTRUE;
			}
		}
	}
	return YSOK;
}

YSRESULT FsWaveSound::Stop(void)
{
	if(isPlaying==YSTRUE)
	{
		waveOutReset(handle);
		waveOutUnprepareHeader(handle,&header,sizeof(header));
		waveOutClose(handle);
		isPlaying=YSFALSE;
	}
	return YSOK;
}

YSBOOL FsWaveSound::IsPlaying(void)
{
	if(isPlaying==YSTRUE)
	{
		MMTIME t;
		t.wType=TIME_BYTES;
		waveOutGetPosition(handle,&t,sizeof(t));
		if(t.u.cb>=nDat)
		{
			Stop();
			return YSFALSE;
		}
		return YSTRUE;
	}
	return YSFALSE;
}

////////////////////////////////////////////////////////////

static FsWaveSound *isPlaying=NULL;

static void FsPlayWaveSound(FsWaveSound *snd,int loop,int volume)
{
	if(NULL!=isPlaying && isPlaying!=snd)
	{
		isPlaying->Stop();
		isPlaying=NULL;
	}

	if(NULL!=snd)
	{
		snd->Play(loop,volume);
		isPlaying=snd;
	}
}

////////////////////////////////////////////////////////////

static YSBOOL fsSoundMasterSwitch=YSTRUE;
static YSBOOL fsSoundEnvironmentalSwitch=YSTRUE;
static YSBOOL fsSoundOneTimeSwitch=YSTRUE;

static FsWaveSound jetWav[10],afterBurnerWav,propWav[10];
static FsWaveSound machineGunWav[FSSND_NUM_MACHINEGUNTYPE];
static FsWaveSound alarmWav[FSSND_NUM_ALARMTYPE];
static FsWaveSound oneTimeWav[FSSND_NUM_ONETIMETYPE];

static FsSoundStatus sndStatus;

extern "C" 
{

__declspec(dllexport) void FsSoundDllInitialize(HWND hWndMain)
{
	jetWav[0].Load("sound/engine0.wav");
	jetWav[1].Load("sound/engine1.wav");
	jetWav[2].Load("sound/engine2.wav");
	jetWav[3].Load("sound/engine3.wav");
	jetWav[4].Load("sound/engine4.wav");
	jetWav[5].Load("sound/engine5.wav");
	jetWav[6].Load("sound/engine6.wav");
	jetWav[7].Load("sound/engine7.wav");
	jetWav[8].Load("sound/engine8.wav");
	jetWav[9].Load("sound/engine9.wav");

	propWav[0].Load("sound/prop0.wav");
	propWav[1].Load("sound/prop1.wav");
	propWav[2].Load("sound/prop2.wav");
	propWav[3].Load("sound/prop3.wav");
	propWav[4].Load("sound/prop4.wav");
	propWav[5].Load("sound/prop5.wav");
	propWav[6].Load("sound/prop6.wav");
	propWav[7].Load("sound/prop7.wav");
	propWav[8].Load("sound/prop8.wav");
	propWav[9].Load("sound/prop9.wav");

	afterBurnerWav.Load("sound/burner.wav");

	machineGunWav[(int)FSSND_MACHINEGUN_MACHINEGUN].Load("sound/gun.wav");

	alarmWav[(int)FSSND_ALARM_STALL].Load("sound/stallhorn.wav");
	alarmWav[(int)FSSND_ALARM_MISSILE].Load("sound/warning.wav");
	alarmWav[(int)FSSND_ALARM_TERRAIN].Load("sound/gearhorn.wav");

	oneTimeWav[(int)FSSND_ONETIME_DAMAGE].Load("sound/damage.wav");
	oneTimeWav[(int)FSSND_ONETIME_MISSILE].Load("sound/missile.wav");
	oneTimeWav[(int)FSSND_ONETIME_BANG].Load("sound/bang.wav");
	oneTimeWav[(int)FSSND_ONETIME_BLAST].Load("sound/blast.wav");
	oneTimeWav[(int)FSSND_ONETIME_TOUCHDWN].Load("sound/touchdwn.wav");
	oneTimeWav[(int)FSSND_ONETIME_HIT].Load("sound/hit.wav");
	oneTimeWav[(int)FSSND_ONETIME_BLAST2].Load("sound/blast2.wav");
	oneTimeWav[(int)FSSND_ONETIME_GEARUP].Load("sound/retractldg.wav");
	oneTimeWav[(int)FSSND_ONETIME_GEARDOWN].Load("sound/extendldg.wav");
	oneTimeWav[(int)FSSND_ONETIME_BOMBSAWAY].Load("sound/bombsaway.wav");
	oneTimeWav[(int)FSSND_ONETIME_ROCKET].Load("sound/rocket.wav");
	oneTimeWav[(int)FSSND_ONETIME_NOTICE].Load("sound/notice.wav");

	sndStatus.Initialize();
}

__declspec(dllexport) void FsSoundDllTerminate(void)
{
	jetWav[0].Free();
	jetWav[1].Free();
	jetWav[2].Free();
	jetWav[3].Free();
	jetWav[4].Free();
	jetWav[5].Free();
	jetWav[6].Free();
	jetWav[7].Free();
	jetWav[8].Free();
	jetWav[9].Free();

	propWav[0].Free();
	propWav[1].Free();
	propWav[2].Free();
	propWav[3].Free();
	propWav[4].Free();
	propWav[5].Free();
	propWav[6].Free();
	propWav[7].Free();
	propWav[8].Free();
	propWav[9].Free();

	afterBurnerWav.Free();

	machineGunWav[(int)FSSND_MACHINEGUN_MACHINEGUN].Free();

	alarmWav[(int)FSSND_ALARM_STALL].Free();
	alarmWav[(int)FSSND_ALARM_MISSILE].Free();
	alarmWav[(int)FSSND_ALARM_TERRAIN].Free();

	oneTimeWav[(int)FSSND_ONETIME_DAMAGE].Free();
	oneTimeWav[(int)FSSND_ONETIME_MISSILE].Free();
	oneTimeWav[(int)FSSND_ONETIME_BANG].Free();
	oneTimeWav[(int)FSSND_ONETIME_BLAST].Free();
	oneTimeWav[(int)FSSND_ONETIME_TOUCHDWN].Free();
	oneTimeWav[(int)FSSND_ONETIME_HIT].Free();
	oneTimeWav[(int)FSSND_ONETIME_BLAST2].Free();
	oneTimeWav[(int)FSSND_ONETIME_GEARUP].Free();
	oneTimeWav[(int)FSSND_ONETIME_GEARDOWN].Free();
	oneTimeWav[(int)FSSND_ONETIME_BOMBSAWAY].Free();
	oneTimeWav[(int)FSSND_ONETIME_ROCKET].Free();

	sndStatus.Initialize();
}

__declspec(dllexport) void FsSoundDllSetMasterSwitch(YSBOOL sw)
{
	fsSoundMasterSwitch=sw;
}

__declspec(dllexport) void FsSoundDllSetEnvironmentalSwitch(YSBOOL sw)
{
	fsSoundEnvironmentalSwitch=sw;
}

__declspec(dllexport) void FsSoundDllSetOneTimeSwitch(YSBOOL sw)
{
	fsSoundOneTimeSwitch=sw;
}

__declspec(dllexport) void FsSoundDllStopAll(void)
{
	FsPlayWaveSound(NULL,0,0);
}

__declspec(dllexport) void FsSoundDllSetVehicleName(const char vehicleName[])
{
	sndStatus.vehicleName=vehicleName;
}

__declspec(dllexport) void FsSoundDllSetEngine(FSSND_ENGINETYPE engineType,int numEngine,const double power)
{
	sndStatus.engineType=engineType;
	sndStatus.numEngine=numEngine;
	sndStatus.enginePower=power;
}

__declspec(dllexport) void FsSoundDllSetMachineGun(FSSND_MACHINEGUNTYPE machineGunType)
{
	sndStatus.machineGunType=machineGunType;
}

__declspec(dllexport) void FsSoundDllSetAlarm(FSSND_ALARMTYPE alarmType)
{
	sndStatus.alarmType=alarmType;
}

__declspec(dllexport) void FsSoundDllSetOneTime(FSSND_ONETIMETYPE oneTimeType)
{
	if(YSTRUE==fsSoundMasterSwitch && YSTRUE==fsSoundOneTimeSwitch)
	{
		FsPlayWaveSound(NULL,0,0);
		if(FSSND_ONETIME_SILENT!=oneTimeType)
		{
			FsPlayWaveSound(&oneTimeWav[(int)oneTimeType],1,32768);
		}
		sndStatus.oneTimeType=oneTimeType;
	}
}

__declspec(dllexport) void FsSoundDllKeepPlaying(void)
{
	if(YSTRUE==fsSoundMasterSwitch && YSTRUE==fsSoundEnvironmentalSwitch)
	{
		if(FSSND_ONETIME_SILENT!=sndStatus.oneTimeType && NULL!=isPlaying && YSTRUE!=isPlaying->IsPlaying())
		{
			FsPlayWaveSound(NULL,0,0);
			sndStatus.oneTimeType=FSSND_ONETIME_SILENT;
		}

		if(FSSND_ONETIME_SILENT==sndStatus.oneTimeType)
		{
			if(FSSND_MACHINEGUN_SILENT!=sndStatus.machineGunType)
			{
				if(isPlaying!=&machineGunWav[(int)sndStatus.machineGunType])
				{
					FsPlayWaveSound(&machineGunWav[(int)sndStatus.machineGunType],10000,32768);
				}
			}
			else if(FSSND_ALARM_SILENT!=sndStatus.alarmType)
			{
				if(isPlaying!=&alarmWav[(int)sndStatus.alarmType])
				{
					FsPlayWaveSound(&alarmWav[(int)sndStatus.alarmType],10000,32768);
				}
			}
			else if(FSSND_ENGINE_SILENT!=sndStatus.engineType)
			{
				FsWaveSound *wavToPlay=NULL;
				int level=(int)(sndStatus.enginePower*10.0);

				if(level<0)
				{
					level=0;
				}
				else if(9<level)
				{
					level=9;
				}

				switch(sndStatus.engineType)
				{
				case FSSND_ENGINE_SILENT:
				case FSSND_ENGINE_CAR:
				case FSSND_ENGINE_SHIP:
					break;
				case FSSND_ENGINE_JETNORMAL:
					wavToPlay=&jetWav[level];
					break;
				case FSSND_ENGINE_JETAFTERBURNER:
					wavToPlay=&afterBurnerWav;
					break;
				case FSSND_ENGINE_PROPELLER:
				case FSSND_ENGINE_TURBOPROP:
				case FSSND_ENGINE_HELICOPTER:
					wavToPlay=&propWav[level];
					break;
				}

				if(isPlaying!=wavToPlay)
				{
					FsPlayWaveSound(wavToPlay,10000,32768);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	return TRUE;  // Means successfully initialized
}

}

