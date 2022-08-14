#include <ysclass.h>
#include <ysunitconv.h>
#include "fs.h"
#include "fsautodrive.h"
#include "fsmissionautopilot.h"


// Can be anti-ground, anti-air, or both

// Probably I can make:
//   FsMissionAutoPilot and derive FsDefenderAutopilot and FsGroundAttackAutoPilot.
//   FsGroundAttackAutoPilot can attack, then return, reload, and launch a new attack to the same base.


// Trigger may be interesting.  When the player plane comes close to the base, it launches the intercepter and also launch ground attacker for retaliation.


// .... I realized that carrier aircrafts always need to stay in the air.  Only land for re-fueling.
// So, it needs STATE_HOLDING.

// What's the condition for switching from HOLDING to IN_MISSION?

// 24-hour:  Always keep nMustBeInTheAir aircrafts in the air.
// TAKEOFF -> HOLDING -> IN_MISSION -> RTB -> TAXI_ON_CARRIER -> WAIT_FOR_FUELTRUCK -> REFUELING -> DISMISS_FUELTRUCK -> TAXI_FOR_TAKEOFF -> TAKEOFF
// Aircraft is considered to be in the mission if the state is IN_MISSION.
// Always keep nMustBeInTheAir aircrafts in the air.
// Threat inbound -> send nEngage aircrafts for the threat. ?
//   The aircraft needs to wait:
//     At the runway threshold if the nMustBeInTheAir or more aircrafts are holding.
//     Hold in the air if nEngage or more aircrafts are in mission.

// Alert:    Keep all the aircrafts on the ground unless a threat is in the vicinity.
// TAKEOFF ->                       IN_MISSION -> RTB -> TAXI_ON_CARRIER -> WAIT_FOR_FUELTRUCK -> REFUELING -> DISMISS_FUELTRUCK -> TAXI_FOR_TAKEOFF -> TAKEOFF
// Aircraft is considered to be in the mission if the state is TAKEOFF, HOLDING or IN_MISSION.
// Threat not inbound -> keep all aircrafts on the ground.
// Threat inbound -> Launch nMustBeInTheAir aircrafts for the threat.

// Need to control the timing of launch and engage.


// States:
//   INITIAL
//     -> WAITING_FOR_SETTLING, IN_MISSION, TAXI_ON_CARRIER, or TAXI_FOR_TAKEOFF
//   WAITING_FOR_SETTLING  : In case the airplane was a little off the ground, wait for the airplane to settle.
//     -> INITIAL
//   WAIT_FOR_FUELTRUCK
//     -> REFUELING
//   REFUELING
//     -> DISMISS_FUELTRUCK
//   DISMISS_FUELTRUCK
//     -> TAXI_ON_CARRIER or TAXI_FOR_TAKEOFF
//   TAXI_ON_CARRIER
//     -> READY_TO_TAKEOFF
//   TAXI_FOR_TAKEOFF
//     -> READY_TO_TAKEOFF
//   READY_TO_TAKEOFF  : On the runway threshold, ready to take off
//     -> TAKEOFF_CLIMB
//   TAKEOFF_CLIMB
//     -> PATROL or FLYING_TOWARD_BOGGY
//   IN_MISSION
//     Substate:
//     PATROL            : Circle over the base.
//     -> FLYING_TOWARD_BOGGY or LANDING
//     FLYING_TOWARD_BOGGY
//       -> INTERCEPT or ENGAGE
//     INTERCEPT
//       -> ENGAGE or APPROACH
//     ENGAGE            : Engaging the enemy
//       -> APPROACH
//   APPROACH          : Returning to the base
//     -> LANDING
//   LANDING
//     -> TAXI_BACK or TAXI_ON_CARRIER
//   TAXI_BACK
//     -> WAIT_FOR_FUELTRUCK

FsMissionAutopilot::SameStateGuard::SameStateGuard(FsMissionAutopilot *ap,const double dt)
{
	this->ap=ap;
	this->prevState=ap->state;
	this->dt=dt;
}

FsMissionAutopilot::SameStateGuard::~SameStateGuard()
{
	if(this->prevState==ap->state)
	{
		ap->sameStateTimer+=dt;
	}
	else
	{
		ap->sameStateTimer=0.0;
	}
}



////////////////////////////////////////////////////////////


FsMissionAutopilot::FsMissionAutopilot()
{
	landingAP=NULL;
	cruiseAP=NULL;
	taxiingAP=NULL;
	takeoffAP=NULL;
	CleanUp();
}

FsMissionAutopilot::~FsMissionAutopilot()
{
	ClearSubAutoPilot();
}

/* static */ const char *FsMissionAutopilot::StateToString(FsMissionAutopilot::STATE state)
{
	switch(state)
	{
	case STATE_INITIAL:
		return "STATE_INITIAL";
	case STATE_WAITING_FOR_SETTLING:
		return "STATE_WAITING_FOR_SETTLING";
	case STATE_WAIT_FOR_FUELTRUCK:
		return "STATE_WAIT_FOR_FUELTRUCK";
	case STATE_REFUELING:
		return "STATE_REFUELING";
	case STATE_DISMISS_FUELTRUCK:
		return "STATE_DISMISS_FUELTRUCK";
	case STATE_TAXI_ON_CARRIER:
		return "STATE_TAXI_ON_CARRIER";
	case STATE_TAXI_FOR_TAKEOFF:
		return "STATE_TAXI_FOR_TAKEOFF";
	case STATE_TAKEOFF:
		return "STATE_TAKEOFF";
	case STATE_HOLDING:
		return "STATE_HOLDING";
	case STATE_IN_MISSION:
		return "STATE_IN_MISSION";
	case STATE_RTB:
		return "STATE_RTB";
	case STATE_APPROACH:
		return "STATE_APPROACH";
	case STATE_STATIONARY:
		return "STATE_STATIONARY";
	case STATE_REFUELING_ON_CARRIER:
		return "STATE_REFUELING_ON_CARRIER";
	default:
		break;
	}
	return "STATE_ERROR";
}

/* static */ FsMissionAutopilot::STATE FsMissionAutopilot::StringToState(const char str[])
{
	if(0==strcmp(str,StateToString(STATE_INITIAL)))
	{
		return STATE_INITIAL;
	}
	else if(0==strcmp(str,StateToString(STATE_WAITING_FOR_SETTLING)))
	{
		return STATE_WAITING_FOR_SETTLING;
	}
	else if(0==strcmp(str,StateToString(STATE_WAIT_FOR_FUELTRUCK)))
	{
		return STATE_WAIT_FOR_FUELTRUCK;
	}
	else if(0==strcmp(str,StateToString(STATE_REFUELING)))
	{
		return STATE_REFUELING;
	}
	else if(0==strcmp(str,StateToString(STATE_DISMISS_FUELTRUCK)))
	{
		return STATE_DISMISS_FUELTRUCK;
	}
	else if(0==strcmp(str,StateToString(STATE_TAXI_ON_CARRIER)))
	{
		return STATE_TAXI_ON_CARRIER;
	}
	else if(0==strcmp(str,StateToString(STATE_TAXI_FOR_TAKEOFF)))
	{
		return STATE_TAXI_FOR_TAKEOFF;
	}
	else if(0==strcmp(str,StateToString(STATE_TAKEOFF)))
	{
		return STATE_TAKEOFF;
	}
	else if(0==strcmp(str,StateToString(STATE_HOLDING)))
	{
		return STATE_HOLDING;
	}
	else if(0==strcmp(str,StateToString(STATE_IN_MISSION)))
	{
		return STATE_IN_MISSION;
	}
	else if(0==strcmp(str,StateToString(STATE_RTB)))
	{
		return STATE_RTB;
	}
	else if(0==strcmp(str,StateToString(STATE_APPROACH)))
	{
		return STATE_APPROACH;
	}
	else if(0==strcmp(str,StateToString(STATE_STATIONARY)))
	{
		return STATE_STATIONARY;
	}
	else if(0==strcmp(str,StateToString(STATE_REFUELING_ON_CARRIER)))
	{
		return STATE_REFUELING_ON_CARRIER;
	}
	return STATE_ERROR;
}

/* static */ const char *FsMissionAutopilot::ReadinessTypeToString(FsMissionAutopilot::READINESS_TYPE readinessType)
{
	switch(readinessType)
	{
	case READINESS_24HOUR:
		return "READINESS_24HOUR";
	case READINESS_ALERT:
		return "READINESS_ALERT";
	}
	return "READINESS_24HOUR"; // ?
}

/* static */ FsMissionAutopilot::READINESS_TYPE FsMissionAutopilot::StringToReadinessType(const char str[])
{
	if(0==strcmp(str,"READINESS_24HOUR"))
	{
		return READINESS_24HOUR;
	}
	if(0==strcmp(str,"READINESS_ALERT"))
	{
		return READINESS_ALERT;
	}
	return READINESS_24HOUR; // ?
}

void FsMissionAutopilot::ClearSubAutoPilot(void)
{
	if(NULL!=landingAP)
	{
		FsAutopilot::Delete(landingAP);
		landingAP=NULL;
	}
	if(NULL!=cruiseAP)
	{
		FsAutopilot::Delete(cruiseAP);
		cruiseAP=NULL;
	}
	if(NULL!=taxiingAP)
	{
		FsAutopilot::Delete(taxiingAP);
		taxiingAP=NULL;
	}
	if(NULL!=takeoffAP)
	{
		FsAutopilot::Delete(takeoffAP);
		takeoffAP=NULL;
	}
}

void FsMissionAutopilot::ReallocSubAutoPilot(void)
{
	ClearSubAutoPilot();
	landingAP=FsLandingAutopilot::Create();
	cruiseAP=FsGotoPosition::Create();
	taxiingAP=FsTaxiingAutopilot::Create();
	takeoffAP=FsTakeOffAutopilot::Create();
}

void FsMissionAutopilot::ResetTakeoffAutopilot(void)
{
	if(NULL!=takeoffAP)
	{
		FsAutopilot::Delete(takeoffAP);
	}
	takeoffAP=FsTakeOffAutopilot::Create();
}

void FsMissionAutopilot::ResetTaxiingAutopilot(void)
{
	if(NULL!=taxiingAP)
	{
		FsAutopilot::Delete(taxiingAP);
	}
	taxiingAP=FsTaxiingAutopilot::Create();
}

void FsMissionAutopilot::ResetLandingAutopilot(void)
{
	if(NULL!=landingAP)
	{
		FsAutopilot::Delete(landingAP);
	}
	landingAP=FsLandingAutopilot::Create();
}

void FsMissionAutopilot::ResetCruiseAutopilot(void)
{
	if(NULL!=cruiseAP)
	{
		FsAutopilot::Delete(cruiseAP);
	}
	cruiseAP=FsGotoPosition::Create();
}

YsVec3 FsMissionAutopilot::GetAirportPosition(void) const
{
	YsVec3 pos=YsOrigin();
	switch(base.GetType())
	{
	case FsSimInfo::AIRPORT:
		if(NULL!=sceneryCache && NULL!=airportCache)
		{
			pos=sceneryCache->GetRectRegionCenter(airportCache);
		}
		break;
	case FsSimInfo::CARRIER:
		if(NULL!=carrierCache)
		{
			pos=carrierCache->GetPosition();
		}
		break;
	};
	pos.SetY(comeBackAlt);
	return pos;
}

/* virtual */ YSBOOL FsMissionAutopilot::IsTakingOff(void) const
{
	if(STATE_TAKEOFF==state)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsMissionAutopilot::CleanUp(void)
{
	ReallocSubAutoPilot();
	state=STATE_INITIAL;
	readinessType=READINESS_24HOUR;
	sameStateTimer=0.0;
	calledFuelTruck=YSFALSE;
	dismissedFuelTruck=YSFALSE;
	teamAir.CleanUp();

	base.SetType(FsSimInfo::AIRPORT);
	base.SetName("");


	holdingAlt=YsUnitConv::FTtoM(15000.0);
	holdingTime=3600.0; // 1 hour shift
	takeOffClimbAlt=YsUnitConv::FTtoM(5000.0);
	comeBackAlt=YsUnitConv::FTtoM(6000.0);;
	beginApproachDist=YsUnitConv::SMtoM(5.0);
	nMustBeInTheAir=1;
	nEngage=1;
	evasiveManeuverGLimit=5.0;
	rtbFuelThr=0.2;
	rtbRunOutSAAM=YSTRUE;
	rtbRunOutAAM=YSTRUE;
	rtbRunOutAGM=YSFALSE;
	rtbRunOutGun=YSTRUE;


	cached=YSFALSE;
	carrierCache=NULL;
	sceneryCache=NULL;
	airportCache=NULL;


	emm=EMM_NONE;
	emergencyTimer=0.0;
	chaffFlareTimer=0.0;
}

void FsMissionAutopilot::StayStill(FsAirplane &air)
{
	air.Prop().TurnOffSpeedController();
	air.Prop().SetThrottle(0.0);
	air.Prop().SetAfterburner(YSFALSE);
	air.Prop().SetBrake(1.0);
}

void FsMissionAutopilot::SetUpForTakeOff(FsAirplane &air,FsSimulation *sim)
{
	YsVec3 rwO,rwV;
	if(FsTaxiingAutopilot::MODE_TAKEOFF_ON_CARRIER==taxiingAP->GetMode())
	{
		if(NULL!=carrierCache)
		{
			auto carrierProp=carrierCache->Prop().GetAircraftCarrierProperty();
			if(YSTRUE==carrierProp->IsOnCatapult(air.GetPosition()))
			{
				YsVec3 catV=carrierProp->GetCatapultVec();
				carrierCache->GetMatrix().Mul(catV,catV,0.0);
				catV.SetY(0.0);
				catV.Normalize();

				YsVec3 airV=air.GetAttitude().GetForwardVector();
				airV.SetY(0.0);
				airV.Normalize();

				if(YsCos1deg<catV*airV)
				{
					rwO=air.GetPosition();
					rwV=catV;
				}
				else
				{
					state=STATE_ERROR;
					return;
				}
			}
			else
			{
				state=STATE_ERROR;
				return;
			}
		}
		else
		{
			state=STATE_ERROR;
			return;
		}
	}
	else
	{
		if(YSOK!=FsTakeOffAutopilot::FindRunwayCenterLine(rwO,rwV,air,sim))
		{
			state=STATE_ERROR;
			return;
		}
	}

	ResetTakeoffAutopilot();
	takeoffAP->UseRunwayCenterLine(rwO,rwV);
	takeoffAP->desigAlt=air.GetPosition().y()+takeOffClimbAlt;
	state=STATE_TAKEOFF;
}

YSRESULT FsMissionAutopilot::SetUpTaxiForTakeOff(FsAirplane &air,FsSimulation *sim)
{
	YsArray <const YsSceneryPointSet *> taxiPathArray;
	if(YSOK==sim->FindTakeOffTaxiPathWithinReach(taxiPathArray,air.GetPosition()))
	{
		const YsSceneryPointSet *taxiPath=sim->FindTaxiPathBestForWindFromCandidateArray(taxiPathArray);
		if(NULL!=taxiPath)
		{
			printf("Found taxi path for take off.\n");

			ResetTaxiingAutopilot();
			taxiingAP->SetMode(FsTaxiingAutopilot::MODE_TAKEOFF);
			for(int pntIdx=0; pntIdx<taxiPath->GetNumPoint(); ++pntIdx)
			{
				const YsVec3 entry=taxiPath->GetTransformedPoint(pntIdx);
				taxiingAP->AddTaxiPathPoint(entry);
			}
			state=STATE_TAXI_FOR_TAKEOFF;
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsMissionAutopilot::SetUpTaxiingOnAircraftCarrier(FsAirplane &air,FsSimulation *)
{
	const FsGround *onThisCarrier=air.Prop().OnThisCarrier();
	const FsAircraftCarrierProperty *carrierProp=NULL;
	if(NULL!=onThisCarrier && NULL!=(carrierProp=onThisCarrier->Prop().GetAircraftCarrierProperty()))
	{
		ResetTaxiingAutopilot();
		taxiingAP->SetMode(FsTaxiingAutopilot::MODE_TAKEOFF_ON_CARRIER);
		state=STATE_TAXI_ON_CARRIER;
		return YSOK;
	}
	return YSERR;
}

void FsMissionAutopilot::SetUpHolding(FsAirplane &,FsSimulation *)
{
	YsVec3 airportPos=GetAirportPosition();
	airportPos.SetY(holdingAlt);

	ResetCruiseAutopilot();
	cruiseAP->SetSingleDestination(airportPos);

	state=STATE_HOLDING;
}

void FsMissionAutopilot::SetUpRTB(FsAirplane &,FsSimulation *)
{
	YsVec3 airportPos=GetAirportPosition();
	airportPos.SetY(comeBackAlt);

	ResetCruiseAutopilot();
	cruiseAP->SetSingleDestination(airportPos);

	state=STATE_RTB;
}

void FsMissionAutopilot::SetUpApproach(FsAirplane &air,FsSimulation *sim)
{
	const FsGround *ils=NULL;
	const YsSceneryPointSet *vfr=NULL;
	switch(base.GetType())
	{
	case FsSimInfo::AIRPORT:
		if(NULL!=airportCache && NULL!=sceneryCache)
		{
			auto ilsArray=sim->FindILSinRectRegion(airportCache);
			auto vfrArray=sim->FindVFRApproachInRectRegion(airportCache);
			if(0<ilsArray.GetN())
			{
				YsPrintf("%s\n",__FUNCTION__);
				YsPrintf("  ILS auto selected.\n");
				ils=sim->FindIlsBestForWindFromCandidateArray(ilsArray);
			}
			else if(vfrArray.GetN())
			{
				YsPrintf("%s\n",__FUNCTION__);
				YsPrintf("  VFR approach path auto selected.\n");
				vfr=sim->FindVisualApproachBestForWindFromCandidateArray(vfrArray);
			}

		}
		break;
	case FsSimInfo::CARRIER:
		ils=carrierCache;
		break;
	}

	if(NULL!=ils || NULL!=vfr)
	{
		ResetLandingAutopilot();

		landingAP->SetAirplaneInfo(air,YsPi/2.0);
		landingAP->useRunwayClearingPathIfAvailable=YSTRUE;
		landingAP->dontStopAtFarEnoughPosition=YSTRUE;
		if(NULL!=ils)
		{
			landingAP->SetIls(air,sim,ils);
		}
		else
		{
			const YsVec3 tdPos=vfr->GetTransformedPoint(0);
			const YsVec3 p1=vfr->GetTransformedPoint(1);
			const YsVec3 rwDir=YsUnitVector(p1-tdPos);
			landingAP->SetVfr(air,sim,tdPos,rwDir);
		}

		state=STATE_APPROACH;
	}
}

void FsMissionAutopilot::InitializeInTheAir(FsAirplane &air,FsSimulation *sim)
{
	SetUpHolding(air,sim);
	// SetUpMission(air,sim);
	// state=STATE_IN_MISSION;
}

YSRESULT FsMissionAutopilot::SetUpTaxiing(FsAirplane &air,FsSimulation *sim)
{
	const FsGround *onThisCarrier=air.Prop().OnThisCarrier();

	if(NULL==onThisCarrier)
	{
		return SetUpTaxiForTakeOff(air,sim);
	}
	else
	{
		return SetUpTaxiingOnAircraftCarrier(air,sim);
	}
}

void FsMissionAutopilot::Encache(FsAirplane &air,FsSimulation *sim)
{
	if(YSTRUE!=cached)
	{
		const FsField *fld=sim->GetField();
		if(NULL!=fld)
		{
			sceneryCache=fld->GetFieldPtr();
		}

		// If the HOMEBASE is not given in the INTENTIO-ENTINTEN block, and if the airplane has its own
		// assigned home base, use it as the home base.
		if(0==base.GetName().Strlen() && 0<air.GetHomeBaseName().Strlen())
		{
			base.SetType(air.GetHomeBaseType());
			base.SetName(air.GetHomeBaseName());
		}

		if(0==base.GetName().Strlen())
		{
			const FsGround *carrier=air.Prop().OnThisCarrier();
			if(NULL!=carrier)
			{
				base.SetType(FsSimInfo::CARRIER);
				base.SetName(carrier->GetName());
				carrierCache=carrier;
			}
			else if(YSTRUE==air.Prop().IsOnGround())
			{
				const YsSceneryRectRegion *rgn=sim->FindAirportFromPosition(air.GetPosition());
				if(NULL!=rgn)
				{
					airportCache=rgn;
					base.SetType(FsSimInfo::AIRPORT);
					base.SetName(rgn->GetTag());
				}
			}
		}
		else if(FsSimInfo::AIRPORT==base.GetType())
		{
			YsArray <const YsSceneryRectRegion *> rgnArray;
			if(YSOK==sceneryCache->SearchRegionByTag(rgnArray,base.GetName()) && 0<rgnArray.GetN())
			{
				airportCache=rgnArray[0];

				auto ilsArray=sim->FindILSinRectRegion(rgnArray[0]);
				for(auto ils : ilsArray)
				{
					auto carrierProp=ils->Prop().GetAircraftCarrierProperty();
					if(NULL!=carrierProp)
					{
						YsAtt3 rwAtt;
						carrierProp->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
						rwAtt.SetP(0.0);
						rwAtt.Mul(rwDir,-YsZVec());
						break;
					}
				}
			}
		}
		else
		{
			carrierCache=sim->FindIlsOrCarrierByTag(base.GetName());
		}

		cached=YSTRUE;
	}
}

void FsMissionAutopilot::Decache(void)
{
	cached=YSFALSE;
}

int FsMissionAutopilot::GetNumAirplaneInMissionOrTakingOff(FsAirplane &self,FsSimulation *sim)
{
	int nFlying=0;
	YSBOOL includedSelf=YSFALSE;
	for(auto &teamMember : teamAir)
	{
		auto air=teamMember.GetAircraft(sim);
		if(&self==air)
		{
			includedSelf=YSTRUE;
		}
		if(NULL!=air && YSTRUE==air->IsAlive())
		{
			auto *ap=dynamic_cast <FsMissionAutopilot *> (air->GetAutopilot());
			if(ap!=NULL)
			{
				if(STATE_TAKEOFF==ap->state || STATE_IN_MISSION==ap->state)
				{
					++nFlying;
				}
			}
		}
	}

	if(YSTRUE!=includedSelf && (STATE_TAKEOFF==state || STATE_IN_MISSION==state))
	{
		++nFlying;
	}

	return nFlying;
}

int FsMissionAutopilot::GetNumAirplaneInHoldingOrTakingOff(FsAirplane &self,FsSimulation *sim)
{
	int nHolding=0;
	YSBOOL includedSelf=YSFALSE;
	for(auto &teamMember : teamAir)
	{
		auto air=teamMember.GetAircraft(sim);
		if(&self==air)
		{
			includedSelf=YSTRUE;
		}
		if(NULL!=air && YSTRUE==air->IsAlive())
		{
			auto *ap=dynamic_cast <FsMissionAutopilot *> (air->GetAutopilot());
			if(ap!=NULL)
			{
				if(STATE_TAKEOFF==ap->state || STATE_HOLDING==ap->state)
				{
					++nHolding;
				}
			}
		}
	}

	if(YSTRUE!=includedSelf && (STATE_TAKEOFF==state || STATE_HOLDING==state))
	{
		++nHolding;
	}

	return nHolding;
}

YSBOOL FsMissionAutopilot::CheckReturnToBaseCondition(FsAirplane &air,FsSimulation *)
{
	if(STATE_HOLDING==state && holdingTime<=sameStateTimer)
	{
		return YSTRUE;
	}
	if(rtbFuelThr*air.Prop().GetMaxFuelLoad()>air.Prop().GetFuelLeft())
	{
		return YSTRUE;
	}
	if(0==air.Prop().GetNumWeapon(FSWEAPON_AIM9)+air.Prop().GetNumWeapon(FSWEAPON_AIM9X) && YSTRUE==rtbRunOutSAAM)
	{
		return YSTRUE;
	}
	if(0==air.Prop().GetNumWeapon(FSWEAPON_AIM9)+air.Prop().GetNumWeapon(FSWEAPON_AIM9X)+air.Prop().GetNumWeapon(FSWEAPON_AIM120) && 
	   YSTRUE==rtbRunOutAAM)
	{
		return YSTRUE;
	}
	if(0==air.Prop().GetNumWeapon(FSWEAPON_AGM65) && YSTRUE==rtbRunOutAGM)
	{
		return YSTRUE;
	}
	if(0==air.Prop().GetNumWeapon(FSWEAPON_GUN) && YSTRUE==rtbRunOutGun)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

/* virtual */ YSRESULT FsMissionAutopilot::MakePriorityDecision(FsAirplane &air)
{
	switch(state)
	{
	default:
	case STATE_INITIAL:
	case STATE_ERROR:
	case STATE_STATIONARY:
	case STATE_WAIT_FOR_FUELTRUCK:
	case STATE_REFUELING:
	case STATE_DISMISS_FUELTRUCK:
		return YSOK;
	case STATE_TAXI_FOR_TAKEOFF:
		if(NULL!=taxiingAP)
		{
			taxiingAP->MakePriorityDecision(air);
		}
		break;
	case STATE_TAKEOFF:
		if(NULL!=takeoffAP)
		{
			takeoffAP->MakePriorityDecision(air);
		}
		break;
	case STATE_RTB:
		if(NULL!=cruiseAP)
		{
			cruiseAP->MakePriorityDecision(air);
		}
		break;
	case STATE_APPROACH:
		if(NULL!=landingAP)
		{
			landingAP->MakePriorityDecision(air);
		}
		break;
	}
	return YSOK;
}

/* virtual */ YSRESULT FsMissionAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(emm!=EMM_NONE)
	{
		emergencyTimer-=dt;
		if(0.0>emergencyTimer)
		{
			ClearEmergencyManeuver(air);
		}
		return YSOK;
	}

	SameStateGuard sameStateGuard(this,dt);

	switch(state)
	{
	case STATE_ERROR:
		break;
	case STATE_INITIAL:
		// Just do nothing for the first 0.5 seconds.
		// Some airplanes start just a little bit off the ground, and the initial state will be settled in the first second.
		if(0.9<air.Prop().GetLandingGear() &&
		   air.GetApproximatedCollideRadius()>air.GetAGL() &&
		   air.Prop().GetVelocity()<0.5 &&
		   YSTRUE!=air.Prop().IsOnGround())
		{
			state=STATE_WAITING_FOR_SETTLING;
			return YSOK;
		}

		// To be absolutely sure, wait for .3 seconds before starting an action.
		// But, this test must be after abve STATE_WAITING_UNTIL_SETTLE test.
		// Otherwise, the airplane may fall and get velocity within 0.3 seconds, and the above test may fail,
		// and ends up launching airplanes from the ramp.
		if(0.3>sameStateTimer)
		{
			// Just do nothing for the first 0.5 seconds.
			// Some airplanes start just a little bit off the ground, and the initial state will be settled in the first second.
			break;
		}

		Encache(air,sim);

		if(YSTRUE==air.Prop().IsOnGround())
		{
			SetUpTaxiing(air,sim);
		}
		else
		{
			InitializeInTheAir(air,sim);
		}

		break;
	case STATE_WAITING_FOR_SETTLING:
		if(YSTRUE==air.Prop().IsOnGround())
		{
			state=STATE_INITIAL; // Try again.
		}
		break;
	case STATE_TAXI_FOR_TAKEOFF:
		if(NULL!=taxiingAP)
		{
			if(READINESS_24HOUR==readinessType)
			{
				if(nMustBeInTheAir<=GetNumAirplaneInHoldingOrTakingOff(air,sim))
				{
					taxiingAP->SetHoldShortOfRunway(YSTRUE);
				}
				else
				{
					taxiingAP->SetHoldShortOfRunway(YSFALSE);
				}
			}
			else // READINESS_ALERT
			{
				if(YSTRUE!=StartMissionCondition(air,sim) ||
				   nEngage<=GetNumAirplaneInMissionOrTakingOff(air,sim))
				{
					taxiingAP->SetHoldShortOfRunway(YSTRUE);
				}
				else
				{
					taxiingAP->SetHoldShortOfRunway(YSFALSE);
				}
			}

			taxiingAP->MakeDecision(air,sim,dt);
			if(YSTRUE==taxiingAP->MissionAccomplished(air,sim) &&
			   nMustBeInTheAir>GetNumAirplaneInMissionOrTakingOff(air,sim))
			{
				SetUpForTakeOff(air,sim);
			}
		}
		break;
	case STATE_HOLDING:
		SetUpEvasiveManeuverIfMissileIsChasing(air,sim);

		if(FsSimInfo::CARRIER==base.GetType() && NULL!=carrierCache)
		{
			YsVec3 pos=carrierCache->GetPosition();
			pos.SetY(holdingAlt);
			cruiseAP->SetSingleDestination(pos);
		}
		if(holdingTime<=sameStateTimer ||
		   YSTRUE==CheckReturnToBaseCondition(air,sim))
		{
			SetUpRTB(air,sim);
		}
		else if(YSTRUE==StartMissionCondition(air,sim) &&
		   nEngage>GetNumAirplaneInMissionOrTakingOff(air,sim))
		{
			SetUpMission(air,sim);
			state=STATE_IN_MISSION;
		}
		else
		{
			cruiseAP->MakeDecision(air,sim,dt);
		}
		break;
	case STATE_TAXI_ON_CARRIER:
		if(NULL!=taxiingAP)
		{
			taxiingAP->MakeDecision(air,sim,dt);
			if(YSTRUE==taxiingAP->MissionAccomplished(air,sim) &&
			   nMustBeInTheAir>GetNumAirplaneInMissionOrTakingOff(air,sim))
			{
				state=STATE_REFUELING_ON_CARRIER;
			}
		}
		break;
	case STATE_REFUELING_ON_CARRIER:
		if(10.0<=sameStateTimer)
		{
			if(READINESS_24HOUR==readinessType &&
			   nMustBeInTheAir>GetNumAirplaneInHoldingOrTakingOff(air,sim))
			{
				SetUpForTakeOff(air,sim);
			}
			else if(READINESS_ALERT==readinessType &&
			        nEngage>GetNumAirplaneInMissionOrTakingOff(air,sim))
			{
				SetUpForTakeOff(air,sim);
			}
		}
		break;
	case STATE_TAKEOFF:
		if(NULL!=takeoffAP)
		{
			takeoffAP->MakeDecision(air,sim,dt);
			if(YSTRUE==takeoffAP->MissionAccomplished(air,sim))
			{
				if(READINESS_24HOUR==readinessType)
				{
					SetUpHolding(air,sim);
				}
				else
				{
					SetUpMission(air,sim);
					state=STATE_IN_MISSION;
				}
			}
		}
		break;

	// STATE_IN_MISSION must be implemented in the sub-class.
	// Must call SetUpRTB(sim) for switching to STATE_RTB
	// SetUpRTB will set the state to STATE_RTB

	case STATE_RTB:
		{
			SetUpEvasiveManeuverIfMissileIsChasing(air,sim);

			if(FsSimInfo::CARRIER==base.GetType() && NULL!=carrierCache)
			{
				YsVec3 pos=carrierCache->GetPosition();
				pos.SetY(comeBackAlt);
				cruiseAP->SetSingleDestination(pos);
			}
			cruiseAP->MakeDecision(air,sim,dt);
			const YsVec3 diff=GetAirportPosition()-air.GetPosition();
			const double d=diff.GetSquareLengthXZ();
			if(d<YsSqr(beginApproachDist))
			{
				SetUpApproach(air,sim);
			}
		}
		break;

	case STATE_APPROACH:
		if(NULL!=landingAP)
		{
			landingAP->MakeDecision(air,sim,dt);
			if(FSGROUNDSTATIC==air.Prop().GetFlightState())
			{
				state=STATE_STATIONARY;
			}
		}
		break;
	case STATE_STATIONARY:
		if(NULL!=air.Prop().OnThisCarrier())
		{
			SetUpTaxiing(air,sim);
		}
		else
		{
			YSBOOL fuel,ammo;
			FsGround *fuelTruck=sim->FindNearbySupplyTruck(fuel,ammo,air);
			if(NULL!=fuelTruck)
			{
				state=STATE_REFUELING;
				calledFuelTruck=YSFALSE;
			}
			else
			{
				FsGround *fuelTruck=sim->FindNearestSupplyTruckInTheSameRamp(air.GetPosition());
				if(NULL!=fuelTruck)
				{
					state=STATE_WAIT_FOR_FUELTRUCK;
					calledFuelTruck=YSFALSE;
				}
				else
				{
					SetUpTaxiing(air,sim);
				}
			}
		}
		break;
	case STATE_WAIT_FOR_FUELTRUCK:
		{
			YSBOOL fuel,ammo;
			FsGround *fuelTruck=sim->FindNearbySupplyTruck(fuel,ammo,air);
			if(NULL!=fuelTruck && fuelTruck->Prop().GetVelocity()<YsTolerance)
			{
				state=STATE_REFUELING;
			}
		}
		break;
	case STATE_REFUELING:
		if(10.0<=sameStateTimer)
		{
			state=STATE_DISMISS_FUELTRUCK;
			dismissedFuelTruck=YSFALSE;
		}
		break;
	case STATE_DISMISS_FUELTRUCK:
		if(YSTRUE==calledFuelTruck)
		{
			if(5.0<sameStateTimer)
			{
				YSBOOL fuel,ammo;
				FsGround *fuelTruck=sim->FindNearbySupplyTruck(fuel,ammo,air);
				if(NULL==fuelTruck)
				{
					SetUpTaxiing(air,sim);
				}
			}
		}
		else
		{
			SetUpTaxiing(air,sim);
		}
		break;
	case STATE_IN_MISSION:
		// Must be handled by the sub-class.
		break;
	}
	return YSOK;
}

/* virtual */ YSRESULT FsMissionAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(emm==EMM_EVADEMISSILE)
	{
		chaffFlareTimer-=dt;
		if(0.0>chaffFlareTimer)
		{
			air.Prop().SetDispenseFlareButton(YSFALSE);
			air.Prop().SetDispenseFlareButton(YSTRUE);
			chaffFlareTimer=4.0;
		}

		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(1.0);
		air.Prop().SetAfterburner(YSTRUE);

		if(emergencyTimer<4.0)
		{
			air.Prop().BankController(YsPi*7.0/18.0);
		}
		else
		{
			air.Prop().BankController(-YsPi*7.0/18.0);
		}

		air.Prop().GController(evasiveManeuverGLimit);
		return YSOK;
	}


	air.Prop().NeutralDirectAttitudeControl();

	switch(state)
	{
	case STATE_INITIAL:
	case STATE_WAITING_FOR_SETTLING:
	case STATE_ERROR:
		StayStill(air);
		break;
	case STATE_TAXI_FOR_TAKEOFF:
	case STATE_TAXI_ON_CARRIER:
		if(NULL!=taxiingAP)
		{
			taxiingAP->ApplyControl(air,sim,dt);
		}
		break;
	case STATE_TAKEOFF:
		if(NULL!=takeoffAP)
		{
			takeoffAP->ApplyControl(air,sim,dt);
		}
		break;

	// STATE_IN_MISSION must be implemented in the sub-class.
	// Must call SetUpRTB(sim) for switching to STATE_RTB
	// SetUpRTB will set the state to STATE_RTB

	case STATE_HOLDING:
		if(NULL!=cruiseAP)
		{
			air.Prop().TurnOffSpeedController();
			cruiseAP->SetThrottle(0.8);
			cruiseAP->SetUseAfterburner(YSFALSE);
			cruiseAP->ApplyControl(air,sim,dt);
		}
		break;
	case STATE_RTB:
		if(NULL!=cruiseAP)
		{
			cruiseAP->ApplyControl(air,sim,dt);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetFlap(0.0);
			air.Prop().SetBrake(0.0);
			air.Prop().SetSpoiler(0.0);
			if(120.0>sameStateTimer)
			{
				air.Prop().SetThrottle(1.0);
				air.Prop().SetAfterburner(YSTRUE);
			}
			else
			{
				air.Prop().SetThrottle(1.0);
				air.Prop().SetAfterburner(YSFALSE);
			}
		}
		break;

	case STATE_APPROACH:
		if(NULL!=landingAP)
		{
			landingAP->ApplyControl(air,sim,dt);
		}
		break;
	case STATE_STATIONARY:
		StayStill(air);
		break;
	case STATE_WAIT_FOR_FUELTRUCK:
		StayStill(air);
		if(YSTRUE!=calledFuelTruck)
		{
			FsGround *fuelTruck=sim->FindNearestSupplyTruckInTheSameRamp(air.GetPosition());
			if(NULL!=fuelTruck)
			{
				FsAutoDriveToExactPosition *autoDrive2=new FsAutoDriveToExactPosition;
				autoDrive2->SetUp(sim,fuelTruck,fuelTruck->initPosition,fuelTruck->initAttitude.h());
				autoDrive2->holdUntil.SetHoldUntilThisObjectGoesAway(&air);
				fuelTruck->PushAutoDrive(autoDrive2);

				FsAutoDriveToObject *autoDrive1=new FsAutoDriveToObject;
				autoDrive1->SetUp(sim,fuelTruck,&air);
				fuelTruck->PushAutoDrive(autoDrive1);

				calledFuelTruck=YSTRUE;
			}
		}
		break;
	case STATE_REFUELING:
	case STATE_REFUELING_ON_CARRIER:
		StayStill(air);
		if(5.0<=sameStateTimer)
		{
			YSBOOL fuel,ammo;
			FsGround *fuelTruck=sim->FindNearbySupplyTruck(fuel,ammo,air);
			if(NULL!=fuelTruck)
			{
				if(fuelTruck->Prop().chFlags&(FsGroundProperty::YSGP_SUPPLYFUEL))
				{
					air.Prop().LoadFuel();
				}
				if(fuelTruck->Prop().chFlags&(FsGroundProperty::YSGP_SUPPLYAMMO))
				{
					air.RecallReloadCommandOnly();
				}
			}
		}
		break;
	case STATE_DISMISS_FUELTRUCK:
		StayStill(air);
		if(YSTRUE==calledFuelTruck && YSTRUE!=dismissedFuelTruck)
		{
			YSBOOL fuel,ammo;
			FsGround *fuelTruck=sim->FindNearbySupplyTruck(fuel,ammo,air);
			if(NULL!=fuelTruck)
			{
				FsAutoDriveToExactPosition *autoDrive=new FsAutoDriveToExactPosition;
				autoDrive->SetUp(sim,fuelTruck,fuelTruck->initPosition,fuelTruck->initAttitude.h());
				autoDrive->StartWithThreePointTurn(sim,fuelTruck);
				autoDrive->ExcludeFromCollisionTest(&air); // This will prevent the dead lock at the beginning.
				fuelTruck->SetAutoDrive(autoDrive);
			}
			for(int i=0; i<sim->GetNumSupplyVehicle(); ++i)
			{
				FsGround *fuelTruck=sim->GetSupplyVehicle(i);
				if(NULL!=fuelTruck &&
				   YSTRUE==fuelTruck->IsAlive() &&
				   YSTRUE==fuelTruck->IsAutoDrivingTo(&air))
				{
					FsAutoDriveToExactPosition *autoDrive=new FsAutoDriveToExactPosition;
					autoDrive->SetUp(sim,fuelTruck,fuelTruck->initPosition,fuelTruck->initAttitude.h());
					autoDrive->StartWithThreePointTurn(sim,fuelTruck);
					autoDrive->ExcludeFromCollisionTest(&air); // This will prevent the dead lock at the beginning.
					fuelTruck->SetAutoDrive(autoDrive);
				}
			}
			dismissedFuelTruck=YSTRUE;
		}
		break;
	case STATE_IN_MISSION:
		// Must be handled by the sub-class.
		break;
	}
	return YSOK;
}

/* virtual */ YSRESULT FsMissionAutopilot::SaveIntention(FILE *fp,const FsSimulation *sim)
{
	fprintf(fp,"READINESS %s\n",ReadinessTypeToString(readinessType));
	fprintf(fp,"STATE %s\n",StateToString(state));
	fprintf(fp,"SAMESTATE %.2lfsec\n",sameStateTimer);

	for(auto &teamMember : teamAir)
	{
		auto air=sim->FindAirplane(teamMember.objKeyCache);
		if(0<teamMember.yfsLabel.Strlen())
		{
			fprintf(fp,"TEAMLABEL %s\n",teamMember.yfsLabel.Txt());
		}
		else if(0<=teamMember.yfsIdx)
		{
			fprintf(fp,"TEAMID %d\n",teamMember.yfsIdx);
		}
		else if(NULL!=air)
		{
			fprintf(fp,"TEAMID %d\n",sim->GetAirplaneIdFromHandle(air));
		}
	}

	fprintf(fp,"HOMEBASE %s \"%s\"\n",FsSimInfo::BaseTypeToString(base.GetType()),base.GetName().Txt());

	fprintf(fp,"TAKEOFFALT %.2lfm\n",takeOffClimbAlt);
	fprintf(fp,"COMEBACKALT %.2lfm\n",comeBackAlt);
	fprintf(fp,"APPROACHDIST %.2lfm\n",beginApproachDist);
	fprintf(fp,"NMUSTBEINTHEAIR %d\n",nMustBeInTheAir);
	fprintf(fp,"NENGAGE %d\n",nEngage);
	fprintf(fp,"EVASIVEGLIMIT %lf\n",evasiveManeuverGLimit);

	fprintf(fp,"HOLDINGALT %.2lfm\n",holdingAlt);
	fprintf(fp,"HOLDINGTIME %.2lfsec\n",holdingTime);
	fprintf(fp,"RTBFUELTHR %.2lf%%\n",rtbFuelThr*100.0);
	fprintf(fp,"RTBRUNOUTSAAM %s\n",YsBoolToStr(rtbRunOutSAAM));
	fprintf(fp,"RTBRUNOUTAAM %s\n",YsBoolToStr(rtbRunOutAAM));
	fprintf(fp,"RTBRUNOUTAGM %s\n",YsBoolToStr(rtbRunOutAGM));
	fprintf(fp,"RTBRUNOUTGUN %s\n",YsBoolToStr(rtbRunOutGun));


	return YSOK;
}

YSRESULT FsMissionAutopilot::ReadIntentionMissionPart(YsTextInputStream &,YsArray <YsString,16> &args)
{
	if(0<args.GetN())
	{
		if(0==strcmp("READINESS",args[0]))
		{
			if(2<=args.GetN())
			{
				readinessType=StringToReadinessType(args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("STATE",args[0]))
		{
			if(2<=args.GetN())
			{
				state=StringToState(args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("SAMESTATE",args[0]))
		{
			if(2<=args.GetN())
			{
				sameStateTimer=atof(args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("TEAMLABEL",args[0]))
		{
			for(int idx=1; idx<args.GetN(); ++idx)
			{
				teamAir.Increment();
				teamAir.Last().Initialize();
				teamAir.Last().yfsLabel=args[idx];
			}
			return YSOK;
		}
		else if(0==strcmp("TEAMID",args[0]))
		{
			for(int idx=1; idx<args.GetN(); ++idx)
			{
				teamAir.Increment();
				teamAir.Last().Initialize();
				teamAir.Last().yfsIdx=atoi(args[idx]);
			}
			return YSOK;
		}
		else if(0==strcmp("HOMEBASE",args[0]))
		{
			if(3<=args.GetN())
			{
				base.SetType(FsSimInfo::StringToBaseType(args[1]));
				base.SetName(args[2]);
			}
			return YSOK;
		}
		else if(0==strcmp("TAKEOFFALT",args[0]))
		{
			if(2<=args.GetN())
			{
				FsGetLength(takeOffClimbAlt,args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("COMEBACKALT",args[0]))
		{
			if(2<=args.GetN())
			{
				FsGetLength(comeBackAlt,args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("APPROACHDIST",args[0]))
		{
			if(2<=args.GetN())
			{
				FsGetLength(beginApproachDist,args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("NMUSTBEINTHEAIR",args[0]))
		{
			if(2<=args.GetN())
			{
				nMustBeInTheAir=atoi(args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("NENGAGE",args[0]))
		{
			if(2<=args.GetN())
			{
				nEngage=atoi(args[1]);
			}
			return YSOK;
		}
		else if(0==strcmp("HOLDINGALT",args[0]))
		{
			if(2<=args.GetN())
			{
				FsGetLength(holdingAlt,args[1]);
			}
		}
		else if(0==strcmp("HOLDINGTIME",args[0]))
		{
			if(2<=args.GetN())
			{
				holdingTime=atof(args[1]);
			}
		}
		else if(0==strcmp("RTBFUELTHR",args[0]))
		{
			if(2<=args.GetN())
			{
				rtbFuelThr=atof(args[1])/100.0;
			}
		}
		else if(0==strcmp("RTBRUNOUTSAAM",args[0]))
		{
			if(2<=args.GetN())
			{
				rtbRunOutSAAM=YsStrToBool(args[1]);
			}
		}
		else if(0==strcmp("RTBRUNOUTGUN",args[0]))
		{
			if(2<=args.GetN())
			{
				rtbRunOutGun=YsStrToBool(args[1]);
			}
		}
		else if(0==strcmp("RTBRUNOUTAAM",args[0]))
		{
			if(2<=args.GetN())
			{
				rtbRunOutAAM=YsStrToBool(args[1]);
			}
		}
		else if(0==strcmp("RTBRUNOUTAGM",args[0]))
		{
			if(2<=args.GetN())
			{
				rtbRunOutAGM=YsStrToBool(args[1]);
			}
		}
		else if(0==strcmp("EVASIVEGLIMIT",args[0]))
		{
			if(2<=args.GetN())
			{
				evasiveManeuverGLimit=atof(args[1]);
			}
		}
		// else if(0==strcmp("",args[0]))
		// {
		// }
	}
	return YSERR;
}


// Pattern A only one aircraft
//   Keep the aircraft on the ground.  Standby at the runway threshold.
//   When an enemy comes within K1 kilometers within the base, launch and engage it.
//   When an anemy leaves K2 kilometers within the base, return to the base, resupply, and standby

// Pattern B team of multiple aircrafts (24-hour patrol)
//   Team of N aircrafts.
//   Keep M aircrafts in the air (or combat ready) all the time
//   When an enemy aircraft (or ground object) moves in to the ADIZ (K1 kilometers within the base), engage it.
//   When all enemy aircrafts leaves within K2 kilometers within the base, return to the base.  Launch new aircraft.
// Need to keep track:
//     nStandby
//     nPatrolling
//     nEngaging
//     nReturning

// Pattern C team of multiple aircrafts (Alert)
//   Team of N aircrafts.
//   When an enemy comes close within K1 kilometers within the base, launch M aircrafts and engage it.
//   

// Question: Do I need to remember the pattern?  Yes.  Need to remember pattern B or C


/* virtual */ YSBOOL FsMissionAutopilot::DoesRespondToRadioCall(void) const
{
	if(STATE_HOLDING==state || STATE_RTB==state || STATE_IN_MISSION==state || STATE_TAKEOFF==state)
	{
		return YSTRUE;
	}
	if(STATE_APPROACH==state && NULL!=landingAP)
	{
		return landingAP->DoesRespondToRadioCall();
	}
	return YSFALSE;
}

void FsMissionAutopilot::SetHomeBaseName(const YsString &str)
{
	base.SetName(str);
}

void FsMissionAutopilot::SetHomeBaseType(FsSimInfo::BASE_TYPE baseType)
{
	base.SetType(baseType);
}

void FsMissionAutopilot::SetUpEvadingFromMissile(void)
{
	emm=EMM_EVADEMISSILE;
	emergencyTimer=6.0;
	chaffFlareTimer=1.0;
}

void FsMissionAutopilot::ClearEmergencyManeuver(FsAirplane &air)
{
	emm=EMM_NONE;
	air.Prop().TurnOffController();
}

void FsMissionAutopilot::SetUpEvasiveManeuverIfMissileIsChasing(FsAirplane &air,const FsSimulation *sim)
{
	FSWEAPONTYPE chasingWeaponType;
	YsVec3 chasingWeaponPos;
	if(sim->IsMissileChasing(chasingWeaponType,chasingWeaponPos,&air)==YSTRUE)
	{
		if((chasingWeaponPos-air.GetPosition()).GetSquareLength()<2000.0*2000.0)
		{
			SetUpEvadingFromMissile();
		}
	}
}
