#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#ifdef _WIN32
#include <winsock2.h> // This needs to be included before windows.h (is included somewhere else.)
#endif

#include <ysclass.h>
#include <ysport.h>
#include <fsgui.h>

#include "fsconfig.h"
#include "fsoption.h"
#include "fsapplyoption.h"
#include "fsnetconfig.h"

#include "fs.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"
#include "fsconmenu.h"
#include "fsfilename.h"

#include "fspluginmgr.h"

#include "graphics/common/fsfontrenderer.h"

#include "fstextresource.h"

#include "fsatc.h"

#include "fsrunloop.h"

#ifdef _WIN32
	#include <float.h>
	#include <direct.h>
	#include <shlobj.h>
#else
	#include <unistd.h>
	#include <sys/stat.h>
#endif


#include "fscmdparaminfo.h"



// Linux ld hasn't fixed the bug about the link order for decades. >>
// In reality, ysglcpp library depends on ysglcpp_nownd, ysglcpp_gl1, ysglcpp_gl2, or ysglcpp_d3d9.
// However, the header dependency is opposite, and I need to write like ysglcpp_* depends on ysglcpp in CMakeLists.txt
// That confuses CMake, and spits a wrong link order.
// In the wrong link order, ysglcpp library is linked once, but no symbol was used at that time.
// Then comes ysglcpp_nownd, but ysglcpp_nownd is not used from anywhere, it is not linked.
// Finally, ysglcpp is linked again after ysglcpp_nownd, but ysglcpp_nownd is not linked again after that.
// Therefore, symbols defined in ysglcpp_nownd are not linked.
// To force ysglcpp_nownd linked, I need to make a dummy call from here.
#include <ysglbuffermanager.h>
void ForceLink(void)
{
	auto bufMan=YsGLBufferManager::GetSharedBufferManager();
};
// Linux ld hasn't fixed the bug about the link order for decades. <<




// #define CRASH_ON_ZERO_DIVISION_ERROR


#include <ysbitmap.h>

const wchar_t *FsProgramName=L"YSFLIGHT";  // Different names for screen saver programs

#ifndef YSFS_TESTVERSION
const char *FsProgramTitle="YS FLIGHT SIMULATOR";  // Different names for screen saver programs
#else
const char *FsProgramTitle="YS FLIGHT SIMULATOR - Untested";  // Different names for screen saver programs
#endif

FsWorld *world=NULL;

static char FsWorkingDirectory[512];

static void FsCaptureWorkingDirectory(void)
{
	getcwd(FsWorkingDirectory,511);
	long long int l=strlen(FsWorkingDirectory);
	if(l>0 && (FsWorkingDirectory[l-1]=='/' || FsWorkingDirectory[l-1]=='\\'))
	{
		FsWorkingDirectory[l-1]=0;
	}
}

////////////////////////////////////////////////////////////

void Initialize(void);
void FsMain(void);

////////////////////////////////////////////////////////////



extern FsScreenMessage fsConsole;



int main(int ac,char *av[])
{
	ForceLink();

	printf("YSFLIGHT\n");
	printf("VERSION %d\n",YSFLIGHT_VERSION);
	printf("YFSVERSION %d\n",YSFLIGHT_YFSVERSION);
	printf("NETVERSION %d\n",YSFLIGHT_NETVERSION);

	// printf("**** %d\n",sizeof(****));



#ifdef __APPLE__
	{
		YsWString argv0;
		argv0.SetUTF8String(av[0]);
		auto realArgv0=YsFileIO::GetRealPath(argv0);
		YsWString dir,fil;
		realArgv0.SeparatePathFile(dir,fil);
		YsFileIO::ChDir(dir);
	}
#else
	FsChangeToProgramDir();
#endif



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



	FsCommandParameter fscp;
	if(fscp.RecognizeCommandParameter(ac,av)!=YSOK)
	{
		return 1;
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



	FsOption opt;
	opt.Load(FsGetOptionFile());

	FsFlightConfig cfg;
	cfg.Load(FsGetConfigFile());

	FsCaptureWorkingDirectory();

	FsBeforeOpenWindow(opt,cfg);
	auto owo=FsGetOpenWindowOption(opt,cfg,FsGetWindowSizeFile(),FsMainWindowTitle());
	FsOpenWindow(owo);
	FsAfterOpenWindow(opt,cfg);

	FsSoundInitialize();
	FsClearScreenAndZBuffer(YsBlack());

	FsGuiObject::defAsciiRenderer=&fsAsciiRenderer;
	FsGuiObject::defUnicodeRenderer=&fsUnicodeRenderer;

	FsSetFont(opt.fontName,opt.fontHeight);

	FsLoadPlugIn();
	FsApplyNonScreenOption(opt);

	fsConsole.Printf("Working Directory=%s\n",FsWorkingDirectory);

	srand((unsigned int)time(NULL));

	FsAirplaneAllocator.SetAllocUnit(16);
	FsGroundAllocator.SetAllocUnit(64);



#ifdef YSFS_TESTVERSION
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
#endif



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
	}



	Initialize();



	FsRunLoop fsRunLoop;
	fsRunLoop.ChangeRunMode(FsRunLoop::YSRUNMODE_MENU);

	world=fsRunLoop.GetWorld();
	if(NULL!=world)
	{
		FsPollDevice();
		FsSoundSetMasterSwitch(YSFALSE);

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
			return -1;
		}



		if(fscp.executionMode==1)
		{
			NetworkHostFlightAuto(fscp.netPlayerName,fscp.fldName,fscp.netPort,fscp);
		}
		// fsmain -client "username" "hostname"
		else if(fscp.executionMode==2)
		{
			printf("Client-mode is unavailable in the console-server.\n");
		}
		else if(fscp.executionMode==3)
		{
			printf("Interactive flight is unavailable in the console-server.\n");
		}
		else if(fscp.executionMode==4)
		{
			printf("Interactive flight is unavailable in the console-server.\n");
		}
		else if(fscp.executionMode==5)
		{
			printf("Endurance mode is unavailable in the console-server.\n");
		}
		else if(fscp.executionMode==6)
		{
			printf("Intercept-mission mode is unavailable in the console-server.\n");
		}
		else if(fscp.executionMode==100)
		{
			printf("Replay-recording mode is unavailable in the console-server.\n");
		}
		else // executionMode=0 i.e., no option is given
		{
			FsConMenu(fscp,world);
		}
	}

	FsFreePlugIn();
	FsCloseWindow();

	return 0;
}

////////////////////////////////////////////////////////////

void Initialize(void)
{
	world=NULL;
	FsWeaponHolder::LoadMissilePattern();
}

////////////////////////////////////////////////////////////

// This function is still used from fsconmenu.cpp
void NetworkHostFlightAuto(const char username[],const char fldName[],int netPort,const class FsCommandParameter &fscp)
{
	int nReset=0;
	YSBOOL resetServer=YSFALSE;

	FsNetConfig netcfg;
	netcfg.Load(FsGetNetConfigFile());

	if(fldName!=NULL && fldName[0]==0)
	{
		fldName=NULL;
	}

RESETSERVER:
	world->TerminateSimulation();
	world->PrepareSimulation();

	{
		int maxNumUser=0;
		FsServerRunLoop svrSta(username,fldName,netPort,world->GetSimulation(),&netcfg);
		while(FsServerRunLoop::SERVER_RUNSTATE_TERMINATED!=svrSta.runState)
		{
			world->RunServerModeOneStep(svrSta);

			int nNetUser=world->GetSimulation()->NetServerGetNumConnectedUser();
			YsMakeGreater<int>(maxNumUser,nNetUser);
			if(YSTRUE==fscp.netCloseServerWhenAllLogOut && 0<maxNumUser && 0==nNetUser)
			{
				resetServer=YSFALSE;
				svrSta.resetServer=YSFALSE;
				break;
			}
		}
		switch(svrSta.fatalError)
		{
		case FsServerRunLoop::SERVER_FATAL_NOERROR:
			resetServer=svrSta.resetServer;
			break;
		case FsServerRunLoop::SERVER_FATAL_FIELD_UNAVAILABLE:
			fsStderr.Printf("Cannot load field. Select another field and try again.\n");
			resetServer=YSFALSE;
			break;
		}
	}


	world->PrepareReplaySimulation();

	if(netcfg.recordWhenServerMode!=YSTRUE)
	{
		world->TerminateSimulation();
		world->UnprepareAllTemplate();
		YsShell::CleanUpVertexPolygonStore();
	}

	nReset++;

	if(resetServer==YSTRUE && (netcfg.endSvrAfterResetNTimes==0 || nReset<netcfg.endSvrAfterResetNTimes))
	{
		printf("Resetting Server %d\n",(int)time(NULL));
		goto RESETSERVER;
	}
}
