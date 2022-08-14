#ifndef FSTEXTUREMANAGER_IS_INCLUDED
#define FSTEXTUREMANAGER_IS_INCLUDED
/* { */

#include <ystexturemanager.h>

class FsCommonTexture
{
public:
	YsTextureManager texMan;

	YsTextureManager::TexHandle GroundTileTexHd;
	YsTextureManager::TexHandle RunwayLightTexHd;
	YsTextureManager::TexHandle ParticleSpriteTexHd;

private:
	enum
	{
		MAX_NUM_SHADOWMAP=3,
	};

	YsTextureManager::TexHandle ShadowMapTexHd[MAX_NUM_SHADOWMAP];

	YsTextureManager::TexHandle UpdateTextureFromPngFile(YsTextureManager::TexHandle texHd,const char fn[]);
	FsCommonTexture(const FsCommonTexture &);
	FsCommonTexture &operator=(const FsCommonTexture &);

public:
	static FsCommonTexture &GetCommonTexture(void);

	FsCommonTexture();

	YsTextureManager &GetTextureManager(void);

	void LoadGroundTileTexture(void);
	void LoadRunwayLightTexture(void);
	void LoadParticleSpriteTexture(void);

	YsTextureManager::TexHandle GetGroundTileTextureHd(void) const;
	YsTextureManager::TexHandle GetRunwayLightTextureHd(void) const;
	YsTextureManager::TexHandle GetParticleSpriteTextureHd(void) const;

	const YsTextureManager::Unit *GetGroundTileTexture(void) const;
	const YsTextureManager::Unit *GetRunwayLightTexture(void) const;
	const YsTextureManager::Unit *GetParticleSpriteTexture(void) const;

	YSRESULT ReadyShadowMap(void);
	YSSIZE_T GetMaxNumShadowMap(void) const;
	const YsTextureManager::Unit *GetShadowMapTexture(YSSIZE_T shadowTexId) const;
};

/* } */
#endif
