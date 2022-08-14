#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>
#include <ysclass.h>

#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#include "fsnavaid.h"

YSBOOL FsNavigationAid::IsInRange(const YsVec3 &pos) const
{
	return YSFALSE;
}

YSRESULT FsNavigationAid::DrawObjectlay(const YsVec3 &pos,const YsAtt3 &att,const YsColor &col) const
{
	return YSERR;
}

YSRESULT FsNavigationAid::DrawOverlay(const YsVec3 &pos,const YsAtt3 &att,const YsColor &col) const
{
	return YSERR;
}

double FsNavigationAid::GetDeviationForScoringPurpose(const YsVec3 &pos,const YsAtt3 &att) const
{
	return 0.0;
}

////////////////////////////////////////////////////////////


YSBOOL FsILS::IsInRange(const YsVec3 &pos) const
{
	double latDev,lngDev;
	if(GetDeviation(latDev,lngDev,pos)==YSOK &&
	   YsAbs(latDev)<20.0 &&
	   YsAbs(lngDev)<60.0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsILS::DrawObjectlay(const YsVec3 &p,const YsAtt3 &a,const YsColor &col) const
{
	return YSOK;
}

YSRESULT FsILS::DrawOverlay(const YsVec3 &p,const YsAtt3 &a,const YsColor &col) const
{
	double latDev,lngDev;
	double angDev;
	YsVec3 ev;

	ev=-att.GetForwardVector();
	angDev=atan2(ev.x(),ev.z());
	if(angDev<0.0)
	{
		angDev+=YsPi*2.0;
	}

	if(GetDeviation(latDev,lngDev,p)==YSOK)
	{
		DrawOverlay(angDev,latDev,lngDev,col);
	}
	return YSOK;
}

void FsILS::DrawOverlay(const double &course,const double &latDev,const double &lngDev,const YsColor &col)
{
	int cenx,ceny,sizx,sizy;
	int devx,devy;

	FsGetWindowSize(sizx,sizy);

	cenx=sizx/2;
	ceny=sizy*2/3;

	sizx=sizx*1/6;
	sizy=sizy*1/6;

	devx=int(latDev*double(sizx)/(YsPi*5.0/180.0));
	devy=int(lngDev*double(sizy)/(YsPi*3.0/180.0));

	devx=YsBound(devx,-sizx, sizx);
	devy=YsBound(devy,-sizy, sizy);

	int s1x,s1y,s2x,s2y;
	char loc[256];
	s1x=cenx+devx;
	s1y=ceny-sizy;
	s2x=cenx+devx;
	s2y=ceny+sizy;

	FsDrawLine(s1x,s1y,s2x,s2y,col);
	sprintf(loc,"LOC[%3.0lfDEG]",YsRadToDeg(course));
	FsDrawString(s1x,s1y,loc,col);

	s1x=cenx+sizx;
	s1y=ceny-devy;
	s2x=cenx-sizx;
	s2y=ceny-devy;
	FsDrawLine(s1x,s1y,s2x,s2y,col);
	FsDrawString(s1x,s1y,"GS",col);
}

double FsILS::GetDeviationForScoringPurpose(const YsVec3 &pos,const YsAtt3 &att) const
{
	// Not implemented yet
	return 0.0;
}

YSRESULT FsILS::Load(FILE *fp,const YsVec3 &fieldPos,const YsAtt3 &fieldAtt)
{
	// Not implemented yet
	return YSERR;
}

YSRESULT FsILS::Set(const YsVec3 &p,const YsAtt3 &a,const double &r)
{
	pos=p;
	att=a;
	range=r;
	return YSOK;
}

YSRESULT FsILS::GetDeviation(double &latDev,double &lngDev,const YsVec3 &p) const
{
	YsVec3 rel;
	rel=p-pos;
	att.MulInverse(rel,rel);

	if(0.0<rel.z() && rel.z()<range)
	{
		latDev=atan2( rel.x(),rel.z());
		lngDev=atan2(-rel.y(),rel.z());
		return YSOK;
	}
	return YSERR;
}

double FsILS::GetLocalizerHeading(void) const
{
	double angDev;
	YsVec3 ev;

	ev=-att.GetForwardVector();
	angDev=atan2(-ev.x(),ev.z());
	if(angDev<0.0)
	{
		angDev+=YsPi*2.0;
	}
	return angDev;
}

const double &FsILS::GetRange(void) const
{
	return range;
}

void FsILS::GetLandingPositionAndAttitude(YsVec3 &p,YsAtt3 &a) const
{
	p=pos;
	a=att;
}

////////////////////////////////////////////////////////////

void FsVor::GetDeviation(int &toFrom,double &deviation,const YsVec3 &airPos,const YsVec3 &vorPos,const double &obsInTrueHeading)
{
	double x;
	YsVec3 obsVec,relVec;

	obsVec.Set(sin(obsInTrueHeading),0.0,cos(obsInTrueHeading));
	relVec=vorPos-airPos;
	relVec.SetY(0.0);

	obsVec.Normalize();
	relVec.Normalize();

	x=obsVec*relVec;
	if(x>0.0)
	{
		toFrom=1; // TO
	}
	else
	{
		toFrom=-1;
	}

	deviation=acos(fabs(x));
	if((obsVec^relVec).y()<0.0)
	{
		deviation=-deviation;
	}
}



