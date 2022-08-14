#ifndef FSSIMINFO_IS_INCLUDED
#define FSSIMINFO_IS_INCLUDED
/* { */


#include "fsdef.h"
#include "fsweather.h"



class FsSimInfo
{
public:
	// For loading from and saving to a .YSF file. >>
	class ObjRef
	{
	public:
		FSEXISTENCETYPE objType;
		YsString yfsLabel;
		int yfsIdx;

		mutable YSHASHKEY objKeyCache;

		ObjRef();
		void Initialize(void);
		void SetObject(const class FsExistence *obj);
		void SetYfsIdent(FSEXISTENCETYPE objType,int yfsIdent);
		void SetYfsLabel(FSEXISTENCETYPE objType,const char yfsLable[]);
		FsExistence *GetObject(class FsSimulation *sim) const;
		const FsExistence *GetObject(const class FsSimulation *sim) const;
	};
	class AirRef : public ObjRef
	{
	public:
		class FsAirplane *GetAircraft(class FsSimulation *sim) const;
	};
	class GndRef : public ObjRef
	{
	public:
		class FsGround *GetGround(class FsSimulation *sim) const;
	};
	// For loading from and saving to a .YSF file. <<


	// For defining an air base >>
	enum BASE_TYPE
	{
		AIRPORT,
		CARRIER
	};

	static const char *BaseTypeToString(BASE_TYPE baseType)
	{
		switch(baseType)
		{
		case AIRPORT:
			return "AIRPORT";
		case CARRIER:
			return "CARRIER";
		}
		return "AIRPORT"; // ?
	}

	static BASE_TYPE StringToBaseType(const char str[])
	{
		if(0==strcmp(str,"AIRPORT"))
		{
			return AIRPORT;
		}
		if(0==strcmp(str,"CARRIER"))
		{
			return CARRIER;
		}
		return AIRPORT; // ?
	}

	class AirBase
	{
	private:
		YsString baseName;
		BASE_TYPE baseType;

		mutable YSBOOL cached;
		mutable YSRESULT cacheError;
		mutable YSHASHKEY carrierKeyCache;
		mutable const class YsSceneryRectRegion *airportCache;

	public:
		AirBase();
		void CleanUp(void);
		void SetName(const YsString &baseName);
		void SetType(BASE_TYPE baseType);
		const YsString &GetName(void) const;
		BASE_TYPE GetType(void) const;

		YSBOOL IsCached(void) const;
		YSRESULT Encache(const class FsSimulation *sim) const;
		void Decache(void) const;
		FsGround *GetCachedCarrier(const class FsSimulation *sim) const;
		const YsSceneryRectRegion *GetCachedAirportRect(const class FsSimulation *sim) const;
	};

	// For defining an air base <<


	// For defining a ground to air threat >>
	class GndToAirThreat
	{
	public:
		YSHASHKEY objKey;
		double range;
	};
	// For defining a ground to air threat <<
};




class FsSimulationEvent
{
public:
	double eventTime;
	int eventType;
	unsigned int eventFlag;

	YsString str;            // For FSEVENT_TEXTMESSAGE, FSEVENT_AIRCOMMAND

	YsArray <int> weaponCfg; // FSEVENT_SETWEAPONCONFIG

	YsArray <FsWeatherCloudLayer> cloudLayer;

	union
	{
		int objKey;                               // FSEVENT_PLAYEROBJCHANGE, FSEVENT_AIRCOMMAND, FSEVENT_WEAPONCONFIG
		FSENVIRONMENT env;                        // FSEVENT_ENVIRONMENTCHANGE
		double visibility;                        // FSEVENT_VISIBILITYCHANGE
	};

	YsVec3 wind;             // FSEVENT_WINDCHANGE


	FsSimulationEvent();
	void Initialize(void);
	void Save(FILE *fp,const class FsSimulation *sim) const;
	void Load(FILE *fp,const FsSimulation *sim);
};

////////////////////////////////////////////////////////////

class FsSimulationEventStore
{
public:
	int nextEvent;
	YsArray <FsSimulationEvent> eventList;

	FsSimulationEventStore();
	void Initialize(void);
	void CleanUp(void);
	void Rewind(void);
	void AddEvent(const double &ctime,const FsSimulationEvent &newEvt);
	void SortEventByTime(void);
	void DeleteFutureEventForResume(const double currentTime);
	void DeleteEventByTypeAll(int eventType);
	void SeekNextEventPointereToCurrentTime(const double currentTime);

	void Save(FILE *fp,const class FsSimulation *sim) const;

	void Load(FILE *fp,const FsSimulation *sim);
	void MatchAirGndYfsId(const FsSimulation *sim);
};

////////////////////////////////////////////////////////////

class FsLandingAnalysis
{
public:
	unsigned int airSearchKey;
	double t;
	YsString airIdentifier;
	YsVec3 p,v;
	double hdg;
};

////////////////////////////////////////////////////////////

class FsCloseAirSupportMissionInfo
{
public:
};

////////////////////////////////////////////////////////////

class FsDemoModeInfo
{
public:
	FsDemoModeInfo(FSDEMOMODETYPE type);
	void Initialize(FSDEMOMODETYPE type);
	class FsAirplane *PickRandomAirplane(int &fomPosition,YSBOOL formation,YSBOOL solo);
	int GetFormationPosition(FsAirplane *air);  // Position   >0:Formation  <0:Solo

	enum
	{
		MAXNUMFORMATION=10,
		MAXNUMSOLO=10
	};

	FSDEMOMODETYPE type;
	double maxTime;
	double nextReconsiderViewTargetTime;
	YSBOOL terminateByUser;
	double endTime;

	// For Landing Screensaver and Concorde Fly-by
	YSBOOL useCockpitView;
	YSBOOL useOutsideView;
	YSBOOL useCarrierView;
	YSBOOL usePlayerToGndView;
	YSBOOL useGndToPlayerView;
	YSBOOL useAirToAirView;
	YSBOOL useAirFromAirView;
	double changeViewPointInterval;

	// For Aerobatic Screensaver
	YsVec3 showCenter;                        // <- Also used in Concorde Fly-by
	class FsGround *refObj;
	int acroType;
	class FsAirplane *formation[MAXNUMFORMATION];   // <- Also used in Concorde Fly-by
	class FsAirplane *solo[MAXNUMSOLO];

	// For Concorde Fly-by
	int concFlyByType;  // 0:Heathrow -> Buckingham -> Back to Heathrow
	                    // 1:Gatwick -> Heathrow
	                    // 2:Formation with Red Arrows
	                    // 3:Supersonic over Hawaii
	                    // 4:Parallel Landing at Heathrow
	int concFlyByStep;
	double nextActionTime;
};



////////////////////////////////////////////////////////////


class FsNewFlightAirplaneData
{
public:
	void Initialize(void);

	int iff;
	YsString typeName;
	YsArray <int,64> weaponConfig;
	int fuel;
	YsString startPos;

	YSRESULT Save(FILE *fp) const;
	YSRESULT Load(FILE *fp);
};



class FsMissionEnemyGroundAttackerInfo
{
public:
	YSBOOL allowAirCover;
	YSBOOL allowStealth;
	YSBOOL allowBomb;
	YSBOOL allowAGM;
	YSBOOL allowHeavyBomber;
	YSBOOL jet,ww2;

	int maxNumAttacker;

	FsMissionEnemyGroundAttackerInfo();
	void Initialize(void);
};


////////////////////////////////////////////////////////////



class FsInterceptMissionInfo
{
public:
	FsInterceptMissionInfo();

	FsNewFlightAirplaneData playerAirInfo;
	FsMissionEnemyGroundAttackerInfo attackerInfo;

	YsString fieldName;

	int numWingman;

	YsArray <YsString> Encode() const;
	YSRESULT Decode(const YsConstArrayMask <YsString> &argv);
	YSRESULT Save(const wchar_t fn[]) const;
	YSRESULT Save(FILE *fp) const;
	YSRESULT Load(const wchar_t fn[]);
	YSRESULT Load(FILE *fp);
};



////////////////////////////////////////////////////////////



class FsNewDriveDialogInfo
{
public:
	YsString fieldName;
	int gobId;
	YSBOOL driveNow;

	void Initialize(void);
};



class FsGroundToAirDefenseMissionInfo : public FsNewDriveDialogInfo
{
public:
	int numIntercepter;
	YsString intercepter;
	YSBOOL infiniteGun;

	FsMissionEnemyGroundAttackerInfo attackerInfo;

	// Bodies written in fssimulation.cpp
	FsGroundToAirDefenseMissionInfo();
	void Initialize(void);

	YsArray <YsString> Encode(void) const;
	YSRESULT Decode(const YsConstArrayMask <YsString> &argv);

	YSRESULT Save(const wchar_t fn[]) const;
	YSRESULT Save(FILE *fp) const;
	YSRESULT Load(const wchar_t fn[]);
	YSRESULT Load(FILE *fp);
};



/* } */
#endif
