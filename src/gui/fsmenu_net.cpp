#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include <ysclass.h>
#include <ysport.h>

#include "fsnetconfig.h"

#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"
#include "fsmenu.h"

#include "graphics/common/fsopengl.h"
#include "fswirefont.h"

#include "fsguinetdialog.h"
#include "fsrunloop.h"

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

#include "fsguiconfigdlg.h"

#include "fsmenu.h"
#include "fsguicommondialog.h"
#include "fsguimaincanvas.h"

void FsGuiMainCanvas::Net_StartServerMode(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Net_StartServerMode_DeleteConfirmed,this);
	}
	else
	{
		Net_StartServerMode_DeleteConfirmed(nullptr,(int)YSOK);
	}
}

void FsGuiMainCanvas::Net_StartServerMode_DeleteConfirmed(FsGuiDialog *closedDlg,int returnCode)
{
	if((int)YSOK!=returnCode)
	{
		return;
	}

	if(YSOK==FsSimulation::TestAssignJoystick() || YSTRUE!=FsSimulation::NeedNoJoystickWarningDialog())
	{
		Net_StartServerMode_JoystickWarned(nullptr,0);
	}
	else
	{
		auto dlg=StartNoJoystickWarningDialog();
		dlg->BindCloseModalCallBack(&THISCLASS::Net_StartServerMode_JoystickWarned,this);
	}
}

void FsGuiMainCanvas::Net_StartServerMode_JoystickWarned(FsGuiDialog *,int)
{
	FsNetConfig cfg;
	cfg.Load(FsGetNetConfigFile());

	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiStartServerDialog>();
	dlg->Initialize();
	dlg->Make(runLoop->GetWorld(),cfg);
	dlg->BindCloseModalCallBack(&THISCLASS::Net_StartServerMode_OptionSelected,this);

	AttachModalDialog(dlg);
}

void FsGuiMainCanvas::Net_StartServerMode_OptionSelected(FsGuiDialog *closedDialog,int)
{
	auto svrDlg=dynamic_cast<FsGuiStartServerDialog *>(closedDialog);
	if(nullptr!=svrDlg && YSOK==svrDlg->res)
	{
		YsString username;
		if(NULL!=svrDlg->userNameTxt)
		{
			svrDlg->userNameTxt->GetText(username);
		}

		YsString fldName;
		if(YSOK!=svrDlg->fldListBox->GetSelectedString(fldName))
		{
			return;
		}

		runLoop->StartNetServerMode(username,fldName,svrDlg->portTxt->GetInteger());
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Net_StartClientMode(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE)
	{
		auto dlg=StartConfirmDeleteFlightDialog((int)YSOK);
		dlg->BindCloseModalCallBack(&THISCLASS::Net_StartClientMode_DeleteConfirmed,this);
	}
	else
	{
		Net_StartClientMode_DeleteConfirmed(nullptr,(int)YSOK);
	}
}

void FsGuiMainCanvas::Net_StartClientMode_DeleteConfirmed(FsGuiDialog *,int returnCode)
{
	if((int)YSOK!=returnCode)
	{
		return;
	}

	if(YSOK==FsSimulation::TestAssignJoystick() || YSTRUE!=FsSimulation::NeedNoJoystickWarningDialog())
	{
		Net_StartClientMode_JoystickWarned(nullptr,(int)YSOK);
	}
	else
	{
		auto dlg=StartNoJoystickWarningDialog();
		dlg->BindCloseModalCallBack(&THISCLASS::Net_StartClientMode_JoystickWarned,this);
	}
}

void FsGuiMainCanvas::Net_StartClientMode_JoystickWarned(FsGuiDialog *,int)
{
	FsNetConfig cfg;
	cfg.Load(FsGetNetConfigFile());

	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiStartClientDialog>();
	dlg->Initialize();
	dlg->Make(cfg);
	dlg->BindCloseModalCallBack(&THISCLASS::Net_StartClientMode_OptionSelected,this);

	AttachModalDialog(dlg);
}

void FsGuiMainCanvas::Net_StartClientMode_OptionSelected(FsGuiDialog *closedDialog,int)
{
	auto cliDlg=dynamic_cast <FsGuiStartClientDialog *>(closedDialog);
	if(nullptr!=cliDlg && YSOK==cliDlg->res)
	{
		YsString hostname,username;
		cliDlg->hostAddrTxt->GetText(hostname);
		cliDlg->userNameTxt->GetText(username);
		int netPort=cliDlg->portTxt->GetInteger();

		runLoop->StartNetClientMode(username,hostname,netPort);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Net_Config(FsGuiPopUpMenuItem *)
{
	auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiNetConfigDialog>();

	FsNetConfig cfg;
	cfg.Load(FsGetNetConfigFile());

	dlg->MakeDialog();
	dlg->InitializeDialog(runLoop->GetWorld(),cfg);
	dlg->MakeBlockedIpList(dlg->svrBlockedIpAddr);

	AttachModalDialog(dlg);
}
