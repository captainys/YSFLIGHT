#include <ysclass.h>

#include "fsdef.h"
#include "fssimulation.h"
#include "fslattice.h"
#include "fsexistence.h"


FsLattice::FsLattice()
{
	totalNumUpdate=0;
	numUpdate=0;
}

FsLattice::~FsLattice()
{
}

void FsLattice::Initialize(void)
{
	SetUp(YsVec2(-1.0,-1.0),YsVec2(1.0,1.0),1,1);
}

YSRESULT FsLattice::SetUp(const YsVec2 &min,const YsVec2 &max,int lx,int ly)
{
	if(Create(lx,ly,min,max)==YSOK)
	{
		SetUpElement(-1,-1);
		for(int x=0; x<GetNumBlockX(); x++)
		{
			for(int y=0; y<GetNumBlockY(); y++)
			{
				SetUpElement(x,y);
			}
		}
		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsLattice::Add(const FsExistence *obj)
{
	int bx0=-1,by0=-1,bx1=-1,by1=-1;
	GetIndexRange(bx0,by0,bx1,by1,obj);

	for(int x=bx0; x<=bx1; x++)
	{
		for(int y=by0; y<=by1; y++)
		{
			FsLatticeElement *elm=GetElement(x,y);
			if(NULL!=elm)
			{
				switch(obj->GetType())
				{
				case FSEX_AIRPLANE:
					elm->air.Append((FsAirplane *)obj);
					break;
				case FSEX_GROUND:
					elm->gnd.Append((FsGround *)obj);
					break;
				default:
					break;
				}
			}
		}
	}

	obj->fsLtcCache[0].Set(bx0,by0);
	obj->fsLtcCache[1].Set(bx1,by1);

	return YSOK;
}

YSRESULT FsLattice::Delete(const FsExistence *obj)
{
	for(int x=obj->fsLtcCache[0].x(); x<=obj->fsLtcCache[1].x(); x++)
	{
		for(int y=obj->fsLtcCache[0].y(); y<=obj->fsLtcCache[1].y(); y++)
		{
			FsLatticeElement *elm=GetElement(x,y);
			switch(obj->GetType())
			{
			case FSEX_AIRPLANE:
				for(int i=(int)elm->air.GetN()-1; i>=0; i--)
				{
					if(elm->air[i]==obj)
					{
						elm->air.Delete(i);
					}
				}
				break;
			case FSEX_GROUND:
				for(int i=(int)elm->gnd.GetN()-1; i>=0; i--)
				{
					if(elm->gnd[i]==obj)
					{
						elm->gnd.Delete(i);
					}
				}
				break;
			default:
				break;
			}
		}
	}
	return YSOK;
}

YSRESULT FsLattice::Update(const FsSimulation *sim)
{
	numUpdate=0;

	for(const FsAirplane *air=NULL; NULL!=(air=sim->FindNextAirplane(air)); )
	{
		int bx0,by0,bx1,by1;
		GetIndexRange(bx0,by0,bx1,by1,air);

		if(bx0!=air->fsLtcCache[0].x() || by0!=air->fsLtcCache[0].y() ||
		   bx1!=air->fsLtcCache[1].x() || by1!=air->fsLtcCache[1].y())
		{
			Delete(air);
			Add(air);
			numUpdate++;
			totalNumUpdate++;
		}
	}

	for(const FsGround *gnd=NULL; NULL!=(gnd=sim->FindNextGround(gnd)); )
	{
		int bx0,by0,bx1,by1;
		GetIndexRange(bx0,by0,bx1,by1,gnd);

		if(bx0!=gnd->fsLtcCache[0].x() || by0!=gnd->fsLtcCache[0].y() ||
		   bx1!=gnd->fsLtcCache[1].x() || by1!=gnd->fsLtcCache[1].y())
		{
			Delete(gnd);
			Add(gnd);
			numUpdate++;
			totalNumUpdate++;
		}
	}

	return YSOK;
}

const FsLatticeElement &FsLattice::GetBlock(const YsVec2i &coord) const
{
	const FsLatticeElement *blk;
	blk=YsLattice2d <FsLatticeElement>::GetBlock(coord.x(),coord.y());
	if(blk!=NULL)
	{
		return *blk;
	}
	else
	{
		return outside;
	}
}

void FsLattice::TestDraw(void)
{
}

YSRESULT FsLattice::SetUpElement(int x,int y)
{
	FsLatticeElement *elm=GetElement(x,y);
	elm->bx=x;
	elm->by=y;
	elm->air.ClearDeep();
	elm->gnd.ClearDeep();
	return YSOK;
}

FsLatticeElement *FsLattice::GetElement(const YsVec3 &pos)
{
	int x,y;
	if(GetIndex(x,y,pos)==YSOK)
	{
		return GetElement(x,y);
	}
	return NULL;
}

FsLatticeElement *FsLattice::GetElement(int x,int y)
{
	if(0<=x && x<GetNumBlockX() &&
	   0<=y && y<GetNumBlockY())
	{
		return GetEditableBlock(x,y);
	}
	else
	{
		return &outside;
	}
}

const FsLatticeElement *FsLattice::GetElement(int x,int y) const
{
	if(0<=x && x<GetNumBlockX() &&
	   0<=y && y<GetNumBlockY())
	{
		return YsLattice2d <FsLatticeElement>::GetBlock(x,y);
	}
	else
	{
		return &outside;
	}
}

YSRESULT FsLattice::GetIndex(int &x,int &y,const YsVec3 &pos)
{
	YsVec2 pos2d;
	pos2d.Set(pos.x(),pos.z());
	if(GetBlockIndex(x,y,pos2d)!=YSOK)
	{
		x=-1;
		y=-1;
	}
	return YSOK;
}

YSRESULT FsLattice::GetIndexRange(int &bx0,int &by0,int &bx1,int &by1,const FsExistence *obj) const
{
	const YsVec3 cen=obj->GetPosition();
	const double rad=obj->GetApproximatedCollideRadius();

	YsVec3 min3d(cen.x()-rad,cen.y()-rad,cen.z()-rad);
	YsVec3 max3d(cen.x()+rad,cen.y()+rad,cen.z()+rad);

	return GetIndexRange(bx0,by0,bx1,by1,min3d,max3d);
}

YSRESULT FsLattice::GetIndexRange(int &bx0,int &by0,int &bx1,int &by1,const YsVec3 &min,const YsVec3 &max) const
{
	if(YSOK!=GetBlockIndex(bx0,by0,YsVec2(min.x(),min.z())) ||
	   YSOK!=GetBlockIndex(bx1,by1,YsVec2(max.x(),max.z())))
	{
		bx0=-1;
		by0=-1;
		bx1=-1;
		by1=-1; // <= GetElement will return &outside
	}
	return YSOK;
}
