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
	lastDamageValue = 0.0;
	gLimit=9.0;
	backSenseRange=YsDegToRad(45.0);
	clock=0.0;
	nextClock=0.0;
	nextBreakClock = 0.0;
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

	//AI plane current position
	pos=air.Prop().GetPosition();

	FsAirplane *can;
	double min=0.0;
	can=NULL;
	while(NULL!=(can=sim->FindNextAirplane(can)))
	{
		//is the target valid? (target IFF check, target airplane state)
		if(YSTRUE==CanBeTarget(&air,can))
		{
			//tpos: current target's position
			tpos=can->GetPosition();

			//choose the closest target
			if(trg==NULL || (tpos-pos).GetSquareLength()<min)
			{
				trg=can;
				min=(tpos-pos).GetSquareLength();
			}
		}
	}

	if(NULL!=trg)
	{
		//valid target found - set mode accordingly
		if(mode==DFMODE_NOTARGET /*-1*/)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		targetAirplaneKey=FsExistence::GetSearchKey(trg);

		//rel1 = AI's relative position to the target aircraft
		GetRelativePosition(rel1,trg->GetPosition(),air,sim);
		rel2=rel1;
		rel3 = rel1;

		return YSOK;
	}
	else
	{
		//no target found
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

//check for emergency recovery condition(s) - stall or low altitude
YSRESULT FsDogfight::MakePriorityDecision(FsAirplane &air)
{
	return FsAutopilot::MakePriorityDecision(air);
}

YSRESULT FsDogfight::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	YsVec3 tpos;

	//if in standby mode, tick down the standby timer 
	if(mode==DFMODE_STANDBY_GROUND_TIMER/*102*/)  // Wait for standBy seconds.
	{
		standBy-=dt;

		//go to normal mode if standBy timer has lapsed
		if(standBy<YsTolerance)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else
		{
			return YSOK;
		}
	}

	//step timers
	clock+=dt;
	modeDuration+=dt;

	//reset the current mode timer if AI switched modes
	if(mode!=prevMode)
	{
		modeDuration=0.0;
		prevMode=mode;
	}


	//if AI has a pending target, set tgt key if it's alive and has valid IFF
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
	//ditto from above for pending wingman
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
	//ditto from above, but check pending target by name instead of ID
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


	//check if the current target is attacking the aircraft that the AI is defending
	FsAirplane *target=GetTarget(sim);
	FsAirplane *defendThis=sim->FindAirplane(defendThisAirplaneKey);
	if(defendThis!=NULL && target!=NULL)
	{
		YSBOOL targetOk=YSFALSE;

		FsAutopilot *ap=target->GetAutopilot();
		if(ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT) // ap->WhatItIs()==FsDogfight::ClassName)
		{
			//is the target airplane trying to attack a plane the AI is protecting?
			FsDogfight *df=(FsDogfight *)ap;
			if(df->GetTarget(sim)==defendThis)
			{
				targetOk=YSTRUE;
			}
		}
		// If none of the airplanes is chasing the defend target, don't need to update the target.
		//if the current target is not trying to attack the aircraft that the AI is defending, try to find a target that is attacking said defense aircraft
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



	//if we cannot find a valid target
	if(target==NULL || target->Prop().IsActive()!=YSTRUE || target->iff==air.iff)
	{
		if(SearchTarget(air,sim)!=YSOK || (target=GetTarget(sim))==NULL)
		{
			mode = DFMODE_NOTARGET;
			return YSOK;
		}
	}

	tpos=target->GetPosition();

	//if target is too far away, try to find a new target
	if((tpos-pos).GetSquareLength()>giveUpDist*giveUpDist)  // 20km
	{
		//if we still could not find a valid target
		if(SearchTarget(air,sim)!=YSOK)
		{
			mode = DFMODE_NOTARGET/*-1*/;
			return YSOK;
		}
		tpos=target->GetPosition();  // Update to newer one
	}

	double targetDist = (tpos - air.GetPosition()).GetLength();

	radar=atan2(sqrt(rel1.x()*rel1.x()+rel1.y()*rel1.y()),YsAbs(rel1.z()));

	//handle standby mode(s)
	if(DFMODE_STANDBY_GROUND/*100*/==mode || DFMODE_STANDBY_AIR/*101*/==mode)  // Wait until the target comes close
	{
		//get out of standby and begin a target search after modeDuration reaches a threshold
		if(modeDuration>1.0)
		{
			SearchTarget(air,sim);
			modeDuration=0.0;
		}

		//go to normal mode if target is close enough (standby used as distance in modes 100/101)
		if((tpos-pos).GetSquareLength()<YsSqr(standBy))
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else
		{
			//exit: target too far, continue in standby mode
			return YSOK;
		}
	}

	//avoid crashing into target 
	if(target!=NULL && DangerOfCollision(air,*target)==YSTRUE)
	{
		mode=DFMODE_AVOIDING_HEADON_COLLISION/*6*/;
		return YSOK; //crash avoidance shoudl take priority over other decisions, don't continue to decide
	}
	//check if the target is at low altitude based on altitude and radar angle
	else if(target!=NULL && air.GetPosition().y()<GetAllowableAltitude(33.0,air) && (DFMODE_TARGET_INFRONT/*2*/!=mode || radar>YsDegToRad(3.0)))
	{
		mode=DFMODE_TARGET_FLYINGLOW/*5*/;
	}

	FSWEAPONTYPE chasingWeaponType;
	YsVec3 chasingWeaponPos;

	//if being chased by a missile, evade 
	if (sim->IsMissileChasing(chasingWeaponType, chasingWeaponPos, &air))
	{
		if (nextBreakClock < clock)
		{
			mode = DFMODE_TARGET_ONBACK_BREAK/*3*/;
			UpdateBreakClocks(0.5, 1.5);
		}
		else
		{
			mode = DFMODE_TARGET_ONBACK;
		}
		return YSOK; //missile evasion should take priority over other decisions, don't continue to decide 
	}

	//if AI aircraft just took damage, evade
	double currDamageValue = air.Prop().GetDamageTolerance();
	if (fabs(currDamageValue - lastDamageValue) >= YsTolerance && mode != DFMODE_TARGET_ONBACK_BREAK)
	{
		lastDamageValue = currDamageValue;

		mode = DFMODE_TARGET_ONBACK_BREAK/*3*/;
		UpdateBreakClocks(0.25, 1.0);

		return YSOK; //threat evasion should take priority over other decisions, don't continue to decide 
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

			//if target is out of range, get within range
			if(YSTRUE!=TargetIsWithinCombatRange(air,*target))
			{
				mode=DFMODE_CRUISE_TO_COMBAT_AREA;
			}

			//check if target is in front of the AI's aircraft:
			//if target is within a 30 degree cone of AI aircraft's attitude: 
			//   /
			//  /
			// / 30 degrees
			//-----------------> AI aircraft attitude (relative Z axis)
			// \ 30 degreees
			//  \
			//   \

			else if(rel1.z()>0.0 && radar<YsDegToRad(30.0))
			{
				mode=DFMODE_TARGET_INFRONT/*2*/;  // Target is Right In Front
			}

			//check if target is behind the AI's aircraft:
			//if target is within a backSenseRange-degree cone of AI aircraft's rear attitude: 
			//   /
			//  /
			// / backSenseRange degrees
			//-----------------> AI aircraft rear attitude (relative Z axis, opposite dir)
			// \ backSenseRange degreees
			//  \
			//   \

			else if(rel1.z()<0.0 && radar < backSenseRange && targetDist <= 500.0)
			{
				if (nextBreakClock < clock)
				{
					mode = DFMODE_TARGET_ONBACK_BREAK/*3*/;  // Target is on the back!!

					//if someone is locked on or a missile is tailing, break more urgently
					if (sim->IsLockedOn(&air, YSTRUE) || sim->GetLockedOn(&air) != NULL)
					{
						UpdateBreakClocks(0.5, 1.5);
					}
					else
					{
						UpdateBreakClocks(0.1, 4.0);
					}
				}
				else
				{
					mode = DFMODE_TARGET_ONBACK;
				}
			}

			//if the target is not in front or behind the AI's aircraft and current mode has been active for a while:
			else if(modeDuration>5.0)
			{
				//50-50: perform a yo-yo or climb
				if(rand()%2==0)
				{
					mode=DFMODE_HIGH_G_YO_YO/*7*/;
				}
				else
				{
					mode=DFMODE_CLIMB_UP/*8*/;
				}
			}
			//if being attacked by wingman's target and AI aircraft's pitch is below 45 degrees, act as a decoy
			else if(BeingChasedByWingmansTarget(air,sim)==YSTRUE && air.GetAttitude().p()<YsPi/4.0)
			{
				jinkDesigBank=0.0;
				jinkNextBankChangeTime=0.0;
				mode=DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/;  // Decoy mode
			}
		}
		break;
	//current mode: target is behind AI's aircraft
	case DFMODE_TARGET_ONBACK/*1*/:
		{
			//target still on back: try to maneuver again 
			if (rel1.z() < 0.0 && nextBreakClock < clock)
			{
				//1 in 4 chance: yo-yo
				if (rand() % 4 == 0)
				{
					mode = DFMODE_HIGH_G_YO_YO;
				}
				//3 in 4 chance: break/bank again
				else
				{
					mode = DFMODE_TARGET_ONBACK_BREAK;
					UpdateBreakClocks(0.5, 4.0);
				}
			}
			//if the aircraft is now in front of us, set mode accordingly
			else if(rel1.z()>0.0 && radar<YsDegToRad(30.0))
			{
				mode=DFMODE_TARGET_INFRONT/*2*/; // Eventually Got Target On The Scope!!
			}
			//if target is not strictly in front or behind AI aircraft, return to normal mode
			else if(rel1.z()>0.0 && radar>=YsDegToRad(30.0))
			{
				mode=DFMODE_NORMAL/*0*/;
			}
		}
		break;
	//current mode: target is in front of AI's aircraft
	case DFMODE_TARGET_INFRONT/*2*/:
		{
			//if target has been in front for a while and is no longer strictly "in front" of AI's aircraft
			if((modeDuration>=20.0 && radar>YsDegToRad(40.0)) ||  // <- Dead lock
			    rel1.z()<0.0 || radar>YsDegToRad(50.0))  // <- The target is no longer in front of the airplane.
			{
				//randomly choose: go back to normal mode, climb, or yo-yo
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
	//current mode: target aircraft is far away in front of AI's aircraft
	case DFMODE_TARGET_INFRONT_FARAWAY/*4*/:
		{
			//calc horizontal distance from AI aircraft (XZ plane)
			YsVec3 horizontalDist;
			horizontalDist=tpos-pos;
			horizontalDist.SetY(0.0);

			//if target is <5000 m from AI aircraft OR AI's pitch/bank are above certain thresholds
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
			//get AI aircraft's current velocity
			YsVec3 vel;
			air.Prop().GetVelocity(vel);

			//if AI aircraft's pitch is above 30 deg or aircraft is climbing (positive Y vel)
			if(air.GetAttitude().p()>YsDegToRad(30.0) || vel.y()>0.0)
			{
				//randomly choose mode: yo-yo maneuver or go back to normal 
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
	//current mode: try not to crash into target
	case DFMODE_AVOIDING_HEADON_COLLISION/*6*/:
		{
			//nothing to avoid anymore, so go back to normal mode
			if(target==NULL)
			{
				mode=DFMODE_NORMAL/*0*/;
			}
			else
			{
				//no danger of collision, go back to normal mode
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
	//current mode: yo-yo maneuver
	case DFMODE_HIGH_G_YO_YO/*7*/:
		{
			//mode timer is stale, go back to normal mode
			if(modeDuration>=10.0)
			{
				mode=DFMODE_NORMAL/*0*/;
			}
			else
			{
				YsVec3 h;
				YsAtt3 hAtt;
				h=target->GetPosition()-air.GetPosition(); //h = target position relative to AI aircraft's position
				hAtt=air.GetAttitude();					   //hAtt = AI aircraft's attitude
				hAtt.SetP(0.0);							   //now, hAtt = XZ plane/component of AI aircraft's attidude (pitch angle zeroed out)
				hAtt.MulInverse(h,h);					   //rotate relative position vector by negative hAtt?

				if(h.z()>0.0 && YsAbs(h.x())<h.z()/20.0)
				{
					mode=DFMODE_NORMAL/*0*/;
				}
			}
		}
		break;
	//current mode: climbing - immediately go back to normal now?
	case DFMODE_CLIMB_UP/*8*/:
		{
			mode=DFMODE_NORMAL/*0*/;  // Nothing now
		}
		break;
	case DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/: // Act as decoy (level off)
		//if no longer being chased, go back to normal
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		//if pitch is less than +/- 20 degrees, transition to decoy/jinking mode
		else if(fabs(air.GetAttitude().p())<YsPi/9.0)
		{
			mode=DFMODE_ACT_AS_DECOY_JINKING/*201*/;
		}
		break;
	case DFMODE_ACT_AS_DECOY_JINKING/*201*/: // Act as decoy
		//if no longer being chased, go back to normal (stop being a decoy)
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		//if the wingman is closing in on their target
		else if(WingmanClosingIn(air,sim)==YSTRUE)
		{
			int a;
			a=rand();
			a&=255;
			//choose randomly:
			//	jink to either side 
			//	barrel roll up to either side
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
		//if no longer being chased, stop being a decoy (go to normal mode)
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		//if mode timer is stale, go back to jinking
		else if(modeDuration>15.0)
		{
			mode=DFMODE_ACT_AS_DECOY_JINKING/*201*/;
		}
		break;
	//current mode: decoy state, barrel roll pitch up
	case DFMODE_ACT_AS_DECOY_BARRELROLL_PITCHUP/*203*/:
		//if no longer being chased, stop being a decoy (go to normal mode)
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		//if bank angle and pitch angle are both less than +/- 10 degrees, begin roll
		else if(fabs(air.GetAttitude().b())<YsPi/18.0 && air.GetAttitude().p()>YsPi/18.0)
		{
			mode=DFMODE_ACT_AS_DECOY_BARRELROLL_ROLL/*204*/;
		}
		break;
	//current mode: decoy state, actively rolling
	case DFMODE_ACT_AS_DECOY_BARRELROLL_ROLL/*204*/:
		//if no longer being chased, stop being a decoy (go to normal mode)
		if(BeingChasedByWingmansTarget(air,sim)!=YSTRUE)
		{
			mode=DFMODE_NORMAL/*0*/;
		}
		else
		{
			//if not rolling and bank angle less than +/- 90 degrees, set roll state
			if(jinkRollState==0 && fabs(air.GetAttitude().b())>YsPi/2.0)
			{
				jinkRollState=1;
			}
			//if roll state set above and bank angle < +/- 10 degrees, go to jinking mode
			else if(jinkRollState==1 && fabs(air.GetAttitude().b())<YsPi/18.0)
			{
				mode=DFMODE_ACT_AS_DECOY_JINKING/*201*/;
			}
		}
		break;
	//current mode: move to target (too far away to engage)
	case DFMODE_CRUISE_TO_COMBAT_AREA:
		{
			//move to target's XZ position (override Y with cruising alt)
			YsVec3 cruiseDestination=target->GetPosition();
			cruiseDestination.SetY(cruiseAlt);

			//configure cruise autopilot
			cruiseAP->SetSingleDestination(cruiseDestination);
			cruiseAP->speed=0.0;
			cruiseAP->throttle=1.0;
			cruiseAP->SetUseAfterburner(closeInMaxSpeed);
			cruiseAP->MakeDecision(air,sim,dt);

			nextTargetSearchTimer-=dt;

			//if search timer expires: reset search timer and search for new target
			if(YsTolerance>nextTargetSearchTimer)
			{
				nextTargetSearchTimer=1.0+(double)(rand()%100)/20.0;
				SearchTarget(air,sim);
			}
			//if target is within range, exit cruise mode (go to normal mode)
			if(YSTRUE==TargetIsWithinCombatRange(air,*target))
			{
				mode=DFMODE_NORMAL;
			}
		}
		break;
	}

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
				//find wingman's current target (duplicate of above method GetWingmansTarget())
				wingmanTarget=df->GetTarget(sim);

				//if wingman's target is within 1200m of wingman
				if(wingmanTarget!=NULL && 
				   (wingmanTarget->GetPosition()-wingman->GetPosition()).GetSquareLength()<1200.0*1200.0)
				{
					YsVec3 relPosToWingmanTarget;
					relPosToWingmanTarget=air.GetPosition()-wingmanTarget->GetPosition();

					//if AI aircraft is within 600m of wingman's target
					if(relPosToWingmanTarget.GetSquareLength()<600.0*600.0)
					{
						wingmanTarget->GetAttitude().MulInverse(relPosToWingmanTarget,relPosToWingmanTarget);

						//check if the player's aircraft is within the wingman's 30 degree view frustum
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

			//if the wingman's target is < 400m from the wingman
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
	//printf("DF ApplyControl(): Mode %d %lf\n",mode,modeDuration);

	FsAirplane *target=GetTarget(sim);

	//reset pitch, yaw, roll controls
	air.Prop().NeutralDirectAttitudeControl();

	if(air.isPlayingRecord!=YSTRUE)
	{
		air.Prop().SetVectorMarker(YSTRUE);
	}


	//standby mode: bank 45 degrees, slow down 
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

	//if AI aircraft is active
	if(air.Prop().IsActive()==YSTRUE)
	{
		YSBOOL dontFire;
		FSWEAPONTYPE woc; //woc = "weapon of choice"

		dontFire=YSFALSE;
		woc=air.Prop().GetWeaponOfChoice();
		air.Prop().SetAllVirtualButton(YSFALSE);

		FsWeapon* seeker = sim->GetLockedOn(&air);

		//if the AI aircraft has flares AND the flare timer has lapsed AND there is a missile pursuing 
		if(air.Prop().GetNumWeapon(FSWEAPON_FLARE)>0 
			&& flareClock<clock 
			&& seeker != NULL)
		{
			//don't flare immediately on missile launch 
			double chasingWeaponDistTraveled = air.Prop().GetAAMRange(seeker->type) - seeker->lifeRemain;
			if (chasingWeaponDistTraveled >= 500.0)
			{
				//"chasing" = actively locked & pursuing the AI aircraft
				YsVec3 chasingWeaponPos = seeker->pos;
				FSWEAPONTYPE chasingWeaponType = seeker->type;

				double missileDist = (chasingWeaponPos - air.GetPosition()).GetLength();
				double flareClockStep = YsGreater(2.0, missileDist / 500);
				double randomStep = FsGetRandomBetween(0.0, 1.0);
				if (rand() % 2)
				{
					flareClockStep += randomStep;
				}
				else
				{
					flareClockStep -= randomStep;
				}
				printf("missile dist: %lf\n", missileDist);
				printf("flare clock step: %lf\n", flareClockStep);

				//dispense flare and reset flare timer based on missile distance to AI aircraft
				switch (chasingWeaponType) // Adapted from Pasutisu's code.
				{
				default:
					break;
				case FSWEAPON_AIM120:
					if ((chasingWeaponPos - air.GetPosition()).GetSquareLength() < 4000.0 * 4000.0)
					{
						air.Prop().SetDispenseFlareButton(YSTRUE);
						flareClock = clock + flareClockStep;
					}
					break;
				case FSWEAPON_AIM9:
					if ((chasingWeaponPos - air.GetPosition()).GetSquareLength() < 2000.0 * 2000.0)
					{
						air.Prop().SetDispenseFlareButton(YSTRUE);
						flareClock = clock + flareClockStep;
					}
					break;

				case FSWEAPON_AIM9X:
					if ((chasingWeaponPos - air.GetPosition()).GetSquareLength() < 1000.0 * 1000.0)
					{
						air.Prop().SetDispenseFlareButton(YSTRUE);
						flareClock = clock + flareClockStep;
					}
					break;

				}
			}
		}
		// 2005/04/01 <<

		//set plane controls based on current dogfight mode
		switch(mode)
		{
		//current mode: no target - idle in circle
		case DFMODE_NOTARGET/*-1*/:
			{
				CircleAround(air,sim,3000.0);
				minAlt=3000.0;
			}
			break;
		//current mode: target far in front of AI airctaft - go straight (1G, no bank)
		case DFMODE_TARGET_INFRONT_FARAWAY/*4*/:
			{
				air.Prop().GController(1.0);
				air.Prop().BankController(0.0);
			}
			break;
		//current mode: avoid collision: max G, 90 degree bank
		case DFMODE_AVOIDING_HEADON_COLLISION/*6*/:
			{
				air.Prop().GController(gLimit);
				air.Prop().BankController(YsPi/2.0);
			}
			break;
		case DFMODE_TARGET_FLYINGLOW/*5*/:
			{
				//if aircraft pitch is less than -60 degrees
				if(air.GetAttitude().p()<YsDegToRad(-60.0))
				{
					//reset controls, pull max G
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

					//determine weight based on aircraft pitch (zero/positive or negative)
					if(air.GetAttitude().p()<0.0)
					{
						wei=(air.GetPosition().y()-100.0)/100.0;
						wei=YsBound(wei,0.0,1.0);  // y<100:wei=0,  100<y<200:0<wei<1,  200<y:wei=1
					}
					else
					{
						wei=1.0;
					}

					//get AI aircraft's current heading (zero out pitch and bank angles)
					att=air.Prop().GetAttitude();
					att.SetP(0.0);
					att.SetB(0.0);

					//get target's position relative to AI aircraft
					rel=target->GetPosition()-air.GetPosition();

					//rotate target's relative position along XZ plane by negative aircraft heading 
					att.MulInverse(rel,rel);

					//determine angular deviation based on target's relative position (bind between +/- 45 degrees)
					angleDeviation=YsBound(-atan2(rel.x(),rel.z()),-YsDegToRad(45.0),YsDegToRad(45.0));

					//apply weighted angle deviation to bank input
					air.Prop().GController(gLimit);
					air.Prop().BankController(angleDeviation*wei);

					//if pitch is above -30 degrees, apply full throttle + afterburner
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
				//manual inputs: full throttle + afterburner, no flaps
				air.Prop().TurnOffController();
				air.Prop().SetThrottle(1.0);
				air.Prop().SetAfterburner(YSTRUE);
				air.Prop().SetSpoiler(0.0);

				//if pitch is above 70 degrees
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

					//bank = current pitch * 3, then bound between 45 and 90 degrees
					bnk=air.GetAttitude().p()*3.0;
					bnk=YsBound(bnk,YsPi/4.0,YsPi/2.0);

					hAtt=air.GetAttitude();
					hAtt.SetP(0.0);
					h=target->GetPosition()-air.GetPosition();
					hAtt.MulInverse(h,h);

					//invert bank angle based on relative position of target
					if(h.x()>0.0)
					{
						bnk=-bnk;
					}
					air.Prop().BankController(bnk);
				}
			}
			break;
		default: 
			// mode==0,1,2 or 3
			//  DFMODE_NORMAL = 0,
			//	DFMODE_TARGET_ONBACK = 1,
			//	DFMODE_TARGET_INFRONT = 2,
			//	DFMODE_TARGET_ONBACK_BREAK = 3,
			{
				const YsVec3 *pos;			//AI aircraft position
				YsVec3 tpos;				//target aircraft position
				const YsAtt3 *att,*tatt;	//AI and target aircraft attitudes, respectively
				YsVec3 tVel,tRVel;			//target velocities, absolute and relative to AI aircraft, respectively
				double tG;					//target Gs 

				//get AI aircraft position and attitude
				pos=&air.Prop().GetPosition();
				att=&air.Prop().GetAttitude();

				//get target velocity and relative velocity (via AI aircraft's inverse transform matrix)
				target->Prop().GetVelocity(tVel);
				air.Prop().GetInverseMatrix().Mul(tRVel,tVel,0.0);

				//get target aircraft position, attitude and Gs
				tpos=target->Prop().GetPosition();
				tatt=&target->Prop().GetAttitude();
				tG=target->Prop().GetG();

				double maxg;

				//update relative position history vars
				rel3=rel2;
				rel2=rel1;

				//store target's relative position in rel1
				GetRelativePosition(rel1,tpos,air,sim);

				//update G history vars (and current Gs)
				g3=g2;
				g2=g1;
				g1=air.Prop().GetG();

				//calculate max G based on current velocity (upper bound of gLimit*gLimitCorrection, lower bound of 4.0)
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
					//if the AI's aircraft is not a jet, or has a thrust-to-weight ratio below 0.9
					if(air.Prop().IsJet()!=YSTRUE || air.Prop().GetThrustWeightRatio()<0.9)
					{
						//if target aircraft altitude is 100 meters or more below the AI aircraft, simply follow
						if(target->GetPosition().y()<air.GetPosition().y()-100.0)
						{
							FollowTarget(air,sim,target->GetPosition(),target->GetAttitude(),tVel,tG,dt,maxg,-3.0,YSFALSE);
						}
						//otherwise, do a shallow pursuit (target aircraft too high to climb effectively?)
						else
						{
							ShallowPursuit(air,sim,target->GetPosition(),target->GetAttitude(),tVel,tG,dt,maxg,-3.0,YSFALSE);
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
							return YSOK;
						}
					}
					//if the AI aircraft is a jet with sufficient thrust-to-weight ratio, simply follow the target
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

					//if the target is close, try to cut speed and have them overshoot
					double targetDist = (target->GetPosition() - air.GetPosition()).GetLength();
					if (targetDist <= 500.0 
						&& (int)nextBreakClock % 2
						&& target->Prop().GetVelocity() * 0.6 >= air.Prop().GetMinimumManeuvableSpeed())
					{
						air.Prop().SpeedController(target->Prop().GetVelocity() * 0.6);
					}

					air.Prop().SetAirTargetKey(YSNULLHASHKEY);
				}
				else if(mode==DFMODE_TARGET_INFRONT/*2*/)
				{
					YsVec3 hVec;	//target position relative to AI aircraft
					double hDist;	//target XZ-plane distance relative to AI aircraft
					hVec=tpos-air.GetPosition();
					hVec.SetY(0.0);
					hDist=hVec.GetLength();

					//if target is within range or the AI aircraft has long range AAMs 
					if(hDist<farThreshold || air.Prop().GetNumWeapon(FSWEAPON_AIM120)>0) // 2005/03/26 AIM120 check added
					{
						double relAng,relSize;

						//follow target, retrieve relative angle for gun firing logic
						relAng=FollowTarget
						    (air,sim,target->GetPosition(),target->GetAttitude(),tVel,tG,dt,maxg,-3.0,YSTRUE);

						//relative size: scale target aircraft's physical size with relative Z distance
						relSize=YsGreater(0.1,target->Prop().GetOutsideRadius()/rel1.z());

						//gun firing conditions: 
						//	target is between 0 and 400m in front of AI aircraft
						//  relative angle is less than relative size (little to no deviation from boresight direction)
						//	"dontFire" flag is not set
						if(0.0<rel1.z() && rel1.z()<700.0 && relAng<relSize && dontFire!=YSTRUE)
						{
							air.Prop().SetFireGunButton(YSTRUE);
						}

						YsVec3 tRFv;	//target relative attitude forward vector (normalized)
						YsAtt3 tRAtt;   //target relative attitude
						double aim9Range,aim120Range,sqDist;
						FSWEAPONTYPE shortRangeType;

						//prefer AIM-9x over AIM-9
						if(0<air.Prop().GetNumWeapon(FSWEAPON_AIM9X))
						{
							shortRangeType=FSWEAPON_AIM9X;
						}
						else
						{
							shortRangeType=FSWEAPON_AIM9;
						}

						//get relative attitude and corresponding forward vector
						GetRelativeAttitude(tRAtt,air,sim);
						tRFv=tRAtt.GetForwardVector();
						tRFv.Normalize();

						//get AAM ranges
						aim9Range=air.Prop().GetAAMRange(shortRangeType);
						aim120Range=air.Prop().GetAAMRange(FSWEAPON_AIM120);

						//target relative squared distance
						sqDist=rel1.GetSquareLength();

						//if target is facing away and within AIM-9/9x range
						if(tRFv.z()>0.0 && sqDist<aim9Range*aim9Range)
						{
							FsExistence *target,*targetNew;
							target=sim->FindAirplane(air.Prop().GetAirTargetKey());
							air.Prop().SetWeaponOfChoice(shortRangeType);
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
							air.LockOn(sim,0.0);
							targetNew=sim->FindAirplane(air.Prop().GetAirTargetKey());

							//if target is farther than 330 meters, increase delay
							double delay;
							if(targetNew!=NULL && targetNew->GetPosition().y()>=330.0)
							{
								delay = FsGetRandomBetween(2.0, 6.0);
							}
							else
							{
								delay= FsGetRandomBetween(1.0, 3.0);
							}

							//if a valid new target was found, reset fire clock
							if(targetNew!=NULL && targetNew!=target)
							{
								fireClock=clock+delay;
							}

							//if a valid new target was found and fireClock has lapsed
							if(targetNew!=NULL && fireClock<clock)
							{
								int weaponId;
								YSBOOL blockedByBombBayDoor;
								weaponId=air.Prop().GetRecentlyFiredMissileId();

								//fire missile if not already fired or locked
								if(sim->IsWeaponGuidedToTarget(weaponId)!=YSTRUE ||
								   sim->IsWeaponShotBy(weaponId,&air)!=YSTRUE)
								{
									air.Prop().FireWeapon
									   (blockedByBombBayDoor,sim,sim->GetClock(),sim->GetWeaponStore(),&air,shortRangeType);
									fireClock=clock + FsGetRandomBetween(3.0, 7.0);
								}
							}
						}
						//check condition for firing long range AAM based on target squared distance
						else if((aim9Range*aim9Range)/9.0<sqDist && sqDist<aim120Range*aim120Range)
						{
							FsExistence *target,*targetNew;

							target=sim->FindAirplane(air.Prop().GetAirTargetKey());
							air.Prop().SetWeaponOfChoice(FSWEAPON_AIM120);
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
							air.LockOn(sim,0.0);
							targetNew=sim->FindAirplane(air.Prop().GetAirTargetKey());

							//if a valid new target was found, reset fire clock
							if(targetNew!=NULL && targetNew!=target)
							{
								fireClock=clock+(double)(rand() % 4);
							}

							//if a valid new target was found and fireClock has lapsed
							if(targetNew!=NULL && fireClock<clock)
							{
								//fire long range AAM and step fireClock by 12 
								YSBOOL blockedByBombBayDoor;
								air.Prop().FireWeapon
								   (blockedByBombBayDoor,sim,sim->GetClock(),sim->GetWeaponStore(),&air,FSWEAPON_AIM120);
								fireClock=clock + FsGetRandomBetween(10.0, 14.0);
							}
						}
						else
						{
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
						}
					}

					//target is not in range OR AI aircraft has no more long-range AAMs: move to target's estimated position
					else
					{
						//if AI aircraft pitch is between +/- 70 degrees
						if(YsDegToRad(-70.0)<=air.GetAttitude().p() && air.GetAttitude().p()<YsDegToRad(70.0))
						{
							double v;
							v=air.Prop().GetVelocity();
							if(v>YsTolerance) //approx. equivalent to if (v > 0.0)
							{
								//tVel: target velocity
								//estTPos: target's estimated position at estTime
								YsVec3 tVel,estTPos; 
								double estTime,bnk;

								//calculate estimated time (relative distance / speed)
								estTime=rel1.GetLength()/v;
								estTime=YsSmaller(estTime,5.0);
								target->Prop().GetVelocity(tVel);

								//estimate target position at estTime
								estTPos=tpos+tVel*estTime;

								double hErr;
								YsAtt3 hAtt;
								
								//get heading of AI aircraft (XZ plane - zero bank and pitch angles)
								hAtt=air.GetAttitude();
								hAtt.SetP(0.0);
								hAtt.SetB(0.0);

								//calculate target's relative position at estTime (accounting for AI aircraft's current heading)
								hAtt.MulInverse(estTPos,estTPos-air.GetPosition());

								//determine bank angle based on XZ ratio of estimated relative target position 
								hErr=atan2(-estTPos.x(),estTPos.z());

								//bank towards target's estimated position (bank angle bounded between +/- 60 degrees)
								bnk=YsBound(hErr*6.0,-YsPi/3.0,YsPi/3.0);
								air.Prop().BankController(bnk);
								ControlGForAltitude(air,sim,tpos.y(),tVel.y());
							}
							air.Prop().SetAirTargetKey(YSNULLHASHKEY);
						}

						//if aircraft's pitch is above 70 degrees or below -70 degrees: bank towards target relative position
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

					//if above 2200m: override bank controller, aileron left or right 
					if(pos->y()>=2200.0)
					{
						air.Prop().TurnOffBankController();

						double aileronStrength = FsGetRandomBetween(0.5, 1.0);
						if ((int)nextBreakClock % 2 == 0)
						{
							air.Prop().SetAileron(aileronStrength);
						}
						else
						{
							air.Prop().SetAileron(-aileronStrength);
						}
					}

					//if below 2200m: bank left or right 45 degrees
					else
					{
						if ((int)nextBreakClock % 2 == 0)
						{
							air.Prop().BankController(YsDegToRad(45));
						}
						else
						{
							air.Prop().BankController(YsDegToRad(45));
						}
					}

					air.Prop().SetAirTargetKey(YSNULLHASHKEY);
				}

				//ev: AI aircraft forward vector
				//tev: target aircraft forward vector
				YsVec3 ev,tev;
				ev=att->GetForwardVector();
				tev=tatt->GetForwardVector();

				//full throttle + afterburner
				if(mode>=10)
				{
					air.Prop().TurnOffSpeedController();
					air.Prop().SetThrottle(1.0);
					air.Prop().SetAfterburner(YSTRUE);
				}

				//target behind AI aircraft or not pursuing target: turn around
				else if(rel1.z()<0.0 || ev*tev<0.0)
				{
// printf("Spd Feed:180.0\n");
					air.Prop().SpeedController(180.0);
				}
				else if(mode==DFMODE_TARGET_INFRONT/*2*/)
				{
					//check that AI is pursuing the target (target not facing AI aircraft)
					//scale velocity based on target relative motion 
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
					//catch up to target 
					air.Prop().SpeedController(target->Prop().GetVelocity()+10.0);
// printf("Spd Feed:target+10.0\n");
				}
			}
			break;

		case DFMODE_ACT_AS_DECOY_LEVELOFF/*200*/:
			{
				const double &p=air.GetAttitude().p();
				if(p>YsPi/3.0) //pitch above 60 degrees
				{
					air.Prop().TurnOffBankController();
					air.Prop().SetAileron(0.0);
					air.Prop().GController(-2.2);
				}
				else if(p>0.0) //pitch between 0 and 60 degrees
				{
					air.Prop().BankController(0.0);
					air.Prop().GController(-2.2);
				}
				else if(p<0.0) //negative pitch
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

				//set speed based on distance to wingman's target
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
				YsAtt3 flatAtt; //AI aircraft heading (XZ plane of attitude)
				YsVec3 pos; //relative position of wingman's target
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

					//determine jink bank angle based on jink timer, position of target and wingman
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

					//scale velocity based on target dist
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
		//breaking: max bank and scale speed based on wingman's target's velocity
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
				//pull between 6 G's and G limit
				double gApply;
				gApply=air.Prop().GetG()+0.8;
				gApply=YsSmaller(gApply,gLimit);
				gApply=YsSmaller(gApply,6.0);

				air.Prop().BankController(0.0);

				//if bank is less than 10 degrees: start pulling Gs
				if(fabs(air.GetAttitude().b())<YsPi/18.0)
				{
					air.Prop().GController(gApply);
				}
				else
				{
					ControlGForVerticalSpeed(air,sim,(air.GetPosition().y()<3300.0 ? 5.0 : 0.0),gLimit);
				}

				//scale speed based on target's velocity
				target=GetWingmansTarget(air,sim);
				if(target!=NULL)
				{
					air.Prop().SpeedController(target->Prop().GetVelocity());
				}
			}
			break;
		case DFMODE_ACT_AS_DECOY_BARRELROLL_ROLL/*204*/:
			{
				//apply aileron control 
				double ail;
				air.Prop().TurnOffBankController();
				air.Prop().GetRollRate(ail);
				ail=jinkRollDir*(YsPi/4.0)/ail; //aileron: +/- 45 degrees / max rollrate
				air.Prop().SetAileron(ail);

				//pull between 6 G's and G limit
				double gApply;
				gApply=air.Prop().GetG()+0.8;
				gApply=YsSmaller(gApply,gLimit);
				gApply=YsSmaller(gApply,6.0);
				air.Prop().GController(gApply);

				//scale speed based on target's velocity
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

		//landing/gear logic
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

void FsDogfight::UpdateBreakClocks(double minDuration, double maxDuration)
{
	nextClock = clock + double(rand() % 100) / 100.0;
	nextBreakClock = clock + FsGetRandomBetween(minDuration, maxDuration);
}

