#ifndef FSFORMATIONAUTOPILOT_IS_INCLUDED
#define FSFORMATIONAUTOPILOT_IS_INCLUDED
/* { */

#include "fsautopilot.h"

enum
{
	FSFOMMODE_INPOSITION=0,
	FSFOMMODE_LOSTPOSITION=1,
	FSFOMMODE_SPEEDCONTROLONLY=2,
	FSFOMMODE_NEEDTAKEOFF=3,
	FSFOMMODE_TOOFAR=4,
	FSFOMMODE_TOOFAR_TOOHIGH=5,
	FSFOMMODE_RECOVER_FROM_LOWALT=6,
};

class FsFormation : public FsAutopilot
{
public:
	virtual FSAUTOPILOTTYPE Type(void) const{return FSAUTOPILOT_FORMATION;}

protected:
	FsFormation();
	virtual ~FsFormation();

private:
	AutoFlareDispenser flareDispenser;

public:
	static FsFormation *Create(void);

	static const char *StateToStr(int fomMode);
	static int StrToState(const char str[]);

	virtual unsigned OverridedControl(void);

	virtual YSRESULT MakePriorityDecision(FsAirplane &air);

	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);


	int position;  // <- This is not used by FsFormation, but can be used for keeping
	               //    track of what airplane is in which position.  (Like a free counter)
	YSBOOL formationTakeOff;


	const double tooFarThreshold,wayTooFarThreshold;


	// leaderId,shouldBe  to save/load
	int pendingLeaderId;
	FsAirplane *leader;
	YsVec3 shouldBe,spacing; // Formation position and temporary spacing in the leader airplane's coordinate system.

	YsVec3 goal;    // Formation posiition in the world coordinate system.  Calculated in MakeDecision
	double hDist;   // Horizontal distance to the goal from the airplane's current position.  Calculated in MakeDecision
	double yMin;    // If the airplane is too far from its goal, it must take high-enough altitude until it comes close.
	double radar;   // Angular deviation of the goal position from the airplane's nose direction.
	double bearing; // Relative bearing to the goal position.  Calculated in MakeDecision if the state is TOOFAR or TOOFAR_TOOHIGH

	int nTransition;
	YsVec3 transition[4];
	double transitionSpeed;  // <- Not used now. (And not saved.)
	YsArray <int,6> afterTransFormationAirplane;
	YsArray <YsVec3,6> afterTransFormationPos;

	YSBOOL inverted;
	YSBOOL autoSpacingForLanding;
	YSBOOL synchronizeTrigger;
	YSBOOL synchronizeLandingGear;

	int mode;
	double tInPosition;
	// 0:Almost In Position
	// 1:Seeking Leader Airplane
	// 2:Throttle Only Mode

	YSBOOL onceInTheAir;

	double prevPathAngle;  // <- must be initialzied when the mode is switched to zero.
	double prevEHdg;       // <- must be initialzied when the mode is switched to zero.
	double prevX;          // <- must be initialized when the mode is switched to zero.
	double prevEY;         // <- must be initialized when the mode is switched to zero.
	double prevDEY;        // <- must be initialized when the mode is switched to zero.
	double iEY;            // <- must be initialized when the mode is switched to zero.
	double prevPch;        // <- must be initialized when the mode is switched to zero.
	// double prevTheataK;    // <- must be initialized when the mode is switched to zero.
	// double iTheataK;       // <- must be initialized when the mode is switched to zero.

	double prevZ,iZ;   // Z in this airplane's coordinate
	double prevLZ,iLZ; // Z in the leader's coordinate

	class FsTakeOffAutopilot *takeoff;
	double takeoffFromAlt;

	class FsGotoPosition *goToPosition;


	void SetSpeedControlOnlyMode(void);
	const double GetDefaultGLimit(FsAirplane &air) const;
	const double GetMinimumAltitude(FsAirplane &air) const;

	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);

	void Mode0SpeedControl(FsAirplane &wingman,FsAirplane &leader,const double &dt,const double &Zoffset,YSBOOL lowSpeedLimitter);
	void Mode0LateralControl(FsAirplane &wingman,FsAirplane &leader,const double &dt);
	void Mode0VerticalControl(FsAirplane &wingman,FsAirplane &leader,const double &dt);

	void Mode1SpeedControl(FsAirplane &wingman,FsAirplane &leader,const double &dt,const double &Zoffset);

//	YSRESULT IntelligentFormationChange(YsVec3 &neoShouldBe,int type);

	YsVec3 GetGoalPosition(void) const;
	void CorrectAltitude(YsVec3 &pos,const double hDist);

	void SetShouldBe(const YsVec3 &shouldBe);
	const YsVec3 &GetShouldBe(void) const;
};



/* } */
#endif
