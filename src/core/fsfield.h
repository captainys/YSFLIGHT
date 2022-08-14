#ifndef FSFIELD_IS_INCLUDED
#define FSFIELD_IS_INCLUDED
/* { */

#include <ysscenery.h>
#include "fsdef.h"
#include "fsvisual.h"
#include "fsnetwork.h"
#include "fsairplaneproperty.h"
#include "fsgroundproperty.h"



class FsField
{
private:
	YsString fldIdName;
	YsVec3 pos;
	YsAtt3 att;
	YsScenery *fld;

public:
	FsField();
	~FsField();

	void Initialize(void);
	void CleanUp(void);
	void SetField(YsScenery *fld);
	void SetIdName(const char idName[]);
	void SetPosition(const YsVec3 &pos);
	void SetAttitude(const YsAtt3 &att);

	void GetBoundingBox(YsVec3 &min,YsVec3 &max) const;

	const YsVec3 &GetPosition(void) const;
	const YsAtt3 &GetAttitude(void) const;
	const char *GetIdName(void) const;
	const YsScenery *GetFieldPtr(void) const;

	void ApplyColorScale(const double &plgScale,const double &linScale,const double &pntScale);
	void CacheMapDrawingOrder(void) const;
	void DrawProtectPolygon
	 (const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,const double &nearZ) const;
	void DrawVisual(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,YSBOOL forShadowMap) const;
	void DrawVisual(const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,YSBOOL forShadowMap) const;
	void DrawMapVisual
	 (FSENVIRONMENT env,
	  const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,
	  const double &elvMin,const double &elvMax,YSBOOL drawPset,
	  const double &currentTime,
	  YSBOOL useMapTexture,YSBOOL useRunwayLightTexture) const;

	void GetMapElevationCache(int &nCache,const double *&cache) const;

	YSRESULT GetGroundSkyColor(YsColor &gnd,YsColor &sky) const;

	const YsSceneryItem *GetFieldElevation(double &elv,const double &x,const double &z) const;
	const YsSceneryItem *GetFieldElevationAndNormal(double &elv,YsVec3 &nom,const double &x,const double &z) const;
	double GetBaseElevation(void) const;
	YSBOOL CanResume(void) const;
	YSBOOL CanContinue(void) const;


	template <const int N>
	YSRESULT GetFieldRegion(YsArray <const class YsSceneryRectRegion *,N> &id,const double &x,const double &z) const;
	YSBOOL GetFieldShellCollision(YsVec3 &itscPos,const YsVec3 &p1,const YsVec3 &p2) const;
	YSBOOL GetFieldShellCollision(const YsShell &shl,const YsMatrix4x4 modelTfm) const;
	YSSCNAREATYPE GetAreaType(const YsVec3 &pos) const;

	YSRESULT GetFieldRegionCenterLine(YsVec3 &o,YsVec3 &v,const class YsSceneryRectRegion *rgn) const;

	template <const int N>
	YSRESULT SearchFieldRegionById(YsArray <const class YsSceneryRectRegion *,N> &lst,int id) const;
	template <const int N>
	YSRESULT SearchFieldRegionByTag(YsArray <const class YsSceneryRectRegion *,N> &lst,const char tag[]) const;
	YSRESULT GetFieldRegionRect(YsVec3 rec[4],const class YsSceneryRectRegion *rgn) const;

	template <const int N>
	YSRESULT SearchPointSetById(YsArray <const YsSceneryPointSet *,N> &pstLst,int id) const;
	template <const int N>
	YSRESULT SearchPointSetByTag(YsArray <const YsSceneryPointSet *,N> &pstLst,const char *tag) const;
	template <const int N>
	YSRESULT GetPointSet(YsArray <YsVec3,N> &point,const YsSceneryPointSet *pst) const;

	YSRESULT GetFirstPointOfPointSet(YsVec3 &point,const YsSceneryPointSet *pst) const;
};

template <const int N>
YSRESULT FsField::GetFieldRegion(YsArray <const class YsSceneryRectRegion *,N> &id,const double &x,const double &z) const
{
	if(fld!=NULL)
	{
		fld->pos=pos;
		fld->att=att;

		if(fld->GetRectRegionFromPoint(id,YsVec3(x,0.0,z))>0)
		{
			return YSOK;
		}
	}
	return YSERR;
}

template <const int N>
YSRESULT FsField::SearchFieldRegionById(YsArray <const YsSceneryRectRegion *,N> &lst,int id) const
{
	lst.Set(0,NULL);
	if(fld!=NULL)
	{
		return fld->SearchRegionById(lst,id);
	}
	return YSERR;
}

template <const int N>
YSRESULT FsField::SearchFieldRegionByTag(YsArray <const class YsSceneryRectRegion *,N> &lst,const char tag[]) const
{
	lst.Set(0,NULL);
	if(fld!=NULL)
	{
		return fld->SearchRegionByTag(lst,tag);
	}
	return YSERR;
}

template <const int N>
YSRESULT FsField::SearchPointSetById(YsArray <const YsSceneryPointSet *,N> &pstLst,int id) const
{
	if(fld!=NULL)
	{
		return fld->SearchPointSetById(pstLst,id);
	}
	else
	{
		pstLst.Set(0,NULL);
		return YSERR;
	}
}

template <const int N>
YSRESULT FsField::SearchPointSetByTag(YsArray <const YsSceneryPointSet *,N> &pstLst,const char *tag) const
{
	if(fld!=NULL)
	{
		return fld->SearchPointSetByTag(pstLst,tag);
	}
	else
	{
		pstLst.Set(0,NULL);
		return YSERR;
	}
}

template <const int N>
YSRESULT FsField::GetPointSet(YsArray <YsVec3,N> &point,const YsSceneryPointSet *pst) const
{
	if(fld!=NULL)
	{
		fld->pos=pos;
		fld->att=att;
		return fld->GetPointSet(point,pst);
	}
	else
	{
		point.Set(0,NULL);
		return YSERR;
	}
}

/* } */
#endif
