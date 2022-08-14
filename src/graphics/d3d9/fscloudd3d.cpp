#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"
#include <ysglbuffermanager.h>
#include <ysglbuffermanager_d3d9.h>

#include <fsdef.h>
#include "graphics/common/fsopengl.h"
#include "fscloud.h"




class FsCloudsGraphicCache
{
public:
	class YsD3dExternalVertexBufferLink *d3dVtxBuf;
};

class FsSolidCloudGraphicCache
{
public:
	class YsD3dExternalVertexBufferLink *d3dVtxBuf;
};


void FsCloud::Draw(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	for(int i=0; i<vtx.GetN(); i++)
	{
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,vtx[i].x(),vtx[i].y(),vtx[i].z(),col.Ri(),col.Gi(),col.Bi(),255);
	}
	ysD3dDev->FlushXyzCol(D3DPT_TRIANGLEFAN);


//	glColor3d(col.Rd(),col.Gd(),col.Bd());
//
//	glBegin(GL_TRIANGLE_FAN);
//	glVertex3dv(cen);
//	int i;
//	for(i=0; i<vtx.GetN(); i++)
//	{
//		glVertex3dv(vtx[i]);
//	}
//	glVertex3dv(vtx[0]);
//	glEnd();
}

void FsClouds::CreateGraphicCache(void)
{
	res=new FsCloudsGraphicCache;
	res->d3dVtxBuf=new YsD3dExternalVertexBufferLink;
}

void FsClouds::DeleteGraphicCache(void)
{
	delete res->d3dVtxBuf;
	delete res;
}

void FsClouds::Draw(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);

	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==vboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(vboHd)->GetState() || YSTRUE==needRemakeVbo)
	{
		MakeOpenGlList();
		needRemakeVbo=YSFALSE;
	}

	if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

		auto unitPtr=bufMan.GetBufferUnit(vboHd);
		if(nullptr!=unitPtr)
		{
			unitPtr->GetActualBuffer()->DrawPrimitiveVtxCol(D3DPT_TRIANGLELIST);
		}
	}
}



void FsSolidClouds::ReduceVisibilityByPolygon(const YsMatrix4x4 &viewTfm,const YsColor &col,YSBOOL transparency)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		YsMatrix4x4 invViewTfm;
		invViewTfm=viewTfm;
		invViewTfm.Invert();


		if(transparency==YSTRUE)
		{
			ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
			ysD3dDev->d3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
			ysD3dDev->d3dDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
			ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHAREF,25);

			ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

			int z;
			for(z=400; z>=200; z-=50)
			{
				YsVec3 pos;
				pos.Set(z*3,z*3,z);
				invViewTfm.Mul(pos,pos,1.0);
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

				pos.Set(-z*3,z*3,z);
				invViewTfm.Mul(pos,pos,1.0);
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

				pos.Set(-z*3,-z*3,z);
				invViewTfm.Mul(pos,pos,1.0);
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);


				pos.Set(-z*3,-z*3,z);
				invViewTfm.Mul(pos,pos,1.0);
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

				pos.Set(z*3,-z*3,z);
				invViewTfm.Mul(pos,pos,1.0);
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

				pos.Set(z*3,z*3,z);
				invViewTfm.Mul(pos,pos,1.0);
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);
			}
			ysD3dDev->FlushXyzCol(D3DPT_TRIANGLELIST);

			//ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
		}
		else
		{
			ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

			int z;
			YsVec3 pos;

			z=200;

			pos.Set(z*3,z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

			pos.Set(-z*3,z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

			pos.Set(-z*3,-z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

			pos.Set(z*3,-z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),160);

			ysD3dDev->FlushXyzCol(D3DPT_TRIANGLEFAN);
		}
	}
}

void FsSolidCloud::CreateGraphicCache(void)
{
	res=new FsSolidCloudGraphicCache;
	res->d3dVtxBuf=new YsD3dExternalVertexBufferLink;
}

void FsSolidCloud::DeleteGraphicCache(void)
{
	delete res->d3dVtxBuf;
	delete res;
}

void FsSolidCloud::Draw(FSENVIRONMENT env,const FsWeather &weather)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);

	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==vboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(vboHd)->GetState())
	{
		MakeOpenGlList();
	}

	if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

		auto unitPtr=bufMan.GetBufferUnit(vboHd);
		if(nullptr!=unitPtr)
		{
			unitPtr->GetActualBuffer()->DrawPrimitiveVtxNomCol(D3DPT_TRIANGLELIST);
		}

		ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	}
}

void FsSolidClouds::Test(void)
{
}

void FsSolidClouds::SetUpCloudPerFrame(void)
{
}

void FsSolidClouds::BeginDrawCloud(void)
{
}

void FsSolidClouds::EndDrawCloud(void)
{
}

