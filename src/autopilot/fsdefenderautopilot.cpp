#include <ysclass.h>
#include <ysunitconv.h>
#include "fs.h"
#include "fsautodrive.h"
#include "fsdefenderautopilot.h"



FsDefenderAutopilot::FsDefenderAutopilot()
{
	defendBase=YSTRUE;
	defendPosition=YSTRUE;
	subState=SUBSTATE_NONE;
	engageRadius=YsUnitConv::SMtoM(20.0);
	adizRadius=YsUnitConv::SMtoM(30.0);
	startDogFightDist=10000.0;
	dogFightGLimit=5.0;
	dogFightTimeLimit=600.0;

	dogFightTimer=0.0;
	resetTargetTimer=0.0;

	df=NULL;
	ResetDogfightAutopilot();

	airTarget.Initialize();
}

/* virtual */ FsDefenderAutopilot::~FsDefenderAutopilot()
{
	if(NULL!=df)
	{
		FsAutopilot::Delete(df);
		df=NULL;
	}
}

/* static */ FsDefenderAutopilot *FsDefenderAutopilot::Create(void)
{
	return new FsDefenderAutopilot;
}

/* static */ const char *FsDefenderAutopilot::SubStateToString(FsDefenderAutopilot::SUBSTATE subState)
{
	switch(subState)
	{
	case SUBSTATE_PATROL:
		return "SUBSTATE_PATROL";
	case SUBSTATE_CLOSE_IN:
		return "SUBSTATE_CLOSE_IN";
	case SUBSTATE_INTERCEPT:
		return "SUBSTATE_INTERCEPT";
	case SUBSTATE_ENGAGE:
		return "SUBSTATE_ENGAGE";
	case SUBSTATE_RETURN_TO_BASE:
		return "SUBSTATE_RETURN_TO_BASE";
	default:
		break;
	}
	return "SUBSTATE_NONE";
}

/* static */ FsDefenderAutopilot::SUBSTATE FsDefenderAutopilot::StringToSubState(const char str[])
{
	if(0==strcmp(str,"SUBSTATE_PATROL"))
	{
		return SUBSTATE_PATROL;
	}
	if(0==strcmp(str,"SUBSTATE_CLOSE_IN"))
	{
		return SUBSTATE_CLOSE_IN;
	}
	if(0==strcmp(str,"SUBSTATE_INTERCEPT"))
	{
		return SUBSTATE_INTERCEPT;
	}
	if(0==strcmp(str,"SUBSTATE_ENGAGE"))
	{
		return SUBSTATE_ENGAGE;
	}
	if(0==strcmp(str,"SUBSTATE_RETURN_TO_BASE"))
	{
		return SUBSTATE_RETURN_TO_BASE;
	}
	return SUBSTATE_NONE;
}

/* virtual */ FSAUTOPILOTTYPE FsDefenderAutopilot::Type(void) const
{
	return FSAUTOPILOT_DEFENDER;
}

/* virtual */ YSRESULT FsDefenderAutopilot::SaveIntention(FILE *fp,const FsSimulation *sim)
{
	fprintf(fp,"DEFENDER\n");
	fprintf(fp,"SUBSTATE %s\n",SubStateToString(subState));
	fprintf(fp,"ENGRADIUS %.1lfm\n",engageRadius);
	fprintf(fp,"ADIZRADIUS %.1lfm\n",adizRadius);
	fprintf(fp,"DFGLIMIT %.1lfG\n",dogFightGLimit);
	fprintf(fp,"DFTIMELIMIT %.1lfsec\n",dogFightTimeLimit);
	fprintf(fp,"DFTIMER %.1lfsec\n",dogFightTimer);
	fprintf(fp,"STARTDFDIST %.1lfm\n",startDogFightDist);
	for(auto &pos : defendTargetPos)
	{
		fprintf(fp,"DEFTARGET %.1lfm %.1lfm %.1lfm\n",pos.x(),pos.y(),pos.z());
	}
	fprintf(fp,"DEFENDBASE %s\n",YsBoolToStr(defendBase));
	fprintf(fp,"DEFENDPOSITION %s\n",YsBoolToStr(defendPosition));
	FsMissionAutopilot::SaveIntention(fp,sim);
	return YSOK;
}

YSRESULT FsDefenderAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &)
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
			else if(0==strcmp("SUBSTATE",args[0]))
			{
				if(2<=args.GetN())
				{
					subState=StringToSubState(args[1]);
				}
			}
			else if(0==strcmp("ENGRADIUS",args[0]))
			{
				if(2<=args.GetN())
				{
					FsGetLength(engageRadius,args[1]);
				}
			}
			else if(0==strcmp("ADIZRADIUS",args[0]))
			{
				if(2<=args.GetN())
				{
					FsGetLength(adizRadius,args[1]);
				}
			}
			else if(0==strcmp("STARTDFDIST",args[0]))
			{
				if(2<=args.GetN())
				{
					FsGetLength(startDogFightDist,args[1]);
				}
			}
			else if(0==strcmp("DFGLIMIT",args[0]))
			{
				if(2<=args.GetN())
				{
					dogFightGLimit=atof(args[1]);
				}
			}
			else if(0==strcmp("DFTIMELIMIT",args[0]))
			{
				if(2<=args.GetN())
				{
					dogFightTimeLimit=atof(args[1]);
				}
			}
			else if(0==strcmp("DFTIMER",args[0]))
			{
				if(2<=args.GetN())
				{
					dogFightTimer=atof(args[1]);
				}
			}
			else if(0==strcmp("DEFTARGET",args[0]))
			{
				if(4<=args.GetN())
				{
					double x,y,z;
					FsGetLength(x,args[1]);
					FsGetLength(y,args[2]);
					FsGetLength(z,args[3]);
					defendTargetPos.Append(YsVec3(x,y,z));
				}
			}
			else if(0==strcmp("DEFENDBASE",args[0]))
			{
				if(2<=args.GetN())
				{
					defendBase=YsStrToBool(args[1]);
				}
			}
			else if(0==strcmp("DEFENDPOSITION",args[0]))
			{
				if(2<=args.GetN())
				{
					defendPosition=YsStrToBool(args[1]);
				}
			}
			else if(0==strcmp("ENDINTEN",args[0]))
			{
				return YSOK;
			}
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

void FsDefenderAutopilot::ResetDogfightAutopilot(void)
{
	if(NULL!=df)
	{
		FsAutopilot::Delete(df);
	}
	df=FsDogfight::Create();
	df->gLimit=dogFightGLimit;
}

/* virtual */ void FsDefenderAutopilot::SetUpMission(FsAirplane &air,FsSimulation *sim)
{
	ResetDogfightAutopilot();
	FsMissionAutopilot::ResetCruiseAutopilot();
	dogFightTimer=dogFightTimeLimit;

	ResetTarget(air,sim);

	subState=SUBSTATE_CLOSE_IN;

	checkEnemyTimer=3.0;
}

const double FsDefenderAutopilot::GetMinimumEnemyDistance(const FsAirplane &air,FsSimulation *sim)
{
	double minDist=YsInfinity;
	for(auto enemyAir=(sim->FindNextAirplane(NULL)); NULL!=enemyAir; enemyAir=sim->FindNextAirplane(enemyAir))
	{
		if(enemyAir->iff!=air.iff && YSTRUE==df->CanBeTarget(&air,enemyAir))
		{
			if(YSTRUE==defendBase)
			{
				YsVec3 basePos=GetAirportPosition();
				minDist=YsSmaller(minDist,(basePos-enemyAir->GetPosition()).GetLength());
			}
			if(YSTRUE==defendPosition)
			{
				for(auto &pos : defendTargetPos)
				{
					minDist=YsSmaller(minDist,(pos-enemyAir->GetPosition()).GetLength());
				}
			}
		}
	}
	return minDist;
}

FsAirplane *FsDefenderAutopilot::GetNearestEnemyToDefendTarget(const FsAirplane &air,FsSimulation *sim)
{
	double minDist=YsInfinity;
	FsAirplane *nearestAir=NULL;
	for(auto enemyAir=(sim->FindNextAirplane(NULL)); NULL!=enemyAir; enemyAir=sim->FindNextAirplane(enemyAir))
	{
		if(enemyAir->iff!=air.iff && YSTRUE==df->CanBeTarget(&air,enemyAir))
		{
			double dist=YsInfinity;
			if(YSTRUE==defendBase)
			{
				YsVec3 basePos=GetAirportPosition();
				dist=(basePos-enemyAir->GetPosition()).GetLength();
			}
			if(YSTRUE==defendPosition)
			{
				for(auto &pos : defendTargetPos)
				{
					dist=YsSmaller(dist,(pos-enemyAir->GetPosition()).GetLength());
				}
			}
			if(dist<minDist)
			{
				minDist=dist;
				nearestAir=enemyAir;
			}
		}
	}
	return nearestAir;
}

void FsDefenderAutopilot::ResetTarget(FsAirplane &air,FsSimulation *sim)
{
	FsAirplane *trg=GetNearestEnemyToDefendTarget(air,sim);
	if(NULL!=trg)
	{
		airTarget.objKeyCache=FsExistence::GetSearchKey(trg);
		airTarget.yfsIdx=trg->ysfId;
		airTarget.yfsLabel=trg->name;
	}
	resetTargetTimer=5.0;
}

const double FsDefenderAutopilot::TargetDist(const FsAirplane *air,FsSimulation *) const
{
	double maxDist=YsInfinity;
	if(YSTRUE==defendBase)
	{
		YsVec3 basePos=GetAirportPosition();
		maxDist=YsSmaller(maxDist,(basePos-air->GetPosition()).GetLength());
	}
	if(YSTRUE==defendPosition)
	{
		for(auto &pos : defendTargetPos)
		{
			maxDist=YsSmaller(maxDist,(pos-air->GetPosition()).GetLength());
		}
	}
	return maxDist;
}

/* virtual */ YSBOOL FsDefenderAutopilot::StartMissionCondition(FsAirplane &air,FsSimulation *sim)
{
	if(engageRadius>GetMinimumEnemyDistance(air,sim))
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

/* virtual */ YSRESULT FsDefenderAutopilot::MakePriorityDecision(FsAirplane &air)
{
	if(state==STATE_IN_MISSION)
	{
		return YSOK;
	}
	else
	{
		return FsMissionAutopilot::MakePriorityDecision(air);
	}
}

/* virtual */ YSRESULT FsDefenderAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(STATE_RTB!=state)
	{
		printf("%s %.1lf %.1lf/%.1lf/%.1lf\n",StateToString(state),sameStateTimer,engageRadius,adizRadius,GetMinimumEnemyDistance(air,sim));
	}
	else
	{
		YsVec3 dest=cruiseAP->GetNextWayPoint();
		printf("%s %s %s\n",StateToString(state),air.GetHomeBaseName().Txt(),dest.Txt());
	}

	if(state==STATE_IN_MISSION)
	{
		SameStateGuard sameStateGuard(this,dt);

		checkEnemyTimer-=dt;
		if(0.0>=checkEnemyTimer)
		{
			if(GetMinimumEnemyDistance(air,sim)>adizRadius)
			{
				SetUpRTB(air,sim);
				return YSOK;  // 2014/10/19 Must return here.  Otherwise, SUBSTATE_CLOSE_IN may override the destination.
			}
			checkEnemyTimer=3.0;
		}

		switch(subState)
		{
		default:
			SetUpRTB(air,sim);
			break;
		case SUBSTATE_CLOSE_IN:
			{
				resetTargetTimer-=dt;
				if(0.0>resetTargetTimer)
				{
					ResetTarget(air,sim);
				}

				auto trg=airTarget.GetAircraft(sim);
				if(NULL!=trg)
				{
					YsVec3 trgPos=trg->GetPosition();
					trgPos.SetY(holdingAlt);
					cruiseAP->SetSingleDestination(trgPos);

					YsVec3 airPos=air.GetPosition();
					const double dsq=(airPos-trgPos).GetSquareLengthXZ();
					if(dsq<YsSqr(startDogFightDist) && YSTRUE==df->CanBeTarget(&air,trg))
					{
						dogFightTimer=dogFightTimeLimit;
						ResetDogfightAutopilot();
						df->SetTarget(trg);
						subState=SUBSTATE_ENGAGE;
					}
				}
				else
				{
					SetUpRTB(air,sim);
				}
			}
			cruiseAP->MakeDecision(air,sim,dt);
			break;
		case SUBSTATE_ENGAGE:
			{
				auto trg=airTarget.GetAircraft(sim);
				if(NULL!=trg)
				{
					if(YSTRUE!=trg->Prop().IsActive() || TargetDist(trg,sim)>adizRadius)
					{
						trg=GetNearestEnemyToDefendTarget(air,sim);
						if(NULL!=trg && YSTRUE==df->CanBeTarget(&air,trg))
						{
							airTarget.objKeyCache=FsExistence::GetSearchKey(trg);
							airTarget.yfsIdx=trg->ysfId;
							airTarget.yfsLabel=trg->name;
							df->SetTarget(trg);
						}
					}
				}

				df->MakeDecision(air,sim,dt);

				dogFightTimer-=dt;
				if(0.0>=dogFightTimer)
				{
					SetUpRTB(air,sim);
				}
			}
			break;
		}

		return YSOK;
	}
	else
	{
		return FsMissionAutopilot::MakeDecision(air,sim,dt);
	}
}

/* virtual */ YSRESULT FsDefenderAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(state==STATE_IN_MISSION)
	{
		switch(subState)
		{
		case SUBSTATE_CLOSE_IN:
			if(sameStateTimer<180.0)
			{
				cruiseAP->SetUseAfterburner(YSTRUE);
			}
			else
			{
				cruiseAP->SetUseAfterburner(YSFALSE);
			}
			cruiseAP->SetThrottle(1.0);
			cruiseAP->ApplyControl(air,sim,dt);
			break;
		case SUBSTATE_ENGAGE:
			df->ApplyControl(air,sim,dt);
			break;
		default:
			break;
		}
		return YSOK;
	}
	else
	{
		return FsMissionAutopilot::ApplyControl(air,sim,dt);
	}
}

/* virtual */ YSBOOL FsDefenderAutopilot::Accomplished(void)
{
	// Never done.  Defend the base indefinitely.
	return YSFALSE;
}
