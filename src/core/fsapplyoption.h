#ifndef FSAPPLYOPTION_IS_INCLUDED
#define FSAPPLYOPTION_IS_INCLUDED
/* { */

#include "fsoption.h" 

YSRESULT FsReloadOption(YSBOOL enableFullScreen);
YSRESULT FsMakeLocalizationFromOption(const FsOption &cfg);
YSRESULT FsApplyOption(const FsOption &opt);
YSRESULT FsApplyNonScreenOption(const FsOption &opt);

/* } */
#endif
