#include "fsaibasic.h"
#include "fssimulation.h"
#include "fsexistence.h"

FsHoldUntil::FsHoldUntil()
{
	Initialize();
}

void FsHoldUntil::Initialize(void)
{
	thisObjGoesAway=-1;
}

void FsHoldUntil::ClearHoldingCondition(void)
{
	Initialize();
}

void FsHoldUntil::SetHoldUntilThisObjectGoesAway(const FsExistence *obj)
{
	if(NULL!=obj)
	{
		thisObjGoesAway=obj->SearchKey();
	}
	else
	{
		thisObjGoesAway=-1;
	}
}

YSBOOL FsHoldUntil::NeedToHold(const class FsSimulation *sim,const class FsExistence *obj)
{
	if(0<=thisObjGoesAway)
	{
		const FsExistence *neiObj=sim->FindObject(thisObjGoesAway);
		if(NULL!=neiObj && YSTRUE==neiObj->IsAlive())
		{
			const double r1=obj->GetApproximatedCollideRadius();
			const double r2=neiObj->GetApproximatedCollideRadius();
			const double d=(obj->GetPosition()-neiObj->GetPosition()).GetLength();
			if((r1+r2)*2.0>d)
			{
				return YSTRUE;
			}
		}
	}


	// Doesn't have to hold.
	ClearHoldingCondition();
	return YSFALSE;
}

