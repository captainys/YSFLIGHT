#ifndef FSEXISTENCE_IS_INCLUDED
#define FSEXISTENCE_IS_INCLUDED
/* { */

#include <ysscenery.h>
#include "fsdef.h"
#include "fsvisual.h"
#include "fsrecord.h"
#include "fsnetwork.h"
#include "fsairplaneproperty.h"
#include "fsgroundproperty.h"
#include "fsatc.h"
#include "fssiminfo.h" // Need FsSimInfo::AirBase  


class FsAutopilot;

// Declaration /////////////////////////////////////////////
class FsExistence
{
private:
	static YSHASHKEY searchKeySeed;
	YSHASHKEY searchKey;
	YSBOOL bouncedLastTime;

protected:
	FsVisualSrf coll,collSrc;
	YsVec3 collCen,collBbx[2];
	double collRadius;

	// 2013/07/29 Initial State
	YSBOOL initialStateCaptured;
	YsArray <YsString> initialStateCommand;

	// 2014/09/20
	YsArray <YsString> reloadCommand;

	// 2014/09/27
	FsSimInfo::AirBase homeBase;
	// To ease the user from explicitly specifying home base for all the airplanes, YSFLIGHT takes:
	//   The first aircraft carrier that the airplane was loaded on (in YSRESULT FsAircraftCarrierProperty::LoadAirplane), or
	//   The first airport where the aircraft took off or landed. (in .... where?)


public:
	mutable YsVec2i fsLtcCache[2];  // Initialized in AddAirplane/AddGround, Updated in FsLattice::Add
	YSBOOL rectRgnCached;
	YsArray <const class YsSceneryRectRegion *,4> rectRgnCache;

	// Cached in SimComputeAirToObjCollision >>
	// At this time, cannot do the same for Gnd due to UpdateGroundCarrierLoading.
	class Collision
	{
	public:
		YSHASHKEY objKey;
		YsVec3 pos;
	};
	YsArray <Collision,8> airCollision;
	YsArray <Collision,8> gndCollision;
	// Cached in SimComputeAirToObjCollision <<

	mutable class FsVisualDnm vis;
	mutable class FsVisualDnm lod;
	mutable class FsVisualDnm cockpit;
	mutable class FsVisualDnm weaponShapeOverrideStatic[FSWEAPON_NUMWEAPONTYPE];
	mutable class FsVisualDnm weaponShapeOverrideFlying[FSWEAPON_NUMWEAPONTYPE];

	FSIFF iff;
	YsString _startPosition;
	YSBOOL isPlayingRecord,isNetSubstitute;
	YsString actualIdName;  // <= For net-substitute.  Airplane Identifier sent from the server.

	double elevation;  // 2007/08/15
	YsVec3 terrainOrg; // 2010/03/22
	YsVec3 terrainNom; // 2007/08/15

	double refTime1,refTime2;
	YsVec3 prevPos;
	double prevDt;

	int ysfId;  // <- .YSF ID (Id number exposed in a YSF file.)
	YsString name;


	// Below this line is for Networking

	// To detect delayed-packet, FsExistence must record of
	// packets' time of departure and time of arrival
	// Initial value should be -1.0
	FSNETTYPE netType;
	FsAirplane *netAirTarget;
	FsGround *netGndTarget;
	YSBOOL netAlive;
	int netDamageTolerance;
	// Usage of netAlive
	//   Set YSTRUE in FsExistence::Initialize(), which is always called when an object is added.
	//   In Server
	//      Set YSFALSE in BroadcastRemoveAirplane (Remote or Local)
	//      Set YSFALSE in BroadcastStateChange (Local)
	//      Set YSFALSE in ReceiveRemoveAirplane (Remote or Local)
	//
	//      BroadcastRemoveAirplane may be called from BroadcastAirplaneState,
	//      however, BroadcastRemoveAirplane is called from BroadcastAirplaneState
	//      only when netResendCount>0 which is possible for Local airplane.
	//      Hence, netAlive is not set YSFALSE in BroadcastAirplaneState.
	//
	//   In Client
	//      Set YSFALSE in SendStateChange(Local)
	//      Set YSFALSE in ReceiveRemoveAirplane (Remote or Local)

	// Following variable is to identify primary target
	// for ground attack airplanes.
	YSBOOL primaryTarget;

	// Motion Path
	const YsSceneryPointSet *motionPath;   // If motionPath!=NULL, only motion path name is saved.
	YsArray <YsVec3> motionPathPnt;         // Otherwise, motionPathPnt,motionPathIsLoop are saved.
	YSBOOL motionPathIsLoop;
	YSBOOL useMotionPathOffset;
	YsVec3 motionPathOffset;
	YSSIZE_T motionPathIndex;

protected:
	FsExistence();
	~FsExistence(); // Protected & Non-Virtual Destructor: Preventing deletion through base-class pointer.  See "C++ Coding Standard" pp. 90
	void CopyFrom(const FsExistence &from);

public:
	static const char *TypeToStr(FSEXISTENCETYPE t);
	static FSEXISTENCETYPE StrToType(const char str[]);

public:
	virtual FSEXISTENCETYPE GetType(void) const=0;
	YSBOOL IsAirplane(void) const;

	void SetHomeBaseName(FsSimInfo::BASE_TYPE baseType,const YsString &baseName);
	FsSimInfo::BASE_TYPE GetHomeBaseType(void) const;
	const YsString &GetHomeBaseName(void) const;

	static YSHASHKEY GetSearchKey(const FsExistence *obj);
	YSHASHKEY SearchKey(void) const;

	const char *GetIdentifier(void) const;
	const char *GetName(void) const;
	FSIFF GetIff(void) const;
	void SetIff(FSIFF iff);

	const char *GetStartPos(void) const;

	void Initialize(void);
	void CleanUp(void);

	void SetSearchKeyForNetworkSynchronizationPurpose(unsigned key);
	double GetRadiusFromCollision(void) const;

	const FsVisualSrf &TransformedCollisionShell(void) const;
	const FsVisualSrf &UntransformedCollisionShell(void) const;
	const YsVec3 *GetCollisionShellBbx(void) const;
	const YsVec3 &GetCollisionShellCenter(void) const;
	void SetCollisionShell(const FsVisualSrf &src);
	void SetTransformationToCollisionShell(const YsMatrix4x4 &mat);
	void ClearCollisionShell(void);
	YSBOOL MayCollideWith(const FsExistence &test,const double clearance=0.0) const;

	/*! A User Report indicated that I was wrongfully calculating propeller rotation of Class-3 objects based from state 0, 
	    not from the static state.  Class-3 objects are supposed to rotate heading rotation regardless of the state 0.
	    However, if the propeller is facing Z-axis and state 0 pitch was 90 degrees, bank and heading are mixed, and 
	    ends up making rotation about Z-axis.
	    So, the DnmState must be reset to state 0 once so that at least pitch is set to state 0 for Class-3 objects.
	*/
	void ResetDnmState(void);

	/*! This function tests if this object collides with test object 
	    when its own inverse matrix is ownInverse and the object is located at oriented as testMat. 
	    This function checks based on the bounding-box.  It does not check polygon-by-polygon.  */
	YSBOOL MayCollideWith(const YsMatrix4x4 &ownInverseMat,const FsExistence &test,const YsMatrix4x4 &testMat,const double clearance) const;

	YSBOOL TestTailStrike(YsVec3 &collisionPosInLocalCoordinate) const;

	virtual void Draw
		(int levelOfDetail,     // Zero:Most Detail 1,2,3,....:Rough
	     const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsVec3 &viewPos,
	     unsigned int drawFlag,
	     const double &ctime) const=0;
	virtual void DrawShadow
	    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm) const=0;

	void DrawApproximatedShadow
	    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm) const;

	virtual YSRESULT GetAttitudeFromRecord(YsAtt3 &att,const double &t) const=0;

	const double GetAGL(void) const;  // <= Never change it to "const double &".  Visual C++ cannot handle it.
	const double GetFieldElevation(void) const;

	void SetBouncedLastTimeFlag(void);
	void ClearBouncedLastTimeFlag(void);
	YSBOOL BouncedLastTime(void) const;

	virtual const FsVehicleProperty &CommonProp(void) const=0;
	virtual FsVehicleProperty &CommonProp(void)=0;

	const YsVec3 &GetPosition(void) const;
	YsVec3 GetCockpitPosition(void) const;
	const YsVec3 &GetLookAtOffset(void) const;
	const YsAtt3 &GetAttitude(void) const;
	const YsMatrix4x4 &GetMatrix(void) const;
	const YsMatrix4x4 &GetInverseMatrix(void) const;
	int GetNumAdditionalView(void) const;
	const class FsAdditionalViewpoint *GetAdditionalView(int id) const;
	double GetApproximatedCollideRadius(void) const;
	YSBOOL IsAlive(void) const;
	YSBOOL IsActive(void) const;

	YSRESULT AddReloadCommand(const char str[]);
	const YsArray <YsString> GetReloadCommand(void) const;

	const YsSceneryRectRegion *IsInsideRunway(void) const;

	virtual void ApplyControlAndGetFeedback(class FsFlightControl &userInput,FSUSERCONTROL userControl,YSBOOL autoRudder)=0;

	virtual YSBOOL LockOn(class FsSimulation *sim,const double &radarAltLimit)=0;
	virtual FSWEAPONTYPE GetWeaponOfChoice(void) const=0;

	virtual YSBOOL GetDamage(YSBOOL &killed,int dmg,FSDIEDOF diedOf)=0;
	virtual void Bounce(const YsVec3 &collPos)=0;

	void SetMotionPath(const YsSceneryPointSet *mpath);

	YSBOOL IsInDeadLockFreeAirport(void) const;

	YSBOOL IsInitialStateCaptured(void) const;
	void GetInitialStateString(YsArray <YsString> &initialStateString) const;
	virtual void CaptureInitialState(void){};
};

// Declaration /////////////////////////////////////////////
// FSSMOKETYPE definition has been moved to fs.h

// Air Flag definition is moved to fsdef.h

class FsAirplane : public FsExistence
{
// Memo: When adding new attribute, make sure to update CopyFrom accordingly.

protected:
	FsAirTrafficInfo airTrafficInfo;

public:
	double gLimit;   // <- For Radio Communication , Break and Attack
	unsigned int airFlag;
	double landWhenLowFuelThr;

	FsRecord <FsFlightRecord> *rec;  // NOTE : Must not be allocated until used.
	class FsInstrumentPanel *instPanel;   // <- instPanel is set in FsAirplane::SetProperty

	YsArray <YsString> cmdLog;

	YsListItem <FsAirplane> *thisInTheList;

	// For network mode >>
	FsNetReceivedAirplaneState netPrevState,netNextState;
	// For network mode <<

protected:
	FsAirplaneProperty prop;
	int defDamageTolerance; // <- defDamageTolerance is set in FsAirplane::SetProperty

	// Autopilot >>
	int curAutoPilotIdx;
	YsArray <FsAutopilot *> autoPilotList;
	// Autopilot <<

public:
	FsAirplane();
	FsAirplane(const FsAirplane &from);
	void CopyFrom(const FsAirplane &from);
	const FsAirplane &operator=(const FsAirplane &from);
	virtual ~FsAirplane();

public:
	virtual FSEXISTENCETYPE GetType(void) const;

	void Initialize(void);
	void CleanUp(void);

	void MakeVaporVertexArray(class YsGLVertexBuffer &vtxBuf,class YsGLColorBuffer &colBuf,double currentTime,double remainTime,int step) const;
	void MakeSmokeVertexArray(class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,double currentTime,double remainTime,FSSMOKETYPE smk,int step) const;
private:
	void AddSingleSmokeVertexArray(
	    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
	    int smkId,double currentTime,double remainTime,FSSMOKETYPE smk,int step) const;
	void AddSmokeRect(
	    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
	    int smkIdx,
	    YsVec3 &p0,YsVec3 &p1,YsVec3 &p2,YsVec3 &p3,
	    YsVec3 &n0,YsVec3 &n1,YsVec3 &n2,YsVec3 &n3,
	    const double &a0,const double &a1,const double &a2,const double &a3) const;
	void AddTowelSmoke(
	    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
	    int smkIdx,
	    YsVec3 &p0,YsAtt3 &a0,const double &r0,
	    YsVec3 &p1,YsAtt3 &a1,const double &r1,
	    const double &alpha0,const double &alpha1) const;
	void AddSolidSmoke(
	    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
	    int smkIdx,
	    YsVec3 &p0,YsAtt3 &a0,const double &r0,
	    YsVec3 &p1,YsAtt3 &a1,const double &r1,
	    const double &alpha0,const double &alpha1) const;
	void AddSolidSmokeLid(
	    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
	    int smkIdx,YsVec3 &p0,YsAtt3 &a0,const double &r0,const double &alpha0) const;


public:
	virtual void Draw
	    (int levelOfDetail,  // Zero:Most Detail 1,2,3,....:Rough
	     const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsVec3 &viewPos,
	     unsigned int drawFlag,
	     const double &ctime) const;
	virtual void DrawShadow
	    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm) const;

	void AddSmokeToParticleManager(class YsGLParticleManager &partMan,double currentTime,double remainTime) const;
	void AddSingleSmokeToParticleManager(class YsGLParticleManager &partMan,int smkId,double currentTime,double remainTime) const;

	void DrawSmoke(double currentTime,double remainTime,FSSMOKETYPE smk,int d,YSBOOL transparency) const;
	void DrawSingleSmoke(int smkId,double currentTime,double remainTime,FSSMOKETYPE smk,int d,YSBOOL transparency) const;
	void DrawVapor(double currentTime,double remainTime,int d,YSBOOL transparency) const;
	YSBOOL HitGround
	  (FSDIEDOF &diedOf,
	   int &collType,  // 1:Ground  2:Shell
	   double ctime,const FsField &fieldLst,YSBOOL takeCrash,YSBOOL canLandAnywhere,
	   class FsExplosionHolder *explosion);

	void Crash(const double &ctime,YSBOOL crashedMeansDead,FsExplosionHolder *explosion,FSDIEDOF diedOf,YSSCNAREATYPE areaType);
	void Overrun(const double &ctime);

	YSRESULT Record(const double &t,YSBOOL forceRecord);
	YSRESULT Record(const double &t,class FsFlightRecord &rec,YSBOOL forceRecord);
	YSRESULT PlayRecord(const double &ct,const double &dt);
	virtual YSRESULT GetAttitudeFromRecord(YsAtt3 &att,const double &t) const;
	const FsFlightRecord *LastAddedRecord(void) const;
	FSFLIGHTSTATE GetFinalState(void) const;
	const YsVec3 &GetFinalPosition(YsVec3 &pos) const;

	template <const int N>
	inline YSBOOL IsApproachingRunway(FSTRAFFICPATTERNLEG &leg,YsArray <const class YsSceneryRectRegion *,N> &runwayRect,YsVec3 &tdPos) const;

	int FindLandingWithinTimeRange(YsArray <YSSIZE_T> &recordIndexArray,const double t1,const double t2) const;

	virtual void ApplyControlAndGetFeedback(class FsFlightControl &userInput,FSUSERCONTROL userControl,YSBOOL autoRudder);

	virtual YSBOOL LockOn(class FsSimulation *sim,const double &radarAltLimit);
	virtual FSWEAPONTYPE GetWeaponOfChoice(void) const;

	virtual YSBOOL GetDamage(YSBOOL &killed,int dmg,FSDIEDOF diedOf);
	virtual void Bounce(const YsVec3 &collPos);

	YSRESULT RipOffEarlyPartOfRecord(void); // for demo mode
	int RerecordByNewInterval(const double &itvl);
	void AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng);

	YSRESULT SetProperty(const class FsAirplaneProperty &prp,const wchar_t tmplRootDir[]);
	const class FsAirplaneProperty &Prop(void) const;
	int GetDefaultDamageTolerance(void) const;
	class FsAirplaneProperty &Prop(void);
	virtual FsVehicleProperty &CommonProp(void);
	virtual const FsVehicleProperty &CommonProp(void) const;

	YSRESULT ClearAutopilot(void);
	YSRESULT SetAutopilot(FsAutopilot *ap);
	YSRESULT AddAutoPilot(FsAutopilot *ap);
	const FsAutopilot *GetAutopilot(void) const;
	FsAutopilot *GetAutopilot(void);
	const double &GetLandWhenLowFuel(void) const;
	YSBOOL NextAutoPilotAvailable(void) const;
	YSRESULT MoveOnToNextAutoPilot(void);
	YSRESULT SaveAutoPilot(FILE *fp,const class FsSimulation *sim);

	YSRESULT RecallReloadCommandOnly(void);

	YSRESULT SendCommand(const char str[]);
	YSRESULT AutoSendCommand(YSSIZE_T nWeaponConfig,const int weaponConfig[],int fuel);
	YSRESULT AutoSendCommand(YSSIZE_T nWeaponConfig,const int weaponConfig[]);
	YSRESULT RecallCommand(void);

	// See memo/designmen/flightschedule.txt
	YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE GetCurrentDestination(YsString &label) const;
	YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE GetOccupyingAirportOrFix(YsString &label) const;

public:
	const FsAirTrafficInfo &GetAirTrafficInfo(void) const;  // airTrafficInfo
	FsAirTrafficInfo &GetAirTrafficInfo(void);  // airTrafficInfo
	YSRESULT AtcRequestIlsApproach(class FsSimulation *sim,class FsAirTrafficController *atc,class FsGround *ils);
};

#include "fsautopilot.h"

template <const int N>
inline YSBOOL FsAirplane::IsApproachingRunway(FSTRAFFICPATTERNLEG &leg,YsArray <const class YsSceneryRectRegion *,N> &runwayRect,YsVec3 &tdPos) const
{
	if(YSTRUE!=Prop().IsOnGround())
	{
		const FsAutopilot *ap=GetAutopilot();
		if(NULL!=ap)
		{
			if(FSAUTOPILOT_LANDING==ap->Type())
			{
				const FsLandingAutopilot *ldg=(const FsLandingAutopilot *)ap;
				if(0<ldg->runwayRectCache.GetN())
				{
					runwayRect=ldg->runwayRectCache;
					leg=ldg->GetCurrentLeg();
					tdPos=ldg->tdPos;
					return YSTRUE;
				}
			}
			else if(FSAUTOPILOT_AIRWAY==ap->Type())
			{
				const FsAirRouteAutopilot *airRoute=(const class FsAirRouteAutopilot *) ap;
				return airRoute->IsApproachingRunway(leg,runwayRect,tdPos);
			}
		}
	}
	leg=FSLEG_NOT_IN_PATTERN;
	return YSFALSE;
}



// Declaration /////////////////////////////////////////////

class FsGround : public FsExistence
{
private:
	YsArray <class FsAutoDrive *> autoDriveStack;  // <- Auto drive will be used from DefaultControl.

public:
	FsGround();
	FsGround(const FsGround &from);
	virtual ~FsGround();

	virtual FSEXISTENCETYPE GetType(void) const;

	void Initialize(void);
	void CleanUp(void);

	void GetWhereToAim(YsVec3 &pos) const;

	virtual void Draw
	    (int levelOfDetail,
	     const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsVec3 &viewPos,
	     unsigned int drawFlag,
	     const double &ctime) const;
	virtual void DrawShadow
	    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm) const;

	virtual YSRESULT GetAttitudeFromRecord(YsAtt3 &att,const double &t) const;

	YSRESULT SetProperty(const class FsGroundProperty &prp);
	FsGroundProperty &Prop(void);
	const FsGroundProperty &Prop(void) const;
	virtual FsVehicleProperty &CommonProp(void);
	virtual const FsVehicleProperty &CommonProp(void) const;

	void ProcessLoadingOnAircraftCarrier(YSSIZE_T nCarrier,FsGround *carrier[]);
	YSBOOL PiggyBack(void) const;

	YSRESULT Settle(const YsVec3 &pos);
	YSRESULT Settle(const YsAtt3 &att);

	YSRESULT Record(const double &t,YSBOOL forceRecord);
	YSRESULT Record(const double &t,FsGroundRecord &rec,YSBOOL forceRecord);
	YSRESULT PlayRecord(const double &ct,const double &dt);
	const FsGroundRecord *LastAddedRecord(void) const;
	FSGNDSTATE GetFinalState(void) const;
	const YsVec3 &GetFinalPosition(YsVec3 &pos) const;
	const YsAtt3 &GetFinalAttitude(YsAtt3 &att) const;

	void SetAutoDrive(FsAutoDrive *drv); // FsGround will take ownership of the auto-drive
	void PushAutoDrive(FsAutoDrive *drv); // FsGround will take ownership of the auto-drive
	void DeleteLastAutoDrive(void);
	int GetNumAutoDrive(void) const;
	YSBOOL IsAutoDrivingTo(const FsExistence *objPtr) const;
private:
	void ClearAutoDrive(void);

public:
	YSRESULT DefaultControl(FsSimulation *sim,const double &dt);
	YSRESULT AimAt(
	    YsAtt3 &aim,
	    const YsAtt3 &prevAim,
	    const YsVec3 &trg,const YsVec3 &gunMount,const double &dt,const double &minPitch,const double &maxPitch);
	void SearchTarget(FsSimulation *sim);

	virtual void ApplyControlAndGetFeedback(class FsFlightControl &userInput,FSUSERCONTROL userControl,YSBOOL autoRudder);

	virtual YSBOOL LockOn(FsSimulation *sim,const double &radarAltLimit);
	virtual FSWEAPONTYPE GetWeaponOfChoice(void) const;

	virtual YSBOOL GetDamage(YSBOOL &killed,int dmg,FSDIEDOF diedOf);
	virtual void Bounce(const YsVec3 &collPos);

	YSRESULT RipOffEarlyPartOfRecord(void); // for demo mode
	int RerecordByNewInterval(const double &itvl);
	void AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng);

	FsRecord <FsGroundRecord> *rec;  // NOTE : Must not be allocated until used.
	YsVec3 initPosition;
	YsAtt3 initAttitude;

	YSRESULT SendCommand(const char str[]);
	YSRESULT RecallCommand(void);
	YsArray <YsString> cmdLog;
	unsigned gndFlag;
	double nextTargetSearchTime;

	YsListItem <FsGround> *thisInTheList;

protected:
	FsGroundProperty prop;


public:
	// For network play
	YsVec3 netPos;
	YsAtt3 netAtt;
	YSBOOL netShootingAaa,netShootingCannon;
	double netClockRemote,netClockLocal;
};



// **** Never forget overriding GetPosition,GetAttitude function
// **** When implementing FsGroundObject



/* } */
#endif
