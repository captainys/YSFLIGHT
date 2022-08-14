#include <ysclass.h>
#include <ysunitconv.h>

#include <fsairproperty.h>

#include "fs.h"
#include "fsautodrive.h"

#include "fsautopilotexperimental.h"

/* static */ FsSpeedOnlyAutopilot *FsSpeedOnlyAutopilot::Create(void)
{
	return new FsSpeedOnlyAutopilot;
}


/* virtual */ FSAUTOPILOTTYPE FsSpeedOnlyAutopilot::Type(void) const
{
	return FSAUTOPILOT_SPEEDONLY;
}

/* virtual */ unsigned FsSpeedOnlyAutopilot::OverridedControl(void)
{
	return ~FSAPPLYCONTROL_THROTTLE;
}

/* virtual */ YSRESULT FsSpeedOnlyAutopilot::MakePriorityDecision(FsAirplane &air)
{
	return YSOK;
}

/* virtual */ YSRESULT FsSpeedOnlyAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	spd=YsUnitConv::KTtoMPS(150.0);
	return YSOK;
}

/* virtual */ YSRESULT FsSpeedOnlyAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// IAS to TAS
	const double rho=FsGetAirDensity(air.GetPosition().y()+sim->GetBaseElevation());
	const double scale=sqrt(rho/1.225);
	const double tas=spd/scale;
	air.Prop().SpeedController(tas);
	return YSOK;
}

