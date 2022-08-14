#ifndef FSGROUNDPROPERTY_IS_INCLUDED
#define FSGROUNDPROPERTY_IS_INCLUDED
/* { */

#include <fsdef.h>
#include "fsnavaid.h"
#include "fsvehicleproperty.h"
#include "fsvisual.h"

// Declaration /////////////////////////////////////////////

class FsAircraftCarrierDeckPolygonCache
{
public:
	YsShellPolygonHandle plHd;
	YsPlane pln;
	YsArray <YsVec3> plVtPos;
	YsArray <YsVec2> projPlVtPos;

	void Initialize(void);
};

class FsAircraftCarrierProperty
{
public:
	FsAircraftCarrierProperty();
	~FsAircraftCarrierProperty();

	YSRESULT LoadAircraftCarrier(const wchar_t tmplRootDir[],const wchar_t fn[]);
private:
	void FindAircraftCarrierPropertyFile(YsWString &fn,const wchar_t tmplRootDir[]);

public:
	YSRESULT LoadAirplane(FsAirplane *air); // When load, subtract velocity
	YSRESULT UnloadAirplane(FsAirplane *air); // When unload, add velocity
	YSRESULT LoadGround(FsGround *gnd);
	YSRESULT UnloadGround(FsGround *gnd);

	YSBOOL IsAirplaneLoaded(const FsAirplane *air) const;
	YSBOOL IsGroundLoaded(const FsGround *gnd) const;
	YSBOOL IsOnDeck(const YsVec3 &pos) const;      // Must return false if it is dead.
	YSBOOL IsOnDeckLocal(const YsVec3 &pos) const;      // Must return false if it is dead.
	YSBOOL IsOnArrester(const YsVec3 &pos) const;  // Must return false if it is dead.
	YSBOOL IsOnCatapult(const YsVec3 &pos) const;  // Must return false if it is dead.
	YSBOOL LandedOnTheDeck
	    (const YsVec3 &prv,const YsVec3 &now,
	     const YsVec3 &gear1,const YsVec3 &gear2,const YsVec3 &gear3) const;
	double GetDeckHeightAndNormal(YsVec3 &nom,const YsVec3 &pos) const;
	const YsVec3 &GetBridgePos(void) const;

	const YsVec3 GetCatapultPos(void) const;
	const YsVec3 GetCatapultVec(void) const;

	YSRESULT MoveCargoAndILS(const YsMatrix4x4 &now,const YsMatrix4x4 &prv,const double &yaw);
	YSRESULT Maneuver(const double &t);

	YSBOOL HasCatapult(void) const;
	YSBOOL CanBeViewpoint(void) const;
	YSBOOL NoAutoTaxi(void) const;

	YSRESULT DrawBridge(const YsMatrix4x4 &viewMat) const;
	static void BeginDrawArrestingWire(void);
	YSRESULT DrawArrestingWire(void) const;
	static void EndDrawArrestingWire(void);


	const class FsILS &GetILS(void) const;  // Use FsILS::GetRange() to see if it is really an ILS.

	void ForceClearLoadedObject(void);

protected:
	YSBOOL IsOnSomething(const YsArray <YsVec3> &ary,const YsVec3 &pos) const;
	YSRESULT PrepareDeckShell(void);
	YSRESULT ConvertShellToArray(YsArray <YsVec3> &ary,const YsShell &shl) const;

	FsVisualDnm bridge;
	YsShell deck;
	YsArray <FsAircraftCarrierDeckPolygonCache> deckCache;

	YsArray <YsVec3> arrester,catapult;
	YsVec3 bbx1,bbx2; // Set y -1.0 through 1.0. Compare position at y=0.0
	double bbxDiagonal;
	YSBOOL canBeViewpoint;

	YSBOOL drawArrestingWire;
	YsVec3 arrestingWirePos[2];

	YsVec3 deckPos,catapultPos;

	YsArray <class FsAirplane *> airList;
	YsArray <class FsGround *> gndList;

	int maneuver;
	double maneuverTime;
	double designatedHeading;

	class FsILS ils;
	YsVec3 ilsPosition;
	YsAtt3 ilsAngle;
	double ilsRange;

	YSBOOL noAutoTaxi;

public:
	class FsGroundProperty *belongTo;
};


class FsGroundProperty : public FsVehicleProperty
{
friend class FsAircraftCarrierProperty;

protected:
	// When adding a state, make sure it is copied in CopyState function.
	FSGNDSTATE staState;

	FSWEAPONTYPE staWoc;
	int staSAM;
	int staGunBullet;
	int staCannon;
	double staAaaTimer;
	double staCannonTimer;

	YSBOOL staFiringAaa;
	YSBOOL staFiringSam;
	YSBOOL staFiringCannon;

	YsVec3 staSpeed; //	2005/03/27 Used to be double Speed;  Now, the velocity in the local coordinate.
	YSBOOL staDrift;
	double staRotation;
	YSBOOL staIsStatic; // Updated in FsGroundPeoperty::Move.
	double staNextMissileFireTime;
	int staSamReloadCount;

	double staTimeCounter;

	YsAtt3 staAaaAim;   // in Global Coordinate System
	YsAtt3 staSamAim;   // in Global Coordinate System
	YsAtt3 staCanAim;   // in Global Coordinate System

	class FsAirplane *staAirTarget;
	class FsGround *staGndTarget;

	// User control
	unsigned char staLightState;
	double staLightTimer;

	FSVEHICLECONTROLLER staWhoIsInControl;
	double staLeftDoor;
	double staRightDoor;
	double staRearDoor;
	double staLeftDoorToBe;
	double staRightDoorToBe;
	double staRearDoorToBe;
	double staSteering,staPitchRotation,staHeadingRotation;
	double staAccel;
	double staReverse;
	double staWheelRotation;
	double staBrake;
	double staSetSpeed;
	double staSetVh;

	double staSetTurretVh;
	double staSetTurretVp;
	double staSetTurretVb;

	// When adding a state, make sure it is copied in CopyState function.


protected:
	static const char *const keyWordSource[];
	static YsKeyWordList keyWordList;
public:
	enum CANMOVEAREA
	{
		CANMOVE_ANYWHERE,      // Can move over water or ground
		CANMOVE_WATER,         // Can move over water
		CANMOVE_GROUND,        // Can move over ground
		CANMOVE_PAVED,         // Can move on runway/taxiway
		CANMOVE_SPECIFICREGION // Can move over specific region id.
	};
	enum
	{
		USERCTL_NONE=0,
		USERCTL_DRIVE=1,       // Can drive
		USERCTL_GUN=2,         // Can aim / shoot gun
		USERCTL_CANNON=4,      // Can aim / shoot cannon
		USERCTL_SAM=8,         // Can aim / shoot SAM
		USERCTL_ATM=16,        // Can aim / shoot ATM
	};
	enum
	{
		MAXNUMAAAPOSITION=2,
		MAXNUMSAMPOSITION=2,
		MAXNUMCANPOSITION=2
	};

	FsGroundProperty();
	~FsGroundProperty();

	void Initialize(void);
	void InitializeState(void);
	void CleanUpAircraftCarrierProperty(void);


	enum
	{
		YSGP_ANTIGROUND=1,
		YSGP_HUGGROUND=2,
		YSGP_NONEGAMEOBJECT=4,
		YSGP_CANFLOAT=8,
		YSGP_STAYUPRIGHT=16,        // <- No Rotation, only available when CANFLAT is true.
		YSGP_NOSHADOW=32,
		YSGP_USEAIMAT=64,           // <- If it is on, use chAimAt for shooting at it.
		YSGP_TARGETANY=128,         // Target Any IFF
		YSGP_ASMTARGET=256,         // Can be a target of anti-ship missile
		YSGP_TRACKAIRTARGET=512,    // Track air target even when no SAM is loaded.
		YSGP_SUPPLYFUEL=1024,       // Can supply fuel.
		YSGP_SUPPLYAMMO=2048,       // Can supply ammo.
	};
	enum
	{
		YSGP_NOTVISUALLANDINGAID,
		YSGP_PAPI,
		YSGP_VASI
	};


	YSRESULT LoadProperty(const wchar_t fn[],YsWString &aircraftCarrier);

	void ApplyControl(const class FsFlightControl &ctl,unsigned int whatToApply);

	void PrepareUserWeaponofChoice(void);  // Called from FsPersona::BoardGround
	YSRESULT CycleWeaponOfChoiceByUser(void);

	FSGROUNDTYPE GetGroundType(void) const;

	void SetWhoIsInControl(FSVEHICLECONTROLLER ctrl);
	FSVEHICLECONTROLLER GetWhoIsInControl(void) const;
	void SetSteering(const double steer);
	void SetBrake(const double brake);
	void SetAccel(const double accel);
	void SetReverse(const double reverse);
	void SetDesiredSpeed(const double speed);

	double CalculateBrakeAcceleration(const double brake) const;
	double CalculateCurrentTurnRate(const double steer) const;
	double CalculateTurnRate(const double steer,const double v) const;

	double MoveDevice(
	    const double &toBe,double rightNow,const double &howFast,const double &dt);

	void Move(
	    YsVec3 &motionPathOffset,YSSIZE_T &motionPathIndex,YSBOOL useMotionPathOffset,
	    YSSIZE_T nMpathPnt,const YsVec3 *mpathPnt,YSBOOL mpathIsLoop, /* const YsSceneryPointSet *motionPath, */
	    const double &t);
	void ManeuverAlongMotionPath(
	    YsVec3 &motionPathOffset,YSSIZE_T &motionPathIndex,
	    YSSIZE_T nMpathPnt,const YsVec3 mpathPnt[],YSBOOL mpathIsLoop, /* const YsSceneryPointSet *motionPath, */
	    YSBOOL useMotionPathOffset);

	void AfterUnloadedFromCarrier(void);
	void AfterLoadedOnCarrier(class FsGround *carrier);


	YSBOOL FireGun(
	    const double &ct,const double &dt,class FsSimulation *sim,class FsWeaponHolder &bul,class FsExistence *own);
	YSBOOL FireMissile(
	    const double &ct,class FsWeaponHolder &bul,class FsExistence *own);
	const double GetTimeBeforeNextMissileCanBeShot(const double currnetTime) const;
	YSBOOL GetDamage(YSBOOL &killed,int dmg);

	double GetBulletSpeed(void) const;
	double GetSAMRadarAngle(void) const;
	double GetSAMRange(void) const;
	double GetAAARange(void) const;
	YSBOOL SetAirTarget(class FsAirplane *trg);
	FsAirplane *GetAirTarget(void) const;
	unsigned int GetAirTargetKey(void) const;
	YSBOOL SetGroundTarget(class FsGround *trg);
	FsGround *GetGroundTarget(void) const;

	const YsVec3 &GetAaaMountPoint(void) const;
	const YsVec3 &GetSamMountPoint(void) const;
	const YsVec3 &GetCannonMountPoint(void) const;

	virtual YSBOOL IsAlive(void) const;
	virtual YSBOOL IsActive(void) const;
	YSBOOL IsStatic(void) const;
	YSBOOL IsControlledByUser(void) const;

	YSBOOL IsAntiGround(void) const;
	YSBOOL MustStayOnGround(void) const;
	YSBOOL IsNonGameObject(void) const;
	YSBOOL CanFloat(void) const;
	YSBOOL StayUpright(void) const;
	YSBOOL NoShadow(void) const;

	YSBOOL TrackAirTarget(void) const;
	YSBOOL TrackGndTarget(void) const;
	YSBOOL TargetAny(void) const;

	YSBOOL IsPapi(void) const;
	YSBOOL IsVasi(void) const;

	YSBOOL SkipGroundToGroundCollisionCheck(void) const;

	void SetupVisual(class FsVisualDnm &vis,const YsVec3 &viewPoint,const double &ctime) const;

	void CopyState(const FsGroundProperty &prop);
	void SetPosition(const YsVec3 &vec);
	void SetAttitude(const YsAtt3 &att);
	void SetPositionAndAttitude(const YsVec3 &vec,const YsAtt3 &att);
	void PutOnGround(void);

	void GetVelocity(YsVec3 &vel) const;
	void SetVelocity(const YsVec3 &newVel);
	void SetRelativeVelocity(const YsVec3 &newVel);
	const double GetVelocity(void) const;
	const double GetMaxSpeed(void) const;
	const double GetMaxStableSpeed(void) const;
	const double GetMinimumManeuvableSpeed(void) const;
	void SetRotation(const double &rot);
	const double GetRotation(void) const;
	YSBOOL IsDrifting(void) const;

	const double &GetVorRange(void) const;

	const double &GetNdbRange(void) const;

	void WriteRecord(class FsGroundRecord &rec) const;
	void ReadbackRecord(class FsGroundRecord &rec,const double &dt,const double &velocity);
	virtual void CaptureState(YsArray <YsString> &stateStringArray) const;

	void SetState(FSGNDSTATE sta);

	const YsVec3 GetUserViewPoint(void) const;
	YSBOOL HasTurret(void) const;
	YSBOOL GetHasAntiAirTurret(void) const;
	YSBOOL GetHasAntiGroundTurret(void) const;
	YSBOOL GetHasPilotControlledTurret(void) const;
	YSBOOL GetHasGunnerControlledTurret(void) const;
	YSRESULT GetFirstPilotControlledTurretDirection(YsVec3 &dir) const;
	YSRESULT GetFirstPilotControlledTurretPosition(YsVec3 &pos) const;

	YSBOOL IsFiringAaa(void) const;
	void StartFiringAaa(void);
	void StopFiringAaa(void);
	YSBOOL IsFiringCannon(void) const;
	void StartFiringCannon(void);
	void StopFiringCannon(void);

	virtual YSRESULT SendCommand(const char cmd[]);

public:

	double chMaxSpeed;
	double chMaxAccel;
	double chMaxSpeedRev;
	double chMaxAccelRev;
	double chMaxBrake;
	double chMaxRotation;
	double chConstHeadingRotation;
	double chConstPitchRotation;
	double chConstBankRotation;
	double chManSpeed1,chManSpeed2,chManSpeed3;
	double chTireRadius;

	int chSamReloadMax;
	double chSamReloadTime;

	double chAaaInterval;

	double chAaaMinAimPitch;
	double chAaaMaxAimPitch;
	double chSamMinAimPitch;
	double chSamMaxAimPitch;
	double chCanMinAimPitch;
	double chCanMaxAimPitch;

	double chGunPrecision;

	FSGROUNDTYPE chType;
	CANMOVEAREA chCanMoveArea;
	int chCanMoveRegionId;
	unsigned int chUserControlLevel;

	unsigned int chFlags;
	unsigned int chIsVisualLandingAid;
	YSBOOL chSkipGroundToGroundCollisionCheck;

	double chGunIniSpeed;
	double chAaaRange;
	double chCannonRange;

	double chAimRotationSpeed,chAimPitchSpeed;
	double chSAMRange;

	YsVec3 chAaaMount;
	YsVec3 chAaaGunnerPosition;
	YsVec3 chAaaGunnerOffset;
	int chNumAaaPosition;  // 0-> Not specified in .DAT.  Default value of 1 is used.
	                       // Never use chNumAaaPosition.  Use GetNumAaaPosition instead.
	YsVec3 chAaaPosition[MAXNUMAAAPOSITION];

	FSWEAPONTYPE chSAMType;
	YsVec3 chSamMount;
	YsVec3 chSamGunnerPosition;
	YsVec3 chSamGunnerOffset;
	int chNumSamPosition;  // 0-> Not specified in .DAT.  Default value of 2 is used.
	                       // Never use chNumSamPosition.  Use GetNumSamPosition instead.
	YsVec3 chSamPosition[MAXNUMSAMPOSITION];

	YsVec3 chCanMount;
	YsVec3 chCanGunnerPosition;
	YsVec3 chCanGunnerOffset;
	int chNumCanPosition;  // 0-> Not specified in .DAT.  Default value of 1 is used.
	                       // Never use chNumCanPosition.  Use GetNumCannonPosition instead.
	YsVec3 chCanPosition[MAXNUMCANPOSITION];

	YSBOOL chSyncAaaSamAim;

	YsVec3 chAimAt;

	double chVorRange;
	double chNdbRange;
	YSBOOL chIsDme;

	// chMinimumDamage
	//   Aircraft Carriers, Bridges do not get damage by machine guns
	//   In such cases, chMinimumDamage=1.
	int chMinimumDamage;

	YSRESULT LoadAircraftCarrierProperty(const wchar_t tmplRootDir[],const wchar_t fn[]);
	FsAircraftCarrierProperty *GetAircraftCarrierProperty(void);
	const FsAircraftCarrierProperty *GetAircraftCarrierProperty(void) const;

	FsGround *belongTo;

	unsigned int NetworkEncode(unsigned char dat[],int idOnSvr,const double &currentTime,YSBOOL shortFormat);
	void NetworkDecode(unsigned int packetLength,unsigned char dat[],double &t0,double &w0,const double &w);

	unsigned EncodeTurretState(unsigned char dat[],int idOnSvr) const;

	void ToggleLight(void);

	YSBOOL HasWeapon(void) const;
	FSWEAPONTYPE GetWeaponOfChoice(void) const;
	int GetNumAaaBullet(void) const;
	int GetNumCannon(void) const;
	int GetNumSAM(void) const;
	void SetNumAaaBullet(int numBullet);

	template <const int N>
	inline void GetWeaponConfig(YsArray <int,N> &loading) const;

	const YsAtt3 &GetCannonAim(void) const;
	const YsAtt3 &GetAaaAim(void) const;
	const YsAtt3 &GetSamAim(void) const;
	void SetCannonAim(const YsAtt3 &aim);
	void SetAaaAim(const YsAtt3 &aim);
	void SetSamAim(const YsAtt3 &aim);

protected:

	// Note : Aircraft Carrier Property will not be loaded until
	//        the ground object is added to the FsSimulation
	//        So, FsWorld is responsible for loading Aircraft Carrir Property.
	YSBOOL isAircraftCarrier;
	FsAircraftCarrierProperty *aircraftCarrierProperty;

	int GetNumAaaPosition(void) const;
	int GetNumSamPosition(void) const;
	int GetNumCannonPosition(void) const;
};

template <const int N>
inline void FsGroundProperty::GetWeaponConfig(YsArray <int,N> &loading) const
{
	loading.Clear();
	if(0<staGunBullet)
	{
		loading.Append(FSWEAPON_GUN);
		loading.Append(staGunBullet);
	}
	if(0<staSAM)
	{
		loading.Append(FSWEAPON_AIM9);
		loading.Append(staSAM);
	}
	if(0<staCannon)
	{
		// loading.Append(FSWEAPON_CANNON);
		// loading.Append(staCannon);
	}
}


/* } */
#endif
