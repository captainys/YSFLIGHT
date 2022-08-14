#ifndef FSWINDOW_IS_INCLUDED
#define FSWINDOW_IS_INCLUDED
/* { */

#include <fssimplewindow.h>

#include "fsdef.h"

#ifndef FS_MOUSEEVENT_TYPE_IS_DEFINED
#define FS_MOUSEEVENT_TYPE_IS_DEFINED
enum
{
	FSMOUSEEVENT_NONE,
	FSMOUSEEVENT_LBUTTONDOWN,
	FSMOUSEEVENT_LBUTTONUP,
	FSMOUSEEVENT_MBUTTONDOWN,
	FSMOUSEEVENT_MBUTTONUP,
	FSMOUSEEVENT_RBUTTONDOWN,
	FSMOUSEEVENT_RBUTTONUP,
	FSMOUSEEVENT_MOVE
};
#endif



#ifdef __cplusplus
// FSMOUSEEVENT enum needs to be included from Objective-C source.

void FsRegisterIntervalCallBack(void (*callback)(void *),void *param);
class FsOpenWindowOption FsGetOpenWindowOption(const class FsOption &opt,const class FsFlightConfig &cfg,const wchar_t prevWindowSizeFile[],const char windowTitle[]);
void FsCloseWindow(void);

void FsSetTopMostWindow(YSBOOL isTopMost);

YSRESULT FsTaskBarAddIcon(void);
YSRESULT FsTaskBarDeleteIcon(void);



// Written per platform >>
void FsAfterOpenWindow(const class FsOption &opt,const class FsFlightConfig &cfg);
void FsBeforeOpenWindow(const class FsOption &opt,const class FsFlightConfig &cfg);
// Written per platform <<



// const int FsMaxNumSibWindow=4;
YSRESULT FsOpenSubWindow(int subWindowId);        // Child window of the main window
YSBOOL FsIsMainWindowActive(void);
YSBOOL FsIsSubWindowActive(int subWndId);
YSBOOL FsIsSubWindowOpen(int subWindowId);
// YSRESULT FsOpenSiblingWindow(int sibWindowId);  // Overlapped window
YSRESULT FsCloseSubWindow(int subWindowId);
// YSRESULT FsCloseSiblingWindow(int sibWindowId);
YSRESULT FsSelectMainWindow(void);
YSRESULT FsSelectSubWindow(int subWindowId);
// YSRESULT FsSelectSiblingWindow(int sibWindowId);
void FsGetDrawingAreaSize(int &wid,int &hei);
void FsGetWindowViewport(int &x0,int &y0,int &wid,int &hei);
YsVec2i FsGetMainWindowDrawingAreaSize(void);
YsVec2i FsGetSubWindowDrawingAreaSize(void);
YsVec2i FsGetDrawingAreaSize(void);

void FsSplitMainWindow(YSBOOL split);
void FsSetActiveSplitWindow(int id);
YSBOOL FsIsMainWindowSplit(void);
int FsGetActiveSplitWindow(void);


class FsWindowLayout
{
public:
	class WindowPosition
	{
	public:
		YSBOOL set;
		int x0,y0,wid,hei;

		void Initialize(void);
	};

	WindowPosition mainWindow;

	YSRESULT Load(const wchar_t fn[]);
	YSRESULT Save(const wchar_t fn[]) const;
};

// x0,y0,wid,hei are the values given to/from FsSimpleWindow framework.
void FsLoadWindowSize(int &x0,int &y0,int &wid,int &hei,const wchar_t fn[]);


void FsSetFullScreen(int wid,int hei,int bpp);
void FsSetNormalWindow(void);
void FsSaveWindowSize(const wchar_t fn[]);


void FsSetOnPaintCallback(class FsOnPaintCallback *callback);

void FsPollDevice(void);
int FsInkey(void);
int FsInkeyChar(void);
int FsTranslateKeyCodeLocal(int fsKey,YSBOOL shift);
void FsMouse(YSBOOL &lb,YSBOOL &mb,YSBOOL &rb,int &mx,int &my);
int FsGetMouseEvent(YSBOOL &lb,YSBOOL &mb,YSBOOL &rb,int &mx,int &my);
int FsGetKeyState(int fskey);


void FsMessageBox(const char msg[],const char title[]);
YSBOOL FsYesNoDialog(const char msg[],const char title[]);

void FsSleep(int ms);
int FsCheckKeyHeldDown(void);
void FsClearEventQueue(void);

class FsJoystick
{
public:
	double axs[FsMaxNumJoyAxis];   // 2004/06/26 axs[4] -> axs[FsMaxNumJoyAxis]
	YSBOOL trg[FsMaxNumJoyTrig];

	YSBOOL pov;
	double povAngle;
};

YSBOOL FsIsJoystickAxisAvailable(int joyId,int axisId);  // Note: Must return YSTRUE for joyId==FsMouseJoyId && 0<=joyAxs && joyAxs<2
YSRESULT FsPollJoystick(FsJoystick &joy,int joyId);




YSRESULT FsFindFileList(YsArray <YsWString> &filelist,const wchar_t dir[],const wchar_t prefix[],const wchar_t ext[]);
YSRESULT FsFindFileDirList(YsArray <YsWString> &fileList,YsArray <YsWString> &dirList,const wchar_t dir[],const wchar_t ext[]);



class FsOnPaintCallback
{
public:
	virtual void OnPaint(void)
	{
	}
};

#endif /* #ifdef __cplusplus */

/* } */
#endif
