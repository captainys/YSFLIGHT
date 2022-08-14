#include <ysclass.h>

#include "ysunitconv.h"

#include "fs.h"
#include "graphics/common/fsopengl.h"

#include "fsinstpanel.h"
#include "fswirefont.h"


#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"



void FsInstrumentPanel::DrawCrossHair3d(void)  // This function is called from BeginDraw3d
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	ysD3dDev->AddXyz(D3DPT_LINELIST,-0.01,0.0,0.5);
	ysD3dDev->AddXyz(D3DPT_LINELIST,-0.03,0.0,0.5);
	ysD3dDev->AddXyz(D3DPT_LINELIST, 0.01,0.0,0.5);
	ysD3dDev->AddXyz(D3DPT_LINELIST, 0.03,0.0,0.5);
	ysD3dDev->AddXyz(D3DPT_LINELIST,0.0,-0.01,0.5);
	ysD3dDev->AddXyz(D3DPT_LINELIST,0.0,-0.03,0.5);
	ysD3dDev->AddXyz(D3DPT_LINELIST,0.0, 0.01,0.5);
	ysD3dDev->AddXyz(D3DPT_LINELIST,0.0, 0.03,0.5);

	ysD3dDev->FlushXyz(D3DPT_LINELIST);

}

void FsInstrumentPanel::EndDraw3d(void)
{
	auto viewPos=viewPosCache;
	auto localViewPos=localViewPosCache;
	auto &prop=*airPropCache;


	D3DMATRIX prevTfm;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&prevTfm);


	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);


	D3DMATRIX viewTransD3d,fwdTransD3d,scaleTransD3d;

	YsMatrix3x3 airRot;
	airRot.Rotate(prop.GetAttitude());
	D3DMATRIX airRotD3d;
	YsD3dSetMatrix(airRotD3d,
	    (FLOAT)airRot.v11(),(FLOAT)airRot.v21(),(FLOAT)airRot.v31(),0.0F,
	    (FLOAT)airRot.v12(),(FLOAT)airRot.v22(),(FLOAT)airRot.v32(),0.0F,
	    (FLOAT)airRot.v13(),(FLOAT)airRot.v23(),(FLOAT)airRot.v33(),0.0F,
	    0.0F               ,0.0F               ,0.0F               ,1.0F);

	YsD3dMakeTranslation(viewTransD3d,(float)viewPos.x(),(float)viewPos.y(),(float)viewPos.z());

	YsVec3 instPanelPos;
	prop.GetInstPanelPos(instPanelPos);
	instPanelPos-=localViewPos;
	YsD3dMakeTranslation(fwdTransD3d,(FLOAT)instPanelPos.x(),(FLOAT)instPanelPos.y(),(FLOAT)instPanelPos.z());

	YsD3dMakeScaling<float>(scaleTransD3d,(FLOAT)prop.GetInstPanelScaling(),(FLOAT)prop.GetInstPanelScaling(),(FLOAT)prop.GetInstPanelScaling());

	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&viewTransD3d);
	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&airRotD3d);

	if(drawCrossHair==YSTRUE)
	{
		DrawCrossHair3d();
	}

	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&fwdTransD3d);
	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&scaleTransD3d);


	const YsAtt3 &instAtt=prop.GetInstPanelAtt();
	D3DMATRIX tfm;
	YsD3dMakeRotationY(tfm,(FLOAT)instAtt.h());
	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tfm);
	YsD3dMakeRotationX(tfm,-(FLOAT)instAtt.p());
	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tfm);
	YsD3dMakeRotationZ(tfm,(FLOAT)instAtt.b());
	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tfm);


	/* D3DMATERIAL9 material;
	material.Emissive.r=(FLOAT)hudCol.Rd();
	material.Emissive.g=(FLOAT)hudCol.Gd();
	material.Emissive.b=(FLOAT)hudCol.Bd();
	material.Emissive.a=1.0F;
	material.Diffuse.a=1.0F;
	material.Specular.a=1.0F;
	material.Ambient.a=1.0F;
	ysD3dDev->d3dDev->SetMaterial(&material); */



	for(int i=0; i<triVtxBuf.GetN(); ++i)
	{
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,
		    triVtxBuf[i][0],triVtxBuf[i][1],triVtxBuf[i][2],
		    255.0*triColBuf[i][0],255.0*triColBuf[i][1],255.0*triColBuf[i][2],255.0*triColBuf[i][3]);
	}
	ysD3dDev->FlushXyzCol(D3DPT_TRIANGLELIST);

	for(int i=0; i<lineVtxBuf.GetN(); ++i)
	{
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,
		    lineVtxBuf[i][0],lineVtxBuf[i][1],lineVtxBuf[i][2],
		    255.0*lineColBuf[i][0],255.0*lineColBuf[i][1],255.0*lineColBuf[i][2],255.0*lineColBuf[i][3]);
	}
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	for(int i=0; i<ovTriVtxBuf.GetN(); ++i)
	{
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,
		    ovTriVtxBuf[i][0],ovTriVtxBuf[i][1],ovTriVtxBuf[i][2],
		    255.0*ovTriColBuf[i][0],255.0*ovTriColBuf[i][1],255.0*ovTriColBuf[i][2],255.0*ovTriColBuf[i][3]);
	}
	ysD3dDev->FlushXyzCol(D3DPT_TRIANGLELIST);

	for(int i=0; i<ovLineVtxBuf.GetN(); ++i)
	{
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,
		    ovLineVtxBuf[i][0],ovLineVtxBuf[i][1],ovLineVtxBuf[i][2],
		    255.0*ovLineColBuf[i][0],255.0*ovLineColBuf[i][1],255.0*ovLineColBuf[i][2],255.0*ovLineColBuf[i][3]);
	}
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);



	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&prevTfm);
}

