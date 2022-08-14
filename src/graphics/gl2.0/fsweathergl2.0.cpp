#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include "fs.h"

#include <ysgl.h>
#include "fsgl2.0util.h"

void FsWeather::DrawCloudLayer(const YsVec3 &cameraPos) const
{
	FsGL2DisableCulling disableCulling;  // Disable in constructor, and re-enable in destructor.

	static FsGL2VariableVertexStorage vtxBuf;
	vtxBuf.CleanUp();

	YsGLSL3DRenderer *renderer=YsGLSLSharedMonoColorPerVtxShading3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat prevTfm[16];
	YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);

	GLfloat tfm[16];
	YsGLCopyMatrixfv(tfm,prevTfm);
	YsGLMultMatrixTranslationfv(tfm,(GLfloat)cameraPos.x(),0.0f,(GLfloat)cameraPos.z());
	YsGLSLSet3DRendererModelViewfv(renderer,tfm);

	const GLfloat cloudColor[4]={0.9f,0.9f,0.9f,1.0f};
	YsGLSLSet3DRendererUniformColorfv(renderer,cloudColor);

	for(int i=0; i<cloudLayer.GetN(); ++i)
	{
		vtxBuf.AddNormal(0.0f,-1.0f,0.0f);
		vtxBuf.AddVertex(-20000.0f,(GLfloat)cloudLayer[i].y0,-20000.0f);
		vtxBuf.AddNormal(0.0f,-1.0f,0.0f);
		vtxBuf.AddVertex(-20000.0f,(GLfloat)cloudLayer[i].y0, 20000.0f);
		vtxBuf.AddNormal(0.0f,-1.0f,0.0f);
		vtxBuf.AddVertex( 20000.0f,(GLfloat)cloudLayer[i].y0, 20000.0f);

		vtxBuf.AddNormal(0.0f,-1.0f,0.0f);
		vtxBuf.AddVertex( 20000.0f,(GLfloat)cloudLayer[i].y0, 20000.0f);
		vtxBuf.AddNormal(0.0f,-1.0f,0.0f);
		vtxBuf.AddVertex( 20000.0f,(GLfloat)cloudLayer[i].y0,-20000.0f);
		vtxBuf.AddNormal(0.0f,-1.0f,0.0f);
		vtxBuf.AddVertex(-20000.0f,(GLfloat)cloudLayer[i].y0,-20000.0f);

		vtxBuf.AddNormal(0.0f,1.0f,0.0f);
		vtxBuf.AddVertex(-20000.0f,(GLfloat)cloudLayer[i].y1,-20000.0f);
		vtxBuf.AddNormal(0.0f,1.0f,0.0f);
		vtxBuf.AddVertex(-20000.0f,(GLfloat)cloudLayer[i].y1, 20000.0f);
		vtxBuf.AddNormal(0.0f,1.0f,0.0f);
		vtxBuf.AddVertex( 20000.0f,(GLfloat)cloudLayer[i].y1, 20000.0f);

		vtxBuf.AddNormal(0.0f,1.0f,0.0f);
		vtxBuf.AddVertex( 20000.0f,(GLfloat)cloudLayer[i].y1, 20000.0f);
		vtxBuf.AddNormal(0.0f,1.0f,0.0f);
		vtxBuf.AddVertex( 20000.0f,(GLfloat)cloudLayer[i].y1,-20000.0f);
		vtxBuf.AddNormal(0.0f,1.0f,0.0f);
		vtxBuf.AddVertex(-20000.0f,(GLfloat)cloudLayer[i].y1,-20000.0f);
	}

	YsGLSLDrawPrimitiveVtxNomfv(renderer,GL_TRIANGLES,vtxBuf.nVtx,vtxBuf.vtxArray,vtxBuf.nomArray);

	YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);
	YsGLSLEndUse3DRenderer(renderer);
}



