#import <Cocoa/Cocoa.h>

#include "fsmacosx_alert.h"

void FsMessageDialogC(const char title[],const char msg[],const char *okBtnTxt)
{
	NSAlert *alert=[[NSAlert alloc] init];
	[alert addButtonWithTitle:[NSString stringWithUTF8String:okBtnTxt]];
	[alert setMessageText:[NSString stringWithUTF8String:msg]];
	[alert setInformativeText:[NSString stringWithUTF8String:title]];
	[alert runModal];
}

int FsYesNoDialogC(const char title[],const char msg[],const char *okBtnTxt,const char *cancelBtnTxt)
{
	int returnValue;
	NSAlert *alert=[[NSAlert alloc] init];
	[alert addButtonWithTitle:[NSString stringWithUTF8String:okBtnTxt]];
	[alert addButtonWithTitle:[NSString stringWithUTF8String:cancelBtnTxt]];
	[alert setMessageText:[NSString stringWithUTF8String:msg]];
	[alert setInformativeText:[NSString stringWithUTF8String:title]];

	returnValue=[alert runModal];
	if(NSAlertFirstButtonReturn)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
