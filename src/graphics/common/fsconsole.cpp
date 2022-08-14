#include <time.h>


#include <fssimplewindow.h>
#include <fsguidialog.h>


#include "graphics/common/fsconsole.h"
#include "graphics/common/fsfontrenderer.h"
#include "graphics/common/fsopengl.h"


FsScreenMessage fsConsole;

FsScreenMessage::FsScreenMessage() :
	strAllocator(256),
	strContainer(strAllocator)
{
#ifndef ANDROID
	autoFlush=YSTRUE;
#else
	autoFlush=YSFALSE;
#endif
	useStdout=YSFALSE;
	dlg=NULL;
}

void FsScreenMessage::SetDialog(class FsGuiDialog *dlg)
{
	this->dlg=dlg;
}

void FsScreenMessage::Show(const class YsBitmap *bgBmp)
{
	if(FsIsConsoleServer()!=YSTRUE)
	{
		FsClearScreenAndZBuffer(YsBlack());
		FsSet2DDrawing();

		Draw(bgBmp);

		FsSwapBuffers();
	}
	else
	{
		if(wInputLine.Strlen()>0)
		{
			YsString inputLine;
			inputLine.EncodeUTF8<wchar_t>(wInputLine);
			if(clock()%CLOCKS_PER_SEC<CLOCKS_PER_SEC/2)
			{
				printf("%s  \r",(const char *)inputLine);
			}
			else
			{
				printf("%s_  \r",(const char *)inputLine);
			}
		}
	}
}

void FsScreenMessage::Draw(const class YsBitmap *bgBmp)
{
	int sx,sy;
	sx=20;

	int wid,hei;
	FsGetWindowSize(wid,hei);
	sy=hei-fsUnicodeRenderer.GetFontHeight();

	if(wInputLine.Strlen()>0)
	{
		if(clock()%CLOCKS_PER_SEC<CLOCKS_PER_SEC/2)
		{
			FsDrawString(sx,sy,wInputLine,YsGreen());
		}
		else
		{
			wInputLine.Append(L"_");
			FsDrawString(sx,sy,wInputLine,YsGreen());
			wInputLine.BackSpace();
		}
		sy-=fsUnicodeRenderer.GetFontHeight();
	}

	YsListItem <FsScreenMessageString> *ptr;
	ptr=NULL;
	while((ptr=strContainer.FindPrev(ptr))!=NULL && sy>=fsUnicodeRenderer.GetFontHeight())
	{
		FsDrawString(sx,sy,ptr->dat.wstr,YsWhite());
		sy-=fsUnicodeRenderer.GetFontHeight();
		sy-=2;
	}

	if(bgBmp!=NULL)
	{
		FsDrawTitleBmp(*bgBmp,YSFALSE);
	}

	if(NULL!=dlg)
	{
		dlg->Show();
	}
}

void FsScreenMessage::Clear(void)
{
	strContainer.CleanUp();
	if(autoFlush==YSTRUE)
	{
		Show();
	}
}

void FsScreenMessage::ReplaceLastLine(const char str[])
{
	YsListItem <FsScreenMessageString> *itm;
	itm=strContainer.FindPrev(NULL);
	if(itm!=NULL)
	{
		itm->dat.wstr.SetUTF8String(str);
		itm->dat.wstr.DeleteTailSpace();
		if(autoFlush==YSTRUE)
		{
			Show();
		}
	}
	else
	{
		Output((char *)str);
	}
}

void FsScreenMessage::SetAutoFlush(YSBOOL automatic)
{
#ifndef ANDROID
	autoFlush=automatic;
#else
	autoFlush=YSFALSE;
#endif
}

void FsScreenMessage::Output(char str[])
{
	if(FsIsConsoleServer()!=YSTRUE)
	{
		YsListItem <FsScreenMessageString> *ptr;
		if(strContainer.GetN()>=256)
		{
			ptr=strContainer.SeekTop();
			strContainer.MoveItemToEnd(ptr);
		}
		else
		{
			ptr=strContainer.Create();
		}

		ptr->dat.wstr.SetUTF8String(str);
		ptr->dat.wstr.DeleteTailSpace();
		if(autoFlush==YSTRUE)
		{
			FsPushOnPaintEvent();
		}
	}
	else
	{
		if(useStdout==YSTRUE)
		{
			printf("%s\n",str);
		}
	}
}

/* virtual */ void FsScreenMessage::Output(const wchar_t wstr[])
{
	if(FsIsConsoleServer()!=YSTRUE)
	{
		YsListItem <FsScreenMessageString> *ptr;
		if(strContainer.GetN()>=256)
		{
			ptr=strContainer.SeekTop();
			strContainer.MoveItemToEnd(ptr);
		}
		else
		{
			ptr=strContainer.Create();
		}

		ptr->dat.wstr.Set(wstr);
		ptr->dat.wstr.DeleteTailSpace();
		if(autoFlush==YSTRUE)
		{
			FsPushOnPaintEvent();
		}
	}
	else
	{
		if(useStdout==YSTRUE)
		{
			YsString utf8;
			utf8.EncodeUTF8<wchar_t>(wstr);
			printf("%s\n",utf8.Txt());
		}
	}
}

