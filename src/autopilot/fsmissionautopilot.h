#ifndef FSMISSIONAUTOPILOT_IS_INCLUDED
#define FSMISSIONAUTOPILOT_IS_INCLUDED
/* { */

#include "fsautopilot.h"

class FsMissionAutopilot : public FsAutopilot
{
public:
	enum READINESS_TYPE
	{
		READINESS_24HOUR,
		READINESS_ALERT
	};

	enum EMERGENCY_MANEUVER
	{
		EMM_NONE,
		EMM_EVADEMISSILE
	};

	enum STATE
	{
		STATE_INITIAL,
		//     -> WAITING_FOR_SETTLING, TAXI_ON_CARRIER, or TAXI_FOR_TAKEOFF
		STATE_WAITING_FOR_SETTLING,
		//     -> TAXI_ON_CARRIER or TAXI_FOR_TAKEOFF
		STATE_WAIT_FOR_FUELTRUCK,
		//     -> REFUELING
		STATE_REFUELING,
		//     -> DISMISS_FUELTRUCK
		STATE_DISMISS_FUELTRUCK,
		//     -> TAXI_ON_CARRIER or TAXI_FOR_TAKEOFF
		STATE_TAXI_ON_CARRIER,
		//     -> READY_TO_TAKEOFF
		STATE_TAXI_FOR_TAKEOFF,
		//     -> TAKEOFF
		STATE_TAKEOFF,
		//     -> TAKEOFF_CLIMB
		STATE_HOLDING,
		//     -> IN_MISSION or RTB
		STATE_IN_MISSION,
		//     -> STATE_RTB
		STATE_RTB,
		//     -> STATE_APPROACH
		STATE_APPROACH,
		//     -> STATE_STATIONARY

		// Staying still on the ground.
		STATE_STATIONARY,
		//     -> STATE_REFUELING or STATE_WAIT_FOR_FUELTRUCK

		// Refueling on carrier
		STATE_REFUELING_ON_CARRIER,

		// ERROR!
		STATE_ERROR
	};

	class SameStateGuard;
	friend class SameStateGuard;

protected:
	// Must be defined in the data file >>
	READINESS_TYPE readinessType;
	YsArray <FsSimInfo::AirRef> teamAir;
	FsSimInfo::AirBase base;
	double holdingAlt;
	double holdingTime;
	double takeOffClimbAlt;
	double comeBackAlt;
	double beginApproachDist;
	double rtbFuelThr;
	YSBOOL rtbRunOutSAAM;
	YSBOOL rtbRunOutAAM;
	YSBOOL rtbRunOutAGM;
	YSBOOL rtbRunOutGun;
	int nMustBeInTheAir;
	int nEngage;
	double evasiveManeuverGLimit;
	// Must be defined in the data file <<

	// State variables >>
protected:
	STATE state;
	double sameStateTimer;

	EMERGENCY_MANEUVER emm;
	double emergencyTimer;
	double chaffFlareTimer;

private:
	YSBOOL calledFuelTruck;
	YSBOOL dismissedFuelTruck;
	// State variables <<

	// Cache >>
	YSBOOL cached;
	const YsScenery *sceneryCache;
protected:
	const YsSceneryRectRegion *airportCache;
	const FsGround *carrierCache;
	YsVec3 tdPos,rwDir;
	// Cache <<

protected:
	FsLandingAutopilot *landingAP;
	FsGotoPosition *cruiseAP;
	FsTaxiingAutopilot *taxiingAP;
	FsTakeOffAutopilot *takeoffAP;

	FsMissionAutopilot();
	~FsMissionAutopilot();

	static const char *StateToString(STATE state);
	static STATE StringToState(const char str[]);

	static const char *ReadinessTypeToString(READINESS_TYPE readinessType);
	static READINESS_TYPE StringToReadinessType(const char str[]);

private:
	void ClearSubAutoPilot(void);
	void ReallocSubAutoPilot(void);
protected:
	void ResetTakeoffAutopilot(void);
	void ResetTaxiingAutopilot(void);
	void ResetLandingAutopilot(void);
	void ResetCruiseAutopilot(void);

protected:
	YsVec3 GetAirportPosition(void) const;

private:
	void SetUpForTakeOff(FsAirplane &air,FsSimulation *sim);
	YSRESULT SetUpTaxiForTakeOff(FsAirplane &air,FsSimulation *sim);
	YSRESULT SetUpTaxiingOnAircraftCarrier(FsAirplane &air,FsSimulation *sim);
protected:
	void SetUpHolding(FsAirplane &air,FsSimulation *sim);
	void SetUpRTB(FsAirplane &air,FsSimulation *sim);
private:
	void SetUpApproach(FsAirplane &air,FsSimulation *sim);

	virtual void InitializeInTheAir(FsAirplane &air,FsSimulation *sim);
	YSRESULT SetUpTaxiing(FsAirplane &air,FsSimulation *sim);

protected:
	void Encache(FsAirplane &air,FsSimulation *sim);
	void Decache(void);

private:
	int GetNumAirplaneInMissionOrTakingOff(FsAirplane &air,FsSimulation *sim);
	int GetNumAirplaneInHoldingOrTakingOff(FsAirplane &air,FsSimulation *sim);
	YSBOOL CheckReturnToBaseCondition(FsAirplane &air,FsSimulation *sim);

protected:
	virtual YSBOOL IsTakingOff(void) const;
	void CleanUp(void);
	void StayStill(FsAirplane &air);
	virtual void SetUpMission(FsAirplane &air,FsSimulation *sim)=0;
	virtual YSBOOL StartMissionCondition(FsAirplane &air,FsSimulation *sim)=0;
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntentionMissionPart(YsTextInputStream &inStream,YsArray <YsString,16> &args);

public:
	virtual YSBOOL DoesRespondToRadioCall(void) const; // True only during STATE_TAKEOFF, STATE_IN_MISSION, and STATE_HOLDING

	void SetHomeBaseName(const YsString &str);
	void SetHomeBaseType(FsSimInfo::BASE_TYPE baseType);

	void SetUpEvadingFromMissile(void);
	void ClearEmergencyManeuver(FsAirplane &air);
	void SetUpEvasiveManeuverIfMissileIsChasing(FsAirplane &air,const FsSimulation *sim);
};


////////////////////////////////////////////////////////////


class FsMissionAutopilot::SameStateGuard
{
private:
	FsMissionAutopilot *ap;
	FsMissionAutopilot::STATE prevState;
	double dt;
public:
	SameStateGuard(FsMissionAutopilot *ap,const double dt);
	~SameStateGuard();
};


/* } */
#endif
