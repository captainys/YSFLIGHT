

// The platform dependent functions are declared in fswindow.h


#include <ysclass.h>
#include <fssimplewindow.h>

#include <time.h>

#include <fsdef.h>
#include <fsconsole.h>

#include <fswindow.h>
#include <fsopengl.h>



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>



////////////////////////////////////////////////////////////

YSRESULT FsTaskBarAddIcon(void)
{
	return YSERR;
}

YSRESULT FsTaskBarDeleteIcon(void)
{
	return YSERR;
}

void FsBeforeOpenWindow(const class FsOption &,const class FsFlightConfig &)
{
	fsConsole.useStdout=YSTRUE;
}
void FsAfterOpenWindow(const class FsOption &,const class FsFlightConfig &)
{
}


void FsSetTopMostWindow(YSBOOL)
{
}

void FsSetFullScreen(int /*wid*/,int /*hei*/,int /*bpp*/)
{
}

void FsSetNormalWindow(void)
{
}

void FsSetOnPaintCallback(class FsOnPaintCallback *)
{
}

void FsHidePartOfScreenForSharewareMessage(void)
{
}

void FsMessageBox(const char msg[],const char title[])
{
	printf("%s\n",msg);
}

YSBOOL FsYesNoDialog(const char msg[],const char title[])
{
	return YSFALSE;
}

YSBOOL FsIsJoystickAxisAvailable(int joyId,int joyAxs)
{
	return YSFALSE;
}

YSRESULT FsPollJoystick(FsJoystick &joy,int joyId)
{
	return YSERR;
}

////////////////////////////////////////////////////////////

