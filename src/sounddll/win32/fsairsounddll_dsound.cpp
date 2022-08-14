#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#define _WINSOCKAPI_
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>


#include "../../common/fsairsound.h"


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

class FsWaveData
{
public:
	unsigned nDat;
	unsigned char *dat;
	int headerSize;
	WAVEFORMATEX fmt;

	FsWaveData();
	~FsWaveData();
	int Load(const char fn[]);
};

FsWaveData::FsWaveData()
{
	nDat=0;
	dat=NULL;
}

FsWaveData::~FsWaveData()
{
	if(NULL!=dat)
	{
		delete dat;
		dat=NULL;
	}
	nDat=0;
}

int FsWaveData::Load(const char fn[])
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
			return 0;
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
		if(NULL!=dat)
		{
			fread(dat,1,nDat,fp);
			fclose(fp);
			return 1;
		}
	}

	if(NULL!=fp)
	{
		fclose(fp);
	}
	return 0;
}



LPDIRECTSOUNDBUFFER CreateWAVSoundBuffer(LPDIRECTSOUND8 itfc,const char fn[])
// Why the hell direct sound does not have a function to read a wav file?
{
	FsWaveData wav;
	if(0!=wav.Load(fn))
	{
		printf("Wave Loaded\n");

		LPDIRECTSOUNDBUFFER directSoundBuffer;
		DSBUFFERDESC desc;
		desc.dwSize=sizeof(desc);
		desc.dwFlags=DSBCAPS_CTRLVOLUME|DSBCAPS_LOCDEFER;
		desc.dwBufferBytes=wav.nDat;
		desc.dwReserved=0;
		desc.lpwfxFormat=&wav.fmt;
		desc.guid3DAlgorithm=GUID_NULL;
		if(DS_OK==itfc->CreateSoundBuffer(&desc,&directSoundBuffer,NULL))
		{
			DWORD writeBufSize1,writeBufSize2;
			unsigned char *writeBuf1,*writeBuf2;
			if(directSoundBuffer->Lock(0,0,(LPVOID *)&writeBuf1,&writeBufSize1,(LPVOID *)&writeBuf2,&writeBufSize2,DSBLOCK_ENTIREBUFFER)==DS_OK &&
			   NULL!=writeBuf1)
			{
				printf("Buffer Locked\n");
				printf("%08x %d %08x %d\n",writeBuf1,writeBufSize1,writeBuf2,writeBufSize2);

				int i;
				for(i=0; i<(int)wav.nDat && i<(int)writeBufSize1; i++)
				{
					writeBuf1[i]=wav.dat[i];
				}

				directSoundBuffer->Unlock(writeBuf1,writeBufSize1,writeBuf2,writeBufSize2);
				return directSoundBuffer;
			}
			directSoundBuffer->Release();
		}
	}
	return NULL;
}



static LPDIRECTSOUND8 dSound8=NULL;

////////////////////////////////////////////////////////////

static YSBOOL fsSoundMasterSwitch=YSTRUE;
static YSBOOL fsSoundEnvironmentalSwitch=YSTRUE;
static YSBOOL fsSoundOneTimeSwitch=YSTRUE;

static LPDIRECTSOUNDBUFFER jetWav[10],afterBurnerWav,propWav[10];
static LPDIRECTSOUNDBUFFER machineGunWav[FSSND_NUM_MACHINEGUNTYPE];
static LPDIRECTSOUNDBUFFER alarmWav[FSSND_NUM_ALARMTYPE];
static LPDIRECTSOUNDBUFFER oneTimeWav[FSSND_NUM_ONETIMETYPE];

static FsSoundStatus sndStatus;

static void FsSoundNullifyEverything(void)
{
	int i;
	for(i=0; i<10; i++)
	{
		jetWav[i]=NULL;
		propWav[i]=NULL;
	}
	afterBurnerWav=NULL;

	for(i=0; i<FSSND_NUM_MACHINEGUNTYPE; i++)
	{
		machineGunWav[i]=NULL;
	}
	for(i=0; i<FSSND_NUM_ALARMTYPE; i++)
	{
		alarmWav[i]=NULL;
	}
	for(i=0; i<FSSND_NUM_ONETIMETYPE; i++)
	{
		oneTimeWav[i]=NULL;
	}
}

static void FsSoundDllReleaseBuffer(LPDIRECTSOUNDBUFFER &buf)
{
	if(NULL!=buf)
	{
		buf->Release();
		buf=NULL;
	}
}

extern "C" 
{

__declspec(dllexport) void FsSoundDllInitialize(HWND hWndMain)
{
	FsSoundNullifyEverything();

	if(DS_OK==DirectSoundCreate8(NULL,&dSound8,NULL))
	{
		dSound8->SetCooperativeLevel(hWndMain,DSSCL_PRIORITY);

		jetWav[0]=CreateWAVSoundBuffer(dSound8,"sound/engine0.wav");
		jetWav[1]=CreateWAVSoundBuffer(dSound8,"sound/engine1.wav");
		jetWav[2]=CreateWAVSoundBuffer(dSound8,"sound/engine2.wav");
		jetWav[3]=CreateWAVSoundBuffer(dSound8,"sound/engine3.wav");
		jetWav[4]=CreateWAVSoundBuffer(dSound8,"sound/engine4.wav");
		jetWav[5]=CreateWAVSoundBuffer(dSound8,"sound/engine5.wav");
		jetWav[6]=CreateWAVSoundBuffer(dSound8,"sound/engine6.wav");
		jetWav[7]=CreateWAVSoundBuffer(dSound8,"sound/engine7.wav");
		jetWav[8]=CreateWAVSoundBuffer(dSound8,"sound/engine8.wav");
		jetWav[9]=CreateWAVSoundBuffer(dSound8,"sound/engine9.wav");

		propWav[0]=CreateWAVSoundBuffer(dSound8,"sound/prop0.wav");
		propWav[1]=CreateWAVSoundBuffer(dSound8,"sound/prop1.wav");
		propWav[2]=CreateWAVSoundBuffer(dSound8,"sound/prop2.wav");
		propWav[3]=CreateWAVSoundBuffer(dSound8,"sound/prop3.wav");
		propWav[4]=CreateWAVSoundBuffer(dSound8,"sound/prop4.wav");
		propWav[5]=CreateWAVSoundBuffer(dSound8,"sound/prop5.wav");
		propWav[6]=CreateWAVSoundBuffer(dSound8,"sound/prop6.wav");
		propWav[7]=CreateWAVSoundBuffer(dSound8,"sound/prop7.wav");
		propWav[8]=CreateWAVSoundBuffer(dSound8,"sound/prop8.wav");
		propWav[9]=CreateWAVSoundBuffer(dSound8,"sound/prop9.wav");

		afterBurnerWav=CreateWAVSoundBuffer(dSound8,"sound/burner.wav");

		machineGunWav[(int)FSSND_MACHINEGUN_MACHINEGUN]=CreateWAVSoundBuffer(dSound8,"sound/gun.wav");

		alarmWav[(int)FSSND_ALARM_STALL]=CreateWAVSoundBuffer(dSound8,"sound/stallhorn.wav");
		alarmWav[(int)FSSND_ALARM_MISSILE]=CreateWAVSoundBuffer(dSound8,"sound/warning.wav");
		alarmWav[(int)FSSND_ALARM_TERRAIN]=CreateWAVSoundBuffer(dSound8,"sound/gearhorn.wav");

		oneTimeWav[(int)FSSND_ONETIME_DAMAGE]=CreateWAVSoundBuffer(dSound8,"sound/damage.wav");
		oneTimeWav[(int)FSSND_ONETIME_MISSILE]=CreateWAVSoundBuffer(dSound8,"sound/missile.wav");
		oneTimeWav[(int)FSSND_ONETIME_BANG]=CreateWAVSoundBuffer(dSound8,"sound/bang.wav");
		oneTimeWav[(int)FSSND_ONETIME_BLAST]=CreateWAVSoundBuffer(dSound8,"sound/blast.wav");
		oneTimeWav[(int)FSSND_ONETIME_TOUCHDWN]=CreateWAVSoundBuffer(dSound8,"sound/touchdwn.wav");
		oneTimeWav[(int)FSSND_ONETIME_HIT]=CreateWAVSoundBuffer(dSound8,"sound/hit.wav");
		oneTimeWav[(int)FSSND_ONETIME_BLAST2]=CreateWAVSoundBuffer(dSound8,"sound/blast2.wav");
		oneTimeWav[(int)FSSND_ONETIME_GEARUP]=CreateWAVSoundBuffer(dSound8,"sound/retractldg.wav");
		oneTimeWav[(int)FSSND_ONETIME_GEARDOWN]=CreateWAVSoundBuffer(dSound8,"sound/extendldg.wav");
		oneTimeWav[(int)FSSND_ONETIME_BOMBSAWAY]=CreateWAVSoundBuffer(dSound8,"sound/bombsaway.wav");
		oneTimeWav[(int)FSSND_ONETIME_ROCKET]=CreateWAVSoundBuffer(dSound8,"sound/rocket.wav");
		oneTimeWav[(int)FSSND_ONETIME_NOTICE]=CreateWAVSoundBuffer(dSound8,"sound/notice.wav");

		sndStatus.Initialize();
	}
}

__declspec(dllexport) void FsSoundDllTerminate(void)
{
	FsSoundDllReleaseBuffer(jetWav[0]);
	FsSoundDllReleaseBuffer(jetWav[1]);
	FsSoundDllReleaseBuffer(jetWav[2]);
	FsSoundDllReleaseBuffer(jetWav[3]);
	FsSoundDllReleaseBuffer(jetWav[4]);
	FsSoundDllReleaseBuffer(jetWav[5]);
	FsSoundDllReleaseBuffer(jetWav[6]);
	FsSoundDllReleaseBuffer(jetWav[7]);
	FsSoundDllReleaseBuffer(jetWav[8]);
	FsSoundDllReleaseBuffer(jetWav[9]);

	FsSoundDllReleaseBuffer(propWav[0]);
	FsSoundDllReleaseBuffer(propWav[1]);
	FsSoundDllReleaseBuffer(propWav[2]);
	FsSoundDllReleaseBuffer(propWav[3]);
	FsSoundDllReleaseBuffer(propWav[4]);
	FsSoundDllReleaseBuffer(propWav[5]);
	FsSoundDllReleaseBuffer(propWav[6]);
	FsSoundDllReleaseBuffer(propWav[7]);
	FsSoundDllReleaseBuffer(propWav[8]);
	FsSoundDllReleaseBuffer(propWav[9]);

	FsSoundDllReleaseBuffer(afterBurnerWav);

	FsSoundDllReleaseBuffer(machineGunWav[(int)FSSND_MACHINEGUN_MACHINEGUN]);

	FsSoundDllReleaseBuffer(alarmWav[(int)FSSND_ALARM_STALL]);
	FsSoundDllReleaseBuffer(alarmWav[(int)FSSND_ALARM_MISSILE]);
	FsSoundDllReleaseBuffer(alarmWav[(int)FSSND_ALARM_TERRAIN]);

	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_DAMAGE]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_MISSILE]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_BANG]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_BLAST]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_TOUCHDWN]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_HIT]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_BLAST2]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_GEARUP]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_GEARDOWN]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_BOMBSAWAY]);
	FsSoundDllReleaseBuffer(oneTimeWav[(int)FSSND_ONETIME_ROCKET]);

	/* I think DirectSound8 object must be released here.
	   However, it creates a dead-lock.
	   The calling program cannot DestroyWindow the window associated with the dSound8.
	if(NULL!=dSound8)
	{
		dSound8->Release();
		dSound8=NULL;
	} */

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
	int i;
	for(i=0; i<10; i++)
	{
		if(NULL!=jetWav[i])
		{
			jetWav[i]->Stop();
		}
		if(NULL!=propWav[i])
		{
			propWav[i]->Stop();
		}
	}
	if(NULL!=afterBurnerWav)
	{
		afterBurnerWav->Stop();
	}

	for(i=0; i<FSSND_NUM_MACHINEGUNTYPE; i++)
	{
		if(NULL!=machineGunWav[i])
		{
			machineGunWav[i]->Stop();
		}
	}
	for(i=0; i<FSSND_NUM_ALARMTYPE; i++)
	{
		if(NULL!=alarmWav[i])
		{
			alarmWav[i]->Stop();
		}
	}

	for(i=0; i<FSSND_NUM_ONETIMETYPE; i++)
	{
		if(NULL!=oneTimeWav[i])
		{
			oneTimeWav[i]->Stop();
		}
	}
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
		if(NULL!=oneTimeWav[(int)oneTimeType])
		{
			oneTimeWav[(int)oneTimeType]->Play(0,0xc0000000,0);
			sndStatus.oneTimeType=oneTimeType;
		}
	}
}

__declspec(dllexport) void FsSoundDllKeepPlaying(void)
{
	if(YSTRUE==fsSoundMasterSwitch && YSTRUE==fsSoundEnvironmentalSwitch)
	{
		int i;
		DWORD sta;

		for(i=0; i<FSSND_NUM_MACHINEGUNTYPE; i++)
		{
			if(NULL!=machineGunWav[i])
			{
				machineGunWav[i]->GetStatus(&sta);
				if(0==(sta&DSBSTATUS_PLAYING) && sndStatus.machineGunType==i)
				{
					machineGunWav[i]->Play(0,0x90000000,DSBPLAY_LOOPING);
				}
				else if(0!=(sta&DSBSTATUS_PLAYING) && sndStatus.machineGunType!=i)
				{
					machineGunWav[i]->Stop();
				}
			}
		}

		for(i=0; i<FSSND_NUM_ALARMTYPE; i++)
		{
			if(NULL!=alarmWav[i])
			{
				alarmWav[i]->GetStatus(&sta);
				if(0==(sta&DSBSTATUS_PLAYING) && sndStatus.alarmType==i)
				{
					alarmWav[i]->Play(0,0xa0000000,DSBPLAY_LOOPING);
				}
				else if(0!=(sta&DSBSTATUS_PLAYING) && sndStatus.alarmType!=i)
				{
					alarmWav[i]->Stop();
				}
			}
		}

		if(FSSND_ENGINE_SILENT!=sndStatus.engineType)
		{
			LPDIRECTSOUNDBUFFER wavToPlay=NULL;
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
				wavToPlay=jetWav[level];
				break;
			case FSSND_ENGINE_JETAFTERBURNER:
				wavToPlay=afterBurnerWav;
				break;
			case FSSND_ENGINE_PROPELLER:
			case FSSND_ENGINE_TURBOPROP:
			case FSSND_ENGINE_HELICOPTER:
				wavToPlay=propWav[level];
				break;
			}

			if(NULL!=wavToPlay)
			{
				wavToPlay->GetStatus(&sta);
				if(0==(sta&DSBSTATUS_PLAYING))
				{
					wavToPlay->Play(0,0x80000000,DSBPLAY_LOOPING);
				}
			}

			for(i=0; i<10; i++)
			{
				if(NULL!=jetWav[i])
				{
					jetWav[i]->GetStatus(&sta);
					if(0!=(sta&DSBSTATUS_PLAYING) && wavToPlay!=jetWav[i])
					{
						jetWav[i]->Stop();
					}
				}

				if(NULL!=propWav[i])
				{
					propWav[i]->GetStatus(&sta);
					if(0!=(sta&DSBSTATUS_PLAYING) && wavToPlay!=propWav[i])
					{
						propWav[i]->Stop();
					}
				}
			}

			if(NULL!=afterBurnerWav)
			{
				afterBurnerWav->GetStatus(&sta);
				if(0!=(sta&DSBSTATUS_PLAYING) && wavToPlay!=afterBurnerWav)
				{
					afterBurnerWav->Stop();
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

