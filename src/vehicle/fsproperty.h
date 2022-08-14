#ifndef FSPROPERTY_IS_INCLUDED
#define FSPROPERTY_IS_INCLUDED
/* { */

#include <fsdef.h>

enum
{
	FSTURRETPROP_ANTIGND=1,
	FSTURRETPROP_ANTIAIR=2,
	FSTURRETPROP_CANBEVIEWPOINT=4
};

class FsRotatingTurretProperty
{
public:
	YsVec3 cen;           // Rotation Center
	YsAtt3 att;           // RelativeAttitude
	double radius;        // Distance from Rotation Center to the tip of the gun

	double hMin,hMax;     // Heading range
	double pMin,pMax;     // Pitch range
	double pZero,hZero;   // Neutral angle
	double vp,vh;         // Turret Rotation Speed

	int maxNumGunBullet;  // Bullet Left
	int destructivePower; // Power
	FSWEAPONTYPE wpnType; // What to shoot?

	// Following names, nodeIDs, and visuals are cache.
	// Why 3?  A turret property can be used for setting up primary FsVisual and coarse FsVisual
	// of the same airplane.  
	mutable YsString dnmNodeName; // Corresponding Dnm Part Name
	mutable int dnmNodeId[3];     // Dnm node ID for Heading+Pitch Rotation
	mutable class FsVisualDnm *dnm[3];  // Associated DNM pointer

	mutable YsString dnmHdgNodeName; // Corresponding Dnm Part Name
	mutable int dnmHdgNodeId[3];     // Dnm node ID for Heading Rotation
	mutable class FsVisualDnm *dnmHdg[3];  // Associated DNM pointer for Heading Rotation

	mutable YsString dnmPchNodeName; // Corresponding Dnm Part Name
	mutable int dnmPchNodeId[3];     // Dnm node ID for Heading Rotation
	mutable class FsVisualDnm *dnmPch[3];  // Associated DNM pointer for Heading Rotation

	unsigned int turretProp;
	FSTURRETCONTROLLER controlledBy;
	double range,bulSpeed;

	double shootInterval; // Fire once every ? second

	FsRotatingTurretProperty();
	void Initialize(void);
};

enum
{
	FSTURRETSTATE_DISABLED=1,
	FSTURRETSTATE_FIRING=2,
	FSTURRETSTATE_GUNNERFIREATWILL=4
};

class FsRotatingTurretState
{
public:
	unsigned int airTargetKey;
	unsigned int gndTargetKey;
	int numBullet;
	double h,p;
	double ctlH,ctlP;
	double shootGunTimer;
	unsigned int turretState;

	FsRotatingTurretState();
	void Initialize(void);
	void MoveGunner(
	    const FsRotatingTurretProperty &chTurret,
	    const double &dt,FSIFF iff,
	    YSSIZE_T nAirTgt,const class FsAirplane * const airTgt[],
	    YSSIZE_T nGndTgt,const class FsGround * const gndTgt[],
	    YSBOOL gunnerFirePermission,
	    const YsMatrix4x4 &invMat);
	void SetupVisual(class FsVisualDnm &vis,const FsRotatingTurretProperty &chTurret) const;
	void FireWeapon(
	    const FsRotatingTurretProperty &chTurret,
	    const double &ctime,
	    const double &dt,
	    const YsVec3 &iniVel,
	    const YsMatrix4x4 &posiMat,
	    class FsSimulation *sim,
	    class FsWeaponHolder &bul,
	    class FsExistence *owner);
};

class FsRotatingTurretStateSave
{
public:
	unsigned int turretState;
	double h,p;
};

////////////////////////////////////////////////////////////

class FsProperty
{
public:
};

////////////////////////////////////////////////////////////

enum FSADDITIONALVIEWPOINTTYPE
{
	FS_ADVW_INSIDE,
	FS_ADVW_OUTSIDE,
	FS_ADVW_CABIN
};

class FsAdditionalViewpoint : public FsProperty
{
public:
	YsString name;
	YsVec3 pos;
	YsAtt3 att;
	FSADDITIONALVIEWPOINTTYPE vpType;

	// The following two properties control if the HUD or InstPanel must be drawn in the EXCAMERA.
	// It will allow HUD or InstPanel visible only in some EXCAMERA.
	// The switches are tested in FsSimulation::GetInstrumentDrawSwitch.
	YSBOOL showHudIfAvailable;
	YSBOOL showInstPanelIfAvailable;
};


/* } */
#endif
