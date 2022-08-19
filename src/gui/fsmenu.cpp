#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>
#include <ysclass.h>
#include <ysport.h>

#include "fsoption.h"
#include "fsapplyoption.h"

#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"
#include "fsmenu.h"

#include "graphics/common/fsopengl.h"
#include "fswirefont.h"



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

#include "fsmenu.h"
#include "fstextresource.h"

#include "fsguinewflightdialog.h"

#include "fsrunloop.h"
#include "fsguimaincanvas.h"


////////////////////////////////////////////////////////////


static void FsKeyFileSubMenu(FsWorld *world);
static void FsKeySimulationSubMenu(FsWorld *world);
static void FsKeyLandingPracticeMode(FsWorld *world);
static void FsKeyOptionSubMenu(FsWorld *world);


void FsKeySetEnglish(void);
void FsKeySetAutomaticLanguage(void);




// static int FsWaitKeyStroke(const class YsBitmap *bgBmp=NULL);
// static YSRESULT FsInputString(char str[],YSBOOL filename);



void FsGuiMainMenu::Make(void)
{
	pullDown=YSTRUE;

	{
		auto fileMenu=AddTextItem(0,FSKEY_F,FSMENU_FILE)->AddSubMenu();

		fileMenu->AddTextItem       (0,FSKEY_O,FSMENU_FILE_OPEN)->BindCallBack(&FsGuiMainCanvas::File_Open,canvas);
		fileMenu->AddTextItem       (0,FSKEY_S,FSMENU_FILE_SAVE)->BindCallBack(&FsGuiMainCanvas::File_Save,canvas);
		fileMenu->AddTextItem(0,FSKEY_M,FSMENU_FILE_OPENMISSION)->BindCallBack(&FsGuiMainCanvas::File_OpenMission,canvas);
		fileMenu->AddTextItem   (0,FSKEY_V,FSMENU_FILE_OPENPREV)->BindCallBack(&FsGuiMainCanvas::File_OpenPrevFlight,canvas);
		fileMenu->AddTextItem      (0,FSKEY_C,FSMENU_FILE_CLOSE)->BindCallBack(&FsGuiMainCanvas::File_CloseFlight,canvas);
		fileMenu->AddTextItem       (MakeIdent("file/exit"),FSKEY_X,FSMENU_FILE_EXIT)->BindCallBack(&FsGuiMainCanvas::File_Exit,canvas);

		this->fileRecent=fileMenu->AddTextItem(0,FSKEY_R,FSGUI_MENU_FILE_RECENT)->AddSubMenu();
		RefreshRecentlyUsedFileList();
	}



	{
		auto simMenu=AddTextItem(0,FSKEY_S,FSMENU_SIM)->AddSubMenu();
		simMenu->AddTextItem(MakeIdent("sim/fly"),      FSKEY_F,FSMENU_SIM_FLY)->BindCallBack(&FsGuiMainCanvas::Sim_Fly,canvas);
		simMenu->AddTextItem(MakeIdent("sim/retry"),    FSKEY_V,FSMENU_SIM_RETRY)->BindCallBack(&FsGuiMainCanvas::Sim_Retry,canvas);
		simMenu->AddTextItem(MakeIdent("sim/replay"),   FSKEY_R,FSMENU_SIM_REPLAY)->BindCallBack(&FsGuiMainCanvas::Sim_ReplayRecord,canvas);
		simMenu->AddTextItem(MakeIdent("sim/create"),   FSKEY_C,FSMENU_SIM_CREATE)->BindCallBack(&FsGuiMainCanvas::Sim_CreateFlight,canvas);
		simMenu->AddTextItem(MakeIdent("sim/env"),      FSKEY_D,FSMENU_SIM_DAYNIGHT)->BindCallBack(&FsGuiMainCanvas::Sim_SelectDayOrNight,canvas);
		simMenu->AddTextItem(MakeIdent("sim/disgnd"),   FSKEY_G,FSMENU_SIM_DISABLEGNDFIRE)->BindCallBack(&FsGuiMainCanvas::Sim_DisableGroundFire,canvas);
		simMenu->AddTextItem(MakeIdent("sim/aircomb"),  FSKEY_B,FSMENU_SIM_AIRCOMBAT)->BindCallBack(&FsGuiMainCanvas::Sim_CreateAirCombat,canvas);
		simMenu->AddTextItem(MakeIdent("sim/createg"),  FSKEY_G,FSMENU_SIM_CREATEINGND)->BindCallBack(&FsGuiMainCanvas::Sim_CreateInGroundObject,canvas);
		simMenu->AddTextItem(MakeIdent("sim/selair"),   FSKEY_A,FSMENU_SIM_SELECTAIR)->BindCallBack(&FsGuiMainCanvas::Sim_ChooseAircraft,canvas);
		simMenu->AddTextItem(MakeIdent("sim/selstp"),   FSKEY_T,FSMENU_SIM_SELECTSTP)->BindCallBack(&FsGuiMainCanvas::Sim_ChooseStartPosition,canvas);
		auto simLdgPracticeMenu=simMenu->AddTextItem(0,FSKEY_L,FSMENU_SIM_LDGPRACTICE)->AddSubMenu();
		simMenu->AddTextItem(MakeIdent("sim/endurance"),FSKEY_E,FSMENU_SIM_ENDURANCE)->BindCallBack(&FsGuiMainCanvas::Sim_EnduranceMode,canvas);
		simMenu->AddTextItem(MakeIdent("sim/intercept"),FSKEY_I,FSMENU_SIM_INTERCEPT)->BindCallBack(&FsGuiMainCanvas::Sim_InterceptMission,canvas);
		simMenu->AddTextItem(MakeIdent("sim/cas"),      FSKEY_S,FSMENU_SIM_CLOSEAIRSUPPORT)->BindCallBack(&FsGuiMainCanvas::Sim_CloseAirSupportMission,canvas);
		simMenu->AddTextItem(MakeIdent("sim/gndtoair"), FSKEY_A,FSMENU_SIM_GROUNDTOAIRMISSION)->BindCallBack(&FsGuiMainCanvas::Sim_GroundToAirMission,canvas);
		simMenu->AddTextItem(MakeIdent("sim/racing"),   FSKEY_NULL,FSGUI_MENU_SIM_RACINGMODE)->BindCallBack(&FsGuiMainCanvas::Sim_RacingMode,canvas);

		int numLdgPracLevel=0;
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_1,FSMENU_SIM_LDG_LEVEL01);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_2,FSMENU_SIM_LDG_LEVEL02);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_3,FSMENU_SIM_LDG_LEVEL03);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_4,FSMENU_SIM_LDG_LEVEL04);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_5,FSMENU_SIM_LDG_LEVEL05);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_6,FSMENU_SIM_LDG_LEVEL06);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_7,FSMENU_SIM_LDG_LEVEL07);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_8,FSMENU_SIM_LDG_LEVEL08);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_9,FSMENU_SIM_LDG_LEVEL09);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_A,FSMENU_SIM_LDG_LEVEL10);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_B,FSMENU_SIM_LDG_LEVEL11);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_C,FSMENU_SIM_LDG_LEVEL12);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_D,FSMENU_SIM_LDG_LEVEL13);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_E,FSMENU_SIM_LDG_LEVEL14);
		simLdgPracticeLevel[numLdgPracLevel++]=simLdgPracticeMenu->AddTextItem(0,FSKEY_F,FSMENU_SIM_LDG_LEVEL15);
		if(NUM_LANDING_PRACTICE_LEVEL!=numLdgPracLevel)
		{
			fprintf(stderr,"Menu Generation not correct!\n");
			fprintf(stderr,"There must be %d Landing-Practice Level.  Only %d added.\n",(int)NUM_LANDING_PRACTICE_LEVEL,numLdgPracLevel);
			exit(1);
		}
		for(int i=0; i<NUM_LANDING_PRACTICE_LEVEL; ++i)
		{
			simLdgPracticeLevel[i]->BindCallBack(&FsGuiMainCanvas::Sim_LandingPractice,canvas);
		}
	}



	{
		auto optMenu=AddTextItem(0,FSKEY_O,FSMENU_OPTION)->AddSubMenu();
		optMenu->AddTextItem(MkId("option/config"),FSKEY_C,FSMENU_OPTION_CONFIG)->BindCallBack(&FsGuiMainCanvas::Option_Config,canvas);
		optMenu->AddTextItem(MkId("option/option"),FSKEY_O,FSMENU_OPTION_OPTION)->BindCallBack(&FsGuiMainCanvas::Option_Option,canvas);
		optMenu->AddTextItem(MkId("option/keyAssign"),FSKEY_A,FSMENU_OPTION_KEYASSIGN)->BindCallBack(&FsGuiMainCanvas::Option_KeyAssign,canvas);
	#ifndef _WIN32
		optMenu->AddTextItem(0,FSKEY_J,FSMENU_OPTION_JOYCALIB)->BindCallBack(&FsGuiMainCanvas::Option_CalibrateJoystick,canvas);;
	#endif

		auto autoDemoMenu=optMenu->AddTextItem(0,FSKEY_D,FSMENU_AUTODEMO)->AddSubMenu();

		auto autoDemoAcroMenu=autoDemoMenu->AddTextItem(0,FSKEY_A,FSMENU_AUTODEMO_ACRO)->AddSubMenu();
		autoDemoMenu->AddTextItem(MkId("demo/dogFight"),FSKEY_D,FSMENU_AUTODEMO_DOGFIGHT)->BindCallBack(&FsGuiMainCanvas::Option_DogFightDemo,canvas);
		autoDemoMenu->AddTextItem(MkId("demo/landing"),FSKEY_L,FSMENU_AUTODEMO_LDG)->BindCallBack(&FsGuiMainCanvas::Option_LandingDemo,canvas);;
		autoDemoMenu->AddTextItem(MkId("demo/carrierLanding"),FSKEY_C,FSMENU_AUTODEMO_CARRIERLDG)->BindCallBack(&FsGuiMainCanvas::Option_CarrierLandingDemo,canvas);

		(acroStarOfDavid=        autoDemoAcroMenu->AddTextItem(MkId("acro/starofdavid"),FSKEY_NULL,FSGUI_MENU_AUTODEMO_ACRO_STAR_OF_DAVID))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);

		(acroCorkScrew=          autoDemoAcroMenu->AddTextItem(MkId("acro/corkscrew"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_CORKSCREW))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroSlowRoll=           autoDemoAcroMenu->AddTextItem(MkId("acro/slowroll"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_SLOWROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroDeltaLoop=          autoDemoAcroMenu->AddTextItem(MkId("acro/deltaloop"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_DELTALOOP))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroDeltaRoll=          autoDemoAcroMenu->AddTextItem(MkId("acro/deltaroll"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_DELTAROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroTightTurn=          autoDemoAcroMenu->AddTextItem(MkId("acro/steepTurn"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_TIGHTTURN))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acro360Loop=            autoDemoAcroMenu->AddTextItem(MkId("acro/360loop"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_360ANDLOOP))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroBombBurstUpward4=   autoDemoAcroMenu->AddTextItem(MkId("acro/bombburst4"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_BOMBBURSTUPWARD4))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroBombBurstUpward6=   autoDemoAcroMenu->AddTextItem(MkId("acro/bombburst6"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_BOMBBURSTUPWARD6))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroChangeOverTurn=     autoDemoAcroMenu->AddTextItem(MkId("acro/chgoverturn"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_CHANGEOVERTURN))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroTrailtoDiamondRoll= autoDemoAcroMenu->AddTextItem(MkId("acro/trailtodia"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_TRAILTODIAMONDROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroCubanEight=         autoDemoAcroMenu->AddTextItem(MkId("acro/cuban8"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_CUBANEIGHT))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroDeltaLoopandBonton= autoDemoAcroMenu->AddTextItem(MkId("acro/deltaloopbonton"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_DELTALOOPANDBONTON))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroBontolRoll=         autoDemoAcroMenu->AddTextItem(MkId("acro/bonton"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_BONTONROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroBombBurstDownward4= autoDemoAcroMenu->AddTextItem(MkId("acro/downburst4"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_BOMBBURSTDOWNWARD4))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroBombBurstDownward6= autoDemoAcroMenu->AddTextItem(MkId("acro/downburst6"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_BOMBBURSTDOWNWARD6))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroRailFall=           autoDemoAcroMenu->AddTextItem(MkId("acro/rainfall"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_RAINFALL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroRollingCombatPitch= autoDemoAcroMenu->AddTextItem(MkId("acro/rollingcombatpitch"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_ROLLINGCOMBATPITCH))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroDiamondTakeOff=     autoDemoAcroMenu->AddTextItem(MkId("acro/diatakeoff"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_DIAMONDTAKEOFF))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroContinuousRoll=     autoDemoAcroMenu->AddTextItem(MkId("acro/roll"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_CONTINUOUSROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroRollOnTakeOff=      autoDemoAcroMenu->AddTextItem(MkId("acro/rollontakeoff"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_ROLLONTAKEOFFANDHALFCUBAN))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroTackCross=          autoDemoAcroMenu->AddTextItem(MkId("acro/tackcross"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_TACKCROSSANDVERTCLIMBROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroBigHeart=           autoDemoAcroMenu->AddTextItem(MkId("acro/bigheart"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_BIGHEART))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroLevelBreak=         autoDemoAcroMenu->AddTextItem(MkId("acro/levelbreak"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_LEVELBREAK))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroRollBacktoArrowhead=autoDemoAcroMenu->AddTextItem(MkId("acro/rollbackarwhead"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_ROLLBACKTOARROWHEAD))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroPitchUpBreak=       autoDemoAcroMenu->AddTextItem(MkId("acro/pitchupbreak"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_PITCHUPBREAK))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroRockWingGearDown=   autoDemoAcroMenu->AddTextItem(MkId("acro/rockwinggear"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_ROCKWINGGEARDOWN))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroRockWingClean=      autoDemoAcroMenu->AddTextItem(MkId("acro/rockwing"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_ROCKWINGCLEAN))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroLetterEight=        autoDemoAcroMenu->AddTextItem(MkId("acro/letter8"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_LETTEREIGHT))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroStarCross=          autoDemoAcroMenu->AddTextItem(MkId("acro/starcross"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_STARCROSS))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroLevelOpener=        autoDemoAcroMenu->AddTextItem(MkId("acro/levelopener"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_LEVELOPENER))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroFormationBreak=     autoDemoAcroMenu->AddTextItem(MkId("acro/formationbreak"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_FORMATIONBREAK))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroLineAbreastRoll=    autoDemoAcroMenu->AddTextItem(MkId("acro/lineabreastroll"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_LINEABREASTROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroLineAbreastLoop=    autoDemoAcroMenu->AddTextItem(MkId("acro/lineabreastloop"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_LINEABREASTLOOP))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroDoubleFarvel=       autoDemoAcroMenu->AddTextItem(MkId("acro/doublefarvel"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_DOUBLEFARVEL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroDiamond9toSwanBend= autoDemoAcroMenu->AddTextItem(MkId("acro/dia9toswan"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_DIAMOND9TOSWAN))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroSwantoApolloRoll=   autoDemoAcroMenu->AddTextItem(MkId("acro/swantoapollo"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_SWANTOAPOLLOROLL))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroLancastertoSplit=   autoDemoAcroMenu->AddTextItem(MkId("acro/lancastersplit"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_LANCASTERTOFIVEFOURSPLIT))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroChampaignSplit=     autoDemoAcroMenu->AddTextItem(MkId("acro/champaignsplit"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_CHAMPAIGNSPLIT))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroVixenBreak=         autoDemoAcroMenu->AddTextItem(MkId("acro/vixenbreak"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_VIXENBREAK))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
		(acroBigBtlToDiamondLoop=autoDemoAcroMenu->AddTextItem(MkId("acro/bigbattletodia"),FSKEY_NULL,FSMENU_AUTODEMO_ACRO_BIGBATTLETOSHORTDIAMONDLOOP))->BindCallBack(&FsGuiMainCanvas::Option_AirShowDemo,canvas);
	}

	{
		auto netMenu=AddTextItem(0,FSKEY_N,FSMENU_NET)->AddSubMenu();
		netMenu->AddTextItem(MkId("network/server"),FSKEY_S,FSMENU_NET_SERVER)->BindCallBack(&FsGuiMainCanvas::Net_StartServerMode,canvas);
		netMenu->AddTextItem(MkId("network/client"),FSKEY_C,FSMENU_NET_CLIENT)->BindCallBack(&FsGuiMainCanvas::Net_StartClientMode,canvas);
		netMenu->AddTextItem(MkId("network/config"),FSKEY_O,FSMENU_NET_CONFIG)->BindCallBack(&FsGuiMainCanvas::Net_Config,canvas);
	}

	{
		auto recUtilMenu=AddTextItem(0,FSKEY_U,FSMENU_FLTRECUTIL)->AddSubMenu();
		recUtilMenu->AddTextItem(0,FSKEY_T,FSMENU_FLTRECUTIL_TIMEINTERVAL)->BindCallBack(&FsGuiMainCanvas::UtilSetTimeInterval,canvas);
		recUtilMenu->AddTextItem(0,FSKEY_S,FSMENU_FLTRECUTIL_SPACEPRECISION)->BindCallBack(&FsGuiMainCanvas::UtilSetSpacePrecision,canvas);
		recUtilMenu->AddTextItem(0,FSKEY_E,FSMENU_FLTRECUTIL_EDITRECORD)->BindCallBack(&FsGuiMainCanvas::Util_EditFlightRecord,canvas);
	}



	{
		auto helpMenu=AddTextItem(0,FSKEY_H,FSMENU_HELP)->AddSubMenu();
		helpE=helpMenu->AddTextItem      (0,FSKEY_E,FSMENU_HELP_HELPENGLISH);
		helpDefKeyE=helpMenu->AddTextItem(0,FSKEY_N,FSMENU_HELP_DEFKEYENGLISH);
		helpJ=helpMenu->AddTextItem      (0,FSKEY_J,FSMENU_HELP_HELPJAPANESE);
		helpDefKeyJ=helpMenu->AddTextItem(0,FSKEY_P,FSMENU_HELP_DEFKEYJAPANESE);
		helpAbout=helpMenu->AddTextItem  (0,FSKEY_A,FSMENU_HELP_ABOUT);
		helpSupport=helpMenu->AddTextItem(0,FSKEY_S,FSMENU_HELP_SUPPORTYSFLIGHT);
	}



	// auto learningSubMenu=AddTextItem(0,FSKEY_L,FSMENU_LEARNING)->AddSubMenu();
	// learningSubMenu->AddTextItem(0,FSKEY_S,FSGUI_MENU_LEARNING_SAVE,FsRunLoop::Learning_SaveTrainingData);
	// learningSubMenu->AddTextItem(0,FSKEY_O,FSGUI_MENU_LEARNING_OPEN,FsRunLoop::Learning_OpenTrainingData);
	// learningSubMenu->AddTextItem(0,FSKEY_C,FSGUI_MENU_LEARNING_CLOSE,FsRunLoop::Learning_CloseTrainingData);
	// learningSubMenu->AddTextItem(0,FSKEY_G,FSMENU_LEARNING_TRAINAUTOPILOT,FsRunLoop::Learning_TrainAutoPilot);
	// learningSubMenu->AddTextItem(0,FSKEY_X,L"Experimental",FsRunLoop::Learning_SetUpExperiment);
}

void FsGuiMainMenu::RefreshRecentlyUsedFileList(void)
{
	YsWString recentFn(FsGetRecentlyUsedFile());
	FILE *fp=YsFileIO::Fopen(recentFn,"r");
	if(NULL!=fp)
	{
		YsTextFileInputStream inStream(fp);
		recent.Open(inStream);
		fclose(fp);

		recent.PopulateMenu(*fileRecent,16,&FsGuiMainCanvas::File_Recent,canvas);
	}
}

void FsGuiMainMenu::AddRecentlyUsedFile(const wchar_t wfn[])
{
	recent.AddFile(wfn);
	recent.PopulateMenu(*fileRecent,16,&FsGuiMainCanvas::File_Recent,canvas);

	YsWString recentFn(FsGetRecentlyUsedFile());  // FsGetRecentlyUsedFile also creates the directory for the recent files.
	FILE *fp=YsFileIO::Fopen(recentFn,"w");

	if(NULL!=fp)
	{
		YsTextFileOutputStream outStream(fp);
		recent.Save(outStream,16);
		fclose(fp);
	}
}


void FsGuiMainMenu::OnSelectMenuItem(FsGuiPopUpMenuItem *item)
{
	if(item==helpJ)
	{
		FsHelpGeneralHelpJapanese();
	}
	else if(item==helpDefKeyJ)
	{
		FsHelpControlJapanese();
	}
	else if(item==helpE)
	{
		FsHelpGeneralHelpEnglish();
	}
	else if(item==helpDefKeyE)
	{
		FsHelpControlEnglish();
	}
	else if(item==helpAbout)
	{
		canvas->StartAboutDialog();
	}
	else if(item==helpSupport)
	{
		canvas->StartSupportYsflightDialog(YSFALSE,0);
	}



	YSBOOL lb,mb,rb;
	int mx,my;
	FsPollDevice();
	while(FsGetMouseEvent(lb,mb,rb,mx,my)!=FSMOUSEEVENT_NONE)
	{
		FsPollDevice();
	}
}

void FsGuiMainDialog::Make(void)
{
	SetTopLeftCorner(16,48);

	SetTextMessage("-- YSFLIGHT --");

	statusMsg=AddStaticText(0,FSKEY_NULL,"(Status)",80,1,YSTRUE);
	newFlightBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_MAINDLG_CREATE,YSTRUE);
	retryPrevMissionBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_MAINDLG_RETRY,YSFALSE);
	loadFlightBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_MAINDLG_OPEN,YSTRUE);
	saveFlightBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_MAINDLG_SAVE,YSFALSE);
	flyNowBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_MAINDLG_FLYNOW,YSTRUE);
	replayRecordBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_MAINDLG_PLAYRECORD,YSFALSE);

	englishBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_ENGLISH,YSTRUE);
	languageBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_AUTOLANGUAGE,YSFALSE);

	showConsoleBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_MAINDLG_SHOWCONSOLE,YSTRUE);

	if(FsGetDate()<=20100315)
	{
		AddStaticText(0,FSKEY_NULL,FSGUI_VOTEYSFLIGHTDLG_MESSAGE,YSTRUE);

		votePageBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,
		    "http://www.vector.co.jp/award/vote.html?no=se121250&vasts=vote",YSTRUE);
	}

	SetTransparency(YSTRUE);
	Fit();
}

void FsGuiMainDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==newFlightBtn)
	{
		canvas->Sim_CreateFlight(nullptr);
	}
	else if(btn==loadFlightBtn)
	{
		canvas->File_Open(nullptr);
	}
	else if(btn==saveFlightBtn)
	{
		canvas->File_Save(nullptr);
	}
	else if(btn==flyNowBtn)
	{
		canvas->Sim_Fly(nullptr);
	}
	else if(btn==replayRecordBtn)
	{
		canvas->Sim_ReplayRecord(nullptr);
	}
	else if(btn==retryPrevMissionBtn)
	{
		canvas->Sim_Retry(nullptr);
	}
	else if(btn==englishBtn)
	{
		FsKeySetEnglish();
		runLoop->SetNeedReloadOption(YSTRUE);
	}
	else if(btn==languageBtn)
	{
		FsKeySetAutomaticLanguage();
		runLoop->SetNeedReloadOption(YSTRUE);
	}
	else if(btn==votePageBtn)
	{
	    YsOpenURL("http://www.vector.co.jp/award/vote.html?no=se121250&vasts=vote");
	}



	YSBOOL lb,mb,rb;
	int mx,my;
	FsPollDevice();
	while(FsGetMouseEvent(lb,mb,rb,mx,my)!=FSMOUSEEVENT_NONE)
	{
		FsPollDevice();
	}
}



////////////////////////////////////////////////////////////

// void FsKeyMenu is now a FsRunLoop::RunMenuOneStep

////////////////////////////////////////////////////////////

// I should be able to get rid of this blocking message box,
// but fsglx.cpp is still calling it.
YSBOOL FsKeyMessageBox(const char title[],const char msg[],const char *yesBtnTxt,const char *noBtnTxt)
{
	FsGuiDialog dlg;
	FsGuiDialogItem *yesBtn,*noBtn,*btn;
	int x1,y1;
	int wid,hei;

	FsGetWindowSize(wid,hei);
	dlg.SetSize(wid,hei);

	dlg.SetTransparency(YSFALSE);

	dlg.SetTopLeftCorner(16,16);
	if(title!=NULL)
	{
		dlg.SetTextMessage(title);
	}

	dlg.AddStaticText(0,FSKEY_NULL,msg,YSTRUE);

	yesBtn=NULL;
	noBtn=NULL;

	yesBtn=dlg.AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,yesBtnTxt,YSTRUE);

	x1=yesBtn->x0+yesBtn->wid;
	y1=yesBtn->y0+yesBtn->hei;
	if(noBtnTxt!=NULL && 0!=noBtnTxt[0])
	{
		noBtn=dlg.AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,noBtnTxt,YSFALSE);
		x1=noBtn->x0+noBtn->wid;
		y1=noBtn->y0+noBtn->hei;
	}

	dlg.Fit();

	int key;
	int mx,my;
	YSBOOL lb,mb,rb;
	YsColor titleCol;

	lb=YSFALSE;
	mb=YSFALSE;
	rb=YSFALSE;


	FsPollDevice();
	while(FsInkey()!=FSKEY_NULL)
	{
		FsPollDevice();
	}

	for(;;)
	{
		FsPollDevice();


		FsClearScreenAndZBuffer(YsGrayScale(0.25));
		FsSet2DDrawing();
		dlg.Show();
		FsSwapBuffers();

		for(;;)
		{
			int eventType=FsGetMouseEvent(lb,mb,rb,mx,my);
			dlg.SetMouseState(lb,mb,rb,mx,my);
			if(FSMOUSEEVENT_NONE==eventType)
			{
				break;
			}
		}

		key=FsInkey();
		dlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
		dlg.CharIn(FsInkeyChar());

		btn=dlg.GetClickedItem();

		if(yesBtn!=NULL && noBtn!=NULL)  // Yes/No dialog
		{
			if(key==FSKEY_Y || yesBtn==btn)
			{
				return YSTRUE;
			}
			else if(key==FSKEY_N || noBtn==btn)
			{
				return YSFALSE;
			}
		}
		else if(yesBtn!=NULL && noBtn==NULL) // Simple dialog
		{
			if(key==FSKEY_SPACE || yesBtn==btn)
			{
				return YSTRUE;
			}
		}


		FsSleep(25);
	}
}



////////////////////////////////////////////////////////////



void FsKeySetEnglish(void)
{
	FsOption opt;
	opt.Load(FsGetOptionFile());
	opt.languageType=FsOption::FORCEENGLISH;
	opt.Save(FsGetOptionFile());
	FsMakeLocalizationFromOption(opt);
}

void FsKeySetAutomaticLanguage(void)
{
	FsOption opt;
	opt.Load(FsGetOptionFile());
	opt.languageType=FsOption::AUTOMATIC;
	if(opt.fontHeight<12)
	{
		opt.fontHeight=12;
	}
	opt.Save(FsGetOptionFile());
	FsMakeLocalizationFromOption(opt);
}



