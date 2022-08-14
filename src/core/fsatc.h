#ifndef FSATC_IS_INCLUDED
#define FSATC_IS_INCLUDED
/* { */

#include <ysclass.h>
#include "fsdef.h"
#include "voicedll/fsvoiceenum.h"

#include <ysscenery.h>

class FsVoicePhrasePrep
{
public:
	FSVOICE_PHRASE_TYPE phraseType;
	YsString english;
};

class FsVoicePhraseAssembly
{
private:
	YsArray <FsVoicePhrasePrep,16> sentence;

public:
	void Clear(void);
	void AddString(FSVOICE_PHRASE_TYPE,const char []);
	void AddInt(FSVOICE_PHRASE_TYPE,int );
	void AddDouble(FSVOICE_PHRASE_TYPE,const double &);
	void AddCommaAndSpace(void);
	void AddSpace(void);
	void AddPeriod(void);
	void AddEndOfSentence(void);

	YSRESULT AssembleString(int &ptr,YsString &str) const;

	template <const int N>
	YSRESULT AssemblePhraseForSpeech(YsArray <FsVoicePhrase,N> &speechPhrase) const;
};

template <const int N>
YSRESULT FsVoicePhraseAssembly::AssemblePhraseForSpeech(YsArray <FsVoicePhrase,N> &speechPhrase) const
{
	speechPhrase.Set(sentence.GetN(),NULL);
	for(int i=0; i<sentence.GetN(); i++)
	{
		speechPhrase[i].phraseType=sentence[i].phraseType;
		speechPhrase[i].phrase=sentence[i].english.Txt();
	}
	if(0<sentence.GetN())
	{
		return YSOK;
	}
	return YSERR;
}

class FsAirTrafficController
{
friend class FsAirTrafficInfo;

private:
	unsigned int searchKey;
	unsigned int jurisdictionFlag;
	YsString name;
	YsArray <unsigned int> talkingAirKey;
	YSBOOL needNoticeSound;

	// Block copy operator and copy constructor
	const FsAirTrafficController &operator=(const FsAirTrafficController &){return *this;}
	FsAirTrafficController(const FsAirTrafficController &) {}

public:
	static const unsigned int PrimaryAirTrafficControllerKey;
	static const unsigned int NullAirTrafficControllerKey;

	static const char *PrimaryAirTrafficControllerName;

	static const double TerrainClearanceAtIAF(void);
	static const double TerrainClearanceApproach(void);
	static const double TimeToReduceSpeedBeforeIAF(void);
	static const double GetRadiusBeginDescendFromEnroute(void);


	enum
	{
		JURISDICTION_ALL=~0,
		JURISDICTION_ENROUTE=1,
		JURISDICTION_APPROACH=2,
		JURISDICTION_TOWER=4,
		JURISDICTION_GROUND=8
	};

	FsAirTrafficController();
	void AssignSearchKey(unsigned int searchKey);
	void CleanUp(void);

	const char *GetName(void) const;
	YSBOOL GetAndClearNeedNoticeFlag(void);

	void Process(class FsSimulation *sim,const double &currentTime);
private:
	void ProcessApproach(class FsSimulation *sim,const double &currentTime,class FsAirplane *air);
	void ProcessApproach_OnGround(class FsSimulation *sim,const double &currentTime,class FsAirplane *air);
	void SetPlayerCallSignMessage(FsVoicePhraseAssembly &phrase,const FsAirplane &air);
	void ProcessApproach_Approach(class FsSimulation *sim,const double &currentTime,class FsAirplane *air);
	YSRESULT AddHeadingMessage(double &magHdg,double &timeForTurn,FsVoicePhraseAssembly &phrase,const class FsSimulation *sim,class FsAirplane &air,const YsVec3 &wayPoint);
	YSRESULT AddAltitudeMessage(double &alt,FsVoicePhraseAssembly &phrase,const class FsSimulation *sim,class FsAirplane &air,const double y);
	void ShowMessage(class FsSimulation *sim,const FsVoicePhraseAssembly &phrase);

public:
	unsigned int SearchKey(void) const;

	void SetName(const char str[]);

	void SetAllJurisdiction(void);
	void SetIsEnrouteController(YSBOOL tf);
	void SetIsApproachController(YSBOOL tf);
	void SetIsTowerController(YSBOOL tf);
	void SetIsGroundController(YSBOOL tf);

	YSBOOL IsEnrouteController(void) const;
	YSBOOL IsApproachController(void) const;
	YSBOOL IsTowerController(void) const;
	YSBOOL IsGroundController(void) const;

	YSRESULT AirplaneCheckIn(class FsSimulation *sim,class FsAirplane *air);
	YSRESULT AirplaneRequestApproachByILS(class FsSimulation *sim,class FsAirplane &air,const class FsGround *ilsFacility,const char dstAirportName[]);
	YSRESULT AirplaneRequestHeading(class FsSimulation *sim,class FsAirplane &air);

	/*! If the airplane is in the final approach phase, re-make the course (same as requesting a new approach).
	    If not, give a heading to the current waypoint.
	*/
	YSRESULT AirplaneRequestNewVector(class FsSimulation *sim,class FsAirplane &air);
	YSRESULT AirplaneDeclareMissedApproach(class FsSimulation *sim,class FsAirplane &air);
	YSRESULT AirplaneCancelIFR(class FsSimulation *sim,class FsAirplane &air);
private:
	YSRESULT RemoveAirplane(class FsSimulation *sim,class FsAirplane *air);
	YSRESULT AddAirplane(class FsSimulation *sim,class FsAirplane *air);

public:
	static YSRESULT GetNewVectorToWayPoint(YsVec3 &newVec,double &Rturn,const YsVec3 &curAirPos,const YsVec3 &curAirVel,const YsVec3 &wayPoint);
	static double RoundUpAltitude(const double &alt,const double &baseElv,const double &denom);
	static double VectorToMagneticHeading(const class FsSimulation *sim,const YsVec3 &vec);
};

class FsFlightPlanFix
{
public:
	enum FIXTYPE
	{
		FIX_NULL,
		FIX_GROUND,
		FIX_DONWINDENTRY,
		FIX_BASEENTRY,
		FIX_DOGLEGENTRY,
		FIX_LOCALIZERINTERCEPT,
		FIX_GLIDESLOPEINTERCEPT,
		FIX_CUSTOM_APPROACH_FIX,
		FIX_WAYPOINT,
	};

	static FIXTYPE StringToFixType(const char str[]);
	static const char *FixTypeToString(FIXTYPE fixType);

protected:
	unsigned int gndKey;
	YsVec3 pos;
	FIXTYPE fixType;
public:
	FsFlightPlanFix();
	void CleanUp();

	void SetPositionFromFacility(const class FsSimulation *sim,const class FsGround *gnd);
	class FsGround *GetFacility(const class FsSimulation *sim);

	void SetPosition(const YsVec3 &pos,FIXTYPE fixType);
	const YsVec3 &GetPosition(void) const;

	FIXTYPE GetFixType(void) const;
	const char *GetFixString(void) const;

	YSBOOL IsApproachFix(void) const;
};

class FsApproach
{
public:
	enum APPROACHTYPE
	{
		APP_NULL,
		APP_VISUAL,
		APP_ILS
	};

	static const char *AppTypeToString(APPROACHTYPE appType);
	static APPROACHTYPE StringToAppType(const char str[]);

protected:
	APPROACHTYPE appType;
	YsString dstName;
	unsigned int gndKey;

	YsString gndNameCacheForFileIO;

public:
	FsApproach();
	void CleanUp(void);

	/*! Returns the destination name.
	*/
	const char *GetDestinationName(void) const;
	class FsGround *GetFacility(const class FsSimulation *sim) const;
	APPROACHTYPE GetApproachType(void) const;
	const char *GetApproachTypeString(void) const;

	void SetDestinationName(const char dstNameIn[]);

	void SetFacility(const class FsSimulation *sim,const FsGround *gnd);
	void SetApproachType(APPROACHTYPE appType);

	YSRESULT Save(FILE *fp,const FsSimulation *sim) const;
	YSRESULT Load(FsSimulation *sim,FILE *fp);
	YSRESULT ReconnectGroundFacility(FsSimulation *sim);  // Called from FsAirTrafficInfo
};

class FsAirTrafficInfo
{
friend class FsAirTrafficController;

public:
	enum FLIGHTSTAGE
	{
		STAGE_NEEDCALC,
		STAGE_ONGROUND,
		STAGE_CLIMB,
		STAGE_ENROUTE,
		STAGE_APPROACH,
		STAGE_FINAL
	};


protected:
	YSBOOL talkingWithAtc;
	unsigned int atcKey;
	FLIGHTSTAGE stage;
	int currentPathIndex;                // Heading to path[currentPathIndex];
	double clockLastInstructionFromAtc;  // Negative: Initial Contact
	double clockNextHeadingCheck;        // Check for heading next time.
	YsArray <FsFlightPlanFix> path;
	FSFLIGHTRULE IFRorVFR;
	FsApproach approach;

	double assignedAltitude,assignedHeading,assignedAirSpeed;

	YsString atcNameCacheForFileIO;

public:
	FsAirTrafficInfo();
	virtual ~FsAirTrafficInfo();
	void CleanUp(void);
	YSBOOL IsApproachAlreadySetUp(void) const;

private: // Called from FsATC
	void SetAirTrafficController(class FsSimulation *sim,FsAirTrafficController *atc);
	void CalculateStage(FsAirplane *air);
	void AssignHeading(const double headingIn);
	void AssignAltitude(const double altitudeIn);
	void AssignAirSpeed(const double airSpeedIn);

	YSBOOL AirplaneReachTurnPoint(FsAirplane *air) const;
	YSBOOL AirplaneMessedUp(FsAirplane *air) const;
	YSBOOL HeadingDeviationExceed(class FsSimulation *sim,FsAirplane *air,const double angle) const;
	void IncrementWayPoint(void);
	const FsFlightPlanFix *GetNextWayPoint(void) const;
	const double DistanceToNextFix(FsAirplane *air) const;
	void RewindFix(void);

public:
	void StopTalkingWithAirTrafficController(void);
	FsAirTrafficController *GetAirTrafficController(class FsSimulation *sim);
	const FsAirTrafficController *GetAirTrafficController(const class FsSimulation *sim) const;

	double GetLastAtcContactTime(void) const;
	void SetLastAtcContactTime(double ctime);

	double GetNextHeadingCheckTime(void) const;
	void SetNextHeadingCheckTime(double nextCheckTimeIn);

	FLIGHTSTAGE GetStage(void) const;
	void SetStage(FLIGHTSTAGE stageIn);

	const double GetAssignedHeading(void) const;  // Magnetic heading.  Not heading of YsAtt3.
	const double GetAssignedAltitude(void) const;
	const double GetAssignedAirSpeed(void) const;

	YSRESULT SelectFirstFixForApproach(FsFlightPlanFix::FIXTYPE fixType);
	YSRESULT FindFixBefore(YsVec3 &pos,YsAtt3 &att,FsFlightPlanFix::FIXTYPE fixType) const;

private: // Called from FsATC
	YSRESULT ResetApproach(class FsSimulation *sim,const class FsAirplane &air);
	YSRESULT SelectApproachByAirportName(class FsSimulation *sim,const class FsAirplane &air,const char dstAirportName[]);
	YSRESULT SelectApproachByILS(class FsSimulation *sim,const class FsAirplane &air,const class FsGround *ilsFacility,const char dstAirportName[]);
private:
	YSRESULT CalculateFlightPathForILS(class FsSimulation *sim,const class FsAirplane &air,const class FsGround *ilsFacility);
public:
	// In the future: SelectApproachByFacilityAndAirportName for ILS, NDB, GPS, and VOR approaches
	YSRESULT CalculateFlightPath(const class YsScenery &scn,const class FsAirplane &air,FLIGHTSTAGE stage);

private:
	YSRESULT CalculateFlightPathFromTdPosHeadingAndGlideSlope(class FsSimulation *sim,const class FsAirplane &air,const YsVec3 &tdPos,const YsAtt3 &tdAtt,int sequence);
	const double GetMaxElevationOfLeg(const class FsSimulation *sim,const YsVec3 &p1,const YsVec3 &p2,const double interval) const;
	const double GetMinClearanceOfLeg(const double alt,const class FsSimulation *sim,const YsVec3 &p1,const YsVec3 &p2,const double interval) const;

public:
	const FsApproach &GetApproach(void) const;
	YSBOOL IsHeadingForFinalFix(void) const;
	YSBOOL IsGuidedByATC(void) const;

	YSRESULT Save(FILE *fp,const FsSimulation *sim,const FsAirplane &air) const;
	YSRESULT Load(FsSimulation *sim,FsAirplane &air,FILE *fp);
	YSRESULT ReconnectAtcAndApproach(FsSimulation *sim,FsAirplane *air);  // Used after Load
};



// See memo/designmen/flightschedule.txt
class FsAirTrafficSequence
{
private:
	class Slot
	{
	protected:
		YsArray <unsigned int,8> occupyingObjKey;
		YsArray <unsigned int,8> incomingObjKey;
	public:
		YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType;
		YsString label;
		int objSearchKey;
		YSBOOL accommodateMultiple;

		void Initialize(void);
		void ClearObject(void);
		void RemoveOccupyingObject(unsigned int objKey);
		void AddOccupyingObject(unsigned int objKey);
		void RemoveIncomingObject(unsigned int objKey);
		void AddIncomingObject(unsigned int objKey);
		YSSIZE_T GetNumObject(void) const;
	};

	class LandingTraffic
	{
	public:
		unsigned int objKey;
		YsVec3 tdPos;  // <- may not be necessary.
		YsVec3 objPos;
		double distToTouchDownSq;
		FSTRAFFICPATTERNLEG leg;
	};

	class Runway
	{
	public:
		const class YsSceneryRectRegion *rectRgn;
		YsArray <LandingTraffic,8> traffic;
	};

	YsKeyWordList labelToAirportSlotIdx;
	YsKeyWordList labelToFixSlotIdx;
	YsKeyWordList labelToVorSlotIdx;
	YsKeyWordList labelToNdbSlotIdx;
	YsKeyWordList labelToCarrierSlotIdx;

	YsHashTable <YSSIZE_T> objKeyToOccupyingSlotIdx;
	YsHashTable <YSSIZE_T> objKeyToIncomingSlotIdx;
	YsSegmentedArray <Slot,4> slotArray;

	YsSegmentedArray <Runway,4> runwayArray;
	YsHashTable <YSSIZE_T> runwayKeyToRunwayIdx;

	double lastUpdateTime;
	double updateInterval;


	FsAirTrafficSequence();
	~FsAirTrafficSequence();
public:
	static FsAirTrafficSequence *Create(void);
	static void Delete(FsAirTrafficSequence *ptr);

	void Initialize(void);

	void ClearSequence(void);
	YSRESULT MakeSlotArray(const class FsSimulation *sim);
	void RefreshAirTrafficSlot(const class FsSimulation *sim);

	void ClearRunwayUsage(void);
	void RefreshRunwayUsage(const class FsSimulation *sim);
protected:
	void SortRunwayTrafficByDistanceToTouchDown(void);

public:
	YSRESULT RequestProceed(unsigned int objKey,YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[]);
	YSRESULT ClearOccupyingAirportOrFix(unsigned int objKey);

	const double GetLastUpdatedTime(void) const;
	const double GetNextUpdateTime(void) const;

protected:
	YSSIZE_T FindSlotIndex(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[]) const;
	void AddIncomingObject(unsigned int objKey,YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[]);
	void AddIncomingObject(unsigned int objKey,YSSIZE_T slotIndex);

public:
	void AddOccupyingObject(unsigned int objKey,YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[]);

protected:
	void AddOccupyingObject(unsigned int objKey,YSSIZE_T slotIndex);

public:
	void AddLandingOrTakingOffTraffic(FSTRAFFICPATTERNLEG leg,const class FsExistence *obj,const YsVec3 &tdPos,YSSIZE_T nRgn,const class YsSceneryRectRegion * const rgnArray[]);
	template <const int N>
	inline void AddLandingOrTakingOffTraffic(FSTRAFFICPATTERNLEG leg,const class FsExistence *obj,const YsVec3 &tdPos,const YsArray <const YsSceneryRectRegion *,N> &rgnArray);

	YSRESULT FindLandingAndTakingOffTraffic(YSSIZE_T &nTra,const LandingTraffic *&traArray,const class YsSceneryRectRegion *runwayRect) const;

	YSBOOL WillCauseRunwayIncursionFromTakeOffTraffic(const class FsAirplane &airTakingOff,const FsSimulation *sim,const class YsSceneryRectRegion *runwayRect) const;
	YSBOOL WillCauseRunwayIncursionFromLandingTraffic(const class FsAirplane &airLanding,const class YsSceneryRectRegion *runwayRect) const;
};

template <const int N>
inline void FsAirTrafficSequence::AddLandingOrTakingOffTraffic(FSTRAFFICPATTERNLEG leg,const FsExistence *obj,const YsVec3 &tdPos,const YsArray <const YsSceneryRectRegion *,N> &rgnArray)
{
	AddLandingOrTakingOffTraffic(leg,obj,tdPos,rgnArray.GetN(),rgnArray.GetArray());
}

/* } */
#endif
