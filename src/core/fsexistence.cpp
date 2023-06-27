#include <ysclass.h>
#include "fs.h"

#include "fsinstpanel.h"

#include "fsrecord.h"

#include "fsatc.h"

#include "fsautodrive.h"

////////////////////////////////////////////////////////////

static YSRESULT YsEstimateVelocityVectorByQuadraticCurveFitting
    (YsVec3 &unitVelocityVector,
     const double &t,
     const double &t0,const YsVec3 &p0,const double &t1,const YsVec3 &p1,const double &t2,const YsVec3 &p2)
{
	int i;
	double dy[3];
	YsMatrixTemplate <3,3> mat;
	YsMatrixTemplate <3,1> sol,rhs;

	// y=att+bt+c
	//   ->  t0*t0*a  +t0*b  +c   =  y0
	//       t1*t1*a  +t1*b  +c   =  y1
	//       t2*t2*a  +t2*b  +c   =  y2
	mat.Set(1,1,t0*t0);
	mat.Set(1,2,t0);
	mat.Set(1,3,1.0);

	mat.Set(2,1,t1*t1);
	mat.Set(2,2,t1);
	mat.Set(2,3,1.0);

	mat.Set(3,1,t2*t2);
	mat.Set(3,2,t2);
	mat.Set(3,3,1.0);

	if(mat.Invert()==YSOK)
	{
		for(i=0; i<3; i++)
		{
			rhs.Set(1,1,p0[i]);
			rhs.Set(2,1,p1[i]);
			rhs.Set(3,1,p2[i]);

			sol=mat*rhs;
			// y'=2at+b  , y'(t)=2*a*t+b
			dy[i]=2.0*sol.v(1,1)*t+sol.v(2,1);
		}
		unitVelocityVector.Set(dy);
		return unitVelocityVector.Normalize();
	}
	return YSERR;
}



// Implementation //////////////////////////////////////////
unsigned FsExistence::searchKeySeed=0x10001;  // Note: Must not be zero.

FsExistence::FsExistence()
{
	vis=NULL;
	lod=NULL;
	cockpit=NULL;

	for(int i=0; i<FSWEAPON_NUMWEAPONTYPE; i++)
	{
		weaponShapeOverrideStatic[i].CleanUp();
		weaponShapeOverrideFlying[i].CleanUp();
	}

	CleanUp();
	Initialize();
}

FsExistence::~FsExistence()
{
	CleanUp();
}

/* static */ const char *FsExistence::TypeToStr(FSEXISTENCETYPE t)
{
	if(FSEX_AIRPLANE==t)
	{
		return "AIR";
	}
	else if(FSEX_GROUND==t)
	{
		return "GND";
	}
	else
	{
		return "NULL";
	}
}

/* static */ FSEXISTENCETYPE FsExistence::StrToType(const char str[])
{
	if(0==strncmp(str,"AIR",3))
	{
		return FSEX_AIRPLANE;
	}
	else if(0==strncmp(str,"GND",3))
	{
		return FSEX_GROUND;
	}
	return FSEX_NULL;
}

void FsExistence::CopyFrom(const FsExistence &from)
{
	*this=from;
}

/* static */YSHASHKEY FsExistence::GetSearchKey(const FsExistence *obj)
{
	if(NULL!=obj)
	{
		return obj->searchKey;
	}
	else
	{
		return FsNullSearchKey;
	}
}

void FsExistence::SetHomeBaseName(FsSimInfo::BASE_TYPE baseType,const YsString &baseName)
{
	homeBase.SetType(baseType);
	homeBase.SetName(baseName);
}

FsSimInfo::BASE_TYPE FsExistence::GetHomeBaseType(void) const
{
	return homeBase.GetType();
}

const YsString &FsExistence::GetHomeBaseName(void) const
{
	return homeBase.GetName();
}

YSHASHKEY FsExistence::SearchKey(void) const
{
	return searchKey;
}

YSBOOL FsExistence::IsAirplane(void) const
{
	return (GetType()==FSEX_AIRPLANE ? YSTRUE : YSFALSE);
}

const char *FsExistence::GetIdentifier(void) const
{
	return CommonProp().GetIdentifier();
}

const char *FsExistence::GetName(void) const
{
	return name;
}

FSIFF FsExistence::GetIff(void) const
{
	return iff;
}
void FsExistence::SetIff(FSIFF iff)
{
	this->iff=iff;
}

const char *FsExistence::GetStartPos(void) const
{
	return _startPosition;
}

const YsVec3 &FsExistence::GetPosition(void) const
{
	return CommonProp().GetPosition();
}

YsVec3 FsExistence::GetCockpitPosition(void) const
{
	return CommonProp().GetCockpitPosition();
}

const YsVec3 &FsExistence::GetLookAtOffset(void) const
{
	return CommonProp().GetLookAtOffset();
}

const YsAtt3 &FsExistence::GetAttitude(void) const
{
	return CommonProp().GetAttitude();
}

const YsMatrix4x4 &FsExistence::GetMatrix(void) const
{
	return CommonProp().GetMatrix();
}

const YsMatrix4x4 &FsExistence::GetInverseMatrix(void) const
{
	return CommonProp().GetInverseMatrix();
}

int FsExistence::GetNumAdditionalView(void) const
{
	return CommonProp().GetNumAdditionalView();
}

const class FsAdditionalViewpoint *FsExistence::GetAdditionalView(int id) const
{
	return CommonProp().GetAdditionalView(id);
}

double FsExistence::GetApproximatedCollideRadius(void) const
{
	return CommonProp().GetOutsideRadius();
}

YSBOOL FsExistence::IsAlive(void) const
{
	return CommonProp().IsAlive();
}

YSBOOL FsExistence::IsActive(void) const
{
	return CommonProp().IsActive();
}

YSRESULT FsExistence::AddReloadCommand(const char str[])
{
	reloadCommand.Append(str);
	return YSOK;
}

const YsArray <YsString> FsExistence::GetReloadCommand(void) const
{
	return reloadCommand;
}

const YsSceneryRectRegion *FsExistence::IsInsideRunway(void) const
{
	if(YSTRUE==rectRgnCached)
	{
		for(YSSIZE_T rgnIdx=0; rgnIdx<rectRgnCache.GetN(); ++rgnIdx)
		{
			if(FS_RGNID_RUNWAY==rectRgnCache[rgnIdx]->GetId())
			{
				return rectRgnCache[rgnIdx];
			}
		}
	}
	return NULL;
}

void FsExistence::Initialize(void)
{
	rectRgnCached=YSFALSE;
	rectRgnCache.Clear();

	iff=FS_IFF0;
	_startPosition.Set("");

	name.Set("");
	ysfId=0;

	vis=NULL;
	lod=NULL;
	coll.CleanUp();  // <- 2003/12/11 Why was I not doing it?
	coll.SetMatrix(YsIdentity4x4());  // 2003/12/19 Preventive

	collSrc.CleanUp();  // <- 2003/12/11 Why was I not doing it?
	collSrc.SetMatrix(YsIdentity4x4());  // 2003/12/19 Preventive

	collBbx[0]=YsOrigin();
	collBbx[1]=YsOrigin();

	collCen=YsOrigin();
	collRadius=0.0;

	refTime1=0.0;
	refTime2=0.0;

	isNetSubstitute=YSFALSE;
	actualIdName.Set("");

	netType=FSNET_LOCAL;

	netAirTarget=NULL;
	netGndTarget=NULL;
	netAlive=YSTRUE;
	netDamageTolerance=0;

	prevPos=YsOrigin();
	prevDt=0.1;

	primaryTarget=YSFALSE;

	motionPath=NULL;
	motionPathPnt.Clear();
	motionPathIsLoop=YSFALSE;
	useMotionPathOffset=YSFALSE;
	motionPathOffset=YsOrigin();
	motionPathIndex=0;

	searchKey=searchKeySeed++;

	elevation=0.0;
	terrainNom=YsYVec();

	fsLtcCache[0].Set(-1,-1);
	fsLtcCache[1].Set(-1,-1);

	bouncedLastTime=YSFALSE;
}

void FsExistence::CleanUp(void)
{
	coll.CleanUp();
	coll.SetMatrix(YsMatrix4x4());
	motionPath=NULL;

	initialStateCaptured=YSFALSE;
	initialStateCommand.Clear();
}

void FsExistence::SetSearchKeyForNetworkSynchronizationPurpose(unsigned key)
{
	searchKey=key;
	if(searchKeySeed<=key)
	{
		searchKeySeed=key+1;
	}
}

double FsExistence::GetRadiusFromCollision(void) const
{
	return collRadius;
}

const FsVisualSrf &FsExistence::TransformedCollisionShell(void) const
{
	return coll;
}

const FsVisualSrf &FsExistence::UntransformedCollisionShell(void) const
{
	return collSrc;
}

const YsVec3 *FsExistence::GetCollisionShellBbx(void) const
{
	return collBbx;
}

const YsVec3 &FsExistence::GetCollisionShellCenter(void) const
{
	return collCen;
}

void FsExistence::SetCollisionShell(const FsVisualSrf &src)
{
	coll.SetMatrix(YsIdentity4x4());  // Preventive
	coll=src;
	coll.Encache();

	collSrc.SetMatrix(YsIdentity4x4());  // Preventive
	collSrc=src;
	collSrc.Encache();

	src.GetBoundingBox(collBbx[0],collBbx[1]);
	collCen=(collBbx[0]+collBbx[1])/2.0;
	collRadius=(collBbx[1]-collBbx[0]).GetLength()/2.0;
}

void FsExistence::SetTransformationToCollisionShell(const YsMatrix4x4 &mat)
{
	coll.SetMatrix(mat);
}

void FsExistence::ClearCollisionShell(void)
{
	coll.SetMatrix(YsIdentity4x4());
	coll.CleanUp();
	collSrc.SetMatrix(YsIdentity4x4());
	coll.CleanUp();
}

YSBOOL FsExistence::MayCollideWith(const FsExistence &test,const double clearance) const
{
	return MayCollideWith(GetInverseMatrix(),test,test.GetMatrix(),clearance);
}

YSBOOL FsExistence::MayCollideWith(const YsMatrix4x4 &ownInverseMat,const FsExistence &test,const YsMatrix4x4 &testMat,const double clearance) const
{
	YsVec3 corner[8];
	corner[0].Set(test.collBbx[0].x(),test.collBbx[0].y(),test.collBbx[0].z());
	corner[1].Set(test.collBbx[1].x(),test.collBbx[0].y(),test.collBbx[0].z());
	corner[2].Set(test.collBbx[0].x(),test.collBbx[1].y(),test.collBbx[0].z());
	corner[3].Set(test.collBbx[1].x(),test.collBbx[1].y(),test.collBbx[0].z());
	corner[4].Set(test.collBbx[0].x(),test.collBbx[0].y(),test.collBbx[1].z());
	corner[5].Set(test.collBbx[1].x(),test.collBbx[0].y(),test.collBbx[1].z());
	corner[6].Set(test.collBbx[0].x(),test.collBbx[1].y(),test.collBbx[1].z());
	corner[7].Set(test.collBbx[1].x(),test.collBbx[1].y(),test.collBbx[1].z());

	YSBOOL allAbove=YSTRUE,allBelow=YSTRUE,allLeft=YSTRUE,allRight=YSTRUE,allAhead=YSTRUE,allBehind=YSTRUE;

	const YsMatrix4x4 &thisMat=ownInverseMat;
	YsMatrix4x4 tfm=thisMat*testMat;

	const double &xMin=collBbx[0].x()-clearance,&xMax=collBbx[1].x()+clearance;
	const double &yMin=collBbx[0].y()-clearance,&yMax=collBbx[1].y()+clearance;
	const double &zMin=collBbx[0].z()-clearance,&zMax=collBbx[1].z()+clearance;

	for(int i=0; i<8; i++)
	{
		YsVec3 tst;
		tfm.Mul(tst,corner[i],1.0);

		if(tst.x()<=xMax)
		{
			allRight=YSFALSE;
		}
		if(xMin<=tst.x())
		{
			allLeft=YSFALSE;
		}

		if(tst.y()<=yMax)
		{
			allAbove=YSFALSE;
		}
		if(yMin<=tst.y())
		{
			allBelow=YSFALSE;
		}

		if(tst.z()<=zMax)
		{
			allBehind=YSFALSE;
		}
		if(zMin<=tst.z())
		{
			allAhead=YSFALSE;
		}
	}

	if(YSTRUE==allAbove || 
	   YSTRUE==allBelow || 
	   YSTRUE==allLeft || 
	   YSTRUE==allRight || 
	   YSTRUE==allBehind || 
	   YSTRUE==allAhead)
	{
		return YSFALSE;
	}

	return YSTRUE;
}

/*! Added 2018/01/06.
*/
void FsExistence::ResetDnmState(void)
{
	vis.ClearState();
	vis.SetStateOfAllPart(0);
	lod.ClearState();
	lod.SetStateOfAllPart(0);
}

YSBOOL FsExistence::TestTailStrike(YsVec3 &collisionPosInLocalCoordinate) const
{
	const auto *collPtr=&TransformedCollisionShell();

	if(YsYVec()==terrainNom)
	{
		for(auto vtHd : collPtr->AllVertex())
		{
			YsVec3 pos;
			collPtr->GetVertexPosition(pos,vtHd);

			if(pos.y()<elevation-0.2)   // 0.2 (^_^;)
			{
				const YsShellVertex *vtx=collPtr->GetVertex(vtHd);
				collisionPosInLocalCoordinate=vtx->GetPosition();
				return YSTRUE;
			}
		}
	}
	else
	{
		// Plane equation (o-p)*n=0
		//   o*n-p*n=0
		//   px*nx+py*ny+pz*nz=o*n
		//   py=(o*n-px*nx-pz*nz)/ny

		if(YSTRUE!=YsEqual(terrainNom.y(),0.0))
		{
			const YsVec3 &o=terrainOrg;
			const YsVec3 &n=terrainNom;
			const double on=o*n;
			const double nx=terrainNom.x();
			const double ny=terrainNom.y();
			const double nz=terrainNom.z();

			for(auto vtHd : collPtr->AllVertex())
			{
				YsVec3 pos;
				collPtr->GetVertexPosition(pos,vtHd);

				const double pxnx=pos.x()*nx;
				const double pznz=pos.z()*nz;
				const double py=(on-pxnx-pznz)/ny;

				if(pos.y()<py-0.2)   // 0.2 (^_^;)
				{
					const YsShellVertex *vtx=collPtr->GetVertex(vtHd);
					collisionPosInLocalCoordinate=vtx->GetPosition();
					return YSTRUE;
				}
			}
		}
	}

	return YSFALSE;
}

void FsExistence::DrawApproximatedShadow
	(const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm) const
{
	collSrc.DrawShadow(viewTfm,projTfm,GetPosition(),GetAttitude(),projPlnTfm);
	// Don't use 'coll' here.  Matrix is changed for collision detection.
}

const double FsExistence::GetAGL(void) const
// Never change it to "const double &".  Visual C++ cannot handle it.
{
	return GetPosition().y()-elevation;
}

const double FsExistence::GetFieldElevation(void) const
{
	return elevation;
}

void FsExistence::SetBouncedLastTimeFlag(void)
{
	bouncedLastTime=YSTRUE;
}

void FsExistence::ClearBouncedLastTimeFlag(void)
{
	bouncedLastTime=YSFALSE;
}

YSBOOL FsExistence::BouncedLastTime(void) const
{
	return bouncedLastTime;
}

void FsExistence::SetMotionPath(const YsSceneryPointSet *mpath)
{
	this->motionPath=mpath;
	this->motionPathIsLoop=mpath->IsLoop();

	this->motionPathPnt.Set(motionPath->GetNumPoint(),NULL);
	for(int i=0; i<motionPath->GetNumPoint(); i++)
	{
		YsVec3 wayPoint;
		const YsSceneryItem *itm;

		wayPoint=motionPath->GetPointArray()[i];
		itm=motionPath;
		while(itm!=NULL)
		{
			itm->GetAttitude().Mul(wayPoint,wayPoint);
			wayPoint+=itm->GetPosition();
			itm=itm->GetOwner();
		}

		this->motionPathPnt[i]=wayPoint;
	}
}

YSBOOL FsExistence::IsInDeadLockFreeAirport(void) const
{
	if(YSTRUE==rectRgnCached)
	{
		for(YSSIZE_T idx=0; idx<rectRgnCache.GetN(); ++idx)
		{
			if(FS_RGNID_AIRPORT_AREA==rectRgnCache[idx]->GetId() &&
			   YsSceneryRectRegion::SUBCLASS_DEADLOCK_FREE_AIRPORT==rectRgnCache[idx]->GetSubClassType())
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

////////////////////////////////////////////////////////////

template <class T>
static int FsRerecordByNewInterval(FsRecord <T> *&newRec,FsRecord <T> *oldRec,const double &itvl)
{
	int i,n,nNew;
	double t,nextRecordTime;
	T *elm;

	n=oldRec->GetNumRecord();
	if(n>0)
	{
		newRec=new FsRecord <T>;
		oldRec->GetElement(t,0);
		nextRecordTime=t-YsTolerance;

		nNew=0;
		for(i=0; i<n-1; i++)
		{
			elm=oldRec->GetElement(t,i);
			if(t>=nextRecordTime)
			{
				newRec->AddElement(*elm,t);
				nNew++;
				nextRecordTime=t+itvl;
			}
		}

		// 2001/06/19 >>
		elm=oldRec->GetElement(t,n-1);
		newRec->AddElement(*elm,t);
		nNew++;
		// 2001/06/19 <<

		return n-nNew;
	}
	else
	{
		return 0;
	}
}

template <class T>
void FsAdjustPrecisionOfFlightRecord(FsRecord <T> *rec,const double &precPos,const double &precAng)
{
	int i,n;
	T *elm;
	double t;
	n=rec->GetNumRecord();
	for(i=0; i<n; i++)
	{
		int xx,yy,zz,hh,pp,bb;
		elm=rec->GetElement(t,i);

		xx=int(elm->pos.x()/precPos);
		yy=int(elm->pos.y()/precPos);
		zz=int(elm->pos.z()/precPos);
		hh=int(elm->h/precAng);
		pp=int(elm->p/precAng);
		bb=int(elm->b/precAng);

		elm->pos.Set((double(xx)*precPos),(double(yy)*precPos),(double(zz)*precPos));
		elm->h=float(double(hh)*precAng);
		elm->p=float(double(pp)*precAng);
		elm->b=float(double(bb)*precAng);
	}
}



// Implementation //////////////////////////////////////////
FsAirplane::FsAirplane()
{
	instPanel=NULL;
	defDamageTolerance=1;
	Initialize();
}

FsAirplane::FsAirplane(const FsAirplane &from)
{
	instPanel=NULL;
	CopyFrom(from);
}

FsAirplane::~FsAirplane()
{
	CleanUp();
}

const FsAirplane &FsAirplane::operator=(const FsAirplane &from)
{
	CopyFrom(from);
	return *this;
}

void FsAirplane::CopyFrom(const FsAirplane &from)
{
	CleanUp();
	FsExistence::CopyFrom(from);

	airTrafficInfo=from.airTrafficInfo;

	gLimit=from.gLimit;
	airFlag=from.airFlag;
	landWhenLowFuelThr=from.landWhenLowFuelThr;

	cmdLog=from.cmdLog;
	prop=from.prop;

	// Must not be copied >>
	// thisInTheList;
	// instPanel;
	// curAutoPilotIdx;
	// autoPilotList;
	// Must not be copied <<

	// Doesn't matter >>
	// netPrevState;
	// netNextState;
	// Doesn't matter <<

	curAutoPilotIdx=0;
	autoPilotList.Clear(); // Don't share it!
	instPanel=NULL;
}

FSEXISTENCETYPE FsAirplane::GetType(void) const
{
	return FSEX_AIRPLANE;
}

void FsAirplane::Initialize(void)
{
	FsExistence::Initialize();

	rec=NULL;
	isPlayingRecord=YSFALSE;

	curAutoPilotIdx=0;
	autoPilotList.Clear();

	thisInTheList=NULL;

	netPrevState.tLocal=-1.0;
	netPrevState.tRemote=-1.0;
	netNextState.tLocal=-1.0;
	netNextState.tRemote=-1.0;

	if(instPanel!=NULL)
	{
		delete instPanel;
		instPanel=NULL;
	}

	gLimit=9.0;
	airFlag=0;

	landWhenLowFuelThr=0.0;

	airTrafficInfo.CleanUp();
}

void FsAirplane::CleanUp(void)
{
	if(rec!=NULL)
	{
		// printf("Number of flight records at last : %d\n",rec->GetNumRecord());
		delete rec;
		rec=NULL;
	}

	ClearAutopilot();

	if(instPanel!=NULL)
	{
		delete instPanel;
		instPanel=NULL;
	}

	airTrafficInfo.CleanUp();

	FsExistence::CleanUp();
}

void FsAirplane::MakeVaporVertexArray(class YsGLVertexBuffer &vtxBuf,class YsGLColorBuffer &colBuf,double currentTime,double remainTime,int step) const
{
	vtxBuf.CleanUp();
	colBuf.CleanUp();

	if(nullptr!=rec)
	{
		YSSIZE_T idx;
		double t,t0,t1;

		// Catch the index
		t0=YsGreater(currentTime-remainTime,0.0);
		t1=currentTime;
		t=currentTime;
		while(t>t0)
		{
			if(rec->GetIndexByTime(idx,t)==YSOK)
			{
				break;
			}
			t-=0.05;
		}

		idx++; // Trick

		// Draw smoke
		FsFlightRecord *r0,*r1;
		FsFlightRecord fakeRecord;
		YsVec3 vap00,vap01,vap10,vap11,left,right;
		double tm0,tm1;
		int d;
		YSBOOL drewPrevious;

		r0=rec->GetLastElement(tm0);
		if(r0!=NULL && t0<tm0)
		{
			r0=NULL;
			r1=NULL;
			tm0=0.0;
			tm1=0.0;
			vap00.Set(0.0,0.0,0.0);
			vap01.Set(0.0,0.0,0.0);
			vap10.Set(0.0,0.0,0.0);
			vap11.Set(0.0,0.0,0.0);
			prop.GetVaporPosition(right);
			left.Set(-right.x(),right.y(),right.z());

			d=1;
			drewPrevious=YSFALSE;

			float alpha=0.6f;
			float prevAlpha=0.6f;

			alpha=1.0f;
			while(t>t0 && idx>0)
			{
				r1=r0;
				tm1=tm0;
				prevAlpha=alpha;

				r0=rec->GetElement(tm0,idx);

				if(isPlayingRecord!=YSTRUE &&
				   prop.IsTrailingVapor()==YSTRUE &&
				   r1==NULL && r0!=NULL)
				{
					const YsVec3 *pos;
					const YsAtt3 *att;
					pos=&GetPosition();
					att=&GetAttitude();
					fakeRecord.smoke=1;
					fakeRecord.pos=*pos;
					fakeRecord.h=float(att->h());
					fakeRecord.p=float(att->p());
					fakeRecord.b=float(att->b());
					r1=&fakeRecord;
					tm1=currentTime;
				}

				if(r0!=NULL && r1!=NULL && (r0->vapor!=0 || r1->vapor!=0))
				{
					YsVec3 r,l;
					YsAtt3 a;
					if(drewPrevious==YSTRUE)
					{
						vap10=vap00;
						vap11=vap01;
					}
					else
					{
						a.Set(r1->h,r1->p,r1->b);

						a.Mul(r,right); //r=a.GetMatrix()*right;
						a.Mul(l,left);  //l=a.GetMatrix()*left;

						vap10=r1->pos+r; // (r1->x+r.x(),r1->y+r.y(),r1->z+r.z())
						vap11=r1->pos+l; // (r1->x+l.x(),r1->y+l.y(),r1->z+l.z())
					}

					a.Set(r0->h,r0->p,r0->b);

					a.Mul(r,right); //r=a.GetMatrix()*right;
					a.Mul(l,left);  //l=a.GetMatrix()*left;

					vap00=r0->pos+r; // (r0->x+r.x(),r0->y+r.y(),r0->z+r.z())
					vap01=r0->pos+l; // (r0->x+l.x(),r0->y+l.y(),r0->z+l.z())

					if(tm0<=currentTime && currentTime<tm1)
					{
						vap10=vap00+(vap10-vap00)*(currentTime-tm0)/(tm1-tm0);
						vap11=vap01+(vap11-vap01)*(currentTime-tm0)/(tm1-tm0);
					}

					alpha=(float)YsSqr((t-t0)/(t1-t0));

					// Transparency: alpha=1.0 at t=t1, alpha=0.0 at t=t0
					colBuf.Add<float>(1.0f,1.0f,1.0f,alpha);
					vtxBuf.Add(vap00);
					colBuf.Add<float>(1.0f,1.0f,1.0f,prevAlpha);
					vtxBuf.Add(vap10);

					colBuf.Add<float>(1.0f,1.0f,1.0f,alpha);
					vtxBuf.Add(vap01);
					colBuf.Add<float>(1.0f,1.0f,1.0f,prevAlpha);
					vtxBuf.Add(vap11);

					drewPrevious=YSTRUE;
				}
				else
				{
					drewPrevious=YSFALSE;
				}
				idx-=d;

				if(r0!=NULL)
				{
					t=tm0;
				}

				if(d==1 && currentTime-tm0>0.5)
				{
					idx=idx-(idx%step);
					d=step;
				}
			}
		}
		// Done
	}
}

void FsAirplane::MakeSmokeVertexArray(class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,double currentTime,double remainTime,FSSMOKETYPE smk,int d) const
{
	vtxBuf.CleanUp();
	nomBuf.CleanUp();
	colBuf.CleanUp();
	for(YSSIZE_T i=0; i<Prop().GetNumSmokeGenerator(); i++)
	{
		AddSingleSmokeVertexArray(vtxBuf,nomBuf,colBuf,i,currentTime,remainTime,smk,d);
	}
}

void FsAirplane::AddSingleSmokeVertexArray(
    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
    int smkId,double currentTime,double remainTime,FSSMOKETYPE smk,int step) const
{
	if(smk==FSSMKNULL)
	{
		return;
	}

	if(rec!=NULL)
	{
		YSSIZE_T idx=0;

		const YsColor smkCol=Prop().GetSmokeColor(smkId);

		// Catch the index
		double t0=YsGreater(currentTime-remainTime,0.0);
		double t=currentTime;
		while(t>t0)
		{
			if(rec->GetIndexByTime(idx,t)==YSOK)
			{
				break;
			}
			t-=0.05;
		}

		idx++; // Trick

		// Draw smoke
		FsFlightRecord *r0,*r1;
		FsFlightRecord fakeRecord;
		YsVec3 smkp0,smkp1,smkgen;
		YsAtt3 smka0,smka1;
		double tm0,tm1;
		int d,smallStepRemain;
		YSBOOL drewPrevious;

		r0=rec->GetLastElement(tm0);
		if(r0!=NULL && t0<tm0)  // Smoke may exist between time t0 and t1
		{
			r0=NULL;
			r1=NULL;
			tm0=0.0;
			tm1=0.0;
			smkp0.Set(0.0,0.0,0.0);
			smkp1.Set(0.0,0.0,0.0);
			smka0.Set(0.0,0.0,0.0);
			smka1.Set(0.0,0.0,0.0);
			prop.GetSmokeGeneratorPosition(smkgen,smkId);

			d=1;
			smallStepRemain=step*2;
			drewPrevious=YSFALSE;

			while(t>t0 && idx>0)
			{
				r1=r0;
				tm1=tm0;

				r0=rec->GetElement(tm0,idx);

				if(isPlayingRecord!=YSTRUE &&
				   prop.IsTrailingSmoke(smkId)==YSTRUE &&
				   r1==NULL && r0!=NULL)
				{
					const YsVec3 *pos;
					const YsAtt3 *att;
					pos=&GetPosition();
					att=&GetAttitude();
					fakeRecord.smoke=1;
					fakeRecord.pos=*pos;
					fakeRecord.h=float(att->h());
					fakeRecord.p=float(att->p());
					fakeRecord.b=float(att->b());
					fakeRecord.smoke|=(1<<smkId);
					r1=&fakeRecord;
					tm1=currentTime;
				}

				if(r0!=NULL && r1!=NULL && ((r0->smoke&(1<<smkId))!=0 || (r1->smoke&(1<<smkId))!=0))
				{
					if(drewPrevious==YSTRUE)
					{
						smkp1=smkp0;
						smka1=smka0;
					}
					else
					{
						smka1.Set(r1->h,r1->p,r1->b);
						smka1.Mul(smkp1,smkgen);  //smkp1=smka1.GetMatrix()*smkgen;
						smkp1=smkp1+r1->pos; // (smkp1.x()+r1->x,smkp1.y()+r1->y,smkp1.z()+r1->z);
					}

					smka0.Set(r0->h,r0->p,r0->b);
					smka0.Mul(smkp0,smkgen); //smkp0=smka0.GetMatrix()*smkgen;
					smkp0=smkp0+r0->pos; // (smkp0.x()+r0->x,smkp0.y()+r0->y,smkp0.z()+r0->z);

					if(tm0<=currentTime && currentTime<tm1)
					{
						smkp1=smkp0+(smkp1-smkp0)*(currentTime-tm0)/(tm1-tm0);
					}

					double ra0,ra1,alpha0,alpha1;
					if(tm0<currentTime)
					{
						ra0=YsBound(sqrt(currentTime-tm0)*5.0,0.0,5.0);
					}
					else
					{
						ra0=0.0;
					}
					if(tm1<currentTime)
					{
						ra1=YsBound(sqrt(currentTime-tm1)*5.0,0.0,5.0);
					}
					else
					{
						ra1=0.0;
					}
					alpha0=0.8-0.8*(currentTime-tm0)/remainTime;
					alpha1=0.8-0.8*(currentTime-tm1)/remainTime;
					switch(smk)
					{
					default:
						break;
					case FSSMKNOODLE:
					case FSSMKTOWEL:
						AddTowelSmoke(vtxBuf,nomBuf,colBuf,smkId,smkp0,smka0,ra0,smkp1,smka1,ra1,alpha0,alpha1);
						break;
					case FSSMKSOLID:
						if((r0->smoke&(1<<smkId))!=0 && (r1->smoke&(1<<smkId))!=0)
						{
							AddSolidSmoke(vtxBuf,nomBuf,colBuf,smkId,smkp0,smka0,ra0,smkp1,smka1,ra1,alpha0,alpha1);
						}
						else if((r0->smoke&(1<<smkId))!=0)
						{
							AddSolidSmokeLid(vtxBuf,nomBuf,colBuf,smkId,smkp0,smka0,ra0,alpha0);
						}
						else if((r1->smoke&(1<<smkId))!=0)
						{
							AddSolidSmokeLid(vtxBuf,nomBuf,colBuf,smkId,smkp1,smka1,ra1,alpha1);
						}
						break;
					}
					drewPrevious=YSTRUE;
				}
				else
				{
					drewPrevious=YSFALSE;
				}
				idx-=d;

				if(r0!=NULL)
				{
					t=tm0;
				}

				if(d==1 && idx%step==0)
				{
					smallStepRemain--;
					if(smallStepRemain<=0)
					{
						d=step;
					}
				}
			}
		}
	}
}

void FsAirplane::AddSmokeRect(
    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
    int smkIdx,
    YsVec3 &p0,YsVec3 &p1,YsVec3 &p2,YsVec3 &p3,
    YsVec3 &n0,YsVec3 &n1,YsVec3 &n2,YsVec3 &n3,
    const double &a0,const double &a1,const double &a2,const double &a3) const
{
	const YsColor smkCol=Prop().GetSmokeColor(smkIdx);

	// Front face
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a0);
	nomBuf.Add(-n0);
	vtxBuf.Add(p0);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a1);
	nomBuf.Add(-n1);
	vtxBuf.Add(p1);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a2);
	nomBuf.Add(-n2);
	vtxBuf.Add(p2);

	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a2);
	nomBuf.Add(-n2);
	vtxBuf.Add(p2);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a3);
	nomBuf.Add(-n3);
	vtxBuf.Add(p3);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a0);
	nomBuf.Add(-n0);
	vtxBuf.Add(p0);

	// Back face
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a3);
	nomBuf.Add(n3);
	vtxBuf.Add(p3);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a2);
	nomBuf.Add(n2);
	vtxBuf.Add(p2);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a1);
	nomBuf.Add(n1);
	vtxBuf.Add(p1);

	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a1);
	nomBuf.Add(n1);
	vtxBuf.Add(p1);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a0);
	nomBuf.Add(n0);
	vtxBuf.Add(p0);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)a3);
	nomBuf.Add(n3);
	vtxBuf.Add(p3);
}

void FsAirplane::AddTowelSmoke(
    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
    int smkIdx,
    YsVec3 &p0,YsAtt3 &a0,const double &r0,
    YsVec3 &p1,YsAtt3 &a1,const double &r1,
    const double &alpha0,const double &alpha1) const
{
	YsVec3 p[4],n0,n1;
	YsMatrix4x4 m0,m1;

	m0.Translate(p0);
	m0.Rotate(a0);

	m1.Translate(p1);
	m1.Rotate(a1);

	p[0].Set( r0,0.0,0.0);
	p[1].Set(-r0,0.0,0.0);
	p[2].Set(-r1,0.0,0.0);
	p[3].Set( r1,0.0,0.0);


	p[0]=m0*p[0];
	p[1]=m0*p[1];
	p[2]=m1*p[2];
	p[3]=m1*p[3];

	n0.Set(0.0,1.0,0.0);
	n1.Set(0.0,1.0,0.0);

	m0.Mul(n0,n0,0.0);
	m1.Mul(n1,n1,0.0);

	AddSmokeRect(vtxBuf,nomBuf,colBuf,smkIdx,p[0],p[1],p[2],p[3],n0,n0,n1,n1,alpha0,alpha0,alpha1,alpha1);
}

void FsAirplane::AddSolidSmoke(
    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
    int smkIdx,
    YsVec3 &p0,YsAtt3 &a0,const double &r0,
    YsVec3 &p1,YsAtt3 &a1,const double &r1,
    const double &alpha0,const double &alpha1) const
{
	YsVec3 p[8],n[8];
	YsMatrix4x4 m0,m1;

	m0.Translate(p0);
	m0.Rotate(a0);

	m1.Translate(p1);
	m1.Rotate(a1);

	p[0].Set( r0,0.0,0.0);
	p[1].Set(0.0, r0,0.0);
	p[2].Set(-r0,0.0,0.0);
	p[3].Set(0.0,-r0,0.0);
	p[4].Set( r1,0.0,0.0);
	p[5].Set(0.0, r1,0.0);
	p[6].Set(-r1,0.0,0.0);
	p[7].Set(0.0,-r1,0.0);

	p[0]=m0*p[0];
	p[1]=m0*p[1];
	p[2]=m0*p[2];
	p[3]=m0*p[3];
	p[4]=m1*p[4];
	p[5]=m1*p[5];
	p[6]=m1*p[6];
	p[7]=m1*p[7];

	n[0].Set( 1.0, 0.0,0.0);
	n[1].Set( 0.0, 1.0,0.0);
	n[2].Set(-1.0, 0.0,0.0);
	n[3].Set( 0.0,-1.0,0.0);
	n[4].Set( 1.0, 0.0,0.0);
	n[5].Set( 0.0, 1.0,0.0);
	n[6].Set(-1.0, 0.0,0.0);
	n[7].Set( 0.0,-1.0,0.0);

	m0.Mul(n[0],n[0],0.0);
	m0.Mul(n[1],n[1],0.0);
	m0.Mul(n[2],n[2],0.0);
	m0.Mul(n[3],n[3],0.0);
	m1.Mul(n[4],n[4],0.0);
	m1.Mul(n[5],n[5],0.0);
	m1.Mul(n[6],n[6],0.0);
	m1.Mul(n[7],n[7],0.0);

	AddSmokeRect(vtxBuf,nomBuf,colBuf,smkIdx,p[0],p[1],p[5],p[4],n[0],n[1],n[5],n[4],alpha0,alpha0,alpha1,alpha1);
	AddSmokeRect(vtxBuf,nomBuf,colBuf,smkIdx,p[1],p[2],p[6],p[5],n[1],n[2],n[6],n[5],alpha0,alpha0,alpha1,alpha1);
	AddSmokeRect(vtxBuf,nomBuf,colBuf,smkIdx,p[2],p[3],p[7],p[6],n[2],n[3],n[7],n[6],alpha0,alpha0,alpha1,alpha1);
	AddSmokeRect(vtxBuf,nomBuf,colBuf,smkIdx,p[3],p[0],p[4],p[7],n[3],n[0],n[4],n[7],alpha0,alpha0,alpha1,alpha1);
}

void FsAirplane::AddSolidSmokeLid(
    class YsGLVertexBuffer &vtxBuf,class YsGLNormalBuffer &nomBuf,class YsGLColorBuffer &colBuf,
    int smkIdx,YsVec3 &p0,YsAtt3 &a0,const double &r0,const double &alpha0) const
{
	YsVec3 p[4],n[4];
	YsMatrix4x4 m0;

	const YsColor smkCol=Prop().GetSmokeColor(smkIdx);

	m0.Translate(p0);
	m0.Rotate(a0);

	p[0].Set( r0,0.0,0.0);
	p[1].Set(0.0, r0,0.0);
	p[2].Set(-r0,0.0,0.0);
	p[3].Set(0.0,-r0,0.0);
	p[0]=m0*p[0];
	p[1]=m0*p[1];
	p[2]=m0*p[2];
	p[3]=m0*p[3];

	n[0].Set( 1.0, 0.0,0.0);
	n[1].Set( 0.0, 1.0,0.0);
	n[2].Set(-1.0, 0.0,0.0);
	n[3].Set( 0.0,-1.0,0.0);
	m0.Mul(n[0],n[0],0.0);
	m0.Mul(n[1],n[1],0.0);
	m0.Mul(n[2],n[2],0.0);
	m0.Mul(n[3],n[3],0.0);

	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[0]);
	vtxBuf.Add(p[0]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[1]);
	vtxBuf.Add(p[1]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[2]);
	vtxBuf.Add(p[2]);

	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[2]);
	vtxBuf.Add(p[2]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[3]);
	vtxBuf.Add(p[3]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[0]);
	vtxBuf.Add(p[0]);


	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[3]);
	vtxBuf.Add(p[3]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[2]);
	vtxBuf.Add(p[2]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[1]);
	vtxBuf.Add(p[1]);

	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[1]);
	vtxBuf.Add(p[1]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[0]);
	vtxBuf.Add(p[0]);
	colBuf.Add<float>(smkCol.Rf(),smkCol.Gf(),smkCol.Bf(),(float)alpha0);
	nomBuf.Add(n[3]);
	vtxBuf.Add(p[3]);
}


void FsAirplane::Draw(
    int levelOfDetail,
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projMat,const YsVec3 &/*viewPoint*/,
    unsigned int drawFlag,
    const double &/*ctime*/) const
{
	if(levelOfDetail>=1 && lod!=nullptr)
	{
		prop.SetupVisual(lod);
		lod.SetUpSpecialRenderingRequirement();
		lod.Draw(viewTfm,projMat,GetPosition(),GetAttitude(),drawFlag);
	}
	else if(vis!=nullptr)
	{
		prop.SetupVisual(vis);
		vis.SetUpSpecialRenderingRequirement();
		vis.Draw(viewTfm,projMat,GetPosition(),GetAttitude(),drawFlag);
	}
	else
	{
		collSrc.Draw(viewTfm,projMat,GetPosition(),GetAttitude(),drawFlag);
	}
}

void FsAirplane::DrawShadow
    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm) const
{
	if(vis!=nullptr)
	{
		vis.DrawShadow(viewTfm,projTfm,GetPosition(),GetAttitude(),projPlnTfm);
	}
	else
	{
		collSrc.DrawShadow(viewTfm,projTfm,GetPosition(),GetAttitude(),projPlnTfm);
	}
}

void FsAirplane::AddSmokeToParticleManager(class YsGLParticleManager &partMan,double currentTime,double remainTime) const
{
	for(int i=0; i<Prop().GetNumSmokeGenerator(); i++)
	{
		AddSingleSmokeToParticleManager(partMan,i,currentTime,remainTime);
	}
}

void FsAirplane::AddSingleSmokeToParticleManager(class YsGLParticleManager &partMan,int smkId,double currentTime,double remainTime) const
{
	if(rec!=NULL)
	{
		YSSIZE_T idx=0;

		const YsColor smkCol=Prop().GetSmokeColor(smkId);

		// Catch the index
		double t0=YsGreater(currentTime-remainTime,0.0);
		double t=currentTime;
		while(t>t0)
		{
			if(rec->GetIndexByTime(idx,t)==YSOK)
			{
				break;
			}
			t-=0.05;
		}

		idx++; // Trick

		// Draw smoke
		FsFlightRecord *r0,*r1;
		FsFlightRecord fakeRecord;
		YsVec3 smkp0,smkp1,smkgen;
		YsAtt3 smka0,smka1;
		double tm0,tm1;
		YSBOOL drewPrevious;

		r0=rec->GetLastElement(tm0);
		if(r0!=NULL && t0<tm0)  // Smoke may exist between time t0 and t1
		{
			r0=NULL;
			r1=NULL;
			tm0=0.0;
			tm1=0.0;
			smkp0.Set(0.0,0.0,0.0);
			smkp1.Set(0.0,0.0,0.0);
			smka0.Set(0.0,0.0,0.0);
			smka1.Set(0.0,0.0,0.0);
			prop.GetSmokeGeneratorPosition(smkgen,smkId);

			drewPrevious=YSFALSE;

			while(t>t0 && idx>0)
			{
				r1=r0;
				tm1=tm0;

				r0=rec->GetElement(tm0,idx);

				if(isPlayingRecord!=YSTRUE &&
				   prop.IsTrailingSmoke(smkId)==YSTRUE &&
				   r1==NULL && r0!=NULL)
				{
					const YsVec3 *pos;
					const YsAtt3 *att;
					pos=&GetPosition();
					att=&GetAttitude();
					fakeRecord.smoke=1;
					fakeRecord.pos=*pos;
					fakeRecord.h=float(att->h());
					fakeRecord.p=float(att->p());
					fakeRecord.b=float(att->b());
					fakeRecord.smoke|=(1<<smkId);
					r1=&fakeRecord;
					tm1=currentTime;
				}

				if(r0!=NULL && r1!=NULL && ((r0->smoke&(1<<smkId))!=0 || (r1->smoke&(1<<smkId))!=0))
				{
					if(drewPrevious==YSTRUE)
					{
						smkp1=smkp0;
						smka1=smka0;
					}
					else
					{
						smka1.Set(r1->h,r1->p,r1->b);
						smka1.Mul(smkp1,smkgen);  //smkp1=smka1.GetMatrix()*smkgen;
						smkp1=smkp1+r1->pos; // (smkp1.x()+r1->x,smkp1.y()+r1->y,smkp1.z()+r1->z);
					}

					smka0.Set(r0->h,r0->p,r0->b);
					smka0.Mul(smkp0,smkgen); //smkp0=smka0.GetMatrix()*smkgen;
					smkp0=smkp0+r0->pos; // (smkp0.x()+r0->x,smkp0.y()+r0->y,smkp0.z()+r0->z);

					if(tm0<=currentTime && currentTime<tm1)
					{
						smkp1=smkp0+(smkp1-smkp0)*(currentTime-tm0)/(tm1-tm0);
					}

					double ra0,ra1;
					if(tm0<currentTime)
					{
						ra0=YsBound(sqrt(currentTime-tm0)*5.0,0.1,60.0);
					}
					else
					{
						ra0=0.0;
					}
					if(tm1<currentTime)
					{
						ra1=YsBound(sqrt(currentTime-tm1)*5.0,0.1,60.0);
					}
					else
					{
						ra1=0.0;
					}

					const double stepDist=(smkp1-smkp0).GetLength();
					const double rMin=YsGreater(0.1,YsSmaller(ra0,ra1));
					int nDiv=1+(int)(stepDist/rMin);
					for(int i=0; i<nDiv; ++i)
					{
						double param=(double)i/(double)nDiv;
						auto r=YsGreater(0.1,ra0*(1.0-param)+ra1*param);
						auto smkp=smkp0*(1.0-param)+smkp1*param;
						auto col=smkCol;

						double t=tm0*(1.0-param)+tm1*param;
						double passed=currentTime-t;
						double alpha=YsBound(0.8*YsSqr(1.0-passed/remainTime),0.0,1.0);
						col.SetAd(alpha*0.5);

						float s=(float)((i+idx)&7)*0.125;
						partMan.Add(smkp,col,r*2.5,s,0);
					}

					drewPrevious=YSTRUE;
				}
				else
				{
					drewPrevious=YSFALSE;
				}
				idx--;

				if(r0!=NULL)
				{
					t=tm0;
				}
			}
		}
	}
}

void FsAirplane::DrawSmoke(double currentTime,double remainTime,FSSMOKETYPE smk,int d,YSBOOL transparency) const
{
	for(int i=0; i<Prop().GetNumSmokeGenerator(); i++)
	{
		DrawSingleSmoke(i,currentTime,remainTime,smk,d,transparency);
	}
}

YSRESULT FsAirplane::Record(const double &t,YSBOOL forceRecord)
{
	if(rec==NULL)
	{
		rec=new FsRecord <FsFlightRecord>;
	}

	if(rec!=NULL)
	{
		FsFlightRecord dat;
		prop.WriteFlightRecord(dat);
		return Record(t,dat,forceRecord);
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsAirplane::Record(const double &t,FsFlightRecord &dat,YSBOOL forceRecord)
{
	if(rec==NULL)
	{
		rec=new FsRecord <FsFlightRecord>;
	}

	if(rec!=NULL)
	{
		const FsFlightRecord *last;
		double tLast;
		last=rec->GetLastElement(tLast);
		if(forceRecord!=YSTRUE && last!=NULL && (*last)==dat)
		{
			return YSOK;  // No Change from previous.
		}
		else if(last==NULL || tLast<t)
		{
			return rec->AddElement(dat,t);
		}
	}
	return YSERR;
}

YSRESULT FsAirplane::PlayRecord(const double &ct,const double &dt)
{
	YSSIZE_T idx;
	double t0=0.0,t1=0.0,t2=0.0,t,velocity;
	FsFlightRecord *r0=NULL,*r1=NULL,*r2=NULL,r;

	//if(rec!=NULL && ct<rec->GetRecordBeginTime())
	//{
	//	prop.SetFlightState(FSDEAD,FSDIEDOF_NULL);
	//	return YSOK;
	//}

	prop.MoveTimer(dt);

	if(rec!=NULL && rec->GetIndexByTime(idx,ct)==YSOK)
	{
		r0=rec->GetElement(t0,idx);
		r1=rec->GetElement(t1,idx+1);
		r2=rec->GetElement(t2,idx+2);
	}
	else if(rec!=NULL)
	{
		r0=NULL;
		r1=NULL;
		r2=NULL;
	}

	if(r0!=NULL && r1!=NULL)
	{
		YsVec3 unitVelVec;
		YsVec3 p,p0,p1,e,e0,e1,u,u0,u1,ed,ud,force;
		YsAtt3 a,a0,a1,vRot;

		t=(ct-t0)/(t1-t0);
		p0=r0->pos;
		p1=r1->pos;
		p=p0+(p1-p0)*t;

		velocity=(p0-p1).GetLength()/(t1-t0);

		unitVelVec=p1-p0;
		unitVelVec.Normalize();

		a0.Set(r0->h,r0->p,r0->b);
		a1.Set(r1->h,r1->p,r1->b);
		e0=a0.GetForwardVector();
		e1=a1.GetForwardVector();
		u0=a0.GetUpVector();
		u1=a1.GetUpVector();
		e=e0+(e1-e0)*t;
		u=u0+(u1-u0)*t;
		a.SetTwoVector(e,u);

		YsMatrix4x4 a0Mat(YSFALSE);
		a0.GetMatrix4x4(a0Mat);
		a0Mat.Transpose();  // Transpose is equivalent to invert for a matrix obtained from YsAtt3
		a0Mat.Mul(ed,e1,0.0);
		a0Mat.Mul(ud,u1,0.0);
		ed/=(t1-t0);
		ud/=(t1-t0);
		vRot.SetTwoVector(ed,ud);  // Rotational Velocity

		if(r0!=NULL && r1!=NULL && r2!=NULL)
		{
			if(r0->g>9999.0F) // Older version will get 10000G
			{
		 		if(r0->state==(unsigned char)FSFLYING)
				{
					YsVec3 acc;
					YsVec3 p2;
					p2=r2->pos;
					acc=((p2-p1)/(t2-t1)-(p1-p0)/(t1-t0))/(t2-t1);
					acc.SetY(acc.y()+FsGravityConst);
					force=acc*prop.GetTotalWeight();
				}
				else
				{
					force.Set(0.0,0.0,0.0);
				}
			}
			else
			{
				double g0,g1;
				g0=r0->g;
				g1=r1->g;
				force.Set(0.0,(g0+(g1-g0)*t)*FsGravityConst,0.0);
				force*=prop.GetTotalWeight();
				a.Mul(force,force);
			}

			YsVec3 p2;
			p2=r2->pos;
			if(YsEstimateVelocityVectorByQuadraticCurveFitting(unitVelVec,ct-t0,t0-t0,p0,t1-t0,p1,t2-t0,p2)!=YSOK)
			{
				unitVelVec=YsOrigin();
			}
		}
		else
		{
			force=YsOrigin(); // 2002/12/22
		}

		r=*r0;
		r.pos=p;
		r.h=(float)a.h();
		r.p=(float)a.p();
		r.b=(float)a.b();

		prop.ReadbackFlightRecord(r,dt,velocity,vRot,force,unitVelVec);

		return YSOK;
	}
	else if(r0!=NULL)
	{
		prop.ReadbackFlightRecord
		    (*r0,dt,0.0,YsAtt3(0.0,0.0,0.0),YsVec3(0.0,0.0,0.0),YsOrigin());
	}
	else if(r1!=NULL)
	{
		prop.ReadbackFlightRecord
		    (*r1,dt,0.0,YsAtt3(0.0,0.0,0.0),YsVec3(0.0,0.0,0.0),YsOrigin());
	}
	else // if(r0==NULL && r1==NULL)
	{
		r0=rec->GetTopElement(t0);
		if(r0!=NULL && ct<t0) // Before born
		{
			prop.SetFlightState(FSDEAD,FSDIEDOF_NULL); // Play dead
		}
		else
		{
			r0=rec->GetLastElement(t0);
			if(r0!=NULL)
			{
				if(t0+0.5<ct)
				{
					prop.SetFlightState(FSDEAD,FSDIEDOF_NULL); // Play dead
				}
				else if(t0<ct)
				{
					prop.ReadbackFlightRecord
					    (*r0,dt,0.0,YsAtt3(0.0,0.0,0.0),YsVec3(0.0,0.0,0.0),YsOrigin());
				}
			}
		}
	}
	return YSERR;
}

YSRESULT FsAirplane::GetAttitudeFromRecord(YsAtt3 &att,const double &t) const
{
	YSSIZE_T idx;
	double t0;

	// Catch the index
	t0=YsGreater(t,0.0);
	if(rec->GetIndexByTime(idx,t0)==YSOK)
	{
		double s,rt0,rt1;
		FsFlightRecord *r[2];
		r[0]=rec->GetElement(rt0,idx);
		r[1]=rec->GetElement(rt1,idx+1);

		if(r[0]!=NULL && r[1]!=NULL)
		{
			s=(t-rt0)/(rt1-rt0);

			YsAtt3 a[2];
			YsVec3 ev[2],uv[2];
			a[0].Set(r[0]->h,r[0]->p,r[0]->b);
			a[1].Set(r[1]->h,r[1]->p,r[1]->b);

			ev[0]=a[0].GetForwardVector();
			uv[0]=a[0].GetUpVector();
			ev[1]=a[1].GetForwardVector();
			uv[1]=a[1].GetUpVector();

			att.SetTwoVector(ev[0]*(1.0-s)+ev[1]*s,uv[0]*(1.0-s)+uv[1]*s);

			return YSOK;
		}
		else if(r[0]!=NULL)
		{
			att.Set(r[0]->h,r[0]->p,r[0]->b);
			return YSOK;
		}
		else if(r[1]!=NULL)
		{
			att.Set(r[1]->h,r[1]->p,r[1]->b);
			return YSOK;
		}
	}
	return YSERR;
}

const FsFlightRecord *FsAirplane::LastAddedRecord(void) const
{
	if(rec!=NULL)
	{
		int nRec;
		double t;
		nRec=rec->GetNumRecord();
		if(nRec>0)
		{
			return rec->GetElement(t,nRec-1);
		}
	}
	return NULL;
}

FSFLIGHTSTATE FsAirplane::GetFinalState(void) const
{
	const FsFlightRecord *lastRecord;
	lastRecord=LastAddedRecord();
	if(lastRecord!=NULL)
	{
		return (FSFLIGHTSTATE)lastRecord->state;
	}
	return FSFLYING;
}

const YsVec3 &FsAirplane::GetFinalPosition(YsVec3 &pos) const
{
	const FsFlightRecord *lastRecord;
	lastRecord=LastAddedRecord();
	if(lastRecord!=NULL)
	{
		pos=lastRecord->pos;
		return pos;
	}
	pos=YsOrigin();
	return pos;
}

void FsAirplane::ApplyControlAndGetFeedback(FsFlightControl &userInput,FSUSERCONTROL userControl,YSBOOL autoRudder)
{
	if(isPlayingRecord!=YSTRUE && userControl==FSUSC_ENABLE)
	{
		FsAutopilot *ap=GetAutopilot();
		if(ap==NULL)
		{
			Prop().ApplyControl(userInput,FSAPPLYCONTROL_ALL);
			if(autoRudder!=YSTRUE)                        // 2002/12/20
			{                                             // 2002/12/20
				Prop().TurnOffSmartRudder(); // 2002/12/20
			}                                             // 2002/12/20
		}
		else
		{
			Prop().ApplyControl(userInput,ap->OverridedControl());
			Prop().ReadBackControl(userInput);
		}
	}
	else
	{
		// Read back the current joystick state to userInput
		// in order to draw correct joystick image on the screen.
		userInput.ctlElevator=Prop().GetElevator();
		userInput.ctlAileron=Prop().GetAileron();
	}
}

YSBOOL FsAirplane::LockOn(FsSimulation *sim,const double &radarAltLimit)
{
	const YsVec3 *pos;
	const YsAtt3 *att;
	double radar,aamAngle,agmAngle;

	pos=&GetPosition();
	att=&GetAttitude();

	YsMatrix4x4 mat(prop.GetMatrix());
	mat.Invert();

	aamAngle=prop.GetAAMRadarAngle();
	agmAngle=prop.GetAGMRadarAngle();


	// Air Target
	if(prop.GuidedAAMIsLoaded()==YSTRUE)
	{
		FsAirplane *airTarget;
		FsAirplane *air;

		radar=YsPi/2.0;
		airTarget=NULL;

		air=NULL;
		while((air=sim->FindNextAirplane(air))!=NULL)
		{
			if(air->prop.IsAlive()==YSTRUE && air->iff!=this->iff)
			{
				double altLimit;
				altLimit=radarAltLimit+1000.0*(1.0-air->Prop().GetRadarCrossSection());
				if(((*pos)-air->GetPosition()).GetSquareLength()<1000.0*1000.0 || // Condition added 2008/02/10
				   pos->y()<air->GetPosition().y() ||                             // Condition added 2008/02/10
				   altLimit<air->GetPosition().y())                               // Condition added 2008/02/10
				{
					YsVec3 tpos;
					tpos=mat*air->GetPosition();

					if(tpos.z()>0.0)
					{
						double r,sqDist,rcs,aamRange;
						r=sqrt(tpos.x()*tpos.x()+tpos.y()*tpos.y());
						r=atan2(r,tpos.z());

						sqDist=tpos.GetSquareLength();
						rcs=air->prop.GetRadarCrossSection();
						if(prop.GetWeaponOfChoice()==FSWEAPON_AIM120)
						{
							aamRange=air->prop.GetAAMRange(FSWEAPON_AIM120);
						}
						else
						{
							aamRange=air->prop.GetAAMRange(FSWEAPON_AIM9);
						}
						aamRange*=rcs;

						if(r<aamAngle && r<radar && sqDist<aamRange*aamRange)
						{
							radar=r;
							airTarget=air;
						}
					}
				}
			}
		}

		prop.SetAirTargetKey(FsExistence::GetSearchKey(airTarget));
	}

	// Ground Target
	if(prop.GuidedAGMIsLoaded()>0)
	{
		FsGround *gnd;
		FsGround *gndTarget;
		gndTarget=NULL;

		radar=YsPi/2.0;

		gnd=NULL;
		while((gnd=sim->FindNextGround(gnd))!=NULL)
		{
			if(gnd->Prop().IsAlive()==YSTRUE && gnd->iff!=this->iff && gnd->Prop().IsNonGameObject()!=YSTRUE)
			{
				YsVec3 tpos;
				gnd->Prop().GetPosition(tpos);

				tpos=mat*tpos;

				if(tpos.z()>0.0)
				{
					double r;
					r=sqrt(tpos.x()*tpos.x()+tpos.y()*tpos.y());
					r=atan2(r,tpos.z());
					if(r<agmAngle && r<radar)
					{
						radar=r;
						gndTarget=gnd;
					}
				}
			}
		}

		prop.SetGroundTargetKey(FsExistence::GetSearchKey(gndTarget));
	}

	return YSTRUE;
}

FSWEAPONTYPE FsAirplane::GetWeaponOfChoice(void) const
{
	return Prop().GetWeaponOfChoice();
}

YSBOOL FsAirplane::GetDamage(YSBOOL &killed,int dmg,FSDIEDOF diedOf)
{
	return prop.GetDamage(killed,dmg,diedOf);
}

void FsAirplane::Bounce(const YsVec3 &collPos)
{
	if(Prop().GetVelocity()<Prop().GetSafeGroundSpeed())
	{
		YsVec3 bounceDir=Prop().GetPosition()-collPos;
		bounceDir.SetY(0.0);
		if(YSOK==bounceDir.Normalize())
		{
			const double outRad=Prop().GetOutsideRadius();
			const YsVec3 newPos=Prop().GetPosition()+bounceDir*outRad/10.0;
			Prop().SetPosition(newPos);
		}
	}
}

YSRESULT FsAirplane::RipOffEarlyPartOfRecord(void)
{
	if(rec!=NULL)
	{
		rec->RipOffEarlyPartOfRecord();
	}
	return YSOK;
}

int FsAirplane::RerecordByNewInterval(const double &itvl)
{
	FsRecord <FsFlightRecord> *newRec;
	int n;

	n=FsRerecordByNewInterval <FsFlightRecord> (newRec,rec,itvl);
	delete rec;
	rec=newRec;

	return n;
}

void FsAirplane::AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng)
{
	FsAdjustPrecisionOfFlightRecord <FsFlightRecord> (rec,precPos,precAng);
}

int FsAirplane::FindLandingWithinTimeRange(YsArray <YSSIZE_T> &recordIndexArray,const double t1,const double t2) const
{
	YSSIZE_T idx1,idx2;

	recordIndexArray.Clear();
	if(NULL!=rec && YSOK==rec->GetIndexByTime(idx1,idx2,t1,t2))
	{
		for(YSSIZE_T idx=idx1; idx<idx2-1 && idx<rec->GetNumRecord(); ++idx)
		{
			double t1,t2;
			const FsFlightRecord *rec1=rec->GetElement(t1,idx);
			const FsFlightRecord *rec2=rec->GetElement(t2,idx+1);
			if(NULL!=rec1 &&
			   NULL!=rec2 &&
			   (FSFLIGHTSTATE)rec1->state!=FSGROUND && (FSFLIGHTSTATE)rec1->state!=FSGROUNDSTATIC &&
			  ((FSFLIGHTSTATE)rec2->state==FSGROUND || (FSFLIGHTSTATE)rec2->state==FSGROUNDSTATIC))
			{
				recordIndexArray.Append(idx);
			}
		}
	}
	return (int)recordIndexArray.GetN();
}

const FsAirplaneProperty &FsAirplane::Prop(void) const
{
	return prop;
}

FsAirplaneProperty &FsAirplane::Prop(void)
{
	return prop;
}

int FsAirplane::GetDefaultDamageTolerance(void) const
{
	return YsGreater(1,defDamageTolerance);
}

FsVehicleProperty &FsAirplane::CommonProp(void)
{
	return prop;
}

const FsVehicleProperty &FsAirplane::CommonProp(void) const
{
	return prop;
}

YSRESULT FsAirplane::SetProperty(const FsAirplaneProperty &prp,const wchar_t tmplRootDir[])
{
	prop=prp;
	prop.belongTo=this;
	defDamageTolerance=prop.GetDamageTolerance();

	if(YSTRUE==prop.HasInstPanel())
	{
		instPanel=new FsInstrumentPanel;
		if(prop.GetInstPanelFileName()!=NULL && prop.GetInstPanelFileName()[0]!=0)
		{
			YsWString fullPathInstPanelName;
			fullPathInstPanelName.MakeFullPathName(tmplRootDir,prop.GetInstPanelFileName());
			instPanel->LoadIsp(fullPathInstPanelName);
		}
	}

	return YSOK;
}

YSRESULT FsAirplane::ClearAutopilot(void)
{
	int i;
	forYsArray(i,autoPilotList)
	{
		if(autoPilotList[i]!=NULL)
		{
			FsAutopilot::Delete(autoPilotList[i]);
		}
	}
	curAutoPilotIdx=0;
	autoPilotList.Clear();
	return YSOK;
}

YSRESULT FsAirplane::SetAutopilot(FsAutopilot *au)
{
	ClearAutopilot();

	if(au==NULL)
	{
		prop.TurnOffController();
	}
	else
	{
		autoPilotList.Append(au);
	}

	return YSOK;
}

YSRESULT FsAirplane::AddAutoPilot(FsAutopilot *ap)
{
	if(ap!=NULL)
	{
		autoPilotList.Append(ap);
		return YSOK;
	}
	return YSERR;
}

const FsAutopilot *FsAirplane::GetAutopilot(void) const
{
	if(autoPilotList.IsInRange(curAutoPilotIdx)==YSTRUE)
	{
		return autoPilotList[curAutoPilotIdx];
	}
	return NULL;
}

FsAutopilot *FsAirplane::GetAutopilot(void)
{
	if(autoPilotList.IsInRange(curAutoPilotIdx)==YSTRUE)
	{
		return autoPilotList[curAutoPilotIdx];
	}
	return NULL;
}

const double &FsAirplane::GetLandWhenLowFuel(void) const
{
	return landWhenLowFuelThr;
}

YSBOOL FsAirplane::NextAutoPilotAvailable(void) const
{
	if(autoPilotList.GetN()>0 && curAutoPilotIdx<autoPilotList.GetN()-1)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsAirplane::MoveOnToNextAutoPilot(void)
{
	if(curAutoPilotIdx<autoPilotList.GetN()-1)
	{
		curAutoPilotIdx++;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsAirplane::SaveAutoPilot(FILE *fp,const FsSimulation *sim)
{
	int i;
	forYsArray(i,autoPilotList)
	{
		if(autoPilotList[i]!=NULL)
		{
			autoPilotList[i]->Save(fp,sim);
		}
	}
	return YSOK;
}

YSRESULT FsAirplane::RecallReloadCommandOnly(void)
{
	for(auto &cmd : reloadCommand)
	{
		prop.SendCommand(cmd);
	}
	return YSOK;
}

YSBOOL FsAirplane::HitGround(
    FSDIEDOF &diedOf,
    int &collType,   // 1:Ground  2:Shell
    double ctime,const FsField &field,YSBOOL takeCrash,YSBOOL canLandAnywhere,
    FsExplosionHolder * /*explosion*/)
{
	collType=0;
	if(prop.IsAlive()==YSTRUE)
	{
		const YsVec3 *pos;
		YSBOOL isOnRunway;

		pos=&prop.GetPosition();
		isOnRunway=YSFALSE;

		if(YSTRUE==field.GetFieldShellCollision(UntransformedCollisionShell().Conv(),GetMatrix()))
		{
			diedOf=FSDIEDOF_TERRAIN;
			collType=2;
			goto COLLIDE;
		}
		if(prop.IsOnGround()==YSTRUE)
		{
			YsArray <const YsSceneryRectRegion *,16> rgn;
			if(field.GetFieldRegion(rgn,pos->x(),pos->z())==YSOK) // 2003/01/04
			{
				int i;
				for(i=0; i<rgn.GetN(); i++)
				{
					if(FsSimulation::IsSafeTerrainRegionId(rgn[i]->GetId())==YSTRUE)
					{
						isOnRunway=YSTRUE;
						break;
					}
				}
			}
		}

		if(prop.IsOnGround()!=YSTRUE || canLandAnywhere==YSTRUE)
		{
			double elv;
			field.GetFieldElevation(elv,pos->x(),pos->z());
			if(pos->y()<=elv+YsTolerance)
			{
				if(takeCrash!=YSTRUE)
				{
					YsVec3 newPos;
					newPos=*pos;
					newPos.SetY(elv+YsTolerance);
					prop.SetPosition(newPos);
				}
				diedOf=FSDIEDOF_TERRAIN;
				collType=1;
				goto COLLIDE;
			}
		}

		if(prop.IsOnGround()==YSTRUE)
		{
			if(isOnRunway==YSTRUE)
			{
				prop.ClearNeedTouchdownCheck();  // <- Yes, it passes touchdown check.
				prop.SetOutOfRunway(YSFALSE);
			}
			else
			{
				if(canLandAnywhere!=YSTRUE)
				{
					if(prop.GetNeedTouchdownCheck()==YSTRUE)  // <- CheckHasJustTouchDown cannot be used here!
					{
						prop.ClearNeedTouchdownCheck();
						diedOf=FSDIEDOF_LANDEDOUTOFRUNWAY;
						collType=1;
						goto COLLIDE;
					}
					else
					{
						diedOf=FSDIEDOF_OVERRUN;
						collType=1;
						goto OVERRUN;
					}
				}
				else
				{
					if(prop.GetNeedTouchdownCheck()==YSTRUE)  // <- CheckHasJustTouchDown cannot be used here!
					{
						prop.ClearNeedTouchdownCheck();

						YSSCNAREATYPE areaType;
						areaType=field.GetAreaType(GetPosition());

						if(areaType!=YSSCNAREA_LAND)
						{
							YsPrintf("Splashed into the water.\n");
							diedOf=FSDIEDOF_LANDEDOUTOFRUNWAY;
							collType=1;
							goto COLLIDE;
						}
					}

					prop.ClearNeedTouchdownCheck();  // <- Yes, it passes touchdown check.
					prop.SetOutOfRunway(YSTRUE);
				}
			}
		}
		else
		{
			prop.SetOutOfRunway(YSFALSE);
		}

		return YSFALSE;
	COLLIDE:
		// This function will be called from FsSimulation.  -> Crash(ctime,takeCrash,explosion,diedOf);
		return YSTRUE;
	OVERRUN:
		Overrun(ctime);
		return YSFALSE;  // It's not considered a 'hitting a ground.'
	}
	return YSFALSE;
}

void FsAirplane::Crash(
   const double &ctime,YSBOOL takeCrash,FsExplosionHolder *explosion,FSDIEDOF diedOf,YSSCNAREATYPE areaType)
{
	const YsVec3 *pos;
	const YsAtt3 *att;
	att=&GetAttitude();
	pos=&GetPosition();
	if(att->p()<0.0)
	{
		YsAtt3 newAtt(*att);
		newAtt.SetP(0.0);
		prop.SetAttitude(newAtt);
	}
	if(explosion!=NULL)
	{
		if(areaType!=YSSCNAREA_WATER)
		{
			explosion->Explode(ctime,*pos,10.0,0.0,prop.GetOutsideRadius()*8.0,YSTRUE,this,YSTRUE);
		}
		else
		{
			explosion->WaterPlume(ctime,*pos,5.0,prop.GetOutsideRadius(),prop.GetOutsideRadius()*5.0,NULL,YSTRUE);
		}
	}
	if(takeCrash==YSTRUE)
	{
		prop.Crash(diedOf);
	}
}

void FsAirplane::Overrun(const double &/*ctime*/)
{
	prop.Overrun();
}

YSRESULT FsAirplane::SendCommand(const char str[])
{
	YsString cmd;
	cmd.Set(str);
	cmdLog.Append(cmd);
	return prop.SendCommand(str);
}

YSRESULT FsAirplane::AutoSendCommand(YSSIZE_T nWeaponConfig,const int weaponConfig[],int fuel)
{
	char cmd[256];

	AutoSendCommand(nWeaponConfig,weaponConfig);

	sprintf(cmd,"INITFUEL %d%%",fuel);
	SendCommand(cmd);

	return YSOK;
}

YSRESULT FsAirplane::AutoSendCommand(YSSIZE_T nWeaponConfig,const int weaponConfig[])
{

	char cmd[256];
	// YSBOOL smk;

	SendCommand("UNLOADWP");

	YSBOOL smokeAlreadyLoaded=YSFALSE;
	for(YSSIZE_T i=0; i<nWeaponConfig-1; i+=2)
	{
		if((FSWEAPONTYPE)weaponConfig[i]==FSWEAPON_SMOKE)
		{
			if(YSTRUE!=smokeAlreadyLoaded)
			{
				SendCommand("SMOKEOIL 100.0kg");
				smokeAlreadyLoaded=YSTRUE;
			}
			const int r=(weaponConfig[i+1]&0xff0000)>>16;
			const int g=(weaponConfig[i+1]&0x00ff00)>>8;
			const int b=(weaponConfig[i+1]&0x0000ff);

			YsString cmd;
			cmd.Printf("SMOKECOL ALL %d %d %d",r,g,b);
			SendCommand(cmd);
		}
		else if((int)FSWEAPON_SMOKE0<=weaponConfig[i] && weaponConfig[i]<=(int)FSWEAPON_SMOKE7)
		{
			const int smkIdx=weaponConfig[i]-(int)FSWEAPON_SMOKE0;

			if(YSTRUE!=smokeAlreadyLoaded)
			{
				SendCommand("SMOKEOIL 100.0kg");
				smokeAlreadyLoaded=YSTRUE;
			}
			const int r=(weaponConfig[i+1]&0xff0000)>>16;
			const int g=(weaponConfig[i+1]&0x00ff00)>>8;
			const int b=(weaponConfig[i+1]&0x0000ff);

			YsString cmd;
			cmd.Printf("SMOKECOL %d %d %d %d",smkIdx,r,g,b);
			SendCommand(cmd);
		}
		else
		{
			sprintf(cmd,"LOADWEPN %s %d",FsGetWeaponString((FSWEAPONTYPE)weaponConfig[i]),weaponConfig[i+1]);
			SendCommand(cmd);
		}
	}

	return YSOK;
}

YSRESULT FsAirplane::RecallCommand(void)
{
	int i;
	for(i=0; i<cmdLog.GetN(); i++)
	{
		prop.SendCommand(cmdLog[i]);
	}
	RecallReloadCommandOnly();
	return YSOK;
}

// See memo/designmen/flightschedule.txt
YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE FsAirplane::GetCurrentDestination(YsString &label) const
{
	const FsAutopilot *ap=GetAutopilot();
	if(NULL!=ap)
	{
		switch(ap->Type())
		{
		case FSAUTOPILOT_AIRWAY:
			{
				const FsAirRouteAutopilot *airRouteAP=(const FsAirRouteAutopilot *)ap;
				label.Set(airRouteAP->CurrentSegmentLabel());
				return airRouteAP->CurrentSegmentType();
			}
			break;
		default:
			break;
		}
	}
	label.Set("");
	return YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
}

YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE FsAirplane::GetOccupyingAirportOrFix(YsString &label) const
{
	const FsAutopilot *ap=GetAutopilot();
	if(NULL!=ap)
	{
		switch(ap->Type())
		{
		case FSAUTOPILOT_AIRWAY:
			{
				const FsAirRouteAutopilot *airRouteAP=(const FsAirRouteAutopilot *)ap;
				label.Set(airRouteAP->OccupyingSegmentLabel());
				return airRouteAP->OccupyingSegmentType();
			}
			break;
		default:
			break;
		}
	}
	label.Set("");
	return YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
}

const FsAirTrafficInfo &FsAirplane::GetAirTrafficInfo(void) const
{
	return airTrafficInfo;
}

FsAirTrafficInfo &FsAirplane::GetAirTrafficInfo(void)
{
	return airTrafficInfo;
}

YSRESULT FsAirplane::AtcRequestIlsApproach(class FsSimulation *sim,class FsAirTrafficController *atc,class FsGround *ils)
{
	if(NULL!=atc && NULL!=ils)
	{
		if(YSOK==atc->AirplaneCheckIn(sim,this) ||
		   GetAirTrafficInfo().GetAirTrafficController(sim)==atc)
		{
			return atc->AirplaneRequestApproachByILS(sim,*this,ils,"");
		}
	}
	return YSERR;
}

// Implementation //////////////////////////////////////////
FsGround::FsGround()
{
	Initialize();
}

FsGround::FsGround(const FsGround &from)
{
	*this=from;
	SetProperty(from.Prop());
}

FsGround::~FsGround()
{
	CleanUp();
}

FSEXISTENCETYPE FsGround::GetType(void) const
{
	return FSEX_GROUND;
}

void FsGround::Initialize(void)
{
	FsExistence::Initialize();

	ClearAutoDrive();

	rec=NULL;
	isPlayingRecord=YSFALSE;
	initPosition.Set(0.0,0.0,0.0);
	initAttitude.Set(0.0,0.0,0.0);

	prop.belongTo=this;
	prop.CleanUpAircraftCarrierProperty();

	netPos=initPosition;
	netAtt=initAttitude;
	netShootingAaa=YSFALSE;
	netShootingCannon=YSFALSE;
	netClockLocal=-1.0;
	netClockRemote=-1.0;

	gndFlag=0;

	thisInTheList=NULL;

	nextTargetSearchTime=0.0;
}

void FsGround::CleanUp(void)
{
	if(rec!=NULL)
	{
		delete rec;
		rec=NULL;
	}

	FsExistence::CleanUp();
}

void FsGround::GetWhereToAim(YsVec3 &trgPos) const
{
	if(Prop().chFlags & FsGroundProperty::YSGP_USEAIMAT)  // 2005/04/09
	{                                                     // 2005/04/09
		trgPos=Prop().chAimAt;                            // 2005/04/09
		GetAttitude().Mul(trgPos,trgPos);                 // 2005/04/09
		trgPos+=GetPosition();                            // 2005/04/09
	}                                                     // 2005/04/09
	else                                                  // 2005/04/09
	{
		double aimHeight;
		aimHeight=(collBbx[0].y()+collBbx[1].y())/2.0;
		trgPos=GetPosition();
		trgPos.AddY(aimHeight);
	}
}

void FsGround::Draw
    (int levelOfDetail,
	 const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsVec3 &viewPos,
	 unsigned int drawFlag,
	 const double &ctime) const
{
	if(levelOfDetail>=1 && lod!=nullptr)
	{
		prop.SetupVisual(lod,viewPos,ctime);
		lod.SetUpSpecialRenderingRequirement();
		lod.Draw(viewTfm,projTfm,GetPosition(),GetAttitude(),drawFlag);
	}
	else if(vis!=nullptr)
	{
		prop.SetupVisual(vis,viewPos,ctime);
		vis.SetUpSpecialRenderingRequirement();
		vis.Draw(viewTfm,projTfm,GetPosition(),GetAttitude(),drawFlag);
	}
}

void FsGround::DrawShadow
    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,const YsMatrix4x4 &projPlnTfm) const
{
	if(vis!=nullptr)
	{
		vis.DrawShadow(viewTfm,projTfm,GetPosition(),GetAttitude(),projPlnTfm);
	}
}

YSRESULT FsGround::GetAttitudeFromRecord(YsAtt3 &/*att*/,const double &/*t*/) const
{
	return YSERR;
}

YSRESULT FsGround::SetProperty(const FsGroundProperty &prp)
{
	prop=prp;
	prop.belongTo=this;
	return YSOK;
}

FsGroundProperty &FsGround::Prop(void)
{
	return prop;
}

const FsGroundProperty &FsGround::Prop(void) const
{
	return prop;
}

FsVehicleProperty &FsGround::CommonProp(void)
{
	return prop;
}

const FsVehicleProperty &FsGround::CommonProp(void) const
{
	return prop;
}

void FsGround::ProcessLoadingOnAircraftCarrier(YSSIZE_T nCarrier,FsGround *carrier[])
{
	FsGround *toLoad=NULL;
	double toLoadDeckHeight=0.0;
	YsVec3 refPos;
	GetMatrix().Mul(refPos,collCen,1.0);

	for(decltype(nCarrier) i=0; i<nCarrier; i++)
	{
		if(carrier[i]!=this)
		{
			FsAircraftCarrierProperty *const carrierProp=carrier[i]->Prop().GetAircraftCarrierProperty();
			if(NULL!=carrierProp && YSTRUE==carrierProp->IsOnDeck(GetPosition()))
			{
				YsVec3 nom;
				const double deckHeight=carrierProp->GetDeckHeightAndNormal(nom,GetPosition());
				if(deckHeight<refPos.y())
				{
					if(NULL==toLoad || toLoadDeckHeight<deckHeight)
					{
						toLoad=carrier[i];
						toLoadDeckHeight=deckHeight;
					}
				}
			}
		}
	}

	if(NULL!=toLoad && toLoad!=Prop().OnThisCarrier())
	{
		if(YSTRUE==Prop().IsOnCarrier())
		{
			Prop().OnThisCarrier()->Prop().GetAircraftCarrierProperty()->UnloadGround(this);
		}
		toLoad->Prop().GetAircraftCarrierProperty()->LoadGround(this);
	}
}

YSBOOL FsGround::PiggyBack(void) const
{
	if(NULL!=Prop().OnThisCarrier() &&
	   motionPath==Prop().OnThisCarrier()->motionPath &&
	   YSTRUE==useMotionPathOffset &&
	   YSTRUE==Prop().OnThisCarrier()->useMotionPathOffset)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsGround::Settle(const YsVec3 &pos)
{
	prop.SetPosition(pos);
	initPosition=pos;
	return YSOK;
}

YSRESULT FsGround::Settle(const YsAtt3 &att)
{
	prop.SetAttitude(att);
	initAttitude=att;
	return YSOK;
}

YSRESULT FsGround::Record(const double &t,YSBOOL forceRecord)
{
	if(rec==NULL)
	{
		rec=new FsRecord <FsGroundRecord>;
	}

	if(rec!=NULL)
	{
		FsGroundRecord dat;
		prop.WriteRecord(dat);
		return Record(t,dat,forceRecord);
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsGround::Record(const double &t,FsGroundRecord &dat,YSBOOL forceRecord)
{
	if(rec==NULL)
	{
		rec=new FsRecord <FsGroundRecord>;
	}

	if(rec!=NULL)
	{
		const FsGroundRecord *last;
		double tLast;
		last=rec->GetLastElement(tLast);
		if(forceRecord!=YSTRUE && last!=NULL && (*last)==dat)
		{
			return YSOK;  // No Change from previous.
		}
		else if(last==NULL || tLast<t)
		{
			return rec->AddElement(dat,t);
		}
	}
	return YSERR;
}

YSRESULT FsGround::PlayRecord(const double &ct,const double &dt)
{
	YSSIZE_T idx;
	double t0=0.0,t1=0.0,velocity;
	FsGroundRecord *r0=NULL,*r1=NULL,r;

	if(rec!=NULL && ct<rec->GetRecordBeginTime())
	{
		prop.SetState(FSGNDDEAD);
		return YSOK;
	}

	if(rec!=NULL && rec->GetIndexByTime(idx,ct)==YSOK)
	{
		r0=rec->GetElement(t0,idx);
		r1=rec->GetElement(t1,idx+1);
	}
	else if(rec!=NULL)
	{
		r0=NULL;
		r1=NULL;
	}

	if(r0!=NULL && r1!=NULL)
	{
		YsVec3 p,p0,p1,e,e0,e1,u,u0,u1;
		YsAtt3 a,a0,a1;

		const double t=(ct-t0)/(t1-t0);
		const int it=(int)(t*256.0);

		p0=r0->pos;
		p1=r1->pos;
		p=p0+(p1-p0)*t;

		velocity=(p0-p1).GetLength()/(t1-t0);

		a0.Set(r0->h,r0->p,r0->b);
		a1.Set(r1->h,r1->p,r1->b);
		e0=a0.GetForwardVector();
		e1=a1.GetForwardVector();
		u0=a0.GetUpVector();
		u1=a1.GetUpVector();
		e=e0+(e1-e0)*t;
		u=u0+(u1-u0)*t;
		a.SetTwoVector(e,u);

		r=*r0;

		r.steering=(unsigned char)(((256-it)*(int)r0->steering+it*(int)r1->steering)/256);
		r.leftDoor=(unsigned char)(((256-it)*(int)r0->leftDoor+it*(int)r1->leftDoor)/256);
		r.rightDoor=(unsigned char)(((256-it)*(int)r0->rightDoor+it*(int)r1->rightDoor)/256);
		r.rearDoor=(unsigned char)(((256-it)*(int)r0->rearDoor+it*(int)r1->rearDoor)/256);

		r.pos=p;
		r.h=(float)a.h();
		r.p=(float)a.p();
		r.b=(float)a.b();
		prop.ReadbackRecord(r,dt,velocity);

		return YSOK;
	}
	else if(r0!=NULL)
	{
		prop.ReadbackRecord(*r0,dt,0.0);
	}
	else if(r1!=NULL)
	{
		prop.ReadbackRecord(*r1,dt,0.0);
	}
	else // if(r0==NULL && r1==NULL)
	{
		r0=rec->GetTopElement(t0);
		if(r0!=NULL && ct<t0) // Before born
		{
			prop.SetState(FSGNDDEAD); // Pretend to be dead
		}
		else
		{
			r0=rec->GetLastElement(t0);
			if(r0!=NULL && t0<ct)
			{
				prop.ReadbackRecord(*r0,dt,0.0);
			}
		}
	}
	return YSERR;
}

const FsGroundRecord *FsGround::LastAddedRecord(void) const
{
	if(rec!=NULL)
	{
		int nRec;
		double t;
		nRec=rec->GetNumRecord();
		if(nRec>0)
		{
			return rec->GetElement(t,nRec-1);
		}
	}
	return NULL;
}

FSGNDSTATE FsGround::GetFinalState(void) const
{
	const FsGroundRecord *rec;
	rec=LastAddedRecord();
	if(rec!=NULL)
	{
		return FSGNDSTATE(rec->state);
	}
	return FSGNDALIVE;
}

const YsVec3 &FsGround::GetFinalPosition(YsVec3 &pos) const
{
	const FsGroundRecord *rec;
	rec=LastAddedRecord();
	if(rec!=NULL)
	{
		pos=rec->pos;;
		return pos;
	}
	pos=YsVec3::Origin();
	return pos;
}

const YsAtt3 &FsGround::GetFinalAttitude(YsAtt3 &att) const
{
	const FsGroundRecord *rec;
	rec=LastAddedRecord();
	if(rec!=NULL)
	{
		att.Set(rec->h,rec->p,rec->b);
		return att;
	}
	att=YsZeroAtt();
	return att;
}

void FsGround::SetAutoDrive(FsAutoDrive *drv) // FsGround will take ownership of the auto-drive
{
	ClearAutoDrive();
	PushAutoDrive(drv);
}

void FsGround::PushAutoDrive(FsAutoDrive *drv) // FsGround will take ownership of the auto-drive
{
	autoDriveStack.Append(drv);
}

void FsGround::ClearAutoDrive(void)
{
	for(int i=0; i<autoDriveStack.GetN(); ++i)
	{
		delete autoDriveStack[i];
	}
	autoDriveStack.Clear();
}

void FsGround::DeleteLastAutoDrive(void)
{
	delete autoDriveStack.GetEnd();
	autoDriveStack.DeleteLast();
}

int FsGround::GetNumAutoDrive(void) const
{
	return (int)autoDriveStack.GetN();
}

YSBOOL FsGround::IsAutoDrivingTo(const FsExistence *objPtr) const
{
	if(NULL!=objPtr && 
	   0<autoDriveStack.GetN() && 
	   autoDriveStack.GetEnd()->GetType()==FsAutoDrive::AUTODRIVE_TOOBJECT)
	{
		const FsAutoDriveToObject *autoDrive=(const FsAutoDriveToObject *)autoDriveStack.GetEnd();
		if(autoDrive->GetGoalObjKey()==(int)objPtr->SearchKey())
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSRESULT FsGround::DefaultControl(FsSimulation *sim,const double &dt)
{
	if(0<autoDriveStack.GetN())
	{
		if(FSVEHICLE_CTRL_BY_PILOT!=Prop().GetWhoIsInControl())
		{
			Prop().SetWhoIsInControl(FSVEHICLE_CTRL_BY_AUTOPILOT);
		}
		autoDriveStack.GetEnd()->Control(sim,this);
		if(YSTRUE==autoDriveStack.GetEnd()->ObjectiveAccomplished(sim,this))
		{
			DeleteLastAutoDrive();
		}
		return YSOK;
	}


	YsVec3 trgEst,trgPos,trgRel,trgVel;
	YsAtt3 relAtt;
	double distance,estTime;

	nextTargetSearchTime-=dt;
	if(nextTargetSearchTime<YsTolerance && 
	   (YSTRUE==prop.HasWeapon() || 
	    (gndFlag&FSGNDFLAG_CASMISSIONTANK)!=0 ||
	    prop.TrackAirTarget()==YSTRUE ||
	    prop.TrackGndTarget()==YSTRUE))
	{
		SearchTarget(sim);
		nextTargetSearchTime=(double)(rand()%80);
		nextTargetSearchTime/=10.0;
	}

	// if(strcmp(prop.GetIdentifier(),"BURKE_DESTROYER")==0 && iff!=0)
	// {
	// 	printf("* %08x %.2lf %d\n",prop.sta.AirTarget,nextTargetSearchTime,iff);
	// 	if(prop.sta.AirTarget!=NULL)
	// 	{
	// 		printf("%s\n",prop.sta.AirTarget->Prop().GetIdentifier());
	// 	}
	// }


	// 2005/03/05
	//   Two variables, firingCannon and firingAaa are controlled the server.
	//   Do not modify if it is not a local object.
	if(netType==FSNET_LOCAL)
	{
		prop.StopFiringAaa();  // Ceasefire if no target is present!  2003/12/23
		prop.StopFiringCannon();
	}


	if(motionPath!=NULL && 
	   motionPath->IsLoop()!=YSTRUE && 
	   motionPath->GetNumPoint()<=motionPathIndex &&
	   (gndFlag&FSGNDFLAG_MOVETOWARDGNDTARGET)!=0)  // 2005/03/28
	{
		motionPath=NULL;
	}


	if(prop.IsAntiGround()==YSTRUE && prop.GetGroundTarget()!=NULL && prop.GetGroundTarget()->IsAlive()==YSTRUE)
	{
		prop.GetGroundTarget()->GetWhereToAim(trgPos);
		prop.GetGroundTarget()->Prop().GetVelocity(trgVel);
		distance=(trgPos-GetPosition()).GetLength();

		if(prop.GetNumCannon()>0)
		{
			estTime=distance/prop.GetBulletSpeed();

			trgEst=trgPos+trgVel*estTime;
			trgEst.AddY(0.5*FsGravityConst*estTime*estTime);

			trgRel=trgEst-GetPosition();
			if(trgRel.GetSquareLength()<YsSqr(prop.chCannonRange)*1.5)
			{
				if(netType==FSNET_LOCAL)
				{
					double sqLength;
					sqLength=trgRel.GetSquareLength();
					if(sqLength<YsSqr(prop.chCannonRange))
					{
						prop.StartFiringCannon();
					}
				}
				YsAtt3 aim;
				AimAt(aim,prop.GetCannonAim(),trgEst,prop.chCanMount,dt,prop.chCanMinAimPitch,prop.chCanMaxAimPitch);
				prop.SetCannonAim(aim);
			}
			else
			{
				prop.SetCannonAim(GetAttitude());
			}
		}
	}

	if(prop.GetAirTarget()!=NULL && prop.GetAirTarget()->IsAlive()==YSTRUE)
	{
		// Aiming GUN. Consider lead and gravity.
		trgPos=prop.GetAirTarget()->GetPosition();
		prop.GetAirTarget()->Prop().GetVelocity(trgVel);

		distance=(trgPos-GetPosition()).GetLength();
		estTime=distance/prop.GetBulletSpeed();

		trgEst=trgPos+trgVel*estTime;
		trgPos.SetY(trgPos.y()+0.5*FsGravityConst*estTime*estTime);

		trgRel=trgEst-GetPosition();
		if(trgRel.GetSquareLength()<YsSqr(prop.chAaaRange)*1.5)
		{
			if(netType==FSNET_LOCAL)
			{
				double sqLength;
				sqLength=trgRel.GetSquareLength();
				if(sqLength<YsSqr(prop.chAaaRange))
				{
					prop.StartFiringAaa();
				}
			}

			YsAtt3 aim;
			AimAt(aim,prop.GetAaaAim(),trgEst,prop.chAaaMount,dt,prop.chAaaMinAimPitch,prop.chAaaMaxAimPitch);
			prop.SetAaaAim(aim);
		}


		if(prop.GetNumSAM()>0)
		{
			trgPos=prop.GetAirTarget()->GetPosition();
			trgEst=trgPos;
			trgRel=trgEst-GetPosition();
			if(trgRel.GetSquareLength()<(prop.chSAMRange*prop.chSAMRange)*1.5)
			{
				YsAtt3 aim;
				AimAt(aim,prop.GetSamAim(),trgEst,prop.chSamMount,dt,prop.chSamMinAimPitch,prop.chSamMaxAimPitch);
				prop.SetSamAim(aim);
			}
		}
		else if(prop.TrackAirTarget()==YSTRUE)
		{
			trgPos=prop.GetAirTarget()->GetPosition();
			trgEst=trgPos;
			trgRel=trgEst-GetPosition();

			YsAtt3 aim;
			AimAt(aim,prop.GetSamAim(),trgEst,prop.chSamMount,dt,prop.chSamMinAimPitch,prop.chSamMaxAimPitch);
			prop.SetSamAim(aim);
		}
	}


	if((gndFlag&FSGNDFLAG_MOVETOWARDGNDTARGET)!=0)
	{
		if(prop.GetGroundTarget()!=NULL && prop.GetGroundTarget()->IsAlive()==YSTRUE)
		{
			YsVec3 v;
			double dh;
			v=prop.GetGroundTarget()->GetPosition()-GetPosition();
			GetAttitude().MulInverse(v,v);
			dh=atan2(-v.x(),v.z());

			prop.SetRotation(YsBound(dh*5.0,-prop.chMaxRotation,prop.chMaxRotation));

			if(v.z()>fabs(v.x()) && v.GetSquareLength()>1500.0*1500.0)
			{
				prop.SetRelativeVelocity(YsVec3(0.0,0.0,prop.GetMaxSpeed()));
			}
			else
			{
				prop.SetRelativeVelocity(YsOrigin());
			}
		}
	}


	return YSOK;
}

YSRESULT FsGround::AimAt(
    YsAtt3 &aim,
    const YsAtt3 &prevAim,
    const YsVec3 &trg,const YsVec3 &gunMount,const double &dt,const double &minAimPitch,const double &maxAimPitch)
{
	double dh,dp;
	double maxHeading,maxPitching;
	int i;
	YsVec3 trgRel,gunPos;
	YsAtt3 relAtt;

	aim=prevAim;

	GetAttitude().Mul(gunPos,gunMount);
	gunPos+=GetPosition();

	trgRel=trg-gunPos;
	relAtt.SetForwardVector(trgRel);

	maxHeading=prop.chAimRotationSpeed*dt;
	maxPitching=prop.chAimPitchSpeed*dt;

	dh=relAtt.h()-aim.h();
	dp=relAtt.p()-aim.p();

	i=0;
	while(dh<-YsPi || YsPi<dh)
	{
		if(dh<-YsPi)
		{
			dh+=YsPi*2.0;
		}
		else
		{
			dh-=YsPi*2.0;
		}
		i++;
		if(i>=10)
		{
			return YSERR;
		}
	}

	if(YsAbs(dh)<maxHeading)
	{
		aim.SetH(relAtt.h());
	}
	else if(dh<0)
	{
		aim.SetH(aim.h()-maxHeading);
	}
	else if(dh>0)
	{
		aim.SetH(aim.h()+maxHeading);
	}

	if(YsAbs(dp)<maxPitching)
	{
		aim.SetP(relAtt.p());
	}
	else if(dp<0)
	{
		aim.SetP(aim.p()-maxPitching);
	}
	else if(dp>0)
	{
		aim.SetP(aim.p()+maxPitching);
	}
	if(aim.p()<minAimPitch)
	{
		aim.SetP(minAimPitch);
	}
	if(aim.p()>maxAimPitch)
	{
		aim.SetP(maxAimPitch);
	}

	return YSOK;
}

void FsGround::SearchTarget(FsSimulation *sim)
{
	double dist;

	if(prop.IsAntiGround()==YSTRUE)
	{
		int i;
		FsGround *gndTrg,*gnd;
		YSBOOL targetAny;


		// No flag -> Targets only Navy Ships and Tanks
		// FSGNDFLAG_TARGETANY       -> Targets anything (Follow path, or stay still)

		if((gndFlag&FSGNDFLAG_TARGETANY)!=0)
		{
			targetAny=YSTRUE;
		}
		else
		{
			targetAny=YSFALSE;
		}


		dist=10000000000.0;
		gndTrg=NULL;
		for(i=0; i<2; i++)
		{
			gnd=NULL;
			while((gnd=sim->FindNextGround(gnd))!=NULL)
			{
				if(i==0 && gnd->primaryTarget!=YSTRUE)
				{
					continue;
				}
				if(targetAny!=YSTRUE && gnd->Prop().chType!=FSTANK && gnd->Prop().chType!=FSNAVYSHIP)
				{
					continue;
				}
				if(gnd->Prop().IsNonGameObject()==YSTRUE)
				{
					continue;
				}

				if(gnd->Prop().IsAlive()==YSTRUE && (gnd->iff!=this->iff || prop.TargetAny()==YSTRUE))
				{
					double tDist;
					tDist=((gnd->GetPosition())-GetPosition()).GetSquareLength();

					if(tDist<dist)
					{
						gndTrg=gnd;
						dist=tDist;
					}
				}
			}

			if(gndTrg!=NULL)
			{
				break;
			}
		}
		prop.SetGroundTarget(gndTrg);
	}


	FsAirplane *air,*airTrg;

	dist=10000000000.0;
	airTrg=NULL;
	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		if(0!=(air->airFlag&FSAIRFLAG_DONTATTACKIFGROUNDSTATIC) && air->Prop().GetFlightState()==FSGROUNDSTATIC)
		{
			continue;
		}

		if(air->Prop().IsActive()==YSTRUE && (air->iff!=this->iff || prop.TargetAny()==YSTRUE))
		{
			double tDist;
			tDist=((air->GetPosition())-GetPosition()).GetSquareLength();

			if(tDist<dist)
			{
				airTrg=air;
				dist=tDist;
			}
		}
	}
	prop.SetAirTarget(airTrg);
}

void FsGround::ApplyControlAndGetFeedback(FsFlightControl &userInput,FSUSERCONTROL userControl,YSBOOL /*autoRudder*/)
{
	if(YSTRUE!=isPlayingRecord && FSUSC_ENABLE==userControl)
	{
		Prop().ApplyControl(userInput,FSAPPLYCONTROL_ALL);
	}
}

YSBOOL FsGround::LockOn(FsSimulation *sim,const double &radarAltLimit)
{
	const YsVec3 *pos;
	const YsAtt3 *att;
	double radar,aamAngle,agmAngle;

	pos=&GetPosition();
	switch(Prop().GetWeaponOfChoice())
	{
	default:
		att=&GetAttitude();
		break;
	case FSWEAPON_GUN:
		att=&Prop().GetAaaAim();
		break;
	case FSWEAPON_AIM9:
	case FSWEAPON_AGM65:
		att=&Prop().GetSamAim();
		break;
	}


	YsMatrix4x4 mat;
	mat.Translate(*pos);
	mat.Rotate(*att);
	mat.Invert();

	aamAngle=YsPi/3.0;
	agmAngle=YsPi/3.0;

	// Air Target
	{
		FsAirplane *airTarget;
		FsAirplane *air;

		radar=YsPi/2.0;
		airTarget=NULL;

		air=NULL;
		while((air=sim->FindNextAirplane(air))!=NULL)
		{
			if(air->IsAlive()==YSTRUE && air->iff!=this->iff)
			{
				double altLimit;
				altLimit=radarAltLimit+1000.0*(1.0-air->Prop().GetRadarCrossSection());
				if(((*pos)-air->GetPosition()).GetSquareLength()<1000.0*1000.0 || // Condition added 2008/02/10
				   pos->y()<air->GetPosition().y() ||                             // Condition added 2008/02/10
				   altLimit<air->GetPosition().y())                               // Condition added 2008/02/10
				{
					YsVec3 tpos;
					tpos=mat*air->GetPosition();

					if(tpos.z()>0.0)
					{
						double r,sqDist,rcs,aamRange;
						r=sqrt(tpos.x()*tpos.x()+tpos.y()*tpos.y());
						r=atan2(r,tpos.z());

						sqDist=tpos.GetSquareLength();
						rcs=air->Prop().GetRadarCrossSection();
						aamRange=Prop().GetSAMRange();
						aamRange*=rcs;

						if(r<aamAngle && r<radar && sqDist<aamRange*aamRange)
						{
							radar=r;
							airTarget=air;
						}
					}
				}
			}
		}

		Prop().SetAirTarget(airTarget);
	}

	// Ground Target
	{
		FsGround *gnd;
		FsGround *gndTarget;
		gndTarget=NULL;

		radar=YsPi/2.0;

		gnd=NULL;
		while((gnd=sim->FindNextGround(gnd))!=NULL)
		{
			if(gnd->Prop().IsAlive()==YSTRUE && gnd->iff!=this->iff && gnd->Prop().IsNonGameObject()!=YSTRUE)
			{
				YsVec3 tpos;
				gnd->Prop().GetPosition(tpos);

				tpos=mat*tpos;

				if(tpos.z()>0.0)
				{
					double r=sqrt(tpos.x()*tpos.x()+tpos.y()*tpos.y());
					r=atan2(r,tpos.z());
					if(r<agmAngle && r<radar)
					{
						radar=r;
						gndTarget=gnd;
					}
				}
			}
		}

		Prop().SetGroundTarget(gndTarget);
	}

	return YSTRUE;
}

FSWEAPONTYPE FsGround::GetWeaponOfChoice(void) const
{
	return Prop().GetWeaponOfChoice();
}

YSBOOL FsGround::GetDamage(YSBOOL &killed,int dmg,FSDIEDOF /*diedOf*/)
{
	return prop.GetDamage(killed,dmg);
}

void FsGround::Bounce(const YsVec3 &collPos)
{
	if(YsTolerance<Prop().GetMaxSpeed())
	{
		YsVec3 bounceDir=Prop().GetPosition()-collPos;
		bounceDir.SetY(0.0);
		if(YSOK==bounceDir.Normalize())
		{
			const double outRad=Prop().GetOutsideRadius();
			const YsVec3 newPos=Prop().GetPosition()+bounceDir*outRad/10.0;
			Prop().SetPosition(newPos);
			Prop().SetVelocity(YsOrigin());
		}
	}
}

YSRESULT FsGround::RipOffEarlyPartOfRecord(void)
{
	if(rec!=NULL)
	{
		rec->RipOffEarlyPartOfRecord();
	}
	return YSOK;
}

int FsGround::RerecordByNewInterval(const double &itvl)
{
	FsRecord <FsGroundRecord> *newRec;
	int n;

	n=FsRerecordByNewInterval <FsGroundRecord> (newRec,rec,itvl);
	delete rec;
	rec=newRec;

	return n;
}

void FsGround::AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng)
{
	FsAdjustPrecisionOfFlightRecord <FsGroundRecord> (rec,precPos,precAng);
}

YSRESULT FsGround::SendCommand(const char str[])
{
	YsString cmd;
	cmd.Set(str);
	cmdLog.Append(cmd);
	return prop.SendCommand(str);
}

YSRESULT FsGround::RecallCommand(void)
{
	int i;
	for(i=0; i<cmdLog.GetN(); i++)
	{
		prop.SendCommand(cmdLog[i]);
	}
	return YSOK;
}

