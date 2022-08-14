#ifndef FSGROUNDATTACKAUTOPILOT_IS_INCLUDED
#define FSGROUNDATTACKAUTOPILOT_IS_INCLUDED
/* { */

#include "fsautopilot.h"

class FsGroundAttack : public FsAutopilot
{
private:
	class GndToAirThreat
	{
	public:
		YsVec3 pos;
		double range;
	};

public:
	enum STATE
	{
		STATE_GETTINGTHERE=0,
		STATE_TURNINBOUND=1,
		STATE_INBOUND_AGM_ROCKET=2,
		STATE_INBOUND_BOMB=3,
		STATE_TURNAWAY=4,
		STATE_EVASIVE_0=10,
		STATE_EVASIVE_1=11
	};
	static const char *StateToStr(STATE state);
	static STATE StrToState(const char str[]);

protected:
	STATE attackPhase;   // 0:Escaping away
	             // 1:Turning to the target
	             // 2:Preparing AGM-65 and Rocket
	             // 3:Bombing run
	             // 4:Turn away
	             // 10:Breaking right
	             // 11:Breaking left

public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_GNDATTACK;}

	FSWEAPONTYPE choice;

private:
	YSBOOL turnAwayAfterWeaponRelease;
	double turnAwayHeading;
	double turnAwayDistance;
	YsArray <GndToAirThreat,16> gndToAirThreat;

	// No save.  Calculated on the fly >>
	YsVec3 turnAwayFrom;    // Only in STATE_TURNAWAY
	YsVec3 turnAwayRelDir;  // Only in STATE_TURNAWAY
	int turnAwayTurnDir;    // Only in STATE_TURNAWAY
	// No save.  Calculated on the fly <<

public:
	// double flareTimer;
	// YSBOOL prevMissileChasing,missileChasing,flareBtn;
	AutoFlareDispenser flareDispenser;

	double agmReleaseDist,rocketReleaseDist,gunReleaseDist;
	double turnRadius,turnRadiusHeavy;
	double inboundSpeed;
	double flareInterval;
	YSBOOL breakOnMissile;
	YSBOOL takeEvasiveAction;
	YsString pendingGndTargetName;

	YsVec3 leadTargetPos;
	YsVec3 dif,difHorizontal;
	YsVec3 rel,relHorizontal;
	double prevDist;
	YsVec3 prevEstimate;
	double breakingTime;
	double bankForEvadingMissile;
	YSBOOL attackDone;
	double gControl;

	// Bomber parameters
	double bomberAlt;
private:
	double attackAlt;

public:
	class FsGround *target;
	unsigned int playerGroundKey;
	// If playerGroundKey is set, FsGroundAttack will choose the nearest primary target to the player,
	// but will not attack the player until all other primary targets are destroyed.

protected:
	FsGroundAttack();
	~FsGroundAttack();

public:
	int GetNumAGMReleasePerPass(const FsAirplane &air) const;

	const double GetAGMReleaseDistance(void) const;
	void SetAGMReleaseDistance(const double dist);

	void SetTurnAwayAfterWeaponRelease(YSBOOL turnAway);
	void SetTurnAwayAndFlyToAtLeastThisDistance(const double dist);
	void SetInboundSpeed(const double vel);
	void SetAttackerAltitude(const double y);
	void SetBomberAltitude(const double y);

public:
	static FsGroundAttack *Create(void);
	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);

private:
	YSBOOL OutOfThreatRange(FsAirplane &air,const double margin) const;
public:
	void SearchTarget(FsAirplane &air,FsSimulation *sim);
	YSBOOL BeingChasedByEnemy(FsAirplane &air,FsSimulation *sim);
	STATE Phase(void) const;
	void SetPhase(FsAirplane &air,STATE phase);
};


/* } */
#endif
