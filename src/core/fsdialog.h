#ifndef FSDIALOG_IS_INCLUDED
#define FSDIALOG_IS_INCLUDED
/* { */

#include "fsdef.h"
#include "fssiminfo.h"

class FsNewFlightEnvironmentInfo
{
public:
	void Initialize(void);

	YSBOOL specifyEnvironment;

	double windDir,windSpd;
	FSENVIRONMENT dayOrNight;
	YSBOOL fog;
	double fogVisibility;

	YsArray <double> cloudLayer; // [2n+0] Floor [2n+1] Top
};

class FsNewFlightDialogOption
{
public:
	YSBOOL canSelectField;
	YSBOOL canSelectStartPosition;
	YSBOOL canSelectLoading;
	YSBOOL canSelectEnvironment;
	YSBOOL canSelectWingmen;
	YSBOOL canChooseNight;
	YSBOOL canChooseFomType;
	YSBOOL flyNowButton;

	YSBOOL forRacingMode;

	YsString fieldName;     // If nonzero, will override FsNewFlightDialogInfo.
	YsString aircraftName;  // If nonzero, will override FsNewFlightDialogInfo.
	YsString startPosName;  // If nonzero, will override FsNewFlightDialogInfo.
	YSBOOL nextStartPos;    // Valid only if startPosName is nonzero.  If true, one stp after startPosName is chosen by default.

	FsNewFlightDialogOption();
};

class FsNewFlightDialogInfo
{
public:
	enum
	{
		MaxNumWingman=8
	};

	void Initialize(void);
	void SetFromConfig(const class FsWorld *world,const class FsFlightConfig &cfg);
	void SetFromOption(const FsNewFlightDialogOption &opt);
	void AutoSetStartPosition(FsWorld *world,const char fieldName[]);
	void AutoSetWingmanStartPosition(FsWorld *world,const char fieldName[],const char playerStartPos[]);

	YsString fieldName;
	YSBOOL addComputerAircraft;

	FsNewFlightAirplaneData playerAirInfo;
	YSBOOL nextStartPos;    // If true, one stp after playerAirInfo.startPos is chosen by default.

	YSBOOL flyImmediately;

	FsNewFlightEnvironmentInfo envInfo;

	YSBOOL wingman[MaxNumWingman];
	FsNewFlightAirplaneData wingmanInfo[MaxNumWingman];

	FSFORMATIONTYPE fomType;
};



class FsEnduranceModeDialogInfo
{
public:
	FsNewFlightAirplaneData playerAirInfo;
	YsString fieldName;
	YSBOOL allowAAM;
	int numWingman,wingmanLevel;
	YSBOOL jet,ww2;
};



/* } */
#endif
