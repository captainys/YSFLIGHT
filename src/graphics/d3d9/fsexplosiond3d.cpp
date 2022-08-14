#ifndef WIN32_LEAN_AND_MEAN
	// Prevent inclusion of winsock.h
	#define WIN32_LEAN_AND_MEAN
#endif

#include <ysclass.h>
#include <ysglbuffermanager.h>
#include <ysglbuffermanager_d3d9.h>

#include "fs.h"
#include "graphics/common/fsopengl.h"

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"



void FsExplosion::Draw(const YsVec3 &viewPos,YSBOOL transparency,YSBOOL useOpenGlDisplayList) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast<FsD3dDevice *>(ysD3dDev);
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(timeRemain>=YsTolerance)
	{
		double r,t,alpha;
		t=YsSmaller(timePassed,0.6)/0.6;

		if(transparency==YSTRUE)
		{
			double d;
			d=timeRemain+timePassed;
			if(d>YsTolerance)
			{
				alpha=0.4+0.6*(timeRemain/d);
			}
			else
			{
				alpha=1.0;
			}
		}
		else
		{
			alpha=1.0;
		}


		D3DMATERIAL9 material;
		ZeroMemory(&material,sizeof(material));

		if(expType==FSEXPLOSION_FIREBALL)
		{
			YSBOOL drawAsFlash=YSFALSE;
			if(flash==YSTRUE && timePassed<=0.05)
			{
				drawAsFlash=YSTRUE;
			}

			if(YSTRUE==drawAsFlash)
			{
				material.Diffuse.r=0.0F;
				material.Diffuse.g=0.0F;
				material.Diffuse.b=0.0F;
				material.Diffuse.a=1.0F;
				material.Ambient.r=0.0F;
				material.Ambient.g=0.0F;
				material.Ambient.b=0.0F;
				material.Ambient.a=1.0F;
				material.Specular.r=0.0F;
				material.Specular.g=0.0F;
				material.Specular.b=0.0F;
				material.Specular.a=1.0F;
				material.Emissive.r=1.0F;
				material.Emissive.g=1.0F;
				material.Emissive.b=1.0F;
				material.Emissive.a=1.0F;

				r=radius/2.0;
			}
			else
			{
				FLOAT c[4];
				c[0]=(float)(1.0-t*t*0.78);
				c[1]=(float)(t*t*0.22);
				c[2]=(float)(t*t*0.22);
				c[3]=(float)alpha;

				material.Diffuse.r=c[0];
				material.Diffuse.g=c[1];
				material.Diffuse.b=c[2];
				material.Diffuse.a=c[3];
				material.Ambient.r=c[0];
				material.Ambient.g=c[1];
				material.Ambient.b=c[2];
				material.Ambient.a=c[3];
				material.Specular.r=0.0F;
				material.Specular.g=0.0F;
				material.Specular.b=0.0F;
				material.Specular.a=c[3];
				material.Emissive.r=0.0F;
				material.Emissive.g=0.0F;
				material.Emissive.b=0.0F;
				material.Emissive.a=c[3];

				r=iniRadius+(radius-iniRadius)*t;
			}

			ysD3dDev->d3dDev->SetMaterial(&material);

			YSBOOL inside=YSFALSE;
			if((viewPos-pos).GetSquareLength()<r*r)
			{
				inside=YSTRUE;
			}

			if((nullptr==fireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(fireballVboHd)->GetState()) ||
			   (nullptr==halfFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(halfFireballVboHd)->GetState()) ||
			   (nullptr==reverseFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(reverseFireballVboHd)->GetState()) ||
			   (nullptr==reverseHalfFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(reverseHalfFireballVboHd)->GetState()))
			{
				PrepareFireballVertexArray();
			}

			YsGLBufferManager::Handle vboHd=nullptr;

			float y;
			if(pos.y()>r)
			{
				y=pos.yf();
				if(YSTRUE!=inside)
				{
					vboHd=reverseFireballVboHd;
				}
				else
				{
					vboHd=fireballVboHd;
				}
			}
			else
			{
				y=0.0f;
				if(YSTRUE!=inside)
				{
					vboHd=reverseHalfFireballVboHd;
				}
				else
				{
					vboHd=halfFireballVboHd;
				}
			}


			D3DMATRIX pushMatrix,tra,sca;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			YsD3dMakeTranslation(tra,pos.xf(),y,pos.zf());
			YsD3dMakeScaling(sca,r,r,r);

			ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tra);
			ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&sca);

			if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
			{
				auto unitPtr=bufMan.GetBufferUnit(vboHd);
				if(nullptr!=unitPtr)
				{
					unitPtr->GetActualBuffer()->DrawPrimitiveVtxNom(D3DPT_TRIANGLELIST);
				}
			}

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);

			if(YSTRUE==drawAsFlash)
			{
				material.Diffuse.r=0.0F;
				material.Diffuse.g=0.0F;
				material.Diffuse.b=0.0F;
				material.Diffuse.a=1.0F;
				material.Ambient.r=0.0F;
				material.Ambient.g=0.0F;
				material.Ambient.b=0.0F;
				material.Ambient.a=1.0F;
				material.Specular.r=0.0F;
				material.Specular.g=0.0F;
				material.Specular.b=0.0F;
				material.Specular.a=1.0F;
				material.Emissive.r=0.0F;
				material.Emissive.g=0.0F;
				material.Emissive.b=0.0F;
				material.Emissive.a=0.0F;
				ysD3dDev->d3dDev->SetMaterial(&material);
			}
		}
		else if(expType==FSEXPLOSION_WATERPLUME)
		{
			material.Diffuse.r=1.0;
			material.Diffuse.g=1.0;
			material.Diffuse.b=1.0;
			material.Diffuse.a=(float)alpha;
			material.Ambient.r=1.0;
			material.Ambient.g=1.0;
			material.Ambient.b=1.0;
			material.Ambient.a=(float)alpha;
			material.Specular.r=0.0F;
			material.Specular.g=0.0F;
			material.Specular.b=0.0F;
			material.Specular.a=(float)alpha;
			material.Emissive.r=0.0F;
			material.Emissive.g=0.0F;
			material.Emissive.b=0.0F;
			material.Emissive.a=(float)alpha;

			ysD3dDev->d3dDev->SetMaterial(&material);

			ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);


			D3DMATRIX pushMatrix,tra,sca;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			YsD3dMakeTranslation(tra,(FLOAT)pos.x(),(FLOAT)pos.y(),(FLOAT)pos.z());
			YsD3dMakeScaling(sca,(FLOAT)radius,(FLOAT)height,(FLOAT)radius);

			ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tra);
			ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&sca);

			if(nullptr==waterPlumeVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(waterPlumeVboHd)->GetState())
			{
				PrepareWaterPlumeVertexArray();
			}

			auto vboHd=waterPlumeVboHd;
			if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
			{
				auto unitPtr=bufMan.GetBufferUnit(vboHd);
				if(nullptr!=unitPtr)
				{
					unitPtr->GetActualBuffer()->DrawPrimitiveVtxNom(D3DPT_TRIANGLESTRIP);
				}
			}

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}
}

void FsExplosionHolder::Draw(const YsVec3 &viewPos,YSBOOL transparency,YSBOOL useOpenGlDisplayList) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	ysD3dDev->d3dDev->SetRenderState(D3DRS_NORMALIZENORMALS,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

	if(transparency==YSTRUE)
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);  // D3DBLEND_SRCALPHA?
		ysD3dDev->d3dDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);  // D3DBLEND_SRCALPHA?
		ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHAREF,25);
	}

	FsExplosion *seeker,*nxt;
	for(seeker=activeList; seeker!=NULL; seeker=nxt)
	{
		nxt=seeker->next;
		seeker->Draw(viewPos,transparency,useOpenGlDisplayList);
	}

	if(transparency==YSTRUE)
	{
		//ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);  // <- Turned on FsExplosion::Draw
	ysD3dDev->d3dDev->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);
}

