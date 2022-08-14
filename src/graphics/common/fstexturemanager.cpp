#include <ysport.h>
#include <fsdef.h>
#include "fstexturemanager.h"

YsTextureManager::TexHandle fsGroundTileTexHd=nullptr;
YsTextureManager::TexHandle fsRunwayLightTexHd=nullptr;
void FsLoadRunwayLightTexture(void);


static FsCommonTexture commonTex;


/* static */ FsCommonTexture &FsCommonTexture::GetCommonTexture(void)
{
	return commonTex;
}

FsCommonTexture::FsCommonTexture()
{
	GroundTileTexHd=nullptr;
	RunwayLightTexHd=nullptr;

	for(auto &hd : ShadowMapTexHd)
	{
		hd=nullptr;
	}
}

YsTextureManager::TexHandle FsCommonTexture::UpdateTextureFromPngFile(YsTextureManager::TexHandle texHd,const char fn[])
{
	if(nullptr==texHd)
	{
		YsFileIO::File fp(fn,"rb");
		if(nullptr!=fp)
		{
			YsArray <unsigned char> dat,readBuf;
			readBuf.Resize(1024*1024);
			for(;;)
			{
				auto dataFilled=fread(readBuf,1,readBuf.GetN(),fp);
				if(0==dataFilled)
				{
					break;
				}
				dat.Add(dataFilled,readBuf);
			}
			texHd=texMan.AddTexture(fn,YsTextureManager::FOM_PNG,0,0,dat.GetN(),dat);
		}
	}
	if(nullptr!=texHd)
	{
		auto unitPtr=texMan.GetTexture(texHd);
		if(nullptr!=unitPtr && YSTRUE!=unitPtr->IsActualTextureReady())
		{
			unitPtr->MakeActualTexture();
		}
	}
	return texHd;
}

YsTextureManager &FsCommonTexture::GetTextureManager(void)
{
	return texMan;
}

void FsCommonTexture::LoadGroundTileTexture(void)
{
	GroundTileTexHd=UpdateTextureFromPngFile(GroundTileTexHd,FS_TEXTURE_GROUNDTILE);
	if(nullptr!=GroundTileTexHd)
	{
		texMan.SetTextureFilterType(GroundTileTexHd,YsTextureManager::Unit::FILTERTYPE_LINEAR);
	}
}

void FsCommonTexture::LoadRunwayLightTexture(void)
{
	RunwayLightTexHd=UpdateTextureFromPngFile(RunwayLightTexHd,FS_TEXTURE_RWLIGHT);
	if(nullptr!=RunwayLightTexHd)
	{
		texMan.SetTextureFilterType(RunwayLightTexHd,YsTextureManager::Unit::FILTERTYPE_LINEAR);
	}
}

void FsCommonTexture::LoadParticleSpriteTexture(void)
{
	ParticleSpriteTexHd=UpdateTextureFromPngFile(ParticleSpriteTexHd,FS_TEXTURE_PARTICLESPRITE);
	if(nullptr!=ParticleSpriteTexHd)
	{
		texMan.SetTextureFilterType(ParticleSpriteTexHd,YsTextureManager::Unit::FILTERTYPE_LINEAR);
		texMan.SetTextureEffectType(ParticleSpriteTexHd,YsTextureManager::Unit::EFFECT_ALPHAMASK_MAXRGB);
	}
}

YsTextureManager::TexHandle FsCommonTexture::GetGroundTileTextureHd(void) const
{
	return GroundTileTexHd;
}
YsTextureManager::TexHandle FsCommonTexture::GetRunwayLightTextureHd(void) const
{
	return RunwayLightTexHd;
}
YsTextureManager::TexHandle FsCommonTexture::GetParticleSpriteTextureHd(void) const
{
	return ParticleSpriteTexHd;
}

const YsTextureManager::Unit *FsCommonTexture::GetGroundTileTexture(void) const
{
	return texMan.GetTextureReady(GroundTileTexHd);
}
const YsTextureManager::Unit *FsCommonTexture::GetRunwayLightTexture(void) const
{
	return texMan.GetTextureReady(RunwayLightTexHd);
}
const YsTextureManager::Unit *FsCommonTexture::GetParticleSpriteTexture(void) const
{
	return texMan.GetTextureReady(ParticleSpriteTexHd);
}

YSRESULT FsCommonTexture::ReadyShadowMap(void)
{
	for(YSSIZE_T idx=0; idx<MAX_NUM_SHADOWMAP; ++idx)
	{
		YsString texName="ShadowMap";
		texName.Append('0'+idx);
		if(nullptr==ShadowMapTexHd[idx])
		{
			ShadowMapTexHd[idx]=texMan.AddShadowMapTexture(texName,YsTextureManager::FOM_RAW_Z,2048,2048);
		}
		if(nullptr!=ShadowMapTexHd[idx] && YSTRUE!=texMan.IsReady(ShadowMapTexHd[idx]))
		{
			texMan.GetTexture(ShadowMapTexHd[idx])->MakeActualTexture();
		}
		if(YSTRUE!=texMan.IsReady(ShadowMapTexHd[idx]))
		{
			return YSERR;
		}
	}
	return YSOK;
}

YSSIZE_T FsCommonTexture::GetMaxNumShadowMap(void) const
{
	return MAX_NUM_SHADOWMAP;
}

const YsTextureManager::Unit *FsCommonTexture::GetShadowMapTexture(YSSIZE_T shadowMapIdx) const
{
	return texMan.GetTextureReady(ShadowMapTexHd[shadowMapIdx]);
}
