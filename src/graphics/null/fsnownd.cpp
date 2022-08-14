#include <ysclass.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"


#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif


#include <ysbitmap.h>


#include <time.h>


extern const char *FsProgramTitle;  // Defined in fsmain.cpp

const char *FsMainWindowTitle(void)
{
	// As of 2014/12/12  Main window title must include keyword "Main Window" so that FsWin32GetMainWindowHandle can find the handle.

	static YsString windowTitle;
	windowTitle.Set(FsProgramTitle);
	windowTitle.Append(" Main Window");
	windowTitle.Append(" (Server Console)");
	return windowTitle;
}


#ifdef _WIN32
HWND FsWin32GetMainWindowHandle(void)
{
	return NULL;
}

HWND FsWin32GetSubWindowHandle(int subWndId)
{
	return NULL;
}

void FsWin32InstallJWordPlugin(void)
{
}

void FsWin32DeleteJWordPlugin(void)
{
}
#endif

int FsGetGuiBitmapScale(void)
{
	return 100;
}

YSBOOL FsIsConsoleServer(void)
{
	return YSTRUE;
}

YSBOOL FsIsShadowMapAvailable(void)
{
	return YSFALSE;
}

void FsInitializeOpenGL(void)
{
}

void FsReinitializeOpenGL(void)
{
	FsInitializeOpenGL();
}

void FsUninitializeOpenGL(void)
{
}

void FsClearScreenAndZBuffer(const YsColor &clearColor)
{
}

void FsClearStencilBuffer(void)
{
}

void FsSetPointLight(const YsVec3 &cameraPosition,const YsVec3 &lightPosition,FSENVIRONMENT env)
{
}

void FsSetDirectionalLight(const YsVec3 &cameraPosition,const YsVec3 &lightDirection,FSENVIRONMENT env)
{
}

void FsFogOn(const YsColor &fogColor,const double &visibility)
{
}

void FsFogOff(void)
{
}

void FsBeginRenderShadowMap(const YsMatrix4x4 &projTfm,const YsMatrix4x4 &viewTfm,int texWid,int texHei)
{
}

void FsEndRenderShadowMap(void)
{
}

void FsEnableShadowMap(
    const YsMatrix4x4 &,const YsMatrix4x4 &,const YsMatrix4x4 &,
    int ,int )
{
}
void FsDisableShadowMap(int,int)
{
}

void FsSetSceneProjection(const class FsProjection &prj)
{
}

void FsSet2DDrawing(void)
{
}

void FsBeginDrawShadow(void)  // Set polygon offset -1,-1 and enable.
{
}

void FsEndDrawShadow(void)    // Disable polygon offset.
{
}

void FsSetCameraPosition(const YsVec3 &pos,const YsAtt3 &att,YSBOOL zClear)
{
}

void FsFlushScene(void)
{
}

void FsDrawString(int x,int y,const char str[],const YsColor &col)
{
//	printf("%s\n",str);
}

void FsDrawLine(int x1,int y1,int x2,int y2,const YsColor &col)
{
}

void FsDrawRect(int x1,int y1,int x2,int y2,const YsColor &col,YSBOOL fill)
{
}

void FsDrawCircle(int x,int y,int rad,const YsColor &col,YSBOOL fill)
{
}

void FsDrawPolygon(int n,int plg[],const YsColor &col)
{
}

void FsDrawDiamond(int x,int y,int r,const YsColor &col,YSBOOL fill)
{
}

void FsDrawX(int x,int y,int r,const YsColor &col)
{
}

void FsDrawPoint(int x,int y,const YsColor &col)
{
}

void FsDrawPoint2Pix(int x,int y,const YsColor &col)
{
}

void FsDrawTitleBmp(const YsBitmap &bmp,YSBOOL tile)
{
}

void FsDrawBmp(const YsBitmap &bmp,int x,int y)
{
}

void FsDrawLine3d(const YsVec3 &p1,const YsVec3 &p2,const YsColor &col)
{
}

void FsGraphicsTest(int i)
{
}

YSBOOL FsIsPointSpriteAvailable(void)
{
	return YSFALSE;
}
