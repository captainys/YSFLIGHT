#define _WIN32_WINDOWS 0x500
#define _WIN32_WINNT 0x500
#define WINVER 0x500



#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"comdlg32.lib")
#pragma comment(lib,"urlmon.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"ddraw.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"wsock32.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")



// The platform dependent functions are declared in fswindow.h


#include <ysclass.h>
#include <ysport.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

//#include <windows.h>
//#include <winuser.h>
//#include <mmsystem.h>

#include <fsfilename.h>

#include <fswindow.h>
#include <fsoption.h>

// #include <resource.h>



#include "fswin32windowhandle.h"



static class FsOnPaintCallback *fsOnPaintCallbackObj=NULL;





void FsWin32PrepareDirectDraw(void);

bool FsWin32CreateGraphicContext(HWND hWnd,HDC hDc);
void FsWin32InitializeGraphicEngine(HWND hWnd,HDC hDc);
void FsWin32UninitializeGraphicEngine(HWND hWnd,HDC hDc);
void FsWin32AfterResize(HWND hWnd,HDC hDc,int winX,int winY);
bool FsWin32SwapBuffers(HWND hWnd,HDC hDc);
void FsWin32HidePartOfScreenForSharewareMessage(void);



////////////////////////////////////////////////////////////
// Below section deals with DirectDraw.

#include <wingdi.h>
//#include <ddraw.h>

// Create an IDirectDraw2 interface.
// static LPDIRECTDRAW lpDD=NULL;   DirectDraw no longer needed because I don't make it specific-resolution full screen.

class FsWin32WindowDcPair
{
public:
	HWND hWnd;
	HDC hDc;

	FsWin32WindowDcPair();
};

FsWin32WindowDcPair::FsWin32WindowDcPair()
{
	hWnd=NULL;
	hDc=NULL;
}

static FsWin32WindowDcPair hMainWnd;


void FsWin32PrepareDirectDraw(void)
{
//	lpDD=NULL;
//
//	HRESULT ddrval;
//	ddrval=DirectDrawCreate(NULL, &lpDD, NULL);
//	if(ddrval!=DD_OK)
//	{
//		printf("DD1\n");
//		goto ERRTRAP;
//	}
//
//	return;
//ERRTRAP:
//	lpDD=NULL;
}


// Changing the style of the main window
// http://support.microsoft.com/default.aspx?scid=kb;en-us;111011
static void FsEnableWindowFrame(HWND hWnd)
{
	DWORD style;
	style=GetWindowLong(hWnd,GWL_STYLE);
	style=(style|(WS_THICKFRAME|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX));
	SetWindowLong(hWnd,GWL_STYLE,style);
	DrawMenuBar(hWnd);
}

static void FsDisableWindowFrame(HWND hWnd)
{
	DWORD style;
	style=GetWindowLong(hWnd,GWL_STYLE);
	style=(style&~(WS_THICKFRAME|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX));
	SetWindowLong(hWnd,GWL_STYLE,style);
	DrawMenuBar(hWnd);
}

//typedef struct
//{
//	YSBOOL valid;
//	unsigned wid,hei,bpp;
//} FSQUERYDISPLAYMODE;

//HRESULT WINAPI FsDdrawEnumCallback(LPDDSURFACEDESC surfDesc,LPVOID lpContext)
//{
//	FSQUERYDISPLAYMODE *query;
//
//	query=(FSQUERYDISPLAYMODE *)lpContext;
//
//	printf("Looking for the resolution....\n");
//	printf("  %d %d %d\n",
//	   surfDesc->dwHeight,
//	   surfDesc->dwWidth,
//	   surfDesc->ddpfPixelFormat.dwRGBBitCount);
//
//	if(surfDesc->dwHeight==query->hei &&
//	   surfDesc->dwWidth==query->wid &&
//	   surfDesc->ddpfPixelFormat.dwRGBBitCount==query->bpp)
//	{
//		query->valid=YSTRUE;
//		printf("Found it. Let's Stop Here.\n");
//		return DDENUMRET_CANCEL;  // I found it. Let's stop here.
//	}
//	return DDENUMRET_OK;
//}

// Above section deals with DirectDraw
////////////////////////////////////////////////////////////

// Taskbar

const int TaskBarIconId=4000;
const int TaskBarMessageId=20001;
static YSBOOL FsTaskBarReady=YSFALSE;

YSRESULT FsTaskBarAddIcon(void)
{
	auto hWndMain=FsWin32GetMainWindowHandle();
	if(FsTaskBarReady!=YSTRUE)
	{
	    NOTIFYICONDATA tnid;
		HINSTANCE inst;

		inst=(HINSTANCE)GetModuleHandle(NULL);

		ZeroMemory(&tnid,sizeof(tnid));

	    tnid.cbSize=sizeof(NOTIFYICONDATA);
	    tnid.hWnd=hWndMain;
	    tnid.uID=TaskBarIconId;
	    tnid.uFlags=NIF_MESSAGE|NIF_ICON|NIF_TIP;
	    tnid.uCallbackMessage=TaskBarMessageId;
	    tnid.hIcon=LoadIcon(inst,"MAINICON");
	    strcpy(tnid.szTip,"YSFLIGHT");

		if(Shell_NotifyIcon(NIM_ADD, &tnid)==TRUE)
		{
			FsTaskBarReady=YSTRUE;
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsTaskBarDeleteIcon(void)
{
	auto hWndMain=FsWin32GetMainWindowHandle();
	if(FsTaskBarReady==YSTRUE)
	{
	    NOTIFYICONDATA tnid;

		ZeroMemory(&tnid,sizeof(tnid));

	    tnid.cbSize =sizeof(NOTIFYICONDATA);
	    tnid.hWnd=hWndMain;
	    tnid.uID=TaskBarIconId;

		FsTaskBarReady=YSFALSE;

	    if(Shell_NotifyIcon(NIM_DELETE, &tnid)==TRUE)
	    {
			return YSOK;
		}
	}
	return YSERR;
}

void FsTaskBarNotification(HWND /*hWnd*/,WPARAM wp, LPARAM lp)
{
	auto hWndMain=FsWin32GetMainWindowHandle();
	UINT uID;
	UINT uMouseMsg;

	uID=(UINT)wp;
	uMouseMsg=(UINT)lp;

	if(uMouseMsg==WM_LBUTTONDOWN && uID==TaskBarIconId)
	{
		ShowWindow(hWndMain,SW_SHOWNORMAL);
	}
}

/*
  in Window Call Back,
	switch(msg)
	{
	case TaskBarMessageId:
		FsTaskBarNotification(hWnd,w,l);
		return 0L;

*/
////////////////////////////////////////////////////////////

bool FsCloseWindowCallBack(void *)
{
	FsOption opt;
	if(opt.Load(FsGetOptionFile())==YSOK && opt.scrnMode==0 && opt.rememberWindowSize==YSTRUE)
	{
		FsSaveWindowSize(FsGetWindowSizeFile());
	}

	auto hWndMain=FsWin32GetMainWindowHandle();
	auto hDcMain=GetDC(hWndMain);
	FsWin32UninitializeGraphicEngine(hWndMain,hDcMain);
	FsTaskBarDeleteIcon();
	return  true;  // true -> Window will be closed as a default behavior.
}

bool FsOpenGLContextCreationCallBack(void *)
{
	/*
	In YSFLIGHT on Direct 3D, graphics context creation is postponed until the window creation is
	fully completed.  However, OpenGL context must not be created.  Therefore, FsWin32CreateGraphicContext must
	retrn true.

	In YSFLIGHT on OpenGL 1.x and 2.0, it must return false so that FsSimpleWindow framework creates a context. 
	*/
	auto hWndMain=FsWin32GetMainWindowHandle();
	auto hDcMain=GetDC(hWndMain);

	// hWndMain may be NULL at this point.

	return FsWin32CreateGraphicContext(hWndMain,hDcMain);
	// Return true for D3D
	// Return false for OpenGL and let FsSimpleWindow framework create OpenGL context
}

bool FsSwapBuffersHook(void *)
{
	auto hWndMain=FsWin32GetMainWindowHandle();
	auto hDcMain=GetDC(hWndMain);
	return FsWin32SwapBuffers(hWndMain,hDcMain);

	// Return true for D3D
	// Return false for OpenGL and let FsSimpleWindow framework swap buffers.
}

void FsOnPaintCallback(void *)
{
	if(NULL!=fsOnPaintCallbackObj)
	{
		fsOnPaintCallbackObj->OnPaint();
	}
}

void FsOnResizeCallBack(void *,int wid,int hei)
{
	auto hWndMain=FsWin32GetMainWindowHandle();
	auto hDcMain=GetDC(hWndMain);
	FsWin32AfterResize(hWndMain,hDcMain,wid,hei);
}

////////////////////////////////////////////////////////////


void FsBeforeOpenWindow(const class FsOption &opt,const class FsFlightConfig &)
{
	FsRegisterBeforeOpenGLContextCreationCallBack(FsOpenGLContextCreationCallBack,NULL);
	FsRegisterCloseWindowCallBack(FsCloseWindowCallBack,NULL);
	FsRegisterSwapBuffersCallBack(FsSwapBuffersHook,NULL);
	FsRegisterOnPaintCallBack(FsOnPaintCallback,NULL);
	FsRegisterWindowResizeCallBack(FsOnResizeCallBack,NULL);
}
void FsAfterOpenWindow(const class FsOption &opt,const class FsFlightConfig &)
{
	if(1==opt.scrnMode)
	{
		FsMaximizeWindow();
	}
	else if(1<opt.scrnMode)
	{
		FsSetFullScreen(0,0,0);
	}

	hMainWnd.hWnd=FsWin32GetMainWindowHandle();
	hMainWnd.hDc=GetDC(hMainWnd.hWnd);

	FsWin32InitializeGraphicEngine(hMainWnd.hWnd,hMainWnd.hDc);  // <- This function is written in fswin32gl.cpp/fswin32bi.cpp
	// FsWin32PrepareDirectDraw();
}



YSRESULT FsWin32OpenWindowExternal(HWND hWnd)  // For screen saver
{
	hMainWnd.hWnd=hWnd;
	hMainWnd.hDc=GetDC(hWnd);
	FsWin32InitializeGraphicEngine(hMainWnd.hWnd,hMainWnd.hDc);  // <- This function is written in fswin32gl.cpp/fswin32bi.cpp
	return YSOK;
}

YSBOOL FsWin32IsWindowActive(void)
{
	auto hWndMain=hMainWnd.hWnd;
	HWND fgw=GetForegroundWindow();
	if(fgw!=NULL && hWndMain==fgw)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void FsSetTopMostWindow(YSBOOL isTopMost)
{
	auto hWndMain=hMainWnd.hWnd;
	if(hWndMain!=NULL)
	{
		if(isTopMost==YSTRUE)
		{
			SetWindowPos(hWndMain,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
		else
		{
			SetWindowPos(hWndMain,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		}
	}
}

void FsSetFullScreen(int /*wid*/,int /*hei*/,int /*bpp*/)
{
	//HRESULT ddrval;
	//char *msg;

	// if(lpDD!=NULL)
	{
		auto hWndMain=hMainWnd.hWnd;
		auto hDcMain=hMainWnd.hDc;

		//ddrval=lpDD->SetCooperativeLevel(hWndMain,DDSCL_EXCLUSIVE|DDSCL_ALLOWMODEX|DDSCL_FULLSCREEN);
		//if(ddrval!=DD_OK)
		//{
		//	msg="Cannot Take EXCLUSIVE MODE of DirectDraw";
		//	goto ERREND;
		//}

		FsDisableWindowFrame(hWndMain);
		ShowWindow(hWndMain,SW_MAXIMIZE);

		int wid,hei;
		FsGetWindowSize(wid,hei);
		FsWin32AfterResize(hWndMain,hDcMain,wid,hei);
	}
//	return;
//ERREND:
//	FsSetNormalWindow();
//
//	fsStderr.Printf("%s\n",msg);
}

void FsSetNormalWindow(void)
{
	//if(lpDD!=NULL)
	{
		auto hWndMain=hMainWnd.hWnd;
		auto hDcMain=hMainWnd.hDc;

		//lpDD->SetCooperativeLevel(hWndMain,DDSCL_NORMAL);
		//lpDD->RestoreDisplayMode();

		FsEnableWindowFrame(hWndMain);
		ShowWindow(hWndMain,SW_RESTORE);

		int wid,hei;
		FsGetWindowSize(wid,hei);
		FsWin32AfterResize(hWndMain,hDcMain,wid,hei);
	}
}

void FsSetOnPaintCallback(class FsOnPaintCallback *callback)
{
	fsOnPaintCallbackObj=callback;
}


void FsHidePartOfScreenForSharewareMessage(void)
{
	FsWin32HidePartOfScreenForSharewareMessage();
}

void FsMessageBox(const char msg[],const char title[])
{
	auto hWndMain=hMainWnd.hWnd;
	MessageBox(hWndMain,msg,title,MB_OK);
}

YSBOOL FsYesNoDialog(const char msg[],const char title[])
{
	auto hWndMain=hMainWnd.hWnd;
	const int id=MessageBox(hWndMain,msg,title,MB_YESNO);
	if(id==IDYES)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

