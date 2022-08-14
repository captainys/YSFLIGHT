#include <ysclass.h>
#include <ysport.h>

#include <fsdef.h>

#include "fsfilename.h"


YSBOOL fsUseLanguageBmp=YSTRUE;

extern const wchar_t *FsProgramName;  // Different names for screen saver programs
                                      //   Declared in fsmain.cpp for YSFLIGHT


static YsWString FsCommonYsFlightDir;
static YsWString FsUserYsflightDir;
static YsWString FsYsflightConfigDir;





void FsUseLocalFolderSetting(void)
{
	FsCommonYsFlightDir.Set(L".");
	FsUserYsflightDir.Set(L".");
}

static YSRESULT YsFileWriteTest(const wchar_t wpath[])
{
	const wchar_t *FsTestFile=L"ystestfile.ystst";
	YsWString ful;
	ful.MakeFullPathName(wpath,FsTestFile);

	FILE *fp=YsFileIO::Fopen(ful,"w");
	if(NULL!=fp)
	{
		fclose(fp);
		YsFileIO::Remove(ful);
		return YSOK;
	}
	return YSERR;
}

const wchar_t *FsGetCommonYsflightDir(void)
{
	if(FsCommonYsFlightDir.Strlen()==0)
	{
		int i;
		for(i=0; i<3; i++)
		{
			YsWString path,ful;

			if(0==i)  // First try COMMON_APPDATA
			{
				if(YSOK!=YsSpecialPath::GetAllUsersDir(path))
				{
					continue;
				}
			}
			else if(1==i)
			{
				if(YSOK!=YsSpecialPath::GetProgramBaseDir(path))
				{
					continue;
				}
			}
			else  // Then try PERSONAL
			{
				if(YSOK!=YsSpecialPath::GetUserDocDir(path))
				{
					continue;
				}
			}

			if(0==i || 2==i)
			{
				ful.MakeFullPathName(path,L"YSFLIGHT.COM");
				YsFileIO::MkDir(path);
				path=ful;

				ful.MakeFullPathName(path,FsProgramName);
				YsFileIO::MkDir(path);
				path=ful;
			}

			if(YSOK==YsFileWriteTest(path))
			{
				FsCommonYsFlightDir=ful;
				YsString cStr;
				YsUnicodeToSystemEncoding(cStr,ful);
				printf("Common YSFLIGHT Dir=%s\n",cStr.Txt());
				break;
			}
		}
	}

	if(FsCommonYsFlightDir.Strlen()==0)
	{
		FsCommonYsFlightDir.Set(L".");

		YsString cStr;
		YsUnicodeToSystemEncoding(cStr,FsCommonYsFlightDir);
		printf("Common YSFLIGHT Dir=%s\n",cStr.Txt());

	}
	return FsCommonYsFlightDir;
}

const wchar_t *FsGetUserYsflightDir(void)
{
	if(FsUserYsflightDir.Strlen()==0)
	{
		YsWString path;
		if(YSOK==YsSpecialPath::GetUserDocDir(path))
		{
			YsWString ful;
			ful.MakeFullPathName(path,L"YSFLIGHT.COM");
			YsFileIO::MkDir(ful);
			path=ful;

			ful.MakeFullPathName(path,FsProgramName);
			YsFileIO::MkDir(ful);
			path=ful;


			YsWString cfg;
			cfg.MakeFullPathName(path,L"config");
			YsFileIO::MkDir(cfg);

			if(YSOK==YsFileWriteTest(cfg))
			{
				FsUserYsflightDir=ful;

				YsString cStr;
				YsUnicodeToSystemEncoding(cStr,FsUserYsflightDir);
				printf("User YSFLIGHT Dir=%s\n",cStr.Txt());
			}
		}
	}

	if(FsUserYsflightDir.Strlen()==0)
	{
		FsUserYsflightDir.Set(L".");

		YsString cStr;
		YsUnicodeToSystemEncoding(cStr,FsUserYsflightDir);
		printf("User YSFLIGHT Dir=%s\n",cStr.Txt());
	}
	return FsUserYsflightDir;
}

void FsOverrideUserConfigDir(const wchar_t path[])
{
	FsYsflightConfigDir.Set(path);
}

void FsOverrideUserDir(const wchar_t path[])
{
	FsUserYsflightDir.Set(path);
}

const wchar_t *FsGetUserYsflightConfigDir(void)
{
#ifdef __APPLE__
	// Try User/Library/Application Support/YSFLIGHT.COM/(FsProgramName)/config
	if(FsYsflightConfigDir.Strlen()==0)
	{
		YsWString ful,path;
		YsSpecialPath::GetUserAppDataDir(path);

		YsFileIO::MkDir(path);


		ful.MakeFullPathName(path,L"YSFLIGHT.COM");
		YsFileIO::MkDir(ful);
		path=ful;

		ful.MakeFullPathName(path,L"config");
		YsFileIO::MkDir(ful);
		path=ful;

		if(YSOK==YsFileWriteTest(path))
		{
			FsYsflightConfigDir=path;
		}
	}
#endif

	if(FsYsflightConfigDir.Strlen()==0)
	{
		FsYsflightConfigDir.MakeFullPathName(FsGetUserYsflightDir(),L"config");

		YsString cStr;
		YsUnicodeToSystemEncoding(cStr,FsYsflightConfigDir);
		printf("YSFLIGHT Config Dir=%s\n",cStr.Txt());
	}

	return FsYsflightConfigDir;
}

const wchar_t *FsGetFirstStartFile(YsWString &fn,int i)
{
	const wchar_t *path=NULL;
	switch(i)
	{
	case 0:
		path=FsGetCommonYsflightDir();
		break;
	case 1:
		path=FsGetUserYsflightConfigDir();
		break;
	}

	if(NULL!=path)
	{
		YsString str;
		str.Printf("%d",YSFLIGHT_VERSION);

		YsWString wStr;
		wStr.SetUTF8String(str);

		fn.MakeFullPathName(path,wStr);
		return fn;
	}
	return NULL;
}

const wchar_t *FsGetVoteYsflightFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/pleaseVote.txt");
	}
	return fn;
}

const wchar_t *FsGetConfigFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/flight.cfg");
	}
	return fn;
}

const wchar_t *FsGetOptionFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/option.cfg");
	}
	return fn;
}

const wchar_t *FsGetNetConfigFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/network.cfg");
	}
	return fn;
}

const wchar_t *FsGetPlugInDir(void)
{
#ifdef _WIN32
	return L".\\plugin";
#else
	return L"./plugin";
#endif
}

const wchar_t *FsGetSoundDllFile(void)
{
	static YsWString fn;

#ifdef _WIN32
	fn.Set(FsGetPlugInDir());
	if(4==sizeof(void *))
	{
		fn.Append(L"\\sndYsflight32.dll");
	}
	else if(8==sizeof(void *))
	{
		fn.Append(L"\\sndYsflight64.dll");
	}
	else
	{
		fn.Append(L"\\sndYsflight.dll");
	}
#elif defined(__APPLE__)
	fn.Set(FsGetPlugInDir());
	if(4==sizeof(void *))
	{
		fn.Append(L"/libsndYsflight32.dylib");
	}
	else if(8==sizeof(void *))
	{
		fn.Append(L"/libsndYsflight64.dylib");
	}
	else
	{
		fn.Append(L"/libsndYsflight.dylib");
	}
#else
	fn.Set(FsGetPlugInDir());
	if(4==sizeof(void *))
	{
		fn.Append(L"/libsndYsflight32.so");
	}
	else if(8==sizeof(void *))
	{
		fn.Append(L"/libsndYsflight64.so");
	}
	else
	{
		fn.Append(L"/libsndYsflight.so");
	}
#endif

	return fn.Txt();
}

const wchar_t *FsGetVoiceDllFile(void)
{
	static YsWString fn;

#ifdef _WIN32
	fn.Set(FsGetPlugInDir());
	if(4==sizeof(void *))
	{
		fn.Append(L"\\voiceYsflight32.dll");
	}
	else if(8==sizeof(void *))
	{
		fn.Append(L"\\voiceYsflight64.dll");
	}
	else
	{
		fn.Append(L"\\voiceYsflight.dll");
	}
#elif defined(__APPLE__)
	fn.Set(FsGetPlugInDir());
	if(4==sizeof(void *))
	{
		fn.Append(L"/libvoiceYsflight32.dylib");
	}
	else if(8==sizeof(void *))
	{
		fn.Append(L"/libvoiceYsflight64.dylib");
	}
	else
	{
		fn.Append(L"/libvoiceYsflight.dylib");
	}
#else
	fn.Set(FsGetPlugInDir());
	if(4==sizeof(void *))
	{
		fn.Append(L"/libvoiceYsflight32.so");
	}
	else if(8==sizeof(void *))
	{
		fn.Append(L"/libvoiceYsflight64.so");
	}
	else
	{
		fn.Append(L"/libvoiceYsflight.so");
	}
#endif

	return fn.Txt();
}

const wchar_t *FsGetNetInterceptMissionConfigFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/netinterceptmission.cfg");
	}
	return fn;
}

const wchar_t *FsGetNetWelcomeMessageFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/netwelcomemsg.txt");
	}
	return fn;
}

const wchar_t *FsGetFontConfigFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/font.cfg");
	}
	return fn;
}

const wchar_t *FsGetControlAssignFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/ctlassign.cfg");
	}
	return fn;
}

const wchar_t *FsGetWindowSizeFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/prevwindowsize.cfg");
	}
	return fn;
}

const wchar_t *FsGetNetServerAddressHistoryFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightConfigDir());
		fn.Append(L"/serverhistory.txt");
	}
	return fn;
}

const wchar_t *FsGetNetChatLogDir(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightDir());
		fn.Append(L"/netchatlog");
	}
	return fn;
}

const wchar_t *FsGetIpBlockFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightDir());
		fn.Append(L"/ipblock.txt");
	}
	return fn;
}

const wchar_t *FsGetRecentlyUsedFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.MakeFullPathName(FsGetUserYsflightDir(),L"recent.txt");
	}
	return fn;
}

const wchar_t *FsGetErrFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightDir());
		fn.Append(L"/fserr.txt");
	}
	return fn;
}

const wchar_t *FsGetPrevFlightFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightDir());
		fn.Append(L"/prevflight.dat");
	}
	return fn;
}

const wchar_t *FsGetDebugInfoFile(void)
{
	static YsWString fn;
	if(fn.Strlen()==0)
	{
		fn.Set(FsGetUserYsflightDir());
		fn.Append(L"/fsdebuginfo.txt");
	}
	return fn;
}

const wchar_t *FsGetSharewareRegistFileName(YsWString &fn,int i)
{
	const wchar_t *path=NULL;
	switch(i)
	{
	default:
	case 0:
		path=FsGetUserYsflightDir();
		break;
	case 1:
		path=FsGetCommonYsflightDir();
		break;
	case 2:
		path=FsGetUserYsflightConfigDir();
		break;
	}

	if(NULL!=path)
	{
		fn.MakeFullPathName(path,L"regist");
		return fn.Txt();
	}
	return NULL;
}
