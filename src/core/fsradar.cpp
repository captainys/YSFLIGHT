#include "fsradar.h"
#include "graphics/common/fsopengl.h"



////////////////////////////////////////////////////////////

// Ground -> CROSS
// Air -> RECT
// Friendly -> White
// Hostile -> Green

void FsHorizontalRadar::Draw(
	 const FsSimulation *sim,int x1,int y1,int x2,int y2,const double &rangeInX,const FsAirplane &withRespectTo,
	 int mode,const double &airAltLimit) const
	// mode 0:air to air    1: air to gnd    2:bombing
{
	const double mag=double(YsAbs(x2-x1))/(rangeInX*1600.0); // Conversion from meters to miles
	const int mkSize=3;


	YsVec2 w1,w2,wc;
	w1.Set(x1,y1);
	w2.Set(x2,y2);
	if(mode==0)
	{
		wc=(w1+w2)/2.0;
	}
	else
	{
		wc.Set((w1.x()+w2.x())/2.0,(w1.y()+w2.y()*3.0)/4.0);
	}


	YsAtt3 attH;
	attH=withRespectTo.GetAttitude();
	attH.SetP(0.0);
	attH.SetB(0.0);


	YsMatrix4x4 ref;
	ref.Translate(withRespectTo.GetPosition());
	ref.Rotate(attH);

	DrawBasic(sim,x1,y1,x2,y2,rangeInX,withRespectTo,withRespectTo.GetPosition(),withRespectTo.GetAttitude(),mode,airAltLimit);

	switch(mode)
	{
	case 0:
	case 1:
		{
			double radar,dy,dx;
			YsVec2 wr;
			if(mode==0)
			{
				radar=withRespectTo.Prop().GetAAMRadarAngle();
			}
			else
			{
				radar=withRespectTo.Prop().GetAGMRadarAngle();
			}
			dy=double(YsAbs(y2-y1))/2.0;
			dx=dy*sin(radar);
			wr.Set(wc.x()-dx,w1.y());
			const int x =(int)wc.x();
			const int y =(int)wc.y();
			int xx=(int)wr.x();
			int yy=(int)wr.y();
			FsDrawLine(x,y,xx,yy,YsGreen());
			wr.Set(wc.x()+dx,w1.y());
			xx=(int)wr.x();
			yy=(int)wr.y();
			FsDrawLine(x,y,xx,yy,YsGreen());
		}
		break;
	case 2:
		{
			YsVec3 bomb;
			if(withRespectTo.Prop().ComputeEstimatedBombLandingPosition(bomb,sim->GetWeather())==YSOK)
			{
				YsVec2 prj;

				ref.MulInverse(bomb,bomb,1.0);
				bomb*=mag;
				prj.Set(bomb.x(),-bomb.z());
				prj+=wc;

				const int x=(int)prj.x();
				const int y=(int)prj.y();
				if(YsCheckInsideBoundingBox2(prj,w1,w2)==YSTRUE)
				{
					FsDrawDiamond(x,y,mkSize,YsGreen(),YSFALSE);
				}

				FsDrawLine((int)wc.x()-3,y1,(int)wc.x()-3,y2,YsGreen());
				FsDrawLine((int)wc.x()+3,y1,(int)wc.x()+3,y2,YsGreen());
			}
		}
		break;
	}
}

void FsHorizontalRadar::DrawBasic(
    const class FsSimulation *sim,int x1,int y1,int x2,int y2,const double &rangeInX,
    const FsExistence &withRespectTo,const YsVec3 &pos,const YsAtt3 &att,
    int mode,const double &airAltLimit) const
{
	const double mag=double(YsAbs(x2-x1))/(rangeInX*1600.0); // Conversion from meters to miles
	int x,y,xx,yy;
	YsAtt3 attH;
	YsMatrix4x4 ref;
	YsVec2 w1,w2,wc;
	YsColor col;
	const int mkSize=3;

	FsDrawRect(x1,y1,x2,y2,YsGreen(),YSFALSE);

	char str[256];
	sprintf(str,"%d MILES",int(rangeInX));
	FsDrawString(x1+8,y1+24,str,YsGreen());


	attH=att;
	attH.SetP(0.0);
	attH.SetB(0.0);

	ref.Translate(pos);
	ref.Rotate(attH);

	w1.Set(x1,y1);
	w2.Set(x2,y2);
	if(mode==0)
	{
		wc=(w1+w2)/2.0;
	}
	else
	{
		wc.Set((w1.x()+w2.x())/2.0,(w1.y()+w2.y()*3.0)/4.0);
	}

	x=(int)wc.x();
	y=(int)wc.y();
	FsDrawLine(x-5,y  ,x+5,y  ,YsGreen());
	FsDrawPoint(x+5,y,YsGreen());
	FsDrawLine(x  ,y-5,x  ,y+5,YsGreen());
	FsDrawPoint(x,y+5,YsGreen());


	const FsGround *gnd;
	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->IsAlive()==YSTRUE && gnd->Prop().IsNonGameObject()!=YSTRUE)
		{
			YsVec3 pos;
			YsVec2 prj;

			ref.MulInverse(pos,gnd->GetPosition(),1.0);

			pos*=mag;

			prj.Set(pos.x(),-pos.z());
			prj+=wc;

			if(YsCheckInsideBoundingBox2(prj,w1,w2)==YSTRUE)
			{
				if(withRespectTo.iff==gnd->iff)
				{
					col=YsWhite();
				}
				else
				{
					col=YsGreen();
				}

				FsDrawX((int)prj.x(),(int)prj.y(),mkSize,col);
			}
		}
	}

	const FsAirplane *air;
	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		if(air!=&withRespectTo && air->IsAlive()==YSTRUE)
		{
			double altLimit;
			altLimit=airAltLimit+1000.0*(1.0-air->Prop().GetRadarCrossSection());
			if(altLimit<air->GetPosition().y())
			{

				YsVec3 pos,vel;
				YsVec2 prj1,prj2;

				air->Prop().GetVelocity(vel);

				ref.MulInverse(pos,air->GetPosition(),1.0);
				ref.MulInverse(vel,vel,0.0);

				if(vel.Normalize()==YSOK)
				{
					vel*=8.0;
				}

				pos*=mag;
				vel+=pos;

				prj1.Set(pos.x(),-pos.z());
				prj2.Set(vel.x(),-vel.z());

				prj1+=wc;
				prj2+=wc;

				if(YsCheckInsideBoundingBox2(prj1,w1,w2)==YSTRUE)
				{
					if(air->Prop().IsActive()!=YSTRUE)
					{
						col=YsBlack();
					}
					else if(withRespectTo.iff==air->iff)
					{
						col=YsWhite();
					}
					else
					{
						col=YsGreen();
					}

					x=(int)prj1.x();
					y=(int)prj1.y();
					FsDrawRect(x-mkSize+1,y-mkSize+1,x+mkSize-1,y+mkSize-1,col,YSTRUE);

					if(YsCheckInsideBoundingBox2(prj2,w1,w2)==YSTRUE)
					{
						xx=(int)prj2.x();
						yy=(int)prj2.y();
						FsDrawLine(x,y,xx,yy,col);
					}
				}
			}
		}
	}

	const FsWeapon *wpn;
	wpn=NULL;
	while((wpn=sim->FindNextActiveWeapon(wpn))!=NULL)
	{
		if(wpn->lifeRemain>YsTolerance && wpn->timeRemain>YsTolerance)
		{
			if(wpn->type==FSWEAPON_AIM9 || wpn->type==FSWEAPON_AIM120)
			{
				col=YsRed();
			}
			else if(wpn->type==FSWEAPON_AIM9X)
			{
				col=YsBlue();
			}
			else if(wpn->type==FSWEAPON_AGM65 || wpn->type==FSWEAPON_ROCKET)
			{
				col=YsYellow();
			}
			else
			{
				continue;
			}

			YsVec3 pos;
			YsVec2 prj;

			ref.MulInverse(pos,wpn->pos,1.0);

			pos*=mag;

			prj.Set(pos.x(),-pos.z());

			prj+=wc;

			if(YsCheckInsideBoundingBox2(prj,w1,w2)==YSTRUE)
			{
				FsDrawPoint2Pix((int)prj.x(),(int)prj.y(),col);
			}
		}
	}
}
