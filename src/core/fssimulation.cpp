#include <ysclass.h>
#include <ysunitconv.h>
#include <ysport.h>
#include <fsgui.h>
#include <assert.h>

#include <ysglparticlemanager.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <fsairproperty.h>

#include "fsconfig.h"

#include "fs.h"
#include "fsradar.h"
#include "fsfilename.h"
#include "fsinstpanel.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"

#include "fspluginmgr.h"
#include "graphics/common/fsfontrenderer.h"
#include "fsguiinfltdlg.h"

#include "fstextresource.h"
#include "ysbitmap.h"

#include "fsinstreading.h"

#include "fsrefuelandtakeoff.h"

#ifndef _WIN32  // Assuming UNIX

#include <sys/time.h>
#endif

#include <time.h>



#include "fsgui.h"
#include "fsguiselectiondialogbase.h"
#include <fsguifiledialog.h>

#include "fschoose.h"

#include "fspersona.h"

#include "fstexturemanager.h"

#ifdef ANDROID
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG,"Ys",__VA_ARGS__)
#endif

// #define CRASHINVESTIGATION
// #define CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
// #define CRASHINVESTIGATION_S1_LEVEL2
// #define CRASHINVESTIGATION_S8_LEVEL2
// #define CRASHINVESTIGATION_SIMDRAWFOREGROUND
// #define CRASHINVESTIGATION_SIMDRAWSCREEN
// #define CRASHINVESTIGATION_SIMCONTROLBYUSER
// #define CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE





YsListAllocator <FsAirplane> FsAirplaneAllocator;
YsListAllocator <FsGround> FsGroundAllocator;



#ifdef SHAREWARE
extern void DrawSharewareMessage(void);
#endif



FsVisualSrf *cockpit=nullptr;

////////////////////////////////////////////////////////////

FsTimedMessage::FsTimedMessage()
{
	timeRemain=0.0;
}

////////////////////////////////////////////////////////////

FsSimulation::ActualViewMode::ActualViewMode()
{
	actualViewMode=FSCOCKPITVIEW;
	actualViewHdg=0.0;
	actualViewPch=0.0;

	viewPoint=YsVec3::Origin();
	viewAttitude=YsZeroAtt();

	viewMagFix=1.0;
	isViewPointInCloud=YSFALSE;
	fogVisibility=0.0;

	viewTargetDist=0.0;
}

////////////////////////////////////////////////////////////

FsHasInFlightDialog::FsHasInFlightDialog()
{
	currentInFlightDlg=NULL;
}

void FsHasInFlightDialog::SetCurrentInFlightDialog(FsGuiInFlightDialog *dlg)
{
	currentInFlightDlg=dlg;
	if(nullptr==dlg)
	{
		FsDisableIME();
	}
}

FsGuiInFlightDialog *FsHasInFlightDialog::GetCurrentInFlightDialog(void) const
{
	return currentInFlightDlg;
}

////////////////////////////////////////////////////////////

FsSimulation::FsSimulation(const FsSimulation &)
	 : airplaneList(FsAirplaneAllocator),groundList(FsGroundAllocator),bulletHolder(nullptr)
{
}

FsSimulation::FsSimulation(FsWorld *w) : airplaneList(FsAirplaneAllocator),groundList(FsGroundAllocator),bulletHolder(this)
{
	world=w;

	needRedraw=YSFALSE;
	firstPlayer.Initialize();

	cfgPtr=new FsFlightConfig;
	hud=new FsHeadUpDisplay;
	hud2=new FsHud2;
	groundSky=new FsGroundSky;
	weather=new FsWeather;
	iniWeather=new FsWeather;
	cloud=new FsClouds;
	solidCloud=new FsSolidClouds;
	goal=new FsMissionGoal;
	goal->SetIsActiveMission(YSFALSE);  // By default, FsSimulation doesn't have an active mission.
	simEvent=new FsSimulationEventStore;

	airTrafficSequence=FsAirTrafficSequence::Create();

	// In-Game Message Bitmap >>
	escTwiceToEnd=new YsBitmap;
	fsUnicodeRenderer.RenderString(*escTwiceToEnd,FSGAMEMSG_ESCTWICETOEND,YsGreen(),YsBlack());
	escOnceToEnd=new YsBitmap;
	fsUnicodeRenderer.RenderString(*escOnceToEnd,FSGAMEMSG_ESCONCETOEND,YsGreen(),YsBlack());
	// In-Game Message Bitmap <<


	localUser=new FsLocalUser;

	SetTerminate(YSFALSE);
	pause=YSFALSE;
	canContinue=YSTRUE;

	escKeyCount=0;
	currentTime=0.0;
	aircraftTroubleTimer=0.0;
	lastTime=0;
	mainWindowViewmode=FSCOCKPITVIEW;
	mainWindowAdditionalAirplaneViewId=0;
	mainWindowActualViewMode.viewPoint=YsOrigin();
	mainWindowActualViewMode.viewAttitude=YsZeroAtt();
	for(auto &swavm : subWindowActualViewMode)
	{
		swavm.viewPoint=YsOrigin();
		swavm.viewAttitude=YsZeroAtt();
	}
	viewAttitudeTransition=YsZeroAtt();
	viewMagUser=1.0;
	ghostViewSpeed=0.0;
	relViewAtt.Set(0.0,-YsPi/9.0,0.0);
	relViewDist=2.0;
	focusAir=NULL;
	focusAir2=NULL;
	focusGnd=NULL;
	towerViewId=0;
	towerViewPos=YsOrigin();

	for(int i=0; i<FsMaxNumSubWindow; i++)
	{
		subWindowViewmode[i]=FSCOCKPITVIEW;
	}

	tallestGroundObjectHeight=0.0;

	ltc.SetUp(YsVec2(-20000.0,-20000.0),YsVec2(20000.0,20000.0),64,64);  // 64x2500m=160000 sq m

	systemMessage[0]=0;

	endTime=0.0;

	nextFpsUpdateTime=0;
	fps=0.0;
	nFrameForFpsCount=0;
	lastFpsUpdateTime=0;

	allowedWeaponType=(unsigned int)FSWEAPON_ALLOWALLWEAPON;

	if(nullptr==cockpit)
	{
		cockpit=new FsVisualSrf;
		if(cockpit->Load(L"aircraft/cockpit1.srf")!=YSOK)
		{
			delete cockpit;
			cockpit=nullptr;
		}
	}

	netServer=NULL;
	netClient=NULL;

	for(int i=0; i<FSNTIMEMARKER; i++)
	{
		timeMarker[i]=0.0;
	}

	subMenu.CleanUp();

	env=FSDAYLIGHT;

	airplaneSearch=new YsHashTable <FsAirplane *> (128);
	groundSearch=new YsHashTable <FsGround *> (128);

	showUserNameMasterSwitch=YSTRUE;

	fieldLoaded=YSFALSE;

	showReplayDlg=YSFALSE;
	replayDlg=NULL;

	contDlg=NULL;

	FsGuiChooseAircraftOption opt;

	SetCurrentInFlightDialog(NULL);

	loadingDlg=new FsGuiChooseAircraft;
	loadingDlg->Initialize();
	loadingDlg->createSearch=YSFALSE;
	loadingDlg->selectListBoxRow=1;
	loadingDlg->showAirplane=YSFALSE;
	loadingDlg->Create(w,opt,0);
	loadingDlg->mainTab->SelectCurrentTab(loadingDlg->loadingTabId);

	chatDlg=FsGuiChatDialog::Create();
	chatDlg->MakeDialog(this);

	stationaryDlg=FsGuiStationaryDialog::Create();
	stationaryDlg->Make(this);

	vehicleChangeDlg=FsGuiVehicleChangeDialog::Create();
	vehicleChangeDlg->Make(this);

	autoPilotDlg=FsGuiAutoPilotDialog::Create();
	autoPilotDlg->Make(this);

	autoPilotVTOLDlg=FsGuiAutoPilotVTOLDialog::Create();
	autoPilotVTOLDlg->Make(this);

	autoPilotHelicopterDlg=FsGuiAutoPilotHelicopterDialog::Create();
	autoPilotHelicopterDlg->Make(this);

	callFuelTruckDlg=FsGuiRadioCommFuelTruckDialog::Create();
	callFuelTruckDlg->Make(this);

	radioCommTargetDlg=FsGuiRadioCommTargetDialog::Create();
	// radioCommTargetDlg must be made in SetUpRadioCommTargetDialog.  Choices may change.

	radioCommToFomDlg=FsGuiRadioCommToFormationDialog::Create();
	radioCommToFomDlg->Make(this);

	radioCommCmdDlg=FsGuiRadioCommCommandDialog::Create();
	radioCommCmdDlg->Make(this);

	requestApproachDlg=FsGuiSelectApproachDialog::Create();
	requestApproachDlg->Make(this);

	atcRequestDlg=FsGuiAtcRequestDialog::Create();
	// atcRequestDlg must be Made in SetUpAtcRequestDialog.  Number of buttons changes.


	primaryAtc.AssignSearchKey(FsAirTrafficController::PrimaryAirTrafficControllerKey);
	primaryAtc.SetAllJurisdiction();

	centerJoystick=NULL;

	fogColor.SetDoubleRGB(0.6,0.6,0.6);
	gndColor.SetIntRGB(0,0,160);
	gndSpecular=YSFALSE;
	skyColor.SetIntRGB(0,128,192);
}

FsSimulation::~FsSimulation()
{
	DeleteCenterJoystick();

	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		air->ClearCollisionShell();
	}

	FsGround *gnd;
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		gnd->ClearCollisionShell();
		gnd->Prop().CleanUpAircraftCarrierProperty();
	}

	airplaneList.CleanUp();
	groundList.CleanUp();
	field.CleanUp();

	FsAirplaneAllocator.CollectGarbage();
	FsGroundAllocator.CollectGarbage();

	delete airplaneSearch;
	delete groundSearch;

	delete hud;
	delete hud2;
	delete groundSky;
	delete weather;
	delete iniWeather;
	delete solidCloud;
	delete cloud;
	delete goal;
	delete simEvent;

	delete localUser;

	FsAirTrafficSequence::Delete(airTrafficSequence);

	delete cfgPtr;

#ifndef YS_SCSV
	if(replayDlg!=NULL)
	{
		FsReplayDialog::Delete(replayDlg);
	}
	if(loadingDlg!=NULL)
	{
		delete loadingDlg;
	}
#endif

	FsGuiStationaryDialog::Delete(stationaryDlg);
	FsGuiVehicleChangeDialog::Delete(vehicleChangeDlg);
	FsGuiAutoPilotDialog::Delete(autoPilotDlg);
	FsGuiAutoPilotVTOLDialog::Delete(autoPilotVTOLDlg);
	FsGuiAutoPilotHelicopterDialog::Delete(autoPilotHelicopterDlg);
	FsGuiRadioCommFuelTruckDialog::Delete(callFuelTruckDlg);
	FsGuiRadioCommCommandDialog::Delete(radioCommCmdDlg);
	FsGuiRadioCommToFormationDialog::Delete(radioCommToFomDlg);
	FsGuiSelectApproachDialog::Delete(requestApproachDlg);
	FsGuiAtcRequestDialog::Delete(atcRequestDlg);
	FsGuiRadioCommTargetDialog::Delete(radioCommTargetDlg);



	// printf("FsSimulation::~FsSimulation()\n");
	// printf("Diagnose airplaneList\n");
	// airplaneList.SelfDiagnostic();
	// printf("Diagnose groundList\n");
	// groundList.SelfDiagnostic();
	// printf("Diagnose fieldList\n");
	// fieldList.SelfDiagnostic();
	// printf("Diagnose FsAirplaneAllocator\n");
	// FsAirplaneAllocator.SelfDiagnostic();
	// printf("Diagnose FsGroundAllocator\n");
	// FsGroundAllocator.SelfDiagnostic();
	// printf("Diagnose FsFieldAllocator\n");
	// FsFieldAllocator.SelfDiagnostic();
}

/* static */ const char *FsSimulation::ViewmodeToStr(FSVIEWMODE viewmode)
{
	switch(viewmode)
	{
	case FSCOCKPITVIEW:
		return "FSCOCKPITVIEW";
	case FSOUTSIDEPLAYERPLANE:
		return "FSOUTSIDEPLAYERPLANE";
	case FSFIXEDPOINTPLAYERPLANE:
		return "FSFIXEDPOINTPLAYERPLANE";
	case FSVARIABLEPOINTPLAYERPLANE:
		return "FSVARIABLEPOINTPLAYERPLANE";
	case FSFROMTOPOFPLAYERPLANE:
		return "FSFROMTOPOFPLAYERPLANE";
	case FSANOTHERAIRPLANE:
		return "FSANOTHERAIRPLANE";
	case FSMISSILEVIEW:
		return "FSMISSILEVIEW";
	case FSAIRTOAIRVIEW:
		return "FSAIRTOAIRVIEW";
	case FSAIRFROMAIRVIEW:
		return "FSAIRFROMAIRVIEW";
	case FSPLAYERPLANEFROMSIDE:
		return "FSPLAYERPLANEFROMSIDE";
	case FSCARRIERVIEW:
		return "FSCARRIERVIEW";
	case FSTESTVIEW1:
		return "FSTESTVIEW1";
	case FSTESTVIEW2:
		return "FSTESTVIEW2";
	case FSOUTSIDEPLAYER2:
		return "FSOUTSIDEPLAYER2";
	case FSOUTSIDEPLAYER3:
		return "FSOUTSIDEPLAYER3";
	case FSBOMBINGVIEW:
		return "FSBOMBINGVIEW";
	case FSTOWERVIEW:
		return "FSTOWERVIEW";
	case FSPLAYERTOGNDVIEW:
		return "FSPLAYERTOGNDVIEW";
	case FSGNDTOPLAYERVIEW:
		return "FSGNDTOPLAYERVIEW";
	case FSSPOTPLANEVIEW:
		return "FSSPOTPLANEVIEW";

	case FSMYWEAPONVIEW_OLD:
		return "FSMYWEAPONVIEW_OLD";
	case FSMYWEAPONVIEW_NEW:
		return "FSMYWEAPONVIEW_NEW";
	case FSBACKMIRRORVIEW:
		return "FSBACKMIRRORVIEW";
	case FS45DEGREERIGHTVIEW:
		return "FS45DEGREERIGHTVIEW";
	case FS45DEGREELEFTVIEW:
		return "FS45DEGREELEFTVIEW";
	case FS90DEGREERIGHTVIEW:
		return "FS90DEGREERIGHTVIEW";
	case FS90DEGREELEFTVIEW:
		return "FS90DEGREELEFTVIEW";
	case FSTELESCOPEVIEW:
		return "FSTELESCOPEVIEW";
	case FSLOCKEDTARGETVIEW:
		return "FSLOCKEDTARGETVIEW";
	case FSGHOSTVIEW:
		return "FSGHOSTVIEW";

	case FSAIRTOTOWERVIEW:
		return "FSAIRTOTOWERVIEW";
	case FSAIRTOTOWERVIEWSOLO:
		return "FSAIRTOTOWERVIEWSOLO";
	case FSTOWERVIEW_NOMAGNIFY:
		return "FSTOWERVIEW_NOMAGNIFY";

	case FSVERTICALORBITINGVIEW:
		return "FSVERTICALORBITINGVIEW";
	case FSHORIZONTALORBITINGVIEW:
		return "FSHORIZONTALORBITINGVIEW";
	case FSTURNVIEW:
		return "FSTURNVIEW";

	case FSADDITIONALAIRPLANEVIEW:
		return "FSADDITIONALAIRPLANEVIEW";
	case FSADDITIONALAIRPLANEVIEW_CABIN:
		return "FSADDITIONALAIRPLANEVIEW_CABIN";
	default:
		break;
	}
	return "Unknown_Viewmode";
}
/* static */ FsSimulation::FSVIEWMODE FsSimulation::StrToViewmode(const char *str)
{
	if(0==YsString::STRCMP("FSCOCKPITVIEW",str))
	{
		return FSCOCKPITVIEW;
	}
	if(0==YsString::STRCMP("FSOUTSIDEPLAYERPLANE",str))
	{
		return FSOUTSIDEPLAYERPLANE;
	}
	if(0==YsString::STRCMP("FSFIXEDPOINTPLAYERPLANE",str))
	{
		return FSFIXEDPOINTPLAYERPLANE;
	}
	if(0==YsString::STRCMP("FSVARIABLEPOINTPLAYERPLANE",str))
	{
		return FSVARIABLEPOINTPLAYERPLANE;
	}
	if(0==YsString::STRCMP("FSFROMTOPOFPLAYERPLANE",str))
	{
		return FSFROMTOPOFPLAYERPLANE;
	}
	if(0==YsString::STRCMP("FSANOTHERAIRPLANE",str))
	{
		return FSANOTHERAIRPLANE;
	}
	if(0==YsString::STRCMP("FSMISSILEVIEW",str))
	{
		return FSMISSILEVIEW;
	}
	if(0==YsString::STRCMP("FSAIRTOAIRVIEW",str))
	{
		return FSAIRTOAIRVIEW;
	}
	if(0==YsString::STRCMP("FSAIRFROMAIRVIEW",str))
	{
		return FSAIRFROMAIRVIEW;
	}
	if(0==YsString::STRCMP("FSPLAYERPLANEFROMSIDE",str))
	{
		return FSPLAYERPLANEFROMSIDE;
	}
	if(0==YsString::STRCMP("FSCARRIERVIEW",str))
	{
		return FSCARRIERVIEW;
	}
	if(0==YsString::STRCMP("FSTESTVIEW1",str))
	{
		return FSTESTVIEW1;
	}
	if(0==YsString::STRCMP("FSTESTVIEW2",str))
	{
		return FSTESTVIEW2;
	}
	if(0==YsString::STRCMP("FSOUTSIDEPLAYER2",str))
	{
		return FSOUTSIDEPLAYER2;
	}
	if(0==YsString::STRCMP("FSOUTSIDEPLAYER3",str))
	{
		return FSOUTSIDEPLAYER3;
	}
	if(0==YsString::STRCMP("FSBOMBINGVIEW",str))
	{
		return FSBOMBINGVIEW;
	}
	if(0==YsString::STRCMP("FSTOWERVIEW",str))
	{
		return FSTOWERVIEW;
	}
	if(0==YsString::STRCMP("FSPLAYERTOGNDVIEW",str))
	{
		return FSPLAYERTOGNDVIEW;
	}
	if(0==YsString::STRCMP("FSGNDTOPLAYERVIEW",str))
	{
		return FSGNDTOPLAYERVIEW;
	}
	if(0==YsString::STRCMP("FSSPOTPLANEVIEW",str))
	{
		return FSSPOTPLANEVIEW;
	}

	if(0==YsString::STRCMP("FSMYWEAPONVIEW_OLD",str))
	{
		return FSMYWEAPONVIEW_OLD;
	}
	if(0==YsString::STRCMP("FSMYWEAPONVIEW_NEW",str))
	{
		return FSMYWEAPONVIEW_NEW;
	}
	if(0==YsString::STRCMP("FSBACKMIRRORVIEW",str))
	{
		return FSBACKMIRRORVIEW;
	}
	if(0==YsString::STRCMP("FS45DEGREERIGHTVIEW",str))
	{
		return FS45DEGREERIGHTVIEW;
	}
	if(0==YsString::STRCMP("FS45DEGREELEFTVIEW",str))
	{
		return FS45DEGREELEFTVIEW;
	}
	if(0==YsString::STRCMP("FS90DEGREERIGHTVIEW",str))
	{
		return FS90DEGREERIGHTVIEW;
	}
	if(0==YsString::STRCMP("FS90DEGREELEFTVIEW",str))
	{
		return FS90DEGREELEFTVIEW;
	}
	if(0==YsString::STRCMP("FSTELESCOPEVIEW",str))
	{
		return FSTELESCOPEVIEW;
	}
	if(0==YsString::STRCMP("FSLOCKEDTARGETVIEW",str))
	{
		return FSLOCKEDTARGETVIEW;
	}
	if(0==YsString::STRCMP("FSGHOSTVIEW",str))
	{
		return FSGHOSTVIEW;
	}

	if(0==YsString::STRCMP("FSAIRTOTOWERVIEW",str))
	{
		return FSAIRTOTOWERVIEW;
	}
	if(0==YsString::STRCMP("FSAIRTOTOWERVIEWSOLO",str))
	{
		return FSAIRTOTOWERVIEWSOLO;
	}
	if(0==YsString::STRCMP("FSTOWERVIEW_NOMAGNIFY",str))
	{
		return FSTOWERVIEW_NOMAGNIFY;
	}

	if(0==YsString::STRCMP("FSVERTICALORBITINGVIEW",str))
	{
		return FSVERTICALORBITINGVIEW;
	}
	if(0==YsString::STRCMP("FSHORIZONTALORBITINGVIEW",str))
	{
		return FSHORIZONTALORBITINGVIEW;
	}
	if(0==YsString::STRCMP("FSTURNVIEW",str))
	{
		return FSTURNVIEW;
	}

	if(0==YsString::STRCMP("FSADDITIONALAIRPLANEVIEW",str))
	{
		return FSADDITIONALAIRPLANEVIEW;
	}
	if(0==YsString::STRCMP("FSADDITIONALAIRPLANEVIEW_CABIN",str))
	{
		return FSADDITIONALAIRPLANEVIEW_CABIN;
	}
	return FSCOCKPITVIEW;
}

void FsSimulation::SetSimulationTitle(const char str[])
{
	simTitle.Set(str);
}

void FsSimulation::SetCanContinue(YSBOOL canContinue)
{
	this->canContinue=canContinue;
}

YSBOOL FsSimulation::GetCanContinue(void) const
{
	return canContinue;
}

void FsSimulation::CreateCenterJoystick(void)
{
	DeleteCenterJoystick();
	centerJoystick=new FsCenterJoystick;
	MakeCenterJoystickDialog(*centerJoystick,0);
}

void FsSimulation::DeleteCenterJoystick(void)
{
	if(NULL!=centerJoystick)
	{
		delete centerJoystick;
		centerJoystick=NULL;
	}
}

void FsSimulation::CenterJoystickOneStep(FSSIMULATIONSTATE &simState)
{
	if(NULL==centerJoystick)
	{
		CreateCenterJoystick();
	}
	centerJoystick->RunOneStep();
	if(FsCenterJoystick::OVER==centerJoystick->state)
	{
		DeleteCenterJoystick();
		simState=FSSIMSTATE_INITIALIZE;
	}
}

void FsSimulation::CenterJoystickDraw(void) const
{
	if(nullptr!=centerJoystick)
	{
		centerJoystick->Draw();
	}
}

YSBOOL FsSimulation::Paused(void) const
{
	return pause;
}

FsWorld *FsSimulation::GetWorldPtr(void)
{
	return world;
}

unsigned int FsSimulation::GetAllowedWeaponType(void) const
{
	return allowedWeaponType;
}

double FsSimulation::CurrentTime(void) const
{
	return currentTime;
}

void FsSimulation::RegisterExtension(std::shared_ptr <FsSimExtensionBase> addOnPtr)
{
	addOnList.push_back(addOnPtr);
}

std::shared_ptr <class FsSimExtensionBase> FsSimulation::FindExtension(const YsString &str) const
{
	for(auto ptr : addOnList)
	{
		if(0==str.Strcmp(ptr->GetIdent()))
		{
			return ptr;
		}
	}
	return nullptr;
}

YSRESULT FsSimulation::TestAircraftCarrierDataIntegrity(void) const
{
	YSRESULT res=YSOK;

	for(YSSIZE_T i=0; i<aircraftCarrierList.GetN(); ++i)
	{
		const FsAircraftCarrierProperty *const carrier=aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty();
		{
			const FsAirplane *air=NULL;
			while(NULL!=(air=FindNextAirplane(air)))
			{
				if(YSTRUE==carrier->IsAirplaneLoaded(air))
				{
					if(YSTRUE!=air->Prop().IsOnCarrier())
					{
						YsPrintf("Aircraft %d[%s] is linked from a carrier, but aircraft is not set as on carrier.\n",
						    air->SearchKey(),
						    air->Prop().GetIdentifier());
						res=YSERR;
					}
					if(air->Prop().OnThisCarrier()!=aircraftCarrierList[i])
					{
						YsPrintf("Aircraft %d[%s] is not linked to the carrier on which it is supposed to be.\n",
						    air->SearchKey(),
						    air->Prop().GetIdentifier());
						res=YSERR;
					}
				}
			}
		}

		{
			const FsGround *gnd=NULL;
			while(NULL!=(gnd=FindNextGround(gnd)))
			{
				if(YSTRUE==carrier->IsGroundLoaded(gnd))
				{
					if(YSTRUE!=gnd->Prop().IsOnCarrier())
					{
						YsPrintf("Ground %d[%s] is linked from a carrier, but ground is not set as on carrier.\n",
						    gnd->SearchKey(),
						    gnd->Prop().GetIdentifier());
						res=YSERR;
					}
					if(gnd->Prop().OnThisCarrier()!=aircraftCarrierList[i])
					{
						YsPrintf("Ground %d[%s] is not linked to the carrier on which it is supposed to be.\n",
						    gnd->SearchKey(),
						    gnd->Prop().GetIdentifier());
						res=YSERR;
					}
				}
			}
		}
	}

	{
		const FsAirplane *air=NULL;
		while(NULL!=(air=FindNextAirplane(air)))
		{
			if(YSTRUE==air->Prop().IsOnCarrier() && NULL==air->Prop().OnThisCarrier())
			{
				YsPrintf("Aircraft %d[%s] is set as on carrier, but OnThisCarrier is NULL.\n",
				    air->SearchKey(),
				    air->Prop().GetIdentifier());
				res=YSERR;
			}
			if(YSTRUE!=air->Prop().IsOnCarrier() && NULL!=air->Prop().OnThisCarrier())
			{
				YsPrintf("Aircraft %d[%s] is not set as on carrier, but OnThisCarrier is not NULL.\n",
				    air->SearchKey(),
				    air->Prop().GetIdentifier());
				res=YSERR;
			}
			if(YSTRUE==air->Prop().IsOnCarrier() && NULL!=air->Prop().OnThisCarrier())
			{
				const FsAircraftCarrierProperty *carrierProp=air->Prop().OnThisCarrier()->Prop().GetAircraftCarrierProperty();
				if(NULL==carrierProp)
				{
					YsPrintf("Aircraft %d[%s] is set as on carrier, but the ground object does not have carrier property.\n",
					    air->SearchKey(),
					    air->Prop().GetIdentifier());
					res=YSERR;
				}
				else if(YSTRUE!=carrierProp->IsAirplaneLoaded(air))
				{
					YsPrintf("Aircraft %d[%s] is set as on carrier, but the not linked back from the carrier.\n",
					    air->SearchKey(),
					    air->Prop().GetIdentifier());
					res=YSERR;
				}
			}
		}
	}

	{
		const FsGround *gnd=NULL;
		while(NULL!=(gnd=FindNextGround(gnd)))
		{
			if(YSTRUE==gnd->Prop().IsOnCarrier() && NULL==gnd->Prop().OnThisCarrier())
			{
				YsPrintf("Ground %d[%s] is set as on carrier, but OnThisCarrier is NULL.\n",
				    gnd->SearchKey(),
				    gnd->Prop().GetIdentifier());
				res=YSERR;
			}
			if(YSTRUE!=gnd->Prop().IsOnCarrier() && NULL!=gnd->Prop().OnThisCarrier())
			{
				YsPrintf("Ground %d[%s] is not set as on carrier, but OnThisCarrier is not NULL.\n",
				    gnd->SearchKey(),
				    gnd->Prop().GetIdentifier());
				res=YSERR;
			}
			if(YSTRUE==gnd->Prop().IsOnCarrier() && NULL!=gnd->Prop().OnThisCarrier())
			{
				const FsAircraftCarrierProperty *carrierProp=gnd->Prop().OnThisCarrier()->Prop().GetAircraftCarrierProperty();
				if(NULL==carrierProp)
				{
					YsPrintf("Ground %d[%s] is set as on carrier, but the ground object does not have carrier property.\n",
					    gnd->SearchKey(),
					    gnd->Prop().GetIdentifier());
					res=YSERR;
				}
				else if(YSTRUE!=carrierProp->IsGroundLoaded(gnd))
				{
					YsPrintf("Ground %d[%s] is set as on carrier, but the not linked back from the carrier.\n",
					    gnd->SearchKey(),
					    gnd->Prop().GetIdentifier());
					res=YSERR;
				}
			}
		}
	}


	return res;
}

// File-related functions are moved out to fssimulationfileio.cpp

YSRESULT FsSimulation::PrepareSimulationEvent(void)
{
	simEvent->MatchAirGndYfsId(this);
	simEvent->SortEventByTime();
	return YSOK;
}

YSBOOL FsSimulation::IsMissionGoalSet(void) const
{
	if(NULL!=goal && 0!=goal->goalFlag)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsSimulation::SetMissionGoal(const class FsMissionGoal &missionGoal)
{
	*goal=missionGoal;
	return YSOK;
}

const class FsMissionGoal &FsSimulation::GetMissionGoal(void) const
{
	return *goal;
}

void FsSimulation::RecordPlayerChange(const FsExistence *newPlayer)
{
	if(NULL!=newPlayer)
	{
		FsSimulationEvent evt;
		evt.Initialize();
		evt.eventTime=currentTime;
		evt.eventType=FSEVENT_PLAYEROBJCHANGE;
		evt.objKey=newPlayer->SearchKey();
		simEvent->AddEvent(currentTime,evt);
	}
}

YSRESULT FsSimulation::SetPlayerAirplane(const FsAirplane *air,YSBOOL record)
{
	if(NULL!=air && YSTRUE==record)
	{
		RecordPlayerChange(air);
	}
	localUser->BoardAircraft((FsAirplane *)air);
	return YSOK;
}

YSRESULT FsSimulation::GetPlayerVehicleHistory(YsArray <PlayerVehicleHistoryInfo> &playerHist) const
{
	playerHist.Clear();
	for(YSSIZE_T evtIdx=0; evtIdx<simEvent->eventList.GetN(); ++evtIdx)
	{
		if(FSEVENT_PLAYEROBJCHANGE==simEvent->eventList[evtIdx].eventType)
		{
			const FsExistence *obj=FindObject(simEvent->eventList[evtIdx].objKey);
			if(NULL!=obj)
			{
				if(0<playerHist.GetN())
				{
					playerHist.GetEnd().tEnd=simEvent->eventList[evtIdx].eventTime;
				}

				playerHist.Increment();
				playerHist.GetEnd().vehicle=obj;
				playerHist.GetEnd().tStart=simEvent->eventList[evtIdx].eventTime;

				if(FSEX_AIRPLANE==obj->GetType())
				{
					const FsAirplane *air=(const FsAirplane *)obj;
					playerHist.GetEnd().tEnd=air->rec->GetRecordEndTime();
				}
				else if(FSEX_GROUND==obj->GetType())
				{
					const FsGround *gnd=(const FsGround *)obj;
					playerHist.GetEnd().tEnd=gnd->rec->GetRecordEndTime();
				}
				else
				{
					playerHist.GetEnd().tEnd=simEvent->eventList[evtIdx].eventTime;
				}
			}
		}
	}

	if(0==playerHist.GetN())
	{
		// The data is probably from an old flight record, which does not have event data structure.
		const FsExistence *obj=GetPlayerObject();
		if(NULL!=obj)
		{
			playerHist.Increment();
			playerHist.GetEnd().vehicle=obj;

			if(FSEX_AIRPLANE==obj->GetType())
			{
				const FsAirplane *air=(const FsAirplane *)obj;
				playerHist.GetEnd().tStart=air->rec->GetRecordBeginTime();
				playerHist.GetEnd().tEnd=air->rec->GetRecordEndTime();
			}
			else if(FSEX_GROUND==obj->GetType())
			{
				const FsGround *gnd=(const FsGround *)obj;
				playerHist.GetEnd().tStart=gnd->rec->GetRecordBeginTime();
				playerHist.GetEnd().tEnd=gnd->rec->GetRecordEndTime();
			}
			else
			{
				playerHist.GetEnd().tStart=0.0;
				playerHist.GetEnd().tEnd=0.0;
			}
		}
	}

	return YSOK;
}

YSRESULT FsSimulation::SetPlayerGround(const FsGround *gnd,YSBOOL record)
{

	if(NULL!=gnd && YSTRUE==record)
	{
		RecordPlayerChange(gnd);
	}
	localUser->BoardGround((FsGround *)gnd);
	return YSOK;
}

YSRESULT FsSimulation::SetPlayerObject(const FsExistence *obj,YSBOOL record)
{
	if(NULL==obj)
	{
		return SetPlayerAirplane(NULL,record);
	}
	else
	{
		switch(obj->GetType())
		{
		case FSEX_AIRPLANE:
			return SetPlayerAirplane((FsAirplane *)obj,record);
		case FSEX_GROUND:
			return SetPlayerGround((FsGround *)obj,record);
		default:
			break;
		}
	}
	return YSERR;
}

void FsSimulation::SetFirstPlayerYfsIdent(FSEXISTENCETYPE objType,int yfsIdent)
{
	firstPlayer.SetYfsIdent(objType,yfsIdent);
}

void FsSimulation::ResetToFirstPlayerObject(void)
{
	auto obj=firstPlayer.GetObject(this);
	if(NULL!=obj)
	{
		SetPlayerObject(obj);
	}
}

FsAirplane *FsSimulation::AddAirplane(FsAirplane &airplane,YSBOOL isPlayerPlane,const wchar_t tmplRootDir[],unsigned netSearchKey)
{
	YsListItem <FsAirplane> *neo;
	neo=airplaneList.Create();
	if(neo!=NULL)
	{
		neo->dat.CleanUp();
		neo->dat.Initialize();

		neo->dat=airplane;
		neo->dat.thisInTheList=neo;

		if(airplane.Prop().TailStrikeAngleHasBeenComputed()!=YSTRUE)  // For Safety 2004/07/23
		{
			airplane.Prop().AutoComputeTailStrikePitchAngle(airplane.TransformedCollisionShell().Conv());
		}

		neo->dat.SetProperty(airplane.Prop(),tmplRootDir);
		neo->dat.ResetDnmState();

		airplaneList.Encache();
		if(isPlayerPlane==YSTRUE)
		{
			SetPlayerAirplane(&neo->dat);
		}

		if(netSearchKey!=0)
		{
			neo->dat.SetSearchKeyForNetworkSynchronizationPurpose(netSearchKey);
		}
		airplaneSearch->AddElement(FsExistence::GetSearchKey(&neo->dat),&neo->dat);

		ltc.Add(&neo->dat);

		return &neo->dat;
	}
	return NULL;
}

YSRESULT FsSimulation::DeleteAirplane(FsAirplane *air)
{
	if(air->thisInTheList->GetContainer()==&airplaneList)
	{
		airplaneSearch->DeleteElement(FsExistence::GetSearchKey(air),air);  // (*)



		air->CleanUp();  // coll is cleaned up in here. (*)



		if(air==GetPlayerAirplane())  // 2004/09/04
		{
			SetPlayerAirplane(NULL);
		}



		if(air->Prop().IsOnCarrier()==YSTRUE)
		{
			FsGround *const onThisCarrier=air->Prop().OnThisCarrier();
			if(onThisCarrier!=NULL)
			{
				FsAircraftCarrierProperty *const carrier=onThisCarrier->Prop().GetAircraftCarrierProperty();
				if(carrier!=NULL)
				{
					carrier->UnloadAirplane(air);
				}
			}
		}



		FsGround *gnd;
		gnd=NULL;
		while((gnd=FindNextGround(gnd))!=NULL)
		{
			if(gnd->Prop().GetAirTarget()==air)
			{
				gnd->Prop().SetAirTarget(NULL);
			}
		}
		FsAirplane *ptr;
		ptr=NULL;
		while((ptr=FindNextAirplane(ptr))!=NULL)
		{
			if(ptr->Prop().GetAirTargetKey()==FsExistence::GetSearchKey(air))
			{
				ptr->Prop().SetAirTargetKey(YSNULLHASHKEY);
			}
		}



		ltc.Delete(air);



		bulletHolder.ObjectIsDeleted(air);



		if(focusAir==air)
		{
			focusAir=NULL;
		}
		if(focusAir2==air)
		{
			focusAir2=NULL;
		}



		airplaneList.Delete(air->thisInTheList);  // (*)
		return YSOK;
	}
	return YSERR;
}

FsGround *FsSimulation::AddGround(FsGround &ground,YSBOOL isCarrier,const wchar_t tmplRootDir[],const wchar_t acpFn[],unsigned netSearchKey)
{
	YsListItem <FsGround> *neo;
	neo=groundList.Create();
	if(neo!=NULL)
	{
		neo->dat.CleanUp();
		neo->dat.Initialize();

		neo->dat=ground;
		neo->dat.thisInTheList=neo;

		groundList.Encache();


		neo->dat.SetProperty(ground.Prop()); // 2009/05/02  Why was the program working without it??


		ltc.Add(&neo->dat);

		if(netSearchKey!=0)
		{
			neo->dat.SetSearchKeyForNetworkSynchronizationPurpose(netSearchKey);
		}
		groundSearch->AddElement(FsExistence::GetSearchKey(&neo->dat),&neo->dat);

		// 2007/01/15 The following block has been moved from FsWorld::AddGround
		if(isCarrier==YSTRUE && acpFn!=NULL)
		{
			YsWString ful1;
			ful1.MakeFullPathName(tmplRootDir,acpFn);
			if(neo->dat.Prop().LoadAircraftCarrierProperty(tmplRootDir,ful1.data())!=YSOK)
			{
				// If it fails, it may be from the program-root dir.
				YsWString ful2;
				ful2.MakeFullPathName(L".",acpFn);
				if(neo->dat.Prop().LoadAircraftCarrierProperty(L".",ful2.data())!=YSOK)
				{
					YsString ful1_utf8,ful2_utf8;
					ful1_utf8.EncodeUTF8(ful1.data());
					ful2_utf8.EncodeUTF8(ful2.data());
					fsStderr.Printf("Failed to load Aircraft Carrier.\n"
					                "  Tried [%s]\n"
					                "  Tried [%s]\n",ful1_utf8.c_str(),ful2_utf8.c_str());
				}
			}
		}



		if(neo->dat.Prop().GetAircraftCarrierProperty()!=NULL)  // 2006/09/21
		{                                                       // 2006/09/21
			aircraftCarrierList.Append(&neo->dat);              // 2006/09/21
		}                                                       // 2006/09/21
		if(neo->dat.Prop().GetVorRange()>YsTolerance)           // 2006/09/21
		{                                                       // 2006/09/21
			vorList.Append(&neo->dat);                          // 2006/09/21
		}                                                       // 2006/09/21
		if(neo->dat.Prop().GetNdbRange()>YsTolerance)           // 2007/08/05
		{                                                       // 2007/08/05
			ndbList.Append(&neo->dat);                          // 2007/08/05
		}                                                       // 2007/08/05



		if(neo->dat.Prop().chFlags&(FsGroundProperty::YSGP_SUPPLYFUEL|FsGroundProperty::YSGP_SUPPLYAMMO))
		{
			supplyList.Append(&neo->dat);
		}

		neo->dat.ResetDnmState();


		return &neo->dat;
	}

	return NULL;
}

YSRESULT FsSimulation::DeleteGround(FsGround *gnd)
{
	if(gnd->thisInTheList->GetContainer()==&groundList)
	{
		groundSearch->DeleteElement(FsExistence::GetSearchKey(gnd),gnd);

		if(gnd->Prop().IsOnCarrier()==YSTRUE)
		{
			FsGround *const onThisCarrier=gnd->Prop().OnThisCarrier();
			if(onThisCarrier!=NULL)
			{
				FsAircraftCarrierProperty *const carrier=onThisCarrier->Prop().GetAircraftCarrierProperty();
				if(carrier!=NULL)
				{
					carrier->UnloadGround(gnd);
				}
			}
		}

		
		for(FsGround *gndPtr=NULL; (gndPtr=FindNextGround(gndPtr))!=NULL; )
		{
			if(gndPtr->Prop().GetGroundTarget()==gnd)
			{
				gndPtr->Prop().SetGroundTarget(NULL);
			}

			if(YSTRUE==gndPtr->Prop().IsOnCarrier() && gnd==gndPtr->Prop().OnThisCarrier())
			{
				FsAircraftCarrierProperty *const carrier=gnd->Prop().GetAircraftCarrierProperty();
				if(NULL!=carrier)
				{
					carrier->UnloadGround(gndPtr);
				}
				else
				{
					fsConsole.Printf("FsSimulation::DeleteGround(FsGround *gnd)");
					fsConsole.Printf("  Reporting Internal Broken Link (Carrier to GND).\n");
				}
			}
		}
		for(FsAirplane *airPtr=NULL; (airPtr=FindNextAirplane(airPtr))!=NULL; )
		{
			if(airPtr->Prop().IsOnCarrier()==YSTRUE && airPtr->Prop().OnThisCarrier()==gnd)
			{
				FsAircraftCarrierProperty *const carrier=gnd->Prop().GetAircraftCarrierProperty();
				if(carrier!=NULL)
				{
					carrier->UnloadAirplane(airPtr);
				}
				else
				{
					fsConsole.Printf("FsSimulation::DeleteGround(FsGround *gnd)");
					fsConsole.Printf("  Reporting Internal Broken Link (Carrier to AIR).\n");
					airPtr->Prop().AfterUnloadedFromCarrier();
				}
			}

			if(airPtr->Prop().GetGroundTargetKey()==FsExistence::GetSearchKey(gnd))
			{
				airPtr->Prop().SetAirTargetKey(YSNULLHASHKEY);
			}
		}

		YSSIZE_T i;
		forYsArrayRev(i,aircraftCarrierList)
		{
			if(aircraftCarrierList[i]==gnd)
			{
				aircraftCarrierList.DeleteBySwapping(i);
			}
		}

		forYsArrayRev(i,vorList)              // 2006/09/21
		{                                     // 2006/09/21
			if(vorList[i]==gnd)               // 2006/09/21
			{                                 // 2006/09/21
				vorList.DeleteBySwapping(i);  // 2006/09/21
			}                                 // 2006/09/21
		}                                     // 2006/09/21

		forYsArrayRev(i,ndbList)              // 2007/08/05
		{                                     // 2007/08/05
			if(ndbList[i]==gnd)               // 2007/08/05
			{                                 // 2007/08/05
				ndbList.DeleteBySwapping(i);  // 2007/08/05
			}                                 // 2007/08/05
		}                                     // 2007/08/05

		forYsArrayRev(i,supplyList)
		{
			if(supplyList[i]==gnd)
			{
				supplyList.DeleteBySwapping(i);
			}
		}

		ltc.Delete(gnd);

		gnd->CleanUp();  // coll is cleaned up in here.
		groundList.Delete(gnd->thisInTheList);
		return YSOK;
	}
	return YSERR;
}

FsField *FsSimulation::SetField(FsField &fld,const YsVec3 &pos,const YsAtt3 &att)
{
	YsColor gnd,sky;
	if(YSOK==fld.GetGroundSkyColor(gnd,sky))
	{
		this->gndColor=gnd;
		this->skyColor=sky;
	}
	else
	{
		this->gndColor.SetIntRGB(0,0,160);
		this->skyColor.SetIntRGB(0,128,192);
	}
	this->gndSpecular=fld.GetFieldPtr()->GetSpecular();



	field.CleanUp();
	field.Initialize();

	field=fld;

	field.SetPosition(pos);
	field.SetAttitude(att);

	if(field.GetFieldPtr()!=NULL)
	{
		YsVec3 cen,rect[4];

		YsArray <const YsSceneryRectRegion *,16> rgn;
		if(field.SearchFieldRegionById(rgn,10)==YSOK && rgn.GetN()>0)  // ID=10 Tower View Position
		{
			int i;
			for(i=0; i<rgn.GetN(); i++)
			{
				field.GetFieldRegionRect(rect,rgn[i]);
				cen=(rect[0]+rect[1]+rect[2]+rect[3])/4.0;
				towerPosition.Append(cen);
			}
		}
	}

	fieldLoaded=YSTRUE;

	RemakeLattice();

	return &field;
}

void FsSimulation::SetFogColor(YsColor col)
{
	fogColor=col;
}
void FsSimulation::SetSkyColor(YsColor col)
{
	skyColor=col;
}
void FsSimulation::SetGroundColor(YsColor col)
{
	gndColor=col;
}

YSRESULT FsSimulation::CheckStartPositionIsAvailable(int /*fieldId*/,const char stpIdName[])
{
	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(strcmp(air->_startPosition,stpIdName)==0)
		{
			return YSERR;
		}
	}

	FsGround *gnd;
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		if(strcmp(gnd->_startPosition,stpIdName)==0)
		{
			return YSERR;
		}
	}
	return YSOK;
}

YSBOOL FsSimulation::SimTestCollision(const class FsVisualSrf &shl,const YsVec3 &pos,const YsAtt3 &att)
// Used from fsnetwork.cpp to check if the new object can be generated without collision.
{
	FsVisualSrf coll1;
	YsMatrix4x4 mat;

	coll1=shl;
	mat.Translate(pos);
	mat.Rotate(att);
	coll1.SetMatrix(mat);

	YsVec3 bbx[2];
	coll1.GetBoundingBox(bbx[0],bbx[1]);

	YsArray <FsAirplane *,256> airCandidate;
	GetLattice().GetAirCollisionCandidate(airCandidate,bbx[0],bbx[1]);
	for(int i=0; i<airCandidate.GetN(); i++)
	{
		FsAirplane *air2=airCandidate[i];
		YsVec3 collPos;
		if(YSTRUE==air2->IsAlive() && YSTRUE==CheckMidAir(collPos,coll1.Conv(),*air2))
		{
			return YSTRUE;
		}
	}

	YsArray <FsGround *,256> gndCandidate;
	GetLattice().GetGndCollisionCandidate(gndCandidate,bbx[0],bbx[1]);
	for(int i=0; i<gndCandidate.GetN(); i++)
	{
		FsGround *gnd2=gndCandidate[i];
		YsVec3 collPos;
		if(YSTRUE==gnd2->IsAlive() && YSTRUE==CheckMidAir(collPos,coll1.Conv(),*gnd2))
		{
			return YSTRUE;
		}
	}

	return YSFALSE;
}

void FsSimulation::SetEnvironment(FSENVIRONMENT env)
{
	this->env=env;
	EnforceEnvironment();
}

void FsSimulation::EnforceEnvironment(void)
{
	switch(env)
	{
	case FSDAYLIGHT:
		field.ApplyColorScale(1.0,1.0,0.7);
		fogColor.SetDoubleRGB(0.6,0.6,0.6);
		YsScenery::lightPointSizePix=1;
		break;
	case FSNIGHT:
		field.ApplyColorScale(0.1,0.1,1.0);
		fogColor.SetDoubleRGB(0.1,0.1,0.1);
		YsScenery::lightPointSizePix=4;
		break;
	}
}

FSENVIRONMENT FsSimulation::GetEnvironment(void) const
{
	return env;
}

YSRESULT FsSimulation::SendConfigString(const char str[])
{
	return cfgPtr->SendCommand(str);
}

int FsSimulation::GetNumAircraftCarrier(void) const
{
	return (int)aircraftCarrierList.GetN();
}

const FsGround *FsSimulation::GetAircraftCarrier(YSSIZE_T i) const
{
	return aircraftCarrierList[i];
}

void FsSimulation::PrepareReplaySimulation(void)
{
	FsAirplane *airPtr;

	ResetToFirstPlayerObject();

	airPtr=NULL;
	while((airPtr=FindNextAirplane(airPtr))!=NULL)
	{
		if(airPtr->rec!=NULL)
		{
			airPtr->isPlayingRecord=YSTRUE;
			airPtr->PlayRecord(0.0,0.0);
			airPtr->Prop().ReleaseVirtualButton();  // 2004/09/12
		}
		else
		{
			airPtr->isPlayingRecord=YSFALSE;
		}
	}

	FsGround *gndPtr;
	gndPtr=NULL;
	while((gndPtr=FindNextGround(gndPtr))!=NULL)
	{
		if(gndPtr->rec!=NULL)
		{
			gndPtr->isPlayingRecord=YSTRUE;
			gndPtr->PlayRecord(0.0,0.0);
		}
		else
		{
			gndPtr->isPlayingRecord=YSFALSE;
		}
	}

	ClearUserInterface();    // 2009/03/29
	ClearTimedMessage();

	// Prepare Bullet Holder
	// Prepare Explosion Holder
}

void FsSimulation::DeleteEventByTypeAll(int eventType)
{
	simEvent->DeleteEventByTypeAll(eventType);
}

void FsSimulation::ClearFirstPlayer(void)
{
	firstPlayer.Initialize();
}

void FsSimulation::CenterJoystick(void)
{
	userInput.CenterJoystick(ctlAssign);
}

YSRESULT FsSimulation::MakeCenterJoystickDialog(class FsCenterJoystick &centerJoystickDialog,int nextActionCode)
{
	centerJoystickDialog.Initialize(&userInput,&ctlAssign,nextActionCode);
	return YSOK;
}

YSRESULT FsSimulation::TestAssignJoystick(void)
{
	FsControlAssignment ctlAssign;
	ctlAssign.Load(FsGetControlAssignFile());
	FsFlightControl userInput;
	return userInput.VerifyAndFixJoystickAxisAssignment(ctlAssign);
}

YSBOOL FsSimulation::NeedNoJoystickWarningDialog(void)
{
	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());
	return cfg.showJoystickUnpluggedWarning;
}

YSRESULT FsSimulation::CheckJoystickAssignmentAndFixIfNecessary(void)
{
	ctlAssign.Load(FsGetControlAssignFile());
	if(YSOK!=userInput.VerifyAndFixJoystickAxisAssignment(ctlAssign))
	{
		return YSERR;
	}
	return YSOK;
}

void FsSimulation::ClearKeyBuffer(void)
{
	FsPollDevice();
	while(FsInkey()!=FSKEY_NULL || FsInkeyChar()!=0)
	{
		FsPollDevice();
	}
}

void FsSimulation::SetReplayMode(FSREPLAYMODE replMode)
{
	replayMode=replMode;
}

void FsSimulation::FastForward(const double &targetTime)
{
	bulletHolder.Clear();
	explosionHolder.Clear();

	while(currentTime<targetTime-YsTolerance)
	{
		double dt;

		dt=YsSmaller(targetTime-currentTime,0.25);

		bulletHolder.PlayRecord(currentTime,dt);
		explosionHolder.PlayRecord(currentTime,dt);

		currentTime+=dt;
	}

	FsAirplane *seeker;
	seeker=NULL;
	while((seeker=FindNextAirplane(seeker))!=NULL)
	{
		seeker->refTime1=targetTime;
		seeker->refTime2=targetTime;
	}
}

void FsSimulation::DeleteFlightRecord(const double &t1,const double &t2)
{
	FsAirplane *air;
	FsGround *gnd;

	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		air->rec->DeleteRecord(t1,t2);
	}
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		gnd->rec->DeleteRecord(t1,t2);
	}
	bulletHolder.toPlay->DeleteRecord(t1,t2);
	explosionHolder.toPlay->DeleteRecord(t1,t2);
}

void FsSimulation::RunSimulationOneStep(FsSimulation::FSSIMULATIONSTATE &simState)
{
	switch(simState)
	{
	case FSSIMSTATE_CENTERJOYSTICK:
		CenterJoystickOneStep(simState);
		//CenterJoystickDraw();
		break;
	case FSSIMSTATE_INITIALIZE:
		ClearKeyBuffer();
		PrepareRunSimulation();
		DecideAllViewPoint(0.0); // Prevent one frame with uninitialized viewpoint.
		simState=FSSIMSTATE_RUNNING;

		for(auto ptr : addOnList)
		{
			ptr->StartSimulation(this);
		}
		break;
	case FSSIMSTATE_RUNNING:
		{
			double passedTime=PassedTime();

			for(auto idx : addOnList.ReverseIndex())
			{
				auto addOnPtr=addOnList[idx];
				auto customDt=addOnPtr->UseCustomTimeStep(this);
				if(YsTolerance<customDt)
				{
					passedTime=customDt;
					break;
				}
			}

			auto userControl=FSUSC_ENABLE;
			auto recordFlight=YSTRUE;
			for(auto idx : addOnList.ReverseIndex())
			{
				auto ouc=addOnList[idx]->OverrideUserControl(this);
				if(FSUSC_DONTCARE!=ouc)
				{
					userControl=ouc;
					break;
				}
			}
			for(auto idx : addOnList.ReverseIndex())
			{
				auto rec=addOnList[idx]->OverrideRecording(this);
				if(YSTFUNKNOWN!=rec)
				{
					recordFlight=rec;
					break;
				}
			}
			SimulateOneStep(passedTime,YSFALSE,recordFlight,YSFALSE,YSFALSE,userControl,YSFALSE);

			for(auto addOnPtr : addOnList)
			{
				if(YSTRUE==addOnPtr->MustKeepRunning(this))
				{
					endTime=0.0;
				}
				if(YSTRUE==addOnPtr->MustTerminate(this))
				{
					SetTerminate(YSTRUE);
				}
			}

			if(endTime>YsTolerance && currentTime>endTime)
			{
				SetTerminate(YSTRUE);
			}
			if(YSTRUE==terminate)
			{
				simState=FSSIMSTATE_CHECKCONTINUE;
			}
		}
		break;
	case FSSIMSTATE_CHECKCONTINUE:
		switch(CheckContinueOneStep())
		{
		case YSTFUNKNOWN:
			break;
		case YSTRUE:
			PrepareContinueSimulation();
			simState=FSSIMSTATE_RUNNING;
			break;
		case YSFALSE:
			simState=FSSIMSTATE_TERMINATING;
			break;
		}
		for(auto ptr : addOnList)
		{
			if(YSTRUE!=ptr->CanContinue(this))
			{
				simState=FSSIMSTATE_TERMINATING;
			}
		}
		break;
	case FSSIMSTATE_TERMINATING:
		SimRecordAir(currentTime,YSTRUE);
		SimRecordGnd(currentTime,YSTRUE);
		AfterSimulation();
		for(auto ptr : addOnList)
		{
			ptr->EndSimulation(this);
		}
		simState=FSSIMSTATE_OVER;
		break;
	case FSSIMSTATE_OVER:
		break;
	}
}

YSBOOL FsSimulation::CheckInterceptMissionAvailable(void) const
{
	FsGround *gndSeeker;
	gndSeeker=NULL;
	while((gndSeeker=FindNextGround(gndSeeker))!=NULL)
	{
		if(gndSeeker->IsAlive()==YSTRUE &&
		   gndSeeker->primaryTarget==YSTRUE &&
		   gndSeeker->iff==FS_IFF0)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsSimulation::CheckCloseAirSupportMissionAvailable(void) const
{
	if(CheckInterceptMissionAvailable()==YSTRUE)
	{
		YsArray <const YsSceneryRectRegion *,16> rgn;
		if(field.SearchFieldRegionById(rgn,6)==YSOK && rgn.GetN()>0)  // ID=6 Generator
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

void FsSimulation::TestSolidCloud(void)
{
	solidCloud->Test();
}

void FsSimulation::MakeSolidCloud
   (int n,const YsVec3 &cen,const double &range,const double &sizeH,const double &y0,const double &y1)
{
	solidCloud->Make(n,cen,range,sizeH,y0,y1);
}

void FsSimulation::SetFog(YSBOOL drawFog,const double &visibility)
{
	cfgPtr->drawFog=YSTRUE;
	cfgPtr->fogVisibility=visibility;
	weather->SetFog(drawFog);
	weather->SetFogVisibility(visibility);
}

void FsSimulation::ReplayInfo::Initialize(const double beginTimeIn,YSBOOL editModeIn)
{
	beginTime=beginTimeIn;
	editMode=editModeIn;
	resumed=YSFALSE;

	originalPlayerObject=NULL;
}

void FsSimulation::RunReplaySimulationOneStep(FSSIMULATIONSTATE &simState,FsSimulation::ReplayInfo &replayInfo)
{
	switch(simState)
	{
	case FSSIMSTATE_CENTERJOYSTICK:
		CenterJoystickOneStep(simState);
		//CenterJoystickDraw();
		break;
	case FSSIMSTATE_INITIALIZE:
		replayInfo.originalPlayerObject=GetPlayerObject();

		ClearKeyBuffer();
		PrepareRunSimulation();

		currentTime=YsGreater(replayInfo.beginTime-30.0,0.0);
		FastForward(replayInfo.beginTime);

		if(replayInfo.editMode==YSTRUE)
		{
			AddEditDialog();
		}
		else
		{
			AddReplayDialog();
		}
		showReplayDlg=YSTRUE;
		replayMode=FSREPLAY_PLAY;

		RefreshOrdinance();

		for(auto ptr : addOnList)
		{
			ptr->StartReplay(this);
		}

		simState=FSSIMSTATE_RUNNING;
		break;
	case FSSIMSTATE_RUNNING:
		{
			double passedTime,actualPassedTime;
			FSREPLAYMODE prevMode;

			// MEMO: replayMode is updated in SimControlByUser();
			//       replayMode is only refered in this function.

			if(YSTRUE!=replayInfo.resumed)
			{
				YSBOOL startResume=YSFALSE;

				passedTime=PassedTime();
				actualPassedTime=passedTime;  // To control view point during pause.

				const double prevCurrentTime=currentTime;

				switch(replayMode)
				{
				case FSREPLAY_VERYFASTREWIND:
					strcpy(systemMessage,"VERY FAST REWIND");
					currentTime-=passedTime*8.0;
					SimMove(0.0);
					passedTime=0.0;
					break;
				case FSREPLAY_FASTREWIND:
					strcpy(systemMessage,"FAST REWIND");
					currentTime-=passedTime*4.0;
					SimMove(0.0);
					passedTime=0.0;
					break;
				case FSREPLAY_BACKWARD:
					strcpy(systemMessage,"BACKWARD");
					currentTime-=passedTime;
					SimMove(0.0);
					passedTime=0.0;
					break;
				case FSREPLAY_PLAY:
					strcpy(systemMessage,"PLAY");
					break;
				case FSREPLAY_FASTFORWARD:
					passedTime*=4.0;
					strcpy(systemMessage,"FAST FORWARD");
					break;
				case FSREPLAY_VERYFASTFORWARD:
					currentTime+=passedTime*8.0;
					passedTime=0.01;
					strcpy(systemMessage,"VERY FAST FORWARD");
					break;
				case FSREPLAY_STEPFORWARD:
					passedTime=0.05;
					replayMode=FSREPLAY_PAUSE;
					break;
				case FSREPLAY_STEPBACK:
					currentTime=YsGreater(currentTime-0.05,0.0);
					SimMove(0.0);
					replayMode=FSREPLAY_PAUSE;
					break;
				case FSREPLAY_PAUSE:
					passedTime=0.0;
					strcpy(systemMessage,"PAUSE [ESC KEY TO END REPLAY]");
					SimMove(0.0);
					break;
				}

		#ifndef YS_SCSV
				if(replayDlg!=NULL && replayDlg->resume==YSTRUE && GetPlayerAirplane()!=NULL)
				{
					if(YSTRUE!=field.CanResume())
					{
						strcpy(systemMessage,"Cannot resume in this scenery");
						replayDlg->resume=YSFALSE;
					}
					else
					{
						startResume=YSTRUE;
					}
					for(auto ptr : addOnList)
					{
						if(YSTRUE!=ptr->CanResume(this))
						{
							strcpy(systemMessage,"Cannot resume");
							startResume=YSFALSE;
							replayDlg->resume=YSFALSE;
						}
					}
				}
		#endif

				if(currentTime<0.0 && 
				  (replayMode==FSREPLAY_FASTREWIND || replayMode==FSREPLAY_VERYFASTREWIND || replayMode==FSREPLAY_BACKWARD || replayMode==FSREPLAY_STEPBACK))
				{
					currentTime=0.0;
					replayMode=FSREPLAY_PAUSE;
				}

				if(replayMode==FSREPLAY_FASTREWIND ||
				   replayMode==FSREPLAY_VERYFASTREWIND ||
				   replayMode==FSREPLAY_VERYFASTFORWARD)
				{
					bulletHolder.Clear();
					explosionHolder.Clear();
					particleStore.Clear();

					FsAirplane *air=NULL;
					while(NULL!=(air=FindNextAirplane(air)))
					{
						if(air->IsAlive()==YSTRUE)
						{
							air->refTime1=YsSmaller(air->refTime1,currentTime);
							air->refTime2=YsSmaller(air->refTime2,currentTime);
						}
					}

					FsGround *gnd=NULL;
					while(NULL!=(gnd=FindNextGround(gnd)))
					{
						if(gnd->IsAlive()==YSTRUE)
						{
							gnd->refTime1=YsSmaller(gnd->refTime1,currentTime);
							gnd->refTime2=YsSmaller(gnd->refTime2,currentTime);
						}
					}
				}

				if(FSREPLAY_VERYFASTREWIND==replayMode ||
				   FSREPLAY_FASTREWIND==replayMode ||
				   FSREPLAY_BACKWARD==replayMode ||
				   FSREPLAY_PLAY==replayMode ||
				   FSREPLAY_STEPBACK==replayMode)
				{
					SimResetPlayerObjectFromRecord(currentTime,prevCurrentTime);
				}

				prevMode=replayMode;
				SimulateOneStep(passedTime,YSFALSE,YSFALSE,YSTRUE,YSFALSE,FSUSC_ENABLE/*YSTRUE*/,YSFALSE);
				SimCheckEndOfFlightRecord();

				if(passedTime<YsTolerance)
				{
					SimControlByUser(actualPassedTime,FSUSC_ENABLE);
					// MEMO:
					//  SimControlByUser is supposed to be called inside SimulateOneStep function,
					//  when the parameter userControl!=FSUSC_DISABLE.  However, when
					//  replayMode==FSREPLAY_PAUSE, the parameter passedTime will be zero, and
					//  SimControlByUser will not move viewing angle, because the rate of
					//  rotation depends on passedTime.  So, to allow the user to control
					//  the viewing angle when the replay is paused, SimControlByUser must
					//  be called here.
				}

				if((prevMode==FSREPLAY_BACKWARD ||
				    prevMode==FSREPLAY_FASTREWIND ||
				    prevMode==FSREPLAY_VERYFASTREWIND ||
				    prevMode==FSREPLAY_VERYFASTFORWARD)&&
				   (replayMode!=FSREPLAY_BACKWARD &&
				    replayMode!=FSREPLAY_FASTREWIND &&
				    replayMode!=FSREPLAY_VERYFASTREWIND &&
				    replayMode!=FSREPLAY_VERYFASTFORWARD))
				{
					double targetTime;
					targetTime=currentTime;

					// 2007/09/22 Disabled weapon flushing
					// currentTime=YsGreater(0.0,currentTime-30.0);
					// FastForward(targetTime);

					if(cfgPtr->drawOrdinance==YSTRUE)
					{
						RefreshOrdinanceByWeaponRecord(currentTime);
					}

					simEvent->SeekNextEventPointereToCurrentTime(currentTime);
				}

				if(endTime>YsTolerance && currentTime>endTime &&
				   (replayMode==FSREPLAY_PLAY || replayMode==FSREPLAY_FASTFORWARD ||
				    replayMode==FSREPLAY_VERYFASTFORWARD))
				{
					currentTime=endTime-YsTolerance;
					replayMode=FSREPLAY_PAUSE;
				}

				if(replayMode==FSREPLAY_PAUSE)
				{
					FsSleep(20);
				}

				if(YSTRUE==terminate || YSTRUE==startResume)
				{
					EraseReplayDialog();
					if(YSTRUE==startResume)
					{
						FsAirplane *playerPlane;
						playerPlane=GetPlayerAirplane();
						if(playerPlane!=NULL && playerPlane->Prop().IsActive()==YSTRUE)
						{
							playerPlane->isPlayingRecord=YSFALSE;
							playerPlane->rec->DeleteRecord(currentTime,YsInfinity);
							bulletHolder.DeleteRecordForResumeFlight(playerPlane,currentTime);
							playerPlane->Prop().ReadBackControl(userInput);

							endTime=0.0;
							SetTerminate(YSFALSE);
							pause=YSTRUE;

							ClearKeyBuffer();
							simEvent->DeleteFutureEventForResume(currentTime);

							replayInfo.resumed=YSTRUE;
						}
						else  // Failed to resume.
						{
							simState=FSSIMSTATE_TERMINATING;
						}
					}
					else
					{
						simState=FSSIMSTATE_TERMINATING;
					}
				}
			}
			else  // Resumed mode
			{
				passedTime=PassedTime();

				SimulateOneStep(passedTime,YSFALSE,YSTRUE,YSFALSE,YSFALSE,FSUSC_ENABLE /*YSTRUE*/,YSFALSE);

				if(endTime>YsTolerance && currentTime>endTime)
				{
					SetTerminate(YSTRUE);
				}

				if(YSTRUE==terminate)
				{
					SimRecordAir(currentTime,YSTRUE);
					SimRecordGnd(currentTime,YSTRUE);
					simState=FSSIMSTATE_TERMINATING;
				}
			}
		}
		break;
	case FSSIMSTATE_TERMINATING:
		AfterSimulation();
		SetPlayerObject(replayInfo.originalPlayerObject,YSFALSE);
		for(auto ptr : addOnList)
		{
			ptr->EndReplay(this);
		}
		simState=FSSIMSTATE_OVER;
		break;
	default:
		simState=FSSIMSTATE_OVER;
		break;
	}
}

void FsSimulation::SimResetPlayerObjectFromRecord(const double &t0,const double &t1)
{
	if(NULL!=simEvent)
	{
		const double T0=YsSmaller(t0,t1);
		const double T1=YsGreater(t0,t1);

		YSBOOL playerChangeTookPlace=YSFALSE;
		for(YSSIZE_T evtIdx=0; evtIdx<simEvent->eventList.GetN(); ++evtIdx)
		{
			if(T0<=simEvent->eventList[evtIdx].eventTime &&
			   simEvent->eventList[evtIdx].eventTime<T1 &&
			   FSEVENT_PLAYEROBJCHANGE==simEvent->eventList[evtIdx].eventType)
			{
				playerChangeTookPlace=YSTRUE;
				break;
			}
		}

		if(YSTRUE==playerChangeTookPlace)
		{
			double playerTime=0.0;
			const FsExistence *player=NULL;

			for(YSSIZE_T evtIdx=0; evtIdx<simEvent->eventList.GetN(); ++evtIdx)
			{
				if(playerTime<=simEvent->eventList[evtIdx].eventTime &&
				   simEvent->eventList[evtIdx].eventTime<T0 &&
				   FSEVENT_PLAYEROBJCHANGE==simEvent->eventList[evtIdx].eventType)
				{
					playerTime=simEvent->eventList[evtIdx].eventTime;
					player=FindObject(simEvent->eventList[evtIdx].objKey);
				}
			}

			if(GetPlayerObject()!=player)
			{
				SetPlayerObject(player,/*record=*/YSFALSE);
			}
		}
	}
}

void FsSimulation::SimSeekNextEventPointereToCurrentTime(const double currentTime)
{
	simEvent->SeekNextEventPointereToCurrentTime(currentTime);
}

void FsSimulation::AddReplayDialog(void)
{
#ifndef YS_SCSV
	if(replayDlg!=NULL)
	{
		FsReplayDialog::Delete(replayDlg);
	}
	replayDlg=FsReplayDialog::Create();
	replayDlg->MakeForReplay(this);
#endif
}

void FsSimulation::AddEditDialog(void)
{
#ifndef YS_SCSV
	if(replayDlg!=NULL)
	{
		FsReplayDialog::Delete(replayDlg);
	}
	replayDlg=FsReplayDialog::Create();
	replayDlg->MakeForEdit(this);
#endif
}

void FsSimulation::EraseReplayDialog(void)
{
#ifndef YS_SCSV
	if(replayDlg!=NULL)
	{
		FsReplayDialog::Delete(replayDlg);
		replayDlg=NULL;
	}
#endif
}

void FsSimulation::RefreshOrdinance(void)
{
	FsAirplane *airSeek;
	airSeek=NULL;
	while((airSeek=FindNextAirplane(airSeek))!=NULL)
	{
		const char *id;
		const FsAirplaneTemplate *tmpl;
		id=airSeek->Prop().GetIdentifier();
		tmpl=world->GetAirplaneTemplate(id);

		YsArray <int,64> loading;
		tmpl->GetProperty()->GetWeaponConfig(loading);
		airSeek->Prop().ApplyWeaponConfig(loading.GetN(),loading);
		airSeek->RecallCommand();
	}
}

void FsSimulation::RefreshOrdinanceByWeaponRecord(const double &currentTime)
{
	RefreshOrdinance();
	bulletHolder.RefreshOrdinanceByWeaponRecord(currentTime);
}

void FsSimulation::PrepareRunSimulation(void)
{
	FsAirplane *playerPlane;
	playerPlane=GetPlayerAirplane();

	endTime=0.0;
	SetTerminate(YSFALSE);
	pause=YSFALSE;
	escKeyCount=0;


	ClearUserInterface();    // 2009/03/29
	ClearTimedMessage();

	fsConsole.SetAutoFlush(YSTRUE);

	fsConsole.Printf("**** Preparing Simulation ****\n");

	fsConsole.Printf("Setting Player Airplane.\n");

	if(playerPlane!=NULL)
	{
		playerPlane->Prop().ReadBackControl(userInput);
		Gear=playerPlane->Prop().GetLandingGear();
		pGear=playerPlane->Prop().GetLandingGear();
		ppGear=playerPlane->Prop().GetLandingGear();
		hideNextGearSound=YSFALSE;
		pFlap=playerPlane->Prop().GetFlap();
		ppFlap=playerPlane->Prop().GetFlap();
		userInput.hasAb=playerPlane->Prop().GetHasAfterburner();
		viewMagUser=playerPlane->Prop().GetDefaultZoom();
	}

	fsConsole.Printf("....\n");

	if((allowedWeaponType&FSWEAPON_ALLOWAAM)==0)
	{
		FsAirplane *seeker;
		seeker=NULL;
		while((seeker=FindNextAirplane(seeker))!=NULL)
		{
			if(seeker->Prop().GetNumWeapon(FSWEAPON_AIM9)>0 ||
			   seeker->Prop().GetNumWeapon(FSWEAPON_AIM120)>0 ||
			   seeker->Prop().GetNumWeapon(FSWEAPON_AIM9X)>0)
			{
				seeker->SendCommand("ULOADAAM");
			}
		}
	}


	if(netServer==NULL && netClient==NULL)
	// 2005/05/30 Don't add cloud in network mode.  I cannot afford transmitting them.
	{
		YsVec3 cloudCenter;
		if(playerPlane!=NULL)
		{
			cloudCenter=playerPlane->GetPosition();
		}
		else
		{
			cloudCenter=YsOrigin();
		}
		if(cfgPtr->cloudType==FSCLOUDFLAT)
		{
			if(cloud->IsReady()!=YSTRUE)
			{
				cloud->Scatter(16,cloudCenter,20000.0,1000.0,cfgPtr->ceiling);
			}
			if(cloud->IsReady()==YSTRUE && cfgPtr->useOpenGlListForCloud==YSTRUE)
			{
				cloud->MakeOpenGlList();
			}
		}
		if(cfgPtr->cloudType==FSCLOUDSOLID)
		{
			if(solidCloud->IsReady()!=YSTRUE)
			{
				solidCloud->Make(12,cloudCenter,30000.0,6000.0,cfgPtr->ceiling-400.0,cfgPtr->ceiling+400.0);
			}
		}
	}

	fsConsole.Printf("Initializing Timer\n");

	PassedTime();
	currentTime=0.0;
	simEvent->Rewind();
	nextAirRecordTime=0.05;
	nextGndRecordTime=1.0;
	nextControlTime=0.025;
	lastControlledTime=0.0;

	nextFpsUpdateTime=0;
	fps=0.0;
	nFrameForFpsCount=0;
	lastFpsUpdateTime=0;

	fsConsole.Printf("Initializing Weapon and Explosion\n");

	bulletHolder.Clear();
	explosionHolder.Clear();
	particleStore.Initialize();



	fsConsole.Printf("Ground Adjustment\n");

	SimMove(0.0);  // Ground Adjust;

	fsConsole.Printf("Preparing Flight Record Buffer\n");

	SimRecordAir(0.0,YSTRUE);
	SimRecordGnd(0.0,YSTRUE);

	fsConsole.Printf("Caching.\n");

	FsAirplane *airSeeker;
	airSeeker=NULL;
	while((airSeeker=FindNextAirplane(airSeeker))!=NULL)
	{
		airSeeker->refTime1=0.0;
		airSeeker->refTime2=0.0;
	}

	airplaneList.Encache();
	groundList.Encache();

	fsConsole.Printf("Garbage Collecting.\n");

	YsShell::CleanUpVertexPolygonStore();

	fsConsole.Printf("Preparation Done.\n");

	ilsToGo=NULL;

	EnforceEnvironment();

	int i;
	for(i=0; i<FsMaxNumJoystick; i++)
	{
		FsPollJoystick(pJoy[i],i);
	}

	showReplayDlg=YSFALSE;
	if(replayDlg!=NULL)
	{
		FsReplayDialog::Delete(replayDlg);
		replayDlg=NULL;
	}

	*iniWeather=*weather;

	airTrafficSequence->MakeSlotArray(this);
	airTrafficSequence->RefreshAirTrafficSlot(this);

}

void FsSimulation::SetTerminate(YSBOOL termi)
{
	terminate=termi;
}

void FsSimulation::PrepareContinueSimulation(void)
{
	if(GetPlayerAirplane()!=NULL)
	{
		FsAirplane *playerPlane=GetPlayerAirplane();
		ResetAirplane(playerPlane);

		FsSimulationEvent evt;

		int i;
		forYsArray(i,playerPlane->cmdLog)
		{
			evt.Initialize();
			evt.eventType=FSEVENT_AIRCOMMAND;
			evt.objKey=FsExistence::GetSearchKey(playerPlane);
			evt.str=playerPlane->cmdLog[i];
			simEvent->AddEvent(currentTime,evt);
		}

		playerPlane->Prop().ReadBackControl(userInput);
	}

	if(GetPlayerGround()!=NULL)
	{
		FsGround *playerGround=GetPlayerGround();
		ResetGround(playerGround);

		FsSimulationEvent evt;

		int i;
		forYsArray(i,playerGround->cmdLog)
		{
			evt.Initialize();
			evt.eventType=FSEVENT_GNDCOMMAND;
			evt.objKey=FsExistence::GetSearchKey(playerGround);
			evt.str=playerGround->cmdLog[i];
			simEvent->AddEvent(currentTime,evt);
		}
	}

	simEvent->DeleteFutureEventForResume(currentTime);

	ClearUserInterface();    // 2009/03/29

	SetTerminate(YSFALSE);
	endTime=0.0;
	escKeyCount=0;
	PassedTime();
}

void FsSimulation::AssignUniqueYsfId(void)
{
	int maxYsfId;
	FsAirplane *air;
	FsGround *gnd;

	maxYsfId=GetLargestYsfId();

	maxYsfId++;

	air=NULL;
	while(NULL!=(air=FindNextAirplane(air)))
	{
		if(air->ysfId==0)
		{
			air->ysfId=maxYsfId++;
		}
	}

	gnd=NULL;
	while(NULL!=(gnd=FindNextGround(gnd)))
	{
		if(gnd->ysfId==0)
		{
			gnd->ysfId=maxYsfId++;
		}
	}
}

int FsSimulation::GetLargestYsfId(void) const
{
	int maxYsfId;
	FsAirplane *air;
	FsGround *gnd;

	maxYsfId=0;
	air=NULL;
	while(NULL!=(air=FindNextAirplane(air)))
	{
		maxYsfId=YsGreater(maxYsfId,air->ysfId);
	}

	gnd=NULL;
	while(NULL!=(gnd=FindNextGround(gnd)))
	{
		maxYsfId=YsGreater(maxYsfId,gnd->ysfId);
	}

	return maxYsfId;
}

void FsSimulation::ReadControlBackFromAirplane(const class FsAirplane *air)
{
	air->Prop().ReadBackControl(userInput);
	userInput.ctlRudder=0;
}

void FsSimulation::SimulateOneStep(
    const double &passedTime,
    YSBOOL demoMode,YSBOOL record,YSBOOL showTimer,YSBOOL networkStandby,FSUSERCONTROL userControl,
    YSBOOL showTimeMarker)
{
	#ifdef CRASHINVESTIGATION
		printf("S0\n");
	#endif


	if(NULL==firstPlayer.GetObject(this) && NULL!=GetPlayerObject())
	{
		firstPlayer.SetObject(GetPlayerObject());
	}


	if(airTrafficSequence->GetNextUpdateTime()<currentTime)
	{
		airTrafficSequence->RefreshAirTrafficSlot(this);
	}
	airTrafficSequence->RefreshRunwayUsage(this);

	if(pause!=YSTRUE)
	{
		double deltaTime=YsSmaller(passedTime,3.0);

		std::vector <std::function <void()> > taskArray;
		taskArray.push_back(std::bind(&FsSimulation::SimCacheFieldElevation,this));
		   // SimCacheFieldElevation should come first.  Otherwise, when passedTime is large,
		   // airplanes may sink into a terrain on which they are placed.  //2008/01/21
		taskArray.push_back(std::bind(&FsSimulation::SimCacheRectRegion,this));
		taskArray.push_back(std::bind(&FsSimulation::SimComputeAirToObjCollision,this));
		threadPool.Run(taskArray.size(),taskArray.data());

		SimProcessCollisionAndTerrain(passedTime);

		const double stepTime=0.025;
		while(deltaTime>YsTolerance)
		{
			double dt;
			if(cfgPtr->accurateTime==YSTRUE)
			{
				dt=YsSmaller(deltaTime,stepTime);
			}
			else
			{
				dt=deltaTime;
			}
		#ifdef CRASHINVESTIGATION
			printf("S1\n");
		#endif
			SimMove(dt);

		#ifdef CRASHINVESTIGATION
			printf("S2\n");
		#endif
			bulletHolder.PlayRecord(currentTime,dt);

		#ifdef CRASHINVESTIGATION
			printf("S3\n");
		#endif
			explosionHolder.PlayRecord(currentTime,dt);


			SimPlayTimedEvent(currentTime);


			currentTime+=dt;
			deltaTime-=dt;

		#ifdef CRASHINVESTIGATION
			printf("S4\n");
		#endif
			// For some reason, FsFormation doesn't work for higher fps
			if(currentTime>=nextControlTime)
			{
				SimControlByComputer(currentTime-lastControlledTime);
				lastControlledTime=currentTime;
				nextControlTime=currentTime+0.025;
			}

		#ifdef CRASHINVESTIGATION
			printf("S4.5\n");
		#endif

			if(demoMode!=YSTRUE && networkStandby!=YSTRUE && userControl!=FSUSC_DISABLE)
			{
				SimControlByUser(dt,userControl);  // CAUTION: SimControlByUser must be after SimControlByComputer
			}

		#ifdef CRASHINVESTIGATION
			printf("S5\n");
		#endif
			if(record==YSTRUE)
			{
				if(currentTime>nextAirRecordTime)
				{
					SimRecordAir(currentTime,YSFALSE);
					nextAirRecordTime=currentTime+0.05;
				}
				if(currentTime>nextGndRecordTime)
				{
					SimRecordGnd(currentTime,YSFALSE);
					nextGndRecordTime=currentTime+1.0;
				}
			}

			FsAirplane *playerPlane=GetPlayerAirplane();
			if(playerPlane!=NULL &&
			   playerPlane->IsAlive()==YSTRUE)
			{
				if(playerPlane->Prop().CheckHasJustTouchDown()==YSTRUE)
				{
					FsSoundSetOneTime(FSSND_ONETIME_TOUCHDWN);
				}

				if(ppGear!=pGear)
				{
					ppGear=pGear;
				}
				if(pGear!=Gear)
				{
					pGear=Gear;
				}
				Gear=playerPlane->Prop().GetLandingGear();
				if(ppGear<=pGear && pGear>Gear)
				{
					if(hideNextGearSound!=YSTRUE)
					{
						FsSoundSetOneTime(FSSND_ONETIME_GEARUP);
					}
					else
					{
						hideNextGearSound=YSFALSE;
					}
				}
				if(ppGear>=pGear && pGear<Gear)
				{
					if(hideNextGearSound!=YSTRUE)
					{
						FsSoundSetOneTime(FSSND_ONETIME_GEARDOWN);
					}
					else
					{
						hideNextGearSound=YSFALSE;
					}
				}

				if(ppFlap!=pFlap)
				{
					ppFlap=pFlap;
				}
				if(pFlap!=Flap)
				{
					pFlap=Flap;
				}
				Flap=playerPlane->Prop().GetFlap();
				if(ppFlap<=pFlap && pFlap>Flap)
				{
					FsSoundSetOneTime(FSSND_ONETIME_FLAPUP);
				}
				if(ppFlap>=pFlap && pFlap<Flap)
				{
					FsSoundSetOneTime(FSSND_ONETIME_FLAPDOWN);
				}
			}

			FsPlugInCallInterval(currentTime,this);
			for(auto &addOnPtr : addOnList)
			{
				addOnPtr->OnInterval(this,dt);
			}
		}

	#ifdef CRASHINVESTIGATION
		printf("S6\n");
	#endif

		SimProcessAirTrafficController();

		SimCheckTailStrike();
	}
	else // if(pause==YSTRUE)
	{
		SimControlByUser(passedTime,userControl);
	}



	SimInFlightDialogTransition();
	SimPlayerAircraftGetTrouble();



	if(networkStandby!=YSTRUE)
	{
	#ifdef CRASHINVESTIGATION
		printf("S7\n");
	#endif
		SimCheckEndOfSimulation();

		SimBlastSound(demoMode); // 2005/06/23

	#ifdef CRASHINVESTIGATION
		printf("S8\n");
	#endif

		needRedraw=YSTRUE;

		if(focusAir==NULL)
		{
			focusAir=GetPlayerAirplane();
			if(focusAir==NULL)
			{
				focusAir=FindNextAirplane(NULL);
			}
		}

		if(CheckNoExtAirView()==YSTRUE)  // 2006/06/11
		{
			focusAir=GetPlayerAirplane();
		}

		DecideAllViewPoint(passedTime);
	}

#ifdef CRASHINVESTIGATION
	printf("S9\n");
#endif

	ltc.Update(this);

#ifdef CRASHINVESTIGATION
	printf("S-1\n");
#endif
}

void FsSimulation::DecideAllViewPoint(const double dt)
{
	SimDecideViewpointAndCheckIsInCloud(mainWindowActualViewMode,mainWindowViewmode,FsGetMainWindowDrawingAreaSize());
	SimAutoViewChange(mainWindowActualViewMode.actualViewMode,dt);
	for(int i=0; i<FsMaxNumSubWindow; i++)
	{
		if(FsIsSubWindowOpen(i)==YSTRUE)
		{
			SimDecideViewpointAndCheckIsInCloud(subWindowActualViewMode[i],subWindowViewmode[i],FsGetSubWindowDrawingAreaSize());
		}
	}
}

void FsSimulation::AfterSimulation(void)
{
	fsConsole.SetAutoFlush(YSTRUE);

	fsConsole.Printf("Simulation Ended\n");

	fsConsole.Printf("Terminating Sound\n");
	FsSoundStopAll();

	fsConsole.Printf("Collecting Flight Record 1/1\n");
	bulletHolder.CollectRecord();
	fsConsole.Printf("Collecting Flight Record 2/2\n");
	explosionHolder.CollectRecord();

	simEvent->SortEventByTime();

	fsConsole.Printf("Post Simulation Process done\n");

	for(int i=0; i<FsMaxNumSubWindow; i++)
	{
		if(FsIsSubWindowOpen(i)==YSTRUE)
		{
			FsCloseSubWindow(i);
			FsSelectMainWindow();
		}
	}
	FsSplitMainWindow(YSFALSE);

	*weather=*iniWeather;
}


void FsSimulation::SetNeedRedraw(YSBOOL needRedraw)
{
	this->needRedraw=needRedraw;
}
YSBOOL FsSimulation::NeedRedraw(void) const
{
	return needRedraw;
}
void FsSimulation::DrawInNormalSimulationMode(FsSimulation::FSSIMULATIONSTATE simState,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker) const
{
	switch(simState)
	{
	case FSSIMSTATE_CENTERJOYSTICK:
		CenterJoystickDraw();
		break;
	case FSSIMSTATE_INITIALIZE:
		FsClearScreenAndZBuffer(YsBlue());
		break;
	case FSSIMSTATE_RUNNING:
		SimDrawAllScreen(demoMode,showTimer,showTimeMarker);
		break;
	case FSSIMSTATE_CHECKCONTINUE:
		CheckContinueDraw();
		break;
	case FSSIMSTATE_TERMINATING:
		FsClearScreenAndZBuffer(YsBlue());
		break;
	case FSSIMSTATE_OVER:
		FsClearScreenAndZBuffer(YsBlue());
		break;
	}
}


void FsSimulation::GenerateEnemyAirplane(
    int &nEnemyMax,double &gLimit,const double &timeRemain,YSBOOL allowAAM,YSBOOL jet,YSBOOL ww2)
{
	int nCurrentEnemy,nEnemyShouldBe,minutePassed;
	YsVec3 refPos;
	YSBOOL airExist;

	refPos=YsOrigin();
	airExist=YSFALSE;

	FsAirplane *airSeek;
	airSeek=NULL;
	while((airSeek=FindNextAirplane(airSeek))!=NULL)
	{
		if(airSeek->Prop().IsActive()==YSTRUE && airSeek->iff!=FS_IFF3)
		{
			refPos=airSeek->GetPosition();
			airExist=YSTRUE;

			// FSAIRPLANECATEGORY c;
			// c=airSeek->Prop().GetAirplaneCategory();
			// if(c==FSAC_WW2FIGHTER || c==FSAC_WW2BOMBER)
			// {
			// 	ww2=YSTRUE;
			// }
			// else
			// {
			// 	jet=YSTRUE;
			// }
		}
	}


	int nCat;
	FSAIRPLANECATEGORY cat[2];
	nCat=0;
	if(ww2==YSTRUE)
	{
		cat[nCat++]=FSAC_WW2FIGHTER;
	}
	if(jet==YSTRUE)
	{
		cat[nCat++]=FSAC_FIGHTER;
	}

	if(nCat==0)
	{
		cat[nCat++]=FSAC_FIGHTER;
	}


	if(airExist==YSTRUE)
	{
		nCurrentEnemy=0;
		airSeek=NULL;
		while((airSeek=FindNextAirplane(airSeek))!=NULL)
		{
			if(airSeek->Prop().IsActive()==YSTRUE && airSeek->iff==FS_IFF3)
			{
				nCurrentEnemy++;
			}
		}

		// When to generate enemies?
		//   1. All enemy airplanes are killed
		//   2. Enemy airplane is not killed during a certain amount of time

		minutePassed=15-int(timeRemain/60.0);
		nEnemyShouldBe=YsBound((minutePassed-2)/3,0,5);


		if(nCurrentEnemy==0 || nEnemyMax<nEnemyShouldBe) // Need to generate new one
		{
			if(nEnemyMax<5)
			{
				nEnemyMax++;
			}

			double radial;
			radial=double(rand()%360)*YsPi/180.0;

			int i;
			YsVec3 diff,neoPos;

			int nAir,airId;
			char *air[256];
			FsAirplane *neo;

			FsAirplane *prevAir;
			FsDogfight *prevDf;

			world->GetAirplaneListByMultiCategory(nAir,air,256,nCat,cat);

			prevAir=NULL;
			prevDf=NULL;
			for(i=nCurrentEnemy; i<nEnemyMax; i++)
			{
				diff.Set(sin(radial),0.0,cos(radial));
				diff=diff*8000.0;

				neoPos=refPos+diff;
				neoPos.SetY(4000.0);

				airId=rand()%nAir;


				neo=world->AddAirplane(air[airId],YSFALSE);

				if(neo!=NULL)
				{
					neo->SendCommand("UNLOADWP");
					if(YSTRUE==allowAAM)
					{
						if(2<=neo->Prop().GetMaxNumWeapon(FSWEAPON_AIM9X))
						{
							neo->SendCommand("LOADWEPN AIM9X 1");
							neo->SendCommand("LOADWEPN AIM9 1");
						}
						else
						{
							neo->SendCommand("LOADWEPN AIM9 2");
						}
					}
					neo->SendCommand("CTLLDGEA FALSE");
					neo->SendCommand("INITFUEL 100%");
					neo->airFlag=FSAIRFLAG_AUTOGENERATED;

					YsVec3 vel;
					vel=-diff;
					vel.Normalize();
					vel*=120.0;
					neo->Prop().SetVelocity(vel);
					neo->Prop().SetPosition(neoPos);

					FsDogfight *df;
					df=FsDogfight::Create();
					df->gLimit=gLimit;
					df->minAlt=1000.0;
					neo->SetAutopilot(df);

					neo->iff=FS_IFF3;

					if(prevAir==NULL)
					{
						prevAir=neo;
						prevDf=df;
					}
					else
					{
						prevDf->wingmanAirplaneKey=FsExistence::GetSearchKey(neo);
						df->wingmanAirplaneKey=FsExistence::GetSearchKey(prevAir);
						prevAir=NULL;
					}

					if(netServer!=NULL)
					{
						netServer->BroadcastAddAirplane(neo,FSNET_REMOTE);
					}
				}

				radial+=YsDegToRad(40.0);
			}

			gLimit=YsSmaller(gLimit+0.7,8.5);
		}
	}
}

void FsSimulation::GenerateAttackerAirplane(
    int nAttacker,const FsMissionEnemyGroundAttackerInfo &info,const double &iniDistFixedWing,const double & /*iniDistRotorWing*/,
    YSBOOL reducedAttackDist,const double initialSpeed)
{
	int nCurrentAttacker;
	FsAirplane *airSeeker;
	FsGround *gndSeeker;

	YSBOOL refPosIsSet;
	YsVec3 refPos;

	refPos=YsOrigin();
	refPosIsSet=YSFALSE;

	/* playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL)
	{
		refPos=playerPlane->GetPosition();
		refPosIsSet=YSTRUE;
	} */

	if(refPosIsSet!=YSTRUE)
	{
		gndSeeker=NULL;
		while((gndSeeker=FindNextGround(gndSeeker))!=NULL)
		{
			if(gndSeeker->IsAlive()==YSTRUE &&
			   gndSeeker->primaryTarget==YSTRUE &&
			   gndSeeker->iff==FS_IFF0)
			{
				refPos=gndSeeker->GetPosition();
				refPosIsSet=YSTRUE;
				break;
			}
		}
	}


	if(refPosIsSet==YSTRUE)
	{
		// Counting number of attackers/air support flying
		nCurrentAttacker=0;
		airSeeker=NULL;
		while((airSeeker=FindNextAirplane(airSeeker))!=NULL)
		{
			FsAutopilot *currentAp;
			currentAp=airSeeker->GetAutopilot();
			if(airSeeker->Prop().IsActive()==YSTRUE &&
			   airSeeker->iff==FS_IFF3)
			{
				if(airSeeker->Prop().GetNumWeapon(FSWEAPON_AGM65)>0 ||
				   airSeeker->Prop().GetNumWeapon(FSWEAPON_BOMB)>0 ||
				   airSeeker->Prop().GetNumWeapon(FSWEAPON_ROCKET)>0 ||
				   airSeeker->Prop().GetNumWeapon(FSWEAPON_BOMB250)>0 ||
				   (airSeeker->Prop().GetPosition()-refPos).GetLength()<10000.0)
				{
					nCurrentAttacker++;
				}
				else if(airSeeker->Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
				        airSeeker->Prop().GetAirplaneCategory()==FSAC_WW2FIGHTER)
				{
					if(currentAp==NULL || currentAp->Type()!=FSAUTOPILOT_DOGFIGHT) // strcmp(currentAp->WhatItIs(),"FsDogfight")!=0)
					{
						FsDogfight *df;
						df=FsDogfight::Create();
						df->gLimit=4.0;
						df->minAlt=333.0;
						airSeeker->SetAutopilot(df);
					}
					nCurrentAttacker++;
				}
				else
				{
					// Delete attacker that ran out of ordinance.
					airSeeker->Prop().Crash(FSDIEDOF_NULL);
				}
			}
		}

		// If need more, let's add it.
		if(nCurrentAttacker<nAttacker)
		{
			int i,retryCount;
			YsVec3 center;
			FsGround *gndSeeker;



			YsArray <FSAIRPLANECATEGORY,200> catCandidate;
			if(info.jet==YSTRUE)
			{
				for(i=0; i<70; i++)
				{
					catCandidate.Append(FSAC_ATTACKER);
				}
				if(info.allowAirCover==YSTRUE)
				{
					for(i=0; i<10; i++)
					{
						catCandidate.Append(FSAC_FIGHTER);
					}
				}
				if(info.allowHeavyBomber==YSTRUE && info.allowBomb==YSTRUE)
				{
					for(i=0; i<20; i++)
					{
						catCandidate.Append(FSAC_HEAVYBOMBER);
					}
				}
			}
			if(info.ww2==YSTRUE)
			{
				for(i=0; i<70; i++)
				{
					catCandidate.Append(FSAC_WW2ATTACKER);
				}
				if(YSTRUE==info.allowBomb)
				{
					for(i=0; i<50; i++)
					{
						catCandidate.Append(FSAC_WW2DIVEBOMBER);
					}
				}
				if(YSTRUE==info.allowHeavyBomber && YSTRUE==info.allowBomb)
				{
					for(i=0; i<50; i++)
					{
						catCandidate.Append(FSAC_WW2BOMBER);
					}
				}
				if(info.allowAirCover==YSTRUE)
				{
					for(i=0; i<10; i++)
					{
						catCandidate.Append(FSAC_WW2FIGHTER);
					}
				}
			}
			if(catCandidate.GetN()==0)  // Zero-Division Protection
			{
				catCandidate.Append(FSAC_ATTACKER);
			}



			center=YsOrigin();
			gndSeeker=NULL;
			while((gndSeeker=FindNextGround(gndSeeker))!=NULL)
			{
				if(gndSeeker->IsAlive()==YSTRUE &&
				   gndSeeker->primaryTarget==YSTRUE &&
				   gndSeeker->iff==FS_IFF0)
				{
					center=gndSeeker->GetPosition();
					break;
				}
			}

			retryCount=10;
			for(i=nCurrentAttacker; i<nAttacker; i++)
			{
				double radial;
				YsVec3 offset,newPos;

			MAKERADIAL:
				radial=YsDegToRad(double(rand()%360));
				offset.Set(cos(radial),0.0,sin(radial));
				offset*=iniDistFixedWing;

				newPos=center+offset;
				newPos.SetY(1515.0);
				if((newPos-refPos).GetLength()<YsSmaller(5000.0,iniDistFixedWing*0.8))
				{
					retryCount--;
					if(retryCount>0)
					{
						goto MAKERADIAL;
					}
					else
					{
						return;
					}
				}

				FSAIRPLANECATEGORY cat;
			CHOOSEAIRPLANE:
				cat=catCandidate[rand()%catCandidate.GetN()];

				int nAirList;
				char *airList[1024];
				world->GetAirplaneListByCategory(nAirList,airList,1024,cat);
				if(nAirList>0)
				{
					int n;
					FsAirplane *neo;
					YsAtt3 att;
					YsVec3 dir,vel;
					const FsAirplaneTemplate *tmpl;

					n=rand()%nAirList;

					tmpl=world->GetAirplaneTemplate(airList[n]);
					if(tmpl==NULL ||
					   (info.allowStealth!=YSTRUE && tmpl->GetProperty()->GetRadarCrossSection()<0.95) ||
					   (cat!=FSAC_WW2BOMBER && // Always allow bomb for WW2BOMBER
					    info.allowBomb!=YSTRUE &&
					   (YSTRUE!=info.allowAGM || tmpl->GetProperty()->GetNumWeapon(FSWEAPON_AGM65)==0) &&
					    tmpl->GetProperty()->GetNumWeapon(FSWEAPON_ROCKET)==0))
					{
						retryCount--;
						if(retryCount>0)
						{
							goto CHOOSEAIRPLANE;
						}
						else
						{
							return;
						}
					}

					neo=world->AddAirplane(airList[n],YSFALSE);

					if(neo!=NULL)
					{
						dir=-offset;
						dir.Normalize();
						att.SetForwardVector(dir);
						vel=dir*initialSpeed;

						neo->Prop().SetVelocity(vel);
						neo->Prop().SetPosition(newPos);
						neo->Prop().SetAttitude(att);
						neo->Prop().SetNumWeapon(FSWEAPON_FLARE,rand()%2);
						neo->iff=FS_IFF3;
						neo->airFlag=FSAIRFLAG_AUTOGENERATED;

						neo->SendCommand("CTLLDGEA FALSE");
						if(cat==FSAC_FIGHTER || cat==FSAC_WW2FIGHTER)
						{
							neo->SendCommand("INITIAGM 0");
							neo->SendCommand("INITBOMB 0");
							neo->SendCommand("INITB250 0");

							FsDogfight *df;
							df=FsDogfight::Create();
							df->gLimit=5.0;
							df->minAlt=333.0;
							neo->SetAutopilot(df);
						}
						else
						{
							FsGroundAttack *ga=FsGroundAttack::Create();

							if(YSTRUE==reducedAttackDist)  // This is for ground-to-air mission
							{
								ga->agmReleaseDist=600.0;
								ga->rocketReleaseDist=600.0;
								ga->gunReleaseDist=500.0;
								ga->bomberAlt=300.0;

								ga->turnRadius=2000.0;
								ga->turnRadiusHeavy=3000.0;
								ga->inboundSpeed=90.0;

								ga->flareInterval=1.0;
								ga->breakOnMissile=YSTRUE;

								const FsGround *playerGround=GetPlayerGround();
								if(NULL!=playerGround)
								{
									ga->playerGroundKey=playerGround->SearchKey();
								}
							}

							neo->SetAutopilot(ga);
							if(info.allowBomb!=YSTRUE && 
							   cat!=FSAC_WW2BOMBER &&
							   cat!=FSAC_WW2DIVEBOMBER) // Always allow bomb for WW2BOMBER
							{
								neo->SendCommand("INITBOMB 0");
								neo->SendCommand("INITB250 0");
							}
						}

						if(YSTRUE!=info.allowAGM)
						{
							neo->SendCommand("INITIAGM 0");
						}

						if(netServer!=NULL)
						{
							netServer->BroadcastAddAirplane(neo,FSNET_REMOTE);
						}
					}
				}
			}
		}
	}
}

void FsSimulation::GenerateFriendlyAirplane(FsInterceptMissionInfo &info)
{
	int nFriendly;
	int nCurrentFriendly;
	FsAirplane *airSeeker;
	FsAirplane *playerPlane;

	nFriendly=info.numWingman;

	playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL &&
	   playerPlane->Prop().GetFlightState()!=FSGROUND &&
	   playerPlane->Prop().GetFlightState()!=FSGROUNDSTATIC)
	{
		// Counting number of attackers/air support flying
		nCurrentFriendly=0;
		airSeeker=NULL;
		while((airSeeker=FindNextAirplane(airSeeker))!=NULL)
		{
			if(airSeeker->iff==playerPlane->iff &&
			   airSeeker->IsAlive()==YSTRUE &&
			   airSeeker!=playerPlane)
			{
				if(airSeeker->Prop().GetFuelLeft()>YsTolerance)
				{
					nCurrentFriendly++;
				}
				else
				{
					// airSeeker->dat.SetAutopilot(NULL);
				}
			}
		}

		if(nFriendly<=nCurrentFriendly)
		{
			return;
		}

		const char *fieldName=field.GetIdName();

		int i;
		char stpName[256];
		for(i=0; world->GetFieldStartPositionName(stpName,fieldName,i)==YSOK; i++)
		{
			if(stpName[0]=='R' && stpName[1]=='W')
			{
				FsAirplane *air;
				FsDogfight *df;
				air=world->AddAirplane((char *)playerPlane->Prop().GetIdentifier(),YSFALSE);
				world->SettleAirplane(*air,stpName);
				air->iff=playerPlane->iff;
				air->SendCommand("INITIAGM 0");
				air->SendCommand("INITBOMB 0");


				const int nAim9=air->Prop().GetNumWeapon(FSWEAPON_AIM9);
				const int nAim9x=air->Prop().GetNumWeapon(FSWEAPON_AIM9X);
				const int nAim120=air->Prop().GetNumWeapon(FSWEAPON_AIM120);
				YsString reloadCommand[5];
				reloadCommand[0]="UNLOADWP";
				reloadCommand[1]="LOADWEPN IFLR 20";
				reloadCommand[2].Printf("LOADWEPN AIM120 %d",nAim120);
				reloadCommand[3].Printf("LOADWEPN AIM9 %d",nAim9);
				reloadCommand[4].Printf("LOADWEPN AIM9X %d",nAim9x);
				for(auto &cmd : reloadCommand)
				{
					air->AddReloadCommand(cmd);
				}


				df=FsDogfight::Create();
				df->gLimit=9.0;
				df->minAlt=333.0;
				air->SetAutopilot(df);
				air->gLimit=9.0;
				air->airFlag=FSAIRFLAG_AUTOGENERATED;

				if(netServer!=NULL)
				{
					netServer->BroadcastAddAirplane(air,FSNET_REMOTE);
				}

				nCurrentFriendly++;
				if(nFriendly<=nCurrentFriendly)
				{
					break;
				}
			}
		}
	}
}

void FsSimulation::GenerateTank
   (int nTank,FsCloseAirSupportMissionInfo & /*info*/,
    YSBOOL mobile,YSBOOL incremental,YSBOOL /*primaryTarget*/,
    FSIFF iff,int regionCode,int strength)
{
	int nCurrentTank;
	FsGround *gnd;

	nCurrentTank=0;

	if(incremental==YSTRUE)
	{
		gnd=NULL;
		while((gnd=FindNextGround(gnd))!=NULL)
		{
			if(gnd->Prop().IsAlive()==YSTRUE &&
			   gnd->iff==iff &&
			   (mobile!=YSTRUE || (gnd->gndFlag&FSGNDFLAG_CASMISSIONTANK)==FSGNDFLAG_CASMISSIONTANK))
			{
				nCurrentTank++;
			}
		}
	}

	// When to generate enemies?
	//   1. All enemy airplanes are killed
	//   2. Enemy airplane is not killed during a certain amount of time

	if(nCurrentTank<nTank)
	{
		int i,nRect;
		YsArray <YsVec3,64> rectList;
		YsVec3 rect[4];

		nRect=0;
		YsArray <const YsSceneryRectRegion *,16> rgn;
		if(field.SearchFieldRegionById(rgn,regionCode)==YSOK && rgn.GetN()>0)  // ID=6 Generator
		{
			for(i=0; i<rgn.GetN(); i++)
			{
				field.GetFieldRegionRect(rect,rgn[i]);
				rectList.Append(4,rect);
				nRect++;
			}
		}

		if(nRect>0 && nCurrentTank<nTank)
		{
			
			YsArray <char const *> tankName;
			world->GetGroundListByType(tankName,FSTANK);
			auto nTankName=tankName.GetN();
			decltype(nTankName) tankNameId;
			if(nTankName>0)
			{
				for(i=nCurrentTank; i<nTank; i++)
				{
					int rectId;
					double param[4],sum;
					YsVec3 neoPos;

					rectId=rand()%nRect;

					param[0]=(double)(rand()%100);
					param[1]=(double)(rand()%100);
					param[2]=(double)(rand()%100);
					param[3]=(double)(rand()%100);

					sum=param[0]+param[1]+param[2]+param[3];
					param[0]/=sum;
					param[1]/=sum;
					param[2]/=sum;
					param[3]/=sum;

					neoPos=rectList[rectId*4  ]*param[0]
					      +rectList[rectId*4+1]*param[1]
					      +rectList[rectId*4+2]*param[2]
					      +rectList[rectId*4+3]*param[3];


					tankNameId=rand()%nTankName;
					gnd=world->AddGround(tankName[tankNameId],YSFALSE);
					if(gnd!=NULL)
					{
						if(mobile==YSTRUE)
						{
							gnd->gndFlag|=FSGNDFLAG_CASMISSIONTANK;
						}

						if(strength>=0)
						{
							char cmd[256];
							sprintf(cmd,"STRENGTH %d",strength);
							gnd->SendCommand(cmd);
						}

						gnd->Prop().SetPosition(neoPos);
						UpdateGroundCarrierLoading(gnd,aircraftCarrierList.GetN(),aircraftCarrierList);
						UpdateGroundTerrainElevationAndNormal(gnd);
						gnd->Prop().PutOnGround();
						gnd->iff=iff;
						gnd->primaryTarget=YSTRUE;

						if(netServer!=NULL)
						{
							netServer->BroadcastAddGround(gnd,FSNET_REMOTE);
						}
					}
				}
			}
		}
	}
}

void FsSimulation::ReviveGround(void)
{
	FsGround *gnd;
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		if((gnd->gndFlag&FSGNDFLAG_DONTREVIVE)==0)
		{
			world->ReviveGround(gnd);
			if(netServer!=NULL)
			{
				netServer->BroadcastAddGround(gnd,FSNET_REMOTE);
			}
		}
	}
}

void FsSimulation::ResetAirplane(FsAirplane *air)
{
	world->ResetAirplane(air);
	if(air->_startPosition[0]!=0)
	{
		world->SettleAirplane(*air,air->_startPosition);
	}
	air->RecallCommand();
}

void FsSimulation::ResetGround(FsGround *gnd)
{
	const YsVec3 iniPos=gnd->initPosition;
	const YsAtt3 iniAtt=gnd->initAttitude;

	world->ResetGround(gnd);
	gnd->RecallCommand();

	gnd->Settle(iniPos);
	gnd->Settle(iniAtt);
}

const char *FsSimulation::FinalAirplaneStateString(FSFLIGHTSTATE sta) const
{
	switch(sta)
	{
	case FSFLYING:
	case FSSTALL:
		return "STILL FLYING, NOT LANDED";
	case FSGROUND:
		return "LANDED";
	case FSGROUNDSTATIC:
		return "LANDED AND SAFELY STOPPED";
	case FSDEAD:
	case FSDEADSPIN:
	case FSDEADFLATSPIN:
		return "DEAD";
	case FSOVERRUN:
		return "OVERRUN";
	}
	return "UNKNOWN";
}

void FsSimulation::ClearTimedMessage(void)
{
	int i;
	for(i=0; i<FSNUMTIMEDMESSAGE; i++)
	{
		timedMessage[i].timeRemain=0.0;
		timedMessage[i].wstr=L"";
	}
}

void FsSimulation::ClearUserInterface(void)
{
	showReplayDlg=YSFALSE;
	SetCurrentInFlightDialog(NULL);
}

void FsSimulation::AddTimedMessageWithNoLog(const char str[]) const
{
	int i,min;
	min=0;
	for(i=0; i<FSNUMTIMEDMESSAGE; i++)
	{
		if(timedMessage[i].timeRemain<YsTolerance)
		{
			min=i;
			break;
		}
		else if(timedMessage[i].timeRemain<timedMessage[min].timeRemain)
		{
			min=i;
		}
	}

	timedMessage[min].timeRemain=15.0;
	timedMessage[min].wstr.SetUTF8String(str);
}

void FsSimulation::AddTimedMessage(const char str[])
{
	AddTimedMessageWithNoLog(str);

	FsSimulationEvent evt;
	evt.eventType=FSEVENT_TEXTMESSAGE;
	evt.str.Set(str);
	AddEvent(evt);
}

void FsSimulation::AddEvent(const class FsSimulationEvent &evt)
{
	simEvent->AddEvent(currentTime,evt);
}

void FsSimulation::SimMove(const double &dt)
{
	int airId;
	FsAirplane *airplane;
	airplane=NULL;
	airId=0;

#ifdef CRASHINVESTIGATION_S1_LEVEL2
	printf("S1-1\n");
#endif

	while((airplane=FindNextAirplane(airplane))!=NULL)
	{
#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-2 %s\n",airplane->Prop().GetIdentifier());
#endif

		airplane->Prop().RecordClimbRatio(dt);
		airplane->prevPos=airplane->GetPosition();
		airplane->prevDt=dt;

		if(airplane->isPlayingRecord==YSTRUE)
		{
			airplane->PlayRecord(currentTime+dt,dt);
			airplane->Prop().ComputeCarrierLandingAfterReadingFlightRecord(dt,aircraftCarrierList);
		}
		else if(airplane->IsAlive()==YSTRUE)
		{
#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-3\n");
#endif
			airplane->Prop().Move(dt,aircraftCarrierList,*weather);  // Elevation must be set in Prop() before this function.

#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-4\n");
#endif

			if(cfgPtr->autoCoordination==YSTRUE &&
			   airplane->isPlayingRecord!=YSTRUE &&
			   airplane->netType==FSNET_LOCAL)
			{
				airplane->Prop().SmartRudder(dt);
			}

#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-5\n");
#endif

			FSDIEDOF diedOf;
			if(airplane->Prop().CheckHasJustTouchDown()==YSTRUE &&
			   airplane->Prop().CheckSafeTouchDown(diedOf)!=YSTRUE)
			{
				AirplaneCrash(airplane,diedOf,1);
			}

			// If it does not have an assigned home base, assign it >>
			if(0==airplane->GetHomeBaseName().Strlen())
			{
				if(YSTRUE==airplane->Prop().CheckHasJustTouchDown())
				{
					auto carrier=airplane->Prop().OnThisCarrier();
					if(NULL!=carrier && 0<strlen(carrier->GetName()))
					{
						airplane->SetHomeBaseName(FsSimInfo::CARRIER,carrier->GetName());
						printf("Home Base set to CARRIER %s\n",(const char *)carrier->GetName());
					}
				}
				else if(YSTRUE==airplane->Prop().CheckHasJustTouchDown() ||
				        YSTRUE==airplane->Prop().CheckHasJustAirborne())
				{
					auto rgn=FindAirportFromPosition(airplane->GetPosition());
					if(NULL!=rgn && 0<strlen(rgn->GetTag()))
					{
						airplane->SetHomeBaseName(FsSimInfo::AIRPORT,rgn->GetTag());
						printf("Home Base set to AIRPORT %s\n",(const char *)rgn->GetTag());
					}
				}
			}
			// If it does not have an assigned home base, assign it <<

#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-6\n");
#endif
		}

		if(airplane->Prop().IsAlive()==YSTRUE &&
		   airplane->Prop().IsActive()!=YSTRUE &&
		   airplane->Prop().GetFlightState()!=FSOVERRUN)
		{
			if(cfgPtr->useParticle==YSTRUE)
			{
				if(airplane->refTime1<currentTime || airplane->refTime2<currentTime)
				{
					YsVec3 vel;
					airplane->Prop().GetVelocity(vel);
					vel.Normalize();

					const YsVec3 particlePos=airplane->GetPosition()+vel*airplane->Prop().GetOutsideRadius()*0.5;

					if(airplane->refTime1<currentTime)
					{
						int i;
						FsParticle *particle;
						double vx,vy,vz;
						YsVec3 vel;

						for(i=0; i<2; i++)
						{
							vx=(double)(rand()%1000)/50.0;
							vy=(double)(rand()%1000)/50.0;
							vz=(double)(rand()%1000)/50.0;
							vel.Set(vx,vy,vz);
							particle=particleStore.CreateParticle(particlePos,vel,0.3,3.0,0.0);
							particle->SetGravity(YsOrigin());
							particle->SetSize(3.0,20.0,5.0);
							particle->SetColorTransition(FSPARTICLECOLOR_BURN);
						}

						for(i=0; i<5; i++)
						{
							vx=(double)(rand()%1000)/50.0;
							vy=(double)(rand()%1000)/50.0;
							vz=(double)(rand()%1000)/50.0;
							vel.Set(vx,vy,vz);
							particle=particleStore.CreateParticle(particlePos,vel,0.3,2.0,0.0);
							particle->SetGravity(YsOrigin());
							particle->SetSize(3.0,20.0,5.0);
							particle->SetColorTransition(FSPARTICLECOLOR_BURN);
						}

						for(i=0; i<7; i++)
						{
							vx=(double)(rand()%1000)/30.0;
							vy=(double)(rand()%1000)/30.0;
							vz=(double)(rand()%1000)/30.0;
							vel.Set(vx,vy,vz);
							particle=particleStore.CreateParticle(particlePos,vel,0.3,1.5,0.0);
							particle->SetSize(3.0,20.0,5.0);
							particle->SetGravity(YsOrigin());
							particle->SetColorTransition(FSPARTICLECOLOR_BURN);
						}

						airplane->refTime1=currentTime+0.03;  // 30 times per sec
					}
					if(airplane->refTime2<currentTime)
					{
						bulletHolder.ThrowRandomDebris(currentTime,particlePos,airplane->GetAttitude(),60.0);
						airplane->refTime2=currentTime+0.18;
					}
				}
			}
			else
			{
				if(currentTime>airplane->refTime1)
				{
					explosionHolder.Explode(currentTime,airplane->GetPosition(),0.7,7.0,15.0,YSFALSE,NULL,YSFALSE);
					airplane->refTime1=currentTime+0.15;
				}
			}
		}

		if(airplane->Prop().GetPendingVorYsfId(0)>0)
		{
			FsGround *gnd=FindGroundByYsfId(airplane->Prop().GetPendingVorYsfId(0));
			if(NULL!=gnd)
			{
				airplane->Prop().SetVorStation(0,FsExistence::GetSearchKey(gnd));
				airplane->Prop().SetPendingVorYsfId(0,0);
			}
		}
		if(airplane->Prop().GetPendingVorYsfId(1)>0)
		{
			FsGround *gnd=FindGroundByYsfId(airplane->Prop().GetPendingVorYsfId(1));
			if(NULL!=gnd)
			{
				airplane->Prop().SetVorStation(0,FsExistence::GetSearchKey(gnd));
				airplane->Prop().SetPendingVorYsfId(1,0);
			}
		}
		if(airplane->Prop().GetPendingNdbYsfId()>0)
		{
			FsGround *gnd=FindGroundByYsfId(airplane->Prop().GetPendingNdbYsfId());
			if(NULL!=gnd)
			{
				airplane->Prop().SetNdbStation(FsExistence::GetSearchKey(gnd));
				airplane->Prop().SetPendingNdbYsfId(0);
			}
		}

		airplane->SetTransformationToCollisionShell(airplane->Prop().GetMatrix());
		airId++;

#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-9\n");
#endif
	}

	FsGround *ground;
	tallestGroundObjectHeight=-10000.0;  // Enough Small Value!
	ground=NULL;
	while((ground=FindNextGround(ground))!=NULL)
	{
		ground->prevPos=ground->GetPosition();
		ground->prevDt=dt;

		if(ground->isPlayingRecord==YSTRUE)
		{
			YSBOOL prevAlive;
			prevAlive=ground->IsAlive();
			ground->PlayRecord(currentTime+dt,dt);
			if(YSTRUE==prevAlive && YSTRUE!=ground->IsAlive())
			{
				if(YSTRUE==cfgPtr->useParticle && replayMode==FSREPLAY_PLAY)
				{
					auto partGenPtr=particleStore.CreateGenerator(FSPARTICLEGENERATOR_BURN,ground->GetPosition(),YsYVec(),10.0,ground->GetPosition().y());
					partGenPtr->SetSize(3.0,20.0,3.0);
				}
			}
		}
		else if(0!=(ground->gndFlag&FSGNDFLAG_DONTMOVEUNLESSUSERISCONTROLLING) &&
		        YSTRUE!=ground->Prop().IsControlledByUser())
		{
			// Don't move it.
		}
		else if(ground->IsAlive()==YSTRUE)
		{
			ground->Prop().Move
			   (ground->motionPathOffset,ground->motionPathIndex,ground->useMotionPathOffset,
			    ground->motionPathPnt.GetN(),ground->motionPathPnt,ground->motionPathIsLoop,
			    dt);
		}

		tallestGroundObjectHeight=YsGreater
			(tallestGroundObjectHeight,ground->GetPosition().y()+ground->GetApproximatedCollideRadius());

		if(0!=(ground->gndFlag&FSGNDFLAG_DONTMOVEUNLESSUSERISCONTROLLING) &&
		        YSTRUE!=ground->Prop().IsControlledByUser())
		{
			// Don't shoot.
		}
		else if(ground->isPlayingRecord!=YSTRUE)
		{
			ground->Prop().FireGun(currentTime,dt,this,bulletHolder,ground);
			if(ground->netType==FSNET_LOCAL)
			{
				// Missiles will be notified by packets
				ground->Prop().FireMissile(currentTime,bulletHolder,ground);
			}
		}

		YsMatrix4x4 mat;
		mat.Translate(ground->GetPosition());
		mat.Rotate(ground->GetAttitude());
		ground->SetTransformationToCollisionShell(mat);
	}

#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-20\n");
#endif

	bulletHolder.Move(dt,currentTime,*weather);
	if(NULL!=GetPlayerGround() && NULL!=GetPlayerGround()->Prop().GetAirTarget())
	{
		FsExistence *target=GetPlayerGround()->Prop().GetAirTarget();
		bulletHolder.CalculateBulletCalibrator(target);
	}
	else
	{
		bulletHolder.ClearBulletCalibrator();
	}
	explosionHolder.Move(dt);
	particleStore.Move(dt,weather->GetWind());

	weather->WeatherTransition(dt);

	airplane=NULL;
	while((airplane=FindNextAirplane(airplane))!=NULL)
	{
		if(airplane->isPlayingRecord!=YSTRUE && airplane->IsAlive()==YSTRUE)
		{
			airplane->Prop().FireGunIfVirtualTriggerIsPressed(currentTime,dt,this,bulletHolder,airplane);

			YSBOOL fired,blockedByBombBay;
			FSWEAPONTYPE woc;
			fired=airplane->Prop().ProcessVirtualButtonPress(blockedByBombBay,woc,this,currentTime,bulletHolder,airplane);
			if(fired==YSTRUE && airplane==GetPlayerAirplane())
			{


				switch(woc)
				{
				case FSWEAPON_AIM9:
				case FSWEAPON_AIM9X:
				case FSWEAPON_AIM120:
					FsSoundSetOneTime(FSSND_ONETIME_MISSILE);
					break;
				default:
				case FSWEAPON_AGM65:
				case FSWEAPON_ROCKET:
					FsSoundSetOneTime(FSSND_ONETIME_ROCKET);
					break;
				case FSWEAPON_BOMB:
				case FSWEAPON_BOMB500HD:
				case FSWEAPON_BOMB250:
					FsSoundSetOneTime(FSSND_ONETIME_BOMBSAWAY);
					break;
				};

			}
			else if(fired!=YSTRUE && blockedByBombBay==YSTRUE)
			{
				if(airplane==GetPlayerAirplane())
				{
					userInput.ctlBombBayDoor=YSTRUE;
					AddTimedMessage("Opening Bomb Bay Door");
				}
				airplane->Prop().SetBombBayDoor(1.0);
			}
		}
	}

#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-30\n");
#endif

	int i;
	for(i=0; i<FSNUMTIMEDMESSAGE; i++)
	{
		if(timedMessage[i].timeRemain>0.0)
		{
			timedMessage[i].timeRemain-=dt;
			if(timedMessage[i].timeRemain<0.0)
			{
				timedMessage[i].timeRemain=0.0;
			}
		}
	}

#ifdef CRASHINVESTIGATION_S1_LEVEL2
		printf("S1-40\n");
#endif
}

void FsSimulation::SimCheckTailStrike(void)
{
	FsAirplane *air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->IsAlive()==YSTRUE && air->isPlayingRecord!=YSTRUE && air->isNetSubstitute!=YSTRUE)
		{
			YsVec3 untransformed;
			if(YSTRUE==air->TestTailStrike(untransformed))
			{
				if(cfgPtr->noTailStrike==YSTRUE)
				{
					if(air==GetPlayerAirplane())
					{
						AddTimedMessage("Tail Strike!");
						air->Prop().BouncePitchByTailStrike();
					}
				}
				else
				{
					if(untransformed.z()<-YsAbs(untransformed.x())*2.0)  // Really tail?
					{
						AirplaneCrash(air,FSDIEDOF_TAILSTRIKE,1);
					}
					else
					{
						AirplaneCrash(air,FSDIEDOF_TERRAIN,1);
					}
				}
			}
		}
	}
}

void FsSimulation::SimCacheFieldElevation(void)
{
	int airId;

	airId=0;
	for(FsAirplane *airPtr=NULL; (airPtr=FindNextAirplane(airPtr))!=NULL; )
	{
		if(airPtr->IsAlive()==YSTRUE)
		{
			const YsVec3 &pos=airPtr->GetPosition();

			airPtr->elevation=0.0;
			airPtr->terrainOrg.Set(pos.x(),0.0,pos.z());
			airPtr->terrainNom=YsYVec();

			FsGround *carrier=airPtr->Prop().OnThisCarrier();
			if(airPtr->Prop().IsOnCarrier()==YSTRUE && NULL!=carrier)
			{
				for(int i=0; i<airPtr->Prop().GetNumTire(); i++)
				{
					YsVec3 pos,nom;
					airPtr->Prop().GetMatrix().Mul(pos,airPtr->Prop().GetTirePosition(i),1.0);

					const double elv=carrier->Prop().GetAircraftCarrierProperty()->GetDeckHeightAndNormal(nom,pos);

					YsVec3 foot(pos.x(),elv,pos.z());
					airPtr->Prop().SetElevationAtTire(i,foot);
				}

				YsVec3 nom;
				const double elv=carrier->Prop().GetAircraftCarrierProperty()->GetDeckHeightAndNormal(nom,airPtr->GetPosition());
				airPtr->elevation=elv;
				airPtr->terrainOrg.Set(pos.x(),elv,pos.z());
				airPtr->terrainNom=nom;
			}
			else
			{
				double elv;
				YsVec3 nom;
				field.GetFieldElevationAndNormal(elv,nom,pos.x(),pos.z());
				airPtr->elevation=elv;
				airPtr->terrainOrg.Set(pos.x(),elv,pos.z());
				airPtr->terrainNom=nom;

				for(int i=0; i<airPtr->Prop().GetNumTire(); i++)
				{
					YsVec3 pos;
					airPtr->Prop().GetMatrix().Mul(pos,airPtr->Prop().GetTirePosition(i),1.0);

					YsVec3 nom;
					double elv;
					field.GetFieldElevationAndNormal(elv,nom,pos.x(),pos.z());

					YsVec3 foot(pos.x(),elv,pos.z());
					airPtr->Prop().SetElevationAtTire(i,foot);
				}
			}

			airPtr->Prop().SetFieldElevation(airPtr->elevation);
			airPtr->Prop().SetFieldNormal(airPtr->terrainNom);
			airPtr->Prop().SetBaseElevation(GetBaseElevation());
		}
		airId++;
	}

	for(FsGround *gndPtr=NULL; (gndPtr=FindNextGround(gndPtr))!=NULL; )
	{
		UpdateGroundTerrainElevationAndNormal(gndPtr);
	}
}

void FsSimulation::SimCacheRectRegion(void)
{
	for(FsAirplane *airPtr=NULL; NULL!=(airPtr=FindNextAirplane(airPtr)); )
	{
		if(YSTRUE==airPtr->IsAlive())
		{
			if(YsTolerance<airPtr->Prop().GetVelocity() || YSTRUE!=airPtr->rectRgnCached)
			{
				field.GetFieldRegion(airPtr->rectRgnCache,airPtr->GetPosition().x(),airPtr->GetPosition().z());
				airPtr->rectRgnCached=YSTRUE;
			}
		}
		else
		{
			airPtr->rectRgnCache.Clear();
		}
	}
	for(FsGround *gndPtr=NULL; NULL!=(gndPtr=FindNextGround(gndPtr)); )
	{
		if(YSTRUE==gndPtr->IsAlive())
		{
			if(YsTolerance<gndPtr->Prop().GetVelocity() || YSTRUE!=gndPtr->rectRgnCached)
			{
				field.GetFieldRegion(gndPtr->rectRgnCache,gndPtr->GetPosition().x(),gndPtr->GetPosition().z());
				gndPtr->rectRgnCached=YSTRUE;
			}
		}
		else
		{
			gndPtr->rectRgnCache.Clear();
		}
	}

}

void FsSimulation::UpdateGroundTerrainElevationAndNormal(FsGround *gndPtr)
{
	if(gndPtr->IsAlive()==YSTRUE)
	{
		const YsVec3 &pos=gndPtr->GetPosition();

		gndPtr->elevation=0.0;
		gndPtr->terrainOrg.Set(pos.x(),0.0,pos.z());
		gndPtr->terrainNom=YsYVec();

		FsGround *carrier=gndPtr->Prop().OnThisCarrier();
		if(gndPtr->Prop().IsOnCarrier()==YSTRUE && NULL!=carrier)
		{
			YsVec3 nom;
			const double elv=carrier->Prop().GetAircraftCarrierProperty()->GetDeckHeightAndNormal(nom,pos);
			gndPtr->elevation=elv;
			gndPtr->terrainOrg.Set(pos.x(),elv,pos.z());
			gndPtr->terrainNom=nom;
		}


		double elv;
		YsVec3 nom;
		field.GetFieldElevationAndNormal(elv,nom,pos.x(),pos.z());
		if(gndPtr->elevation<elv)
		{
			gndPtr->elevation=elv;
			gndPtr->terrainOrg.Set(pos.x(),elv,pos.z());
			gndPtr->terrainNom=nom;
		}
	}
}

void FsSimulation::SimComputeAirToObjCollision(void)
{
	YsArray <FsAirplane *,256> airCandidate;
	YsArray <FsGround *,256> gndCandidate;

	for(FsAirplane *airPtr=NULL; NULL!=(airPtr=FindNextAirplane(airPtr)); )
	{
		airPtr->airCollision.CleanUp();
		airPtr->gndCollision.CleanUp();
	}

	for(FsAirplane *air1=NULL; NULL!=(air1=FindNextAirplane(air1)); )
	{
		if(air1->IsAlive()==YSTRUE)
		{
			GetLattice().GetAirCollisionCandidate(airCandidate,air1);

			for(int j=0; j<airCandidate.GetN(); j++)
			{
				FsAirplane *air2=airCandidate[j];
				if(YSTRUE==cfgPtr->midAirCollision || YSTRUE==air2->Prop().IsRacingCheckPoint())
				{
					if(air1->SearchKey()<air2->SearchKey() &&
					   air2->IsAlive()==YSTRUE &&
					  (air1->Prop().IsActive()==YSTRUE || air2->Prop().IsActive()==YSTRUE))  // 2005/03/03
					{
						YsVec3 collPos;
						if(CheckMidAir(collPos,*air1,*air2)==YSTRUE)
						{
							air1->airCollision.Increment();
							air1->airCollision.Last().objKey=air2->SearchKey();
							air1->airCollision.Last().pos=collPos;

							air2->airCollision.Increment();
							air2->airCollision.Last().objKey=air1->SearchKey();
							air2->airCollision.Last().pos=collPos;
						}
					}
				}
			}
		}

		if(air1->GetPosition().y()-air1->GetApproximatedCollideRadius()<tallestGroundObjectHeight)
		{
			GetLattice().GetGndCollisionCandidate(gndCandidate,air1);
			for(int j=0; j<gndCandidate.GetN(); j++)
			{
				FsGround *gnd2=gndCandidate[j];
				if(YSTRUE==cfgPtr->midAirCollision || YSTRUE==gnd2->Prop().IsRacingCheckPoint())
				{
					if(gnd2->IsAlive()==YSTRUE)
					{
						YsVec3 collPos;
						if(CheckMidAir(collPos,*air1,*gnd2)==YSTRUE)
						{
							air1->gndCollision.Increment();
							air1->gndCollision.Last().objKey=gnd2->SearchKey();
							air1->gndCollision.Last().pos=collPos;

							gnd2->airCollision.Increment();
							gnd2->airCollision.Last().objKey=air1->SearchKey();
							gnd2->airCollision.Last().pos=collPos;
						}
					}
				}
			}
		}
	}
}

void FsSimulation::SimProcessCollisionAndTerrain(const double & /*dt*/)
{
	// Call HitObject BEFORE HitGround
	bulletHolder.HitObject(currentTime,&explosionHolder,this,tallestGroundObjectHeight);
	bulletHolder.HitGround(currentTime,field,&explosionHolder,this);

	YsArray <FsAirplane *,256> airCandidate;
	YsArray <FsGround *,256> gndCandidate;

	for(FsAirplane *airPtr=NULL; NULL!=(airPtr=FindNextAirplane(airPtr)); )
	{
		if(airPtr->isPlayingRecord!=YSTRUE)
		{
			int i;
			YsVec3 airPos;
			YSBOOL onCarrier;
			if(airPtr->Prop().IsOnGround()==YSTRUE)
			{
				airPos=airPtr->GetPosition();
				onCarrier=YSFALSE;
				for(i=0; i<aircraftCarrierList.GetN(); i++)
				{
					FsAircraftCarrierProperty *prop;
					prop=aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty();
					if(prop->IsOnDeck(airPos)==YSTRUE)
					{
						// Even if flight state is FSGROUND and the airplane is not on
						// the runway, possibly the airplane is on the flight deck of
						// an aircraft carrier.
						airPtr->Prop().SetOutOfRunway(YSFALSE);
						onCarrier=YSTRUE;
						break;
					}
				}

				if(onCarrier==YSTRUE)
				{
					goto NEXTAIRPLANE;
				}
			}

			FSDIEDOF diedOf;
			int collType;
			if(airPtr->HitGround(
			    diedOf,collType,currentTime,field,cfgPtr->takeCrash,cfgPtr->canLandAnywhere,&explosionHolder)==YSTRUE)
			{
				AirplaneCrash(airPtr,diedOf,collType);
			}
		}
	NEXTAIRPLANE:
		;
	}

	for(FsAirplane *air1=NULL; NULL!=(air1=FindNextAirplane(air1)); )
	{
		if(air1->IsAlive()==YSTRUE)
		{
			for(auto air2Coll : air1->airCollision)
			{
				auto air2=FindAirplane(air2Coll.objKey);
				if(nullptr!=air2 && (cfgPtr->midAirCollision==YSTRUE || YSTRUE==air2->Prop().IsRacingCheckPoint()))
				{
					if(air1->SearchKey()<air2->SearchKey() &&
					   air2->IsAlive()==YSTRUE &&
					  (air1->Prop().IsActive()==YSTRUE || air2->Prop().IsActive()==YSTRUE))  // 2005/03/03
					{
						Explode(*air1,YSTRUE);  // Sound=YSTRUE
						Explode(*air2,YSFALSE); // Sound=YSFALSE
						YSBOOL killed;
						air1->GetDamage(killed,12,FSDIEDOF_COLLISION);
						if(killed==YSTRUE)
						{
							bulletHolder.ThrowMultiDebris
							   (5,currentTime,air1->GetPosition(),air1->GetAttitude(),60.0);
						}

						air2->GetDamage(killed,12,FSDIEDOF_COLLISION);
						if(killed==YSTRUE)
						{
							bulletHolder.ThrowMultiDebris
							   (5,currentTime,air2->GetPosition(),air2->GetAttitude(),60.0);
						}
					}
				}
			}
			for(auto gnd2Coll : air1->gndCollision)
			{
				auto gnd2=FindGround(gnd2Coll.objKey);
				if(nullptr!=gnd2 && (cfgPtr->midAirCollision==YSTRUE || YSTRUE==gnd2->Prop().IsRacingCheckPoint()))
				{
					if(gnd2->IsAlive()==YSTRUE)
					{
						if(YSTRUE==gnd2->Prop().IsRacingCheckPoint())
						{
							FsSoundSetOneTime(FSSND_ONETIME_NOTICE);
							gnd2->Prop().SetState(FSGNDDEAD);
						}
						else
						{
							Explode(*air1,YSTRUE);  // Sound=YSTRUE
							Explode(*gnd2,YSFALSE); // Sound=YSFALSE

							YSBOOL killed;
							air1->GetDamage(killed,12,FSDIEDOF_COLLISION);
							if(killed==YSTRUE)
							{
								bulletHolder.ThrowMultiDebris
								   (5,currentTime,air1->GetPosition(),air1->GetAttitude(),60.0);
							}

							if(gnd2->Prop().GetAircraftCarrierProperty()!=NULL)
							{
								air1->GetDamage(killed,100,FSDIEDOF_COLLISION);
								air1->Prop().SetFlightState(FSDEAD,FSDIEDOF_COLLISION);
							}
							gnd2->GetDamage(killed,12,FSDIEDOF_COLLISION);
							if(killed==YSTRUE)
							{
								bulletHolder.ThrowMultiDebris
								   (5,currentTime,gnd2->GetPosition(),gnd2->GetAttitude(),60.0);
							}
						}
					}
				}
			}
		}
	}

	YsArray <FsExistence *,128> bounced;
	for(FsGround *gnd1=NULL; NULL!=(gnd1=FindNextGround(gnd1)); )
	{
		if(gnd1->IsAlive()==YSTRUE)
		{
			if((YSTRUE!=gnd1->Prop().IsStatic() || YSTRUE==gnd1->BouncedLastTime()) && YSTRUE!=gnd1->Prop().SkipGroundToGroundCollisionCheck())
			{
				// GetLattice().GetAirCollisionCandidate(airCandidate,gnd1);

				// for(int j=0; j<airCandidate.GetN(); j++)
				// {
				// 	FsAirplane *air2=airCandidate[j];
				// 	if(YSTRUE==air2->IsAlive())
				// 	{
				// 		YsVec3 collPos;
				// 		if(YSTRUE==CheckMidAir(collPos,*gnd1,*air2))
				// 		{
				// 			gnd1->Bounce(collPos);
				// 			bounced.Append(gnd1);
				// 		}
				// 	}
				// }

				GetLattice().GetGndCollisionCandidate(gndCandidate,gnd1);
				for(auto j=gndCandidate.GetN()-1; j>=0; j--)
				{
					FsGround *gnd2=gndCandidate[j];

					if(NULL==GetPlayerGround())
					{
						goto CARRIERCHECK;
					}

					if((YSTRUE!=gnd2->Prop().IsStatic() || YSTRUE==gnd2->BouncedLastTime()) && gnd1>gnd2)
					{
						goto CARRIERCHECK;
					}
					if(YSTRUE==gnd2->Prop().SkipGroundToGroundCollisionCheck())
					{
						goto CARRIERCHECK;
					}
					if((gnd1->Prop().OnThisCarrier()==gnd2 && gnd1->Prop().GetVelocity()<YsTolerance) ||
					   (gnd2->Prop().OnThisCarrier()==gnd1 && gnd2->Prop().GetVelocity()<YsTolerance))
					{
						goto CARRIERCHECK;
					}

					if(gnd2!=gnd1 && YSTRUE==gnd2->IsAlive())
					{
						YsVec3 collPos;
						if(YSTRUE==CheckMidAir(collPos,*gnd1,*gnd2))
						{
							gnd1->Bounce(collPos);
							gnd2->Bounce(collPos);
							bounced.Append(gnd1);
							bounced.Append(gnd2);
						}
					}

				CARRIERCHECK:
					if(NULL==gndCandidate[j]->Prop().GetAircraftCarrierProperty())
					{
						gndCandidate.DeleteBySwapping(j);
					}
				}

				// At this line, gndCandidate contains aircraft carrier candidates.

				UpdateGroundCarrierLoading(gnd1,gndCandidate.GetN(),gndCandidate);
			}
		}
	}

	for(FsAirplane *air1=NULL; NULL!=(air1=FindNextAirplane(air1)); )
	{
		air1->ClearBouncedLastTimeFlag();
	}
	for(FsGround *gnd1=NULL; NULL!=(gnd1=FindNextGround(gnd1)); )
	{
		gnd1->ClearBouncedLastTimeFlag();
	}

	for(int i=0; i<bounced.GetN(); i++)
	{
		bounced[i]->SetBouncedLastTimeFlag();
	}
}

YSBOOL FsSimulation::MayCollide(const YsVec3 &objPos,const double objRad,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const
{
	if(YSTRUE==MayCollideWithAir(objPos,objRad,selfPtr,nExclude,exclude) ||
	   YSTRUE==MayCollideWithGround(objPos,objRad,selfPtr,nExclude,exclude))
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsSimulation::MayCollideWithAir(const YsVec3 &objPos,const double objRad,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const
{
	YsArray <FsAirplane *,256> airCandidate;
	GetLattice().GetAirCollisionCandidate(airCandidate,objPos,objRad);
	for(int i=0; i<airCandidate.GetN(); i++)
	{
		const YsVec3 dif=objPos-airCandidate[i]->GetPosition();;
		const double air2Rad=airCandidate[i]->Prop().GetOutsideRadius();
		if(airCandidate[i]!=selfPtr &&
		   YSTRUE==airCandidate[i]->IsAlive() &&
		   YSTRUE!=YsIsIncluded <const FsExistence *> (nExclude,exclude,airCandidate[i]) && 
		   dif.GetSquareLength()<=YsSqr((objRad+air2Rad)))
		{
			printf("May collide with %s\n",airCandidate[i]->GetIdentifier());
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsSimulation::MayCollideWithGround(const YsVec3 &objPos,const double objRad,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const
{
	YsArray <FsGround *,256> gndCandidate;
	GetLattice().GetGndCollisionCandidate(gndCandidate,objPos,objRad);
	for(int i=0; i<gndCandidate.GetN(); i++)
	{
		const YsVec3 dif=objPos-gndCandidate[i]->GetPosition();
		const double gnd2Rad=gndCandidate[i]->Prop().GetOutsideRadius();
		if(gndCandidate[i]!=selfPtr &&
		   YSTRUE!=gndCandidate[i]->IsAlive() &&
		   YSTRUE!=YsIsIncluded <const FsExistence *> (nExclude,exclude,gndCandidate[i]) && 
		   dif.GetSquareLength()<=YsSqr((objRad+gnd2Rad)))
		{
			printf("May collide with %s\n",gndCandidate[i]->GetIdentifier());
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsSimulation::MayCollide(const YsVec3 &objPos,const YsAtt3 &objAtt,const double clearance,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const
{
	if(YSTRUE==MayCollideWithAir(objPos,objAtt,clearance,selfPtr,nExclude,exclude) ||
	   YSTRUE==MayCollideWithGround(objPos,objAtt,clearance,selfPtr,nExclude,exclude))
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsSimulation::MayCollideWithAir(const YsVec3 &objPos,const YsAtt3 &objAtt,const double clearance,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const
{
	const double objRad=selfPtr->GetApproximatedCollideRadius();

	YsArray <FsAirplane *,256> airCandidate;
	GetLattice().GetAirCollisionCandidate(airCandidate,objPos,objRad);
	if(0<airCandidate.GetN())
	{
		YsMatrix4x4 objMat;
		objMat.Multiply(objPos,objAtt);

		YsMatrix4x4 objMatInverse;
		objMatInverse.MultiplyInverse(objPos,objAtt);

		for(int i=0; i<airCandidate.GetN(); i++)
		{
			const YsVec3 dif=objPos-airCandidate[i]->GetPosition();;
			const double air2Rad=airCandidate[i]->Prop().GetOutsideRadius();
			if(airCandidate[i]!=selfPtr &&
			   YSTRUE!=YsIsIncluded <const FsExistence *> (nExclude,exclude,airCandidate[i]) && 
			   dif.GetSquareLength()<=YsSqr((objRad+air2Rad)))
			{
				if(airCandidate[i]->MayCollideWith(airCandidate[i]->GetInverseMatrix(),*selfPtr,objMat,clearance) &&
				   selfPtr->MayCollideWith(objMatInverse,*airCandidate[i],airCandidate[i]->GetMatrix(),clearance))
				{
					printf("Bbx may collide with %s\n",airCandidate[i]->GetIdentifier());
					return YSTRUE;
				}
			}
		}
	}
	return YSFALSE;
}

YSBOOL FsSimulation::MayCollideWithGround(const YsVec3 &objPos,const YsAtt3 &objAtt,const double clearance,const FsExistence *selfPtr,const int nExclude,const FsExistence * const exclude[]) const
{
	const double objRad=selfPtr->GetApproximatedCollideRadius();

	YsArray <FsGround *,256> gndCandidate;
	GetLattice().GetGndCollisionCandidate(gndCandidate,objPos,objRad);
	if(0<gndCandidate.GetN())
	{
		YsMatrix4x4 objMat;
		objMat.Multiply(objPos,objAtt);

		YsMatrix4x4 objMatInverse=objMat;
		objMatInverse.MultiplyInverse(objPos,objAtt);

		for(int i=0; i<gndCandidate.GetN(); i++)
		{
			const YsVec3 dif=objPos-gndCandidate[i]->GetPosition();
			const double gnd2Rad=gndCandidate[i]->Prop().GetOutsideRadius();
			if(gndCandidate[i]!=selfPtr &&
			   YSTRUE!=YsIsIncluded <const FsExistence *> (nExclude,exclude,gndCandidate[i]) && 
			   dif.GetSquareLength()<=YsSqr((objRad+gnd2Rad)))
			{
				if(gndCandidate[i]->MayCollideWith(gndCandidate[i]->GetInverseMatrix(),*selfPtr,objMat,clearance) &&
				   selfPtr->MayCollideWith(objMatInverse,*gndCandidate[i],gndCandidate[i]->GetMatrix(),clearance))
				{
					printf("Bbx may collide with %s\n",gndCandidate[i]->GetIdentifier());
					return YSTRUE;
				}
			}
		}
	}
	return YSFALSE;
}

void FsSimulation::UpdateGroundCarrierLoading(FsGround *gnd1,const YSSIZE_T nCarrierCandidate,FsGround *carrierCandidate[])
{
	if(YSTRUE==gnd1->Prop().IsOnCarrier() && NULL!=gnd1->Prop().OnThisCarrier())
	{
		FsAircraftCarrierProperty *const carrierProp=gnd1->Prop().OnThisCarrier()->Prop().GetAircraftCarrierProperty();
		if(carrierProp!=NULL)
		{
			YsVec3 deckNom;
			const YsVec3 &gndPos=gnd1->GetPosition();
			if(carrierProp->IsOnDeck(gndPos)!=YSTRUE ||
			   carrierProp->GetDeckHeightAndNormal(deckNom,gndPos)+1.0<gndPos.y())  // Avoid catapult in the air
			{
				carrierProp->UnloadGround(gnd1);
			}
		}
	}
	if(0<nCarrierCandidate)
	{
		gnd1->ProcessLoadingOnAircraftCarrier(nCarrierCandidate,carrierCandidate);
	}
}

void FsSimulation::KillCallBack(FsExistence &obj,const YsVec3 &pos)
{
	const double &rad=obj.GetApproximatedCollideRadius();
	if(obj.IsAirplane()==YSTRUE)
	{
		explosionHolder.Explode(currentTime,pos,10.0,1.0,rad+15.0,YSTRUE,NULL,YSTRUE);
	}
	else
	{
		if(cfgPtr->useParticle==YSTRUE)
		{
			explosionHolder.Explode(currentTime,pos,1.0,1.0,rad+15.0,YSTRUE,NULL,YSTRUE);
			auto partGenPtr=particleStore.CreateGenerator(FSPARTICLEGENERATOR_BURN,pos,YsYVec(),10.0,pos.y());
			partGenPtr->SetSize(3.0,20.0,3.0);
		}
		else
		{
			explosionHolder.Explode(currentTime,pos,10.0,1.0,rad+15.0,YSTRUE,NULL,YSTRUE);
		}
	}
}

void FsSimulation::SimCheckEndOfSimulation(void)
{
	if(endTime<YsTolerance)
	{
		// Player Plane is controlled by player & it's dead.
		const FsExistence *playerObj=GetPlayerObject();
		if(NULL!=playerObj &&
		   YSTRUE==playerObj->IsActive())
		{
		}
		else if(playerObj!=NULL &&
		        playerObj->isPlayingRecord!=YSTRUE &&
		        playerObj->IsActive()!=YSTRUE)
		{
			endTime=currentTime+3.0;
		}
		else if(GetNumAirplane()==0)
		{
			endTime=currentTime;
		}
	}
}

void FsSimulation::SimCheckEndOfFlightRecord(void)
{
	if(endTime<YsTolerance)
	{
		double lastRecordTime;
		if(AllRecordedFlightsAreOver(lastRecordTime)==YSTRUE)
		{
			currentTime=lastRecordTime;
			endTime=currentTime;  // Stop Right Away
		}
	}
}

void FsSimulation::SimControlByUser(const double &dt,FSUSERCONTROL userControl)
{
	int mx,my;
	YSBOOL lb,mb,rb;

	while(FSMOUSEEVENT_MOVE==FsGetMouseEvent(lb,mb,rb,mx,my))
	{
		// Skip all mouse-move events.
	}

	auto inFltDlg=GetCurrentInFlightDialog();

	if(NULL!=inFltDlg)
	{
		// 2014/11/02 It was blocking other controls if inFltDlg->SetMouseState returned YSTRUE.
		//            However, it was blocking some key strokes that were not supposed to be taken by this.
		//            Therefore, it is no longer blocking.  The question is if it causes something bad.
		inFltDlg->SetMouseState(lb,mb,rb,mx,my);
	}

	if(NULL!=inFltDlg && inFltDlg==loadingDlg)
	{
		SimProcessLoadingDialog(lb,mb,rb,mx,my);
	}
	// else if(NULL!=inFltDlg && YSTRUE==inFltDlg->IsMouseOnDialog(mx,my))
	// {
	//     2014/11/02 Commented out to prevent key strokes pile up in the key buffer while the mouse cursor is
	//     on an in-flight dialog.  The question is if there are any bad thing to ignore IsMouseOnDialog.
	// }
	else
	{
		int rawKey;
		int chr;

		FsExistence *playerObj=GetPlayerObject();
		// FsGround *playerGround=GetPlayerGround();
		FsAirplane *playerPlane=GetPlayerAirplane();

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C1\n");
#endif

		if(playerObj!=NULL)
		{
			playerObj->LockOn(this,cfgPtr->radarAltitudeLimit);
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C2\n");
#endif

		if(userControl==FSUSC_DISABLE)
		{
			return;
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C3\n");
#endif

		if(NULL!=replayDlg && YSTRUE==replayDlg->IsTakingNumberKey())
		{
			ctlAssign.processNumberKey=YSFALSE;
		}
		else if(NULL!=inFltDlg && YSTRUE==inFltDlg->WouldProcessNumberKey())
		{
			ctlAssign.processNumberKey=YSFALSE;
		}
		else if(FSSUBMENU_NONE==subMenu.GetSubMenu() || 
		        (FSSUBMENU_SELECTVOR==subMenu.GetSubMenu() && 200==userInput.Nav()))
		{
			ctlAssign.processNumberKey=YSTRUE;
		}
		else
		{
			ctlAssign.processNumberKey=YSFALSE;
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C4\n");
#endif

		FsJoystick joy[FsMaxNumJoystick];
		for(int i=0; i<FsMaxNumJoystick; i++)
		{
			FsPollJoystick(joy[i],i);
		}
		// 2014/11/27 Prevent mouse button firing a weapon when the cursor is on a in-flight dialog.
		if(NULL!=inFltDlg && YSTRUE==inFltDlg->IsMouseOnDialog(mx,my))
		{
			for(auto &trg : joy[FsMouseJoyId].trg)
			{
				trg=YSFALSE;
			}
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C5\n");
#endif

		if(showReplayDlg==YSTRUE && replayDlg!=NULL)
		{
			replayDlg->SetMouseState(lb,mb,rb,mx,my);
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C6\n");
#endif

		subMenu.CheckKeyRelease();

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C7\n");
#endif

		if(inFltDlg!=NULL && inFltDlg==chatDlg)
		{
			SimProcessChatMode(lb,mb,rb,mx,my);
		}
		else
		{
			while((rawKey=FsInkey())!=FSKEY_NULL)
			{
				if(NULL!=inFltDlg && YSTRUE==inFltDlg->ProcessRawKeyInput(rawKey))
				{
					SetIgnoreThisKeyHolding(rawKey); // Don't take this key as button holding until it is released again.
					continue;
				}
				YSBOOL keyTaken=YSFALSE;
				for(auto idx : addOnList.ReverseIndex())
				{
					auto addOnPtr=addOnList[idx];
					if(YSOK==addOnPtr->OnKeyDown(this,rawKey))
					{
						keyTaken=YSTRUE;
						break;
					}
				}
				if(YSTRUE==keyTaken)
				{
					continue;
				}

				const FSSUBMENU curSubMenu=subMenu.GetSubMenu();

				SimProcessRawKey(rawKey);  // Does nothing unless in the replay mode.
				SimProcessSubMenu(rawKey); // Sub menus

				// curSubMenu needs to be cached before SimProcessSubMenu because sub menu may change in SimProcessSubMenu.
				if(YSTRUE!=subMenu.SubMenuEatRawKey(curSubMenu,rawKey))
				{
					const FSBUTTONFUNCTION fnc=ctlAssign.TranslateKeyStroke(rawKey);
					SimProcessButtonFunction(fnc,userControl);
				}
			}
			while((chr=FsInkeyChar())!=0)
			{
				if(showReplayDlg==YSTRUE && replayDlg!=NULL)
				{
					replayDlg->CharIn(chr);
				}
			}
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C8\n");
#endif

		SimProcessJoystickTrigger(joy,userControl);

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C9\n");
#endif

		if(FSSUBMENU_INFLTCONFIG==subMenu.GetSubMenu() ||  // 2005/03/31
		  inFltDlg==chatDlg)                               // 2010/07/22
		{                                                  // 2005/03/31
			ctlAssign.checkKeyHolding=YSFALSE;             // 2005/03/31
		}                                                  // 2005/03/31
		else                                               // 2005/03/31
		{                                                  // 2005/03/31
			ctlAssign.checkKeyHolding=YSTRUE;              // 2005/03/31
		}                                                  // 2005/03/31
		ctlAssign.CheckIgnoredKeyRelease();
		userInput.ReadControl(ctlAssign,pJoy,joy);
		userInput.Move(ctlAssign,joy,dt);
		ctlAssign.checkKeyHolding=YSTRUE;                  // 2005/03/31

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C10\n");
#endif

		// 2010/02/12 Control View Attitude by Hat>>
		if(mainWindowViewmode==FSOUTSIDEPLAYER2 || mainWindowViewmode==FSOUTSIDEPLAYER3)
		{
			double dh,dp;
			switch(userInput.pov)
			{
			default:
				dh=0.0;
				dp=0.0;
				break;
			case 1:
				dh=0.0;
				dp=-YsPi/3.0;
				break;
			case 3:
				dh=-YsPi/3.0;
				dp=0.0;
				break;
			case 5:
				dh=0.0;
				dp=YsPi/3.0;
				break;
			case 7:
				dh=YsPi/3.0;
				dp=0.0;
				break;
			}

			if(0!=userInput.pov)
			{
				double newH=relViewAtt.h()+dh*dt;
				double newP=YsBound(relViewAtt.p()+dp*dt,-YsPi/2.0,YsPi/2.0);
				relViewAtt.SetH(newH);
				relViewAtt.SetP(newP);
				relViewAtt.SetB(0.0);
			}
		}
		// 2010/02/12 <<

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C11\n");
#endif

		/* if(ctlAssign.IsButtonPressed(FSBTF_OUTSIDEPLAYERVIEW2,joy)==YSTRUE ||
		   ctlAssign.IsButtonPressed(FSBTF_OUTSIDEPLAYERVIEW3,joy)==YSTRUE)
		{
			if(userInput.ctlFireWeaponButton!=YSTRUE && userInput.ctlCycleWeaponButton!=YSTRUE)
			{
				relViewAtt.NoseUp((YsPi/2.0)*userInput.ctlElevator*dt);
				relViewAtt.YawLeft((YsPi/2.0)*userInput.ctlAileron*dt);
			}
			else if(userInput.ctlFireWeaponButton==YSTRUE && userInput.ctlCycleWeaponButton==YSTRUE)
			{
				relViewAtt.Set(0.0,-YsPi/9.0,0.0);
				relViewDist=2.0;
			}
			else if(userInput.ctlFireWeaponButton==YSTRUE)
			{
				relViewDist=YsBound(relViewDist+userInput.ctlElevator*dt,0.5,10.0);
			}
			else if(userInput.ctlCycleWeaponButton==YSTRUE)
			{
				relViewAtt.SetB(relViewAtt.b()+(YsPi/2.0)*userInput.ctlAileron*dt);
			}

			if(ctlAssign.IsButtonPressed(FSBTF_RADAR,joy)==YSTRUE)
			{
				relViewAtt.SetB(0.0);
			}
		}
		else*/ if(mainWindowViewmode==FSGHOSTVIEW)
		{
			SimProcessGhostView(dt);
		}
		else if(NULL!=playerObj)
		{
			playerObj->ApplyControlAndGetFeedback(userInput,userControl,cfgPtr->autoCoordination);
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C12\n");
#endif

		if(playerPlane!=NULL)
		{
			if(ctlAssign.IsButtonPressed(FSBTF_ROTATEVORRIGHT,joy)==YSTRUE ||
			   ctlAssign.IsButtonPressed(FSBTF_ROTATEVORLEFT,joy)==YSTRUE)
			{
				if(ctlAssign.IsButtonPressed(FSBTF_ROTATEVORRIGHT,joy)==YSTRUE)
				{
					if(200==userInput.Nav()) // Heading Bug
					{
						double hdg=playerPlane->Prop().GetHeadingBug()-dt*YsPi/9.0;
						if(hdg<0.0)
						{
							hdg+=YsPi*2.0;
						}
						playerPlane->Prop().SetHeadingBug(hdg);
					}
					if(100==userInput.Nav()) // ADF
					{
						// ... I haven't added OBS for ADF.  Haven't I.
					}
					else
					{
						double hdg=playerPlane->Prop().GetVorObs(userInput.Nav())-dt*YsPi/9.0;
						if(hdg<0.0)
						{
							hdg+=YsPi*2.0;
						}
						playerPlane->Prop().SetVorObs(userInput.Nav(),hdg);
					}
				}
				else // if(ctlAssign.IsButtonPressed(FSBTF_ROTATEVORLEFT,joy)==YSTRUE)
				{
					if(200==userInput.Nav()) // Heading Bug
					{
						double hdg=playerPlane->Prop().GetHeadingBug()+dt*YsPi/9.0;
						if(hdg>YsPi*2.0)
						{
							hdg-=YsPi*2.0;
						}
						playerPlane->Prop().SetHeadingBug(hdg);
					}
					if(100==userInput.Nav()) // ADF
					{
						// ... I haven't added OBS for ADF.  Haven't I.
					}
					else
					{
						double hdg=playerPlane->Prop().GetVorObs(userInput.Nav())+dt*YsPi/9.0;
						if(hdg>YsPi*2.0)
						{
							hdg-=YsPi*2.0;
						}
						playerPlane->Prop().SetVorObs(userInput.Nav(),hdg);
					}
				}
			}
			if(YSTRUE==ctlAssign.IsButtonPressed(FSBTF_TRIMUP,joy))
			{
				userInput.ctlElvTrim=YsSmaller(userInput.ctlElvTrim+0.025*dt, 1.0);
			}
			if(YSTRUE==ctlAssign.IsButtonPressed(FSBTF_TRIMDOWN,joy))
			{
				userInput.ctlElvTrim=YsSmaller(userInput.ctlElvTrim-0.025*dt, 1.0);
			}
			if(YSTRUE==ctlAssign.IsButtonPressed(FSBTF_TURRETLEFT,joy))
			{
				userInput.ctlTurretHdg+=0.2*dt;
				if(userInput.ctlTurretHdg>=1.0)
				{
					userInput.ctlTurretHdg=1.0;
				}
			}
			if(YSTRUE==ctlAssign.IsButtonPressed(FSBTF_TURRETRIGHT,joy))
			{
				userInput.ctlTurretHdg-=0.2*dt;
				if(userInput.ctlTurretHdg<=-1.0)
				{
					userInput.ctlTurretHdg=-1.0;
				}
			}
			if(YSTRUE==ctlAssign.IsButtonPressed(FSBTF_TURRETUP,joy))
			{
				userInput.ctlTurretPch+=0.2*dt;
				if(userInput.ctlTurretPch>=1.0)
				{
					userInput.ctlTurretPch=1.0;
				}
			}
			if(YSTRUE==ctlAssign.IsButtonPressed(FSBTF_TURRETDOWN,joy))
			{
				userInput.ctlTurretPch-=0.2*dt;
				if(userInput.ctlTurretPch<=-1.0)
				{
					userInput.ctlTurretPch=-1.0;
				}
			}
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C13\n");
#endif

		for(int i=0; i<FsMaxNumJoystick; i++)
		{
			pJoy[i]=joy[i];
		}

		ctlAssign.processNumberKey=YSTRUE;
	}
#ifdef CRASHINVESTIGATION_SIMCONTROLBYUSER
		printf("C14\n");
#endif
}

void FsSimulation::SimPlayerAircraftGetTrouble(void)
{
	FsAirplane *playerAir=GetPlayerAirplane();
	if(NULL!=playerAir && aircraftTroubleTimer+10.0<currentTime)
	{
		aircraftTroubleTimer=currentTime+10.0;

		const double luck=(double)rand()/(double)RAND_MAX;

		// Probability cfgPtr->frequency times per 3600 sec.
		//             cfgPtr->frequency/360 times per 10 sec.
		const double probability=cfgPtr->aircraftTroubleFrequency/360.0;
		// printf("Luck %lf Trouble Probability %lf\n",luck,probability);
		if(luck<probability)
		{
			SendAircraftRandomTrouble(*playerAir,cfgPtr->aircraftReliability,YSTRUE);
		}
	}
}

void FsSimulation::SendAircraftRandomTrouble(FsAirplane &air,int reliability,YSBOOL record)
{
	// Reliability
	//   100   Perfectly reliable
	//    90   Landing gear lock eventually comes off
	//    80   Radar malfunction
	//    70   Loss of navigation equipment
	//    60   Airspeed indicator malfunction
	//    50   Altimeter and VSI malfunction
	//    40   Attitude indicator malfunction
	//    30   Partial power loss (engine up to 70%)
	//    20   Partial power loss (engine up to 50%)
	//    10   Partial power loss (envine up to 30%)
	//     0   Total power loss

	YsArray <FSAIRCRAFTTROUBLE,16> troubleArray;
	if(100>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_LOSSGEARLOCK);
	}
	if(90>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_AUTOPILOT);
		troubleArray.Append(FSAIRTROUBLE_FLAPSTUCK);
		troubleArray.Append(FSAIRTROUBLE_RADAR);
	}
	if(80>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_ALTIMETER);
		troubleArray.Append(FSAIRTROUBLE_VSI);
	}
	if(70>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_VOR);
		troubleArray.Append(FSAIRTROUBLE_ADF);
	}
	if(60>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_AIRSPEED);
	}
	if(50>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_ATTITUDE);
	}
	if(40>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_HUDFLICKER);
	}
	if(30>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_PARTIALPOWERLOSS_70);
	}
	if(20>reliability)
	{
		troubleArray.Append(FSAIRTROUBLE_PARTIALPOWERLOSS_30);
		troubleArray.Append(FSAIRTROUBLE_TOTALPOWERLOSS);
	}

	if(0<troubleArray.GetN())
	{
		YSSIZE_T idx=rand()%troubleArray.GetN();
		SendAircraftTrouble(air,troubleArray[idx],record);
	}
}

void FsSimulation::SendAircraftTrouble(FsAirplane &air,FSAIRCRAFTTROUBLE trouble,YSBOOL record)
{
	// Memo: Add event log.

	YsString troubleCmd;

	switch(trouble)
	{
	case FSAIRTROUBLE_LOSSGEARLOCK:
		printf("Landing Gear Lock\n");
		if(FSGROUNDSTATIC!=air.Prop().GetFlightState() &&
		   FSGROUND!=air.Prop().GetFlightState() &&
		   YsUnitConv::FTtoM(200.0)<air.GetAGL())
		{
			if(0.8<userInput.ctlGear)
			{
				userInput.ctlGear=0.6;
				hideNextGearSound=YSTRUE;
			}
			else
			{
				userInput.ctlGearTrouble=YSTRUE;
			}
		}
		else
		{
			userInput.ctlGearTrouble=YSTRUE;
		}
		break;
	case FSAIRTROUBLE_AUTOPILOT:
		printf("Auto pilot failure.\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		air.SetAutopilot(NULL);
		break;
	case FSAIRTROUBLE_FLAPSTUCK:
		printf("Flaps inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_RADAR:
		printf("Radar inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_VOR:
		printf("VOR inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_ADF:
		printf("ADF inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_AIRSPEED:
		printf("Airspeed inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,air.GetPosition().y(),air.Prop().GetVelocity(),0.0);
		break;
	case FSAIRTROUBLE_ALTIMETER:
		printf("Altimeter inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,air.GetPosition().y(),0.0,0.0);
		break;
	case FSAIRTROUBLE_VSI:
		printf("VSI inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_ATTITUDE:
		printf("Attitude indicator inop\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,
		   air.GetAttitude().h(),
		   air.GetAttitude().p(),
		   air.GetAttitude().b());
		break;
	case FSAIRTROUBLE_HUDFLICKER:
		printf("HUD flicker\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_PARTIALPOWERLOSS_70:
		printf("Partial power loss 70\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_PARTIALPOWERLOSS_30:
		printf("Partial power loss 30\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	case FSAIRTROUBLE_TOTALPOWERLOSS:
		printf("Total power loss\n");
		air.Prop().MakeUpAircraftTroubleCommand(troubleCmd,trouble,0.0,0.0,0.0);
		break;
	}

	if(0<troubleCmd.Strlen())
	{
		air.Prop().SendCommand(troubleCmd);

		if(YSTRUE==record)
		{
			FsSimulationEvent evt;

			evt.Initialize();
			evt.eventType=FSEVENT_AIRCOMMAND;
			evt.objKey=air.SearchKey();
			evt.str=troubleCmd;
			simEvent->AddEvent(currentTime,evt);
		}
	}
}

void FsSimulation::ClearAircraftTrouble(FsAirplane &air,FSAIRCRAFTTROUBLE trouble,YSBOOL record)
{
	YsString troubleCmd;
	air.Prop().MakeUpClearAircraftTroubleCommand(troubleCmd,trouble);
	if(0<troubleCmd.Strlen())
	{
		air.Prop().SendCommand(troubleCmd);

		if(YSTRUE==record)
		{
			FsSimulationEvent evt;

			evt.Initialize();
			evt.eventType=FSEVENT_AIRCOMMAND;
			evt.objKey=air.SearchKey();
			evt.str=troubleCmd;
			simEvent->AddEvent(currentTime,evt);
		}
	}
}

void FsSimulation::SimProcessJoystickTrigger(const FsJoystick joy[],FSUSERCONTROL userControl)
{
	for(int i=0; i<FsMaxNumJoystick; i++)
	{
		for(int j=0; j<FsMaxNumJoyTrig; j++)
		{
			if(pJoy[i].trg[j]!=YSTRUE && joy[i].trg[j]==YSTRUE)
			{
				FSBUTTONFUNCTION fnc;
				fnc=ctlAssign.TranslateTrigger(i,j);
				SimProcessButtonFunction(fnc,userControl);
			}
		}
	}
}

void FsSimulation::SimProcessRawKey(int rawKey)
{
	// Here, replayMode is updated. But, it is not refered
	// unless the simulation is running by RunReplaySimulation()

	if(showReplayDlg==YSTRUE && replayDlg!=NULL)
	{
		replayDlg->KeyIn(rawKey,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
	}

	switch(rawKey)
	{
	case FSKEY_ENTER:
		if(replayDlg!=NULL)
		{
			showReplayDlg=YSTRUE;
		}
		break;
	case FSKEY_Z:
		replayMode=FSREPLAY_VERYFASTREWIND;
		break;
	case FSKEY_X:
		replayMode=FSREPLAY_FASTREWIND;
		break;
	case FSKEY_C:
		replayMode=FSREPLAY_PLAY;
		break;
	case FSKEY_V:
		replayMode=FSREPLAY_FASTFORWARD;
		break;
	case FSKEY_B:
		replayMode=FSREPLAY_VERYFASTFORWARD;
		break;
	case FSKEY_D:
		replayMode=FSREPLAY_PAUSE;
		break;
	case FSKEY_HOME:
		if(EveryAirplaneIsRecordedAirplane()==YSTRUE)
		{
			double t0,prevT;
			t0=GetFirstRecordTime();
			prevT=YsGreater(0.0,t0-30.0);

			currentTime=prevT;
			FastForward(t0);
			RefreshOrdinanceByWeaponRecord(prevT);
			replayMode=FSREPLAY_PLAY;
		}
		break;
	case FSKEY_END:
		if(EveryAirplaneIsRecordedAirplane()==YSTRUE)
		{
			double t0,prevT;
			t0=GetLastRecordTime();
			prevT=t0-30.0;
			FastForward(t0);
			RefreshOrdinanceByWeaponRecord(prevT);
			replayMode=FSREPLAY_PLAY;
		}
		break;
	}
	// Here timeMarker is updated, but it does not affect
	// anything while flying.
	if(FSKEY_0<=rawKey && rawKey<=FSKEY_9)
	{
		const int i=rawKey-FSKEY_0;
		timeMarker[i]=currentTime;
	}
	if(rawKey==FSKEY_ESC)
	{
		if(subMenu.GetSubMenu()!=FSSUBMENU_NONE)
		{
			escKeyCount=0;
			subMenu.SetSubMenu(this,FSSUBMENU_NONE);
		}
		else
		{
			escKeyCount++;
			if(escKeyCount>=2)
			{
				SetTerminate(YSTRUE);
			}
		}
	}
	else
	{
		escKeyCount=0;
	}



	// >> Test key
	switch(rawKey)
	{
	case FSKEY_NUMLOCK:
		break;
	case FSKEY_SLASH:
		break;
	case FSKEY_TILDA:
		break;
	case FSKEY_BACKSLASH:
		break;
	case FSKEY_LBRACKET:
		break;
	case FSKEY_RBRACKET:
		break;
	}
	// << Test key
}

void FsSimulation::SimProcessChatMode(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my)
{
	chatDlg->SetMouseState(lb,mb,rb,mx,my);

	int rawKey;
	while(FSKEY_NULL!=(rawKey=FsInkey()))
	{
		chatDlg->KeyIn(rawKey,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));

		FSBUTTONFUNCTION fnc;
		fnc=ctlAssign.TranslateKeyStroke(rawKey);
		if(FSBTF_INFLTMESSAGE==fnc)
		{
			SetCurrentInFlightDialog(NULL);
		}
	}

	int chr;
	while(0!=(chr=FsInkeyChar()))
	{
		chatDlg->CharIn(chr);
	}
}

void FsSimulation::SimProcessGhostView(const double dt)
{
	auto &viewPoint=mainWindowActualViewMode.viewPoint;
	auto &viewAttitude=mainWindowActualViewMode.viewAttitude;

	const double accel=120.0;
	double vp,vh,vb,vy,desigSpd;
	vp=userInput.ctlElevator*(YsPi/2.0)*dt;
	vb=userInput.ctlAileron*(YsPi)*dt;
	vh=(sin(viewAttitude.b())/5.0*dt)*YsAbs(cos(viewAttitude.p()));
	vy=userInput.ctlRudder*dt;


#if 1
	{
		viewAttitude.SetP(YsBound(viewAttitude.p()+vp,-YsPi/2.5,YsPi/2.5));
		viewAttitude.SetH(viewAttitude.h()+vb);  // vb~=Aileron.  Let aileron control heading.
		if(viewAttitude.p()<-YsPi/2.5)
		{
			viewAttitude.SetP(-YsPi/2.5);
		}
		else if(viewAttitude.p()>YsPi/2.5)
		{
			viewAttitude.SetP(YsPi/2.5);
		}
		viewAttitude.SetB(viewAttitude.b()/2.0);

		if(userInput.ctlFireWeaponButton==YSTRUE)
		{
			desigSpd=280.0;
		}
		else if(userInput.ctlCycleWeaponButton==YSTRUE)
		{
			desigSpd=-280.0;
		}
		else
		{
			desigSpd=0.0;
		}
	}
#else
	{
		viewAttitude.NoseUp(vp);
		viewAttitude.SetH(viewAttitude.h()+vh);
		viewAttitude.SetB(viewAttitude.b()+vb);
		viewAttitude.YawLeft(vy);

		// 1200m/s in 10secs -> 120m/ss
		desigSpd=userInput.ctlThrottle*300.0;
	}
#endif

	if(ghostViewSpeed<desigSpd)
	{
		ghostViewSpeed+=accel*dt;
		if(ghostViewSpeed>desigSpd)
		{
			ghostViewSpeed=desigSpd;
		}
	}
	else
	{
		ghostViewSpeed-=accel*dt;
		if(ghostViewSpeed<desigSpd)
		{
			ghostViewSpeed=desigSpd;
		}
	}

	YsVec3 displacement;
	displacement=viewAttitude.GetForwardVector()*ghostViewSpeed*dt;
	viewPoint+=displacement;

	double elv;
	elv=GetFieldElevation(viewPoint.x(),viewPoint.z());
	if(viewPoint.y()<elv+10.0)
	{
		viewPoint.SetY(elv+10.0);
	}
}

void FsSimulation::SimProcessLoadingDialog(YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my)
{
	loadingDlg->SetMouseState(lb,mb,rb,mx,my);

	int rawKey;
	while((rawKey=FsInkey())!=FSKEY_NULL)
	{
		loadingDlg->KeyIn(rawKey,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
	}

	FsGuiDialogItem *clickedItem=loadingDlg->GetClickedItem();
	if(clickedItem==loadingDlg->okBtn)
	{
		YSBOOL fuel,ammo;
		FsAirplane *playerPlane=GetPlayerAirplane();
		loadingDlg->CaptureSelection();
		if(playerPlane!=NULL && NULL!=FindNearbySupplyTruck(fuel,ammo,*playerPlane))
		{
			FsSimulationEvent evt;
			const FsAirplaneTemplate *tmpl;
			char str[256];

			if(ammo==YSTRUE)
			{
				YsArray <int,64> loading;

				loading=loadingDlg->selWeaponConfig;
				if(allowedWeaponType & FSWEAPON_ALLOWGUN)
				{
					loading.Append(FSWEAPON_GUN);
					loading.Append(playerPlane->Prop().GetMaxNumWeapon(FSWEAPON_GUN));

					for(int i=0; i<playerPlane->Prop().GetNumTurret(); i++)
					{
						int max;
						char cmd[256];
						max=playerPlane->Prop().GetMaxNumTurretBullet(i);
						sprintf(cmd,"TURRETAM %d %d",i,max);

						playerPlane->Prop().SendCommand(cmd);

						evt.Initialize();
						evt.eventType=FSEVENT_AIRCOMMAND;
						evt.objKey=FsExistence::GetSearchKey(playerPlane);
						evt.str.Set(cmd);
						simEvent->AddEvent(currentTime,evt);
					}
				}
				loading.Append(FSWEAPON_FLARE_INTERNAL);
				loading.Append(playerPlane->Prop().GetMaxNumWeapon(FSWEAPON_FLARE_INTERNAL));

				playerPlane->Prop().ApplyWeaponConfig(loading.GetN(),loading);

				evt.Initialize();
				evt.eventType=FSEVENT_SETWEAPONCONFIG;
				evt.objKey=FsExistence::GetSearchKey(playerPlane);
				evt.weaponCfg.Set(loading.GetN(),loading);
				simEvent->AddEvent(currentTime,evt);

				if(playerPlane->netType==FSNET_LOCAL)
				{
					if(netServer!=NULL)
					{
						netServer->BroadcastWeaponConfig(FsExistence::GetSearchKey(playerPlane),loading.GetN(),loading);
					}
					if(netClient!=NULL)
					{
						netClient->SendWeaponConfig(FsExistence::GetSearchKey(playerPlane),loading.GetN(),loading);
					}
				}
			}
			else
			{
				if(loadingDlg->SmokeLoaded()==YSTRUE)
				{
					playerPlane->Prop().SendCommand("SMOKEOIL 100.0kg");

					evt.Initialize();
					evt.eventType=FSEVENT_AIRCOMMAND;
					evt.objKey=FsExistence::GetSearchKey(playerPlane);
					evt.str.Set("SMOKEOIL 100.0kg");
					simEvent->AddEvent(currentTime,evt);
				}
			}

			if(fuel==YSTRUE)
			{
				sprintf(str,"INITFUEL %d%%",loadingDlg->selFuel);

				playerPlane->Prop().SendCommand(str);

				evt.Initialize();
				evt.eventType=FSEVENT_AIRCOMMAND;
				evt.objKey=FsExistence::GetSearchKey(playerPlane);
				evt.str.Set(str);
				simEvent->AddEvent(currentTime,evt);
			}

			tmpl=world->GetAirplaneTemplate(playerPlane->Prop().GetIdentifier());
			if(tmpl!=NULL)
			{
				sprintf(str,"STRENGTH %d",tmpl->GetProperty()->GetDamageTolerance());

				playerPlane->Prop().SendCommand(str);

				evt.Initialize();
				evt.eventType=FSEVENT_AIRCOMMAND;
				evt.objKey=FsExistence::GetSearchKey(playerPlane);
				evt.str.Set(str);
				simEvent->AddEvent(currentTime,evt);
			}
		}

		SetCurrentInFlightDialog(NULL);
	}
	else if(clickedItem==loadingDlg->cancelBtn)
	{
		SetCurrentInFlightDialog(NULL);
	}
}

void FsSimulation::SimProcessSubMenu(int rawKey)
{
	subMenu.ProcessSubMenu(this,*cfgPtr,rawKey);
}

void FsSimulation::SimProcessButtonFunction(FSBUTTONFUNCTION fnc,FSUSERCONTROL userControl)
{
	FsExistence *playerObj=GetPlayerObject();
	FsAirplane *playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL && playerPlane->isPlayingRecord!=YSTRUE)
	{
		if(fnc==FSBTF_OPENAUTOPILOTMENU && (userControl==FSUSC_ENABLE || FSUSC_SCRIPT==userControl))
		{
			ToggleAutoPilotDialog();
		}
		else if(fnc==FSBTF_OPENRADIOCOMMMENU && (userControl==FSUSC_ENABLE || FSUSC_SCRIPT==userControl))
		{
			SetUpRadioCommTargetDialog();
		}
		else if(fnc==FSBTF_OPENVORMENU)
		{
			if(subMenu.GetSubMenu()==FSSUBMENU_SELECTVOR)
			{
				userInput.CycleNav();
				subMenu.SelectNav(userInput.Nav());
			}
			else
			{
				subMenu.SetSubMenu(this,FSSUBMENU_SELECTVOR);
			}
		}
		else if(fnc==FSBTF_OPENADFMENU)
		{
			subMenu.SetSubMenu(this,FSSUBMENU_SELECTVOR);
			userInput.SelectNav(100);
			subMenu.SelectNav(100);
		}
		else if(fnc==FSBTF_SUPPLYDIALOG)
		{
			if(playerPlane!=NULL && playerPlane->Prop().GetFlightState()==FSGROUNDSTATIC)
			{
				YSBOOL fuel,ammo;
				if(NULL!=FindNearbySupplyTruck(fuel,ammo,*playerPlane))
				{
					OpenLoadingDialog(fuel,ammo,*playerPlane);
				}
				else
				{
					AddTimedMessage("## YOU NEED TO BE CLOSE TO THE FUEL/SUPPLY VEHICLE ##");
				}
			}
			else
			{
				AddTimedMessage("## YOU NEED TO BE AT COMPLETE STOP FOR RE-FUELING ##");
			}
		}
	}

	FsGround *playerGround=GetPlayerGround();
	if(NULL!=playerGround && YSTRUE!=playerGround->isPlayingRecord)
	{
		switch(fnc)
		{
		case FSBTF_SELECTWEAPON:
			playerGround->Prop().CycleWeaponOfChoiceByUser();
			break;
		default:
			break;
		}
	}

	if(fnc==FSBTF_INFLTMESSAGE) // 2007/01/12 Don't need a player plane
	{
		if(chatDlg!=GetCurrentInFlightDialog())
		{
			OpenChatDialog();
		}
		else
		{
			SetCurrentInFlightDialog(NULL);
		}

		FsPollDevice();
		while(0!=FsInkeyChar())
		{
			FsPollDevice();
		}
		while(FSKEY_NULL!=FsInkey())
		{
			FsPollDevice();
		}
	}


	switch(fnc)
	{
	case FSBTF_OPENSUBWINDOWMENU:
		subMenu.SetSubMenu(this,FSSUBMENU_OPENSUBWINDOW);
		break;
	case FSBTF_INFLTCONFIG:
		subMenu.SetSubMenu(this,FSSUBMENU_INFLTCONFIG);
		break;
	case FSBTF_CHANGEHUDCOLOR:
		if(hud->hudCol.Ri()==100 && hud->hudCol.Gi()==255 && hud->hudCol.Bi()==100)
		{
			hud->hudCol.SetIntRGB(100,100,255);
		}
		else if(hud->hudCol.Ri()==100 && hud->hudCol.Gi()==100 && hud->hudCol.Bi()==255)
		{
			hud->hudCol.SetIntRGB(255,255,255);
		}
		else if(hud->hudCol.Ri()==255 && hud->hudCol.Gi()==255 && hud->hudCol.Bi()==255)
		{
			hud->hudCol.SetIntRGB(255,100,100);
		}
		else
		{
			hud->hudCol.SetIntRGB(100,255,100);
		}
		hud2->SetColor(hud->hudCol);
		break;
	case FSBTF_PAUSE:
		if(netServer==NULL && netClient==NULL && userControl!=FSUSC_DISABLE)
		{
			YsFlip(pause);
		}
		else if(EveryAirplaneIsRecordedAirplane()==YSTRUE)  // Playing record.  Do nothing.
		{
		}
		else
		{
			AddTimedMessage("You cannot pause in the network mode.");
		}
		break;
	case FSBTF_SENSITIVITYUP:
		userInput.SensitivityUp();
		break;
	case FSBTF_SENSITIVITYDOWN:
		userInput.SensitivityDown();
		break;
	case FSBTF_CYCLESENSITIVITY:
		userInput.CycleSensitivity();
		break;


	case FSBTF_FIREWEAPON:                    //  Fire Selected Weapon
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlFireWeaponButtonExt=YSTRUE;
		}
		break;
	case FSBTF_FIREGUN:                       //  Fire Machine Gun
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlFireGunButtonExt=YSTRUE;
		}
		break;
	case FSBTF_FIREAAM:                       //  Fire AAM
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlFireAAMButtonExt=YSTRUE;
		}
		break;
	case FSBTF_FIREAGM:                       //  Fire AAM
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlFireAGMButtonExt=YSTRUE;
		}
		break;
	case FSBTF_FIREROCKET:                    //  Fire Rocket
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlFireRocketButtonExt=YSTRUE;
		}
		break;
	case FSBTF_DROPBOMB:                      //  Drop Bomb
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlDropBombButtonExt=YSTRUE;
		}
		break;
	case FSBTF_DISPENSEFLARE:                 //  Dispense Flare
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlDispenseFlareButtonExt=YSTRUE;
		}
		break;
	case FSBTF_SELECTWEAPON:                  //  Select Weapon
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlCycleWeaponButtonExt=YSTRUE;
		}
		break;
	case FSBTF_BRAKEHOLD:                     //  Brake On While Holding
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlSmokeButtonExt=YSTRUE;
		}
		break;
	case FSBTF_SMOKE:                         //  Smoke
		if(FSUSC_SCRIPT==userControl)
		{
			userInput.ctlCycleSmokeSelectorButtonExt=YSTRUE;
		}
		break;


	default:
		break;
	}

	userInput.ProcessButtonFunction(currentTime,playerObj,fnc);
	ViewingControl(fnc,userControl);
}

void FsSimulation::SimControlByComputer(const double &dt)
{
	YsArray <YsVec2i,256> blockList;
	YsArray <FsAirplane *,256> potentialAirTarget;
	YsArray <FsGround *,256> potentialGndTarget;

#ifdef CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	for(FsAirplane *air=NULL; NULL!=(air=FindNextAirplane(air)); )
	{
#ifdef CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
		printf("%s\n",(const char *)air->GetIdentifier());
#endif

		FsAutopilot *ap=air->GetAutopilot();
		if(air->IsAlive()==YSTRUE && ap!=NULL)
		{
#ifdef CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
			printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
			ap->Control(*air,this,dt);
#ifdef CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
			printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
			if(air->GetLandWhenLowFuel()>YsTolerance &&
			   ap->Type()!=FSAUTOPILOT_LANDING &&
			   ap->Type()!=FSAUTOPILOT_VLANDING)
			{
				double fuelThr;
				fuelThr=air->GetLandWhenLowFuel()*air->Prop().GetMaxFuelLoad();
				if(air->Prop().GetFuelLeft()<fuelThr)
				{
					if(air->Prop().GetAircraftClass()!=FSCL_HELICOPTER)
					{
						FsLandingAutopilot *ap;
						ap=FsLandingAutopilot::Create();
						air->SetAutopilot(ap);
					}
					else
					{
						FsVerticalLandingAutopilot *ap;
						ap=FsVerticalLandingAutopilot::Create();
						air->SetAutopilot(ap);
					}
				}
			}

			if(air->NextAutoPilotAvailable()==YSTRUE &&
			   ap->MissionAccomplished(*air,this)==YSTRUE)
			{
				air->MoveOnToNextAutoPilot();
			}
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
		printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		if(air->Prop().IsActive()==YSTRUE && air->Prop().GetHasGunnerControlledTurret()==YSTRUE)
		{
			if(air->netType==FSNET_LOCAL)
			{
				potentialAirTarget.Set(0,NULL);
				potentialGndTarget.Set(0,NULL);

				const double range=air->Prop().GetMaxRotatinTurretRange();

				if(air->Prop().GetHasAntiAirTurret()==YSTRUE)
				{
					GetLattice().GetAirCollisionCandidate(potentialAirTarget,air->GetPosition(),range);
				}
				if(air->Prop().GetHasAntiGroundTurret()==YSTRUE)
				{
					GetLattice().GetGndCollisionCandidate(potentialGndTarget,air->GetPosition(),range);
				}

				for(auto i=potentialGndTarget.GetN()-1; i>=0; i--)
				{
					if(YSTRUE==potentialGndTarget[i]->Prop().IsNonGameObject())
					{
						potentialGndTarget.DeleteBySwapping(i);
					}
				}

				air->Prop().MoveTurretGunner(
				    dt,
				    air->iff,
				    potentialAirTarget.GetN(),potentialAirTarget,
				    potentialGndTarget.GetN(),potentialGndTarget);
			}
		}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	}

#ifdef CRASHINVESTIGATION_SIMCONTROLBYCOMPUTER
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	for(FsGround *gnd=NULL; NULL!=(gnd=FindNextGround(gnd)); )
	{
		if(gnd->IsAlive()==YSTRUE)
		{
			if(GetPlayerGround()!=gnd)
			{
				gnd->DefaultControl(this,dt);
			}

			if(gnd->Prop().GetHasGunnerControlledTurret()==YSTRUE &&
			   gnd->netType==FSNET_LOCAL)
			{
				potentialAirTarget.Set(0,NULL);
				potentialGndTarget.Set(0,NULL);

				const double range=gnd->Prop().GetMaxRotatinTurretRange();

				if(gnd->Prop().GetHasAntiAirTurret()==YSTRUE)
				{
					GetLattice().GetAirCollisionCandidate(potentialAirTarget,gnd->GetPosition(),range);
				}
				if(gnd->Prop().GetHasAntiGroundTurret()==YSTRUE)
				{
					GetLattice().GetGndCollisionCandidate(potentialGndTarget,gnd->GetPosition(),range);
				}

				for(auto i=potentialGndTarget.GetN()-1; i>=0; i--)
				{
					if(YSTRUE==potentialGndTarget[i]->Prop().IsNonGameObject())
					{
						potentialGndTarget.DeleteBySwapping(i);
					}
				}

				gnd->Prop().MoveTurretGunner(
				    dt,
				    gnd->iff,
				    potentialAirTarget.GetN(),potentialAirTarget,
				    potentialGndTarget.GetN(),potentialGndTarget);
			}
		}
	}
}

void FsSimulation::SimMakeUpCockpitIndicationSet(class FsCockpitIndicationSet &cockpitIndicationSet) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	const FsGround *playerGround=GetPlayerGround();

	YsArray <int,64> loading;
	int samInterval=0;
	int maxNumGunBullet=0;
	FSWEAPONTYPE woc=FSWEAPON_NULL;

	cockpitIndicationSet.CleanUp();

	// Indicated Air Speed
	if(NULL!=playerPlane)
	{
		cockpitIndicationSet.inst.heading=InternalHeadingToMagnetic(playerPlane->Prop().GetAttitude().h());
		cockpitIndicationSet.inst.headingBug=InternalHeadingToMagnetic(playerPlane->Prop().GetHeadingBug());
		cockpitIndicationSet.inst.headingBugSelected=(200==userInput.Nav() ? YSTRUE : YSFALSE);
		cockpitIndicationSet.inst.pitch=playerPlane->Prop().GetIndicatedAttitude().p();
		cockpitIndicationSet.inst.bank=playerPlane->Prop().GetIndicatedAttitude().b();

		YsAtt3 rot;
		playerPlane->Prop().GetRotationSpeed(rot);

		cockpitIndicationSet.inst.turnRate=rot.h();
		cockpitIndicationSet.inst.sideSlip=playerPlane->Prop().GetSSA();
		cockpitIndicationSet.inst.altitude=playerPlane->Prop().GetIndicatedTrueAltitude();
		cockpitIndicationSet.inst.verticalSpeed=playerPlane->Prop().GetClimbRatioWithTimeDelay();

		cockpitIndicationSet.inst.airSpeed=0.0;
		if(YSTRUE==playerPlane->Prop().CheckAirspeedInop())
		{
			cockpitIndicationSet.inst.airSpeed=playerPlane->Prop().GetMalfunctioningAirspeedIndication();
		}
		else if(playerPlane->prevDt>YsTolerance)
		{
			YsVec3 vel=(playerPlane->GetPosition()-playerPlane->prevPos)/playerPlane->prevDt;
			vel-=GetWeather().GetWind();
			cockpitIndicationSet.inst.airSpeed=vel.GetLength();
		}
		else
		{
			cockpitIndicationSet.inst.airSpeed=playerPlane->Prop().GetVelocity();
		}

		cockpitIndicationSet.inst.Vfe=playerPlane->Prop().GetEstimatedLandingSpeed()*1.8;
		cockpitIndicationSet.inst.Vno=playerPlane->Prop().GetEstimatedCruiseSpeed()*1.4;
		cockpitIndicationSet.inst.Vne=playerPlane->Prop().GetEstimatedCruiseSpeed()*1.5;
		cockpitIndicationSet.inst.VindicatorRange=playerPlane->Prop().GetEstimatedCruiseSpeed()*1.7;

		if(YSTRUE==cfgPtr->showIAS)
		{
			const double rho=FsGetAirDensity(playerPlane->GetPosition().y()+GetBaseElevation());
			const double scale=sqrt(rho/1.225);
			cockpitIndicationSet.inst.airSpeed*=scale;
		}

		cockpitIndicationSet.inst.nEngine=1;
		cockpitIndicationSet.inst.engineOutput[0]=playerPlane->Prop().GetThrottle();
		cockpitIndicationSet.inst.afterBurner[0]=playerPlane->Prop().GetAfterBurner();

		cockpitIndicationSet.inst.nFuelTank=1;
		cockpitIndicationSet.inst.fuelRemain[0]=playerPlane->Prop().GetFuelLeft();
		cockpitIndicationSet.inst.fuelCapacity[0]=playerPlane->Prop().GetMaxFuelLoad();

		cockpitIndicationSet.inst.mach=playerPlane->Prop().GetMach();
		cockpitIndicationSet.inst.gForce=playerPlane->Prop().GetG();
		cockpitIndicationSet.inst.hasVectorThrust=playerPlane->Prop().GetHasThrustVectoring();
		playerPlane->Prop().GetThrustDirection(cockpitIndicationSet.inst.nozzleDirection);
		cockpitIndicationSet.inst.hasRetractableGear=playerPlane->Prop().HasRetractableLandingGear();
		cockpitIndicationSet.inst.gearPos=playerPlane->Prop().GetLandingGear();
		cockpitIndicationSet.inst.brake=(YSTRUE==playerPlane->Prop().GetBrake() ? 1.0 : 0.0);
		cockpitIndicationSet.inst.flaps=playerPlane->Prop().GetFlap();
		cockpitIndicationSet.inst.hasSpoiler=playerPlane->Prop().GetHasSpoiler();
		cockpitIndicationSet.inst.spoiler=playerPlane->Prop().GetSpoiler();
		cockpitIndicationSet.inst.autoPilot=(NULL!=playerPlane->GetAutopilot() ? YSTRUE : YSFALSE);

		cockpitIndicationSet.inst.elevator=playerPlane->Prop().GetElevator();
		cockpitIndicationSet.inst.elevatorTrim=playerPlane->Prop().GetElvTrim();
		cockpitIndicationSet.inst.aileron=playerPlane->Prop().GetAileron();
		cockpitIndicationSet.inst.rudder=playerPlane->Prop().GetRudderUserInput();

		YsVec3 vel;
		playerPlane->Prop().GetVelocity(vel);
		playerPlane->GetAttitude().MulInverse(vel,vel);
		cockpitIndicationSet.inst.velocity=vel;  // Relative to the aircraft coordinate



		for(int navId=0; navId<FsCockpitIndicationSet::NUM_NAV; ++navId)
		{
			cockpitIndicationSet.nav[navId].navId=navId;

			YSBOOL tuned,isIls,isDme;
			if(YSOK==GetVorIlsIndication(
			    navId,
			    cockpitIndicationSet.nav[navId].vorId,
			    tuned,
			    isIls,
			    cockpitIndicationSet.nav[navId].toFrom,
			    cockpitIndicationSet.nav[navId].obs,
			    cockpitIndicationSet.nav[navId].lateralDev,
			    cockpitIndicationSet.nav[navId].glideSlopeDev,
			    isDme,
			    cockpitIndicationSet.nav[navId].dme))
			{
				cockpitIndicationSet.nav[navId].SetInRange(YSTRUE);
			}
			cockpitIndicationSet.nav[navId].SetTuned(tuned);
			cockpitIndicationSet.nav[navId].SetIsILS(isIls);
			cockpitIndicationSet.nav[navId].SetIsDME(isDme);
			cockpitIndicationSet.nav[navId].SetSelected(userInput.Nav()==navId ? YSTRUE : YSFALSE);
			cockpitIndicationSet.nav[navId].SetInop(playerPlane->Prop().CheckVORInop());
		}

		for(int adfId=0; adfId<FsCockpitIndicationSet::NUM_ADF; ++adfId)
		{
			cockpitIndicationSet.adf[adfId].adfId=adfId;

			YSBOOL tuned;
			if(YSOK==GetAdfIndication(
				cockpitIndicationSet.adf[adfId].ndbId,
				tuned,
				cockpitIndicationSet.adf[adfId].bearing))
			{
				cockpitIndicationSet.adf[adfId].SetInRange(YSTRUE);
			}

			cockpitIndicationSet.adf[adfId].SetTuned(tuned);
			cockpitIndicationSet.adf[adfId].SetSelected(userInput.Nav()==100 ? YSTRUE : YSFALSE);
			cockpitIndicationSet.adf[adfId].SetInop(playerPlane->Prop().CheckADFInop());
		}

		playerPlane->Prop().GetWeaponConfig(loading);

		const int gun=playerPlane->Prop().GetNumWeapon(FSWEAPON_GUN)+playerPlane->Prop().GetNumPilotControlledTurretBullet();
		if(0<gun)
		{
			loading.Append(FSWEAPON_GUN);
			loading.Append(gun);
		}
		const double smokeOil=playerPlane->Prop().GetSmokeOil();
		if(0.0<smokeOil)
		{
			loading.Append(FSWEAPON_SMOKE);
			loading.Append((int)smokeOil);
		}

		maxNumGunBullet=playerPlane->Prop().GetMaxNumWeapon(FSWEAPON_GUN)+playerPlane->Prop().GetMaxNumPilotControlledTurretBullet();
		woc=playerPlane->Prop().GetWeaponOfChoice();
	}
	else if(NULL!=playerGround)
	{
		YsVec3 vel;
		playerGround->Prop().GetVelocity(vel);
		cockpitIndicationSet.inst.airSpeed=vel.GetLength();

		playerGround->Prop().GetWeaponConfig(loading);
		samInterval=(int)(1000.0*playerGround->Prop().GetTimeBeforeNextMissileCanBeShot(currentTime));
		woc=playerGround->Prop().GetWeaponOfChoice();
	}



	// AMMO

	int gun=0,aim9=0,aim9x=0,aim120=0,agm65=0,rocket=0,bomb500=0,bomb250=0,bomb500hd=0;
	double smokeOil=0.0;
	int extFuel=0,extFuelLeft=0;
	int flare=0;

	for(int i=0; i<=loading.GetN()-2; i+=2)
	{
		switch(loading[i])
		{
		case FSWEAPON_GUN:
			gun+=loading[i+1];
			break;
		case FSWEAPON_AIM9:
			aim9+=loading[i+1];
			break;
		case FSWEAPON_AIM9X:
			aim9x+=loading[i+1];
			break;
		case FSWEAPON_AGM65:
			agm65+=loading[i+1];
			break;
		case FSWEAPON_BOMB:
			bomb500+=loading[i+1];
			break;
		case FSWEAPON_ROCKET:
			rocket+=loading[i+1];
			break;
		case FSWEAPON_FLARE_INTERNAL:
		case FSWEAPON_FLARE:
			flare+=loading[i+1];
			break;
		case FSWEAPON_AIM120:
			aim120+=loading[i+1];
			break;
		case FSWEAPON_BOMB250:
			bomb250+=loading[i+1];
			break;
		case FSWEAPON_BOMB500HD:
			bomb500hd+=loading[i+1];
			break;
		case FSWEAPON_SMOKE:
			smokeOil+=(double)loading[i+1];
			break;
		case FSWEAPON_FUELTANK:
			extFuel++;
			extFuelLeft+=loading[i+1];
			break;
		}
	}

	unsigned int smokeSelector=0,smokeAvailable=0;
	if(YsTolerance<smokeOil && NULL!=playerPlane)
	{
		smokeAvailable=(1<<playerPlane->Prop().GetNumSmokeGenerator())-1;
		smokeSelector=playerPlane->Prop().GetSmokeSelector();
	}


	if(gun>0 || woc==FSWEAPON_GUN)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_GUN;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_GUN ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=gun;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=maxNumGunBullet;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}

	// AAM
	if(aim9>0 || woc==FSWEAPON_AIM9)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_AIM9;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_AIM9 ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=aim9;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=samInterval;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}
	if(aim9x>0 || woc==FSWEAPON_AIM9X)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_AIM9X;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_AIM9X ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=aim9x;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=samInterval;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}
	if(aim120>0 || woc==FSWEAPON_AIM120)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_AIM120;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_AIM120 ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=aim120;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=samInterval;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}

	// AGM
	if(agm65>0 || woc==FSWEAPON_AGM65)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_AGM65;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_AGM65 ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=agm65;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}

	// Rocket
	if(rocket>0 || woc==FSWEAPON_ROCKET)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_ROCKET;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_ROCKET ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=rocket;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}

	// Bomb
	if(bomb500>0 || woc==FSWEAPON_BOMB)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_BOMB;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_BOMB ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=bomb500;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}
	if(bomb250>0 || woc==FSWEAPON_BOMB250)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_BOMB250;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_BOMB250 ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=bomb250;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}
	if(bomb500hd>0 || woc==FSWEAPON_BOMB500HD)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_BOMB500HD;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_BOMB500HD ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=bomb500hd;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}

	// Flare
	if(flare>0 || woc==FSWEAPON_FLARE)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_FLARE;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_FLARE ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=flare;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=0;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}

	if(0<extFuel || woc==FSWEAPON_FUELTANK)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_FUELTANK;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_FUELTANK ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=extFuel;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().level=extFuelLeft;
		cockpitIndicationSet.ammo.ammoArray.Last().standByTimer=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=0;;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=0;;
	}
	// Smoke
	if(smokeOil>0.0 || woc==FSWEAPON_SMOKE)
	{
		cockpitIndicationSet.ammo.ammoArray.Increment();
		cockpitIndicationSet.ammo.ammoArray.Last().wpnType=FsAmmunitionIndication::WPNTYPE_SMOKE;
		cockpitIndicationSet.ammo.ammoArray.Last().selected=(woc==FSWEAPON_SMOKE ? YSTRUE : YSFALSE);
		cockpitIndicationSet.ammo.ammoArray.Last().quantity=1;
		cockpitIndicationSet.ammo.ammoArray.Last().maxQuantity=0;
		cockpitIndicationSet.ammo.ammoArray.Last().channel=smokeSelector;
		cockpitIndicationSet.ammo.ammoArray.Last().availableChannel=smokeAvailable;
	}
}

void FsSimulation::SimDrawAllScreen(YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker) const
{
	for(auto addOnPtr : addOnList)
	{
		if(YSOK==addOnPtr->OnDraw(this))
		{
			return;
		}
	}


	FsCockpitIndicationSet cockpitIndicationSet;
	SimMakeUpCockpitIndicationSet(cockpitIndicationSet);

#ifdef CRASHINVESTIGATION_S8_LEVEL2
	printf("S8-0\n");
#endif

	// FsSplitMainWindow(YSTRUE);  <- Test split.

	FsSelectMainWindow();
	if(YSTRUE!=FsIsMainWindowSplit())
	{
		SimDrawScreen(0,cockpitIndicationSet,demoMode,showTimer,showTimeMarker,mainWindowActualViewMode);
	}
	else
	{
		FsSetActiveSplitWindow(0);
		SimDrawScreen(0,cockpitIndicationSet,demoMode,showTimer,showTimeMarker,mainWindowActualViewMode);
		FsSetActiveSplitWindow(1);
		SimDrawScreen(0,cockpitIndicationSet,demoMode,showTimer,showTimeMarker,mainWindowActualViewMode);
	}

#ifdef CRASHINVESTIGATION_S8_LEVEL2
	printf("S8-1\n");
#endif

	int i;
	YSBOOL drewSubWindow;

#ifdef CRASHINVESTIGATION_S8_LEVEL2
	printf("S8-2\n");
#endif

	drewSubWindow=YSFALSE;
	for(i=0; i<FsMaxNumSubWindow; i++)
	{
		if(FsIsSubWindowOpen(i)==YSTRUE)
		{
			FsSelectSubWindow(i);
			SimDrawScreen(0,cockpitIndicationSet,demoMode,YSFALSE,YSFALSE,subWindowActualViewMode[i]);
			drewSubWindow=YSTRUE;
		}
	}

#ifdef CRASHINVESTIGATION_S8_LEVEL2
	printf("S8-3\n");
#endif

	if(drewSubWindow==YSTRUE)
	{
		FsSelectMainWindow();
	}

	SimDrawGuiDialog();

	SimDrawFlush(); // <- Swap buffers inside.

#ifdef CRASHINVESTIGATION_S8_LEVEL2
	printf("S8-4\n");
#endif
}

void FsSimulation::SimDrawScreen(
    const double &dt,const FsCockpitIndicationSet &cockpitIndicationSet,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker,const ActualViewMode &actualViewMode) const
{
#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-1\n");
#endif
	FsProjection prj;
	GetProjection(prj,actualViewMode);


	// printf("%s\n",ViewmodeToStr(actualViewMode.actualViewMode));


	YsGLParticleManager partMan;
	{
		particleStore.AddToParticleManager(partMan);
		if(YSTRUE==cfgPtr->useParticle)
		{
			solidCloud->AddToParticleManager(partMan,env,*weather,actualViewMode.viewAttitude.GetForwardVector(),actualViewMode.viewMat,prj.nearz,prj.farz,prj.tanFov);
			bulletHolder.AddToParticleManager(partMan,currentTime);

			for(FsAirplane *seeker=nullptr; nullptr!=(seeker=FindNextAirplane(seeker)); )
			{
				seeker->AddSmokeToParticleManager(partMan,currentTime,cfgPtr->smkRemainTime);
			}
		}
		partMan.Sort(actualViewMode.viewPoint,actualViewMode.viewAttitude.GetForwardVector(),threadPool);	

		auto &commonTexture=FsCommonTexture::GetCommonTexture();
		commonTexture.GetParticleSpriteTexture();

		if(YSTRUE==FsIsPointSpriteAvailable())
		{
			const double pointSpriteDistThreshold=1000.0;
			partMan.MakeBufferForTriangle(actualViewMode.viewAttitude.GetForwardVector(),0.125,pointSpriteDistThreshold);
			partMan.MakeBufferForPointSprite(pointSpriteDistThreshold);
		}
		else
		{
			partMan.MakeBufferForTriangle(actualViewMode.viewAttitude.GetForwardVector(),0.125,prj.farz);
		}
	}



#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-2\n");
#endif

	SimDrawPrepare(actualViewMode);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-3\n");
#endif

	if(weather->GetFog()==YSTRUE)
	{
		FsFogOn(fogColor,actualViewMode.fogVisibility);
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-4\n");
#endif

	if(YSTRUE==FsIsShadowMapAvailable())
	{
		SimDrawShadowMap(actualViewMode);
	}


	auto projTfmBkg=SimDrawPrepareBackground(actualViewMode);
	SimDrawBackground(actualViewMode,projTfmBkg);
	if(weather->GetFog()==YSTRUE)
	{
		FsFogOff();
	}



#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-5\n");
#endif

// LARGE_INTEGER ctr1,ctr2,ctr3,ctr4,ctr5;
// QueryPerformanceCounter(&ctr1);

	{
#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-5.1\n");
#endif


		if(actualViewMode.actualViewMode!=FSCOCKPITVIEW &&
		   actualViewMode.actualViewMode!=FSADDITIONALAIRPLANEVIEW &&
		   actualViewMode.actualViewMode!=FSADDITIONALAIRPLANEVIEW_CABIN)
		{
			prj.nearz=1.0;
		}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-5.2\n");
#endif

		// See update.txt for changes of depth intervals.
		// Changes has been cancelled (2008/01/11)
		//   Protect Polygon are drawn with side walls to prevent
		//   something to be seen through the protect polygon due to
		//   the clipping.
		if(cfgPtr->zbuffQuality<=0)
		{
			auto usedProj=SimDrawPrepareRange(actualViewMode,prj.nearz,prj.farz);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
		}
		else if(cfgPtr->zbuffQuality==1)
		{
			auto usedProj=SimDrawPrepareRange(actualViewMode,200.0    ,prj.farz);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
			usedProj=SimDrawPrepareRange(actualViewMode,prj.nearz,201.0);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
		}
		else if(cfgPtr->zbuffQuality==2)
		{
			auto usedProj=SimDrawPrepareRange(actualViewMode,400.0    ,prj.farz);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
			usedProj=SimDrawPrepareRange(actualViewMode,100.0    ,401.0);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
			usedProj=SimDrawPrepareRange(actualViewMode,prj.nearz,101.0);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
		}
		else if(cfgPtr->zbuffQuality>=3)
		{
			auto usedProj=SimDrawPrepareRange(actualViewMode,1000.0   ,prj.farz);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
			usedProj=SimDrawPrepareRange(actualViewMode,500.0    ,1001.0);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
			usedProj=SimDrawPrepareRange(actualViewMode,300.0    ,501.0);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
			usedProj=SimDrawPrepareRange(actualViewMode,100.0    ,301.0);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
			usedProj=SimDrawPrepareRange(actualViewMode,prj.nearz,101.0);
			SimDrawScreenZBufferSensitive(cockpitIndicationSet,partMan,actualViewMode,usedProj);
		}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-5.y\n");
#endif

	}

	auto projForeGround=SimDrawPrepareNormal(actualViewMode);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-6\n");
#endif

// QueryPerformanceCounter(&ctr2);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-7\n");
#endif

	if(cfgPtr->drawVirtualJoystick==YSTRUE)
	{
		SimDrawJoystick(actualViewMode);
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-8\n");
#endif

	SimDrawForeground(actualViewMode,projForeGround,cockpitIndicationSet,demoMode,showTimer,showTimeMarker);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-9\n");
#endif

	SimDrawBlackout(actualViewMode);

// QueryPerformanceCounter(&ctr3);

	nFrameForFpsCount++;
	auto fpsTimer=FsSubSecondTimer();
	if(nextFpsUpdateTime<fpsTimer)
	{
		auto dtMS=fpsTimer-lastFpsUpdateTime;
		double dt=(double)dtMS/1000.0;
		fps=(double)nFrameForFpsCount/dt;
		nFrameForFpsCount=0;
		lastFpsUpdateTime=fpsTimer;
		nextFpsUpdateTime=fpsTimer+500;
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-10\n");
#endif

	if(cfgPtr->showFps==YSTRUE && FsIsMainWindowActive()==YSTRUE)
	{
		int wid,hei;
		FsGetWindowSize(wid,hei);

		char str[256];
		sprintf(str,"%.2lf FPS",fps);
		FsDrawString(0,hei-2,str,YsWhite());
	}


#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-11\n");
#endif


#ifdef SHAREWARE
	FsSet2DDrawing();
	DrawSharewareMessage();
#endif

// QueryPerformanceCounter(&ctr4);

// QueryPerformanceCounter(&ctr5);
// printf("+ %d %d %d %d\n",ctr2.LowPart-ctr1.LowPart,ctr3.LowPart-ctr2.LowPart,ctr4.LowPart-ctr3.LowPart,ctr5.LowPart-ctr4.LowPart);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-99\n");
#endif
}

void FsSimulation::SimDrawShadowMap(const ActualViewMode &actualViewMode) const
{
	if(YSTRUE==FsIsShadowMapAvailable() && cfgPtr->drawShadow==YSTRUE)
	{
		auto &commonTexture=FsCommonTexture::GetCommonTexture();
		commonTexture.ReadyShadowMap();

		for(int i=0; i<commonTexture.GetMaxNumShadowMap(); ++i)
		{
			auto texUnit=commonTexture.GetShadowMapTexture(i);
			if(nullptr!=texUnit)
			{
				auto &projMat=actualViewMode.shadowProjMat[i];
				auto &viewMat=actualViewMode.shadowViewMat[i];
				auto texWid=texUnit->GetWidth();
				auto texHei=texUnit->GetHeight();

				texUnit->BindFrameBuffer();

				FsBeginRenderShadowMap(projMat,viewMat,texWid,texHei);

				field.DrawVisual(viewMat,projMat,YSTRUE); // forShadowMap=YSTRUE

				FsAirplane *airSeeker;
				YsVec3 pos;
				airSeeker=NULL;
				while((airSeeker=FindNextAirplane(airSeeker))!=NULL)
				{
					if(cfgPtr->shadowOfDeadAirplane!=YSTRUE && airSeeker->IsAlive()!=YSTRUE)
					{
						continue;
					}

					airSeeker->DrawShadow(viewMat,projMat,YsIdentity4x4());
					if(cfgPtr->drawOrdinance==YSTRUE)
					{
						airSeeker->Prop().DrawOrdinanceVisual(
						    cfgPtr->drawCoarseOrdinance,airSeeker->weaponShapeOverrideStatic,viewMat,projMat,YsVisual::DRAWALL);
					}
				}

				FsGround *gndSeeker;
				FsProjection prj;
				GetProjection(prj,actualViewMode);

				gndSeeker=NULL;
				while((gndSeeker=FindNextGround(gndSeeker))!=NULL)
				{
					if(cfgPtr->shadowOfDeadAirplane!=YSTRUE && gndSeeker->IsAlive()!=YSTRUE)
					{
						continue;
					}
					if(gndSeeker->Prop().NoShadow()==YSTRUE)
					{
						continue;
					}

					gndSeeker->DrawShadow(viewMat,projMat,YsIdentity4x4());
				}

				FsEndRenderShadowMap();

				texUnit->Bind(5+i);
				FsEnableShadowMap(actualViewMode.viewMat,projMat,viewMat,5+i,0+i);
			}
		}
	}
	else
	{
		auto &commonTexture=FsCommonTexture::GetCommonTexture();
		for(int i=0; i<commonTexture.GetMaxNumShadowMap(); ++i)
		{
			FsDisableShadowMap(5+i,0+i);
		}
	}
}

void FsSimulation::SimDrawGuiDialog(void) const
{
	FsSet2DDrawing();

	if(showReplayDlg==YSTRUE && replayDlg!=NULL)
	{
		replayDlg->Show();
	}

	if(FsIsMainWindowActive()==YSTRUE)
	{
		const FsGuiDialog *dlg=GetCurrentInFlightDialog();
		if(NULL!=dlg)
		{
			dlg->Show();
		}
	}

	if(contDlg!=NULL)
	{
		contDlg->Show();
	}

	FsFlushScene();
}

void FsSimulation::SimDrawScreenZBufferSensitive(
	const FsCockpitIndicationSet &,
	const YsGLParticleManager &particleMan,
	const ActualViewMode &actualViewMode,
	class FsProjection &proj) const
{
#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	YsColor fogColor;
	switch(env)
	{
	case FSDAYLIGHT:
		fogColor.SetDoubleRGB(0.6,0.6,0.6);
		break;
	case FSNIGHT:
		fogColor.SetDoubleRGB(0.1,0.1,0.1);
		break;
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	if(weather->GetFog()==YSTRUE)
	{
		FsFogOn(fogColor,actualViewMode.fogVisibility);
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	SimDrawMap(actualViewMode,proj,0.0,0.0);

	// field.DrawProtectPolygon(actualViewMode.viewPoint,actualViewMode.viewAttitude,proj.GetMatrix(),nearZ);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	auto smokeTrailType=cfgPtr->smkType;
	if(YSTRUE==cfgPtr->useParticle)
	{
		smokeTrailType=FSSMKNULL;
	}
	if(cfgPtr->drawTransparency!=YSTRUE || cfgPtr->drawTransparentLater!=YSTRUE)
	{
		SimDrawAirplane(actualViewMode,proj,FSVISUAL_DRAWALL);
		SimDrawGround(actualViewMode,proj,FSVISUAL_DRAWALL);
		bulletHolder.BeginDraw();
		bulletHolder.Draw(
		    cfgPtr->drawCoarseOrdinance,actualViewMode.viewMat,proj.GetMatrix(),cfgPtr->drawTransparentSmoke,smokeTrailType,currentTime,FSVISUAL_DRAWALL);
		bulletHolder.EndDraw();
	}
	else
	{
		SimDrawAirplane(actualViewMode,proj,FSVISUAL_DRAWOPAQUE);
		SimDrawGround(actualViewMode,proj,FSVISUAL_DRAWOPAQUE);
		bulletHolder.BeginDraw();
		bulletHolder.Draw(
		    cfgPtr->drawCoarseOrdinance,actualViewMode.viewMat,proj.GetMatrix(),cfgPtr->drawTransparentSmoke,smokeTrailType,currentTime,FSVISUAL_DRAWOPAQUE);
		bulletHolder.EndDraw();
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	bulletHolder.DrawGunCalibrator();

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	YsScenery::numSceneryDrawn=0;
	SimDrawField(actualViewMode,proj);
	// printf("# Scn Drawn=%d\n",YsScenery::numSceneryDrawn);
	// printf("View Point=%s\n",viewPoint.Txt());
	// printf("nearZ=%.2lf  farZ=%.2lf  tanFov=%.2lf\n",nearZ,farZ,tanFov);


#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	weather->DrawCloudLayer(actualViewMode.viewPoint);  // DrawCloudLayer must come before drawing solid clouds, which may be drawn by particles

	if(YSTRUE!=cfgPtr->useParticle)
	{
		solidCloud->Draw(env,*weather,actualViewMode.viewMat,proj.nearz,proj.farz,proj.tanFov);
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	if(cfgPtr->drawTransparency==YSTRUE && cfgPtr->drawTransparentLater==YSTRUE)
	{
		SimDrawAirplane(actualViewMode,proj,FSVISUAL_DRAWTRANSPARENT);
		SimDrawGround(actualViewMode,proj,FSVISUAL_DRAWTRANSPARENT);
		bulletHolder.BeginDraw();
		bulletHolder.Draw(
		    cfgPtr->drawCoarseOrdinance,actualViewMode.viewMat,proj.GetMatrix(),cfgPtr->drawTransparentSmoke,smokeTrailType,currentTime,FSVISUAL_DRAWTRANSPARENT);
		bulletHolder.EndDraw();
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	SimDrawShadow(actualViewMode,proj);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	particleStore.Draw(particleMan);  // Draw before explosion.  Burning fire must not be hidden by semi-transparent smoke sphere.
	explosionHolder.Draw(actualViewMode.viewPoint,cfgPtr->drawTransparency,cfgPtr->useOpenGlListForExplosion);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	SimDrawAirplaneVaporSmoke();

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif


	if(weather->GetFog()==YSTRUE)
	{
		FsFogOff();
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

	if(actualViewMode.isViewPointInCloud==YSTRUE)
	{
		if(weather->GetFog()==YSFALSE)
		{
			solidCloud->ReduceVisibilityByPolygon(actualViewMode.viewMat,fogColor,cfgPtr->drawTransparency);
		}
	}

#ifdef CRASHINVESTIGATION_SIMDRAWSCREENZBUFFERSENSITIVE
	printf("SimDrawScreenZBufferSensitive %d\n",__LINE__);
#endif

// #### Draw a wall of quadrilateral in front of the camera if isViewPointInCloud==YSTRUE && (it is Non-OpenGL or fog is off)
}

FsProjection FsSimulation::SimDrawPrepare(const ActualViewMode &actualViewMode) const
{
#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-2.1\n");
#endif

	FsProjection prj;
	int wid,hei,sizx,sizy;

	FsGetWindowSize(wid,hei);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-2.2\n");
#endif

	GetProjection(prj,actualViewMode);
	nearZ=prj.nearz;
	farZ=prj.farz;
	tanFov=prj.tanFov;


#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-2.3\n");
#endif

	FsClearScreenAndZBuffer(YsBlack());
	FsSetSceneProjection(prj);
	FsSetCameraPosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,YSTRUE);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-2.6\n");
#endif

	FsSetDirectionalLight(actualViewMode.viewPoint,cfgPtr->lightSourceDirection,env);

	sizx=hei*4/3;
	sizy=hei;
	hud->SetAreaByCenter(wid/2,hei*2/3,sizx*2/3,sizy*2/3);

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-2.8\n");
#endif

#ifdef CRASHINVESTIGATION_SIMDRAWSCREEN
	printf("SIMDRAW-2.9\n");
#endif

	return prj;
}

FsProjection FsSimulation::SimDrawPrepareBackground(const ActualViewMode &actualViewMode) const
{
	FsProjection prj;
	int wid,hei;

	FsGetWindowSize(wid,hei);

	GetProjection(prj,actualViewMode);
	prj.farz=40000.0; // #### 80000.0 ? 20000.0  What to do with sky sphere?
	nearZ=prj.nearz;
	farZ=prj.farz;
	tanFov=prj.tanFov;


	FsSetSceneProjection(prj);
	FsSetCameraPosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,YSTRUE);

	return prj;
}

FsProjection FsSimulation::SimDrawPrepareRange(const ActualViewMode &actualViewMode,const double &nZ,const double &fZ) const  // OpenGL Only
{
	FsProjection prj;
	int wid,hei;
	FsGetWindowSize(wid,hei);

	FsFlushScene();

	GetProjection(prj,actualViewMode);
	prj.nearz=nZ;
	prj.farz=fZ;


	FsSetSceneProjection(prj);
	FsSetCameraPosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,YSTRUE);

	nearZ=prj.nearz;
	farZ=prj.farz;
	tanFov=prj.tanFov;

	return prj;
}

FsProjection FsSimulation::SimDrawPrepareNormal(const ActualViewMode &actualViewMode) const // OpenGL Only
{
	FsProjection prj;
	int wid,hei;
	FsGetWindowSize(wid,hei);

	FsFlushScene();

	GetProjection(prj,actualViewMode);
	nearZ=prj.nearz;
	farZ=prj.farz;
	tanFov=prj.tanFov;


	FsSetSceneProjection(prj);
	FsSetCameraPosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,YSTRUE);

	return prj;
}

void FsSimulation::SimDrawBackground(const ActualViewMode &actualViewMode,const FsProjection &proj) const
{
	auto gnd=gndColor;
	auto sky=skyColor;

	YSBOOL gndSpecular=this->gndSpecular;
	YsColor horizonColor;
	switch(env)
	{
	case FSNIGHT:
		gnd.SetDoubleRGB(gnd.Rd()*0.1,gnd.Gd()*0.1,gnd.Bd()*0.1);
		sky.SetDoubleRGB(sky.Rd()*0.1,sky.Gd()*0.1,sky.Bd()*0.1);
		horizonColor=YsGrayScale(0.1);
		gndSpecular=YSFALSE;
		break;
	case FSDAYLIGHT:
		horizonColor.SetDoubleRGB(0.7,0.7,0.7);
		break;
	}

	if(weather->GetFog()==YSTRUE)
	{
		groundSky->DrawByFog(actualViewMode.viewPoint,actualViewMode.viewAttitude,gnd,sky,horizonColor,farZ,gndSpecular);
	}
	else if(cfgPtr->horizonGradation==YSTRUE)
	{
		groundSky->DrawGradation(actualViewMode.viewPoint,actualViewMode.viewAttitude,gnd,sky,horizonColor,farZ,gndSpecular);
	}
	else
	{
		groundSky->DrawCrappy(actualViewMode.viewPoint,gnd,sky,farZ,gndSpecular);
	}

	// 2016/11/26 To reduce the number of fragments,
	//            DrawGroundMesh only draws pixels where stencil value is equal to zero.
	//            SimDrawMap must increment the stencil value in each pixel.
	{
		int div;
		auto y=actualViewMode.viewPoint.y();
		if(y>2700.0)
		{
			div=4050;
		}
		else if(y>900.0)
		{
			div=1350;
		}
		else if(y>300.0)
		{
			div=450;
		}
		else if(y>100.0)
		{
			div=150;
		}
		else
		{
			div=50;
		}
		groundSky->DrawGroundMesh(actualViewMode.viewPoint,actualViewMode.viewAttitude,gnd,div,gndSpecular);
	}
}

void FsSimulation::SimDrawMap(const ActualViewMode &actualViewMode,const FsProjection &proj,const double &elvMin,const double &elvMax) const
{
	// >>>> Map and shadow
	YSBOOL drawPset=YSFALSE;
	if(FSDAYLIGHT!=GetEnvironment())
	{
		drawPset=YSTRUE;
	}
	else if(FSDAYLIGHT==GetEnvironment() && 
	        cfgPtr->drawLightsInDaylight==YSTRUE &&
	        weather->GetFogVisibility()<cfgPtr->drawLightsInDaylightVisibilityThr)
	{
		drawPset=YSTRUE;
	}

	YsScenery::numSceneryDrawn=0;
	field.CacheMapDrawingOrder();
	field.DrawMapVisual
	   (env,
	    actualViewMode.viewPoint,actualViewMode.viewAttitude,proj.GetMatrix(),elvMin,elvMax,drawPset,currentTime,
	    cfgPtr->useOpenGlGroundTexture,cfgPtr->useOpenGlRunwayLightTexture);

	int nGndShadowDrawn,nAirShadowDrawn;
	nGndShadowDrawn=0;
	nAirShadowDrawn=0;
	// <<<< Map and shadow
}

void FsSimulation::SimDrawAirplane(const ActualViewMode &actualViewMode,const FsProjection &proj,unsigned int drawFlag) const
{
	auto &viewPoint=actualViewMode.viewPoint;
	auto &viewMat=actualViewMode.viewMat;

	FsAirplane *seeker;
	double airRad;
	YsVec3 airPos;

	seeker=NULL;
	while((seeker=FindNextAirplane(seeker))!=NULL)
	{
		if(seeker->IsAlive()==YSTRUE)
		{
			airPos=seeker->GetPosition();
			airRad=seeker->GetApproximatedCollideRadius();

			if(actualViewMode.actualViewMode==FSBOMBINGVIEW &&
			   IsPlayerAirplane(seeker)==YSTRUE &&
			   seeker->IsAlive()==YSTRUE)
			{
				continue;
			}

			{
				YSBOOL drawCoarseOrdinance;
				drawCoarseOrdinance=cfgPtr->drawCoarseOrdinance;

				double lodDist;

				// If cfgPtr->airLod==0 (Automatic), Weapon LOD is also automatic
				// Otherwise, Weapon LOD depends on drawCoarseOrdinance

				switch(cfgPtr->airLod)
				{
				case 0: // Default
					lodDist=airRad*16.0*actualViewMode.viewMagFix; // 20041204
					lodDist*=viewMagUser; // 2006/08/01
					if((airPos-viewPoint).GetSquareLength()<lodDist*lodDist)
					{
						seeker->Draw(0,actualViewMode.viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
					}
					else
					{
						seeker->Draw(1,actualViewMode.viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
						drawCoarseOrdinance=YSTRUE;
					}
					break;
				case 1: // Always High Quality
					seeker->Draw(0,actualViewMode.viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
					break;
				case 2: // Always Coarse
					seeker->Draw(1,actualViewMode.viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
					break;
				case 3: // Super-coarse
					seeker->UntransformedCollisionShell().Draw(
					    actualViewMode.viewMat,proj.GetMatrix(),seeker->GetPosition(),seeker->GetAttitude(),drawFlag);
					break;
				}
				if(cfgPtr->drawOrdinance==YSTRUE)
				{
					seeker->Prop().DrawOrdinanceVisual(
					    drawCoarseOrdinance,seeker->weaponShapeOverrideStatic,actualViewMode.viewMat,proj.GetMatrix(),drawFlag);
				}
			}
		}

		if((actualViewMode.actualViewMode==FSCOCKPITVIEW ||
		    actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW ||
		    actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW_CABIN) &&
		   IsPlayerAirplane(seeker)==YSTRUE)
		{
			if(seeker->cockpit!=nullptr)
			{
				seeker->cockpit.Draw(actualViewMode.viewMat,proj.GetMatrix(),seeker->GetPosition(),seeker->GetAttitude(),drawFlag);
			}
			else if(cockpit!=nullptr)
			{
				cockpit->Draw(actualViewMode.viewMat,proj.GetMatrix(),seeker->GetPosition(),seeker->GetAttitude(),drawFlag);
			}
		}
	}


	// Smoke and vapor must be drawn later due to trasnparency.
	// Hence, now they are drawn in SimDrawAirplaneVaporSmoke
}

void FsSimulation::SimDrawGround(const ActualViewMode &actualViewMode,const FsProjection &proj,unsigned int drawFlag) const
{
	auto &viewPoint=actualViewMode.viewPoint;
	auto &viewMat=actualViewMode.viewMat;

	FsGround *seeker;

	seeker=NULL;
	while((seeker=FindNextGround(seeker))!=NULL)
	{
		if(seeker->IsAlive()==YSTRUE)
		{
			if(actualViewMode.actualViewMode==FSBOMBINGVIEW &&
			   GetPlayerGround()==seeker)
			{
				continue;
			}


			double objRad,distance,apparentRad;

			objRad=seeker->Prop().GetOutsideRadius();
			distance=(seeker->GetPosition()-viewPoint).GetLength();
			apparentRad=objRad*proj.prjPlnDist/distance;

			if(apparentRad>=1)  // Apparent Radius is larger than 1 pixels
			{
				switch(cfgPtr->gndLod)
				{
				case 0: // Default
					if((seeker->GetPosition()-viewPoint).GetSquareLength()<(objRad*objRad)*400.0)
					{
						seeker->Draw(0,viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
					}
					else
					{
						seeker->Draw(1,viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
					}
					break;
				case 1: // Always High Quality
					seeker->Draw(0,viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
					break;
				case 2: // Always Coarse
					seeker->Draw(1,viewMat,proj.GetMatrix(),viewPoint,drawFlag,currentTime);
					break;
				case 3: // Super-coarse
					seeker->UntransformedCollisionShell().Draw(
					    viewMat,proj.GetMatrix(),seeker->GetPosition(),seeker->GetAttitude(),drawFlag);
					break;
				}
			}
		}

		if((actualViewMode.actualViewMode==FSCOCKPITVIEW ||
		    actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW ||
		    actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW_CABIN) &&
		   GetPlayerGround()==seeker)
		{
			if(seeker->cockpit!=nullptr)
			{
				seeker->cockpit.Draw(actualViewMode.viewMat,proj.GetMatrix(),seeker->GetPosition(),seeker->GetAttitude(),drawFlag);
			}
		}
	}

	FsAircraftCarrierProperty::BeginDrawArrestingWire();
	for(int i=0; i<aircraftCarrierList.GetN(); i++)
	{
		if(aircraftCarrierList[i]->IsAlive()==YSTRUE)
		{
			aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty()->DrawBridge(viewMat);
			aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty()->DrawArrestingWire();
		}
	}
	FsAircraftCarrierProperty::EndDrawArrestingWire();
}

void FsSimulation::SimDrawAirplaneVaporSmoke(void) const
{
	FsAirplane *seeker;
	seeker=NULL;
	while((seeker=FindNextAirplane(seeker))!=NULL)
	{
		if(seeker->IsAlive()==YSTRUE)
		{
			seeker->DrawVapor(currentTime,0.5,4,cfgPtr->drawTransparentVapor);
		}
		if(YSTRUE!=cfgPtr->useParticle)
		{
			seeker->DrawSmoke(currentTime,cfgPtr->smkRemainTime,cfgPtr->smkType,cfgPtr->smkStep,cfgPtr->drawTransparentSmoke);
		}
	}
}

void FsSimulation::SimDrawField(const ActualViewMode &actualViewMode,const class FsProjection &proj) const
{
	field.DrawVisual(actualViewMode.viewPoint,actualViewMode.viewAttitude,proj.GetMatrix(),YSFALSE); // forShadowMap=YSFALSE

	if(cfgPtr->drawCloud==YSTRUE && env!=FSNIGHT)
	{
		cloud->Draw();
	}
}

void FsSimulation::SimDrawShadow(const ActualViewMode &actualViewMode,const class FsProjection &proj) const  // For OpenGL/Direct3D, not for BlueImpulseSDK
{
	if(YSTRUE!=FsIsShadowMapAvailable() || cfgPtr->drawShadow!=YSTRUE)
	{
		FsBeginDrawShadow();  // Set polygon offset -1,-1 and enable.
		SimDrawComplexShadow(actualViewMode,proj);
		FsEndDrawShadow();    // Disable polygon offset.
	}
}

void FsSimulation::SimDrawComplexShadow(const ActualViewMode &actualViewMode,const class FsProjection &proj) const // For OpenGL/Direct3D, not for BlueImpulseSDK
{
	auto &viewPoint=actualViewMode.viewPoint;
	auto &viewMat=actualViewMode.viewMat;
	auto &projMat=proj.GetMatrix();

	if(cfgPtr->drawShadow==YSTRUE)
	{
		FsAirplane *airSeeker;
		YsVec3 pos;
		airSeeker=NULL;
		while((airSeeker=FindNextAirplane(airSeeker))!=NULL)
		{
			if(cfgPtr->shadowOfDeadAirplane!=YSTRUE && airSeeker->IsAlive()!=YSTRUE)
			{
				continue;
			}

			pos=airSeeker->GetPosition();

			// Shadow on deck >>
			int i;
			for(i=0; i<aircraftCarrierList.GetN(); i++)
			{
				FsAircraftCarrierProperty *prop;
				double deckHeight;
				YsVec3 deckNom;

				prop=aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty();
				deckHeight=prop->GetDeckHeightAndNormal(deckNom,pos);

				if(aircraftCarrierList[i]->IsAlive()==YSTRUE &&
				   viewPoint.y()>=deckHeight &&
				   pos.y()>=deckHeight &&
				   prop->IsOnDeck(pos)==YSTRUE)
				{
					const YsVec3 terOrg(pos.x(),deckHeight+0.01,pos.z());
					const YsVec3 &terNom=YsYVec();
					const YsVec3 &lightDir=-YsYVec();
					YsMatrix4x4 shadowTfm;
					if(YSOK==YsMakePlaneProjectionMatrix(shadowTfm,terOrg,terNom,lightDir))
					{
						if((viewPoint-terOrg).GetSquareLength()<100.0*100.0)
						{
							airSeeker->DrawShadow(viewMat,projMat,shadowTfm);
						}
						else
						{
							airSeeker->DrawApproximatedShadow(viewMat,projMat,shadowTfm);
						}
					}
				}
			}
			// Shadow on deck <<

			YsVec3 terOrg=airSeeker->terrainOrg;
			const YsVec3 &terNom=airSeeker->terrainNom;
			const YsVec3 &lightDir=-YsYVec();
			terOrg.AddY(0.01);
			YsMatrix4x4 shadowTfm;
			if(YSOK==YsMakePlaneProjectionMatrix(shadowTfm,terOrg,terNom,lightDir))
			{
				if(airSeeker->IsAlive()==YSTRUE && (viewPoint-terOrg).GetSquareLength()<100.0*100.0)
				{
					airSeeker->DrawShadow(viewMat,projMat,shadowTfm);
				}
				else
				{
					airSeeker->DrawApproximatedShadow(viewMat,projMat,shadowTfm);
				}
			}
		}

		FsGround *gndSeeker;
		FsProjection prj;
		GetProjection(prj,actualViewMode);

		gndSeeker=NULL;
		while((gndSeeker=FindNextGround(gndSeeker))!=NULL)
		{
			if(cfgPtr->shadowOfDeadAirplane!=YSTRUE && gndSeeker->IsAlive()!=YSTRUE)
			{
				continue;
			}
			if(gndSeeker->Prop().NoShadow()==YSTRUE || YSTRUE==gndSeeker->PiggyBack())
			{
				continue;
			}

			pos=gndSeeker->GetPosition();
			pos.SetY(gndSeeker->elevation);

			YsVec3 terOrg=gndSeeker->terrainOrg;
			const YsVec3 &terNom=gndSeeker->terrainNom;
			const YsVec3 &lightDir=-YsYVec();
			terOrg.AddY(0.01);
			YsMatrix4x4 shadowTfm;
			if(YSOK==YsMakePlaneProjectionMatrix(shadowTfm,terOrg,terNom,lightDir))
			{
				if(gndSeeker->IsAlive()==YSTRUE && (viewPoint-pos).GetSquareLength()<100.0*100.0)
				{
					gndSeeker->DrawShadow(viewMat,projMat,shadowTfm);
				}
				else
				{
					double objRad,distance,apparentRad;

					objRad=gndSeeker->Prop().GetOutsideRadius();
					distance=(gndSeeker->GetPosition()-viewPoint).GetLength();
					apparentRad=objRad*prj.prjPlnDist/distance;

					if(apparentRad>=1)  // Apparent Radius is larger than 1 pixels
					{
						gndSeeker->DrawApproximatedShadow(viewMat,projMat,shadowTfm);
					}
				}
			}
		}
	}
	else
	{
		FsAirplane *airSeeker;
		airSeeker=NULL;
		while((airSeeker=FindNextAirplane(airSeeker))!=NULL)
		{
			// 2001/11/22
			if(cfgPtr->shadowOfDeadAirplane!=YSTRUE && airSeeker->IsAlive()!=YSTRUE)
			{
				continue;
			}

			YsVec3 terOrg=airSeeker->terrainOrg;
			const YsVec3 &terNom=airSeeker->terrainNom;
			const YsVec3 &lightDir=-YsYVec();
			terOrg.AddY(0.01);
			YsMatrix4x4 shadowTfm;
			if(YSOK==YsMakePlaneProjectionMatrix(shadowTfm,terOrg,terNom,lightDir))
			{
				airSeeker->DrawApproximatedShadow(viewMat,projMat,shadowTfm);
			}
		}
	}
}

void FsSimulation::SimDrawJoystick(const ActualViewMode &actualViewMode) const
{
	if(actualViewMode.actualViewMode==FSCOCKPITVIEW)
	{
		YsVec3 jsPos,jsUv,jsEv;
		YsAtt3 jsAtt;

		jsPos.Set(-3.0,-1.0,5.0);
		jsAtt.Set(0.0,YsDegToRad(15.0),0.0);

		// Memo: New FsVisual::Draw assumes that the viewpoint is at the origin looking straight ahead.
		// Doesn't have to transform it with viewPoint and viewAttitude.
		userInput.DrawJoystick(jsPos,jsAtt);
	}
}

extern void FsTestPerformance(void);

void FsSimulation::SimDrawAircraftInterior(const ActualViewMode &actualViewMode,const FsProjection &proj,const FsAirplane *air,unsigned int instDrawSwitch,const FsCockpitIndicationSet &cockpitIndicationSet) const
{
	YsVec3 localViewPos;
	YsVec3 offset;
	YsMatrix4x4 instViewMat;
	YsAtt3 instViewAtt;

	offset=air->GetPosition();

	switch(actualViewMode.actualViewMode)
	{
	default:
	case FSCOCKPITVIEW:
		air->Prop().GetCockpitPosition(localViewPos);
		break;
	case FSADDITIONALAIRPLANEVIEW:
	case FSADDITIONALAIRPLANEVIEW_CABIN:
		const FsAdditionalViewpoint *vp;
		vp=air->Prop().GetAdditionalView(mainWindowAdditionalAirplaneViewId);
		if(vp!=NULL)
		{
			localViewPos=vp->pos;
		}
		break;
	}
	YsVec3 fakeViewPos=localViewPos;
	air->GetAttitude().Mul(fakeViewPos,fakeViewPos);


	instViewAtt=actualViewMode.viewAttitude;

	instViewMat.Initialize();
	instViewMat.RotateXY(-instViewAtt.b());
	instViewMat.RotateZY(-instViewAtt.p());
	instViewMat.RotateXZ(-instViewAtt.h());
	instViewMat.Translate(-fakeViewPos);

	FsSetCameraPosition(fakeViewPos,instViewAtt,YSFALSE); // BiStartBuffer(&eye);

	if(YSTRUE==NeedToDrawInstrument(actualViewMode))
	{
		// This block draws 3D HUD and 3D Inst Panel
		if(FSCOCKPITVIEW!=actualViewMode.actualViewMode && FSADDITIONALAIRPLANEVIEW!=actualViewMode.actualViewMode)
		{
			instViewAtt=air->GetAttitude();

			instViewMat.Initialize();
			instViewMat.RotateXY(-instViewAtt.b());
			instViewMat.RotateZY(-instViewAtt.p());
			instViewMat.RotateXZ(-instViewAtt.h());
			instViewMat.Translate(-fakeViewPos);

			FsSetCameraPosition(fakeViewPos,instViewAtt,YSFALSE); // BiStartBuffer(&eye);
		}

		if(0!=(instDrawSwitch&(FSISS_3DHUD|FSISS_2DHUD)) && YSTRUE==air->Prop().CheckHUDVisible())
		{
			// Velocity Vector Marker
			if(air->Prop().GetVectorMarker()==YSTRUE)
			{
				YsVec3 vel;
				air->Prop().GetVelocity(vel);
				hud->DrawVelocityVectorIndicator(fakeViewPos,instViewAtt,vel);
			}
		}


		if(0!=(instDrawSwitch&FSISS_2DHUD) && YSTRUE==air->Prop().CheckHUDVisible())
		{
			// Attitude Indicator (2D HUD)
			YsAtt3 indicatedAttitude(
			    cockpitIndicationSet.inst.heading,
			    cockpitIndicationSet.inst.pitch,
			    cockpitIndicationSet.inst.bank);
			hud->DrawAttitude(fakeViewPos,indicatedAttitude,fakeViewPos,instViewAtt);
		}

		if(0!=(instDrawSwitch&FSISS_3DINSTPANEL))
		{
			SimDrawInstPanel3d(fakeViewPos,localViewPos,cockpitIndicationSet);
		}
		if(0!=(instDrawSwitch&FSISS_3DHUD) && YSTRUE==air->Prop().CheckHUDVisible())
		{
			SimDrawHud3d(fakeViewPos,instViewAtt,cockpitIndicationSet);
		}
	}

	// // Prepare to draw attitude on HUD
	// printf("2\n");FsTestPerformance();
	FsFlushScene(); // BiFlushBuffer();
	FsSetCameraPosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,YSTRUE); // BiStartBuffer(&eye);


	// #### ltc.TestDraw();  Lattice Test
	if(cfgPtr->neverDrawAirplaneContainer==YSFALSE)
	{
		if(YSTRUE==NeedToDrawGameInfo(actualViewMode))
		{
			SimDrawContainer(actualViewMode);
			SimDrawGunAim();
			SimDrawBombingAim(actualViewMode);
		}
		else if(cfgPtr->drawPlayerNameAlways==YSTRUE)
		{
			SimDrawContainer(actualViewMode);
		}
	}

	FsFlushScene(); // Added 2003/05/14
}

void FsSimulation::SimDrawGroundInterior(const ActualViewMode &actualViewMode,const class FsProjection &proj,const FsGround *gnd,const FsCockpitIndicationSet &cockpitIndicationSet) const
{
	YsVec3 localViewPos;
	YsVec3 offset;
	YsMatrix4x4 instViewMat;
	YsAtt3 instViewAtt;

	offset=gnd->GetPosition();

	switch(actualViewMode.actualViewMode)
	{
	default:
	case FSCOCKPITVIEW:
		localViewPos=gnd->Prop().GetUserViewPoint();
		break;
	case FSADDITIONALAIRPLANEVIEW:
	case FSADDITIONALAIRPLANEVIEW_CABIN:
		const FsAdditionalViewpoint *vp;
		vp=gnd->Prop().GetAdditionalView(mainWindowAdditionalAirplaneViewId);
		if(vp!=NULL)
		{
			localViewPos=vp->pos;
		}
		break;
	}
	YsVec3 fakeViewPos=localViewPos;
	gnd->GetAttitude().Mul(fakeViewPos,fakeViewPos);


	instViewAtt=actualViewMode.viewAttitude;

	instViewMat.Initialize();
	instViewMat.RotateXY(-instViewAtt.b());
	instViewMat.RotateZY(-instViewAtt.p());
	instViewMat.RotateXZ(-instViewAtt.h());
	instViewMat.Translate(-fakeViewPos);

	FsSetCameraPosition(fakeViewPos,instViewAtt,YSFALSE); // BiStartBuffer(&eye);

	if(YSTRUE==NeedToDrawInstrument(actualViewMode))
	{
		SimDrawHud3d(fakeViewPos,instViewAtt,cockpitIndicationSet);
	}


	FsFlushScene(); // BiFlushBuffer();
	FsSetCameraPosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,YSTRUE); // BiStartBuffer(&eye);


	if(cfgPtr->neverDrawAirplaneContainer==YSFALSE)
	{
		if(YSTRUE==NeedToDrawGameInfo(actualViewMode))
		{
			SimDrawContainer(actualViewMode);
			SimDrawGunAim();
			SimDrawBombingAim(actualViewMode);
		}
		else if(cfgPtr->drawPlayerNameAlways==YSTRUE)
		{
			SimDrawContainer(actualViewMode);
		}
	}


	FsFlushScene(); // BiFlushBuffer();
}

void FsSimulation::SimDrawForeground(const ActualViewMode &actualViewMode,const class FsProjection &proj,const FsCockpitIndicationSet &cockpitIndicationSet,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker) const
{
	// printf("1\n");FsTestPerformance();

#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-1\n");
#endif

	FsFlushScene(); // BiFlushBuffer();

	const FsAirplane *playerPlane=GetPlayerAirplane();
	const FsGround *playerGround=GetPlayerGround();


	// Four cases:
	//   Viewpoint   AlwaysShowHUD
	//   Inside      YSTRUE            viewAttitude for cockpit and exterior     instViewAtt for HUD
	//   Inside      YSFALSE           viewAttitude for cockpit and exterior


#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-2\n");
#endif

	unsigned int instDrawSwitch=GetInstrumentDrawSwitch(actualViewMode);

	if(playerPlane!=NULL &&
	   playerPlane->IsAlive()==YSTRUE &&
	   (actualViewMode.actualViewMode==FSCOCKPITVIEW || 
	    actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW ||
	    actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW_CABIN ||
	    actualViewMode.actualViewMode==FSBOMBINGVIEW ||
	    YSTRUE==NeedToDrawGameInfo(actualViewMode) ||
	    YSTRUE==NeedToDrawInstrument(actualViewMode)))
	{
#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
		printf("SimDrawForeground-3\n");
#endif
		SimDrawAircraftInterior(actualViewMode,proj,playerPlane,instDrawSwitch,cockpitIndicationSet);
#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
		printf("SimDrawForeground-4\n");
#endif
	}
	else if(playerGround!=NULL &&
	        playerGround->IsAlive()==YSTRUE &&
	        (actualViewMode.actualViewMode==FSCOCKPITVIEW || 
	         actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW ||
	         actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW_CABIN ||
	         actualViewMode.actualViewMode==FSBOMBINGVIEW ||
	         YSTRUE==NeedToDrawGameInfo(actualViewMode) ||
	         YSTRUE==NeedToDrawInstrument(actualViewMode)))
	{
#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
		printf("SimDrawForeground-5\n");
#endif
		SimDrawGroundInterior(actualViewMode,proj,playerGround,cockpitIndicationSet);
#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
		printf("SimDrawForeground-6\n");
#endif
	}



	// Everything in this function below this line is 2D elements
	FsSet2DDrawing();


#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-7\n");
#endif


	if(playerPlane!=NULL)
	{
		if(0!=(instDrawSwitch&FSISS_2DHUD) && YSTRUE==NeedToDrawInstrument(actualViewMode) && YSTRUE==playerPlane->Prop().CheckHUDVisible())
		{
			YSBOOL autoPilot=(NULL!=playerPlane->GetAutopilot() ? YSTRUE : YSFALSE);
			if(long(currentTime*2.0)%2==0)
			{
				hud->Draw(autoPilot,cockpitIndicationSet);
			}
			else
			{
				hud->Draw(YSFALSE,cockpitIndicationSet);
			}

			SimDraw2dVor1(cockpitIndicationSet);
			SimDraw2dVor2(cockpitIndicationSet);
			SimDraw2dAdf(cockpitIndicationSet);
		}

		if(playerPlane->Prop().IsActive()!=YSTRUE)
		{
			int sx,sy;
			FsGetWindowSize(sx,sy);
			sx=sx/2-40;
			sy=sy*1/4+20;

			if(playerPlane->Prop().GetDiedOf()==FSDIEDOF_TAILSTRIKE)
			{
				FsDrawString(sx,sy,"TAIL STRIKE!!",YsRed());
			}
			else if(playerPlane->Prop().GetDiedOf()==FSDIEDOF_OVERRUN)
			{
				FsDrawString(sx,sy,"OVERRUN!!",YsRed());
			}
		}
		else if(playerPlane->Prop().IsActive()==YSTRUE && playerPlane->Prop().IsOutOfRunway()==YSTRUE)
		{
			int sx,sy;
			FsGetWindowSize(sx,sy);
			sx=sx/2-72;
			sy=sy*1/4+20;

			FsDrawString(sx,sy,"!!!!UNPAVED FIELD!!!!",YsRed());
			sy+=20;
			FsDrawString(sx,sy,"!!!!   OVERRUN   !!!!",YsRed());
		}

		if(YSTRUE==NeedToDrawInstrument(actualViewMode))
		{
			// Draw structural integrity
			int sx,sy;
			FsGetWindowSize(sx,sy);

			const int wid=sx/4;

			sx/=2;

			const double percent=YsGreater(0.0,100.0*(double)playerPlane->Prop().GetDamageTolerance()/(double)playerPlane->GetDefaultDamageTolerance());

			YsColor col;
			if(50.0<percent)
			{
				col=YsGreen();
			}
			else if(25.0<percent)
			{
				col=YsYellow();
			}
			else
			{
				col=YsRed();
			}
			const int barWid=(int)((double)wid*(double)percent/100.0);
			FsDrawRect(sx-barWid/2,0,sx+barWid/2,4,col,YSTRUE);

			FsDrawRect(sx-wid/2,0,sx+wid/2,4,hud->hudCol,YSFALSE);
		}
	}

#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-8\n");
#endif

	if(NULL!=playerGround)
	{
		int wid,hei;
		FsGetWindowSize(wid,hei);
		
		const double vel=playerGround->Prop().GetVelocity();

		const double kmph=YsUnitConv::MPStoKMPH(vel);
		const double mph=YsUnitConv::KMPHtoMPH(kmph);

		YsString msg;
		msg.Printf("%.0lf km/h = %.0lf mph",kmph,mph);
		FsDrawString(16,hei-16,msg,hud->hudCol);

		if(playerGround->Prop().IsDrifting())
		{
			const int sx=wid/2-72;
			const int sy=hei*1/4+20;

			FsDrawString(sx,sy,"!!!! Skidding !!!!",YsRed());
		}
	}

#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-9\n");
#endif

	if(NULL!=playerPlane || NULL!=playerGround)
	{
		if(YSTRUE==FsIsMainWindowActive() && NeedToDrawInstrument(actualViewMode)==YSTRUE)
		{
			SimDrawRadar(actualViewMode);
		}
	}

#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-10\n");
#endif

	if(playerPlane!=NULL && playerPlane->Prop().IsActive()==YSTRUE && demoMode!=YSTRUE)
	{
		int sx,sy;
		FsGetWindowSize(sx,sy);
		sx/=2;
		sy/=2;
		if(bulletHolder.IsLockedOn(playerPlane)==YSTRUE)
		{
			sx-=40;
			FsDrawString(sx,sy,"!!MISSILE!!",YsRed());
		}
		else if(IsLockedOn(playerPlane)==YSTRUE)
		{
			sx-=80;
			FsDrawString(sx,sy,"!!YOU ARE LOCKED ON!!",YsRed());
		}
		else if(playerPlane->Prop().GetFlightState()==FSSTALL)
		{
			sx-=30;
			FsDrawString(sx,sy,"STALL",YsYellow());
		}
	}


#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-11\n");
#endif


	// SYSTEM MESSAGE
	if(FsIsMainWindowActive()==YSTRUE)
	{
		int i;
		int sx,sy;
		sx=20;
		sy=6+fsAsciiRenderer.GetFontHeight();

		if(pause==YSTRUE)
		{
			FsDrawString(sx,sy,"*** PAUSE ***",YsWhite());
			sy+=fsAsciiRenderer.GetFontHeight();
		}

		if(userInput.GetSensitivity()<0.99)
		{
			YsString str;
			str.Printf("Stick Sensitivity: %.2lf",userInput.GetSensitivity());
			FsDrawString(sx,sy,str,YsWhite());
			sy+=fsAsciiRenderer.GetFontHeight();
		}

		if(escKeyCount>0)
		{
			if(escOnceToEnd!=NULL && escOnceToEnd->GetWidth()>0 && escOnceToEnd->GetHeight()>0)
			{
				FsDrawBmp(*escOnceToEnd,sx,sy-fsAsciiRenderer.GetFontHeight());
				sy+=escOnceToEnd->GetHeight();
			}
			else
			{
				FsDrawString(sx,sy,"Press ESC Once More to Quit",YsGreen());
				sy+=fsAsciiRenderer.GetFontHeight();
			}
		}
		else if(demoMode==YSTRUE)
		{
			FsDrawString(sx,sy,"Programmed By CaptainYS",YsWhite());
			sy+=fsAsciiRenderer.GetFontHeight();
			FsDrawString(sx,sy,"http://www.ysflight.com",YsWhite());
			sy+=fsAsciiRenderer.GetFontHeight();
			sy+=fsAsciiRenderer.GetFontHeight();
		}
		else // if(escKeyCount==0 && demoMode!=YSTRUE)
		{
			if(playerPlane!=NULL && playerPlane->Prop().GetFlightState()==FSGROUNDSTATIC)
			{
				if(escTwiceToEnd!=NULL && escTwiceToEnd->GetWidth()>0 && escTwiceToEnd->GetHeight()>0)
				{
					FsDrawBmp(*escTwiceToEnd,sx,sy-fsAsciiRenderer.GetFontHeight());
					sy+=escTwiceToEnd->GetHeight();
				}
				else
				{
					FsDrawString(sx,sy,"Press ESC Twice to Terminate the Simulation",YsGreen());
					sy+=fsAsciiRenderer.GetFontHeight();
				}
			}
		}

		if(mainWindowViewmode==FSGHOSTVIEW)
		{
			YsString str;
			str.Printf(str,"%.1lf m/s",ghostViewSpeed);
			FsDrawString(sx,sy,str,YsGreen());
			sy+=fsAsciiRenderer.GetFontHeight();
		}

		subMenu.Draw(this,*cfgPtr,sx,sy);

		if(systemMessage[0]!=0)
		{
			FsDrawString(sx,sy,systemMessage,YsGreen());
			sy+=fsAsciiRenderer.GetFontHeight();
		}

		if(showTimeMarker==YSTRUE)
		{
			int i;
			for(i=0; i<FSNTIMEMARKER; i++)
			{
				char str[256],buf[256];
				YsOptimizeDoubleString(str,timeMarker[i]);
				sprintf(buf,"MK%d:%s",i,str);
				FsDrawString(sx,sy,buf,YsGreen());
				sy+=fsAsciiRenderer.GetFontHeight();
			}
		}

		for(i=0; i<FSNUMTIMEDMESSAGE; i++)
		{
			if(timedMessage[i].timeRemain>YsTolerance)
			{
				FsDrawString(sx,sy,timedMessage[i].wstr,YsGreen());
				sy+=fsUnicodeRenderer.GetFontHeight();
			}
		}


		// Playtime clock
		if(showTimer==YSTRUE)
		{
			int s1x,s1y,s2x,s2y;
			char str[256];

			FsGetWindowSize(s1x,s1y);
			s2x=s1x;
			s2y=s1y;

			s1x=s1x-64;
			s1y=0;
			s2y=24;
			FsDrawRect(s1x,s1y,s2x,s2y,YsWhite(),YSTRUE);

			s1x+=8;
			s1y=20;
			sprintf(str,"%.2lf",currentTime);
			FsDrawString(s1x,s1y,str,YsBlack());
		}
	} // if(FsIsMainWindowActive()==YSTRUE)


#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-12\n");
#endif

	FsPlugInCallDrawForeground(currentTime);
	for(auto &addOnPtr : addOnList)
	{
		int windowId=0;
		int wid,hei;
		FsGetWindowSize(wid,hei);
		if(YSTRUE==FsIsMainWindowActive())
		{
			windowId=0;
			FsGetWindowSize(wid,hei);
		}
		else
		{
			for(int i=0; i<2; ++i)
			{
				if(YSTRUE==FsIsSubWindowActive(i))
				{
					windowId=i+1;
					FsGetDrawingAreaSize(wid,hei);
				}
			}
		}

		for(auto &bmpAndPos : addOnPtr->Draw2D(this,windowId,wid,hei))
		{
			FsDrawBmp(bmpAndPos.bmp,bmpAndPos.pos.x(),bmpAndPos.pos.y());
		}
	}


#ifdef CRASHINVESTIGATION_SIMDRAWFOREGROUND
	printf("SimDrawForeground-16\n");
#endif

	FsFlushScene();
}

void FsSimulation::SimDrawRadar(const ActualViewMode &actualViewMode) const
{
	FsHorizontalRadar radar;

	if(NULL!=GetPlayerAirplane())
	{
		const FsAirplane *playerPlane=GetPlayerAirplane();
		double radarRange;

		radarRange=playerPlane->Prop().GetCurrentRadarRange();

		if(YsEqual(radarRange,0.0)!=YSTRUE)
		{
			int wid,hei;
			FsGetWindowSize(wid,hei);

			long radarSize=wid/5;

			long x1=wid-radarSize-10;
			long y1=10;
			long x2=wid-10;
			long y2=10+radarSize;

			switch(playerPlane->Prop().GetWeaponOfChoice())
			{
			default:
			case FSWEAPON_GUN:
			case FSWEAPON_AIM9:
			case FSWEAPON_AIM9X:
			case FSWEAPON_AIM120:
				radar.Draw(this,x1,y1,x2,y2,radarRange,*GetPlayerAirplane(),0,cfgPtr->radarAltitudeLimit);
				break;
			case FSWEAPON_AGM65:
				radar.Draw(this,x1,y1,x2,y2,radarRange,*GetPlayerAirplane(),1,cfgPtr->radarAltitudeLimit);
				break;
			case FSWEAPON_BOMB:
			case FSWEAPON_BOMB250:
				radar.Draw(this,x1,y1,x2,y2,radarRange,*GetPlayerAirplane(),2,cfgPtr->radarAltitudeLimit);
				break;
			case FSWEAPON_BOMB500HD:
				radar.Draw(this,x1,y1,x2,y2,radarRange,*GetPlayerAirplane(),1,cfgPtr->radarAltitudeLimit);
				break;
			}
		}
	}

	if(NULL!=GetPlayerGround())
	{
		const FsGround *playerGround=GetPlayerGround();
		const double radarRange=10.0; // 10 miles

		int wid,hei;
		FsGetWindowSize(wid,hei);

		long radarSize=wid/5;

		long x1=wid-radarSize-10;
		long y1=10;
		long x2=wid-10;
		long y2=10+radarSize;

		radar.DrawBasic(this,x1,y1,x2,y2,radarRange,*playerGround,playerGround->GetPosition(),actualViewMode.viewAttitude,0,cfgPtr->radarAltitudeLimit);
	}
}

void FsSimulation::SimDrawInstPanel3d(const YsVec3 &fakeViewPos,const YsVec3 &localViewPos,const FsCockpitIndicationSet &cockpitIndicationSet) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		FsInstrumentPanel *instPanel=playerPlane->instPanel;

		instPanel->BeginDraw3d(fakeViewPos,localViewPos,playerPlane->Prop());
		instPanel->Draw3d(playerPlane->Prop(),cockpitIndicationSet);

		const FsInstrumentIndication &inst=cockpitIndicationSet.inst;

		for(int navId=0; navId<2; navId++)
		{
			const FsVORIndication &nav=cockpitIndicationSet.nav[navId];

			instPanel->DrawNav3d(
			   nav.navId,
			   (YSRESULT)nav.IsInRange(),
			   nav.vorId,
			   nav.IsTuned(),
			   nav.IsILS(),
			   nav.toFrom,
			   nav.obs,
			   nav.lateralDev,
			   nav.glideSlopeDev,
			   nav.IsDME(),
			   nav.dme,
			   nav.IsSelected(),
			   nav.IsInop());

			if(0==navId && YSTRUE==instPanel->HasHSI())
			{
				instPanel->DrawHsi3d(
					inst.heading,
					(YSRESULT)nav.IsInRange(),
					nav.vorId,
					nav.IsTuned(),
					nav.IsILS(),
					nav.toFrom,
					nav.obs,
					nav.lateralDev,
					nav.glideSlopeDev,
					nav.IsDME(),
					nav.dme,
					nav.IsSelected(),
					YSTRUE,
					inst.headingBug,
					inst.headingBugSelected,
					nav.IsInop());
			}
		}

		{
			const FsADFIndication &adf=cockpitIndicationSet.adf[0];

			instPanel->DrawAdf3d(
			    (YSRESULT)adf.IsInRange(),
			    adf.ndbId,
			    adf.IsTuned(),
			    0.0,
			    adf.bearing,
			    adf.IsSelected(),
			    adf.IsInop());
		}

		instPanel->EndDraw3d();
	}
}

void FsSimulation::SimDrawHud3d(const YsVec3 &fakeViewPos,const YsAtt3 &instViewAtt,const FsCockpitIndicationSet &cockpitIndicationSet) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		const FsInstrumentIndication &inst=cockpitIndicationSet.inst;

		// HUD2 >>
		hud2->BeginDrawHud(fakeViewPos,playerPlane->GetAttitude());
		{
			YsAtt3 indicatedAttitude(
			    cockpitIndicationSet.inst.heading,
			    cockpitIndicationSet.inst.pitch,
			    cockpitIndicationSet.inst.bank);
			hud2->DrawAttitude(
			    1.0,0.20,0.03,0.05,
			    fakeViewPos,playerPlane->GetAttitude(),indicatedAttitude,fakeViewPos,instViewAtt);

			hud2->DrawCrossHair(0.2,0.1,0.03);

			hud2->DrawThrottle(-0.92,-0.6,0.05,0.38,inst.nEngine,inst.engineOutput,inst.afterBurner);

			hud2->DrawFuelLeft(-0.85,-0.6,0.05,0.38,inst.fuelRemain[0],inst.fuelCapacity[0]);

			if(YSTRUE==inst.hasVectorThrust)
			{
				hud2->DrawNozzle(-0.79,-0.6,0.05,0.38,inst.nozzleDirection);
			}

			hud2->DrawBank(inst.bank);
			{
				const FsVORIndication &nav=cockpitIndicationSet.nav[0];
				hud2->DrawHSI(0.0,-0.5, 0.25,
				    inst.heading,
				    (YSRESULT)nav.IsInRange(),
				    nav.vorId,
				    nav.IsTuned(),
				    nav.IsILS(),
				    nav.toFrom,
				    nav.obs,
				    nav.lateralDev,
				    nav.glideSlopeDev,
				    nav.IsDME(),
				    nav.dme,
				    nav.IsSelected(),
				    YSTRUE,
				    inst.headingBug,
				    inst.headingBugSelected,
				    nav.IsInop());
			}

			hud2->DrawAltitude(0.6,-0.2,0.25,0.8,inst.altitude);
			hud2->DrawVSI(0.85,-0.1,0.2,0.6,YsUnitConv::MtoFT(inst.verticalSpeed*60.0));
			hud2->DrawAirSpeed(-0.85,-0.2,0.25,0.8,inst.airSpeed);

			if(YSTRUE==inst.hasRetractableGear)
			{
				hud2->DrawGear(0.6,-0.30,0.25,0.05,inst.gearPos);
			}
			hud2->DrawBrake(0.6,-0.37,0.25,0.05,inst.brake);
			hud2->DrawFlap(0.87,-0.6,0.05,0.38,inst.flaps);
			if(YSTRUE==inst.hasSpoiler)
			{
				hud2->DrawSpoiler(0.94,-0.6,0.05,0.38,inst.spoiler);
			}

			if(YSTRUE==inst.autoPilot)
			{
				if(long(currentTime*2.0)%2==0)
				{
					hud2->DrawAutoPilot(-0.6,-0.7,0.2,0.08,YSTRUE);
				}
			}

			hud2->DrawControlSurface(
			    -0.6,-0.5,0.2,0.2,
				inst.elevator,
				inst.elevatorTrim,
				inst.aileron,
				inst.rudder);

			hud2->DrawElevatorTrim(-1.0,-0.7,0.05,0.60,inst.elevatorTrim);

			hud2->DrawMachAndG(-1.05,0.2-0.02,0.02,0.03,inst.mach,inst.gForce);

			hud2->DrawTurnAndSlipIndicator(-1.25,0.4,0.25,inst.sideSlip,inst.turnRate);


			YsArray <int,64> loading;
			playerPlane->Prop().GetWeaponConfig(loading);

			int gun=playerPlane->Prop().GetNumWeapon(FSWEAPON_GUN)+playerPlane->Prop().GetNumPilotControlledTurretBullet();
			if(0<gun)
			{
				loading.Append(FSWEAPON_GUN);
				loading.Append(gun);
			}
			double smokeOil=playerPlane->Prop().GetSmokeOil();
			if(0.0<smokeOil)
			{
				loading.Append(FSWEAPON_SMOKE);
				loading.Append((int)smokeOil);
			}
			hud2->DrawAmmo(-0.9,0.9,0.025,0.04,cockpitIndicationSet.ammo);


			double ix=1.25;
			double iy=0.4;;
			double irad=0.25;

			for(int navId=1; navId<2; navId++)
			{
				const FsVORIndication &nav=cockpitIndicationSet.nav[navId];
				if(YSTRUE==nav.IsInRange() || YSTRUE==nav.IsTuned())
				{
					hud2->DrawNav(
					   ix,iy,irad,
					   navId,
					   (YSRESULT)nav.IsInRange(),
					   nav.vorId,
					   nav.IsTuned(),
					   nav.IsILS(),
					   nav.toFrom,
					   nav.obs,
					   nav.lateralDev,
					   nav.glideSlopeDev,
					   nav.IsDME(),
					   nav.dme,
					   nav.IsSelected(),
					   nav.IsInop());
				}

				iy-=irad*2.0;
			}
			
			{
				const FsADFIndication &adf=cockpitIndicationSet.adf[0];
				if(YSTRUE==adf.IsInRange() || YSTRUE==adf.IsTuned())
				{
					hud2->DrawAdf(
					   ix,iy,irad,
					   (YSRESULT)adf.IsInRange(),
					   adf.ndbId,
					   adf.IsTuned(),
					   0.0,
					   adf.bearing,
					   adf.IsSelected(),
					   adf.IsInop());
				}
			}
		}
		hud2->EndDrawHud();
		// HUD2 <<
	}

	const FsGround *playerGround=GetPlayerGround();
	if(NULL!=playerGround)
	{
		hud2->BeginDrawHud(fakeViewPos,instViewAtt);

		hud2->DrawCrossHair(0.2,0.1,0.03);

		YsArray <int,64> loading;
		playerGround->Prop().GetWeaponConfig(loading);
		hud2->DrawAmmo(-0.9,0.9,0.025,0.04,cockpitIndicationSet.ammo);

		hud2->EndDrawHud();
	}
}

void FsSimulation::SimDraw2dVor1(const class FsCockpitIndicationSet &cockpitIndicationSet) const
{
	int sx,sy;
	FsGetWindowSize(sx,sy);

	const int outRad=sx/12;
	const int inRad=outRad*9/10;
	const int cx=sx-outRad;
	const int cy=20+(sx/5)+outRad;

	const FsVORIndication &nav=cockpitIndicationSet.nav[0];
	if(YSTRUE==nav.IsInRange())
	{
		if(YSTRUE==nav.IsILS())
		{
			FsILS::DrawOverlay(YsPi*2.0-nav.obs,nav.lateralDev,nav.glideSlopeDev,hud->hudCol);
		}

		FsHeadUpDisplay::DrawCircularFrame(cx,cy,outRad,hud->hudCol);
		FsHeadUpDisplay::DrawIls(
		    cx,cy,outRad,inRad,hud->hudCol,
		    nav.IsILS(),
		    nav.lateralDev,nav.glideSlopeDev,nav.obs,nav.vorId,nav.toFrom,
			nav.IsDME(),nav.dme,
			nav.IsSelected());
	}
	else if(YSTRUE==nav.IsTuned())
	{
		FsHeadUpDisplay::DrawCircularFrame(cx,cy,outRad,hud->hudCol);
		FsHeadUpDisplay::DrawIls(
		    cx,cy,outRad,inRad,hud->hudCol,
		    YSFALSE, // isIls
		    0.0,0.0,nav.obs,nav.vorId,0,
			nav.IsDME(),nav.dme,
		    nav.IsSelected());
	}
}

void FsSimulation::SimDraw2dVor2(const class FsCockpitIndicationSet &cockpitIndicationSet) const
{
	int sx,sy;
	FsGetWindowSize(sx,sy);

	const int outRad=sx/12;
	const int inRad=outRad*9/10;
	const int cx=sx-outRad;
	const int cy=20+(sx/5)+outRad*3;

	const FsVORIndication &nav=cockpitIndicationSet.nav[1];
	if(YSTRUE==nav.IsInRange())
	{
		FsHeadUpDisplay::DrawCircularFrame(cx,cy,outRad,hud->hudCol);
		FsHeadUpDisplay::DrawVor(
		    cx,cy,outRad,inRad,hud->hudCol,
		    nav.IsILS(),nav.lateralDev,nav.obs,nav.vorId,nav.toFrom,
		    nav.IsDME(),nav.dme,nav.IsSelected());
	}
	else if(YSTRUE==nav.IsTuned())
	{
		FsHeadUpDisplay::DrawCircularFrame(cx,cy,outRad,hud->hudCol);
		FsHeadUpDisplay::DrawVor(
		    cx,cy,outRad,inRad,hud->hudCol,
		    YSFALSE, // isIls
		    0.0,nav.obs,nav.vorId,0,
		    nav.IsDME(),nav.dme,nav.IsSelected());
	}
}

void FsSimulation::SimDraw2dAdf(const class FsCockpitIndicationSet &cockpitIndicationSet) const
{
	int sx,sy;
	FsGetWindowSize(sx,sy);

	const int outRad=sx/12;
	const int cx=sx-outRad;
	const int cy=20+(sx/5)+outRad*5;

	const FsADFIndication &adf=cockpitIndicationSet.adf[0];
	if(YSTRUE==adf.IsInRange())
	{
		FsHeadUpDisplay::DrawCircularFrame(cx,cy,outRad,hud->hudCol);
		FsHeadUpDisplay::DrawAdf(cx,cy,outRad,hud->hudCol,-adf.bearing,adf.ndbId,adf.IsSelected());
	}
	else if(adf.IsTuned()==YSTRUE)
	{
		FsHeadUpDisplay::DrawCircularFrame(cx,cy,outRad,hud->hudCol);
		FsHeadUpDisplay::DrawAdf(cx,cy,outRad,hud->hudCol,-adf.bearing,adf.ndbId,adf.IsSelected());
	}
}

YSRESULT FsSimulation::GetAdfIndication(
    YsString &adfId,
    YSBOOL &tuned,
    double &bearing) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		YsVec3 pos;
		YsAtt3 att;
		FsGround *ndb;

		pos=playerPlane->GetPosition();
		att=playerPlane->GetAttitude();

		if((ndb=FindGround(playerPlane->Prop().GetNdbStationKey()))!=NULL)
		{
			YsVec3 dif;

			dif=ndb->GetPosition()-pos;
			att.MulInverse(dif,dif);

			adfId.Set(ndb->name);
			tuned=YSTRUE;
			bearing=atan2(-dif.x(),dif.z());
			return YSOK;
		}
		else
		{
			adfId.Set("");
			tuned=YSFALSE;
			bearing=0.0;
		}
	}
	return YSERR;
}

YSRESULT FsSimulation::GetVorIlsIndication(
    int navId,
    YsString &vorId,
    YSBOOL &tuned,
    YSBOOL &isIls,int &toFrom,double &course,double &lateral,double &vertical,
    YSBOOL &isDme,double &dme) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL)
	{
		YsVec3 pos;
		FsGround *vor1;
		const FsAircraftCarrierProperty *prop;
		const FsILS *recvILS;

		// pos=playerPlane->GetPosition();
		playerPlane->Prop().GetTransformedRearGearPosition(pos);

		if((vor1=FindGround(playerPlane->Prop().GetVorStationKey(navId)))!=NULL)
		{
			tuned=YSTRUE;
			vorId.Set(vor1->name);

			course=playerPlane->Prop().GetVorObs(navId);
			course=InternalHeadingToMagnetic(course);
			course=TrueHeadingToInternalHeading(course);

			if((prop=vor1->Prop().GetAircraftCarrierProperty())!=NULL && (recvILS=&prop->GetILS())!=NULL)
			{
			    if(recvILS->IsInRange(pos)==YSTRUE)
			    {
					isIls=YSTRUE;
					recvILS->GetDeviation(lateral,vertical,pos);
					toFrom=0;

					isDme=vor1->Prop().chIsDme;
					dme=(vor1->GetPosition()-pos).GetLength();

					return YSOK;
				}
			}
			else
			{
				isIls=YSFALSE;
				FsVor::GetDeviation(toFrom,lateral,playerPlane->GetPosition(),vor1->GetPosition(),InternalHeadingToTrueHeading(playerPlane->Prop().GetVorObs(navId)));
				vertical=0.0;

				isDme=vor1->Prop().chIsDme;
				dme=(vor1->GetPosition()-pos).GetLength();

				return YSOK;
			}
		}
		else
		{
			tuned=YSFALSE;
			vorId.Set("");
			isIls=YSFALSE;
			toFrom=0;
			lateral=0.0;
			vertical=0.0;
			isDme=YSFALSE;
			dme=0.0;
		}
	}
	toFrom=0;
	return YSERR;
}

void FsSimulation::SimDrawFlush(void) const
{
	FsSwapBuffers();
	FsPlugInCallWindowBufferSwapped(currentTime,this);
}

void FsSimulation::SimDrawContainer(const ActualViewMode &actualViewMode) const
{
	auto &viewPoint=actualViewMode.viewPoint;
	auto &viewAttitude=actualViewMode.viewAttitude;

	const YsVec3 *trg;
	YsMatrix4x4 mat;
	double spd;
	YsString caption;
	FsAirplane *air;
	double showUserNameDistSq;

	if(cfgPtr->showUserName==2 || showUserNameMasterSwitch!=YSTRUE)  // 2005/09/14
	{
		return;
	}
	if(cfgPtr->showUserName>=4)
	{
		showUserNameDistSq=(double)cfgPtr->showUserName;
		showUserNameDistSq*=showUserNameDistSq;
	}
	else
	{
		showUserNameDistSq=YsInfinity;
	}

	mat.Translate(viewPoint);
	mat.Rotate(viewAttitude);
	mat.Invert();

	const FsExistence *playerObj=GetPlayerObject();
	if(playerObj!=NULL)
	{
		air=NULL;
		while((air=FindNextAirplane(air))!=NULL)
		{
			if(air->IsAlive()==YSTRUE && IsPlayerAirplane(air)!=YSTRUE)
			{
				trg=&air->GetPosition();

				if(cfgPtr->showUserName>=4 && (*trg-viewPoint).GetSquareLength()>showUserNameDistSq)
				{
					continue;
				}


				double altLimit;
				altLimit=cfgPtr->radarAltitudeLimit+1000.0*(1.0-air->Prop().GetRadarCrossSection());
				if((*trg-viewPoint).GetSquareLength()>=1000.0*1000.0 && trg->y()<altLimit)
				{
					continue;
				}

				spd=air->Prop().GetVelocity();
				if(FSEX_AIRPLANE==playerObj->GetType())
				{
					spd=spd-((FsAirplane *)playerObj)->Prop().GetVelocity();
				}
				spd=spd*3600.0/1800.0;

				const char *spdFmt=(10.0>fabs(spd) ? "%+.1lfkt" : "%+.0lfkt");
				YsString spdTxt;
				spdTxt.Printf(spdFmt,spd);

				if(air->name[0]==0)
				{
					caption=spdTxt;
				}
				else
				{
					if(16>=air->name.Strlen())
					{
						caption.Printf("[%s]%s",(const char *)air->name,(const char *)spdTxt);
					}
					else
					{
						YsString tmpName;
						tmpName=air->name;
						tmpName.SetLength(16);
						tmpName.Append("...");
						caption.Printf("[%s]%s",(const char *)tmpName,(const char *)spdTxt);
					}
				}
				if(air->iff!=playerObj->iff)
				{
					hud->DrawCircleContainer(mat,viewAttitude,*trg,*trg,hud->hudCol,caption,air->actualIdName,YSFALSE,0,30);
				}
				else
				{
					hud->DrawCircleContainer(mat,viewAttitude,*trg,*trg,YsWhite(),caption,air->actualIdName,YSFALSE,0,30);
				}

				FSWEAPONTYPE woc=FSWEAPON_NULL;
				YSHASHKEY airTargetKey=YSNULLHASHKEY;
				double range=0.0;
				switch(playerObj->GetType())
				{
				case FSEX_AIRPLANE:
					woc=((FsAirplane *)playerObj)->Prop().GetWeaponOfChoice();
					airTargetKey=((FsAirplane *)playerObj)->Prop().GetAirTargetKey();
					range=((FsAirplane *)playerObj)->Prop().GetAAMRange(((FsAirplane *)playerObj)->Prop().GetWeaponOfChoice());
					break;
				case FSEX_GROUND:
					woc=((FsGround *)playerObj)->Prop().GetWeaponOfChoice();
					airTargetKey=((FsGround *)playerObj)->Prop().GetAirTargetKey();
					range=((FsGround *)playerObj)->Prop().GetSAMRange();
					break;
				}

				if((woc==FSWEAPON_AIM9 ||
				    woc==FSWEAPON_AIM9X ||
				    woc==FSWEAPON_AIM120) &&
				    air->SearchKey()==airTargetKey)
				{
					double distance;
					distance=(air->GetPosition()-playerObj->GetPosition()).GetLength();

					if(distance<range/2.0)
					{
						const YsColor *col;
						col=(int(currentTime*2.0)&1 ? &YsRed() : &hud->hudCol);
						hud->DrawCircleContainer(mat,viewAttitude,*trg,*trg,*col,"",NULL,YSFALSE,90,90);
					}
					else
					{
						hud->DrawCircleContainer(mat,viewAttitude,*trg,*trg,YsYellow(),"",NULL,YSFALSE,90,90);
					}
				}
			}
		}

		if(FSEX_AIRPLANE==playerObj->GetType())
		{
			const FsAirplane *playerPlane=(const FsAirplane *)playerObj;

			FsGround *gnd;
			gnd=FindGround(playerPlane->Prop().GetGroundTargetKey());
			if(gnd!=NULL && playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_AGM65)
			{
				double distance;
				distance=(gnd->GetPosition()-playerPlane->GetPosition()).GetLength();
				if(distance<playerPlane->Prop().GetAGMRange())
				{
					const YsColor *col;
					col=(int(currentTime*2.0)&1 ? &YsRed() : &hud->hudCol);
					hud->DrawCircleContainer
					    (mat,viewAttitude,gnd->GetPosition(),gnd->GetPosition(),*col,"SHOOT",NULL,YSFALSE,45,90);
				}
				else
				{
					hud->DrawCircleContainer
					    (mat,viewAttitude,gnd->GetPosition(),gnd->GetPosition(),YsYellow(),"",NULL,YSFALSE,45,90);
				}
			}
		}

		for(FsGround *gnd=NULL; NULL!=(gnd=FindNextGround((FsGround *)gnd)); )
		{
			if(gnd->IsAlive()==YSTRUE && playerObj->iff!=gnd->iff && gnd->Prop().IsNonGameObject()!=YSTRUE)
			{
				if((gnd->GetPosition()-playerObj->GetPosition()).GetSquareLength()<YsSqr(5000.0))
				{
					hud->DrawCrossDesignator
					    (mat,viewAttitude,gnd->GetPosition(),gnd->GetPosition(),hud->hudCol,YSFALSE);
				}
			}
		}
	}
}

void FsSimulation::SimDrawGunAim(void) const
{
	const FsExistence *playerObj=GetPlayerObject();
	YSBOOL hasLeadGunSight=(NULL!=GetPlayerAirplane() ? GetPlayerAirplane()->Prop().GetLeadGunSight() : YSTRUE);

	if(playerObj!=NULL &&
	   playerObj->GetWeaponOfChoice()==FSWEAPON_GUN &&
	   YSTRUE==hasLeadGunSight)
	{
		const FsAirplane *target;
		YsVec3 aim;
		if(SimCalculateGunAim(target,aim)==YSOK)
		{
			YsVec3 pos;
			YsAtt3 att;
			YsMatrix4x4 mat;

			if(FSEX_GROUND==playerObj->GetType())
			{
				const FsGround *playerGround=(const FsGround *)playerObj;
				pos=playerObj->GetPosition();
				att=playerGround->Prop().GetAaaAim();
				mat.Translate(pos);
				mat.Rotate(att);
				mat.Invert();
			}
			else
			{
				pos=playerObj->GetPosition();
				att=playerObj->GetAttitude();
				mat=playerObj->GetInverseMatrix();
			}

			hud->DrawCircleContainer(mat,att,aim,target->GetPosition(),YsRed(),"",NULL,YSTRUE,0,30);
		}
	}

	const FsAirplane *playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL && 
	   playerPlane->Prop().GetHasPilotControlledTurret()==YSTRUE)
	{
		YsVec3 dir,aim,cock;
		if(playerPlane->Prop().GetFirstPilotControlledTurretDirection(dir)==YSOK)
		{
			YsVec3 pos;
			YsAtt3 att;
			YsMatrix4x4 mat;

			pos=playerPlane->Prop().GetPosition();
			att=playerPlane->Prop().GetAttitude();
			mat=playerPlane->Prop().GetInverseMatrix();

			playerPlane->Prop().GetCockpitPosition(cock);
			playerPlane->Prop().GetMatrix().Mul(cock,cock,1.0);

			aim=cock+dir*100.0;
			hud->DrawCrossDesignator2(mat,att,aim,aim,hud->hudCol,YSFALSE);
		}
	}
}

YSRESULT FsSimulation::SimCalculateGunAim(const FsAirplane *&target,YsVec3 &aim) const
{
	YsVec3 pos;
	YsAtt3 att;
	YsMatrix4x4 mat;
	const FsExistence *playerObj;
	const FsAirplane *air;

	playerObj=GetPlayerObject();
	if(NULL!=playerObj && playerObj->GetWeaponOfChoice()==FSWEAPON_GUN)
	{
		if(FSEX_GROUND==playerObj->GetType())
		{
			const FsGround *playerGround=(const FsGround *)playerObj;
			pos=playerObj->GetPosition();
			att=playerGround->Prop().GetAaaAim();
			mat.Translate(pos);
			mat.Rotate(att);
			mat.Invert();
		}
		else
		{
			pos=playerObj->GetPosition();
			att=playerObj->GetAttitude();
			mat=playerObj->GetInverseMatrix();
		}

		double targetZ;

		target=NULL;
		targetZ=100000.0;
		air=NULL;
		while((air=FindNextAirplane(air))!=NULL)
		{
			if(air->IsAlive()==YSTRUE && IsPlayerAirplane(air)!=YSTRUE)
			{
				YsVec3 trg;

				trg=mat*(air->GetPosition());

				if(YsTolerance<trg.z() &&
				  atan(YsAbs(trg.x())/trg.z())<YsDegToRad(30) &&
				  atan(YsAbs(trg.y())/trg.z())<YsDegToRad(30) &&
				  trg.z()<targetZ &&
				  trg.z()<990.0)
				{
					target=air;
					targetZ=trg.z();
				}
			}
		}


		if(target!=NULL)
		{
			YsVec3 rTrg,tVec,tRVec;
			double bulSpeed=100.0;

			const YsVec3 trg=target->Prop().GetPosition();
			target->Prop().GetVelocity(tVec);
			if(FSEX_GROUND==playerObj->GetType())
			{
				bulSpeed=((const FsGround *)playerObj)->Prop().GetBulletSpeed();
			}
			else if(FSEX_AIRPLANE==playerObj->GetType())
			{
				bulSpeed=((const FsAirplane *)playerObj)->Prop().GetBulletSpeed();
			}

			mat.Mul(tRVec,tVec,0.0);
			rTrg=mat*trg;

			double tEstimated;
			tEstimated=rTrg.z()/(bulSpeed-tRVec.z());


			YsVec3 bVec;
			bVec.Set(0.0,0.0,bulSpeed*tEstimated);
			att.Mul(bVec,bVec);
			aim=pos+bVec-tVec*tEstimated;
			aim.SetY(aim.y()-0.5*FsGravityConst*tEstimated*tEstimated);

			return YSOK;
		}
	}
	return YSERR;
}

void FsSimulation::SimDrawBombingAim(const ActualViewMode &actualViewMode) const
{
	auto &viewPoint=actualViewMode.viewPoint;
	auto &viewAttitude=actualViewMode.viewAttitude;

	const YsMatrix4x4 *mat;
	const FsAirplane *playerPlane;

	playerPlane=GetPlayerAirplane();
	mat=&playerPlane->Prop().GetInverseMatrix();

	if(playerPlane!=NULL &&
	   (playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_BOMB ||
	    playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_BOMB250) &&
	   ((fabs(userInput.viewHdg)< YsDegToRad( 10.0) && fabs(userInput.viewPch)< YsDegToRad( 10.0)) ||
	    actualViewMode.actualViewMode==FSBOMBINGVIEW))
	{
		YsVec3 estimated;
		if(playerPlane->Prop().ComputeEstimatedBombLandingPosition(estimated,GetWeather())==YSOK)
		{
			YsVec3 root;
			root.Set(0.0,0.0,10.0);

			playerPlane->Prop().GetAttitude().Mul(root,root);

			root+=viewPoint;

			hud->DrawCircleContainer(*mat,viewAttitude,estimated,root,hud->hudCol,"BOMB",NULL,YSTRUE,0,30);
		}
	}
}

void FsSimulation::SimBlastSound(YSBOOL demoMode)
{
	FsAirplane *playerPlane;
	playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL && playerPlane->IsAlive()==YSTRUE)
	{
		double thr=playerPlane->Prop().GetThrottle();
		{
			double rpmMin,rpmMax;
			if(YSOK==playerPlane->Prop().GetRPMRangeForSoundEffect(rpmMin,rpmMax,0) &&
			   YsTolerance<rpmMax-rpmMin)
			{
				double rpm=playerPlane->Prop().GetRealPropRPM(0);
				thr=YsBound((rpm-rpmMin)/(rpmMax-rpmMin),0.0,1.0);
			}
		}

		FsSoundSetVehicleName(playerPlane->Prop().GetIdentifier());

		YSBOOL isInVTOLMode=YSFALSE;
		if(YSTRUE==playerPlane->Prop().GetHasThrustVectoring())
		{
			YsVec3 dir;
			playerPlane->Prop().GetThrustDirection(dir);
			if(dir.y()>fabs(dir.x()) && dir.y()>fabs(dir.z()))
			{
				isInVTOLMode=YSTRUE;
			}
		}

		if(bulletHolder.IsLockedOn(playerPlane)==YSTRUE)
		{
			FsSoundSetAlarm(FSSND_ALARM_MISSILE);
		}
		else if(YSTRUE!=demoMode &&
		        YSTRUE!=isInVTOLMode &&
		        FSCL_HELICOPTER!=playerPlane->Prop().GetAircraftClass() &&
		        playerPlane->Prop().HasStallHorn()==YSTRUE &&
		        playerPlane->Prop().GetAOA()>playerPlane->Prop().GetCriticalAOA()-YsPi/60.0)
		{
			FsSoundSetAlarm(FSSND_ALARM_STALL);
		}
		else if(YSTRUE!=demoMode &&
		        playerPlane->Prop().HasGearHorn()==YSTRUE &&
		        playerPlane->Prop().GetAGL()<152.4 &&        // Less than 500ft
		        playerPlane->Prop().GetLandingGear()<0.99 && // Gear is not down and locked
		        0.0>playerPlane->Prop().GetClimbRatio() &&   // Descending
		       (playerPlane->Prop().GetThrottle()<0.2 ||     // Less than 20% power
		        playerPlane->Prop().GetFlap()>0.60 ||        // Flap down more than 60%
		        playerPlane->Prop().GetVelocity()<playerPlane->Prop().GetEstimatedLandingSpeed()*1.1))  // Slower than 110% of landing speed
		{
			FsSoundSetAlarm(FSSND_ALARM_TERRAIN);
		}
		else
		{
			FsSoundSetAlarm(FSSND_ALARM_SILENT);
		}

		if(playerPlane->Prop().IsFiringGun()==YSTRUE || playerPlane->Prop().IsFiringPilotControlledTurret()==YSTRUE)
		{
			FsSoundSetMachineGun(FSSND_MACHINEGUN_MACHINEGUN);
		}
		else
		{
			FsSoundSetMachineGun(FSSND_MACHINEGUN_SILENT);
		}


		if(demoMode!=YSTRUE)
		{
			if(playerPlane->Prop().IsJet()==YSTRUE)
			{
				YSBOOL ab;
				ab=playerPlane->Prop().GetAfterBurner();
				if(ab==YSTRUE)
				{
					FsSoundSetEngine(FSSND_ENGINE_JETAFTERBURNER,1,thr);
				}
				else
				{
					FsSoundSetEngine(FSSND_ENGINE_JETNORMAL,1,thr);
				}
			}
			else
			{
				FsSoundSetEngine(FSSND_ENGINE_PROPELLER,1,thr);
			}
		}
		else
		{
			FsSoundSetEngine(FSSND_ENGINE_JETNORMAL,1,0.0);
		}
	}
	else
	{
		FsSoundSetEngine(FSSND_ENGINE_SILENT,0,0.0);
		FsSoundSetMachineGun(FSSND_MACHINEGUN_SILENT);
		FsSoundSetAlarm(FSSND_ALARM_SILENT);
	}

	FsSoundKeepPlaying();
}

void FsSimulation::SimRecordAir(const double &t,YSBOOL forceRecord)
{
	FsAirplane *airplane;
	airplane=NULL;
	while((airplane=FindNextAirplane(airplane))!=NULL)
	{
		if(forceRecord==YSTRUE && airplane->IsAlive()!=YSTRUE)
		{
			continue;
		}
		if(airplane->isPlayingRecord==YSFALSE)
		{
			airplane->Record(t,forceRecord);
		}
	}
}

void FsSimulation::SimRecordGnd(const double &t,YSBOOL forceRecord)
{
	FsGround *ground;
	ground=NULL;
	while((ground=FindNextGround(ground))!=NULL)
	{
		if(forceRecord==YSTRUE && ground->IsAlive()!=YSTRUE)
		{
			continue;
		}

		if(ground->isPlayingRecord==YSFALSE)
		{
			ground->Record(t,forceRecord);
		}
	}
}

void FsSimulation::SimPlayTimedEvent(const double &ctime)
{
	int i;
	YsVec3 windVec;
	FsAirplane *air;
	for(i=simEvent->nextEvent; i<simEvent->eventList.GetN() && simEvent->eventList[i].eventTime<ctime; i++)
	{
		if((simEvent->eventList[i].eventFlag&FSEVENTFLAG_UNSORTED)==0)
		{
			switch(simEvent->eventList[i].eventType)
			{
			case FSEVENT_TEXTMESSAGE:
				AddTimedMessageWithNoLog(simEvent->eventList[i].str);
				break;
			case FSEVENT_PLAYEROBJCHANGE:
				{
					const FsExistence *newPlayer=FindObject(simEvent->eventList[i].objKey);
					if(NULL!=newPlayer)
					{
						SetPlayerObject(newPlayer,YSFALSE);  // <- YSFALSE: Should not record.
					}
				}
				break;
			case FSEVENT_AIRCOMMAND:
				air=FindAirplane(simEvent->eventList[i].objKey);
				if(air!=NULL)
				{
					air->SendCommand(simEvent->eventList[i].str);
				}
				break;
			case FSEVENT_SETWEAPONCONFIG:
				air=FindAirplane(simEvent->eventList[i].objKey);
				if(air!=NULL)
				{
					air->Prop().ApplyWeaponConfig(simEvent->eventList[i].weaponCfg.GetN(),simEvent->eventList[i].weaponCfg);
				}
				break;
			case FSEVENT_ENVIRONMENTCHANGE:
				SetEnvironment(simEvent->eventList[i].env);
				break;
			case FSEVENT_VISIBILITYCHANGE:
				weather->SetTransFogVisibility(simEvent->eventList[i].visibility);
				break;
			case FSEVENT_WINDCHANGE:
				weather->SetTransWind(simEvent->eventList[i].wind);
				break;
			case FSEVENT_CLOUDLAYERCHANGE:
				weather->SetCloudLayer(simEvent->eventList[i].cloudLayer.GetN(),simEvent->eventList[i].cloudLayer);
				break;
			}
		}
	}
	simEvent->nextEvent=i;
}

void FsSimulation::SimProcessAirTrafficController(void)
{
	primaryAtc.Process(this,currentTime);
	if(YSTRUE==primaryAtc.GetAndClearNeedNoticeFlag())
	{
		FsSoundSetOneTime(FSSND_ONETIME_NOTICE);
	}
}

void FsSimulation::SimInFlightDialogTransition(void)
{
	if(0.5>currentTime)
	{
		// When objects are loaded from .YFS file, objects may not be put in lattice yet.
		// Wait for at least a few frames to make sure they are in.
		// After that, it should be fine.
		return;
	}

	if(NULL!=GetCurrentInFlightDialog())
	{
		GetCurrentInFlightDialog()->UpdateDialog();
	}

	if(NULL==GetCurrentInFlightDialog() || stationaryDlg==GetCurrentInFlightDialog())
	{
		FsGuiInFlightDialog *nextInFltDlg=GetCurrentInFlightDialog();

		const FsAirplane *playerAir=GetPlayerAirplane();
		if(NULL!=playerAir)
		{
			if(playerAir->Prop().GetFlightState()==FSGROUNDSTATIC && 0.1>userInput.ctlThrottle)
			{
				nextInFltDlg=stationaryDlg;
			}
			else
			{
				nextInFltDlg=NULL;
			}
		}
		const FsGround *playerGnd=GetPlayerGround();
		if(NULL!=playerGnd)
		{
			if(/* 0.9<userInput.ctlBrake && */ // Probably this condtion will be just confusing.
			   YsTolerance>playerGnd->Prop().GetVelocity() &&
			   0.1>userInput.ctlThrottle)
			{
				nextInFltDlg=stationaryDlg;
			}
			else
			{
				nextInFltDlg=NULL;
			}
		}

		if(nextInFltDlg!=GetCurrentInFlightDialog())
		{
			YSBOOL canSupply=YSFALSE,canChangeVehicle=YSFALSE;

			if(NULL!=GetPlayerAirplane())
			{
				YSBOOL fuel,ammo;
				if(NULL!=FindNearbySupplyTruck(fuel,ammo,*playerAir))
				{
					canSupply=YSTRUE;
				}
			}

			if(NULL!=GetPlayerObject())
			{
				YsArray <const FsExistence *,16> boardable;
				if(YSOK==FindNearbyBoardableVehicle(boardable,*GetPlayerObject()))
				{
					canChangeVehicle=YSTRUE;
				}
			}

			stationaryDlg->SetUp(canSupply,canChangeVehicle);
			SetCurrentInFlightDialog(nextInFltDlg);
		}
	}
}

void FsSimulation::ToggleAutoPilotDialog(void)
{
	auto playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane && YSTRUE==playerPlane->Prop().CheckAutoPilotInop())
	{
		AddTimedMessage("AUTO PILOT MALFUNCTION");
		return;
	}

	auto ifd=GetCurrentInFlightDialog();
	if(ifd==autoPilotDlg || ifd==autoPilotVTOLDlg || ifd==autoPilotHelicopterDlg)
	{
		SetCurrentInFlightDialog(NULL);
		return;
	}

	if(NULL!=playerPlane && playerPlane->Prop().GetAircraftClass()==FSCL_AIRPLANE)
	{
		if(ifd!=autoPilotDlg && ifd!=autoPilotVTOLDlg)
		{
			YsVec3 thrDir;
			playerPlane->Prop().GetThrustDirection(thrDir);
			if(thrDir.z()>YsTolerance &&
			   fabs(thrDir.x()/thrDir.z())<0.1 &&
			   fabs(thrDir.y()/thrDir.z())<0.1)
			{
				SetCurrentInFlightDialog(autoPilotDlg);
			}
			else
			{
				SetCurrentInFlightDialog(autoPilotVTOLDlg);
			}
		}
	}
	else if(NULL!=playerPlane && playerPlane->Prop().GetAircraftClass()==FSCL_HELICOPTER)
	{
		SetCurrentInFlightDialog(autoPilotHelicopterDlg);
	}
}

void FsSimulation::SetUpRadioCommTargetDialog(void)
{
	auto playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto allCallable=FsGuiRadioCommDialogBase::GetCallable(this,playerPlane);
		if(1==allCallable.GetN() && allCallable[0].callableType==FsGuiRadioCommDialogBase::ATC)
		{
			SetUpAtcRequestDialog();
		}
		else
		{
			radioCommTargetDlg->Make(this,allCallable);
			SetCurrentInFlightDialog(radioCommTargetDlg);
		}
	}
}

void FsSimulation::SetUpCallFuelTruckDialog(void)
{
	callFuelTruckDlg->CacheCallableFuelTruck();
	SetCurrentInFlightDialog(callFuelTruckDlg);
}

void FsSimulation::SetUpRadioCommCommandDialog(YSSIZE_T n,FsAirplane *const air[])
{
	radioCommCmdDlg->SetCommTarget(n,air);
	SetCurrentInFlightDialog(radioCommCmdDlg);
}

void FsSimulation::SetUpRadioCommToFormationDialog(void)
{
	auto playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane && YSTRUE==AirplaneInFormation(playerPlane))
	{
		SetCurrentInFlightDialog(radioCommToFomDlg);
	}
	else
	{
		AddTimedMessage("No Aircrafts in the Formation.");
	}
}

YSBOOL FsSimulation::AirplaneInFormation(FsAirplane *leader)
{
	FsAirplane *air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air!=leader && air->Prop().IsActive()==YSTRUE && air->netType==FSNET_LOCAL)
		{
			auto fm=dynamic_cast <FsFormation *> (air->GetAutopilot());
			if(NULL!=fm && fm->leader==leader)
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

void FsSimulation::SetUpRequestApproachDialog(void)
{
	requestApproachDlg->Reset();
	SetCurrentInFlightDialog(requestApproachDlg);
}

void FsSimulation::SetUpAtcRequestDialog(void)
{
	atcRequestDlg->Make(this);
	SetCurrentInFlightDialog(atcRequestDlg);
}

void FsSimulation::SetIgnoreThisKeyHolding(int rawKey)
{
	// In SimControlByUser,
	//			ctlAssign.processNumberKey=YSFALSE;
	// if submenu is not FSSUBMENU_NONE.  If processNumberKey is YSFALSE, ctlAssign.IsButtonPressed assumes
	// number keys are up regardless of the actual key state.
	// And, the key stroke for selecting menu won't be processed as a key press.

	ctlAssign.SetIgnoreThisKeyHolding(rawKey);

	// subMenu.SetSubMenu(this,FSSUBMENU_WAITKEYRELEASE); // Tentative solution.
}

void FsSimulation::RadioCommSendBreakAndAttack(FsAirplane *target,FsAirplane * /*sender*/)
{
	FsDogfight *df;
	df=FsDogfight::Create();
	df->gLimit=target->gLimit;
	df->minAlt=0.0;
	target->SetAutopilot(df);
}

void FsSimulation::RadioCommSendCoverMe(FsAirplane *target,FsAirplane *sender)
{
	FsDogfight *df;
	df=FsDogfight::Create();
	df->gLimit=target->gLimit;
	df->minAlt=0.0;
	df->defendThisAirplaneKey=FsExistence::GetSearchKey(sender);
	target->SetAutopilot(df);
}

void FsSimulation::RadioCommSendAttackGround(FsAirplane *target,FsAirplane * /*sender*/)
{
	FsGroundAttack *ga;
	ga=FsGroundAttack::Create();
	ga->SetTurnAwayAfterWeaponRelease(YSTRUE);
	ga->SetTurnAwayAndFlyToAtLeastThisDistance(2500.0);
	ga->SetAGMReleaseDistance(target->Prop().GetAGMRange()*0.9);
	ga->SetInboundSpeed(target->Prop().GetEstimatedCruiseSpeed()*1.2);
	target->SetAutopilot(ga);
}

void FsSimulation::RadioCommSendFormOnMyWing(FsAirplane *target,FsAirplane *sender,YSBOOL synchroTrigger)
{
	const double safetyMargine=2.0736; // 2014/11/22 Safety Margin.  Now can form on my wing -> tighten the formation

	YsArray <YsVec3,256> occupiedPosition;
	YsArray <double,256> occupiedRadius;
	FsAirplane *air;
	YsVec3 newPosition;
	YSBOOL alreadyInTheFormation;

	alreadyInTheFormation=YSFALSE;

	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air!=sender && air->Prop().IsActive()==YSTRUE && air->netType==FSNET_LOCAL)
		{
			FsAutopilot *ap;
			FsFormation *fm;
			ap=air->GetAutopilot();
			if(ap!=NULL && ap->Type()==FSAUTOPILOT_FORMATION)
			{
				fm=(FsFormation *)ap;
				if(fm->leader==sender)
				{
					if(air!=target)
					{
						occupiedPosition.Append(fm->shouldBe);
						occupiedRadius.Append(air->Prop().GetOutsideRadius()*safetyMargine);
					}
					else
					{
						// Already in the formation.
						alreadyInTheFormation=YSTRUE;
					}
				}
			}
		}
	}


	// Secret feature
	if(alreadyInTheFormation==YSTRUE && occupiedPosition.GetN()==0)
	{
		FsAutopilot *ap;
		ap=sender->GetAutopilot();
		if(ap!=NULL && ap->Type()==FSAUTOPILOT_GOTO)
		{
			FsAutopilot *ap;
			FsFormation *fom;

			ap=target->GetAutopilot();
			if(ap!=NULL && ap->Type()==FSAUTOPILOT_FORMATION)
			{
				fom=(FsFormation *)ap;
				if((target->Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
				    target->Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
				    target->Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
				    target->Prop().GetAirplaneCategory()==FSAC_TRAINER) &&
				    fom->mode==0 &&
				    fom->inverted==YSFALSE)
				{
					int trick;
					trick=rand()%90;
					if(trick<30)
					{
						fom->shouldBe.SetX(0.0);
						fom->shouldBe.SetY(10.0);
						fom->nTransition=1;
						fom->transition[0].Set(0.0,10.0,0.0);
						fom->inverted=YSTRUE;
					}
					else if(trick<60)
					{
						fom->inverted=YSTRUE;
					}
					else
					{
						FsAirshowControl *csw;
						csw=FsAirshowControl::Create();
						csw->action=FsAirshowControl::CORKSCREW;
						csw->cswRadius=60.0;
						csw->cswMode=0;
						csw->leader=GetPlayerAirplane();;
						target->SetAutopilot(csw);
					}
				}
			}
		}
		return;
	}


	// See document/autoformation.doc
	double x,z;
	double rLeader;
	rLeader=sender->Prop().GetOutsideRadius()*safetyMargine;
	if(occupiedPosition.GetN()==0)
	{
		double rWingman;
		rWingman=target->Prop().GetOutsideRadius()*safetyMargine;
		x=-rWingman;
		z=-(rLeader+rWingman);
	}
	else if(occupiedPosition.GetN()==1)
	{
		double rWingman;
		rWingman=target->Prop().GetOutsideRadius()*safetyMargine;
		if(occupiedPosition[0].x()<0.0)
		{
			x=rWingman;
		}
		else
		{
			x=-rWingman;
		}
		z=-(rLeader+rWingman);
	}
	else if(occupiedPosition.GetN()==2)
	{
		double rWingman0,rWingman;
		x=0.0;
		rWingman0=YsGreater(occupiedRadius[0],occupiedRadius[1]);
		rWingman=target->Prop().GetOutsideRadius()*safetyMargine;
		z=-(rLeader+rWingman0*2.0+rWingman);
	}
	else if(occupiedPosition.GetN()==3 || occupiedPosition.GetN()==4)
	{
		int i,slotAirplane;
		double z0,rWingman;
		slotAirplane=0;
		for(i=1; i<occupiedPosition.GetN(); i++)
		{
			double z1,z2;
			z1=occupiedPosition[slotAirplane].z()+occupiedRadius[slotAirplane];
			z2=occupiedPosition[i].z()+occupiedRadius[i];
			if(z2<z1)
			{
				slotAirplane=i;
			}
		}
		z0=occupiedPosition[slotAirplane].z()+occupiedRadius[slotAirplane];

		rWingman=target->Prop().GetOutsideRadius()*safetyMargine;
		z=z0-rWingman;
		x=occupiedRadius[slotAirplane]+rWingman;
		if(occupiedPosition.GetN()==4)
		{
			x=-x;
		}
	}
	else
	{
		int i,slotAirplane;
		double rWingman;

		x=0.0;
		rWingman=target->Prop().GetOutsideRadius()*safetyMargine;

		slotAirplane=0;
		for(i=1; i<occupiedPosition.GetN(); i++)
		{
			double z1,z2;
			z1=occupiedPosition[slotAirplane].z()-occupiedRadius[slotAirplane];
			z2=occupiedPosition[i].z()-occupiedRadius[i];
			if(z2<z1)
			{
				slotAirplane=i;
			}
		}
		z=occupiedPosition[slotAirplane].z()-occupiedRadius[slotAirplane]-rWingman;
	}

	x*=1.1;  // To make delta wider
	z/=1.1;

	double y;
	y=z/5.0;
	newPosition.Set(x,y,z);

	FsFormation *fom;
	fom=FsFormation::Create();
	fom->minAlt=0.0;
	fom->leader=sender;
	fom->shouldBe=newPosition;
	fom->synchronizeTrigger=synchroTrigger;
	target->SetAutopilot(fom);
}

void FsSimulation::RadioCommSendReturnToBase(FsAirplane *target,FsAirplane * /*sender*/)
{
	if(target->GetAutopilot()==NULL || // This condition was missing and added on 2003/01/23
	   target->GetAutopilot()->Type()!=FSAUTOPILOT_LANDING)
	{
		FsLandingAutopilot *ap;
		ap=FsLandingAutopilot::Create();
		target->SetAutopilot(ap);
	}
}

void FsSimulation::RadioCommSendHoldingPattern(FsAirplane *target,FsAirplane * /*sender*/)
{
	FsGotoPosition *gp;
	gp=FsGotoPosition::Create();
	gp->SetSingleDestination(target->GetPosition());
	gp->destination.GetEnd().SetY(1000.0+double(rand()%3)*165.0);
	gp->speed=YsGreater(target->Prop().GetEstimatedLandingSpeed()*1.2,target->Prop().GetVelocity());
	gp->minAlt=0.0;
	gp->straightFlightMode=YSFALSE;
	target->SetAutopilot(gp);
}

void FsSimulation::RadioCommSendLandRefuelAndTakeOff(FsAirplane *target,FsAirplane * /*sender*/)
{
	auto *ap=FsRefuelAndTakeOffAutopilot::Create();
	target->SetAutopilot(ap);
}

void FsSimulation::RadioCommSendSpreadFormation(FsAirplane *sender,const double factor)
{
	FsAirplane *air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air!=sender && air->Prop().IsActive()==YSTRUE && air->netType==FSNET_LOCAL)
		{
			auto fm=dynamic_cast <FsFormation *> (air->GetAutopilot());
			if(NULL!=fm && fm->leader==sender)
			{
				auto newPos=fm->GetShouldBe()*factor;
				fm->SetShouldBe(newPos);
			}
		}
	}
}

void FsSimulation::RadioCommSendTightenFormation(FsAirplane *sender,const double divFactor)
{
	YsArray <YsPair <FsAirplane *,YsVec3>,16> airInFormation;
	GetAircraftInFormation(airInFormation,sender);

	for(auto &air : airInFormation)
	{
		air.b/=divFactor;
	}

	for(int i=0; i<airInFormation.GetN(); ++i)
	{
		for(int j=i+1; j<airInFormation.GetN(); ++j)
		{
			const double d=(airInFormation[i].b-airInFormation[j].b).GetLength();
			const double safeDist=airInFormation[i].a->GetApproximatedCollideRadius()
			                     +airInFormation[j].a->GetApproximatedCollideRadius();
			if(d<safeDist*0.7)
			{
				return; // Too close
			}
		}
	}

	for(auto &fom : airInFormation)
	{
		if(fom.a!=sender)
		{
			auto fm=dynamic_cast <FsFormation *> (fom.a->GetAutopilot());
			if(NULL!=fm && fm->leader==sender)
			{
				fm->SetShouldBe(fom.b);
			}
		}
	}
}

void FsSimulation::GetAircraftInFormation(YsArray <YsPair <FsAirplane *,YsVec3>,16> &airInFormation,FsAirplane *wingLeader) const
{
	airInFormation.CleanUp();

	airInFormation.Increment();
	airInFormation.Last().a=wingLeader;
	airInFormation.Last().b=YsOrigin();

	FsAirplane *air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air!=wingLeader && 
		   (air->airFlag&FSAIRFLAG_INDEPENDENT)==0 &&
		   air->Prop().IsActive()==YSTRUE && 
		   air->netType==FSNET_LOCAL)
		{
			auto fm=dynamic_cast <FsFormation *> (air->GetAutopilot());
			if(NULL!=fm && fm->leader==wingLeader)
			{
				airInFormation.Increment();
				airInFormation.Last().a=air;
				airInFormation.Last().b=fm->GetShouldBe();
			}
		}
	}
}

void FsSimulation::SetUpGunnerSubMenu(void)
{
	auto playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		if(playerPlane->Prop().GetHasGunnerControlledTurret()==YSTRUE)
		{
			subMenu.SetSubMenu(this,FSSUBMENU_GUNNER);
		}
	}
}

void FsSimulation::GetProjection(FsProjection &prj,const ActualViewMode &actualViewMode) const
{
	int wid,hei,fovInPixel;
	const FsAirplane *playerPlane;

	FsGetDrawingAreaSize(wid,hei);

	playerPlane=GetPlayerAirplane();

	if(0!=(GetInstrumentDrawSwitch(actualViewMode)&FSISS_2DHUD))
	{
		prj.cx=wid/2;
		prj.cy=hei*2/3;
	}
	else if(FSCOCKPITVIEW==actualViewMode.actualViewMode && NULL!=playerPlane)
	{
		const YsVec2 scrnCen=playerPlane->Prop().GetScreenCenter();
		prj.cx=(int)((double)wid*(1.0+scrnCen.x())/2.0);
		prj.cy=(int)((double)hei*(1.0-scrnCen.y())/2.0);
	}
	else
	{
		prj.cx=wid/2;
		prj.cy=hei/2;
	}

	fovInPixel=YsGreater(wid/2,hei/2);  // 2010/07/05 It was ...,prj.cx,prj.cy);

	prj.prjMode=YsProjectionTransformation::PERSPECTIVE;
	prj.prjPlnDist=(double)hei/(1.41421356*720.0/960.0);  // 2010/07/05 Fix vertical fov (double)wid/(double)1.41421356;
	prj.prjPlnDist*=(actualViewMode.viewMagFix*viewMagUser/1.8);
	prj.tanFov=(double)fovInPixel/prj.prjPlnDist;
	prj.viewportDim.Set(wid,hei);

	prj.nearz=0.1;
	prj.farz=18000.0;

	prj.UncacheMatrix();
}

void FsSimulation::SetSubWindowViewMode(int windowId,FSVIEWMODE viewMode)
{
	subWindowViewmode[windowId]=viewMode;
}

void FsSimulation::FlipShowUserNameMasterSwitch(void)
{
	YsFlip(showUserNameMasterSwitch);
}

YSBOOL FsSimulation::GetShowUserNameMasterSwitch(void) const
{
	return showUserNameMasterSwitch;
}

void FsSimulation::GetStandardProjection(class FsProjection &prj)
{
	int wid,hei,fovInPixel;
	FsGetWindowSize(wid,hei);

	prj.prjMode=YsProjectionTransformation::PERSPECTIVE;

	prj.cx=wid/2;
	prj.cy=hei/2;
	fovInPixel=YsGreater(prj.cx,prj.cy);

	prj.prjPlnDist=(double)wid/(double)1.41421356;
	prj.tanFov=(double)fovInPixel/prj.prjPlnDist;
	prj.viewportDim.Set(wid,hei);

	prj.nearz=0.1;
	prj.farz=20000.0;

	prj.UncacheMatrix();
}

double FsSimulation::GetFirstRecordTime(void)
{
	YSBOOL first;
	double t0,t;
	FsAirplane *air;

	t0=0.0;
	first=YSTRUE;

	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->isPlayingRecord==YSTRUE &&
		   air->rec->GetTopElement(t)!=NULL &&
		   (first==YSTRUE || t<t0))
		{
			t0=t;
			first=YSFALSE;
		}
	}

	return t0;
}

double FsSimulation::GetLastRecordTime(void)
{
	double t0,t;
	FsAirplane *air;

	t0=0.0;

	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->isPlayingRecord==YSTRUE &&
		   air->rec->GetLastElement(t)!=NULL &&
		   t0<t)
		{
			t0=t;
		}
	}

	return t0;
}

YSBOOL FsSimulation::IsLockedOn(const FsExistence *ex) const
{
	if(NULL!=ex)
	{
		FsAirplane *airplane;
		airplane=NULL;
		while((airplane=FindNextAirplane(airplane))!=NULL)
		{
			if(airplane->IsAlive()==YSTRUE &&
			   airplane->Prop().GetAirTargetKey()==FsExistence::GetSearchKey(ex))
			{
				return YSTRUE;
			}
		}
		// Ground
	}

	return YSFALSE;
}

YSBOOL FsSimulation::IsMissileChasing(FSWEAPONTYPE &wpnType,YsVec3 &wpnPos,const FsExistence *ex) const
{
	return bulletHolder.IsLockedOn(wpnType,wpnPos,ex);
}

YSBOOL FsSimulation::AllRecordedFlightsAreOver(double &lastRecordTime)
{
	int nRecorded,nOver;

	nRecorded=0;
	nOver=0;
	lastRecordTime=0;

	{
		FsAirplane *air=NULL;
		while((air=FindNextAirplane(air))!=NULL)
		{
			if(air->isPlayingRecord==YSTRUE)
			{
				nRecorded++;
				if(air->rec!=NULL)
				{
					double lastTime;
					air->rec->GetLastElement(lastTime);
					lastRecordTime=YsGreater(lastRecordTime,lastTime);
					if(lastTime<=currentTime)
					   // Does not count
					{
						nOver++;
					}
				}
			}
		}
	}

	{
		FsGround *gnd=NULL;
		while(NULL!=(gnd=FindNextGround(gnd)))
		{
			if(YSTRUE==gnd->isPlayingRecord &&
			   YSTRUE==gnd->Prop().IsControlledByUser() &&
			   NULL!=gnd->rec)
			{
				nRecorded++;

				double lastTime;
				gnd->rec->GetLastElement(lastTime);
				lastRecordTime=YsGreater(lastRecordTime,lastTime);
				if(lastTime<=currentTime)
				{
					nOver++;
				}
			}
		}
	}

	if(nRecorded>0 && nRecorded<=nOver)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSimulation::CheckMidAir(YsVec3 &collisionPos,FsExistence &ex1,FsExistence &ex2)
{
	const YsVec3 *p1=&ex1.GetPosition();
	const YsVec3 *p2=&ex2.GetPosition();
	const double r1=ex1.GetApproximatedCollideRadius();
	const double r2=ex2.GetApproximatedCollideRadius();

	if((*p1-*p2).GetSquareLength()<(r1+r2)*(r1+r2))
	{
		YsShellPolygonHandle plHd1,plHd2;
		if(YSTRUE==ex1.MayCollideWith(ex2) && YSTRUE==ex2.MayCollideWith(ex1))
		{
			if(YsCheckShellCollisionEx(collisionPos,plHd1,plHd2,ex1.TransformedCollisionShell().Conv(),ex2.TransformedCollisionShell().Conv())==YSTRUE)
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

YSBOOL FsSimulation::CheckMidAir(YsVec3 &collisionPos,const YsShell &coll,FsExistence &ex2)
{
	YsShellPolygonHandle plHd1,plHd2;
	if(YsCheckShellCollisionEx(collisionPos,plHd1,plHd2,coll,ex2.TransformedCollisionShell().Conv())==YSTRUE)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsSimulation::Explode(FsExistence &obj,YSBOOL sound)
{
	const YsVec3 p=obj.GetPosition();
	const double r=obj.GetApproximatedCollideRadius();

	explosionHolder.Explode(currentTime,p,10.0,1.0,r+15.0,YSTRUE,NULL,YSTRUE);
	if(YSTRUE==sound)
	{
		FsSoundSetOneTime(FSSND_ONETIME_BLAST);
	}
	return YSTRUE;
}

void FsSimulation::SimAutoViewChange(FSVIEWMODE mainWindowViewMode,const double dt)
{
	switch(mainWindowViewMode)
	{
	default:
		break;
	case FSFIXEDPOINTPLAYERPLANE:
		UpdateViewpointAccordingToPlayerAirplane(2000.0,YSFALSE);
		break;
	case FSOUTSIDEPLAYERPLANE:
		UpdateViewpointAccordingToPlayerAirplane(2000.0,YSFALSE);
		break;
	case FSVARIABLEPOINTPLAYERPLANE:
		{
			auto playerPlane=GetPlayerAirplane();
			if(nullptr!=playerPlane)
			{
				YsVec3 tmp=playerPlane->Prop().GetPosition();

				if((tmp-viewRefPoint).GetSquareLength()>=300.0*300.0)
				{
					YsVec3 mov;
					mov=viewRefPoint-tmp;
					mov.Normalize();
					mov=mov*300.0;
					viewRefPoint=tmp+mov;
				}
			}
		}
		break;
	case FSANOTHERAIRPLANE:
		if(CheckNoExtAirView()!=YSTRUE)  // 2006/06/11
		{
			for(int i=0; i<GetNumAirplane(); i++)
			{
				const FsAirplane *air=focusAir;
				if(air==NULL || air==GetPlayerAirplane() || air->IsAlive()!=YSTRUE)
				{
					focusAir=FindNextAirplane(focusAir);
					if(focusAir==NULL)
					{
						focusAir=FindNextAirplane(focusAir);
					}
				}
				else
				{
					break;
				}
			}
		}
		break;
	case FSTOWERVIEW:
	case FSTOWERVIEW_NOMAGNIFY:
	case FSAIRTOTOWERVIEW:
	case FSAIRTOTOWERVIEWSOLO:
		if(focusAir==NULL)
		{
			focusAir=FindNextAirplane(NULL);
		}
		break;
	case FSSPOTPLANEVIEW:
		if(focusAir!=NULL)
		{
			YsVec3 dir;
			double dist;
			dist=relViewDist*focusAir->GetApproximatedCollideRadius();
			dir.Set(0,0,dist);

			YsVec3 ev1,ev2;
			ev1=focusAir->GetAttitude().GetForwardVector();
			ev2=relViewAtt.GetForwardVector();

			relViewAtt.Mul(dir,dir); // dir=relViewAtt.GetMatrix()*dir;
			relViewAtt.SetB((ev1*ev2)*focusAir->GetAttitude().b()/2.0);
		}
		break;
	case FSVERTICALORBITINGVIEW:
		relViewAtt.NoseUp(-dt*YsPi/12.0);
		break;
	case FSHORIZONTALORBITINGVIEW:
		relViewAtt.YawLeft(dt*YsPi/12.0);
		break;
	case FSTURNVIEW:
		relViewAtt.SetH(relViewAtt.h()+dt*YsPi/12.0);
		break;
	}
}

void FsSimulation::SimDecideViewpointAndCheckIsInCloud(ActualViewMode &actualViewMode,FSVIEWMODE viewmode,YsVec2i drawingAreaSize) const
{
	SimDecideViewpoint(actualViewMode,viewmode);

	actualViewMode.viewMat.Initialize();
	actualViewMode.viewMat.RotateXY(-actualViewMode.viewAttitude.b());
	actualViewMode.viewMat.RotateZY(-actualViewMode.viewAttitude.p());
	actualViewMode.viewMat.RotateXZ(-actualViewMode.viewAttitude.h());
	actualViewMode.viewMat.Translate(-actualViewMode.viewPoint);

	actualViewMode.isViewPointInCloud=weather->IsInCloudLayer(actualViewMode.viewPoint);
	if(actualViewMode.isViewPointInCloud!=YSTRUE)
	{
		actualViewMode.isViewPointInCloud=solidCloud->IsInCloud(actualViewMode.viewPoint);
	}

	if(actualViewMode.isViewPointInCloud!=YSTRUE)
	{
		actualViewMode.fogVisibility=weather->GetFogVisibility();
	}
	else
	{
		actualViewMode.fogVisibility=100.0;
	}

	if(YSTRUE==FsIsShadowMapAvailable())
	{
		FsProjection prj;
		GetProjection(prj,actualViewMode);
		YsAtt3 lightViewAtt;

		const auto cameraEv=actualViewMode.viewAttitude.GetForwardVector();
		const auto cameraUv=actualViewMode.viewAttitude.GetUpVector();
		
		const YsVec3 lightEv=-YsUnitVector(cfgPtr->lightSourceDirection);
		const double cameraEvSimilarity=fabs(lightEv*cameraEv);
		const double cameraUvSimilarity=fabs(lightEv*cameraUv);
		const YsVec3 lightUv=(cameraEvSimilarity<cameraUvSimilarity ? cameraEv : cameraUv);
		
		lightViewAtt.SetTwoVector(lightEv,lightUv);


//		// Cascade Shadow Map
//		auto projMat=prj.GetMatrix();
//		YsPlane pln[6];
//		YsCalcualteViewFrustumPlaneFromProjectionMatrix(pln,projMat);
//		YsVec3 v[4]=
//		{
//			pln[0].GetNormal()^pln[2].GetNormal(),
//			pln[2].GetNormal()^pln[1].GetNormal(),
//			pln[1].GetNormal()^pln[3].GetNormal(),
//			pln[3].GetNormal()^pln[0].GetNormal(),
//		};
//		double nearZ=pln[4].GetOrigin().z();
//		double farZ=pln[5].GetOrigin().z();
//		// v[] vectors of four corners of the view frustum.
//		// nearZ, farZ positive clipping plane distance.
//
//		auto vz=v[0].z();
//		for(auto &x : v)
//		{
//			actualViewMode.viewMat.MulInverse(x,x,0.0);
//		}
//
//		YsArray <double,8> zProfile;
//		zProfile.Add(10.0);
//		zProfile.Add(100.0);
//		zProfile.Add(500.0);
//		zProfile.Add(8000.0);  // Must be max_num_shadowmap+1 dists.
//
//		auto &commonTexture=FsCommonTexture::GetCommonTexture();
//		assert(commonTexture.GetMaxNumShadowMap()==ActualViewMode::NUM_SHADOW_MAP);
//		assert(commonTexture.GetMaxNumShadowMap()==zProfile.GetN()-1);
//
//		for(YSSIZE_T i=0; i<zProfile.GetN()-1; ++i)
//		{
//			double t0=zProfile[i]  /vz;
//			double t1=zProfile[i+1]/vz;
//
//			YsVec3 frustumCorner[8]=
//			{
//				actualViewMode.viewPoint+v[0]*t0,
//				actualViewMode.viewPoint+v[1]*t0,
//				actualViewMode.viewPoint+v[2]*t0,
//				actualViewMode.viewPoint+v[3]*t0,
//				actualViewMode.viewPoint+v[0]*t1,
//				actualViewMode.viewPoint+v[1]*t1,
//				actualViewMode.viewPoint+v[2]*t1,
//				actualViewMode.viewPoint+v[3]*t1,
//			};
//			YsBoundingBoxMaker3 bbx;
//			for(auto &fc : frustumCorner)
//			{
//				lightViewAtt.MulInverse(fc,fc);
//				bbx.Add(fc);
//			}
//
//			const double dx=bbx[1].x()-bbx[0].x();
//			const double dy=bbx[1].y()-bbx[0].y();
//			const double dz=bbx[1].z()-bbx[0].z();
//			const double aspect=dx/dy;
//printf("%d %lf %lf %lf\n",i,dx,dy,dz);
//			auto nearCen=bbx.GetCenter();
//			nearCen.SetZ(bbx[0].z());
//
//			lightViewAtt.Mul(nearCen,nearCen);
//			const YsVec3 lightOrigin=nearCen;
//
//			auto &m=actualViewMode.shadowViewMat[i];
//			m.LoadIdentity();
//			m.RotateXY(-lightViewAtt.b());
//			m.RotateZY(-lightViewAtt.p());
//			m.RotateXZ(-lightViewAtt.h());
//			m.Translate(-lightOrigin);
//
//			YsProjectionTransformation proj;
//			proj.SetProjectionMode(YsProjectionTransformation::ORTHOGONAL);
//			proj.SetAspectRatio(aspect);
//			proj.SetOrthogonalProjectionHeight(dy);
//			proj.SetNearFar(0.0,dz);
//			actualViewMode.shadowProjMat[i]=proj.GetProjectionMatrix();
//		}


		// Control distance based on sin(theata)
		// where theata is the angle between the lightVec and cameraVec
		{
			const auto cameraEv=actualViewMode.viewAttitude.GetForwardVector();
			const YsVec3 lightEv=-YsUnitVector(cfgPtr->lightSourceDirection);
			const double cosTheata=cameraEv*lightEv;
			const double sinTheata=sqrt(1.0-YsSmaller(1.0,cosTheata*cosTheata));

			const double baseDist=YsBound(actualViewMode.viewTargetDist,30.0,200.0);

			const double lightVolumeProfile[3]=
			{
				baseDist,
				1500.0,
				12000.0
			};
			const double lightDepthProfile[3]=
			{
				lightVolumeProfile[0]*30.0,
				lightVolumeProfile[1]*3.5,
				lightVolumeProfile[2]*0.6
			};
			const double lightPullProfile[3]=
			{
				lightDepthProfile[0]/3.0,
				lightDepthProfile[1]/3.0,
				lightDepthProfile[2]/3.0
			};
			for(int i=0; i<3; ++i)
			{
				const YsVec3 push=cameraEv*lightVolumeProfile[i]*sinTheata;
				const YsVec3 lightOrigin=actualViewMode.viewPoint+push-lightEv*lightPullProfile[i];
			
				actualViewMode.shadowViewMat[i].LoadIdentity();
				actualViewMode.shadowViewMat[i].RotateXY(-lightViewAtt.b());
				actualViewMode.shadowViewMat[i].RotateZY(-lightViewAtt.p());
				actualViewMode.shadowViewMat[i].RotateXZ(-lightViewAtt.h());
				actualViewMode.shadowViewMat[i].Translate(-lightOrigin);
			
				// Needs adjustment, and should use cascade-shadow map. -> Nah, forget about it.  Cascade shadow map doesn't work.
				YsProjectionTransformation proj;
				proj.SetProjectionMode(YsProjectionTransformation::ORTHOGONAL);
				proj.SetAspectRatio(1.0);
				proj.SetOrthogonalProjectionHeight(lightVolumeProfile[i]*2.0);
				proj.SetNearFar(0.0,lightDepthProfile[i]);
				actualViewMode.shadowProjMat[i]=proj.GetProjectionMatrix();
			}
		}


		// Ad-hoc shadow transformation
//		{
//			const YsVec3 lightOrigin=actualViewMode.viewPoint-lightEv*200.0;
//		
//			for(auto &m : actualViewMode.shadowViewMat)
//			{
//				m.LoadIdentity();
//				m.RotateXY(-lightViewAtt.b());
//				m.RotateZY(-lightViewAtt.p());
//				m.RotateXZ(-lightViewAtt.h());
//				m.Translate(-lightOrigin);
//			}
//		
//			// Needs adjustment, and should use cascade-shadow map.
//			YsProjectionTransformation proj;
//			proj.SetProjectionMode(YsProjectionTransformation::ORTHOGONAL);
//			proj.SetAspectRatio(1.0);
//			proj.SetOrthogonalProjectionHeight(150.0);
//			proj.SetNearFar(0.0,1000.0);
//			actualViewMode.shadowProjMat[0]=proj.GetProjectionMatrix();
//		
//			proj.SetOrthogonalProjectionHeight(2000.0);
//			proj.SetNearFar(0.0,5000.0);
//			actualViewMode.shadowProjMat[1]=proj.GetProjectionMatrix();
//
//			proj.SetOrthogonalProjectionHeight(5000.0);
//			proj.SetNearFar(0.0,10000.0);
//			actualViewMode.shadowProjMat[2]=proj.GetProjectionMatrix();
//		}
	}
	else
	{
		for(auto &m : actualViewMode.shadowProjMat)
		{
			m.LoadIdentity();
		}
		for(auto &m : actualViewMode.shadowViewMat)
		{
			m.LoadIdentity();
		}
	}
}

void FsSimulation::SimDecideViewpoint(ActualViewMode &actualViewMode,FSVIEWMODE mode) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	const FsGround *playerGround=GetPlayerGround();

	actualViewMode.actualViewMode=mode;  // by Default
	actualViewMode.viewMagFix=1.0;       // by Default
	actualViewMode.actualViewHdg=userInput.viewHdg;  // by Default
	actualViewMode.actualViewPch=userInput.viewPch;  // by Default

	if(mode==FSGHOSTVIEW)
	{
		return;
	}

	if(NULL!=playerPlane)
	{
		SimDecideViewpoint_Air(actualViewMode,mode,playerPlane);
	}
	else if(NULL!=playerGround)
	{
		SimDecideViewpoint_Gnd(actualViewMode,mode,playerGround);
	}
	else
	{
		if(playerPlane==NULL)
		{
			if(CheckNoExtAirView()!=YSTRUE)  // 2006/06/11
			{
				FsAirplane *air;
				air=NULL;
				while((air=FindNextAirplane(air))!=NULL)
				{
					if(air!=NULL)
					{
						SimDecideViewpoint_Air(actualViewMode,mode,air);
						return;
					}
				}
			}

			actualViewMode.viewPoint.Set(0.0,10.0,0.0);
			actualViewMode.viewAttitude.Set(0.0,0.0,0.0);
			return;
		}
	}
}

void FsSimulation::SimDecideViewpoint_Air(ActualViewMode &actualViewMode,FSVIEWMODE mode,const FsAirplane *playerPlane) const
{
	actualViewMode.actualViewMode=mode;  // by Default
	actualViewMode.viewMagFix=1.0;       // by Default

	switch(mode)
	{
	case FSCOCKPITVIEW:
		if(playerPlane->Prop().IsActive()==YSTRUE || playerPlane->Prop().IsAlive()==YSFALSE)
		{
			YsVec3 cock;
			YsMatrix4x4 mat;
			playerPlane->Prop().GetCockpitPosition(cock);

			mat.Translate(playerPlane->GetPosition());
			mat.Rotate(playerPlane->GetAttitude());

			actualViewMode.viewPoint=mat*cock;

			actualViewMode.viewAttitude=playerPlane->GetAttitude();

			const YsAtt3 &neutAtt=playerPlane->Prop().GetNeutralHeadDirection();

			actualViewMode.viewAttitude.YawLeft(neutAtt.h());
			actualViewMode.viewAttitude.NoseUp(neutAtt.p());
			actualViewMode.viewAttitude.SetB(actualViewMode.viewAttitude.b()+neutAtt.b());

			actualViewMode.viewAttitude.YawLeft(userInput.viewHdg);
			actualViewMode.viewAttitude.NoseUp(userInput.viewPch);
		}
		else
		{
			if(playerPlane->Prop().GetFlightState()==FSOVERRUN)
			{
				SimDecideViewpoint(actualViewMode,FSFROMTOPOFPLAYERPLANE);
			}
			else
			{
				SimDecideViewpoint(actualViewMode,FSFIXEDPOINTPLAYERPLANE);
			}
		}
		break;
	case FSBOMBINGVIEW:
		if(playerPlane->Prop().IsActive()==YSTRUE || playerPlane->Prop().IsAlive()==YSFALSE)
		{
			YsVec3 cock;
			YsMatrix4x4 mat;
			playerPlane->Prop().GetCockpitPosition(cock);

			mat.Translate(playerPlane->GetPosition());
			mat.Rotate(playerPlane->GetAttitude());

			actualViewMode.viewPoint=mat*cock;

			actualViewMode.viewAttitude=playerPlane->GetAttitude();
			actualViewMode.viewAttitude.NoseUp(-YsDegToRad(60.0));
		}
		else
		{
			if(playerPlane->Prop().GetFlightState()==FSOVERRUN)
			{
				SimDecideViewpoint(actualViewMode,FSFROMTOPOFPLAYERPLANE);
			}
			else
			{
				SimDecideViewpoint(actualViewMode,FSFIXEDPOINTPLAYERPLANE);
			}
		}
		break;
	case FSOUTSIDEPLAYERPLANE:
		{
			actualViewMode.viewPoint=playerPlane->GetPosition();

			auto dist=playerPlane->GetApproximatedCollideRadius()*2.0;

			YsVec3 tmp;
			tmp=viewRefPoint-actualViewMode.viewPoint;
			tmp.Normalize();
			tmp*=dist;
			actualViewMode.viewPoint+=tmp;
			actualViewMode.viewAttitude.SetForwardVector(-tmp);
			actualViewMode.viewTargetDist=dist;
		}
		break;
	case FSFIXEDPOINTPLAYERPLANE:
		{
			YsVec3 tmp;
			tmp=(playerPlane->GetPosition())-viewRefPoint;

			actualViewMode.viewPoint=viewRefPoint+tmp*5.0/6.0;
			actualViewMode.viewAttitude.SetForwardVector(tmp);
		}
		break;
	case FSVARIABLEPOINTPLAYERPLANE:
		{
			YsVec3 tmp;
			tmp=playerPlane->Prop().GetPosition();
			actualViewMode.viewPoint=viewRefPoint;
			tmp=tmp-actualViewMode.viewPoint;
			actualViewMode.viewAttitude.SetForwardVector(tmp);
		}
		break;
	case FSFROMTOPOFPLAYERPLANE:
		actualViewMode.viewPoint=playerPlane->Prop().GetPosition();
		actualViewMode.viewPoint.Set(actualViewMode.viewPoint.x(),actualViewMode.viewPoint.y()+30.0,actualViewMode.viewPoint.z()-10.0);
		actualViewMode.viewAttitude.Set(0.0,YsDegToRad(-72),0.0);
		break;
	case FSPLAYERPLANEFROMSIDE:
		{
			const double viewTargetDist=playerPlane->Prop().GetOutsideRadius()*1.5;

			YsVec3 offset;
			actualViewMode.viewPoint=playerPlane->Prop().GetPosition();
			actualViewMode.viewAttitude=playerPlane->Prop().GetAttitude();
			actualViewMode.viewAttitude.Set(actualViewMode.viewAttitude.h()+YsPi/2.0,0.0,0.0);
			offset.Set(0.0,0.0,-viewTargetDist);
			offset.RotateXZ(actualViewMode.viewAttitude.h());
			actualViewMode.viewPoint+=offset;

			actualViewMode.viewTargetDist=viewTargetDist;
		}
		break;
	case FSANOTHERAIRPLANE:
		if(CheckNoExtAirView()!=YSTRUE)  // 2006/06/11
		{
			const FsAirplane *air=focusAir;
			if(air!=NULL && air!=playerPlane && air->IsAlive()==YSTRUE)
			{
				const YsVec3 &p1=air->GetPosition();
				const YsAtt3 &a1=air->GetAttitude();

				const YsVec3 &p2=playerPlane->GetPosition();
				// const YsAtt3 &a2=playerPlane->GetAttitude();

				const double radius=air->GetApproximatedCollideRadius();

				const YsVec3 viewDir=p2-p1;

				actualViewMode.viewAttitude.SetForwardVector(viewDir);
				actualViewMode.viewAttitude.SetB(a1.b());

				if(YSTRUE==air->Prop().IsOnGround() && radius*sin(actualViewMode.viewAttitude.p())>air->Prop().GetGroundStandingHeight()/2.0)
				{
					const double p=asin((air->Prop().GetGroundStandingHeight()/2.0)/radius);
					actualViewMode.viewAttitude.SetP(p);
				}

				YsVec3 off(1.0,0.6,-3.0);
				off*=radius;
				actualViewMode.viewAttitude.Mul(off,off); // off=att.GetMatrix()*off;

				actualViewMode.viewTargetDist=off.GetLength();

				actualViewMode.viewPoint=p1+off;

				if(actualViewMode.viewPoint.y()<air->Prop().GetGroundElevation()+0.5)
				{
					actualViewMode.viewPoint.SetY(air->Prop().GetGroundElevation()+0.5);
				}

				return;
			}
		}
		// If no other airplane is found,
		SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);  // Actual viewmode will be automatically set
		break;
	case FSMISSILEVIEW:
		{
			YsVec3 &pos=actualViewMode.viewPoint;
			YsAtt3 &att=actualViewMode.viewAttitude;
			if(bulletHolder.FindFirstMissilePositionThatIsReallyGuided(pos,att)==YSOK)
			{
				YsVec3 off;
				att.SetB(0.0);
				off.Set(0.0,2.0,-10.0);
				att.Mul(off,off);
				pos=pos+off;
			}
			else if(CheckNoExtAirView()==YSTRUE)  // 2006/06/11
			{
				SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
			}
			else
			{
				SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
			}
		}
		break;
	case FSMYWEAPONVIEW_OLD:
	case FSMYWEAPONVIEW_NEW:
		{
			YSRESULT res;
			res=YSERR;
			if(mode==FSMYWEAPONVIEW_OLD)
			{
				res=bulletHolder.FindOldestMissilePosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,playerPlane);
			}
			else if(mode==FSMYWEAPONVIEW_NEW)
			{
				res=bulletHolder.FindNewestMissilePosition(actualViewMode.viewPoint,actualViewMode.viewAttitude,playerPlane);
			}

			if(res==YSOK)
			{
				YsVec3 off;
				actualViewMode.viewAttitude.SetB(0.0);
				off.Set(0.0,2.0,-10.0);
				actualViewMode.viewAttitude.Mul(off,off);
				actualViewMode.viewPoint+=off;
			}
			else if(CheckNoExtAirView()==YSTRUE)
			{
				SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
			}
			else
			{
				SimDecideViewpoint(actualViewMode,FSLOCKEDTARGETVIEW);
			}
		}
		break;
	case FSLOCKEDTARGETVIEW:
		if(playerPlane!=NULL)
		{
			const FsExistence *trg;
			if(playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_AIM9 ||
			   playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_AIM9X ||
			   playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_AIM120 ||
			   playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_AGM65)
			{
				if(playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_AGM65)
				{
					trg=FindGround(playerPlane->Prop().GetGroundTargetKey());
				}
				else
				{
					trg=FindAirplane(playerPlane->Prop().GetAirTargetKey());
				}

				if(trg!=NULL)
				{
					double viewTargetDist=trg->GetApproximatedCollideRadius()*1.5;

					YsVec3 ev,uv;
					ev=trg->GetPosition()-playerPlane->GetPosition();
					ev.Normalize();
					uv=playerPlane->GetAttitude().GetUpVector();
					actualViewMode.viewPoint=trg->GetPosition()-ev*viewTargetDist;
					actualViewMode.viewAttitude.SetTwoVector(ev,uv);
					return;
				}
			}
			else if(playerPlane->Prop().GetWeaponOfChoice()==FSWEAPON_GUN)
			{
				if(playerPlane->Prop().GetHasPilotControlledTurret()==YSTRUE)
				{
					YsVec3 dir;
					playerPlane->Prop().GetFirstPilotControlledTurretPosition(actualViewMode.viewPoint);
					playerPlane->Prop().GetFirstPilotControlledTurretDirection(dir);

					actualViewMode.viewAttitude.SetTwoVector(dir,playerPlane->GetAttitude().GetUpVector());
					actualViewMode.viewMagFix=40.0;
					return;
				}
				else
				{
					const FsAirplane *target;
					YsVec3 aim;
					if(SimCalculateGunAim(target,aim)==YSOK)
					{
						double viewTargetDist=playerPlane->GetApproximatedCollideRadius();

						YsVec3 ev;
						ev=aim-playerPlane->Prop().GetPosition();
						ev.Normalize();
						actualViewMode.viewPoint=playerPlane->Prop().GetPosition()+ev*viewTargetDist;
						actualViewMode.viewAttitude.SetTwoVector(ev,playerPlane->GetAttitude().GetUpVector());
						actualViewMode.viewMagFix=40.0;
						actualViewMode.viewTargetDist=viewTargetDist;
						return;
					}
				}
			}

			double viewTargetDist=playerPlane->GetApproximatedCollideRadius();

			YsVec3 ev;
			actualViewMode.viewPoint=playerPlane->Prop().GetPosition();
			actualViewMode.viewAttitude=playerPlane->Prop().GetAttitude();
			ev=actualViewMode.viewAttitude.GetForwardVector();
			actualViewMode.viewPoint+=ev*viewTargetDist;
			actualViewMode.viewMagFix=40.0;
			actualViewMode.viewTargetDist=viewTargetDist;
		}
		break;
	case FSAIRTOAIRVIEW:
	case FSAIRFROMAIRVIEW:
		if(CheckNoExtAirView()!=YSTRUE)  // 2006/06/11
		{
			const FsAirplane *from=focusAir;
			const FsAirplane *to=focusAir2;

			if(from!=NULL && to!=NULL)
			{
				YsVec3 off;
				const YsVec3 *p1,*p2;
				const YsAtt3 *a1,*a2;
				YsVec3 upv;

				p1=&from->GetPosition();
				a1=&from->GetAttitude();

				p2=&to->GetPosition();
				a2=&to->GetAttitude();

				if(from->Prop().IsActive()==YSTRUE)
				{
					upv=a1->GetUpVector();
				}
				else
				{
					upv=a2->GetUpVector();
				}

				if(mode==FSAIRTOAIRVIEW)
				{
					actualViewMode.viewAttitude.SetTwoVector(*p2-*p1,upv);
				}
				else if(mode==FSAIRFROMAIRVIEW)
				{
					actualViewMode.viewAttitude.SetTwoVector(*p1-*p2,upv);
				}

				off.Set(1.0,0.6,-3.0);
				off*=from->GetApproximatedCollideRadius();
				actualViewMode.viewAttitude.Mul(off,off);  // off=att.GetMatrix()*off;
				actualViewMode.viewTargetDist=off.GetLength();

				actualViewMode.viewPoint=*p1+off;
				if(actualViewMode.viewPoint.y()<1.0)
				{
					actualViewMode.viewPoint.SetY(1.0);
				}
			}
		}
		else
		{
			SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
		}
		break;
	case FSCARRIERVIEW:
		if(NULL!=focusGnd && NULL!=focusAir)
		{
			const FsGround *gnd=focusGnd;
			if(gnd!=NULL && gnd->IsAlive()==YSTRUE)
			{
				YsVec3 off;
				YsMatrix4x4 mat;
				mat.Initialize();
				mat.Translate(gnd->GetPosition());
				mat.Rotate(gnd->GetAttitude());

				if(gnd->Prop().GetAircraftCarrierProperty()!=NULL)
				{
					off=gnd->Prop().GetAircraftCarrierProperty()->GetBridgePos();
				}
				else
				{
					off.Set(25.0,52.0,-28.0);
				}

				YsVec3 ofstAir=focusAir->GetLookAtOffset();
				focusAir->GetAttitude().Mul(ofstAir,ofstAir);

				actualViewMode.viewPoint=ofstAir+mat*off;
				actualViewMode.viewTargetDist=ofstAir.GetLength();

				actualViewMode.viewMagFix=2.0;

				actualViewMode.viewAttitude.SetForwardVector(focusAir->GetPosition()-actualViewMode.viewPoint);
			}
			else
			{
				SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
			}
		}
		break;
	case FSTOWERVIEW:
	case FSTOWERVIEW_NOMAGNIFY:
		{
			if(focusAir!=NULL)
			{
				YsVec3 dir;
				dir=focusAir->GetPosition()-towerViewPos;
				dir.Normalize();
				actualViewMode.viewAttitude.SetForwardVector(dir);
				actualViewMode.viewPoint=towerViewPos;

				if(mode==FSTOWERVIEW)
				{
					actualViewMode.viewMagFix=8.0;
				}
			}
			else
			{
				SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
			}
		}
		break;
	case FSAIRTOTOWERVIEW:
	case FSAIRTOTOWERVIEWSOLO:
		if(focusAir!=NULL)
		{
			double r;
			YsVec3 dir;
			dir=towerViewPos-focusAir->GetPosition();
			dir.Normalize();

			r=focusAir->GetRadiusFromCollision();

			if(mode==FSAIRTOTOWERVIEW)
			{
				r*=3.0;
			}
			else
			{
				r*=1.5;
			}

			YsVec3 ofst=focusAir->GetLookAtOffset();
			focusAir->GetAttitude().Mul(ofst,ofst);

			actualViewMode.viewPoint=focusAir->GetPosition()+ofst-dir*r;
			if(actualViewMode.viewPoint.y()<towerViewPos.y())
			{
				actualViewMode.viewPoint.SetY(towerViewPos.y());
			}
			actualViewMode.viewTargetDist=(ofst-dir*r).GetLength();

			dir=focusAir->GetPosition()-actualViewMode.viewPoint;

			actualViewMode.viewAttitude.SetForwardVector(dir);
			actualViewMode.viewAttitude.SetB(0.0);

			actualViewMode.viewMagFix=1.0;
		}
		else
		{
			SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
		}
		break;
	case FSPLAYERTOGNDVIEW:
		if(focusGnd!=NULL)
		{
			YsVec3 gnd=focusGnd->GetCollisionShellCenter();

			focusGnd->GetAttitude().Mul(gnd,gnd);
			gnd+=focusGnd->GetPosition();

			YsVec3 vec=gnd-playerPlane->GetPosition();
			vec.Normalize();

			const double rad=playerPlane->Prop().GetOutsideRadius()*2.0;
			vec*=rad;
			vec.SubY(rad*0.5);
			if(vec.y()>=0.0)
			{
				vec.SetY(0.0);
			}

			actualViewMode.viewAttitude.SetForwardVector(vec);
			actualViewMode.viewAttitude.SetB(0.0);
			actualViewMode.viewTargetDist=rad;

			actualViewMode.viewPoint=playerPlane->GetPosition()-vec;
		}
		else
		{
			SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
		}
		break;
	case FSGNDTOPLAYERVIEW:
		if(focusGnd!=NULL)
		{
			YsVec3 gnd=focusGnd->GetCollisionShellCenter();

			focusGnd->GetAttitude().Mul(gnd,gnd);
			gnd+=focusGnd->GetPosition();

			YsVec3 vec=playerPlane->GetPosition()-gnd;
			vec.Normalize();

			const double rad=playerPlane->Prop().GetOutsideRadius()*2.0;
			vec*=rad;
			vec.SubY(rad*0.5);
			if(vec.y()>=0.0)
			{
				vec.SetY(0.0);
			}

			actualViewMode.viewAttitude.SetForwardVector(vec);
			actualViewMode.viewAttitude.SetB(0.0);
			actualViewMode.viewTargetDist=rad;

			actualViewMode.viewPoint=playerPlane->GetPosition()-vec;
		}
		else
		{
			SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
		}
		break;
	case FSSPOTPLANEVIEW:
		if(focusAir!=NULL)
		{
			YsVec3 dir,fomCen;
			double dist=relViewDist*focusAir->GetApproximatedCollideRadius();
			dir.Set(0,0,dist);

			relViewAtt.Mul(dir,dir);

			actualViewMode.viewPoint=focusAir->GetPosition()-dir;
			actualViewMode.viewAttitude=relViewAtt;
			actualViewMode.viewTargetDist=dist;
		}
		else
		{
			SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
		}
		break;
	case FSVERTICALORBITINGVIEW:
		SimDecideViewpoint(actualViewMode,FSOUTSIDEPLAYER2);
		actualViewMode.viewAttitude.SetB(0.0);
		break;
	case FSHORIZONTALORBITINGVIEW:
		SimDecideViewpoint(actualViewMode,FSOUTSIDEPLAYER2);
		break;
	case FSTURNVIEW:
		SimDecideViewpoint(actualViewMode,FSOUTSIDEPLAYER2);
		break;
	case FSBACKMIRRORVIEW:
	case FS45DEGREERIGHTVIEW:
	case FS45DEGREELEFTVIEW:
	case FS90DEGREERIGHTVIEW:
	case FS90DEGREELEFTVIEW:
	case FSADDITIONALAIRPLANEVIEW:
	case FSOUTSIDEPLAYER2:
	case FSOUTSIDEPLAYER3:
		SimDecideViewpoint_Common(actualViewMode,mode);
		break;
	}
}

void FsSimulation::SimDecideViewpoint_Gnd(ActualViewMode &actualViewMode,FSVIEWMODE mode,const FsGround *playerGround) const
{
	switch(mode)
	{
	default:
		actualViewMode.actualViewMode=FSCOCKPITVIEW;
		// Fall down to FSCOCKPITVIEW
	case FSCOCKPITVIEW:
		if(playerGround->Prop().IsActive()==YSTRUE || playerGround->Prop().IsAlive()==YSFALSE)
		{
			const YsVec3 cock=playerGround->Prop().GetUserViewPoint();

			actualViewMode.viewPoint=playerGround->GetMatrix()*cock;
			actualViewMode.viewAttitude=playerGround->GetAttitude();

			const YsAtt3 &neutAtt=YsZeroAtt(); // Will be added.

			actualViewMode.viewAttitude.YawLeft(neutAtt.h());
			actualViewMode.viewAttitude.NoseUp(neutAtt.p());
			actualViewMode.viewAttitude.SetB(actualViewMode.viewAttitude.b()+neutAtt.b());

			switch(playerGround->Prop().GetWeaponOfChoice())
			{
			default:
			case FSWEAPON_NULL:
				break;
			case FSWEAPON_GUN:
				/* Was it correct? att.YawLeft(playerGround->Prop().GetAaaAim().h());
				att.NoseUp(playerGround->Prop().GetAaaAim().p()); */
				actualViewMode.viewAttitude=playerGround->Prop().GetAaaAim();
				break;
			case FSWEAPON_AIM9:
			case FSWEAPON_AGM65:
				/* Was it correct? att.YawLeft(playerGround->Prop().GetSamAim().h());
				att.NoseUp(playerGround->Prop().GetSamAim().p()); */
				actualViewMode.viewAttitude=playerGround->Prop().GetSamAim();
				break;
			}

			actualViewMode.viewAttitude.YawLeft(userInput.viewHdg);
			actualViewMode.viewAttitude.NoseUp(userInput.viewPch);
		}
		else
		{
			// Outside view to let the user that the user is killed.
		}
		break;
	case FSTOWERVIEW:
	case FSTOWERVIEW_NOMAGNIFY:
		if(playerGround!=NULL)
		{
			YsVec3 dir=playerGround->GetPosition()-towerViewPos;
			dir.Normalize();
			actualViewMode.viewAttitude.SetForwardVector(dir);

			YsVec3 ofst=playerGround->GetLookAtOffset();
			playerGround->GetAttitude().Mul(ofst,ofst);

			actualViewMode.viewPoint=towerViewPos;

			if(mode==FSTOWERVIEW)
			{
				actualViewMode.viewMagFix=8.0;
			}
		}
		else
		{
			SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
		}
		break;
	case FSBACKMIRRORVIEW:
	case FS45DEGREERIGHTVIEW:
	case FS45DEGREELEFTVIEW:
	case FS90DEGREERIGHTVIEW:
	case FS90DEGREELEFTVIEW:
	case FSADDITIONALAIRPLANEVIEW:
	case FSOUTSIDEPLAYER2:
	case FSOUTSIDEPLAYER3:
		SimDecideViewpoint_Common(actualViewMode,mode);
		break;
	case FSLOCKEDTARGETVIEW:
		if(playerGround!=NULL)
		{
			const YsVec3 cock=playerGround->Prop().GetUserViewPoint();

			actualViewMode.viewPoint=playerGround->GetMatrix()*cock;
			actualViewMode.viewAttitude=playerGround->Prop().GetAttitude();

			const FsExistence *trg;
			if(playerGround->GetWeaponOfChoice()==FSWEAPON_AIM9 ||
			   playerGround->GetWeaponOfChoice()==FSWEAPON_AIM9X ||
			   playerGround->GetWeaponOfChoice()==FSWEAPON_AIM120 ||
			   playerGround->GetWeaponOfChoice()==FSWEAPON_AGM65)
			{
				actualViewMode.viewAttitude=playerGround->Prop().GetSamAim();

				if(playerGround->Prop().GetWeaponOfChoice()==FSWEAPON_AGM65)
				{
					trg=playerGround->Prop().GetGroundTarget();
				}
				else
				{
					trg=playerGround->Prop().GetAirTarget();
				}

				if(trg!=NULL)
				{
					YsVec3 ev,uv;
					ev=trg->GetPosition()-playerGround->GetPosition();
					ev.Normalize();
					uv=playerGround->GetAttitude().GetUpVector();
					actualViewMode.viewPoint=trg->GetPosition()-ev*trg->GetApproximatedCollideRadius()*1.5;
					actualViewMode.viewAttitude.SetTwoVector(ev,uv);
					return;
				}
			}
			else if(playerGround->Prop().GetWeaponOfChoice()==FSWEAPON_GUN)
			{
				actualViewMode.viewAttitude=playerGround->Prop().GetAaaAim();

				if(playerGround->Prop().GetHasPilotControlledTurret()==YSTRUE)
				{
					YsVec3 dir;
					playerGround->Prop().GetFirstPilotControlledTurretPosition(actualViewMode.viewPoint);
					playerGround->Prop().GetFirstPilotControlledTurretDirection(dir);

					actualViewMode.viewAttitude.SetTwoVector(dir,playerGround->GetAttitude().GetUpVector());
					actualViewMode.viewMagFix=40.0;
					return;
				}
				else
				{
					/* FsAirplane *target;
					YsVec3 aim;
					if(SimCalculateGunAim(target,aim)==YSOK)
					{
						YsVec3 ev;
						ev=aim-playerGround->Prop().GetPosition();
						ev.Normalize();
						pos=playerGround->Prop().GetPosition()+ev*playerGround->GetApproximatedCollideRadius();
						att.SetTwoVector(ev,playerGround->GetAttitude().GetUpVector());
						viewMagFix=40.0;
						return;
					} */
					actualViewMode.viewAttitude=playerGround->Prop().GetAaaAim();
					actualViewMode.viewMagFix=40.0;
					return;
				}
			}

			YsVec3 ev;
			ev=actualViewMode.viewAttitude.GetForwardVector();
			actualViewMode.viewPoint+=ev*playerGround->GetApproximatedCollideRadius();
			actualViewMode.viewMagFix=40.0;
		}
		break;
	}
}

void FsSimulation::SimDecideViewpoint_Common(ActualViewMode &actualViewMode,FSVIEWMODE mode) const
{
	const FsExistence *playerObj=GetPlayerObject();
	switch(mode)
	{
	case FSOUTSIDEPLAYER2:
		{
			const FsExistence *toLookAt=NULL;
			if(NULL!=playerObj && FSEX_GROUND==playerObj->GetType())
			{
				toLookAt=playerObj;
			}
			else
			{
				toLookAt=focusAir;
			}
			if(NULL!=toLookAt)
			{
				YsVec3 dir,fomCen;
				double dist;
				dist=relViewDist*toLookAt->GetApproximatedCollideRadius();
				dir.Set(0,0,dist);

				relViewAtt.Mul(dir,dir);

				YsVec3 ofst=toLookAt->GetLookAtOffset();
				toLookAt->GetAttitude().Mul(ofst,ofst);

				actualViewMode.viewPoint=toLookAt->GetPosition()+ofst-dir;
				actualViewMode.viewAttitude=relViewAtt;
				actualViewMode.viewTargetDist=dist;
			}
		}
		break;
	case FSOUTSIDEPLAYER3:
		{
			const FsExistence *toLookAt=NULL;
			if(NULL!=playerObj && FSEX_GROUND==playerObj->GetType())
			{
				toLookAt=playerObj;
			}
			else
			{
				toLookAt=focusAir;
			}
			if(NULL!=toLookAt)
			{
				YsVec3 ev[2],uv[2],newEv,newUv;
				YsVec3 dir,fomCen;
				double dist;
				dist=relViewDist*toLookAt->GetApproximatedCollideRadius();
				dir.Set(0,0,dist);

				YsAtt3 airAtt;
				if(cfgPtr->externalCameraDelay!=YSTRUE || toLookAt->GetAttitudeFromRecord(airAtt,currentTime-0.8)!=YSOK)
				{
					airAtt=toLookAt->GetAttitude();
				}
				airAtt.Mul(newEv,relViewAtt.GetForwardVector());
				airAtt.Mul(newUv,relViewAtt.GetUpVector());
				actualViewMode.viewAttitude.SetTwoVector(newEv,newUv);
				actualViewMode.viewAttitude.Mul(dir,dir);
				actualViewMode.viewTargetDist=dist;

				YsVec3 ofst=toLookAt->GetLookAtOffset();
				toLookAt->GetAttitude().Mul(ofst,ofst);

				actualViewMode.viewPoint=toLookAt->GetPosition()+ofst-dir;
			}
		}
		break;
	case FSADDITIONALAIRPLANEVIEW:
		if(playerObj!=NULL)
		{
			const FsAdditionalViewpoint *vp;
			vp=playerObj->GetAdditionalView(mainWindowAdditionalAirplaneViewId);
			if(vp!=NULL)
			{
				switch(vp->vpType)
				{
				default:
				case FS_ADVW_INSIDE:
					actualViewMode.actualViewMode=FSADDITIONALAIRPLANEVIEW;
					break;
				case FS_ADVW_OUTSIDE:
					actualViewMode.actualViewMode=FSOUTSIDEPLAYER2;
					break;
				case FS_ADVW_CABIN:
					actualViewMode.actualViewMode=FSADDITIONALAIRPLANEVIEW_CABIN;
					break;
				}

				YsVec3 ev,uv;
				playerObj->GetMatrix().Mul(actualViewMode.viewPoint,vp->pos,1.0);
				playerObj->GetMatrix().Mul(ev,vp->att.GetForwardVector(),0.0);
				playerObj->GetMatrix().Mul(uv,vp->att.GetUpVector(),0.0);
				actualViewMode.viewAttitude.SetTwoVector(ev,uv);

				actualViewMode.viewAttitude.YawLeft(userInput.viewHdg);
				actualViewMode.viewAttitude.NoseUp(userInput.viewPch);
			}
			else
			{
				SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
			}
		}
		break;
	case FSBACKMIRRORVIEW:
	case FS45DEGREERIGHTVIEW:
	case FS45DEGREELEFTVIEW:
	case FS90DEGREERIGHTVIEW:
	case FS90DEGREELEFTVIEW:
		switch(mode)
		{
		case FSBACKMIRRORVIEW:
			actualViewMode.actualViewHdg=YsPi;
			actualViewMode.actualViewPch=0.0;
			break;
		case FS45DEGREERIGHTVIEW:
			actualViewMode.actualViewHdg=-YsPi/4.0;
			actualViewMode.actualViewPch=0.0;
			break;
		case FS45DEGREELEFTVIEW:
			actualViewMode.actualViewHdg=YsPi/4.0;
			actualViewMode.actualViewPch=0.0;
			break;
		case FS90DEGREERIGHTVIEW:
			actualViewMode.actualViewHdg=-YsPi/2.0;
			actualViewMode.actualViewPch=0.0;
			break;
		case FS90DEGREELEFTVIEW:
			actualViewMode.actualViewHdg=YsPi/2.0;
			actualViewMode.actualViewPch=0.0;
			break;
		}

		if(playerObj->IsActive()==YSTRUE || playerObj->IsAlive()==YSFALSE)
		{
			YsVec3 cock=playerObj->GetCockpitPosition();

			YsMatrix4x4 mat;
			mat.Translate(playerObj->GetPosition());
			mat.Rotate(playerObj->GetAttitude());

			actualViewMode.viewPoint=mat*cock;

			actualViewMode.viewAttitude=playerObj->GetAttitude();
			actualViewMode.viewAttitude.YawLeft(actualViewMode.actualViewHdg);
			actualViewMode.viewAttitude.NoseUp(actualViewMode.actualViewPch);

			actualViewMode.actualViewMode=FSCOCKPITVIEW;
		}
		else
		{
			SimDecideViewpoint(actualViewMode,FSCOCKPITVIEW);
		}
		break;
	}
}

YSBOOL FsSimulation::CheckNoExtAirView(void) const
{
	if(cfgPtr->noExtAirView==YSTRUE && EveryAirplaneIsRecordedAirplane()!=YSTRUE)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void FsSimulation::UpdateViewpointAccordingToPlayerAirplane(const double &distance,YSBOOL reset)
{
	const YsAtt3 *att;
	const YsVec3 *pos;
	FsAirplane *playerPlane;

	playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL)
	{
		pos=&playerPlane->GetPosition();

		if(reset==YSTRUE || (*pos-viewRefPoint).GetSquareLength()>=distance*distance)
		{
			YsVec3 offset;
			att=&playerPlane->GetAttitude();
			offset.Set(20.0,5.0,distance/4.0);
			att->Mul(offset,offset);
			viewRefPoint=*pos+offset;
			if(viewRefPoint.y()<5.0)
			{
				viewRefPoint.Set(viewRefPoint.x(),5.0,viewRefPoint.z());
			}
		}
	}
}

void FsSimulation::AirplaneCrash(FsAirplane *crashedPlane,FSDIEDOF diedOf,int collType)
{
	YSSCNAREATYPE areaType;
	if(collType==0 || collType==1)
	{
		areaType=GetAreaType(crashedPlane->GetPosition());
	}
	else
	{
		areaType=YSSCNAREA_LAND;
	}

	crashedPlane->Crash(currentTime,cfgPtr->takeCrash,&explosionHolder,diedOf,areaType);

	if(IsPlayerAirplane(crashedPlane)==YSTRUE)
	{
		FsSoundStopAll();
#ifndef WIN32
		// In Linux, there is no way to force OSS driver to play sound in the playback buffer
		// unless filling the buffer up.  (Or, playback will be blocking.)  So, I need to play
		// silent background sound.  Isn't it ridiculous?

		// This will set internal background sound to silence.
		FsSoundSetEngine(FSSND_ENGINE_SILENT,0,0.0);
		FsSoundSetMachineGun(FSSND_MACHINEGUN_SILENT);
		FsSoundSetAlarm(FSSND_ALARM_SILENT);
#endif
		FsSoundSetOneTime(FSSND_ONETIME_BLAST2);
	}
	else
	{
		FsSoundSetOneTime(FSSND_ONETIME_BLAST);
	}
}

YSRESULT FsSimulation::DestroyAutoGeneratedAirAndGnd(void)
{
	FsAirplane *air;
	air=NULL;
	while(NULL!=(air=FindNextAirplane(air)))
	{
		if(air->airFlag&FSAIRFLAG_AUTOGENERATED)
		{
			air->Prop().SetFlightState(FSDEAD,FSDIEDOF_NULL);
		}
	}

	FsGround *gnd;
	gnd=NULL;
	while(NULL!=(gnd=FindNextGround(gnd)))
	{
		if(gnd->gndFlag&FSGNDFLAG_AUTOGENERATED)
		{
			gnd->Prop().SetState(FSGNDDEAD);
		}
	}

	return YSOK;
}


// YSBOOL FsSimulation::CheckContinue(void)
// {
// #ifndef YS_SCSV
// 	YSBOOL res=YSTFUNKNOWN;
// 	while(YSTFUNKNOWN==(res=CheckContinueOneStep()))
// 	{
// 	}
// 	return res;
// #else
// 	return YSFALSE;
// #endif
// }

YSBOOL FsSimulation::CheckContinueOneStep(void)
{
	if(YSTRUE!=canContinue)
	{
		return YSFALSE;
	}


	if(YSTRUE!=field.CanContinue())
	{
		if(NULL!=contDlg)
		{
			FsContinueDialog::Delete(contDlg);
			contDlg=NULL;
		}
		return YSFALSE;
	}

	if(NULL==contDlg)
	{
		contDlg=FsContinueDialog::Create();
		contDlg->MakeDialog(this);

		int mx,my;
		YSBOOL lb,mb,rb;

		FsPollDevice();
		while(FSMOUSEEVENT_NONE!=FsGetMouseEvent(lb,mb,rb,mx,my))
		{
			FsPollDevice();
		}
	}



	const int ky=FsInkey();
	const int c=FsInkeyChar();

	for(;;)
	{
		int mx,my;
		YSBOOL lb,mb,rb;
		const int eventType=FsGetMouseEvent(lb,mb,rb,mx,my);
		contDlg->SetMouseState(lb,mb,rb,mx,my);
		if(FSMOUSEEVENT_NONE==eventType)
		{
			break;
		}
	}
	
	contDlg->KeyIn(ky,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
	contDlg->CharIn(c);

	YSBOOL res=YSTFUNKNOWN;

	FsGuiDialogItem *btn=contDlg->GetClickedItem();
	if(btn==contDlg->contButton)
	{
		res=YSTRUE;
	}
	else if(btn==contDlg->endButton)
	{
		res=YSFALSE;
	}

	if(YSTFUNKNOWN!=res && NULL!=contDlg)
	{
		FsContinueDialog::Delete(contDlg);
		contDlg=NULL;
	}

	return res;
}

void FsSimulation::CheckContinueDraw(void) const
{
	FsClearScreenAndZBuffer(YsGrayScale(0.25));

	FsCockpitIndicationSet cockpitIndicationSet;
	SimMakeUpCockpitIndicationSet(cockpitIndicationSet);

	SimDrawScreen(0.0,cockpitIndicationSet,YSFALSE,YSFALSE,YSFALSE,mainWindowActualViewMode);
	SimDrawGuiDialog();
	SimDrawFlush(); // <- Swap buffers inside.

}

const double &FsSimulation::GetClock(void)
{
	return currentTime;
}

FsAirplane *FsSimulation::GetPlayerAirplane(void)
{
	return localUser->GetAircraft();
}

const FsAirplane *FsSimulation::GetPlayerAirplane(void) const
{
	return localUser->GetAircraft();;
}

FsGround *FsSimulation::GetPlayerGround(void)
{
	return localUser->GetGround();
}

const FsGround *FsSimulation::GetPlayerGround(void) const
{
	return localUser->GetGround();;
}

FsExistence *FsSimulation::GetPlayerObject(void)
{
	if(NULL!=localUser->GetAircraft())
	{
		return localUser->GetAircraft();
	}
	else if(NULL!=localUser->GetGround())
	{
		return localUser->GetGround();
	}
	else
	{
		return NULL;
	}
}

const FsExistence *FsSimulation::GetPlayerObject(void) const
{
	if(NULL!=localUser->GetAircraft())
	{
		return localUser->GetAircraft();
	}
	else if(NULL!=localUser->GetGround())
	{
		return localUser->GetGround();
	}
	else
	{
		return NULL;
	}
}

FsExistence *FsSimulation::FindObject(YSHASHKEY searchKey)
{
	FsExistence *obj=FindAirplane(searchKey);
	if(NULL!=obj)
	{
		return obj;
	}
	obj=FindGround(searchKey);
	if(NULL!=obj)
	{
		return obj;
	}
	return NULL;
}

const FsExistence *FsSimulation::FindObject(YSHASHKEY searchKey) const
{
	const FsExistence *obj=FindAirplane(searchKey);
	if(NULL!=obj)
	{
		return obj;
	}
	obj=FindGround(searchKey);
	if(NULL!=obj)
	{
		return obj;
	}
	return NULL;
}

int FsSimulation::GetNumAirplane(void) const
{
	return airplaneList.GetN();
}

FsAirplane *FsSimulation::FindNextAirplane(const FsAirplane *ptr) const
{
	YsListItem <FsAirplane> *itm;
	if(ptr!=NULL)
	{
		itm=airplaneList.FindNext(ptr->thisInTheList);
	}
	else
	{
		itm=airplaneList.FindNext(NULL);
	}

	if(itm!=NULL)
	{
		return &itm->dat;
	}
	else
	{
		return NULL;
	}
}

FsAirplane *FsSimulation::FindPrevAirplane(const FsAirplane *ptr) const
{
	YsListItem <FsAirplane> *itm;
	if(ptr!=NULL)
	{
		itm=airplaneList.FindPrev(ptr->thisInTheList);
	}
	else
	{
		itm=airplaneList.FindPrev(NULL);
	}

	if(itm!=NULL)
	{
		return &itm->dat;
	}
	else
	{
		return NULL;
	}
}

FsAirplane *FsSimulation::GetAirplaneById(int id) const
{
	YsListItem <FsAirplane> *seeker;
	seeker=airplaneList.GetItemFromId(id);
	if(seeker!=NULL)
	{
		return &seeker->dat;
	}
	else
	{
		return NULL;
	}
}

FsAirplane *FsSimulation::FindAirplane(YSHASHKEY searchKey) const
{
	FsAirplane *air;
	if(airplaneSearch->FindElement(air,searchKey)==YSOK)
	{
		return air;
	}
	else
	{
		return NULL;
	}
}

int FsSimulation::GetAirplaneIdFromHandle(FsExistence *air) const
{
	if(air!=NULL && FSEX_AIRPLANE==air->GetType())
	{
		return airplaneList.GetIdFromItem(((FsAirplane *)air)->thisInTheList);
	}
	return -1;
}

FsAirplane *FsSimulation::FindAirplaneByYsfId(int id) const
{
	if(id>0)
	{
		FsAirplane *air;
		air=NULL;
		while(NULL!=(air=FindNextAirplane(air)))
		{
			if(air->ysfId==id)
			{
				return air;
			}
		}
	}
	return NULL;
}

FsAirplane *FsSimulation::FindAirplaneByName(const char name[]) const
{
	if(name!=NULL)
	{
		FsAirplane *air;
		air=NULL;
		while(NULL!=(air=FindNextAirplane(air)))
		{
			if(strcmp(air->name,name)==0)
			{
				return air;
			}
		}
	}
	return NULL;
}

const FsAirplane *FsSimulation::FindFirstAliveAirplane(const FsAirplane *afterThis) const
{
	auto ptr=afterThis;
	for(int i=0; i<GetNumAirplane(); ++i)
	{
		ptr=FindNextAirplane(ptr);
		if(NULL!=ptr && YSTRUE==ptr->IsAlive())
		{
			return ptr;
		}
	}
	return NULL;
}

int FsSimulation::GetNumGround(void)
{
	return groundList.GetN();
}

int FsSimulation::GetNumPrimaryGroundTarget(FSIFF iff) const
{
	FsGround *gnd;
	int n;
	n=0;
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		if(gnd->iff==iff && gnd->primaryTarget==YSTRUE)
		{
			n++;
		}
	}
	return n;
}

FsGround *FsSimulation::FindNextGround(const FsGround *ptr) const
{
	YsListItem <FsGround> *itm;
	if(ptr!=NULL)
	{
		itm=groundList.FindNext(ptr->thisInTheList);
	}
	else
	{
		itm=groundList.FindNext(NULL);
	}

	if(itm!=NULL)
	{
		return &itm->dat;
	}
	else
	{
		return NULL;
	}
}

FsGround *FsSimulation::FindPrevGround(const FsGround *ptr) const
{
	YsListItem <FsGround> *itm;
	if(ptr!=NULL)
	{
		itm=groundList.FindPrev(ptr->thisInTheList);
	}
	else
	{
		itm=groundList.FindPrev(NULL);
	}

	if(itm!=NULL)
	{
		return &itm->dat;
	}
	else
	{
		return NULL;
	}
}

FsGround *FsSimulation::GetGroundById(int id) const
{
	YsListItem <FsGround> *seeker;
	seeker=groundList.GetItemFromId(id);
	if(seeker!=NULL)
	{
		return &seeker->dat;
	}
	else
	{
		return NULL;
	}
}

FsGround *FsSimulation::FindGround(YSHASHKEY searchKey) const
{
	FsGround *gnd;
	if(groundSearch->FindElement(gnd,searchKey)==YSOK)
	{
		return gnd;
	}
	else
	{
		return NULL;
	}
}

FsGround *FsSimulation::FindGroundByTag(const char tag[]) const
{
	FsGround *gnd;
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		if(strcmp(gnd->name,tag)==0)
		{
			return gnd;
		}
	}
	return NULL;
}

int FsSimulation::GetGroundIdFromHandle(FsExistence *gnd) const
{
	if(gnd!=NULL && FSEX_GROUND==gnd->GetType())
	{
		return groundList.GetIdFromItem(((FsGround *)gnd)->thisInTheList);
	}
	return -1;
}

FsGround *FsSimulation::FindGroundByYsfId(int id) const
{
	if(id>0)
	{
		FsGround *gnd;
		gnd=NULL;
		while(NULL!=(gnd=FindNextGround(gnd)))
		{
			if(gnd->ysfId==id)
			{
				return gnd;
			}
		}
	}
	return NULL;
}

const FsGround *FsSimulation::FindVorByTag(const char tag[]) const
{
	for(int vorIdx=0; vorIdx<vorList.GetN(); ++vorIdx)
	{
		if(0==strcmp(tag,vorList[vorIdx]->GetName()))
		{
			return vorList[vorIdx];
		}
	}
	return NULL;
}

const FsGround *FsSimulation::FindNdbByTag(const char tag[]) const
{
	for(int ndbIdx=0; ndbIdx<ndbList.GetN(); ++ndbIdx)
	{
		if(0==strcmp(tag,ndbList[ndbIdx]->GetName()))
		{
			return ndbList[ndbIdx];
		}
	}
	return NULL;
}

const FsGround *FsSimulation::FindIlsOrCarrierByTag(const char tag[]) const
{
	for(int ilsIdx=0; ilsIdx<aircraftCarrierList.GetN(); ++ilsIdx)
	{
		if(0==strcmp(tag,aircraftCarrierList[ilsIdx]->GetName()))
		{
			return aircraftCarrierList[ilsIdx];
		}
	}
	return NULL;
}

int FsSimulation::GetNumVOR(void) const
{
	return (int)vorList.GetN();
}

const FsGround *FsSimulation::GetVOR(int idx) const
{
	if(YSTRUE==vorList.IsInRange(idx))
	{
		return vorList[idx];
	}
	return NULL;
}

int FsSimulation::GetNumNDB(void) const
{
	return (int)ndbList.GetN();
}

const FsGround *FsSimulation::GetNDB(int idx) const
{
	if(YSTRUE==ndbList.IsInRange(idx))
	{
		return ndbList[idx];
	}
	return NULL;
}

YSRESULT FsSimulation::GetAirBasePosition(YsVec3 &pos,const FsSimInfo::AirBase &base) const
{
	if(YSTRUE!=base.IsCached() && YSOK!=base.Encache(this))
	{
		return YSERR;
	}

	pos=YsOrigin();

	switch(base.GetType())
	{
	case FsSimInfo::AIRPORT:
		{
			auto fld=GetField();
			if(NULL!=fld)
			{
				auto scn=GetField()->GetFieldPtr();
				auto rgn=base.GetCachedAirportRect(this);
				if(NULL!=scn && NULL!=rgn)
				{
					pos=scn->GetRectRegionCenter(rgn);
					return YSOK;
				}
			}
		}
		break;
	case FsSimInfo::CARRIER:
		{
			auto carrier=base.GetCachedCarrier(this);
			if(NULL!=carrier)
			{
				pos=carrier->GetPosition();
				return YSOK;
			}
		}
		break;
	}
	return YSERR;
}

int FsSimulation::GetNumILSFacility(void) const
{
	return (int)aircraftCarrierList.GetN();
}

FsGround *FsSimulation::GetILS(int id) const
{
	if(0<=id && id<aircraftCarrierList.GetN())
	{
		return aircraftCarrierList[id];
	}
	else
	{
		return NULL;
	}
}

YsArray <const FsGround *> FsSimulation::FindILSinRectRegion(const YsSceneryRectRegion *rgn) const
{
	YsArray <const FsGround *> ilsArray;
	auto fld=GetField();
	if(NULL!=fld)
	{
		auto scn=fld->GetFieldPtr();
		for(auto ils : aircraftCarrierList)
		{
			if(YSTRUE==scn->IsInsideRectRegion(ils->GetPosition(),rgn))
			{
				ilsArray.Append(ils);
			}
		}
	}
	return ilsArray;
}

YsArray <const YsSceneryPointSet *> FsSimulation::FindVFRApproachInRectRegion(const YsSceneryRectRegion *rgn) const
{
	YsArray <const YsSceneryPointSet *> vfrArray;

	auto fld=GetField();
	if(NULL!=fld)
	{
		auto scn=fld->GetFieldPtr();

		YsArray <const YsSceneryPointSet *> pstArray;
		scn->SearchPointSetById(pstArray,FS_MPATHID_LANDING_RUNWAY);

		for(int pstId=0; pstId<pstArray.GetN(); ++pstId)
		{
			if(2<=pstArray[pstId]->GetNumPoint())
			{
				const YsVec3 pos=pstArray[pstId]->GetTransformedPoint(0);
				if(YSTRUE==scn->IsInsideRectRegion(pos,rgn))
				{
					vfrArray.Append(pstArray[pstId]);
				}
			}
		}
	}

	return vfrArray;
}


const FsILS *FsSimulation::FindIlsFromPosition(const YsVec3 &pos,const YsAtt3 &att) const
{
	int i;
	double hdgILS;
	const FsILS *recvILS;

	recvILS=NULL;
	hdgILS=YsInfinity;
	for(i=0; i<aircraftCarrierList.GetN(); i++)
	{
		if(aircraftCarrierList[i]->IsAlive()==YSTRUE)
		{
			const FsAircraftCarrierProperty *prop;
			const FsILS *ils;

			prop=aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty();
			ils=&prop->GetILS();

			if(ils->IsInRange(pos)==YSTRUE)
			{
				if(recvILS==NULL)
				{
					recvILS=ils;
				}
				else
				{
					YsVec3 ilsPos;
					ilsPos=aircraftCarrierList[i]->Prop().GetPosition();
					ilsPos-=pos;
					att.MulInverse(ilsPos,ilsPos);
					if(YsAbs(ilsPos.z())>YsTolerance)
					{
						double x;
						x=YsAbs(ilsPos.x()/ilsPos.z());
						if(x<hdgILS)
						{
							recvILS=ils;
							hdgILS=x;
						}
					}
				}
			}
		}
	}

	return recvILS;
}

int FsSimulation::GetNumTowerView(void) const
{
	return (int)towerPosition.GetN();
}

const YsVec3 &FsSimulation::GetTowerView(int towerViewId) const
{
	if(0<=towerViewId && towerViewId<towerPosition.GetN())
	{
		return towerPosition[towerViewId];
	}
	return YsOrigin();
}

int FsSimulation::GetNumSupplyVehicle(void) const
{
	return (int)supplyList.GetN();
}

FsGround *FsSimulation::GetSupplyVehicle(int idx) const
{
	if(YSTRUE==supplyList.IsInRange(idx))
	{
		return supplyList[idx];
	}
	return NULL;
}

const class FsAirTrafficController *FsSimulation::FindAirTrafficController(unsigned int searchKey) const
{
	if(FsAirTrafficController::PrimaryAirTrafficControllerKey==searchKey)
	{
		return &primaryAtc;
	}
	return NULL;
}

class FsAirTrafficController *FsSimulation::FindAirTrafficController(unsigned int searchKey)
{
	if(FsAirTrafficController::PrimaryAirTrafficControllerKey==searchKey)
	{
		return &primaryAtc;
	}
	return NULL;
}

const class FsAirTrafficController *FsSimulation::FindAirTrafficControllerByName(const YsString &name) const
{
	if(0==name.Strcmp(FsAirTrafficController::PrimaryAirTrafficControllerName))
	{
		return &primaryAtc;
	}
	return NULL;
}

class FsAirTrafficController *FsSimulation::FindAirTrafficControllerByName(const YsString &name)
{
	if(0==name.Strcmp(FsAirTrafficController::PrimaryAirTrafficControllerName))
	{
		return &primaryAtc;
	}
	return NULL;
}

const class FsAirTrafficSequence &FsSimulation::GetAirTrafficSequence(void) const
{
	return *airTrafficSequence;
}

class FsAirTrafficSequence &FsSimulation::GetAirTrafficSequence(void)
{
	return *airTrafficSequence;
}

const FsField *FsSimulation::GetField(void) const
{
	if(fieldLoaded==YSTRUE)
	{
		return &field;
	}
	return NULL;
}

const YsSceneryPointSet *FsSimulation::SearchMotionPathByTag(const char tag[]) const
{
	YsArray <const YsSceneryPointSet *,16> pstLst;

	if(field.SearchPointSetByTag(pstLst,tag)==YSOK && pstLst.GetN()>0)
	{
		return pstLst[0];
	}

	return NULL;
}



void FsSimulation::ViewingControl(FSBUTTONFUNCTION fnc,FSUSERCONTROL userControl)
{
	const int dir=(YSTRUE!=FsGetKeyState(FSKEY_SHIFT) ? 1 : -1);

	switch(fnc)
	{
	case FSBTF_COCKPITVIEW:
		{
			const FsExistence *playerObj=GetPlayerObject();
			if(playerObj!=NULL)
			{
				YsArray <ViewModeAndIndex> viewModeAndIndex;
				viewModeAndIndex.Increment();
				viewModeAndIndex.Last().Set(FSCOCKPITVIEW,0);

				if(playerObj->GetType()==FSEX_AIRPLANE)
				{
					const FsAirplane *air=(const FsAirplane *)playerObj;
					for(int idx=0; idx<air->Prop().GetNumAdditionalView(); ++idx)
					{
						viewModeAndIndex.Increment();
						viewModeAndIndex.Last().Set(FSADDITIONALAIRPLANEVIEW,idx);
					}
					if(air->Prop().FreeFallBombIsLoaded()>0)
					{
						viewModeAndIndex.Increment();
						viewModeAndIndex.Last().Set(FSBOMBINGVIEW,0);
					}
				}
				else if(playerObj->GetType()==FSEX_GROUND)
				{
					const FsGround *gnd=(const FsGround *)playerObj;
					for(int idx=0; idx<gnd->Prop().GetNumAdditionalView(); ++idx)
					{
						viewModeAndIndex.Increment();
						viewModeAndIndex.Last().Set(FSADDITIONALAIRPLANEVIEW,idx);
					}
				}

				FSVIEWMODE nextViewMode=FSCOCKPITVIEW;
				int nextAdditionalAirplaneViewId=0;
				const int curIndex=(mainWindowViewmode==FSADDITIONALAIRPLANEVIEW ? mainWindowAdditionalAirplaneViewId : 0);
				for(int idx=0; idx<viewModeAndIndex.GetN(); ++idx)
				{
					if(mainWindowViewmode==viewModeAndIndex[idx].viewmode &&
					   curIndex==viewModeAndIndex[idx].refIndex)
					{
						nextViewMode=viewModeAndIndex.GetCyclic(idx+dir).viewmode;
						nextAdditionalAirplaneViewId=viewModeAndIndex.GetCyclic(idx+dir).refIndex;
						break;
					}
				}
				mainWindowViewmode=nextViewMode;
				mainWindowAdditionalAirplaneViewId=nextAdditionalAirplaneViewId;
			}
		}
		break;
	case FSBTF_OUTSIDEPLAYERVIEW:
		if(GetPlayerAirplane()!=NULL)
		{
			if(mainWindowViewmode==FSOUTSIDEPLAYERPLANE)
			{
				mainWindowViewmode=FSFIXEDPOINTPLAYERPLANE;
				UpdateViewpointAccordingToPlayerAirplane(500.0,YSTRUE);
			}
			else if(mainWindowViewmode==FSFIXEDPOINTPLAYERPLANE)
			{
				mainWindowViewmode=FSVARIABLEPOINTPLAYERPLANE;
				UpdateViewpointAccordingToPlayerAirplane(500.0,YSTRUE);
			}
			else if(mainWindowViewmode==FSVARIABLEPOINTPLAYERPLANE)
			{
				mainWindowViewmode=FSFROMTOPOFPLAYERPLANE;
				UpdateViewpointAccordingToPlayerAirplane(500.0,YSTRUE);
			}
			else if(mainWindowViewmode==FSFROMTOPOFPLAYERPLANE)
			{
				mainWindowViewmode=FSPLAYERPLANEFROMSIDE;
			}
			else
			{
				mainWindowViewmode=FSOUTSIDEPLAYERPLANE;
			}
		}
		break;
	case FSBTF_COMPUTERAIRPLANEVIEW:
		if(0<=dir)
		{
			if(mainWindowViewmode==FSANOTHERAIRPLANE)
			{
				focusAir=FindNextAirplane(focusAir);
				for(int i=0; i<2; ++i) // NULL and playerPlane may appear in a sequence.
				{
					if(focusAir==NULL || focusAir==GetPlayerObject())
					{
						focusAir=FindNextAirplane(focusAir);
					}
				}
			}
			else
			{
				mainWindowViewmode=FSANOTHERAIRPLANE;
				focusAir=FindNextAirplane(NULL);
			}
		}
		else
		{
			if(mainWindowViewmode==FSANOTHERAIRPLANE)
			{
				focusAir=FindPrevAirplane(focusAir);
				for(int i=0; i<2; ++i) // NULL and playerPlane may appear in a sequence.
				{
					if(focusAir==NULL || focusAir==GetPlayerObject())
					{
						focusAir=FindPrevAirplane(focusAir);
					}
				}
			}
			else
			{
				mainWindowViewmode=FSANOTHERAIRPLANE;
				focusAir=FindPrevAirplane(NULL);
			}
		}
		break;
	case FSBTF_WEAPONVIEW:
		{
			mainWindowViewmode=FSMISSILEVIEW;
		}
		break;
	case FSBTF_CHANGEAIRPLANE:
		if(EveryAirplaneIsRecordedAirplane()==YSTRUE || userControl==FSUSC_VIEWCONTROLONLY)
		{
			if(CheckNoExtAirView()!=YSTRUE)  // 2006/07/19, corrected 2006/08/25
			{
				int i,nAir;
				FsAirplane *next;

				nAir=GetNumAirplane();
				next=GetPlayerAirplane();
				for(i=0; i<nAir; i++)
				{
					next=FindNextAirplane(next);
					if(next==NULL)
					{
						next=FindNextAirplane(next);
					}
					if(next->IsAlive()==YSTRUE)
					{
						SetPlayerAirplane(next);
						break;
					}
				}
			}
			else
			{
				AddTimedMessage("The server does not allow third airplane view.");
			}
		}
		break;
	case FSBTF_ILSVIEW:
	case FSBTF_CONTROLTOWERVIEW:
		{
			YsArray <ViewModeAndIndexAndPosition> view;
			if(FSBTF_ILSVIEW==fnc)
			{
				view=MakeAvailableILSView();
			}
			else
			{
				view=MakeAvailableTowerView();
			}
			if(0<view.GetN())
			{
				if(NULL==focusAir || (FSCARRIERVIEW!=mainWindowViewmode && FSTOWERVIEW!=mainWindowViewmode))
				{
					focusAir=GetPlayerAirplane();
					if(NULL==focusAir || YSTRUE!=focusAir->IsAlive())
					{
						focusAir=FindFirstAliveAirplane(focusAir);
					}
				}
				if(NULL!=focusAir)
				{
					YsArray <double> dist(view.GetN(),NULL);
					for(int idx=0; idx<view.GetN(); ++idx)
					{
						dist[idx]=(view[idx].pos-focusAir->GetPosition()).GetSquareLength();
					}
					YsQuickSort(dist.GetN(),dist.GetEditableArray(),view.GetEditableArray());

					FSVIEWMODE curMode=mainWindowViewmode;
					int curIndex=0;
					if(FSCARRIERVIEW==mainWindowViewmode && NULL!=focusGnd)
					{
						curIndex=(int)focusGnd->SearchKey();
					}
					else if(FSTOWERVIEW==mainWindowViewmode)
					{
						curIndex=towerViewId;
					}

					YSSIZE_T nextIndex=0;
					for(YSSIZE_T idx=0; idx<view.GetN(); ++idx)
					{
						if(curMode==view[idx].viewmode && curIndex==view[idx].refIndex)
						{
							nextIndex=(idx+view.GetN()+dir)%view.GetN();
							break;
						}
					}

					auto &nextView=view[nextIndex];
					if(FSCARRIERVIEW==nextView.viewmode)
					{
						focusGnd=FindGround((YSHASHKEY)nextView.refIndex);
						if(NULL!=focusGnd)
						{
							mainWindowViewmode=FSCARRIERVIEW;
						}
					}
					else if(FSTOWERVIEW==nextView.viewmode)
					{
						mainWindowViewmode=FSTOWERVIEW;
						towerViewId=nextView.refIndex;
						towerViewPos=nextView.pos;
					}
				}
			}
		}
		break;
	case FSBTF_OUTSIDEPLAYERVIEW2:
	case FSBTF_OUTSIDEPLAYERVIEW3:
		if(FSOUTSIDEPLAYER2!=mainWindowViewmode && FSOUTSIDEPLAYER3!=mainWindowViewmode)
		{
			relViewAtt.SetB(0.0);
			focusAir=GetPlayerAirplane();
		}

		if(FSOUTSIDEPLAYER2==mainWindowViewmode)
		{
			mainWindowViewmode=FSOUTSIDEPLAYER3;
		}
		else
		{
			mainWindowViewmode=FSOUTSIDEPLAYER2;
		}
		break;
	case FSBTF_GHOSTVIEW:
		{
			if(mainWindowViewmode!=FSGHOSTVIEW)
			{
				mainWindowViewmode=FSGHOSTVIEW;
				ghostViewSpeed=0.0;
			}
		}
		break;
	case FSBTF_VIEWZOOM:
		if(viewMagUser<12.0)
		{
			viewMagUser*=1.1;
		}
		break;
	case FSBTF_VIEWMOOZ:
		if(viewMagUser>1.0)
		{
			viewMagUser/=1.1;
		}
		break;
	case FSBTF_SWITCHVIEWTARGET:
		{
			const YSBOOL includePlayer=YSTRUE;
			auto targetAirCandidate=MakeAvailableViewTargetAirplane(includePlayer);
			if(0<targetAirCandidate.GetN())
			{
				const int dir=(YSTRUE!=FsGetKeyState(FSKEY_SHIFT) ? 1 : -1);

				auto nextFocusAir=targetAirCandidate[0];
				for(int idx=0; idx<targetAirCandidate.GetN(); ++idx)
				{
					if(targetAirCandidate[idx]==focusAir)
					{
						nextFocusAir=targetAirCandidate.GetCyclic(idx+dir);
						break;
					}
				}
				focusAir=nextFocusAir;
			}
		}
		break;
	}
}

YsArray <FsSimulation::ViewModeAndIndexAndPosition> FsSimulation::MakeAvailableILSView(void) const
{
	YsArray <ViewModeAndIndexAndPosition> view;

	// Tower: refIndex=towerIdx
	// ILS/Carrier: refIndex=gnd search key

	for(auto carrierPtr : aircraftCarrierList)
	{
		const FsAircraftCarrierProperty *carrierProp=carrierPtr->Prop().GetAircraftCarrierProperty();
		if(YSTRUE==carrierProp->CanBeViewpoint())
		{
			view.Increment();
			view.Last().Set(FSCARRIERVIEW,(int)carrierPtr->SearchKey(),carrierPtr->GetPosition());
		}
	}

	return view;
}

YsArray <FsSimulation::ViewModeAndIndexAndPosition> FsSimulation::MakeAvailableTowerView(void) const
{
	YsArray <ViewModeAndIndexAndPosition> view;

	// Tower: refIndex=towerIdx
	// ILS/Carrier: refIndex=gnd search key

	for(int towerIdx=0; towerIdx<towerPosition.GetN(); ++towerIdx)
	{
		view.Increment();
		view.Last().Set(FSTOWERVIEW,towerIdx,towerPosition[towerIdx]);
	}

	return view;
}

YsArray <const FsAirplane *> FsSimulation::MakeAvailableViewTargetAirplane(YSBOOL includePlayer) const
{
	YsArray <const FsAirplane *> allAir;
	YsArray <double> dist;

	auto *playerPlane=GetPlayerAirplane();
	const YsVec3 ref=(NULL!=playerPlane ? playerPlane->GetPosition() : YsOrigin());

	for(const FsAirplane *air=NULL; NULL!=(air=FindNextAirplane(air)); )
	{
		if(YSTRUE==air->IsAlive() && (YSTRUE==includePlayer || playerPlane!=air))
		{
			allAir.Append(air);
			dist.Append((ref-air->GetPosition()).GetSquareLength());
		}
	}

	if(NULL!=playerPlane)
	{
		YsQuickSort(dist.GetN(),dist.GetEditableArray(),allAir.GetEditableArray());
	}

	return allAir;
}


#ifdef __APPLE__
extern "C" int FsPassedTimeC(void);
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

double FsSimulation::PassedTime(void)  // <- This function must wait at least 0.020 seconds.
                                       //    Otherwise, SimMove will not be called in
                                       //    SimulateOneStep, and FsAirplane's collision shell
                                       //    location will not be updated -> Yields unexpected crash.
{
	unsigned long long clk=FsSubSecondTimer();
	if(clk<lastTime)
	{
		lastTime=clk;
	}
	double passed=(double)(clk-lastTime)/1000.0;
	if(passed<0.010)
	{
		FsSleep(5);  // Let's give 10ms rest
	}
	while(passed<0.010 && lastTime<=clk)
	{
		clk=FsSubSecondTimer();
		passed=(double)(clk-lastTime)/1000.0;
	}

	if(clk<lastTime)  // Underflow took place.
	{
		passed=0.02;
	}

	lastTime=clk;

	return passed;
}

YSRESULT FsSimulation::SetAllowedWeaponType(unsigned int allowedWeaponType)
{
	this->allowedWeaponType=allowedWeaponType;
	return YSOK;
}

void FsSimulation::AllowGun(YSBOOL a)
{
	if(a==YSTRUE)
	{
		allowedWeaponType|=FSWEAPON_ALLOWGUN;
	}
	else
	{
		allowedWeaponType&=(~FSWEAPON_ALLOWGUN);
	}
}

void FsSimulation::AllowAAM(YSBOOL a)
{
	if(a==YSTRUE)
	{
		allowedWeaponType|=FSWEAPON_ALLOWAAM;
	}
	else
	{
		allowedWeaponType&=(~FSWEAPON_ALLOWAAM);
	}
}

void FsSimulation::AllowAGM(YSBOOL a)
{
	if(a==YSTRUE)
	{
		allowedWeaponType|=FSWEAPON_ALLOWAGM;
	}
	else
	{
		allowedWeaponType&=(~FSWEAPON_ALLOWAGM);
	}
}

void FsSimulation::AllowBomb(YSBOOL a)
{
	if(a==YSTRUE)
	{
		allowedWeaponType|=FSWEAPON_ALLOWBOMB;
	}
	else
	{
		allowedWeaponType&=(~FSWEAPON_ALLOWBOMB);
	}
}

void FsSimulation::AllowRocket(YSBOOL a)
{
	if(a==YSTRUE)
	{
		allowedWeaponType|=FSWEAPON_ALLOWROCKET;
	}
	else
	{
		allowedWeaponType&=(~FSWEAPON_ALLOWROCKET);
	}
}

YSRESULT FsSimulation::LoadConfigFile(const wchar_t fn[],YSBOOL changeEnvironment)
{
	YSRESULT r=cfgPtr->Load(fn);

	if(changeEnvironment==YSTRUE)
	{
		env=cfgPtr->env;
		weather->SetFog(cfgPtr->drawFog);
		weather->SetFogVisibility(cfgPtr->fogVisibility);
	}

	return r;
}

YSBOOL FsSimulation::EveryAirplaneIsRecordedAirplane(void) const
{
	if(GetNumAirplane()==0)
	{
		return YSFALSE;
	}

	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->isPlayingRecord!=YSTRUE)
		{
			return YSFALSE;
		}
	}
	return YSTRUE;
}

YSBOOL FsSimulation::AtLeastOneAirplaneIsRecordedAirplane(void)
{
	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->isPlayingRecord==YSTRUE)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

double FsSimulation::LastRecordedTime(void) const
{
	FsAirplane *air;
	double lastRecTime,lastTime;

	lastRecTime=0.0;

	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->rec!=NULL && air->rec->GetLastElement(lastTime)!=NULL && lastRecTime<lastTime)
		{
			lastRecTime=lastTime;
		}
	}

	return lastRecTime;
}

YSRESULT FsSimulation::GetLoadedField(YsString &fieldName,YsVec3 &pos,YsAtt3 &att)
{
	if(fieldLoaded==YSTRUE)
	{
		pos=field.GetPosition();
		att=field.GetAttitude();
		fieldName.Set(field.GetIdName());
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::GetRunwayRectFromPosition(const YsSceneryRectRegion *&rgn,YsVec3 rect[4],const YsVec3 &pos) const
{
	rgn=NULL;

	YsArray <const YsSceneryRectRegion *,16> rgnLst;
	if(YSOK==GetRunwayRectFromPositionAll(rgnLst,pos))
	{
		int i;
		YsVec3 bbx[2];
		double lng;
		lng=0.0;
		for(i=0; i<rgnLst.GetN(); i++)
		{
			if(rgnLst[i]->GetId()==FS_RGNID_RUNWAY)
			{
				rgnLst[i]->GetBoundingBox(bbx);

				if(fabs(bbx[0].x()-bbx[1].x())>lng || fabs(bbx[0].z()-bbx[1].z())>lng)
				{
					lng=YsGreater(fabs(bbx[0].x()-bbx[1].x()),fabs(bbx[0].z()-bbx[1].z()));
					rgn=rgnLst[i];
					field.GetFieldRegionRect(rect,rgnLst[i]);
				}
			}
		}
		printf("Found Runway Length=%lf\n",lng);
		if(lng>500.0)
		{
			return YSOK;
		}
	}
	return YSERR;
}

const YsSceneryRectRegion *FsSimulation::FindAirportFromPosition(const YsVec3 &pos) const
{
	const FsField *fld=GetField();
	if(NULL!=fld)
	{
		auto scenery=fld->GetFieldPtr();

		YsArray <const YsSceneryRectRegion *,16> rgnList;
		scenery->GetRectRegionFromPoint(rgnList,pos);
		for(auto rgn : rgnList)
		{
			if(FS_RGNID_AIRPORT_AREA==rgn->GetId())
			{
				return rgn;
			}
		}
	}

	return NULL;
}

YSRESULT FsSimulation::FindRunwayClearingPathCandidate(
	    YsArray <YsPair <const YsSceneryPointSet *,double>,16> &pathCan,
	    const FsExistence &obj) const
{
	pathCan.Clear();

	if(fieldLoaded==YSTRUE)
	{
		const YsAtt3 &att=obj.GetAttitude();

		YsVec3 turnRadiusOffset=YsOrigin();
		if(YSTRUE==obj.IsAirplane())
		{
			const FsAirplane *air=(FsAirplane *)&obj;
			turnRadiusOffset=att.GetForwardVector()*air->Prop().CalculateTurnRatiusOnGround(air->Prop().GetVelocity());
		}
		const YsVec3 &pos=obj.GetPosition()+turnRadiusOffset;

		YsArray <const YsSceneryRectRegion *,16> rgnLst;
		if(YSTRUE==obj.rectRgnCached)
		{
			rgnLst=obj.rectRgnCache;
		}
		else
		{
			field.GetFieldRegion(rgnLst,pos.x(),pos.z());
		}

		for(auto idx=rgnLst.GetN()-1; 0<=idx; --idx)
		{
			if(FS_RGNID_RUNWAY!=rgnLst[idx]->GetId())
			{
				rgnLst.DeleteBySwapping(idx);
			}
		}


		const YsVec3 ev=att.GetForwardVector();

		YsArray <const YsSceneryPointSet *,16> pstLst;
		if(0<rgnLst.GetN())
		{
			field.SearchPointSetById(pstLst,FS_MPATHID_TAXI_TO_RAMP /* =10 */);

			for(int rgnIdx=0; rgnIdx<rgnLst.GetN(); ++rgnIdx)
			{
				YsVec3 rect[4],rwyDir,relRwyDir;
				field.GetFieldRegionRect(rect,rgnLst[rgnIdx]);

				rwyDir=YsLonger(rect[1]-rect[0],rect[2]-rect[1]);  // vL
				att.MulInverse(relRwyDir,rwyDir);
				if(fabs(relRwyDir.x())<0.087*fabs(relRwyDir.z()))  // 0.087=tan(5deg)
				{
					for(int pstIdx=0; pstIdx<pstLst.GetN(); ++pstIdx)
					{
						YsVec3 mPathFirstPoint;
						field.GetFirstPointOfPointSet(mPathFirstPoint,pstLst[pstIdx]);
						if(YsCheckInsidePolygon3(mPathFirstPoint,4,rect)==YSINSIDE)
						{
							YsVec3 relMPath0;
							att.MulInverse(relMPath0,mPathFirstPoint-pos);
							if(relMPath0.z()>0.0 && fabs(relMPath0.x())<0.577350*relMPath0.z()) //30deg envlp.
							{
								pathCan.Increment();
								pathCan.GetEnd().a=pstLst[pstIdx];
								pathCan.GetEnd().b=relMPath0.z();
							}
						}
					}
				}
			}

			YsArray <double,16> distArray(pathCan.GetN(),NULL);
			for(int pathIdx=0; pathIdx<pathCan.GetN(); ++pathIdx)
			{
				distArray[pathIdx]=pathCan[pathIdx].b;
			}

			YsQuickSort <double,YsPair <const YsSceneryPointSet *,double> > (distArray.GetN(),distArray,pathCan);

			if(0<pathCan.GetN())
			{
				return YSOK;
			}
		}
	}

	return YSERR;
}

YSRESULT FsSimulation::FindRunwayClearingPath(YsArray <YsVec3,16> &rwClearPath,const FsExistence &obj)
{
    YsArray <YsPair <const YsSceneryPointSet *,double>,16> pathCan;
	if(YSOK==FindRunwayClearingPathCandidate(pathCan,obj) && 0<pathCan.GetN())
	{
		field.GetPointSet(rwClearPath,pathCan[0].a);
		return YSOK;
	}
	rwClearPath.Clear();
	return YSERR;
}

double FsSimulation::GetFieldElevation(const double &x,const double &z) const
{
	double maxElv;

	maxElv=0.0;

	double elv;
	field.GetFieldElevation(elv,x,z);
	if(elv>maxElv)
	{
		maxElv=elv;
	}

	return maxElv;
}

double FsSimulation::GetBaseElevation(void) const
{
	return field.GetBaseElevation();
}

YSSCNAREATYPE FsSimulation::GetAreaType(const YsVec3 &pos)
{
	return field.GetAreaType(pos);
}

YSRESULT FsSimulation::GetRegionRect(YsVec3 rect[4],const YsSceneryRectRegion *rgn) const
{
	const FsField *fld=GetField();
	if(NULL!=fld && NULL!=rgn)
	{
		fld->GetFieldRegionRect(rect,rgn);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::GetRegionRectById(YsVec3 rect[4],int id) const
{
	const FsField *fld;
	YsArray <const YsSceneryRectRegion *,16> rgn;

	if(NULL!=(fld=GetField()))
	{
		if(fld->SearchFieldRegionById(rgn,id)==YSOK && rgn.GetN()>0)
		{
			fld->GetFieldRegionRect(rect,rgn[0]);
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsSimulation::GetRegionRectByTag(YsVec3 rect[4],const char tag[]) const
{
	const FsField *fld;
	YsArray <const YsSceneryRectRegion *,16> rgn;

	if(NULL!=(fld=GetField()))
	{
		if(fld->SearchFieldRegionByTag(rgn,tag)==YSOK && rgn.GetN()>0)
		{
			fld->GetFieldRegionRect(rect,rgn[0]);
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsSimulation::GetRegionCenterById(YsVec3 &cen,int id) const
{
	YsVec3 rect[4];
	if(GetRegionRectById(rect,id)==YSOK)
	{
		cen=(rect[0]+rect[1]+rect[2]+rect[3])/4.0;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::GetRegionCenterByTag(YsVec3 &cen,const char tag[]) const
{
	YsVec3 rect[4];
	if(GetRegionRectByTag(rect,tag)==YSOK)
	{
		cen=(rect[0]+rect[1]+rect[2]+rect[3])/4.0;
		return YSOK;
	}
	return YSERR;
}

void FsSimulation::MakeSortedIlsList(YsArray <FsGround *,64> &ilsList,YsArray <double,64> &ilsDist,const YsVec3 &refp) const
{
	YsVec3 diff;

	ilsList.Set(0,NULL);
	ilsDist.Set(0,NULL);

	for(int i=0; i<GetNumILSFacility(); i++)
	{
		FsGround *ils=GetILS(i);
		if(NULL!=ils)
		{
			const FsAircraftCarrierProperty *acProp=ils->Prop().GetAircraftCarrierProperty();
			if(NULL!=acProp)
			{
				const FsILS &ilsProp=acProp->GetILS();
				if(YsTolerance<ilsProp.GetRange())
				{
					YsVec3 diff=refp-ils->GetPosition();
					diff.SetY(0.0);

					const double dist=diff.GetLength();

					ilsList.Append(ils);
					ilsDist.Append(dist);
				}
			}
		}
	}

	YsQuickSort <double,FsGround *> (ilsDist.GetN(),ilsDist,ilsList);
}

void FsSimulation::MakeSortedVorList(YsArray <FsGround *,64> &vorInRange,YsArray <double,64> &vorDist,const YsVec3 &refp) const
{
	int i;
	YsVec3 diff;

	vorInRange.Set(0,NULL);
	vorDist.Set(0,NULL);

	for(i=0; i<vorList.GetN(); i++)
	{
		if(vorList[i]->IsAlive()==YSTRUE)
		{
			double dist;
			diff=refp-vorList[i]->GetPosition();
			diff.SetY(0.0);
			dist=diff.GetLength();
			if(dist<=vorList[i]->Prop().GetVorRange())
			{
				vorInRange.Append(vorList[i]);
				vorDist.Append(dist);
			}
		}
	}

	for(i=0; i<aircraftCarrierList.GetN(); i++)
	{
		if(aircraftCarrierList[i]->IsAlive()==YSTRUE &&
		   aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty()!=NULL)
		{
			double dist;
			diff=refp-aircraftCarrierList[i]->GetPosition();
			diff.SetY(0.0);
			dist=diff.GetLength();
			if(dist<aircraftCarrierList[i]->Prop().GetAircraftCarrierProperty()->GetILS().GetRange())
			{
				vorInRange.Append(aircraftCarrierList[i]);
				vorDist.Append(dist);
			}
		}
	}

	YsQuickSort <double,FsGround *> (vorDist.GetN(),vorDist,vorInRange);
}

void FsSimulation::MakeSortedNdbList(YsArray <FsGround *,64> &ndbInRange,YsArray <double,64> &ndbDist,const YsVec3 &refp) const
{
	int i;
	YsVec3 diff;

	ndbInRange.Set(0,NULL);
	ndbDist.Set(0,NULL);

	for(i=0; i<ndbList.GetN(); i++)
	{
		if(ndbList[i]->IsAlive()==YSTRUE)
		{
			double dist;
			diff=refp-ndbList[i]->GetPosition();
			diff.SetY(0.0);
			dist=diff.GetLength();
			if(dist<=ndbList[i]->Prop().GetNdbRange())
			{
				ndbInRange.Append(ndbList[i]);
				ndbDist.Append(dist);
			}
		}
	}

	YsQuickSort <double,FsGround *> (ndbDist.GetN(),ndbDist,ndbInRange);
}

void FsSimulation::GetFieldElevationAndNormal(double &elv,YsVec3 &nom,const double &x,const double &z)
{
	double maxElv;
	YsVec3 nomAtMaxElv;
	const FsField *fld;
	YsVec3 n;

	maxElv=0.0;
	nomAtMaxElv=YsYVec();
	if(NULL!=(fld=GetField()))
	{
		double e;
		fld->GetFieldElevationAndNormal(e,n,x,z);
		if(e>maxElv)
		{
			maxElv=e;
			nomAtMaxElv=n;
		}
	}
	elv=maxElv;
	nom=nomAtMaxElv;
}

double FsSimulation::GetFieldMagneticVariation(void) const
{
	const YsScenery *scn=field.GetFieldPtr();
	if(NULL!=scn)
	{
		return scn->GetMagneticVariation();
	}
	return 0.0;
}

double FsSimulation::TrueHeadingToMagneticHeading(const double trueHeading) const
{
	// In YsScenery: magnetic heading=true heading+magnetic variation
	// 2018/04/28
	//   Forget about 2018/04/06 comment.  I have already defined internal mag-var as difference
	//   from true heading to magnetic heading.
	// 2018/04/06
	//   http://www.atpforum.eu/forum/technical-subjects/-061-general-navigation/9139-variation-and-deviation
	//   Looks like the standard is W=minus and E=plus.
	//   I need to flip it.
	// 10W  -> -10.0
	// 10E  -> +10.0
	//   EG.  Near Pittsburgh 9W  TRUE 360DEG=MAGNETIC 9DEG    MAG=TRUE+9
	return trueHeading+GetFieldMagneticVariation();
}
double FsSimulation::InternalHeadingToTrueHeading(const double internalHeading) const
{
	return -internalHeading;
}
double FsSimulation::TrueHeadingToInternalHeading(const double internalHeading) const
{
	return -internalHeading;
}
double FsSimulation::InternalHeadingToMagnetic(const double trueHeading) const
{
	return TrueHeadingToMagneticHeading(InternalHeadingToTrueHeading(trueHeading));
	// Why plus?  Internal hdg and world hdg are inverted.  
	// By the way, in the conventional heading definition, (magnetic heading) = (true heading) - (variation)
}

double FsSimulation::MagneticHeadingToTrueHeading(const double magHeading) const
{
	return magHeading-GetFieldMagneticVariation();
}

double FsSimulation::MagneticHeadingToInternalHeading(const double magHeading) const
{
	return TrueHeadingToInternalHeading(MagneticHeadingToTrueHeading(magHeading));
}

void FsSimulation::AdjustStartPositionByRunwayRect
	   (YsVec3 &pos,const YsVec3 rwRect[],const YsVec3 &offsetFromRwThr,const YsAtt3 &att)
{
	YsVec3 rwDir,rwCen,dx,dy,rwEnd[2],rwThr;

	dx=rwRect[1]-rwRect[0];
	dy=rwRect[2]-rwRect[1];
	if(dx.GetSquareLength()>dy.GetSquareLength())
	{
		rwDir=dx;
		rwDir.Normalize();
		rwEnd[0]=(rwRect[0]+rwRect[3])/2.0;
		rwEnd[1]=(rwRect[1]+rwRect[2])/2.0;
	}
	else
	{
		rwDir=dy;
		rwDir.Normalize();
		rwEnd[0]=(rwRect[0]+rwRect[1])/2.0;
		rwEnd[1]=(rwRect[2]+rwRect[3])/2.0;
	}


	if(att.GetForwardVector()*rwDir<0.0)
	{
		rwDir=-rwDir;
	}

	if(att.GetForwardVector()*rwEnd[0]<att.GetForwardVector()*rwEnd[1])
	{
		rwThr=rwEnd[0];
	}
	else
	{
		rwThr=rwEnd[1];
	}


	pos=rwThr+rwDir*offsetFromRwThr.z();
	rwDir.RotateXZ(-YsPi/2.0);
	pos+=rwDir*offsetFromRwThr.x();
}

const FsWeather &FsSimulation::GetWeather(void) const
{
	return *weather;
}

FsWeather &FsSimulation::GetWeather(void)
{
	return *weather;
}

void FsSimulation::ConstrainRunwayVectorFromRunwayRect(YsVec3 &rwVec,const YsVec3 &tdPos) const
{
	YsArray <const YsSceneryRectRegion *,16> rgnLst;
	if(YSOK==GetRunwayRectFromPositionAll(rgnLst,tdPos))
	{
		YsVec3 sigDir=YsOrigin();
		double maxDotProd=0.0;
		YSBOOL found=YSFALSE;
		for(YSSIZE_T rgnIdx=0; rgnIdx<rgnLst.GetN(); ++rgnIdx)
		{
			YsVec3 rect[4];
			GetRegionRect(rect,rgnLst[rgnIdx]);

			const YsVec3 v1=rect[1]-rect[0];
			const YsVec3 v2=rect[2]-rect[1];

			const YsVec3 &realRwDir=YsUnitVector(v1.GetSquareLength()<v2.GetSquareLength() ? v2 : v1);
			const double dotProd=fabs(realRwDir*rwVec);
			if(maxDotProd<dotProd)
			{
				maxDotProd=dotProd;
				sigDir=realRwDir;
				found=YSTRUE;
			}
		}

		if(YSTRUE==found)
		{
			if(0.0<rwVec*sigDir)
			{
				rwVec=sigDir;
			}
			else
			{
				rwVec=-sigDir;
			}
		}
	}
}

void FsSimulation::FindGroundToAirThreat(YsArray <FsSimInfo::GndToAirThreat,16> &threatFound,const YsVec3 &A,const YsVec3 &B,const FsAirplane &air) const
{
	threatFound.CleanUp();

	if(FS_IFF_NEUTRAL==air.GetIff())
	{
		return;
	}

	// Maximum weapon range is unknown.  So, I cannot use lattice.
	for(const FsGround *gnd=NULL; NULL!=(gnd=FindNextGround(gnd)); )
	{
		if(FS_IFF_NEUTRAL!=gnd->GetIff() && air.GetIff()!=gnd->GetIff())
		{
			double range=0.0;
			if(0<gnd->Prop().GetNumSAM())
			{
				range=YsGreater(range,gnd->Prop().GetSAMRange());
			}
			if(0<gnd->Prop().GetNumAaaBullet())
			{
				range=YsGreater(range,gnd->Prop().GetAAARange());
			}

			if(YsTolerance<range &&
			   YsGetPointLineDistance3(A,B,gnd->GetPosition())<range)
			{
				threatFound.Increment();
				threatFound.Last().objKey=gnd->SearchKey();
				threatFound.Last().range=range;
			}
		}
	}
}

const FsFlightConfig &FsSimulation::GetConfig(void) const
{
	return *cfgPtr;
}

void FsSimulation::SetBlackOut(YSBOOL blackOut)
{
	cfgPtr->blackOut=blackOut;
}

void FsSimulation::SetMidAirCollision(YSBOOL midAirCollision)
{
	cfgPtr->midAirCollision=midAirCollision;
}

void FsSimulation::SetCanLandAnywhere(YSBOOL canLandAnywhere)
{
	cfgPtr->canLandAnywhere=canLandAnywhere;
}

void FsSimulation::SetShowUserName(int showUserName)
{
	cfgPtr->showUserName=showUserName;
}

void FsSimulation::SetDisableThirdAirplaneView(YSBOOL sw)
{
	cfgPtr->noExtAirView=sw;
}

FsWeaponHolder &FsSimulation::GetWeaponStore(void)
{
	return bulletHolder;
}

const FsWeaponHolder &FsSimulation::GetWeaponStore(void) const
{
	return bulletHolder;
}

const FsWeapon *FsSimulation::FindNextActiveWeapon(const FsWeapon *wpn) const
{
	return bulletHolder.FindNextActiveWeapon(wpn);
}

YSBOOL FsSimulation::IsWeaponGuidedToTarget(int weaponId) const
{
	const FsWeapon *wpn;
	wpn=bulletHolder.GetWeapon(weaponId);
	if(wpn!=NULL &&
	   wpn->lifeRemain>YsTolerance &&
	   wpn->timeRemain>YsTolerance &&
	  (wpn->type==FSWEAPON_AIM9 || wpn->type==FSWEAPON_AIM9X || wpn->type==FSWEAPON_AGM65) &&
	   wpn->target!=NULL)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSimulation::IsWeaponShotBy(int weaponId,FsAirplane *air) const
{
	const FsWeapon *wpn;
	wpn=bulletHolder.GetWeapon(weaponId);
	if(wpn!=NULL &&
	   wpn->lifeRemain>YsTolerance &&
	   wpn->timeRemain>YsTolerance &&
	   wpn->firedBy==air)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSimulation::IsPlayerAirplane(const FsAirplane *air) const
{
	if(GetPlayerAirplane()==air)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSimulation::IsOnRunway(const YsVec3 &pos) const
{
	const FsField *fld;

	if(NULL!=(fld=GetField()))
	{
		YsArray <const YsSceneryRectRegion *,16> rgn;
		if(fld->GetFieldRegion(rgn,pos.x(),pos.z())==YSOK)  // 2003/01/04
		{
			int i;
			for(i=0; i<rgn.GetN(); i++)
			{
				if(IsSafeTerrainRegionId(rgn[i]->GetId())==YSTRUE)
				{
					return YSTRUE;
				}
			}
		}
	}
	return YSFALSE;
}

YSBOOL FsSimulation::IsSafeTerrainRegionId(int id)
{
	if(id==1 || id==2)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSimulation::CheckRunwayLength(const YsVec3 &org,const YsVec3 &dir,const double &lng) const
{
	YsVec3 uDir;
	uDir=dir;
	uDir.Normalize();
	if(IsOnRunway(org)==YSTRUE &&
	   IsOnRunway(org+uDir*lng)==YSTRUE &&
	   IsOnRunway(org+uDir*lng/4.0)==YSTRUE &&
	   IsOnRunway(org+uDir*lng/2.0)==YSTRUE &&
	   IsOnRunway(org+uDir*lng*3.0/4.0)==YSTRUE)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSimulation::IsRunwayClear(const YsVec3 &org,const YsVec3 &dir,const double &lng) const
{
	YsVec3 tst;
	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->Prop().IsAlive()==YSTRUE && air->Prop().IsOnGround()==YSTRUE)
		{
			if(YsGetNearestPointOnLine3(tst,org,org+dir,air->GetPosition())==YSOK)
			{
				if((tst-org)*dir>0.0 &&
				   (tst-air->GetPosition()).GetSquareLength()<YsSqr(30.0) &&
				   (tst-org).GetSquareLength()<YsSqr(lng))
				{
					return YSFALSE;
				}
			}
		}
	}
	return YSTRUE;
}

FsGround *FsSimulation::FindNearbySupplyTruck(YSBOOL &fuel,YSBOOL &ammo,const FsExistence &air) const
{
	double airRad,objRad,distSq;
	int i;
	FsGround *supplyTruck=NULL;

	fuel=YSFALSE;
	ammo=YSFALSE;

	airRad=air.GetApproximatedCollideRadius();
	forYsArray(i,supplyList)
	{
		if(supplyList[i]->IsAlive()==YSTRUE)
		{
			objRad=supplyList[i]->Prop().GetOutsideRadius();
			distSq=(air.GetPosition()-supplyList[i]->GetPosition()).GetSquareLength();
			if(distSq<YsSqr(airRad+objRad+10.0))
			{
				if(supplyList[i]->Prop().chFlags & FsGroundProperty::YSGP_SUPPLYFUEL)
				{
					supplyTruck=supplyList[i];
					fuel=YSTRUE;
				}
				if(supplyList[i]->Prop().chFlags & FsGroundProperty::YSGP_SUPPLYAMMO)
				{
					supplyTruck=supplyList[i];
					ammo=YSTRUE;
				}
			}
		}
	}

	if(fuel==YSTRUE || ammo==YSTRUE)
	{
		return supplyTruck;
	}
	return NULL;
}

FsGround *FsSimulation::FindNearestSupplyTruckInTheSameRamp(const YsVec3 &pos) const
{
	FsGround *nearestSupplyTruck=NULL;
	YsArray <const YsSceneryRectRegion *,16> rgnLst;
	if(field.GetFieldRegion(rgnLst,pos.x(),pos.z())==YSOK)
	{
		for(int rgnIdx=0; rgnIdx<rgnLst.GetN(); ++rgnIdx)
		{
			if(rgnLst[rgnIdx]->GetId()==FS_RGNID_TAXIWAY ||
			   rgnLst[rgnIdx]->GetId()==FS_RGNID_RUNWAY)
			{
				for(int supplyIdx=0; supplyIdx<supplyList.GetN(); ++supplyIdx)
				{
					if(FSNET_LOCAL==supplyList[supplyIdx]->netType &&
					   YsTolerance>supplyList[supplyIdx]->Prop().GetVelocity() &&
					   0==supplyList[supplyIdx]->GetNumAutoDrive() &&
					   YSTRUE==field.GetFieldPtr()->IsInsideRectRegion(supplyList[supplyIdx]->GetPosition(),rgnLst[rgnIdx]))
					{
						if(NULL==nearestSupplyTruck)
						{
							nearestSupplyTruck=supplyList[supplyIdx];
						}
						else
						{
							const double d0=(nearestSupplyTruck->GetPosition()-pos).GetSquareLength();
							const double d=(supplyList[supplyIdx]->GetPosition()-pos).GetSquareLength();
							if(d<d0)
							{
								nearestSupplyTruck=supplyList[supplyIdx];
							}
						}
					}
				}
			}
		}
	}
	return nearestSupplyTruck;
}

YSRESULT FsSimulation::FindNearbyBoardableVehicle(YsArray <const FsExistence *,16> &objArray,const FsExistence &from) const
{
	objArray.Clear();

	YsArray <FsAirplane *,16> nearbyAir;
	ltc.GetAirCollisionCandidate(nearbyAir,from.GetPosition(),from.GetApproximatedCollideRadius()*2.0);

	YsArray <FsGround *,16> nearbyGnd;
	ltc.GetGndCollisionCandidate(nearbyGnd,from.GetPosition(),from.GetApproximatedCollideRadius()*2.0);

	for(int idx=0; idx<nearbyAir.GetN(); ++idx)
	{
		if(nearbyAir[idx]!=&from && 
		   FSGROUNDSTATIC==nearbyAir[idx]->Prop().GetFlightState() &&
		   NULL==nearbyAir[idx]->GetAutopilot())
		{
			const double d=(nearbyAir[idx]->GetPosition()-from.GetPosition()).GetLength();
			if(d<(nearbyAir[idx]->GetApproximatedCollideRadius()+from.GetApproximatedCollideRadius())*2.0)
			{
				objArray.Append(nearbyAir[idx]);
			}
		}
	}

	for(int idx=0; idx<nearbyGnd.GetN(); ++idx)
	{
		if(nearbyGnd[idx]!=&from && 
		   YSTRUE==nearbyGnd[idx]->Prop().IsAlive() && 
		   0!=(nearbyGnd[idx]->gndFlag & FSGNDFLAG_CANBEUSEROBJECT) &&
		   YsTolerance>nearbyGnd[idx]->Prop().GetVelocity())
		{
			const double d=(nearbyGnd[idx]->GetPosition()-from.GetPosition()).GetLength();
			if(d<(nearbyGnd[idx]->GetApproximatedCollideRadius()+from.GetApproximatedCollideRadius())*2.0)
			{
				objArray.Append(nearbyGnd[idx]);
			}
		}
	}

	if(0<objArray.GetN())
	{
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::PlayerChangeVehicleIfPossible(int objSearchKey)
{
	FsExistence *player=GetPlayerObject();
	if(YSTRUE==player->isPlayingRecord)
	{
		AddTimedMessage("Cannot change vehicle from the recorded vehilce.");
		return YSERR;
	}

	YsArray <const FsExistence *,16> boardable;
	if(NULL!=player && YSOK==FindNearbyBoardableVehicle(boardable,*player))
	{
		for(int idx=0; idx<boardable.GetN(); ++idx)
		{
			if((int)boardable[idx]->SearchKey()==objSearchKey)
			{
				if(NULL==netClient)
				{
					SetPlayerObject(boardable[idx],YSTRUE);
					return YSOK;
				}
				else
				{
					AddTimedMessage("Change-of-vehilce not supported in the network environment, yet.");
					return YSERR;
				}
			}
		}
	}

	AddTimedMessage("The vehilce is no longer nearby.");
	return YSERR;
}

YSRESULT FsSimulation::FindTakeOffTaxiPathWithinReach(YsArray <const YsSceneryPointSet *> &taxiPathArray,const YsVec3 &pos) const
{
	taxiPathArray.Clear();

	YsArray <const YsSceneryRectRegion *,16> rgnLst;
	if(field.GetFieldRegion(rgnLst,pos.x(),pos.z())==YSOK)
	{
		const YsScenery &fld=*field.GetFieldPtr();

		YsArray <const YsSceneryPointSet *,16> motionPathList;
		fld.SearchPointSetById(motionPathList,FS_MPATHID_TAXI_FOR_TAKEOFF);

		for(int mpathIdx=0; mpathIdx<motionPathList.GetN(); ++mpathIdx)
		{
			if(0<motionPathList[mpathIdx]->GetNumPoint())
			{
				const YsVec3 entry=motionPathList[mpathIdx]->GetTransformedPoint(0);
				for(int rgnIdx=0; rgnIdx<rgnLst.GetN(); ++rgnIdx)
				{
					if((rgnLst[rgnIdx]->GetId()==FS_RGNID_TAXIWAY || rgnLst[rgnIdx]->GetId()==FS_RGNID_RUNWAY))
					{
						if(YSTRUE==fld.IsInsideRectRegion(entry,rgnLst[rgnIdx]))
						{
							taxiPathArray.Append(motionPathList[mpathIdx]);
							break;
						}
					}
				}
			}
		}
		return YSOK;
	}
	return YSERR;
}

const YsSceneryPointSet *FsSimulation::FindTaxiPathBestForWindFromCandidateArray(const YsArray <const class YsSceneryPointSet *> &taxiPathArray) const
{
	YsArray <YsVec3> takeOffDir(taxiPathArray.GetN(),NULL);
	for(int pathIdx=0; pathIdx<taxiPathArray.GetN(); ++pathIdx)
	{
		const int nPnt=taxiPathArray[pathIdx]->GetNumPoint();
		if(2<=nPnt)
		{
			const YsVec3 lastPath[2]=
			{
				taxiPathArray[pathIdx]->GetTransformedPoint(nPnt-2),
				taxiPathArray[pathIdx]->GetTransformedPoint(nPnt-1)
			};
			takeOffDir[pathIdx]=YsUnitVector(lastPath[1]-lastPath[0]);
			ConstrainRunwayVectorFromRunwayRect(takeOffDir[pathIdx],lastPath[0]);
		}
		else
		{
			takeOffDir[pathIdx]=YsOrigin();
		}
	}

	const YsSceneryPointSet *selected;

	YsVec3 wind=GetWeather().GetWind();
	if(YSOK==wind.Normalize() && NULL!=(selected=FindTaxiPathBestForWindFromCandidateArray(taxiPathArray,takeOffDir,wind)))
	{
		return selected;
	}
	if(NULL!=(selected=FindTaxiPathBestForWindFromCandidateArray(taxiPathArray,takeOffDir,YsXVec())))
	{
		return selected;
	}
	if(NULL!=(selected=FindTaxiPathBestForWindFromCandidateArray(taxiPathArray,takeOffDir,YsZVec())))
	{
		return selected;
	}
	return NULL;
}

const class YsSceneryPointSet *FsSimulation::FindTaxiPathBestForWindFromCandidateArray(
     const YsArray <const class YsSceneryPointSet *> &taxiPathArray,const YsArray <YsVec3> &takeOffDirArray,const YsVec3 &unitWind) const
{
	int selectedPathIdx=-1;
	double selectedDotProd=0.0;
	for(int pathIdx=0; pathIdx<taxiPathArray.GetN(); ++pathIdx)
	{
		const double dot=unitWind*takeOffDirArray[pathIdx];
		if(FsCosineRunwayWindThreshold<fabs(dot) && (0>selectedPathIdx || dot<selectedDotProd))
		{
			selectedPathIdx=pathIdx;
			selectedDotProd=dot;
		}
	}

	if(0<=selectedPathIdx)
	{
		return taxiPathArray[selectedPathIdx];
	}
	return NULL;
}

YSRESULT FsSimulation::GetFormationCenter(YsVec3 &pos,FsAirplane &wingLeader) const
{
	YsBoundingBoxMaker3 makeBbx;
	YsVec3 bbx[2],cen;
	FsAirplane *air;
	int n;

	n=0;
	air=NULL;
	makeBbx.Begin(YsOrigin());
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air!=&wingLeader && air->Prop().IsActive()==YSTRUE)
		{
			FsAutopilot *ap;
			FsFormation *fom;
			ap=air->GetAutopilot();
			if(ap!=NULL && ap->Type()==FSAUTOPILOT_FORMATION)
			{
				fom=(FsFormation *)ap;
				if(fom->leader==&wingLeader)
				{
					makeBbx.Add(fom->shouldBe);
					n++;
				}
			}
		}
	}

	if(n>0)
	{
		makeBbx.Get(bbx[0],bbx[1]);
		cen=(bbx[0]+bbx[1])/2.0;
		wingLeader.GetAttitude().Mul(cen,cen);
		pos=wingLeader.GetPosition()+cen;
		return YSOK;
	}
	else
	{
		pos=wingLeader.GetPosition();
		return YSOK;
	}
}

void FsSimulation::RemakeLattice(void)
{
	ltc.Initialize();

	YsVec3 bbx[2];
	field.GetBoundingBox(bbx[0],bbx[1]);

	YsVec2 min2d(bbx[0].x()-20000.0,bbx[0].z()-20000.0);  // 20km margin
	YsVec2 max2d(bbx[1].x()+20000.0,bbx[1].z()+20000.0);  // 20km margin
	const YsVec2 dim(max2d-min2d);

	int nx=1+(int)(dim.x()/1000.0);
	int ny=1+(int)(dim.y()/1000.0);

	printf("FsLattice\n");
	printf(" %s to %s\n",min2d.Txt(),max2d.Txt());
	printf(" NX=%d, NY=%d\n",nx,ny);

	while(1<(nx/1024)*(ny/1024))
	{
		printf("Lattice Size Too Big! Reducing.\n");
		if(1024<nx)
		{
			nx/=2;
		}
		if(1024<ny)
		{
			ny/=2;
		}
		printf("%d %d\n",nx,ny);
	}

	ltc.SetUp(min2d,max2d,nx,ny);

	for(const FsAirplane *air=NULL; NULL!=(air=FindNextAirplane(air)); )
	{
		ltc.Add(air);
	}

	for(const FsGround *gnd=NULL; NULL!=(gnd=FindNextGround(gnd)); )
	{
		ltc.Add(gnd);
	}
}

const FsLattice &FsSimulation::GetLattice(void) const
{
	return ltc;
}

int FsSimulation::RerecordByNewInterval(const double &itvl)
{
	FsAirplane *air;
	FsGround *gnd;
	int n;

	n=0;

	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		n+=air->RerecordByNewInterval(itvl);
	}

	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		n+=gnd->RerecordByNewInterval(itvl);
	}

	return n;
}

void FsSimulation::AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng)
{
	FsAirplane *air;
	FsGround *gnd;

	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		air->AdjustPrecisionOfFlightRecord(precPos,precAng);
	}

	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		gnd->AdjustPrecisionOfFlightRecord(precPos,precAng);
	}
}

YSBOOL FsSimulation::NeedToDrawInstrument(const ActualViewMode &actualViewMode) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	if(playerPlane!=NULL && playerPlane->IsAlive()==YSTRUE)
	{
		unsigned int instDrawSwitch=GetInstrumentDrawSwitch(actualViewMode);
		const double angleThr=YsDegToRad(10.0);

		if(actualViewMode.actualViewMode==FSCOCKPITVIEW || actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW)
		{
			if(0!=(instDrawSwitch&(FSISS_3DHUD|FSISS_3DINSTPANEL)))
			{
				return YSTRUE;
			}
			else if(fabs(actualViewMode.actualViewHdg)<angleThr && fabs(actualViewMode.actualViewPch)<angleThr)
			{
				return YSTRUE;
			}

		}
		if(cfgPtr->showHudAlways==YSTRUE)
		{
			if(actualViewMode.actualViewMode==FSCOCKPITVIEW ||
			   actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW ||
			   actualViewMode.actualViewMode==FSOUTSIDEPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSFIXEDPOINTPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSVARIABLEPOINTPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSFROMTOPOFPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSPLAYERPLANEFROMSIDE ||
			   actualViewMode.actualViewMode==FSOUTSIDEPLAYER2 ||
			   actualViewMode.actualViewMode==FSOUTSIDEPLAYER3)
			{
				return YSTRUE;
			}
		}
	}

	const FsGround *playerGround=GetPlayerGround();
	if(NULL!=playerGround && YSTRUE==playerGround->IsAlive())
	{
		if(FSCOCKPITVIEW==actualViewMode.actualViewMode /* && FSWEAPON_NULL!=playerGround->Prop().GetWeaponOfChoice() */)
		{
			return YSTRUE;
		}
	}

	return YSFALSE;
}

YSBOOL FsSimulation::NeedToDrawGameInfo(const ActualViewMode &actualViewMode) const
{
	const FsExistence *playerObj=GetPlayerObject();
	if(playerObj!=NULL && playerObj->IsAlive()==YSTRUE)
	{
		if(FSBOMBINGVIEW==actualViewMode.actualViewMode)
		{
			return YSTRUE;
		}

		if(FSCOCKPITVIEW==actualViewMode.actualViewMode && FSEX_GROUND==playerObj->GetType())
		{
			return YSTRUE;
		}

		unsigned int instDrawSwitch=GetInstrumentDrawSwitch(actualViewMode);
		const double angleThr=(0!=(instDrawSwitch&(FSISS_2DHUD|FSISS_3DHUD|FSISS_3DINSTPANEL)) ? YsDegToRad(60.0) : YsDegToRad(10.0));
		if(FSCOCKPITVIEW==actualViewMode.actualViewMode &&
		   fabs(actualViewMode.actualViewHdg)<angleThr &&
		   fabs(actualViewMode.actualViewPch)<angleThr)
		{
			return YSTRUE;
		}
		if(cfgPtr->showHudAlways==YSTRUE)
		{
			if(actualViewMode.actualViewMode==FSADDITIONALAIRPLANEVIEW ||
			   actualViewMode.actualViewMode==FSOUTSIDEPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSFIXEDPOINTPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSVARIABLEPOINTPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSFROMTOPOFPLAYERPLANE ||
			   actualViewMode.actualViewMode==FSPLAYERPLANEFROMSIDE ||
			   actualViewMode.actualViewMode==FSOUTSIDEPLAYER2 ||
			   actualViewMode.actualViewMode==FSOUTSIDEPLAYER3)
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

unsigned int FsSimulation::GetInstrumentDrawSwitch(const ActualViewMode &actualViewMode) const
{
	unsigned int sw=0;

	const FsAirplane *playerPlane=GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		if(NULL!=playerPlane->instPanel)
		{
			sw|=FSISS_3DINSTPANEL;
		}

		if(playerPlane->Prop().HasHud()==YSTRUE || cfgPtr->useHudAlways==YSTRUE)
		{
			if(YSTRUE!=cfgPtr->useSimpleHud)
			{
				sw|=FSISS_3DHUD;
			}
			else
			{
				sw|=FSISS_2DHUD;
			}
		}

		if(0==sw)  // Defensive programming.  Just in case.
		{
			sw=FSISS_3DHUD;
		}

		if(FSCOCKPITVIEW==actualViewMode.actualViewMode && YSTRUE!=playerPlane->Prop().ShowHudInCockpit())
		{
			sw&=~(FSISS_3DHUD|FSISS_2DHUD);
		}
		if(FSCOCKPITVIEW==actualViewMode.actualViewMode && YSTRUE!=playerPlane->Prop().ShowInstPanelInCockpit())
		{
			sw&=~FSISS_3DINSTPANEL;
		}
		if(FSADDITIONALAIRPLANEVIEW==actualViewMode.actualViewMode)
		{
			const FsAdditionalViewpoint *vp=playerPlane->GetAdditionalView(mainWindowAdditionalAirplaneViewId);
			if(vp!=NULL && YSTRUE!=vp->showHudIfAvailable)
			{
				sw&=~(FSISS_3DHUD|FSISS_2DHUD);
			}
			if(NULL!=vp && YSTRUE!=vp->showInstPanelIfAvailable)
			{
				sw&=~FSISS_3DINSTPANEL;
			}
		}
	}

	return sw;
}

void FsSimulation::SetNetServer(class FsSocketServer *svr)
{
	netServer=svr;
	bulletHolder.SetNetServer(svr);
	explosionHolder.SetNetServer(svr);
}

void FsSimulation::SetNetClient(class FsSocketClient *cli)
{
	netClient=cli;
	bulletHolder.SetNetClient(cli);
	explosionHolder.SetNetClient(cli);
}

void FsSimulation::NetWeaponLaunch(const FsWeaponRecord &rec)
{
	bulletHolder.LaunchRecord(rec,currentTime,YSTRUE,YSFALSE);
	//                                               ^^^^^^^Prevents packet bouncing back
}

void FsSimulation::SendNetChatMessage(const char msg[])
{
	if(0!=msg[0])
	{
		YsString str;
		str.Set("(");
		if(netServer!=NULL)  // 2006/07/29
		{
			str.Append(netServer->username);
		}
		else if(netClient!=NULL)
		{
			str.Append(netClient->username);
		}
		else
		{
			str.Append("????");
		}
		str.Append(")");
		str.Append(msg);
		if(NULL!=netServer)
		{
			netServer->BroadcastChatTextMessage(str);
			fsConsole.Printf("%s",(const char *)str);
			AddTimedMessage(str);
		}
		if(NULL!=netClient)
		{
			netClient->SendTextMessage(str);
		}
		if(NULL==netServer && NULL==netClient)
		{
			AddTimedMessage(msg);
		}
	}
}

int FsSimulation::NetServerGetNumConnectedUser(void) const
{
	if(nullptr!=netServer)
	{
		return netServer->GetNumConnectedClient();
	}
	return 0;
}

void FsSimulation::OpenVehicleChangeDialog(void)
{
	if(NULL!=GetPlayerObject())
	{
		YsArray <const FsExistence *,16> boardable;
		if(YSOK==FindNearbyBoardableVehicle(boardable,*GetPlayerObject()))
		{
			vehicleChangeDlg->SetUp(boardable);
			SetCurrentInFlightDialog(vehicleChangeDlg);
		}
	}
}

void FsSimulation::CloseVehicleChangeDialog(void)
{
	SetCurrentInFlightDialog(NULL);
}

void FsSimulation::OpenChatDialog(void)
{
	if(NULL!=chatDlg)
	{
		SetCurrentInFlightDialog(chatDlg);

		int wid,hei;
		FsGetWindowSize(wid,hei);

		chatDlg->Move(0,hei-chatDlg->GetHeight());
		chatDlg->SetFocus(chatDlg->chatMsg);
	}
}

void FsSimulation::OpenLoadingDialog(YSBOOL fuel,YSBOOL ammo,const FsAirplane &air)
{
	YsArray <int,64> loading;
	loadingDlg->DisableSelectListBox();
	loadingDlg->DisableCategoryButton();
	loadingDlg->SetDefault(air.Prop().GetIdentifier());

	loadingDlg->allowAam=((allowedWeaponType & FSWEAPON_ALLOWAAM) ? YSTRUE : YSFALSE);
	loadingDlg->allowAgm=((allowedWeaponType & FSWEAPON_ALLOWAGM) ? YSTRUE : YSFALSE);
	loadingDlg->allowBomb=((allowedWeaponType & FSWEAPON_ALLOWBOMB) ? YSTRUE : YSFALSE);
	loadingDlg->allowRocket=((allowedWeaponType & FSWEAPON_ALLOWROCKET) ? YSTRUE : YSFALSE);

	if(fuel==YSTRUE)
	{
		loadingDlg->EnableFuelButton();
	}
	else
	{
		loadingDlg->DisableFuelButton();
	}

	if(ammo==YSTRUE)
	{
		loadingDlg->EnableAamButton();
		loadingDlg->EnableAgmButton();
		loadingDlg->EnableBombButton();
		loadingDlg->EnableRocketButton();
	}
	else
	{
		loadingDlg->DisableAamButton();
		loadingDlg->DisableAgmButton();
		loadingDlg->DisableBombButton();
		loadingDlg->DisableRocketButton();
		loadingDlg->allowAam=YSFALSE;
		loadingDlg->allowAgm=YSFALSE;
		loadingDlg->allowBomb=YSFALSE;
		loadingDlg->allowRocket=YSFALSE;
	}

	loadingDlg->SetOrdinanceByAirplaneProp(air.Prop()); // Needs to be after allow??? are all set.

	loadingDlg->SetFocus(loadingDlg->okBtn);

	SetCurrentInFlightDialog(loadingDlg);
}

void FsSimulation::CloseChatDialog(void)
{
	if(GetCurrentInFlightDialog()==chatDlg)
	{
		SetCurrentInFlightDialog(NULL);
		FsDisableIME();
	}
}

