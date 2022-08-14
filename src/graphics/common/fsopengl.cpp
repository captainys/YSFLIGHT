#include <ysviewcontrol.h>
#include <ysbitmap.h>
#include <yssystemfont.h>
#include "graphics/common/fsopengl.h"
#include "graphics/common/fsfontrenderer.h"

FsProjection::FsProjection()
{
	matrixCached=YSFALSE;
}

void FsProjection::CacheMatrix(void) const
{
	matrixCached=YSTRUE;

	double aspect=viewportDim.xd()/viewportDim.yd();

	YsProjectionTransformation prjTfm;
	prjTfm.SetProjectionMode(prjMode);
	prjTfm.SetAspectRatio(aspect);
	prjTfm.SetFOVY(atan(tanFov));
	prjTfm.SetNearFar(nearz,farz);
	prjTfm.SetZoom(1.0);

	projMatCache=prjTfm.GetProjectionMatrix();
}

void FsProjection::UncacheMatrix(void) const
{
	matrixCached=YSFALSE;
}

const YsMatrix4x4 &FsProjection::GetMatrix(void) const
{
	if(YSTRUE!=matrixCached)
	{
		CacheMatrix();
	}
	return projMatCache;
}

////////////////////////////////////////////////////////////

void FsDrawString(int x,int y,const wchar_t str[],YsColor col)
{
	YsBitmap bmp;
	YsColor transparent=col;
	transparent.SetAi(0);
	fsUnicodeRenderer.RenderString(bmp,str,col,transparent);
	FsDrawBmp(bmp,x,y);
}

