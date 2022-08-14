// $Id: fsstdout.h,v 1.4 2007/09/13 02:43:52 soji Exp $

#ifndef FSSTDOUT_IS_INCLUDED
#define FSSTDOUT_IS_INCLUDED
/* { */

// Declaration /////////////////////////////////////////////
class FsStdout : public YsPrintf
{
public:
	static const char *ClassName;
	virtual const char *WhatItIs(void){return FsStdout::ClassName;}
	FsStdout();
	FsStdout(const FsStdout &from);
	virtual ~FsStdout();

	int noMoreErr;

	virtual void Output(char str[]);
};

// Declaration /////////////////////////////////////////////
class FsStderr : public YsPrintf
{
public:
	static const char *ClassName;
	virtual const char *WhatItIs(void){return FsStderr::ClassName;}
	FsStderr();
	FsStderr(const FsStderr &from);
	virtual ~FsStderr();

	YSBOOL noMoreErr;
	YSBOOL first;
	virtual void Output(char str[]);
};

extern FsStdout fsStdout;
extern FsStderr fsStderr;

////////////////////////////////////////////////////////////

void FsClearConsoleWindow(void);
void FsResetConsoleWindowCursorPosition(void);
void FsSetConsoleCursorLocation(int x,int y);

/* } */
#endif
