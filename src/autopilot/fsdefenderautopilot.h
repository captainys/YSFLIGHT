#ifndef FSDEFENDERAUTOPIILOT_IS_INCLUDED
#define FSDEFENDERAUTOPIILOT_IS_INCLUDED
/* { */

#include "fsautopilot.h"
#include "fsmissionautopilot.h"

////////////////////////////////////////////////////////////

class FsDefenderAutopilot : public FsMissionAutopilot
{
public:
	enum SUBSTATE
	{
		SUBSTATE_NONE,
		SUBSTATE_PATROL,
		SUBSTATE_CLOSE_IN,
		SUBSTATE_INTERCEPT,
		SUBSTATE_ENGAGE,
		SUBSTATE_RETURN_TO_BASE
	};


private:
	SUBSTATE subState;
	double engageRadius; // Engage if the enemy comes within this radius.
	double adizRadius;   // Return to base if the enemy leaves this radius.
	double startDogFightDist; // Start FsDogfight autopilot at this distance.
	double dogFightGLimit;
	double dogFightTimeLimit;
	YsArray <YsVec3> defendTargetPos;

	double resetTargetTimer;

	double dogFightTimer;
	double checkEnemyTimer;

	YSBOOL defendBase;
	YSBOOL defendPosition;

	FsDogfight *df;
	FsSimInfo::AirRef airTarget;


private:
	FsDefenderAutopilot();
	virtual ~FsDefenderAutopilot();
public:
	static const char *SubStateToString(SUBSTATE subState);
	static SUBSTATE StringToSubState(const char str[]);

	static FsDefenderAutopilot *Create(void);
	virtual FSAUTOPILOTTYPE Type(void) const;

	void ResetDogfightAutopilot(void);

	virtual void SetUpMission(FsAirplane &air,FsSimulation *sim);

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);

	const double GetMinimumEnemyDistance(const FsAirplane &air,FsSimulation *sim);
	FsAirplane *GetNearestEnemyToDefendTarget(const FsAirplane &air,FsSimulation *sim);
	const double TargetDist(const FsAirplane *air,FsSimulation *sim) const;

	void ResetTarget(FsAirplane &air,FsSimulation *sim);

	virtual YSBOOL StartMissionCondition(FsAirplane &air,FsSimulation *sim);
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSBOOL Accomplished(void);
};

/* } */
#endif
