#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <memory>

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

#include "fssimextension_racing.h"

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
#include "fsguimaincanvas.h"
#include "fsguicommondialog.h"


////////////////////////////////////////////////////////////


void FsGuiMainCanvas::Sim_Fly(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()!=YSTRUE)
	{
		StartMessageBox(L"ERROR",FSERRMSG_NEEDCREATEFLIGHT,FSGUI_COMMON_OK,NULL,0,0);
		return;
	}
	if(world->IsFieldLoaded()!=YSTRUE)
	{
		StartMessageBox(L"ERROR",L"Please Select Field.",FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	if(YSTRUE!=world->PlayerPlaneIsReady() &&
	   YSTRUE!=world->PlayerGroundIsReady())
	{
		StartMessageBox(L"ERROR",L"Please Select Airplane.",FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	// Save prevflight.dat
	world->Save(FsGetPrevFlightFile(),3,4,2,2,2,2,0.0);
	Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
}

void FsGuiMainCanvas::Sim_Fly_StartTakeOffSequence(FsRunLoop::RUNMODE nextRunMode)
{
	this->nextRunMode=nextRunMode;

	auto world=runLoop->GetWorld();
	const FsSimulation *sim=world->GetSimulation();
	if(NULL!=sim && YSTRUE==sim->IsMissionGoalSet())
	{
		auto dlg=StartMissionGoalDialog(sim);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_Fly_MissionGoalDone,this);
		return;
	}

	Sim_Fly_MissionGoalDone(nullptr,0);
}

void FsGuiMainCanvas::Sim_Fly_MissionGoalDone(FsGuiDialog *,int)
{
	auto world=runLoop->GetWorld();
	if(YSOK!=world->CheckJoystickAssignmentAndFixIfNecessary() && YSTRUE==FsSimulation::NeedNoJoystickWarningDialog())
	{
		auto dlg=StartNoJoystickWarningDialog();
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_Fly_JoystickWarningDone,this);
		return;
	}
	Sim_Fly_JoystickWarningDone(nullptr,0);
}

void FsGuiMainCanvas::Sim_Fly_JoystickWarningDone(FsGuiDialog *,int)
{
	runLoop->TakeOff(this->nextRunMode);
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_ReplayRecord(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()!=YSTRUE || world->PlayerPlaneIsReady()==YSTRUE)
	{
		StartMessageBox(L"ERROR",FSERRMSG_NOFLTRECORD,FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	// Show Mission Goal?
	Sim_ReplayRecord_RecordExist();
}

void FsGuiMainCanvas::Sim_ReplayRecord_RecordExist(void)
{
	auto world=runLoop->GetWorld();
	if(YSOK!=world->CheckJoystickAssignmentAndFixIfNecessary() && YSTRUE==FsSimulation::NeedNoJoystickWarningDialog())
	{
		auto dlg=StartNoJoystickWarningDialog();
		dlg->BindCloseModalCallBack(&FsGuiMainCanvas::Sim_ReplayRecord_JoystickChecked,this);
		return;
	}
	Sim_ReplayRecord_JoystickChecked(nullptr,0);
}

void FsGuiMainCanvas::Sim_ReplayRecord_JoystickChecked(FsGuiDialog *,int)
{
	runLoop->StartReplayRecord(/*EditMode=*/YSFALSE);
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_Retry(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&FsGuiMainCanvas::Sim_Retry_DeleteConfirmed,this);
		return;
	}
	Sim_Retry_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_Retry_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK!=returnCode)
	{
		return;
	}
	if(Sim_RetryStart()!=YSOK)
	{
		StartMessageBox(L"ERROR",FSERRMSG_NOPREVFLIGHT,FSGUI_COMMON_OK,NULL,0,0);
	}
}

YSRESULT FsGuiMainCanvas::Sim_RetryStart(void)
{
	YsString firstLine;
	{
		YsFileIO::File fp(FsGetPrevFlightFile(),"r");
		if(nullptr==fp || nullptr==firstLine.Fgets(fp))
		{
			return YSERR;
		}
	}

	auto world=runLoop->GetWorld();
	world->TerminateSimulation();

	if(YSTRUE==firstLine.DOESSTARTWITH("YFSVERSI"))
	{
		if(YSOK==world->Load(FsGetPrevFlightFile()))
		{
			Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
			return YSOK;
		}
	}

	return YSERR;
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_CreateFlight(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&FsGuiMainCanvas::Sim_CreateFlight_DeleteConfirmed,this);
		return;
	}
	Sim_CreateFlight_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_CreateFlight_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		auto world=runLoop->GetWorld();

		FsNewFlightDialogOption opt;
		opt.canSelectWingmen=YSTRUE;
		opt.canChooseNight=YSTRUE;
		opt.canChooseFomType=YSTRUE;
		opt.flyNowButton=YSTRUE;
		auto dlg=StartNewFlightDialog(world,opt);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_CreateFlight_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Sim_CreateFlight_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiNewFlightDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		Sim_CreateFlight_Create(dlg->info);
	}
}

void FsGuiMainCanvas::Sim_CreateFlight_Create(FsNewFlightDialogInfo &info)
{
	auto world=runLoop->GetWorld();
	YSBOOL inTheAir;

	world->TerminateSimulation();
	world->PrepareSimulation();
	const YSBOOL loadYFS=YSTRUE;
	const YSBOOL addComAir=info.addComputerAircraft;
	if(NULL==world->AddField(NULL,info.fieldName,YsOrigin(),YsZeroAtt(),loadYFS,addComAir))
	{
		StartMessageBox(
		    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
		    FSGUI_COMMON_OK,NULL,0,0);
		world->TerminateSimulation();
		return;
	}
	world->SetEnvironment(info.envInfo.dayOrNight);

	FsAirplane *air;
	air=world->AddAirplane(info.playerAirInfo.typeName,YSTRUE);
	air->iff=FS_IFF0;
	air->AutoSendCommand(info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,info.playerAirInfo.fuel);
	world->SettleAirplane(*air,info.playerAirInfo.startPos);


	if(air->Prop().GetVelocity()>YsTolerance &&
	   air->Prop().GetLandingGear()<YsTolerance)
	{
		inTheAir=YSTRUE;
	}
	else
	{
		inTheAir=YSFALSE;
	}


	switch(info.fomType)
	{
	default:
	case FSFOM_NONE:
		{
			for(int i=0; i<FsNewFlightDialogInfo::MaxNumWingman; i++)
			{
				if(info.wingman[i]==YSTRUE)
				{
					air=world->AddAirplane(info.wingmanInfo[i].typeName,YSFALSE);
					if(air!=NULL)
					{
						air->iff=FS_IFF0;
						air->AutoSendCommand(info.wingmanInfo[i].weaponConfig.GetN(),info.wingmanInfo[i].weaponConfig,info.wingmanInfo[i].fuel);
						world->SettleAirplane(*air,info.wingmanInfo[i].startPos);
					}
				}
			}
		}
		break;
	case FSFOM_DIAMOND:
	case FSFOM_DELTA:
		{
			char cmd[256];
			double r,vSep;
			vSep=1.5;
			r=air->GetRadiusFromCollision()*0.85;
			if(inTheAir==YSTRUE)
			{
				FsAirplane *wingman;

				double pos[]=
				{
					-1.0, -1.0,
					 1.0, -1.0,
					 0.0, -2.0,
					-2.0, -2.0,
					 2.0, -2.0
				};

				int n;
				n=(info.fomType==FSFOM_DELTA ? 5 : 3);
				for(int i=0; i<n; i++)
				{
					YsVec3 p;
					YsAtt3 a;
					wingman=NULL;
					if(info.wingmanInfo[i].typeName[0]!=0)
					{
						wingman=world->AddAirplane(info.playerAirInfo.typeName,YSFALSE);
					}
					if(wingman==NULL)
					{
						wingman=world->AddAirplane(info.playerAirInfo.typeName,YSFALSE);
					}
					a=air->GetAttitude();

					p.Set(r*pos[i*2],vSep*pos[i*2+1],r*pos[i*2+1]);
					FsFormation *fom;
					fom=FsFormation::Create();
					fom->minAlt=0.0;
					fom->leader=air;
					fom->shouldBe=p;
					fom->synchronizeTrigger=YSTRUE;

					a.Mul(p,p);
					p+=air->GetPosition();

					sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
					wingman->SendCommand(cmd);
					sprintf(cmd,"ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
					    YsRadToDeg(a.h()),YsRadToDeg(a.p()),YsRadToDeg(a.b()));
					wingman->SendCommand(cmd);
					wingman->SendCommand("CTLLDGEA FALSE");

					sprintf(cmd,"INITSPED %.2lfm/s",air->Prop().GetVelocity());
					wingman->SendCommand(cmd);

					wingman->AutoSendCommand(info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,info.playerAirInfo.fuel);

					wingman->SetAutopilot(fom);
				}
			}
			else
			{
				FsAirplane *wingman[5];
				YsVec3 p;
				YsAtt3 a;
				FsFormation *fom;

				double pos[]=
				{
					-1.0, -1.0,   // #2
					 1.0, -1.0,   // #3

					-2.0, -2.0,   // #4  <- Transition to Diamond

					-2.0, -6.0,   // #5
					 2.0, -6.0    // #6
				};

				int n;

				n=(info.fomType==FSFOM_DELTA ? 5 : 3);



				YsVec3 rwRect[4];
				const YsSceneryRectRegion *rwRgn;
				if(world->GetRunwayRectFromPosition(rwRgn,rwRect,air->GetPosition())==YSOK)
				{
					double elv;
					YsVec3 offset;
					offset.SetY(0.0);
					if(info.fomType==FSFOM_DELTA)
					{
						offset.SetX(0.0);
						offset.SetZ(r*10.0);
					}
					else
					{
						offset.SetX(r*0.5);
						offset.SetZ(r*5.0);
					}
					FsSimulation::AdjustStartPositionByRunwayRect(p,rwRect,offset,air->GetAttitude());

					elv=world->GetFieldElevation(p.x(),p.z());
					p.SetY(elv+air->Prop().GetGroundStandingHeight());

					sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
					air->SendCommand(cmd);
				}


				for(int i=0; i<n; i++)
				{
					wingman[i]=NULL;
					if(info.wingmanInfo[i].typeName[0]!=0)
					{
						wingman[i]=world->AddAirplane(info.wingmanInfo[i].typeName,YSFALSE);
					}
					if(wingman[i]==NULL)
					{
						wingman[i]=world->AddAirplane(info.playerAirInfo.typeName,YSFALSE);
					}
					a=air->GetAttitude();

					p.Set(r*pos[i*2],0.0,r*pos[i*2+1]);
					a.Mul(p,p);
					p+=air->GetPosition();

					sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
					wingman[i]->SendCommand(cmd);
					sprintf(cmd,"ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
					    YsRadToDeg(a.h()),YsRadToDeg(a.p()),YsRadToDeg(a.b()));
					wingman[i]->SendCommand(cmd);
					wingman[i]->AutoSendCommand(info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,info.playerAirInfo.fuel);
				}

				for(int i=0; i<2; i++)
				{
					p.Set(r*pos[i*2],vSep*pos[i*2+1],r*pos[i*2+1]);
					fom=FsFormation::Create();
					fom->minAlt=0.0;
					fom->leader=air;
					fom->shouldBe=p;
					fom->synchronizeTrigger=YSTRUE;
					wingman[i]->SetAutopilot(fom);
				}


				p.Set(r*pos[4],vSep*pos[5]-4.0,r*pos[5]);
				fom=FsFormation::Create();
				fom->minAlt=0.0;
				fom->leader=air;
				fom->shouldBe=p;

				p.DivX(3.0);
				fom->transition[fom->nTransition++]=p;
				p.SetX(0.0);
				fom->transition[fom->nTransition++]=p;
				p.SetY(vSep*pos[5]);
				fom->transition[fom->nTransition++]=p;

				if(info.fomType==FSFOM_DELTA)
				{
					fom->afterTransFormationAirplane.Append(world->GetAirplaneIdFromHandle(wingman[3]));
					fom->afterTransFormationPos.Append(YsVec3(-2.0*r,-vSep*2.0,-2.0*r));
					fom->afterTransFormationAirplane.Append(world->GetAirplaneIdFromHandle(wingman[4]));
					fom->afterTransFormationPos.Append(YsVec3( 2.0*r,-vSep*2.0,-2.0*r));
				}

				wingman[2]->SetAutopilot(fom);

				if(info.fomType==FSFOM_DELTA)
				{
					for(int i=3; i<5; i++)
					{
						p.Set(r*pos[i*2],-vSep*2.0,r*pos[i*2+1]);
						fom=FsFormation::Create();
						fom->minAlt=0.0;
						fom->leader=air;
						fom->shouldBe=p;
						fom->synchronizeTrigger=YSTRUE;
						wingman[i]->SetAutopilot(fom);
					}
				}
			}
		}
		break;
	}

	if(YSTRUE==info.envInfo.specifyEnvironment)
	{
		YsVec3 windVec;
		windVec.Set(0.0,0.0,-info.envInfo.windSpd);
		windVec.RotateXZ(-info.envInfo.windDir);
		world->SetWind(windVec);

		world->SetEnvironment(info.envInfo.dayOrNight);

		world->SetFog(info.envInfo.fog);
		world->SetFogVisibility(info.envInfo.fogVisibility);

		for(int i=0; i<=info.envInfo.cloudLayer.GetN()-2; i+=2)
		{
			world->AddOvercastLayer(info.envInfo.cloudLayer[i],info.envInfo.cloudLayer[i+1]);
		}
	}

	if(info.flyImmediately==YSTRUE)
	{
		Sim_Fly(nullptr);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_ChooseAircraft(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();

	if(world->SimulationIsPrepared()!=YSTRUE)
	{
		StartMessageBox(
		    L"ERROR",
		    FSERRMSG_NEEDCREATEFLIGHT,
		    FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	if(world->PlayerPlaneIsReady()!=YSTRUE)
	{
		YsString fieldName;
		YsVec3 fldPos;
		YsAtt3 fldAtt;
		if(world->GetLoadedField(fieldName,fldPos,fldAtt)==YSOK)
		{
			FsNewFlightDialogOption opt;
			opt.canSelectField=YSFALSE;
			opt.canSelectEnvironment=YSFALSE;
			opt.canSelectWingmen=YSFALSE;
			opt.canChooseNight=YSFALSE;
			opt.canChooseFomType=YSFALSE;
			opt.flyNowButton=YSTRUE;
			opt.fieldName=fieldName;
			auto playerPlane=world->GetPlayerAirplane();
			if(NULL!=playerPlane)
			{
				opt.startPosName=playerPlane->GetStartPos();
				opt.nextStartPos=YSTRUE;
			}

			auto dlg=StartNewFlightDialog(world,opt);
			dlg->BindCloseModalCallBack(&THISCLASS::Sim_ChooseAircraft_And_Startpos_Select,this);
		}
		else
		{
			StartMessageBox(
			    L"ERROR",
			    L"Field is not chosen.",
			    FSGUI_COMMON_OK,NULL,0,0);
		}
	}
	else
	{
		FsAirplane *playerPlane=world->GetPlayerAirplane();
		auto dlg=StartSelectAircraftDialog(playerPlane->Prop().GetIdentifier(),(int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_ChooseAircraft_Select,this);
	}
}

void FsGuiMainCanvas::Sim_ChooseAircraft_And_Startpos_Select(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast <FsGuiNewFlightDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		Sim_ChooseAircraft_And_Startpos_Select(dlg->info);
	}
}

void FsGuiMainCanvas::Sim_ChooseAircraft_And_Startpos_Select(const FsNewFlightDialogInfo &info)
{
	auto world=runLoop->GetWorld();
	if(world->CheckStartPositionIsAvailable(0,info.playerAirInfo.startPos)==YSOK)
	{
		FsAirplane *air;
		air=world->AddAirplane(info.playerAirInfo.typeName,YSTRUE);
		air->iff=(FSIFF)info.playerAirInfo.iff;
		air->AutoSendCommand(
		    info.playerAirInfo.weaponConfig.GetN(),
		    info.playerAirInfo.weaponConfig,
		    info.playerAirInfo.fuel);
		world->SettleAirplane(*air,info.playerAirInfo.startPos);

		// Call DeleteEventByTypeAll after adding a player airplane.
		// AddAirplane with isPlayer==YSTRUE will add a player-change event.
		world->DeleteEventByTypeAll(FSEVENT_PLAYEROBJCHANGE);

		// Prevent reset of the player in Retry Previous Flight.
		world->ClearFirstPlayer();

		if(info.flyImmediately==YSTRUE)
		{
			Sim_Fly(nullptr);
		}
	}
	else
	{
		YsString str;
		str.Printf("Start Position\n [%s]is already used.",info.playerAirInfo.startPos.Txt());

		YsWString wStr;
		wStr.SetUTF8String(str);

		StartMessageBox(L"ERROR!",wStr,FSGUI_COMMON_OK,NULL,0,0);
	}
}

void FsGuiMainCanvas::Sim_ChooseAircraft_Select(FsGuiDialog *closedDialog,int returnCode)
{
	printf("%s %d\n",__FUNCTION__,__LINE__);
	auto dlg=dynamic_cast <FsGuiChooseAircraft *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		Sim_ChooseAircraft_Select(dlg);
	}
}

void FsGuiMainCanvas::Sim_ChooseAircraft_Select(const FsGuiChooseAircraft *dlg)
{
	auto world=runLoop->GetWorld();

	FsAirplane *playerPlane=world->GetPlayerAirplane();

	world->ReplaceAirplane(playerPlane,dlg->selAir);

	playerPlane=world->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		playerPlane->AutoSendCommand(
		    (int)dlg->selWeaponConfig.GetN(),
		    dlg->selWeaponConfig,
		    dlg->selFuel);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_ChooseStartPosition(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();

	if(world->SimulationIsPrepared()!=YSTRUE)
	{
		StartMessageBox(L"ERROR!",FSERRMSG_NEEDCREATEFLIGHT,FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	FsAirplane *air=world->GetPlayerAirplane();

	if(world->PlayerPlaneIsReady()!=YSTRUE || NULL==air)
	{
		StartMessageBox(L"ERROR!",L"Select Player Airplane First.",FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	YsString startPosition,fieldName;
	YsVec3 fldPos;
	YsAtt3 fldAtt;
	if(world->GetLoadedField(fieldName,fldPos,fldAtt)!=YSOK)
	{
		StartMessageBox(L"ERROR!",L"Field is not selected.",FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	auto selStpDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiChooseField>();
	selStpDlg->Initialize();
	selStpDlg->useFldListBox=YSFALSE;
	selStpDlg->fldNameForStpSelector=fieldName;
	selStpDlg->Create(world);

	selStpDlg->Select(fieldName,air->_startPosition);

	selStpDlg->ReloadField();
	selStpDlg->ReloadStartPosition();

	selStpDlg->BindCloseModalCallBack(&THISCLASS::Sim_ChooseStartPosition_Selected,this);

	AttachModalDialog(selStpDlg);
}

void FsGuiMainCanvas::Sim_ChooseStartPosition_Selected(FsGuiDialog *closedDialog,int returnCode)
{
	auto selStp=dynamic_cast <FsGuiChooseField *>(closedDialog);
	if(nullptr!=selStp && (int)YSOK==returnCode)
	{
		auto world=runLoop->GetWorld();
		FsAirplane *air=world->GetPlayerAirplane();

		if(NULL!=air)
		{
			if(0==selStp->selStp.Strcmp(air->_startPosition))
			{
				// Reset. For safety.
				world->SettleAirplane(*air,selStp->selStp);
				return;
			}

			if(world->CheckStartPositionIsAvailable(0,selStp->selStp)!=YSOK)
			{
				YsString str;
				str="Position ";
				str.Append(selStp->selStp);
				str.Append(" is Already Used.");

				YsWString wStr;
				wStr.SetUTF8String(str);
				StartMessageBox(L"Error",wStr,FSGUI_COMMON_OK,NULL,0,0);
				return;
			}

			world->SettleAirplane(*air,selStp->selStp);
		}
	}
}

////////////////////////////////////////////////////////////

class FsGuiMainCanvas::SelectDayOrNightDialog : public FsGuiDialog
{
public:
	FsGuiButton *dayNightBtn[2];
	FsGuiButton *okBtn,*cancelBtn;
	FsWorld *world;
	FSENVIRONMENT env;

	void Make(FsWorld *world);
	virtual void OnButtonClick(FsGuiButton *btn);
};

void FsGuiMainCanvas::SelectDayOrNightDialog::Make(FsWorld *world)
{
	this->world=world;

	SetTextMessage("-- Select Environment --");

	dayNightBtn[0]=AddTextButton(0,FSKEY_D,FSGUI_RADIOBUTTON,"[D] Day",YSTRUE);
	dayNightBtn[1]=AddTextButton(1,FSKEY_N,FSGUI_RADIOBUTTON,"[N] Night",YSFALSE);
	dayNightBtn[0]->SetCheck(YSTRUE);
	SetRadioButtonGroup(2,dayNightBtn);

	okBtn=AddTextButton(2,FSKEY_ENTER,FSGUI_PUSHBUTTON,"OK",YSTRUE);
	cancelBtn=AddTextButton(3,FSKEY_ESC,FSGUI_PUSHBUTTON,"Cancel",YSTRUE);

	SetTransparency(YSFALSE);
	Fit();
}

/* virtual */ void FsGuiMainCanvas::SelectDayOrNightDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		if(YSTRUE==dayNightBtn[0]->GetCheck())
		{
			env=FSDAYLIGHT;
		}
		else // if(YSTRUE==dayNightBtn[1]->GetCheck())
		{
			env=FSNIGHT;
		}
		CloseModalDialog((int)YSOK);
	}
	else if(btn==cancelBtn)
	{
		CloseModalDialog((int)YSERR);
	}
}

void FsGuiMainCanvas::Sim_SelectDayOrNight(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();

	if(world->SimulationIsPrepared()!=YSTRUE)
	{
		StartMessageBox(
		    L"ERROR",
		    FSERRMSG_NEEDCREATEFLIGHT,
		    FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<SelectDayOrNightDialog>();
	dlg->Make(world);
	dlg->BindCloseModalCallBack(&THISCLASS::Sim_SelectDayOrNight_Selected,this);
	AttachModalDialog(dlg);
}
void FsGuiMainCanvas::Sim_SelectDayOrNight_Selected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast <SelectDayOrNightDialog *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		auto world=runLoop->GetWorld();
		world->SetEnvironment(dlg->env);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_CreateAirCombat(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_CreateAirCombat_ConfirmDeleteCurrentFlight,this);
		return;
	}
	Sim_CreateAirCombat_ConfirmDeleteCurrentFlight(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_CreateAirCombat_ConfirmDeleteCurrentFlight(FsGuiDialog *closedDlg,int returnCode)
{
	if(nullptr!=closedDlg && (int)YSOK!=returnCode)
	{
		return;
	}

	if(nullptr==airCombatDlg)
	{
		airCombatDlg=new FsGuiAirCombatDialog;
	}

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());

	auto world=runLoop->GetWorld();
	airCombatDlg->Make(world);
	airCombatDlg->Initialize(world,cfg);

	AttachModalDialog(airCombatDlg);
	airCombatDlg->BindCloseModalCallBack(&THISCLASS::Sim_CreateAirCombat_DialogClosed,this);
}

void FsGuiMainCanvas::Sim_CreateAirCombat_DialogClosed(FsGuiDialog *closedDlg,int returnCode)
{
	if(closedDlg!=airCombatDlg)
	{
		return;
	}

	auto dlg=airCombatDlg;
	if((int)YSOK==returnCode && YSTRUE==dlg->flyNow)
	{
		Sim_Fly(nullptr);
	}
	else if((int)YSOK!=returnCode)
	{
		switch(dlg->errCode)
		{
		case FsGuiAirCombatDialog::ACMDLG_ERROR_STARTPOS_UNSELECTED:
			StartMessageBox(L"Error",L"Start position for at least one airplane is not chosen.",FSGUI_COMMON_OK,NULL,0,0);
			break;
		case FsGuiAirCombatDialog::ACMDLG_ERROR_STARTPOS_COLLISION:
			StartMessageBox(L"Error",L"Two or more airplanes are using same starting position.",FSGUI_COMMON_OK,NULL,0,0);
			break;
		case FsGuiAirCombatDialog::ACMDLG_ERROR_PLAYER_NOTSELECTED:
			StartMessageBox(L"Error",FSGUI_AIRCOMBATDLG_MSG_NOPLAYER,FSGUI_COMMON_OK,NULL,0,0);
			break;
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_DisableGroundFire(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	world->DisableGroundFire();
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_EnduranceMode(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_EnduranceMode_DeleteConfirmed,this);
		return;
	}
	Sim_EnduranceMode_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_EnduranceMode_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		auto dlg=StartEnduranceModeDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&FsGuiMainCanvas::Sim_EnduranceMode_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Sim_EnduranceMode_OptionSelected(FsGuiDialog *dlg,int returnCode)
{
	auto enduranceModeDialog=dynamic_cast <FsGuiEnduranceModeDialog *>(dlg);
	if(nullptr!=enduranceModeDialog && (int)YSOK==returnCode)
	{
		auto &info=enduranceModeDialog->info;
		if(YSOK==runLoop->SetUpEnduranceMode(
		    info.playerAirInfo.typeName,info.jet,info.ww2,
		    info.fieldName,info.numWingman,info.wingmanLevel,info.allowAAM,
		    info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,
		    info.playerAirInfo.fuel))
		{
			auto world=runLoop->GetWorld();
			world->Save(FsGetPrevFlightFile(),3,4,2,2,2,2,0.0);
			Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_InterceptMission(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_InterceptMission_DeleteConfirmed,this);
		return;
	}
	Sim_InterceptMission_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_InterceptMission_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		auto dlg=StartInterceptMissionDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_InterceptMission_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Sim_InterceptMission_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiInterceptMissionDialog *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		if(YSOK==runLoop->SetUpInterceptMission(dlg->info))
		{
			auto world=runLoop->GetWorld();
			world->Save(FsGetPrevFlightFile(),3,4,2,2,2,2,0.0);
			Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_CloseAirSupportMission(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_CloseAirSupportMission_DeleteConfirmed,this);
		return;
	}
	Sim_CloseAirSupportMission_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_CloseAirSupportMission_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		auto dlg=StartCloseAirSupportMissionDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_CloseAirSupportMission_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Sim_CloseAirSupportMission_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiCloseAirSupportMissionDialog *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		if(YSOK==runLoop->SetUpCloseAirSupportMission(
		    dlg->playerAirNameString,
		    dlg->fieldString,
		    dlg->playerWeaponConfig.GetN(),
		    dlg->playerWeaponConfig,
		    dlg->playerFuel))
		{
			auto world=runLoop->GetWorld();
			world->Save(FsGetPrevFlightFile(),3,4,2,2,2,2,0.0);
			Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_CreateInGroundObject(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_CreateInGroundObject_DeleteConfirmed,this);
		return;
	}
	Sim_CreateInGroundObject_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_CreateInGroundObject_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsCreateNewSimulationInGroundObjectDialogClass>();

		FsFlightConfig cfg;
		cfg.Load(FsGetConfigFile());

		auto world=runLoop->GetWorld();
		dlg->Make(world,/*airDefenseMission=*/YSFALSE);
		dlg->Initialize(cfg);
		AttachModalDialog(dlg);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_CreateInGroundObject_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Sim_CreateInGroundObject_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast <FsCreateNewSimulationInGroundObjectDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		FsNewDriveDialogInfo info;
		dlg->Retrieve(info);

		auto world=runLoop->GetWorld();

		FsAddedFieldInfo addedFieldInfo;
		world->TerminateSimulation();
		world->PrepareSimulation();
		if(nullptr==world->AddField(&addedFieldInfo,info.fieldName,YsOrigin(),YsZeroAtt())) // This function needs to return gobId's object
		{
			StartMessageBox(
			    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
			    FSGUI_COMMON_OK,NULL,0,0);
			world->TerminateSimulation();
			return;
		}
		world->SetEnvironment(FSDAYLIGHT);

		if(YSTRUE==addedFieldInfo.gndArray.IsInRange(info.gobId) &&
		   nullptr!=addedFieldInfo.gndArray[info.gobId])
		{
			world->SetPlayerGround(addedFieldInfo.gndArray[info.gobId]);
			if(YSTRUE==info.driveNow)
			{
				Sim_Fly(nullptr);
			}
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_GroundToAirMission(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_GroundToAirMission_DeleteConfirmed,this);
		return;
	}
	Sim_GroundToAirMission_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_GroundToAirMission_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK!=returnCode)
	{
		return;
	}

	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsCreateNewSimulationInGroundObjectDialogClass>();

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());
	dlg->Make(runLoop->GetWorld(),/*airDefenseMission=*/YSTRUE);
	dlg->Initialize(cfg);
	AttachModalDialog(dlg);
	dlg->BindCloseModalCallBack(&THISCLASS::Sim_GroundToAirMission_OptionSelected,this);
}

void FsGuiMainCanvas::Sim_GroundToAirMission_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast <FsCreateNewSimulationInGroundObjectDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		FsGroundToAirDefenseMissionInfo info;
		dlg->Retrieve(info);
		if(YSOK==runLoop->SetUpGroundToAirMission(info))
		{
			auto world=runLoop->GetWorld();
			world->Save(FsGetPrevFlightFile(),3,4,2,2,2,2,0.0);
			Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_LandingPractice(FsGuiPopUpMenuItem *menuItem)
{
	Sim_LandingPractice_Level=0;
	for(int i=0; i<FsGuiMainMenu::NUM_LANDING_PRACTICE_LEVEL; i++)
	{
		if(menuItem==mainMenu->simLdgPracticeLevel[i])
		{
			Sim_LandingPractice_Level=i;
			break;
		}
	}

	if(runLoop->GetWorld()->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&FsGuiMainCanvas::Sim_LandingPractice_DeleteConfirmed,this);
		return;
	}
	Sim_LandingPractice_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_LandingPractice_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK!=returnCode)
	{
		return;
	}

	FsNewFlightDialogOption opt;
	opt.canSelectStartPosition=YSFALSE;
	opt.canSelectLoading=YSFALSE;
	opt.canSelectEnvironment=YSFALSE;
	opt.canSelectWingmen=YSFALSE;
	opt.canChooseNight=YSFALSE;
	opt.canChooseFomType=YSFALSE;
	opt.flyNowButton=YSFALSE;

	auto world=runLoop->GetWorld();
	auto dlg=StartNewFlightDialog(world,opt);
	dlg->BindCloseModalCallBack(&THISCLASS::Sim_LandingPractice_OptionSelected,this);
}

void FsGuiMainCanvas::Sim_LandingPractice_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiNewFlightDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		auto &info=dlg->info;
		auto level=Sim_LandingPractice_Level;

		FSTRAFFICPATTERNLEG leg;
		double crossWind;
		YSBOOL lowCloud;
		YSBOOL lowVisibility;

		switch(level)
		{
		default:
		case 0:
			leg=FSLEG_FINAL;
			crossWind=0.0;
			lowCloud=YSFALSE;
			lowVisibility=YSFALSE;
			break;
		case 1:
			leg=FSLEG_FINAL;
			crossWind=2.5;
			lowCloud=YSFALSE;
			lowVisibility=YSFALSE;
			break;
		case 2:
			leg=FSLEG_DOG;
			crossWind=0.0;
			lowCloud=YSFALSE;
			lowVisibility=YSFALSE;
			break;
		case 3:
			leg=FSLEG_BASE;
			crossWind=2.5;
			lowCloud=YSFALSE;
			lowVisibility=YSFALSE;
			break;
		case 4:
			leg=FSLEG_BASE;
			crossWind=5.0;
			lowCloud=YSFALSE;
			lowVisibility=YSFALSE;
			break;
		case 5:
			leg=FSLEG_BASE;
			crossWind=7.5;
			lowCloud=YSFALSE;
			lowVisibility=YSFALSE;
			break;
		case 6:
			leg=FSLEG_FINAL;
			crossWind=0.0;
			lowCloud=YSTRUE;
			lowVisibility=YSFALSE;
			break;
		case 7:
			leg=FSLEG_DOG;
			crossWind=0.0;
			lowCloud=YSTRUE;
			lowVisibility=YSFALSE;
			break;
		case 8:
			leg=FSLEG_BASE;
			crossWind=0.0;
			lowCloud=YSTRUE;
			lowVisibility=YSFALSE;
			break;
		case 9:
			leg=FSLEG_BASE;
			crossWind=2.5;
			lowCloud=YSTRUE;
			lowVisibility=YSFALSE;
			break;
		case 10:
			leg=FSLEG_BASE;
			crossWind=5.0;
			lowCloud=YSTRUE;
			lowVisibility=YSFALSE;
			break;
		case 11:
			leg=FSLEG_FINAL;
			crossWind=0.0;
			lowCloud=YSFALSE;
			lowVisibility=YSTRUE;
			break;
		case 12:
			leg=FSLEG_BASE;
			crossWind=0.0;
			lowCloud=YSFALSE;
			lowVisibility=YSTRUE;
			break;
		case 13:
			leg=FSLEG_BASE;
			crossWind=5.0;
			lowCloud=YSFALSE;
			lowVisibility=YSTRUE;
			break;
		case 14:
			leg=FSLEG_BASE;
			crossWind=7.5;
			lowCloud=YSTRUE;
			lowVisibility=YSTRUE;
			break;
		}

		YSBOOL leftTraffic;
		if(YSOK==runLoop->SetUpLandingPracticeMode(info.fieldName,info.playerAirInfo.typeName,leftTraffic,leg,crossWind,lowCloud,lowVisibility))
		{
			YsVec3 wind(crossWind,0.0,0.0);
			runLoop->StartShowLandingPracticeInfoMode(leftTraffic,leg,wind,lowCloud,lowVisibility);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Sim_RacingMode(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&FsGuiMainCanvas::Sim_RacingMode_DeleteConfirmed,this);
		return;
	}
	Sim_RacingMode_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::Sim_RacingMode_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		auto world=runLoop->GetWorld();

		FsNewFlightDialogOption opt;
		opt.canSelectWingmen=YSTRUE;
		opt.canChooseNight=YSTRUE;
		opt.canChooseFomType=YSTRUE;
		opt.flyNowButton=YSTRUE;
		opt.forRacingMode=YSTRUE;
		auto dlg=StartNewFlightDialog(world,opt);
		dlg->BindCloseModalCallBack(&THISCLASS::Sim_RacingMode_OptionSelected,this);
	}
}

void FsGuiMainCanvas::Sim_RacingMode_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiNewFlightDialogClass *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		Sim_RacingMode_Create(dlg->info);
	}
}

void FsGuiMainCanvas::Sim_RacingMode_Create(FsNewFlightDialogInfo &info)
{
	auto world=runLoop->GetWorld();
	YSBOOL inTheAir;

	world->TerminateSimulation();
	world->PrepareSimulation();

	std::shared_ptr <FsSimExtensionBase> racingModeAddOn(new FsSimExtension_RacingMode);
	world->RegisterExtension(racingModeAddOn);

	const YSBOOL loadYFS=YSTRUE;
	const YSBOOL addComAir=info.addComputerAircraft;
	if(NULL==world->AddField(NULL,info.fieldName,YsOrigin(),YsZeroAtt(),loadYFS,addComAir))
	{
		StartMessageBox(
		    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
		    FSGUI_COMMON_OK,NULL,0,0);
		world->TerminateSimulation();
		return;
	}
	world->SetEnvironment(info.envInfo.dayOrNight);

	FsAirplane *air;
	air=world->AddAirplane(info.playerAirInfo.typeName,YSTRUE);
	air->iff=FS_IFF0;
	air->AutoSendCommand(info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,info.playerAirInfo.fuel);
	world->SettleAirplane(*air,info.playerAirInfo.startPos);


	if(air->Prop().GetVelocity()>YsTolerance &&
	   air->Prop().GetLandingGear()<YsTolerance)
	{
		inTheAir=YSTRUE;
	}
	else
	{
		inTheAir=YSFALSE;
	}


	switch(info.fomType)
	{
	default:
	case FSFOM_NONE:
		{
			for(int i=0; i<FsNewFlightDialogInfo::MaxNumWingman; i++)
			{
				if(info.wingman[i]==YSTRUE)
				{
					air=world->AddAirplane(info.wingmanInfo[i].typeName,YSFALSE);
					if(air!=NULL)
					{
						air->iff=FS_IFF0;
						air->AutoSendCommand(info.wingmanInfo[i].weaponConfig.GetN(),info.wingmanInfo[i].weaponConfig,info.wingmanInfo[i].fuel);
						world->SettleAirplane(*air,info.wingmanInfo[i].startPos);
					}
				}
			}
		}
		break;
	case FSFOM_DIAMOND:
	case FSFOM_DELTA:
		{
			char cmd[256];
			double r,vSep;
			vSep=1.5;
			r=air->GetRadiusFromCollision()*0.85;
			if(inTheAir==YSTRUE)
			{
				FsAirplane *wingman;

				double pos[]=
				{
					-1.0, -1.0,
					 1.0, -1.0,
					 0.0, -2.0,
					-2.0, -2.0,
					 2.0, -2.0
				};

				int n;
				n=(info.fomType==FSFOM_DELTA ? 5 : 3);
				for(int i=0; i<n; i++)
				{
					YsVec3 p;
					YsAtt3 a;
					wingman=NULL;
					if(info.wingmanInfo[i].typeName[0]!=0)
					{
						wingman=world->AddAirplane(info.playerAirInfo.typeName,YSFALSE);
					}
					if(wingman==NULL)
					{
						wingman=world->AddAirplane(info.playerAirInfo.typeName,YSFALSE);
					}
					a=air->GetAttitude();

					p.Set(r*pos[i*2],vSep*pos[i*2+1],r*pos[i*2+1]);
					FsFormation *fom;
					fom=FsFormation::Create();
					fom->minAlt=0.0;
					fom->leader=air;
					fom->shouldBe=p;
					fom->synchronizeTrigger=YSTRUE;

					a.Mul(p,p);
					p+=air->GetPosition();

					sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
					wingman->SendCommand(cmd);
					sprintf(cmd,"ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
					    YsRadToDeg(a.h()),YsRadToDeg(a.p()),YsRadToDeg(a.b()));
					wingman->SendCommand(cmd);
					wingman->SendCommand("CTLLDGEA FALSE");

					sprintf(cmd,"INITSPED %.2lfm/s",air->Prop().GetVelocity());
					wingman->SendCommand(cmd);

					wingman->AutoSendCommand(info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,info.playerAirInfo.fuel);

					wingman->SetAutopilot(fom);
				}
			}
			else
			{
				FsAirplane *wingman[5];
				YsVec3 p;
				YsAtt3 a;
				FsFormation *fom;

				double pos[]=
				{
					-1.0, -1.0,   // #2
					 1.0, -1.0,   // #3

					-2.0, -2.0,   // #4  <- Transition to Diamond

					-2.0, -6.0,   // #5
					 2.0, -6.0    // #6
				};

				int n;

				n=(info.fomType==FSFOM_DELTA ? 5 : 3);



				YsVec3 rwRect[4];
				const YsSceneryRectRegion *rwRgn;
				if(world->GetRunwayRectFromPosition(rwRgn,rwRect,air->GetPosition())==YSOK)
				{
					double elv;
					YsVec3 offset;
					offset.SetY(0.0);
					if(info.fomType==FSFOM_DELTA)
					{
						offset.SetX(0.0);
						offset.SetZ(r*10.0);
					}
					else
					{
						offset.SetX(r*0.5);
						offset.SetZ(r*5.0);
					}
					FsSimulation::AdjustStartPositionByRunwayRect(p,rwRect,offset,air->GetAttitude());

					elv=world->GetFieldElevation(p.x(),p.z());
					p.SetY(elv+air->Prop().GetGroundStandingHeight());

					sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
					air->SendCommand(cmd);
				}


				for(int i=0; i<n; i++)
				{
					wingman[i]=NULL;
					if(info.wingmanInfo[i].typeName[0]!=0)
					{
						wingman[i]=world->AddAirplane(info.wingmanInfo[i].typeName,YSFALSE);
					}
					if(wingman[i]==NULL)
					{
						wingman[i]=world->AddAirplane(info.playerAirInfo.typeName,YSFALSE);
					}
					a=air->GetAttitude();

					p.Set(r*pos[i*2],0.0,r*pos[i*2+1]);
					a.Mul(p,p);
					p+=air->GetPosition();

					sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
					wingman[i]->SendCommand(cmd);
					sprintf(cmd,"ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
					    YsRadToDeg(a.h()),YsRadToDeg(a.p()),YsRadToDeg(a.b()));
					wingman[i]->SendCommand(cmd);
					wingman[i]->AutoSendCommand(info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,info.playerAirInfo.fuel);
				}

				for(int i=0; i<2; i++)
				{
					p.Set(r*pos[i*2],vSep*pos[i*2+1],r*pos[i*2+1]);
					fom=FsFormation::Create();
					fom->minAlt=0.0;
					fom->leader=air;
					fom->shouldBe=p;
					fom->synchronizeTrigger=YSTRUE;
					wingman[i]->SetAutopilot(fom);
				}


				p.Set(r*pos[4],vSep*pos[5]-4.0,r*pos[5]);
				fom=FsFormation::Create();
				fom->minAlt=0.0;
				fom->leader=air;
				fom->shouldBe=p;

				p.DivX(3.0);
				fom->transition[fom->nTransition++]=p;
				p.SetX(0.0);
				fom->transition[fom->nTransition++]=p;
				p.SetY(vSep*pos[5]);
				fom->transition[fom->nTransition++]=p;

				if(info.fomType==FSFOM_DELTA)
				{
					fom->afterTransFormationAirplane.Append(world->GetAirplaneIdFromHandle(wingman[3]));
					fom->afterTransFormationPos.Append(YsVec3(-2.0*r,-vSep*2.0,-2.0*r));
					fom->afterTransFormationAirplane.Append(world->GetAirplaneIdFromHandle(wingman[4]));
					fom->afterTransFormationPos.Append(YsVec3( 2.0*r,-vSep*2.0,-2.0*r));
				}

				wingman[2]->SetAutopilot(fom);

				if(info.fomType==FSFOM_DELTA)
				{
					for(int i=3; i<5; i++)
					{
						p.Set(r*pos[i*2],-vSep*2.0,r*pos[i*2+1]);
						fom=FsFormation::Create();
						fom->minAlt=0.0;
						fom->leader=air;
						fom->shouldBe=p;
						fom->synchronizeTrigger=YSTRUE;
						wingman[i]->SetAutopilot(fom);
					}
				}
			}
		}
		break;
	}

	if(YSTRUE==info.envInfo.specifyEnvironment)
	{
		YsVec3 windVec;
		windVec.Set(0.0,0.0,-info.envInfo.windSpd);
		windVec.RotateXZ(-info.envInfo.windDir);
		world->SetWind(windVec);

		world->SetEnvironment(info.envInfo.dayOrNight);

		world->SetFog(info.envInfo.fog);
		world->SetFogVisibility(info.envInfo.fogVisibility);

		for(int i=0; i<=info.envInfo.cloudLayer.GetN()-2; i+=2)
		{
			world->AddOvercastLayer(info.envInfo.cloudLayer[i],info.envInfo.cloudLayer[i+1]);
		}
	}

	if(info.flyImmediately==YSTRUE)
	{
		Sim_Fly(nullptr);
	}
}

////////////////////////////////////////////////////////////
