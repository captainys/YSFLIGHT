#import <Cocoa/Cocoa.h>
#include "../fsairsoundenum.h"



typedef struct
{
	const char *vehicleName;

	NSSound *enginePlaying;
	NSSound *machineGunPlaying;
	NSSound *alarmPlaying;
} FsSoundStatus;

static void FsInitializeSoundStatus(FsSoundStatus *status)
{
	status->vehicleName="";

	status->enginePlaying=NULL;
	status->machineGunPlaying=NULL;
	status->alarmPlaying=NULL;
}



static bool fsSoundMasterSwitch=true;
static bool fsSoundEnvironmentalSwitch=true;
static bool fsSoundOneTimeSwitch=true;

static NSSound *jetWav[10],*afterBurnerWav,*propWav[10];
static NSSound *machineGunWav[FSSND_NUM_MACHINEGUNTYPE];
static NSSound *alarmWav[FSSND_NUM_ALARMTYPE];
static NSSound *oneTimeWav[FSSND_NUM_ONETIMETYPE];

static FsSoundStatus sndStatus;




void FsSoundDllInitialize(void)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	jetWav[0]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine0.wav" byReference:NO];
	jetWav[1]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine1.wav" byReference:NO];
	jetWav[2]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine2.wav" byReference:NO];
	jetWav[3]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine3.wav" byReference:NO];
	jetWav[4]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine4.wav" byReference:NO];
	jetWav[5]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine5.wav" byReference:NO];
	jetWav[6]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine6.wav" byReference:NO];
	jetWav[7]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine7.wav" byReference:NO];
	jetWav[8]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine8.wav" byReference:NO];
	jetWav[9]=[[NSSound alloc] initWithContentsOfFile:@"sound/engine9.wav" byReference:NO];

	propWav[0]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop0.wav" byReference:NO];
	propWav[1]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop1.wav" byReference:NO];
	propWav[2]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop2.wav" byReference:NO];
	propWav[3]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop3.wav" byReference:NO];
	propWav[4]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop4.wav" byReference:NO];
	propWav[5]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop5.wav" byReference:NO];
	propWav[6]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop6.wav" byReference:NO];
	propWav[7]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop7.wav" byReference:NO];
	propWav[8]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop8.wav" byReference:NO];
	propWav[9]=[[NSSound alloc] initWithContentsOfFile:@"sound/prop9.wav" byReference:NO];

	afterBurnerWav=[[NSSound alloc] initWithContentsOfFile:@"sound/burner.wav" byReference:NO];

	machineGunWav[(int)FSSND_MACHINEGUN_SILENT]=NULL;
	machineGunWav[(int)FSSND_MACHINEGUN_MACHINEGUN]=[[NSSound alloc] initWithContentsOfFile:@"sound/gun.wav" byReference:NO];

	alarmWav[(int)FSSND_ALARM_SILENT]=NULL;
	alarmWav[(int)FSSND_ALARM_STALL]=[[NSSound alloc] initWithContentsOfFile:@"sound/stallhorn.wav" byReference:NO];
	alarmWav[(int)FSSND_ALARM_MISSILE]=[[NSSound alloc] initWithContentsOfFile:@"sound/warning.wav" byReference:NO];
	alarmWav[(int)FSSND_ALARM_TERRAIN]=[[NSSound alloc] initWithContentsOfFile:@"sound/gearhorn.wav" byReference:NO];

	oneTimeWav[(int)FSSND_ONETIME_SILENT]=NULL;
	oneTimeWav[(int)FSSND_ONETIME_DAMAGE]=[[NSSound alloc] initWithContentsOfFile:@"sound/damage.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_MISSILE]=[[NSSound alloc] initWithContentsOfFile:@"sound/missile.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_BANG]=[[NSSound alloc] initWithContentsOfFile:@"sound/bang.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_BLAST]=[[NSSound alloc] initWithContentsOfFile:@"sound/blast.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_TOUCHDWN]=[[NSSound alloc] initWithContentsOfFile:@"sound/touchdwn.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_HIT]=[[NSSound alloc] initWithContentsOfFile:@"sound/hit.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_BLAST2]=[[NSSound alloc] initWithContentsOfFile:@"sound/blast2.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_GEARUP]=[[NSSound alloc] initWithContentsOfFile:@"sound/retractldg.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_GEARDOWN]=[[NSSound alloc] initWithContentsOfFile:@"sound/extendldg.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_BOMBSAWAY]=[[NSSound alloc] initWithContentsOfFile:@"sound/bombsaway.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_ROCKET]=[[NSSound alloc] initWithContentsOfFile:@"sound/rocket.wav" byReference:NO];
	oneTimeWav[(int)FSSND_ONETIME_NOTICE]=[[NSSound alloc] initWithContentsOfFile:@"sound/notice.wav" byReference:NO];

	FsInitializeSoundStatus(&sndStatus);

	[pool release];
}

void FsSoundDllTerminate(void)
{
	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	[jetWav[0] release];
	[jetWav[1] release];
	[jetWav[2] release];
	[jetWav[3] release];
	[jetWav[4] release];
	[jetWav[5] release];
	[jetWav[6] release];
	[jetWav[7] release];
	[jetWav[8] release];
	[jetWav[9] release];

	[propWav[0] release];
	[propWav[1] release];
	[propWav[2] release];
	[propWav[3] release];
	[propWav[4] release];
	[propWav[5] release];
	[propWav[6] release];
	[propWav[7] release];
	[propWav[8] release];
	[propWav[9] release];

	[afterBurnerWav release];

	machineGunWav[(int)FSSND_MACHINEGUN_SILENT]=NULL;
	[machineGunWav[(int)FSSND_MACHINEGUN_MACHINEGUN] release];

	alarmWav[(int)FSSND_ALARM_SILENT]=NULL;
	[alarmWav[(int)FSSND_ALARM_STALL] release];
	[alarmWav[(int)FSSND_ALARM_MISSILE] release];
	[alarmWav[(int)FSSND_ALARM_TERRAIN] release];

	oneTimeWav[(int)FSSND_ONETIME_SILENT]=NULL;
	[oneTimeWav[(int)FSSND_ONETIME_DAMAGE] release];
	[oneTimeWav[(int)FSSND_ONETIME_MISSILE] release];
	[oneTimeWav[(int)FSSND_ONETIME_BANG] release];
	[oneTimeWav[(int)FSSND_ONETIME_BLAST] release];
	[oneTimeWav[(int)FSSND_ONETIME_TOUCHDWN] release];
	[oneTimeWav[(int)FSSND_ONETIME_HIT] release];
	[oneTimeWav[(int)FSSND_ONETIME_BLAST2] release];
	[oneTimeWav[(int)FSSND_ONETIME_GEARUP] release];
	[oneTimeWav[(int)FSSND_ONETIME_GEARDOWN] release];
	[oneTimeWav[(int)FSSND_ONETIME_BOMBSAWAY] release];
	[oneTimeWav[(int)FSSND_ONETIME_ROCKET] release];

	FsInitializeSoundStatus(&sndStatus);

	[pool release];
}

void FsSoundDllSetMasterSwitch(bool sw)
{
	fsSoundMasterSwitch=sw;
}

void FsSoundDllSetEnvironmentalSwitch(bool sw)
{
	fsSoundEnvironmentalSwitch=sw;
}

void FsSoundDllSetOneTimeSwitch(bool sw)
{
	fsSoundOneTimeSwitch=sw;
}


void FsSoundDllStopAll(void)
{
	if(true==fsSoundMasterSwitch)
	{
		NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

		if(NULL!=sndStatus.enginePlaying)
		{
			[sndStatus.enginePlaying stop];
			sndStatus.enginePlaying=NULL;
		}
		if(NULL!=sndStatus.machineGunPlaying)
		{
			[sndStatus.machineGunPlaying stop];
			sndStatus.machineGunPlaying=NULL;
		}
		if(NULL!=sndStatus.alarmPlaying)
		{
			[sndStatus.alarmPlaying stop];
			sndStatus.alarmPlaying=NULL;
		}

		[pool release];
	}
}

void FsSoundDllSetVehicleName(const char vehicleName[])
{
	sndStatus.vehicleName=vehicleName;
}

void FsSoundDllSetEngine(FSSND_ENGINETYPE engineType,int numEngine,const double power)
{
	if(true==fsSoundMasterSwitch && true==fsSoundEnvironmentalSwitch)
	{
		NSSound *wavToPlay=NULL;
		int level=(int)(power*10.0);

		if(level<0)
		{
			level=0;
		}
		else if(9<level)
		{
			level=9;
		}

		switch(engineType)
		{
		default:
			break;
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

		if(sndStatus.enginePlaying!=wavToPlay)
		{
			NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

			if(NULL!=sndStatus.enginePlaying)
			{
				[sndStatus.enginePlaying stop];
			}
			if(NULL!=wavToPlay)
			{
				[wavToPlay setLoops:YES];
				[wavToPlay play];
			}
			sndStatus.enginePlaying=wavToPlay;

			[pool release];
		}
	}
}

void FsSoundDllSetMachineGun(FSSND_MACHINEGUNTYPE machineGunType)
{
	if(true==fsSoundMasterSwitch && true==fsSoundEnvironmentalSwitch)
	{
		NSSound *wavToPlay=machineGunWav[(int)machineGunType];
		if(sndStatus.machineGunPlaying!=wavToPlay)
		{
			NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

			if(NULL!=sndStatus.machineGunPlaying)
			{
				[sndStatus.machineGunPlaying stop];
			}
			if(NULL!=wavToPlay)
			{
				[wavToPlay setLoops:YES];
				[wavToPlay play];
			}
			sndStatus.machineGunPlaying=wavToPlay;

			[pool release];
		}
	}
}

void FsSoundDllSetAlarm(FSSND_ALARMTYPE alarmType)
{
	if(true==fsSoundMasterSwitch && true==fsSoundEnvironmentalSwitch)
	{
		NSSound *wavToPlay=alarmWav[(int)alarmType];
		if(sndStatus.alarmPlaying!=wavToPlay)
		{
			NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

			if(NULL!=sndStatus.alarmPlaying)
			{
				[sndStatus.alarmPlaying stop];
			}
			if(NULL!=wavToPlay)
			{
				[wavToPlay setLoops:YES];
				[wavToPlay play];
			}
			sndStatus.alarmPlaying=wavToPlay;

			[pool release];
		}
	}
}

void FsSoundDllSetOneTime(FSSND_ONETIMETYPE oneTimeType)
{
	if(true==fsSoundMasterSwitch && true==fsSoundOneTimeSwitch && NULL!=oneTimeWav[(int)oneTimeType])
	{
		[oneTimeWav[(int)oneTimeType] setLoops:NO];
		[oneTimeWav[(int)oneTimeType] play];
	}
}

void FsSoundDllKeepPlaying(void)
{
	if(true==fsSoundMasterSwitch && true==fsSoundEnvironmentalSwitch)
	{
		/*
		Mac OSX's sound-playback has a problem of blank or gap when it comes to the end of the
		wav duration.  This is a problem and Apple must fix it.

		For the meanwhile, it is the best and simplest hack I found to eliminate the gap.
		*/

		if(NULL!=sndStatus.enginePlaying)
		{
			if(0.4<=[sndStatus.enginePlaying duration] && 0.05>[sndStatus.enginePlaying duration]-[sndStatus.enginePlaying currentTime])
			{
				[sndStatus.enginePlaying setCurrentTime:0.0];
			}
		}

		if(NULL!=sndStatus.machineGunPlaying)
		{
			if(0.4<=[sndStatus.machineGunPlaying duration] && 0.05>[sndStatus.machineGunPlaying duration]-[sndStatus.machineGunPlaying currentTime])
			{
				[sndStatus.machineGunPlaying setCurrentTime:0.0];
			}
		}

		if(NULL!=sndStatus.alarmPlaying)
		{
			if(0.4<=[sndStatus.alarmPlaying duration] && 0.05>[sndStatus.alarmPlaying duration]-[sndStatus.alarmPlaying currentTime])
			{
				[sndStatus.alarmPlaying setCurrentTime:0.0];
			}
		}
	}
}
