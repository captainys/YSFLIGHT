#import <Cocoa/Cocoa.h>




const char *FsGetMacOSXUserDirectory(char buf[])
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	strcpy(buf,[NSHomeDirectory() UTF8String]);
	[pool release];
	return buf;
}

const char *FsGetMacOSXBundleDirectory(char buf[])
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
	NSString *path;
	path=[[NSBundle mainBundle] bundlePath];
	[pool release];
	return buf;
}


