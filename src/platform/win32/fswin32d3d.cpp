#include <ysclass.h>
#include <ysbitmap.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <fs.h>
#include <fswindow.h>

#include <fsfontrenderer.h>

#include <fsgui.h>

#include <fsopengl.h>
#include <ysglfontdata.h>

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include <fsd3d.h>

#include <windowsx.h>



// This file includes functions called from fswin32.cpp



static void FsWin32InitDirect3d(YsD3dDevice *dev);


FsD3dDevice d3dDevMain; // ,d3dDevSub[FsMaxNumSubWindow];
D3DCAPS9 ysD3dDevCaps;


FsD3dDevice::FsD3dDevice()
{
	gunBuffer=NULL;
	instNeedleBuffer=NULL;
	debrisBuffer=NULL;

	gndCol=YsBlack();
	gndPlgByFog=NULL;
	skyCol=YsBlack();
	skyPlgByFog=NULL;
	gndSkyGradation=NULL;

	line2d=NULL;

	subWnd[0]=NULL;
	subWnd[1]=NULL;
}

FsD3dDevice::~FsD3dDevice()
{
	ReleaseBuffer();
}

void FsD3dDevice::PrepareBuffer(void)
{
	line2d=new YsXyzColBuffer;
	line2d->PrepareBuffer(*this,512);

	YsD3dDevice::PrepareBuffer();

	int i;
	for(i=0; i<4; i++)
	{
		smkFront[i].PrepareBuffer(*this,396);  // Prepare for 198 steps maximum.
		smkBack[i].PrepareBuffer(*this,396);
	}
}

#define FsD3dDeviceReleaseBuffer(buf) \
	if((buf)!=NULL) \
	{ \
		(buf)->ReleaseBuffer(); \
		delete (buf); \
		(buf)=NULL; \
	}

void FsD3dDevice::ReleaseBuffer(void)
{
	FsD3dDeviceReleaseBuffer(gunBuffer);
	FsD3dDeviceReleaseBuffer(instNeedleBuffer);
	FsD3dDeviceReleaseBuffer(debrisBuffer);

	FsD3dDeviceReleaseBuffer(gndPlgByFog);
	FsD3dDeviceReleaseBuffer(skyPlgByFog);
	FsD3dDeviceReleaseBuffer(gndSkyGradation);

	FsD3dDeviceReleaseBuffer(line2d);

	if(subWnd[0]!=NULL)
	{
		subWnd[0]->Release();
		subWnd[0]=NULL;
	}

	if(subWnd[1]!=NULL)
	{
		subWnd[1]->Release();
		subWnd[1]=NULL;
	}


	int i;
	for(i=0; i<4; i++)
	{
		smkFront[i].ReleaseBuffer();
		smkBack[i].ReleaseBuffer();
	}

	YsD3dDevice::ReleaseBuffer();
}


static LPDIRECT3D9 ysD3d=NULL;

bool FsWin32CreateGraphicContext(HWND,HDC)
{
	return true;  // This will prevent FsSimpleWindow framework from creating OpenGL context.
}

void FsWin32InitializeGraphicEngine(HWND hWnd,HDC hDc)
{
	if(ysD3d==NULL)
	{
		ysD3d=Direct3DCreate9(D3D_SDK_VERSION);
		if(ysD3d==NULL)
		{
			printf("Cannot start Direct3D.\n");
			exit(1);
		}
	}

	YsCoordSysModel=YSLEFT_ZPLUS_YPLUS;

	d3dDevMain.MakeCurrent();  // MakeCurrent then Start.  Why?  PrepareResource, PrepareBuffer will call back.
	d3dDevMain.Start(ysD3d,hWnd);

	FsWin32InitDirect3d(&d3dDevMain);

	d3dDevMain.d3dDev->GetDeviceCaps(&ysD3dDevCaps);
	printf("DevCaps::Max Point Size=%f\n",ysD3dDevCaps.MaxPointSize);

	d3dDevMain.d3dDev->Clear(0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(255,255,255),1.0F,0);
	FsSwapBuffers();

	fsDirectFixedFontRenderer.RemakeFont();
}

void FsWin32UninitializeGraphicEngine(HWND hWnd,HDC hDc)
{
	d3dDevMain.Terminate();
}

void FsWin32AfterResize(HWND hWnd,HDC hDc,int winX,int winY)
{
	if(hWnd==d3dDevMain.hWnd)
	{
		d3dDevMain.Reset(hWnd);
		fsDirectFixedFontRenderer.RemakeFont();
	}

//	else
//	{
//		int i;
//		for(i=0; i<FsMaxNumSubWindow; i++)
//		{
//			if(hWnd==d3dDevSub[i].hWnd)
//			{
//				d3dDevSub[i].Reset(hWnd);
//			}
//		}
//	}
}

bool FsWin32SwapBuffers(HWND hWnd,HDC hDc)
{
	if(hWnd!=NULL)
	{
		if(d3dDevMain.line2d!=NULL)
		{
			d3dDevMain.line2d->Flush(D3DPT_LINELIST);
		}
		d3dDevMain.d3dDev->EndScene();

		if(hWnd==d3dDevMain.hWnd)
		{
			d3dDevMain.d3dDev->Present(NULL,NULL,NULL,NULL);

			// The following attempt did not solve unsmooth frame ratio.
			// IDirect3DSwapChain9 *backBuff;
			// if(d3dDevMain.d3dDev->GetSwapChain(0,&backBuff)==D3D_OK)
			// {
			// 	backBuff->Present(NULL,NULL,hWnd,NULL,D3DPRESENT_INTERVAL_IMMEDIATE);
			// 	backBuff->Release();
			// }
		}
		else
		{
			int i;
			for(i=0; i<FsMaxNumSubWindow; i++)
			{
				if(hWnd==d3dDevMain.hWndSub[i])
				{
					d3dDevMain.subWnd[i]->Present(NULL,NULL,hWnd,NULL,D3DPRESENT_INTERVAL_IMMEDIATE);
				}
			}
		}
	}

	return true; // Returning true will prevent FsSimpleWindow framework from calling wglSwapBuffers.
}



////////////////////////////////////////////////////////////

static void FsWin32InitDirect3d(YsD3dDevice *d3dDev)
{
	if(d3dDev->d3dDev->BeginScene()==D3D_OK)
	{
		D3DMATRIX identity;
		YsD3dMakeIdentity(identity);
		d3dDev->d3dDev->SetTransform(D3DTS_VIEW,&identity);
		d3dDev->d3dDev->SetTransform(D3DTS_PROJECTION,&identity);
		d3dDev->d3dDev->SetTransform(D3DTS_WORLD,&identity);

		D3DVECTOR lightDir;
		D3DLIGHT9 light;
		ZeroMemory(&light,sizeof(light));
		light.Type=D3DLIGHT_DIRECTIONAL;
		light.Diffuse.r=0.8F;
		light.Diffuse.g=0.8F;
		light.Diffuse.b=0.8F;
		light.Diffuse.a=1.0F;

		light.Specular.r=0.9F;
		light.Specular.g=0.9F;
		light.Specular.b=0.9F;
		light.Specular.a=1.0F;

		light.Ambient.r=0.4F;
		light.Ambient.g=0.4F;
		light.Ambient.b=0.4F;
		light.Ambient.a=1.0F;

		lightDir.x=1.0;
		lightDir.y=0.0;
		lightDir.z=1.0;
		YsD3dNormalize(light.Direction,lightDir);
		light.Range=100.0F;
		d3dDev->d3dDev->SetLight(0,&light);
		d3dDev->d3dDev->LightEnable(0,TRUE);
		d3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);

		d3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
		d3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		d3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
		d3dDev->d3dDev->SetRenderState(D3DRS_NORMALIZENORMALS,TRUE);

		d3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		d3dDev->d3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		d3dDev->d3dDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

		d3dDev->d3dDev->EndScene();
	}
}

void FsWin32HidePartOfScreenForSharewareMessage(void)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);


	// Preparing 2D drawing
	D3DMATRIX identity;
	YsD3dMakeIdentity(identity);
	ysD3dDev->d3dDev->SetTransform(D3DTS_PROJECTION,&identity);
	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&identity);
	ysD3dDev->d3dDev->SetTransform(D3DTS_VIEW,&identity);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	D3DMATRIX projMat2d;
	D3DVIEWPORT9 viewport;
	ysD3dDev->d3dDev->GetViewport(&viewport);
	YsD3dMakeOrthoOffCenterLH
	   (projMat2d,
	    (FLOAT)viewport.X                  ,(FLOAT)viewport.X+viewport.Width-1,
	    (FLOAT)viewport.Y+viewport.Height-1,(FLOAT)viewport.Y,
	    0.0F,1.0F);
	ysD3dDev->d3dDev->SetTransform(D3DTS_PROJECTION,&projMat2d);


	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,0.0           ,0.0              ,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,viewport.Width,0.0              ,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,viewport.Width,viewport.Height/3,0.5,0,0,0,255);

	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,viewport.Width,viewport.Height/3,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,0.0           ,0.0              ,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,0.0           ,viewport.Height/3,0.5,0,0,0,255);

	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,0.0           ,viewport.Height*2/3,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,viewport.Width,viewport.Height*2/3,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,viewport.Width,viewport.Height    ,0.5,0,0,0,255);

	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,0.0           ,viewport.Height*2/3,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,viewport.Width,viewport.Height    ,0.5,0,0,0,255);
	ysD3dDev->AddXyzCol(D3DPT_TRIANGLELIST,0.0           ,viewport.Height    ,0.5,0,0,0,255);

	ysD3dDev->FlushXyzCol(D3DPT_TRIANGLELIST);
}
