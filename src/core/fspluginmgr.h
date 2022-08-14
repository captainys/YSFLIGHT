#ifndef FSPLUGINMGR_IS_INCLUDED
#define FSPLUGINMGR_IS_INCLUDED
/* { */

void FsLoadPlugIn(void);
void FsFreePlugIn(void);

void FsPlugInCallInitialize(FsWorld *world);
void FsPlugInCallInterval(const double &ctime,FsSimulation *sim);
void FsPlugInCallDrawForeground(const double &ctime);
void FsPlugInCallWindowBufferSwapped(const double &ctime,const FsSimulation *sim);

void FsPluginCallNetFreeMemory(const double &ctime,FsSimulation *sim);

/* } */
#endif
