#include <ysclass.h>
#include "fs.h"
#include "fsautodrive.h"
#include <ysunitconv.h>



FsAcroTypeName::FsAcroTypeName()
{
	Set(FSACRO_NONE,NULL);
}

void FsAcroTypeName::Set(int t,const char n[])
{
	acroType=t;
	acroName=n;
}

FsAcroTypeNameTable::FsAcroTypeNameTable()
{
	typeNameList.Set(0,NULL);
	Add(FSACRO_CORKSCREW,"CorkScrew");
	Add(FSACRO_BIGHEART,"BigHeart");
	Add(FSACRO_ROLLONTAKEOFFANDHALFCUBAN,"RollOnTakeOffAndHalfCuban");
	Add(FSACRO_SLOWROLL,"SlowRoll");
	Add(FSACRO_DELTALOOP,"DeltaLoop");
	Add(FSACRO_DELTAROLL,"DeltaRoll");
	Add(FSACRO_TIGHTTURN,"TightTurn");
	Add(FSACRO_360ANDLOOP,"360andLoop");
	Add(FSACRO_CUBANEIGHT,"CubanEight");
	Add(FSACRO_ROCKWINGCLEAN,"RockWingPass-Clean");
	Add(FSACRO_ROCKWINGDIRTY,"RockWingPass-Dirty");
	Add(FSACRO_BOMBBURST4SHIP,"BombBurstUpward-4Ship");
	Add(FSACRO_BOMBBURST6SHIP,"BombBurstUpward-6Ship");
	Add(FSACRO_CHANGEOVERTURN,"ChangeOverTurn");
	Add(FSACRO_TRAILTODIAMONDROLL,"TrailToDiamondRoll");
	Add(FSACRO_DELTALOOPANDBONTON,"DeltaLoopAndBonton");
	Add(FSACRO_BONTONROLL,"BontonRoll");
	Add(FSACRO_BOMBBURSTDOWN4SHIP,"BombBurstDownward-4Ship");
	Add(FSACRO_BOMBBURSTDOWN6SHIP,"BombBurstDownward-6Sihp");
	Add(FSACRO_ROLLINGCOMBATPITCH,"RollingCombatPitch");
	Add(FSACRO_DIAMONDTAKEOFF,"DiamondTakeoff");
	Add(FSACRO_CONTINUOUSROLL,"ContinuousRoll");
	Add(FSACRO_TACKCROSSANDVERTICALCLIMBROLL,"TackCrossAndVerticalClimbRoll");
	Add(FSACRO_LEVELBREAK,"LevelBreak");
	Add(FSACRO_ROLLBACKTOARROWHEAD,"RollBackToArrowhead");
	Add(FSACRO_PITCHUPBREAK,"PitchUpBreak");
	Add(FSACRO_RAINFALL,"RainFall");
	Add(FSACRO_LETTEREIGHT,"LetterEight");
	Add(FSACRO_STARCROSS,"StarCross");
	Add(FSACRO_LEVELOPENER,"LevelOpener");
	Add(FSACRO_FORMATIONBREAK,"FormationBreak");
	Add(FSACRO_LINEABREASTLOOP,"LineAbreastLoop");
	Add(FSACRO_LINEABREASTROLL,"LineAbreastRoll");
	Add(FSACRO_DOUBLEFARVEL,"DoubleFarvel");
	Add(FSACRO_DIAMOND9TOSWANBEND,"Diamond9ToSwanBend");
	Add(FSACRO_SWANTOAPOLLOROLL,"SwanToApolloRoll");
	Add(FSACRO_LANCASTERTO5_4SPLIT,"LancasterTo5-4Split");
	Add(FSACRO_CHAMPAIGNSPLIT,"ChampaignSplit");
	Add(FSACRO_VIXENBREAK,"VixenBreak");
	Add(FSACRO_BIGBATTLETOSHORTDIAMONDLOOP,"BigBattleToShortDiamondLoop");
	Add(FSACRO_STAROFDAVID,"StarOfDavid");
}

void FsAcroTypeNameTable::Add(int t,const char n[])
{
	FsAcroTypeName atn;
	atn.Set(t,n);
	typeNameList.Append(atn);
}

void FsAcroTypeNameTable::Verify(void)
{
	int i;
	printf("Verifying FsAcroTypeNameTable\n");
	for(i=1; i<FSACRO_NUMACROTYPE; i++)
	{
		printf("%03d ",i);
		int j;
		YSBOOL ok;
		ok=YSFALSE;
		for(j=0; typeNameList.GetN(); j++)
		{
			if(i==typeNameList[j].acroType)
			{
				ok=YSTRUE;
				printf("%s\n",typeNameList[j].acroName);
				break;
			}
		}
		if(ok!=YSTRUE)
		{
			fsStderr.Printf("Acro Type %d is not included in the table.",i);
		}
	}
	printf("Done.\n");
}

int FsAcroTypeNameTable::GetAcroType(const char *str)
{
	int i;
	for(i=0; i<typeNameList.GetN(); i++)
	{
		if(strcmp(typeNameList[i].acroName,str)==0)
		{
			return typeNameList[i].acroType;
		}
	}
	return -1;
}

const char *FsAcroTypeNameTable::GetAcroName(int acroType)
{
	int i;
	for(i=0; i<typeNameList.GetN(); i++)
	{
		if(typeNameList[i].acroType==acroType)
		{
			return typeNameList[i].acroName;
		}
	}
	return NULL;
}





FsAirshowControl::FsAirshowControl()
{
	int i;
	for(i=0; i<NFORMATION; i++)
	{
		formation[i]=NULL;
	}
	InitializeParameter();
}

/* static */ FsAirshowControl *FsAirshowControl::Create(void)
{
	return new FsAirshowControl;
}

void FsAirshowControl::InitializeParameter(void)
{
	int i;

	action=NOACTION;
	endOfAction=YSFALSE;
	waitTimer=4.0;
	smokeOffTimer=0.0;
	fomPosition=1;
	pitchIntegrator=0.0;
	rollIntegrator=0.0;
	headingIntegrator=0.0;
	entryAlt=0.0;
	entrySpeed=0.0;
	turnDir=1.0;

	cswRefPoint=YsOrigin();
	cswRefDirection=YsOrigin();
	cswRadius=30.0;
	cswMode=0;

	brlMode=0;
	brlRate=YsPi*2.0/15.0;
	brlG=1.0;

	loopMode=0;
	loopG=4.0;
	loopThr=0.9;
	for(i=0; i<18; i++)
	{
		loopPathAltitude[i]=0.0;
	}

	turnMode=0;
	turnBankAngle=YsPi/3.0;
	turnAndLoop=YSFALSE;
	turnEntrySpeed=0.0;
	letterEightBoost=0;

	bombBurstMode=0;
	bombBurstLevelSpeed=0.0;
	bombBurstEntrySpeed=0.0;
	bombBurstDesigUv1=YsOrigin();
	bombBurstDesigUv2=YsOrigin();
	bombBurstEntryPoint=YsOrigin();
	bombBurstBreakPoint=YsOrigin();
	for(i=0; i<9; i++)
	{
		bombBurstClimbPath[i]=YsOrigin();
	}

	changeOverTurnMode=0;
	changeOverTurnAltitude=0.0;
	changeOverTurnEntryHeading=0.0;
	changeOverTurnEntrySpeed=0.0;

	cpMode=0;
	cpLeftTurn=YSTRUE;

	dtoMode=0;
	dtoSpeed=0.0;
	dtoStartAlt=0.0;

	tcMode=0;
	tcEntryHeading=0.0;

	rwMode=0;
	rwDirty=YSFALSE;

	ivMode=0;
}

YSRESULT FsAirshowControl::GetDeltaPosition(YsVec3 &pos,int no,const double &r)
{
	switch(no)
	{
	case 1:
		pos.Set(0.0,0.0,0.0);
		break;
	case 2:
		pos.Set(-1.0,-0.2,-1.0);
		break;
	case 3:
		pos.Set( 1.0,-0.2,-1.0);
		break;
	case 4:
		pos.Set( 0.0,-0.4,-2.0);
		break;
	case 5:
		pos.Set(-2.0,-0.4,-2.0);
		break;
	case 6:
		pos.Set( 2.0,-0.4,-2.0);
		break;
	default:
		pos=YsOrigin();
		return YSERR;
	}
	pos*=r;
	return YSOK;
}

YSRESULT FsAirshowControl::GetDiamond9Position(YsVec3 &pos,int no,const double &r)
{
	switch(no)
	{
	case 1:
		pos.Set(0.0,0.0,0.0);
		break;
	case 2:
		pos.Set(-1.0,-0.2,-1.0);
		break;
	case 3:
		pos.Set( 1.0,-0.2,-1.0);
		break;
	case 4:
		pos.Set( 0.0,-0.4,-2.0);
		break;
	case 5:
		pos.Set(-2.0,-0.4,-2.0);
		break;
	case 6:
		pos.Set( 2.0,-0.4,-2.0);
		break;
	case 7:
		pos.Set(-1.0,-0.6,-3.0);
		break;
	case 8:
		pos.Set( 1.0,-0.6,-3.0);
		break;
	case 9:
		pos.Set( 0.0,-0.8,-4.0);
		break;

	default:
		pos=YsOrigin();
		return YSERR;
	}
	pos*=r;
	return YSOK;
}

YSRESULT FsAirshowControl::GetArrowheadPosition(YsVec3 &pos,int no,const double &r)
{
	switch(no)
	{
	case 1:
		pos.Set(0.0,0.0,0.0);
		break;
	case 2:
		pos.Set(-1.0,-0.6,-2.8);
		break;
	case 3:
		pos.Set( 1.0,-0.6,-2.8);
		break;
	case 4:
		pos.Set( 0.0,-0.4,-1.8);
		break;
	default:
		pos=YsOrigin();
		return YSERR;
	}
	pos*=r;
	return YSERR;
}

YSRESULT FsAirshowControl::GetLancasterPosition(YsVec3 &pos,int no,const double &r)
{
	switch(no)
	{
	case 1:
		pos.Set(0.0,0.0,0.0);
		break;
	case 2:
		pos.Set(-1.0,-0.2,-1.0);
		break;
	case 3:
		pos.Set( 1.0,-0.2,-1.0);
		break;
	case 4:
		pos.Set(-2.8,-0.2,-1.0);
		break;
	case 5:
		pos.Set( 2.8,-0.2,-1.0);
		break;

	case 6:
		pos.Set( 0.0,-0.4,-2.0);
		break;
	case 7:
		pos.Set( 0.0,-0.6,-4.0);
		break;
	case 8:
		pos.Set(-1.0,-0.8,-5.0);
		break;
	case 9:
		pos.Set( 1.0,-0.8,-5.0);
		break;


	default:
		pos=YsOrigin();
		return YSERR;
	}
	pos*=r;
	return YSOK;
}

YSRESULT FsAirshowControl::GetChampaignPosition(YsVec3 &pos,int no,const double &r)
{
	switch(no)
	{
	case 1:
		pos.Set(0.0,0.0,0.0);
		break;
	case 2:
		pos.Set(-1.0,-0.2,-1.0);
		break;
	case 3:
		pos.Set( 1.0,-0.2,-1.0);
		break;
	case 4:
		pos.Set(-2.0,-0.4,-2.0);
		break;
	case 5:
		pos.Set( 2.0,-0.4,-2.0);
		break;

	case 6:
		pos.Set(-1.0,-0.6,-4.0);
		break;
	case 7:
		pos.Set( 1.0,-0.6,-4.0);
		break;
	case 8:
		pos.Set(-1.0,-0.8,-6.0);
		break;
	case 9:
		pos.Set( 1.0,-0.8,-6.0);
		break;

	default:
		pos=YsOrigin();
		return YSERR;
	}
	pos*=r;
	return YSOK;
}

YSRESULT FsAirshowControl::GetSwan9Position(YsVec3 &pos,int no,const double &r)
{
	switch(no)
	{
	case 1:
		pos.Set(0.0,0.0,0.0);
		break;
	case 2:
		pos.Set(-1.0,-1.0,-4.8);
		break;
	case 3:
		pos.Set( 1.0,-1.0,-4.8);
		break;
	case 4:
		pos.Set( 0.0,-0.4,-2.0);
		break;
	case 5:
		pos.Set(-3.0,-1.4,-6.4);
		break;
	case 6:
		pos.Set( 3.0,-1.4,-6.4);
		break;
	case 7:
		pos.Set(-2.0,-1.2,-5.6);
		break;
	case 8:
		pos.Set( 2.0,-1.2,-5.6);
		break;
	case 9:
		pos.Set( 0.0,-0.8,-4.0);
		break;

	default:
		pos=YsOrigin();
		return YSERR;
	}
	pos*=r;
	return YSOK;
}

YSRESULT FsAirshowControl::GetApollo9Position(YsVec3 &pos,int no,const double &r)
{
	switch(no)
	{
	case 1:
		pos.Set(0.0,0.0,0.0);
		break;
	case 2:
		pos.Set(-1.0,-0.2,-1.0);
		break;
	case 3:
		pos.Set( 1.0,-0.2,-1.0);
		break;
	case 4:
		pos.Set( 0.0,-0.4,-2.0);
		break;
	case 5:
		pos.Set(-2.0,-0.8,-4.0);
		break;
	case 6:
		pos.Set( 2.0,-0.8,-4.0);
		break;
	case 7:
		pos.Set(-1.0,-0.6,-3.0);
		break;
	case 8:
		pos.Set( 1.0,-0.6,-3.0);
		break;
	case 9:
		pos.Set( 0.0,-0.8,-4.0);
		break;

	default:
		pos=YsOrigin();
		return YSERR;
	}
	pos*=r;
	return YSOK;
}

FsFormation *FsAirshowControl::GetFormationAP(FsAirplane *air)
{
	FsAutopilot *ap;
	if(air!=NULL && (ap=air->GetAutopilot())!=NULL)
	{
		if(ap->Type()==FSAUTOPILOT_FORMATION || // ap->WhatItIs()==FsFormation::ClassName || 
		   ap->Type()==FSAUTOPILOT_AIRSHOW)     // ap->WhatItIs()==FsAirshowControl::ClassName)
		{
			return (FsFormation *)ap;
		}
	}
	return NULL;
}

FsAirshowControl *FsAirshowControl::GetAirshowAP(FsAirplane *air)
{
	FsAutopilot *ap;
	if(air!=NULL && (ap=air->GetAutopilot())!=NULL)
	{
		if(ap->Type()==FSAUTOPILOT_AIRSHOW) // ap->WhatItIs()==FsAirshowControl::ClassName)
		{
			return (FsAirshowControl *)ap;
		}
	}
	return NULL;
}

YSRESULT FsAirshowControl::SwitchLeader(FsAirplane *newLeader,FsAirplane *newWingman)
{
	if(newLeader!=NULL && newWingman!=NULL)
	{
		const YsVec3 &leadPos=newLeader->GetPosition();
		const YsAtt3 &leadAtt=newLeader->GetAttitude();
		YsVec3 newRelPos;

		FsAutopilot *ap;
		FsFormation *fom;
		if(newWingman!=NULL && (ap=newWingman->GetAutopilot())!=NULL)
		{
			if(ap->Type()==FSAUTOPILOT_FORMATION || // strcmp(ap->WhatItIs(),FsFormation::ClassName)==0 ||
			   ap->Type()==FSAUTOPILOT_AIRSHOW)     // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
			{
				fom=(FsFormation *)ap;
				leadAtt.MulInverse(newRelPos,newWingman->GetPosition()-leadPos);
				fom->leader=newLeader;
				fom->shouldBe=newRelPos;
				return YSOK;
			}
		}
	}
	return YSERR;
}

unsigned FsAirshowControl::OverridedControl(void)
{
	if(action==NOACTION)
	{
		return FsFormation::OverridedControl();
	}
	return FSAPPLYCONTROL_NONE;
}

YSRESULT FsAirshowControl::MakePriorityDecision(FsAirplane &air)
{
	return FsFormation::MakePriorityDecision(air);
}

YSRESULT FsAirshowControl::MakeDecision(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	switch(action)
	{
	case CORKSCREW:
		return MakeDecisionCorkScrew(air,sim,dt);
	case BARRELROLL:
	case TRAILTODIAMONDROLL:
		return MakeDecisionBarrelRoll(air,sim,dt);
	case DELTALOOPANDBONTON:
	case LOOP:
	case LINEABREASTLOOP:
		return MakeDecisionLoop(air,sim,dt);
	case TURN360:
		return MakeDecision360Turn(air,sim,dt);
	case BOMBBURST4SHIP:
		return MakeDecision4ShipBombBurst(air,sim,dt);
	case CHANGEOVERTURN:
		return MakeDecisionChangeOverTurn(air,sim,dt);
	case CUBANEIGHT:
		return MakeDecisionCubanEight(air,sim,dt);
	case BONTONROLL:
		return MakeDecisionBontonRoll(air,sim,dt);
	case BOMBBURSTDOWN4SHIP:
	case BOMBBURSTDOWN6SHIP:
	case RAINFALL:
		return MakeDecisionBombBurstDownward(air,sim,dt);
	case ROLLINGCOMBATPITCH:
		return MakeDecisionRollingCombatPitch(air,sim,dt);
	case DIAMONDTAKEOFF:
		return MakeDecisionDiamondTakeoff(air,sim,dt);
	case CONTINUOUSROLL:
		return MakeDecisionContinuousRoll(air,sim,dt);
	case ROLLONTAKEOFFANDHALFCUBAN:
		return MakeDecisionRollOnTakeoff(air,sim,dt);
	case TACKCROSSANDVERTICALCLIMBROLL:
		return MakeDecisionTackCrossAndVerticalClimbRoll(air,sim,dt);
	case BIGHEART:
		return MakeDecisionBigHeart(air,sim,dt);
	case LEVELBREAK:
		return MakeDecisionLevelBreak(air,sim,dt);
	case ROLLBACKTOARROWHEAD:
		return MakeDecisionRollBackToArrowhead(air,sim,dt);
	case PITCHUPBREAK:
		return MakeDecisionPitchUpBreak(air,sim,dt);
	case ROCKWING:
		return MakeDecisionRockWing(air,sim,dt);
	case LETTEREIGHT:
		return MakeDecisionLetterEight(air,sim,dt);
	case STARCROSS:
	case BOMBBURST6SHIP:
	case STAROFDAVID:
		return MakeDecisionStarCross(air,sim,dt);
	case LEVELOPENER:
		return MakeDecisionLevelOpener(air,sim,dt);
	case FORMATIONBREAK:
		return MakeDecisionFormationBreak(air,sim,dt);
	case DOUBLEFARVEL:
	case MIRRORROLL:
		return MakeDecisionDoubleFarvel(air,sim,dt);
	case DIAMOND9TOSWANBEND:
		return MakeDecisionDiamond9ToSwanBend(air,sim,dt);
	case SWANTOAPOLLOROLL:
		return MakeDecisionSwanToApolloRoll(air,sim,dt);
	case LANCASTERTO5_4SPLIT:
		return MakeDecisionLancasterTo5_4Split(air,sim,dt);
	case CHAMPAIGNSPLIT:
		return MakeDecisionChampaignSplit(air,sim,dt);
	case VIXENBREAK:
		return MakeDecisionVixenBreak(air,sim,dt);
	case BIGBATTLETOSHORTDIAMONDLOOP:
		return MakeDecisionBigBattleToShortDiamondLoop(air,sim,dt);
	}
	return FsFormation::MakeDecision(air,sim,dt);
}

YSRESULT FsAirshowControl::ApplyControl(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	air.Prop().NeutralDirectAttitudeControl();

	switch(action)
	{
	case NOACTION:
		return FsFormation::ApplyControl(air,sim,dt);
	case CORKSCREW:
		return ApplyControlCorkScrew(air,sim,dt);
	case BARRELROLL:
	case TRAILTODIAMONDROLL:
		return ApplyControlBarrelRoll(air,sim,dt);
	case DELTALOOPANDBONTON:
	case LOOP:
	case LINEABREASTLOOP:
		return ApplyControlLoop(air,sim,dt);
	case TURN360:
		return ApplyControl360Turn(air,sim,dt);
	case BOMBBURST4SHIP:
		return ApplyControl4ShipBombBurst(air,sim,dt);
	case CHANGEOVERTURN:
		return ApplyControlChangeOverTurn(air,sim,dt);
	case CUBANEIGHT:
		return ApplyControlCubanEight(air,sim,dt);
	case BONTONROLL:
		return ApplyControlBontonRoll(air,sim,dt);
	case BOMBBURSTDOWN4SHIP:
	case BOMBBURSTDOWN6SHIP:
	case RAINFALL:
		return ApplyControlBombBurstDownward(air,sim,dt);
	case ROLLINGCOMBATPITCH:
		return ApplyControlRollingCombatPitch(air,sim,dt);
	case DIAMONDTAKEOFF:
		return ApplyControlDiamondTakeoff(air,sim,dt);
	case CONTINUOUSROLL:
		return ApplyControlContinuousRoll(air,sim,dt);
	case ROLLONTAKEOFFANDHALFCUBAN:
		return ApplyControlRollOnTakeoff(air,sim,dt);
	case TACKCROSSANDVERTICALCLIMBROLL:
		return ApplyControlTackCrossAndVerticalClimbRoll(air,sim,dt);
	case BIGHEART:
		return ApplyControlBigHeart(air,sim,dt);
	case LEVELBREAK:
		return ApplyControlLevelBreak(air,sim,dt);
	case ROLLBACKTOARROWHEAD:
		return ApplyControlRollBackToArrowhead(air,sim,dt);
	case PITCHUPBREAK:
		return ApplyControlPitchUpBreak(air,sim,dt);
	case ROCKWING:
		return ApplyControlRockWing(air,sim,dt);
	case LETTEREIGHT:
		return ApplyControlLetterEight(air,sim,dt);
	case STARCROSS:
	case BOMBBURST6SHIP:
	case STAROFDAVID:
		return ApplyControlStarCross(air,sim,dt);
	case LEVELOPENER:
		return ApplyControlLevelOpener(air,sim,dt);
	case FORMATIONBREAK:
		return ApplyControlFormationBreak(air,sim,dt);
	case DOUBLEFARVEL:
	case MIRRORROLL:
		return ApplyControlDoubleFarvel(air,sim,dt);
	case DIAMOND9TOSWANBEND:
		return ApplyControlDiamond9ToSwanBend(air,sim,dt);
	case SWANTOAPOLLOROLL:
		return ApplyControlSwanToApolloRoll(air,sim,dt);
	case LANCASTERTO5_4SPLIT:
		return ApplyControlLancasterTo5_4Split(air,sim,dt);
	case CHAMPAIGNSPLIT:
		return ApplyControlChampaignSplit(air,sim,dt);
	case VIXENBREAK:
		return ApplyControlVixenBreak(air,sim,dt);
	case BIGBATTLETOSHORTDIAMONDLOOP:
		return ApplyControlBigBattleToShortDiamondLoop(air,sim,dt);
	}
	return FsFormation::ApplyControl(air,sim,dt);
}

YSRESULT FsAirshowControl::MakeDecisionCorkScrew(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(cswMode==0)
	{
		YsVec3 refPos,relRefPos;

		if(leader!=NULL)
		{
			cswRefPoint=leader->GetPosition();
			leader->Prop().GetVelocity(cswRefDirection);
		}
		cswRefDirection.Normalize();

		if(YsGetNearestPointOnLine3(refPos,cswRefPoint,cswRefPoint+cswRefDirection,pos)==YSOK)
		{
			//   In general, up vector must point to the center line.  However,
			//   When the airplane's nose direction is parallel to the plane
			//   made by the airplane location and the center line, the up vector
			//   should not point the center line, because the airplane will
			//   hit the center line in this configuration.  So, in that case,
			//   dBank must be corrected.
			YsVec3 nom,ev,uv;   // Normal of the plane

			nom=(refPos-pos)^cswRefDirection;
			nom.Normalize();
			ev=att.GetForwardVector();  // When nom*uv is zero, airplane shouldn't bank from the plane.
			uv=att.GetUpVector();

			if(uv*nom<0.0)
			{
				nom*=-1.0;
			}

			if(nom*ev>0.258819)  // cos(75deg)
			{
				cswMode=1;
			}
		}
	}
	else if(cswMode==10)
	{
		if(fomPosition==5)
		{
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				waitTimer=20.0;
				cswMode=11;
				FsAutopilot *ap;
				FsAirshowControl *fom;
				if(formation[5]!=NULL && 
				   (ap=formation[5]->GetAutopilot())!=NULL &&
				   ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
				{
					fom=(FsAirshowControl *)ap;
					fom->cswMode=0;
				}
			}
		}
		else if(fomPosition==6)
		{
			return FsFormation::MakeDecision(air,sim,dt);
		}
	}
	else if(cswMode==11)
	{
		if(fomPosition==5)
		{
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				FsAutopilot *ap;
				FsAirshowControl *fom;
				if(formation[5]!=NULL && 
				   (ap=formation[5]->GetAutopilot())!=NULL &&
				   ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
				{
					YsAtt3 wAtt;
					wAtt=formation[5]->GetAttitude();
					if(YsPi/4.0<fabs(wAtt.b()) && fabs(wAtt.b())<YsPi)
					{
						fom=(FsAirshowControl *)ap;
						fom->cswMode=12;
						cswMode=12;
						waitTimer=5.0;
					}
				}
			}
		}
	}
	else if(cswMode==12)
	{
		if(fomPosition==5)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				endOfAction=YSTRUE;
			}
		}
	}

	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlCorkScrew(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(cswMode==12)  // After breaking
	{
		if(fomPosition==5 && formation[5]!=NULL)
		{
			YsAtt3 att;
			att=formation[5]->GetAttitude();
			if(att.b()>0.0)
			{
				air.Prop().BankController(YsPi/3.0);
			}
			else
			{
				air.Prop().BankController(-YsPi/3.0);
			}
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SetThrottle(0.6);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else if(fomPosition==6)
		{
			YsAtt3 att;
			att=air.GetAttitude();
			if(att.b()>0.0)
			{
				air.Prop().BankController(YsPi/4.0);
			}
			else
			{
				air.Prop().BankController(-YsPi/4.0);
			}
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SetThrottle(0.6);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else
		{
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SetThrottle(0.6);
			air.Prop().SetAfterburner(YSFALSE);
			air.Prop().BankController(0.0);
		}
		return YSOK;
	}


	if(fomPosition==5)
	{
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		if(cswMode==10)
		{
			air.Prop().BankController(0.0);
		}
		else if(cswMode==11)
		{
			air.Prop().BankController(YsPi);
		}
		air.Prop().SetSmokeButton(YSTRUE);
		return YSOK;
	}
	else if(fomPosition==6 && cswMode==10)
	{
		return FsFormation::ApplyControl(air,sim,dt);
	}


	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	YsVec3 refPos,relRefPos;

	if(leader!=NULL)
	{
		cswRefPoint=leader->GetPosition();
		leader->Prop().GetVelocity(cswRefDirection);
	}
	cswRefDirection.Normalize();

	if(YsGetNearestPointOnLine3(refPos,cswRefPoint,cswRefPoint+cswRefDirection,pos)==YSOK)
	{
		air.Prop().GetMatrix().MulInverse(relRefPos,refPos,1.0);

		if(cswMode==0)
		{
			double dBank;
			if(relRefPos.x()>0)
			{
				dBank=atan2(relRefPos.y(),relRefPos.x());
			}
			else
			{
				dBank=atan2(-relRefPos.y(),-relRefPos.x());
			}
			air.Prop().BankController(att.b()+dBank);
			air.Prop().GController(4.0);
		}
		else if(cswMode==1)
		{
			air.Prop().TurnOffController();


			YsVec3 absoluteOffset;
			absoluteOffset=pos-refPos;


			YsVec3 V;
			YsVec3 Vp;   // Velocity component that is perpendicular to the reference direction
			YsVec3 Vtan; // Velocity component that is tangential to the ideal circle
			YsVec3 Vcen; // Velocity component that is centrifugal of the ideal circle (Should be close to zero!)
			YsVec3 Dcen;
			YsVec3 VpN;

			air.Prop().GetVelocity(V);
			Vp=V-cswRefDirection*(V*cswRefDirection);
			VpN=Vp;
			VpN.Normalize();

			Dcen=refPos-pos;
			Dcen.Normalize();
			Vcen=Dcen*(Vp*Dcen);
			Vtan=Vp-Vcen;

			double Greq,Rcur;
			double dBank;
			double roll,maxRoll,rollCorrection;

			Rcur=relRefPos.GetLength();


			dBank=atan2(-relRefPos.x(),relRefPos.y());

			// Correction to dBank
			//   In general, up vector must point to the center line.  However,
			//   When the airplane's nose direction is parallel to the plane
			//   made by the airplane location and the center line, the up vector
			//   should not point the center line, because the airplane will
			//   hit the center line in this configuration.  So, in that case,
			//   dBank must be corrected.
			// YsVec3 nom,ev;   // Normal of the plane

			// nom=(refPos-pos)^cswRefDirection;
			// nom.Normalize();
			// ev=att.GetForwardVector();  // When nom*uv is zero, airplane shouldn't bank from the plane.

//dBank*=YsAbs(nom*ev);
//				if(YsAbs(nom*ev)<0.173648)
//				{
//					double wei;
//					wei=YsAbs(nom*ev)/0.173648;   // 0.173648=cos(10deg)
//printf("* %lf\n",wei);
//					dBank*=wei;
//				}


			// v = radius x ommega  ->  ommega = v / radius
			double ommega;
			ommega=Vtan.GetLength()/cswRadius; // Rcur;
			// So, should I roll left or right?
			{
				YsVec3 posFuture;   // One second later, if I don't roll.
				YsVec3 refPosFuture;
				posFuture=pos+V;

				YsGetNearestPointOnLine3(refPosFuture,cswRefPoint,cswRefPoint+cswRefDirection,posFuture);
				refPosFuture-=posFuture;
				att.MulInverse(refPosFuture,refPosFuture);
				if(refPosFuture.x()>0.0)  // Aha, I should roll right.
				{
					ommega=-ommega;
				}
// printf("FutureX:%lf",refPosFuture.x());
				// else, I should turn left.  Ommega should be positive.
			}

			rollCorrection=dBank*10.0;
			rollCorrection=YsBound(rollCorrection,-YsAbs(ommega)*0.9,YsAbs(ommega)*0.9);
			roll=ommega+rollCorrection;

			air.Prop().GetRollRate(maxRoll);
			air.Prop().SetAileron(roll/maxRoll);

//				roll=dBank*50.0;
//				rollCorrection=YsBound(Rcur/cswRadius,0.3,4.0);  // Faster roll -> tighter barrel
//				air.Prop().GetRollRate(maxRoll);
//				if(air.Prop().GetG()<3.0)  // Why don't I wait until G increases?  <= This is not bad, but not perfect.
//				{
//					roll=0.0;
//				}
//				air.Prop().SetAileron(roll*rollCorrection/maxRoll);

// printf("%lf %lf %lf %lf\n",Rcur,rollCorrection,cswRadius,VpN*Dcen);

			Greq=6.0-Dcen*Vcen+(Rcur-cswRadius)*10.0/cswRadius;

			YsVec3 earthGravityCorrection;
			earthGravityCorrection.Set(0.0,1.0,0.0);
			att.MulInverse(earthGravityCorrection,earthGravityCorrection);
			Greq+=earthGravityCorrection.y();

			Greq=YsBound(Greq,4.0,8.0);

			air.Prop().GController(Greq);
			if(leader==NULL)
			{
				air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.6);
			}
			else
			{
				// Speed along the axis is:
				//   velocity * cos(theata)  where theata is the angle between axis and current V direction.
				double cosTheata;
				YsVec3 v;
				v=V;
				v.Normalize();
				cosTheata=YsAbs(v*cswRefDirection);
				if(cosTheata<=YsTolerance)
				{
					air.Prop().SpeedController(air.Prop().GetEstimatedCruiseSpeed()*0.6);
				}
				else
				{
					YsVec3 relPosToRefAirplane;
					double refAirSpd,desiredAxialSpeed;
					leader->Prop().GetMatrix().MulInverse(relPosToRefAirplane,pos,1.0);
					refAirSpd=leader->Prop().GetVelocity();

					desiredAxialSpeed=refAirSpd+(-60.0-relPosToRefAirplane.z());
					desiredAxialSpeed=YsBound(desiredAxialSpeed,refAirSpd-30.0,refAirSpd+30.0);

					air.Prop().SpeedController(desiredAxialSpeed/cosTheata);
				}
			}

			if(air.Prop().GetSmokeOil()>YsTolerance)
			{
				air.Prop().SetSmokeButton(YSTRUE);
			}

			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsAirshowControl::MakeDecisionBarrelRoll(FsAirplane &air,FsSimulation *,const double &dt)
{
	// This function may be called from MakeDecisionSwanToApolloRoll

	int i0,i;
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(brlMode)
	{
	case 0:
		if(waitTimer>YsTolerance)
		{
			waitTimer-=dt;
		}
		else if(fabs(att.p())<YsDegToRad(3.0) && fabs(att.b())<YsDegToRad(3.0))
		{
			brlMode=1;
			entryAlt=pos.y();
		}
		break;
	case 1:
		if(att.p()>YsDegToRad(25.0))
		{
			brlMode=2;
		}

		if(att.p()>YsDegToRad(15.0) && action==TRAILTODIAMONDROLL)
		{
			double r,x;
			r=air.GetRadiusFromCollision()*0.85;
			x=-r;
			for(i=1; i<=2; i++)
			{
				FsFormation *fom;
				fom=GetFormationAP(formation[i]);
				if(fom!=NULL)
				{
					fom->shouldBe.SetX(x);
				}
				x=-x;
			}
		}

		if(brlG<3.0)
		{
			brlG+=dt;
		}
		break;
	case 2:   // Until inverted completely
		if(fabs(att.b())>YsPi/9.0)
		{
			if(action==SWANTOAPOLLOROLL)
			{
				FsAirshowControl *ap;
				YsVec3 pos;
				double r;

				r=air.GetRadiusFromCollision()*0.85;

				GetApollo9Position(pos,2,r);
				ap=GetAirshowAP(formation[1]);
				if(ap!=NULL)
				{
					ap->shouldBe=pos;
					ap->brlMode=1;
				}

				pos.MulX(-1.0);
				ap=GetAirshowAP(formation[2]);
				if(ap!=NULL)
				{
					ap->shouldBe=pos;
					ap->brlMode=1;
				}
			}
		}
		if(fabs(att.b())>YsPi/4.0)
		{
			if(action==TRAILTODIAMONDROLL)
			{
				double r,y,z;
				r=air.GetRadiusFromCollision()*0.85;
				z=-r;
				y=-2.5;
				for(i=1; i<=3; i++)
				{
					FsFormation *fom;

					if(i==3)
					{
						z=-2.0*r;
						y*=2;
					}

					fom=GetFormationAP(formation[i]);
					if(fom!=NULL)
					{
						fom->shouldBe.SetY(y);
						fom->shouldBe.SetZ(z);
					}
				}
			}
		}
		if(fabs(att.b())>YsPi/2.0)
		{
			if(action==SWANTOAPOLLOROLL)
			{
				FsAirshowControl *ap;
				YsVec3 pos;
				double r;
				int i;

				r=air.GetRadiusFromCollision()*0.85;

				for(i=5; i<=8; i++)
				{
					GetApollo9Position(pos,i,r);
					ap=GetAirshowAP(formation[i-1]);
					if(ap!=NULL)
					{
						ap->shouldBe=pos;
					}
				}
			}
		}

		i0=YsGreater((int)fabs(att.b()/(YsPi/18.0)),0);
		for(i=i0; i<18; i++)
		{
			brlPathAltitude[i]=pos.y();
		}

		if(fabs(att.b())>YsDegToRad(177.0))
		{
			brlMode=3;
		}

		if(brlG<3.0)
		{
			brlG+=dt;
		}
		break;
	case 3:
		if(fabs(att.b())<YsDegToRad(3.0))
		{
			smokeOffTimer=8.0;
			brlMode=10;
		}
		if(brlG<3.0)
		{
			brlG+=dt;
		}
		break;

	case 10:  // Roll out
		if(smokeOffTimer>0.0)
		{
			smokeOffTimer-=dt;
		}
		else
		{
			smokeOffTimer=0.0;
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlBarrelRoll(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	double ail,ailStep;
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(brlMode)
	{
	default:  // 0 & 4
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0,brlRate);
		air.Prop().SetThrottle(0.8);
		break;
	case 10:
		ControlGForAltitude(air,sim,entryAlt,0.0);
		air.Prop().BankController(0.0,brlRate);
		air.Prop().SetThrottle(0.8);
		break;
	case 1:
		air.Prop().GController(brlG);
		air.Prop().BankController(0.0);
		air.Prop().SetThrottle(0.8);
		break;
	case 2:
		air.Prop().GController(brlG);
		air.Prop().TurnOffBankController();
		air.Prop().GetRollRate(ail);
		ail=turnDir*brlRate/ail;
		ailStep=ail*dt;

		if(fabs(att.b())>YsPi/2.0)
		{
			// Must complete inverting before the nose hits the horizon.
			double desigPitch;
			desigPitch=(YsPi-fabs(att.b()))/2.0;
			if(att.p()<desigPitch)
			{
				ail*=1.2;
			}
		}

		if(fabs(air.Prop().GetAileron()-ail)<ailStep)
		{
			air.Prop().SetAileron(ail);
		}
		else if(ail<air.Prop().GetAileron())
		{
			air.Prop().SetAileron(ail-ailStep);
		}
		else
		{
			air.Prop().SetAileron(ail+ailStep);
		}
		air.Prop().SetThrottle(0.8);
		break;
	case 3:
		{
			air.Prop().GetRollRate(ail);
			ail=turnDir*brlRate/ail;

			int i;
			double alt,alt1,alt2,b1,b2;
			i=YsBound((int)(fabs(att.b())/(YsPi/18.0)),0,17);
			if(i>1)
			{
				alt1=brlPathAltitude[i-1];
				b1=(YsPi/18.0)*(double)(i-1);
				alt2=brlPathAltitude[i];
				b2=(YsPi/18.0)*(double)i;
			}
			else
			{
				alt1=entryAlt;
				b1=0.0;
				alt2=brlPathAltitude[1];
				b2=YsPi/18.0;
			}
			alt=alt1+(att.b()-b1)*(alt2-alt1)*(b2-b1);

			air.Prop().TurnOffBankController();
			if(pos.y()<alt)
			{
				if(fabs(att.b())>YsPi/2.0)
				{
					air.Prop().GController(brlG-0.3);
				}

				air.Prop().SetAileron(ail*1.2);
				air.Prop().SetThrottle(0.6);
			}
			else
			{
				if(fabs(att.b())>YsPi/2.0)
				{
					air.Prop().GController(brlG+0.3);
				}

				air.Prop().SetAileron(ail*0.8);
				air.Prop().SetThrottle(0.8);
			}

			YsAtt3 vAtt;
			YsVec3 ev;
			air.Prop().GetVelocity(ev);
			vAtt.SetTwoVector(ev,att.GetUpVector());

			if(fabs(att.b())<YsPi/2.0)
			{
				double corG,desigPitch;
				desigPitch=-fabs(att.b()/2.0);
				if(vAtt.p()<desigPitch)
				{
					corG=YsRadToDeg(desigPitch-vAtt.p());
					corG/=5.0;  // 5.0 deg -> 1G
					corG=YsBound(corG,0.0,2.0);
				}
				else
				{
					corG=0.0;
				}
				air.Prop().GController(brlG+corG);
			}
		}
		break;
	}

	// This function may be called from ApplyControlSwanToApolloRoll
	if(action!=SWANTOAPOLLOROLL && (brlMode!=10 || smokeOffTimer>YsTolerance))
	{
		air.Prop().SetSmokeButton(YSTRUE);
	}
	else
	{
		air.Prop().SetSmokeButton(YSFALSE);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionLoop(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	int i0,i;
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(action==DELTALOOPANDBONTON && fomPosition!=1 && loopMode<11)
	{
		return FsFormation::MakeDecision(air,sim,dt);
	}
	if(action==LINEABREASTLOOP && fomPosition!=1)
	{
		return FsFormation::MakeDecision(air,sim,dt);
	}

	switch(loopMode)
	{
	case 0:
		if(waitTimer>YsTolerance)
		{
			waitTimer-=dt;
		}
		else if(fabs(att.p())<YsDegToRad(3.0) && fabs(att.b())<YsDegToRad(0.8))
		{
			entryAlt=pos.y();
			for(i=0; i<18; i++)
			{
				loopPathAltitude[i]=pos.y();
			}
			loopMode=1;
		}
		break;
	case 1:   // level, vertical up, inverted to -50deg pitch
		if(att.p()>0.0)
		{
			if(fabs(att.b())<YsPi/2.0)  // First 90 degree (level to vertical up)
			{
				i0=YsGreater((int)(att.p()/(YsPi/18.0)),0);
				for(i=i0; i<9; i++)
				{
					loopPathAltitude[i]=pos.y();
				}
			}
			else  // Next 90 degree (vertical up to inverted)
			{
				i0=YsGreater(17-(int)(att.p()/(YsPi/18.0)),9);
				for(i=i0; i<18; i++)
				{
					loopPathAltitude[i]=pos.y();
				}
			}
		}

		if(action==DELTALOOPANDBONTON && fabs(att.b())>YsPi/2.0 && att.p()<0.0)
		{
			int i0,i;
			double ratio[6];

			i0=1;
			ratio[1]=1.0-2.0*(att.p()+YsDegToRad(10.0))/YsDegToRad(60.0);
			ratio[2]=ratio[1];
			ratio[3]=1.0-2.0*att.p()/YsDegToRad(70.0);
			ratio[4]=ratio[3];
			ratio[5]=ratio[3];

			for(i=i0; i<6; i++)
			{
				FsAutopilot *ap;
				FsAirshowControl *fom;
				if(formation[i]!=NULL && 
				   (ap=formation[i]->GetAutopilot())!=NULL && 
				   ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
				{
					fom=(FsAirshowControl *)ap;

					double r;
					YsVec3 pos;
					r=air.GetRadiusFromCollision()*0.85;
					GetDeltaPosition(pos,i+1,r);
					fom->shouldBe=pos*ratio[i];
				}
			}
		}
		if(action==LINEABREASTLOOP)
		{
			FsAirshowControl *ap;
			YsVec3 pos;
			double r;
			if(fabs(att.b())>YsPi/2.0 && att.p()<0.0)
			{
				r=air.GetRadiusFromCollision()*0.85;
				ap=GetAirshowAP(formation[3]);
				GetDeltaPosition(pos,4,r);
				if(ap!=NULL && ap->loopMode==0)
				{
					ap->shouldBe=pos;
					ap->shouldBe.MulY(2.0);

					ap->transition[ap->nTransition++]=pos;

					ap->loopMode=1;
				}
			}
			if(fabs(att.b())>YsPi/2.0 && att.p()<-YsPi/4.0)
			{
				r=air.GetRadiusFromCollision()*0.85;
				ap=GetAirshowAP(formation[1]);
				if(ap!=NULL)
				{
					GetDeltaPosition(pos,2,r);
					ap->shouldBe=pos;
				}
				ap=GetAirshowAP(formation[2]);
				if(ap!=NULL)
				{
					GetDeltaPosition(pos,3,r);
					ap->shouldBe=pos;
				}
			}
		}

		if(att.p()<YsDegToRad(-80.0))
		{
			loopMode=2;
		}
//		if(loopG<4.0)
//		{
//			loopG+=dt;
//		}
		break;
	case 2:
		if(action!=DELTALOOPANDBONTON)
		{
			if(att.p()>YsDegToRad(-2.0))
			{
				loopMode=3;
				smokeOffTimer=6.0;
			}
		}
		else
		{
			if(att.p()>YsDegToRad(-2.0))
			{
				loopMode=10;
			}
		}
//		if(loopG<5.0)
//		{
//			loopG+=dt;
//		}
		break;
	case 3:
		if(smokeOffTimer>0.0)
		{
			smokeOffTimer-=dt;
		}
		else
		{
			smokeOffTimer=0.0;
			endOfAction=YSTRUE;
		}
		break;

	case 10:
		{
			YsVec3 vel;
			air.Prop().GetVelocity(vel);
			if(fabs(vel.y())<1.0)
			{
				YSBOOL stable;
				stable=YSTRUE;
				for(i=0; i<6; i++)
				{
					if(formation[i]!=NULL)
					{
						formation[i]->Prop().GetVelocity(vel);
						if(fabs(vel.y())>1.0)
						{
							stable=YSFALSE;
							break;
						}
					}
				}
				if(stable==YSTRUE)
				{
					loopMode=11;
					for(i=0; i<6; i++)
					{
						FsAutopilot *ap;
						FsAirshowControl *fom;
						if(formation[i]!=NULL && 
						   (ap=formation[i]->GetAutopilot())!=NULL && 
						   ap->Type()==FSAUTOPILOT_AIRSHOW)  // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
						{
							fom=(FsAirshowControl *)ap;
							if(fom->action==DELTALOOPANDBONTON)
							{
								fom->loopMode=11;
							}
						}
					}
				}
			}
		}
		break;

	case 11: // Bonton
		if(fabs(att.b())>YsPi*3.0/4.0 && rollIntegrator<YsTolerance)
		{
			rollIntegrator=YsPi/2.0;
		}
		else if(fabs(att.b())<YsPi/2.0 && rollIntegrator>YsTolerance)
		{
			loopMode=12;
			smokeOffTimer=4.0;
		}
		break;
	case 12:
		smokeOffTimer-=dt;
		if(smokeOffTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}

		if(leader!=NULL && fabs(leader->GetAttitude().b())<YsPi/180.0)
		{
			YsAtt3 vAtt;
			leader->Prop().GetRotationSpeed(vAtt);
			if(fabs(vAtt.b())<YsPi/180.0)
			{
				FsFormation::MakeDecision(air,sim,dt);
			}
		}

		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlLoop(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(action==DELTALOOPANDBONTON && fomPosition!=1 && loopMode<11)
	{
		return FsFormation::ApplyControl(air,sim,dt);
	}
	if(action==LINEABREASTLOOP && fomPosition!=1)
	{
		return FsFormation::ApplyControl(air,sim,dt);
	}

	switch(loopMode)
	{
	default:  // 0 && 4
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0,YsPi/5.0);
		air.Prop().SetThrottle(loopThr);
		air.Prop().SetAfterburner(YSFALSE);
		break;
	case 1:
	case 2:
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		if(att.p()>0.0)
		{
			double g;
			air.Prop().SetThrottle(loopThr);
			g=air.Prop().GetG();
			if(g+0.5<loopG)
			{
				air.Prop().GController(g+0.5);
			}
			else
			{
				air.Prop().GController(loopG);
			}
		}
		else
		{
			int i;
			double lattitude;
			double alt,alt1,alt2,p1,p2;
			if(fabs(att.b())>YsPi/2.0)  // inverted to straight down
			{
				lattitude=YsPi+att.p();
			}
			else  // straight down to rolling out
			{
				lattitude=-att.p();
			}

			i=YsSmaller((int)(lattitude/(YsPi/18.0)),17);  // 17->170deg to 180deg
			if(i>0)
			{
				alt1=loopPathAltitude[i-1];
				p1=(YsPi/18.0)*(double)i;
				alt2=loopPathAltitude[i];
				p2=(YsPi/18.0)*(double)(i+1);
			}
			else
			{
				alt1=entryAlt;
				p1=0.0;
				alt2=loopPathAltitude[0];
				p2=YsPi/18.0;
			}
			alt=alt1+(lattitude-p1)*(alt2-alt1)/(p2-p1);

			if(pos.y()<alt)
			{
				// printf("Low %lf\n",alt-pos.y());
			}
			else
			{
				// printf("High %lf\n",alt-pos.y());
			}

			YsVec3 vel;
			air.Prop().GetVelocity(vel);
			if(vel.y()<0.0)
			{
				double plusG,thr;
				plusG=YsBound((alt-pos.y())/5.0,-0.5,3.0);
				thr=YsBound(0.5-(alt-pos.y())/50.0,0.1,0.9);
				air.Prop().GController(loopG+plusG);
				air.Prop().SetThrottle(thr);
			}
			else
			{
				air.Prop().GController(loopG);
				air.Prop().SetThrottle(loopThr);
			}
		}
		break;
	case 3:
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0);
		air.Prop().SetThrottle(loopThr);
		break;

	case 10:
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0);
		air.Prop().SetThrottle(loopThr);
		break;

	case 11:  // Bonton
		if(fabs(att.b())<YsPi/4.0)
		{
			air.Prop().GController(1.2);
		}
		else if(fabs(att.b())>YsPi*3.0/4.0)
		{
			air.Prop().GController(-0.8);
		}
		else
		{
			air.Prop().GController(0.0);
		}
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(-0.7);
		if(fomPosition!=1 && leader!=NULL)
		{
			air.Prop().SpeedController(leader->Prop().GetVelocity());
		}
		break;
	case 12:
		if(leader!=NULL && fabs(leader->GetAttitude().b())<YsPi/180.0)
		{
			YsAtt3 vAtt;
			leader->Prop().GetRotationSpeed(vAtt);
			if(fabs(vAtt.b())<YsPi/180.0)
			{
				FsFormation::ApplyControl(air,sim,dt);
				break;
			}
		}

		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0,YsPi/3.0);
		air.Prop().SetThrottle(loopThr);
		if(fomPosition!=1 && leader!=NULL)
		{
			air.Prop().SpeedController(leader->Prop().GetVelocity());
		}

		break;
	}

	if((loopMode!=3 && loopMode!=12) || smokeOffTimer>YsTolerance)
	{
		air.Prop().SetSmokeButton(YSTRUE);
	}
	else
	{
		air.Prop().SetSmokeButton(YSFALSE);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecision360Turn(FsAirplane &air,FsSimulation *,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(turnEntrySpeed<YsTolerance)
	{
		turnEntrySpeed=air.Prop().GetVelocity();
	}

	switch(turnMode)
	{
	case 0:
		if(waitTimer>YsTolerance)
		{
			waitTimer-=dt;
		}
		else if(fabs(att.p())<YsDegToRad(3.0) && fabs(att.b())<YsDegToRad(3.0))
		{
			turnMode=1;
			turnEntryHeading=att.h();
			entryAlt=air.GetPosition().y();
		}
		break;
	case 1:
		{
			double hdgDiff;
			YsVec2 v1,v2;
			v1.Set(cos(turnEntryHeading),sin(turnEntryHeading));
			v2.Set(cos(att.h()),sin(att.h()));
			hdgDiff=v1*v2;
			if(hdgDiff<cos(YsPi*17.0/18.0))
			{
				turnMode=2;
			}
		}
		break;
	case 2:
		{
			double hdgDiff;
			YsVec2 v1,v2;
			v1.Set(cos(turnEntryHeading),sin(turnEntryHeading));
			v2.Set(cos(att.h()),sin(att.h()));
			hdgDiff=v1*v2;
			if(hdgDiff>cos(YsDegToRad(15.0)))
			{
				turnMode=3;
				smokeOffTimer=6.0;
				if(turnAndLoop==YSTRUE)
				{
					InitializeParameter();

					action=LOOP;
					entryAlt=0.0;
					waitTimer=2.0;
					air.Prop().TurnOffSpeedController();
				}
			}
		}
		break;
	case 3:
		if(smokeOffTimer>0.0)
		{
			smokeOffTimer-=dt;
		}
		else
		{
			smokeOffTimer=0.0;
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControl360Turn(FsAirplane &air,FsSimulation *sim,const double & /*dt*/)
{
	switch(turnMode)
	{
	default:
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0); // ,YsPi/5.0);
		air.Prop().SpeedController(turnEntrySpeed);
		break;
	case 1:
	case 2:
		air.Prop().BankController(turnDir*turnBankAngle); // ,YsPi/5.0);
		air.Prop().SpeedController(turnEntrySpeed);
		air.Prop().SpeedController(turnEntrySpeed);
		ControlGForAltitude(air,sim,entryAlt,0.0,9.0);
		break;
	case 3:
		{
			YsVec2 v;
			double a,b;
			v.Set(-sin(turnEntryHeading),cos(turnEntryHeading));
			v.Rotate(-air.GetAttitude().h());
			a=-asin(v.x());
			b=YsBound(a*3.0,-turnBankAngle,turnBankAngle);
			air.Prop().BankController(b); // ,YsPi/5.0);
			air.Prop().SpeedController(turnEntrySpeed);
			ControlGForAltitude(air,sim,entryAlt,0.0,9.0);
		}
		break;
	}
	if(turnMode!=3 || smokeOffTimer>YsTolerance)
	{
		air.Prop().SetSmokeButton(YSTRUE);
	}
	else
	{
		air.Prop().SetSmokeButton(YSFALSE);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecision4ShipBombBurst(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(1<=fomPosition && fomPosition<=4 && 5<=bombBurstMode)  // Common for 1 to 4, beyond mode 5
	{
		switch(bombBurstMode)
		{
		case 5:
			if(att.p()<-YsDegToRad(80.0))
			{
				bombBurstMode=6;
			}
			break;
		case 6:
			if(att.p()>-YsDegToRad(70.0))
			{
				bombBurstMode=7;
			}
			break;
		case 7:
		case 8:
			{
				YsVec3 rel;
				YsMatrix4x4 mat;

				YsAtt3 hdg(att);
				hdg.SetP(0.0);
				hdg.SetB(0.0);

				mat.Translate(pos);
				mat.Rotate(hdg);
				mat.Invert();

				rel=mat*bombBurstBreakPoint;
				if(bombBurstMode==7 && att.p()>-YsDegToRad(2.0))
				{
					bombBurstMode=8;
				}
				else if(bombBurstMode==8 && rel.z()<50.0)
				{
					bombBurstMode=9;
					smokeOffTimer=5.0;
				}
			}
			break;
		case 9:
			if(smokeOffTimer>0.0)
			{
				smokeOffTimer-=dt;
			}
			if(smokeOffTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
	}
	else if(fomPosition==1)
	{
		int i,j,n;
		switch(bombBurstMode)
		{
		case 0:
			if(waitTimer>YsTolerance)
			{
				waitTimer-=dt;
			}
			else if(fabs(att.b())<YsDegToRad(3.0))
			{
				bombBurstMode=1;
				bombBurstEntryPoint=pos;

				for(i=1; i<NFORMATION; i++)
				{
					if(formation[i]!=NULL)
					{
						FsAutopilot *ap;
						FsAirshowControl *show;
						ap=formation[i]->GetAutopilot();
						if(ap!=NULL && 
						   ap->Type()==FSAUTOPILOT_AIRSHOW) //strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
						{
							show=(FsAirshowControl *)ap;
							show->bombBurstEntryPoint=bombBurstEntryPoint;
							show->bombBurstEntrySpeed=air.Prop().GetVelocity();
							for(j=0; j<9; j++)
							{
								show->bombBurstClimbPath[j]=air.GetPosition();
							}
						}
					}
				}
			}
			break;
		case 1:
			if(att.p()>YsDegToRad(87.0))
			{
				double rr;
				bombBurstMode=2;
				bombBurstBreakPoint=pos;
				air.Prop().GetRollRate(rr);
				waitTimer=YsPi/rr;

				for(i=1; i<NFORMATION; i++)
				{
					if(formation[i]!=NULL)
					{
						FsAutopilot *ap;
						FsAirshowControl *show;
						ap=formation[i]->GetAutopilot();
						if(ap!=NULL && 
						   ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
						{
							show=(FsAirshowControl *)ap;
							show->bombBurstEntryPoint=bombBurstEntryPoint;
							show->bombBurstBreakPoint=bombBurstBreakPoint;

							YsVec3 ref;
							switch(i)
							{
							case 1:  // #2
								ref.Set(-10000.0,0.0,0.0);
								att.Mul(ref,ref);
								show->bombBurstRefPoint=pos+ref;
								show->bombBurstRefPoint.SetY(0.0);
								break;
							case 2:  // #3
								ref.Set( 10000.0,0.0,0.0);
								att.Mul(ref,ref);
								show->bombBurstRefPoint=pos+ref;
								show->bombBurstRefPoint.SetY(0.0);
								break;
							case 3:  // #4
								ref.Set(0.0,-10000.0,0.0);
								att.Mul(ref,ref);
								show->bombBurstRefPoint=pos+ref;
								show->bombBurstRefPoint.SetY(0.0);
								break;
							}
						}
					}
				}
			}
			n=(int)(air.GetAttitude().p()*9.0/(YsPi/2.0));
			for(i=1; i<NFORMATION; i++)
			{
				if(formation[i]!=NULL)
				{
					FsAutopilot *ap;
					FsAirshowControl *show;
					ap=formation[i]->GetAutopilot();
					if(ap!=NULL && 
					   ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
					{
						show=(FsAirshowControl *)ap;
						for(j=n; j<9; j++)
						{
							show->bombBurstClimbPath[j]=air.GetPosition();
						}
					}
				}
			}
			break;
		case 2:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				bombBurstMode=3;
			}
			break;
		case 3:
			if(att.p()<YsDegToRad(3.0))
			{
				bombBurstMode=4;
				bombBurstLevelSpeed=air.Prop().GetVelocity();
				waitTimer=10.0;
			}
			break;
		case 4:
			waitTimer-=dt;
			if(waitTimer<=YsTolerance)
			{
				bombBurstMode=5;
				int i;
				for(i=1; i<4; i++)
				{
					if(formation[i]!=NULL)
					{
						FsAutopilot *ap;
						FsAirshowControl *show;
						ap=formation[i]->GetAutopilot();
						if(ap!=NULL && 
						   ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
						{
							show=(FsAirshowControl *)ap;
							show->bombBurstMode=5;
							show->bombBurstLevelSpeed=bombBurstLevelSpeed;
						}
					}
				}
			}
			break;
		}
	}
	else if(2<=fomPosition && fomPosition<=4)
	{
		switch(bombBurstMode)
		{
		case 0:
			FsFormation::MakeDecision(air,sim,dt);
			if(att.p()<YsDegToRad(1.0) || bombBurstEntryPoint==YsOrigin())
			{
				bombBurstEntryPoint=pos;
			}

			if(att.p()>YsDegToRad(87.0))
			{
				YsAtt3 newAtt;
				newAtt=att;
				switch(fomPosition)
				{
				case 2:
					newAtt.SetB(newAtt.b()+YsPi/2.0);
					bombBurstDesigUv1=newAtt.GetUpVector();
					bombBurstDesigUv2=bombBurstDesigUv1;
					break;
				case 3:
					newAtt.SetB(newAtt.b()-YsPi/2.0);
					bombBurstDesigUv1=newAtt.GetUpVector();
					bombBurstDesigUv2=bombBurstDesigUv1;
					break;
				case 4:
					newAtt.SetB(newAtt.b()+YsPi/2.0);
					bombBurstDesigUv1=newAtt.GetUpVector();
					newAtt.SetB(newAtt.b()+YsPi/2.0);
					bombBurstDesigUv2=newAtt.GetUpVector();
					break;
				}
				bombBurstMode=1;
				bombBurstBreakPoint=pos;
			}
			break;
		case 1:
			{
				YsVec3 uv,testUv;
				YsAtt3 testAtt;
				testAtt=att;
				testAtt.SetUpVector(bombBurstDesigUv1);
				testUv=testAtt.GetUpVector();

				uv=att.GetUpVector();
				if(uv*testUv>0.5) // 60 degree difference
				{
					bombBurstMode=2;
				}
			}
			break;
		case 2:
			{
				YsVec3 uv,testUv;
				YsAtt3 testAtt;
				testAtt=att;
				testAtt.SetUpVector(bombBurstDesigUv2);
				testUv=testAtt.GetUpVector();

				uv=att.GetUpVector();
				if(uv*testUv>0.9986) // 2 degree difference
				{
					bombBurstMode=3;
				}
			}
			break;
		case 3:
			if(att.p()<YsDegToRad(3.0))
			{
				bombBurstMode=4;
			}
			break;
		}
	}
	else if(fomPosition==5)
	{
		switch(bombBurstMode)
		{
		case 0:  // Level -> Entry Point
			if(bombBurstEntryPoint!=YsOrigin())
			{
				YsVec3 tst;
				tst=bombBurstEntryPoint-pos;
				att.MulInverse(tst,tst);
				if(tst.z()<0.0)
				{
					bombBurstMode=1;
				}
			}
			break;
		case 1:  // Entry -> Break Point
			if(bombBurstBreakPoint!=YsOrigin()) 
			{
				YsVec3 tst;
				tst=bombBurstBreakPoint-pos;
				att.MulInverse(tst,tst);
				if(tst.z()<0.0)
				{
					bombBurstMode=2;
				}
			}
			break;
		case 2:  // Until it comes to vertical
			if(att.p()>YsDegToRad(88.0))
			{
				waitTimer=6.0;
				bombBurstMode=3;
			}
			break;
		case 3:  // Vertical Climb Roll
			waitTimer-=dt;
			if(waitTimer<YsTolerance || air.Prop().GetVelocity()<air.Prop().GetFullyManeuvableSpeed()*2.0)
			{
				bombBurstMode=4;
			}
			break;
		case 4:
			if(att.p()<YsDegToRad(3.0))
			{
				bombBurstMode=5;
			}
			break;
		}
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControl4ShipBombBurst(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	// const YsAtt3 &att=air.GetAttitude();

	if(1<=fomPosition && fomPosition<=4 && 5<=bombBurstMode)  // One Point Cross
	{
		switch(bombBurstMode)
		{
		case 5:
			air.Prop().BankController(YsPi);
			if(air.GetAttitude().b()>YsDegToRad(170.0) || air.GetAttitude().b()<-YsDegToRad(170.0))
			{
				double dif;
				dif=(YsPi-fabs(air.GetAttitude().b()))/YsDegToRad(10.0);
				dif=1.0-dif;
				air.Prop().GController(5.0*dif);
			}
			else
			{
				air.Prop().GController(0.0);
			}
			air.Prop().SpeedController(bombBurstLevelSpeed);
			break;
		case 6:
			air.Prop().GController(5.0);
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			break;
		case 7:
		case 8:
			{
				YsVec3 rel,destin;
				const YsVec3 *cur;
				const YsAtt3 *att;
				YsMatrix4x4 mat;
				double radial;

				cur=&air.GetPosition();
				att=&air.GetAttitude();

				YsAtt3 hdg(*att);
				hdg.SetP(0.0);
				hdg.SetB(0.0);

				mat.Translate(*cur);
				mat.Rotate(hdg);
				mat.Invert();

				destin=bombBurstBreakPoint;
				switch(fomPosition)
				{
				case 1:
					destin.AddX(20.0);
					break;
				case 2:
					destin.AddZ(20.0);
					break;
				case 3:
					destin.SubZ(20.0);
					break;
				case 4:
					destin.SubX(20.0);
					break;
				}
				rel=mat*destin;
				radial=atan2(-rel.x(),rel.z());

				double bnk;
				bnk=radial*10.0;
				bnk=YsBound(bnk,-YsDegToRad(40),YsDegToRad(40));
				air.Prop().BankController(bnk);

				double alt;
				alt=bombBurstEntryPoint.y()+30.0*(double)fomPosition;
				if(bombBurstMode==7)
				{
					ControlGForRollOut(air,sim,alt,YsPi/4.0,-YsPi/4.0,6.0,-2.0);
				}
				else
				{
					ControlGForAltitude(air,sim,alt,0.0);
				}

				if(rel.z()<500.0)
				{
					air.Prop().SetSmokeButton(YSTRUE);
				}
			}
			break;
		case 9:
			{
				double alt;
				alt=bombBurstEntryPoint.y()+30.0*(double)fomPosition;
				ControlGForAltitude(air,sim,alt,0.0);
				air.Prop().BankController(0.0);
				if(smokeOffTimer>YsTolerance)
				{
					air.Prop().SetSmokeButton(YSTRUE);
				}
				else
				{
					air.Prop().SetSmokeButton(YSFALSE);
				}
			}
			break;
		}
	}
	else if(fomPosition==1)
	{
		switch(bombBurstMode)
		{
		case 0:
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0,YsPi/5.0);
			air.Prop().SetThrottle(0.9);
			break;
		case 1:
			air.Prop().GController(4.0);
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			break;
		case 2:
			air.Prop().GController(0.0);
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 3:
			air.Prop().GController(4.0);
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 4:
			air.Prop().BankController(0.0);
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SpeedController(bombBurstLevelSpeed);
			air.Prop().SetFireWeaponButton(YSFALSE);
			air.Prop().SetSmokeButton(YSFALSE);
			break;
		}
	}
	else if(2<=fomPosition && fomPosition<=4)
	{
		YsVec3 uv;
		switch(bombBurstMode)
		{
		case 0:
			FsFormation::ApplyControl(air,sim,dt);
			break;
		case 1:   // Break -> Rolling (1)
			air.Prop().GController(0.0); // leader->Prop().GetG());
			air.Prop().BankController(bombBurstDesigUv1);
			air.Prop().SpeedController(leader->Prop().GetVelocity());
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 2:  // Break -> Rolling (2)
			air.Prop().GController(0.0); // leader->Prop().GetG());
			air.Prop().BankController(bombBurstDesigUv2);
			air.Prop().SpeedController(leader->Prop().GetVelocity());
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 3:
			uv=bombBurstRefPoint-pos;
			air.Prop().BankController(uv);
//			air.Prop().TurnOffBankController();
//			air.Prop().SetAileron(0.0);

			air.Prop().GController(leader->Prop().GetG());
			air.Prop().SpeedController(leader->Prop().GetVelocity());
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 4:
			ControlGForAltitude(air,sim,leader->GetPosition().y(),0.0);
			air.Prop().BankController(0.0);
			air.Prop().SpeedController(leader->Prop().GetVelocity());
			air.Prop().SetFireWeaponButton(YSFALSE);
			air.Prop().SetSmokeButton(YSFALSE);
			break;
		}
	}
	else if(fomPosition==5)
	{
		switch(bombBurstMode)
		{
		case 0:
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0);
			if(bombBurstEntrySpeed>YsTolerance)
			{
				air.Prop().SpeedController(bombBurstEntrySpeed);
			}
			else if(leader!=NULL)
			{
				air.Prop().SpeedController(leader->Prop().GetVelocity());
			}
			else
			{
				air.Prop().SetThrottle(0.9);
			}
			break;
		case 1:
			{
				int i;
				YsVec3 target,rel;

				target=bombBurstBreakPoint;
				for(i=0; i<9; i++)
				{
					air.GetAttitude().MulInverse(rel,bombBurstClimbPath[i]-air.GetPosition());
					if(rel.z()>0.0)
					{
						if(i<8)
						{
							target=(bombBurstClimbPath[i]+bombBurstClimbPath[i+1])/2.0;
							break;
						}
						else
						{
							target=bombBurstClimbPath[i];
							break;
						}
					}
				}

				if(target!=YsOrigin())
				{
					double angle,g;
					air.GetAttitude().MulInverse(rel,target-air.GetPosition());
					angle=rel.y()/rel.z();
					g=YsBound(angle*60.0,-3.0,7.0);
					air.Prop().GController(g);
				}
				else
				{
					ControlGForVerticalSpeed(air,sim,0.0);
				}

				air.Prop().TurnOffBankController();
				air.Prop().SetAileron(0.0);

				air.Prop().TurnOffSpeedController();
				air.Prop().SetThrottle(1.0);
				air.Prop().SetAfterburner(YSTRUE);
			}
			break;
		case 2:
			if(fabs(air.GetAttitude().b())<YsPi/2.0)
			{
				air.Prop().GController(5.0);
			}
			else
			{
				air.Prop().GController(-3.0);
			}
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 3:
			air.Prop().SetAileron(0.5);
			if(cos(air.GetAttitude().b())>0.5)
			{
				air.Prop().GController(3.0);
			}
			else if(cos(air.GetAttitude().b())<-0.5)
			{
				air.Prop().GController(-3.0);
			}
			else
			{
				air.Prop().GController(0.0);
			}
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 4:
			air.Prop().SetAileron(0.0);
			air.Prop().GController(3.0);
			air.Prop().SetSmokeButton(YSFALSE);
			break;
		case 5:
			air.Prop().BankController(YsPi/2.5);
			ControlGForAltitude(air,sim,bombBurstBreakPoint.y(),0.0);
			air.Prop().SetThrottle(0.6);
			air.Prop().SetAfterburner(YSFALSE);
			break;
		}
		
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionChangeOverTurn(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	int i;
	if(fomPosition==1)
	{
		switch(changeOverTurnMode)
		{
		case 0:
			if(changeOverTurnAltitude<YsTolerance)
			{
				changeOverTurnAltitude=air.GetPosition().y();
				changeOverTurnEntryHeading=air.GetAttitude().h();
			}
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				double r;
				double shouldBeX[]=
				{
					-1.0,   // #2
					 1.0,   // #3
					 0.0,   // #4
					-2.0,   // #5
					 2.0    // #6
				};

				r=air.GetRadiusFromCollision();

				waitTimer=5.0;
				changeOverTurnMode=1;
				changeOverTurnEntrySpeed=air.Prop().GetVelocity();
				for(i=1; i<6; i++)
				{
					FsAirshowControl *fom;
					fom=GetAirshowAP(formation[i]);
					if(fom!=NULL)
					{
						fom->shouldBe.SetX(r*shouldBeX[i-1]);
						fom->changeOverTurnMode=1;
					}
				}
			}
			break;
		case 1:
			{
				YsVec2 v1,v2;
				v1.Set(cos(changeOverTurnEntryHeading),sin(changeOverTurnEntryHeading));
				v2.Set(cos(air.GetAttitude().h()),sin(air.GetAttitude().h()));
				if(v1*v2<0.0)  // 90deg
				{
					double r;
					double shouldBeYZ[]=
					{
						-1.0,   // #2
						-1.0,   // #3
						-2.0,   // #4
						-2.0,   // #5
						-2.0    // #6
					};

					r=air.GetRadiusFromCollision();

					waitTimer=8.0;
					changeOverTurnMode=2;
					for(i=1; i<6; i++)
					{
						FsAutopilot *ap;
						FsFormation *fom;
						if(formation[i]!=NULL && 
						   (ap=formation[i]->GetAutopilot())!=NULL &&
						   (ap->Type()==FSAUTOPILOT_FORMATION || // strcmp(ap->WhatItIs(),FsFormation::ClassName)==0 || 
						    ap->Type()==FSAUTOPILOT_AIRSHOW)) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0))
						{
							fom=(FsFormation *)ap;
							fom->shouldBe.SetY(2.0*shouldBeYZ[i-1]);
							fom->shouldBe.SetZ(r*shouldBeYZ[i-1]);
						}
					}
				}
			}
			break;
		case 2:
			{
				YsVec2 v1,v2;
				v1.Set(cos(changeOverTurnEntryHeading),sin(changeOverTurnEntryHeading));
				v2.Set(cos(air.GetAttitude().h()),sin(air.GetAttitude().h()));
				if(v1*v2>cos(YsDegToRad(5.0)))
				{
					waitTimer=5.0;
					changeOverTurnMode=3;
				}
			}
			break;
		case 3:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
	}
	else
	{
		FsFormation::MakeDecision(air,sim,dt);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlChangeOverTurn(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(fomPosition==1)
	{
		switch(changeOverTurnMode)
		{
		default:
			air.Prop().BankController(0.0,YsPi/7.0);
			air.Prop().SetSmokeButton(YSFALSE);
			air.Prop().SetThrottle(0.6);
			break;
		case 1:
		case 2:
			air.Prop().BankController(turnDir*changeOverTurnBankAngle,YsPi/6.0);
			air.Prop().SetSmokeButton(YSFALSE);
			air.Prop().SpeedController(changeOverTurnEntrySpeed);
			break;
		}
		ControlGForAltitude(air,sim,changeOverTurnAltitude,0.0);
	}
	else
	{
		FsFormation::ApplyControl(air,sim,dt);
		if(changeOverTurnMode>0)
		{
			air.Prop().SetSmokeButton(YSTRUE);
		}
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionCubanEight(FsAirplane &air,FsSimulation *,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(loopMode)
	{
	case 0:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			loopMode=1;
			entryAlt=pos.y();
			loopPathAltitude[0]=air.Prop().GetVelocity();
		}
		break;

	case 1:   // Vertical up
	case 11:
		if(att.p()>YsDegToRad(88.0) || fabs(att.b())>YsPi/2.0)
		{
			loopMode++;
		}
		break;
	case 2:  // Inverted
	case 12:
		if(att.p()<0.0)
		{
			loopMode++;
		}
		break;
	case 3:  // Bank=180, Pitch=-35
	case 13:
		if(att.p()<-YsDegToRad(35.0))
		{
			loopMode++;
		}
		break;

	case 4:  // 0.5 roll
	case 14:  // 0.5 roll
		if(fabs(att.b())<YsDegToRad(5.0))
		{
			loopMode++;
		}
		break;
	case 5: // 1.0 roll
	case 15: // 1.0 roll
		if(fabs(att.b())>YsDegToRad(175.0))
		{
			loopMode++;
		}
		break;

	case 6:
	case 16:
		if(fabs(att.b())<YsDegToRad(45.0))
		{
			loopMode+=4;
		}
		break;


	case 10:
	case 20:
		if(att.p()>-YsDegToRad(2.0))
		{
			loopMode++;
		}
		break;

	case 21:
		if(att.p()>-YsDegToRad(2.0))
		{
			waitTimer=5.0;
			loopMode++;
		}
		break;
	case 22:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlCubanEight(FsAirplane &air,FsSimulation *sim,const double & /*dt*/ )
{
	// const YsVec3 &pos=air.GetPosition();
	// const YsAtt3 &att=air.GetAttitude();

	switch(loopMode)
	{
	case 0:
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0,YsPi/5.0);
		air.Prop().SetThrottle(0.9);
		air.Prop().SetAfterburner(YSFALSE);
		break;

	case 1:   // Until vertical up
	case 11:
		air.Prop().GController(5.0);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetSpoiler(0.0);
		air.Prop().SetThrottle(1.0);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	case 2:  // Until inverted
	case 12:
		air.Prop().GController(5.0);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		air.Prop().SetThrottle(1.0);
		air.Prop().SetAfterburner(YSFALSE);
		break;
	case 3:  // Until bank=180, Pitch=-40
	case 13:
		air.Prop().GController(5.0);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		air.Prop().SetThrottle(0.5);
		air.Prop().SetAfterburner(YSFALSE);
		break;

	case 4:  // 0.5 roll
	case 6:
	case 8:
	case 14:  // 0.5 roll
	case 16:
	case 18:
		air.Prop().GController(0.0);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(turnDir*0.4);
		air.Prop().SpeedController(loopPathAltitude[0]);
		break;
	case 5: // 1.0 roll
	case 7:
	case 9:
	case 15: // 1.0 roll
	case 17:
	case 19:
		air.Prop().GController(0.0);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(turnDir*0.4);
		air.Prop().SpeedController(loopPathAltitude[0]);
		break;

	case 10:
	case 20:
		ControlGForRollOut(air,sim,entryAlt,YsPi/4.0,-YsPi/4.0,6.0,-2.0);
		air.Prop().BankController(0.0);
		air.Prop().SpeedController(loopPathAltitude[0]);
		break;

	case 21:
		ControlGForRollOut(air,sim,entryAlt,YsPi/4.0,-YsPi/4.0,6.0,-2.0);
		air.Prop().BankController(0.0);
		air.Prop().SetThrottle(0.5);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().SetSpoiler(0.0);
		break;
	case 22:
		ControlGForAltitude(air,sim,entryAlt,0.0);
		air.Prop().BankController(0.0,YsPi/5.0);
		air.Prop().SetThrottle(0.9);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().SetSmokeButton(YSFALSE);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionBontonRoll(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(brlMode)
	{
	case 0:
		if(fomPosition==1)
		{
			if(waitTimer>YsTolerance)
			{
				waitTimer-=dt;
			}
			if(waitTimer<YsTolerance)
			{
				int i;
				brlMode=1;
				for(i=1; i<6; i++)
				{
					FsAutopilot *ap;
					FsAirshowControl *fom;
					if(formation[i]!=NULL && (ap=formation[i]->GetAutopilot())!=NULL)
					{
						if(ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
						{
							fom=(FsAirshowControl *)ap;
							fom->brlMode=1;
						}
					}
				}
			}
		}
		else
		{
			FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 1:
		if(fabs(att.b())>rollIntegrator)
		{
			rollIntegrator=fabs(att.b());
		}
		if(rollIntegrator>YsPi*3.0/4.0 && fabs(att.b())<YsPi/2.0)
		{
			brlMode=2;
		}
		break;
	case 2:
		if(fomPosition==1)
		{
			YsAtt3 vAtt;
			air.Prop().GetRotationSpeed(vAtt);
			if(fabs(att.b())<YsDegToRad(1.0) && fabs(vAtt.b())<YsDegToRad(1.0))
			{
				brlMode=3;
				smokeOffTimer=3.0;
			}
		}
		else if(leader!=NULL)
		{
			YsAtt3 att,vAtt;
			att=leader->GetAttitude();
			leader->Prop().GetRotationSpeed(vAtt);
			if(fabs(att.b())<YsDegToRad(1.0) && fabs(vAtt.b())<YsDegToRad(1.0))
			{
				brlMode=3;
				smokeOffTimer=3.0;
			}
		}
		break;
	case 3:
		smokeOffTimer-=dt;
		if(smokeOffTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlBontonRoll(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	// const YsAtt3 &att=air.GetAttitude();

	switch(brlMode)
	{
	case 0:
	case 3:
		if(fomPosition==1)
		{
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0,YsPi/5.0);
			air.Prop().SetThrottle(0.5);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 1:
		air.Prop().TurnOffBankController();
		if(turnDir>=0.0)
		{
			air.Prop().SetAileron(0.5);
		}
		else
		{
			air.Prop().SetAileron(-0.5);
		}

		if(fomPosition!=1 && leader!=NULL)
		{
			air.Prop().SpeedController(leader->Prop().GetVelocity());
		}

		break;
	case 2:
		if(fomPosition==1)
		{
			air.Prop().SetThrottle(0.5);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else if(leader!=NULL)
		{
			air.Prop().SpeedController(leader->Prop().GetVelocity());
		}
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0,YsPi/5.0);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionBombBurstDownward(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	int i;
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1)
	{
		switch(bombBurstMode)
		{
		case 0:
			if(waitTimer>YsTolerance)
			{
				waitTimer-=dt;
			}
			else if(fabs(att.b())<YsDegToRad(3.0))
			{
				bombBurstMode=1;
				bombBurstEntryPoint=pos;
				bombBurstLevelSpeed=air.Prop().GetVelocity();
			}
			break;
		case 1:  // Loop
			if(att.p()<-YsDegToRad(87.0))  // Break
			{
				if(action==BOMBBURSTDOWN4SHIP)
				{
					for(i=1; i<4; i++)
					{
						FsAirshowControl *fom;
						if((fom=GetAirshowAP(formation[i]))!=NULL)
						{
							YsVec3 v;

							switch(i)
							{
							case 1:
								v.Set(-10000.0,0.0,0.0);
								break;
							case 2:
								v.Set( 10000.0,0.0,0.0);
								break;
							case 3:
								v.Set( 0.0,-10000.0,0.0);
								break;
							default:
								v=YsOrigin();
								break;
							}

							att.Mul(v,v);
							v+=pos;
							v.SetY(pos.y()+1000.0);

							fom->bombBurstRefPoint=v;
							fom->bombBurstMode=2;
							fom->bombBurstBreakPoint=pos;
							fom->bombBurstEntryPoint=bombBurstEntryPoint;
						}
					}
				}
				else if(action==BOMBBURSTDOWN6SHIP)
				{
					for(i=1; i<6; i++)
					{
						FsAirshowControl *fom;
						if((fom=GetAirshowAP(formation[i]))!=NULL)
						{
							YsVec3 v;

							switch(i)
							{
							case 1:
								v.Set(-10000.0,5000.0,0.0);
								break;
							case 2:
								v.Set( 10000.0,5000.0,0.0);
								break;
							case 3:
								v.Set( 0.0,-10000.0,0.0);
								break;
							case 4:
								v.Set(-10000.0,-5000.0,0.0);
								break;
							case 5:
								v.Set( 10000.0,-5000.0,0.0);
								break;
							default:
								v=YsOrigin();
								break;
							}

							att.Mul(v,v);
							v+=pos;
							v.SetY(pos.y()+1000.0);

							fom->bombBurstRefPoint=v;
							fom->bombBurstMode=2;
							fom->bombBurstEntryPoint=bombBurstEntryPoint;
							fom->bombBurstBreakPoint=pos;
						}
					}
				}
				else if(action==RAINFALL)
				{
					for(i=1; i<6; i++)
					{
						if(i==3)
						{
							continue;
						}
						FsAirshowControl *fom;
						if((fom=GetAirshowAP(formation[i]))!=NULL)
						{
							YsVec3 v;

							switch(i)
							{
							case 1:
								v.Set(-sin(YsPi/6.0),cos(YsPi/6.0),0.0);
								break;
							case 2:
								v.Set( sin(YsPi/6.0),cos(YsPi/6.0),0.0);
								break;
							case 4:
								v.Set(-sin(YsPi/3.0),cos(YsPi/3.0),0.0);
								break;
							case 5:
								v.Set( sin(YsPi/3.0),cos(YsPi/3.0),0.0);
								break;
							default:
								v=YsOrigin();
								break;
							}
							v*=10000.0;

							att.Mul(v,v);
							v+=pos;
							v.SetY(pos.y()+1000.0);

							fom->bombBurstRefPoint=v;
							fom->bombBurstMode=2;
							fom->bombBurstEntryPoint=bombBurstEntryPoint;
							fom->bombBurstBreakPoint=pos;
						}
					}
				}

				double rr;
				air.Prop().GetRollRate(rr);
				waitTimer=YsPi/rr;
				pitchIntegrator=att.p();
				bombBurstMode=2;
				bombBurstBreakPoint=pos;
			}
			break;
		case 2:  // Rolling out
			pitchIntegrator=YsGreater(att.p(),pitchIntegrator);
			waitTimer-=dt;
			if(att.p()>-YsDegToRad(2.0))
			{
				bombBurstMode=3;
				waitTimer=5.0;
			}
			break;
		case 3:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
		return YSOK;
	}
	else
	{
		switch(bombBurstMode)
		{
		default:
		case 0:
			return FsFormation::MakeDecision(air,sim,dt);
		case 2:
			pitchIntegrator=YsGreater(att.p(),pitchIntegrator);
			if(att.p()>-YsDegToRad(2.0))
			{
				bombBurstMode=3;
			}
			break;
		case 3:
			break;
		}
		return YSOK;
	}
	// Never reach here return YSERR;
}

YSRESULT FsAirshowControl::ApplyControlBombBurstDownward(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1)
	{
		switch(bombBurstMode)
		{
		case 0:  // Straight
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0,YsPi/5.0);
			air.Prop().SetThrottle(0.9);
			break;
		case 1:  // Making a loop -> Break
			air.Prop().GController(4.0);
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			if(att.p()<0.0)
			{
				air.Prop().SetThrottle(0.1);
			}
			else
			{
				air.Prop().SetThrottle(0.9);
			}
			break;
		case 2:
			if(waitTimer>YsTolerance)
			{
				air.Prop().GController(1.0);
			}
			else if(pitchIntegrator>-YsDegToRad(60.0))
			{
				ControlGForRollOut(air,sim,bombBurstEntryPoint.y(),YsPi/4.0,-YsPi/4.0,7.0,-2.0);
			}
			else
			{
				air.Prop().GController(5.0);
			}
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 3:
			air.Prop().BankController(0.0);
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SpeedController(bombBurstLevelSpeed);
			air.Prop().SetFireWeaponButton(YSFALSE);
			air.Prop().SetSmokeButton(YSFALSE);
			break;
			
		}
		return YSOK;
	}
	else
	{
		YsVec3 uv;

		switch(bombBurstMode)
		{
		default:
		case 0:
			return FsFormation::ApplyControl(air,sim,dt);
		case 2:
			uv=bombBurstRefPoint-pos;
			air.Prop().BankController(uv);
			att.MulInverse(uv,uv);
			if(uv.y()<0.0)
			{
				air.Prop().GController(0.0);
			}
			else if(pitchIntegrator>-YsDegToRad(60.0))
			{
				ControlGForRollOut(air,sim,bombBurstEntryPoint.y(),YsPi/4.0,-YsPi/4.0,7.0,-2.0);
			}
			else
			{
				air.Prop().GController(5.0);
			}
			air.Prop().SetSmokeButton(YSTRUE);
			air.Prop().SetThrottle(0.1);
			break;
		case 3:
			air.Prop().BankController(0.0);
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SpeedController(bombBurstLevelSpeed);
			air.Prop().SetFireWeaponButton(YSFALSE);
			air.Prop().SetSmokeButton(YSFALSE);
			break;
		}
		return YSOK;
	}
	// Never reach here return YSERR;
}

YSRESULT FsAirshowControl::MakeDecisionRollingCombatPitch(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1 && endOfAction==YSTRUE)
	{
	}

	switch(cpMode)
	{
	case 0:
		if(fomPosition==1)
		{
			if(waitTimer>0.0)
			{
				waitTimer-=dt;
			}
			if(att.p()>YsPi/36.0 && fabs(att.b())>YsPi/36.0)
			{
				cpMode=10;
				cpEntryHeading=att.h();
			}
		}
		else
		{
			return FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 10:
		waitTimer-=dt;
		if(waitTimer<0.0)
		{
			FsAirshowControl *airshow;
			FsAirplane *newLeader;
			int i;
			cpMode=11;

			if(fomPosition<NFORMATION && (newLeader=formation[fomPosition])!=NULL)
			{
				if((airshow=GetAirshowAP(newLeader))!=NULL)
				{
					airshow->cpMode=10;
					airshow->waitTimer=1.0;
					for(i=fomPosition+1; i<NFORMATION; i++)
					{
						SwitchLeader(newLeader,formation[i]);
					}
				}
			}
			else
			{
				for(i=0; i<NFORMATION; i++)
				{
					if(formation[i]!=NULL && (airshow=GetAirshowAP(formation[i]))!=NULL)
					{
						airshow->smokeOffTimer=8.0;
						airshow->cpMode=12;
					}
				}
			}
		}
		break;
	case 11:
		break;
	case 12:
		smokeOffTimer-=dt;
		if(smokeOffTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlRollingCombatPitch(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	air.Prop().SetSmokeButton(YSTRUE);

	switch(cpMode)
	{
	case 0:
		if(fomPosition==1)
		{
			air.Prop().PitchController(YsPi/18.0);
			if(cpLeftTurn==YSTRUE)
			{
				air.Prop().BankController(-YsPi/18.0,YsPi/6.0);
			}
			else
			{
				air.Prop().BankController( YsPi/18.0,YsPi/6.0);
			}
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.9);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 10:
		air.Prop().PitchController(YsPi/18.0);
		if(cpLeftTurn==YSTRUE)
		{
			air.Prop().BankController(-YsPi/18.0,YsPi/6.0);
		}
		else
		{
			air.Prop().BankController( YsPi/18.0,YsPi/6.0);
		}
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.9);
		air.Prop().SetAfterburner(YSFALSE);
		break;
	case 11:
	case 12:
		rollIntegrator=YsGreater(rollIntegrator,fabs(att.b()));
		if(rollIntegrator<YsDegToRad(160.0))
		{
			double ail;
			air.Prop().GetRollRate(ail);
			ail=YsPi/ail;
			air.Prop().TurnOffBankController();
			if(cpLeftTurn==YSTRUE)
			{
				air.Prop().SetAileron(-ail);
			}
			else
			{
				air.Prop().SetAileron( ail);
			}

			air.Prop().GController(1.0);
		}
		else
		{
			if(cpLeftTurn==YSTRUE)
			{
				air.Prop().BankController( YsDegToRad(70.0),YsPi);
			}
			else
			{
				air.Prop().BankController(-YsDegToRad(70.0),YsPi);
			}
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.9);
		air.Prop().SetAfterburner(YSFALSE);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionDiamondTakeoff(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	// const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1)
	{
		switch(dtoMode)
		{
		case 0:
			FsAutopilot *ap;
			FsFormation *fom;
			if(formation[3]!=NULL && (ap=formation[3]->GetAutopilot())!=NULL)
			{
				if(ap->Type()==FSAUTOPILOT_FORMATION || // strcmp(ap->WhatItIs(),FsFormation::ClassName)==0 ||
				   ap->Type()==FSAUTOPILOT_AIRSHOW)     // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
				{
					fom=(FsFormation *)ap;
					if(fom->nTransition==0)
					{
						waitTimer=5.0;
						dtoMode=1;
					}
				}
			}

			if(dtoStartAlt<YsTolerance)
			{
				dtoStartAlt=pos.y();
			}

			if(pos.y()<dtoStartAlt+10.0)
			{
				dtoSpeed=air.Prop().GetVelocity();
			}

			break;
		case 1:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
		return YSOK;
	}
	else
	{
		return FsFormation::MakeDecision(air,sim,dt);
	}
}

YSRESULT FsAirshowControl::ApplyControlDiamondTakeoff(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1)
	{
		if(pos.y()>dtoStartAlt+10.0)
		{
			air.Prop().SetGear(0.0);
		}

		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.9);
		air.Prop().SetAfterburner(YSTRUE);

		double g;
		g=1.0+(5.0-YsRadToDeg(att.p()))*0.1;
		g=YsBound(g,0.9,1.1);
		if(air.Prop().GetVelocity()>air.Prop().GetEstimatedLandingSpeed()*1.5)
		{
			air.Prop().GController(g);
			if(air.Prop().IsOnGround()==YSTRUE)
			{
				air.Prop().SetGControllerAOALimit
				   (air.Prop().GetGroundStaticPitchAngle(),air.Prop().GetTailStrikePitchAngle(0.8));
			}
			else
			{
				air.Prop().SetGControllerAOALimit
				   (0.0,air.Prop().GetTailStrikePitchAngle(0.8));
			}
		}
		else
		{
			air.Prop().AOAController(air.Prop().GetGroundStaticPitchAngle());
		}

		return YSOK;
	}
	else
	{
		return FsFormation::ApplyControl(air,sim,dt);
	}
}

YSRESULT FsAirshowControl::MakeDecisionContinuousRoll(FsAirplane &air,FsSimulation *,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(brlMode)
	{
	case 0:
		waitTimer-=dt;
		entryAlt=pos.y();

		if(waitTimer<YsTolerance)
		{
			YsVec3 vel;
			air.Prop().GetVelocity(vel);
			if(vel.y()>5.0)
			{
				brlMode=1;
			}
		}
		break;
	case 1:
		if(rollIntegrator>3.49 && fabs(att.b())<YsPi/2.0)
		{
			brlMode=2;
			smokeOffTimer=4.0;
		}
		else if(rollIntegrator>2.99 && fabs(att.b())>YsPi*3.0/4.0)
		{
			rollIntegrator=3.5;
		}
		else if(rollIntegrator>2.49 && fabs(att.b())<YsPi/4.0)
		{
			rollIntegrator=3.0;
		}
		else if(rollIntegrator>1.99 && fabs(att.b())>YsPi*3.0/4.0)
		{
			rollIntegrator=2.5;
		}
		else if(rollIntegrator>1.49 && fabs(att.b())<YsPi/4.0)
		{
			rollIntegrator=2.0;
		}
		else if(rollIntegrator>0.99 && fabs(att.b())>YsPi*3.0/4.0)
		{
			rollIntegrator=1.5;
		}
		else if(rollIntegrator>0.49 && fabs(att.b())<YsPi/4.0)
		{
			rollIntegrator=1.0;
		}
		if(rollIntegrator<0.49 && fabs(att.b())>YsPi*3.0/4.0)
		{
			rollIntegrator=0.5;
		}
		break;
	case 2:
		smokeOffTimer-=dt;
		if(smokeOffTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlContinuousRoll(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	double g;
	YsVec3 vel;

	air.Prop().GetVelocity(vel);

	switch(brlMode)
	{
	case 0:
		if(waitTimer<YsTolerance)
		{
			air.Prop().GController(3.0);
		}
		else
		{
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		air.Prop().SetThrottle(0.9);
		air.Prop().BankController(0.0);
		air.Prop().SmartRudder(dt);
		air.Prop().SetSmokeButton(YSTRUE);
		break;

	case 1:
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(turnDir*0.6);

		if(fabs(att.b())<YsPi/4.0)
		{
			g=1.0+(entryAlt-pos.y())/10.0;
			g=YsBound(g,1.0,3.0);
			air.Prop().GController(g);
		}
		else if(fabs(att.b())>YsPi*3.0/4.0)
		{
			g=-1.0-(entryAlt-pos.y())/10.0;
			g=YsBound(g,-1.0,-2.2);
			air.Prop().GController(g);
		}
		else
		{
			air.Prop().GController(0.0);
		}

		if(YsPi/4.0<att.b() && att.b()<YsPi*3.0/4.0)
		{
			air.Prop().SetRudder(-1.0);
		}
		else if(-YsPi/4.0>att.b() && att.b()>-YsPi*3.0/4.0)
		{
			air.Prop().SetRudder(1.0);
		}
		else
		{
			air.Prop().SetRudder(0.0);
		}

		air.Prop().SetSmokeButton(YSTRUE);

		break;

	case 2:
		air.Prop().SetThrottle(0.9);
		air.Prop().BankController(0.0);
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().SetRudder(0.0);
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionRollOnTakeoff(FsAirplane &air,FsSimulation *,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	double speedThr;

	if(fomPosition==5)
	{
		speedThr=(air.Prop().IsJet()==YSTRUE ? 85.0 : 35);

		switch(dtoMode)
		{
		case 0:
			if(dtoStartAlt<YsTolerance)
			{
				dtoStartAlt=pos.y();
			}
			headingIntegrator=att.h()+YsPi;
			if(air.Prop().GetVelocity()>speedThr)
			{
				dtoMode=1;
			}
			break;
		case 1:
			if(pos.y()>dtoStartAlt+150.0)
			{
				dtoMode=2;
				rollIntegrator=0.0;
			}
			break;
		case 2:
			if(rollIntegrator>0.49 && fabs(att.b())<YsPi/2.0)
			{
				dtoMode=3;
			}
			else if(rollIntegrator<YsTolerance && fabs(att.b())>YsPi*17.0/18.0)
			{
				rollIntegrator=0.5;
			}
			break;
		case 3:
			{
				YsVec3 vel;
				air.Prop().GetVelocity(vel);
				if(vel.y()<0.0)
				{
					dtoMode=4;
				}
			}
			break;
		case 4:
			{
				YsVec3 v;
				YsAtt3 a;
				a.SetH(headingIntegrator);
				a.SetP(0.0);
				a.SetB(0.0);
				v=a.GetForwardVector();

				a.Set(att.h(),0.0,0.0);
				a.MulInverse(v,v);

				if(v.z()>YsTolerance && fabs(v.x())<tan(YsPi/60.0))
				{
					waitTimer=3.0;
					dtoMode=5;
				}
			}
			break;

		case 5:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
	}
	else if(fomPosition==6)
	{
		speedThr=(air.Prop().IsJet()==YSTRUE ? 100.0 : 45);

		switch(dtoMode)
		{
		case 0:
			if(dtoStartAlt<YsTolerance)
			{
				dtoStartAlt=pos.y();
			}
			if(dtoStartAlt+4.0<pos.y())
			{
				dtoMode=1;
			}
			break;
		case 1:
			if(air.Prop().GetVelocity()>speedThr)
			{
				dtoMode=2;
			}
			break;
		case 2:
			if(att.p()<-YsPi/9.0 && fabs(att.b())>YsPi/2.0)
			{
				dtoMode=3;
				rollIntegrator=0.0;
			}
			break;
		case 3:
			if(rollIntegrator>=1.49 && fabs(att.b())<YsPi*4.0/6.0)
			{
				dtoMode=4;
			}
			else if(rollIntegrator>=0.99 && fabs(att.b())>YsPi*5.0/6.0)
			{
				rollIntegrator=1.5;
			}
			else if(rollIntegrator>=0.49 && fabs(att.b())<YsPi/2.0)
			{
				rollIntegrator=1.0;
			}
			else if(rollIntegrator<YsTolerance && fabs(att.b())>YsPi*3.0/4.0)
			{
				rollIntegrator=0.5;
			}
			break;
		case 4:
			if(att.p()>-YsDegToRad(2.0))
			{
				dtoMode=5;
				waitTimer=10.0;
			}
			break;
		case 5:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlRollOnTakeoff(FsAirplane &air,FsSimulation *sim,const double & /*dt*/ )
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	double g,ail;
	double speedThr;

	speedThr=(air.Prop().IsJet()==YSTRUE ? 85.0 : 35);

	if(fomPosition==5)
	{

		switch(dtoMode)
		{
		case 0:
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);
			break;
		case 1:
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);

			g=30.0-YsRadToDeg(att.p());
			g=YsBound(g,0.5,4.0);
			air.Prop().GController(g);
			air.Prop().SetGControllerAOALimit
			   (air.Prop().GetGroundStaticPitchAngle(),air.Prop().GetTailStrikePitchAngle(0.8));
			break;
		case 2:
			air.Prop().GetRollRate(ail);
			ail=(YsPi/3.0)/ail;
			air.Prop().SetAileron(-ail);
			air.Prop().GController(2.0);
			break;
		case 3:
			if(att.b()<0.0)
			{
				air.Prop().BankController(-YsPi/2.0);
			}
			else
			{
				air.Prop().BankController(YsPi/2.0);
			}
			air.Prop().GController(4.0);
			break;
		case 4:
		case 5:
			{
				YsVec3 v;
				YsAtt3 a;
				a.SetH(headingIntegrator);
				a.SetP(0.0);
				a.SetB(0.0);
				v=a.GetForwardVector();

				a.Set(att.h(),0.0,0.0);
				a.MulInverse(v,v);

				double d;
				d=YsRadToDeg(atan2(-v.x(),v.z()));
				if(fabs(d)<10.0)
				{
					air.Prop().BankController(YsDegToRad(70.0)*d/10.0,YsPi/3.0);
					air.Prop().SetGear(0.0);
				}
				else
				{
					air.Prop().BankController(YsDegToRad(70.0),YsPi/3.0);
				}

				YsVec3 vel;
				air.Prop().GetVelocity(vel);
				ControlGForVerticalSpeed(air,sim,0.0,6.0);

				air.Prop().SpeedController(speedThr);
			}
			break;
		}
		air.Prop().SetSmokeButton(YSTRUE);
	}
	else if(fomPosition==6)
	{
		switch(dtoMode)
		{
		case 0:
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);
			air.Prop().GController(1.1);
			air.Prop().SetGControllerAOALimit
			   (air.Prop().GetGroundStaticPitchAngle(),air.Prop().GetTailStrikePitchAngle(0.8));
			if(air.Prop().GetFlightState()==FSFLYING)
			{
				air.Prop().SetGear(0.0);
			}
			break;
		case 1:
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);
			ControlGForAltitude(air,sim,dtoStartAlt+7.0,0.0);
			air.Prop().SetGear(0.0);
			break;
		case 2:  // Pull up -> Inverted
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);
			air.Prop().GController(4.0);
			break;
		case 3:
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(turnDir*0.6);
			air.Prop().GController(0.0);
			break;
		case 4:
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);
			if(fabs(att.b())<YsPi/2.0)
			{
				air.Prop().GController(4.0*cos(att.b()));
			}
			else
			{
				air.Prop().GController(0.0);
			}
			air.Prop().BankController(0.0);
			break;
		case 5:
			air.Prop().SpeedController(speedThr);

			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0);
			break;
		}
		air.Prop().SetSmokeButton(YSTRUE);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionTackCrossAndVerticalClimbRoll
    (FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(tcMode)
	{
	case 0:
		if(fomPosition!=6)
		{
			switch(tcMode)
			{
			case 0:
				if(entrySpeed<YsTolerance)
				{
					entrySpeed=air.Prop().GetVelocity();
					entryAlt=pos.y();
				}

				waitTimer-=dt;
				if(waitTimer<YsTolerance)
				{
					tcMode=1;
					rollIntegrator=0.0;

					entryAlt=pos.y();
					tcEntryHeading=att.h();
					tcBreakPoint=pos;
					tcBreakVec=att.GetForwardVector();

					FsAirshowControl *fom;
					if((fom=GetAirshowAP(formation[5]))!=NULL)
					{
						fom->tcMode=1;
						fom->entrySpeed=entrySpeed;
						fom->tcEntryHeading=tcEntryHeading;
						fom->entryAlt=entryAlt;
						fom->rollIntegrator=0.0;
						fom->tcBreakPoint=tcBreakPoint;
						fom->tcBreakVec=tcBreakVec;
					}
				}
				break;
			}
		}
		else if(fomPosition==6)
		{
			switch(tcMode)
			{
			case 0:
				return FsFormation::MakeDecision(air,sim,dt);
			}
		}
		break;
	case 1:
		if(rollIntegrator>0.49 && fabs(att.b())<YsPi*15.0/18.0)
		{
			tcMode=2;
		}
		else if(rollIntegrator<YsTolerance && fabs(att.b())>YsPi*17.0/18.0)
		{
			rollIntegrator=0.5;
		}
		break;
	case 2:
//	case 4:
		{
			double desigHdg;
			YsVec2 v1,v2;
			if(fomPosition!=6)
			{
				desigHdg=(tcEntryHeading+YsPi/2.0);
			}
			else
			{
				desigHdg=(tcEntryHeading-YsPi/2.0);
			}

			if(tcMode==4)
			{
				desigHdg+=YsPi;
			}

			v1.Set(cos(desigHdg),sin(desigHdg));
			v2.Set(cos(att.h()),sin(att.h()));
			if(v1*v2>=cos(YsPi/180.0))
			{
				waitTimer=10.0;
				tcMode++;
			}
		}
		break;
	case 3:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			if(fomPosition!=6)
			{
				YsVec3 crs;
				YsGetNearestPointOnLine3(crs,tcBreakPoint,tcBreakPoint+tcBreakVec,pos);
				tcCrossPoint=crs+tcBreakVec*air.GetRadiusFromCollision()*2.0;
				FsAirshowControl *fom;
				if((fom=GetAirshowAP(formation[5]))!=NULL)
				{
					fom->tcCrossPoint=crs-tcBreakVec*air.GetRadiusFromCollision()*2.0;
				}
			}
			entryAlt=pos.y();
			tcMode=4;
		}
		break;
	case 4:  // Half Cuban
		if(pitchIntegrator>=1.49 && att.p()>-YsDegToRad(3.0))
		{
			tcMode=5;
		}
		else if(0.99<=pitchIntegrator && pitchIntegrator<=1.49 && att.p()<-YsPi/9.0)
		{
			pitchIntegrator=1.5;
		}
		else if(0.49<=pitchIntegrator && pitchIntegrator<0.99 && att.p()<0.0)
		{
			pitchIntegrator=1.0;
		}
		else if(pitchIntegrator<YsTolerance && att.p()>YsPi*7.0/18.0)
		{
			pitchIntegrator=0.5;
		}
		break;
	case 5:
		{
			FsAirplane *opposingSolo;
			if(fomPosition==5)
			{
				opposingSolo=formation[5];
			}
			else if(fomPosition==6)
			{
				opposingSolo=formation[4];
			}
			else
			{
				opposingSolo=NULL;
				endOfAction=YSTRUE;
			}

			if(opposingSolo!=NULL)
			{
				YsVec3 p;
				p=opposingSolo->GetPosition();
				p-=pos;
				att.MulInverse(p,p);
				if(p.z()<100.0)
				{
					tcMode=6;
				}
			}
		}
		break;
	case 6:
		if(att.p()>YsDegToRad(87.0))
		{
			waitTimer=7.0;
			tcMode=7;
		}
		break;
	case 7:
		waitTimer-=dt;
		if(waitTimer<YsTolerance || air.Prop().GetVelocity()<air.Prop().GetFullyManeuvableSpeed()*2.0)
		{
			tcMode=8;
		}
		break;
	case 8:
		if(att.p()<YsDegToRad(3.0))
		{
			waitTimer=3.0;
			tcMode=9;
		}
		break;
	case 9:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlTackCrossAndVerticalClimbRoll
    (FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(tcMode)
	{
	case 0:
		if(fomPosition!=6)
		{
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0);
			air.Prop().SpeedController(entrySpeed);
		}
		else if(fomPosition==6)
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	case 1:
		air.Prop().TurnOffBankController();
		if(fomPosition!=6)
		{
			air.Prop().SetAileron(-0.6);
		}
		else
		{
			air.Prop().SetAileron(0.6);
		}

		if(YsPi/4.0<att.b() && att.b()<YsPi*3.0/4.0)
		{
			air.Prop().SetRudder(-1.0);
		}
		else if(-YsPi/4.0>att.b() && att.b()>-YsPi*3.0/4.0)
		{
			air.Prop().SetRudder(1.0);
		}
		else
		{
			air.Prop().SetRudder(0.0);
		}

		ControlGForAltitude(air,sim,entryAlt,0.0,8.0);
		air.Prop().SpeedController(entrySpeed);
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	case 2:
	case 3:
	case 5:
		{
			YsVec3 v;
			double h,r;
			if(fomPosition!=6)
			{
				h=tcEntryHeading+YsPi/2.0;
			}
			else
			{
				h=tcEntryHeading-YsPi/2.0;
			}

			if(tcMode==4 || tcMode==5)
			{
				h=h+YsPi;
			}

			if(fomPosition==6 && tcMode==5)  // Horizontal spacing
			{
				YsVec3 rel;
				r=air.GetRadiusFromCollision();
				rel=formation[4]->GetPosition()-pos;
				rel.RotateXZ(h);
				if(0.0<rel.x() && rel.x()<r*2.0)
				{
					h+=YsBound(YsDegToRad(r*2.0-rel.x()),-YsPi/180.0,YsPi/180.0);
				}
				else if(-r*2.0<=rel.x() && rel.x()<0.0)
				{
					h+=YsBound(YsDegToRad(-r*2.0-rel.x()),-YsPi/180.0,YsPi/180.0);
				}
			}

			v.Set(-sin(h),0.0,cos(h));
			v.RotateXZ(-att.h());

			double d;
			d=YsRadToDeg(atan2(-v.x(),v.z()));
			if(fabs(d)<10.0)
			{
				air.Prop().BankController(YsDegToRad(70.0)*d/10.0,YsPi/3.0);
			}
			else if(d>0.0)
			{
				air.Prop().BankController(YsDegToRad(70.0),YsPi/3.0);
			}
			else
			{
				air.Prop().BankController(-YsDegToRad(70.0),YsPi/3.0);
			}
		}

		if(tcMode==5 || fomPosition==6)
		{
			ControlGForAltitude(air,sim,entryAlt+3.0,0.0,8.0);
		}
		else
		{
			ControlGForAltitude(air,sim,entryAlt,0.0,8.0);
		}
		air.Prop().SetRudder(0.0);

		if(tcMode!=5)
		{
			air.Prop().SpeedController(entrySpeed);
		}
		else
		{
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(1.0);
			air.Prop().SetAfterburner(YSTRUE);
			air.Prop().SetSpoiler(0.0);
		}

		air.Prop().SetSmokeButton(tcMode==2 ? YSTRUE : YSFALSE);
		break;
	case 4:
		if(pitchIntegrator<0.49)
		{
			air.Prop().BankController(0.0);
			air.Prop().GController(5.0);
			air.Prop().SpeedController(entrySpeed);
		}
		else if(pitchIntegrator<0.99)
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			air.Prop().GController(5.0);
			air.Prop().SpeedController(entrySpeed);
		}
		else if(pitchIntegrator<1.49)
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			air.Prop().GController(5.0);
			air.Prop().SpeedController(entrySpeed);
		}
		else
		{
			YsVec3 v;
			double h,b;
			if(fomPosition!=6)
			{
				h=tcEntryHeading-YsPi/2.0;
			}
			else
			{
				h=tcEntryHeading+YsPi/2.0;
			}
			v.Set(-sin(h),0.0,cos(h));
			v.RotateXZ(-att.h());
			b=5.0*asin(-v.x());
			b=YsBound(b,-YsPi/36.0,YsPi/36.0);

			air.Prop().BankController(b);
			ControlGForRollOut(air,sim,entryAlt,YsPi/4.0,-YsPi/4.0,7.0,-2.0);
			air.Prop().SpeedController(entrySpeed);
		}
		break;

	case 6:  // Pull-up
		air.Prop().GController(5.0);
		if(att.p()<YsPi/3.0)
		{
			air.Prop().BankController(0.0);
		}
		else
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
		}
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	case 7: // Vertical climb roll
		air.Prop().SetAileron(0.5);
		if(cos(air.GetAttitude().b())>0.5)
		{
			air.Prop().GController(3.0);
		}
		else if(cos(air.GetAttitude().b())<-0.5)
		{
			air.Prop().GController(-3.0);
		}
		else
		{
			air.Prop().GController(0.0);
		}
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	case 8: // Level Off
		air.Prop().SetAileron(0.0);
		air.Prop().GController(3.0);
		air.Prop().SetSmokeButton(YSFALSE);
		break;
	case 9:
		air.Prop().BankController(YsPi/2.5);
		ControlGForAltitude(air,sim,bombBurstBreakPoint.y(),0.0);
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionBigHeart(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	YsVec3 uv;

	switch(bombBurstMode)
	{
	case 0:
		waitTimer-=dt;
		smokeOffTimer=0.5;
		if(entryAlt<YsTolerance)
		{
			entryAlt=pos.y();
		}
		if(fomPosition==5)
		{
			if(att.p()>YsDegToRad(88.0))
			{
				YsVec3 brkDir,refP;
				brkDir.Set(1.0,0.0,0.0);
				att.Mul(brkDir,brkDir);
				refP=pos+brkDir*10000.0;
				refP.SetY(0.0);
				bombBurstRefPoint=refP;
				bombBurstBreakPoint=pos;
				bombBurstMode=1;

				FsAutopilot *ap;
				FsAirshowControl *fom;
				if(formation[5]!=NULL && 
				   (ap=formation[5]->GetAutopilot())!=NULL &&
				   ap->Type()==FSAUTOPILOT_AIRSHOW) // strcmp(ap->WhatItIs(),FsAirshowControl::ClassName)==0)
				{
					fom=(FsAirshowControl *)ap;
					fom->bombBurstMode=1;
					refP=pos-brkDir*10000.0;
					refP.SetY(0.0);
					fom->bombBurstRefPoint=refP;
					fom->bombBurstBreakPoint=pos;
				}
			}
		}
		else
		{
			return FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 1:
		uv=bombBurstRefPoint-pos;
		att.MulInverse(uv,uv);
		uv.SetZ(0.0);
		if(uv.Normalize()==YSOK && uv.y()>cos(YsPi/90.0))
		{
			bombBurstMode=2;
		}
		break;
	case 2:
		if(att.p()<0.0)
		{
			YsVec3 p;
			double r;
			r=air.GetRadiusFromCollision();
			p.Set(r,0.0,0.0);
			att.Mul(p,p);
			bombBurstRefPoint=bombBurstBreakPoint+p;

			entrySpeed=air.Prop().GetVelocity();
			bombBurstMode=3;
			pitchIntegrator=0.0;
		}
		break;
	case 3:
		if(pitchIntegrator>0.49 && att.p()>-YsPi/6.0)
		{
			bombBurstMode=4;
		}
		else if(pitchIntegrator<YsTolerance && att.p()<-YsPi/3.0)
		{
			pitchIntegrator=0.5;
		}
		break;
	case 4:
		{
			FsAirplane *opposingSolo;
			if(fomPosition==5)
			{
				opposingSolo=formation[5];
			}
			else // if(fomPosition==6)
			{
				opposingSolo=formation[4];
			}
			if(opposingSolo!=NULL)
			{
				YsVec3 p;
				p=opposingSolo->GetPosition();
				p-=pos;
				att.MulInverse(p,p);
				if(p.z()<0.0)
				{
					smokeOffTimer-=dt;
				}
			}

			if(att.p()>-YsPi/90.0)
			{
				bombBurstMode=5;
				waitTimer=3.0;
			}
		}
		break;
	case 5:
		waitTimer-=dt;
		if(waitTimer<=YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlBigHeart(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	// const YsAtt3 &att=air.GetAttitude();
	YsVec3 uv;

	switch(bombBurstMode)
	{
	case 0:
		if(fomPosition==5)
		{
			air.Prop().SetThrottle(0.9);
			air.Prop().SetAfterburner(YSFALSE);
			if(waitTimer<YsTolerance)
			{
				air.Prop().GController(4.0);
			}
			else
			{
				ControlGForVerticalSpeed(air,sim,0.0);
			}
			air.Prop().SetAileron(0.0);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 1:  // Break
		uv=bombBurstRefPoint-pos;
		air.Prop().BankController(uv);
		air.Prop().GController(1.0);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.9);
		air.Prop().SetAfterburner(YSFALSE);
		break;
	case 2:  // Break -> Inverted
		uv=bombBurstRefPoint-pos;
		air.Prop().BankController(uv);
		air.Prop().GController(4.0);
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	case 3:  // Inverted -> -30degree pitch
		uv=bombBurstRefPoint-pos;
		air.Prop().BankController(uv);
		air.Prop().SpeedController(entrySpeed);
		air.Prop().GController(4.0);
		break;
	case 4:
		ControlGForRollOut(air,sim,entryAlt,YsPi/6.0,-YsPi/6.0,5.0,-2.0);
		air.Prop().SetAileron(0.0);
		air.Prop().SpeedController(entrySpeed);
		if(smokeOffTimer<YsTolerance)
		{
			air.Prop().SetSmokeButton(YSFALSE);
		}
		break;
	case 5:
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0);
		air.Prop().SpeedController(entrySpeed);
		air.Prop().SetSmokeButton(YSFALSE);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionLevelBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	int i;
	FsAirshowControl *fom;

	if(bombBurstMode==0)
	{
		waitTimer-=dt;
		entryHeading=att.h();
		entryAlt=pos.y();
		if(fomPosition==1)
		{
			if(waitTimer<0.5)
			{
				for(i=3; i<6; i++)
				{
					if((fom=GetAirshowAP(formation[i]))!=NULL)
					{
						if(fom->bombBurstMode==0)
						{
							fom->bombBurstMode=1;
							fom->entryHeading=att.h();
							fom->entryAlt=pos.y();
						}
					}
				}
			}
			if(waitTimer<YsTolerance)
			{
				for(i=1; i<3; i++)
				{
					if((fom=GetAirshowAP(formation[i]))!=NULL)
					{
						if(fom->bombBurstMode==0)
						{
							fom->bombBurstMode=1;
							fom->entryHeading=att.h();
							fom->entryAlt=pos.y();
						}
					}
				}

				bombBurstMode++;
			}
		}
		else
		{
			return FsFormation::MakeDecision(air,sim,dt);
		}
	}
	else
	{
		if(fomPosition==1)
		{
			YsVec3 vel;
			air.Prop().GetVelocity(vel);
			switch(bombBurstMode)
			{
			case 1:
				if(pos.y()>entryAlt+60.0 || vel.y()>FsGravityConst*2.0)
				{
					bombBurstMode++;
				}
				break;
			case 2:
				if(rollIntegrator>0.49 && fabs(att.b())<YsPi/2.0)
				{
					waitTimer=8.0;
					bombBurstMode++;
				}
				else if(rollIntegrator <YsTolerance && fabs(att.b())>YsPi*3.0/4.0)
				{
					rollIntegrator=0.5;
				}
				break;
			case 3:
				waitTimer-=dt;
				if(waitTimer<YsTolerance)
				{
					endOfAction=YSTRUE;
				}
				break;
			}
		}
		else if(fomPosition==4)
		{
			switch(bombBurstMode)
			{
			case 1:
				if(rollIntegrator>0.49 && fabs(att.b())<YsPi/2.0)
				{
					bombBurstMode++;
				}
				else if(rollIntegrator <YsTolerance && fabs(att.b())>YsPi*3.0/4.0)
				{
					rollIntegrator=0.5;
				}
				break;
			}
		}
		else if(fomPosition==5 || fomPosition==6)
		{
			switch(bombBurstMode)
			{
			case 1:
				if(pos.y()>entryAlt+30.0)
				{
					bombBurstMode=2;
				}
				break;
			case 2:
				if(rollIntegrator>0.49 && fabs(att.b())<YsPi/18.0)
				{
					bombBurstMode++;
				}
				else if(rollIntegrator <YsTolerance && fabs(att.b())>YsPi*3.0/4.0)
				{
					rollIntegrator=0.5;
				}
				break;
			}
		}
		else if(fomPosition==2 || fomPosition==3)
		{
			switch(bombBurstMode)
			{
			case 1:
				if(rollIntegrator>0.49 && fabs(att.b())<YsPi/2.0)
				{
					bombBurstMode++;
				}
				else if(rollIntegrator<YsTolerance && fabs(att.b())>YsPi*3.0/4.0)
				{
					rollIntegrator=0.5;
				}
				break;
			case 2:
				break;
			}
		}
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlLevelBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(bombBurstMode==0)
	{
		waitTimer-=dt;
		if(fomPosition==1)
		{
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0);
			air.Prop().SetThrottle(0.8);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
	}
	else
	{
		if(fomPosition==1)
		{
			switch(bombBurstMode)
			{
			case 1:
				air.Prop().GController(4.0);
				air.Prop().BankController(0.0);
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			case 2:
				air.Prop().GController(1.0);
				air.Prop().TurnOffBankController();
				air.Prop().SetAileron(-0.5);
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			case 3:
				ControlGForVerticalSpeed(air,sim,0.0);
				air.Prop().BankController(0.0);
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			}
		}
		else if(fomPosition==4)
		{
			switch(bombBurstMode)
			{
			case 1:
				air.Prop().GController(0.0);
				air.Prop().TurnOffBankController();
				air.Prop().SetAileron(0.5);
				air.Prop().TurnOffSpeedController();
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			case 2:
				ControlGForVerticalSpeed(air,sim,0.0);
				air.Prop().BankController(0.0);
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			}
		}
		else if(fomPosition==5 || fomPosition==6)
		{
			switch(bombBurstMode)
			{
			case 1:
				air.Prop().BankController(fomPosition==5 ? YsPi/4.0 : -YsPi/4.0);
				air.Prop().GController(4.0);
				air.Prop().TurnOffSpeedController();
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			case 2:
				air.Prop().TurnOffBankController();
				air.Prop().SetAileron(fomPosition==5 ? 0.8 : -0.8);
				air.Prop().TurnOffSpeedController();
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			case 3:
				ControlGForVerticalSpeed(air,sim,0.0);
				air.Prop().BankController(0.0);
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			}
		}
		else if(fomPosition==2 || fomPosition==3)
		{
			double rollDir;
			rollDir=(fomPosition==2 ? 1.0 : -1.0);
			switch(bombBurstMode)
			{
			case 1:
				air.Prop().TurnOffBankController();
				air.Prop().SetAileron(0.8*rollDir);
				if(sin(att.b())*rollDir>0.0)
				{
					air.Prop().GController(1.0);
				}
				else
				{
					air.Prop().GController(0.0);
				}
				air.Prop().TurnOffSpeedController();
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			case 2:
				ControlGForVerticalSpeed(air,sim,0.0);
				air.Prop().BankController(rollDir*YsPi/6.0);
				air.Prop().SetThrottle(0.8);
				air.Prop().SetAfterburner(YSFALSE);
				break;
			}
		}
	}

	air.Prop().SetSmokeButton(YSTRUE);

	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionRollBackToArrowhead(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	int i;

	if(fomPosition==1)
	{
		switch(brlMode)
		{
		case 0:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				for(i=1; i<4; i++)
				{
					FsAirshowControl *ap;
					ap=GetAirshowAP(formation[i]);
					if(ap!=NULL)
					{
						ap->brlMode=1;
					}
				}
				brlMode=1;
			}
			break;
		case 1:
			if((formation[1]==NULL || formation[1]->GetPosition().y()>pos.y()) &&
			   (formation[2]==NULL || formation[2]->GetPosition().y()>pos.y()))
			{
				double r;
				r=air.GetRadiusFromCollision()*0.85;

				for(i=1; i<4; i++)
				{
					FsAirshowControl *ap;
					ap=GetAirshowAP(formation[i]);
					if(ap!=NULL)
					{
						YsVec3 p1,p2;
						GetArrowheadPosition(p1,i+1,r);
						p2=p1;
						p1.MulX(1.5);
						ap->shouldBe=p1;
						ap->transition[ap->nTransition++]=(p1+p2)/2.0;
						ap->transition[ap->nTransition++]=p1*0.25+p2*0.75;
						ap->transition[ap->nTransition++]=p2;
					}
				}
				brlMode++;
			}
			break;
		case 2:
			break;
		}

		if(endOfAction==YSTRUE)
		{
		}
	}
	else if(fomPosition==2 || fomPosition==3)
	{
		switch(brlMode)
		{
		case 0:
			return FsFormation::MakeDecision(air,sim,dt);
		case 1:
			if(leader!=NULL && leader->GetPosition().y()+10.0<=pos.y())
			{
				rollIntegrator=0.0;
				brlMode++;
			}
			break;
		case 2:
			if(rollIntegrator>0.49 && fabs(att.b())<YsPi/2.0)
			{
				brlMode++;
			}
			else if(rollIntegrator<YsTolerance && fabs(att.b())>YsPi*3.0/4.0)
			{
				rollIntegrator=0.5;
			}
			break;
		case 3:
			if(fabs(att.b())<YsPi/9.0)
			{
				waitTimer=6.0;
				brlMode++;
			}
			break;
		case 4:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				FsAirshowControl *ap;
				if((ap=GetAirshowAP(leader))!=NULL)
				{
					ap->endOfAction=YSTRUE;
				}
			}
			break;
		}
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlRollBackToArrowhead(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(fomPosition==1)
	{
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().BankController(0.0);
		air.Prop().SetSmokeButton(YSFALSE);
	}
	else if(fomPosition==2 || fomPosition==3)
	{
		double rollDir,ail;
		rollDir=(fomPosition==2 ? 1.0 : -1.0);
		air.Prop().SetSmokeButton(YSTRUE);
		switch(brlMode)
		{
		case 0:
			return FsFormation::ApplyControl(air,sim,dt);
			break;
		case 1:
			ControlGForVerticalSpeed(air,sim,5.0);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.6);
			air.Prop().SetAfterburner(YSFALSE);
			air.Prop().BankController(0.0);
			break;
		case 2:
			air.Prop().GetRollRate(ail);
			ail=(YsPi*3.0/4.0)/ail;
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(ail*rollDir);
			if(rollIntegrator<YsTolerance)
			{
				air.Prop().GController(1.0);
			}
			else
			{
				air.Prop().GController(0.0);
			}
			air.Prop().SetThrottle(0.5);
			air.Prop().SetAfterburner(YSFALSE);
			break;
		case 3:
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SetThrottle(0.6);
			air.Prop().SetAfterburner(YSFALSE);
			air.Prop().BankController(0.0);
			break;
		case 4:
			return FsFormation::ApplyControl(air,sim,dt);
		}
	}
	else
	{
		air.Prop().SetSmokeButton(YSTRUE);
		return FsFormation::ApplyControl(air,sim,dt);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionPitchUpBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	int i;
	YsVec3 vel;
	FsAirplane *newLeader;
	FsAirshowControl *ap;

	if(fomPosition==1 && cpMode==0)
	{
		cpMode=10;
	}

	if(fomPosition==1 && endOfAction==YSTRUE)
	{
	}

	switch(cpMode)
	{
	case 0:
		return FsFormation::MakeDecision(air,sim,dt);
	case 10:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			if(fomPosition<NFORMATION && 
			   (newLeader=formation[fomPosition])!=NULL && 
			   (ap=GetAirshowAP(newLeader))!=NULL)
			{
				ap->waitTimer=1.6;
				ap->cpMode=10;
				for(i=fomPosition+1; i<NFORMATION; i++)
				{
					SwitchLeader(newLeader,formation[i]);
				}
			}
			else
			{
				endOfAction=YSTRUE;
			}
			cpMode++;
			entryAlt=pos.y();
		}
		break;
	case 11:
		air.Prop().GetVelocity(vel);
		if(att.p()>YsPi/4.0 || pos.y()>entryAlt+30.0 || vel.y()>15.0)
		{
			cpMode++;
			rollIntegrator=0.0;
		}
		break;
	case 12:
		if(fabs(att.b())>YsPi*3.0/4.0)
		{
			cpMode++;
		}
		break;
	case 13:
		air.Prop().GetVelocity(vel);
		if(vel.y()<0.0)
		{
			smokeOffTimer=3.0;
			cpMode++;
		}
		break;
	case 14:
		smokeOffTimer-=dt;
		if(smokeOffTimer<YsTolerance && endOfAction==YSTRUE)
		{
			ap=GetAirshowAP(formation[0]);
			if(ap!=NULL)
			{
				ap->endOfAction=YSTRUE;
			}
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlPitchUpBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	const double rollDir=-1.0;

	air.Prop().SetSmokeButton(YSTRUE);

	switch(cpMode)
	{
	case 0:
		return FsFormation::ApplyControl(air,sim,dt);
	case 10:
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().BankController(0.0);
		break;
	case 11:
		air.Prop().GController(5.0);
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		break;
	case 12:
		if(fabs(att.b())<YsPi/2.0)
		{
			air.Prop().GController(4.0);
		}
		else
		{
			air.Prop().GController(0.0);
		}
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(rollDir*0.9);
		break;
	case 13:
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().BankController(-rollDir*YsPi/2.0);
		air.Prop().GController(4.0);
		break;
	case 14:
		air.Prop().SetThrottle(0.6);
		air.Prop().SetAfterburner(YSFALSE);
		air.Prop().BankController(-rollDir*YsPi*7.0/18.0);
		ControlGForVerticalSpeed(air,sim,0.0);
		break;
	}

	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionRockWing(FsAirplane &air,FsSimulation *,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	double b;

	switch(rwMode)
	{
	case 0:
		if(entrySpeed<YsTolerance)
		{
			entrySpeed=air.Prop().GetVelocity();
		}
		if(entryAlt<YsTolerance)
		{
			entryAlt=pos.y();
		}

		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			waitTimer=5.0;
			rwMode=1;
		}
		break;

	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		b=((rwMode%2) ? 1.0 : -1.0);
		if(att.b()*b>YsPi/4.0)
		{
			rwMode++;
		}
		break;

	default:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}

	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlRockWing(FsAirplane &air,FsSimulation *sim,const double & /*dt*/ )
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	double b,r;

	switch(rwMode)
	{
	default:
		ControlGForAltitude(air,sim,entryAlt,0.0);
		air.Prop().SpeedController(entrySpeed);
		air.Prop().BankController(0.0);
		air.Prop().SetGear(rwDirty==YSTRUE ? 1.0 : 0.0);
		air.Prop().SetRudder(0.0);
		break;

	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		b=((rwMode%2) ? 1.0 : -1.0);
		ControlGForAltitude(air,sim,entryAlt,0.0);
		air.Prop().SpeedController(entrySpeed);
		air.Prop().BankController(b*YsPi/4.0,YsPi/4.0);
		air.Prop().SetGear(rwDirty==YSTRUE ? 1.0 : 0.0);
		r=-att.b()/(YsPi/4.0);
		air.Prop().SetRudder(r);
		break;
	}

	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionLetterEight(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1)
	{
		switch(turnMode)
		{
		case 0:
			if(entrySpeed<YsTolerance)
			{
				entrySpeed=air.Prop().GetVelocity();
			}
			if(entryAlt<YsTolerance)
			{
				entryAlt=pos.y();
			}

			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				turnMode++;
				headingIntegrator=0.0;
				entryHeading=att.h();

				FsAirshowControl *ap;
				ap=GetAirshowAP(formation[3]);
				if(ap!=NULL)
				{
					ap->turnMode=1;
					ap->entrySpeed=entrySpeed;
					ap->entryAlt=entryAlt;
					ap->entryHeading=entryHeading;
				}
			}
			break;
		case 1:
			{
				double rx,rz,vx,vz,d;
				rx=cos(entryHeading);
				rz=sin(entryHeading);
				vx=cos(att.h());
				vz=sin(att.h());

				d=rx*vx+rz*vz;
				if(1.49<headingIntegrator && d>cos(YsDegToRad(15.0)))
				{
					waitTimer=14.0;
					turnMode++;
				}
				else if(0.99<headingIntegrator && headingIntegrator<1.49 && d>0.0)
				{
					headingIntegrator=1.5;
				}
				else if(0.49<headingIntegrator && headingIntegrator<0.99 && d<-0.9)
				{
					headingIntegrator=1.0;
				}
				else if(headingIntegrator<YsTolerance && d<0.0)
				{
					headingIntegrator=0.5;
				}
			}
			break;
		case 2:
			{
				waitTimer-=dt;
				if(waitTimer<YsTolerance)
				{
					endOfAction=YSTRUE;
				}
			}
			break;
		}
	}
	else if(fomPosition==4)
	{
		switch(turnMode)
		{
		default:
		case 0:
			return FsFormation::MakeDecision(air,sim,dt);
		case 1:
		case 2:
			{
				double rx,rz,vx,vz,d;
				rx=cos(entryHeading);
				rz=sin(entryHeading);
				vx=cos(att.h());
				vz=sin(att.h());

				d=rx*vx+rz*vz;
				if(1.49<headingIntegrator && d>cos(YsDegToRad(5.0)))
				{
					headingIntegrator=0.0;
					turnMode++;
				}
				else if(0.99<headingIntegrator && headingIntegrator<1.49 && d>0.0)
				{
					headingIntegrator=1.5;
				}
				else if(0.49<headingIntegrator && headingIntegrator<0.99 && d<-0.9)
				{
					headingIntegrator=1.0;
				}
				else if(headingIntegrator<YsTolerance && d<0.0)
				{
					headingIntegrator=0.5;
				}

				if(turnMode==2 && leader!=NULL)
				{
					YsVec3 rel;
					rel=leader->GetPosition()-pos;
					rel.RotateXZ(-att.h());
					if(rel.z()>YsTolerance && fabs(rel.x()/rel.z())<tan(YsPi/36.0))
					{
						turnMode++;
					}
				}
			}
			break;
		case 3:
			if(leader!=NULL)
			{
				YsVec3 rel;
				leader->GetAttitude().Mul(rel,shouldBe);
				rel+=leader->GetPosition();
				att.MulInverse(rel,rel-pos);
				if(rel.z()<air.GetRadiusFromCollision()*8.0)  // 2*n*r -> n times the length of the airplane
				{
					nTransition=1;
					transition[0]=shouldBe;
					shouldBe.SubY(2.0);

					turnMode=4;
				}
			}
			break;
		case 4:
			if(leader!=NULL)
			{
				YsVec3 rel;
				double dv,r;

				leader->GetAttitude().Mul(rel,shouldBe);
				rel+=leader->GetPosition();
				att.MulInverse(rel,rel-pos);

				dv=air.Prop().GetVelocity()-leader->Prop().GetVelocity();

				FsAirshowControl *ap;
				ap=GetAirshowAP(leader);
				if(ap!=NULL)
				{
					if(fabs(dv)>2.0)
					{
						r=rel.z()/dv;   //   Small -> Speed difference too large -> leader should accelerate
						                //   Large -> Speed difference too small -> leader should decelerate

						if(r<1.7)
						{
							ap->letterEightBoost=1;
						}
						else if(r>6.0)
						{
							ap->letterEightBoost=-1;
						}
					}
				}
			}
			return FsFormation::MakeDecision(air,sim,dt);
		}
	}
	else
	{
		return FsFormation::MakeDecision(air,sim,dt);
	}

	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlLetterEight(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1)
	{
		
		switch(turnMode)
		{
		case 0:
			air.Prop().BankController(0.0,YsPi/4.0);
			ControlGForAltitude(air,sim,entryAlt,0.0);
			break;
		case 1:
			air.Prop().BankController(YsPi/3.0,YsPi/4.0);
			ControlGForAltitude(air,sim,entryAlt,0.0,6.0);
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 2:
			air.Prop().BankController(0.0,YsPi/6.0);
			ControlGForAltitude(air,sim,entryAlt,0.0);
			break;
		}

		if(letterEightBoost==0)
		{
			air.Prop().SpeedController(entrySpeed);
		}
		else if(letterEightBoost>0)
		{
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.9);
			air.Prop().SetAfterburner(YSTRUE);
		}
		else
		{
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.2);
			air.Prop().SetAfterburner(YSFALSE);
		}
	}
	else if(fomPosition==4)
	{
		switch(turnMode)
		{
		default:
		case 0:
			return FsFormation::ApplyControl(air,sim,dt);
		case 1:
			air.Prop().BankController(-YsPi*7.5/18.0);
			air.Prop().SpeedController(entrySpeed*0.9);
			ControlGForAltitude(air,sim,entryAlt,0.0,6.0);
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 2:
			air.Prop().BankController(YsPi*7.5/18.0);
			air.Prop().SpeedController(entrySpeed*0.9);
			ControlGForAltitude(air,sim,entryAlt,0.0,6.0);
			air.Prop().SetSmokeButton(YSTRUE);
			break;
		case 3:
			if(leader!=NULL)
			{
				YsVec3 aim,leadPos;
				double deltaP,deltaB;

				leadPos=leader->GetPosition();


				YsVec3 leadVel;
				double t;
				att.MulInverse(aim,leadPos-pos);
				t=YsBound(aim.z()/600.0,1.0,3.0);

				leader->Prop().GetVelocity(leadVel);
				leadPos+=leadVel*t;


				att.MulInverse(aim,leadPos-pos);
				deltaP=atan2(aim.y(),aim.z());

				deltaB=atan2(-aim.x(),aim.y());
				air.Prop().BankController(att.b()+deltaB);

				double g;
				g=leader->Prop().GetG()+(deltaP-YsPi/36.0)/YsDegToRad(1.0);
				g=YsBound(g,0.0,9.0);
				air.Prop().GController(g);


				double speedCorrection;
				if(aim.z()<600.0)
				{
					speedCorrection=1.2;
				}
				else
				{
					speedCorrection=1.5;
				}
				air.Prop().SpeedController(entrySpeed*speedCorrection);
			}
			break;
		case 4:
			return FsFormation::ApplyControl(air,sim,dt);
		}
	}
	else
	{
		return FsFormation::ApplyControl(air,sim,dt);
	}

	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionStarCross(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	int i;

	switch(bombBurstMode)
	{
	case 0:
		if(entryAlt<YsTolerance)
		{
			entryAlt=pos.y();
		}

		if(fomPosition==1)
		{

			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				bombBurstMode=1;
			}
		}
		else
		{
			return FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 1:  // fomPosition==1 only
		if(att.p()>YsDegToRad(87.0) && fomPosition==1)
		{
			int i;
			double *angle;
			double starCrossAngle[]={0.0, 72.0,  -72.0,  180.0, 144.0,  -144.0};
			double bombBurst6Angle[]={0.0, 60.0, -60.0, 180.0, 120.0, -120.0};

			angle=(action==STARCROSS ? starCrossAngle : bombBurst6Angle);

			bombBurstBreakPoint=pos;
			for(i=0; i<6; i++)
			{
				YsVec3 v;
				v.Set(-sin(YsDegToRad(angle[i])),cos(YsDegToRad(angle[i])),0.0);
				att.Mul(v,v);
				v*=10000.0;
				v+=pos;

				FsAirshowControl *ap;
				if(formation[i]!=NULL && (ap=GetAirshowAP(formation[i]))!=NULL)
				{
					ap->bombBurstRefPoint=v;
					ap->bombBurstBreakPoint=pos;
					ap->bombBurstMode=2;
				}
			}

			double rr;
			air.Prop().GetRollRate(rr);
			waitTimer=YsPi/rr;
			bombBurstMode=2;
		}
		break;
	case 2:  // Break
		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				bombBurstMode=3;
			}
		}
		else
		{
			YsVec3 uv;
			double a;
			uv=bombBurstRefPoint-pos;
			att.MulInverse(uv,uv);
			a=atan2(-uv.x(),uv.y());
			if(fabs(a)<YsPi/36.0)
			{
				bombBurstMode=3;
			}
		}
		break;
	case 3:  // After break, until pitch=0
		if(att.p()<YsPi/60.0)
		{
			if(entrySpeed<YsTolerance)
			{
				entrySpeed=air.Prop().GetVelocity();
			}

			if(fomPosition==1)
			{
				for(i=0; i<6; i++)
				{
					FsAirshowControl *ap;
					ap=GetAirshowAP(formation[i]);
					if(ap!=NULL)
					{
						ap->entrySpeed=entrySpeed;
					}
				}
			}

			bombBurstMode=4;
			switch(action)
			{
			case STARCROSS:
				waitTimer=15.0;
				break;
			case STAROFDAVID:
				waitTimer=12.0;
				break;
			default:
				waitTimer=9.0;
				break;
			}
		}
		break;
	case 4:
		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				for(i=0; i<6; i++)
				{
					FsAirshowControl *ap;
					ap=GetAirshowAP(formation[i]);
					if(ap!=NULL)
					{
						ap->bombBurstMode=5;
						ap->pitchIntegrator=0.0;
						if(action==BOMBBURST6SHIP)
						{
							ap->entryAlt=entryAlt+25.0*(double)i;
						}
					}
				}
				bombBurstMode=5;
				pitchIntegrator=0.0;
			}
		}
		break;
	case 5:
		{
			if(0.99<pitchIntegrator && att.p()>-YsPi/60.0)
			{
				bombBurstMode=((action==STARCROSS || STAROFDAVID==action) ? 6 : 100);
				waitTimer=((action==STARCROSS || STAROFDAVID==action) ? 3.0 : 5.0);
			}
			else if(0.49<pitchIntegrator && pitchIntegrator<0.99 && att.p()>-YsPi/4.0)
			{
				pitchIntegrator=1.0;
			}
			else if(pitchIntegrator<YsTolerance && att.p()<-YsDegToRad(70.0))
			{
				pitchIntegrator=0.5;
			}
		}
		break;
	case 6:
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				if(STARCROSS==action)
				{
					entryHeading=att.h()-YsDegToRad(18.0);  // One corner angle of a star is 36 degree.
					                                 // Outer angle = 2 rotation (720 degree) / 5 corners
					waitTimer=0.0;
				}
				else // if(STAROFDAVID==action)
				{
					entryHeading=att.h()-YsDegToRad(15.0);
					waitTimer=3.0;
				}
				bombBurstMode=7;
			}
		}
		break;
	case 7:
		{
			YsVec3 v;
			v.Set(-sin(entryHeading),0.0,cos(entryHeading));
			v.RotateXZ(-att.h());
			if(fabs(v.x())<sin(YsPi/60.0))
			{
				waitTimer-=dt;
				if(waitTimer<0.0)
				{
					bombBurstRefPoint=pos;
					bombBurstMode=8;
				}
			}
		}
		break;
	case 8:
		{
			FsAirplane *refAir=GetStarCrossRefAir(fomPosition);
			FsAirshowControl *refAp;
			if(refAir!=NULL && (refAp=GetAirshowAP(refAir))!=NULL && refAp->bombBurstMode>=8)
			{
				YsVec3 v;
				v=refAp->bombBurstRefPoint-pos;
				v.RotateXZ(-att.h());
				if(v.z()<0.0)
				{
					bombBurstMode=9;
					waitTimer=4.0;
				}
			}
		}
		break;
	case 9:
		waitTimer-=dt;
		if(waitTimer<0.0)
		{
			endOfAction=YSTRUE;
		}
		break;

	case 100:  // BOMBBURST6SHIP
		{
			YsVec3 rel;
			rel=bombBurstBreakPoint-pos;
			rel.RotateXZ(-att.h());
			if(rel.z()<0.0)
			{
				waitTimer-=dt;
				if(waitTimer<0.0)
				{
					endOfAction=YSTRUE;
				}
			}
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlStarCross(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(bombBurstMode)
	{
	case 0:
		if(fomPosition==1)
		{
			ControlGForAltitude(air,sim,entryAlt,0.0);
			air.Prop().SetThrottle(0.9);
			air.Prop().BankController(0.0);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 1:
		if(fomPosition==1)
		{
			air.Prop().GController(4.0);
			air.Prop().SetThrottle(0.9);
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
		}
		break;
	case 2:
	case 3:
		if(bombBurstMode==2)
		{
			air.Prop().GController(0.0);
		}
		else
		{
			air.Prop().GController(4.0);
		}
		air.Prop().BankController(bombBurstRefPoint-pos);
		air.Prop().TurnOffSpeedController();
		air.Prop().SetThrottle(0.9);
		air.Prop().SetSmokeButton(YSTRUE);
		break;

	case 4:   // Take distance
		{
			YsVec3 rel;
			double a,b;
			rel=bombBurstRefPoint-pos;
			rel.RotateXZ(-att.h());
			a=atan2(-rel.x(),rel.z());
			b=YsBound(a*3.0,-YsPi/4.0,YsPi/4.0);

			air.Prop().BankController(b);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().SetSmokeButton(YSFALSE);
		}
		break;
	case 5:    // Split-S
		{
			YsVec3 uv;
			uv=bombBurstBreakPoint-pos;
			air.Prop().BankController(uv);
			air.Prop().SpeedController(entrySpeed);
			att.MulInverse(uv,uv);
			air.Prop().SetSmokeButton(YSFALSE);

			if(action==BOMBBURST6SHIP && pitchIntegrator>0.99)
			{
				ControlGForRollOut(air,sim,entryAlt,YsPi/6.0,-YsPi/6.0,5.0,-2.0);
			}
			else
			{
				if(fabs(atan2(uv.x(),uv.y()))<YsPi/6.0)
				{
					air.Prop().GController(4.0);
				}
				else
				{
					air.Prop().GController(0.0);
				}
			}
		}
		break;
	case 6:
		{
			air.Prop().BankController(0.0);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		break;
	case 7:
		{
			double dh,b;
			YsVec3 v;
			v.Set(-sin(entryHeading),0.0,cos(entryHeading));
			v.RotateXZ(-att.h());
			dh=-asin(v.x());
			b=YsBound(dh*5.0,-YsDegToRad(80.0),YsDegToRad(80.0));

			air.Prop().BankController(b);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		break;
	case 8:
	case 9:
		{
			FsAirplane *refAir=GetStarCrossRefAir(fomPosition);
			FsAirshowControl *refAp;
			if(refAir!=NULL && (refAp=GetAirshowAP(refAir))!=NULL && refAp->bombBurstMode==8)
			{
				double dh,b;
				YsVec3 v;
				v=refAp->bombBurstRefPoint-pos;
				v.RotateXZ(-att.h());
				if(v.z()>100.0)
				{
					dh=atan2(-v.x(),v.z());
					b=YsBound(dh*10.0,-YsPi/4.0,YsPi/4.0);
				}
				else
				{
					b=0.0;
				}

				if(v.z()<0.0)
				{
					air.Prop().SetSmokeButton(YSFALSE);
				}
				else
				{
					air.Prop().SetSmokeButton(YSTRUE);
				}

				air.Prop().BankController(b);
				air.Prop().SpeedController(entrySpeed);
				ControlGForAltitude(air,sim,refAp->bombBurstRefPoint.y(),0.0);
			}
			else
			{
				air.Prop().BankController(0.0);
				air.Prop().SpeedController(entrySpeed);
				ControlGForVerticalSpeed(air,sim,0.0);
				air.Prop().SetSmokeButton(YSFALSE);
			}
		}
		break;


	case 100:  // Bomb Burst 6 Ship
		air.Prop().BankController(0.0);
		air.Prop().SpeedController(entrySpeed);
		ControlGForAltitude(air,sim,entryAlt,0.0);
		air.Prop().SetSmokeButton(YSTRUE);
		break;
	}
	return YSOK;
}

FsAirplane *FsAirshowControl::GetStarCrossRefAir(int fomPosition) const
{
	FsAirplane *refAir=nullptr;
	if(STARCROSS==action)
	{
		// 1->5 [5]
		// 2->3 [2]
		// 3->4 [4]
		// 4->1 [0]
		// 5->2 [1]
		switch(fomPosition)
		{
		default:
			refAir=NULL;
			break;
		case 1:
			refAir=formation[5];
			break;
		case 2:
			refAir=formation[2];
			break;
		case 3:
			refAir=formation[4];
			break;
		case 5:
			refAir=formation[0];
			break;
		case 6:
			refAir=formation[1];
			break;
		}
	}
	else // if(STAROFDAVID==action)
	{
		// 1->6 [5]
		// 2->3 [2]
		// 3->4 [3]
		// 4->2 [1]
		// 5->1 [0]
		// 6->5 [4]
		switch(fomPosition)
		{
		default:
			refAir=NULL;
			break;
		case 1:
			refAir=formation[5];
			break;
		case 2:
			refAir=formation[2];
			break;
		case 3:
			refAir=formation[3];
			break;
		case 4:
			refAir=formation[1];
			break;
		case 5:
			refAir=formation[0];
			break;
		case 6:
			refAir=formation[4];
			break;
		}
	}
	return refAir;
}

YSRESULT FsAirshowControl::MakeDecisionLevelOpener(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	int i;
	FsAirshowControl *ap;

	switch(bombBurstMode)
	{
	case 0:
		if(entrySpeed<YsTolerance)
		{
			entrySpeed=air.Prop().GetVelocity();
		}

		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				double h;
				h=YsPi*2.0/18.0;
				for(i=1; i<=2; i++)
				{
					ap=GetAirshowAP(formation[i]);
					if(ap!=NULL)
					{
						ap->bombBurstMode=2;
						ap->entryHeading=att.h()+h;
					}
					h=-h;
				}

				waitTimer=1.0;
				bombBurstMode=1;
			}
		}
		else
		{
			FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 1:
		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				double h;
				h=YsPi/18.0;
				for(i=0; i<=3; i+=3)
				{
					ap=GetAirshowAP(formation[i]);
					if(ap!=NULL)
					{
						ap->bombBurstMode=2;
						ap->entryHeading=att.h()+h;
					}
					h=-h;
				}

				ap=GetAirshowAP(formation[4]);
				if(ap!=NULL)
				{
					ap->bombBurstMode=2;
					ap->entryHeading=att.h();
				}

				waitTimer=4.0;
				bombBurstMode=2;
			}
		}
		break;
	case 2:
		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
				   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
				   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
				   air.Prop().GetAirplaneCategory()==FSAC_TRAINER)
				{
					for(i=0; i<NFORMATION; i++)
					{
						ap=GetAirshowAP(formation[i]);
						if(ap!=NULL)
						{
							ap->bombBurstMode=3;
						}
					}
				}
				else
				{
					waitTimer=6.0;
					bombBurstMode=4;
				}
			}
		}
		break;
	case 3:
		if(rollIntegrator>0.4 && fabs(att.b())<YsPi*0.7)
		{
			waitTimer=6.0;
			bombBurstMode=4;
		}
		else if(rollIntegrator<YsTolerance && fabs(att.b())>YsPi*0.9)
		{
			rollIntegrator=0.5;
		}
		break;
	case 4:
		waitTimer-=dt;
		if(waitTimer<0.0)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlLevelOpener(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	switch(bombBurstMode)
	{
	case 0:
	case 1:
		if(fomPosition==1)
		{
			air.Prop().SpeedController(entrySpeed);
			air.Prop().BankController(0.0);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 2:
		{
			double dh,b;
			YsVec3 v;
			v.Set(-sin(entryHeading),0.0,cos(entryHeading));
			v.RotateXZ(-att.h());
			dh=-asin(v.x());
			b=YsBound(dh*5.0,-YsPi/3.0,YsPi/3.0);

			air.Prop().BankController(b);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		break;
	case 3:
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(turnDir*0.6);
		air.Prop().GController(1.0+cos(att.b()));

		if(YsPi/4.0<att.b() && att.b()<YsPi*3.0/4.0)
		{
			air.Prop().SetRudder(-1.0);
		}
		else if(-YsPi/4.0>att.b() && att.b()>-YsPi*3.0/4.0)
		{
			air.Prop().SetRudder(1.0);
		}
		else
		{
			air.Prop().SetRudder(0.0);
		}

		break;
	case 4:
		air.Prop().SpeedController(entrySpeed);
		air.Prop().BankController(0.0,YsPi/4.0);
		ControlGForVerticalSpeed(air,sim,0.0);
		break;
	}
	air.Prop().SetSmokeButton(YSTRUE);
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionFormationBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1 && endOfAction==YSTRUE)
	{
	}

	switch(cpMode)
	{
	case 0:
		if(entrySpeed<YsTolerance)
		{
			entrySpeed=air.Prop().GetVelocity();
		}

		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				waitTimer=1.0;
				cpMode=10;
				cpEntryHeading=att.h();
			}
		}
		else
		{
			return FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 10:
		waitTimer-=dt;
		if(waitTimer<0.0)
		{
			FsAirshowControl *airshow;
			FsAirplane *newLeader;
			int i;
			cpMode=11;

			if(fomPosition<NFORMATION && (newLeader=formation[fomPosition])!=NULL)
			{
				if((airshow=GetAirshowAP(newLeader))!=NULL)
				{
					airshow->cpMode=10;
					airshow->waitTimer=2.0;
					for(i=fomPosition+1; i<NFORMATION; i++)
					{
						SwitchLeader(newLeader,formation[i]);
					}
				}
			}
			else
			{
				for(i=0; i<NFORMATION; i++)
				{
					if(formation[i]!=NULL && (airshow=GetAirshowAP(formation[i]))!=NULL)
					{
						airshow->smokeOffTimer=8.0;
						airshow->cpMode=12;
					}
				}
			}
		}
		break;
	case 11:
		break;
	case 12:
		smokeOffTimer-=dt;
		if(smokeOffTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlFormationBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	// const YsAtt3 &att=air.GetAttitude();

	air.Prop().SetSmokeButton(YSTRUE);

	switch(cpMode)
	{
	case 0:
		if(fomPosition==1)
		{
			air.Prop().BankController(0.0);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 10:
		air.Prop().BankController(0.0);
		air.Prop().SpeedController(entrySpeed);
		ControlGForVerticalSpeed(air,sim,0.0);
		break;
	case 11:
	case 12:
		if(air.Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
		   air.Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
		   air.Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
		   air.Prop().GetAirplaneCategory()==FSAC_TRAINER)
		{
			air.Prop().BankController(turnDir*YsPi/3.0);
		}
		else
		{
			air.Prop().BankController(turnDir*YsPi/4.0);
		}
		air.Prop().SpeedController(entrySpeed);
		ControlGForVerticalSpeed(air,sim,0.0);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionDoubleFarvel(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	FsAirshowControl *ap;
	YsAtt3 vAtt;
	double r;
	YsVec3 p;
	int i,i0;

	switch(ivMode)
	{
	case 0:
		entryHeading=att.h();
		if(entrySpeed<YsTolerance)
		{
			entrySpeed=air.Prop().GetVelocity();
		}
		if(entryAlt<YsTolerance)
		{
			entryAlt=pos.y();
		}
		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				rollIntegrator=0.0;
				pitchIntegrator=0.0;
				ivMode=1;
				ap=GetAirshowAP(formation[3]);
				if(ap!=NULL)
				{
					ap->rollIntegrator=0.0;
					ap->pitchIntegrator=0.0;
					ap->ivMode=1;
				}
			}
		}
		break;
	case 1:
		air.Prop().GetRotationSpeed(vAtt);
		if(pitchIntegrator<YsTolerance && (att.p()>YsPi/60.0 || pos.y()>entryAlt+1.5))
		{
			pitchIntegrator=1.0;
		}
		else if(rollIntegrator>0.5 && fabs(att.b())>YsPi*178.0/180.0 && fabs(vAtt.b())<YsPi/180.0)
		{
			if(fomPosition==1)
			{
				if(action!=MIRRORROLL)
				{
					ivMode=2;
					waitTimer=10.0;
				}
				else
				{
					ivMode=100;
					waitTimer=3.0;
				}
				r=air.GetRadiusFromCollision()*0.85;
				for(i=2; i<=4; i++)
				{
					ap=GetAirshowAP(formation[i-1]);
					if(ap!=NULL)
					{
						GetDeltaPosition(p,i,r);
						p.MulY(1.0);
						if(i!=4)
						{
							p.MulX(-1.0);
							ap->inverted=YSTRUE;
						}
						ap->nTransition=2;
						ap->transition[1]=p;
						p.MulX(1.2);
						ap->transition[0]=p;
						p.MulX(1.2);
						ap->shouldBe=p;
						ap->ivMode=2;
					}
				}
			}
		}
		else if(rollIntegrator<0.5 && fabs(att.b())>YsPi/2.0)
		{
			rollIntegrator=1.0;
		}
		break;
	case 2:
		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<0.0)
			{
				ivMode=3;
				waitTimer=2.0;
				r=air.GetRadiusFromCollision()*0.85;
				for(i=2; i<=4; i++)
				{
					ap=GetAirshowAP(formation[i-1]);
					if(ap!=NULL)
					{
						GetDeltaPosition(p,i,r);
						p.MulZ(2.0);
						ap->shouldBe=p;
						ap->ivMode=3;
						ap->waitTimer=2.0;
					}
				}
			}
		}
		else
		{
			return FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 3:
		waitTimer-=dt;
		if(waitTimer<0.0)
		{
			ivMode=4;
			waitTimer=4.0;
		}
		break;
	case 4:
		waitTimer-=dt;
		if(waitTimer<0.0)
		{
			endOfAction=YSTRUE;
		}
		break;

	case 100: // Double farvel roll
		waitTimer-=dt;
		if(waitTimer<0.0)
		{
			entryAlt=pos.y();
			ivMode++;
		}
		break;
	case 101:
		if(att.p()>YsDegToRad(25.0))
		{
			ivMode++;
			rollIntegrator=0.0;
		}
		break;
	case 102:
		i0=YsGreater((int)fabs(att.b()/(YsPi/18.0)),0);
		for(i=i0; i<18; i++)
		{
			brlPathAltitude[i]=pos.y();
		}
		if(fabs(att.b())<YsDegToRad(3.0))
		{
			ivMode++;
		}
		break;
	case 103:
		if(fabs(att.b())>YsDegToRad(177.0))
		{
			ivMode=2;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlDoubleFarvel(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();
	double ail,ailStep;

	switch(ivMode)
	{
	case 0:
		air.Prop().SpeedController(entrySpeed);
		air.Prop().BankController(0.0);
		ControlGForVerticalSpeed(air,sim,0.0);
		break;
	case 1:
		if(pitchIntegrator<YsTolerance)
		{
			air.Prop().SpeedController(entrySpeed);
			air.Prop().BankController(0.0);
			air.Prop().GController(2.0);
		}
		else if(rollIntegrator<0.5)
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(1.0*turnDir);
			air.Prop().GController(0.0);
		}
		else
		{
			air.Prop().BankController(YsPi,YsPi/6.0);
			if(fabs(att.b())>YsPi*5.0/6.0)
			{
				ControlGForVerticalSpeed(air,sim,0.0);
			}
			else
			{
				air.Prop().GController(0.0);
			}
		}
		air.Prop().SpeedController(entrySpeed);
		break;
	case 2:
		if(fomPosition==1)
		{
			air.Prop().BankController(YsPi,YsPi/4.0);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		else
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 3:
		if(fomPosition==1)
		{
			air.Prop().BankController(YsPi,YsPi/4.0);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		else if(fomPosition==4)
		{
			return FsFormation::ApplyControl(air,sim,dt);
		}
		else
		{
			double bnk;
			bnk=(fomPosition==2 ? YsPi/9.0 : -YsPi/9.0);
			air.Prop().BankController(bnk);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		break;
	case 4:
		if(fabs(att.b())>YsPi/2.0)
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(1.0*turnDir);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		else
		{
			double dh,b;
			YsVec3 v;
			v.Set(-sin(entryHeading),0.0,cos(entryHeading));
			v.RotateXZ(-att.h());
			dh=-asin(v.x());
			b=YsBound(dh*5.0,-YsPi/3.0,YsPi/3.0);

			air.Prop().BankController(b);
			air.Prop().SpeedController(entrySpeed);
			ControlGForVerticalSpeed(air,sim,0.0);
		}
		break;

	case 101:
		air.Prop().GController(-2.0);
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		air.Prop().SetThrottle(0.8);
		break;
	case 102:
		air.Prop().GController(-2.0);
		air.Prop().TurnOffBankController();
		air.Prop().GetRollRate(ail);
		ail=turnDir*(YsPi/9.0)/ail;  // 20 deg/sec roll
		ailStep=ail*dt;

		if(fabs(att.b())<YsPi/2.0)
		{
			// Must complete uninverting before the nose hits the horizon.
			double desigPitch;
			desigPitch=fabs(att.b())/2.0;
			if(att.p()<desigPitch)
			{
				ail*=1.2;
			}
		}

		if(fabs(air.Prop().GetAileron()-ail)<ailStep)
		{
			air.Prop().SetAileron(ail);
		}
		else if(ail<air.Prop().GetAileron())
		{
			air.Prop().SetAileron(ail-ailStep);
		}
		else
		{
			air.Prop().SetAileron(ail+ailStep);
		}
		air.Prop().SetThrottle(0.8);
		break;
	case 103:
		{
			air.Prop().GetRollRate(ail);
			ail=turnDir*(YsPi/9.0)/ail;

			int i;
			double alt,alt1,alt2,b1,b2;
			i=YsBound((int)(fabs(att.b())/(YsPi/18.0)),0,17);
			if(i>1)
			{
				alt1=brlPathAltitude[i-1];
				b1=(YsPi/18.0)*(double)(i-1);
				alt2=brlPathAltitude[i];
				b2=(YsPi/18.0)*(double)i;
			}
			else
			{
				alt1=entryAlt;
				b1=0.0;
				alt2=brlPathAltitude[1];
				b2=YsPi/18.0;
			}
			alt=alt1+(att.b()-b1)*(alt2-alt1)*(b2-b1);

			air.Prop().TurnOffBankController();
			if(pos.y()<alt)
			{
				if(fabs(att.b())<YsPi/2.0)
				{
					air.Prop().GController(-1.7);
				}

				air.Prop().SetAileron(ail*1.2);
				air.Prop().SetThrottle(0.6);
			}
			else
			{
				if(fabs(att.b())<YsPi/2.0)
				{
					air.Prop().GController(-2.3);
				}

				air.Prop().SetAileron(ail*0.8);
				air.Prop().SetThrottle(0.8);
			}

			YsAtt3 vAtt;
			YsVec3 ev;
			air.Prop().GetVelocity(ev);
			vAtt.SetTwoVector(ev,att.GetUpVector());

			if(fabs(att.b())>YsPi/2.0)
			{
				double corG,desigPitch;
				desigPitch=-fabs((YsPi-att.b())/2.0);
				if(vAtt.p()<desigPitch)
				{
					corG=YsRadToDeg(desigPitch-vAtt.p());
					corG/=5.0;  // 5.0 deg -> 1G
					corG=YsBound(corG,0.0,2.0);
				}
				else
				{
					corG=0.0;
				}
				air.Prop().GController(-2.0-corG);
			}
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionDiamond9ToSwanBend(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(fomPosition==1)
	{
		YsVec3 pos;
		YsVec2 v1,v2;

		switch(changeOverTurnMode)
		{
		case 0:
			if(changeOverTurnAltitude<YsTolerance)
			{
				changeOverTurnAltitude=air.GetPosition().y();
				changeOverTurnEntryHeading=air.GetAttitude().h();
			}
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				waitTimer=5.0;
				changeOverTurnMode=1;
				changeOverTurnEntrySpeed=air.Prop().GetVelocity();
			}
			break;
		case 1:
			{
				v1.Set(cos(changeOverTurnEntryHeading),sin(changeOverTurnEntryHeading));
				v2.Set(cos(air.GetAttitude().h()),sin(air.GetAttitude().h()));
				if(v1*v2<0.0)  // 90deg
				{
					double r;

					waitTimer=2.0;
					changeOverTurnMode=2;

					r=air.GetRadiusFromCollision()*0.85;

					GetSwan9Position(pos,5,r);

					FsFormation *fom;
					fom=GetFormationAP(formation[4]);
					if(fom!=NULL)
					{
						fom->shouldBe=pos;
					}

					pos.MulX(-1.0);
					fom=GetFormationAP(formation[5]);
					if(fom!=NULL)
					{
						fom->shouldBe=pos;
					}
				}
			}
			break;
		case 2:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				double r;

				waitTimer=2.0;
				changeOverTurnMode=3;

				r=air.GetRadiusFromCollision()*0.85;

				GetSwan9Position(pos,7,r);

				FsFormation *fom;
				fom=GetFormationAP(formation[6]);
				if(fom!=NULL)
				{
					fom->shouldBe=pos;
				}


				pos.MulX(-1.0);
				fom=GetFormationAP(formation[7]);
				if(fom!=NULL)
				{
					fom->shouldBe=pos;
				}
			}
			break;
		case 3:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				double r;

				waitTimer=2.0;
				changeOverTurnMode=4;

				r=air.GetRadiusFromCollision()*0.85;

				GetSwan9Position(pos,2,r);

				FsAirshowControl *fom;
				fom=GetAirshowAP(formation[1]);
				if(fom!=NULL)
				{
					fom->shouldBe=pos;
					fom->changeOverTurnMode=1;
				}


				pos.MulX(-1.0);
				fom=GetAirshowAP(formation[2]);
				if(fom!=NULL)
				{
					fom->shouldBe=pos;
					fom->changeOverTurnMode=1;
				}
			}
			break;

		case 4:
			{
				v1.Set(cos(changeOverTurnEntryHeading),sin(changeOverTurnEntryHeading));
				v2.Set(cos(air.GetAttitude().h()),sin(air.GetAttitude().h()));
				if(v1*v2>cos(YsDegToRad(5.0)))
				{
					waitTimer=5.0;
					changeOverTurnMode=5;
				}
			}
			break;
		case 5:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
	}
	else
	{
		FsFormation::MakeDecision(air,sim,dt);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlDiamond9ToSwanBend(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(fomPosition==1)
	{
		switch(changeOverTurnMode)
		{
		default:
			air.Prop().BankController(0.0,YsPi/7.0);
			air.Prop().SetSmokeButton(YSFALSE);
			air.Prop().SetThrottle(0.6);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			air.Prop().BankController(turnDir*changeOverTurnBankAngle,YsPi/6.0);
			air.Prop().SetSmokeButton(YSFALSE);
			air.Prop().SpeedController(changeOverTurnEntrySpeed);
			break;
		}
		ControlGForAltitude(air,sim,changeOverTurnAltitude,0.0);
	}
	else
	{
		FsFormation::ApplyControl(air,sim,dt);
		if(fomPosition==5 || fomPosition==6 || fomPosition==7 || fomPosition==8 || fomPosition==9)
		{
			air.Prop().SetSmokeButton(YSTRUE);
		}
		if((fomPosition==2 || fomPosition==3) && changeOverTurnMode>0)
		{
			air.Prop().SetSmokeButton(YSTRUE);
		}
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionSwanToApolloRoll(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(fomPosition==1)
	{
		return MakeDecisionBarrelRoll(air,sim,dt);
	}
	else
	{
		FsFormation::MakeDecision(air,sim,dt);
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlSwanToApolloRoll(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	if(fomPosition==1)
	{
		return ApplyControlBarrelRoll(air,sim,dt);
	}
	else
	{
		FsFormation::ApplyControl(air,sim,dt);
		if(fomPosition==5 || fomPosition==6 || fomPosition==7 || fomPosition==8 || fomPosition==9)
		{
			air.Prop().SetSmokeButton(YSTRUE);
		}
		if((fomPosition==2 || fomPosition==3) && brlMode==0)
		{
			air.Prop().SetSmokeButton(YSTRUE);
		}
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionLancasterTo5_4Split(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// loopMode
	// 0:Initial Straight
	// 1:Pull up to 80 degree pitch
	// 2:Roll
	// 3:Loop until pitch<-55 degree
	// 4:Loop until pitch>-50 degree
	// 5:Level off

	const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1 || (fomPosition==6 && loopMode>=2))
	{
		switch(loopMode)
		{
		case 0:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				loopMode++;
				if(fomPosition==1)
				{
					FsAirshowControl *ap;
					entryAlt=pos.y()+33.0;
					ap=GetAirshowAP(formation[5]);
					if(ap!=NULL)
					{
						ap->entryAlt=pos.y();
					}
				}
			}
			break;
		case 1:
			if(att.p()>YsPi*8.0/18.0)
			{
				YsAtt3 targetAtt;
				targetAtt=att;
				targetAtt.SetB(att.b()-YsPi/2.0);

				bombBurstDesigUv1=targetAtt.GetUpVector();

				FsAirshowControl *ap;
				ap=GetAirshowAP(formation[5]);
				if(ap!=NULL)
				{
					targetAtt.SetB(att.b()+YsPi/2.0);
					ap->bombBurstDesigUv1=targetAtt.GetUpVector();
					ap->loopMode=2;
					ap->loopG=loopG-0.1;

					int i;
					double r;
					YsVec3 ref,rel;
					r=air.GetRadiusFromCollision()*0.85;
					GetLancasterPosition(ref,6,r);
					for(i=7; i<=9; i++)
					{
						GetLancasterPosition(rel,i,r);
						rel-=ref;
						ap=GetAirshowAP(formation[i-1]);
						if(ap!=NULL)
						{
							ap->leader=formation[5];
							ap->shouldBe=rel;
							ap->loopMode=2;
						}
					}

					ap=GetAirshowAP(formation[1]);
					if(ap!=NULL)
					{
						ap->loopMode=2;
					}
					ap=GetAirshowAP(formation[2]);
					if(ap!=NULL)
					{
						ap->loopMode=2;
					}

					ap=GetAirshowAP(formation[3]);
					if(ap!=NULL)
					{
						GetDeltaPosition(rel,5,r);
						ap->shouldBe=rel;
						ap->loopMode=2;
					}
					ap=GetAirshowAP(formation[4]);
					if(ap!=NULL)
					{
						GetDeltaPosition(rel,6,r);
						ap->shouldBe=rel;
						ap->loopMode=2;
					}
				}

				loopMode++;
			}
			break;
		case 2:
			{
				YsVec3 relUv;
				att.MulInverse(relUv,bombBurstDesigUv1);
				if(relUv.y()>YsTolerance && fabs(relUv.x())/relUv.y()<0.05)
				{
					loopMode++;
				}
			}
			break;
		case 3:
			if(att.p()<-YsPi*55.0/180.0)
			{
				loopMode++;
			}
			break;
		case 4:
			if(att.p()>-YsPi*50.0/180.0)
			{
				loopMode++;
			}
			break;
		case 5:
			if(att.p()>-YsPi/36.0)
			{
				loopMode++;
				waitTimer=4.0;
			}
			break;
		case 6:
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				endOfAction=YSTRUE;
			}
			break;
		}
	}
	else
	{
		return FsFormation::MakeDecision(air,sim,dt);
	}

	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlLancasterTo5_4Split(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	// const YsAtt3 &att=air.GetAttitude();


	if(fomPosition==1 || (fomPosition==6 && loopMode>=2))
	{
		switch(loopMode)
		{
		case 0:  // Entry
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0,YsPi/5.0);
			air.Prop().SetThrottle(0.8);
			air.Prop().SetAfterburner(YSFALSE);
			break;
		case 1:  // Pull up
		case 3:
		case 4:
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			double g;

			air.Prop().SetThrottle(0.8);

			g=air.Prop().GetG();
			if(g+0.5<loopG)
			{
				air.Prop().GController(g+0.5);
			}
			else
			{
				air.Prop().GController(loopG);
			}
			break;
		case 2:  // Roll
			air.Prop().SetSmokeButton(YSTRUE);
			air.Prop().BankController(bombBurstDesigUv1,YsPi/9.0);
			air.Prop().GController(1.0);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.8);
			air.Prop().SetAfterburner(YSFALSE);
			break;
		case 5:  // Level off
		case 6:  // Level off
			ControlGForVerticalSpeed(air,sim,0.0,loopG);  //ControlGForAltitude(air,sim,entryAlt,0.0,loopG);
			air.Prop().BankController(0.0);
			air.Prop().SetThrottle(0.8);
			break;
		}
	}
	else
	{
		if(fomPosition==4 || fomPosition==5 || fomPosition==7 || fomPosition==8 || fomPosition==9)
		{
			air.Prop().SetSmokeButton(YSTRUE);
		}
		if((fomPosition==2 || fomPosition==3) && loopMode>=2)
		{
			air.Prop().SetSmokeButton(YSTRUE);
		}

		return FsFormation::ApplyControl(air,sim,dt);
	}

	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionChampaignSplit(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// bombBurstMode
	// 0: Entry
	// 1: Pull up to 80 degrees
	// 2: #1 to #5 roll, #6 to #9 continue loop until pitch<-80 degree
	// 3: #1 to #5 level off, #6 to #9 roll
	// 4: #6 to #9 level off
	// 5: fly straight

	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	YsVec3 ref,rel;
	double r;

	switch(bombBurstMode)
	{
	case 0:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			bombBurstMode++;
		}
		break;
	case 1:
		if(fomPosition==1)
		{
			if(att.p()>YsPi*8.0/18.0)
			{
				YsAtt3 targetAtt;

				waitTimer=1.0;
				targetAtt=att;
				bombBurstDesigUv1=att.GetUpVector();

				FsAirshowControl *ap;
				ap=GetAirshowAP(formation[1]); // #2
				if(ap!=NULL)
				{
					targetAtt.SetB(att.b()+YsPi/6.0);
					ap->bombBurstDesigUv1=targetAtt.GetUpVector();
					ap->bombBurstMode=2;
				}
				ap=GetAirshowAP(formation[2]); // #3
				if(ap!=NULL)
				{
					targetAtt.SetB(att.b()-YsPi/6.0);
					ap->bombBurstDesigUv1=targetAtt.GetUpVector();
					ap->bombBurstMode=2;
				}
				ap=GetAirshowAP(formation[3]); // #4
				if(ap!=NULL)
				{
					targetAtt.SetB(att.b()+YsPi/3.0);
					ap->bombBurstDesigUv1=targetAtt.GetUpVector();
					ap->bombBurstMode=2;
				}
				ap=GetAirshowAP(formation[4]); // #5
				if(ap!=NULL)
				{
					targetAtt.SetB(att.b()-YsPi/3.0);
					ap->bombBurstDesigUv1=targetAtt.GetUpVector();
					ap->bombBurstMode=2;
				}

				ap=GetAirshowAP(formation[5]); // #6
				if(ap!=NULL)
				{
					ap->bombBurstMode=2;
				}
				ap=GetAirshowAP(formation[6]); // #7
				if(ap!=NULL)
				{
					ap->bombBurstMode=2;
				}

				r=air.GetRadiusFromCollision()*0.85;

				ap=GetAirshowAP(formation[7]); // #8
				if(ap!=NULL)
				{
					GetChampaignPosition(ref,6,r);
					GetChampaignPosition(rel,8,r);
					rel-=ref;
					ap->bombBurstMode=2;
					ap->shouldBe=rel;
					ap->leader=formation[5];
				}
				ap=GetAirshowAP(formation[8]); // #9
				if(ap!=NULL)
				{
					GetChampaignPosition(ref,7,r);
					GetChampaignPosition(rel,9,r);
					rel-=ref;
					ap->bombBurstMode=2;
					ap->shouldBe=rel;
					ap->leader=formation[6];
				}

				bombBurstMode++;
			}
		}
		else
		{
			FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 2:
		if(1<=fomPosition && fomPosition<=5)
		{
			YsVec3 relUv;
			att.MulInverse(relUv,bombBurstDesigUv1);
			if(relUv.y()>YsTolerance && fabs(relUv.x())/relUv.y()<0.05)
			{
				bombBurstMode++;
			}
		}
		else if(fomPosition==6 || fomPosition==7)
		{
			if(att.p()<-YsPi*8.0/18.0)
			{
				YsAtt3 targetAtt;
				FsAirshowControl *ap;

				targetAtt=att;
				if(fomPosition==6)
				{
					targetAtt.SetB(att.b()+YsPi/6.0);
					bombBurstDesigUv1=targetAtt.GetUpVector();
					bombBurstMode++;

					ap=GetAirshowAP(formation[7]); // #8
					if(ap!=NULL)
					{
						targetAtt.SetB(att.b()+YsPi/3.0);
						ap->bombBurstDesigUv1=targetAtt.GetUpVector();
						ap->bombBurstMode=3;
					}
				}
				else
				{
					targetAtt.SetB(att.b()-YsPi/6.0);
					bombBurstDesigUv1=targetAtt.GetUpVector();
					bombBurstMode++;

					ap=GetAirshowAP(formation[8]); // #9
					if(ap!=NULL)
					{
						targetAtt.SetB(att.b()-YsPi/3.0);
						ap->bombBurstDesigUv1=targetAtt.GetUpVector();
						ap->bombBurstMode=3;
					}
				}
			}
		}
		else
		{
			FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 3:
		if(1<=fomPosition && fomPosition<=5)
		{
			if(att.p()<YsPi/36.0)
			{
				waitTimer=45.0;
				bombBurstMode=5;
			}
		}
		else
		{
			YsVec3 relUv;
			att.MulInverse(relUv,bombBurstDesigUv1);
			if(relUv.y()>YsTolerance && fabs(relUv.x())/relUv.y()<0.05)
			{
				bombBurstMode++;
			}
		}
		break;
	case 4:
		if(att.p()>-YsPi/36.0)
		{
			bombBurstMode++;
			waitTimer=5.0;
		}
		break;
	case 5:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			FsAirshowControl *ap;
			ap=GetAirshowAP(formation[0]);
			if(ap!=NULL)
			{
				ap->endOfAction=YSTRUE;
			}
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlChampaignSplit(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	air.Prop().SetSmokeButton(YSTRUE);

	switch(bombBurstMode)
	{
	case 0:
		if(fomPosition==1)
		{
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0);
			air.Prop().SetThrottle(0.8);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else
		{
			FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 1:
		if(fomPosition==1)
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			air.Prop().SetThrottle(0.8);
			air.Prop().GController(4.0);
		}
		else
		{
			FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 2:
		if(1<=fomPosition && fomPosition<=5)
		{
			air.Prop().BankController(bombBurstDesigUv1);
			air.Prop().GController(4.0);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.8);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else if(fomPosition==6 || fomPosition==7)
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			if(att.p()>0.0)
			{
				air.Prop().SetThrottle(0.8);
			}
			else
			{
				air.Prop().SetThrottle(0.6);
			}
			air.Prop().GController(5.0);
		}
		else
		{
			FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 3:
		if(1<=fomPosition && fomPosition<=5)
		{
			air.Prop().TurnOffBankController();
			air.Prop().SetAileron(0.0);
			air.Prop().SetThrottle(0.8);
			air.Prop().GController(4.0);
			air.Prop().SetAfterburner(YSFALSE);
		}
		else
		{
			air.Prop().BankController(bombBurstDesigUv1);
			air.Prop().GController(5.0);
			air.Prop().TurnOffSpeedController();
			air.Prop().SetThrottle(0.6);
			air.Prop().SetAfterburner(YSFALSE);
		}
		break;

	case 4:
		air.Prop().TurnOffBankController();
		air.Prop().SetAileron(0.0);
		air.Prop().SetThrottle(0.6);
		air.Prop().GController(5.0);
		air.Prop().SetAfterburner(YSFALSE);
		break;

	case 5:
		ControlGForVerticalSpeed(air,sim,0.0);
		air.Prop().BankController(0.0);
		air.Prop().SetThrottle(0.8);
		air.Prop().SetAfterburner(YSFALSE);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionVixenBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	switch(loopMode)
	{
	case 0:
	case 1:
	case 2:
		if(fomPosition==1)
		{
			MakeDecisionLoop(air,sim,dt);
		}
		else
		{
			FsFormation::MakeDecision(air,sim,dt);
		}
		if(loopMode==3)
		{
			waitTimer=5.0;
		}
		break;
	case 3:
		if(fomPosition==1)
		{
			waitTimer-=dt;
			if(waitTimer<YsTolerance)
			{
				int i;
				for(i=0; i<7; i++)
				{
					FsAirshowControl *ap;
					ap=GetAirshowAP(formation[i]);
					if(ap!=NULL)
					{
						ap->waitTimer=2.5;
						ap->loopMode=4;
					}
				}
			}
		}
		else
		{
			FsFormation::MakeDecision(air,sim,dt);
		}
		break;
	case 4:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			waitTimer=8.0;
			loopMode++;
		}
		break;
	case 5:
		waitTimer-=dt;
		if(waitTimer<YsTolerance)
		{
			endOfAction=YSTRUE;
		}
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::ApplyControlVixenBreak(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	double breakBank[]=
	{
		0.0,
		YsPi/6.0,
		-YsPi/6.0,
		YsPi/3.0,
		-YsPi/3.0,
		YsPi*7.0/18.0,
		-YsPi*7.0/18.0,
	};

	double breakPitch[]=
	{
		YsPi/4.0,
		YsPi/6.0,
		YsPi/6.0,
		YsPi/12.0,
		YsPi/12.0,
	};

	switch(loopMode)
	{
	case 0:
	case 1:
	case 2:
		if(fomPosition==1)
		{
			ApplyControlLoop(air,sim,dt);
		}
		else
		{
			FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 3:
		if(fomPosition==1)
		{
			ControlGForVerticalSpeed(air,sim,0.0);
			air.Prop().BankController(0.0);
			air.Prop().SetThrottle(0.9);
		}
		else
		{
			FsFormation::ApplyControl(air,sim,dt);
		}
		break;
	case 4:
		air.Prop().BankController(breakBank[fomPosition-1]);
		if(fomPosition!=6 && fomPosition!=7)
		{
			air.Prop().PitchController(breakPitch[fomPosition-1]);
		}
		else
		{
			ControlGForVerticalSpeed(air,sim,0.0,5.0);
		}
		air.Prop().SetThrottle(1.0);
		break;
	case 5:
		air.Prop().BankController(0.0);
		if(fomPosition!=6 && fomPosition!=7)
		{
			air.Prop().PitchController(breakPitch[fomPosition-1]);
		}
		else
		{
			ControlGForVerticalSpeed(air,sim,0.0,5.0);
		}
		air.Prop().SetThrottle(1.0);
		break;
	}
	return YSOK;
}

YSRESULT FsAirshowControl::MakeDecisionBigBattleToShortDiamondLoop(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	// const YsVec3 &pos=air.GetPosition();
	const YsAtt3 &att=air.GetAttitude();

	if(fomPosition==1)
	{
		MakeDecisionLoop(air,sim,dt);

		if(fabs(att.b())>YsPi/2.0)
		{
			YsVec3 pos;
			FsAirshowControl *ap;
			double r;

			r=air.GetRadiusFromCollision()*0.85;

			if(att.p()<YsDegToRad(85.0))
			{
				ap=GetAirshowAP(formation[3]); // #4
				if(ap!=NULL && ap->loopMode<2)
				{
					ap->loopMode=2;
					GetDiamond9Position(pos,4,r);
					ap->nTransition=2;
					ap->transition[0]=pos;
					ap->transition[0].SubY(2.0);
					ap->transition[1]=pos;
				}
				ap=GetAirshowAP(formation[6]); // #7
				if(ap!=NULL && ap->loopMode<2)
				{
					ap->loopMode=2;
					GetDiamond9Position(pos,7,r);
					ap->nTransition=2;
					ap->transition[0]=pos;
					ap->transition[0].SubY(2.0);
					ap->transition[1]=pos;
				}
			}
			if(att.p()<YsDegToRad(70.0))
			{
				ap=GetAirshowAP(formation[5]); // #6
				if(ap!=NULL && ap->loopMode<2)
				{
					ap->loopMode=2;
					GetDiamond9Position(pos,6,r);
					ap->shouldBe=pos;
				}
				ap=GetAirshowAP(formation[7]); // #8
				if(ap!=NULL && ap->loopMode<2)
				{
					ap->loopMode=2;
					GetDiamond9Position(pos,8,r);
					// ap->shouldBe=pos;
					// ap->shouldBe.SubY(2.0);
					ap->shouldBe.SetX(pos.x());
					ap->shouldBe.SetY(pos.y()-2.0);
					ap->nTransition=1;
					ap->transition[0]=pos;
				}
			}
			if(att.p()<YsDegToRad(0.0))
			{
				ap=GetAirshowAP(formation[8]); // #9
				if(ap!=NULL && ap->loopMode<2)
				{
					ap->loopMode=2;
					GetDiamond9Position(pos,9,r);
					ap->nTransition=3;

					ap->transition[0]=pos;
					ap->transition[0].SetX(shouldBe.x()/2.0);
					ap->transition[0].SubY(4.0);

					ap->transition[1]=pos;
					ap->transition[1].SetX(shouldBe.x()/4.0);
					ap->transition[1].SubY(4.0);

					ap->transition[2]=pos;
				}
			}
		}
		return YSOK;
	}
	else
	{
		return FsFormation::MakeDecision(air,sim,dt);
	}
}

YSRESULT FsAirshowControl::ApplyControlBigBattleToShortDiamondLoop(FsAirplane &air,FsSimulation *sim,const double &dt)
{
	air.Prop().SetSmokeButton(YSTRUE);
	if(fomPosition==1)
	{
		return ApplyControlLoop(air,sim,dt);
	}
	else
	{
		return FsFormation::ApplyControl(air,sim,dt);
	}
}

