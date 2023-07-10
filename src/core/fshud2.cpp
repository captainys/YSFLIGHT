#include <ysclass.h>
#include <ysunitconv.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"

#include "fshud2.h"
#include "fsinstpanel.h"

#include "fswirefont.h"

FsHud2::FsHud2() : refZPlane(1.2f)
{
	zPlane=refZPlane;

	specificResource=NULL;

	TakeSpecificResource();
	hudCol.SetIntRGB(100,255,100);

	int i;

	// Bank >>
	const int nBankArc=24;
	const double bankArcRad=0.6;
	bankArc.Set((nBankArc+1)*2,NULL);
	for(i=0; i<=nBankArc; i++)
	{
		const int ii=i-nBankArc/2;
		const double a=(double)ii*(YsPi*2.0/3.0)/(double)nBankArc;

		bankArc[i*2  ]=-sin(a)*bankArcRad;
		bankArc[i*2+1]= cos(a)*bankArcRad;
	}


	// Short hairs at -45,-20,-10,10,20,45   (x6)
	// Long hairs at -60, -30, 0, 30, 60     (x5)
	bankHair.Set(11*4,NULL);

	bankHair[ 0]=-YsSin45deg*bankArcRad;
	bankHair[ 1]= YsCos45deg*bankArcRad;
	bankHair[ 2]=-YsSin45deg*(bankArcRad+0.03);
	bankHair[ 3]= YsCos45deg*(bankArcRad+0.03);

	bankHair[ 4]=-YsSin20deg*bankArcRad;
	bankHair[ 5]= YsCos20deg*bankArcRad;
	bankHair[ 6]=-YsSin20deg*(bankArcRad+0.03);
	bankHair[ 7]= YsCos20deg*(bankArcRad+0.03);

	bankHair[ 8]=-YsSin10deg*bankArcRad;
	bankHair[ 9]= YsCos10deg*bankArcRad;
	bankHair[10]=-YsSin10deg*(bankArcRad+0.03);
	bankHair[11]= YsCos10deg*(bankArcRad+0.03);

	bankHair[12]= YsSin45deg*bankArcRad;
	bankHair[13]= YsCos45deg*bankArcRad;
	bankHair[14]= YsSin45deg*(bankArcRad+0.03);
	bankHair[15]= YsCos45deg*(bankArcRad+0.03);

	bankHair[16]= YsSin20deg*bankArcRad;
	bankHair[17]= YsCos20deg*bankArcRad;
	bankHair[18]= YsSin20deg*(bankArcRad+0.03);
	bankHair[19]= YsCos20deg*(bankArcRad+0.03);

	bankHair[20]= YsSin10deg*bankArcRad;
	bankHair[21]= YsCos10deg*bankArcRad;
	bankHair[22]= YsSin10deg*(bankArcRad+0.03);
	bankHair[23]= YsCos10deg*(bankArcRad+0.03);

	bankHair[24]=-YsSin60deg*bankArcRad;
	bankHair[25]= YsCos60deg*bankArcRad;
	bankHair[26]=-YsSin60deg*(bankArcRad+0.06);
	bankHair[27]= YsCos60deg*(bankArcRad+0.06);

	bankHair[28]=-YsSin30deg*bankArcRad;
	bankHair[29]= YsCos30deg*bankArcRad;
	bankHair[30]=-YsSin30deg*(bankArcRad+0.06);
	bankHair[31]= YsCos30deg*(bankArcRad+0.06);

	bankHair[32]= 0.0*bankArcRad;
	bankHair[33]= 1.0*bankArcRad;
	bankHair[34]= 0.0*(bankArcRad+0.06);
	bankHair[35]= 1.0*(bankArcRad+0.06);

	bankHair[36]= YsSin60deg*bankArcRad;
	bankHair[37]= YsCos60deg*bankArcRad;
	bankHair[38]= YsSin60deg*(bankArcRad+0.06);
	bankHair[39]= YsCos60deg*(bankArcRad+0.06);

	bankHair[40]= YsSin30deg*bankArcRad;
	bankHair[41]= YsCos30deg*bankArcRad;
	bankHair[42]= YsSin30deg*(bankArcRad+0.06);
	bankHair[43]= YsCos30deg*(bankArcRad+0.06);
	// Bank <<

	// Heading >>
	headingOutside.Set(32,NULL);
	for(i=0; i<8; i++)
	{
		const double a=(double)(i*45)*YsPi/180.0;
		headingOutside[i*4  ]=sin(a)*1.02;
		headingOutside[i*4+1]=cos(a)*1.02;
		headingOutside[i*4+2]=sin(a)*1.04;
		headingOutside[i*4+3]=cos(a)*1.04;
	}

	headingInside.Set(72*4,NULL);
	for(i=0; i<72; i++)
	{
		const double a=(double)i*YsPi/36.0;
		const double len=((i%2)==0 ? 0.10 : 0.05);

		headingInside[i*4  ]=sin(a)*1.0;
		headingInside[i*4+1]=cos(a)*1.0;
		headingInside[i*4+2]=sin(a)*(1.0-len);
		headingInside[i*4+3]=cos(a)*(1.0-len);
	}
	// Heading <<

}

FsHud2::~FsHud2()
{
	ReleaseSpecificResource();
}

void FsHud2::SetColor(const YsColor &col)
{
	this->hudCol=col;
}

void FsHud2::DrawCrossHair(const double &lx,const double &ly,const double &centerBlank)
{
	const float crsHairVtx[3*8]=
	{
		-(float)lx         ,0.0f,                 zPlane,
		-(float)centerBlank,0.0f,                 zPlane,

		(float)lx          ,0.0f,                 zPlane,
		(float)centerBlank ,0.0f,                 zPlane,

		0.0f,                 -(float)ly,         zPlane,
		0.0f,                 -(float)centerBlank,zPlane,

		0.0f,                 (float)ly,          zPlane,
		0.0f,                 (float)centerBlank, zPlane,
	};
	for(int i=0; i<8; ++i)
	{
		lineVtxBuf.Add(crsHairVtx[i*3],crsHairVtx[i*3+1],crsHairVtx[i*3+2]);
		lineColBuf.Add(hudCol);
	}
}

void FsHud2::DrawThrottle(const double &x0,const double &y0,const double &wid,const double &hei,int nEng,const double thr[],const YSBOOL ab[])
{
	double x=x0;
	for(int i=0; i<nEng; i++)
	{
		YsColor col;
		if(YSTRUE==ab[i])
		{
			col=YsRed();
		}
		else
		{
			col=hudCol;
		}

		const float rectVtx[6*3]=
		{
			(float)(x    ),(float)(y0           ),zPlane,
			(float)(x    ),(float)(y0+thr[i]*hei),zPlane,
			(float)(x+wid),(float)(y0+thr[i]*hei),zPlane,

			(float)(x+wid),(float)(y0+thr[i]*hei),zPlane,
			(float)(x+wid),(float)(y0           ),zPlane,
			(float)(x    ),(float)(y0           ),zPlane,
		};
		triVtxBuf.Add(rectVtx[ 0],rectVtx[ 1],rectVtx[ 2]);
		triVtxBuf.Add(rectVtx[ 3],rectVtx[ 4],rectVtx[ 5]);
		triVtxBuf.Add(rectVtx[ 6],rectVtx[ 7],rectVtx[ 8]);
		triVtxBuf.Add(rectVtx[ 9],rectVtx[10],rectVtx[11]);
		triVtxBuf.Add(rectVtx[12],rectVtx[13],rectVtx[14]);
		triVtxBuf.Add(rectVtx[15],rectVtx[16],rectVtx[17]);
		triColBuf.Add(col);
		triColBuf.Add(col);
		triColBuf.Add(col);
		triColBuf.Add(col);
		triColBuf.Add(col);
		triColBuf.Add(col);

		const float intelGMAsucks[3*6]=
		{
			(float)x+(float)wid     /3.0f,(float)y0                    ,zPlane,
			(float)x+(float)wid     /3.0f,(float)y0+(float)(thr[i]*hei),zPlane,

			(float)x+(float)wid*2.0f/3.0f,(float)y0                    ,zPlane,
			(float)x+(float)wid*2.0f/3.0f,(float)y0+(float)(thr[i]*hei),zPlane,

			(float)x+(float)wid     /5.0f,(float)y0+(float)(thr[i]*hei),zPlane,
			(float)x+(float)wid*4.0f/5.0f,(float)y0+(float)(thr[i]*hei),zPlane
		};
		lineVtxBuf.Add(intelGMAsucks[ 0],intelGMAsucks[ 1],intelGMAsucks[ 2]);
		lineVtxBuf.Add(intelGMAsucks[ 3],intelGMAsucks[ 4],intelGMAsucks[ 5]);
		lineVtxBuf.Add(intelGMAsucks[ 6],intelGMAsucks[ 7],intelGMAsucks[ 8]);
		lineVtxBuf.Add(intelGMAsucks[ 9],intelGMAsucks[10],intelGMAsucks[11]);
		lineVtxBuf.Add(intelGMAsucks[12],intelGMAsucks[13],intelGMAsucks[14]);
		lineVtxBuf.Add(intelGMAsucks[15],intelGMAsucks[16],intelGMAsucks[17]);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);



		const float frameVtx[3*4]=
		{
			(float)x           ,(float)y0           ,zPlane,
			(float)x           ,(float)y0+(float)hei,zPlane,
			(float)x+(float)wid,(float)y0+(float)hei,zPlane,
			(float)x+(float)wid,(float)y0           ,zPlane
		};
		lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
		lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);

		lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
		lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);

		lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
		lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);

		lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
		lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);

		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
		lineColBuf.Add(col);

		x+=wid;
	}
}

void FsHud2::DrawFuelLeft(const double &x0,const double &y0,const double &wid,const double &hei,const double &fuel,const double &maxFuel)
{
	YsColor col;

	const double ratio=fuel/maxFuel;
	if(0.2<ratio)
	{
		col=YsGreen();
	}
	else if(0.1<ratio)
	{
		col=YsYellow();
	}
	else
	{
		col=YsRed();
	}

	const float fillVtx[6*4]=
	{
		(float)(x0    ),(float)(y0          ),zPlane,
		(float)(x0    ),(float)(y0+hei*ratio),zPlane,
		(float)(x0+wid),(float)(y0+hei*ratio),zPlane,

		(float)(x0+wid),(float)(y0+hei*ratio),zPlane,
		(float)(x0+wid),(float)(y0          ),zPlane,
		(float)(x0    ),(float)(y0          ),zPlane,
	};
	for(int i=0; i<6; ++i)
	{
		triVtxBuf.Add(fillVtx[i*3],fillVtx[i*3+1],fillVtx[i*3+2]);
		triColBuf.Add(col);
	}


	const float GMAGlitch[3*6]=
	{
		(float)(x0+wid    /3.0),(float)(y0          ),zPlane,
		(float)(x0+wid    /3.0),(float)(y0+hei*ratio),zPlane,
                                           
		(float)(x0+wid*2.0/3.0),(float)(y0          ),zPlane,
		(float)(x0+wid*2.0/3.0),(float)(y0+hei*ratio),zPlane,
                                           
		(float)(x0+wid    /5.0),(float)(y0+hei*ratio),zPlane,
		(float)(x0+wid*4.0/5.0),(float)(y0+hei*ratio),zPlane,
	};
	for(int i=0; i<6; ++i)
	{
		lineVtxBuf.Add(GMAGlitch[i*3],GMAGlitch[i*3+1],GMAGlitch[i*3+2]);
		lineColBuf.Add(col);
	}



	const float frameVtx[3*5]=
	{
		(float)x0             ,(float)y0           ,zPlane,
		(float)x0             ,(float)y0+(float)hei,zPlane,
		(float)x0+(float)wid,(float)y0+(float)hei  ,zPlane,
		(float)x0+(float)wid,(float)y0             ,zPlane,

		(float)x0             ,(float)y0           ,zPlane,
	};
	for(int i=0; i<4; ++i)
	{
		lineVtxBuf.Add(frameVtx[i*3  ],frameVtx[i*3+1],frameVtx[i*3+2]);
		lineColBuf.Add(col);
		lineVtxBuf.Add(frameVtx[i*3+3],frameVtx[i*3+4],frameVtx[i*3+5]);
		lineColBuf.Add(col);
	}

	const double fontWid=wid*0.6;
	const double fontHei=wid*0.8;

	YsMatrix4x4 tfm;
	tfm.Translate(x0+wid*0.9,y0+hei*0.1,zPlane);
	tfm.RotateXY(YsPi/2.0);
	tfm.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,tfm,"FUEL",YsWhite());
}

void FsHud2::DrawNozzle(const double &x0,const double &y0,const double &wid,const double &hei,const YsVec3 &vec)
{
	YsVec3 nom=vec;
	nom.Normalize();

	const double loc=0.1+(1.0-YsBound(asin(nom.y())/(YsPi/2.0),0.0,1.0))*0.8;

	double fontWid=wid*0.6;
	double fontHei=wid*0.8;


	const float fillVtx[3*6]=
	{
		(float)(x0        ),(float)(y0+hei*loc        ),zPlane,
		(float)(x0+wid/2.0),(float)(y0+hei*loc+wid/2.0),zPlane,
		(float)(x0+wid    ),(float)(y0+hei*loc        ),zPlane,

		(float)(x0+wid    ),(float)(y0+hei*loc        ),zPlane,
		(float)(x0+wid/2.0),(float)(y0+hei*loc-wid/2.0),zPlane,
		(float)(x0        ),(float)(y0+hei*loc        ),zPlane,
	};
	for(int i=0; i<6; ++i)
	{
		triVtxBuf.Add(fillVtx[i*3],fillVtx[i*3+1],fillVtx[i*3+2]);
		triColBuf.Add(hudCol);
	}

	const float frameVtx[3*5]=
	{
		(float)x0             ,(float)y0           ,zPlane,
		(float)x0             ,(float)y0+(float)hei,zPlane,
		(float)x0+(float)wid,(float)y0+(float)hei  ,zPlane,
		(float)x0+(float)wid,(float)y0             ,zPlane,

		(float)x0             ,(float)y0           ,zPlane,
	};
	for(int i=0; i<4; ++i)
	{
		lineVtxBuf.Add(frameVtx[i*3  ],frameVtx[i*3+1],frameVtx[i*3+2]);
		lineColBuf.Add(hudCol);
		lineVtxBuf.Add(frameVtx[i*3+3],frameVtx[i*3+4],frameVtx[i*3+5]);
		lineColBuf.Add(hudCol);
	}

	YsMatrix4x4 tfm;
	tfm.Translate((float)(x0+wid*0.9),(float)(y0+hei*0.1),zPlane);
	tfm.RotateXY(YsPi/2.0f);
	tfm.Scale((float)fontWid,(float)fontHei,1.0f);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,tfm,"NOZZLE",YsWhite());
}

void FsHud2::DrawGear(const double &x0,const double &y0,const double &wid,const double &hei,const double &gearPos)
{
	const char *str;
	YsColor col=YsWhite();

	if(gearPos>=1.0-YsTolerance)
	{
		str="GEAR DOWN";
		col=hudCol;
	}
	else if(gearPos<=YsTolerance)
	{
		str="GEAR UP";
		col=YsRed();
	}
	else if(gearPos>=0.75)
	{
		str="GEAR ###";
		col=YsYellow();
	}
	else if(gearPos>=0.50)
	{
		str="GEAR ##";
		col=YsYellow();
	}
	else if(gearPos>=0.25)
	{
		str="GEAR #";
		col=YsYellow();
	}
	else
	{
		str="GEAR";
		col=YsYellow();
	}

	const float fontWid=(float)wid/12.0f;
	const float fontHei=(float)hei*0.8f;
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,(float)(x0+fontWid/2.0),(float)(y0+hei*0.1),zPlane,str,col,fontWid,fontHei);



	float frameVtx[12]=
	{
		(float)(x0    ),(float)(y0)    ,zPlane,
		(float)(x0+wid),(float)(y0)    ,zPlane,
		(float)(x0+wid),(float)(y0+hei),zPlane,
		(float)(x0    ),(float)(y0+hei),zPlane,
	};
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineColBuf.Add(col);
	lineColBuf.Add(col);
	lineColBuf.Add(col);
	lineColBuf.Add(col);
	lineColBuf.Add(col);
	lineColBuf.Add(col);
	lineColBuf.Add(col);
	lineColBuf.Add(col);
}

void FsHud2::DrawBrake(const double &x0,const double &y0,const double &wid,const double &hei,const double &brake)
{
	if(0.5<brake)
	{
		const float fontWid=(float)wid/12.0f;
		const float fontHei=(float)hei*0.8f;
		const float sx=(float)(x0+fontWid/2.0f);
		const float sy=(float)(y0+hei*0.1f);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,sx,sy,zPlane,"BRAKE",hudCol,fontWid,fontHei);
	}

	const float frameVtx[3*4]=
	{
		(float)(x0    ),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0+hei),zPlane,
		(float)(x0    ),(float)(y0+hei),zPlane,
	};
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
}

void FsHud2::DrawFlap(const double &x0,const double &y0,const double &wid,const double &hei,const double &flap)
{
	const float loc=(float)(0.1+(1.0-flap)*0.8);


	const float fontWid=(float)wid*0.6f;
	const float fontHei=(float)wid*0.8f;


	const float fillVtx[6*3]=
	{
		(float)(x0        ),(float)(y0+hei*loc        ),zPlane,
		(float)(x0+wid/2.0),(float)(y0+hei*loc+wid/2.0),zPlane,
		(float)(x0+wid    ),(float)(y0+hei*loc        ),zPlane,
		(float)(x0+wid    ),(float)(y0+hei*loc        ),zPlane,
		(float)(x0+wid/2.0),(float)(y0+hei*loc-wid/2.0),zPlane,
		(float)(x0        ),(float)(y0+hei*loc        ),zPlane,
	};
	for(int i=0; i<6; ++i)
	{
		triVtxBuf.Add(fillVtx[i*3],fillVtx[i*3+1],fillVtx[i*3+2]);
		triColBuf.Add(hudCol);
	}


	const float intelGMAsucks[5*3]=
	{
		(float)(x0        ),(float)(y0+hei*loc        ),zPlane,
		(float)(x0+wid/2.0),(float)(y0+hei*loc+wid/2.0),zPlane,
		(float)(x0+wid    ),(float)(y0+hei*loc        ),zPlane,
		(float)(x0+wid/2.0),(float)(y0+hei*loc-wid/2.0),zPlane,

		(float)(x0        ),(float)(y0+hei*loc        ),zPlane,
	};
	for(int i=0; i<4; i++)
	{
		lineVtxBuf.Add(intelGMAsucks[i*3  ],intelGMAsucks[i*3+1],intelGMAsucks[i*3+2]);
		lineColBuf.Add(hudCol);
	}


	const float frameVtx[3*4]=
	{
		(float)(x0    ),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0+hei),zPlane,
		(float)(x0    ),(float)(y0+hei),zPlane
	};
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);

	YsMatrix4x4 tfm;
	tfm.Translate((float)x0+(float)wid*0.9f,(float)y0+(float)hei*0.1f,zPlane);
	tfm.RotateXY(YsPi/2.0);
	tfm.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,tfm,"FLAPS",YsWhite());
}

void FsHud2::DrawSpoiler(const double &x0,const double &y0,const double &wid,const double &hei,const double &spoiler)
{
	const float fillVtx[3*6]=
	{
		(float)(x0    ),(float)(y0            ),zPlane,
		(float)(x0    ),(float)(y0+hei*spoiler),zPlane,
		(float)(x0+wid),(float)(y0+hei*spoiler),zPlane,
		(float)(x0+wid),(float)(y0+hei*spoiler),zPlane,
		(float)(x0+wid),(float)(y0            ),zPlane,
		(float)(x0    ),(float)(y0            ),zPlane,
	};
	for(int i=0; i<6; ++i)
	{
		triVtxBuf.Add(fillVtx[i*3],fillVtx[i*3+1],fillVtx[i*3+2]);
		triColBuf.Add(YsRed());
	}


	const float intelGMAsucks[6*3]=
	{
		(float)(x0+wid    /3.0),(float)(y0            ),zPlane,
		(float)(x0+wid    /3.0),(float)(y0+hei*spoiler),zPlane,
		(float)(x0+wid*2.0/3.0),(float)(y0            ),zPlane,
		(float)(x0+wid*2.0/3.0),(float)(y0+hei*spoiler),zPlane,
		(float)(x0+wid    /5.0),(float)(y0+hei*spoiler),zPlane,
		(float)(x0+wid*4.0/5.0),(float)(y0+hei*spoiler),zPlane,
	};
	for(int i=0; i<6; ++i)
	{
		lineVtxBuf.Add(intelGMAsucks[i*3],intelGMAsucks[i*3+1],intelGMAsucks[i*3+2]);
		lineColBuf.Add(YsRed());
	}

	const float frameVtx[3*4]=
	{
		(float)(x0    ),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0+hei),zPlane,
		(float)(x0    ),(float)(y0+hei),zPlane
	};
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);


	const double fontWid=(double)wid*0.6f;
	const double fontHei=(double)wid*0.8f;

	YsMatrix4x4 tfm;
	tfm.Translate((double)x0+(double)wid*0.9,(double)y0+(double)hei*0.1,zPlane);
	tfm.RotateXY(YsPi/2.0);
	tfm.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,tfm,"SPOILER",YsWhite());
}

void FsHud2::DrawAutoPilot(const double &x0,const double &y0,const double &wid,const double &hei,YSBOOL /*autoPilot*/)
{
	const float fontWid=(float)wid/8.0f;
	const float fontHei=(float)hei*0.8f;
	const float x=(float)x0+fontWid/2.0f;
	const float y=(float)(y0+hei*0.1);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,x,y,zPlane,"AUTO",hudCol,fontWid,fontHei);
}

void FsHud2::DrawMachAndG(const double &x0,const double &y0,const double &fontWid,const double &fontHei,const double &mach,const double &g)
{
	char str[256];

	float x=(float)x0;
	float y=(float)y0;

	sprintf(str,"%5.2lf M",mach);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,x,y,zPlane,str,hudCol,fontWid,fontHei);
	y-=fontHei*1.2f;

	sprintf(str,"%5.2lf G",g);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,x,y,zPlane,str,hudCol,fontWid,fontHei);
}

void FsHud2::DrawControlSurface(
    const double &x0,const double &y0,const double &wid,const double &hei,
    const double &elv,const double &elvTrim,const double &ail,const double &rud)
{
	const float lineVtx[12*3]=
	{
		(float)(x0+wid/2.0-rud*wid*0.5),(float)(y0),                        zPlane,
		(float)(x0+wid/2.0-rud*wid*0.5),(float)(y0-hei*0.3),                zPlane,
		(float)(x0        )            ,(float)(y0+hei*0.6+ail*hei*0.4),    zPlane,
		(float)(x0+wid/3.0)            ,(float)(y0+hei*0.6+ail*hei*0.4),    zPlane,
		(float)(x0+wid*2.0/3.0)        ,(float)(y0+hei*0.6-ail*hei*0.4),    zPlane,
		(float)(x0+wid        )        ,(float)(y0+hei*0.6-ail*hei*0.4),    zPlane,
		(float)(x0+wid/3.0    )        ,(float)(y0+hei*0.6+elv*hei*0.4),    zPlane,
		(float)(x0+wid*2.0/3.0)        ,(float)(y0+hei*0.6+elv*hei*0.4),    zPlane,
		(float)(x0+wid/3.0    )        ,(float)(y0+hei*0.6+elvTrim*hei*0.4),zPlane,
		(float)(x0+wid*2.0/3.0)        ,(float)(y0+hei*0.6+elvTrim*hei*0.4),zPlane,
		(float)(x0+wid/2.0)            ,(float)(y0+hei*0.6),                zPlane,
		(float)(x0+wid/2.0)            ,(float)(y0+hei*0.6+elvTrim*hei*0.4),zPlane,
	};
	for(int i=0; i<12; i+=2)
	{
		lineVtxBuf.Add(lineVtx[i*3  ],lineVtx[i*3+1],lineVtx[i*3+2]);
		lineVtxBuf.Add(lineVtx[i*3+3],lineVtx[i*3+4],lineVtx[i*3+5]);
		lineColBuf.Add(hudCol);
		lineColBuf.Add(hudCol);
	}

	const float pointVtx[6*3]=
	{
		(float)(x0            ),(float)(y0+hei*0.6),zPlane,
		(float)(x0+wid/3.0    ),(float)(y0+hei*0.6),zPlane,
		(float)(x0+wid*2.0/3.0),(float)(y0+hei*0.6),zPlane,
		(float)(x0+wid        ),(float)(y0+hei*0.6),zPlane,

		(float)(x0+wid/2.0)    ,(float)(y0),        zPlane,
		(float)(x0+wid/2.0)    ,(float)(y0-hei*0.3),zPlane
	};
	for(int i=0; i<6; ++i)
	{
		pointVtxBuf.Add(pointVtx[i*3],pointVtx[i*3+1],pointVtx[i*3+2]);
		pointColBuf.Add(hudCol);
	}
}

void FsHud2::DrawElevatorTrim(
    const double &x0,const double &y0,const double &wid,const double &hei,const double elvTrim)
{
	const float triVtx[3*3]=
	{
		(float)(x0+wid/2.0),(float)(y0+(1.0+elvTrim     )*hei/2.0),zPlane,
		(float)(x0+wid    ),(float)(y0+(1.0+elvTrim+0.05)*hei/2.0),zPlane,
		(float)(x0+wid    ),(float)(y0+(1.0+elvTrim-0.05)*hei/2.0),zPlane
	};
	triVtxBuf.Add(triVtx[0],triVtx[1],triVtx[2]);
	triVtxBuf.Add(triVtx[3],triVtx[4],triVtx[5]);
	triVtxBuf.Add(triVtx[6],triVtx[7],triVtx[8]);
	triColBuf.Add(hudCol);
	triColBuf.Add(hudCol);
	triColBuf.Add(hudCol);



	const float lineVtx[3*2]=
	{
		(float) x0,     (float)(y0+hei/2.0),zPlane,
		(float)(x0+wid),(float)(y0+hei/2.0),zPlane
	};
	lineVtxBuf.Add(lineVtx[0],lineVtx[1],lineVtx[2]);
	lineVtxBuf.Add(lineVtx[3],lineVtx[4],lineVtx[5]);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);



	const float frameVtx[3*4]=
	{
		(float)(x0    ),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0    ),zPlane,
		(float)(x0+wid),(float)(y0+hei),zPlane,
		(float)(x0    ),(float)(y0+hei),zPlane
	};
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
}

void FsHud2::DrawAttitude(
    const double &rad,const double &baseLng,const double &fontWid,const double &fontHei,
    const YsVec3 &cockpitPos,const YsAtt3 &att,const YsAtt3 &indicated,const YsVec3 & /*viewPos*/ ,const YsAtt3 & /*viewAtt*/)
{
	YsMatrix4x4 mat;
	mat.Translate(0,0,zPlane-refZPlane);

	// Simulating gyro failure. >>
	mat.RotateXY(-(indicated.b()));
	mat.RotateYZ( (indicated.p()));



	const int pitchInInt=(int)(YsRadToDeg(att.p()));
	for(int i=-90; i<=90; i+=5)
	{
		if(pitchInInt-15<=i && i<=pitchInInt+15)
		{
			double lng;

			YsMatrix4x4 inMat=mat;
			inMat.RotateZY(YsDegToRad(i));

			if(0==i)
			{
				lng=baseLng*1.5;
			}
			else if(0==i%10)
			{
				lng=baseLng;
			}
			else
			{
				lng=baseLng/1.5;
			}

			int nLineVtx=0;
			float lineVtx[20*3];

			if(0<=i)
			{
				nLineVtx=2;
				lineVtx[0]=(float)-lng;
				lineVtx[1]=0.0f;
				lineVtx[2]=(float)rad;
				lineVtx[3]=(float) lng;
				lineVtx[4]=0.0f;
				lineVtx[5]=(float)rad;
			}
			else
			{
				nLineVtx=20;
				for(int j=0; j<10; j++)
				{
					const float x0=(float)(-lng+lng*2.2*(double)j/10.0);
					const float x1=(float)(x0+lng*0.055);
					lineVtx[j*6+0]=x0;
					lineVtx[j*6+1]=0.0f;
					lineVtx[j*6+2]=(float)rad;
					lineVtx[j*6+3]=x1;
					lineVtx[j*6+4]=0.0f;
					lineVtx[j*6+5]=(float)rad;
				}
			}

			for(int j=0; j<nLineVtx; ++j)
			{
				YsVec3 pos(lineVtx[j*3],lineVtx[j*3+1],lineVtx[j*3+2]);
				pos=inMat*pos;
				lineVtxBuf.Add(pos);
				lineColBuf.Add(hudCol);
			}

			if(i%10==0)
			{
				char str[4];
				if(0==i)
				{
					str[0]=' ';
					str[1]=' ';
					str[2]='0';
				}
				else if(0<=i)
				{
					str[0]=' ';
					str[1]='0'+i/10;
					str[2]='0'+i%10;
				}
				else
				{
					str[0]='-';
					str[1]='0'+(-i)/10;
					str[2]='0'+(-i)%10;
				}
				str[3]=0;

				auto fontMat=inMat;
				fontMat.Translate((float)(-lng-fontWid*3.5),(float)(-fontHei/2.0f),(float)rad);
				fontMat.Scale((float)fontWid,(float)fontHei,1.0);
				FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,hudCol);

				fontMat=inMat;
				fontMat.Translate((float)lng,(float)(-fontHei/2.0f),(float)rad);
				fontMat.Scale((float)fontWid,(float)fontHei,1.0);
				FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,hudCol);
			}
		}
	}
}

void FsHud2::DrawBank(const double &bank)
{
	// Bank >>
	const int nBankArc=24;
	const double bankArcRad=0.52;
	double prevX,prevY;
	bankArc.Set((nBankArc+1)*2,NULL);
	for(int i=0; i<=nBankArc; i++)
	{
		const int ii=i-nBankArc/2;
		const double a=(double)ii*(YsPi*2.0/3.0)/(double)nBankArc;

		double x=-sin(a-bank)*bankArcRad;
		double y= cos(a-bank)*bankArcRad;
		if(0<i)
		{
			lineVtxBuf.Add<double>(prevX,prevY,zPlane);
			lineColBuf.Add(hudCol);
			lineVtxBuf.Add<double>(x,y,zPlane);
			lineColBuf.Add(hudCol);
		}
		prevX=x;
		prevY=y;
	}


	// Short hairs at -45,-20,-10,10,20,45   (x6)
	// Long hairs at -60, -30, 0, 30, 60     (x5)
	double bankHair[11*4]=
	{
		-YsSin45deg*bankArcRad,
		 YsCos45deg*bankArcRad,
		-YsSin45deg*(bankArcRad+0.03),
		 YsCos45deg*(bankArcRad+0.03),

		-YsSin20deg*bankArcRad,
		 YsCos20deg*bankArcRad,
		-YsSin20deg*(bankArcRad+0.03),
		 YsCos20deg*(bankArcRad+0.03),

		-YsSin10deg*bankArcRad,
		 YsCos10deg*bankArcRad,
		-YsSin10deg*(bankArcRad+0.03),
		 YsCos10deg*(bankArcRad+0.03),

		 YsSin45deg*bankArcRad,
		 YsCos45deg*bankArcRad,
		 YsSin45deg*(bankArcRad+0.03),
		 YsCos45deg*(bankArcRad+0.03),

		 YsSin20deg*bankArcRad,
		 YsCos20deg*bankArcRad,
		 YsSin20deg*(bankArcRad+0.03),
		 YsCos20deg*(bankArcRad+0.03),

		 YsSin10deg*bankArcRad,
		 YsCos10deg*bankArcRad,
		 YsSin10deg*(bankArcRad+0.03),
		 YsCos10deg*(bankArcRad+0.03),

		-YsSin60deg*bankArcRad,
		 YsCos60deg*bankArcRad,
		-YsSin60deg*(bankArcRad+0.06),
		 YsCos60deg*(bankArcRad+0.06),

		-YsSin30deg*bankArcRad,
		 YsCos30deg*bankArcRad,
		-YsSin30deg*(bankArcRad+0.06),
		 YsCos30deg*(bankArcRad+0.06),

		 0.0*bankArcRad,
		 1.0*bankArcRad,
		 0.0*(bankArcRad+0.06),
		 1.0*(bankArcRad+0.06),

		 YsSin60deg*bankArcRad,
		 YsCos60deg*bankArcRad,
		 YsSin60deg*(bankArcRad+0.06),
		 YsCos60deg*(bankArcRad+0.06),

		 YsSin30deg*bankArcRad,
		 YsCos30deg*bankArcRad,
		 YsSin30deg*(bankArcRad+0.06),
		 YsCos30deg*(bankArcRad+0.06),
	};
	for(int i=0; i<11; ++i)
	{
		const double s=sin(-bank);
		const double c=cos(-bank);

		const double xx0=bankHair[i*4];
		const double yy0=bankHair[i*4+1];
		const double xx1=bankHair[i*4+2];
		const double yy1=bankHair[i*4+3];

		const double x0=c*xx0-s*yy0;
		const double y0=s*xx0+c*yy0;
		const double x1=c*xx1-s*yy1;
		const double y1=s*xx1+c*yy1;

		lineVtxBuf.Add<double>(x0,y0,zPlane);
		lineColBuf.Add(hudCol);
		lineVtxBuf.Add<double>(x1,y1,zPlane);
		lineColBuf.Add(hudCol);
	}
	// Bank <<

	const float tri[9]=
	{
		 0.0f ,0.52f,zPlane,
		-0.03f,0.47f,zPlane,
		 0.03f,0.47f,zPlane,
	};
	triVtxBuf.Add<float>(tri[0],tri[1],tri[2]);
	triVtxBuf.Add<float>(tri[3],tri[4],tri[5]);
	triVtxBuf.Add<float>(tri[6],tri[7],tri[8]);
	triColBuf.Add(hudCol);
	triColBuf.Add(hudCol);
	triColBuf.Add(hudCol);
}

void FsHud2::DrawHUDText(const double& x0, const double& y0, const double& fontWid, const double& fontHei, const YsString& str)
{
	double centerTxtOffsetX = 0.0;
	if (str.length() % 2 == 0)
	{
		centerTxtOffsetX = fontWid * (double)str.length() / 2.0 + fontWid / 2.0;
	}
	else
	{
		centerTxtOffsetX = fontWid * (double)str.length() / 2.0;
	}
	FsAddWireFontVertexBuffer(lineVtxBuf, lineColBuf, triVtxBuf, triColBuf, x0 - centerTxtOffsetX, y0, zPlane, str, hudCol, fontWid, fontHei);

	//float frameVtx[12] =
	//{
	//	(float)(x0),(float)(y0)    ,zPlane,
	//	(float)(x0 + wid),(float)(y0)    ,zPlane,
	//	(float)(x0 + wid),(float)(y0 + hei),zPlane,
	//	(float)(x0),(float)(y0 + hei),zPlane,
	//};
	//lineVtxBuf.Add(frameVtx[0], frameVtx[1], frameVtx[2]);
	//lineVtxBuf.Add(frameVtx[3], frameVtx[4], frameVtx[5]);
	//lineVtxBuf.Add(frameVtx[3], frameVtx[4], frameVtx[5]);
	//lineVtxBuf.Add(frameVtx[6], frameVtx[7], frameVtx[8]);
	//lineVtxBuf.Add(frameVtx[6], frameVtx[7], frameVtx[8]);
	//lineVtxBuf.Add(frameVtx[9], frameVtx[10], frameVtx[11]);
	//lineVtxBuf.Add(frameVtx[9], frameVtx[10], frameVtx[11]);
	//lineVtxBuf.Add(frameVtx[0], frameVtx[1], frameVtx[2]);
	//lineColBuf.Add(hudCol);
	//lineColBuf.Add(hudCol);
	//lineColBuf.Add(hudCol);
	//lineColBuf.Add(hudCol);
	//lineColBuf.Add(hudCol);
	//lineColBuf.Add(hudCol);
	//lineColBuf.Add(hudCol);
	//lineColBuf.Add(hudCol);
}

void FsHud2::DrawAltitude(const double &x0,const double &y0,const double &wid,const double &hei,const double &altInMeter)
{
	// Dimension: (0,0)-(0.2,1.0);

	const float scaleX=(float)wid/0.2f;
	const float scaleY=(float)hei;
	const float transX=(float)x0;
	const float transY=(float)y0;

	const float fontWid=0.02f;
	const float fontHei=0.03f;


	float frameVtx[4*3]=
	{
		0.0f,0.0f,zPlane,
		0.2f,0.0f,zPlane,
		0.2f,1.0f,zPlane,
		0.0f,1.0f,zPlane,
	};
	YsVec2 framePlg[10];
	YsBoundingBoxMaker <YsVec2> frameBbx;
	for(int i=0; i<4; ++i)
	{
		frameVtx[i*3  ]=transX+scaleX*frameVtx[i*3];
		frameVtx[i*3+1]=transY+scaleY*frameVtx[i*3+1];
		framePlg[i].Set(frameVtx[i*3],frameVtx[i*3+1]);
		frameBbx.Add(framePlg[i]);
	}
	YsConstArrayMask <YsVec2> frameClip(4,framePlg);

	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[3],frameVtx[ 4],frameVtx[ 5]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[6],frameVtx[ 7],frameVtx[ 8]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[9],frameVtx[10],frameVtx[11]);
	lineVtxBuf.Add(frameVtx[0],frameVtx[ 1],frameVtx[ 2]);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);
	lineColBuf.Add(hudCol);

	const float zoomVtx[10*3]=
	{
		0.02f                 ,0.5f+fontHei/2.0f+0.02f  ,zPlane,
		0.18f-fontWid*2.0f-0.02f,0.5f+fontHei/2.0f+0.02f,zPlane,
		0.18f-fontWid*2.0f-0.02f,0.5f+fontHei*1.5f+0.02f,zPlane,
		0.18f                 ,0.5f+fontHei*1.5f+0.02f  ,zPlane,
		0.18f                 ,0.5f-fontHei*1.5f-0.02f  ,zPlane,
		0.18f-fontWid*2.0f-0.02f,0.5f-fontHei*1.5f-0.02f,zPlane,
		0.18f-fontWid*2.0f-0.02f,0.5f-fontHei/2.0f-0.02f,zPlane,
		0.02f                 ,0.5f-fontHei/2.0f-0.02f  ,zPlane,
		0.0f                  ,0.5f                     ,zPlane,

		0.02f                 ,0.5f+fontHei/2.0f+0.02f  ,zPlane,
	};
	for(int i=0; i<9; ++i)
	{
		lineVtxBuf.Add(transX+zoomVtx[i*3  ]*scaleX,transY+zoomVtx[i*3+1]*scaleY,zoomVtx[i*3+2]);
		lineColBuf.Add(hudCol);
		lineVtxBuf.Add(transX+zoomVtx[i*3+3]*scaleX,transY+zoomVtx[i*3+4]*scaleY,zoomVtx[i*3+5]);
		lineColBuf.Add(hudCol);
	}
	YsVec2 zoomPlg[10];
	YsBoundingBoxMaker <YsVec2> zoomBbx;
	for(int i=0; i<10; ++i)
	{
		zoomPlg[i].Set(transX+scaleX*zoomVtx[i*3],transY+scaleY*zoomVtx[i*3+1]);
		zoomBbx.Add(zoomPlg[i]);
	}
	YsConstArrayMask <YsVec2> zoomClip(10,zoomPlg);

	const float altInFt=(float)YsUnitConv::MtoFT(altInMeter);
	const float showNumber=altInFt-fmod(altInFt,20.0f);

	const int showNumberInt=(int)(showNumber+0.5);
	const int lowDigit=showNumberInt%100;
	{
		const int highDigit=showNumberInt/100;
		char highDigitStr[16];

		float sx=transX+scaleX*0.03f;
		float sy=transY+scaleY*(0.5f-fontHei/2.0f);

		if(80<=lowDigit)
		{
			const float highDigitShift=fmod(altInFt,20.0f)/20.0f;

			sprintf(highDigitStr,"%4d",highDigit);
			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy-highDigitShift*scaleY*fontHei*1.4f,zPlane,highDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);


			sprintf(highDigitStr,"%4d",highDigit+1);
			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy-highDigitShift*scaleY*fontHei*1.4f+scaleY*fontHei*1.4f,zPlane,highDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);
		}
		else
		{
			sprintf(highDigitStr,"%4d",highDigit);
			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy,zPlane,highDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);
		}
	}
	{
		const float lowDigitShift=fmod(altInFt,20.0f)/20.0f; // 20ft -> shift by 1.0

		char lowDigitStr[16];
		sprintf(lowDigitStr,"%02d",lowDigit);

		float sx=transX+scaleX*0.03f;
		float sy=transY+scaleY*(0.5f-fontHei/2.0f);
		sx+=scaleX*fontWid*4.7f;
		sy+=scaleY*fontHei*(-lowDigitShift*1.4f);
		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,lowDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSINSIDE,zoomClip,zoomBbx);

		char str[16];
		sy-=scaleY*fontHei*1.4f;
		if(showNumberInt>20)
		{
			sprintf(str,"%02d",(lowDigit+80)%100);
			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy,zPlane,str,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);
		}

		sprintf(str,"%02d",(lowDigit+20)%100);
		sy+=scaleY*fontHei*2.8f;
		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,str,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSINSIDE,zoomClip,zoomBbx);

		sprintf(str,"%02d",(lowDigit+40)%100);
		sy+=scaleY*fontHei*1.4f;
		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,lowDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSINSIDE,zoomClip,zoomBbx);
	}

	// 600ft -> 0.5
	int altInFtInt=(int)altInFt;

	for(int alt=((altInFtInt-300)/20)*20+20; alt<altInFtInt+300; alt+=20)
	{
		if(alt>=0)
		{
			const float y=0.5f+((float)alt-altInFt)/600.0f;

			float x0,y0,x1;
			x0=transX+scaleX*0.0f;
			y0=transY+scaleY*y;

			if(alt%100==0)
			{
				x1=transX+scaleX*0.04f;
			}
			else
			{
				x1=transX+scaleX*0.02f;
			}

			YsArray <YsVec2,2> inside,outside;
			YsClipLineByPolygon(inside,outside,YsVec2(x0,y0),YsVec2(x1,y0),zoomClip,zoomBbx);
			for(int i=0; i<outside.GetN()-1; i+=2)
			{
				lineVtxBuf.Add<double>(outside[i  ].x(),outside[i  ].y(),zPlane);
				lineVtxBuf.Add<double>(outside[i+1].x(),outside[i+1].y(),zPlane);
				lineColBuf.Add(hudCol);
				lineColBuf.Add(hudCol);
			}
		}
	}

	for(int alt=((altInFtInt-300)/20)*20; alt<=altInFtInt+300; alt+=20)
	{
		if(alt>=0 && alt%100==0)
		{
			const float y=0.5f+((float)alt-altInFt)/600.0f;
			const float subFontWid=fontWid*0.9f;
			const float subFontHei=fontHei*0.9f;

			char str[16];
			sprintf(str,"%d",alt);

			float sx=transX+scaleX*0.06f;
			float sy=transY+scaleY*(y-subFontHei/2.0f);
			if(altInFtInt-100<=alt && alt<=altInFtInt+100)
			{
				FsAddWireFontVertexBuffer(
				    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
				    sx,sy,zPlane,str,hudCol,scaleX*subFontWid,scaleY*subFontHei,
				    YSOUTSIDE,zoomClip,zoomBbx);
			}
			else
			{
				FsAddWireFontVertexBuffer(
				    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
				    sx,sy,zPlane,str,hudCol,scaleX*subFontWid,scaleY*subFontHei,
				    YSINSIDE,frameClip,frameBbx);
			}
		}
	} 
}

void FsHud2::DrawVSI(const double &x0,const double &y0,const double &wid,const double &hei,const double &ftPerMin)
{
	const int vsiRange=4000;  // 4000ft per min
	const float fontWid=0.03f;
	const float fontHei=0.05f;

	const float scaleX=(float)wid/0.2f;
	const float scaleY=(float)hei;
	const float transX=x0;
	const float transY=y0;

	const float indicate=(float)YsBound(ftPerMin,-(double)vsiRange,(double)vsiRange);
	const float y=0.5f+0.45f*(float)ftPerMin/(float)vsiRange;

	const float needlePenta[6*3]=
	{
		0.0f                           ,y,                   zPlane,
		fontHei/2.0f+0.02f             ,y+fontHei/2.0f+0.02f,zPlane,
		fontHei/2.0f+fontWid*5.0f+0.04f,y+fontHei/2.0f+0.02f,zPlane,
		fontHei/2.0f+fontWid*5.0f+0.04f,y-fontHei/2.0f-0.02f,zPlane,
		fontHei/2.0f+0.02f             ,y-fontHei/2.0f-0.02f,zPlane,

		0.0f                           ,y,                   zPlane,
	};
	YsVec2 zoomPlg[5];
	YsBoundingBoxMaker <YsVec2> zoomBbx;
	for(int i=0; i<5; ++i)
	{
		lineVtxBuf.Add(transX+scaleX*needlePenta[i*3  ],transY+scaleY*needlePenta[i*3+1],needlePenta[i*3+2]);
		lineColBuf.Add(hudCol);
		lineVtxBuf.Add(transX+scaleX*needlePenta[i*3+3],transY+scaleY*needlePenta[i*3+4],needlePenta[i*3+5]);
		lineColBuf.Add(hudCol);
		zoomPlg[i].Set(transX+scaleX*needlePenta[i*3],transY+scaleY*needlePenta[i*3+1]);
		zoomBbx.Add(zoomPlg[i]);
	}
	YsConstArrayMask <YsVec2> zoomClip(5,zoomPlg);



	char str[256];
	sprintf(str,"%d",(int)indicate);
	{
		float sx=x0+scaleX*(fontHei/2.0f+0.02f);
		float sy=y0+scaleY*(y-fontHei/2.0f);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,sx,sy,zPlane,str,hudCol,scaleX*fontWid,scaleY*fontHei);
	}


	const float frameVtx[8*3]=
	{
		0.0f,0.01f,zPlane,
		0.0f,0.99f,zPlane,
		0.1f,0.99f,zPlane,
		0.1f,0.60f,zPlane,
		0.0f,0.50f,zPlane,
		0.1f,0.40f,zPlane,
		0.1f,0.01f,zPlane,
		0.0f,0.01f,zPlane
	};
	for(int i=0; i<7; ++i)
	{
		double x0=transX+scaleX*frameVtx[i*3];
		double y0=transY+scaleY*frameVtx[i*3+1];
		double x1=transX+scaleX*frameVtx[i*3+3];
		double y1=transY+scaleY*frameVtx[i*3+4];

		YsArray <YsVec2,2> inside,outside;
		YsClipLineByPolygon(inside,outside,YsVec2(x0,y0),YsVec2(x1,y1),zoomClip,zoomBbx);
		for(int i=0; i<outside.GetN()-1; i+=2)
		{
			lineVtxBuf.Add<double>(outside[i  ].x(),outside[i  ].y(),zPlane);
			lineVtxBuf.Add<double>(outside[i+1].x(),outside[i+1].y(),zPlane);
			lineColBuf.Add(hudCol);
			lineColBuf.Add(hudCol);
		}
	}

	for(int i=0; i<4; i++)
	{
		double x0,y0,x1,y1;
		YsArray <YsVec2,2> inside,outside;

		x0=transX+scaleX*0.0f;
		y0=transY+scaleY*(0.5f+(i+1)*0.45f/4.0f);
		x1=transX+scaleX*0.02f;
		y1=transY+scaleY*(0.5f+(i+1)*0.45f/4.0f);
		YsClipLineByPolygon(inside,outside,YsVec2(x0,y0),YsVec2(x1,y1),zoomClip,zoomBbx);
		for(int i=0; i<outside.GetN()-1; i+=2)
		{
			lineVtxBuf.Add<double>(outside[i  ].x(),outside[i  ].y(),zPlane);
			lineVtxBuf.Add<double>(outside[i+1].x(),outside[i+1].y(),zPlane);
			lineColBuf.Add(hudCol);
			lineColBuf.Add(hudCol);
		}

		x0=transX+scaleX*0.0f;
		y0=transY+scaleY*(0.5f-(i+1)*0.45f/4.0f);
		x1=transX+scaleX*0.02f;
		y1=transY+scaleY*(0.5f-(i+1)*0.45f/4.0f);
		YsClipLineByPolygon(inside,outside,YsVec2(x0,y0),YsVec2(x1,y1),zoomClip,zoomBbx);
		for(int i=0; i<outside.GetN()-1; i+=2)
		{
			lineVtxBuf.Add<double>(outside[i  ].x(),outside[i  ].y(),zPlane);
			lineVtxBuf.Add<double>(outside[i+1].x(),outside[i+1].y(),zPlane);
			lineColBuf.Add(hudCol);
			lineColBuf.Add(hudCol);
		}
	}

	for(int i=2; i<=4; i+=2)
	{
		char str[16];
		str[0]='0'+(char)i;
		str[1]=0;

		float sx=transX+scaleX*0.05f;
		float sy=transY+scaleY*(0.5f+(float)i*0.45f/4.0f-fontHei/2.0f);
		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,str,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSOUTSIDE,zoomClip,zoomBbx);

		sy=transY+scaleY*(0.5f-(float)i*0.45f/4.0f-fontHei/2.0f);
		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,str,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSOUTSIDE,zoomClip,zoomBbx);
	}
}

void FsHud2::DrawAirSpeed(const double &x0,const double &y0,const double &wid,const double &hei,const double &speedInMeterPerSec)
{
	const float scaleX=(float)wid/0.2f;
	const float scaleY=(float)hei;
	float transX=(float)x0;
	float transY=(float)y0;

	const float fontWid=0.03f;
	const float fontHei=0.05f;

	const float frameVtx[5*3]=
	{
		0.0f,0.0f,zPlane,
		0.2f,0.0f,zPlane,
		0.2f,1.0f,zPlane,
		0.0f,1.0f,zPlane,

		0.0f,0.0f,zPlane,
	};
	for(int i=0; i<4; ++i)
	{
		lineVtxBuf.Add(transX+frameVtx[i*3  ]*scaleX,transY+frameVtx[i*3+1]*scaleY,frameVtx[i*3+2]);
		lineColBuf.Add(hudCol);
		lineVtxBuf.Add(transX+frameVtx[i*3+3]*scaleX,transY+frameVtx[i*3+4]*scaleY,frameVtx[i*3+5]);
		lineColBuf.Add(hudCol);
	}
	YsVec2 framePlg[4];
	YsBoundingBoxMaker <YsVec2> frameBbx;
	for(int i=0; i<4; ++i)
	{
		framePlg[i].Set(transX+frameVtx[i*3  ]*scaleX,transY+frameVtx[i*3+1]*scaleY);
		frameBbx.Add(framePlg[i]);
	}
	YsConstArrayMask <YsVec2> frameClip(4,framePlg);


	const float zoomVtx[12*3]=
	{
		0.2f                   ,0.5f                   ,zPlane,
		0.18f                  ,0.5f+fontHei/2.0f+0.02f,zPlane,
		0.18f                  ,0.5f+fontHei*1.5f+0.02f,zPlane,
		0.18f-fontWid-0.02f    ,0.5f+fontHei*1.5f+0.02f,zPlane,
		0.18f-fontWid-0.02f    ,0.5f+fontHei/2.0f+0.02f,zPlane,
		0.02f                  ,0.5f+fontHei/2.0f+0.02f,zPlane,
		0.02f                  ,0.5f-fontHei/2.0f-0.02f,zPlane,
		0.18f-fontWid-0.02f    ,0.5f-fontHei/2.0f-0.02f,zPlane,
		0.18f-fontWid-0.02f    ,0.5f-fontHei*1.5f-0.02f,zPlane,
		0.18f                  ,0.5f-fontHei*1.5f-0.02f,zPlane,
		0.18f                  ,0.5f-fontHei/2.0f-0.02f,zPlane,

		0.2f                   ,0.5f                   ,zPlane,
	};
	for(int i=0; i<11; ++i)
	{
		lineVtxBuf.Add(transX+zoomVtx[i*3  ]*scaleX,transY+zoomVtx[i*3+1]*scaleY,zoomVtx[i*3+2]);
		lineColBuf.Add(hudCol);
		lineVtxBuf.Add(transX+zoomVtx[i*3+3]*scaleX,transY+zoomVtx[i*3+4]*scaleY,zoomVtx[i*3+5]);
		lineColBuf.Add(hudCol);
	}
	YsVec2 zoomPlg[12];
	YsBoundingBoxMaker <YsVec2> zoomBbx;
	for(int i=0; i<12; ++i)
	{
		zoomPlg[i].Set(transX+scaleX*zoomVtx[i*3],transY+scaleY*zoomVtx[i*3+1]);
		zoomBbx.Add(zoomPlg[i]);
	}
	YsConstArrayMask <YsVec2> zoomClip(12,zoomPlg);



	const float spdInKt=(float)speedInMeterPerSec*1.94384449f;

	const float showNumber=spdInKt-fmod(spdInKt,1.0f);
	const float lowDigitShift=fmod(spdInKt,1.0f);

	const int showNumberInt=(int)(showNumber+0.5f);
	const int lowDigit=showNumberInt%10;
	const int highDigit=showNumberInt/10;

	char highDigitStr[16];
	char lowDigitStr[16];
	lowDigitStr[0]='0'+lowDigit;
	lowDigitStr[1]=0;



	{
		float sx=transX+scaleX*(0.17f-fontWid*5.0f);
		float sy=transY+scaleY*(0.5f-fontHei/2.0f);

		if(9<=lowDigit)
		{
			const float highDigitShift=fmod(spdInKt,1.0f);

			sprintf(highDigitStr,"%3d",highDigit);

			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy-highDigitShift*fontHei*1.4f,zPlane,highDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);


			sprintf(highDigitStr,"%3d",highDigit+1);

			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy-highDigitShift*fontHei*1.4f+fontHei*1.4f,zPlane,highDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);
		}
		else
		{
			sprintf(highDigitStr,"%3d",highDigit);

			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy,zPlane,highDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);
		}

		sx=transX+scaleX*(0.17f-fontWid);
		sy=transY+scaleY*(0.5f-fontHei/2.0f-fontHei*lowDigitShift*1.4f);
		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,lowDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSINSIDE,zoomClip,zoomBbx);

		sy-=scaleY*fontHei*1.4;

		if(showNumberInt>20)
		{
			lowDigitStr[0]='0'+(lowDigit+9)%10;
			lowDigitStr[1]=0;

			FsAddWireFontVertexBuffer(
			    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
			    sx,sy,zPlane,lowDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
			    YSINSIDE,zoomClip,zoomBbx);
		}

		lowDigitStr[0]='0'+(lowDigit+1)%10;
		lowDigitStr[1]=0;

		sy+=scaleY*fontHei*2.8;

		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,lowDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSINSIDE,zoomClip,zoomBbx);


		lowDigitStr[0]='0'+(lowDigit+2)%10;
		lowDigitStr[1]=0;

		sy+=scaleY*fontHei*1.4;

		FsAddWireFontVertexBuffer(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    sx,sy,zPlane,lowDigitStr,hudCol,scaleX*fontWid,scaleY*fontHei,
		    YSINSIDE,zoomClip,zoomBbx);
	}



	// 30kt -> 0.5
	const int spdInKtInt=(int)spdInKt;

	for(int spd=((spdInKtInt-30)/5)*5+5; spd<spdInKtInt+30; spd+=5)
	{
		if(spd>=0)
		{
			const float y=0.5f+((float)spd-spdInKt)/60.0f;
			YsVec2 p0,p1;
			if(spd%10==0)
			{
				p0.Set(transX+scaleX*0.2f, transY+scaleY*y);
				p1.Set(transX+scaleX*0.16f,transY+scaleY*y);
			}
			else
			{
				p0.Set(transX+scaleX*0.2f, transY+scaleY*y);
				p1.Set(transX+scaleX*0.18f,transY+scaleY*y);
			}
			YsArray <YsVec2,2> inside,outside;
			YsClipLineByPolygon(inside,outside,p0,p1,zoomClip,zoomBbx);
			for(int i=0; i<outside.GetN()-1; i+=2)
			{
				lineVtxBuf.Add<double>(outside[i  ].x(),outside[i  ].y(),zPlane);
				lineVtxBuf.Add<double>(outside[i+1].x(),outside[i+1].y(),zPlane);
				lineColBuf.Add(hudCol);
				lineColBuf.Add(hudCol);
			}
		}
	}

	for(int spd=((spdInKtInt-30)/5)*5; spd<=spdInKtInt+30; spd+=5)
	{
		if(spd>=0)
		{
			if(spd%10==0)
			{
				const float y=0.5f+((float)spd-spdInKt)/60.0f;
				const float subFontWid=fontWid*0.9f;
				const float subFontHei=fontHei*0.9f;

				char label[16];
				sprintf(label,"%4d",spd);

				float sx=transX+scaleX*(0.14f-subFontWid*4.0f);
				float sy=transY+scaleY*(y-subFontHei/2.0f);
				if(spdInKtInt-5<=spd && spd<=spdInKtInt+5)
				{
					FsAddWireFontVertexBuffer(
					    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
					    sx,sy,zPlane,label,hudCol,scaleX*subFontWid,scaleY*subFontHei,
					    YSOUTSIDE,zoomClip,zoomBbx);
				}
				else
				{
					FsAddWireFontVertexBuffer(
					    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
					    sx,sy,zPlane,label,hudCol,scaleX*subFontWid,scaleY*subFontHei,
					    YSINSIDE,frameClip,frameBbx);
				}
			}
		 }
	}
}

void FsHud2::DrawAmmo(
    const double &x0,const double &y0,const double &fontWid,const double &fontHei,
    const class FsAmmunitionIndication &ammo)
{
	float x=(float)x0;
	float y=(float)(y0+fontHei);

	for(auto &a : ammo.ammoArray)
	{
		auto str=a.FormatString();
		YsColor col;
		if(YSTRUE==a.ReadyToFire())
		{
			col=hudCol;
		}
		else
		{
			col=YsRed();
		}
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,x,y,zPlane,str,col,fontWid,fontHei);
		y-=fontHei*1.1;
	}
}

void FsHud2::DrawTurnAndSlipIndicator(const double cxd,const double cyd,const double rad,const double ssa,const double turnRate)
{
	const float cx=(float)cxd;
	const float cy=(float)cyd;

	const float outRad=(float)rad;
	const float inRad=outRad*0.8f;

	// Ball
	const float ballCirCx=cx;
	const float ballCirCy=cy+outRad*0.6f;
	const float ballCirRad=outRad;

	{
		float first[3],prev[3];
		for(int a=-30; a<=30; a+=10)  // 7 steps total * 2 vtx each
		{
			const float c=(float)cos(YsDegToRad(a));
			const float s=(float)sin(YsDegToRad(a));

			float x=ballCirCx+s*ballCirRad*0.9f;
			float y=ballCirCy-c*ballCirRad*0.9f;
			float z=zPlane;

			if(-30==a)
			{
				first[0]=x;
				first[1]=y;
				first[2]=z;
			}
			else
			{
				lineVtxBuf.Add(prev[0],prev[1],prev[2]);
				lineVtxBuf.Add(x,y,z);
				lineColBuf.Add(hudCol);
				lineColBuf.Add(hudCol);
			}
			prev[0]=x;
			prev[1]=y;
			prev[2]=z;
		}
		for(int a=30; a>=-30; a-=10)
		{
			const float c=(float)cos(YsDegToRad(a));
			const float s=(float)sin(YsDegToRad(a));

			float x=ballCirCx+s*ballCirRad*1.1f;
			float y=ballCirCy-c*ballCirRad*1.1f;
			float z=zPlane;

			lineVtxBuf.Add(prev[0],prev[1],prev[2]);
			lineVtxBuf.Add(x,y,z);
			lineColBuf.Add(hudCol);
			lineColBuf.Add(hudCol);

			prev[0]=x;
			prev[1]=y;
			prev[2]=z;
		}

		lineVtxBuf.Add(prev[0],prev[1],prev[2]);
		lineVtxBuf.Add(first[0],first[1],first[2]);
		lineColBuf.Add(hudCol);
		lineColBuf.Add(hudCol);
	}


	const float lineVtx[8*3]=
	{
		cx-inRad,                     cy,                            zPlane,
		cx-outRad,                    cy,                            zPlane,
		cx+inRad,                     cy,                            zPlane,
		cx+outRad,                    cy,                            zPlane,

		cx-inRad*(float)YsCos20deg, cy-inRad*(float)YsSin20deg,  zPlane,
		cx-outRad*(float)YsCos20deg,cy-outRad*(float)YsSin20deg, zPlane,
		cx+inRad*(float)YsCos20deg, cy-inRad*(float)YsSin20deg,  zPlane,
		cx+outRad*(float)YsCos20deg,cy-outRad*(float)YsSin20deg, zPlane
	};
	for(int i=0; i<8; ++i)
	{
		lineVtxBuf.Add(lineVtx[i*3],lineVtx[i*3+1],lineVtx[i*3+2]);
		lineColBuf.Add(hudCol);
	}



	float tilt,c,s,x,y;

	tilt=YsBound <float> ((float)ssa*5.0f,-(float)YsPi/9.0f,(float)YsPi/9.0f);
	c=(float)cos(tilt);
	s=(float)sin(tilt);
	x=ballCirCx+s*ballCirRad;
	y=ballCirCy-c*ballCirRad;

	const float scl=ballCirRad*0.1f;
	const float ballVtx[8*3]=
	{
		x+scl*1.0f,             y+scl*0.0f,             zPlane,
		x+scl*(float)YsCos45deg,y+scl*(float)YsSin45deg,zPlane,
		x+scl*0.0f,             y+scl*1.0f,             zPlane,
		x-scl*(float)YsCos45deg,y+scl*(float)YsSin45deg,zPlane,
		x-scl*1.0f,             y+scl*0.0f,             zPlane,
		x-scl*(float)YsCos45deg,y-scl*(float)YsSin45deg,zPlane,
		x+scl*0.0f,             y-scl*1.0f,             zPlane,
		x+scl*(float)YsCos45deg,y-scl*(float)YsSin45deg,zPlane,
	};
	for(int i=0; i<8; ++i)
	{
		lineVtxBuf.Add(ballVtx[i*3],ballVtx[i*3+1],ballVtx[i*3+2]);
		lineColBuf.Add(hudCol);
		lineVtxBuf.Add(ballVtx[((i+1)&7)*3],ballVtx[((i+1)&7)*3+1],ballVtx[((i+1)&7)*3+2]);
		lineColBuf.Add(hudCol);
	}



	// 360deg/2min -> 20 deg tilt
	// 3deg/sec -> 20 deg tilt
	// 1deg/sec -> 20/3 deg tilt
	tilt=(float)turnRate*20.0f/3.0f;
	tilt=YsBound <float> (tilt,-(float)YsPi/3.0f,(float)YsPi/3.0f);

	const float miniatureAirplaneVtx[24*3]=
	{
		-0.700000f,-0.010000f,zPlane,
		-0.700000f, 0.010000f,zPlane,
		-0.117694f, 0.023411f,zPlane,
		-0.099776f, 0.066668f,zPlane,
		-0.066668f, 0.099776f,zPlane,
		-0.023411f, 0.117694f,zPlane,
		-0.010000f, 0.300000f,zPlane,
		 0.010000f, 0.300000f,zPlane,
		 0.023411f, 0.117694f,zPlane,
		 0.066668f, 0.099776f,zPlane,
		 0.099776f, 0.066668f,zPlane,
		 0.117694f, 0.023411f,zPlane,
		 0.700000f, 0.010000f,zPlane,
		 0.700000f,-0.010000f,zPlane,
		 0.117694f,-0.023411f,zPlane,
		 0.110866f,-0.045922f,zPlane,
		 0.084853f,-0.084853f,zPlane,
		 0.045922f,-0.110866f,zPlane,
		 0.000000f,-0.120000f,zPlane,
		-0.045922f,-0.110866f,zPlane,
		-0.084853f,-0.084853f,zPlane,
		-0.110866f,-0.045922f,zPlane,
		-0.117694f,-0.023411f,zPlane,
		-0.700000f,-0.010000f,zPlane,
	};
	{
		float prev[2],first[2];
		float c=(float)cos(tilt);
		float s=(float)sin(tilt);
		for(int i=0; i<24; ++i)
		{
			float xx=inRad*miniatureAirplaneVtx[i*3];
			float yy=inRad*miniatureAirplaneVtx[i*3+1];
			float x=cx+c*xx-s*yy;
			float y=cy+s*xx+c*yy;

			if(0==i)
			{
				first[0]=x;
				first[1]=y;
			}
			else
			{
				lineVtxBuf.Add<float>(prev[0],prev[1],zPlane);
				lineVtxBuf.Add<float>(x,y,zPlane);
				lineColBuf.Add(hudCol);
				lineColBuf.Add(hudCol);
			}

			prev[0]=x;
			prev[1]=y;
		}
		lineVtxBuf.Add<float>(prev[0],prev[1],zPlane);
		lineVtxBuf.Add<float>(first[0],first[1],zPlane);
		lineColBuf.Add(hudCol);
		lineColBuf.Add(hudCol);
	}
}

void FsHud2::DrawHSI(
	const double x0,const double y0,const double scale,
    const double magHdg,
    YSRESULT inRange,
    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
    YSBOOL isDme,const double &dme,
    YSBOOL selectedObs,
    YSBOOL showHdgBug,const double &hdgBug,YSBOOL selectedBug,YSBOOL vorInop)
{
	YsMatrix4x4 tfm;
	tfm.Translate(x0,y0,zPlane);
	tfm.Scale(scale,scale,1.0);

	FsInstrumentPanel::AddHsiVertexArray(
	    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
	    -magHdg,
	    hudCol,
	    tfm,
	    inRange,
	    vorId,tuned,isIls,toFrom,obs,lc,gs,
	    isDme,dme,
	    selectedObs,
	    showHdgBug,-hdgBug,selectedBug,vorInop);
}

void FsHud2::DrawAdf(
	    const double &x0,const double &y0,const double &rad,
	    YSRESULT inRange,
	    const char adfId[],YSBOOL tuned,const double &obs,const double &bearing,YSBOOL selected,YSBOOL inop)
{
	YsMatrix4x4 tfm;
	tfm.Translate(x0,y0,zPlane);
	tfm.Scale(rad,rad,1.0);

	FsInstrumentPanel::AddAdfVertexArray(
	    lineVtxBuf,lineColBuf,
	    triVtxBuf,triColBuf,
	    hudCol,hudCol,
	    tfm,
	    inRange,
	    adfId,tuned,obs,bearing,selected,inop);
}

void FsHud2::DrawNav(
	    const double &x0,const double &y0,const double &rad,
	    int navId,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selected,YSBOOL inop)
{
	YsMatrix4x4 tfm;
	tfm.Translate(x0,y0,zPlane);
	tfm.Scale(rad,rad,1.0);

	FsInstrumentPanel::AddNavVertexArray(
	    lineVtxBuf,lineColBuf,
	    triVtxBuf,triColBuf,
	    hudCol,hudCol,
	    tfm,

	    navId,
	    inRange,
	    vorId,tuned,isIls,toFrom,obs,lc,gs,
	    isDme,dme,
	    selected,inop);
}
