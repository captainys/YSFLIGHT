#include <ysclass.h>

#include <windows.h>
#include <mmsystem.h>

#include <stdio.h>
#include <stdlib.h>

#include <ysd3d9.h>

#include "ysscenery.h"

#include <ystexturemanager_d3d9.h>



// Note: Texture pointes must be assigned in FsExistence::DrawMapVisual
//       and must be NULLed in FsD3dDevice::ReleaseBuffer
//IDirect3DTexture9 *ysRwLightTex=NULL;

static YsMatrix4x4 invViewTfm;


// Bbx & Axis of 2D Drawing      Magenta
// Bbx & Axis of Elevation Grid  Cyan
// Bbx & Axis of Child Sscenary  DarkYellow
// Bbx & Axis of Shell           White
// Bbx & Axis of GndObj          Blue

/*
YSRESULT DoSomethingForEverything(YsScenery *scenary)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsScenery> *scn;

	drw=NULL;
	while((drw=scenary->FindNextMap(drw))!=NULL)
	{
	}

	shl=NULL;
	while((shl=scenary->FindNextShell(shl))!=NULL)
	{
	}

	evg=NULL;
	while((evg=scenary->FindNextElevationGrid(evg))!=NULL)
	{
	}

	drw=NULL;
	while((drw=scenary->FindNextSignBoard(drw))!=NULL)
	{
	}

	rgn=NULL;
	while((rgn=scenary->FindNextRectRegion(rgn))!=NULL)
	{
	}

	scn=NULL;
	while((scn=scenary->FindNextChildScenery(scn))!=NULL)
	{
		DoSomethingForEverything(&scn->dat);
	}

	return YSOK;
}
*/


void YsD3dDrawAxis(const double &,const YsColor &)
{
}

static void Ys2DDrawing_SwitchTexture(
    YSBOOL mapMode,Ys2DDrawingElement::OBJTYPE objType,
    const YsTextureManager::Unit *ownTexUnitPtr,
    const YsTextureManager::Unit *commonGndTexUnitPtr,
    const YsTextureManager::Unit *commonRwLightTexUnitPtr)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);

	switch(objType)
	{
	case Ys2DDrawingElement::POLYGON:
	case Ys2DDrawingElement::GRADATIONQUADSTRIP:
	case Ys2DDrawingElement::QUADSTRIP:
	case Ys2DDrawingElement::QUADS:
	case Ys2DDrawingElement::TRIANGLES:
		if(nullptr!=ownTexUnitPtr)
		{
			ownTexUnitPtr->Bind();
		}
		else if(nullptr!=commonGndTexUnitPtr)
		{
			commonGndTexUnitPtr->Bind();
		}
		break;
	case Ys2DDrawingElement::APPROACHLIGHT:
	case Ys2DDrawingElement::POINTS:
		if(mapMode==YSTRUE && nullptr!=commonRwLightTexUnitPtr)
		{
			commonRwLightTexUnitPtr->Bind();
		 	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,TRUE);
		}
		else
		{
		 	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);
		}
		break;
	default:
		if(mapMode==YSTRUE)
		{
			
			ysD3dDev->d3dDev->SetTexture(0,NULL);
		 	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);
		}
		break;
	}
}

void Ys2DDrawing::AllocCache(void) const
{
}

void Ys2DDrawing::DeleteCache(void) const
{
}

YSBOOL Ys2DDrawing::IsCached(void) const
{
	return YSFALSE;
}

void Ys2DDrawing::Draw(
    const double &plgColScale,const double &linColorScale,const double &pntColorScale,
    YSBOOL drawPset,
    YSBOOL mapMode,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL name2DElem,
    const double &currentTime,
    YsMatrix4x4 *viewModelTfm)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	const YsTextureManager *commonTexManPtr=YsScenery::commonTexManPtr;
	const YsTextureManager::Unit *commonGndTexUnitPtr=nullptr;
	const YsTextureManager::Unit *commonRwLightTexUnitPtr=nullptr;
	if(nullptr!=commonTexManPtr)
	{
		commonGndTexUnitPtr=commonTexManPtr->GetTextureReady(YsScenery::commonGroundTexHd);
		commonRwLightTexUnitPtr=commonTexManPtr->GetTextureReady(YsScenery::commonRunwayLightTexHd);
	}

	YsColor color,color2;
	YsListItem <Ys2DDrawingElement> *itm;
	D3DPRIMITIVETYPE primType,nextPrimType;
	YSBOOL first;
	double invView[16];
	D3DSHADEMODE curShadMode;

	invViewTfm.GetOpenGlCompatibleMatrix(invView);

	D3DMATRIX mapModeFlip;
	YsD3dSetMatrix(mapModeFlip,
		1.0F,0.0F,0.0F,0.0F,
		0.0F,0.0F,1.0F,0.0F,
		0.0F,1.0F,0.0F,0.0F,
		0.0F,0.0F,0.0F,1.0F
	);

	D3DMATRIX texTfm;
	YsD3dSetMatrix(texTfm,
		0.02F,0.00F,0.00F,0.00F,
		0.00F,0.00F,0.02F,0.00F,
		0.00F,0.02F,0.00F,0.00F,
		0.00F,0.00F,0.00F,0.02F
	);
	D3DMATRIX autoTexTfm;
	YsD3dSetMatrix(autoTexTfm,
		(FLOAT)invView[ 0],(FLOAT)invView[ 1],(FLOAT)invView[ 2],(FLOAT)invView[ 3],
		(FLOAT)invView[ 4],(FLOAT)invView[ 5],(FLOAT)invView[ 6],(FLOAT)invView[ 7],
		(FLOAT)invView[ 8],(FLOAT)invView[ 9],(FLOAT)invView[10],(FLOAT)invView[11],
		(FLOAT)invView[12],(FLOAT)invView[13],(FLOAT)invView[14],(FLOAT)invView[15]
	);
	D3DMATRIX identity;
	YsD3dMakeIdentity(identity);



	itm=NULL;
	primType=D3DPT_TRIANGLESTRIP;
	first=YSTRUE;

	color=YsBlue();

	ysD3dDev->d3dDev->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_FLAT);
	curShadMode=D3DSHADE_FLAT;

	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

	BOOL lighting=TRUE;


	YSBOOL useOwnTexture=YSFALSE;
	ysD3dDev->d3dDev->SetTransform(D3DTS_TEXTURE0,&texTfm);
	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_TEXTURE0,&autoTexTfm);
	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);

	if(mapMode==YSTRUE)
	{
		ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&mapModeFlip);
	}

	const double nx=0.0,ny=0.0,nz=1.0;

	while((itm=elemList.FindNext(itm))!=NULL)
	{
		int i;
		double t;
		BOOL nextLighting=lighting;

		if(itm->dat.pnt.GetN()==0)
		{
			goto NEXTOBJ;
		}

		if(drawPset!=YSTRUE && itm->dat.t==Ys2DDrawingElement::POINTS)
		{
			goto NEXTOBJ;
		}

		auto ownTexManPtr=itm->dat.texManCache;
		const YsTextureManager::Unit *ownTexUnitPtr=nullptr;
		if(nullptr!=ownTexManPtr)
		{
			ownTexUnitPtr=ownTexManPtr->GetTextureReady(itm->dat.texHdCache);
		}

		if(0<itm->dat.texLabel.Strlen() && NULL!=owner)
		{
			auto scn=owner->GetOwner();
			auto texHd=scn->textureManager.FindTextureFromLabel(itm->dat.texLabel);
			if(NULL!=texHd)
			{
				itm->dat.texManCache=&scn->textureManager;
				itm->dat.texHdCache=texHd;
			}
		}

		{
			if(nullptr!=ownTexUnitPtr && itm->dat.pnt.GetN()==itm->dat.texCoord.GetN())
			{
				useOwnTexture=YSTRUE;
			}
			else
			{
				useOwnTexture=YSFALSE;
			}

			if(YSTRUE==useOwnTexture)
			{
				// ysD3dDev->d3dDev->SetTransform(D3DTS_TEXTURE0,&identity);
				ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
				ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU);
			}
			else
			{
				// ysD3dDev->d3dDev->SetTransform(D3DTS_TEXTURE0,&texTfm);
				// ysD3dDev->d3dDev->MultiplyTransform(D3DTS_TEXTURE0,&autoTexTfm);
				ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
				ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
			}
		}


		if(viewModelTfm!=NULL && YsTolerance<itm->dat.VisibleDist())
		{
			YsVec3 cen;
			if(mapMode==YSTRUE)
			{
				cen.SetXZ((itm->dat.bbx[0]+itm->dat.bbx[1])/2.0);
				cen.SetY(0.0);
			}
			else
			{
				cen.SetXY((itm->dat.bbx[0]+itm->dat.bbx[1])/2.0);
				cen.SetZ(0.0);
			}
			viewModelTfm->Mul(cen,cen,1.0);
			if(YsSqr(itm->dat.VisibleDist())<cen.GetSquareLength())
			{
				goto NEXTOBJ;
			}
		}


		if(itm->dat.t==Ys2DDrawingElement::GRADATIONQUADSTRIP)
		{
			if(curShadMode!=D3DSHADE_GOURAUD)
			{
				ysD3dDev->d3dDev->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD);
				curShadMode=D3DSHADE_GOURAUD;
			}
		}
		else
		{
			if(curShadMode!=D3DSHADE_FLAT)
			{
				ysD3dDev->d3dDev->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_FLAT);
				curShadMode=D3DSHADE_FLAT;
			}
		}


		switch(itm->dat.t)
		{
		default:
			goto NEXTOBJ;

		case Ys2DDrawingElement::APPROACHLIGHT:
			if(first!=YSTRUE && primType!=D3DPT_POINTLIST)
			{
				ysD3dDev->FlushAll(primType);
			}
			Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);


			first=YSFALSE;
			primType=D3DPT_POINTLIST;

			if(TRUE==lighting)
			{
				lighting=FALSE;
				ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
			}

			color=itm->dat.c;

			if(currentTime>=0.0)
			{
				t=fmod(currentTime,0.5)/0.5;
				if(itm->dat.pnt.GetN()>2)
				{
					i=(int)((double)itm->dat.pnt.GetN()*t);
					i=YsBound <int> (i,0,(int)itm->dat.pnt.GetN()-1);

					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();

					ysD3dDev->AddXyzCol(primType,x    ,y    , 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x+0.4,y+0.0, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x-0.4,y+0.0, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y+0.0, 0.4,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y+0.0,-0.4,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y+0.4, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y-0.4, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
				}
				else if(t<0.1)
				{
					for(i=0; i<itm->dat.pnt.GetN(); i++)
					{
						const double &x=itm->dat.pnt[i].x();
						const double &y=itm->dat.pnt[i].y();
						ysD3dDev->AddXyzCol(primType,x    ,y    , 0.0,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzCol(primType,x+0.4,y+0.0, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzCol(primType,x-0.4,y+0.0, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzCol(primType,x    ,y+0.0, 0.4,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzCol(primType,x    ,y+0.0,-0.4,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzCol(primType,x    ,y+0.4, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzCol(primType,x    ,y-0.4, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					}
				}
			}
			else
			{
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzCol(primType,x    ,y    , 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x+0.4,y+0.0, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x-0.4,y+0.0, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y+0.0, 0.4,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y+0.0,-0.4,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y+0.4, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzCol(primType,x    ,y-0.4, 0.0,color.Ri(),color.Gi(),color.Bi(),255);
				}
			}
			break;

		case Ys2DDrawingElement::POINTS:
		case Ys2DDrawingElement::LINESEGMENTS:
		case Ys2DDrawingElement::LINES:
			switch(itm->dat.t)
			{
			case Ys2DDrawingElement::POINTS:
				color=itm->dat.c;
				nextPrimType=D3DPT_POINTLIST;
				nextLighting=FALSE;
				break;
			case Ys2DDrawingElement::LINESEGMENTS:
				color=itm->dat.c;
				nextPrimType=D3DPT_LINESTRIP;
				nextLighting=TRUE;
				break;
			case Ys2DDrawingElement::LINES:
				color=itm->dat.c;
				nextPrimType=D3DPT_LINELIST;
				nextLighting=TRUE;
				break;
			}
			if(first!=YSTRUE && primType!=nextPrimType)
			{
				ysD3dDev->FlushAll(primType);
			}

			Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

			if(nextLighting!=lighting)
			{
				lighting=nextLighting;
				ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,lighting);
			}

			first=YSFALSE;
			primType=nextPrimType;

			for(i=0; i<itm->dat.pnt.GetN(); i++)
			{
				const double &x=itm->dat.pnt[i].x();
				const double &y=itm->dat.pnt[i].y();
				if(TRUE==lighting)
				{
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
				}
				else
				{
					ysD3dDev->AddXyzCol(primType,x,y,0.0,color.Ri(),color.Gi(),color.Bi(),255);
				}
			}

			if(itm->dat.t==Ys2DDrawingElement::LINESEGMENTS)
			{
				ysD3dDev->FlushAll(primType);  // Should not continue to the next.
			}
			break;
		case Ys2DDrawingElement::POLYGON:
			color=itm->dat.c;

			if(TRUE!=lighting)
			{
				if(YSTRUE!=first)
				{
					ysD3dDev->FlushAll(primType);
					first=YSFALSE;
				}
				lighting=TRUE;
				ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,lighting);
			}

			if(wire==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_LINESTRIP)
				{
					ysD3dDev->FlushAll(primType);
				}
				if(primType==D3DPT_POINTLIST)
				{
					ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);
					ysD3dDev->d3dDev->SetTexture(0,NULL);
				}
				first=YSFALSE;
				primType=D3DPT_LINESTRIP;

				int lastI;
				lastI=(int)itm->dat.pnt.GetN()-1;
				for(i=0; i<=itm->dat.pnt.GetN(); i++)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					lastI=i;
				}
			}

			if(fill==YSTRUE)
			{
				if(itm->dat.pnt.GetN()==3)
				{
					if(first!=YSTRUE && primType!=D3DPT_TRIANGLELIST)
					{
						ysD3dDev->FlushAll(primType);
					}
					Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

					first=YSFALSE;
					primType=D3DPT_TRIANGLELIST;

					for(i=0; i<itm->dat.pnt.GetN(); i++)
					{
						const double &x=itm->dat.pnt[i].x();
						const double &y=itm->dat.pnt[i].y();
						if(YSTRUE==useOwnTexture)
						{
							const double &u=itm->dat.texCoord[i].x();
							const double &v=itm->dat.texCoord[i].y();
							ysD3dDev->AddXyzNomColTex(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u,v);
						}
						else
						{
							ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						}
					}
				}
				else if(itm->dat.cvx==YSTRUE)
				{
					if(first!=YSTRUE)
					{
						ysD3dDev->FlushAll(primType);
					}
					primType=D3DPT_TRIANGLEFAN;
					Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

					for(i=0; i<itm->dat.pnt.GetN(); i++)
					{
						const double &x=itm->dat.pnt[i].x();
						const double &y=itm->dat.pnt[i].y();
						if(YSTRUE==useOwnTexture)
						{
							const double &u=itm->dat.texCoord[i].x();
							const double &v=itm->dat.texCoord[i].y();
							ysD3dDev->AddXyzNomColTex(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u,v);
						}
						else
						{
							ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						}
					}

					ysD3dDev->FlushAll(primType);
					first=YSTRUE;
				}
			}
			break;
		case Ys2DDrawingElement::GRADATIONQUADSTRIP:
			color=itm->dat.c;
			color2=itm->dat.c2;

			if(TRUE!=lighting)
			{
				if(YSTRUE!=first)
				{
					ysD3dDev->FlushAll(primType);
					first=YSFALSE;
				}
				lighting=TRUE;
				ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,lighting);
			}

			if(wire==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_LINESTRIP)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_LINESTRIP;

				for(i=0; i<itm->dat.pnt.GetN()-1; i+=2)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
				}

				for(i=1; i<itm->dat.pnt.GetN(); i+=2)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color2.Ri(),color2.Gi(),color2.Bi(),255);
				}

				ysD3dDev->FlushAll(primType);
				primType=D3DPT_LINELIST;
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color2.Ri(),color2.Gi(),color2.Bi(),255);
				}
			}

			if(fill==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_TRIANGLESTRIP)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_TRIANGLESTRIP;

				for(i=0; i<=itm->dat.pnt.GetN()-2; i+=2)
				{
					double x,y;
					x=itm->dat.pnt[i].x();
					y=itm->dat.pnt[i].y();
					if(YSTRUE==useOwnTexture)
					{
						const double &u=itm->dat.texCoord[i].x();
						const double &v=itm->dat.texCoord[i].y();
						ysD3dDev->AddXyzNomColTex(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u,v);
					}
					else
					{
						ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					}

					x=itm->dat.pnt[i+1].x();
					y=itm->dat.pnt[i+1].y();
					if(YSTRUE==useOwnTexture)
					{
						const double &u=itm->dat.texCoord[i+1].x();
						const double &v=itm->dat.texCoord[i+1].y();
						ysD3dDev->AddXyzNomColTex(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u,v);
					}
					else
					{
						ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color2.Ri(),color2.Gi(),color2.Bi(),255);
					}
				}
				ysD3dDev->FlushAll(primType);  // Must not continue to the next.
			}
			break;
		case Ys2DDrawingElement::QUADSTRIP:
			color=itm->dat.c;

			if(TRUE!=lighting)
			{
				if(YSTRUE!=first)
				{
					ysD3dDev->FlushAll(primType);
					first=YSFALSE;
				}
				lighting=TRUE;
				ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,lighting);
			}

			if(wire==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_LINESTRIP)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_LINESTRIP;

				for(i=0; i<itm->dat.pnt.GetN()-1; i+=2)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
				}
				for(i=1; i<itm->dat.pnt.GetN(); i+=2)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
				}
				ysD3dDev->FlushAll(primType);
				primType=D3DPT_LINELIST;
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					const double &x=itm->dat.pnt[i].x();
					const double &y=itm->dat.pnt[i].y();
					ysD3dDev->AddXyzNomCol(primType,x,y,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
				}
			}

			if(fill==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_TRIANGLESTRIP)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_TRIANGLESTRIP;

				for(i=0; i<=itm->dat.pnt.GetN()-2; i+=2)
				{
					const double &x1=itm->dat.pnt[i].x();
					const double &y1=itm->dat.pnt[i].y();
					const double &x2=itm->dat.pnt[i+1].x();
					const double &y2=itm->dat.pnt[i+1].y();

					if(YSTRUE==useOwnTexture)
					{
						const double &u1=itm->dat.texCoord[i].x();
						const double &v1=itm->dat.texCoord[i].y();
						const double &u2=itm->dat.texCoord[i+1].x();
						const double &v2=itm->dat.texCoord[i+1].y();
						ysD3dDev->AddXyzNomColTex(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u1,v1);
						ysD3dDev->AddXyzNomColTex(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u2,v2);
					}
					else
					{
						ysD3dDev->AddXyzNomCol(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					}
				}
				ysD3dDev->FlushAll(primType);  // Must not continue to the next.
			}
			break;
		case Ys2DDrawingElement::QUADS:
			color=itm->dat.c;

			if(TRUE!=lighting)
			{
				if(YSTRUE!=first)
				{
					ysD3dDev->FlushAll(primType);
					first=YSFALSE;
				}
				lighting=TRUE;
				ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,lighting);
			}

			if(wire==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_LINELIST)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_LINELIST;

				for(i=0; i<itm->dat.pnt.GetN()-4; i+=4)
				{
					int j,lastJ;
					double x1,y1,x2,y2;
					lastJ=3;
					for(j=0; j<4; j++)
					{
						x1=itm->dat.pnt[i+lastJ].x();
						y1=itm->dat.pnt[i+lastJ].y();
						x2=itm->dat.pnt[i+j].x();
						y2=itm->dat.pnt[i+j].y();
						ysD3dDev->AddXyzNomCol(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						lastJ=j;
					}
				}
			}

			if(fill==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_TRIANGLELIST)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_TRIANGLELIST;

				for(i=0; i<=itm->dat.pnt.GetN()-4; i+=4)
				{
					double x1,y1,x2,y2,x3,y3,x4,y4;

					x1=itm->dat.pnt[i  ].x();
					y1=itm->dat.pnt[i  ].y();
					x2=itm->dat.pnt[i+1].x();
					y2=itm->dat.pnt[i+1].y();
					x3=itm->dat.pnt[i+2].x();
					y3=itm->dat.pnt[i+2].y();
					x4=itm->dat.pnt[i+3].x();
					y4=itm->dat.pnt[i+3].y();

					if(YSTRUE==useOwnTexture)
					{
						const double &u1=itm->dat.texCoord[i].x();
						const double &v1=itm->dat.texCoord[i].y();
						const double &u2=itm->dat.texCoord[i+1].x();
						const double &v2=itm->dat.texCoord[i+1].y();
						const double &u3=itm->dat.texCoord[i+2].x();
						const double &v3=itm->dat.texCoord[i+2].y();
						const double &u4=itm->dat.texCoord[i+3].x();
						const double &v4=itm->dat.texCoord[i+3].y();

						ysD3dDev->AddXyzNomColTex(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u1,v1);
						ysD3dDev->AddXyzNomColTex(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u2,v2);
						ysD3dDev->AddXyzNomColTex(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u3,v3);
	                                                   
						ysD3dDev->AddXyzNomColTex(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u1,v1);
						ysD3dDev->AddXyzNomColTex(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u3,v3);
						ysD3dDev->AddXyzNomColTex(primType,x4,y4,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u4,v4);
					}
					else
					{
						ysD3dDev->AddXyzNomCol(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
	                                                   
						ysD3dDev->AddXyzNomCol(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x4,y4,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					}
				}

				ysD3dDev->FlushAll(primType);
			}
			break;


		case Ys2DDrawingElement::TRIANGLES:
			color=itm->dat.c;

			if(TRUE!=lighting)
			{
				if(YSTRUE!=first)
				{
					ysD3dDev->FlushAll(primType);
					first=YSFALSE;
				}
				lighting=TRUE;
				ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,lighting);
			}

			if(wire==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_LINELIST)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_LINELIST;

				for(i=0; i<=itm->dat.pnt.GetN()-3; i+=3)
				{
					double x1,y1,x2,y2,x3,y3;
					x1=itm->dat.pnt[i  ].x();
					y1=itm->dat.pnt[i  ].y();
					x2=itm->dat.pnt[i+1].x();
					y2=itm->dat.pnt[i+1].y();
					x3=itm->dat.pnt[i+2].x();
					y3=itm->dat.pnt[i+2].y();

					ysD3dDev->AddXyzNomCol(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzNomCol(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
                                                   
					ysD3dDev->AddXyzNomCol(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzNomCol(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
                                                   
					ysD3dDev->AddXyzNomCol(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					ysD3dDev->AddXyzNomCol(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
				}
			}

			if(fill==YSTRUE)
			{
				if(first!=YSTRUE && primType!=D3DPT_TRIANGLELIST)
				{
					ysD3dDev->FlushAll(primType);
				}
				Ys2DDrawing_SwitchTexture(mapMode,itm->dat.t,ownTexUnitPtr,commonGndTexUnitPtr,commonRwLightTexUnitPtr);

				first=YSFALSE;
				primType=D3DPT_TRIANGLELIST;

				for(i=0; i<=itm->dat.pnt.GetN()-3; i+=3)
				{
					double x1,y1,x2,y2,x3,y3;
					x1=itm->dat.pnt[i  ].x();
					y1=itm->dat.pnt[i  ].y();
					x2=itm->dat.pnt[i+1].x();
					y2=itm->dat.pnt[i+1].y();
					x3=itm->dat.pnt[i+2].x();
					y3=itm->dat.pnt[i+2].y();

					if(YSTRUE==useOwnTexture)
					{
						const double &u1=itm->dat.texCoord[i].x();
						const double &v1=itm->dat.texCoord[i].y();
						const double &u2=itm->dat.texCoord[i+1].x();
						const double &v2=itm->dat.texCoord[i+1].y();
						const double &u3=itm->dat.texCoord[i+2].x();
						const double &v3=itm->dat.texCoord[i+2].y();

						ysD3dDev->AddXyzNomColTex(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u1,v1);
						ysD3dDev->AddXyzNomColTex(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u2,v2);
						ysD3dDev->AddXyzNomColTex(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255,u3,v3);
					}
					else
					{
						ysD3dDev->AddXyzNomCol(primType,x1,y1,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x2,y2,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
						ysD3dDev->AddXyzNomCol(primType,x3,y3,0.0,nx,ny,nz,color.Ri(),color.Gi(),color.Bi(),255);
					}
				}
				ysD3dDev->FlushAll(primType);
			}
			break;
		}

	NEXTOBJ:
		;
	}

	if(first!=YSTRUE)
	{
		ysD3dDev->FlushAll(primType);
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);
	ysD3dDev->d3dDev->SetTexture(0,NULL);

	if(drawBbx==YSTRUE)
	{
		DrawBoundingBox(mapMode);
	}
	if(mapMode==YSTRUE)
	{
		ysD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&mapModeFlip);
	}

	YsD3dMakeIdentity(texTfm);
	ysD3dDev->d3dDev->SetTransform(D3DTS_TEXTURE0,&texTfm);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD);


	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU);
}



void Ys2DDrawing::DrawBoundingBox(YSBOOL mapMode)
{
	YsVec2 inflatedBbx[2],dgn,XZ;
	double l;
	dgn=bbx[1]-bbx[0];
	l=YsGreater(dgn.x(),dgn.y());
	XZ.Set(1.0,1.0);
	inflatedBbx[0]=bbx[0]-l*0.025*XZ;
	inflatedBbx[1]=bbx[1]+l*0.025*XZ;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	if(mapMode==YSTRUE)
	{
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[0].x(),0.0,inflatedBbx[0].y(),255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[0].x(),0.0,inflatedBbx[1].y(),255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[1].x(),0.0,inflatedBbx[1].y(),255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[1].x(),0.0,inflatedBbx[0].y(),255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[0].x(),0.0,inflatedBbx[0].y(),255,0,255,255);
	}
	else
	{
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[0].x(),inflatedBbx[0].y(),0.0,255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[0].x(),inflatedBbx[1].y(),0.0,255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[1].x(),inflatedBbx[1].y(),0.0,255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[1].x(),inflatedBbx[0].y(),0.0,255,0,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,inflatedBbx[0].x(),inflatedBbx[0].y(),0.0,255,0,255,255);
	}
	ysD3dDev->FlushAll(D3DPT_LINESTRIP);
}

void YsElevationGrid::AllocCache(void) const
{
}

void YsElevationGrid::DeleteCache(void) const
{
}

YSBOOL YsElevationGrid::IsCached(void) const
{
	return YSFALSE;
}

void YsElevationGrid::MakeCache(const double &,YSBOOL)
{
}

void YsElevationGrid::Draw
   (const double &plgColorScale,
    YSBOOL invert,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL shrinkTriangle,
    YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
	int i,j,baseIdx;
	double x,z;
	YsVec3 rc[4],*tri[6];

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

	for(j=0; j<nz; j++)
	{
		z=double(j)*zWid;
		x=0.0;
		baseIdx=(nx+1)*j;

		//  1  3
		//
		//  0  2
		rc[2].Set(x,node[baseIdx     ].y,z);
		rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
		for(i=0; i<nx; i++)
		{
			rc[0]=rc[2];
			rc[1]=rc[3];
			rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
			rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

			if(node[baseIdx+i].lup==YSTRUE)
			{
				// (3,1,2),(0,2,1)
				tri[0]=&rc[3];
				tri[1]=&rc[1];
				tri[2]=&rc[2];
				tri[3]=&rc[0];
				tri[4]=&rc[2];
				tri[5]=&rc[1];
			}
			else
			{
				// (1,0,3),(2,3,0)
				tri[0]=&rc[1];
				tri[1]=&rc[0];
				tri[2]=&rc[3];
				tri[3]=&rc[2];
				tri[4]=&rc[3];
				tri[5]=&rc[0];
			}
			int k;
			for(k=0; k<2; k++)
			{
				YsVec3 nom[3];
				YsElvGridFaceId nodId[3];
				GetTriangleNodeId(nodId,i,j,k);
				nom[0]=node[(nx+1)*nodId[0].z+nodId[0].x].nomOfNode;
				nom[1]=node[(nx+1)*nodId[1].z+nodId[1].x].nomOfNode;
				nom[2]=node[(nx+1)*nodId[2].z+nodId[2].x].nomOfNode;

				if(node[baseIdx+i].visible[k]==YSTRUE)
				{
					if(fill==YSTRUE)
					{
						// glNormal3dv(node[baseIdx+i].nom[k]);
						if(colorByElevation!=YSTRUE)
						{
							const YsColor &c=node[baseIdx+i].c[k];
							if(invert!=YSTRUE)
							{
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3  ],nom[0],c.Ri(),c.Gi(),c.Bi(),255);
								// glNormal3dv(nom[0]);
								// glVertex3dv(*tri[k*3]);
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+1],nom[1],c.Ri(),c.Gi(),c.Bi(),255);
								// glNormal3dv(nom[1]);
								// glVertex3dv(*tri[k*3+1]);
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+2],nom[2],c.Ri(),c.Gi(),c.Bi(),255);
								// glNormal3dv(nom[2]);
								// glVertex3dv(*tri[k*3+2]);
							}
							else
							{
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+2],nom[2],c.Ri(),c.Gi(),c.Bi(),255);
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+1],nom[1],c.Ri(),c.Gi(),c.Bi(),255);
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3  ],nom[0],c.Ri(),c.Gi(),c.Bi(),255);
							}
						}
						else
						{
							YsColor c;
							if(invert!=YSTRUE)
							{
								c=ColorByElevation(tri[k*3]->y());
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3  ],nom[0],c.Ri(),c.Gi(),c.Bi(),255);
								// glNormal3dv(nom[0]);
								// glVertex3dv(*tri[k*3]);
								c=ColorByElevation(tri[k*3+1]->y());
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+1],nom[1],c.Ri(),c.Gi(),c.Bi(),255);
								// glNormal3dv(nom[1]);
								// glVertex3dv(*tri[k*3+1]);
								c=ColorByElevation(tri[k*3+2]->y());
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+2],nom[2],c.Ri(),c.Gi(),c.Bi(),255);
								// glNormal3dv(nom[2]);
								// glVertex3dv(*tri[k*3+2]);
							}
							else
							{
								c=ColorByElevation(tri[k*3+2]->y());
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+2],nom[2],c.Ri(),c.Gi(),c.Bi(),255);
								c=ColorByElevation(tri[k*3+1]->y());
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3+1],nom[1],c.Ri(),c.Gi(),c.Bi(),255);
								c=ColorByElevation(tri[k*3]->y());
								ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,*tri[k*3  ],nom[0],c.Ri(),c.Gi(),c.Bi(),255);
							}
						}
						ysD3dDev->FlushXyzNomCol(D3DPT_TRIANGLELIST);
					}

					if(wire==YSTRUE)
					{
						const YsColor &c=node[baseIdx+i].c[k];

						ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
						ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,*tri[k*3  ],c.Ri(),c.Gi(),c.Bi(),255);
						ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,*tri[k*3+1],c.Ri(),c.Gi(),c.Bi(),255);
						ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,*tri[k*3+2],c.Ri(),c.Gi(),c.Bi(),255);
						ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,*tri[k*3  ],c.Ri(),c.Gi(),c.Bi(),255);
						ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
						ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);
					}
				}

				if(shrinkTriangle==YSTRUE)
				{
					YsVec3 shrink[3],cen;
					cen=((*tri[k*3])+(*tri[k*3+1])+(*tri[k*3+2]))/3.0;
					shrink[0]=cen+((*tri[k*3  ])-cen)*0.7;
					shrink[1]=cen+((*tri[k*3+1])-cen)*0.7;
					shrink[2]=cen+((*tri[k*3+2])-cen)*0.7;

					ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
					ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,shrink[0],0,0,0,255);
					ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,shrink[1],0,0,0,255);
					ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,shrink[2],0,0,0,255);
					ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,shrink[0],0,0,0,255);
					ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
					ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);
				}
			}
			x+=xWid;
		}
	}

	if(sideWall[0]==YSTRUE) // Bottom
	{
		double x,y0,y1;
		x=0.0;
		baseIdx=0;
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[0];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[1]==YSTRUE) // Right
	{
		double z,y0,y1;
		z=0.0;
		baseIdx=nx;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[1];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y1 ,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y1 ,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}
	if(sideWall[2]==YSTRUE) // Top
	{
		double x,y0,y1;
		x=0.0;
		baseIdx=nz*(nx+1);
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[2];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[3]==YSTRUE) // Left
	{
		double z,y0,y1;
		z=0.0;
		baseIdx=0;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[3];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y1 ,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y1 ,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}
	ysD3dDev->FlushXyzNomCol(D3DPT_TRIANGLELIST);

	if(drawBbx==YSTRUE)
	{
		DrawBoundingBox();
	}
}

void YsElevationGrid::DrawFastFillOnly(const double &plgColorScale)
{
	int i,j,baseIdx;
	double x,z;
	YsVec3 rc[4],*tri[6];


	DWORD prevCullMode;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetRenderState(D3DRS_CULLMODE,&prevCullMode);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

	// Adding depth bias to terrain was not such a good idea.  It makes artifacts.
	// float depthBias=+0.00001f;
	// ysD3dDev->d3dDev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,*(DWORD *)(&depthBias));
	// ysD3dDev->d3dDev->SetRenderState(D3DRS_DEPTHBIAS,*(DWORD *)(&depthBias));



	// Setting up texture >>
//	double invView[16];
//	invViewTfm.GetOpenGlCompatibleMatrix(invView);
//
//	D3DXMATRIXA16 texTfm
//	(
//		0.02F,0.00F,0.00F,0.00F,
//		0.00F,0.00F,0.02F,0.00F,
//		0.00F,0.02F,0.00F,0.00F,
//		0.00F,0.00F,0.00F,0.02F
//	);
//	D3DXMATRIXA16 autoTexTfm
//	(
//		(FLOAT)invView[ 0],(FLOAT)invView[ 1],(FLOAT)invView[ 2],(FLOAT)invView[ 3],
//		(FLOAT)invView[ 4],(FLOAT)invView[ 5],(FLOAT)invView[ 6],(FLOAT)invView[ 7],
//		(FLOAT)invView[ 8],(FLOAT)invView[ 9],(FLOAT)invView[10],(FLOAT)invView[11],
//		(FLOAT)invView[12],(FLOAT)invView[13],(FLOAT)invView[14],(FLOAT)invView[15]
//	);
//
//	ysD3dDev->d3dDev->SetTransform(D3DTS_TEXTURE0,&texTfm);
//	ysD3dDev->d3dDev->MultiplyTransform(D3DTS_TEXTURE0,&autoTexTfm);
//	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
//	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);
//
//	ysD3dDev->d3dDev->SetTexture(0,ysGroundTileTex);
//	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);


	YSBOOL useOwnTexture=YSFALSE;
	{
		if(0<texLabel.Strlen() && NULL!=owner)
		{
			auto scn=owner->GetOwner();
			auto texHd=scn->textureManager.FindTextureFromLabel(texLabel);
			if(NULL!=texHd)
			{
				texManCache=&scn->textureManager;
				texHdCache=texHd;
			}
		}
		if(NULL!=texManCache && NULL!=texHdCache)
		{
			auto tex=texManCache->GetTexture(texHdCache);
			useOwnTexture=(YSOK==tex->Bind() ? YSTRUE : YSFALSE);
		}

		ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
		ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU);
	}

	// Setting up texture <<



	for(j=0; j<nz; j++)
	{
		const double t0=(double)j/(double)nz;
		const double t1=(double)(j+1)/(double)nz;

		z=double(j)*zWid;
		x=0.0;
		baseIdx=(nx+1)*j;

		//  1  3
		//
		//  0  2
		rc[2].Set(x,node[baseIdx     ].y,z);
		rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
		for(i=0; i<nx; i++)
		{
			rc[0]=rc[2];
			rc[1]=rc[3];
			rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
			rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

			const double s0=(double)i/(double)nx;
			const double s1=(double)(i+1)/(double)nx;

			double texCoord[4][2]=
			{
				{s0,t0},{s0,t1},{s1,t0},{s1,t1}
			};
			double triTexCoord[6][2];

			if(YSTRUE!=useOwnTexture)
			{
				texCoord[0][0]=rc[0].x()*0.02;
				texCoord[0][1]=rc[0].z()*0.02;
				texCoord[1][0]=rc[1].x()*0.02;
				texCoord[1][1]=rc[1].z()*0.02;
				texCoord[2][0]=rc[2].x()*0.02;
				texCoord[2][1]=rc[2].z()*0.02;
				texCoord[3][0]=rc[3].x()*0.02;
				texCoord[3][1]=rc[3].z()*0.02;
			}

			if(node[baseIdx+i].lup==YSTRUE)
			{
				// (3,1,2),(0,2,1)
				tri[0]=&rc[3];
				tri[1]=&rc[1];
				tri[2]=&rc[2];
				tri[3]=&rc[0];
				tri[4]=&rc[2];
				tri[5]=&rc[1];

				triTexCoord[0][0]=texCoord[3][0];
				triTexCoord[0][1]=texCoord[3][1];
				triTexCoord[1][0]=texCoord[1][0];
				triTexCoord[1][1]=texCoord[1][1];
				triTexCoord[2][0]=texCoord[2][0];
				triTexCoord[2][1]=texCoord[2][1];
				triTexCoord[3][0]=texCoord[0][0];
				triTexCoord[3][1]=texCoord[0][1];
				triTexCoord[4][0]=texCoord[2][0];
				triTexCoord[4][1]=texCoord[2][1];
				triTexCoord[5][0]=texCoord[1][0];
				triTexCoord[5][1]=texCoord[1][1];
			}
			else
			{
				// (1,0,3),(2,3,0)
				tri[0]=&rc[1];
				tri[1]=&rc[0];
				tri[2]=&rc[3];
				tri[3]=&rc[2];
				tri[4]=&rc[3];
				tri[5]=&rc[0];


				triTexCoord[0][0]=texCoord[1][0];
				triTexCoord[0][1]=texCoord[1][1];
				triTexCoord[1][0]=texCoord[0][0];
				triTexCoord[1][1]=texCoord[0][1];
				triTexCoord[2][0]=texCoord[3][0];
				triTexCoord[2][1]=texCoord[3][1];
				triTexCoord[3][0]=texCoord[2][0];
				triTexCoord[3][1]=texCoord[2][1];
				triTexCoord[4][0]=texCoord[3][0];
				triTexCoord[4][1]=texCoord[3][1];
				triTexCoord[5][0]=texCoord[0][0];
				triTexCoord[5][1]=texCoord[0][1];
			}
			int k;
			for(k=0; k<2; k++)
			{
				YsVec3 nom[3];
				YsElvGridFaceId nodId[3];
				GetTriangleNodeId(nodId,i,j,k);
				nom[0]=node[(nx+1)*nodId[0].z+nodId[0].x].nomOfNode;
				nom[1]=node[(nx+1)*nodId[1].z+nodId[1].x].nomOfNode;
				nom[2]=node[(nx+1)*nodId[2].z+nodId[2].x].nomOfNode;

				if(node[baseIdx+i].visible[k]==YSTRUE)
				{
					if(colorByElevation!=YSTRUE)
					{
						const YsColor &c=node[baseIdx+i].c[k];
						ysD3dDev->AddXyzNomColTex(D3DPT_TRIANGLELIST,*tri[k*3  ],nom[0],c.Ri(),c.Gi(),c.Bi(),255,triTexCoord[k*3  ]);
						ysD3dDev->AddXyzNomColTex(D3DPT_TRIANGLELIST,*tri[k*3+1],nom[1],c.Ri(),c.Gi(),c.Bi(),255,triTexCoord[k*3+1]);
						ysD3dDev->AddXyzNomColTex(D3DPT_TRIANGLELIST,*tri[k*3+2],nom[2],c.Ri(),c.Gi(),c.Bi(),255,triTexCoord[k*3+2]);
					}
					else
					{
						YsColor c;
						c=ColorByElevation(tri[k*3]->y());
						ysD3dDev->AddXyzNomColTex(D3DPT_TRIANGLELIST,*tri[k*3  ],nom[0],c.Ri(),c.Gi(),c.Bi(),255,triTexCoord[k*3  ]);
						c=ColorByElevation(tri[k*3+1]->y());
						ysD3dDev->AddXyzNomColTex(D3DPT_TRIANGLELIST,*tri[k*3+1],nom[1],c.Ri(),c.Gi(),c.Bi(),255,triTexCoord[k*3+1]);
						c=ColorByElevation(tri[k*3+2]->y());
						ysD3dDev->AddXyzNomColTex(D3DPT_TRIANGLELIST,*tri[k*3+2],nom[2],c.Ri(),c.Gi(),c.Bi(),255,triTexCoord[k*3+2]);
					}
				}
			}
			x+=xWid;
		}
	}

	ysD3dDev->FlushXyzNomColTex(D3DPT_TRIANGLELIST);
	ysD3dDev->d3dDev->SetTexture(0,NULL);

	if(sideWall[0]==YSTRUE) // Bottom
	{
		double x,y0,y1;
		x=0.0;
		baseIdx=0;
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[0];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,0.0,0.0,0.0,-1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[1]==YSTRUE) // Right
	{
		double z,y0,y1;
		z=0.0;
		baseIdx=nx;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[1];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y1 ,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,0.0,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y1 ,z+zWid,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,double(nx)*xWid,y0 ,z     ,1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}
	if(sideWall[2]==YSTRUE) // Top
	{
		double x,y0,y1;
		x=0.0;
		baseIdx=nz*(nx+1);
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[2];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,0.0,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x     ,y0 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,x+xWid,y1 ,double(nz)*zWid,0.0,0.0,1.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[3]==YSTRUE) // Left
	{
		double z,y0,y1;
		z=0.0;
		baseIdx=0;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				const YsColor &c=sideWallColor[3];
				if(YsZero(y0)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y1 ,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);

				}
			}
			else
			{
				YsColor c;
				if(YsZero(y0)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					c=ColorByElevation(0.0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,0.0,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y0);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y0 ,z     ,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
					c=ColorByElevation(y1);
					ysD3dDev->AddXyzNomCol(D3DPT_TRIANGLELIST,0.0,y1 ,z+zWid,-1.0,0.0,0.0,c.Ri(),c.Gi(),c.Bi(),255);
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}

	ysD3dDev->FlushXyzNomCol(D3DPT_TRIANGLELIST);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,prevCullMode);

	// depthBias=0.0f;
	// ysD3dDev->d3dDev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,*(DWORD *)(&depthBias));
	// ysD3dDev->d3dDev->SetRenderState(D3DRS_DEPTHBIAS,*(DWORD *)(&depthBias));


	// Turning off texture >>
	ysD3dDev->d3dDev->SetTexture(0,NULL);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);

	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU);
	// Turning off texture <<
}

static void DrawQuadAsTwoTri(const YsVec3 &q0,const YsVec3 &q1,const YsVec3 &q2,const YsVec3 &q3)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,q0,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,q1,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,q2,0,0,0,255);

	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,q2,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,q3,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,q0,0,0,0,255);
}

void YsElevationGrid::DrawProtectPolygon(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(hasProtectPolygon!=YSFALSE)
	{
		int i,j,baseIdx;
		double x,z;
		YsVec3 rc[4],*tri[6];


		DWORD prevCullMode;
		ysD3dDev->d3dDev->GetRenderState(D3DRS_CULLMODE,&prevCullMode);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);


		// float depthBias=+0.00001f;
		// ysD3dDev->d3dDev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,*(DWORD *)(&depthBias));
		// ysD3dDev->d3dDev->SetRenderState(D3DRS_DEPTHBIAS,*(DWORD *)(&depthBias));


		for(j=0; j<nz; j++)
		{
			z=double(j)*zWid;
			x=0.0;
			baseIdx=(nx+1)*j;

			//  1  3
			//
			//  0  2
			rc[2].Set(x,node[baseIdx     ].y,z);
			rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
			for(i=0; i<nx; i++)
			{
				rc[0]=rc[2];
				rc[1]=rc[3];
				rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
				rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

				if(node[baseIdx+i].lup==YSTRUE)
				{
					// (3,1,2),(0,2,1)
					tri[0]=&rc[3];
					tri[1]=&rc[1];
					tri[2]=&rc[2];
					tri[3]=&rc[0];
					tri[4]=&rc[2];
					tri[5]=&rc[1];
				}
				else
				{
					// (1,0,3),(2,3,0)
					tri[0]=&rc[1];
					tri[1]=&rc[0];
					tri[2]=&rc[3];
					tri[3]=&rc[2];
					tri[4]=&rc[3];
					tri[5]=&rc[0];
				}
				int k;
				for(k=0; k<2; k++)
				{
					YsVec3 btm[3],cen,cenBtm;
					YsElvGridFaceId nodId[3];
					GetTriangleNodeId(nodId,i,j,k);

					if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
					{
						hasProtectPolygon=YSTRUE;

						ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,*tri[k*3+0],0,0,0,255);
						ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,*tri[k*3+1],0,0,0,255);
						ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,*tri[k*3+2],0,0,0,255);

						// Side walls
						btm[0].Set(tri[k*3  ]->x(),0.0,tri[k*3  ]->z());
						btm[1].Set(tri[k*3+1]->x(),0.0,tri[k*3+1]->z());
						btm[2].Set(tri[k*3+2]->x(),0.0,tri[k*3+2]->z());

						cen=(*tri[k*3]+*tri[k*3+1]+*tri[k*3+2])/3.0;
						cenBtm.Set(cen.x(),0.0,cen.z());

						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+0],btm[0]);
						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+1],btm[1]);
						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+2],btm[2]);
					}
				}
				x+=xWid;
			}
		}

		if(hasProtectPolygon==YSTFUNKNOWN)
		{
			hasProtectPolygon=YSFALSE;
		}

		ysD3dDev->FlushXyzCol(D3DPT_TRIANGLELIST);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,prevCullMode);

		// depthBias=0.0f;
		// ysD3dDev->d3dDev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,*(DWORD *)(&depthBias));
		// ysD3dDev->d3dDev->SetRenderState(D3DRS_DEPTHBIAS,*(DWORD *)(&depthBias));
	}
}


// See YSFLIGHT/document/20081116.jnt
void YsElevationGrid::DrawClippedProtectPolygon(
    const YsVec3 &cameraPos,const YsPlane &clipPln,const YsPlane &nearPln,const YsVec3 &t0,const YsVec3 &t1,const YsVec3 &t2)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	int nClp1,nClp2;
	YsVec3 clp1[5],clp2[5];
	YsVec3 const *tri[4];
	int i;

	tri[0]=&t0;
	tri[1]=&t1;
	tri[2]=&t2;
	tri[3]=&t0;

	// Clipping the polygon by clipPln (Extract only forward part)
	nClp1=0;
	for(i=0; i<3 && nClp1<4; i++)
	{
		if(clipPln.GetSideOfPlane(*tri[i])>=0)
		{
			clp1[nClp1++]=*tri[i];
			if(clipPln.GetSideOfPlane(*tri[i+1])<0)  // Going out
			{
				if(clipPln.GetIntersection(clp1[nClp1],*tri[i],*tri[i+1]-*tri[i])==YSOK)
				{
					nClp1++;
				}
			}
		}
		else if(clipPln.GetSideOfPlane(*tri[i+1])>=0)  // Coming in
		{
			if(clipPln.GetIntersection(clp1[nClp1],*tri[i],*tri[i+1]-*tri[i])==YSOK)
			{
				nClp1++;
			}
		}
	}
	if(nClp1==0)
	{
		return;
	}
	clp1[nClp1]=clp1[0];

	// Clipping the polygon by nearPln (Extract only rear part)
	nClp2=0;
	for(i=0; i<nClp1 && nClp2<5; i++)
	{
		if(nearPln.GetSideOfPlane(clp1[i])<0)
		{
			clp2[nClp2++]=clp1[i];
			if(nearPln.GetSideOfPlane(clp1[i+1])>=0)  // Going out
			{
				if(nearPln.GetIntersection(clp2[nClp2],clp1[i],clp1[i+1]-clp1[i])==YSOK)
				{
					nClp2++;
				}
			}
		}
		else if(nearPln.GetSideOfPlane(clp1[i+1])<0)  // Coming in
		{
			if(nearPln.GetIntersection(clp2[nClp2],clp1[i],clp1[i+1]-clp1[i])==YSOK)
			{
				nClp2++;
			}
		}
	}


	// Extend the polygon to nearPln
	for(i=0; i<nClp2; i++)
	{
		if(nearPln.GetSideOfPlane(clp2[i])<0)
		{
			nearPln.GetIntersection(clp2[i],cameraPos,clp2[i]-cameraPos);
		}
	}


	// Then draw
	if(nClp2>=3)
	{
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[0],0,0,0,255);
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[1],0,0,0,255);
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[2],0,0,0,255);

		if(nClp2>=4)
		{
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[0],0,0,0,255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[2],0,0,0,255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[3],0,0,0,255);
		}
		if(nClp2>=5)
		{
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[0],0,0,0,255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[3],0,0,0,255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,clp2[4],0,0,0,255);
		}
	}
}

void YsElevationGrid::DrawProtectPolygonAccurate(const YsMatrix4x4 &evgTfm,const double &nearZ)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	if(hasProtectPolygon!=YSFALSE)
	{
		int i,j,baseIdx;
		double x,z;
		YsVec3 rc[4],*tri[6];



		// See YSFLIGHT/document/20081116.jnt for why cameraPos, nearPln, and clipPln are needed.
		// posInCameraCoord=viewMdlMat*posInEvgCoord
		YsMatrix4x4 camToEvg(evgTfm);
		YsPlane nearPln,clipPln;
		YsVec3 cameraPos,org,nom;

		camToEvg.Invert();  // Now posInEvgCoord=camToEvg*posInCameraCoord

		camToEvg.Mul(cameraPos,YsOrigin(),1.0);
		org.Set(0.0,0.0,nearZ*1.001);
		nom.Set(0.0,0.0,1.0);
		camToEvg.Mul(org,org,1.0);
		camToEvg.Mul(nom,nom,0.0);
		nearPln.Set(org,nom);  // Side<0 means a point is closer than the near plane.

		org.Set(0.0,0.0,0.5);
		camToEvg.Mul(org,org,1.0);
		clipPln.Set(org,nom);



		DWORD prevCullMode;
		ysD3dDev->d3dDev->GetRenderState(D3DRS_CULLMODE,&prevCullMode);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);



		for(j=0; j<nz; j++)
		{
			z=double(j)*zWid;
			x=0.0;
			baseIdx=(nx+1)*j;

			//  1  3
			//
			//  0  2
			rc[2].Set(x,node[baseIdx     ].y,z);
			rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
			for(i=0; i<nx; i++)
			{
				rc[0]=rc[2];
				rc[1]=rc[3];
				rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
				rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

				if(node[baseIdx+i].lup==YSTRUE)
				{
					// (3,1,2),(0,2,1)
					tri[0]=&rc[3];
					tri[1]=&rc[1];
					tri[2]=&rc[2];
					tri[3]=&rc[0];
					tri[4]=&rc[2];
					tri[5]=&rc[1];
				}
				else
				{
					// (1,0,3),(2,3,0)
					tri[0]=&rc[1];
					tri[1]=&rc[0];
					tri[2]=&rc[3];
					tri[3]=&rc[2];
					tri[4]=&rc[3];
					tri[5]=&rc[0];
				}
				int k;
				for(k=0; k<2; k++)
				{
					YsVec3 btm[3],cen,cenBtm;
					YsElvGridFaceId nodId[3];
					GetTriangleNodeId(nodId,i,j,k);

					if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
					{
						hasProtectPolygon=YSTRUE;

						ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,*tri[k*3+0],0,0,0,255);
						ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,*tri[k*3+1],0,0,0,255);
						ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,*tri[k*3+2],0,0,0,255);


						if(nearPln.GetSideOfPlane(*tri[k*3  ])<0 ||
						   nearPln.GetSideOfPlane(*tri[k*3+1])<0 ||
						   nearPln.GetSideOfPlane(*tri[k*3+2])<0)
						{
							// See YSFLIGHT/document/20081116.jnt
							DrawClippedProtectPolygon(cameraPos,clipPln,nearPln,*tri[k*3],*tri[k*3+1],*tri[k*3+2]);
						}


						// Side walls
						btm[0].Set(tri[k*3  ]->x(),0.0,tri[k*3  ]->z());
						btm[1].Set(tri[k*3+1]->x(),0.0,tri[k*3+1]->z());
						btm[2].Set(tri[k*3+2]->x(),0.0,tri[k*3+2]->z());

						cen=(*tri[k*3]+*tri[k*3+1]+*tri[k*3+2])/3.0;
						cenBtm.Set(cen.x(),0.0,cen.z());

						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+0],btm[0]);
						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+1],btm[1]);
						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+2],btm[2]);
					}
				}
				x+=xWid;
			}
		}

		if(hasProtectPolygon==YSTFUNKNOWN)
		{
			hasProtectPolygon=YSFALSE;
		}

		ysD3dDev->FlushXyzCol(D3DPT_TRIANGLELIST);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,prevCullMode);
	}
}

void YsElevationGrid::DrawBoundingBox(void)
{
	const YsColor &c=YsCyan();

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
}

void YsSceneryShell::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
//	auto ysD3dDev=YsD3dDevice::GetCurrent();
//	if(drawBbx==YSTRUE)
//	{
//		YsVec3 bbx[2];
//		shl.GetBoundingBox(bbx[0],bbx[1]);
//
//		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
//
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[0].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[0].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
//
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[0].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[1].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[0].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[1].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[0].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[1].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[0].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[1].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
//
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[1].y(),bbx[1].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[1].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[0].z(),255,255,255,255);
//		ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
//	}
//
//	shl.Draw();
}

void YsSceneryRectRegion::Draw(void)
{
}

void YsSceneryGndObj::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
}

void YsSceneryPointSet::Draw(void)
{
}

void YsSceneryPointSet::DrawStar(void)
{
}

void YsScenery::GlSetColor(const YsColor &col)
{
}

void YsScenery::DrawBoundingBox(void)
{
	YsVec3 bbx[2],dgn,XZ;
	double l;
	GetBoundingBox(bbx);
	dgn=bbx[1]-bbx[0];
	l=YsGreater(YsGreater(dgn.x(),dgn.y()),dgn.z());
	XZ.Set(1.0,0.0,1.0);
	bbx[0]-=l*0.05*XZ;
	bbx[1]+=l*0.05*XZ;


	const YsColor &c=YsDarkYellow();

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[0].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[0].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[0].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,bbx[1].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[1].y(),bbx[1].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[1].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,bbx[0].x(),bbx[1].y(),bbx[0].z(),c.Ri(),c.Gi(),c.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
}

void YsScenery::Draw
	   (YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryPointSet> *pst;
	YsListItem <YsScenery> *scn;


	D3DMATRIX pushMatrix;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
	ysD3dDev->MultiplyModelMatrix(pos.x(),pos.y(),pos.z(),att.h(),att.p(),att.b());

	if(drawScn==YSTRUE)
	{
		if(drawBbx==YSTRUE)
		{
			DrawBoundingBox();
		}
	}


	if(drawShl==YSTRUE)
	{
		shl=NULL;
		while((shl=shlList.FindNext(shl))!=NULL)
		{
			// shl->dat.shl.Draw(viewTfm,nearZ,farZ,tanFov,shl->dat.pos,shl->dat.att,YsVisual::DRAWALL);

			// Old code for reference.
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
			ysD3dDev->MultiplyModelMatrix
			   (shl->dat.pos.x(),shl->dat.pos.y(),shl->dat.pos.z(),
			    shl->dat.att.h(),shl->dat.att.p(),shl->dat.att.b());
			shl->dat.Draw(wire,fill,drawBbx);
			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawEvg==YSTRUE)
	{
		evg=NULL;
		while((evg=evgList.FindNext(evg))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
			ysD3dDev->MultiplyModelMatrix
			   (evg->dat.pos.x(),evg->dat.pos.y(),evg->dat.pos.z(),
			    evg->dat.att.h(),evg->dat.att.p(),evg->dat.att.b());
			evg->dat.evg.Draw(plgColorScale,YSFALSE,wire,fill,drawBbx,drawShrink,YSFALSE,YSFALSE);
			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawSbd==YSTRUE)
	{
		drw=NULL;
		while((drw=sbdList.FindNext(drw))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
			ysD3dDev->MultiplyModelMatrix
			   (drw->dat.pos.x(),drw->dat.pos.y(),drw->dat.pos.z(),
			    drw->dat.att.h(),drw->dat.att.p(),drw->dat.att.b());
			drw->dat.drw.Draw(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,YSFALSE,-1.0);
			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawRgn==YSTRUE)
	{
		rgn=NULL;
		while((rgn=rgnList.FindNext(rgn))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
			ysD3dDev->MultiplyModelMatrix
			   (rgn->dat.pos.x(),rgn->dat.pos.y(),rgn->dat.pos.z(),
			    rgn->dat.att.h(),rgn->dat.att.p(),rgn->dat.att.b());
			YsScenery::GlSetColor(YsThreeBitColor(rgn->dat.id%8));
			rgn->dat.Draw();
			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawGndObj==YSTRUE)
	{
		gnd=NULL;
		while((gnd=gndList.FindNext(gnd))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
			ysD3dDev->MultiplyModelMatrix
			   (gnd->dat.pos.x(),gnd->dat.pos.y(),gnd->dat.pos.z(),
			    gnd->dat.att.h(),gnd->dat.att.p(),gnd->dat.att.b());
			gnd->dat.Draw(wire,fill,drawBbx);
			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	// if(drawPst==YSTRUE)
	{
		pst=NULL;
		while((pst=pstList.FindNext(pst))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
			ysD3dDev->MultiplyModelMatrix
			   (pst->dat.pos.x(),pst->dat.pos.y(),pst->dat.pos.z(),
			    pst->dat.att.h(),pst->dat.att.p(),pst->dat.att.b());
			pst->dat.Draw();
			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.Draw(wire,fill,drawBbx,drawShrink,drawShl,drawEvg,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);
	}

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
}

void YsScenery::DrawMap(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsScenery> *scn;

	D3DMATRIX pushMatrix;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
	ysD3dDev->MultiplyModelMatrix(pos.x(),pos.y(),pos.z(),att.h(),att.p(),att.b());

	drw=NULL;
	while((drw=mapList.FindNext(drw))!=NULL)
	{
		D3DMATRIX pushMatrix;
		ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
		ysD3dDev->MultiplyModelMatrix
		   (drw->dat.pos.x(),drw->dat.pos.y(),drw->dat.pos.z(),
		    drw->dat.att.h(),drw->dat.att.p(),drw->dat.att.b());
		drw->dat.drw.Draw(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSTRUE,wire,fill,drawBbx,YSFALSE,-1.0);
		ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.DrawMap(wire,fill,drawBbx);
	}

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
}

void YsScenery::DrawProtectPolygon(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,const double nearZ,
    const double &currentTime)
{
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery> *scn;
	YsMatrix4x4 viewModelTfm(YSFALSE),newModelTfm(YSFALSE),evgTfm(YSFALSE);

	D3DMATRIX pushMatrix;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
	ysD3dDev->MultiplyModelMatrix(pos.x(),pos.y(),pos.z(),att.h(),att.p(),att.b());

	newModelTfm=modelTfm;
	newModelTfm.Translate(pos);
	newModelTfm.RotateXZ(att.h());
	newModelTfm.RotateZY(att.p());
	newModelTfm.RotateXY(att.b());

	viewModelTfm=viewTfm*newModelTfm;


	invViewTfm=viewTfm;
	invViewTfm.Invert();


	ysD3dDev->d3dDev->SetRenderState(D3DRS_COLORWRITEENABLE,0);


	evg=NULL;
	while((evg=evgList.FindNext(evg))!=NULL)
	{
		if(evg->dat.evg.hasProtectPolygon!=YSFALSE &&
		   IsItemVisible(viewModelTfm,projTfm,&evg->dat)==YSTRUE)
		{
			evgTfm=viewModelTfm;
			evgTfm.Translate(evg->dat.pos);
			evgTfm.RotateXZ(evg->dat.att.h());
			evgTfm.RotateZY(evg->dat.att.p());
			evgTfm.RotateXY(evg->dat.att.b());


			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (evg->dat.pos.x(),evg->dat.pos.y(),evg->dat.pos.z(),
			    evg->dat.att.h(),evg->dat.att.p(),evg->dat.att.b());
			evg->dat.evg.DrawProtectPolygonAccurate(evgTfm,nearZ);

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&scn->dat)==YSTRUE)
		{
			scn->dat.DrawProtectPolygon(viewTfm,newModelTfm,projTfm,nearZ,currentTime);
		}
	}

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);

	ysD3dDev->d3dDev->SetRenderState(
	    D3DRS_COLORWRITEENABLE,
	    D3DCOLORWRITEENABLE_ALPHA|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);
}

int YsScenery::numSceneryDrawn;

void YsScenery::DrawVisual
    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,const double &currentTime,YSBOOL forShadowMap)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsScenery> *scn;
	const YSBOOL wire=YSTRUE,fill=YSTRUE,drawBbx=YSFALSE,drawShrink=YSFALSE;
	YsMatrix4x4 viewModelTfm(YSFALSE),newModelTfm(YSFALSE),shlTfm(YSFALSE);

	D3DMATRIX pushMatrix;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
	ysD3dDev->MultiplyModelMatrix(pos.x(),pos.y(),pos.z(),att.h(),att.p(),att.b());

	newModelTfm=modelTfm;
	newModelTfm.Translate(pos);
	newModelTfm.RotateXZ(att.h());
	newModelTfm.RotateZY(att.p());
	newModelTfm.RotateXY(att.b());

	viewModelTfm=viewTfm*newModelTfm;


	invViewTfm=viewTfm;
	invViewTfm.Invert();


	numSceneryDrawn++;

	shl=NULL;
	while((shl=shlList.FindNext(shl))!=NULL)
	{
		shl->dat.shl.Draw(viewModelTfm,projTfm,shl->dat.pos,shl->dat.att,YsVisual::DRAWALL);
	}

	evg=NULL;
	while((evg=evgList.FindNext(evg))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&evg->dat)==YSTRUE)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (evg->dat.pos.x(),evg->dat.pos.y(),evg->dat.pos.z(),
			    evg->dat.att.h(),evg->dat.att.p(),evg->dat.att.b());
			evg->dat.evg.DrawFastFillOnly(plgColorScale);

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	drw=NULL;
	while((drw=sbdList.FindNext(drw))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&drw->dat)==YSTRUE)
		{
			shlTfm=viewModelTfm;
			shlTfm.Translate(drw->dat.pos);
			shlTfm.RotateXZ(drw->dat.att.h());
			shlTfm.RotateZY(drw->dat.att.p());
			shlTfm.RotateXY(drw->dat.att.b());

			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (drw->dat.pos.x(),drw->dat.pos.y(),drw->dat.pos.z(),
			    drw->dat.att.h(),drw->dat.att.p(),drw->dat.att.b());
			drw->dat.drw.Draw
			(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,YSFALSE,currentTime,&shlTfm);

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&scn->dat)==YSTRUE)
		{
			scn->dat.DrawVisual(viewTfm,newModelTfm,projTfm,currentTime,forShadowMap);
		}
	}

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
}

void YsScenery::DrawMapVisual
    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &,const YsMatrix4x4 &projTfm,
     const double &,const double &,YSBOOL drawPset,const double &currentTime)
{
	invViewTfm=viewTfm;
	invViewTfm.Invert();

	YsMatrix4x4 shlTfm(YSFALSE);

	numSceneryDrawn++;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	DWORD prevCullMode;
	ysD3dDev->d3dDev->GetRenderState(D3DRS_CULLMODE,&prevCullMode);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	D3DMATRIX pushMatrix;
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);
	ysD3dDev->MultiplyModelMatrix(pos.x(),pos.y(),pos.z(),att.h(),att.p(),att.b());

	const YSBOOL wire=YSFALSE,fill=YSTRUE,drawBbx=YSFALSE;

	for(auto &samePlaneMapGroup : mapDrawingOrderCache.samePlaneMapGroup)
	{
		for(int i=0; i<2; ++i)
		{
			if(0==i)
			{
				ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
				ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
				ysD3dDev->d3dDev->SetRenderState(D3DRS_COLORWRITEENABLE,0x0f);
				ysD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
			}
			else
			{
				ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
				ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
				ysD3dDev->d3dDev->SetRenderState(D3DRS_COLORWRITEENABLE,0);
				ysD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
			}

			for(auto &mapDrawingInfo : samePlaneMapGroup.mapDrawingInfo)
			{
				auto viewModelTfm=viewTfm*mapDrawingInfo.mapOwnerToWorldTfm;
				if(IsItemVisible(viewModelTfm,projTfm,mapDrawingInfo.mapPtr)==YSTRUE)
				{
					YsMatrix4x4 mapTfm=mapDrawingInfo.mapOwnerToWorldTfm;
					mapTfm.Translate(mapDrawingInfo.mapPtr->pos);
					mapTfm.RotateXZ(mapDrawingInfo.mapPtr->att.h());
					mapTfm.RotateZY(mapDrawingInfo.mapPtr->att.p());
					mapTfm.RotateXY(mapDrawingInfo.mapPtr->att.b());

					YsMatrix4x4 shlTfm=viewTfm*mapTfm;

					D3DMATRIX pushMatrix;
					ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

					// D3D stores View matrix and Modeling matrix separately.
					D3DMATRIX mapTfmD3d;
					YsD3dSetMatrix(mapTfmD3d,mapTfm);
					ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&mapTfmD3d);

					mapDrawingInfo.mapPtr->GetDrawing().Draw
					    (plgColorScale,linColorScale,pntColorScale,drawPset,YSTRUE,wire,fill,drawBbx,YSFALSE,
					     currentTime,&shlTfm);

					ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
				}
			}
		}
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_COLORWRITEENABLE,0x0f);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,prevCullMode);

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
}


void YsScenery::DrawAxis
   (const double &axsSize,
    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsScenery> *scn;
	YsListItem <YsSceneryGndObj> *gnd;

	auto ysD3dDev=YsD3dDevice::GetCurrent();

	if(drawMap==YSTRUE)
	{
		drw=NULL;
		while((drw=mapList.FindNext(drw))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (drw->dat.pos.x(),drw->dat.pos.y(),drw->dat.pos.z(),
			    drw->dat.att.h(),drw->dat.att.p(),drw->dat.att.b());
			YsD3dDrawAxis(axsSize,YsMagenta());

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}



	if(drawShl==YSTRUE)
	{
		shl=NULL;
		while((shl=shlList.FindNext(shl))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (shl->dat.pos.x(),shl->dat.pos.y(),shl->dat.pos.z(),
			    shl->dat.att.h(),shl->dat.att.p(),shl->dat.att.b());
			YsD3dDrawAxis(axsSize,YsWhite());

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawEvg==YSTRUE)
	{
		evg=NULL;
		while((evg=evgList.FindNext(evg))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (evg->dat.pos.x(),evg->dat.pos.y(),evg->dat.pos.z(),
			    evg->dat.att.h(),evg->dat.att.p(),evg->dat.att.b());
			YsD3dDrawAxis(axsSize,YsCyan());

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawSbd==YSTRUE)
	{
		drw=NULL;
		while((drw=sbdList.FindNext(drw))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (drw->dat.pos.x(),drw->dat.pos.y(),drw->dat.pos.z(),
			    drw->dat.att.h(),drw->dat.att.p(),drw->dat.att.b());
			YsD3dDrawAxis(axsSize,YsMagenta());

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawRgn==YSTRUE)
	{
		rgn=NULL;
		while((rgn=rgnList.FindNext(rgn))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (rgn->dat.pos.x(),rgn->dat.pos.y(),rgn->dat.pos.z(),
			    rgn->dat.att.h(),rgn->dat.att.p(),rgn->dat.att.b());
			YsScenery::GlSetColor(YsThreeBitColor(rgn->dat.id%8));
			YsD3dDrawAxis(axsSize,YsGreen());

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	if(drawGndObj==YSTRUE)
	{
		gnd=NULL;
		while((gnd=gndList.FindNext(gnd))!=NULL)
		{
			D3DMATRIX pushMatrix;
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

			ysD3dDev->MultiplyModelMatrix
			   (gnd->dat.pos.x(),gnd->dat.pos.y(),gnd->dat.pos.z(),
			    gnd->dat.att.h(),gnd->dat.att.p(),gnd->dat.att.b());
			YsD3dDrawAxis(axsSize,YsThreeBitColor(gnd->dat.iff+1));

			ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
		}
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		D3DMATRIX pushMatrix;
		ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

		ysD3dDev->MultiplyModelMatrix
		   (scn->dat.pos.x(),scn->dat.pos.y(),scn->dat.pos.z(),
		    scn->dat.att.h(),scn->dat.att.p(),scn->dat.att.b());
		if(drawScn==YSTRUE)
		{
			YsD3dDrawAxis(axsSize,YsDarkYellow());
		}
		scn->dat.DrawAxis(axsSize,drawShl,drawEvg,drawMap,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);

		ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
	}
}

void YsScenery::DrawItemAxis(const YsSceneryItem *itm,const double &axsSize)
{
	YsScenery *scn;
	YsArray <YsScenery *,16> parentLink;
	int i;

	D3DMATRIX pushMatrix;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

	scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}
	for(i=(int)parentLink.GetN()-1; i>=0; i--)
	{
		ysD3dDev->MultiplyModelMatrix
		   (parentLink[i]->pos.x(),parentLink[i]->pos.y(),parentLink[i]->pos.z(),
		    parentLink[i]->att.h(),parentLink[i]->att.p(),parentLink[i]->att.b());
	}
	ysD3dDev->MultiplyModelMatrix
	   (itm->pos.x(),itm->pos.y(),itm->pos.z(),
	    itm->att.h(),itm->att.p(),itm->att.b());

	YsD3dDrawAxis(axsSize,YsWhite());

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
}

void YsScenery::DrawItem
   (const YsSceneryItem *itm,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
    YSBOOL name2DElem,YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
	YsArray <YsScenery *,16> parentLink;
	int i;

	D3DMATRIX pushMatrix;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

	YsScenery *scn;
	scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}
	for(i=(int)parentLink.GetN()-1; i>=0; i--)
	{
		ysD3dDev->MultiplyModelMatrix
		   (parentLink[i]->pos.x(),parentLink[i]->pos.y(),parentLink[i]->pos.z(),
		    parentLink[i]->att.h(),parentLink[i]->att.p(),parentLink[i]->att.b());
	}
	ysD3dDev->MultiplyModelMatrix
	   (itm->pos.x(),itm->pos.y(),itm->pos.z(),
	    itm->att.h(),itm->att.p(),itm->att.b());

	switch(itm->objType)
	{
	case YsSceneryItem::SHELL:
		((YsSceneryShell *)itm)->Draw(wire,fill,drawBbx);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		((YsSceneryElevationGrid *)itm)->evg.Draw
		    (plgColorScale,YSFALSE,wire,fill,drawBbx,drawShrink,nameElvGridFace,nameElvGridNode);
		break;
	case YsSceneryItem::MAP:
		((YsScenery2DDrawing *)itm)->drw.Draw
		    (plgColorScale,linColorScale,pntColorScale,YSTRUE,YSTRUE,wire,fill,drawBbx,name2DElem,-1.0);
		break;
	case YsSceneryItem::SIGNBOARD:
		((YsScenery2DDrawing *)itm)->drw.Draw
		    (plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,name2DElem,-1.0);
		break;
	case YsSceneryItem::RECTREGION:
		YsScenery::GlSetColor(YsThreeBitColor(((YsSceneryRectRegion *)itm)->id%8));
		((YsSceneryRectRegion *)itm)->Draw();
		break;
	case YsSceneryItem::GROUNDOBJECT:
		((YsSceneryGndObj *)itm)->Draw(wire,fill,drawBbx);
		break;
	case YsSceneryItem::SUBSCENARY:
		if(drawBbx==YSTRUE)
		{
			((YsScenery *)itm)->DrawBoundingBox();
		}
		break;
	case YsSceneryItem::POINTSET:
		((YsSceneryPointSet *)itm)->Draw();
		break;
	}

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
}

void YsScenery::DrawItemStar(const YsSceneryItem *itm)
{
	YsArray <YsScenery *,16> parentLink;
	int i;

	D3DMATRIX pushMatrix;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

	YsScenery *scn;
	scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}
	for(i=(int)parentLink.GetN()-1; i>=0; i--)
	{
		ysD3dDev->MultiplyModelMatrix
		   (parentLink[i]->pos.x(),parentLink[i]->pos.y(),parentLink[i]->pos.z(),
		    parentLink[i]->att.h(),parentLink[i]->att.p(),parentLink[i]->att.b());
	}
	ysD3dDev->MultiplyModelMatrix
	   (itm->pos.x(),itm->pos.y(),itm->pos.z(),
	    itm->att.h(),itm->att.p(),itm->att.b());

	if(itm->objType==YsSceneryItem::POINTSET)
	{
		((YsSceneryPointSet *)itm)->DrawStar();
	}

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);
}

void YsScenery::Draw2DDrawingElement
   (YsScenery2DDrawing *drw,const Ys2DDrawingElement *itm,
    YSBOOL nameVtId,YSBOOL wire,YSBOOL points)
{
}

void YsScenery::DrawILSGuideline(void)
{
}

void YsScenery::DrawItemILSGuideline(YsSceneryGndObj *gnd)
{
}
