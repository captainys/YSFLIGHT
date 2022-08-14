#ifndef FSAIRPLANEPROPERTY_IS_INCLUDED
#define FSAIRPLANEPROPERTY_IS_INCLUDED
/* { */

#include "fsvehicleproperty.h"
#include "fsrealprop.h"

class FsFlightControl;
class FsSimulation;

const double FsMinimumAirspeed=0.083;


#define FSAPPLYCONTROL_NONE          0x00000000
#define FSAPPLYCONTROL_ALL           (~(unsigned int)0)
#define FSAPPLYCONTROL_STICK         0x00000001
#define FSAPPLYCONTROL_THROTTLE      0x00000002
#define FSAPPLYCONTROL_TRIGGER       0x00000004
#define FSAPPLYCONTROL_VGW           0x00000008
#define FSAPPLYCONTROL_GEAR          0x00000010
#define FSAPPLYCONTROL_BRAKE         0x00000020
#define FSAPPLYCONTROL_SPOILER       0x00000040
#define FSAPPLYCONTROL_FLAP          0x00000080
#define FSAPPLYCONTROL_NAVAID        0x00000100
#define FSAPPLYCONTROL_THRVEC        0x00000200
#define FSAPPLYCONTROL_THRREV        0x00000400
#define FSAPPLYCONTROL_BOMBBAYDOOR   0x00000800
#define FSAPPLYCONTROL_TURRET        0x00001000
#define FSAPPLYCONTROL_PROPELLER     0x00002000



// Declaration /////////////////////////////////////////////
const double FsClimbRatioRecordStep=0.1;

class FsWeaponSlotLoading
{
public:
	FSWEAPONTYPE wpnType;
	int nLoaded;
	FSWEAPONTYPE containerWpnType;
	int nContainerLoaded;
	double fuelLoaded;

	FsWeaponSlotLoading(void);
	void Initialize(void);
};

class FsWeaponSlot
{
public:
	int nLoad[FSWEAPON_NUMWEAPONTYPE];
	int nSubLoad[FSWEAPON_NUMWEAPONTYPE];  // Default 1, 800 for FUELTANK
	YsVec3 pos;
	YSBOOL isExternal;

	FsWeaponSlot();
	void Initialize(void);
};

class FsAirplaneProperty : public FsVehicleProperty
{
public:
	enum
	{
		MaxNumGun=8,
		MaxNumSmokeGenerator=8,
		MaxNumFlareDispenser=8
	};

	class SmokeGeneratorState
	{
	public:
		YsColor col;
	};

	class SmokeGeneratorProperty
	{
	public:
		YsVec3 pos;
		YsColor defCol;
	};

	enum VIRTUALBUTTON
	{
		VBT_FIREWEAPON,
		VBT_FIREAAM,
		VBT_FIREAGM,
		VBT_FIREROCKET,
		VBT_DROPBOMB,
		VBT_DISPENSEFLARE,
		VBT_CYCLEWEAPON,
		VBT_CYCLESMOKESELECTOR,
	};


protected:
	// State
	FSFLIGHTSTATE staState;     //FSFLYING/FSGROUND/FSSTALL(*)
	FSDIEDOF staDiedOf;
	enum
	{
		NCLIMBRATIORECORD=24
	};

	YSBOOL staNeedTouchdownCheck;  // <- This is set in FsAirplaneProperty::CalculateGround() function,
	                            //    and refered in FsAirplane::HitGround() function.
	                            //    The airplane must touch down on runway.

	YSBOOL staOutOfRunway;   // <- For "Can Land Anywhere" option
	double staHobbsTime;

	YsArray <VIRTUALBUTTON> staVirtualButtonQueue;

	double staGndElevation;
	YsVec3 staFootAtMainGearL,staFootAtMainGearR,staFootAtWheel;
	YsVec3 staGndNormal;
	double staBaseElevation;

	YSBOOL staAb;            //Afterburner On/Off?(*)
	double staThrottle;      //Throttle 0.0-1.0(*)
	YsArray <double> staPropLever;  // Propeller lever 0.0 low RPM   1.0 high RPM

	double staSpoiler;       //Spoiler 0.0-1.0(*)
	double staGear;          //Landing Gear 0.0-1.0(*)
	double staFlap;          //Flap 0.0-1.0(*)
	double staVgw;           //Variable Geometry Wing 0.0-1.0(*)
	double staBrake;         //Brake 0.0-1.0
	double staThrVec;        //Thrust Vectoring
	double staThrRev;        //Thrust Reverser
	double staBombBayDoor;   //Bomb Bay Door

	YSBOOL staBeacon;
	YSBOOL staNavLight;
	YSBOOL staStrobe;
	YSBOOL staLandingLight;

	double staPayload;       //Payload(*)
	double staFuelLoad;      //Fuel Remains(*)

	YsVec3 staVelocity;      //Velocity Vector
	double staAOA;           //Angle of attack
	double staSSA;           //Side slip angle
	YSBOOL staMovingBackward;

	double staRho;           //Density of air at airplane's altitude
	double staLift;          //Lift Generated
	double staDrag;          //Drag Generated
	double staThrust;        //Thrust Generated
	double staV;             //V(Magnitude of Velocity)(*)
	double staVHorizontal;   //V in horizontal direction.

	double staVPitch;
	double staVYaw;
	double staVRoll;

	double staDVPitch;
	double staDVYaw;
	double staDVRoll;

	double staRotorAngle;    //Rotor Angle (for Prop and Helicopters)
	double staWheelAngle;

	double staRadarRange;

	unsigned int staVorKey[2],staNdbKey;
	int staPendingVorYsfId[2],staPendingNdbYsfId;
	double staObs[2];
	double staHdgBug;

	YSBOOL staVectorMarker;


	FSWEAPONTYPE staWoc;
	int staGunBullet;
	double staGunTimer;
	// int AAM;
	// int AIM9X;
	// int AGM;
	// int Rocket;
	// int Bomb;
	int staFlare;  // Number of internally-loaded flare
	// int AIM120;
	// int Bomb250;
	// int Bomb500Hd;
	double staSmokeOil;
	SmokeGeneratorState staSmokeGenerator[MaxNumSmokeGenerator];
	unsigned int staAirTargetKey,staGroundTargetKey;
	int staRecentlyFiredMissileId;

	double staG,staPrevG,staPrevPrevG;
	double staLateralForce,staPrevLateralForce;// Force component perpendicular to the velocity, on XZ plane of the airplane


	YSBOOL staAutoPilotInop;
	YSBOOL staFlapInop;
	YSBOOL staRadarInop;
	YSBOOL staVorInop;
	YSBOOL staAdfInop;
	YSBOOL staAirspeedInop;
	double staAirspeedStuckAltitude; // Simulating frozen pitot tube
	double staAirspeedStuckAirspeed; // Simulating frozen pitot tube
	YSBOOL staAltimeterInop;
	double staAltimeterStuckAltitude;
	YSBOOL staVSIInop;
	YSBOOL staAttitudeIndicatorInop;
	YsAtt3 staAttitudeIndicatorStuckAttitude;
	YSBOOL staPowerLoss;
	double staEngineOutputCap;                // Engine output limited to this value (%).
	YSBOOL staHUDFlicker;
	YSBOOL staHUDVisible;
	double staHUDFlickerTimer;


	YsArray <FsWeaponSlotLoading> staWeaponSlot;

	//Previous State
	FSFLIGHTSTATE staPState;
	double staStateTimer;
	YsVec3 staPPos,staPVelocity;
	YsAtt3 staPAtt;

	//Touchdown Condition
	YsVec3 staTDPos,staTDVelocity;
	YsAtt3 staTDAtt;

	//Temporary Variable
	double staWeight;
	YsVec3 staTotalAerodynamicForce;
	YsVec3 staTotalGravityForce;

	// Climb Ratio
	double staIndicatedClimbRatio;

	// For network
	enum
	{
		nNetCorrection=3
	};
	YsVec3 staNetCorrection[nNetCorrection];
	double staNetCorrectionTime[nNetCorrection];



	// Charactoristics
	FSAIRPLANECATEGORY chCategory;
	FSAIRCRAFTCLASS chClass;

	// The following two properties control if the HUD or InstPanel must be drawn in the FSCOCKPITVIEW.
	// It will allow HUD or InstPanel visible only in some EXCAMERA.
	// The switches are tested in FsSimulation::GetInstrumentDrawSwitch.
	YSBOOL chShowInstPanelInCockpit;
	YSBOOL chShowHudInCockpit;

	double chStrobeBlankTime;
	double chStrobeLightTime;
	double chBeaconBlankTime;
	double chBeaconLightTime;

	YSBOOL chIsJet;          //Jet engine? TRUE->chThrMil is N otherwise J/s
	YSBOOL chHasAb;          //Has Afterburner
	YSBOOL chHasStallHorn;   //Has Stall warning horn
	YSBOOL chHasGearHorn;    //Has Landing-gear warning horn
	double chThrAb;          //Thrust with Afterburner
	double chThrMil;         //Thrust with Military Power or Power of propeller engine
	double chPropEfficiency; //Propeller efficiency when it is not a jet
	double chPropV0;         //Minimum speed that T=P/v becomes valid.
	double chPropK;          //Maximum force produced by propeller for v<v0 is computed as F=Pmax*Kv+C

	YsArray <FsPropellerEngine> chRealProp;
	// Memo 2015/05/07  When adding chRealJet, look EncodeEngineProperty.
	//                  Number of realjet=0 must be sent to the client if no real jet engine.
	YsArray <YsString> chEnginePropCmd;

	double chCleanWeight;    //Weight Without Load
	double chMaxFuelLoad;    //Max Weight of Fuel
	double chMaxPayload;     //Max Weight of Payload
	double chAbFuelConsume;  //Fuel Consumption/Sec when Burner on
	double chMilFuelConsume; //Fuel Consumption/Sec when Mil Power
	double chThrustReverser; //Effect of thrust reverser

	// YSBOOL chLimitWeaponBySlot; Now always on.
	int chMaxNumGunBullet;
	int chMaxNumFlare;       // Preserved it as internally-loaded flare loading
	double chGunInterval;

	YsVec3 chMainGearL;      //Left Main Gear Position
	YsVec3 chMainGearR;      //Right Main Gear Position
	YsVec3 chWheel;          //Wheel Position
	double chWheelRadius;
	YsVec3 chArrestingHook;  //Arresting Hook Position
	int chNumGun;
	YsVec3 chGunPosition[MaxNumGun];    //Machine Gun Position
	YsVec3 chGunDirection[MaxNumGun];   //Machine Gun Direction
	YSBOOL chLeadGunSight;   //
	int chNumSmokeGenerator;
	SmokeGeneratorProperty chSmokeGenerator[MaxNumSmokeGenerator];  //Smoke Generator Property
	YsVec3 chVapor0;         //Vapor (wing is swept back)
	YsVec3 chVapor1;         //Vapor (wing is spread)

	double chClZero;         //Cl at Zero AOA
	double chClSlope;        //Cl increase/AOA(degree)
	double chMaxAOA;         //Critical AOA (Plus)
	double chMinAOA;         //Critical AOA (Minus)

	double chMaxCdAOA;       // AOA that gives the maximum Cd
	double chFlatClRange1;   // See NewClCdApproximation.docx
	double chFlatClRange2;   // See NewClCdApproximation.docx
	double chClDecay1;       // See NewClCdApproximation.docx
	double chClDecay2;       // See NewClCdApproximation.docx

	double chCdZero;         //Cd at Zero AOA (less than critical airspeed)
	double chCdZeroMax;      //Cd at Maximum Speed
	double chVCritical;      //Critical Airspeed
	double chVMax;           //Maximum Airspeed
	double chCdConst;        //Cd Constant

	double chBrakeConst;
	double chTireFrictionConst;

	YSBOOL chHasSpoiler;     //Has Spoiler
	YSBOOL chGearRetractable;//Landing Gear is Retractable
	YSBOOL chHasVGW;         //Has Variable Geometry Wing
	double chClVgw;          //Increase of Cl when VGW is extended
	double chCdVgw;          //Increase of Cd when VGW is extended
	double chClFlap;         //Increase of Cl when flap full down (0.2-0.6)
	double chCdFlap;         //Increase of Cd when flap full down (0.5-0.8)
	double chCdGear;         //Increase of Cd when gear down (0.8-1.0)
	double chCdSpoiler;      //Increase of Cd when spoiler is deployed(0.8-1.0)

	YSBOOL chHasVGN;         // Has Variable Geometry Nose : Concorde

	YSBOOL chHasThrustVector;//Has Thrust Vectoring Capability
	YsVec3 chThrVec0,chThrVec1;  // Thrust Vector position 0 and position 1
	double chPostStallVPitch; // Post-Stall Pitch Controllability
	double chPostStallVRoll;  // Post-Stall Roll Controllability
	double chPostStallVYaw;   // Post-Stall Yaw Controllability

	double chWingArea;       //Area of Wing

	double chMaxInputAOA;    //Max Input AOA
	double chMaxInputSSA;    //Max Input YAW
	double chMaxInputROLL;   //Max Input Roll Ratio

	double chManSpeed1;      //Minimum Maneuvable Speed
	double chManSpeed2;      //Fully Maneuvable Speed
	double chManSpeed3;      //Best Maneuvable Speed
	double chDirectAttitudeControlSpeed1;  // Maximum airspeed at which the direct attitude control is fully effective
	double chDirectAttitudeControlSpeed2;  // Airspeed at which the direct attitude control becomes fully ineffective
	double chDirectAttitudeControlReqThr1;  // Minimum required throttle setting for direct attitude control.
	double chDirectAttitudeControlReqThr2;  // Throttle setting at which the direct attitude control is fully effective.
	double chPitchManConst;  //Pitch Maneuvability Constant
	double chPitchStabConst; //Pitch Stability Constant
	double chYawManConst;    //Yaw Maneuvability Constant
	double chYawStabConst;   //Yaw Stability Constant
	double chRollManConst;   //Roll Maneuvability Constant

	YSBOOL chNoLandingFlare; // Must not flare on landing.  Used from FsLandingAutoPilot


	YsArray <FsWeaponSlot> chWeaponSlot;
	YSBOOL chAAMVisible;
	YSBOOL chAGMVisible;
	YSBOOL chRocketVisible;
	YSBOOL chBombVisible;
	YsWString chWeaponShapeFileStatic[FSWEAPON_NUMWEAPONTYPE];
	YsWString chWeaponShapeFileFlying[FSWEAPON_NUMWEAPONTYPE];

	int chNumFlareDispenser;
	YsVec3 chFlareDispenser[MaxNumFlareDispenser];
	YsVec3 chFlareDispensingVel[MaxNumFlareDispenser];


	YSBOOL chFlapPositionSet;
	YsArray <double> chFlapPosition;



	YSBOOL chTailStrikePitchIsComputed;
	double chTailStrikePitch;   // Pitch Angle of Tail Strike
	double chGroundStaticPitch; // Pitch Angle when the airplane is stable on the ground.


	int chGunPower; // <- When changing it to double, change the variable name to chGunPowerD for making use of error output
	double chBulInitSpeed;
	double chBulRange;
	double chRadarCrossSection;
	double chAAMRange;      // <- Not used 2005/01/22
	double chAGMRange;
	double chRocketRange;
	YSBOOL chBombInBombBay;
	double chBombBayRcs;    // Effect of Bomb Bay on Radar Cross Section

	// Control
	double ctlGear;
	double ctlBrake;
	double ctlSpoiler;
	YSBOOL ctlAb;
	double ctlThrottle;
	double ctlFlap;
	double ctlVgw;
	double ctlThrVec;
	double ctlThrRev;
	YSBOOL ctlAutoVgw;
	double chAutoVgwSpeed1,chAutoVgwSpeed2;
	double ctlBombBayDoor;


	double ctlElevator,ctlElvTrim,ctlAileron;
	double ctlRudderUser;
	double ctlRudderControl;

	double ctlDirectPitch,ctlDirectRoll,ctlDirectYaw;

	YSBOOL ctlSpeedControllerSwitch;
	YSBOOL ctlSpeedControllerCanUseAfterburner;
	double ctlSpeedControllerThrottleCap;
	double ctlSpeedControllerInput;
	double ctlIntegralSpdErr;
	double ctlPrevSpdErr;
	YSBOOL ctlBankControllerSwitch;
	YsVec3 ctlBankControllerInput;
	double ctlBankControllerRollRateLimit; // 2004/11/17
	YSBOOL ctlPitchControllerSwitch;
	double ctlPitchControllerInput;
	YSBOOL ctlGControllerSwitch;
	double ctlGControllerInput;
	double ctlGControllerSmoother;
	double ctlGControllerMinAOALimit,ctlGControllerMaxAOALimit;
	YSBOOL ctlAOAControllerSwitch;
	double ctlAOAControllerInput;
	double ctlAOAControllerGLimit;


	YsAtt3 chNeutralHeadAttitude;
	YSBOOL chHasHud;           // Equipped with a Head Up Display
	YSBOOL chHasInstPanel;
	YSBOOL chUseBothHudAndInstPanel;
	YsWString chInstPanelFileName;  // Instrument Panel

	YSBOOL chScreenCenterLocked;
	YsVec2 chScreenCenter;

	YSBOOL ch3dInstPanelPosSet;
	YsVec3 ch3dInstPanelPos;
	YsAtt3 ch3dInstPanelAtt;
	double ch3dInstPanelScale;


	YSBOOL ctlFireWeaponButton,pCtlFireWeaponButton;
	YSBOOL ctlFireGunButton,pCtlFireGunButton;
	YSBOOL ctlFireAAMButton,pCtlFireAAMButton;
	YSBOOL ctlFireAGMButton,pCtlFireAGMButton;
	YSBOOL ctlFireRocketButton,pCtlFireRocketButton;
	YSBOOL ctlDropBombButton,pCtlDropBombButton;
	YSBOOL ctlDispenseFlareButton,pCtlDispenseFlareButton;
	YSBOOL ctlCycleWeaponButton,pCtlCycleWeaponButton;
	YSBOOL ctlSmokeButton,pCtlSmokeButton;
	YSBOOL ctlCycleSmokeSelectorButton,pCtlCycleSmokeSelectorButton;

	unsigned char ctlSmokeSelector;



	// To take care of aircraft carrier
	YSBOOL staCatapulted,staArrested;


	// Radar
	YSBOOL chHasBombingRadar;
	YSBOOL chHasGroundRadar;
	YSBOOL chHasAirRadar;


	//Reference Vars
	double refSpdCruise;     //Cruising Speed
	double refAltCruise;     //Cruising Altitude
	double refThrCruise;     //Cruisint Throttle Setting
	double refSpdLanding;    //Speed while approaching
	double refAOALanding;    //AOA while approaching
	double refLNGRunway;     //Runway Length required to land
	double refThrLanding;    //Throttle Setting while landing



protected:
	static const char *const keyWordSource[];
	static YsKeyWordList keyWordList;

public:
	FsAirplaneProperty();
	void InitializeState(void);
	void Initialize(void);
	void ClearMalfunction(void);
	void ReleaseVirtualButton(void);

	class FsAirplane *belongTo;  // <- Must be set in FsSimulation::AddAirplane

	// enum
	// {
	// 	MaxNumAAMSlot=8,
	// 	MaxNumAGMSlot=8,
	// 	MaxNumBombSlot=8,
	// 	MaxNumRocketSlot=2
	// };

	YSRESULT LoadProperty(const wchar_t fn[]);

	static FSFLIGHTSTATE StringToState(const char str[]);
	static const char *StateToString(FSFLIGHTSTATE sta);

	YSBOOL ShowInstPanelInCockpit(void) const;
	YSBOOL ShowHudInCockpit(void) const;

	YSBOOL IsJet(void) const;
	YSBOOL HasStallHorn(void) const;
	YSBOOL HasGearHorn(void) const;
	double GetThrustWeightRatio(void) const;
	FSAIRPLANECATEGORY GetAirplaneCategory(void) const;
	FSAIRCRAFTCLASS GetAircraftClass(void) const;
	YSBOOL IsTailDragger(void) const;
	YSBOOL CanManeuverPostStall(void) const;
	YSBOOL NoLandingFlare(void) const;

	double GetCdSpoiler(void) const;

	FSFLIGHTSTATE GetState(void) const;
	FSFLIGHTSTATE GetFlightState(void) const;
	void SetState(FSFLIGHTSTATE sta,FSDIEDOF diedOf);
	void SetFlightState(FSFLIGHTSTATE sta,FSDIEDOF diedOf);
	const double &GetVariableGeometryWingState(void) const;
	void Crash(FSDIEDOF diedOf);
	void Overrun(void);
	virtual YSBOOL IsAlive(void) const;
	virtual YSBOOL IsActive(void) const;
	FSDIEDOF GetDiedOf(void) const;
	YSBOOL GetNeedTouchdownCheck(void) const;
	void ClearNeedTouchdownCheck(void);

	void SetOutOfRunway(YSBOOL oor);
	YSBOOL IsOutOfRunway(void) const;

	void Move(const double &t,const YsArray <class FsGround *> &carrierList,const class FsWeather &weather);
	void MoveTimer(const double &dt);
	void BeforeMove(void);
	void RecordClimbRatio(const double &dt);
	const double &GetClimbRatioWithTimeDelay(void) const;
	const double GetClimbRatio(void) const; // <- For some reason, const double & doesn't work. Bug of VC2010?



	void PressVirtualButton(VIRTUALBUTTON btn);
	YSRESULT CycleWeaponOfChoice(void);
	void CycleSmokeSelector(void);
	YSRESULT SetWeaponOfChoice(FSWEAPONTYPE woc);
	FSWEAPONTYPE GetWeaponOfChoice(void) const;
	YSRESULT SelectWeapon(void);
	YSBOOL FireGunIfVirtualTriggerIsPressed
	    (const double &ct,const double &dt,class FsSimulation *sim,class FsWeaponHolder &bul,class FsExistence *own);
	YSBOOL ProcessVirtualButtonPress(
	    YSBOOL &blockedByBombBay,FSWEAPONTYPE &firedWeapon,
	    FsSimulation *sim,const double &ctime,class FsWeaponHolder &bul,FsExistence *owner);
private:
	YSBOOL RunVirtualButtonQueue(
	    YSBOOL &blockedByBombBay,FSWEAPONTYPE &firedWeapon,
	    FsSimulation *sim,const double &ctime,FsWeaponHolder &bul,FsExistence *owner);
public:
	YSBOOL FireSelectedWeapon(
	    YSBOOL &blockedByBombBay,
	    FsSimulation *sim,const double &ct,class FsWeaponHolder &bul,class FsExistence *own);
	YSBOOL FireWeapon(
	    YSBOOL &blockedByBombBay,FsSimulation *sim,const double &ct,class FsWeaponHolder &bul,class FsExistence *own,FSWEAPONTYPE woc);
	int GetRecentlyFiredMissileId(void) const;
	void FireMissileByRecord(FSWEAPONTYPE wpnType);
	YSRESULT ToggleRadarRange(int dir);
	YSRESULT IncreaseRadarRange(void);
	YSRESULT ReduceRadarRange(void);
	YSRESULT TurnOnRadar(void);
	YSRESULT TurnOffRadar(void);
	YSBOOL GetDamage(YSBOOL &killed,int dmg,FSDIEDOF diedOf);

	YSRESULT ToggleLight(void);
	YSRESULT ToggleNavLight(void);
	YSRESULT ToggleBeacon(void);
	YSRESULT ToggleStrobe(void);
	YSRESULT ToggleLandingLight(void);


	int LoadWeaponToSlot(FSWEAPONTYPE wpnType,int n); // Returns number of weapons actually loaded
	int AddWeaponToSlot(FSWEAPONTYPE wpnType,int n);  // Returns number of weapons actually loaded
	YSBOOL GuidedAAMIsLoaded(void) const;
	YSBOOL GuidedAGMIsLoaded(void) const;
	YSBOOL FreeFallBombIsLoaded(void) const;
	YSBOOL AntiGroundWeaponIsLoaded(void) const;
	void UnloadAllWeapon(void);
	void UnloadGuidedAAM(void);
	void UnloadGuidedAGM(void);
	void UnloadUnguidedWeapon(void);
	int WeaponTypeToSlotBit(FSWEAPONTYPE wpnType) const;
	int CountSlotWeaponLoad(FSWEAPONTYPE wpnType);
	void ShowSlotConfig(void);
	int GetMaxNumWeapon(FSWEAPONTYPE wpnType) const;
	int GetMaxNumSlotWeapon(FSWEAPONTYPE wpnType) const;
	void AddWeaponSlot(const YsVec3 &pos,int nLoad[FSWEAPON_NUMWEAPONTYPE],int nSubUnit[FSWEAPON_NUMWEAPONTYPE],YSBOOL isExternal);
	void AddWeapon(FSWEAPONTYPE wpnType,int n);
	YSRESULT GetWeaponConfig(YsArray <int,64> &loading) const;
	YSRESULT ApplyWeaponConfig(YSSIZE_T n,const int loading[]);
	static void MakeUpWeaponConfigForOldVersion(YsArray <int,64> &loading,int aam,int agm,int rkt,int bom);
	static void RemoveWeaponFromWeaponConfig(YSSIZE_T nWeaponConfig,int weaponConfig[],FSWEAPONTYPE wpnType);


	/*! Returns the reference cruising altitude.
	*/
	const double GetReferenceCruisingAltitude(void) const;

	const double GetRadarCrossSection(void) const;
	const double &GetBulletSpeed(void) const;
	const double &GetRocketSpeed(void) const;
	const double &GetAAMRadarAngle(void) const;
	const double GetAAMRange(FSWEAPONTYPE wpnType) const;
	YSBOOL GetLeadGunSight(void) const;
	int GetNumWeapon(FSWEAPONTYPE wpnType) const;
	int GetNumSlotWeapon(FSWEAPONTYPE wpnType) const;
	double GetExternalFuelLeft(void) const;
	void SetNumWeapon(FSWEAPONTYPE wpnType,int n);
	const wchar_t *GetWeaponShapeFile(FSWEAPONTYPE wpnType,int state) const;  // state: 0 Static  1 Flying

	const double &GetSmokeOil(void) const;
	const YsColor &GetDefaultSmokeColor(int smkIdx) const;
	const YsColor &GetSmokeColor(int smkIdx) const;
	void SetSmokeOil(const double &oil);
	YSBOOL SetAirTargetKey(unsigned int airKey);
	unsigned int GetAirTargetKey(void) const;
	const double & GetAGMRadarAngle(void) const;
	const double & GetAGMRange(void) const;
	int GetAGMDestructivePower(void) const;
	const double & GetRocketRange(void) const;
	YSBOOL SetGroundTargetKey(unsigned int gndKey);
	unsigned int GetGroundTargetKey(void) const;

	YSRESULT GetGunPosition(YsVec3 &gun,YsVec3 &gunDir) const;
	const double &GetGunInterval(void) const;
	int GetGunPower(void) const;

	YSBOOL CheckTouchDownAndLayOnGround(double &gDistance);
	void CalculateGroundStopDistanceAndTime(double &dist,double &tm,double v0) const;
	void CalculateGroundDescelerateDistanceAndTime(double &dist,double &tm,double v0,double vTarget) const;

	void SetupVisual(FsVisualDnm &vis) const;
	void DrawOrdinance(
	    unsigned int drawFlag,
	    const YsMatrix4x4 *mat=NULL,
	    const class FsVisualDnm weaponShapeOverrideStatic[]=NULL) const;
	void DrawOrdinanceVisual(
	     YSBOOL coarse,
	     const class FsVisualDnm weaponShapeOverrideStatic[],
	     const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,
	     unsigned int drawFlag,
	     const YsMatrix4x4 *mat=NULL) const;
	void DrawOrdinanceShadow(
	     YSBOOL coarse,
	     const class FsVisualDnm weaponShapeOverrideStatic[],
	     const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm,
	     const YsMatrix4x4 *mat=NULL) const;
	YSBOOL IsWeaponSlotCurrentlyVisible(int slotId) const;


	YSRESULT ToggleGear(void);
	YSRESULT SetGear(const double &a);
	YSRESULT ToggleBrake(void);
	YSRESULT SetBrake(const double &a);
	YSRESULT SetSpoiler(const double &a);
	YSRESULT SetThrottle(const double &a);
	YSRESULT SetAfterburner(YSBOOL sw);
	YSRESULT SetPropellerAll(const double prop);
	YSRESULT SetThrottleByRequiredThrust(const double &t);
	const double GetThrottleForRequiredThrust(YSBOOL &ab,const double requiredThrust,YSBOOL canUseAfterburner);  // Cannot be const: May need iterative calculation for real prop
	const double GetThrottleForRequiredThrustForVelocity(YSBOOL &ab,const double requiredThrust,const double vel,YSBOOL canUseAfterburner);  // Cannot be const: May need iterative calculation for real prop
	YSRESULT SetThrustReverser(const double &a);
	YSRESULT ToggleAfterburner(void);
	YSRESULT SetElevator(const double &a);
	YSRESULT SetElvTrim(const double &a);
	YSRESULT SetAileron(const double &a);
	YSRESULT SetRudder(const double &a);
	YSRESULT SetDirectPitchControl(const double &a);
	YSRESULT SetDirectRollControl(const double &a);
	YSRESULT SetDirectYawControl(const double &a);
	void NeutralDirectAttitudeControl(void);
	void NeutralDirectPitchControl(void);
	void NeutralDirectRollControl(void);
	void NeutralDirectYawControl(void);
	YSRESULT ToggleFlap(void);
	YSRESULT SetFlap(const double &a);
	YSRESULT SetThrustVector(const double &a);
	YSRESULT SetVgw(const double &a);
	YSRESULT SetAutoVgw(YSBOOL a);
	YSRESULT SetBombBayDoor(const double &a);

	YSRESULT BouncePitchByTailStrike(void);

	YSRESULT SetVectorMarker(YSBOOL vectMark);
	YSBOOL GetVectorMarker(void) const;

	void SetAllVirtualButton(YSBOOL btn);
	void SetFireWeaponButton(YSBOOL btn);
	YSBOOL GetFireWeaponButton(void);
	YSBOOL IsFireWeaponButtonJustPressed(void);
	void SetFireGunButton(YSBOOL btn);
	YSBOOL GetFireGunButton(void);
	YSBOOL IsFireGunButtonJustPressed(void);
	void SetFireAAMButton(YSBOOL btn);
	YSBOOL GetFireAAMButton(void);
	YSBOOL IsFireAAMButtonJustPressed(void);
	void SetFireAGMButton(YSBOOL btn);
	YSBOOL GetFireAGMButton(void);
	YSBOOL IsFireAGMButtonJustPressed(void);
	void SetFireRocketButton(YSBOOL btn);
	YSBOOL GetFireRocketButton(void);
	YSBOOL IsFireRocketButtonJustPressed(void);
	void SetDropBombButton(YSBOOL btn);
	YSBOOL GetDropBombButton(void);
	YSBOOL IsDropBombButtonJustPressed(void);
	void SetDispenseFlareButton(YSBOOL btn);
	YSBOOL GetDispenseFlareButton(void);
	YSBOOL IsDispenseFlareButtonJustPressed(void);
	void SetCycleWeaponButton(YSBOOL btn);
	YSBOOL GetCycleWeaponButton(void);
	YSBOOL IsCycleWeaponButtonJustPressed(void);
	void SetSmokeButton(YSBOOL btn);
	void SetSmokeSelector(unsigned char genPos);
	void SetCycleSmokeSelectorButton(YSBOOL btn);
	unsigned char GetSmokeSelector(void) const;
	YSBOOL GetSmokeButton(void);
	YSBOOL IsSmokeButtonJustPressed(void);
	YSBOOL IsCycleSmokeSelectorButtonJustPressed(void);

	YSBOOL IsFiringGun(void) const;

	YSBOOL IsOnGround(void) const;

	// Controller: When Pitch Controller is turned on,
	//             G controller must be turned off. And vise-versa.
	void TurnOffController(void);
	void TurnOffSpeedController(void);
	YSRESULT SpeedController(const double &bnk);
	YSRESULT SpeedControllerDontUseAfterburner(const double &spd,const double throttleCap);
	void TurnOffBankController(void);
	YSRESULT BankController(const double &bnk,const double &rollRateLimit=YsInfinity);
	YSRESULT BankController(const YsVec3 &uv,const double &rollRateLimit=YsInfinity);
	void TurnOffPitchController(void);
	YSRESULT PitchController(const double &pit);
	void TurnOffGController(void);
	YSRESULT GController(const double &g);
	YSRESULT GControllerSmooth(const double &g,const double dG,const double dt);
	void CaptureGControllerSmoother(void);
	void SetGControllerAOALimit(const double &minAOA,const double &maxAOA);
	void TurnOffAOAController(void);
	YSRESULT AOAController(const double &aoa,const double &gLimit=9.0);

	YSRESULT ConfigElevatorByAoa(const double &aoa);


	void SetPosition(const YsVec3 &vec);
	void SetAttitude(const YsAtt3 &att);
	void SetPositionAndAttitude(const YsVec3 &vec,const YsAtt3 &att);
	void SetVelocity(const YsVec3 &vel);

	void SetFieldElevation(const double &elv);
	void SetFieldNormal(const YsVec3 &nom);
	void SetBaseElevation(const double &elv);
	const double &GetGroundElevation(void) const;
	double GetAGL(void) const;
	const YsVec3 &GetGroundNormal(void) const;
	const double GetTrueAltitude(void) const;
	const double GetIndicatedTrueAltitude(void) const;

	const double &GetThrottle(void) const;
	const double GetRealPropPropellerSetting(YSSIZE_T engineIdx) const;
	const double GetRealPropRPM(YSSIZE_T engineIdx) const;
	YSRESULT GetRPMRangeForSoundEffect(double &min,double &max,int engineIdx) const;
	YSBOOL HasFixedSpeedPropeller(void) const;
	const double &GetThrustReverser(void) const;
	const double &GetElevator(void) const;
	const double &GetElvTrim(void) const;
	const double &GetAileron(void) const;
	const double GetRudder(void) const;
	const double GetCurrentThrust(void) const;
	const double &GetThrustVector(void) const;
	const double &GetRudderUserInput(void) const;
	const double &GetRudderAutoCoordinator(void) const;
	const double GetVelocity(void) const;
	const double GetHorizontalVelocity(void) const;
	void GetVelocity(YsVec3 &vel) const;
	void GetRotationSpeed(YsAtt3 &rot) const;
	void GetAcceleration(YsVec3 &acc) const;
	YSBOOL LandingGearIsLocked(void) const;
	YSBOOL HasRetractableLandingGear(void) const;
	const double &GetLandingGear(void) const;
	const double &GetBombBayDoor(void) const;
	YSBOOL GetBombIsInBombBay(void) const;
	const double &GetBombBayRadarCrossSection(void) const;
	YSBOOL GetBrake(void) const;
	const double GetBrakeMovingRate(void) const;
	const double CalculateForceByBrake(const double brakeControl) const;
	const double &GetFlap(void) const;
	const double &GetSpoiler(void) const;
	YSBOOL GetHasSpoiler(void)const;
	const double &GetControlVgw(void) const;
	YSBOOL GetAfterBurner(void) const;
	YSBOOL GetHasAfterburner(void) const;

	YSBOOL GetHasThrustVectoring(void) const;
	void GetThrustDirection(YsVec3 &dir) const;

	int GetNumFlapPosition(void) const;
	const double &GetFlapPosition(int flpPosId) const;

	YSBOOL GetHasThrustReverser(void) const;

	YSBOOL GetHasVariableGeometryNose(void) const;

	const double GetSafeGroundSpeed(void) const;

	int GetNumTurret(void) const;
	int GetMaxNumTurretBullet(int turretId) const;
	YSBOOL GetHasAntiAirTurret(void) const;
	YSBOOL GetHasAntiGroundTurret(void) const;
	YSBOOL GetHasPilotControlledTurret(void) const;
	YSBOOL GetHasGunnerControlledTurret(void) const;
	YSRESULT GetFirstPilotControlledTurretDirection(YsVec3 &dir) const;
	YSRESULT GetFirstPilotControlledTurretPosition(YsVec3 &pos) const;

	const double &GetEstimatedCruiseSpeed(void) const;
	const double &GetEstimatedLandingSpeed(void) const;
	const double &GetEstimatedRequiredRunwayLength(void) const;


	YSRESULT EncodeProperty(YsArray <YsString> &cmd,YsArray <YsString> &turretCmd,unsigned int netVersion);
	YsArray <YsString> EncodeEngineProperty(unsigned int netVersion) const;


	void MakeShortFormat(YsString &str,const char srcCmd[],int netVersion) const;
	virtual YSRESULT SendCommand(const char cmd[]);
	YSRESULT AddWeaponSlotByArgument(int ac,char *av[]);


	void AutoCalculate(void);
	void LoadFuel(void);
	void LoadFuel(const double fuel);
	const double &GetFuelLeft(void) const;
	const double &GetMaxFuelLoad(void) const;
	double GetG(void) const;
	const double &GetAOA(void) const;
	const double &GetSSA(void) const;
	double GetLateralForce(void) const;
	double GetMach(void) const;
	double GetCleanWeight(void) const;
	double GetPayloadWeight(void) const;
	double GetTotalWeight(void) const;

	const YsAtt3 &GetIndicatedAttitude(void) const;

	const YsAtt3 &GetNeutralHeadDirection(void) const;
	const YsVec2 &GetScreenCenter(void) const;
	YSBOOL HasHud(void) const;
	YSBOOL HasInstPanel(void) const;
	const wchar_t *GetInstPanelFileName(void) const;
	const YsVec3 &GetInstPanelPos(YsVec3 &instPanelPos) const;
	const YsAtt3 &GetInstPanelAtt(void) const;
	const double &GetInstPanelScaling(void) const;

	const int GetNumTire(void) const;
	const YsVec3 &GetTirePosition(int id) const;
	void SetElevationAtTire(int id,const YsVec3 &foot);

	void GetTransformedRearGearPosition(YsVec3 &rgp) const;
	void GetTransformedFrontGearPosition(YsVec3 &fgp) const;
	void GetTransformedArrestingHookPosition(YsVec3 &hook) const;
	double GetGroundStandingHeight(void) const;
	int GetNumSmokeGenerator(void) const;
	void GetSmokeGeneratorPosition(YsVec3 &smk,int smkId) const;
	void GetVaporPosition(YsVec3 &vap) const;

	YSBOOL CheckHasJustTouchDown(void) const;
	YSBOOL CheckSafeTouchDown(FSDIEDOF &diedOf) const;
	YSBOOL CheckHasJustAirborne(void) const;

	YSBOOL IsTrailingVapor(void) const;
	YSBOOL IsTrailingSmoke(int smkIdx) const;

	void WriteFlightRecord(class FsFlightRecord &rec) const;
	void ReadbackFlightRecord
	(const FsFlightRecord &rec,
	 const double &dt,const double &velocity,
	 const YsAtt3 &vRot,const YsVec3 &totalForce,const YsVec3 &unitVelocityVector);
	void ComputeCarrierLandingAfterReadingFlightRecord(const double &dt,const YsArray <FsGround *> &carrierList);
	virtual void CaptureState(YsArray <YsString> &stateStringArray) const;


	unsigned NetworkEncode(unsigned char dat[],int idOnSvr,const double &currentTime,YSBOOL shortFormat) const;
	      // this returns number of bytes
	void NetworkDecode(class FsNetReceivedAirplaneState &prevState,class FsNetReceivedAirplaneState &recvState);
		// prevState will be updated.
		// recvState's tLocal and tRemote will be calibrated.

	unsigned EncodeTurretState(unsigned char dat[],int idOnSvr) const;

	void ApplyNetCorrection(const double &dt);
	void AddNetCorrection(const YsVec3 &err,const double &duration);


	const double &GetMinimumManeuvableSpeed(void) const;
	const double &GetFullyManeuvableSpeed(void) const;

	double GetLift(const double &aoa,const double &vel) const;
	double GetDrag(const double &aoa,const double &vel) const;
	double GetThrust(const double &thr,const double &alt,const double &vel,YSBOOL ab) const;
	double GetConvergentThrust(const double &thr,const double &alt,const double &vel,YSBOOL ab); // Cannot be const.  It actually test-drive real prop.
	const double ComputeAOAForRequiredG(const double &gInput);
	const double ComputeAOAForRequiredG(const double &gInput,const double v) const;
	const double ComputeElevatorTrimForAOA(const double aoa) const;

	void ApplyControl(const FsFlightControl &ctl,unsigned int whatToApply);
	void ReadBackControl(FsFlightControl &ctl) const;

	void SmartRudder(const double &dt);
	void TurnOffSmartRudder(void);
	void ConfigureRudderBySSA(const double &ssa);
	void AutoCoordinate(void);

	void AfterUnloadedFromCarrier(void);
	void AfterLoadedOnCarrier(FsGround *carrier);
	YSBOOL IsOnArrestingWire(void) const;


	const double GetCurrentRadarRange(void) const;
	YSBOOL HasAirToAirRadar(void) const;
	YSBOOL HasAirToGndRadar(void) const;
	YSBOOL HasBombingRadar(void) const;
	YSRESULT ComputeEstimatedBombLandingPosition(YsVec3 &pos,const class FsWeather &weather) const;

	void SetVorStation(int navId,unsigned int vorKey);
	void UnsetVorStation(int navId);
	void SetVorObs(int navId,const double &heading);
	unsigned int GetVorStationKey(int navId) const;
	const double &GetVorObs(int navId) const;
	int GetPendingVorYsfId(int navId) const;
	void SetPendingVorYsfId(int navId,int ysfId);

	/*! Set heading bug in the internal heading angle. (Not True, Not Magnetic.)
	*/
	void SetHeadingBug(const double &hdgBug);

	/*! Returns heading bug in the internal heading angle. (Not True, Not Magnetic.)
	*/
	const double &GetHeadingBug(void) const;

	void SetNdbStation(unsigned int ndbKey);
	void UnsetNdbStation(void);
	unsigned int GetNdbStationKey(void) const;
	int GetPendingNdbYsfId(void) const;
	void SetPendingNdbYsfId(int ysfId);

	YSRESULT AutoComputeTailStrikePitchAngle(const YsShell &collisionShell);  // This computes ground static angle as well
	YSBOOL TailStrikeAngleHasBeenComputed(void) const;
	double GetTailStrikePitchAngle(const double &safetyFactor);
	const double &GetGroundStaticPitchAngle(void) const;
	const double &GetCriticalAOA(void) const;

	void GetPitchManeuvabilityConst(double &kPitch,double &bPitch);
	double GetRollManeuvabilityConst(void);
	double GetYawManeuvabilityConst(void) const;
	double GetGroundYawSpeed(const double &rudInput) const;
	double GetYawStabilityConst(void);
	double GetBankControllerGain(void);

	// It is confusing.  Use GetMaxRollRate() instead;
	void GetRollRate(double &vRoll);

	const double GetMaxRollRate(void) const;

protected:
	void CalculateCurrentState(void);
	void CalculateFuelConsumption(const double &dt);

	void CalculateRungeKutta4(const double &dt);
	void CalculateGravitationalForce(void);
	void AdjustGravitationalForceForSlope(void);
	void CalculateForce(void);
	void CalculateTranslation(const double &dt);
	void CalculateRotationalAcceleration(void);
	void CalculateRotation(const double &dt);
	void CalculateCarrierLanding(const double &dt,const YsArray <FsGround *> &carrierList);
	void CalculateGround(const double &dt);  // Field elevation must be set before calling this function
	void CalculateStall(void);
	void CalculateWeather(const double &dt,const FsWeather &weather);
public:
	void CalculateWindDrift(YsVec3 &driftVec,const FsWeather &weather) const;

protected:
	double GetInputAOA(void);
	double GetInputSSA(void);
	double GetMaxInputSSA(void) const;
	double CalculateMaxTurnRateOnGround(void) const;
public:
	double CalculateTurnRatiusOnGround(const double v) const;
protected:
	double GetInputROLL(void);

	void CalculateAutoPilot(const double &dt);

	void ControlSpeed(const double &speedInput,const double &dt);
	void ControlBank(const YsVec3 &bankInput,const double &rollRateLimit,const double &dt);
	void ControlPitch(const double &pitchInput,const double &dt);
	void ControlG(const double &gInput,const double &minAOALimit,const double &maxAOALimit,const double &dt);
	void ControlAOA(const double &aoaInput,const double &gLimit,const double &dt);

	void LimitElevator(); //(const double &minAOA=-YsPi/2.0,const double &maxAOA=YsPi/2.0);

	double CalculatePropellerThrust(const double &thr,const double &alt,const double &vel) const;

protected:
	double MoveDevice(const double & toBe,double rightNow,const double & howFast,const double & t);

public:
	void MakeUpAircraftTroubleCommand(YsString &cmd,FSAIRCRAFTTROUBLE trouble,const double param0,const double param1,const double param2) const;
	void MakeUpClearAircraftTroubleCommand(YsString &cmd,FSAIRCRAFTTROUBLE trouble) const;
	YSBOOL CheckAutoPilotInop(void) const;
	YSBOOL CheckFlapInop(void) const;
	YSBOOL CheckRadarInop(void) const;
	YSBOOL CheckVORInop(void) const;
	YSBOOL CheckADFInop(void) const;
	YSBOOL CheckAirspeedInop(void) const;
	const double GetMalfunctioningAirspeedIndication(void) const;
	YSBOOL CheckAltimeterInop(void) const;
	YSBOOL CheckVSIInop(void) const;
	YSBOOL CheckAttitudeIndicatorInop(void) const;
	YSBOOL CheckHUDFlicker(void) const;
	YSBOOL CheckHUDVisible(void) const;
};


double FsGetJetEngineEfficiency(const double &alt);



FSAIRPLANECATEGORY FsGetAirplaneCategoryFromString(const char str[]);
const char *FsGetAirplaneCategoryString(FSAIRPLANECATEGORY cat);


/* } */
#endif
