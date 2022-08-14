#ifndef FSWORLD_IS_INCLUDED
#define FSWORLD_IS_INCLUDED
/* { */

#include <memory>

#include "fsgroundproperty.h"
#include "fssimulation.h"

// Declaration /////////////////////////////////////////////
class FsAirplaneTemplate
{
public:
	FsAirplaneTemplate();
	virtual ~FsAirplaneTemplate();

	void Initialize(void);
	void Unprepare(void);

	YsString idName;
	FSAIRPLANECATEGORY airCat;

	const wchar_t *GetTemplateRootDirectory(void) const;

	const wchar_t *GetPropFileName(void) const;
	const wchar_t *GetVisualFileName(void) const;
	const wchar_t *GetCockpitFileName(void) const;
	const wchar_t *GetLodFileName(void) const;
	const wchar_t *GetCollisionFileName(void) const;

	class FsVisualDnm GetVisual(void) const;
	class FsVisualDnm GetLod(void) const;
	class FsVisualDnm GetCockpit(void) const;
	const class FsVisualSrf *GetCollision(void) const;
	const class FsAirplaneProperty *GetProperty(void) const;

	class FsVisualDnm GetWeaponVisual(FSWEAPONTYPE wpnType,int state) const;

	class FsVisualSrf *GetCollision(void);
	class FsAirplaneProperty *GetProperty(void);

	void SetRootDir(const wchar_t rootDir[]);
	void SetPropFileName(const wchar_t fn[]);
	void SetVisualFileName(const wchar_t fn[]);
	void SetCockpitFileName(const wchar_t fn[]);
	void SetLodFileName(const wchar_t fn[]);
	void SetCollisionFileName(const wchar_t fn[]);

protected:
	mutable class FsVisualDnm vis;
	mutable class FsVisualDnm cockpit;
	mutable class FsVisualDnm lod;
	mutable class FsVisualSrf *coll;
	mutable class FsAirplaneProperty *prop;
	mutable class FsVisualDnm weaponShapeOverride[2][FSWEAPON_NUMWEAPONTYPE];

	YsWString tmplRootDir;

	YsWString _propFileName;
	YsWString _visFileName;
	YsWString _cockFileName;
	YsWString _lodFileName;
	YsWString _collFileName;

	mutable YsWString propFullPathName;
	mutable YsWString visFullPathName;
	mutable YsWString cockFullPathName;
	mutable YsWString lodFullPathName;
	mutable YsWString collFullPathName;
};




// Declaration /////////////////////////////////////////////

// NOTE : Aircraft Carrier Property is not loaded when the template is loaded
//        but is loaded when template is instantiated (added to FsSimulation)

class FsGroundTemplate
{
public:
	FsGroundTemplate();
	virtual ~FsGroundTemplate();

	void Initialize(void);

	const wchar_t *GetVisualFileName(void) const;
	const wchar_t *GetLodFileName(void) const;
	const wchar_t *GetCollisionFileName(void) const;
	const wchar_t *GetRawCollisionFileName(void) const;
	const wchar_t *GetCockpitFileName(void) const;
	const wchar_t *GetAircraftCarrierFileName(void) const;
	const wchar_t *GetTemplateRootDirectory(void) const;

	void SetRootDir(const wchar_t rootDir[]);
	void SetVisualFileName(const wchar_t fn[]);
	void SetLodFileName(const wchar_t fn[]);
	void SetCollisionFileName(const wchar_t fn[]);
	void SetCockpitFileName(const wchar_t fn[]);
	void SetAircraftCarrierFileName(const wchar_t fn[]);

	FsVisualDnm vis;
	FsVisualDnm lod;
	FsVisualSrf *coll;
	FsVisualDnm cockpit;
	FsGroundProperty prop;
	YSBOOL isAircraftCarrier;

protected:
	YsWString tmplRootDir;

	YsWString _visFileName;
	YsWString _lodFileName;
	YsWString _collFileName;
	YsWString cockpitFileName;
	YsWString _aircraftCarrierFileName;

	mutable YsWString visFullPathName;
	mutable YsWString lodFullPathName;
	mutable YsWString collFullPathName;
	mutable YsWString cockpitFullPathName;
	mutable YsWString aircraftCarrierFullPathName;
};





// Declaration /////////////////////////////////////////////
class FsFieldTemplate
{
private:
	YsWString tmplRootDir;

	YsString idName;
	YsWString visFileName;
	YsWString yfsFileName;
	YsArray <YsString> startPosFile;

	mutable YsWString visFullPathName;
	mutable YsWString yfsFullPathName;

	class YsScenery *fld;
	YsScenery::ERRORCODE fldError;

	YSBOOL raceCourseAvailable;

public:
	FsFieldTemplate();
	virtual ~FsFieldTemplate();

	void Initialize(void);

	void MakeEmptyTemplate(void);
	class YsScenery *GetField(void) const;
	YSRESULT LoadField(void);
	void DeleteField(void);

	void SetRootDir(const wchar_t dir[]);
	void SetIdName(const char name[]);
	void SetVisualFileName(const wchar_t fn[]);
	void SetYfsFileName(const wchar_t fn[]);
	void AddStartPosFileLine(const char str[]);

	void SetRaceCourseFlag(YSBOOL flg);
	YSBOOL IsRaceCourseAvailable(void) const;

	const char *GetIdName(void) const;
	const wchar_t *GetVisualFileName(void) const;
	const wchar_t *GetYfsFileName(void) const;

	int GetNumStartPosFileLine(void) const;
	const char *GetStartPosFileLine(int i) const;

	YsScenery::ERRORCODE GetLastFieldError(void) const;
};



////////////////////////////////////////////////////////////

class FsStartPosInfo
{
public:
	enum
	{
		IFF0=0,
		IFF1=1,
		IFF2=2,
		IFF3=3,
		IFF_ANY=99
	};

	FsStartPosInfo();
	void Initialize();

	YSRESULT InterpretPosition(YsVec3 &pos,YsAtt3 &att);


	YsArray <char,256> idName;
	int iff;
	YsArray <YsArray <char,256> > cmd;


	YSBOOL onCarrier;
	YsArray <char,256> carrierTag;
	YsVec3 relPosCarrier;
	YsAtt3 relAttCarrier;
};



////////////////////////////////////////////////////////////

class FsAddedFieldInfo
{
public:
	YsArray <FsGround *> gndArray;

	FsAddedFieldInfo();
	void Initialize(void);
};


// Declaration /////////////////////////////////////////////

class FsWorld
{
protected:
	FsSimulation *sim;
	YsListContainer <FsAirplaneTemplate> airplaneTemplate;
	YsListContainer <FsGroundTemplate> groundTemplate;
	YsListContainer <FsFieldTemplate> fieldTemplate;

	/* emptyFieldTemplate is used for the situation that field template is unnecessary.
	*/
	FsFieldTemplate emptyFieldTemplate;

private:
	FsWorld(const FsWorld &from);
	FsWorld &operator=(const FsWorld &from);
public:
	enum ERRORCODE
	{
		ERROR_NOERROR,
		ERROR_IO_FILE_NOT_FOUND,
		ERROR_FLD_READERROR,
		ERROR_FLD_VERSION
	};

	class InitializationOption
	{
	public:
		YSBOOL loadDefAir,loadUserAir;
		YSBOOL loadDefField,loadUserField;
		YSBOOL loadDefGnd,loadUserGnd;

		InitializationOption()
		{
			loadDefAir=YSTRUE;
			loadUserAir=YSTRUE;
			loadDefField=YSTRUE;
			loadUserField=YSTRUE;
			loadDefGnd=YSTRUE;
			loadUserGnd=YSTRUE;
		}
	};

private:
	mutable ERRORCODE lastError;
	mutable YsWString lastErrorMessage;

	std::unique_ptr <class FsSimExtensionRegistry> extensionRegistry;

public:
	FsWorld();
	virtual ~FsWorld();

	/*! This returns the last error and clears the lastError=ERRCODE_NOERROR */
	ERRORCODE GetLastError(void) const;

	/*! This returns the error message from the last error, but does not clear the message. */
	const wchar_t *GetLastErrorMessage(void) const;


	class FsSimulation *GetSimulation(void);
	const class FsSimulation *GetSimulation(void) const;


	YSRESULT Save(const wchar_t fn[],
	     int airPosPrecision,int airAttPrecision,
	     int gndPosPrecision,int gndAttPresicion,
	     int wpnPosPrecision,int wpnAttPrecision,
	     const double &timeStep);
	YSRESULT Load(const wchar_t fn[]);

	void SetCanContinue(YSBOOL canContinue);
	void RegisterExtension(std::shared_ptr <FsSimExtensionBase> addOnPtr);

	YSRESULT GetAirplaneTemplateName(char idName[],int id);
	YSRESULT GetAirplaneTemplateName(YsString &airName,int id);
	const char *GetAirplaneTemplateName(int id) const;
	FSAIRPLANECATEGORY GetAirplaneTemplateCategory(int id) const;
	FSAIRPLANECATEGORY GetAirplaneTemplateCategory(const char idName[]) const;
	const wchar_t *GetAirplanePropFileName(const char idName[]) const;

	YSRESULT LoadTemplateAll(void);
	YSRESULT LoadTemplate(InitializationOption opt);

	YSRESULT LoadAirplaneTemplate(InitializationOption opt);
	YSRESULT LoadAirplaneTemplateList(const wchar_t rootDir[],const wchar_t dir[],const wchar_t prefix[],const wchar_t ext[]);
	YSRESULT LoadAirplaneTemplate(
	    const wchar_t rootDir[],
	    const wchar_t prop[],const wchar_t vis[],const wchar_t coll[],const wchar_t cock[],const wchar_t lod[]);
	FsVisualDnm GetAirplaneVisual(const char idName[]) const;
	const FsVisualSrf *GetAirplaneCollision(const char idName[]) const;
	const FsVisualDnm GetAirplaneWeaponShapeOverride(const char idName[],FSWEAPONTYPE wpnType,int state) const;

	YSRESULT GetFighterList(int &nFig,char *fig[],int maxn) const;
	YSRESULT GetAttackerList(int &nFig,char *fig[],int maxn) const;
	YSRESULT GetBomberList(int &nFig,char *fig[],int maxn) const;
	YSRESULT GetJetAirlinerList(int &nAir,char *air[],int maxn) const;
	YSRESULT GetAirplaneListByCategory(int &nFig,char *fig[],int maxn,FSAIRPLANECATEGORY cat) const;
	YSRESULT GetAirplaneListByMultiCategory(int &nFig,char *fig[],int maxn,int nCat,FSAIRPLANECATEGORY cat[]) const;
	YSRESULT GetGroundListByType(YsArray <const char *> &gnd,FSGROUNDTYPE gndType) const;


	YSRESULT LoadGroundTemplate(InitializationOption opt);
	YSRESULT GetGroundTemplateName(char idName[],int id) const;
	const char *GetGroundTemplateName(int id) const;
	YSRESULT LoadGroundTemplateList(const wchar_t rootDir[],const wchar_t dir[],const wchar_t prefix[],const wchar_t ext[]);
	YSRESULT LoadGroundTemplate
	    (const wchar_t rootDir[],const wchar_t prop[],const wchar_t vis[],const wchar_t coll[],const wchar_t cockFileName[],const wchar_t lodFileName[]);

	YSRESULT LoadFieldTemplate(InitializationOption opt);
	YSRESULT GetFieldTemplateName(char idName[],int id) const;
	YSRESULT GetFieldTemplateName(YsString &idName,int id) const;
	const char *GetFieldTemplateName(int id) const;
	YSBOOL IsFieldTemplateRaceCourseAvailable(int id) const;
	YSRESULT LoadFieldTemplateList(const wchar_t rootDir[],const wchar_t dir[],const wchar_t prefix[],const wchar_t ext[]);
	YSRESULT LoadFieldTemplate(const wchar_t rootDir[],const char idName[],const wchar_t vis[],const wchar_t stp[],const wchar_t yfs[],YSBOOL raceCourse);


	YSRESULT GetFieldStartPositionName(char idName[],const char fld[],int id) const;
	YSRESULT GetFieldStartPositionName(YsString &idName,const char fld[],int id) const;
	YSRESULT CheckStartPositionIsAvailable(int fieldId,const char str[]);
	YSRESULT GetStartPositionInfo(FsStartPosInfo &info,const char sta[]);
	YSRESULT GetStartPositionInfo(FsStartPosInfo &info,const char fldIdName[],const char stpName[]);
	YSRESULT SettleAirplane(FsAirplane &air,const char sta[]);
	YSRESULT GetLandingPracticeStartPosition
	    (YSBOOL &leftTraffic,class FsGround *&ils,double &hdg,
	     FsAirplane &air,YsVec3 &pos,YsAtt3 &att,YsVec3 &tdPos,YsAtt3 &tdAtt,
	     double &initSpd,FSTRAFFICPATTERNLEG leg);
	YSRESULT SettleAirplaneForLandingDemo(FsAirplane &air,const double &rwLength,const double &bankLimitOverride);
	YSRESULT SettleAirplaneForCarrierLandingDemo(FsAirplane &air);
	FsGround *PickRandomPrimaryIls(void);

	YSRESULT GetRunwayRectFromPosition(const class YsSceneryRectRegion *&rgn,YsVec3 rect[4],const YsVec3 &pos);

	YSRESULT PrepareSimulation(void);
	YSRESULT SetMissionGoal(class FsMissionGoal &goal);
	YSRESULT TieDownCarrier(void);
	YSRESULT DisableGroundFire(void);
	YSRESULT SetEnvironment(FSENVIRONMENT env);
	FSENVIRONMENT GetEnvironment(void);
	FsAirplane *AddAirplane(const char idName[],YSBOOL isPlayerPlane,unsigned netSearchKey=0);
	FsAirplane *ResetAirplane(FsAirplane *air);
	FsAirplane *AddMatchingAirplane(
	    FSAIRCRAFTCLASS airClass,FSAIRPLANECATEGORY airCategory,unsigned int nationality,YSBOOL isJet,const double &dimension,
	    YSBOOL isPlayerPlane,unsigned netSearchKey=0);
	int GetAirplaneIdFromHandle(FsAirplane *air) const;
	YSRESULT ReplaceAirplane(FsAirplane *air,const char idName[]);
	FsGround *AddGround(const char idName[],YSBOOL isPlayerGound,unsigned netSearchKey=0);
	FsGround *ResetGround(FsGround *gnd);
	YSRESULT SetPlayerGround(FsGround *gnd,YSBOOL record=YSTRUE);
	void ReviveGround(FsGround *gnd);
	YSRESULT SettleGround(FsGround &gnd,const YsVec3 &pos);
	YSRESULT SettleGround(FsGround &gnd,const YsAtt3 &att);
	class FsField *AddField(
	    FsAddedFieldInfo *addedFieldInfo,  // NULL -> Nothing will be returned
	    const char idName[],const YsVec3 &pos,const YsAtt3 &att,
	    YSBOOL loadYFS=YSTRUE,YSBOOL loadAir=YSTRUE,unsigned iffControl=~0);

	/*! SetEmptyField can be used instead of AddField if the field information is not needed.
	*/
	void SetEmptyField(void);

	YSRESULT GetFieldVisual(class YsScenery &scn,const char fldName[]); //2005/03/12 For showing satellite view in Scenery Selection
	
	void CenterJoystick(void);
	YSRESULT MakeCenterJoystickDialog(class FsCenterJoystick &centerJoystickDialog,int nextActionCode);
	YSRESULT CheckJoystickAssignmentAndFixIfNecessary(void);
	YSRESULT RunSimulationOneStep(FsSimulation::FSSIMULATIONSTATE &state);

	/*! SimulateOneStep moves the simulation one step forward for the given time step.
	*/
	YSRESULT SimulateOneStep(
	    const double &dt,
	    YSBOOL demoMode,YSBOOL record,YSBOOL showTimer,YSBOOL networkStandby,FSUSERCONTROL userControl,
	    YSBOOL showTimeMarker);

	void AssignUniqueYsfId(void);
	void RunClientMode(FsNetConfig &netcfg);

	void RunServerModeOneStep(class FsServerRunLoop &svrSta);
	void RunClientModeOneStep(class FsClientRunLoop &cliSta);

	YSBOOL CheckInterceptMissionAvailable(void) const;
	YSBOOL CheckCloseAirSupportMissionAvailable(void) const;

	YSRESULT RunReplayOneStep(FsSimulation::FSSIMULATIONSTATE &state,FsSimulation::ReplayInfo &replayInfo);
	YSRESULT RunDemoMode
	   (FsDemoModeInfo &info,YSBOOL &terminatedByUser,const char sysMsg[],const double &maxTime,
	    YSBOOL drawSmokeAndVapor,YSBOOL preserveFlightRecord);
	YSRESULT PrepareRunDemoMode(FsDemoModeInfo &info,const char sysMsg[],const double &maxTime);
	YSRESULT PrepareAcrobat(FsDemoModeInfo &info,const char airType[],int acroType);
	YSBOOL DemoModeOneStep(FsDemoModeInfo &info,YSBOOL drawSmokeAndVapor,YSBOOL preserveFlightRecord);
	YSRESULT PrepareConcordeFlyby(FsDemoModeInfo &info,int concFlyByType);
	YSRESULT AfterDemoMode(FsDemoModeInfo &info);
	YSRESULT PrepareReplaySimulation(void);
	void DeleteEventByTypeAll(int eventType);

	/*! Call this function to erase first-player object info.  When flying in a flight record using Select Airplane,
	    this information must be removed, or "Retry Previous Flight" will reset the player to the first player object in FsWorld::LoadInternal.
	*/
	void ClearFirstPlayer(void);

	YSRESULT TerminateSimulation(void);
	void TestSolidCloud(void);
	YSRESULT MakeSolidCloud
	   (int n,const YsVec3 &cen,const double &range,const double &sizeH,const double &y0,const double &y1);
	YSRESULT SetFog(YSBOOL drawFog,const double &visibility);



	void DrawInNormalSimulationMode(FsSimulation::FSSIMULATIONSTATE simState,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker);
	void DrawInClientMode(const class FsClientRunLoop &clientModeRunLoop);
	void DrawInServerMode(const class FsServerRunLoop &serverModeRunLoop);



	YSRESULT SetAllowedWeaponType(unsigned int allowedWeaponType);
	void AllowGun(YSBOOL a);
	void AllowAAM(YSBOOL a);
	void AllowAGM(YSBOOL a);
	void AllowBomb(YSBOOL a);
	void AllowRocket(YSBOOL a);

	// External Control (2002/05/14)
	void SetReplayMode(FsSimulation::FSREPLAYMODE replMode);
	const double &GetSimulationTime(void);
	void Jump(const double &newCurrentTime);
	void JumpToFirstRecordTime(void);
	void JumpToLastRecordTime(void);
	YSRESULT DeleteFlightRecord(const double &t1,const double &t2);

	static YSRESULT ConfigDialog(void);
	YSRESULT ApplyConfig(YSBOOL changeEnvironment);

	YSBOOL SimulationIsPrepared(void);
	YSBOOL PlayerPlaneIsReady(void) const;
	YSBOOL PlayerGroundIsReady(void) const;
	YSBOOL IsFlightRecord(void);
	FsAirplane *GetPlayerAirplane(void) const;
	FsGround *GetPlayerGround(void) const;
	int GetNumAirplaneLoaded(void);
	int GetNumPrimaryGroundTarget(FSIFF iff);

	YSBOOL IsFieldLoaded(void) const;
	YSRESULT GetLoadedField(YsString &fieldName,YsVec3 &pos,YsAtt3 &att);

	void UnprepareAllTemplate(void);
	void UnprepareAirplaneTemplate(void);

	YSRESULT SetWind(const YsVec3 &wind);
	YSRESULT GetWind(YsVec3 &wind) const;
	YSRESULT SetFogVisibility(const double &visi);
	double GetFogVisibility(void) const;
	YSRESULT SetFog(YSBOOL fog);
	YSBOOL GetFog(void) const;
	double GetFieldElevation(const double &x,const double &z) const;
	YSRESULT AddOvercastLayer(const double &flr,const double &top);

	int RerecordByNewInterval(const double &itvl);
	void AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng);

	const FsAirplaneTemplate *GetAirplaneTemplate(const char idName[]) const;



protected:
	YsListItem <FsAirplaneTemplate> *FindAirplaneTemplate(const char idName[]) const;
	YSRESULT PrepareAirplaneVisual(YsListItem <FsAirplaneTemplate> *templ) const;
	YsListItem <FsGroundTemplate> *FindGroundTemplate(const char idName[]) const;
	YSRESULT PrepareGroundVisual(YsListItem <FsGroundTemplate> *templ) const;
	YsListItem <FsFieldTemplate> *FindFieldTemplate(const char idName[]) const;
	YSRESULT PrepareFieldVisual(YsListItem <FsFieldTemplate> *templ) const;

	YSRESULT LoadInternal(const wchar_t fn[],const YsVec3 &pos,const YsAtt3 &att);
};

/* } */
#endif
