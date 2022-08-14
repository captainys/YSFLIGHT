#ifndef FSCONMENU_IS_INCLUDED
#define FSCONMENU_IS_INCLUDED
/* { */



void FsConMenu(const class FsCommandParameter &fscp,class FsWorld *world);
void FsConStartServer(const class FsCommandParameter &fscp,class FsWorld *world);
void FsConConfigServer(FsWorld *world);

void NetworkHostFlightAuto(const char username[],const char fldName[],int netPort,const class FsCommandParameter &fscp);



/* } */
#endif
