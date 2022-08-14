#include <ysclass.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"


#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"

#include <ysbitmap.h>

#include <time.h>

#include "fsparticle.h"
#include "fstexturemanager.h"


void FsParticleStore::Draw(const class YsGLParticleManager &partMan) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_NONE);
	ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_NONE);
	ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);

	auto &commonTexture=FsCommonTexture::GetCommonTexture();
	if(nullptr==commonTexture.GetParticleSpriteTextureHd())
	{
		commonTexture.LoadParticleSpriteTexture();
	}

	auto texUnitPtr=commonTexture.GetParticleSpriteTexture();
	if(nullptr!=texUnitPtr)
	{
		texUnitPtr->Bind(0);
	}
	for(YSSIZE_T i=0; i<partMan.triVtxBuf.GetN(); ++i)
	{
		int r=(int)(partMan.triColBuf[i][0]*255.0);
		int g=(int)(partMan.triColBuf[i][1]*255.0);
		int b=(int)(partMan.triColBuf[i][2]*255.0);
		int a=(int)(partMan.triColBuf[i][3]*255.0);
		ysD3dDev->AddXyzColTex(D3DPT_TRIANGLELIST,
		    partMan.triVtxBuf[i][0],partMan.triVtxBuf[i][1],partMan.triVtxBuf[i][2],
		    r,g,b,a,
		    partMan.triTexCoordBuf[i][0],partMan.triTexCoordBuf[i][1]);
	}
	ysD3dDev->FlushXyzColTex(D3DPT_TRIANGLELIST);

	ysD3dDev->d3dDev->SetTexture(0,NULL);

	/* float pointSize=0.3F;
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSCALEENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize));

	YsListItem <FsParticle> *ptr;
	particleList.RewindPointer();
	while(ptr=particleList.StepPointer())
	{
		const YsColor &col=ptr->dat.GetColor();
		const YsVec3 &pos=ptr->dat.GetPosition();
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,pos.x(),pos.y(),pos.z(),col.Ri(),col.Gi(),col.Bi(),255);
	}
	ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);

	pointSize=1.0F;
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSCALEENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&pointSize)); */
}
