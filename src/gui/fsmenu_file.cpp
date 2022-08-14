#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include <ysclass.h>
#include <ysport.h>

#include <fsgui.h>
#include <fsguifiledialog.h>

#include <fsdialog.h>
#include <fsworld.h>
#include <fsfilename.h>

#include <fstextresource.h>

#include "fsrunloop.h"
#include "fsmenu.h"
#include "fsguicommondialog.h"
#include "fsguimaincanvas.h"



void FsGuiMainCanvas::File_Open(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::File_Open_DeletedConfirmed,this);
		return;
	}
	File_Open_DeletedConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::File_Open_DeletedConfirmed(FsGuiDialog *,int returnCode)
{
	if(returnCode!=(int)YSOK)
	{
		return;
	}

	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		world->TerminateSimulation();
	}

	YsWString defFn;
	defFn.Set(FsGetUserYsflightDir());
	defFn.Append(L"/*.yfs");

	auto fdlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiFileDialog>();
	fdlg->Initialize();
	fdlg->mode=FsGuiFileDialog::MODE_OPEN;
	fdlg->title.Set(L"Open Flight");
	fdlg->fileExtensionArray.Append(L".yfs");
	fdlg->defaultFileName.Set(defFn);
	fdlg->BindCloseModalCallBack(&THISCLASS::File_Open_FileSelected,this);
	AttachModalDialog(fdlg);
}

void FsGuiMainCanvas::File_Open_FileSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto fdlg=dynamic_cast <FsGuiFileDialog *>(closedDialog);
	if(nullptr!=fdlg && (int)YSOK==returnCode && 0<fdlg->selectedFileArray.GetN())
	{
		printf("[%ls]\n",fdlg->selectedFileArray[0].Txt());

		auto world=runLoop->GetWorld();
		YSRESULT res=world->Load(fdlg->selectedFileArray[0]);
		if(res==YSOK)
		{
			mainMenu->AddRecentlyUsedFile(fdlg->selectedFileArray[0]);
		}
		else
		{
			StartMessageBox(
			    FSGUI_COMMON_ERROR,
			    FSGUI_FILEDLG_CANNOT_LOAD_FILE,
			    FSGUI_COMMON_OK,NULL,0,0);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::File_Save(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()!=YSTRUE)
	{
		StartMessageBox(
		    FSGUI_COMMON_ERROR,
		    FSERRMSG_NEEDCREATEFLIGHT,
		    FSGUI_COMMON_OK,NULL,0,0);
		return;
	}

	YsWString defFn;
	defFn.Set(FsGetUserYsflightDir());
	defFn.Append(L"/untitled.yfs");

	auto fdlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiFileDialog>();
	fdlg->Initialize();
	fdlg->mode=FsGuiFileDialog::MODE_SAVE;
	fdlg->title.Set(L"Save Flight");
	fdlg->fileExtensionArray.Append(L".yfs");
	fdlg->defaultFileName.Set(defFn);
	fdlg->BindCloseModalCallBack(&THISCLASS::File_SaveStep2,this);
	AttachModalDialog(fdlg);
}

void FsGuiMainCanvas::File_SaveStep2(FsGuiDialog *closedDialog,int returnCode)
{
	auto fdlg=dynamic_cast<FsGuiFileDialog *>(closedDialog);
	if(nullptr!=fdlg && (int)YSOK==returnCode && 0<fdlg->selectedFileArray.GetN())
	{
		printf("[%ls]\n",fdlg->selectedFileArray[0].Txt());

		const YsWString ful=fdlg->selectedFileArray[0];
		printf("[%ls]\n",ful.Txt());

		YsFileIO::File fp(ful,"rb");
		if(NULL==fp)
		{
			File_Save_Save(ful);
		}
		else
		{
			auto msgDlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiMessageBoxDialogWithPayload <YsWString> >();
			msgDlg->payload=fdlg->selectedFileArray[0];
			msgDlg->Make(
			    L"",
			    FSGUI_FILEDLG_ASK_OVERWRITE,
			    FSGUI_COMMON_YES,FSGUI_COMMON_NO);
			msgDlg->BindCloseModalCallBack(&THISCLASS::File_SaveStep3,this);
			AttachModalDialog(msgDlg);
		}
	}
}

void FsGuiMainCanvas::File_SaveStep3(FsGuiDialog *closedDialog,int returnCode)
{
	auto msgDlg=dynamic_cast <FsGuiMessageBoxDialogWithPayload <YsWString> *>(closedDialog);
	if(nullptr!=msgDlg && (int)YSOK==returnCode)
	{
		File_Save_Save(msgDlg->payload);
	}
}

void FsGuiMainCanvas::File_Save_Save(YsWString ful)
{
	auto world=runLoop->GetWorld();
	if(world->Save(ful,3,4,2,2,2,2,0.0)==YSOK)
	{
		StartMessageBox(
		    L"",
		    FSGUI_FILEDLG_SAVED,
		    FSGUI_COMMON_CLOSE,NULL,
		    0,0);
	}
	else
	{
		StartMessageBox(
		    FSGUI_COMMON_ERROR,
		    FSGUI_FILEDLG_CANNOT_WRITE_FILE,
		    FSGUI_COMMON_CLOSE,NULL,
		    0,0);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::File_OpenMission(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::File_OpenMission_DeleteConfirmed,this);
		return;
	}
	File_OpenMission_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::File_OpenMission_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if(returnCode!=(int)YSOK)
	{
		return;
	}

	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		world->TerminateSimulation();
	}

	auto dlg=StartSelectMissionDialog();
	dlg->BindCloseModalCallBack(&THISCLASS::File_OpenMission_MissionSelected,this);
}

void FsGuiMainCanvas::File_OpenMission_MissionSelected(FsGuiDialog *dlg,int returnCode)
{
	auto openMissionDialog=dynamic_cast <FsGuiSelectMissionDialog *>(dlg);

	if(nullptr==openMissionDialog)
	{
		printf("Fatal internal error!\n");
		printf("%s %d\n",__FUNCTION__,__LINE__);
		exit(1);
	}

	YSBOOL flyNow=YSFALSE;
	if(openMissionDialog->lastClicked==openMissionDialog->okBtn)
	{
		flyNow=YSFALSE;
	}
	else if(openMissionDialog->lastClicked==openMissionDialog->flyNowBtn)
	{
		flyNow=YSTRUE;
	}
	else // if(openMissionDialog->lastClicked==openMissionDialog->cancelBtn)
	{
		return;
	}

	YsWString missionSel;
	if(YSOK==openMissionDialog->GetSelectedMission(missionSel))
	{
		YsWString ful;
		ful.Set(L"mission/");
		ful.Append(missionSel);

		YsString cFul;
		cFul.EncodeUTF8 <wchar_t> (ful);
		printf("[%s]\n",cFul.Txt());
	
		auto world=runLoop->GetWorld();
		if(YSOK==world->Load(ful))
		{
			if(flyNow==YSTRUE)
			{
				Sim_Fly(nullptr);
			}
		}
		else
		{
			StartMessageBox(
			    L"ERROR",
			    L"Cannot Load File",
			    FSGUI_COMMON_OK,NULL,0,0);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::File_OpenPrevFlight(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::File_OpenPrevFlight_DeleteConfirmed,this);
		return;
	}
	File_OpenPrevFlight_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::File_OpenPrevFlight_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK!=returnCode)
	{
		return;
	}

	YsString versionStr;
	{
		YsFileIO::File fp(FsGetPrevFlightFile(),"r");
		if(fp!=NULL)
		{
			versionStr.Fgets(fp);
		}
		else
		{
			StartMessageBox(
			    L"ERROR",
			    FSERRMSG_NOPREVFLIGHT,
			    FSGUI_COMMON_OK,NULL,0,0);
			return;
		}
	}

	if(YSTRUE==versionStr.DOESSTARTWITH("YFSVERSI"))
	{
		auto world=runLoop->GetWorld();
		world->TerminateSimulation();

		auto res=world->Load(FsGetPrevFlightFile());

		if(res!=YSOK)
		{
			StartMessageBox(
			    L"Error",
			    L"Failed to load the previous mission.",
			    FSGUI_COMMON_OK,NULL,0,0);
		}
	}
	else if(YSTRUE==versionStr.DOESSTARTWITH("ENDURANCE") || 
	        YSTRUE==versionStr.DOESSTARTWITH("INTERCEPT") ||
	        YSTRUE==versionStr.DOESSTARTWITH("CLSAIRSPT"))
	{
		StartMessageBox(
		    L"Error",
		    L"Endurance Mode and Intercept Mission cannot be loaded.\n"
		    L"Use \"Flight\"->\"Retry Previous Mission\"",
		    FSGUI_COMMON_OK,NULL,0,0);
		return;
	}
	else
	{
		StartMessageBox(
		    L"ERROR",
		    FSERRMSG_NOPREVFLIGHT,
		    FSGUI_COMMON_OK,NULL,0,0);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::File_CloseFlight(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::File_CloseFlight_DeleteConfirmed,this);
		return;
	}
	File_CloseFlight_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::File_CloseFlight_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK!=returnCode)
	{
		return;
	}

	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		world->TerminateSimulation();
	}

	world->UnprepareAllTemplate();
	YsShell::CleanUpVertexPolygonStore();
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::File_Exit(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::File_Exit_DeleteConfirmed,this);
		return;
	}
	File_Exit_DeleteConfirmed(nullptr,(int)YSOK);
}

void FsGuiMainCanvas::File_Exit_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		auto dlg=StartMessageBox(
		    L"",
		    FSGUI_EXITDLG_MESSAGE,
		    FSGUI_EXITDLG_EXIT,FSGUI_EXITDLG_BACKTOMENU,
		    (int)YSOK,(int)YSERR);
		dlg->BindCloseModalCallBack(&THISCLASS::File_Exit_ReallyExit,this);
	}
}

void FsGuiMainCanvas::File_Exit_ReallyExit(FsGuiDialog *,int returnCode)
{
	if((int)YSOK==returnCode)
	{
		runLoop->SetTerminateFlag(YSTRUE);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::File_Recent(FsGuiPopUpMenuItem *itm)
{
	const wchar_t *wfn=itm->GetString();

	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::File_Recent_DeleteConfirmed,this);
		dlg->whatToOpenNext=wfn;
		return;
	}

	File_Recent_Load(wfn);
}

void FsGuiMainCanvas::File_Recent_DeleteConfirmed(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast <FsGuiConfirmDeleteFlightDialog *>(closedDialog);
	if((int)YSOK==returnCode && nullptr!=dlg)
	{
		File_Recent_Load(dlg->whatToOpenNext);
	}
}

void FsGuiMainCanvas::File_Recent_Load(const YsWString &fn)
{
printf("%s %d\n",__FUNCTION__,__LINE__);
printf("%ls\n",fn.Txt());
	auto world=runLoop->GetWorld();

	if(world->SimulationIsPrepared()==YSTRUE)
	{
		world->TerminateSimulation();
	}

	YSRESULT res=world->Load(fn);

	if(res==YSOK)
	{
		mainMenu->AddRecentlyUsedFile(fn);
	}
	else
	{
		StartMessageBox(
		    FSGUI_COMMON_ERROR,
		    FSGUI_FILEDLG_CANNOT_LOAD_FILE,
		    FSGUI_COMMON_OK,NULL,0,0);
	}
}
