#ifndef FSHUD_IS_INCLUDED
#define FSHUD_IS_INCLUDED
/* { */

class FsHeadUpDisplay
{
public:
	FsHeadUpDisplay();

protected:
	long wid,hei,lupX,lupY;

	static YsArray <double> trigonomyTable;

public:
	YsColor hudCol;

	enum
	{
		FONTPITCH=22
	};

	void SetArea(long x1,long y1,long dx,long dy);
	void SetAreaByCenter(long cx,long cy,long dx,long dy);

	void Draw(YSBOOL autoPilot,const class FsCockpitIndicationSet &cockpitIndicationSet);
	void DrawCrossHair(void);
	void DrawHeading(const YsAtt3 &hdg,YSBOOL showHdgBug,const double &hdgBug,YSBOOL selected);
	void DrawThrottle(int nEng,const double thr[],const YSBOOL ab[]);
	void DrawNozzle(const YsVec3 &vec);
	void DrawFuelLeft(double fuel,double maxFuel);
	void DrawWeapon(const class FsAmmunitionIndication &ammo);
	void DrawG(double g);
	void DrawMach(double mach);
	void DrawElevator(double elv,double trim,YSBOOL au);
	void DrawAileron(double ail,YSBOOL au);
	void DrawRudder(double rud);
	void DrawSpeed(const double &spd);
	void DrawAltitude(double alt);
	void DrawGearFlapBrake(double ldg,double brk,double flp,double abr,YSBOOL autoPilot);
	void DrawClimbRatio(const double &climbRatio);
	void DrawBank(const double &b);

	void DrawAttitude(const YsVec3 &cockpitPos,const YsAtt3 &att,const YsVec3 &viewPos,const YsAtt3 &viewAtt);
	void DrawVelocityVectorIndicator(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsVec3 &vel);

	void DrawCircleContainer
		   (const YsMatrix4x4 &viewpoint,
		    const YsAtt3 &viewAtt,
		    const YsVec3 &target,
		    const YsVec3 &from,
		    const YsColor &color,
		    const char caption[],
		    const char caption2[],
		    YSBOOL dot,
		    int begin,int step);

	void DrawCrossDesignator
		   (const YsMatrix4x4 &viewpoint,
		    const YsAtt3 &viewAtt,
		    const YsVec3 &target,
		    const YsVec3 &from,
		    const YsColor &color,
		    YSBOOL dot);

	void DrawCrossDesignator2
		   (const YsMatrix4x4 &viewpoint,
		    const YsAtt3 &viewAtt,
		    const YsVec3 &target,
		    const YsVec3 &from,
		    const YsColor &color,
		    YSBOOL dot);

	void DrawTurnAndSlipIndicator(const double ssa,const double turnRate) const;

	static void DrawCircularBackground(int cx,int cy,int rad);
	static void DrawCircularFrame(int cx,int cy,int rad,const YsColor &col);
	static void DrawNeedle(int cx,int cy,int wid,int lng,const double &ang,int tailLng);

	static void DrawIls(
	    int cx,int cy,int outRad,int inRad,const YsColor &col,
	    YSBOOL isIls,
	    const double &lc,const double &gs,const double &hdg,const char stationName[],int toFrom,
	    YSBOOL isDme,const double &dme,YSBOOL currentSign);
	static void DrawVor(
	    int cx,int cy,int outRad,int inRad,const YsColor &col,
	    YSBOOL isIls,
	    const double &lc,const double &hdg,const char stationName[],int toFrom,
	    YSBOOL isDme,const double &dme,YSBOOL currentSign);
	static void DrawAdf(int cx,int cy,int rad,const YsColor &col,const double &relHdg,const char stationName[],YSBOOL currentSign);
};

/* } */
#endif
