#include <ysclass.h>

#include "ysunitconv.h"

#include "fs.h"
#include "graphics/common/fsopengl.h"

#include "fsinstpanel.h"
#include "fswirefont.h"


#include <ysgl.h>
#include <ysglslcpp.h>

#include "fsgl2.0util.h"

void FsInstrumentPanel::EndDraw3d(void)
{
	auto viewPos=viewPosCache;
	auto localViewPos=localViewPosCache;
	auto &prop=*airPropCache;

	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_ALWAYS);


	YsMatrix4x4 airRot;
	GLfloat airRotOpenGL[16];
	airRot.Rotate(prop.GetAttitude());
	airRot.GetOpenGlCompatibleMatrix(airRotOpenGL);

	YsVec3 instPanelOffset;
	prop.GetInstPanelPos(instPanelOffset);
	instPanelOffset-=localViewPos;

	const YsAtt3 &instAtt=prop.GetInstPanelAtt();



	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat prevTfm[16];
	YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);

	GLfloat tfm[16];
	YsGLCopyMatrixfv(tfm,prevTfm);

	YsGLMultMatrixTranslationfv(tfm,(GLfloat)viewPos.x(),(GLfloat)viewPos.y(),(GLfloat)viewPos.z());
	YsGLMultMatrixfv(tfm,tfm,airRotOpenGL);

	if(drawCrossHair==YSTRUE)
	{
		YsGLSLSet3DRendererModelViewfv(renderer,tfm);
		DrawCrossHair3d();
	}

	YsGLMultMatrixTranslationfv(tfm,(GLfloat)instPanelOffset.x(),(GLfloat)instPanelOffset.y(),(GLfloat)instPanelOffset.z());
	YsGLMultMatrixScalingfv(tfm,(GLfloat)prop.GetInstPanelScaling(),(GLfloat)prop.GetInstPanelScaling(),(GLfloat)prop.GetInstPanelScaling());

	YsGLMultMatrixRotationXZfv(tfm,(GLfloat)instAtt.h());
	YsGLMultMatrixRotationZYfv(tfm,(GLfloat)instAtt.p());
	YsGLMultMatrixRotationXYfv(tfm,(GLfloat)instAtt.b());
	YsGLSLSet3DRendererModelViewfv(renderer,tfm);

	YsGLSLEndUse3DRenderer(renderer);



	{
		YsGLSLPlain3DRenderer renderer;
		renderer.SetTextureType(YSGLSL_TEX_TYPE_NONE);
		renderer.DrawVtxCol(YsGL::TRIANGLES,triVtxBuf.GetN(),triVtxBuf,triColBuf);
		renderer.DrawVtxCol(YsGL::LINES,lineVtxBuf.GetN(),lineVtxBuf,lineColBuf);
		renderer.DrawVtxCol(YsGL::POINTS,pointVtxBuf.GetN(),pointVtxBuf,pointColBuf);

		renderer.DrawVtxCol(YsGL::TRIANGLES,ovTriVtxBuf.GetN(),ovTriVtxBuf,ovTriColBuf);
		renderer.DrawVtxCol(YsGL::LINES,ovLineVtxBuf.GetN(),ovLineVtxBuf,ovLineColBuf);
	}



	YsGLSLUse3DRenderer(renderer);
	YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);
	YsGLSLEndUse3DRenderer(renderer);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
}


////////////////////////////////////////////////////////////


void FsInstrumentPanel::DrawCrossHair3d(void)  // This function is called from BeginDraw3d
{
	GLfloat color[4]={1.0f,1.0f,1.0f,1.0f};

	GLfloat crsHairVtx[8*3]=
	{
		-0.01f,0.0f,0.5f,
		-0.03f,0.0f,0.5f,
		 0.01f,0.0f,0.5f,
		 0.03f,0.0f,0.5f,

		 0.0f,-0.01f,0.5f,
		 0.0f,-0.03f,0.5f,
		 0.0f, 0.01f,0.5f,
		 0.0f, 0.03f,0.5f
	};
	YsGLSLSet3DRendererUniformColorfv(YsGLSLSharedFlat3DRenderer(),color);
	YsGLSLDrawPrimitiveVtxfv(YsGLSLSharedFlat3DRenderer(),GL_LINES,8,crsHairVtx);
}

