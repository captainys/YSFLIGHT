#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <windows.h>

#include "yssync_module.h"


YsSyncDirs::YsSyncDirs()
{
	excludeExtList=NULL;
}

SDBOOL YsSyncDirs::Sync(const char src[],const char dst[],YsSyncExtList *exclude)
{
	excludeExtList=exclude;

	if(IfExist(src)==SD_TRUE && IsDirectory(src)!=SD_TRUE &&
	   IfExist(dst)==SD_TRUE && IsDirectory(dst)==SD_TRUE)
	{
		/* File to Directory */
		return SyncFileDirectory(src,dst);
	}
	else
	{
		/* Directory to Directory */
		return SyncDirectory(src,dst);
	}
}

SDBOOL YsSyncDirs::SyncFileDirectory(const char src[],const char dst[])
{
	char srcFul[256],dstFul[256],*dstFile;
	if(GetFullPathName(src,256,srcFul,&dstFile)>0)
	{
		sprintf(dstFul,"%s/%s",dst,dstFile);
		//printf("File to Directory\n");
		//printf("Trying from:%s\n",srcFul);
		//printf("         to:%s\n",dstFul);

		if(IsNewer(srcFul,dstFul)==SD_TRUE)
		{
			if(IsExcluded(srcFul)==SD_TRUE)
			{
				printf("  Excluded:%s\n",srcFul);
			}
			else
			{
				printf("  Update:%s\n",dstFul);
				CopyFile(srcFul,dstFul,FALSE);
			}
		}
		else
		{
			printf("  The file do not have to be updated.\n");
		}

		return SD_TRUE;
	}
	else
	{
		printf("Cannot extract filename.\n");
		return SD_FALSE;
	}
}

SDBOOL YsSyncDirs::SyncDirectory(const char srcDir[],const char dstDir[])
{
	//printf("Directory to Directory\n");
	//printf("Trying from:%s\n",srcDir);
	//printf("         to:%s\n",dstDir);

	if(IfExist(dstDir)!=SD_TRUE)
	{
		if(CreateDirectory(dstDir,NULL)!=TRUE)
		{
			printf("Cannot create directory.\n");
			printf("  %s\n",dstDir);
			return SD_FALSE;
		}
	}

	if(IsDirectory(srcDir)!=SD_TRUE || IsDirectory(dstDir)!=SD_TRUE)
	{
		printf("Source and Destination must be\n");
		printf("   Directory to Directory\n");
		printf("       or\n");
		printf("   File to Directory.\n");
		return SD_FALSE;
	}

	if(IsPartOf(srcDir,dstDir)==SD_TRUE)
	{
		/* Ex. C:\SRC to C:\SRC\SRC2 will cause infinite recursion */
		printf("Recursive Copy is not allowed.\n");
		return SD_TRUE;
	}

	if(SyncNormalFiles(srcDir,dstDir)!=SD_TRUE)
	{
		return SD_FALSE;
	}
	if(SyncSubdirectories(srcDir,dstDir)!=SD_TRUE)
	{
		return SD_FALSE;
	}
	return SD_TRUE;
}

SDBOOL YsSyncDirs::IfExist(const char fn[])
{
	if(IsDriveName(fn)==SD_TRUE)
	{
		return SD_TRUE;
	}

	HANDLE handle;
	WIN32_FIND_DATA attr;
	handle=FindFirstFile(fn,&attr);
	if(handle==INVALID_HANDLE_VALUE)
	{
		return SD_FALSE;
	}
	FindClose(handle);
	return SD_TRUE;
}

SDBOOL YsSyncDirs::IsDriveName(const char fn[])
{
	if(('a'<=fn[0] && fn[0]<='z') || ('A'<=fn[0] && fn[0]<='Z'))
	{
		if(fn[1]==':' && (fn[2]=='\\' || fn[2]=='/') && fn[3]==0)
		{
			return SD_TRUE;
		}
		if(fn[1]==':' && fn[2]==0)
		{
			return SD_TRUE;
		}
	}
	return SD_FALSE;
}

SDBOOL YsSyncDirs::IsDirectory(const char fn[])
{
	if(IsDriveName(fn)==SD_TRUE)
	{
		return SD_TRUE;
	}

	HANDLE handle;
	WIN32_FIND_DATA attr;
	handle=FindFirstFile(fn,&attr);
	if(handle==INVALID_HANDLE_VALUE)
	{
		fprintf(stderr,"File (%s) Not Found.\n",fn);
		return SD_FALSE;
	}

	SDBOOL r;
	r=((attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0 ?
	    SD_TRUE : SD_FALSE);

	FindClose(handle);

	return r;
}

void YsSyncDirs::MakePath(char wc[],const char dir[],const char name[])
{
	int l;
	l=strlen(dir);
	if(l<=0 || dir[l-1]=='\\' || dir[l-1]=='/')
	{
		sprintf(wc,"%s%s",dir,name);
	}
	else
	{
		sprintf(wc,"%s\\%s",dir,name);
	}
}

SDBOOL YsSyncDirs::SyncNormalFiles(const char srcDir[],const char dstDir[])
{
	HANDLE handle;
	WIN32_FIND_DATA attr;
	char srcWildCard[MAX_PATH],srcFul[MAX_PATH],dstFul[MAX_PATH];

	MakePath(srcWildCard,srcDir,"*.*");

	handle=FindFirstFile(srcWildCard,&attr);
	if(handle==INVALID_HANDLE_VALUE)
	{
		return SD_TRUE;
	}
	while(1)
	{
		if((attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0)
		{
			MakePath(srcFul,srcDir,attr.cFileName);
			MakePath(dstFul,dstDir,attr.cFileName);
			if(IsNewer(srcFul,dstFul)==SD_TRUE)
			{
				if(IsExcluded(srcFul)==SD_TRUE)
				{
					printf("  Excluded:%s\n",srcFul);
				}
				else
				{
					printf("  Update:%s\n",dstFul);
					CopyFile(srcFul,dstFul,FALSE);
				}
			}
		}
		if(FindNextFile(handle,&attr)!=TRUE)
		{
			break;
		}
	}
	FindClose(handle);
	return SD_TRUE;
}

SDBOOL YsSyncDirs::SyncSubdirectories(const char srcDir[],const char dstDir[])
{
	HANDLE handle;
	WIN32_FIND_DATA attr;
	char srcWildCard[MAX_PATH],srcFul[MAX_PATH],dstFul[MAX_PATH];

	MakePath(srcWildCard,srcDir,"*.*");

	handle=FindFirstFile(srcWildCard,&attr);
	if(handle==INVALID_HANDLE_VALUE)
	{
		return SD_TRUE;
	}
	while(1)
	{
		if((attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0 &&
		   strcmp(attr.cFileName,".")!=0 &&
		   strcmp(attr.cFileName,"..")!=0)
		{
			MakePath(srcFul,srcDir,attr.cFileName);
			MakePath(dstFul,dstDir,attr.cFileName);
			if(SyncDirectory(srcFul,dstFul)!=SD_TRUE)
			{
				FindClose(handle);
				return SD_FALSE;
			}
		}
		if(FindNextFile(handle,&attr)!=TRUE)
		{
			break;
		}
	}
	FindClose(handle);
	return SD_TRUE;
}

SDBOOL YsSyncDirs::IsPartOf(const char srcDir[],const char dstDir[])
{
	int i,l;
	char src[MAX_PATH],dst[MAX_PATH];
	for(i=0; srcDir[i]!=0; i++)
	{
		src[i]=(('a'<=srcDir[i] && srcDir[i]<='z') ?
		         srcDir[i]+'A'-'a' : srcDir[i]);
		if(src[i]=='/')
		{
			src[i]='\\';
		}
	}
	src[i]=0;
	for(i=0; dstDir[i]!=0; i++)
	{
		dst[i]=(('a'<=dstDir[i] && dstDir[i]<='z') ?
		          dstDir[i]+'A'-'a' : dstDir[i]);
		if(dst[i]=='/')
		{
			dst[i]='\\';
		}
	}
	dst[i]=0;

	l=strlen(src);
	if(strcmp(src,dst)==0 ||
	   (strncmp(src,dst,l)==0 && (dst[l]=='\\' || dst[l]=='/')))
	{
		return SD_TRUE;
	}

	return SD_FALSE;
}

SDBOOL YsSyncDirs::IsNewer(const char src[],const char dst[])
{
	HANDLE handle;
	WIN32_FIND_DATA srcAttr,dstAttr;
	FILETIME srcTime,dstTime;
	handle=FindFirstFile(dst,&dstAttr);
	if(handle==INVALID_HANDLE_VALUE)
	{
		return SD_TRUE;
	}
	dstTime=dstAttr.ftLastWriteTime;
	FindClose(handle);

	handle=FindFirstFile(src,&srcAttr);
	if(handle==INVALID_HANDLE_VALUE)
	{
		printf("Some errors have happend during the process.\n");
		return SD_FALSE;
	}
	srcTime=srcAttr.ftLastWriteTime;
	FindClose(handle);

	if(srcTime.dwHighDateTime>dstTime.dwHighDateTime ||
	  (srcTime.dwHighDateTime==dstTime.dwHighDateTime &&
	   srcTime.dwLowDateTime>dstTime.dwLowDateTime))
	{
		return SD_TRUE;
	}
	return SD_FALSE;
}

SDBOOL YsSyncDirs::IsExcluded(const char fn[])
{
	int i,lf,le;
	YsSyncExtList *ext;
	char uncap[256];

	strcpy(uncap,fn);
	for(i=0; uncap[i]!=0; i++)
	{
		if('A'<=uncap[i] && uncap[i]<='Z')
		{
			uncap[i]=uncap[i]+'a'-'A';
		}
	}
	uncap[i]=0;

	lf=strlen(fn);
	for(ext=excludeExtList; ext!=NULL; ext=ext->next)
	{
		le=strlen(ext->ext);
		if(lf>=le && strcmp(uncap+lf-le,ext->ext)==0)
		{
			return SD_TRUE;
		}
	}

	return SD_FALSE;
}

////////////////////////////////////////////////////////////

YsSyncExtList *LoadExtList(const char fn[])
{
	FILE *fp;
	YsSyncExtList *lst,*neo;

	lst=NULL;

	fp=fopen(fn,"r");
	if(fp!=NULL)
	{
		char buf[256],ext[256];
		while(fgets(buf,256,fp)!=NULL)
		{
			int i;
			for(i=0; buf[i]!=0; i++)
			{
				if(buf[i]=='\r' || buf[i]=='\n')
				{
					buf[i]=0;
					break;
				}
				else if('A'<=buf[i] && buf[i]<='Z')
				{
					buf[i]=buf[i]+'a'-'A';
				}
			}

			neo=new YsSyncExtList;
			strncpy(neo->ext,buf,15);
			neo->ext[15]=0;
			neo->next=lst;
			lst=neo;

			printf("%s\n",neo->ext);
		}
		fclose(fp);
		return lst;
	}
	else
	{
		printf("Cannot load extension list.\n");
		return NULL;
	}
}

////////////////////////////////////////////////////////////

/*
int main(int ac,char *av[])
{
	printf("YSSYNC\n");
	printf("Synchronize Directories by Captain S.Yamakawa\n");
	printf("      PEB01130@niftyserve.or.jp\n");

	if(ac<3)
	{
		printf("Usage : yssync <source[Dir|File]> <destination[Dir]> -xEXCLUDEEXTENSIONLIST.LST\n");
		printf("\n");
		printf("Ex.   : yssync C:\\Web E:\\Web\n");
		printf("        Copy updated files from C:\\Web to E:\\Web.\n");
		return 1;
	}

	YsSyncDirs sd;
	YsSyncExtList *excludeExtList;

	excludeExtList=NULL;

	int i;
	for(i=3; i<ac; i++)
	{
		if(av[i][0]=='-' && av[i][1]=='x')
		{
			printf("Loading excluded extension list...\n");
			excludeExtList=LoadExtList(av[i]+2);
		}
	}

	if(sd.Sync(av[1],av[2],excludeExtList)!=SD_TRUE)
	{
		return 1;
	}
	return 0;
} */

