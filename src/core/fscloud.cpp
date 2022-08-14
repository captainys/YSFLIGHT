#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>


#include "fsdef.h"
#include "graphics/common/fsopengl.h"
#include "fscloud.h"





FsCloud::FsCloud()
{
	vtx.Set(0,NULL);
	col=YsGrayScale(200.0/256.0);
}

FsCloud::~FsCloud()
{
}

void FsCloud::Generate(int n,double dx,double dz,const YsVec3 &center)
{
	vtx.Set(n,NULL);

	int i;
	for(i=0; i<n; i++)
	{
		vtx[i].SetX(dx*cos(2.0*YsPi*double(i)/double(n)));
		vtx[i].SetZ(dz*sin(2.0*YsPi*double(i)/double(n)));
		vtx[i].SetY(0.0);
	}

	int r,x,*m;
	r=0;
	m=new int [n];
	if(m!=NULL)
	{
		for(i=0; i<n; i++)
		{
			r=r+(rand()%20)-10;
			m[i]=r;
		}

		x=m[n-1];
		for(i=0; i<n; i++)
		{
			m[i]=m[i]-x*i/n;
			vtx[i]=center+vtx[i]*double(m[i]+100)/100.0;
		}

		delete [] m;

		cen=center;
	}
}

// FsCloud::Draw is moved to fscloudbi.cpp

YSRESULT FsCloud::Save(FILE *fp) const
{
	int i;
	fprintf(fp,"%d\n",(int)vtx.GetN());
	fprintf(fp,"%d %d %d\n",col.Ri(),col.Gi(),col.Bi());
	for(i=0; i<vtx.GetN(); i++)
	{
		fprintf(fp,"%s\n",vtx[i].Txt());
	}
	return YSOK;
}

YSRESULT FsCloud::Load(FILE *fp)
{
	vtx.Set(0,NULL);

	char buf[256];
	int ac;
	char *av[16];

	if(fgets(buf,256,fp)!=NULL && YsArguments(&ac,av,16,buf)==YSOK && ac>=1)
	{
		int nv;
		nv=atoi(av[0]);

		int r,g,b;
		if(fgets(buf,256,fp)!=NULL && YsArguments(&ac,av,16,buf)==YSOK && ac>=3)
		{
			r=atoi(av[0]);
			g=atoi(av[1]);
			b=atoi(av[2]);
			col.SetIntRGB(r,g,b);

			if(nv>0)
			{
				cen=YsOrigin();
				vtx.Set(nv,NULL);
				int i;
				for(i=0; i<nv; i++)
				{
					double x,y,z;

					if(fgets(buf,256,fp)!=NULL && YsArguments(&ac,av,16,buf)==YSOK && ac>=3)
					{
						x=atof(av[0]);
						y=atof(av[1]);
						z=atof(av[2]);
						vtx[i].Set(x,y,z);
						cen+=vtx[i];
					}
					else
					{
						return YSERR;
					}
				}
				cen/=double(nv);

				return YSOK;
			}
			else
			{
				return YSOK;
			}
		}
	}
	return YSERR;
}

////////////////////////////////////////////////////////////

FsClouds::FsClouds()
{
	nCld=0;
	cld=NULL;
	ready=YSFALSE;
	needRemakeVbo=YSTRUE;
	res=NULL;
	vboHd=nullptr;

	CreateGraphicCache();
}

FsClouds::~FsClouds()
{
	DeleteGraphicCache();

	if(cld!=NULL)
	{
		delete [] cld;
	}
}

void FsClouds::Scatter
    (int n,const YsVec3 &center,double range,double averageSize,double ceiling)
{
	if(cld!=NULL)
	{
		delete [] cld;
		nCld=0;
		cld=NULL;
	}

	cld=new FsCloud[n];
	if(cld!=NULL)
	{
		nCld=n;

		int i;
		for(i=0; i<n; i++)
		{
			double dx,dy;
			YsVec3 cen;

			dx=averageSize*double(100+rand()%120-60)/100.0;
			dy=averageSize*double(100+rand()%120-60)/100.0;

			cen.SetX(center.x()+range*(double)(rand()%100-50)/50.0);
			cen.SetY(ceiling);
			cen.SetZ(center.z()+range*(double)(rand()%100-50)/50.0);

			cld[i].Generate(16,dx,dy,cen);
		}
	}

	ready=YSTRUE;
}

void FsClouds::MakeOpenGlList(void)
{
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==vboHd)
	{
		vboHd=bufMan.Create();
	}
	if(nullptr!=vboHd)
	{
		YsGLVertexBuffer vtxBuf;
		YsGLColorBuffer colBuf;

		for(int i=0; i<nCld; i++)
		{
			auto &cld=this->cld[i];

			for(int i=0; i<cld.vtx.GetN(); ++i)
			{
				vtxBuf.Add(cld.cen);
				vtxBuf.Add(cld.vtx[i]);
				vtxBuf.Add(cld.vtx.GetCyclic(i+1));

				colBuf.Add(cld.col);
				colBuf.Add(cld.col);
				colBuf.Add(cld.col);
			}
		}

		bufMan.MakeVtxCol(vboHd,YsGL::TRIANGLES,vtxBuf,colBuf);
	}
}

// FsClouds::Draw(void) is moved to fscloudbi.cpp and fscloudgl.cpp

YSRESULT FsClouds::Save(FILE *fp) const
{
	int i;
	fprintf(fp,"CLOUDCFG\n");
	fprintf(fp,"%d\n",nCld);
	for(i=0; i<nCld; i++)
	{
		if(cld[i].Save(fp)!=YSOK)
		{
			return YSERR;
		}
	}

	return YSOK;
}

YSRESULT FsClouds::Load(FILE *fp)
{
	// Keyword "CLOUD" is already read by FsWorld

	if(cld!=NULL)
	{
		delete [] cld;
		nCld=0;
		cld=NULL;
	}


	char buf[256];
	int ac;
	char *av[16];
	if(fgets(buf,256,fp)!=NULL && YsArguments(&ac,av,16,buf)==YSOK)
	{
		nCld=atoi(av[0]);
		if(nCld>0)
		{
			int i;

			cld=new FsCloud[nCld];
			for(i=0; i<nCld; i++)
			{
				if(cld[i].Load(fp)!=YSOK)
				{
					return YSERR;
				}
			}

			ready=YSTRUE;
			return YSOK;
		}
		else
		{
			ready=YSTRUE;
			return YSOK;
		}
	}

	return YSERR;
}

YSBOOL FsClouds::IsReady(void) const
{
	return ready;
}


////////////////////////////////////////////////////////////

static const int sphere_e2_nVtx=75;
static const int sphere_e2_nPlg=146;
static double sphere_e2_vtx[]=
{
        4.359678,-2.095299,1.131486,
        3.357309,-2.118750,2.983472,
        2.386277,-2.777851,-0.890860,
        4.907726,-0.666873,-0.269996,
        4.517337,-0.342579,2.033129,
        3.929735,1.621881,2.586884,
        3.240900,0.160793,3.756945,
        4.773269,1.216009,0.585685,
        4.180511,2.590823,-0.779813,
        3.738126,3.068650,1.111539,
        4.589577,0.890126,-1.711163,
        3.515962,0.554612,-3.473692,
        3.324372,2.738115,-2.504270,
        4.221942,-1.176400,-2.325491,
        2.689270,-1.184351,-4.024777,
        0.686109,-1.454210,-4.691121,
        1.270945,0.352172,-4.778217,
        1.243159,-2.777851,-3.873582,
        -0.676644,-2.777851,-2.116688,
        -0.912153,-2.670182,-4.106650,
        3.079667,-2.777851,-2.769125,
        -2.494758,-1.462291,-4.033606,
        -0.950454,-0.467218,-4.860371,
        -2.688419,-2.777851,-2.599467,
        -4.138180,-2.516507,-1.089813,
        -4.043223,-1.062722,-2.724672,
        -1.933202,-2.777851,-0.519969,
        -1.182780,-2.777851,1.497220,
        -3.314984,-2.777851,1.054495,
        0.365172,-2.777851,-0.044108,
        1.005813,-2.777851,2.091575,
        1.464219,-2.777851,3.879332,
        -0.587919,-2.777851,3.470737,
        -1.736742,-1.544569,4.393539,
        -2.795456,-2.687634,3.120140,
        0.154998,-1.291462,4.792801,
        1.052005,0.556613,4.823691,
        -1.166179,0.443926,4.800599,
        -2.507266,1.899839,3.877144,
        -3.021387,-0.319034,3.917030,
        -0.245331,2.269526,4.404890,
        0.727294,3.698309,3.249941,
        -1.512366,3.546674,3.165902,
        2.066116,2.014083,4.047579,
        -2.270280,4.187011,1.499881,
        -3.721586,2.707913,1.893069,
        -0.253268,4.688367,1.661112,
        0.824570,4.915536,-0.221769,
        -1.312956,4.794173,-0.246752,
        1.863192,4.380724,1.431783,
        -2.141767,4.022891,-2.004930,
        -3.294145,3.726082,-0.093464,
        -0.126956,4.545225,-2.039680,
        1.237119,3.460902,-3.365253,
        -0.872736,3.349032,-3.571266,
        2.868481,4.050735,-0.398803,
        -2.685692,2.390156,-3.414926,
        -4.209786,1.045963,-2.431766,
        -4.883505,-0.211400,-0.971390,
        -4.582434,1.937659,-0.243737,
        -2.914424,0.423074,-3.991620,
        -1.204483,1.432938,-4.590214,
        -4.803446,0.548124,1.121611,
        -4.161864,-1.191477,2.428236,
        2.555590,-2.777851,0.981784,
        -4.705331,-1.456198,0.535718,
        -3.708639,2.809302,-1.783880,
        1.895528,4.209286,-1.895528,
        -4.002698,0.951853,2.819227,
        1.079476,-2.777851,-2.119095,
        0.464368,2.083229,-4.482895,
        2.328489,1.900033,-3.972636,
        2.100251,-1.172149,4.343791,
        2.660123,3.201804,2.721193,
        4.167693,-2.610127,-0.805191
};
static int sphere_e2_idx[]=
{
        69,20,2,
        69,2,29,
        18,69,29,
        23,18,26,
        28,26,27,
        32,27,30,
        64,30,29,
        2,64,29,
        29,30,27,
        27,26,29,
        26,18,29,
        30,31,32,
        69,17,20,
        32,31,35,
        74,13,3,
        6,1,4,
        57,25,58,
        68,45,62,
        7,8,9,
        62,45,59,
        8,10,12,
        9,8,55,
        8,12,55,
        56,50,54,
        50,52,54,
        72,35,31,
        30,64,1,
        31,30,1,
        18,23,19,
        19,23,21,
        21,22,19,
        4,1,0,
        4,0,3,
        6,43,36,
        40,36,43,
        43,41,40,
        60,57,56,
        66,56,57,
        56,61,60,
        71,53,12,
        12,10,11,
        12,11,71,
        4,5,6,
        43,6,5,
        41,43,73,
        9,73,5,
        73,43,5,
        45,68,38,
        38,68,39,
        54,52,53,
        56,54,61,
        70,61,54,
        71,70,53,
        52,67,53,
        53,70,54,
        9,55,49,
        73,9,49,
        47,49,55,
        48,50,51,
        3,0,74,
        0,1,64,
        32,35,33,
        32,33,34,
        27,32,34,
        27,34,28,
        24,26,28,
        65,24,28,
        23,26,24,
        25,23,24,
        15,17,19,
        18,19,17,
        69,18,17,
        64,2,74,
        0,64,74,
        34,63,28,
        65,28,63,
        15,14,17,
        20,17,14,
        2,20,74,
        13,74,20,
        31,1,72,
        1,6,72,
        34,33,39,
        63,34,39,
        68,63,39,
        62,58,65,
        58,25,24,
        65,58,24,
        57,60,25,
        23,25,21,
        60,21,25,
        16,15,22,
        19,22,15,
        13,20,14,
        11,13,14,
        37,35,36,
        6,36,72,
        35,72,36,
        38,39,37,
        35,37,33,
        39,33,37,
        68,62,63,
        65,63,62,
        22,21,60,
        61,22,60,
        14,15,16,
        11,14,16,
        71,11,16,
        11,10,13,
        13,10,3,
        4,3,7,
        5,4,7,
        9,5,7,
        36,40,37,
        57,58,59,
        62,59,58,
        16,22,61,
        71,16,70,
        61,70,16,
        3,10,7,
        8,7,10,
        37,40,38,
        57,59,66,
        40,41,42,
        38,40,42,
        46,42,41,
        45,38,42,
        44,45,42,
        46,44,42,
        51,59,45,
        59,51,66,
        50,56,66,
        51,50,66,
        41,73,49,
        46,41,49,
        45,44,51,
        48,51,44,
        53,67,12,
        55,12,67,
        49,47,46,
        44,46,48,
        46,47,48,
        50,48,52,
        48,47,52,
        47,55,67,
        52,47,67
};
static double sphere_e2_nom[]=
{
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        0.000000,-1.000000,0.000000,
        -0.154368,-0.612435,0.775302,
        0.927640,-0.287667,-0.238184,
        0.767423,-0.170521,0.618049,
        -0.903982,-0.012037,-0.427400,
        -0.883736,0.321050,0.340497,
        0.880012,0.466743,0.087917,
        -0.900566,0.369025,0.229786,
        0.831506,0.407032,-0.378051,
        0.744470,0.667634,0.005461,
        0.677240,0.681203,-0.278044,
        -0.408167,0.670757,-0.619262,
        -0.221205,0.817925,-0.531100,
        0.230916,-0.353656,0.906424,
        0.174785,-0.953874,0.244080,
        0.295669,-0.952277,-0.075814,
        0.013342,-0.998364,-0.055597,
        -0.492291,-0.607302,-0.623566,
        -0.282202,-0.314948,-0.906184,
        0.838621,-0.306931,0.450009,
        0.964465,-0.184195,0.189417,
        0.454980,0.150775,0.877645,
        0.181302,0.360977,0.914782,
        0.186528,0.537687,0.822253,
        -0.681624,0.277722,-0.676948,
        -0.721534,0.410046,-0.557899,
        -0.467489,0.298320,-0.832141,
        0.474002,0.586866,-0.656438,
        0.785577,0.307358,-0.537029,
        0.670721,0.349340,-0.654289,
        0.812114,0.079989,0.577990,
        0.627780,0.284310,0.724611,
        0.351107,0.613923,0.706981,
        0.718980,0.540635,0.436786,
        0.610515,0.434301,0.662310,
        -0.686465,0.422154,0.592074,
        -0.635954,0.160920,0.754763,
        0.019114,0.783434,-0.621182,
        -0.327497,0.487208,-0.809552,
        -0.132624,0.483178,-0.865419,
        0.273443,0.508834,-0.816282,
        0.176248,0.843004,-0.508213,
        0.044358,0.614266,-0.787851,
        0.584425,0.791630,0.178242,
        0.578793,0.747572,0.325784,
        0.392861,0.918225,0.050221,
        -0.478092,0.864711,-0.153960,
        0.934332,-0.356398,0.002120,
        0.336297,-0.926232,0.170289,
        -0.075315,-0.641428,0.763477,
        -0.142588,-0.674698,0.724190,
        -0.042879,-0.998997,0.012925,
        -0.009558,-0.998894,0.046031,
        -0.095824,-0.991839,-0.084097,
        -0.712813,-0.674728,0.191413,
        -0.129553,-0.990456,0.047050,
        -0.647830,-0.550192,-0.526882,
        0.067998,-0.503404,-0.861371,
        -0.044579,-0.997818,-0.048712,
        0.000000,-1.000000,0.000000,
        0.094140,-0.995523,-0.008512,
        0.330414,-0.919776,0.211750,
        -0.768813,-0.600588,0.219594,
        -0.713873,-0.635612,0.293908,
        0.337558,-0.387721,-0.857745,
        0.451764,-0.481255,-0.751201,
        0.092063,-0.995173,0.033987,
        0.794599,-0.455599,-0.401301,
        0.503403,-0.418122,0.756148,
        0.636140,-0.218591,0.739962,
        -0.575923,-0.309502,0.756651,
        -0.667918,-0.293768,0.683802,
        -0.776181,-0.056909,0.627936,
        -0.995801,-0.066986,0.062399,
        -0.911698,-0.279305,-0.301323,
        -0.944809,-0.298561,-0.134899,
        -0.763186,0.029381,-0.645511,
        -0.622139,-0.533280,-0.573198,
        -0.659459,-0.130302,-0.740361,
        0.061932,-0.068062,-0.995757,
        -0.097885,-0.323685,-0.941088,
        0.702743,-0.326085,-0.632314,
        0.735622,-0.139653,-0.662841,
        -0.009802,-0.011955,0.999880,
        0.440654,0.018051,0.897496,
        0.230507,-0.128005,0.964615,
        -0.467810,0.124123,0.875070,
        -0.185606,-0.145672,0.971766,
        -0.405200,-0.070330,0.911519,
        -0.903355,-0.011129,0.428750,
        -0.947805,-0.131328,0.290547,
        -0.432029,-0.076154,-0.898639,
        -0.371959,0.081736,-0.924644,
        0.330340,-0.151867,-0.931564,
        0.499593,0.036771,-0.865479,
        0.491486,0.113581,-0.863447,
        0.853785,0.002878,-0.520617,
        0.951337,-0.080845,-0.297358,
        0.985782,-0.005920,0.167928,
        0.919387,0.175814,0.351877,
        0.869066,0.404913,0.284201,
        -0.021431,0.222101,0.974788,
        -0.943973,0.216564,-0.249027,
        -0.989557,0.143440,-0.014201,
        -0.014590,0.138834,-0.990208,
        0.279037,0.286387,-0.916581,
        -0.001811,0.167352,-0.985896,
        0.988915,0.113905,-0.095250,
        0.949012,0.292560,-0.117407,
        -0.265317,0.330179,0.905864,
        -0.884025,0.360549,-0.297496,
        -0.072618,0.656416,0.750896,
        -0.273711,0.515929,0.811726,
        -0.076855,0.824218,0.561033,
        -0.559480,0.589592,0.582549,
        -0.545077,0.669118,0.505145,
        -0.249400,0.860516,0.444198,
        -0.810311,0.573652,0.119663,
        -0.800449,0.586854,-0.122002,
        -0.567675,0.638206,-0.520036,
        -0.614369,0.746875,-0.254415,
        0.342237,0.789154,0.510009,
        0.175693,0.880517,0.440252,
        -0.665873,0.712327,0.221818,
        -0.471124,0.880763,0.047948,
        0.467619,0.685219,-0.558398,
        0.610115,0.724897,-0.319819,
        0.162850,0.964142,0.209546,
        -0.251166,0.948687,0.192118,
        -0.057485,0.994541,0.087084,
        -0.244827,0.925057,-0.290394,
        -0.053833,0.983593,-0.172182,
        0.374664,0.915502,-0.146571,
        0.176784,0.942245,-0.284469
};

FsSolidCloud::FsSolidCloud() : ltc(1)
{
	ltc.DisablePolygonToCellHashTable();
	CreateGraphicCache();

}

FsSolidCloud::~FsSolidCloud()
{
	DeleteGraphicCache();
}

void FsSolidCloud::Initialize(void)
{
	particle.Clear();
	shl.CleanUp();
}

const YsShell &FsSolidCloud::GetShell(void) const
{
	return shl;
}

const YsVec3 &FsSolidCloud::GetCenter(void) const
{
	return cen;
}

void FsSolidCloud::Make(const YsVec3 &mov,const double &sizeX,const double &sizeZ,const double &y0,const double &y1)
{
	YsArray <YsShellVertexHandle> vtHdList;
	YsShellPolygonHandle plHd;
	YsShellVertexHandle vtHd;
	YsVec3 diagon;
	double bump;

	Initialize();

	for(int i=0; i<sphere_e2_nVtx; i++)
	{
		YsVec3 pos;
		pos.Set(sphere_e2_vtx[i*3],sphere_e2_vtx[i*3+1],sphere_e2_vtx[i*3+2]);
		vtHd=shl.AddVertexH(pos);
		vtHdList.Append(vtHd);
	}

	for(int i=0; i<sphere_e2_nPlg; i++)
	{
		YsVec3 plNom;
		YsShellPolygonHandle plHd;
		YsShellVertexHandle plVtHd[3];
		plNom.Set(sphere_e2_nom[i*3],sphere_e2_nom[i*3+1],sphere_e2_nom[i*3+2]);
		plVtHd[0]=vtHdList[sphere_e2_idx[i*3]];
		plVtHd[1]=vtHdList[sphere_e2_idx[i*3+1]];
		plVtHd[2]=vtHdList[sphere_e2_idx[i*3+2]];
		plHd=shl.AddPolygonH(3,plVtHd);
		shl.SetNormalOfPolygon(plHd,plNom);
	}


	shl.GetBoundingBox(bbx[0],bbx[1]);
	diagon=bbx[1]-bbx[0];
	bump=diagon.GetLength()/20.0;

	plHd=NULL;
	while((plHd=shl.FindNextPolygon(plHd))!=NULL)
	{
		if(rand()%100<50)
		{
			int i,nPlVt;
			const YsShellVertexHandle *plVtHd;
			double t;
			YsVec3 nom,pos;
			shl.GetNormalOfPolygon(nom,plHd);
			shl.GetVertexListOfPolygon(nPlVt,plVtHd,plHd);
			t=(double)(rand()%100-50)/50.0;
			for(i=0; i<nPlVt; i++)
			{
				shl.GetVertexPosition(pos,plVtHd[i]);
				pos+=t*nom;
				shl.ModifyVertexPosition(plVtHd[i],pos);
			}
		}
	}
	shl.AutoComputeNormalAll(YSTRUE,YSTRUE);



	double scaleX,scaleY,scaleZ,offsetY;
	shl.GetBoundingBox(bbx[0],bbx[1]);
	diagon=bbx[1]-bbx[0];
	scaleX=sizeX/diagon.x();
	scaleY=(y1-y0)/diagon.y();
	scaleZ=sizeZ/diagon.z();
	offsetY=-bbx[0].y();

	vtHd=NULL;
	while((vtHd=shl.FindNextVertex(vtHd))!=NULL)
	{
		YsVec3 pos;
		shl.GetVertexPosition(pos,vtHd);
		pos.AddY(offsetY);
		pos.MulX(scaleX);
		pos.MulY(scaleY);
		pos.MulZ(scaleZ);

		pos.AddX(mov.x());
		pos.AddY(y0);
		pos.AddZ(mov.z());

		shl.ModifyVertexPosition(vtHd,pos);
	}



	shl.AutoComputeNormalAll(YSTRUE,YSTRUE);
	shl.AutoComputeVertexNormalAll(YSTRUE);
	ltc.SetDomain(shl,1024);



	shl.GetBoundingBox(bbx[0],bbx[1]);
	cen=(bbx[0]+bbx[1])/2.0;

	ScatterParticle(400);
}

void FsSolidClouds::MakeOpenGlList(void)
{
	int i;
	YsListItem <FsSolidCloud> *itm;
	cloudContainer.RewindPointer();
	i=0;
	while(itm=cloudContainer.StepPointer())
	{
		itm->dat.MakeOpenGlList();
		i++;
	}
}

void FsSolidCloud::MakeOpenGlList(void)
{
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==vboHd)
	{
		vboHd=bufMan.Create();
	}
	if(nullptr!=vboHd)
	{
		YsGLVertexBuffer vtxBuf;
		YsGLColorBuffer colBuf;
		YsGLNormalBuffer nomBuf;

		YsColor col;
		col.SetIntRGBA(255,255,255,220);

		for(auto plHd : shl.AllPolygon())
		{
			int i,nPlVt;
			const YsShellVertexHandle *plVtHd;
			YsVec3 nom,pos;
			shl.GetVertexListOfPolygon(nPlVt,plVtHd,plHd);

			for(i=nPlVt-1; i>=0; i--)
			{
				nom=shl.GetVertex(plVtHd[i])->GetNormal();
				shl.GetVertexPosition(pos,plVtHd[i]);
				vtxBuf.Add(pos);
				nomBuf.Add(pos);
				colBuf.Add(col);
			}
		}

		bufMan.MakeVtxNomCol(vboHd,YsGL::TRIANGLES,vtxBuf,nomBuf,colBuf);
	}
}


YSBOOL FsSolidCloud::IsInCloud(const YsVec3 &pos) const
{
	if(YsCheckInsideBoundingBox3(pos,bbx[0],bbx[1])==YSTRUE)
	{
		if(ltc.CheckInsideSolid(pos)==YSINSIDE)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

void FsSolidCloud::ScatterParticle(int nParticle)
{
	particle.Set(nParticle,NULL);
	if(0<nParticle)
	{
		const double cloudVol=shl.ComputeVolume();
		const double particleVol=cloudVol/(double)nParticle;
		const double particleRad=pow(particleVol*0.75/YsPi,1.0/3.0)*3.0; // consider 3.0 overlap
		// Sphere Volume = (4/3)*PI*(r^3)

		const YsVec3 d=bbx[1]-bbx[0];

		int n=0;
		for(int i=0; i<nParticle*100 && n<nParticle; ++i)
		{
			double x=(double)rand()/(double)RAND_MAX;
			double y=(double)rand()/(double)RAND_MAX;
			double z=(double)rand()/(double)RAND_MAX;

			x*=d.x();
			y*=d.y();
			z*=d.z();

			YsVec3 tst=bbx[0]+YsVec3(x,y,z);
			if(YSINSIDE==ltc.CheckInsideSolid(tst))
			{
				if(tst.y()-particleRad<bbx[0].y()-100.0)
				{
					tst.SetY(bbx[0].y()-100.0+particleRad);
				}
				particle[n].pos=tst;
				particle[n].rad=particleRad;
				particle[n].particleType=rand()&7;
				particle[n].colorCorrection=0.1*((double)rand()/(double)RAND_MAX)-0.05;
				++n;
			}
		}

		particle.Resize(n);

		printf("%d particles / cloud.\n",n);
	}
}

FsSolidClouds::FsSolidClouds() : cloudAllocator(4),cloudContainer(cloudAllocator)
{
}

FsSolidClouds::~FsSolidClouds()
{
	cloudContainer.CleanUp();
}

YSBOOL FsSolidClouds::IsReady(void) const
{
	if(cloudContainer.GetN()>0)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSolidClouds::IsInCloud(const YsVec3 &pos) const
{
	YsListItem <FsSolidCloud> *itm;
	cloudContainer.RewindPointer();
	while(NULL!=(itm=cloudContainer.StepPointer()))
	{
		if(itm->dat.IsInCloud(pos)==YSTRUE)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSRESULT FsSolidClouds::Save(FILE *fp) const
{
	fprintf(fp,"SLDCLOUD\n");
	fprintf(fp,"0 %d\n",cloudContainer.GetN());

	YsListItem <FsSolidCloud> *ptr;
	cloudContainer.RewindPointer();
	while(NULL!=(ptr=cloudContainer.StepPointer()))
	{
		const YsShell &shl=ptr->dat.GetShell();

		shl.Encache();
		fprintf(fp,"Surf\n");

		YsShellVertexHandle vtHd=NULL;
		while(NULL!=(vtHd=shl.FindNextVertex(vtHd)))
		{
			YsVec3 pos;
			shl.GetVertexPosition(pos,vtHd);
			fprintf(fp,"V %.2lf %.2lf %.2lf R\n",pos.x(),pos.y(),pos.z());
		}

		YsShellPolygonHandle plHd=NULL;
		while(NULL!=(plHd=shl.FindNextPolygon(plHd)))
		{
			int nPlVt;
			const YsShellVertexHandle *plVtHd;
			YsVec3 nom;
			shl.GetVertexListOfPolygon(nPlVt,plVtHd,plHd);
			shl.GetNormalOfPolygon(nom,plHd);

			fprintf(fp,"F\n");
			for(int i=0; i<nPlVt; i++)
			{
				if(i%16==0)
				{
					fprintf(fp,"V");
				}
				fprintf(fp," %d",shl.GetVertexIdFromHandle(plVtHd[i]));
				if(i%16==15 || i==nPlVt-1)
				{
					fprintf(fp,"\n");
				}
			}
			fprintf(fp,"N %lf %lf %lf\n",nom.x(),nom.y(),nom.z());
			fprintf(fp,"E\n");
		}

		fprintf(fp,"E\n");
	}
	return YSOK;
}

YSRESULT FsSolidClouds::Load(FILE *fp)
{
	// Keyword "SOLIDCLOUD" is already read.
	YsString str;
	YsArray <YsString,16> args;

	str.Fgets(fp);
	str.Arguments(args);

	int ver,nCloud;

	if(args.GetN()>=2)
	{
		ver=atoi(args[0]);
		nCloud=atoi(args[1]);
	}
	else
	{
		return YSERR;
	}

	switch(ver)
	{
	case 0:
		{
			int i;
			cloudContainer.CleanUp();
			for(i=0; i<nCloud; i++)
			{
				int state;
				state=0; // 0:Outside  1:Inside

				YsListItem <FsSolidCloud> *itm;
				itm=cloudContainer.Create();
				itm->dat.Initialize();
				itm->dat.shl.BeginReadSrf();
				while(str.Fgets(fp)!=NULL)
				{
					if(state==0)
					{
						if(str[0]=='F' || str[0]=='f')
						{
							state=1;
						}
						else if(str[0]=='E' || str[0]=='e')
						{
							break;
						}
					}
					else
					{
						if(str[0]=='E' || str[0]=='e')
						{
							state=0;
						}
					}
					itm->dat.shl.ReadSrfOneLine(str);
				}
				itm->dat.shl.EndReadSrf();

				itm->dat.shl.GetBoundingBox(itm->dat.bbx[0],itm->dat.bbx[1]);
				itm->dat.cen=(itm->dat.bbx[0]+itm->dat.bbx[1])/2.0;

				itm->dat.shl.AutoComputeVertexNormalAll(YSTRUE);
				itm->dat.ltc.SetDomain(itm->dat.shl,1024);

				itm->dat.ScatterParticle(400);
			}
		}
		break;
	}

	return YSOK;
}

void FsSolidClouds::Make(
    int n,const YsVec3 &cen,const double &range,const double &sizeH,const double &y0,const double &y1)
{
	for(int i=0; i<n; i++)
	{
		YsVec3 mov;
		double sizeX,sizeZ,dx,dz;
		YsListItem <FsSolidCloud> *newCloud;

		sizeX=sizeH*(1.0+(double)(rand()%25-50)/100.0);
		sizeZ=sizeH*(1.0+(double)(rand()%25-50)/100.0);

		dx=range*(double)(rand()%100-50)/100.0;
		dz=range*(double)(rand()%100-50)/100.0;

		mov=cen;
		mov.AddX(dx);
		mov.SetY(0.0);
		mov.AddZ(dz);

		newCloud=cloudContainer.Create();
		newCloud->dat.Make(mov,sizeX,sizeZ,y0,y1);
	}
}

void FsSolidClouds::AddToParticleManager(
	class YsGLParticleManager &partMan,
	FSENVIRONMENT env,const class FsWeather &weather,
	const YsVec3 &viewDir,const YsMatrix4x4 &viewMdlTfm,const double &nearZ,const double &farZ,const double &tanFov)
{
	const double baseBrightness=(FSDAYLIGHT==env ? 0.7 : 0.15);

	YsListItem <FsSolidCloud> *itm=NULL;
	while((itm=cloudContainer.FindNext(itm))!=NULL)
	{
		if(YsIsBoundingBoxVisible(itm->dat.bbx,viewMdlTfm,nearZ,farZ,tanFov)==YSTRUE)
		{
			for(auto &particle : itm->dat.particle)
			{
				double brightness=baseBrightness+particle.colorCorrection;

 				YsColor col;
				col.SetDoubleRGBA(brightness,brightness,brightness,0.5);

				float s=(float)particle.particleType*0.125f;
				partMan.Add(particle.pos,col,particle.rad*2.0,s,0);
			}
		}
	}
}

void FsSolidClouds::Draw(
    FSENVIRONMENT env,const FsWeather &weather,
    const YsMatrix4x4 &viewMdlTfm,const double &nearZ,const double &farZ,const double &tanFov)
{
	YsArray <double,32> dist;
	YsArray <FsSolidCloud *,32> toDraw;

	BeginDrawCloud();

	YsListItem <FsSolidCloud> *itm;
	itm=NULL;
	while((itm=cloudContainer.FindNext(itm))!=NULL)
	{
		if(YsIsBoundingBoxVisible(itm->dat.bbx,viewMdlTfm,nearZ,farZ,tanFov)==YSTRUE)
		{
			toDraw.Append(&itm->dat);
		}
		else
		{
			static int a=0;
		}
	}

	for(int i=0; i<toDraw.GetN(); ++i)
	{
		toDraw[i]->Draw(env,weather);
	}

	EndDrawCloud();
}

