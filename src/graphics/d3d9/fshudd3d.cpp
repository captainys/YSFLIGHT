#include <ysclass.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"




void FsHeadUpDisplay::DrawCrossHair(void)
{
	long cenX,cenY,lng,lat;

	cenX=lupX+wid/2;
	cenY=lupY+hei/2;

	lat=wid/20;
	lng=lat*2/3;

	/* ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX-lat,cenY,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX-4,cenY  ,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+lat,cenY,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+4,cenY  ,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX,cenY-lng,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX,cenY-4  ,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX,cenY+lng,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX,cenY+4  ,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST); */

	FsDrawLine(cenX-lat,cenY,cenX-4,cenY,hudCol);
	FsDrawLine(cenX+lat,cenY,cenX+4,cenY,hudCol);
	FsDrawLine(cenX,cenY-lng,cenX,cenY-4,hudCol);
	FsDrawLine(cenX,cenY+lng,cenX,cenY+4,hudCol);
}

void FsHeadUpDisplay::DrawHeading(const YsAtt3 &a,YSBOOL showHdgBug,const double &hdgBug,YSBOOL selected)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	long cenX,cenY;
	long deg,num;
	int s1x,s1y,s2x,s2y;
	double hdg,interval;

	cenX=lupX+wid/2;
	cenY=lupY+hei/6;

	interval=(double)wid/64;

	hdg=-YsRadToDeg(a.h());
	deg= (long)(hdg-12.0F);

	FsDrawLine(cenX,cenY+1,cenX,cenY+8,hudCol);

	s1y=cenY;
	while(deg<(long)hdg+12)
	{
		s2y=(deg%10==0 ? cenY-8 : cenY-2);

		s1x=cenX+(long)(((double)deg-hdg)*interval);
		s2x=s1x;

		if(deg%10==0)
		{
			s2y=cenY-8;

			num=(deg>0 ? deg/10 : (deg+360)/10);
			char str[3];
			str[0]=(char)('0'+num/10);
			str[1]=(char)('0'+num%10);
			str[2]=0;

			FsDrawString(s2x-12,s2y-12,str,hudCol);
		}
		FsDrawLine(s1x,s1y,s2x,s2y,hudCol);

		deg++;
	}



	if(YSTRUE==showHdgBug)
	{
		double relHdgBug=-YsRadToDeg(hdgBug-a.h());
		while(relHdgBug<-180.0)
		{
			relHdgBug+=360.0;
		}
		while(180.0<relHdgBug)
		{
			relHdgBug-=360.0;
		}

		relHdgBug=YsBound(relHdgBug,-12.0,12.0);
		int cx=cenX+(long)(relHdgBug*interval);
		int cy=cenY;

		if(YSTRUE!=selected)
		{
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx-12,cy-10,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx-12,cy,   0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx-4,cy,    0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx,cy+4,    0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx+4,cy,    0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx+12,cy,   0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx+12,cy-10,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cx-12,cy-10,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
		}
		else
		{
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx,cy-10,   0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx-12,cy-10,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx-12,cy,   0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx-4,cy,    0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx,cy+4,    0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx+4,cy,    0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx+12,cy,   0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cx+12,cy-10,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			ysD3dDev->FlushXyzCol(D3DPT_TRIANGLEFAN);
		}

		YsString bugTxt;
		int digit=(int)YsRadToDeg(-hdgBug);
		while(digit<0)
		{
			digit+=360;
		}
		while(360<digit)
		{
			digit-=360;
		}
		bugTxt.Printf("%03d",digit);
		FsDrawString(cx-14,cy+20,bugTxt,hudCol);
	}
}

void FsHeadUpDisplay::DrawThrottle(int nEng,const double thr[],const YSBOOL ab[])
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	int i;
	YSBOOL abr;
	long cenX,cenY,sizX,sizY,barY;
	double deg;
	YsColor col;


	cenX=lupX+wid/12;
	cenY=lupY+hei*10/12;

	sizX=wid/64;
	sizY=wid/16;

	for(i=0; i<nEng; i++)
	{
		deg=thr[i];
		abr=ab[i];

		barY=(long)(((double)sizY)*deg);

		if(abr==YSTRUE)
		{
			col=YsRed();
		}
		else
		{
			col=hudCol;
		}

		ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cenX+1     ,cenY     ,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cenX+sizX-1,cenY     ,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cenX+sizX-1,cenY-barY,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_TRIANGLEFAN,cenX+1     ,cenY-barY,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_TRIANGLEFAN);

		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cenX+1     ,cenY     ,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cenX+sizX-1,cenY     ,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cenX+sizX-1,cenY-sizY,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cenX+1     ,cenY-sizY,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,cenX+1     ,cenY     ,0.5,col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);

		cenX+=sizX;
	}
}

void FsHeadUpDisplay::DrawNozzle(const YsVec3 &vec)
{
	long cenX,cenY,sizX,dirX,dirY;

	cenX=lupX;
	cenY=lupY+hei*10/12-wid/64;

	sizX=wid/32;

	FsDrawString(cenX-24,cenY,"NZL",hudCol);

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX     ,cenY,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+sizX,cenY,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	dirX=int(vec.z()*double(sizX));
	dirY=int(vec.y()*double(sizX));
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+sizX     ,cenY     ,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+sizX+dirX,cenY+dirY,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
}

void FsHeadUpDisplay::DrawElevator(double elv,double trim,YSBOOL au)
{
	long cenX,cenY,sizX,sizY;
	YsColor col;

	if(elv>=0.0)
	{
		elv=sqrt(elv);
	}
	else
	{
		elv=-sqrt(-elv);
	}

	if(trim>=0.0)
	{
		trim=sqrt(trim);
	}
	else
	{
		trim=-sqrt(-trim);
	}

	cenX=lupX+wid*2/12;
	cenY=lupY+hei*10/12;

	sizX=wid/48;
	sizY=wid/16;

	cenY-=sizY/2;

	int s1x,s1y,s2x,s2y;
	s1x=cenX+sizX;
	s1y=cenY;
	s2x=cenX+sizX*2;
	s2y=cenY;


	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->AddXyzCol(D3DPT_POINTLIST,s1x,s1y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_POINTLIST,s2x,s2y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);


	if(au==YSTRUE)
	{
		col=YsYellow();
	}
	else
	{
		col=hudCol;
	}
	s1y=cenY-(long)((double)sizY*elv/2.0);
	s2y=s1y;
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s1x,s1y,0.5,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s2x,s2y,0.5,col.Ri(),col.Gi(),col.Bi(),255);


	s1y=cenY-(long)(trim*double(sizY)/2.0);
	s2y=s1y;
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s1x,s1y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s2x,s2y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	s1x=cenX+sizX*3/2;
	s1y=cenY-(long)(trim*double(sizY)/2.0);
	s2x=s1x;
	s2y=cenY;
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s1x,s1y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s2x,s2y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
}

void FsHeadUpDisplay::DrawAileron(double ail,YSBOOL au)
{
	long cenX,cenY,sizX,sizY;
	int s1x,s1y,s2x,s2y;
	YsColor col;

	if(ail>=0.0)
	{
		ail=sqrt(ail);
	}
	else
	{
		ail=-sqrt(-ail);
	}

	cenX=lupX+wid*2/12;
	cenY=lupY+hei*10/12;

	sizX=wid/48;
	sizY=wid/16;

	cenY-=sizY/2;

	s1x=cenX;
	s1y=cenY;
	s2x=cenX+sizX;
	s2y=cenY;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->AddXyzCol(D3DPT_POINTLIST,s1x,s1y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_POINTLIST,s2x,s2y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);


	if(au==YSTRUE)
	{
		col=YsYellow();
	}
	else
	{
		col=hudCol;
	}

	s1y=cenY-(long)((double)sizY*ail/2.0F);
	s2y=s1y;
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s1x,s1y,0.5,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s2x,s2y,0.5,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	s1x=cenX+sizX*2;
	s1y=cenY;
	s2x=cenX+sizX*3;
	s2y=cenY;

	ysD3dDev->AddXyzCol(D3DPT_POINTLIST,s1x,s1y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_POINTLIST,s2x,s2y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);


	if(au==YSTRUE)
	{
		col=YsYellow();
	}
	else
	{
		col=hudCol;
	}

	s1y=cenY+(long)((double)sizY*ail/2.0F);
	s2y=s1y;
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s1x,s1y,0.5,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s2x,s2y,0.5,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
}

void FsHeadUpDisplay::DrawRudder(double rud)
{
	long cenX,cenY,sizX;

	cenX=lupX+wid*2/12;
	cenY=lupY+hei*10/12;

	sizX=wid/16;

	cenX+=sizX/2;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX,cenY+2,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX,cenY+8,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	sizX=(long)((double)sizX*rud);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX-sizX,cenY+2   ,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX-sizX,cenY+8+14,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
}

void FsHeadUpDisplay::DrawSpeed(const double &spd)
{
	long cenX,cenY;
	long i,deg;
	double y;

	cenX=lupX+wid/5;
	cenY=lupY+hei/2;

	int s1x,s1y,s2x,s2y;
	s1x=cenX;
	s1y=cenY;
	s2x=cenX+4;
	s2y=cenY;

	double interval;
	interval=(double)wid/64;

	FsDrawLine(s1x,s1y,s2x,s2y,hudCol);

	s1x=cenX-5;
	s2x=cenX-1;
	for(i=-10; i<=10; i++)
	{
		deg=(long)spd+i;

		if(deg>=0)
		{
			y=(spd-(double)deg)*interval;
			s1y=(long)(cenY+y);
			s2y=(long)(cenY+y);

			if(deg%10==0)
			{
				s1x=cenX-5;
				FsDrawLine(s1x,s1y,s2x,s2y,hudCol);

				char str[16];
				sprintf(str,"%-4d",deg);
				FsDrawString(s1x-40,s1y,str,hudCol);
			}
			else
			{
				s1x=cenX-2;
				FsDrawLine(s1x,s1y,s2x,s2y,hudCol);
			}
		}
	}
}

void FsHeadUpDisplay::DrawAltitude(double alt)
{
	long cenX,cenY;
	long i,deg;
	double y;
	int s1x,s1y,s2x,s2y;

	cenX=lupX+wid*4/5;
	cenY=lupY+hei/2;

	s1x=cenX;
	s1y=cenY;
	s2x=cenX-4;
	s2y=cenY;

	FsDrawLine(s1x,s1y,s2x,s2y,hudCol);


	double interval;
	interval=(double)wid/64;


	alt/=10.0F;

	s1x=cenX+5;
	s2x=cenX+1;
	for(i=-10; i<=10; i++)
	{
		deg=(long)alt+i;

		if(deg>=0)
		{
			y=(alt-(double)deg)*interval;
			s1y=(long)(cenY+y);
			s2y=(long)(cenY+y);

			if(deg%10==0)
			{
				s1x=cenX+5;
				FsDrawLine(s1x,s1y,s2x,s2y,hudCol);

				char str[16];
				sprintf(str,"%-5d",deg*10);
				FsDrawString(s1x+6,s1y,str,hudCol);
			}
			else
			{
				s1x=cenX+2;
				FsDrawLine(s1x,s1y,s2x,s2y,hudCol);
			}
		}
	}
}

void FsHeadUpDisplay::DrawClimbRatio(const double &climbRatio)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	int i,y,y0,d;
	char str[128];
	int sx,sy,s1x,s1y,s2x,s2y;

	sx=lupX+wid*11/12;
	sy=lupY+hei*2/12-FONTPITCH;

	FsDrawString(sx,sy,"CLIMB",hudCol);
	sy+=FONTPITCH;

	d=hei/4;
	y0=sy;
	i=0;
	for(y=15; y>=-15; y-=5)
	{
		sy=y0+(d*i)/6;

		s1x=sx+6;
		s1y=sy;

		s2x=sx+11;
		s2y=sy;

		if(y==-15 || y==15 || y==0)
		{
			s2x+=3;
		}

		ysD3dDev->AddXyzCol(D3DPT_LINELIST,s1x,s1y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,s2x,s2y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

		s2x+=4;

		if(y==-15 || y==15)
		{
			sprintf(str,"%+d",y);
			FsDrawString(s2x,s2y,str,hudCol);
		}
		else if(y==0)
		{
			FsDrawString(s2x,s2y,"0",hudCol);
		}

		i++;
	}


	int r;
	r=int(YsBound(climbRatio,-1500.0,1500.0));
	r=((1500-r)*d)/3000;;
	s1x=sx;
	s1y=y0+r;
	s2x=s1x+4;
	s2y=s1y;

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s1x,s1y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,s2x,s2y,0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
}

void FsHeadUpDisplay::DrawBank(const double &b)
{
	int i;
	long cenX,cenY;
	const double s[]={-0.866025,-0.707107,-0.5,0.0,0.5,0.707107,0.866025};
	const double c[]={0.5,0.707107,0.866025,1.0,0.866025,0.707107,0.5};
	double x,y,rad;

	cenX=lupX+wid/2;
	cenY=lupY+hei/6;
	rad=double(hei/3);

	auto ysD3dDev=YsD3dDevice::GetCurrent();

	for(i=0; i<=6; i++)
	{
		x=s[i];
		y=c[i];

		ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+int(x*(rad+9)),cenY-int(y*(rad+9)),0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+int(x*(rad+14)),cenY-int(y*(rad+14)),0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	}

	if(-60.0<YsRadToDeg(b) && YsRadToDeg(b)<60.0)
	{
		x=sin(b);
		y=cos(b);

		ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+int(x*(rad)),cenY-int(y*(rad)),0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,cenX+int(x*(rad+8)),cenY-int(y*(rad+8)),0.5,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	}

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
}

void FsHeadUpDisplay::DrawAttitude(const YsVec3 &pos,const YsAtt3 &att,const YsVec3 &viewPos,const YsAtt3 &viewAtt)
{
	YsVec3 trans,p1,p2;
	YsAtt3 attH;
	int i,j,s,w,pitch;

	attH.Set(0.0,att.p(),att.b());

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);

	pitch=(int)YsRadToDeg(att.p());
	for(i=-90; i<=90; i+=10)
	{
		if(pitch-30<=i && i<pitch+30)
		{
			double a,y,z;
			a=double(i);
			z=100.0*cos(YsDegToRad(a));
			y=100.0*sin(YsDegToRad(a));

			if(i>=0)
			{
				s=6;
				w=6;
			}
			else
			{
				s=2;
				w=1;
			}

			for(j=20; j<32; j+=s)
			{
				p1.Set(j,y,z);
				attH.MulInverse(p1,p1);
				viewAtt.Mul(p1,p1);
				p1+=viewPos;

				p2.Set(j+w,y,z);
				attH.MulInverse(p2,p2);
				viewAtt.Mul(p2,p2);
				p2+=viewPos;

				ysD3dDev->AddXyzCol(D3DPT_LINELIST,p1.x(),p1.y(),p1.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
				ysD3dDev->AddXyzCol(D3DPT_LINELIST,p2.x(),p2.y(),p2.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);


				p1.Set(-j,y,z);
				attH.MulInverse(p1,p1);
				viewAtt.Mul(p1,p1);
				p1+=viewPos;

				p2.Set(-j-w,y,z);
				attH.MulInverse(p2,p2);
				viewAtt.Mul(p2,p2);
				p2+=viewPos;

				ysD3dDev->AddXyzCol(D3DPT_LINELIST,p1.x(),p1.y(),p1.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
				ysD3dDev->AddXyzCol(D3DPT_LINELIST,p2.x(),p2.y(),p2.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
			}

			D3DVIEWPORT9 d3dViewport;
		    D3DMATRIX d3dProj,d3dModel,d3dView; // ,d3dAllTfm;
		    ysD3dDev->d3dDev->GetViewport(&d3dViewport);
			ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&d3dModel);
			ysD3dDev->d3dDev->GetTransform(D3DTS_PROJECTION,&d3dProj);
			ysD3dDev->d3dDev->GetTransform(D3DTS_VIEW,&d3dView);
			D3DVECTOR v3d,vSc;
			v3d.x=(float)p2.x();
			v3d.y=(float)p2.y();
			v3d.z=(float)p2.z();
			YsD3dComputeProjection(vSc,v3d,d3dViewport,d3dProj,d3dView,d3dModel);

			char str[256];
			sprintf(str,"%3d",i);

			/* RECT rc;
			rc.left=(LONG)vSc.x;
			rc.right=(LONG)vSc.x+32;
			rc.top=(LONG)vSc.y;
			rc.bottom=(LONG)vSc.y+14;
			ysD3dDev->d3dFont->DrawText(NULL,str,-1,&rc,DT_LEFT|DT_TOP,D3DCOLOR_ARGB(255,hudCol.Ri(),hudCol.Gi(),hudCol.Bi())); */

			FsDrawString((int)vSc.x,(int)vSc.y+14,str,hudCol);
		}
	}
	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x()-30.0,pos.y()+100.0,pos.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x()+30.0,pos.y()+100.0,pos.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x()-30.0,pos.y()-100.0,pos.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x()+30.0,pos.y()-100.0,pos.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x(),pos.y()+100.0,pos.z()-30.0,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x(),pos.y()+100.0,pos.z()+30.0,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x(),pos.y()-100.0,pos.z()-30.0,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,pos.x(),pos.y()-100.0,pos.z()+30.0,hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
}

void FsHeadUpDisplay::DrawVelocityVectorIndicator(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsVec3 &v)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);


	YsVec3 vel,target,ev,uv,rv,p1,p2;
	ev=viewAtt.GetForwardVector();
	uv=viewAtt.GetUpVector();
	rv=ev^uv;

	vel=v;

	viewAtt.MulInverse(vel,vel);
	if(vel.z()>YsTolerance)
	{
		vel.DivX(vel.z());
		vel.MulX(5.0);
		vel.DivY(vel.z());
		vel.MulY(5.0);
		vel.SetZ(5.0);
		viewAtt.Mul(vel,vel);
		target=viewPos+vel;

		int i;
		const double rad=0.1;

		for(i=0; i<=360; i+=30)
		{
			p1=(target+(rv*trigonomyTable[i*2]+uv*trigonomyTable[i*2+1])*rad);
			ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,p1.x(),p1.y(),p1.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		}
		ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);

		p1=target+rv*rad;
		p2=target+rv*rad*1.8;
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,p1.x(),p1.y(),p1.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,p2.x(),p2.y(),p2.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

		p1=target-rv*rad;
		p2=target-rv*rad*1.8;
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,p1.x(),p1.y(),p1.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,p2.x(),p2.y(),p2.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

		p1=target+uv*rad;
		p2=target+uv*rad*1.8;
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,p1.x(),p1.y(),p1.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,p2.x(),p2.y(),p2.z(),hudCol.Ri(),hudCol.Gi(),hudCol.Bi(),255);

		ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
}

void FsHeadUpDisplay::DrawCircleContainer
	   (const YsMatrix4x4 &viewpoint,
	    const YsAtt3 &viewAtt,
	    const YsVec3 &target,
	    const YsVec3 &from,
	    const YsColor &col,
	    const char caption[],
	    const char caption2[],
	    YSBOOL dot,
	    int begin,int step)
{
	int i;
	YsVec3 p1,p2;
	YsVec3 rel,tgt;

	rel=viewpoint*target;
	if(rel.z()<0.0)
	{
		return;
	}

	if(rel.z()>5000.0)
	{
		rel.MulX(5000.0/rel.z());
		rel.MulY(5000.0/rel.z());
		rel.SetZ(5000.0);
		viewpoint.MulInverse(tgt,rel,1.0);
	}
	else
	{
		tgt=target;
	}


	auto ysD3dDev=YsD3dDevice::GetCurrent();

    D3DMATRIX prevD3dModel;
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&prevD3dModel);
	ysD3dDev->MultiplyModelMatrix(tgt.x(),tgt.y(),tgt.z(),viewAtt.h(),viewAtt.p(),viewAtt.b());

//	glPushMatrix();
//	glTranslated(tgt.x(),tgt.y(),tgt.z());
//	glRotated(-YsRadToDeg(viewAtt.h()),0.0,1.0,0.0);
//	glRotated(-YsRadToDeg(viewAtt.p()),1.0,0.0,0.0);
//	glRotated( YsRadToDeg(viewAtt.b()),0.0,0.0,1.0);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);

	for(i=begin; i<=begin+360; i+=step)
	{
		ysD3dDev->AddXyzCol
		   (D3DPT_LINESTRIP,rel.z()/18.0*trigonomyTable[i*2],rel.z()/18.0*trigonomyTable[i*2+1],0.0,
		    col.Ri(),col.Gi(),col.Bi(),255);
	}
	ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);


//	glRasterPos3d(rel.z()/18.0,-rel.z()/18.0,0.0);
//	FsGlSetListBase(FS_GL_FONT_BITMAP_BASE);
//	glCallLists(strlen(caption),GL_UNSIGNED_BYTE,caption);

	D3DVIEWPORT9 d3dViewport;
    D3DMATRIX d3dProj,d3dModel,d3dView; // ,d3dAllTfm;
    ysD3dDev->d3dDev->GetViewport(&d3dViewport);
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&d3dModel);
	ysD3dDev->d3dDev->GetTransform(D3DTS_PROJECTION,&d3dProj);
	ysD3dDev->d3dDev->GetTransform(D3DTS_VIEW,&d3dView);
	D3DVECTOR v3d,vSc;
	v3d.x=rel.zf()/18.0f;
	v3d.y=-rel.zf()/18.0f;
	v3d.z=0.0f;
	YsD3dComputeProjection(vSc,v3d,d3dViewport,d3dProj,d3dView,d3dModel);


	/* RECT rc;
	rc.left=vSc.x;
	rc.right=vSc.x+128;
	rc.top=vSc.y;
	rc.bottom=vSc.y+16;
	ysD3dDev->d3dFont->DrawText(NULL,caption,-1,&rc,DT_LEFT|DT_TOP,D3DCOLOR_ARGB(255,hudCol.Ri(),hudCol.Gi(),hudCol.Bi())); */
	FsDrawString((int)vSc.x,(int)vSc.y+16,caption,hudCol);

	if(caption2!=NULL)
	{
		/* rc.left=vSc.x;
		rc.right=vSc.x+128;
		rc.top=vSc.y-48;
		rc.bottom=vSc.y-32;
		ysD3dDev->d3dFont->DrawText(NULL,caption2,-1,&rc,DT_LEFT|DT_TOP,D3DCOLOR_ARGB(255,hudCol.Ri(),hudCol.Gi(),hudCol.Bi())); */
		FsDrawString((int)vSc.x,(int)vSc.y-32,caption2,hudCol);
	}


	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&prevD3dModel);



	if(dot==YSTRUE)
	{
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,tgt.x(),tgt.y(),tgt.z(),col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);
	}

	if(tgt!=from)
	{
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,tgt.x(),tgt.y(),tgt.z(),col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->AddXyzCol(D3DPT_LINELIST,from.x(),from.y(),from.z(),col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_LINELIST);
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
}


void FsHeadUpDisplay::DrawCrossDesignator
	   (const YsMatrix4x4 &viewpoint,
	    const YsAtt3 &viewAtt,
	    const YsVec3 &target,
	    const YsVec3 &from,
	    const YsColor &col,
	    YSBOOL dot)
{
	YsVec3 p1,p2;
	YsVec3 rel,tgt;

	rel=viewpoint*target;
	if(rel.z()>5000.0)
	{
		rel.MulX(5000.0/rel.z());
		rel.MulY(5000.0/rel.z());
		rel.SetZ(5000.0);
		viewpoint.MulInverse(tgt,rel,1.0);
	}
	else
	{
		tgt=target;
	}

//	glPushMatrix();
//	glTranslated(tgt.x(),tgt.y(),tgt.z());
//	glRotated(-YsRadToDeg(viewAtt.h()),0.0,1.0,0.0);
//	glRotated(-YsRadToDeg(viewAtt.p()),1.0,0.0,0.0);
//	glRotated( YsRadToDeg(viewAtt.b()),0.0,0.0,1.0);
    D3DMATRIX prevD3dModel;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&prevD3dModel);
	ysD3dDev->MultiplyModelMatrix(tgt.x(),tgt.y(),tgt.z(),viewAtt.h(),viewAtt.p(),viewAtt.b());

	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);



	ysD3dDev->AddXyzCol(D3DPT_LINELIST,rel.z()/32.0,0.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,rel.z()/96.0,0.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,-rel.z()/32.0,0.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,-rel.z()/96.0,0.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,0.0,rel.z()/32.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,0.0,rel.z()/96.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,0.0,-rel.z()/32.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,0.0,-rel.z()/96.0,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&prevD3dModel);


	if(dot==YSTRUE)
	{
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,tgt.x(),tgt.y(),tgt.z(),col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
}


void FsHeadUpDisplay::DrawCrossDesignator2
	   (const YsMatrix4x4 &viewpoint,
	    const YsAtt3 &viewAtt,
	    const YsVec3 &target,
	    const YsVec3 &from,
	    const YsColor &col,
	    YSBOOL dot)
{
	YsVec3 p1,p2;
	YsVec3 rel,tgt;

	rel=viewpoint*target;
	if(rel.z()>5000.0)
	{
		rel.MulX(5000.0/rel.z());
		rel.MulY(5000.0/rel.z());
		rel.SetZ(5000.0);
		viewpoint.MulInverse(tgt,rel,1.0);
	}
	else
	{
		tgt=target;
	}

//	glPushMatrix();
//	glTranslated(tgt.x(),tgt.y(),tgt.z());
//	glRotated(-YsRadToDeg(viewAtt.h()),0.0,1.0,0.0);
//	glRotated(-YsRadToDeg(viewAtt.p()),1.0,0.0,0.0);
//	glRotated( YsRadToDeg(viewAtt.b()),0.0,0.0,1.0);

    D3DMATRIX prevD3dModel;
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	ysD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&prevD3dModel);
	ysD3dDev->MultiplyModelMatrix(tgt.x(),tgt.y(),tgt.z(),viewAtt.h(),viewAtt.p(),viewAtt.b());

	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);



	double l1,l2;
	l1=rel.z()/32.0;
	l2=rel.z()/96.0;

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,l1,l1,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,l2,l2,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,-l1,l1,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,-l2,l2,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,l1,-l1,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,l2,-l2,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->AddXyzCol(D3DPT_LINELIST,-l1,-l1,0.0,col.Ri(),col.Gi(),col.Bi(),255);
	ysD3dDev->AddXyzCol(D3DPT_LINELIST,-l2,-l2,0.0,col.Ri(),col.Gi(),col.Bi(),255);

	ysD3dDev->FlushXyzCol(D3DPT_LINELIST);

	ysD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&prevD3dModel);



	if(dot==YSTRUE)
	{
		ysD3dDev->AddXyzCol(D3DPT_POINTLIST,tgt.x(),tgt.y(),tgt.z(),col.Ri(),col.Gi(),col.Bi(),255);
		ysD3dDev->FlushXyzCol(D3DPT_POINTLIST);
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
}

void FsHeadUpDisplay::DrawNeedle(int cx,int cy,int wid,int lng,const double &ang,int tailLng)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);

	if(fsD3dDev->instNeedleBuffer==NULL)
	{
		fsD3dDev->instNeedleBuffer=new YsXyzColBuffer;
		fsD3dDev->instNeedleBuffer->PrepareBuffer(*fsD3dDev,8);
	}

	double needle[10];
	double c,s,w,t;

	c=cos(ang);
	s=sin(ang);
	w=(double)wid;
	t=(double)tailLng;

	needle[0]=(double)cx-(c*(-w)-s*(-t));
	needle[1]=(double)cy-(s*(-w)+c*(-t));

	needle[2]=(double)cx-(c*( w)-s*(-t));
	needle[3]=(double)cy-(s*( w)+c*(-t));

	needle[4]=(double)cx-(c*( w)-s*(lng-wid*2));
	needle[5]=(double)cy-(s*( w)+c*(lng-wid*2));

	needle[6]=(double)cx-(c*( 0.0)-s*(lng));
	needle[7]=(double)cy-(s*( 0.0)+c*(lng));

	needle[8]=(double)cx-(c*(-w)-s*(lng-wid*2));
	needle[9]=(double)cy-(s*(-w)+c*(lng-wid*2));

	fsD3dDev->instNeedleBuffer->Add(D3DPT_TRIANGLEFAN,needle[0],needle[1],0.5,80,80,80,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_TRIANGLEFAN,needle[2],needle[3],0.5,80,80,80,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_TRIANGLEFAN,needle[4],needle[5],0.5,80,80,80,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_TRIANGLEFAN,needle[6],needle[7],0.5,80,80,80,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_TRIANGLEFAN,needle[8],needle[9],0.5,80,80,80,255);
	fsD3dDev->instNeedleBuffer->Flush(D3DPT_TRIANGLEFAN);

	fsD3dDev->instNeedleBuffer->Add(D3DPT_LINESTRIP,needle[0],needle[1],0.5,255,255,255,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_LINESTRIP,needle[2],needle[3],0.5,255,255,255,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_LINESTRIP,needle[4],needle[5],0.5,255,255,255,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_LINESTRIP,needle[6],needle[7],0.5,255,255,255,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_LINESTRIP,needle[8],needle[9],0.5,255,255,255,255);
	fsD3dDev->instNeedleBuffer->Add(D3DPT_LINESTRIP,needle[0],needle[1],0.5,255,255,255,255);
	fsD3dDev->instNeedleBuffer->Flush(D3DPT_LINESTRIP);
}

