#include <ysclass.h>
#include "ysunitconv.h"

#include "graphics/common/fsopengl.h"
#include "fshud2.h"
#include "fsinstreading.h"

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>


#include "fswirefont.h"


extern YSBOOL FsD3dMyDriverSucks;  // Defined in fsoption.cpp


class FsHud2D3dSpecificResource
{
public:
	D3DMATRIX prevMatrix;
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


	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	this->viewPos=viewPos;
	this->airAtt=airAtt;

	FsHud2D3dSpecificResource *res=(FsHud2D3dSpecificResource *)specificResource;
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&res->prevMatrix);


	D3DMATRIX viewTransD3d;

	YsMatrix3x3 airRot;
	airRot.Rotate(airAtt);
	D3DMATRIX airRotD3d;
	YsD3dSetMatrix(airRotD3d,
	    (float)(airRot.v11()),(float)(airRot.v21()),(float)(airRot.v31()),0.0F,
	    (float)(airRot.v12()),(float)(airRot.v22()),(float)(airRot.v32()),0.0F,
	    (float)(airRot.v13()),(float)(airRot.v23()),(float)(airRot.v33()),0.0F,
	    0.0F        ,0.0F        ,0.0F        ,1.0F);

	YsD3dMakeTranslation(viewTransD3d,(float)viewPos.x(),(float)viewPos.y(),(float)viewPos.z());


	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&viewTransD3d);
	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&airRotD3d);
}

void FsHud2::EndDrawHud(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

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

	FsHud2D3dSpecificResource *res=(FsHud2D3dSpecificResource *)specificResource;
	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&res->prevMatrix);
}
