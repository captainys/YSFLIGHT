#ifndef FSREFUELANDTAKEOFF_IS_INCLUDED
#define FSREFUELANDTAKEOFF_IS_INCLUDED
/* { */

#include "fsautopilot.h"
#include "fsmissionautopilot.h"



class FsRefuelAndTakeOffAutopilot : public FsMissionAutopilot
{
private:
	STATE prevState;
public:
	virtual FSAUTOPILOTTYPE Type(void) const {return FSAUTOPILOT_REFUEL_AND_TAKEOFF;}
protected:
	FsRefuelAndTakeOffAutopilot();
public:
	static FsRefuelAndTakeOffAutopilot *Create(void);
	virtual void SetUpMission(FsAirplane &air,FsSimulation *sim);
	virtual void InitializeInTheAir(FsAirplane &air,FsSimulation *sim);
	virtual YSRESULT SaveIntention(FILE *fp,const FsSimulation *sim);
	YSRESULT ReadIntention(YsTextInputStream &inStream,const YsString &firstLine);
	virtual YSBOOL StartMissionCondition(FsAirplane &air,FsSimulation *sim);
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSBOOL Accomplished(void);
};



/* } */
#endif
