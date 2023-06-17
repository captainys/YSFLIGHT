#include <ysclass.h>
#include <ysunitconv.h>
#include "fs.h"
#include "fsautopilot.h"



// Implementation //////////////////////////////////////////
FsDogfight::FsDogfight()
{
	closeInMaxSpeed=YSFALSE;
	targetFlyingAircraftOnly=YSFALSE;

	pendingTargetAirId=-1;
	pendingWingmanAirId=-1;

	defendThisAirplaneKey=YSNULLHASHKEY;
	targetAirplaneKey=YSNULLHASHKEY;
	wingmanAirplaneKey=YSNULLHASHKEY;
	mode=DFMODE_NORMAL /*0*/;
	prevMode=DFMODE_NORMAL /*0*/;
	g1=0.0;
	g2=0.0;
	g3=0.0;
	gLimit=9.0;
	backSenseRange=YsDegToRad(20.0);
	clock=0.0;
	nextClock=0.0;
	fireClock=0.0;
	flareClock=0.0;

	modeDuration=0.0;
	gLimitCorrection=1.0;
	farThreshold=5000.0;
	combatThreshold=YsUnitConv::NMtoM(17.0);

	nextTargetSearchTimer=0.0;

	giveUpDist=20000.0;
	cruiseAlt=YsUnitConv::FTtoM(30000.0);
	cruiseAP=FsGotoPosition::Create();

	standBy=0.0;
}

FsDogfight::~FsDogfight()
{
	FsAutopilot::Delete(cruiseAP);
}

/* static */ FsDogfight *FsDogfight::Create(void)
{
	return new FsDogfight;
}

YSRESULT FsDogfight::SaveIntention(FILE *fp,const FsSimulation *sim)
{
	// Single line.  Don't interrupt >>
	{
		fprintf(fp,"DOGFIGHT G%.2lf B%.2lf",gLimit,YsRadToDeg(backSenseRange));
		if(mode==DFMODE_STANDBY_GROUND/*100*/)
		{
			fprintf(fp," R%.2lfm",standBy);  // Need unit (m)
		}
		if(mode==DFMODE_STANDBY_AIR/*101*/)
		{
			fprintf(fp," S%.2lfm",standBy);  // Need unit (m)
		}
		if(mode==DFMODE_STANDBY_GROUND_TIMER/*102*/)
		{
			fprintf(fp," W%.2lf",standBy);  // Don't need unit
		}
		FsAirplane *wingman;
		if(pendingWingmanAirId>=0)
		{
			fprintf(fp," F%d",pendingWingmanAirId);
		}
		else if(NULL!=(wingman=sim->FindAirplane(wingmanAirplaneKey)))
		{
			fprintf(fp," F%d",sim->GetAirplaneIdFromHandle(wingman));
		}
		fprintf(fp,"\n");
	}
	// Single line.  Don't interrupt <<

	fprintf(fp,"CLOSEINMAXSPEED %s\n",YsBoolToStr(closeInMaxSpeed));
	fprintf(fp,"TARGETFLYINGONLY %s\n",YsBoolToStr(targetFlyingAircraftOnly));
	fprintf(fp,"COMBATTHRESHOLD %.1lfm\n",combatThreshold);

	if(pendingTargetName.Strlen()>0)
	{
		fprintf(fp,"AIRTARGT \"%s\"\n",pendingTargetName.Txt());
	}

	fprintf(fp,"GIVEUPDS %.1lfm\n",giveUpDist);


	return YSOK;
}

YSRESULT FsDogfight::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
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
			if(0==strcmp(args[0],"DOGFIGHT"))
			{
				for(int i=1; i<args.GetN(); i++)
				{
					switch(args[i][0])
					{
					case 'G':
					case 'g':
						gLimit=atof(args[i].Txt()+1);
						break;
					case 'B':
					case 'b':
						backSenseRange=YsDegToRad(atof(args[i].Txt()+1));
						break;
					case 'R':
					case 'r':
						FsGetLength(standBy,args[i].Txt()+1);
						mode=100;
						prevMode=100;
						break;
					case 'S':
					case 's':
						FsGetLength(standBy,args[i].Txt()+1);
						mode=101;
						prevMode=101;
						break;
					case 'W':
					case 'w':
						standBy=atof(args[i].Txt()+1);
						mode=102;
						prevMode=102;
						break;
					case 'F':
					case 'f':
						pendingWingmanAirId=atoi(args[i].Txt()+1);
						break;
					}
				}
			}
			else if(0==strcmp(args[0],"AIRTARGT"))
			{
				pendingTargetName=args[1];
			}
			else if(0==strcmp(args[0],"GIVEUPDS"))
			{
				FsGetLength(giveUpDist,args[1]);
			}
			else if(0==strcmp(args[0],"CLOSEINMAXSPEED"))
			{
				if(2<=args.GetN())
				{
					closeInMaxSpeed=YsStrToBool(args[1]);
				}
			}
			else if(0==strcmp(args[0],"TARGETFLYINGONLY"))
			{
				if(2<=args.GetN())
				{
					targetFlyingAircraftOnly=YsStrToBool(args[1]);
				}
			}
			else if(0==strcmp(args[0],"COMBATTHRESHOLD"))
			{
				if(2<=args.GetN())
				{
					FsGetLength(combatThreshold,args[1]);
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

void FsDogfight::SetCloseInMaxSpeed(YSBOOL sw)
{
	closeInMaxSpeed=sw;
}
YSBOOL FsDogfight::GetCloseInMaxSpeed(void) const
{
	return closeInMaxSpeed;
}
void FsDogfight::SetTargetFlyingAircraftOnly(YSBOOL sw)
{
	targetFlyingAircraftOnly=sw;
}
YSBOOL FsDogfight::GetTargetFlyingAircraftOnly(void) const
{
	return targetFlyingAircraftOnly;
}

YSRESULT FsDogfight::GetRelativePosition
    (YsVec3 &rel,const YsVec3 &org,FsAirplane &air,FsSimulation *)
{
	rel=air.Prop().GetInverseMatrix()*org;
	return YSOK;
}

YSRESULT FsDogfight::GetRelativeAttitude(YsAtt3 &rel,FsAirplane &air,FsSimulation *sim)
{
	FsAirplane *target;
	target=sim->FindAirplane(targetAirplaneKey);
	if(target!=NULL)
	{
		YsVec3 tev,tuv;

		tev=target->GetAttitude().GetForwardVector();
		tuv=target->GetAttitude().GetUpVector();

		air.GetAttitude().MulInverse(tev,tev);
		air.GetAttitude().MulInverse(tuv,tuv);

		rel.SetTwoVector(tev,tuv);
		return YSOK;
	}
	rel.Set(0.0,0.0,0.0);
	return YSERR;
}

double FsDogfight::GetRelativeBank(FsAirplane &air,FsSimulation *sim)
{
	FsAirplane *target;
	target=sim->FindAirplane(targetAirplaneKey);
	if(target!=NULL)
	{
		return FsAutopilot::GetRelativeBank(target->GetAttitude(),air.GetAttitude());
	}
	return 0.0;
}

YSRESULT FsDogfight::SearchTarget(FsAirplane &air,FsSimulation *sim)
{
	FsAirplane *trg;
	YsVec3 tpos,pos;

	trg=NULL;
	targetAirplaneKey=YSNULLHASHKEY;

	pos=air.Prop().GetPosition();

	FsAirplane *can;
	double min=0.0;
	can=NULL;
	while(NULL!=(can=sim->FindNextAirplane(can)))
	{
		if(YSTRUE==CanBeTarget(&air,can))
		{
			tpos=can->GetPosition();
			if(trg==NULL || (tpos-pos).GetSquareLength()<min)
			{
				trg=can;
				min=(tpos-pos).GetSquareLength();
			}
		}
	}

	if(NULL!=trg)
	{
		if(mode==DFMODE_NOTARGET /*-1*/)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		targetAirplaneKey=FsExistence::GetSearchKey(trg);

		GetRelativePosition(rel1,trg->GetPosition(),air,sim);
		rel2=rel1;
		rel3=rel1;
		return YSOK;
	}
	else
	{
		targetAirplaneKey=YSNULLHASHKEY;
		mode=DFMODE_NOTARGET/*-1*/;
		return YSERR;
	}
}

YSBOOL FsDogfight::CanBeTarget(const FsAirplane *air,const FsAirplane *trg) const
{
	if(0!=(trg->airFlag&FSAIRFLAG_DONTATTACKIFGROUNDSTATIC) && trg->Prop().GetFlightState()==FSGROUNDSTATIC)
	{
		return YSFALSE;
	}
	if(YSTRUE==targetFlyingAircraftOnly && YSTRUE==trg->Prop().IsOnGround())
	{
		return YSFALSE;
	}
	if(trg->Prop().IsActive()!=YSTRUE || trg->iff==air->iff)
	{
		return YSFALSE;
	}
	return YSTRUE;
}

void FsDogfight::SetTarget(FsAirplane *air)
{
	targetAirplaneKey=air->SearchKey();
}

FsAirplane *FsDogfight::GetTarget(FsSimulation *sim)
{
	return sim->FindAirplane(targetAirplaneKey);
}

YSRESULT FsDogfight::MakePriorityDecision(FsAirplane &air)
{
	if(mode==DFMODE_NOTARGET/*-1*/ || air.Prop().GetFlightState()==FSSTALL)
	{
		return FsAutopilot::MakePriorityDecision(air);
	}
	else
	{
		emr=EMR_NONE;
		return YSOK;
	}
}

YSRESULT FsDogfight::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	YsVec3 tpos;



	if(mode==DFMODE_STANDBY_GROUND_TIMER/*102*/)  // Wait for standBy seconds.
	{
		standBy-=dt;
		if(standBy<YsTolerance)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else
		{
			return YSOK;
		}
	}


	clock+=dt;
	modeDuration+=dt;
	if(mode!=prevMode)
	{
		modeDuration=0.0;
		prevMode=mode;
	}


	if(pendingTargetAirId>=0)
	{
		FsAirplane *pendingTarget;
		pendingTarget=sim->GetAirplaneById(pendingTargetAirId);
		if(pendingTarget!=NULL && pendingTarget->Prop().IsActive()==YSTRUE && pendingTarget->iff!=air.iff)
		{
			targetAirplaneKey=FsExistence::GetSearchKey(pendingTarget);
		}
		pendingTargetAirId=-1;
	}
	if(pendingWingmanAirId>=0)
	{
		FsAirplane *pendingWingman;
		pendingWingman=sim->GetAirplaneById(pendingWingmanAirId);
		if(pendingWingman!=NULL && pendingWingman->Prop().IsActive()==YSTRUE && pendingWingman->iff==air.iff)
		{
			wingmanAirplaneKey=FsExistence::GetSearchKey(pendingWingman);
		}
		pendingWingmanAirId=-1;
	}
// printf("DF1\n");
	if(pendingTargetName.Strlen()>0)
	{
// printf("DF2\n");
		FsAirplane *pendingTarget=sim->FindAirplaneByName(pendingTargetName);
// printf("DF3\n");
		if(pendingTarget!=NULL && pendingTarget->Prop().IsActive()==YSTRUE && pendingTarget->iff!=air.iff)
		{
			targetAirplaneKey=FsExistence::GetSearchKey(pendingTarget);
		}
		pendingTargetName.Set("");
// printf("DF4\n");
	}


	FsAirplane *target=GetTarget(sim);
	FsAirplane *defendThis=sim->FindAirplane(defendThisAirplaneKey);
	if(defendThis!=NULL && target!=NULL)
	{
		YSBOOL targetOk=YSFALSE;

		FsAutopilot *ap=target->GetAutopilot();
		if(ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT) // ap->WhatItIs()==FsDogfight::ClassName)
		{
			FsDogfight *df=(FsDogfight *)ap;
			if(df->GetTarget(sim)==defendThis)
			{
				targetOk=YSTRUE;
			}
		}
		// If none of the airplanes is chasing the defend target, don't need to update the target.
		if(targetOk!=YSTRUE)
		{
			FsAirplane *tst=NULL;
			while((tst=sim->FindNextAirplane(tst))!=NULL)
			{
				ap=tst->GetAutopilot();
				if(tst->Prop().IsActive()==YSTRUE &&
				   ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT) // (ap->WhatItIs()==FsDogfight::ClassName))
				{
					FsDogfight *df=(FsDogfight *)ap;
					if(df->GetTarget(sim)==defendThis)
					{
						targetAirplaneKey=FsExistence::GetSearchKey(tst);
						target=tst;
						break;
					}
				}
			}
		}
	}




	if(target==NULL || target->Prop().IsActive()!=YSTRUE || target->iff==air.iff)
	{
		if(SearchTarget(air,sim)!=YSOK || (target=GetTarget(sim))==NULL)
		{
			goto NOTARGET;
		}
	}

	tpos=target->GetPosition();

	if((tpos-pos).GetSquareLength()>giveUpDist*giveUpDist)  // 20km
	{
		if(SearchTarget(air,sim)!=YSOK)
		{
			goto NOTARGET;
		}
		else
		{
			tpos=target->GetPosition();  // Update to newer one
		}
	}

	radar=atan2(sqrt(rel1.x()*rel1.x()+rel1.y()*rel1.y()),YsAbs(rel1.z()));


	if(DFMODE_STANDBY_GROUND/*100*/==mode || DFMODE_STANDBY_AIR/*101*/==mode)  // Wait until the target comes close
	{
		if(modeDuration>1.0)
		{
			SearchTarget(air,sim);
			modeDuration=0.0;
		}

		if((tpos-pos).GetSquareLength()<YsSqr(standBy))
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else
		{
			return YSOK;
		}
	}


	if(target!=NULL && DangerOfCollision(air,*target)==YSTRUE)
	{
		mode=DFMODE_AVOIDING_HEADON_COLLISION/*6*/;
	}
	else if(target!=NULL && air.GetPosition().y()<GetAllowableAltitude(33.0,air) && (DFMODE_TARGET_INFRONT/*2*/!=mode || radar>YsDegToRad(3.0)))
	{
		mode=DFMODE_TARGET_FLYINGLOW/*5*/;
	}


	switch(mode)
	{
	case DFMODE_NORMAL/*0*/:
		{
			// if(fmod(modeDuration,20.0)<=17.0)
			// {
			// 	gLimitCorrection=1.0;
			// }
			// else
			// {
			// 	gLimitCorrection=0.5;
			// }

			if(YSTRUE!=TargetIsWithinCombatRange(air,*target))
			{
				mode=DFMODE_CRUISE_TO_COMBAT_AREA;
			}
			else if(rel1.z()>0.0 && radar<YsDegToRad(30.0))
			{
				mode=DFMODE_TARGET_INFRONT/*2*/;  // Target is Right In Front
			}
			else if(rel1.z()<0.0 && radar<backSenseRange)
			{
				mode=DFMODE_TARGET_ONBACK_BREAK/*3*/;  // Target is on the back!!
				nextClock=clock+double(rand()%100)/100.0;
			}
			else if(modeDuration>20.0)
			{
				if(rand()%2==0)
				{
					mode=DFMODE_HIGH_G_YO_YO/*7*/;
				}
				else
				{
					mode=DFMODE_CLIMB_UP/*8*/;
				}
			}
			else if(BeingChasedByWingmansTarget(air,sim)==YSTRUE && air.GetAttitude().p()<YsPi/4.0)
			{
				jinkDesigBank=0.0;
				jinkNextBankChangeTime=0.0;
				mode=DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/;  // Decoy mode
			}
		}
		break;
	case DFMODE_TARGET_ONBACK/*1*/:
		{
			if(rel1.z()>0.0 && radar<YsDegToRad(30.0))
			{
				mode=DFMODE_TARGET_INFRONT/*2*/; // Eventually Got Target On The Scope!!
			}
			else if(rel1.z()>0.0 || radar>backSenseRange)
			{
				mode=DFMODE_NORMAL/*0*/;
			}
		}
		break;
	case DFMODE_TARGET_INFRONT/*2*/:
		{
			if((modeDuration>=20.0 && radar>YsDegToRad(40.0)) ||  // <- Dead lock
			    rel1.z()<0.0 || radar>YsDegToRad(50.0))  // <- The target is no longer in front of the airplane.
			{
				int d;
				d=rand()%4;
				switch(d)
				{
				case 0:
				case 1:
					mode=DFMODE_NORMAL/*0*/;
					break;
				case 2:
					mode=DFMODE_HIGH_G_YO_YO/*7*/;
					break;
				case 3:
					mode=DFMODE_CLIMB_UP/*8*/;
					break;
				}
			}
		}
		break;
	case DFMODE_TARGET_ONBACK_BREAK/*3*/:
		{
			if(nextClock<clock)
			{
				mode=DFMODE_TARGET_ONBACK/*1*/;
			}
		}
		break;
	case DFMODE_TARGET_INFRONT_FARAWAY/*4*/:
		{
			YsVec3 horizontalDist;
			horizontalDist=tpos-pos;
			horizontalDist.SetY(0.0);
			if(horizontalDist.GetSquareLength()<5000.0*5000.0 ||
			   YsAbs(air.GetAttitude().p())>YsDegToRad(45.0) ||
			   cos(air.GetAttitude().b())>0.94)
			{
				mode=DFMODE_TARGET_INFRONT/*2*/;
			}
		}
		break;
	case DFMODE_TARGET_FLYINGLOW/*5*/:
		{
			YsVec3 vel;
			air.Prop().GetVelocity(vel);
			if(air.GetAttitude().p()>YsDegToRad(30.0) || vel.y()>0.0)
			{
				if(rand()%3==1)
				{
					mode=DFMODE_HIGH_G_YO_YO/*7*/;
				}
				else
				{
					mode=DFMODE_NORMAL/*0*/;
				}
			}
		}
		break;
	case DFMODE_AVOIDING_HEADON_COLLISION/*6*/:
		{
			if(target==NULL)
			{
				mode=DFMODE_NORMAL/*0*/;
			}
			else
			{
				YsVec3 rel;
				rel=target->GetPosition()-air.GetPosition();
				air.GetAttitude().MulInverse(rel,rel);
				if(rel.z()<0.0)
				{
					mode=DFMODE_NORMAL/*0*/;
				}
			}
		}
		break;
	case DFMODE_HIGH_G_YO_YO/*7*/:
		{
			if(modeDuration>=10.0)
			{
				mode=DFMODE_NORMAL/*0*/;
			}
			else
			{
				YsVec3 h;
				YsAtt3 hAtt;
				h=target->GetPosition()-air.GetPosition();
				hAtt=air.GetAttitude();
				hAtt.SetP(0.0);
				hAtt.MulInverse(h,h);
				if(h.z()>0.0 && YsAbs(h.x())<h.z()/20.0)
				{
					mode=DFMODE_NORMAL/*0*/;
				}
			}
		}
		break;
	case DFMODE_CLIMB_UP/*8*/:
		{
			mode=DFMODE_NORMAL/*0*/;  // Nothing now
		}
		break;
	case DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/: // Act as decoy (level off)
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else if(fabs(air.GetAttitude().p())<YsPi/9.0)
		{
			mode=DFMODE_ACT_AS_DECOY_JINKING/*201*/;
		}
		break;
	case DFMODE_ACT_AS_DECOY_JINKING/*201*/: // Act as decoy
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else if(WingmanClosingIn(air,sim)==YSTRUE)
		{
			int a;
			a=rand();
			a&=255;
			if(a<64)
			{
				mode=DFMODE_ACT_AS_DECOY_BREAK/*202*/;
				jinkRollDir=1.0;
			}
			else if(a<128)
			{
				mode=DFMODE_ACT_AS_DECOY_BREAK/*202*/;
				jinkRollDir=-1.0;
			}
			else if(a<192)
			{
				mode=DFMODE_ACT_AS_DECOY_BARRELROLL_PITCHUP/*203*/;
				jinkRollState=0;
				jinkRollDir=1.0;
			}
			else
			{
				mode=DFMODE_ACT_AS_DECOY_BARRELROLL_PITCHUP/*203*/;
				jinkRollState=0;
				jinkRollDir=-1.0;
			}
		}
		break;
	case DFMODE_ACT_AS_DECOY_BREAK/*202*/:
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else if(modeDuration>15.0)
		{
			mode=DFMODE_ACT_AS_DECOY_JINKING/*201*/;
		}
		break;
	case DFMODE_ACT_AS_DECOY_BARRELROLL_PITCHUP/*203*/:
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else if(fabs(air.GetAttitude().b())<YsPi/18.0 && air.GetAttitude().p()>YsPi/18.0)
		{
			mode=DFMODE_ACT_AS_DECOY_BARRELROLL_ROLL/*204*/;
		}
		break;
	case DFMODE_ACT_AS_DECOY_BARRELROLL_ROLL/*204*/:
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else
		{
			if(jinkRollState==0 && fabs(air.GetAttitude().b())>YsPi/2.0)
			{
				jinkRollState=1;
			}
			else if(jinkRollState==1 && fabs(air.GetAttitude().b())<YsPi/18.0)
			{
				mode=DFMODE_ACT_AS_DECOY_JINKING/*201*/;
			}
		}
		break;
	case DFMODE_CRUISE_TO_COMBAT_AREA:
		{
			YsVec3 cruiseDestination=target->GetPosition();
			cruiseDestination.SetY(cruiseAlt);
			cruiseAP->SetSingleDestination(cruiseDestination);
			cruiseAP->speed=0.0;
			cruiseAP->throttle=1.0;
			cruiseAP->SetUseAfterburner(closeInMaxSpeed);
			cruiseAP->MakeDecision(air,sim,dt);

			nextTargetSearchTimer-=dt;
			if(YsTolerance>nextTargetSearchTimer)
			{
				nextTargetSearchTimer=1.0+(double)(rand()%100)/20.0;
				SearchTarget(air,sim);
			}
			if(YSTRUE==TargetIsWithinCombatRange(air,*target))
			{
				mode=DFMODE_NORMAL;
			}
		}
		break;
	}

	return YSOK;

NOTARGET:
	mode=DFMODE_NOTARGET/*-1*/;
	return YSOK;
}

FsAirplane *FsDogfight::GetWingmansTarget(FsAirplane &,FsSimulation *sim)
{
	FsAirplane *wingman;
	wingman=sim->FindAirplane(wingmanAirplaneKey);
	if(wingman!=NULL)
	{
		FsAutopilot *ap;
		ap=wingman->GetAutopilot();
		if(ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT)
		{
			FsDogfight *df;
			df=(FsDogfight *)ap;
			if(df->mode<DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/ || DFMODE_ACT_AS_DECOY_LASTRESERVED/*299*/<df->mode)
			{
				return df->GetTarget(sim);
			}
		}
	}
	return NULL;
}

YSBOOL FsDogfight::BeingChasedByWingmansTarget(FsAirplane &air,FsSimulation *sim)
{
	FsAirplane *wingman;
	wingman=sim->FindAirplane(wingmanAirplaneKey);
	if(wingman!=NULL)
	{
		FsAutopilot *ap;
		ap=wingman->GetAutopilot();
		if(ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT)
		{
			FsDogfight *df;
			FsAirplane *wingmanTarget;
			df=(FsDogfight *)ap;
			if(df->mode<DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/ || DFMODE_ACT_AS_DECOY_LASTRESERVED/*299*/<df->mode)
			{
				wingmanTarget=df->GetTarget(sim);
				if(wingmanTarget!=NULL && 
				   (wingmanTarget->GetPosition()-wingman->GetPosition()).GetSquareLength()<1200.0*1200.0)
				{
					YsVec3 relPosToWingmanTarget;
					relPosToWingmanTarget=air.GetPosition()-wingmanTarget->GetPosition();
					if(relPosToWingmanTarget.GetSquareLength()<600.0*600.0)
					{
						wingmanTarget->GetAttitude().MulInverse(relPosToWingmanTarget,relPosToWingmanTarget);
						if(relPosToWingmanTarget.z()>0.0 &&
						   fabs(relPosToWingmanTarget.x()/relPosToWingmanTarget.z())<0.577 &&
						   fabs(relPosToWingmanTarget.y()/relPosToWingmanTarget.z())<0.577)  // 30 degree frustum
						{
							return YSTRUE;
						}
					}
				}
			}
		}
	}
	return YSFALSE;
}

YSBOOL FsDogfight::WingmanClosingIn(FsAirplane &,FsSimulation *sim)
{
	FsAirplane *wingman;
	wingman=sim->FindAirplane(wingmanAirplaneKey);
	if(wingman!=NULL)
	{
		FsAutopilot *ap;
		ap=wingman->GetAutopilot();
		if(ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT)
		{
			FsDogfight *df;
			FsAirplane *wingmanTarget;
			df=(FsDogfight *)ap;
			wingmanTarget=df->GetTarget(sim);
			if(wingmanTarget!=NULL && 
			   (wingmanTarget->GetPosition()-wingman->GetPosition()).GetSquareLength()<400.0*400.0)
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

YSRESULT FsDogfight::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// char n[256];
	// air.Prop().GetIdentifier(n);
	// printf("DF: Mode %d %lf %s\n",mode,modeDuration,n);

	FsAirplane *target=GetTarget(sim);

	air.Prop().NeutralDirectAttitudeControl();

	if(air.isPlayingRecord!=YSTRUE)
	{
		air.Prop().SetVectorMarker(YSTRUE);
	}



	if(mode==DFMODE_STANDBY_AIR/*101*/)
	{
		air.Prop().BankController(YsPi/4.0);
		air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.8);
		air.Prop().SetGear(0.0);
		ControlGForVerticalSpeed(air,sim,0.0);
		return YSOK;
	}
	if(mode==DFMODE_STANDBY_GROUND /*100*/ || mode==DFMODE_STANDBY_GROUND_TIMER/*102*/)
	{
		air.Prop().TurnOffController();
		air.Prop().SetThrottle(0.0);
		air.Prop().SetElevator(0.0);
		air.Prop().SetAileron(0.0);
		air.Prop().SetRudder(0.0);
		return YSOK;
	}


	if(air.Prop().IsActive()==YSTRUE)
	{
		YSBOOL dontFire;
		FSWEAPONTYPE woc;

		dontFire=YSFALSE;
		woc=air.Prop().GetWeaponOfChoice();
		air.Prop().SetAllVirtualButton(YSFALSE);


		// 2005/04/01 >>
		FSWEAPONTYPE chasingWeaponType;
		YsVec3 chasingWeaponPos;
		if(air.Prop().GetNumWeapon(FSWEAPON_FLARE)>0 && 
		   flareClock<clock &&
		   sim->IsMissileChasing(chasingWeaponType,chasingWeaponPos,&air)==YSTRUE)
		{
			switch(chasingWeaponType) // Adapted from Pasutisu's code.
			{
				default:
					break;
				case FSWEAPON_AIM120:
					if((chasingWeaponPos-air.GetPosition()).GetSquareLength()<4000.0*4000.0)
					{
						air.Prop().SetDispenseFlareButton(YSTRUE);
						flareClock=clock+8.0;
					}
					break;
				case FSWEAPON_AIM9:
					if((chasingWeaponPos-air.GetPosition()).GetSquareLength()<2000.0*2000.0)
					{
						air.Prop().SetDispenseFlareButton(YSTRUE);
						flareClock=clock+4.0;
					}
					break;

				case FSWEAPON_AIM9X:
					if((chasingWeaponPos-air.GetPosition()).GetSquareLength()<1000.0*1000.0)
					{
						air.Prop().SetDispenseFlareButton(YSTRUE);
						flareClock=clock+2.0;
					}
					break;
			
			}
		}
		// 2005/04/01 <<

		switch(mode)
		{
		case DFMODE_NOTARGET/*-1*/:
			{
				CircleAround(air,sim,3000.0);
				minAlt=3000.0;
			}
			break;
		case DFMODE_TARGET_INFRONT_FARAWAY/*4*/:
			{
				air.Prop().GController(1.0);
				air.Prop().BankController(0.0);
			}
			break;
		case DFMODE_AVOIDING_HEADON_COLLISION/*6*/:
			{
				air.Prop().GController(gLimit);
				air.Prop().BankController(YsPi/2.0);
			}
			break;
		case DFMODE_TARGET_FLYINGLOW/*5*/:
			{
				if(air.GetAttitude().p()<YsDegToRad(-60.0))
				{
					air.Prop().TurnOffController();
					air.Prop().SetAileron(0.0);
					air.Prop().SetRudder(0.0);
					air.Prop().SpeedController(air.Prop().GetEstimatedLandingSpeed()*1.2);
					air.Prop().GController(gLimit);
				}
				else
				{
					YsAtt3 att;
					YsVec3 rel;
					double angleDeviation,wei;

					if(air.GetAttitude().p()<0.0)
					{
						wei=(air.GetPosition().y()-100.0)/100.0;
						wei=YsBound(wei,0.0,1.0);  // y<100:wei=0,  100<y<200:0<wei<1,  200<y:wei=1
					}
					else
					{
						wei=1.0;
					}

					att=air.Prop().GetAttitude();
					att.SetP(0.0);
					att.SetB(0.0);

					rel=target->GetPosition()-air.GetPosition();
					att.MulInverse(rel,rel);

					angleDeviation=YsBound(-atan2(rel.x(),rel.z()),-YsDegToRad(45.0),YsDegToRad(45.0));

					air.Prop().GController(gLimit);
					air.Prop().BankController(angleDeviation*wei);
					if(air.GetAttitude().p()>YsDegToRad(-30.0))
					{
						air.Prop().TurnOffSpeedController();
						air.Prop().SetThrottle(1.0);
						air.Prop().SetAfterburner(YSTRUE);
					}
				}
			}
			break;
		case DFMODE_HIGH_G_YO_YO/*7*/:  // Something like a high-G yo-yo
			{
				air.Prop().TurnOffController();
				air.Prop().SetThrottle(1.0);
				air.Prop().SetAfterburner(YSTRUE);
				air.Prop().SetSpoiler(0.0);
				if(YsAbs(air.GetAttitude().p())>YsDegToRad(70.0))
				{
					air.Prop().GController(gLimit);
					air.Prop().SetAileron(0.0);
				}
				else
				{
					double bnk;
					YsVec3 h;
					YsAtt3 hAtt;
					bnk=air.GetAttitude().p()*3.0;
					bnk=YsBound(bnk,YsPi/4.0,YsPi/2.0);

					hAtt=air.GetAttitude();
					hAtt.SetP(0.0);
					h=target->GetPosition()-air.GetPosition();
					hAtt.MulInverse(h,h);
					if(h.x()>0.0)
					{
						bnk=-bnk;
					}
					air.Prop().BankController(bnk);
				}
			}
			break;
		default: // mode==0,1,2 or 3
			{
				const YsVec3 *pos;
				YsVec3 tpos;
				const YsAtt3 *att,*tatt;
				YsVec3 tVel,tRVel;
				double tG;

				pos=&air.Prop().GetPosition();
				att=&air.Prop().GetAttitude();

				target->Prop().GetVelocity(tVel);
				air.Prop().GetInverseMatrix().Mul(tRVel,tVel,0.0);

				tpos=target->Prop().GetPosition();
				tatt=&target->Prop().GetAttitude();

				tG=target->Prop().GetG();

				double maxg;
				rel3=rel2;
				rel2=rel1;
				GetRelativePosition(rel1,tpos,air,sim);


				g3=g2;
				g2=g1;
				g1=air.Prop().GetG();

				// 9.0 at 150.0 m/s (300kt)
				// 4.0 at  75.0 m/s (150kt)
				maxg=4.0+5.0*(air.Prop().GetVelocity()-75.0)/75.0;
				if(maxg>gLimit*gLimitCorrection)
				{
					maxg=gLimit*gLimitCorrection;
				}
				else if(maxg<4.0)  // Note:maxg can be less than 4.0
				{
					maxg=4.0;
				}

				if(mode==DFMODE_TARGET_INFRONT/*2*/)// Limiting maxg when the target is not maneuvering rapidly, to avoid overcontrol
				{
					double t1,t2,t;
					t1=YsBound(3.0-tG,0.0,1.0);
					t2=YsBound(1.0-radar/YsDegToRad(30.0),0.0,1.0);
					t=t1*t2;
					maxg=3.0*t+maxg*(1.0-t);
				}

				if(mode==DFMODE_NORMAL/*0*/)
				{
					if(air.Prop().IsJet()!=YSTRUE || air.Prop().GetThrustWeightRatio()<0.9)
					{
						if(target->GetPosition().y()<air.GetPosition().y()-100.0)
						{
							FollowTarget(air,sim,target->GetPosition(),target->GetAttitude(),tVel,tG,dt,maxg,-3.0,YSFALSE);
						}
						else
						{
							ShallowPursuit(air,sim,target->GetPosition(),target->GetAttitude(),tVel,tG,dt,maxg,-3.0,YSFALSE);
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
							return YSOK;
						}
					}
					else
					{
						FollowTarget(air,sim,target->GetPosition(),target->GetAttitude(),tVel,tG,dt,maxg,-3.0,YSFALSE);
					}
					air.Prop().SetAirTargetKey(YSNULLHASHKEY);
				}
				else if(mode==DFMODE_TARGET_ONBACK/*1*/)
				{
					// The target is on the back
					air.Prop().TurnOffBankController();
					air.Prop().TurnOffPitchController();
					air.Prop().GController(maxg);
					air.Prop().SetAileron(0.0);
					air.Prop().SetRudder(0.0);

					air.Prop().SetAirTargetKey(YSNULLHASHKEY);
				}
				else if(mode==DFMODE_TARGET_INFRONT/*2*/)
				{
					YsVec3 hVec;
					double hDist;
					hVec=tpos-air.GetPosition();
					hVec.SetY(0.0);
					hDist=hVec.GetLength();

					if(hDist<farThreshold || air.Prop().GetNumWeapon(FSWEAPON_AIM120)>0) // 2005/03/26 AIM120 check added
					{
						double relAng,relSize;

						relAng=FollowTarget
						    (air,sim,target->GetPosition(),target->GetAttitude(),tVel,tG,dt,maxg,-3.0,YSTRUE);

						relSize=YsGreater(0.1,target->Prop().GetOutsideRadius()/rel1.z());

						if(0.0<rel1.z() && rel1.z()<400.0 && relAng<relSize && dontFire!=YSTRUE)
						{
							air.Prop().SetFireGunButton(YSTRUE);
						}

						YsVec3 tRFv;
						YsAtt3 tRAtt;
						double aim9Range,aim120Range,sqDist;
						FSWEAPONTYPE shortRangeType;

						if(0<air.Prop().GetNumWeapon(FSWEAPON_AIM9X))
						{
							shortRangeType=FSWEAPON_AIM9X;
						}
						else
						{
							shortRangeType=FSWEAPON_AIM9;
						}

						GetRelativeAttitude(tRAtt,air,sim);
						tRFv=tRAtt.GetForwardVector();
						tRFv.Normalize();
						aim9Range=air.Prop().GetAAMRange(shortRangeType);
						aim120Range=air.Prop().GetAAMRange(FSWEAPON_AIM120);

						sqDist=rel1.GetSquareLength();

						if(tRFv.z()>0.0 && sqDist<aim9Range*aim9Range)
						{
							FsExistence *target,*targetNew;
							target=sim->FindAirplane(air.Prop().GetAirTargetKey());
							air.Prop().SetWeaponOfChoice(shortRangeType);
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
							air.LockOn(sim,0.0);
							targetNew=sim->FindAirplane(air.Prop().GetAirTargetKey());

							double delay;
							if(targetNew!=NULL && targetNew->GetPosition().y()>=330.0)
							{
								delay=4.0;
							}
							else
							{
								delay=1.0;
							}

							if(targetNew!=NULL && targetNew!=target)
							{
								fireClock=clock+delay;
							}
							if(targetNew!=NULL && fireClock<clock)
							{
								int weaponId;
								YSBOOL blockedByBombBayDoor;
								weaponId=air.Prop().GetRecentlyFiredMissileId();
								if(sim->IsWeaponGuidedToTarget(weaponId)!=YSTRUE ||
								   sim->IsWeaponShotBy(weaponId,&air)!=YSTRUE)
								{
									air.Prop().FireWeapon
									   (blockedByBombBayDoor,sim,sim->GetClock(),sim->GetWeaponStore(),&air,shortRangeType);
									fireClock=clock+5.0;
								}
							}
						}
						else if((aim9Range*aim9Range)/9.0<sqDist && sqDist<aim120Range*aim120Range)
						{
							FsExistence *target,*targetNew;

							target=sim->FindAirplane(air.Prop().GetAirTargetKey());
							air.Prop().SetWeaponOfChoice(FSWEAPON_AIM120);
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
							air.LockOn(sim,0.0);
							targetNew=sim->FindAirplane(air.Prop().GetAirTargetKey());

							if(targetNew!=NULL && targetNew!=target)
							{
								fireClock=clock+3.5;
							}
							if(targetNew!=NULL && fireClock<clock)
							{
								YSBOOL blockedByBombBayDoor;
								air.Prop().FireWeapon
								   (blockedByBombBayDoor,sim,sim->GetClock(),sim->GetWeaponStore(),&air,FSWEAPON_AIM120);
								fireClock=clock+12.0;
							}
						}
						else
						{
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
						}
					}
					else
					{
						if(YsDegToRad(-70.0)<=air.GetAttitude().p() && air.GetAttitude().p()<YsDegToRad(70.0))
						{
							double v;
							v=air.Prop().GetVelocity();
							if(v>YsTolerance)
							{
								YsVec3 tVel,estTPos;
								double estTime,bnk;
								estTime=rel1.GetLength()/v;
								estTime=YsSmaller(estTime,5.0);
								target->Prop().GetVelocity(tVel);

								estTPos=tpos+tVel*estTime;

								double hErr;
								YsAtt3 hAtt;
								hAtt=air.GetAttitude();
								hAtt.SetP(0.0);
								hAtt.SetB(0.0);

								hAtt.MulInverse(estTPos,estTPos-air.GetPosition());
								hErr=atan2(-estTPos.x(),estTPos.z());

								bnk=YsBound(hErr*6.0,-YsPi/3.0,YsPi/3.0);
								air.Prop().BankController(bnk);
								ControlGForAltitude(air,sim,tpos.y(),tVel.y());
							}
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
						}
						else
						{
							YsVec3 uv;
							uv=tpos-air.GetPosition();
							air.Prop().BankController(uv);
							air.Prop().GController(gLimit);
						}
					}
				}
				else if(mode==DFMODE_TARGET_ONBACK_BREAK/*3*/)
				{
					// Begin Breaking Off
					air.Prop().GController(maxg);
					if(pos->y()>=2200.0)
					{
						air.Prop().TurnOffBankController();
						if(int(nextClock)%2==0)
						{
							air.Prop().SetAileron(0.1);
						}
						else
						{
							air.Prop().SetAileron(-0.1);
						}
					}
					else
					{
						if(int(nextClock)%2==0)
						{
							air.Prop().BankController(YsDegToRad(45.0));
						}
						else
						{
							air.Prop().BankController(YsDegToRad(-45.0));
						}
					}

					air.Prop().SetAirTargetKey(YSNULLHASHKEY);
				}

				YsVec3 ev,tev;
				ev=att->GetForwardVector();
				tev=tatt->GetForwardVector();

				if(mode>=10)
				{
					air.Prop().TurnOffSpeedController();
					air.Prop().SetThrottle(1.0);
					air.Prop().SetAfterburner(YSTRUE);
				}
				else if(rel1.z()<0.0 || ev*tev<0.0)
				{
// printf("Spd Feed:180.0\n");
					air.Prop().SpeedController(180.0);
				}
				else if(mode==DFMODE_TARGET_INFRONT/*2*/)
				{
					if(tev*ev>0.0)
					{
						double spdDeviation,relativeMotion;
						double err,k,b;
						double idealDist;

						if(air.Prop().IsJet()==YSTRUE)
						{
							idealDist=100.0+(double)(FsExistence::GetSearchKey(&air)%5)*20.0;
							k=5.0;
							b=30.0;
						}
						else
						{
							idealDist=80.0+(double)(FsExistence::GetSearchKey(&air)%5)*12.0;
							k=2.0;
							b=20.0;
						}

						relativeMotion=(rel1.z()-rel2.z())/dt;

						err=(rel1.z()-idealDist)/50.0;
						err=err*fabs(err)*50.0; // fabs to keep sign
						spdDeviation=err*k+relativeMotion*b;
						spdDeviation=YsBound <double> (spdDeviation,-50,100);
						air.Prop().SpeedController(target->Prop().GetVelocity()+spdDeviation);
					}
// printf("TgtSpd:%lf  Dist:%lf  Correction:%lf  Feed:%lf\n",
//     trg->Prop().GetVelocity(),rel1.z(),spdDeviation,trg->Prop().GetVelocity()+spdDeviation);
				}
				else
				{
					air.Prop().SpeedController(target->Prop().GetVelocity()+10.0);
// printf("Spd Feed:target+10.0\n");
				}
			}
			break;

		case DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/:
			{
				const double &p=air.GetAttitude().p();
				if(p>YsPi/3.0)
				{
					air.Prop().TurnOffBankController();
					air.Prop().SetAileron(0.0);
					air.Prop().GController(-2.2);
				}
				else if(p>0.0)
				{
					air.Prop().BankController(0.0);
					air.Prop().GController(-2.2);
				}
				else if(p<0.0)
				{
					air.Prop().BankController(0.0);
					air.Prop().GController(YsSmaller(gLimit,4.5));
				}
				else // if(p<-YsPi/3.0)
				{
					air.Prop().TurnOffBankController();
					air.Prop().SetAileron(0.0);
					air.Prop().GController(YsSmaller(gLimit,4.5));
				}

				double targetDist;
				FsAirplane *target;
				target=GetWingmansTarget(air,sim);
				targetDist=(target->GetPosition()-air.GetPosition()).GetLength();
				if(targetDist>300.0)
				{
					air.Prop().SpeedController(target->Prop().GetVelocity()-50.0);
				}
				if(targetDist>150.0)
				{
					air.Prop().SpeedController(target->Prop().GetVelocity()-10.0);
				}
				else if(targetDist<100.0)
				{
					air.Prop().SpeedController(target->Prop().GetVelocity()+30.0);
				}
				else
				{
					air.Prop().SpeedController(target->Prop().GetVelocity());
				}
			}
			break;
		case DFMODE_ACT_AS_DECOY_JINKING/*201*/:
			{
				FsAirplane *wingman;
				YsAtt3 flatAtt;
				YsVec3 pos;
				double wingmanRadial,targetRadial,targetDist;

				flatAtt=air.GetAttitude();
				flatAtt.SetB(0.0);
				flatAtt.SetP(0.0);

				targetRadial=0.0;
				targetDist=0.0;
				wingmanRadial=0.0;

				wingman=sim->FindAirplane(wingmanAirplaneKey);
				if(wingman!=NULL)
				{
					FsAutopilot *ap;
					ap=wingman->GetAutopilot();
					if(ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT)
					{
						FsDogfight *df;
						FsAirplane *wingmanTarget;
						df=(FsDogfight *)ap;
						if(df->mode<200 || 299<df->mode)
						{
							wingmanTarget=df->GetTarget(sim);
							if(wingmanTarget!=NULL)
							{
								pos=wingmanTarget->GetPosition();
								pos-=air.GetPosition();
								flatAtt.MulInverse(pos,pos);
								targetRadial=atan2(-pos.x(),pos.z());
								targetDist=-pos.z();
							}
						}
					}

					jinkNextBankChangeTime-=dt;
					if(jinkNextBankChangeTime<0.0)
					{
						pos=wingman->GetPosition();
						pos-=air.GetPosition();
						flatAtt.MulInverse(pos,pos);
						wingmanRadial=atan2(-pos.x(),pos.z());

						double maxBank;
						maxBank=acos(1/(gLimit-0.5));
						if(targetRadial*wingmanRadial>0.0)
						{
							jinkDesigBank=maxBank;
							if(targetRadial>0.0)
							{
								jinkDesigBank=-jinkDesigBank;
							}
						}
						else
						{
							jinkDesigBank=YsPi/6.0;
							if(targetRadial>0.0)
							{
								jinkDesigBank=-jinkDesigBank;
							}
						}
						jinkNextBankChangeTime=3.0+(double)(rand()%5);
					}

					air.Prop().BankController(jinkDesigBank);
					ControlGForVerticalSpeed(air,sim,(air.GetPosition().y()<3300.0 ? 5.0 : 0.0),gLimit);

					if(targetDist>300.0)
					{
						air.Prop().SpeedController(target->Prop().GetVelocity()-50.0);
					}
					if(targetDist>150.0)
					{
						air.Prop().SpeedController(target->Prop().GetVelocity()-10.0);
					}
					else if(targetDist<100.0)
					{
						air.Prop().SpeedController(target->Prop().GetVelocity()+30.0);
					}
					else
					{
						air.Prop().SpeedController(target->Prop().GetVelocity());
					}
				}
			}
			break;
		case DFMODE_ACT_AS_DECOY_BREAK/*202*/:
			{
				FsAirplane *target;
				double maxBank;
				maxBank=jinkRollDir*acos(1/(gLimit-0.5));

				air.Prop().BankController(maxBank);
				ControlGForVerticalSpeed(air,sim,(air.GetPosition().y()<3300.0 ? 5.0 : 0.0),gLimit);

				target=GetWingmansTarget(air,sim);
				if(target!=NULL)
				{
					air.Prop().SpeedController(target->Prop().GetVelocity());
				}
			}
			break;
		case DFMODE_ACT_AS_DECOY_BARRELROLL_PITCHUP/*203*/:
			{
				double gApply;
				gApply=air.Prop().GetG()+0.8;
				gApply=YsSmaller(gApply,gLimit);
				gApply=YsSmaller(gApply,6.0);

				air.Prop().BankController(0.0);
				if(fabs(air.GetAttitude().b())<YsPi/18.0)
				{
					air.Prop().GController(gApply);
				}
				else
				{
					ControlGForVerticalSpeed(air,sim,(air.GetPosition().y()<3300.0 ? 5.0 : 0.0),gLimit);
				}

				target=GetWingmansTarget(air,sim);
				if(target!=NULL)
				{
					air.Prop().SpeedController(target->Prop().GetVelocity());
				}
			}
			break;
		case DFMODE_ACT_AS_DECOY_BARRELROLL_ROLL/*204*/:
			{
				double ail;
				air.Prop().TurnOffBankController();
				air.Prop().GetRollRate(ail);
				ail=jinkRollDir*(YsPi/4.0)/ail;
				air.Prop().SetAileron(ail);

				double gApply;
				gApply=air.Prop().GetG()+0.8;
				gApply=YsSmaller(gApply,gLimit);
				gApply=YsSmaller(gApply,6.0);
				air.Prop().GController(gApply);

				target=GetWingmansTarget(air,sim);
				if(target!=NULL)
				{
					air.Prop().SpeedController(target->Prop().GetVelocity());
				}
			}
			break;
		case DFMODE_CRUISE_TO_COMBAT_AREA:
			{
				cruiseAP->ApplyControl(air,sim,dt);
			}
			break;
		}

		if(air.Prop().IsOnGround()==YSTRUE || air.GetPosition().y()<10.0)
		{
			if(air.Prop().IsOnGround()==YSTRUE && air.Prop().GetVelocity()<air.Prop().GetEstimatedLandingSpeed()*1.2)
			{
				air.Prop().TurnOffGController();
				air.Prop().SetElevator(0.0);
			}
			else
			{
				double tailStrikeAngle;
				tailStrikeAngle=air.Prop().GetTailStrikePitchAngle(0.8);
				air.Prop().SetGControllerAOALimit(0.0,tailStrikeAngle);
			}
			air.Prop().BankController(0.0);
		}
		else
		{
			air.Prop().SetGear(0.0);
		}
	}
	else
	{
		air.Prop().SetAirTargetKey(YSNULLHASHKEY);
	}

	return YSOK;
}

YSBOOL FsDogfight::TargetIsWithinCombatRange(const FsExistence &air,const FsExistence &target) const
{
	YsVec3 d=air.GetPosition()-target.GetPosition();
	if(d.GetSquareLengthXZ()<YsSqr(GetCombatRange()))
	{
		return YSTRUE;
	}
	return YSFALSE;
}

const double FsDogfight::GetCombatRange(void) const
{
	return combatThreshold;
}

