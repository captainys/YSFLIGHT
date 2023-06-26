#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <ysclass.h>
#include <ysunitconv.h>

#include "fshud.h"
#include "graphics/common/fsopengl.h"
#include "fswirefont.h"
#include "fsinstreading.h"

YsArray <double> FsHeadUpDisplay::trigonomyTable;

FsHeadUpDisplay::FsHeadUpDisplay()
{
	hudCol.SetIntRGB(100,255,100);

	if(trigonomyTable.GetN()<721*2)
	{
		int i;
		for(i=0; i<=720; i++)
		{
			double a;
			a=YsDegToRad((double)i);
			trigonomyTable.Append(cos(a));
			trigonomyTable.Append(sin(a));
		}
	}
}

void FsHeadUpDisplay::SetArea(long x1,long y1,long dx,long dy)
{
	lupX=x1;
	lupY=y1;
	wid=dx;
	hei=dy;
}

void FsHeadUpDisplay::SetAreaByCenter(long cx,long cy,long dx,long dy)
{
	lupX=cx-dx/2;
	lupY=cy-dy/2;
	wid=dx;
	hei=dy;
}

void FsHeadUpDisplay::Draw(YSBOOL autoPilot,const class FsCockpitIndicationSet &cockpitIndicationSet)
{
	const FsInstrumentIndication &inst=cockpitIndicationSet.inst;

	const YsAtt3 attitudeIndication(-inst.heading,inst.pitch,inst.bank);

	DrawCrossHair();
	DrawHeading(attitudeIndication,YSTRUE,-inst.headingBug,inst.headingBugSelected);
	DrawThrottle(inst.nEngine,inst.engineOutput,inst.afterBurner);
	if(inst.hasVectorThrust==YSTRUE)
	{
		DrawNozzle(inst.nozzleDirection);
	}
	DrawFuelLeft(inst.fuelRemain[0],inst.fuelCapacity[0]);
	DrawG(inst.gForce);
	DrawWeapon(cockpitIndicationSet.ammo);
	DrawMach(inst.mach);
	DrawElevator(inst.elevator,inst.elevatorTrim,YSFALSE);
	DrawAileron(inst.aileron,YSFALSE);
	DrawRudder(inst.rudder);
	DrawSpeed(YsUnitConv::MPStoKT(inst.airSpeed));
	DrawAltitude(YsUnitConv::MtoFT(inst.altitude));
	DrawGearFlapBrake(inst.gearPos,
	                  inst.brake,
	                  inst.flaps,
	                  inst.spoiler,
	                  autoPilot);

	DrawClimbRatio(YsUnitConv::MtoFT(inst.verticalSpeed)*60.0);
	DrawTurnAndSlipIndicator(inst.sideSlip,inst.turnRate);

	DrawBank(inst.bank);
}

void FsHeadUpDisplay::DrawWeapon(const FsAmmunitionIndication &ammo)
{
	int x,y;

	x=lupX-64;
	y=lupY;

	for(auto &a : ammo.ammoArray)
	{
		YsColor col;
		auto str=a.FormatString();
		if(YSTRUE==a.ReadyToFire())
		{
			col=hudCol;
		}
		else
		{
			col=YsRed();
		}
		FsDrawString(x,y,str,hudCol);
		y+=FONTPITCH;
	}
}

void FsHeadUpDisplay::DrawG(double g)
{
	char str[256];
	int x,y;

	x=lupX+wid/12-48;
	y=lupY+hei*8/12+FONTPITCH;

	sprintf(str,"%3.1lfG",g);
	if(g<-5.0 || g>9.0)
	{
		FsDrawString(x,y,str,YsRed());
	}
	else if(g<-3.0 || g>6.0)
	{
		FsDrawString(x,y,str,YsYellow());
	}
	else
	{
		FsDrawString(x,y,str,hudCol);
	}
}

void FsHeadUpDisplay::DrawMach(double mach)
{
	char str[256];
	int x,y;

	x=lupX+wid/12-48;
	y=lupY+hei*8/12;

	sprintf(str,"%3.1lfM",mach);
	FsDrawString(x,y,str,hudCol);
}

void FsHeadUpDisplay::DrawFuelLeft(double fuel,double maxFuel)
{
	double ratio;
	char str[256];
	int x,y;

	ratio=100.0*fuel/maxFuel;
	sprintf(str,"FUEL:%6.2lf%%",ratio);

	x=lupX+wid/12-48;
	y=lupY+hei*10/12+FONTPITCH;

	if(ratio>=10.0)
	{
		FsDrawString(x,y,str,hudCol);
	}
	else if(ratio>=5.0)
	{
		FsDrawString(x,y,str,YsYellow());
	}
	else
	{
		FsDrawString(x,y,str,YsRed());
	}
}

void FsHeadUpDisplay::DrawGearFlapBrake(double ldg,double brk,double flp,double abr,YSBOOL autoPilot)
{
	int x,y;
	char str[128];

	x=lupX+wid*11/12;
	y=lupY+hei*8/12-FONTPITCH;
	if(ldg>=1.0-YsTolerance)
	{
		FsDrawString(x,y,"LDG:DWN",hudCol);
	}
	else if(ldg<=YsTolerance)
	{
		FsDrawString(x,y,"LDG:UP",hudCol);
	}
	else if(ldg>=0.75)
	{
		FsDrawString(x,y,"LDG:###",YsYellow());
	}
	else if(ldg>=0.50)
	{
		FsDrawString(x,y,"LDG:##",YsYellow());
	}
	else if(ldg>=0.25)
	{
		FsDrawString(x,y,"LDG:#",YsYellow());
	}
	else
	{
		FsDrawString(x,y,"LDG:",YsYellow());
	}

	y+=FONTPITCH;
	if(0.8<brk)
	{
		FsDrawString(x,y,"BRK:ON",hudCol);
	}
	else
	{
		FsDrawString(x,y,"BRK:OFF",hudCol);
	}

	y+=FONTPITCH;
	sprintf(str,"FLP:%d%%",(int)(flp*100.0F));
	FsDrawString(x,y,str,hudCol);

	y+=FONTPITCH;
	sprintf(str,"SPL:%d%%",(int)(abr*100.0F));
	FsDrawString(x,y,str,hudCol);

	if(YSTRUE==autoPilot)
	{
		y+=FONTPITCH;
		FsDrawString(x,y,"AUTO",hudCol);
	}
}

void FsHeadUpDisplay::DrawCircularBackground(int cx,int cy,int rad)
{
	FsDrawCircle(cx,cy,rad,YsBlack(),YSTRUE);
	FsDrawCircle(cx,cy,rad,YsWhite(),YSFALSE);
}

void FsHeadUpDisplay::DrawCircularFrame(int cx,int cy,int rad,const YsColor &col)
{
	FsDrawCircle(cx,cy,rad,col,YSFALSE);
}

void FsHeadUpDisplay::DrawIls(
    int cx,int cy,int outRad,int inRad,const YsColor &col,
    YSBOOL isIls, // YSFALSE -> VOR
    const double &lc,const double &gs,const double &obs,const char stationName[],int toFrom,
    YSBOOL isDme,const double &dme,YSBOOL currentSign)
{
	int i,x1,y1,x2,y2;
	double gsBracket,lcBracket;
	double a,c,s;

	char str[256];

	if(currentSign==YSTRUE)
	{
		x1=cx-outRad*8/9;
		y1=cy+outRad*8/9;
		FsDrawRect(x1-outRad/9,y1-outRad/9,x1+outRad/9,y1+outRad/9,col,YSTRUE);
	}

	int fontWid,fontHei;
	fontWid=(int)(inRad/8.0);
	fontHei=(int)(inRad/6.0);

	for(i=5; i<20; i+=5)
	{
		a=YsDegToRad((double)i);
		c=cos(a);
		s=sin(a);

		x1=cx-inRad+(int)(c*1.6*(double)inRad);
		y1=cy      +(int)(s*1.6*(double)inRad);
		x2=cx-inRad+(int)(c*1.6*(double)outRad);
		y2=cy      +(int)(s*1.6*(double)outRad);
		FsDrawLine(x1,y1,x2,y2,col);

		y1=cy      -(int)(s*1.6*(double)inRad);
		y2=cy      -(int)(s*1.6*(double)outRad);
		FsDrawLine(x1,y1,x2,y2,col);
	}
	x1=cx-inRad+(int)(1.6*(double)inRad);
	y1=cy;
	x2=cx-inRad+(int)(1.65*(double)outRad);
	y2=cy;
	FsDrawLine(x1,y1,x2,y2,col);


	for(i=5; i<20; i+=5)
	{
		a=YsDegToRad((double)i);
		c=cos(a);
		s=sin(a);

		x1=cx      +(int)(s*1.4*(double)inRad);
		y1=cy-inRad+(int)(c*1.4*(double)inRad);
		x2=cx      +(int)(s*1.4*(double)outRad);
		y2=cy-inRad+(int)(c*1.4*(double)outRad);
		FsDrawLine(x1,y1,x2,y2,col);

		x1=cx      -(int)(s*1.4*(double)inRad);
		x2=cx      -(int)(s*1.4*(double)outRad);
		FsDrawLine(x1,y1,x2,y2,col);
	}
	x1=cx;
	y1=cy-inRad+(int)(1.4*(double)inRad);
	x2=cx;
	y2=cy-inRad+(int)(1.45*(double)outRad);
	FsDrawLine(x1,y1,x2,y2,col);


	strncpy(str,stationName,10);
	str[10]=0;
	// FsDrawWireFont2D(cx,cy+inRad-fontHei*3/2,str,col,fontWid,fontHei);
	FsDrawString(cx-fontWid*6+2,cy+inRad-fontHei/2-2,str,col);
	FsDrawRect(cx-fontWid*6,cy+inRad-fontHei/2,cx+fontWid*6,cy+inRad-fontHei*5/2,col,YSFALSE);

	sprintf(str,"%03d",(int)YsRadToDeg(YsPi*2.0-obs));
	// FsDrawWireFont2D(cx-fontWid*4,cy-fontHei,str,col,fontWid,fontHei);
	FsDrawString(cx-fontWid*6+2,cy-2,str,col);
	FsDrawRect(cx-fontWid*6,cy-fontHei*2,cx-fontWid*2,cy,col,YSFALSE);


	if(toFrom!=0)
	{
		int tri[6];
		tri[0]=inRad/4;
		tri[1]=inRad/10;
		tri[2]=inRad/2;
		tri[3]=inRad/10;
		tri[4]=(tri[0]+tri[2])/2;
		tri[5]=inRad/5;

		if(toFrom>0)
		{
			tri[1]=-tri[1];
			tri[3]=-tri[3];
			tri[5]=-tri[5];
		}

		tri[0]+=cx;
		tri[1]+=cy;
		tri[2]+=cx;
		tri[3]+=cy;
		tri[4]+=cx;
		tri[5]+=cy;

		FsDrawPolygon(3,tri,col);
	}


	if(isIls==YSTRUE) // i.e., has Glide Slope
	{
		// FsDrawWireFont2D(cx-fontWid*4,cy+fontHei,"GS",col,fontWid,fontHei);
		FsDrawString(cx-fontWid*5,cy+fontHei,"GS",col);
		gsBracket=YsBound(gs,-YsPi/60.0,YsPi/60.0);
		DrawNeedle(cx-inRad,cy,1,inRad*3/2,YsPi/2.0-gsBracket*10.0,2);
	}
	else
	{
		DrawNeedle(cx-inRad,cy,1,inRad*3/2,YsPi/2.0,2);
	}

	if(isDme==YSTRUE)
	{
		sprintf(str,"%.1lfNM",dme/1852.0);
		// FsDrawWireFont2D(cx,cy-inRad+fontHei*2,str,col,fontWid,fontHei);
		FsDrawString(cx-fontWid*3,cy-inRad+fontHei*3-2,str,col);
	}
	else
	{
		// FsDrawWireFont2D(cx,cy-inRad+fontHei*2,"**",col,fontWid,fontHei);
		FsDrawString(cx-fontWid*3,cy-inRad+fontHei*3-2,"**",col);
	}
	FsDrawRect(cx-fontWid*6,cy-inRad+fontHei,cx+fontWid*6,cy-inRad+fontHei*3,col,YSFALSE);

	lcBracket=YsBound(lc,-YsPi/18.0,YsPi/18.0);
	DrawNeedle(cx,cy-inRad,1,inRad*3/2,YsPi-lcBracket*3.0,2);
}

void FsHeadUpDisplay::DrawVor(
	int cx,int cy,int outRad,int inRad,const YsColor &col,
    YSBOOL /*isILS*/,
    const double &lc,const double &obs,const char stationName[],int toFrom,
    YSBOOL isDme,const double &dme,YSBOOL currentSign)
{
	int i,x1,y1,x2,y2;
	double a,c,s;

	char str[256];

	if(currentSign==YSTRUE)
	{
		x1=cx-outRad*8/9;
		y1=cy+outRad*8/9;
		FsDrawRect(x1-outRad/9,y1-outRad/9,x1+outRad/9,y1+outRad/9,col,YSTRUE);
	}

	int fontWid,fontHei;
	fontWid=(int)(inRad/8.0);
	fontHei=(int)(inRad/6.0);

	strncpy(str,stationName,10);
	str[10]=0;
	// FsDrawWireFont2D(cx,cy+inRad-fontHei*3/2,str,col,fontWid,fontHei);
	FsDrawString(cx-fontWid*6+2,cy+inRad-fontHei/2-2,str,col);
	FsDrawRect(cx-fontWid*6,cy+inRad-fontHei/2,cx+fontWid*6,cy+inRad-fontHei*5/2,col,YSFALSE);

	sprintf(str,"%03d",(int)YsRadToDeg(YsPi*2.0-obs));
	// FsDrawWireFont2D(cx-fontWid*4,cy,str,col,fontWid,fontHei);
	FsDrawString(cx-fontWid*5,cy+fontHei-2,str,col);
	FsDrawRect(cx-fontWid*6,cy-fontHei,cx-fontWid*2,cy+fontHei,col,YSFALSE);


	if(toFrom>0)
	{
		// FsDrawWireFont2D(cx+fontWid*4,cy,"TO",col,fontWid,fontHei);
		FsDrawString(cx+fontWid*3,cy+fontHei-2,"TO",col);
	}
	else if(toFrom<0)
	{
		// FsDrawWireFont2D(cx+fontWid*4,cy,"FR",col,fontWid,fontHei);
		FsDrawString(cx+fontWid*3,cy+fontHei-2,"FR",col);
	}
	FsDrawRect(cx+fontWid*2,cy-fontHei,cx+fontWid*6,cy+fontHei,col,YSFALSE);


	if(isDme==YSTRUE)
	{
		sprintf(str,"%.1lfNM",dme/1852.0);
		// FsDrawWireFont2D(cx,cy-inRad+fontHei*2,str,col,fontWid,fontHei);
		FsDrawString(cx-fontWid*3,cy-inRad+fontHei*3-2,str,col);
	}
	FsDrawRect(cx-fontWid*6,cy-inRad+fontHei,cx+fontWid*6,cy-inRad+fontHei*3,col,YSFALSE);


	for(i=5; i<20; i+=5)
	{
		a=YsDegToRad((double)i);
		c=cos(a);
		s=sin(a);

		x1=cx      +(int)(s*1.4*(double)inRad);
		y1=cy-inRad+(int)(c*1.4*(double)inRad);
		x2=cx      +(int)(s*1.4*(double)outRad);
		y2=cy-inRad+(int)(c*1.4*(double)outRad);
		FsDrawLine(x1,y1,x2,y2,col);

		x1=cx      -(int)(s*1.4*(double)inRad);
		x2=cx      -(int)(s*1.4*(double)outRad);
		FsDrawLine(x1,y1,x2,y2,col);
	}
	x1=cx;
	y1=cy-inRad+(int)(1.4*(double)inRad);
	x2=cx;
	y2=cy-inRad+(int)(1.5*(double)outRad);
	FsDrawLine(x1,y1,x2,y2,col);

	double dev;
	dev=YsBound(lc,-YsPi/18.0,YsPi/18.0);
	DrawNeedle(cx,cy-inRad,1,inRad*3/2,YsPi-dev*3.0,2);
}

void FsHeadUpDisplay::DrawAdf(int cx,int cy,int rad,const YsColor &col,const double &relHdg,const char stationName[],YSBOOL currentSign)
{
	int ang;

	if(currentSign==YSTRUE)
	{
		int x1,y1;
		x1=cx-rad*8/9;
		y1=cy+rad*8/9;
		FsDrawRect(x1-rad/9,y1-rad/9,x1+rad/9,y1+rad/9,col,YSTRUE);
	}

	for(ang=0; ang<90; ang+=5)
	{
		int l,x1,y1,x2,y2;
		double r,s,c,dx,dy;

		if(ang%45==0)
		{
			l=85;
		}
		else if(ang%10==0)
		{
			l=95;
		}
		else if(ang%5==0)
		{
			l=98;
		}

		r=(double)ang*YsPi/180.0;
		s=sin(r);
		c=cos(r);
		dx=c*(double)rad;
		dy=s*(double)rad;

		x1=(int)dx;
		y1=(int)dy;
		x2=x1*l/100;
		y2=y1*l/100;

		FsDrawLine(cx+x1,cy+y1,cx+x2,cy+y2,col);
		FsDrawLine(cx-y1,cy+x1,cx-y2,cy+x2,col);
		FsDrawLine(cx-x1,cy-y1,cx-x2,cy-y2,col);
		FsDrawLine(cx+y1,cy-x1,cx+y2,cy-x2,col);
	}

	char str[256];
	int fontWid,fontHei;
	fontWid=(int)(rad/8.0);
	fontHei=(int)(rad/6.0);

	strncpy(str,stationName,10);
	str[10]=0;
	// FsDrawWireFont2D(cx,cy+rad*4/5-fontHei*3/2,str,col,fontWid,fontHei);
	FsDrawString(cx-fontWid*6+2,cy+rad*4/5-fontHei/2-2,str,col);
	FsDrawRect(cx-fontWid*6,cy+rad*4/5-fontHei/2,cx+fontWid*6,cy+rad*4/5-fontHei*5/2,col,YSFALSE);

	DrawNeedle(cx,cy,2,rad*98/100,relHdg,rad*8/10);
}

void FsHeadUpDisplay::DrawTurnAndSlipIndicator(const double ssa,const double turnRate) const
{
	const float outRad=(float)(wid/12);
	const float inRad=(float)(outRad*9/10);
	const float cx=(float)lupX-outRad;
	const float cy=(float)lupY+hei/2;



	// Ball
	const float ballCirCx=cx;
	const float ballCirCy=cy-outRad*0.6f;
	const float ballCirRad=outRad;


	int a;


	const int maxNWindowVtx=14;
	int nWindowVtx=0;
	float windowVtx[maxNWindowVtx*3];
	for(a=-30; a<=30; a+=10)  // 7 steps total * 2 vtx each
	{
		const float c=(float)cos(YsDegToRad(a));
		const float s=(float)sin(YsDegToRad(a));

		windowVtx[nWindowVtx*3  ]=ballCirCx+s*ballCirRad*0.9f;
		windowVtx[nWindowVtx*3+1]=ballCirCy+c*ballCirRad*0.9f;
		windowVtx[nWindowVtx*3+2]=0.0f;

		windowVtx[(maxNWindowVtx-1-nWindowVtx)*3  ]=ballCirCx+s*ballCirRad*1.1f;
		windowVtx[(maxNWindowVtx-1-nWindowVtx)*3+1]=ballCirCy+c*ballCirRad*1.1f;
		windowVtx[(maxNWindowVtx-1-nWindowVtx)*3+2]=0.0f;

		++nWindowVtx;
	}
	if(2*nWindowVtx!=maxNWindowVtx)
	{
		printf("Boom!\n");
		printf("%s %d\n",__FUNCTION__,__LINE__);
		exit(1);
	}
	for(int i=0; i<2*nWindowVtx-1; ++i)
	{
		FsDrawLine((int)windowVtx[i*3],(int)(windowVtx[i*3+1]),(int)windowVtx[i*3+3],(int)(windowVtx[i*3+4]),hudCol);
	}
	FsDrawLine((int)windowVtx[(2*nWindowVtx-1)*3],(int)(windowVtx[(2*nWindowVtx-1)*3+1]),(int)windowVtx[0],(int)(windowVtx[1]),hudCol);



	const float lineVtx[8*3]=
	{
		cx-inRad,                     cy,                            0.0f,
		cx-outRad,                    cy,                            0.0f,
		cx+inRad,                     cy,                            0.0f,
		cx+outRad,                    cy,                            0.0f,

		cx-inRad*(float)YsCos20deg, cy+inRad*(float)YsSin20deg,  0.0f,
		cx-outRad*(float)YsCos20deg,cy+outRad*(float)YsSin20deg, 0.0f,
		cx+inRad*(float)YsCos20deg, cy+inRad*(float)YsSin20deg,  0.0f,
		cx+outRad*(float)YsCos20deg,cy+outRad*(float)YsSin20deg, 0.0f
	};
	for(int i=0; i<8; i+=2)
	{
		FsDrawLine((int)lineVtx[i*3],(int)(lineVtx[i*3+1]),(int)lineVtx[i*3+3],(int)(lineVtx[i*3+4]),hudCol);
	}




	float tilt,c,s,x,y;
	const float scl=ballCirRad*0.1f;

	tilt=YsBound <float> ((float)ssa*5.0f,-(float)YsPi/9.0f,(float)YsPi/9.0f);
	c=(float)cos(tilt);
	s=(float)sin(tilt);
	x=ballCirCx+s*ballCirRad;
	y=ballCirCy+c*ballCirRad;

	FsDrawCircle((int)x,(int)y,(int)scl,hudCol,YSFALSE);



	// 360deg/2min -> 20 deg tilt
	// 3deg/sec -> 20 deg tilt
	// 1deg/sec -> 20/3 deg tilt



	const float miniatureAirplaneVtx[25*3]=
	{
		-0.700000f,-0.010000f,0.0f,
		-0.700000f, 0.010000f,0.0f,
		-0.117694f, 0.023411f,0.0f,
		-0.099776f, 0.066668f,0.0f,
		-0.066668f, 0.099776f,0.0f,
		-0.023411f, 0.117694f,0.0f,
		-0.010000f, 0.300000f,0.0f,
		 0.010000f, 0.300000f,0.0f,
		 0.023411f, 0.117694f,0.0f,
		 0.066668f, 0.099776f,0.0f,
		 0.099776f, 0.066668f,0.0f,
		 0.117694f, 0.023411f,0.0f,
		 0.700000f, 0.010000f,0.0f,
		 0.700000f,-0.010000f,0.0f,
		 0.117694f,-0.023411f,0.0f,
		 0.110866f,-0.045922f,0.0f,
		 0.084853f,-0.084853f,0.0f,
		 0.045922f,-0.110866f,0.0f,
		 0.000000f,-0.120000f,0.0f,
		-0.045922f,-0.110866f,0.0f,
		-0.084853f,-0.084853f,0.0f,
		-0.110866f,-0.045922f,0.0f,
		-0.117694f,-0.023411f,0.0f,
		-0.700000f,-0.010000f,0.0f,
		-0.700000f,-0.010000f,0.0f,
	};
	int miniatureAirplaneVtxTfm[25*3];

	tilt=(float)turnRate*20.0f/3.0f;
	tilt=YsBound <float> (tilt,-(float)YsPi/3.0f,(float)YsPi/3.0f);

	for(int i=0; i<25; ++i)
	{
		YsVec2 vec(miniatureAirplaneVtx[i*3],miniatureAirplaneVtx[i*3+1]);
		vec.Rotate(tilt);
		vec*=inRad;
		miniatureAirplaneVtxTfm[i*3  ]=(int)cx+(int)vec.x();
		miniatureAirplaneVtxTfm[i*3+1]=(int)cy-(int)vec.y();
	}

	for(int i=0; i<24; ++i)
	{
		FsDrawLine(miniatureAirplaneVtxTfm[i*3],miniatureAirplaneVtxTfm[i*3+1],miniatureAirplaneVtxTfm[i*3+3],miniatureAirplaneVtxTfm[i*3+4],hudCol);
	}
}
