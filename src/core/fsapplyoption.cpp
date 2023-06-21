#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <ysclass.h>
#include <ysport.h>
#include <fsgui.h>

#include "fsapplyoption.h"

#include "fs.h"
#include "graphics/common/fsopengl.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsfontrenderer.h"

#include "fstextresource.h"



#ifdef _WIN32
YSBOOL FsD3dMyDriverSucks=YSFALSE;
#endif

YSRESULT FsReloadOption(YSBOOL /*enableFullScreen*/)
{
	FsOption opt;
	opt.Load(FsGetOptionFile());
	FsApplyOption(opt);
	return YSOK;
}

YSRESULT FsLoadTextResource(const wchar_t fn[])
{
	{
		YsFileIO::File fp(fn,"r");
		if(nullptr!=fp && YSOK==fsTextResource.LoadFile(fp))
		{
			return YSOK;
		}
	}

	{
		YsWString ful;
		ful.MakeFullPathName(L"language",fn);
		YsFileIO::File fp(ful,"r");
		if(nullptr!=fp && YSOK==fsTextResource.LoadFile(fp))
		{
			return YSOK;
		}
	}

	{
		YsWString userLangDir;
		userLangDir.MakeFullPathName(FsGetUserYsflightDir(),L"language");

		YsWString ful;
		ful.MakeFullPathName(userLangDir,fn);
		YsFileIO::File fp(ful,"r");
		if(nullptr!=fp && YSOK==fsTextResource.LoadFile(fp))
		{
			return YSOK;
		}
	}

	return YSERR;
}

YSRESULT FsClearTextResource(void)
{
	fsTextResource.Initialize();
	return YSOK;
}

YSRESULT FsMakeLocalizationFromOption(const FsOption &cfg)
{
	switch(cfg.languageType)
	{
	case FsOption::FORCEENGLISH:
		FsOption::SetLanguageStringOverrideFromOption("en");
		FsLoadTextResource(L"en.uitxt");
		break;
	case FsOption::AUTOMATIC:
		{
			YsLocale locale;
			YsString langFn(FsOption::GetLanguageString());
			langFn.ReplaceExtension("uitxt");
			printf("Localization File: %s\n",langFn.Txt());

			FsOption::SetLanguageStringOverrideFromOption("");

			YsWString wLangFn;
			wLangFn.SetUTF8String(langFn);
			if(YSOK!=FsLoadTextResource(wLangFn))
			{
				printf("Failed to read the localization.  Fall back to English\n");
				YsString langFn("en.uitxt");
				wLangFn.SetUTF8String(langFn);
				FsLoadTextResource(wLangFn);
			}
		}
		break;
	case FsOption::SPECIFYFILE:
		{
			FsOption::SetLanguageStringOverrideFromOption("");

			FsLoadTextResource(cfg.languageFile);
			YsString cStr;
			YsUnicodeToSystemEncoding(cStr,cfg.languageFile);
			printf("Localization File: %s\n",cStr.Txt());
		}
		break;
	}
	return YSOK;
}

YSRESULT FsApplyOption(const FsOption &opt)
{
	{
#ifdef _WIN32
		FsD3dMyDriverSucks=opt.myD3dDriverSucks;
#endif

		int wid,hei,depth;
		if(opt.scrnMode==0)
		{
			FsSetNormalWindow();
		}
		else if(opt.scrnMode==1)
		{
			FsMaximizeWindow();
		}
		else if(2<=opt.scrnMode && opt.GetFullScreenResolution(wid,hei,depth)==YSOK)
		{
			FsSetFullScreen(wid,hei,depth);
		}


		if(opt.scrnMode==0 || opt.scrnMode==1)
		{
			FsSetTopMostWindow(opt.alwaysOnTop);
		}

		FsSetFont(opt.fontName,opt.fontHeight);
		FsMakeLocalizationFromOption(opt);

		return FsApplyNonScreenOption(opt);
	}
	return YSERR;
}

YSRESULT FsApplyNonScreenOption(const FsOption &opt)
{
#ifdef _WIN32
	FsD3dMyDriverSucks=opt.myD3dDriverSucks;
#endif

	FsSoundSetMasterSwitch(opt.sound);

	if(opt.useTaskBarIcon==YSTRUE)
	{
		FsTaskBarAddIcon();
	}
	else
	{
		FsTaskBarDeleteIcon();
	}

	FsSetFont(opt.fontName,opt.fontHeight);
	FsMakeLocalizationFromOption(opt);

	FsGuiObject::defRoundRadius=(double)opt.fontHeight/3.0;
	FsGuiObject::defHScrollBar=opt.fontHeight;
	FsGuiObject::defHAnnotation=opt.fontHeight*9/10;

	FsGuiObject::scheme=FsGuiObject::MODERN;

	FsGuiObject::defDialogBgCol.SetDoubleRGB(0.75,0.75,0.75);

	FsGuiObject::defTabBgCol.SetDoubleRGB(0.82,0.82,0.82);
	FsGuiObject::defTabClosedFgCol.SetDoubleRGB(0.8,0.8,0.8);
	FsGuiObject::defTabClosedBgCol.SetDoubleRGB(0.2,0.2,0.2);

	FsGuiObject::defBgCol.SetDoubleRGB(0.85,0.85,0.85);
	FsGuiObject::defFgCol.SetDoubleRGB(0.0,0.0,0.0);
	FsGuiObject::defActiveBgCol.SetDoubleRGB(0.3,0.3,0.7);
	FsGuiObject::defActiveFgCol.SetDoubleRGB(1.0,1.0,1.0);
	FsGuiObject::defFrameCol.SetDoubleRGB(0.0,0.0,0.0);

	FsGuiObject::defListFgCol.SetDoubleRGB(0.0,0.0,0.0);
	FsGuiObject::defListBgCol.SetDoubleRGB(0.8,0.8,0.8);
	FsGuiObject::defListActiveFgCol.SetDoubleRGB(1.0,1.0,1.0);
	FsGuiObject::defListActiveBgCol.SetDoubleRGB(0.3,0.3,0.7);

	return YSOK;
}

