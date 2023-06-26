#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>

#include "fsconfig.h"
#include "fsutil.h"


// Implementation //////////////////////////////////////////
FsFlightConfig::FsFlightConfig()
{
	SetDefault();
}

void FsFlightConfig::SetDefault(void)
{
	smkType=FSSMKSOLID;
	useParticle=YSTRUE;
	smkRemainTime=30.0;
	smkStep=4;
	takeCrash=YSTRUE;
	terminateUponPlayerCrash=YSTRUE;
	horizonGradation=YSTRUE;
	drawShadow=YSTRUE;
	drawCloud=YSTRUE;
	ceiling=4000.0;
	cloudType=FSCLOUDSOLID;
	blackOut=YSTRUE;
	canLandAnywhere=YSTRUE;
	midAirCollision=YSTRUE;
	noTailStrike=YSTRUE;
	autoCoordination=YSTRUE;
	shadowOfDeadAirplane=YSTRUE;
	drawOrdinance=YSTRUE;
	drawFog=YSTRUE;
	perPixelShading=YSFALSE;
	showHudAlways=YSFALSE;
	additionalCalibration=YSFALSE;
	drawCoarseOrdinance=YSFALSE;
	zbuffQuality=1;
	drawTransparency=YSTRUE;
	drawTransparentVapor=YSTRUE;
	drawTransparentSmoke=YSTRUE;
	drawTransparentLater=YSTRUE;

	drawPlayerNameAlways=YSTRUE;
	drawLightsInDaylight=YSTRUE;
	drawLightsInDaylightVisibilityThr=4667.1; // 2.9 miles visibility.
	drawVirtualJoystick=YSTRUE;

	externalCameraDelay=YSFALSE;

	showJoystickUnpluggedWarning=YSTRUE;

	useHudAlways=YSFALSE;

	useSimpleHud=YSFALSE;

	neverDrawAirplaneContainer=YSFALSE;

	accurateTime=YSTRUE;

	fogVisibility=FS_FOG_VISIBILITY_MAX;
	radarAltitudeLimit=1000.0*0.3048;
	noExtAirView=YSFALSE;

	env=FSDAYLIGHT;
	lightSourceDirection.Set(0.0,sqrt(3.0)/2.0,-0.5);

	useOpenGlListForCloud=YSTRUE;
	useOpenGlListForExplosion=YSTRUE;
	useOpenGlListForWeapon=YSTRUE;
	useOpenGlGroundTexture=YSTRUE;
	useOpenGlRunwayLightTexture=YSTRUE;

	useOpenGlAntiAliasing=YSFALSE;

	airLod=0;
	gndLod=0;

	showUserName=0;
	showFps=YSFALSE;

	showIAS=YSTRUE;

	strcpy(defAirplane,"F-18E_SUPERHORNET");
	strcpy(defField,"AOMORI");
	strcpy(defStartPos,"MISAWA_RW10");

	aircraftReliability=95;
	aircraftTroubleFrequency=0.1;
}

void FsFlightConfig::SetDetailedMode(void)
{
	smkType=FSSMKSOLID;
	smkRemainTime=60.0;
	smkStep=1;
	drawShadow=YSTRUE;
	horizonGradation=YSTRUE;
	drawCloud=YSTRUE;
	cloudType=FSCLOUDSOLID;
	drawOrdinance=YSTRUE;
	drawCoarseOrdinance=YSFALSE;
	drawTransparency=YSTRUE;
	drawTransparentSmoke=YSTRUE;
	drawTransparentVapor=YSTRUE;
	drawTransparentLater=YSTRUE;
	drawFog=YSTRUE;
	zbuffQuality=2;
	airLod=0;

	useOpenGlGroundTexture=YSTRUE;
	useOpenGlRunwayLightTexture=YSTRUE;
	useOpenGlAntiAliasing=YSTRUE;
}

void FsFlightConfig::SetFastMode(void)
{
	smkType=FSSMKCIRCLE;
	smkRemainTime=15.0;
	smkStep=8;
	drawShadow=YSFALSE;
	horizonGradation=YSFALSE;
	drawCloud=YSFALSE;
	drawOrdinance=YSFALSE;
	drawCoarseOrdinance=YSTRUE;
	drawTransparency=YSFALSE;
	drawTransparentSmoke=YSFALSE;
	drawTransparentVapor=YSFALSE;
	drawTransparentLater=YSFALSE;
	drawFog=YSFALSE;
	zbuffQuality=0;
	airLod=2;

	useOpenGlGroundTexture=YSFALSE;
	useOpenGlRunwayLightTexture=YSFALSE;
	useOpenGlAntiAliasing=YSFALSE;
}

const char *const FsFlightConfig::keyWordSource[]=
{
	"REM",
	"AILASSIGN",
	"ELVASSIGN",
	"THRASSIGN",
	"RUDASSIGN",
	"SMOKETYPE",
	"SMOKETIME",
	"SMOKESTEP",
	"SIMPLHRZN",
	"DRWSHADOW",
	"DRAWCLOUD",
	"GBLACKOUT",
	"ACCURATET",
	"AUTORUDDR",
	"MIDAIRCOL",
	"HRIZNGRAD",
	"MOSTHROTT",
	"MOSRUDDER",
	"SHADWDEAD",
	"DRAWORDIN",
	"DRAWFOG__",
	"ZBUFFQUAL",
	"DEFAIRPLN",
	"HUDALWAYS",
	"JSCALIBRA",
	"VISIBILIT",
	"ENVIRONME",
	"COARSEWPN",
	"AIRLVODTL",
	"DEFFIELD_",
	"DEFSTPOS_",

	"DRWPLRNAM",
	"DRWLITDAY",

	"TRANSPARE",
	"TRSPVAPOR",
	"TRSPSMOKE",

	"LANDANWHR",

	"DRAWVJSTK",

	"GNDLVODTL",

	"JSWARNING",  // 2005/03/03

	"TRSPLATER",  // 2005/03/07
	"CAMERADLY",  // 2005/03/08

	"NOINSTPNL",  // 2005/03/29

	"CLOUDTYPE",  // 2005/05/30

	"OGLSTCLUD",  // 2005/06/16
	"OGLSTEXPL",  // 2005/06/16
	"OGLSTWEPN",  // 2005/06/18

	"LIGHTSRCD",  // 2005/10/07

	"RADARALTI",  // 2006/05/13
	"NOEXAIRVW",  // 2006/06/11

	"GDTEXTURE",  // 2006/07/03
	"RLTEXTURE",  // 2006/07/03

	"NOTAILSTK",  // 2007/05/27

	"FRMPERSEC",  // 2009/06/01

	"SMKPARTCL",  // 2010/01/23

	"ANTIALIAS",  // 2010/02/03

	"SIMPLEHUD",  // 2010/07/06

	"SHOWKIAS_",  // 2011/01/15

	"TRBLFREQ_",  // 2013/04/27
	"RELIABILI",  // 2013/04/27

	"PHONGSHAD",  // 2014/07/15

	"RLDAYVISI",  // 2017/05/02

	NULL
};
YsKeyWordList FsFlightConfig::keyWordList;

static const char *const cmdSmokeType[]=
{
	"NOODLE",
	"CIRCLE",
	"TOWEL",
	"SOLID",
	"NULL",

	NULL
};

YSRESULT FsFlightConfig::SendCommand(const char cmd[])
{
	char buf[256];
	int ac;
	char *av[32];
	int t;

	strncpy(buf,cmd,255);
	buf[255]=0;
	if(YsArguments(&ac,av,32,buf)==YSOK && ac>0)
	{
		int cmd;

		if(keyWordList.GetN()==0)
		{
			keyWordList.MakeList(keyWordSource);
		}

		cmd=keyWordList.GetId(av[0]);
		// if(YsCommandNumber(&cmd,av[0],cmdBasic)==YSOK)
		if(cmd>=0)
		{
			switch(cmd)
			{
			case 0: //"REM"
				return YSOK;
			case 1: //"AILASSIGN",
				return YSOK;
			case 2: //"ELVASSIGN",
				return YSOK;
			case 3: //"THRASSIGN",
				return YSOK;
			case 4: //"RUDASSIGN",
				return YSOK;
			case 5: //"SMOKETYPE",
				if(YsCommandNumber(&t,av[1],cmdSmokeType)==YSOK)
				{
					switch(t)
					{
					case 0:
						smkType=FSSMKNOODLE;
						break;
					case 1:
						smkType=FSSMKCIRCLE;
						break;
					case 2:
						smkType=FSSMKTOWEL;
						break;
					case 3:
						smkType=FSSMKSOLID;
						break;
					case 4:
						smkType=FSSMKNULL;
						break;
					default:
						// fsStderr.Printf("Unknown Smoke Type: %s\n",av[1]);
						break;
					}
					return YSOK;
				}
				else
				{
					return YSERR;
				}
			case 6: //"SMOKETIME",
				smkRemainTime=atof(av[1]);
				return YSOK;
			case 7: //"SMOKESTEP",
				smkStep=atoi(av[1]);
				return YSOK;
			case 8: //"SIMPLHRZN",
				horizonGradation=YsStrToBool(av[1]);
				horizonGradation=(horizonGradation==YSTRUE ? YSFALSE : YSTRUE);
				return YSOK;
			case 9: //"DRWSHADOW",
				drawShadow=YsStrToBool(av[1]);
				return YSOK;
			case 10: //"drawCloud",
				drawCloud=YsStrToBool(av[1]);
				return YSOK;
			case 11: //"GBLACKOUT"
				blackOut=YsStrToBool(av[1]);
				return YSOK;
			case 12: //"ACCURATET"
				accurateTime=YsStrToBool(av[1]);
				return YSOK;
			case 13: //	"AUTORUDDR",
				autoCoordination=YsStrToBool(av[1]);
				return YSOK;
			case 14: // "MIDAIRCOL",
				midAirCollision=YsStrToBool(av[1]);
				return YSOK;
			case 15: //"HRIZNGRAD",
				horizonGradation=YsStrToBool(av[1]);
				return YSOK;
			case 16: //"MOSTHROTT"
				return YSOK;
			case 17: //"MOSRUDDER"
				return YSOK;
			case 18: //	"SHADWDEAD",
				shadowOfDeadAirplane=YsStrToBool(av[1]);
				return YSOK;
			case 19: // "DRAWORDIN"
				drawOrdinance=YsStrToBool(av[1]);
				return YSOK;
			case 20: // "DRAWFOG__"
				drawFog=YsStrToBool(av[1]);
				return YSOK;
			case 21: // "ZBUFFQUAL"
				if(strcmp(av[1],"TRUE")==0)
				{
					zbuffQuality=1;
				}
				else if(strcmp(av[1],"FALSE")==0)
				{
					zbuffQuality=0;
				}
				else
				{
					zbuffQuality=atoi(av[1]);
				}
				return YSOK;
			case 22: // "DEFAIRPLN"
				strcpy(defAirplane,av[1]);
				return YSOK;
			case 23: // "HUDALWAYS"
				showHudAlways=YsStrToBool(av[1]);
				return YSOK;
			case 24: // "JSCALIBRA"
				additionalCalibration=YsStrToBool(av[1]);
				return YSOK;
			case 25: //	"VISIBILIT",
				return FsGetLength(fogVisibility,av[1]);
			case 26: //	"ENVIRONME",
				if(strcmp(av[1],"DAY")==0)
				{
					env=FSDAYLIGHT;
				}
				else if(strcmp(av[1],"NIGHT")==0)
				{
					env=FSNIGHT;
				}
				return YSOK;

			case 27: //	"COARSEWPN",
				drawCoarseOrdinance=YsStrToBool(av[1]);
				return YSOK;

			case 28: // "AIRLVODTL",
				airLod=atoi(av[1]);
				return YSOK;
			case 29: // "DEFFIELD_",
				strcpy(defField,av[1]);
				return YSOK;
			case 30: // "DEFSTPOS_",
				strcpy(defStartPos,av[1]);
				return YSOK;

			case 31: // "DRWPLRNAM",
				drawPlayerNameAlways=YsStrToBool(av[1]);
				return YSOK;
			case 32: //	"DRWLITDAY",
				drawLightsInDaylight=YsStrToBool(av[1]);
				return YSOK;

			case 33: //	"TRANSPARE",
				drawTransparency=YsStrToBool(av[1]);
				return YSOK;
			case 34: // "TRSPVAPOR",
				drawTransparentVapor=YsStrToBool(av[1]);
				return YSOK;
			case 35: // "TRSPSMOKE",
				drawTransparentSmoke=YsStrToBool(av[1]);
				return YSOK;

			case 36: // "LANDANWHR",
				canLandAnywhere=YsStrToBool(av[1]);
				return YSOK;

			case 37: //	"DRAWVJSTK",
				drawVirtualJoystick=YsStrToBool(av[1]);
				return YSOK;

			case 38: // "GNDLVODTL",
				gndLod=atoi(av[1]);
				return YSOK;

			case 39:  // "JSWARNING"
				showJoystickUnpluggedWarning=YsStrToBool(av[1]);
				return YSOK;

			case 40:  // "TRSPLATER",
				drawTransparentLater=YsStrToBool(av[1]);
				return YSOK;

			case 41: // "CAMERADLY",  // 2005/03/08
				externalCameraDelay=YsStrToBool(av[1]);
				break;

			case 42: // "NOINSTPNL",  // 2005/03/29
				useHudAlways=YsStrToBool(av[1]);
				break;

			case 43: // "CLOUDTYPE"
				if(strcmp(av[1],"FLAT")==0 || strcmp(av[1],"flat")==0)
				{
					cloudType=FSCLOUDFLAT;
				}
				else if(strcmp(av[1],"SOLID")==0 || strcmp(av[1],"solid")==0)
				{
					cloudType=FSCLOUDSOLID;
				}
				else
				{
					cloudType=FSNOCLOUD;
				}
				break;

			case 44: //	"OGLSTCLUD",  // 2005/06/16
				useOpenGlListForCloud=YsStrToBool(av[1]);
				break;
			case 45: // "OGLSTEXPL",  // 2005/06/16
				useOpenGlListForExplosion=YsStrToBool(av[1]);
				break;
			case 46: // "OGLSTWEPN",  // 2005/06/18
				useOpenGlListForWeapon=YsStrToBool(av[1]);
				break;

			case 47: // "LIGHTSRCD"   // 2005/10/07
				lightSourceDirection.Set(atof(av[1]),atof(av[2]),atof(av[3]));
				lightSourceDirection.Normalize();
				break;

			case 48: // "RADARALTI",  // 2006/05/13
				return FsGetLength(radarAltitudeLimit,av[1]);

			case 49: // "NOENMYVIW",  // 2006/06/11
				return FsGetBool(noExtAirView,av[1]);

			case 50: // "GDTEXTURE",  // 2006/07/03
				return FsGetBool(useOpenGlGroundTexture,av[1]);
			case 51: // "RLTEXTURE",  // 2006/07/03
				return FsGetBool(useOpenGlRunwayLightTexture,av[1]);
			case 52:  // "NOTAILSTK",  // 2007/05/27
				return FsGetBool(noTailStrike,av[1]);
			case 53: // "FRMPERSEC",  // 2009/06/01
				return FsGetBool(showFps,av[1]);

			case 54: // "SMKPARTCL",  // 2010/01/23
				return FsGetBool(useParticle,av[1]);

			case 55: // "ANTIALIAS",  // 2010/02/03
				return FsGetBool(useOpenGlAntiAliasing,av[1]);
			case 56: // "SIMPLEHUD",  // 2010/06/22
				return FsGetBool(useSimpleHud,av[1]);
			case 57: // SHOWKIAS_
				return FsGetBool(showIAS,av[1]);

			case 58: // "TRBLFREQ_",  // 2013/04/27
				aircraftTroubleFrequency=atof(av[1]);
				return YSOK;
			case 59: // "RELIABILI",  // 2013/04/27
				aircraftReliability=atoi(av[1]);
				return YSOK;

			case 60: // 	"PHONGSHAD",  // 2014/07/15
				return FsGetBool(perPixelShading,av[1]);

			case 61: // 	"RLDAYVISI",  // 2017/05/02
				return FsGetLength(drawLightsInDaylightVisibilityThr,av[1]);
			}
		}
		else
		{
			return YSERR;
		}
	}
	return YSOK;
}

YSRESULT FsFlightConfig::Load(const wchar_t fn[])
{
	char buf[256];

	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		YSRESULT res;
		res=YSOK;
		while(fgets(buf,256,fp)!=NULL)
		{
			if(SendCommand(buf)!=YSOK)
			{
				// fsStderr.Printf("Unrecognizable Line : %s\n",buf);
				res=YSERR;
			}
		}
		fclose(fp);
		return res;
	}
	return YSERR;
}


YSRESULT FsFlightConfig::Save(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"w");
	if(fp!=NULL)
	{
		switch(smkType)
		{
		case FSSMKNOODLE:
			fprintf(fp,"SMOKETYPE NOODLE\n");
			break;
		case FSSMKCIRCLE:
			fprintf(fp,"SMOKETYPE CIRCLE\n");
			break;
		case FSSMKTOWEL:
			fprintf(fp,"SMOKETYPE TOWEL\n");
			break;
		case FSSMKSOLID:
			fprintf(fp,"SMOKETYPE SOLID\n");
			break;
		case FSSMKNULL:
			fprintf(fp,"SMOKETYPE NULL\n");
			break;
		}

		fprintf(fp,"SMOKETIME %lf\n",smkRemainTime);

		fprintf(fp,"SMOKESTEP %d\n",smkStep);

		fprintf(fp,"DRWSHADOW %s\n",FsTrueFalseString(drawShadow));
		fprintf(fp,"HRIZNGRAD %s\n",FsTrueFalseString(horizonGradation));
		fprintf(fp,"DRAWCLOUD %s\n",FsTrueFalseString(drawCloud));
		switch(cloudType)
		{
		case FSNOCLOUD:
			fprintf(fp,"CLOUDTYPE NONE\n");
			break;
		case FSCLOUDFLAT:
			fprintf(fp,"CLOUDTYPE FLAT\n");
			break;
		case FSCLOUDSOLID:
			fprintf(fp,"CLOUDTYPE SOLID\n");
			break;
		}

		fprintf(fp,"GBLACKOUT %s\n",FsTrueFalseString(blackOut));
		fprintf(fp,"ACCURATET %s\n",FsTrueFalseString(accurateTime));

		fprintf(fp,"AUTORUDDR %s\n",FsTrueFalseString(autoCoordination));
		fprintf(fp,"MIDAIRCOL %s\n",FsTrueFalseString(midAirCollision));

		fprintf(fp,"SHADWDEAD %s\n",FsTrueFalseString(shadowOfDeadAirplane));
		fprintf(fp,"DRAWORDIN %s\n",FsTrueFalseString(drawOrdinance));
		fprintf(fp,"COARSEWPN %s\n",FsTrueFalseString(drawCoarseOrdinance));

		fprintf(fp,"DRAWFOG__ %s\n",FsTrueFalseString(drawFog));

		fprintf(fp,"ZBUFFQUAL %d\n",zbuffQuality);

		fprintf(fp,"DEFAIRPLN \"%s\"\n",defAirplane);

		fprintf(fp,"HUDALWAYS %s\n",FsTrueFalseString(showHudAlways));

		fprintf(fp,"JSCALIBRA %s\n",FsTrueFalseString(additionalCalibration));

		fprintf(fp,"VISIBILIT %.2lfm\n",fogVisibility);

		fprintf(fp,"AIRLVODTL %d\n",airLod);

		fprintf(fp,"DRWPLRNAM %s\n",FsTrueFalseString(drawPlayerNameAlways));
		fprintf(fp,"DRWLITDAY %s\n",FsTrueFalseString(drawLightsInDaylight));

		fprintf(fp,"TRANSPARE %s\n",FsTrueFalseString(drawTransparency));
		fprintf(fp,"TRSPVAPOR %s\n",FsTrueFalseString(drawTransparentVapor));
		fprintf(fp,"TRSPSMOKE %s\n",FsTrueFalseString(drawTransparentSmoke));


		fprintf(fp,"LANDANWHR %s\n",FsTrueFalseString(canLandAnywhere));

		fprintf(fp,"DRAWVJSTK %s\n",FsTrueFalseString(drawVirtualJoystick));

		switch(env)
		{
		case FSDAYLIGHT:
			fprintf(fp,"ENVIRONME DAY\n");
			break;
		case FSNIGHT:
			fprintf(fp,"ENVIRONME NIGHT\n");
			break;
		}

		fprintf(fp,"DEFFIELD_ \"%s\"\n",defField);
		fprintf(fp,"DEFSTPOS_ \"%s\"\n",defStartPos);

		fprintf(fp,"GNDLVODTL %d\n",gndLod);

		fprintf(fp,"JSWARNING %s\n",FsTrueFalseString(showJoystickUnpluggedWarning));

		fprintf(fp,"TRSPLATER %s\n",FsTrueFalseString(drawTransparentLater));

		fprintf(fp,"CAMERADLY %s\n",FsTrueFalseString(externalCameraDelay));

		fprintf(fp,"NOINSTPNL %s\n",FsTrueFalseString(useHudAlways));

		fprintf(fp,"OGLSTCLUD %s\n",FsTrueFalseString(useOpenGlListForCloud));
		fprintf(fp,"OGLSTEXPL %s\n",FsTrueFalseString(useOpenGlListForExplosion));
		fprintf(fp,"OGLSTWEPN %s\n",FsTrueFalseString(useOpenGlListForWeapon));

		fprintf(fp,"LIGHTSRCD %lf %lf %lf\n",
		    lightSourceDirection.x(),lightSourceDirection.y(),lightSourceDirection.z());

		fprintf(fp,"RADARALTI %.2lfm\n",radarAltitudeLimit);

		// NOEXAIRVW is not saved.  It is for Network Purpose.

		fprintf(fp,"GDTEXTURE %s\n",FsTrueFalseString(useOpenGlGroundTexture));
		fprintf(fp,"RLTEXTURE %s\n",FsTrueFalseString(useOpenGlRunwayLightTexture));

		fprintf(fp,"NOTAILSTK %s\n",FsTrueFalseString(noTailStrike));

		fprintf(fp,"FRMPERSEC %s\n",FsTrueFalseString(showFps));

		fprintf(fp,"SMKPARTCL %s\n",FsTrueFalseString(useParticle));

		fprintf(fp,"ANTIALIAS %s\n",FsTrueFalseString(useOpenGlAntiAliasing));

		fprintf(fp,"SIMPLEHUD %s\n",FsTrueFalseString(useSimpleHud));

		fprintf(fp,"SHOWKIAS_ %s\n",YsBoolToStr(showIAS));

		fprintf(fp,"TRBLFREQ_ %lf\n",aircraftTroubleFrequency);
		fprintf(fp,"RELIABILI %d\n",aircraftReliability);

		fprintf(fp,"PHONGSHAD %s\n",FsTrueFalseString(perPixelShading));

		fprintf(fp,"RLDAYVISI %lfm\n",drawLightsInDaylightVisibilityThr);

		fclose(fp);
		return YSOK;
	}

// ERRTRAP:
	if(fp!=NULL)
	{
		fclose(fp);
	}
	// fsStderr.Printf("Cannot Write Config File.");
	return YSERR;
}

