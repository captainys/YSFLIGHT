#ifndef FSCMDPARAMINFO_IS_INCLUDED
#define FSCMDPARAMINFO_IS_INCLUDED
/* { */

#include <ysclass.h>
#include "fsweather.h"
#include "fsdialog.h" // FsInterceptMissionInfo is defined here.  Shall be moved to an appropriate place.

class FsCommandParameter
{
public:
	FsCommandParameter();
	YSRESULT RecognizeCommandParameter(int ac,char *av[]);

	void ShowHelp(void);

	YSBOOL keyMenu,autoExit;

	enum
	{
		EXEMODE_NORMAL=0,
		EXEMODE_SERVER=1,
		EXEMODE_CLIENT=2,
		EXEMODE_FREEFLIGHT=3,
		EXEMODE_FLYYFS=4,
		EXEMODE_ENDURANCE=5,
		EXEMODE_INTERCEPT=6,
		EXEMODE_REPLAYRECORD=100,
		EXEMODE_OPENINGDEMOFOREVER=200
	};

	int executionMode;  //  0:Normal
	                    //  1:Server
	                    //  2:Client
	                    //  3:Free Flight
	                    //  4:Fly YFS
	                    //  5:Endurance Mode
	                    //  6:Intercept Mission
	                    //100:Replay Record
	                    //200:Opening demo forever

	FsInterceptMissionInfo interceptMissionInfo;
	int endModeNumWingman,endModeWingmanLevel;
	YSBOOL endModeAllowAAM;

	YSBOOL showAirList,showFldList,showStpList;
	YSBOOL deleteFirstStartFile;

	int netPort;

	YsString airName,fldName,startPos;

	YsString netPlayerName;
	YsString netServerName;
	YSBOOL netCloseServerWhenAllLogOut;

	YSBOOL autoSave;
	YsWString yfsFilename;

	YsWString testScriptFilename;

	YSBOOL prepareRelease;
	YSBOOL setDefConfig,setDefNetConfig,setDefKey,setDefOption;
};

/* } */
#endif
