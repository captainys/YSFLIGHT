#include <ysclass.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif


void FsHeadUpDisplay::DrawCrossHair(void)
{
	long cenX,cenY,lng,lat;

	cenX=lupX+wid/2;
	cenY=lupY+hei/2;

	lat=wid/20;
	lng=lat*2/3;

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

	glBegin(GL_LINES);

	glVertex2i(cenX-lat,cenY);
	glVertex2i(cenX-4  ,cenY);

	glVertex2i(cenX+lat,cenY);
	glVertex2i(cenX+4  ,cenY);

	glVertex2i(cenX,cenY-lng);
	glVertex2i(cenX,cenY-4);

	glVertex2i(cenX,cenY+lng);
	glVertex2i(cenX,cenY+4);
	glEnd();
}

void FsHeadUpDisplay::DrawHeading(const YsAtt3 &a,YSBOOL showHdgBug,const double &hdgBug,YSBOOL selected)
{
	long cenX,cenY;
	long deg,num;
	int s1x,s1y,s2x,s2y;
	double hdg,interval;

	cenX=lupX+wid/2;
	cenY=lupY+hei/6;

	interval=(double)wid/64;

	hdg=-YsRadToDeg(a.h());
	deg= (long)(hdg-12.0F);

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

	glBegin(GL_LINES);
	glVertex2i(cenX,cenY+1);
	glVertex2i(cenX,cenY+8);

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
			str[0]='0'+num/10;
			str[1]='0'+num%10;
			str[2]=0;

			glEnd();
			FsDrawString(s2x-12,s2y-12,str,hudCol);
			glBegin(GL_LINES);
		}
		glVertex2i(s1x,s1y);
		glVertex2i(s2x,s2y);

		deg++;
	}

	glEnd();


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
			glBegin(GL_LINE_LOOP);
			glVertex2i(cx-12,cy-10);
			glVertex2i(cx-12,cy);
			glVertex2i(cx-4,cy);
			glVertex2i(cx,cy+4);
			glVertex2i(cx+4,cy);
			glVertex2i(cx+12,cy);
			glVertex2i(cx+12,cy-10);
			glEnd();
		}
		else
		{
			glBegin(GL_TRIANGLE_FAN);
			glVertex2i(cx,cy-10);
			glVertex2i(cx-12,cy-10);
			glVertex2i(cx-12,cy);
			glVertex2i(cx-4,cy);
			glVertex2i(cx,cy+4);
			glVertex2i(cx+4,cy);
			glVertex2i(cx+12,cy);
			glVertex2i(cx+12,cy-10);
			glEnd();
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
	int i;
	YSBOOL abr;
	long cenX,cenY,sizX,sizY,barY;
	double deg;


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
			glColor3d(1.0,0.0,0.0);
		}
		else
		{
			glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());
		}

		glBegin(GL_QUADS);
		glVertex2i(cenX+1     ,cenY);
		glVertex2i(cenX+sizX-1,cenY);
		glVertex2i(cenX+sizX-1,cenY-barY);
		glVertex2i(cenX+1     ,cenY-barY);
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex2i(cenX+1     ,cenY);
		glVertex2i(cenX+sizX-1,cenY);
		glVertex2i(cenX+sizX-1,cenY-sizY);
		glVertex2i(cenX+1     ,cenY-sizY);
		glEnd();

		glBegin(GL_POINTS);
		glVertex2i(cenX+1     ,cenY);
		glVertex2i(cenX+sizX-1,cenY);
		glVertex2i(cenX+sizX-1,cenY-sizY);
		glVertex2i(cenX+1     ,cenY-sizY);
		glEnd();

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

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

	glBegin(GL_LINES);

	glVertex2i(cenX     ,cenY);
	glVertex2i(cenX+sizX,cenY);

	dirX=int(vec.z()*double(sizX));
	dirY=int(vec.y()*double(sizX));
	glVertex2i(cenX+sizX     ,cenY);
	glVertex2i(cenX+sizX+dirX,cenY+dirY);
	glEnd();
}

void FsHeadUpDisplay::DrawElevator(double elv,double trim,YSBOOL au)
{
	long cenX,cenY,sizX,sizY;

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

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

	glBegin(GL_POINTS);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);
	glEnd();

	glBegin(GL_LINES);

	if(au==YSTRUE)
	{
		glColor3d(1.0,1.0,0.0);
	}
	else
	{
		glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());
	}

	s1y=cenY-(long)((double)sizY*elv/2.0);
	s2y=s1y;
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

	s1y=cenY-(long)(trim*double(sizY)/2.0);
	s2y=s1y;
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);

	s1x=cenX+sizX*3/2;
	s1y=cenY-(long)(trim*double(sizY)/2.0);
	s2x=s1x;
	s2y=cenY;
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);

	glEnd();
}

void FsHeadUpDisplay::DrawAileron(double ail,YSBOOL au)
{
	long cenX,cenY,sizX,sizY;
	int s1x,s1y,s2x,s2y;

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

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());
	glBegin(GL_POINTS);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);
	glEnd();

	if(au==YSTRUE)
	{
		glColor3d(1.0,1.0,0.0);
	}
	else
	{
		glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());
	}

	s1y=cenY-(long)((double)sizY*ail/2.0F);
	s2y=s1y;
	glBegin(GL_LINES);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);
	glEnd();

	s1x=cenX+sizX*2;
	s1y=cenY;
	s2x=cenX+sizX*3;
	s2y=cenY;

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());
	glBegin(GL_POINTS);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);
	glEnd();

	if(au==YSTRUE)
	{
		glColor3d(1.0,1.0,0.0);
	}
	else
	{
		glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());
	}

	s1y=cenY+(long)((double)sizY*ail/2.0F);
	s2y=s1y;
	glBegin(GL_LINES);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);
	glEnd();
}

void FsHeadUpDisplay::DrawRudder(double rud)
{
	long cenX,cenY,sizX;

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

	cenX=lupX+wid*2/12;
	cenY=lupY+hei*10/12;

	sizX=wid/16;

	cenX+=sizX/2;

	glBegin(GL_LINES);

	glVertex2i(cenX,cenY+2);
	glVertex2i(cenX,cenY+8);

	sizX=(long)((double)sizX*rud);
	glVertex2i(cenX-sizX,cenY+2);
	glVertex2i(cenX-sizX,cenY+8+14);

	glEnd();
}

void FsHeadUpDisplay::DrawSpeed(const double &spd)
{
	long cenX,cenY;
	long i,deg;
	double y;

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

	cenX=lupX+wid/5;
	cenY=lupY+hei/2;

	int s1x,s1y,s2x,s2y;
	s1x=cenX;
	s1y=cenY;
	s2x=cenX+4;
	s2y=cenY;

	double interval;
	interval=(double)wid/64;

	glBegin(GL_LINES);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);

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
				glVertex2i(s1x,s1y);
				glVertex2i(s2x,s2y);
				glEnd();

				char str[16];
				sprintf(str,"%-4d",deg);
				FsDrawString(s1x-40,s1y,str,hudCol);

				glBegin(GL_LINES);
			}
			else
			{
				s1x=cenX-2;
				glVertex2i(s1x,s1y);
				glVertex2i(s2x,s2y);
			}
		}
	}
	glEnd();
}

void FsHeadUpDisplay::DrawAltitude(double alt)
{
	long cenX,cenY;
	long i,deg;
	double y;
	int s1x,s1y,s2x,s2y;

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

	cenX=lupX+wid*4/5;
	cenY=lupY+hei/2;

	s1x=cenX;
	s1y=cenY;
	s2x=cenX-4;
	s2y=cenY;
	glBegin(GL_LINES);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);


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
				glVertex2i(s1x,s1y);
				glVertex2i(s2x,s2y);
				glEnd();

				char str[16];
				sprintf(str,"%-5d",deg*10);
				FsDrawString(s1x+6,s1y,str,hudCol);

				glBegin(GL_LINES);
			}
			else
			{
				s1x=cenX+2;
				glVertex2i(s1x,s1y);
				glVertex2i(s2x,s2y);
			}
		}
	}
	glEnd();
}

void FsHeadUpDisplay::DrawClimbRatio(const double &climbRatio)
{
	int i,y,y0,d;
	char str[128];
	int sx,sy,s1x,s1y,s2x,s2y;

	sx=lupX+wid*11/12;
	sy=lupY+hei*2/12-FONTPITCH;

	FsDrawString(sx,sy,"CLIMB",hudCol);
	sy+=FONTPITCH;

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

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

		glBegin(GL_LINES);
		glVertex2i(s1x,s1y);
		glVertex2i(s2x,s2y);
		glEnd();

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
	r=((1500-r)*d)/3000;

	s1x=sx;
	s1y=y0+r;
	s2x=s1x+4;
	s2y=s1y;

	glBegin(GL_LINES);
	glVertex2i(s1x,s1y);
	glVertex2i(s2x,s2y);
	glEnd();
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

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());
	glBegin(GL_LINES);

	for(i=0; i<=6; i++)
	{
		x=s[i];
		y=c[i];

		glVertex2i(cenX+int(x*(rad+9)),cenY-int(y*(rad+9)));
		glVertex2i(cenX+int(x*(rad+14)),cenY-int(y*(rad+14)));
	}

	if(-60.0<YsRadToDeg(b) && YsRadToDeg(b)<60.0)
	{
		x=sin(b);
		y=cos(b);

		glVertex2i(cenX+int(x*(rad)),cenY-int(y*(rad)));
		glVertex2i(cenX+int(x*(rad+8)),cenY-int(y*(rad+8)));
	}

	glEnd();
}

void FsHeadUpDisplay::DrawAttitude(const YsVec3 &pos,const YsAtt3 &att,const YsVec3 &viewPos,const YsAtt3 &viewAtt)
{
	YsVec3 trans,p1,p2;
	YsAtt3 attH;
	int i,j,s,w,pitch;

	attH.Set(0.0,att.p(),att.b());

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

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

			glBegin(GL_LINES);
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

				glVertex3dv(p1);
				glVertex3dv(p2);


				p1.Set(-j,y,z);
				attH.MulInverse(p1,p1);
				viewAtt.Mul(p1,p1);
				p1+=viewPos;

				p2.Set(-j-w,y,z);
				attH.MulInverse(p2,p2);
				viewAtt.Mul(p2,p2);
				p2+=viewPos;

				glVertex3dv(p1);
				glVertex3dv(p2);
			}
			glEnd();

			char str[256];
			sprintf(str,"%3d",i);

			glRasterPos3dv(p2);
			glListBase(FS_GL_FONT_BITMAP_BASE);
			glCallLists(strlen(str),GL_UNSIGNED_BYTE,str);
			glListBase(0);
		}
	}

	glBegin(GL_LINES);

	glVertex3d(pos.x()-30.0,pos.y()+100.0,pos.z());
	glVertex3d(pos.x()+30.0,pos.y()+100.0,pos.z());

	glVertex3d(pos.x()-30.0,pos.y()-100.0,pos.z());
	glVertex3d(pos.x()+30.0,pos.y()-100.0,pos.z());

	glVertex3d(pos.x(),pos.y()+100.0,pos.z()-30.0);
	glVertex3d(pos.x(),pos.y()+100.0,pos.z()+30.0);

	glVertex3d(pos.x(),pos.y()-100.0,pos.z()-30.0);
	glVertex3d(pos.x(),pos.y()-100.0,pos.z()+30.0);

	glEnd();

	glEnable(GL_DEPTH_TEST);
}

void FsHeadUpDisplay::DrawVelocityVectorIndicator(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsVec3 &v)
{
	YsVec3 vel,target,ev,uv,rv,p1,p2;
	ev=viewAtt.GetForwardVector();
	uv=viewAtt.GetUpVector();
	rv=ev^uv;

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glColor3d(hudCol.Rd(),hudCol.Gd(),hudCol.Bd());

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
		glBegin(GL_LINE_LOOP);
		for(i=0; i<360; i+=30)
		{
			p1=(target+(rv*trigonomyTable[i*2]+uv*trigonomyTable[i*2+1])*rad);
			glVertex3dv(p1);
		}
		glEnd();

		glBegin(GL_LINES);
		p1=target+rv*rad;
		p2=target+rv*rad*1.8;
		glVertex3dv(p1);
		glVertex3dv(p2);

		p1=target-rv*rad;
		p2=target-rv*rad*1.8;
		glVertex3dv(p1);
		glVertex3dv(p2);

		p1=target+uv*rad;
		p2=target+uv*rad*1.8;
		glVertex3dv(p1);
		glVertex3dv(p2);

		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
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

	glPushMatrix();
	glTranslated(tgt.x(),tgt.y(),tgt.z());
	glRotated(-YsRadToDeg(viewAtt.h()),0.0,1.0,0.0);
	glRotated(-YsRadToDeg(viewAtt.p()),1.0,0.0,0.0);
	glRotated( YsRadToDeg(viewAtt.b()),0.0,0.0,1.0);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glColor3d(col.Rd(),col.Gd(),col.Bd());

	glBegin(GL_LINE_LOOP);
	for(i=begin; i<=begin+360; i+=step)
	{
		glVertex3d(rel.z()/18.0*trigonomyTable[i*2],rel.z()/18.0*trigonomyTable[i*2+1],0.0);
	}
	glEnd();

	glRasterPos3d(rel.z()/18.0,-rel.z()/18.0,0.0);
	glListBase(FS_GL_FONT_BITMAP_BASE);
	glCallLists(strlen(caption),GL_UNSIGNED_BYTE,caption);
	glListBase(0);

	if(caption2!=NULL)
	{
		glRasterPos3d(rel.z()/18.0,rel.z()/18.0,0.0);
		glListBase(FS_GL_FONT_BITMAP_BASE);
		glCallLists(strlen(caption2),GL_UNSIGNED_BYTE,caption2);
		glListBase(0);
	}

	glPopMatrix();

	if(dot==YSTRUE)
	{
		glBegin(GL_POINTS);
		glVertex3dv(tgt);
		glEnd();
	}

	if(tgt!=from)
	{
		glBegin(GL_LINES);
		glVertex3dv(tgt);
		glVertex3dv(from);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
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

	glPushMatrix();
	glTranslated(tgt.x(),tgt.y(),tgt.z());
	glRotated(-YsRadToDeg(viewAtt.h()),0.0,1.0,0.0);
	glRotated(-YsRadToDeg(viewAtt.p()),1.0,0.0,0.0);
	glRotated( YsRadToDeg(viewAtt.b()),0.0,0.0,1.0);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glColor3d(col.Rd(),col.Gd(),col.Bd());

	glBegin(GL_LINES);
	glVertex2d(rel.z()/32.0,0.0);
	glVertex2d(rel.z()/96.0,0.0);

	glVertex2d(-rel.z()/32.0,0.0);
	glVertex2d(-rel.z()/96.0,0.0);

	glVertex2d(0.0,rel.z()/32.0);
	glVertex2d(0.0,rel.z()/96.0);

	glVertex2d(0.0,-rel.z()/32.0);
	glVertex2d(0.0,-rel.z()/96.0);

	glEnd();

	glPopMatrix();

	if(dot==YSTRUE)
	{
		glBegin(GL_POINTS);
		glVertex3dv(tgt);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
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

	glPushMatrix();
	glTranslated(tgt.x(),tgt.y(),tgt.z());
	glRotated(-YsRadToDeg(viewAtt.h()),0.0,1.0,0.0);
	glRotated(-YsRadToDeg(viewAtt.p()),1.0,0.0,0.0);
	glRotated( YsRadToDeg(viewAtt.b()),0.0,0.0,1.0);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glColor3d(col.Rd(),col.Gd(),col.Bd());

	double l1,l2;
	l1=rel.z()/32.0;
	l2=rel.z()/96.0;

	glBegin(GL_LINES);
	glVertex2d(l1,l1);
	glVertex2d(l2,l2);

	glVertex2d(-l1,l1);
	glVertex2d(-l2,l2);

	glVertex2d(l1,-l1);
	glVertex2d(l2,-l2);

	glVertex2d(-l1,-l1);
	glVertex2d(-l2,-l2);

	glEnd();

	glPopMatrix();

	if(dot==YSTRUE)
	{
		glBegin(GL_POINTS);
		glVertex3dv(tgt);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
}

////////////////////////////////////////////////////////////

void FsHeadUpDisplay::DrawNeedle(int cx,int cy,int wid,int lng,const double &ang,int tailLng)
{
	int needle[10];
	double c,s,w,t;

	c=cos(ang);
	s=sin(ang);
	w=(double)wid;
	t=(double)tailLng;

	needle[0]=cx-(int)(c*(-w)-s*(-t));
	needle[1]=cy-(int)(s*(-w)+c*(-t));

	needle[2]=cx-(int)(c*( w)-s*(-t));
	needle[3]=cy-(int)(s*( w)+c*(-t));

	needle[4]=cx-(int)(c*( w)-s*(lng-wid*2));
	needle[5]=cy-(int)(s*( w)+c*(lng-wid*2));

	needle[6]=cx-(int)(c*( 0.0)-s*(lng));
	needle[7]=cy-(int)(s*( 0.0)+c*(lng));

	needle[8]=cx-(int)(c*(-w)-s*(lng-wid*2));
	needle[9]=cy-(int)(s*(-w)+c*(lng-wid*2));

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glColor3ub(80,80,80);
	glBegin(GL_POLYGON);
	glVertex2i(needle[0],needle[1]);
	glVertex2i(needle[2],needle[3]);
	glVertex2i(needle[4],needle[5]);
	glVertex2i(needle[6],needle[7]);
	glVertex2i(needle[8],needle[9]);
	glEnd();

	glColor3ub(255,255,255);
	glBegin(GL_LINE_LOOP);
	glVertex2i(needle[0],needle[1]);
	glVertex2i(needle[2],needle[3]);
	glVertex2i(needle[4],needle[5]);
	glVertex2i(needle[6],needle[7]);
	glVertex2i(needle[8],needle[9]);
	glEnd();

	glEnable(GL_DEPTH_TEST);
}
