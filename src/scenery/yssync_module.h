#ifndef YSSYNC_MODULE_IS_INCLUDED
#define YSSYNC_MODULE_IS_INCLUDED
/* { */

class YsSyncExtList
{
public:
	YsSyncExtList *next;
	char ext[16];
};



typedef enum
{
	SD_TRUE,
	SD_FALSE
} SDBOOL;

class YsSyncDirs
{
public:
	YsSyncDirs();

	SDBOOL Sync(const char src[],const char dst[],YsSyncExtList *excludeExtList);
	SDBOOL SyncFileDirectory(const char src[],const char dst[]);
	SDBOOL SyncDirectory(const char srcDir[],const char dstDir[]);
	SDBOOL IfExist(const char fn[]);
	SDBOOL IsDriveName(const char fn[]);
	SDBOOL IsDirectory(const char fn[]);
	void MakePath(char wc[],const char dir[],const char name[]);
	SDBOOL SyncNormalFiles(const char srcDir[],const char dstDir[]);
	SDBOOL SyncSubdirectories(const char srcDir[],const char dstDir[]);
	SDBOOL IsPartOf(const char srcDir[],const char dstDir[]);
	SDBOOL IsNewer(const char src[],const char dst[]);

	SDBOOL IsExcluded(const char fn[]);

	YsSyncExtList *excludeExtList;
};

/* } */
#endif
