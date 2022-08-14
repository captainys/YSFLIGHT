#ifndef FSGROUNDSKY_IS_INCLUDED
#define FSGROUNDSKY_IS_INCLUDED
/* { */

const int FsGndSkyDivX=8;
const int FsGndSkyDivY=10;

// Declaration /////////////////////////////////////////////
class FsGroundSky
{
private:
	YsVec3 FsGndSkyHalfSphere[FsGndSkyDivY][FsGndSkyDivX];

	int nLayer;
	double nDeg;

	class FsGroundSkyGraphicCache *res;

public:
	FsGroundSky();
	~FsGroundSky();

	void DrawGradation
	    (const YsVec3 &pos,const YsAtt3 &att,const YsColor &gnd,const YsColor &sky,const YsColor &horizon,
	     const double &farZ,YSBOOL specular);
	void DrawByFog
	    (const YsVec3 &pos,const YsAtt3 &att,const YsColor &gnd,const YsColor &sky,const YsColor &horizon,
	     const double &farZ,YSBOOL specular);
	void DrawCrappy
	    (const YsVec3 &pos,const YsColor &ignd,const YsColor &isky,
	     const double &farZ,YSBOOL specular);

	void DrawGroundMesh(const YsVec3 &pos,const YsAtt3 &att,const YsColor &ignd,int div,YSBOOL specular);
};

/* } */
#endif
