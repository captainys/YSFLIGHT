#ifndef FSCONFIG_IS_INCLUDED
#define FSCONFIG_IS_INCLUDED
/* { */


#include <ysclass.h>
#include <ysport.h>

#include "fsdef.h"


// Declaration /////////////////////////////////////////////
class FsFlightConfig
{
public:
	FsFlightConfig();

	static const char *const keyWordSource[];
	static YsKeyWordList keyWordList;

	// ? char err[256];


	FSSMOKETYPE smkType;
	YSBOOL useParticle;
	double smkRemainTime;
	int smkStep;
	YSBOOL takeCrash;
	YSBOOL terminateUponPlayerCrash;
	YSBOOL horizonGradation;
	YSBOOL drawShadow;
	YSBOOL drawCloud;
	double ceiling;
	FSCLOUDTYPE cloudType;
	YSBOOL blackOut;
	YSBOOL canLandAnywhere;
	YSBOOL midAirCollision;
	YSBOOL noTailStrike;
	YSBOOL autoCoordination;
	YSBOOL shadowOfDeadAirplane;
	YSBOOL drawOrdinance;
	YSBOOL drawFog;
	YSBOOL perPixelShading;
	YSBOOL showFps;
	int zbuffQuality; // 0:Fast  1:Default  2:Detailed  3:Highest Quality
	YSBOOL showHudAlways;
	YSBOOL additionalCalibration;
	YSBOOL drawCoarseOrdinance;
	YSBOOL drawPlayerNameAlways;
	YSBOOL drawLightsInDaylight;
	double drawLightsInDaylightVisibilityThr;
	YSBOOL drawTransparency;
	YSBOOL drawTransparentVapor;
	YSBOOL drawTransparentSmoke;
	YSBOOL drawTransparentLater;
	YSBOOL useOpenGlListForCloud;
	YSBOOL useOpenGlListForExplosion;
	YSBOOL useOpenGlListForWeapon;
	YSBOOL useOpenGlGroundTexture;
	YSBOOL useOpenGlRunwayLightTexture;
	YSBOOL showIAS;

	YSBOOL useOpenGlAntiAliasing;

	YSBOOL useSimpleHud;

	int aircraftReliability;          // See FsSimulation::PlayerAircraftGetTrouble
	double aircraftTroubleFrequency;  // See FsSimulation::PlayerAircraftGetTrouble

	int showUserName;  // 0,1,3:Show Always  2:Never  n>100:Within n meters

	YSBOOL externalCameraDelay;

	YSBOOL showJoystickUnpluggedWarning;

	YSBOOL drawVirtualJoystick;

	YSBOOL useHudAlways;

	YSBOOL neverDrawAirplaneContainer;

	int airLod;  // 0: Automatic  1:Always detailed model  2:Always coarse model
	int gndLod;  // 0: Automatic  1:Always detailed model  2:Always coarse model

	char defAirplane[256];
	char defField[256];
	char defStartPos[256];

	FSENVIRONMENT env;
	double fogVisibility;

	double radarAltitudeLimit;
	YSBOOL noExtAirView;

	YsVec3 lightSourceDirection;

	YSBOOL accurateTime;

	void SetDefault(void);
	void SetDetailedMode(void);
	void SetFastMode(void);

	YSRESULT SendCommand(const char cmd[]);

	YSRESULT Load(const wchar_t fn[]);
	YSRESULT Save(const wchar_t fn[]);
};



/* } */
#endif
