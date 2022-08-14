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
		firstStart=YSFALSE;
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
		}

		if(YSTRUE==firstStart)
		{
			auto firstLaunchDialog=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiFirstDialogClass>();
			firstLaunchDialog->Initialize();
			firstLaunchDialog->Make(0);
			firstLaunchDialog->BindCloseModalCallBack(&FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed,mainCanvasPtr);
			canvasPtr->AppendModalDialog(firstLaunchDialog);

			auto supportYsflightDlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiSupportYsflightDialogClass>();
			supportYsflightDlg->Initialize();
			supportYsflightDlg->Make(firstStart,0);
			supportYsflightDlg->BindCloseModalCallBack(&FsGuiMainCanvas::PopRunModeWhenLastModalDialogIsClosed,mainCanvasPtr);
			canvasPtr->AppendModalDialog(supportYsflightDlg);
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


			if(FsIsConsoleServer()==YSTRUE && fscp.executionMode!=1 && fscp.executionMode!=0)
			{
				printf("Unavailable option.\n");
				exit(1);
			}



			fsRunLoop.StartOpeningDemo();
			fsRunLoop.SetAutoDemoForever(YSTRUE);
			fsRunLoop.StartOpeningDemo();

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

