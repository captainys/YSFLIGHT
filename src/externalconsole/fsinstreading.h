#ifndef FSINSTREADING_IS_INCLUDED
#define FSINSTREADING_IS_INCLUDED
/* { */

#include <ysclass.h>

class FsVORIndication
{
public:
	enum
	{
		FLAG_NULL=0,    // Flags are used in the encoding.  Don't change.  Add if needed.
		FLAG_SELECTED=1,
		FLAG_INOP=2,
		FLAG_INRANGE=4,
		FLAG_TUNED=8,
		FLAG_ISILS=16,
		FLAG_ISDME=32
	};

	int navId;               // 0:Nav1   1:Nav2
	YsString vorId;
	unsigned int flags;
	int toFrom;              // 1:TO     0:No indication  2:FROM
	double obs,lateralDev,glideSlopeDev,dme;

	void CleanUp(void);

	void SetSelected(YSBOOL selected);
	YSBOOL IsSelected(void) const;

	void SetInop(YSBOOL selected);
	YSBOOL IsInop(void) const;

	void SetInRange(YSBOOL inRange);
	YSBOOL IsInRange(void) const;

	void SetTuned(YSBOOL tuned);
	YSBOOL IsTuned(void) const;

	void SetIsILS(YSBOOL isILS);
	YSBOOL IsILS(void) const;

	void SetIsDME(YSBOOL isDME);
	YSBOOL IsDME(void) const;

	YSSIZE_T NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const;
	YSRESULT NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize);
};

class FsADFIndication
{
public:
	enum
	{
		FLAG_NULL=0,    // Flags are used in the encoding.  Don't change.  Add if needed.
		FLAG_SELECTED=1,
		FLAG_INOP=2,
		FLAG_INRANGE=4,
		FLAG_TUNED=8,
	};

	int adfId;               // 0:Adf1
	YsString ndbId;
	unsigned int flags;
	double bearing;

	void CleanUp(void);

	void SetSelected(YSBOOL selected);
	YSBOOL IsSelected(void) const;

	void SetInop(YSBOOL selected);
	YSBOOL IsInop(void) const;

	void SetInRange(YSBOOL inRange);
	YSBOOL IsInRange(void) const;

	void SetTuned(YSBOOL tuned);
	YSBOOL IsTuned(void) const;

	YSSIZE_T NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const;
	YSRESULT NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize);
};

class FsRadarIndication
{
public:
	enum RADARECHOTYPE
	{
		ECHO_NONE=0,
		ECHO_AIRCRAFT=1,
		ECHO_GROUND=2,
		ECHO_AAM=3,
		ECHO_AAAM=4,
		ECHO_AGM=5,
		ECHO_BOMB=6
	};

	class RadarEcho
	{
	public:
		RADARECHOTYPE echoType;

		// All relative to the observing aircraft
		YsVec3 position;
		YsVec2 horizontal;
		YsVec3 velocity;
	};

	double radarRange,maxRadarRange;
	YsArray <RadarEcho,32> echoArray;

	void CleanUp(void);

	YSSIZE_T NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const;
	YSRESULT NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize);
};

class FsAmmunitionIndication
{
public:
	enum WEAPONTYPE
	{
		WPNTYPE_NULL,           // The order shall not be changed.
		WPNTYPE_GUN,            // The order shall not be changed.
		WPNTYPE_AIM9,           // The order shall not be changed.
		WPNTYPE_AGM65,          // The order shall not be changed.
		WPNTYPE_BOMB,           // The order shall not be changed.
		WPNTYPE_ROCKET,         // The order shall not be changed.
		WPNTYPE_FLARE,          // The order shall not be changed.
		WPNTYPE_AIM120,         // The order shall not be changed.
		WPNTYPE_BOMB250,        // The order shall not be changed.
		WPNTYPE_SMOKE,          // The order shall not be changed.
		WPNTYPE_BOMB500HD,      // The order shall not be changed.
		WPNTYPE_AIM9X,          // The order shall not be changed.
		WPNTYPE_FUELTANK,       // The order shall not be changed.
	};

	class Ammunition
	{
	public:
		FsAmmunitionIndication::WEAPONTYPE wpnType;
		YSBOOL selected;
		int quantity,maxQuantity;       // maxQuantity is only for WPNTYPE_GUN
		int level;                      // Remaining fuel for external fuel tank.
		int standByTimer;               // In milli seconds
		unsigned int channel;           // For smoke, choice of smoke generators by bits.
		unsigned int availableChannel;  // For smoke, possible smoke generators by bits.

		YsString FormatString(void) const;
		YSBOOL ReadyToFire(void) const;
	};

	YsArray <Ammunition,8> ammoArray;

	void CleanUp(void);

	YSSIZE_T NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const;
	YSRESULT NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize);
};

class FsInstrumentIndication
{
public:
	enum
	{
		MAX_NUM_ENGINE=16,
		MAX_NUM_FUELTANK=16
	};

	double heading,headingBug;
	YSBOOL headingBugSelected;
	double pitch,bank;
	double turnRate;
	double sideSlip;
	double altitude;
	double verticalSpeed;
	double airSpeed;
	double Vfe,Vno,Vne,VindicatorRange;
	int nEngine;
	double engineOutput[MAX_NUM_ENGINE];
	YSBOOL afterBurner[MAX_NUM_ENGINE];

	int nFuelTank;
	double fuelRemain[MAX_NUM_FUELTANK],fuelCapacity[MAX_NUM_FUELTANK];

	double mach;
	double gForce;
	YSBOOL hasVectorThrust;
	YsVec3 nozzleDirection;
	YSBOOL hasRetractableGear;
	double gearPos;
	double brake;
	double flaps;
	YSBOOL hasSpoiler;
	double spoiler;
	YSBOOL autoPilot;

	double elevator,elevatorTrim,aileron,rudder;

	YsVec3 velocity;  // Relative to the aircraft coordinate

	void CleanUp(void);

	YSSIZE_T NetworkEncode(unsigned char buf[],YSSIZE_T bufSize) const;
	YSRESULT NetworkDecode(const unsigned char buf[],YSSIZE_T codeSize);
};


class FsCockpitIndicationSet
{
public:
	enum
	{
		NUM_NAV=2,
		NUM_ADF=1
	};

	FsInstrumentIndication inst;
	FsVORIndication nav[NUM_NAV];
	FsADFIndication adf[NUM_ADF];
	FsRadarIndication radar;
	FsAmmunitionIndication ammo;

	void CleanUp(void);
};

/* } */
#endif
