#ifndef FSAUTOPILOTEXPERIMENTAL_IS_INCLUDED
#define FSAUTOPILOTEXPERIMENTAL_IS_INCLUDED
/* { */

#include "fsexistence.h"

class FsSpeedOnlyAutopilot : public FsAutopilot
{
private:
	FsSpeedOnlyAutopilot(){};
	virtual ~FsSpeedOnlyAutopilot(){};

public:
	static FsSpeedOnlyAutopilot *Create(void);
	double spd;

	virtual FSAUTOPILOTTYPE Type(void) const;
	virtual unsigned OverridedControl(void);
	virtual YSRESULT MakePriorityDecision(FsAirplane &air);
	virtual YSRESULT MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt);
	virtual YSRESULT ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt);
};

/* } */
#endif
