#include <ysclass.h>
#include "fs.h"

#include "fsinstpanel.h"

#include <fstexturemanager.h>



FsField::FsField()
{
	Initialize();
	fld=NULL;
}

FsField::~FsField()
{
	CleanUp();
}

void FsField::Initialize(void)
{
	pos.Set(0.0,0.0,0.0);
	att.Set(0.0,0.0,0.0);
}

void FsField::CleanUp(void)
{
}

void FsField::SetField(YsScenery *fld)
{
	this->fld=fld;
}

void FsField::SetIdName(const char idName[])
{
	this->fldIdName.Set(idName);
}

void FsField::SetPosition(const YsVec3 &pos)
{
	this->pos=pos;
}

void FsField::SetAttitude(const YsAtt3 &att)
{
	this->att=att;
}

void FsField::GetBoundingBox(YsVec3 &min,YsVec3 &max) const
{
	if(NULL!=fld)
	{
		YsVec3 bbx[2];
		fld->GetBoundingBox(bbx);
		min=bbx[0];
		max=bbx[1];
	}
	else
	{
		min=YsOrigin();
		max=YsOrigin();
	}
}

const YsVec3 &FsField::GetPosition(void) const
{
	return pos;
}

const YsAtt3 &FsField::GetAttitude(void) const
{
	return att;
}

const char *FsField::GetIdName(void) const
{
	return fldIdName;
}

const YsScenery *FsField::GetFieldPtr(void) const
{
	return fld;
}

void FsField::ApplyColorScale(const double &plgScale,const double &linScale,const double &pntScale)
{
	if(fld!=NULL)
	{
		fld->SetColorScale(plgScale,linScale,pntScale);
	}
}

void FsField::CacheMapDrawingOrder(void) const
{
	if(nullptr!=fld && YSTRUE!=fld->MapDrawingOrderCached())
	{
		fld->CacheMapDrawingOrder();
	}
}

void FsField::DrawProtectPolygon(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,const double &nearZ) const
{
	if(fld!=NULL)
	{
		YsMatrix4x4 viewTfm;
		viewTfm.RotateXY(-viewAtt.b());
		viewTfm.RotateZY(-viewAtt.p());
		viewTfm.RotateXZ(-viewAtt.h());
		viewTfm.Translate(-viewPos);

		fld->pos=pos;
		fld->att=att;
		fld->DrawProtectPolygon(viewTfm,YsIdentity4x4(),projMat,nearZ,-1.0);
	}
}

void FsField::DrawVisual(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsMatrix4x4 &projMat,YSBOOL forShadowMap) const
{
	YsMatrix4x4 viewTfm;
	viewTfm.RotateXY(-viewAtt.b());
	viewTfm.RotateZY(-viewAtt.p());
	viewTfm.RotateXZ(-viewAtt.h());
	viewTfm.Translate(-viewPos);
	DrawVisual(viewTfm,projMat,forShadowMap);
}

void FsField::DrawVisual(const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,YSBOOL forShadowMap) const
{
	auto &commonTexture=FsCommonTexture::GetCommonTexture();

	commonTexture.LoadGroundTileTexture();
	commonTexture.LoadRunwayLightTexture();

	YsScenery::commonTexManPtr=&commonTexture.GetTextureManager();
	YsScenery::commonGroundTexHd=commonTexture.GetGroundTileTextureHd();
	YsScenery::commonRunwayLightTexHd=commonTexture.GetRunwayLightTextureHd();

	if(fld!=NULL)
	{
		fld->pos=pos;
		fld->att=att;
		fld->DrawVisual(viewMat,YsIdentity4x4(),projMat,-1.0,forShadowMap);
	}
}

void FsField::GetMapElevationCache(int &nCache,const double *&cache) const
{
	if(fld!=NULL)
	{
		if(fld->mapElevationCache.GetN()==0)
		{
			fld->CalculateMapElevationCache();
		}
		nCache=(int)fld->mapElevationCache.GetN();
		cache=fld->mapElevationCache;
	}
}

YSRESULT FsField::GetGroundSkyColor(YsColor &gnd,YsColor &sky) const
{
	if(fld!=NULL)
	{
		gnd=fld->GetGroundColor();
		sky=fld->GetSkyColor();
		return YSOK;
	}
	return YSERR;
}

const YsSceneryItem * FsField::GetFieldElevation(double &elv,const double &x,const double &z) const
{
	elv=0.0;
	if(fld!=NULL)
	{
		const YsSceneryItem *itm;

		fld->pos=pos;
		fld->att=att;

		elv=0.0;
		elv=fld->GetElevation(itm,YsVec3(x,0.0,z));
		return itm;
	}
	return NULL;
}

const YsSceneryItem * FsField::GetFieldElevationAndNormal(double &elv,YsVec3 &nom,const double &x,const double &z) const
{
	elv=0.0;
	nom=YsYVec();
	if(fld!=NULL)
	{
		const YsSceneryItem *itm;

		fld->pos=pos;
		fld->att=att;
		fld->GetElevationAndNormal(itm,elv,nom,YsVec3(x,0.0,z));
		return itm;
	}
	return NULL;
}

double FsField::GetBaseElevation(void) const
{
	if(NULL!=fld)
	{
		return fld->GetBaseElevation();
	}
	return 0.0;
}

YSBOOL FsField::CanResume(void) const
{
	if(NULL!=fld)
	{
		return fld->GetCanResume();
	}
	return YSFALSE;
}

YSBOOL FsField::CanContinue(void) const
{
	if(NULL!=fld)
	{
		return fld->GetCanContinue();
	}
	return YSFALSE;
}

YSBOOL FsField::GetFieldShellCollision(YsVec3 &itscPos,const YsVec3 &p1,const YsVec3 &p2) const
{
	if(nullptr!=fld)
	{
		fld->pos=pos;
		fld->att=att;
		if(nullptr!=fld->CheckShellCollision(itscPos,p1,p2,YsIdentity4x4()))
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsField::GetFieldShellCollision(const YsShell &shl,const YsMatrix4x4 modelTfm) const
{
	if(nullptr!=fld)
	{
		fld->pos=pos;
		fld->att=att;
		if(nullptr!=fld->CheckShellCollision(shl,modelTfm))
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSSCNAREATYPE FsField::GetAreaType(const YsVec3 &pos) const
{
	if(fld!=NULL)
	{
		return fld->GetAreaTypeFromPoint(pos);
	}
	return YSSCNAREA_NOAREA;
}

YSRESULT FsField::GetFieldRegionCenterLine(YsVec3 &o,YsVec3 &v,const YsSceneryRectRegion *rgn) const
{
	YsVec3 rect[4];
	GetFieldRegionRect(rect,rgn);

	const YsVec3 v1=rect[1]-rect[0];
	const YsVec3 v2=rect[2]-rect[1];

	o=(rect[0]+rect[2])/2.0;
	if(v1.GetSquareLength()>v2.GetSquareLength())
	{
		v=v1;
	}
	else
	{
		v=v2;
	}
	return v.Normalize();
}

YSRESULT FsField::GetFieldRegionRect(YsVec3 rec[4],const YsSceneryRectRegion *rgn) const
{
	if(fld!=NULL)
	{
		fld->pos=pos;
		fld->att=att;
		return fld->GetRegionRect(rec,rgn);
	}
	return YSERR;
}

YSRESULT FsField::GetFirstPointOfPointSet(YsVec3 &point,const YsSceneryPointSet *pst) const
{
	if(NULL!=fld)
	{
		return fld->GetFirstPointOfPointSet(point,pst);
	}
	return YSERR;
}
