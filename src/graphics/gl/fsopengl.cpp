#include <ysclass.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fs.h"
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#include "graphics/common/fsfontrenderer.h"

#include <fstexturemanager.h>

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif


#include <ysbitmap.h>


#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif


#include <time.h>



// #define YSOGLERRORCHECK

extern const char *FsProgramTitle;  // Defined in fsmain.cpp

const char *FsMainWindowTitle(void)
{
	// As of 2014/12/12  Main window title must include keyword "Main Window" so that FsWin32GetMainWindowHandle can find the handle.

	static YsString windowTitle;
	windowTitle.Set(FsProgramTitle);
	windowTitle.Append(" Main Window");
	windowTitle.Append(" (OpenGL 1.x)");
	return windowTitle;
}



#ifdef YSOGLERRORCHECK
void FsOpenGlShowError(const char tag[])
{
	int err;
	err=glGetError();
	if(err!=GL_NO_ERROR)
	{
		printf("%s ",tag);
		switch(err)
		{
		case GL_NO_ERROR:
			printf("GL_NO_ERROR (%d)\n",err);
			break;
		case GL_INVALID_ENUM:
			printf("GL_INVALID_ENUM (%d)\n",err);
			break;
		case GL_INVALID_VALUE:
			printf("GL_INVALID_VALUE (%d)\n",err);
			break;
		case GL_INVALID_OPERATION:
			printf("GL_INVALID_OPERATION (%d)\n",err);
			break;
		case GL_STACK_OVERFLOW:
			printf("GL_STACK_OVERFLOW (%d)\n",err);
			break;
		case GL_STACK_UNDERFLOW:
			printf("GL_STACK_UNDERFLOW (%d)\n",err);
			break;
		case GL_OUT_OF_MEMORY:
			printf("GL_OUT_OF_MEMORY (%d)\n",err);
			break;
//		case GL_TABLE_TOO_LARGE:
//			printf("GL_TABLE_TOO_LARGE\n");
//			break;
		default:
			printf("Uknown Error (%d)\n",err);
			break;
		}
	}
}
#endif



YSBOOL FsIsConsoleServer(void)
{
	return YSFALSE;
}

YSBOOL FsIsShadowMapAvailable(void)
{
	return YSFALSE;
}

YSBOOL FsIsPointSpriteAvailable(void)
{
	return YSFALSE;
}


#ifdef WIN32
static LARGE_INTEGER frmClock1,frmClock2;
#endif

// extern void FsGlMakeWireFontList(void);  // In fswirefontgl.cpp

double fsOpenGLVersion=1.0;

void FsInitializeOpenGL(void)
{
	const char *verStr=(const char *)glGetString(GL_VERSION);
	if(NULL!=verStr)
	{
		fsOpenGLVersion=atof(verStr);
	}

	YsBitmap bmp;

	const double tsc=0.025;
	double texGenParamS[4]={tsc,0.0,0.0,0.0};
	double texGenParamT[4]={0.0,0.0,tsc,0.0};
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
	glTexGendv(GL_S,GL_OBJECT_PLANE,texGenParamS);
	glTexGendv(GL_T,GL_OBJECT_PLANE,texGenParamT);

	GLfloat amb[]={0.0F,0.0F,0.0F,1.0F};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb);

	auto &commonTexture=FsCommonTexture::GetCommonTexture();
	commonTexture.LoadGroundTileTexture();
	commonTexture.LoadRunwayLightTexture();

	if(glIsList(FS_GL_FONT_BITMAP_BASE))
	{
		glDeleteLists(FS_GL_FONT_BITMAP_BASE,256);
	}
	if(glIsList(FS_GL_WIRE_BITMAP_BASE))
	{
		glDeleteLists(FS_GL_WIRE_BITMAP_BASE,256);
	}

	FS_GL_FONT_BITMAP_BASE=glGenLists(256); // 256 lists
	FS_GL_WIRE_BITMAP_BASE=glGenLists(256); // 256 lists

	// FsGlMakeWireFontList();

	// 2010/03/02
	// Polygon offset no longer needed.  Lights are now drawn as textured polygon.  Not really apoint.
	// Therefore, polygon offset doesn't matter.
	// glEnable(GL_POLYGON_OFFSET_FILL);
	// glPolygonOffset(1,1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS,0,0);
}

void FsReinitializeOpenGL(void)
{
	FsInitializeOpenGL();
}

void FsUninitializeOpenGL(void)
{
	if(glIsList(FS_GL_FONT_BITMAP_BASE))
	{
		glDeleteLists(FS_GL_FONT_BITMAP_BASE,256);
	}
	if(glIsList(FS_GL_WIRE_BITMAP_BASE))
	{
		glDeleteLists(FS_GL_WIRE_BITMAP_BASE,256);
	}
}

void FsClearScreenAndZBuffer(const YsColor &clearColor)
{
	if(YSTRUE!=FsIsMainWindowActive() || YSTRUE==FsIsMainWindowSplit())
	{
		int x0,y0,wid,hei;
		int mainWid,mainHei;
		FsGetWindowViewport(x0,y0,wid,hei); // x0,y0 is top-left corner.  OpenGL takes bottom-left corner.
		FsGetWindowSize(mainWid,mainHei);
		y0+=hei;  // Now y0 is bottom of the view port.

		glScissor(x0,mainHei-y0,wid,hei);
		glEnable(GL_SCISSOR_TEST);
	}

	glClearColor((float)clearColor.Rd(),(float)clearColor.Gd(),(float)clearColor.Bd(),1.0F);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

	if(YSTRUE!=FsIsMainWindowActive() || YSTRUE==FsIsMainWindowSplit())
	{
		glDisable(GL_SCISSOR_TEST);
	}

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsClearScreenAndZBuffer");
#endif

//#ifdef WIN32
//	frmClock1=frmClock2;
//	QueryPerformanceCounter(&frmClock2);
//	printf("* %d\n",frmClock2.LowPart-frmClock1.LowPart);
//#endif
}

void FsClearStencilBuffer(void)
{
	glClear(GL_STENCIL_BUFFER_BIT);
}

static void FsResetMaterial(void)
{
	float zero[4]={0,0,0,0};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,zero);
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,zero);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,zero);

	int shininess[1]={0};
	glMaterialiv(GL_FRONT_AND_BACK,GL_SHININESS,shininess);
}

void FsSetPointLight(const YsVec3 &cameraPosition,const YsVec3 &lightPosition,FSENVIRONMENT env)
{
	float light[4];
	light[0]=(float)lightPosition.x();
	light[1]=(float)lightPosition.y();
	light[2]=(float)lightPosition.z();
	light[3]=1.0;

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetPointLight In ");
#endif

	glLightfv(GL_LIGHT0,GL_POSITION,light);

	GLfloat dif[4];
	GLfloat amb[4];
	GLfloat spc[4];

	switch(env)
	{
	case FSDAYLIGHT:
		dif[0]=0.6F;
		dif[1]=0.6F;
		dif[2]=0.6F;
		dif[3]=1.0F;

		amb[0]=0.3F;
		amb[1]=0.3F;
		amb[2]=0.3F;
		amb[3]=1.0F;

		spc[0]=0.9F;
		spc[1]=0.9F;
		spc[2]=0.9F;
		spc[3]=1.0F;
		break;
	case FSNIGHT:
		dif[0]=0.05F;
		dif[1]=0.05F;
		dif[2]=0.05F;
		dif[3]=1.0F;

		amb[0]=0.05F;
		amb[1]=0.05F;
		amb[2]=0.05F;
		amb[3]=1.0F;

		spc[0]=0.0F;
		spc[1]=0.0F;
		spc[2]=0.0F;
		spc[3]=1.0F;
		break;
	}

	glLightfv(GL_LIGHT0,GL_DIFFUSE,dif);
	glLightfv(GL_LIGHT0,GL_AMBIENT,amb);
	glLightfv(GL_LIGHT0,GL_SPECULAR,spc);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetPointLight Out");
#endif

	FsResetMaterial();
}

void FsSetDirectionalLight(const YsVec3 &cameraPosition,const YsVec3 &lightDirection,FSENVIRONMENT env)
{
	float light[4];

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetDirectionalLight In");
#endif

	light[0]=(float)lightDirection.x();
	light[1]=(float)lightDirection.y();
	light[2]=(float)lightDirection.z();
	light[3]=0.0;
	glLightfv(GL_LIGHT0,GL_POSITION,light);

	GLfloat dif[4];
	GLfloat amb[4];
	GLfloat spc[4];

	switch(env)
	{
	case FSDAYLIGHT:
		dif[0]=0.6F;
		dif[1]=0.6F;
		dif[2]=0.6F;
		dif[3]=1.0F;

		amb[0]=0.3F;
		amb[1]=0.3F;
		amb[2]=0.3F;
		amb[3]=1.0F;

		spc[0]=0.9F;
		spc[1]=0.9F;
		spc[2]=0.9F;
		spc[3]=1.0F;
		break;
	case FSNIGHT:
		dif[0]=0.05F;
		dif[1]=0.05F;
		dif[2]=0.05F;
		dif[3]=1.0F;

		amb[0]=0.05F;
		amb[1]=0.05F;
		amb[2]=0.05F;
		amb[3]=1.0F;

		spc[0]=0.0F;
		spc[1]=0.0F;
		spc[2]=0.0F;
		spc[3]=1.0F;
		break;
	}

	glLightfv(GL_LIGHT0,GL_DIFFUSE,dif);
	glLightfv(GL_LIGHT0,GL_AMBIENT,amb);
	glLightfv(GL_LIGHT0,GL_SPECULAR,spc);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetDirectionalLight Out");
#endif

	FsResetMaterial();
}

void FsFogOn(const YsColor &col,const double &visibility)
{
	float fogColor[]={0.7F,0.7F,0.7F,0.0F};

	fogColor[0]=(float)col.Rd();
	fogColor[1]=(float)col.Gd();
	fogColor[2]=(float)col.Bd();

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsFogOn In");
#endif

	glEnable(GL_FOG);
	glFogf(GL_FOG_MODE,GL_EXP2);
	glFogf(GL_FOG_DENSITY,1.0F/(float)visibility);
	glFogfv(GL_FOG_COLOR,fogColor);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsFogOn Out");
#endif
}

void FsFogOff(void)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsFogOff In");
#endif

	glDisable(GL_FOG);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsFogOff Out");
#endif
}

static void FsSetupViewport(void)
{
	int x0,y0,wid,hei;
	int mainWid,mainHei;
	FsGetWindowViewport(x0,y0,wid,hei); // x0,y0 is top-left corner.  OpenGL takes bottom-left corner.
	FsGetWindowSize(mainWid,mainHei);
	y0+=hei;  // Now y0 is bottom of the view port.
	glViewport(x0,mainHei-y0,wid,hei);
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
//	int viewport[4],wid,hei;
//	glGetIntegerv(GL_VIEWPORT,viewport);
//	wid=viewport[2];
//	hei=viewport[3];

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetSceneProjection In");
#endif

	FsSetupViewport();
	int xx0,yy0,wid,hei;
	FsGetWindowViewport(xx0,yy0,wid,hei); // x0,y0 is top-left corner.  OpenGL takes bottom-left corner.

	double lft,rit,top,btm;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	lft=(double)(   -prj.cx)*prj.nearz/prj.prjPlnDist;
	rit=(double)(wid-prj.cx)*prj.nearz/prj.prjPlnDist;
	top=(double)(    prj.cy)*prj.nearz/prj.prjPlnDist;
	btm=(double)(prj.cy-hei)*prj.nearz/prj.prjPlnDist;
	glFrustum(lft,rit,btm,top,prj.nearz,prj.farz);

	glMatrixMode(GL_MODELVIEW);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


	// Anti-aliasing options >>
	/* 
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
	*/
	// Anti-aliasing options <<



#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetSceneProjection Out");
#endif
}

void FsSet2DDrawing(void)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSet2DDrawing In");
#endif

	FsSetupViewport();

	int viewport[4],wid,hei;
	glGetIntegerv(GL_VIEWPORT,viewport);
	wid=viewport[2];
	hei=viewport[3];

	glDepthFunc(GL_ALWAYS);
	glDepthMask(0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

#ifdef GLX
	glOrtho(0,(GLdouble)wid,(GLdouble)hei,0,-1,1);
#else
	glOrtho(-0.5,(GLdouble)wid-0.5,(GLdouble)hei-0.5,-0.5,-1,1);
#endif

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_CULL_FACE);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSet2DDrawing Out");
#endif
}

void FsBeginDrawShadow(void)  // Set polygon offset -1,-1 and enable.
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-3,-3);
}

void FsEndDrawShadow(void)    // Disable polygon offset.
{
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void FsSetCameraPosition(const YsVec3 &pos,const YsAtt3 &att,YSBOOL zClear)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetCameraPosition In");
#endif

	glDepthFunc(GL_LEQUAL);
	glDepthMask(~0);

	if(zClear==YSTRUE)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glScaled(1.0,1.0,-1.0);
	glRotated(-YsRadToDeg(att.b()),0.0,0.0,1.0);
	glRotated( YsRadToDeg(att.p()),1.0,0.0,0.0);
	glRotated( YsRadToDeg(att.h()),0.0,1.0,0.0);
	glTranslated(-pos.x(),-pos.y(),-pos.z());

	float zero[4]={0,0,0,0};

	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	// glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
	// glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,zero);
	glDisable(GL_CULL_FACE);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetCameraPosition Out");
#endif
}

void FsFlushScene(void)
{
	// This function is for compabitiliby with Blue Impulse 3DG-SDK only.
	// Nothing to do here.
	glFlush();
}

void FsDrawString(int x,int y,const char str[],const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawString In");
#endif

	fsDirectFixedFontRenderer.RenderAsciiString(x,y,str,col);

	// glColor3d(col.Rd(),col.Gd(),col.Bd());
	// glRasterPos2i(x,y);

	// FsGlSetListBase(FS_GL_FONT_BITMAP_BASE);

	// glCallLists(strlen(str),GL_UNSIGNED_BYTE,str);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawString Out");
#endif
}

void FsDrawLine(int x1,int y1,int x2,int y2,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawLine In");
#endif

	glColor3d(col.Rd(),col.Gd(),col.Bd());
	glBegin(GL_LINES);
	glVertex2i(x1,y1);
	glVertex2i(x2,y2);
	glEnd();

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawLine Out");
#endif
}

void FsDrawRect(int x1,int y1,int x2,int y2,const YsColor &col,YSBOOL fill)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawRect In");
#endif

	glColor3d(col.Rd(),col.Gd(),col.Bd());
	if(fill==YSTRUE)
	{
		glBegin(GL_QUADS);
		glVertex2i(x1,y1);
		glVertex2i(x2,y1);
		glVertex2i(x2,y2);
		glVertex2i(x1,y2);
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_LOOP);
		glVertex2i(x1,y1);
		glVertex2i(x2,y1);
		glVertex2i(x2,y2);
		glVertex2i(x1,y2);
		glEnd();

		glBegin(GL_POINTS);
		glVertex2i(x1,y1);
		glVertex2i(x2,y1);
		glVertex2i(x2,y2);
		glVertex2i(x1,y2);
		glEnd();
	}

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawRect Out");
#endif
}

void FsDrawCircle(int x,int y,int rad,const YsColor &col,YSBOOL fill)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawCircle In");
#endif

	int i;
	static YsArray <YsVec2> circle;

	glColor3d(col.Rd(),col.Gd(),col.Bd());

	if(circle.GetN()<65)
	{
		circle.Set(65,NULL);
		for(i=0; i<=64; i++)
		{
			double a;
			a=YsPi*((double)i)/32.0;
			circle[i].Set(cos(a),sin(a));
		}
	}

	glPushMatrix();
	glTranslatef((float)x,(float)y,0);
	glScalef((float)rad,(float)rad,(float)rad);

	if(fill==YSTRUE)
	{
		glBegin(GL_POLYGON);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}

	for(i=0; i<64; i++)
	{
		glVertex2dv(circle[i]);
	}

	glEnd();
	glPopMatrix();

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawCircle Out");
#endif
}

void FsDrawPolygon(int n,int plg[],const YsColor &col)
{
	int i;

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPolygon In");
#endif

	glColor3d(col.Rd(),col.Gd(),col.Bd());
	glBegin(GL_POLYGON);
	for(i=0; i<n; i++)
	{
		glVertex2i(plg[i*2],plg[i*2+1]);
	}
	glEnd();

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPolygon Out");
#endif
}

void FsDrawDiamond(int x,int y,int r,const YsColor &col,YSBOOL fill)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawDiamond In");
#endif

	glColor3d(col.Rd(),col.Gd(),col.Bd());
	if(fill==YSTRUE)
	{
		glBegin(GL_QUADS);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}

	glVertex2i(x+r,y  );
	glVertex2i(x  ,y+r);
	glVertex2i(x-r,y  );
	glVertex2i(x  ,y-r);
	glEnd();

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawDiamond Out");
#endif
}

void FsDrawX(int x,int y,int r,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawX In");
#endif

	glColor3d(col.Rd(),col.Gd(),col.Bd());

	glBegin(GL_LINES);
	glVertex2i(x-r,y-r);
	glVertex2i(x+r,y+r);

	glVertex2i(x+r,y-r);
	glVertex2i(x-r,y+r);
	glEnd();

	glBegin(GL_POINTS);  // OpenGL's bug: One of the end points is not drawn.
	glVertex2i(x-r,y-r);
	glVertex2i(x+r,y+r);

	glVertex2i(x+r,y-r);
	glVertex2i(x-r,y+r);
	glEnd();
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawX Out");
#endif
}

void FsDrawPoint(int x,int y,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint In");
#endif

	glColor3d(col.Rd(),col.Gd(),col.Bd());

	glBegin(GL_POINTS);
	glVertex2i(x,y);
	glEnd();

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint Out");
#endif
}

void FsDrawPoint2Pix(int x,int y,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint2Pix In");
#endif

	glColor3d(col.Rd(),col.Gd(),col.Bd());

	glBegin(GL_POINTS);
	glVertex2i(x  ,y  );
	glVertex2i(x+1,y  );
	glVertex2i(x+1,y+1);
	glVertex2i(x  ,y+1);
	glEnd();

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint2Pix Out");
#endif
}

void FsDrawTitleBmp(const YsBitmap &bmp,YSBOOL tile)
{
	int wid,hei;
	//float zoomFactor;

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawTitleBmp In");
#endif

	FsGetWindowSize(wid,hei);

	// zoomFactor=float(wid)/float(bmp.GetWidth());
	// glPixelZoom(zoomFactor,zoomFactor);

	int x,y,bmpHei,bmpWid;
	bmpWid=bmp.GetWidth();
	bmpHei=bmp.GetHeight();

	if(tile==YSTRUE)
	{
		for(x=0; x<wid; x+=bmpWid)
		{
			for(y=hei-1; y>0; y-=bmpHei)
			{
				glRasterPos2i(x,y);
				glDrawPixels(bmpWid,bmpHei,GL_RGBA,GL_UNSIGNED_BYTE,bmp.GetRGBABitmapPointer());
			}
		}
	}
	else
	{
		glRasterPos2i(wid-bmpWid,hei-1);
		glDrawPixels(bmpWid,bmpHei,GL_RGBA,GL_UNSIGNED_BYTE,bmp.GetRGBABitmapPointer());
	}

	glFlush();
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawTitleBmp Out");
#endif
}

void FsDrawBmp(const YsBitmap &bmp,int x,int y)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawBmp In");
#endif

	int bmpHei,bmpWid;
	bmpWid=bmp.GetWidth();
	bmpHei=bmp.GetHeight();

	glRasterPos2i(x,y+bmpHei-1);
	glDrawPixels(bmpWid,bmpHei,GL_RGBA,GL_UNSIGNED_BYTE,bmp.GetRGBABitmapPointer());

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawBmp Out");
#endif
}

void FsDrawLine3d(const YsVec3 &p1,const YsVec3 &p2,const YsColor &col)
{
	glColor3d(col.Rd(),col.Gd(),col.Bd());

	glBegin(GL_LINES);
	glVertex3dv(p1);
	glVertex3dv(p2);
	glEnd();
}


void FsDrawMask(const YsColor &fgCol,const YsColor &bgCol,int x0,int y0,int wid,int hei)
{
	if(fgCol.Ri()>200 && fgCol.Gi()>200 && fgCol.Bi()>200)
	{
		glColor4d(0.2,0.2,0.2,0.8);
	}
	else
	{
		glColor4d(0.8,0.8,0.8,0.8);
	}

	glBegin(GL_QUADS);
	glVertex2i(x0    ,y0);
	glVertex2i(x0+wid,y0);
	glVertex2i(x0+wid,y0+hei);
	glVertex2i(x0    ,y0+hei);
	glEnd();
}

void FsSetClipRect(int x0,int y0,int wid,int hei)
{
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);

	glScissor(x0,viewport[3]-y0-hei,wid,hei+1);

	glEnable(GL_SCISSOR_TEST);
}

void FsClearClipRect(void)
{
	glDisable(GL_SCISSOR_TEST);
}

void FsGraphicsTest(int i)
{
}

