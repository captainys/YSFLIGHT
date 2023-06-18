#ifndef FSFILENAME_IS_INCLUDED
#define FSFILENAME_IS_INCLUDED
/* { */

void FsUseLocalFolderSetting(void);

const wchar_t *FsGetCommonYsflightDir(void);
const wchar_t *FsGetUserYsflightDir(void);
// Windows:  My Documents/YSFLIGHT.COM/(ProgramName)
// Mac OSX:  Documents/YSFLIGHT.COM/(ProgramName)
// Linux:    Documents/YSFLIGHT.COM/(ProgramName)

const wchar_t *FsGetFirstStartFile(YsWString &fn,int i);
const wchar_t *FsGetVoteYsflightFile(void);
void FsOverrideUserConfigDir(const wchar_t path[]);
void FsOverrideUserDir(const wchar_t path[]);
const wchar_t *FsGetConfigFile(void);
const wchar_t *FsGetOptionFile(void);
const wchar_t *FsGetNetConfigFile(void);
const wchar_t *FsGetNetInterceptMissionConfigFile(void);
const wchar_t *FsGetNetWelcomeMessageFile(void);
const wchar_t *FsGetFontConfigFile(void);
const wchar_t *FsGetControlAssignFile(void);
const wchar_t *FsGetWindowSizeFile(void);
const wchar_t *FsGetNetServerAddressHistoryFile(void);
const wchar_t *FsGetNetChatLogDir(void);
const wchar_t *FsGetPlugInDir(void);
const wchar_t *FsGetSoundDllFile(void);
const wchar_t *FsGetVoiceDllFile(void);
const wchar_t *FsGetIpBlockFile(void);
const wchar_t *FsGetRecentlyUsedFile(void);

const wchar_t *FsGetPrevFlightFile(void);
const wchar_t *FsGetErrFile(void);
const wchar_t *FsGetDebugInfoFile(void);

const wchar_t *FsGetSharewareRegistFileName(YsWString &fn,int i);

#define FSGUI_ABOUTDLG_BANNER                             "misc/ysfhq_main.png"



/* } */
#endif
