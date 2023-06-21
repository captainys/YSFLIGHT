#include <ysclass.h>
#include <ysunitconv.h>
#include <ysport.h>

#include <ysshelldnmident.h>

#include <ysscenery.h>

#include <fsdef.h>

#define FSSIMPLEWINDOW_MACRO_ONLY
#include <fssimplewindow.h>
#undef FSSIMPLEWINDOW_MACRO_ONLY

#include <fsairproperty.h>

#include "fsutil.h"
#include "fsnetutil.h"
#include "fsweather.h"
#include "fsnavaid.h"
#include "fsrecord.h"
#include "fsvisual.h"
#include "fsweapon.h"
#include "fsproperty.h"
#include "fsairplaneproperty.h"
#include "fsgroundproperty.h"
#include "fsnetwork.h"
#include "fsexistence.h"
#include "fsstdout.h"
#include "fscontrol.h"

#ifdef WIN32
#include <float.h>
#endif



// Memo
//  It's better to protect staState to prevent illegal state transition
//     Illegal FSDEADSPIN -> FS****
//             FSDEADFLATSPIN -> FS****
//             FSDEAD -> FS****


//Weapon Loading:
//LMTBYHDP (chLimitWeaponBySlot) is TRUE ->
//  The number of weapons that can be loaded is limited by the slots (except rockets).
//  One slot can store one unit.  When all slots are occupied, no more weapon can
//  be loaded.  chMaxNum(WeaponType) are ignored.
//
//LMTBYHDP (chLimitWeaponBySlot) is FALSE ->
//  The number of weapons that can be loaded is limited by chMaxNum(WeaponType).



#define ELVRESPONCE 2.0


// #define CRASHINVESTIGATION_MOVE
// #define CRASHINVESTIGATION_MOVE_M6
// #define CRASHINVESTIGATION_AUTOPILOT

////////////////////////////////////////////////////////////

FsWeaponSlotLoading::FsWeaponSlotLoading(void)
{
	Initialize();
}

void FsWeaponSlotLoading::Initialize(void)
{
	wpnType=FSWEAPON_GUN;
	nLoaded=0;
	containerWpnType=FSWEAPON_NULL;
	nContainerLoaded=0;
	fuelLoaded=0.0;
}

////////////////////////////////////////////////////////////

FsWeaponSlot::FsWeaponSlot()
{
	Initialize();
}

void FsWeaponSlot::Initialize(void)
{
	for(int i=0; i<FSWEAPON_NUMWEAPONTYPE; i++)
	{
		nLoad[i]=0;
		nSubLoad[i]=FsGetDefaultSubUnitPerLoadingUnit((FSWEAPONTYPE)i);
	}
	pos=YsOrigin();
	isExternal=YSTRUE;
}


// Implementation //////////////////////////////////////////
FsAirplaneProperty::FsAirplaneProperty()
{
	Initialize();
}

void FsAirplaneProperty::InitializeState(void)
{
	FsVehicleProperty::InitializeState();

	// State
	staVirtualButtonQueue.CleanUp();
	staState=FSFLYING;
	staHobbsTime=0.0;
	staStateTimer=0.0;
	staDiedOf=FSDIEDOF_NULL;
	staNeedTouchdownCheck=YSFALSE;
	staOutOfRunway=YSFALSE;

	staGndElevation=0.0;
	staFootAtMainGearL=YsOrigin();
	staFootAtMainGearR=YsOrigin();
	staFootAtWheel=YsOrigin();
	staGndNormal=YsYVec();
	staBaseElevation=0.0;


	staAb=YSFALSE;
	staThrottle=0.0;

	staSpoiler=0.0;
	staGear=1.0;
	staFlap=0.0;
	staBrake=0.0;
	staVgw=0.0;
	staThrVec=0.0;
	staThrRev=0.0;
	staBombBayDoor=0.0;

	staBeacon=YSTRUE;
	staNavLight=YSTRUE;
	staStrobe=YSTRUE;
	staLandingLight=YSTRUE;

	staPayload=0.0;
	staFuelLoad=0.0;

	staVelocity.Set(0.0,0.0,0.0);
	staAOA=0.0;
	staSSA=0.0;
	staMovingBackward=YSFALSE;

	staRho=1.25;  // Standard air at mean sea level.
	staLift=0.0;
	staDrag=0.0;
	staThrust=0.0;
	staV=0.0;
	staVHorizontal=0.0;

	staVPitch=0.0;
	staVYaw=0.0;
	staVRoll=0.0;

	staDVPitch=0.0;
	staDVYaw=0.0;
	staDVRoll=0.0;

	staRotorAngle=0.0;
	staWheelAngle=0.0;

	staRadarRange=0.0;   // Radar Off

	staVorKey[0]=YSNULLHASHKEY;
	staVorKey[1]=YSNULLHASHKEY;
	staNdbKey=YSNULLHASHKEY;
	staPendingVorYsfId[0]=0;
	staPendingVorYsfId[1]=0;
	staPendingNdbYsfId=0;
	staObs[0]=0.0;
	staObs[1]=0.0;
	staHdgBug=0.0;


	staVectorMarker=YSTRUE;

	// Temporary
	staWoc=FSWEAPON_GUN;
	staGunBullet=3000;
	staGunTimer=0.0;
	staFlare=20;
	staSmokeOil=0.0;
	for(int smkIdx=0; smkIdx<MaxNumSmokeGenerator; ++smkIdx)
	{
		staSmokeGenerator[smkIdx].col=chSmokeGenerator[smkIdx].defCol;
	}
	staAirTargetKey=YSNULLHASHKEY;
	staGroundTargetKey=YSNULLHASHKEY;
	staRecentlyFiredMissileId=-1;

	staG=0.0;
	staPrevG=0.0;
	staPrevPrevG=0.0;

	staLateralForce=0.0;
	staPrevLateralForce=0.0;



	staPState=FSGROUND;
	staPPos.Set(0.0,0.0,0.0);
	staPVelocity.Set(0.0,0.0,0.0);
	staPAtt.Set(0.0,0.0,0.0);

	staTDPos.Set(0.0,0.0,0.0);
	staTDVelocity.Set(0.0,0.0,0.0);
	staTDAtt.Set(0.0,0.0,0.0);

	staWeight=0.0;
	staTotalAerodynamicForce=YsOrigin();
	staTotalGravityForce=YsOrigin();

	staIndicatedClimbRatio=0.0;

	// For smooth network play
	for(int i=0; i<nNetCorrection; i++)
	{
		staNetCorrection[i]=YsOrigin();
		staNetCorrectionTime[i]=0.0;
	}

	ClearMalfunction();
}

void FsAirplaneProperty::Initialize(void)
{
	InitializeState();

	FsVehicleProperty::InitializeCharacteristic();

	chCategory=FSAC_UNKNOWN;
	chClass=FSCL_AIRPLANE;

	chIsJet=YSTRUE;
	chHasAb=YSTRUE;

	chHasStallHorn=YSTRUE;
	chHasGearHorn=YSTRUE;

	chThrAb=0.0;
	chThrMil=0.0;
	chPropEfficiency=0.7;
	chPropV0=30.0;
	chPropK=0.0;

	chStrobeBlankTime=0.9;
	chStrobeLightTime=0.1;
	chBeaconBlankTime=0.8;
	chBeaconLightTime=0.8;

	chCleanWeight=0.0;
	chMaxFuelLoad=0.0;
	chMaxPayload=0.0;
	chAbFuelConsume=0.0;
	chMilFuelConsume=0.0;
	chThrustReverser=0.0;

	chMaxNumGunBullet=0;
	chGunInterval=0.075;
	chMaxNumFlare=20;

	chMainGearL.Set(-3.0,-1.0,-3.0);
	chMainGearR.Set( 3.0,-1.0,-3.0);
	chWheel.Set(0.0,-1.0,2.0);
	chWheelRadius=0.3;
	chArrestingHook.Set(0.0,-3.0,-1.0);
	chNumGun=1;
	for(int i=0; i<MaxNumGun; i++)
	{
		chGunPosition[i].Set(1.0,0.0,5.0);
		chGunDirection[i].Set(0.0,0.0,1.0);
	}
	chLeadGunSight=YSTRUE;
	chNumSmokeGenerator=0;  // GetNumSmokeGenerator returns 1 as minimum regardless of chNumSmokeGenerator
	for(int i=0; i<MaxNumSmokeGenerator; i++)
	{
		chSmokeGenerator[i].pos=YsOrigin();
		chSmokeGenerator[i].defCol=YsWhite();
		staSmokeGenerator[i].col=YsWhite();
	}
	chVapor0.Set(5.0,0.0,0.0);
	chVapor1.Set(5.0,0.0,0.0);

	chClZero=0.0;
	chClSlope=0.0;
	chMaxAOA=0.0;
	chMinAOA=0.0;

	chMaxCdAOA=YsPi/4.0;
	chFlatClRange1=0.0;
	chFlatClRange2=0.0;
	chClDecay1=0.0;
	chClDecay2=0.0;

	chCdZero=0.0;
	chCdZeroMax=0.0;
	chVCritical=0.0;
	chVMax=0.0;
	chCdConst=0.0;

	chBrakeConst=0.0;
	chTireFrictionConst=0.0;

	chHasSpoiler=YSTRUE;
	chGearRetractable=YSTRUE;
	chHasVGW=YSFALSE;
	chClVgw=0.0;
	chCdVgw=0.0;
	chClFlap=0.0;
	chCdFlap=0.0;
	chCdGear=0.0;
	chCdSpoiler=0.0;

	chHasVGN=YSFALSE;

	chHasThrustVector=YSTRUE;
	chThrVec0=YsZVec();
	chThrVec1=YsZVec();
	chPostStallVPitch=0.0;
	chPostStallVRoll=0.0;
	chPostStallVYaw=0.0;

	chWingArea=0.0;

	chMaxInputAOA=0.0;
	chMaxInputSSA=0.0;
	chMaxInputROLL=0.0;

	chManSpeed1=0.0;
	chManSpeed2=0.0;
	chManSpeed3=100.0;

	chDirectAttitudeControlSpeed1=99998.0;
	chDirectAttitudeControlSpeed2=99999.0;
	chDirectAttitudeControlReqThr1=0.15;
	chDirectAttitudeControlReqThr2=0.8;

	chPitchManConst=0.0;
	chPitchStabConst=0.0;
	chYawManConst=0.0;
	chYawStabConst=0.0;
	chRollManConst=0.0;
	chNoLandingFlare=YSFALSE;

	chWeaponSlot.Set(0,NULL);
	staWeaponSlot.Set(0,NULL);
	chAAMVisible=YSTRUE;
	chAGMVisible=YSTRUE;
	chRocketVisible=YSTRUE;
	chBombVisible=YSTRUE;
	for(int i=0; i<FSWEAPON_NUMWEAPONTYPE; i++)
	{
		chWeaponShapeFileStatic[i].Set(L"");
		chWeaponShapeFileFlying[i].Set(L"");
	}

	chNumFlareDispenser=0;
	for(int i=0; i<MaxNumFlareDispenser; i++)
	{
		chFlareDispenser[i]=YsOrigin();
		chFlareDispensingVel[i]=YsOrigin();
	}

	chFlapPositionSet=YSFALSE;
	chFlapPosition.Set(0,NULL);
	chFlapPosition.Append(0.0);
	chFlapPosition.Append(0.25);
	chFlapPosition.Append(0.5);
	chFlapPosition.Append(0.75);
	chFlapPosition.Append(1.0);


	chTailStrikePitchIsComputed=YSFALSE;
	chTailStrikePitch=YsPi/2.0;
	chGroundStaticPitch=0.0;

	chGunPower=1;
	chBulInitSpeed=340.0*5.0;  // Mach 5.0
	chBulRange=3000.0;      // 5000m
	chRadarCrossSection=1.0;
	chAAMRange=5000.0;
	chAGMRange=5000.0;
	chRocketRange=10000.0;
	chBombInBombBay=YSFALSE;
	chBombBayRcs=0.0;

	chEnginePropCmd.CleanUp();




	// Control
	ctlGear=0.0;
	ctlBrake=0.0;
	ctlSpoiler=0.0;
	ctlAb=YSFALSE;
	ctlThrottle=0.0;
	ctlFlap=0.0;
	ctlVgw=0.0;
	ctlThrVec=0.0;
	ctlThrRev=0.0;
	ctlAutoVgw=YSTRUE;
	chAutoVgwSpeed1=340.0*0.3;
	chAutoVgwSpeed2=340.0*0.8;
	ctlBombBayDoor=0.0;

	ctlElevator=0.0;
	ctlElvTrim=0.0;
	ctlAileron=0.0;

	ctlDirectPitch=0.0;
	ctlDirectRoll=0.0;
	ctlDirectYaw=0.0;

	ctlRudderUser=0.0;
	ctlRudderControl=0.0;

	ctlSpeedControllerSwitch=YSFALSE;
	ctlSpeedControllerCanUseAfterburner=YSTRUE;
	ctlSpeedControllerThrottleCap=1.0;
	ctlSpeedControllerInput=0.0;
	ctlIntegralSpdErr=0.0;
	ctlPrevSpdErr=0.0;
	ctlBankControllerSwitch=YSFALSE;
	ctlBankControllerInput=YsYVec();
	ctlBankControllerRollRateLimit=YsInfinity;
	ctlPitchControllerSwitch=YSFALSE;
	ctlPitchControllerInput=0.0;
	ctlGControllerSwitch=YSFALSE;
	ctlGControllerInput=1.0;
	ctlGControllerSmoother=1.0;
	ctlGControllerMinAOALimit=-YsPi/2.0;
	ctlGControllerMaxAOALimit= YsPi/2.0;
	ctlAOAControllerSwitch=YSFALSE;
	ctlAOAControllerInput=0.0;
	ctlAOAControllerGLimit=9.0;

	ReleaseVirtualButton();


	chNeutralHeadAttitude.Set(0.0,0.0,0.0);
	chHasHud=YSTRUE;
	chHasInstPanel=YSFALSE;
	chUseBothHudAndInstPanel=YSFALSE;
	chInstPanelFileName.Set(L"");
	chShowInstPanelInCockpit=YSTRUE;
	chShowHudInCockpit=YSTRUE;


	chScreenCenterLocked=YSFALSE;  // If SCRNCNTR command has issued, it is locked.  Otherwise, INSTPANL or ISPNLHUD may change chScreenCenter
	chScreenCenter.Set(0.0,-0.33333333);


	ch3dInstPanelPosSet=YSFALSE;
	ch3dInstPanelPos=YsOrigin();
	ch3dInstPanelAtt.Set(0.0,0.0,0.0);
	ch3dInstPanelScale=1.0;


	chHasBombingRadar=YSTRUE;
	chHasGroundRadar=YSTRUE;
	chHasAirRadar=YSTRUE;



	refSpdCruise=0.0;
	refAltCruise=0.0;
	refThrCruise=1.0;
	refSpdLanding=0.0;
	refAOALanding=0.0;
	refLNGRunway=0.0;
	refThrLanding=0.0;

	RemakeMatrix();


	// Aircraft Carrier
	staOnThisCarrier=NULL;
	staCatapulted=YSFALSE;
	staArrested=YSFALSE;
}

void FsAirplaneProperty::ClearMalfunction(void)
{
	staAutoPilotInop=YSFALSE;
	staFlapInop=YSFALSE;
	staRadarInop=YSFALSE;
	staVorInop=YSFALSE;
	staAdfInop=YSFALSE;
	staAirspeedInop=YSFALSE;
	staAirspeedStuckAltitude=0.0;
	staAirspeedStuckAirspeed=0.0;
	staAltimeterInop=YSFALSE;
	staAltimeterStuckAltitude=0.0;
	staVSIInop=YSFALSE;
	staAttitudeIndicatorInop=YSFALSE;
	staAttitudeIndicatorStuckAttitude=YsZeroAtt();
	staPowerLoss=YSFALSE;
	staEngineOutputCap=0.0;                // Engine output limited to this value (%).
	staHUDFlicker=YSFALSE;
	staHUDVisible=YSTRUE;
	staHUDFlickerTimer=0.0;
}

void FsAirplaneProperty::ReleaseVirtualButton(void)
{
	ctlFireWeaponButton=YSFALSE;
	pCtlFireWeaponButton=YSFALSE;
	ctlFireGunButton=YSFALSE;
	pCtlFireGunButton=YSFALSE;
	ctlFireAAMButton=YSFALSE;
	pCtlFireAAMButton=YSFALSE;
	ctlFireAGMButton=YSFALSE;
	pCtlFireAGMButton=YSFALSE;
	ctlFireRocketButton=YSFALSE;
	pCtlFireRocketButton=YSFALSE;
	ctlDropBombButton=YSFALSE;
	pCtlDropBombButton=YSFALSE;
	ctlDispenseFlareButton=YSFALSE;
	pCtlDispenseFlareButton=YSFALSE;
	ctlCycleWeaponButton=YSFALSE;
	pCtlCycleWeaponButton=YSFALSE;
	ctlSmokeButton=YSFALSE;
	pCtlSmokeButton=YSFALSE;
	ctlCycleSmokeSelectorButton=YSFALSE;
	pCtlCycleSmokeSelectorButton=YSFALSE;
	ctlSmokeSelector=255;
}

YSBOOL FsAirplaneProperty::ShowInstPanelInCockpit(void) const
{
	return chShowInstPanelInCockpit;
}

YSBOOL FsAirplaneProperty::ShowHudInCockpit(void) const
{
	return chShowHudInCockpit;
}

YSBOOL FsAirplaneProperty::IsJet(void) const
{
	return chIsJet;
}

YSBOOL FsAirplaneProperty::HasStallHorn(void) const
{
	return chHasStallHorn;
}

YSBOOL FsAirplaneProperty::HasGearHorn(void) const
{
	return chHasGearHorn;
}

double FsAirplaneProperty::GetThrustWeightRatio(void) const
{
	if(IsJet()==YSTRUE)
	{
		return chThrAb/(GetTotalWeight()*FsGravityConst);
	}
	else
	{
		return 0.0;
	}
}

FSAIRPLANECATEGORY FsAirplaneProperty::GetAirplaneCategory(void) const
{
	return chCategory;
}

FSAIRCRAFTCLASS FsAirplaneProperty::GetAircraftClass(void) const
{
	return chClass;
}

YSBOOL FsAirplaneProperty::IsTailDragger(void) const
{
	if(chWheel.z()<(chMainGearL.z()+chMainGearR.z())/2.0)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsAirplaneProperty::CanManeuverPostStall(void) const
{
	if(YsTolerance<chPostStallVPitch ||
	   YsTolerance<chPostStallVYaw ||
	   YsTolerance<chPostStallVRoll)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::NoLandingFlare(void) const
{
	return chNoLandingFlare;
}

double FsAirplaneProperty::GetCdSpoiler(void) const
{
	return chCdSpoiler;
}

FSFLIGHTSTATE FsAirplaneProperty::GetState(void) const
{
	return staState;
}

FSFLIGHTSTATE FsAirplaneProperty::GetFlightState(void) const
{
	return GetState();
}

void FsAirplaneProperty::SetState(FSFLIGHTSTATE sta,FSDIEDOF diedOf)
{
	SetFlightState(sta,diedOf);
}

void FsAirplaneProperty::SetFlightState(FSFLIGHTSTATE sta,FSDIEDOF diedOf)
{
	// Don't check 'never alive again' here.
	// There'll be problem during replaying record.
	// Instead, 'never alive again' test is done in CalculateGround function.
	//if((State==FSDEAD || State==FSDEADSPIN || State==FSDEADFLATSPIN) &&
	//   (sta!=FSDEAD && sta!=FSDEADSPIN && sta!=FSDEADFLATSPIN))
	//{
	//	// Never alive again.
	//	return YSERR;
	//}

	if(staDiedOf==FSDIEDOF_NULL &&
	   staState!=FSDEAD && staState!=FSDEADSPIN && staState!=FSDEADFLATSPIN && staState!=FSOVERRUN &&
	   (sta==FSDEAD || sta==FSDEADSPIN || sta==FSDEADFLATSPIN || sta==FSOVERRUN))
	{
		staDiedOf=diedOf;
	}

	staState=sta;

	if(staState==FSOVERRUN)
	{
		staOutOfRunway=YSTRUE;
	}
}

const double &FsAirplaneProperty::GetVariableGeometryWingState(void) const
{
	return staVgw;
}

void FsAirplaneProperty::Crash(FSDIEDOF diedOf)
{
	staDamageTolerance=0;
	SetState(FSDEAD,diedOf);
	printf("Died of: %d\n",diedOf);
	printf("Position: %s\n",GetPosition().Txt());
}

void FsAirplaneProperty::Overrun(void)
{
	SetState(FSOVERRUN,FSDIEDOF_OVERRUN);
}

YSBOOL FsAirplaneProperty::IsAlive(void) const
{
	if(staState!=FSDEAD)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::IsActive(void) const
{
	FSFLIGHTSTATE state;
	state=staState;
	if(state!=FSDEAD && state!=FSDEADSPIN && state!=FSDEADFLATSPIN && state!=FSOVERRUN)
	// Don't include stall.  This function gives YSTRUE only when the airplane is in unsurvivable state.
	{
		return YSTRUE;
	}
	return YSFALSE;
}

FSDIEDOF FsAirplaneProperty::GetDiedOf(void) const
{
	return staDiedOf;
}

YSBOOL FsAirplaneProperty::GetNeedTouchdownCheck(void) const
{
	return staNeedTouchdownCheck;
}

void FsAirplaneProperty::ClearNeedTouchdownCheck(void)
{
	staNeedTouchdownCheck=YSFALSE;
}

void FsAirplaneProperty::SetOutOfRunway(YSBOOL oor)
{
	staOutOfRunway=oor;
}

YSBOOL FsAirplaneProperty::IsOutOfRunway(void) const
{
	return staOutOfRunway;
}

YSRESULT FsAirplaneProperty::LoadProperty(const wchar_t fn[])
{
	FILE *fp;
	char dat[256];

	// For taking backward compatibility, the following four commands,
	// "INITIAAM", "INITIAGM", "INITBOMB", and "INITRCKT" must be sent after
	// loading everything.

	YsArray <YsString,8> initLoading;
	YsString str;



	fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		while(fgets(dat,256,fp)!=NULL)
		{
			if(dat[0]=='I' && dat[1]=='N' && dat[2]=='I' && dat[3]=='T')
			{
				if(strncmp(dat+4,"IAAM",4)==0 || strncmp(dat+4,"IAGM",4)==0 ||
				   strncmp(dat+4,"BOMB",4)==0 || strncmp(dat+4,"RCKT",4)==0)
				{
					str.Set(dat);
					initLoading.Append(str);
					goto NEXTLINE;
				}
			}

			if(SendCommand(dat)!=YSOK)
			{
				fclose(fp);
				return YSERR;
			}
		NEXTLINE:
			;
		}
		fclose(fp);

		int i;
		for(i=0; i<initLoading.GetN(); i++)
		{
			SendCommand(initLoading[i]);
		}

		return YSOK;
	}
	return YSERR;
}

/* static */ FSFLIGHTSTATE FsAirplaneProperty::StringToState(const char str[])
{
	if(0==strcmp(str,"FSFLYING"))
	{
		return FSFLYING;
	}
	if(0==strcmp(str,"FSGROUND"))
	{
		return FSGROUND;
	}
	if(0==strcmp(str,"FSSTALL"))
	{
		return FSSTALL;
	}
	if(0==strcmp(str,"FSDEAD"))
	{
		return FSDEAD;
	}
	if(0==strcmp(str,"FSDEADSPIN"))
	{
		return FSDEADSPIN;
	}
	if(0==strcmp(str,"FSDEADFLATSPIN"))
	{
		return FSDEADFLATSPIN;
	}
	if(0==strcmp(str,"FSGROUNDSTATIC"))
	{
		return FSGROUNDSTATIC;
	}
	if(0==strcmp(str,"FSOVERRUN"))
	{
		return FSOVERRUN;
	}
	return FSDEAD;
}

/* static */ const char *FsAirplaneProperty::StateToString(FSFLIGHTSTATE sta)
{
	switch(sta)
	{
	case FSFLYING:
		return "FSFLYING";
	case FSGROUND:
		return "FSGROUND";
	case FSSTALL:
		return "FSSTALL";
	case FSDEAD:
		return "FSDEAD";
	case FSDEADSPIN:
		return "FSDEADSPIN";
	case FSDEADFLATSPIN:
		return "FSDEADFLATSPIN";
	case FSGROUNDSTATIC:
		return "FSGROUNDSTATIC";
	case FSOVERRUN:
		return "FSOVERRUN";
	default:
		break;
	}
	return "FSDEAD";
}

void FsAirplaneProperty::Move(
    const double &dt,const YsArray <FsGround *> &carrierList,const FsWeather &weather)
{
	int i;

	BeforeMove();

#ifdef CRASHINVESTIGATION_MOVE
	printf("M1 %d\n",GetState());
#endif

	if(GetFlightState()==FSOVERRUN)
	{
		ctlThrottle=0.0;
		ctlAb=YSFALSE;
		SetVelocity(YsOrigin());
	}

#ifdef CRASHINVESTIGATION_MOVE
	printf("M2\n");
#endif

	if(chGearRetractable==YSTRUE)
	{
		staGear    =MoveDevice(ctlGear    ,staGear    ,1.0/2.0,dt);
	}
	else
	{
		staGear=1.0;
	}

	staBrake   =MoveDevice(ctlBrake   ,staBrake   ,GetBrakeMovingRate(),dt);
	if(chHasSpoiler==YSTRUE)
	{
		staSpoiler =MoveDevice(ctlSpoiler ,staSpoiler ,1.0/2.0,dt);
	}
	else
	{
		staSpoiler=0.0;
	}

#ifdef CRASHINVESTIGATION_MOVE
	printf("M3\n");
#endif

	staThrottle=MoveDevice(ctlThrottle,staThrottle,1.0/0.5,dt);
	if(YSTRUE==staPowerLoss && staEngineOutputCap<staThrottle)
	{
		staThrottle=staEngineOutputCap;
	}
	if(YSTRUE!=staFlapInop)
	{
		staFlap    =MoveDevice(ctlFlap    ,staFlap    ,1.0/2.0,dt);
	}
	staVgw     =MoveDevice(ctlVgw     ,staVgw     ,1.0/5.0,dt);
	staThrVec  =MoveDevice(ctlThrVec  ,staThrVec  ,1.0/1.0,dt);
	staThrRev  =MoveDevice(ctlThrRev  ,staThrRev  ,1.0/1.0,dt);
	staBombBayDoor=MoveDevice(ctlBombBayDoor,staBombBayDoor,1.0/3.0,dt);

	if(staState==FSGROUND)
	{
		const double dWheel=staV*dt/chWheelRadius;
		if(YSTRUE!=staMovingBackward)
		{
			staWheelAngle-=dWheel;
		}
		else
		{
			staWheelAngle+=dWheel;
		}
	}

#ifdef CRASHINVESTIGATION_MOVE
	printf("M4\n");
#endif

	for(i=0; i<chTurret.GetN(); i++)
	{
		staTurret[i].h=MoveDevice(staTurret[i].ctlH,staTurret[i].h,chTurret[i].vh,dt);
		staTurret[i].p=MoveDevice(staTurret[i].ctlP,staTurret[i].p,chTurret[i].vp,dt);
	}

	if(IsActive()!=YSTRUE)
	{
		staThrottle=0.0;
	}


	staRotorAngle=staRotorAngle+(2.0*YsPi)*dt*(staThrottle+0.1)+YsPi/50.0;

#ifdef CRASHINVESTIGATION_MOVE
	printf("M4.5\n");
#endif

	CalculateAutoPilot(dt);

#ifdef CRASHINVESTIGATION_MOVE
	printf("M5\n");
#endif


	if(0<chRealProp.GetN())
	{
		const YSBOOL engineOut=(YsTolerance<staFuelLoad ? YSFALSE : YSTRUE);

		YsVec3 relVelAirframe;
		staInverseMatrix.Mul(relVelAirframe,staVelocity,0.0);
		for(auto realPropIdx : chRealProp.AllIndex())
		{
			auto &realProp=chRealProp[realPropIdx];
			realProp.SetRho(staRho);
			realProp.Move(relVelAirframe,staThrottle,dt,engineOut);
			realProp.ControlPitch(staPropLever[realPropIdx],dt);
		}
	}


	if(YSTRUE!=staPowerLoss)
	{
		staAb=ctlAb;
	}
	else
	{
		staAb=YSFALSE;
	}
	CalculateCurrentState();
	CalculateFuelConsumption(dt);
	CalculateGravitationalForce();
	AdjustGravitationalForceForSlope();

#ifdef CRASHINVESTIGATION_MOVE
	printf("M6\n");
#endif


	// Euler's Method

#ifdef CRASHINVESTIGATION_MOVE_M6
	printf("M6.1\n");
#endif
	CalculateForce();
#ifdef CRASHINVESTIGATION_MOVE_M6
	printf("M6.2\n");
#endif
	CalculateTranslation(dt);  // CalculateTranslation,CalculateRotation,RemakeMatrix
#ifdef CRASHINVESTIGATION_MOVE_M6
	printf("M6.3\n");
#endif
	CalculateRotationalAcceleration();
#ifdef CRASHINVESTIGATION_MOVE_M6
	printf("M6.4\n");
#endif
	CalculateRotation(dt);     // must be called in this order, without calling other
#ifdef CRASHINVESTIGATION_MOVE_M6
	printf("M6.5\n");
#endif
	ApplyNetCorrection(dt);
#ifdef CRASHINVESTIGATION_MOVE_M6
	printf("M6.6\n");
#endif
	RemakeMatrix();            // functions.

#ifdef CRASHINVESTIGATION_MOVE
	printf("M7\n");
#endif


	// RK4 Method
	//CalculateRungeKutta4(dt);
	//RemakeMatrix();
	// Maybe I can try after releasing next one.


	CalculateCarrierLanding(dt,carrierList);
	CalculateWeather(dt,weather);
	CalculateGround(dt);
	CalculateStall();

#ifdef CRASHINVESTIGATION_MOVE
	printf("M8\n");
#endif

	staPrevPrevG=staPrevG;
	staPrevG=staG;
	staG=GetG();

	staPrevLateralForce=staLateralForce;
	staLateralForce=GetLateralForce();

#ifdef CRASHINVESTIGATION_MOVE
	printf("M9\n");
#endif

	if(staState==staPState)
	{
		staStateTimer+=dt;
	}
	else
	{
		staStateTimer=0.0;
	}

	staHobbsTime+=dt;

	if(YSTRUE==staHUDFlicker)
	{
		staHUDFlickerTimer-=dt;
		if(0.0>staHUDFlickerTimer)
		{
			YsFlip(staHUDVisible);
			staHUDFlickerTimer=(double)rand()/(double)RAND_MAX;
		}
	}
}

void FsAirplaneProperty::MoveTimer(const double &dt)
{
	staHobbsTime+=dt;
}

void FsAirplaneProperty::PressVirtualButton(VIRTUALBUTTON btn)
{
	staVirtualButtonQueue.Append(btn);
}

YSRESULT FsAirplaneProperty::CycleWeaponOfChoice(void)
{
	int i;
	FSWEAPONTYPE woc;
	woc=GetWeaponOfChoice();

	for(i=0; i<FSWEAPON_NUMWEAPONTYPE; i++)
	{
		if(woc==FSWEAPON_GUN)
		{
			woc=FSWEAPON_AIM9;
		}
		else if(woc==FSWEAPON_AIM9)
		{
			woc=FSWEAPON_AIM9X;
		}
		else if(woc==FSWEAPON_AIM9X)
		{
			woc=FSWEAPON_AIM120;
		}
		else if(woc==FSWEAPON_AIM120)
		{
			woc=FSWEAPON_AGM65;
		}
		else if(woc==FSWEAPON_AGM65)
		{
			woc=FSWEAPON_ROCKET;
		}
		else if(woc==FSWEAPON_ROCKET)
		{
			woc=FSWEAPON_BOMB;
		}
		else if(woc==FSWEAPON_BOMB)
		{
			woc=FSWEAPON_BOMB250;
		}
		else if(woc==FSWEAPON_BOMB250)
		{
			woc=FSWEAPON_BOMB500HD;
		}
		else if(woc==FSWEAPON_BOMB500HD)
		{
			woc=FSWEAPON_FLARE;
		}
		else if(woc==FSWEAPON_FLARE)
		{
			woc=FSWEAPON_FUELTANK;
		}
		else if(woc==FSWEAPON_FUELTANK)
		{
			woc=FSWEAPON_SMOKE;
		}
		else if(woc==FSWEAPON_SMOKE)
		{
			woc=FSWEAPON_GUN;
		}

		if(SetWeaponOfChoice(woc)==YSOK)
		{
			return YSOK;
		}
	}
	return YSERR;
}

void FsAirplaneProperty::CycleSmokeSelector(void)
{
	if(255==ctlSmokeSelector)
	{
		ctlSmokeSelector=1;
	}
	else if(128>ctlSmokeSelector && (1<<(GetNumSmokeGenerator()-1))>ctlSmokeSelector)
	{
		ctlSmokeSelector<<=1;
	}
	else
	{
		ctlSmokeSelector=255;
	}
}

YSRESULT FsAirplaneProperty::SetWeaponOfChoice(FSWEAPONTYPE woc)
{
	switch(woc)
	{
	case FSWEAPON_GUN:
		if(GetNumWeapon(FSWEAPON_GUN)+GetNumPilotControlledTurretBullet()<=0)
		{
			return YSERR;
		}
		break;
	default:
		if(GetNumWeapon(woc)<=0)
		{
			return YSERR;
		}
		break;
	}
	staWoc=woc;
	return YSOK;
}

FSWEAPONTYPE FsAirplaneProperty::GetWeaponOfChoice(void) const
{
	return staWoc;
}

void FsAirplaneProperty::BeforeMove(void)
{
	staPState=staState;
	staPPos=staPosition;
	staPAtt=staAttitude;
	staPVelocity=staVelocity;
}

void FsAirplaneProperty::RecordClimbRatio(const double &dt)
{
	if(YSTRUE!=staVSIInop)
	{
		// 500ft / min  = 152.4m  / min  = 2.54m / sec
		// 200ft / min  =  60.96m / min

		double diff;

		diff=(staVelocity.y()-staIndicatedClimbRatio);
		diff=YsBound(diff,-2.0,2.0);

		staIndicatedClimbRatio+=diff*dt;


		staIndicatedClimbRatio=YsBound(staIndicatedClimbRatio,-20.32,20.32);
	}
	else
	{
		const double mov=YsUnitConv::FTtoM(100.0)*dt;
		if(mov<staIndicatedClimbRatio)
		{
			staIndicatedClimbRatio-=mov;
		}
		else if(staIndicatedClimbRatio<-mov)
		{
			staIndicatedClimbRatio+=mov;
		}
		else
		{
			staIndicatedClimbRatio=0.0;
		}
	}
}

const double &FsAirplaneProperty::GetClimbRatioWithTimeDelay(void) const
{
	return staIndicatedClimbRatio;
}

const double FsAirplaneProperty::GetClimbRatio(void) const // <- For some reason, const double & doesn't work. Bug of VC2010?
{
	return staVelocity.y();
}

YSBOOL FsAirplaneProperty::CheckTouchDownAndLayOnGround(double &gDistance)
{
	YsVec3 nose,left,right;
	YsVec3 nosevc,leftvc,rightvc;
	YsVec3 nosert,leftrt,rightrt;
	YsVec3 tail,lateral,ev,uv;
	double dy;

	nose=staMatrix*chWheel;
	left=staMatrix*chMainGearL;
	right=staMatrix*chMainGearR;

	nosevc.Set(0.0,chWheel.y(),0.0);
	leftvc.Set(0.0,chMainGearL.y(),0.0);
	rightvc.Set(0.0,chMainGearR.y(),0.0);
	nosevc=staMatrix*nosevc;
	leftvc=staMatrix*leftvc;
	rightvc=staMatrix*rightvc;

	const double gDistLeft=left.y()-staFootAtMainGearL.y();
	const double gDistRight=right.y()-staFootAtMainGearR.y();
	const double gDistWheel=nose.y()-staFootAtWheel.y();

	if(YsTolerance<gDistWheel &&
	   YsTolerance<gDistLeft &&
	   YsTolerance<gDistRight)
	{
		gDistance=YsSmaller(gDistWheel,gDistLeft);
		gDistance=YsSmaller(gDistance,gDistRight);
		return YSFALSE;
	}
	else
	{
		YSBOOL diggingIn=YSFALSE;
		// Digging In?
		{
			const YsVec3 v1=staFootAtMainGearR-staFootAtWheel;
			const YsVec3 v2=staFootAtMainGearL-staFootAtWheel;

			YsVec3 gndNom=v1^v2;
			if(gndNom.y()<0.0)
			{
				gndNom=-gndNom;
			}
			gndNom.Normalize();

			YsVec3 vNom;
			GetVelocity(vNom);
			vNom.Normalize();

			if(cos(YsPi*91.0/180.0)>vNom*gndNom)
			{
				diggingIn=YSTRUE;
			}
		}



		gDistance=0.0;

		/* Phase 1 If airspeed is faster than chManSpeed1, allow
		   airplane to be lifted a little bit. */
		double deepest,airspeed;
		deepest=YsSmaller(gDistWheel,gDistLeft);
		deepest=YsSmaller(deepest,gDistRight);
		deepest=-deepest;
		airspeed=GetVelocity();

		if(YSTRUE!=diggingIn)
		{
			if(airspeed>chManSpeed2)
			{
				staPosition.SetY(staPosition.y()+deepest);
				return YSTRUE;
			}
			else if(airspeed>chManSpeed1)
			{
				double dy;
				dy=deepest*(airspeed-chManSpeed1)/(chManSpeed2-chManSpeed1);
				staPosition.SetY(staPosition.y()+dy);
			}
		}


		/* Phase 2 Adjust attitude */
		nose=chWheel;  // Force symmetric
		nose.SetX((chMainGearL.x()+chMainGearR.x())/2.0);
		nose=staMatrix*nose;

		nose.Set(nose.x(),YsGreater(nose.y(),staFootAtWheel.y()),nose.z());
		left.Set(left.x(),YsGreater(left.y(),staFootAtMainGearL.y()),left.z());
		right.Set(right.x(),YsGreater(right.y(),staFootAtMainGearR.y()),right.z());

		nosert=nose-nosevc;
		leftrt=left-leftvc;
		rightrt=right-rightvc;

		tail=(leftrt+rightrt)/2.0;

		if(chWheel.z()>chMainGearL.z())
		{
			ev=nosert-tail;
		}
		else
		{
			ev=tail-nosert;
		}

		YsVec3 vSave;
		staInverseMatrix.Mul(vSave,staVelocity,0.0);

		lateral=leftrt-rightrt;
		uv=lateral^ev;

		staAttitude.SetTwoVector(ev,uv);

		RemakeMatrix();

		staMatrix.Mul(staVelocity,vSave,0.0);



		/* Phase 3 Adjust Position */
		nose=staMatrix*chWheel;
		left=staMatrix*chMainGearL;
		right=staMatrix*chMainGearR;

		dy=0.0;
		if(nose.y()<staFootAtWheel.y())
		{
			dy=YsGreater(staFootAtWheel.y()-nose.y(),dy);
		}
		if(left.y()<staFootAtMainGearL.y())
		{
			dy=YsGreater(staFootAtMainGearL.y()-left.y(),dy);
		}
		if(right.y()<staFootAtMainGearR.y())
		{
			dy=YsGreater(staFootAtMainGearR.y()-right.y(),dy);
		}
		staPosition.SetY(staPosition.y()+dy);
		RemakeMatrix();
		return YSTRUE;
	}
}

void FsAirplaneProperty::CalculateGroundDescelerateDistanceAndTime(double &dist,double &tm,double v0,const double vTarget) const
{
	const double maxBrakeForce=CalculateForceByBrake(1.0);
	const double A=maxBrakeForce/GetTotalWeight();
	const double T0=1.0/GetBrakeMovingRate();  // Time to take brake to be fully effective.

	// t=0 to T0
	//     v0-0.5
	//     a=kt,  A=kT0 ->  k=A/T0
	//     v=v0-0.5*k*t*t
	//     d=v0t-(1/6)kttt
	const double k=A/T0;

	// Does it stop before t=T0?
	{
		// v0-0.5ktt=vTarget
		// v0-vTarget=0.5ktt
		// t=sqrt(2.0*(v0-vTarget)/k)
		const double t=sqrt(2.0*(v0-vTarget)/k);
		if(t<T0)
		{
			dist=v0*t-(1.0/6.0)*k*t*t*t;
			tm=t;
			return;
		}
	}

	const double dist0=(v0-(1.0/6.0)*k*T0*T0)*T0;
	const double V1=v0-0.5*k*T0*T0;

	const double t=(V1-vTarget)/A;
	const double dist1=(V1-0.5*A*t)*t;

	dist=dist0+dist1;
	tm=T0+t;
}

void FsAirplaneProperty::CalculateGroundStopDistanceAndTime(double &dist,double &tm,double v0) const
{
	CalculateGroundDescelerateDistanceAndTime(dist,tm,v0,0.0);
}

void FsAirplaneProperty::CalculateCurrentState(void)
{
	staV=staVelocity.GetLength();
	staVHorizontal=sqrt(YsSqr(staVelocity.x())+YsSqr(staVelocity.z()));

	YsVec3 relVec;
	staInverseMatrix.Mul(relVec,staVelocity,0.0);

	if(relVec.z()<0.0)
	{
		if(IsOnGround()==YSTRUE)   // 2005/10/11 Prevent Su-27's back spring
		{
			relVec=-relVec;
		}
		staMovingBackward=YSTRUE;
	}
	else
	{
		staMovingBackward=YSFALSE;
	}


	if(relVec.GetSquareLength()>=FsMinimumAirspeed)
	{
		if(staV>YsTolerance)
		{
			relVec=relVec/staV;
		}

		staAOA=atan2(-relVec.y(),relVec.z());
		staSSA=atan2( relVec.x(),relVec.z());
	}
	else
	{
		staAOA=0.0;
		staSSA=0.0;
	}

	staRho=FsGetAirDensity(staPosition.y()+staBaseElevation);
//printf("AOA %lf SSA %lf\n",YsRadToDeg(staAOA),YsRadToDeg(staSSA));
}

void FsAirplaneProperty::CalculateFuelConsumption(const double &dt)
{
	double dFuel;
	if(chHasAb==YSTRUE && staAb==YSTRUE)
	{
		dFuel=chAbFuelConsume;
	}
	else
	{
		dFuel=chMilFuelConsume*staThrottle;
	}


	int i;
	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		if(staWeaponSlot[i].wpnType==FSWEAPON_FUELTANK &&
		   0<staWeaponSlot[i].nLoaded &&
		   YsTolerance<staWeaponSlot[i].fuelLoaded)
		{
			staWeaponSlot[i].fuelLoaded-=dFuel*dt;
			if(0.0>staWeaponSlot[i].fuelLoaded)
			{
				staWeaponSlot[i].fuelLoaded=0.0;
			}
			return;
		}
	}


	staFuelLoad=staFuelLoad-dFuel*dt;
	if(staFuelLoad<YsTolerance)
	{
		staFuelLoad=0.0;
	}
}


void FsAirplaneProperty::CalculateRungeKutta4(const double &dt)
{
	YsVec3 initPos,initVel;
	YsAtt3 initAtt;
	double initVp,initVy,initVr;
	YsVec3 v1,v2,v3,v4,dv1,dv2,dv3,dv4;
	double vp1,vp2,vp3,vp4,vy1,vy2,vy3,vy4,vr1,vr2,vr3,vr4;
	double dvp1,dvp2,dvp3,dvp4,dvy1,dvy2,dvy3,dvy4,dvr1,dvr2,dvr3,dvr4;

	initPos=staPosition;
	initVel=staVelocity;
	initAtt=staAttitude;
	initVp=staVPitch;
	initVy=staVYaw;
	initVr=staVRoll;

	CalculateForce();
	CalculateRotationalAcceleration();
	v1=staVelocity;
	vp1=staVPitch;
	vy1=staVYaw;
	vr1=staVRoll;
	dv1=(staTotalAerodynamicForce+staTotalGravityForce)/GetTotalWeight();
	dvp1=staDVPitch;
	dvy1=staDVYaw;
	dvr1=staDVRoll;

	// Translational velocity and rotational velocity is updated to K1 state
	// Now, using this translational velocity and rotational velocity,
	// compute K2 state.
	staPosition=initPos+v1*dt/2.0;
	staVelocity=initVel+dv1*dt/2.0;
	staAttitude=initAtt;
	staAttitude.NoseUp(vp1*dt/2.0);
	staAttitude.YawLeft(vy1*dt/2.0);
	staAttitude.SetB(staAttitude.b()+vr1*dt/2.0);
	staVPitch=initVp+vp1*dt/2.0;
	staVYaw=initVy+vy1*dt/2.0;
	staVRoll=initVr+vr1*dt/2.0;
	RemakeMatrix();

	CalculateForce();
	CalculateRotationalAcceleration();
	v2=staVelocity;
	vp2=staVPitch;
	vy2=staVYaw;
	vr2=staVRoll;
	dv2=(staTotalAerodynamicForce+staTotalGravityForce)/GetTotalWeight();
	dvp2=staDVPitch;
	dvy2=staDVYaw;
	dvr2=staDVRoll;

	// Now got K2 state. Go to K3
	staPosition=initPos+v2*dt/2.0;
	staVelocity=initVel+dv2*dt/2.0;
	staAttitude=initAtt;
	staAttitude.NoseUp(vp2*dt/2.0);
	staAttitude.YawLeft(vy2*dt/2.0);
	staAttitude.SetB(staAttitude.b()+vr2*dt/2.0);
	staVPitch=initVp+vp2*dt/2.0;
	staVYaw=initVy+vy2*dt/2.0;
	staVRoll=initVr+vr2*dt/2.0;
	RemakeMatrix();

	CalculateForce();
	CalculateRotationalAcceleration();
	v3=staVelocity;
	vp3=staVPitch;
	vy3=staVYaw;
	vr3=staVRoll;
	dv3=(staTotalAerodynamicForce+staTotalGravityForce)/GetTotalWeight();
	dvp3=staDVPitch;
	dvy3=staDVYaw;
	dvr3=staDVRoll;

	// Now got K3 state. Go to K4
	staPosition=initPos+v3*dt;
	staVelocity=initVel+dv3*dt;
	staAttitude=initAtt;
	staAttitude.NoseUp(vp3*dt);
	staAttitude.YawLeft(vy3*dt);
	staAttitude.SetB(staAttitude.b()+vr3*dt);
	staVPitch=initVp+vp3*dt;
	staVYaw=initVy+vy3*dt;
	staVRoll=initVr+vr3*dt;
	RemakeMatrix();

	CalculateForce();
	CalculateRotationalAcceleration();
	v4=staVelocity;
	vp4=staVPitch;
	vy4=staVYaw;
	vr4=staVRoll;
	dv4=(staTotalAerodynamicForce+staTotalGravityForce)/GetTotalWeight();
	dvp4=staDVPitch;
	dvy4=staDVYaw;
	dvr4=staDVRoll;


	YsVec3 v,dv;
	double vp,vy,vr,dvp,dvy,dvr;

	v=(v1+2.0*v2+2.0*v3+v4)*dt/6.0;
	vp=(vp1+2.0*vp2+2.0*vp3+vp4)*dt/6.0;
	vy=(vy1+2.0*vy2+2.0*vy3+vy4)*dt/6.0;
	vr=(vr1+2.0*vr2+2.0*vr3+vr4)*dt/6.0;
	dv=(dv1+2.0*dv2+2.0*dv3+dv4)*dt/6.0;
	dvp=(dvp1+2.0*dvp2+2.0*dvp3+dvp4)*dt/6.0;
	dvy=(dvy1+2.0*dvy2+2.0*dvy3+dvy4)*dt/6.0;
	dvr=(dvr1+2.0*dvr2+2.0*dvr3+dvr4)*dt/6.0;


	// From origin
	staPosition=initPos;
	staAttitude=initAtt;
	staVPitch=initVp;
	staVYaw=initVy;
	staVRoll=initVr;

	staPosition+=v;
	staAttitude.NoseUp(vp);
	staAttitude.YawLeft(vy);
	staAttitude.SetB(staAttitude.b()+vr);

	staVelocity+=dv;
	staVPitch+=dvp;
	staVYaw+=dvy;
	staVRoll+=dvr;
}


void FsAirplaneProperty::CalculateGravitationalForce(void)
{
	staWeight=GetTotalWeight();
	staTotalGravityForce.Set(0.0,-staWeight*FsGravityConst,0.0);
}

void FsAirplaneProperty::AdjustGravitationalForceForSlope(void)
{
	if(staState==FSGROUND || staState==FSGROUNDSTATIC)
	{
		const YsVec3 totalForce(0.0,staTotalAerodynamicForce.y()+staTotalGravityForce.y(),0.0);
		if(staGndNormal*totalForce<0.0)
		{
			// See memo/forceOnSlope.docx
			YsVec3 Fslope=totalForce-(staGndNormal*totalForce)*staGndNormal;

			YsVec3 ZVec;
			staMatrix.Mul(ZVec,YsZVec(),0.0);

			Fslope=ZVec*(ZVec*Fslope);
			staTotalGravityForce+=Fslope;
		}
	}
}

void FsAirplaneProperty::CalculateForce(void)
{
	YsVec3 vBrake;
	double lift,thrust,drag;
	YsVec3 relVel;

	double vCorrected=staV;
	double aoaCorrected=staAOA;
	if(FSGROUNDSTATIC==staState)
	{
		vCorrected=0.0;
		aoaCorrected=0.0;

		staInverseMatrix.Mul(relVel,staVelocity,0.0);
	}
	else if(FSGROUNDSTATIC==staState && 0.0>staGndNormal*staVelocity)
	{
		YsVec3 vel=staVelocity-(staGndNormal*staVelocity)*staGndNormal;
		vCorrected=vel.GetLength();

		staInverseMatrix.Mul(relVel,vel,0.0);
		aoaCorrected=atan2(-relVel.y(),relVel.z());
	}
	lift=GetLift(aoaCorrected,vCorrected);
	drag=GetDrag(aoaCorrected,vCorrected);


	if(IsOnGround()!=YSTRUE)
	{
		vBrake=YsOrigin();
	}
	else
	{
		const double normalForce=-staGndNormal*(staTotalGravityForce+staTotalAerodynamicForce);
		const double tireFrictionForce=(0.0<normalForce ? normalForce*chTireFrictionConst : 0.0);
		const double brakingForce=(chClass==FSCL_AIRPLANE ? CalculateForceByBrake(staBrake) : CalculateForceByBrake(1.0));

		const double frictionForce=YsGreater(tireFrictionForce,brakingForce);

		vBrake.Set(staVelocity.x(),0.0,staVelocity.z());
		if(vBrake.Normalize()==YSOK)
		{
			vBrake*=-frictionForce;
		}
		else
		{
			vBrake=YsOrigin();
		}
	}

	YsVec3 vThrust;
	if(chHasThrustVector!=YSTRUE)
	{
		vThrust.Set(0.0,0.0,1.0);
	}
	else
	{
		vThrust=chThrVec0*(1.0-staThrVec)+chThrVec1*staThrVec;
		vThrust.Normalize();
	}


	if(YsTolerance<chThrustReverser && YsTolerance<staThrRev)
	{
		vThrust=-vThrust*chThrustReverser*staThrRev;
	}


	if(staFuelLoad>YsTolerance && IsActive()==YSTRUE && staArrested!=YSTRUE)
	{
		if(0<chRealProp.GetN())
		{
			YsVec3 thrustForTest=YsOrigin();
			double rpm=0.0;
			for(auto &realProp : chRealProp)
			{
				thrustForTest+=realProp.GetForce();
				rpm=realProp.radianPerSec*60.0/(YsPi*2.0);
			}
			thrust=thrustForTest.z();
			//printf("RealProp %.2lfN   RPM %.0lf\n",thrustForTest.GetLength(),rpm,thrust);
			//printf("RealProp Vec %s\n",thrustForTest.Txt());
		}
		else
		{
			double v;  // v must be velocity component of thrust direction.
			YsVec3 vThrTfm;
			staMatrix.Mul(vThrTfm,vThrust,0.0);
			v=staVelocity*vThrTfm;
			thrust=GetThrust(staThrottle,staPosition.y(),v,staAb);
		}
	}
	else
	{
		thrust=0.0;
		staAb=YSFALSE;
		staThrottle=0.0;
	}



	YsVec3 vDrag,vLift;
	vThrust*=thrust;
	vDrag.Set(0.0, drag*sin(staAOA),-drag*cos(staAOA));  // 2003/05/04
	vLift.Set(-lift*cos(staAOA)*sin(staSSA),
	           lift*cos(staAOA)*cos(staSSA),
	           lift*sin(staAOA));
	// staTotalForce=vDrag+vLift+vThrust;     // 2003/05/04
	staTotalAerodynamicForce=vLift+vThrust;              // 2003/05/04
	staMatrix.Mul(staTotalAerodynamicForce,staTotalAerodynamicForce,0.0);
	vDrag=-staVelocity;                 // 2003/05/04
	if(vDrag.Normalize()==YSOK)            // 2003/05/04
	{                                      // 2003/05/04
		staTotalAerodynamicForce+=vDrag*drag;        // 2003/05/04
	}                                      // 2003/05/04
	staTotalAerodynamicForce+=vBrake;
	staThrust=thrust;
}

void FsAirplaneProperty::CalculateTranslation(const double &dt)
{
	if(staVHorizontal<FsMinimumAirspeed &&
	   (staBrake>=0.9 || chClass==FSCL_HELICOPTER) &&
	   IsOnGround()==YSTRUE)
	{
		staVelocity=staVelocity+(staTotalAerodynamicForce+staTotalGravityForce)/GetTotalWeight()*dt;
		staVelocity.SetX(0.0);
		staVelocity.SetZ(0.0);
	}
	else
	{
		staVelocity=staVelocity+(staTotalAerodynamicForce+staTotalGravityForce)/GetTotalWeight()*dt;
	}
	staPosition=staPosition+staVelocity*dt;
// printf("V %s\n",staVelocity.Txt());
}

void FsAirplaneProperty::CalculateRotationalAcceleration(void)
{
	if(staState!=FSDEADSPIN && staState!=FSDEADFLATSPIN)
	{
		double inputAOA,inputSSA,inputROLL;
		inputAOA=GetInputAOA();
		inputSSA=GetInputSSA();
		inputROLL=GetInputROLL();

		if(IsOnGround()==YSTRUE)   // Implement Effect by the gear as 3deg smaller AOA
		// This offset must be taken into account in ControlG, ControlAOA, and ControlPitch
		{
			inputAOA-=(1.0+GetBrake())*YsDegToRad(2.0);  // stronger brake-> tends to bow.
		}

		// Don't bow!  (For front-wheel airplane)    2002/11/03
		if(chMainGearL.z()<chWheel.z() && chMainGearR.z()<chWheel.z())
		{
			if(IsOnGround()==YSTRUE && inputAOA<GetGroundStaticPitchAngle())
			{
				inputAOA=GetGroundStaticPitchAngle();
			}
		}

		double kPitch,bPitch,kYaw,kRoll;

		GetPitchManeuvabilityConst(kPitch,bPitch);
		kRoll=GetRollManeuvabilityConst();
		kYaw=GetYawManeuvabilityConst();


		if(IsOnGround()==YSTRUE && IsOutOfRunway()!=YSTRUE)
		{
			staVRoll=0.0;
		}

		if(chClass==FSCL_AIRPLANE)
		{
			staDVPitch=(inputAOA-staAOA)*kPitch-staVPitch*bPitch;
		}
		else // if(chClass==FSCL_HELICOPTER)
		{
			staDVPitch=0.0;
		}

		double inputSsaCorrection;                              // 2005/10/02
		inputSsaCorrection=inputSSA;                            // 2005/10/02
		if(IsOnGround()==YSTRUE)                                // 2005/10/02
		{                                                       // 2005/10/02
			YsVec3 tst;                                         // 2005/10/02
			tst=staVelocity;                                   // 2005/10/02
			tst.SetY(0.0);  // <- Y component may be non zero.     2005/10/02
			staInverseMatrix.Mul(tst,tst,0.0);                 // 2005/10/02

			if(tst.z()<-FsMinimumAirspeed)                      // 2005/10/02
			{                                                   // 2005/10/02
				inputSsaCorrection=YsPi-inputSsaCorrection;     // 2005/10/02
				if(staSSA+YsPi<inputSsaCorrection)             // 2005/10/02
				{                                               // 2005/10/02
					inputSsaCorrection-=YsPi*2.0;               // 2005/10/02
				}                                               // 2005/10/02
				else if(inputSsaCorrection<staSSA-YsPi)        // 2005/10/02
				{                                               // 2005/10/02
					inputSsaCorrection+=YsPi*2.0;               // 2005/10/02
				}                                               // 2005/10/02
			}                                                   // 2005/10/02
			else if(tst.z()<FsMinimumAirspeed)                  // 2005/10/02
			{                                                   // 2005/10/02
				inputSsaCorrection=staSSA;  // Forced zero input. 2005/10/02
			}                                                   // 2005/10/02
		}                                                       // 2005/10/02


		if(inputSsaCorrection<staSSA-YsPi/2.0)
		{
			inputSsaCorrection+=YsPi;
		}
		else if(staSSA+YsPi/2.0<inputSsaCorrection)
		{
			inputSsaCorrection-=YsPi;
		}


		staDVYaw=(inputSsaCorrection-staSSA)*kYaw-staVYaw*chYawStabConst;
		staDVRoll=(inputROLL-staVRoll)*kRoll;


		// Post-Stall Maneuver
		if(staState!=FSGROUNDSTATIC && GetThrottle()>chDirectAttitudeControlReqThr1)  // 2003/05/04
		{
			double effectiveness=1.0;
			if(staV<chDirectAttitudeControlSpeed1)
			{
			}
			else if(chDirectAttitudeControlSpeed2<staV)
			{
				effectiveness=0.0;
			}
			else
			{
				effectiveness=(chDirectAttitudeControlSpeed2-staV)/(chDirectAttitudeControlSpeed2-chDirectAttitudeControlSpeed1);
			}

			if(GetThrottle()<chDirectAttitudeControlReqThr1)
			{
				effectiveness=0.0;
			}
			else if(chDirectAttitudeControlReqThr2<GetThrottle())
			{
			}
			else
			{
				const double t=(GetThrottle()-chDirectAttitudeControlReqThr1)/(chDirectAttitudeControlReqThr2-chDirectAttitudeControlReqThr1);
				effectiveness*=t;
			}

			if(fabs(staVPitch)<chPostStallVPitch)
			{
				staDVPitch+=effectiveness*chPitchManConst*(chPostStallVPitch*ctlDirectPitch-staVPitch);
			}
			if(fabs(staVYaw)<chPostStallVYaw)
			{
				staDVYaw+=effectiveness*chYawManConst*(chPostStallVYaw*ctlDirectYaw-staVYaw);
			}
			if(fabs(staVRoll)<chPostStallVRoll)
			{
				staDVRoll+=effectiveness*chRollManConst*(chPostStallVRoll*ctlDirectRoll-staVRoll);
			}
		}
	}
	else if(staState==FSDEADSPIN)
	{
		staDVPitch=-YsPi/2.0-staAttitude.p()-staVPitch*0.5;
		staDVYaw/=2.0;
		staDVRoll=(YsPi/2.0-staVRoll);
	}
	else if(staState==FSDEADFLATSPIN)
	{
		staDVPitch=-YsPi/6.0-staAOA-staVPitch*0.5;
		staDVYaw=(YsPi/6.0-staSSA)-staVYaw*0.5;
		if(staVelocity.y()>-YsGreater(fabs(staVelocity.x()),fabs(staVelocity.z())))
		{
			staDVRoll=YsPi/36.0-staVRoll;
		}
		else
		{
			staDVRoll=YsPi-staVRoll;
		}
	}
}

extern void FsAltYawLeft(YsAtt3 &att,const double &d);
extern void FsAltNoseUp(YsAtt3 &att,const double &d);

void FsAirplaneProperty::CalculateRotation(const double &dt)
{
	staVPitch=staVPitch+staDVPitch*dt;
	staVYaw=staVYaw+staDVYaw*dt;
	staVRoll=staVRoll+staDVRoll*dt;

	staAttitude.NoseUp(staVPitch*dt);

	// 2008/11/17 Yawing of 1.0e-100 or smaller may cause FPU to crash.
	staAttitude.YawLeft(staVYaw*dt);
	staAttitude.SetB(staAttitude.b()+staVRoll*dt);
}

void FsAirplaneProperty::CalculateCarrierLanding(const double &dt,const YsArray <FsGround *> &carrierList)
{
	if(staOnThisCarrier==NULL)
	{
		YsVec3 nose,left,right;

		nose=staMatrix*chWheel;
		left=staMatrix*chMainGearL;
		right=staMatrix*chMainGearR;

		int i;
		for(i=0; i<carrierList.GetNumItem(); i++)
		{
			FsAircraftCarrierProperty *prop;
			prop=carrierList[i]->Prop().GetAircraftCarrierProperty();
			if(prop!=NULL && prop->LandedOnTheDeck(staPPos,staPosition,nose,left,right)==YSTRUE)
			{
				// staOnThisCarrier=carrierList[i];  This should be done from LoadAirplane
				prop->LoadAirplane(belongTo);
				break;
			}
		}
	}
	else
	{
		FsAircraftCarrierProperty *const prop=staOnThisCarrier->Prop().GetAircraftCarrierProperty();
		if(prop!=NULL)
		{
			YsVec3 deckNom;
			if(prop->IsOnDeck(staPosition)!=YSTRUE ||
			   prop->GetDeckHeightAndNormal(deckNom,staPosition)+GetGroundStandingHeight()+1.0<staPosition.y())  // 2003/04/08 Avoid catapult in the air
			{
				prop->UnloadAirplane(belongTo);  // <- This will call back AfterUnloadedFromCarrier, which cleans up stuffs.
			}
		}
		else
		{
			// Something was wrong.
			staOnThisCarrier=NULL;
			staArrested=YSFALSE;
			staCatapulted=YSFALSE;
		}
	}

	if(staOnThisCarrier!=NULL)
	{
		FsAircraftCarrierProperty *prop;
		prop=staOnThisCarrier->Prop().GetAircraftCarrierProperty();

		if(GetBrake()>=0.95 && prop->IsOnArrester(staPosition)==YSTRUE)
		{
			staArrested=YSTRUE;
		}
		else if(GetBrake()<0.95)
		{
			staArrested=YSFALSE;
		}

		if(GetThrottle()>=0.95 && prop->IsOnCatapult(staPosition)==YSTRUE)
		{
			staCatapulted=YSTRUE;
		}
		else if(GetThrottle()<=0.95)
		{
			staCatapulted=YSFALSE;
		}
	}

	if(staArrested==YSTRUE && staOnThisCarrier!=NULL)
	{
		// Move 300ft to make 120kt->0kt
		//   120kt=1800m*120nm/3600sec=60m/s
		//   300ft=100m
		//     Implication Reduce 0.6m/s as it moves 1m

		double arresterConstant;
		double translation,brake,newV,oldV;

		if(staOnThisCarrier->Prop().GetVelocity()<YsTolerance)
		{
			arresterConstant=1.00;
		}
		else
		{
			arresterConstant=0.50;
		}

		oldV=staV;

		translation=staV*dt;
		brake=arresterConstant*translation;
		newV=staV-brake;
		if(newV<0.0)
		{
			newV=0.0;
		}

		staV=newV;
		if(newV<=YsTolerance)
		{
			staVelocity.Set(0.0,0.0,0.0);
		}
		else if(oldV>YsTolerance)
		{
			staVelocity=staVelocity*newV/oldV;
		}
	}
	if(staCatapulted==YSTRUE && staOnThisCarrier!=NULL)
	{
		double catapultConstant;
		double translation,accel;
		YsVec3 accelVec;

		if(staOnThisCarrier->Prop().GetVelocity()<YsTolerance)
		{
			catapultConstant=0.28;
		}
		else
		{
			catapultConstant=0.12;
		}

		translation=staV*dt;
		accel=catapultConstant*translation;

		accelVec.Set(0.0,0.0,accel);
		staOnThisCarrier->GetAttitude().Mul(accelVec,accelVec);

		staVelocity+=accelVec;
		staV=staVelocity.GetLength();
	}

}

// Field elevation must be set before calling this function
void FsAirplaneProperty::CalculateGround(const double &dt)
{
	double gDist;
	if(IsOnGround()==YSTRUE)
	{
		// Cheat:To Stabilize airplane on the ground
		double dy,da;
		da=(chManSpeed1-staV)/chManSpeed1;
		if(da>0.0)
		{
			dy=YsAbs(chMainGearR.x()-chMainGearL.x())*(YsPi/40.0)*da*dt;
			staPosition.SetY(staPosition.y()-dy);
			RemakeMatrix();
		}

		const double gndSpeedThr=8.0;
		if(IsOutOfRunway()==YSTRUE && IsActive()==YSTRUE)
		{
			if(GetVelocity()>gndSpeedThr)
			{
				// Change in 0.2 sec is 100 %
				// Change in dt sec is 100*(dt/0.2) %
				double rnd;
				rnd=(double)(rand()%10000)/10000.0;
				if(rnd<dt/0.2)
				{
					double v;
					v=GetVelocity()-gndSpeedThr;
					if(staAttitude.p()>0.0)
					{
						staVPitch=-YsDegToRad(v*4.0);
					}
					else
					{
						staVPitch=YsDegToRad(v*4.0);
					}
					if(staAttitude.b()>0.0)
					{
						staVRoll=-YsDegToRad(v*5.0);
					}
					else
					{
						staVRoll=YsDegToRad(v*5.0);
					}
				}
			}
			else
			{
				staVRoll=0.0;
			}
		}
	}

	if(CheckTouchDownAndLayOnGround(gDist)==YSTRUE)
	{
		if(IsOnGround()!=YSTRUE)
		{
			staTDPos=staPPos;
			staTDAtt=staPAtt;
			staTDVelocity=staPVelocity;

			// Never alive again by touching down.
			if(staState!=FSDEAD &&
			   staState!=FSDEADSPIN &&
			   staState!=FSDEADFLATSPIN)
			{
				SetState(FSGROUND,FSDIEDOF_NULL);
				staNeedTouchdownCheck=YSTRUE;  // <- This is referred in FsAirplane::HitGround()
			}
			else
			{
				SetState(FSDEAD,FSDIEDOF_NULL);
			}
		}

		if(staVelocity.y()<0.0)
		{
			if(chClass==FSCL_AIRPLANE)
			{
				if(staV<GetEstimatedLandingSpeed()*1.25)
				{
					staVelocity.SetY(0.0);
				}
				else
				{
					staVelocity.MulY(-0.6);
				}
			}
			else // if(chClass==FSCL_HELICOPTER)
			{
				staVelocity.SetY(0.0);
			}
		}
	}

	if(IsOnGround()==YSTRUE)
	{
		// YsVec3 tst;                                         // 2005/10/02
		// tst=staVelocity;                                   // 2005/10/02
		// tst.SetY(0.0);  // <- Y component may be non zero.     2005/10/02
		// staInverseMatrix.Mul(tst,tst,0.0);                 // 2005/10/02

		staInverseMatrix.Mul(staVelocity,staVelocity,0.0); // <- Before 2005/10/02

		if(staVelocity.z()>=YsTolerance)  // 2003/04/06 to stop inversion when a VTOL lands while moving backward
		{
			double vHorizontal;
			vHorizontal=sqrt(staVelocity.x()*staVelocity.x()
			                +staVelocity.z()*staVelocity.z());

			staVelocity.Set(0.0,staVelocity.y(),vHorizontal);
			staMatrix.Mul(staVelocity,staVelocity,0.0);
		}
		else if(staVelocity.z()<-YsTolerance)
		{
			double vHorizontal;
			vHorizontal=sqrt(staVelocity.x()*staVelocity.x()      // 2005/10/02
			                +staVelocity.z()*staVelocity.z());

			staVelocity.Set(0.0,staVelocity.y(),-vHorizontal);
			staMatrix.Mul(staVelocity,staVelocity,0.0);

			// staVelocity.Set(0.0,staVelocity.y(),0.0); <- Before 2005/10/02
		}
	}

	if(gDist>1.0 && staState!=FSDEADSPIN && staState!=FSDEADFLATSPIN)
	{
		SetState(FSFLYING,FSDIEDOF_NULL);
	}

	if(staState==FSGROUND && staV<=FsMinimumAirspeed)
	{
		SetState(FSGROUNDSTATIC,FSDIEDOF_NULL);
	}
	else if(staState==FSGROUNDSTATIC && staV>FsMinimumAirspeed)
	{
		SetState(FSGROUND,FSDIEDOF_NULL);
	}
}

void FsAirplaneProperty::CalculateStall(void)
{
	if(chClass!=FSCL_HELICOPTER)
	{
		FSFLIGHTSTATE state;
		state=staState;
		if(state==FSFLYING && (staAOA<chMinAOA || chMaxAOA<staAOA))
		{
			if(staV>chManSpeed1 || chHasThrustVector!=YSTRUE)
			{
				SetState(FSSTALL,FSDIEDOF_NULL);
			}
			else
			{
				YsVec3 thrVec;
				GetThrustDirection(thrVec);
				if(thrVec.y()<YsAbs(thrVec.x()) || thrVec.y()<YsAbs(thrVec.z()))
				{
					SetState(FSSTALL,FSDIEDOF_NULL);
				}
			}
		}
	}
}

void FsAirplaneProperty::CalculateWeather(const double &dt,const FsWeather &weather)
{
	YsVec3 windDrift;
	CalculateWindDrift(windDrift,weather);
	staPosition+=windDrift*dt;
}

void FsAirplaneProperty::CalculateWindDrift(YsVec3 &driftVec,const FsWeather &weather) const
{
	if(staState==FSFLYING || FSSTALL==staState)
	{
		driftVec=weather.GetWind();
	}
	else if(IsOnGround()==YSTRUE)
	{
		double t;

		t=YsSmaller(1.0,staV/chManSpeed2);
		t=t*t*t*t*t;
		driftVec=weather.GetWind()*t;
	}
	else
	{
		driftVec=YsOrigin();
	}
}

double FsAirplaneProperty::GetLift(const double &aoa,const double &vel) const
{
	if(IsActive()==YSTRUE)
	{
		double cl,lift;

		if(chMinAOA<=aoa && aoa<=chMaxAOA)
		{
			cl=(chClZero+aoa*chClSlope)*(1.0+chClFlap*staFlap);
		}
		else if(chMaxAOA<=aoa && aoa<chMaxAOA+chFlatClRange1)
		{
			cl=(chClZero+chMaxAOA*chClSlope)*(1.0+chClFlap*staFlap);
		}
		else if(chMinAOA-chFlatClRange2<aoa && aoa<=chMinAOA)
		{
			cl=(chClZero+chMinAOA*chClSlope)*(1.0+chClFlap*staFlap);
		}
		else if(chMaxAOA+chFlatClRange1<=aoa && aoa<chMaxAOA+chFlatClRange1+chClDecay1)
		{
			const double t=1.0-(aoa-(chMaxAOA+chFlatClRange1))/chClDecay1;
			const double maxCl=(chClZero+chMaxAOA*chClSlope)*(1.0+chClFlap*staFlap);
			cl=maxCl*t;
		}
		else if(chMinAOA-chFlatClRange2-chClDecay2<aoa && aoa<=chMinAOA-chFlatClRange2)
		{
			const double t=1.0-((chMinAOA-chFlatClRange2)-aoa)/chClDecay2;
			const double minCl=(chClZero+chMinAOA*chClSlope)*(1.0+chClFlap*staFlap);
			cl=minCl*t;
		}
		else
		{
			cl=0.0;
		}
		if(chHasVGW==YSTRUE)
		{
			cl=cl*(1.0+chClVgw*staVgw);
		}
		lift=0.5*cl*staRho*vel*vel*chWingArea;

		return lift;
	}
	else
	{
		return 0.0;
	}
}

double FsAirplaneProperty::GetDrag(const double &aoa,const double &vel) const
{
	double nomAOA,cd,drag;

	if(aoa>YsPi/2.0)
	{
		nomAOA=YsPi-aoa;
	}
	else if(aoa<-YsPi/2.0)
	{
		nomAOA=-YsPi-aoa;
	}
	else
	{
		nomAOA=aoa;
	}

	nomAOA=YsBound(nomAOA,-chMaxCdAOA,chMaxCdAOA); // Kind of cheat (1)

	if(staState==FSDEADFLATSPIN)
	{
		nomAOA=0.0;   // Kind of cheat (2)
	}

	if(vel<chVCritical && chCdZeroMax>chCdZero)
	{
		cd=chCdZero+chCdConst*nomAOA*nomAOA;
	}
	else
	{
		cd=chCdZero
		  +chCdConst*nomAOA*nomAOA;

		if(chVMax-chVCritical>YsTolerance)
		{
			cd+=(chCdZeroMax-chCdZero)*(vel-chVCritical)/(chVMax-chVCritical);  // 20040821
		}

		if(cd<0.0)
		{
			// Protection from blowing up
			cd=chCdZeroMax;
		}
	}
	if(chHasSpoiler==YSTRUE)
	{
		cd=cd*(1.0+chCdSpoiler*staSpoiler);
	}
	if(chHasVGW==YSTRUE)
	{
		cd=cd*(1.0+chCdVgw*staVgw);
	}
	cd=cd*(1.0+chCdFlap*staFlap);
	cd=cd*(1.0+chCdGear*staGear);
	drag=0.5*cd*staRho*vel*vel*chWingArea;

	return drag;
}

// Thrust is a function of altitude,velocity and throttle setting (+after burner)
double FsAirplaneProperty::GetThrust(const double &thr,const double &alt,const double &vel,YSBOOL ab) const
{
	double thrust;
	if(chIsJet==YSTRUE)
	{
		if(chHasAb==YSTRUE && ab==YSTRUE)
		{
			thrust=chThrMil+(chThrAb-chThrMil)*thr;
		}
		else
		{
			thrust=chThrMil*thr;
		}
		thrust=thrust*FsGetJetEngineEfficiency(alt);
	}
	else
	{
		thrust=CalculatePropellerThrust(thr,alt,vel);
	}
	return thrust;
}

double FsAirplaneProperty::GetConvergentThrust(const double &thr,const double &alt,const double &vel,YSBOOL ab)
{
	if(0<chRealProp.GetN())
	{
		const double rho=FsGetAirDensity(alt);
		double radianPerSec;
		double thrust=0.0;
		for(auto &prop : chRealProp)
		{
			thrust+=prop.GetConvergentThrust(radianPerSec,thr,rho,YsVec3(0.0,0.0,vel),100,0.05);
		}
		return thrust;
	}
	else
	{
		return GetThrust(thr,alt,vel,ab);
	}
}

void FsAirplaneProperty::ApplyControl(const FsFlightControl &ctl,unsigned int whatToApply)
{
	if(IsActive()==YSTRUE)
	{
		if((whatToApply&FSAPPLYCONTROL_STICK)!=0)
		{
			SetElevator(ctl.ctlElevator*ctl.ctlSensitivity);
			SetElvTrim(ctl.ctlElvTrim);
			SetAileron(ctl.ctlAileron*ctl.ctlSensitivity);
			SetRudder(ctl.ctlRudder);

			// Synchronize stick input with direct attitude control 2014/06/24
			SetDirectPitchControl(ctl.ctlElevator*ctl.ctlSensitivity);
			SetDirectRollControl(ctl.ctlAileron*ctl.ctlSensitivity);
			SetDirectYawControl(ctl.ctlRudder);
		}

		if((whatToApply&FSAPPLYCONTROL_NAVAID)!=0)
		{
			SetVectorMarker(ctl.ctlVectorMarker);
		}

		if((whatToApply&FSAPPLYCONTROL_GEAR)!=0)
		{
			SetGear(ctl.ctlGear);
		}

		if((whatToApply&FSAPPLYCONTROL_BRAKE)!=0)
		{
			SetBrake(ctl.ctlBrake);
		}

		if((whatToApply&FSAPPLYCONTROL_SPOILER)!=0)
		{
			SetSpoiler(ctl.ctlSpoiler);
		}

		if((whatToApply&FSAPPLYCONTROL_THROTTLE)!=0)
		{
			SetThrottle(ctl.ctlThrottle);
			SetAfterburner(ctl.ctlAb);
		}

		if(0!=(whatToApply&FSAPPLYCONTROL_PROPELLER))
		{
			SetPropellerAll(ctl.ctlPropeller);
		}

		if((whatToApply&FSAPPLYCONTROL_THRREV)!=0)
		{
			SetThrustReverser(ctl.ctlThrRev);
			if(GetThrustReverser()>YsTolerance && chThrustReverser>YsTolerance)
			{
				SetThrottle(GetThrustReverser());  // Synchronize throttle with thrust reverser
				SetAfterburner(YSFALSE);
			}
		}

		if((whatToApply&FSAPPLYCONTROL_FLAP)!=0)
		{
			SetFlap(ctl.ctlFlap);
		}

		if((whatToApply&FSAPPLYCONTROL_VGW)!=0)
		{
			if(ctl.ctlAutoVgw==YSFALSE)
			{
				SetVgw(ctl.ctlVgw);
				SetAutoVgw(ctl.ctlAutoVgw);
			}
			else
			{
				SetAutoVgw(ctl.ctlAutoVgw);
			}
		}

		if((whatToApply&FSAPPLYCONTROL_TRIGGER)!=0)
		{
			SetFireWeaponButton(ctl.ctlFireWeaponButton);
			SetFireGunButton(ctl.ctlFireGunButton);
			SetFireAAMButton(ctl.ctlFireAAMButton);
			SetFireAGMButton(ctl.ctlFireAGMButton);
			SetFireRocketButton(ctl.ctlFireRocketButton);
			SetDropBombButton(ctl.ctlDropBombButton);
			SetDispenseFlareButton(ctl.ctlDispenseFlareButton);
			SetCycleWeaponButton(ctl.ctlCycleWeaponButton);
			SetSmokeButton(ctl.ctlSmokeButton);
			SetCycleSmokeSelectorButton(ctl.ctlCycleSmokeSelectorButton);

			int i;
			YSBOOL fireGunButton;
			if((staWoc==FSWEAPON_GUN && ctlFireWeaponButton==YSTRUE) || ctlFireGunButton==YSTRUE)
			{
				fireGunButton=YSTRUE;
			}
			else
			{
				fireGunButton=YSFALSE;
			}

			for(i=0; i<chTurret.GetN(); i++)
			{
				if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
				{
					if(fireGunButton==YSTRUE)
					{
						staTurret[i].turretState|=FSTURRETSTATE_FIRING;
					}
					else
					{
						staTurret[i].turretState&=~FSTURRETSTATE_FIRING;
					}
				}
			}
		}

		if((whatToApply&FSAPPLYCONTROL_THRVEC)!=0)
		{
			SetThrustVector(ctl.ctlThrVec);
		}

		if((whatToApply&FSAPPLYCONTROL_TURRET)!=0)
		{
			SetPilotControlledTurretHeading(YsPi*ctl.ctlTurretHdg);
			SetPilotControlledTurretPitch(YsPi*0.5*ctl.ctlTurretPch);
		}

		if((whatToApply&FSAPPLYCONTROL_BOMBBAYDOOR)!=0)
		{
			SetBombBayDoor(ctl.ctlBombBayDoor==YSTRUE ? 1.0 : 0.0);
		}
	}
}

void FsAirplaneProperty::ReadBackControl(FsFlightControl &ctl) const
{
	ctl.ctlGear=ctlGear;
	ctl.ctlBrake=ctlBrake;
	//ctl.ctlSpoiler=ctlSpoiler;
	if(ctlBrake==YSTRUE)
	{
		ctl.ctlSpoiler=ctlSpoiler;
	}
	else
	{
		ctl.ctlSpoiler=0.0;
	}
	ctl.ctlAb=ctlAb;
	ctl.ctlThrottle=ctlThrottle;
	ctl.ctlFlap=ctlFlap;
	ctl.ctlElevator=ctlElevator;
	ctl.ctlElvTrim=ctlElvTrim;
	ctl.ctlAileron=ctlAileron;
	ctl.ctlRudder=GetRudderUserInput(); // 2014/07/03 Changed from GetRudder() to GetRudderUserInput()
	                                    //            Reason: If I use GetRudder(), which is the total of
	                                    //                    user input plus SmartRudder, SmartRudder and
	                                    //                    user control resonate and go berserk.
	ctl.ctlVgw=ctlVgw;
	ctl.ctlThrVec=ctlThrVec;
	// ctl.ctlILS=YSFALSE;    // 2006/04/25
}

void FsAirplaneProperty::CaptureState(YsArray <YsString> &stateStringArray) const
{
	// To be implemented
}

unsigned FsAirplaneProperty::NetworkEncode(unsigned char dat[],int idOnSvr,const double &currentTime,YSBOOL shortFormat) const
{
	short h,p,b,g,flag;
	int vp,vy,vr;
	int gun,aam,agm,bom,rocket;

	h=(short)(staAttitude.h()*32768.0/YsPi);
	p=(short)(staAttitude.p()*32768.0/YsPi);
	b=(short)(staAttitude.b()*32768.0/YsPi);
	vp=(int)(staVPitch*32768.0/YsPi);
	vy=(int)(staVYaw*32768.0/YsPi);
	vr=(int)(staVRoll*32768.0/YsPi);

	g=(short)(GetG()*100.0);

	gun=GetNumWeapon(FSWEAPON_GUN);
	aam=GetNumWeapon(FSWEAPON_AIM9);
	agm=GetNumWeapon(FSWEAPON_AGM65);
	bom=GetNumWeapon(FSWEAPON_BOMB);
	rocket=GetNumWeapon(FSWEAPON_ROCKET);

	flag=0;
	if(GetAfterBurner()==YSTRUE)
	{
		flag|=1;
	}
	if(IsTrailingVapor()==YSTRUE)
	{
		flag|=4;
	}
	if(IsFiringGun()==YSTRUE)
	{
		flag|=8;
	}
	if(YSTRUE==staBeacon)
	{
		flag|=16;
	}
	if(YSTRUE==staNavLight)
	{
		flag|=32;
	}
	if(YSTRUE==staStrobe)
	{
		flag|=64;
	}
	if(YSTRUE==staLandingLight)
	{
		flag|=128;
	}

	for(int smkIdx=0; smkIdx<MaxNumSmokeGenerator; ++smkIdx)
	{
		if(IsTrailingSmoke(smkIdx)==YSTRUE)
		{
			flag|=2; // For older client.
			flag|=(1<<(8+smkIdx));
		}
	}


	unsigned char *ptr;
	ptr=dat;
	FsPushInt  (ptr,FSNETCMD_AIRPLANESTATE);
	FsPushFloat(ptr,(float)currentTime);
	FsPushInt  (ptr,idOnSvr);


	// Version
	//   0:vh,vp,vr are 16 bit short
	// >=1:vh,vp,vr are 32 bit integer, and they are appended at the end.
	//     16 bit short's are left for the backward compatibility. (Fault design costs...)
	// >=2:thrVector, thrReverser, bombBay
	// >=3:idOnSvr is 32bit integer
	//   4:Short Format (after 20060805)
	//   5:Short Format (after 20060805)

	// Remark: Long format should be preserved for replying to QueryAirState from the client.
	//         Short format rounds off the ammos to 255 (cannot send accurate ammo of guns typically)

	if(shortFormat==YSTRUE)
	{
		g=(short)(GetG()*10.0);

		if(aam<256 && agm<256 && bom<256 && rocket<256 &&
		   -32768<vp && vp<32768 &&
		   -32768<vy && vy<32768 &&
		   -128<g && g<128 &&
		   GetDamageTolerance()<256)
		{
			int version,thrVec,thrRev,bomDor;
			thrVec=int(GetThrustVector()*255.0);
			thrRev=int(GetThrustReverser()*255.0);
			bomDor=int(GetBombBayDoor()*255.0);

			if(thrVec!=0 || bomDor!=0 || thrRev!=0)
			{
				version=4;  // Short format with thrVec, thrRev, bomDor
			}
			else
			{
				version=5;  // Short format without thrVec, thrRev, bomDor
			}

			FsPushShort(ptr,(short)version);   // Version
			FsPushFloat(ptr,(float)staPosition.x());
			FsPushFloat(ptr,(float)staPosition.y());
			FsPushFloat(ptr,(float)staPosition.z());
			FsPushShort(ptr,h);
			FsPushShort(ptr,p);
			FsPushShort(ptr,b);
			FsPushShort(ptr,(short)(staVelocity.x()*10.0));
			FsPushShort(ptr,(short)(staVelocity.y()*10.0));
			FsPushShort(ptr,(short)(staVelocity.z()*10.0));
			FsPushShort(ptr,(short)vp);
			FsPushShort(ptr,(short)vy);
			FsPushShort(ptr,(short)YsBound(vr,-32767,32767));

			FsPushShort(ptr,(short)GetSmokeOil());
			FsPushInt(ptr,(int)GetFuelLeft());  // 2007/01/07 Short -> Int Related modification in FsNetReceivedAirplaneState::Decode
			FsPushShort(ptr,(short)staPayload);

			FsPushUnsignedChar(ptr,(unsigned char)GetFlightState());
			FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetVariableGeometryWingState()*255.0),0,255));

			unsigned int c;
			c=YsBound(int(GetSpoiler()*15.0),0,15)*16+YsBound(int(GetLandingGear()*15.0),0,15);
			FsPushUnsignedChar(ptr,(unsigned char)c);
			c=YsBound(int(GetFlap()*15.0),0,15)*16+YsBound(int(GetBrake()*15.0),0,15);
			FsPushUnsignedChar(ptr,(unsigned char)c);

			FsPushShort(ptr,flag);

			FsPushShort(ptr,(short)YsSmaller(gun,32767));
			FsPushShort(ptr,(short)YsSmaller(rocket,32767));

			FsPushUnsignedChar(ptr,(unsigned char)YsSmaller(aam,255));
			FsPushUnsignedChar(ptr,(unsigned char)YsSmaller(agm,255));
			FsPushUnsignedChar(ptr,(unsigned char)YsSmaller(bom,255));
			FsPushUnsignedChar(ptr,(unsigned char)YsSmaller(GetDamageTolerance(),255));

			FsPushChar(ptr,(char)g);

			FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetThrottle()*99.0),0,99));
			FsPushChar(ptr,(char)YsBound(int(GetElevator()*99.0),-99,99));
			FsPushChar(ptr,(char)YsBound(int(GetAileron()*99.0),-99,99));
			FsPushChar(ptr,(char)YsBound(int(GetRudder()*99.0),-99,99));
			FsPushChar(ptr,(char)YsBound(int(GetElvTrim()*99.0),-99,99));

			if(version==4)
			{
				c=YsBound(int(GetThrustVector()*15.0),0,15)*16+YsBound(int(GetThrustReverser()*15.0),0,15);
				FsPushUnsignedChar(ptr,(unsigned char)c);

				c=YsBound(int(GetBombBayDoor()*15.0),0,15)*16;
				FsPushUnsignedChar(ptr,(unsigned char)c);
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
		FsPushShort(ptr,3);   // Version
		FsPushShort(ptr,0);
		FsPushFloat(ptr,(float)staPosition.x());
		FsPushFloat(ptr,(float)staPosition.y());
		FsPushFloat(ptr,(float)staPosition.z());
		FsPushShort(ptr,h);
		FsPushShort(ptr,p);
		FsPushShort(ptr,b);
		FsPushShort(ptr,(short)(staVelocity.x()*10.0));
		FsPushShort(ptr,(short)(staVelocity.y()*10.0));
		FsPushShort(ptr,(short)(staVelocity.z()*10.0));
		FsPushShort(ptr,(short)(vp));
		FsPushShort(ptr,(short)(vy));
		FsPushShort(ptr,(short)(vr));
		FsPushShort(ptr,g);
		FsPushShort(ptr,(short)gun);
		FsPushShort(ptr,(short)aam);
		FsPushShort(ptr,(short)agm);
		FsPushShort(ptr,(short)bom);
		FsPushShort(ptr,(short)GetSmokeOil());
		FsPushFloat(ptr,(float)GetFuelLeft());
		FsPushFloat(ptr,(float)staPayload);
		FsPushShort(ptr,(short)GetDamageTolerance());
		FsPushUnsignedChar(ptr,(unsigned char)GetFlightState());
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetVariableGeometryWingState()*255.0),0,255));
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetSpoiler()*255.0),0,255));
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetLandingGear()*255.0),0,255));
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetFlap()*255.0),0,255));
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetBrake()*255.0),0,255));
		FsPushShort(ptr,flag);
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetThrottle()*99.0),0,99));
		FsPushChar(ptr,(char)YsBound(int(GetElevator()*99.0),-99,99));
		FsPushChar(ptr,(char)YsBound(int(GetAileron()*99.0),-99,99));
		FsPushChar(ptr,(char)YsBound(int(GetRudder()*99.0),-99,99));
		FsPushChar(ptr,(char)YsBound(int(GetElvTrim()*99.0),-99,99));

		// Added 2001/06/24
		FsPushShort(ptr,(short)rocket);

		// Added 2002/10/25
		FsPushFloat(ptr,(float)(staVPitch));
		FsPushFloat(ptr,(float)(staVYaw));
		FsPushFloat(ptr,(float)(staVRoll));

		// Added 2003/04/11
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetThrustVector()*255.0),0,255));
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetThrustReverser()*255.0),0,255));
		FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetBombBayDoor()*255.0),0,255));
	}

	return (unsigned int)(ptr-dat+1);
}

void FsAirplaneProperty::NetworkDecode(FsNetReceivedAirplaneState &prevState,FsNetReceivedAirplaneState &recvState)
{
	double x,y,z,h,p,b,g,vx,vy,vz,vp,vh,vr;
	double T0Remote,T0Local,TRemote,TLocal,frRemote,frLocal;
	YsVec3 guess;



	// FsNetReceivedAirplaneState recvState;
	// recvState.Decode(dat,TLocal);

	T0Remote=prevState.tRemote;
	T0Local=prevState.tLocal;

	TRemote=recvState.tRemote;
	TLocal=recvState.tLocal;


	frLocal=TLocal-T0Local;  // frw Passed time on the local PC
	frRemote=TRemote-T0Remote;  // frt Passed time on the remote PC

//	printf("\n");
//	printf("Tlocal %-08.2lf  Tremote %-08.2lf  dTLocal:%-08.2lf  dTremote:%-08.2lf\n",TLocal,TRemote,frLocal,frRemote);



	if(T0Remote<TRemote)
	{
		staGunBullet=recvState.gun;
		if(GetNumWeapon(FSWEAPON_AIM9)>(int)recvState.aam)
		{
			SetNumWeapon(FSWEAPON_AIM9,recvState.aam);
		}
		if(GetNumWeapon(FSWEAPON_AGM65)>(int)recvState.agm)
		{
			SetNumWeapon(FSWEAPON_AGM65,recvState.agm);
		}
		if(GetNumWeapon(FSWEAPON_BOMB)>(int)recvState.bomb)
		{
			SetNumWeapon(FSWEAPON_BOMB,recvState.bomb);
		}
		staSmokeOil=recvState.smokeOil;
		if(GetNumWeapon(FSWEAPON_ROCKET)>(int)recvState.rocket)
		{
			SetNumWeapon(FSWEAPON_ROCKET,recvState.rocket);
		}
		staFuelLoad=recvState.fuel;
		staPayload= recvState.payload;
		staDamageTolerance=recvState.life;

		SetState(recvState.state,FSDIEDOF_NULL);
		staVgw=recvState.vgw;
		ctlVgw=recvState.vgw;
		staSpoiler=recvState.spoiler;
		ctlSpoiler=recvState.spoiler;
		staGear=recvState.ldg;
		ctlGear=recvState.ldg;
		staFlap=recvState.flap;
		ctlFlap=recvState.flap;
		staBrake=recvState.brake;
		ctlBrake=recvState.brake;

		if(recvState.ab==YSTRUE)
		{
			staAb=YSTRUE;
			ctlAb=YSTRUE;
		}
		else
		{
			staAb=YSFALSE;
			ctlAb=YSFALSE;
		}

		SetFireGunButton(recvState.firingGun);
		if(0!=recvState.trailingSmokeFlag)
		{
			SetSmokeSelector(recvState.trailingSmokeFlag);
			SetSmokeButton(YSTRUE);
		}
		else
		{
			SetSmokeButton(YSFALSE);
		}

		staBeacon=recvState.beacon;
		staNavLight=recvState.navLight;
		staStrobe=recvState.strobe;
		staLandingLight=recvState.landingLight;

		staThrottle=recvState.throttle;
		ctlThrottle=staThrottle;
		ctlElevator=recvState.elevator;
		ctlAileron=recvState.aileron;
		ctlRudderUser=recvState.rudder;
		ctlRudderControl=0.0;
		ctlElvTrim=recvState.elvTrim;

		staThrVec=recvState.thrVector;
		ctlThrVec=recvState.thrVector;
		staThrRev=recvState.thrReverser;
		ctlThrRev=recvState.thrReverser;
		staBombBayDoor=recvState.bombBay;
		ctlBombBayDoor=recvState.bombBay;
	}



	guess=staPosition;


	const double timeOut=2.0;
	double fraction;
	fraction=frLocal/frRemote;

	if(T0Remote<0.0 ||
	   timeOut<frRemote ||
	   (frRemote>YsTolerance && frLocal>YsTolerance && 0.2<=fraction && fraction<2.0))
	//  T0Remote<0.0    This packet is the first packet.
	//  frRemote>0.0  This packet is at least newer than the last one.
	//  frLocal>0.0  This packet is received after the last one.

	// Memo: 2004/07/26 It was using TRemote<0.0 to find the first state packet, but
	//                  it is more appropriate to use T0Remote<0.0.
	//                  T0Remote is prevState.tRemote, which is -1.0 until it receives the first state packet.
	//                  If it is less than 0.0, it means that the packet is the first state packet.

	{
		x=recvState.x;
		y=recvState.y;
		z=recvState.z;
		staPosition.Set(x,y,z);

		h=recvState.h;
		p=recvState.p;
		b=recvState.b;
		staAttitude.Set(h,p,b);

		vx=recvState.vx;
		vy=recvState.vy;
		vz=recvState.vz;
		staVelocity.Set(vx,vy,vz);

		vp=recvState.vp;
		vh=recvState.vh;
		vr=recvState.vb;
		staVPitch=vp;
		staVYaw=vh;
		staVRoll=vr;

		g=recvState.g;

		// Make correction for position and velocity with time delay taken into account
		if(T0Remote>YsTolerance &&  // <- TRemote -> T0Remote 2004/07/26
		   frRemote>YsTolerance && frLocal>YsTolerance && 0.2<=fraction && fraction<2.0 && frLocal<=timeOut)
		{
			x=double(prevState.x)*(1.0-fraction)+x*fraction;
			y=double(prevState.y)*(1.0-fraction)+y*fraction;
			z=double(prevState.z)*(1.0-fraction)+z*fraction;

			YsAtt3 prevA,nextA,correctA;
			YsVec3 prevF,prevU,nextF,nextU,correctF,correctU;
			prevA.Set(prevState.h,prevState.p,prevState.b);
			nextA.Set(h,p,b);

			prevF=prevA.GetForwardVector();
			prevU=prevA.GetUpVector();
			nextF=nextA.GetForwardVector();
			nextU=nextA.GetUpVector();

			correctF=prevF*(1.0-fraction)+nextF*fraction;
			correctU=prevU*(1.0-fraction)+nextU*fraction;
			correctA.SetTwoVector(correctF,correctU);

			staPosition.Set(x,y,z);
			staAttitude=correctA;

			vx=double(prevState.vx)*(1.0-fraction)+vx*fraction;
			vy=double(prevState.vy)*(1.0-fraction)+vy*fraction;
			vz=double(prevState.vz)*(1.0-fraction)+vz*fraction;
			staVelocity.Set(vx,vy,vz);

			vh=double(prevState.vh)*(1.0-fraction)+vh*fraction;
			vp=double(prevState.vp)*(1.0-fraction)+vp*fraction;
			vr=double(prevState.vb)*(1.0-fraction)+vr*fraction;
			staVPitch=vp;
			staVYaw=vh;
			staVRoll=vr;



			// Assume the program received the packet not at clock=w, but at clock=w0+frt*0.9+frw*0.1
			T0Remote=TRemote;
			T0Local=T0Local+frRemote*0.9+frLocal*0.1;
			// Updating T0Local
			//   T0Local is the clock time when the local computer received the packet
			//   for the last time.  However, due to network delay, a packet that should
			//   have arrived earlier may arrive late.  If there's no network delay,
			//   the packet should have reached at T0Local+frRemote.  However, if
			//   the previous packet arrived late, T0Local+frLocal may be the planned
			//   arrival time.  So, frRemote*0.9+frlocal*0.1 takes in the middle.

			recvState.tRemote=T0Remote;
			recvState.tLocal=T0Local;
			prevState=recvState;
		}
		else // if(t<0.0 || timeOut<frt)
		{
			// If, it is the first packet, or the first packet since two seconds ago,
			// let's reset it.  I.e., take it as it is.
			T0Remote=TRemote-0.0; // -x.0 is for testing purpose.
			T0Local=TLocal;

			recvState.tRemote=T0Remote;
			recvState.tLocal=T0Local;
			prevState=recvState;
		}


		YsVec3 newPos,err;
		newPos.Set(x,y,z);
		err=newPos-guess;


		// Smooth correction;
		staPosition=guess;  // Recover guessed position.
		AddNetCorrection(err,0.5);
	}
	else
	{
//printf("?\n");
	}
}

unsigned FsAirplaneProperty::EncodeTurretState(unsigned char dat[],int idOnSvr) const
{
	const int netCmd=FSNETCMD_AIRTURRETSTATE;
	return FsVehicleProperty::EncodeTurretState(dat,idOnSvr,netCmd);
}

void FsAirplaneProperty::ApplyNetCorrection(const double &dt)
{
	int i;
	YsVec3 correctionSpeed,correctionForThisTimeStep;
	for(i=0; i<nNetCorrection; i++)
	{
		if(staNetCorrectionTime[i]>dt)
		{
			correctionSpeed=staNetCorrection[i]/staNetCorrectionTime[i];
			correctionForThisTimeStep=correctionSpeed*dt;
			staPosition+=correctionForThisTimeStep;

			staNetCorrection[i]-=correctionForThisTimeStep;
			staNetCorrectionTime[i]-=dt;
		}
		else
		{
			staPosition+=staNetCorrection[i];
			staNetCorrectionTime[i]=0.0;
			staNetCorrection[i]=YsOrigin();
		}
	}
}

void FsAirplaneProperty::AddNetCorrection(const YsVec3 &err,const double &duration)
{
	int i,empty,longest;
	empty=-1;
	longest=0;
	for(i=0; i<nNetCorrection; i++)
	{
		if(staNetCorrectionTime[i]<=YsTolerance)
		{
			empty=i;
			break;
		}
		else if(staNetCorrectionTime[i]>staNetCorrectionTime[longest])
		{
			longest=i;
		}
	}

	if(empty<0)
	{
		empty=longest;
	}

	staNetCorrectionTime[empty]=0.0;
	staNetCorrection[empty]=YsOrigin();


	YsVec3 plannedCorrection;
	plannedCorrection=YsOrigin();
	for(i=0; i<nNetCorrection; i++)
	{
		plannedCorrection+=staNetCorrection[i];
	}


	YsVec3 extraCorrection;
	extraCorrection=err-plannedCorrection;

	staNetCorrection[empty]=extraCorrection;
	staNetCorrectionTime[empty]=duration;
}

void FsAirplaneProperty::SmartRudder(const double & /*dt*/)
{
	if(staV>=chManSpeed1 && IsOnGround()!=YSTRUE && chMaxInputSSA>YsTolerance)
	{
		// More sophisticated control : Take rudder input as a desired SSA
		double ssa,desiredSSA;
		YsVec3 vel;
		double Kym,Kys,ommega;
		double ssaInput;

		desiredSSA=-chMaxInputSSA*ctlRudderUser;

		staAttitude.MulInverse(vel,staVelocity);
		ssa=atan2(-vel.x(),vel.z());

		Kym=GetYawManeuvabilityConst();
		Kys=GetYawStabilityConst();

		ommega=-staLateralForce/(staV*GetTotalWeight());

		ssaInput=(Kys/Kym)*ommega+desiredSSA+(ssa-desiredSSA)*4.0;  // The last term has no theoretical reasoning!
		ctlRudderControl=ssaInput/chMaxInputSSA-ctlRudderUser;
	}
	else
	{
		ctlRudderControl=0.0;
	}
}

void FsAirplaneProperty::TurnOffSmartRudder(void)
{
	ctlRudderControl=0.0;
}

void FsAirplaneProperty::ConfigureRudderBySSA(const double &ssa)
{
	SetRudder(-ssa/chMaxInputSSA);
}

void FsAirplaneProperty::AutoCoordinate(void)
{
	if((staState==FSFLYING || staState==FSSTALL) && staV>1.0)
	{
		double ssa;
		YsVec3 rel;

		YsMatrix4x4 mat(YSFALSE);
		staAttitude.GetMatrix4x4(mat);
		mat.Transpose();  // must be equivalent to mat.Invert();

		rel=mat*staVelocity;

		ssa=atan2(-rel.x(),rel.z());

		staAttitude.YawLeft(ssa);
	}
}

// void FsAirplaneProperty::MagicRudder(const double &dt)
// {
// 	if((staState==FSFLYING || staState==FSSTALL) && staV>1.0)
// 	{
// 		YsVec3 rel;
//
// 		YsMatrix4x4 mat(YSFALSE);
// 		staAttitude.GetMatrix4x4(mat);
// 		mat.Transpose();  // Invert(); must be equivalent for a matrix from YsAtt3
//
// 		rel=mat*staVelocity;
// 		rel.RotateXZ(chMaxInputSSA*ctlRudderUser*dt*chYawManConst/chYawStabConst/3.0);
// 		mat.MulInverse(staVelocity,rel,0.0);
// 	}
// }


YSBOOL FsAirplaneProperty::IsOnArrestingWire(void) const
{
	if(staOnThisCarrier!=NULL && staArrested==YSTRUE)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::AfterUnloadedFromCarrier(void)
{
	staOnThisCarrier=NULL;
	staArrested=YSFALSE;
	staCatapulted=YSFALSE;

	SetState(FSFLYING,FSDIEDOF_NULL);

	// This function needs to update foot.  Otherwise, what's going to happen is:
	//   (1) CalculateCarrierLanding function calls UnloadAirplane from which this function is called
	//   (2) Flight State is set to FSFLYING
	//     * Foot positions of the wheels stays the same *
	//   (3) CalculateGround function sees that the wheels are touching the ground and makes flight state back to FSGROUND
	//   (4) FsAirplane;:HitCollision is later called from FsSimulation and detects it as off-airport landing.
	//
	// Probably the best solution is as soon as the airplane is unloaded, it should be checked if the airplane
	// moved on to the next carrier, the airplane moved to an elevation grid, or really took off.
	//
	// It means that this function must know FsSimulation.

	// Second best solution >>
	for(int i=0; i<GetNumTire(); i++)
	{
		YsVec3 tirePos=staMatrix*GetTirePosition(i);
		tirePos.SetY(0.0);
		SetElevationAtTire(i,tirePos);
	}
	staGndElevation=0.0;
	staGndNormal=YsYVec();
	// << Second best solution
}

void FsAirplaneProperty::AfterLoadedOnCarrier(FsGround *carrier)
{
	staOnThisCarrier=carrier;
	staOutOfRunway=YSFALSE;
}

const double FsAirplaneProperty::GetCurrentRadarRange(void) const
{
	if(YSTRUE!=staRadarInop)
	{
		return staRadarRange;
	}
	else
	{
		return 0.0;
	}
}

YSBOOL FsAirplaneProperty::HasAirToAirRadar(void) const
{
	return chHasAirRadar;
}

YSBOOL FsAirplaneProperty::HasAirToGndRadar(void) const
{
	return chHasGroundRadar;
}

YSBOOL FsAirplaneProperty::HasBombingRadar(void) const
{
	return chHasBombingRadar;
}

YSRESULT FsAirplaneProperty::ComputeEstimatedBombLandingPosition(YsVec3 &estimated,const class FsWeather &weather) const
{
	const YsVec3 *pos;
	const YsAtt3 *att;
	YsVec3 vel,wind;
	const YsMatrix4x4 *mat;

	double a,b,c;
	double t,det,tCandidate1,tCandidate2;

	pos=&GetPosition();
	att=&GetAttitude();
	mat=&GetInverseMatrix();
	GetVelocity(vel);

	// y(t)=y0+vy0*t-0.5*FsGravityConst*t*t
	//
	// 0.5*FsGravityConst*t^2 -vy0*t -y0=0

	a= 0.5*FsGravityConst;
	b=-vel.y();
	c=-pos->y();

	det=b*b-4.0*a*c;
	if(det>=0)
	{
		tCandidate1=(-b+sqrt(det))/(2.0*a);
		tCandidate2=(-b-sqrt(det))/(2.0*a);

		// tCandidate2 is supposed to be negative.
		if(tCandidate1>=0.0)
		{
			t=tCandidate1;
		}
		else if(tCandidate2>=0.0)
		{
			t=tCandidate2;
		}
		else
		{
			return YSERR;
		}

		wind=weather.GetWind();

		estimated=(*pos)+(vel+wind)*t;
		estimated.SetY(0.0);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

void FsAirplaneProperty::SetVorStation(int navId,unsigned int vorKey)
{
	if(0<=navId && navId<2)
	{
		staVorKey[navId]=vorKey;
	}
}

void FsAirplaneProperty::SetVorObs(int navId,const double &heading)
{
	if(0<=navId && navId<2)
	{
		staObs[navId]=heading;
	}
}

unsigned int FsAirplaneProperty::GetVorStationKey(int navId) const
{
	if(0<=navId && navId<2)
	{
		return staVorKey[navId];
	}
	return YSNULLHASHKEY;
}

const double &FsAirplaneProperty::GetVorObs(int navId) const
{
	if(0<=navId && navId<2)
	{
		return staObs[navId];
	}
	return staObs[0];
}

int FsAirplaneProperty::GetPendingVorYsfId(int navId) const
{
	if(0<=navId && navId<2)
	{
		return staPendingVorYsfId[navId];
	}
	return 0;
}

void FsAirplaneProperty::SetPendingVorYsfId(int navId,int ysfId)
{
	if(0<=navId && navId<2)
	{
		staPendingVorYsfId[navId]=ysfId;
	}
}

void FsAirplaneProperty::SetHeadingBug(const double &hdgBug)
{
	staHdgBug=hdgBug;
}

const double &FsAirplaneProperty::GetHeadingBug(void) const
{
	return staHdgBug;
}

void FsAirplaneProperty::SetNdbStation(unsigned int ndbKey)
{
	staNdbKey=ndbKey;
}

void FsAirplaneProperty::UnsetNdbStation(void)
{
	staNdbKey=YSNULLHASHKEY;
}

unsigned int FsAirplaneProperty::GetNdbStationKey(void) const
{
	return staNdbKey;
}

int FsAirplaneProperty::GetPendingNdbYsfId(void) const
{
	return staPendingNdbYsfId;
}

void FsAirplaneProperty::SetPendingNdbYsfId(int ysfId)
{
	staPendingNdbYsfId=ysfId;
}

YSRESULT FsAirplaneProperty::AutoComputeTailStrikePitchAngle(const YsShell &collisionShell)
{
	// All computation is done in Z-Y plane  -> Converted to X-Y plane



	// First compute the stable ground attitude
	int i,j;
	YsVec2 gear[3];
	gear[0].Set(chMainGearL.z(),chMainGearL.y());
	gear[1].Set(chMainGearR.z(),chMainGearR.y());
	gear[2].Set(chWheel.z(),chWheel.y());

	for(i=0; i<3; i++)   // Just 3.  Run Bubble Sort
	{
		for(j=i+1; j<3; j++)
		{
			if(gear[i].x()>gear[j].x())
			{
				YsVec2 tmp;
				tmp=gear[i];
				gear[i]=gear[j];
				gear[j]=tmp;
			}
		}
	}

	// At this point, gear[0] has rear gear position,  gear[2] has front gear position.
	double stableGroundPitch;
	YsVec2 groundStaticLine;
	groundStaticLine=gear[2]-gear[0];
	stableGroundPitch=-atan2(groundStaticLine.y(),groundStaticLine.x());



	// Compute angle against all point of collisionShell that are behind
	// the rear wheel.
	YsVec2 &rear=gear[0];
	YsShellVertexHandle vtHd;
	const YsShellVertex *vtxPtr;
	double rearWheelToTailAngle;

	vtHd=NULL;
	rearWheelToTailAngle=YsPi/2.0;
	while((vtHd=collisionShell.FindNextVertex(vtHd))!=NULL)
	{
		YsVec3 vtxPos;
		YsVec2 tst;
		double angle;

		vtxPtr=collisionShell.GetVertex(vtHd);
		vtxPos=vtxPtr->GetPosition();

		tst.Set(vtxPos.z(),vtxPos.y());
		if(tst.x()<rear.x())
		{
			tst=tst-rear;
			if(tst.y()<=0.0)
			{
				printf("Warning : Wrong Collision Shell.\n");
				printf("   [%s]\n",GetIdentifier());
			}
			angle=atan2(tst.y(),-tst.x());
			if(angle<rearWheelToTailAngle)
			{
				rearWheelToTailAngle=angle;
			}
		}
	}


	chTailStrikePitch=YsGreater(rearWheelToTailAngle,stableGroundPitch);
	chGroundStaticPitch=stableGroundPitch;
	chTailStrikePitchIsComputed=YSTRUE;

	// printf("TailStrikeAngle [%s]=%.2lf\n",GetIdentifier(),YsRadToDeg(chTailStrikePitch));
	// printf("GroundStatic=%.2lf\n",YsRadToDeg(chGroundStaticPitch));
	return YSOK;
}

YSBOOL FsAirplaneProperty::TailStrikeAngleHasBeenComputed(void) const
{
	return chTailStrikePitchIsComputed;
}

double FsAirplaneProperty::GetTailStrikePitchAngle(const double &safetyFactor)
{
	if(chTailStrikePitchIsComputed!=YSTRUE)
	{
		printf("TailStrikeAngle is not computed for %s\n",GetIdentifier());
	}
	return chTailStrikePitch*safetyFactor;
}

const double &FsAirplaneProperty::GetGroundStaticPitchAngle(void) const
{
	if(chTailStrikePitchIsComputed!=YSTRUE)
	{
		printf("GroundStaticAngle is not computed for %s\n",GetIdentifier());
	}
	return chGroundStaticPitch;;
}

const double &FsAirplaneProperty::GetCriticalAOA(void) const
{
	return chMaxAOA;
}

void FsAirplaneProperty::GetPitchManeuvabilityConst(double &kPitch,double &bPitch)
{
	if(staV>=chManSpeed2)
	{
		kPitch=chPitchManConst;
	}
	else if(staV<chManSpeed1)
	{
		kPitch=0.0;
	}
	else
	{
		kPitch=chPitchManConst*(staV-chManSpeed1)/(chManSpeed2-chManSpeed1);
	}

	if(chPostStallVPitch<YsTolerance && fabs(ctlElevator)<0.1 && kPitch<chPitchManConst*0.1)
	{
		kPitch=chPitchManConst*0.1;
	}

	bPitch=chPitchStabConst;
}

double FsAirplaneProperty::GetRollManeuvabilityConst(void)
{
	if(staV>=chManSpeed2)
	{
		return chRollManConst;
	}
	else if(staV<chManSpeed1)
	{
		return 0.0;
	}
	else
	{
		return chRollManConst*(staV-chManSpeed1)/(chManSpeed2-chManSpeed1);
	}
}

double FsAirplaneProperty::GetYawManeuvabilityConst(void) const
{
	double kYaw;

	if(staV>=chManSpeed2)
	{
		kYaw=chYawManConst;
	}
	else if(staV<chManSpeed1)
	{
		kYaw=0.0;
	}
	else
	{
		kYaw=chYawManConst*(staV-chManSpeed1)/(chManSpeed2-chManSpeed1);
	}

	// kYaw is not zero so that user can control airplane on the ground
	if(staState==FSGROUND && staV>YsTolerance &&
	   staVelocity.y()<YsTolerance) // 2003/05/03 <- prevent spin of helicopters upon take off
	                                 // 2003/05/09 YsAbs is removed because other airplane cannot steer on the gnd
	{
		kYaw=YsGreater(kYaw,chYawManConst*3.0);
	}

	return kYaw;
}

double FsAirplaneProperty::GetGroundYawSpeed(const double &rudInput) const
{
	// Governing Equation
	//   h''=Kym*(ssaInput-ssa)-Kys*h'

	double Kym,Kys,inputSSA;
	Kym=GetYawManeuvabilityConst();
	Kys=chYawStabConst;

	// If h''=0, i.e., h' is stabilized,
	//   h'=(Kym/Kys)*(ssaInput-ssa), which is yaw speed.
	inputSSA=chMaxInputSSA*rudInput;
	return (Kym/Kys)*(inputSSA-staSSA);
}

double FsAirplaneProperty::GetBankControllerGain(void)
{
	return chRollManConst*chRollManConst/(8.0*chRollManConst*chMaxInputROLL);
}

double FsAirplaneProperty::GetYawStabilityConst(void)
{
	return chYawStabConst;
}

void FsAirplaneProperty::GetRollRate(double &vRoll)
{
	vRoll=chMaxInputROLL;
}

const double FsAirplaneProperty::GetMaxRollRate(void) const
{
	return chMaxInputROLL;
}

double FsAirplaneProperty::GetInputAOA(void)
{
	if(staOnThisCarrier==NULL || (staArrested!=YSTRUE && staCatapulted!=YSTRUE))
	{
		return chMaxInputAOA*(ctlElevator+ctlElvTrim);
	}
	else // if(staOnThisCarrier!=NULL && (staArrested==YSTRUE || staCatapulted==YSTRUE))
	{
		return 0.0;
	}
}

double FsAirplaneProperty::GetInputSSA(void)
{
	return chMaxInputSSA*GetRudder();
}

double FsAirplaneProperty::GetMaxInputSSA(void) const
{
	return chMaxInputSSA;
}

double FsAirplaneProperty::CalculateMaxTurnRateOnGround(void) const
{
	// staDVYaw=0 when (inputSsaCorrection-staSSA)*kYaw=staVYaw*chYawStabConst
	// inputSsaCorrection is same as inputSSA in a normal condition.
	// staSSA is zero on the ground
	// staVYaw=inputSsa*kYaw/chYawStabConst

	// Maximum turn rate on the ground is:
	//   maxInputSSA*kYaw/chYawStabConst

	const double kYaw=GetYawManeuvabilityConst();
	const double maxInputSSA=GetMaxInputSSA();
	return maxInputSSA*kYaw/chYawStabConst;
}

double FsAirplaneProperty::CalculateTurnRatiusOnGround(const double v) const
{
	// v=rw -> r=v/w  (w:turnRate)
	const double turnRate=CalculateMaxTurnRateOnGround();
	if(YsTolerance<turnRate)
	{
		return v/turnRate;
	}
	return 1.0;  // It is in fact, infinity.  It is a cowardly way of avoiding zero division.
}

double FsAirplaneProperty::GetInputROLL(void)
{
	return chMaxInputROLL*ctlAileron;
}

void FsAirplaneProperty::CalculateAutoPilot(const double &dt)
{
#ifdef CRASHINVESTIGATION_AUTOPILOT
	printf("AP1\n");
	printf("%lf\n",ctlSpeedControllerInput);
	printf("AP1.5\n");
#endif

	if(ctlSpeedControllerSwitch==YSTRUE)
	{
		ControlSpeed(ctlSpeedControllerInput,dt);
	}

#ifdef CRASHINVESTIGATION_AUTOPILOT
	printf("AP2\n");
#endif

	if(ctlBankControllerSwitch==YSTRUE)
	{
		ControlBank(ctlBankControllerInput,ctlBankControllerRollRateLimit,dt);
	}

#ifdef CRASHINVESTIGATION_AUTOPILOT
	printf("AP3\n");
#endif

	if(ctlPitchControllerSwitch==YSTRUE)
	{
		ControlPitch(ctlPitchControllerInput,dt);
	}

#ifdef CRASHINVESTIGATION_AUTOPILOT
	printf("AP4\n");
#endif

	if(ctlGControllerSwitch==YSTRUE)
	{
		ControlG(ctlGControllerInput,ctlGControllerMinAOALimit,ctlGControllerMaxAOALimit,dt);
	}

#ifdef CRASHINVESTIGATION_AUTOPILOT
	printf("AP5\n");
#endif

	if(ctlAOAControllerSwitch==YSTRUE)
	{
		ControlAOA(ctlAOAControllerInput,ctlAOAControllerGLimit,dt);
	}

#ifdef CRASHINVESTIGATION_AUTOPILOT
	printf("AP6\n");
#endif

	if(ctlAutoVgw==YSTRUE)
	{
		double v;
		v=GetVelocity();
		if(v<chAutoVgwSpeed1)
		{
			ctlVgw=1.0;
		}
		else if(v>chAutoVgwSpeed2)
		{
			ctlVgw=0.0;
		}
		else
		{
			ctlVgw=1.0-(v-chAutoVgwSpeed1)/(chAutoVgwSpeed2-chAutoVgwSpeed1);
		}

//		double mach;
//		mach=GetMach();
//		if(mach<0.3)
//		{
//			ctlVgw=1.0;
//		}
//		else if(mach>0.8)
//		{
//			ctlVgw=0.0;
//		}
//		else
//		{
//			ctlVgw=1.0-(mach-0.3)/0.5;
//		}
	}

#ifdef CRASHINVESTIGATION_AUTOPILOT
	printf("AP7\n");
#endif

}

void FsAirplaneProperty::ControlSpeed(const double &spd,const double &dt)
{
	// Calculate Equilibrium
	// At at desired speed, at current AOA, at current attitude,
	// How large is the drag?

	// MEMO:
	//  Giving inappropriate desired speed will cause unexpected result in the speed controller,
	//  because it cannot estimate lift and drag accurately.

	double aoa,drag,lift,weight,grav,thr;
	YsVec3 vDrag,vLift,vGrav,vTotal;
	const double currentV=GetVelocity();

	aoa=GetAOA();
	aoa=YsBound(aoa,YsDegToRad(-89.0),YsDegToRad(89.0)); // Prevent div by 0
	lift=GetLift(aoa,spd);
	drag=GetDrag(aoa,spd);
	weight=GetTotalWeight();
	grav=FsGravityConst*weight;

	// Assuming Small Sideslip Angle
	vLift.Set(0.0, lift*cos(aoa), lift*sin(aoa));
	vDrag.Set(0.0, drag*sin(aoa),-drag*cos(aoa));

	vGrav.Set(0.0,-grav,0.0);
	GetAttitude().MulInverse(vGrav,vGrav);

	// All forces are transformed into the Airplane's local coordinate
	vTotal=vLift+vDrag+vGrav;


	double requiredThrust;
	requiredThrust=-vTotal.z()/cos(aoa);


	// Accumulate Error
	ctlIntegralSpdErr+=(spd-GetVelocity())*dt;
	ctlIntegralSpdErr=YsBound(ctlIntegralSpdErr,-5.0,5.0);

	// Calculate differential
	const double spdErr=spd-staV;
	const double ctlDiffSpdErr=(YsTolerance<dt ? (spdErr-ctlPrevSpdErr)/dt : 0.0);
	ctlPrevSpdErr=spdErr;


	if(0<chRealProp.GetN())
	{
		const double thr0=GetConvergentThrust(0.0,staPosition.y()+staBaseElevation,spd,YSFALSE);
		const double thrCurrent=GetConvergentThrust(staThrottle,staPosition.y()+staBaseElevation,spd,YSFALSE);
		const double thr1=GetConvergentThrust(1.0,staPosition.y()+staBaseElevation,spd,YSFALSE);

		double throttleCorrection=0.0;
		if(YsTolerance<refSpdCruise)
		{
			const double spdDiffRelativeToCruisingSpeed=(spd-staV)/refSpdCruise;
			const double Kp=2.0;
			const double Ki=0.01;
			throttleCorrection=Kp*spdDiffRelativeToCruisingSpeed+Ki*ctlIntegralSpdErr;
		}

		// printf("%s Required %lf Current %lf\n",GetIdentifier(),spd,staV);
		// printf("   Integral %lf Correction %lf\n",ctlIntegralSpdErr,throttleCorrection);

		if(requiredThrust<thr0)
		{
			// cd=cd*(1.0+chCdSpoiler*staSpoiler);  <- effect of the spoiler
			double drag0,spl;

			drag0=drag/(1.0+chCdSpoiler*staSpoiler);
			if(chCdSpoiler>YsTolerance && YsEqual(drag0,0.0)!=YSTRUE)
			{
				spl=((-(requiredThrust-thr0)/drag0)-1.0)/chCdSpoiler;
				spl=YsBound(spl,0.0,1.0);
			}
			else
			{
				spl=0.0;
			}

			SetThrottle(0.0);
			SetSpoiler(spl);
		}
		else if(thr1<requiredThrust)
		{
			SetThrottle(1.0);
			SetSpoiler(0.0);
		}
		else
		{
			double thr=0.0;
			if(requiredThrust<thrCurrent)
			{
				thr=staThrottle*(requiredThrust-thr0)/(thrCurrent-thr0);
			}
			else
			{
				const double t=(requiredThrust-thrCurrent)/(thr1-thrCurrent);
				thr=staThrottle*(1.0-t)+1.0*t;
			}
			SetThrottle(thr+throttleCorrection);
			SetSpoiler(0.0);
		}
	}
	else
	{
		// Propotional Controller
		// Say 1.0/Kp seconds to correct speed difference
		const double Kp=0.2;
		double Yp=Kp*(spd-currentV)*weight*FsGravityConst;

		// Integral Controller
		const double Ki=0.3;
		double Yi=Ki*ctlIntegralSpdErr*weight*FsGravityConst;

		requiredThrust+=Yp+Yi;

		if(requiredThrust<0.0)
		{
			// cd=cd*(1.0+chCdSpoiler*staSpoiler);  <- effect of the spoiler
			double drag0,spl;

			drag0=drag/(1.0+chCdSpoiler*staSpoiler);
			if(chCdSpoiler>YsTolerance && YsEqual(drag0,0.0)!=YSTRUE)   // 2008/09/16
			{
				spl=((-requiredThrust/drag0)-1.0)/chCdSpoiler;
				spl=YsBound(spl,0.0,1.0);
			}
			else
			{
				spl=0.0;
			}

			SetThrottle(0.0);
			SetSpoiler(spl);
			// printf("Spoiler Yp:%.2lf  Yi:%.2lf  Req:%.2lf  Drag:%.2lf  Out:%.2lf\n",Yp,Yi,requiredThrust,drag,spl);
		}
		else
		{
			SetSpoiler(0.0);
			if(YSTRUE!=GetHasAfterburner() || YSTRUE!=ctlSpeedControllerCanUseAfterburner)
			{
				double thr1,thr2,thrm;
				thr1=0.0;
				thr2=1.0;
				while(thr2-thr1>0.01)
				{
					thrm=(thr1+thr2)/2.0;
					if(requiredThrust<GetThrust(thrm,staPosition.y()+staBaseElevation,staV,YSFALSE))
					{
						thr2=thrm;
					}
					else
					{
						thr1=thrm;
					}
				}
				SetThrottle(YsSmaller(thrm,ctlSpeedControllerThrottleCap));
				SetAfterburner(YSFALSE);
			}
			else
			{
				double maxAbThr,minAbThr,milThr;
				milThr=GetThrust(1.0,staPosition.y(),staV,YSFALSE);
				maxAbThr=GetThrust(1.0,staPosition.y(),staV,YSTRUE);
				minAbThr=milThr*0.8+maxAbThr*0.2;

				// printf("Yp:%.1lf  Yi:%.1lf  Req:%.1lf  Mil:%.1lf  AB:%.1lf\n",Yp,Yi,requiredThrust,milThr,maxAbThr);

				// Goal : Make them reluctant to turn on/off afterburner,
				//        at the same time, speed must be precisely and quickly controlled.
				if(GetAfterBurner()==YSTRUE)
				{
					const double ratio=0.7;
					// Don't turn it off unless required thrust is smaller than milThr*ratio
					if(requiredThrust<milThr*ratio)
					{
						SetAfterburner(YSFALSE);
						SetThrottle(requiredThrust/milThr);
					}
					else if(requiredThrust<milThr*(1.0-ratio)+maxAbThr*ratio)
					{
						SetThrottle(ratio);
					}
					else
					{
					 	if(YsAbs(maxAbThr-milThr)>YsTolerance)
					 	{
					 		thr=(requiredThrust-milThr)/(maxAbThr-milThr);
					 	}
					 	else
					 	{
					 		thr=0.0;
					 	}
					 	SetThrottle(YsSmaller(thr,ctlSpeedControllerThrottleCap));
					}
				}
				else
				{
					const double ratio=0.3;
					// Don't turn it on uless required thrust is greater than milThr*(1.0-ratio)+maxAbThr*ratio
					if(milThr*(1.0-ratio)+maxAbThr*ratio<requiredThrust)
					{
					 	if(YsAbs(maxAbThr-milThr)>YsTolerance)
					 	{
					 		thr=(requiredThrust-milThr)/(maxAbThr-milThr);
					 	}
					 	else
					 	{
					 		thr=0.0;
					 	}
					 	SetAfterburner(YSTRUE);
					 	SetThrottle(YsSmaller(thr,ctlSpeedControllerThrottleCap));
					}
					else
					{
						thr=requiredThrust/milThr;
						thr=YsBound(thr,0.0,1.0);
						SetThrottle(YsSmaller(thr,ctlSpeedControllerThrottleCap));
					}
				}
			}
		}
	}
}

void FsAirplaneProperty::ControlBank(const YsVec3 &uv,const double &rollRateLimit,const double &dt)
{
	double diff,Kp;   // Kp : Propotional Controller Gain
	Kp=GetBankControllerGain();

	YsVec3 ref;
	staAttitude.MulInverse(ref,uv);
	diff=atan2(-ref.x(),ref.y());

	// diff=bankInput-staAttitude.b();


	double ailLimit;
	ailLimit=YsSmaller(1.0,rollRateLimit/chMaxInputROLL);

	ctlAileron=YsBound(Kp*diff*4.0,-ailLimit,ailLimit);
}

void FsAirplaneProperty::ControlPitch(const double &pitchInput,const double &dt)
{
	double dElv;
	double estimatedPitch;
	const double deltaT=1.0;

	YsAtt3 future(staAttitude);
	future.NoseUp(staVPitch*deltaT+0.5*staDVPitch*deltaT*deltaT);
	estimatedPitch=future.p();


	double estimatedG;
	double dg,ddg;

	dg=(staG-staPrevG)/dt;
	ddg=((staG-staPrevG)-(staPrevG-staPrevPrevG))/dt;

	estimatedG=staG+dg*deltaT+0.5*ddg*deltaT;  // After 1 second

	dElv=0.0;

	if(-2.0<estimatedG && estimatedG<5.0)
	{
		if(estimatedPitch>pitchInput)
		{
			dElv=-ELVRESPONCE*dt;
		}
		if(estimatedPitch<pitchInput)
		{
			dElv=+ELVRESPONCE*dt;
		}

		if(YsAbs(staAttitude.b())>YsPi/2.0)
		{
			dElv=-dElv;
		}
	}
	else if(estimatedG<-3.0)
	{
			dElv=+ELVRESPONCE*dt;
	}
	else // if(estimatedG>5.0)
	{
		dElv=-ELVRESPONCE*dt;
	}

	ctlElevator=YsBound(ctlElevator+dElv,-1.0,1.0);
	LimitElevator();
}

void FsAirplaneProperty::ControlG
    (const double &gInput,const double &minAOALimit,const double &maxAOALimit,const double &dt)
{
// So far, it looks the best controller I came up with.
	double L0,L5;  // Lex : expected
	double Lreq,Areq,elvEstim;
	double Ainp;

	L0=GetLift(0.0,staV);
	L5=GetLift(YsDegToRad(5.0),staV);
	Lreq=gInput*(GetTotalWeight()*FsGravityConst);  // Required lift
	if(YsTolerance<L5-L0)
	{
		double Lex,Gex;

		Lex=L0+GetAOA()*(L5-L0)/YsDegToRad(5.0);
		Gex=Lex/(GetTotalWeight()*FsGravityConst);

		Areq=(Lreq-L0)/((L5-L0)/YsDegToRad(5.0));

		// Why don't I limit AOA here?
		Areq=YsBound(Areq,minAOALimit,maxAOALimit);
	}
	else
	{
		Areq=0.0;
	}

	if(IsOnGround()==YSTRUE)  // See CalculateRotationalAcceleration for explanation of this correction.
	{
		Areq+=YsDegToRad(2.0);
	}

	YsVec3 Lvec;
	double dP_est;
	Lvec.Set(0.0,Lreq,0.0);
	staAttitude.Mul(Lvec,Lvec);
	Lvec.AddY(-FsGravityConst*GetTotalWeight());
	staAttitude.MulInverse(Lvec,Lvec);

	if(YsTolerance<staV)
	{
		dP_est=Lvec.y()/(GetTotalWeight()*staV);
	}
	else
	{
		dP_est=0.0;
	}

	NeutralDirectPitchControl();
	NeutralDirectRollControl();
	NeutralDirectYawControl();

	// 2014/06/24 Separated direct attitude control and movable wing surface control.
	// if(chPostStallVPitch<YsTolerance || YsAbs(staVPitch)>=chPostStallVPitch)
	{
		double Kp,Kb,correction;

		GetPitchManeuvabilityConst(Kp,Kb);
		if(Kp>YsTolerance)
		{
			correction=(Kb/Kp)*dP_est;
		}
		else
		{
			correction=0.0;
		}

		Ainp=Areq+correction;

		// printf("AREQ:% 8.2lf  AINP:% 8.2lf  CORR:% 8.2lf  DP_EST:% 8.2lf\n",Areq,Ainp,correction,dP_est);
		elvEstim=Ainp/chMaxInputAOA-ctlElvTrim;
	}
	// else
	// {
	// 	double Kp,Kb;
	// 	GetPitchManeuvabilityConst(Kp,Kb);
	// 	if(Kp>YsTolerance && YsTolerance<chMaxInputAOA+chPostStallVPitch)
	// 	{
	// 		elvEstim=((Kb/Kp+1)*dP_est+Areq)/(chMaxInputAOA+chPostStallVPitch);
	// 	}
	// 	else
	// 	{
	// 		elvEstim=0.0;
	// 	}
	// }
	ctlElevator=elvEstim;

//printf("Ginput:% 8.2lf  Goutput:% 8.2lf  Err:% 8.2lf\n",gInput,staG,gInput-staG);
//printf("Areq:% 8.2lf  AInput: % 8.2lf  AOutput:% 8.2lf  Err:% 8.2lf\n",
//    YsRadToDeg(Areq),YsRadToDeg(Ainp),YsRadToDeg(GetAOA()),YsRadToDeg(Areq)-YsRadToDeg(GetAOA()));

	LimitElevator(); // (minAOALimit,maxAOALimit);

// printf("Elv(c):%.2lf\n",ctlElevator);

// printf("E:%lf\n",ctlElevator);
}

void FsAirplaneProperty::ControlAOA(const double &aoaInput,const double &gLimit,const double &dt)
{
	// Similar to the G Controller

	double Lreq,Areq,elvEstim;
	double Ainp;

	Areq=aoaInput;
	if(IsOnGround()==YSTRUE)  // See CalculateRotationalAcceleration for explanation of this correction.
	{
		Areq+=YsDegToRad(2.0);
	}

	Lreq=GetLift(aoaInput,staV);

	YsVec3 Lvec;
	double dP_est;
	Lvec.Set(0.0,Lreq,0.0);
	staAttitude.Mul(Lvec,Lvec);
	Lvec.AddY(-FsGravityConst*GetTotalWeight());
	staAttitude.MulInverse(Lvec,Lvec);

	if(YsTolerance<staV)
	{
		dP_est=Lvec.y()/(GetTotalWeight()*staV);
	}
	else
	{
		dP_est=0.0;
	}


	if(IsOnGround()==YSTRUE && dP_est<0.0)
	{
		dP_est=0.0;
	}

	NeutralDirectPitchControl();
	NeutralDirectRollControl();
	NeutralDirectYawControl();

	// 2014/06/24 Separated direct attitude control and movable wing surface control.
	// if(chPostStallVPitch<YsTolerance || YsAbs(staVPitch)>=chPostStallVPitch)
	{
		double Kp,Kb,correction;
		GetPitchManeuvabilityConst(Kp,Kb);
		if(Kp>YsTolerance)
		{
			correction=(Kb/Kp)*dP_est;
		}
		else
		{
			correction=0.0;
		}
		Ainp=Areq+correction;

		// printf("AREQ:% 8.2lf  AINP:% 8.2lf  CORR:% 8.2lf  DP_EST:% 8.2lf\n",Areq,Ainp,correction,dP_est);
		if(chMaxInputAOA>YsTolerance)  // 20060807
		{
			elvEstim=Ainp/chMaxInputAOA-ctlElvTrim;
		}
		else
		{
			elvEstim=0.0;
		}
	}
	// else
	// {
	// 	double Kp,Kb;
	// 	GetPitchManeuvabilityConst(Kp,Kb);
	// 	if(Kp>YsTolerance && (chMaxInputAOA+chPostStallVPitch)>YsTolerance)  // 20060807
	// 	{
	// 		elvEstim=((Kb/Kp+1)*dP_est+Areq)/(chMaxInputAOA+chPostStallVPitch);
	// 	}
	// 	else
	// 	{
	// 		elvEstim=0.0;
	// 	}
	// }
	ctlElevator=elvEstim;

	LimitElevator();

	// printf("AOA-in:%6.2lf  AOA-out:%6.2lf\n",YsRadToDeg(aoaInput),YsRadToDeg(GetAOA()));
}

void FsAirplaneProperty::LimitElevator() //(const double &minAOA,const double &maxAOA)
{
	ctlElevator=YsBound(ctlElevator,-1.0,1.0);
}

double FsAirplaneProperty::CalculatePropellerThrust(const double &thr,const double &alt,const double &vel) const
{
	double rhoRef,rhoZero;

	rhoRef=FsGetAirDensity(alt+staBaseElevation);
	rhoZero=FsGetZeroAirDensity();
	if(chPropV0<vel)
	{
		double power;
		power=chThrMil*thr*chPropEfficiency;
		return (rhoRef/rhoZero)*(power/vel);
	}
	else
	{
		double maxForceV0,maxForceV;
		maxForceV0=chThrMil*chPropEfficiency/chPropV0;
		maxForceV=maxForceV0-chPropK*(chPropV0-vel);  // See propthrust.doc

		return maxForceV*thr*(rhoRef/rhoZero);
	}
}

void FsAirplaneProperty::SetupVisual(FsVisualDnm &vis) const
{
	// Test Concorde Gear Door 20030901
	//vis.SetState(0,0,1,staGear);
	if(staGear<0.2)
	{
		vis.SetState(0,0,1,0.0);
		vis.SetState(14,0,1,staGear/0.2);
		vis.SetState(17,0,1,staGear/0.2);
		vis.SetState(YSDNM_CLASSID_TIRE,0);
		// vis.SetState(15,0,1,staGear);  Changed the gear room behavior 2006/04/22
	}
	else if(staGear<0.8)
	{
		vis.SetState(0,0,1,(staGear-0.2)/0.6);
		vis.SetState(14,1);
		vis.SetState(17,1);
		vis.SetState(YSDNM_CLASSID_TIRE,1);
		// vis.SetState(15,0,1,staGear);  Changed the gear room behavior 2006/04/22
	}
	else
	{
		vis.SetState(0,1);
		vis.SetState(14,0,1,(1.0-staGear)/0.2);
		vis.SetState(17,1);
		vis.SetState(YSDNM_CLASSID_TIRE,1);
		// vis.SetState(15,0,1,staGear);  Changed the gear room behavior 2006/04/22
	}

	if(staGear<0.6) //   Changed the gear room behavior 2006/04/22
	{
		vis.SetState(15,0,1,staGear/0.6);
	}
	else
	{
		vis.SetState(15,0,1,1.0);
	}
	//

	vis.SetState(1,0,1,staVgw);
	vis.SetState(4,0,1,staSpoiler);
	vis.SetState(5,0,1,staFlap);

	vis.SetState(11,0,1,staThrRev);

	vis.SetState(9,0,1,staBombBayDoor);

	double elvPlusTrim;
	elvPlusTrim=YsBound(ctlElevator+ctlElvTrim,-1.0,1.0);
	if(elvPlusTrim>=0.0)
	{
		vis.SetState(6,0,1,elvPlusTrim);
	}
	else
	{
		vis.SetState(6,0,2,-elvPlusTrim);
	}
	if(ctlAileron>=0.0)
	{
		vis.SetState(7,0,1,ctlAileron);
	}
	else
	{
		vis.SetState(7,0,2,-ctlAileron);
	}
	if(GetRudder()>=0.0)
	{
		vis.SetState(YSDNM_CLASSID_RUDDER,0,1,GetRudder());
		vis.SetState(YSDNM_CLASSID_STEERING,0,1,GetRudder());
	}
	else
	{
		vis.SetState(YSDNM_CLASSID_RUDDER,0,2,-GetRudder());
		vis.SetState(YSDNM_CLASSID_STEERING,0,2,-GetRudder());
	}

	if(chHasAb==YSTRUE && staAb==YSTRUE)
	{
		vis.SetState(2,1);
	}
	else
	{
		vis.SetState(2,0);
	}

	if(chHasThrustVector==YSTRUE)
	{
		vis.SetState(10,0,1,GetThrustVector());
	}


	if(staThrottle>0.6)
	{
		vis.SetShow(20,YSTRUE);  // Fast
		vis.SetShow(18,YSFALSE); // Slow
	}
	else if(staThrottle>0.3)
	{
		vis.SetShow(20,YSTRUE);  // Fast
		vis.SetShow(18,YSTRUE);  // Slow
	}
	else
	{
		vis.SetShow(20,YSFALSE); // Fast
		vis.SetShow(18,YSTRUE);  // Slow
	}


	vis.SetState(12,0,1,GetThrustVector());
	vis.SetState(13,0,1,YsBound(GetThrustVector()*2.5,0.0,1.0));
	vis.SetState(YSDNM_CLASSID_BRAKE,0,1,staBrake);

	vis.SetAngle(YSDNM_CLASSID_ROTOR,staRotorAngle);
	vis.SetRotation(YSDNM_CLASSID_ROTOR_CUSTOM_AXIS,staRotorAngle);
	vis.SetPitch(YSDNM_CLASSID_TIRE,staWheelAngle);

	int i;
	for(i=0; i<chTurret.GetN(); i++)
	{
		staTurret[i].SetupVisual(vis,chTurret[i]);
	}


	vis.SetShow(YSDNM_CLASSID_NAVLIGHT,staNavLight);
	vis.SetShow(YSDNM_CLASSID_LANDINGLIGHT,staLandingLight);

	if(YSTRUE==staLandingLight && staGear>0.2)
	{
		vis.SetShow(YSDNM_CLASSID_LIGHTONLANDINGGEAR,YSTRUE);
	}
	else
	{
		vis.SetShow(YSDNM_CLASSID_LIGHTONLANDINGGEAR,YSFALSE);
	}

	if(staBeacon==YSTRUE && fmod(staHobbsTime,chBeaconBlankTime+chBeaconLightTime)<chBeaconLightTime)
	{
		vis.SetShow(YSDNM_CLASSID_BEACON,YSTRUE);
	}
	else
	{
		vis.SetShow(YSDNM_CLASSID_BEACON,YSFALSE);
	}

	if(staStrobe==YSTRUE && fmod(staHobbsTime,chStrobeBlankTime+chStrobeLightTime)<chStrobeLightTime)
	{
		vis.SetShow(YSDNM_CLASSID_STROBE,YSTRUE);
	}
	else
	{
		vis.SetShow(YSDNM_CLASSID_STROBE,YSFALSE);
	}

	vis.CacheTransformation();
}

void FsAirplaneProperty::DrawOrdinance(
    unsigned int,  // drawFlag
    const YsMatrix4x4 *drawMat,
    const class FsVisualDnm weaponShapeOverrideStatic[]) const
{
	int i;
	YsVec3 pos;
	const YsMatrix4x4 *mat;

	if(drawMat==NULL)
	{
		mat=&staMatrix;
	}
	else
	{
		mat=drawMat;
	}

	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		if(YSTRUE==IsWeaponSlotCurrentlyVisible(i))
		{
			if(NULL!=weaponShapeOverrideStatic &&
			   FSWEAPON_NUMWEAPONTYPE>staWeaponSlot[i].wpnType &&
			   nullptr!=weaponShapeOverrideStatic[(int)staWeaponSlot[i].wpnType])
			{
	 			pos=(*mat)*chWeaponSlot[i].pos;
	 			weaponShapeOverrideStatic[(int)staWeaponSlot[i].wpnType].Draw(pos,staAttitude);
			}
			else switch(staWeaponSlot[i].wpnType)
			{
			default:
				break;
			case FSWEAPON_AIM9:
				if(chAAMVisible==YSTRUE && FsWeapon::aim9s!=nullptr)
				{
		 			pos=(*mat)*chWeaponSlot[i].pos;
		 			FsWeapon::aim9s.Draw(pos,staAttitude);
				}
				break;
			case FSWEAPON_AIM9X:
				if(chAAMVisible==YSTRUE && FsWeapon::aim9xs!=nullptr)
				{
		 			pos=(*mat)*chWeaponSlot[i].pos;
		 			FsWeapon::aim9xs.Draw(pos,staAttitude);
				}
				break;
			case FSWEAPON_AIM120:
				if(chAAMVisible==YSTRUE && FsWeapon::aim120s!=nullptr)
				{
		 			pos=(*mat)*chWeaponSlot[i].pos;
		 			FsWeapon::aim120s.Draw(pos,staAttitude);
				}
				break;
			case FSWEAPON_AGM65:
				if(chAGMVisible==YSTRUE && FsWeapon::agm65s!=nullptr)
				{
		 			pos=(*mat)*chWeaponSlot[i].pos;
		 			FsWeapon::agm65s.Draw(pos,staAttitude);
		 		}
				break;
			case FSWEAPON_BOMB:
				if(chBombVisible==YSTRUE && FsWeapon::bomb!=nullptr)
				{
					pos=(*mat)*chWeaponSlot[i].pos;
		 			FsWeapon::bomb.Draw(pos,staAttitude);
		 		}
				break;
			case FSWEAPON_BOMB250:
				if(chBombVisible==YSTRUE && FsWeapon::bomb250!=nullptr)
				{
					pos=(*mat)*chWeaponSlot[i].pos;
		 			FsWeapon::bomb250.Draw(pos,staAttitude);
		 		}
				break;
			case FSWEAPON_BOMB500HD:
				if(YSTRUE==chBombVisible && nullptr!=FsWeapon::bomb500hd)
				{
					pos=(*mat)*chWeaponSlot[i].pos;
					FsWeapon::bomb500hds.Draw(pos,staAttitude);
				}
				break;
			case FSWEAPON_ROCKET:
				if(chRocketVisible==YSTRUE && FsWeapon::rockets!=nullptr)
				{
					pos=(*mat)*chWeaponSlot[i].pos;
					FsWeapon::rockets.Draw(pos,staAttitude);
				}
				break;
			case FSWEAPON_FLARE:
				{
					pos=(*mat)*chWeaponSlot[i].pos;
					FsWeapon::flarePod.Draw(pos,staAttitude);
				}
				break;
			case FSWEAPON_FUELTANK:
				{
					pos=(*mat)*chWeaponSlot[i].pos;
					FsWeapon::fuelTank.Draw(pos,staAttitude);
				}
				break;
			}
		}
	}
}

void FsAirplaneProperty::DrawOrdinanceVisual(
    YSBOOL coarse,
	const class FsVisualDnm weaponShapeOverrideStatic[FSWEAPON_NUMWEAPONTYPE],
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,
    unsigned int drawFlag,
    const YsMatrix4x4 *drawMat) const
{
	int i;
	YsVec3 pos;
	const YsMatrix4x4 *mat;

	if(drawMat==NULL)
	{
		mat=&staMatrix;
	}
	else
	{
		mat=drawMat;
	}

	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		if(YSTRUE==IsWeaponSlotCurrentlyVisible(i))
		{
			FSWEAPONTYPE wpnType;
			switch(staWeaponSlot[i].wpnType)
			{
			default:
				wpnType=staWeaponSlot[i].wpnType;
				break;
			case FSWEAPON_FLARE:
				wpnType=FSWEAPON_FLAREPOD;
				break;
			}

			pos=(*mat)*chWeaponSlot[i].pos;

			if(YSTRUE!=coarse &&
			   NULL!=weaponShapeOverrideStatic &&
			   FSWEAPON_NUMWEAPONTYPE>staWeaponSlot[i].wpnType &&
			   NULL!=weaponShapeOverrideStatic[(int)staWeaponSlot[i].wpnType])
			{
	 			weaponShapeOverrideStatic[(int)staWeaponSlot[i].wpnType].Draw(viewTfm,projTfm,pos,staAttitude,drawFlag);
			}
			else
			{
				FsWeapon::DrawVisual(wpnType,coarse,viewTfm,projTfm,pos,staAttitude,drawFlag);
			}
		}
	}
}

void FsAirplaneProperty::DrawOrdinanceShadow(
	     YSBOOL coarse,
	     const class FsVisualDnm weaponShapeOverrideStatic[],
	     const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm,
	     const YsMatrix4x4 *drawMat) const
{
	int i;
	YsVec3 pos;
	const YsMatrix4x4 *mat;

	if(drawMat==NULL)
	{
		mat=&staMatrix;
	}
	else
	{
		mat=drawMat;
	}

	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		if(YSTRUE==IsWeaponSlotCurrentlyVisible(i))
		{
			FSWEAPONTYPE wpnType;
			switch(staWeaponSlot[i].wpnType)
			{
			default:
				wpnType=staWeaponSlot[i].wpnType;
				break;
			case FSWEAPON_FLARE:
				wpnType=FSWEAPON_FLAREPOD;
				break;
			}

			pos=(*mat)*chWeaponSlot[i].pos;

			if(YSTRUE!=coarse &&
			   NULL!=weaponShapeOverrideStatic &&
			   FSWEAPON_NUMWEAPONTYPE>staWeaponSlot[i].wpnType &&
			   NULL!=weaponShapeOverrideStatic[(int)staWeaponSlot[i].wpnType])
			{
	 			weaponShapeOverrideStatic[(int)staWeaponSlot[i].wpnType].DrawShadow(viewTfm,projTfm,pos,staAttitude,projPlnTfm);
			}
			else
			{
				FsWeapon::DrawShadow(wpnType,coarse,viewTfm,projTfm,pos,staAttitude,projPlnTfm);
			}
		}
	}
}

YSBOOL FsAirplaneProperty::IsWeaponSlotCurrentlyVisible(int slotId) const
{
	if(YSTRUE==chWeaponSlot.IsInRange(slotId) && YSTRUE==chWeaponSlot[slotId].isExternal)
	{
		if(0<staWeaponSlot[slotId].nLoaded || 0<staWeaponSlot[slotId].nContainerLoaded)
		{
			switch(staWeaponSlot[slotId].wpnType)
			{
			case FSWEAPON_AIM120:
			case FSWEAPON_AIM9:
			case FSWEAPON_AIM9X:
				return chAAMVisible;
			case FSWEAPON_AGM65:
				return chAGMVisible;
			case FSWEAPON_BOMB:
			case FSWEAPON_BOMB250:
			case FSWEAPON_BOMB500HD:
				return chBombVisible;
			case FSWEAPON_ROCKET:
				return chRocketVisible;
			default:
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

YSRESULT FsAirplaneProperty::ToggleGear(void)
{
	if(ctlGear>0.5 && IsOnGround()!=YSTRUE)
	{
		ctlGear=0.0;
	}
	else
	{
		ctlGear=1.0;
	}
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetGear(const double &a)
{
	double neo;
	neo=a;
	neo=YsBound(neo,0.0,1.0);
	if(IsOnGround()!=YSTRUE || neo>=1.0-YsTolerance)
	{
		ctlGear=neo;
	}
	return YSOK;
}

YSRESULT FsAirplaneProperty::ToggleBrake(void)
{
	if(ctlBrake>0.5)
	{
		ctlBrake=0.0;
		ctlSpoiler=0.0;
	}
	else
	{
		ctlBrake=1.0;
		ctlSpoiler=1.0;
	}
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetBrake(const double &a)
{
	ctlBrake=YsBound(a,0.0,1.0);
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetSpoiler(const double &a)
{
	ctlSpoiler=YsBound(a,0.0,1.0);
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetAfterburner(YSBOOL sw)
{
	if(chHasAb==YSTRUE)
	{
		ctlAb=sw;
		if(sw==YSTRUE && ctlThrottle<0.2)
		{
			ctlThrottle=0.2;
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsAirplaneProperty::SetPropellerAll(const double prop)
{
	for(auto &propLever : staPropLever)
	{
		propLever=prop;
	}
	return YSOK;
}

YSRESULT FsAirplaneProperty::ToggleAfterburner(void)
{
	if(chHasAb==YSTRUE)
	{
		return SetAfterburner(ctlAb==YSTRUE ? YSFALSE : YSTRUE);
	}
	return YSERR;
}

YSRESULT FsAirplaneProperty::SetThrottle(const double &a)
{
	double pCtlThrottle;

	pCtlThrottle=ctlThrottle;


	ctlThrottle=YsBound(a,0.0,1.0);

	if(ctlThrottle<0.2)
	{
		ctlAb=YSFALSE;
	}

	return YSOK;
}

YSRESULT FsAirplaneProperty::SetThrottleByRequiredThrust(const double &t)
{
	if(IsJet()==YSTRUE)
	{
		if(t<chThrMil || chHasAb!=YSTRUE)
		{
			SetAfterburner(YSFALSE);
			SetThrottle(t/chThrMil);
		}
		else
		{
			double thr;
			SetAfterburner(YSTRUE);
			thr=(t-chThrMil)/(chThrAb-chThrMil);
			thr=YsBound(thr,0.2,1.0);
			SetThrottle(thr);
		}
	}
	else
	{
		double thr1,thr2,thrm;
		thr1=0.0;
		thr2=1.0;
		while(thr2-thr1>0.01)  // Must converge within 8 iterations
		{
			thrm=(thr1+thr2)/2.0;
			if(t<GetThrust(thrm,staPosition.y(),staV,YSFALSE))
			{
				thr2=thrm;
			}
			else
			{
				thr1=thrm;
			}
		}
		SetThrottle(thrm);
	}
	return YSOK;
}

const double FsAirplaneProperty::GetThrottleForRequiredThrust(YSBOOL &ab,const double requiredThrust,YSBOOL canUseAfterburner)
{
	return GetThrottleForRequiredThrustForVelocity(ab,requiredThrust,staV,canUseAfterburner);
}

const double FsAirplaneProperty::GetThrottleForRequiredThrustForVelocity(YSBOOL &ab,const double requiredThrust,const double vel,YSBOOL canUseAfterburner)
{
	if(IsJet()==YSTRUE)
	{
		if(requiredThrust<chThrMil || YSTRUE!=chHasAb || YSTRUE!=canUseAfterburner)
		{
			ab=YSFALSE;
			return YsBound(requiredThrust/chThrMil,0.0,1.0);
		}
		else
		{
			ab=YSTRUE;

			double thr;
			thr=(requiredThrust-chThrMil)/(chThrAb-chThrMil);
			thr=YsBound(thr,0.2,1.0);
			return thr;
		}
	}
	else
	{
		double thr1,thr2,thrm;
		thr1=0.0;
		thr2=1.0;
		while(thr2-thr1>0.01)  // Must converge within 8 iterations
		{
			thrm=(thr1+thr2)/2.0;
			if(requiredThrust<GetThrust(thrm,staPosition.y(),vel,YSFALSE))
			{
				thr2=thrm;
			}
			else
			{
				thr1=thrm;
			}
		}

		ab=YSFALSE;
		return thrm;
	}
}

YSRESULT FsAirplaneProperty::SetThrustReverser(const double &a)
{
	ctlThrRev=YsBound(a,0.0,1.0);
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetElevator(const double &a)
{
	if(ctlPitchControllerSwitch==YSFALSE &&
	   ctlGControllerSwitch==YSFALSE)
	{
		ctlElevator=YsBound(a,-1.0,1.0);
	}
	return YSOK;
}


YSRESULT FsAirplaneProperty::SetElvTrim(const double &a)
{
	ctlElvTrim=YsBound(a,-1.0,1.0);
	return YSOK;
}


YSRESULT FsAirplaneProperty::SetAileron(const double &a)
{
	if(ctlBankControllerSwitch==YSFALSE)
	{
		double pAil;
		pAil=ctlAileron;

		ctlAileron=YsBound(a,-1.0,1.0);

		if((pAil> YsTolerance && ctlAileron<-YsTolerance) ||
		   (pAil<-YsTolerance && ctlAileron> YsTolerance))
		{
			ctlAileron=0.0;
		}
	}

	return YSOK;
}


YSRESULT FsAirplaneProperty::SetRudder(const double &a)
{
	ctlRudderUser=YsBound(a,-1.0,1.0);
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetDirectPitchControl(const double &a)
{
	ctlDirectPitch=a;
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetDirectRollControl(const double &a)
{
	ctlDirectRoll=a;
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetDirectYawControl(const double &a)
{
	ctlDirectYaw=a;
	return YSOK;
}

void FsAirplaneProperty::NeutralDirectAttitudeControl(void)
{
	NeutralDirectPitchControl();
	NeutralDirectRollControl();
	NeutralDirectYawControl();
}
void FsAirplaneProperty::NeutralDirectPitchControl(void)
{
	if(YsTolerance<chPostStallVPitch)
	{
		ctlDirectPitch=staVPitch/chPostStallVPitch;
	}
	else
	{
		ctlDirectPitch=0.0;
	}
}
void FsAirplaneProperty::NeutralDirectRollControl(void)
{
	if(YsTolerance<chPostStallVYaw)
	{
		ctlDirectYaw=staVYaw/chPostStallVYaw;
	}
	else
	{
		ctlDirectYaw=0;
	}
}
void FsAirplaneProperty::NeutralDirectYawControl(void)
{
	if(YsTolerance<chPostStallVRoll)
	{
		ctlDirectRoll=staVRoll/chPostStallVRoll;
	}
	else
	{
		ctlDirectRoll=0.0;
	}
}

YSRESULT FsAirplaneProperty::ToggleFlap(void)
{
	if(ctlFlap>0.5)
	{
		ctlFlap=0.0;
	}
	else
	{
		ctlFlap=1.0;
	}
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetFlap(const double &a)
{
	ctlFlap=YsBound(a,0.0,1.0);
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetThrustVector(const double &a)
{
	ctlThrVec=a;
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetVgw(const double &a)
{
	ctlVgw=YsBound(a,0.0,1.0);
	return YSOK;
}



void FsAirplaneProperty::SetAllVirtualButton(YSBOOL btn)
{
	SetFireWeaponButton(btn);
	SetFireGunButton(btn);
	SetFireAAMButton(btn);
	SetFireAGMButton(btn);
	SetFireRocketButton(btn);
	SetDropBombButton(btn);
	SetDispenseFlareButton(btn);
	SetCycleWeaponButton(btn);
	SetSmokeButton(btn);
}

void FsAirplaneProperty::SetFireWeaponButton(YSBOOL btn)
{
	pCtlFireWeaponButton=ctlFireWeaponButton;
	ctlFireWeaponButton=btn;
}

YSBOOL FsAirplaneProperty::GetFireWeaponButton(void)
{
	return ctlFireWeaponButton;
}

YSBOOL FsAirplaneProperty::IsFireWeaponButtonJustPressed(void)
{
	if(pCtlFireWeaponButton!=YSTRUE && ctlFireWeaponButton==YSTRUE)
	{
		pCtlFireWeaponButton=ctlFireWeaponButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetFireGunButton(YSBOOL btn)
{
	pCtlFireGunButton=ctlFireGunButton;
	ctlFireGunButton=btn;
}

YSBOOL FsAirplaneProperty::GetFireGunButton(void)
{
	return ctlFireGunButton;
}

YSBOOL FsAirplaneProperty::IsFireGunButtonJustPressed(void)
{
	if(pCtlFireGunButton!=YSTRUE && ctlFireGunButton==YSTRUE)
	{
		pCtlFireGunButton=ctlFireGunButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetFireAAMButton(YSBOOL btn)
{
	pCtlFireAAMButton=ctlFireAAMButton;
	ctlFireAAMButton=btn;
}

YSBOOL FsAirplaneProperty::GetFireAAMButton(void)
{
	return ctlFireAAMButton;
}

YSBOOL FsAirplaneProperty::IsFireAAMButtonJustPressed(void)
{
	if(pCtlFireAAMButton!=YSTRUE && ctlFireAAMButton==YSTRUE)
	{
		pCtlFireAAMButton=ctlFireAAMButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetFireAGMButton(YSBOOL btn)
{
	pCtlFireAGMButton=ctlFireAGMButton;
	ctlFireAGMButton=btn;
}

YSBOOL FsAirplaneProperty::GetFireAGMButton(void)
{
	return ctlFireAGMButton;
}

YSBOOL FsAirplaneProperty::IsFireAGMButtonJustPressed(void)
{
	if(pCtlFireAGMButton!=YSTRUE && ctlFireAGMButton==YSTRUE)
	{
		pCtlFireAGMButton=ctlFireAGMButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetFireRocketButton(YSBOOL btn)
{
	pCtlFireRocketButton=ctlFireRocketButton;
	ctlFireRocketButton=btn;
}

YSBOOL FsAirplaneProperty::GetFireRocketButton(void)
{
	return ctlFireRocketButton;
}

YSBOOL FsAirplaneProperty::IsFireRocketButtonJustPressed(void)
{
	if(pCtlFireRocketButton!=YSTRUE && ctlFireRocketButton==YSTRUE)
	{
		pCtlFireRocketButton=ctlFireRocketButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetDropBombButton(YSBOOL btn)
{
	pCtlDropBombButton=ctlDropBombButton;
	ctlDropBombButton=btn;
}

YSBOOL FsAirplaneProperty::GetDropBombButton(void)
{
	return ctlDropBombButton;
}

YSBOOL FsAirplaneProperty::IsDropBombButtonJustPressed(void)
{
	if(pCtlDropBombButton!=YSTRUE && ctlDropBombButton==YSTRUE)
	{
		pCtlDropBombButton=ctlDropBombButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetDispenseFlareButton(YSBOOL btn)
{
	pCtlDispenseFlareButton=ctlDispenseFlareButton;
	ctlDispenseFlareButton=btn;
}

YSBOOL FsAirplaneProperty::GetDispenseFlareButton(void)
{
	return ctlDispenseFlareButton;
}

YSBOOL FsAirplaneProperty::IsDispenseFlareButtonJustPressed(void)
{
	if(pCtlDispenseFlareButton!=YSTRUE && ctlDispenseFlareButton==YSTRUE)
	{
		pCtlDispenseFlareButton=ctlDispenseFlareButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetCycleWeaponButton(YSBOOL btn)
{
	pCtlCycleWeaponButton=ctlCycleWeaponButton;
	ctlCycleWeaponButton=btn;
}

YSBOOL FsAirplaneProperty::GetCycleWeaponButton(void)
{
	return ctlCycleWeaponButton;
}

YSBOOL FsAirplaneProperty::IsCycleWeaponButtonJustPressed(void)
{
	if(pCtlCycleWeaponButton!=YSTRUE && ctlCycleWeaponButton==YSTRUE)
	{
		pCtlCycleWeaponButton=ctlCycleWeaponButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsAirplaneProperty::SetSmokeButton(YSBOOL btn)
{
	pCtlSmokeButton=ctlSmokeButton;
	ctlSmokeButton=btn;
}

void FsAirplaneProperty::SetCycleSmokeSelectorButton(YSBOOL btn)
{
	pCtlCycleSmokeSelectorButton=ctlCycleSmokeSelectorButton;
	ctlCycleSmokeSelectorButton=btn;
}

void FsAirplaneProperty::SetSmokeSelector(unsigned char sel)
{
	ctlSmokeSelector=sel;
}

unsigned char FsAirplaneProperty::GetSmokeSelector(void) const
{
	return ctlSmokeSelector;
}

YSBOOL FsAirplaneProperty::GetSmokeButton(void)
{
	return ctlSmokeButton;
}

YSBOOL FsAirplaneProperty::IsSmokeButtonJustPressed(void)
{
	if(pCtlSmokeButton!=YSTRUE && ctlSmokeButton==YSTRUE)
	{
		pCtlSmokeButton=ctlSmokeButton;
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsAirplaneProperty::IsCycleSmokeSelectorButtonJustPressed(void)
{
	if(YSTRUE!=pCtlCycleSmokeSelectorButton && YSTRUE==ctlCycleSmokeSelectorButton)
	{
		pCtlCycleSmokeSelectorButton=ctlCycleSmokeSelectorButton;
		return YSTRUE;
	}
	return YSFALSE;
}


YSBOOL FsAirplaneProperty::IsFiringGun(void) const
{
	if(GetNumWeapon(FSWEAPON_GUN)>0)
	{
		if((staWoc==FSWEAPON_GUN && ctlFireWeaponButton==YSTRUE) || ctlFireGunButton==YSTRUE)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::IsOnGround(void) const
{
	if(staState==FSGROUND || staState==FSGROUNDSTATIC || staState==FSOVERRUN)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSRESULT FsAirplaneProperty::SetAutoVgw(YSBOOL a)
{
	ctlAutoVgw=a;
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetBombBayDoor(const double &a)
{
	ctlBombBayDoor=a;
	return YSOK;
}

YSRESULT FsAirplaneProperty::BouncePitchByTailStrike(void)
{
	if(TailStrikeAngleHasBeenComputed()==YSTRUE)
	{
		double pitch;
		pitch=GetTailStrikePitchAngle(0.98);

		if(staAttitude.p()>pitch)
		{
			staAttitude.SetP(pitch);
			staVPitch=-YsPi/18.0; // 10 degree/sec
		}
	}
	return YSOK;
}

YSRESULT FsAirplaneProperty::SetVectorMarker(YSBOOL vectMark)
{
	staVectorMarker=vectMark;
	return YSOK;
}

YSBOOL FsAirplaneProperty::GetVectorMarker(void) const
{
	return staVectorMarker;
}

void FsAirplaneProperty::TurnOffController(void)
{
	ctlSpeedControllerSwitch=YSFALSE;
	ctlBankControllerSwitch=YSFALSE;
	ctlPitchControllerSwitch=YSFALSE;
	ctlGControllerSwitch=YSFALSE;
	ctlAOAControllerSwitch=YSFALSE;     // 2003/01/10
}

YSRESULT FsAirplaneProperty::SpeedController(const double &spd)
{
	if(ctlSpeedControllerSwitch!=YSTRUE)
	{
		ctlIntegralSpdErr=0.0;
		ctlPrevSpdErr=0.0;
	}
	ctlSpeedControllerSwitch=YSTRUE;
	ctlSpeedControllerCanUseAfterburner=YSTRUE;
	ctlSpeedControllerThrottleCap=1.0;
	ctlSpeedControllerInput=spd;
	return YSOK;
}

YSRESULT FsAirplaneProperty::SpeedControllerDontUseAfterburner(const double &spd,const double throttleCap)
{
	if(ctlSpeedControllerSwitch!=YSTRUE)
	{
		ctlIntegralSpdErr=0.0;
		ctlPrevSpdErr=0.0;
	}
	ctlSpeedControllerSwitch=YSTRUE;
	ctlSpeedControllerCanUseAfterburner=YSFALSE;
	ctlSpeedControllerThrottleCap=throttleCap;
	ctlSpeedControllerInput=spd;
	return YSOK;
}

void FsAirplaneProperty::TurnOffSpeedController(void)
{
	ctlSpeedControllerSwitch=YSFALSE;
}

YSRESULT FsAirplaneProperty::BankController(const double &bnk,const double &rollRateLimit)
{
	YsAtt3 refAtt;
	refAtt=staAttitude;
	refAtt.SetB(bnk);

	ctlBankControllerSwitch=YSTRUE;
	ctlBankControllerInput=refAtt.GetUpVector();
	ctlBankControllerRollRateLimit=rollRateLimit;
	return YSOK;
}

YSRESULT FsAirplaneProperty::BankController(const YsVec3 &uv,const double &rollRateLimit)
{
	ctlBankControllerSwitch=YSTRUE;
	ctlBankControllerInput=uv;
	ctlBankControllerRollRateLimit=rollRateLimit;
	return YSOK;
}

void FsAirplaneProperty::TurnOffBankController(void)
{
	ctlBankControllerSwitch=YSFALSE;
}

YSRESULT FsAirplaneProperty::PitchController(const double &bnk)
{
	TurnOffGController();    // Exclusive
	TurnOffAOAController();
	ctlPitchControllerSwitch=YSTRUE;
	ctlPitchControllerInput=bnk;
	return YSOK;
}

void FsAirplaneProperty::TurnOffPitchController(void)
{
	ctlPitchControllerSwitch=YSFALSE;
}

YSRESULT FsAirplaneProperty::GController(const double &gInput)
{
	TurnOffPitchController();  // Exclusive
	TurnOffAOAController();
	ctlGControllerSwitch=YSTRUE;
	ctlGControllerInput=gInput;
	ctlGControllerMinAOALimit=-YsPi/2.0;
	ctlGControllerMaxAOALimit= YsPi/2.0;
	return YSOK;
}

YSRESULT FsAirplaneProperty::GControllerSmooth(const double &g,const double dG,const double dt)
{
	if(ctlGControllerSmoother<g)
	{
		ctlGControllerSmoother+=dG*dt;
		if(g<ctlGControllerSmoother)
		{
			ctlGControllerSmoother=g;
		}
		return GController(ctlGControllerSmoother);
	}
	else
	{
		ctlGControllerSmoother-=dG*dt;
		if(ctlGControllerSmoother<g)
		{
			ctlGControllerSmoother=g;
		}
		return GController(ctlGControllerSmoother);
	}
}

void FsAirplaneProperty::CaptureGControllerSmoother(void)
{
	ctlGControllerSmoother=GetG();
}

void FsAirplaneProperty::SetGControllerAOALimit(const double &minAOA,const double &maxAOA)
{
	ctlGControllerMinAOALimit=minAOA;
	ctlGControllerMaxAOALimit=maxAOA;
}

const double FsAirplaneProperty::ComputeAOAForRequiredG(const double &gInput)
{
	return ComputeAOAForRequiredG(gInput,staV);
}

const double FsAirplaneProperty::ComputeAOAForRequiredG(const double &gInput,const double v) const
{
	double L0,L5;
	double Lreq,Areq;

	L0=GetLift(0.0,v);
	L5=GetLift(YsDegToRad(5.0),v);

	Lreq=gInput*(GetTotalWeight()*FsGravityConst);  // Required lift
	if(YsTolerance<L5-L0)
	{
		Areq=(Lreq-L0)/((L5-L0)/YsDegToRad(5.0));
		return Areq;
	}
	else
	{
		return 0.0;
	}
}

const double FsAirplaneProperty::ComputeElevatorTrimForAOA(const double aoa) const
{
	return aoa/chMaxInputAOA;
}

void FsAirplaneProperty::TurnOffGController(void)
{
	ctlGControllerSwitch=YSFALSE;
}

void FsAirplaneProperty::TurnOffAOAController(void)
{
	ctlAOAControllerSwitch=YSFALSE;
}

YSRESULT FsAirplaneProperty::AOAController(const double &aoa,const double &gLimit)
{
	TurnOffPitchController();
	TurnOffGController();
	ctlAOAControllerSwitch=YSTRUE;
	ctlAOAControllerInput=aoa;
	ctlAOAControllerGLimit=gLimit;
	return YSOK;
}


YSRESULT FsAirplaneProperty::ConfigElevatorByAoa(const double &aoa)
{
	return SetElevator(aoa/chMaxInputAOA-ctlElvTrim);
}

void FsAirplaneProperty::SetPosition(const YsVec3 &p)
{
	staPosition=p;
	RemakeMatrix();
}

void FsAirplaneProperty::SetAttitude(const YsAtt3 &a)
{
	staAttitude=a;
	RemakeMatrix();
}

void FsAirplaneProperty::SetPositionAndAttitude(const YsVec3 &p,const YsAtt3 &a)
{
	staPosition=p;
	staAttitude=a;
	RemakeMatrix();
}

void FsAirplaneProperty::SetVelocity(const YsVec3 &vel)
{
	staVelocity=vel;
	staV=vel.GetLength();
}

void FsAirplaneProperty::SetFieldElevation(const double &elv)
{
	staGndElevation=elv;
}

void FsAirplaneProperty::SetFieldNormal(const YsVec3 &nom)
{
	staGndNormal=nom;
}

void FsAirplaneProperty::SetBaseElevation(const double &elv)
{
	staBaseElevation=elv;
}

const double &FsAirplaneProperty::GetGroundElevation(void) const
{
	return staGndElevation;
}

double FsAirplaneProperty::GetAGL(void) const
{
	return staPosition.y()-staGndElevation;
}

const YsVec3 &FsAirplaneProperty::GetGroundNormal(void) const
{
	return staGndNormal;
}

const double FsAirplaneProperty::GetTrueAltitude(void) const
{
	return staPosition.y()+staBaseElevation;
}

const double FsAirplaneProperty::GetIndicatedTrueAltitude(void) const
{
	if(YSTRUE!=staAltimeterInop)
	{
		return GetTrueAltitude();
	}
	else
	{
		return staAltimeterStuckAltitude;
	}
}

const double &FsAirplaneProperty::GetThrottle(void) const
{
	return staThrottle;
}

const double FsAirplaneProperty::GetRealPropRPM(YSSIZE_T engineIdx) const
{
	if(YSTRUE==chRealProp.IsInRange(engineIdx))
	{
		return chRealProp[engineIdx].radianPerSec*60.0/(YsPi*2.0);
	}
	return 0.0;
}

YSRESULT FsAirplaneProperty::GetRPMRangeForSoundEffect(double &min,double &max,int engineIdx) const
{
	if(YSTRUE==chRealProp.IsInRange(engineIdx))
	{
		chRealProp[engineIdx].GetRPMRangeForSoundEffect(min,max);
		return YSOK;
	}
	min=0.0;
	max=0.0;
	return YSERR;
}

YSBOOL FsAirplaneProperty::HasFixedSpeedPropeller(void) const
{
	for(auto &prop : chRealProp)
	{
		for(auto &blade : prop.bladeArray)
		{
			if(YsTolerance<fabs(blade.maxPitch-blade.minPitch))
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

const double &FsAirplaneProperty::GetThrustReverser(void) const
{
	return staThrRev;
}

const double &FsAirplaneProperty::GetElevator(void) const
{
	return ctlElevator;
}

const double &FsAirplaneProperty::GetElvTrim(void) const
{
	return ctlElvTrim;
}

const double &FsAirplaneProperty::GetAileron(void) const
{
	return ctlAileron;
}

const double FsAirplaneProperty::GetCurrentThrust(void) const
{
	return staThrust;
}

const double &FsAirplaneProperty::GetThrustVector(void) const
{
	return staThrVec;
}

const double FsAirplaneProperty::GetRudder(void) const
{
	return YsBound(ctlRudderUser+ctlRudderControl,-1.0,1.0);
}

const double &FsAirplaneProperty::GetRudderUserInput(void) const
{
	return ctlRudderUser;
}

const double &FsAirplaneProperty::GetRudderAutoCoordinator(void) const
{
	return ctlRudderControl;
}

const double FsAirplaneProperty::GetVelocity(void) const
{
	return staVelocity.GetLength();
}

const double FsAirplaneProperty::GetHorizontalVelocity(void) const
{
	return staVelocity.GetLengthXZ();
}

void FsAirplaneProperty::GetVelocity(YsVec3 &vel) const
{
	vel=staVelocity;
}

void FsAirplaneProperty::GetRotationSpeed(YsAtt3 &rot) const
{
	rot.Set(staVYaw,staVPitch,staVRoll);
}

void FsAirplaneProperty::GetAcceleration(YsVec3 &acc) const
{
	acc=(staTotalAerodynamicForce+staTotalGravityForce)/GetTotalWeight();
}

YSBOOL FsAirplaneProperty::LandingGearIsLocked(void) const
{
	return ((staGear>=1.0-YsTolerance) ? YSTRUE : YSFALSE);
}

YSBOOL FsAirplaneProperty::HasRetractableLandingGear(void) const
{
	return chGearRetractable;
}

const double &FsAirplaneProperty::GetLandingGear(void) const
{
	return staGear;
}

const double &FsAirplaneProperty::GetBombBayDoor(void) const
{
	return staBombBayDoor;
}

YSBOOL FsAirplaneProperty::GetBombIsInBombBay(void) const
{
	return chBombInBombBay;
}

const double &FsAirplaneProperty::GetBombBayRadarCrossSection(void) const
{
	return chBombBayRcs;
}

YSBOOL FsAirplaneProperty::GetBrake(void) const
{
	return ((staBrake>=0.5) ? YSTRUE : YSFALSE);
}

const double FsAirplaneProperty::GetBrakeMovingRate(void) const
{
	return 1.0/0.5;
}

const double FsAirplaneProperty::CalculateForceByBrake(const double brakeControl) const
{
	return chBrakeConst*brakeControl;
}

const double &FsAirplaneProperty::GetFlap(void) const
{
	return staFlap;
}

const double &FsAirplaneProperty::GetSpoiler(void) const
{
	return staSpoiler;
}

YSBOOL FsAirplaneProperty::GetHasSpoiler(void)const
{
	return chHasSpoiler;
}

const double &FsAirplaneProperty::GetControlVgw(void) const
{
	return ctlVgw;
}

YSBOOL FsAirplaneProperty::GetAfterBurner(void) const
{
	if(chHasAb==YSTRUE && staAb==YSTRUE)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::GetHasAfterburner(void) const
{
	return chHasAb;
}

int FsAirplaneProperty::GetNumFlapPosition(void) const
{
	return (int)chFlapPosition.GetN();
}

const double &FsAirplaneProperty::GetFlapPosition(int flpPosId) const
{
	if(YSTRUE==chFlapPosition.IsInRange(flpPosId))
	{
		return chFlapPosition[flpPosId];
	}
	return chFlapPosition[0];
}

YSBOOL FsAirplaneProperty::GetHasThrustVectoring(void) const
{
	if(chHasThrustVector==YSTRUE && chThrVec0!=chThrVec1)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void FsAirplaneProperty::GetThrustDirection(YsVec3 &dir) const
{
	if(chHasThrustVector!=YSTRUE)
	{
		dir.Set(0.0,0.0,1.0);
	}
	else
	{
		dir=chThrVec0*(1.0-staThrVec)+chThrVec1*staThrVec;
		dir.Normalize();
	}
}

YSBOOL FsAirplaneProperty::GetHasThrustReverser(void) const
{
	if(chThrustReverser>YsTolerance)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::GetHasVariableGeometryNose(void) const
{
	return chHasVGN;
}

const double FsAirplaneProperty::GetSafeGroundSpeed(void) const
{
	return 3.0;  // Roughly 10km/h
}

int FsAirplaneProperty::GetNumTurret(void) const
{
	return (int)chTurret.GetN();
}

int FsAirplaneProperty::GetMaxNumTurretBullet(int turretId) const
{
	if(chTurret.IsInRange(turretId)==YSTRUE)
	{
		return chTurret[turretId].maxNumGunBullet;
	}
	return 0;
}

YSBOOL FsAirplaneProperty::GetHasAntiAirTurret(void) const
{
	return chHasAntiAirTurret;
}

YSBOOL FsAirplaneProperty::GetHasAntiGroundTurret(void) const
{
	return chHasAntiGndTurret;
}

YSBOOL FsAirplaneProperty::GetHasPilotControlledTurret(void) const
{
	return chHasPilotControlledTurret;
}

YSBOOL FsAirplaneProperty::GetHasGunnerControlledTurret(void) const
{
	return chHasGunnerControlledTurret;
}

YSRESULT FsAirplaneProperty::GetFirstPilotControlledTurretDirection(YsVec3 &dir) const
{
	int i;
	for(i=0; i<chTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			YsAtt3 att;

			att.Set(staTurret[i].h,staTurret[i].p,0.0);
			dir=att.GetForwardVector();

			chTurret[i].att.Mul(dir,dir);

			staMatrix.Mul(dir,dir,0.0);
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsAirplaneProperty::GetFirstPilotControlledTurretPosition(YsVec3 &pos) const
{
	int i;
	for(i=0; i<chTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			staMatrix.Mul(pos,chTurret[i].cen,1.0);
			return YSOK;
		}
	}
	return YSERR;
}

const double &FsAirplaneProperty::GetEstimatedCruiseSpeed(void) const
{
	return refSpdCruise;
}

const double &FsAirplaneProperty::GetEstimatedLandingSpeed(void) const
{
	return refSpdLanding;
}

const double &FsAirplaneProperty::GetEstimatedRequiredRunwayLength(void) const
{
	return refLNGRunway;
}

double FsAirplaneProperty::MoveDevice(const double &toBe,double rightNow,const double &howFast,const double &dt)
{
	double motion;
	motion=howFast*dt; // (1.0/howFast)*dt;

	if(toBe<rightNow)
	{
		rightNow=rightNow-motion;
		if(rightNow<toBe)
		{
			rightNow=toBe;
		}
	}
	else if(toBe>rightNow)
	{
		rightNow=rightNow+motion;
		if(rightNow>toBe)
		{
			rightNow=toBe;
		}
	}
	return rightNow;
}

YSBOOL FsAirplaneProperty::FireGunIfVirtualTriggerIsPressed(
    const double &ctime,const double &dt,FsSimulation *sim,FsWeaponHolder &bul,FsExistence *owner)
{
	int i;
	YSBOOL pilotFiring,fireGunButton;
	pilotFiring=YSFALSE;


	if((staWoc==FSWEAPON_GUN && ctlFireWeaponButton==YSTRUE) || ctlFireGunButton==YSTRUE)
	{
		fireGunButton=YSTRUE;
	}
	else
	{
		fireGunButton=YSFALSE;
	}

	// turretState FSTURRETSTATE_FIRING is set in ApplyControl

	if(IsActive()==YSTRUE)
	{
		for(i=0; i<chTurret.GetN(); i++)
		{
			if(staTurret[i].numBullet>0 && (staTurret[i].turretState&FSTURRETSTATE_FIRING)!=0)
			{
				if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
				{
					pilotFiring=YSTRUE;
				}
			}

			staTurret[i].FireWeapon(chTurret[i],ctime,dt,staVelocity,staMatrix,sim,bul,owner);
		}

		if(fireGunButton==YSTRUE)
		{
			if(staGunBullet>0)
			{
				staGunTimer-=dt;
				while(staGunTimer<=0.0)
				{
					YsVec3 gun,dir;
					YsAtt3 att;

					for(i=0; i<chNumGun && staGunBullet>0; i++)
					{
						staAttitude.Mul(dir,chGunDirection[i]);
						att.SetForwardVector(dir);
						att.SetB(0.0);

						gun=staMatrix*chGunPosition[i];
						bul.Fire(ctime,gun,att,chBulInitSpeed,chBulRange,chGunPower,owner,YSTRUE,YSTRUE);
						staGunBullet--;
					}

					staGunTimer+=chGunInterval;
				}
				pilotFiring=YSTRUE;
			}
			else
			{
				staGunTimer=0.0;
			}
		}
	}

	return pilotFiring;
}

YSBOOL FsAirplaneProperty::ProcessVirtualButtonPress(
    YSBOOL &blockedByBombBay,FSWEAPONTYPE &firedWeapon,
    FsSimulation *sim,const double &ctime,FsWeaponHolder &bul,FsExistence *owner)
{
	blockedByBombBay=YSFALSE;
	if(IsFireWeaponButtonJustPressed()==YSTRUE)
	{
		staVirtualButtonQueue.Append(VBT_FIREWEAPON);
	}
	if(IsFireAAMButtonJustPressed()==YSTRUE)
	{
		staVirtualButtonQueue.Append(VBT_FIREAAM);
	}
	if(IsFireAGMButtonJustPressed()==YSTRUE)
	{
		staVirtualButtonQueue.Append(VBT_FIREAGM);
	}
	if(IsFireRocketButtonJustPressed()==YSTRUE)
	{
		staVirtualButtonQueue.Append(VBT_FIREROCKET);
	}
	if(YSTRUE==IsCycleSmokeSelectorButtonJustPressed())
	{
		staVirtualButtonQueue.Append(VBT_CYCLESMOKESELECTOR);
	}
	if(IsDropBombButtonJustPressed()==YSTRUE)
	{
		staVirtualButtonQueue.Append(VBT_DROPBOMB);
	}
	if(IsDispenseFlareButtonJustPressed()==YSTRUE)
	{
		staVirtualButtonQueue.Append(VBT_DISPENSEFLARE);
	}
	if(IsCycleWeaponButtonJustPressed()==YSTRUE)
	{
		staVirtualButtonQueue.Append(VBT_CYCLEWEAPON);
	}
	return RunVirtualButtonQueue(blockedByBombBay,firedWeapon,sim,ctime,bul,owner);
}

YSBOOL FsAirplaneProperty::RunVirtualButtonQueue(
    YSBOOL &blockedByBombBay,FSWEAPONTYPE &firedWeapon,
    FsSimulation *sim,const double &ctime,FsWeaponHolder &bul,FsExistence *owner)
{
	while(0<staVirtualButtonQueue.GetN())
	{
		auto btn=staVirtualButtonQueue[0];
		staVirtualButtonQueue.Delete(0);
		switch(btn)
		{
		case VBT_FIREWEAPON:
			firedWeapon=staWoc;
			return FireSelectedWeapon(blockedByBombBay,sim,ctime,bul,owner);
		case VBT_FIREAAM:
			if(0<GetNumWeapon(FSWEAPON_AIM9X))
			{
				firedWeapon=FSWEAPON_AIM9X;
				return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,firedWeapon);
			}
			else if(0<GetNumWeapon(FSWEAPON_AIM9))
			{
				firedWeapon=FSWEAPON_AIM9;
				return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,firedWeapon);
			}
			else
			{
				firedWeapon=FSWEAPON_AIM120;
				return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,firedWeapon);
			}
			break;
		case VBT_FIREAGM:
			firedWeapon=FSWEAPON_AGM65;
			return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,FSWEAPON_AGM65);
		case VBT_FIREROCKET:
			firedWeapon=FSWEAPON_ROCKET;
			return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,FSWEAPON_ROCKET);
		case VBT_DROPBOMB:
			if(GetNumWeapon(FSWEAPON_BOMB)>0)
			{
				firedWeapon=FSWEAPON_BOMB;
				return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,FSWEAPON_BOMB);
			}
			else if(GetNumWeapon(FSWEAPON_BOMB250)>0)
			{
				firedWeapon=FSWEAPON_BOMB250;
				return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,FSWEAPON_BOMB250);
			}
			else if(GetNumWeapon(FSWEAPON_BOMB500HD)>0)
			{
				firedWeapon=FSWEAPON_BOMB500HD;
				return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,FSWEAPON_BOMB500HD);
			}
			break;
		case VBT_DISPENSEFLARE:
			firedWeapon=FSWEAPON_FLARE;
			return FireWeapon(blockedByBombBay,sim,ctime,bul,owner,FSWEAPON_FLARE);
		case VBT_CYCLEWEAPON:
			CycleWeaponOfChoice();
			return YSFALSE;
		case VBT_CYCLESMOKESELECTOR:
			CycleSmokeSelector();
			return YSFALSE;
		}
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::FireSelectedWeapon(
    YSBOOL &blockedByBombBay,
    FsSimulation *sim,const double &ct,class FsWeaponHolder &bul,class FsExistence *own)
{
	blockedByBombBay=YSFALSE;
	return FireWeapon(blockedByBombBay,sim,ct,bul,own,staWoc);
}

YSBOOL FsAirplaneProperty::FireWeapon(
    YSBOOL &blockedByBombBay,FsSimulation *sim,const double &ctime,class FsWeaponHolder &bul,FsExistence *owner,FSWEAPONTYPE wpnType)
{
	YSBOOL fired;
	fired=YSFALSE;

	blockedByBombBay=YSFALSE;

	int slot;
	YsVec3 missilePos,flareVel=YsOrigin();
	YsAtt3 missileAtt;
	slot=-1;


	// Bomb bay door check
	if((wpnType==FSWEAPON_BOMB || wpnType==FSWEAPON_BOMB250 || wpnType==FSWEAPON_BOMB500HD) && chBombInBombBay==YSTRUE && staBombBayDoor<0.95)
	{
		blockedByBombBay=YSTRUE;
		return YSFALSE;
	}


	if(wpnType==FSWEAPON_AIM9 || wpnType==FSWEAPON_AIM9X || wpnType==FSWEAPON_AGM65 ||
	   wpnType==FSWEAPON_BOMB || wpnType==FSWEAPON_ROCKET ||
	   wpnType==FSWEAPON_AIM120 || wpnType==FSWEAPON_BOMB250 || wpnType==FSWEAPON_BOMB500HD || wpnType==FSWEAPON_FUELTANK)
	{
		int i;

		// 2010/08/13 Prioritize external slot
		// First search for a visible weapon.
		for(i=(int)staWeaponSlot.GetN()-1; i>=0; i--)
		{
			if(YSTRUE==chWeaponSlot[i].isExternal && staWeaponSlot[i].nLoaded>0 && staWeaponSlot[i].wpnType==wpnType)
			{
				if(slot<0 || staWeaponSlot[slot].nLoaded<=staWeaponSlot[i].nLoaded)
				{
					slot=i;
				}
			}
		}
		if(0>slot)
		{
			for(i=(int)staWeaponSlot.GetN()-1; i>=0; i--)
			{
				if(staWeaponSlot[i].nLoaded>0 && staWeaponSlot[i].wpnType==wpnType)
				{
					if(slot<0 || staWeaponSlot[slot].nLoaded<=staWeaponSlot[i].nLoaded)
					{
						slot=i;
					}
				}
			}
		}


		if(slot>=0)
		{
			missilePos=chWeaponSlot[slot].pos;
		}
		else
		{
			missilePos=YsOrigin();
		}
	}
	else if(wpnType==FSWEAPON_FLARE)
	{

		for(int i=(int)staWeaponSlot.GetN()-1; i>=0; i--)
		{
			if(staWeaponSlot[i].nLoaded>0 && staWeaponSlot[i].wpnType==wpnType)
			{
				if(slot<0 || staWeaponSlot[slot].nLoaded<=staWeaponSlot[i].nLoaded)
				{
					slot=i;
				}
			}
		}

		if(slot>=0)
		{
			missilePos=chWeaponSlot[slot].pos;
		}
		else if(0<chNumFlareDispenser)
		{
			missilePos=chFlareDispenser[staFlare%chNumFlareDispenser];
			flareVel=chFlareDispensingVel[staFlare%chNumFlareDispenser];
		}
		else
		{
			missilePos=chFlareDispenser[0];
			flareVel=YsOrigin();
		}
	}
	else
	{
		const int nTotalLoad=GetNumWeapon(FSWEAPON_AIM9)+GetNumWeapon(FSWEAPON_AGM65)+GetNumWeapon(FSWEAPON_BOMB)+GetNumWeapon(FSWEAPON_ROCKET);
		missilePos.Set(4.0*((nTotalLoad%2)!=0 ? 1.0 : -1.0),-1.0,0.0);
	}

	missilePos=staMatrix*missilePos;
	if(staMovingBackward==YSTRUE || staV<=chManSpeed1)
	{
		missileAtt=staAttitude;
	}
	else if(staV>chManSpeed2)
	{
		missileAtt.SetForwardVector(staVelocity);
		missileAtt.SetB(staAttitude.b());
	}
	else
	{
		double t;
		YsVec3 v1,v2,ev;
		v1=staAttitude.GetForwardVector();
		v2=staVelocity;
		v2.Normalize();
		t=(staV-chManSpeed1)/(chManSpeed2-chManSpeed1);
		ev=v1*(1.0-t)+v2*t;
		missileAtt.SetForwardVector(ev);
		missileAtt.SetB(staAttitude.b());
	}

	if(0<GetNumWeapon(wpnType))
	{
		switch(wpnType)
		{
		default:
			break;
		case FSWEAPON_AIM9:
			{
				fired=YSTRUE;
				staRecentlyFiredMissileId=
				bul.Fire(ctime,
				         wpnType,
				         missilePos,
				         missileAtt, //staAttitude,
				         staV,
				         340.0*3.0,
				         GetAAMRange(wpnType),
				         YsDegToRad(90.0),
				         GetAAMRadarAngle(),
				         12,
				         owner,
				         staAirTargetKey, // <- Locked On Target
				         YSTRUE,YSTRUE);
			}
			break;
		case FSWEAPON_AIM9X:
			{
				fired=YSTRUE;
				staRecentlyFiredMissileId=
				bul.Fire(ctime,
				         wpnType,
				         missilePos,
				         missileAtt, //staAttitude,
				         staV,
				         340.0*3.0,
				         GetAAMRange(wpnType),
				         YsDegToRad(80.0),
				         GetAAMRadarAngle(),
				         12,
				         owner,
				         staAirTargetKey, // <- Locked On Target
				         YSTRUE,YSTRUE);
			}
			break;
		case FSWEAPON_AIM120:
			{
				fired=YSTRUE;
				staRecentlyFiredMissileId=
					bul.Fire(ctime,
					         wpnType,
					         missilePos,
					         missileAtt, //staAttitude,
					         staV,
					         340.0*4.0,
					         30000.0,  // 30km
					         YsDegToRad(55.0),
					         GetAAMRadarAngle(),
					         12,
					         owner,
					         staAirTargetKey, // <- Locked On Target
					         YSTRUE,YSTRUE);
			}
			break;
		case FSWEAPON_AGM65:
			{
				fired=YSTRUE;
				staRecentlyFiredMissileId=
					bul.Fire(ctime,
					         wpnType,
					         missilePos,
					         missileAtt, //staAttitude,
					         staV,
					         340.0,
					         GetAGMRange(),
					         YsDegToRad(90.0),
					         GetAGMRadarAngle(),
					         GetAGMDestructivePower(),
					         owner,
					         staGroundTargetKey, // <- Locked On Target
					         YSTRUE,
					         YSTRUE);
			}
			break;
		case FSWEAPON_ROCKET:
			{
				fired=YSTRUE;
				bul.Fire(ctime,
				         wpnType,
				         missilePos,
				         missileAtt, //staAttitude,
				         staV,
				         GetRocketSpeed(),
				         GetRocketRange(),
				         YsDegToRad(90.0),
				         0.0,
				         10,
				         owner,
				         YSNULLHASHKEY,              // <- Locked On Target
				         YSTRUE,
				         YSTRUE);
			}
			break;
		case FSWEAPON_BOMB:
			{
				fired=YSTRUE;
				bul.Bomb(ctime,
				         wpnType,
				         missilePos,
				         staAttitude,
				         staVelocity,
				         340.0,
				         50,
				         owner,
				         YSTRUE,
				         YSTRUE);
			}
			break;
		case FSWEAPON_BOMB250:
			{
				fired=YSTRUE;
				bul.Bomb(ctime,
				         wpnType,
				         missilePos,
				         staAttitude,
				         staVelocity,
				         340.0,
				         35,
				         owner,
				         YSTRUE,
				         YSTRUE);
			}
			break;
		case FSWEAPON_BOMB500HD:
			{
				fired=YSTRUE;
				bul.Bomb(ctime,
				         wpnType,
				         missilePos,
				         staAttitude,
				         staVelocity,
				         340.0,
				         35,
				         owner,
				         YSTRUE,
				         YSTRUE);
			}
			break;
		case FSWEAPON_FUELTANK:
			{
				int destructive=1;
				if(0<=slot)
				{
					destructive+=(int)(staWeaponSlot[slot].fuelLoaded/80.0);
				}
				fired=YSTRUE;
				bul.Bomb(ctime,
				         wpnType,
				         missilePos,
				         staAttitude,
				         staVelocity,
				         340.0,
				         destructive,
				         owner,
				         YSTRUE,
				         YSTRUE);
			}
			break;
		case FSWEAPON_FLARE:
			{
				staAttitude.Mul(flareVel,flareVel);
				if(0>slot && 0<staFlare)
				{
					staFlare--;
				}
				fired=YSTRUE;
				bul.DispenseFlare(ctime,missilePos,staVelocity+flareVel,120.0,1000.0,owner,YSTRUE,YSTRUE);
			}
			break;
		}
	}

	if(slot>=0 && staWeaponSlot[slot].nLoaded>0)
	{
		staWeaponSlot[slot].nLoaded--;
	}

	return fired;
}

int FsAirplaneProperty::GetRecentlyFiredMissileId(void) const
{
	return staRecentlyFiredMissileId;
}

void FsAirplaneProperty::FireMissileByRecord(FSWEAPONTYPE wpnType)
{
	int slot;
	slot=-1;
	for(int i=(int)staWeaponSlot.GetN()-1; i>=0; i--)
	{
		if(staWeaponSlot[i].nLoaded>0 && staWeaponSlot[i].wpnType==wpnType)
		{
			if(slot<0 || staWeaponSlot[slot].nLoaded<=staWeaponSlot[i].nLoaded)
			{
				slot=i;
			}
		}
	}
	if(slot>=0 && staWeaponSlot[slot].nLoaded>0)
	{
		staWeaponSlot[slot].nLoaded--;
	}
}

YSRESULT FsAirplaneProperty::ToggleRadarRange(int dir)
{
	const int nChoice=6;
	const double rangeChoice[nChoice]={0.0,5.0,10.0,25.0,50.0,100.0};

	if(0<=dir)
	{
		double nextRadarRange=0.0;
		for(int i=0; i<nChoice; ++i)
		{
			if(YSTRUE==YsEqual(staRadarRange,rangeChoice[i]))
			{
				nextRadarRange=rangeChoice[(i+1)%nChoice];
				break;
			}
		}
		staRadarRange=nextRadarRange;
	}
	else
	{
		double nextRadarRange=rangeChoice[nChoice-1];
		for(int i=0; i<nChoice; ++i)
		{
			if(YSTRUE==YsEqual(staRadarRange,rangeChoice[i]))
			{
				nextRadarRange=rangeChoice[(i+nChoice-1)%nChoice];
				break;
			}
		}
		staRadarRange=nextRadarRange;
	}

	return YSOK;
}

YSRESULT FsAirplaneProperty::IncreaseRadarRange(void)
{
	return ToggleRadarRange(1);
}

YSRESULT FsAirplaneProperty::ReduceRadarRange(void)
{
	return ToggleRadarRange(-1);
}

YSRESULT FsAirplaneProperty::TurnOnRadar(void)
{
	staRadarRange=5.0;
	return YSOK;
}

YSRESULT FsAirplaneProperty::TurnOffRadar(void)
{
	staRadarRange=0.0;
	return YSOK;
}

YSBOOL FsAirplaneProperty::GetDamage(YSBOOL &killed,int dmg,FSDIEDOF diedOf)
{
	killed=YSFALSE;
	if(IsActive()==YSTRUE)
	{
		staDamageTolerance-=dmg;
		if(staDamageTolerance<=0)
		{
			switch((rand()%700)/100)
			{
			case 0:
				SetState(FSDEAD,diedOf);
				staDamageTolerance=0;
				break;
			case 1:
			case 2:
			case 3:
				SetState(FSDEADSPIN,diedOf);
				staDamageTolerance=1;
				break;
			case 4:
			case 5:
			case 6:
				SetState(FSDEADFLATSPIN,diedOf);
				staDamageTolerance=1;
				break;
			}
			killed=YSTRUE;
		}
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsAirplaneProperty::ToggleLight(void)
{
	YsFlip(staBeacon);
	staNavLight=staBeacon;
	staStrobe=staBeacon;
	staLandingLight=staBeacon;
	return YSOK;
}

YSRESULT FsAirplaneProperty::ToggleNavLight(void)
{
	YsFlip(staNavLight);
	return YSOK;
}

YSRESULT FsAirplaneProperty::ToggleBeacon(void)
{
	YsFlip(staBeacon);
	return YSOK;
}

YSRESULT FsAirplaneProperty::ToggleStrobe(void)
{
	YsFlip(staStrobe);
	return YSOK;
}

YSRESULT FsAirplaneProperty::ToggleLandingLight(void)
{
	YsFlip(staLandingLight);
	return YSOK;
}

int FsAirplaneProperty::LoadWeaponToSlot(FSWEAPONTYPE wpnType,int n)
{
	if(FSWEAPON_FLARE_INTERNAL==wpnType)
	{
		staFlare=0;
	}
	else
	{
		int i;
		for(i=0; i<staWeaponSlot.GetN(); i++)
		{
			if(staWeaponSlot[i].wpnType==wpnType)
			{
				staWeaponSlot[i].Initialize();
			}
		}
	}
	return AddWeaponToSlot(wpnType,n);
}

int FsAirplaneProperty::AddWeaponToSlot(const FSWEAPONTYPE wpnType,const int n)  // for FSWEAPON_FUELTANK n is taken as kilogram.
{
	if(0<=wpnType && wpnType<FSWEAPON_NUMWEAPONTYPE)
	{
		int i,toLoad=n,nLoaded=0;

		for(i=0; i<chWeaponSlot.GetN() && 0<toLoad; i++)
		{
			if(0<chWeaponSlot[i].nLoad[wpnType])
			{
				if(FSWEAPON_FUELTANK==wpnType)
				{
					if(0==staWeaponSlot[i].nLoaded)
					{
						const double actualLoad=(double)YsSmaller <double> (toLoad,chWeaponSlot[i].nSubLoad[wpnType]);
						staWeaponSlot[i].wpnType=FSWEAPON_FUELTANK;
						staWeaponSlot[i].nLoaded=1;
						staWeaponSlot[i].fuelLoaded=actualLoad;
						nLoaded=1;
						toLoad=0;
						break;
					}
				}
				else
				{
					if(FSWEAPON_GUN==staWeaponSlot[i].wpnType)
					{
						staWeaponSlot[i].Initialize();
					}

					if(0==staWeaponSlot[i].nLoaded || wpnType==staWeaponSlot[i].wpnType)
					{
						const int nCanLoad=chWeaponSlot[i].nLoad[wpnType]-staWeaponSlot[i].nLoaded;
						const int load=YsSmaller(toLoad,nCanLoad);

						staWeaponSlot[i].wpnType=wpnType;
						staWeaponSlot[i].nLoaded+=load;

						if(wpnType==FSWEAPON_FLARE)
						{
							staWeaponSlot[i].containerWpnType=FSWEAPON_FLAREPOD;
							staWeaponSlot[i].nContainerLoaded=1;
						}
						else
						{
							staWeaponSlot[i].containerWpnType=FSWEAPON_NULL;
							staWeaponSlot[i].nContainerLoaded=0;
						}

						nLoaded+=load;
						toLoad-=load;
					}
				}
			}
		}

		return nLoaded;
	}
	else if(FSWEAPON_FLARE_INTERNAL==wpnType)
	{
		const int nCanLoad=chMaxNumFlare-staFlare;
		staFlare+=nCanLoad;
		return nCanLoad;
	}
	return 0;
}

YSBOOL FsAirplaneProperty::GuidedAAMIsLoaded(void) const
{
	if(GetNumWeapon(FSWEAPON_AIM9)>0 || GetNumWeapon(FSWEAPON_AIM9X)>0 || GetNumWeapon(FSWEAPON_AIM120)>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::GuidedAGMIsLoaded(void) const
{
	if(GetNumWeapon(FSWEAPON_AGM65)>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::FreeFallBombIsLoaded(void) const
{
	if(GetNumWeapon(FSWEAPON_BOMB)>0 || GetNumWeapon(FSWEAPON_BOMB250)>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::AntiGroundWeaponIsLoaded(void) const
{
	if(GetNumWeapon(FSWEAPON_GUN)>0 ||  // 2006/05/13
	   GetNumWeapon(FSWEAPON_AGM65)>0 ||
	   GetNumWeapon(FSWEAPON_ROCKET)>0 ||
	   GetNumWeapon(FSWEAPON_BOMB)>0 ||
	   GetNumWeapon(FSWEAPON_BOMB250)>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void FsAirplaneProperty::UnloadAllWeapon(void)
{
	int i;
	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		staWeaponSlot[i].Initialize();
	}
}

void FsAirplaneProperty::UnloadGuidedAAM(void)
{
	SetNumWeapon(FSWEAPON_AIM9,0);
	SetNumWeapon(FSWEAPON_AIM9X,0);
	SetNumWeapon(FSWEAPON_AIM120,0);
}

void FsAirplaneProperty::UnloadGuidedAGM(void)
{
	SetNumWeapon(FSWEAPON_AGM65,0);
}

void FsAirplaneProperty::UnloadUnguidedWeapon(void)
{
	staGunBullet=0;
	SetNumWeapon(FSWEAPON_ROCKET,0);
	SetNumWeapon(FSWEAPON_BOMB,0);
	SetNumWeapon(FSWEAPON_BOMB250,0);
	SetNumWeapon(FSWEAPON_BOMB500HD,0);

	int i;
	for(i=0; i<staTurret.GetN(); i++)
	{
		staTurret[i].numBullet=0;
	}
}

/* int FsAirplaneProperty::WeaponTypeToSlotBit(FSWEAPONTYPE wpnType) const
{
	switch(wpnType)
	{
	case FSWEAPON_AIM9:
		return FSSLOT_AIM9;
	case FSWEAPON_AIM9X:
		return FSSLOT_AIM9X;
	case FSWEAPON_AGM65:
		return FSSLOT_AGM65;
	case FSWEAPON_BOMB:
		return FSSLOT_BOMB;
	case FSWEAPON_ROCKET:
		return FSSLOT_ROCKET;
	case FSWEAPON_AIM120:
		return FSSLOT_AIM120;
	case FSWEAPON_BOMB250:
		return FSSLOT_BOMB250;
	case FSWEAPON_BOMB500HD:
		return FSSLOT_BOMB500HD;
	}
	return 0;
} */

int FsAirplaneProperty::CountSlotWeaponLoad(FSWEAPONTYPE wpnType)
{
	int i,n;
	n=0;
	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		if(staWeaponSlot[i].wpnType==wpnType)
		{
			n+=staWeaponSlot[i].nLoaded;
		}
	}
	return n;
}

void FsAirplaneProperty::ShowSlotConfig(void)
{
	int i;
	printf("--------\n");
	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		printf("[%d] w%d n%d\n",i,staWeaponSlot[i].wpnType,staWeaponSlot[i].nLoaded);
	}
}

int FsAirplaneProperty::GetMaxNumWeapon(FSWEAPONTYPE wpnType) const
{
	int n,i;
	switch(wpnType)
	{
	case FSWEAPON_GUN:
		return chMaxNumGunBullet;
	case FSWEAPON_FLARE_INTERNAL:
		return chMaxNumFlare;
	default:
		n=0;
		if(wpnType==FSWEAPON_FLARE)
		{
			n=chMaxNumFlare;
		}
		for(i=0; i<chWeaponSlot.GetN(); i++)
		{
			n+=chWeaponSlot[i].nLoad[wpnType];
		}
		return n;
	}
	return 0;
}

int FsAirplaneProperty::GetMaxNumSlotWeapon(FSWEAPONTYPE wpnType) const
{
	int n=0,i;
	switch(wpnType)
	{
	case FSWEAPON_GUN:
		return 0;
	default:
		for(i=0; i<chWeaponSlot.GetN(); i++)
		{
			n+=chWeaponSlot[i].nLoad[wpnType];
		}
		return n;
	}
	return 0;
}


void FsAirplaneProperty::AddWeaponSlot(const YsVec3 &pos,int nLoad[FSWEAPON_NUMWEAPONTYPE],int nSubLoad[FSWEAPON_NUMWEAPONTYPE],YSBOOL isExternal)
{
	FsWeaponSlot newSlot;

	int i;
	for(i=0; i<FSWEAPON_NUMWEAPONTYPE; i++)
	{
		newSlot.nLoad[i]=nLoad[i];
		newSlot.nSubLoad[i]=nSubLoad[i];
	}
	newSlot.pos=pos;
	newSlot.isExternal=isExternal;
	chWeaponSlot.Append(newSlot);

	FsWeaponSlotLoading newSlotLoading;
	newSlotLoading.wpnType=FSWEAPON_GUN;
	newSlotLoading.nLoaded=0;
	staWeaponSlot.Append(newSlotLoading);
}

void FsAirplaneProperty::AddWeapon(FSWEAPONTYPE wpnType,int n) // For wpnType==FSWEAPON_FUEL n is taken as kilogram.
{
	if(wpnType==FSWEAPON_FLARE_INTERNAL)
	{
		int canLoad=chMaxNumFlare-staFlare;
		staFlare+=canLoad;
	}
	else
	{
		AddWeaponToSlot(wpnType,n);
	}
}

YSRESULT FsAirplaneProperty::GetWeaponConfig(YsArray <int,64> &loading) const
{
	int i;
	YsArray <FsWeaponSlotLoading> WeaponSlot;

	WeaponSlot=staWeaponSlot;
	loading.Set(0,NULL);

	if(0<staFlare)
	{
		loading.Append((int)FSWEAPON_FLARE_INTERNAL);
		loading.Append(staFlare);
	}

	if(YsTolerance<GetSmokeOil() && 0<GetNumSmokeGenerator())
	{
		YSBOOL allSameColor=YSTRUE;
		for(int i=0; i<GetNumSmokeGenerator(); ++i)
		{
			auto col=GetSmokeColor(i);
			if(col!=GetSmokeColor(0))
			{
				allSameColor=YSFALSE;
			}
		}
		if(YSTRUE==allSameColor)
		{
			auto col=GetSmokeColor(0);
			loading.Append((int)FSWEAPON_SMOKE);
			loading.Append((col.Ri()<<16)+(col.Gi()<<8)+(col.Bi()));
		}
		else
		{
			for(int i=0; i<GetNumSmokeGenerator(); ++i)
			{
				const int wpnType=(int)FSWEAPON_SMOKE0+i;
				if(FSWEAPON_SMOKE7<wpnType)
				{
					break;
				}
				auto col=GetSmokeColor(i);
				loading.Append(wpnType);
				loading.Append((col.Ri()<<16)+(col.Gi()<<8)+(col.Bi()));
			}
		}
	}

	for(i=0; i<WeaponSlot.GetN(); i++)
	{
		if(0<WeaponSlot[i].nLoaded)
		{
			if(FSWEAPON_FUELTANK==WeaponSlot[i].wpnType)
			{
				if(0<WeaponSlot[i].nLoaded)
				{
					loading.Append(WeaponSlot[i].wpnType);
					loading.Append((int)WeaponSlot[i].fuelLoaded);
				}
			}
			else
			{
				if(2<=loading.GetN() && loading[loading.GetN()-2]==WeaponSlot[i].wpnType)
				{
					loading[loading.GetN()-1]+=WeaponSlot[i].nLoaded;
				}
				else
				{
					loading.Append(WeaponSlot[i].wpnType);
					loading.Append(WeaponSlot[i].nLoaded);
				}
			}
		}
	}


	/* Original version
	for(;;)
	{
		YSBOOL copied;
		copied=YSFALSE;
		for(i=0; i<WeaponSlot.GetN(); i++)
		{
			if(WeaponSlot[i].nLoaded>0)  // Turned out I need to go one by one anyways.
			{
				if(loading.GetN()>0 && loading[loading.GetN()-2]==(int)WeaponSlot[i].wpnType)
				{
					loading[loading.GetN()-1]++;
				}
				else
				{
					loading.Append((int)WeaponSlot[i].wpnType);
					loading.Append(1);
				}
				WeaponSlot[i].nLoaded--;
				copied=YSTRUE;
			}
		}

		if(copied!=YSTRUE)
		{
			break;
		}
	} */

	return YSOK;
}

YSRESULT FsAirplaneProperty::ApplyWeaponConfig(YSSIZE_T n,const int loading[])
{
	UnloadAllWeapon();
	for(decltype(n) i=0; i<n-1; i+=2)
	{
		if((FSWEAPONTYPE)loading[i]==FSWEAPON_SMOKE)
		{
			SetSmokeOil(1.0);

			const int r=(loading[i+1]&0xff0000)>>16;
			const int g=(loading[i+1]&0x00ff00)>>8;
			const int b=(loading[i+1]&0x0000ff);

			for(auto &smkGen : staSmokeGenerator)
			{
				smkGen.col.SetIntRGB(r,g,b);
			}
		}
		else if((int)FSWEAPON_SMOKE0<=loading[i] && loading[i]<=(int)FSWEAPON_SMOKE7)
		{
			SetSmokeOil(1.0);

			const int smkIdx=loading[i]-(int)FSWEAPON_SMOKE0;
			if(0<=smkIdx && smkIdx<GetNumSmokeGenerator())
			{
				const int r=(loading[i+1]&0xff0000)>>16;
				const int g=(loading[i+1]&0x00ff00)>>8;
				const int b=(loading[i+1]&0x0000ff);
				staSmokeGenerator[smkIdx].col.SetIntRGB(r,g,b);
			}
		}
		else if((FSWEAPONTYPE)loading[i]==FSWEAPON_GUN)
		{
			staGunBullet=loading[i+1];
		}
		else if((FSWEAPONTYPE)loading[i]==FSWEAPON_FLARE_INTERNAL)
		{
			staFlare=loading[i+1];
		}
		else
		{
			AddWeapon((FSWEAPONTYPE)loading[i],loading[i+1]);
		}
	}
	return YSOK;
}

void FsAirplaneProperty::MakeUpWeaponConfigForOldVersion(YsArray <int,64> &loading,int aam,int agm,int rkt,int bom)
{
	loading.Set(0,NULL);
	loading.Append((int)FSWEAPON_AIM9);
	loading.Append(aam);
	loading.Append((int)FSWEAPON_AGM65);
	loading.Append(agm);
	loading.Append((int)FSWEAPON_ROCKET);
	loading.Append(rkt);
	loading.Append((int)FSWEAPON_BOMB);
	loading.Append(bom);
}

void FsAirplaneProperty::RemoveWeaponFromWeaponConfig(YSSIZE_T nWeaponConfig,int weaponConfig[],FSWEAPONTYPE wpnType)
{

	for(decltype(nWeaponConfig) i=0; i<nWeaponConfig-1; i+=2)
	{
		if(weaponConfig[i]==(int)wpnType)
		{
			weaponConfig[i+1]=0;
		}
	}
}

const double FsAirplaneProperty::GetReferenceCruisingAltitude(void) const
{
	return refAltCruise;
}

const double FsAirplaneProperty::GetRadarCrossSection(void) const
{
	int i;
	forYsArray(i,chWeaponSlot)
	{
		if(YSTRUE==IsWeaponSlotCurrentlyVisible(i))
		{
			return 1.0;
		}
	}

	return chRadarCrossSection+chBombBayRcs*(staBombBayDoor+staGear);
}

const double &FsAirplaneProperty::GetBulletSpeed(void) const
{
	return chBulInitSpeed;
}

const double &FsAirplaneProperty::GetRocketSpeed(void) const
{
	static double chRktSpd=800.0;
	return chRktSpd;
}

const double &FsAirplaneProperty::GetAAMRadarAngle(void) const
{
	static double x=YsPi/6.0;
	return x;
}

const double FsAirplaneProperty::GetAAMRange(FSWEAPONTYPE wpnType) const
{
	switch(wpnType)
	{
	default:
		break;
	case FSWEAPON_AIM9:
	case FSWEAPON_AIM9X:
		return 5000.0;
	case FSWEAPON_AIM120:
		return 30000.0;
	}
	return 0.0;
}

YSBOOL FsAirplaneProperty::GetLeadGunSight(void) const
{
	return chLeadGunSight;
}

int FsAirplaneProperty::GetNumWeapon(FSWEAPONTYPE wpnType) const
{
	int i,n=0;

	switch(wpnType)
	{
	case FSWEAPON_GUN:
		return staGunBullet;
	case FSWEAPON_SMOKE:
		if(YsTolerance<staSmokeOil)
		{
			return 1;
		}
		break;
	default:
		if(FSWEAPON_FLARE==wpnType)
		{
			n=staFlare;
		}
		for(i=0; i<staWeaponSlot.GetN(); i++)
		{
			if(staWeaponSlot[i].wpnType==wpnType)
			{
				n+=staWeaponSlot[i].nLoaded;
			}
		}
		return n;
	}
	return 0;
}

int FsAirplaneProperty::GetNumSlotWeapon(FSWEAPONTYPE wpnType) const
{
	int i,n=0;

	switch(wpnType)
	{
	case FSWEAPON_GUN:
		return 0;
	default:
		for(i=0; i<staWeaponSlot.GetN(); i++)
		{
			if(staWeaponSlot[i].wpnType==wpnType)
			{
				n+=staWeaponSlot[i].nLoaded;
			}
		}
		return n;
	}
	return 0;
}

double FsAirplaneProperty::GetExternalFuelLeft(void) const
{
	int i;
	double left=0.0;
	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		if(staWeaponSlot[i].wpnType==FSWEAPON_FUELTANK &&
		   0<staWeaponSlot[i].nLoaded)
		{
			left+=staWeaponSlot[i].fuelLoaded;
		}
	}
	return left;
}

void FsAirplaneProperty::SetNumWeapon(FSWEAPONTYPE wpnType,int n)
{
	switch(wpnType)
	{
	case FSWEAPON_GUN:
		staGunBullet=n;
		break;
	case FSWEAPON_FLARE_INTERNAL:
		staFlare=YsSmaller(n,chMaxNumFlare);
		break;
	default:
		LoadWeaponToSlot(wpnType,n);
		break;
	}
}

const wchar_t *FsAirplaneProperty::GetWeaponShapeFile(FSWEAPONTYPE wpnType,int state) const
{
	if(wpnType<FSWEAPON_NUMWEAPONTYPE)
	{
		switch(state)
		{
		case 0:
			return chWeaponShapeFileStatic[(int)wpnType];
		case 1:
			return chWeaponShapeFileFlying[(int)wpnType];
		}
	}
	return L"";
}

YSBOOL FsAirplaneProperty::SetAirTargetKey(unsigned int trgKey)
{
	if(GuidedAAMIsLoaded()==YSTRUE)
	{
		staAirTargetKey=trgKey;
		return YSTRUE;
	}
	return YSFALSE;
}

unsigned int FsAirplaneProperty::GetAirTargetKey(void) const
{
	if(GuidedAAMIsLoaded()==YSTRUE)
	{
		return staAirTargetKey;
	}
	return YSNULLHASHKEY;
}

const double &FsAirplaneProperty::GetAGMRadarAngle(void) const
{
	static double x=YsPi/9.0;
	return x;
}

const double &FsAirplaneProperty::GetAGMRange(void) const
{
	return chAGMRange;
}

int FsAirplaneProperty::GetAGMDestructivePower(void) const
{
	return FsWeapon::POWER_AGM65;
}

const double &FsAirplaneProperty::GetRocketRange(void) const
{
	return chRocketRange;
}

YSBOOL FsAirplaneProperty::SetGroundTargetKey(unsigned int gndKey)
{
	if(GuidedAGMIsLoaded()>0)
	{
		staGroundTargetKey=gndKey;
		return YSTRUE;
	}
	return YSFALSE;
}

unsigned int FsAirplaneProperty::GetGroundTargetKey(void) const
{
	if(GuidedAGMIsLoaded()>0)
	{
		return staGroundTargetKey;
	}
	return YSNULLHASHKEY;
}

YSRESULT FsAirplaneProperty::GetGunPosition(YsVec3 &gun,YsVec3 &gunDir) const
{
	gun=chGunPosition[0];
	gunDir=chGunDirection[0];
	return YSOK;
}

const double &FsAirplaneProperty::GetGunInterval(void) const
{
	return chGunInterval;
}

int FsAirplaneProperty::GetGunPower(void) const
{
	return chGunPower;
}

const double &FsAirplaneProperty::GetSmokeOil(void) const
{
	return staSmokeOil;
}

const YsColor &FsAirplaneProperty::GetSmokeColor(int smkIdx) const
{
	if(0<=smkIdx && smkIdx<MaxNumSmokeGenerator)
	{
		return staSmokeGenerator[smkIdx].col;
	}
	return YsWhite();
}

const YsColor &FsAirplaneProperty::GetDefaultSmokeColor(int smkIdx) const
{
	if(0<=smkIdx && smkIdx<MaxNumSmokeGenerator)
	{
		return chSmokeGenerator[smkIdx].defCol;
	}
	return YsWhite();
}

void FsAirplaneProperty::SetSmokeOil(const double &oil)
{
	staSmokeOil=oil;
}


void FsAirplaneProperty::MakeUpAircraftTroubleCommand(YsString &cmd,FSAIRCRAFTTROUBLE trouble,const double param0,const double param1,const double param2) const
{
	switch(trouble)
	{
	default:
	case FSAIRTROUBLE_LOSSGEARLOCK:
		cmd.Set("");
		break;
	case FSAIRTROUBLE_AUTOPILOT:
		cmd.Set("MALFUNCT AUTO");
		break;
	case FSAIRTROUBLE_FLAPSTUCK:
		cmd.Set("MALFUNCT FLAP");
		break;
	case FSAIRTROUBLE_RADAR:
		cmd.Set("MALFUNCT RADAR");
		break;
	case FSAIRTROUBLE_VOR:
		cmd.Set("MALFUNCT VOR");
		break;
	case FSAIRTROUBLE_ADF:
		cmd.Set("MALFUNCT ADF");
		break;
	case FSAIRTROUBLE_AIRSPEED:
		cmd.Printf("MALFUNCT AIRSPEED %.2lfm %.2lfm/s",param0,param1);
		break;
	case FSAIRTROUBLE_ALTIMETER:
		cmd.Printf("MALFUNCT ALTIMETER %.2lfm",param0);
		break;
	case FSAIRTROUBLE_VSI:
		cmd.Set("MALFUNCT VSI");
		break;
	case FSAIRTROUBLE_ATTITUDE:
		cmd.Printf("MALFUNCT ATTITUDE %.2lfrad %.2lfrad %.2lfrad",param0,param1,param2);
		break;
	case FSAIRTROUBLE_HUDFLICKER:
		cmd.Set("MALFUNCT HUDFLICKER");
		break;
	case FSAIRTROUBLE_PARTIALPOWERLOSS_70:
		cmd.Set("MALFUNCT POWER 0.7");
		break;
	case FSAIRTROUBLE_PARTIALPOWERLOSS_30:
		cmd.Set("MALFUNCT POWER 0.3");
		break;
	case FSAIRTROUBLE_TOTALPOWERLOSS:
		cmd.Set("MALFUNCT POWER 0.0");
		break;
	}
}

void FsAirplaneProperty::MakeUpClearAircraftTroubleCommand(YsString &cmd,FSAIRCRAFTTROUBLE trouble) const
{
	switch(trouble)
	{
	default:
	case FSAIRTROUBLE_LOSSGEARLOCK:
		cmd="";
		break;
	case FSAIRTROUBLE_AUTOPILOT:
		cmd="REPAIRFN AUTO";
		break;
	case FSAIRTROUBLE_FLAPSTUCK:
		cmd="REPAIRFN FLAP";
		break;
	case FSAIRTROUBLE_RADAR:
		cmd="REPAIRFN RADAR";
		break;
	case FSAIRTROUBLE_VOR:
		cmd="REPAIRFN VOR";
		break;
	case FSAIRTROUBLE_ADF:
		cmd="REPAIRFN ADF";
		break;
	case FSAIRTROUBLE_AIRSPEED:
		cmd="REPAIRFN AIRSPEED";
		break;
	case FSAIRTROUBLE_ALTIMETER:
		cmd="REPAIRFN ALTIMETER";
		break;
	case FSAIRTROUBLE_VSI:
		cmd="REPAIRFN VSI";
		break;
	case FSAIRTROUBLE_ATTITUDE:
		cmd="REPAIRFN ATTITUDE";
		break;
	case FSAIRTROUBLE_HUDFLICKER:
		cmd="REPAIRFN HUDFLICKER";
		break;
	case FSAIRTROUBLE_PARTIALPOWERLOSS_70:
		cmd="REPAIRFN POWER";
		break;
	case FSAIRTROUBLE_PARTIALPOWERLOSS_30:
		cmd="REPAIRFN POWER";
		break;
	case FSAIRTROUBLE_TOTALPOWERLOSS:
		cmd="REPAIRFN POWER";
		break;
	}
}

YSBOOL FsAirplaneProperty::CheckAutoPilotInop(void) const
{
	return staAutoPilotInop;
}

YSBOOL FsAirplaneProperty::CheckFlapInop(void) const
{
	return staFlapInop;
}

YSBOOL FsAirplaneProperty::CheckRadarInop(void) const
{
	return staRadarInop;
}

YSBOOL FsAirplaneProperty::CheckVORInop(void) const
{
	return staVorInop;
}

YSBOOL FsAirplaneProperty::CheckADFInop(void) const
{
	return staAdfInop;
}

YSBOOL FsAirplaneProperty::CheckAirspeedInop(void) const
{
	return staAirspeedInop;
}

const double FsAirplaneProperty::GetMalfunctioningAirspeedIndication(void) const
{
	return staAirspeedStuckAirspeed+(staPosition.y()-staAirspeedStuckAltitude)/25.0;
}

YSBOOL FsAirplaneProperty::CheckAltimeterInop(void) const
{
	return staAltimeterInop;
}

YSBOOL FsAirplaneProperty::CheckVSIInop(void) const
{
	return staVSIInop;
}

YSBOOL FsAirplaneProperty::CheckAttitudeIndicatorInop(void) const
{
	return staAttitudeIndicatorInop;
}

YSBOOL FsAirplaneProperty::CheckHUDFlicker(void) const
{
	return staHUDFlicker;
}

YSBOOL FsAirplaneProperty::CheckHUDVisible(void) const
{
	return staHUDVisible;
}



YSRESULT FsAirplaneProperty::EncodeProperty(
   YsArray <YsString> &cmd,YsArray <YsString> &turretCmd,unsigned int netVersion)
{
	int i;
	char cmdStr[256],a1[256]; // ,a2[256],a3[256];
	YsString str;

	cmd.Set(0,NULL);
	turretCmd.Set(0,NULL);

	sprintf(cmdStr,"AFTBURNR %s",FsTrueFalseString(chHasAb));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"WEIGHCLN %dkg",(int)chCleanWeight);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"WEIGFUEL %dkg",(int)chMaxFuelLoad);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"WEIGLOAD %dkg",(int)chMaxPayload);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"FUELABRN %skg",YsOptimizeDoubleString(a1,chAbFuelConsume));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"FUELMILI %skg",YsOptimizeDoubleString(a1,chMilFuelConsume));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CRITAOAP %.3lfrad",chMaxAOA);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CRITAOAM %.3lfrad",chMinAOA);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CRITSPED %sm/s",YsOptimizeDoubleString(a1,chVCritical));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"MAXSPEED %sm/s",YsOptimizeDoubleString(a1,chVMax));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"HASSPOIL %s",FsTrueFalseString(chHasSpoiler));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"RETRGEAR %s",FsTrueFalseString(chGearRetractable));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"VARGEOMW %s",FsTrueFalseString(chHasVGW));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CLVARGEO %s",YsOptimizeDoubleString(a1,chClVgw));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CDVARGEO %s",YsOptimizeDoubleString(a1,chCdVgw));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CLBYFLAP %s",YsOptimizeDoubleString(a1,chClFlap));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CDBYFLAP %s",YsOptimizeDoubleString(a1,chCdFlap));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CDBYGEAR %s",YsOptimizeDoubleString(a1,chCdGear));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CDSPOILR %s",YsOptimizeDoubleString(a1,chCdSpoiler));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"WINGAREA %sm^2",YsOptimizeDoubleString(a1,chWingArea));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"MXIPTAOA %.3lfrad",chMaxInputAOA);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"MXIPTSSA %.3lfrad",chMaxInputSSA);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"MXIPTROL %.3lfrad",chMaxInputROLL);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CPITMANE %s",YsOptimizeDoubleString(a1,chPitchManConst));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CPITSTAB %s",YsOptimizeDoubleString(a1,chPitchStabConst));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CYAWMANE %s",YsOptimizeDoubleString(a1,chYawManConst));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CYAWSTAB %s",YsOptimizeDoubleString(a1,chYawStabConst));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"CROLLMAN %s",YsOptimizeDoubleString(a1,chRollManConst));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"REFVCRUS %dm/s",(int)refSpdCruise);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"REFACRUS %dm",(int)refAltCruise);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"REFVLAND %sm/s",YsOptimizeDoubleString(a1,refSpdLanding));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"REFAOALD %.3lfrad",refAOALanding);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"REFLNRWY %sm",YsOptimizeDoubleString(a1,refLNGRunway));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"REFTHRLD %s",YsOptimizeDoubleString(a1,refThrLanding));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"REFTCRUS %s",YsOptimizeDoubleString(a1,refThrCruise));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"MANESPD1 %sm/s",YsOptimizeDoubleString(a1,chManSpeed1));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"MANESPD2 %sm/s",YsOptimizeDoubleString(a1,chManSpeed2));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"THRSTREV %s",YsOptimizeDoubleString(a1,chThrustReverser));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"HTRADIUS %sm",YsOptimizeDoubleString(a1,chOutsideRadius));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"STRENGTH %d",(int)staDamageTolerance);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	if(chIsJet==YSTRUE)
	{
		sprintf(cmdStr,"THRAFTBN %sN",YsOptimizeDoubleString(a1,chThrAb));
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);

		sprintf(cmdStr,"THRMILIT %sN",YsOptimizeDoubleString(a1,chThrMil));
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);
	}
	else
	{
		sprintf(cmdStr,"PROPELLR %.2lfJ/s",chThrMil);
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);

		sprintf(cmdStr,"PROPEFCY %.3lf",chPropEfficiency);
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);

		sprintf(cmdStr,"PROPVMIN %.3lfm/s",chPropV0);
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);
	}

	sprintf(cmdStr,"MANESPD3 %sm/s",YsOptimizeDoubleString(a1,chManSpeed3));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"RADARCRS %s",YsOptimizeDoubleString(a1,chRadarCrossSection));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	if(2<=chNumGun)
	{
		sprintf(cmdStr,"MACHNGN2 %.2lfm %.2lfm %.2lfm",chGunPosition[1].x(),chGunPosition[1].y(),chGunPosition[1].z());
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);
	}
	if(netVersion>=20060805)
	{
		sprintf(cmdStr,"NMACHNGN %d",chNumGun);
	}

	if(netVersion>=20100630)
	{
		sprintf(cmdStr,"MAXNMFLR %d",chMaxNumFlare);
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);
	}

	sprintf(cmdStr,"TRSTVCTR %s",FsTrueFalseString(chHasThrustVector));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	if(chHasThrustVector==YSTRUE)
	{
		sprintf(cmdStr,"TRSTDIR0 %.2lfm %.2lfm %.2lfm",chThrVec0.x(),chThrVec0.y(),chThrVec0.z());
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);

		sprintf(cmdStr,"TRSTDIR1 %.2lfm %.2lfm %.2lfm",chThrVec1.x(),chThrVec1.y(),chThrVec1.z());
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);
	}

	sprintf(cmdStr,"PSTMPTCH %srad",YsOptimizeDoubleString(a1,chPostStallVPitch));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"PSTMYAW_ %srad",YsOptimizeDoubleString(a1,chPostStallVYaw));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	sprintf(cmdStr,"PSTMROLL %srad",YsOptimizeDoubleString(a1,chPostStallVRoll));
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	switch(chClass)
	{
	case FSCL_AIRPLANE:
		str.Set("AIRCLASS AIRPLANE");
		cmd.Append(str);
		break;
	case FSCL_HELICOPTER:
		str.Set("AIRCLASS HELICOPTER");
		cmd.Append(str);
		break;
	}

	sprintf(cmdStr,"GUNPOWER %d",chGunPower);
	MakeShortFormat(str,cmdStr,netVersion);
	cmd.Append(str);

	if(netVersion>=20050207)
	{
		sprintf(cmdStr,"BOMINBAY %s",FsTrueFalseString(chBombInBombBay));
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);

		sprintf(cmdStr,"BMBAYRCS %s",YsOptimizeDoubleString(a1,chBombBayRcs));
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);

		sprintf(cmdStr,"GUNINTVL %.2lfsec",chGunInterval);
		MakeShortFormat(str,cmdStr,netVersion);
		cmd.Append(str);
	}

	str.Set("AUTOCALC");
	cmd.Append(str);





	sprintf(cmdStr,"NMTURRET %d",(int)chTurret.GetN());
	MakeShortFormat(str,cmdStr,netVersion);
	turretCmd.Append(str);
	for(i=0; i<chTurret.GetN(); i++)
	{
		sprintf(cmdStr,"TURRETPO %d %.2lfm %.2lfm %.2lfm %.2lfrad %.2lfrad %.2lfrad",
		   i,
		   chTurret[i].cen.x(),chTurret[i].cen.y(),chTurret[i].cen.z(),
		   chTurret[i].att.h(),chTurret[i].att.p(),chTurret[i].att.b());
		MakeShortFormat(str,cmdStr,netVersion);
		turretCmd.Append(str);

		sprintf(cmdStr,"TURRETPT %d %.2lfrad %.2lfrad %.2lfrad",
		   i,chTurret[i].pMin,chTurret[i].pMax,chTurret[i].pZero);
		MakeShortFormat(str,cmdStr,netVersion);
		turretCmd.Append(str);

		sprintf(cmdStr,"TURRETHD %d %.2lfrad %.2lfrad %.2lfrad",
		   i,chTurret[i].hMin,chTurret[i].hMax,chTurret[i].hZero);
		MakeShortFormat(str,cmdStr,netVersion);
		turretCmd.Append(str);

		sprintf(cmdStr,"TURRETAM %d %d",i,chTurret[i].maxNumGunBullet);
		MakeShortFormat(str,cmdStr,netVersion);
		turretCmd.Append(str);

		sprintf(cmdStr,"TURRETIV %d %.2lfsec",i,chTurret[i].shootInterval);
		MakeShortFormat(str,cmdStr,netVersion);
		turretCmd.Append(str);

		if(chTurret[i].dnmNodeName[0]!=0)
		{
			sprintf(cmdStr,"TURRETNM %d \"%s\"",i,chTurret[i].dnmNodeName.GetArray());
			MakeShortFormat(str,cmdStr,netVersion);
			turretCmd.Append(str);
		}

		if(chTurret[i].turretProp&FSTURRETPROP_ANTIAIR)
		{
			sprintf(cmdStr,"TURRETAR %d",i);
			MakeShortFormat(str,cmdStr,netVersion);
			turretCmd.Append(str);
		}

		if(chTurret[i].turretProp&FSTURRETPROP_ANTIGND)
		{
			sprintf(cmdStr,"TURRETGD %d",i);
			MakeShortFormat(str,cmdStr,netVersion);
			turretCmd.Append(str);
		}

		switch(chTurret[i].controlledBy)
		{
		default:
			break;
		case FSTURRET_CTRL_BY_PILOT:
			sprintf(cmdStr,"TURRETCT %d PILOT",i);
			MakeShortFormat(str,cmdStr,netVersion);
			turretCmd.Append(str);
			break;
		case FSTURRET_CTRL_BY_GUNNER:
			sprintf(cmdStr,"TURRETCT %d GUNNER",i);
			MakeShortFormat(str,cmdStr,netVersion);
			turretCmd.Append(str);
			break;
		}

		sprintf(cmdStr,"TURRETRG %d %sm",i,YsOptimizeDoubleString(a1,chTurret[i].range));
		MakeShortFormat(str,cmdStr,netVersion);
		turretCmd.Append(str);

		if(chTurret[i].dnmHdgNodeName[0]!=0)
		{
			sprintf(cmdStr,"TURRETNH %d \"%s\"",i,chTurret[i].dnmHdgNodeName.GetArray());
			MakeShortFormat(str,cmdStr,netVersion);
			turretCmd.Append(str);
		}

		if(chTurret[i].dnmPchNodeName[0]!=0)
		{
			sprintf(cmdStr,"TURRETNP %d \"%s\"",i,chTurret[i].dnmPchNodeName.GetArray());
			MakeShortFormat(str,cmdStr,netVersion);
			turretCmd.Append(str);
		}
	}

	return YSOK;
}

YsArray <YsString> FsAirplaneProperty::EncodeEngineProperty(unsigned int netVersion) const
{
	YsArray <YsString> engineProp;
	if(0==chRealProp.GetN())  // 2015/05/07 If there is zero real-prop, "NREALPRP 0" should be sent.
	{
		engineProp.Increment();
		MakeShortFormat(engineProp.Last(),"NREALPRP 0",netVersion);
	}
	engineProp.Append(chEnginePropCmd);
	return engineProp;
}


// Memo: after the 20040603 version, slot definition must come before INITIAAM, INITIAGM, and other initial loadings.

const char *const FsAirplaneProperty::keyWordSource[]=
{
	"AFTBURNR", //TRUE/FALSE          HAS AFTERBURNER
	"THRAFTBN", //##[N][KG][LB]       AFTERBURNER POWER
	"THRMILIT", //##[N][KG][LB]       MILITARY POWER
	"WEIGHCLN", //##[KG][LB]          CLEAN WEIGHT
	"WEIGFUEL", //##[KG][LB]          MAX WEIGHT OF FUEL
	"WEIGLOAD", //##[KG][LB]          MAX WEIGHT OF PAYLOAD
	"FUELABRN", //##[KG][LB]          FUEL CONSUMPTION/SEC WHEN BURNER ON
	"FUELMILI", //##[KG][LB]          FUEL CONSUMPTION/SEC WHEN MIL POWER

	"LEFTGEAR", //X Y Z [M][IN]        LEFT MAIN GEAR POSITION
	"RIGHGEAR", //X Y Z [M][IN]        RIGHT MAIN GEAR POSITION
	"WHELGEAR", //X Y Z [M][IN]        WHEEL POSITION

	"CRITAOAP", //##[RAD][DEG]        CRITICAL AOA (PLUS)
	"CRITAOAM", //##[RAD][DEG]        CRITICAL AOA (MINUS

	"CRITSPED", //##[KT][KM/H][M/S][MACH]CRITICAL AIRSPEED
	"MAXSPEED", //##[KT][KM/H][M/S][MACH]MAXIMUM AIRSPEED

	"HASSPOIL", //TRUE/FALSE          HAS SPOILER
	"RETRGEAR", //TRUE/FALSE          LANDING GEAR IS RETRACTABLE
	"VARGEOMW", //TRUE/FALSE          HAS VARIABLE GEOMETRY WING

	"CLVARGEO", //##(DIMENSIONLESS)   INCREASE OF CL WHEN VGW IS EXTENDED
	"CDVARGEO", //##(DIMENSIONLESS)   INCREASE OF CD WHEN VGW IS EXTENDED
	"CLBYFLAP", //##(DIMENSIONLESS)   INCREASE OF CL WHEN FLAP FULL DOWN
	"CDBYFLAP", //##(DIMENSIONLESS)   INCREASE OF CD WHEN FLAP FULL DOWN
	"CDBYGEAR", //##(DIMENSIONLESS)   INCREASE OF CD WHEN GEAR DOWN
	"CDSPOILR", //##(DIMENSIONLESS)   INCREASE OF CD WHEN SPOILER IS DEPLOYED

	"WINGAREA", //##[M^2][IN^2]       AREA OF WING

	"MXIPTAOA", //##[RAD][DEG]           MAX INPUT AOA
	"MXIPTSSA", //##[RAD][DEG]           MAX INPUT YAW
	"MXIPTROL", //##[RAD][DEG]           MAX INPUT ROLL RATIO

	"CPITMANE", //##(DIMENSIONLESS)   PITCH MANEUVABILITY CONSTANT
	"CPITSTAB", //##(DIMENSIONLESS)   PITCH STABILITY CONSTANT
	"CYAWMANE", //##(DIMENSIONLESS)   YAW MANEUVABILITY CONSTANT
	"CYAWSTAB", //##(DIMENSIONLESS)   YAW STABILITY CONSTANT
	"CROLLMAN", //##(DIMENSIONLESS)   ROLL MANEUVABILITY CONSTANT


	"CTLLDGEA", //TRUE/FALSE          INITIAL GEAR
	"CTLBRAKE", //TRUE/FALSE          INITIAL BRAKE
	"CTLSPOIL", //0.0-1.0             INITIAL SPOILER
	"CTLABRNR", //TRUE/FALSE          INITIAL AFTERBURNER
	"CTLTHROT", //0.0-1.0             INITIAL THROTTLE
	"CTLIFLAP", //0.0-1.0             INITIAL FLAP
	"CTLINVGW", //0.0-1.0             INITIAL VGW
	"CTLATVGW", //TRUE/FALSE          INITIAL AUTO VGW

	"POSITION", //X Y Z [M][IN]
	"ATTITUDE", //H P B [DEG][RAD]
	"INITFUEL", //##[KG][LB]
	"INITLOAD", //##[KG][LB]
	"INITSPED", //##[M/S][KT][MACH]



	"REFVCRUS", //##[M/S][KM/H][KT]   CRUISING SPEED
	"REFACRUS", //##[M][FT]           CRUISING ALTITUDE
	"REFVLAND", //##[M/S][KM/H][KT]   LANDING SPEED
	"REFAOALD", //##[DEG][RAD]        AOA WHILE APPROACHING
	"REFLNRWY", //##[M][FT][KM]       RUNWAY LENGTH REQUIRED TO LAND

	"REM",

	"COCKPITP",
	"REFTHRLD",
	"REFTCRUS",

	"AUTOCALC",

	"IDENTIFY",

	"MANESPD1",
	"MANESPD2",

	"MACHNGUN",
	"SMOKEGEN",
	"HTRADIUS",
	"TRIGGER1",
	"TRIGGER2",
	"TRIGGER3",
	"TRIGGER4",

	"STRENGTH",

	"PROPELLR",

	"VAPORPO0",
	"VAPORPO1",

	"INITIGUN",
	"INITIAAM",
	"INITIAGM",

	"MANESPD3",

	"RADARCRS",

	"MACHNGN2",

	"SMOKEOIL",
	"WEAPONCH",
	"INITBOMB",

	"MONTRILS",

	"GUNPOWER",

	"CATEGORY",  // Normal,Utility or Aerobatic (+fighter, attacker)

	"VGWSPED1",  // Auto Vgw Reference Speed (Slower Speed)
	"VGWSPED2",  // Auto Vgw Reference Speed (Faster Speed)

	"GUNDIREC",  // GUN direction

	// 2001/05/06 >>
	"INITRCKT",  // Initial number of rockets
	"MAXNMGUN",  // chMaxNumGunBullet
	"MAXNMAAM",  // chMaxNumAAM               Deprecated 2010/08/04
	"MAXNMAGM",  // chMaxNumAGM               Deprecated 2010/08/04
	"MAXNMRKT",  // chMaxNumRocket            Deprecated 2010/08/04

	// 2001/06/05 >>
	"AAMSLOT_",  // chAAMSlot[chNumAAMSlot++]
	"AGMSLOT_",  // chAGMSlot[chNumAGMSlot++]
	"RKTSLOT_",  // chRocketSlot[chNumRocketSlot++]
	"BOMBSLOT",  // chBombSlot[chNumBombSlot++]
	"AAMVISIB",  // chAAMVisible;
	"AGMVISIB",  // chAGMVisible;
	"BOMVISIB",  // chBombVisible;
	"RKTVISIB",  // chRocketVisible
	"MAXNBOMB",  // chMaxNumBomb              Deprecated 2010/08/04

	// 2002/12/11 >>
	"ARRESTER",  // chArrestingHook

	// 2003/02/02 >>
	"TRSTVCTR",  // chHasThrustVector
	"TRSTDIR0",  // chThrVec0
	"TRSTDIR1",  // chThrVec1
	"PSTMPTCH",  // Post-Stall VPitch
	"PSTMYAW_",  // Post-Stall VYaw
	"PSTMROLL",  // Post-Stall VRoll

	// 2003/02/12
	"AIRCLASS",  // Aircraft class

	// 2003//02/15
	"PROPEFCY",  // Propeller efficiency
	"PROPVMIN",  // Minimum speed that T=P/v becomes valid

	// 2003/09/19
	"VRGMNOSE",  // Variable Geometry Nose : Concorde only


	// 2003/11/25
	"THRSTREV",  // Effectiveness of the Thrust Reverser


	// 2004/05/22
	"GUNSIGHT",  // Lead Gun Sight


	// 2004/06/14
	"HRDPOINT",  // Defining a hardpoint
	"LOADWEPN",  // Load weapons
	"LMTBYHDP",  // Limit weapons by hardpoint definition.
	"UNLOADWP",  // Unload All Weapons (Missiles, Bombs, Rockets.  Excluding Guns, Smokes, and Flare)

	// 2005/01/03
	"INSTPANL",  // Draw an instrument panel instead of a hud. (av[1] for inst panel definition file.)

	// 2005/01/05
	"MACHNGN3",
	"MACHNGN4",
	"MACHNGN5",
	"MACHNGN6",
	"MACHNGN7",
	"MACHNGN8",

	// 2005/01/11
	"BOMINBAY",
	"BMBAYRCS",

	// 2005/01/23
	"INITAAMM",  // Mid-Range AAM
	"MAXNAAMM",  // Max # Mid-Range AAM
	"INITB250",  // 250lb Bomb
	"MAXNB250",  // Max # 250lb Bomb

	// 2005/03/08
	"GUNINTVL",  // Gun Interval

	// 2005/06/26
	"NMTURRET",  // Number of turret
	"TURRETPO",  //  0 0m -0.8m 2.7m 0deg 0deg 0deg      # Number x y z h p b
	"TURRETPT",  //  0 -40deg 0deg 0deg                  # Number MinPitch MaxPitch NeutralPitch
	"TURRETHD",  //  0 -120deg 120deg 0deg               # Number MinHdg MaxHdg NeutralHdg
	"TURRETAM",  //  0 0                                 # Ammo(zero -> staGunBullet will be used)
	"TURRETIV",  //  0 0.5sec                            # Number ShootingInterval
	"TURRETNM",  //  0 GUN                               # DNM Node Name
	"TURRETAR",  //  0 FALSE                             # TRUE -> Anti Air Capable
	"TURRETGD",  //  0 TRUE                              # TRUE -> Anti Ground Capable
	"TURRETCT",  //  "PILOT" or "GUNNER"
	"TURRETRG",  //  Range
	// 2005/09/28
	"TURRETNH",  // DNM Node Name (Heading Rotation)
	"TURRETNP",  // DNM Node Name (Pitch Rotation)

	// 2006/04/25
	"SETCNTRL",  // Set Control eg. ILS TRIM:0.3 etc.

	// 2006/07/19
	"EXCAMERA",  // Extra Camera

	// 2006/08/05
	"NMACHNGN",  // Number of machine guns.

	// 2007/04/06
	"SMOKECOL",  // Smoke Color #dmy# R G B

	// 2007/09/16
	"SUBSTNAM",  // Substitute airplane (In case the airplane was not installed)

	// 2010/06/26
	"ISPNLPOS",  // Instrument Panel Position
	"ISPNLSCL",  // Instrument Panel Scaling

	// 2010/06/29
	"ISPNLHUD",  // Use both inst panel and HUD
	"COCKPITA",  // Neutral Head Direction

	// 2010/06/30
	"SCRNCNTR",  // Screen center (Relative.  (-1.0,-1.0)-(1.0,1.0)
	"ISPNLATT",  // Instrument Panel Orientation
	"MAXNMFLR",  // Maximum number of flare

	// 2010/07/01
	"FLAPPOSI",  // Flap position
	"FLAREPOS",  // Flare Dispenser Position and Direction

	// 2010/12/11
	"INITAAAM",  // Initialize AIM9X
	"INITHDBM",  // Initialize High-Drag bomb
	"ULOADAAM",  // Unload all AAMs
	"ULOADAGM",  // Unload all AGMs
	"ULOADBOM",  // Unload all Bombs
	"ULOADFLR",  // Unload all Flare
	"ULOADGUN",  // Unload all Gun
	"ULOADRKT",  // Unload all Rocket

	// 2011/12/25
	"LOOKOFST",  // Look-at Offset

	// 2012/02/02
	"WPNSHAPE",  // Weapon-shape override

	// 2012/02/21
	"GEARHORN",  // Landing-gear warning horn
	"STALHORN",  // Stall-warning horn

	// 2013/04/14
	"CKPITIST",  // To Make inst panel available in only one of EXCAMERAs, it can be hidden in the default cockpit view.
	"CKPITHUD",  // To Make HUD available in only one of EXCAMERAs, it can be hidden in the default cockpit view.

	// 2013/04/25
	"MALFUNCT",  // Malfunction
	"REPAIRAL",  // Repair all

	// 2013/04/25
	"REPAIRFN",  // Repair functionality

	// 2013/06/02
	"NOLDGFLR",  // No landing flare

	// 2014/06/05
	"NREALPRP",  // Number of (real) propeller engines
	"REALPROP",  // Support for realistic propeller engine

	// 2014/06/13
	"TIREFRIC",  // Tire friction coefficient

	// 2014/06/24
	"PSTMSPD1",   // Maximum speed that the direct attitude control is fully effective.
	"PSTMSPD2",   // Speed at which the direct attitude control becomes ineffective.
	"PSTMPWR1",   // Minimum required power setting for direct attitude control
	"PSTMPWR2",   // Power setting at which the direct attitude control is fully effective

	// 2014/07/11
	"MAXCDAOA",
	"FLATCLR1",
	"FLATCLR2",
	"CLDECAY1",
	"CLDECAY2",

	// 2014/10/17
	"AIRSTATE",  // I'm shocked that I didn't have it yet.

	// 2018/10/07
	"INITZOOM",  // Initial zoom factor

	NULL
};

YsKeyWordList FsAirplaneProperty::keyWordList;

void FsAirplaneProperty::MakeShortFormat(YsString &str,const char srcCmd[],int netVersion) const
{
	if(netVersion>=20060805)
	{
		int i;
		YsArray <char,16> cmd;

		if(keyWordList.GetN()==0)
		{
			keyWordList.MakeList(keyWordSource);
		}

		for(i=0; srcCmd[i]!=' ' && srcCmd[i]!='\t' && srcCmd[i]!=0; i++)
		{
			cmd.Append(srcCmd[i]);
		}
		cmd.Append(0);

		int id;
		id=keyWordList.GetId(cmd);

		if(id>=0)
		{
			char buf[256];
			sprintf(buf,"*%d",id);
			str.Set(buf);
			str.Append(srcCmd+i);
		}
		else
		{
			str.Set(srcCmd);
		}
	}
	else
	{
		str.Set(srcCmd);
	}
}

YSRESULT FsAirplaneProperty::SendCommand(const char in[])
{
	if(NULL!=in && '#'==in[0])
	{
		return YSOK;
	}

	int turretId;
	int ac;
	char *av[32];
	char buf[256];
	strcpy(buf,in);
	if(YsArguments(&ac,av,32,buf)==YSOK && ac>0)
	{
		int cmd;
		YSRESULT res;
		YSBOOL boo;
		double dou;

		if(keyWordList.GetN()==0)
		{
			keyWordList.MakeList(keyWordSource);
		}

		// if(YsCommandNumber(&cmd,av[0],FsAirPropCommand)==YSOK)

		if(av[0][0]=='*')
		{
			cmd=atoi(((const char *)(av[0]))+1);
		}
		else
		{
			cmd=keyWordList.GetId(av[0]);
		}

		if(cmd<0 && strcmp(av[0],"HRDPOINT_")==0)  // 2006/07/20
		{                                          // Band-aid for O.C.P. F-15E
			cmd=keyWordList.GetId("HRDPOINT");     //
		}                                          //

		if(cmd>=0)
		{
			res=YSERR;
			switch(cmd)
			{
			case  0: //"AFTBURNR", //TRUE/FALSE
				res=FsGetBool(chHasAb,av[1]);
				chIsJet=YSTRUE;
				break;
			case  1: //"THRAFTBN", //##[N][KG][LB]
				res=FsGetForce(chThrAb,av[1]);
				chIsJet=YSTRUE;
				break;
			case  2: //"THRMILIT", //##[N][KG][LB]
				res=FsGetForce(chThrMil,av[1]);
				chIsJet=YSTRUE;
				break;
			case  3: //"WEIGHCLN", //##[KG][LB]
				res=FsGetWeight(chCleanWeight,av[1]);
				break;
			case  4: //"WEIGFUEL", //##[KG][LB]
				res=FsGetWeight(chMaxFuelLoad,av[1]);
				break;
			case  5: //"WEIGLOAD", //##[KG][LB]
				res=FsGetWeight(chMaxPayload,av[1]);
				break;
			case  6: //"FUELABRN", //##[KG][LB]
				res=FsGetWeight(chAbFuelConsume,av[1]);
				break;
			case  7: //"FUELMILI", //##[KG][LB]
				res=FsGetWeight(chMilFuelConsume,av[1]);
				break;
			case  8: //"LEFTGEAR", //X Y Z [M][IN]
				res=FsGetVec3(chMainGearL,ac-1,av+1);
				break;
			case  9: //"RIGHGEAR", //X Y Z [M][IN]
				res=FsGetVec3(chMainGearR,ac-1,av+1);
				break;
			case 10: //"WHELGEAR", //X Y Z [M][IN]
				res=FsGetVec3(chWheel,ac-1,av+1);
				break;
			case 11: //"CRITAOAP", //##[RAD][DEG]
				res=FsGetAngle(chMaxAOA,av[1]);
				break;
			case 12: //"CRITAOAM", //##[RAD][DEG]
				res=FsGetAngle(chMinAOA,av[1]);
				break;
			case 13: //"CRITSPED", //##[KT][KM/H][M/S][MA
				res=FsGetSpeed(chVCritical,av[1]);
				break;
			case 14: //"MAXSPEED", //##[KT][KM/H][M/S][MA
				res=FsGetSpeed(chVMax,av[1]);
				break;
			case 15: //"HASSPOIL", //TRUE/FALSE
				res=FsGetBool(chHasSpoiler,av[1]);
				break;
			case 16: //"RETRGEAR", //TRUE/FALSE
				res=FsGetBool(chGearRetractable,av[1]);
				break;
			case 17: //"VARGEOMW", //TRUE/FALSE
				res=FsGetBool(chHasVGW,av[1]);
				break;
			case 18: //"CLVARGEO", //##(DIMENSIONLESS)
				res=FsGetNonDimensional(chClVgw,av[1]);
				break;
			case 19: //"CDVARGEO", //##(DIMENSIONLESS)
				res=FsGetNonDimensional(chCdVgw,av[1]);
				break;
			case 20: //"CLBYFLAP", //##(DIMENSIONLESS)
				res=FsGetNonDimensional(chClFlap,av[1]);
				break;
			case 21: //"CDBYFLAP", //##(DIMENSIONLESS)
				res=FsGetNonDimensional(chCdFlap,av[1]);
				break;
			case 22: //"CDBYGEAR", //##(DIMENSIONLESS)
				res=FsGetNonDimensional(chCdGear,av[1]);
				break;
			case 23: //"CDSPOILR", //##(NonDimensional)
				res=FsGetNonDimensional(chCdSpoiler,av[1]);
				break;
			case 24: //"WINGAREA", //##[M^2][IN^2]
				res=FsGetArea(chWingArea,av[1]);
				break;
			case 25: //"MXIPTAOA", //##[RAD][DEG]
				res=FsGetAngle(chMaxInputAOA,av[1]);
				break;
			case 26: //"MXIPTSSA", //##[RAD][DEG]
				res=FsGetAngle(chMaxInputSSA,av[1]);
				break;
			case 27: //"MXIPTROL", //##[RAD][DEG]
				res=FsGetAngle(chMaxInputROLL,av[1]);
				break;
			case 28: //"CPITMANE", //##(NonDimensional)
				res=FsGetNonDimensional(chPitchManConst,av[1]);
				break;
			case 29: //"CPITSTAB", //##(NonDimensional)
				res=FsGetNonDimensional(chPitchStabConst,av[1]);
				break;
			case 30: //"CYAWMANE", //##(NonDimensional)
				res=FsGetNonDimensional(chYawManConst,av[1]);
				break;
			case 31: //"CYAWSTAB", //##(NonDimensional)
				res=FsGetNonDimensional(chYawStabConst,av[1]);
				break;
			case 32: //"CROLLMAN", //##(NonDimensional)
				res=FsGetNonDimensional(chRollManConst,av[1]);
				break;
			case 33: //"CTLLDGEA", //TRUE/FALSE
				res=FsGetBool(boo,av[1]);
				ctlGear=(boo==YSTRUE ? 1.0 : 0.0);
				staGear=ctlGear;
				break;
			case 34: //"CTLBRAKE", //TRUE/FALSE
				res=FsGetBool(boo,av[1]);
				ctlBrake=(boo==YSTRUE ? 1.0 : 0.0);
				staBrake=ctlBrake;
				break;
			case 35: //"CTLSPOIL", //0.0-1.0
				res=FsGetNonDimensional(ctlSpoiler,av[1]);
				staSpoiler=ctlSpoiler;
				break;
			case 36: //"CTLABRNR", //TRUE/FALSE
				res=FsGetBool(ctlAb,av[1]);
				staAb=ctlAb;
				break;
			case 37: //"CTLTHROT", //0.0-1.0
				res=FsGetNonDimensional(ctlThrottle,av[1]);
				staThrottle=ctlThrottle;
				break;
			case 38: //"CTLIFLAP", //0.0-1.0
				res=FsGetNonDimensional(ctlFlap,av[1]);
				staFlap=ctlFlap;
				break;
			case 39: //"CTLINVGW", //0.0-1.0
				res=FsGetNonDimensional(ctlVgw,av[1]);
				staVgw=ctlVgw;
				break;
			case 40: //"CTLATVGW", //TRUE/FALSE
				res=FsGetBool(ctlAutoVgw,av[1]);
				break;
			case 41: //"POSITION", //X Y Z [M][IN]
				res=FsGetVec3(staPosition,ac-1,av+1);
				break;
			case 42: //"ATTITUDE", //H P B [DEG][RAD]
				res=FsGetAtt3(staAttitude,ac-1,av+1);
				break;
			case 43: //"INITFUEL", //##[KG][LB]
				if(av[1][strlen(av[1])-1]=='%')
				{
					res=YSOK;
					double percent;
					percent=atof(av[1])/100.0;
					staFuelLoad=chMaxFuelLoad*percent;

					// if(staFuelLoad<YsTolerance)
					// {
					// 	fsStderr.Printf("Zero fuel loaded\n");
					// }
				}
				else
				{
					res=FsGetWeight(staFuelLoad,av[1]);
				}
				break;
			case 44: //"INITLOAD", //##[KG][LB]
				res=FsGetWeight(staPayload,av[1]);
				break;
			case 45: //"INITSPED", //##[M/S][KT][MACH]
				res=FsGetSpeed(dou,av[1]);
				staV=dou;
				staVelocity.Set(0.0,0.0,dou);
				staAttitude.Mul(staVelocity,staVelocity);
				break;
			case 46: //"REFVCRUS", //##[M/S][KM/H][KT]
				res=FsGetSpeed(refSpdCruise,av[1]);
				break;
			case 47: //"REFACRUS", //##[M][FT]
				res=FsGetLength(refAltCruise,av[1]);
				break;
			case 48: //"REFVLAND", //##[M/S][KM/H][KT]
				res=FsGetSpeed(refSpdLanding,av[1]);
				break;
			case 49: //"REFAOALD", //##[DEG][RAD]
				res=FsGetAngle(refAOALanding,av[1]);
				break;
			case 50: //"REFLNRWY", //##[M][FT][KM]
				res=FsGetLength(refLNGRunway,av[1]);
				break;
			case 51: // "REM"
				res=YSOK;
				break;
			case 52: // "COCKPITP"
				res=FsGetVec3(chCockpit,ac-1,av+1);
				break;
			case 53: // "REFTHRLD"
				res=FsGetNonDimensional(refThrLanding,av[1]);
				break;
			case 54: // "REFTCRUS"
				res=FsGetNonDimensional(refThrCruise,av[1]);
				break;
			case 55: // "AUTOCALC"
				AutoCalculate();
				res=YSOK;
				break;
			case 56: // "IDENTIFY"
				chIdName.Set(av[1]);
				for(int i=0; chIdName[i]!=0; i++)
				{
					if(' '==chIdName[i] || '\t'==chIdName[i] || '\"'==chIdName[i])
					{
						chIdName.Set(i,'_');
					}
				}
				chIdName.Capitalize();
				res=YSOK;
				break;
			case 57: // "MANESPD1"
				res=FsGetSpeed(chManSpeed1,av[1]);
				break;
			case 58: // "MANESPD2"
				res=FsGetSpeed(chManSpeed2,av[1]);
				break;
			case 59: //"MACHNGUN"
				res=FsGetVec3(chGunPosition[0],ac-1,av+1);
				break;
			case 60: //"SMOKEGEN"
				if(chNumSmokeGenerator<MaxNumSmokeGenerator)
				{
					res=FsGetVec3(chSmokeGenerator[chNumSmokeGenerator].pos,ac-1,av+1);
					chNumSmokeGenerator++;
				}
				break;
			case 61: //"HTRADIUS"
				res=FsGetLength(chOutsideRadius,av[1]);
				break;
			case 62: //"TRIGGER1"
				res=YSOK;
				break;
			case 63: //"TRIGGER2"
				res=YSOK;
				break;
			case 64: //"TRIGGER3"
				res=YSOK;
				break;
			case 65: //"TRIGGER4"
				res=YSOK;
				break;
			case 66: //"STRENGTH"
				res=YSOK;
				staDamageTolerance=atoi(av[1]);
				break;
			case 67: //"PROPELLR"
				res=FsGetJoulePerSecond(chThrMil,av[1]);
				chIsJet=YSFALSE;
				break;
			case 68: //"VAPORPO0"
				res=FsGetVec3(chVapor0,ac-1,av+1);
				break;
			case 69: //"VAPORPO1"
				res=FsGetVec3(chVapor1,ac-1,av+1);
				break;
			case 70: //"INITIGUN",
				chMaxNumGunBullet=YsGreater(chMaxNumGunBullet,atoi(av[1]));
				staGunBullet=atoi(av[1]);
				res=YSOK;
				break;
			case 71: //"INITIAAM",
				// chMaxNumAAM=YsGreater(chMaxNumAAM,atoi(av[1]));  // Max must be set before SetNum
				SetNumWeapon(FSWEAPON_AIM9,atoi(av[1]));
				res=YSOK;
				break;
			case 72: //"INITIAGM",
				// chMaxNumAGM=YsGreater(chMaxNumAGM,atoi(av[1]));  // Max must be set before SetNum
				SetNumWeapon(FSWEAPON_AGM65,atoi(av[1]));
				res=YSOK;
				break;
			case 73: //"MANESPD3"
				res=FsGetSpeed(chManSpeed3,av[1]);
				break;

			case 74: //"RADARCRS"
				res=YSOK;
				chRadarCrossSection=atof(av[1]);
				break;
			case 75: //"MACHNGN2",
				chNumGun=YsGreater(chNumGun,2);
				res=FsGetVec3(chGunPosition[1],ac-1,av+1);
				break;
			case 76: //"SMOKEOIL"
				res=FsGetWeight(staSmokeOil,av[1]);
				break;
			case 77: //"WEAPONCH"
				res=FsGetWeaponOfChoice(staWoc,av[1]);
				break;
			case 78: //"INITBOMB"
				// chMaxNumBomb=YsGreater(chMaxNumBomb,atoi(av[1]));  // Max must be set before SetNum
				SetNumWeapon(FSWEAPON_BOMB,atoi(av[1]));
				res=YSOK;
				break;
			case 79: // "MONTRILS"
				// res=FsGetBool(staIls,av[1]);
				break;
			case 80: // "GUNPOWER"
				chGunPower=atoi(av[1]);
				res=YSOK;
				break;
			case 81: // "CATEGORY"
				res=YSOK;
				chCategory=FsGetAirplaneCategoryFromString(av[1]);
				break;
			case 82:
				res=FsGetSpeed(chAutoVgwSpeed1,av[1]);
				break;
			case 83:
				res=FsGetSpeed(chAutoVgwSpeed2,av[1]);
				break;
			case 84:
				res=FsGetVec3(chGunDirection[0],ac-1,av+1);
				for(int i=0; i<MaxNumGun; i++)
				{
					chGunDirection[i]=chGunDirection[0];
					chGunDirection[i].Normalize();
				}
				break;

			case 85: //	"INITRCKT",  // Initial number of rockets
				// chMaxNumRocket=YsGreater(chMaxNumRocket,atoi(av[1]));  // Max must be set before SetNum
				SetNumWeapon(FSWEAPON_ROCKET,atoi(av[1]));
				res=YSOK;
				break;
			case 86: //	"MAXNMGUN",  // chMaxNumGunBullet
				chMaxNumGunBullet=atoi(av[1]);
				res=YSOK;
				break;
			case 87: //	"MAXNMAAM",  // chMaxNumAAM
				// chMaxNumAAM=atoi(av[1]);
				res=YSOK;
				break;
			case 88: //	"MAXNMAGM",  // chMaxNumAGM
				// chMaxNumAGM=atoi(av[1]);
				res=YSOK;
				break;
			case 89: //	"MAXNMRKT",  // chMaxNumRocket
				// chMaxNumRocket=atoi(av[1]);
				res=YSOK;
				break;

			case 90: //"AAMSLOT_",  // chAAMSlot[chNumAAMSlot++]
				{
					YsVec3 pos;
					if((res=FsGetVec3(pos,ac-1,av+1))==YSOK)
					{
						FsWeaponSlot slot;
						slot.nLoad[FSWEAPON_AIM9]=1;
						AddWeaponSlot(pos,slot.nLoad,slot.nSubLoad,YSTRUE);
					}
				}
				break;
			case 91: //"AGMSLOT_",  // chAGMSlot[chNumAGMSlot++]
				{
					YsVec3 pos;
					if((res=FsGetVec3(pos,ac-1,av+1))==YSOK)
					{
						FsWeaponSlot slot;
						slot.nLoad[FSWEAPON_AGM65]=1;
						AddWeaponSlot(pos,slot.nLoad,slot.nSubLoad,YSTRUE);
					}
				}
				break;
			case 92: //"RKTSLOT_",  // chRocketSlot[chNumRocketSlot++]
				{
					YsVec3 pos;
					if((res=FsGetVec3(pos,ac-1,av+1))==YSOK)
					{
						FsWeaponSlot slot;
						slot.nLoad[FSWEAPON_ROCKET]=19;
						AddWeaponSlot(pos,slot.nLoad,slot.nSubLoad,YSTRUE);
					}
				}
				break;
			case 93: //"BOMBSLOT",  // chBombSlot[chNumBombSlot++]
				{
					YsVec3 pos;
					if((res=FsGetVec3(pos,ac-1,av+1))==YSOK)
					{
						FsWeaponSlot slot;
						slot.nLoad[FSWEAPON_BOMB]=1;
						AddWeaponSlot(pos,slot.nLoad,slot.nSubLoad,YSTRUE);
					}
				}
				break;

			case 94: //	"AAMVISIB",  // chAAMVisible;
				res=FsGetBool(chAAMVisible,av[1]);
				break;
			case 95: //	"AGMVISIB",  // chAGMVisible;
				res=FsGetBool(chAGMVisible,av[1]);
				break;
			case 96: //	"BOMVISIB",  // chBombVisible;
				res=FsGetBool(chBombVisible,av[1]);
				break;
			case 97: //	"RKTVISIB",  // chRocketVisible
				res=FsGetBool(chRocketVisible,av[1]);
				break;

			case 98: //	"MAXNBOMB",  // chMaxNumBomb
				// chMaxNumBomb=atoi(av[1]);
				res=YSOK;
				break;
			case 99: // "ARRESTER",  // chArrestingHook
				res=FsGetVec3(chArrestingHook,ac-1,av+1);
				break;
			case 100: // "TRSTVCTR",  // chHasThrustVector
				res=FsGetBool(chHasThrustVector,av[1]);
				break;
			case 101: // "TRSTDIR0",  // chThrVec0
				res=FsGetVec3(chThrVec0,ac-1,av+1);
				break;
			case 102: // "TRSTDIR1",  // chThrVec1
				res=FsGetVec3(chThrVec1,ac-1,av+1);
				break;
			case 103: // "PSTMPTCH",  // Post-Stall VPitch
				res=FsGetAngle(chPostStallVPitch,av[1]);
				break;
			case 104: // "PSTMYAW_",  // Post-Stall VYaw
				res=FsGetAngle(chPostStallVYaw,av[1]);
				break;
			case 105: // "PSTMROLL",  // Post-Stall VRoll
				res=FsGetAngle(chPostStallVRoll,av[1]);
				break;
			case 106: // 	"AIRCLASS",  // Aircraft class
				if(strcmp(av[1],"AIRPLANE")==0 || strcmp(av[1],"Airplane")==0 || strcmp(av[1],"airplane")==0)
				{
					chClass=FSCL_AIRPLANE;
					res=YSOK;
				}
				else if(strcmp(av[1],"HELICOPTER")==0 ||
				        strcmp(av[1],"Helicopter")==0 ||
				        strcmp(av[1],"helicopter")==0)
				{
					chClass=FSCL_HELICOPTER;
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 107:   //"PROPEFCN",  // Propeller efficiency
				chPropEfficiency=atof(av[1]);
				res=YSOK;
				break;
			case 108:   //"PROPVMIN",  // Minimum speed that T=P/v becomes valid
				res=FsGetSpeed(chPropV0,av[1]);
				break;
			case 109:   //"VRGMNOSE"   // Variable Geometry Nose TRUE/FALSE
				res=FsGetBool(chHasVGN,av[1]);
				break;

			case 110:  	// "THRSTREV",  // Effectiveness of the Thrust Reverser
				chThrustReverser=YsBound(atof(av[1]),0.0,1.0);
				res=YSOK;
				break;


			case 111:	// "GUNSIGHT",  // Lead Gun Sight
				res=FsGetBool(chLeadGunSight,av[1]);
				break;

			case 112:   //"HRDPOINT",  // Defining a hardpoint
				res=AddWeaponSlotByArgument(ac,av);
				break;
			case 113:   //"LOADWEPN",  // Load weapons
				AddWeapon(FsGetWeaponTypeByString(av[1]),atoi(av[2]));
				res=YSOK;
				break;
			case 114:   //"LMTBYHDP",  // Limit weapons by WeaponSlot definition.
				res=YSOK; // Now it is always TRUE.  FsGetBool(chLimitWeaponBySlot,av[1]);
				break;
			case 115:   //"UNLOADWP",  // Unload All Weapons (Excluding Guns, Smokes, and Flare)
				UnloadAllWeapon();
				res=YSOK;
				break;
			case 116:  //"INSTPANL",  // Draw an instrument panel instead of a hud.
				chHasHud=YSFALSE;
				chHasInstPanel=YSTRUE;
				if(ac>=2)
				{
					chInstPanelFileName.SetUTF8String(av[1]);
				}
				else
				{
					chInstPanelFileName.Set(L"");
				}

				if(YSTRUE!=chScreenCenterLocked)
				{
					chScreenCenter.Set(0.0,0.0);
				}

				res=YSOK;
				break;


			case 117: //"MACHNGN3",
				chNumGun=YsGreater(chNumGun,3);
				res=FsGetVec3(chGunPosition[2],ac-1,av+1);
				break;
			case 118: //"MACHNGN4",
				chNumGun=YsGreater(chNumGun,4);
				res=FsGetVec3(chGunPosition[3],ac-1,av+1);
				break;
			case 119: //"MACHNGN5",
				chNumGun=YsGreater(chNumGun,5);
				res=FsGetVec3(chGunPosition[4],ac-1,av+1);
				break;
			case 120: //"MACHNGN6",
				chNumGun=YsGreater(chNumGun,6);
				res=FsGetVec3(chGunPosition[5],ac-1,av+1);
				break;
			case 121: //"MACHNGN7",
				chNumGun=YsGreater(chNumGun,7);
				res=FsGetVec3(chGunPosition[6],ac-1,av+1);
				break;
			case 122: //"MACHNGN8",
				chNumGun=YsGreater(chNumGun,8);
				res=FsGetVec3(chGunPosition[7],ac-1,av+1);
				break;

			case 123: // "BOMINBAY",
				res=FsGetBool(chBombInBombBay,av[1]);
				break;
			case 124: // "BMBAYRCS"
				chBombBayRcs=atof(av[1]);
				res=YSOK;
				break;


			case 125: //	"INITAAMM",  // Mid-Range AAM
				// chMaxNumAIM120=YsGreater(chMaxNumAIM120,atoi(av[1]));  // Max must be set before SetNum
				SetNumWeapon(FSWEAPON_AIM120,atoi(av[1]));
				res=YSOK;
				break;
			case 126: // 	"MAXNAAMM",  // Max # Mid-Range AAM
				// chMaxNumAIM120=atoi(av[1]);
				res=YSOK;
				break;
			case 127: // 	"INITB250",  // 250lb Bomb
				// chMaxNumBomb250=YsGreater(chMaxNumBomb250,atoi(av[1]));
				SetNumWeapon(FSWEAPON_BOMB250,atoi(av[1]));
				res=YSOK;
				break;
			case 128: //    "MAXNB250",  // Max # 250lb Bomb
				// chMaxNumBomb250=atoi(av[1]);
				res=YSOK;
				break;

			case 129: // "GUNINTVL"   // Gun Interval
				chGunInterval=atof(av[1]);
				res=YSOK;
				break;

			case 130: // "NMTURRET",  // Number of turrets
				if(ac>=2)
				{
					int i,n;
					n=atoi(av[1]);
					chTurret.Set(n,NULL);
					staTurret.Set(n,NULL);
					for(i=0; i<n; i++)
					{
						chTurret[i].Initialize();
						staTurret[i].Initialize();
					}
					res=YSOK;
				}
				break;
			case 131: // "TURRETPO",  //   Number x y z h p b
				if(ac>=8)
				{
					turretId=atoi(av[1]);
					if(0<=turretId &&
					   turretId<chTurret.GetN() &&
					   FsGetVec3(chTurret[turretId].cen,ac-2,av+2)==YSOK &&
					   FsGetAtt3(chTurret[turretId].att,ac-5,av+5)==YSOK)
					{
						res=YSOK;
					}
				}
				break;
			case 132: // "TURRETPT",  //   Number MinPitch MaxPitch NeutralPitch
				if(ac>=5)
				{
					turretId=atoi(av[1]);
					if(0<=turretId &&
					   turretId<chTurret.GetN() &&
					   FsGetAngle(chTurret[turretId].pMin,av[2])==YSOK &&
					   FsGetAngle(chTurret[turretId].pMax,av[3])==YSOK &&
					   FsGetAngle(chTurret[turretId].pZero,av[4])==YSOK)
					{
						res=YSOK;
					}
				}
				break;
			case 133: // "TURRETHD",  //   Number MinHdg MaxHdg NeutralHdg
				if(ac>=5)
				{
					turretId=atoi(av[1]);
					if(0<=turretId &&
					   turretId<chTurret.GetN() &&
					   FsGetAngle(chTurret[turretId].hMin,av[2])==YSOK &&
					   FsGetAngle(chTurret[turretId].hMax,av[3])==YSOK &&
					   FsGetAngle(chTurret[turretId].hZero,av[4])==YSOK)
					{
						res=YSOK;
					}
				}
				break;
			case 134: // "TURRETAM",  //   Ammo(zero -> staGunBullet will be used)
				if(ac>=3)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						int n;
						n=atoi(av[2]);
						chTurret[turretId].maxNumGunBullet=n;
						staTurret[turretId].numBullet=n;
						res=YSOK;
					}
				}
				break;
			case 135: // "TURRETIV",  //   Number ShootingInterval
				if(ac>=3)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].shootInterval=atof(av[2]);
						res=YSOK;
					}
				}
				break;
			case 136: // "TURRETNM",  //   DNM Node Name
				if(ac>=3)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].dnmNodeName.Set(av[2]);
						res=YSOK;
					}
				}
				break;
			case 137: // "TURRETAR",  //   Anti Air Capable
				if(ac>=2)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						res=YSOK;
						chTurret[turretId].turretProp|=FSTURRETPROP_ANTIAIR;
						chHasAntiAirTurret=YSTRUE;
					}
				}
				break;
			case 138: // "TURRETGD",  //   Anti Ground Capable
				if(ac>=2)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						res=YSOK;
						chTurret[turretId].turretProp|=FSTURRETPROP_ANTIGND;
						chHasAntiGndTurret=YSTRUE;
					}
				}
				break;
			case 139: // "TURRETCT",  //  "PILOT" or "GUNNER"
				if(ac>=3)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						if(strcmp(av[2],"PILOT")==0 || strcmp(av[2],"pilot")==0 || strcmp(av[2],"Pilot")==0)
						{
							chTurret[turretId].controlledBy=FSTURRET_CTRL_BY_PILOT;
							chHasPilotControlledTurret=YSTRUE;
							res=YSOK;
						}
						else if(strcmp(av[2],"GUNNER")==0 || strcmp(av[2],"gunner")==0 || strcmp(av[2],"Gunner")==0)
						{
							chTurret[turretId].controlledBy=FSTURRET_CTRL_BY_GUNNER;
							chHasGunnerControlledTurret=YSTRUE;
							res=YSOK;
						}
					}
				}
				break;
			case 140: // "TURRETRG"
				if(ac>=3)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						res=FsGetLength(chTurret[turretId].range,av[2]);
					}
				}
				break;

			case 141: // "TURRETNH",  // DNM Node Name (Heading Rotation)
				if(ac>=3)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].dnmHdgNodeName.Set(av[2]);
						res=YSOK;
					}
				}
				break;
			case 142: // "TURRETNP",  // DNM Node Name (Pitch Rotation)
				if(ac>=3)
				{
					turretId=atoi(av[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].dnmPchNodeName.Set(av[2]);
						res=YSOK;
					}
				}
				break;
			case 143: // "SETCNTRL",  // Set Control eg. ILS TRIM:0.3 etc.
				res=YSOK;
				for(int i=1; i<ac; i++)
				{
					if(strncmp(av[i],"ILS:",4)==0)
					{
						staPendingVorYsfId[0]=atoi(av[i]+4);
					}
					else if(strncmp(av[i],"NAV1:",5)==0)
					{
						staPendingVorYsfId[0]=atoi(av[i]+5);
					}
					else if(strncmp(av[i],"NAV2:",5)==0)
					{
						staPendingVorYsfId[1]=atoi(av[i]+5);
					}
					else if(strncmp(av[i],"OBS1:",5)==0)
					{
						staObs[0]=atof(av[i]+5);
					}
					else if(strncmp(av[i],"OBS2:",5)==0)
					{
						staObs[1]=atof(av[i]+5);
					}
					else if(strncmp(av[i],"NDB:",4)==0)
					{
						staPendingNdbYsfId=atoi(av[i]+4);
					}
					else if(strncmp(av[i],"TRIM:",5)==0)
					{
						SetElvTrim(atof(av[i]+5));
					}
					else
					{
						res=YSERR;
					}
				}
				break;
			case 144: // "EXCAMERA",  // Extra Camera
				if(ac>=9)
				{
					FsAdditionalViewpoint view;
					view.name.Set(av[1]);
					if(FsGetVec3(view.pos,ac-2,av+2)==YSOK &&
					   FsGetAtt3(view.att,ac-5,av+5)==YSOK)
					{
						view.vpType=FS_ADVW_INSIDE;
						view.showHudIfAvailable=YSTRUE;
						view.showInstPanelIfAvailable=YSTRUE;
						res=YSOK;

						for(int i=8; i<ac; ++i)
						{
							if(0==strcmp(av[i],"INSIDE"))
							{
								view.vpType=FS_ADVW_INSIDE;
								res=YSOK;
							}
							else if(0==strcmp(av[i],"OUTSIDE"))
							{
								view.vpType=FS_ADVW_OUTSIDE;
								res=YSOK;
							}
							else if(0==strcmp(av[i],"CABIN"))
							{
								view.vpType=FS_ADVW_CABIN;
								res=YSOK;
							}
							else if(0==strcmp(av[i],"NOHUD"))
							{
								view.showHudIfAvailable=YSFALSE;
							}
							else if(0==strcmp(av[i],"NOINSTPANEL"))
							{
								view.showInstPanelIfAvailable=YSFALSE;
							}
						}
						chExtraView.Append(view);
					}
				}
				break;
			case 145: // "NMACHNGN",  // Number of machine guns.
				if(ac>=2)
				{
					chNumGun=atoi(av[1]);
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 146: // "SMOKECOL"
				if(ac>=5)
				{
					const int r=atoi(av[2]);
					const int g=atoi(av[3]);
					const int b=atoi(av[4]);

					if(0==strcmp(av[1],"ALL"))
					{
						for(auto &smkGen : chSmokeGenerator)
						{
							smkGen.defCol.SetIntRGB(r,g,b);
						}
						for(auto &smkGen : staSmokeGenerator)
						{
							smkGen.col.SetIntRGB(r,g,b);
						}
					}
					else
					{
						const int smkIdx=atoi(av[1]);
						if(0<=smkIdx && smkIdx<MaxNumSmokeGenerator)
						{
							chSmokeGenerator[smkIdx].defCol.SetIntRGB(r,g,b);
							staSmokeGenerator[smkIdx].col.SetIntRGB(r,g,b);
						}
					}
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 147: // "SUBSTNAM",  // Substitute airplane (In case the airplane was not installed)
				if(ac>=2)
				{
					chSubstIdName.Set(av[1]);
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;

			case 148: // "ISPNLPOS",  // Instrument Panel Position
				if(YSOK==FsGetVec3(ch3dInstPanelPos,ac-1,av+1))
				{
					ch3dInstPanelPosSet=YSTRUE;
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 149: // "ISPNLSCL",  // Instrument Panel Scaling
				if(ac>=2)
				{
					ch3dInstPanelScale=atof(av[1]);
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 150: //"ISPNLHUD",  // Use both inst panel and HUD
				if(ac>=2)
				{
					res=FsGetBool(chUseBothHudAndInstPanel,av[1]);
					if(YSTRUE==chUseBothHudAndInstPanel)
					{
						if(YSTRUE!=chScreenCenterLocked)
						{
							chScreenCenter.Set(0.0,-0.33333333);
						}
					}
				}
				else
				{
					res=YSERR;
				}
				break;
			case 151: // "COCKPITA",  // Neutral Head Direction
				if(ac>=4)
				{
					res=FsGetAtt3(chNeutralHeadAttitude,ac-1,av+1);
				}
				else
				{
					res=YSERR;
				}
				break;
			case 152: // "SCRNCNTR",  // Screen center (Relative.  (-1.0,-1.0)-(1.0,1.0)
				if(ac>=3)
				{
					chScreenCenter.Set(atof(av[1]),atof(av[2]));
					chScreenCenterLocked=YSTRUE;
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 153: // "ISPNLATT",  // Instrument Panel Orientation
				if(YSOK==FsGetAtt3(ch3dInstPanelAtt,ac-1,av+1))
				{
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 154: // "MAXNMFLR",  // Maximum number of flare
				if(ac>=2)
				{
					chMaxNumFlare=atoi(av[1]);
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 155: // "FLAPPOSI",  // Flap position
				if(ac>=2)
				{
					const double flapPos=YsBound(atof(av[1]),0.0,1.0);
					if(YSTRUE!=chFlapPositionSet)
					{
						chFlapPositionSet=YSTRUE;
						chFlapPosition.Set(0,NULL);
					}
					chFlapPosition.Append(flapPos);
					res=YSOK;
				}
				break;
			case 156: // "FLAREPOS",  // Flare Dispenser Position and Dispensing Speed
				if(ac>=4)
				{
					res=YSOK;
					if(MaxNumFlareDispenser>chNumFlareDispenser)
					{
						res=FsGetVec3(chFlareDispenser[chNumFlareDispenser],ac-1,av+1);
						if(ac>=7 && YSOK==res)
						{
							res=FsGetVec3(chFlareDispensingVel[chNumFlareDispenser],ac-4,av+4);
						}
						chNumFlareDispenser++;
					}
				}
				else
				{
					res=YSERR;
				}
				break;
			case 157: // "INITAAAM",  // Initialize AIM9X
				SetNumWeapon(FSWEAPON_AIM9X,atoi(av[1]));
				res=YSOK;
				break;
			case 158: // "INITHDBM",  // Initialize High-Drag bomb
				SetNumWeapon(FSWEAPON_BOMB500HD,atoi(av[1]));
				res=YSOK;
				break;
			case 159: // "ULOADAAM",  // Unload all AAMs
				SetNumWeapon(FSWEAPON_AIM9,0);
				SetNumWeapon(FSWEAPON_AIM9X,0);
				SetNumWeapon(FSWEAPON_AIM120,0);
				res=YSOK;
				break;
			case 160: // "ULOADAGM",  // Unload all AGMs
				SetNumWeapon(FSWEAPON_AGM65,0);
				res=YSOK;
				break;
			case 161: // "ULOADBOM",  // Unload all Bombs
				SetNumWeapon(FSWEAPON_BOMB,0);
				SetNumWeapon(FSWEAPON_BOMB250,0);
				SetNumWeapon(FSWEAPON_BOMB500HD,0);
				res=YSOK;
				break;
			case 162: // "ULOADFLR",  // Unload all Flare
				SetNumWeapon(FSWEAPON_FLARE,0);
				SetNumWeapon(FSWEAPON_FLARE_INTERNAL,0);
				res=YSOK;
				break;
			case 163: // "ULOADGUN",  // Unload all Gun
				SetNumWeapon(FSWEAPON_GUN,0);
				res=YSOK;
				break;
			case 164: // "ULOADRKT"
				SetNumWeapon(FSWEAPON_ROCKET,0);
				res=YSOK;
				break;
			case 165: // "LOOKOFST",  // Look-at Offset
				res=FsGetVec3(chLookAt,ac-1,av+1);
				break;
			case 166: // "WPNSHAPE",  // Weapon-shape override
				if(4<=ac)
				{
					const int wpnType=(int)FsGetWeaponTypeByString(av[1]);
					if(0<=wpnType && wpnType<=FSWEAPON_NUMWEAPONTYPE)
					{
						if(0==strcmp(av[2],"STATIC"))
						{
							chWeaponShapeFileStatic[wpnType].SetUTF8String(av[3]);
							res=YSOK;
						}
						else if(0==strcmp(av[2],"FLYING"))
						{
							chWeaponShapeFileFlying[wpnType].SetUTF8String(av[3]);
							res=YSOK;
						}
					}
				}
				break;
			case 167: // "GEARHORN",  // Landing-gear warning horn
				if(2<=ac)
				{
					res=FsGetBool(chHasGearHorn,av[1]);
				}
				break;
			case 168: // "STALHORN",  // Stall-warning horn
				if(2<=ac)
				{
					res=FsGetBool(chHasStallHorn,av[1]);
				}
				break;

			case 169: // "CKPITIST",  // To Make inst panel available in only one of EXCAMERAs, it can be hidden in the default cockpit view.
				if(2<=ac)
				{
					res=FsGetBool(chShowInstPanelInCockpit,av[1]);
				}
				break;
			case 170: // "CKPITHUD",  // To Make HUD available in only one of EXCAMERAs, it can be hidden in the default cockpit view.
				if(2<=ac)
				{
					res=FsGetBool(chShowHudInCockpit,av[1]);
				}
				break;
			case 171: // "MALFUNCT",  // Malfunction
				if(2<=ac)
				{
					if(0==strcmp("AUTO",av[1]))
					{
						staAutoPilotInop=YSTRUE;
					}
					else if(0==strcmp("FLAP",av[1]))
					{
						staFlapInop=YSTRUE;
					}
					else if(0==strcmp("RADAR",av[1]))
					{
						staRadarInop=YSTRUE;
					}
					else if(0==strcmp("VOR",av[1]))
					{
						staVorInop=YSTRUE;
					}
					else if(0==strcmp("ADF",av[1]))
					{
						staAdfInop=YSTRUE;
					}
					else if(0==strcmp("AIRSPEED",av[1]))
					{
						staAirspeedInop=YSTRUE;
						if(4<=ac)
						{
							FsGetLength(staAirspeedStuckAltitude,av[2]);
							FsGetSpeed(staAirspeedStuckAirspeed,av[3]);
						}
					}
					else if(0==strcmp("ALTIMETER",av[1]))
					{
						staAltimeterInop=YSTRUE;
						if(3<=ac)
						{
							FsGetLength(staAltimeterStuckAltitude,av[2]);
						}
					}
					else if(0==strcmp("VSI",av[1]))
					{
						staVSIInop=YSTRUE;
					}
					else if(0==strcmp("ATTITUDE",av[1]))
					{
						staAttitudeIndicatorInop=YSTRUE;
						if(5<=ac)
						{
							FsGetAtt3(staAttitudeIndicatorStuckAttitude,ac-2,av+2);
						}
					}
					else if(0==strcmp("POWER",av[1]) && 3<=ac)
					{
						staPowerLoss=YSTRUE;
						staEngineOutputCap=atof(av[2]);
					}
					else if(0==strcmp("HUDFLICKER",av[1]))
					{
						staHUDFlicker=YSTRUE;
					}
					res=YSOK;
				}
				break;
			case 172:  // "REPAIRAL",  // Repair all
				ClearMalfunction();
				break;
			case 173:  // "REPAIRFN",  // Repair functionality
				if(2<=ac)
				{
					if(0==strcmp("AUTO",av[1]))
					{
						staAutoPilotInop=YSFALSE;
					}
					else if(0==strcmp("FLAP",av[1]))
					{
						staFlapInop=YSFALSE;
					}
					else if(0==strcmp("RADAR",av[1]))
					{
						staRadarInop=YSFALSE;
					}
					else if(0==strcmp("VOR",av[1]))
					{
						staVorInop=YSFALSE;
					}
					else if(0==strcmp("ADF",av[1]))
					{
						staAdfInop=YSFALSE;
					}
					else if(0==strcmp("AIRSPEED",av[1]))
					{
						staAirspeedInop=YSFALSE;
					}
					else if(0==strcmp("ALTIMETER",av[1]))
					{
						staAltimeterInop=YSFALSE;
					}
					else if(0==strcmp("VSI",av[1]))
					{
						staVSIInop=YSFALSE;
					}
					else if(0==strcmp("ATTITUDE",av[1]))
					{
						staAttitudeIndicatorInop=YSFALSE;
					}
					else if(0==strcmp("POWER",av[1]))
					{
						staPowerLoss=YSFALSE;
						staEngineOutputCap=1.0;
					}
					else if(0==strcmp("HUDFLICKER",av[1]))
					{
						staHUDFlicker=YSFALSE;
					}
					res=YSOK;
				}
				break;
			case 174: // "NOLDGFLR"
				if(2<=ac)
				{
					res=FsGetBool(chNoLandingFlare,av[1]);
				}
				break;
			case 175: // "NREALPRP",  // Number of (real) propeller engines
				if(2<=ac)
				{
					YsString raw(in);
					raw.DropSingleLineComment("#");
					raw.DeleteTailSpace();
					raw.PinchConsecutiveSpace();
					MakeShortFormat(chEnginePropCmd.New(),raw,YSFLIGHT_NETVERSION);

					const int nRealPropEngine=atoi(av[1]);
					chRealProp.Set(nRealPropEngine,NULL);
					for(auto &engine : chRealProp)
					{
						engine.Initialize();
					}
					staPropLever.Set(nRealPropEngine,NULL);
					for(auto &propLever : staPropLever)
					{
						propLever=1.0; // Full forward by default.
					}
					res=YSOK;
				}
				break;
			case 176: // "REALPROP"
				if(2<=ac)
				{
					YsString raw(in);
					raw.DropSingleLineComment("#");
					raw.DeleteTailSpace();
					raw.PinchConsecutiveSpace();
					MakeShortFormat(chEnginePropCmd.New(),raw,YSFLIGHT_NETVERSION);
					chEnginePropCmd.Last()=FsPropellerEngine::MakeShortFormat(chEnginePropCmd.Last());

					const int engineIdx=atoi(av[1]);
					if(YSTRUE==chRealProp.IsInRange(engineIdx))
					{
						res=chRealProp[engineIdx].SendCommand(ac-2,av+2);
					}
				}
				break;
			case 177: // "TIREFRIC",  // Tire friction coefficient
				if(2<=ac)
				{
					chTireFrictionConst=atof(av[1]);
					res=YSOK;
				}
				break;
			case 178: // "PSTMSPD1",   // Maximum speed that the direct attitude control is fully effective.
				if(2<=ac)
				{
					res=FsGetSpeed(chDirectAttitudeControlSpeed1,av[1]);
				}
				break;
			case 179: // "PSTMSPD2",   // Speed at which the direct attitude control becomes ineffective.
				if(2<=ac)
				{
					res=FsGetSpeed(chDirectAttitudeControlSpeed2,av[1]);
				}
				break;
			case 180: // "PSTMPWR1",   // Required power setting for direct attitude control
				if(2<=ac)
				{
					chDirectAttitudeControlReqThr1=atof(av[1]);
					res=YSOK;
				}
				break;
			case 181: // "PSTMPWR2",   // Required power setting for direct attitude control
				if(2<=ac)
				{
					chDirectAttitudeControlReqThr2=atof(av[1]);
					res=YSOK;
				}
				break;
			case 182: // "MAXCDAOA",
				if(2<=ac)
				{
					res=FsGetAngle(chMaxCdAOA,av[1]);
				}
				break;
			case 183: // "FLATCLR1",
				if(2<=ac)
				{
					res=FsGetAngle(chFlatClRange1,av[1]);
				}
				break;
			case 184: // "FLATCLR2",
				if(2<=ac)
				{
					res=FsGetAngle(chFlatClRange2,av[1]);
				}
				break;
			case 185: // "CLDECAY1",
				if(2<=ac)
				{
					res=FsGetAngle(chClDecay1,av[1]);
				}
				break;
			case 186: // "CLDECAY2",
				if(2<=ac)
				{
					res=FsGetAngle(chClDecay2,av[1]);
				}
				break;
			case 187: // 	"AIRSTATE",  // I'm shocked that I didn't have it yet.
				if(2<=ac)
				{
					staState=StringToState(av[1]);
					res=YSOK;
				}
				break;
			case 188: // 	"INITZOOM",  // Initial zoom factor
				if(2<=ac)
				{
					chDefZoom=atof(av[1]);  // 2018/11/24 Close shave!  I was writing [i] here.
					res=YSOK;
				}
				break;
			}
			if(res!=YSOK)
			{
				fsStderr.Printf("Error in parameter:%s\n",in);
			}
			return res;
		}
		else
		{
			fsStderr.Printf("Error in keyword:%s\n",in);
			return YSERR;
		}
	}
	// Ignore Blank line
	return YSOK;
}

YSRESULT FsAirplaneProperty::AddWeaponSlotByArgument(int ac,char *av[])
{// HRDPOINT 5 -1 0 AIM9 BOMB AGM65 RKT*19
	YsVec3 pos;
	if(YSOK==FsGetVec3(pos,ac-1,av+1))
	{
		FsWeaponSlot newSlot;
		YSBOOL isExternal=YSTRUE;
		int i;

		for(i=4; i<ac; i++)
		{
			if('$'==av[i][0])
			{
				if(0==strcmp(av[i],"$INTERNAL"))
				{
					isExternal=YSFALSE;
				}
			}
			else
			{
				const FSWEAPONTYPE wpnType=FsGetWeaponTypeByString(av[i]);
				newSlot.nLoad[wpnType]=FsGetDefaultWeaponLoadingUnit(wpnType);
				newSlot.nSubLoad[wpnType]=FsGetDefaultSubUnitPerLoadingUnit(wpnType);

				YsString weaponCode(av[i]);
				for(int j=0; j<weaponCode.Strlen(); j++)
				{
					if(weaponCode[j]=='*')
					{
						newSlot.nLoad[wpnType]=atoi(weaponCode.Txt()+j+1);
					}
					else if(weaponCode[j]=='&')
					{
						newSlot.nSubLoad[wpnType]=atoi(weaponCode.Txt()+j+1);
					}
				}
			}
		}
		AddWeaponSlot(pos,newSlot.nLoad,newSlot.nSubLoad,isExternal);
		return YSOK;
	}

	return YSERR;
}

void FsAirplaneProperty::AutoCalculate(void)
{
	if(chIsJet!=YSTRUE)
	{
		chPropK=-(chThrMil*chPropEfficiency)/(chPropV0*chPropV0);
	}

	if(chClass==FSCL_AIRPLANE)
	{
		double m,v,rho,s;
		double t;

		// Cl Formula
		m=chCleanWeight+chMaxFuelLoad;
		v=refSpdCruise;
		rho=FsGetAirDensity(refAltCruise);
		s=chWingArea;
		chClZero=m*FsGravityConst/(0.5*rho*v*v*s);

		double chClLand;
		rho=FsGetAirDensity(0.0);
		v=refSpdLanding;
		chClLand=m*FsGravityConst/(0.5*rho*v*v*s)/(1.0+chClFlap)/(1.0+chClVgw);
		chClSlope=(chClLand-chClZero)/refAOALanding;

		if(chClSlope<0.0)
		{
			fsStderr.Printf("Negative Cl Slope!!\n");
		}

		//Cd Formula
		const double cruiseThrust=GetConvergentThrust(refThrCruise,refAltCruise,refSpdCruise,YSFALSE);
		const double vmaxThrust=GetConvergentThrust(1.0,refAltCruise,chVMax,YSTRUE);
		const double landingThrust=GetConvergentThrust(refThrLanding,0.0,refSpdLanding,YSFALSE);

		t=cruiseThrust;
		v=refSpdCruise;
		rho=FsGetAirDensity(refAltCruise);
		chCdZero=t/(0.5*rho*v*v*s);

		double chCdLand;
		t=landingThrust;
		v=refSpdLanding;
		rho=FsGetAirDensity(0.0);
		chCdLand=t/(0.5*rho*v*v*s)/(1.0+chCdFlap)/(1.0+chCdVgw)/(1.0+chCdGear);
		chCdConst=(chCdLand-chCdZero)/(refAOALanding*refAOALanding);

		if(chCdConst<0.0)
		{
			fsStderr.Printf("%s Negative Cd Const (%lf %lf)",chIdName.Txt(),chCdZero,chCdLand);
		}

		if(chCdZero<0.0)
		{
			fsStderr.Printf("%s Negative Cd at alpha=0 (%lf %lf)",chIdName.Txt(),chCdZero,chCdLand);
		}


		t=vmaxThrust;
		v=chVMax;
		rho=FsGetAirDensity(refAltCruise);
		chCdZeroMax=t/(0.5*rho*v*v*s);

		if(chCdZeroMax<chCdZero)  // Don't reduce drag as the speed increases.  2003/02/08
		{
			chCdZeroMax=chCdZero;
		}

		if(chVMax<refSpdCruise)
		{
			fsStderr.Printf("Max Speed is slower than Cruising Speed!!\n");
		}

		if(chVCritical==0.0)
		{
			chVCritical=refSpdCruise+(chVMax-refSpdCruise)*0.3+1.0;
		}

		v=refSpdLanding;
		chBrakeConst=(v*v*m)/(2.0*refLNGRunway);
	}
	else if(chClass==FSCL_HELICOPTER)
	{
/*
		// Calculate propK
		// Assume that the propeller thrust produces g*weight+maxLoad+maxFuel at v=0
		double maxWeight,maxForce,forceV0;
		maxWeight=chCleanWeight+chMaxFuelLoad+chMaxPayload;
		maxForce=maxWeight*FsGravityConst;
		forceV0=chThrMil*chPropEfficiency/chPropV0;

		chPropK=(forceV0-maxForce)/chPropV0;
		if(chPropK>0.0)
		{
			fsStderr.Printf("Positive chPropK.!\n");
			chPropK=0.0;
		}
*/

		// PropK must be computed by this point.


		// Assume -30 pitch for max speed
		double vThr;  // Velocity component of thrust direction;
		double T,Th;
		double pitch,rho;

		rho=FsGetAirDensity(refAltCruise);

		pitch=YsPi/18.0;
		vThr=chVMax*sin(pitch);
		T=GetThrust(refThrCruise,refAltCruise,vThr,YSFALSE);
		Th=T*sin(pitch);

		chCdZero=Th/(0.5*rho*chVMax*chVMax*chWingArea);
		chCdZeroMax=chCdZero;
		chCdConst=0.0;

		chClSlope=0.0;
		chClZero=0.0;

		double m,v;
		v=30.0;
		m=chCleanWeight+chMaxFuelLoad;
		chBrakeConst=(v*v*m)/(2.0*refLNGRunway);
	}
}

void FsAirplaneProperty::LoadFuel(void)
{
	staFuelLoad=chMaxFuelLoad;
}

void FsAirplaneProperty::LoadFuel(const double fuel)
{
	staFuelLoad=fuel;
}

const double &FsAirplaneProperty::GetFuelLeft(void) const
{
	return staFuelLoad;
}

const double &FsAirplaneProperty::GetMaxFuelLoad(void) const
{
	return chMaxFuelLoad;
}

double FsAirplaneProperty::GetG(void) const
{
	YsVec3 relForce;
	double weight,force;

	weight=GetTotalWeight();
	staInverseMatrix.Mul(relForce,staTotalAerodynamicForce,0.0);
	force=relForce.y();
	return force/weight/FsGravityConst;
}

double FsAirplaneProperty::GetLateralForce(void) const
{
	YsVec3 trueTotalForce;
	trueTotalForce=staTotalAerodynamicForce+staTotalGravityForce;
	staAttitude.MulInverse(trueTotalForce,trueTotalForce);

	YsVec3 relVel;
	double ssa;
	staAttitude.MulInverse(relVel,staVelocity);
	ssa=atan2(-relVel.x(),relVel.z());
	trueTotalForce.RotateXZ(-ssa);
// printf("LF:%.2lf  A:%.2lf\n",trueTotalForce.x(),trueTotalForce.x()/GetTotalWeight());
	return trueTotalForce.x();
}

const double &FsAirplaneProperty::GetAOA(void) const
{
	return staAOA;
}

const double &FsAirplaneProperty::GetSSA(void) const
{
	return staSSA;
}

double FsAirplaneProperty::GetMach(void) const
{
	double machOne;
	machOne=FsGetMachOne(staPosition.y());
	return staV/machOne;
}

double FsAirplaneProperty::GetCleanWeight(void) const
{
	return chCleanWeight+staFuelLoad;
}

double FsAirplaneProperty::GetPayloadWeight(void) const
{
	// AIM-9 -> about 250 kg
	// AGM-65-> about 300 kg;
	// Mk82  -> 500lb = about 250 kg;
	double weight=staPayload
	      +double(GetNumWeapon(FSWEAPON_AIM9))*90.0
	      +double(GetNumWeapon(FSWEAPON_AIM9X))*90.0
	      +double(GetNumWeapon(FSWEAPON_AIM120))*150.0
	      +double(GetNumWeapon(FSWEAPON_AGM65))*300.0
	      +double(GetNumWeapon(FSWEAPON_BOMB))*250.0
	      +double(GetNumWeapon(FSWEAPON_BOMB250))*120.0
	      +double(GetNumWeapon(FSWEAPON_BOMB500HD))*250.0
	      +double(GetNumWeapon(FSWEAPON_ROCKET))*10.0
	      +staSmokeOil;

	int i;
	for(i=0; i<staWeaponSlot.GetN(); i++)
	{
		if(staWeaponSlot[i].wpnType==FSWEAPON_FLARE &&
		   0<staWeaponSlot[i].nContainerLoaded) // Flare Pod
		{
			weight+=150.0;
		}
		else if(staWeaponSlot[i].wpnType==FSWEAPON_FUELTANK &&
		        0<staWeaponSlot[i].nLoaded)
		{
			weight+=150.0+staWeaponSlot[i].fuelLoaded;
		}
	}

	return weight;
}

double FsAirplaneProperty::GetTotalWeight(void) const
{
	return GetCleanWeight()+GetPayloadWeight();
}

const YsAtt3 &FsAirplaneProperty::GetIndicatedAttitude(void) const
{
	if(YSTRUE!=staAttitudeIndicatorInop)
	{
		return staAttitude;
	}
	else
	{
		return staAttitudeIndicatorStuckAttitude;
	}
}

const YsVec2 &FsAirplaneProperty::GetScreenCenter(void) const
{
	return chScreenCenter;
}

const YsAtt3 &FsAirplaneProperty::GetNeutralHeadDirection(void) const
{
	return chNeutralHeadAttitude;
}

const YsVec3 &FsAirplaneProperty::GetInstPanelPos(YsVec3 &instPanelPos) const
{
	if(YSTRUE==ch3dInstPanelPosSet)
	{
		instPanelPos=ch3dInstPanelPos;
	}
	else
	{
		instPanelPos=chCockpit+YsVec3(0.0,0.0,1.2);
	}
	return instPanelPos;
}

const YsAtt3 &FsAirplaneProperty::GetInstPanelAtt(void) const
{
	return ch3dInstPanelAtt;
}

const double &FsAirplaneProperty::GetInstPanelScaling(void) const
{
	return ch3dInstPanelScale;
}

YSBOOL FsAirplaneProperty::HasHud(void) const
{
	if(YSTRUE==chHasHud || YSTRUE==	chUseBothHudAndInstPanel)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::HasInstPanel(void) const
{
	return chHasInstPanel;
}

const wchar_t *FsAirplaneProperty::GetInstPanelFileName(void) const
{
	return chInstPanelFileName;
}

const int FsAirplaneProperty::GetNumTire(void) const
{
	return 3; // Currently fixed.  Wheel, main gear L, and main gear R.
}

const YsVec3 &FsAirplaneProperty::GetTirePosition(int id) const
{
	switch(id)
	{
	case 0:
		return chMainGearL;
	case 1:
		return chMainGearR;
	default:
	case 2:
		return chWheel;
	}
}

void FsAirplaneProperty::SetElevationAtTire(int id,const YsVec3 &foot)
{
	switch(id)
	{
	case 0:
		staFootAtMainGearL=foot;
		break;
	case 1:
		staFootAtMainGearR=foot;
		break;
	default:
	case 2:
		staFootAtWheel=foot;
		break;
	}
}

void FsAirplaneProperty::GetTransformedRearGearPosition(YsVec3 &pos) const
{
	if(chMainGearL.z()<chMainGearR.z() && chMainGearL.z()<chWheel.z())
	{
		pos=chMainGearL;
	}
	else if(chMainGearR.z()<chWheel.z())
	{
		pos=chMainGearR;
	}
	else
	{
		pos=chWheel;
	}
	pos.SetX(0.0);
	pos=staMatrix*pos;
}

void FsAirplaneProperty::GetTransformedFrontGearPosition(YsVec3 &pos) const
{
	if(chMainGearL.z()>chMainGearR.z() && chMainGearL.z()>chWheel.z())
	{
		pos=chMainGearL;
	}
	else if(chMainGearR.z()>chWheel.z())
	{
		pos=chMainGearR;
	}
	else
	{
		pos=chWheel;
	}
	pos.SetX(0.0);
	pos=staMatrix*pos;
}

void FsAirplaneProperty::GetTransformedArrestingHookPosition(YsVec3 &hook) const
{
	hook=staMatrix*chArrestingHook;
}

double FsAirplaneProperty::GetGroundStandingHeight(void) const
{
	double h[3];
	h[0]=-chMainGearL.y();
	h[1]=-chMainGearR.y();
	h[2]=-chWheel.y();
	return YsGreater(YsGreater(h[0],h[1]),h[2]);
}

int FsAirplaneProperty::GetNumSmokeGenerator(void) const
{
	if(chNumSmokeGenerator>0)
	{
		return chNumSmokeGenerator;
	}
	return 0;
}

void FsAirplaneProperty::GetSmokeGeneratorPosition(YsVec3 &smk,int smkId) const
{
	smk=chSmokeGenerator[smkId].pos;
}

void FsAirplaneProperty::GetVaporPosition(YsVec3 &vap) const
{
	if(chHasVGW==YSTRUE)
	{
		vap=chVapor1*staVgw+chVapor0*(1.0-staVgw);
	}
	else
	{
		vap=chVapor0;
	}
}

YSBOOL FsAirplaneProperty::CheckHasJustTouchDown(void) const
{
	if(staPState!=FSGROUND && staPState!=FSGROUNDSTATIC && staPState!=FSOVERRUN && IsOnGround()==YSTRUE)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::CheckSafeTouchDown(FSDIEDOF &diedOf) const
{
	if(staGear<0.9)
	{
		diedOf=FSDIEDOF_LANDINGGEARNOTEXTENDED;
		return YSFALSE;
	}
	if(staTDAtt.b()<YsDegToRad(-45.0) || YsDegToRad(45.0)<staTDAtt.b())
	{
		diedOf=FSDIEDOF_BADBANKANGLE;
		return YSFALSE;
	}
	if(staTDAtt.p()<YsDegToRad(-10.0) || YsDegToRad(45.0)<staTDAtt.p())
	{
		diedOf=FSDIEDOF_BADPITCHANGLE;
		return YSFALSE;
	}
	return YSTRUE;
}

YSBOOL FsAirplaneProperty::CheckHasJustAirborne(void) const
{
	if(staPState==FSGROUND && staState==FSFLYING)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::IsTrailingVapor(void) const
{
	double g;
	g=GetG();
	if(g<-3.0 || 4.0<g)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirplaneProperty::IsTrailingSmoke(int smkIdx) const
{
	if(YsTolerance<=staSmokeOil && 0!=(ctlSmokeSelector&(1<<smkIdx)))
	{
		if((staWoc==FSWEAPON_SMOKE && ctlFireWeaponButton==YSTRUE) || ctlSmokeButton==YSTRUE)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

void FsAirplaneProperty::WriteFlightRecord(FsFlightRecord &rec) const
{
	rec.pos=staPosition;
	rec.h=(float)staAttitude.h();
	rec.p=(float)staAttitude.p();
	rec.b=(float)staAttitude.b();
	rec.g=(float)GetG();

	rec.state=(char)staState;
	rec.vgw=(unsigned char)(YsBound(int(staVgw*255.0),0,255));
	rec.spoiler=(unsigned char)(YsBound(int(staSpoiler*255.0),0,255));
	rec.gear=(unsigned char)(YsBound(int(staGear*255.0),0,255));
	rec.flap=(unsigned char)(YsBound(int(staFlap*255.0),0,255));
	rec.brake=(unsigned char)(YsBound(int(staBrake*255.0),0,255));
	rec.dmgTolerance=(unsigned char)staDamageTolerance;

	rec.flags=0;
	if(staAb==YSTRUE)
	{
		rec.flags|=FsFlightRecord::FLAGS_AB;
	}
	// if(staIls==YSTRUE)
	// {
	// 	rec.flags|=FsFlightRecord::FLAGS_ILS;
	// }
	if(staVectorMarker==YSTRUE)
	{
		rec.flags|=FsFlightRecord::FLAGS_VECTOR;
	}
	if(staOutOfRunway==YSTRUE)
	{
		rec.flags|=FsFlightRecord::FLAGS_OUTOFRUNWAY;
	}
	if(YSTRUE==staBeacon)
	{
		rec.flags|=FsFlightRecord::FLAGS_BEACON;
	}
	if(YSTRUE==staNavLight)
	{
		rec.flags|=FsFlightRecord::FLAGS_NAVLIGHT;
	}
	if(YSTRUE==staStrobe)
	{
		rec.flags|=FsFlightRecord::FLAGS_STROBE;
	}
	if(YSTRUE==staLandingLight)
	{
		rec.flags|=FsFlightRecord::FLAGS_LANDINGLIGHT;
	}



	rec.thr=YsBound(int(staThrottle*99),0,99);
	rec.elv=YsBound(int(ctlElevator*99),-99,99);
	rec.ail=YsBound(int(ctlAileron*99),-99,99);
	rec.rud=YsBound(int(GetRudder()*99),-99,99);
	rec.elvTrim=YsBound(int(ctlElvTrim*99),-99,99);

	rec.smoke=0;
	rec.vapor=0;
	for(int smkIdx=0; smkIdx<MaxNumSmokeGenerator; ++smkIdx)
	{
		if(IsTrailingSmoke(smkIdx)==YSTRUE)
		{
			rec.smoke|=(1<<smkIdx);
		}
	}
	if(IsTrailingVapor()==YSTRUE)
	{
		rec.vapor=1;
	}

	rec.thrVector=YsBound((int)(ctlThrVec*99),0,99);
	rec.bombBay=YsBound((int)(staBombBayDoor*99),0,99);
	rec.thrReverser=YsBound((int)(staThrRev*99.0),0,99);

	if(staTurret.GetN()>0)
	{
		rec.turret.Alloc((int)staTurret.GetN());
		if(rec.turret.GetN()==staTurret.GetN())
		{
			int i;
			for(i=0; i<staTurret.GetN(); i++)
			{
				rec.turret[i].h=(float)staTurret[i].h;
				rec.turret[i].p=(float)staTurret[i].p;
				rec.turret[i].turretState=staTurret[i].turretState;
			}
		}
	}
}

void FsAirplaneProperty::ReadbackFlightRecord(
    const FsFlightRecord &rec,
    const double &dt,const double &velocity,
    const YsAtt3 &vRot,const YsVec3 &totalForce,const YsVec3 &unitVelocityVector)
{
	BeforeMove();

	if(FSFLIGHTSTATE(rec.state)==FSGROUND)
	{
		YsVec3 newPos=rec.pos;
		newPos-=staPosition;
		staAttitude.MulInverse(newPos,newPos);
		staWheelAngle-=newPos.z()/chWheelRadius;
	}

	staPosition=rec.pos;
	staAttitude.Set(rec.h,rec.p,rec.b);
	RemakeMatrix();

	SetState(FSFLIGHTSTATE(rec.state),FSDIEDOF_NULL);
	staVgw=double(rec.vgw)/255.0;
	staSpoiler=double(rec.spoiler)/255.0;
	staGear=double(rec.gear)/255.0;
	staFlap=double(rec.flap)/255.0;
	staBrake=double(rec.brake)/255.0;
	staDamageTolerance=rec.dmgTolerance;
	staAb=((rec.flags&FsFlightRecord::FLAGS_AB)!=0 ? YSTRUE : YSFALSE);
	// staIls=((rec.flags&FsFlightRecord::FLAGS_ILS)!=0 ? YSTRUE : YSFALSE);
	staVectorMarker=((rec.flags&FsFlightRecord::FLAGS_VECTOR)!=0 ? YSTRUE : YSFALSE);
	staOutOfRunway=((rec.flags&FsFlightRecord::FLAGS_OUTOFRUNWAY)!=0 ? YSTRUE : YSFALSE);

	staBeacon=((rec.flags&FsFlightRecord::FLAGS_BEACON)!=0 ? YSTRUE : YSFALSE);
	staNavLight=((rec.flags&FsFlightRecord::FLAGS_NAVLIGHT)!=0 ? YSTRUE : YSFALSE);
	staStrobe=((rec.flags&FsFlightRecord::FLAGS_STROBE)!=0 ?  YSTRUE : YSFALSE);
	staLandingLight=((rec.flags&FsFlightRecord::FLAGS_LANDINGLIGHT)!=0 ?  YSTRUE : YSFALSE);

	if(unitVelocityVector.GetSquareLength()>=YsSqr(YsTolerance))
	{
		staVelocity=unitVelocityVector*velocity;
	}
	else
	{
		staVelocity.Set(0.0,0.0,velocity);
		staAttitude.Mul(staVelocity,staVelocity);  // staVelocity=staAttitude.GetMatrix()*staVelocity;
	}


	// 2011/01/31 >>
	YsVec3 relVec;
	staInverseMatrix.Mul(relVec,staVelocity,0.0);
	if(relVec.GetSquareLength()>=FsMinimumAirspeed)
	{
		staAOA=atan2(-relVec.y(),relVec.z());
		staSSA=atan2( relVec.x(),relVec.z());
	}
	else
	{
		staAOA=0.0;
		staSSA=0.0;
	}
	// 2011/01/31 <<


//	staTotalForce.Set(0.0,0.0,0.0);
	staTotalAerodynamicForce=totalForce;

	staRotorAngle=staRotorAngle+(2.0*YsPi)*dt*(staThrottle+0.1)+YsPi/50.0;

	staVPitch=vRot.p();
	staVYaw=vRot.h();
	staVRoll=vRot.b();

	ctlThrottle=double(rec.thr)/99.0;
	ctlElevator=double(rec.elv)/99.0;
	ctlAileron=double(rec.ail)/99.0;
	ctlRudderUser=double(rec.rud)/99.0;
	ctlRudderControl=0.0;
	ctlElvTrim=double(rec.elvTrim)/99.0;

	staThrottle=ctlThrottle;

	ctlThrVec=double(rec.thrVector)/99.0;
	staThrVec=ctlThrVec;
	staBombBayDoor=((double)rec.bombBay)/99.0;
	staThrRev=((double)rec.thrReverser)/99.0;

	if(staTurret.GetN()>0 && rec.turret.GetN()==staTurret.GetN())
	{
		int i;
		for(i=0; i<staTurret.GetN(); i++)
		{
			staTurret[i].h=(double)rec.turret[i].h;
			staTurret[i].p=(double)rec.turret[i].p;
			staTurret[i].ctlH=(double)rec.turret[i].h;
			staTurret[i].ctlP=(double)rec.turret[i].p;
			staTurret[i].turretState=rec.turret[i].turretState;
		}
	}

}

void FsAirplaneProperty::ComputeCarrierLandingAfterReadingFlightRecord
    (const double &dt,const YsArray <FsGround *> &carrierList)
{
	CalculateCarrierLanding(dt,carrierList);
}

const double &FsAirplaneProperty::GetMinimumManeuvableSpeed(void) const
{
	return chManSpeed1;
}

const double &FsAirplaneProperty::GetFullyManeuvableSpeed(void) const
{
	return chManSpeed2;
}


////////////////////////////////////////////////////////////

double FsGetJetEngineEfficiency(const double &alt)
/* Updated for YSFCE on 23 August 2022 to remove the 20000m thrust efficiency asymptote that was a problem for YSF. Originally was set to 0 at 20000m. Now matches the 20000m to 32000m thrust efficiency to have a smooth transition.*/
{
	int a;
	static double etaTab[]=
	{
		1.0,  /*     0m */
		1.0,  /*  4000m */
		0.8,  /*  8000m */
		0.6,  /* 12000m */
		0.3,  /* 16000m */
		0.084991   /* 20000m */
	};

	a=(int)(alt/4000.0F);
	if(8<a)
	{
		return 0.0F;
	}
	else if(a<0)
	{
		return etaTab[0];
	}
	else if(4<a)
	{
		return 0.084991F;
	}
	else
	{
		double base,diff,t;
		base=etaTab[a];
		diff=etaTab[a+1]-etaTab[a];
		t=(alt-4000.0F*a)/4000.0F;
		return base+diff*t;
	}
}

////////////////////////////////////////////////////////////

YSRESULT FsGetWeaponOfChoice(FSWEAPONTYPE &woc,const char str[])
{
	const char *const choice[]=
	{
		"GUN",
		"AAM",
		"AGM",
		"BOM",
		"SMK",
		"B25",
		"B5H",
		"A9X",
		NULL
	};
	int cmd;
	if(YsCommandNumber(&cmd,str,choice)==YSOK)
	{
		switch(cmd)
		{
		case 0:
			woc=FSWEAPON_GUN;
			return YSOK;
		case 1:
			woc=FSWEAPON_AIM9;
			return YSOK;
		case 2:
			woc=FSWEAPON_AGM65;
			return YSOK;
		case 3:
			woc=FSWEAPON_BOMB;
			return YSOK;
		case 4:
			woc=FSWEAPON_SMOKE;
			return YSOK;
		case 5:
			woc=FSWEAPON_BOMB250;
			return YSOK;
		case 6:
			woc=FSWEAPON_BOMB500HD;
			return YSOK;
		case 7:
			woc=FSWEAPON_AIM9X;
			return YSOK;
		}
	}
	return YSERR;
}

FSAIRPLANECATEGORY FsGetAirplaneCategoryFromString(const char str[])
{
	if(strcmp(str,"NORMAL")==0)
	{
		return FSAC_NORMAL;
	}
	else if(strcmp(str,"UTILITY")==0)
	{
		return FSAC_UTILITY;
	}
	else if(strcmp(str,"AEROBATIC")==0)
	{
		return FSAC_AEROBATIC;
	}
	else if(strcmp(str,"FIGHTER")==0)
	{
		return FSAC_FIGHTER;
	}
	else if(strcmp(str,"ATTACKER")==0)
	{
		return FSAC_ATTACKER;
	}
	else if(strcmp(str,"TRAINER")==0)
	{
		return FSAC_TRAINER;
	}
	else if(strcmp(str,"HEAVYBOMBER")==0)
	{
		return FSAC_HEAVYBOMBER;
	}
	else if(strcmp(str,"WW2FIGHTER")==0)
	{
		return FSAC_WW2FIGHTER;
	}
	else if(strcmp(str,"WW2BOMBER")==0)
	{
		return FSAC_WW2BOMBER;
	}
	else if(strcmp(str,"WW2ATTACKER")==0)
	{
		return FSAC_WW2ATTACKER;
	}
	else if(strcmp(str,"WW2DIVEBOMBER")==0)
	{
		return FSAC_WW2DIVEBOMBER;
	}
	else
	{
		fsStderr.Printf("Unknown category : %s\n",str);
		return FSAC_UNKNOWN;
	}
}

const char *FsGetAirplaneCategoryString(FSAIRPLANECATEGORY cat)
{
	if(cat==FSAC_NORMAL)
	{
		return "NORMAL";
	}
	else if(cat==FSAC_UTILITY)
	{
		return "UTILITY";
	}
	else if(cat==FSAC_AEROBATIC)
	{
		return "AEROBATIC";
	}
	else if(cat==FSAC_FIGHTER)
	{
		return "FIGHTER";
	}
	else if(cat==FSAC_ATTACKER)
	{
		return "ATTACKER";
	}
	else if(cat==FSAC_TRAINER)
	{
		return "TRAINER";
	}
	else if(cat==FSAC_HEAVYBOMBER)
	{
		return "HEAVYBOMBER";
	}
	else
	{
		return "";
	}
}

