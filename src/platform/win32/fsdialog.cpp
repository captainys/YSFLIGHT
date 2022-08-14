

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <direct.h>

#include <time.h>

#include <ysclass.h>
#include <ysport.h>

#include <fs.h>
#include <fsfilename.h>
#include <fsopengl.h>
#include <fswindow.h>

#include <windows.h>
#include <float.h>

#include <ysbitmap.h>

#include <fsgui.h>
// #include "../fsguiselectiondialogbase.h"
// #include "../fschoose.h"



#define FS_JWORD_EXE "bundle\\jwv_aff-vector_for_partner.exe"
#define FS_JWORD_COMMAND FS_JWORD_EXE" PA005354 1"


extern void FsClearEventQueue(void);
extern int FsCheckKeyHeldDown(void);


static FsWorld *currentWorld=NULL;

extern HWND FsWin32GetMainWindowHandle(void);

////////////////////////////////////////////////////////////

static void FsInstallJWordPlugin(HWND hDlg)
{
	STARTUPINFO sui;
	sui.cb=sizeof(STARTUPINFO);
	sui.lpReserved=NULL;
	sui.lpDesktop=NULL;
	sui.lpTitle=NULL;
	sui.dwFlags=0;
	sui.cbReserved2=0;
	sui.lpReserved2=0;

	PROCESS_INFORMATION pi;
	BOOL err;

	err=CreateProcess
	   (NULL,
	    FS_JWORD_COMMAND,
	    NULL,
	    NULL,
	    FALSE,
	    CREATE_DEFAULT_ERROR_MODE,
	    NULL,
	    NULL,
	    &sui,
	    &pi);

	MessageBoxA(hDlg,"JWordプラグインをインストールいただき、ありがとうございました。","Thank you!",MB_OK);
}

void FsWin32InstallJWordPlugin(void) // Called from fsmenu.cpp as a cheating function.
{
	FsInstallJWordPlugin(FsWin32GetMainWindowHandle());
}

void FsWin32DeleteJWordPlugin(void)
{
	remove(FS_JWORD_EXE);
}

