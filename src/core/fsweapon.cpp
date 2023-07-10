#include <ysclass.h>
#include <ysglparticlemanager.h>

#include <fsairproperty.h>

#include "fs.h"
#include "fsnetutil.h"
#include "graphics/common/fsopengl.h"
#include "fsinstreading.h"


////////////////////////////////////////////////////////////

int FsKillCredit::GetAntiAirScore(YsString &msg,const FsSimulation *,const FsExistence *player) const
{
	msg.Set("");
	if(NULL==whoKilled || NULL==whom)
	{
		return 0;
	}

	YSBOOL playerResponsible=YSFALSE;
	if(creditOwner==FSWEAPON_CREDIT_OWNER_PLAYER)
	{
		playerResponsible=YSTRUE;
	}
	else if(creditOwner==FSWEAPON_CREDIT_OWNER_NOT_RECORDED && player==whoKilled)
	{
		playerResponsible=YSTRUE;
	}

	FsAirplane *victim=(FsAirplane *)whom;
	if(YSTRUE==playerResponsible && FSEX_AIRPLANE==victim->GetType())
	{
		if(whoKilled->iff==whom->iff)
		{
			msg.Set("Killed Friendly Airplane   -100");
			return -100;   // You killed your friendly!
		}
		else
		{
			unsigned score=0;

			msg.Append("Killed ");
			msg.Append(victim->Prop().GetIdentifier());

			if(FSEX_AIRPLANE==whoKilled->GetType())
			{
				const double alt=where.y();
				if(alt<909.1)  // 3300ft or less
				{
					msg.Append("@Low Altitude(10) ");
					score=10;
				}
				else if(alt>3300.0)
				{
					msg.Append("@High Altitude(30) ");
					score=30;
				}
				else
				{
					msg.Append("@Mid Altitude(20) ");
					score=20;
				}
			}
			else
			{
				switch(victim->Prop().GetAirplaneCategory())
				{
				default:
					msg.Append("Non-Combat Airplane(5) ");
					score=5;
					break;
				case FSAC_TRAINER:
					msg.Append("Trainer (10) ");
					score=10;
					break;
				case FSAC_FIGHTER:
				case FSAC_WW2FIGHTER:
					msg.Append("Fighter (20) ");
					score=20;
					break;
				case FSAC_ATTACKER:
				case FSAC_WW2ATTACKER:
				case FSAC_WW2DIVEBOMBER:
					msg.Append("Fighter (30) ");
					score=30;
					break;
				case FSAC_HEAVYBOMBER:
				case FSAC_WW2BOMBER:
					msg.Append("Bomber (40) ");
					score=40;
					break;
				}
			}

			switch(byWhatKindOfWeapon)
			{
			case FSWEAPON_GUN:
				msg.Append(" By GUN (Times 1.5) ");
				score=score*3/2;
				break;
			case FSWEAPON_AGM65:
				msg.Append(" By AGM (Times 2.0) ");
				score=score*2;
				break;
			case FSWEAPON_BOMB:
			case FSWEAPON_BOMB250:
			case FSWEAPON_BOMB500HD:
				msg.Append(" By Bomb (Times 1.2) ");
				score=score*6/5;
				break;
			case FSWEAPON_AIM9:
			case FSWEAPON_AIM9X:
			case FSWEAPON_AIM120:
				msg.Append(" By AAM (Times 1.0) ");
				break;
			case FSWEAPON_ROCKET:
				msg.Append(" By Rocket (Times 2.0) ");
				score=score*2;
				break;
			default:
				msg.Append(" By ? (Times 1.0) ");
				break;
			}

			char sco[256];
			sprintf(sco,"= %d",score);
			msg.Append(sco);

			return score;
		}
	}
	return 0;
}

int FsKillCredit::GetAntiGroundScore(YsString &msg,const FsSimulation *,const FsExistence *player) const
{
	msg.Set("");
	if(NULL==whoKilled || NULL==whom)
	{
		return 0;
	}

	YSBOOL playerResponsible=YSFALSE;
	if(creditOwner==FSWEAPON_CREDIT_OWNER_PLAYER)
	{
		playerResponsible=YSTRUE;
	}
	else if(creditOwner==FSWEAPON_CREDIT_OWNER_NOT_RECORDED && player==whoKilled)
	{
		playerResponsible=YSTRUE;
	}

	const FsGround *victim=(FsGround *)whom;
	if(YSTRUE==playerResponsible && FSEX_GROUND==victim->GetType())
	{
		if(whoKilled->iff==whom->iff)
		{
			msg.Append("Destroyed Friendly Facility    -200");
			return -200;   // You killed your friendly!
		}
		else
		{
			unsigned score=0;

			msg.Append("Destroyed ");
			msg.Append(victim->Prop().GetIdentifier());


			const double rad=whom->GetApproximatedCollideRadius();
			if(rad<20.0)
			{
				msg.Append("(5) ");
				score=5;
			}
			else if(rad<100.0)
			{
				msg.Append("(10) ");
				score=10;
			}
			else
			{
				msg.Append("(20) ");
				score=20;
			}

			switch(byWhatKindOfWeapon)
			{
			case FSWEAPON_GUN:
				msg.Append(" By GUN (Times 1.0) ");
				break;
			case FSWEAPON_AGM65:
				msg.Append(" By AGM (Times 1.2) ");
				score=score*6/5;
				break;
			case FSWEAPON_BOMB:
			case FSWEAPON_BOMB250:
			case FSWEAPON_BOMB500HD:
				msg.Append(" By Bomb (Times 1.5) ");
				score=score*3/2;
				break;
			case FSWEAPON_ROCKET:
				msg.Append(" By Rocket (Times 1.5) ");
				score=score*3/2;
				break;
			case FSWEAPON_AIM9:
			case FSWEAPON_AIM9X:
			case FSWEAPON_AIM120:
				msg.Append(" By AAM (Times 1.0) ");
				score=score*6/5;
				break;

			default:
				msg.Append(" By ? (Times 1.0) ");
				break;
			}

			char sco[256];
			sprintf(sco,"=%d",score);
			msg.Append(sco);

			return score;
		}
	}
	return 0;
}


////////////////////////////////////////////////////////////

void FsWeaponSmokeTrail::Initialize(void)
{
	trailBegin=0;
	trailUsed=0;
	trailNextRecord=double(TIMEPERSEG)/1000.0;
}

void FsWeaponSmokeTrail::MakeVertexArray(
    YsGLVertexBuffer vtxBuf[4],YsGLNormalBuffer nomBuf[4],YsGLColorBuffer colBuf[4],
    const YsVec3 &pos,const YsAtt3 &att,FSSMOKETYPE smk,const double &cTime) const
{
	int a;
	double w;
	YsVec3 v,v1,v2,n,n1,n2,refV1,refV2,prev;
	int iAlpha;

	for(int i=0; i<4; ++i)
	{
		vtxBuf[i].CleanUp();
		nomBuf[i].CleanUp();
		colBuf[i].CleanUp();
	}

	switch(smk)
	{
	default:
		break;
	case FSSMKSOLID:
		refV1.Set(-1.0, 0.0,0.0);
		refV2.Set( 0.0, 1.0,0.0);
		for(a=0; a<4; a++)
		{
			prev=pos;
			if(trailUsed>0 && prev!=trailPos[(trailBegin+trailUsed-1)%MAXNUMTRAIL])
			{
				v1=refV1*0.1;
				v1.SetZ(-1.5);
				v2=refV2*0.1;
				v2.SetZ(-1.5);
				att.Mul(v1,v1);
				v1+=pos;
				att.Mul(v2,v2);
				v2+=pos;

				att.Mul(n1,refV1);
				att.Mul(n2,refV2);

				colBuf[a].Add<float>(1.0f,1.0f,1.0f,0.7f);
				nomBuf[a].Add(n2);
				vtxBuf[a].Add(v2);

				colBuf[a].Add<float>(1.0f,1.0f,1.0f,0.7f);
				nomBuf[a].Add(n1);
				vtxBuf[a].Add(v1);
			}

			iAlpha=MAXNUMTRAIL-1;
			for(int i=trailUsed-1; i>=0; i--)
			{
				iAlpha--;

				const int idx=(trailBegin+i)%MAXNUMTRAIL;

				if(prev==trailPos[idx])
				{
					continue;
				}
				prev=trailPos[(trailBegin+i)%MAXNUMTRAIL];

				float alpha=1.0f-(float)(cTime-trailTime[idx])/(float)((TIMEPERSEG*MAXNUMTRAIL-500)/1000);
				alpha=YsBound(alpha,0.0f,0.7f);

				w=1.0*double(trailUsed-i);
				w=YsSmaller(w,2.5);

				/* Known working set, looks to me an error. >>
				v1=refV1*w; // v1.Set(-w,0.0,0.0);
				v2=refV2*w; // v2.Set( w,0.0,0.0);
				trailAtt[idx].Mul(v1,v1);
				v1+=trailPos[idx];
				trailAtt[idx].Mul(v2,v2);
				v2+=trailPos[idx];

				att.Mul(n1,refV1); // att.Mul(n,YsYVec());
				att.Mul(n2,refV2); */

				trailAtt[idx].Mul(n1,refV1);
				trailAtt[idx].Mul(n2,refV2);
				v1=trailPos[idx]+n1*w;
				v2=trailPos[idx]+n2*w;

				colBuf[a].Add<float>(1.0f,1.0f,1.0f,alpha);
				nomBuf[a].Add(n2);
				vtxBuf[a].Add(v2);

				colBuf[a].Add<float>(1.0f,1.0f,1.0f,alpha);
				nomBuf[a].Add(n1);
				vtxBuf[a].Add(v1);
			}

			double xx,yy;
			xx=refV1.x();
			yy=refV1.y();
			refV1.Set(yy,-xx,0.0);
			xx=refV2.x();
			yy=refV2.y();
			refV2.Set(yy,-xx,0.0);
		}
		break;

	case FSSMKNOODLE:
	case FSSMKCIRCLE:
	case FSSMKTOWEL:
		prev=pos;
		if(trailUsed>0 && prev!=trailPos[(trailBegin+trailUsed-1)%MAXNUMTRAIL])
		{
			v1.Set(-0.1,0.0,-1.5);
			v2.Set( 0.1,0.0,-1.5);
			att.Mul(v1,v1);
			v1+=pos;
			att.Mul(v2,v2);
			v2+=pos;
			att.Mul(n,YsYVec());

			colBuf[0].Add<float>(1.0f,1.0f,1.0f,0.7f);
			nomBuf[0].Add(n);
			vtxBuf[0].Add(v2);

			colBuf[0].Add<float>(1.0f,1.0f,1.0f,0.7f);
			nomBuf[0].Add(n);
			vtxBuf[0].Add(v1);
		}

		iAlpha=MAXNUMTRAIL-1;
		for(int i=trailUsed-1; i>=0; i--)
		{
			iAlpha--;

			const int idx=(trailBegin+i)%MAXNUMTRAIL;

			if(prev==trailPos[idx])
			{
				continue;
			}
			prev=trailPos[(trailBegin+i)%MAXNUMTRAIL];

			float alpha=1.0f-(float)(cTime-trailTime[idx])/(float)((TIMEPERSEG*MAXNUMTRAIL-500)/1000);
			alpha=YsBound(alpha,0.0f,0.7f);

			w=1.0*double(trailUsed-i);
			w=YsSmaller(w,2.5);
			v1.Set(-w,0.0,0.0);
			v2.Set( w,0.0,0.0);
			trailAtt[idx].Mul(v1,v1);
			v1+=trailPos[idx];
			trailAtt[idx].Mul(v2,v2);
			v2+=trailPos[idx];
			att.Mul(n,YsYVec());

			colBuf[0].Add<float>(1.0f,1.0f,1.0f,alpha);
			nomBuf[0].Add(n);
			vtxBuf[0].Add(v2);

			colBuf[0].Add<float>(1.0f,1.0f,1.0f,alpha);
			nomBuf[0].Add(n);
			vtxBuf[0].Add(v1);
		}
		break;
	}
}

// The following two functions are moved to fsweapongl.cpp and fsweaponbi.cpp
// void FsWeaponSmokeTrail::Draw(const YsVec3 &pos,const YsAtt3 &att)
// void FsWeaponSmokeTrail::DrawFlare(const YsVec3 &pos,const YsAtt3 &att)

void FsWeaponSmokeTrail::Add(const double &dt,const double &cTime,const YsVec3 &pos,const YsAtt3 &att)
{
	trailNextRecord-=dt;
	if(trailNextRecord<=0.0)
	{
		trailNextRecord=double(TIMEPERSEG)/1000.0;
		trailPos[(trailBegin+trailUsed)%MAXNUMTRAIL]=pos;
		trailAtt[(trailBegin+trailUsed)%MAXNUMTRAIL]=att;
		trailTime[(trailBegin+trailUsed)%MAXNUMTRAIL]=cTime;
		if(trailUsed<MAXNUMTRAIL)
		{
			trailUsed++;
		}
		else
		{
			trailBegin=(trailBegin+1)%MAXNUMTRAIL;
		}
	}
}

void FsWeaponSmokeTrail::AddToParticleManagerAsFlare(class YsGLParticleManager &partMan,const YsVec3 cPos,const double cTime,YSBOOL includeCurrentPos)
{
	int c=0;
	YSSIZE_T i0=trailUsed;
	if(YSTRUE!=includeCurrentPos)
	{
		--i0;
	}
	for(YSSIZE_T i=i0; i>0; i--)
	{
		const int idx0=(trailBegin+i)%MAXNUMTRAIL;
		const int idx1=(trailBegin+i-1)%MAXNUMTRAIL;

		const double t0=(i==trailUsed ? 0.0 : cTime-trailTime[idx0]);
		const double t1=cTime-trailTime[idx1];

		const auto p0=(i==trailUsed ? cPos : trailPos[idx0]);
		const auto p1=trailPos[idx1];

		float sz=0.5f+(float)(t0*4.0);
		if(16.0f<sz)
		{
			sz=16.0f;
		}

		int nDiv=3+(int)((p0-p1).GetLength()/sz);
		for(int i=0; i<nDiv; ++i)
		{
			const double param=(double)i/(double)nDiv;

			const double t=t0*(1.0-param)+t1*param;
			const auto pos=p0*(1.0-param)+p1*param;


			YsColor col;
			// Color
			// t=0   -> Red
			// t=0.1 -> Red
			// t=0.3 -> White
			// Alpha
			// t<5   -> 1.0
			// t=8   -> 0.0
			double r,g,b,a;
			if(t<0.1)
			{
				r=1.0;
				g=t*10.0;
				b=t*10.0;
			}
			else
			{
				r=1.0;
				g=1.0;
				b=1.0;
			}
			// t=0.5   a=1.0
			// t=1.0   a=0.3
			// t=5.0   a=0.0
			if(t<0.1)
			{
				a=1.0;
			}
			else if(t<0.3)
			{
				a=1.0-(t-0.1)*4.5;
			}
			else if(t<5.0)
			{
				a=0.1*(5.0-t)/4.7;
			}
			else
			{
				a=0.0;
			}
			col.SetDoubleRGBA(r,g,b,a*0.7);

			float sz=0.5f+(float)(t*4.0);
			if(16.0f<sz)
			{
				sz=16.0f;
			}

			float s=(float)((i+idx0)&7)*0.125;
			partMan.Add(pos,col,sz,s,0);
		}
		c++;
	}
}

void FsWeaponSmokeTrail::AddToParticleManager(class YsGLParticleManager &partMan,const YsVec3 cPos,const double cTime,YSBOOL includeCurrentPos)
{
	int c=0;
	YSSIZE_T i0=trailUsed;
	if(YSTRUE!=includeCurrentPos)
	{
		--i0;
	}
	for(YSSIZE_T i=i0; i>0; i--)
	{
		const int idx0=(trailBegin+i)%MAXNUMTRAIL;
		const int idx1=(trailBegin+i-1)%MAXNUMTRAIL;

		const double t0=(i==trailUsed ? 0.0 : cTime-trailTime[idx0]);
		const double t1=cTime-trailTime[idx1];

		const auto p0=(i==trailUsed ? cPos : trailPos[idx0]);
		const auto p1=trailPos[idx1];

		float sz=0.5f+(float)(t0*20.0);
		if(40.0f<sz)
		{
			sz=40.0f;
		}

		int nDiv=3+(int)((p0-p1).GetLength()/sz);
		for(int i=0; i<nDiv; ++i)
		{
			const double param=(double)i/(double)nDiv;

			const double t=t0*(1.0-param)+t1*param;
			const auto pos=p0*(1.0-param)+p1*param;


			YsColor col;
			// Color
			// t=0   -> Red
			// t=0.1 -> Red
			// t=0.3 -> White
			// Alpha
			// t<5   -> 1.0
			// t=8   -> 0.0
			double r,g,b,a;
			r=0.7;
			g=0.7;
			b=0.7;

			a=1.0f-t/(double)((TIMEPERSEG*MAXNUMTRAIL-500)/1000);
			a=YsBound(a,0.0,0.7);
			col.SetDoubleRGBA(r,g,b,a);

			float sz=0.5f+(float)(t*20.0);
			if(40.0f<sz)
			{
				sz=40.0f;
			}

			float s=(float)((i+idx0)&7)*0.125;
			partMan.Add(pos,col,sz,s,0);
		}
		c++;
	}
}

// Implementation //////////////////////////////////////////
FsVisualDnm FsWeapon::aim9=nullptr;
FsVisualDnm FsWeapon::agm65=nullptr;
FsVisualDnm FsWeapon::bomb=nullptr;
FsVisualDnm FsWeapon::rocket=nullptr;
FsVisualDnm FsWeapon::aim9s=nullptr;
FsVisualDnm FsWeapon::agm65s=nullptr;
FsVisualDnm FsWeapon::rockets=nullptr;
FsVisualDnm FsWeapon::aim_coarse=nullptr;
FsVisualDnm FsWeapon::agm_coarse=nullptr;
FsVisualDnm FsWeapon::bomb_coarse=nullptr;
FsVisualDnm FsWeapon::rocket_coarse=nullptr;

FsVisualDnm FsWeapon::aim120=nullptr;
FsVisualDnm FsWeapon::aim120s=nullptr;
FsVisualDnm FsWeapon::aim120_coarse=nullptr;
FsVisualDnm FsWeapon::bomb250=nullptr;
FsVisualDnm FsWeapon::bomb250s=nullptr;
FsVisualDnm FsWeapon::bomb250_coarse=nullptr;
FsVisualDnm FsWeapon::bomb500hd=nullptr;
FsVisualDnm FsWeapon::bomb500hds=nullptr;
FsVisualDnm FsWeapon::bomb500hd_coarse=nullptr;

FsVisualDnm FsWeapon::aim9x=nullptr;
FsVisualDnm FsWeapon::aim9xs=nullptr;
FsVisualDnm FsWeapon::aim9x_coarse=nullptr;

FsVisualDnm FsWeapon::flarePod=nullptr;
FsVisualDnm FsWeapon::fuelTank=nullptr;


const double FsTrailTimePerSegment=0.1;


FsWeapon::FsWeapon()
{
	lifeRemain=0.0;
	timeRemain=0.0;
	timeUnguided=0.0;
	velocity=0.0;
	pos.Set(0.0,0.0,0.0);
	vec.Set(0.0,0.0,0.0);
	firedBy=NULL;
	creditOwner=FSWEAPON_CREDIT_OWNER_NOT_RECORDED;

	prev=NULL;
	next=NULL;
	prevFlare=NULL;
	nextFlare=NULL;

	trail=NULL;
}

/*static*/ FsAmmunitionIndication::WEAPONTYPE FsWeapon::WeaponTypeToWeaponIndicationType(FSWEAPONTYPE wpnType)
{
	switch(wpnType)
	{
	default:
		return FsAmmunitionIndication::WPNTYPE_NULL;
	case FSWEAPON_GUN:
		return FsAmmunitionIndication::WPNTYPE_GUN;
	case FSWEAPON_AIM9:
		return FsAmmunitionIndication::WPNTYPE_AIM9;
	case FSWEAPON_AGM65:
		return FsAmmunitionIndication::WPNTYPE_AGM65;
	case FSWEAPON_BOMB:
		return FsAmmunitionIndication::WPNTYPE_BOMB;
	case FSWEAPON_ROCKET:
		return FsAmmunitionIndication::WPNTYPE_ROCKET;
	case FSWEAPON_FLARE:
	case FSWEAPON_FLARE_INTERNAL:
		return FsAmmunitionIndication::WPNTYPE_FLARE;
	case FSWEAPON_AIM120:
		return FsAmmunitionIndication::WPNTYPE_AIM120;
	case FSWEAPON_BOMB250:
		return FsAmmunitionIndication::WPNTYPE_BOMB250;
	case FSWEAPON_SMOKE:
		return FsAmmunitionIndication::WPNTYPE_SMOKE;
	case FSWEAPON_BOMB500HD:
		return FsAmmunitionIndication::WPNTYPE_BOMB500HD;
	case FSWEAPON_AIM9X:
		return FsAmmunitionIndication::WPNTYPE_AIM9X;
	case FSWEAPON_FUELTANK:
		return FsAmmunitionIndication::WPNTYPE_FUELTANK;
	}
}

/* static */void FsWeapon::DrawVisual
	   (FSWEAPONTYPE wpnType,YSBOOL coarse,
	   const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,
	   const YsVec3 &pos,const YsAtt3 &att,unsigned int /*drawFlag*/)
{
	FsVisualDnm *vis=nullptr;
	switch(wpnType)
	{
	default:
		break;
	case FSWEAPON_AIM120:
		vis=(coarse!=YSTRUE ? &aim120s : &aim120_coarse);
		break;
	case FSWEAPON_AIM9:
		vis=(coarse!=YSTRUE ? &aim9s : &aim_coarse);
		break;
	case FSWEAPON_AIM9X:
		vis=(coarse!=YSTRUE ? &aim9xs : &aim9x_coarse);
		break;
	case FSWEAPON_AGM65:
		vis=(coarse!=YSTRUE ? &agm65s : &agm_coarse);
		break;
	case FSWEAPON_BOMB:
		vis=(coarse!=YSTRUE ? &bomb : &bomb_coarse);
		break;
	case FSWEAPON_BOMB250:
		vis=(coarse!=YSTRUE ? &bomb250 : &bomb250_coarse);
		break;
	case FSWEAPON_BOMB500HD:
		vis=(coarse!=YSTRUE ? &bomb500hds : &bomb500hd_coarse);
		break;
	case FSWEAPON_ROCKET:
		vis=(coarse!=YSTRUE ? &rockets : &rocket_coarse);
		break;
	case FSWEAPON_FLAREPOD:
		vis=&flarePod;
		break;
	case FSWEAPON_FUELTANK:
		vis=&fuelTank;
		break;
	}

	if(vis!=nullptr)
	{
		vis->Draw(viewTfm,projTfm,pos,att,FSVISUAL_DRAWALL);
	}
}
/* static */ void FsWeapon::DrawShadow(
    FSWEAPONTYPE wpnType,YSBOOL coarse,
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &projTfm,
    const YsVec3 &pos,const YsAtt3 &att,const YsMatrix4x4 &projPlnTfm)
{
	FsVisualDnm *vis=nullptr;
	switch(wpnType)
	{
	default:
		break;
	case FSWEAPON_AIM120:
		vis=(coarse!=YSTRUE ? &aim120s : &aim120_coarse);
		break;
	case FSWEAPON_AIM9:
		vis=(coarse!=YSTRUE ? &aim9s : &aim_coarse);
		break;
	case FSWEAPON_AIM9X:
		vis=(coarse!=YSTRUE ? &aim9xs : &aim9x_coarse);
		break;
	case FSWEAPON_AGM65:
		vis=(coarse!=YSTRUE ? &agm65s : &agm_coarse);
		break;
	case FSWEAPON_BOMB:
		vis=(coarse!=YSTRUE ? &bomb : &bomb_coarse);
		break;
	case FSWEAPON_BOMB250:
		vis=(coarse!=YSTRUE ? &bomb250 : &bomb250_coarse);
		break;
	case FSWEAPON_BOMB500HD:
		vis=(coarse!=YSTRUE ? &bomb500hds : &bomb500hd_coarse);
		break;
	case FSWEAPON_ROCKET:
		vis=(coarse!=YSTRUE ? &rockets : &rocket_coarse);
		break;
	case FSWEAPON_FLAREPOD:
		vis=&flarePod;
		break;
	case FSWEAPON_FUELTANK:
		vis=&fuelTank;
		break;
	}

	if(vis!=nullptr)
	{
		vis->DrawShadow(viewTfm,projTfm,pos,att,projPlnTfm);
	}
}

void FsWeapon::Fire(
    const double &/*ctime*/,YsVec3 &p,YsAtt3 &a,const double &v,const double &l,int destruction,
    FsExistence *owner,
    FSWEAPON_CREDIT_OWNER creditOwnerIn)
{
	type=FSWEAPON_GUN;
	vec.Set(0.0,0.0,v);
	a.Mul(vec,vec); // vec=a.GetMatrix()*vec;
	prv=p;
	pos=p;
	lastChecked=p;
	firedBy=owner;
	creditOwner=creditOwnerIn;
	velocity=v;
	lifeRemain=l;
	timeRemain=0.0;
	target=NULL;
	trail=NULL;
	destructivePower=destruction;
}

void FsWeapon::Fire(
    const double &/*ctime*/,
    FSWEAPONTYPE tp,
    const YsVec3 &p,const YsAtt3 &a,
    const double &v,const double &vmax,const double &l,const double &m,const double &r,
    int destruction,
    FsExistence *owner,
    FSWEAPON_CREDIT_OWNER creditOwnerIn,
    FsExistence *t,FsWeaponSmokeTrail *tr)
{
	type=tp;
	prv=p;
	pos=p;
	lastChecked=p;
	att=a;
	lifeRemain=l;
	timeRemain=2.0*double(FsWeaponSmokeTrail::TIMEPERSEG*FsWeaponSmokeTrail::MAXNUMTRAIL)/1000.0;
	        // ^^^ 2.0 : take margine. because the trail buffer may not be updated regulary by TIMEPERSEG

	switch(tp)
	{
	case FSWEAPON_AIM120:
		timeUnguided=3.0;
		break;
	case FSWEAPON_AIM9X:
		timeUnguided=0.5;
		break;
	case FSWEAPON_AGM65:
		timeUnguided=1.6;
		break;
	default:
		timeUnguided=0.0;
		break;
	}

	vec.Set(0.0,0.0,v);
	a.Mul(vec,vec); // vec=a.GetMatrix()*vec;

	velocity=v;
	maxVelocity=vmax;
	mobility=m;
	radar=r;
	firedBy=owner;
	creditOwner=creditOwnerIn;
	target=t;

	destructivePower=destruction;

	trail=tr;
	if(tr!=NULL)
	{
		tr->Initialize();
	}
}

void FsWeapon::DispenseFlare(
    const double &/*ctime*/,
    const YsVec3 &p,const YsAtt3 &a,
    const double &v,const double &vMax,const double &l,
    class FsExistence *owner,
    FSWEAPON_CREDIT_OWNER creditOwnerIn,
    FsWeaponSmokeTrail *tr)
{
	type=FSWEAPON_FLARE;

	prv=p;
	pos=p;
	lastChecked=p;
	att=a;
	lifeRemain=l;
	timeRemain=2.0*double(FsWeaponSmokeTrail::TIMEPERSEG*FsWeaponSmokeTrail::MAXNUMTRAIL)/1000.0;
	        // ^^^ 2.0 : take margine. because the trail buffer may not be updated regulary by TIMEPERSEG

	vec.Set(0.0,0.0,v);
	a.Mul(vec,vec); // vec=a.GetMatrix()*vec;

	velocity=vec.GetLength();
	maxVelocity=vMax;
	mobility=0;
	radar=0;
	firedBy=owner;
	creditOwner=creditOwnerIn;
	target=NULL;

	destructivePower=0;

	trail=tr;
	if(tr!=NULL)
	{
		tr->Initialize();
	}
}

void FsWeapon::ThrowDebris(const double &/*ctime*/,const YsVec3 &p,const YsVec3 &v,const double &l)
{
	type=FSWEAPON_DEBRIS;
	prv=p;
	pos=p;
	vec=v;
	lastChecked=p;
	firedBy=NULL;
	creditOwner=FSWEAPON_CREDIT_OWNER_NON_PLAYER;
	velocity=v.GetLength();
	lifeRemain=l;
	timeRemain=0.1;
	target=NULL;
	trail=NULL;
	destructivePower=0;
	att=YsZeroAtt();
}

void FsWeapon::Move(const double &dt,const double &cTime,const FsWeather &weather,const FsWeapon *flareList)
{
	if(lifeRemain>0.0)
	{
		switch(type)
		{
		case FSWEAPON_GUN:
			vec.Set(vec.x(),vec.y()-FsGravityConst*dt,vec.z());
			break;
		case FSWEAPON_BOMB:
		case FSWEAPON_BOMB250:
		case FSWEAPON_FUELTANK:
			vec.Set(vec.x(),vec.y()-FsGravityConst*dt,vec.z());
			att.SetForwardVector(vec);
			break;
		case FSWEAPON_BOMB500HD:
			vec.Set(vec.x(),vec.y()-FsGravityConst*dt,vec.z());
			{
				const double cdS=0.8;
				const double D=0.5*cdS*vec.GetSquareLength()*FsGetAirDensity(pos.y());
				const double m=226.8;
				const double a=D/m;
				YsVec3 dVec=-vec;
				if(YSOK==dVec.Normalize())
				{
					vec+=a*dt*dVec;
				}
			}
			att.SetForwardVector(vec);
			break;
		case FSWEAPON_ROCKET:
			if(velocity<maxVelocity)
			{
				velocity+=50.0*dt;  // 20.0 m/ss (^_^;)
			}
			else if(velocity>maxVelocity)
			{
				velocity-=20.0*dt;
			}
			vec.Set(0.0,0.0,velocity);
			att.Mul(vec,vec); // vec=att.GetMatrix()*vec;
			break;
		case FSWEAPON_AIM9:
		case FSWEAPON_AIM9X:
		case FSWEAPON_AIM120:
		case FSWEAPON_AGM65:
			if(target!=NULL && timeUnguided<YsTolerance)
			{
				YsVec3 tpos;
				YsMatrix4x4 mat;

				mat.Translate(pos);
				mat.Rotate(att);
				mat.Invert();

				tpos=mat*(target->GetPosition());
				if(type==FSWEAPON_AIM9 || type==FSWEAPON_AIM9X || type==FSWEAPON_AIM120) // Extension 2001/11/23
				{
					const FsWeapon *flare;
					YsVec3 flarePos;
					double flareZ;
					YSBOOL fooled;
					fooled=YSFALSE;
					flareZ=lifeRemain;
					for(flare=flareList; flare!=NULL; flare=flare->nextFlare)
					{
						flarePos=mat*flare->pos;
						if(flarePos.z()>0.0 && flarePos.z()<flareZ &&
						   atan2(flarePos.x()*flarePos.x()+flarePos.y()*flarePos.y(),flarePos.z())<radar)
						{
							fooled=YSTRUE;
							tpos=flarePos;
							flareZ=flarePos.z();
						}
					}
				}

				double r;
				r=atan2(sqrt(tpos.x()*tpos.x()+tpos.y()*tpos.y()),tpos.z());
				if(r<radar || ((type==FSWEAPON_AIM9X || type==FSWEAPON_AIM120) && YSTRUE==IsOwnerStillHaveTarget()))
				{
					double maxMovement;
					maxMovement=mobility*dt;

					double yaw,pit;
					yaw=atan2(-tpos.x(),tpos.z());
					pit=atan2( tpos.y(),tpos.z());

					yaw=YsBound(yaw,-maxMovement,maxMovement);
					pit=YsBound(pit,-maxMovement,maxMovement);
					att.YawLeft(yaw);
					att.NoseUp(pit);
				}
				else if(tpos.z()<-300.0)
				{
					target=NULL;
				}
			}

			if(timeUnguided>0.0)
			{
				timeUnguided-=dt;
			}

			if(velocity<maxVelocity)
			{
				velocity+=50.0*dt;  // 20.0 m/ss (^_^;)
			}
			else if(velocity>maxVelocity)
			{
				velocity-=20.0*dt;
			}
			vec.Set(0.0,0.0,velocity);
			att.Mul(vec,vec); // vec=att.GetMatrix()*vec;
			break;
		case FSWEAPON_FLARE:
			{
				vec.Set(vec.x(),vec.y()-FsGravityConst*dt,vec.z());
				att.SetForwardVector(vec);
				velocity=vec.GetLength();
				if(velocity>maxVelocity)
				{
					velocity-=20.0*dt;
					vec.Set(0.0,0.0,velocity);
					att.Mul(vec,vec); // vec=att.GetMatrix()*vec;
				}
			}
			break;
		case FSWEAPON_DEBRIS:
			vec.SubY(FsGravityConst*dt);
			velocity=vec.GetLength();
			att.SetH(att.h()+YsPi*dt);
			att.SetP(att.p()+YsPi*dt*2.0);
			att.SetB(att.b()+YsPi*dt*3.0);
			break;
		}

		prv=pos;
		pos=pos+vec*dt;

		pos+=weather.GetWind()*dt;

		if(type!=FSWEAPON_BOMB && type!=FSWEAPON_BOMB250 && type!=FSWEAPON_BOMB500HD && type!=FSWEAPON_FUELTANK)  // Bomb falls until it hits the ground
		{
			lifeRemain=lifeRemain-velocity*dt;
			if(lifeRemain<=YsTolerance)
			{
				lifeRemain=0.0;
			}
		}
	}
	else if(timeRemain>0.0)
	{
		timeRemain-=dt;
		if(timeRemain<=YsTolerance)
		{
			timeRemain=0.0;
		}
	}

	if(type==FSWEAPON_AIM9 || type==FSWEAPON_AIM9X || type==FSWEAPON_AGM65 || type==FSWEAPON_AIM120 || type==FSWEAPON_FLARE)
	{
		if(nullptr!=trail && lifeRemain>0.0)
		{
			trail->Add(dt,cTime,pos,att);
		}
	}
}

YSBOOL FsWeapon::IsOwnerStillHaveTarget(void)
{
	YSHASHKEY ownerAirTargetKey=YSNULLHASHKEY;
	if(NULL!=firedBy)
	{
		if(FSEX_AIRPLANE==firedBy->GetType())
		{
			ownerAirTargetKey=((FsAirplane *)firedBy)->Prop().GetAirTargetKey();
		}
		else if(FSEX_GROUND==firedBy->GetType())
		{
			ownerAirTargetKey=FsExistence::GetSearchKey(((FsGround *)firedBy)->Prop().GetAirTarget());
		}
	}

	if(NULL!=firedBy && NULL!=target && ownerAirTargetKey==FsExistence::GetSearchKey(target))
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsWeapon::HitGround(
    FsWeaponHolder *callback,
    const double &ctime,
    const FsField &fld,
    FsExplosionHolder *explode,
    FsSimulation *sim,
    YsList <FsKillCredit> *&killCredit)
{
	if(lifeRemain>0.0)
	{
		double elv;
		const YsSceneryItem *evg;
		int collType; // 1:Ground  2:Shell
		YSSCNAREATYPE areaType;
		YsVec3 shellItsc;

		evg=fld.GetFieldElevation(elv,pos.x(),pos.z());

		collType=0;
		areaType=YSSCNAREA_NOAREA;
		if(pos.y()<=elv+YsTolerance)
		{
			collType=1;
			areaType=sim->GetAreaType(pos);
		}
		else if(YSTRUE==fld.GetFieldShellCollision(shellItsc,prv,pos))
		{
			collType=2;
		}

		if(collType!=0)
		{
			lifeRemain=0.0;
			if(explode!=NULL)
			{
				static int step=0;
				switch(type)
				{
				case FSWEAPON_GUN:
					if((step&3)==0)
					{
						// Adjust for elevation grid.
						// Do this intersection-calculation only for guns.
						// The location calculated by the intersection-calculation may
						// be far away from the actual exploding location.  For example,
						// when the bomb hits side wall of the elevation grid.
						// This error may cause unexpected damage to airplanes or
						// grounds.
						if(collType==1)
						{
							YsVec3 cen,nom,crs;
							YsPlane pln;
							cen=pos;
							cen.SetY(elv);

							fld.GetFieldElevationAndNormal(elv,nom,pos.x(),pos.z());
							pln.Set(cen,nom);
							if(pln.GetIntersection(crs,pos,pos-prv)==YSOK)
							{
								pos=crs;
							}
						}
						else if(collType==2)
						{
							pos=shellItsc;
						}

						if(evg==NULL && collType==1 && areaType==YSSCNAREA_WATER)
						{
							explode->WaterPlume(ctime,pos,3.0,1.0,10.0,NULL,YSFALSE);
						}
						else
						{
							explode->Explode(ctime,pos,1.0,0.0,5.0,YSTRUE,NULL,YSFALSE);
						}
					}
					step=(step+1)&255;
					break;
				case FSWEAPON_BOMB:
				case FSWEAPON_BOMB250:
				case FSWEAPON_BOMB500HD:
				case FSWEAPON_FUELTANK:
					if(collType==1)
					{
						pos.SetY(elv);
					}
					else if(collType==2)
					{
						pos=shellItsc;
					}
					if(evg==NULL && collType==1 && areaType==YSSCNAREA_WATER)
					{
						ExplodeBombInWater(callback,ctime,pos,destructivePower*5/10,explode,sim,killCredit);
					}
					else
					{
						ExplodeBomb(callback,ctime,pos,destructivePower*5/2,explode,sim,killCredit);
					}
					break;
				case FSWEAPON_ROCKET:
					if(collType==1)
					{
						pos.SetY(elv);
					}
					else if(collType==2)
					{
						pos=shellItsc;
					}
					if(evg==NULL && collType==1 && areaType==YSSCNAREA_WATER)
					{
						ExplodeBombInWater(callback,ctime,pos,10.0,explode,sim,killCredit);
					}
					else
					{
						ExplodeBomb(callback,ctime,pos,50.0,explode,sim,killCredit);
					}
					break;
				case FSWEAPON_DEBRIS:
					// Do nothing
					break;
				default:
					if(pos.y()<=elv+YsTolerance)
					{
						pos.SetY(elv);
					}
					else if(collType==2)
					{
						pos=shellItsc;
					}
					if(evg==NULL && collType==1 && areaType==YSSCNAREA_WATER)
					{
						explode->WaterPlume(ctime,pos,3.0,4.0,20.0,NULL,YSFALSE);
					}
					else
					{
						explode->Explode(ctime,pos,3.0,0.0,20.0,YSTRUE,NULL,YSFALSE);
					}
					break;
				}
			}
		}
	}
}

YSBOOL FsWeapon::HitObject(
    FsWeaponHolder *callback,
    const double &ctime,
    FsExistence &obj,
    FsExplosionHolder *explosion,
    FsSimulation *sim,
    YsList <FsKillCredit> *&killCredit)
{
	if(lifeRemain>0.0 && firedBy!=&obj)
	{
		const auto &coll=obj.TransformedCollisionShell();

		const YsVec3 *tpos;
		tpos=&obj.GetPosition();

		double sqDist1,sqDist2,sqDist3,sqLastMovedDist,rad;

		rad=obj.GetApproximatedCollideRadius();

		sqDist1=(pos-(*tpos)).GetSquareLength();
		sqDist2=(lastChecked-(*tpos)).GetSquareLength();

		// Proximity tube >>
		if(type==FSWEAPON_AIM9 || type==FSWEAPON_AIM9X || type==FSWEAPON_AIM120)
		{
			double range;
			YsVec3 np;
			YsGetNearestPointOnLine3(np,pos,lastChecked,*tpos);
			sqDist3=(np-(*tpos)).GetSquareLength();

			switch(type)
			{
			default:
			case FSWEAPON_AIM9:
				range=18.0;
				break;
			case FSWEAPON_AIM9X:
				range=23.0;
				break;
			case FSWEAPON_AIM120:
				range=25.0;
				break;
			}

			if(YsCheckInBetween3(np,pos,lastChecked)==YSTRUE && sqDist3<range*range)
			{
				double l,dmg;
				l=sqrt(sqDist3);

				dmg=1.0+(double)destructivePower*(range-l)/range;

				YSBOOL killed;
				callback->GiveDamage(killed,obj,(int)dmg,FSDIEDOF_MISSILE,*this);
				callback->ThrowMultiDebris(5,ctime,pos,obj.GetAttitude(),60.0);
				if(killed==YSTRUE)
				{
					sim->KillCallBack(obj,*tpos);
					AddKillCredit(killCredit,&obj,ctime);
				}

				explosion->Explode(ctime,pos,20.0,5.0,range,YSTRUE,NULL,YSTRUE);

				lifeRemain=0.0;
				target=NULL;

				return YSTRUE;
			}
		}
		// Proximity tube <<

		sqLastMovedDist=(lastChecked-pos).GetSquareLength();
		if(sqDist1<=sqLastMovedDist+rad*rad ||
		   sqDist2<=sqLastMovedDist+rad*rad)
		{
			YsVec3 np;
			YsGetNearestPointOnLine3(np,pos,lastChecked,*tpos);
			sqDist3=(np-(*tpos)).GetSquareLength();

			if(type==FSWEAPON_GUN)
			{
				if(sqDist1>rad*rad && sqDist2>rad*rad && sqDist3>rad*rad)
				{
					return YSFALSE;
				}

				YsShellPolygonHandle plHd;
				YsVec3 intersect;
				plHd=coll.ShootRayH(intersect,lastChecked,pos-lastChecked);

				if(plHd!=NULL && YsCheckInBetween3(intersect,pos,lastChecked)==YSTRUE)
				{
					YSBOOL killed;
					lifeRemain=0.0;

					if(callback->GiveDamage(killed,obj,destructivePower,FSDIEDOF_GUN,*this)==YSTRUE &&
					   explosion!=NULL)
					{
						callback->ThrowRandomDebris(ctime,intersect,obj.GetAttitude(),60.0);
						if(killed==YSTRUE)
						{
							callback->ThrowMultiDebris(5,ctime,pos,obj.GetAttitude(),60.0);
							sim->KillCallBack(obj,*tpos);
							AddKillCredit(killCredit,&obj,ctime);
						}
						else
						{
							explosion->Explode(ctime,intersect,5.0,3.0,3.0,YSTRUE,NULL,YSTRUE);
						}
					}

					return YSTRUE;
				}
			}
			else if(type==FSWEAPON_BOMB || type==FSWEAPON_BOMB250 || type==FSWEAPON_BOMB500HD || type==FSWEAPON_FUELTANK)
			{
				if(sqDist1>rad*rad && sqDist2>rad*rad && sqDist3>rad*rad)
				{
					return YSFALSE;
				}


				YsShellPolygonHandle plHd;
				YsVec3 intersect;
				plHd=coll.ShootRayH(intersect,lastChecked,pos-lastChecked);

				if(plHd!=NULL && YsCheckInBetween3(intersect,pos,lastChecked)==YSTRUE)
				{
					YSBOOL killed;
					lifeRemain=0.0;

					if(callback->GiveDamage(killed,obj,destructivePower,FSDIEDOF_GUN,*this)==YSTRUE &&
					   explosion!=NULL)
					{
						if(killed==YSTRUE)
						{
							callback->ThrowMultiDebris(5,ctime,pos,obj.GetAttitude(),60.0);
							sim->KillCallBack(obj,*tpos);
							AddKillCredit(killCredit,&obj,ctime);
						}
						else
						{
							explosion->Explode(ctime,intersect,5.0,3.0,3.0,YSTRUE,NULL,YSTRUE);
						}
					}

					ExplodeBomb(callback,ctime,pos,destructivePower*5/2,explosion,sim,killCredit);
					return YSTRUE;
				}
			}
			else if(type==FSWEAPON_AIM9 || type==FSWEAPON_AIM9X || type==FSWEAPON_AIM120 || type==FSWEAPON_AGM65 || type==FSWEAPON_ROCKET)
			{
				YsVec3 is;
				// Came close to the designated target
				// Or, direct impact
				if((&obj==target && YsCheckInBetween3(np,pos,lastChecked)==YSTRUE && sqDist3<rad*rad) ||
				   (coll.ShootRayH(is,lastChecked,pos-lastChecked)!=NULL &&
				    YsCheckInBetween3(is,pos,lastChecked)==YSTRUE))
				{
					YSBOOL killed;
					lifeRemain=0.0;
					target=NULL;
					if(callback->GiveDamage(killed,obj,destructivePower,FSDIEDOF_MISSILE,*this)==YSTRUE && explosion!=NULL)
					{
						if(killed==YSTRUE)
						{
							callback->ThrowMultiDebris(5,ctime,pos,obj.GetAttitude(),60.0);
							sim->KillCallBack(obj,*tpos);
							AddKillCredit(killCredit,&obj,ctime);
						}
						else
						{
							explosion->Explode(ctime,pos,20.0,5.0,20.0,YSTRUE,NULL,YSTRUE);
						}
					}

					if(type==FSWEAPON_ROCKET)
					{
						ExplodeBomb(callback,ctime,pos,50.0,explosion,sim,killCredit);
					}
					return YSTRUE;
				}
			}
		}
	}
	return YSFALSE;
}

void FsWeapon::ExplodeBomb
    (FsWeaponHolder *callback,
     const double &ctime,
     const YsVec3 &pos,
     const double &rad,
     FsExplosionHolder *explosion,
     FsSimulation *sim,
     YsList <FsKillCredit> *&killCredit)
{
	double sqDist,dist;
	double dmg;
	FsAirplane *air;
	FsGround *gnd;

	explosion->Explode(ctime,pos,10.0,0.0,rad+5.0,YSTRUE,NULL,YSFALSE);

	FsSoundSetOneTime(FSSND_ONETIME_BLAST);

	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		if(air->Prop().IsAlive()==YSTRUE)
		{
			const YsVec3 &objPos=air->GetPosition();
			sqDist=(objPos-pos).GetSquareLength();
			if(sqDist<rad*rad)
			{
				YSBOOL killed;
				dist=sqrt(sqDist);
				dmg=double(destructivePower)*(rad-dist)/rad;
				callback->GiveDamage(killed,*air,int(dmg),FSDIEDOF_BOMB,*this);
				if(killed==YSTRUE)
				{
					AddKillCredit(killCredit,air,ctime);
					callback->ThrowMultiDebris(5,ctime,objPos,air->GetAttitude(),60.0);
					sim->KillCallBack(*air,objPos);
				}
			}
		}
	}

	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->Prop().IsAlive()==YSTRUE)
		{
			const YsVec3 &objPos=gnd->GetPosition();
			sqDist=(objPos-pos).GetSquareLength();
			if(sqDist<rad*rad)
			{
				YSBOOL killed;
				dist=sqrt(sqDist);
				dmg=double(destructivePower)*(rad-dist)/rad;
				callback->GiveDamage(killed,*gnd,int(dmg),FSDIEDOF_BOMB,*this);
				if(killed==YSTRUE)
				{
					AddKillCredit(killCredit,gnd,ctime);
					callback->ThrowMultiDebris(5,ctime,objPos,gnd->GetAttitude(),60.0);
					sim->KillCallBack(*gnd,objPos);
				}
			}
		}
	}
}

void FsWeapon::ExplodeBombInWater(
    FsWeaponHolder *callback,
    const double &ctime,
    const YsVec3 &pos,
    const double &rad,  // B500->50/2  B250->35/2  RKT->10
    FsExplosionHolder *explosion,
    FsSimulation *sim,
    YsList <FsKillCredit> *&killCredit)
{
	double sqDist,dist;
	YsVec3 objPos;
	double dmg;
	FsAirplane *air;
	FsGround *gnd;

	explosion->WaterPlume(ctime,pos,10.0,rad,rad*10.0,NULL,YSFALSE);

	FsSoundSetOneTime(FSSND_ONETIME_BLAST);

	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		if(air->Prop().IsAlive()==YSTRUE)
		{
			objPos=air->GetPosition();
			sqDist=(objPos-pos).GetSquareLength();
			if(sqDist<rad*rad)
			{
				YSBOOL killed;
				dist=sqrt(sqDist);
				dmg=double(destructivePower)*(rad-dist)/rad;
				callback->GiveDamage(killed,*air,int(dmg),FSDIEDOF_BOMB,*this);
				if(killed==YSTRUE)
				{
					AddKillCredit(killCredit,air,ctime);
				}
			}
		}
	}

	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->Prop().IsAlive()==YSTRUE)
		{
			objPos=gnd->GetPosition();
			sqDist=(objPos-pos).GetSquareLength();
			if(sqDist<rad*rad)
			{
				YSBOOL killed;
				dist=sqrt(sqDist);
				dmg=double(destructivePower)*(rad-dist)/rad;
				callback->GiveDamage(killed,*gnd,int(dmg),FSDIEDOF_BOMB,*this);
				if(killed==YSTRUE)
				{
					AddKillCredit(killCredit,gnd,ctime);
				}
			}
		}
	}
}

// The following function is moved to fsweapongl.cpp and fsweaponbi.cpp
// void FsWeapon::Draw(void)

YSRESULT FsWeapon::AddKillCredit(YsList <FsKillCredit> *&killCredit,FsExistence *whoIsKilled,const double &when) const
{
	if(whoIsKilled!=NULL && whoIsKilled->isPlayingRecord!=YSTRUE)
	{
		if(FSEX_GROUND==whoIsKilled->GetType())
		{
			FsGround *gnd;
			gnd=(FsGround *)whoIsKilled;
			if(gnd->Prop().IsNonGameObject()==YSTRUE)  // 2005/02/05 Non-game object check
			{
				return YSOK;
			}
		}

		YsList <FsKillCredit> *neo;
		neo=new YsList <FsKillCredit>;
		neo->dat.whoKilled=firedBy;
		neo->dat.creditOwner=creditOwner;
		neo->dat.whom=whoIsKilled;
		neo->dat.byWhatKindOfWeapon=type;
		neo->dat.where=pos;
		neo->dat.when=when;

		killCredit=killCredit->Append(neo);
	}
	return YSOK;
}

void FsWeapon::AddToParticleManager(class YsGLParticleManager &partMan,const double cTime) const
{
	if(nullptr!=trail)
	{
		YSBOOL includeCurrentPos=(0.0<lifeRemain ? YSTRUE : YSFALSE);
		if(type==FSWEAPON_FLARE)
		{
			trail->AddToParticleManagerAsFlare(partMan,pos,cTime,includeCurrentPos);
		}
		else
		{
			trail->AddToParticleManager(partMan,pos,cTime,includeCurrentPos);
		}
	}
}

// Implementation //////////////////////////////////////////
FsWeaponHolder::FsWeaponHolder(FsSimulation *simPtr) : sim(simPtr)
{
	LoadMissilePattern();

	toPlay=NULL;
	toSave=new FsRecord <FsWeaponRecord>;

	activeList=NULL;
	freeList=NULL;

	killCredit=NULL;

	netServer=NULL;
	netClient=NULL;

	Clear();
}

FsWeaponHolder::~FsWeaponHolder()
{
	if(toSave!=NULL)
	{
		delete toSave;
	}
	if(toPlay!=NULL)
	{
		delete toPlay;
	}
	killCredit->DeleteList();
}

YSRESULT FsWeaponHolder::LoadMissilePattern(void)
{
	if(FsWeapon::aim9==nullptr)
	{
		FsWeapon::aim9.Load(L"misc/aim9.srf");
		if(FsWeapon::aim9==nullptr)
		{
			fsStderr.Printf("Cannot read AIM-9 pattern\n");
			return YSERR;
		}
	}

	if(FsWeapon::aim9s==nullptr)
	{
		FsWeapon::aim9s.Load(L"misc/aim9s.srf");
		if(FsWeapon::aim9s==nullptr)
		{
			fsStderr.Printf("Cannot read AIM-9(silent) pattern\n");
			return YSERR;
		}
	}

	if(FsWeapon::agm65==nullptr)
	{
		FsWeapon::agm65.Load(L"misc/agm65.srf");
		if(FsWeapon::agm65==nullptr)
		{
			fsStderr.Printf("Cannot read AGM-65 pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::agm65s==nullptr)
	{
		FsWeapon::agm65s.Load(L"misc/agm65s.srf");
		if(FsWeapon::agm65s==nullptr)
		{
			fsStderr.Printf("Cannot read AGM-65(silent) pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::bomb==nullptr)
	{
		FsWeapon::bomb.Load(L"misc/bomb.srf");
		if(FsWeapon::bomb==nullptr)
		{
			fsStderr.Printf("Cannot read BOMB pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::rocket==nullptr)
	{
		FsWeapon::rocket.Load(L"misc/rocket.srf");
		if(FsWeapon::rocket==nullptr)
		{
			fsStderr.Printf("Cannot read ROCKET pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::rockets==nullptr)
	{
		FsWeapon::rockets.Load(L"misc/rockets.srf");
		if(FsWeapon::rockets==nullptr)
		{
			fsStderr.Printf("Cannot read ROCKET(silent) pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::aim_coarse==nullptr)
	{
		FsWeapon::aim_coarse.Load(L"misc/aim_coarse.srf");
		if(FsWeapon::aim_coarse==nullptr)
		{
			fsStderr.Printf("Cannot read AIM(coarse) pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::agm_coarse==nullptr)
	{
		FsWeapon::agm_coarse.Load(L"misc/agm_coarse.srf");
		if(FsWeapon::agm_coarse==nullptr)
		{
			fsStderr.Printf("Cannot read AGM(coarse) pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::bomb_coarse==nullptr)
	{
		FsWeapon::bomb_coarse.Load(L"misc/bomb_coarse.srf");
		if(FsWeapon::bomb_coarse==nullptr)
		{
			fsStderr.Printf("Cannot read BOMB(coarse) pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::rocket_coarse==nullptr)
	{
		FsWeapon::rocket_coarse.Load(L"misc/rocket_coarse.srf");
		if(FsWeapon::rocket_coarse==nullptr)
		{
			fsStderr.Printf("Cannot read ROCKET(coarse) pattern.\n");
			return YSERR;
		}
	}




	if(FsWeapon::aim120==nullptr)
	{
		FsWeapon::aim120.Load(L"misc/aim120.srf");
		if(FsWeapon::aim120==nullptr)
		{
			fsStderr.Printf("Cannot read AIM120 pattern.\n");
			return YSERR;
		}
	}
	if(FsWeapon::aim120s==nullptr)
	{
		FsWeapon::aim120s.Load(L"misc/aim120s.srf");
		if(FsWeapon::aim120s==nullptr)
		{
			fsStderr.Printf("Cannot read AIM120 (silent) pattern.\n");
			return YSERR;
		}
	}
	if(FsWeapon::aim120_coarse==nullptr)
	{
		FsWeapon::aim120_coarse.Load(L"misc/aim120_coarse.srf");
		if(FsWeapon::aim120_coarse==nullptr)
		{
			fsStderr.Printf("Cannot read AIM120 (coarse) pattern.\n");
			return YSERR;
		}
	}


	if(FsWeapon::bomb250==nullptr)
	{
		FsWeapon::bomb250.Load(L"misc/bomb250.srf");
		if(FsWeapon::bomb250==nullptr)
		{
			fsStderr.Printf("Cannot read BOMB250 pattern.\n");
			return YSERR;
		}
	}

	if(FsWeapon::bomb250_coarse==nullptr)
	{
		FsWeapon::bomb250_coarse.Load(L"misc/bomb250_coarse.srf");
		if(FsWeapon::bomb250_coarse==nullptr)
		{
			fsStderr.Printf("Cannot read BOMB250(coarse) pattern.\n");
			return YSERR;
		}
	}


	if(nullptr==FsWeapon::bomb500hd)
	{
		FsWeapon::bomb500hd.Load(L"misc/bomb500hd.srf");
		if(nullptr==FsWeapon::bomb500hd)
		{
			fsStderr.Printf("Cannot read BOMB500HD pattern.\n");
			return YSERR;
		}
	}

	if(nullptr==FsWeapon::bomb500hds)
	{
		FsWeapon::bomb500hds.Load(L"misc/bomb500hds.srf");
		if(nullptr==FsWeapon::bomb500hds)
		{
			fsStderr.Printf("Cannot read BOMB500HD (silent) pattern.\n");
			return YSERR;
		}
	}

	if(nullptr==FsWeapon::bomb500hd_coarse)
	{
		FsWeapon::bomb500hd_coarse.Load(L"misc/bomb500hd_coarse.srf");
		if(nullptr==FsWeapon::bomb500hd_coarse)
		{
			fsStderr.Printf("Cannot read BOMB500HD (coarse) pattern.\n");
			return YSERR;
		}
	}


	if(nullptr==FsWeapon::aim9x)
	{
		FsWeapon::aim9x.Load(L"misc/aim9x.srf");
		if(nullptr==FsWeapon::aim9x)
		{
			fsStderr.Printf("Cannot read AIM9X pattern.\n");
			return YSERR;
		}
	}

	if(nullptr==FsWeapon::aim9xs)
	{
		FsWeapon::aim9xs.Load(L"misc/aim9xs.srf");
		if(nullptr==FsWeapon::aim9xs)
		{
			fsStderr.Printf("Cannot read AIM9X (silent) pattern.\n");
			return YSERR;
		}
	}

	if(nullptr==FsWeapon::aim9x_coarse)
	{
		FsWeapon::aim9x_coarse.Load(L"misc/aim9x_coarse.srf");
		if(nullptr==FsWeapon::aim9x_coarse)
		{
			fsStderr.Printf("Cannot read AIM9X (coarse) pattern.\n");
			return YSERR;
		}
	}

	if(nullptr==FsWeapon::flarePod)
	{
		FsWeapon::flarePod.Load(L"misc/flarepod.srf");
		if(nullptr==FsWeapon::flarePod)
		{
			fsStderr.Printf("Cannot read Flare Pod pattern.\n");
			return YSERR;
		}
	}

	if(nullptr==FsWeapon::fuelTank)
	{
		FsWeapon::fuelTank.Load(L"misc/fueltank.srf");
		if(nullptr==FsWeapon::fuelTank)
		{
			fsStderr.Printf("Cannot read fuel tank pattern.\n");
			return YSERR;
		}
	}


	return YSOK;
}

/* static */ void FsWeaponHolder::FreeMissilePattern(void)
{
	if(FsWeapon::aim9!=nullptr)
	{
		FsWeapon::aim9.CleanUp();
	}

	if(FsWeapon::aim9s!=nullptr)
	{
		FsWeapon::aim9s.CleanUp();
	}

	if(FsWeapon::agm65!=nullptr)
	{
		FsWeapon::agm65.CleanUp();
	}

	if(FsWeapon::agm65s!=nullptr)
	{
		FsWeapon::agm65s.CleanUp();
	}

	if(FsWeapon::bomb!=nullptr)
	{
		FsWeapon::bomb.CleanUp();
	}

	if(FsWeapon::rocket!=nullptr)
	{
		FsWeapon::rocket.CleanUp();
	}

	if(FsWeapon::rockets!=nullptr)
	{
		FsWeapon::rockets.CleanUp();
	}

	if(FsWeapon::aim_coarse!=nullptr)
	{
		FsWeapon::aim_coarse.CleanUp();
	}

	if(FsWeapon::agm_coarse!=nullptr)
	{
		FsWeapon::agm_coarse.CleanUp();
	}

	if(FsWeapon::bomb_coarse!=nullptr)
	{
		FsWeapon::bomb_coarse.CleanUp();
	}

	if(FsWeapon::rocket_coarse!=nullptr)
	{
		FsWeapon::rocket_coarse.CleanUp();
	}




	if(FsWeapon::aim120!=nullptr)
	{
		FsWeapon::aim120.CleanUp();
	}
	if(FsWeapon::aim120s!=nullptr)
	{
		FsWeapon::aim120s.CleanUp();
	}
	if(FsWeapon::aim120_coarse!=nullptr)
	{
		FsWeapon::aim120_coarse.CleanUp();
	}


	if(FsWeapon::bomb250!=nullptr)
	{
		FsWeapon::bomb250.CleanUp();
	}

	if(FsWeapon::bomb250_coarse!=nullptr)
	{
		FsWeapon::bomb250_coarse.CleanUp();
	}


	if(nullptr!=FsWeapon::bomb500hd)
	{
		FsWeapon::bomb500hd.CleanUp();
	}

	if(nullptr!=FsWeapon::bomb500hds)
	{
		FsWeapon::bomb500hds.CleanUp();
	}

	if(nullptr!=FsWeapon::bomb500hd_coarse)
	{
		FsWeapon::bomb500hd_coarse.CleanUp();
	}


	if(nullptr!=FsWeapon::aim9x)
	{
		FsWeapon::aim9x.CleanUp();
	}

	if(nullptr!=FsWeapon::aim9xs)
	{
		FsWeapon::aim9xs.CleanUp();
	}

	if(nullptr!=FsWeapon::aim9x_coarse)
	{
		FsWeapon::aim9x_coarse.CleanUp();
	}

	if(nullptr!=FsWeapon::flarePod)
	{
		FsWeapon::flarePod.CleanUp();
	}

	if(nullptr!=FsWeapon::fuelTank)
	{
		FsWeapon::fuelTank.CleanUp();
	}
}

YSRESULT FsWeaponHolder::Save(FILE *fp,FsSimulation *sim,int wpnPosPrecision,int wpnAttPrecision)
{
	if(toPlay!=NULL)
	{
		int i,nRec;
		char format[256];

		nRec=toPlay->GetNumRecord();
		fprintf(fp,"BULRECOR\n");

		fprintf(fp,"VERSION 4\n");
			//  Version 3 Added rocket
			//  Version 4 Added credit owner in both launch and kill records

		fprintf(fp,"NUMRECO %d\n",nRec);

		sprintf(format,"%s %s%d%s %s%d%s %s%d%s %s%d%s %s%d%s %s%d%s\n",
		    "%g %d",
		    "%.",wpnPosPrecision,"f",
            "%.",wpnPosPrecision,"f",
            "%.",wpnPosPrecision,"f",
            "%.",wpnAttPrecision,"f",
            "%.",wpnAttPrecision,"f",
            "%.",wpnAttPrecision,"f");


		for(i=0; i<nRec; i++)
		{
			int ownerId;
			double t;

			const FsWeaponRecord *rec=toPlay->GetElement(t,i);

			fprintf(fp,format, //  "%g %d %.2f %.2f %.2f %.2f %.2f %.2f\n",
			    t,rec->type,rec->x,rec->y,rec->z,rec->h,rec->p,rec->b);
			fprintf(fp,"%.2f %.2f %d ",
			    rec->velocity,rec->lifeRemain,rec->power);

			if((ownerId=sim->GetAirplaneIdFromHandle(rec->firedBy))>=0)
			{
				fprintf(fp,"A%d ",ownerId);
			}
			else if((ownerId=sim->GetGroundIdFromHandle(rec->firedBy))>=0)
			{
				fprintf(fp,"G%d ",ownerId);
			}
			else
			{
				fprintf(fp,"N ");
			}

			switch(rec->creditOwner)
			{
			default:
			case FSWEAPON_CREDIT_OWNER_NOT_RECORDED:
				fprintf(fp,"U\n");  // Unknown
				break;
			case FSWEAPON_CREDIT_OWNER_PLAYER:
				fprintf(fp,"P\n");  // Player
				break;
			case FSWEAPON_CREDIT_OWNER_NON_PLAYER:
				fprintf(fp,"N\n");  // Non-Player
				break;
			}

			if(rec->type==FSWEAPON_AIM9 || rec->type==FSWEAPON_AIM9X || rec->type==FSWEAPON_AIM120)
			{
				int targetId;
				targetId=sim->GetAirplaneIdFromHandle(rec->target);
				fprintf(fp,"%.2f %.2f %.2f %d\n",
				    rec->vMax,rec->mobility,rec->radar,targetId);
			}
			else if(rec->type==FSWEAPON_AGM65)
			{
				int targetId;
				targetId=-1;
				targetId=sim->GetGroundIdFromHandle(rec->target);
				fprintf(fp,"%.2f %.2f %.2f %d\n",
				    rec->vMax,rec->mobility,rec->radar,targetId);
			}
			else if(rec->type==FSWEAPON_ROCKET)
			{
				fprintf(fp,"%.2f\n",rec->vMax);
			}
		}

		if(killCredit->GetNumObject()>0)
		{
			YsList <FsKillCredit> *kcSeeker;
			fprintf(fp,"KILLCREDIT 1 %d\n",killCredit->GetNumObject());  // 1 : Version 1
			for(kcSeeker=killCredit; kcSeeker!=NULL; kcSeeker=kcSeeker->Next())
			{
				int whoKilled,whom;

				fprintf(fp,"%d ",kcSeeker->dat.byWhatKindOfWeapon);

				if((whoKilled=sim->GetAirplaneIdFromHandle(kcSeeker->dat.whoKilled))>=0)
				{
					fprintf(fp,"A%d ",whoKilled);
				}
				else if((whoKilled=sim->GetGroundIdFromHandle(kcSeeker->dat.whoKilled))>=0)
				{
					fprintf(fp,"G%d ",whoKilled);
				}
				else
				{
					fprintf(fp,"N ");
				}

				if((whom=sim->GetAirplaneIdFromHandle(kcSeeker->dat.whom))>=0)
				{
					fprintf(fp,"A%d ",whom);
				}
				else if((whom=sim->GetGroundIdFromHandle(kcSeeker->dat.whom))>=0)
				{
					fprintf(fp,"G%d ",whom);
				}
				else
				{
					fprintf(fp,"N ");
				}

				switch(kcSeeker->dat.creditOwner)
				{
				default:
				case FSWEAPON_CREDIT_OWNER_NOT_RECORDED:
					fprintf(fp,"U ");  // Unknown
					break;
				case FSWEAPON_CREDIT_OWNER_PLAYER:
					fprintf(fp,"P ");  // Player
					break;
				case FSWEAPON_CREDIT_OWNER_NON_PLAYER:
					fprintf(fp,"N ");  // Non-Player
					break;
				}

				fprintf(fp,"%.1lf %.1lf %.1lf %.1lf\n",
				    kcSeeker->dat.where.x(),
				    kcSeeker->dat.where.y(),
				    kcSeeker->dat.where.z(),
				    kcSeeker->dat.when);
			}
		}

		fprintf(fp,"ENDRECO\n");
	}
	return YSOK;
}

YSRESULT FsWeaponHolder::Load(FILE *fp,FsSimulation *sim)
{
	if(toSave!=NULL)
	{
		delete toSave;
		toSave=NULL;
	}
	if(toPlay!=NULL)
	{
		delete toPlay;
		toPlay=NULL;
	}
	killCredit->DeleteList();
	killCredit=NULL;

	toPlay=new FsRecord <FsWeaponRecord>;

	FsWeaponRecord rec;
	char buf[256];
	int version;
	version=-1;
	while(fgets(buf,256,fp)!=NULL)
	{
		if(strncmp(buf,"VERSION 1",9)==0)
		{
			version=1;
			continue;
		}
		else if(strncmp(buf,"VERSION 2",9)==0)
		{
			version=2;
			continue;
		}
		else if(strncmp(buf,"VERSION 3",9)==0)
		{
			version=3;
			continue;
		}
		else if(0==strncmp(buf,"VERSION 4",9))
		{
			version=4;
			continue;
		}
		else if(strncmp(buf,"NUMRECO",7)==0)
		{
			int i,nRec;
			nRec=atoi(buf+8);

			if(version>=4)
			{
				for(i=0; i<nRec; i++)
				{
					int type;
					char str[256],creditOwnerId;
					double t;

					fgets(buf,256,fp);
					sscanf(buf,"%lf%d%f%f%f%f%f%f",
					    &t,&type,&rec.x,&rec.y,&rec.z,&rec.h,&rec.p,&rec.b);

					rec.type=(FSWEAPONTYPE)type;

					fgets(buf,256,fp);
					sscanf(buf,"%f%f%d%s%c",&rec.velocity,&rec.lifeRemain,&rec.power,str,&creditOwnerId);

					rec.firedBy=FindObjectByAxxGxxN(str,sim);
					switch(creditOwnerId)
					{
					default:
					case 'U':
						rec.creditOwner=FSWEAPON_CREDIT_OWNER_NOT_RECORDED;
						break;
					case 'P':
						rec.creditOwner=FSWEAPON_CREDIT_OWNER_PLAYER;
						break;
					case 'N':
						rec.creditOwner=FSWEAPON_CREDIT_OWNER_NON_PLAYER;
						break;
					}

					rec.target=NULL;
					if(rec.type==FSWEAPON_GUN)
					{
						rec.vMax=rec.velocity;
						rec.mobility=0.0F;
						rec.target=NULL;
					}
					else if(rec.type==FSWEAPON_AIM9 || rec.type==FSWEAPON_AIM9X || rec.type==FSWEAPON_AIM120)
					{
						int id;
						fgets(buf,256,fp);
						sscanf(buf,"%f%f%f%d",
						    &rec.vMax,&rec.mobility,&rec.radar,&id);
						rec.target=sim->GetAirplaneById(id);
					}
					else if(rec.type==FSWEAPON_AGM65)
					{
						int id;
						fgets(buf,256,fp);
						sscanf(buf,"%f%f%f%d",
						    &rec.vMax,&rec.mobility,&rec.radar,&id);
						rec.target=sim->GetGroundById(id);
					}
					else if(rec.type==FSWEAPON_ROCKET)
					{
						fgets(buf,256,fp);
						sscanf(buf,"%f",&rec.vMax);
						rec.target=NULL;
						rec.mobility=0.0;
					}

					toPlay->AddElement(rec,t);
				}
			}
			else if(version>=2)
			{
				for(i=0; i<nRec; i++)
				{
					int type;
					char str[256];
					double t;

					fgets(buf,256,fp);
					sscanf(buf,"%lf%d%f%f%f%f%f%f",
					    &t,&type,&rec.x,&rec.y,&rec.z,&rec.h,&rec.p,&rec.b);

					rec.type=(FSWEAPONTYPE)type;

					fgets(buf,256,fp);
					sscanf(buf,"%f%f%d%s",&rec.velocity,&rec.lifeRemain,&rec.power,str);

					rec.firedBy=FindObjectByAxxGxxN(str,sim);
					rec.creditOwner=FSWEAPON_CREDIT_OWNER_NOT_RECORDED;

					rec.target=NULL;
					if(rec.type==FSWEAPON_GUN)
					{
						rec.vMax=rec.velocity;
						rec.mobility=0.0F;
						rec.target=NULL;
					}
					else if(rec.type==FSWEAPON_AIM9 || rec.type==FSWEAPON_AIM9X || rec.type==FSWEAPON_AIM120)
					{
						int id;
						fgets(buf,256,fp);
						sscanf(buf,"%f%f%f%d",
						    &rec.vMax,&rec.mobility,&rec.radar,&id);
						rec.target=sim->GetAirplaneById(id);
					}
					else if(rec.type==FSWEAPON_AGM65)
					{
						int id;
						fgets(buf,256,fp);
						sscanf(buf,"%f%f%f%d",
						    &rec.vMax,&rec.mobility,&rec.radar,&id);
						rec.target=sim->GetGroundById(id);
					}
					else if(rec.type==FSWEAPON_ROCKET)
					{
						fgets(buf,256,fp);
						sscanf(buf,"%f",&rec.vMax);
						rec.target=NULL;
						rec.mobility=0.0;
					}

					toPlay->AddElement(rec,t);
				}
			}
			else if(version==1)
			{
				for(i=0; i<nRec; i++)
				{
					int type;
					char str[256];
					FSWEAPONTYPE typeList[]=
					{
						FSWEAPON_GUN,
						FSWEAPON_AIM9,
						FSWEAPON_AGM65
					};
					double t;

					fgets(buf,256,fp);
					sscanf(buf,"%lf%d%f%f%f%f%f%f",
					    &t,&type,&rec.x,&rec.y,&rec.z,&rec.h,&rec.p,&rec.b);

					rec.type=typeList[type];
					if(rec.type==FSWEAPON_GUN)
					{
						rec.power=1;
					}
					else
					{
						rec.power=12;
					}

					fgets(buf,256,fp);
					sscanf(buf,"%f%f%s",
					    &rec.velocity,&rec.lifeRemain,str);

					rec.firedBy=FindObjectByAxxGxxN(str,sim);
					rec.creditOwner=FSWEAPON_CREDIT_OWNER_NOT_RECORDED;

					rec.target=NULL;
					if(rec.type==FSWEAPON_GUN)
					{
						rec.vMax=rec.velocity;
						rec.mobility=0.0F;
						rec.target=NULL;
					}
					else if(rec.type==FSWEAPON_AIM9)
					{
						int id;
						fgets(buf,256,fp);
						sscanf(buf,"%f%f%f%d",
						    &rec.vMax,&rec.mobility,&rec.radar,&id);
						rec.target=sim->GetAirplaneById(id);
					}
					else if(rec.type==FSWEAPON_AGM65)
					{
						int id;
						fgets(buf,256,fp);
						sscanf(buf,"%f%f%f%d",
						    &rec.vMax,&rec.mobility,&rec.radar,&id);
						//rec.target=sim->GetGroundObject(id);
					}

					toPlay->AddElement(rec,t);
				}
			}
		}
		else if(strncmp(buf,"KILLCREDIT",10)==0)
		{
			// 0  1  2  3  4  5  6  7  8  9 10 11 12  13
			// K  I  L  L  C  R  E  D  I  T  _  1  _  (#ofCredit)
			if(buf[11]=='1')
			{
				int i,nCredit;
				nCredit=atoi(buf+13);
				for(i=0; i<nCredit; i++)
				{
					char buf[256];
					char obj1[256],obj2[256],creditOwnerId;
					int type;
					double x,y,z,when;
					YsList <FsKillCredit> *neo;

					if(fgets(buf,256,fp)!=NULL)
					{
						if(4<=version)
						{
							sscanf(buf,"%d%s%s%c%lf%lf%lf%lf",&type,obj1,obj2,&creditOwnerId,&x,&y,&z,&when);
							neo=new YsList <FsKillCredit>;
							neo->dat.whoKilled=FindObjectByAxxGxxN(obj1,sim);
							neo->dat.whom=FindObjectByAxxGxxN(obj2,sim);

							switch(creditOwnerId)
							{
							default:
							case 'U':
								neo->dat.creditOwner=FSWEAPON_CREDIT_OWNER_NOT_RECORDED;
								break;
							case 'P':
								neo->dat.creditOwner=FSWEAPON_CREDIT_OWNER_PLAYER;
								break;
							case 'N':
								neo->dat.creditOwner=FSWEAPON_CREDIT_OWNER_NON_PLAYER;
								break;
							}

							neo->dat.byWhatKindOfWeapon=(FSWEAPONTYPE)type;
							neo->dat.where.Set(x,y,z);
							neo->dat.when=when;
						}
						else
						{
							sscanf(buf,"%d%s%s%lf%lf%lf%lf",&type,obj1,obj2,&x,&y,&z,&when);
							neo=new YsList <FsKillCredit>;
							neo->dat.whoKilled=FindObjectByAxxGxxN(obj1,sim);
							neo->dat.creditOwner=FSWEAPON_CREDIT_OWNER_NOT_RECORDED;
							neo->dat.whom=FindObjectByAxxGxxN(obj2,sim);
							neo->dat.byWhatKindOfWeapon=(FSWEAPONTYPE)type;
							neo->dat.where.Set(x,y,z);
							neo->dat.when=when;
						}
						killCredit=killCredit->Append(neo);
					}
					else
					{
						fsStderr.Printf("Unexpected End Of File while Reading kill credits.\n");
						return YSERR;
					}
				}
			}
			else
			{
				fsStderr.Printf("Incorrect Version Number.\n");
				fsStderr.Printf("Please download newest version and try again.\n");
				return YSERR;
			}
		}
		else if(strncmp(buf,"ENDRECO",7)==0)
		{
			break;
		}
		else
		{
			fsStderr.Printf("Unrecognized Bullet Record\n");
			return YSERR;
		}
	}

	return YSOK;
}

void FsWeaponHolder::Clear(void)
{
	int i;
	for(i=0; i<NumBulletBuffer; i++)
	{
		buf[i].lifeRemain=0.0;
		if(i==0)
		{
			buf[i].prev=NULL;
			buf[i].next=&buf[i+1];
		}
		else if(i==NumBulletBuffer-1)
		{
			buf[i].prev=&buf[i-1];
			buf[i].next=NULL;
		}
		else
		{
			buf[i].prev=&buf[i-1];
			buf[i].next=&buf[i+1];
		}
	}

	for(i=0; i<NumSmokeTrailBuffer; i++)
	{
		trl[i].used=YSFALSE;
	}

	activeList=NULL;
	freeList=&buf[0];

	bulletCalibrator.Clear();
}

void FsWeaponHolder::MoveToActiveList(FsWeapon *wep)  // Now the order matters.  Don't change this order.
{
	FsWeapon *next,*prev;

	next=wep->next;
	prev=wep->prev;
	if(next!=NULL)
	{
		next->prev=prev;
	}
	if(prev!=NULL)
	{
		prev->next=next;
	}
	else
	{
		freeList=next;
	}

	if(activeList!=NULL)
	{
		activeList->prev=wep;
	}
	wep->next=activeList;
	wep->prev=NULL;
	activeList=wep;
}

void FsWeaponHolder::MoveToFreeList(FsWeapon *wep)
{
	FsWeapon *next,*prev;

	next=wep->next;
	prev=wep->prev;
	if(next!=NULL)
	{
		next->prev=prev;
	}
	if(prev!=NULL)
	{
		prev->next=next;
	}
	else
	{
		activeList=next;
	}

//	if(wep->type==FSWOC_FLARE)
//	{
//		if(wep->nextFlare!=NULL)
//		{
//			wep->nextFlare->prevFlare=wep->prevFlare;
//		}
//		if(wep->prevFlare!=NULL)
//		{
//			wep->prevFlare->nextFlare=wep->nextFlare;
//		}
//		else
//		{
//			flareList=wep->nextFlare;
//		}
//		wep->nextFlare=NULL;
//		wep->prevFlare=NULL;
//	}

	if(freeList!=NULL)
	{
		freeList->prev=wep;
	}
	wep->next=freeList;
	wep->prev=NULL;
	freeList=wep;

	if(wep->trail!=NULL)
	{
		wep->trail->used=YSFALSE;
		wep->trail=NULL;
	}
}

void FsWeaponHolder::ClearBulletCalibrator(void)
{
	bulletCalibrator.Clear();
}

void FsWeaponHolder::CalculateBulletCalibrator(const FsExistence *target)
{
	ClearBulletCalibrator();
	for(FsWeapon *seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		if(FSWEAPON_GUN==seeker->type)
		{
			const YsVec3 &prv=seeker->prv;
			const YsVec3 &pos=seeker->pos;

			YsVec3 nearPos;
			if(YSTRUE==YsCheckInBetween3(target->GetPosition(),pos,prv) &&
			   YSOK==YsGetNearestPointOnLine3(nearPos,prv,pos,target->GetPosition()))
			{
				bulletCalibrator.Append(nearPos);
			}
		}
	}
}

YSBOOL FsWeaponHolder::GiveDamage(YSBOOL &killed,FsExistence &obj,int destPower,FSDIEDOF diedOf,FsWeapon &wpn)
{
	if(&obj!=NULL && obj.GetDamage(killed,destPower,diedOf)==YSTRUE)
	{
		if(YSTRUE==killed && NULL!=netServer)
		{
			netServer->ReportKill(&obj,wpn.firedBy,wpn.type);
		}

		// Network Transmission
		if(wpn.firedBy!=NULL && wpn.firedBy->netType==FSNET_LOCAL)
		{
			if(NULL!=netServer)
			{
				netServer->BroadcastGetDamage(&obj,wpn.firedBy,destPower,diedOf);
			}
			if(NULL!=netClient)
			{
				netClient->SendGetDamage(&obj,wpn.firedBy,destPower,diedOf,wpn.type);
			}
		}
		else if(wpn.firedBy==NULL)  // Let's assume it's server's local.
		{
			if(netServer!=NULL)
			{
				netServer->BroadcastGetDamage(&obj,wpn.firedBy,destPower,diedOf);
			}
		}
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSRESULT FsWeaponHolder::RipOffEarlyPartOfRecord(void)
{
	if(toSave!=NULL)
	{
		toSave->RipOffEarlyPartOfRecord();
	}
	return YSOK;
}

YSRESULT FsWeaponHolder::DeleteRecordForResumeFlight(FsAirplane *shotBy,const double &startTime)
{
	int i,nRec;
	double t;

	FsRecord <FsWeaponRecord> *neo;

	neo=new FsRecord <FsWeaponRecord>;

	if(neo!=NULL)
	{
		nRec=toPlay->GetNumRecord();
		for(i=0; i<nRec; i++)
		{
			FsWeaponRecord *elem;
			elem=toPlay->GetElement(t,i);
			if(elem->firedBy==shotBy && startTime<=t)
			{
			}
			else
			{
				neo->AddElement(*elem,t);
			}
		}

		if(nRec!=neo->GetNumRecord())
		{
			delete toPlay;
			toPlay=neo;
		}
		else
		{
			delete neo;
		}
	}

	return YSOK;
}

YSRESULT FsWeaponHolder::RefreshOrdinanceByWeaponRecord(const double &currentTime)
{
	if(toPlay!=NULL)
	{
		int i,n;
		double t;
		FsWeaponRecord *rec;

		t=0.0;
		n=toPlay->GetNumRecord();
		for(i=0; i<n; i++)
		{
			rec=toPlay->GetElement(t,i);
			if(currentTime<t)
			{
				break;
			}

			if(rec!=NULL &&
			   rec->firedBy!=NULL &&
			   rec->firedBy->IsAirplane()==YSTRUE &&
			   rec->type!=FSWEAPON_GUN)
			{
				FsAirplane *air;
				air=(FsAirplane *)rec->firedBy;
				air->Prop().FireMissileByRecord(rec->type);
			}
		}
		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

int FsWeaponHolder::Fire
    (const double &ctime,
     YsVec3 &pos,
     YsAtt3 &att,
     double v,
     double l,
     int destructivePower,
     FsExistence *owner,
     YSBOOL recordIt,YSBOOL /*transmit*/)
{
	if(freeList!=NULL)
	{
		FsWeapon *toShoot=freeList;
		FSWEAPON_CREDIT_OWNER creditOwner=(sim->GetPlayerObject()==owner ? FSWEAPON_CREDIT_OWNER_PLAYER : FSWEAPON_CREDIT_OWNER_NON_PLAYER);

		toShoot->Fire(ctime,pos,att,v,l,destructivePower,owner,creditOwner);
		MoveToActiveList(toShoot);

		if(recordIt==YSTRUE && toSave!=NULL)
		{
			FsWeaponRecord rec;
			rec.type=FSWEAPON_GUN;
			rec.x=float(pos.x());
			rec.y=float(pos.y());
			rec.z=float(pos.z());
			rec.h=float(att.h());
			rec.p=float(att.p());
			rec.b=float(att.b());
			rec.velocity=float(v);
			rec.lifeRemain=float(l);
			rec.power=destructivePower;
			rec.firedBy=owner;
			rec.creditOwner=creditOwner;

			rec.vMax=float(v);
			rec.mobility=float(0.0);
			rec.target=NULL;

			toSave->AddElement(rec,ctime);
		}
		return (int)(toShoot-buf);
	}
	return -1;
}

int FsWeaponHolder::Fire
    (const double &ctime,
     FSWEAPONTYPE missileType,
     YsVec3 &pos,YsAtt3 &att,
     double v,double vmax,double l,double mobility,double radar,
     int destructivePower,
     FsExistence *owner,unsigned int targetKey,
     YSBOOL recordIt,YSBOOL transmit)
{
	FsExistence *target=NULL;

	target=sim->FindAirplane(targetKey);
	if(NULL==target)
	{
		target=sim->FindGround(targetKey);
	}

	if(freeList!=NULL)
	{
		FsWeaponSmokeTrail *trail;
		int i;
		trail=NULL;
		for(i=0; i<NumSmokeTrailBuffer; i++)
		{
			if(trl[i].used!=YSTRUE)
			{
				trl[i].used=YSTRUE;
				trail=&trl[i];
				break;
			}
		}

		FsWeapon *toShoot=freeList;
		FSWEAPON_CREDIT_OWNER creditOwner=(sim->GetPlayerObject()==owner ? FSWEAPON_CREDIT_OWNER_PLAYER : FSWEAPON_CREDIT_OWNER_NON_PLAYER);

		toShoot->Fire(ctime,missileType,pos,att,v,vmax,l,mobility,radar,destructivePower,owner,creditOwner,target,trail);
		MoveToActiveList(toShoot);

		if((recordIt==YSTRUE && toSave!=NULL) ||
		   (transmit==YSTRUE && (netServer!=NULL || netClient!=NULL)))
		{
			FsWeaponRecord rec;
			rec.type=missileType;
			rec.x=float(pos.x());
			rec.y=float(pos.y());
			rec.z=float(pos.z());
			rec.h=float(att.h());
			rec.p=float(att.p());
			rec.b=float(att.b());
			rec.velocity=float(v);
			rec.lifeRemain=float(l);
			rec.power=destructivePower;
			rec.firedBy=owner;
			rec.creditOwner=creditOwner;

			rec.vMax=float(vmax);
			rec.mobility=float(mobility);
			rec.radar=float(radar);
			rec.target=target;

			if(recordIt==YSTRUE && toSave!=NULL)
			{
				toSave->AddElement(rec,ctime);
			}
			if(transmit==YSTRUE && netServer!=NULL)
			{
				netServer->BroadcastMissileLaunch(rec);
			}
			if(transmit==YSTRUE && netClient!=NULL)
			{
				netClient->SendMissileLaunch(rec);
			}
		}
		return (int)(toShoot-buf);
	}

	return -1;
}

int FsWeaponHolder::Bomb(
    const double &ctime,
    FSWEAPONTYPE bombType,
    const YsVec3 &pos,const YsAtt3 &/*att*/,const YsVec3 &iniVelocity,
    const double &vMax,
    int destructivePower,
    class FsExistence *owner,
    YSBOOL recordIt,YSBOOL transmit)
{
	if(freeList!=NULL)
	{
		YsAtt3 attByVel;
		FsWeapon *toShoot=freeList;
		FSWEAPON_CREDIT_OWNER creditOwner=(sim->GetPlayerObject()==owner ? FSWEAPON_CREDIT_OWNER_PLAYER : FSWEAPON_CREDIT_OWNER_NON_PLAYER);

		attByVel.SetForwardVector(iniVelocity);

		toShoot->Fire(ctime,bombType,pos,attByVel,iniVelocity.GetLength(),vMax,1.0,0.0,0.0,destructivePower,owner,creditOwner,NULL,NULL);
		MoveToActiveList(toShoot);

		if((recordIt==YSTRUE && toSave!=NULL) || (transmit==YSTRUE && (netServer!=NULL || netClient!=NULL)))
		{
			FsWeaponRecord rec;
			rec.type=bombType;
			rec.x=float(pos.x());
			rec.y=float(pos.y());
			rec.z=float(pos.z());
			rec.h=float(attByVel.h());
			rec.p=float(attByVel.p());
			rec.b=float(attByVel.b());
			rec.velocity=float(iniVelocity.GetLength());
			rec.lifeRemain=1.0F;  // Until hit the ground
			rec.power=destructivePower;
			rec.firedBy=owner;
			rec.creditOwner=creditOwner;

			rec.vMax=float(vMax);
			rec.mobility=0.0F;
			rec.radar=0.0F;
			rec.target=NULL;

			if(recordIt==YSTRUE && toSave!=NULL)
			{
				toSave->AddElement(rec,ctime);
			}
			if(transmit==YSTRUE && netServer!=NULL)
			{
				netServer->BroadcastMissileLaunch(rec);
			}
			if(transmit==YSTRUE && netClient!=NULL)
			{
				netClient->SendMissileLaunch(rec);
			}
		}
		return (int)(toShoot-buf);
	}

	return -1;
}

int FsWeaponHolder::DispenseFlare(
    const double &ctime,
    const YsVec3 &pos,const YsVec3 &vel,const double &vMax,const double &l,
    class FsExistence *owner,YSBOOL recordIt,YSBOOL transmit)
{
	if(freeList!=NULL)
	{
		YsAtt3 attByVel;
		FsWeapon *toShoot=freeList;
		FSWEAPON_CREDIT_OWNER creditOwner=(sim->GetPlayerObject()==owner ? FSWEAPON_CREDIT_OWNER_PLAYER : FSWEAPON_CREDIT_OWNER_NON_PLAYER);

		attByVel.SetForwardVector(vel);

		FsWeaponSmokeTrail *trail;
		int i;
		trail=NULL;
		for(i=0; i<NumSmokeTrailBuffer; i++)
		{
			if(trl[i].used!=YSTRUE)
			{
				trl[i].used=YSTRUE;
				trail=&trl[i];
				break;
			}
		}

		toShoot->DispenseFlare(ctime,pos,attByVel,vel.GetLength(),vMax,l,owner,creditOwner,trail);
		MoveToActiveList(toShoot);

//		toShoot->prevFlare=NULL;
//		toShoot->nextFlare=flareList;
//		flareList=toShoot;
//		if(toShoot->nextFlare!=NULL)
//		{
//			toShoot->nextFlare->prevFlare=toShoot;
//		}

		if((recordIt==YSTRUE && toSave!=NULL) ||
		   (transmit==YSTRUE && (netServer!=NULL || netClient!=NULL)))
		{
			FsWeaponRecord rec;
			rec.type=FSWEAPON_FLARE;
			rec.x=float(pos.x());
			rec.y=float(pos.y());
			rec.z=float(pos.z());
			rec.h=float(attByVel.h());
			rec.p=float(attByVel.p());
			rec.b=float(attByVel.b());
			rec.velocity=float(vel.GetLength());
			rec.lifeRemain=float(l);
			rec.power=0;
			rec.firedBy=owner;
			rec.creditOwner=creditOwner;

			rec.vMax=float(vMax);
			rec.mobility=0.0F;
			rec.radar=0.0F;
			rec.target=NULL;

			if(recordIt==YSTRUE && toSave!=NULL)
			{
				toSave->AddElement(rec,ctime);
			}
			if(transmit==YSTRUE && netServer!=NULL)
			{
				netServer->BroadcastMissileLaunch(rec);
			}
			if(transmit==YSTRUE && netClient!=NULL)
			{
				netClient->SendMissileLaunch(rec);
			}
		}
		return (int)(toShoot-buf);
	}

	return -1;
}

int FsWeaponHolder::ThrowDebris(const double &ctime,const YsVec3 &pos,const YsVec3 &vec,const double &l)
{
	if(freeList!=NULL)
	{
		FsWeapon *toShoot;
		toShoot=freeList;

		toShoot->ThrowDebris(ctime,pos,vec,l);
		MoveToActiveList(toShoot);
		return (int)(toShoot-buf);
	}

	return -1;
}

void FsWeaponHolder::ThrowRandomDebris(const double &ctime,const YsVec3 &pos,const YsAtt3 &att,const double &l)
{
	YsVec3 debrisVec;
	debrisVec.Set(rand()&2047-1024,rand()&1023,rand()&2047-1024);
	debrisVec/=16.0;
	att.Mul(debrisVec,debrisVec);
	ThrowDebris(ctime,pos,debrisVec,l);
}

void FsWeaponHolder::ThrowMultiDebris(int n,const double &ctime,const YsVec3 &pos,const YsAtt3 &att,const double &l)
{
	while(n>0)
	{
		ThrowRandomDebris(ctime,pos,att,l);
		n--;
	}
}

YSBOOL FsWeaponHolder::IsLockedOn(const FsExistence *ex) const
{
	FsWeapon *seeker;
	for(seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		if(seeker->lifeRemain>YsTolerance && seeker->target==ex) // Need to check lifeRemain since smoke may remain for a few seconds after the missile runs out of its range.
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsWeaponHolder::IsLockedOn(FSWEAPONTYPE &wpnType,YsVec3 &wpnPos,const FsExistence *ex) const
{
	FsWeapon *seeker;
	for(seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		if(seeker->lifeRemain>YsTolerance && seeker->target==ex) // 2014/10/30 lifeRemain check was missing and has been added.
		{
			wpnType=seeker->type;
			wpnPos=seeker->pos;
			return YSTRUE;
		}
	}
	return YSFALSE;
}

//returns a reference to the weapon currently pursuing ex, or NULL if none was found 
FsWeapon* FsWeaponHolder::GetLockedOn(const FsExistence* ex) const
{
	FsWeapon* seeker;
	for (seeker = activeList; seeker != NULL; seeker = seeker->next)
	{
		if (seeker->lifeRemain > YsTolerance && seeker->target == ex)
		{
			return seeker;
		}
	}
	return NULL;
}


YSRESULT FsWeaponHolder::FindFirstMissilePositionThatIsReallyGuided(YsVec3 &vec,YsAtt3 &att) const
{
	double maxLifeRemain;
	YSRESULT res;
	FsWeapon *seeker;

	maxLifeRemain=0.0;
	res=YSERR;

	for(seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		if(seeker->lifeRemain>maxLifeRemain)
		{
			if(((seeker->type==FSWEAPON_AIM9 || seeker->type==FSWEAPON_AIM9X || seeker->type==FSWEAPON_AGM65 ||
			     seeker->type==FSWEAPON_AIM120) && seeker->target!=NULL) ||
		        (seeker->type==FSWEAPON_BOMB || seeker->type==FSWEAPON_BOMB250 || seeker->type==FSWEAPON_BOMB500HD))
			{
				vec=seeker->pos;
				att=seeker->att;
				maxLifeRemain=seeker->lifeRemain;
				res=YSOK;
			}
		}
	}
	return res;
}

YSRESULT FsWeaponHolder::FindOldestMissilePosition(YsVec3 &vec,YsAtt3 &att,const FsExistence *fired) const
{
	YSRESULT res;
	FsWeapon *seeker;

	res=YSERR;
	for(seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		if(seeker->firedBy==fired && seeker->lifeRemain>YsTolerance)
		{
			if(((seeker->type==FSWEAPON_AIM9 || seeker->type==FSWEAPON_AIM9X || seeker->type==FSWEAPON_AGM65 ||
			     seeker->type==FSWEAPON_AIM120) && seeker->target!=NULL) ||
		        (seeker->type==FSWEAPON_BOMB || seeker->type==FSWEAPON_BOMB250 || seeker->type==FSWEAPON_BOMB500HD))
			{
				vec=seeker->pos;
				att=seeker->att;
				res=YSOK;
			}
		}
	}
	return res;
}

YSRESULT FsWeaponHolder::FindNewestMissilePosition(YsVec3 &vec,YsAtt3 &att,const FsExistence *fired) const
{
	FsWeapon *seeker;
	for(seeker=activeList; seeker!=NULL; seeker=seeker->next)  // First one in the active list is the newest one.
	{
		if(seeker->firedBy==fired && seeker->lifeRemain>YsTolerance)
		{
			if(((seeker->type==FSWEAPON_AIM9 || seeker->type==FSWEAPON_AIM9X || seeker->type==FSWEAPON_AGM65 ||
			     seeker->type==FSWEAPON_AIM120) && seeker->target!=NULL) ||
		        (seeker->type==FSWEAPON_BOMB || seeker->type==FSWEAPON_BOMB250 || seeker->type==FSWEAPON_BOMB500HD))
			{
				vec=seeker->pos;
				att=seeker->att;
				return YSOK;
			}
		}
	}
	return YSERR;
}

const FsWeapon *FsWeaponHolder::FindNextActiveWeapon(const FsWeapon *wpn) const
{
	if(wpn==NULL)
	{
		return activeList;
	}
	else
	{
		return wpn->next;
	}
}

const FsWeapon *FsWeaponHolder::GetWeapon(int id) const
{
	if(0<=id && id<NumBulletBuffer)
	{
		return &buf[id];
	}
	else
	{
		return NULL;
	}
}

void FsWeaponHolder::ObjectIsDeleted(FsExistence *obj) const
{
	FsWeapon *wpn;
	for(wpn=activeList; wpn!=NULL; wpn=wpn->next)
	{
		if(wpn->firedBy==obj)
		{
			wpn->firedBy=NULL;
		}
		if(wpn->target==obj)
		{
			wpn->target=NULL;
		}
	}
}

void FsWeaponHolder::Move(const double &dt,const double &cTime,const FsWeather &weather)
{
	FsWeapon *seeker,*nxt;
	FSWEAPONTYPE type;

	FsWeapon *flareList;
	flareList=NULL;
	for(seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		if(seeker->type==FSWEAPON_FLARE)
		{
			seeker->prevFlare=NULL;
			seeker->nextFlare=flareList;
			if(seeker->nextFlare!=NULL)
			{
				seeker->nextFlare->prevFlare=seeker;
			}
			flareList=seeker;
		}
	}

	for(seeker=activeList; seeker!=NULL; seeker=nxt)
	{
		nxt=seeker->next;
		type=seeker->type;
		seeker->Move(dt,cTime,weather,flareList);
		if(seeker->lifeRemain<=YsTolerance && seeker->timeRemain<=YsTolerance)
		{
			MoveToFreeList(seeker);
		}
	}
}

void FsWeaponHolder::HitGround(
    const double &ctime,const class FsField &field,
    FsExplosionHolder *explosion,class FsSimulation *sim)
{
	FsWeapon *seeker,*nxt;
	for(seeker=activeList; seeker!=NULL; seeker=nxt)
	{
		nxt=seeker->next;

		seeker->HitGround(this,ctime,field,explosion,sim,killCredit);
		if(seeker->lifeRemain<=YsTolerance && seeker->timeRemain<=YsTolerance)
		{
			MoveToFreeList(seeker);
			goto NEXT;
		}

	NEXT:
		;
	}
}

void FsWeaponHolder::HitObject(
    const double &ctime,FsExplosionHolder *explo,FsSimulation *sim,const double &/*tallestGroundObjectHeight*/)
{
	int nAir,nGnd;
	FsAirplane *air;
	FsGround *gnd;
	FsWeapon *seeker,*nxt;

	nAir=sim->GetNumAirplane();
	nGnd=sim->GetNumGround();

	for(seeker=activeList; seeker!=NULL; seeker=nxt)
	{
		nxt=seeker->next;

		/* Smarter version */
		YsArray <FsAirplane *,256> airCandidate;
		YsArray <FsGround *,256> gndCandidate;
		sim->GetLattice().GetAirCollisionCandidate(airCandidate,seeker->pos,seeker->lastChecked);
		sim->GetLattice().GetGndCollisionCandidate(gndCandidate,seeker->pos,seeker->lastChecked);

		for(int j=0; j<airCandidate.GetN(); j++)
		{
			air=airCandidate[j];
			if(air->Prop().IsAlive()==YSTRUE)
			{
				if(seeker->HitObject(this,ctime,*air,explo,sim,killCredit)==YSTRUE)
				{
					if(air->Prop().IsActive()==YSTRUE)
					{
						if(air==sim->GetPlayerAirplane())
						{
							FsSoundSetOneTime(FSSND_ONETIME_DAMAGE);
						}
						else
						{
							FsSoundSetOneTime(FSSND_ONETIME_HIT);
						}
					}
					else
					{
						FsSoundSetOneTime(FSSND_ONETIME_BLAST);
					}
				}
			}
		}

		for(int j=0; j<gndCandidate.GetN(); j++)
		{
			gnd=gndCandidate[j];
			if(gnd->Prop().IsAlive()==YSTRUE)
			{
				if(seeker->HitObject(this,ctime,*gnd,explo,sim,killCredit)==YSTRUE)
				{
					if(gnd->Prop().IsAlive()==YSTRUE)
					{
						FsSoundSetOneTime(FSSND_ONETIME_HIT);
					}
					else
					{
						FsSoundSetOneTime(FSSND_ONETIME_BLAST);
					}
				}

			}
		}


		/* Exhaustive Search Version
		//for(j=0; j<nAir; j++)
		//{
		//	air=sim->GetAirplane(j);
		//	if(air->Prop().IsAlive()==YSTRUE)
		//	{
		//		if(seeker->HitObject(ctime,*air,explo,sim)==YSTRUE)
		//		{
		//			if(air->Prop().IsActive()==YSTRUE)
		//			{
		//				if(air==sim->GetPlayerAirplane())
		//				{
		//					fsAirsound->PlayOneShot(FSSND_DAMAGE);
		//				}
		//				else
		//				{
		//					fsAirsound->PlayOneShot(FSSND_HIT);
		//				}
		//			}
		//			else
		//			{
		//				fsAirsound->PlayOneShot(FSSND_BLAST);
		//			}
		//		}
		//	}
		//}
        //
		//if(seeker->pos.y()<tallestGroundObjectHeight || seeker->prv.y()<tallestGroundObjectHeight)
		//{
		//	for(j=0; j<nGnd; j++)
		//	{
		//		gnd=sim->GetGround(j);
		//		if(gnd->Prop().IsAlive()==YSTRUE)
		//		{
		//			if(seeker->HitObject(ctime,*gnd,explo,sim)==YSTRUE)
		//			{
		//				if(gnd->Prop().IsAlive()==YSTRUE)
		//				{
		//					fsAirsound->PlayOneShot(FSSND_HIT);
		//				}
		//				else
		//				{
		//					fsAirsound->PlayOneShot(FSSND_BLAST);
		//				}
		//			}
		//		}
		//	}
		//}
		*/

		seeker->lastChecked=seeker->pos;

		if(seeker->lifeRemain<=YsTolerance && seeker->timeRemain<=YsTolerance)
		{
			MoveToFreeList(seeker);
		}
	}
}

void FsWeaponHolder::Draw(
    YSBOOL coarse,const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,
    YSBOOL transparency,FSSMOKETYPE smk,const double &cTime,unsigned int drawFlag) const
{
	FsWeapon *seeker;
	for(seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		seeker->Draw(coarse,viewMat,projMat,transparency,smk,cTime,drawFlag);
	}
}

void FsWeaponHolder::CollectRecord(void)
{
	FsRecord <FsWeaponRecord> *neo;

	neo=new FsRecord <FsWeaponRecord>;
	//fsConsole.Printf("  A\n");
	if(neo!=NULL)
	{
		int nToSave,nToPlay;
		nToSave=(toSave!=NULL ? toSave->GetNumRecord() : 0);
		nToPlay=(toPlay!=NULL ? toPlay->GetNumRecord() : 0);

		int i,j;
		i=0;
		j=0;
		//fsConsole.Printf("  B\n");
		while(i<nToSave || j<nToPlay)
		{
			FsWeaponRecord *s,*p;
			double st=0.0,pt=0.0;

			s=(toSave!=NULL ? toSave->GetElement(st,i) : NULL);
			p=(toPlay!=NULL ? toPlay->GetElement(pt,j) : NULL);

			if(s!=NULL && p==NULL)
			{
				neo->AddElement(*s,st);
				i++;
			}
			else if(s==NULL && p!=NULL)
			{
				neo->AddElement(*p,pt);
				j++;
			}
			else if(st<pt)
			{
				neo->AddElement(*s,st);
				i++;
			}
			else
			{
				neo->AddElement(*p,pt);
				j++;
			}

			if(p==NULL && s==NULL)
			{
				fsStderr.Printf("Something Unexpected Happens.\n");
				break;
			}
		}
		//fsConsole.Printf("  C\n");

		if(toSave!=NULL)
		{
			delete toSave;
			toSave=NULL;
		}
		if(toPlay!=NULL)
		{
			delete toPlay;
			toPlay=NULL;
		}

		//fsConsole.Printf("  D\n");
		toSave=new FsRecord <FsWeaponRecord>;;
		toPlay=neo;
	}
	//fsConsole.Printf("  E\n");
}

void FsWeaponHolder::PlayRecord(const double &t1,const double &dt)
{
	YSSIZE_T i,i1,i2;
	double t2,t;
	t2=t1+dt;

	if(toPlay!=NULL)
	{
		if(toPlay->GetIndexByTime(i1,i2,t1,t2)==YSOK)
		{
			for(i=i1; i<=i2; i++)
			{
				FsWeaponRecord *rec;
				rec=toPlay->GetElement(t,i);

				if(rec!=NULL)
				{
					LaunchRecord(*rec,t,YSFALSE,YSFALSE);   // No Record, No Transmit
				}
			}
		}
	}
}

void FsWeaponHolder::LaunchRecord(const FsWeaponRecord &rec,const double &t,YSBOOL recordIt,YSBOOL transmit)
{
	YsVec3 pos;
	YsAtt3 att;
	pos.Set(rec.x,rec.y,rec.z);
	att.Set(rec.h,rec.p,rec.b);

	if(rec.type==FSWEAPON_GUN)
	{
		Fire
		    (t,
		     pos,
		     att,
		     rec.velocity,
		     rec.lifeRemain,
		     rec.power,
		     rec.firedBy,
		     recordIt,
		     transmit);
	}
	else if(rec.type==FSWEAPON_FLARE)
	{
		YsVec3 vel;
		vel=att.GetForwardVector();
		vel*=double(rec.velocity);
		DispenseFlare
		    (t,
		     pos,
		     vel,
		     rec.vMax,
		     rec.lifeRemain,
		     rec.firedBy,
		     recordIt,
		     transmit);
	}
	else
	{
		YSHASHKEY targetKey=(NULL!=rec.target ? FsExistence::GetSearchKey(rec.target) : YSNULLHASHKEY);
		Fire
		    (t,
		     rec.type,
		     pos,
		     att,
		     rec.velocity,
		     rec.vMax,
		     rec.lifeRemain,
		     rec.mobility,
		     rec.radar,
		     rec.power,
		     rec.firedBy,
		     targetKey,
		     recordIt,
		     transmit);

		if(rec.firedBy!=NULL && rec.firedBy->IsAirplane()==YSTRUE)
		{
			FsAirplane *air;
			air=(FsAirplane *)rec.firedBy;
			air->Prop().FireMissileByRecord(rec.type);
		}
	}
}

FsExistence *FsWeaponHolder::FindObjectByAxxGxxN(const char str[],const class FsSimulation *sim)
{
	if(str[0]=='A')
	{
		return sim->GetAirplaneById(atoi(str+1));
	}
	if(str[0]=='G')
	{
		return sim->GetGroundById(atoi(str+1));
	}
	else
	{
		return NULL;
	}
}


unsigned FsEncodeWeaponRecord(unsigned char dat[],FsSimulation *,const FsWeaponRecord &rec)
{
	unsigned char *ptr;
	ptr=dat;

	FsPushInt(ptr,FSNETCMD_MISSILELAUNCH);
	FsPushShort(ptr,(short)rec.type);
	FsPushFloat(ptr,rec.x);
	FsPushFloat(ptr,rec.y);
	FsPushFloat(ptr,rec.z);
	FsPushFloat(ptr,rec.h);
	FsPushFloat(ptr,rec.p);
	FsPushFloat(ptr,rec.b);
	FsPushFloat(ptr,rec.velocity);
	FsPushFloat(ptr,rec.lifeRemain);
	FsPushShort(ptr,(short)rec.power);

	int idOnSvr,type;

	idOnSvr=-1;
	type=0;
	if(rec.firedBy!=NULL)
	{
		if(rec.firedBy->GetType()==FSEX_AIRPLANE)
		{
			type=0;  // Airplane
			idOnSvr=FsExistence::GetSearchKey(rec.firedBy);
		}
		else if(rec.firedBy->GetType()==FSEX_GROUND)
		{
			type=1;
			idOnSvr=FsExistence::GetSearchKey(rec.firedBy);
		}
		else
		{
			type=0;
			idOnSvr=-1;
		}
	}
	FsPushInt(ptr,type);   // Airplane
	FsPushInt(ptr,idOnSvr);  // NULL


	if(rec.type==FSWEAPON_AGM65 ||
	   rec.type==FSWEAPON_AIM9 ||
	   rec.type==FSWEAPON_AIM9X ||
	   rec.type==FSWEAPON_AIM120 || // 2005/03/03
	   rec.type==FSWEAPON_ROCKET)
	{
		FsPushFloat(ptr,rec.vMax);
		FsPushFloat(ptr,rec.mobility);
		FsPushFloat(ptr,rec.radar);

		idOnSvr=-1;
		type=0;
		if(rec.target!=NULL)
		{
			if(rec.target->GetType()==FSEX_AIRPLANE)
			{
				type=0;
				idOnSvr=FsExistence::GetSearchKey(rec.target);
			}
			else if(rec.target->GetType()==FSEX_GROUND)
			{
				type=1;
				idOnSvr=FsExistence::GetSearchKey(rec.target);
			}
			else
			{
				idOnSvr=-1;
				type=0;
			}
		}
		FsPushInt(ptr,type);   // Airplane
		FsPushInt(ptr,idOnSvr);  // NULL
	}
	else if(rec.type==FSWEAPON_FLARE)
	{
		FsPushFloat(ptr,rec.vMax);
	}

	return (unsigned int)(ptr-dat);
}

YSRESULT FsDecodeWeaponRecord
    (FsWeaponRecord &rec,
     int &firedBy,YSBOOL &firedByAirplane,int &firedAt,YSBOOL &firedAtAirplane,
     unsigned char dat[],FsSimulation *)
{
	const unsigned char *ptr=dat;

	FsPopInt(ptr);
	rec.type=(FSWEAPONTYPE)FsPopShort(ptr);
	rec.x=FsPopFloat(ptr);
	rec.y=FsPopFloat(ptr);
	rec.z=FsPopFloat(ptr);
	rec.h=FsPopFloat(ptr);
	rec.p=FsPopFloat(ptr);
	rec.b=FsPopFloat(ptr);
	rec.velocity=FsPopFloat(ptr);
	rec.lifeRemain=FsPopFloat(ptr);
	rec.power=FsPopShort(ptr);

	firedByAirplane=(FsPopInt(ptr)==0 ? YSTRUE : YSFALSE);
	firedBy=FsPopInt(ptr);

	if(rec.type==FSWEAPON_AGM65 ||
	   rec.type==FSWEAPON_AIM9 ||
	   rec.type==FSWEAPON_AIM9X ||
	   rec.type==FSWEAPON_AIM120 || // 2005/03/03
	   rec.type==FSWEAPON_ROCKET)
	{
		rec.vMax=FsPopFloat(ptr);
		rec.mobility=FsPopFloat(ptr);
		rec.radar=FsPopFloat(ptr);

		firedAtAirplane=(FsPopInt(ptr)==0 ? YSTRUE : YSFALSE);
		firedAt=FsPopInt(ptr);
	}
	else if(rec.type==FSWEAPON_FLARE)
	{
		rec.vMax=FsPopFloat(ptr);
	}

	return YSOK;
}

void FsWeaponHolder::SetNetServer(class FsSocketServer *svr)
{
	netServer=svr;
}

void FsWeaponHolder::SetNetClient(class FsSocketClient *cli)
{
	netClient=cli;
}


int FsGetDefaultWeaponLoadingUnit(FSWEAPONTYPE wpnType)
{
	switch(wpnType)
	{
	default:
		return 1;
	case FSWEAPON_ROCKET:
		return 19;
	case FSWEAPON_FLARE:
		return 20;
	}
}

int FsGetDefaultSubUnitPerLoadingUnit(FSWEAPONTYPE wpnType)
{
	switch(wpnType)
	{
	default:
		return 1;
	case FSWEAPON_FUELTANK:
		return 800;
	}
}

FSWEAPONTYPE FsGetWeaponTypeByString(const char strIn[])
{
	YsString str(strIn);
	int i;
	for(i=0; i<str.Strlen(); i++)
	{
		if(str[i]=='*' || str[i]=='&' || str[i]=='|' || str[i]=='@')
		{
			str.SetLength(i);
			break;
		}
	}

	if(strcmp(str,"AIM9")==0)
	{
		return FSWEAPON_AIM9;
	}
	if(strcmp(str,"AGM65")==0)
	{
		return FSWEAPON_AGM65;
	}
	if(strcmp(str,"B500")==0)
	{
		return FSWEAPON_BOMB;
	}
	if(strcmp(str,"RKT")==0)
	{
		return FSWEAPON_ROCKET;
	}
	if(strcmp(str,"FLR")==0)
	{
		return FSWEAPON_FLARE;
	}
	if(strcmp(str,"AIM120")==0)
	{
		return FSWEAPON_AIM120;
	}
	if(strcmp(str,"B250")==0)
	{
		return FSWEAPON_BOMB250;
	}
	if(strcmp(str,"GUN")==0)
	{
		return FSWEAPON_GUN;
	}
	if(strcmp(str,"SMK")==0)
	{
		return FSWEAPON_SMOKE;
	}
	if(strcmp(str,"B500HD")==0)
	{
		return FSWEAPON_BOMB500HD;
	}
	if(strcmp(str,"AIM9X")==0)
	{
		return FSWEAPON_AIM9X;
	}
	if(strcmp(str,"IFLR")==0)
	{
		return FSWEAPON_FLARE_INTERNAL;
	}
	if(strcmp(str,"FUEL")==0)
	{
		return FSWEAPON_FUELTANK;
	}

	return FSWEAPON_NULL;
}

const char *FsGetWeaponString(FSWEAPONTYPE wpnType)
{
	switch(wpnType)
	{
	case FSWEAPON_GUN:
		return "GUN";
	case FSWEAPON_AIM9:
		return "AIM9";
	case FSWEAPON_AGM65:
		return "AGM65";
	case FSWEAPON_BOMB:
		return "B500";
	case FSWEAPON_ROCKET:
		return "RKT";
	case FSWEAPON_FLARE:
		return "FLR";
	case FSWEAPON_AIM120:
		return "AIM120";
	case FSWEAPON_BOMB250:
		return "B250";
	case FSWEAPON_SMOKE:
		return "SMK";
	case FSWEAPON_BOMB500HD:
		return "B500HD";
	case FSWEAPON_AIM9X:
		return "AIM9X";
	case FSWEAPON_FLARE_INTERNAL:
		return "IFLR";
	case FSWEAPON_FUELTANK:
		return "FUEL";
	default:
		return "NULL";
	}
}

void FsWeaponHolder::AddToParticleManager(class YsGLParticleManager &partMan,const double cTime) const
{
	for(auto seeker=activeList; seeker!=NULL; seeker=seeker->next)
	{
		seeker->AddToParticleManager(partMan,cTime);
	}
}
