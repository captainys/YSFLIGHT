#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>
#include <ysclass.h>
#include <ysport.h>

#include "fsconfig.h"

#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"

#include "graphics/common/fsopengl.h"
#include "fswirefont.h"

#include "fsdialog.h"
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
#include <fsguiconfigdlg.h>

#include "fsrunloop.h"
#include "fsmenu.h"
#include "fsguimaincanvas.h"
#include "fsguicommondialog.h"


#ifndef _WIN32
#include <fsjoycalibdlg.h>
#endif


////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Option_Config(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiConfigDialog>();

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());

	dlg->Initialize();
	dlg->world=runLoop->GetWorld();
	dlg->MakeDialog(runLoop->GetWorld(),cfg);
	dlg->InitializeDialog(runLoop->GetWorld(),cfg);
	AttachModalDialog(dlg);
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Option_Option(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiOptionDialog>();
	FsOption opt;

	opt.Load(FsGetOptionFile());
	dlg->Make();
	dlg->Initialize(opt);
	dlg->BindCloseModalCallBack(&THISCLASS::Option_Option_OptionSelected,this);
	AttachModalDialog(dlg);
}
void FsGuiMainCanvas::Option_Option_OptionSelected(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		runLoop->SetNeedReloadOption(YSTRUE);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Option_KeyAssign(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiKeyAssignDialogClass>();
	dlg->ctlAssign.Load(FsGetControlAssignFile());
	dlg->ctlAssign.BuildMapping();
	dlg->Make();
	dlg->Initialize();
	AttachModalDialog(dlg);
}

////////////////////////////////////////////////////////////

#ifndef _WIN32
extern int FsGetNumYsJoyReader(void);
extern YsJoyReader *FsGetYsJoyReaderArray(void);
#endif

void FsGuiMainCanvas::Option_CalibrateJoystick(FsGuiPopUpMenuItem *)
{
#ifndef _WIN32
	auto *dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsCalibrationDialog>();

	int nJoy=FsGetNumYsJoyReader();
	auto joyArray=FsGetYsJoyReaderArray();

	dlg->SetJoystick(nJoy,joyArray);
	dlg->MakeDialog();
	AttachModalDialog(dlg);
#endif
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Option_DogFightDemo(FsGuiPopUpMenuItem *)
{
	if(runLoop->GetWorld()->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Option_DogFightDemo_DeleteConfirmed,this);
		return;
	}
	Option_DogFightDemo_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Option_DogFightDemo_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if(returnCode==(int)YSOK)
	{
		runLoop->StartDogFightDemo();
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Option_LandingDemo(FsGuiPopUpMenuItem *)
{
	if(runLoop->GetWorld()->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Option_LandingDemo_DeleteConfirmed,this);
		return;
	}
	Option_LandingDemo_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Option_LandingDemo_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if(returnCode==(int)YSOK)
	{
		FsNewFlightDialogOption opt;
		opt.canSelectWingmen=YSTRUE;
		opt.canChooseNight=YSTRUE;
		opt.canChooseFomType=YSFALSE;
		opt.flyNowButton=YSFALSE;
		auto dlg=StartNewFlightDialog(runLoop->GetWorld(),opt);
		dlg->BindCloseModalCallBack(&THISCLASS::Option_LandingDemo_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Option_LandingDemo_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiNewFlightDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		runLoop->StartLandingDemo(dlg->info.fieldName,dlg->info.playerAirInfo.typeName);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Option_CarrierLandingDemo(FsGuiPopUpMenuItem *)
{
	if(runLoop->GetWorld()->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Option_CarrierLandingDemo_DeleteConfirmed,this);
		return;
	}
	Option_CarrierLandingDemo_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Option_CarrierLandingDemo_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if(returnCode==(int)YSOK)
	{
		FsNewFlightDialogOption opt;
		opt.canSelectWingmen=YSTRUE;
		opt.canChooseNight=YSTRUE;
		opt.canChooseFomType=YSFALSE;
		opt.flyNowButton=YSFALSE;
		auto dlg=StartNewFlightDialog(runLoop->GetWorld(),opt);
		dlg->BindCloseModalCallBack(&THISCLASS::Option_CarrierLandingDemo_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Option_CarrierLandingDemo_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiNewFlightDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		runLoop->StartCarrierLandingDemo(dlg->info.fieldName,dlg->info.playerAirInfo.typeName);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Option_AirShowDemo(FsGuiPopUpMenuItem *item)
{
	if(item==mainMenu->acroStarOfDavid)
	{
		Option_AirShowDemo(FSACRO_STAROFDAVID);
	}
	else if(item==mainMenu->acroCorkScrew)
	{
		Option_AirShowDemo(FSACRO_CORKSCREW);
	}
	else if(item==mainMenu->acroSlowRoll)
	{
		Option_AirShowDemo(FSACRO_SLOWROLL);
	}
	else if(item==mainMenu->acroDeltaLoop)
	{
		Option_AirShowDemo(FSACRO_DELTALOOP);
	}
	else if(item==mainMenu->acroDeltaRoll)
	{
		Option_AirShowDemo(FSACRO_DELTAROLL);
	}
	else if(item==mainMenu->acroTightTurn)
	{
		Option_AirShowDemo(FSACRO_TIGHTTURN);
	}
	else if(item==mainMenu->acro360Loop)
	{
		Option_AirShowDemo(FSACRO_360ANDLOOP);
	}
	else if(item==mainMenu->acroBombBurstUpward4)
	{
		Option_AirShowDemo(FSACRO_BOMBBURST4SHIP);
	}
	else if(item==mainMenu->acroBombBurstUpward6)
	{
		Option_AirShowDemo(FSACRO_BOMBBURST6SHIP);
	}
	else if(item==mainMenu->acroChangeOverTurn)
	{
		Option_AirShowDemo(FSACRO_CHANGEOVERTURN);
	}
	else if(item==mainMenu->acroTrailtoDiamondRoll)
	{
		Option_AirShowDemo(FSACRO_TRAILTODIAMONDROLL);
	}
	else if(item==mainMenu->acroCubanEight)
	{
		Option_AirShowDemo(FSACRO_CUBANEIGHT);
	}
	else if(item==mainMenu->acroDeltaLoopandBonton)
	{
		Option_AirShowDemo(FSACRO_DELTALOOPANDBONTON);
	}
	else if(item==mainMenu->acroBontolRoll)
	{
		Option_AirShowDemo(FSACRO_BONTONROLL);
	}
	else if(item==mainMenu->acroBombBurstDownward4)
	{
		Option_AirShowDemo(FSACRO_BOMBBURSTDOWN4SHIP);
	}
	else if(item==mainMenu->acroBombBurstDownward6)
	{
		Option_AirShowDemo(FSACRO_BOMBBURSTDOWN6SHIP);
	}
	else if(item==mainMenu->acroRailFall)
	{
		Option_AirShowDemo(FSACRO_RAINFALL);
	}
	else if(item==mainMenu->acroRollingCombatPitch)
	{
		Option_AirShowDemo(FSACRO_ROLLINGCOMBATPITCH);
	}
	else if(item==mainMenu->acroDiamondTakeOff)
	{
		Option_AirShowDemo(FSACRO_DIAMONDTAKEOFF);
	}
	else if(item==mainMenu->acroContinuousRoll)
	{
		Option_AirShowDemo(FSACRO_CONTINUOUSROLL);
	}
	else if(item==mainMenu->acroRollOnTakeOff)
	{
		Option_AirShowDemo(FSACRO_ROLLONTAKEOFFANDHALFCUBAN);
	}
	else if(item==mainMenu->acroTackCross)
	{
		Option_AirShowDemo(FSACRO_TACKCROSSANDVERTICALCLIMBROLL);
	}
	else if(item==mainMenu->acroBigHeart)
	{
		Option_AirShowDemo(FSACRO_BIGHEART);
	}
	else if(item==mainMenu->acroLevelBreak)
	{
		Option_AirShowDemo(FSACRO_LEVELBREAK);
	}
	else if(item==mainMenu->acroRollBacktoArrowhead)
	{
		Option_AirShowDemo(FSACRO_ROLLBACKTOARROWHEAD);
	}
	else if(item==mainMenu->acroPitchUpBreak)
	{
		Option_AirShowDemo(FSACRO_PITCHUPBREAK);
	}
	else if(item==mainMenu->acroRockWingGearDown)
	{
		Option_AirShowDemo(FSACRO_ROCKWINGDIRTY);
	}
	else if(item==mainMenu->acroRockWingClean)
	{
		Option_AirShowDemo(FSACRO_ROCKWINGCLEAN);
	}
	else if(item==mainMenu->acroLetterEight)
	{
		Option_AirShowDemo(FSACRO_LETTEREIGHT);
	}
	else if(item==mainMenu->acroStarCross)
	{
		Option_AirShowDemo(FSACRO_STARCROSS);
	}
	else if(item==mainMenu->acroLevelOpener)
	{
		Option_AirShowDemo(FSACRO_LEVELOPENER);
	}
	else if(item==mainMenu->acroFormationBreak)
	{
		Option_AirShowDemo(FSACRO_FORMATIONBREAK);
	}
	else if(item==mainMenu->acroLineAbreastRoll)
	{
		Option_AirShowDemo(FSACRO_LINEABREASTROLL);
	}
	else if(item==mainMenu->acroLineAbreastLoop)
	{
		Option_AirShowDemo(FSACRO_LINEABREASTLOOP);
	}
	else if(item==mainMenu->acroDoubleFarvel)
	{
		Option_AirShowDemo(FSACRO_DOUBLEFARVEL);
	}
	else if(item==mainMenu->acroDiamond9toSwanBend)
	{
		Option_AirShowDemo(FSACRO_DIAMOND9TOSWANBEND);
	}
	else if(item==mainMenu->acroSwantoApolloRoll)
	{
		Option_AirShowDemo(FSACRO_SWANTOAPOLLOROLL);
	}
	else if(item==mainMenu->acroLancastertoSplit)
	{
		Option_AirShowDemo(FSACRO_LANCASTERTO5_4SPLIT);
	}
	else if(item==mainMenu->acroChampaignSplit)
	{
		Option_AirShowDemo(FSACRO_CHAMPAIGNSPLIT);
	}
	else if(item==mainMenu->acroVixenBreak)
	{
		Option_AirShowDemo(FSACRO_VIXENBREAK);
	}
	else if(item==mainMenu->acroBigBtlToDiamondLoop)
	{
		Option_AirShowDemo(FSACRO_BIGBATTLETOSHORTDIAMONDLOOP);
	}
}

void FsGuiMainCanvas::Option_AirShowDemo(int acroType)
{
	Option_AirShowDemo_AcroType=acroType;
	if(runLoop->GetWorld()->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Option_AirShowDemo_DeleteConfirmed,this);
		return;
	}
	Option_AirShowDemo_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Option_AirShowDemo_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if(returnCode==(int)YSOK)
	{
		FsNewFlightDialogOption opt;
		opt.canSelectWingmen=YSFALSE;
		opt.canChooseNight=YSFALSE;
		opt.canChooseFomType=YSFALSE;
		opt.flyNowButton=YSFALSE;
		auto dlg=StartNewFlightDialog(runLoop->GetWorld(),opt);
		dlg->BindCloseModalCallBack(&THISCLASS::Option_AirShowDemo_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Option_AirShowDemo_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiNewFlightDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		runLoop->StartAirShowDemo(dlg->info.fieldName,dlg->info.playerAirInfo.typeName,Option_AirShowDemo_AcroType);
	}
}
