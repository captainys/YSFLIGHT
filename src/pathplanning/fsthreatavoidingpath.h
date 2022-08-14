#ifndef FSTHREATAVOIDINGPATH_IS_INCLUDED
#define FSTHREATAVOIDINGPATH_IS_INCLUDED
/* { */

#include <ysgeometry.h>
#include <yspredefined.h>
#include <ysarray.h>
#include <yslattice.h>
#include <ysadvgeometry.h>

class FsThreatAvoidingPathFinder
{
public:
	class ThreatCircle : public YsCircle
	{
	public:
		YSBOOL isAlive;
	};
	class PathThreatIntersection
	{
	public:
		YSSIZE_T circleIdx;
		YsVec2 itsc[2];
		double dist[2];
	};

	YsVec2 start,goal;
	YsArray <ThreatCircle> circleArray;
	YsLattice2d <YsArray <int> > circleLattice;
	YsArray <PathThreatIntersection> itscArray;
	YsArray <YsVec2> flightPath;


	FsThreatAvoidingPathFinder();
	~FsThreatAvoidingPathFinder();
	void CleanUp(void);
	void AddCircle(const YsVec2 &pos,const double rad);
	YSRESULT MakeLattice(int maxNumLatticeCell);
private:
	void AddCircleToLattice(YSSIZE_T idx);
	void DeleteCircleFromLattice(YSSIZE_T idx);
public:
	YSBOOL RemoveOverlap(void); // Returns YSFALSE when no overlap is found.

public:
	void SetStart(const YsVec2 &pos);
	const YsVec2 &GetStart(void) const;
	void SetGoal(const YsVec2 &pos);
	const YsVec2 &GetGoal(void) const;

	void DisplaceStartPositionOutOfThreat(void);
	void CalculatePathCircleIntersection(void);
	void MakeFlightPath(void);
};

/* } */
#endif
