#include "fsautodrive.h"
#include "fsdef.h"
#include "fssimulation.h"
#include "fsexistence.h"
#include "fsgroundproperty.h"
#include "fsutil.h"

FsAutoDrive::FsAutoDrive()
{
}

FsAutoDrive::~FsAutoDrive()
{
}

void FsAutoDrive::SetUp(class FsSimulation *sim,class FsGround *gnd)
{
	if(NULL!=sim && NULL!=gnd)
	{
		this->startTime=sim->currentTime;
		this->lastControlTime=sim->currentTime;
	}
}

YSBOOL FsAutoDrive::CheckCollisionPotentialAir(class FsSimulation *sim,class FsGround *gnd,int nExclude,const FsExistence *const exclude[]) const
{
	const double v=gnd->Prop().GetVelocity();
	const double a=gnd->Prop().CalculateBrakeAcceleration(1.0);
	const double t=v/a;

	YsVec3 vel;
	gnd->Prop().GetVelocity(vel);

	const YsVec3 futurePos=gnd->GetPosition()+vel*t;

	return sim->MayCollideWithAir(futurePos,gnd->GetAttitude(),2.0,gnd,nExclude,exclude);
}

YSBOOL FsAutoDrive::CheckUnreachable(class FsSimulation *,const class FsGround *gnd,const YsVec2 &goalPos) const
{
	const YsVec2 currentPos(gnd->GetPosition().x(),gnd->GetPosition().z());
	const double turnRate=gnd->Prop().CalculateCurrentTurnRate(1.0);
	const double currentVel=gnd->Prop().GetVelocity();
	const double turnRad=currentVel/turnRate; // v=rw  -> r=v/w

	const YsVec2 currentVec(-sin(gnd->GetAttitude().h()),cos(gnd->GetAttitude().h()));
	const YsVec2 rightVec(currentVec.y(),-currentVec.x());

	const YsVec2 cen[2]=
	{
		currentPos+rightVec*turnRad,
		currentPos-rightVec*turnRad,
	};

	// If the goal is within the turn radius
	if((goalPos-cen[0]).GetSquareLength()<turnRad*turnRad ||
	   (goalPos-cen[1]).GetSquareLength()<turnRad*turnRad)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsAutoDrive::Save(class FsSimulation *sim,class FsGround *gnd,FILE *fp)
{
	YsTextFileOutputStream outStream(fp);
	return Save(sim,gnd,outStream);
}

FsAutoDrive *FsAutoDrive::Load(class FsSimulation *sim,FILE *fp)
{
	YsTextFileInputStream inStream(fp);
	return Load(sim,inStream);
}

const char *const FsAutoDrive::keyWordSource[]=
{
	"BGNATDRV",
	"ENDATDRV",
	"STRTTIME",
	"DRVTOOBJ",
	"GOALOBJE",
	"DONTSTOP",

	"DRVEXACT",
	"GOALPOSI",
	"GOALHDG_",

	NULL
};

YsKeyWordList FsAutoDrive::keyWordList;

FsAutoDrive *FsAutoDrive::Load(class FsSimulation *,YsTextInputStream &inStream)
{
	if(keyWordList.GetN()==0)
	{
		keyWordList.MakeList(keyWordSource);
	}

	YsString str;
	YsArray <YsString,16> args;

	FsAutoDrive *autoDrive=NULL;
	FsAutoDriveToObject *autoDriveToObj=NULL;
	FsAutoDriveToExactPosition *autoDriveToExact=NULL;

	while(NULL!=inStream.Gets(str))
	{
		str.Arguments(args);

		int cmd;

		if(args[0][0]=='*')
		{
			cmd=atoi(((const char *)(args[0]))+1);
		}
		else
		{
			cmd=keyWordList.GetId(args[0]);
		}

		switch(cmd)
		{
		case 0: // "BGNATDRV"  Just a header.  Ignore it.  It should be eaten by bigger block though.
			break;
		case 1: // "ENDATDRV\n",
			return autoDrive;
		case 2: // "STRTTIME"
			if(2<=args.GetN() && NULL!=autoDrive)
			{
				autoDrive->startTime=atof(args[1]);
			}
			break;
		case 3: // "DRVTOOBJ\n",
			if(NULL!=autoDriveToObj)
			{
				delete autoDriveToObj;
			}
			autoDriveToObj=new FsAutoDriveToObject;
			autoDrive=autoDriveToObj;
			break;
		case 4: // "GOALOBJE\n",
			if(2<=args.GetN() && NULL!=autoDriveToObj)
			{
				autoDriveToObj->goalObjKey=atoi(args[1]); // Tentative assignment.  Will be re-matched with ysfId.
			}
			break;
		case 5: // DONTSTOP
			if(2<=args.GetN() && NULL!=autoDriveToObj)
			{
				FsGetBool(autoDriveToObj->dontStop,args[1]);
			}
			break;

		case 6: // "DRVEXACT",
			if(NULL!=autoDriveToExact)
			{
				delete autoDriveToExact;
			}
			autoDriveToExact=new FsAutoDriveToExactPosition;
			autoDrive=autoDriveToExact;
			break;
		case 7: // "GOALPOSI",
			if(NULL!=autoDriveToExact)
			{
				FsGetVec3(autoDriveToExact->goalPos,3,args.GetArray()+1);
			}
			break;
		case 8: // "GOALHDG_",
			if(NULL!=autoDriveToExact)
			{
				FsGetAngle(autoDriveToExact->goalHdg,args[1]);
			}
			break;
		}
	}

	return autoDrive;
}

void FsAutoDrive::DriveToDestination(
    double &steering,double &desiredSpeed,
    class FsSimulation *,class FsGround *gnd,const double gndRad,const YsVec3 &goalPos,const double goalRad)
{
	YsVec3 rel=(goalPos-gnd->GetPosition());
	gnd->GetAttitude().MulInverse(rel,rel);

	const double relAng=atan2(-rel.x(),rel.z());
	const double currentSpeed=gnd->Prop().GetVelocity();

	if(gnd->Prop().GetMaxStableSpeed()>currentSpeed)
	{
		steering=YsBound(relAng,-1.0,1.0);
	}
	else
	{
		steering=0.0;
	}

	const double angleDeviation=fabs(relAng);
	if(angleDeviation>YsPi/180.0 && gnd->Prop().GetMaxStableSpeed()<currentSpeed)
	{
		desiredSpeed=0.0;
	}
	else if(YsPi<angleDeviation)
	{
		desiredSpeed=gnd->Prop().GetMinimumManeuvableSpeed()*0.9+gnd->Prop().GetMaxStableSpeed()*0.1;
	}
	else
	{
		const double maxSpeed=(angleDeviation<=YsPi/181.0 ? gnd->Prop().GetMaxSpeed() : gnd->Prop().GetMaxStableSpeed()*0.95);

		const double brakeAcceleration=gnd->Prop().CalculateBrakeAcceleration(1.0);
		const double timeToStop=currentSpeed/brakeAcceleration;
		const double distToStop=currentSpeed*timeToStop-0.5*brakeAcceleration*YsSqr(timeToStop);

		const double currentDist=(gnd->GetPosition()-goalPos).GetLength();
		const double desiredDist=gndRad+goalRad;

		if(currentDist-desiredDist<distToStop)
		{
			desiredSpeed=0.0;
		}
		else
		{
			desiredSpeed=maxSpeed;
		}
	}
}

////////////////////////////////////////////////////////////

void FsAutoDrive::ThreePointTurn::Initialize(const double turnDir,const YsVec3 &initVec)
{
	this->threePointPhase=THREE_POINT_BACKWARD_FORWARD_PHASE1;
	this->turnDir=turnDir;
	this->initVec=initVec;
}

YSBOOL FsAutoDrive::ThreePointTurn::Accomplished(void) const
{
	return (threePointPhase==THREE_POINT_DONE ? YSTRUE : YSFALSE);
}

void FsAutoDrive::ThreePointTurn::Start(const FsGround *gnd,const double turnDir)
{
	const YsVec3 iniVec=gnd->GetAttitude().GetForwardVector();
	Initialize(turnDir,iniVec);
}

void FsAutoDrive::ThreePointTurn::Drive(FsGround *gnd)
{
	switch(threePointPhase)
	{
	case THREE_POINT_BACKWARD_FORWARD_PHASE1:
		{
			YsVec3 turnToVec=initVec;
			turnToVec.RotateXZ(turnDir*YsPi/2.0);
			turnToVec.SetY(0.0);
			turnToVec.Normalize();

			turnToVec.RotateXZ(-gnd->GetAttitude().h());

			const double angleDiff=atan2(-turnToVec.x(),turnToVec.z());
			const double steering=YsBound(-angleDiff,-1.0,1.0);

			gnd->Prop().SetSteering(steering);
			if(YsPi/180.0>fabs(angleDiff))
			{
				gnd->Prop().SetReverse(1.0);
				gnd->Prop().SetDesiredSpeed(0.0);
				gnd->Prop().SetBrake(1.0);
				if(YsTolerance>gnd->Prop().GetVelocity())
				{
					threePointPhase=THREE_POINT_BACKWARD_FORWARD_PHASE2;
				}
			}
			else
			{
				gnd->Prop().SetReverse(1.0);
				gnd->Prop().SetDesiredSpeed(-(gnd->Prop().GetMinimumManeuvableSpeed()*0.7+gnd->Prop().GetMaxStableSpeed()*0.3));
				gnd->Prop().SetBrake(0.0);
			}
		}
		break;
	case THREE_POINT_BACKWARD_FORWARD_PHASE2:
		{
			YsVec3 turnToVec=-initVec;

			turnToVec.RotateXZ(-gnd->GetAttitude().h());

			const double angleDiff=atan2(-turnToVec.x(),turnToVec.z());
			const double steering=YsBound(angleDiff,-1.0,1.0);

			gnd->Prop().SetSteering(steering);
			if(YsPi/180.0>fabs(angleDiff))
			{
				gnd->Prop().SetReverse(0.0);
				gnd->Prop().SetDesiredSpeed(0.0);
				gnd->Prop().SetBrake(1.0);
				if(YsTolerance>gnd->Prop().GetVelocity())
				{
					threePointPhase=THREE_POINT_DONE;
				}
			}
			else
			{
				gnd->Prop().SetReverse(0.0);
				gnd->Prop().SetDesiredSpeed((gnd->Prop().GetMinimumManeuvableSpeed()*0.7+gnd->Prop().GetMaxStableSpeed()*0.3));
				gnd->Prop().SetBrake(0.0);
			}
		}
		break;
	case THREE_POINT_DONE:
		break;
	}
}

////////////////////////////////////////////////////////////

void FsAutoDrive::Align::Start(const class FsGround *gnd,const double desiredSpeed,const YsVec3 &goalPos,const double goalHdg)
{
	this->desiredSpeed=desiredSpeed;
	this->goalPos.Set(goalPos.x(),goalPos.z());
	this->goalVec.Set(-sin(goalHdg),cos(goalHdg));

	this->alignReverse=YSFALSE;
	this->alignAndGoToGoal=YSFALSE;

	const YsVec2 currentPos(gnd->GetPosition().x(),gnd->GetPosition().z());
	const YsVec2 currentVec(-sin(gnd->GetAttitude().h()),cos(gnd->GetAttitude().h()));

	YsVec2 itsc;
	if(YSOK==YsGetLineIntersection2(itsc,currentPos,currentPos+currentVec,this->goalPos,this->goalPos+this->goalVec))
	{
		if(0.0>(itsc-currentPos)*currentVec)
		{
			this->alignReverse=YSTRUE;
		}
	}
}

void FsAutoDrive::Align::Drive(FsGround *gnd)
{
	double desiredSpeed=this->desiredSpeed;
	const double v=gnd->Prop().GetVelocity();

	const YsVec2 currentPos(gnd->GetPosition().x(),gnd->GetPosition().z());
	const YsVec2 currentVec(-sin(gnd->GetAttitude().h()),cos(gnd->GetAttitude().h()));

	const YsVec2 rightVec(goalVec.y(),-goalVec.x());
	const double dLateral=(currentPos-goalPos)*rightVec;

	double dAngle=0.0; // Angle from current to goal
	if(0.0<(currentVec^goalVec)) // Goal vec is to the left
	{
		dAngle=acos(currentVec*goalVec);
	}
	else
	{
		dAngle=-acos(currentVec*goalVec);
	}


	if(YSTRUE==alignAndGoToGoal)
	{
		if(0.0>(goalPos-currentPos)*currentVec)
		{
			alignReverse=YSTRUE;
		}
		else
		{
			alignReverse=YSFALSE;
		}
	}


	const double angleThrLateralKickIn=YsPi/4.0;
	const double kA=(alignReverse!=YSTRUE ? 1.0 : -1.0);
	const double kD=YsPi/18.0; // 1m off -> 10 degree.

	double steering=(fabs(dAngle)<angleThrLateralKickIn ? kA*dAngle+kD*dLateral : kA*dAngle);
	steering=YsBound(steering,-1.0,1.0);

	lastLateraldeviation=dLateral;
	lastAngleDeviation=dAngle;


	if(YSTRUE==alignAndGoToGoal && 0.0<currentVec*(goalPos-currentPos))
	{
		const double a=gnd->Prop().CalculateBrakeAcceleration(1.0);
		const double t=v/a;
		const double distToStop=v*t-0.5*a*t*t;

		const double d=(goalPos-currentPos).GetLength();
		if(d<distToStop)
		{
			desiredSpeed=0.0;
		}
	}


	if(YsTolerance<desiredSpeed)
	{
		if(YSTRUE==this->alignReverse)
		{
			gnd->Prop().SetReverse(1.0);
			gnd->Prop().SetDesiredSpeed(-desiredSpeed);
			gnd->Prop().SetBrake(0.0);
		}
		else
		{
			gnd->Prop().SetReverse(0.0);
			gnd->Prop().SetDesiredSpeed(desiredSpeed);
			gnd->Prop().SetBrake(0.0);
		}
	}
	else
	{
		gnd->Prop().SetReverse(0.0);
		gnd->Prop().SetDesiredSpeed(0.0);
		gnd->Prop().SetBrake(1.0);
	}

	gnd->Prop().SetSteering(steering);
}

////////////////////////////////////////////////////////////

FsAutoDriveToObject::FsAutoDriveToObject()
{
	dontStop=YSFALSE;
	goalObjKey=-1;
}

FsAutoDrive::AUTODRIVETYPE FsAutoDriveToObject::GetType(void) const
{
	return AUTODRIVE_TOOBJECT;
}

void FsAutoDriveToObject::SetUp(class FsSimulation *sim,class FsGround *gnd,class FsExistence *goal)
{
	FsAutoDrive::SetUp(sim,gnd);
	if(NULL!=goal)
	{
		goalObjKey=goal->SearchKey();
	}
	else
	{
		goalObjKey=-1;
	}
	this->holdUntil.Initialize();
}

int FsAutoDriveToObject::GetGoalObjKey(void) const
{
	return goalObjKey;
}

YSRESULT FsAutoDriveToObject::Save(class FsSimulation *sim,class FsGround *,YsTextOutputStream &outStream)
{
	const FsExistence *goal=sim->FindObject(goalObjKey);
	if(NULL!=goal)
	{
		outStream.Printf("BGNATDRV\n");

		outStream.Printf("DRVTOOBJ\n");
		outStream.Printf("STRTTIME %.2lf\n",startTime);
		outStream.Printf("GOALOBJE %d\n",goal->ysfId);
		outStream.Printf("DONTSTOP %s\n",FsTrueFalseString(dontStop));

		outStream.Printf("ENDATDRV\n");
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsAutoDriveToObject::ReconnectObjKeyAfterLoading(class FsSimulation *,const YsHashTable <int> &ysfIdToObjKey)
{
	const int goalObjKeyBuffer=goalObjKey;
	return ysfIdToObjKey.FindElement(goalObjKey,goalObjKeyBuffer);
}

void FsAutoDriveToObject::Control(FsSimulation *sim,class FsGround *gnd)
{
	if(YSTRUE==holdUntil.NeedToHold(sim,gnd))
	{
		gnd->Prop().SetReverse(0.0);
		gnd->Prop().SetDesiredSpeed(0.0);
		gnd->Prop().SetBrake(1.0);
		return;
	}


	const FsExistence *goal=sim->FindObject(goalObjKey);

	const int nExclude=(NULL!=goal ? 1 : 0);
	const FsExistence *const exclude[1]={goal};
	if(YSTRUE==CheckCollisionPotentialAir(sim,gnd,nExclude,exclude))
	{
		// Stop until the collision situation is resolved.
		gnd->Prop().SetReverse(0.0);
		gnd->Prop().SetDesiredSpeed(0.0);
		gnd->Prop().SetBrake(1.0);
		return;
	}

	if(NULL!=goal)
	{
		double steering,desiredSpeed;
		DriveToDestination(
		    steering,desiredSpeed,
		    sim,gnd,gnd->GetApproximatedCollideRadius(),goal->GetPosition(),goal->GetApproximatedCollideRadius());

		gnd->Prop().SetSteering(steering);

		if(YsTolerance<desiredSpeed)
		{
			gnd->Prop().SetReverse(0.0);
			gnd->Prop().SetDesiredSpeed(desiredSpeed);
			gnd->Prop().SetBrake(0.0);
		}
		else
		{
			gnd->Prop().SetReverse(0.0);
			gnd->Prop().SetDesiredSpeed(0.0);
			gnd->Prop().SetBrake(1.0);
		}
	}
}

YSBOOL FsAutoDriveToObject::ObjectiveAccomplished(FsSimulation *sim,FsGround *gnd) const
{
	if(YSTRUE==dontStop)
	{
		return YSFALSE;  // Chase the object all the time.
	}

	if(NULL!=gnd && YsTolerance>gnd->Prop().GetVelocity())
	{
		const FsExistence *goal=sim->FindObject(goalObjKey);
		if(NULL!=goal)
		{
			const double r1=gnd->GetApproximatedCollideRadius();
			const double r2=goal->GetApproximatedCollideRadius();
			const double d=(goal->GetPosition()-gnd->GetPosition()).GetLength();

			if((r1+r2)*1.5>d)
			{
				return YSTRUE;
			}
		}
	}

	return YSFALSE;
}

////////////////////////////////////////////////////////////

FsAutoDriveToExactPosition::FsAutoDriveToExactPosition()
{
	turnDir=1.0;
	temporarilyExcludedFromCollisionTestKey=-1;
}

FsAutoDrive::AUTODRIVETYPE FsAutoDriveToExactPosition::GetType(void) const
{
	return AUTODRIVE_TOEXACTPOSITION;
}

void FsAutoDriveToExactPosition::SetUp(class FsSimulation *sim,class FsGround *gnd,const YsVec3 &pos,const double hdg)
{
	FsAutoDrive::SetUp(sim,gnd);

	this->phase=PHASE_INITIAL;
	this->goalPos=pos;
	this->goalHdg=hdg;

	const double currentHdg=gnd->GetAttitude().h();
	this->prevGndDir.Set(-sin(currentHdg),cos(currentHdg));
	this->totalUnreachableTurn=0.0;

	this->waitingForStop=YSTRUE;

	this->holdUntil.Initialize();
}

void FsAutoDriveToExactPosition::StartWithThreePointTurn(class FsSimulation *,class FsGround *gnd)
{
	this->phase=PHASE_THREE_POINT_TURN;
	threePointTurn.Start(gnd,1.0);
}

void FsAutoDriveToExactPosition::ExcludeFromCollisionTest(const FsExistence *objPtr)
{
	if(NULL!=objPtr)
	{
		temporarilyExcludedFromCollisionTestKey=objPtr->SearchKey();
	}
	else
	{
		temporarilyExcludedFromCollisionTestKey=-1;
	}
}

YSRESULT FsAutoDriveToExactPosition::Save(class FsSimulation *,class FsGround *,YsTextOutputStream &outStream)
{
	outStream.Printf("BGNATDRV\n");

	outStream.Printf("DRVEXACT\n");
	outStream.Printf("GOALPOSI %.2lfm %.2lfm %.2lfm\n",goalPos.x(),goalPos.y(),goalPos.z());
	outStream.Printf("GOALHDG_ %.2deg\n",YsRadToDeg(goalHdg));

	outStream.Printf("ENDATDRV\n");

	return YSOK;
}

YSRESULT FsAutoDriveToExactPosition::ReconnectObjKeyAfterLoading(class FsSimulation *,const YsHashTable <int> &)
{
	return YSOK;
}

void FsAutoDriveToExactPosition::Control(class FsSimulation *sim,class FsGround *gnd)
{
	if(YSTRUE==holdUntil.NeedToHold(sim,gnd))
	{
		gnd->Prop().SetReverse(0.0);
		gnd->Prop().SetDesiredSpeed(0.0);
		gnd->Prop().SetBrake(1.0);
		return;
	}

	// Check for stop before check for collision.
	// Collision exclusion mustn't be cleared until it comes to the complete stop.
	if(YSTRUE==waitingForStop && YsTolerance<gnd->Prop().GetVelocity())
	{
		gnd->Prop().SetReverse(0.0);
		gnd->Prop().SetDesiredSpeed(0.0);
		gnd->Prop().SetBrake(1.0);
		return;
	}
	waitingForStop=YSFALSE;


	const FsExistence *const excludeFromCollisionTest[1]={sim->FindObject(temporarilyExcludedFromCollisionTestKey)};
	const int nExcludeFromCollision=(NULL!=excludeFromCollisionTest[0] ? 1 : 0);

	if(YSTRUE==CheckCollisionPotentialAir(sim,gnd,nExcludeFromCollision,excludeFromCollisionTest))
	{
		gnd->Prop().SetReverse(0.0);
		gnd->Prop().SetDesiredSpeed(0.0);
		gnd->Prop().SetBrake(1.0);
		return;
	}

	if(NULL!=excludeFromCollisionTest[0])
	{
		const double radSum=gnd->GetApproximatedCollideRadius()+excludeFromCollisionTest[0]->GetApproximatedCollideRadius();
		const double d=(gnd->GetPosition()-excludeFromCollisionTest[0]->GetPosition()).GetSquareLength();
		if(d>radSum*radSum)
		{
			temporarilyExcludedFromCollisionTestKey=-1;
			printf("Cleared of the excluded object.\n");
		}
	}


	const double finalSpeed=gnd->Prop().GetMinimumManeuvableSpeed()*0.7+gnd->Prop().GetMaxStableSpeed()*0.3;
	const double finalTurnRate=fabs(gnd->Prop().CalculateTurnRate(1.0,finalSpeed));
	const double finalTurnRad=(YsTolerance<finalTurnRate ? finalSpeed/finalTurnRate : 1.0);

	const double finalGuidanceLimit=finalTurnRad*3.0; // 1.5 x diameter


	const double currentHdg=gnd->GetAttitude().h();
	const YsVec2 currentGndDir(-sin(currentHdg),cos(currentHdg));
	const double turnFromPrevious=acos(this->prevGndDir*currentGndDir);
	this->prevGndDir=currentGndDir;


	switch(phase)
	{
	default:
		break;
	case PHASE_INITIAL:
		phase=PHASE_APPROACH;
		break;
	case PHASE_THREE_POINT_TURN:
		threePointTurn.Drive(gnd);
		if(YSTRUE==threePointTurn.Accomplished())
		{
			phase=PHASE_INITIAL;
		}
		break;
	case PHASE_APPROACH:
		{
			double steering,desiredSpeed;
			DriveToDestination(steering,desiredSpeed,sim,gnd,0.0,goalPos,0.0);
			if((goalPos-gnd->GetPosition()).GetSquareLength()<finalGuidanceLimit && 
			    gnd->Prop().GetVelocity()<finalSpeed)
			{
				alignControl.Start(gnd,finalSpeed,goalPos,goalHdg);
				phase=PHASE_ALIGN;
			}
			gnd->Prop().SetSteering(steering);
			gnd->Prop().SetReverse(0.0);
			if(YsTolerance<desiredSpeed)
			{
				gnd->Prop().SetDesiredSpeed(desiredSpeed);
				gnd->Prop().SetBrake(0.0);
			}
			else
			{
				gnd->Prop().SetDesiredSpeed(0.0);
				gnd->Prop().SetBrake(1.0);
			}

			const YsVec2 goal2d(goalPos.x(),goalPos.z());
			if(YSTRUE==CheckUnreachable(sim,gnd,goal2d))
			{
				totalUnreachableTurn+=turnFromPrevious;
				printf("Unreachable!! %lf deg\n",YsRadToDeg(totalUnreachableTurn));
				if(YsPi<totalUnreachableTurn)
				{
					alignControl.Start(gnd,finalSpeed,goalPos,goalHdg);
					phase=PHASE_ALIGN; // Desperate.  Break the loop.
				}
			}
			else
			{
				totalUnreachableTurn=0.0;
			}
		}
		break;
	case PHASE_ALIGN:
		{
			alignControl.Drive(gnd);
			if(1.0>fabs(alignControl.lastLateraldeviation) &&
			   YsPi/90.0>fabs(alignControl.lastAngleDeviation))
			{
				alignControl.alignAndGoToGoal=YSTRUE;
				phase=PHASE_FINALGUIDANCE;
			}
		}
	case PHASE_FINALGUIDANCE:
		{
			alignControl.Drive(gnd);
			if(2.0>gnd->Prop().GetVelocity())
			{
				YsVec3 diff=gnd->Prop().GetPosition()-goalPos;
				diff.SetY(0.0);
				if(diff.GetSquareLength()<4.0)
				{
					const double currentHdg=gnd->GetAttitude().h();

					const YsVec2 currentVec(-sin(currentHdg),cos(currentHdg));
					const YsVec2 goalVec(-sin(goalHdg),cos(goalHdg));

					if(YsCos3deg<currentVec*goalVec)
					{
						phase=PHASE_DONE;
					}
				}
			}
		}
		break;
	case PHASE_DONE:
		gnd->Prop().SetReverse(0.0);
		gnd->Prop().SetDesiredSpeed(0.0);
		gnd->Prop().SetBrake(1.0);
		break;
	}
}

YSBOOL FsAutoDriveToExactPosition::ObjectiveAccomplished(class FsSimulation *,class FsGround *gnd) const
{
	if(PHASE_DONE==phase && YsTolerance>gnd->Prop().GetVelocity())
	{
		printf("%s Done!\n",__FUNCTION__);
		return YSTRUE;
	}
	return YSFALSE;
}
