#include <ysclass.h>

#include <fsairproperty.h>

#include "fs.h"
#include "fsautodrive.h"
#include "fsdefenderautopilot.h"
#include "fsrefuelandtakeoff.h"
#include <ysunitconv.h>

// Climbing Attitude is hard coded. It should be written in
// Aircraft data file in the future.




// #define CRASHINVESTIGATION
// #define CRASHINVESTIGATION_AIRROUTE
// #define CRASHINVESTIGATION_TAXIING

FsAutopilot::AutoFlareDispenser::AutoFlareDispenser()
{
	nextDispensingTimer=0.0;
	dispensingInterval=10.0;
	missileDistanceThreshold=2000.0;
	missileChasing=YSFALSE;
	prevMissileChasing=YSFALSE;
	timerUpCounter=0;
	flareBtn=YSFALSE;
}
void FsAutopilot::AutoFlareDispenser::SetDispensingInterval(double t)
{
	dispensingInterval=t;
	if(t<nextDispensingTimer)
	{
		nextDispensingTimer=t;
	}
}
void FsAutopilot::AutoFlareDispenser::SetMissileDistanceThreshold(double dist)
{
	missileDistanceThreshold=dist;
}
YSBOOL FsAutopilot::AutoFlareDispenser::MissileChasing(void) const
{
	return missileChasing;
}
int FsAutopilot::AutoFlareDispenser::GetTimerUpCounter(void) const
{
	return timerUpCounter;
}
void FsAutopilot::AutoFlareDispenser::SetFlareInterval(double t)
{
	dispensingInterval=t;
}
double FsAutopilot::AutoFlareDispenser::GetFlareInterval(void) const
{
	return dispensingInterval;
}
YSBOOL FsAutopilot::AutoFlareDispenser::GetFlareButton(void) const
{
	return flareBtn;
}
/* virtual */ YSRESULT FsAutopilot::AutoFlareDispenser::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	FSWEAPONTYPE chasingMissileType;
	YsVec3 chasingMissilePos;

	prevMissileChasing=missileChasing;
	missileChasing=sim->IsMissileChasing(chasingMissileType,chasingMissilePos,&air);

	if(nextDispensingTimer>0.0)
	{
		nextDispensingTimer-=dt;
		if(nextDispensingTimer<=YsTolerance)
		{
			++timerUpCounter;
		}
	}
	if(missileChasing==YSTRUE && (prevMissileChasing!=YSTRUE || nextDispensingTimer<=YsTolerance))
	{
		flareBtn=YSTRUE;
		nextDispensingTimer=dispensingInterval;
	}
	else
	{
		flareBtn=YSFALSE;
	}
	return YSOK;
}
/* virtual */ YSRESULT FsAutopilot::AutoFlareDispenser::ApplyControl(FsAirplane &air,FsSimulation *,const double &)
{
	if(YSTRUE==flareBtn)
	{
		air.Prop().PressVirtualButton(FsAirplaneProperty::VBT_DISPENSEFLARE);
	}
	return YSOK;
}

////////////////////////////////////////////////////////////

const char *const FsAutopilot::intentionList[]=
{
	"NOTHING_",
	"MINIALTI",
	"ENDINTEN",
	"GOTOPOSI",
	"DOGFIGHT",
	"FORMATIO",
	"GNDATACK",
	"LANDING_",
	"AIRSHOW_",
	"TAKEOFF_",
	"HOVER___",
	"VERTLAND",
	"TAXIING_",
	"AIRROUTE",
	"DEFENDER",
	"REFUEL__",
	NULL
};

FsAutopilot *FsReadIntention(FILE *fp)
{
	YsTextFileInputStream inStream(fp);
	return FsReadIntention(inStream);
}

FsAutopilot *FsReadIntention(YsTextInputStream &inStream)
{
	YsString str;
	YsArray <YsString,16> args;

	YSBOOL minAltOutside=YSFALSE;
	double minAlt=YsUnitConv::FTtoM(1000.0);

	FsAutopilot *ap=NULL;

	YSBOOL endIntention=YSFALSE;
	while(YSTRUE!=endIntention && NULL!=inStream.Gets(str))
	{
		if(YSOK==str.Arguments(args) && 0<args.GetN())
		{
			int cmd;
			if(YsCommandNumber(&cmd,args[0],FsAutopilot::intentionList)==YSOK)
			{
				switch(cmd)
				{
				case 0:// "NOTHING_",
					ap=NULL;
					break;
				case 1:// "MINIALTI",
					minAlt=atof(args[1]);
					minAltOutside=YSTRUE;
					break;
				case 2:// "ENDINTEN",
					endIntention=YSTRUE;
					break;
				case 3:// "GOTOPOSI",
					{
						FsGotoPosition *gotoposition=FsGotoPosition::Create();
						if(YSOK==gotoposition->ReadIntention(inStream,str))
						{
							ap=gotoposition;
						}
						else
						{
							FsAutopilot::Delete(gotoposition);
						}
						endIntention=YSTRUE;
					}
					break;
				case 4:// "DOGFIGHT",
					{
						FsDogfight *dogfight=FsDogfight::Create();
						if(YSOK==dogfight->ReadIntention(inStream,str))
						{
							ap=dogfight;
						}
						else
						{
							FsAutopilot::Delete(dogfight);
						}
						endIntention=YSTRUE;
					}
					break;
				case 5:// "FORMATIO",
					{
						FsFormation *formation=FsFormation::Create();
						if(YSOK==formation->ReadIntention(inStream,str))
						{
							ap=formation;
						}
						else
						{
							FsAutopilot::Delete(formation);
						}
						endIntention=YSTRUE;
					}
					break;
				case 6:// "GNDATACK",
					{
						FsGroundAttack *gndattack=FsGroundAttack::Create();
						if(YSOK==gndattack->ReadIntention(inStream,str))
						{
							ap=gndattack;
						}
						else
						{
							FsAutopilot::Delete(gndattack);
						}
						endIntention=YSTRUE;
					}
					break;
				case 7:// "LANDING_",
					{
						FsLandingAutopilot *landing=FsLandingAutopilot::Create();
						landing->autoGoAround=(YSBOOL)atoi(args[1]);
						landing->autoClearRunway=(YSBOOL)atoi(args[2]);
						if(YSOK==landing->ReadIntention(inStream,str))
						{
							ap=landing;
						}
						else
						{
							FsAutopilot::Delete(landing);
						}
						endIntention=YSTRUE;
					}
					break;
				case 8:// "AIRSHOW_",
					ap=NULL;
					break;
				case 9:// "TAKEOFF_",
					{
						FsTakeOffAutopilot *takeoff=FsTakeOffAutopilot::Create();
						if(YSOK==takeoff->ReadIntention(inStream,str))
						{
							ap=takeoff;
						}
						else
						{
							FsAutopilot::Delete(takeoff);
						}
						endIntention=YSTRUE;
					}
					break;
				case 10:// "HOVER___",
					{
						FsHoveringAutopilot *hover=FsHoveringAutopilot::Create();
						if(YSOK==hover->ReadIntention(inStream,str))
						{
							ap=hover;
						}
						else
						{
							FsAutopilot::Delete(hover);
						}
						endIntention=YSTRUE;
					}
					break;
				case 11:// "VERTLAND",
					{
						FsVerticalLandingAutopilot *vertland=FsVerticalLandingAutopilot::Create();
						if(YSOK==vertland->ReadIntention(inStream,str))
						{
							ap=vertland;
						}
						else
						{
							FsAutopilot::Delete(vertland);
						}
						endIntention=YSTRUE;
					}
					break;
				case 12: // "TAXIING_", // 2013/01/19
					{
						FsTaxiingAutopilot *taxiing=FsTaxiingAutopilot::Create();
						if(YSOK==taxiing->ReadIntention(inStream,str))
						{
							ap=taxiing;
						}
						else
						{
							FsAutopilot::Delete(taxiing);
						}
						endIntention=YSTRUE;
					}
					break;
				case 13: // "AIRROUTE", // 2013/01/21
					{
						FsAirRouteAutopilot *airRoute=FsAirRouteAutopilot::Create();
						if(YSOK==airRoute->ReadIntention(inStream,str))
						{
							ap=airRoute;
						}
						else
						{
							FsAutopilot::Delete(airRoute);
						}
						endIntention=YSTRUE;
					}
					break;
				case 14: // "DEFENDER",
					{
						auto *defender=FsDefenderAutopilot::Create();
						if(YSOK==defender->ReadIntention(inStream,str))
						{
							ap=defender;
						}
						else
						{
							FsAutopilot::Delete(defender);
						}
						endIntention=YSTRUE;
					}
					break;
				case 15: // "REFUEL__",
					{
						auto *refuelAndTakeOff=FsRefuelAndTakeOffAutopilot::Create();
						if(YSOK==refuelAndTakeOff->ReadIntention(inStream,str))
						{
							ap=refuelAndTakeOff;
						}
						else
						{
							FsAutopilot::Delete(refuelAndTakeOff);
						}
						endIntention=YSTRUE;
					}
					break;
				}
			}
		}
	}

	if(YSTRUE==minAltOutside && ap!=NULL)
	{
		ap->minAlt=minAlt;
	}

	return ap;
}




// Implementation //////////////////////////////////////////
FsAutopilot::FsAutopilot()
{
	emr=EMR_NONE;
	minAlt=330.0;
	thr=0.0;

	// For target tracking
	iTheataK=0.0;
	prevTheataK=0.0;
	iTheataL=0.0;
	prevTheataL=0.0;

	tInverted=0.0;
	tRecoveryFromInverted=0.0;
	refUvForRecoveryFromInverted=YsYVec();

	prevYaw=0.0;

	nextObjective=NULL;

	// For ShallowPursuit
	shallowPursuitState=0;
	shallowPursuitStateChangeTimer=0.0;
}

FsAutopilot::~FsAutopilot()
{
}

/* static */ void FsAutopilot::Delete(FsAutopilot *ap)
{
	delete ap;
}

/* static */ FsAutopilot *FsAutopilot::ReadIntentionEZ(YsConstArrayMask <YsString> argv)
{
	if(0<argv.size())
	{
		if(0==argv[0].STRCMP("LANDING"))
		{
			return FsLandingAutopilot::Create();
		}
		else if(0==argv[0].STRCMP("CIRCLE"))
		{
			auto ap=FsGotoPosition::Create();
			ap->state=FsGotoPosition::STATE_CAPTURE;
			return ap;
		}
		else if(0==argv[0].STRCMP("STRAIGHT"))
		{
			auto ap=FsGotoPosition::Create();
			ap->state=FsGotoPosition::STATE_CAPTURE;
			ap->straightFlightMode=YSTRUE;
			return ap;
		}
	}
	return nullptr;
}

/* virtual */ YSBOOL FsAutopilot::IsTakingOff(void) const
{
	return YSTFUNKNOWN;
}

/* virtual */ YSBOOL FsAutopilot::IsLanding(void)
{
	return YSTFUNKNOWN;
}

/* virtual */ YSBOOL FsAutopilot::DoesRespondToRadioCall(void) const
{
	return YSTRUE;
}

unsigned FsAutopilot::OverridedControl(void)
{
	return FSAPPLYCONTROL_NONE;
}

YSBOOL FsAutopilot::ObjectiveAccomplished(void)
{
	return YSFALSE;
}

YSRESULT FsAutopilot::Control(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(emr!=EMR_NONE)
	{
		return EmergencyRecovery(air);
	}
	else
	{
		MakePriorityDecision(air);
		if(emr==EMR_NONE)
		{
			if(MakeDecision(air,sim,dt)==YSOK && ApplyControl(air,sim,dt)==YSOK)
			{
				return YSOK;
			}
			return YSERR;
		}
		return YSOK;
	}
}

YSRESULT FsAutopilot::Save(FILE *fp,const FsSimulation *sim)
{
	fprintf(fp,"INTENTIO\n");
	SaveIntention(fp,sim);
	fprintf(fp,"MINIALTI %g\n",minAlt);
	fprintf(fp,"ENDINTEN\n");
	return YSOK;
}

YSRESULT FsAutopilot::GetRelativeAttitude
    (YsAtt3 &rel,const YsAtt3 &attOfInterest,FsAirplane &air)
{
	YsVec3 tev,tuv;
	const YsAtt3 *att,*tatt;


	att=&air.GetAttitude();
	tatt=&attOfInterest;

	tev=tatt->GetForwardVector();
	tuv=tatt->GetUpVector();

	YsMatrix4x4 mat(YSFALSE);
	att->GetMatrix4x4(mat);
	mat.Invert();

	tev=mat*tev;
	tuv=mat*tuv;

	rel.SetTwoVector(tev,tuv);
	return YSOK;
}

double FsAutopilot::GetRelativeBank(const YsAtt3 &attOfInterest,const YsAtt3 &withRespectTo)
{
	YsVec3 uv;
	uv=attOfInterest.GetUpVector();
	withRespectTo.MulInverse(uv,uv);
	return atan2(-uv.x(),uv.y());
}

double FsAutopilot::GetAllowableAltitude(const double &minAlt,const FsAirplane &air)
{
	YsVec3 vel;
	double correctedMinAlt;

	air.Prop().GetVelocity(vel);

	correctedMinAlt=minAlt*sin(-air.GetAttitude().p())-vel.y()*5.0+33.0;;
        // correctedMinAlt    Minimum Altitude with respect to the pitch
        //   minAlt*sin(~)      When the airplane is heading down, minAlt must be largest.
        //   -vel.y()*5.0       Say, if the airplane will go below the minimum altitude in 5 seconds
        //   +33.0              100ft margine

	return YsGreater(correctedMinAlt,33.0);
}

YSBOOL FsAutopilot::MissionAccomplished(FsAirplane &,FsSimulation *) const
{
	return YSFALSE;
}

YSRESULT FsAutopilot::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"NOTHING_\n");
	return YSOK;
}

YSRESULT FsAutopilot::ReadCommonProperty(YSBOOL &endIntention,YSSIZE_T ac,const YsString av[])
{
	endIntention=YSFALSE;
	if(0<ac && (0==strcmp(av[0],"REM") || '#'==av[0][0]))
	{
		return YSOK;
	}
	if(2<=ac && 0==strcmp(av[0],"MINIALTI"))
	{
		minAlt=atof(av[1]);
		return YSOK;
	}
	else if(0<ac && 0==strcmp(av[0],"ENDINTEN"))
	{
		endIntention=YSTRUE;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsAutopilot::MakePriorityDecision(FsAirplane &air)
{
	double vel,spd1;
	YsVec3 dir;
	FSFLIGHTSTATE sta;

	vel=air.Prop().GetVelocity();
	sta=air.Prop().GetFlightState();
	spd1=air.Prop().GetMinimumManeuvableSpeed();

	if(vel<spd1 || sta==FSSTALL)
	{
		emr=EMR_STALL;
		return YSOK;
	}

	air.Prop().GetVelocity(dir);
	if(air.GetPosition().y()<GetAllowableAltitude(minAlt,air))
	{
		emr=EMR_LOWALTITUDE;
	}

	return YSOK;
}

YSRESULT FsAutopilot::EmergencyRecovery(FsAirplane &air)
{
	// printf("Emergency Recovery.\n");
	switch(emr)
	{
	default:
		break;
	case EMR_LOWALTITUDE:
		// printf("Low alt.\n");
		RecoverFromLowAltitude(air,minAlt,8.0);
		break;
	case EMR_HIGHLOWPITCH:
		// printf("High/Low pitch.\n");
		RecoverFromHighLowPitch(air);
		break;
	case EMR_STALL:
		// printf("Stall.\n");
		RecoverFromStall(air);
		break;
	}
	// if(emr==EMR_NONE)
	// {
	//	printf("recovery is done.\n");
	// }
	return YSOK;
}

YSRESULT FsAutopilot::RecoverFromLowAltitude(FsAirplane &air,const double &alt,const double gLimit)
{
	const YsAtt3 *att;
	const YsVec3 *pos;
	att=&air.GetAttitude();
	pos=&air.GetPosition();

	if(pos->y()>3.0 && air.Prop().GetFlightState()==FSFLYING)
	{
		air.Prop().SetGear(0.0);
	}

	if(YsDegToRad(70.0)<att->p())
	{
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(1.0);
		air.Prop().SetSpoiler(0.0);
		air.Prop().SetAfterburner(YSTRUE);
		air.Prop().GController(0.0);
	}
	else if(YsDegToRad(-5.0)<att->p())
	{
		if(pos->y()>33.0)
		{
			double pit,spd;
			spd=air.Prop().GetVelocity();
			pit=15.0+spd-60.0;
			pit=YsBound(pit,15.0,30.0);
			pit=YsDegToRad(pit);
			air.Prop().TurnOffGController();
			air.Prop().PitchController(pit);
		}
		else
		{
			double tailStrikeAngle;

			tailStrikeAngle=air.Prop().GetTailStrikePitchAngle(0.95);

			air.Prop().TurnOffGController();
			air.Prop().PitchController(tailStrikeAngle);
		}

		air.Prop().BankController(0.0);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(1.0);
		air.Prop().SetSpoiler(0.0);
		air.Prop().SetAfterburner(YSTRUE);
	}
	else if(att->p()<YsDegToRad(-70.0))
	{
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.0);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().GController(gLimit);
	}
	else
	{
		if(att->b()<-YsPi/6.0 || YsPi/6.0<att->b())
		{
			air.Prop().SetElevator(0.0);
			air.Prop().BankController(0.0);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.0);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else
		{
			air.Prop().SetAileron(0.0);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSFALSE);
			air.Prop().GController(gLimit);
		}
	}

	if(pos->y()>GetAllowableAltitude(alt,air)+66.0)
	{
		emr=EMR_NONE;
		air.Prop().TurnOffController();
	}

	return YSOK;
}

YSRESULT FsAutopilot::RecoverFromHighLowPitch(FsAirplane &air)
{
	const YsAtt3 *att;
	att=&air.GetAttitude();

	if(att->p()>YsPi*70.0/180.0)
	{
		air.Prop().SetAileron(0.0);
		air.Prop().GController(8.0);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(1.0);
		air.Prop().SetAfterburner(YSTRUE);
	}
	else if(att->p()<-YsPi*70.0/180.0)
	{
		air.Prop().SetAileron(0.0);
		air.Prop().GController(8.0);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.0);
		air.Prop().SetAfterburner(YSFALSE);
	}
	else if(att->p()<-YsPi/18.0)
	{
		air.Prop().BankController(0.0);
		air.Prop().PitchController(0.0);
	}
	else if(att->p()>YsPi/6.0)
	{
		air.Prop().BankController(YsPi);
		air.Prop().PitchController(0.0);
	}
	else
	{
		air.Prop().BankController(0.0);
		air.Prop().PitchController(0.0);
	}

	if(-YsPi/36.0<att->p() && att->p()<YsPi/36.0)
	{
		emr=EMR_NONE;
		air.Prop().TurnOffBankController();
		air.Prop().TurnOffPitchController();
		air.Prop().TurnOffGController();
	}

	return YSOK;
}

YSRESULT FsAutopilot::RecoverFromStall(FsAirplane &air)
{
	air.Prop().TurnOffBankController();
	air.Prop().TurnOffPitchController();
	air.Prop().TurnOffGController();

	air.Prop().SetElevator(0.0);
	air.Prop().SetAileron(0.0);

	air.Prop().TurnOffSpeedController();
	air.Prop().SetThrottle(1.0);
	air.Prop().SetAfterburner(YSTRUE);


	double vel,spd1;
	FSFLIGHTSTATE sta;
	vel=air.Prop().GetVelocity();
	sta=air.Prop().GetFlightState();
	spd1=air.Prop().GetMinimumManeuvableSpeed();

	if(vel>spd1*1.2 && sta!=FSSTALL)
	{
		emr=EMR_NONE;
	}

	return YSOK;
}

YSBOOL FsAutopilot::Accomplished(void)
{
	return YSFALSE;
}

YSRESULT FsAutopilot::CircleAround(FsAirplane &air,FsSimulation *,const double &alt,const double &bnkLimit)
{
	// Just Circle Around

	const YsAtt3 *att;
	att=&air.GetAttitude();


	if(att->p()<-YsDegToRad(60.0))
	{
		air.Prop().TurnOffBankController();
		air.Prop().TurnOffPitchController();
		air.Prop().SpeedController(air.Prop().GetEstimatedLandingSpeed()*1.4);
		air.Prop().GController(3.0);
		air.Prop().SetAileron(0.0);
		air.Prop().SetRudder(0.0);
	}
	else if(att->p()>YsDegToRad(60.0))
	{
		air.Prop().TurnOffBankController();
		air.Prop().TurnOffPitchController();
		air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.8);
		air.Prop().GController(3.0);
		air.Prop().SetAileron(0.0);
		air.Prop().SetRudder(0.0);
	}
	else
	{
		if(air.Prop().GetPosition().y()<alt-330.0)
		{
			air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed());
			air.Prop().BankController(0.0);
			air.Prop().PitchController(YsDegToRad(20.0));
		}
		else if(att->p()>YsDegToRad(30.0))
		{
			air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.8);
			if(YsAbs(att->b())>=YsPi/2.0)
			{
				air.Prop().GController(2.0);
			}
			else
			{
				air.Prop().GController(-1.0);
			}
		}
		else if(att->p()<-YsDegToRad(30.0))
		{
			air.Prop().TurnOffPitchController();
			air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.8);
			air.Prop().BankController(0.0);
			air.Prop().GController(3.0);
		}
		else
		{
			double designatedPitch,diffPitch;

			designatedPitch=(alt-air.Prop().GetPosition().y())*(10.0/330.0);
			designatedPitch=YsBound(designatedPitch,-10.0,10.0);
			designatedPitch=YsDegToRad(designatedPitch);

			diffPitch=(designatedPitch-att->p());

			air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.8);

			double bnk;
			switch(air.Prop().GetAirplaneCategory())
			{
			default:
				bnk=YsDegToRad(45.0);
				break;
			case FSAC_FIGHTER:
			case FSAC_TRAINER:
			case FSAC_ATTACKER:
			case FSAC_AEROBATIC:
				bnk=YsDegToRad(60.0);
				break;
			}
			bnk=YsBound(bnk,-bnkLimit,bnkLimit);
			air.Prop().BankController(bnk);

			double gNeeded,gVerticalComponent;

			if(YsAbs(att->b())<YsDegToRad(80.0) || YsAbs(att->b())>YsDegToRad(110.0))
			{
				gVerticalComponent=1.0+diffPitch/YsDegToRad(10.0);
			}
			else
			{
				gVerticalComponent=1.0;
			}
			gVerticalComponent=YsBound(gVerticalComponent,-2.0,6.0);

			if(YsAbs(att->b())<YsDegToRad(80.0))
			{
				gNeeded=YsBound(gVerticalComponent/cos(YsAbs(att->b())),-2.0,6.0);
			}
			else
			{
				gNeeded=6.0;
			}

			//if(att->p()>designatedPitch)
			//{
			//	gNeeded-=(YsAbs(att->b())<YsPi/2.0 ? 0.5 :-0.5);
			//}
			//else if(att->p()<designatedPitch)
			//{
			//	gNeeded+=(YsAbs(att->b())<YsPi/2.0 ? 0.5 :-0.5);
			//}
			air.Prop().GController(gNeeded);
		}
	}
	return YSOK;
}

YSRESULT FsAutopilot::ControlGForAltitude
    (FsAirplane &air,FsSimulation *sim,const double &alt,const double &climbRate,const double &GLimit)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	YsVec3 vel;
	double dy,vy,vyReq,vh,vyLimit;

	air.Prop().GetVelocity(vel);

	vh=sqrt(vel.x()*vel.x()+vel.z()*vel.z());


	// Calculation of the maximum climb angle = acos(G/LiftMax(v))
	double G,Lmax,Amax,maxClimbAngle,loadMax;
	Amax=air.Prop().GetCriticalAOA()*0.8;
	Lmax=air.Prop().GetLift(Amax,air.Prop().GetVelocity());
	G=air.Prop().GetTotalWeight()*FsGravityConst;
	Lmax=Lmax*cos(att.b());

	loadMax=Lmax/G;

	// Commented Out ????/??/?? There is no guarantee that this is a steady state.  Lmax may be greater than G.
	// if(YsTolerance<Lmax && G<Lmax)
	// {
	// 	maxClimbAngle=acos(G/Lmax);
	// }
	// else
	// {
	// 	maxClimbAngle=0.0;
	// }

	// Commented Out 2014/06/17 Cutting off at loadMax=2.0 was too restrictive.
	// loadMax==1.0 means there is no margin for climb.
	// Being able to make 30 degree bank turn means that loadMax is at least 1.15.
	// if(loadMax<2.0)
	// {
	// 	maxClimbAngle=0.0;
	// }
	// else
	// {
	// 	maxClimbAngle=(YsPi/18.0)*(loadMax-2.0);
	// }

	// 2014/06/17 Say loadMax==2.0 makes 1 degree climb angle.
	if(loadMax<1.0)
	{
		maxClimbAngle=0.0;
	}
	else if(loadMax<2.0)
	{
		maxClimbAngle=(YsPi/180.0)*(loadMax-1.0);
	}
	else
	{
		maxClimbAngle=(YsPi/180.0)+(YsPi/18.0)*(loadMax-2.0);
	}

	if(&air==sim->GetPlayerObject())
	{
		// printf("%s ClimbAngle %lf\n",air.GetIdentifier(),YsRadToDeg(maxClimbAngle));
		// printf("loadMax %lf\n",loadMax);
	}

	if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
	   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
	   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER)
	{
		maxClimbAngle=YsSmaller(maxClimbAngle,YsPi/18.0);
	}
	else
	{
		maxClimbAngle=YsSmaller(maxClimbAngle,YsPi/36.0);
	}

	vyLimit=vh*tan(maxClimbAngle);


	const double descendAngle=(YSTRUE==air.Prop().GetHasSpoiler() ? YsPi/18.0 : YsPi/36.0);  // 10deg or 5deg.


	dy=alt-pos.y();
	vy=vel.y();

	vyReq=climbRate+dy/10.0;
	vyReq=YsBound(vyReq,-vh*tan(descendAngle),vyLimit);

	if(fabs(att.b())<YsDegToRad(89.0))
	{
		double Gvertical,Greq;
		Gvertical=1.0+(vyReq-vy)/10.0;
		Greq=Gvertical/cos(att.b());
		Greq=YsBound(Greq,0.5,GLimit);

		if(Greq>air.Prop().GetG()+1.0)  // Test 2004/11/17
		{
			Greq=air.Prop().GetG()+1.0;
		}
		air.Prop().GController(Greq);
	}
	else
	{
		air.Prop().GController(1.0);
	}
	return YSOK;
}

YSRESULT FsAutopilot::ControlGForVerticalSpeed(FsAirplane &air,FsSimulation *,const double &vAlt,const double &GLimit)
{
	const YsAtt3 &att=air.GetAttitude();
	double vy,dvy,c;
	YsVec3 vel;

	air.Prop().GetVelocity(vel);
	vy=vel.y();
	dvy=vAlt-vy;

	c=cos(att.b());
	if(fabs(c)>YsTolerance)
	{
		double Gvertical,Greq;
		Gvertical=1.0+dvy/10.0;
		Greq=Gvertical/c;
		Greq=YsBound(Greq,-1.5,GLimit);
		if(Greq>air.Prop().GetG()+1.0)  // Test 2004/11/17
		{
			Greq=air.Prop().GetG()+1.0;
		}
		air.Prop().GController(Greq);
	}
	else
	{
		air.Prop().GController(1.0);
	}
	return YSOK;
}

YSRESULT FsAutopilot::ControlGForRollOut
	   (FsAirplane &air,FsSimulation *,const double &alt,
	    const double & /*maxClimbAngle*/,const double &maxDescendAngle,
	    const double &maxGLimit,const double &minGLimit)
{
	YsAtt3 vAtt;
	YsVec3 uv,vel;
	double gMax,gMin,v,w,rad;  // w:Rotational velocity

	uv=air.GetAttitude().GetUpVector();
	air.Prop().GetVelocity(vel);
	vAtt.SetTwoVector(vel,uv);

	v=air.Prop().GetVelocity();


	gMax=maxGLimit*cos(air.GetAttitude().b());
	gMin=minGLimit*cos(air.GetAttitude().b());

	if(alt<air.GetPosition().y())   // Descending to the designated altitude
	{
		// (1) Test maxDescendAngle & gMax;
		if(v>YsTolerance)
		{
			w=gMax*FsGravityConst/v;

			double altLoss;  // Altitude loss until zero pitch.
			rad=v/w;
			altLoss=rad-rad*cos(maxDescendAngle);

			if(air.GetPosition().y()-altLoss<alt)
			{
				air.Prop().GController(maxGLimit);
			}
			else
			{
				double g;
				g=1.0-0.1*YsRadToDeg(vAtt.p()-maxDescendAngle);
				g=YsBound(g,minGLimit,maxGLimit);
				air.Prop().GController(g);
			}
		}
	}
	else
	{
// Climbing portion has not be written yet.
	}
	return YSOK;
}

void FsAutopilot::ShallowPursuit
   (FsAirplane &air,FsSimulation *sim,
    const YsVec3 &targetPos,const YsAtt3 & /*targetAtt*/,const YsVec3 &targetVelocity,const double & /*targetG*/,
    const double &dt,const double &maxG,const double & /*minG*/,
    YSBOOL /*aimingGun*/ )   // this returns deviation in angle relative to the target
{
	YSBOOL flyingAway;
	YsVec3 relVel,relTargPos;
	YsAtt3 flatAtt;
	double speed;

	flatAtt=air.GetAttitude();
	flatAtt.SetP(0.0);
	flatAtt.SetB(0.0);

	air.Prop().GetVelocity(relVel);
	relVel-=targetVelocity;
	flatAtt.MulInverse(relTargPos,targetPos-air.Prop().GetPosition());
	if(relTargPos.z()<0.0)
	{
		flyingAway=YSTRUE;
		if(air.Prop().IsJet()==YSTRUE)
		{
			speed=FsGetMachOne(air.GetPosition().y())*0.6;
		}
		else
		{
			speed=200.0*(1800.0/3600.0);
		}
	}
	else
	{
		double tVel,spd1,spd2;
		flyingAway=YSFALSE;

		tVel=targetVelocity.GetLength();
		spd1=tVel*0.8;
		spd2=tVel+10.0;
		if(relTargPos.z()<80.0)
		{
			speed=spd1;
		}
		else if(relTargPos.z()>200.0)
		{
			speed=spd2;
		}
		else
		{
			double t;
			t=(relTargPos.z()-80.0)/120.0;
			speed=spd1*(1.0-t)+spd2*t;
		}
	}
	air.Prop().SpeedController(speed);



	double desigBank,radial;
	radial=atan2(-relTargPos.x(),relTargPos.z());

	if(shallowPursuitStateChangeTimer<=0.0)
	{
		int prevState;
		prevState=shallowPursuitState;
		if(fabs(radial)<YsPi/6.0)
		{
			shallowPursuitState=0;
		}
		else
		{
			shallowPursuitState=1;
		}
		if(prevState!=shallowPursuitState)
		{
			shallowPursuitStateChangeTimer=2.0;
		}
	}
	else
	{
		shallowPursuitStateChangeTimer-=dt;
	}

	if(shallowPursuitState==0)
	{
		// 30 degree off -> 60 degree bank
		// 10 degree off -> 5 degree bank
		//  0 degree off -> 0 degree bank
		if(radial<-YsPi/6.0)
		{
			desigBank=-YsPi/3.0;
		}
		else if(YsPi/6.0<radial)
		{
			desigBank=YsPi/3.0;
		}
		else if(radial<-YsPi/18.0)
		{
			desigBank=-YsPi/36.0+(radial+YsPi/18.0)*(55.0/20.0);
		}
		else if(YsPi/18.0<radial)
		{
			desigBank=YsPi/36.0+(radial-YsPi/18.0)*(55.0/20.0);
		}
		else
		{
			desigBank=radial/2.0;
		}

		switch(air.Prop().GetAirplaneCategory())
		{
		default:
			break;
		case FSAC_WW2FIGHTER:
		case FSAC_AEROBATIC:
		case FSAC_FIGHTER:
			desigBank=YsBound(desigBank,-YsPi*8.0/18.0,YsPi*8.0/18.0);
			break;

		case FSAC_TRAINER:
		case FSAC_ATTACKER:
			desigBank=YsBound(desigBank,-YsPi/3.0,YsPi/3.0);
			break;

		case FSAC_NORMAL:
		case FSAC_UTILITY:
		case FSAC_HEAVYBOMBER:
		case FSAC_WW2BOMBER:
			desigBank=YsBound(desigBank,-YsPi/6.0,YsPi/6.0);
			break;
		}
		air.Prop().BankController(desigBank);

		double bank,relAlt,climb;
		bank=air.GetAttitude().b();
		relAlt=targetPos.y()-air.GetPosition().y();
		climb=relAlt/30.0; // 30 seconds to reach the altitude
		ControlGForAltitude(air,sim,targetPos.y(),climb,maxG);
	}
	else
	{
		YsVec3 vel;
		double oneGBank,desigBank,relAlt,r,err;
		oneGBank=acos(1.0/maxG);

		relAlt=targetPos.y()-air.GetPosition().y();
		r=1.0-relAlt*0.2/100.0;
		r=YsBound(r,0.9,1.1);

		if(air.Prop().GetVelocity()<100.0 && r>1.0)  // Gain Speed
		{
			r=1.0;
		}
		air.Prop().GetVelocity(vel);
		if(vel.y()>800.0/3 && r>1.0)
		{
			r=1.0;
		}

		desigBank=oneGBank*r;
		if(radial<0.0)
		{
			desigBank*=-1.0;
		}


		switch(air.Prop().GetAirplaneCategory())
		{
		default:
			break;
		case FSAC_WW2FIGHTER:
		case FSAC_AEROBATIC:
		case FSAC_FIGHTER:
			desigBank=YsBound(desigBank,-YsPi*8.0/18.0,YsPi*8.0/18.0);
			break;

		case FSAC_TRAINER:
		case FSAC_ATTACKER:
			desigBank=YsBound(desigBank,-YsPi/3.0,YsPi/3.0);
			break;

		case FSAC_NORMAL:
		case FSAC_UTILITY:
		case FSAC_HEAVYBOMBER:
		case FSAC_WW2BOMBER:
			desigBank=YsBound(desigBank,-YsPi/6.0,YsPi/6.0);
			break;
		}


		air.Prop().BankController(desigBank);

		err=air.GetAttitude().b()/desigBank;
		if(0.8<err && err<1.2)
		{
			air.Prop().GController(YsSmaller(air.Prop().GetG()+1.5,maxG));
		}
		else
		{
			ControlGForAltitude(air,sim,air.GetPosition().y(),0.0,maxG);
		}
	}
}

double FsAutopilot::FollowTarget
    (FsAirplane &air,FsSimulation *,
     const YsVec3 &targetPresent,const YsAtt3 &targetAtt,const YsVec3 &targetVelocity,const double &targetG,
     const double &dt,const double &maxG,const double &minG,
     YSBOOL aimingGun)
{
	YsVec3 rel,aprVel;
	YsAtt3 airAtt,targetAttVel;
	double ssa;
	YsVec3 airVel;
	YsVec3 airEv,trgEv;
	YsVec3 target;
	YsVec3 gravity;
	double radarAngle;
	double vSpeed;

	const double hDist=(targetPresent-air.GetPosition()).GetLengthXZ();

	// printf("[%s]\n",air.Prop().GetIdentifier());

	target=targetPresent;  // <- Make correction to target for gun aiming.

	if(targetVelocity.GetSquareLength()>YsSqr(YsTolerance))
	{
		YsVec3 uv;
		uv=targetAtt.GetUpVector();
		targetAttVel.SetTwoVector(targetVelocity,uv);
	}
	else
	{
		targetAttVel=targetAtt;
	}

	airAtt=air.GetAttitude();
	air.Prop().GetVelocity(airVel);
	vSpeed=airVel.y();
	airAtt.MulInverse(airVel,airVel);
	ssa=atan2(-airVel.x(),airVel.z());
	airAtt.YawLeft(ssa);


	rel=target-air.GetPosition();
	airAtt.MulInverse(rel,rel);
	airAtt.MulInverse(aprVel,targetVelocity);


	if(aimingGun==YSTRUE && 0.0<rel.z() && rel.z()<700.0)  // Adjust for gun
	{
		YsVec3 gun,dir;
		air.Prop().GetGunPosition(gun,dir);
		dir.Normalize();
		rel-=gun;

		double tEst;
		tEst=rel.z()/(air.Prop().GetBulletSpeed()-aprVel.z());

		// Say, the bullet will hit the target tEst seconds later.
		if(tEst>0.0)
		{
			YsVec3 acc;   // Acceleration : Assuming that the airplane is behind the target, and pulling at the same G
			YsVec3 tDsp;  // Target displacement
			YsVec3 bDrp;  // Bullet drop

			air.Prop().GetAcceleration(acc);

			tDsp=targetVelocity*tEst+0.5*acc*tEst*tEst;
			bDrp.Set(0.0,0.5*FsGravityConst*tEst*tEst,0.0);

			// Here's the correction for gun aiming.
			target+=tDsp+bDrp;
			airAtt.MulInverse(rel,target-air.GetPosition());
		}
	}



	if(YsAbs(rel.z())<YsTolerance)
	{
		radarAngle=YsPi/2.0;
	}
	else if(YsAbs(rel.x())>YsAbs(rel.y()))
	{
		radarAngle=YsAbs(rel.x())/YsAbs(rel.z());
	}
	else
	{
		radarAngle=YsAbs(rel.y())/YsAbs(rel.z());
	}



	// Recover from negative G flight
	if(rel.z()>YsGreater(YsAbs(rel.x()),YsAbs(rel.y()))*11.4 &&   // 5 degree frustum
	   air.Prop().GetG()<-0.5)
	{
		tInverted+=dt;
		if(tInverted>=5.0)
		{
			tRecoveryFromInverted=2.0;
			refUvForRecoveryFromInverted=-air.GetAttitude().GetUpVector();
		}
	}
	else
	{
		tInverted=0.0;
	}
	if(tRecoveryFromInverted>0.0)
	{
		tRecoveryFromInverted-=dt;
		air.Prop().BankController(refUvForRecoveryFromInverted);
		air.Prop().GController(0.0);

		YsVec3 uv;
		uv=air.GetAttitude().GetUpVector();
		if(uv*refUvForRecoveryFromInverted>0.707)
		{
			tRecoveryFromInverted=0.0;  // OK, it recovered.
		}
		return radarAngle;
	}



	// Bank Control
	int mode;
	air.Prop().GetVelocity(airEv);
	trgEv=targetVelocity;
	airEv.Normalize();
	if(trgEv.Normalize()!=YSOK)
	{
		trgEv=targetAtt.GetForwardVector();
	}



	// See 20141025-FollowTargetMode1Bias.jnt
	// This bias tightens lateral frustum tolerance so that the airplane is less likely to go too low.
	double negaBias=0.0,posiBias=0.0;
	const double biasCb=YsBound(fabs(air.GetAttitude().b())/YsDegToRad(80.0),0.0,1.0);
	const double biasCd=YsBound((hDist-1000.0)/1000.0,0.0,1.0);
	const double biasCc=YsBound((YsDegToRad(1.0)-air.GetAttitude().p())/YsDegToRad(1.0),0.0,1.0);
	const double biasC=biasCb*biasCd*biasCc;
	if(0.0<air.GetAttitude().b())
	{
		posiBias=biasC;
	}
	else
	{
		negaBias=biasC;
	}

	const double negaTan=1.0-0.7*negaBias;
	const double posiTan=1.0-0.7*posiBias;

	double bnkErr;
	// The target is in front and the velocity is almost parallel.
	// The first three condition used to be rel.z()>YsGreater(fabs(rel.x()),fabs(rel.y())
	if(fabs(rel.y())<rel.z() &&     // This condition guarantees that rel.z() is positive
	   -rel.z()*negaTan<rel.x() &&  // Frustum limit to the negative X direction with bias taken into account.
	    rel.x()<rel.z()*posiTan &&  // Frustum limit to the positive X direction with bias taken into account.
	   (aprVel.z()>YsGreater(fabs(aprVel.x()),fabs(aprVel.y())) ||     // 45 degree frustum
	    aprVel.z()<-YsGreater(fabs(aprVel.x()),fabs(aprVel.y()))*2.0)) // 30 degree frustum (negative)
	// Note   FollowTarget(1)
	//   This frustum must be larger than the frustum for the G controller to
	//   start final correction.  It is easier to adjust G controller for a given
	//   bank than adjusting bank for a given G.
	{
		double yaw,rud,dYaw;

		yaw=atan2(-rel.x(),rel.z());

		dYaw=(yaw-prevYaw)/dt;
		prevYaw=yaw;

		double relbank;
		YsVec3 tev,tuv;

		tev=targetAtt.GetForwardVector();
		tuv=targetAtt.GetUpVector();

		air.GetAttitude().MulInverse(tev,tev);
		air.GetAttitude().MulInverse(tuv,tuv);

		YsVec3 nom;
		nom=tev^tuv;
		nom.Normalize();

		YsPlane pln(YsVec3(0.0,0.0,0.0),nom);
		YsPlane xy(YsVec3(0.0,0.0,0.0),YsVec3(0.0,0.0,1.0));
		YsVec3 lnOrg,lnVec;
		if(YsGetTwoPlaneCrossLine(lnOrg,lnVec,pln,xy)==YSOK)
		{
			relbank=atan(-lnVec.x()/lnVec.y());
		}
		else
		{
			relbank=0.0;
		}


		bnkErr=yaw*3.0+dYaw*5.0;
		bnkErr=YsBound(bnkErr,-YsPi/4.0,YsPi/4.0);

		if(air.Prop().GetG()<0.0)
		{
			bnkErr=-bnkErr;
		}

		bnkErr+=relbank;
		bnkErr=YsBound(bnkErr,-YsPi*2.0/3.0,YsPi*2.0/3.0);

		air.Prop().BankController(air.GetAttitude().b()+bnkErr*1.2); // 1.2: Acceleration

		rud=yaw/YsDegToRad(5.0);
		rud=YsBound(rud,-1.0,1.0);
		air.Prop().SetRudder(rud);

		mode=1;
	}
	else if(rel.z()>0.0 && 
	        ((fabs(rel.x())<rel.z()*0.0875 && fabs(rel.y())<rel.z()*0.0875) || // 0.0875=tan(5deg) 0.2679=tan(15deg)
	         (-rel.z()*0.2679<rel.y() && rel.y()<rel.z()*0.0875 && aprVel.y()>0.0)))
    // 2006/03/13
    //   The condition YsAbs(rel.y())<rel.z()*0.0875 has been changed to
    //   -rel.z()*0.2679<rel.y() && rel.y()<rel.z()*0.0875
    //   If it overshoots the target, rel.y() may be less than rel.z()*tan(-5deg)
    //   in such a case, the previous condition doesn't match, and this if block
    //   may fall to the last condition.  It may end up with trembling the airplane.
	{
		// When the target is nearly dead head on, I cannot compute bnkErr as bnkErr=atan2(-rel.x(),rel.y),
		// because it's nearly computing bank angle near north/south poles.  So, it must be dealt as
		// an exception.
		YsVec3 uvToAlign,tEv,tUv,nomRel;

		// naprVel=aprVel;          // Align air's up vector to either target's velocity or target's up vector.
		// naprVel.Normalize();

		air.GetAttitude().MulInverse(tUv,targetAtt.GetUpVector());
		air.GetAttitude().MulInverse(tEv,targetAtt.GetForwardVector());
		if(targetG<0.0)
		{
			tUv=-tUv;
		}

		if(YsSqr(tEv.x())+YsSqr(tEv.y())>YsSqr(tUv.x())+YsSqr(tUv.y()))
		{
			uvToAlign=tEv;
		}
		else
		{
			uvToAlign=tUv;
		}


		// Bank limit 2011/01/16 >>
		double bnkLimit=YsPi;
		if(air.GetPosition().y()<targetPresent.y() && 0.0>vSpeed)
		{
			double delta=-vSpeed+(targetPresent.y()-air.GetPosition().y());
			bnkLimit=YsPi/2.0-(YsPi/3.0)*(delta/100.0);
			if(YsPi/8.0>bnkLimit)
			{
				bnkLimit=YsPi/8.0;
			}
		}
		// Bank limit 2011/01/16  <<


		bnkErr=atan2(-uvToAlign.x(),uvToAlign.y());
		bnkErr=YsBound(bnkErr,-YsPi*2.0/3.0,YsPi*2.0/3.0);


		// Take lateral deviation into account 2011/01/16 >>
		const double lateral=YsBound(3.0*atan(-rel.x()/rel.z()),-YsPi/4.0,YsPi/4.0);
		bnkErr+=lateral;
		// Take lateral deviation into account 2011/01/16 <<


		double GSign,bnkOfst;
		GSign=(air.Prop().GetG()>0.0 ? 1.0 : -1.0);
		bnkOfst=-(rel.x()/rel.z())*(40.0*YsPi/180.0);
		bnkOfst=YsBound(bnkOfst,-YsPi/18,YsPi/18);

		bnkErr+=bnkOfst*GSign;

		double reqBank=air.GetAttitude().b()+bnkErr;
		reqBank=YsBound(reqBank,-bnkLimit,bnkLimit);

		air.Prop().BankController(reqBank);

		mode=2;
	}
	else
	{
		bnkErr=atan2(-rel.x(),rel.y());
		bnkErr=YsBound(bnkErr,-YsPi*2.0/3.0,YsPi*2.0/3.0);
		air.Prop().BankController(air.GetAttitude().b()+bnkErr*1.2);  // 1.2: Acceleration

		mode=3;
	}

	if(rel.z()>0.0 && air.Prop().IsOnGround()!=YSTRUE)
	{
		double ssa;
		ssa=atan2(rel.x(),rel.z());
		air.Prop().ConfigureRudderBySSA(ssa);
	}
	else
	{
		air.Prop().SetRudder(0.0);
	}
	air.Prop().SmartRudder(dt);





	// G Control
	double Greq;
	double Greq1;  // G based on this airplane's location relative to the target
	double Greq2;  // G based on the target's location relative to this airplane.


	// Based on this airplane's location relative to the target.
	YsVec3 relAirEv,relAirUv,relAirPos;
	double theataL,dTheataL;
	targetAttVel.MulInverse(relAirPos,air.GetPosition()-target);
	targetAttVel.MulInverse(relAirEv,airEv);
	targetAttVel.MulInverse(relAirUv,air.GetAttitude().GetUpVector());
	theataL=atan2(-relAirPos.y(),YsGreater(-relAirPos.z(),50.0));
	//                           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^Cheat

	dTheataL=(theataL-prevTheataL)/dt;
	iTheataL+=theataL*dt;
	iTheataL=YsBound(iTheataL,-YsDegToRad(3.0),YsDegToRad(3.0));
	prevTheataL=theataL;
	// 1/Kp_g [deg] difference will add 1G
	// 1/Kd_g [deg/sec] increase of the difference will add 1G
	// integral(diff)/Ki_g will add 1G
	const double Kp_g1=0.33333,Kd_g1=0.33333,Ki_g1=0.33333;
	Greq1=YsRadToDeg(Kp_g1*theataL+Kd_g1*dTheataL+Ki_g1*iTheataL);
	if(aimingGun!=YSTRUE) // Spice
	{
		Greq1-=relAirEv.y()*10.0;
	}
	if(relAirUv.y()<0.0)
	{
		Greq1=-Greq1;
	}



	// Based on the target location relative to this airplane.
	double theataK,dTheataK;
	theataK=atan2(rel.y(),rel.z());
	dTheataK=(theataK-prevTheataK)/dt;
	iTheataK+=theataK*dt;
	if(aimingGun==YSTRUE)
	{
		iTheataK=YsBound(iTheataK,-YsDegToRad(30.0),YsDegToRad(30.0));  // Go wild!  Kill it!
		// Attempts :  +-15 was too small
		//             +-40 seems to be too big
	}
	else
	{
		iTheataK=YsBound(iTheataK,-YsDegToRad(3.0),YsDegToRad(3.0));  // Be more stable.
	}
	prevTheataK=theataK;
	// 1/Kp_g [deg] difference will add 1G
	// 1/Kd_g [deg/sec] increase of the difference will add 1G
	// integral(diff)/Ki_g will add 1G
	const double Kp_g2=0.33333,Kd_g2=0.33333,Ki_g2=0.33333;
	Greq2=YsRadToDeg(Kp_g2*theataK+Kd_g2*dTheataK+Ki_g2*iTheataK);



	// Weighing Greq1 and Greq2
	//   If atan2(aprVel.xy,aprVel.z) is near zero, Greq1 must dominate.
	//   If atan2(rel.xz,rel.z) is near zero, Greq1 must dominate.
	//   If rel.z()>=300.0 Greq2 must dominate.
	double w1;
	YsVec3 v;
	w1=1.0;
	v=aprVel;
	if(v.Normalize()==YSOK)
	{
		// w1*=YsBound(((v.z()-0.5)/0.36025),0.0,1.0);  // 60deg->0.0   30deg->1.0
		w1*=YsBound((v.z()-0.86025)/(0.939693-0.86025),0.0,1.0);  // 30deg->0.0  20deg->1.0
		//  See Note "FollowTarget(1)"
	}
	else
	{
		w1=0.0;
	}

	v=rel;
	if(v.Normalize()==YSOK)
	{
		// w1*=YsBound(((v.z()-0.5)/0.36025),0.0,1.0);  // 60deg->0.0   30deg->1.0
		w1*=YsBound((v.z()-0.86025)/(0.939693-0.86025),0.0,1.0);  // 30deg->0.0  20deg->1.0
		//  See Note "FollowTarget(1)"
	}
	else
	{
		w1=0.0;
	}

	if(rel.z()>=300.0)
	{
		w1*=YsBound((600.0-rel.z())/300.0,0.0,1.0);
	}

	w1*=YsBound(2.0-targetG,0.0,1.0);
	w1=YsBound(w1,0.0,1.0);
	Greq=w1*Greq1+(1.0-w1)*Greq2;

	//printf("% 8.2lf % 8.2lf % 8.2lf ",YsRadToDeg(theataL),YsRadToDeg(dTheataL),YsRadToDeg(iTheataL));
	//printf("% 8.2lf % 8.2lf % 8.2lf ",YsRadToDeg(theataK),YsRadToDeg(dTheataK),YsRadToDeg(iTheataK));
	//printf("% 8.2lf\n",w1);


	gravity=YsYVec();
	air.GetAttitude().MulInverse(gravity,gravity);
	Greq+=gravity.y();
	Greq=YsBound(Greq,minG,maxG);
	air.Prop().GController(Greq);

	return radarAngle;
}

YSBOOL FsAutopilot::NeedImmediateRecoveryFromLowAltitude(FsAirplane &air,const double &minAlt,const double gLimit)
{
	YsVec3 vel;
	air.Prop().GetVelocity(vel);
	if(vel.y()>-YsTolerance)
	{
		return YSFALSE;  // It's climbing.  Why the heck it needs recovery.
	}

	// Assume, the airplane starts recovery immediately,
	// what's the minimum altitude does this airplane reach?
	double vh,vv;
	vh=sqrt(vel.x()*vel.x()+vel.z()*vel.z());
	vv=vel.y();


	// It takes a second or so to recover from inverted position.  Take it into account.  Multiply 50% safety factor.
	const double tRecoverBank=1.5*fabs(air.GetAttitude().b()/air.Prop().GetMaxRollRate());
	const double altLossRecoverBank=vv*tRecoverBank;
	printf("tBankRecovery=%lf\n",tRecoverBank);


	double descendingAngle,angularVelocity,t,r;
	descendingAngle=atan2(-vv,vh);
	angularVelocity=(gLimit*FsGravityConst)/air.Prop().GetVelocity();   // ommega=a/v  (from high school physics)
	t=descendingAngle/angularVelocity;
	r=YsSqr(air.Prop().GetVelocity())/(gLimit*FsGravityConst);

	// Now, t is the time required for the airplane to reach the bottom of the turn,
	// and r is the radius of the turn if there's no gravity.

	const double ymin=air.GetPosition().y()+altLossRecoverBank+r*cos(descendingAngle)-r-0.5*FsGravityConst*t*t;


	// It'll also take time to recover bank.
//	double tExtra,vRoll;
//	air.Prop().GetRollRate(vRoll);
//	tExtra=YsAbs(air.GetAttitude().b())/vRoll;
//	ymin+=vel.y()*tExtra;


	if(ymin<minAlt)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSRESULT FsAutopilot::VTOLHover(FsAirplane &air,FsSimulation *,const YsVec3 &desigPos,const YsVec3 &speedPlus)
{
	if(air.Prop().GetAircraftClass()==FSCL_HELICOPTER ||
	   air.Prop().GetHasThrustVectoring()==YSTRUE)
	{
		double wingLift;
		double alt=desigPos.y();

		air.Prop().SetThrustVector(1.0);

		if(air.Prop().GetAircraftClass()!=FSCL_HELICOPTER)
		{
			double v,aoa;
			v=air.Prop().GetVelocity();
			aoa=air.Prop().GetAOA();
			wingLift=air.Prop().GetLift(aoa,v);
			wingLift*=cos(air.Prop().GetAttitude().p())*cos(air.Prop().GetAttitude().b());
		}
		else
		{
			wingLift=0.0;
		}

		YsVec3 thrDir;
		double zeroP,zeroB;
		air.Prop().GetThrustDirection(thrDir);
		zeroP= asin(thrDir.z());
		zeroB=-asin(thrDir.x());

		YsVec3 vel,pos;
		YsAtt3 att;
		air.Prop().GetVelocity(vel);
		att=air.Prop().GetAttitude();
		att.SetP(0.0);
		att.SetB(0.0);
		att.MulInverse(vel,vel);
		vel.SetY(0.0);
		pos=desigPos-air.Prop().GetPosition();
		att.MulInverse(pos,pos);


		double speed,speedLimit,dist;
		dist=pos.GetLength();
		speedLimit=air.Prop().GetMinimumManeuvableSpeed()*0.9;
		if(dist<5.0)
		{
			speed=1.0*dist/5.0;
		}
		if(dist<20.0)
		{
			speed=1.0+4.0*((dist-5.0)/15.0);   // 20m->10kt
		}
		else if(dist<100.0)
		{
			speed=5.0+(speedLimit-5.0)*((dist-20.0)/80.0);
		}
		else
		{
			speed=speedLimit;
		}

		YsVec3 course,offset,plus;
		course=pos;
		if(course.Normalize()==YSOK)
		{
			course*=speed;
		}
		att.MulInverse(plus,speedPlus);
		course+=plus;
		if(course.GetSquareLength()>YsSqr(speedLimit))  // This may take place due to speedPlus;
		{
			course.Normalize();
			course*=speedLimit;
		}
		offset=course-vel;

		double bnk,pch;
		bnk=YsDegToRad(-offset.x()*3.0);
		pch=YsDegToRad(-offset.z()*3.0);
		bnk=YsBound(bnk,-YsPi/12.0,YsPi/12.0);
		pch=YsBound(pch,-YsPi/18.0,YsPi/18.0);

		bnk+=zeroB;
		pch+=zeroP;
		if(air.Prop().GetAircraftClass()==FSCL_HELICOPTER ||
		   air.Prop().GetVelocity()<=air.Prop().GetMinimumManeuvableSpeed())
		{
			air.Prop().TurnOffPitchController();
			air.Prop().TurnOffBankController();

			double d;
			d=YsRadToDeg(pch-air.Prop().GetAttitude().p());
			air.Prop().SetDirectPitchControl(d*0.2);

			d=YsRadToDeg(bnk-air.Prop().GetAttitude().b());
			air.Prop().SetDirectRollControl(d*0.2);
		}
		else
		{
			air.Prop().NeutralDirectPitchControl();
			air.Prop().NeutralDirectRollControl();
			air.Prop().PitchController(pch);
			air.Prop().BankController(bnk);
		}



		double Kp=0.2;
		const double Kd=0.2;
		double Fy,Ay,Dy,Vy;
		air.Prop().GetVelocity(vel);  // Re-acquire

		Dy=alt-air.Prop().GetPosition().y();
		Vy=vel.y();

		if(Vy>=0.0)
		{
			Kp=0.2;
		}
		else if(Vy<-6.0)
		{
			Kp=0.0;
		}
		else
		{
			Kp=0.2*((6.0+Vy)/6.0);
		}

		Ay=Kp*Dy-Kd*Vy+FsGravityConst;
		Fy=air.Prop().GetTotalWeight()*Ay-wingLift;

		air.Prop().GetThrustDirection(thrDir);  // Re-aquire
		air.Prop().GetAttitude().Mul(thrDir,thrDir);

		if(thrDir.y()>YsTolerance)
		{
			Fy/=thrDir.y();
		}
		else
		{
			Fy=0.0;
		}

		if(Fy>YsTolerance)
		{
			air.Prop().SetThrottleByRequiredThrust(Fy);
		}
		else
		{
			air.Prop().SetThrottle(0.0);
		}

		air.Prop().SetDirectYawControl(0.0);
		air.Prop().SetRudder(0.0);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

void FsAutopilot::DecelerationAfterLanding(FsAirplane &air,FsSimulation *,const double & /*dt*/)
{
	air.Prop().TurnOffGController();
	air.Prop().TurnOffSpeedController();

	if(air.Prop().GetHasThrustReverser()==YSTRUE)
	{
		air.Prop().SetThrustReverser(1.0);
		air.Prop().SetThrottle(1.0);
	}
	else
	{
		air.Prop().SetThrottle(0.0);
	}
	air.Prop().SetBrake(1.0);
	air.Prop().SetSpoiler(1.0);

	double pitch,pitchByG;
	if(YSTRUE!=air.Prop().IsTailDragger())
	{
		pitch=air.Prop().GetTailStrikePitchAngle(0.9);
	}
	else
	{
		pitch=air.Prop().GetGroundStaticPitchAngle()*0.9;
	}

	// printf("TSP:% 8.2lf  ",pitch);

	pitchByG=air.Prop().ComputeAOAForRequiredG(0.9);
	// printf("PBG:% 8.2lf  ",pitchByG);
	if(pitchByG<pitch)
	{
		pitch=pitchByG;
	}

	// printf("OUT:% 8.2lf \n",pitch);

	//YsVec3 vel;
	//air.Prop().GetVelocity(vel);
	// printf("Mode9(landed): %.2lf %.2lf %.2lf %.2lf %.2lf\n",wei,YsRadToDeg(pitch),vel.x(),vel.y(),vel.z());
	air.Prop().AOAController(pitch);
}

YSBOOL FsAutopilot::DangerOfCollision(FsAirplane &air1,FsAirplane &air2) const
{
	YsVec3 rel;
	double d;

	rel=air1.GetPosition()-air2.GetPosition();
	air2.GetAttitude().MulInverse(rel,rel);
	if(rel.z()<YsTolerance || air2.Prop().GetVelocity()*2.0<rel.z())
	{
		return YSFALSE;
	}
	d=YsGreater(YsAbs(rel.x()),YsAbs(rel.y()));
	d/=rel.z();
	if(d>0.2)
	{
		return YSFALSE;
	}


	rel=air2.GetPosition()-air1.GetPosition();
	air1.GetAttitude().MulInverse(rel,rel);
	if(rel.z()<YsTolerance || air1.Prop().GetVelocity()*2.0<rel.z())
	{
		return YSFALSE;
	}
	d=YsGreater(YsAbs(rel.x()),YsAbs(rel.y()));
	d/=rel.z();
	if(d>0.2)
	{
		return YSFALSE;
	}

	return YSTRUE;
}


////////////////////////////////////////////////////////////

FsTaxiingAutopilot::FsTaxiingAutopilot()
{
	Initialize();
}

/* static */ FsTaxiingAutopilot *FsTaxiingAutopilot::Create(void)
{
	return new FsTaxiingAutopilot;
}

void FsTaxiingAutopilot::Initialize(void)
{
	mode=MODE_NORMAL;

	speedOnRunway=YsUnitConv::KTtoMPS(30.0);
	fastTaxiSpeed=YsUnitConv::KTtoMPS(20.0);
	slowTaxiSpeed=YsUnitConv::KTtoMPS(10.0);
	lastTaxiSpeed=YsUnitConv::KTtoMPS(7.0);

	holdShortOfRunway=YSFALSE;

	taxiPathIdx=0;
	taxiPath.Clear();

	canGoForward=YSTRUE;
	taxiDir=YsOrigin();
	desiredTaxiSpeed=0.0;

	samplePointArray.CleanUp();
	incursionTestResultArray.CleanUp();

	runwayCenterLineO=YsOrigin();
	runwayCenterLineV=YsOrigin();
	runwayHeading=0.0;
	runwayAlignmentTimer=0.0;
}

YSRESULT FsTaxiingAutopilot::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"TAXIING_\n");

	fprintf(fp,"SPDONRWY %.2lfm/s",speedOnRunway);
	fprintf(fp,"FASTTAXI %.2lfm/s",fastTaxiSpeed);
	fprintf(fp,"SLOWTAXI %.2lfm/s",slowTaxiSpeed);
	fprintf(fp,"LASTTAXI %.2lfm/s",lastTaxiSpeed);

	fprintf(fp,"HLDSHRWY %s\n",YsBoolToStr(holdShortOfRunway));

	switch(mode)
	{
	default:
		break;
	case MODE_NORMAL:
		fprintf(fp,"TAXIMODE NORMAL\n");
		break;
	case MODE_TAKEOFF:
		fprintf(fp,"TAXIMODE TAKEOFF\n");
		break;
	}
	for(int i=0; i<taxiPath.GetN(); ++i)
	{
		fprintf(fp,"TAXIPATH %.2lfm %.2lfm %.2lfm\n",taxiPath[i].x(),taxiPath[i].y(),taxiPath[i].z());
	}

	fprintf(fp,"TAXIINDX %d\n",taxiPathIdx);

	return YSOK;
}

YSRESULT FsTaxiingAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
{
	YsString str=firstLine;
	YsArray <YsString,16> args;
	for(;;)
	{
		str.Arguments(args);

		if(0==args.GetN())
		{
			continue;
		}

		YSBOOL endIntention;
		if(YSOK==ReadCommonProperty(endIntention,args.GetN(),args))
		{
			if(YSTRUE==endIntention)
			{
				return YSOK;
			}
		}
		else
		{
			if(0==strcmp(args[0],"FASTTAXI"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				SetFastTaxiSpeed(spd);
			}
			else if(0==strcmp(args[0],"SLOWTAXI"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				SetSlowTaxiSpeed(spd);
			}
			else if(0==strcmp(args[0],"LASTTAXI"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				SetLastTaxiSpeed(spd);
			}
			else if(0==strcmp(args[0],"TAXIING_"))
			{
			}
			else if(0==strcmp(args[0],"TAXIMODE"))
			{
				if(0==strcmp("NORMAL",args[1]) || 0==strcmp("normal",args[1]) || 0==strcmp("Normal",args[1]))
				{
					SetMode(FsTaxiingAutopilot::MODE_NORMAL);
				}
				else if(0==strcmp("TAKEOFF",args[1]) || 0==strcmp("takeoff",args[1]) || 0==strcmp("Takeoff",args[1]) || 0==strcmp("TakeOff",args[1]))
				{
					SetMode(FsTaxiingAutopilot::MODE_TAKEOFF);
				}
			}
			else if(0==strcmp(args[0],"TAXIPATH"))
			{
				YsVec3 pos;
				FsGetVec3(pos,args.GetN()-1,args.GetArray()+1);
				AddTaxiPathPoint(pos);
			}
			else if(0==strcmp(args[0],"TAXIINDX"))
			{
				SetTaxiPathIndex(atoi(args[1]));
			}
			else if(0==strcmp(args[0],"SPDONRWY"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				SetSpeedOnRunway(spd);
			}
			else if(0==strcmp(args[0],"HLDSHRWY"))
			{
				holdShortOfRunway=YsStrToBool(args[1]);
			}
			else
			{
				static YSBOOL first=YSTRUE;
				if(YSTRUE==first)
				{
					first=YSFALSE;
					fsStderr.Printf(
					    "Error %s %d\nUnrecognized Token %s"
					    ,__FUNCTION__,__LINE__,args[0].Txt());
				}
			}
		}

		if(NULL==inStream.Gets(str))
		{
			break;
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

void FsTaxiingAutopilot::SetMode(TAXIINGMODE mode)
{
	this->mode=mode;
}

FsTaxiingAutopilot::TAXIINGMODE FsTaxiingAutopilot::GetMode(void) const
{
	return this->mode;
}

void FsTaxiingAutopilot::SetTaxiPath(int nPnt,const YsVec3 pnt[])
{
	taxiPath.Set(nPnt,pnt);
	taxiPathIdx=0;
}

void FsTaxiingAutopilot::SetTaxiPathIndex(int idx)
{
	taxiPathIdx=idx;
}

void FsTaxiingAutopilot::AddTaxiPathPoint(const YsVec3 &pnt)
{
	taxiPath.Append(pnt);
}

void FsTaxiingAutopilot::SetFastTaxiSpeed(const double spd)
{
	fastTaxiSpeed=spd;
}

void FsTaxiingAutopilot::SetSpeedOnRunway(const double spd)
{
	speedOnRunway=spd;
}

void FsTaxiingAutopilot::SetSlowTaxiSpeed(const double spd)
{
	slowTaxiSpeed=spd;
}

void FsTaxiingAutopilot::SetLastTaxiSpeed(const double spd)
{
	lastTaxiSpeed=spd;
}

YSRESULT FsTaxiingAutopilot::GetRunwayCenterLine(YsVec3 &o,YsVec3 &v) const
{
	if(MODE_TAKEOFF==mode && taxiPathIdx==taxiPath.GetN()-1)
	{
		o=runwayCenterLineO;
		v=runwayCenterLineV;
		return YSOK;
	}
	return YSERR;
}

void FsTaxiingAutopilot::SetHoldShortOfRunway(YSBOOL holdShort)
{
	holdShortOfRunway=holdShort;
}

YSBOOL FsTaxiingAutopilot::IsTurningToAlignForRunwayHeading(void) const
{
	if((MODE_TAKEOFF==mode || MODE_TAKEOFF_ON_CARRIER==mode) && taxiPath.GetN()-1<=taxiPathIdx)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsTaxiingAutopilot::MakePriorityDecision(FsAirplane & /*air*/ )
{
	return YSOK;
}

YSRESULT FsTaxiingAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double & /*dt*/ )
{
	const double v=air.Prop().GetVelocity();
	// const double brakeForce=air.Prop().CalculateForceByBrake(1.0);


	YsVec3 windDriftVec;
	air.Prop().CalculateWindDrift(windDriftVec,sim->GetWeather());


	// lastTaxiSpeed-a*timeToStop=0.0

	double preciseTime,preciseDist;
	air.Prop().CalculateGroundStopDistanceAndTime(preciseDist,preciseTime,v);


	// const double timeToStop=preciseTime; // Rough time -> (YsTolerance<a ? v/a : YsInfinity);
	const double distToStop=preciseDist; // Rough dist -> (v-0.5*a*timeToStop)*timeToStop;

	const double turnSpeed=(MODE_TAKEOFF_ON_CARRIER ? 2.0 : slowTaxiSpeed);
	const double turnRadius=air.Prop().CalculateTurnRatiusOnGround(v)+5.0;  // 5.0m Saba
	const double collisionMargin=(MODE_TAKEOFF_ON_CARRIER ? 10.0 : 20.0);

	const YsVec3 &airPos=air.GetPosition();
	const YsAtt3 &airAtt=air.GetAttitude();


	YsMatrix4x4 pathTfm;
	int nCollisionExclusion=0;
	const FsExistence *collisionExclusion[1];
	if(MODE_TAKEOFF_ON_CARRIER==mode)
	{
		const FsGround *carrier=air.Prop().OnThisCarrier();
		const FsAircraftCarrierProperty *carrierProp;
		if(NULL!=carrier && 
		   NULL!=(carrierProp=carrier->Prop().GetAircraftCarrierProperty()) &&
		   YSTRUE!=carrierProp->NoAutoTaxi() &&
		   YSTRUE==carrierProp->HasCatapult())
		{
			const YsVec3 o=carrierProp->GetCatapultPos();
			const YsVec3 v=carrierProp->GetCatapultVec();

			pathTfm=carrier->Prop().GetMatrix();

			if(0==taxiPath.GetN())
			{
				const double alignLength=40.0;
				const YsVec3 lateral(-v.z(),0.0,v.x());

				const YsVec3 initial[2]=
				{
					o-v*alignLength+lateral*turnRadius*2.0,
					o-v*alignLength-lateral*turnRadius*2.0
				};
				const YsVec3 initialTfm[2]=
				{
					pathTfm*initial[0],
					pathTfm*initial[1],
				};

				int nearIdx=0;
				if((initialTfm[1]-airPos).GetSquareLength()<(initialTfm[0]-airPos).GetSquareLength())
				{
					nearIdx=1;
				}

				taxiPath.Clear();
				taxiPath.Append(initial[nearIdx]);
				taxiPath.Append(o-v*alignLength);
				taxiPath.Append(o);
			}

			runwayCenterLineO=pathTfm*o;
			pathTfm.Mul(runwayCenterLineV,v,0.0);
			runwayHeading=atan2(-runwayCenterLineV.x(),runwayCenterLineV.z());

			nCollisionExclusion=1;
			collisionExclusion[0]=carrier;
		}
	}



	if(0==taxiPath.GetN())
	{
		canGoForward=YSFALSE;
		taxiDir=YsOrigin();
		desiredTaxiSpeed=0.0;
		return YSOK;
	}



	const int nextTurnIdx=YsSmaller <int> ((int)this->taxiPath.GetN()-1,this->taxiPathIdx);
	const YsVec3 nextTurnPoint=pathTfm*this->taxiPath[nextTurnIdx];


	this->canGoForward=YSTRUE;



	if(NULL==air.IsInsideRunway())  // IsOnGround is supposed to be YSTRUE.
	{
		const FsAirTrafficSequence &trafficInfo=sim->GetAirTrafficSequence();

		YsArray <YsVec3,16> samplePnt;
		GetNextSamplePointArrayFor160m <16> (samplePnt,air,pathTfm);

		YsArray <const YsSceneryRectRegion *,16> sampleRectRgn;
		for(int sampleIdx=0; sampleIdx<samplePnt.GetN(); ++sampleIdx)
		{
			if(YSOK==sim->GetRegionRectFromPositionAll(sampleRectRgn,samplePnt[sampleIdx]))
			{
				for(int rectIdx=0; rectIdx<sampleRectRgn.GetN(); ++rectIdx)
				{
					if(FS_RGNID_RUNWAY==sampleRectRgn[rectIdx]->GetId())
					{
						if(YSTRUE==holdShortOfRunway ||
						   YSTRUE==trafficInfo.WillCauseRunwayIncursionFromTakeOffTraffic(air,sim,sampleRectRgn[rectIdx]))
						{
							canGoForward=YSFALSE;
						}
						break;
					}
				}
			}
		}
	}



	YsVec3 futurePos,futureDir,futureNom;
	GetNextSamplePointAndDirection(futurePos,futureDir,air,distToStop+collisionMargin,pathTfm);



	if(NULL!=air.Prop().OnThisCarrier()) // 2014/09/21 Potential collision was not detected on carrier.
	{
		futurePos.SetY(air.GetPosition().y());
		futureNom=air.GetAttitude().GetUpVector();
	}
	else
	{
		double elv;
		sim->GetFieldElevationAndNormal(elv,futureNom,futurePos.x(),futurePos.z());

		futurePos.SetY(elv);
		futurePos+=futureNom*air.Prop().GetGroundStandingHeight();
	}

	const YsVec3 futureVecLeft=futureDir^futureNom;
	futureDir=YsUnitVector(futureNom^futureVecLeft);

	YsAtt3 futureAtt;
	futureAtt.SetTwoVector(futureDir,futureNom);
	futureAtt.NoseUp(air.Prop().GetGroundStaticPitchAngle());

	if(YSTRUE==sim->MayCollide(futurePos,futureAtt,2.0,&air,nCollisionExclusion,collisionExclusion))
	{
		canGoForward=YSFALSE;
	}



	// This was a big mistake. Point and attitude must be sampled along the taxiing-path. >>
	// YsVec3 vecToStop(0.0,0.0,distToStop+20.0);
	// YsVec3 tst;
	// air.GetAttitude().Mul(vecToStop,vecToStop);  
	// air.Prop().GetTransformedFrontGearPosition(tst);
	// tst+=vecToStop;
	// if(YSTRUE==sim->MayCollide(tst,air.GetAttitude(),2.0,&air,nCollisionExclusion,collisionExclusion))
	// {
	// 	canGoForward=YSFALSE;
	// }
	// This was a big mistake. Point and attitude must be sampled along the taxiing-path. <<


	// air.Prop().GetTransformedFrontGearPosition(tst);
	// tst+=vel*1.0;
	// if(sim->IsOnRunway(tst)!=YSTRUE)
	// {
	// 	canGoForward=YSFALSE;
	// }

	if(this->taxiPath.GetN()>0 && this->taxiPathIdx>=this->taxiPath.GetN())
	{
		canGoForward=YSFALSE;
	}

	if(0<this->taxiPath.GetN() && this->taxiPath.GetN()-1==this->taxiPathIdx)
	{
		YsVec3 dHorizontal=nextTurnPoint-airPos;
		dHorizontal.SetY(0.0);
		const double d=dHorizontal.GetLength();
		if(distToStop>d)
		{
			canGoForward=YSFALSE;
		}
	}



	if(YSTRUE==this->taxiPath.IsInRange(this->taxiPathIdx))
	{
		this->taxiDir=nextTurnPoint-airPos;
		this->taxiDir.SetY(0.0);
		this->taxiDir.Normalize();


		// Wind Correction 2013/05/22 >>
		this->taxiDir*=v;
		this->taxiDir-=windDriftVec;
		this->taxiDir.Normalize();
		// Wind Correction 2013/05/22 <<

		YsVec3 tst=nextTurnPoint-airPos;
		tst.SetY(0.0);
		if(tst.GetSquareLength()<turnRadius*turnRadius)
		{
			this->taxiPathIdx++;
			printf("taxiPathIdx++ %d %d\n",(int)this->taxiPathIdx,(int)this->taxiPath.GetN());

			if(MODE_TAKEOFF==mode && taxiPathIdx==taxiPath.GetN()-1)
			{
				runwayCenterLineO=taxiPath[taxiPathIdx-1];
				runwayCenterLineV=YsUnitVector(taxiPath[taxiPathIdx]-taxiPath[taxiPathIdx-1]);
				runwayHeading=atan2(-runwayCenterLineV.x(),runwayCenterLineV.z());
			}
		}
	}



	// Desired Taxiing Speed: See memo/technical/20130119-deceleration.docx
	//   vx=sqrt(vs*vs+2.0*a*x)
	//      vx max speed at distance x from the turning point.
	//      vs speed at the turning point (zero if it is the last leg)
	//      a braking acceleration

	this->desiredTaxiSpeed=0.0;

	if(MODE_TAKEOFF_ON_CARRIER==mode)
	{
		desiredTaxiSpeed=2.0;
	}
	else if(YSTRUE==IsTurningToAlignForRunwayHeading())
	{
		desiredTaxiSpeed=lastTaxiSpeed;
	}
	else if(this->taxiPath.GetN()>0)
	{
		YsVec3 tst=nextTurnPoint-airPos;
		tst.SetY(0.0);
		airAtt.MulInverse(tst,tst);

		double distOffset;
		double speedAtNextTurnPoint;
		if(this->taxiPath.GetN()-1>this->taxiPathIdx)
		{
			speedAtNextTurnPoint=this->slowTaxiSpeed;
			distOffset=turnRadius;
		}
		else if(this->taxiPath.GetN()-1==this->taxiPathIdx)
		{
			const double thr1=100.0,thr2=50.0;

			if(thr1<=tst.z())
			{
				speedAtNextTurnPoint=slowTaxiSpeed;
				distOffset=thr1;
			}
			else if(thr2<=tst.z())
			{
				speedAtNextTurnPoint=lastTaxiSpeed;
				distOffset=thr2;
			}
			else
			{
				speedAtNextTurnPoint=0.0;
				distOffset=0.0;
			}
		}
		else
		{
			speedAtNextTurnPoint=0.0;
			distOffset=0.0;
		}

		if(YsTolerance>tst.z() || YsTan3deg<fabs(tst.x()/tst.z()))
		{
			this->desiredTaxiSpeed=slowTaxiSpeed;
		}
		else
		{
			const double distBeforeTurn=tst.z()-distOffset;

			double distToDescelerate,timeToDescelerate;
			air.Prop().CalculateGroundDescelerateDistanceAndTime(distToDescelerate,timeToDescelerate,v,speedAtNextTurnPoint);
			if(distBeforeTurn<distToDescelerate && speedAtNextTurnPoint<v)
			{
				this->desiredTaxiSpeed=0.0;
			}
			else if(distBeforeTurn<distToDescelerate+10)
			{
				this->desiredTaxiSpeed=speedAtNextTurnPoint;
			}
			else
			{
				if(NULL!=air.IsInsideRunway())
				{
					this->desiredTaxiSpeed=speedOnRunway;
				}
				else
				{
					this->desiredTaxiSpeed=fastTaxiSpeed;
				}
			}

			if(air.Prop().GetEstimatedLandingSpeed()*0.6<this->desiredTaxiSpeed)
			{
				this->desiredTaxiSpeed=air.Prop().GetEstimatedLandingSpeed()*0.6;
			}

			// const double d=YsSqr(speedAtNextTurnPoint)+2.0*a*distBeforeTurn;
			// const double maxSpeed=(YsTolerance<distBeforeTurn ? sqrt(d) : speedAtNextTurnPoint);
			// this->desiredTaxiSpeed=YsSmaller(fastTaxiSpeed,maxSpeed);
		}

//		if(this->taxiPathIdx==0)
//		{
//			airAtt.MulInverse(tst,tst);
//			if(tst.z()>40.0)
//			{
//				this->desiredTaxiSpeed=this->fastTaxiSpeed;
//			}
//			else
//			{
//				this->desiredTaxiSpeed=this->slowTaxiSpeed;
//			}
//		}
//		else if(this->taxiPathIdx==this->taxiPath.GetN()-1 && tst.z()<80.0)
//		{
//			this->desiredTaxiSpeed=this->lastTaxiSpeed;
//		}
//		else
//		{
//			this->desiredTaxiSpeed=this->slowTaxiSpeed;
//		}
	}
	else
	{
		this->desiredTaxiSpeed=this->fastTaxiSpeed;
	}

	return YSOK;
}

YSRESULT FsTaxiingAutopilot::ApplyControl(FsAirplane &air,FsSimulation *,const double & dt)
{
#ifdef CRASHINVESTIGATION_TAXIING
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	air.Prop().SetThrustReverser(0.0);

	air.Prop().NeutralDirectAttitudeControl();

	air.Prop().TurnOffGController();
	air.Prop().TurnOffAOAController();
	air.Prop().TurnOffPitchController();
	if(YSTRUE!=air.Prop().CanManeuverPostStall())
	{
		air.Prop().SetElevator(0.1); // Prevent tail-draggers from leaning forward
	}
	else
	{
		air.Prop().SetElevator(0.0);
		air.Prop().SetElvTrim(0.0);
	}

#ifdef CRASHINVESTIGATION_TAXIING
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	if(this->canGoForward==YSTRUE)
	{
		air.Prop().SetBrake(0.0);
		air.Prop().SetSpoiler(0.0);
		air.Prop().SetFlap(0.0);

		//if(this->taxiPath.GetN()-1>this->taxiPathIdx)
		//{
		//	air.Prop().SpeedControllerDontUseAfterburner(this->desiredTaxiSpeed,0.3);
		//	if(air.Prop().GetVelocity()>this->desiredTaxiSpeed)
		//	{
		//		air.Prop().SetBrake(1.0);
		//	}
		//	else
		//	{
		//		air.Prop().SetBrake(0.0);
		//	}
		//}
		//else  // Last leg very moderate speed control.
		{
			air.Prop().TurnOffSpeedController();
			if(air.Prop().GetVelocity()<this->desiredTaxiSpeed)
			{
				const double diff=(this->desiredTaxiSpeed-0.25-air.Prop().GetVelocity());
				air.Prop().SetBrake(0.0);
				air.Prop().SetThrottle(YsSmaller(0.1,diff));
			}
			else if(air.Prop().GetVelocity()>this->desiredTaxiSpeed)
			{
				air.Prop().SetBrake(1.0);
				air.Prop().SetThrottle(0.0);
			}
			else
			{
				air.Prop().SetBrake(0.0);
				air.Prop().SetThrottle(0.0);
			}
		}
	}
	else
	{
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.0);
		air.Prop().SetBrake(1.0);
	}

#ifdef CRASHINVESTIGATION_TAXIING
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	const double rudderLimit=1.0;

	if(YSTRUE==IsTurningToAlignForRunwayHeading())
	{
		YsVec3 airRelToCtrLine;
		airRelToCtrLine=runwayCenterLineO-air.GetPosition();
		airRelToCtrLine.RotateXZ(-runwayHeading);

		const double Kx=(MODE_TAKEOFF==mode ? 4.0 : 8.0);

		const double dx=-airRelToCtrLine.x();
		const double hdgOffset=YsDegToRad(YsBound(dx*Kx,-20.0,20.0));

		const double desiredHeading=runwayHeading+hdgOffset;

		double relHdg=fmod(desiredHeading-air.GetAttitude().h(),YsPi*2.0);

#ifdef CRASHINVESTIGATION_TAXIING
		printf("%s %d\n",__FUNCTION__,__LINE__);
		printf("%lf\n",relHdg);
#endif

		while(YsPi<relHdg)
		{
			relHdg-=YsPi*2.0;
		}

#ifdef CRASHINVESTIGATION_TAXIING
		printf("%s %d\n",__FUNCTION__,__LINE__);
		printf("%lf\n",relHdg);
#endif

		while(-YsPi>relHdg)
		{
			relHdg+=YsPi*2.0;
		}

#ifdef CRASHINVESTIGATION_TAXIING
		printf("%s %d\n",__FUNCTION__,__LINE__);
		printf("%lf\n",relHdg);
#endif

		air.Prop().SetRudder(YsBound(relHdg,-rudderLimit,rudderLimit));

		//    Runway Heading
		//          ^
		//   dx-    |    dx+
		//  relHdg+ |   relHdg-
		//          |
		//     Vel  |  Vel
		//     /    |    \
		//    /     |     \
		//  Air     |     Air
		//          |

		if(fabs(dx)<2.0 && fabs(relHdg)<0.5*YsPi/180.0)  // Fully aligned, or
		{
			runwayAlignmentTimer+=dt;
		}
		else if(-2.0<=dx && dx<0.0 && 0.0<relHdg && relHdg<YsPi/180.0) // Almost aligned from the left, or
		{
			runwayAlignmentTimer+=dt;
		}
		else if( 0.0<=dx && dx<2.0 && -YsPi/180.0<relHdg && relHdg<=0.0) // Almost aligned from the right.
		{
			runwayAlignmentTimer+=dt;
		}
		else
		{
			runwayAlignmentTimer=0.0;
		}
	}
	else if(this->taxiDir.GetSquareLength()>YsSqr(YsTolerance))
	{
		YsVec3 dir;

		double dHdg,rud;
		air.GetAttitude().MulInverse(dir,this->taxiDir);
		dHdg=atan2(-dir.x(),dir.z());
		rud=dHdg/YsDegToRad(5.0);
		rud=YsBound(rud,-rudderLimit,rudderLimit);
		air.Prop().SetRudder(rud);
	}

#ifdef CRASHINVESTIGATION_TAXIING
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	return YSOK;
}

YSBOOL FsTaxiingAutopilot::MissionAccomplished(FsAirplane &air,FsSimulation *) const
{
	if(MODE_TAKEOFF==mode && 1.0<runwayAlignmentTimer)
	{
		return YSTRUE;
	}
	if(MODE_TAKEOFF_ON_CARRIER==mode &&
	   FSGROUNDSTATIC==air.Prop().GetFlightState())
	{
		const FsGround *carrier=air.Prop().OnThisCarrier();
		const FsAircraftCarrierProperty *carrierProp;

		if(NULL!=carrier && 
		   NULL!=(carrierProp=carrier->Prop().GetAircraftCarrierProperty()) &&
		   YSTRUE==carrierProp->HasCatapult() &&
		   YSTRUE==carrierProp->IsOnCatapult(air.GetPosition()))
		{
			YsVec3 v=carrierProp->GetCatapultVec();

			auto tfm=carrier->Prop().GetMatrix();
			tfm.Mul(v,v,0.0);
			v.SetY(0.0);
			v.Normalize();

			auto fv=air.Prop().GetAttitude().GetForwardVector();
			fv.SetY(0.0);
			fv.Normalize();
			if(YsCos1deg<=v*fv)
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

// Implementation //////////////////////////////////////////
FsGotoPosition::FsGotoPosition()
{
	state=STATE_NORMAL;
	destinationIdx=0;
	destination.Clear();
	speed=150.0;
	throttle=0.0;
	useAfterburner=YSFALSE;
	straightFlightMode=YSFALSE;
	flyHeadingBugMode=YSFALSE;
	forcedTurn=0;
	radial=0.0;
	rel=YsOrigin();
}

FsGotoPosition::~FsGotoPosition()
{
}

/* static */ const char *FsGotoPosition::StateToStr(STATE state)
{
	switch(state)
	{
	default:
	case STATE_NORMAL:
		return "NORMAL";
	case STATE_TOOHIGH:
		return "TOOHIGH";
	case STATE_CAPTURE:
		return "CAPTURE";
	}
}
/* static */ FsGotoPosition::STATE FsGotoPosition::StrToState(const char *str)
{
	if(0==strcmp(str,"TOOHIGH"))
	{
		return STATE_TOOHIGH;
	}
	else if(0==strcmp(str,"CAPTURE"))
	{
		return STATE_CAPTURE;
	}
	else
	{
		return STATE_NORMAL;
	}
}

/* static */ FsGotoPosition *FsGotoPosition::Create(void)
{
	return new FsGotoPosition;
}

unsigned FsGotoPosition::OverridedControl(void)
{
	return FSAPPLYCONTROL_NAVAID|FSAPPLYCONTROL_TRIGGER|FSAPPLYCONTROL_TURRET;
}

void FsGotoPosition::SetSingleDestination(const YsVec3 &pos)
{
	destination.Set(1,NULL);
	destination[0]=pos;
	destinationIdx=0;
}

void FsGotoPosition::SetSingleDestination(const double &x,const double &y,const double &z)
{
	destination.Set(1,NULL);
	destination[0].Set(x,y,z);
	destinationIdx=0;
}

const YsVec3 &FsGotoPosition::GetNextWayPoint(void) const
{
	if(destination.IsInRange(destinationIdx)==YSTRUE)
	{
		return destination[destinationIdx];
	}
	else
	{
		return YsOrigin();
	}
}

void FsGotoPosition::SetUseAfterburner(YSBOOL useAB)
{
	useAfterburner=useAB;
}
YSBOOL FsGotoPosition::GetUseAfterburner(void) const
{
	return useAfterburner;
}

void FsGotoPosition::SetThrottle(const double thr)
{
	this->speed=0.0;
	this->throttle=thr;
}

void FsGotoPosition::SetSpeed(const double speed)
{
	this->speed=speed;
	this->throttle=0.0;
}

YSRESULT FsGotoPosition::SaveIntention(FILE *fp,const FsSimulation *)
{
	if(destination.GetN()==0)
	{
		fprintf(fp,"GOTOPOSI 0 0 0\n");
	}
	else
	{
		int i;
		forYsArray(i,destination)
		{
			fprintf(fp,"GOTOPOSI %g %g %g\n",
			    destination[i].x(),
			    destination[i].y(),
			    destination[i].z());
		}
	}
	fprintf(fp,"STATE %s\n",StateToStr(state));
	fprintf(fp,"AIRSPEED %lfm/s\n",speed); // 2009/04/17
	fprintf(fp,"THROTTLE %lf\n",throttle);
	fprintf(fp,"AFTERBURNER %s\n",YsBoolToStr(useAfterburner));
	return YSOK;
}

YSRESULT FsGotoPosition::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
{
	YsString str=firstLine;
	YsArray <YsString,16> args;
	for(;;)
	{
		str.Arguments(args);

		if(0==args.GetN())
		{
			continue;
		}

		YSBOOL endIntention;
		if(YSOK==ReadCommonProperty(endIntention,args.GetN(),args))
		{
			if(YSTRUE==endIntention)
			{
				return YSOK;
			}
		}
		else
		{
			if(0==strcmp(args[0],"GOTOPOSI"))
			{
				destination.Increment();
				destination.GetEnd().Set(atof(args[1]),atof(args[2]),atof(args[3]));
			}
			else if(0==strcmp(args[0],"AIRSPEED"))
			{
				if(2<=args.GetN() && FsGetSpeed(speed,args[1])!=YSOK)
				{
					speed=150.0;
				}
			}
			else if(0==strcmp(args[0],"THROTTLE"))
			{
				if(2<=args.GetN())
				{
					throttle=atof(args[1]);
				}
			}
			else if(0==strcmp(args[0],"AFTERBURNER"))
			{
				if(2<=args.GetN())
				{
					useAfterburner=YsStrToBool(args[1]);
				}
			}
			else if(0==strcmp(args[0],"STATE"))
			{
				if(2<=args.GetN())
				{
					state=StrToState(args[1]);
				}
			}
			else
			{
				static YSBOOL first=YSTRUE;
				if(YSTRUE==first)
				{
					first=YSFALSE;
					fsStderr.Printf(
					    "Error %s %d\nUnrecognized Token %s"
					    ,__FUNCTION__,__LINE__,args[0].Txt());
				}
			}
		}

		if(NULL==inStream.Gets(str))
		{
			break;
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

YSRESULT FsGotoPosition::MakeDecision(FsAirplane &air,FsSimulation *sim,const double & /*dt*/)
{
	const YsVec3 *cur;
	const YsAtt3 *att;

	cur=&air.GetPosition();
	att=&air.GetAttitude();

	if(STATE_CAPTURE==state)
	{
		destinationIdx=0;
		destination.resize(1);
		destination[0]=*cur;
		state=STATE_NORMAL;
	}

	if(STATE_TOOHIGH!=state &&
	   (air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
	    air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
	    air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
	    air.Prop().GetAirplaneCategory()==FSAC_TRAINER ||
	    air.Prop().GetAirplaneCategory()==FSAC_WW2FIGHTER ||
	    air.Prop().GetAirplaneCategory()==FSAC_WW2ATTACKER ||
	    air.Prop().GetAirplaneCategory()==FSAC_WW2DIVEBOMBER) &&
	    GetNextWayPoint().y()+YsUnitConv::FTtoM(1000.0)<cur->y() &&
	    YsPi/6.0<air.GetAttitude().p() &&
	    -YsUnitConv::FTtoM(-500.0/60.0)<air.Prop().GetClimbRatio())
	{
		state=STATE_TOOHIGH;
	}
	else if(GetNextWayPoint().y()+YsUnitConv::FTtoM(500.0)>cur->y() ||
	        YsDegToRad(-5.0)>air.GetAttitude().p() ||
	        -YsUnitConv::FTtoM(-700.0/60.0)>air.Prop().GetClimbRatio())
	{
		state=STATE_NORMAL;
	}

	if(state!=STATE_TOOHIGH && 
	   (air.Prop().GetAirplaneCategory()!=FSAC_AEROBATIC &&
	    air.Prop().GetAirplaneCategory()!=FSAC_FIGHTER &&
	    air.Prop().GetAirplaneCategory()!=FSAC_ATTACKER &&
	    air.Prop().GetAirplaneCategory()!=FSAC_TRAINER &&
	    air.Prop().GetAirplaneCategory()!=FSAC_WW2FIGHTER &&
	    air.Prop().GetAirplaneCategory()!=FSAC_WW2ATTACKER &&
	    air.Prop().GetAirplaneCategory()!=FSAC_WW2DIVEBOMBER) &&
	   YsDegToRad(70.0)<air.GetAttitude().p())
	{
		emr=EMR_HIGHLOWPITCH;
	}
	if(air.GetAttitude().p()<-YsDegToRad(70.0))
	{
		emr=EMR_HIGHLOWPITCH;
	}



	if(YSTRUE!=flyHeadingBugMode)
	{
		YsMatrix4x4 mat;
		YsAtt3 hdg(*att);
		hdg.SetP(0.0);
		hdg.SetB(0.0);

		mat.Translate(*cur);
		mat.Rotate(hdg);
		mat.Invert();

		rel=mat*GetNextWayPoint();
		radial=atan2(-rel.x(),rel.z());

		if(YsSqr(rel.x())+YsSqr(rel.z())<YsSqr(80.0))
		{
			destinationIdx++;
			if(destination.IsInRange(destinationIdx)!=YSTRUE)
			{
				destinationIdx=0;
			}
		}
	}
	else
	{
		auto desigHdg=air.Prop().GetHeadingBug();
		rel=YsOrigin();
		radial=desigHdg-att->h();
		while(radial<-YsPi)
		{
			radial+=YsPi*2.0;
		}
		while(YsPi<radial)
		{
			radial-=YsPi*2.0;
		}
	}


	if(forcedTurn==0 && (radial<-YsPi*175.0/180.0 || YsPi*175.0/180.0<radial))
	{
		if(radial>0.0)
		{
			forcedTurn=1;
		}
		else
		{
			forcedTurn=-1;
		}
	}

	if(-YsPi/2.0<radial && radial<YsPi/2.0)
	{
		forcedTurn=0;
	}

	return YSOK;
}

YSRESULT FsGotoPosition::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 *cur;
	const YsAtt3 *att;
	cur=&air.GetPosition();
	att=&air.GetAttitude();

	air.Prop().NeutralDirectAttitudeControl();

	if(YsTolerance<speed)
	{
		if(YSTRUE!=useAfterburner)
		{
			air.Prop().SpeedControllerDontUseAfterburner(speed,1.0);
		}
		else
		{
			air.Prop().SpeedController(speed);
		}
	}
	else
	{
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(throttle);
		air.Prop().SetAfterburner(useAfterburner);
	}

	double gLimit=3.0;

	if(STATE_NORMAL==state)
	{
		if(straightFlightMode!=YSTRUE)
		{
			double bnk;
			if(forcedTurn!=0)
			{
				bnk=YsPi*(double)forcedTurn;
			}
			else
			{
				bnk=radial*5.0;
			}

			double bankLimit=YsDegToRad(20.0);
			const double baseAlt=GetNextWayPoint().y()-YsUnitConv::FTtoM(1500.0);
			if(baseAlt<cur->y())
			{
				const double diff=cur->y()-baseAlt;
				const double correction=YsDegToRad(20.0)*diff/YsUnitConv::FTtoM(500.0);
				bankLimit+=YsSmaller <double> (correction,YsDegToRad(20.0));
			}


			if((FSAC_AEROBATIC==air.Prop().GetAirplaneCategory() ||
			    FSAC_FIGHTER==air.Prop().GetAirplaneCategory() ||
			    FSAC_ATTACKER==air.Prop().GetAirplaneCategory() ||
			    FSAC_TRAINER==air.Prop().GetAirplaneCategory()) &&
			    YsUnitConv::KTtoMPS(250.0)<air.Prop().GetVelocity())
			{
				bankLimit+=YsDegToRad(40.0)*(air.Prop().GetVelocity()-YsUnitConv::KTtoMPS(250.0))/YsUnitConv::KTtoMPS(100.0);
				if(YsDegToRad(76.0)<bankLimit)  // 4G turn (75.522488)
				{
					bankLimit=YsDegToRad(76.0);
				}
				gLimit=YsGreater(3.0,1.0+1.0/cos(bankLimit));
			}

			bnk=YsBound(bnk,-bankLimit,+bankLimit);

			if(air.Prop().GetAirplaneCategory()==FSAC_NORMAL ||
			   air.Prop().GetAirplaneCategory()==FSAC_HEAVYBOMBER ||
			   air.Prop().GetAirplaneCategory()==FSAC_WW2BOMBER)
			{
				bnk=YsBound(bnk,-YsDegToRad(30),YsDegToRad(30));
				gLimit=3.0;
			}

			air.Prop().BankController(bnk);
		}
		else
		{
			air.Prop().BankController(0.0);
		}

		air.Prop().SmartRudder(dt);

		ControlGForAltitude(air,sim,GetNextWayPoint().y(),0.0,gLimit);
	}
	else // STATE_TOOHIGH   FIGHTER, ATTACKER, TRAINER, AEROBATIC, WW2FIGHTER, WW2ATTACKER, WW2DIVEBOMBER only
	{
		const double bnkDiff=YsBound(radial*5.0,YsDegToRad(-80.0),YsDegToRad(80.0));
		const double bnk=YsPi-bnkDiff;
		air.Prop().BankController(bnk);
		air.Prop().GController(gLimit);
	}

	return YSOK;
}

YSBOOL FsGotoPosition::MissionAccomplished(FsAirplane &air,FsSimulation *) const
{
	if(destinationIdx==destination.GetN()-1)
	{
		double dx,dz;
		dx=destination.GetEnd().x()-air.GetPosition().x();
		dz=destination.GetEnd().z()-air.GetPosition().z();
	    if(dx*dx+dz*dz<100.0*100.0)  // If it comes to 80m radius, the index will be reset to zero.
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}


////////////////////////////////////////////////////////////

FsLandingAutopilot::FsLandingAutopilot()
{
	landingPhase=PHASE_FLYINGTOENTRY; // 0
	prevLandingPhase=PHASE_FLYINGTOENTRY; // 0

	approachType=APPROACH_UNDECIDED;
	ils=NULL;
	initializedAirplaneInfo=YSFALSE;
	nDrop=0;
	phaseTimer=0.0;
	landingSpeed=0.0;
	landingSpeedCorrection=1.2;

	lateralOffset=0.0;
	autoGoAround=YSTRUE;
	alwaysGoAround=YSFALSE;
	autoClearRunway=YSTRUE;
	flareAlt=15.0;
	wheelControlOnGround=YSTRUE;


	entrySpeed=0.0;
	entryTurnRadius=0.0;
	requiredRunwayLength=0.0;
	groundHeight=0.0;
	tdPos=YsOrigin();
	rwAtt=YsZeroAtt();
	bankLimit=0.0;
	turnRadius=0.0;
	turnRatio=0.0;
	trafficPatternAltitude=0.0;
	trafficPatternAltitudeAGL=0.0;
	verticalSeparation=YsUnitConv::FTtoM(500.0);
	glideSlope=0.0;

	nintyTurnTime=0.0;
	baseLegTime=0.0;
	downwindLegTime=0.0;
	finalLegTime=0.0;

	entryPoint=YsOrigin();
	entryTurnCenter=YsOrigin();
	entryTurnCenterCandidate[0]=YsOrigin();
	entryTurnCenterCandidate[1]=YsOrigin();
	entryTargetPoint=YsOrigin();
	downwindWidth=0.0;

	baseTurnStart=YsOrigin();
	baseTurnEnd=YsOrigin();
	calibratedTurnRadius=0.0;

	distToGo=0.0;
	timeToGo=0.0;
	altToGo=0.0;
	yHold=0.0;
	mode7IdealVY=0.0;

	aimingPoint=YsOrigin();
	aimingDir=YsOrigin();

	needFlare=YSFALSE;
	interceptGlideSlope=YSFALSE;
	tdPitch=0.0;
	prevErrX=0.0;
	flare=YSFALSE;

	taxi.Initialize();

	dontStopAtFarEnoughPosition=YSFALSE;
	stopTaxiWhenThisFarAwayFromRunwayCenterline=300.0;
	useRunwayClearingPathIfAvailable=YSTRUE;

	destinationSegTypeCache=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
	destinationSegLabelCache.Set("");
}

/* static */ FsLandingAutopilot *FsLandingAutopilot::Create(void)
{
	return new FsLandingAutopilot;
}

void FsLandingAutopilot::SetAirplaneInfo(const FsAirplane &air,const double &bankLimitOverride)
{
	if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
	   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
	   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
	   air.Prop().GetAirplaneCategory()==FSAC_UTILITY)
	{
		bankLimit=YsDegToRad(45.0);
		baseLegTime=15.0;
		downwindLegTime=15.0;
		finalLegTime=15.0;
		glideSlope=YsDegToRad(4.0);
		groundHeight=air.Prop().GetGroundStandingHeight();
	}
	else
	{
		bankLimit=YsDegToRad(27.0);
		baseLegTime=20.0;
		downwindLegTime=20.0;
		finalLegTime=40.0;
		glideSlope=YsDegToRad(3.0);
		groundHeight=air.Prop().GetGroundStandingHeight();
	}
	landingSpeed=air.Prop().GetEstimatedLandingSpeed()*landingSpeedCorrection;

	bankLimit=YsSmaller(bankLimitOverride,bankLimit);

  	double Glat;
	Glat=tan(bankLimit);
	turnRadius=YsSqr(landingSpeed)/(Glat*FsGravityConst);
	turnRatio=(Glat*FsGravityConst)/landingSpeed;
	nintyTurnTime=(YsPi/2.0)/turnRatio;

	entrySpeed=landingSpeed*1.2;
	entryTurnRadius=YsSqr(entrySpeed)/(Glat*FsGravityConst);

	if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
	   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
	   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
	   air.Prop().GetAirplaneCategory()==FSAC_UTILITY)
	{
		requiredRunwayLength=1800.0;
	}
	else
	{
		requiredRunwayLength=2200.0;
	}

	if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
	   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
	   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
	   air.Prop().GetAirplaneCategory()==FSAC_UTILITY)
	{
		trafficPatternAltitudeAGL=YsUnitConv::FTtoM(1000.0);
		verticalSeparation=YsUnitConv::FTtoM(500.0);
	}
	else
	{
		if(air.Prop().GetTotalWeight()>=50000)
		{
			trafficPatternAltitudeAGL=YsUnitConv::FTtoM(2000.0);
			verticalSeparation=YsUnitConv::FTtoM(1000.0);
		}
		else
		{
			trafficPatternAltitudeAGL=YsUnitConv::FTtoM(1000.0);
			verticalSeparation=YsUnitConv::FTtoM(500.0);
		}
	}

	initializedAirplaneInfo=YSTRUE;
}

// SetAirplaneInfo must be done before this function.
YSRESULT FsLandingAutopilot::GetApproachPath
    (double &spd,YsVec3 pos[2],YsAtt3 att[2],YsArray <YsVec3,16> path[2],FsGround *ils)
{
	YsVec3 tdPos;
	YsAtt3 rwAtt;
	YsMatrix4x4 ilsTfm;
	double downwindWidth;

	YsVec3 finalEntry,baseEntry,base1,base2,beginDescend,entry,initial,entryVec,ev,uv;

	if(ils->Prop().GetAircraftCarrierProperty()!=NULL)
	{
		ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
		rwAtt.SetP(0.0);

		printf("TDP: %s\n",tdPos.Txt());

		ilsTfm.Translate(tdPos);
		ilsTfm.Rotate(rwAtt);

		downwindWidth=turnRadius*2.0+landingSpeed*baseLegTime;

		// Left traffic
		finalEntry.Set(0.0,0.0,finalLegTime*landingSpeed);
		baseEntry=finalEntry+YsXVec()*downwindWidth;
		base1=baseEntry-turnRadius*YsXVec()+turnRadius*YsZVec();
		base2=          turnRadius*YsXVec()+turnRadius*YsZVec();
		beginDescend=YsXVec()*downwindWidth;
		entry=beginDescend-YsZVec()*(landingSpeed*downwindLegTime);  // Entrypoint

		entryVec.Set(-1.0,0.0,1.0);
		entryVec.Normalize();

		initial=entry
		       +YsXVec()*(entryTurnRadius-entryTurnRadius*cos(YsPi/4.0))-YsZVec()*entryTurnRadius*sin(YsPi/4.0)
		       -entryVec*landingSpeed*5.0;

		ev.Set(-1.0,0.0,1.0);
		uv=YsYVec();

		ilsTfm.Mul(finalEntry,finalEntry,1.0);
		ilsTfm.Mul(baseEntry,baseEntry,1.0);
		ilsTfm.Mul(base1,base1,1.0);
		ilsTfm.Mul(base2,base2,1.0);
		ilsTfm.Mul(beginDescend,beginDescend,1.0);
		ilsTfm.Mul(entry,entry,1.0);
		ilsTfm.Mul(initial,initial,1.0);
		ilsTfm.Mul(ev,ev,0.0);
		ilsTfm.Mul(uv,uv,0.0);

		initial.SetY(trafficPatternAltitudeAGL+tdPos.y()); // Must come after transformation

		pos[0]=initial;
		att[0].SetTwoVector(ev,uv);
		path[0].Set(0,NULL);
		path[0].Append(initial);
		path[0].Append(entry);
		path[0].Append(beginDescend);
		path[0].Append(baseEntry);
		path[0].Append(base1);
		path[0].Append(base2);
		path[0].Append(finalEntry);


		// Right traffic
		finalEntry.Set(0.0,0.0,finalLegTime*landingSpeed);
		baseEntry=finalEntry-YsXVec()*downwindWidth;
		base1=baseEntry+turnRadius*YsXVec()+turnRadius*YsZVec();
		base2=         -turnRadius*YsXVec()+turnRadius*YsZVec();
		beginDescend=-YsXVec()*downwindWidth;
		entry=beginDescend-YsZVec()*(landingSpeed*downwindLegTime); // Entrypoint

		entryVec.Set(1.0,0.0,1.0);
		entryVec.Normalize();

		initial=entry
		       -YsXVec()*(entryTurnRadius-entryTurnRadius*cos(YsPi/4.0))-YsZVec()*entryTurnRadius*sin(YsPi/4.0)
		       -entryVec*landingSpeed*5.0;

		ev.Set(1.0,0.0,1.0);
		uv=YsYVec();

		ilsTfm.Mul(finalEntry,finalEntry,1.0);
		ilsTfm.Mul(baseEntry,baseEntry,1.0);
		ilsTfm.Mul(base1,base1,1.0);
		ilsTfm.Mul(base2,base2,1.0);
		ilsTfm.Mul(beginDescend,beginDescend,1.0);
		ilsTfm.Mul(entry,entry,1.0);
		ilsTfm.Mul(initial,initial,1.0);
		ilsTfm.Mul(ev,ev,0.0);
		ilsTfm.Mul(uv,uv,0.0);

		initial.SetY(trafficPatternAltitudeAGL+tdPos.y());

		pos[1]=initial;
		att[1].SetTwoVector(ev,uv);
		path[1].Set(0,NULL);
		path[1].Append(initial);
		path[1].Append(entry);
		path[1].Append(beginDescend);
		path[1].Append(baseEntry);
		path[0].Append(base1);
		path[0].Append(base2);
		path[1].Append(finalEntry);

		spd=entrySpeed;

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

void FsLandingAutopilot::SetIls(const FsAirplane &air,const FsSimulation *sim,const FsGround *gnd)
{
	if(gnd->Prop().GetAircraftCarrierProperty()!=NULL)
	{
		ils=gnd;

		approachType=APPROACH_ILS;

		if(gnd->Prop().chType==FSNAVYSHIP || // Must be an aircraft carrier.
		   YSTRUE==air.Prop().NoLandingFlare())
		{
			flareAlt=0.1;
			wheelControlOnGround=YSFALSE;
		}

		YsVec3 tdPos;
		YsAtt3 rwAtt;
		ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);

		sim->GetRunwayRectFromPositionAll(runwayRectCache,tdPos);

		CalculateTrafficPattern(air,tdPos,rwAtt);  // Note:Airplane Info including landing speed must be set before this.
		if(CanMakeStraightIn(air)==YSTRUE)
		{
			yHold=YsGreater(air.GetPosition().y(),330.0);
			timeToGo=0.0;
			landingPhase=PHASE_BASE_TO_FINAL; // 8 Attempt Straight In
		}
		else
		{
			landingPhase=PHASE_FLYINGTOENTRY; // 0
		}
	}
}

void FsLandingAutopilot::SetVfr(const FsAirplane &air,const FsSimulation *sim,const YsVec3 &o,const YsVec3 &v)
{
	approachType=APPROACH_VISUAL;

	YsVec3 tdPos=o;
	YsAtt3 rwAtt=YsZeroAtt();
	rwAtt.SetForwardVector(-v);
	rwAtt.SetP(YsDegToRad(3.0));

	sim->GetRunwayRectFromPositionAll(runwayRectCache,tdPos);

	CalculateTrafficPattern(air,tdPos,rwAtt);  // Note:Airplane Info including landing speed must be set before this.
	if(CanMakeStraightIn(air)==YSTRUE)
	{
		yHold=YsGreater(air.GetPosition().y(),330.0);
		timeToGo=0.0;
		landingPhase=PHASE_BASE_TO_FINAL; // 8 Attempt Straight In
	}
	else
	{
		landingPhase=PHASE_FLYINGTOENTRY; // 0
	}
}

void FsLandingAutopilot::CalculateTrafficPattern(const FsAirplane &air,const YsVec3 tdPosIn,const YsAtt3 rwAttIn)
{
	const YsVec3 &airPos=air.GetPosition();
	const YsAtt3 &airAtt=air.GetAttitude();
	YsVec3 gndAirPos;
	YsAtt3 gndAirAtt;
	gndAirPos=airPos;
	gndAirPos.SetY(0.0);
	gndAirAtt=airAtt;
	gndAirAtt.SetP(0.0);
	gndAirAtt.SetB(0.0);

	tdPos=tdPosIn;
	rwAtt=rwAttIn;
	rwAtt.SetP(0.0);

	trafficPatternAltitude=tdPos.y()+trafficPatternAltitudeAGL;
	printf("TDP: %s\n",tdPos.Txt());
	printf("TPA: %.2lfm MSL (%.2lfm AGL)\n",trafficPatternAltitude,trafficPatternAltitudeAGL);

	// Lateral offset : for landing of a flock of small airplanes.
	YsVec3 latOffset;
	rwAtt.Mul(latOffset,YsXVec());
	latOffset*=lateralOffset;
	tdPos+=latOffset;


	// Estimate entry point.
	YsVec3 oCandidate[2],eCandidate[2],dwDir,bsDir;

	downwindWidth=turnRadius*2.0+landingSpeed*baseLegTime;

	rwAtt.Mul(dwDir,YsZVec());

	bsDir.Set(dwDir.z(),0.0,-dwDir.x());

	eCandidate[0]=tdPos+bsDir*downwindWidth-dwDir*(landingSpeed*downwindLegTime);  // Entrypoint
	eCandidate[1]=tdPos-bsDir*downwindWidth-dwDir*(landingSpeed*downwindLegTime);  // Entrypoint

	if((eCandidate[0]-gndAirPos).GetSquareLength()<(eCandidate[1]-gndAirPos).GetSquareLength())
	{
		entryPoint=eCandidate[0];
	}
	else
	{
		entryPoint=eCandidate[1];
	}

	oCandidate[0]=entryPoint+bsDir*entryTurnRadius;
	oCandidate[1]=entryPoint-bsDir*entryTurnRadius;

	entryTurnCenterCandidate[0]=oCandidate[0];
	entryTurnCenterCandidate[1]=oCandidate[1];
	entryTurnCenterCandidate[0].SetY(0.0);
	entryTurnCenterCandidate[1].SetY(0.0);

	if((oCandidate[0]-gndAirPos).GetSquareLength()<(oCandidate[1]-gndAirPos).GetSquareLength())
	{
		entryTurnCenter=oCandidate[0];
	}
	else
	{
		entryTurnCenter=oCandidate[1];
	}
}

YSBOOL FsLandingAutopilot::CanMakeStraightIn(const FsAirplane &air)
{
	double v,y0,yGS;
	YsVec3 p0,v0;

	rwAtt.SetP(0.0);

	p0=air.GetPosition()-tdPos;
	rwAtt.MulInverse(p0,p0);

	air.Prop().GetVelocity(v0);
	rwAtt.MulInverse(v0,v0);

	v=air.Prop().GetVelocity();
	y0=air.GetPosition().y();
	yGS=p0.z()*tan(glideSlope);

	printf("GS: %lf\n",YsRadToDeg(glideSlope));
	printf("v0: %lf  y0: %lf  yGS: %lf  z0: %lf\n",v,y0,yGS,p0.z());

	if(p0.z()<=landingSpeed*30.0 || fabs(p0.x())>=p0.z()*tan(YsPi/6.0))  // Within +-15 degree envelope.
	{
		return YSFALSE;
	}

	printf("In envelope\n");

	if(y0>=yGS)
	{
		return YSFALSE;
	}

	printf("Below Glide Path\n");

	if(v0.z()<0.0 && fabs(v0.x())<=-v0.z()*tan(YsPi/6.0))
	{
		printf("Heading to the runway.\n");
	}
	else if(v0.z()<0.0 && fabs(v0.x())<=-v0.z()*tan(YsPi/3.0) && p0.z()>=landingSpeed*120.0)
	{
		printf("Not heading to the runway, but have sufficient spacing.\n");
	}
	else
	{
		return YSFALSE;
	}


	printf("v %lf Vldg %lf v/Vldg %lf p0z %lf\n",v,landingSpeed,v/landingSpeed,p0.z());

	if(v<landingSpeed*1.2)
	{
		printf("Close to the landing speed.\n");
	}
	else
	{
		// v/landingSpeed=1.5 -> z>landingSpeed*90
		// v/landingSpeed=2.0 -> z>landingSpeed*150
		double vRate,spacing;
		vRate=v/landingSpeed;
		spacing=(90.0+60.0*(vRate-1.5)*2.0)*landingSpeed;

		printf("vRate %lf  spacing %lf\n",vRate,spacing);

		if(p0.z()>spacing)
		{
			printf("Fast, but have sufficient spacing.\n");
		}
		else
		{
			return YSFALSE;
		}
	}

	return YSTRUE;
}

void FsLandingAutopilot::UpdateCarrierTouchDownPosition(const double dt)
{
	if(APPROACH_ILS==approachType && NULL!=ils && ils->Prop().GetVelocity()>YsTolerance)
	{
		YsVec3 vec;
		ils->Prop().GetVelocity(vec);
		tdPos+=vec*dt;
		entryPoint+=vec*dt;
		entryTurnCenter+=vec*dt;
		entryTurnCenterCandidate[0]+=vec*dt;
		entryTurnCenterCandidate[1]+=vec*dt;
	}
}

YSBOOL FsLandingAutopilot::GoAroundIfCarrierTurned(const FsAirplane &air)
{
	if(APPROACH_ILS==approachType && NULL!=ils && ils->Prop().GetVelocity()>YsTolerance)
	{
		YsVec3 tdPos;
		YsAtt3 rwAtt;
		ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);

		double dh=fmod(fabs(rwAtt.h()-this->rwAtt.h()),YsPi*2.0);
		if(YsPi<dh)
		{
			dh=YsPi*2.0-dh;
		}
		if(YsPi/90.0<dh) // 2 degree rotation
		{
			printf("Going around!\n");
			landingPhase=PHASE_WAITING_UNTIL_CARRIER_TURN_STOP;
			CalculateTrafficPattern(air,tdPos,rwAtt);
			this->tdPos=tdPos;
			this->rwAtt=rwAtt;
			return YSTRUE;
		}
	}
	return YSFALSE;
}

FSTRAFFICPATTERNLEG FsLandingAutopilot::GetCurrentLeg(void) const
{
	if(PHASE_FLYINGTOENTRY==landingPhase)
	{
		return FSLEG_45DEG;
	}
	else if(PHASE_TURN_TO_DOWNWIND<=landingPhase && landingPhase<PHASE_DOWNWIND_TO_BASE)
	{
		return FSLEG_DOWNWIND;
	}
	else if(PHASE_DOWNWIND_TO_BASE<=landingPhase && landingPhase<PHASE_BASE_TO_FINAL)
	{
		return FSLEG_BASE;
	}
	else if(PHASE_BASE_TO_FINAL<=landingPhase && landingPhase<PHASE_FULLSTOP)
	{
		return FSLEG_FINAL;
	}
	return FSLEG_NOT_IN_PATTERN;

}

YSRESULT FsLandingAutopilot::ChooseNamedVfrLanding(YsVec3 &tdPos,YsVec3 &rwDir,const char vfrName[],const FsSimulation *sim) const
{
	const FsField *fld=sim->GetField();
	if(NULL==fld)
	{
		return YSERR;
	}

	const YsScenery *scn=fld->GetFieldPtr();
	if(NULL==scn)
	{
		return YSERR;
	}

	YsArray <const YsSceneryPointSet *,4> mpathArray;
	if(YSOK==scn->SearchPointSetByTag(mpathArray,vfrName))
	{
		for(int mpathIdx=0; mpathIdx<mpathArray.GetN(); ++mpathIdx)
		{
			if(mpathArray[mpathIdx]->GetId()==FS_MPATHID_LANDING_RUNWAY && 2<=mpathArray[mpathIdx]->GetNumPoint())
			{
				tdPos=mpathArray[mpathIdx]->GetTransformedPoint(0);
				const YsVec3 p1=mpathArray[mpathIdx]->GetTransformedPoint(1);
				rwDir=YsUnitVector(p1-tdPos);
				return YSOK;
			}
		}
	}

	return YSERR;
}

YSRESULT FsLandingAutopilot::AutoChooseVfrLanding(YsVec3 &tdPos,YsVec3 &rwDir,const double &requiredRunwayLength,const FsAirplane &air,const FsSimulation *sim) const
{
	const FsField *fld=sim->GetField();
	if(NULL==fld)
	{
		return YSERR;
	}

	const YsScenery *scn=fld->GetFieldPtr();
	if(NULL==scn)
	{
		return YSERR;
	}

	YSRESULT found=YSERR;
	YsVec3 nearestTdPos;
	YsVec3 nearestRwDir;
	double nearestTdPosDist=0.0;
	const YsSceneryPointSet *nearestMPath=NULL;

	YsArray <const YsSceneryPointSet *,4> mpathArray;
	if(YSOK==scn->SearchPointSetById(mpathArray,FS_MPATHID_LANDING_RUNWAY))
	{
		for(int mpathIdx=(int)mpathArray.GetN()-1; 0<=mpathIdx; --mpathIdx)
		{
			if(2>mpathArray[mpathIdx]->GetNumPoint())
			{
				mpathArray.DeleteBySwapping(mpathIdx);
			}
		}

		for(int mpathIdx=0; mpathIdx<mpathArray.GetN(); ++mpathIdx)
		{
			tdPos=mpathArray[mpathIdx]->GetTransformedPoint(0);
			const YsVec3 p1=mpathArray[mpathIdx]->GetTransformedPoint(1);
			rwDir=YsUnitVector(p1-tdPos);

			if(YSTRUE==sim->CheckRunwayLength(tdPos,rwDir,requiredRunwayLength))
			{
				double dSq=(tdPos-air.GetPosition()).GetSquareLength();
				if(YSOK!=found || dSq<nearestTdPosDist)
				{
					found=YSOK;
					nearestTdPos=tdPos;
					nearestRwDir=rwDir;
					nearestTdPosDist=dSq;
					nearestMPath=mpathArray[mpathIdx];
				}
			}
		}
	}

	if(YSOK==found)  // Is there a better visual approach for the wind?
	{
		YsArray <YsVec3,16> visAppCan;
		visAppCan.Append(nearestTdPos);
		visAppCan.Append(nearestRwDir);

		YsArray <const YsSceneryRectRegion *,16> rgnArray;
		if(YSOK==sim->GetRegionRectFromPositionAll(rgnArray,tdPos))
		{
			for(int idx=(int)rgnArray.GetN()-1; 0<=idx; --idx)
			{
				if(FS_RGNID_AIRPORT_AREA!=rgnArray[idx]->GetId())
				{
					rgnArray.DeleteBySwapping(idx);
				}
			}

			for(int rgnIdx=0; rgnIdx<rgnArray.GetN(); ++rgnIdx)
			{
				YsVec3 rect[4];
				sim->GetRegionRect(rect,rgnArray[rgnIdx]);

				for(int mpathIdx=0; mpathIdx<mpathArray.GetN(); ++mpathIdx)
				{
					const YsVec3 tdPos=mpathArray[mpathIdx]->GetTransformedPoint(0);
					if(YSINSIDE==YsCheckInsidePolygon3(tdPos,4,rect))
					{
						const YsVec3 p1=mpathArray[mpathIdx]->GetTransformedPoint(1);
						YsVec3 rwDir=YsUnitVector(p1-tdPos);

						if(YSTRUE==sim->CheckRunwayLength(tdPos,rwDir,requiredRunwayLength))
						{
							visAppCan.Append(tdPos);
							visAppCan.Append(rwDir);
						}
					}
				}
			}

			YSSIZE_T selectedVfrIdx=sim->FindVisualApproachBestForWindFromCandidateArray(visAppCan);
			if(0<=selectedVfrIdx)
			{
				nearestTdPos=visAppCan[selectedVfrIdx];
				nearestRwDir=visAppCan[selectedVfrIdx+1];
			}
		}
	}

	if(YSOK==found)
	{
		tdPos=nearestTdPos;
		rwDir=nearestRwDir;
	}

	return found;
}

const FsGround *FsLandingAutopilot::ChooseNamedILS(const FsAirplane &,const FsSimulation *sim,const char ilsName[]) const
{
	if(NULL!=ilsName && 0!=ilsName[0])
	{
		for(int i=0; i<sim->GetNumILSFacility(); i++)
		{
			const FsGround *gnd=sim->GetILS(i);
			if(gnd->IsAlive()==YSTRUE && strcmp(gnd->name,ilsName)==0)
			{
				return gnd;
			}
		}
	}
	return NULL;
}

/*static*/ const FsGround *FsLandingAutopilot::AutoChooseILS(const double &requiredRunwayLength,const FsAirplane &air,const FsSimulation *sim)
{
	// Find the nearest ILS of a runway longer than requiredRunwayLength.
	YsVec3 h;
	double dist=0.0;
	YsAtt3 rwAtt;
	YsVec3 tdPos,rwDir;

	const FsGround *ils=NULL;

	for(int i=0; i<sim->GetNumILSFacility(); i++)
	{
		FsGround *gnd=sim->GetILS(i);
		if(gnd->IsAlive()==YSTRUE && (FS_IFF_NEUTRAL==gnd->iff || gnd->iff==air.iff))
		{
			gnd->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
			rwAtt.SetP(0.0);

			YsVec3 vel;
			gnd->Prop().GetVelocity(vel);
			if(vel.GetSquareLength()<YsSqr(YsTolerance))
			{
				rwAtt.Mul(rwDir,-YsZVec());
				if(sim->CheckRunwayLength(tdPos,rwDir,requiredRunwayLength)==YSTRUE)
				{
					if(ils==NULL)
					{
						ils=gnd;
						h=tdPos-air.GetPosition();
						h.SetY(0.0);
						dist=h.GetSquareLength();
					}
					else
					{
						h=tdPos-air.GetPosition();
						h.SetY(0.0);
						if(h.GetSquareLength()<dist)
						{
							dist=h.GetSquareLength();
							ils=gnd;
						}
					}
				}
				else
				{
					// printf("Runway %03d is rejected because it's too short.\n",int(YsRadToDeg(rwAtt.h())));
					// printf("Name=%s\n",gnd->Prop().GetIdentifier());
				}
			}
		}
	}

	// Wait, is there better ILS good for the wind?
	if(NULL!=ils && YSTRUE==ils->rectRgnCached)
	{
		YsArray <const FsGround *,16> ilsCandidate;
		ilsCandidate.Append(ils);

		for(int rgnIdx=0; rgnIdx<ils->rectRgnCache.GetN(); ++rgnIdx)
		{
			if(FS_RGNID_AIRPORT_AREA==ils->rectRgnCache[rgnIdx]->GetId())
			{
				const YsSceneryRectRegion *airportRect=ils->rectRgnCache[rgnIdx];
				for(int ilsIdx=0; ilsIdx<sim->GetNumILSFacility(); ++ilsIdx)
				{
					FsGround *tst=sim->GetILS(ilsIdx);

					YsVec3 vel;
					tst->Prop().GetVelocity(vel);
					if(vel.GetSquareLength()<YsSqr(YsTolerance))
					{
						YsVec3 tdPos,rwDir;
						YsAtt3 rwAtt;
						tst->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
						rwAtt.SetP(0.0);

						rwAtt.Mul(rwDir,-YsZVec());
						if(YSTRUE!=sim->CheckRunwayLength(tdPos,rwDir,requiredRunwayLength))
						{
							continue;
						}
					}

					if(ils!=tst &&
					   YSTRUE==tst->IsAlive() &&
					   YSTRUE==tst->rectRgnCached &&
					   YSTRUE==tst->rectRgnCache.IsIncluded(airportRect))
					{
						ilsCandidate.Append(tst);
					}
				}
			}
		}

		ils=sim->FindIlsBestForWindFromCandidateArray(ilsCandidate);
	}

	return ils;
}

YSBOOL FsLandingAutopilot::IsFinalPhaseOfLanding(void) const
{
	if(landingPhase>=PHASE_SHORTFINAL) // 9
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsLandingAutopilot::GetVelocityCorrection(YsVec3 &vec,FsSimulation *sim) const
{
	YsVec3 Vc,Vw;  // Vc:Velocity of the carrier  Vw:Wind velocity
	Vw=sim->GetWeather().GetWind();
	if(NULL!=ils)
	{
		ils->Prop().GetVelocity(Vc);
	}
	else
	{
		Vc=YsOrigin();
	}
	vec=Vw-Vc;
}

double FsLandingAutopilot::GetHeadingCorrection(FsAirplane &air,FsSimulation *sim,const double &courseHeading) const
{
	YsVec3 Vcr;
	GetVelocityCorrection(Vcr,sim);
	if(Vcr.GetSquareLength()>=YsSqr(YsTolerance))
	{
		double d,Va;
		Vcr.RotateXZ(-courseHeading);
		d=Vcr.x();
		Va=air.Prop().GetVelocity();
		if(Va>YsTolerance)
		{
			return asin(YsBound(d/Va,-1.0,1.0));  // YsBound added 2008/02/24
		}
	}
	return 0.0;
}

unsigned FsLandingAutopilot::OverridedControl(void)
{
	return FSAPPLYCONTROL_TRIGGER|FSAPPLYCONTROL_TURRET;
}

/* virtual */ YSBOOL FsLandingAutopilot::DoesRespondToRadioCall(void) const
{
	if(YSTRUE==IsFinalPhaseOfLanding())
	{
		return YSFALSE;
	}
	return YSTRUE;
}

YSRESULT FsLandingAutopilot::MakePriorityDecision(FsAirplane &air)
{
	if(air.Prop().GetFlightState()==FSSTALL && flare!=YSTRUE)
	{
		emr=EMR_STALL;
		landingPhase=PHASE_EMERGENCY_STALL; // -2
		landingSpeed+=air.Prop().GetEstimatedLandingSpeed()*0.05;
		return YSOK;
	}
	return YSOK;
}

YSRESULT FsLandingAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
if(&air==sim->GetPlayerObject())
{
printf("Phase %d\n",landingPhase);
printf("Elv %lf  Trim %lf\n",air.Prop().GetElevator(),air.Prop().GetElvTrim());
}
	const YsVec3 &airPos=air.GetPosition();
	const YsAtt3 &airAtt=air.GetAttitude();
	YsVec3 gndAirPos;
	YsAtt3 gndAirAtt;
	gndAirPos=airPos;
	gndAirPos.SetY(0.0);
	gndAirAtt=airAtt;
	gndAirAtt.SetP(0.0);
	gndAirAtt.SetB(0.0);


#ifdef CRASHINVESTIGATION
	printf("FsLandingAutopilot::MakeDecision Mode %d\n",landingPhase);
#endif
	prevLandingPhase=landingPhase;
	phaseTimer+=dt;


	if(initializedAirplaneInfo!=YSTRUE)
	{
		SetAirplaneInfo(air,YsPi/2.0);
	}

	if(APPROACH_ILS==approachType && (NULL==ils || YSTRUE!=ils->IsAlive()))
	{
		approachType=APPROACH_UNDECIDED;
	}

	if(APPROACH_UNDECIDED==approachType)
	{
		YsVec3 tdPos,rwDir;
		const FsGround *ils;
		if(0<vfrName.Strlen() && YSOK==ChooseNamedVfrLanding(tdPos,rwDir,vfrName,sim))
		{
			SetVfr(air,sim,tdPos,rwDir);
		}
		else if(0<ilsName.Strlen() && NULL!=(ils=ChooseNamedILS(air,sim,ilsName)))
		{
			SetIls(air,sim,ils);
		}
		else if(NULL!=(ils=AutoChooseILS(requiredRunwayLength,air,sim)))
		{
			SetIls(air,sim,ils);
		}
		else if(YSOK==AutoChooseVfrLanding(tdPos,rwDir,requiredRunwayLength,air,sim))
		{
			SetVfr(air,sim,tdPos,rwDir);
		}
		else
		{
			landingPhase=PHASE_EMERGENCY_NO_ILS; // -1
		}
	}

#ifdef CRASHINVESTIGATION
	printf("MakeDecision1\n");
#endif

	if(landingPhase==PHASE_EMERGENCY_STALL) // -2
	{
		if(air.Prop().GetVelocity()>=air.Prop().GetFullyManeuvableSpeed() &&
		   airPos.y()>=trafficPatternAltitude)
		{
			landingPhase=PHASE_TAKING_DISTANCE; // 2
		}
		else
		{
			return YSOK;
		}
	}


#ifdef CRASHINVESTIGATION
	printf("MakeDecision2\n");
#endif


	if(APPROACH_UNDECIDED==approachType || landingPhase==PHASE_EMERGENCY_NO_ILS) // -1
	{
		landingPhase=PHASE_EMERGENCY_NO_ILS; // -1
		return YSOK;
	}

#ifdef CRASHINVESTIGATION
	printf("MakeDecision3\n");
#endif

	UpdateCarrierTouchDownPosition(dt);

#ifdef CRASHINVESTIGATION
	printf("MakeDecision4\n");
#endif

	if(0<runwayRectCache.GetN())
	{
		const FsAirTrafficSequence &trafficInfo=sim->GetAirTrafficSequence();
		for(YSSIZE_T idx=0; idx<runwayRectCache.GetN(); ++idx)
		{
			if(YSTRUE==trafficInfo.WillCauseRunwayIncursionFromLandingTraffic(air,runwayRectCache[idx]))
			{
				// Go around.
				landingPhase=PHASE_GOING_TO_TPA;
			}
		}
	}

	if(landingPhase==PHASE_FLYINGTOENTRY)
	{
		double theata,dist,l;
		dist=(entryTurnCenter-gndAirPos).GetLength();
		// printf("Mode 0: D=%.2lf R=%.2lf\n",dist,entryTurnRadius);

		if(dist<YsMeterPerNm*5.0 && // 2007/09/14
		   (air.Prop().GetVelocity()<entrySpeed*0.8 ||
		    /* air.Prop().GetVelocity()>entrySpeed*1.2 || 2013/05/21 Disabled.  Some airplanes accelerate during descend to TPA. */
		    airPos.y()>trafficPatternAltitude+verticalSeparation+33.0 ||
		    airPos.y()<trafficPatternAltitude-33.0))
		{
			entryPoint.SetY(0.0);
			entryTurnCenter.SetY(0.0);
			landingPhase=PHASE_GOING_TO_TPA; // 1
			if(airPos.y()<trafficPatternAltitude-33.0)
			{
				nDrop++;
				if(nDrop>=2)
				{
					landingSpeedCorrection+=0.05;
					SetAirplaneInfo(air,bankLimit);
					CalculateTrafficPattern(air,tdPos,rwAtt);
				}
			}
			return YSOK;
		}

		if(dist>=entryTurnRadius*1.01)
		{
			theata=asin(entryTurnRadius/dist);
			l=sqrt(YsSqr(dist)-YsSqr(entryTurnRadius));

			YsVec3 v,v1,v2;
			v=entryTurnCenter-gndAirPos;
			v1=v*l/dist;
			v1.RotateXZ(theata);
			v2=v*l/dist;
			v2.RotateXZ(-theata);

			YsVec3 d,tCandidate[2];
			d.Set(0.0,0.0,1.0);
			rwAtt.Mul(d,d);

			tCandidate[0]=gndAirPos+v1;
			tCandidate[1]=gndAirPos+v2;

			if((v1^(tCandidate[0]-entryTurnCenter))*(d^(entryPoint-entryTurnCenter))>=0.0)
			{
				entryTargetPoint=tCandidate[0];
			}
			else
			{
				entryTargetPoint=tCandidate[1];
			}
		}
		else
		{
			printf("Turn to downwind: Dist %lf EntruTurnRadius %lf\n",dist,entryTurnRadius);
			landingPhase=PHASE_TURN_TO_DOWNWIND; // 3
		}
		entryPoint.SetY(0.0);
		entryTurnCenter.SetY(0.0);
		entryTargetPoint.SetY(0.0);
		return YSOK;
	}
	else if(landingPhase==PHASE_GOING_TO_TPA)
	{
		if(air.Prop().GetVelocity()>=entrySpeed*0.9 &&
		   air.Prop().GetVelocity()<=entrySpeed*1.1 &&
		   airPos.y()>=trafficPatternAltitude+verticalSeparation-20.0 &&
		   airPos.y()<=trafficPatternAltitude+verticalSeparation+20.0)
		{
			air.Prop().TurnOffSpeedController(); // This resets the controller
			landingPhase=PHASE_TAKING_DISTANCE; // 2
		}
		return YSOK;
	}
	else if(landingPhase==PHASE_TAKING_DISTANCE) // 2
	{
		const double d0=(gndAirPos-entryTurnCenterCandidate[0]).GetSquareLength();
		const double d1=(gndAirPos-entryTurnCenterCandidate[1]).GetSquareLength();
		if(d0>YsSqr(RequiredDistanceBeforeTurningToEntry()) &&
		   d1>YsSqr(RequiredDistanceBeforeTurningToEntry()))
		{
			if(d0<d1)
			{
				entryTurnCenter=entryTurnCenterCandidate[0];
			}
			else
			{
				entryTurnCenter=entryTurnCenterCandidate[1];
			}
			landingPhase=PHASE_FLYINGTOENTRY;
		}
		return YSOK;
	}
	else if(landingPhase==PHASE_WAITING_UNTIL_CARRIER_TURN_STOP)
	{
		if(APPROACH_ILS!=approachType || NULL==ils || YsTolerance>ils->Prop().GetRotation())
		{
			landingPhase=PHASE_TAKING_DISTANCE;

			YsVec3 tdPos;
			YsAtt3 rwAtt;
			ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);

			CalculateTrafficPattern(air,tdPos,rwAtt);
			this->tdPos=tdPos;
			this->rwAtt=rwAtt;
		}
	}
	else if(landingPhase==PHASE_TURN_TO_DOWNWIND) // 3
	{
		if(YSTRUE==GoAroundIfCarrierTurned(air))
		{
			return YSOK;
		}

		YsVec3 dwDir;
		double dwHdg,hdgCorrection;

		rwAtt.Mul(dwDir,YsZVec());
		dwHdg=atan2(-dwDir.x(),dwDir.z());
		hdgCorrection=GetHeadingCorrection(air,sim,dwHdg);
		dwDir.RotateXZ(hdgCorrection);

		airAtt.MulInverse(dwDir,dwDir);

		double dHdg;
		dHdg=atan2(-dwDir.x(),dwDir.z());

		if(airPos.y()<trafficPatternAltitude-33.0)
		{
			landingPhase=PHASE_GOING_TO_TPA; // 1
			nDrop++;
			if(nDrop>=2)
			{
				landingSpeedCorrection+=0.05;
				SetAirplaneInfo(air,bankLimit);
				CalculateTrafficPattern(air,tdPos,rwAtt);
			}
		}

		if(YsAbs(dHdg)<YsDegToRad(1.0))
		{
			landingPhase=PHASE_DOWNWIND; // 4
		}
		return YSOK;
	}
	else if(landingPhase==PHASE_DOWNWIND) // 4
	{
		if(YSTRUE==GoAroundIfCarrierTurned(air))
		{
			return YSOK;
		}

		YsVec3 rel;
		rel=airPos-tdPos;
		rwAtt.MulInverse(rel,rel);

		if(airPos.y()<trafficPatternAltitude-33.0)
		{
			landingPhase=PHASE_GOING_TO_TPA; // 1
			nDrop++;
			if(nDrop>=2)
			{
				landingSpeedCorrection+=0.05;
				SetAirplaneInfo(air,bankLimit);
				CalculateTrafficPattern(air,tdPos,rwAtt);
			}
		}
		if(rel.z()>0.0)
		{
			landingPhase=PHASE_DOWNWIND_BEAM; // 5
		}

		const double estimatedBaseLegTime=(fabs(rel.x())-turnRadius*2.0)/landingSpeed;
		if(2.0>estimatedBaseLegTime)
		{
			printf("Need to take distance: Estimated Base Leg Time=%lf\n",estimatedBaseLegTime);
			landingPhase=PHASE_TAKING_DISTANCE;
		}

		return YSOK;
	}
	else if(landingPhase==PHASE_DOWNWIND_BEAM) // 5 Begin descend!
	{
		if(YSTRUE==GoAroundIfCarrierTurned(air))
		{
			return YSOK;
		}

		YsVec3 rel;
		rel=airPos-tdPos;
		rwAtt.MulInverse(rel,rel);

		if(rel.z()>landingSpeed*finalLegTime)
		{
			baseTurnStart=airPos;
			yHold=airPos.y();
			distToGo=turnRadius*YsPi+landingSpeed*(baseLegTime+finalLegTime);
			altToGo=airPos.y()-tdPos.y();
			timeToGo=distToGo/landingSpeed;
			landingPhase=PHASE_DOWNWIND_TO_BASE; // 6
		}

		return YSOK;
	}
	else if(landingPhase==PHASE_DOWNWIND_TO_BASE) // 6 Turn to base
	{
		if(YSTRUE==GoAroundIfCarrierTurned(air))
		{
			return YSOK;
		}

		YsVec3 vel;
		air.Prop().GetVelocity(vel);
		rwAtt.MulInverse(vel,vel);
		if(atan2(YsAbs(vel.z()),YsAbs(vel.x()))<YsDegToRad(1.0))
		{
			// Re-estimate altitude to go
			YsVec3 rel;
			rel=airPos-tdPos;
			rwAtt.MulInverse(rel,rel);

			double glideSlopeAlt;
			glideSlopeAlt=tdPos.y()+rel.z()*tan(glideSlope)+groundHeight;
			altToGo=airPos.y()-glideSlopeAlt;
			distToGo=YsAbs(rel.x())-turnRadius;
			timeToGo=distToGo/landingSpeed;
			yHold=airPos.y();
			mode7IdealVY=0.0;  // << updated in ApplyControl

			baseTurnEnd=airPos;
			YsVec3 p1,p2;
			p1=baseTurnStart-tdPos;
			p2=baseTurnEnd-tdPos;
			p1.SetY(0.0);
			p2.SetY(0.0);
			rwAtt.MulInverse(p1,p1);
			rwAtt.MulInverse(p2,p2);
			calibratedTurnRadius=YsAbs(p1.z()-p2.z());

			landingPhase=PHASE_BASE; // 7
		}
		return YSOK;
	}
	else if(landingPhase==PHASE_BASE) // 7 Base leg
	{
		if(YSTRUE==GoAroundIfCarrierTurned(air))
		{
			return YSOK;
		}

		YsVec3 rel;
		rel=airPos-tdPos;
		rwAtt.MulInverse(rel,rel);

		if(YsAbs(rel.x())<calibratedTurnRadius)  // Undershoot is easier to correcct
		{
			double zWhenRollout,yWhenRollout;
			yHold=air.GetPosition().y();

			timeToGo=YsPi/turnRatio;
			zWhenRollout=rel.z()-calibratedTurnRadius;
			yWhenRollout=zWhenRollout*tan(glideSlope)+tdPos.y();  // +tdPos.y() was missing, and added on 2003/01/27
			altToGo=air.GetPosition().y()-yWhenRollout;

			distToGo=turnRadius*YsPi/2.0;
			landingPhase=PHASE_BASE_TO_FINAL; // 8
		}
		// printf("Mode7: %lf %lf\n",rel.x(),calibratedTurnRadius);
		return YSOK;
	}
	else if(landingPhase==PHASE_BASE_TO_FINAL) // 8 Turn to final!
	{
		if(YSTRUE==GoAroundIfCarrierTurned(air))
		{
			return YSOK;
		}

		YsVec3 rel,dir;
		rel=airPos-tdPos;
		rel.SetY(0.0);
		rwAtt.MulInverse(rel,rel);

		aimingPoint.Set(0.0,0.0,rel.z()-landingSpeed*6.0-YsAbs(rel.x()));
		rwAtt.Mul(aimingPoint,aimingPoint);
		aimingPoint+=tdPos;

		aimingDir=aimingPoint-gndAirPos;

		double courseHdg,hdgCorrection;
		courseHdg=atan2(-aimingDir.x(),aimingDir.z());
		hdgCorrection=GetHeadingCorrection(air,sim,courseHdg);
		aimingDir.RotateXZ(hdgCorrection);

		gndAirAtt.MulInverse(aimingDir,aimingDir);

		YsVec3 rwDir,airDir;
		rwAtt.Mul(rwDir,-YsZVec());
		gndAirAtt.MulInverse(rwDir,rwDir);

		gndAirAtt.Mul(airDir,YsZVec());
		rwAtt.MulInverse(airDir,airDir);

		if(atan2(YsAbs(aimingDir.x()),aimingDir.z())<YsDegToRad(1.0) ||
		   atan2(YsAbs(rwDir.x()),rwDir.z())<YsDegToRad(1.0) ||
		   rel.x()*airDir.x()>0.0)
		{
			// printf("To Mode9: %.2lf %.2lf %.2lf %.2lf\n",
			//     YsRadToDeg(atan2(aimingDir.x(),aimingDir.z())),
			//     YsRadToDeg(atan2(rwDir.x(),rwDir.z())),
			//     rel.x(),airDir.x());

			interceptGlideSlope=YSFALSE;
			yHold=airPos.y();
			prevErrX=rel.x();


			YsVec3 rwDir;
			rwAtt.Mul(rwDir,-YsZVec());
			if(autoGoAround==YSTRUE && sim->IsRunwayClear(tdPos,rwDir,requiredRunwayLength)!=YSTRUE)
			{
				// Go around.
				landingPhase=PHASE_GOING_TO_TPA; // 1
			}
			else if(alwaysGoAround==YSTRUE)
			{
				// Go around.
				landingPhase=PHASE_GOING_TO_TPA; // 1
			}
			else
			{
				landingPhase=PHASE_SHORTFINAL; // 9
				nextTaxiwayScanTime=0.0;
			}
		}

		return YSOK;
	}
	else if(landingPhase==PHASE_SHORTFINAL)  // OK, let's land!
	{
		if(YSTRUE!=air.Prop().IsOnGround() && YSTRUE==GoAroundIfCarrierTurned(air))
		{
			return YSOK;
		}

		YsVec3 rel,dir;
		rel=airPos-tdPos;
		rel.SetY(0.0);
		rwAtt.MulInverse(rel,rel);

		aimingPoint.Set(0.0,0.0,rel.z()-landingSpeed*6.0-YsAbs(rel.x()));
		rwAtt.Mul(aimingPoint,aimingPoint);
		aimingPoint+=tdPos;

		aimingDir=aimingPoint-gndAirPos;

		double courseHdg,hdgCorrection;
		courseHdg=atan2(-aimingDir.x(),aimingDir.z());
		hdgCorrection=GetHeadingCorrection(air,sim,courseHdg);
		aimingDir.RotateXZ(hdgCorrection);

		gndAirAtt.MulInverse(aimingDir,aimingDir);

		if(autoClearRunway==YSTRUE && air.Prop().GetVelocity()<taxi.fastTaxiSpeed)
		{
			nextTaxiwayScanTime-=dt;
			YsArray <YsVec3,16> clearRwyPath;
			if(YSTRUE==air.IsInDeadLockFreeAirport() && YSTRUE==air.rectRgnCached)
			{
				if(0.0>nextTaxiwayScanTime)
				{
					nextTaxiwayScanTime=1.5;

					const YsSceneryRectRegion *airportRect=NULL;
					for(YSSIZE_T idx=0; idx<air.rectRgnCache.GetN(); ++idx)
					{
						if(FS_RGNID_AIRPORT_AREA==air.rectRgnCache[idx]->GetId() &&
						   YsSceneryRectRegion::SUBCLASS_DEADLOCK_FREE_AIRPORT==air.rectRgnCache[idx]->GetSubClassType())
						{
							airportRect=air.rectRgnCache[idx];
							break;
						}
					}

					YsArray <const FsAirplane *> airplaneInTheSameAirport;
					for(const FsAirplane *airPtr=NULL; NULL!=(airPtr=sim->FindNextAirplane(airPtr)); )
					{
						if(airPtr!=&air && YSTRUE==airPtr->IsAlive() && sim->GetPlayerObject()!=airPtr && YSTRUE==airPtr->rectRgnCached)
						{
							for(YSSIZE_T idx=0; idx<airPtr->rectRgnCache.GetN(); ++idx)
							{
								if(airPtr->rectRgnCache[idx]==airportRect)
								{
									airplaneInTheSameAirport.Append(airPtr);
									break;
								}
							}
						}
					}

					YSBOOL movingAirplaneInFront=YSFALSE;
					for(YSSIZE_T airIdx=0; airIdx<airplaneInTheSameAirport.GetN(); ++airIdx)
					{
						YsVec3 rel=airplaneInTheSameAirport[airIdx]->GetPosition()-air.GetPosition();
						air.GetAttitude().MulInverse(rel,rel);
						if(0.0<rel.z()-airplaneInTheSameAirport[airIdx]->GetApproximatedCollideRadius())
						{
							movingAirplaneInFront=YSTRUE;
							break;
						}
					}

					const FsField *field=sim->GetField();

					YsArray <YsPair <const YsSceneryPointSet *,double>,16> pathCan;
					if(NULL!=field)
					{
						if(YSOK==sim->FindRunwayClearingPathCandidate(pathCan,air) && 0<pathCan.GetN())
						{
							const YsVec3 ev=air.GetAttitude().GetForwardVector();
							YsArray <YsPair <const YsSceneryPointSet *,double>,16> deadLockFreePathCan;

							deadLockFreePathCan.Clear();

							for(YSSIZE_T pathIdx=0; pathIdx<pathCan.GetN(); ++pathIdx)
							{
								YsArray <YsVec3,16> mpath;
								field->GetPointSet(mpath,pathCan[pathIdx].a);
								if(2<=mpath.GetN())
								{
									const YsVec3 lastVec=YsUnitVector(mpath[mpath.GetN()-1]-mpath[mpath.GetN()-2]);
									if(-YsCos45deg>lastVec*ev)
									{
										deadLockFreePathCan.Append(pathCan[pathIdx]);
									}
								}
							}

							if(YSTRUE!=movingAirplaneInFront)
							{
								if(0<deadLockFreePathCan.GetN())
								{
									field->GetPointSet(clearRwyPath,deadLockFreePathCan[0].a);
								}
								else if(0<pathCan.GetN())
								{
									field->GetPointSet(clearRwyPath,pathCan[0].a);
								}
							}
							else
							{
								if(1==deadLockFreePathCan.GetN())
								{
									field->GetPointSet(clearRwyPath,deadLockFreePathCan[0].a);
								}
								else if(1==pathCan.GetN())
								{
									field->GetPointSet(clearRwyPath,pathCan[0].a);
								}
							}

							if(0<clearRwyPath.GetN())
							{
								taxi.SetTaxiPath((int)clearRwyPath.GetN(),clearRwyPath);
								printf("Use runway clearing path. (Deadlock free airport)\n");
								landingPhase=PHASE_CLEARINGRUNWAY_WITH_TAXIPATH;
							}
						}
						else
						{
							printf("Search taxiway.\n");
							taxi.SetTaxiPath(0,NULL);
							landingPhase=PHASE_CLEARINGRUNWAY_SEARCHTAXIWAY;
							nextTaxiwayScanTime=0.0;
						}
					}
				}
			}
			else
			{
				if(YSTRUE==useRunwayClearingPathIfAvailable &&
				   sim->FindRunwayClearingPath(clearRwyPath,air)==YSOK)
				{
					taxi.SetTaxiPath((int)clearRwyPath.GetN(),clearRwyPath);
					printf("Use runway clearing path.\n");
					landingPhase=PHASE_CLEARINGRUNWAY_WITH_TAXIPATH;
				}
				else
				{
					printf("Search taxiway.\n");
					taxi.SetTaxiPath(0,NULL);
					landingPhase=PHASE_CLEARINGRUNWAY_SEARCHTAXIWAY;
					nextTaxiwayScanTime=0.0;
				}
			}
		}
		return YSOK;
	}
	else if(landingPhase==PHASE_CLEARINGRUNWAY_WITH_TAXIPATH)
	{
		if(YSTRUE!=dontStopAtFarEnoughPosition)
		{
			YsVec3 rel=air.GetPosition()-tdPos;
			rwAtt.MulInverse(rel,rel);
			if(YsAbs(rel.x())>stopTaxiWhenThisFarAwayFromRunwayCenterline)  // OK, it's far enough
			{
				landingPhase=PHASE_FULLSTOP;
			}
		}
		return taxi.MakeDecision(air,sim,dt);
	}
	else if(landingPhase==PHASE_CLEARINGRUNWAY_SEARCHTAXIWAY)
	{
		const double v=air.Prop().GetVelocity();
		if(YsTolerance>v)
		{
			const FsGround *carrier=air.Prop().OnThisCarrier();
			const FsAircraftCarrierProperty *carrierProp;
			if(NULL!=carrier && 
			   NULL!=(carrierProp=carrier->Prop().GetAircraftCarrierProperty()) &&
			   YSTRUE!=carrierProp->NoAutoTaxi() &&
			   YSTRUE==carrierProp->HasCatapult())
			{
				taxi.Initialize();
				taxi.SetMode(FsTaxiingAutopilot::MODE_TAKEOFF_ON_CARRIER);
				landingPhase=PHASE_CLEARING_CARRIER_DECK;
				return taxi.MakeDecision(air,sim,dt);
			}
		}

		// const double brakeForce=air.Prop().CalculateForceByBrake(1.0);
		// const double a=brakeForce/air.Prop().GetTotalWeight();

		// lastTaxiSpeed-a*t=0.0
		// const double timeToStop=(YsTolerance<a ? v/a : YsInfinity);
		// const double distToStop=(v-0.5*a*timeToStop)*timeToStop;
		const double turnRadius=air.Prop().CalculateTurnRatiusOnGround(v);

		nextTaxiwayScanTime-=dt;
		if(nextTaxiwayScanTime<YsTolerance)
		{
			YsVec3 dir=YsXVec();
			YsVec3 span=YsZVec();
			rwAtt.Mul(dir,dir);
			rwAtt.Mul(span,span);

			YsVec3 unitVel;
			air.Prop().GetVelocity(unitVel);
			unitVel.Normalize();
			const YsVec3 tst=air.GetPosition()+unitVel*turnRadius;

			for(int i=0; i<2; i++)
			{
				if(sim->IsOnRunway(tst+dir*20.0)==YSTRUE &&
				   sim->IsOnRunway(tst+dir*30.0)==YSTRUE &&
				   sim->IsOnRunway(tst+dir*40.0)==YSTRUE &&
				   sim->IsOnRunway(tst+dir*50.0)==YSTRUE)
				{
					// Yes!  It looks like a taxiway.
					int j;
					YsVec3 taxiwayWidth[2];
					double upStep,downStep;
					taxiwayWidth[0]=tst;
					taxiwayWidth[1]=tst;
					upStep=5.0;
					downStep=5.0;
					for(j=0; j<6; j++)
					{
						if(sim->IsOnRunway(taxiwayWidth[0]+dir*20.0+span*upStep)==YSTRUE &&
						   sim->IsOnRunway(taxiwayWidth[0]+dir*30.0+span*upStep)==YSTRUE &&
						   sim->IsOnRunway(taxiwayWidth[0]+dir*40.0+span*upStep)==YSTRUE &&
						   sim->IsOnRunway(taxiwayWidth[0]+dir*50.0+span*upStep)==YSTRUE)
						{
							taxiwayWidth[0]+=span*upStep;
						}
						else
						{
							upStep/=2.0;
						}
						if(sim->IsOnRunway(taxiwayWidth[1]+dir*20.0-span*downStep)==YSTRUE &&
						   sim->IsOnRunway(taxiwayWidth[1]+dir*30.0-span*downStep)==YSTRUE &&
						   sim->IsOnRunway(taxiwayWidth[1]+dir*40.0-span*downStep)==YSTRUE &&
						   sim->IsOnRunway(taxiwayWidth[1]+dir*50.0-span*downStep)==YSTRUE)
						{
							taxiwayWidth[1]-=span*downStep;
						}
						else
						{
							downStep/=2.0;
						}
					}

					const YsVec3 turnThreshold=(taxiwayWidth[0]+taxiwayWidth[1])/2.0;
					YsVec3 stopThreshold=turnThreshold;
					for(int sampleStep=0; sampleStep<100; sampleStep+=5)
					{
						const double d=stopTaxiWhenThisFarAwayFromRunwayCenterline*(double)sampleStep/100.0;
						const YsVec3 tst=turnThreshold+d*dir;
						if(YSTRUE!=sim->IsOnRunway(tst))
						{
							break;
						}
						stopThreshold=turnThreshold+dir*(d-air.GetApproximatedCollideRadius());
					}


					YsVec3 taxiPath[2]=
					{
						turnThreshold,
						stopThreshold
					};
					taxi.SetTaxiPath(2,taxiPath);
					landingPhase=PHASE_CLEARINGRUNWAY_WITH_TAXIPATH;

					break;
				}
				dir*=-1.0;
			}
			nextTaxiwayScanTime=1.5;
		}
		return taxi.MakeDecision(air,sim,dt);
	}
	else if(PHASE_CLEARING_CARRIER_DECK==landingPhase)
	{
		return taxi.MakeDecision(air,sim,dt);
	}
	else if(PHASE_FULLSTOP==landingPhase)
	{
		return YSOK;
	}
	else
	{
	}

#ifdef CRASHINVESTIGATION
	printf("MakeDecisionZ\n");
#endif

	return YSERR;
}

YSRESULT FsLandingAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{

#ifdef CRASHINVESTIGATION
	printf("FsLandingAutopilot::ApplyControl %d\n",landingPhase);
#endif

	air.Prop().NeutralDirectAttitudeControl();

	if(prevLandingPhase!=landingPhase)
	{
		phaseTimer=0.0;
		return YSOK;
		// Skip once to make sure all variables are computed.
		//   2008/02/24
		//     Bug: aimingPoint and aimingDir were not initialized in the constructor.
		//          If NAN is set in these, when landingPhase is switched from 7 to 8, NAN is
		//          still in because MakeDecision with landingPhase 8 is not called between
		//          landingPhase switching and ApplyControl.
		//          It can be solved by initializing aimingPoint and aimingDir in the
		//          constructor.  However, for a weak defence, ApplyControl can skip
		//          once just after landingPhase has changed.
	}


	const YsVec3 &airPos=air.GetPosition();
	const YsAtt3 &airAtt=air.GetAttitude();
	YsVec3 gndAirPos;
	YsAtt3 gndAirAtt;
	gndAirPos=airPos;
	gndAirPos.SetY(0.0);
	gndAirAtt=airAtt;
	gndAirAtt.SetP(0.0);
	gndAirAtt.SetB(0.0);

	if(air.isPlayingRecord!=YSTRUE)
	{
		air.Prop().SetVectorMarker(YSTRUE);
		// air.Prop().SetIls(YSTRUE);
	}

	flare=YSFALSE;  // <= will become true only while flareing

#ifdef CRASHINVESTIGATION
	printf("ApplyControl1\n");
#endif

	if(landingPhase==PHASE_FLYINGTOENTRY)
	{
		YsVec3 dir;
		double courseHdg,hdgCorrection;

		double dist,correctedSpd,correctedAlt;
		dist=(gndAirPos-entryTargetPoint).GetLength();


		dir=entryTargetPoint-air.GetPosition();
		courseHdg=atan2(-dir.x(),dir.z());
		hdgCorrection=GetHeadingCorrection(air,sim,courseHdg);
		dir.RotateXZ(hdgCorrection);

		gndAirAtt.MulInverse(dir,dir);

		double dHdg,bnk;
		dHdg=atan2(-dir.x(),dir.z());

		bnk=dHdg*3.0;  // 5degree difference -> 15degree bank
		bnk=YsBound(bnk,-bankLimit,bankLimit);

		air.Prop().BankController(bnk);
		air.Prop().SetRudder(0.0);

		if(dist>YsMeterPerNm*16.0)
		{
			correctedAlt=trafficPatternAltitude+YsMeterPerFt*2000.0;
			correctedSpd=entrySpeed+60.0*YsMeterPerNm/3600.0;
			correctedSpd=YsSmaller(correctedSpd,200*YsMeterPerNm/3600.0);
		}
		else if(dist>YsMeterPerNm*8.0)
		{
			correctedAlt=trafficPatternAltitude+YsMeterPerFt*1000.0;
			correctedSpd=entrySpeed+30.0*YsMeterPerNm/3600.0;
			correctedSpd=YsSmaller(correctedSpd,200*YsMeterPerNm/3600.0);
		}
		else
		{
			correctedAlt=trafficPatternAltitude;
			correctedSpd=entrySpeed;
		}

		ControlGForAltitude(air,sim,correctedAlt,0.0);

		air.Prop().SetRudder(0.0);
		air.Prop().SmartRudder(dt);  // air.Prop().AutoCoordinate();

		air.Prop().SpeedController(correctedSpd);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl2\n");
#endif

		return YSOK;
	}
	else if(landingPhase==PHASE_GOING_TO_TPA || 
	        landingPhase==PHASE_WAITING_UNTIL_CARRIER_TURN_STOP) // 1
	{
		// printf("Mode 1: TPA:%.2lf ALT:%.2lf SPD:%.2lf LDGSPD:%.2lf AOA:%.2lf\n",
		// 	trafficPatternAltitude,air.GetPosition().y(),air.Prop().GetVelocity(),
		// 	landingSpeed,YsRadToDeg(air.Prop().GetAOA()));

		double speedCorrection;
		speedCorrection=1.0+(trafficPatternAltitude+verticalSeparation-airPos.y())*0.25/33.0;
		speedCorrection=YsBound(speedCorrection,1.0,1.25);

		const double yErr=air.GetPosition().y()-(trafficPatternAltitude+verticalSeparation);
		const double speedErr=air.Prop().GetVelocity()-entrySpeed;

		const int c=(int)(phaseTimer/20.0);

		double bnk=0.0;
		if(0==(c&1) &&(0.0<yErr || 0.0<speedErr))
		{
			const double t0=YsGreater(0.0,yErr/330.0);
			const double t1=YsGreater(0.0,speedErr/2.572);

			bnk=(YsPi/6.0)*YsGreater(t0,t1);  // 1000ft, 5kt
			bnk=YsBound(bnk,-bankLimit,bankLimit);
		}

		air.Prop().BankController(bnk);
		air.Prop().SpeedController(entrySpeed*speedCorrection);
		air.Prop().SetRudder(0.0);
		air.Prop().SmartRudder(dt);  // air.Prop().AutoCoordinate();

		ControlGForAltitude(air,sim,trafficPatternAltitude+verticalSeparation,0.0);

		air.Prop().SetGear(0.0);
		air.Prop().SetFlap(0.0);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl3\n");
#endif

		return YSOK;
	}
	else if(landingPhase==PHASE_TAKING_DISTANCE) // 2
	{
		// printf("Mode 2\n");
		air.Prop().BankController(0.0);
		air.Prop().SetRudder(0.0);

		air.Prop().SpeedController(entrySpeed);

		air.Prop().SetRudder(0.0);
		air.Prop().SmartRudder(dt); // air.Prop().AutoCoordinate();

		air.Prop().SetGear(0.0);
		air.Prop().SetFlap(0.0);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl4\n");
#endif

		return ControlGForAltitude(air,sim,trafficPatternAltitude+verticalSeparation,0.0);
	}
	else if(landingPhase==PHASE_TURN_TO_DOWNWIND) // 3
	{
		air.Prop().SpeedController(entrySpeed);

		YsVec3 relCen;
		relCen=entryTurnCenter-gndAirPos;
		gndAirAtt.MulInverse(relCen,relCen);

		double dwHdg,hdgCorrection;
		YsVec3 dwDir;

		rwAtt.Mul(dwDir,YsZVec());
		dwHdg=atan2(-dwDir.x(),dwDir.z());
		hdgCorrection=GetHeadingCorrection(air,sim,dwHdg);
		dwDir.RotateXZ(hdgCorrection);

		gndAirAtt.MulInverse(dwDir,dwDir);

		double dHdg,bnk,bnkCorrection,radiusRatio;
		dHdg=atan2(-dwDir.x(),dwDir.z());

		radiusRatio=relCen.GetLength()/entryTurnRadius;
		bnkCorrection=YsDegToRad(3.0)*(radiusRatio-1.0)/0.03;  // 3% deviation -> 3 degree bank
		bnkCorrection=YsBound(bnkCorrection,YsDegToRad(-3.0),YsDegToRad(3.0));
		if(relCen.x()>0.0)
		{
			bnkCorrection=-bnkCorrection;
		}

		// printf("Mode 3: %.2lf %.2lf  R:%.2lf  TR:%.2lf  Ratio:%.2lf BC:%.2lf\n",
		//     relCen.x(),YsRadToDeg(dHdg),relCen.GetLength(),entryTurnRadius,relCen.GetLength()/entryTurnRadius,
		//     YsRadToDeg(bnkCorrection));

		if((relCen.x()>0.0 && dHdg<0.0) || (relCen.x()<0.0 && dHdg>0.0))  // Must turn toward entryTurnCenter
		{
			bnk=dHdg*3.0;  // 5degree difference -> 15degree bank
			bnk=YsBound(bnk,-bankLimit,bankLimit);
			air.Prop().BankController(bnk+bnkCorrection);
		}
		else if(relCen.x()>0.0)
		{
			air.Prop().BankController(-bankLimit+bnkCorrection);
		}
		else
		{
			air.Prop().BankController(bankLimit+bnkCorrection);
		}
		air.Prop().SetRudder(0.0);
		air.Prop().SmartRudder(dt); // air.Prop().AutoCoordinate();

		ControlGForAltitude(air,sim,trafficPatternAltitude,0.0);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl5\n");
#endif
		return YSOK;
	}
	else if(landingPhase==PHASE_DOWNWIND || landingPhase==PHASE_DOWNWIND_BEAM) // 4 || 5
	{
		air.Prop().SetGear(1.0);
		air.Prop().SetFlap(1.0);

		if(air.Prop().GetHasVariableGeometryNose()==YSTRUE &&
		   air.Prop().GetHasThrustVectoring()!=YSTRUE)
		{
			air.Prop().SetThrustVector(1.0);
		}

		YsVec3 dwDir;
		rwAtt.Mul(dwDir,YsZVec());

		double courseHdg,hdgCorrection;
		courseHdg=atan2(-dwDir.x(),dwDir.z());
		hdgCorrection=GetHeadingCorrection(air,sim,courseHdg);
		dwDir.RotateXZ(hdgCorrection);

		air.GetAttitude().MulInverse(dwDir,dwDir);

		double dHdg,bnk;
		dHdg=atan2(-dwDir.x(),dwDir.z());

		bnk=dHdg*3.0;  // 5degree difference -> 15degree bank

		bnk=YsBound(bnk,-bankLimit,bankLimit);
		// Correction for downwind width must be added (future) //
		air.Prop().BankController(bnk);
		air.Prop().SetRudder(0.0);
		air.Prop().SmartRudder(dt); // air.Prop().AutoCoordinate();

		air.Prop().SpeedController(landingSpeed);

		if(landingPhase==PHASE_DOWNWIND) // 4
		{
			ControlGForAltitude(air,sim,trafficPatternAltitude,0.0);

			YsVec3 rel;
			rel=air.GetPosition()-tdPos;
			rwAtt.MulInverse(rel,rel);
			// printf("Mode 4: DWW:%.2lf  X:%.2lf\n",downwindWidth,rel.x());
		}
		else if(landingPhase==PHASE_DOWNWIND_BEAM) // 5
		{
			// Total distance= v*t_final + turnRadius*(PI/2) + v*t_base + turnRadius*(PI/2) + v*t_final
			// Descend: TPA -> tdPos.y()

			double agl;  // Traffic pattern altitude (above ground level)
			double distAll,distFlew,tAll;
			double v;
			double idealY,idealVY;

			v=landingSpeed;
			distAll=(finalLegTime+baseLegTime+finalLegTime)*v+turnRadius*YsPi;
			tAll=distAll/v;

			YsVec3 rel;
			rel=air.GetPosition()-tdPos;
			rwAtt.MulInverse(rel,rel);
			distFlew=rel.z();

			agl=trafficPatternAltitude-tdPos.y();

			idealY=agl*(1.0-distFlew/distAll)+tdPos.y(); // +tdPos.y() was missing and is added on 2003/01/22
			idealVY=-agl/tAll;

			ControlGForAltitude(air,sim,idealY,idealVY);

			// printf("Mode 5: %lf %lf %lf\n",air.GetPosition().y(),idealY,air.GetPosition().y()-idealY);
		}

#ifdef CRASHINVESTIGATION
	printf("ApplyControl6\n");
#endif
		return YSOK;
	}
	else if(landingPhase==PHASE_DOWNWIND_TO_BASE || landingPhase==PHASE_BASE) // 6 || 7
	{
		YsVec3 rel,baseDir;
		rel=air.GetPosition()-tdPos;
		rwAtt.MulInverse(rel,rel);

		if(rel.x()>0.0)
		{
			baseDir.Set(-1.0,0.0,0.0);
		}
		else
		{
			baseDir.Set(1.0,0.0,0.0);
		}
		rwAtt.Mul(baseDir,baseDir);

		double courseHdg,hdgCorrection;
		courseHdg=atan2(-baseDir.x(),baseDir.z());
		hdgCorrection=GetHeadingCorrection(air,sim,courseHdg);
		baseDir.RotateXZ(hdgCorrection);

		air.GetAttitude().MulInverse(baseDir,baseDir);

		double dHdg,bnk;
		dHdg=atan2(-baseDir.x(),baseDir.z());

		bnk=dHdg*3.0;  // 5degree difference -> 15degree bank
		bnk=YsBound(bnk,-bankLimit,bankLimit);
		// Correction for downwind width must be added (future) //

		air.Prop().BankController(bnk);
		air.Prop().SetRudder(0.0);
		air.Prop().SmartRudder(dt); // air.Prop().AutoCoordinate();

		if(landingPhase==PHASE_DOWNWIND_TO_BASE) // 6
		{
			double idealY,idealVY,distRemain,a;
			YsVec3 ev;
			ev=air.GetAttitude().GetForwardVector();
			rwAtt.MulInverse(ev,ev);
			a=YsPi/2.0-atan2(YsAbs(ev.x()),ev.z());
			distRemain=turnRadius*a+landingSpeed*baseLegTime+turnRadius*YsPi/2.0+landingSpeed*finalLegTime;

			idealY=tdPos.y()+altToGo*(distRemain/distToGo);
			idealVY=-altToGo/timeToGo;

			// printf("Mode 6: %lf %lf\n",distRemain,distToGo);

			air.Prop().SpeedController(landingSpeed);

			ControlGForAltitude(air,sim,idealY,idealVY);
		}
		else if(landingPhase==PHASE_BASE) // 7
		{
			// double distRemain,idealY,idealVY;
			// distRemain=YsAbs(rel.x())-turnRadius;
			//
			// if(altToGo>=0.0)
			// {
			// 	idealY=yHold-altToGo*(1.0-distRemain/distToGo);  // tdPos.y()+altToGo*distRemain/distToGo;
			// 	idealVY=-altToGo/timeToGo;
			// }
			// else
			// {
			// 	idealY=yHold;
			// 	idealVY=0.0;
			// }
			// ControlGForAltitude(air,sim,idealY,idealVY);

			double timeToFinal,distToFinal,idealY,idealVY;
			double altAtFinal;
			YsVec3 airRelPos;
			rwAtt.MulInverse(airRelPos,airPos-tdPos);
			distToFinal=YsAbs(airRelPos.x());
			timeToFinal=distToFinal/landingSpeed;
			altAtFinal=tdPos.y()+airRelPos.z()*tan(glideSlope);
			idealY=altAtFinal+distToFinal*tan(glideSlope);
			idealVY=(altAtFinal-idealY)/timeToFinal;
			if(air.Prop().GetPosition().y()>idealY)
			{
				ControlGForAltitude(air,sim,idealY,idealVY);
				mode7IdealVY=idealVY;
			}
			else
			{
				ControlGForAltitude(air,sim,yHold,0.0);
				mode7IdealVY=0.0;
			}

			air.Prop().SpeedController(landingSpeed);

			// printf("Mode 7: %lf %lf %lf\n",idealY,air.GetPosition().y(),idealVY);
		}

#ifdef CRASHINVESTIGATION
	printf("ApplyControl7\n");
#endif
		return YSOK;
	}
	else if(landingPhase==PHASE_BASE_TO_FINAL) // 8
	{
#ifdef CRASHINVESTIGATION
	printf("ApplyControl8A\n");
#endif

		YsVec3 rel;
		rel=air.GetPosition()-tdPos;
		rwAtt.MulInverse(rel,rel);

		// YsVec3 dir;
		// dir=YsZVec()*landingSpeed*2.0;
		// rwAtt.Mul(dir,dir);
		// dir+=tdPos;  // <= two seconds before touch down
		// dir-=air.GetPosition();

		// air.GetAttitude().MulInverse(dir,dir);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl8B\n");
#endif

		double dHdg,bnk;
		// dHdg=atan2(-dir.x(),dir.z());
		dHdg=atan2(-aimingDir.x(),aimingDir.z());
		bnk=dHdg*3.0;  // 5degree difference -> 15degree bank
		bnk=YsBound(bnk,-bankLimit,bankLimit);
		// Correction for downwind width must be added (future) //

#ifdef CRASHINVESTIGATION
	printf("ApplyControl8C\n");
#endif

		air.Prop().BankController(bnk);
		air.Prop().SetRudder(0.0);
		air.Prop().SmartRudder(dt); // air.Prop().AutoCoordinate();

#ifdef CRASHINVESTIGATION
	printf("ApplyControl8D\n");
#endif

		YsVec3 relVel;
		double angleRemain,distRemain,idealY,idealVY;

		air.Prop().GetVelocity(relVel);
		rwAtt.MulInverse(relVel,relVel);
		angleRemain=atan2(YsAbs(relVel.x()),YsAbs(relVel.z()));
		distRemain=turnRadius*angleRemain;

#ifdef CRASHINVESTIGATION
	printf("ApplyControl8E %lf %lf\n",glideSlope,timeToGo);
#endif

		idealY=tdPos.y()+rel.z()*tan(glideSlope)+YsAbs(rel.x())*tan(glideSlope);
		//                                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^for smooth transition from landingPhase 7
		// idealY=yHold-altToGo+altToGo*distRemain/distToGo;

		if(timeToGo>YsTolerance)
		{
			idealVY=-altToGo/timeToGo;
		}
		else
		{
			idealVY=0.0;
		}

		// printf("Mode 8: %.2lf %.2lf %.2lf %.2lf\n",idealY,air.GetPosition().y(),rel.z(),idealVY);


#ifdef CRASHINVESTIGATION
	printf("ApplyControl8F\n");
#endif


		if(air.Prop().GetHasVariableGeometryNose()==YSTRUE &&
		   air.Prop().GetHasThrustVectoring()!=YSTRUE)
		{
			air.Prop().SetThrustVector(1.0);
		}

#ifdef CRASHINVESTIGATION
	printf("ApplyControl8G\n");
#endif

		if(air.GetPosition().y()>idealY)
		{
			ControlGForAltitude(air,sim,idealY,idealVY);
		}
		else
		{
			ControlGForAltitude(air,sim,yHold,0.0);
		}

#ifdef CRASHINVESTIGATION
	printf("ApplyControl8H\n");
#endif

		air.Prop().SpeedController(landingSpeed);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl8X\n");
#endif

		return YSOK;
	}
	else if(landingPhase==PHASE_SHORTFINAL)  // 9
	{
		YsVec3 rel;
		rel=air.GetPosition()-tdPos;
		rwAtt.MulInverse(rel,rel);

		if(air.Prop().IsOnGround()!=YSTRUE)
		{
			// This is the same trick that I used for formation autopilot.
			// Make a ficticious reference point that is running away on the centerline of the runway.
			// The airplane tries to maintain the reference point straight ahead, then it'll
			// automatically align with the runway.
			double dHdg,bnk;
			//YsVec3 refPoint;
			//refPoint.Set(0.0,0.0,rel.z()-distanceToFicticiousReferencePoint);
			//rwAtt.Mul(refPoint,refPoint);
			//refPoint+=tdPos;
			//air.Prop().GetMatrix().MulInverse(refPoint,refPoint,1.0);
			//dHdg=atan2(-refPoint.x(),refPoint.z());
			dHdg=atan2(-aimingDir.x(),aimingDir.z());
			bnk=dHdg*3.0;
			bnk=YsBound(bnk,-bankLimit,bankLimit);
			air.Prop().BankController(bnk);
			// printf("Mode 9: %.2lf %.2lf %.2lf %.2lf %.2lf\n",dHdg,bnk,aimingDir.x(),aimingDir.z(),rel.x());



			air.Prop().SetRudder(0.0);
			air.Prop().SmartRudder(dt);

			double altToTouchDown;
			altToTouchDown=air.GetPosition().y()-groundHeight-tdPos.y();
			if(altToTouchDown>flareAlt)
			{
				double idealY,idealVY;
				if(interceptGlideSlope!=YSTRUE)
				{
					idealY=tdPos.y()+rel.z()*tan(glideSlope)+groundHeight;
					if(air.GetPosition().y()>=idealY)
					{
						interceptGlideSlope=YSTRUE;
						distToGo=rel.z();
						altToGo=air.GetPosition().y()-tdPos.y();
						timeToGo=(tdPos-air.GetPosition()).GetLength()/landingSpeed;
					}
				}

				if(interceptGlideSlope!=YSTRUE)
				{
					ControlGForAltitude(air,sim,yHold,0.0);
					// printf("Mode9(yHold): %lf %lf %lf\n",air.GetPosition().y(),idealY,0.0);
				}
				else
				{
					YsVec3 Vcr,Vair,Vrel;
					air.Prop().GetVelocity(Vair);
					GetVelocityCorrection(Vcr,sim);
					Vair+=Vcr;
					rwAtt.MulInverse(Vair,Vair);
					timeToGo=rel.z()/(-Vair.z());

					idealY=tdPos.y()+rel.z()*tan(glideSlope)+groundHeight;
					idealVY=-(air.GetPosition().y()-groundHeight-tdPos.y())/timeToGo;

					ControlGForAltitude(air,sim,idealY,idealVY);
					// printf("Mode9(on g.p.): %lf %lf %lf %lf\n",
					//    air.GetPosition().y()-groundHeight,idealY,idealVY,Vair.y());

					air.Prop().SetGear(1.0);
					air.Prop().SetFlap(1.0);
					if(air.Prop().GetHasVariableGeometryNose()==YSTRUE &&
					   air.Prop().GetHasThrustVectoring()!=YSTRUE)
					{
						air.Prop().SetThrustVector(1.0);
					}
				}
				air.Prop().SpeedController(landingSpeed);
			}
			else
			{
				YsVec3 vel;
				double maxPitch,glideAngle;
				air.Prop().GetVelocity(vel);
				if(YSTRUE!=air.Prop().IsTailDragger())
				{
					maxPitch=air.Prop().GetTailStrikePitchAngle(0.8);
				}
				else
				{
					maxPitch=air.Prop().GetGroundStaticPitchAngle()*0.8;
				}
				glideAngle=atan2(-vel.y(),sqrt(vel.x()*vel.x()+vel.z()*vel.z()));

				air.Prop().TurnOffSpeedController();
				air.Prop().SetThrottle(0.0);

				double Greq;
				Greq=1.0-(vel.y()+0.5+2.78*altToTouchDown/flareAlt)*0.1;  // 2.78m/s = 500ft/min descend
				Greq=YsBound(Greq,1.0,1.2);
				air.Prop().GController(Greq);
				air.Prop().SetGControllerAOALimit(glideAngle,maxPitch+glideAngle);

				flare=YSTRUE;

				// printf("Mode9(Flare!) %.2lf %.2lf %.2lf\n",YsRadToDeg(maxPitch),YsRadToDeg(glideAngle),Greq);
			}

			tdPitch=air.GetAttitude().p();
		}
		else
		{
			air.Prop().TurnOffSmartRudder();
			// 2013/02/08
			// TurnOffSmartRudder() will set rudder correction from auto coordination to zero.  On carrier landing, the side-slip angle is calculated 
			// non zero for a fraction of a second, which triggers big rudder correction.  In some unfortunate situation, the rudder correction 
			// will stay non-zero and makes the airplane veer off the carrier.  To make sure rudder correction is turned off, it needs to call
			// TurnOffSmartRudder() here.

			if(wheelControlOnGround==YSTRUE)
			{
				YsVec3 dir;
				double vx;
				vx=-0.4*rel.x();  // Say, 10m difference makes 4m/s motion
				dir.Set(vx,0.0,-landingSpeed);

				rwAtt.Mul(dir,dir);
				air.GetAttitude().MulInverse(dir,dir);

				double dHdg;
				dHdg=atan2(-dir.x(),dir.z());
				air.Prop().BankController(0.0);
				air.Prop().SetRudder(YsRadToDeg(dHdg)/20.0);
			}
			else
			{
				air.Prop().SetRudder(0.0);
			}

			DecelerationAfterLanding(air,sim,dt);
		}

#ifdef CRASHINVESTIGATION
	printf("ApplyControl9\n");
#endif

		return YSOK;
	}
	else if(PHASE_CLEARINGRUNWAY_WITH_TAXIPATH==landingPhase ||
	        PHASE_CLEARINGRUNWAY_SEARCHTAXIWAY==landingPhase ||
	        PHASE_CLEARING_CARRIER_DECK==landingPhase)
	{
		return taxi.ApplyControl(air,sim,dt);
	}
	else if(PHASE_FULLSTOP==landingPhase)
	{
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.0);
		air.Prop().SetBrake(1.0);
	}
	else if(PHASE_EMERGENCY_NO_ILS==landingPhase) // -1
	{
		CircleAround(air,sim,3330.0);
		air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed());

#ifdef CRASHINVESTIGATION
	printf("ApplyControl12\n");
#endif

		return YSOK;
	}
	else if(landingPhase==PHASE_EMERGENCY_STALL) // -2
	{
		air.Prop().TurnOffSpeedController();
		air.Prop().BankController(0.0);
		air.Prop().SetThrottle(1.0);
		air.Prop().SetAfterburner(YSTRUE);
		double g,p;
		YsVec3 vel;
		air.Prop().GetVelocity(vel);
		p=atan2(vel.y(),sqrt(vel.x()*vel.x()+vel.z()*vel.z()));
		g=1.0+0.4*(YsPi/36.0-p)/(YsPi/36.0);
		g=YsBound(g,0.0,1.4);
		air.Prop().GController(g);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl13\n");
#endif
		return YSOK;
	}

	air.Prop().BankController(0.0);
	air.Prop().SetRudder(0.0);
	air.Prop().SpeedController(landingSpeed);
	ControlGForAltitude(air,sim,trafficPatternAltitude,0.0);

#ifdef CRASHINVESTIGATION
	printf("ApplyControl14\n");
#endif

	return YSOK;
}

YSRESULT FsLandingAutopilot::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"LANDING_ %d %d\n",autoGoAround,autoClearRunway);
	if(0<ilsName.Strlen())
	{
		fprintf(fp,"GNDTARGT \"%s\"\n",ilsName.Txt());
	}

	if(0<vfrName.Strlen())
	{
		fprintf(fp,"VFRAPRCH \"%s\"\n",vfrName.Txt());
	}

	fprintf(fp,"FASTTAXI %.2lfm/s\n",taxi.fastTaxiSpeed);
	fprintf(fp,"SLOWTAXI %.2lfm/s\n",taxi.slowTaxiSpeed);
	fprintf(fp,"LASTTAXI %.2lfm/s\n",taxi.lastTaxiSpeed);
	fprintf(fp,"CLRPATH_ %s\n",FsTrueFalseString(useRunwayClearingPathIfAvailable));
	fprintf(fp,"DONTSTOP %s\n",FsTrueFalseString(dontStopAtFarEnoughPosition));
	fprintf(fp,"STOPDIST %.2lfm\n",stopTaxiWhenThisFarAwayFromRunwayCenterline);

	return YSOK;
}

YSRESULT FsLandingAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
{
	YsString str=firstLine;
	YsArray <YsString,16> args;
	for(;;)
	{
		str.Arguments(args);

		if(0==args.GetN())
		{
			continue;
		}

		YSBOOL endIntention;
		if(YSOK==ReadCommonProperty(endIntention,args.GetN(),args))
		{
			if(YSTRUE==endIntention)
			{
				return YSOK;
			}
		}
		else
		{
			if(0==strcmp(args[0],"LANDING_"))
			{
				autoGoAround=(YSBOOL)atoi(args[1]);
				autoClearRunway=(YSBOOL)atoi(args[2]);
			}
			else if(0==strcmp(args[0],"GNDTARGT"))
			{
				ilsName.Set(args[1]);
			}
			else if(0==strcmp(args[0],"FASTTAXI"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				taxi.SetFastTaxiSpeed(spd);
			}
			else if(0==strcmp(args[0],"SLOWTAXI"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				taxi.SetSlowTaxiSpeed(spd);
			}
			else if(0==strcmp(args[0],"LASTTAXI"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				taxi.SetLastTaxiSpeed(spd);
			}
			else if(0==strcmp(args[0],"STOPDIST"))
			{
				FsGetLength(stopTaxiWhenThisFarAwayFromRunwayCenterline,args[1]);
			}
			else if(0==strcmp(args[0],"CLRPATH_"))
			{
				FsGetBool(useRunwayClearingPathIfAvailable,args[1]);
			}
			else if(0==strcmp(args[0],"DONTSTOP"))
			{
				FsGetBool(dontStopAtFarEnoughPosition,args[1]);
			}
			else if(0==strcmp(args[0],"VFRAPRCH"))
			{
				vfrName.Set(args[1]);
			}
			else if(0==strcmp(args[0],"SPDONRWY"))
			{
				double spd;
				FsGetSpeed(spd,args[1]);
				taxi.SetSpeedOnRunway(spd);
			}
			else
			{
				static YSBOOL first=YSTRUE;
				if(YSTRUE==first)
				{
					first=YSFALSE;
					fsStderr.Printf(
					    "Error %s %d\nUnrecognized Token %s"
					    ,__FUNCTION__,__LINE__,args[0].Txt());
				}
			}
		}

		if(NULL==inStream.Gets(str))
		{
			break;
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

const double FsLandingAutopilot::RequiredDistanceBeforeTurningToEntry(void) const
{
	return entryTurnRadius*3.2; // 3.0(+0.2 safety factor)
}

const YsVec3 FsLandingAutopilot::GetRunwayHeadingVector(void) const
{
	const double h=rwAtt.h()+YsPi;
	return YsVec3(-sin(h),0.0,cos(h));
}

////////////////////////////////////////////////////////////

FsTakeOffAutopilot::FsTakeOffAutopilot()
{
	Initialize();
}

/* static */ FsTakeOffAutopilot *FsTakeOffAutopilot::Create(void)
{
	return new FsTakeOffAutopilot;
}

void FsTakeOffAutopilot::Initialize(void)
{
	state=AP_TAKEOFF_STATE_ONTHEGROUND;
	desigAlt=3300.0;
	captureInitialStateAsRunwayCenterline=YSFALSE;
	useRunwayCenterLine=YSFALSE;

	// Memo: 2013/01/29 When FsAirRouteAutopilot state changed from STATE_TAXI_FOR_TAKEOFF to STATE_TAKEOFF,
	//   the state change took place in MakeDecision, and the following ApplyControl called FsTakeOffAutopilot::ApplyControl
	//   without using FsTakeOffAutopilot::MakeDecision once.  That made desiredBank and desiredHeading used without initialization and
	//   made infinite loop in the ApplyControl when it tried to shift the angle to the range -PI to PI.
	//   One fraction of secont delay should not be catastrophic.  This infinite loop can be avoided by initializing
	//   desiredHeading here.
	centerLineO=YsOrigin();
	centerLineV=YsZVec();
	runwayHeading=0.0;

	desiredBank=0.0;
	desiredHeading=0.0;
}

void FsTakeOffAutopilot::UseRunwayCenterLine(const YsVec3 &o,const YsVec3 &v)
{
	useRunwayCenterLine=YSTRUE;
	centerLineO=o;
	centerLineV=YsUnitVector(v);

	runwayHeading=atan2(-centerLineV.x(),centerLineV.z());
}

void FsTakeOffAutopilot::DontUseRunwayCenterLine(void)
{
	useRunwayCenterLine=YSFALSE;
}

YSRESULT FsTakeOffAutopilot::FindRunwayCenterLine(YsVec3 &runwayCenter,YsVec3 &runwayDir,const FsAirplane &air,const FsSimulation *sim)
{
	const FsField *fld=sim->GetField();
	if(NULL!=fld)
	{
		const YsScenery *scn=fld->GetFieldPtr();
		YsArray <const YsSceneryRectRegion *,16> rgnArray;
		scn->GetRectRegionFromPoint(rgnArray,air.GetPosition());
		for(int rgnIdx=(int)rgnArray.GetN()-1; 0<=rgnIdx; --rgnIdx)
		{
			if(FS_RGNID_RUNWAY!=rgnArray[rgnIdx]->GetId())
			{
				rgnArray.DeleteBySwapping(rgnIdx);
			}
		}

		if(1==rgnArray.GetN())
		{
			YsMatrix4x4 tfm;
			scn->GetTransformation(tfm,rgnArray[0]);

			YsVec3 min,max;
			rgnArray[0]->GetMinMax(min,max);
			const YsVec3 d=max-min;

			YsVec3 ref[2];
			if(fabs(d.x())>fabs(d.z()))
			{
				tfm.Mul(ref[0], YsXVec(),0.0);
				tfm.Mul(ref[1],-YsXVec(),0.0);
			}
			else
			{
				tfm.Mul(ref[0], YsZVec(),0.0);
				tfm.Mul(ref[1],-YsZVec(),0.0);
			}

			runwayCenter=tfm*((min+max)/2.0);

			const YsVec3 airFwd=air.GetAttitude().GetForwardVector();
			if(ref[0]*airFwd>ref[1]*airFwd)
			{
				runwayDir=ref[0];
			}
			else
			{
				runwayDir=ref[1];
			}

			printf("Identified Runway Center Line\n");
			printf(" O=%s\n",runwayCenter.Txt());
			printf(" V=%s\n",runwayDir.Txt());

			return YSOK;
		}
	}
	return YSERR;
}

/* virtual */ YSBOOL FsTakeOffAutopilot::IsTakingOff(void) const
{
	if(AP_TAKEOFF_STATE_DONE!=state)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

unsigned FsTakeOffAutopilot::OverridedControl(void)
{
	return FSAPPLYCONTROL_NAVAID|FSAPPLYCONTROL_TRIGGER|FSAPPLYCONTROL_TURRET;
}

YSRESULT FsTakeOffAutopilot::MakePriorityDecision(FsAirplane &air)
{
	if(air.Prop().GetFlightState()==FSSTALL)
	{
		emr=EMR_STALL;
	}
	return YSOK;
}

YSRESULT FsTakeOffAutopilot::MakeDecision(FsAirplane &air,FsSimulation *,const double & /*dt*/)
{
	if(YSTRUE==captureInitialStateAsRunwayCenterline)
	{
		const YsVec3 pos=air.GetPosition();
		YsVec3 dir=air.GetAttitude().GetForwardVector();
		dir.SetY(0.0);
		dir.Normalize();
		UseRunwayCenterLine(pos,dir);
		captureInitialStateAsRunwayCenterline=YSFALSE;
	}

	if(YSTRUE==useRunwayCenterLine)
	{
		YsVec3 airRelToCtrLine;
		airRelToCtrLine=centerLineO-air.GetPosition();
		airRelToCtrLine.RotateXZ(-runwayHeading);

		const double dx=-airRelToCtrLine.x();
		const double hdgOffset=YsDegToRad(YsBound(dx*2.0,-20.0,20.0));

		desiredBank=hdgOffset;
		desiredHeading=runwayHeading+hdgOffset;
	}
	else
	{
		desiredBank=0.0;
		desiredHeading=0.0;
	}

	switch(state)
	{
	case AP_TAKEOFF_STATE_ONTHEGROUND:  // On the ground
		{
			if(air.Prop().GetFlightState()==FSFLYING)
			{
				state=AP_TAKEOFF_STATE_INTHEAIR;
			}
		}
		break;
	case AP_TAKEOFF_STATE_INTHEAIR:  // In the air
		{
			if(air.Prop().GetPosition().y()>=desigAlt)
			{
				state=AP_TAKEOFF_STATE_DONE;
			}
		}
		break;
	case AP_TAKEOFF_STATE_DONE: // Took off
		{
		}
		break;
	}
	return YSOK;
}

YSRESULT FsTakeOffAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double & /*dt*/)
{
	air.Prop().NeutralDirectAttitudeControl();

	if(YSTRUE==useRunwayCenterLine)
	{
		air.Prop().BankController(desiredBank);

		double relHdg=desiredHeading-air.GetAttitude().h();
		while(YsPi<relHdg)
		{
			relHdg-=YsPi*2.0;
		}
		while(-YsPi>relHdg)
		{
			relHdg+=YsPi*2.0;
		}

		const double rud=YsBound(relHdg,-0.3,0.3);
		air.Prop().SetRudder(rud);
	}

	switch(state)
	{
	case AP_TAKEOFF_STATE_ONTHEGROUND:
		{
			air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed());
			air.Prop().SetBrake(0.0);

			if(air.Prop().GetVelocity()>=air.Prop().GetEstimatedLandingSpeed())
			{
				double t,aoa;
				t=(air.Prop().GetVelocity()/air.Prop().GetEstimatedLandingSpeed())-1.0;
				t/=0.2;  // <- V/Vldg==1.2 makes 1.0
				t=YsBound(t,0.0,0.95);
				aoa=air.Prop().GetTailStrikePitchAngle(t);
				if(aoa<air.Prop().GetGroundStaticPitchAngle())
				{
					aoa=air.Prop().GetGroundStaticPitchAngle();
				}
				air.Prop().AOAController(aoa);
			}
			else
			{
				air.Prop().AOAController(air.Prop().GetGroundStaticPitchAngle());
			}
		}
		break;
	case AP_TAKEOFF_STATE_INTHEAIR:
		{
			air.Prop().SetGear(0.0);

			air.Prop().BankController(0.0);

			// Let's control pitch based on the airspeed.
			//   EstimatedLandingSpeed+10kt -> pitch=CriticalAOA
			//   EstimatedLandingSpeed-10kt -> pitch=0

			const double spdWindow=YsUnitConv::KTtoMPS(10.0);
			const double minSpd=air.Prop().GetEstimatedLandingSpeed()-spdWindow;

			const double t=YsBound((air.Prop().GetVelocity()-minSpd)/(spdWindow*2.0),0.0,1.0);

			const double p=air.Prop().GetCriticalAOA()*t;

			air.Prop().PitchController(p);
			air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed());
		}
		break;
	case AP_TAKEOFF_STATE_DONE:
		{
			CircleAround(air,sim,desigAlt);
			minAlt=desigAlt-100.0;
		}
		break;
	}
	return YSOK;
}

YSBOOL FsTakeOffAutopilot::MissionAccomplished(FsAirplane &,FsSimulation *) const
{
	if(state==AP_TAKEOFF_STATE_DONE)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSRESULT FsTakeOffAutopilot::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"TAKEOFF_ %.2lfm\n",desigAlt);
	if(YSTRUE==captureInitialStateAsRunwayCenterline)
	{
		fprintf(fp,"CAPTRNWY TRUE\n");
	}
	return YSOK;
}

YSRESULT FsTakeOffAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
{
	YsString str=firstLine;
	YsArray <YsString,16> args;
	for(;;)
	{
		str.Arguments(args);

		if(0==args.GetN())
		{
			continue;
		}

		YSBOOL endIntention;
		if(YSOK==ReadCommonProperty(endIntention,args.GetN(),args))
		{
			if(YSTRUE==endIntention)
			{
				return YSOK;
			}
		}
		else
		{
			if(0==strcmp(args[0],"TAKEOFF_"))
			{
				if(2<=args.GetN())
				{
					FsGetLength(desigAlt,args[1]);
				}
			}
			else if(0==strcmp(args[0],"CAPTRNWY"))
			{
				if(2<=args.GetN())
				{
					FsGetBool(captureInitialStateAsRunwayCenterline,args[1]);
				}
			}
			else
			{
				static YSBOOL first=YSTRUE;
				if(YSTRUE==first)
				{
					first=YSFALSE;
					fsStderr.Printf(
					    "Error %s %d\nUnrecognized Token %s"
					    ,__FUNCTION__,__LINE__,args[0].Txt());
				}
			}
		}

		if(NULL==inStream.Gets(str))
		{
			break;
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

////////////////////////////////////////////////////////////

FsHoveringAutopilot::FsHoveringAutopilot()
{
	state=0;
	desigPos=YsOrigin();
}

FsHoveringAutopilot::~FsHoveringAutopilot()
{
}

/* static */ FsHoveringAutopilot *FsHoveringAutopilot::Create(void)
{
	return new FsHoveringAutopilot;
}

unsigned FsHoveringAutopilot::OverridedControl(void)
{
	return FSAPPLYCONTROL_NAVAID|FSAPPLYCONTROL_TRIGGER|FSAPPLYCONTROL_TURRET;
}

YSRESULT FsHoveringAutopilot::MakePriorityDecision(FsAirplane &air)
{
	if(YsAbs(air.Prop().GetAttitude().p())>YsPi/3.0)
	{
		emr=EMR_HIGHLOWPITCH;
	}
	return YSOK;
}

YSRESULT FsHoveringAutopilot::MakeDecision(FsAirplane &air,FsSimulation *,const double & /*dt*/)
{
	if(state==0)
	{
		desigPos=air.Prop().GetPosition();
		state=1;
	}
	return YSOK;
}

YSRESULT FsHoveringAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double & /*dt*/)
{
	return VTOLHover(air,sim,desigPos,YsOrigin());
}

YSBOOL FsHoveringAutopilot::MissionAccomplished(FsAirplane &air,FsSimulation *) const
{
	if(air.Prop().GetVelocity()<0.5 &&
	   YsAbs(air.Prop().GetPosition().y()-desigPos.y())<0.5)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsHoveringAutopilot::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"HOVER___ %.2lfm %.2lfm %.2lfm\n",desigPos.x(),desigPos.y(),desigPos.z());
	return YSOK;
}

YSRESULT FsHoveringAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
{
	YsString str=firstLine;
	YsArray <YsString,16> args;
	for(;;)
	{
		str.Arguments(args);

		if(0==args.GetN())
		{
			continue;
		}

		YSBOOL endIntention;
		if(YSOK==ReadCommonProperty(endIntention,args.GetN(),args))
		{
			if(YSTRUE==endIntention)
			{
				return YSOK;
			}
		}
		else
		{
			if(0==strcmp(args[0],"HOVER___"))
			{
				if(4<=args.GetN())
				{
					if(YSOK!=FsGetVec3(desigPos,args.GetN()-1,args.GetArray()+1))
					{
						desigPos.Set(args[1],args[2],args[3]);  // Error will be shown, but will be loaded correctly.
					}
					if(0==state)
					{
						state=1;
					}
				}
			}
			else
			{
				static YSBOOL first=YSTRUE;
				if(YSTRUE==first)
				{
					first=YSFALSE;
					fsStderr.Printf(
					    "Error %s %d\nUnrecognized Token %s"
					    ,__FUNCTION__,__LINE__,args[0].Txt());
				}
			}
		}

		if(NULL==inStream.Gets(str))
		{
			break;
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

////////////////////////////////////////////////////////////

FsVerticalLandingAutopilot::FsVerticalLandingAutopilot()
{
	state=0;
	site=NULL;
	prevSitePos=YsOrigin();
	includeStatic=YSTRUE;
	includeCarrier=YSTRUE;
	siteVelocity=YsOrigin();
}

FsVerticalLandingAutopilot::~FsVerticalLandingAutopilot()
{
}

/* static */ FsVerticalLandingAutopilot *FsVerticalLandingAutopilot::Create(void)
{
	return new FsVerticalLandingAutopilot;
}

YSRESULT FsVerticalLandingAutopilot::AutoChooseILS
    (FsAirplane &air,FsSimulation *sim,YSBOOL includeStatic,YSBOOL includeCarrier)
{
	int i;
	YsVec3 h;
	double minDist=YsInfinity;
	YsAtt3 rwAtt;
	YsVec3 tdPos,rwDir;

	site=NULL;
	for(i=0; i<sim->GetNumILSFacility(); i++)
	{
		FsGround *gnd;
		gnd=sim->GetILS(i);
		if(gnd->IsAlive()==YSTRUE)
		{
			gnd->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
			rwAtt.SetP(0.0);

			if((includeStatic==YSTRUE && gnd->Prop().GetVelocity()<YsTolerance) ||
			   (includeCarrier==YSTRUE && gnd->Prop().GetVelocity()>=YsTolerance))
			{
				if(site==NULL)
				{
					site=gnd;
					h=tdPos-air.GetPosition();
					h.SetY(0.0);
					minDist=h.GetSquareLength();
				}
				else
				{
					h=tdPos-air.GetPosition();
					h.SetY(0.0);
					if(h.GetSquareLength()<minDist)
					{
						minDist=h.GetSquareLength();
						site=gnd;
					}
				}
			}
		}
	}
	if(site!=NULL)
	{
		site->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
	}
	return (site!=NULL ? YSOK : YSERR);
}

unsigned FsVerticalLandingAutopilot::OverridedControl(void)
{
	return FSAPPLYCONTROL_TRIGGER|FSAPPLYCONTROL_TURRET;
}

YSRESULT FsVerticalLandingAutopilot::MakePriorityDecision(FsAirplane &)
{
	return YSOK;
}

YSRESULT FsVerticalLandingAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(site==NULL)
	{
		AutoChooseILS(air,sim,includeStatic,includeCarrier);
		if(site==NULL)
		{
			state=999;
			hold=air.GetPosition();
		}
		else
		{
			prevSitePos=site->GetPosition();
			state=0;
		}
	}

	if(state==0 || state==1)
	{
		YsVec3 sitePos,airPos;
		sitePos=site->GetPosition();
		siteVelocity=(sitePos-prevSitePos)/dt;
		prevSitePos=sitePos;

		site->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);

		/* YsVec3 add;
		add.Set(0.0,0.0,-10.0);
		rwAtt.Mul(add,add);
		tdPos+=add; */

		airPos=air.GetPosition();
		airPos.SetY(0.0);
		tdPos.SetY(0.0);

		if((airPos-tdPos).GetSquareLength()<YsSqr(5.0))
		{
			state=1;
		}
		else
		{
			state=0;
		}
	}

	if(air.Prop().IsOnGround()==YSTRUE)
	{
		state=3;
	}

	return YSOK;
}

YSRESULT FsVerticalLandingAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double & /*dt*/ )
{
	if(state==0 || state==1)
	{
		YsVec3 desigPos;
		desigPos=tdPos;
		if(state==0)
		{
			desigPos.AddY(100.0);
		}
		else
		{
			desigPos.SubY(-0.5);
		}

		air.Prop().SetGear(1.0);
		VTOLHover(air,sim,desigPos,siteVelocity);
	}
	else if(state==3)
	{
		air.Prop().TurnOffController();
		air.Prop().SetThrottle(0.0);
		air.Prop().SetElevator(0.0);
		air.Prop().SetAileron(0.0);
		air.Prop().SetRudder(0.0);
	}
	else
	{
		VTOLHover(air,sim,hold,YsOrigin());
	}
	return YSOK;
}

YSBOOL FsVerticalLandingAutopilot::MissionAccomplished(FsAirplane &,FsSimulation *) const
{
	if(state==3)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsVerticalLandingAutopilot::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"VERTLAND\n");
	return YSOK;
}

YSRESULT FsVerticalLandingAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
{
	YsString str=firstLine;
	YsArray <YsString,16> args;
	for(;;)
	{
		str.Arguments(args);

		if(0==args.GetN())
		{
			continue;
		}

		YSBOOL endIntention;
		if(YSOK==ReadCommonProperty(endIntention,args.GetN(),args))
		{
			if(YSTRUE==endIntention)
			{
				return YSOK;
			}
		}
		else
		{
			if(0==strcmp(args[0],"VERTLAND"))
			{
			}
			else
			{
				static YSBOOL first=YSTRUE;
				if(YSTRUE==first)
				{
					first=YSFALSE;
					fsStderr.Printf(
					    "Error %s %d\nUnrecognized Token %s"
					    ,__FUNCTION__,__LINE__,args[0].Txt());
				}
			}
		}

		if(NULL==inStream.Gets(str))
		{
			break;
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

////////////////////////////////////////////////////////////

FsAirRouteAutopilot::FsAirRouteAutopilot()
{
	landingAP=NULL;
	cruiseAP=NULL;
	taxiingAP=NULL;
	takeoffAP=NULL;
	Initialize();
}

FsAirRouteAutopilot::~FsAirRouteAutopilot()
{
	Initialize();
}

/* static */ FsAirRouteAutopilot *FsAirRouteAutopilot::Create(void)
{
	return new FsAirRouteAutopilot;
}

void FsAirRouteAutopilot::Initialize(void)
{
	state=STATE_INITIAL;
	sameStateTimer=0.0;
	airRouteIdx=0;
	ClearSubAutopilot();
	sceneryCache=NULL;
	airRouteCache=NULL;
	airportCache=NULL;
	airRouteTag.Set("");
	needToDismissFuelTruck=YSFALSE;
	holdingFix=YsOrigin();
	reachedFix=YSFALSE;

	legStart=YsOrigin();

	occupyingSegIdx=-1;
	occupyingSegType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
	occupyingSegLabel.Set("");
}

void FsAirRouteAutopilot::ClearSubAutopilot(void)
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

void FsAirRouteAutopilot::SetAirRouteTag(const char tag[])
{
	airRouteTag.Set(tag);
}

void FsAirRouteAutopilot::SetAirRouteIndex(int idx)
{
	airRouteIdx=idx;
}

void FsAirRouteAutopilot::SetLegStart(const YsVec3 &legStart)
{
	this->legStart=legStart;
}

YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE FsAirRouteAutopilot::CurrentSegmentType(void) const
{
	if(NULL!=airRouteCache && YSTRUE==airRouteCache->routeSequence.IsInRange(airRouteIdx))
	{
		return airRouteCache->routeSequence[airRouteIdx].segType;
	}
	return YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
}

const char *FsAirRouteAutopilot::CurrentSegmentLabel(void) const
{
	if(NULL!=airRouteCache && YSTRUE==airRouteCache->routeSequence.IsInRange(airRouteIdx))
	{
		return airRouteCache->routeSequence[airRouteIdx].label;
	}
	return "";
}

YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE FsAirRouteAutopilot::NextSegmentType(void) const
{
	if(NULL!=airRouteCache && 0<airRouteCache->routeSequence.GetN())
	{
		YSSIZE_T nextIndex=(airRouteIdx+1)%airRouteCache->routeSequence.GetN();
		return airRouteCache->routeSequence[nextIndex].segType;
	}
	return YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
}

const char *FsAirRouteAutopilot::NextSegmentLabel(void) const
{
	if(NULL!=airRouteCache && 0<airRouteCache->routeSequence.GetN())
	{
		YSSIZE_T nextIndex=(airRouteIdx+1)%airRouteCache->routeSequence.GetN();
		return airRouteCache->routeSequence[nextIndex].label;
	}
	return "";
}

YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE FsAirRouteAutopilot::OccupyingSegmentType(void) const
{
	return occupyingSegType;
}

const char *FsAirRouteAutopilot::OccupyingSegmentLabel(void) const
{
	return occupyingSegLabel;
}

YSRESULT FsAirRouteAutopilot::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"AIRROUTE\n");
	if(NULL!=airRouteCache)
	{
		fprintf(fp,"ROUTETAG \"%s\"\n",(const char *)airRouteCache->label.Txt());
	}
	else if(0<airRouteTag.Strlen())
	{
		fprintf(fp,"ROUTETAG \"%s\"\n",(const char *)airRouteTag.Txt());
	}
	fprintf(fp,"ROUTEIDX %d\n",airRouteIdx);
	fprintf(fp,"LEGSTART %lfm %lfm %lfm\n",legStart.x(),legStart.y(),legStart.z());
	return YSOK;
}

YSRESULT FsAirRouteAutopilot::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
{
	YsString str=firstLine;
	YsArray <YsString,16> args;
	for(;;)
	{
		str.Arguments(args);

		if(0==args.GetN())
		{
			continue;
		}

		YSBOOL endIntention;
		if(YSOK==ReadCommonProperty(endIntention,args.GetN(),args))
		{
			if(YSTRUE==endIntention)
			{
				return YSOK;
			}
		}
		else
		{
			if(0==strcmp(args[0],"AIRROUTE"))
			{
			}
			else if(0==strcmp(args[0],"ROUTETAG"))
			{
				SetAirRouteTag(args[1]);
			}
			else if(0==strcmp(args[0],"ROUTEIDX"))
			{
				SetAirRouteIndex(atoi(args[1]));
			}
			else if(0==strcmp(args[0],"LEGSTART"))
			{
				YsVec3 legStart;
				FsGetVec3(legStart,args.GetN()-1,args.GetArray()+1);
				SetLegStart(legStart);
			}
			else
			{
				static YSBOOL first=YSTRUE;
				if(YSTRUE==first)
				{
					first=YSFALSE;
					fsStderr.Printf(
					    "Error %s %d\nUnrecognized Token %s"
					    ,__FUNCTION__,__LINE__,args[0].Txt());
				}
			}
		}

		if(NULL==inStream.Gets(str))
		{
			break;
		}
	}
	return YSERR; // Missing "ENDINTEN"
}

YSRESULT FsAirRouteAutopilot::MakePriorityDecision(FsAirplane &air)
{
	switch(state)
	{
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
	case STATE_ENROUTE:
		if(NULL!=cruiseAP)
		{
			cruiseAP->MakePriorityDecision(air);
		}
		break;
	case STATE_HOLDING:
		break;
	case STATE_APPROACH:
		if(NULL!=landingAP)
		{
			landingAP->MakePriorityDecision(air);
		}
		break;
	default:
		break;
	}
	return YSOK;
}

/* virtual */ YSBOOL FsAirRouteAutopilot::IsTakingOff(void) const
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

YSRESULT FsAirRouteAutopilot::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	AIRROUTE_STATE prevState=state;

	// printf("Flying To: %s  Occupying: %s  State: %d\n",CurrentSegmentLabel(),occupyingSegLabel.Txt(),state);

	switch(state)
	{
	case STATE_INITIAL:
		// Just do nothing for the first 0.5 seconds.
		// Some airplanes start just a little bit off the ground, and the initial state will be settled in the first second.
		if(0.9<air.Prop().GetLandingGear() &&
		   air.GetApproximatedCollideRadius()>air.GetAGL() &&
		   air.Prop().GetVelocity()<0.5 &&
		   YSTRUE!=air.Prop().IsOnGround())
		{
			state=STATE_WAITING_UNTIL_SETTLE;
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

		{
			const FsField *fld=sim->GetField();
			if(NULL==fld)
			{
				state=STATE_ERROR;
				return YSERR;
			}
			sceneryCache=fld->GetFieldPtr();
		}
		//   Fetch the airRoute from the scenery, and populate the cache.

		//   If in the air, fly heading to the airRoutePoint[airRouteIdx];

		//   If on the ground, check:
		//     if the airplane is at one of the airports, set airRouteIdx accordingly, and 
		//     set state to TAXIING and ClearSubAutopilot
		if(YSOK==FetchAirRoute(sim,airRouteTag))
		{
			printf("Fetched Air Route\n");
			if(YSTRUE==air.Prop().IsOnGround())
			{
				return InitializeOnTheGround(air,sim);
			}
			else
			{
				return InitializeInTheAir(air,sim);
			}
		}
		else
		{
			if(YSTRUE==air.Prop().IsOnGround())
			{
				YsArray <const YsSceneryRectRegion *,16> rgnArray;
				sceneryCache->GetRectRegionFromPoint(rgnArray,air.GetPosition());
				for(int rgnIdx=(int)rgnArray.GetN()-1; 0<=rgnIdx; --rgnIdx)
				{
					if(rgnArray[rgnIdx]->GetId()!=FS_RGNID_AIRPORT_AREA)
					{
						rgnArray.DeleteBySwapping(rgnIdx);
					}
				}

				YsArray <const YsSceneryAirRoute *> airRouteArray;
				sceneryCache->FindAllAirRoute(airRouteArray);
				for(int rgnIdx=0; rgnIdx<rgnArray.GetN(); ++rgnIdx)
				{
					for(int routeIdx=0; routeIdx<airRouteArray.GetN(); ++routeIdx)
					{
						for(int seqIdx=0; seqIdx<airRouteArray[routeIdx]->routeSequence.GetN(); ++seqIdx)
						{
							if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT==airRouteArray[routeIdx]->routeSequence[seqIdx].segType &&
							   0==strcmp(airRouteArray[routeIdx]->routeSequence[seqIdx].label,rgnArray[rgnIdx]->GetTag()))
							{
								printf("Use AirRoute [%s]\n",(const char *)airRouteArray[routeIdx]->label);
								airRouteTag.Set(airRouteArray[routeIdx]->label);
								state=STATE_INITIAL; // Let it try again.
								return YSOK; // Initialized in the next.
							}
						}
					}
				}

				const FsGround *onThisCarrier=air.Prop().OnThisCarrier();
				if(NULL!=onThisCarrier)
				{
					for(int routeIdx=0; routeIdx<airRouteArray.GetN(); ++routeIdx)
					{
						for(int seqIdx=0; seqIdx<airRouteArray[routeIdx]->routeSequence.GetN(); ++seqIdx)
						{
							if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER==airRouteArray[routeIdx]->routeSequence[seqIdx].segType &&
							   0==strcmp(airRouteArray[routeIdx]->routeSequence[seqIdx].label,onThisCarrier->GetName()))
							{
								printf("Use AirRoute [%s]\n",(const char *)airRouteArray[routeIdx]->label);
								airRouteTag.Set(airRouteArray[routeIdx]->label);
								state=STATE_INITIAL; // Let it try again.
								return YSOK; // Initialized in the next.
							}
						}
					}
				}
			}
		}
		state=STATE_ERROR;
		return YSERR;
	case STATE_ERROR:
		// Don't waste time trying to re-initialize..
		break;
	case STATE_STATIONARY:
		{
			YSBOOL fuel,ammo;
			FsGround *fuelTruck=sim->FindNearbySupplyTruck(fuel,ammo,air);
			if(NULL!=fuelTruck)
			{
				state=STATE_REFUELING;
				needToDismissFuelTruck=YSFALSE;
			}
			else
			{
				FsGround *fuelTruck=sim->FindNearestSupplyTruckInTheSameRamp(air.GetPosition());
				if(NULL!=fuelTruck)
				{
					state=STATE_CALLING_FUELTRUCK;
					needToDismissFuelTruck=YSTRUE;
				}
				else if(YSOK==sim->GetAirTrafficSequence().RequestProceed(air.SearchKey(),NextSegmentType(),NextSegmentLabel()))
				{
					IncrementRouteIndexForDeparture(air,sim);
					SetUpTaxiForTakeOff(air,sim);
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
			if(YSTRUE==needToDismissFuelTruck)
			{
				state=STATE_DISMISS_FUELTRUCK;
			}
			else if(YSOK==sim->GetAirTrafficSequence().RequestProceed(air.SearchKey(),NextSegmentType(),NextSegmentLabel()))
			{
				if(airRouteCache->routeSequence[airRouteIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER)
				{
					IncrementRouteIndexForDeparture(air,sim);
					SetUpDepartureFromAircraftCarrier(air,sim);
				}
				else
				{
					IncrementRouteIndexForDeparture(air,sim);
					SetUpTaxiForTakeOff(air,sim);
				}
			}
		}
		break;
	case STATE_DISMISS_FUELTRUCK:
		break;
	case STATE_WAIT_FOR_FUELTRUCK_GONE:
		if(10.0<=sameStateTimer &&
		   YSOK==sim->GetAirTrafficSequence().RequestProceed(air.SearchKey(),NextSegmentType(),NextSegmentLabel()))
		{
			IncrementRouteIndexForDeparture(air,sim);
			SetUpTaxiForTakeOff(air,sim);
		}
		break;
	case STATE_TAXI_FOR_TAKEOFF:
		if(NULL!=taxiingAP)
		{
			taxiingAP->MakeDecision(air,sim,dt);
			if(YSTRUE==taxiingAP->MissionAccomplished(air,sim))
			{
				YsVec3 o,v;
				taxiingAP->GetRunwayCenterLine(o,v);
				SetUpForTakeOff(air,sim,o,v);
			}
		}
		break;
	case STATE_TAKEOFF:
		if(NULL!=takeoffAP)
		{
			takeoffAP->MakeDecision(air,sim,dt);
			if(YSTRUE==takeoffAP->MissionAccomplished(air,sim))
			{
				SetUpLeg(air,sim,airRouteIdx);
				ReleaseOccupiedAirportOrFix(air,sim);
			}
		}
		break;
	case STATE_ENROUTE:
		if(NULL!=cruiseAP)
		{
			cruiseAP->MakeDecision(air,sim,dt);
			const YsVec3 nextDestination=GetNextDestination(sim);
			const YsVec3 toNextDesitnation=nextDestination-air.GetPosition();
			const double distToNext=toNextDesitnation.GetLengthXZ();

			if((airRouteCache->routeSequence[airRouteIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT ||
			    airRouteCache->routeSequence[airRouteIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER) &&
			    distToNext<airRouteCache->routeSequence[airRouteIdx].beginApproachAt)
			{
				OccupyAirportOrFix(air,sim,airRouteIdx);
				SetUpForApproach(air,sim,airRouteIdx);
				reachedFix=YSFALSE;
			}
			else if(airRouteCache->routeSequence[airRouteIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR ||
			         airRouteCache->routeSequence[airRouteIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB ||
			         airRouteCache->routeSequence[airRouteIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX)
			{
				YsVec3 fixToAir=air.GetPosition()-nextDestination;
				fixToAir.SetY(0.0);

				YsVec3 legDir=nextDestination-legStart;
				legDir.SetY(0.0);

				if(0.0<fixToAir*legDir)
				{
					reachedFix=YSTRUE;
				}
				cruiseAP->SetSingleDestination(nextDestination);
			}

			if(YSTRUE==reachedFix &&
			   YSOK==sim->GetAirTrafficSequence().RequestProceed(air.SearchKey(),NextSegmentType(),NextSegmentLabel()))
			{
				airRouteIdx=(airRouteIdx+1)%airRouteCache->routeSequence.GetN();
				SetUpLeg(air,sim,airRouteIdx);
			}
		}
		break;
	case STATE_HOLDING:
		break;
	case STATE_APPROACH:
		if(NULL!=landingAP)
		{
			landingAP->MakeDecision(air,sim,dt);
			if(FSGROUNDSTATIC==air.Prop().GetFlightState())
			{
				if(airRouteCache->routeSequence[airRouteIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER)
				{
					ResetTaxiingAutopilot();
					taxiingAP->SetMode(FsTaxiingAutopilot::MODE_TAKEOFF_ON_CARRIER);
					state=STATE_TAXI_ON_CARRIER;
				}
				else
				{
					state=STATE_STATIONARY;
				}
			}
		}
		break;
	case STATE_TAXI_ON_CARRIER:
		if(NULL!=taxiingAP)
		{
			taxiingAP->MakeDecision(air,sim,dt);
			if(5.0<sameStateTimer && FSGROUNDSTATIC==air.Prop().GetFlightState())
			{
				state=STATE_REFUELING;
			}
		}
		break;
	case STATE_WAITING_UNTIL_SETTLE:
		if(YSTRUE==air.Prop().IsOnGround())
		{
			state=STATE_INITIAL; // Try again.
		}
		break;
	default:
		break;
	}

	if(prevState==state)
	{
		sameStateTimer+=dt;
	}
	else
	{
		sameStateTimer=0.0;
	}

	return YSOK;
}

YSRESULT FsAirRouteAutopilot::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	air.Prop().NeutralDirectAttitudeControl();

#ifdef CRASHINVESTIGATION_AIRROUTE
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	switch(state)
	{
	case STATE_INITIAL:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		// Nothing to do.
		break;
	case STATE_ERROR:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		// Don't waste time trying to re-initialize..
		break;
	case STATE_CALLING_FUELTRUCK:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
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

				state=STATE_WAIT_FOR_FUELTRUCK;
				sameStateTimer=0.0;
			}

			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.0);
			air.Prop().SetAfterburner(YSFALSE);
			air.Prop().SetBrake(1.0);
		}
		break;

	case STATE_DISMISS_FUELTRUCK:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
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
			state=STATE_WAIT_FOR_FUELTRUCK_GONE;
			sameStateTimer=0.0;
		}
		break;

	case STATE_STATIONARY:
	case STATE_WAIT_FOR_FUELTRUCK:
	case STATE_WAIT_FOR_FUELTRUCK_GONE:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.0);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().SetBrake(1.0);
		break;
	case STATE_REFUELING:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
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
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.0);
			air.Prop().SetAfterburner(YSFALSE);
			air.Prop().SetBrake(1.0);
		}
		break;
	case STATE_TAXI_FOR_TAKEOFF:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		if(NULL!=taxiingAP)
		{
			taxiingAP->ApplyControl(air,sim,dt);
		}
		break;
	case STATE_TAKEOFF:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		if(NULL!=takeoffAP)
		{
			takeoffAP->ApplyControl(air,sim,dt);
		}
		break;
	case STATE_ENROUTE:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		if(NULL!=cruiseAP)
		{
			cruiseAP->ApplyControl(air,sim,dt);
		}
		break;
	case STATE_HOLDING:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		break;
	case STATE_APPROACH:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		if(NULL!=landingAP)
		{
			landingAP->ApplyControl(air,sim,dt);
		}
		break;
	case STATE_TAXI_ON_CARRIER:
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		if(NULL!=taxiingAP)
		{
			taxiingAP->ApplyControl(air,sim,dt);
		}
		break;
	default:
		break;
	}
#ifdef CRASHINVESTIGATION_AIRROUTE
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	return YSOK;
}

const YsVec3 FsAirRouteAutopilot::GetNextDestination(const FsSimulation *sim) const
{
	return GetSegmentDestination(sim,airRouteIdx);
}

const YsVec3 FsAirRouteAutopilot::GetSegmentDestination(const FsSimulation *sim,YSSIZE_T segIdx) const
{
	if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX==airRouteCache->routeSequence[segIdx].segType ||
	   YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT==airRouteCache->routeSequence[segIdx].segType)
	{
		YsVec3 nextPos=airRouteCache->routeSequence[segIdx].pos;
		nextPos.SetY(airRouteCache->routeSequence[segIdx].altitude);
		return nextPos;
	}
	else if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR==airRouteCache->routeSequence[segIdx].segType ||
	        YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB==airRouteCache->routeSequence[segIdx].segType ||
	        YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER==airRouteCache->routeSequence[segIdx].segType)
	{
		const FsExistence *obj=sim->FindObject(airRouteCache->routeSequence[segIdx].gndObjKey);
		if(NULL!=obj)
		{
			YsVec3 nextPos=obj->GetPosition();
			nextPos.SetY(airRouteCache->routeSequence[segIdx].altitude);
			return nextPos;
		}
	}
	return YsOrigin();
}

YSBOOL FsAirRouteAutopilot::Accomplished(void)
{
	return YSFALSE;
}

YSRESULT FsAirRouteAutopilot::FetchAirRoute(const FsSimulation *sim,const char tag[])
{
	if(NULL==sceneryCache || NULL==(airRouteCache=sceneryCache->FindAirRouteByTag(tag)))
	{
		state=STATE_ERROR;
		return YSERR;
	}

	for(int seqIdx=0; seqIdx<airRouteCache->routeSequence.GetN(); ++seqIdx)
	{
		switch(airRouteCache->routeSequence[seqIdx].segType)
		{
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX:
			{
				YsArray <const YsSceneryRectRegion *> rgnArray;
				if(YSOK==sceneryCache->SearchRegionByTag(rgnArray,airRouteCache->routeSequence[seqIdx].label) && 0<rgnArray.GetN())
				{
					YsMatrix4x4 tfm(YSFALSE);
					sceneryCache->GetTransformation(tfm,rgnArray[0]);
					airRouteCache->routeSequence[seqIdx].pos=tfm*YsOrigin();
				}
				else
				{
					YsString msg;
					msg.Printf("No fix called %s found.",airRouteCache->routeSequence[seqIdx].label.Txt());
					static YSBOOL first=YSTRUE;
					if(YSTRUE==first)
					{
						first=YSFALSE;
						sim->AddTimedMessageWithNoLog(msg);
					}
					goto ERREND;
				}
			}
			break;
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR:
			{
				const FsGround *gnd=sim->FindVorByTag(airRouteCache->routeSequence[seqIdx].label);
				const int searchKey=(NULL!=gnd ? gnd->SearchKey() : -1);
				airRouteCache->routeSequence[seqIdx].gndObjKey=searchKey;
			}
			break;
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB:
			{
				const FsGround *gnd=sim->FindNdbByTag(airRouteCache->routeSequence[seqIdx].label);
				const int searchKey=(NULL!=gnd ? gnd->SearchKey() : -1);
				airRouteCache->routeSequence[seqIdx].gndObjKey=searchKey;
			}
			break;
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT:
			{
				YsArray <const YsSceneryRectRegion *> rgnArray;
				if(YSOK==sceneryCache->SearchRegionByTag(rgnArray,airRouteCache->routeSequence[seqIdx].label) && 0<rgnArray.GetN())
				{
					YsMatrix4x4 tfm(YSFALSE);
					sceneryCache->GetTransformation(tfm,rgnArray[0]);
					airRouteCache->routeSequence[seqIdx].pos=tfm*YsOrigin();
				}
				else
				{
					YsString msg;
					msg.Printf("No airport called %s found.\n",airRouteCache->routeSequence[seqIdx].label.Txt());
					static YSBOOL first=YSTRUE;
					if(YSTRUE==first)
					{
						first=YSFALSE;
						sim->AddTimedMessageWithNoLog(msg);
					}
					goto ERREND;
				}

				airRouteCache->routeSequence[seqIdx].ilsKeyArray.Clear();
				airRouteCache->routeSequence[seqIdx].vfrApproachArray.Clear();
				for(int ilsIdx=0; ilsIdx<airRouteCache->routeSequence[seqIdx].ilsArray.GetN(); ++ilsIdx)
				{
					const FsGround *gnd=sim->FindGroundByTag(airRouteCache->routeSequence[seqIdx].ilsArray[ilsIdx]);
					YsArray <const YsSceneryPointSet *> pstArray;
					if(0<airRouteCache->routeSequence[seqIdx].ilsArray[ilsIdx].Strlen() && NULL!=gnd)
					{
						airRouteCache->routeSequence[seqIdx].ilsKeyArray.Append(gnd->SearchKey());
					}
					else if(YSOK==sceneryCache->SearchPointSetByTag(pstArray,airRouteCache->routeSequence[seqIdx].ilsArray[ilsIdx]))
					{
						for(int pstIdx=0; pstIdx<pstArray.GetN(); ++pstIdx)
						{
							if(pstArray[pstIdx]->GetId()==FS_MPATHID_LANDING_RUNWAY)
							{
								airRouteCache->routeSequence[seqIdx].vfrApproachArray.Append(pstArray[pstIdx]);
							}
						}
					}
				}
				if(0==airRouteCache->routeSequence[seqIdx].ilsArray.GetN()+airRouteCache->routeSequence[seqIdx].vfrApproachArray.GetN())
				{
					YsString msg;
					msg.Printf("No approach to the airport %s available.\n",airRouteCache->routeSequence[seqIdx].label.Txt());
					static YSBOOL first=YSTRUE;
					if(YSTRUE==first)
					{
						first=YSFALSE;
						sim->AddTimedMessageWithNoLog(msg);
					}
					goto ERREND;
				}
			}
			break;
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER:
			{
				const FsGround *gnd=sim->FindIlsOrCarrierByTag(airRouteCache->routeSequence[seqIdx].label);
				const int searchKey=(NULL!=gnd ? gnd->SearchKey() : -1);
				airRouteCache->routeSequence[seqIdx].gndObjKey=searchKey;
			}
			break;
		default:
			break;
		}
	}
	return YSOK;

ERREND:
	state=STATE_ERROR;
	return YSERR;
}

YSRESULT FsAirRouteAutopilot::InitializeOnTheGround(FsAirplane &air,FsSimulation *sim)
{
	int airportIdx=-1;
	YSBOOL onAircraftCarrier=YSFALSE;

	YsArray <const YsSceneryRectRegion *,16> rgnArray;
	sceneryCache->GetRectRegionFromPoint(rgnArray,air.GetPosition());

	const YsSceneryRectRegion *altAirport=NULL;  // If it is not in an airport within the route, it may be in another airport.

	airportCache=NULL;
	for(int rgnIdx=0; rgnIdx<rgnArray.GetN() && NULL==airportCache; ++rgnIdx)
	{
		if(NULL==altAirport && rgnArray[rgnIdx]->GetId()==FS_RGNID_AIRPORT_AREA)
		{
			altAirport=rgnArray[rgnIdx];
		}

		for(int routeIdx=0; routeIdx<airRouteCache->routeSequence.GetN(); ++routeIdx)
		{
			if(airRouteCache->routeSequence[routeIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT &&
			   0==strcmp(rgnArray[rgnIdx]->GetTag(),airRouteCache->routeSequence[routeIdx].label))
			{
				airportCache=rgnArray[rgnIdx];
				airRouteIdx=routeIdx;
				airportIdx=routeIdx;
				printf("Airplane is on the ground at %s\n",(const char *)rgnArray[rgnIdx]->GetTag());
				break;
			}
		}
	}

	if(NULL!=airportCache)
	{
		occupyingSegIdx=airportIdx;
		occupyingSegType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT;
		occupyingSegLabel.Set(airportCache->GetTag());
	}
	else if(NULL==airportCache)
	{
		// Maybe it's on carrier.
		const FsGround *onThisCarrier=air.Prop().OnThisCarrier();
		if(NULL!=onThisCarrier)
		{
			for(int routeIdx=0; routeIdx<airRouteCache->routeSequence.GetN(); ++routeIdx)
			{
				if(airRouteCache->routeSequence[routeIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER &&
				   0==strcmp(onThisCarrier->GetName(),airRouteCache->routeSequence[routeIdx].label))
				{
					airRouteIdx=routeIdx;
					onAircraftCarrier=YSTRUE;

					occupyingSegIdx=routeIdx;
					occupyingSegType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER;
					occupyingSegLabel.Set(onThisCarrier->GetName());

					printf("Airplane is on an aircraft carrier at %s\n",(const char *)onThisCarrier->GetName());
					break;
				}
			}
		}

#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		if(YSTRUE!=onAircraftCarrier)
		{
			airRouteIdx=0; // It's ok.  As long as it can take off.
			if(NULL!=altAirport)
			{
				occupyingSegIdx=-1;
				occupyingSegType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT;
				occupyingSegLabel.Set(altAirport->GetTag());
			}
		}
	}

#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	if(YSTRUE!=onAircraftCarrier)
	{
		if(YSOK==SetUpTaxiForTakeOff(air,sim))
		{
			IncrementRouteIndexForDeparture(air,sim);
			return YSOK;
		}

		YsVec3 o,v;
		if(YSOK==FsTakeOffAutopilot::FindRunwayCenterLine(o,v,air,sim) && YSOK==SetUpForTakeOff(air,sim,o,v))
		{
			IncrementRouteIndexForDeparture(air,sim);
			return YSOK;
		}
	}
	else
	{
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		if(YSOK==SetUpDepartureFromAircraftCarrier(air,sim))
		{
#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
			// airRouteIdx will be updated at the catapult.
			return YSOK;
		}

#ifdef CRASHINVESTIGATION_AIRROUTE
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	}

#ifdef CRASHINVESTIGATION_AIRROUTE
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	state=STATE_ERROR;
	return YSERR;
}

YSRESULT FsAirRouteAutopilot::InitializeInTheAir(FsAirplane &air,FsSimulation *sim)
{
	int nearIdx=-1;
	double nearDist=0.0;
	for(int seqIdx=0; seqIdx<airRouteCache->routeSequence.GetN(); ++seqIdx)
	{
		if(airRouteCache->routeSequence[seqIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX ||
		   airRouteCache->routeSequence[seqIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT)
		{
			const double dist=(airRouteCache->routeSequence[seqIdx].pos-air.GetPosition()).GetSquareLength();
			if(0>nearIdx || dist<nearDist)
			{
				nearIdx=seqIdx;
				nearDist=dist;
			}
		}
		else if(airRouteCache->routeSequence[seqIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR ||
		   airRouteCache->routeSequence[seqIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB ||
		   airRouteCache->routeSequence[seqIdx].segType==YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER)
		{
			const FsExistence *gnd=sim->FindObject(airRouteCache->routeSequence[seqIdx].gndObjKey);
			if(NULL!=gnd)
			{
				const double dist=(gnd->GetPosition()-air.GetPosition()).GetSquareLength();
				if(0>nearIdx || dist<nearDist)
				{
					nearIdx=seqIdx;
					nearDist=dist;
				}
			}
		}
	}

	if(0<=nearIdx)
	{
		SetUpLeg(air,sim,nearIdx);
		return YSOK;
	}
	else
	{
		state=STATE_ERROR;

		YsString msg;
		msg.Printf("Cannot find the nearest fix or airport.");
		static YSBOOL first=YSTRUE;
		if(YSTRUE==first)
		{
			first=YSFALSE;
			sim->AddTimedMessageWithNoLog(msg);
		}
		return YSERR;
	}
}

void FsAirRouteAutopilot::ResetTaxiingAutopilot(void)
{
	if(taxiingAP!=NULL)
	{
		FsAutopilot::Delete(taxiingAP);
		taxiingAP=NULL;
	}
	taxiingAP=FsTaxiingAutopilot::Create();
}

void FsAirRouteAutopilot::ResetLandingAutopilot(void)
{
	if(landingAP!=NULL)
	{
		FsAutopilot::Delete(landingAP);
		landingAP=NULL;
	}
	landingAP=FsLandingAutopilot::Create();
}

void FsAirRouteAutopilot::ResetTakeoffAutopilot(void)
{
	if(takeoffAP!=NULL)
	{
		FsAutopilot::Delete(takeoffAP);
		takeoffAP=NULL;
	}
	takeoffAP=FsTakeOffAutopilot::Create();
}

void FsAirRouteAutopilot::ResetCruiseAutopilot(void)
{
	if(cruiseAP!=NULL)
	{
		FsAutopilot::Delete(cruiseAP);
		cruiseAP=NULL;
	}
	cruiseAP=FsGotoPosition::Create();
}

YSRESULT FsAirRouteAutopilot::SetUpLeg(const FsAirplane &air,const FsSimulation *sim,int destinationIdx)
{
	class CalculateMaximumCruiseSpeedForTheLeg
	{
	public:
		inline static double Calculate(const FsAirplane &air,const YsVec3 &nextDest)
		{
			// a=vw  (1)
			// v=rw  (2)
			// (1) -> w=a/v (3)
			// (3) & (2) -> v=ra/v (4)
			// (4) -> r=v^2/a  (5)

			// a=g*tan(30deg)=about 4.9m/ss

			//  Concorde: 600m/s -> over 70km radius.  Never reach the destination :-P

			// If r is fixed:
			// (5) -> v^2=ra -> v=sqrt(ra)

			const double distToNext=(nextDest-air.GetPosition()).GetLengthXZ();
			const double R=distToNext/5.0;  // Want to finish turn within 1/5 into the leg.

			const double a=FsGravityConst*YsTan30deg;

			const double Vsuggested=sqrt(R*a);

			return YsGreater(YsSmaller(Vsuggested,air.Prop().GetEstimatedCruiseSpeed()),air.Prop().GetEstimatedLandingSpeed()*1.2);
		}
	};

	legStart=air.GetPosition();
	legStart.SetY(0.0);
	if(YSTRUE==airRouteCache->routeSequence.IsInRange(destinationIdx))
	{
		switch(airRouteCache->routeSequence[destinationIdx].segType)
		{
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX:
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT:
			ResetCruiseAutopilot();
			{
				YsVec3 pos=airRouteCache->routeSequence[destinationIdx].pos;
				pos.SetY(airRouteCache->routeSequence[destinationIdx].altitude);
				cruiseAP->SetSingleDestination(pos);
				cruiseAP->speed=CalculateMaximumCruiseSpeedForTheLeg::Calculate(air,pos);
			}
			airRouteIdx=destinationIdx;
			state=STATE_ENROUTE;
			reachedFix=YSFALSE;
			return YSOK;
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB:
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR:
		case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER:
			ResetCruiseAutopilot();
			{
				const FsExistence *gnd=sim->FindObject(airRouteCache->routeSequence[destinationIdx].gndObjKey);
				if(NULL!=gnd)
				{
					YsVec3 pos=gnd->GetPosition();
					pos.SetY(airRouteCache->routeSequence[destinationIdx].altitude);
					cruiseAP->SetSingleDestination(pos);
					cruiseAP->speed=CalculateMaximumCruiseSpeedForTheLeg::Calculate(air,pos);

					airRouteIdx=destinationIdx;
					state=STATE_ENROUTE;
					reachedFix=YSFALSE;
					return YSOK;
				}
			}
			break;
		default:
			break;
		}
	}
	return YSERR;
}

YSRESULT FsAirRouteAutopilot::SetUpTaxiForTakeOff(const FsAirplane &air,const FsSimulation *sim)
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
		else
		{
			YsString msg;
			msg.Printf("%s # Cannot decide taxi path for take-off (%d taxi paths).\n",(const char *)air.GetIdentifier(),taxiPathArray.GetN());
			static YSBOOL first=YSTRUE;
			if(YSTRUE==first)
			{
				first=YSFALSE;
				sim->AddTimedMessageWithNoLog(msg);
			}
			return YSERR;
		}
	}
	else
	{
		YsString msg;
		msg.Printf("%s # No taxi path for take-off.",(const char *)air.GetIdentifier());
		static YSBOOL first=YSTRUE;
		if(YSTRUE==first)
		{
			first=YSFALSE;
			sim->AddTimedMessageWithNoLog(msg);
		}
		return YSERR;
	}
}

YSRESULT FsAirRouteAutopilot::SetUpForTakeOff(const FsAirplane &air,const FsSimulation *,const YsVec3 &rwO,const YsVec3 &rwV)
{
	double climbAlt=YsUnitConv::FTtoM(500.0);


	if(YSTRUE==airRouteCache->routeSequence.IsInRange(occupyingSegIdx) &&
	   (YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT==airRouteCache->routeSequence[occupyingSegIdx].segType ||
	    YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER==airRouteCache->routeSequence[occupyingSegIdx].segType))
	{
		climbAlt=airRouteCache->routeSequence[occupyingSegIdx].takeOffClimbAlt;
	}

	ResetTakeoffAutopilot();
	takeoffAP->UseRunwayCenterLine(rwO,rwV);
	takeoffAP->desigAlt=air.GetPosition().y()+climbAlt;
	state=STATE_TAKEOFF;
	return YSOK;
}

YSRESULT FsAirRouteAutopilot::SetUpDepartureFromAircraftCarrier(const FsAirplane &air,const FsSimulation *sim)
{
	const FsGround *onThisCarrier=air.Prop().OnThisCarrier();
	const FsAircraftCarrierProperty *carrierProp=NULL;
	if(NULL!=onThisCarrier && NULL!=(carrierProp=onThisCarrier->Prop().GetAircraftCarrierProperty()))
	{
		if(YSTRUE==carrierProp->IsOnCatapult(air.GetPosition()))
		{
			YsVec3 catV=carrierProp->GetCatapultVec();
			onThisCarrier->GetMatrix().Mul(catV,catV,0.0);
			catV.SetY(0.0);
			catV.Normalize();

			YsVec3 airV=air.GetAttitude().GetForwardVector();
			airV.SetY(0.0);
			airV.Normalize();

			if(YsCos1deg<catV*airV)
			{
				IncrementRouteIndexForDeparture(air,sim);
				SetUpForTakeOff(air,sim,air.GetPosition(),catV);
				return YSOK;
			}
		}

		ResetTaxiingAutopilot();
		taxiingAP->SetMode(FsTaxiingAutopilot::MODE_TAKEOFF_ON_CARRIER);
		state=STATE_TAXI_ON_CARRIER;
		return YSOK;
	}
	return YSERR;
}

void FsAirRouteAutopilot::IncrementRouteIndexForDeparture(const FsAirplane &air,const FsSimulation *)
{
	// Don't rely on occupyingSegIdx.  It may be cleared before departure.
	if(YSTRUE==airRouteCache->routeSequence.IsInRange(airRouteIdx))
	{
		if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER==airRouteCache->routeSequence[airRouteIdx].segType)
		{
			const FsGround *gnd=air.Prop().OnThisCarrier();
			if(NULL!=gnd && (int)gnd->SearchKey()==airRouteCache->routeSequence[airRouteIdx].gndObjKey)
			{
				airRouteIdx=(airRouteIdx+1)%airRouteCache->routeSequence.GetN();
			}
		}
		else if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT==airRouteCache->routeSequence[airRouteIdx].segType)
		{
			YsArray <const YsSceneryRectRegion *,16> rgnArray;
			sceneryCache->GetRectRegionFromPoint(rgnArray,air.GetPosition());

			for(int rgnIdx=0; rgnIdx<rgnArray.GetN(); ++rgnIdx)
			{
				if(rgnArray[rgnIdx]->GetId()==FS_RGNID_AIRPORT_AREA &&
				   0==strcmp(rgnArray[rgnIdx]->GetTag(),airRouteCache->routeSequence[airRouteIdx].label))
				{
					airRouteIdx=(airRouteIdx+1)%airRouteCache->routeSequence.GetN();
					return;
				}
			}
		}
	}
}

YSRESULT FsAirRouteAutopilot::SetUpForApproach(const FsAirplane &air,const FsSimulation *sim,int destinationIdx)
{
	ResetLandingAutopilot();

	landingAP->SetAirplaneInfo(air,YsPi/2.0);
	landingAP->useRunwayClearingPathIfAvailable=YSTRUE;
	landingAP->dontStopAtFarEnoughPosition=YSTRUE;

	if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER==airRouteCache->routeSequence[destinationIdx].segType)
	{
		const FsGround *ils=sim->FindGround(airRouteCache->routeSequence[destinationIdx].gndObjKey);
		if(NULL!=ils)
		{
			landingAP->SetIls(air,sim,ils);
		}
	}
	else
	{
		if(0<airRouteCache->routeSequence[destinationIdx].ilsKeyArray.GetN())
		{
			YsArray <const FsGround *,16> ilsArray;
			for(YSSIZE_T ilsIdx=0; ilsIdx<airRouteCache->routeSequence[destinationIdx].ilsKeyArray.GetN(); ++ilsIdx)
			{
				const FsGround *ils=sim->FindGround(airRouteCache->routeSequence[destinationIdx].ilsKeyArray[ilsIdx]);
				if(NULL!=ils)
				{
					ilsArray.Append(ils);
				}
			}
			const FsGround *ilsFound=sim->FindIlsBestForWindFromCandidateArray(ilsArray);
			if(NULL!=ilsFound)
			{
				landingAP->SetIls(air,sim,ilsFound);
			}
		}
		else if(0<airRouteCache->routeSequence[destinationIdx].vfrApproachArray.GetN())
		{
			YsArray <YsVec3,16> vfrArray;

			for(YSSIZE_T vfrIdx=0; vfrIdx<airRouteCache->routeSequence[destinationIdx].vfrApproachArray.GetN(); ++vfrIdx)
			{
				const YsVec3 tdPos=airRouteCache->routeSequence[destinationIdx].vfrApproachArray[vfrIdx]->GetTransformedPoint(0);
				const YsVec3 p1=airRouteCache->routeSequence[destinationIdx].vfrApproachArray[vfrIdx]->GetTransformedPoint(1);
				const YsVec3 rwDir=YsUnitVector(p1-tdPos);

				vfrArray.Append(tdPos);
				vfrArray.Append(rwDir);
			}

			YSSIZE_T selectedVfrIdx=sim->FindVisualApproachBestForWindFromCandidateArray(vfrArray);
			if(0<=selectedVfrIdx)
			{
				landingAP->SetVfr(air,sim,vfrArray[selectedVfrIdx],vfrArray[selectedVfrIdx+1]);
			}
		}
	}

	airRouteIdx=destinationIdx;
	state=STATE_APPROACH;
	return YSOK;
}

void FsAirRouteAutopilot::ReleaseOccupiedAirportOrFix(const FsAirplane &air,FsSimulation *sim)
{
	sim->GetAirTrafficSequence().ClearOccupyingAirportOrFix(air.SearchKey());
	occupyingSegIdx=-1;
	occupyingSegType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
	occupyingSegLabel.Set("");
}

void FsAirRouteAutopilot::OccupyAirportOrFix(const FsAirplane &air,FsSimulation *sim,YSSIZE_T segIdx)
{
	occupyingSegIdx=(int)segIdx;
	occupyingSegType=airRouteCache->routeSequence[segIdx].segType;
	occupyingSegLabel.Set(airRouteCache->routeSequence[segIdx].label);
	sim->GetAirTrafficSequence().AddOccupyingObject(air.SearchKey(),occupyingSegType,occupyingSegLabel);
}

