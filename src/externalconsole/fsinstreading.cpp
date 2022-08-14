#include "fsinstreading.h"
#include "fsexternalconsolecommand.h"
#include "fsnetutil.h"

#include <stdlib.h>

void FsVORIndication::CleanUp(void)
{
	navId=0;
	vorId.Set("");
	flags=0;
	toFrom=0;
	obs=0.0;
	lateralDev=0.0;
	glideSlopeDev=0.0;
	dme=0.0;
}

void FsVORIndication::SetSelected(YSBOOL selected)
{
	if(YSTRUE==selected)
	{
		flags|=FLAG_SELECTED;
	}
	else
	{
		flags&=(~FLAG_SELECTED);
	}
}

YSBOOL FsVORIndication::IsSelected(void) const
{
	return (0!=(flags&FLAG_SELECTED) ? YSTRUE : YSFALSE);
}

void FsVORIndication::SetInop(YSBOOL inop)
{
	if(YSTRUE==inop)
	{
		flags|=FLAG_INOP;
	}
	else
	{
		flags&=(~FLAG_INOP);
	}
}

YSBOOL FsVORIndication::IsInop(void) const
{
	return (0!=(flags&FLAG_INOP) ? YSTRUE : YSFALSE);
}

void FsVORIndication::SetInRange(YSBOOL inRange)
{
	if(YSTRUE==inRange)
	{
		flags|=FLAG_INRANGE;
	}
	else
	{
		flags&=(~FLAG_INRANGE);
	}
}

YSBOOL FsVORIndication::IsInRange(void) const
{
	return (0!=(flags&FLAG_INRANGE) ? YSTRUE : YSFALSE);
}


void FsVORIndication::SetTuned(YSBOOL tuned)
{
	if(YSTRUE==tuned)
	{
		flags|=FLAG_TUNED;
	}
	else
	{
		flags&=(~FLAG_TUNED);
	}
}

YSBOOL FsVORIndication::IsTuned(void) const
{
	return (0!=(flags&FLAG_TUNED) ? YSTRUE : YSFALSE);
}


void FsVORIndication::SetIsILS(YSBOOL isILS)
{
	if(YSTRUE==isILS)
	{
		flags|=FLAG_ISILS;
	}
	else
	{
		flags&=(~FLAG_ISILS);
	}
}

YSBOOL FsVORIndication::IsILS(void) const
{
	return (0!=(flags&FLAG_ISILS) ? YSTRUE : YSFALSE);
}


void FsVORIndication::SetIsDME(YSBOOL isDME)
{
	if(YSTRUE==isDME)
	{
		flags|=FLAG_ISDME;
	}
	else
	{
		flags&=(~FLAG_ISDME);
	}
}

YSBOOL FsVORIndication::IsDME(void) const
{
	return (0!=(flags&FLAG_ISDME) ? YSTRUE : YSFALSE);
}

YSSIZE_T FsVORIndication::NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const
{
	unsigned char *ptr=buf;

	FsPushInt(ptr,FSEXCCMD_NAV_INDICATION);
	FsPushShort(ptr,0); //  Version
	FsPushShort(ptr,0); //  VOR/GPS mode.  Currently there is no mode.
	FsPushShort(ptr,(short)navId);
	FsPushShort(ptr,0); //  Padding

	for(int idx=0; idx<vorId.Strlen() && idx<16; ++idx)
	{
		(*ptr++)=(unsigned char)vorId[idx];
	}
	for(int idx=(int)vorId.Strlen(); idx<16; ++idx)
	{
		(*ptr++)=0;
	}

	FsPushInt(ptr,flags);
	FsPushShort(ptr,(short)toFrom);
	FsPushShort(ptr,0); // Padding
	FsPushFloat(ptr,(float)obs);
	FsPushFloat(ptr,(float)lateralDev);
	FsPushFloat(ptr,(float)glideSlopeDev);
	FsPushFloat(ptr,(float)dme);

	YSSIZE_T codeSize=ptr-buf;
	if(bufSize<codeSize)
	{
		printf("%s\n",__FUNCTION__);
		printf("Internal Error! Buffer Overflow!\n");
		exit(1);
	}

	return codeSize;
}

YSRESULT FsVORIndication::NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize)
{
	const unsigned char *ptr=buf;

	FsPopInt(ptr);   // Skipping FSEXCCMD_NAV_INDICATION
	const int version=FsPopShort(ptr);
	FsPopShort(ptr); // Skipping mode.
	navId=FsPopShort(ptr);
	FsPopShort(ptr); // Skipping padding.

	vorId="";
	for(int idx=0; idx<16; ++idx)
	{
		if(0!=(*ptr))
		{
			vorId.Append((char)*ptr);
		}
		++ptr;
	}

	flags=FsPopInt(ptr);
	toFrom=FsPopShort(ptr);
	FsPopShort(ptr); // Skipping padding
	obs=FsPopFloat(ptr);
	lateralDev=FsPopFloat(ptr);
	glideSlopeDev=FsPopFloat(ptr);
	dme=FsPopFloat(ptr);

	return YSOK;
}

////////////////////////////////////////////////////////////

void FsADFIndication::CleanUp(void)
{
	flags=0;
	adfId=0;
	ndbId.Set("");
	bearing=0.0;
}

void FsADFIndication::SetSelected(YSBOOL selected)
{
	if(YSTRUE==selected)
	{
		flags|=FLAG_SELECTED;
	}
	else
	{
		flags&=(~FLAG_SELECTED);
	}
}

YSBOOL FsADFIndication::IsSelected(void) const
{
	return (0!=(flags&FLAG_SELECTED) ? YSTRUE : YSFALSE);
}

void FsADFIndication::SetInop(YSBOOL inop)
{
	if(YSTRUE==inop)
	{
		flags|=FLAG_INOP;
	}
	else
	{
		flags&=(~FLAG_INOP);
	}
}

YSBOOL FsADFIndication::IsInop(void) const
{
	return (0!=(flags&FLAG_INOP) ? YSTRUE : YSFALSE);
}

void FsADFIndication::SetInRange(YSBOOL inRange)
{
	if(YSTRUE==inRange)
	{
		flags|=FLAG_INRANGE;
	}
	else
	{
		flags&=(~FLAG_INRANGE);
	}
}

YSBOOL FsADFIndication::IsInRange(void) const
{
	return (0!=(flags&FLAG_INRANGE) ? YSTRUE : YSFALSE);
}


void FsADFIndication::SetTuned(YSBOOL tuned)
{
	if(YSTRUE==tuned)
	{
		flags|=FLAG_TUNED;
	}
	else
	{
		flags&=(~FLAG_TUNED);
	}
}

YSBOOL FsADFIndication::IsTuned(void) const
{
	return (0!=(flags&FLAG_TUNED) ? YSTRUE : YSFALSE);
}

YSSIZE_T FsADFIndication::NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const
{
	unsigned char *ptr=buf;
	FsPushInt(ptr,FSEXCCMD_ADF_INDICATION);
	FsPushShort(ptr,0); // Version
	FsPushShort(ptr,(short)adfId);
	for(int idx=0; idx<ndbId.Strlen() && idx<16; ++idx)
	{
		(*ptr++)=ndbId[idx];
	}
	for(int idx=(int)ndbId.Strlen(); idx<16; ++idx)
	{
		(*ptr++)=0;
	}
	FsPushInt(ptr,flags);
	FsPushFloat(ptr,(float)bearing);

	YSSIZE_T codeSize=ptr-buf;
	if(bufSize<codeSize)
	{
		printf("%s\n",__FUNCTION__);
		printf("Internal Error! Buffer Overflow!\n");
		exit(1);
	}

	return codeSize;
}

YSRESULT FsADFIndication::NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize)
{
	const unsigned char *ptr=buf;
	FsPopInt(ptr); // Skipping FSEXCCMD_ADF_INDICATION
	FsPopShort(ptr); // Skipping Version
	adfId=FsPopShort(ptr);

	ndbId="";
	for(int idx=0; idx<16; ++idx)
	{
		if(0!=(*ptr))
		{
			ndbId.Append((char)*ptr);
		}
		++ptr;
	}
	flags=FsPopInt(ptr);
	bearing=FsPopFloat(ptr);

	return YSOK;
}

////////////////////////////////////////////////////////////

void FsRadarIndication::CleanUp(void)
{
	radarRange=0.0;
	maxRadarRange=0.0;
	echoArray.Clear();
}

void FsAmmunitionIndication::CleanUp(void)
{
	ammoArray.Clear();
}

YSSIZE_T FsAmmunitionIndication::NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const
{
	// 20 bytes per ammo + 8 bytes

	unsigned char *ptr=buf;
	FsPushInt(ptr,FSEXCCMD_AMMUNITION_INDICATION);
	FsPushShort(ptr,0); // Version
	FsPushShort(ptr,(short)ammoArray.GetN());

	for(YSSIZE_T idx=0; idx<ammoArray.GetN(); ++idx)
	{
		FsPushShort(ptr,(short)ammoArray[idx].wpnType);
		(*ptr++)=(unsigned char)ammoArray[idx].selected;
		(*ptr++)=0;  // Padding
		FsPushInt(ptr,ammoArray[idx].quantity);
		FsPushInt(ptr,ammoArray[idx].maxQuantity);
		FsPushInt(ptr,ammoArray[idx].level);
		FsPushInt(ptr,ammoArray[idx].standByTimer);
	}

	YSSIZE_T codeSize=ptr-buf;
	if(bufSize<codeSize)
	{
		printf("%s\n",__FUNCTION__);
		printf("Internal Error! Buffer Overflow!\n");
		exit(1);
	}

	return codeSize;
}

YSRESULT FsAmmunitionIndication::NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize)
{
	const unsigned char *ptr=buf;
	FsPopInt(ptr);   // Skipping FSEXCCMD_AMMUNITION_INDICATION);
	FsPopShort(ptr); // Skipping Version
	YSSIZE_T nAmmo=FsPopShort(ptr);

	ammoArray.Set(nAmmo,NULL);
	for(YSSIZE_T idx=0; idx<nAmmo; ++idx)
	{
		ammoArray[idx].wpnType=(FsAmmunitionIndication::WEAPONTYPE)FsPopShort(ptr);
		ammoArray[idx].selected=(YSBOOL)(*ptr++);
		ptr++;  // Padding
		ammoArray[idx].quantity=FsPopInt(ptr);
		ammoArray[idx].maxQuantity=FsPopInt(ptr);
		ammoArray[idx].level=FsPopInt(ptr);
		ammoArray[idx].standByTimer=FsPopInt(ptr);
	}

	return YSOK;
}

////////////////////////////////////////////////////////////

void FsInstrumentIndication::CleanUp(void)
{
	heading=0.0;
	headingBug=0.0;
	headingBugSelected=YSFALSE;	
	pitch=0.0;
	bank=0.0;
	turnRate=0.0;
	sideSlip=0.0;
	altitude=0.0;
	verticalSpeed=0.0;

	airSpeed=0.0;

	nEngine=1;
	for(int idx=0; idx<MAX_NUM_ENGINE; ++idx)
	{
		engineOutput[idx]=0.0;
	}

	nFuelTank=1;
	for(int idx=0; idx<MAX_NUM_FUELTANK; ++idx)
	{
		fuelRemain[idx]=0.0;
		fuelCapacity[idx]=0.0;
	}

	mach=0.0;
	gForce=0.0;
	hasVectorThrust=YSFALSE;
	nozzleDirection=YsOrigin();
	hasRetractableGear=YSFALSE;
	gearPos=0.0;
	brake=0.0;
	flaps=0.0;
	hasSpoiler=YSFALSE;
	spoiler=0.0;
	autoPilot=YSFALSE;

	elevator=0.0;
	elevatorTrim=0.0;
	aileron=0.0;
	rudder=0.0;

	velocity=YsOrigin();
}

YSSIZE_T FsInstrumentIndication::NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const
{
	unsigned char *ptr=buf;
	FsPushInt(ptr,FSEXCCMD_INSTRUMENT_INDICATION);
	FsPushShort(ptr,0);  // Version
	FsPushShort(ptr,(short)headingBugSelected);
	FsPushFloat(ptr,(float)heading);
	FsPushFloat(ptr,(float)headingBug);
	FsPushFloat(ptr,(float)pitch);
	FsPushFloat(ptr,(float)bank);
	FsPushFloat(ptr,(float)turnRate);
	FsPushFloat(ptr,(float)sideSlip);
	FsPushFloat(ptr,(float)altitude);
	FsPushFloat(ptr,(float)verticalSpeed);
	FsPushFloat(ptr,(float)airSpeed);
	FsPushFloat(ptr,(float)Vfe);
	FsPushFloat(ptr,(float)Vno);
	FsPushFloat(ptr,(float)Vne);
	FsPushFloat(ptr,(float)VindicatorRange);

	FsPushShort(ptr,(short)nEngine);
	FsPushShort(ptr,(short)nFuelTank);

	for(int engineIdx=0; engineIdx<MAX_NUM_ENGINE; ++engineIdx)
	{
		FsPushFloat(ptr,(float)engineOutput[engineIdx]);
	}
	for(int engineIdx=0; engineIdx<MAX_NUM_ENGINE; ++engineIdx)
	{
		(*ptr++)=(unsigned char)afterBurner[engineIdx];
	}
	for(int fuelTankIdx=0; fuelTankIdx<MAX_NUM_FUELTANK; ++fuelTankIdx)
	{
		FsPushFloat(ptr,(float)fuelRemain[fuelTankIdx]);
		FsPushFloat(ptr,(float)fuelCapacity[fuelTankIdx]);
	}
	FsPushFloat(ptr,(float)mach);
	FsPushFloat(ptr,(float)gForce);

	(*ptr++)=(unsigned char)hasVectorThrust;
	(*ptr++)=(unsigned char)hasRetractableGear;
	(*ptr++)=(unsigned char)hasSpoiler;
	(*ptr++)=(unsigned char)autoPilot;

	FsPushFloat(ptr,(float)nozzleDirection.x());
	FsPushFloat(ptr,(float)nozzleDirection.y());
	FsPushFloat(ptr,(float)nozzleDirection.z());

	FsPushFloat(ptr,(float)gearPos);
	FsPushFloat(ptr,(float)brake);
	FsPushFloat(ptr,(float)flaps);
	FsPushFloat(ptr,(float)spoiler);
	FsPushFloat(ptr,(float)elevator);
	FsPushFloat(ptr,(float)elevatorTrim);
	FsPushFloat(ptr,(float)aileron);
	FsPushFloat(ptr,(float)rudder);
	FsPushFloat(ptr,(float)velocity.x());
	FsPushFloat(ptr,(float)velocity.y());
	FsPushFloat(ptr,(float)velocity.z());

	const YSSIZE_T codeSize=ptr-buf;
	if(bufSize<codeSize)
	{
		printf("Internal error!  Buffer overflow!\n");
		exit(1);
	}

	return codeSize;
}

YSRESULT FsInstrumentIndication::NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize)
{
	const unsigned char *ptr=buf;

	FsPopInt(ptr);  // Skipping FSEXCCMD_INSTRUMENT_INDICATION

	const int version=FsPopShort(ptr);
	headingBugSelected=(YSBOOL)FsPopShort(ptr);
	heading=FsPopFloat(ptr);
	headingBug=FsPopFloat(ptr);
	pitch=FsPopFloat(ptr);
	bank=FsPopFloat(ptr);
	turnRate=FsPopFloat(ptr);
	sideSlip=FsPopFloat(ptr);
	altitude=FsPopFloat(ptr);
	verticalSpeed=FsPopFloat(ptr);
	airSpeed=FsPopFloat(ptr);
	Vfe=FsPopFloat(ptr);
	Vno=FsPopFloat(ptr);
	Vne=FsPopFloat(ptr);
	VindicatorRange=FsPopFloat(ptr);

	nEngine=FsPopShort(ptr);
	nFuelTank=FsPopShort(ptr);

	for(int engineIdx=0; engineIdx<MAX_NUM_ENGINE; ++engineIdx)
	{
		engineOutput[engineIdx]=FsPopFloat(ptr);
	}
	for(int engineIdx=0; engineIdx<MAX_NUM_ENGINE; ++engineIdx)
	{
		afterBurner[engineIdx]=(YSBOOL)(*ptr++);
	}
	for(int fuelTankIdx=0; fuelTankIdx<MAX_NUM_FUELTANK; ++fuelTankIdx)
	{
		fuelRemain[fuelTankIdx]=FsPopFloat(ptr);
		fuelCapacity[fuelTankIdx]=FsPopFloat(ptr);
	}
	mach=FsPopFloat(ptr);
	gForce=FsPopFloat(ptr);

	hasVectorThrust=(YSBOOL)(*ptr++);
	hasRetractableGear=(YSBOOL)(*ptr++);
	hasSpoiler=(YSBOOL)(*ptr++);
	autoPilot=(YSBOOL)(*ptr++);

	nozzleDirection.SetX(FsPopFloat(ptr));
	nozzleDirection.SetY(FsPopFloat(ptr));
	nozzleDirection.SetZ(FsPopFloat(ptr));

	gearPos=FsPopFloat(ptr);
	gearPos=FsPopFloat(ptr);
	flaps=FsPopFloat(ptr);
	spoiler=FsPopFloat(ptr);
	elevator=FsPopFloat(ptr);
	elevatorTrim=FsPopFloat(ptr);
	aileron=FsPopFloat(ptr);
	rudder=FsPopFloat(ptr);
	velocity.SetX(FsPopFloat(ptr));
	velocity.SetY(FsPopFloat(ptr));
	velocity.SetZ(FsPopFloat(ptr));
	// << Version 0

	return YSOK;
}

////////////////////////////////////////////////////////////

void FsCockpitIndicationSet::CleanUp(void)
{
	inst.CleanUp();

	for(int i=0; i<NUM_NAV; ++i)
	{
		nav[i].CleanUp();
	}
	for(int i=0; i<NUM_ADF; ++i)
	{
		adf[i].CleanUp();
	}
	
	radar.CleanUp();
	ammo.CleanUp();
}


////////////////////////////////////////////////////////////


YsString FsAmmunitionIndication::Ammunition::FormatString(void) const
{
	YsString intervalStr;
	YsString ammoStr;

	switch(wpnType)
	{
	default:
		break;
	case WPNTYPE_GUN:
		ammoStr.Printf("GUN:%d",quantity);
		break;
	case WPNTYPE_AIM9:
		if(YsTolerance<standByTimer)
		{
			intervalStr.Printf(" [%d]",standByTimer);
		}
		ammoStr.Printf("AAM:%d (Short-Range)",quantity);
		break;
	case WPNTYPE_AIM9X:
		if(YsTolerance<standByTimer)
		{
			intervalStr.Printf(" [%d]",standByTimer);
		}
		ammoStr.Printf("A-AAM:%d (Short-Range)",quantity);
		break;
	case WPNTYPE_AIM120:
		if(YsTolerance<standByTimer)
		{
			intervalStr.Printf("  [%d]",standByTimer);
		}
		ammoStr.Printf("AAM:%d (Mid-Range)",quantity);
		break;
	case WPNTYPE_AGM65:
		ammoStr.Printf("AGM:%d  ",quantity);
		break;
	case WPNTYPE_ROCKET:
		ammoStr.Printf("RKT:%d  ",quantity);
		break;
	case WPNTYPE_BOMB:
		ammoStr.Printf("BOM:%d (500lb)  ",quantity);
		break;
	case WPNTYPE_BOMB250:
		ammoStr.Printf("BOM:%d (250lb) ",quantity);
		break;
	case WPNTYPE_BOMB500HD:
		ammoStr.Printf("BOM:%d (500lb High Drag) ",quantity);
		break;
	case WPNTYPE_FLARE:
		ammoStr.Printf("FLR:%d  ",quantity);
		break;
	case WPNTYPE_FUELTANK:
		ammoStr.Printf("FUEL:%d (%d)  ",quantity,level);
		break;
	case WPNTYPE_SMOKE:
		ammoStr="SMOKE";
		if(availableChannel==1)
		{
		}
		else if((channel&availableChannel)==availableChannel)
		{
			ammoStr.Append("(ALL)");
		}
		else
		{
			ammoStr.Append("(");
			for(unsigned int bit=1; 0!=(bit&availableChannel); bit<<=1)
			{
				if(0!=(channel&bit))
				{
					ammoStr.Append("1");
				}
				else
				{
					ammoStr.Append("0");
				}
			}
			ammoStr.Append(")");
		}
		break;
	}

	ammoStr.Append(intervalStr);

	YsString msgStr;
	if(YSTRUE==selected)
	{
		msgStr=">>";
		msgStr.Append(ammoStr);
		msgStr.Append("<<");
	}
	else
	{
		msgStr="  ";
		msgStr.Append(ammoStr);
	}

	return msgStr;
}

YSBOOL FsAmmunitionIndication::Ammunition::ReadyToFire(void) const
{
	switch(wpnType)
	{
	default:
		break;
	case WPNTYPE_AIM9:
	case WPNTYPE_AIM9X:
	case WPNTYPE_AIM120:
		if(YsTolerance<standByTimer)
		{
			return YSFALSE;
		}
		break;
	}
	return YSTRUE;
}
