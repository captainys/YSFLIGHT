#include <Cocoa/Cocoa.h>


int FsMacOSXGetFileList(char buf[],const char dir[],const char prefix[],const char ext[]) 
{
	const int lBuf=4096;

	int bufPtr=0;
	buf[0]=0;
	buf[1]=0;

	NSAutoreleasePool *pool;
	pool=[[NSAutoreleasePool alloc] init];

	NSFileManager *fileMngr;
	fileMngr=[[NSFileManager alloc] init];

	printf("File Listing at %s (%s %s)\n",dir,prefix,ext);

	NSArray *fileList,*atPath;
	atPath=[[NSString alloc] initWithUTF8String:dir];
	fileList=[fileMngr contentsOfDirectoryAtPath:atPath error:NULL];
	[atPath release];

	printf("%d files are found.\n",[fileList count]);

	int lPrefix,lExt;
	lPrefix=strlen(prefix);
	lExt=strlen(ext);

	int i,j;
	for(i=0; i<[fileList count]; i++)
	{
		NSString *str;
		char utf8[256];
		int lFile;

		str=[fileList objectAtIndex:i];
		strncpy(utf8,[str UTF8String],255);
		utf8[255]=0;

		lFile=strlen(utf8);
		if(lFile>lPrefix && lFile>lExt && strncmp(utf8,prefix,lPrefix)==0 && strncmp(utf8+lFile-lExt,ext,lExt)==0)
		{
			printf("[%d] %s\n",i,utf8);

			if(bufPtr+lFile<lBuf-2)
			{
				strcpy(buf+bufPtr,utf8);
				bufPtr+=lFile+1;
				buf[bufPtr]=0;
			}
		}
	}

	[pool release];

	return 0;
}
