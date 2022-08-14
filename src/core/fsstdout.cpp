#include <ysclass.h>
#include <ysport.h>
#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"

FsStdout fsStdout;
FsStderr fsStderr;

// Implementation //////////////////////////////////////////
const char *FsStdout::ClassName="FsStdout";

FsStdout::FsStdout()
{
	noMoreErr=YSFALSE;
}

FsStdout::FsStdout(const FsStdout &from)
{
	*this=from;
}

FsStdout::~FsStdout()
{
}

void FsStdout::Output(char str[])
{
	printf("%s",str);
	if(noMoreErr!=YSTRUE)
	{
		FsMessageBox(str,"MESSAGE");
	}
}

// Implementation //////////////////////////////////////////
const char *FsStderr::ClassName="FsStderr";

FsStderr::FsStderr()
{
	noMoreErr=YSFALSE;
	first=YSTRUE;
}

FsStderr::FsStderr(const FsStderr &from)
{
	*this=from;
}

FsStderr::~FsStderr()
{
}

void FsStderr::Output(char str[])
{
	FILE *fp;
	if(first==YSTRUE)
	{
		fp=YsFileIO::Fopen(FsGetErrFile(),"w");
		first=YSFALSE;
	}
	else
	{
		fp=YsFileIO::Fopen(FsGetErrFile(),"a");
	}
	if(fp!=NULL)
	{
		fprintf(fp,"%s",str);
		fclose(fp);
	}

	if(noMoreErr!=YSTRUE)
	{
		FsMessageBox(str,"ERROR");
	}
}


////////////////////////////////////////////////////////////

void FsClearConsoleWindow(void)
{
#ifdef WIN32
	HANDLE hCon;
	COORD pos;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD a;

	pos.X=0;
	pos.Y=0;

	hCon=GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(hCon,&csbi);

	FillConsoleOutputCharacter(hCon,(TCHAR)' ',csbi.dwSize.X*csbi.dwSize.Y,pos,&a);
	FillConsoleOutputAttribute(hCon,csbi.wAttributes,csbi.dwSize.X*csbi.dwSize.Y,pos,&a);

	SetConsoleCursorPosition(hCon,pos);

#else
	printf("\x1b[2J");
	printf("\x1b[0;0H");
#endif
}

void FsResetConsoleWindowCursorPosition(void)
{
#ifdef WIN32
	HANDLE hCon;
	COORD pos;

	pos.X=0;
	pos.Y=0;

	hCon=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hCon,pos);
#else
	printf("\x1b[0;0H");
#endif
}

void FsSetConsoleCursorLocation(int x,int y)
{
#ifdef WIN32
	HANDLE hCon;
	COORD pos;

	pos.X=x;
	pos.Y=y;

	hCon=GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hCon,pos);
#else
	char escSeq[256];

	sprintf(escSeq,"E[%d;%dH",x,y);
	escSeq[0]=0x1b;
	puts(escSeq);
#endif
}
