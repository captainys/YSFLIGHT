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



void FsGuiMainCanvas::UtilSetSpacePrecision(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE &&
	   world->PlayerPlaneIsReady()!=YSTRUE)
	{
		auto dlg=StartInputNumberDialog(
			/*default=*/0.01, /*belowDecimal=*/2,
			L"Set Spatial Precision",L"Precision (0.001m to 1.0m)",L"",
			FSGUI_COMMON_OK,FSGUI_COMMON_CANCEL,(int)YSOK,(int)YSERR);
		dlg->BindCloseModalCallBack(&THISCLASS::UtilSetSpacePrecision_OptionSelected,this);
	}
	else
	{
		StartMessageBox(L"ERROR",FSERRMSG_NOFLTRECORD,FSGUI_COMMON_OK,NULL,0,0);
	}
}

void FsGuiMainCanvas::UtilSetSpacePrecision_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiInputNumberDialog *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		const double prec=dlg->GetNumber();
		if(prec<0.001F || 1.0<prec)
		{
			StartMessageBox(L"Error",L"Invalid Precision",FSGUI_COMMON_OK,NULL,0,0);
			return;
		}

		const double posPrec=prec;
		const double angPrec=YsSmaller(prec/10.0,0.01);
		runLoop->GetWorld()->AdjustPrecisionOfFlightRecord(posPrec,angPrec);

		StartMessageBox(L"Done.",L"Done.",FSGUI_COMMON_OK,NULL,0,0);
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::UtilSetTimeInterval(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE &&
	   world->PlayerPlaneIsReady()!=YSTRUE)
	{
		auto dlg=StartInputNumberDialog(
			/*default=*/0.05, /*belowDecimal=*/2,
			L"Set Time Precision",L"Record Time Interval (0.0sec to 2.0sec)",L"",
			FSGUI_COMMON_OK,FSGUI_COMMON_CANCEL,(int)YSOK,(int)YSERR);
		dlg->BindCloseModalCallBack(&THISCLASS::UtilSetTimeInterval_OptionSelected,this);
	}
	else
	{
		StartMessageBox(L"ERROR",FSERRMSG_NOFLTRECORD,FSGUI_COMMON_OK,NULL,0,0);
	}
}

void FsGuiMainCanvas::UtilSetTimeInterval_OptionSelected(FsGuiDialog *closedDialog,int returnCode)
{
	auto dlg=dynamic_cast<FsGuiInputNumberDialog *>(closedDialog);
	if(nullptr!=dlg && (int)YSOK==returnCode)
	{
		const double itvl=dlg->GetNumber();
		if(itvl<0.0 || 2.0<itvl)
		{
			StartMessageBox(L"Error",L"Invalid Interval",FSGUI_COMMON_OK,NULL,0,0);
			return;
		}

		auto world=runLoop->GetWorld();
		const int n=world->RerecordByNewInterval(itvl);
		if(n>0)
		{
			YsString msg;
			msg.Printf("Reduced %d steps",n);

			YsWString wStr;
			wStr.SetUTF8String(msg);
			StartMessageBox(wStr,wStr,FSGUI_COMMON_OK,NULL,0,0);
		}
		else
		{
			StartMessageBox(L"",L"No reduction in data length",FSGUI_COMMON_OK,NULL,0,0);
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiMainCanvas::Util_EditFlightRecord(FsGuiPopUpMenuItem *)
{
	auto world=runLoop->GetWorld();
	if(world->SimulationIsPrepared()==YSTRUE &&
	   world->PlayerPlaneIsReady()!=YSTRUE)
	{
		if(YSOK!=world->CheckJoystickAssignmentAndFixIfNecessary() && YSTRUE==FsSimulation::NeedNoJoystickWarningDialog())
		{
			auto dlg=StartNoJoystickWarningDialog();
			dlg->BindCloseModalCallBack(&THISCLASS::Util_EditFlightRecord_JoystickWarned,this);
			return;
		}
		Util_EditFlightRecord_JoystickWarned(nullptr,0);
	}
	else
	{
		StartMessageBox(L"ERROR",FSERRMSG_NOFLTRECORD,FSGUI_COMMON_OK,NULL,0,0);
	}
}

void FsGuiMainCanvas::Util_EditFlightRecord_JoystickWarned(FsGuiDialog *,int)
{
	runLoop->StartReplayRecord(/*EditMode=*/YSTRUE);
}

