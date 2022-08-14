#include <ysclass.h>
#include <ysport.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fsconfig.h"
#include "fsoption.h"
#include "platform/common/fswindow.h"



#include <ysport.h>



FsOpenWindowOption FsGetOpenWindowOption(const class FsOption &opt,const class FsFlightConfig &cfg,const wchar_t prevWindowSizeFile[],const char windowTitle[])
{
	FsOpenWindowOption windowOpt;

	int wid,hei,depth;

	if(opt.scrnMode==0 || YSOK!=opt.GetFullScreenResolution(wid,hei,depth))
	{
		int x0,y0;

		wid=960;
		hei=720;
		x0=32;
		y0=32;

		if(opt.rememberWindowSize==YSTRUE)
		{
			FsLoadWindowSize(x0,y0,wid,hei,prevWindowSizeFile);
		}

		windowOpt.x0=x0;
		windowOpt.y0=y0;
		windowOpt.wid=wid;
		windowOpt.hei=hei;
		windowOpt.useDoubleBuffer=1;
		windowOpt.useMultiSampleBuffer=(int)cfg.useOpenGlAntiAliasing;
		windowOpt.windowTitle=windowTitle;
	}
	else
	{
		windowOpt.x0=0;
		windowOpt.y0=0;
		windowOpt.wid=800;
		windowOpt.hei=600;
		windowOpt.useDoubleBuffer=1;
		windowOpt.useMultiSampleBuffer=(int)cfg.useOpenGlAntiAliasing;
		windowOpt.windowTitle=windowTitle;
		windowOpt.sizeOpt=windowOpt.FULLSCREEN;
	}

	return windowOpt;
}




class FsSubWindowManager
{
public:
	YSBOOL mainWindowIsActive;

	// Split Window >>
	YSBOOL mainWindowSplit;
	int activeSplitWindow;
	// Split Window <<

	YSBOOL subWindowIsActive;
	int activeSubWindowId;

	YSBOOL subWindowIsOpen[FsMaxNumSubWindow];



	FsSubWindowManager();
};


FsSubWindowManager::FsSubWindowManager()
{
	int i;
	for(i=0; i<FsMaxNumSubWindow; i++)
	{
		subWindowIsOpen[i]=YSFALSE;
	}

	mainWindowIsActive=YSTRUE;
	mainWindowSplit=YSFALSE;
	activeSplitWindow=0;

	subWindowIsActive=YSFALSE;
	activeSubWindowId=0;
}

static FsSubWindowManager subWindowManager;

YSRESULT FsOpenSubWindow(int subWndId)
{
	if(YSTRUE!=subWindowManager.subWindowIsOpen[subWndId])
	{
		subWindowManager.subWindowIsOpen[subWndId]=YSTRUE;
		return YSOK;
	}
	return YSERR;
}

YSBOOL FsIsMainWindowActive(void)
{
	return subWindowManager.mainWindowIsActive;
}

YSBOOL FsIsSubWindowActive(int subWndId)
{
	if(0<=subWndId && subWndId<FsMaxNumSubWindow &&
	   YSTRUE==subWindowManager.subWindowIsOpen[subWndId] &&
	   YSTRUE==subWindowManager.subWindowIsActive &&
	   subWndId==subWindowManager.activeSubWindowId)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsIsSubWindowOpen(int subWndId)
{
	if(0<=subWndId && subWndId<FsMaxNumSubWindow)
	{
		return subWindowManager.subWindowIsOpen[subWndId];
	}
	return YSFALSE;

}

YSRESULT FsCloseSubWindow(int subWndId)
{
	if(0<=subWndId && subWndId<FsMaxNumSubWindow)
	{
		if(YSTRUE==FsIsSubWindowActive(subWndId))
		{
			FsSelectMainWindow();
		}
		subWindowManager.subWindowIsOpen[subWndId]=YSFALSE;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSelectMainWindow(void)
{
	subWindowManager.mainWindowIsActive=YSTRUE;
	subWindowManager.subWindowIsActive=YSFALSE;
	return YSOK;
}

YSRESULT FsSelectSubWindow(int subWndId)
{
	if(YSTRUE==FsIsSubWindowOpen(subWndId))
	{
		subWindowManager.mainWindowIsActive=YSFALSE;
		subWindowManager.subWindowIsActive=YSTRUE;
		subWindowManager.activeSubWindowId=subWndId;
		return YSOK;
	}
	return YSERR;
}

YsVec2i FsGetMainWindowDrawingAreaSize(void)
{
	int wid,hei;
	if(YSTRUE!=FsIsMainWindowSplit())
	{
		FsGetWindowSize(wid,hei);
	}
	else
	{
		FsGetWindowSize(wid,hei);
		wid/=2;
	}
	return YsVec2i(wid,hei);
}

YsVec2i FsGetSubWindowDrawingAreaSize(void)
{
	int size,mainWid,mainHei;
	FsGetWindowSize(mainWid,mainHei);
	size=mainWid*22/100;
	return YsVec2i(size,size);
}

void FsGetDrawingAreaSize(int &wid,int &hei)
{
	if(YSTRUE==FsIsMainWindowActive())
	{
		auto s=FsGetMainWindowDrawingAreaSize();
		wid=s.x();
		hei=s.y();
		return;
	}

	if(YSTRUE==subWindowManager.subWindowIsActive)
	{
		auto s=FsGetSubWindowDrawingAreaSize();
		wid=s.x();
		hei=s.y();
		return;
	}

	FsGetWindowSize(wid,hei);
}

YsVec2i FsGetDrawingAreaSize(void)
{
	int w,h;
	FsGetDrawingAreaSize(w,h);
	return YsVec2i(w,h);
}

void FsGetWindowViewport(int &x0,int &y0,int &wid,int &hei)
{
	if(YSTRUE==FsIsMainWindowActive())
	{
		FsGetWindowSize(wid,hei);
		if(YSTRUE!=FsIsMainWindowSplit())
		{
			x0=0;
			y0=0;
		}
		else
		{
			switch(FsGetActiveSplitWindow())
			{
			default:
			case 0:
				x0=0;
				y0=0;
				break;
			case 1:
				x0=wid/2;
				y0=0;
				break;
			}
			wid/=2;
		}
		return;
	}

	if(YSTRUE==subWindowManager.subWindowIsActive)
	{
		int mainWid,mainHei;
		FsGetWindowSize(mainWid,mainHei);
		FsGetDrawingAreaSize(wid,hei);

		switch(subWindowManager.activeSubWindowId)
		{
		default:
		case 0:
			x0=0;
			y0=mainHei-hei;
			break;
		case 1:
			x0=mainWid-wid;
			y0=mainHei-hei;
			break;
		}
		return;
	}

	x0=0;
	y0=0;
	FsGetWindowSize(wid,hei);
}

void FsSplitMainWindow(YSBOOL split)
{
	subWindowManager.mainWindowSplit=split;
}

void FsSetActiveSplitWindow(int id)
{
	subWindowManager.activeSplitWindow=id;
}

YSBOOL FsIsMainWindowSplit(void)
{
	return subWindowManager.mainWindowSplit;
}

int FsGetActiveSplitWindow(void)
{
	return subWindowManager.activeSplitWindow;
}

void FsLoadWindowSize(int &x0,int &y0,int &wid,int &hei,const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		YsString str;
		YsArray <YsString,16> args;

		while(str.Fgets(fp)!=NULL)
		{
			if(str.Arguments(args)==YSOK && args.GetN()>0)
			{
				switch(str[0])
				{
				case 'm':
				case 'M':
					if(args.GetN()>=5)
					{
						x0=atoi(args[1]);
						y0=atoi(args[2]);
						wid=atoi(args[3]);
						hei=atoi(args[4]);

						wid=YsGreater(320,wid);
						hei=YsGreater(240,hei);
					}
					break;
				}
			}
		}
		fclose(fp);
	}
}





// ex FsFindFileList(filelist,"aircraft","air","lst");
YSRESULT FsFindFileList(YsArray <YsWString> &filelist,const wchar_t dir[],const wchar_t prefix[],const wchar_t ext[])
{
	YsFileList fileList;
	if(YSOK==fileList.FindFileList(dir,prefix,ext))
	{
		filelist.Clear();

		for(int i=0; i<fileList.GetN(); i++)
		{
			if(YSTRUE!=fileList.IsDirectory(i))
			{
				filelist.Increment();
				filelist.GetEnd().Set(fileList.GetFileName(i));
			}
		}
		return YSOK;
	}
	else
	{
		filelist.Clear();
		return YSERR;
	}
}

YSRESULT FsFindFileDirList(YsArray <YsWString> &filelist,YsArray <YsWString> &dirlist,const wchar_t dir[],const wchar_t ext[])
{
	YsFileList fileList;
	if(YSOK==fileList.FindFileList(dir,L"",ext))
	{
		filelist.Clear();
		dirlist.Clear();

		int i;
		for(i=0; i<fileList.GetN(); i++)
		{
			if(YSTRUE!=fileList.IsDirectory(i))
			{
				filelist.Increment();
				filelist.GetEnd().Set(fileList.GetFileName(i));
			}
			else
			{
				dirlist.Increment();
				dirlist.GetEnd().Set(fileList.GetFileName(i));
			}
		}
		return YSOK;
	}
	else
	{
		filelist.Clear();
		dirlist.Clear();
		return YSERR;
	}
}


////////////////////////////////////////////////////////////

void FsMouse(YSBOOL &lb,YSBOOL &mb,YSBOOL &rb,int &mx,int &my)
{
	int l,m,r;
	FsGetMouseState(l,m,r,mx,my);
	lb=(l==0 ? YSFALSE : YSTRUE);
	mb=(m==0 ? YSFALSE : YSTRUE);
	rb=(r==0 ? YSFALSE : YSTRUE);
}

int FsGetMouseEvent(YSBOOL &lb,YSBOOL &mb,YSBOOL &rb,int &mx,int &my)
{
	int e,l,m,r;
	e=FsGetMouseEvent(l,m,r,mx,my);
	lb=(l==0 ? YSFALSE : YSTRUE);
	mb=(m==0 ? YSFALSE : YSTRUE);
	rb=(r==0 ? YSFALSE : YSTRUE);
	return e;

}

void FsSaveWindowSize(const wchar_t fn[])
{
	int x0,y0,wid,hei;

	FsGetWindowPosition(x0,y0);
	FsGetWindowSize(wid,hei);

	x0&=(~3);
	y0&=(~3);
	wid&=(~3);
	hei&=(~3);

	YsFileIO::File fp(fn,"w");
	if(fp!=nullptr)
	{
		fprintf(fp,"M %d %d %d %d\n",x0,y0,wid,hei);
	}
}
