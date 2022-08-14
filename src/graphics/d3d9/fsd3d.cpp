#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fs.h"
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#include <fsgui.h>

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"

#include <ysbitmap.h>

#include <time.h>

#include "graphics/common/fsfontrenderer.h"


extern const char *FsProgramTitle;  // Defined in fsmain.cpp

const char *FsMainWindowTitle(void)
{
	// As of 2014/12/12  Main window title must include keyword "Main Window" so that FsWin32GetMainWindowHandle can find the handle.
	static YsString windowTitle;
	windowTitle.Set(FsProgramTitle);
	windowTitle.Append(" Main Window");
	windowTitle.Append(" (Direct3D9)");
	return windowTitle;
}



YSBOOL FsIsConsoleServer(void)
{
	return YSFALSE;
}

YSBOOL FsIsShadowMapAvailable(void)
{
	return YSFALSE;
}

YSBOOL FsIsPointSpriteAvailable(void)
{
	return YSFALSE;
}


static LARGE_INTEGER frmClock1,frmClock2;


void FsInitializeOpenGL(void)
{
}

void FsReinitializeOpenGL(void)
{
	FsInitializeOpenGL();
}

void FsUninitializeOpenGL(void)
{
}

void FsClearScreenAndZBuffer(const YsColor &clearColor)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		int r,g,b;
		r=clearColor.Ri();
		g=clearColor.Gi();
		b=clearColor.Bi();
		if(YSTRUE==FsIsMainWindowActive() && YSTRUE!=FsIsMainWindowSplit())
		{
			ysD3dDev->d3dDev->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(r,g,b),1.0F,0);
		}
		else
		{
			int x0,y0,wid,hei;
			FsGetWindowViewport(x0,y0,wid,hei);

			D3DRECT rect;
			rect.x1=x0;
			rect.y1=y0;
			rect.x2=x0+wid-1;
			rect.y2=y0+hei-1;

			ysD3dDev->d3dDev->Clear(1,&rect,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL,D3DCOLOR_XRGB(r,g,b),1.0F,0);
		}
		ysD3dDev->d3dDev->BeginScene();

		frmClock1=frmClock2;
		QueryPerformanceCounter(&frmClock2);
		// printf("* %d\n",frmClock2.LowPart-frmClock1.LowPart);

		D3DMATRIX identity;
		YsD3dMakeIdentity(identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_VIEW,&identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_PROJECTION,&identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&identity);

		ysD3dDev->d3dDev->SetRenderState(D3DRS_NORMALIZENORMALS,FALSE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,D3DMCS_COLOR1);

		ysD3dDev->d3dDev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,D3DMCS_COLOR1);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,D3DMCS_COLOR1);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_COLOR1);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_COLOR2);

		ysD3dDev->d3dDev->SetRenderState(D3DRS_STENCILPASS,D3DSTENCILOP_INCR);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_STENCILENABLE,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_ALWAYS);

		// ysD3dDev->d3dSprite->SetTransform(&identity);  // 2011/02/05 Just for safety

		D3DMATERIAL9 material;                                  //2007/04/08
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);  //2007/04/08
		ZeroMemory(&material,sizeof(material));                 //2007/04/08
		ysD3dDev->d3dDev->SetMaterial(&material);               //2007/04/08
	}
}

void FsClearStencilBuffer(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		if(YSTRUE==FsIsMainWindowActive() && YSTRUE!=FsIsMainWindowSplit())
		{
			ysD3dDev->d3dDev->Clear(0,NULL,D3DCLEAR_STENCIL,D3DCOLOR_XRGB(0,0,0),1.0F,0);
		}
		else
		{
			int x0,y0,wid,hei;
			FsGetWindowViewport(x0,y0,wid,hei);

			D3DRECT rect;
			rect.x1=x0;
			rect.y1=y0;
			rect.x2=x0+wid-1;
			rect.y2=y0+hei-1;

			ysD3dDev->d3dDev->Clear(1,&rect,D3DCLEAR_STENCIL,D3DCOLOR_XRGB(0,0,0),1.0F,0);
		}
	}
}

void FsSetPointLight(const YsVec3 &cameraPosition,const YsVec3 &lightPosition,FSENVIRONMENT env)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		D3DLIGHT9 light;
		ZeroMemory(&light,sizeof(light));
		light.Type=D3DLIGHT_POINT;

		switch(env)
		{
		case FSDAYLIGHT:
			light.Diffuse.r=0.6F;
			light.Diffuse.g=0.6F;
			light.Diffuse.b=0.6F;
			light.Diffuse.a=1.0;

			light.Ambient.r=0.3F;
			light.Ambient.g=0.3F;
			light.Ambient.b=0.3F;
			light.Ambient.a=1.0F;

			light.Specular.r=0.9F;
			light.Specular.g=0.9F;
			light.Specular.b=0.9F;
			light.Specular.a=1.0F;
			break;
		case FSNIGHT:
			light.Diffuse.r=0.025F;
			light.Diffuse.g=0.025F;
			light.Diffuse.b=0.025F;
			light.Diffuse.a=1.0;

			light.Ambient.r=0.025F;
			light.Ambient.g=0.025F;
			light.Ambient.b=0.025F;
			light.Ambient.a=1.0F;

			light.Specular.r=0.0F;
			light.Specular.g=0.0F;
			light.Specular.b=0.0F;
			light.Specular.a=1.0F;
			break;
		}

		light.Position.x=(FLOAT)lightPosition.x();
		light.Position.y=(FLOAT)lightPosition.y();
		light.Position.z=(FLOAT)lightPosition.z();

		light.Range=500.0F;

		light.Attenuation0=1.0;
		light.Attenuation1=0.0;
		light.Attenuation2=0.0;

		ysD3dDev->d3dDev->SetLight(0,&light);
		ysD3dDev->d3dDev->LightEnable(0,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);
	}
}

void FsSetDirectionalLight(const YsVec3 &cameraPosition,const YsVec3 &lightDirection,FSENVIRONMENT env)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		D3DVECTOR lightDir;
		D3DLIGHT9 light;
		ZeroMemory(&light,sizeof(light));
		light.Type=D3DLIGHT_DIRECTIONAL;

		switch(env)
		{
		case FSDAYLIGHT:
			light.Diffuse.r=0.6F;
			light.Diffuse.g=0.6F;
			light.Diffuse.b=0.6F;
			light.Diffuse.a=1.0;

			light.Ambient.r=0.3F;
			light.Ambient.g=0.3F;
			light.Ambient.b=0.3F;
			light.Ambient.a=1.0F;

			light.Specular.r=0.9F;
			light.Specular.g=0.9F;
			light.Specular.b=0.9F;
			light.Specular.a=1.0F;
			break;
		case FSNIGHT:
			light.Diffuse.r=0.05F;
			light.Diffuse.g=0.05F;
			light.Diffuse.b=0.05F;
			light.Diffuse.a=1.0;

			light.Ambient.r=0.05F;
			light.Ambient.g=0.05F;
			light.Ambient.b=0.05F;
			light.Ambient.a=1.0F;

			light.Specular.r=0.0F;
			light.Specular.g=0.0F;
			light.Specular.b=0.0F;
			light.Specular.a=1.0F;
			break;
		}

		lightDir.x=(float)-lightDirection.x();
		lightDir.y=(float)-lightDirection.y();
		lightDir.z=(float)-lightDirection.z();
		YsD3dNormalize(light.Direction,lightDir);
		light.Range=100.0F;
		ysD3dDev->d3dDev->SetLight(0,&light);
		ysD3dDev->d3dDev->LightEnable(0,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);
	}
}

void FsFogOn(const YsColor &col,const double &visibility)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		float density;
		density=1.0F/(FLOAT)visibility;

		ysD3dDev->d3dDev->SetRenderState(D3DRS_FOGENABLE,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_FOGCOLOR,D3DCOLOR_ARGB(0,col.Ri(),col.Gi(),col.Bi())); // 178,178,178));
		ysD3dDev->d3dDev->SetRenderState(D3DRS_FOGVERTEXMODE,D3DFOG_EXP2);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_FOGDENSITY,*(DWORD *)(&density));
	}
}

void FsFogOff(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_FOGENABLE,FALSE);
	}
}

static void FsSetupViewport(void)
{
	int x0,y0,wid,hei;
	FsGetWindowViewport(x0,y0,wid,hei);

	D3DVIEWPORT9 viewport;
	viewport.X=x0;
	viewport.Y=y0;
	viewport.Width=wid;
	viewport.Height=hei;
	viewport.MinZ=0.0f;
	viewport.MaxZ=1.0f;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetViewport(&viewport);
}

void FsBeginRenderShadowMap(const YsMatrix4x4 &projTfm,const YsMatrix4x4 &viewTfm,int texWid,int texHei)
{
}

void FsEndRenderShadowMap(void)
{
}

void FsEnableShadowMap(
    const YsMatrix4x4 &,const YsMatrix4x4 &,const YsMatrix4x4 &,
    int ,int )
{
}
void FsDisableShadowMap(int,int)
{
}

void FsSetSceneProjection(const class FsProjection &prj)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		int x0,y0,wid,hei;
		FsGetWindowViewport(x0,y0,wid,hei);

		FsSetupViewport();

		double lft,rit,top,btm;
		D3DMATRIX projMat;

		lft=(double)(   -prj.cx)*prj.nearz/prj.prjPlnDist;
		rit=(double)(wid-prj.cx)*prj.nearz/prj.prjPlnDist;
		top=(double)(    prj.cy)*prj.nearz/prj.prjPlnDist;
		btm=(double)(prj.cy-hei)*prj.nearz/prj.prjPlnDist;

		YsD3dMakePerspectiveOffCenterLH
		   (projMat,(float)lft,(float)rit,(float)btm,(float)top,(float)prj.nearz,(float)prj.farz);
		ysD3dDev->d3dDev->SetTransform(D3DTS_PROJECTION,&projMat);
	}
}

void FsSet2DDrawing(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

		FsSetupViewport();

		D3DMATRIX identity;
		YsD3dMakeIdentity(identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_PROJECTION,&identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_VIEW,&identity);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);  // <- This will 

		D3DMATRIX projMat2d;
		D3DVIEWPORT9 viewport;
		ysD3dDev->d3dDev->GetViewport(&viewport);
		YsD3dMakeOrthoOffCenterLH
		   (projMat2d,
		    (FLOAT)viewport.X                  ,(FLOAT)viewport.X+viewport.Width-1,
		    (FLOAT)viewport.Y+viewport.Height-1,(FLOAT)viewport.Y,
		    0.0F,1.0F);
		ysD3dDev->d3dDev->SetTransform(D3DTS_PROJECTION,&projMat2d);
	}
}

void FsBeginDrawShadow(void)  // Set polygon offset -1,-1 and enable.
{
	float depthBias=-0.0001f;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,*(DWORD *)(&depthBias));
	ysD3dDev->d3dDev->SetRenderState(D3DRS_DEPTHBIAS,*(DWORD *)(&depthBias));
}

void FsEndDrawShadow(void)    // Disable polygon offset.
{
	float depthBias=0.0f;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS,*(DWORD *)(&depthBias));
	ysD3dDev->d3dDev->SetRenderState(D3DRS_DEPTHBIAS,*(DWORD *)(&depthBias));
}

void FsSetCameraPosition(const YsVec3 &pos,const YsAtt3 &att,YSBOOL zClear)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);

		ysD3dDev->d3dDev->LightEnable(0,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

		if(zClear==YSTRUE)
		{
			ysD3dDev->d3dDev->Clear(0,NULL,D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0),1.0F,0);
		}

		D3DMATRIX identity;
		YsD3dMakeIdentity(identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_VIEW,&identity);
		ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&identity);
		ysD3dDev->SetCameraMatrix(pos.x(),pos.y(),pos.z(),att.h(),att.p(),att.b(),0.0);
	}
}

void FsFlushScene(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);
		if(fsD3dDev->line2d!=NULL)
		{
			fsD3dDev->line2d->Flush(D3DPT_LINELIST);
		}
	}
}

void FsDrawString(int x,int y,const char str[],const YsColor &col)
{
	fsDirectFixedFontRenderer.RenderAsciiString(x,y,str,col);
}

void FsDrawLine(int x1,int y1,int x2,int y2,const YsColor &col)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		auto fsD3dDev=dynamic_cast<FsD3dDevice *>(ysD3dDev);
		if(fsD3dDev->line2d!=NULL)
		{
			fsD3dDev->line2d->Add(D3DPT_LINELIST,(double)x1,(double)y1,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			fsD3dDev->line2d->Add(D3DPT_LINELIST,(double)x2,(double)y2,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		}
	}
}

void FsDrawRect(int x1,int y1,int x2,int y2,const YsColor &col,YSBOOL fill)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		if(fill==YSTRUE)
		{
			int wid,hei;
			FsGetWindowSize(wid,hei);

			IDirect3DSurface9 *pSurface;
			RECT rc;
			rc.left=YsSmaller(x1,x2);
			rc.right=YsGreater(x1,x2);
			rc.top=YsSmaller(y1,y2);
			rc.bottom=YsGreater(y1,y2);

			rc.left=YsGreater <int> (rc.left,0);
			rc.top=YsGreater <int> (rc.top,0);
			rc.right=YsSmaller <int> (rc.right,wid);
			rc.bottom=YsSmaller <int> (rc.bottom,hei);

			if(ysD3dDev->d3dDev->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pSurface)==D3D_OK)
			{
				ysD3dDev->d3dDev->ColorFill(pSurface,&rc,D3DCOLOR_XRGB(col.Ri(),col.Gi(),col.Bi()));
				pSurface->Release();
			}

			// Alternative
			// ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,(double)x1,(double)y1,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			// ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,(double)x2,(double)y1,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			// ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,(double)x2,(double)y2,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			// ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,(double)x1,(double)y2,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			// ysD3dDev->FlushXyzCol(D3DPT_TRIANGLEFAN);
		}
		else
		{
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)x1,(double)y1,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)x2,(double)y1,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)x2,(double)y2,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)x1,(double)y2,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)x1,(double)y1,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
		}
	}
}

void FsDrawCircle(int x,int y,int rad,const YsColor &col,YSBOOL fill)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		int i;
		static YsArray <YsVec2> circle;

		if(circle.GetN()<65)
		{
			circle.Set(65,NULL);
			for(i=0; i<=64; i++)
			{
				double a;
				a=YsPi*((double)i)/32.0;
				circle[i].Set(cos(a),sin(a));
			}
		}


		if(fill==YSTRUE)
		{
			for(i=0; i<64; i++)
			{
				float xx,yy;
				xx=(float)circle[i].x()*(float)rad+(float)x;
				yy=(float)circle[i].y()*(float)rad+(float)y;
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,(double)xx,(double)yy,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			}
			ysD3dDev->FlushXyzCol(D3DPT_TRIANGLEFAN);
		}
		else
		{
			for(i=0; i<65; i++)
			{
				float xx,yy;
				xx=(float)circle[i].x()*(float)rad+(float)x;
				yy=(float)circle[i].y()*(float)rad+(float)y;
				ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,(double)xx,(double)yy,0.5,col.Ri(),col.Gi(),col.Bi(),255);
			}
			ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
		}
	}
}

void FsDrawPolygon(int n,int plg[],const YsColor &col)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		int i;
		for(i=0; i<n; i++)
		{
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,(double)plg[i*2],(double)plg[i*2+1],0.5,col.Ri(),col.Gi(),col.Bi(),255);
		}
		ysD3dDev->FlushXyzCol(D3DPT_TRIANGLEFAN);
	}
}

void FsDrawDiamond(int x,int y,int r,const YsColor &col,YSBOOL fill)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)(x+r),(double)(y  ),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)(x  ),(double)(y+r),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)(x-r),(double)(y  ),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)(x  ),(double)(y-r),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,(double)(x+r),(double)(y  ),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
	}
}

void FsDrawX(int x,int y,int r,const YsColor &col)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,(double)(x-r  ),(double)(y-r  ),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,(double)(x+r+1),(double)(y+r+1),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,(double)(x+r  ),(double)(y-r  ),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,(double)(x-r-1),(double)(y+r+1),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
	}
}

void FsDrawPoint(int x,int y,const YsColor &col)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,(double)x,(double)y,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);
	}
}

void FsDrawPoint2Pix(int x,int y,const YsColor &col)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,(double) x   ,(double) y   ,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,(double)(x+1),(double)(y  ),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,(double)(x  ),(double)(y+1),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,(double)(x+1),(double)(y+1),0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);
	}
}

void FsDrawTitleBmp(const YsBitmap &bmp,YSBOOL tile)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		IDirect3DTexture9 *tex,*texCopy;

		// Why two textures are required?
		//   First texture D3DPOOL_SYSTEMMEM can be directly modified by the CPU, therefore it is used
		//   for setting up the bitmap.  However, it cannot be used for actually drawing.
		//   The second texture is copied from the first texture, but it is located in D3DPOOL_DEFAULT,
		//   and it can be used for actual drawing.


		// ID3DXSprite *sprite;
		const UINT mipsLev=0;  // Don't use D3DX_DEFAULT.  It cannot be given to d3dDev->CreateTexture
		if(D3D_OK==ysD3dDev->d3dDev->CreateTexture
		   (bmp.GetWidth(),bmp.GetHeight(),mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&tex,nullptr))
		{
			if(D3D_OK!=ysD3dDev->d3dDev->CreateTexture
			   (bmp.GetWidth(),bmp.GetHeight(),mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&texCopy,nullptr))
			{
				tex->Release();
				return;
			}



			D3DLOCKED_RECT locked;
			RECT rect;
			const int level=0;

			rect.left=0;
			rect.top=0;
			rect.right=bmp.GetWidth();
			rect.bottom=bmp.GetHeight();
			if(tex->LockRect(level,&locked,&rect,D3DLOCK_DISCARD)==D3D_OK)
			{
				int x,y;
				unsigned char *buf,*dstTop;
				const unsigned char *srcTop;
				buf=(unsigned char *)locked.pBits;

				for(y=0; y<bmp.GetHeight(); y++)
				{
					dstTop=buf+y*locked.Pitch;
					srcTop=bmp.GetRGBAPixelPointer(0,bmp.GetHeight()-1-y);

					// memcpy(dstTop,srcTop,bmp.GetWidth()*4); ARGB sucks.  Should be RGBA.
					for(x=0; x<bmp.GetWidth(); x++)
					{
						dstTop[x*4+3]=srcTop[x*4+3];
						dstTop[x*4+2]=srcTop[x*4+0];
						dstTop[x*4+1]=srcTop[x*4+1];
						dstTop[x*4+0]=srcTop[x*4+2];
					}
				}

				tex->UnlockRect(level);
				ysD3dDev->d3dDev->UpdateTexture(tex,texCopy);
				tex->Release();


				// if(D3DXCreateSprite(ysD3dDev->d3dDev,&sprite)==D3D_OK)
				{
					int wid,hei;
					FsGetWindowSize(wid,hei);

					// ysD3dDev->d3dSprite->Begin(D3DXSPRITE_ALPHABLEND);

					if(tile==YSTRUE)
					{
						for(x=0; x<wid; x+=bmp.GetWidth())
						{
							for(y=0; y<hei; y+=bmp.GetHeight())
							{
								ysD3dDev->d3dDev->SetTexture(0,texCopy); // Can it just be tex?
								ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_NONE);
								ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_NONE);

								auto sx=x;
								auto sy=y;
								auto w=bmp.GetWidth();
								auto h=bmp.GetHeight();
								ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy,  0.5,255,255,255,255,0.0,0.0);
								ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy,  0.5,255,255,255,255,1.0,0.0);
								ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy+h,0.5,255,255,255,255,1.0,1.0);
								ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy+h,0.5,255,255,255,255,0.0,1.0);
								ysD3dDev->FlushXyzColTex(D3DPT_TRIANGLEFAN);

								ysD3dDev->d3dDev->SetTexture(0,NULL);
								ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
								ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);

								// D3DXVECTOR3 pos;
								// pos.x=x;
								// pos.y=y;
								// pos.z=0;
								// ysD3dDev->d3dSprite->Draw(texCopy,&rect,NULL,&pos,0xffffffff);
							}
						}
					}
					else
					{
						ysD3dDev->d3dDev->SetTexture(0,texCopy); // Can it just be tex?
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_NONE);
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_NONE);

						auto sx=0;
						auto sy=0;
						auto w=bmp.GetWidth();
						auto h=bmp.GetHeight();
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy,  0.5,255,255,255,255,0.0,0.0);
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy,  0.5,255,255,255,255,1.0,0.0);
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy+h,0.5,255,255,255,255,1.0,1.0);
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy+h,0.5,255,255,255,255,0.0,1.0);
						ysD3dDev->FlushXyzColTex(D3DPT_TRIANGLEFAN);

						ysD3dDev->d3dDev->SetTexture(0,NULL);
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);


						// ysD3dDev->d3dSprite->Draw(texCopy,&rect,NULL,NULL,0xffffffff);
					}

					// ysD3dDev->d3dSprite->End();
					// sprite->Release();
				}
			}
			texCopy->Release();
		}
	}
}

void FsDrawBmp(const YsBitmap &bmp,int sx,int sy)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		IDirect3DTexture9 *tex,*texCopy;

		const UINT mipsLev=0; // Don't use D3DX_DEFAULT.
		if(D3D_OK==ysD3dDev->d3dDev->CreateTexture
		   (bmp.GetWidth(),bmp.GetHeight(),mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&tex,nullptr))
		{
			if(D3D_OK!=ysD3dDev->d3dDev->CreateTexture
			   (bmp.GetWidth(),bmp.GetHeight(),mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&texCopy,nullptr))
			{
				tex->Release();
				return;
			}



			D3DLOCKED_RECT locked;
			RECT rect;
			const int level=0;

			rect.left=0;
			rect.top=0;
			rect.right=bmp.GetWidth();
			rect.bottom=bmp.GetHeight();
			if(tex->LockRect(level,&locked,&rect,D3DLOCK_DISCARD)==D3D_OK)
			{
				int x,y;
				unsigned char *buf,*dstTop;
				const unsigned char *srcTop;
				buf=(unsigned char *)locked.pBits;

				for(y=0; y<bmp.GetHeight(); y++)
				{
					dstTop=buf+y*locked.Pitch;
					srcTop=bmp.GetRGBAPixelPointer(0,bmp.GetHeight()-1-y);

					// memcpy(dstTop,srcTop,bmp.GetWidth()*4); ARGB sucks.  Should be RGBA.
					for(x=0; x<bmp.GetWidth(); x++)
					{
						dstTop[x*4+3]=srcTop[x*4+3];
						dstTop[x*4+2]=srcTop[x*4+0];
						dstTop[x*4+1]=srcTop[x*4+1];
						dstTop[x*4+0]=srcTop[x*4+2];
					}
				}

				tex->UnlockRect(level);
				ysD3dDev->d3dDev->UpdateTexture(tex,texCopy);

				{
					ysD3dDev->d3dDev->SetTexture(0,texCopy); // Can it just be tex?
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_NONE);
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_NONE);

					auto w=bmp.GetWidth();
					auto h=bmp.GetHeight();
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy,  0.5,255,255,255,255,0.0,0.0);
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy,  0.5,255,255,255,255,1.0,0.0);
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy+h,0.5,255,255,255,255,1.0,1.0);
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy+h,0.5,255,255,255,255,0.0,1.0);
					ysD3dDev->FlushXyzColTex(D3DPT_TRIANGLEFAN);

					ysD3dDev->d3dDev->SetTexture(0,NULL);
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);



					// int wid,hei;
					// FsGetWindowSize(wid,hei);
					// ysD3dDev->d3dSprite->Begin(D3DXSPRITE_ALPHABLEND);
					// D3DXVECTOR3 pos;
					// pos.x=sx;
					// pos.y=sy;
					// pos.z=0;
					// ysD3dDev->d3dSprite->Draw(texCopy,&rect,NULL,&pos,0xffffffff);
					// ysD3dDev->d3dSprite->End();
				}
			}
			tex->Release();
			texCopy->Release();
		}
	}
}

void FsDrawLine3d(const YsVec3 &p1,const YsVec3 &p2,const YsColor &col)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(ysD3dDev!=NULL)
	{
		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,p1.xf(),p1.yf(),p1.zf(),col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,p2.xf(),p2.yf(),p2.zf(),col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
	}
}

void FsGraphicsTest(int i)
{
}

