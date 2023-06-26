#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <errno.h>

#include <ysclass.h>
#include <ysport.h>
#include <ysbase64.h>
#include <yseditarray.h>
#include <ysunitconv.h>
#include "ysscenery.h"


extern YSRESULT FsGetLength(double &dat,const char in[]);
extern YSRESULT FsGetArea(double &dat,const char in[]);
extern YSRESULT FsGetWeight(double &dat,const char in[]);
extern YSRESULT FsGetForce(double &dat,const char in[]);
extern YSRESULT FsGetSpeed(double &dat,const char in[]);
extern YSRESULT FsGetAngle(double &dat,const char in[]);
extern YSRESULT FsGetBool(YSBOOL &dat,const char in[]);
extern YSRESULT FsGetVec3(YsVec3 &vec,int ac,char * const av[]);
extern YSRESULT FsGetAtt3(YsAtt3 &att,int ac,char * const av[]);


/*
Pict2                           At the top of file

PST                             psets
DST d                           maximum visible distance
COL r g b
VER x1 y1
VER x2 y2
    :
ENDO

PLL                             Polyline of n vertex
DST d                           maximum visible distance
COL r g b
VER x1 y1
VER x2 y2
    :
ENDO

LSQ                             Line Sequence of n vertex
DST d                           maximum visible distance
COL r g b
VER x1 y1
VER x2 y2
    :
ENDO

PLG                             Polygon of n vertex
DST d                           maximum visible distance
COL r g b
VER x1 y1
VER x2 y2
    :
ENDO

ENDPICT                         Termination Code
*/

YSRESULT Ys2DDrawing::BeginLoadPc2(void)
{
	loadingState=0;
	loadingElem=NULL;
	DeleteCache();
	return YSOK;
}

const char *const Ys2DDrawing::state0KeyWordSource[]=
{
	"PICT2",    // 0
	"PST",      // 1
	"PLL",      // 2
	"LSQ",      // 3
	"PLG",      // 4
	"APL",      // 5
	"GQS",      // 6
	"QST",      // 7
	"QDR",      // 8
	"TRI",      // 9
	"ENDPICT",  //10
	NULL
};

YsKeyWordList Ys2DDrawing::state0KeyWordList;

const char *const Ys2DDrawing::state1KeyWordSource[]=
{
	"PICT2",   // 0
	"DST",     // 1
	"COL",     // 2
	"CL2",     // 3
	"VER",     // 4
	"ENDO",    // 5
	"TXL",     // 6
	"TXC",     // 7
	"SPEC",    // 8 Specular reflection flag
	NULL
};
YsKeyWordList Ys2DDrawing::state1KeyWordList;

YSRESULT Ys2DDrawing::LoadPc2OneLine(const char cmd[])
{
	YsString buf(cmd);
	YsArray <YsString,16> args;
	buf.Arguments(args);

	if(0<args.GetN())
	{
		args[0].Capitalize();

		if(strcmp(args[0],"PICT2")==0)
		{
			return YSOK;
		}
		else if(loadingState==0)
		{
			if(state0KeyWordList.GetN()==0)
			{
				state0KeyWordList.MakeList(state0KeyWordSource);
			}

			int cmd;
			cmd=state0KeyWordList.GetId(args[0]);
			switch(cmd)
			{
			case 0: // "PICT2",    // 0
				// Ignore
				break;
			case 1: // "PST",      // 1  if(strcmp(args[0],"PST")==0)
				loadingElem=CreateElement(Ys2DDrawingElement::POINTS);
				loadingState=1;
				break;
			case 2: // "PLL",      // 2  else if(strcmp(args[0],"PLL")==0)
				loadingElem=CreateElement(Ys2DDrawingElement::LINESEGMENTS);
				loadingState=1;
				break;
			case 3: // "LSQ",      // 3  else if(strcmp(args[0],"LSQ")==0)
				loadingElem=CreateElement(Ys2DDrawingElement::LINES);
				loadingState=1;
				break;
			case 4: // "PLG",      // 4  else if(strcmp(args[0],"PLG")==0)
				loadingElem=CreateElement(Ys2DDrawingElement::POLYGON);
				loadingState=1;
				break;
			case 5: // "APL",      // 5  else if(strcmp(args[0],"APL")==0)  // 2005/01/09
				loadingElem=CreateElement(Ys2DDrawingElement::APPROACHLIGHT);
				loadingState=1;
				break;
			case 6: // "GQS",      // 6  else if(strcmp(args[0],"GQS")==0) // 2005/05/08 Gradation Quadrilateral Strip
				loadingElem=CreateElement(Ys2DDrawingElement::GRADATIONQUADSTRIP);
				loadingState=1;
				break;
			case 7: // "QST",      // 7  else if(strcmp(args[0],"QST")==0) // 2005/05/08 Quad Strip
				loadingElem=CreateElement(Ys2DDrawingElement::QUADSTRIP);
				loadingState=1;
				break;
			case 8: // "QDR",      // 8  else if(strcmp(args[0],"QDR")==0) // 2005/05/08 Quadrilaterals
				loadingElem=CreateElement(Ys2DDrawingElement::QUADS);
				loadingState=1;
				break;
			case 9: // "TRI",      // 9  else if(strcmp(args[0],"TRI")==0) // 2005/05/08 Triangles
				loadingElem=CreateElement(Ys2DDrawingElement::TRIANGLES);
				loadingState=1;
				break;
			case 10: // "ENDPICT",  //10  else if(strcmp(args[0],"ENDPICT")==0)
				loadingState=2;
				break;
			}
		}
		else if(loadingState==1)
		{
			if(state1KeyWordList.GetN()==0)
			{
				state1KeyWordList.MakeList(state1KeyWordSource);
			}

			int cmd;
			cmd=state1KeyWordList.GetId(args[0]);
			switch(cmd)
			{
			case 0: // "PICT2",   // 0
				// Ignore
				break;
			case 1: // "DST",     // 1  if(strcmp(args[0],"DST")==0)
				loadingElem->dat.visibleDist=atof(args[1]);
				break;
			case 2: // "COL",     // 2  else if(strcmp(args[0],"COL")==0)
				loadingElem->dat.c.SetIntRGB(atoi(args[1]),atoi(args[2]),atoi(args[3]));
				break;
			case 3: // "CL2",     // 3  else if(strcmp(args[0],"CL2")==0)
				loadingElem->dat.c2.SetIntRGB(atoi(args[1]),atoi(args[2]),atoi(args[3]));
				break;
			case 4: // "VER",     // 4  else if(strcmp(args[0],"VER")==0)
				{
					YsVec2 p;
					p.Set(atof(args[1]),atof(args[2]));
					loadingElem->dat.pnt.Append(p);
				}
				break;
			case 5: // "ENDO",    // 5  else if(strcmp(args[0],"ENDO")==0)
				loadingElem->dat.cvx=YsCheckConvexByAngle2(loadingElem->dat.pnt.GetN(),loadingElem->dat.pnt);
				loadingState=0;
				loadingElem=NULL;
				break;
			case 6: // "TXL",     // 6
				loadingElem->dat.texLabel=args[1];
				break;
			case 7: // "TXC",     // 7
				loadingElem->dat.texCoord.Append(YsVec2(args[1],args[2]));
				break;
			case 8: //	"SPEC",    // 8 Specular reflection flag
				if(2<=args.GetN())
				{
					loadingElem->dat.SetSpecular(YsStrToBool(args[1]));
				}
				break;
			}
		}
	}
	return YSOK;
}

YSRESULT Ys2DDrawing::EndLoadPc2(void)
{
	RecomputeBoundingBox();

	TriangulateAll();

	loadingState=0;
	loadingElem=NULL;
	return YSOK;
}

YSRESULT Ys2DDrawing::LoadPc2(const char fn[])
{
	FILE *fp=fopen(fn,"r");
	if(fp!=NULL)
	{
		YsTextFileInputStream inStream(fp);
		YSRESULT res=LoadPc2(inStream);
		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT Ys2DDrawing::LoadPc2(YsTextInputStream &inStream)
{
	BeginLoadPc2();

	YsString buf;
	while(NULL!=inStream.Gets(buf) && 2!=loadingState)
	{
		if(YSOK!=LoadPc2OneLine(buf))
		{
			return YSERR;
		}
	}

	EndLoadPc2();
	return YSOK;
}

YSRESULT Ys2DDrawing::LoadPc2(const YsTextFile &txtFile)
{
	const YsListItem <YsString> *str;
	BeginLoadPc2();
	str=NULL;
	while((str=txtFile.GetText().FindNext(str))!=NULL)
	{
		if(LoadPc2OneLine(str->dat)!=YSOK)
		{
			return YSERR;
		}
	}
	EndLoadPc2();
	return YSOK;
}

YSRESULT Ys2DDrawing::SavePc2(const char fn[]) const
{
	FILE *fp=fopen(fn,"w");
	if(NULL!=fp)
	{
		YsTextFileOutputStream textOut(fp);
		YSRESULT res=SavePc2(textOut);
		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT Ys2DDrawing::SavePc2(YsTextOutputStream &textOut) const
{
	textOut.Printf("Pict2\n");

	YsListItem <Ys2DDrawingElement> *itm;
	itm=NULL;
	while((itm=elemList.FindNext(itm))!=NULL)
	{
		switch(itm->dat.t)
		{
		case Ys2DDrawingElement::POINTS:
			textOut.Printf("PST\n");
			break;
		case Ys2DDrawingElement::LINESEGMENTS:
			textOut.Printf("PLL\n");
			break;
		case Ys2DDrawingElement::LINES:
			textOut.Printf("LSQ\n");
			break;
		case Ys2DDrawingElement::POLYGON:
			textOut.Printf("PLG\n");
			break;
		case Ys2DDrawingElement::APPROACHLIGHT:
			textOut.Printf("APL\n");
			break;
		case Ys2DDrawingElement::GRADATIONQUADSTRIP:
			textOut.Printf("GQS\n");
			break;
		case Ys2DDrawingElement::QUADSTRIP:
			textOut.Printf("QST\n");
			break;
		case Ys2DDrawingElement::QUADS:
			textOut.Printf("QDR\n");
			break;
		case Ys2DDrawingElement::TRIANGLES:
			textOut.Printf("TRI\n");
			break;
		default:
			YsPrintf("Unknown type object is included.\n");
			goto UNKNOWNTYPE;
		}

		if(YsTolerance<itm->dat.visibleDist)
		{
			textOut.Printf("DST %.2lf\n",itm->dat.visibleDist);
		}

		int r,g,b;
		itm->dat.c.GetIntRGB(r,g,b);
		textOut.Printf("COL %d %d %d\n",r,g,b);

		if(itm->dat.t==Ys2DDrawingElement::GRADATIONQUADSTRIP)
		{
			int r,g,b;
			itm->dat.c2.GetIntRGB(r,g,b);
			textOut.Printf("CL2 %d %d %d\n",r,g,b);
		}

		for(int i=0; i<itm->dat.pnt.GetN(); i++)
		{
			textOut.Printf("VER %.2lf %.2lf\n",itm->dat.pnt[i].x(),itm->dat.pnt[i].y());
		}

		if(0<itm->dat.texLabel.Strlen())
		{
			textOut.Printf("TXL \"%s\"\n",itm->dat.texLabel.Txt());
		}

		for(auto &coord : itm->dat.texCoord)
		{
			textOut.Printf("TXC %.4lf %.4lf\n",coord.x(),coord.y());
		}

		textOut.Printf("SPEC %s\n",YsBoolToStr(itm->dat.GetSpecular()));

		textOut.Printf("ENDO\n");

	UNKNOWNTYPE:
		;
	}

	textOut.Printf("ENDPICT\n");
	return YSOK;
}

////////////////////////////////////////////////////////////

/*
Elevation Grid Data Format

TerrMesh     <- Header
NBLOCK x z   <- Number of Blocks
TMSIZE x z   <- Real Size of Terrain Mesh
BOTTOM R G B <- Enable Botton Wall and Set Color
RIGHT R G B  <- Enable Right Wall and Set Color
LEFT R G B   <- Enable Left Wall and Set Color
TOP R G B    <- Enable Top Wall and Set Color
BLOCK y [L/R] [ON/OFF] R1 G1 B1 [ON/OFF] R2 G2 B2
END

in short
  NBLOCK -> NBL
  TMSIZE -> TMS
  BOTTOM -> BOT
  RIGHT  -> RIG
  LEFT   -> LEF
  TOP    -> TOP
  BLOCK  -> BLO
  END    -> END

Extension:
CBE y0 y1 r0 g0 b0 r1 g1 b1  <- Color by elevation
*/

YSRESULT YsElevationGrid::BeginLoadTer(void)
{
	node.Set(0,NULL);
	reachEnd=YSFALSE;
	return YSOK;
}

YSRESULT YsElevationGrid::LoadTerOneLine(const char str[])
{
	YsString buf(str);
	YsArray <YsString,16> av;
	buf.Arguments(av);
	const auto ac=av.GetN();

	if(0<ac)
	{
		av[0].Capitalize();
		if(strcmp(av[0],"TERRMESH")==0)
		{
			return YSOK;
		}
		else if(0==strcmp(av[0],"SPEC"))
		{
			if(2<=av.GetN())
			{
				SetSpecular(YsStrToBool(av[1]));
			}
			return YSOK;
		}
		else if(strncmp(av[0],"NBLOCK",3)==0)
		{
			nx=atoi(av[1]);
			nz=atoi(av[2]);
			return YSOK;
		}
		else if(strncmp(av[0],"TMSIZE",3)==0)
		{
			xWid=atof(av[1]);
			zWid=atof(av[2]);
			return YSOK;
		}
		else if(strncmp(av[0],"BOTTOM",3)==0)
		{
			sideWall[0]=YSTRUE;
			sideWallColor[0].SetIntRGB(atoi(av[1]),atoi(av[2]),atoi(av[3]));
			return YSOK;
		}
		else if(strncmp(av[0],"RIGHT",3)==0)
		{
			sideWall[1]=YSTRUE;
			sideWallColor[1].SetIntRGB(atoi(av[1]),atoi(av[2]),atoi(av[3]));
			return YSOK;
		}
		else if(strncmp(av[0],"TOP",3)==0)
		{
			sideWall[2]=YSTRUE;
			sideWallColor[2].SetIntRGB(atoi(av[1]),atoi(av[2]),atoi(av[3]));
			return YSOK;
		}
		else if(strncmp(av[0],"LEFT",3)==0)
		{
			sideWall[3]=YSTRUE;
			sideWallColor[3].SetIntRGB(atoi(av[1]),atoi(av[2]),atoi(av[3]));
			return YSOK;
		}
		else if(strncmp(av[0],"BLOCK",3)==0)
		{
			unsigned int flags;
			YsElevationGridNode newNode;
			newNode.y=atof(av[1]);
			if(ac>2)
			{
				if(av[2][0]=='L')
				{
					newNode.lup=YSTRUE;
				}
				else
				{
					newNode.lup=YSFALSE;
				}

				if('0'<=av[3][0] && av[3][0]<='9')
				{
					flags=atoi(av[3]);
					newNode.visible[0]=((flags&1) ? YSTRUE : YSFALSE);
					newNode.protectPolygon[0]=((flags&2) ? YSTRUE : YSFALSE);
				}
				else if(strcmp(av[3],"ON")==0)
				{
					newNode.visible[0]=YSTRUE;
					newNode.protectPolygon[0]=YSFALSE;
				}
				else
				{
					newNode.visible[0]=YSFALSE;
					newNode.protectPolygon[0]=YSFALSE;
				}
				newNode.c[0].SetIntRGB(atoi(av[4]),atoi(av[5]),atoi(av[6]));

				if('0'<=av[7][0] && av[7][0]<='9')
				{
					flags=atoi(av[7]);
					newNode.visible[1]=((flags&1) ? YSTRUE : YSFALSE);
					newNode.protectPolygon[1]=((flags&2) ? YSTRUE : YSFALSE);
				}
				else if(strcmp(av[7],"ON")==0)
				{
					newNode.visible[1]=YSTRUE;
					newNode.protectPolygon[1]=YSFALSE;
				}
				else
				{
					newNode.visible[1]=YSFALSE;
					newNode.protectPolygon[1]=YSFALSE;
				}
				newNode.c[1].SetIntRGB(atoi(av[8]),atoi(av[9]),atoi(av[10]));

				if(newNode.protectPolygon[0]==YSTRUE || newNode.protectPolygon[1]==YSTRUE)
				{
					hasProtectPolygon=YSTRUE;
				}
			}
			else
			{
				newNode.lup=YSFALSE;
				newNode.visible[0]=YSFALSE;
				newNode.visible[1]=YSFALSE;
				newNode.c[0]=YsBlack();
				newNode.c[1]=YsBlack();
			}
			node.Append(newNode);
			return YSOK;
		}
		else if(strncmp(av[0],"CBE",3)==0)
		{
			colorByElevation=YSTRUE;
			colorByElevation_Elevation[0]=atof(av[1]);
			colorByElevation_Elevation[1]=atof(av[2]);
			colorByElevation_Color[0].SetIntRGB(atoi(av[3]),atoi(av[4]),atoi(av[5]));
			colorByElevation_Color[1].SetIntRGB(atoi(av[6]),atoi(av[7]),atoi(av[8]));
			return YSOK;
		}
		else if(0==strncmp(av[0],"TEXTURE",3))
		{
			if(3<=ac)
			{
				texLabel=av[2];
			}
			return YSOK;
		}
		else if(strncmp(av[0],"END",3)==0)
		{
			reachEnd=YSTRUE;
			if(node.GetN()==(nx+1)*(nz+1))
			{
				return YSOK;
			}
			else
			{
				YsPrintf("Number of nodes and number of blocks mismatch.\n");
				return YSERR;
			}
		}
		YsPrintf("Unrecognized command %s\n",str);
		return YSERR;
	}
	return YSOK;
}

YSRESULT YsElevationGrid::EndLoadTer(void)
{
	RecomputeBoundingBox();
	RecomputeNormal();
	return YSOK;
}

YSRESULT YsElevationGrid::LoadTer(const char fn[])
{
	FILE *fp=fopen(fn,"r");
	if(NULL!=fp)
	{
		YsTextFileInputStream inStream(fp);
		YSRESULT res=LoadTer(inStream);
		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT YsElevationGrid::LoadTer(YsTextInputStream &inStream)
{
	BeginLoadTer();

	YsString buf;
	while(NULL!=inStream.Gets(buf) && reachEnd!=YSTRUE)
	{
		LoadTerOneLine(buf);
	}
	EndLoadTer();

	return YSOK;
}

YSRESULT YsElevationGrid::LoadTer(const YsTextFile &txtFile)
{
	YsListItem <YsString> *str;
	BeginLoadTer();
	str=NULL;
	while((str=txtFile.GetText().FindNext(str))!=NULL)
	{
		if(LoadTerOneLine(str->dat)!=YSOK)
		{
			return YSERR;
		}
	}
	EndLoadTer();
	return YSOK;
}

YSRESULT YsElevationGrid::SaveTer(const char fn[]) const
{
	FILE *fp=fopen(fn,"w");
	if(NULL!=fp)
	{
		YsTextFileOutputStream textOut(fp);
		YSRESULT res=SaveTer(textOut);
		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT YsElevationGrid::SaveTer(YsTextOutputStream &textOut) const
{
	int i;

	textOut.Printf("TerrMesh\n");

	textOut.Printf("SPEC %s\n",YsBoolToStr(GetSpecular()));

	if(0<texLabel.Strlen())
	{
		textOut.Printf("TEX MAIN \"%s\"\n",texLabel.Txt());
	}

	textOut.Printf("NBL %d %d\n",nx,nz);

	textOut.Printf("TMS %.2lf %.2lf\n",xWid,zWid);

	if(colorByElevation==YSTRUE)
	{
		int r1,g1,b1,r2,g2,b2;
		colorByElevation_Color[0].GetIntRGB(r1,g1,b1);
		colorByElevation_Color[1].GetIntRGB(r2,g2,b2);
		textOut.Printf("CBE %.2lf %.2lf %d %d %d %d %d %d\n",
		    colorByElevation_Elevation[0],
		    colorByElevation_Elevation[1],
		    r1,g1,b1,
		    r2,g2,b2);
	}

	const char *const sideWallTag[]=
	{
		"BOT","RIG","TOP","LEF"
	};
	for(i=0; i<4; i++)
	{
		if(sideWall[i]==YSTRUE)
		{
			int r,g,b;
			sideWallColor[i].GetIntRGB(r,g,b);
			textOut.Printf("%s %d %d %d\n",sideWallTag[i],r,g,b);
		}
	}
	for(i=0; i<node.GetN(); i++)
	{
		if(i%(nx+1)!=nx && i<node.GetN()-(nx+1))  // Sccond condition is to save space for the last row
		{
			int r1,g1,b1,r2,g2,b2;
			unsigned int flags1,flags2;

			flags1=(node[i].visible[0]==YSTRUE ? 1 : 0)
			      +(node[i].protectPolygon[0]==YSTRUE ? 2 : 0);

			flags2=(node[i].visible[1]==YSTRUE ? 1 : 0)
			      +(node[i].protectPolygon[1]==YSTRUE ? 2 : 0);

			node[i].c[0].GetIntRGB(r1,g1,b1);
			node[i].c[1].GetIntRGB(r2,g2,b2);
			textOut.Printf("BLO %.2lf %s %d %d %d %d %d %d %d %d\n",
			      node[i].y,
			     (node[i].lup==YSTRUE ? "L" : "R"),
			     flags1,r1,g1,b1,
			     flags2,r2,g2,b2);
		}
		else
		{
			textOut.Printf("BLO %.2lf\n",node[i].y);
		}
	}

	textOut.Printf("END\n");

	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT YsSceneryItem::WriteCommonData(YsTextOutputStream &textOut)
{
	if(GetObjType()!=RECTREGION && GetObjType()!=GROUNDOBJECT && GetObjType()!=AIRCRAFT)
	{
		textOut.Printf("FIL \"%s\"\n",fName.c_str());
	}
	textOut.Printf("POS %.2lf %.2lf %.2lf %.2lf %.2lf %.2lf\n",
	    pos.x(),
	    pos.y(),
	    pos.z(),
	    att.h()*32768.0/YsPi,
	    att.p()*32768.0/YsPi,
	    att.b()*32768.0/YsPi);
	textOut.Printf("ID %d\n",
	    id);
	if(0<tagStr.Strlen())
	{
		textOut.Printf("TAG \"%s\"\n",tagStr.c_str());
	}
	if(YsTolerance<visibleDist)
	{
		textOut.Printf("LOD %.2lf\n",visibleDist);
	}
	return YSOK;
}

////////////////////////////////////////////////////////////

/************************************************************************
FIELD

GND r g b
SKY r g b

PC2
FIL filename.pc2
POS x y z h p b
END

PLT
FIL filename.pc2
POS x y z h p b
END

SRF
FIL filename.srf
POS x y z h p b
ID id
TAG string
LOD distance
END

TER
FIL filename.ter
POS x y z h p b
ID id
TAG string
LOD distance
CBE y0 y1 color0 color1   <- Color By Elevation (extension)
END

RGN
ARE xmin zmin xmax zmax
POS x y z h p b
ID id
TAG string
END

PST
ID id
TAG string
ISLOOP TRUE/FALSE
PNT x y z
PNT x y z
   :
   :
END

GOB                       <- YSFLIGHT Ground Object Seed (extension)
POS x y z h p b
NAM "GroundObjectName"
IFF number
PMT    <- If this exist, this ground object is a primary target.
FLG flags
ID id
TAG "string"
END

FLD
FIL filename.fld
POS x y z h p b
LOD distance
END

ENDF
************************************************************************/

const char *const YsScenery::state0KeyWordSource[]=
{
	"FIELD",     //  0
	"GND",       //  1
	"SKY",       //  2
	"PCK",       //  3
	"PC2",       //  4
	"PLT",       //  5
	"SRF",       //  6
	"TER",       //  7
	"RGN",       //  8
	"FLD",       //  9
	"ENDF",      // 10
	"GOB",       // 11
	"PST",       // 12
	"AOB",       // 13
	"EXTRA",     // 14
	"DEFAREA",   // 15 Area type of Field (not to confuse with Point Set)
	"BASEELV",   // 16 Base elevation.  Y=0 in this map is taken as true altitude of this.
	"MAGVAR",    // 17 Magnetic Variation (TO BE IMPLEMENTED)
	"ATC",       // 18 Begin ATC-related setting (TO BE IMPLEMENTED)
	"AIRSPACE",  // 19 Begin Airspace setting  (TO BE IMPLEMENTED)
	"FLDNAME",   // 20 Field Name (TO BE IMPLEMENTED)
	"STP",       // 21 Begin STP
	"AIRROUTE",  // 22 Begin Air Route
	"CANRESUME", // 23 Can resume
	"CANCONTINUE",//24 Can continue
	"TEXMAN",    // 25 Textuer-Manager command.
	"GNDSPECULAR", // 26 Ground specular reflection flag
	"FLDVERSION", // 27 .FLD version
	"APPROACH",   // 28 Approach-related data (TO BE IMPLEMENTED)
	"AIRPORT",    // 29 Airport-related data (TO BE IMPLEMENTED)
	"WAYPOINT",   // 30 Waypoint-related data (TO BE IMPLEMENTED)
	"WEATHER",    // 31 Weather-related data  (TO BE IMPLEMENTED)
	"SPECOBJ",    // 32 Field-Specific Object
	"REM",        // 33 Remark.  Apparently I was forgetting.

	NULL
};
YsKeyWordList YsScenery::state0KeyWordList;

const char *const YsScenery::state2KeyWordSource[]=
{
	"FIL",    //  0
	"POS",    //  1
	"ID",     //  2
	"TAG",    //  3
	"LOD",    //  4
	"ARE",    //  5
	"NAM",    //  6
	"IFF",    //  7
	"PMT",    //  8
	"FLG",    //  9
	"MPN",    // 10
	"MPS",    // 11
	"ISLOOP", // 12
	"PNT",    // 13
	"END",    // 14
	"EXTRA",  // 15
	"CMD",    // 16
	"FUEL",   // 17
	"SPEED",  // 18
	"LOWFUEL",// 19
	"ACTION", // 20
	"AREA",   // 21 Area type of Point Set (not to confuse with Field)
	"SUB",    // 22
	"COLL",   // 23 Collision shell for YsSceneryShell
	NULL
};
YsKeyWordList YsScenery::state2KeyWordList;

const char *const YsScenery::airActionKeyWordSource[]=
{
	"NULL",
	"LAND",
	"CIRCLE",
	"STRAIGHT",
	"FLYAIRROUTE",
	NULL
};
YsKeyWordList YsScenery::airActionKeyWordList;

const char *const YsScenery::airRouteKeyWordSource[]=
{
	"LABEL",
	"FIX",
	"AIRPORT",
	"ENDAIRROUTE",
	"VOR",
	"NDB",
	"CARRIER",
	NULL
};

YsKeyWordList YsScenery::airRouteKeyWordList;



YSRESULT YsScenery::LoadFldOneLine(const char str[])
{
	if('#'==str[0] || (0==strncmp(str,"REM",3) && (str[3]==0 || str[3]==' ' || str[3]=='\t')))
	{
		// Comment line
		return YSOK;
	}

	if(loadingState==LOADINGSTATE_PCK /*1*/)
	{
		YsListItem <YsString> *newLine;
		newLine=currentPckFile->CreateNewLine();
		if(newLine!=NULL)
		{
			newLine->dat=str;
		}
		else
		{
			return YSERR;
		}
		nPckFileRemain--;
		if(nPckFileRemain==0)
		{
			loadingState=LOADINGSTATE_OUTSIDE;;
		}
	}
	else
	{
		YsArray <YsString,16> args;
		YsString _buf(str);
		_buf.DeleteTailSpace();
		if(YSOK==_buf.Arguments(args) && 0<args.GetN())
		{
			args[0].Capitalize();
			if(loadingState==LOADINGSTATE_OUTSIDE /*0*/ )
			{
				if(state0KeyWordList.GetN()==0)
				{
					state0KeyWordList.MakeList(state0KeyWordSource);
				}

				const int cmd=state0KeyWordList.GetId(args[0]);
				switch(cmd)
				{
				case 0: // "FIELD",  //  0  if(strcmp(av[0],"FIELD")==0)
					return YSOK;
				case 1: // "GND",    //  1  else if(strcmp(av[0],"GND")==0)
					gndColor.SetIntRGB(atoi(args[1]),atoi(args[2]),atoi(args[3]));
					break;
				case 2: // "SKY",    //  2  else if(strcmp(av[0],"SKY")==0)
					skyColor.SetIntRGB(atoi(args[1]),atoi(args[2]),atoi(args[3]));
					break;
				case 3: // "PCK",    //  3  else if(strcmp(av[0],"PCK")==0)
					{
						YsListItem <YsTextFile> *newPack;
						newPack=pckFileList.Create();
						if(newPack!=NULL)
						{
							newPack->dat.Initialize();
							currentPckFile=&newPack->dat;
							newPack->dat.fName=args[1];
							nPckFileRemain=atoi(args[2]);
							loadingState=LOADINGSTATE_PCK /*1*/;

							// printf("Packed file  : %s\n",args[1].Txt());
							// printf("Num of lines : %d\n",nPckFileRemain);
						}
						else
						{
							return YSERR;
						}
					}
					break;
				case 4: // "PC2",    //  4  else if(strcmp(av[0],"PC2")==0)
					currentItem=&CreateMap()->dat;
					loadingState=LOADINGSTATE_PC2 /*2*/;
					break;
				case 5: // "PLT",    //  5  else if(strcmp(av[0],"PLT")==0)
					currentItem=&CreateSignBoard()->dat;
					loadingState=LOADINGSTATE_PLT /*3*/;
					break;
				case 6: // "SRF",    //  6  else if(strcmp(av[0],"SRF")==0)
					currentItem=&CreateShell()->dat;
					loadingState=LOADINGSTATE_SRF /*4*/;
					break;
				case 7: // "TER",    //  7  else if(strcmp(av[0],"TER")==0)
					currentItem=&CreateElevationGrid()->dat;
					loadingState=LOADINGSTATE_TER /*5*/;
					break;
				case 8: // "RGN",    //  8  else if(strcmp(av[0],"RGN")==0)
					currentItem=&CreateRectRegion()->dat;
					loadingState=LOADINGSTATE_RGN /*6*/;
					break;
				case 9: // "FLD",    //  9  else if(strcmp(av[0],"FLD")==0)
					currentItem=&CreateChildScenery()->dat;
					loadingState=LOADINGSTATE_FLD /*7*/;
					break;
				case 10: // "ENDF",   // 10  else if(strcmp(av[0],"ENDF")==0)
					loadingState=LOADINGSTATE_ENDOFFILE /*8*/;
					break;
				case 11: // "GOB",    // 11  else if(strcmp(av[0],"GOB")==0)
					currentItem=&CreateGndObj()->dat;
					loadingState=LOADINGSTATE_GOB /*9*/;
					break;
				case 12: // "PST",    // 12  else if(strcmp(av[0],"PST")==0)
					currentItem=&CreatePointSet()->dat;
					loadingState=LOADINGSTATE_PST /*10*/;
					break;
				case 13: // "AOB",    // 13  else if(strcmp(av[0],"AOB")==0)
					currentItem=&CreateAir()->dat;
					loadingState=LOADINGSTATE_AOB /*11*/;
					break;
				case 14: // "EXTRA",  // 14  else if(strcmp(av[0],"EXTRA")==0)
					break;
				case 15: // "DEFAREA"
					areaType=YsScenery::GetAreaTypeFromString(args[1]);
					break;
				case 16: // "BASEELV",   // 16 Base elevation.  Y=0 in this map is taken as true altitude of this.
					FsGetLength(baseElevation,args[1]);
					break;
				case 17: // "MAGVAR", // 17 Magnetic Variation
					FsGetAngle(magneticVariation,args[1]);
					break;
				case 18: // "ATC",    // 18 ATC-related setting
					loadingState=LOADINGSTATE_ATC /*100*/;
					break;
				case 19: // "AIRSPACE", // 19 Airspace setting  (to be implemented)
					loadingState=LOADINGSTATE_AIRSPACE /*101*/;
					break;
				case 20: // "FLDNAME",   // 20 Field Name (to be implemented)
					if(2<=args.GetN())
					{
						idName=args[1];
					}
					break;
				case 21: // "STP",       // 21 Begin STP
					loadingState=LOADINGSTATE_STP;
					break;
				case 22:
					{
						YsSceneryAirRouteHandle arHd=CreateAirRoute();
						currentAirRoute=airRouteList[arHd];
					}
					loadingState=LOADINGSTATE_AIRROUTE;
					break;
				case 23:
					if(2<=args.GetN())
					{
						FsGetBool(canResume,args[1]);
					}
					break;
				case 24: // "CANCONTINUE"
					if(2<=args.GetN())
					{
						FsGetBool(canContinue,args[1]);
					}
					break;
				case 25: // "TEXMAN"
					textureManager.LoadTexManOneLine(_buf,args.GetN(),args);
					break;
				case 26: // 	"GNDSPECULAR", // 26 Ground specular reflection flag
					if(2<=args.GetN())
					{
						SetSpecular(YsStrToBool(args[1]));
					}
					break;
				case 27: // "FLDVERSION", // 27 .FLD version
					if(2<=args.GetN())
					{
						const int fldVersion=atoi(args[1]);
						if(YSSCN_VERSION<fldVersion)
						{
							SetLastError(ERROR_IO_NEED_NEW_VERSION);
							goto ERROR;
						}
					}
					break;
				case 28: // "APPROACH",   // 28 Approach-related data (to be implemented)
					loadingState=LOADINGSTATE_APPROACH;
					break;
				case 29: // "AIRPORT",    // 29 Airport-related data (TO BE IMPLEMENTED)
					loadingState=LOADINGSTATE_AIRPORT;
					break;
				case 30: // "WAYPOINT",   // 30 Waypoint-related data (TO BE IMPLEMENTED)
					break;
				case 31: // "WEATHER",    // 31 Weather-related data  (TO BE IMPLEMENTED)
					loadingState=LOADINGSTATE_WEATHER;
					break;
				case 32: // "SPECOBJ"     // 32 Field-specific object
					loadingState=LOADINGSTATE_SPECOBJ;
					break;

				default:
					goto ERROR;
				}
			}
			else if(LOADINGSTATE_PC2==loadingState ||
					LOADINGSTATE_PLT==loadingState ||
					LOADINGSTATE_SRF==loadingState ||
					LOADINGSTATE_TER==loadingState ||
					LOADINGSTATE_RGN==loadingState ||
					LOADINGSTATE_FLD==loadingState ||
					LOADINGSTATE_GOB==loadingState ||
					LOADINGSTATE_PST==loadingState ||
					LOADINGSTATE_AOB==loadingState)
				// (2<=loadingState && loadingState<=7) || loadingState==9 || loadingState==10 || loadingState==11)
			{
				if(state2KeyWordList.GetN()==0)
				{
					state2KeyWordList.MakeList(state2KeyWordSource);
				}

				const int cmd=state2KeyWordList.GetId(args[0]);
				switch(cmd)
				{
				case  0: // "FIL",    //  0  if(strcmp(args[0],"FIL")==0)
					{
						YsListItem <YsTextFile> *txtFile;
						txtFile=NULL;
						while((txtFile=pckFileList.FindNext(txtFile))!=NULL)
						{
							if(txtFile->dat.fName.Strcmp(args[1])==0)
							{
								// printf("Packed File:%s\n",txtFile->dat.fn.Txt());
								break;
							}
						}

						if(txtFile!=NULL)
						{
							YSRESULT err;
							switch(loadingState)
							{
							case LOADINGSTATE_PC2 /*2*/:
								err=((YsScenery2DDrawing *)currentItem)->drw.LoadPc2(txtFile->dat);
								break;
							case LOADINGSTATE_PLT /*3*/:
								err=((YsScenery2DDrawing *)currentItem)->drw.LoadPc2(txtFile->dat);
								break;
							case LOADINGSTATE_SRF /*4*/:
								err=((YsSceneryShell *)currentItem)->LoadSrf(txtFile->dat);
								break;
							case LOADINGSTATE_TER /*5*/:
								err=((YsSceneryElevationGrid *)currentItem)->evg.LoadTer(txtFile->dat);
								break;
							case LOADINGSTATE_FLD /*7*/:
								err=((YsScenery *)currentItem)->LoadFld(txtFile->dat);
								break;
							default:
								err=YSOK; // Ignore it
								break;
							}
							if(err!=YSOK)
							{
								return YSERR;
							}
						}
						else
						{
							YSRESULT err;
							YsString fulPath;
							fulPath.MakeFullPathName(curPath,args[1]);
							switch(loadingState)
							{
							case LOADINGSTATE_PC2 /*2*/:
								err=((YsScenery2DDrawing *)currentItem)->drw.LoadPc2(fulPath);
								break;
							case LOADINGSTATE_PLT /*3*/:
								err=((YsScenery2DDrawing *)currentItem)->drw.LoadPc2(fulPath);
								break;
							case LOADINGSTATE_SRF /*4*/:
								{
									YsFileIO::File fp(fulPath,"r");
									if(nullptr!=fp)
									{
										YsTextFileInputStream inStream(fp);
										err=((YsSceneryShell *)currentItem)->shl.Load(inStream);
									}
								}
								break;
							case LOADINGSTATE_TER /*5*/:
								err=((YsSceneryElevationGrid *)currentItem)->evg.LoadTer(fulPath);
								break;
							case LOADINGSTATE_FLD /*7*/:
								err=((YsScenery *)currentItem)->LoadFld(fulPath);
								break;
							default:
								err=YSOK; // Ignore it
								break;
							}
							if(err!=YSOK)
							{
								return YSERR;
							}
						}
						currentItem->fName=args[1];
					}
					break;
				case  1: // "POS",    //  1  else if(strcmp(args[0],"POS")==0)
					currentItem->pos.Set(atof(args[1]),atof(args[2]),atof(args[3]));
					currentItem->att.Set(atof(args[4])*YsPi/32768.0,
					                     atof(args[5])*YsPi/32768.0,
					                     atof(args[6])*YsPi/32768.0);
					break;
				case  2: // "ID",     //  2  else if(strcmp(args[0],"ID")==0)
					currentItem->id=atoi(args[1]);
					break;
				case  3: // "TAG",    //  3  else if(strcmp(args[0],"TAG")==0)
					currentItem->tagStr=args[1];
					break;
				case  4: // "LOD",    //  4  else if(strcmp(args[0],"LOD")==0)
					currentItem->visibleDist=atof(args[1]);
					break;
				case  5: // "ARE",    //  5  else if(strcmp(args[0],"ARE")==0 && loadingState==6)
					if(loadingState==LOADINGSTATE_RGN /*6*/)
					{
						YsSceneryRectRegion *rgn;
						rgn=(YsSceneryRectRegion *)currentItem;
						rgn->min.Set(atof(args[1]),atof(args[2]));
						rgn->max.Set(atof(args[3]),atof(args[4]));
					}
					break;
				case  6: // "NAM",    //  6  else if(strcmp(args[0],"NAM")==0 && loadingState==9)
					if(loadingState==LOADINGSTATE_GOB /*9*/)
					{
						YsSceneryGndObj *gnd;
						gnd=(YsSceneryGndObj *)currentItem;
						gnd->objName=args[1];
					}
					else if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air;
						air=(YsSceneryAir *)currentItem;
						air->objName=args[1];
					}
					break;
				case  7: // "IFF",    //  7  else if(strcmp(args[0],"IFF")==0 && loadingState==9)
					if(loadingState==LOADINGSTATE_GOB /*9*/)
					{
						YsSceneryGndObj *gnd;
						gnd=(YsSceneryGndObj *)currentItem;
						gnd->iff=atoi(args[1]);
					}
					else if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air;
						air=(YsSceneryAir *)currentItem;
						air->iff=atoi(args[1]);
					}
					break;
				case  8: // "PMT",    //  8  else if(strcmp(args[0],"PMT")==0 && loadingState==9)
					if(loadingState==LOADINGSTATE_GOB /*9*/)
					{
						YsSceneryGndObj *gnd;
						gnd=(YsSceneryGndObj *)currentItem;
						gnd->primaryTarget=YSTRUE;
					}
					break;
				case  9: // "FLG",    //  9  else if(strcmp(args[0],"FLG")==0 && loadingState==9)
					if(loadingState==LOADINGSTATE_GOB /*9*/)
					{
						YsSceneryGndObj *gnd;
						gnd=(YsSceneryGndObj *)currentItem;
						gnd->gndFlag=atoi(args[1]);
					}
					else if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air;
						air=(YsSceneryAir *)currentItem;
						air->airFlag=atoi(args[1]);
					}
					break;
				case 10: // "MPN",    // 10  else if(strcmp(args[0],"MPN")==0 && loadingState==9)
					if(loadingState==LOADINGSTATE_GOB /*9*/)
					{
						YsSceneryGndObj *gnd;
						gnd=(YsSceneryGndObj *)currentItem;
						gnd->motionPathName=args[1];
					}
					break;
				case 11: // "MPS",    // 11  else if(strcmp(args[0],"MPS")==0 && loadingState==9)
					if(loadingState==LOADINGSTATE_GOB /*9*/)
					{
						YsSceneryGndObj *gnd;
						gnd=(YsSceneryGndObj *)currentItem;
						gnd->motionPathOffset=YSTRUE;
					}
					break;
				case 12: // "ISLOOP", // 12  else if(strcmp(args[0],"ISLOOP")==0 && loadingState==10)
					if(loadingState==LOADINGSTATE_PST /*10*/)
					{
						YsSceneryPointSet *pst;
						pst=(YsSceneryPointSet *)currentItem;
						pst->isLoop=(args[1].STRCMP("TRUE")==0 ? YSTRUE : YSFALSE);
					}
					break;
				case 13: // "PNT",    // 13  else if(strcmp(args[0],"PNT")==0 && loadingState==10)
					if(loadingState==LOADINGSTATE_PST /*10*/)
					{
						YsSceneryPointSet *pst;
						pst=(YsSceneryPointSet *)currentItem;
						pst->pnt.Append(YsVec3(atof(args[1]),atof(args[2]),atof(args[3])));
					}
					break;
				case 14: // "END",    // 14  else if(strcmp(args[0],"END")==0)
					switch(loadingState)
					{
					case LOADINGSTATE_SRF:
						{
							auto srf=dynamic_cast <YsSceneryShell *>(currentItem);
							srf->CacheCollLattice();
						}
						break;
					default:
						break;
					}
					loadingState=LOADINGSTATE_OUTSIDE /*0*/;
					break;
				case 15: // "EXTRA",  // 15
					// For future updates.
					break;
				case 16: // "CMD"
					if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air;
						air=(YsSceneryAir *)currentItem;

						if(strcmp(args[1],"CTLLDGEA")==0)
						{
							if(strcmp(args[2],"TRUE")==0)
							{
								air->ldg=YSTRUE;
							}
							else
							{
								air->ldg=YSFALSE;
							}
						}
					}
					break;
				case 17: // "FUEL",   // 17
					if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air;
						air=(YsSceneryAir *)currentItem;
						air->fuel=atoi(args[1]);
					}
					break;
				case 18: // "SPEED",  // 18
					if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air;
						air=(YsSceneryAir *)currentItem;
						FsGetSpeed(air->speed,args[1]);
					}
					break;
				case 19: //	"LOWFUEL",// 19
					if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air;
						air=(YsSceneryAir *)currentItem;
						air->landWhenLowFuel=atof(args[1]);
					}
					break;
				case 20: // "ACTION", // 20
					if(loadingState==LOADINGSTATE_AOB /*11*/)
					{
						YsSceneryAir *air=(YsSceneryAir *)currentItem;
						if(airActionKeyWordList.GetN()==0)
						{
							airActionKeyWordList.MakeList(airActionKeyWordSource);
						}

						int actType;
						actType=airActionKeyWordList.GetId(args[1]);
						air->action.actType=(YsSceneryAirAction::ACTIONTYPE)actType;

						switch(air->action.actType)
						{
						default:
							break;
						case YsSceneryAirAction::FLYAIRROUTE:
							air->action.actParam[0].Set("");
							if(3<=args.GetN())
							{
								air->action.actParam[0]=args[2];
							}
							break;
						}
					}
					break;
				case 21: // "AREA" // 21
					if(loadingState==10)
					{
						YsSceneryPointSet *pst;
						pst=(YsSceneryPointSet *)currentItem;
						pst->areaType=YsScenery::GetAreaTypeFromString(args[1]);
					}
					break;
				case 22: // "SUB" // 22
					if(loadingState==LOADINGSTATE_RGN && 2<=args.GetN()/*6*/)
					{
						YsSceneryRectRegion::SUBCLASSTYPE subClassType=YsSceneryRectRegion::GetSubClassTypeFromString(args[1]);

						YsSceneryRectRegion *rgn;
						rgn=(YsSceneryRectRegion *)currentItem;
						rgn->SetSubClassType(subClassType);
					}
					break;
				case 23: // "COLL"
					if(LOADINGSTATE_SRF==loadingState && 2<=args.size())
					{
						auto srf=dynamic_cast <YsSceneryShell *>(currentItem);
						YsListItem <YsTextFile> *txtFile=nullptr;
						while((txtFile=pckFileList.FindNext(txtFile))!=NULL)
						{
							if(txtFile->dat.fName.Strcmp(args[1])==0)
							{
								// printf("Packed File:%s\n",txtFile->dat.fn.Txt());
								break;
							}
						}

						YSRESULT err=YSERR;
						if(nullptr!=txtFile)
						{
							switch(loadingState)
							{
							case LOADINGSTATE_SRF /*4*/:
								err=srf->LoadCollSrf(txtFile->dat);
								break;
							default:
								break;
							}
						}
						else
						{
							YsString fulPath;
							fulPath.MakeFullPathName(curPath,args[1]);
							YsFileIO::File fp(fulPath,"r");
							if(nullptr!=fp)
							{
								YsTextFileInputStream inStream(fp);
								switch(loadingState)
								{
								case LOADINGSTATE_SRF /*4*/:
									err=srf->collShl.LoadSrf(inStream);
									break;
								default:
									break;
								}
							}
						}
						if(err!=YSOK)
						{
							return YSERR;
						}
					}
					break;
				default:
					goto ERROR;
				}
			}
			else if(LOADINGSTATE_ATC /*100*/==loadingState) // ATC
			{
				if(0==strcmp(args[0],"ENDATC"))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
			else if(LOADINGSTATE_AIRSPACE /*101*/==loadingState) // AIRSPACE
			{
				if(0==strcmp(args[0],"ENDAIRSPACE"))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
			else if(LOADINGSTATE_STP /*102*/==loadingState) // STP
			{
				if(0==strcmp(args[0],"ENDSTP"))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
			else if(LOADINGSTATE_AIRROUTE==loadingState)
			{
				if(airRouteKeyWordList.GetN()==0)
				{
					airRouteKeyWordList.MakeList(airRouteKeyWordSource);
				}

				if(NULL!=currentAirRoute)
				{
					int seqIdx=-1;
					const int cmd=airRouteKeyWordList.GetId(args[0]);
					switch(cmd)
					{
					case 0: // "LABEL",
						currentAirRoute->label=args[1];
						break;
					case 1: // "FIX",
						seqIdx=currentAirRoute->AddSequence(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX);
						break;
					case 2: // "AIRPORT",
						seqIdx=currentAirRoute->AddSequence(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT);
						break;
					case 3: // "ENDAIRROUTE",
						loadingState=LOADINGSTATE_OUTSIDE;
						break;
					case 4: // "VOR",
						seqIdx=currentAirRoute->AddSequence(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR);
						break;
					case 5: // "NDB",
						seqIdx=currentAirRoute->AddSequence(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB);
						break;
					case 6: // "CARRIER",
						seqIdx=currentAirRoute->AddSequence(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER);
						break;
					}
					if(0<=seqIdx)
					{
						for(int i=1; i<args.GetN(); ++i)
						{
							if(0==strncmp("LABEL:",args[i],6))
							{
								currentAirRoute->SetSequenceLabel(seqIdx,args[i].Txt()+6);
							}
							else if(0==strncmp("ALT:",args[i],4))
							{
								double alt;
								FsGetLength(alt,args[i].Txt()+4);
								currentAirRoute->SetSequenceAltitude(seqIdx,alt);
							}
							else if(0==strncmp("DEPTALT:",args[i],8))
							{
								double alt;
								FsGetLength(alt,args[i].Txt()+8);
								currentAirRoute->SetTakeOffClimbAltitude(seqIdx,alt);
							}
							else if(0==strncmp("ILS:",args[i],4))
							{
								currentAirRoute->AddSequenceILS(seqIdx,args[i].Txt()+4);
							}
							else if(0==strncmp("APPROACH:",args[i],9))
							{
								double dist;
								FsGetLength(dist,args[i].Txt()+9);
								currentAirRoute->SetSequenceBeginApproachDist(seqIdx,dist);
							}
						}
					}
				}
				else if(0==strcmp("ENDAIRROUTE",args[0]))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
			else if(LOADINGSTATE_APPROACH==loadingState)
			{
				if(0==args[0].Strcmp("ENDAPPROACH"))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
			else if(LOADINGSTATE_AIRPORT==loadingState)
			{
				if(0==args[0].Strcmp("ENDAIRPORT"))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
			else if(LOADINGSTATE_WEATHER==loadingState)
			{
				if(0==args[0].Strcmp("ENDWEATHER"))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
			else if(LOADINGSTATE_SPECOBJ==loadingState)
			{
				if(0==args[0].Strcmp("ENDSPECOBJ"))
				{
					loadingState=LOADINGSTATE_OUTSIDE;
				}
			}
		}
	}
	return YSOK;

ERROR:
	YsPrintf("Unrecognized line\n");
	YsPrintf("%s\n",str);
	return YSERR;
}

YSRESULT YsScenery::EndLoadFld(void)
{
	RecomputeBoundingBox();
	pckFileList.CleanUp();
	pckFileAllocator.CollectGarbage();

	// printf("pckFileList\n");
	// pckFileList.SelfDiagnostic();
	// printf("pckFileAllocator\n");
	// pckFileAllocator.SelfDiagnostic();

	textureManager.EndLoadTexMan();

	return YSOK;
}

YSRESULT YsScenery::LoadFld(const char fn[])
{
	FILE *fp=fopen(fn,"r");
	if(NULL!=fp)
	{
		YsString ful(fn),pth,fil,str;
		ful.SeparatePathFile(pth,fil);

		BeginLoadFld(pth);
		while(str.Fgets(fp)!=NULL)
		{
			if(LoadFldOneLine(str)!=YSOK)
			{
				EndLoadFld();
				fclose(fp);
				return YSERR;
			}
		}
		EndLoadFld();
		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsScenery::LoadFld(FILE *fp)
{
	if(NULL!=fp)
	{
		YsString str;
		BeginLoadFld(".");
		while(str.Fgets(fp)!=NULL)
		{
			if(LoadFldOneLine(str)!=YSOK)
			{
				EndLoadFld();
				return YSERR;
			}
		}
		EndLoadFld();
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsScenery::LoadFld(const YsTextFile &txtFile)
{
	YsListItem <YsString> *str;
	BeginLoadFld(".");
	str=NULL;
	while((str=txtFile.GetText().FindNext(str))!=NULL)
	{
		if(LoadFldOneLine(str->dat)!=YSOK)
		{
			return YSERR;
		}
	}
	EndLoadFld();
	return YSOK;
}

YSRESULT YsScenery::SaveFld(const char fn[])
{
	GetLastError();

	FILE *fp=fopen(fn,"w");
	if(NULL!=fp)
	{
		YsTextFileOutputStream textOut(fp);
		YSRESULT res=SaveFld(textOut);

		if(ferror(fp))
		{
			const int err=errno;
			lastErrorStr.Set(strerror(err));
			lastErrorStr.Append("(J)");
			res=YSERR;
		}

		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT YsScenery::SaveFld(YsTextOutputStream &textOut)
{
	GetLastError();

	lastErrorNo=0;
	lastErrorStr.Set("");

	int n;
	n=0;
	if(AssignUniqueFilename(n)==YSOK &&
	   SaveSubScenery(textOut)==YSOK)
	{
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsScenery::ExportYfs(const char fn[]) const
{
	FILE *fp;
	fp=fopen(fn,"w");
	if(fp!=NULL)
	{
		YsArray <YsSceneryGndObj *> gobList;
		YsArray <YsSceneryAir *> airList;
		int i;
		YsVec3 ev,uv,p;
		YsMatrix4x4 objMat;
		YsAtt3 a;

		fprintf(fp,"YFSVERSI 20141101\n");
		fprintf(fp,"\n");
		fprintf(fp,"REM FIELDNAM \"\" 0 0 0 0 0 0 FALSE LOADAIR:FALSE\n");
		fprintf(fp,"\n");

		for(auto &str : yfsHeader)
		{
			fprintf(fp,"%s\n",str.Txt());
		}


		YsCoordSysModel=YSBLUEIMPULSE;


		MakeListOfAir(airList);
		for(i=0; i<airList.GetN(); i++)
		{
			GetTransformation(objMat,airList[i]);
			ev=YsZVec();
			uv=YsYVec();
			p=YsOrigin();

			objMat.Mul(ev,ev,0.0);
			objMat.Mul(uv,uv,0.0);
			objMat.Mul(p,p,1.0);

			a.SetTwoVector(ev,uv);

			fprintf(fp,"AIRPLANE \"%s\" %s\n",airList[i]->GetObjName(),YsBoolToStr(airList[i]->isPlayer));
			fprintf(fp,"IDENTIFY %d\n",airList[i]->iff);

			if(airList[i]->airFlag!=0)
			{
				fprintf(fp,"AIRFLAGS %u\n",airList[i]->airFlag);
			}

			fprintf(fp,"AIRPCMND POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
			fprintf(fp,"AIRPCMND ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg\n",
			    YsRadToDeg(a.h()),YsRadToDeg(a.p()),YsRadToDeg(a.b()));

			if(airList[i]->GetLandingGear()==YSTRUE)
			{
				fprintf(fp,"AIRPCMND CTLLDGEA TRUE\n");
			}
			else
			{
				fprintf(fp,"AIRPCMND CTLLDGEA FALSE\n");
			}

			fprintf(fp,"AIRPCMND INITSPED %.2lfm/s\n",airList[i]->GetSpeed());
			fprintf(fp,"AIRPCMND INITFUEL %d%%\n",airList[i]->GetFuel());

			if(airList[i]->GetId()!=0 || airList[i]->GetTag()[0]!=0)
			{
				fprintf(fp,"IDANDTAG %d \"%s\"\n",airList[i]->GetId(),airList[i]->GetTag());
			}

			fprintf(fp,"LANDLWFL %.2lf\n",airList[i]->GetLandWhenLowFuel());

			if(YsSceneryAirAction::CUSTOM==airList[i]->action.actType)
			{
				for(auto &str : airList[i]->action.customIntention)
				{
					fprintf(fp,"%s\n",str.Txt());
				}
			}

			for(auto &str : airList[i]->otherCommand)
			{
				fprintf(fp,"%s\n",str.Txt());
			}

			fprintf(fp,"\n");
		}


		MakeListOfGndObj(gobList);
		for(i=0; i<gobList.GetN(); i++)
		{
			GetTransformation(objMat,gobList[i]);
			ev=YsZVec();
			uv=YsYVec();
			p=YsOrigin();

			objMat.Mul(ev,ev,0.0);
			objMat.Mul(uv,uv,0.0);
			objMat.Mul(p,p,1.0);

			a.SetTwoVector(ev,uv);

			fprintf(fp,"GROUNDOB \"%s\"\n",gobList[i]->GetObjName());
			fprintf(fp,"IDENTIFY %d\n",gobList[i]->iff);
			fprintf(fp,"GNDPOSIT %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
			fprintf(fp,"GNDATTIT %.2lfdeg %.2lfdeg %.2lfdeg\n",YsRadToDeg(a.h()),YsRadToDeg(a.p()),YsRadToDeg(a.b()));
			if(gobList[i]->GetId()!=0 || gobList[i]->GetTag()[0]!=0)
			{
				fprintf(fp,"IDANDTAG %d \"%s\"\n",gobList[i]->GetId(),gobList[i]->GetTag());
			}
			if(gobList[i]->IsPrimaryTarget()==YSTRUE)
			{
				fprintf(fp,"PRTARGET\n");
			}
			fprintf(fp,"GNDFLAGS %d\n",gobList[i]->GetFlag());
			if(gobList[i]->motionPathName.Strlen()>0)
			{
				fprintf(fp,"MOTNPATH \"%s\" %s\n",
				   gobList[i]->motionPathName.data(),
				   (gobList[i]->motionPathOffset==YSTRUE ? "TRUE" : "FALSE"));
			}
			fprintf(fp,"\n");
		}

		YsCoordSysModel=YSOPENGL;

		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsScenery::BeginLoadFld(const char pth[])
{
	loadingState=LOADINGSTATE_OUTSIDE;
	pckFileList.CleanUp();
	nPckFileRemain=0;
	currentPckFile=NULL;
	currentItem=NULL;
	currentAirRoute=NULL;

	textureManager.BeginLoadTexMan();

	curPath=pth;
	if(curPath[0]==0 ||
	   (curPath.back()!='/' && curPath.back()!='\\'))
	{
		curPath.push_back('/');
	}
	return YSOK;
}

YSRESULT YsScenery::AssignUniqueFilename(int &n)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsScenery> *scn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryPointSet> *pst;

	shl=NULL;
	while((shl=FindNextShell(shl))!=NULL)
	{
		shl->dat.fName.Printf("%08d.srf",n++);
		shl->dat.collFName.Printf("%08d.srf",n++);
	}

	evg=NULL;
	while((evg=FindNextElevationGrid(evg))!=NULL)
	{
		evg->dat.fName.Printf("%08d.ter",n++);
	}

	drw=NULL;
	while((drw=FindNextMap(drw))!=NULL)
	{
		drw->dat.fName.Printf("%08d.pc2",n++);
	}

	drw=NULL;
	while((drw=FindNextSignBoard(drw))!=NULL)
	{
		drw->dat.fName.Printf("%08d.pc2",n++);
	}

	rgn=NULL;
	while((rgn=FindNextRectRegion(rgn))!=NULL)
	{
		rgn->dat.fName="";
	}

	gnd=NULL;
	while((gnd=gndList.FindNext(gnd))!=NULL)
	{
		gnd->dat.fName="";
	}

	pst=NULL;
	while((pst=pstList.FindNext(pst))!=NULL)
	{
		pst->dat.fName="";
	}

	scn=NULL;
	while((scn=FindNextChildScenery(scn))!=NULL)
	{
		scn->dat.fName.Printf("%08d.fld",n++);
		scn->dat.AssignUniqueFilename(n);
	}
	return YSOK;
}

YSRESULT YsScenery::SaveSubScenery(YsTextOutputStream &textOut)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsSceneryPointSet> *pst;
	YsListItem <YsScenery> *scn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryAir> *air;

	{
		textOut.Printf("FIELD\n");

		textOut.Printf("FLDVERSION %d\n",(int)YSSCN_VERSION);

		textureManager.Save(textOut);

		if(0<idName.Strlen())
		{
			YsString conv(idName);
			for(int i=0; i<conv.Strlen(); i++)
			{
				if(' '==conv[i] || '\t'==conv[i])
				{
					conv.Set(i,"_");
				}
			}
			textOut.Printf("FLDNAME %s\n",conv.Txt());
		}

		int r,g,b;
		gndColor.GetIntRGB(r,g,b);
		textOut.Printf("GND %d %d %d\n",r,g,b);
		skyColor.GetIntRGB(r,g,b);
		textOut.Printf("SKY %d %d %d\n",r,g,b);

		textOut.Printf("GNDSPECULAR %s\n",YsBoolToStr(GetSpecular()));

		textOut.Printf("DEFAREA %s\n",GetAreaTypeString(areaType));
		textOut.Printf("BASEELV %.2lfm\n",baseElevation);
		textOut.Printf("MAGVAR %.2lfdeg\n",YsRadToDeg(magneticVariation));
		textOut.Printf("CANRESUME %s\n",YsBoolToStr(canResume));


		YsSceneryAirRouteHandle arHd=NULL;
		while(YSOK==airRouteList.MoveToNext(arHd))
		{
			const YsSceneryAirRoute *airRoute=airRouteList[arHd];
			if(NULL!=airRoute)
			{
				textOut.Printf("AIRROUTE\n");
				textOut.Printf("LABEL \"%s\"\n",airRoute->label.Txt());
				for(int seqIdx=0; seqIdx<airRoute->routeSequence.GetN(); ++seqIdx)
				{
					switch(airRoute->routeSequence[seqIdx].segType)
					{
					default:
						break;
					case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX:
						textOut.Printf("FIX");
						break;
					case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT:
						textOut.Printf("AIRPORT");
						for(int ilsIdx=0; ilsIdx<airRoute->routeSequence[seqIdx].ilsArray.GetN(); ++ilsIdx)
						{
							textOut.Printf(" ILS:%s",airRoute->routeSequence[seqIdx].ilsArray[ilsIdx].Txt());
						}
						textOut.Printf(" APPROACH:%.2lfm",airRoute->routeSequence[seqIdx].beginApproachAt);
						textOut.Printf(" DEPTALT:%.2lfft",YsUnitConv::MtoFT(airRoute->routeSequence[seqIdx].takeOffClimbAlt));
						break;
					case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR:
						textOut.Printf("VOR");
						break;
					case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB:
						textOut.Printf("NDB");
						break;
					case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER:
						textOut.Printf("CARRIER");
						textOut.Printf(" APPROACH:%.2lfm",airRoute->routeSequence[seqIdx].beginApproachAt);
						textOut.Printf(" DEPTALT:%.2lfft",YsUnitConv::MtoFT(airRoute->routeSequence[seqIdx].takeOffClimbAlt));
						break;

					}
					textOut.Printf(" LABEL:%s",airRoute->routeSequence[seqIdx].label.Txt());
					textOut.Printf(" ALT:%.2lfft",YsUnitConv::MtoFT(airRoute->routeSequence[seqIdx].altitude));
					textOut.Printf("\n");
				}
				textOut.Printf("ENDAIRROUTE\n");
			}
		}


		shl=NULL;
		while((shl=FindNextShell(shl))!=NULL)
		{
			{
				YsTextMemoryOutputStream tmpfn;
printf("%s %d\n",__FUNCTION__,__LINE__);
				if(shl->dat.Save(tmpfn)!=YSOK || MergePackedFile(textOut,shl->dat.fName.c_str(),tmpfn)!=YSOK)
				{
					return YSERR;
				}
printf("%s %d\n",__FUNCTION__,__LINE__);
			}
			if(0<shl->dat.collShl.GetNumPolygon())
			{
				YsTextMemoryOutputStream collOut;
printf("%s %d\n",__FUNCTION__,__LINE__);
				if(YSOK!=shl->dat.SaveCollSrf(collOut) || YSOK!=MergePackedFile(textOut,shl->dat.collFName.c_str(),collOut))
				{
					return YSERR;
				}
printf("%s %d\n",__FUNCTION__,__LINE__);
			}
		}

		evg=NULL;
		while((evg=FindNextElevationGrid(evg))!=NULL)
		{
			YsTextMemoryOutputStream tmpfn;
			if(evg->dat.Save(tmpfn)!=YSOK || MergePackedFile(textOut,evg->dat.fName.c_str(),tmpfn)!=YSOK)
			{
				return YSERR;
			}
		}

		drw=NULL;
		while((drw=FindNextMap(drw))!=NULL)
		{
			YsTextMemoryOutputStream tmpfn;
			if(drw->dat.Save(tmpfn)!=YSOK || MergePackedFile(textOut,drw->dat.fName.c_str(),tmpfn)!=YSOK)
			{
				return YSERR;
			}
		}

		drw=NULL;
		while((drw=FindNextSignBoard(drw))!=NULL)
		{
			YsTextMemoryOutputStream tmpfn;
			if(drw->dat.Save(tmpfn)!=YSOK || MergePackedFile(textOut,drw->dat.fName.c_str(),tmpfn)!=YSOK)
			{
				return YSERR;
			}
		}

		int nSubScnWritten=0;

		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			YsTextMemoryOutputStream tmpfn;
			if(scn->dat.SaveSubScenery(tmpfn)!=YSOK || MergePackedFile(textOut,scn->dat.fName.c_str(),tmpfn)!=YSOK)
			{
				return YSERR;
			}
			nSubScnWritten++;
		}


printf("%s %d\n",__FUNCTION__,__LINE__);
		shl=NULL;
		while((shl=FindNextShell(shl))!=NULL)
		{
printf("%s %d\n",__FUNCTION__,__LINE__);
			textOut.Printf("SRF\n");
			shl->dat.WriteCommonData(textOut);
			if(0<shl->dat.collShl.GetNumPolygon())
			{
				textOut.Printf("COLL %s\n",shl->dat.collFName.c_str());
			}
			textOut.Printf("END\n");
printf("%s %d\n",__FUNCTION__,__LINE__);
		}
printf("%s %d\n",__FUNCTION__,__LINE__);


		evg=NULL;
		while((evg=FindNextElevationGrid(evg))!=NULL)
		{
			textOut.Printf("TER\n");
			evg->dat.WriteCommonData(textOut);
			textOut.Printf("END\n");
		}


		drw=NULL;
		while((drw=FindNextMap(drw))!=NULL)
		{
			textOut.Printf("PC2\n");
			drw->dat.WriteCommonData(textOut);
			textOut.Printf("END\n");
		}


		drw=NULL;
		while((drw=FindNextSignBoard(drw))!=NULL)
		{
			textOut.Printf("PLT\n");
			drw->dat.WriteCommonData(textOut);
			textOut.Printf("END\n");
		}


		rgn=NULL;
		while((rgn=FindNextRectRegion(rgn))!=NULL)
		{
			textOut.Printf("RGN\n");
			textOut.Printf("ARE %s %s\n",rgn->dat.min.Txt(),rgn->dat.max.Txt());
			if(YsSceneryRectRegion::SUBCLASS_NONE!=rgn->dat.GetSubClassType())
			{
				textOut.Printf("SUB %s\n",YsSceneryRectRegion::GetSubClassTypeString(rgn->dat.GetSubClassType()));
			}
			rgn->dat.WriteCommonData(textOut);
			textOut.Printf("END\n");
		}


		pst=NULL;
		while((pst=FindNextPointSet(pst))!=NULL)
		{
			int i;
			textOut.Printf("PST\n");
			textOut.Printf("ISLOOP %s\n",(pst->dat.isLoop==YSTRUE ? "TRUE" : "FALSE"));
			if(pst->dat.areaType!=YSSCNAREA_NOAREA)
			{
				textOut.Printf("AREA %s\n",YsScenery::GetAreaTypeString(pst->dat.areaType));
			}
			for(i=0; i<pst->dat.pnt.GetN(); i++)
			{
				textOut.Printf("PNT %s\n",pst->dat.pnt[i].Txt("%.2lf %.2lf %.2lf"));
			}
			pst->dat.WriteCommonData(textOut);
			textOut.Printf("END\n");
		}


		gnd=NULL;
		while((gnd=gndList.FindNext(gnd))!=NULL)
		{
			textOut.Printf("GOB\n");
			gnd->dat.WriteCommonData(textOut);
			textOut.Printf("NAM %s\n",gnd->dat.objName.data());
			textOut.Printf("IFF %d\n",gnd->dat.iff);
			textOut.Printf("FLG %u\n",gnd->dat.gndFlag);
			if(gnd->dat.motionPathName.Strlen()>0)
			{
				textOut.Printf("MPN \"%s\"\n",gnd->dat.motionPathName.data());
			}
			if(gnd->dat.motionPathOffset==YSTRUE)
			{
				textOut.Printf("MPS\n");
			}
			if(gnd->dat.primaryTarget==YSTRUE)
			{
				textOut.Printf("PMT\n");
			}
			textOut.Printf("END\n");
		}


		air=NULL;
		while(NULL!=(air=airList.FindNext(air)))
		{
			textOut.Printf("AOB\n");
			air->dat.WriteCommonData(textOut);
			textOut.Printf("NAM %s\n",(const char *)air->dat.objName);
			textOut.Printf("IFF %d\n",air->dat.iff);
			textOut.Printf("FLG %u\n",air->dat.airFlag);
			textOut.Printf("FUEL %d%%\n",air->dat.fuel);
			textOut.Printf("SPEED %.2lfm/s\n",air->dat.speed);
			textOut.Printf("LOWFUEL %.2lf\n",air->dat.landWhenLowFuel);

			if(air->dat.GetAction().actType!=YsSceneryAirAction::DONOTHING)
			{
				int action;
				action=(int)air->dat.GetAction().actType;
				textOut.Printf("ACTION %s",airActionKeyWordSource[action]);
				switch(action)
				{
				case YsSceneryAirAction::FLYAIRROUTE:
					textOut.Printf(" \"%s\"",air->dat.GetAction().actParam[0].Txt());
					break;
				}
				// Future place for action parameters
				textOut.Printf("\n");
			}

			if(air->dat.ldg==YSTRUE)
			{
				textOut.Printf("CMD CTLLDGEA TRUE\n");
			}
			else
			{
				textOut.Printf("CMD CTLLDGEA FALSE\n");
			}
			textOut.Printf("END\n");
		}


		int nFldWritten=0;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			textOut.Printf("FLD\n");
			scn->dat.WriteCommonData(textOut);
			textOut.Printf("END\n");
			nFldWritten++;
		}

		if(nSubScnWritten!=nFldWritten)
		{
			lastErrorStr.Printf("nScn(%d) and nFld(%d) doesn't match",nSubScnWritten,nFldWritten);
			return YSERR;
		}

		return YSOK;
	}
	// Unreachable return YSERR;
}

YSRESULT YsScenery::MergePackedFile(YsTextOutputStream &textOut,const char packName[],const YsTextMemoryOutputStream &tmpfn)
{
	const int nLine=(int)tmpfn.GetNumLine();

	textOut.Printf("PCK \"%s\" %d\n",packName,nLine);

	for(auto &str : tmpfn)
	{
		textOut.Printf("%s\n",str.Txt());
	}
	textOut.Printf("\n\n");
	return YSOK;
}


