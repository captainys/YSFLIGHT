#ifndef FSAUTOPILOT_IS_INCLUDED
#define FSAUTOPILOT_IS_INCLUDED
/* { */


#include <fsdef.h>

class FsAutopilot *FsReadIntention(YsTextInputStream &inStream);
class FsAutopilot *FsReadIntention(FILE *fp);

class FsExistence;
class FsAirplane;
class FsGround;


// In the future, add "EMR_ATTACKEDBYENEMY"

typedef enum
{
	EMR_NONE,
	EMR_LOWALTITUDE,
	EMR_HIGHLOWPITCH,
	EMR_STALL
} FSEMERGENCYRECOVERY;


// Declaration /////////////////////////////////////////////
class FsAutopilot
{
public:
	class Trigger
	{
	public:
		enum
		{
			TRIGGER_NONE=0,
			TRIGGER_ENEMY_IN_RANGE=2,
		};

		unsigned int triggerFlag;
		double enemy_in_range_threshold;

		Trigger();
	};

	class AutoFlareDispenser
	{
	private:
		double nextDispensingTimer;
		double dispensingInterval;
		double missileDistanceThreshold;
		int timerUpCounter;
		YSBOOL flareBtn;

		YSBOOL prevMissileChasing,missileChasing;
		YsVec3 missilePos;

	public:
		AutoFlareDispenser();
		void SetDispensingInterval(double t);
		void SetMissileDistanceThreshold(double dist);
		int GetTimerUpCounter(void) const;
		YSBOOL MissileChasing(void) const;
		void SetFlareInterval(double t);
		double GetFlareInterval(void) const;
		YSBOOL GetFlareButton(void) const;
		virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
		virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	};

private:
	FsAutopilot(const FsAutopilot &);
	FsAutopilot &operator=(const FsAutopilot &);

public:
	static const char *const intentionList[];

	virtual FSAUTOPILOTTYPE Type(void) const=0;
	virtual YSBOOL IsTakingOff(void) const;
	virtual YSBOOL IsLanding(void);
	virtual YSBOOL DoesRespondToRadioCall(void) const; // By default true.

protected:
	FsAutopilot();
	virtual ~FsAutopilot();
public:
	static void Delete(FsAutopilot *ap);

	static FsAutopilot *ReadIntentionEZ(YsConstArrayMask <YsString> argv);


	FSEMERGENCYRECOVERY emr;
	double minAlt;
	double thr;

	virtual unsigned OverridedControl(void);
	virtual YSBOOL ObjectiveAccomplished(void);

	YSRESULT Control(class FsAirplane &air,class FsSimulation *sim,const double &dt);
	YSRESULT Save(FILE *fp,const FsSimulation *sim);

	YSRESULT GetRelativeAttitude
	    (YsAtt3 &rel,const YsAtt3 &attOfInterest,FsAirplane &air);
	double GetRelativeBank
	    (const YsAtt3 &attOfInterest,const YsAtt3 &withRespectTo);

	int FindNumberInFormationWighRespectToPlayer(FsAirplane &air,FsSimulation *sim);

	double GetAllowableAltitude(const double &minAlt,const FsAirplane &air);

protected:
	// For chasing a target >>
	double iTheataK,prevTheataK;
	double prevYaw;
	double iTheataL,prevTheataL;
	double tInverted,tRecoveryFromInverted;
	YsVec3 refUvForRecoveryFromInverted;
	// <<

	// For ShallowPursuit >>
	int shallowPursuitState;
	double shallowPursuitStateChangeTimer;
	// <<


	FsAutopilot *nextObjective;

	YSRESULT EmergencyRecovery(FsAirplane &air);
public:
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);

protected:
	YSRESULT RecoverFromLowAltitude(FsAirplane &air,const double &alt,const double gLimit);
	YSRESULT RecoverFromHighLowPitch(FsAirplane &air);
	YSRESULT RecoverFromStall(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)=0;
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)=0;
	virtual YSBOOL Accomplished(void);

	YSRESULT CircleAround(FsAirplane &air,FsSimulation *sim,const double &alt,const double &bnkLimit=YsPi/2.0);
	YSRESULT ControlGForAltitude
	   (FsAirplane &air,FsSimulation *sim,const double &alt,const double &climbRate,const double &GLimit=3.0);
	YSRESULT ControlGForVerticalSpeed(FsAirplane &air,FsSimulation *sim,const double &vAlt,const double &GLimit=3.0);
	YSRESULT ControlGForRollOut
	   (FsAirplane &air,FsSimulation *sim,const double &alt,
	    const double &maxClimbAngle,const double &maxDescendAngle,
	    const double &maxGLimit,const double &minGLimit);
	void ShallowPursuit
	   (FsAirplane &air,FsSimulation *sim,
	    const YsVec3 &target,const YsAtt3 &targetAtt,const YsVec3 &targetVelocity,const double &targetG,
	    const double &dt,const double &maxG,const double &minG,
	    YSBOOL aimingGun);   // this returns deviation in angle relative to the target
	double FollowTarget
	   (FsAirplane &air,FsSimulation *sim,
	    const YsVec3 &target,const YsAtt3 &targetAtt,const YsVec3 &targetVelocity,const double &targetG,
	    const double &dt,const double &maxG,const double &minG,
	    YSBOOL aimingGun);   // this returns deviation in angle relative to the target
	YSBOOL NeedImmediateRecoveryFromLowAltitude(FsAirplane &air,const double &minAlt,const double gLimit);



	YSRESULT VTOLHover(FsAirplane &air,FsSimulation *sim,const YsVec3 &desigPos,const YsVec3 &speedPlus);

	void DecelerationAfterLanding(FsAirplane &air,FsSimulation *sim,const double &dt);

	YSBOOL DangerOfCollision(FsAirplane &air1,FsAirplane &air2) const;

public:
	virtual YSBOOL MissionAccomplished(FsAirplane &air,FsSimulation *sim) const;
	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadCommonProperty(YSBOOL &endIntention,YSSIZE_T ac,const YsString av[]);
};


////////////////////////////////////////////////////////////

class FsTaxiingAutopilot : public FsAutopilot
{
friend class FsLandingAutopilot;

public:
	enum TAXIINGMODE
	{
		MODE_NORMAL,
		MODE_TAKEOFF,
		MODE_TAKEOFF_ON_CARRIER,
	};

	/*
	In MODE_TAKEOFF_ON_CAERRIER mode, 
	  - taxiPath will be relative to the carrier, and the speed will be up to 2kt.
	  - taxiPath will be calculated in MakeDecision, therefore do not set taxi path by SetTaxiPath.
	*/

private:
	TAXIINGMODE mode;

	double speedOnRunway;
	double fastTaxiSpeed;
	double slowTaxiSpeed;
	double lastTaxiSpeed;

	int taxiPathIdx;
	YsArray <YsVec3,16> taxiPath;
	YSBOOL holdShortOfRunway;

	// Variables calculated in MakeDecision >>
	YSBOOL canGoForward;
	YsVec3 taxiDir;
	double desiredTaxiSpeed;

	// Runway-incursion check >>
	YsArray <YsVec3,16> samplePointArray;
	YsArray <YSBOOL,16> incursionTestResultArray;
	// Runway-incursion check <<

	// For MODE_TAKEOFF >>
	YsVec3 runwayCenterLineO,runwayCenterLineV;  // Calculated when taxiPathIdx is incremented to the last leg.
	double runwayHeading;
	double runwayAlignmentTimer;
	// For MODE_TAKEOFF <<
	// Variables calculated in MakeDecision <<

protected:
	FsTaxiingAutopilot();

public:
	static FsTaxiingAutopilot *Create(void);
	void Initialize(void);

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	void SetMode(TAXIINGMODE mode);
	TAXIINGMODE GetMode(void) const;
	void SetTaxiPath(int nPnt,const YsVec3 pnt[]);
	void SetTaxiPathIndex(int idx);
	void AddTaxiPathPoint(const YsVec3 &pnt);
	void SetSpeedOnRunway(const double spd);
	void SetFastTaxiSpeed(const double spd);
	void SetSlowTaxiSpeed(const double spd);
	void SetLastTaxiSpeed(const double spd);
	void SetSteepTurningTaxiSpeed(const double spd);

	YSRESULT GetRunwayCenterLine(YsVec3 &o,YsVec3 &v) const;
	void SetHoldShortOfRunway(YSBOOL holdShort);

	YSBOOL IsTurningToAlignForRunwayHeading(void) const;

	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_TAXIING;}
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSBOOL MissionAccomplished(FsAirplane &air,FsSimulation *sim) const;
private:
	template <const YSSIZE_T N>
	inline void GetNextSamplePointArrayFor160m(YsArray <YsVec3,N> &samplePointArray,const FsAirplane &air,const YsMatrix4x4 &pathTfm) const;

	inline void GetNextSamplePointAndDirection(YsVec3 &pos,YsVec3 &dir,const FsAirplane &air,const double distAhead,const YsMatrix4x4 &pathTfm) const;
};

// Declaration /////////////////////////////////////////////
class FsGotoPosition : public FsAutopilot
{
public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_GOTO;}

protected:
	FsGotoPosition();
	virtual ~FsGotoPosition();
public:
	enum STATE
	{
		STATE_NORMAL,
		STATE_TOOHIGH,
		STATE_CAPTURE,  // Current position will be captured in the next time step.
	};

	static const char *StateToStr(STATE state);
	static STATE StrToState(const char *str);

	static FsGotoPosition *Create(void);
	virtual unsigned OverridedControl(void);

	void SetSingleDestination(const YsVec3 &pos);
	void SetSingleDestination(const double &x,const double &y,const double &z);
	const YsVec3 &GetNextWayPoint(void) const;

	void SetUseAfterburner(YSBOOL useAB);
	YSBOOL GetUseAfterburner(void) const;

	void SetThrottle(const double thr);

	void SetSpeed(const double speed);

	STATE state;
	int destinationIdx;
	YsArray <YsVec3> destination;
	double speed,throttle;
	YSBOOL useAfterburner;
	YSBOOL straightFlightMode,flyHeadingBugMode;

	int forcedTurn;
	double radial;
	YsVec3 rel;  // Horizontal, Relative destination

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);

	virtual YSBOOL MissionAccomplished(FsAirplane &air,FsSimulation *sim) const;
};



////////////////////////////////////////////////////////////

#include "fsdogfightautopilot.h"
#include "fsgroundattackautopilot.h"
#include "fsformationautopilot.h"

////////////////////////////////////////////////////////////

enum
{
	FSACRO_NONE,
	FSACRO_CORKSCREW,
	FSACRO_SLOWROLL,
	FSACRO_DELTALOOP,
	FSACRO_DELTAROLL,
	FSACRO_TIGHTTURN,
	FSACRO_360ANDLOOP,
	FSACRO_BOMBBURST4SHIP,
	FSACRO_CHANGEOVERTURN,
	FSACRO_TRAILTODIAMONDROLL,
	FSACRO_CUBANEIGHT,
	FSACRO_DELTALOOPANDBONTON,
	FSACRO_BONTONROLL,
	FSACRO_BOMBBURSTDOWN4SHIP,
	FSACRO_BOMBBURSTDOWN6SHIP,
	FSACRO_ROLLINGCOMBATPITCH,
	FSACRO_DIAMONDTAKEOFF,
	FSACRO_CONTINUOUSROLL,
	FSACRO_ROLLONTAKEOFFANDHALFCUBAN,
	FSACRO_TACKCROSSANDVERTICALCLIMBROLL,
	FSACRO_BIGHEART,
	FSACRO_LEVELBREAK,
	FSACRO_ROLLBACKTOARROWHEAD,
	FSACRO_PITCHUPBREAK,
	FSACRO_RAINFALL,
	FSACRO_ROCKWINGCLEAN,
	FSACRO_ROCKWINGDIRTY,
	FSACRO_LETTEREIGHT,
	FSACRO_STARCROSS,
	FSACRO_BOMBBURST6SHIP,
	FSACRO_LEVELOPENER,
	FSACRO_FORMATIONBREAK,
	FSACRO_LINEABREASTLOOP,
	FSACRO_LINEABREASTROLL,
	FSACRO_DOUBLEFARVEL,
	FSACRO_DIAMOND9TOSWANBEND,
	FSACRO_SWANTOAPOLLOROLL,
	FSACRO_LANCASTERTO5_4SPLIT,
	FSACRO_CHAMPAIGNSPLIT,
	FSACRO_VIXENBREAK,
	FSACRO_BIGBATTLETOSHORTDIAMONDLOOP,
	FSACRO_STAROFDAVID,

// When a new acro is added, add a label in FsAcroTypeNameTable::FsAcroTypeNameTable()
FSACRO_NUMACROTYPE
};

class FsAcroTypeName
{
public:
	FsAcroTypeName();
	void Set(int t,const char n[]);
	int acroType;
	const char *acroName;
};

class FsAcroTypeNameTable
{
public:
	FsAcroTypeNameTable();
	void Add(int t,const char n[]);
	void Verify(void);
	int GetAcroType(const char *str);
	const char *GetAcroName(int acroType);
	YsArray <FsAcroTypeName> typeNameList;
};

class FsAirshowControl : public FsFormation
{
public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_AIRSHOW;}

	enum
	{
		NOACTION,
		CORKSCREW,
		BARRELROLL,
		LOOP,
		TURN360,
		BOMBBURST4SHIP,     // Bomb burst uses FsFormation's parameters until breaking.
		CHANGEOVERTURN,     // 6-Ship Change over turn #1 position
		TRAILTODIAMONDROLL, // Trail to diamond roll #1 position
		CUBANEIGHT,
		DELTALOOPANDBONTON,
		BONTONROLL,
		BOMBBURSTDOWN4SHIP,
		BOMBBURSTDOWN6SHIP,
		ROLLINGCOMBATPITCH,
		PITCHUPBREAK,
		DIAMONDTAKEOFF,
		CONTINUOUSROLL,
		ROLLONTAKEOFFANDHALFCUBAN,
		TACKCROSSANDVERTICALCLIMBROLL,
		BIGHEART,
		LEVELBREAK,
		ROLLBACKTOARROWHEAD,
		RAINFALL,
		ROCKWING,
		LETTEREIGHT,
		STARCROSS,
		BOMBBURST6SHIP,
		LEVELOPENER,
		FORMATIONBREAK,
		LINEABREASTLOOP,
		DOUBLEFARVEL,
		DIAMOND9TOSWANBEND,
		SWANTOAPOLLOROLL,
		LANCASTERTO5_4SPLIT,
		CHAMPAIGNSPLIT,
		VIXENBREAK,
		MIRRORROLL,
		BIGBATTLETOSHORTDIAMONDLOOP,
		STAROFDAVID
	};


	// Positions
	//    1
	//   2 3
	//  5 4 6

	// Diamond 9
	//     1
	//    2 3
	//   5 4 6
	//    7 8
	//     9

	// Champaign
	//     1
	//    2 3
	//   4   5
	//    6 7
	//    8 9
	enum
	{
		NFORMATION=9
	};
	FsAirplane *formation[NFORMATION];


	int action;
	YSBOOL endOfAction;
	double waitTimer,smokeOffTimer,pitchIntegrator,rollIntegrator,headingIntegrator;
	int fomPosition;
	double entryAlt,entrySpeed,entryHeading;
	double turnDir;

	// Attributes for corkscrew
	YsVec3 cswRefPoint,cswRefDirection;
	double cswRadius;
	int cswMode;

	// Attribute for barrel roll , Trail to diamond roll, bonton roll, Roll Back to Arrowhead
	int brlMode;   // 0:Straight   1:Pull up to 30 degree  2:Roll until inverted  3:Recover  4:Straight
	double brlRate;
	double brlG;
	double brlPathAltitude[18];

	// Attribute for loop, cuban eight, Lancaster to 5-4 split, Vixen Break
	int loopMode;
	double loopG,loopThr;
	double loopPathAltitude[18];  // loopPathAltitude[0] will be used as speed for cuban eight

	// Attribute for 360 degree turn, Letter Eight
	int turnMode;
	double turnBankAngle;
	YSBOOL turnAndLoop;
	double turnEntryHeading;
	double turnEntrySpeed;
	int letterEightBoost;

	// Attributes for 4 ship bomb burst (#5 will be the spear), vertical heart, level break,
	// and Champaign Split
	// bombBurstDesigUv1 is also used by LancasterTo5_4Split
	int bombBurstMode;
	double bombBurstLevelSpeed,bombBurstEntrySpeed;
	YsVec3 bombBurstDesigUv1,bombBurstDesigUv2,bombBurstEntryPoint,bombBurstBreakPoint;
	YsVec3 bombBurstClimbPath[9],bombBurstRefPoint;

	// Attributes for change over turn
	// Also used by Diamond9ToSwanBend
	int changeOverTurnMode;
	double changeOverTurnBankAngle;
	double changeOverTurnAltitude,changeOverTurnEntryHeading,changeOverTurnEntrySpeed;

	// Rolling Combat Pitch, Pitch Up Break
	int cpMode;
	YSBOOL cpLeftTurn;
	double cpEntryHeading;

	// Diamond Take Off , Roll on Take off & Half Cuban
	int dtoMode;
	double dtoSpeed;
	double dtoStartAlt;

	// Tack Cross and Vertical Climb Roll
	int tcMode;
	double tcEntryHeading,tcEntryAltitude,tcEntrySpeed;
	YsVec3 tcBreakPoint,tcBreakVec,tcCrossPoint;

	// Rock Wing
	int rwMode;
	YSBOOL rwDirty;

	// Inverted, Double Farvel, Mirror Roll
	int ivMode;

protected:
	FsAirshowControl();
public:
	static FsAirshowControl *Create(void);
	void InitializeParameter(void);
	static YSRESULT GetDeltaPosition(YsVec3 &pos,int no,const double &r);
	static YSRESULT GetDiamond9Position(YsVec3 &pos,int no,const double &r);
	static YSRESULT GetSwan9Position(YsVec3 &pos,int no,const double &r);
	static YSRESULT GetApollo9Position(YsVec3 &pos,int no,const double &r);
	static YSRESULT GetArrowheadPosition(YsVec3 &pos,int no,const double &r);
	static YSRESULT GetLancasterPosition(YsVec3 &pos,int no,const double &r);
	static YSRESULT GetChampaignPosition(YsVec3 &pos,int no,const double &r);
	static FsFormation *GetFormationAP(FsAirplane *air);
	static FsAirshowControl *GetAirshowAP(FsAirplane *air);

	YSRESULT SwitchLeader(FsAirplane *newLeader,FsAirplane *newWingman);

	virtual unsigned OverridedControl(void);
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionCorkScrew(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlCorkScrew(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionBarrelRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlBarrelRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionLoop(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlLoop(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecision360Turn(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControl360Turn(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecision4ShipBombBurst(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControl4ShipBombBurst(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionChangeOverTurn(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlChangeOverTurn(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionCubanEight(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlCubanEight(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionBontonRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlBontonRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionBombBurstDownward(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlBombBurstDownward(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionRollingCombatPitch(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlRollingCombatPitch(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionDiamondTakeoff(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlDiamondTakeoff(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionContinuousRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlContinuousRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionRollOnTakeoff(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlRollOnTakeoff(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionTackCrossAndVerticalClimbRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlTackCrossAndVerticalClimbRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionBigHeart(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlBigHeart(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionLevelBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlLevelBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionRollBackToArrowhead(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlRollBackToArrowhead(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionPitchUpBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlPitchUpBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionRockWing(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlRockWing(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionLetterEight(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlLetterEight(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionStarCross(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlStarCross(FsAirplane &air,FsSimulation *sim,const double &dt);
	FsAirplane *GetStarCrossRefAir(int fomPosition) const;
	YSRESULT MakeDecisionLevelOpener(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlLevelOpener(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionFormationBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlFormationBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionDoubleFarvel(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlDoubleFarvel(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionDiamond9ToSwanBend(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlDiamond9ToSwanBend(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionSwanToApolloRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlSwanToApolloRoll(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionLancasterTo5_4Split(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlLancasterTo5_4Split(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionChampaignSplit(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlChampaignSplit(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionVixenBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlVixenBreak(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT MakeDecisionBigBattleToShortDiamondLoop(FsAirplane &air,FsSimulation *sim,const double &dt);
	YSRESULT ApplyControlBigBattleToShortDiamondLoop(FsAirplane &air,FsSimulation *sim,const double &dt);
};


////////////////////////////////////////////////////////////

class FsLandingAutopilot : public FsAutopilot
{
public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_LANDING;}

	enum
	{
		PHASE_FLYINGTOENTRY=0,
		PHASE_GOING_TO_TPA=1,
		PHASE_TAKING_DISTANCE=2,
		PHASE_TURN_TO_DOWNWIND=3,
		PHASE_DOWNWIND=4,
		PHASE_DOWNWIND_BEAM=5,
		PHASE_DOWNWIND_TO_BASE=6,
		PHASE_BASE=7,
		PHASE_BASE_TO_FINAL=8,
		PHASE_SHORTFINAL=9,
		PHASE_CLEARINGRUNWAY_WITH_TAXIPATH=10,
		PHASE_CLEARINGRUNWAY_SEARCHTAXIWAY=11,
		PHASE_FULLSTOP=12,
		PHASE_CLEARING_CARRIER_DECK=13,

		PHASE_EMERGENCY_STALL=100,
		PHASE_EMERGENCY_NO_ILS=101,
		PHASE_WAITING_UNTIL_CARRIER_TURN_STOP=102,
	};

	enum APPROACHTYPE
	{
		APPROACH_UNDECIDED,
		APPROACH_ILS,
		APPROACH_VISUAL
	};

	// 2008/02/24
	//   All member variables must once be initialized in the constructor

	YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE destinationSegTypeCache;
	YsString destinationSegLabelCache;

	int nDrop;
	double phaseTimer;
	double lateralOffset;
	YSBOOL autoGoAround,autoClearRunway;
	YSBOOL alwaysGoAround;
	double flareAlt;
	YSBOOL wheelControlOnGround;

	YSBOOL useRunwayClearingPathIfAvailable;

	// Control if the airplane must stop when far enough away from the runway centerline. >>
	YSBOOL dontStopAtFarEnoughPosition;
	double stopTaxiWhenThisFarAwayFromRunwayCenterline;
	// Control if the airplane must stop when far enough away from the runway centerline. <<

	APPROACHTYPE approachType;
	int landingPhase,prevLandingPhase;
	YsString ilsName;  // Prioritize ils search.
	YsString vfrName;  // Prioritize ils search.
	const FsGround *ils;
	YsArray <const YsSceneryRectRegion *,2> runwayRectCache;
	YSBOOL initializedAirplaneInfo;

	double landingSpeed,landingSpeedCorrection;   // <- After lowering flap
	double entrySpeed,entryTurnRadius;            // <- Before lowering flap (must fly a little faster)
	double requiredRunwayLength;
	double groundHeight;
	YsVec3 tdPos;
	YsAtt3 rwAtt;
	double bankLimit,turnRadius,turnRatio,trafficPatternAltitude;
	double trafficPatternAltitudeAGL;  // Set in SetAirplaneInfo
	double verticalSeparation;         // Set in SetAirplaneInfo
	double glideSlope;

	double nintyTurnTime,baseLegTime,downwindLegTime,finalLegTime;

	// Mode -2:  STALL

	// Mode -1:  ILS not found.  I don't know what to do.

	// Mode 0:  Head to the entry point
	YsVec3 entryPoint;                  // Must be set before PHASE_FLYINGTOENTRY.  Point at which the airplane rolls out to the downwind.
	YsVec3 entryTurnCenter;             // Must be set before PHASE_FLYINGTOENTRY
	YsVec3 entryTurnCenterCandidate[2]; // Must be set in the CalculateTrafficPattern.  entryTurnCenter may change 
	                                    // in PHASE_TAKING_DISTANCE when it changes to PHASE_FLYINGTOENTRY
	                                    // because closer entryTurnCenter may be different from the time when
	                                    // the traffic pattern was calculated.
	YsVec3 entryTargetPoint;            // Updated every iteration in PHASE_FLYINGTOENTRY
	double downwindWidth;

	// Mode 1:  Hold in pattern until the airspeed reaches 110% of the landing speed

	// Mode 2:  I cannot enter the pattern because I'm too close to the pattern.  Go around.

	// Mode 3:  Turning to downwind leg.

	// Mode 4:  Downwind leg.

	// Mode 5:  Begin descend

	// Mode 6:  Turn to base
	YsVec3 baseTurnStart,baseTurnEnd;
	double calibratedTurnRadius;

	// Mode 7:  Base leg
	double distToGo,timeToGo,altToGo,yHold;
	double mode7IdealVY;

	// Mode 8:  Turn to final
	YsVec3 aimingPoint,aimingDir;

	// Mode 9:
	YSBOOL needFlare,interceptGlideSlope;
	double tdPitch;  // Record pitch angle when the airplane touched down.
	double prevErrX; // Lateral error
	YSBOOL flare;

	// Mode 10:  Hey, clear the runway!
	FsTaxiingAutopilot taxi;
	double nextTaxiwayScanTime;

protected:
	FsLandingAutopilot();
public:
	static FsLandingAutopilot *Create(void);
	void SetAirplaneInfo(const FsAirplane &air,const double &bankLimitOverride);
	YSRESULT GetApproachPath(double &spd,YsVec3 pos[2],YsAtt3 att[2],YsArray <YsVec3,16> path[2],FsGround *ils);
	void SetIls(const FsAirplane &air,const FsSimulation *sim,const FsGround *ils);
	void SetVfr(const FsAirplane &air,const FsSimulation *sim,const YsVec3 &o,const YsVec3 &v);
	void CalculateTrafficPattern(const FsAirplane &air,const YsVec3 tdPos,const YsAtt3 rwAtt); // tdPos,rwAtt may be this->*, don't make these a reference.
	YSBOOL CanMakeStraightIn(const FsAirplane &air);
	void UpdateCarrierTouchDownPosition(const double dt);
	YSBOOL GoAroundIfCarrierTurned(const FsAirplane &air);

	FSTRAFFICPATTERNLEG GetCurrentLeg(void) const;

	YSRESULT ChooseNamedVfrLanding(YsVec3 &tdPos,YsVec3 &rwDir,const char vfrName[],const FsSimulation *sim) const;
	YSRESULT AutoChooseVfrLanding(YsVec3 &tdPos,YsVec3 &rwDir,const double &requiredRunwayLength,const FsAirplane &air,const FsSimulation *sim) const;
	const FsGround *ChooseNamedILS(const FsAirplane &air,const FsSimulation *sim,const char ilsName[]) const;
	static const FsGround *AutoChooseILS(const double &requiredRunwayLength,const FsAirplane &air,const FsSimulation *sim);
	YSBOOL IsFinalPhaseOfLanding(void) const;

	void GetVelocityCorrection(YsVec3 &vec,FsSimulation *sim) const;
	double GetHeadingCorrection(FsAirplane &air,FsSimulation *sim,const double &courseHeading) const;

	virtual unsigned OverridedControl(void);
	virtual YSBOOL DoesRespondToRadioCall(void) const;
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

private:
	const double RequiredDistanceBeforeTurningToEntry(void) const;
	const YsVec3 GetRunwayHeadingVector(void) const;
};

////////////////////////////////////////////////////////////

class FsTakeOffAutopilot : public FsAutopilot
{
public:
	enum TAKEOFFSTATE
	{
		AP_TAKEOFF_STATE_ONTHEGROUND,
		AP_TAKEOFF_STATE_INTHEAIR,
		AP_TAKEOFF_STATE_DONE
	};

	TAKEOFFSTATE state;  // 0:On the ground  1:In the air  999:Take-off Procedure is done.
	double desigAlt;

	double desiredHeading,desiredBank;

private:
	YSBOOL captureInitialStateAsRunwayCenterline;
	YSBOOL useRunwayCenterLine;
	YsVec3 centerLineO,centerLineV;
	double runwayHeading;

public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_TAKEOFF;}

protected:
	FsTakeOffAutopilot();
public:
	static FsTakeOffAutopilot *Create(void);
	void Initialize(void);
	void UseRunwayCenterLine(const YsVec3 &o,const YsVec3 &v);
	void DontUseRunwayCenterLine(void);
	static YSRESULT FindRunwayCenterLine(YsVec3 &runwayCenter,YsVec3 &runwayDir,const FsAirplane &air,const FsSimulation *sim);

	virtual YSBOOL IsTakingOff(void) const;
	virtual unsigned OverridedControl(void);
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSBOOL MissionAccomplished(FsAirplane &air,FsSimulation *sim) const;

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);
};

////////////////////////////////////////////////////////////

class FsHoveringAutopilot : public FsAutopilot
{
public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_HOVER;}

protected:
	FsHoveringAutopilot();
	~FsHoveringAutopilot();
public:
	static FsHoveringAutopilot *Create(void);
	virtual unsigned OverridedControl(void);
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSBOOL MissionAccomplished(FsAirplane &air,FsSimulation *sim) const;

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	int state;
	YsVec3 desigPos;
};

////////////////////////////////////////////////////////////

class FsVerticalLandingAutopilot : public FsAutopilot
{
public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_VLANDING;}

protected:
	FsVerticalLandingAutopilot();
	~FsVerticalLandingAutopilot();
public:
	static FsVerticalLandingAutopilot *Create(void);
	YSRESULT AutoChooseILS(FsAirplane &air,FsSimulation *sim,YSBOOL includeStatic,YSBOOL includeCarrier);
	virtual unsigned OverridedControl(void);
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSBOOL MissionAccomplished(FsAirplane &air,FsSimulation *sim) const;
	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	YSBOOL includeStatic,includeCarrier;

	int state;  // 0:Hovering Toward the Landing Site  1:Descend  3:Landed  999:No landing site
	FsGround *site;
	YsAtt3 rwAtt;
	YsVec3 tdPos;
	YsVec3 prevSitePos;  // To numerically compute speed
	YsVec3 hold;  // Used when state==999
	YsVec3 siteVelocity;  // Carrier may be moving
};


class FsAirRouteAutopilot : public FsAutopilot
{
public:
	enum AIRROUTE_STATE
	{
		STATE_INITIAL, // <- Assess the situation and appropriately initialize
		STATE_ERROR,
		STATE_STATIONARY,
		STATE_CALLING_FUELTRUCK,
		STATE_WAIT_FOR_FUELTRUCK,
		STATE_REFUELING,
		STATE_DISMISS_FUELTRUCK,
		STATE_WAIT_FOR_FUELTRUCK_GONE,
		STATE_TAXI_FOR_TAKEOFF,
		STATE_TAKEOFF,
		STATE_ENROUTE,
		STATE_HOLDING,
		STATE_APPROACH,
		STATE_TAXI_ON_CARRIER,
		STATE_WAITING_UNTIL_SETTLE

	};

private:
	AIRROUTE_STATE state;
	double sameStateTimer;

	int airRouteIdx;
	YsVec3 legStart;
	YsString airRouteTag;
	const class YsScenery *sceneryCache;
	const class YsSceneryAirRoute *airRouteCache;
	const class YsSceneryRectRegion *airportCache;

	int occupyingSegIdx;
	YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE occupyingSegType;
	YsString occupyingSegLabel;

	// STATE_STATIONARY >>
	YSBOOL needToDismissFuelTruck;
	// STATE_STATIONARY <<

	// STATE_HOLDING >>
	YsVec3 holdingFix;
	// STATE_HOLDING <<

	// STATE_ENROUTE >>
	YSBOOL reachedFix;
	// STATE_ENROUTE <<


	FsLandingAutopilot *landingAP;
	FsGotoPosition *cruiseAP;
	FsTaxiingAutopilot *taxiingAP;
	FsTakeOffAutopilot *takeoffAP;


protected:
	FsAirRouteAutopilot();
	~FsAirRouteAutopilot();
public:
	static FsAirRouteAutopilot *Create(void);
	void Initialize(void);
	void ClearSubAutopilot(void);
	void SetAirRouteTag(const char tag[]);
	void SetAirRouteIndex(int idx);
	void SetLegStart(const YsVec3 &legStart);
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_AIRWAY;}

	YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE CurrentSegmentType(void) const;
	const char *CurrentSegmentLabel(void) const;

	YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE OccupyingSegmentType(void) const;
	const char *OccupyingSegmentLabel(void) const;

	YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE NextSegmentType(void) const;
	const char *NextSegmentLabel(void) const;

	template <const int N>
	inline YSBOOL IsApproachingRunway(FSTRAFFICPATTERNLEG &leg,YsArray <const class YsSceneryRectRegion *,N> &runwayRect,YsVec3 &tdPos) const;

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	virtual YSBOOL IsTakingOff(void) const;
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSBOOL Accomplished(void);

	const YsVec3 GetNextDestination(const FsSimulation *sim) const;
	const YsVec3 GetSegmentDestination(const FsSimulation *sim,YSSIZE_T segIdx) const;

private:
	YSRESULT FetchAirRoute(const FsSimulation *sim,const char tag[]);
	YSRESULT InitializeOnTheGround(FsAirplane &air,FsSimulation *simt);
	YSRESULT InitializeInTheAir(FsAirplane &air,FsSimulation *sim);
	void ResetTaxiingAutopilot(void);
	void ResetLandingAutopilot(void);
	void ResetTakeoffAutopilot(void);
	void ResetCruiseAutopilot(void);
	YSRESULT SetUpLeg(const FsAirplane &air,const FsSimulation *sim,int destinationIdx);
	YSRESULT SetUpTaxiForTakeOff(const FsAirplane &air,const FsSimulation *sim);
	YSRESULT SetUpForTakeOff(const FsAirplane &air,const FsSimulation *sim,const YsVec3 &rwO,const YsVec3 &rwV);
	YSRESULT SetUpForApproach(const FsAirplane &air,const FsSimulation *sim,int destinationIdx);
	YSRESULT SetUpDepartureFromAircraftCarrier(const FsAirplane &air,const FsSimulation *sim);
	void IncrementRouteIndexForDeparture(const FsAirplane &air,const FsSimulation *sim);
	void ReleaseOccupiedAirportOrFix(const FsAirplane &air,FsSimulation *sim);
	void OccupyAirportOrFix(const FsAirplane &air,FsSimulation *sim,YSSIZE_T segIdx);
};

#include "fsexistence.h"  // FsExistence needs FsAirRouteAutopilot definition.  Therefore, don't re-include it before FsAirRouteAutopilot definition.

template <const YSSIZE_T N>
inline void FsTaxiingAutopilot::GetNextSamplePointArrayFor160m(YsArray <YsVec3,N> &samplePointArray,const FsAirplane &air,const YsMatrix4x4 &pathTfm) const
{
	samplePointArray.Clear();

	YsVec3 curPos=air.GetPosition();
	YSSIZE_T curIdx=taxiPathIdx;

	double distToGo=160.0+air.GetApproximatedCollideRadius();
	const double stepDist=40.0;

	double stepDistToGo=stepDist;
	while(0.0<distToGo && curIdx<taxiPath.GetN())
	{
		const YsVec3 curPosToNextPathPnt=pathTfm*taxiPath[curIdx]-curPos;
		const double curPosToNextPathPntDist=curPosToNextPathPnt.GetLength();
		if(curPosToNextPathPntDist>stepDistToGo)
		{
			const YsVec3 toVec=YsUnitVector(curPosToNextPathPnt);
			curPos+=toVec*stepDistToGo;
			distToGo-=stepDistToGo;
			stepDistToGo=stepDist;
			samplePointArray.Append(curPos);
		}
		else // if(stepDistToGo>curPosToNextPathPntDist)
		{
			stepDistToGo-=curPosToNextPathPntDist;
			distToGo-=curPosToNextPathPntDist;
			curPos=pathTfm*taxiPath[curIdx];
			++curIdx;
		}
	}
}

inline void FsTaxiingAutopilot::GetNextSamplePointAndDirection(YsVec3 &pos,YsVec3 &dir,const FsAirplane &air,const double distAhead,const YsMatrix4x4 &pathTfm) const
{
	YsVec3 curPos=air.GetPosition();
	YSSIZE_T curIdx=taxiPathIdx;
	dir=YsOrigin();

	double distToGo=distAhead;
	while(0.0<distToGo && curIdx<taxiPath.GetN())
	{
		const YsVec3 curPosToNextPathPnt=pathTfm*taxiPath[curIdx]-curPos;
		const double curPosToNextPathPntDist=curPosToNextPathPnt.GetLength();
		if(curPosToNextPathPntDist>distToGo)
		{
			dir=YsUnitVector(curPosToNextPathPnt);
			pos=curPos+dir*distToGo;
			return;
		}
		else // if(stepDistToGo>curPosToNextPathPntDist)
		{
			distToGo-=curPosToNextPathPntDist;
			curPos=pathTfm*taxiPath[curIdx];
			dir=curPosToNextPathPnt;
			++curIdx;
		}
	}

	pos=curPos;
	dir.Normalize();
}

template <const int N>
inline YSBOOL FsAirRouteAutopilot::IsApproachingRunway(FSTRAFFICPATTERNLEG &leg,YsArray <const class YsSceneryRectRegion *,N> &runwayRect,YsVec3 &tdPos) const
{
	if(STATE_APPROACH==state && NULL!=landingAP && 0<landingAP->runwayRectCache.GetN())
	{
		runwayRect=landingAP->runwayRectCache;
		leg=landingAP->GetCurrentLeg();
		tdPos=landingAP->tdPos;
		return YSTRUE;
	}
	leg=FSLEG_NOT_IN_PATTERN;
	return YSFALSE;
}

/* } */
#endif
