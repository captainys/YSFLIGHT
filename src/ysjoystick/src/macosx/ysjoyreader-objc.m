#include <stdio.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/NSPathUtilities.h> /* -framework Foundation */

FILE *YsJoyReaderOpenJoystickCalibrationFileC(const char mode[])
{
	FILE *fp;

	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	NSString *homeDir;
	NSString *cwd;

	homeDir=NSHomeDirectory();  // Do I release the string?
	cwd=[[NSFileManager defaultManager] currentDirectoryPath];

	[[NSFileManager defaultManager] changeCurrentDirectoryPath:homeDir];

	fp=fopen(".ysjoycalib",mode);

	[[NSFileManager defaultManager] changeCurrentDirectoryPath:cwd];

	[pool release];

	return fp;
}
