#ifndef FSINSTPANEL_IS_INCLUDED
#define FSINSTPANEL_IS_INCLUDED
/* { */

#include <ysglcpp.h>

class FsInstrumentPanel
{
public:
	class Tachometer
	{
	public:
		YsVec2 pos;
		double rad;
		int rpmMin,rpmMax;
		int greenArcMin,greenArcMax;
		int redLine;
		YsArray <int> tickLocationArray;
		double startAngle,endAngle;;
	};

	YsGLVertexBuffer pointVtxBuf,lineVtxBuf,triVtxBuf;
	YsGLColorBuffer pointColBuf,lineColBuf,triColBuf;

	YsGLVertexBuffer ovLineVtxBuf,ovTriVtxBuf;
	YsGLColorBuffer ovLineColBuf,ovTriColBuf;

public:
	int wid,hei;  // Screen width (Temporary use in draw2D functions)

	YsVec2 panelRange[2];

	YSBOOL drawSpeed;
	YsVec2 speedPos;
	double speedRad;
	YSBOOL speedShowMachNumber;
	double speedVfe,speedVno,speedVne,speedIndicatorRange;
	double speedArc;

	YSBOOL drawAlt;
	YsVec2 altPos;
	double altRad;

	YSBOOL drawAtt;
	YsVec2 attPos;
	double attRad;

	YSBOOL drawHdg;
	YsVec2 hdgPos;
	double hdgRad;

	YSBOOL drawHsi;
	YsVec2 hsiPos;
	double hsiRad;

	YSBOOL drawClimb;
	YsVec2 climbPos;
	double climbRad;

	YSBOOL drawEngine;
	YsVec2 enginePos;
	double engineRad;

	YSBOOL drawIls;
	YsVec2 ilsPos;
	double ilsRad;

	YSBOOL drawVor;
	YsVec2 vorPos;
	double vorRad;

	YSBOOL drawAdf;
	YsVec2 adfPos;
	double adfRad;

	YSBOOL drawTurnCoordinator;
	YsVec2 turnCoordinatorPos;
	double turnCoordinatorRad;

	YSBOOL drawCrossHair;
	YsVec2 crossHairPos;
	double crossHairRad;

	YSBOOL drawAmmo;
	YsVec2 ammoPos;
	double ammoRad;

	YSBOOL drawG;
	YsVec2 g1,g2;

	YSBOOL drawTrim;
	YsVec2 trim1,trim2;

	YSBOOL drawFuel;
	YsVec2 fuel1,fuel2;

	YSBOOL drawGear;
	YsVec2 gear1,gear2;

	YSBOOL drawFlap;
	YsVec2 flap1,flap2;

	YSBOOL drawSpoiler;
	YsVec2 spoil1,spoil2;

	YSBOOL drawBrake;
	YsVec2 brake1,brake2;

	YSBOOL drawRPM;
	YsArray <Tachometer> tachometerArray;

	YsArray <YsVec2> panelPlg;

	YsVec3 viewPosCache;
	YsVec3 localViewPosCache;
	const class FsAirplaneProperty *airPropCache;

public:
	FsInstrumentPanel();
	~FsInstrumentPanel();

	YSRESULT LoadIsp(const wchar_t fn[]);



// 3D Inst Panel
public:
	void BeginDraw3d(const YsVec3 &viewPos,const YsVec3 &localViewPos,const class FsAirplaneProperty &prop);
	void Draw3d(const FsAirplaneProperty &airProp,const class FsCockpitIndicationSet &cockpitIndicationSet);
	void EndDraw3d(void);

	YSBOOL HasHSI(void) const;

protected:
	void DrawBackground(void);

	void AddCircularInstrumentFrameVertexArray(const YsVec2 &cen,const double &rad);
	void AddRectangularInstrumentFrameVertexArray(const YsVec2 &g1,const YsVec2 &g2);

	static void AddNeedleVertexArray(
   	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
		YsColor lineCol,YsColor fillCol,
	    const YsMatrix4x4 &tfm,
	    const double &wid,const double &lng,const double &ang,const double &tailLng);

	void DrawSpeed3d(
	    const double &spd,const double &vfe,const double &vno,const double &vne,const double &vRng,const double &mach,
	    const double &vArc);

	void DrawCrossHair3d(void);

	void DrawAltitude3d(const double &iAlt);

	void DrawAttitude3d(const double &p,const double &b);
	void DrawVSI3d(const double &vSpd);
	void DrawAmmo3d(int nGun,int maxNGun,FSWEAPONTYPE wpnChoice,int wpnChoiceLeft);
	void DrawEngine3d(const double &thr,YSBOOL ab);
	void DrawTurnCoordinator3d(const double &ssa,const double &turn);
	void DrawG3d(const double &g);
	void DrawTrim3d(const double elvTrim) const;
	void DrawSimpleRectInstrument3d(const YsVec2 &g1,const YsVec2 &g2,const char caption[],const char captionBtm[],const char captionTop[],const double &fuelPercent);
	void DrawBrake3d(const double &brake);
	void DrawGear3d(const double &brake);
	void DrawTach3d(int tachIdx,const double rpm);

	static void AddDirectionVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
		const YsColor col,
	    const YsMatrix4x4 &tfm);// Can also be used from ILS(NAV1)/VOR(NAV2)
	static void AddReverseDirectionVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &tricolBuf,
		const YsColor col,
	    const YsMatrix4x4 &tfm); // Can also be used from ILS(NAV1)/VOR(NAV2)

	static void AddHeadingBugVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
	    double hdgBug,YSBOOL selected,
		const YsColor col,
	    const YsMatrix4x4 &instTfm);

	void DrawHeading3d(const double &hdg,YSBOOL showHdgBug,const double &hdgBug,YSBOOL selected);

public:
	void DrawNav3d(
	    int navId,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selected,YSBOOL inop);

	static void AddNavVertexArray(
		    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,
		    YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
		    YsColor lineCol,YsColor fillCol,
		    const YsMatrix4x4 &tfmIn,

		    int navId,
		    YSRESULT inRange,
		    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
		    YSBOOL isDme,const double &dme,
		    YSBOOL selected,YSBOOL inop);

	void DrawAdf3d(
	    YSRESULT inRange,
	    const char adfId[],YSBOOL tuned,const double &obs,const double &bearing,YSBOOL selected,YSBOOL inop);

	static void AddAdfVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,
	    YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
	    YsColor lineCol,YsColor fillCol,
	    const YsMatrix4x4 &tfm,
	    YSRESULT inRange,
	    const char adfId[],YSBOOL tuned,const double &obs,const double &bearing,YSBOOL selected,YSBOOL inop);


	void DrawHsi3d(
	    const double &hdg,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selectedObs,
	    YSBOOL showHdgBug,const double &hdgBug,YSBOOL selectedBug,YSBOOL vorInop);

	static void AddHsiVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,
	    YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
	    const double hdg,
	    YsColor col,
	    const YsMatrix4x4 &tfm,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selectedObs,
	    YSBOOL showHdgBug,const double &hdgBug,YSBOOL selectedBug,YSBOOL vorInop);
};


/* } */
#endif
