#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


#include "../fsairsoundenum.h"

#include "../yswavfile/yswavfile.h"
#include "yspulseaudio.h"


YsPulseAudioContext *ysPaContext=NULL;
YsPulseAudioWavPlayer *ysPaEnginePlayer=NULL;
YsPulseAudioWavPlayer *ysPaGunPlayer=NULL;
YsPulseAudioWavPlayer *ysPaAlarmPlayer=NULL;
YsPulseAudioWavPlayer *ysPaOneTimePlayer=NULL;
// Make sure to call WriteData in FsSoundDllKeepPlaying(void) function and
// call Stop in FsSoundDllStopAll(void) function.

////////////////////////////////////////////////////////////

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

static YSBOOL fsSoundMasterSwitch=YSTRUE;
static YSBOOL fsSoundEnvironmentalSwitch=YSTRUE;
static YSBOOL fsSoundOneTimeSwitch=YSTRUE;

static YsWavFile jetWav[10],afterBurnerWav,propWav[10];
static YsWavFile machineGunWav[FSSND_NUM_MACHINEGUNTYPE];
static YsWavFile alarmWav[FSSND_NUM_ALARMTYPE];
static YsWavFile oneTimeWav[FSSND_NUM_ONETIMETYPE];

static FsSoundStatus sndStatus;




extern "C" void FsSoundDllInitialize(void)
{
	ysPaContext=new YsPulseAudioContext;
	ysPaEnginePlayer=new YsPulseAudioWavPlayer(ysPaContext,"YSFLIGHT_Engine_Sound");
	ysPaGunPlayer=new YsPulseAudioWavPlayer(ysPaContext,"YSFLIGHT_Gun_Sound");
	ysPaAlarmPlayer=new YsPulseAudioWavPlayer(ysPaContext,"YSFLIGHT_Alarm_Sound");
	ysPaOneTimePlayer=new YsPulseAudioWavPlayer(ysPaContext,"YSFLIGHT_OneTime_Sound");
	 

	jetWav[0].LoadWav("sound/engine0.wav");
	jetWav[1].LoadWav("sound/engine1.wav");
	jetWav[2].LoadWav("sound/engine2.wav");
	jetWav[3].LoadWav("sound/engine3.wav");
	jetWav[4].LoadWav("sound/engine4.wav");
	jetWav[5].LoadWav("sound/engine5.wav");
	jetWav[6].LoadWav("sound/engine6.wav");
	jetWav[7].LoadWav("sound/engine7.wav");
	jetWav[8].LoadWav("sound/engine8.wav");
	jetWav[9].LoadWav("sound/engine9.wav");

	propWav[0].LoadWav("sound/prop0.wav");
	propWav[1].LoadWav("sound/prop1.wav");
	propWav[2].LoadWav("sound/prop2.wav");
	propWav[3].LoadWav("sound/prop3.wav");
	propWav[4].LoadWav("sound/prop4.wav");
	propWav[5].LoadWav("sound/prop5.wav");
	propWav[6].LoadWav("sound/prop6.wav");
	propWav[7].LoadWav("sound/prop7.wav");
	propWav[8].LoadWav("sound/prop8.wav");
	propWav[9].LoadWav("sound/prop9.wav");

	afterBurnerWav.LoadWav("sound/burner.wav");

	machineGunWav[(int)FSSND_MACHINEGUN_MACHINEGUN].LoadWav("sound/gun.wav");

	alarmWav[(int)FSSND_ALARM_STALL].LoadWav("sound/stallhorn.wav");
	alarmWav[(int)FSSND_ALARM_MISSILE].LoadWav("sound/warning.wav");
	alarmWav[(int)FSSND_ALARM_TERRAIN].LoadWav("sound/gearhorn.wav");

	oneTimeWav[(int)FSSND_ONETIME_DAMAGE].LoadWav("sound/damage.wav");
	oneTimeWav[(int)FSSND_ONETIME_MISSILE].LoadWav("sound/missile.wav");
	oneTimeWav[(int)FSSND_ONETIME_BANG].LoadWav("sound/bang.wav");
	oneTimeWav[(int)FSSND_ONETIME_BLAST].LoadWav("sound/blast.wav");
	oneTimeWav[(int)FSSND_ONETIME_TOUCHDWN].LoadWav("sound/touchdwn.wav");
	oneTimeWav[(int)FSSND_ONETIME_HIT].LoadWav("sound/hit.wav");
	oneTimeWav[(int)FSSND_ONETIME_BLAST2].LoadWav("sound/blast2.wav");
	oneTimeWav[(int)FSSND_ONETIME_GEARUP].LoadWav("sound/retractldg.wav");
	oneTimeWav[(int)FSSND_ONETIME_GEARDOWN].LoadWav("sound/extendldg.wav");
	oneTimeWav[(int)FSSND_ONETIME_BOMBSAWAY].LoadWav("sound/bombsaway.wav");
	oneTimeWav[(int)FSSND_ONETIME_ROCKET].LoadWav("sound/rocket.wav");
	oneTimeWav[(int)FSSND_ONETIME_NOTICE].LoadWav("sound/notice.wav");

	sndStatus.Initialize();
}

extern "C" void FsSoundDllTerminate(void)
{
	delete ysPaEnginePlayer;
	delete ysPaGunPlayer;
	delete ysPaAlarmPlayer;
	delete ysPaOneTimePlayer;
	delete ysPaContext;
}

extern "C" void FsSoundDllSetMasterSwitch(YSBOOL sw)
{
	fsSoundMasterSwitch=sw;
}

extern "C" void FsSoundDllSetEnvironmentalSwitch(YSBOOL sw)
{
	fsSoundEnvironmentalSwitch=sw;
}

extern "C" void FsSoundDllSetOneTimeSwitch(YSBOOL sw)
{
	fsSoundOneTimeSwitch=sw;
}

extern "C" void FsSoundDllStopAll(void)
{
	ysPaEnginePlayer->Stop();
	ysPaGunPlayer->Stop();
	ysPaAlarmPlayer->Stop();
	ysPaOneTimePlayer->Stop();
}

extern "C" void FsSoundDllSetVehicleName(const char vehicleName[])
{
	sndStatus.vehicleName=vehicleName;
}

extern "C" void FsSoundDllSetEngine(FSSND_ENGINETYPE engineType,int numEngine,const double power)
{
	sndStatus.engineType=engineType;
	sndStatus.numEngine=numEngine;
	sndStatus.enginePower=power;
}

extern "C" void FsSoundDllSetMachineGun(FSSND_MACHINEGUNTYPE machineGunType)
{
	sndStatus.machineGunType=machineGunType;
}

extern "C" void FsSoundDllSetAlarm(FSSND_ALARMTYPE alarmType)
{
	sndStatus.alarmType=alarmType;
}

extern "C" void FsSoundDllSetOneTime(FSSND_ONETIMETYPE oneTimeType)
{
	if(YSTRUE==fsSoundMasterSwitch && 
	   YSTRUE==fsSoundOneTimeSwitch &&
	   NULL!=ysPaOneTimePlayer)
	{
		if(FSSND_ONETIME_SILENT!=oneTimeType)
		{
			ysPaOneTimePlayer->Stop();
		}
		ysPaOneTimePlayer->Play(&oneTimeWav[(int)oneTimeType]);
		sndStatus.oneTimeType=oneTimeType;
	}
}

extern "C" void FsSoundDllKeepPlaying(void)
{
	if(YSTRUE==fsSoundMasterSwitch && NULL!=ysPaContext)
	{
		ysPaContext->KeepPlaying();
	}

	if(YSTRUE==fsSoundMasterSwitch && YSTRUE==fsSoundOneTimeSwitch)
	{
		// Write data for all streams >>
		if(NULL!=ysPaOneTimePlayer)
		{
			ysPaOneTimePlayer->WriteData(YSFALSE);
		}
		// << Write data for all streams
	}

	if(YSTRUE==fsSoundMasterSwitch && YSTRUE==fsSoundEnvironmentalSwitch)
	{
		// Write data for all streams >>
		if(NULL!=ysPaEnginePlayer)
		{
			ysPaEnginePlayer->WriteData(YSTRUE);
		}
		if(NULL!=ysPaGunPlayer)
		{
			ysPaGunPlayer->WriteData(YSTRUE);
		}
		if(NULL!=ysPaAlarmPlayer)
		{
			ysPaAlarmPlayer->WriteData(YSTRUE);
		}
		// << Write data for all streams


		if(FSSND_MACHINEGUN_SILENT!=sndStatus.machineGunType)
		{
			ysPaGunPlayer->Play(&machineGunWav[(int)sndStatus.machineGunType]);
		}
		else
		{
			ysPaGunPlayer->Stop();
		}

		if(FSSND_ALARM_SILENT!=sndStatus.alarmType)
		{
			ysPaAlarmPlayer->Play(&alarmWav[(int)sndStatus.alarmType]);
		}
		else
		{
			ysPaAlarmPlayer->Stop();
		}

		if(FSSND_ENGINE_SILENT!=sndStatus.engineType)
		{
			YsWavFile *wavToPlay=NULL;
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
				ysPaEnginePlayer->Stop();
				break;
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
			if(NULL!=wavToPlay)
			{
				ysPaEnginePlayer->Play(wavToPlay);
			}
		}
	}
}

