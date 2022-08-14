#ifndef FSLATTICE_IS_INCLUDED
#define FSLATTICE_IS_INCLUDED
/* { */

#include <ysclass.h>

class FsAirplane;
class FsGround;

class FsLatticeElement
{
public:
	int bx,by;
	YsArray <FsAirplane *,4> air;
	YsArray <FsGround *,4> gnd;
};

class FsLattice : protected YsLattice2d <FsLatticeElement>
{
protected:
	FsLatticeElement outside;

public:
	FsLattice();
	~FsLattice();

	int totalNumUpdate;
	int numUpdate;

	void Initialize(void);

	YSRESULT SetUp(const YsVec2 &min,const YsVec2 &max,int lx,int ly);

	YSRESULT Add(const class FsExistence *obj);
	YSRESULT Delete(const class FsExistence *obj);
	YSRESULT Update(const FsSimulation *sim);

	template <const int N>
	inline YSRESULT GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,const FsExistence *from) const;
	template <const int N>
	inline YSRESULT GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,const FsExistence *from) const;

	template <const int N>
	inline YSRESULT GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,const YsVec3 &cen,const double rad) const;
	template <const int N>
	inline YSRESULT GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,const YsVec3 &cen,const double rad) const;

	template <const int N>
	inline YSRESULT GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,const YsVec3 &p0,const YsVec3 &p1) const;
	template <const int N>
	inline YSRESULT GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,const YsVec3 &p0,const YsVec3 &p1) const;

	template <const int N>
	inline YSRESULT GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,int bx0,int by0,int bx1,int by1) const;
	template <const int N>
	inline YSRESULT GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,int bx0,int by0,int bx1,int by1) const;

	const FsLatticeElement &GetBlock(const YsVec2i &blk) const;

	void TestDraw(void);

protected:
	YSRESULT SetUpElement(int x,int y);
	FsLatticeElement *GetElement(const YsVec3 &pos);
	FsLatticeElement *GetElement(int x,int y);
	const FsLatticeElement *GetElement(int x,int y) const;
	YSRESULT GetIndex(int &x,int &y,const YsVec3 &pos);
	YSRESULT GetIndexRange(int &bx0,int &by0,int &bx1,int &by1,const FsExistence *obj) const;
	YSRESULT GetIndexRange(int &bx0,int &by0,int &bx1,int &by1,const YsVec3 &min,const YsVec3 &max) const;
};


template <const int N>
YSRESULT FsLattice::GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,const FsExistence *from) const
{
	int bx0,by0,bx1,by1;
	GetIndexRange(bx0,by0,bx1,by1,from);
	return GetAirCollisionCandidate(airCan,bx0,by0,bx1,by1);
}

template <const int N>
YSRESULT FsLattice::GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,const FsExistence *from) const
{
	int bx0,by0,bx1,by1;
	GetIndexRange(bx0,by0,bx1,by1,from);
	return GetGndCollisionCandidate(gndCan,bx0,by0,bx1,by1);
}

template <const int N>
YSRESULT FsLattice::GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,const YsVec3 &cen,const double rad) const
{
	const YsVec3 min3d(cen.x()-rad,cen.y()-rad,cen.z()-rad);
	const YsVec3 max3d(cen.x()+rad,cen.y()+rad,cen.z()+rad);

	int bx0,by0,bx1,by1;
	GetIndexRange(bx0,by0,bx1,by1,min3d,max3d);
	return GetAirCollisionCandidate(airCan,bx0,by0,bx1,by1);
}

template <const int N>
YSRESULT FsLattice::GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,const YsVec3 &cen,const double rad) const
{
	const YsVec3 min3d(cen.x()-rad,cen.y()-rad,cen.z()-rad);
	const YsVec3 max3d(cen.x()+rad,cen.y()+rad,cen.z()+rad);

	int bx0,by0,bx1,by1;
	GetIndexRange(bx0,by0,bx1,by1,min3d,max3d);
	return GetGndCollisionCandidate(gndCan,bx0,by0,bx1,by1);
}

template <const int N>
YSRESULT FsLattice::GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,const YsVec3 &p0,const YsVec3 &p1) const
{
	YsBoundingBoxMaker3 mkBbx;
	mkBbx.Add(p0);
	mkBbx.Add(p1);

	YsVec3 q0,q1;
	mkBbx.Get(q0,q1);

	int bx0,by0,bx1,by1;
	GetIndexRange(bx0,by0,bx1,by1,q0,q1);
	return GetAirCollisionCandidate(airCan,bx0,by0,bx1,by1);
}

template <const int N>
YSRESULT FsLattice::GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,const YsVec3 &p0,const YsVec3 &p1) const
{
	YsBoundingBoxMaker3 mkBbx;
	mkBbx.Add(p0);
	mkBbx.Add(p1);

	YsVec3 q0,q1;
	mkBbx.Get(q0,q1);

	int bx0,by0,bx1,by1;
	GetIndexRange(bx0,by0,bx1,by1,q0,q1);
	return GetGndCollisionCandidate(gndCan,bx0,by0,bx1,by1);
}

template <const int N>
YSRESULT FsLattice::GetAirCollisionCandidate(YsArray <FsAirplane *,N> &airCan,int bx0,int by0,int bx1,int by1) const
{
	airCan.Clear();
	for(int x=bx0; x<=bx1; x++)
	{
		for(int y=by0; y<=by1; y++)
		{
			const FsLatticeElement *elm=GetElement(x,y);
			if(NULL!=elm)
			{
				airCan.Append(elm->air);
			}
		}
	}
	YsRemoveDuplicateInUnorderedArray(airCan);
	return YSOK;
}

template <const int N>
YSRESULT FsLattice::GetGndCollisionCandidate(YsArray <FsGround *,N> &gndCan,int bx0,int by0,int bx1,int by1) const
{
	gndCan.Clear();
	for(int x=bx0; x<=bx1; x++)
	{
		for(int y=by0; y<=by1; y++)
		{
			const FsLatticeElement *elm=GetElement(x,y);
			if(NULL!=elm)
			{
				gndCan.Append(elm->gnd);
			}
		}
	}
	YsRemoveDuplicateInUnorderedArray(gndCan);
	return YSOK;
}

/* } */
#endif
