#ifndef FSHUD__IS_INCLUDED
#define FSHUD__IS_INCLUDED
/* { */

#include <ysglcpp.h>

class FsHud2
{
protected:
	float zPlane;
	const float refZPlane;

	void *specificResource;

	YsColor hudCol;
	YsVec3 viewPos;
	YsAtt3 airAtt;

	YsArray <double> bankArc;
	YsArray <double> bankHair;

	YsArray <double> headingOutside;
	YsArray <double> headingInside;

	YsGLVertexBuffer pointVtxBuf,lineVtxBuf,triVtxBuf;
	YsGLColorBuffer pointColBuf,lineColBuf,triColBuf;

	void TakeSpecificResource(void);
	void ReleaseSpecificResource(void);
public:
	FsHud2();
	~FsHud2();

	void BeginDrawHud(const YsVec3 &viewPos,const YsAtt3 &airAtt);
	void EndDrawHud(void);

	void SetColor(const YsColor &col);

	void DrawBank(const double &bank);

	void DrawHSI(
		const double x0,const double y0,const double scale,
	    const double hdg,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selectedObs,
	    YSBOOL showHdgBug,const double &hdgBug,YSBOOL selectedBug,YSBOOL vorInop);

	void DrawAdf(
	    const double &x0,const double &y0,const double &rad,
	    YSRESULT inRange,
	    const char adfId[],YSBOOL tuned,const double &obs,const double &bearing,YSBOOL selected,YSBOOL inop);

	void DrawNav(
	    const double &x0,const double &y0,const double &rad,
	    int navId,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selected,YSBOOL inop);

	void DrawAltitude(const double &x0,const double &y0,const double &wid,const double &hei,const double &altInMeter);
	void DrawVSI(const double &x0,const double &y0,const double &wid,const double &hei,const double &ftPerMin);

	void DrawAirSpeed(const double &x0,const double &y0,const double &wid,const double &hei,const double &speedInMeterPerSec);

	void DrawCrossHair(const double &lx,const double &ly,const double &centerBlank);

	void DrawThrottle(const double &x0,const double &y0,const double &wid,const double &hei,int nEng,const double thr[],const YSBOOL ab[]);
	void DrawFuelLeft(const double &x0,const double &y0,const double &wid,const double &hei,const double &fuel,const double &maxFuel);
	void DrawMachAndG(const double &x0,const double &y0,const double &fontWid,const double &fontHei,const double &mach,const double &g);
	void DrawNozzle(const double &x0,const double &y0,const double &wid,const double &hei,const YsVec3 &vec);

	void DrawGear(const double &x0,const double &y0,const double &wid,const double &hei,const double &gearPos);
	void DrawBrake(const double &x0,const double &y0,const double &wid,const double &hei,const double &brake);
	void DrawFlap(const double &x0,const double &y0,const double &wid,const double &hei,const double &flap);
	void DrawSpoiler(const double &x0,const double &y0,const double &wid,const double &hei,const double &spoiler);

	void DrawAutoPilot(const double &x0,const double &y0,const double &wid,const double &hei,YSBOOL autoPilot);

	void DrawControlSurface(
	    const double &x0,const double &y0,const double &wid,const double &hei,
	    const double &elv,const double &elvTrim,const double &ail,const double &rud);
	void DrawElevatorTrim(
	    const double &x0,const double &y0,const double &wid,const double &hei,const double elvTrim);

	void DrawAttitude(
	    const double &rad,const double &lng,const double &fontWid,const double &fontHei,
	    const YsVec3 &cockpitPos,const YsAtt3 &att,const YsAtt3 &indicated,const YsVec3 &viewPos,const YsAtt3 &viewAtt);

	void DrawAmmo(
	    const double &x0,const double &y0,const double &fontWid,const double &fontHei,
	    const class FsAmmunitionIndication &ammo);

	void DrawVelocityVectorIndicator(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsVec3 &vel);

	void DrawTurnAndSlipIndicator(const double cx,const double cy,const double rad,const double ssa,const double turnRate);
};

/* } */
#endif
