#include <ysclass.h>
#include <yscompilerwarning.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fs.h"
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#include "graphics/common/fsfontrenderer.h"

#include <ysbitmap.h>

#include <ysgl.h>
#include <ysglcpp.h>
#include <ysglslcpp.h>
#include <ysglsldrawfontbitmap.h>

#include "fsopengl2.0.h"

#include <time.h>


#ifdef _WIN32
#include <windows.h>
#endif


// #define YSOGLERRORCHECK

extern const char *FsProgramTitle;  // Defined in fsmain.cpp

const char *FsMainWindowTitle(void)
{
	// As of 2014/12/12  Main window title must include keyword "Main Window" so that FsWin32GetMainWindowHandle can find the handle.

	static YsString windowTitle;
	windowTitle.Set(FsProgramTitle);
	windowTitle.Append(" Main Window");
	windowTitle.Append(" (OpenGL 2.0 / ES 2.0)");
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
	return YSTRUE;
}

YSBOOL FsIsPointSpriteAvailable(void)
{
	return YSTRUE;
}


#ifdef WIN32
static LARGE_INTEGER frmClock1,frmClock2;
#endif

extern GLuint ysScnGlRwLightTex;
extern GLuint ysScnGlMapTex;



const int fsMaxNumMaskTexture=10;

int fsNumExplosionTex=0;
GLuint fsExplosionTex[fsMaxNumMaskTexture];

int fsNumCloudParticleTex=0;
GLuint fsCloudParticleTex[fsMaxNumMaskTexture];
// 2012/01/02 Cloud Lighting is currently disabled due to performance issue. >>
// Nonetheless, cloudParticleTexSrc and cloudParticleTexBuf are set up anyway.
FsGL2Bitmap cloudParticleTexSrc[fsMaxNumMaskTexture];
FsGL2Bitmap cloudParticleTexBuf[fsMaxNumMaskTexture];
// 2012/01/02 Cloud Lighting is currently disabled due to performance issue. <<

int fsNumFlashTex=0;
GLuint fsFlashTex[fsMaxNumMaskTexture];

int fsNumCloudTex=0;
GLuint fsCloudTex[fsMaxNumMaskTexture];

GLuint fsParticleTexture=0;





double fsOpenGLVersion=1.0;

static void FsSetBmpTexture(GLuint texId,const YsBitmap &bmp,YSBOOL repeat)
{
	glBindTexture(GL_TEXTURE_2D,texId);

	// glPixelStorei(GL_UNPACK_ALIGNMENT,1);  Do I need it?
	if(YSTRUE==repeat)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	}
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glTexImage2D
	    (GL_TEXTURE_2D,
	     0,
	     4,
	     bmp.GetWidth(),
	     bmp.GetHeight(),
	     0,
	     GL_RGBA,
	     GL_UNSIGNED_BYTE,
	     bmp.GetRGBABitmapPointer());
}

static void FsMakeAlphaMask(YsBitmap &bmp)
{
	const int nPix=bmp.GetWidth()*bmp.GetHeight();
	unsigned char *rgba=bmp.GetEditableRGBABitmapPointer();
	for(int i=0; i<nPix; ++i)
	{
		rgba[i*4+3]=rgba[i*4];

		rgba[i*4  ]=255;
		rgba[i*4+1]=255;
		rgba[i*4+2]=255;
	}
}

void FsInitializeOpenGL(void)
{
	const char *verStr=(const char *)glGetString(GL_VERSION);
	if(NULL!=verStr)
	{
		fsOpenGLVersion=atof(verStr);
#ifdef _WIN32
		if(1.999999>fsOpenGLVersion)
		{
			YsString sysErrorMsg;
			sysErrorMsg.Printf(
			   "Newer OpenGL version required.\n"
			   "Required Version:     2.0\n"
			   "Your device supports: %.1lf\n"
			   "Please use YSFLIGHT on OpenGL 1.X instead.",fsOpenGLVersion);
			MessageBoxA(NULL,sysErrorMsg,"Newer OpenGL version required.",MB_OK);
			exit(1);
		}
#endif
	}

	YsBitmap bmp;

	ysScnGlRwLightTex=~(unsigned int)0;
	ysScnGlMapTex=~(unsigned int)0;

	if(bmp.LoadPng(FS_TEXTURE_RWLIGHT)==YSOK)
	{
		glGenTextures(1,&ysScnGlRwLightTex);
		FsSetBmpTexture(ysScnGlRwLightTex,bmp,YSFALSE);
	}

	if(bmp.LoadPng(FS_TEXTURE_GROUNDTILE)==YSOK)
	{
		glGenTextures(1,&ysScnGlMapTex);
		FsSetBmpTexture(ysScnGlMapTex,bmp,YSTRUE);
	}

	glGenTextures(fsMaxNumMaskTexture,fsExplosionTex);
	glGenTextures(fsMaxNumMaskTexture,fsCloudParticleTex);
	glGenTextures(fsMaxNumMaskTexture,fsFlashTex);
	glGenTextures(fsMaxNumMaskTexture,fsCloudTex);
	glGenTextures(1,&fsParticleTexture);

	fsNumExplosionTex=0;
	fsNumCloudParticleTex=0;

	for(int i=1; i<=fsMaxNumMaskTexture; ++i)
	{
		YsString fn;
		fn.Printf("misc/explosion%02d.png",i);
		if(bmp.LoadPng(fn)==YSOK)
		{
			FsMakeAlphaMask(bmp);
			FsSetBmpTexture(fsExplosionTex[fsNumExplosionTex++],bmp,YSFALSE);

			FsSetBmpTexture(fsCloudParticleTex[fsNumCloudParticleTex],bmp,YSFALSE);
			cloudParticleTexSrc[fsNumCloudParticleTex].MakeFromYsBitmap(bmp);
			fsNumCloudParticleTex++;
		}

		fn.Printf("misc/flash%02d.png",i);
		if(bmp.LoadPng(fn)==YSOK)
		{
			FsMakeAlphaMask(bmp);
			FsSetBmpTexture(fsFlashTex[fsNumFlashTex++],bmp,YSFALSE);
		}

		fn.Printf("misc/cloud%02d.png",i);
		if(bmp.LoadPng(fn)==YSOK)
		{
			FsMakeAlphaMask(bmp);
			FsSetBmpTexture(fsCloudTex[fsNumCloudTex++],bmp,YSTRUE);
		}
	}

	if(YSOK==bmp.LoadPng("misc/particle01.png"))
	{
		FsSetBmpTexture(fsParticleTexture,bmp,YSFALSE);
	}

	// FsGlMakeWireFontList();

	// 2010/03/02
	// Polygon offset no longer needed.  Lights are now drawn as textured polygon.  Not really apoint.
	// Therefore, polygon offset doesn't matter.
	// glEnable(GL_POLYGON_OFFSET_FILL);
	// glPolygonOffset(1,1);

	int sampleBuffers;
	glGetIntegerv(GL_SAMPLE_BUFFERS_ARB, &sampleBuffers);
	printf("SAMPLE_BUFFERS_ARB = %d\n", sampleBuffers);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS,0,0);

	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

	YsGLSLCreateSharedRenderer();
	YsGLSLSetShared3DRendererAlphaCutOff(0.02f);

	YsGLSLSetPerPixRendering(YSTRUE); 
	YsGLSLSetShared3DRendererSpecularExponent(600.0f);

	YsGLSLCreateSharedBitmapFontRenderer();
}

void FsReinitializeOpenGL(void)
{
	YsGLSLDeleteSharedRenderer();
	YsGLSLDeleteSharedBitmapFontRenderer();
	FsInitializeOpenGL();
}

void FsUninitializeOpenGL(void)
{
	glDeleteTextures(1,&ysScnGlRwLightTex);
	glDeleteTextures(1,&ysScnGlMapTex);

	glDeleteTextures(fsMaxNumMaskTexture,fsExplosionTex);
	glDeleteTextures(fsMaxNumMaskTexture,fsFlashTex);

	YsGLSLDeleteSharedBitmapFontRenderer();
	YsGLSLDeleteSharedRenderer();
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

	// Why forced resetting is needed?
	//   List base is reset back to zero when OpenGL context is re-made.
	//   OpenGL context is re-made when the window is maximized, minimized or re-sized.

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

void FsSetPointLight(const YsVec3 &/*cameraPosition*/ ,const YsVec3 &lightPosition,FSENVIRONMENT env)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetPointLight In ");
#endif

	/* const GLfloat light[4]=
	{
		(GLfloat)lightPosition.x(),
		(GLfloat)lightPosition.y(),
		(GLfloat)lightPosition.z(),
		1.0f
	}; */

	YsDisregardVariable(lightPosition);
	YsDisregardVariable(env);
	// Not supported yet.

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetPointLight Out");
#endif
}

void FsSetDirectionalLight(const YsVec3 &/*cameraPosition*/ ,const YsVec3 &lightDirection,FSENVIRONMENT env)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetDirectionalLight In");
#endif

	const GLfloat light[4]=
	{
		(GLfloat)lightDirection.x(),
		(GLfloat)lightDirection.y(),
		(GLfloat)lightDirection.z(),
		0.0f
	};

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

	YsGLSLSetShared3DRendererDirectionalLightfv(0,light);
	YsGLSLSetShared3DRendererLightColor(0,dif);
	YsGLSLSetShared3DRendererAmbientColor(amb);
	YsGLSLSetShared3DRendererSpecularColor(spc);

	YsGLSLUse3DRenderer(YsGLSLSharedFlash3DRenderer());
	switch(env)
	{
	case FSDAYLIGHT:
		YsGLSLSet3DRendererUniformFlashSize(YsGLSLSharedFlash3DRenderer(),0.1f);
		YsGLSLSet3DRendererFlashRadius(YsGLSLSharedFlash3DRenderer(),0.6f,1.0f);
		break;
	case FSNIGHT:
		YsGLSLSet3DRendererUniformFlashSize(YsGLSLSharedFlash3DRenderer(),1.0f);
		YsGLSLSet3DRendererFlashRadius(YsGLSLSharedFlash3DRenderer(),0.2f,1.0f);
		break;
	}
	YsGLSLEndUse3DRenderer(YsGLSLSharedFlash3DRenderer());


#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetDirectionalLight Out");
#endif
}

void FsFogOn(const YsColor &col,const double &visibility)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsFogOn In");
#endif

	const GLfloat fogColor[]=
	{
		(GLfloat)col.Rf(),
		(GLfloat)col.Gf(),
		(GLfloat)col.Bf(),
		1.0f
	};

	// From GLSL comment
	// f  0:Completely fogged out   1:Clear
	// f=e^(-d*d)
	// d  0:Clear      Infinity: Completely fogged out
	// 99% fogged out means:  e^(-d*d)=0.01  Whatfs d?
	// -d*d=loge(0.01)
	// -d*d= -4.60517
	// d=2.146
	// If visibility=V, d=2.146 at fogZ=V -> fogDensity=2.146/V

	YsGLSLSetShared3DRendererFog(1,2.146/(GLfloat)visibility,fogColor);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsFogOn Out");
#endif
}

void FsFogOff(void)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsFogOff In");
#endif

	const float fogColor[]={0.7F,0.7F,0.7F,0.0F};
	YsGLSLSetShared3DRendererFog(0,1.0,fogColor);

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
	glViewport(0,0,texWid,texHei);

	GLfloat projMat[16];
	projTfm.GetOpenGlCompatibleMatrix(projMat);

	YsMatrix4x4 viewTfmLH;
	viewTfmLH.ScaleZ(-1.0);
	viewTfmLH*=viewTfm;

	GLfloat viewMatLH[16];
	viewTfmLH.GetOpenGlCompatibleMatrix(viewMatLH);

	YsGLSLSetShared3DRendererProjection(projMat);
	YsGLSLSetShared3DRendererModelView(viewMatLH);

	{
		auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
		YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
		YsGLSLSetBitmapFontRendererProjectionfv(fsBitmapFontRenderer,projMat);
		YsGLSLSetBitmapFontRendererModelViewfv(fsBitmapFontRenderer,viewMatLH);
		YsGLSLSetBitmapFontRendererViewportSize(fsBitmapFontRenderer,texWid,texHei);
		YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);
	}

	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE); // Man!  It took several days to find this single line was missing!
	glDepthFunc(GL_LEQUAL);  // Actually, two lines.
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
}

void FsEndRenderShadowMap(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER,0);
}

static void SwitchTexture(int texIdent)
{
	switch(texIdent)
	{
	case 0:
		glActiveTexture(GL_TEXTURE0);
		break;
	case 1:
		glActiveTexture(GL_TEXTURE1);
		break;
	case 2:
		glActiveTexture(GL_TEXTURE2);
		break;
	case 3:
		glActiveTexture(GL_TEXTURE3);
		break;
	case 4:
		glActiveTexture(GL_TEXTURE4);
		break;
	case 5:
		glActiveTexture(GL_TEXTURE5);
		break;
	case 6:
		glActiveTexture(GL_TEXTURE6);
		break;
	case 7:
		glActiveTexture(GL_TEXTURE7);
		break;
	case 8:
		glActiveTexture(GL_TEXTURE8);
		break;
	case 9:
		glActiveTexture(GL_TEXTURE9);
		break;
	}
}

/*! Shadow-map texture must be bound to the texture-unit samplerIdent before this call.
    Binding is a responsibility of YsTextureManager.

    ShadowMapIdent is an identifier for the renderer {0 or 1}.
*/
void FsEnableShadowMap(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &shadowProjTfm,const YsMatrix4x4 &shadowViewTfm,
    int samplerIdent,int shadowMapIdent)
{
	YsGLSLShaded3DRenderer renderer;

	YsMatrix4x4 viewInv;
	viewInv.ScaleZ(-1.0); // Right->Left Tfm
	viewInv*=viewTfm;
	viewInv.Invert();

	YsMatrix4x4 shadowViewTfmLH;
	shadowViewTfmLH.ScaleZ(-1.0);
	shadowViewTfmLH*=shadowViewTfm;

	YsMatrix4x4 overAllTfm=shadowProjTfm*shadowViewTfmLH*viewInv;
	GLfloat overAllMat[16];
	overAllTfm.GetOpenGlCompatibleMatrix(overAllMat);

	YsGLSLSet3DRendererUniformShadowMapMode(renderer,shadowMapIdent,YSGLSL_SHADOWMAP_USE);
	YsGLSLSet3DRendererUniformShadowMapTexture(renderer,shadowMapIdent,samplerIdent);
	YsGLSLSet3DRendererUniformShadowMapTransformation(renderer,shadowMapIdent,overAllMat);

	YsGLSLSet3DRendererUniformLightDistOffset(renderer,shadowMapIdent,0.0001);
	YsGLSLSet3DRendererUniformLightDistScaling(renderer,shadowMapIdent,1.0001);
}

void FsDisableShadowMap(int samplerIdent,int shadowMapIdent)
{
	YsGLSLShaded3DRenderer renderer;
	YsGLSLSet3DRendererUniformShadowMapMode(renderer,shadowMapIdent,YSGLSL_SHADOWMAP_NONE);

	SwitchTexture(samplerIdent);
	glBindTexture(GL_TEXTURE_2D,0);
}


void FsSetSceneProjection(const class FsProjection &prj)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsSetSceneProjection In");
#endif

	FsSetupViewport();
	int xx0,yy0,wid,hei;
	FsGetWindowViewport(xx0,yy0,wid,hei); // x0,y0 is top-left corner.  OpenGL takes bottom-left corner.

	double lft,rit,top,btm;

	lft=(double)(   -prj.cx)*prj.nearz/prj.prjPlnDist;
	rit=(double)(wid-prj.cx)*prj.nearz/prj.prjPlnDist;
	top=(double)(    prj.cy)*prj.nearz/prj.prjPlnDist;
	btm=(double)(prj.cy-hei)*prj.nearz/prj.prjPlnDist;



	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Anti-aliasing options >>
	/*
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
	*/
	// Anti-aliasing options <<


	GLfloat projMat[16];
	YsGLMakeFrustum(projMat,(GLfloat)lft,(GLfloat)rit,(GLfloat)btm,(GLfloat)top,(GLfloat)prj.nearz,(GLfloat)prj.farz);
	YsGLSLSetShared3DRendererProjection(projMat);

	{
		auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
		YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
		YsGLSLSetBitmapFontRendererProjectionfv(fsBitmapFontRenderer,projMat);
		YsGLSLSetBitmapFontRendererViewportSize(fsBitmapFontRenderer,wid,hei);
		YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);
	}


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


	{
		auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
		YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
		YsGLSLSetBitmapFontRendererViewportSize(fsBitmapFontRenderer,wid,hei);
		YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);
	}

	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);


	glDisable(GL_CULL_FACE);

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
	YsGLSLUseWindowCoordinateInPlain2DDrawing(YsGLSLSharedPlain2DRenderer(),1);
	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

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
	glDepthMask(GL_TRUE);

	if(zClear==YSTRUE)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	glEnable(GL_CULL_FACE);

	YsMatrix4x4 tfm;
	tfm.Scale(1.0,1.0,-1.0);
	tfm.RotateXY(-att.b());
	tfm.RotateZY(-att.p());
	tfm.RotateXZ(-att.h());
	tfm.Translate(-pos);
	GLfloat modelViewMat[16];
	tfm.GetOpenGlCompatibleMatrix(modelViewMat);
	YsGLSLSetShared3DRendererModelView(modelViewMat);

	auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
	YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
	YsGLSLSetBitmapFontRendererModelViewfv(fsBitmapFontRenderer,modelViewMat);
	YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);

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

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawString Out");
#endif
}

void FsDrawLine(int x1,int y1,int x2,int y2,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawLine In");
#endif

	const GLfloat vtx[2*2]=
	{
		(GLfloat)x1,
		(GLfloat)y1,
		(GLfloat)x2,
		(GLfloat)y2,
	};
	const GLfloat color[4*2]=
	{
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
	};

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);
	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINES,2,vtx,color);
	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawLine Out");
#endif
}

void FsDrawRect(int x1,int y1,int x2,int y2,const YsColor &col,YSBOOL fill)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawRect In");
#endif

	const GLfloat vtx[2*4]=
	{
		(GLfloat)x1,(GLfloat)y1,
		(GLfloat)x2,(GLfloat)y1,
		(GLfloat)x2,(GLfloat)y2,
		(GLfloat)x1,(GLfloat)y2
	};
	const GLfloat color[4*4]=
	{
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f
	};

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);

	if(fill==YSTRUE)
	{
		YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,color);
	}
	else
	{
		YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINE_LOOP,4,vtx,color);
	}

	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawRect Out");
#endif
}

void FsDrawCircle(int x,int y,int rad,const YsColor &col,YSBOOL fill)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawCircle In");
#endif

	static YsArray <GLfloat> vtx,color;

	if(vtx.GetN()<128)
	{
		vtx.Set(128,NULL);
	}
	if(color.GetN()<256)
	{
		color.Set(256,NULL);
	}

	for(int i=0; i<64; i++)
	{
		const GLfloat a=(GLfloat)YsPi*((GLfloat)i)/32.0f;
		vtx[i*2  ]=(GLfloat)x+(GLfloat)rad*cosf(a);
		vtx[i*2+1]=(GLfloat)y+(GLfloat)rad*sinf(a);
		color[i*4  ]=col.Rf();
		color[i*4+1]=col.Gf();
		color[i*4+2]=col.Bf();
		color[i*4+3]=1.0f;
	}

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);
	if(fill==YSTRUE)
	{
		YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,64,vtx,color);
	}
	else
	{
		YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINE_LOOP,64,vtx,color);
	}
	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawCircle Out");
#endif
}

void FsDrawPolygon(int n,int plg[],const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPolygon In");
#endif

	YsArray <GLfloat,32> vtx;
	YsArray <GLfloat,64> color;
	vtx.Set(n*2,NULL);
	color.Set(n*4,NULL);
	for(int i=0; i<n; ++i)
	{
		vtx[i*2  ]=(GLfloat)plg[i*2];
		vtx[i*2+1]=(GLfloat)plg[i*2+1];
		color[i*4  ]=col.Rf();
		color[i*4+1]=col.Gf();
		color[i*4+2]=col.Bf();
		color[i*4+3]=1.0f;
	}

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);
	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,n,vtx,color);
	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPolygon Out");
#endif
}

void FsDrawDiamond(int x,int y,int r,const YsColor &col,YSBOOL fill)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawDiamond In");
#endif

	const GLfloat vtx[8]=
	{
		(GLfloat)(x+r),(GLfloat)(y  ),
		(GLfloat)(x  ),(GLfloat)(y+r),
		(GLfloat)(x-r),(GLfloat)(y  ),
		(GLfloat)(x  ),(GLfloat)(y-r)
	};
	const GLfloat color[16]=
	{
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f
	};

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);

	if(fill==YSTRUE)
	{
		YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,color);
	}
	else
	{
		YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINE_LOOP,4,vtx,color);
	}

	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawDiamond Out");
#endif
}

void FsDrawX(int x,int y,int r,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawX In");
#endif

	const GLfloat vtx[8]=
	{
		(GLfloat)(x-r),(GLfloat)(y-r),
		(GLfloat)(x+r),(GLfloat)(y+r),
		(GLfloat)(x+r),(GLfloat)(y-r),
		(GLfloat)(x-r),(GLfloat)(y+r)
	};

	const GLfloat color[16]=
	{
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f
	};

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);

	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_LINES,4,vtx,color);

	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawX Out");
#endif
}

void FsDrawPoint(int x,int y,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint In");
#endif

	const GLfloat vtx[2]=
	{
		(GLfloat)x,(GLfloat)y
	};
	const GLfloat color[4]=
	{
		col.Rf(),col.Gf(),col.Bf(),1.0f
	};

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);
	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_POINTS,1,vtx,color);
	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint Out");
#endif
}

void FsDrawPoint2Pix(int x,int y,const YsColor &col)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint2Pix In");
#endif

	const GLfloat vtx[8]=
	{
		(GLfloat)(x  ),(GLfloat)(y  ),
		(GLfloat)(x+1),(GLfloat)(y  ),
		(GLfloat)(x+1),(GLfloat)(y+1),
		(GLfloat)(x  ),(GLfloat)(y+1)
	};
	const GLfloat color[16]=
	{
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f
	};
	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);
	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_POINTS,4,vtx,color);
	YsGLSLEndUsePlain2DRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawPoint2Pix Out");
#endif
}

void FsDrawTitleBmp(const YsBitmap &bmp,YSBOOL tile)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawTitleBmp In");
#endif
	YsGLSLBitmapRenderer *renderer=YsGLSLSharedBitmapRenderer();
	YsGLSLUseBitmapRenderer(renderer);

	GLuint texId[1];
	glGenTextures(1,texId);

	int x,y;
	const int bmpWid=bmp.GetWidth();
	const int bmpHei=bmp.GetHeight();
	const unsigned char *bmpPtr=bmp.GetRGBABitmapPointer();

	int wid,hei;
	FsGetWindowSize(wid,hei);

	// zoomFactor=float(wid)/float(bmp.GetWidth());
	// glPixelZoom(zoomFactor,zoomFactor);

	GLenum prevActiveTexture;
	GLuint prevBinding2d;

	glGetIntegerv(GL_ACTIVE_TEXTURE,(GLint *)&prevActiveTexture);
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D,(GLint *)&prevBinding2d);

	glBindTexture(GL_TEXTURE_2D,texId[0]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,bmpWid,bmpHei,0,GL_RGBA,GL_UNSIGNED_BYTE,bmpPtr);

	if(tile==YSTRUE)
	{
		for(x=0; x<wid; x+=bmpWid)
		{
			for(y=hei-1; y>0; y-=bmpHei)
			{
				YsGLSLRenderTexture2D(renderer,x,y,YSGLSL_HALIGN_LEFT,YSGLSL_VALIGN_BOTTOM,bmpWid,bmpHei,texId[0]);
			}
		}
	}
	else
	{
		YsGLSLRenderTexture2D(renderer,wid-bmpWid,hei-1,YSGLSL_HALIGN_LEFT,YSGLSL_VALIGN_BOTTOM,bmpWid,bmpHei,texId[0]);
	}

	glBindTexture(GL_TEXTURE_2D,prevBinding2d);
	glActiveTexture(prevActiveTexture);

	glDeleteTextures(1,texId);

	YsGLSLEndUseBitmapRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawTitleBmp Out");
#endif
}

void FsDrawBmp(const YsBitmap &bmp,int x,int y)
{
#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawBmp In");
#endif

	const int bmpWid=bmp.GetWidth();
	const int bmpHei=bmp.GetHeight();
	const unsigned char *rgba=bmp.GetRGBABitmapPointer();

	YsGLSLBitmapRenderer *renderer=YsGLSLSharedBitmapRenderer();
	YsGLSLUseBitmapRenderer(renderer);

	YsGLSLRenderRGBABitmap2D(renderer,x,y,YSGLSL_HALIGN_LEFT,YSGLSL_VALIGN_TOP,bmpWid,bmpHei,rgba);

	YsGLSLEndUseBitmapRenderer(renderer);

#ifdef YSOGLERRORCHECK
	FsOpenGlShowError("FsDrawBmp Out");
#endif
}

void FsDrawLine3d(const YsVec3 &p1,const YsVec3 &p2,const YsColor &col)
{
	const GLfloat vtx[]=
	{
		(GLfloat)p1.x(),
		(GLfloat)p1.y(),
		(GLfloat)p1.z(),
		(GLfloat)p2.x(),
		(GLfloat)p2.y(),
		(GLfloat)p2.z()
	};
	const GLfloat color[4*2]=
	{
		col.Rf(),col.Gf(),col.Bf(),1.0f,
		col.Rf(),col.Gf(),col.Bf(),1.0f,
	};

	auto *renderer=YsGLSLSharedVariColor3DRenderer();
	YsGLSLUse3DRenderer(renderer);
	YsGLSLDrawPrimitiveVtxColfv(renderer,GL_LINES,2,vtx,color);
	YsGLSLEndUse3DRenderer(renderer);
}


void FsDrawMask(const YsColor &fgCol,const YsColor &bgCol,int x0,int y0,int wid,int hei)
{
	YsDisregardVariable(bgCol);

	const GLfloat vtx[8]=
	{
		(GLfloat)(x0    ),(GLfloat)(y0),
		(GLfloat)(x0+wid),(GLfloat)(y0),
		(GLfloat)(x0+wid),(GLfloat)(y0+hei),
		(GLfloat)(x0    ),(GLfloat)(y0+hei)
	};

	GLfloat r,g,b,a;
	if(fgCol.Ri()>200 && fgCol.Gi()>200 && fgCol.Bi()>200)
	{
		r=0.2f;
		g=0.2f;
		b=0.2f;
		a=0.8f;
	}
	else
	{
		r=0.8f;
		g=0.8f;
		b=0.8f;
		a=0.8f;
	}

	const GLfloat color[16]=
	{
		r,g,b,a,
		r,g,b,a,
		r,g,b,a,
		r,g,b,a
	};

	YsGLSLPlain2DRenderer *renderer=YsGLSLSharedPlain2DRenderer();
	YsGLSLUsePlain2DRenderer(renderer);
	YsGLSLDrawPlain2DPrimitivefv(renderer,GL_TRIANGLE_FAN,4,vtx,color);
	YsGLSLEndUsePlain2DRenderer(renderer);
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
	auto bitmapRenderer=YsGLSLSharedBitmapRenderer();
	YsGLSLUseBitmapRenderer(bitmapRenderer);
	YsGLSLRenderTexture2D(bitmapRenderer,0,0,YSGLSL_HALIGN_LEFT,YSGLSL_VALIGN_TOP,256,256,i);
	YsGLSLEndUseBitmapRenderer(bitmapRenderer);
}

