#include <memory>
#include <queue>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h> // This needs to be included before windows.h (is included somewhere else.)
#endif

#include <fslazywindow.h>

#include <ysclass.h>
#include <ysport.h>
#include <fsgui.h>

#include "fsconfig.h"
#include "fsapplyoption.h"
#include "fsnetconfig.h"

#include "graphics/common/fsconsole.h"

#include "fs.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"
#include "fsmenu.h"
#include "fsfilename.h"

#include "fspluginmgr.h"

#include "graphics/common/fsfontrenderer.h"

#include "fstextresource.h"

#include "fsatc.h"

#include "fsrunloop.h"
#include "fsguicommondialog.h"
#include "fsguimaincanvas.h"

#include "graphics/common/fstexturemanager.h"

#ifdef _WIN32
#include <float.h>
#include <direct.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif


#include "fscmdparaminfo.h"
#include "fsscript.h"


// YSFS_TESTVERSION macro may be defined in fs.h
// #define CRASH_ON_ZERO_DIVISION_ERROR


#include <ysbitmap.h>

const wchar_t *FsProgramName=L"YSFLIGHT";  // Different names for screen saver programs

#ifndef YSFS_TESTVERSION
const char *FsProgramTitle="YS FLIGHT SIMULATOR";  // Different names for screen saver programs
#else
const char *FsProgramTitle="YS FLIGHT SIMULATOR - Untested";  // Different names for screen saver programs
#endif


////////////////////////////////////////////////////////////



class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool initialized;
	int initializationCounter;
	YSBOOL firstStart,failedToRecordFirstLaunch;
	FsGuiMainCanvas *mainCanvasPtr;

	FsWorld *world;
	FsOption opt;
	FsFlightConfig cfg;
	std::shared_ptr <FsRunLoop> runLoopPtr;
	std::shared_ptr <FsRunLoop::GuiCanvasBase> canvasPtr;
	FsScript testScript;
	FsCommandParameter fscp;
	YsWString prevCWD;

public:
	FsLazyWindowApplication();
	virtual void BeforeEverything(int argc,char *argv[]);
	virtual void GetOpenWindowOption(FsOpenWindowOption &OPT) const;
	virtual void Initialize(int argc,char *argv[]);
	virtual void Interval(void);
	virtual void BeforeTerminate(void);
	virtual void Draw(void);
	virtual bool UserWantToCloseProgram(void);
	virtual bool MustTerminate(void) const;
	virtual long long int GetMinimumSleepPerInterval(void) const;
	virtual bool NeedRedraw(void) const;
	virtual void ContextLostAndRecreated(void);

	YSBOOL StepByStepInitialization(void);
	void SaveWindowPositionAndSizeIfNecessary(void);
};

FsLazyWindowApplication::FsLazyWindowApplication()
{
	world=nullptr;
}

/* virtual */ void FsLazyWindowApplication::BeforeEverything(int ac,char *av[])
{
	printf("YSFLIGHT\n");
	printf("VERSION %d\n",YSFLIGHT_VERSION);
	printf("YFSVERSION %d\n",YSFLIGHT_YFSVERSION);
	printf("NETVERSION %d\n",YSFLIGHT_NETVERSION);

	YsCoordSysModel=YSLEFT_ZPLUS_YPLUS;

	prevCWD=YsFileIO::Getcwd();

	FsChangeToProgramDir();



	if(YsFileExist("misc/aim9.srf")!=YSTRUE)                                    // 2005/03/14 CWD test
	{                                                                           // 2005/03/14
		char fil[512],pth[512];                                                 // 2005/03/14
		YsSeparatePathFile(pth,fil,av[0]);                                      // 2005/03/14
		if(pth[0]!=0 && (pth[strlen(pth)-1]=='/' || pth[strlen(pth)-1]=='\\'))  // 2005/03/14
		{                                                                       // 2005/03/14
			pth[strlen(pth)-1]=0;                                               // 2005/03/14
		}                                                                       // 2005/03/14
		chdir(pth);                                                             // 2005/03/14
		printf("Move CWD to %s\n",pth);                                         // 2005/03/14
	}                                                                           // 2005/03/14



	// At this time, FsRunLoop doesn't exist yet.
	// It needs its own command parameter info.
	// The program looks through command parameter twice, but so what?
	if(fscp.RecognizeCommandParameter(ac,av)!=YSOK)
	{
		exit(1);
	}

	if(fscp.setDefConfig==YSTRUE)
	{
		FsFlightConfig cfg;
		cfg.Save(FsGetConfigFile());
	}

	if(fscp.setDefNetConfig==YSTRUE)
	{
		FsNetConfig netCfg;
		netCfg.Save(FsGetNetConfigFile());
		FsInterceptMissionInfo interceptInfo;
		interceptInfo.Save(FsGetNetInterceptMissionConfigFile());
	}

	if(fscp.setDefKey==YSTRUE)
	{
		FsControlAssignment ass;
		ass.Save(FsGetControlAssignFile());
	}

	if(fscp.setDefOption==YSTRUE)
	{
		FsOption opt;
		opt.Save(FsGetOptionFile());
		YsFileIO::Remove(FsGetFontConfigFile());
		YsFileIO::Remove(FsGetWindowSizeFile());
		YsFileIO::Remove(FsGetNetServerAddressHistoryFile());
	}



#ifdef CRASH_ON_ZERO_DIVISION_ERROR
	printf("FPU Setting >>\n");
	// _control87(0,_EM_ZERODIVIDE); // Check only Zero division
	// _control87(0,_MCW_EM);  // All FPU Exception (Doesn't work on Athlon XP)
	// _control87(0,_EM_ZERODIVIDE|_EM_INVALID|_EM_DENORMAL|_EM_OVERFLOW|_EM_UNDERFLOW);  // Strict Check for Floating-Point Exception
	_control87(0,_EM_ZERODIVIDE|_EM_INVALID|_EM_OVERFLOW);  // _EM_UNDERFLOW,_EM_DENORMAL will cause crash too often.  See update.txt 2008/11/18
	printf("FPU Setting <<\n");
#endif



	opt.Load(FsGetOptionFile());
	cfg.Load(FsGetConfigFile());

	FsBeforeOpenWindow(opt,cfg);
}
/* virtual */ void FsLazyWindowApplication::GetOpenWindowOption(FsOpenWindowOption &owo) const
{
	owo=FsGetOpenWindowOption(opt,cfg,FsGetWindowSizeFile(),FsMainWindowTitle());
}
/* virtual */ void FsLazyWindowApplication::Initialize(int argc,char *argv[])
{
	// Either of Draw or Interval may be called next.
	// Make sure that Draw can render console after this function.

	FsInitializeOpenGL();
	FsAfterOpenWindow(opt,cfg);

	FsSoundInitialize();

	FsGuiObject::defAsciiRenderer=&fsAsciiRenderer;
	FsGuiObject::defUnicodeRenderer=&fsUnicodeRenderer;

	FsSetFont(opt.fontName,opt.fontHeight);

	FsLoadPlugIn();
	FsApplyNonScreenOption(opt);

	srand((unsigned int)time(NULL));

	FsAirplaneAllocator.SetAllocUnit(16);
	FsGroundAllocator.SetAllocUnit(64);

	initialized=false;
	initializationCounter=0;
}
/* virtual */ void FsLazyWindowApplication::Interval(void)
{
	if(true!=initialized)
	{
		printf("Initialization Counter %d\n",initializationCounter);
		this->StepByStepInitialization();
	}
	else
	{
		testScript.RunOneStep();
		if(YSTRUE!=runLoopPtr->RunOneStep())
		{
			SetMustTerminate(true);
		}
	}
}
/* virtual */ void FsLazyWindowApplication::BeforeTerminate(void)
{
	FsTaskBarDeleteIcon();

printf("Closing-1\n");
	if(nullptr!=world)
	{
		auto &fscp=runLoopPtr->GetCommandParameter();
		if(fscp.autoSave==YSTRUE)
		{
			if(world->SimulationIsPrepared()==YSTRUE)
			{
				if(world->Save(fscp.yfsFilename,3,4,2,2,2,2,0.0)==YSOK)
				{
					YsString cYfsFilename;
					YsUnicodeToSystemEncoding(cYfsFilename,fscp.yfsFilename);
					printf("Saved %s\n",cYfsFilename.Txt());
				}
				else
				{
					printf("Could not save the flight.\n");
				}
			}
		}
	}

printf("Closing-4\n");

	SaveWindowPositionAndSizeIfNecessary();

printf("Closing-5\n");

	// Memo: Window position can be saved at event handler of WM_CLOSE

printf("Closing-6\n");

	runLoopPtr.reset();

printf("Closing-9\n");

	// FsAirplaneAllocator.SelfDiagnostic();
	// FsGroundAllocator.SelfDiagnostic();

	FsWeaponHolder::FreeMissilePattern();
	YsScenery::CollectGarbage(); // 2018/10/14 Prevent crash in macOS.  static stroages in YsScenery were not freed in a happy order.

printf("Closing-10\n");
	FsFreePlugIn();
	FsSoundTerminate();
printf("Closing-11\n");
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{
	if(true==initialized)
	{
		runLoopPtr->Draw();
	}
	else
	{
		fsConsole.Show();
	}
}
/* virtual */ bool FsLazyWindowApplication::UserWantToCloseProgram(void)
{
printf("%s %d\n",__FUNCTION__,__LINE__);
	SaveWindowPositionAndSizeIfNecessary();
	FsTaskBarDeleteIcon();
	return true; // Returning true will just close the program.
}
/* virtual */ bool FsLazyWindowApplication::MustTerminate(void) const
{
	return FsLazyWindowApplicationBase::MustTerminate();
}
/* virtual */ long long int FsLazyWindowApplication::GetMinimumSleepPerInterval(void) const
{
	return 0;
}
/* virtual */ bool FsLazyWindowApplication::NeedRedraw(void) const
{
	if(true==initialized)
	{
		return (YSTRUE==runLoopPtr->NeedRedraw() ? true : false);
	}
	else
	{
		return true;
	}
}

void FsLazyWindowApplication::SaveWindowPositionAndSizeIfNecessary(void)
{
	opt.Load(FsGetOptionFile());

	auto &fscp=runLoopPtr->GetCommandParameter();
	if(opt.scrnMode==0 && 
	   opt.rememberWindowSize==YSTRUE &&
	   fscp.prepareRelease!=YSTRUE)
	{
		FsSaveWindowSize(FsGetWindowSizeFile());
	}
}

YSBOOL FsLazyWindowApplication::StepByStepInitialization(void)
{
	switch(initializationCounter)
	{
	default:
		initialized=true;
		return YSTRUE;
	case 0:
		{
			fsConsole.Printf("**** STARTING YS FLIGHT SIMULATOR ****\n");
		}
		break;
	case 1:
		{
			mainCanvasPtr=new FsGuiMainCanvas;
			auto runLoop=new FsRunLoop(FsRunLoop::StepByStepInitializationOption());
			runLoopPtr.reset(runLoop);
			canvasPtr.reset(mainCanvasPtr);
		}
		break;
	case 2:
		if(YSTRUE!=runLoopPtr->InitializeOneStep(FsWorld::InitializationOption()))
		{
			return YSFALSE; // Don't increment the counter until getting YSTRUE.
		}
		break;
	case 3:
		{
			testScript.SetYSFLIGHT(runLoopPtr,canvasPtr);
			if(0<fscp.testScriptFilename.size())
			{
				YsWString ful;
				ful.MakeFullPathName(prevCWD,fscp.testScriptFilename);

				YsFileIO::File ifp(ful,"r");
				if(nullptr==ifp)
				{
					ifp.Fopen(fscp.testScriptFilename,"r");
				}

				if(nullptr!=ifp)
				{
					YsString str;
					while(nullptr!=str.Fgets(ifp))
					{
						testScript.AddLine(str);
					}
				}
			}

			runLoopPtr->SetCommandParameter(fscp);

			mainCanvasPtr->MakeMainMenu(runLoopPtr.get());
		}
		break;
	case 4:
		mainCanvasPtr->MakeMainDialog(runLoopPtr.get());
		runLoopPtr->SetMainCanvas(canvasPtr.get());
		break;
	case 5:
	#ifdef YSFS_TESTVERSION
		if(YSTRUE!=FsIsConsoleServer() && YSTRUE!=fscp.autoExit)
		{
			auto testVerMessageDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
			testVerMessageDlg->Initialize();
			testVerMessageDlg->Make(
			    L"YSFLIGHT  Untested Version",
			    FSGUI_MISC_TESTVERSION,
			    FSGUI_COMMON_CLOSE,
			    NULL,
			    0,0);
			testVerMessageDlg->BindCloseModalCallBack(&FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed,mainCanvasPtr);
			canvasPtr->AppendModalDialog(testVerMessageDlg);
		}
		else
		{
			printf("-- YSFLIGHT Untested Version --\n");
			printf("This program is an untested version of YSFLIGHT for debugging purpose,\n");
			printf("i.e., I haven't systematically tested this program.  Naturally, a lot\n");
			printf("of bugs must be included in this program.  If you'd like to use a little\n");
			printf("more stable version, please download current latest official release\n");
			printf("version.  If you notice a bug in this untested version, please E-Mail\n");
			printf("me (PEB01130@nifty.com).  I'm trying to reply as many as possible,\n");
			printf("but sometimes (especially when I'm busy) you may receive my reply\n");
			printf("after two or three months, or sometimes I just forget.  I apologize\n");
			printf("if you experience such delay (^_^;)\n");
			printf("\n");
		}
	#endif


		firstStart=YSTRUE;
		failedToRecordFirstLaunch=YSFALSE;

		if(YSTRUE==fscp.deleteFirstStartFile)
		{
			YsWString fn;
			YsString cStr;
			for(int i=0; NULL!=FsGetFirstStartFile(fn,i); i++)
			{
				YsUnicodeToSystemEncoding(cStr,fn);
				YsPrintf("Removing %s\n",cStr.Txt());
				YsFileIO::Remove(fn);
			}

			firstStart=YSFALSE; // Prevent first dialog from being shown.
		}
		else if(fscp.prepareRelease==YSTRUE)
		{
			int i;
			YsWString fn;
			YsString cStr;

			YsUnicodeToSystemEncoding(cStr,FsGetVoteYsflightFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetVoteYsflightFile());

			for(i=0; NULL!=FsGetFirstStartFile(fn,i); i++)
			{
				YsUnicodeToSystemEncoding(cStr,fn);
				YsPrintf("Removing %s\n",cStr.Txt());
				YsFileIO::Remove(fn);
			}

			YsUnicodeToSystemEncoding(cStr,FsGetPrevFlightFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetPrevFlightFile());

			YsUnicodeToSystemEncoding(cStr,FsGetFontConfigFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetFontConfigFile());

			YsUnicodeToSystemEncoding(cStr,FsGetWindowSizeFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetWindowSizeFile());

			YsUnicodeToSystemEncoding(cStr,FsGetErrFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetErrFile());

			YsUnicodeToSystemEncoding(cStr,FsGetNetServerAddressHistoryFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetNetServerAddressHistoryFile());

			YsUnicodeToSystemEncoding(cStr,FsGetIpBlockFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetIpBlockFile());

			YsArray <YsWString> fileList;
			if(FsFindFileList(fileList,FsGetNetChatLogDir(),L"",L"txt")==YSOK)
			{
				int i;
				YsWString str;
				forYsArray(i,fileList)
				{
					str.Set(FsGetNetChatLogDir());
					str.Append(L"/");
					str.Append(fileList[i]);

					YsUnicodeToSystemEncoding(cStr,str);
					YsPrintf("Removing %s\n",cStr.Txt());
					YsFileIO::Remove(str);
				}
			}

			YsUnicodeToSystemEncoding(cStr,FsGetDebugInfoFile());
			YsPrintf("Removing %s\n",cStr.Txt());
			YsFileIO::Remove(FsGetDebugInfoFile());

			firstStart=YSFALSE; // Prevent first dialog from being shown.
		}
		else
		{
			int i;
			YsWString fn;
			for(i=0; NULL!=FsGetFirstStartFile(fn,i); i++)
			{
				FILE *fp=YsFileIO::Fopen(fn,"rb");
				if(NULL!=fp)
				{
					YsString cStr;
					YsUnicodeToSystemEncoding(cStr,fn);
					printf("Detected %s\n",cStr.Txt());

					fclose(fp);
					firstStart=YSFALSE;
					break;
				}
			}

			if(YSTRUE==firstStart)
			{
				if(FsControlAssignment::MergeDefaultControl()==YSOK)
				{
					printf("Default Key/Joystick Assignments have been added.\n");
				}

				YsFileIO::MkDir(FsGetNetChatLogDir());

				YSBOOL recorded=YSFALSE;
				YsWString fn;

				for(i=0; NULL!=FsGetFirstStartFile(fn,i); i++)
				{
					FILE *fp=YsFileIO::Fopen(fn,"w");
					if(fp!=NULL)
					{
						recorded=YSTRUE;
						fprintf(fp,"YS\n");
						fclose(fp);
					}
				}

				if(YSTRUE!=recorded)
				{
					failedToRecordFirstLaunch=YSTRUE;
				}
			}


			if(YSTRUE==failedToRecordFirstLaunch)
			{
				auto failedToRecordFirstLaunchDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
				failedToRecordFirstLaunchDlg->Initialize();
				failedToRecordFirstLaunchDlg->Make(
				    L"Error",
				    FSGUI_MISC_FIRSTSTARTRECORDFAILURE,
				    FSGUI_COMMON_CLOSE,
				    NULL,
				    0,0);
				failedToRecordFirstLaunchDlg->BindCloseModalCallBack(&FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed,mainCanvasPtr);
				canvasPtr->AppendModalDialog(failedToRecordFirstLaunchDlg);
			}

			if(FsGetDate()<=20100315)
			{
				FILE *voteFp=YsFileIO::Fopen(FsGetVoteYsflightFile(),"rb");
				if(NULL==voteFp)
				{
					voteFp=YsFileIO::Fopen(FsGetVoteYsflightFile(),"w");
					if(NULL!=voteFp)
					{
						fprintf(voteFp,"Final Round of Vector Award is open through March 15, 2010\n");
						fprintf(voteFp,"Please vote for YSFLIGHT from the following URL:\n");
						fprintf(voteFp,"  http://www.vector.co.jp/award/vote.html?no=se121250&vasts=vote\n");
						fclose(voteFp);
					}

					auto voteYsflightDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiVoteYsflightDialogClass>();
					voteYsflightDlg->Initialize();
					voteYsflightDlg->Make();
					canvasPtr->AppendModalDialog(voteYsflightDlg);
				}
				else
				{
					fclose(voteFp);
				}
			}
		}

		if(YSTRUE==firstStart)
		{
			auto firstLaunchDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiFirstDialogClass>();
			firstLaunchDialog->Initialize();
			firstLaunchDialog->Make(0);
			firstLaunchDialog->BindCloseModalCallBack(&FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed,mainCanvasPtr);
			canvasPtr->AppendModalDialog(firstLaunchDialog);

			// auto supportYsflightDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiSupportYsflightDialogClass>();
			// supportYsflightDlg->Initialize();
			// supportYsflightDlg->Make(firstStart,0);
			// supportYsflightDlg->BindCloseModalCallBack(&FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed,mainCanvasPtr);
			// canvasPtr->AppendModalDialog(supportYsflightDlg);
		}

		if(fscp.autoExit==YSTRUE &&
		   (fscp.setDefConfig==YSTRUE || fscp.setDefNetConfig==YSTRUE ||
		    fscp.setDefKey==YSTRUE || fscp.setDefOption==YSTRUE))
		{
			auto initializeNotificationDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
			initializeNotificationDlg->Initialize();
			initializeNotificationDlg->Make(
			    L"Notification",
			    FSGUI_MISC_INITIALIZED,
			    FSGUI_COMMON_CLOSE,
			    NULL,
			    0,0);
			initializeNotificationDlg->BindCloseModalCallBack(&FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed,mainCanvasPtr);
			// This doesn't need a stack - initializeNotificationDlg.AttachModalDialog(startUpDialogStack);
			canvasPtr->AttachModalDialog(initializeNotificationDlg);
		}
		break;
	case 6:
		this->world=NULL;
		FsWeaponHolder::LoadMissilePattern();
		break;
	case 7:
		runLoopPtr->ChangeRunMode(FsRunLoop::YSRUNMODE_MENU);
		this->world=runLoopPtr->GetWorld();
		if(NULL!=world)
		{
			FsRunLoop &fsRunLoop=*runLoopPtr;
			fsConsole.Printf("Checking Available Title Bitmaps");

			FsPollDevice();
			if(FsGetKeyState(FSKEY_SHIFT)==YSTRUE && FsGetKeyState(FSKEY_CTRL)==YSTRUE)
			{
				FsSoundSetMasterSwitch(YSFALSE);
				fsStderr.Printf("Turn Off Sound Temporarily.");
			}

			//printf("Airplane List\n");
			//for(i=0; world->GetAirplaneTemplateName(i)!=NULL; i++)
			//{
			//	printf("\"%s\"\n",world->GetAirplaneTemplateName(i));
			//}
			//printf("--------\n");

			fsConsole.Printf("Preparing Joystick Polygon Model");
			FsFlightControl::PrepareJoystickPolygonModel();

			if(fscp.showAirList==YSTRUE)
			{
				int n;
				const char *nam;
				n=0;
				while(world->GetAirplaneTemplateName(n)!=NULL)
				{
					n++;
				}
				printf("NAIR:%d\n",n);
				n=0;
				while((nam=world->GetAirplaneTemplateName(n))!=NULL)
				{
					printf("AIR:%s\n",nam);
					n++;
				}
			}
			if(fscp.showFldList==YSTRUE)
			{
				int n;
				const char *nam;
				n=0;
				while(world->GetFieldTemplateName(n)!=NULL)
				{
					n++;
				}
				printf("NFLD:%d\n",n);
				n=0;
				while((nam=world->GetFieldTemplateName(n))!=NULL)
				{
					printf("FLD:%s\n",nam);
					n++;
				}
			}
			if(fscp.showStpList==YSTRUE)
			{
				int n;
				char nam[256];
				n=0;
				while(world->GetFieldStartPositionName(nam,fscp.fldName,n)==YSOK)
				{
					n++;
				}
				printf("NSTP:%d\n",n);
				n=0;
				while((world->GetFieldStartPositionName(nam,fscp.fldName,n))==YSOK)
				{
					printf("STP:%s\n",nam);
					n++;
				}
			}



			if(FsIsConsoleServer()==YSTRUE && fscp.executionMode!=1 && fscp.executionMode!=0)
			{
				printf("Unavailable option.\n");
				exit(1);
			}



			if(fscp.executionMode==1)
			{
				fsRunLoop.StartNetServerMode(fscp.netPlayerName,fscp.fldName,fscp.netPort);
			}
			// fsmain -client "username" "hostname"
			else if(fscp.executionMode==2)
			{
				fsRunLoop.StartNetClientMode(fscp.netPlayerName,fscp.netServerName,fscp.netPort);
			}
			else if(fscp.executionMode==3)
			{
				world->TerminateSimulation();
				world->PrepareSimulation();

				YsVec3 vec(0.0,0.0,0.0);
				YsAtt3 att(0.0,0.0,0.0);
				if(world->AddField(NULL,fscp.fldName,vec,att)!=NULL)
				{
					printf("Field:%s\n",fscp.fldName.GetArray());
					FsAirplane *air;
					air=world->AddAirplane(fscp.airName,YSTRUE);
					if(air!=NULL)
					{
						printf("Airplane:%s\n",fscp.airName.GetArray());

						air->iff=FS_IFF0;
						world->SettleAirplane(*air,fscp.startPos);

						// This main is for GUI.  It is ok to use mainCanvas->Sim_Fly.
						auto mainCanvas=dynamic_cast <FsGuiMainCanvas *>(fsRunLoop.GetCanvas());
						if(nullptr!=mainCanvas)
						{
							mainCanvas->Sim_Fly(nullptr);
						}
					}
				}
			}
			else if(fscp.executionMode==4)
			{
				if(world->Load(fscp.yfsFilename)==YSOK && world->PlayerPlaneIsReady()==YSTRUE)
				{
					// This main is for GUI.  It is ok to use mainCanvas->Sim_Fly.
					auto mainCanvas=dynamic_cast <FsGuiMainCanvas *>(fsRunLoop.GetCanvas());
					if(nullptr!=mainCanvas)
					{
						mainCanvas->Sim_Fly(nullptr);
					}
					else
					{
						fsRunLoop.TakeOff();
					}
				}
			}
			else if(fscp.executionMode==5)
			{
				int aam,rkt;
				// YSBOOL smk;
				const FsAirplaneTemplate *tmpl;
				if((tmpl=world->GetAirplaneTemplate(fscp.airName))!=NULL)
				{
					YsArray <int,64> weaponConfig;

					aam=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_AIM9);
					rkt=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_ROCKET);
					// smk=(tmpl->GetProperty()->GetSmokeOil()>YsTolerance ? YSTRUE : YSFALSE);

					FsAirplaneProperty::MakeUpWeaponConfigForOldVersion(weaponConfig,aam,0,rkt,0);

					if(YSOK==fsRunLoop.SetUpEnduranceMode(
					    fscp.airName,
					    YSTRUE, // Jet
					    YSFALSE,// Ww2
					    fscp.fldName,
					    fscp.endModeNumWingman,
					    fscp.endModeWingmanLevel,
					    fscp.endModeAllowAAM,
					    weaponConfig.GetN(),weaponConfig,75))
					{
						auto mainCanvas=dynamic_cast <FsGuiMainCanvas *>(fsRunLoop.GetCanvas());
						if(nullptr!=mainCanvas)
						{
							mainCanvas->Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
						}
						else
						{
							fsRunLoop.TakeOff(FsRunLoop::YSRUNMODE_FLY_REGULAR);
						}
					}
				}
			}
			else if(fscp.executionMode==6)
			{
				YsArray <int,64> weaponConfig;
				int aam,rkt;
				const FsAirplaneTemplate *tmpl;
				if((tmpl=world->GetAirplaneTemplate(fscp.airName))!=NULL)
				{
					FsInterceptMissionInfo info;
					info=fscp.interceptMissionInfo;

					info.playerAirInfo.typeName.Set(fscp.airName);
					info.playerAirInfo.fuel=75;

					info.fieldName.Set(fscp.fldName);

					aam=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_AIM9);
					rkt=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_ROCKET);
					FsAirplaneProperty::MakeUpWeaponConfigForOldVersion(info.playerAirInfo.weaponConfig,aam,0,rkt,0);
					if(YSOK==fsRunLoop.SetUpInterceptMission(info))
					{
						auto mainCanvas=dynamic_cast <FsGuiMainCanvas *>(fsRunLoop.GetCanvas());
						if(nullptr!=mainCanvas)
						{
							mainCanvas->Sim_Fly_StartTakeOffSequence(FsRunLoop::YSRUNMODE_FLY_REGULAR);
						}
						else
						{
							fsRunLoop.TakeOff(FsRunLoop::YSRUNMODE_FLY_REGULAR);
						}
					}
				}
			}
			else if(fscp.executionMode==100)
			{
				if(world->Load(fscp.yfsFilename)==YSOK && world->PlayerPlaneIsReady()!=YSTRUE)
				{
					fsRunLoop.StartReplayRecord(YSFALSE);
				}
			}
			else if(FsCommandParameter::EXEMODE_OPENINGDEMOFOREVER==fscp.executionMode)
			{
				fsRunLoop.StartOpeningDemo();
				fsRunLoop.SetAutoDemoForever(YSTRUE);
			}
			else // executionMode=0 i.e., no option is given
			{
				if(fscp.autoExit!=YSTRUE && YSTRUE==opt.openingDemo)
				{
					fsRunLoop.StartOpeningDemo();
				}
			}

			fsRunLoop.SetAutoExit(fscp.autoExit);

			if(nullptr!=canvasPtr && nullptr!=canvasPtr->GetModalDialog())
			{
				fsRunLoop.PushRunMode();  // Run mode will be popped after SupportYsflighDialog
				fsRunLoop.ChangeRunMode(FsRunLoop::YSRUNMODE_MENU);
			}
		}
		break;
	}
	++initializationCounter;
	return YSFALSE;
}

/* virtual */ void FsLazyWindowApplication::ContextLostAndRecreated(void)
{
	FsReinitializeOpenGL();

	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();
	bufMan.ContextLost();

	auto &commonTex=FsCommonTexture::GetCommonTexture();
	commonTex.GetTextureManager().ContextLost();
}



static FsLazyWindowApplication *appPtr=nullptr;

/* static */ FsLazyWindowApplicationBase *FsLazyWindowApplicationBase::GetApplication(void)
{
	if(nullptr==appPtr)
	{
		appPtr=new FsLazyWindowApplication;
	}
	return appPtr;
}




////////////////////////////////////////////////////////////


// void FsConcordeFlybyDemonstration(int concordeFlybyType)
// {
// 	YSBOOL terminateByUser;
// 
// 
// 	world->TerminateSimulation();
// 	world->PrepareSimulation();
// 
// 
// 	world->AddField(NULL,"HEATHROW",YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0),YSTRUE,YSFALSE);
// 	world->DisableGroundFire();
// 
// 
// 	double bankLimitOverride;
// 	bankLimitOverride=YsPi/2.0;
// 	// if(formationLanding==YSTRUE)
// 	{
// 		bankLimitOverride=YsPi/6.0;
// 	}
// 
// 
// 	FsDemoModeInfo info(FSDEMO_CONCORDEFLYBY);
// 	if(world->PrepareConcordeFlyby(info,concordeFlybyType)==YSOK)
// 	{
// 
// 		world->RunDemoMode(info,terminateByUser,"Press key to exit demonstration.",0.0,YSTRUE,YSTRUE);
// 
// 		world->PrepareReplaySimulation();        // To check flight record
// 
// //		world->TerminateSimulation();            // To clean up memory
// //		world->UnprepareAllTemplate();           // To clean up memory
// //		YsShell::CleanUpVertexPolygonStore();    // To clean up memory
// 
// 	}
// 	else
// 	{
// 		printf("Cannot settle airplane for landing demo!\n");
// 	}
// }

// void FsCrackDownMemoryLeak(void)
// {
// 	printf("CrackDownMemoryLeak\n");
// 	// printf("ShellVertexStore\n");
// 	// YsShell::vertexStore.SelfDiagnostic();
// 	// printf("ShellPolygonStore\n");
// 	// YsShell::polygonStore.SelfDiagnostic();
// 
// 	int i;
// 	world->TerminateSimulation();
// 	for(i=0; i<100; i++)
// 	{
// 		printf("$$$$%d$$$$\n",i);
// 		world->PrepareSimulation();
// 		printf("-----------------START ADDFIELD\n");
// 		world->AddField(NULL,"HAWAII",YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0),YSTRUE);
// 		printf("-----------------END ADDFIELD\n");
// 		world->TerminateSimulation();
// 
// 		// printf("ShellVertexStore\n");
// 		// YsShell::vertexStore.SelfDiagnostic();
// 		// printf("ShellPolygonStore\n");
// 		// YsShell::polygonStore.SelfDiagnostic();
// 	}
// }

void FsHelpGeneralHelpEnglish(void)
{
#ifdef WIN32
	YsOpenURL("document\\english.html");
#else
	YsOpenURL("document/english.html");
#endif
}

void FsHelpControlEnglish(void)
{
#ifdef WIN32
	YsOpenURL("document\\control.html");
#else
	YsOpenURL("document/control.html");
#endif
}

void FsHelpGeneralHelpJapanese(void)
{
#ifdef WIN32
	YsOpenURL("document\\japanese.html");
#else
	YsOpenURL("document/japanese.html");
#endif
}

void FsHelpControlJapanese(void)
{
#ifdef WIN32
	YsOpenURL("document\\controlj.html");
#else
	YsOpenURL("document/controlj.html");
#endif
}

