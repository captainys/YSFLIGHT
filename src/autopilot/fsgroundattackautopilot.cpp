#include <ysclass.h>
#include <ysunitconv.h>
#include "fs.h"
#include "fsautopilot.h"


/* static */ const char *FsGroundAttack::StateToStr(FsGroundAttack::STATE state)
{
	switch(state)
	{
	case STATE_GETTINGTHERE:
		return "GETTINGTHERE";
	case STATE_TURNINBOUND:
		return "TURNINBOUND";
	case STATE_INBOUND_AGM_ROCKET:
		return "INBOUND_AGM_ROCKET";
	case STATE_INBOUND_BOMB:
		return "INBOUND_BOMB";
	case STATE_TURNAWAY:
		return "TURNAWAY";
	case STATE_EVASIVE_0:
		return "EVASIVE_0";
	case STATE_EVASIVE_1:
		return "EVASIVE_1";
	}
	return "GETTINGTHERE";
}
/* static */ FsGroundAttack::STATE FsGroundAttack::StrToState(const char str[])
{
	if(0==strcmp("GETTINGTHERE",str))
	{
		return STATE_GETTINGTHERE;
	}
	else if(0==strcmp("TURNINBOUND",str))
	{
		return STATE_TURNINBOUND;
	}
	else if(0==strcmp("INBOUND_AGM_ROCKET",str))
	{
		return STATE_INBOUND_AGM_ROCKET;
	}
	else if(0==strcmp("INBOUND_BOMB",str))
	{
		return STATE_INBOUND_BOMB;
	}
	else if(0==strcmp("TURNAWAY",str))
	{
		return STATE_TURNAWAY;
	}
	else if(0==strcmp("EVASIVE_0",str))
	{
		return STATE_EVASIVE_0;
	}
	else if(0==strcmp("EVASIVE_1",str))
	{
		return STATE_EVASIVE_1;
	}
	return STATE_GETTINGTHERE;
}


FsGroundAttack::FsGroundAttack()
{
	attackPhase=STATE_GETTINGTHERE;
	turnAwayAfterWeaponRelease=YSFALSE;
	turnAwayHeading=0.0;
	turnAwayDistance=15000.0;
	turnAwayRelDir=YsOrigin();
	turnAwayTurnDir=-1;
	target=NULL;
	playerGroundKey=YSNULLHASHKEY;
	prevDist=0;
	breakingTime=0.0;
	// flareTimer=0.0;

	leadTargetPos=YsOrigin();

	// prevMissileChasing=YSFALSE;
	// missileChasing=YSFALSE;;
	bankForEvadingMissile=YsPi/4.0;
	// flareButton=YSFALSE;
	attackDone=YSFALSE;

	takeEvasiveAction=YSTRUE;

	bomberAlt=YsUnitConv::FTtoM(5000.0);
	attackAlt=YsUnitConv::FTtoM(5000.0);

	agmReleaseDist=3050.0;
	rocketReleaseDist=2550.0;
	gunReleaseDist=700.0;

	turnRadius=5000.0;
	turnRadiusHeavy=10000.0;
	inboundSpeed=150.0;

	flareDispenser.SetFlareInterval(5.0);
	breakOnMissile=YSFALSE;
}

FsGroundAttack::~FsGroundAttack()
{
}

/* static */ FsGroundAttack *FsGroundAttack::Create(void)
{
	return new FsGroundAttack;
}

YSRESULT FsGroundAttack::SaveIntention(FILE *fp,const FsSimulation *)
{
	fprintf(fp,"GNDATACK\n");
	fprintf(fp,"ATKSTATE %s\n",StateToStr(attackPhase));
	if(pendingGndTargetName[0]!=0)
	{
		fprintf(fp,"GNDTARGT %s\n",(const char *)pendingGndTargetName);
	}
	fprintf(fp,"TURNAWAY %s\n",YsBoolToStr(turnAwayAfterWeaponRelease));
	fprintf(fp,"TNAWYHDG %lfdeg\n",YsRadToDeg(turnAwayHeading));
	fprintf(fp,"TNAWYDST %.2lfm\n",turnAwayDistance);
	for(auto t : gndToAirThreat)
	{
		fprintf(fp,"GATHREAT %.0lfm %.0lfm %.0lfm %.0lf\n",t.pos.x(),t.pos.y(),t.pos.z(),t.range);
	}
	if(takeEvasiveAction!=YSTRUE)
	{
		fprintf(fp,"NOEVASIV\n");
	}
	fprintf(fp,"BOMBRALT %lfm\n",bomberAlt);
	fprintf(fp,"ATACKALT %lfm\n",attackAlt);

	fprintf(fp,"AGMRDIST %lfm\n",agmReleaseDist);
	fprintf(fp,"RKTRDIST %lfm\n",rocketReleaseDist);
	fprintf(fp,"GUNRDIST %lfm\n",gunReleaseDist);

	fprintf(fp,"RADLIGHT %lfm\n",turnRadius);
	fprintf(fp,"RADHEAVY %lfm\n",turnRadiusHeavy);
	fprintf(fp,"INBSPEED %lfm/s\n",inboundSpeed);

	fprintf(fp,"FLARITVL %lfsec\n",flareDispenser.GetFlareInterval());
	fprintf(fp,"EVADEMSL %s\n",FsTrueFalseString(breakOnMissile));

	return YSOK;
}

YSRESULT FsGroundAttack::ReadIntention(YsTextInputStream &inStream,const YsString &firstLine)
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
			if(0==strcmp(args[0],"GNDATACK"))
			{
			}
			else if(0==strcmp(args[0],"ATKSTATE"))
			{
				if(2<=args.GetN())
				{
					attackPhase=StrToState(args[1]);
				}
			}
			else if(0==strcmp(args[0],"TURNAWAY"))
			{
				if(2<=args.GetN())
				{
					turnAwayAfterWeaponRelease=YsStrToBool(args[1]);
				}
			}
			else if(0==strcmp(args[0],"TNAWYHDG"))
			{
				if(2<=args.GetN())
				{
					FsGetAngle(turnAwayHeading,args[1]);
				}
			}
			else if(0==strcmp(args[0],"TNAWYDST"))
			{
				if(2<=args.GetN())
				{
					FsGetLength(turnAwayDistance,args[1]);
				}
			}
			else if(0==strcmp(args[0],"GNDTARGT"))
			{
				pendingGndTargetName.Set(args[1]);
			}
			else if(0==strcmp(args[0],"NOEVASIV"))
			{
				takeEvasiveAction=YSFALSE;
			}
			else if(0==strcmp(args[0],"BOMBRALT"))
			{
				FsGetLength(bomberAlt,args[1]);
			}
			else if(0==strcmp(args[0],"ATACKALT"))
			{
				if(2<=args.GetN())
				{
					FsGetLength(attackAlt,args[1]);
				}
			}
			else if(0==strcmp(args[0],"AGMRDIST"))
			{
				FsGetLength(agmReleaseDist,args[1]);
			}
			else if(0==strcmp(args[0],"RKTRDIST"))
			{
				FsGetLength(rocketReleaseDist,args[1]);
			}
			else if(0==strcmp(args[0],"GUNRDIST"))
			{
				FsGetLength(gunReleaseDist,args[1]);
			}
			else if(0==strcmp(args[0],"RADLIGHT"))
			{
				FsGetLength(turnRadius,args[1]);
			}
			else if(0==strcmp(args[0],"RADHEAVY"))
			{
				FsGetLength(turnRadiusHeavy,args[1]);
			}
			else if(0==strcmp(args[0],"INBSPEED"))
			{
				FsGetSpeed(inboundSpeed,args[1]);
			}
			else if(0==strcmp(args[0],"FLARITVL"))
			{
				flareDispenser.SetFlareInterval(atof(args[1]));
			}
			else if(0==strcmp(args[0],"EVADEMSL"))
			{
				FsGetBool(breakOnMissile,args[1]);
			}
			else if(0==strcmp(args[0],"GATHREAT"))
			{
				if(5<=args.GetN())
				{
					double x,y,z,r;
					FsGetLength(x,args[1]);
					FsGetLength(y,args[2]);
					FsGetLength(z,args[3]);
					FsGetLength(r,args[4]);
					gndToAirThreat.Increment();
					gndToAirThreat.Last().pos.Set(x,y,z);
					gndToAirThreat.Last().range=r;
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

int FsGroundAttack::GetNumAGMReleasePerPass(const FsAirplane &air) const
{
	if(NULL!=target)
	{
		int dmgTol=target->Prop().GetDamageTolerance();
		int nRelease=(dmgTol+air.Prop().GetAGMDestructivePower()-1)/air.Prop().GetAGMDestructivePower();;
		if(0<nRelease)
		{
			return nRelease;
		}
		return 1;
	}
	return 2;
}

const double FsGroundAttack::GetAGMReleaseDistance(void) const
{
	return agmReleaseDist;
}
void FsGroundAttack::SetAGMReleaseDistance(const double dist)
{
	this->agmReleaseDist=dist;
}

void FsGroundAttack::SetTurnAwayAfterWeaponRelease(YSBOOL turnAway)
{
	this->turnAwayAfterWeaponRelease=turnAway;
}
void FsGroundAttack::SetTurnAwayAndFlyToAtLeastThisDistance(const double dist)
{
	this->turnAwayDistance=dist;
}

void FsGroundAttack::SetInboundSpeed(const double vel)
{
	this->inboundSpeed=vel;
}

void FsGroundAttack::SetAttackerAltitude(const double y)
{
	attackAlt=y;
}

void FsGroundAttack::SetBomberAltitude(const double y)
{
	bomberAlt=y;
}

YSRESULT FsGroundAttack::MakePriorityDecision(FsAirplane &)
{
	emr=EMR_NONE;
	return YSOK;
}

YSRESULT FsGroundAttack::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if((target==NULL || target->IsAlive()!=YSTRUE) && attackPhase!=STATE_TURNAWAY)
	{
		SearchTarget(air,sim);

		air.Prop().SetGroundTargetKey(FsExistence::GetSearchKey(target));  // 2005/02/27 
		SetPhase(air,STATE_GETTINGTHERE);
		air.Prop().CaptureGControllerSmoother();
	}

	if(air.Prop().AntiGroundWeaponIsLoaded()!=YSTRUE)
	{
		target=NULL;
	}

	breakingTime+=dt;
	if(takeEvasiveAction==YSTRUE && Phase()!=STATE_EVASIVE_0 && Phase()!=STATE_EVASIVE_1 && BeingChasedByEnemy(air,sim)==YSTRUE)
	{
		SetPhase(air,((rand()&1)==0 ? STATE_EVASIVE_0 : STATE_EVASIVE_1));
		air.Prop().CaptureGControllerSmoother();
		breakingTime=0.0;
	}


	flareDispenser.MakeDecision(air,sim,dt);
	bankForEvadingMissile=(1-2*(flareDispenser.GetTimerUpCounter()&1))*fabs(bankForEvadingMissile);

	if(YSTRUE==flareDispenser.MissileChasing() && YSTRUE==breakOnMissile && Phase()!=STATE_EVASIVE_0 && Phase()!=STATE_EVASIVE_1)
	{
		SetPhase(air,((rand()&1)==0 ? STATE_EVASIVE_0 : STATE_EVASIVE_1));
		air.Prop().CaptureGControllerSmoother();
		breakingTime=0.0;
	}


	if(target==NULL)  // <- Target==NULL even after searching
	{
		// Climb up to 5000ft
		// Fly straight at max power
		leadTargetPos=YsOrigin();
	}
	else
	{
		if(target->Prop().GetVelocity()>YsTolerance)
		{
			double leadTime;
			switch(choice)
			{
			default:
				leadTime=0.0;
				break;
			case FSWEAPON_GUN:
				leadTime=YsSmaller(10.0,dif.GetLength()/air.Prop().GetBulletSpeed());
				break;
			case FSWEAPON_BOMB:
			case FSWEAPON_BOMB250:
				{
					// vy*t-0.5g t^2=dif.y   dif.y:negative
					// -0.5g t^2+ vy*t-dif.y=0
					// t=(-b+-sqrt(b*b-4*a*c)/(2a)

					double a,b,c,det;
					YsVec3 airVel;
					air.Prop().GetVelocity(airVel);
					a=-0.5*FsGravityConst;
					b=0.5*airVel.y();
					c=-dif.y();
					det=b*b-4.0*a*c;
					if(det>0.0)
					{
						leadTime=YsSmaller(20.0,(-b-sqrt(det))/(2.0*a));  // Since a is negative, this can be only positive solution.
					}
					else
					{
						leadTime=0.0;
					}
				}
				break;
			case FSWEAPON_ROCKET:
				{
					YsVec3 airVel;
					air.Prop().GetVelocity(airVel);
					if(airVel.Normalize()==YSOK)
					{
						airVel*=air.Prop().GetRocketSpeed();
						airVel.SetY(0.0);
						leadTime=2450.0/airVel.GetLength();  // 2450m: Launch distance (2350-2550).  See below.
					}
					else
					{
						leadTime=0.0;
					}
				}
				break;
			}

			YsVec3 gndVel;
			target->Prop().GetVelocity(gndVel);
			leadTargetPos=target->GetPosition()+gndVel*leadTime;  // In total: dif=targetPos+leadVec-airPos;
		}
		else
		{
			leadTargetPos=target->GetPosition();
		}


		dif=leadTargetPos-air.GetPosition();

		difHorizontal=dif;
		difHorizontal.SetY(0.0);

		YsAtt3 attHorizontal;
		attHorizontal=air.GetAttitude();
		attHorizontal.SetP(0.0);
		attHorizontal.SetB(0.0);

		YsMatrix4x4 matHorizontal(YSFALSE);
		attHorizontal.GetMatrix4x4(matHorizontal);
		matHorizontal.Transpose();  // Transpose is equivalent to invert for a matrix obtained from YsAtt3
		matHorizontal.Mul(rel,dif,0.0);
		matHorizontal.Mul(relHorizontal,difHorizontal,0.0);

		if(Phase()==STATE_GETTINGTHERE /*0*/)
		{
			// Climb up to 5000ft
			// Fly straight at max power
			// until the distance from the target reaches
			// 18km if it is a heavy bomber,
			// or 5km if it is a fighter, an attacker, an aerobatic or a trainer.

			double reqDist;

			if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
			   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
			   air.Prop().GetAirplaneCategory()==FSAC_WW2FIGHTER ||
			   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
			   air.Prop().GetAirplaneCategory()==FSAC_TRAINER)
			{
				reqDist=turnRadius;
			}
			else
			{
				reqDist=turnRadiusHeavy;
			}

			if(/* relHorizontal.z()<0.0 && <- is it necessary? */
			   difHorizontal.GetLength()>reqDist)
			{
				SetPhase(air,STATE_TURNINBOUND /*1*/);
				air.Prop().CaptureGControllerSmoother();
			}
		}
		else if(Phase()==STATE_TURNINBOUND /*1*/)
		{
			attackDone=YSFALSE;
			if(relHorizontal.z()>YsTolerance &&
			   YsAbs(relHorizontal.x()/relHorizontal.z())<0.1)
			{
				int nSel;
				FSWEAPONTYPE sel[5];

				nSel=0;

				if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
				   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
				   air.Prop().GetAirplaneCategory()==FSAC_WW2FIGHTER ||
				   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
				   air.Prop().GetAirplaneCategory()==FSAC_WW2ATTACKER ||
				   air.Prop().GetAirplaneCategory()==FSAC_TRAINER)
				{
					if(0<air.Prop().GetNumWeapon(FSWEAPON_GUN))
					{
						sel[nSel++]=FSWEAPON_GUN;
					}
					if(air.Prop().GetNumWeapon(FSWEAPON_BOMB)>0)
					{
						sel[nSel++]=FSWEAPON_BOMB;
					}
					if(air.Prop().GetNumWeapon(FSWEAPON_BOMB250)>0)
					{
						sel[nSel++]=FSWEAPON_BOMB250;
					}
					if(air.Prop().GetNumWeapon(FSWEAPON_AGM65)>0)
					{
						sel[nSel++]=FSWEAPON_AGM65;
					}
					if(air.Prop().GetNumWeapon(FSWEAPON_ROCKET)>0)
					{
						sel[nSel++]=FSWEAPON_ROCKET;
					}

					if(nSel==0)
					{
						SetPhase(air,STATE_INBOUND_AGM_ROCKET /*2*/);
						air.Prop().CaptureGControllerSmoother();
						choice=FSWEAPON_GUN;
					}
					else
					{
						choice=sel[rand()%nSel];
						air.Prop().SetWeaponOfChoice(choice);
						switch(choice)
						{
						default:
						case FSWEAPON_AGM65:
						case FSWEAPON_ROCKET:
							SetPhase(air,STATE_INBOUND_AGM_ROCKET /*2*/);
							break;
						case FSWEAPON_BOMB:
						case FSWEAPON_BOMB250:
							SetPhase(air,STATE_INBOUND_BOMB /*3*/);
							break;
						}
					}
				}
				else
				{
					if(air.Prop().GetNumWeapon(FSWEAPON_BOMB)>0)
					{
						sel[nSel++]=FSWEAPON_BOMB;
					}
					if(air.Prop().GetNumWeapon(FSWEAPON_BOMB250)>0)
					{
						sel[nSel++]=FSWEAPON_BOMB250;
					}

					if(nSel>0)
					{
						SetPhase(air,STATE_INBOUND_BOMB /*3*/);
						choice=sel[rand()%nSel];
						air.Prop().SetWeaponOfChoice(choice);
					}
					else
					{
						choice=FSWEAPON_BOMB;
					}

					SetPhase(air,STATE_INBOUND_BOMB /*3*/);
				}
			}
		}
		else if(Phase()==STATE_INBOUND_AGM_ROCKET /*2*/ || Phase()==STATE_INBOUND_BOMB /*3*/)
		{
			if(YSTRUE==turnAwayAfterWeaponRelease && YSTRUE==attackDone)
			{
				// Record threat position
				YsVec3 dir=YsZVec();
				dir.RotateXZ(air.GetAttitude().h());

				YsVec3 p0=air.GetPosition();
				YsVec3 p1=target->GetPosition()+dir*turnRadius;

				YsArray <FsSimInfo::GndToAirThreat,16> threatFound;
				sim->FindGroundToAirThreat(threatFound,p0,p1,air);
				this->gndToAirThreat.CleanUp();
				for(auto &t : threatFound)
				{
					const FsExistence *obj=sim->FindObject(t.objKey);
					if(NULL!=obj)
					{
						gndToAirThreat.Increment();
						gndToAirThreat.Last().pos=obj->GetPosition();
						gndToAirThreat.Last().range=t.range;
					}
				}

				// Turn toward the player airplane or random.
				const FsExistence *player=sim->GetPlayerObject();
				if(NULL!=player)
				{
					YsVec3 pos=(player->GetPosition()-air.GetPosition());
					pos.RotateXZ(-air.GetAttitude().h());
					if(pos.x()<0.0)
					{
						turnAwayHeading=air.GetAttitude().h()+YsPi*0.75;
					}
					else
					{
						turnAwayHeading=air.GetAttitude().h()-YsPi*0.75;
					}
				}
				else
				{
					// It is flying straight to the target anyway.  Choose left or right randomly.
					const double hdgOffset=(0==(rand()&1) ? YsPi*0.75 : -YsPi*0.75);
					turnAwayHeading=air.GetAttitude().h()+hdgOffset;
				}
				turnAwayFrom=target->GetPosition();
				SetPhase(air,STATE_TURNAWAY);
			}
			else if(relHorizontal.z()<0.0 || attackDone==YSTRUE)
			{
				SetPhase(air,STATE_GETTINGTHERE /*0*/);
			}
		}
		else if(Phase()==STATE_EVASIVE_0 /*10*/ || Phase()==STATE_EVASIVE_1 /*11*/)
		{
			if(breakingTime>8.0)
			{
				if(BeingChasedByEnemy(air,sim)==YSTRUE)
				{
					SetPhase(air,(Phase()==STATE_EVASIVE_0 ? STATE_EVASIVE_1 : STATE_EVASIVE_0));
					breakingTime=0.0;
				}
				else
				{
					SetPhase(air,STATE_GETTINGTHERE /*0*/);
				}
			}
		}
		else if(attackPhase==STATE_TURNAWAY)
		{
			YsVec3 vel;
			air.Prop().GetVelocity(vel);
			const double vHdg=atan2(-vel.x(),vel.z());

			turnAwayRelDir=YsZVec();
			turnAwayRelDir.RotateXZ(turnAwayHeading-vHdg);
			const double relHdg=atan2(-turnAwayRelDir.x(),turnAwayRelDir.z());
			if(YsDegToRad(15.0)<fabs(relHdg))
			{
				turnAwayTurnDir=(turnAwayRelDir.x()<0.0 ? 1 : -1);
			}

			const double hDist=(turnAwayFrom-air.GetPosition()).GetLengthXZ();

			if(turnAwayDistance<hDist && YSTRUE==OutOfThreatRange(air,5000.0))
			{
				SetPhase(air,STATE_GETTINGTHERE);
			}
		}
	}
	return YSOK;
}

YSRESULT FsGroundAttack::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	YsVec3 vel;
	double neutralG,cosB,courseSlope;

	air.Prop().NeutralDirectAttitudeControl();

	air.Prop().SetAllVirtualButton(YSFALSE);

	if(air.isPlayingRecord!=YSTRUE)
	{
		air.Prop().SetVectorMarker(YSTRUE);
	}


	flareDispenser.ApplyControl(air,sim,dt);


	cosB=cos(air.GetAttitude().b());
	if(cosB<0.1)
	{
		neutralG=5.0;
	}
	else
	{
		neutralG=YsBound(1.0/cosB,0.1,5.0);
	}

	if(air.Prop().IsOnGround()!=YSTRUE)  // 2002/12/26
	{
		air.Prop().SetGear(0.0);
	}

	air.Prop().GetVelocity(vel);
	courseSlope=vel.y()/sqrt(YsSqr(vel.z())+YsSqr(vel.x()));

	double horizontalDist;
	horizontalDist=relHorizontal.GetLength();

	YsVec3 estimate;
	air.Prop().ComputeEstimatedBombLandingPosition(estimate,sim->GetWeather());

	if(Phase()!=STATE_INBOUND_BOMB /*3*/)
	{
		air.Prop().SetBombBayDoor(0.0);
	}

	// Speed
	if(NULL==target)
	{
		air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.6);
	}
	else if(STATE_GETTINGTHERE==Phase() || STATE_EVASIVE_1==Phase() || STATE_EVASIVE_0==Phase())
	{
		air.Prop().TurnOffSpeedController();
		air.Prop().SetAfterburner(YSTRUE);
		air.Prop().SetThrottle(1.0);
		air.Prop().SetSpoiler(0.0);
	}
	else
	{
		air.Prop().SpeedController(inboundSpeed);
	}

	// Control
	if(target==NULL)
	{
		CircleAround(air,sim,3000.0);
	}
	else if(Phase()==STATE_GETTINGTHERE /*0*/)
	{
		if(flareDispenser.MissileChasing()==YSTRUE)
		{
			air.Prop().BankController(bankForEvadingMissile);
		}
		else
		{
			air.Prop().BankController(0.0);
		}


		const double goalAlt=air.GetFieldElevation()+attackAlt;

		double altErr,pitch;
		altErr=goalAlt-air.GetPosition().y();
		pitch=(altErr/100.0)*YsDegToRad(15.0);
		pitch=YsBound(pitch,YsDegToRad(-15.0),YsDegToRad(15.0));
		air.Prop().PitchController(pitch);
	}
	else
	{
		double hdgErr,bnk;
		hdgErr=atan2(-relHorizontal.x(),relHorizontal.z());

		if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
		   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
		   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
		   air.Prop().GetAirplaneCategory()==FSAC_TRAINER)
		{
			bnk=YsBound(hdgErr*5.0,YsDegToRad(-80.0),YsDegToRad(80.0));
		}
		else
		{
			bnk=YsBound(hdgErr*5.0,YsDegToRad(-60.0),YsDegToRad(60.0));
		}

		if(air.Prop().GetG()>0.0)
		{
			air.Prop().BankController(bnk);
		}
		else
		{
			air.Prop().BankController(-bnk);
		}

		if(Phase()==STATE_TURNINBOUND /*1*/)
		{
			double G;
			if(air.GetAttitude().p()>YsDegToRad(15.0))
			{
				G=-0.8;
			}
			else if(air.GetAttitude().p()<YsDegToRad(-15.0))
			{
				G=2.0;
			}
			else
			{
				double altErr=1515.0-air.GetPosition().y();
				G=altErr/33.0;
			}
			G=YsBound(G,-2.0,2.0);
			air.Prop().GController(neutralG+G);
		}
		else if(Phase()==STATE_INBOUND_AGM_ROCKET /*2*/) // Rocket/Anti-Ground Missile
		{
			if(horizontalDist<7000.0)
			{
				double slope;
				slope=atan2(rel.y(),relHorizontal.GetLength());
				slope+=air.Prop().GetAOA();
				slope=YsBound(slope,YsDegToRad(-30.0),0.0);

				double pitchErr,G;
				pitchErr=slope-air.Prop().GetAttitude().p();
				G=YsBound(pitchErr*10.0,-2.0,2.0);
				air.Prop().GController(neutralG+G);
			}
			else
			{
				double altErr,pitch;
				altErr=1515.0-air.GetPosition().y();
				pitch=(altErr/100.0)*YsDegToRad(10.0);
				pitch=YsBound(pitch,YsDegToRad(-10.0),YsDegToRad(10.0));
				air.Prop().PitchController(pitch);
			}

			switch(choice)
			{
			default:
				break;
			case FSWEAPON_AGM65:
				{
					const double distInterval=200.0/(double)GetNumAGMReleasePerPass(air);
					if(agmReleaseDist-200.0<horizontalDist && horizontalDist<agmReleaseDist &&
					   int(prevDist/distInterval)!=int(horizontalDist/distInterval))
					{
						air.Prop().SetFireAGMButton(YSTRUE);
					}
				}
				break;
			case FSWEAPON_ROCKET:
				if(rocketReleaseDist-200.0<horizontalDist && horizontalDist<rocketReleaseDist &&
			       int(prevDist/40.0)!=int(horizontalDist/40.0))
				{
					air.Prop().SetFireRocketButton(YSTRUE);
				}
				break;
			case FSWEAPON_GUN:
				if(gunReleaseDist-150.0<horizontalDist && horizontalDist<gunReleaseDist)
				{
					air.Prop().SetFireGunButton(YSTRUE);
				}
				break;
			}

			if((choice==FSWEAPON_AGM65 && horizontalDist<agmReleaseDist-200.0) ||
			   (choice==FSWEAPON_ROCKET && horizontalDist<rocketReleaseDist-200.0) ||
			   (choice==FSWEAPON_GUN && horizontalDist<gunReleaseDist-150.0))
			{
				attackDone=YSTRUE;
				air.Prop().SetFireAGMButton(YSFALSE);
				air.Prop().SetFireRocketButton(YSFALSE);
				air.Prop().SetFireGunButton(YSFALSE);
			}
		}
		else if(Phase()==STATE_INBOUND_BOMB /*3*/) // Bombing
		{
			const double goalAlt=bomberAlt+air.GetFieldElevation();

			double pitch=(goalAlt-air.GetPosition().y())/300.0*3.0;
			pitch=YsBound(pitch,-5.0,5.0);

			air.Prop().PitchController(YsDegToRad(pitch));

			const double dist1=(prevEstimate-leadTargetPos).GetLength();
			const double dist2=(estimate-leadTargetPos).GetLength();

			if(dist1<1500.0)
			{
				air.Prop().SetBombBayDoor(1.0);
			}

			if(air.Prop().GetAirplaneCategory()==FSAC_HEAVYBOMBER ||
			   air.Prop().GetAirplaneCategory()==FSAC_WW2BOMBER)
			{
				if(dist1<200.0 && int(dist1/60.0)!=int(dist2/60.0))
				{
					air.Prop().SetFireWeaponButton(YSTRUE);
				}
			}
			else
			{
				if((dist1-30.0)*(dist2-30.0)<0.0)
				{
					air.Prop().SetFireWeaponButton(YSTRUE);
				}
			}
		}
		else if(Phase()==STATE_EVASIVE_0 || Phase()==STATE_EVASIVE_1)
		{
			double bnk=(Phase()==STATE_EVASIVE_0 ? 1.0 : -1.0);

			if(air.Prop().GetAirplaneCategory()!=FSAC_HEAVYBOMBER &&
			   air.Prop().GetAirplaneCategory()!=FSAC_WW2BOMBER)
			{
				if(YsAbs(air.GetAttitude().p())<YsDegToRad(70.0))
				{
					double a;
					a=60.0+YsRadToDeg(air.GetAttitude().p());
					a=YsBound(a,0.0,90.0);
					air.Prop().BankController(bnk*YsDegToRad(a));
				}
				else
				{
					air.Prop().TurnOffBankController();
					air.Prop().SetAileron(0.0);
				}
				air.Prop().GControllerSmooth(5.0,1.6,dt);

				if(
				   (air.Prop().GetNumWeapon(FSWEAPON_BOMB)>0 && 
				    (air.Prop().GetNumWeapon(FSWEAPON_AGM65)>0 || air.Prop().GetNumWeapon(FSWEAPON_ROCKET)>10)) ||

				   (air.Prop().GetNumWeapon(FSWEAPON_BOMB)>2))
				{
					YSBOOL blockedByBombBayDoor;
					air.Prop().FireWeapon
					   (blockedByBombBayDoor,sim,sim->GetClock(),sim->GetWeaponStore(),&air,FSWEAPON_BOMB);
				}
			}
			else
			{
				air.Prop().BankController(bnk*YsDegToRad(45.0));
				air.Prop().PitchController(YsDegToRad(3.0));
			}
		}
		else if(attackPhase==STATE_TURNAWAY)
		{
			const double maxG=5.0;
			// Neutral bank: 1/cos(bnk)=maxG -> bank0=acos(1/maxG)
			// Vertical Speed-Desired Vertical Speed
			//                +5m/s -> 90deg
			//                 0m/s -> bank0
			//                -5m/s -> bank0-20deg

			const double desiredAlt=air.GetFieldElevation()+YsUnitConv::FTtoM(800.0);
			const double desiredVSpd=YsBound((desiredAlt-air.GetPosition().y())/50.0,-5.0,5.0);

			const double vSpd=air.Prop().GetClimbRatio()-desiredVSpd;
			const double bank0=acos(1.0/maxG);

			double bank;
			if(0.0<vSpd)
			{
				const double bankMax=YsPi/2.0;
				const double t=YsBound(vSpd/7.0,0.0,1.0); // vSpd=7.0 => t=1.0 => bankMax
				bank=bankMax*t+bank0*(1.0-t);
			}
			else
			{
				const double bankMin=bank0-YsPi/9.0;
				const double t=YsBound(-vSpd/7.0,0.0,1.0); // vSpd=-7.0 => t=1.0 => bankMin
				bank=bankMin*t+bank0*(1.0-t);
			}

			if(0<turnAwayTurnDir)
			{
				air.Prop().BankController(bank);
			}
			else
			{
				air.Prop().BankController(-bank);
			}
			air.Prop().GControllerSmooth(5.0,2.0,dt);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);
			air.Prop().SetSpoiler(0.0);
			air.Prop().SetBrake(0.0);
		}
	}

	air.Prop().SmartRudder(dt);

	prevDist=horizontalDist;
	prevEstimate=estimate;

	return YSOK;
}

YSBOOL FsGroundAttack::OutOfThreatRange(FsAirplane &air,const double margin) const
{
	for(auto &t : gndToAirThreat)
	{
		if((air.GetPosition()-t.pos).GetLength()<t.range+margin)
		{
			return YSFALSE;
		}
	}
	return YSTRUE;
}

void FsGroundAttack::SearchTarget(FsAirplane &air,FsSimulation *sim)
{
	FsGround *can,*tmp;
	double canDist,tmpDist;


	// if pendingGndTargetName is not empty, search for that target
	if(pendingGndTargetName[0]!=0)
	{
		tmp=NULL;
		while((tmp=sim->FindNextGround(tmp))!=NULL)
		{
			if(tmp->IsAlive()==YSTRUE && strcmp(pendingGndTargetName,tmp->name)==0)
			{
				target=tmp;
				return;
			}
		}
	}

	YsVec3 refPos=air.GetPosition();
	{
		const FsGround *playerGround=sim->FindGround(playerGroundKey);
		if(NULL!=playerGround)
		{
			refPos=playerGround->GetPosition();
		}
	}

	// First, let's look for a primary target.
	can=NULL;
	canDist=YsInfinity;
	tmp=NULL;
	while((tmp=sim->FindNextGround(tmp))!=NULL)
	{
		if(tmp->IsAlive()==YSTRUE &&
		   tmp->iff!=air.iff &&
		   tmp->primaryTarget==YSTRUE &&
		   tmp->SearchKey()!=playerGroundKey)
		{
			tmpDist=(tmp->GetPosition()-refPos).GetLength();
			if(can==NULL || tmpDist<canDist)
			{
				can=tmp;
				canDist=tmpDist;
			}
		}
	}
	if(can!=NULL)
	{
		target=can;
		return;
	}


	{
		FsGround *playerGround=sim->FindGround(playerGroundKey);
		if(NULL!=playerGround &&
		   YSTRUE==playerGround->IsAlive() &&
		   air.iff!=playerGround->iff &&
		   YSTRUE==playerGround->primaryTarget)
		{
			target=playerGround;
			return;
		}
	}


	// If not found, let's look for any target.
	can=NULL;
	canDist=YsInfinity;
	while((tmp=sim->FindNextGround(tmp))!=NULL)
	{
		if(tmp->IsAlive()==YSTRUE && tmp->iff!=air.iff)
		{
			tmpDist=(tmp->GetPosition()-air.GetPosition()).GetLength();
			if(can==NULL || tmpDist<canDist)
			{
				can=tmp;
				canDist=tmpDist;
			}
		}
	}

	if(can!=NULL)
	{
		target=can;
	}
}


YSBOOL FsGroundAttack::BeingChasedByEnemy(FsAirplane &air,FsSimulation *sim)
{
	FsAirplane *enemy;

	enemy=NULL;
	while((enemy=sim->FindNextAirplane(enemy))!=NULL)
	{
		if(enemy->iff!=air.iff)
		{
			YsVec3 ePos,eEv,eUv;
			YsAtt3 eAtt;

			ePos=enemy->GetPosition();
			eAtt=enemy->GetAttitude();
			eEv=eAtt.GetForwardVector();
			eUv=eAtt.GetUpVector();

			air.Prop().GetInverseMatrix().Mul(ePos,ePos,1.0);
			air.Prop().GetInverseMatrix().Mul(eEv,eEv,0.0);
			air.Prop().GetInverseMatrix().Mul(eUv,eUv,0.0);

			if(-900.0<ePos.z() && ePos.z()<0.0 &&
			   YsAbs(ePos.x()/ePos.z())<0.5 &&
			   YsAbs(ePos.y()/ePos.z())<0.5 &&
			   eEv.z()>0.0 &&
			   YsAbs(eEv.x()/eEv.z())<0.5 &&
			   YsAbs(eEv.y()/eEv.z())<0.5)
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

FsGroundAttack::STATE FsGroundAttack::Phase(void) const
{
	return attackPhase;
}

void FsGroundAttack::SetPhase(FsAirplane &air,FsGroundAttack::STATE phase)
{
	attackPhase=phase;
	air.Prop().CaptureGControllerSmoother();
}
