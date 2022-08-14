#include "fsdef.h"


extern void FsDef_DummyFunc(void);

void FsDef_DummyFunc(void)
{
}



YsString FsAllowedWeaponTypeToStr(unsigned int allowedWeaponType)
{
	YsString str;
	if(0!=(allowedWeaponType&FSWEAPON_ALLOWGUN))
	{
		str.Append(" GUN");
	}
	if(0!=(allowedWeaponType&FSWEAPON_ALLOWROCKET))
	{
		str.Append(" RKT");
	}
	if(0!=(allowedWeaponType&FSWEAPON_ALLOWAAM))
	{
		str.Append(" AAM");
	}
	if(0!=(allowedWeaponType&FSWEAPON_ALLOWAGM))
	{
		str.Append(" AGM");
	}
	if(0!=(allowedWeaponType&FSWEAPON_ALLOWBOMB))
	{
		str.Append(" BOM");
	}
	return str;
}

unsigned int FsArgvToAllowedWeaponType(const YsConstArrayMask <YsString> &argv)
{
	unsigned int allowedWeaponType=0;
	for(auto &str : argv)
	{
		if(0==str.STRCMP("GUN"))
		{
			allowedWeaponType|=FSWEAPON_ALLOWGUN;
		}
		if(0==str.STRCMP("RKT"))
		{
			allowedWeaponType|=FSWEAPON_ALLOWROCKET;
		}
		if(0==str.STRCMP("AAM"))
		{
			allowedWeaponType|=FSWEAPON_ALLOWAAM;
		}
		if(0==str.STRCMP("AGM"))
		{
			allowedWeaponType|=FSWEAPON_ALLOWAGM;
		}
		if(0==str.STRCMP("BOM"))
		{
			allowedWeaponType|=FSWEAPON_ALLOWBOMB;
		}
	}
	return allowedWeaponType;
}

const char *FsFlightStateToStr(FSFLIGHTSTATE sta)
{
	switch(sta)
	{
	case FSFLYING:
		return "FLYING";
	case FSGROUND:
		return "GROUND";
	case FSSTALL:
		return "STALL";
	case FSDEAD:
		return "DEAD";
	case FSDEADSPIN:
		return "DEADSPIN";
	case FSDEADFLATSPIN:
		return "DEADFLATSPIN";
	case FSGROUNDSTATIC:
		return "GROUNDSTATIC";
	case FSOVERRUN:
		return "OVERRUN";
	}
	return "Unknown";
}
