#ifndef FSSIMULATION_IS_INCLUDED
#define FSSIMULATION_IS_INCLUDED
/* { */

//NOTICE:
//FsSpecialDrawingForAircraftCarrier is added.
//This must be considered in SimDrawAirplane and
//SimDrawGround functions. However, currently
//only SimDrawAircraft function is considering
//this feature. In the future, when some ground
//object is loaded to the aircraft carrier,
//this point must be considered.

#include <memory>

#include <ysclass11.h>

#include "fsdef.h"
#include "fsparticle.h"
#include "fsweather.h"

#include "fsfield.h"

#include "fshud2.h"

#include "fswindow.h" // class FsJoystick
#include "fscontrol.h"
#include "fsexplosion.h"


#include "fssubmenu.h"

#include "fssiminfo.h"

#include "fsatc.h"

#include "graphics/common/fsconsole.h"

#include "fssimextension.h"

class FsWorld;



////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

#include "fslattice.h"

// Declaration /////////////////////////////////////////////

extern YsListAllocator <class FsAirplane> FsAirplaneAllocator;
extern YsListAllocator <class FsGround> FsGroundAllocator;

class FsSpecialDrawingForAircraftCarrier
{
public:
	FsExistence *obj;
	FsGround *carrier;
};

class FsTimedMessage
{
public:
	FsTimedMessage();
	double timeRemain;
	YsWString wstr;
};

class FsHasInFlightDialog
{
private:
	class FsGuiInFlightDialog *currentInFlightDlg;
public:
	FsHasInFlightDialog();
	void SetCurrentInFlightDialog(FsGuiInFlightDialog *dlg);
	class FsGuiInFlightDialog *GetCurrentInFlightDialog(void) const;
};

class FsSimulation : public FsHasInFlightDialog
{
private:
	YsString simTitle;

	FsSimulation(const FsSimulation &);
	FsSimulation &operator=(const FsSimulation &);

	FsAirTrafficController primaryAtc;
	FsAirTrafficSequence *airTrafficSequence=nullptr;;

	mutable YsThreadPool threadPool;

	YsArray <std::shared_ptr <FsSimExtensionBase> > addOnList;

public:
	enum FSSIMULATIONSTATE
	{
		FSSIMSTATE_CENTERJOYSTICK,
		FSSIMSTATE_INITIALIZE,
		FSSIMSTATE_RUNNING,
		FSSIMSTATE_CHECKCONTINUE,
		FSSIMSTATE_TERMINATING,
		FSSIMSTATE_OVER
	};

	enum FSVIEWMODE
	{
		FSCOCKPITVIEW,
		FSOUTSIDEPLAYERPLANE,
		FSFIXEDPOINTPLAYERPLANE,
		FSVARIABLEPOINTPLAYERPLANE,
		FSFROMTOPOFPLAYERPLANE,
		FSANOTHERAIRPLANE,
		FSMISSILEVIEW,
		FSAIRTOAIRVIEW,
		FSAIRFROMAIRVIEW,
		FSPLAYERPLANEFROMSIDE,
		FSCARRIERVIEW,
		FSTESTVIEW1,
		FSTESTVIEW2,
		FSOUTSIDEPLAYER2,
		FSOUTSIDEPLAYER3,
		FSBOMBINGVIEW,
		FSTOWERVIEW,
		FSPLAYERTOGNDVIEW,
		FSGNDTOPLAYERVIEW,
		FSSPOTPLANEVIEW,

		FSMYWEAPONVIEW_OLD,  // For sub window
		FSMYWEAPONVIEW_NEW,
		FSBACKMIRRORVIEW,
		FS45DEGREERIGHTVIEW,
		FS45DEGREELEFTVIEW,
		FS90DEGREERIGHTVIEW,
		FS90DEGREELEFTVIEW,
		FSTELESCOPEVIEW,
		FSLOCKEDTARGETVIEW,
		FSGHOSTVIEW,

		FSAIRTOTOWERVIEW,
		FSAIRTOTOWERVIEWSOLO,
		FSTOWERVIEW_NOMAGNIFY,

		FSVERTICALORBITINGVIEW,         // 2005/06/07
		FSHORIZONTALORBITINGVIEW,       // 2005/06/07
		FSTURNVIEW,                     // 2005/06/07

		FSADDITIONALAIRPLANEVIEW,       // 2006/07/19 For additional view in cockpit
		FSADDITIONALAIRPLANEVIEW_CABIN  // 2011/02/01 For additional view in cabin
	};
	enum FSREPLAYMODE
	{
		FSREPLAY_PLAY,
		FSREPLAY_BACKWARD,
		FSREPLAY_FASTFORWARD,
		FSREPLAY_VERYFASTFORWARD,
		FSREPLAY_FASTREWIND,
		FSREPLAY_VERYFASTREWIND,
		FSREPLAY_PAUSE,
		FSREPLAY_STEPFORWARD,
		FSREPLAY_STEPBACK,
	};
	enum
	{
		FSNTIMEMARKER=16
	};
	enum
	{
		FSNUMTIMEDMESSAGE=10
	};
	enum FSINSTRUMENTDRAWSWITCH
	{
		FSISS_NOINSTRUMENT=0,
		FSISS_2DHUD=1,
		FSISS_2DINSTPANEL_NOT_USED_ANY_MORE=2,
		FSISS_3DHUD=4,
		FSISS_3DINSTPANEL=8
	};

	class ViewModeAndIndex
	{
	public:
		FSVIEWMODE viewmode;
		int refIndex;

		void Set(FSVIEWMODE m,int r)
		{
			viewmode=m;
			refIndex=r;
		}
	};
	class ViewModeAndIndexAndPosition : public ViewModeAndIndex
	{
	public:
		YsVec3 pos;

		void Set(FSVIEWMODE m,int r,const YsVec3 &p)
		{
			viewmode=m;
			refIndex=r;
			pos=p;
		}
	};

	class ReplayInfo
	{
	public:
		double beginTime;
		YSBOOL editMode;
		YSBOOL resumed;

		// The following two will be set in RunReplaySimulationOneStep when the state is FSSIMSTATE_INITIALIZE
		FsExistence *originalPlayerObject;

		void Initialize(const double beginTimeIn,YSBOOL editModeIn);
	};

	class PlayerVehicleHistoryInfo
	{
	public:
		const FsExistence *vehicle;
		double tStart,tEnd;
	};



	class FsWorld *world;

	YSBOOL showReplayDlg;
	class FsReplayDialog *replayDlg;
	class FsContinueDialog *contDlg;

	// In-Flight Dialogs >>
	class FsGuiChooseAircraft *loadingDlg;  // Loading Dialog is opened in SimProcessButtonFunction (FSBTF_SUPPLYDIALOG)
	class FsGuiChatDialog *chatDlg;
	class FsGuiStationaryDialog *stationaryDlg;
	class FsGuiVehicleChangeDialog *vehicleChangeDlg;
	class FsGuiAutoPilotDialog *autoPilotDlg;
	class FsGuiAutoPilotVTOLDialog *autoPilotVTOLDlg;
	class FsGuiAutoPilotHelicopterDialog *autoPilotHelicopterDlg;
	class FsGuiRadioCommFuelTruckDialog *callFuelTruckDlg;
	class FsGuiRadioCommTargetDialog *radioCommTargetDlg;
	class FsGuiRadioCommCommandDialog *radioCommCmdDlg;
	class FsGuiRadioCommToFormationDialog *radioCommToFomDlg;
	class FsGuiAtcRequestDialog *atcRequestDlg;
	class FsGuiSelectApproachDialog *requestApproachDlg;
	// In-Flight Dialogs <<

	class FsCenterJoystick *centerJoystick;

	double currentTime,endTime;
	double aircraftTroubleTimer;

	mutable int nFrameForFpsCount;
	mutable YSSIZE_T lastFpsUpdateTime,nextFpsUpdateTime;
	mutable double fps;

protected:
	char systemMessage[256];
	mutable FsTimedMessage timedMessage[FSNUMTIMEDMESSAGE];

	class YsBitmap *escTwiceToEnd,*escOnceToEnd;

	YSBOOL showUserNameMasterSwitch;

	class FsLocalUser *localUser;
	YsListContainer <FsAirplane> airplaneList;
	YsListContainer <FsGround> groundList;

	YSBOOL fieldLoaded;
	FsField field;

	YsHashTable <FsAirplane *> *airplaneSearch;
	YsHashTable <FsGround *> *groundSearch;

	FsLattice ltc;

	double tallestGroundObjectHeight; // Updated everytime ground object moves

	double nextAirRecordTime,nextGndRecordTime;
	double nextControlTime,lastControlledTime;

	int escKeyCount;
	YSBOOL pause;
	YSBOOL canContinue;

	FSENVIRONMENT env;
	YsColor fogColor;
	YsColor skyColor,gndColor;
	YSBOOL gndSpecular;

	class ActualViewMode
	{
	public:
		FSVIEWMODE actualViewMode;
		double actualViewHdg,actualViewPch;
		YsVec3 viewPoint;
		YsAtt3 viewAttitude;
		YsMatrix4x4 viewMat;
		double viewMagFix;
		YSBOOL isViewPointInCloud;
		double fogVisibility;

		double viewTargetDist;

		enum
		{
			NUM_SHADOW_MAP=3
		};
		YsMatrix4x4 shadowProjMat[NUM_SHADOW_MAP];
		YsMatrix4x4 shadowViewMat[NUM_SHADOW_MAP];

		ActualViewMode();
	};

	FSREPLAYMODE replayMode;
	FSVIEWMODE mainWindowViewmode;
	int mainWindowAdditionalAirplaneViewId;
	const FsAirplane *focusAir,*focusAir2;
	const FsGround *focusGnd;
	int towerViewId;
	YsVec3 towerViewPos;

	ActualViewMode mainWindowActualViewMode;
	ActualViewMode subWindowActualViewMode[FsMaxNumSubWindow];
	// mutable FSVIEWMODE actualViewmode;
	// mutable double actualViewHdg,actualViewPch;
	// mutable double viewMagFix;
	// YSBOOL isViewPointInCloud;
	// double fogVisibility;

	double ghostViewSpeed;
	YsVec3 viewRefPoint;
	YsAtt3 viewAttitudeTransition;
	double viewMagUser;  // viewMagFix: Depends on view mode  viewMagUser: User control
	mutable double nearZ,farZ,tanFov;
	YsAtt3 relViewAtt;
	double relViewDist;
	YsArray <YsVec3> towerPosition;  // Must be set in AddField, Tower->focusAirId,
	                                 // FsField must store tower positions

	FSVIEWMODE subWindowViewmode[FsMaxNumSubWindow];

	YSBOOL terminate;

	unsigned int allowedWeaponType;

	class FsHeadUpDisplay *hud;
	class FsHud2 *hud2;
	class FsGroundSky *groundSky;

	unsigned long long lastTime;

	FsJoystick pJoy[FsMaxNumJoystick];
	FsFlightControl userInput;
	FsSubMenu subMenu;

	FsWeaponHolder bulletHolder;
	FsExplosionHolder explosionHolder;
	FsParticleStore particleStore;

	class FsWeather *weather,*iniWeather;
	class FsClouds *cloud;
	class FsSolidClouds *solidCloud;

	class FsMissionGoal *goal;

	class FsSimulationEventStore *simEvent;

	// firstPlayer is nullified in the constructor,
	// It is set only two places:
	//   In SimulateOneStep when firstPlayer.GetObject() is NULL and GetPlayerObject is not NULL.
	//   In FsWorld::Load when it encounters INITPLYR tag.
	// This information is used during the preparation of the flight-record replay.
	FsSimInfo::ObjRef firstPlayer;


	YsArray <class FsGround *> aircraftCarrierList;  // Still used for generating ground object in the middle.
	YsArray <class FsGround *> supplyList;
	class FsGround *ilsToGo;  // <- for Communication,Return to Base
	YsArray <class FsGround *> vorList,ndbList;

// Customizable Factor
	class FsFlightConfig *cfgPtr;

// For landing gear / flap sound
	double pGear,ppGear,Gear;
	YSBOOL hideNextGearSound;
	double pFlap,ppFlap,Flap;

// For flight record editing
	double timeMarker[FSNTIMEMARKER];

// For network play
protected:
	class FsSocketServer *netServer;
	class FsSocketClient *netClient;

public:
	FsControlAssignment ctlAssign;



public:
	FsSimulation(class FsWorld *world);
	~FsSimulation();

	static const char *ViewmodeToStr(FSVIEWMODE viewmode);
	static FSVIEWMODE StrToViewmode(const char *str);

	void SetSimulationTitle(const char str[]);

	void SetCanContinue(YSBOOL canContinue);
	YSBOOL GetCanContinue(void) const;

	void CreateCenterJoystick(void);
	void DeleteCenterJoystick(void);
	void CenterJoystickOneStep(FSSIMULATIONSTATE &simState);
	void CenterJoystickDraw(void) const;

	YSBOOL Paused(void) const;
	FsWorld *GetWorldPtr(void);
	unsigned int GetAllowedWeaponType(void) const;
	double CurrentTime(void) const;
	void RegisterExtension(std::shared_ptr <FsSimExtensionBase> addOnPtr);
	std::shared_ptr <class FsSimExtensionBase> FindExtension(const YsString &str) const;

	YSRESULT TestAircraftCarrierDataIntegrity(void) const;

	YSRESULT Save(FILE *fp,
	     int airPosPrecision,int airAttPrecision,
	     int gndPosPrecision,int gndAttPresicion,
	     int wpnPosPrecision,int wpnAttPrecision,
	     const double &timeStep);
	YSRESULT LoadWeaponRecord(FILE *fp);
	YSRESULT LoadExplosionRecord(FILE *fp);
	YSRESULT LoadCloud(FILE *fp);
	YSRESULT LoadSolidCloud(FILE *fp);
	YSRESULT LoadWeather(FILE *fp);
	YSRESULT LoadSimulationEvent(FILE *fp);

	YSRESULT PrepareSimulationEvent(void);

	YSBOOL IsMissionGoalSet(void) const;
	YSRESULT SetMissionGoal(const class FsMissionGoal &missionGoal);
	const class FsMissionGoal &GetMissionGoal(void) const;

	/*  Change of player airplane/ground will not be recorded if:
	      SetPlayerSomething is called from the event play-back, or
	      SetPlayerSomething is called when terminating replay mode (for making the player plane back to the first player plane)
	   */
	void RecordPlayerChange(const FsExistence *newPlayer);
	YSRESULT SetPlayerAirplane(const FsAirplane *air,YSBOOL record=YSTRUE);
	YSRESULT SetPlayerGround(const FsGround *gnd,YSBOOL record=YSTRUE);
	YSRESULT SetPlayerObject(const FsExistence *obj,YSBOOL record=YSTRUE);
	YSRESULT GetPlayerVehicleHistory(YsArray <PlayerVehicleHistoryInfo> &playerHist) const;
	void SetFirstPlayerYfsIdent(FSEXISTENCETYPE objType,int yfsIdent);
	void ResetToFirstPlayerObject(void);

	FsAirplane *AddAirplane(FsAirplane &airplane,YSBOOL isPlayerPlane,const wchar_t tmplRootDir[],unsigned netSearchKey=0);
	FsGround *AddGround(FsGround &ground,YSBOOL isCarrier,const wchar_t tmplRootDir[],const wchar_t acpFn[],unsigned netSearchKey=0);
	YSRESULT DeleteAirplane(FsAirplane *air);
	YSRESULT DeleteGround(FsGround *gnd);
	FsField *SetField(FsField &field,const YsVec3 &pos,const YsAtt3 &att);
	void SetFogColor(YsColor col);
	void SetSkyColor(YsColor col);
	void SetGroundColor(YsColor col);
	YSRESULT CheckStartPositionIsAvailable(int fieldId,const char stpIdName[]);

	// Preliminary check.  Not a strict check.
	YSBOOL SimTestCollision(const FsVisualSrf &shl,const YsVec3 &pos,const YsAtt3 &att);


	void SetEnvironment(FSENVIRONMENT env);
	void EnforceEnvironment(void);
	FSENVIRONMENT GetEnvironment(void) const;

	YSRESULT SendConfigString(const char str[]);


	// In the future
	// Be careful that most part is written assuming airplane and
	// ground is not deleted.
	// And, be careful about aircraftCarrierList (at the bottom of this class def)


	int GetNumAircraftCarrier(void) const;
	const FsGround *GetAircraftCarrier(YSSIZE_T i) const;


	void PrepareReplaySimulation(void);
	void DeleteEventByTypeAll(int eventType);

	/*! Call this function to erase first-player object info.  When flying in a flight record using Select Airplane,
	    this information must be removed, or "Retry Previous Flight" will reset the player to the first player object in FsWorld::LoadInternal.
	*/
	void ClearFirstPlayer(void);

	void AddReplayDialog(void);
	void AddEditDialog(void);
	void EraseReplayDialog(void);

	void CenterJoystick(void);
	YSRESULT MakeCenterJoystickDialog(class FsCenterJoystick &centerJoystickDialog,int nextActionCode);
	static YSRESULT TestAssignJoystick(void);
	static YSBOOL NeedNoJoystickWarningDialog(void);
	YSRESULT CheckJoystickAssignmentAndFixIfNecessary(void);
	void ClearKeyBuffer(void);

	// External Control
	void SetReplayMode(FSREPLAYMODE replMode);
	void FastForward(const double &targetTime);
	void DeleteFlightRecord(const double &t1,const double &t2);

	// Network play (fsnetwork.cpp)
	YSBOOL NetActivity(void);
	void NetFreeMemoryWhenPossible
	    (double &netNoActivityTime,const double &passedTime,FsSocketServer *server);

	YSRESULT ServerJoin(class FsSocketServer &svr,class FsGuiChooseAircraft &air,const char startPos[]);
	YSRESULT ServerState_StandBy
	   (const double &passedTime,
	    class FsSocketServer &svr,
	    FsGuiChooseAircraft &chooseAirplane,
	    FsGuiChooseField &chooseStartPosition,
	    class FsChoose &dispellUser,
	    class FsGuiServerDialog &svrDlg);
	YSRESULT ServerState_Flying(const double &passedTime,class FsSocketServer &svr);
	YSRESULT ServerState_GhostMode(const double &passedTime,class FsSocketServer &svr);
	YSRESULT ServerState_DispellUser(const double &passedTime,class FsSocketServer &svr,FsChoose &dispellUser);
	void RunServerModeTest(const char name[],const char fldName[],FsNetConfig &netcfg);
	void RunServerModeOneStep(FsServerRunLoop &svrSta);

	YSRESULT ClientState_StandBy
	   (const double &passedTime,
	    class FsSocketClient &cli,FsGuiChooseAircraft &chooseAirplane,FsGuiChooseField &chooseStartPosition,
	    class FsGuiClientDialog &cliDlg);
	YSRESULT ClientState_Flying(const double &passedTime,class FsSocketClient &cli);
	YSRESULT ClientState_GhostMode(const double &passedTime,class FsSocketClient &cli);
	YSRESULT ClientState_SideWindow(const double &passedTime,class FsSocketClient &cli);

	void RunClientModeOneStep(class FsClientRunLoop &cliSta);


	// For modal simulation
	void RunSimulationOneStep(FSSIMULATIONSTATE &simState);

	YSBOOL CheckInterceptMissionAvailable(void) const;
	YSBOOL CheckCloseAirSupportMissionAvailable(void) const;
	void TestSolidCloud(void);
	void MakeSolidCloud
	   (int n,const YsVec3 &cen,const double &range,const double &sizeH,const double &y0,const double &y1);
	void SetFog(YSBOOL drawFog,const double &visibility);

	void RunReplaySimulationOneStep(FSSIMULATIONSTATE &state,ReplayInfo &replayInfo);
	void SimResetPlayerObjectFromRecord(const double &t0,const double &t1);
	void SimSeekNextEventPointereToCurrentTime(const double currentTime);
	void RefreshOrdinance(void);
	void RefreshOrdinanceByWeaponRecord(const double &currentTime);

	YSRESULT PrepareRunDemoMode(FsDemoModeInfo &info,const char sysMsg[],const double &maxTime);
	YSBOOL DemoModeOneStep(FsDemoModeInfo &info,YSBOOL drawSmokeVapor,YSBOOL preserveFlightRecord);
	YSBOOL ConcordeFlyByOneStep(FsDemoModeInfo &info,YSBOOL record);
	YSRESULT AfterDemoMode(FsDemoModeInfo &info);
	YSRESULT DemoModeReconsiderViewTarget(const FsAirplane *&fromAirplane,const FsAirplane *&toAirplane);
	YSRESULT DemoModeReconsiderPlayerAirplane(const FsAirplane *fromAirplane,const FsAirplane *toAirplane);
	YSRESULT DemoModeReconsiderLandingViewMode(FsDemoModeInfo &info);
	YSRESULT DemoModeReconsiderAcrobatViewMode(FsDemoModeInfo &info);
	YSRESULT DemoModeReconsiderConcordeFlyByViewMode(FsDemoModeInfo &info);
	YSRESULT DemoModeSetAcrobatViewModeTowerToAirplane(FsDemoModeInfo &info,FsAirplane *air);
	YSRESULT DemoModeSetAcrobatViewModeAirplaneToTower(FsDemoModeInfo &info,FsAirplane *air);
	YSRESULT DemoModeSetAcrobatViewModeCockpitView(FsDemoModeInfo &info,FsAirplane *air);
	YSRESULT DemoModeSetAcrobatViewModeOutsideView3(FsDemoModeInfo &info,FsAirplane *air);
	YSRESULT DemoModeRipOffEarlyPartOfRecord(void);
	YSBOOL DemoModeOneSideWon(void);
	// Or for step by step simulation
	void PrepareRunSimulation(void);
	void PrepareContinueSimulation(void);
	void SetTerminate(YSBOOL termi);

	void AssignUniqueYsfId(void);
	int GetLargestYsfId(void) const;

	void ReadControlBackFromAirplane(const class FsAirplane *air);

	void SimulateOneStep
	    (const double &dt,
	     YSBOOL demoMode,YSBOOL record,YSBOOL showTimer,YSBOOL networkStandby,FSUSERCONTROL userControl,
	     YSBOOL showTimeMarker);
	void DecideAllViewPoint(const double dt);
	void AfterSimulation(void);


	YSBOOL needRedraw;
	void SetNeedRedraw(YSBOOL needRedraw);
	YSBOOL NeedRedraw(void) const;

	void DrawInNormalSimulationMode(FsSimulation::FSSIMULATIONSTATE simState,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker) const;
	void DrawInClientMode(const class FsClientRunLoop &clientModeRunLoop) const;
	void DrawInServerMode(const class FsServerRunLoop &serverModeRunLoop) const;


	void GenerateEnemyAirplane(
	    int &nEnemy,double &gLimit,const double &timeRemain,YSBOOL allowAAM,YSBOOL jet,YSBOOL ww2);
	void GenerateAttackerAirplane(
	    int nEnemy,const class FsMissionEnemyGroundAttackerInfo &info,const double &iniDistFixedWing,const double &iniDistRotorWing,
	    YSBOOL reducedAttackDist,const double initialSpeed);
	void GenerateFriendlyAirplane(FsInterceptMissionInfo &info);
	void GenerateTank
	   (int nTank,FsCloseAirSupportMissionInfo &info,
	    YSBOOL mobile,YSBOOL incremental,YSBOOL primaryTarget,
	    FSIFF iff,int regionCode,int strength);
	void ReviveGround(void);
	void ResetAirplane(FsAirplane *air);
	void ResetGround(FsGround *gnd);

	const char *FinalAirplaneStateString(FSFLIGHTSTATE sta) const;

	void ClearTimedMessage(void);
	void ClearUserInterface(void);
	void AddTimedMessageWithNoLog(const char str[]) const;
	void AddTimedMessage(const char str[]);
	void AddEvent(const class FsSimulationEvent &evt);


	const double &GetClock(void);
	FsAirplane *GetPlayerAirplane(void);
	const FsAirplane *GetPlayerAirplane(void) const;
	FsGround *GetPlayerGround(void);
	const FsGround *GetPlayerGround(void) const;
	FsExistence *GetPlayerObject(void);
	const FsExistence *GetPlayerObject(void) const;

	FsExistence *FindObject(YSHASHKEY searchKey);
	const FsExistence *FindObject(YSHASHKEY searchKey) const;

	int GetNumAirplane(void) const;
	FsAirplane *FindNextAirplane(const FsAirplane *ptr) const;
	FsAirplane *FindPrevAirplane(const FsAirplane *ptr) const;
	FsAirplane *GetAirplaneById(int id) const;
	FsAirplane *FindAirplane(YSHASHKEY searchKey) const;
	int GetAirplaneIdFromHandle(FsExistence *air) const;
	FsAirplane *FindAirplaneByYsfId(int id) const;
	FsAirplane *FindAirplaneByName(const char name[]) const;
	const FsAirplane *FindFirstAliveAirplane(const FsAirplane *afterThis) const;

	int GetNumGround(void);
	int GetNumPrimaryGroundTarget(FSIFF iff) const;
	FsGround *FindNextGround(const FsGround *ptr) const;
	FsGround *FindPrevGround(const FsGround *ptr) const;
	FsGround *GetGroundById(int id) const;
	FsGround *FindGround(YSHASHKEY) const;
	FsGround *FindGroundByTag(const char tag[]) const;
	int GetGroundIdFromHandle(FsExistence *gnd) const;
	FsGround *FindGroundByYsfId(int id) const;
	const FsGround *FindVorByTag(const char tag[]) const;
	const FsGround *FindNdbByTag(const char tag[]) const;
	const FsGround *FindIlsOrCarrierByTag(const char tag[]) const;

	int GetNumVOR(void) const;
	const FsGround *GetVOR(int idx) const;
	int GetNumNDB(void) const;
	const FsGround *GetNDB(int idx) const;

	YSRESULT GetAirBasePosition(YsVec3 &pos,const FsSimInfo::AirBase &base) const;

	int GetNumILSFacility(void) const;
	FsGround *GetILS(int id) const;
	YsArray <const FsGround *> FindILSinRectRegion(const YsSceneryRectRegion *rgn) const;
	YsArray <const YsSceneryPointSet *> FindVFRApproachInRectRegion(const YsSceneryRectRegion *rgn) const;
	const FsILS *FindIlsFromPosition(const YsVec3 &pos,const YsAtt3 &att) const;
	int GetNumTowerView(void) const;
	const YsVec3 &GetTowerView(int towerViewId) const;

	int GetNumSupplyVehicle(void) const;
	FsGround *GetSupplyVehicle(int idx) const;

	const class FsAirTrafficController *FindAirTrafficController(unsigned int searchKey) const;
	class FsAirTrafficController *FindAirTrafficController(unsigned int searchKey);

	const class FsAirTrafficController *FindAirTrafficControllerByName(const YsString &name) const;
	class FsAirTrafficController *FindAirTrafficControllerByName(const YsString &name);

	const class FsAirTrafficSequence &GetAirTrafficSequence(void) const;
	class FsAirTrafficSequence &GetAirTrafficSequence(void);

	const FsField *GetField(void) const;
	const YsSceneryPointSet *SearchMotionPathByTag(const char tag[]) const;

	YSRESULT SetAllowedWeaponType(unsigned int allowedWeaponType);
	void AllowGun(YSBOOL a);
	void AllowAAM(YSBOOL a);
	void AllowAGM(YSBOOL a);
	void AllowBomb(YSBOOL a);
	void AllowRocket(YSBOOL a);

	YSRESULT SendCommand(char cmd[]);
	YSRESULT LoadConfigFile(const wchar_t fn[],YSBOOL changeEnvironment);

	YSBOOL EveryAirplaneIsRecordedAirplane(void) const;
	YSBOOL AtLeastOneAirplaneIsRecordedAirplane(void);
	double LastRecordedTime(void) const;

	YSRESULT GetLoadedField(YsString &fieldName,YsVec3 &pos,YsAtt3 &att);
	YSRESULT GetRunwayRectFromPosition(const YsSceneryRectRegion *&rgn,YsVec3 rect[4],const YsVec3 &pos) const;
	const YsSceneryRectRegion *FindAirportFromPosition(const YsVec3 &pos) const;

	template <const int N>
	inline YSRESULT GetRunwayRectFromPositionAll(YsArray <const YsSceneryRectRegion *,N> &rgnArray,const YsVec3 &pos) const;

	template <const int N>
	inline YSRESULT GetRegionRectFromPositionAll(YsArray <const YsSceneryRectRegion *,N> &rgnArray,const YsVec3 &pos) const;

public:
	YSRESULT FindRunwayClearingPath(YsArray <YsVec3,16> &rwClearPath,const FsExistence &obj);

	// Returns path candidates and distances to the first point of the path.
	YSRESULT FindRunwayClearingPathCandidate(
	    YsArray <YsPair <const YsSceneryPointSet *,double>,16> &pathCan,
	    const FsExistence &obj) const;

private:

public:
	double GetFieldElevation(const double &x,const double &z) const;
	double GetBaseElevation(void) const;
	void GetFieldElevationAndNormal(double &elv,YsVec3 &nom,const double &x,const double &z);
	double GetFieldMagneticVariation(void) const;

	double InternalHeadingToTrueHeading(const double internalHeading) const;
	double TrueHeadingToInternalHeading(const double internalHeading) const;
	double TrueHeadingToMagneticHeading(const double trueHeading) const;
	double InternalHeadingToMagnetic(const double trueHeading) const;

	double MagneticHeadingToTrueHeading(const double magHeading) const;
	double MagneticHeadingToInternalHeading(const double magHeading) const;

	YSSCNAREATYPE GetAreaType(const YsVec3 &pos);
	YSRESULT GetRegionRect(YsVec3 rect[4],const YsSceneryRectRegion *rgn) const;
	YSRESULT GetRegionRectById(YsVec3 rect[4],int id) const;
	YSRESULT GetRegionRectByTag(YsVec3 rect[4],const char tag[]) const;
	YSRESULT GetRegionCenterById(YsVec3 &cen,int id) const;
	YSRESULT GetRegionCenterByTag(YsVec3 &cen,const char tag[]) const;
	static void AdjustStartPositionByRunwayRect(
	    YsVec3 &pos,const YsVec3 rwRect[],const YsVec3 &offsetFromRwThr,const YsAtt3 &att);

	void MakeSortedIlsList(YsArray <FsGround *,64> &ilsList,YsArray <double,64> &ilsDist,const YsVec3 &refp) const;
	void MakeSortedVorList(YsArray <FsGround *,64> &vorList,YsArray <double,64> &vorDist,const YsVec3 &refp) const;
	void MakeSortedNdbList(YsArray <FsGround *,64> &ndbList,YsArray <double,64> &ndbDist,const YsVec3 &refp) const;

	const class FsWeather &GetWeather(void) const;
	class FsWeather &GetWeather(void);

	template <const int N>
	inline const FsGround *FindIlsBestForWindFromCandidateArray(const YsArray <const FsGround *,N> &ilsArray) const;
private:
	template <const int N>
	inline const FsGround *FindIlsBestForWindFromCandidateArray(
	    const YsArray <const FsGround *,N> &ilsArray,
	    const YsArray <YsVec3,N> &tdVecArray,
	    const YsVec3 &unitWind) const;

public:
	// Returns an index.  vfrArray[ret]=tdPos vfrArray[ret+1]=tdVec
	template <const int N>
	inline YSSIZE_T FindVisualApproachBestForWindFromCandidateArray(const YsArray <YsVec3,N> &vfrArray) const;

	template <const int N>
	inline const YsSceneryPointSet *FindVisualApproachBestForWindFromCandidateArray(const YsArray <const YsSceneryPointSet *,N> &vfrArray) const;
private:
	template <const int N>
	inline YSSIZE_T FindVisualApproachBestForWindFromCandidateArray(const YsArray <YsVec3,N> &tdVecArray,const YsVec3 &unitWind) const;

	void ConstrainRunwayVectorFromRunwayRect(YsVec3 &rwVec,const YsVec3 &tdPos) const;

public:
	/*! This function finds threats between points A and B. 
	    Note: This function is altitude aware.  Set altitude of A and B as the flight path. */
	void FindGroundToAirThreat(YsArray <FsSimInfo::GndToAirThreat,16> &threatFound,const YsVec3 &A,const YsVec3 &B,const FsAirplane &air) const;


public:
	const class FsFlightConfig &GetConfig(void) const;
	void SetBlackOut(YSBOOL blackOut);
	void SetMidAirCollision(YSBOOL midAirCollision);
	void SetCanLandAnywhere(YSBOOL canLandAnywhere);
	void SetShowUserName(int showUserName);
	void SetDisableThirdAirplaneView(YSBOOL sw);

	FsWeaponHolder &GetWeaponStore(void);
	const FsWeaponHolder &GetWeaponStore(void) const;
	const FsWeapon *FindNextActiveWeapon(const FsWeapon *wpn) const;
	YSBOOL IsWeaponGuidedToTarget(int weaponId) const;
	YSBOOL IsWeaponShotBy(int weaponId,FsAirplane *air) const;

	YSBOOL IsPlayerAirplane(const FsAirplane *air) const;

	YSBOOL IsOnRunway(const YsVec3 &pos) const;
	static YSBOOL IsSafeTerrainRegionId(int id);
	YSBOOL CheckRunwayLength(const YsVec3 &org,const YsVec3 &dir,const double &lng) const;
	YSBOOL IsRunwayClear(const YsVec3 &org,const YsVec3 &dir,const double &lng) const;
	FsGround *FindNearbySupplyTruck(YSBOOL &fuel,YSBOOL &ammo,const FsExistence &air) const;
	FsGround *FindNearestSupplyTruckInTheSameRamp(const YsVec3 &pos) const;
	YSRESULT FindNearbyBoardableVehicle(YsArray <const FsExistence *,16> &objArray,const FsExistence &from) const;
	YSRESULT PlayerChangeVehicleIfPossible(int objSearchKey);
	YSRESULT FindTakeOffTaxiPathWithinReach(YsArray <const YsSceneryPointSet *> &taxiPathArray,const YsVec3 &pos) const;

	const class YsSceneryPointSet *FindTaxiPathBestForWindFromCandidateArray(const YsArray <const class YsSceneryPointSet *> &taxiPathArray) const;
private:
	const class YsSceneryPointSet *FindTaxiPathBestForWindFromCandidateArray(
	     const YsArray <const class YsSceneryPointSet *> &taxiPathArray,const YsArray <YsVec3> &takeOffDirArray,const YsVec3 &unitWind) const;

public:
	YSRESULT GetFormationCenter(YsVec3 &pos,FsAirplane &wingLeader) const;

	void RemakeLattice(void);
	const FsLattice &GetLattice(void) const;

	int RerecordByNewInterval(const double &itvl);
	void AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng);

	YSBOOL NeedToDrawInstrument(const ActualViewMode &ActualViewMode) const;
	YSBOOL NeedToDrawGameInfo(const ActualViewMode &ActualViewMode) const;
	unsigned int GetInstrumentDrawSwitch(const ActualViewMode &ActualViewMode) const;

protected:
	void ViewingControl(FSBUTTONFUNCTION fnc,FSUSERCONTROL userControl);
	YsArray <ViewModeAndIndexAndPosition> MakeAvailableILSView(void) const;
	YsArray <ViewModeAndIndexAndPosition> MakeAvailableTowerView(void) const;
	YsArray <const FsAirplane *> MakeAvailableViewTargetAirplane(YSBOOL includePlayer) const;

	double PassedTime(void);

	void SimMove(const double &deltaTime);
	void SimCheckTailStrike(void);

protected:
	void SimCacheFieldElevation(void);
	void UpdateGroundTerrainElevationAndNormal(FsGround *gndPtr);
	void SimCacheRectRegion(void);

public:
	void SimComputeAirToObjCollision(void);
	void SimProcessCollisionAndTerrain(const double &dt);

	/*! The following three MayCollide, MayCollideWithAir, and MayCollideWithGround will test bounding sphere collision between objects.
	    It does not consider bounding box, so the result is very conservative. */
	YSBOOL MayCollide(const YsVec3 &objPos,const double objRad,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const;
	YSBOOL MayCollideWithAir(const YsVec3 &objPos,const double objRad,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const;
	YSBOOL MayCollideWithGround(const YsVec3 &objPos,const double objRad,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const;

	/*! The following three MayCollide, MayCollideWithAir, and MayCollideWithGround will test bounding box collision between objects.
	    It does not consider bounding box, so the result is very conservative. */
	YSBOOL MayCollide(const YsVec3 &objPos,const YsAtt3 &objAtt,const double clearance,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const;
	YSBOOL MayCollideWithAir(const YsVec3 &objPos,const YsAtt3 &objAtt,const double clearance,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const;
	YSBOOL MayCollideWithGround(const YsVec3 &objPos,const YsAtt3 &objAtt,const double clearance,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const;
protected:
	void UpdateGroundCarrierLoading(FsGround *gnd1,const YSSIZE_T nCarrierCandidate,FsGround *carrierCandidate[]);

public:
	void KillCallBack(FsExistence &obj,const YsVec3 &pos);
protected:
	void SimCheckEndOfSimulation(void);
	void SimCheckEndOfFlightRecord(void);
	void SimControlByUser(const double &dt,FSUSERCONTROL userControl);
	void SimPlayerAircraftGetTrouble(void);
	void SendAircraftRandomTrouble(FsAirplane &air,int reliability,YSBOOL record);
	void SendAircraftTrouble(FsAirplane &air,FSAIRCRAFTTROUBLE trouble,YSBOOL record);
	void ClearAircraftTrouble(FsAirplane &air,FSAIRCRAFTTROUBLE trouble,YSBOOL record);
	void SimProcessJoystickTrigger(const FsJoystick joy[],FSUSERCONTROL userControl);
	void SimProcessRawKey(int rawKey);  // Replay-mode and also test keys.
	void SimProcessChatMode(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my);
	void SimProcessGhostView(const double dt);
	void SimProcessLoadingDialog(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my);
	void SimProcessSubMenu(int rawKey);
public:
	void SimProcessButtonFunction(FSBUTTONFUNCTION fnc,FSUSERCONTROL userControl);
protected:
	void SimControlByComputer(const double &dt);
	void SimMakeUpCockpitIndicationSet(class FsCockpitIndicationSet &cockpitIndicationSet) const;
	void SimDrawAllScreen(YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker) const;
	void SimDrawScreen(const double &dt,const FsCockpitIndicationSet &cockpitIndicationSet,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker,const ActualViewMode &actualViewMode) const;
	void SimDrawShadowMap(const ActualViewMode &actualViewMode) const;
	void SimDrawGuiDialog(void) const;
	void SimDrawScreenZBufferSensitive(
		const FsCockpitIndicationSet &cockpitIndicationSet,
		const class YsGLParticleManager &particleMan,
		const ActualViewMode &actualViewMode,
		class FsProjection &proj) const;

	void SimAutoViewChange(FSVIEWMODE mainWindowViewMode,const double dt);
	void SimDecideViewpointAndCheckIsInCloud(ActualViewMode &actualViewMode,FSVIEWMODE viewmode,YsVec2i drawingAreaSize) const;
	void SimDecideViewpoint(ActualViewMode &actualViewMode,FSVIEWMODE viewmode) const;
	void SimDecideViewpoint_Air(ActualViewMode &actualViewMode,FSVIEWMODE viewmode,const FsAirplane *playerPlane) const;
	void SimDecideViewpoint_Gnd(ActualViewMode &actualViewMode,FSVIEWMODE viewmode,const FsGround *playerGround) const;
	void SimDecideViewpoint_Common(ActualViewMode &actualViewMode,FSVIEWMODE viewmode) const;
	YSBOOL CheckNoExtAirView(void) const;
	class FsProjection SimDrawPrepare(const ActualViewMode &) const;
	class FsProjection SimDrawPrepareBackground(const ActualViewMode &actualViewMode) const;
	class FsProjection SimDrawPrepareRange(const ActualViewMode &actualViewMode,const double &nearZ,const double &farZ) const;
	class FsProjection SimDrawPrepareNormal(const ActualViewMode &actualViewMode) const; // OpenGL Only
	void SimDrawBackground(const ActualViewMode &actualViewMode,const FsProjection &proj) const;
	void SimDrawMap(const ActualViewMode &actualViewMode,const FsProjection &prj,const double &elvMin,const double &elvMax) const;
	void SimDrawJoystick(const ActualViewMode &actualViewMode) const;
	void SimDrawForeground(const ActualViewMode &actualViewMode,const class FsProjection &proj,const FsCockpitIndicationSet &cockpitIndicationSet,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker) const;
	void SimDrawAircraftInterior(const ActualViewMode &actualViewMode,const class FsProjection &proj,const FsAirplane *air,unsigned int instDrawSwitch,const FsCockpitIndicationSet &cockpitIndicationSet) const;
	void SimDrawGroundInterior(const ActualViewMode &actualViewMode,const class FsProjection &proj,const FsGround *gnd,const class FsCockpitIndicationSet &cockpitIndicationSet) const;
	void SimDrawRadar(const ActualViewMode &actualViewMode) const;
	void SimDrawInstPanel3d(const YsVec3 &fakeViewPos,const YsVec3 &localViewPos,const class FsCockpitIndicationSet &cockpitIndicationSet) const;
	void SimDrawHud3d(const YsVec3 &instViewPos,const YsAtt3 &instViewAtt,const FsCockpitIndicationSet &ias) const;
	void SimDraw2dVor1(const class FsCockpitIndicationSet &cockpitIndicationSet) const;
	void SimDraw2dVor2(const class FsCockpitIndicationSet &cockpitIndicationSet) const;
	YSRESULT GetVorIlsIndication(
	    int navId,
	    YsString &vorId,
	    YSBOOL &tuned,
	    YSBOOL &isIls,int &toFrom,double &course,double &lateral,double &vertical,
	    YSBOOL &isDme,double &dme) const;
	void SimDraw2dAdf(const class FsCockpitIndicationSet &cockpitIndicationSet) const;
	YSRESULT GetAdfIndication(
	    YsString &adfId,
	    YSBOOL &tuned,
	    double &bearing) const;
	void SimDrawBlackout(const ActualViewMode &actualViewMode) const;
	void SimDrawAirplane(const ActualViewMode &actualViewMode,const class FsProjection &proj,unsigned int drawFlag) const;
	void SimDrawGround(const ActualViewMode &actualViewMode,const class FsProjection &proj,unsigned int drawFlag) const;
	void SimDrawAirplaneVaporSmoke(void) const;
	void SimDrawField(const ActualViewMode &actualViewMode,const class FsProjection &proj) const;
	void SimDrawShadow(const ActualViewMode &actualViewMode,const class FsProjection &proj) const;        // For OpenGL/Direct3D, not for BlueImpulseSDK
	void SimDrawComplexShadow(const ActualViewMode &actualViewMode,const class FsProjection &proj) const; // For OpenGL/Direct3D, not for BlueImpulseSDK
	void SimDrawFlush(void) const;

	void SimDrawContainer(const ActualViewMode &actualViewMode) const;
	void SimDrawCrossDesignator(void) const;

	void SimDrawGunAim(void) const;
	YSRESULT SimCalculateGunAim(const FsAirplane *&target,YsVec3 &aim) const;
	void SimDrawBombingAim(const ActualViewMode &actualViewMode) const;

	void SimBlastSound(YSBOOL demoMode);

	void SimRecordAir(const double &t,YSBOOL forceRecord);
	void SimRecordGnd(const double &t,YSBOOL forceRecord);

	// void SimSetFormationAutopilot(int fomType);

	void SimPlayTimedEvent(const double &ctime);
	void SimProcessAirTrafficController(void);

	void SimInFlightDialogTransition(void);
	void ToggleAutoPilotDialog(void);
public:
	void SetIgnoreThisKeyHolding(int rawKey);
	void SetUpRadioCommTargetDialog(void);
	void SetUpCallFuelTruckDialog(void);
	void SetUpRadioCommCommandDialog(YSSIZE_T n,FsAirplane *const air[]);
	template <const int N>
	void SetUpRadioCommCommandDialog(const YsArray <FsAirplane *,N> &air)
	{
		SetUpRadioCommCommandDialog(air.GetN(),air);
	}
	void SetUpRadioCommToFormationDialog(void);
	void SetUpRequestApproachDialog(void);
	void SetUpAtcRequestDialog(void);
	YSBOOL AirplaneInFormation(FsAirplane *leader);
	void SetUpGunnerSubMenu(void);

public:
	void RadioCommSendBreakAndAttack(FsAirplane *target,FsAirplane *sender);
	void RadioCommSendCoverMe(FsAirplane *target,FsAirplane *sender);
	void RadioCommSendAttackGround(FsAirplane *target,FsAirplane *sender);
	void RadioCommSendFormOnMyWing(FsAirplane *target,FsAirplane *sender,YSBOOL synchroTrigger);
	void RadioCommSendReturnToBase(FsAirplane *target,FsAirplane *sender);
	void RadioCommSendHoldingPattern(FsAirplane *target,FsAirplane *sender);
	void RadioCommSendLandRefuelAndTakeOff(FsAirplane *target,FsAirplane *sender);
	void RadioCommSendSpreadFormation(FsAirplane *sender,const double factor);
	void RadioCommSendTightenFormation(FsAirplane *sender,const double divFactor);

	// fom[0] will always be the wingLeader, and position to be YsOrigin()
	void GetAircraftInFormation(YsArray <YsPair <FsAirplane *,YsVec3>,16> &fom,FsAirplane *wingLeader) const;

	void GetProjection(class FsProjection &prj,const ActualViewMode &actualViewMode) const;
	void SetSubWindowViewMode(int windowId,FSVIEWMODE viewMode);
	void FlipShowUserNameMasterSwitch(void);
	YSBOOL GetShowUserNameMasterSwitch(void) const;

public:
	static void GetStandardProjection(class FsProjection &prj);

public:
	double GetFirstRecordTime(void);
	double GetLastRecordTime(void);

	YSBOOL IsLockedOn(const FsExistence *ex) const;
	YSBOOL IsMissileChasing(FSWEAPONTYPE &wpnType,YsVec3 &wpnPos,const FsExistence *ex) const;

protected:
	YSBOOL AllRecordedFlightsAreOver(double &lastRecordTime);

	YSBOOL CheckMidAir(YsVec3 &collisionPos,FsExistence &ex1,FsExistence &ex2);
	YSBOOL CheckMidAir(YsVec3 &collisionPos,const YsShell &shl,FsExistence &ex2);
	YSBOOL Explode(FsExistence &ex,YSBOOL sound);

	void UpdateViewpointAccordingToPlayerAirplane(const double &distance,YSBOOL reset);

	void AirplaneCrash(FsAirplane *crashedPlane,FSDIEDOF diedOf,int collType);  // collType  1:Ground  2:Shell
	YSRESULT DestroyAutoGeneratedAirAndGnd(void);

	YSBOOL CheckContinue(void);
	YSBOOL CheckContinueOneStep(void);
	void CheckContinueDraw(void) const;

public:
	void SetNetServer(class FsSocketServer *svr);
	void SetNetClient(class FsSocketClient *cli);
	void NetWeaponLaunch(const FsWeaponRecord &rec);
	void SendNetChatMessage(const char msg[]);

	/*! Returns the number of connected users.
	    If the simulation is not in the server mode, it returns zero.
	*/
	int NetServerGetNumConnectedUser(void) const;

	void OpenVehicleChangeDialog(void);
	void CloseVehicleChangeDialog(void);
	void OpenChatDialog(void);
	void CloseChatDialog(void);
	void OpenLoadingDialog(YSBOOL fuel,YSBOOL ammo,const FsAirplane &air);
};

#include "fsmissiongoal.h"

template <const int N>
inline YSRESULT FsSimulation::GetRunwayRectFromPositionAll(YsArray <const YsSceneryRectRegion *,N> &rgnArray,const YsVec3 &pos) const
{
	field.GetFieldRegion(rgnArray,pos.x(),pos.z());
	for(YSSIZE_T idx=rgnArray.GetN()-1; 0<=idx; --idx)
	{
		if(rgnArray[idx]->GetId()!=FS_RGNID_RUNWAY)
		{
			rgnArray.DeleteBySwapping(idx);
		}
	}
	if(0<rgnArray.GetN())
	{
		return YSOK;
	}
	return YSERR;
}

template <const int N>
inline YSRESULT FsSimulation::GetRegionRectFromPositionAll(YsArray <const YsSceneryRectRegion *,N> &rgnArray,const YsVec3 &pos) const
{
	field.GetFieldRegion(rgnArray,pos.x(),pos.z());
	if(0<rgnArray.GetN())
	{
		return YSOK;
	}
	return YSERR;
}

template <const int N>
inline const FsGround *FsSimulation::FindIlsBestForWindFromCandidateArray(const YsArray <const FsGround *,N> &ilsArray) const
{
	const class FsWeather &weather=GetWeather();
	YsVec3 wind=weather.GetWind();


	YsArray <YsVec3,N> tdVecArray(ilsArray.GetN(),NULL);
	for(YSSIZE_T ilsIdx=0; ilsIdx<ilsArray.GetN(); ++ilsIdx)
	{
		const FsAircraftCarrierProperty *ilsProp=ilsArray[ilsIdx]->Prop().GetAircraftCarrierProperty();

		if(NULL!=ilsProp)
		{
			YsVec3 tdPos;
			YsAtt3 ilsAtt;
			ilsProp->GetILS().GetLandingPositionAndAttitude(tdPos,ilsAtt);  // Forward Vector of tdAtt is from tdPos to glide slope. Opposite of airplane heading at tdPos.

			ilsAtt.SetP(0.0);
			tdVecArray[ilsIdx]=-ilsAtt.GetForwardVector();
			ConstrainRunwayVectorFromRunwayRect(tdVecArray[ilsIdx],tdPos);
		}
		else
		{
			tdVecArray[ilsIdx]=YsOrigin();
		}
	}


	const FsGround *ilsFound=NULL;

	if(YSOK==wind.Normalize() && NULL!=(ilsFound=FindIlsBestForWindFromCandidateArray(ilsArray,tdVecArray,wind)))
	{
		return ilsFound;
	}
	if(NULL!=(ilsFound=FindIlsBestForWindFromCandidateArray(ilsArray,tdVecArray,YsXVec())))
	{
		return ilsFound;
	}
	if(NULL!=(ilsFound=FindIlsBestForWindFromCandidateArray(ilsArray,tdVecArray,YsZVec())))
	{
		return ilsFound;
	}
	return NULL;
}

template <const int N>
inline const FsGround *FsSimulation::FindIlsBestForWindFromCandidateArray(
    const YsArray <const FsGround *,N> &ilsArray,
    const YsArray <YsVec3,N> &tdVecArray,
    const YsVec3 &unitWind) const
{
	const FsGround *ilsFound=NULL;

	double bestWind=0.0;
	for(YSSIZE_T ilsIdx=0; ilsIdx<ilsArray.GetN(); ++ilsIdx)
	{
		const FsAircraftCarrierProperty *ilsProp=ilsArray[ilsIdx]->Prop().GetAircraftCarrierProperty();
		if(NULL!=ilsProp)
		{
			const YsVec3 tdVec=tdVecArray[ilsIdx];
			const double dotProd=tdVec*unitWind;

			if(FsCosineRunwayWindThreshold<fabs(dotProd) && (NULL==ilsFound || bestWind>dotProd))
			{
				ilsFound=ilsArray[ilsIdx];
				bestWind=dotProd;
			}
		}
	}

	return ilsFound;
}

template <const int N>
inline YSSIZE_T FsSimulation::FindVisualApproachBestForWindFromCandidateArray(const YsArray <YsVec3,N> &vfrArray) const
{
	const class FsWeather &weather=GetWeather();
	YsVec3 wind=weather.GetWind();

	YsArray <YsVec3,N> tdVecArray(vfrArray.GetN()/2,NULL);
	for(YSSIZE_T idx=0; idx<=vfrArray.GetN()-2; idx+=2)
	{
		tdVecArray[idx/2]=vfrArray[idx+1];
		ConstrainRunwayVectorFromRunwayRect(tdVecArray[idx/2],vfrArray[idx]);
	}

	YSSIZE_T idxFound=-1;
	if(YSOK==wind.Normalize() && 0<=(idxFound=2*FindVisualApproachBestForWindFromCandidateArray(tdVecArray,wind)))
	{
		return idxFound;
	}
	if(0<=(idxFound=2*FindVisualApproachBestForWindFromCandidateArray(tdVecArray,YsXVec())))
	{
		return idxFound;
	}
	if(0<=(idxFound=2*FindVisualApproachBestForWindFromCandidateArray(tdVecArray,YsZVec())))
	{
		return idxFound;
	}
	return -1;
}

template <const int N>
inline const YsSceneryPointSet *FsSimulation::FindVisualApproachBestForWindFromCandidateArray(const YsArray <const YsSceneryPointSet *,N> &vfrArray) const
{
	YsArray <YsVec3,N> tdVecArray(vfrArray.GetN(),NULL);
	for(auto idx : vfrArray.AllIndex())
	{
		const YsVec3 tdPos=vfrArray[idx]->GetTransformedPoint(0);
		const YsVec3 p1=vfrArray[idx]->GetTransformedPoint(1);
		tdVecArray[idx]=YsUnitVector(p1-tdPos);
	}

	if(0<vfrArray.GetN())
	{
		const class FsWeather &weather=GetWeather();
		YsVec3 wind=weather.GetWind();
		if(YSOK==wind.Normalize())
		{
			auto idx=FindVisualApproachBestForWindFromCandidateArray(tdVecArray,wind);
			if(0<=idx)
			{
				return vfrArray[idx];
			}
		}
		return vfrArray[0];
	}
	return NULL;
}

template <const int N>
inline YSSIZE_T FsSimulation::FindVisualApproachBestForWindFromCandidateArray(const YsArray <YsVec3,N> &tdVecArray,const YsVec3 &unitWind) const
{
	YSSIZE_T idxFound=-1;

	double bestWind=0.0;
	for(YSSIZE_T idx=0; idx<=tdVecArray.GetN(); ++idx)
	{
		const YsVec3 &tdVec=tdVecArray[idx];
		const double dotProd=tdVec*unitWind;
		if(FsCosineRunwayWindThreshold<fabs(dotProd) && (0>idxFound || bestWind>dotProd))
		{
			idxFound=idx;
			bestWind=dotProd;
		}
	}

	return idxFound;
}

////////////////////////////////////////////////////////////

/* } */
#endif
