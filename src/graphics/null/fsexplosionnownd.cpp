#include <ysclass.h>
#include <yscompilerwarning.h>
#include "fs.h"


void FsExplosion::Draw(const YsVec3 &viewPos,YSBOOL transparency,YSBOOL useOpenGlDisplayList) const
{
	YsDisregardVariable(viewPos);
	YsDisregardVariable(transparency);
	YsDisregardVariable(useOpenGlDisplayList);
}

void FsExplosionHolder::Draw(const YsVec3 &viewPos,YSBOOL transparency,YSBOOL useOpenGlDisplayList) const
{
	YsDisregardVariable(viewPos);
	YsDisregardVariable(transparency);
	YsDisregardVariable(useOpenGlDisplayList);
}

