#include "fsvehicleproperty.h"

#include "fsutil.h"
#include "fsnetutil.h"
#include "fsweather.h"
#include "fsnavaid.h"
#include "fsrecord.h"
#include "fsvisual.h"
#include "fsweapon.h"
#include "fsproperty.h"
#include "fsairplaneproperty.h"
#include "fsgroundproperty.h"
#include "fsnetwork.h"
#include "fsexistence.h"
#include "fsstdout.h"
#include "fscontrol.h"

FsVehicleProperty::FsVehicleProperty()
{
	InitializeState();
	InitializeCharacteristic();
}

FsVehicleProperty::~FsVehicleProperty()  // Protected & Non-Virtual: Not allowing deletion through base-class pointer.  See "C++ Codint Standard" pp. 90
{
}

void FsVehicleProperty::InitializeState(void)
{
	staPosition.Set(0.0,0.0,0.0);
	staAttitude.Set(0.0,0.0,0.0);
	staMatrix.Initialize();
	staInverseMatrix.Initialize();

	for(int i=0; i<staTurret.GetN(); i++)
	{
		staTurret[i].Initialize();
	}
	staSavedTurret.ClearDeep();
	staGunnerFirePermission=YSTRUE;  // Should be true by default.  Otherwise, ground object will not shoot at you!

	staDamageTolerance=10;

	staOnThisCarrier=NULL;
}

void FsVehicleProperty::InitializeCharacteristic(void)
{
	chIdName.Set("");
	chSubstIdName.Set("");
	chOutsideRadius=10.0;

	chCockpit.Set(0.0,2.0,5.0);
	chLookAt=YsOrigin();
	chDefZoom=1.0;
	chExtraView.ClearDeep();

	chTurret.Set(0,NULL);
	chHasAntiAirTurret=YSFALSE;
	chHasAntiGndTurret=YSFALSE;
	chHasPilotControlledTurret=YSFALSE;
	chHasGunnerControlledTurret=YSFALSE;

	chIsRacingCheckPoint=YSFALSE;
}



const char *FsVehicleProperty::GetIdentifier(void) const
{
	return chIdName;
}

const char *FsVehicleProperty::GetSubstIdName(void) const
{
	return chSubstIdName;
}

const double &FsVehicleProperty::GetOutsideRadius(void) const
{
	return chOutsideRadius;
}

const YsVec3 &FsVehicleProperty::GetCockpitPosition(YsVec3 &cock) const
{
	cock=chCockpit;
	return cock;
}

const YsVec3 &FsVehicleProperty::GetCockpitPosition(void) const
{
	return chCockpit;
}

const YsVec3 &FsVehicleProperty::GetLookAtOffset(void) const
{
	return chLookAt;
}

const double FsVehicleProperty::GetDefaultZoom(void) const
{
	return chDefZoom;
}

int FsVehicleProperty::GetNumAdditionalView(void) const
{
	return (int)chExtraView.GetN();
}

const FsAdditionalViewpoint *FsVehicleProperty::GetAdditionalView(int id) const
{
	if(YSTRUE==chExtraView.IsInRange(id))
	{
		return &chExtraView[id];
	}
	return NULL;
}

YSBOOL FsVehicleProperty::IsRacingCheckPoint(void) const
{
	return chIsRacingCheckPoint;
}
void FsVehicleProperty::SetIsRacingCheckPoint(YSBOOL flg)
{
	chIsRacingCheckPoint=flg;
}


const YsVec3 &FsVehicleProperty::GetPosition(void) const
{
	return staPosition;
}

const YsAtt3 &FsVehicleProperty::GetAttitude(void) const
{
	return staAttitude;
}

YsVec3 &FsVehicleProperty::GetPosition(YsVec3 &vec) const
{
	vec=staPosition;
	return vec;
}

YsAtt3 &FsVehicleProperty::GetAttitude(YsAtt3 &att) const
{
	att=staAttitude;
	return att;
}

const YsMatrix4x4 &FsVehicleProperty::GetMatrix(void) const
{
	return staMatrix;
}

const YsMatrix4x4 &FsVehicleProperty::GetInverseMatrix(void) const
{
	return staInverseMatrix;
}

void FsVehicleProperty::RemakeMatrix(void)
{
	staMatrix.Initialize();
	staMatrix.Translate(staPosition);
	staMatrix.Rotate(staAttitude);

	staInverseMatrix=staMatrix;
	staInverseMatrix.Invert();
}

void FsVehicleProperty::MoveTurretGunner(
    const double &dt,
    FSIFF iff,
    YSSIZE_T nAirTgt,const FsAirplane * const airTgt[],
    YSSIZE_T nGndTgt,const FsGround * const gndTgt[])
{
	for(int i=0; i<chTurret.GetN(); i++)
	{
		if(FSTURRET_CTRL_BY_GUNNER==chTurret[i].controlledBy)
		{
			staTurret[i].MoveGunner(
			    chTurret[i],dt,iff,nAirTgt,airTgt,nGndTgt,gndTgt,
			    staGunnerFirePermission,
			    staInverseMatrix);
		}
	}
}

const double FsVehicleProperty::GetMaxRotatinTurretRange(void) const
{
	double range=0.0;
	for(int i=0; i<chTurret.GetN(); i++)
	{
		range=YsGreater(range,chTurret[i].range);
	}
	return range;
}

void FsVehicleProperty::SetGunnerFirePermission(YSBOOL permission)
{
	staGunnerFirePermission=permission;
}

YSBOOL FsVehicleProperty::GetGunnerFirePermission(void) const
{
	return staGunnerFirePermission;
}

int FsVehicleProperty::GetNumPilotControlledTurretBullet(void) const
{
	int n=0;
	for(int i=0; i<staTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			n+=staTurret[i].numBullet;
		}
	}
	return n;
}

int FsVehicleProperty::GetMaxNumPilotControlledTurretBullet(void) const
{
	int n=0;
	for(int i=0; i<staTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			n+=chTurret[i].maxNumGunBullet;
		}
	}
	return n;
}

YSBOOL FsVehicleProperty::IsFiringPilotControlledTurret(void)
{
	int i;
	forYsArray(i,staTurret)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT &&
		   staTurret[i].numBullet>0 &&
		   (staTurret[i].turretState&FSTURRETSTATE_FIRING)!=0)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSRESULT FsVehicleProperty::SetPilotControlledTurretHeading(const double &a)
{
	int i;
	for(i=0; i<chTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			staTurret[i].ctlH=YsBound(a+chTurret[i].hZero,chTurret[i].hMin,chTurret[i].hMax);
		}
	}
	return YSOK;
}

YSRESULT FsVehicleProperty::SetPilotControlledTurretPitch(const double &a)
{
	int i;
	for(i=0; i<chTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			staTurret[i].ctlP=YsBound(a+chTurret[i].pZero,chTurret[i].pMin,chTurret[i].pMax);
		}
	}
	return YSOK;
}

YSBOOL FsVehicleProperty::TurretStateChanged(void) const
{
	if(staTurret.GetN()!=staSavedTurret.GetN())
	{
		return YSTRUE;
	}

	int i;
	forYsArray(i,staTurret)
	{
		if(staTurret[i].turretState!=staSavedTurret[i].turretState ||
		   YsEqual(staTurret[i].h,staSavedTurret[i].h)!=YSTRUE ||
		   YsEqual(staTurret[i].p,staSavedTurret[i].p)!=YSTRUE)
		{
			return YSTRUE;
		}
	}

	return YSFALSE;
}

void FsVehicleProperty::SaveTurretState(void) const
{
	if(staSavedTurret.GetN()!=staTurret.GetN())
	{
		staSavedTurret.Set(staTurret.GetN(),NULL);
	}
	int i;
	forYsArray(i,staTurret)
	{
		staSavedTurret[i].turretState=staTurret[i].turretState;
		staSavedTurret[i].h=staTurret[i].h;
		staSavedTurret[i].p=staTurret[i].p;
	}
}

unsigned int FsVehicleProperty::EncodeTurretState(unsigned char dat[],int idOnSvr,const int netCmd) const
{
	if(staTurret.GetN()>0)
	{
		unsigned char *ptr;
		ptr=dat;
		FsPushInt  (ptr,netCmd);
		FsPushInt  (ptr,idOnSvr);
		FsPushShort(ptr,0);   // Version
		FsPushShort(ptr,0);   // Padding
		
		int i;
		short h,p;
		for(i=0; i<staTurret.GetN() && (ptr-dat)<240; i++)
		{
			FsPushInt(ptr,staTurret[i].turretState);

			h=(short)(staTurret[i].h*32768.0/YsPi);
			p=(short)(staTurret[i].p*32768.0/YsPi);
			FsPushShort(ptr,h);
			FsPushShort(ptr,p);
		}

		return (unsigned int)(ptr-dat);
	}
	else
	{
		return 0;
	}
}

YSRESULT FsVehicleProperty::DecodeTurretState(unsigned char dat[],unsigned int packetLength)
{
	int i;
	short version,h,p;
	const unsigned char *ptr=dat;

	FsPopInt(ptr);  // Skipping FSNETCMD
	FsPopInt(ptr);  // Skipping idOnSvr
	version=FsPopShort(ptr);
	FsPopShort(ptr);
	if(version==0)
	{
		for(i=0; i<staTurret.GetN() && (unsigned int)(ptr-dat)<packetLength; i++)
		{
			if(i==staTurret.GetN())
			{
				return YSERR;
			}

			staTurret[i].turretState=FsPopInt(ptr);
			h=FsPopShort(ptr);
			p=FsPopShort(ptr);
			staTurret[i].h=(double)h*YsPi/32768.0;
			staTurret[i].p=(double)p*YsPi/32768.0;
			staTurret[i].ctlH=staTurret[i].h;
			staTurret[i].ctlP=staTurret[i].p;
		}
	}

	return YSOK;
}

int FsVehicleProperty::GetDamageTolerance(void) const
{
	return staDamageTolerance;
}

void FsVehicleProperty::SetDamageTolerance(int tol)
{
	staDamageTolerance=tol;
}

YSBOOL FsVehicleProperty::IsOnCarrier(void) const
{
	return (staOnThisCarrier!=NULL ? YSTRUE : YSFALSE);
}

FsGround *FsVehicleProperty::OnThisCarrier(void) const
{
	return staOnThisCarrier;
}
