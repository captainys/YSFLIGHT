#include <ysclass.h>
#include <ysunitconv.h>
#include "fs.h"
#include "fsrefuelandtakeoff.h"

FsRefuelAndTakeOffAutopilot::FsRefuelAndTakeOffAutopilot()
{
	state=STATE_INITIAL;
}

/* static */ FsRefuelAndTakeOffAutopilot *FsRefuelAndTakeOffAutopilot::Create(void)
{
	return new FsRefuelAndTakeOffAutopilot;
}

/* virtual */ void FsRefuelAndTakeOffAutopilot::SetUpMission(FsAirplane &,FsSimulation *)
{
}

/* virtual */ void FsRefuelAndTakeOffAutopilot::InitializeInTheAir(FsAirplane &air,FsSimulation *sim)
{
	// Need to find an airport or aircraft carrier
	// If the aircraft has its own home base set, use it, and set cached=YSFALSE; and call EnCache();

	if(0<air.GetHomeBaseName().Strlen())
	{
		SetHomeBaseName(air.GetHomeBaseName());
		SetHomeBaseType(air.GetHomeBaseType());
		Decache();
		Encache(air,sim);
	}
	else
	{
		auto ils=FsLandingAutopilot::AutoChooseILS(1800.0,air,sim);
		if(NULL!=ils)
		{
			auto rgn=sim->FindAirportFromPosition(ils->GetPosition());
			if(NULL!=rgn)
			{
				SetHomeBaseName(rgn->GetTag());
				SetHomeBaseType(FsSimInfo::AIRPORT);
				Decache();
				Encache(air,sim);
			}
		}
	}

	if((FsSimInfo::AIRPORT==air.GetHomeBaseType() && NULL!=airportCache) ||
	   (FsSimInfo::CARRIER==air.GetHomeBaseType() && NULL!=carrierCache))
	{
		SetUpRTB(air,sim);
	}
	else
	{
		SetUpHolding(air,sim);
		YsString msg;
		msg.Printf("[%s] cannot find the airport to land.",(const char *)air.GetName());
		sim->AddTimedMessage(msg);
	}
}

/* virtual */ YSRESULT FsRefuelAndTakeOffAutopilot::SaveIntention(FILE *fp,const FsSimulation *sim)
{
	fprintf(fp,"REFUEL__\n");
	FsMissionAutopilot::SaveIntention(fp,sim);
	return YSOK;
}

YSRESULT FsRefuelAndTakeOffAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &)
{
	YsString str;
	while(NULL!=inStream.Gets(str))
	{
		YsArray <YsString,16> args;
		str.Arguments(args);
		if(0<args.GetN())
		{
			if(YSOK==FsMissionAutopilot::ReadIntentionMissionPart(inStream,args))
			{
				// Processed by the base class
			}
			else if(0==strcmp("ENDINTEN",args[0]))
			{
				return YSOK;
			}
		}
	}
	return YSERR; // Missing "ENDINTEN"
}
/* virtual */ YSBOOL FsRefuelAndTakeOffAutopilot::StartMissionCondition(FsAirplane &,FsSimulation *)
{
	return YSFALSE;
}
/* virtual */ YSRESULT FsRefuelAndTakeOffAutopilot::MakePriorityDecision(FsAirplane &air)
{
	return FsMissionAutopilot::MakePriorityDecision(air);
}
/* virtual */ YSRESULT FsRefuelAndTakeOffAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(STATE_IN_MISSION==state)
	{
		SetUpRTB(air,sim);
		return YSOK;
	}
	else
	{
		if(STATE_HOLDING!=prevState && STATE_HOLDING==state)
		{
			YsString msg;
			msg.Printf("[%s] is ready.",(const char *)air.GetName());
			sim->AddTimedMessage(msg);
		}
		prevState=state;
		return FsMissionAutopilot::MakeDecision(air,sim,dt);
	}
}
/* virtual */ YSRESULT FsRefuelAndTakeOffAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(STATE_IN_MISSION==state)
	{
		return YSOK;
	}
	else
	{
		return FsMissionAutopilot::ApplyControl(air,sim,dt);
	}
}
/* virtual */ YSBOOL FsRefuelAndTakeOffAutopilot::Accomplished(void)
{
	return YSFALSE;
}
