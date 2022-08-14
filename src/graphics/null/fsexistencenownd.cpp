#include <ysclass.h>
#include <yscompilerwarning.h>
#include "fs.h"

void FsAirplane::DrawVapor(double currentTime,double remainTime,int step,YSBOOL transparency) const
{
	YsDisregardVariable(currentTime);
	YsDisregardVariable(remainTime);
	YsDisregardVariable(step);
	YsDisregardVariable(transparency);
}

void FsAirplane::DrawSingleSmoke(int smkId,double currentTime,double remainTime,FSSMOKETYPE smk,int step,YSBOOL transparency) const
{
	YsDisregardVariable(smkId);
	YsDisregardVariable(currentTime);
	YsDisregardVariable(remainTime);
	YsDisregardVariable(smk);
	YsDisregardVariable(step);
	YsDisregardVariable(transparency);
}

void FsField::DrawMapVisual(
    FSENVIRONMENT env,
    const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,const double &elvMin,const double &elvMax,YSBOOL drawPset,
    const double &currentTime,
    YSBOOL useOpenGlGroundTexture,YSBOOL useOpenGlRunwayLightTexture) const
{
	YsDisregardVariable(env);
	YsDisregardVariable(viewPos);
	YsDisregardVariable(viewAtt);
	YsDisregardVariable(elvMin);
	YsDisregardVariable(elvMax);
	YsDisregardVariable(drawPset);
	YsDisregardVariable(currentTime);
	YsDisregardVariable(projMat);
	YsDisregardVariable(useOpenGlGroundTexture);
	YsDisregardVariable(useOpenGlRunwayLightTexture);
}

