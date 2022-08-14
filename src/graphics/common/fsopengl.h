#ifndef FSOPENGL_IS_INCLUDED
#define FSOPENGL_IS_INCLUDED
/* { */

#include <ysviewcontrol.h>
#include "fsdef.h"

class FsProjection
{
public:
	YsProjectionTransformation::PROJMODE prjMode;

//   |    /
//   |FOV/
//   |  /
//   | /
//   |/
//   V
	double tanFov;      // Tangent of field of view (FOV)
	double prjPlnDist;  // =scrnwidth/(2*tanFov)
	double nearz,farz;
	YsVec2i viewportDim;

	int cx,cy;

protected:
	mutable YSBOOL matrixCached;
	mutable YsMatrix4x4 projMatCache;

public:
	FsProjection();
	void CacheMatrix(void) const;
	void UncacheMatrix(void) const;
	const YsMatrix4x4 &GetMatrix(void) const;
};

extern unsigned int FS_GL_FONT_BITMAP_BASE; // 256 lists
extern unsigned int FS_GL_WIRE_BITMAP_BASE; // 256 lists


const char *FsMainWindowTitle(void);


void FsInitializeOpenGL(void);
void FsReinitializeOpenGL(void);
void FsUninitializeOpenGL(void);

YSBOOL FsIsConsoleServer(void);

YSBOOL FsIsShadowMapAvailable(void);

/*! Returns YSTRUE if point sprite is available.
    As of 2018/04/06, only OpenGL 2.0 library returns YSTRUE.  D3D9, OpenGL 1.1 library return YSFALSE.
*/
YSBOOL FsIsPointSpriteAvailable(void);

void FsClearScreenAndZBuffer(const YsColor &clearColor);
void FsClearStencilBuffer(void);
void FsSetPerPixelShading(YSBOOL perPix);
void FsSetPointLight(const YsVec3 &cameraPosition,const YsVec3 &lightPosition,FSENVIRONMENT env);
void FsSetDirectionalLight(const YsVec3 &cameraPosition,const YsVec3 &lightDirection,FSENVIRONMENT env);
void FsFogOn(const YsColor &fogColor,const double &visibility);
void FsFogOff(void);

void FsBeginRenderShadowMap(const YsMatrix4x4 &projTfm,const YsMatrix4x4 &viewTfm,int texWid,int texHei);
void FsEndRenderShadowMap(void);
void FsEnableShadowMap(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &shadowProjTfm,const YsMatrix4x4 &shadowViewTfm,
    int samplerIdent,int shadowMapIdent);
void FsDisableShadowMap(int samplerIdent,int shadowMapIdent);

void FsSetSceneProjection(const class FsProjection &prj);
void FsSet2DDrawing(void);

void FsBeginDrawShadow(void);  // Set polygon offset -1,-1 and enable.
void FsEndDrawShadow(void);    // Disable polygon offset.

void FsSetCameraPosition(const YsVec3 &pos,const YsAtt3 &att,YSBOOL zClear);
void FsFlushScene(void);

void FsDrawString(int x,int y,const char str[],const YsColor &col);
void FsDrawString(int x,int y,const wchar_t str[],YsColor col);
void FsDrawLine(int x1,int y1,int x2,int y2,const YsColor &col);

void FsDrawRect(int x1,int y1,int x2,int y2,const YsColor &col,YSBOOL fill);
void FsDrawCircle(int x,int y,int rad,const YsColor &col,YSBOOL fill);
void FsDrawPolygon(int n,int plg[],const YsColor &col);
void FsDrawDiamond(int x,int y,int r,const YsColor &col,YSBOOL fill);
void FsDrawX(int x,int y,int r,const YsColor &col);
void FsDrawPoint(int x,int y,const YsColor &col);
void FsDrawPoint2Pix(int x,int y,const YsColor &col);

void FsDrawTitleBmp(const class YsBitmap &bmp,YSBOOL tile);
void FsDrawBmp(const class YsBitmap &bmp,int x,int y);

void FsGlDepthMask(YSBOOL sw);

void FsDrawLine3d(const YsVec3 &p1,const YsVec3 &p2,const YsColor &col);






extern int BiWorkSize;
extern char BiWork[];

extern double fsOpenGLVersion;

void FsGraphicsTest(int);



/* } */
#endif
