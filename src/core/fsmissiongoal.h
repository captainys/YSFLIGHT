#ifndef FSMISSIONGOAL_IS_INCLUDED
#define FSMISSIONGOAL_IS_INCLUDED
/* { */

#include "fsutil.h"
#include "fssiminfo.h"
#include "fssimulation.h"

class FsMissionGoal
{
private:
	YSBOOL isActiveMission;

public:
	YsArray <FsInternationalText> iText;
	YsArray <YsString> text;
	YsWString pngFn;

	unsigned int goalFlag;
	double duration;

	YsString landRegionName;
	YsString landCarrierName;

	int numPrmGndMustSurvive;
	int numPrmGndMustBeDestroyed;

	YsArray <YsString> defendGndName;
	YsArray <YsString> defendAirName;

	YsArray <YsString> destroyGndName;
	YsArray <YsString> destroyAirName;

private:
	YsArray <YsString> base64png,emptyBase64png;

public:
	class AircraftMustLand
	{
	public:
		YsString airLabel;
		FsSimInfo::AirBase base; // If 0<base.BaseName().Strlen(), the aircraft must land at the specified location.
	};
	YsArray <AircraftMustLand> mustLandAirName;



	FsMissionGoal();
	void Initialize(void);

	void SetIsActiveMission(YSBOOL isActiveMissionIn);
	const YSBOOL IsActiveMission(void) const;

	YSRESULT WriteFile(FILE *fp);
	YSRESULT SendCommand(const char str[],const wchar_t relPath[]);
	YSRESULT SendCommand(YSSIZE_T ac,YsString ag[],const wchar_t relPath[]);

	const YsArray <YsString> &GetText(void) const;
	const YsArray <FsInternationalText> &GetInternationalText(void) const;
	const YsArray <YsString> &GetBase64Png(int pngIdx) const;



	YSBOOL TestAllMissionGoalIsSatisfied(const class FsSimulation *sim) const;
	YSBOOL TestMissionDuration(const class FsSimulation *sim) const;
	YSBOOL TestSurvive(const class FsSimulation *sim) const;
	YSBOOL TestLanding(const class FsSimulation *sim) const;
	YSBOOL TestDefendPrimaryGround(int &nSurvive,const class FsSimulation *sim) const;
	YSBOOL TestDefendGround(const class FsSimulation *sim) const;
	YSBOOL TestDefendAir(const class FsSimulation *sim) const;
	YSBOOL TestDestroyPrimaryGround(int &nKill,const class FsSimulation *sim) const;
	YSBOOL TestDestroyGround(const class FsSimulation *sim) const;
	YSBOOL TestDestroyAir(const class FsSimulation *sim) const;
	YSBOOL TestDestroyAllAir(const class FsSimulation *sim) const;
	YSBOOL TestMustLandAir(const class FsSimulation *sim) const;
};

////////////////////////////////////////////////////////////

/* } */
#endif
