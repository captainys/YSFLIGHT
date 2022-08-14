#include "fsthreatavoidingpath.h"

FsThreatAvoidingPathFinder::FsThreatAvoidingPathFinder()
{
	CleanUp();
}
FsThreatAvoidingPathFinder::~FsThreatAvoidingPathFinder()
{
	CleanUp();
}
void FsThreatAvoidingPathFinder::CleanUp(void)
{
	circleArray.CleanUp();
	circleLattice.CleanUp();
}

void FsThreatAvoidingPathFinder::AddCircle(const YsVec2 &pos,const double rad)
{
	circleArray.Increment();
	circleArray.Last().SetCenter(pos);
	circleArray.Last().SetRadius(rad);
	circleArray.Last().isAlive=YSTRUE;
}

YSRESULT FsThreatAvoidingPathFinder::MakeLattice(int maxNumLatticeCell)
{
	if(0==circleArray.GetN())
	{
		return YSERR;
	}

	double avgRad=0.0;

	YsBoundingBoxMaker2 mkBbx;
	for(auto circle : circleArray)
	{
		const YsVec2 &cen=circle.Center();
		const double rad=circle.Radius();

		const auto min=cen-YsXYVec2()*rad;
		const auto max=cen+YsXYVec2()*rad;

		mkBbx.Add(min);
		mkBbx.Add(max);

		avgRad+=rad;
	}

	if(YsTolerance>avgRad)
	{
		return YSERR;
	}

	YsVec2 bbx[2],diagon;
	mkBbx.Get(bbx);
	diagon=bbx[1]-bbx[0];
	avgRad/=(double)circleArray.GetN();

	const double avgRadSq=YsGreater(avgRad*avgRad,YsTolerance);
	const double ltcArea=diagon.x()*diagon.y();

	if(YsTolerance>ltcArea)
	{
		return YSERR;
	}

	const YSSIZE_T nBlk=YsBound<int>((int)(ltcArea/avgRadSq),1,maxNumLatticeCell);

	circleLattice.Create(nBlk,bbx[0],bbx[1]);

	for(auto circleIdx : circleArray.AllIndex())
	{
		AddCircleToLattice((int)circleIdx);
	}

	return YSOK;
}

void FsThreatAvoidingPathFinder::AddCircleToLattice(YSSIZE_T circleIdx)
{
	auto &circle=circleArray[circleIdx];

	const YsVec2 min=circle.Center()-circle.Radius()*YsXYVec2();
	const YsVec2 max=circle.Center()+circle.Radius()*YsXYVec2();

	const YsVec2i b0=circleLattice.GetBlockIndexAutoBound(min);
	const YsVec2i b1=circleLattice.GetBlockIndexAutoBound(max);

	for(auto idx : YsVec2iRange(b0,b1))
	{
		auto &blk=circleLattice[idx];
		blk.Append(circleIdx);
	}
}

void FsThreatAvoidingPathFinder::DeleteCircleFromLattice(YSSIZE_T circleIdx)
{
	auto &circle=circleArray[circleIdx];

	const YsVec2 min=circle.Center()-circle.Radius()*YsXYVec2();
	const YsVec2 max=circle.Center()+circle.Radius()*YsXYVec2();

	const YsVec2i b0=circleLattice.GetBlockIndexAutoBound(min);
	const YsVec2i b1=circleLattice.GetBlockIndexAutoBound(max);

	for(auto bxby : YsVec2iRange(b0,b1))
	{
		auto &blk=circleLattice[bxby];
		for(YSSIZE_T idx=blk.GetN()-1; 0<=idx; --idx)
		{
			if(blk[idx]==circleIdx)
			{
				blk.DeleteBySwapping(idx);
			}
		}
	}
}

YSBOOL FsThreatAvoidingPathFinder::RemoveOverlap(void)
{
	YSBOOL overlapping=YSFALSE;

	for(auto circleIdx : circleArray.AllIndex())
	{
		auto &circle=circleArray[circleIdx];
		if(YSTRUE!=circle.isAlive)
		{
			continue;
		}

		const YsVec2 min=circle.Center()-circle.Radius()*YsXYVec2();
		const YsVec2 max=circle.Center()+circle.Radius()*YsXYVec2();

		const YsVec2i b0=circleLattice.GetBlockIndexAutoBound(min);
		const YsVec2i b1=circleLattice.GetBlockIndexAutoBound(max);

		for(auto bxby : YsVec2iRange(b0,b1))
		{
		RETRY:
			auto &blk=circleLattice[bxby];
			for(auto circleIdx2 : blk)
			{
				if(circleIdx2!=circleIdx &&
				   YSTRUE==circle.Overlap(circleArray[circleIdx2]))
				{
					auto r0=circle.Radius();
					auto &c0=circle.Center();
					auto r1=circleArray[circleIdx2].Radius();
					auto &c1=circleArray[circleIdx2].Center();

					YsVec2 newC;
					double newR;

					const double cenDist=(c0-c1).GetLength();
					if(cenDist+r0<r1) // c1,r1 eats c0,r0
					{
						newC=c1;
						newR=r1;
					}
					else if(cenDist+r1<r0) // c0,r0 eats c1,r1
					{
						newC=c0;
						newR=r0;
					}
					else
					{
						newR=(cenDist+r0+r1)/2.0;
						newC=((newR-r0)*c1+(newR-r1)*c0)/(newR+newR-r0-r1);
					}

					DeleteCircleFromLattice(circleIdx);
					DeleteCircleFromLattice(circleIdx2);
					circleArray[circleIdx2].isAlive=YSFALSE;

					circle.SetCenter(newC);
					circle.SetRadius(newR);
					AddCircleToLattice(circleIdx);

					overlapping=YSTRUE;
					goto RETRY;
				}
			}
		}
	}

	return overlapping;
}

void FsThreatAvoidingPathFinder::SetStart(const YsVec2 &pos)
{
	start=pos;
}
const YsVec2 &FsThreatAvoidingPathFinder::GetStart(void) const
{
	return start;
}
void FsThreatAvoidingPathFinder::SetGoal(const YsVec2 &pos)
{
	goal=pos;
}
const YsVec2 &FsThreatAvoidingPathFinder::GetGoal(void) const
{
	return goal;
}

void FsThreatAvoidingPathFinder::DisplaceStartPositionOutOfThreat(void)
{
	for(auto &circle : circleArray)
	{
		if(YSTRUE==circle.IsInside(start))
		{
			YsVec2 outDir=start-circle.Center();
			if(YSOK==outDir.Normalize())
			{
				start=circle.Center()+outDir*circle.Radius()*1.1;
			}
		}
	}
}

void FsThreatAvoidingPathFinder::CalculatePathCircleIntersection(void) // Still order of N.
{
	itscArray.CleanUp();

	YsArray <double,16> itscDist;

	YsVec2 pathDir=YsUnitVector(goal-start);
	for(auto circleIdx : circleArray.AllIndex())
	{
		auto &circle=circleArray[circleIdx];
		YsVec2 itsc[2];
		if(YSTRUE==circle.isAlive &&
		   YSOK==YsComputeCircleLineIntersection(itsc,circle.Center(),circle.Radius(),start,goal))
		{
			double dist[2]=
			{
				(itsc[0]-start)*pathDir,
				(itsc[1]-start)*pathDir,
			};
			if(dist[0]>dist[1])
			{
				YsSwapSomething(dist[0],dist[1]);
				YsSwapSomething(itsc[0],itsc[1]);
			}

			itscArray.Increment();
			itscArray.Last().circleIdx=circleIdx;
			itscArray.Last().itsc[0]=itsc[0];
			itscArray.Last().itsc[1]=itsc[1];
			itscArray.Last().dist[0]=dist[0];
			itscArray.Last().dist[1]=dist[1];

			itscDist.Append(dist[0]);
		}
	}

	YsQuickSort(itscDist.GetN(),itscDist.GetEditableArray(),itscArray.GetEditableArray());
}

void FsThreatAvoidingPathFinder::MakeFlightPath(void)
{
	// Very ad-hoc and incomplete path-calculation. See pathCalculation.jnt

	flightPath.MakeUnitLength(start);

	const YsVec2 pathDir=YsUnitVector(goal-start);
	const YsVec2 pathNom=YsVec2(pathDir.y(),-pathDir.x());

	YsVec2 currentPos=start;
	for(auto itscIdx : itscArray.AllIndex())
	{
		auto &itsc=itscArray[itscIdx];
		auto &circle=circleArray[itsc.circleIdx];

		const YsVec2 detourCandidate[2]=
		{
			circle.Center()+circle.Radius()*pathNom,
			circle.Center()-circle.Radius()*pathNom,
		};
		const double detourCandidateDist[2]=
		{
			YsGetPointLineDistance2(start,goal,detourCandidate[0]),
			YsGetPointLineDistance2(start,goal,detourCandidate[1])
		};

		const YsVec2 &detour=(detourCandidateDist[0]<detourCandidateDist[1] ? detourCandidate[0] : detourCandidate[1]);

		// See pathCalculation.jnt
		YsVec2 midVec=YsUnitVector(detour+itsc.itsc[0]-2.0*circle.Center());
		YsVec2 midPoint=circle.Center()+midVec*circle.Radius();
		YsVec2 tanVec(midVec.y(),-midVec.x());
		YsVec2 wayPoint;

		if(YSOK==YsGetLineIntersection2(wayPoint,start,goal,midPoint,midPoint+tanVec))
		{
			flightPath.Append(wayPoint);
		}
		if(YSOK==YsGetLineIntersection2(wayPoint,detour,detour+pathDir,midPoint,midPoint+tanVec))
		{
			flightPath.Append(wayPoint);
			flightPath.Append(detour+(detour-wayPoint));
		}

		midVec=YsUnitVector(detour+itsc.itsc[1]-2.0*circle.Center());
		midPoint=circle.Center()+midVec*circle.Radius();
		tanVec.Set(midVec.y(),-midVec.x());
		if(YSOK==YsGetLineIntersection2(wayPoint,start,goal,midPoint,midPoint+tanVec))
		{
			flightPath.Append(wayPoint);
		}
	}

	flightPath.Append(goal);
}
