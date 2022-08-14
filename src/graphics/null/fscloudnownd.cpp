#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include <fsdef.h>
#include "fscloud.h"


void FsCloud::Draw(void)
{
}

void FsClouds::CreateGraphicCache(void)
{
}

void FsClouds::DeleteGraphicCache(void)
{
}

void FsClouds::Draw(void)
{
}

void FsSolidClouds::ReduceVisibilityByPolygon(const YsMatrix4x4 &viewTfm,const YsColor &col,YSBOOL transparency)
{
	viewTfm;
	col;
	transparency;
}

void FsSolidCloud::CreateGraphicCache(void)
{
}

void FsSolidCloud::DeleteGraphicCache(void)
{
}

void FsSolidCloud::Draw(FSENVIRONMENT /*env*/,const FsWeather & /*weather*/)
{
}

void FsSolidClouds::Test(void)
{
}

void FsSolidClouds::SetUpCloudPerFrame(void)
{
}

void FsSolidClouds::BeginDrawCloud(void)
{
}

void FsSolidClouds::EndDrawCloud(void)
{
}
