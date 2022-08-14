#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include "fs.h"

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"




void FsWeather::DrawCloudLayer(const YsVec3 &cameraPos) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);


	D3DMATERIAL9 material;
	ZeroMemory(&material,sizeof(material));

	material.Diffuse.r=0.9F;
	material.Diffuse.g=0.9F;
	material.Diffuse.b=0.9F;
	material.Diffuse.a=1.0F;
	material.Ambient.r=0.9F;
	material.Ambient.g=0.9F;
	material.Ambient.b=0.9F;
	material.Ambient.a=1.0F;
	ysD3dDev->d3dDev->SetMaterial(&material);


	int i;
	forYsArray(i,cloudLayer)
	{
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()-20000.0,cloudLayer[i].y0,cameraPos.z()-20000.0,0.0,-1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()-20000.0,cloudLayer[i].y0,cameraPos.z()+20000.0,0.0,-1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()+20000.0,cloudLayer[i].y0,cameraPos.z()+20000.0,0.0,-1.0,0.0);

		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()-20000.0,cloudLayer[i].y0,cameraPos.z()-20000.0,0.0,-1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()+20000.0,cloudLayer[i].y0,cameraPos.z()+20000.0,0.0,-1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()+20000.0,cloudLayer[i].y0,cameraPos.z()-20000.0,0.0,-1.0,0.0);

		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()-20000.0,cloudLayer[i].y1,cameraPos.z()-20000.0,0.0, 1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()-20000.0,cloudLayer[i].y1,cameraPos.z()+20000.0,0.0, 1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()+20000.0,cloudLayer[i].y1,cameraPos.z()+20000.0,0.0, 1.0,0.0);

		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()-20000.0,cloudLayer[i].y1,cameraPos.z()-20000.0,0.0, 1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()+20000.0,cloudLayer[i].y1,cameraPos.z()+20000.0,0.0, 1.0,0.0);
		ysD3dDev->AddXyzNom(D3DPT_TRIANGLELIST,cameraPos.x()+20000.0,cloudLayer[i].y1,cameraPos.z()-20000.0,0.0, 1.0,0.0);
	}

	ysD3dDev->FlushXyzNom(D3DPT_TRIANGLELIST);
}



