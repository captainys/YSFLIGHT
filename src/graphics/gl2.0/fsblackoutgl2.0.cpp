#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fsconfig.h"

#include "fs.h"
#include "graphics/common/fsopengl.h"


#include <ysgl.h>
#include <ysglslcpp.h>
#include "fsgl2.0util.h"


#include "fsblackout.h"


void FsSimulation::SimDrawBlackout(const ActualViewMode &actualViewMode) const
{
	const FsAirplane *playerPlane=GetPlayerAirplane();
	const GLfloat plusGLimit=9.0f;
	const GLfloat minusGLimit=-5.0f;

	if(cfgPtr->blackOut==YSTRUE &&
	   actualViewMode.actualViewMode==FSCOCKPITVIEW &&
	   playerPlane!=NULL &&
	   playerPlane->isPlayingRecord!=YSTRUE &&
	   playerPlane->IsAlive()==YSTRUE &&
	   (playerPlane->Prop().GetG()>plusGLimit ||
	    playerPlane->Prop().GetG()<minusGLimit))
	{
		YsGLVertexBuffer2D vtxBuf;
		YsGLColorBuffer colBuf;
		FsMakeBlackOutPolygon(vtxBuf,colBuf,playerPlane->Prop().GetG());

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		YsGLSL2DRenderer renderer;
		renderer.DrawVtxCol(YsGL::TRIANGLES,vtxBuf.GetN(),vtxBuf,colBuf);

		YsString str;
		str.Printf("%.1lfG",playerPlane->Prop().GetG());

		int wid,hei;
		FsGetWindowSize(wid,hei);
		FsDrawString(wid/2-20,hei/2,str,YsRed());

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
}

