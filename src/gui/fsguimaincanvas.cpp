#include <fsconfig.h>
#include <fsoption.h>
#include <fsguiconfigdlg.h>

#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"

#include "graphics/common/fsopengl.h"
#include "fswirefont.h"

#include "fsguinewflightdialog.h"

#ifdef WIN32
#include <float.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <ysbitmap.h>

#include "fsgui.h"
#include "fsguiselectiondialogbase.h"
#include <fsguifiledialog.h>

#include "fschoose.h"

#include "graphics/common/fsfontrenderer.h"

#include "fstextresource.h"

#include "fsrunloop.h"
#include "fsmenu.h"
#include "fsguicommondialog.h"
#include "fsguimaincanvas.h"

////////////////////////////////////////////////////////////

// Simulation Event Handlers
/* virtual */ void FsGuiMainCanvas::OnTerminateSimulation(FsRunLoop::RUNMODE prevRunMode)
{
	auto world=runLoop->GetWorld();
	StartSimulationResultDialog(world->GetSimulation(),0);
}

/* virtual */ void FsGuiMainCanvas::OnTerminateReplay(void)
{
	auto world=runLoop->GetWorld();
	StartSimulationResultDialog(world->GetSimulation(),0);
}

/* virtual */ YSBOOL FsGuiMainCanvas::ShowConsole(void)
{
	return mainDlg->showConsoleBtn->GetCheck();
}

/* virtual */ void FsGuiMainCanvas::AfterReloadOption(class FsOption &opt)
{
	mainMenu->Initialize();
	mainMenu->Make();

	mainDlg->Initialize();
	mainDlg->Make();
}

////////////////////////////////////////////////////////////

FsGuiMainCanvas::FsGuiMainCanvas()
{
	mainMenu=nullptr;
	mainDlg=nullptr;

	Sim_LandingPractice_Level=0;
	Option_AirShowDemo_AcroType=0;

	enduranceModeDialog=NULL;
	simResultDialog=NULL;
	missionGoalDialog=NULL;
	interceptMissionDialog=NULL;
	closeAirSupportMissionDialog=NULL;
	chooseAircraftDialog=NULL;
	inputNumberDialog=NULL;
	airCombatDlg=NULL;

	readiness=SIMULATION_INITIAL;
}

FsGuiMainCanvas::~FsGuiMainCanvas()
{
	if(NULL!=airCombatDlg)
	{
		delete airCombatDlg;
		airCombatDlg=NULL;
	}
	if(NULL!=inputNumberDialog)
	{
		delete inputNumberDialog;
		inputNumberDialog=NULL;
	}
	if(NULL!=chooseAircraftDialog)
	{
		delete chooseAircraftDialog;
		chooseAircraftDialog=NULL;
	}
	if(NULL!=closeAirSupportMissionDialog)
	{
		delete closeAirSupportMissionDialog;
		closeAirSupportMissionDialog=NULL;
	}
	if(NULL!=interceptMissionDialog)
	{
		delete interceptMissionDialog;
		interceptMissionDialog=NULL;
	}
	if(NULL!=enduranceModeDialog)
	{
		delete enduranceModeDialog;
		enduranceModeDialog=NULL;
	}
	if(NULL!=missionGoalDialog)
	{
		delete missionGoalDialog;
		missionGoalDialog=NULL;
	}
	if(NULL!=simResultDialog)
	{
		FsGuiResultDialogClass::Delete(simResultDialog);
		simResultDialog=NULL;
	}
	if(nullptr!=mainMenu)
	{
		delete mainMenu;
		mainMenu=nullptr;
	}
	if(nullptr!=mainDlg)
	{
		delete mainDlg;
		mainDlg=nullptr;
	}
}

/* virtual */ void FsGuiMainCanvas::OnInterval(void)
{
	auto world=runLoop->GetWorld();

	auto nextReadiness=readiness;
	if(world->SimulationIsPrepared()!=YSTRUE)
	{
		nextReadiness=SIMULATION_NOT_READY;
	}
	else if(YSTRUE==world->PlayerPlaneIsReady() || YSTRUE==world->PlayerGroundIsReady())
	{
		nextReadiness=SIMULATION_READY_TO_FLY;
	}
	else
	{
		nextReadiness=SIMULATION_READY_TO_REPLAY;
	}

	if(nextReadiness!=readiness)
	{
		switch(nextReadiness)
		{
		case SIMULATION_NOT_READY:
			mainDlg->statusMsg->SetText("Please Load Flight or Create New Flight");

			mainDlg->saveFlightBtn->Disable();
			mainDlg->flyNowBtn->Disable();
			mainDlg->replayRecordBtn->Disable();
			break;
		case SIMULATION_READY_TO_FLY:
			mainDlg->statusMsg->SetText("Select \"Simulation\"->\"Fly\" to Start Flying");

			mainDlg->saveFlightBtn->Enable();
			mainDlg->flyNowBtn->Enable();
			mainDlg->replayRecordBtn->Disable();
			break;
		case SIMULATION_READY_TO_REPLAY:
			mainDlg->statusMsg->SetText("Select \"Simulation\"->\"Replay Flight Record\" to play flight record");

			mainDlg->saveFlightBtn->Enable();
			mainDlg->flyNowBtn->Disable();
			mainDlg->replayRecordBtn->Enable();
			break;
		}
	}
	readiness=nextReadiness;
}

void FsGuiMainCanvas::MakeMainMenu(FsRunLoop *runLoop)
{
	if(nullptr!=mainMenu)
	{
		delete mainMenu;
		mainMenu=nullptr;
	}
	mainMenu=new FsGuiMainMenu;
	mainMenu->Initialize();
	mainMenu->world=runLoop->GetWorld();
	mainMenu->canvas=this;
	mainMenu->runLoop=runLoop;
	mainMenu->Make();
	SetMainMenu(mainMenu);
}

void FsGuiMainCanvas::MakeMainDialog(FsRunLoop *runLoop)
{
	if(nullptr!=mainDlg)
	{
		delete mainDlg;
		mainDlg=nullptr;
	}
	mainDlg=new FsGuiMainDialog;
	mainDlg->Initialize();
	mainDlg->canvas=this;
	mainDlg->runLoop=runLoop;
	mainDlg->Make();
	mainDlg->SetIsPermanent(YSTRUE);
	AddDialog(mainDlg);
}

void FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed(FsGuiDialog *,int returnCode)
{
	if(nullptr==GetActiveModalDialog())
	{
		runLoop->PopRunMode();
	}
}

FsGuiConfirmDeleteFlightDialog *FsGuiMainCanvas::StartConfirmDeleteFlightDialog(int nextActionCode)
{
	auto confirmDeleteFlightDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiConfirmDeleteFlightDialog>();
	confirmDeleteFlightDialog->Initialize();
	confirmDeleteFlightDialog->SetIdent("confirmDeleteDlg");
	confirmDeleteFlightDialog->Make(0,nextActionCode);
	confirmDeleteFlightDialog->SetCloseModalCallBack(NULL);
	AttachModalDialog(confirmDeleteFlightDialog);

	return confirmDeleteFlightDialog;
}

void FsGuiMainCanvas::StartAboutDialog(void)
{
	auto aboutDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiAboutDialog>();
	aboutDialog->Initialize();
	aboutDialog->Make();
	AttachModalDialog(aboutDialog);
}

void FsGuiMainCanvas::StartSupportYsflightDialog(YSBOOL firstStart,int nextActionCode)
{
	auto supportYsflightDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiSupportYsflightDialogClass>();
	supportYsflightDlg->Initialize();
	supportYsflightDlg->Make(firstStart,nextActionCode);
	AttachModalDialog(supportYsflightDlg);
}

class FsGuiMessageBoxDialog *FsGuiMainCanvas::StartMessageBox(
    const wchar_t title[],const wchar_t msg[],const wchar_t yesBtn[],const wchar_t noBtn[],
    int nextActionYes,int nextActionNo)
{
	auto messageBoxDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
	messageBoxDialog->Initialize();
	messageBoxDialog->Make(title,msg,yesBtn,noBtn,nextActionYes,nextActionNo);
	AttachModalDialog(messageBoxDialog);

	return messageBoxDialog;
}

FsGuiDialog *FsGuiMainCanvas::StartInputNumberDialog(
    const double defNumber,const int belowDecimal,
    const wchar_t title[],const wchar_t msg[],const wchar_t label[],
    const wchar_t yesBtn[],const wchar_t noBtn[],int returnCodeYes,int returnCodeNo)
{
	if(NULL==inputNumberDialog)
	{
		inputNumberDialog=new FsGuiInputNumberDialog;
	}
	inputNumberDialog->Initialize();
	inputNumberDialog->Make(defNumber,belowDecimal,title,msg,label,yesBtn,noBtn,returnCodeYes,returnCodeNo);
	AttachModalDialog(inputNumberDialog);

	return inputNumberDialog;
}

FsGuiDialog *FsGuiMainCanvas::StartMissionGoalDialog(const FsSimulation *sim)
{
	if(NULL==missionGoalDialog)
	{
		missionGoalDialog=new FsGuiMissionGoalDialogClass;
	}
	missionGoalDialog->Initialize();
	missionGoalDialog->Create(sim,0);
	AttachModalDialog(missionGoalDialog);
	return missionGoalDialog;
}

FsGuiNoJoystickWarningDialogClass *FsGuiMainCanvas::StartNoJoystickWarningDialog(void)
{
	auto noJoystickWarningDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiNoJoystickWarningDialogClass>();
	noJoystickWarningDialog->Initialize();
	noJoystickWarningDialog->Make(0);
	noJoystickWarningDialog->SetCloseModalCallBack(NULL);
	AttachModalDialog(noJoystickWarningDialog);
	return noJoystickWarningDialog;
}

void FsGuiMainCanvas::StartSimulationResultDialog(const FsSimulation *sim,int nextActionCode)
{
	if(NULL==simResultDialog)
	{
		simResultDialog=FsGuiResultDialogClass::Create();
	}
	simResultDialog->Initialize();
	simResultDialog->Make(sim,nextActionCode);
	AttachModalDialog(simResultDialog);
}

class FsGuiNewFlightDialogClass *FsGuiMainCanvas::StartNewFlightDialog(
    class FsWorld *world,
    const class FsNewFlightDialogOption &option)
{
	auto newFlightDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiNewFlightDialogClass>();
	newFlightDialog->InitialSetUp(option);

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());

	newFlightDialog->Make(world);

    FsNewFlightDialogInfo info;
	info.SetFromConfig(runLoop->GetWorld(),cfg);
	info.SetFromOption(option);                  // SetFromOption must come after SetFromConfig so that the option has priority.
	newFlightDialog->InitializeDialog(runLoop->GetWorld(),info);

	AttachModalDialog(newFlightDialog);

	return newFlightDialog;
}

FsGuiDialog *FsGuiMainCanvas::StartEnduranceModeDialog(int nextActionCode)
{
	if(NULL!=enduranceModeDialog)
	{
		delete enduranceModeDialog;
		enduranceModeDialog=NULL;
	}
	enduranceModeDialog=new FsGuiEnduranceModeDialog;

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());

	enduranceModeDialog->Make(runLoop->GetWorld(),nextActionCode);
	enduranceModeDialog->Initialize(runLoop->GetWorld(),cfg);

	AttachModalDialog(enduranceModeDialog);

	return enduranceModeDialog;
}

FsGuiDialog *FsGuiMainCanvas::StartInterceptMissionDialog(int nextActionCode)
{
	if(NULL!=interceptMissionDialog)
	{
		delete interceptMissionDialog;
		interceptMissionDialog=NULL;
	}
	interceptMissionDialog=new FsGuiInterceptMissionDialog;

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());

	interceptMissionDialog->Make(runLoop->GetWorld(),nextActionCode);
	interceptMissionDialog->Initialize(runLoop->GetWorld(),cfg);

	AttachModalDialog(interceptMissionDialog);

	return interceptMissionDialog;
}

FsGuiDialog *FsGuiMainCanvas::StartCloseAirSupportMissionDialog(int nextActionCode)
{
	if(NULL!=closeAirSupportMissionDialog)
	{
		delete closeAirSupportMissionDialog;
		closeAirSupportMissionDialog=NULL;
	}
	closeAirSupportMissionDialog=new FsGuiCloseAirSupportMissionDialog;

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());

	closeAirSupportMissionDialog->Make(runLoop->GetWorld(),cfg,nextActionCode);

	AttachModalDialog(closeAirSupportMissionDialog);

	return closeAirSupportMissionDialog;
}

FsGuiChooseAircraft *FsGuiMainCanvas::StartSelectAircraftDialog(const char defAirplane[],int returnCode)
{
	if(NULL!=chooseAircraftDialog)
	{
		delete chooseAircraftDialog;
		chooseAircraftDialog=NULL;
	}
	FsGuiChooseAircraftOption opt;

	chooseAircraftDialog=new FsGuiChooseAircraft;
	chooseAircraftDialog->Initialize();
	chooseAircraftDialog->Create(runLoop->GetWorld(),opt,returnCode);
	chooseAircraftDialog->SetDefault(defAirplane);

	AttachModalDialog(chooseAircraftDialog);
	return chooseAircraftDialog;
}

class FsGuiSelectMissionDialog *FsGuiMainCanvas::StartSelectMissionDialog(void)
{
	YsArray <YsWString> filelist;
	YsWString subDir(L"mission");
	FsFindFileList(filelist,subDir,L"",L"yfs");
	if(0<filelist.GetN())
	{
		auto selectMissionDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiSelectMissionDialog>();
		selectMissionDialog->Make(subDir,filelist);
		AttachModalDialog(selectMissionDialog);
		return selectMissionDialog;
	}
	return nullptr;
}

