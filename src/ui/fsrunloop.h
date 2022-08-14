#ifndef FSRUNLOOP_IS_INCLUDED
#define FSRUNLOOP_IS_INCLUDED
/* { */

#include <time.h>
#include <ysclass.h>
#include <ysbitmap.h>

#include <fsgui.h>

#include "fsworld.h"
#include "fscmdparaminfo.h"
#include "fssimulation.h"

////////////////////////////////////////////////////////////

class FsShowLandingPracticeInfo
{
private:
	YSBOOL leftTraffic;
	FSTRAFFICPATTERNLEG leg;
	YsVec2 wind;
	YSBOOL lowCloud;
	YSBOOL lowVisibility;

	YsArray <YsVec2> path;
	YSSIZE_T pathPtr,windPtr;

public:
	enum
	{
		STATE_RUNNING,
		STATE_OVER
	};

	int state;

	FsShowLandingPracticeInfo();
	~FsShowLandingPracticeInfo();
	void CleanUp(void);

	void SetUp(YSBOOL leftTraffic,FSTRAFFICPATTERNLEG leg,const YsVec3 &_wind,YSBOOL lowCloud,YSBOOL lowVisibility);
	void RunOneStep(void);
	void Draw(void) const;
};

////////////////////////////////////////////////////////////

class FsRunLoop
{
public:
	enum INITIALIZATION_STAGE
	{
		INIT_STARTED
	};

	enum RUNMODE
	{
		YSRUNMODE_NONE,
		YSRUNMODE_MENU,
		YSRUNMODE_SHOWLANDINGPRACTICEINFO,
		YSRUNMODE_FLY_REGULAR,
		YSRUNMODE_FLY_DEMOMODE,
		YSRUNMODE_REPLAYRECORD,
		YSRUNMODE_FLY_CLIENTMODE,
		YSRUNMODE_FLY_SERVERMODE,
	};

	static const char *RunModeToString(RUNMODE runMode);

public:
	class GuiCanvasBase : public FsGuiCanvas
	{
	public:
		class FsRunLoop *runLoop;
		virtual void OnTerminateSimulation(FsRunLoop::RUNMODE prevRunMode)=0;
		virtual void OnTerminateReplay(void)=0;

		/*! Return YSTRUE if console output should be shown.
		    Return YSFALSE if title bitmap should be shown.
		*/
		virtual YSBOOL ShowConsole(void)=0;

		/*! This function is called after the option is re-loaded.
		    May have to deal with font-size change, language change, etc.
		    This function is NOT called at the beginning of the program.
		*/
		virtual void AfterReloadOption(class FsOption &opt)=0;

		GuiCanvasBase()
		{
			runLoop=nullptr;
		}
	};

private:
	class RunModeAndCounter
	{
	public:
		RUNMODE runMode;
		unsigned int runModeCounter;
	};

	FsCommandParameter fscp;

	int initializationCounter;
	YSBOOL terminate;
	YSBOOL terminateWhenNoModalDialogIsOpenAndBackToMenu;
	YSBOOL autoDemoForever;

	YSBOOL needReloadOption;
	YSBOOL needRedraw;

	YsArray <RunModeAndCounter> runModeStack;
	class FsWorld *world;
	class FsOption *opt;

	int nTitleBmp;
	YsBitmap titleBmp,*titleBmpPtr;
	YsBitmap newFltMsgBmp,simFlyMsgBmp,simRepMsgBmp;

	class GuiCanvasBase *mainCanvas;
	class FsShowLandingPracticeInfo *showLandingPracticeInfo;

	// Variables used in YSRUNMODE_MENU >>
	time_t lastChangePicture;
	// Variables used in YSRUNMODE_MENU <<

	// Variables used in YSRUNMODE_FLY_REGULAR >>
	FsSimulation::FSSIMULATIONSTATE simState;
	// Variables used in YSRUNMODE_FLY_REGULAR <<

	// Variables used in YSRUNMODE_REPLAYRECORD >>
	FsSimulation::ReplayInfo replayInfo;
	// Variables used in YSRUNMODE_REPLAYRECORD <<

	// Variables used in YSRUNMODE_FLY_DEMOMODE >>
	class FsDemoModeInfo *demoModeInfo;
	YSBOOL demoModeDrawSmoke,demoModeRecordFlight;
    int demoModeAcroType;
	// Variables used in YSRUNMODE_FLY_DEMOMODE <<

	// Variables used in YSRUNMODE_TRAINING_AUTOPILOT >>
	YSBOOL drawGraphicsWhileTrainingAutopilot;
	// Variables used in YSRUNMODE_TRAINING_AUTOPILOT <<

	// Variables used in the client mode >>
	FsClientRunLoop *clientModeRunLoop;
	class FsNetConfig *netcfgPtr;
	// Variables used in the client mode <<

	// Variables used in the server mode >>
	FsServerRunLoop *serverModeRunLoop;
	// Variables used in the server mode <<

public:
	class StepByStepInitializationOption
	{
	};
	FsRunLoop();

	/*! If FsRunLoop is created with StepByStepInitializationOption, the constructor does not call Initialize().
	    The application must call InitializeOneStep until the function returns YSTRUE.
	*/
	FsRunLoop(StepByStepInitializationOption opt);

	FsRunLoop(FsWorld::InitializationOption worldOpt);
private:
	void RealInitialize(void);
public:
	~FsRunLoop();
	void Free(void);

	RUNMODE GetCurrentRunMode(void) const;

	void SetCommandParameter(FsCommandParameter fscp);
	const FsCommandParameter &GetCommandParameter(void) const;
	void SetAutoExit(YSBOOL autoExit);
	void SetAutoDemoForever(YSBOOL autoDemoForever); // 2013/11/27 For cracking down the freeze problem.
	void SetTerminateFlag(YSBOOL term);
	void SetNeedReloadOption(YSBOOL nrlo);


	void SetMainCanvas(GuiCanvasBase *canvasPtr); // <- Later should be made GuiCanvasBase *.
	class GuiCanvasBase *GetCanvas(void);
	void AttachGuiMessageBox(const YsWString &title,const YsWString &msg,const YsWString &okBtn);

	class FsWorld *GetWorld(void);
	const class FsWorld *GetWorld(void) const;
	void Initialize(void);
	void Initialize(FsWorld::InitializationOption worldOpt);

	/*! This function initializes the runloop one step.
	    Returns YSTRUE when the initialization is done.
	*/
	YSBOOL InitializeOneStep(FsWorld::InitializationOption worldOpt);

	void StartShowLandingPracticeInfoMode( // Next run mode is always fixed.
	    YSBOOL leftTraffic,FSTRAFFICPATTERNLEG leg,const YsVec3 &wind,YSBOOL lowCloud,YSBOOL lowVisibility);

public:
	void StartReplayRecord(YSBOOL editMode);
	void StartOpeningDemo(void);
	void StartLandingDemo(const char fldName[],const char airName[]);
	YSRESULT SetUpLandingDemo(const char fldName[],const char airName[]);
	void StartCarrierLandingDemo(const char fldName[],const char airName[]);
	YSRESULT SetUpCarrierLandingDemo(const char fldName[],const char airName[]);
	void StartDogFightDemo(void);
	YSRESULT SetUpDogFightDemo(void);
	void StartAirShowDemo(const char fldName[],const char airName[],int acroType);
private:
	// StartDemoModeInGeneral is not used from StartAirShowDemo due to the timing of initialization.
	void StartDemoModeInGeneral(FSDEMOMODETYPE demoType,const char sysMsg[],const double maxTime);
	FsDemoModeInfo *MakeFreshDemoModeInfo(FSDEMOMODETYPE demoType);
	void AfterDemoAction(void);

public:
	void ChangeRunMode(RUNMODE runMode);
	void ChangeSimulationState(FsSimulation::FSSIMULATIONSTATE simState);
	void PushRunMode(void);
	void PopRunMode(void);

	/*! This function will change runMode to YSRUNMODE_FLY_REGULAR, and simState to FsSimulation::FSSIMSTATE_CENTERJOYSTICK.
	*/
	void TakeOff(RUNMODE nextRunMode=YSRUNMODE_FLY_REGULAR);


	YSSIZE_T attentionClock,prevAttentionClock;
	YSBOOL NeedRedraw(void) const;
	void SetNeedRedraw(YSBOOL needRedraw);
	void Draw(void);

	YSBOOL RunOneStep(void);

	/*! Processes a gui script command.
	    In YSRUNMODE_MENU, the command is sent to canvasPtr.
	*/
	void ProcessGuiCommand(const char *cmd);

	/*! Processes a file command. */
	void ProcessFileCommand(const char *cmd);

	/*! Processes a flight script command.
	*/
	void ProcessFlightCommand(const char *cmd);

	/*! Processes a button function.
	*/
	void ProcessButtonFunction(const char *cmd);

	/*! Processes an assert command. */
	void ProcessAssertCommand(const char *cmd);

	/*! Returns YSTRUE if:
	      Server is ready to accept client connections or
	      Client has connected to the server and ready to take user command.
	    Always returns YSFALSE if the running mode is not YSRUNMODE_FLY_SERVERMODE nor YSRUNMODE_FLY_CLIENTMODE.
	*/
	YSBOOL NetReady(void) const;

	/*! Returns YSTRUE if the simulation is running and the user (or a player object) is controlling a vehicle. */
	YSBOOL Flying(void) const;


private:
	YSBOOL RunSimulationOneStep(void);
	void DrawInSimulationMode(void);  // May not be able to const-nize this at this moment.

	void EndSimulationMode(void);

	YSBOOL RunReplayOneStep(void);

	YSBOOL RunShowLandingPracticeInfoOneStep(void);


	// Main Menu
	YSBOOL RunMenuOneStep(void);
	void DrawMenu(void) const;
	void ChangeBitmapColor(YsBitmap &bmp,const YsColor &set);
public:

	/*! This function sets up endurance mode.
	    After this function, call TakeOff with YSRUNMODE_FLY_ENDURANCEMODE to start.
	*/
	YSRESULT SetUpEnduranceMode(
	    const char playerPlane[],
	    YSBOOL jet,YSBOOL ww2,
	    const char fieldName[],int numWingman,int wingmanLevel,YSBOOL allowAAM,
	    YSSIZE_T nWeaponConfigInput,const int weaponConfigInput[],int fuel);

	/*! This function sets up close-air-support mission mode.
	    After this function, call TakeOff with YSRUNMODE_FLY_CLOSEAIRSUPPORTMISSION to start.
	*/
	YSRESULT SetUpCloseAirSupportMission(const char airName[],const char fldName[],YSSIZE_T nWeaponConfig,const int weaponConfig[],int fuel);


	/*! This function sets up intercept mission mode.
	    After this function, call TakeOff with YSRUNMODE_FLY_INTERCEPTMISSION to start.
	*/
	YSRESULT SetUpInterceptMission(const class FsInterceptMissionInfo &info);

	/*! This function sets up ground-to-air mission mode.
	    After this function, call TakeOff with YSRUNMODE_GROUND_TO_AIR_MISSION to start.
	*/
	YSRESULT SetUpGroundToAirMission(const FsGroundToAirDefenseMissionInfo &info);



	/*! This function sets up a landing-practice mode.
	    After this function, call StartShowLandingPracticeInfoMode (Not TakeOff) to start.
	*/
	YSRESULT SetUpLandingPracticeMode(
	    const char fldName[],const char airName[],
	    YSBOOL &leftTraffic, // Need to return to the caller....
	    FSTRAFFICPATTERNLEG leg,const double &crossWind,YSBOOL lowCloud,YSBOOL lowVisibility);



	void StartNetServerMode(const char username[],const char fieldName[],int netPort);
	void StartNetClientMode(const char username[],const char hostname[],int netPort);
};

/* } */
#endif
