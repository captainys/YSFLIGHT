

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include "fs.h"





const char *FsWeatherCloudLayer::CloudLayerTypeString(int cloudLayerType)
{
	switch(cloudLayerType)
	{
	default:
	case FSCLOUDLAYER_NONE:
		return "CLR";
	case FSCLOUDLAYER_OVERCAST:
		return "OVC";
	};
}

int FsWeatherCloudLayer::CloudLayerTypeFromString(const char str[])
{
	if(strcmp(str,"OVC")==0 || strcmp(str,"ovc")==0)
	{
		return FSCLOUDLAYER_OVERCAST;
	}
	return FSCLOUDLAYER_NONE;
}





FsWeather::FsWeather()
{
	wind.Set(0.0,0.0,0.0);
	fog=YSTRUE;
	fogVisibility=20000;

	transWind.Set(0.0,0.0,0.0);
	transFogVisibility=20000;
}

FsWeather::~FsWeather()
{
}

void FsWeather::WeatherTransition(const double &dt)
{
	if(wind!=transWind)
	{
		YsVec3 d;
		double l,chg;

		chg=4.0*dt;

		d=transWind-wind;
		l=d.GetLength();
		if(l<chg)
		{
			wind=transWind;
		}
		else
		{
			d/=l;
			wind+=d*chg;
		}
	}

	if(YsEqual(fogVisibility,transFogVisibility)!=YSTRUE)
	{
		double d,chg;

		d=transFogVisibility-fogVisibility;
		chg=10000.0*dt;

		if(fabs(d)<chg)
		{
			fogVisibility=transFogVisibility;
		}
		else
		{
			if(d>0)
			{
				fogVisibility+=chg;
			}
			else
			{
				fogVisibility-=chg;
			}
		}
	}
}

const YsVec3 &FsWeather::GetWind(void) const
{
	return wind;
}

YSRESULT FsWeather::SetWind(const YsVec3 &w)
{
	wind=w;
	transWind=w;
	return YSOK;
}

YSRESULT FsWeather::SetTransWind(const YsVec3 &w)
{
	transWind=w;
	return YSOK;
}

const double &FsWeather::GetFogVisibility(void) const
{
	return fogVisibility;
}

YSRESULT FsWeather::SetFogVisibility(const double &visibility)
{
	fogVisibility=YsBound(visibility,FS_FOG_VISIBILITY_MIN,FS_FOG_VISIBILITY_MAX);
	transFogVisibility=fogVisibility;
	return YSOK;
}

YSRESULT FsWeather::SetTransFogVisibility(const double &vis)
{
	transFogVisibility=vis;
	return YSOK;
}

void FsWeather::SetFog(YSBOOL f)
{
	fog=f;
}

YSBOOL FsWeather::GetFog(void) const
{
	return fog;
}

void FsWeather::SetCloudLayer(YSSIZE_T nLayer,const FsWeatherCloudLayer layer[])
{
	cloudLayer.Set(nLayer,layer);
}

void FsWeather::AddCloudLayer(const FsWeatherCloudLayer &layer)
{
	cloudLayer.Append(layer);
}

void FsWeather::GetCloudLayer(int &nLayer,const FsWeatherCloudLayer *&layer) const
{
	nLayer=(int)cloudLayer.GetN();
	layer=cloudLayer;
}

YSBOOL FsWeather::IsInCloudLayer(const YsVec3 &pos) const
{
	int i;
	forYsArray(i,cloudLayer)
	{
		if(cloudLayer[i].y0<=pos.y() && pos.y()<=cloudLayer[i].y1)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSRESULT FsWeather::Save(FILE *fp) const
{
	fprintf(fp,"WEATHERX\n");

	fprintf(fp,"CONSTWIND %lfm/s %lfm/s %lfm/s\n",wind.x(),wind.y(),wind.z());
	fprintf(fp,"VISIBILIT %lfm\n",fogVisibility);

	int i;
	forYsArray(i,cloudLayer)
	{
		fprintf(fp,"CLOUDLAYER %s %lfm %lfm\n",
		    FsWeatherCloudLayer::CloudLayerTypeString(cloudLayer[i].cloudLayerType),
		    cloudLayer[i].y0,
		    cloudLayer[i].y1);
	}

	fprintf(fp,"ENDWEATHER\n");

	return YSOK;
}

static const char *const weatherCmd[]=
{
	"ENDWEATHER",
	"CONSTWIND",
	"VISIBILIT",
	"CLOUDLAYER",
	NULL
};

YSRESULT FsWeather::Load(FILE *fp)
{
	// Keyword WEATHER is already read by FsWorld
	char buf[256];
	int ac;
	char *av[16];

	while(fgets(buf,256,fp)!=NULL)
	{
		if(YsArguments(&ac,av,16,buf)==YSOK)
		{
			int cmd;
			if(YsCommandNumber(&cmd,av[0],weatherCmd)==YSOK)
			{
				switch(cmd)
				{
				case 0:  // ENDWEATHER
					goto ENDWEATHER;
				case 1:  // CONSTWIND
					if(ac>=4)
					{
						double x,y,z;
						if(FsGetSpeed(x,av[1])==YSOK &&
						   FsGetSpeed(y,av[2])==YSOK &&
						   FsGetSpeed(z,av[3])==YSOK)
						{
							wind.Set(x,y,z);
							transWind.Set(x,y,z);  // 2009/04/28
						}
					}
					break;
				case 2:  // VISIBILIT
					if(ac>=2)
					{
						double l;
						if(FsGetLength(l,av[1])==YSOK)
						{
							fogVisibility=l;
							transFogVisibility=l;  // 2009/04/28
						}
					}
					break;
				case 3: // 	"CLOUDLAYER",
					if(ac>=4)
					{
						FsWeatherCloudLayer lyr;
						lyr.cloudLayerType=FsWeatherCloudLayer::CloudLayerTypeFromString(av[1]);
						FsGetLength(lyr.y0,av[2]);
						FsGetLength(lyr.y1,av[3]);
						cloudLayer.Append(lyr);
					}
					break;
				}
			}
		}
	}

ENDWEATHER:
	return YSOK;
}


