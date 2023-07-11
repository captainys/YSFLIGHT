#ifndef FSWEAPON_IS_INCLUDED
#define FSWEAPON_IS_INCLUDED
/* { */

////////////////////////////////////////////////////////////

#include "fsdef.h"
#include "fsrecord.h"
#include "fsinstreading.h"

class FsWeaponRecord
{
public:
	FSWEAPONTYPE type;
	float x,y,z,h,p,b;
	float velocity,lifeRemain;
	int power;
	class FsExistence *firedBy;
	FSWEAPON_CREDIT_OWNER creditOwner;

	float vMax,mobility,radar;
	class FsExistence *target;
};

class FsKillCredit
{
public:
	FsExistence *whoKilled,*whom;
	FSWEAPON_CREDIT_OWNER creditOwner;
	FSWEAPONTYPE byWhatKindOfWeapon;
	YsVec3 where;
	double when;

	int GetAntiAirScore(YsString &msg,const class FsSimulation *sim,const class FsExistence *player) const;
	int GetAntiGroundScore(YsString &msg,const class FsSimulation *sim,const class FsExistence *player) const;
};


////////////////////////////////////////////////////////////


class FsWeaponSmokeTrail
{
public:
	enum
	{
		TIMEPERSEG=100, // ms
		MAXNUMTRAIL=64
	};

	int trailBegin,trailUsed;
	double trailNextRecord;
	YsVec3 trailPos[MAXNUMTRAIL];
	YsAtt3 trailAtt[MAXNUMTRAIL];
	double trailTime[MAXNUMTRAIL];

	YSBOOL used;

	void Initialize(void);

	void MakeVertexArray(
	    class YsGLVertexBuffer vtxBuf[4],class YsGLNormalBuffer nomBuf[4],class YsGLColorBuffer colBuf[4],
	    const YsVec3 &pos,const YsAtt3 &att,FSSMOKETYPE smk,const double &cTime) const;

	void Draw(const YsVec3 &pos,const YsAtt3 &att,YSBOOL transparency,FSSMOKETYPE smk,const double &cTime) const;
	void Add(const double &dt,const double &cTime,const YsVec3 &pos,const YsAtt3 &att);

	void AddToParticleManagerAsFlare(class YsGLParticleManager &partMan,const YsVec3 cPos,const double cTime,YSBOOL includeCurrentPos);
	void AddToParticleManager(class YsGLParticleManager &partMan,const YsVec3 cPos,const double cTime,YSBOOL includeCurrentPos);
};



// Declaration /////////////////////////////////////////////
class FsWeapon
{
public:
	enum
	{
		POWER_AGM65=12
	};


	FsWeapon *prev,*next;
	FsWeapon *prevFlare,*nextFlare;

	FSWEAPONTYPE type;

	double lifeRemain;   // Life remaining by distance
	double timeRemain;   // Time remaining after impact (to draw smoke better)
	double timeUnguided; // Duration of unguided flying  2005/02/18 Set in FsWeapon::Fire
	double velocity;
	YsVec3 prv,pos,vec;
	YsVec3 lastChecked;
	class FsExistence *firedBy;
	FSWEAPON_CREDIT_OWNER creditOwner;

	int destructivePower;

	// for Missile Only
	double maxVelocity;
	double mobility,radar;
	YsAtt3 att;
	class FsExistence *target;

	FsWeaponSmokeTrail *trail;



	// ***s (eg. aim9s, agm65s) stands for silent.
	static class FsVisualDnm aim9,aim9s,aim_coarse;
	static class FsVisualDnm aim9x,aim9xs,aim9x_coarse;
	static class FsVisualDnm agm65,agm65s,agm_coarse;
	static class FsVisualDnm bomb,bomb_coarse;
	static class FsVisualDnm rocket,rockets,rocket_coarse;

	static class FsVisualDnm aim120,aim120s,aim120_coarse;  // 2004/01/22
	static class FsVisualDnm bomb250,bomb250s,bomb250_coarse;  // 2004/01/22
	static class FsVisualDnm bomb500hd,bomb500hds,bomb500hd_coarse;  // 2004/01/22

	static class FsVisualDnm flarePod;
	static class FsVisualDnm fuelTank;

	static FsAmmunitionIndication::WEAPONTYPE WeaponTypeToWeaponIndicationType(FSWEAPONTYPE wpnType);

	static void DrawVisual
	   (FSWEAPONTYPE type,YSBOOL coarse,
	   const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,
	   const YsVec3 &pos,const YsAtt3 &att,unsigned int drawFlag);
	static void DrawShadow
	   (FSWEAPONTYPE type,YSBOOL coarse,
	   const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,
	   const YsVec3 &pos,const YsAtt3 &att,const YsMatrix4x4 &projPlnTfm);



	FsWeapon();

	// Firing Gun
	void Fire
	    (const double &ctime,
	     YsVec3 &pos,
	     YsAtt3 &att,
	     const double &v,
	     const double &l,
	     int destruction,
	     class FsExistence *owner,
	     FSWEAPON_CREDIT_OWNER creditOwnerIn);

	// Firing Missile
	void Fire
	    (const double &ctime,
	     FSWEAPONTYPE missileType,
	     const YsVec3 &pos,const YsAtt3 &att,
	     const double &v,const double &vmax,const double &l,const double &mobility,const double &radar,
	     int destruction,
	     class FsExistence *owner,
	     FSWEAPON_CREDIT_OWNER creditOwnerIn,
	     class FsExistence *target,FsWeaponSmokeTrail *trail);

	// Flare
	void DispenseFlare
	    (const double &ctime,
	     const YsVec3 &pos,const YsAtt3 &att,
	     const double &v,const double &vMax,const double &l,
	     class FsExistence *owner,
	     FSWEAPON_CREDIT_OWNER creditOwnerIn,
	     FsWeaponSmokeTrail *trail);

	// Debris
	void ThrowDebris(const double &ctime,const YsVec3 &pos,const YsVec3 &vec,const double &l);


	void Move(const double &dt,const double &cTime,const class FsWeather &weather,const FsWeapon *flareList);
	YSBOOL IsOwnerStillHaveTarget(void);
	void HitGround
	    (class FsWeaponHolder *callback,
	     const double &ctime,
	     const class FsField &fld,
	     class FsExplosionHolder *explosion,
	     class FsSimulation *sim,
	     YsList <FsKillCredit> *&killCredit);
	YSBOOL HitObject
	    (class FsWeaponHolder *callback,
	     const double &ctime,
	     class FsExistence &obj,
	     class FsExplosionHolder *explosion,
	     class FsSimulation *sim,
	     YsList <FsKillCredit> *&killCredit);
	void ExplodeBomb
	    (class FsWeaponHolder *callback,
	     const double &ctime,
	     const YsVec3 &pos,
	     const double &rad,
	     class FsExplosionHolder *explosion,
	     class FsSimulation *sim,
	     YsList <FsKillCredit> *&killCredit);
	void ExplodeBombInWater
	    (class FsWeaponHolder *callback,
	     const double &ctime,
	     const YsVec3 &pos,
	     const double &rad,
	     class FsExplosionHolder *explosion,
	     class FsSimulation *sim,
	     YsList <FsKillCredit> *&killCredit);
	void Draw(
	    YSBOOL coarse,const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,
	    YSBOOL transparency,FSSMOKETYPE smk,const double &cTime,unsigned int drawFlag) const;

	void AddToParticleManager(class YsGLParticleManager &partMan,const double cTime) const;

protected:
	YSRESULT AddKillCredit(YsList <FsKillCredit> *&killCredit,FsExistence *whoIsKilled,const double &when) const;
};



// Declaration /////////////////////////////////////////////
class FsWeaponHolder
{
protected:
	FsSimulation *sim;
	YsArray <YsVec3,64> bulletCalibrator;

public:
	FsWeaponHolder(FsSimulation *simPtr);
	~FsWeaponHolder();

	static YSRESULT LoadMissilePattern(void);
	static void FreeMissilePattern(void);

	enum
	{
		NumBulletBuffer=1024,
		NumSmokeTrailBuffer=32
	};

	YSRESULT Save(FILE *fp,class FsSimulation *sim,int wpnPosPrecision,int wpnAttPrecision);
	YSRESULT Load(FILE *fp,class FsSimulation *sim);

	void Clear(void);
	void MoveToActiveList(FsWeapon *wep);
	void MoveToFreeList(FsWeapon *wep);


	void ClearBulletCalibrator(void);
	void CalculateBulletCalibrator(const FsExistence *target);


	YSBOOL GiveDamage(YSBOOL &killed,FsExistence &obj,int power,enum FSDIEDOF diedOf,FsWeapon &wpn);

	YSRESULT RipOffEarlyPartOfRecord(void);

	YSRESULT DeleteRecordForResumeFlight(class FsAirplane *shotBy,const double &startTime);

	YSRESULT RefreshOrdinanceByWeaponRecord(const double &currentTime);

	// Firing Gun
	int Fire
	    (const double &ctime,
	     YsVec3 &pos,
	     YsAtt3 &att,
	     double v,
	     double l,
	     int destructivePower,
	     class FsExistence *owner,
	     YSBOOL recordIt,YSBOOL transmit);

	// Firing Missile
	int Fire
	    (const double &ctime,
	     FSWEAPONTYPE missileType,
	     YsVec3 &pos,YsAtt3 &att,
	     double v,double vmax,double l,double mobility,double radar,
	     int destructivePower,
	     class FsExistence *owner,unsigned int targetKey,
	     YSBOOL recordIt,YSBOOL transmit);

	// Bomb away!
	int Bomb
	   (const double &ctime,
	    FSWEAPONTYPE bombType,
	    const YsVec3 &pos,const YsAtt3 &att,const YsVec3 &iniVelocity,
	    const double &vMax,
	    int destructivePower,
	    class FsExistence *owner,
	    YSBOOL recordIt,YSBOOL transmit);

	// Dispense Flare
	int DispenseFlare
	   (const double &ctime,
	    const YsVec3 &pos,const YsVec3 &vel,const double &vMax,const double &l,
	    class FsExistence *owner,YSBOOL recordIt,YSBOOL transmit);

	// Debris
	int ThrowDebris(const double &ctime,const YsVec3 &pos,const YsVec3 &vec,const double &l);
	void ThrowRandomDebris(const double &ctime,const YsVec3 &pos,const YsAtt3 &att,const double &l);
	void ThrowMultiDebris(int n,const double &ctime,const YsVec3 &pos,const YsAtt3 &att,const double &l);

	YSBOOL IsLockedOn(const FsExistence *ex) const;
	YSBOOL IsLockedOn(FSWEAPONTYPE &wpnType,YsVec3 &wpnPos,const FsExistence *ex) const;
	FsWeapon* GetLockedOn(const FsExistence* ex) const;

	YSRESULT FindFirstMissilePositionThatIsReallyGuided(YsVec3 &vec,YsAtt3 &att) const;
	YSRESULT FindOldestMissilePosition(YsVec3 &vec,YsAtt3 &att,const FsExistence *fired) const;
	YSRESULT FindNewestMissilePosition(YsVec3 &vec,YsAtt3 &att,const FsExistence *fired) const;

	const FsWeapon *FindNextActiveWeapon(const FsWeapon *wpn) const;
	const FsWeapon *GetWeapon(int id) const;
	void ObjectIsDeleted(FsExistence *obj) const;

	void Move(const double &dt,const double &cTime,const class FsWeather &weather);
	void HitGround(
	    const double &ctime,const class FsField &field,class FsExplosionHolder *xp,class FsSimulation *sim);
	void HitObject(
	    const double &ctime,FsExplosionHolder *explo,class FsSimulation *sim,const double &tallestGroundObjectHeight);

	void AddToParticleManager(class YsGLParticleManager &partMan,const double cTime) const;

	void BeginDraw(void) const;
	void Draw(
	    YSBOOL coarse,const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,
	    YSBOOL transparency,FSSMOKETYPE smk,const double &cTime,unsigned int drawFlag) const;
	void EndDraw(void) const;

	void DrawGunCalibrator(void) const;

	void CollectRecord(void);
	void PlayRecord(const double &t1,const double &t2);
	void LaunchRecord(const FsWeaponRecord &rec,const double &t,YSBOOL recordIt,YSBOOL transmit);

	FsWeapon buf[NumBulletBuffer];
	FsWeaponSmokeTrail trl[NumSmokeTrailBuffer];

	FsWeapon *activeList,*freeList;

	FsRecord <FsWeaponRecord> *toPlay;
	FsRecord <FsWeaponRecord> *toSave;

	YsList <FsKillCredit> *killCredit;

protected:
	FsExistence *FindObjectByAxxGxxN(const char identifier[],const class FsSimulation *sim);


// For network play
protected:
	class FsSocketServer *netServer;
	class FsSocketClient *netClient;
public:
	void SetNetServer(class FsSocketServer *svr);
	void SetNetClient(class FsSocketClient *cli);
};


unsigned FsEncodeWeaponRecord(unsigned char dat[],FsSimulation *sim,const FsWeaponRecord &rec);
YSRESULT FsDecodeWeaponRecord
    (FsWeaponRecord &rec,
     int &firedBy,YSBOOL &firedByAirplane,int &firedAt,YSBOOL &firedAtAirplane,
     unsigned char dat[],FsSimulation *sim);


int FsGetDefaultWeaponLoadingUnit(FSWEAPONTYPE wpnType);
int FsGetDefaultSubUnitPerLoadingUnit(FSWEAPONTYPE wpnType);
FSWEAPONTYPE FsGetWeaponTypeByString(const char str[]);
const char *FsGetWeaponString(FSWEAPONTYPE wpnType);

/* } */
#endif
