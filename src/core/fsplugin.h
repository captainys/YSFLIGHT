#ifndef FSPLUGIN_IS_INCLUDED
#define FSPLUGIN_IS_INCLUDED
/* { */


class FsTerrainRegionId20080220
{
public:
	int rgnId;
	const char *rgnTag;
	YsVec3 cen,rect[4];
};

enum
{
	FSGETFLDRGN_NULL=0,
	FSGETFLDRGN_SETRECT=1  // Set this flag to get rect.  Otherwise rect will not be set.
};

class FsCallBack20080220
{
public:
// OS Dependent
#ifdef WIN32
	virtual HWND GetMainWindowHandle(void);  // Return NULL in server-console version
#endif

// Inquiry
	virtual void GetWindowSize(int &width,int &height);
	virtual YSBOOL DrawingMainWindow(void);
	virtual YSBOOL DrawingSubWindow(int subWndId);  // subWndId should be 1 or 2

	virtual class FsAirplane *GetPlayerAirplane(class FsSimulation *sim);

	virtual int GetNumAirplane(class FsSimulation *sim);
	virtual class FsAirplane *FindNextAirplane(class FsSimulation *sim,class FsAirplane *air);  // Give air=NULL to get the first airplane
	virtual class FsAirplane *FindPrevAirplane(class FsSimulation *sim,class FsAirplane *air);  // Give air=NULL to get the last airplane
	virtual YSBOOL IsAirplane(class FsExistence *obj); // True -> obj can be casted to FsAirplane *
	virtual YSBOOL IsGround(class FsExistence *obj); // True -> obj can be casted to FsGround *

	// Airplane State
	virtual const char *GetAirplaneName(class FsAirplane *air); // YFS name.  Not type identifier.
	virtual const char *GetAirplaneTypeName(class FsAirplane *air); // Type name (eg. F-15_EAGLE)
	virtual FSFLIGHTSTATE GetAirplaneState(class FsAirplane *air);
	virtual const YsVec3 &GetAirplanePosition(class FsAirplane *air);
	virtual const YsAtt3 &GetAirplaneAttitude(class FsAirplane *air);
	virtual const YsVec3 &GetAirplaneSpeed(YsVec3 &velocity,class FsAirplane *air);
	virtual const double &GetAirplaneFieldElevation(class FsAirplane *air); // Field elevation below the airplane
	virtual const YsVec3 &GetAirplaneFieldNormal(class FsAirplane *air); // Field normal below the airplane
	virtual const class FsGround *IsAirplaneOnCarrier(class FsSimulation *sim,class FsAirplane *air);

	// Airplane Control State
	virtual const double &GetAirplaneElevator(class FsAirplane *air);
	virtual const double &GetAirplaneElevatorTrim(class FsAirplane *air);
	virtual const double &GetAirplaneAileron(class FsAirplane *air);
	virtual const double GetAirplaneRudder(class FsAirplane *air);
	virtual const double &GetAirplaneThrottle(class FsAirplane *air);
	virtual YSBOOL GetAirplaneAfterburner(class FsAirplane *air);
	virtual YSBOOL IsFiringGun(class FsAirplane *air);

	// Airplane Characteristic
	virtual YSBOOL IsJet(class FsAirplane *air);
	virtual YSBOOL HasAfterburner(class FsAirplane *air);


	virtual int GetNumGround(class FsSimulation *sim);
	virtual class FsGround *FindNextGround(class FsSimulation *sim,class FsGround *gnd);
	virtual class FsGround *FindPrevGround(class FsSimulation *sim,class FsGround *gnd);
	virtual const YsVec3 &GetGroundPosition(class FsGround *gnd);
	virtual const YsAtt3 &GetGroundAttitude(class FsGround *gnd);
	virtual const YsVec3 &GetGroundSpeed(YsVec3 &velocity,class FsGround *gnd);

	// Ground State
	virtual const char *GetGroundName(class FsGround *gnd); // YFS name.  Not type identifier.
	virtual const char *GetGroundTypeName(class FsGround *gnd); // Type name (eg. F-15_EAGLE)


	// Field
	virtual const int GetFieldRegion(YsArray <FsTerrainRegionId20080220,16> &rgnList,class FsSimulation *sim,const double &x,const double &z,unsigned int flags);

// 2D Drawing (Use only in DrawForeground)
	virtual void DrawString(int x,int y,const char str[],const YsColor &col);
	virtual void DrawLine(int x1,int y1,int x2,int y2,const YsColor &col);
	virtual void DrawRect(int x1,int y1,int x2,int y2,const YsColor &col,YSBOOL fill);
	virtual void DrawCircle(int x,int y,int rad,const YsColor &col,YSBOOL fill);
	virtual void DrawPolygon(int n,int plg[],const YsColor &col);
	virtual void DrawDiamond(int x,int y,int r,const YsColor &col,YSBOOL fill);
	virtual void DrawX(int x,int y,int r,const YsColor &col);
	virtual void DrawPoint(int x,int y,const YsColor &col);

// Messages
	virtual void AddMessage(class FsSimulation *sim,const char str[]);
};


class FsPlugIn20080220
{
public:
	virtual void Initialize(class FsWorld *world);

	virtual void Interval(const double &ctime,class FsWorld *world,class FsSimulation *sim);
	virtual void DrawForeground(const double &ctime);
	virtual void WindowBufferSwapped(const double &ctime,class FsWorld *world,const class FsSimulation *sim);

	virtual void NetworkFreeMemory(const double &ctime,class FsWorld *world,class FsSimulation *sim);  // Called on garbage collection in the network mode.
};



/* } */
#endif
