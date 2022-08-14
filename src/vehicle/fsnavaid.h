#ifndef FSNAVAID_IS_INCLUDED
#define FSNAVAID_IS_INCLUDED
/* { */


class FsNavigationAid
{
public:
	virtual YSBOOL IsInRange(const YsVec3 &pos) const;
	virtual YSRESULT DrawObjectlay(const YsVec3 &pos,const YsAtt3 &att,const YsColor &col) const;
	virtual YSRESULT DrawOverlay(const YsVec3 &pos,const YsAtt3 &att,const YsColor &col) const;
	virtual double GetDeviationForScoringPurpose(const YsVec3 &pos,const YsAtt3 &att) const;
};

class FsILS
{
public:
	virtual YSBOOL IsInRange(const YsVec3 &pos) const;
	virtual YSRESULT DrawObjectlay(const YsVec3 &pos,const YsAtt3 &att,const YsColor &col) const;
	virtual YSRESULT DrawOverlay(const YsVec3 &pos,const YsAtt3 &att,const YsColor &col) const;

	static void DrawOverlay(const double &course,const double &latDev,const double &lngDev,const YsColor &col);

	virtual double GetDeviationForScoringPurpose(const YsVec3 &pos,const YsAtt3 &att) const;
	YSRESULT GetDeviation(double &latDev,double &lngDev,const YsVec3 &pos) const;
	double GetLocalizerHeading(void) const;
	const double &GetRange(void) const;  // If it is zero, it is not actually an ILS (like bridge)

	// This att is a bit confusing.  The forward vector of this att is reverse of the runway direction.  It points from the touch down point to the glide slope.
	void GetLandingPositionAndAttitude(YsVec3 &pos,YsAtt3 &att) const;

	YSRESULT Load(FILE *fp,const YsVec3 &fieldPos,const YsAtt3 &fieldAtt);
	YSRESULT Set(const YsVec3 &pos,const YsAtt3 &att,const double &range);
protected:

	YsVec3 pos;  // <- These values are updated in FsAircraftCarrierProperty::Move,
	YsAtt3 att;  // <- which is constantly called by FsGroundProperty::Move and FsGroundProperty::ReadbackRecord
	double range;// <- True location and attitude of ILS is stored in FsAircraftCarrierProperty
};

////////////////////////////////////////////////////////////

class FsVor
{
public:
	// obs is True Heading.
	static void GetDeviation(int &toFrom,double &deviation,const YsVec3 &airPos,const YsVec3 &vorPos,const double &obsInTrueHeading);
};

/* } */
#endif
