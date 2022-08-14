#ifndef FSVEHICLEPROPERTY_IS_INCLUDED
#define FSVEHICLEPROPERTY_IS_INCLUDED
/* { */

#include <ysclass.h>
#include "fsproperty.h"

class FsVehicleProperty : public FsProperty
{
protected:
	FsVehicleProperty();
	~FsVehicleProperty(); // Protected & Non-Virtual Destructor: Preventing deletion through base-class pointer.  See "C++ Coding Standard" pp. 90
protected:
	// State
	YsVec3 staPosition;
	YsAtt3 staAttitude;
	YsMatrix4x4 staMatrix;
	YsMatrix4x4 staInverseMatrix;

	YsArray <FsRotatingTurretState> staTurret;
	mutable YsArray <FsRotatingTurretStateSave> staSavedTurret;
	YSBOOL staGunnerFirePermission;

	int staDamageTolerance;

	class FsGround *staOnThisCarrier;



	// Characteristic
	YsString chIdName;
	YsString chSubstIdName;
	double chOutsideRadius;

	YsVec3 chCockpit;          // Cockpit(?) Position
	YsVec3 chLookAt;
	double chDefZoom;
	YsArray <FsAdditionalViewpoint> chExtraView;

	YsArray <FsRotatingTurretProperty> chTurret;
	YSBOOL chHasAntiAirTurret;
	YSBOOL chHasAntiGndTurret;
	YSBOOL chHasPilotControlledTurret;
	YSBOOL chHasGunnerControlledTurret;

	YSBOOL chIsRacingCheckPoint;


public:
	void InitializeState(void);
	void InitializeCharacteristic(void);

	const char *GetIdentifier(void) const;
	const char *GetSubstIdName(void) const;
	const double &GetOutsideRadius(void) const;

	const YsVec3 &GetCockpitPosition(YsVec3 &cock) const;
	const YsVec3 &GetCockpitPosition(void) const;
	const YsVec3 &GetLookAtOffset(void) const;
	const double GetDefaultZoom(void) const;

	int GetNumAdditionalView(void) const;
	const FsAdditionalViewpoint *GetAdditionalView(int id) const;

	YSBOOL IsRacingCheckPoint(void) const;
	void SetIsRacingCheckPoint(YSBOOL flg);


	YsVec3 &GetPosition(YsVec3 &vec) const;
	YsAtt3 &GetAttitude(YsAtt3 &att) const;
	const YsVec3 &GetPosition(void) const;
	const YsAtt3 &GetAttitude(void) const;

	virtual YSBOOL IsAlive(void) const=0;
	virtual YSBOOL IsActive(void) const=0;

	const YsMatrix4x4 &GetMatrix(void) const;
	const YsMatrix4x4 &GetInverseMatrix(void) const;

	void RemakeMatrix(void);

	void MoveTurretGunner
	   (const double &dt,
	    FSIFF iff,
	    YSSIZE_T nAirTgt,const class FsAirplane * const airTgt[],
	    YSSIZE_T nGndTgt,const class FsGround * const gndTgt[]);
	void SetGunnerFirePermission(YSBOOL permission);
	YSBOOL GetGunnerFirePermission(void) const;

	int GetNumPilotControlledTurretBullet(void) const;
	int GetMaxNumPilotControlledTurretBullet(void) const;
	YSBOOL IsFiringPilotControlledTurret(void);
	const double GetMaxRotatinTurretRange(void) const;
	YSRESULT SetPilotControlledTurretHeading(const double &a);
	YSRESULT SetPilotControlledTurretPitch(const double &a);
	YSBOOL TurretStateChanged(void) const;
	void SaveTurretState(void) const;
	unsigned int EncodeTurretState(unsigned char dat[],int idOnSvr,const int netCmd) const; // netCmd can be FSNETCMD_AIRTURRETSTATE or FSNETCMD_GNDTURRETSTATE
	YSRESULT DecodeTurretState(unsigned char dat[],unsigned int packetLength);

	void SetDamageTolerance(int t);
	int GetDamageTolerance(void) const;

	YSBOOL IsOnCarrier(void) const;
	class FsGround *OnThisCarrier(void) const;

	virtual void CaptureState(YsArray <YsString> &stateStringArray) const=0;
};

/* } */
#endif
