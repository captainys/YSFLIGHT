#include <ysclass.h>
#include "fs.h"

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"





void FsWeaponSmokeTrail::Draw
   (const YsVec3 &pos,const YsAtt3 &att,YSBOOL,FSSMOKETYPE smk,const double &cTime) const
{
	YsGLVertexBuffer vtxBuf[4];
	YsGLNormalBuffer nomBuf[4];
	YsGLColorBuffer colBuf[4];

	this->MakeVertexArray(
	    vtxBuf,nomBuf,colBuf,
	    pos,att,smk,cTime);

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast<FsD3dDevice *>(ysD3dDev);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

	for(int i=0; i<4; ++i)
	{
		for(YSSIZE_T j=0; j<vtxBuf[i].size(); ++j)
		{
			ysD3dDev->AddXyzNomCol(
			    D3DPT_TRIANGLESTRIP,
			    vtxBuf[i][j][0],vtxBuf[i][j][1],vtxBuf[i][j][2],
			    nomBuf[i][j][0],nomBuf[i][j][1],nomBuf[i][j][2],
			    (int)(255.0f*colBuf[i][j][0]),(int)(255.0f*colBuf[i][j][1]),(int)(255.0f*colBuf[i][j][2]),(int)(255.0f*colBuf[i][j][3]));
		}
		ysD3dDev->FlushXyzNomCol(D3DPT_TRIANGLESTRIP);
	}
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
}

void FsWeapon::Draw(
    YSBOOL coarse,const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,
    YSBOOL transparency,FSSMOKETYPE smk,const double &cTime,unsigned int drawFlag) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);
	D3DMATRIX pushMatrix;

	if(fsD3dDev->gunBuffer==NULL)
	{
		fsD3dDev->gunBuffer=new YsXyzColBuffer;
		fsD3dDev->gunBuffer->PrepareBuffer(*fsD3dDev,32);
	}
	if(fsD3dDev->debrisBuffer==NULL)
	{
		fsD3dDev->debrisBuffer=new YsXyzNomColBuffer;
		fsD3dDev->debrisBuffer->PrepareBuffer(*fsD3dDev,4);

		fsD3dDev->debrisBuffer->LockBuffer();
		fsD3dDev->debrisBuffer->Set(0, 0.6,0.0, 0.6,0.0,1.0,0.0,32,32,32,255);
		fsD3dDev->debrisBuffer->Set(1,-0.6,0.0, 0.6,0.0,1.0,0.0,32,32,32,255);
		fsD3dDev->debrisBuffer->Set(2, 0.6,0.0,-0.6,0.0,1.0,0.0,32,32,32,255);
		fsD3dDev->debrisBuffer->Set(3,-0.6,0.0,-0.6,0.0,1.0,0.0,32,32,32,255);
		fsD3dDev->debrisBuffer->UnlockBuffer();
		fsD3dDev->debrisBuffer->nBufUsed=4;
	}

	if(lifeRemain>YsTolerance || timeRemain>YsTolerance)
	{
		if(lifeRemain>YsTolerance &&
		   YSTRUE!=coarse &&
		   FSWEAPON_NUMWEAPONTYPE>type &&
		   NULL!=firedBy &&
		   NULL!=firedBy->weaponShapeOverrideFlying[(int)type])
		{
			firedBy->weaponShapeOverrideFlying[(int)type].Draw(viewMat,projMat,pos,att,drawFlag);
		}
		else switch(type)
		{
		case FSWEAPON_GUN:
			{
				YsVec3 ps2;
				ps2=pos+vec/velocity*10.0;
				fsD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
				fsD3dDev->gunBuffer->Add(D3DPT_LINELIST,pos.x(),pos.y(),pos.z(),255,255,0,255);
				fsD3dDev->gunBuffer->Add(D3DPT_LINELIST,ps2.x(),ps2.y(),ps2.z(),255,255,255,255);
			}
			break;

		case FSWEAPON_AIM120:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? aim120 : aim120_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AIM9:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? aim9 : aim_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AIM9X:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? aim9x : aim9x_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AGM65:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? agm65 : agm_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? bomb : bomb_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB250:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? bomb250 : bomb250_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB500HD:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? bomb500hd : bomb500hd_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_ROCKET:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=(coarse!=YSTRUE ? rocket : rocket_coarse);
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_FUELTANK:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=fuelTank;
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_FLARE:
			break;

		case FSWEAPON_DEBRIS:
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
			ysD3dDev->MultiplyModelMatrix(pos.x(),pos.y(),pos.z(),att.h(),att.p(),att.b());

			fsD3dDev->debrisBuffer->DrawPrimitive(D3DPT_TRIANGLESTRIP);
			fsD3dDev->debrisBuffer->DrawPrimitive(D3DPT_POINTLIST);

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);

			break;
		}


		fsD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
		fsD3dDev->gunBuffer->Flush(D3DPT_LINELIST);

		if(trail!=nullptr &&
		   FSSMKNULL!=smk &&
		   0!=(drawFlag&FSVISUAL_DRAWTRANSPARENT))
		{
			trail->Draw(pos,att,transparency,smk,cTime);
		}
	}
}

////////////////////////////////////////////////////////////

void FsWeaponHolder::BeginDraw(void) const
{
}

void FsWeaponHolder::EndDraw(void) const
{
}

void FsWeaponHolder::DrawGunCalibrator(void) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(0<bulletCalibrator.GetN())
	{
		FLOAT pointSize=3.0F;
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));

		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);


		for(int i=0; i<bulletCalibrator.GetN(); i++)
		{
			const YsVec3 &nearPos=bulletCalibrator[i];
			ysD3dDev->AddXyzCol(D3DPT_POINTLIST,nearPos.x(),nearPos.y(),nearPos.z(),255,0,0,255);
		}


		ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);


		pointSize=1.0F;
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));
	}
}

