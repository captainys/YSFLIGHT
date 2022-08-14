#ifndef FSCLOUD_IS_INCLUDED
#define FSCLOUD_IS_INCLUDED
/* { */

#include <ysglbuffermanager.h>

class FsCloud
{
private:
	const FsCloud &operator=(const FsCloud &from);

public:
	FsCloud();
	~FsCloud();

	void Generate(int n,double dx,double dy,const YsVec3 &cen);
	void Draw(void);
	YSRESULT Save(FILE *fp) const;
	YSRESULT Load(FILE *fp);

	YsArray <YsVec3> vtx;
	YsVec3 cen;
	YsColor col;
};


class FsClouds
{
private:
	const FsClouds &operator=(const FsClouds &from);
	class FsCloudsGraphicCache *res;

	void CreateGraphicCache(void);
	void DeleteGraphicCache(void);

public:
	FsClouds();
	~FsClouds();
	void Scatter
	    (int n,const YsVec3 &center,double range,double averageSize,double ceiling);
//	void Scroll
//	    (const YsVec3 &neoCenter,double range,double averageSize,double ceiling);
	void Draw(void);
	void MakeOpenGlList(void);

	YSRESULT Save(FILE *fp) const;
	YSRESULT Load(FILE *fp);

	YSBOOL IsReady(void) const;

	YSBOOL ready;
	YSBOOL needRemakeVbo;
	int nCld;
	FsCloud *cld;

	YsGLBufferManager::Handle vboHd;
};


class FsSolidCloud
{
friend class FsSolidClouds;
friend class FsSolidCloudGraphicCache;

public:
	class CloudParticle
	{
	public:
		YsVec3 pos;
		double rad;
		int particleType; // 0 to 7.  Corresponds to s=particleType*0.125
		double colorCorrection; // Tweak brightness for this particle.
	};

private:
	const FsSolidCloud &operator=(const FsSolidCloud &from);
	// Copy operator must be prohibited for using YsD3dExternalVertexBufferLink

	class FsSolidCloudGraphicCache *res;

	void CreateGraphicCache(void);
	void DeleteGraphicCache(void);

	YsArray <CloudParticle> particle;

	YsShell shl;
	YsShellLattice ltc;
	YsVec3 bbx[2],cen;
	YsGLBufferManager::Handle vboHd;

public:
	FsSolidCloud();
	~FsSolidCloud();
	void Initialize(void);

	const YsShell &GetShell(void) const;
	const YsVec3 &GetCenter(void) const;

	// Cloud rendering is view-direction dependent.
	void Draw(FSENVIRONMENT env,const class FsWeather &weather);

	void MakeOpenGlList(void);

	void Make(const YsVec3 &cen,const double &sizeX,const double &sizeZ,const double &y0,const double &y1);
	YSBOOL IsInCloud(const YsVec3 &pos) const;

	void ScatterParticle(int nParticle);
};

class FsSolidClouds
{
public:
	YsListAllocator <FsSolidCloud> cloudAllocator;
	YsListContainer <FsSolidCloud> cloudContainer;

	FsSolidClouds();
	~FsSolidClouds();

	void Test(void);

	YSBOOL IsReady(void) const;
	YSBOOL IsInCloud(const YsVec3 &pos) const;

	YSRESULT Save(FILE *fp) const;
	YSRESULT Load(FILE *fp);

	void AddToParticleManager(
		class YsGLParticleManager &partMan,
	    FSENVIRONMENT env,const class FsWeather &weather,
	    const YsVec3 &viewDir,const YsMatrix4x4 &viewMdlTfm,const double &nearZ,const double &farZ,const double &tanFov);


	void MakeOpenGlList(void);

	void SetUpCloudPerFrame(void);
	void BeginDrawCloud(void);
	void Draw(
	    FSENVIRONMENT env,const class FsWeather &weather,
	    const YsMatrix4x4 &viewMdlTfm,const double &nearZ,const double &farZ,const double &tanFov);
	void EndDrawCloud(void);
	void ReduceVisibilityByPolygon(const YsMatrix4x4 &viewTfm,const YsColor &col,YSBOOL transparency);
	void Make(int n,const YsVec3 &cen,const double &range,const double &sizeX,const double &y0,const double &y1);
};


/* } */
#endif
