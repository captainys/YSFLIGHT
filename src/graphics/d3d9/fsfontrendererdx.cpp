#include "graphics/common/fsfontrenderer.h"

#include <ysglfontdata.h>

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"

extern YSBOOL FsD3dMyDriverSucks; // Defined in fsoption.cpp



YSRESULT FsDirectFixedFontRenderer::RequestDefaultFontWithPixelHeight(unsigned int fontHeight)
{
	fontPtr=YsGlSelectFontBitmapPointerByHeight(&fontWid,&fontHei,fontHeight);

	auto ysD3dDev=YsD3dDevice::GetCurrent();

	if(NULL!=ysD3dDev && NULL!=fontPtr)
	{
		auto ysD3dDev=YsD3dDevice::GetCurrent();
		auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);
 
	 	int i;
	 	YsArray <unsigned char> rgbaBuf;
	 	rgbaBuf.Set(fontWid*fontHei*4,NULL);
	 	for(i=0; i<256; i++)
	 	{
	 		if(NULL!=fsD3dDev->bitmapFont[i].buf)
	 		{
	 			fsD3dDev->bitmapFont[i].buf->ReleaseBuffer();
	 		}
	 		const UINT mipsLev=0;
	 		fsD3dDev->CreateExternalTextureBuffer(
	 		   &fsD3dDev->bitmapFont[i],fontWid,fontHei,mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT);
	 		if(NULL!=fsD3dDev->bitmapFont[i].buf)
	 		{
	 			char c[2];
	 			c[0]=i;
	 			c[1]=0;
	 
	 			int j;
	 			forYsArray(j,rgbaBuf)
	 			{
	 				rgbaBuf[j]=0;
	 			}
	 
	 			YsGlWriteStringToRGBA8Bitmap(c,255,255,255,255,rgbaBuf,fontWid,fontHei,0,0,fontPtr,fontWid,fontHei);
	 
	 			IDirect3DTexture9 *tex;
	 			const UINT mipsLev=0;
	 			if(D3D_OK==ysD3dDev->d3dDev->CreateTexture
	 			   (fontWid,fontHei,mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&tex,nullptr))
	 			{
	 				D3DLOCKED_RECT locked;
	 				RECT rect;
	 				const int level=0;
	 
	 				rect.left=0;
	 				rect.top=0;
	 				rect.right=fontWid;
	 				rect.bottom=fontHei;
	 				if(tex->LockRect(level,&locked,&rect,D3DLOCK_DISCARD)==D3D_OK)
	 				{
	 					int y;
	 					unsigned char *buf,*dstTop;
	 					const unsigned char *srcTop;
	 					buf=(unsigned char *)locked.pBits;
	 
	 					for(y=0; y<fontHei; y++)
	 					{
	 						dstTop=buf+y*locked.Pitch;
	 						srcTop=rgbaBuf+(fontHei-1-y)*fontWid*4;
	 						memcpy(dstTop,srcTop,fontWid*4);
	 					}
	 
	 					tex->UnlockRect(level);
	 					ysD3dDev->d3dDev->UpdateTexture(tex,fsD3dDev->bitmapFont[i].buf->tex);
	 				}
	 				tex->Release();
	 			}
	 
	 			printf("Texture buffer %d allocated.\n",i);
	 		}
	 	}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsDirectFixedFontRenderer::RenderAsciiStringSingleLine(int leftX,int bottomY,const char str[],const YsColor &col)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(NULL!=ysD3dDev)
	{
		if(YSTRUE==FsD3dMyDriverSucks)
		{
			// DirectX forgets all pre-allocated objects, or does not let me pre-alloc after resizing the window.

			const int len=(int)strlen(str);
			const int wid=len*fontWid;
			const int hei=fontHei;

			IDirect3DTexture9 *gpuTex;
			const UINT mipsLev=0;
			if(D3D_OK==ysD3dDev->d3dDev->CreateTexture(wid,hei,mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&gpuTex,nullptr))
			{
	 			IDirect3DTexture9 *memTex;
	 			if(D3D_OK==ysD3dDev->d3dDev->CreateTexture(wid,hei,mipsLev,0,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&memTex,nullptr))
	 			{
				 	YsArray <unsigned char> rgbaBuf;
				 	rgbaBuf.Set(wid*hei*4,NULL);
				 	for(YSSIZE_T i=0; i<wid*hei*4; ++i)
				 	{
						rgbaBuf[i]=0;
					}

		 			YsGlWriteStringToRGBA8Bitmap(str,255,255,255,255,rgbaBuf,wid,hei,0,0,fontPtr,fontWid,fontHei);



	 				D3DLOCKED_RECT locked;
	 				RECT rect;
	 				const int level=0;
	 
	 				rect.left=0;
	 				rect.top=0;
	 				rect.right=wid;
	 				rect.bottom=hei;
	 				if(D3D_OK==memTex->LockRect(level,&locked,&rect,D3DLOCK_DISCARD))
	 				{
	 					int y;
	 					unsigned char *buf,*dstTop;
	 					const unsigned char *srcTop;
	 					buf=(unsigned char *)locked.pBits;
	 
	 					for(y=0; y<hei; y++)
	 					{
	 						dstTop=buf+y*locked.Pitch;
	 						srcTop=rgbaBuf+(hei-1-y)*wid*4;
	 						memcpy(dstTop,srcTop,wid*4);
	 					}
	 
	 					memTex->UnlockRect(level);
	 					ysD3dDev->d3dDev->UpdateTexture(memTex,gpuTex);


						ysD3dDev->d3dDev->SetTexture(0,gpuTex); // Can it just be tex?
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_NONE);
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_NONE);

						auto sx=leftX;
						auto sy=bottomY-fontHei+1;
						auto w=wid;
						auto h=hei;
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy,  0.5,255,255,255,255,0.0,0.0);
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy,  0.5,255,255,255,255,1.0,0.0);
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy+h,0.5,255,255,255,255,1.0,1.0);
						ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy+h,0.5,255,255,255,255,0.0,1.0);
						ysD3dDev->FlushXyzColTex(D3DPT_TRIANGLEFAN);

						ysD3dDev->d3dDev->SetTexture(0,NULL);
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
						ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);

						// d3dSprite->Begin(D3DXSPRITE_ALPHABLEND);
						// D3DXVECTOR3 pos;
						// pos.x=leftX;
						// pos.y=bottomY-fontHei+1;
						// pos.z=0;
	 					// d3dSprite->Draw(gpuTex,&rect,NULL,&pos,D3DCOLOR_ARGB(255,col.Ri(),col.Gi(),col.Bi()));
						// d3dSprite->End();
	 				}
	 				memTex->Release();
				}
 				gpuTex->Release();
			}
		}
		else
		{
			RECT rect;
			rect.left=0;
			rect.top=0;
			rect.right=fontWid;
			rect.bottom=fontHei;

			FsD3dDevice *fsD3dDev=(FsD3dDevice *)ysD3dDev;

			// ysD3dDev->d3dSprite->Begin(D3DXSPRITE_ALPHABLEND);

			int i;
			for(i=0; str[i]!=0; i++)
			{
				unsigned char c=str[i];
				if(NULL!=fsD3dDev->bitmapFont[c].buf)
				{
					ysD3dDev->d3dDev->SetTexture(0,fsD3dDev->bitmapFont[c].buf->tex);
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_NONE);
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_NONE);

					auto sx=leftX;
					auto sy=bottomY-fontHei+1;
					auto w=fontWid;
					auto h=fontHei;
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy,  0.5,255,255,255,255,0.0,0.0);
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy,  0.5,255,255,255,255,1.0,0.0);
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx+w,(double)sy+h,0.5,255,255,255,255,1.0,1.0);
					ysD3dDev->AddXyzColTex(D3DPT_TRIANGLEFAN,(double)sx,  (double)sy+h,0.5,255,255,255,255,0.0,1.0);
					ysD3dDev->FlushXyzColTex(D3DPT_TRIANGLEFAN);

					ysD3dDev->d3dDev->SetTexture(0,NULL);
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
					ysD3dDev->d3dDev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);


					// D3DXVECTOR3 pos;
					// pos.x=leftX;
					// pos.y=bottomY-fontHei+1;
					// pos.z=0;
					// ysD3dDev->d3dSprite->Draw(,&rect,NULL,&pos,D3DCOLOR_ARGB(255,col.Ri(),col.Gi(),col.Bi()));
				}

				leftX+=fontWid;
			}

			// ysD3dDev->d3dSprite->End();
		}
		return YSOK;
	}
	return YSERR;
}
