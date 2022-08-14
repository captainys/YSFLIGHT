#ifndef FSAUTODRIVE_IS_INCLUDED
#define FSAUTODRIVE_IS_INCLUDED
/* { */

#include <ysclass.h>
#include "fsaibasic.h"


class FsAutoDrive : public FsAIObject
{
public:
	enum AUTODRIVETYPE
	{
		AUTODRIVE_TOOBJECT,
		AUTODRIVE_TOEXACTPOSITION
	};

protected:
	class ThreePointTurn
	{
	public:
		enum Phase
		{
			THREE_POINT_BACKWARD_FORWARD_PHASE1, // Backing up and turn
			THREE_POINT_BACKWARD_FORWARD_PHASE2, // Moving forward and turn
			THREE_POINT_DONE
		};
		Phase threePointPhase;
		YsVec3 initVec;
		double turnDir;  // +1.0 or -1.0

		void Initialize(const double turnDir,const YsVec3 &initVec);
		YSBOOL Accomplished(void) const;

		void Start(const class FsGround *gnd,const double turnDir);
		void Drive(FsGround *gnd);
	};

	class Align
	{
	public:
		enum Phase
		{
			ALIGN_PHASE_INIT
		};

		Phase alignPhase;

		double desiredSpeed;
		YsVec2 goalPos,goalVec;  // Align with this line
		YSBOOL alignReverse;
		YSBOOL alignAndGoToGoal;

		double lastLateraldeviation,lastAngleDeviation;

		void Start(const class FsGround *gnd,const double desiredSpeed,const YsVec3 &goalPos,const double goalHdg);
		void Drive(FsGround *gnd);
	};

private:
	static const char *const keyWordSource[];
	static YsKeyWordList keyWordList;

protected:
	double startTime,lastControlTime;

public:
	FsAutoDrive();
	virtual ~FsAutoDrive();

	void SetUp(class FsSimulation *sim,class FsGround *gnd);

	// Exceptions >>
	YSBOOL CheckCollisionPotentialAir(class FsSimulation *sim,class FsGround *gnd,int nExclude,const class FsExistence *const exclude[]) const;
	YSBOOL CheckUnreachable(class FsSimulation *sim,const class FsGround *gnd,const YsVec2 &goal) const;
	// Exceptions <<

	// Basic maneuvers >>
	void DriveToDestination(
	    double &steering,double &desiredSpeed,
	    class FsSimulation *sim,class FsGround *gnd,const double gndRad,const YsVec3 &goalPos,const double goalRad);
	// Basic maneuvers <<

	virtual AUTODRIVETYPE GetType(void) const=0;

	YSRESULT Save(class FsSimulation *sim,class FsGround *gnd,FILE *fp);
	virtual YSRESULT Save(class FsSimulation *sim,class FsGround *gnd,YsTextOutputStream &outStream)=0;
	virtual YSRESULT ReconnectObjKeyAfterLoading(class FsSimulation *sim,const YsHashTable <int> &ysfIdToObjKey)=0;

	virtual void Control(class FsSimulation *sim,class FsGround *gnd)=0;
	virtual YSBOOL ObjectiveAccomplished(class FsSimulation *sim,class FsGround *gnd) const=0;

	static FsAutoDrive *Load(class FsSimulation *sim,FILE *fp);
	static FsAutoDrive *Load(class FsSimulation *sim,YsTextInputStream &inStream);
};


class FsAutoDriveToObject : public FsAutoDrive
{
friend class FsAutoDrive;

private:
	int goalObjKey;
	YSBOOL dontStop;

public:
	FsHoldUntil holdUntil;

	FsAutoDriveToObject();
	virtual AUTODRIVETYPE GetType(void) const;
	void SetUp(class FsSimulation *sim,class FsGround *gnd,class FsExistence *goal);
	int GetGoalObjKey(void) const;
	void HoldUntilThisObjectLeaves(class FsSimulation *sim,const class FsExistence *thisObj);
	virtual YSRESULT Save(class FsSimulation *sim,class FsGround *gnd,YsTextOutputStream &outStream);
	virtual YSRESULT ReconnectObjKeyAfterLoading(class FsSimulation *sim,const YsHashTable <int> &ysfIdToObjKey);
	virtual void Control(class FsSimulation *sim,class FsGround *gnd);
	virtual YSBOOL ObjectiveAccomplished(class FsSimulation *sim,class FsGround *gnd) const;
};


class FsAutoDriveToExactPosition : public FsAutoDrive
{
friend class FsAutoDrive;

public:
	enum Phase
	{
		PHASE_INITIAL,
		PHASE_BACKUP,           // Taking distance from the nearby airplane.
		PHASE_APPROACH,         // Just drive straight until coming close to the goal.
		PHASE_THREE_POINT_TURN, // Backing up & Turn
		PHASE_ALIGN,
		PHASE_FINALGUIDANCE,
		PHASE_DONE
	};

private:
	// Pattern 1: Go behind the goal and come around
	// Pattern 2: Pass in front of the goal and back up

	// Properties to be saved >>
	Phase phase;
	YsVec3 goalPos;
	double goalHdg;
	// Properties to be saved <<

	// Variable >>
	ThreePointTurn threePointTurn;
	Align alignControl;
	YSBOOL waitingForStop;
	double turnDir;
	int temporarilyExcludedFromCollisionTestKey;
	YsVec2 prevGndDir;
	double totalUnreachableTurn;
	// Variable <<
public:
	FsHoldUntil holdUntil;

	FsAutoDriveToExactPosition();
	void SetUp(class FsSimulation *sim,class FsGround *gnd,const YsVec3 &pos,const double hdg);
	void StartWithThreePointTurn(class FsSimulation *sim,class FsGround *gnd);
	void ExcludeFromCollisionTest(const FsExistence *objPtr);
	virtual AUTODRIVETYPE GetType(void) const;
	virtual YSRESULT Save(class FsSimulation *sim,class FsGround *gnd,YsTextOutputStream &outStream);
	virtual YSRESULT ReconnectObjKeyAfterLoading(class FsSimulation *sim,const YsHashTable <int> &ysfIdToObjKey);
	virtual void Control(class FsSimulation *sim,class FsGround *gnd);
	virtual YSBOOL ObjectiveAccomplished(class FsSimulation *sim,class FsGround *gnd) const;
};


/* } */
#endif
