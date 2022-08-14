#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fs.h"

#include <windows.h>
#include <mmsystem.h>


#include <ysd3d9.h>
#include "fsd3d.h"

#include "fsrecord.h"

#include <fstexturemanager.h>


extern D3DCAPS9 ysD3dDevCaps;

void FsAirplane::DrawVapor(double currentTime,double remainTime,int step,YSBOOL) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	YsGLVertexBuffer vtxBuf;
	YsGLColorBuffer colBuf;
	this->MakeVaporVertexArray(vtxBuf,colBuf,currentTime,remainTime,step);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	for(YSSIZE_T idx=0; idx<vtxBuf.size(); ++idx)
	{
		ysD3dDev->AddXyzCol(
		    D3DPT_LINELIST,
		    vtxBuf[idx][0],vtxBuf[idx][1],vtxBuf[idx][2],
		    (int)(255.0f*colBuf[idx][0]),(int)(255.0f*colBuf[idx][1]),(int)(255.0f*colBuf[idx][2]),(int)(255.0f*colBuf[idx][3]));
	}

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);  // Must flush before turning off alpha blending.
}

void FsAirplane::DrawSingleSmoke(int smkId,double currentTime,double remainTime,FSSMOKETYPE smk,int step,YSBOOL transparency) const
{
	D3DPRIMITIVETYPE primType;
	YsColor smkCol;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);
	smkCol=Prop().GetSmokeColor(smkId);

	if(smk==FSSMKNULL)
	{
		return;
	}

	YsGLVertexBuffer vtxBuf;
	YsGLNormalBuffer nomBuf;
	YsGLColorBuffer colBuf;
	AddSingleSmokeVertexArray(vtxBuf,nomBuf,colBuf,smkId,currentTime,remainTime,smk,step);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);

	for(YSSIZE_T idx=0; idx<vtxBuf.size(); ++idx)
	{
		ysD3dDev->AddXyzNomCol(
		    D3DPT_TRIANGLELIST,
		    vtxBuf[idx][0],vtxBuf[idx][1],vtxBuf[idx][2],
		    nomBuf[idx][0],nomBuf[idx][1],nomBuf[idx][2],
		    (int)(255.0f*colBuf[idx][0]),(int)(255.0f*colBuf[idx][1]),(int)(255.0f*colBuf[idx][2]),(int)(255.0f*colBuf[idx][3]));
	}
	ysD3dDev->FlushXyzNomCol(D3DPT_TRIANGLELIST);

	float pointSize=1.0F;
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));

}

void FsField::DrawMapVisual
    (FSENVIRONMENT env,
     const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projTfm,const double &elvMin,const double &elvMax,YSBOOL drawPset,
     const double &currentTime,
     YSBOOL useOpenGlGroundTexture,YSBOOL useOpenGlRunwayLightTexture) const
{
	int wid,hei;
	float pointSize,pointScaleA,pointScaleB,pointScaleC;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);

	FsGetWindowSize(wid,hei);


	float pointScale;
	if(env==FSNIGHT)
	{
		pointScale=1.0;
	}
	else
	{
		pointScale=0.5;
	}

	if(useOpenGlRunwayLightTexture==YSTRUE) // Runway Light Texture is on
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSCALEENABLE,TRUE);

		auto &commonTexture=FsCommonTexture::GetCommonTexture();
		auto unitPtr=commonTexture.GetRunwayLightTexture();

		if(nullptr!=unitPtr)
		{
			pointSize=pointScale*0.4F;
			ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));
		}
		else
		{
			pointSize=pointScale*0.2F; // 4.0F/(float)hei;
			ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));
		}

		pointScaleA=1.0F;
		pointScaleB=0.0F;
		pointScaleC=1.0F;
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSCALE_A,*((DWORD *)&pointScaleA));
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSCALE_B,*((DWORD *)&pointScaleB));
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSCALE_C,*((DWORD *)&pointScaleC));
	}
	else
	{
		pointSize=(float)wid/1024.0F;  // 1024 pixel -> 2 dots
		pointSize=YsBound(pointSize,1.0F,2.0F);
		pointSize*=pointScale;
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));
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
		fld->DrawMapVisual(viewTfm,YsIdentity4x4(),projTfm,elvMin,elvMax,drawPset,currentTime);
	}

	pointSize=1.0F;
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSCALEENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);
}

