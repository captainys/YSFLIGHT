#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include "fs.h"

void FsHeadUpDisplay::DrawCrossHair(void)
{
}

void FsHeadUpDisplay::DrawHeading(const YsAtt3 &a,YSBOOL showHdgBug,const double &hdgBug,YSBOOL selected)
{
}

void FsHeadUpDisplay::DrawThrottle(int nEng,const double thr[],const YSBOOL ab[])
{
}

void FsHeadUpDisplay::DrawNozzle(const YsVec3 &vec)
{
}

void FsHeadUpDisplay::DrawElevator(double elv,double trim,YSBOOL au)
{
}

void FsHeadUpDisplay::DrawAileron(double ail,YSBOOL au)
{
}

void FsHeadUpDisplay::DrawRudder(double rud)
{
}

void FsHeadUpDisplay::DrawSpeed(const double &spd)
{
}

void FsHeadUpDisplay::DrawAltitude(double alt)
{
}

void FsHeadUpDisplay::DrawClimbRatio(const double &climbRatio)
{
}


void FsHeadUpDisplay::DrawBank(const double &b)
{
}


void FsHeadUpDisplay::DrawAttitude(const YsVec3 &pos,const YsAtt3 &att,const YsVec3 &viewPos,const YsAtt3 &viewAtt)
{
}

void FsHeadUpDisplay::DrawVelocityVectorIndicator(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsVec3 &v)
{
	viewPos;
	viewAtt;
	v;
}

void FsHeadUpDisplay::DrawCircleContainer(
    const YsMatrix4x4 &viewpoint,
    const YsAtt3 &viewAtt,
    const YsVec3 &target,
    const YsVec3 &from,
    const YsColor &color,
    const char caption[],
    const char caption2[],
    YSBOOL dot,
    int begin,int step)
{
	viewpoint;
	viewAtt;
	target;
	from;
	color;
	caption;
	caption2;
	dot;
	begin;
	step;
}

void FsHeadUpDisplay::DrawCrossDesignator(
    const YsMatrix4x4 &viewpoint,
    const YsAtt3 &viewAtt,
    const YsVec3 &target,
    const YsVec3 &from,
    const YsColor &color,
    YSBOOL dot)
{
	viewpoint;
	viewAtt;
	target;
	from;
	color;
	dot;
}

void FsHeadUpDisplay::DrawCrossDesignator2(
    const YsMatrix4x4 &viewpoint,
    const YsAtt3 &viewAtt,
    const YsVec3 &target,
    const YsVec3 &from,
    const YsColor &color,
    YSBOOL dot)
{
	viewpoint;
	viewAtt;
	target;
	from;
	color;
	dot;
}

void FsHeadUpDisplay::DrawNeedle(int cx,int cy,int wid,int lng,const double &ang,int tailLng)
{
}

