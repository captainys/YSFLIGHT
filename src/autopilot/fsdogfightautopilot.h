#ifndef FSDOGFIGHTAUTOPILOT_IS_INCLUDED
#define FSDOGFIGHTAUTOPILOT_IS_INCLUDED
/* { */

#include "fsautopilot.h"

class FsDogfight : public FsAutopilot
{
private:
	class FsGotoPosition *cruiseAP;

public:
	enum 
	{
		DFMODE_NOTARGET=-1,

		DFMODE_NORMAL=0,
		DFMODE_TARGET_ONBACK=1,
		DFMODE_TARGET_INFRONT=2,
		DFMODE_TARGET_ONBACK_BREAK=3,
		DFMODE_TARGET_INFRONT_FARAWAY=4,
		DFMODE_TARGET_FLYINGLOW=5,
		DFMODE_AVOIDING_HEADON_COLLISION=6,
		DFMODE_HIGH_G_YO_YO=7,
		DFMODE_CLIMB_UP=8,
		DFMODE_CRUISE_TO_COMBAT_AREA=9,

		DFMODE_STANDBY_GROUND=100,
		DFMODE_STANDBY_AIR=101,
		DFMODE_STANDBY_GROUND_TIMER=102,

		DFMODE_ACT_AS_DECOY_LEVELOFF=200,
		DFMODE_ACT_AS_DECOY_JINKING=201,
		DFMODE_ACT_AS_DECOY_BREAK=202,
		DFMODE_ACT_AS_DECOY_BARRELROLL_PITCHUP=203,
		DFMODE_ACT_AS_DECOY_BARRELROLL_ROLL=204,
		/* ACT_AS_DECOY reserves 200 to 299 */
		DFMODE_ACT_AS_DECOY_LASTRESERVED=299
	};

	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_DOGFIGHT;}

protected:
	YSBOOL closeInMaxSpeed;
	YSBOOL targetFlyingAircraftOnly;

	FsDogfight();
	virtual ~FsDogfight();
public:
	static FsDogfight *Create(void);

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	void SetCloseInMaxSpeed(YSBOOL sw);
	YSBOOL GetCloseInMaxSpeed(void) const;

	void SetTargetFlyingAircraftOnly(YSBOOL sw);
	YSBOOL GetTargetFlyingAircraftOnly(void) const;

	double farThreshold;
	double combatThreshold;

	int pendingTargetAirId,pendingWingmanAirId;
	YsString pendingTargetName,pendingWingmanName;

	double giveUpDist;
	double cruiseAlt;

private:
	YSHASHKEY targetAirplaneKey;

public:
	unsigned int defendThisAirplaneKey;
	unsigned int wingmanAirplaneKey;
	int mode,prevMode;
		// Negative:No more target
		// 0:Normal
		// 1:Target is on the Back!!
		// 2:Target is just in front!!
		// 3:
		// 4:Target is in front, but far away.  Let's recover from inverted position.
		// 5:Target is in low altitude.  Stay behind, but maintain altitude!
		// 6:Avoid head-on collision!
		// 7:Dead lock.  Something like high-g yo-yo
		// 8:Dead lock.  Climb, take distance, and try again.

		// 100:Do nothing on the ground until the target comes close
		// 101:Do nothing in the air until the target comes close
		// 102:Wait on the ground for the modeDuration

		// 200:Acting as decoy for the wingman (Leveling off)
		// 201:Acting as decoy for the wingman (Jinking)
		// 202:Acting as decoy: Turn (direction controlled by jinkRollDir)
		// 203:Acting as decoy: Barrel Roll (Level off -> Pitch 10 degree up)
		// 204:Acting as decoy: Barrel Roll (Roll)
		//  :
		// 299:Reserved for decoy mode


	double radar;
	YsVec3 rel1,rel2,rel3;
	double g1,g2,g3;
	double lastDamageValue;
	double gLimit,gLimitCorrection;
	double backSenseRange;
	double clock, nextClock, nextBreakClock;
	double fireClock,flareClock;
	double standBy;
	//  Used in mode 0:  Count how long mode 0 is lasting
	//  Used in mode 100:  as distance
	//  Used in mode 101:  as distance

	double nextTargetSearchTimer;
	// Used in
	//   DFMODE_CRUISE_TO_COMBAT_AREA and re-searches target when the timer reaches zero.

	double jinkDesigBank;
	double jinkNextBankChangeTime;
	double jinkRollDir;
	int jinkRollState;
	//  Used in mode 200-299


	double modeDuration;

	YSRESULT GetRelativePosition(YsVec3 &rel,const YsVec3 &org,FsAirplane &air,FsSimulation *sim);
	YSRESULT GetRelativeAttitude(YsAtt3 &att,FsAirplane &air,FsSimulation *sim);
	double GetRelativeBank(FsAirplane &air,FsSimulation *sim);

	YSRESULT SearchTarget(FsAirplane &air,FsSimulation *sim);

	YSBOOL CanBeTarget(const FsAirplane *air,const FsAirplane *trg) const;
	void SetTarget(FsAirplane *trg);
	FsAirplane *GetTarget(FsSimulation *sim);

	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	FsAirplane *GetWingmansTarget(FsAirplane &air,FsSimulation *sim);
	YSBOOL BeingChasedByWingmansTarget(FsAirplane &air,FsSimulation *sim);
	YSBOOL WingmanClosingIn(FsAirplane &air,FsSimulation *sim);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);

	YSBOOL TargetIsWithinCombatRange(const FsExistence &air,const FsExistence &target) const;
	const double GetCombatRange(void) const;
	void UpdateBreakClocks(double minDuration, double maxDuration);
};


/* } */
#endif
