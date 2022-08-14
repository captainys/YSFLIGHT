#include <ysclass.h>
#include <yscompilerwarning.h>

#include "fs.h"

FsRotatingTurretProperty::FsRotatingTurretProperty()
{
	Initialize();
}

void FsRotatingTurretProperty::Initialize(void)
{
	cen=YsOrigin();
	att=YsZeroAtt();
	radius=1.0;

	hMin=-YsPi*2.0;
	hMax= YsPi*2.0;

	pMin=0.0;
	pMax=YsPi/6.0;

	hZero=0.0;
	pZero=0.0;

	vp=YsPi;
	vh=YsPi;

	maxNumGunBullet=1000;
	destructivePower=1;
	wpnType=FSWEAPON_GUN;

	dnm[0]=NULL;
	dnm[1]=NULL;
	dnm[2]=NULL;
	dnmNodeId[0]=-1;
	dnmNodeId[1]=-1;
	dnmNodeId[2]=-1;

	dnmHdg[0]=NULL;
	dnmHdg[1]=NULL;
	dnmHdg[2]=NULL;
	dnmHdgNodeId[0]=-1;
	dnmHdgNodeId[1]=-1;
	dnmHdgNodeId[2]=-1;

	dnmPch[0]=NULL;
	dnmPch[1]=NULL;
	dnmPch[2]=NULL;
	dnmPchNodeId[0]=-1;
	dnmPchNodeId[1]=-1;
	dnmPchNodeId[2]=-1;

	shootInterval=0.5;

	turretProp=0;

	range=3000.0;
	bulSpeed=340.0*5.0;
}

FsRotatingTurretState::FsRotatingTurretState()
{
	Initialize();
}

void FsRotatingTurretState::Initialize(void)
{
	airTargetKey=YSNULLHASHKEY;
	gndTargetKey=YSNULLHASHKEY;
	numBullet=1000;
	h=0.0;
	p=0.0;
	shootGunTimer=0.0;
	turretState=0;
	ctlH=0.0;
	ctlP=0.0;
};

void FsRotatingTurretState::MoveGunner(
    const FsRotatingTurretProperty &chTurret,
    const double &dt,FSIFF iff,
    YSSIZE_T nAirTgt,const FsAirplane * const airTgt[],
    YSSIZE_T nGndTgt,const FsGround * const gndTgt[],
    YSBOOL gunnerFirePermission,
    const YsMatrix4x4 &invMat)
{
	YsDisregardVariable(dt);

	const FsAirplane *airTgtInRange;
	const FsGround *gndTgtInRange;
	int j;
	YsVec3 relPos;
	YsAtt3 relAtt,attAir,attGnd;
	double airDist=YsSqr(YsInfinity),gndDist=YsSqr(YsInfinity);
	double radarCrossSection=1.0;

	relAtt.SetB(0.0);

	double sqDist,minSqDist=YsSqr(YsInfinity);
	airTgtInRange=NULL;
	gndTgtInRange=NULL;

	if(chTurret.turretProp&FSTURRETPROP_ANTIAIR)
	{
		minSqDist=YsSqr(YsInfinity);
		for(j=0; j<nAirTgt; j++)
		{
			if(airTgt[j]->Prop().IsActive()==YSTRUE && airTgt[j]->iff!=iff)
			{
				invMat.Mul(relPos,airTgt[j]->GetPosition(),1.0);
				sqDist=relPos.GetSquareLength();
				if(sqDist<minSqDist)
				{
					chTurret.att.MulInverse(relPos,relPos-chTurret.cen);
					relAtt.SetForwardVector(relPos);

					if(chTurret.hMin<=relAtt.h() && relAtt.h()<chTurret.hMax &&
					   chTurret.pMin<=relAtt.p() && relAtt.p()<chTurret.pMax)
					{
						attAir=relAtt;
						airDist=sqDist;
						minSqDist=sqDist;
						airTgtInRange=airTgt[j];
					}
				}
			}
		}
	}

	if(chTurret.turretProp&FSTURRETPROP_ANTIGND)
	{
		minSqDist=YsSqr(YsInfinity);
		for(j=0; j<nGndTgt; j++)
		{
			if(gndTgt[j]->IsAlive()==YSTRUE && gndTgt[j]->iff!=iff)
			{
				YsVec3 tgtPos;

				gndTgt[j]->GetWhereToAim(tgtPos);

				invMat.Mul(relPos,tgtPos,1.0);
				sqDist=relPos.GetSquareLength();

				if(sqDist<minSqDist)
				{
					chTurret.att.MulInverse(relPos,relPos-chTurret.cen);
					relAtt.SetForwardVector(relPos);

					if(chTurret.hMin<=relAtt.h() && relAtt.h()<chTurret.hMax &&
					   chTurret.pMin<=relAtt.p() && relAtt.p()<chTurret.pMax)
					{
						attGnd=relAtt;
						gndDist=sqDist;
						minSqDist=sqDist;
						gndTgtInRange=gndTgt[j];
					}
				}
			}
		}
	}

	if(airTgtInRange!=NULL && gndTgtInRange!=NULL)
	{
		if(airDist<gndDist)
		{
			gndTgtInRange=NULL;
		}
		else
		{
			airTgtInRange=NULL;
		}
	}


	airTargetKey=YSNULLHASHKEY;
	gndTargetKey=YSNULLHASHKEY;

	if(NULL!=airTgtInRange)
	{
		double t;
		YsVec3 pos,vel;
		airDist=airDist;
		t=sqrt(airDist)/chTurret.bulSpeed;
		airTgtInRange->Prop().GetVelocity(vel);
		pos=airTgtInRange->GetPosition()+vel*t;

		invMat.Mul(pos,pos,1.0);
		chTurret.att.MulInverse(pos,pos-chTurret.cen);
		relAtt.SetForwardVector(pos);

		this->ctlH=relAtt.h();
		this->ctlP=relAtt.p();
		minSqDist=airDist;

		radarCrossSection=airTgtInRange->Prop().GetRadarCrossSection();

		airTargetKey=FsExistence::GetSearchKey(airTgtInRange);
	}
	else if(gndTgtInRange!=NULL)
	{
		this->ctlH=attGnd.h();
		this->ctlP=attGnd.p();
		minSqDist=gndDist;

		gndTargetKey=FsExistence::GetSearchKey(gndTgtInRange);
	}
	else
	{
		this->ctlH=0.0;   // 2005/09/21
		this->ctlP=0.0;   // 2005/09/21
	}

	if(gunnerFirePermission==YSTRUE && (airTgtInRange!=NULL || gndTgtInRange!=NULL))
	{
		double hErr,pErr;
		hErr=YsDegToRad((double)(rand()%400-200)/100.0);
		pErr=YsDegToRad((double)(rand()%400-200)/100.0);
		this->ctlH+=hErr;
		this->ctlP+=pErr;

		this->ctlH=YsBound(this->ctlH,chTurret.hMin,chTurret.hMax);
		this->ctlP=YsBound(this->ctlP,chTurret.pMin,chTurret.pMax);

		if(minSqDist<YsSqr(chTurret.range*radarCrossSection) &&
		   fabs(this->ctlH-this->h)<YsPi/6.0 &&
		   fabs(this->ctlP-this->p)<YsPi/6.0)
		{
			this->turretState|=FSTURRETSTATE_FIRING;
		}
		else
		{
			this->turretState&=~FSTURRETSTATE_FIRING;
		}
	}
	else
	{
		this->turretState&=~FSTURRETSTATE_FIRING;
	}
}

void FsRotatingTurretState::SetupVisual(FsVisualDnm &vis,const FsRotatingTurretProperty &chTurret) const
{
	int j;
	for(j=0; j<3; j++)
	{
		if(chTurret.dnm[j]==&vis)
		{
			if(0<=chTurret.dnmNodeId[j])
			{
				vis.SetAngleByDnmNodeNumber(chTurret.dnmNodeId[j],this->h,this->p);
			}
			goto HEADINGROTATION;
		}
	}
	for(j=0; j<3; j++)
	{
		if(chTurret.dnm[j]==NULL)
		{
			chTurret.dnmNodeName.Capitalize();
			chTurret.dnm[j]=&vis;
			chTurret.dnmNodeId[j]=vis.GetDnmNodeNumberFromName(chTurret.dnmNodeName);
			vis.SetAngleByDnmNodeNumber(chTurret.dnmNodeId[j],this->h,this->p);
			break;
		}
	}

HEADINGROTATION:

	for(j=0; j<3; j++)
	{
		if(chTurret.dnmHdg[j]==&vis)
		{
			if(0<=chTurret.dnmHdgNodeId[j])
			{
				vis.SetAngleByDnmNodeNumber(chTurret.dnmHdgNodeId[j],this->h,0.0);
			}
			goto PITCHROTATION;
		}
	}
	for(j=0; j<3; j++)
	{
		if(chTurret.dnmHdg[j]==NULL)
		{
			chTurret.dnmNodeName.Capitalize();
			chTurret.dnmHdg[j]=&vis;
			chTurret.dnmHdgNodeId[j]=vis.GetDnmNodeNumberFromName(chTurret.dnmHdgNodeName);
			vis.SetAngleByDnmNodeNumber(chTurret.dnmHdgNodeId[j],this->h,0.0);
			break;
		}
	}

PITCHROTATION:

	for(j=0; j<3; j++)
	{
		if(chTurret.dnmPch[j]==&vis)
		{
			if(0<=chTurret.dnmPchNodeId[j])
			{
				vis.SetAngleByDnmNodeNumber(chTurret.dnmPchNodeId[j],0.0,this->p);
			}
			goto NEXTTURRET;
		}
	}
	for(j=0; j<3; j++)
	{
		if(chTurret.dnmPch[j]==NULL)
		{
			chTurret.dnmNodeName.Capitalize();
			chTurret.dnmPch[j]=&vis;
			chTurret.dnmPchNodeId[j]=vis.GetDnmNodeNumberFromName(chTurret.dnmPchNodeName);
			vis.SetAngleByDnmNodeNumber(chTurret.dnmPchNodeId[j],0.0,this->p);
			break;
		}
	}

NEXTTURRET:
	;
}

void FsRotatingTurretState::FireWeapon(
	    const FsRotatingTurretProperty &chTurret,
	    const double &ctime,
	    const double &dt,
		const YsVec3 &, // iniVel
	    const YsMatrix4x4 &posiMat,
	    class FsSimulation *,
	    class FsWeaponHolder &bul,
	    class FsExistence *owner)
{
	if(this->numBullet>0 && (this->turretState&FSTURRETSTATE_FIRING)!=0)
	{
		this->shootGunTimer-=dt;
		while(this->shootGunTimer<=0.0)
		{
			YsVec3 gun,dir;
			YsAtt3 att;

			gun=posiMat*chTurret.cen;

			att.Set(this->h,this->p,0.0);
			dir=att.GetForwardVector();

			chTurret.att.Mul(dir,dir);

			posiMat.Mul(dir,dir,0.0);

			att.SetForwardVector(dir);
			gun+=dir*chTurret.radius;

			switch(chTurret.wpnType)
			{
			default:
				break;
			case FSWEAPON_GUN:
				bul.Fire
				   (ctime,gun,att,
				    chTurret.bulSpeed,
				    chTurret.range,
				    chTurret.destructivePower,owner,YSTRUE,YSTRUE);
				break;
			case FSWEAPON_AIM9:
			case FSWEAPON_AGM65:
			case FSWEAPON_ROCKET:
			case FSWEAPON_AIM120:
				{
					YSHASHKEY targetKey=YSNULLHASHKEY;

					if(airTargetKey!=YSNULLHASHKEY)
					{
						targetKey=airTargetKey;
					}
					else if(gndTargetKey!=YSNULLHASHKEY)
					{
						targetKey=gndTargetKey;
					}

					bul.Fire(
					    ctime,
					    chTurret.wpnType,
					    gun,
					    att,
					    1.0,
					    800.0,     // Max Speed
					    chTurret.range,
					    YsPi/2.0,  // Mobility
					    YsPi/4.0,  // Radar Range
					    chTurret.destructivePower,
					    owner,
					    targetKey,
					    YSTRUE,YSTRUE);
				}
				break;
			case FSWEAPON_BOMB:
			case FSWEAPON_BOMB250:
				bul.Bomb(
				    ctime,
				    chTurret.wpnType,
				    gun,att,
				    YsOrigin(),
				    340.0, // Max Speed
				    chTurret.destructivePower,owner,YSTRUE,YSTRUE);
				break;

			case FSWEAPON_FLARE:
				bul.DispenseFlare(ctime,gun,YsOrigin(),120.0,1000.0,owner,YSTRUE,YSTRUE);
				break;
			}

			this->numBullet--;

			this->shootGunTimer+=chTurret.shootInterval;
		}
	}
	else
	{
		this->shootGunTimer=0.0;
	}
}

