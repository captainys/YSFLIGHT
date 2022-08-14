#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fs.h"

#include <ysgl.h>

#include "fsrecord.h"
#include "graphics/common/fsopengl.h"

#include <ysbitmap.h>
#include "fsgl2.0util.h"


//#define YSOGLERRORCHECk
//extern void FsOpenGlShowError(const char tag[]);

class FsAirplaneGraphicCache
{
public:
	FsGL2VariableVertexStorage vtxStore;
};

static FsAirplaneGraphicCache *fsAirplaneGraphicCache=NULL;

static FsAirplaneGraphicCache *FsGetAirplaneGraphicCache(void)
{
	if(NULL==fsAirplaneGraphicCache)
	{
		fsAirplaneGraphicCache=new FsAirplaneGraphicCache;
	}
	return fsAirplaneGraphicCache;
}

////////////////////////////////////////////////////////////

void FsAirplane::DrawVapor(double currentTime,double remainTime,int step,YSBOOL /*transparency*/ ) const
{
#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawVapor In");
#endif

	YsGLVertexBuffer vtxBuf;
	YsGLColorBuffer colBuf;
	this->MakeVaporVertexArray(vtxBuf,colBuf,currentTime,remainTime,step);

	YsGLSL3DRenderer *renderer=YsGLSLSharedVariColor3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	YsGLSLDrawPrimitiveVtxColfv(renderer,GL_LINES,vtxBuf.size(),vtxBuf.data(),colBuf.data());

	YsGLSLEndUse3DRenderer(renderer);

#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawVapor Out");
#endif
}

void FsAirplane::DrawSingleSmoke(int smkId,double currentTime,double remainTime,FSSMOKETYPE smk,int step,YSBOOL /*transparency*/ ) const
{
#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawSmoke In");
#endif

	YsGLVertexBuffer vtxBuf;
	YsGLNormalBuffer nomBuf;
	YsGLColorBuffer colBuf;
	AddSingleSmokeVertexArray(vtxBuf,nomBuf,colBuf,smkId,currentTime,remainTime,smk,step);

	auto renderer=YsGLSLSharedVariColorPerVtxShading3DRenderer();
	glEnable(GL_CULL_FACE);  // Supposed to be always on

	YsGLSLUse3DRenderer(renderer);
	YsGLSLDrawPrimitiveVtxNomColfv(renderer,GL_TRIANGLES,vtxBuf.size(),vtxBuf.data(),nomBuf.data(),colBuf.data());
	YsGLSLEndUse3DRenderer(renderer);

#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsAirplane::DrawSmoke Out");
#endif
}

void FsField::DrawMapVisual
    (FSENVIRONMENT env,
     const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,const double &elvMin,const double &elvMax,YSBOOL drawPset,
     const double &currentTime,
     YSBOOL useOpenGlGroundTexture,YSBOOL useOpenGlRunwayLightTexture) const
{
#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsField::DrawMapVisual In");
#endif


	int wid,hei;
	float pointSize;
	FsGetWindowSize(wid,hei);


	if(env==FSNIGHT)
	{
		YsScenery::lightPointSize3d=0.4f;

		pointSize=(float)wid/1024.0F;  // 1024 pixel -> 2 dots
		pointSize=YsBound(pointSize,1.0F,2.0F);
		//glPointSize(pointSize);
	}
	else
	{
		YsScenery::lightPointSize3d=0.1f;

		//glPointSize(1.0f);
	}

	if(fld!=NULL)
	{
		YsMatrix4x4 viewTfm;
		viewTfm.RotateXY(-viewAtt.b());
		viewTfm.RotateZY(-viewAtt.p());
		viewTfm.RotateXZ(-viewAtt.h());
		viewTfm.Translate(-viewPos);

		fld->pos=pos;
		fld->att=att;
		fld->DrawMapVisual(viewTfm,YsIdentity4x4(),projMat,elvMin,elvMax,drawPset,currentTime);
	}

	// glPointSize(1);

#ifdef YSOGLERRORCHECk
	FsOpenGlShowError("FsField::DrawMapVisual Out");
#endif
}

