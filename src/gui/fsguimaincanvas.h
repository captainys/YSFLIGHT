#ifndef FSGUIMAINCANVAS_IS_INCLUDED
#define FSGUIMAINCANVAS_IS_INCLUDED
/* { */



#include <fsgui.h>
#include <fsguifiledialog.h>

#include "fsconfig.h"
#include "fsoption.h"
#include "fsnetconfig.h"

#include "fsmenu.h"
#include "fsfilename.h"

#include "fstextresource.h"

#include "fsguinewflightdialog.h"
#include "fsguisiminfodialog.h"

#include "fschoose.h"

#include "fsguicommondialog.h"



#include "fsrunloop.h"

class FsGuiMainCanvas : public FsRunLoop::GuiCanvasBase
{
public:
	typedef FsGuiMainCanvas THISCLASS;

	enum SIMULATION_READINESS
	{
		SIMULATION_INITIAL,
		SIMULATION_NOT_READY,
		SIMULATION_READY_TO_FLY,
		SIMULATION_READY_TO_REPLAY
	};

	class FsGuiMainMenu *mainMenu;
	class FsGuiMainDialog *mainDlg;
	SIMULATION_READINESS readiness;

	class FsGuiEnduranceModeDialog *enduranceModeDialog;
	class FsGuiResultDialogClass *simResultDialog;
	class FsGuiMissionGoalDialogClass *missionGoalDialog;
	class FsGuiInterceptMissionDialog *interceptMissionDialog;
	class FsGuiCloseAirSupportMissionDialog *closeAirSupportMissionDialog;
	class FsGuiChooseAircraft *chooseAircraftDialog;
	class FsGuiInputNumberDialog *inputNumberDialog;
	class FsGuiAirCombatDialog *airCombatDlg;

	FsGuiMainCanvas();
	~FsGuiMainCanvas();

	virtual void OnInterval(void);

	void MakeMainMenu(FsRunLoop *runLoop);
	void MakeMainDialog(FsRunLoop *runLoop);

	/*! Bind this function if the run mode must be popped when all modal dialogs have been closed. */
	void PopRunModeWhenLastModalDialogIsClosed(FsGuiDialog *,int returnCode);


	class FsGuiMessageBoxDialog *StartMessageBox(
	    const wchar_t title[],const wchar_t msg[],const wchar_t yesBtn[],const wchar_t noBtn[],
	    int nextActionYes,int nextActionNo);
	FsGuiDialog *StartInputNumberDialog(
	    const double defNumber,const int belowDecimal,
	    const wchar_t title[],const wchar_t msg[],const wchar_t label[],
	    const wchar_t yesBtn[],const wchar_t noBtn[],int returnCodeYes,int returnCodeNo);

	class FsGuiConfirmDeleteFlightDialog *StartConfirmDeleteFlightDialog(int nextActionCode);  // Replaces FsGuiCheckDeleteCurrentFlight

	void StartAboutDialog(void);
	void StartSupportYsflightDialog(YSBOOL firstStart,int nextActionCode);

	FsGuiDialog *StartMissionGoalDialog(const FsSimulation *sim);
	class FsGuiNoJoystickWarningDialogClass *StartNoJoystickWarningDialog(void);
	void StartSimulationResultDialog(const FsSimulation *sim,int nextActionCode);

	class FsGuiNewFlightDialogClass *StartNewFlightDialog(
	    class FsWorld *world,
	    const class FsNewFlightDialogOption &option);
	FsGuiDialog *StartEnduranceModeDialog(int nextActionCode);
	FsGuiDialog *StartInterceptMissionDialog(int nextActionCode);
	FsGuiDialog *StartCloseAirSupportMissionDialog(int nextActionCode);

	FsGuiChooseAircraft *StartSelectAircraftDialog(const char defAirplane[],int returnCode);

	class FsGuiSelectMissionDialog *StartSelectMissionDialog(void);

////////////////////////////////////////////////////////////

	virtual void OnTerminateSimulation(FsRunLoop::RUNMODE prevRunMode);
	virtual void OnTerminateReplay(void);
	virtual YSBOOL ShowConsole(void);
	virtual void AfterReloadOption(class FsOption &opt);

////////////////////////////////////////////////////////////
// Menu Call-Backs
	void File_Open(FsGuiPopUpMenuItem *);
	void File_Open_DeletedConfirmed(FsGuiDialog *,int);
	void File_Open_FileSelected(FsGuiDialog *,int);

	void File_Save(FsGuiPopUpMenuItem *);
	void File_SaveStep2(FsGuiDialog *,int returnCode);
	void File_SaveStep3(FsGuiDialog *,int returnCode);
	void File_Save_Save(YsWString fn);

	void File_OpenMission(FsGuiPopUpMenuItem *);
	void File_OpenMission_DeleteConfirmed(FsGuiDialog *,int);
	void File_OpenMission_MissionSelected(FsGuiDialog *closedModalDialog,int returnCode);

	void File_OpenPrevFlight(FsGuiPopUpMenuItem *);
	void File_OpenPrevFlight_DeleteConfirmed(FsGuiDialog *,int);

	void File_CloseFlight(FsGuiPopUpMenuItem *);
	void File_CloseFlight_DeleteConfirmed(FsGuiDialog *,int);

	void File_Exit(FsGuiPopUpMenuItem *);
	void File_Exit_DeleteConfirmed(FsGuiDialog *closedModalDialog,int returnCode);
	void File_Exit_ReallyExit(FsGuiDialog *closedModalDialog,int returnCode);

	void File_Recent(FsGuiPopUpMenuItem *);
	void File_Recent_DeleteConfirmed(FsGuiDialog *,int returnCode);
	void File_Recent_Load(const YsWString &fn);


	FsRunLoop::RUNMODE nextRunMode;
	void Sim_Fly(FsGuiPopUpMenuItem *);

	/*! This function caches nextRunMode in this->nextRunMode, and starts the take-off sequence.
	*/
	void Sim_Fly_StartTakeOffSequence(FsRunLoop::RUNMODE nextRunMode);
	void Sim_Fly_MissionGoalDone(FsGuiDialog *,int);
	void Sim_Fly_JoystickWarningDone(FsGuiDialog *,int);

	void Sim_ReplayRecord(FsGuiPopUpMenuItem *);
	void Sim_ReplayRecord_RecordExist(void);
	void Sim_ReplayRecord_JoystickChecked(FsGuiDialog *,int);
	void Sim_ReplayRecord_Terminate(void);

	void Sim_ChooseStartPosition(FsGuiPopUpMenuItem *);
	void Sim_ChooseStartPosition_Selected(FsGuiDialog *dlg,int returnCode);

	class SelectDayOrNightDialog;
	void Sim_SelectDayOrNight(FsGuiPopUpMenuItem *);
	void Sim_SelectDayOrNight_Selected(FsGuiDialog *dlg,int returnCode);

	void Sim_CreateAirCombat(FsGuiPopUpMenuItem *);
	void Sim_CreateAirCombat_ConfirmDeleteCurrentFlight(FsGuiDialog *closedModalDialog,int returnCode);
	void Sim_CreateAirCombat_DialogClosed(FsGuiDialog *closedModalDialog,int returnCode);

	void Sim_Retry(FsGuiPopUpMenuItem *);
	void Sim_Retry_DeleteConfirmed(FsGuiDialog *,int);
	YSRESULT Sim_RetryStart(void);

	void Sim_CreateFlight(FsGuiPopUpMenuItem *);
	void Sim_CreateFlight_DeleteConfirmed(FsGuiDialog *,int);
	void Sim_CreateFlight_OptionSelected(FsGuiDialog *closedModalDialog,int returnCode);
	void Sim_CreateFlight_Create(class FsNewFlightDialogInfo &info);

	void Sim_ChooseAircraft(FsGuiPopUpMenuItem *);
	void Sim_ChooseAircraft_And_Startpos_Select(FsGuiDialog *,int);
	void Sim_ChooseAircraft_And_Startpos_Select(const FsNewFlightDialogInfo &info);
	void Sim_ChooseAircraft_Select(FsGuiDialog *,int);
	void Sim_ChooseAircraft_Select(const class FsGuiChooseAircraft *dlg);

	void Sim_DisableGroundFire(FsGuiPopUpMenuItem *);

	void Sim_EnduranceMode(FsGuiPopUpMenuItem *);
	void Sim_EnduranceMode_DeleteConfirmed(FsGuiDialog *,int);
	void Sim_EnduranceMode_OptionSelected(FsGuiDialog *,int);

	void Sim_InterceptMission(FsGuiPopUpMenuItem *);
	void Sim_InterceptMission_DeleteConfirmed(FsGuiDialog *,int);
	void Sim_InterceptMission_OptionSelected(FsGuiDialog *,int);

	void Sim_CloseAirSupportMission(FsGuiPopUpMenuItem *);
	void Sim_CloseAirSupportMission_DeleteConfirmed(FsGuiDialog *,int);
	void Sim_CloseAirSupportMission_OptionSelected(FsGuiDialog *,int);

	void Sim_CreateInGroundObject(FsGuiPopUpMenuItem *);
	void Sim_CreateInGroundObject_DeleteConfirmed(FsGuiDialog *closedModalDialog,int returnCode);
	void Sim_CreateInGroundObject_OptionSelected(FsGuiDialog *closedModalDialog,int returnCode);

	void Sim_GroundToAirMission(FsGuiPopUpMenuItem *);
	void Sim_GroundToAirMission_DeleteConfirmed(FsGuiDialog *closedModalDialog,int returnCode);
	void Sim_GroundToAirMission_OptionSelected(FsGuiDialog *closedModalDialog,int returnCode);

	int Sim_LandingPractice_Level;
	void Sim_LandingPractice(FsGuiPopUpMenuItem *);
	void Sim_LandingPractice_DeleteConfirmed(FsGuiDialog *,int);
	void Sim_LandingPractice_OptionSelected(FsGuiDialog *closedModalDialog,int returnCode);

	void Sim_RacingMode(FsGuiPopUpMenuItem *);
	void Sim_RacingMode_DeleteConfirmed(FsGuiDialog *closedModalDialog,int returnCode);
	void Sim_RacingMode_OptionSelected(FsGuiDialog *closedModalDialog,int returnCode);
	void Sim_RacingMode_Create(class FsNewFlightDialogInfo &info);

	void Net_StartServerMode(FsGuiPopUpMenuItem *);
	void Net_StartServerMode_DeleteConfirmed(FsGuiDialog *,int);
	void Net_StartServerMode_JoystickWarned(FsGuiDialog *,int);
	void Net_StartServerMode_OptionSelected(FsGuiDialog *,int);

	void Net_StartClientMode(FsGuiPopUpMenuItem *);
	void Net_StartClientMode_DeleteConfirmed(FsGuiDialog *,int);
	void Net_StartClientMode_JoystickWarned(FsGuiDialog *,int);
	void Net_StartClientMode_OptionSelected(FsGuiDialog *,int);

	void Net_Config(FsGuiPopUpMenuItem *);


	void UtilSetSpacePrecision(FsGuiPopUpMenuItem *);
	void UtilSetSpacePrecision_OptionSelected(FsGuiDialog *,int);

	void UtilSetTimeInterval(FsGuiPopUpMenuItem *);
	void UtilSetTimeInterval_OptionSelected(FsGuiDialog *,int);

	void Util_EditFlightRecord(FsGuiPopUpMenuItem *);
	void Util_EditFlightRecord_JoystickWarned(FsGuiDialog *,int);


	void Option_Config(FsGuiPopUpMenuItem *);

	void Option_Option(FsGuiPopUpMenuItem *);
	void Option_Option_OptionSelected(FsGuiDialog *closedDialog,int returnCode);

	void Option_KeyAssign(FsGuiPopUpMenuItem *);

	void Option_CalibrateJoystick(FsGuiPopUpMenuItem *);

	void Option_DogFightDemo(FsGuiPopUpMenuItem *);
	void Option_DogFightDemo_DeleteConfirmed(FsGuiDialog *,int);

	void Option_LandingDemo(FsGuiPopUpMenuItem *);
	void Option_LandingDemo_DeleteConfirmed(FsGuiDialog *,int returnCode);
	void Option_LandingDemo_OptionSelected(FsGuiDialog *closedModalDialog,int returnCode);

	void Option_CarrierLandingDemo(FsGuiPopUpMenuItem *);
	void Option_CarrierLandingDemo_DeleteConfirmed(FsGuiDialog *,int);
	void Option_CarrierLandingDemo_OptionSelected(FsGuiDialog *closedDialog,int returnCode);

	int Option_AirShowDemo_AcroType;
	void Option_AirShowDemo(FsGuiPopUpMenuItem *item);
	void Option_AirShowDemo(int acroType);
	void Option_AirShowDemo_DeleteConfirmed(FsGuiDialog *,int);
	void Option_AirShowDemo_OptionSelected(FsGuiDialog *closedModalDialog,int returnCode);



	void Learning_OpenTrainingData(FsGuiPopUpMenuItem *);
	void Learning_OpenTrainingData_FileSelected(FsGuiDialog *,int returnCode);

	void Learning_SaveTrainingData(FsGuiPopUpMenuItem *);
	void Learning_SaveTrainingData_ConfirmOverwrite(FsGuiDialog *,int returnCode);
	void Learning_SaveTrainingData_FileSelected(FsGuiDialog *,int returnCode);

	void Learning_CloseTrainingData(FsGuiPopUpMenuItem *);

	void Learning_TrainAutoPilot(FsGuiPopUpMenuItem *);
	void Learning_TrainAutoPilot_DeleteConfirmed(FsGuiDialog *,int returnCode);
	void Learning_TrainAutoPilot_AircraftSelected(FsGuiDialog *closedDialog,int returnCode);

	void Learning_SetUpExperiment(FsGuiPopUpMenuItem *);
	void Learning_SetUpExperiment_DeleteConfirmed(FsGuiDialog *,int returnCode);
	void Learning_SetUpExperiment_AircraftSelected(FsGuiDialog *closedDialog,int returnCode);
};



/* } */
#endif
