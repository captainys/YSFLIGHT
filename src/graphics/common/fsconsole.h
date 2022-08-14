#ifndef FSCONSOLE_IS_INCLUDED
#define FSCONSOLE_IS_INCLUDED
/* { */

#include <ysclass.h>

class FsScreenMessageString
{
public:
	YsWString wstr;
};

class FsScreenMessage : public YsPrintf
{
protected:
	YSBOOL autoFlush;
	YsListAllocator <FsScreenMessageString> strAllocator;
	YsListContainer <FsScreenMessageString> strContainer;

	class FsGuiDialog *dlg;

public:
	FsScreenMessage();

	void SetDialog(class FsGuiDialog *dlg);

	YSBOOL useStdout;  // <- Will be set in FsOpenWindow in nownd\fsifnownd.cpp
	YsWString wInputLine;

	virtual void Output(char str[]);
	virtual void Output(const wchar_t wstr[]);
	void SetAutoFlush(YSBOOL automatic);
	void Show(const class YsBitmap *bgBmp=NULL);
	void Draw(const class YsBitmap *bgBmp=NULL);

	void Clear(void);
	void ReplaceLastLine(const char str[]);
};

extern FsScreenMessage fsConsole;

/* } */
#endif
