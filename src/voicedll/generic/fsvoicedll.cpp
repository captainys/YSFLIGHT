#include <stdio.h>
#include "../fsvoiceenum.h"


#ifdef _WIN32
	#include <windows.h>
	#define WINDOWS_NEEDS_THIS_STUPID_KEYWORD __declspec(dllexport)
#else
	#define WINDOWS_NEEDS_THIS_STUPID_KEYWORD
#endif

static YSBOOL fsVoiceMasterSwitch=YSTRUE;

#ifdef _WIN32
extern "C" WINDOWS_NEEDS_THIS_STUPID_KEYWORD void FsVoiceDllInitialize(HWND hMainWnd)
{
	printf("[VOICEDLL] This is a skeleton for the future voice extension.\n");
	printf("[VOICEDLL] This DLL does not actually function at this time.\n");
	printf("%s (Line %d)\n",__FUNCTION__,__LINE__);
}
#else
extern "C" void FsVoiceDllInitialize(void)
{
	printf("[VOICEDLL] This is a skeleton for the future voice extension.\n");
	printf("[VOICEDLL] This DLL does not actually function at this time.\n");
	printf("%s (Line %d)\n",__FUNCTION__,__LINE__);
}
#endif

extern "C" WINDOWS_NEEDS_THIS_STUPID_KEYWORD void FsVoiceDllTerminate(void)
{
	printf("%s (Line %d)\n",__FUNCTION__,__LINE__);
}

extern "C" WINDOWS_NEEDS_THIS_STUPID_KEYWORD void FsVoiceDllSetMasterSwitch(YSBOOL sw)
{
	fsVoiceMasterSwitch=sw;
	printf("%s (Line %d)\n",__FUNCTION__,__LINE__);
}

extern "C" WINDOWS_NEEDS_THIS_STUPID_KEYWORD void FsVoiceDllStopAll(void)
{
	printf("%s (Line %d)\n",__FUNCTION__,__LINE__);
}

extern "C" WINDOWS_NEEDS_THIS_STUPID_KEYWORD void FsVoiceDllSpeak(int nVoicePhrase,const struct FsVoicePhrase voicePhrase[])
{
	printf("%s (Line %d)\n",__FUNCTION__,__LINE__);
	if(YSTRUE==fsVoiceMasterSwitch)
	{
		printf("* Begin Speech\n");
		for(int i=0; i<nVoicePhrase; i++)
		{
			switch(voicePhrase[i].phraseType)
			{
			case FSVOICE_GENERAL:
				printf("[GENERAL]");
				break;
			case FSVOICE_COMMA:
				printf("[COMMA]");
				break;
			case FSVOICE_SPACE:
				printf("[SPACE]");
				break;
			case FSVOICE_PERIOD:
				printf("[PERIOD]");
				break;
			case FSVOICE_END_OF_SENTENCE:
				printf("[END_OF_SENTENCE]");
				break;
			case FSVOICE_CALLSIGN:
				printf("[CALLSIGN]");
				break;
			case FSVOICE_HEADING:
				printf("[HEADING]");
				break;
			case FSVOICE_GENERAL_NUMBER:
				printf("[GENERAL_NUMBER]");
				break;
			case FSVOICE_ALTITUDE_IN_FEET:
				printf("[ALTITUDE_IN_FEET]");
				break;
			case FSVOICE_ALTITUDE_IN_FLIGHT_LEVEL:
				printf("[ALTITUDE_IN_FLIGHT_LEVEL]");
				break;
			case FSVOICE_SPEED_IN_KNOT:
				printf("[SPEED_IN_KNOT]");
				break;
			case FSVOICE_RUNWAY:
				printf("[RUNWAY]");
				break;
			case FSVOICE_APPROACH_GENERAL:
				printf("[APPROACH_GENERAL]");
				break;
			case FSVOICE_APPROACH_ILS:
				printf("[APPROACH_ILS]");
				break;
			case FSVOICE_APPROACH_VOR:
				printf("[APPROACH_VOR]");
				break;
			case FSVOICE_APPROACH_NDB:
				printf("[APPROACH_NDB]");
				break;
			case FSVOICE_APPROACH_GPS:
				printf("[APPROACH_GPS]");
				break;
			}
			if(NULL!=voicePhrase[i].phrase)
			{
				printf("%s",voicePhrase[i].phrase);
			}
			printf("\n");
		}
		printf("* End Speech\n");
	}
}

extern "C" WINDOWS_NEEDS_THIS_STUPID_KEYWORD void FsVoiceDllKeepSpeaking(void)
{
	printf("%s (Line %d)\n",__FUNCTION__,__LINE__);
}

