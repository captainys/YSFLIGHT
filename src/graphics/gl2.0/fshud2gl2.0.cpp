#include <ysclass.h>
#include <ysglcpp.h>
#include <ysglslcpp.h>

#include "ysunitconv.h"

#include "graphics/common/fsopengl.h"
#include "fshud2.h"
#include "fsinstreading.h"
#include <ysgl.h>

#include "fswirefont.h"

#include "fsgl2.0util.h"


class FsHud2D3dSpecificResource
{
public:
	GLfloat prevTfm[16];
	YsArray <GLfloat> bankArcVtx,bankHairVtx;
	YsArray <GLfloat> hdgOutVtx,hdgInVtx;
};

void FsHud2::TakeSpecificResource(void)
{
	specificResource=(void *)new FsHud2D3dSpecificResource;
}

void FsHud2::ReleaseSpecificResource(void)
{
	if(NULL!=specificResource)
	{
		FsHud2D3dSpecificResource *res=(FsHud2D3dSpecificResource *)specificResource;
		delete res;
	}
}



void FsHud2::BeginDrawHud(const YsVec3 &viewPos,const YsAtt3 &airAtt)
{
	pointVtxBuf.CleanUp();
	pointColBuf.CleanUp();
	lineVtxBuf.CleanUp();
	lineColBuf.CleanUp();
	triVtxBuf.CleanUp();
	triColBuf.CleanUp();


	FsHud2D3dSpecificResource *specificResource=(FsHud2D3dSpecificResource *)(this->specificResource);



	glDisable(GL_CULL_FACE);  // 2010/12/19 Does this solve missing flaps, throttle, spoiler, etc.?

	this->viewPos=viewPos;
	this->airAtt=airAtt;

	YsMatrix4x4 airRot;
	GLfloat airRotOpenGL[16];
	airRot.Rotate(airAtt);
	airRot.GetOpenGlCompatibleMatrix(airRotOpenGL);



	GLfloat tfm[16];

	YsGLSLUse3DRenderer(YsGLSLSharedFlat3DRenderer());

	if(NULL!=specificResource)
	{
		YsGLSLGet3DRendererModelViewfv(specificResource->prevTfm,YsGLSLSharedFlat3DRenderer());
	}

	const GLfloat color[4]={(GLfloat)hudCol.Rd(),(GLfloat)hudCol.Gd(),(GLfloat)hudCol.Bd(),1.0f};
	YsGLSLSet3DRendererUniformColorfv(YsGLSLSharedFlat3DRenderer(),color);

	YsGLSLGet3DRendererModelViewfv(tfm,YsGLSLSharedFlat3DRenderer());
	YsGLMultMatrixTranslationfv(tfm,(GLfloat)viewPos.x(),(GLfloat)viewPos.y(),(GLfloat)viewPos.z());
	YsGLMultMatrixfv(tfm,tfm,airRotOpenGL);

	YsGLSLSet3DRendererModelViewfv(YsGLSLSharedFlat3DRenderer(),tfm);

	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);
}

void FsHud2::EndDrawHud(void)
{
	YsGLSLEndUse3DRenderer(YsGLSLSharedFlat3DRenderer());


	{
		YsGLSLPlain3DRenderer renderer;
		renderer.SetTextureType(YSGLSL_TEX_TYPE_NONE);
		renderer.DrawVtxCol(YsGL::TRIANGLES,triVtxBuf.GetN(),triVtxBuf,triColBuf);
		renderer.DrawVtxCol(YsGL::LINES,lineVtxBuf.GetN(),lineVtxBuf,lineColBuf);
		renderer.DrawVtxCol(YsGL::POINTS,pointVtxBuf.GetN(),pointVtxBuf,pointColBuf);
	}


	FsHud2D3dSpecificResource *specificResource=(FsHud2D3dSpecificResource *)(this->specificResource);
	if(NULL!=specificResource)
	{
		YsGLSLUse3DRenderer(YsGLSLSharedFlat3DRenderer());
		YsGLSLSet3DRendererModelViewfv(YsGLSLSharedFlat3DRenderer(),specificResource->prevTfm);
		YsGLSLEndUse3DRenderer(YsGLSLSharedFlat3DRenderer());
	}

	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
}



