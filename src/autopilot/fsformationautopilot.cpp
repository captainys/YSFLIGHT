#include <ysclass.h>
#include <ysunitconv.h>
#include "fs.h"
#include "fsautopilot.h"



// Implementation //////////////////////////////////////////
FsFormation::FsFormation() : tooFarThreshold(3000.0),wayTooFarThreshold(6000.0)
{
	mode=FSFOMMODE_INPOSITION;
	tInPosition=0.0;

	nTransition=0;
	transitionSpeed=4.0;

	position=0;

	pendingLeaderId=-1;
	leader=NULL;
	shouldBe.Set(-9.0,-1.0,-11.0);
	spacing=YsOrigin();
	onceInTheAir=YSFALSE;
	inverted=YSFALSE;

	prevPathAngle=0.0;
	prevEHdg=0.0;
	prevX=0.0;
	prevEY=0.0;
	prevDEY=0.0;
	iEY=0.0;
	prevPch=0.0;
	prevZ=0.0;
	iZ=0.0;
	prevLZ=0.0;
	iLZ=0.0;

	synchronizeTrigger=YSTRUE;
	synchronizeLandingGear=YSTRUE;
	autoSpacingForLanding=YSTRUE;

	formationTakeOff=YSFALSE;
	takeoff=NULL;
	takeoffFromAlt=0.0;

	goToPosition=FsGotoPosition::Create();
}

FsFormation::~FsFormation()
{
	if(NULL!=takeoff)
	{
		FsAutopilot::Delete(takeoff);
		takeoff=NULL;
	}
	if(NULL!=goToPosition)
	{
		FsGotoPosition::Delete(goToPosition);
	}
}

/* static */ FsFormation *FsFormation::Create(void)
{
	return new FsFormation;
}

/* static */ const char *FsFormation::StateToStr(int fomMode)
{
	switch(fomMode)
	{
	default:
	case FSFOMMODE_INPOSITION:
		return "INPOSITION";
	case FSFOMMODE_LOSTPOSITION:
		return "LOSTPOSITION";
	case FSFOMMODE_SPEEDCONTROLONLY:
		return "SPEEDCONTROLONLY";
	case FSFOMMODE_NEEDTAKEOFF:
		return "NEEDTAKEOFF";
	case FSFOMMODE_TOOFAR:
		return "TOOFAR";
	case FSFOMMODE_TOOFAR_TOOHIGH:
		return "TOOFAR_TOOHIGH";
	case FSFOMMODE_RECOVER_FROM_LOWALT:
		return "RECOVER_FROM_LOWALT";
	}
}
/* static */ int FsFormation::StrToState(const char str[])
{
	if(0==strcmp(str,"INPOSITION"))
	{
		return FSFOMMODE_INPOSITION;
	}
	if(0==strcmp(str,"LOSTPOSITION"))
	{
		return FSFOMMODE_LOSTPOSITION;
	}
	if(0==strcmp(str,"SPEEDCONTROLONLY"))
	{
		return FSFOMMODE_SPEEDCONTROLONLY;
	}
	if(0==strcmp(str,"NEEDTAKEOFF"))
	{
		return FSFOMMODE_NEEDTAKEOFF;
	}
	if(0==strcmp(str,"TOOFAR"))
	{
		return FSFOMMODE_TOOFAR;
	}
	if(0==strcmp(str,"TOOFAR_TOOHIGH"))
	{
		return FSFOMMODE_TOOFAR_TOOHIGH;
	}
	if(0==strcmp(str,"RECOVER_FROM_LOWALT"))
	{
		return FSFOMMODE_RECOVER_FROM_LOWALT;
	}
	return FSFOMMODE_INPOSITION;
}

unsigned FsFormation::OverridedControl(void)
{
	if(mode==FSFOMMODE_SPEEDCONTROLONLY)
	{
		return FSAPPLYCONTROL_TRIGGER|FSAPPLYCONTROL_TURRET|FSAPPLYCONTROL_STICK|FSAPPLYCONTROL_NAVAID;
	}
	else // if(mode==FSFOMMODE_INPOSITION || mode==FSFOMMODE_LOSTPOSITION)
	{
		return FSAPPLYCONTROL_TRIGGER|FSAPPLYCONTROL_TURRET|FSAPPLYCONTROL_NAVAID;
	}
}

YSRESULT FsFormation::MakePriorityDecision(FsAirplane &)
{
	return YSOK;
}

YSRESULT FsFormation::SaveIntention(FILE *fp,const FsSimulation *sim)
{
	if(leader!=NULL)
	{
		fprintf(fp,"FORMATIO %d %.2lf %.2lf %.2lf\n",
		    sim->GetAirplaneIdFromHandle(leader),shouldBe.x(),shouldBe.y(),shouldBe.z());
	}
	else if(pendingLeaderId>=0)
	{
		fprintf(fp,"FORMATIO %d %.2lf %.2lf %.2lf\n",
		    pendingLeaderId,shouldBe.x(),shouldBe.y(),shouldBe.z());
	}
	else
	{
		fprintf(fp,"FORMATIO %d %.2lf %.2lf %.2lf\n",
		    -1,shouldBe.x(),shouldBe.y(),shouldBe.z());
	}

	fprintf(fp,"FOMSTATE %s\n",StateToStr(mode));

	int i;
	for(i=0; i<nTransition; i++)
	{
		fprintf(fp,"FOM_TRNS %.2lf %.2lf %.2lf\n",transition[i].x(),transition[i].y(),transition[i].z());
	}
	for(i=0; i<afterTransFormationAirplane.GetN(); i++)
	{
		if(afterTransFormationAirplane[i]>=0)
		{
			fprintf(fp,"FOM_AFTR %d %.2lf %.2lf %.2lf\n",
			   afterTransFormationAirplane[i],
			   afterTransFormationPos[i].x(),
			   afterTransFormationPos[i].y(),
			   afterTransFormationPos[i].z());
		}
	}

	fprintf(fp,"FOM_SYNC %s\n",FsTrueFalseString(synchronizeTrigger));


	return YSOK;
}

YSRESULT FsFormation::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
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
			if(0==strcmp(args[0],"FORMATIO"))
			{
				pendingLeaderId=atoi(args[1]);
				shouldBe.Set(atof(args[2]),atof(args[3]),atof(args[4]));
			}
			else if(0==strcmp(args[0],"FOM_TRNS"))
			{
				YsVec3 trns;
				trns.Set(atof(args[1]),atof(args[2]),atof(args[3]));
				transition[nTransition++]=trns;
			}
			else if(0==strcmp(args[0],"FOM_AFTR"))
			{
				YsVec3 pos;
				const int id=atoi(args[1]);
				pos.Set(atof(args[2]),atof(args[3]),atof(args[4]));
				afterTransFormationAirplane.Append(id);
				afterTransFormationPos.Append(pos);
			}
			else if(0==strcmp(args[0],"FOM_SYNC"))
			{
				FsGetBool(synchronizeTrigger,args[1]);
			}
			else if(0==strcmp(args[0],"FOMSTATE"))
			{
				if(2<=args.GetN())
				{
					mode=StrToState(args[1]);
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

void FsFormation::SetSpeedControlOnlyMode(void)
{
	mode=FSFOMMODE_SPEEDCONTROLONLY;
}

const double FsFormation::GetDefaultGLimit(FsAirplane &) const
{
	return 5.0;
}

const double FsFormation::GetMinimumAltitude(FsAirplane &air) const
{
	return air.GetFieldElevation()+YsUnitConv::FTtoM(100.0);
}

YSRESULT FsFormation::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
printf("%s %d %s\n",__FUNCTION__,__LINE__,StateToStr(mode));
	flareDispenser.MakeDecision(air,sim,dt);

	const double defGLimit=GetDefaultGLimit(air);;
	const double minAlt=GetMinimumAltitude(air);

	if(pendingLeaderId>=0)
	{
		leader=sim->GetAirplaneById(pendingLeaderId);
		pendingLeaderId=-1;
	}


	if(air.Prop().GetFlightState()==FSFLYING &&
	   air.GetAGL()>30.0)   // 2004/11/18 10.0->30.0 To be consistent with autoSpacingForLanding
	{
		onceInTheAir=YSTRUE;
	}

	if(YSTRUE==onceInTheAir &&
	   FSFOMMODE_INPOSITION!=mode && 
	  (minAlt>air.GetPosition().y() ||
	   YSTRUE==NeedImmediateRecoveryFromLowAltitude(air,minAlt,defGLimit-1.0)))
	{
		mode=FSFOMMODE_RECOVER_FROM_LOWALT;
	}

	if(leader!=NULL)
	{
		goal=GetGoalPosition();
		hDist=(goal-air.GetPosition()).GetLengthXZ();

		YsVec3 dev;
		air.Prop().GetMatrix().MulInverse(dev,goal,1.0);

printf("%lf\n",dev.GetLength());

		radar=atan2(sqrt(dev.x()*dev.x()+dev.y()*dev.y()),YsAbs(dev.z()));

		const YsAtt3 *myAtt,*leadAtt;
		myAtt=&air.GetAttitude();
		leadAtt=&leader->GetAttitude();

		YsVec3 myEv,leadEv;
		myEv=myAtt->GetForwardVector();
		leadEv=leadAtt->GetForwardVector();

		// printf("DEV %lf\n",dev.GetLength());


		if(YSTRUE!=formationTakeOff &&
		   YSTRUE==leader->Prop().IsOnGround() &&
		   YSTRUE==air.Prop().IsOnGround() &&
		   dev.GetSquareLength()<800.0*800.0)
		{
			formationTakeOff=YSTRUE;
			printf("Set for formation take off\n");
		}


		// Formation transition
		if(dev.GetSquareLength()<6.25)  // within 2.5m radius
		{
			tInPosition+=dt;
		}
		else
		{
			tInPosition=0.0;
		}

		if(tInPosition>=1.0 && nTransition>0)
		{
			shouldBe=transition[0];

			int i;
			tInPosition=0.0;
			for(i=0; i<nTransition-1; i++)
			{
				transition[i]=transition[i+1];
			}
			nTransition--;

			if(nTransition==0)
			{
				for(i=0; i<afterTransFormationAirplane.GetN(); i++)
				{
					FsAirplane *air;
					air=sim->GetAirplaneById(afterTransFormationAirplane[i]);
					if(air!=NULL)
					{
						FsAutopilot *ap;
						FsFormation *fom;
						ap=air->GetAutopilot();
						if(ap->Type()==FSAUTOPILOT_FORMATION) // strcmp(ap->WhatItIs(),FsFormation::ClassName)==0)
						{
							fom=(FsFormation *)ap;
							fom->shouldBe=afterTransFormationPos[i];
						}
					}
				}
			}
		}


		if(YSTRUE==air.Prop().IsOnGround() && (mode==FSFOMMODE_TOOFAR || mode==FSFOMMODE_TOOFAR_TOOHIGH))
		{
			mode=FSFOMMODE_INPOSITION;
			yMin=0.0;
		}
		else if(3000.0<hDist && YSTRUE!=air.Prop().IsOnGround())
		{
			yMin=YsSmaller((hDist-3000.0)*tan(YsPi/36.0),YsUnitConv::FTtoM(17000.0));
			if(FSFOMMODE_TOOFAR_TOOHIGH!=mode &&
			   YsPi/6.0<air.GetAttitude().p() &&
			   YsUnitConv::FTtoM(500.0/60.0)<air.Prop().GetClimbRatio() &&
			   YsGreater(goal.y(),yMin)+YsUnitConv::FTtoM(100.0)<air.GetPosition().y())
			{
				mode=FSFOMMODE_TOOFAR_TOOHIGH;
			}
			else if(FSFOMMODE_TOOFAR_TOOHIGH==mode &&
			        (-YsPi/18.0>air.GetAttitude().p() ||
			         YsUnitConv::FTtoM(-500.0/60.0)>air.Prop().GetClimbRatio() ||
			         YsGreater(goal.y(),yMin)>air.GetPosition().y()))
			{
				mode=FSFOMMODE_TOOFAR;
			}
			else if(FSFOMMODE_TOOFAR!=mode && FSFOMMODE_TOOFAR_TOOHIGH!=mode)
			{
				mode=FSFOMMODE_TOOFAR;
			}
		}
		else if(YSTRUE!=air.Prop().IsOnGround())
		{
			if(mode==FSFOMMODE_TOOFAR || mode==FSFOMMODE_TOOFAR_TOOHIGH)
			{
				mode=FSFOMMODE_INPOSITION;
			}
			yMin=0.0;
		}


		if(mode==FSFOMMODE_INPOSITION)
		{
			if(air.Prop().IsOnGround()==YSTRUE && YSTRUE==onceInTheAir)
			{
				// Just keep mode0
			}
			else if(YSTRUE==air.Prop().IsOnGround() && YSTRUE!=onceInTheAir && YSTRUE!=formationTakeOff)
			{
				mode=FSFOMMODE_NEEDTAKEOFF;
				if(NULL==takeoff)
				{
					takeoff=FsTakeOffAutopilot::Create();
				}
				takeoffFromAlt=air.Prop().GetPosition().y();
			}
			else if(myEv*leadEv<cos(YsDegToRad(45.0)) || (dev.GetSquareLength()>80.0*80.0 && radar>YsPi/6.0))
			    // Note on the second mode-changing criterion:
			    //   Distance itself is not good for formation landing.  After landing,
			    //   the distance of this airplane and the formation leader may become
			    //   large.  If mode is switched 1 due to large distance, it will launch
			    //   this airplane again to the air and may end up crashing into graound.
			{
				prevZ=0.0;
				iZ=0.0;
				prevLZ=0.0;
				iLZ=0.0;
				mode=FSFOMMODE_LOSTPOSITION;
			}
		}
		else if(mode==FSFOMMODE_LOSTPOSITION)
		{
			if(radar<YsDegToRad(25.0) && dev.GetSquareLength()<70.0*70.0)
			{
				mode=FSFOMMODE_INPOSITION;
				prevPathAngle=0.0;
				prevEHdg=0.0;
				prevX=0.0;
				prevEY=0.0;
				prevDEY=0.0;
				iEY=0.0;
				prevPch=0.0;
				//prevTheataK=0.0;
				//iTheataK=0.0;
				prevZ=0.0;
				iZ=0.0;
				prevLZ=0.0;
				iLZ=0.0;
			}
		}
		else if(mode==FSFOMMODE_NEEDTAKEOFF)
		{
			if(NULL!=takeoff)
			{
				takeoff->MakeDecision(air,sim,dt);
			}
			if(takeoffFromAlt+152<=air.Prop().GetPosition().y())
			{
				prevZ=0.0;
				iZ=0.0;
				prevLZ=0.0;
				iLZ=0.0;
				mode=FSFOMMODE_LOSTPOSITION;
			}
		}
		else if(FSFOMMODE_TOOFAR==mode || FSFOMMODE_TOOFAR_TOOHIGH==mode)
		{
			YsAtt3 att=air.GetAttitude();
			att.SetP(0.0);
			att.SetB(0.0);

			YsVec3 rel=goal-air.GetPosition();
			att.MulInverse(rel,rel);

			bearing=atan2(-rel.x(),rel.z());
		}
		else if(FSFOMMODE_RECOVER_FROM_LOWALT==mode)
		{
			if(minAlt+YsUnitConv::FTtoM(200.0)<air.GetPosition().y() &&
			   0.0<air.Prop().GetClimbRatio())
			{
				mode=FSFOMMODE_LOSTPOSITION;
			}
		}
	}
	else
	{
		goal=YsOrigin();
		hDist=0.0;
		radar=0.0;
	}
	return YSOK;
}

YSRESULT FsFormation::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	flareDispenser.ApplyControl(air,sim,dt);

// printf("%s %d %s\n",__FUNCTION__,__LINE__,StateToStr(mode));
	const double defGLimit=GetDefaultGLimit(air);;

	air.Prop().NeutralDirectAttitudeControl();

	if(leader!=NULL)
	{
		if(leader->Prop().GetVelocity()<=0.5 && air.Prop().GetVelocity()<=0.5)
		{
			air.Prop().SetBrake(1.0);
			air.Prop().SpeedController(0.0);
			air.Prop().SetRudder(0.0);
			air.Prop().SmartRudder(dt);
			return YSOK;
		}

		if(onceInTheAir!=YSTRUE ||                 // 2004/11/18
		   air.GetAGL()>30.0 ||
		   leader->Prop().GetLandingGear()<0.5 ||  // 2002/11/21 Don't take spacing if leader's ldg is up.
		   autoSpacingForLanding!=YSTRUE)          // 2003/01/21 Let a program choose if it take space or not.
		{
			spacing=YsOrigin();
		}
		else
		{
			// Spacing for landing
			double k;
			k=(30.0-air.GetAGL())/30.0;
			spacing.Set(0.0,shouldBe.y()*k,shouldBe.z()*k);
		}


		// Synchronizing Gear,Flap and Brake (Must come after SpeedControl)
		if(synchronizeLandingGear==YSTRUE)  // 2005/06/22
		{
			air.Prop().SetGear(leader->Prop().GetLandingGear());
		}
		air.Prop().SetFlap(leader->Prop().GetFlap());
		air.Prop().SetSpoiler(leader->Prop().GetSpoiler());
		if(leader->Prop().GetBrake()==YSTRUE)
		{
			air.Prop().SetBrake(1.0);
		}
		else
		{
			air.Prop().SetBrake(0.0);
		}


		if(synchronizeTrigger==YSTRUE)
		{
			air.Prop().SetFireWeaponButton(leader->Prop().GetFireWeaponButton());
			air.Prop().SetFireGunButton(leader->Prop().GetFireGunButton());
			air.Prop().SetFireAAMButton(leader->Prop().GetFireAAMButton());
			air.Prop().SetFireAGMButton(leader->Prop().GetFireAGMButton());
			air.Prop().SetFireRocketButton(leader->Prop().GetFireRocketButton());
			air.Prop().SetDropBombButton(leader->Prop().GetDropBombButton());
			air.Prop().SetDispenseFlareButton(leader->Prop().GetDispenseFlareButton());
			air.Prop().SetCycleWeaponButton(YSFALSE);
			air.Prop().SetSmokeButton(leader->Prop().GetSmokeButton());

			air.Prop().SetWeaponOfChoice(leader->Prop().GetWeaponOfChoice());

			// // Trigger is set here. But, if player is in command,
			// // these trigger will be overridden in FsSimulation::SimControlByUser
			// YSBOOL pTrig,trig;
			// air.Prop().SetWeaponOfChoice(leader->Prop().GetWeaponOfChoice());
			// pTrig=air.Prop().GetTriggerState(0);
			// air.Prop().SetTrigger(0,leader->Prop().GetTriggerState(0));
			// trig=air.Prop().GetTriggerState(0);
			// if(pTrig!=YSTRUE && trig==YSTRUE)
			// {
			// 	air.Prop().FireSelectedWeapon(sim->GetClock(),sim->GetWeaponStore(),&air);
			// }  // <- 2003/06/07  Why did I explicitly fire weapon here?  I forgot....
		}

		const double lG=leader->Prop().GetG();
		const double maxG=YsBound(lG+2.0,5.0,9.0);

		// Stick Control
		if(FSFOMMODE_INPOSITION==mode)  // Almost In Position
		{
			Mode0LateralControl(air,*leader,dt);
			Mode0VerticalControl(air,*leader,dt);

			if(air.Prop().IsOnGround()==YSTRUE)
			{
				YsVec3 leaderEv;
				leaderEv=leader->GetAttitude().GetForwardVector();
				air.GetAttitude().MulInverse(leaderEv,leaderEv);

				if(onceInTheAir==YSTRUE)
				{
					air.Prop().SetRudder(-leaderEv.x());

					/* air.Prop().SetBrake(1.0);
					air.Prop().SpeedController(0.0);
					air.Prop().SmartRudder(dt);

					double tailStrikeAngle;
					tailStrikeAngle=air.Prop().GetTailStrikePitchAngle(0.8);
					air.Prop().GController(0.8);
					air.Prop().SetGControllerAOALimit(0.0,tailStrikeAngle); */
					DecelerationAfterLanding(air,sim,dt);
				}
				else
				{
					air.Prop().SetRudder(0.0);
					air.Prop().SmartRudder(dt);
					Mode0SpeedControl(air,*leader,dt,0.0,YSFALSE); // YSFALSE:No low-speed limitter
				}
			}
			else
			{
				Mode0SpeedControl(air,*leader,dt,0.0,YSFALSE); // YSFALSE: No low-speed limitter

				air.Prop().SetRudder(0.0);
				air.Prop().SmartRudder(dt);
			}
		}
		else if(FSFOMMODE_LOSTPOSITION==mode)  // Hey, I lost the position;
		{
			YsVec3 destination,leadV;
			YsVec3 aimingPoint;

			aimingPoint.Set(shouldBe.x(),shouldBe.y(),shouldBe.z()+50.0);
			destination=leader->Prop().GetMatrix()*aimingPoint;

			leader->Prop().GetVelocity(leadV);
			FollowTarget(air,sim,destination,leader->GetAttitude(),leadV,lG,dt,maxG,-3.0,YSFALSE);

			// In fact, it seems to make more sense to keep faster speed.
			// Mode0SpeedControl(air,*leader,dt,-40.0,YSTRUE); // YSTRUE: Use low-speed limitter

			// Experiment showed it is good if Cd Spoiler is 2.5 (F-18E)
			// (d-20)/5.0
			//     20m back    0.0
			//    120m back  +20.0
			//    270m back  +50.0
			// What about F-4E (Cd Spoiler is 1.0)?
			// (d-35)/10.0?

			const double CdSpoiler=air.Prop().GetCdSpoiler();
			const double backOff=20.0; // YsBound(35.0-10.0*(CdSpoiler-1.0),20.0,40.0);
			//  1.0->35  
			//  2.5->20  35.0-(CdSpoiler-1.0)*10.0,  or is 20m just fine?
			const double denomi=YsBound(11.0-4.0*(CdSpoiler-1.0),5.0,20.0);
			//  1.0->11.0
			//  2.5->5.0

			const double d=(goal-air.GetPosition()).GetLength();
			const double vCorrection=YsBound((d-backOff)/denomi,0.0,50.0);
			air.Prop().SpeedController(leader->Prop().GetVelocity()+vCorrection);

			air.Prop().SetRudder(0.0);
			air.Prop().SmartRudder(dt);
		}
		else if(FSFOMMODE_SPEEDCONTROLONLY==mode)
		{
			air.Prop().TurnOffBankController();
			air.Prop().TurnOffGController();
			air.Prop().TurnOffPitchController();
		}
		else if(FSFOMMODE_NEEDTAKEOFF==mode)
		{
			if(NULL!=takeoff)
			{
				takeoff->ApplyControl(air,sim,dt);
			}
		}
		else if(FSFOMMODE_TOOFAR==mode)
		{
			// 1.0/cos(bank)=maxG
			// Take 0.5G margine =>  1.0/cos(bank)=maxG-0.5
			// cos(bank)=1.0/(maxG-0.5)
			const double maxBank=acos(1.0/(maxG-0.5));
			const double bank=YsBound(bearing*5.0,-maxBank,maxBank);

			air.Prop().BankController(bank);
			ControlGForAltitude(air,sim,YsGreater(yMin,goal.y()),0.0,maxG);
			air.Prop().SpeedController(leader->Prop().GetVelocity()+100.0);

			air.Prop().SetRudder(0.0);
			air.Prop().SmartRudder(dt);
		}
		else if(FSFOMMODE_TOOFAR_TOOHIGH==mode)
		{
			const double bnkDiff=YsBound(bearing*5.0,YsDegToRad(-80.0),YsDegToRad(80.0));
			const double bnk=YsPi-bnkDiff;

			air.Prop().BankController(bnk);
			air.Prop().GController(maxG);
			air.Prop().SpeedController(leader->Prop().GetVelocity()+100.0);

			air.Prop().SetRudder(0.0);
			air.Prop().SmartRudder(dt);
		}
		else if(FSFOMMODE_RECOVER_FROM_LOWALT==mode)
		{
			air.Prop().BankController(0.0);

			if(-YsPi*8.0/18.0<air.GetAttitude().b() && air.GetAttitude().b()<YsPi*8.0/18.0)
			{
				air.Prop().GController(defGLimit);
			}
			else if(air.GetAttitude().b()<-YsPi*11.0/18.0 || YsPi*11.0/18.0<air.GetAttitude().b())
			{
				air.Prop().GController(-1.0);
			}
			else
			{
				air.Prop().GController(0.0);
			}

			YsVec3 vel;
			air.Prop().GetVelocity(vel);
			if(vel.y()<0.0)
			{
				air.Prop().SetThrottle(0.0);
				air.Prop().SetAfterburner(YSFALSE);
				air.Prop().SetBrake(1.0);
			}
			else
			{
				air.Prop().SetThrottle(1.0);
				air.Prop().SetAfterburner(YSTRUE);
				air.Prop().SetBrake(0.0);
			}
		}

		return YSOK;
	}
	return YSERR;
}

void FsFormation::Mode0SpeedControl(FsAirplane &wingman,FsAirplane &leader,const double &dt,const double &Zoffset,YSBOOL lowSpeedLimitter)
{
	YsVec3 aimingPoint;
	YsAtt3 wingmanAtt;
	YsVec3 wingmanEv,wingmanUv,leaderVel;
	double Z,dZ;
	double leaderSpd;

	wingman.Prop().GetVelocity(wingmanEv);
	if(wingmanEv==YsOrigin())                                // 2003/05/21
	{                                                        // 2003/05/21
		wingmanEv=wingman.GetAttitude().GetForwardVector();  // 2003/05/21
	}                                                        // 2003/05/21
	wingmanUv=wingman.GetAttitude().GetUpVector();
	wingmanAtt.SetTwoVector(wingmanEv,wingmanUv);

	aimingPoint=shouldBe+spacing;
	aimingPoint.AddZ(Zoffset);
	leader.Prop().GetMatrix().Mul(aimingPoint,aimingPoint,1.0);

	aimingPoint-=wingman.GetPosition();
	wingmanAtt.MulInverse(aimingPoint,aimingPoint);

	Z=aimingPoint.z();
	iZ=YsBound(iZ+Z*dt,-3.0,3.0);
	dZ=(Z-prevZ)/dt;

	// printf("Z % 8.2lf DZ % 8.2lf IZ % 8.2lf\n",Z,dZ,iZ);

	leader.Prop().GetVelocity(leaderVel);
	wingmanAtt.MulInverse(leaderVel,leaderVel);
	leaderSpd=leaderVel.z();

	double spd1;
	const double Kp=1.0/1.0;   // 1.0m corresponds to 2kt, 100m/s=200kt, 1kt=0.5m/s
	const double Kd=10.0;      // If the leader airplane is going away (z-plus)
	                           // at 1m/s, counter it by 2m/s
	const double Ki=1.0/0.3;   // If 0.3m difference remained for 1 second,
	                           // apply 1m/s forward.
	spd1=Kp*Z+Ki*iZ+Kd*dZ;
	spd1=YsBound(spd1,-100.0,100.0);  // Don't give inappropriate speed, like 8km per seconds!
	spd1=leaderSpd+spd1;
	prevZ=Z;

	double spd2;
	YsVec3 wingmanPos;
	double LZ,dLZ;
	aimingPoint=shouldBe+spacing;
	aimingPoint.AddZ(Zoffset);
	leader.Prop().GetMatrix().MulInverse(wingmanPos,wingman.GetPosition(),1.0);
	LZ=aimingPoint.z()-wingmanPos.z();
	dLZ=(LZ-prevLZ)/dt;
	iLZ=YsBound(iLZ+LZ*dt,-3.0,3.0);
	spd2=Kp*LZ+Ki*iLZ+Kd*dLZ;
	spd2=YsBound(spd2,-100.0,100.0);  // Don't give inappropriate speed, like 8km per seconds!
	spd2=leaderSpd+spd2;
	prevLZ=LZ;

	double w2;
	YsVec3 Vl,Va;
	leader.Prop().GetVelocity(Vl);
	wingman.Prop().GetVelocity(Va);
	Vl.Normalize();
	Va.Normalize();
	w2=YsBound(((Vl*Va-0.5)/0.36025),0.0,1.0);  // 60deg->0.0   30deg->1.0

	double spd;
	spd=(1.0-w2)*spd1+w2*spd2;

	if(YSTRUE==lowSpeedLimitter)
	{
		if(Vl*Va<YsCos80deg)
		{
			spd=leader.Prop().GetVelocity();
		}
		else if(spd<wingman.Prop().GetMinimumManeuvableSpeed())
		{
			spd=wingman.Prop().GetMinimumManeuvableSpeed()+2.0;
		}
	}

	wingman.Prop().SpeedController(spd);
}

void FsFormation::Mode0LateralControl(FsAirplane &wingman,FsAirplane &leader,const double &dt)
{
	// Everything must be computed in the leader airplane's coordinate
	YsVec3 wingmanPos,evWingman,uvWingman,wingmanVel;
	double pathAngle,dPathAngle,eHdg,dEHdg,dX,bnk;
	YsAtt3 leaderAtt;

	if(leader.Prop().GetVelocity()<1.0)
	{
		leaderAtt=leader.GetAttitude();
	}
	else
	{
		YsVec3 leaderEv,leaderUv;
		leader.Prop().GetVelocity(leaderEv);
		leaderUv=leader.GetAttitude().GetUpVector();
		leaderAtt.SetTwoVector(leaderEv,leaderUv);
	}


	wingmanPos=wingman.GetPosition()-leader.GetPosition();
	leaderAtt.MulInverse(wingmanPos,wingmanPos);

	leaderAtt.MulInverse(evWingman,wingman.GetAttitude().GetForwardVector());
	leaderAtt.MulInverse(uvWingman,wingman.GetAttitude().GetUpVector());

	wingman.Prop().GetVelocity(wingmanVel);
	leaderAtt.MulInverse(wingmanVel,wingmanVel);

	pathAngle=atan2(-wingmanVel.x(),wingmanVel.z());
	dPathAngle=(pathAngle-prevPathAngle)/dt;

	eHdg=-pathAngle;
	dEHdg=(eHdg-prevEHdg)/dt;

	dX=(wingmanPos.x()-prevX)/dt;

	// if(not inverted position)
	{
		YsAtt3 att;
		double damping;

		damping=1.0;
		if(wingman.Prop().GetVelocity()>150.0 || wingman.Prop().GetG()>1.0)
		{
			double r;
			r=wingman.Prop().GetVelocity()/150.0;
			damping=9.0*(r+(1.0+wingman.Prop().GetG()/10.0));
		}
		bnk=eHdg*0.0-dPathAngle*damping;  // <- coefficient is from experiments
		// Counter bank to dPathAngle works as a damper (as a result of experiment)

		double counterVx;  // Lateral acceleration=9.8*G*sin(bnk);
		counterVx=YsDegToRad(dX)*8.0;
		bnk+=counterVx;

		double counterX,offX;   // Adjust lateral deviation
		offX=wingmanPos.x()-shouldBe.x();
		counterX=YsDegToRad(offX)*4.0;
		bnk+=counterX;


		YsAtt3 leaderRotationalSpeed;
		double leaderVbCorrection,leaderKr,leaderRmax,leaderKp;
		leaderKr=leader.Prop().GetRollManeuvabilityConst();
		leaderKp=leader.Prop().GetBankControllerGain();
		leader.Prop().GetRollRate(leaderRmax);
		leader.Prop().GetRotationSpeed(leaderRotationalSpeed);
		leaderVbCorrection=leaderRotationalSpeed.b();
		if(fabs(leaderVbCorrection)>YsTolerance)               // 2004/10/03
		{                                                      // 2004/10/03
			double denom=(leaderRmax*leaderKp*leaderKr);       // 2004/10/03
			if(fabs(denom)>YsTolerance)                        // 2004/10/03
			{                                                  // 2004/10/03
				leaderVbCorrection/=denom;                     // 2004/10/03
			}                                                  // 2004/10/03
		}                                                      // 2004/10/03
		bnk+=leaderVbCorrection*0.5;  // <- theoretically 1.0 should work.

		// printf("EHDG:% .2lf BNK:% .2lf PATH:% .2lf VX:% .2lf DX:% .2lf  X:% .2lf LSSA:% .2lf\n",
		// YsRadToDeg(eHdg),YsRadToDeg(wingman.GetAttitude().b()),YsRadToDeg(pathAngle),wingmanVel.x(),dX,offX);

		// if(leader.Prop().GetG()>2.0)
		// {
		// 	double damper;
		// 	damper=1.0-(leader.Prop().GetG()-2.0)*0.0625;
		// 	if(damper<0.5)
		// 	{
		// 		damper=0.5;
		// 	}
		// 	bnk*=damper;
		// }

		bnk=YsBound(bnk,-YsPi/4.0,YsPi/4.0);
		if((inverted!=YSTRUE && wingman.Prop().GetG()<0.0) ||
		   (inverted==YSTRUE && wingman.Prop().GetG()>0.0))
		{
			bnk=-bnk;
		}

		if(inverted==YSTRUE)
		{
			bnk+=YsPi;
		}

		att.SetTwoVector(evWingman,uvWingman);  // <- This is done in the leader's coordinate
		att.SetB(bnk);
		uvWingman=att.GetUpVector();  // <- Here, the up vector is updated.

		// Now, we can think in the global coordinate,
		leaderAtt.Mul(uvWingman,uvWingman);

		// and in the wingman's coordinate.
		wingman.GetAttitude().MulInverse(uvWingman,uvWingman);

		double bnkOffset;
		bnkOffset=atan2(-uvWingman.x(),uvWingman.y());
		bnkOffset=YsBound(bnkOffset,-YsPi*2.0/3.0,YsPi*2.0/3.0);

		// From numerical methods (Numerical optimization);
		bnkOffset*=1.2;

		double requiredBnk;
		requiredBnk=wingman.GetAttitude().b()+bnkOffset;
		if(wingman.GetPosition().y()<33.0)
		{
			double bnkLimit;
			bnkLimit=YsDegToRad(wingman.GetPosition().y());
			requiredBnk=YsBound(requiredBnk,-bnkLimit,bnkLimit);
		}

		wingman.Prop().BankController(requiredBnk);
	}

	prevPathAngle=pathAngle;
	prevEHdg=eHdg;
	prevX=wingmanPos.x();
}

// This is not bad at all, but I have another idea.
//void FsFormation::Mode0LateralControl(FsAirplane &wingman,FsAirplane &leader,const double &dt)
//{
//	// Everything must be computed in the leader airplane's coordinate
//	double aimingDist;
//	YsVec3 aimingPoint,wingmanPos,evWingman,uvWingman,vel,desiredDir,velToDesiredDir;
//	double pathAngle,dPathAngle,dHdg,dDHdg,dX,bnk;
//
//	wingmanPos=wingman.GetPosition()-leader.GetPosition();
//	leader.GetAttitude().MulInverse(wingmanPos,wingmanPos);
//
//	aimingDist=wingman.Prop().GetVelocity()/2.0;
//	aimingPoint.Set(shouldBe.x(),shouldBe.y(),wingmanPos.z()+aimingDist);  // <- aiming point in the leader's coord
//
//	leader.GetAttitude().MulInverse(evWingman,wingman.GetAttitude().GetForwardVector());
//	leader.GetAttitude().MulInverse(uvWingman,wingman.GetAttitude().GetUpVector());
//
//	wingman.Prop().GetVelocity(vel);
//	leader.GetAttitude().MulInverse(vel,vel);
//
//	desiredDir=aimingPoint-wingmanPos;
//
//	pathAngle=atan2(-vel.x(),vel.z());
//	dPathAngle=(pathAngle-prevPathAngle)/dt;
//
//	velToDesiredDir=desiredDir;
//	velToDesiredDir.RotateXZ(-pathAngle);
//	dHdg=atan2(-velToDesiredDir.x(),velToDesiredDir.z());
//	dDHdg=(dHdg-prevDHdg)/dt;
//
//	dX=(wingmanPos.x()-prevX)/dt;
//
//	// if(not inverted position)
//	{
//		YsAtt3 att;
//
//		bnk=dHdg*9.0-dPathAngle*9.0;  // <- coefficient is from experiments
//
//
//		double counterVx;  // Lateral acceleration=9.8*G*sin(bnk);
//		counterVx=YsDegToRad(dX)*3.0;
//		bnk+=counterVx;
//
//printf("DHDG:% .2lf BNK:% .2lf PATH:% .2lf VX:% .2lf DX:% .2lf\n",
//YsRadToDeg(dHdg),YsRadToDeg(bnk),YsRadToDeg(pathAngle),vel.x(),dX);
//
//
//		bnk=YsBound(bnk,-YsPi/6.0,YsPi/6.0);
//		if(wingman.Prop().GetG()<0.0)
//		{
//			bnk=-bnk;
//		}
//
//		att.SetTwoVector(evWingman,uvWingman);  // <- This is done in the leader's coordinate
//		att.SetB(bnk);
//		uvWingman=att.GetUpVector();  // <- Here, the up vector is updated.
//
//		// Now, we can think in the global coordinate,
//		leader.GetAttitude().Mul(uvWingman,uvWingman);
//
//		// and in the wingman's coordinate.
//		wingman.GetAttitude().MulInverse(uvWingman,uvWingman);
//
//		double bnkOffset;
//		bnkOffset=atan2(-uvWingman.x(),uvWingman.y());
//		wingman.Prop().BankController(wingman.GetAttitude().b()+bnkOffset);
//	}
//
//	prevPathAngle=pathAngle;
//	prevDHdg=dHdg;
//	prevX=wingmanPos.x();
//}

void FsFormation::Mode0VerticalControl(FsAirplane &wingman,FsAirplane &leader,const double &dt)
{
//Use, pitch maneuvability const and pitch stability const.
//Say, something=Kpm/Kps -> use it as a magnification factor.
//Kpm/Kps=10.0 for F-16
//Kpm/Kps=4.0 for T-4
//Kpm/Kps=5.0 for F-18
//Kpm/Kps=4.0 for F-15
//Kpm/Kps=4.0 for T-2
	double pitchManeuvability;
	double b,k;
	wingman.Prop().GetPitchManeuvabilityConst(k,b);
	pitchManeuvability=k/10.0;


	// Everything must be computed in the leader airplane's coordinate
	YsVec3 wingmanPos,wingmanVel,wingmanEv,wingmanUv;
	YsAtt3 leaderAtt,wingmanAtt;

	leaderAtt=leader.GetAttitude();

	// Strictly speaking, leaderAtt is not the attitude of the leader airplane.
	// It is an attitude of a virtual airplane (well, everything in this game is virtual though)
	// whose velocity is exactly equal to the leader airplane but with zero angle of attack and
	// zero side slip angle.

	wingmanPos=wingman.GetPosition()-leader.GetPosition();
	leaderAtt.MulInverse(wingmanPos,wingmanPos);
	wingman.Prop().GetVelocity(wingmanVel);
	leaderAtt.MulInverse(wingmanVel,wingmanVel);

	wingmanEv=wingman.GetAttitude().GetForwardVector();
	wingmanUv=wingman.GetAttitude().GetUpVector();
	leaderAtt.MulInverse(wingmanEv,wingmanEv);
	leaderAtt.MulInverse(wingmanUv,wingmanUv);
	wingmanAtt.SetTwoVector(wingmanEv,wingmanUv);

	YSBOOL calypso;
	double relativePitchVelocity,pch,bnk;
	pch=atan2(wingmanEv.y(),wingmanEv.z());
	relativePitchVelocity=(pch-prevPch)/dt;
	prevPch=pch;

	bnk=atan2(YsAbs(wingmanUv.x()),wingmanUv.y());
	if(bnk<YsDegToRad(70.0))
	{
		calypso=YSFALSE;
	}
	else if(bnk>YsDegToRad(70.0))
	{
		calypso=YSTRUE;
	}
	else
	{
		wingman.Prop().GController(0.0);
		return;
	}

	double eY,dEY,dDEY;

	eY=(shouldBe.y()+spacing.y())-wingmanPos.y();
	iEY=YsBound(iEY+eY*dt,-3.0,3.0);
	dEY=(eY-prevEY)/dt;
	dDEY=(dEY-prevDEY)/dt;  // <- second derivative
	prevEY=eY;
	prevDEY=dEY;

	double Greq;
	Greq=0.0;

//	double desiredVY;
//	if(eY>5.0)  // wingmanPos is too low
//	{
//		desiredVY=5.0;
//	}
//	else if(eY<-5.0)  // wingmanPos is too high
//	{
//		desiredVY=-5.0;
//	}
//	else
//	{
//		desiredVY=eY;
//	}
//	Greq+=(desiredVY+dEy)*0.2;

	if(calypso!=YSTRUE)
	{
		Greq+=eY*1.0+dEY*0.4+iEY*0.1;
	}
	else
	{
		Greq-=eY*1.0+dEY*0.4+iEY*0.1;
	}


// The following block didn't give a good result.
//	if(eY*dEy>0.0 && YsAbs(eY)>1.0)  // The airplane is moving far away, and the distance became more than 0.5.
//	{
//		double correction,coeff;
//		coeff=(YsAbs(eY)-1.0)/5.0;
//		correction=dEy*coeff;
//		correction=YsBound(correction,-1.0,1.0);
//		Greq+=correction;
//	}


	// <<<< DAMPING >>>>
	// Experiments show that applying G force so that the wingman tends to make its nose
	// parallel to the nose of the leader airplane works as a strong damper.
	// The following correction in Greq is a known working damper.
	//{
	//	Greq-=YsRadToDeg(wingmanAtt.p())*1.0;
	//}
	// A little better controller than the above.
	{
		double coeff;
		coeff=wingman.Prop().GetVelocity()*0.4/wingman.Prop().GetEstimatedLandingSpeed();   // Vlanding -> 0.4;
		coeff=YsBound(coeff,0.4,1.0);
		// Why is this coefficient required?
		//    This damping referes to the pitch angle.  And, the physics tells ommega=a*v,
		//    i.e., rotational velocity is proportional to the velocity and a (which is 9.8*G).
		//    Thus, the magnitude of G must be controlled according to the velocity.
		//    If I apply too much G force when the velocity is low, it will become overcontrol

		YsVec3 leaderVel;
		double p,reqAOA,pitchCorrection;
		leader.Prop().GetVelocity(leaderVel);
		wingman.GetAttitude().MulInverse(leaderVel,leaderVel);
		if(calypso!=YSTRUE)
		{
			reqAOA=wingman.Prop().ComputeAOAForRequiredG(leader.Prop().GetG());
		}
		else
		{
			reqAOA=wingman.Prop().ComputeAOAForRequiredG(-leader.Prop().GetG());
		}

		pitchCorrection=eY*YsDegToRad(1.0)+dEY*YsDegToRad(1.0);  // A spice making this controller very effective!
		if(calypso==YSTRUE)
		{
			pitchCorrection=-pitchCorrection;
		}
		pitchCorrection=YsBound(pitchCorrection,-0.3,0.3);

		p=atan2(leaderVel.y(),leaderVel.z())+reqAOA+pitchCorrection;
		Greq+=YsRadToDeg(p)*coeff;
		// printf("P:%6.2lf ",YsRadToDeg(p));

		// YsAtt3 leaderRotation,wingmanRotation;
		// double leaderVp,wingmanVp;
		// leader.Prop().GetRotationSpeed(leaderRotation);
		// wingman.Prop().GetRotationSpeed(wingmanRotation);
		// leaderVp=leaderRotation.p();  // <- negate for inverted mode
		// wingmanVp=wingmanRotation.p();
		// Greq-=YsRadToDeg(wingmanVp-leaderVp)*coeff;
		// printf("DVP:%6.2lf ",YsRadToDeg(wingmanVp-leaderVp));
		if(calypso!=YSTRUE)
		{
			Greq-=YsRadToDeg(relativePitchVelocity)*coeff;
		}
		else
		{
			Greq+=YsRadToDeg(relativePitchVelocity)*coeff;
		}
		// printf("DVP:%6.2lf ",YsRadToDeg(relativePitchVelocity));
	}

	if(calypso!=YSTRUE)
	{
		Greq+=leader.Prop().GetG();
	}
	else
	{
		Greq-=leader.Prop().GetG();
	}


	// printf("EY:% 6.2lf DEY:% 6.2lf IEY:% 6.2lf G:%6.2lf  Gin:%6.2lf\n",eY,dEY,iEY,wingman.Prop().GetG(),Greq);

	// Heuristics from numerical optimization.
	//   Assumption: Actual G output response is slow.
	Greq=wingman.Prop().GetG()+(Greq-wingman.Prop().GetG())*1.5;

	if(wingman.GetPosition().y()-wingman.Prop().GetGroundStandingHeight()>3.0)  // In the air
	{
		wingman.Prop().GController(Greq);
	}
	else  // Near ground
	{
		double glideAngle,tailStrikeAngle;
		YsVec3 vel,velH;

		wingman.Prop().GetVelocity(vel);
		velH=vel;
		velH.SetY(0.0);
		glideAngle=atan2(-vel.y(),velH.GetLength());

		tailStrikeAngle=wingman.Prop().GetTailStrikePitchAngle(0.90);
		wingman.Prop().GController(Greq);
		wingman.Prop().SetGControllerAOALimit(glideAngle,tailStrikeAngle+glideAngle);
	}


// Save this portion.  This is slow, but stable controller >>
//	double desiredVY;
//	if(eY>5.0)  // wingmanPos is too low
//	{
//		desiredVY=5.0;
//	}
//	else if(eY<-5.0)  // wingmanPos is too high
//	{
//		desiredVY=-5.0;
//	}
//	else
//	{
//		desiredVY=eY;
//	}
//	double Greq;
//	Greq=(desiredVY+dEy)*0.1;
//	wingman.Prop().GController(leader.Prop().GetG()+Greq);
// << Save this portion.  This is slow, but stable controller


//	double Gtotal,Gprop,Gdiff,Gpitch,cosBnk;
//	const double Kp= 0.8;       // 0.1m difference corresponds to 0.1G
//	const double Kd=0.1;      // Just guess
//	const double Ka=1.0;
//	const double Kd2=1.0;
//	const double Ki= 0.1;       // Just guess
//
//	Gprop=Kp*eY;
//	Gprop=YsBound(Gprop,-1.0,1.0);
//
//	Gdiff=Kd*dEy+Ka*dDEy;
//	Gdiff=YsBound(Gdiff,-0.5,0.5);
//
//	Gpitch=-Kd2*YsRadToDeg(wingmanAtt.p());
//
//	Gtotal=Gprop+Gdiff+Gpitch;
//
//
//	cosBnk=cos(wingmanAtt.b());
//	// if(not inverted position)
//	{
//		if(YsAbs(cosBnk)>0.1)
//		{
//// 			relG/=cosBnk;
//		}
//		Gtotal=YsBound(Gtotal,-3.0,4.0);
//		Gtotal+=leader.Prop().GetG();
//		wingman.Prop().GController(Gtotal);
//printf("% 6.2lf % 6.2lf Gin:% 6.2lf  Gout:% 6.2lf\n",dEy,eY,Gtotal,wingman.Prop().GetG());
//	}
}

//YSRESULT FsFormation::IntelligentFormationChange(YsVec3 &neoShouldBe,int type)
//{
//	int i;
//	if(type==TODELTA || type==TOARROWHEAD) // To Delta, X first
//	{
//		nVia=8;
//		via[0]=shouldBe;
//		via[0].SetX(neoShouldBe.x());
//		for(i=1; i<8; i++)
//		{
//			via[i]=via[0]+(neoShouldBe-via[0])*double(i)/7.0;
//		}
//		via[7]=neoShouldBe;
//	}
//	else if(type==TOTRAIL || type==TOECHELON) // To Trail, Z first
//	{
//		nVia=8;
//		via[6]=shouldBe;
//		via[6].SetY(neoShouldBe.y());
//		via[6].SetZ(neoShouldBe.z());
//		for(i=0; i<7; i++)
//		{
//			via[i]=shouldBe+(via[6]-shouldBe)*double(i)/7.0;
//		}
//		via[7]=neoShouldBe;
//	}
//	else // if(type==NOTHING)
//	{
//		shouldBe=neoShouldBe;
//	}
//	return YSOK;
//}

YsVec3 FsFormation::GetGoalPosition(void) const
{
	auto goal=shouldBe+spacing;
	leader->Prop().GetMatrix().Mul(goal,goal,1.0);
	return goal;
}

void FsFormation::CorrectAltitude(YsVec3 &pos,const double hDist)
{
	// hDist            minAlt
	// 3000.0           0.0
	// 30000.0          10000ft
	const double t=YsBound((hDist-3000.0)/27000.0,0.0,1.0);
	const double minAlt=YsUnitConv::FTtoM(10000.0)*t;
	if(pos.y()<minAlt)
	{
		pos.SetY(minAlt);
	}
}

void FsFormation::SetShouldBe(const YsVec3 &shouldBe)
{
	this->shouldBe=shouldBe;
}
const YsVec3 &FsFormation::GetShouldBe(void) const
{
	return shouldBe;
}
