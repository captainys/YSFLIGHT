#ifndef FSAIRSOUND_IS_INCLUDED
#define FSAIRSOUND_IS_INCLUDED
/* { */

#include "../sounddll/fsairsoundenum.h"
#include "../voicedll/fsvoiceenum.h"

void FsSoundInitialize(void);
void FsSoundTerminate(void);

void FsSoundSetMasterSwitch(YSBOOL sw);
void FsSoundSetEnvironmentalSwitch(YSBOOL sw);
void FsSoundSetOneTimeSwitch(YSBOOL sw);

void FsSoundStopAll(void);
void FsSoundSetVehicleName(const char vehicleName[]);
void FsSoundSetEngine(FSSND_ENGINETYPE engineType,int numEngine,const double power);
void FsSoundSetMachineGun(FSSND_MACHINEGUNTYPE machineGunType);
void FsSoundSetAlarm(FSSND_ALARMTYPE alarmType);
void FsSoundSetOneTime(FSSND_ONETIMETYPE oneTimeType);
void FsSoundKeepPlaying(void);

void FsVoiceStopAll(void);
void FsVoiceSpeak(int nVoicePhrase,const FsVoicePhrase voicePhrase[]);
void FsVoiceKeepSpeaking(void);

/* } */
#endif
