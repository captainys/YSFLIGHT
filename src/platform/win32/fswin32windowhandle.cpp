#include "fswin32windowhandle.h"

#include <ysclass.h>
#include <windows.h>

class FsWin32ApplicationWindowFinder
{
public:
	/*! This function finds a handle of the largest window that is visible and is associated with the current process Id.
	    This window may not be an application main window.  But, what else can I do?
	    This function can be used for giving a window handle to a badly-designed APIs such as Direct Sound API.
	*/
	HWND Find(const YsWString &title);
private:
	YsWString titleStr;
	HWND SearchTopLevelWindow(HWND hWnd,DWORD procId);
};
HWND FsWin32ApplicationWindowFinder::Find(const YsWString &title)
{
	titleStr=title;

	return SearchTopLevelWindow(NULL,GetCurrentProcessId());
}
HWND FsWin32ApplicationWindowFinder::SearchTopLevelWindow(HWND hWnd,DWORD procId)
{
	DWORD windowProcId;
	GetWindowThreadProcessId(hWnd,&windowProcId);
	if(windowProcId==procId)
	{
		if(TRUE==IsWindowVisible(hWnd))
		{
			wchar_t str[256];
			GetWindowTextW(hWnd,str,255);

			YsWString wstr(str);
			if(YSTRUE==wstr.FindWord(NULL,titleStr))
			{
				// printf("Window Title=%ls\n",str);
				// printf("Match %ls\n",titleStr.Txt());
				return hWnd;
			}
		}
	}

	HWND hWndChild=NULL;
	while(NULL!=(hWndChild=FindWindowEx(hWnd,hWndChild,NULL,NULL))!=NULL)
	{
		auto found=SearchTopLevelWindow(hWndChild,procId);
		if(NULL!=found)
		{
			return found;
		}
	}

	return NULL;
}

HWND FsWin32GetMainWindowHandle(void)
{
	FsWin32ApplicationWindowFinder finder;
	auto hWnd=finder.Find(L"Main Window");
	return hWnd;
}
