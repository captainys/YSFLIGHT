#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <ysclass.h>
#include <yscompilerwarning.h>

// test compile:
//   cl /c ysscenerygl2.0.cpp /I ..\..\ysgl\src /I ..\..\imported\include

#include "ysgl.h"

#include "ysscenery.h"

#include <ystexturemanager_gl.h>


namespace SetUpMatrix
{
	inline void SetDirect(const double glMat[16])
	{
		YsGLSLUse3DRenderer(YsGLSLSharedVariColor3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedVariColor3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedVariColor3DRenderer());

		YsGLSLUse3DRenderer(YsGLSLSharedFlat3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedFlat3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedFlat3DRenderer());

		YsGLSLUse3DRenderer(YsGLSLSharedVariColorShaded3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedVariColorShaded3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedVariColorShaded3DRenderer());

		YsGLSLUse3DRenderer(YsGLSLSharedMonoColorShaded3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedMonoColorShaded3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedMonoColorShaded3DRenderer());

		YsGLSLUse3DRenderer(YsGLSLSharedVariColorShadedWithTexCoord3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedVariColorShadedWithTexCoord3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedVariColorShadedWithTexCoord3DRenderer());

		YsGLSLUse3DRenderer(YsGLSLSharedMonoColorShadedWithTexCoord3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedMonoColorShadedWithTexCoord3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedMonoColorShadedWithTexCoord3DRenderer());


		YsGLSLUse3DRenderer(YsGLSLSharedFlash3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedFlash3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedFlash3DRenderer());

		YsGLSLUse3DRenderer(YsGLSLSharedFlashByPointSprite3DRenderer());
		YsGLSLSet3DRendererModelViewdv(YsGLSLSharedFlashByPointSprite3DRenderer(),glMat);
		YsGLSLEndUse3DRenderer(YsGLSLSharedFlashByPointSprite3DRenderer());
	}
	inline void SetUp(const YsVec3 &localPos,const YsAtt3 &localAtt,const YsMatrix4x4 &baseTfm)
	{
		YsMatrix4x4 localTfm(baseTfm);
		localTfm.Multiply(localPos,localAtt);
		double glMat[16];
		localTfm.GetOpenGlCompatibleMatrix(glMat);

		SetDirect(glMat);
	}
	inline void SetUpLF(const YsVec3 &localPos,const YsAtt3 &localAtt,const YsMatrix4x4 &baseTfm)
	{
		YsMatrix4x4 localTfm(baseTfm);
		localTfm.Multiply(localPos,localAtt);
		double glMat[16];
		localTfm.GetOpenGlCompatibleMatrix(glMat);

		YsGLInvertZdv(glMat);

		SetDirect(glMat);
	}
};

YSBOOL ysScnGlUseRwLightTexture=YSFALSE;
GLuint ysScnGlRwLightTex=~(unsigned int)0;

YSBOOL ysScnGlUseMapTexture=YSFALSE;
GLuint ysScnGlMapTex=~(unsigned int)0;

int ysScnFontBitmapBase=-1;


void YsSceneryPrepareGroundTextureTile(void)
{
	unsigned char rgba[16*16*4];
	for(int i=0; i<256; i++)
	{
		rgba[i*4  ]=YsScenery::groundTileTexture[i];
		rgba[i*4+1]=YsScenery::groundTileTexture[i];
		rgba[i*4+2]=YsScenery::groundTileTexture[i];
		rgba[i*4+3]=255;
	}

	glGenTextures(1,&ysScnGlMapTex);
	glBindTexture(GL_TEXTURE_2D,ysScnGlMapTex);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	glTexImage2D
	    (GL_TEXTURE_2D,
	     0,
	     4,
	     16,
	     16,
	     0,
	     GL_RGBA,
	     GL_UNSIGNED_BYTE,
	     rgba);
}

// Bbx & Axis of 2D Drawing      Magenta
// Bbx & Axis of Elevation Grid  Cyan
// Bbx & Axis of Child Sscenary  DarkYellow
// Bbx & Axis of Shell           White
// Bbx & Axis of GndObj          Blue

/*
YSRESULT DoSomethingForEverything(YsScenery *scenary)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsScenery> *scn;

	drw=NULL;
	while((drw=scenary->FindNextMap(drw))!=NULL)
	{
	}

	shl=NULL;
	while((shl=scenary->FindNextShell(shl))!=NULL)
	{
	}

	evg=NULL;
	while((evg=scenary->FindNextElevationGrid(evg))!=NULL)
	{
	}

	drw=NULL;
	while((drw=scenary->FindNextSignBoard(drw))!=NULL)
	{
	}

	rgn=NULL;
	while((rgn=scenary->FindNextRectRegion(rgn))!=NULL)
	{
	}

	scn=NULL;
	while((scn=scenary->FindNextChildScenery(scn))!=NULL)
	{
		DoSomethingForEverything(&scn->dat);
	}

	return YSOK;
}
*/



void YsGlDrawAxis(const double & /*axsSize*/)
{
}

static const YsColor YsScaleColor(const YsColor &col,const double &)
{
	// 2012/12/14 Ignore scaling.  Map color will be calculated by lighting.
	//YsColor newCol;
	//newCol.SetDoubleRGB(col.Rd()*scl,col.Gd()*scl,col.Bd()*scl);
	//return newCol;
	return col;
}

////////////////////////////////////////////////////////////


class YsSceneryGraphicCachePrimitiveTemplate
{
public:
	YsArray <GLfloat> vtxArray,texCoordArray,nomArray,colArray;
	YsArray <GLfloat> flashVtxArray,flashColArray,flashPointSizeArray;
	YsArray <GLfloat> protectPlgVtxArray;

	GLuint vboId,vtxPtr,nomPtr,colPtr,flashVtxPtr,flashColPtr,flashPointSizePtr,texCoordPtr,protectPlgPtr;

	YsSceneryGraphicCachePrimitiveTemplate()
	{
		vboId=0;
	}
	~YsSceneryGraphicCachePrimitiveTemplate()
	{
		if(0<vboId)
		{
			glDeleteBuffers(1,&vboId);
			vboId=0;
		}
	}

	inline void CleanUp(void)
	{
		vtxArray.Clear();
		texCoordArray.Clear();
		nomArray.Clear();
		colArray.Clear();
		flashVtxArray.Clear();
		flashPointSizeArray.Clear();
		flashColArray.Clear();
		protectPlgVtxArray.CleanUp();
	}
	inline void AddPoint(const YsVec3 &pnt,const YsVec3 &nom,const YsColor &col)
	{
		AddPoint(pnt);
		AddNormal(nom);
		AddColor(col);
	}
	inline void AddPoint(const YsVec3 &pnt)
	{
		vtxArray.Append((GLfloat)pnt.x());
		vtxArray.Append((GLfloat)pnt.y());
		vtxArray.Append((GLfloat)pnt.z());
	}
	inline void AddPoint(const YsVec2 &pnt,YSBOOL mapMode)
	{
		if(mapMode==YSTRUE)
		{
			vtxArray.Append((GLfloat)pnt.x());
			vtxArray.Append(0.0f);
			vtxArray.Append((GLfloat)pnt.y());

			nomArray.Append(0.0f);
			nomArray.Append(1.0f);
			nomArray.Append(0.0f);
		}
		else
		{
			vtxArray.Append((GLfloat)pnt.x());
			vtxArray.Append((GLfloat)pnt.y());
			vtxArray.Append(0.0f);

			nomArray.Append(0.0f);
			nomArray.Append(0.0f);
			nomArray.Append(-1.0f);
		}
	}
	inline void AddNormal(const YsVec3 &nom)
	{
		nomArray.Append((GLfloat)nom.x());
		nomArray.Append((GLfloat)nom.y());
		nomArray.Append((GLfloat)nom.z());
	}
	inline void AddTexCoord(const YsVec2 &pnt)
	{
		texCoordArray.Append((GLfloat)pnt.x());
		texCoordArray.Append((GLfloat)pnt.y());
	}
	inline void AddFlashVtxPoint(const YsVec2 &pnt,YSBOOL mapMode)
	{
		if(mapMode==YSTRUE)
		{
			flashVtxArray.Append((GLfloat)pnt.x());
			flashVtxArray.Append(0.0f);
			flashVtxArray.Append((GLfloat)pnt.y());
		}
		else
		{
			flashVtxArray.Append((GLfloat)pnt.x());
			flashVtxArray.Append((GLfloat)pnt.y());
			flashVtxArray.Append(0.0f);
		}
	}
	inline void AddFlashVtxColor(const YsColor &col)
	{
		flashColArray.Append(col.Rf());
		flashColArray.Append(col.Gf());
		flashColArray.Append(col.Bf());
		flashColArray.Append(1.0f);
	}
	inline void AddFlashPointSize(const GLfloat pointSize)
	{
		flashPointSizeArray.Append(pointSize);
	}
	inline void AddColor(const YsColor &col)
	{
		colArray.Append((GLfloat)col.Rd());
		colArray.Append((GLfloat)col.Gd());
		colArray.Append((GLfloat)col.Bd());
		colArray.Append(1.0f);
	}
	inline void AddProtectTri(const YsVec3 &p1,const YsVec3 &p2,const YsVec3 &p3)
	{
		protectPlgVtxArray.Append((GLfloat)p1.x());
		protectPlgVtxArray.Append((GLfloat)p1.y());
		protectPlgVtxArray.Append((GLfloat)p1.z());

		protectPlgVtxArray.Append((GLfloat)p2.x());
		protectPlgVtxArray.Append((GLfloat)p2.y());
		protectPlgVtxArray.Append((GLfloat)p2.z());

		protectPlgVtxArray.Append((GLfloat)p3.x());
		protectPlgVtxArray.Append((GLfloat)p3.y());
		protectPlgVtxArray.Append((GLfloat)p3.z());
	}
	inline void AddProtectQuad(const YsVec3 &p1,const YsVec3 &p2,const YsVec3 &p3,const YsVec3 &p4)
	{
		protectPlgVtxArray.Append((GLfloat)p1.x());
		protectPlgVtxArray.Append((GLfloat)p1.y());
		protectPlgVtxArray.Append((GLfloat)p1.z());

		protectPlgVtxArray.Append((GLfloat)p2.x());
		protectPlgVtxArray.Append((GLfloat)p2.y());
		protectPlgVtxArray.Append((GLfloat)p2.z());

		protectPlgVtxArray.Append((GLfloat)p3.x());
		protectPlgVtxArray.Append((GLfloat)p3.y());
		protectPlgVtxArray.Append((GLfloat)p3.z());

		protectPlgVtxArray.Append((GLfloat)p3.x());
		protectPlgVtxArray.Append((GLfloat)p3.y());
		protectPlgVtxArray.Append((GLfloat)p3.z());

		protectPlgVtxArray.Append((GLfloat)p4.x());
		protectPlgVtxArray.Append((GLfloat)p4.y());
		protectPlgVtxArray.Append((GLfloat)p4.z());

		protectPlgVtxArray.Append((GLfloat)p1.x());
		protectPlgVtxArray.Append((GLfloat)p1.y());
		protectPlgVtxArray.Append((GLfloat)p1.z());
	}


	void DrawPrimitiveVtx(YsGLSL3DRenderer *renderer,GLenum primType)
	{
		if(0<vboId)
		{
			glBindBuffer(GL_ARRAY_BUFFER,vboId);
			YsGLSLDrawPrimitiveVtxfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    (GLfloat *)vtxPtr);
			glBindBuffer(GL_ARRAY_BUFFER,0);
		}
		else
		{
			YsGLSLDrawPrimitiveVtxfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    vtxArray);
		}
	}
	void DrawPrimitiveVtxNomfv(YsGLSL3DRenderer *renderer,GLenum primType)
	{
		if(0<vboId)
		{
			glBindBuffer(GL_ARRAY_BUFFER,vboId);
			YsGLSLDrawPrimitiveVtxNomfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    (GLfloat *)((YSGLSL_POINTER_CAST)vtxPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)nomPtr));
			glBindBuffer(GL_ARRAY_BUFFER,0);
		}
		else
		{
			YsGLSLDrawPrimitiveVtxNomfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    vtxArray,
			    nomArray);
		}
	}
	void DrawPrimitiveVtxNomColfv(YsGLSL3DRenderer *renderer,GLenum primType)
	{
		if(0<vboId)
		{
			glBindBuffer(GL_ARRAY_BUFFER,vboId);
			YsGLSLDrawPrimitiveVtxNomColfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    (GLfloat *)((YSGLSL_POINTER_CAST)vtxPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)nomPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)colPtr));
			glBindBuffer(GL_ARRAY_BUFFER,0);
		}
		else
		{
			YsGLSLDrawPrimitiveVtxNomColfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    vtxArray,
			    nomArray,
			    colArray);
		}
	}

	void DrawPrimitiveVtxTexCoordNomfv(YsGLSL3DRenderer *renderer,GLenum primType)
	{
		if(0<vboId)
		{
			glBindBuffer(GL_ARRAY_BUFFER,vboId);
			YsGLSLDrawPrimitiveVtxTexCoordNomfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    (GLfloat *)((YSGLSL_POINTER_CAST)vtxPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)texCoordPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)nomPtr));
			glBindBuffer(GL_ARRAY_BUFFER,0);
		}
		else
		{
			YsGLSLDrawPrimitiveVtxTexCoordNomfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    vtxArray,
			    texCoordArray,
			    nomArray);
		}
	}
	void DrawPrimitiveVtxTexCoordNomColfv(YsGLSL3DRenderer *renderer,GLenum primType)
	{
		if(0<vboId)
		{
			glBindBuffer(GL_ARRAY_BUFFER,vboId);
			YsGLSLDrawPrimitiveVtxTexCoordNomColfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    (GLfloat *)((YSGLSL_POINTER_CAST)vtxPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)texCoordPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)nomPtr),
			    (GLfloat *)((YSGLSL_POINTER_CAST)colPtr));
			glBindBuffer(GL_ARRAY_BUFFER,0);
		}
		else
		{
			YsGLSLDrawPrimitiveVtxTexCoordNomColfv(
			    renderer,
			    primType,
			    (int)vtxArray.GetN()/3,
			    vtxArray,
			    texCoordArray,
			    nomArray,
			    colArray);
		}
	}

	void MakeVbo(void)
	{
		if(0<vboId)
		{
			glDeleteBuffers(1,&vboId);
		}

		glGenBuffers(1,&vboId);

		glBindBuffer(GL_ARRAY_BUFFER,vboId);

		YSSIZE_T totalBufSize=sizeof(GLfloat)*
		    (vtxArray.GetN()+texCoordArray.GetN()+nomArray.GetN()+colArray.GetN()
		    +flashVtxArray.GetN()+flashColArray.GetN()+flashPointSizeArray.GetN()+protectPlgVtxArray.GetN());

		glBufferData(GL_ARRAY_BUFFER,(int)totalBufSize,NULL,GL_STATIC_DRAW);

		unsigned int bufPtr=0;

		vtxPtr=bufPtr;
		glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*vtxArray.GetN(),vtxArray);
		bufPtr+=(int)sizeof(GLfloat)*(int)vtxArray.GetN();;

		texCoordPtr=bufPtr;
		if(0<texCoordArray.GetN())
		{
			glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*texCoordArray.GetN(),texCoordArray);
			bufPtr+=(int)sizeof(GLfloat)*(int)texCoordArray.GetN();
		}

		nomPtr=bufPtr;
		if(0<nomArray.GetN())
		{
			glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*nomArray.GetN(),nomArray);
			bufPtr+=(int)sizeof(GLfloat)*(int)nomArray.GetN();
		}

		colPtr=bufPtr;
		if(0<colArray.GetN())
		{
			glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*colArray.GetN(),colArray);
			bufPtr+=(int)sizeof(GLfloat)*(int)colArray.GetN();
		}

		flashVtxPtr=bufPtr;
		if(0<flashVtxArray.GetN())
		{
			glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*flashVtxArray.GetN(),flashVtxArray);
			bufPtr+=(int)sizeof(GLfloat)*(int)flashVtxArray.GetN();
		}

		flashColPtr=bufPtr;
		if(0<flashColArray.GetN())
		{
			glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*flashColArray.GetN(),flashColArray);
			bufPtr+=(int)sizeof(GLfloat)*(int)flashColArray.GetN();
		}

		flashPointSizePtr=bufPtr;
		if(0<flashPointSizeArray.GetN())
		{
			glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*flashPointSizeArray.GetN(),flashPointSizeArray);
			bufPtr+=(int)sizeof(GLfloat)*(int)flashPointSizeArray.GetN();
		}

		protectPlgPtr=bufPtr;
		if(0<protectPlgVtxArray.GetN())
		{
			glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*protectPlgVtxArray.GetN(),protectPlgVtxArray);
			bufPtr+=(int)sizeof(GLfloat)*(int)protectPlgVtxArray.GetN();
		}

		glBindBuffer(GL_ARRAY_BUFFER,0);
	}
};


////////////////////////////////////////////////////////////

static const int maxNumTessVtx=1024;
static GLdouble GlTessVtx[maxNumTessVtx*3];

class Ys2DDrawingGraphicCache
{
public:
	class Primitive : public YsSceneryGraphicCachePrimitiveTemplate
	{
	public:
		Ys2DDrawingElement::OBJTYPE objType;
		YsColor color;
		YSBOOL specular;
		GLint textureId;
		GLint filterType;

		Primitive()
		{
			textureId=0;
		}
	};

	double plgColorScale,linColorScale,pntColorScale;
	YsSegmentedArray <Primitive,4> primitiveArray;

	inline YSBOOL ColorScaleChanged(const double &plgColorScale,const double &linColorScale,const double &pntColorScale)
	{
		if(YSTRUE!=YsEqual(this->plgColorScale,plgColorScale) ||
		   YSTRUE!=YsEqual(this->linColorScale,linColorScale) ||
		   YSTRUE!=YsEqual(this->pntColorScale,pntColorScale))
		{
			return YSTRUE;
		}
		return YSFALSE;
	}
};

void Ys2DDrawing::AllocCache(void) const
{
	DeleteCache();
	graphicCache=new Ys2DDrawingGraphicCache;
}

void Ys2DDrawing::DeleteCache(void) const
{
	if(NULL!=graphicCache)
	{
		delete graphicCache;
		graphicCache=NULL;
	}
}

YSBOOL Ys2DDrawing::IsCached(void) const
{
	if(NULL!=graphicCache)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void Ys2DDrawing::MakeCache(const double &plgColorScale,const double &linColorScale,const double &pntColorScale,YSBOOL mapMode)
{
	AllocCache();

	printf("Making map cache! %s %d\n",__FUNCTION__,__LINE__);

	graphicCache->plgColorScale=plgColorScale;
	graphicCache->linColorScale=linColorScale;
	graphicCache->pntColorScale=pntColorScale;

	YsListItem <Ys2DDrawingElement> *itm=NULL;
	while(NULL!=(itm=elemList.FindNext(itm)))
	{
		switch(itm->dat.t)
		{
		default:
			goto NEXTOBJ;

		case Ys2DDrawingElement::APPROACHLIGHT:
		case Ys2DDrawingElement::POINTS:
			{
				graphicCache->primitiveArray.Increment();
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.GetEnd();
				primitive.objType=itm->dat.t;
				primitive.color=YsScaleColor(itm->dat.c,pntColorScale);
				primitive.specular=itm->dat.GetSpecular();
				primitive.vtxArray.Clear();
				primitive.flashVtxArray.Clear();
				primitive.flashPointSizeArray.Clear();
				primitive.colArray.Clear();

				for(YSSIZE_T i=0; i<itm->dat.pnt.GetN(); i++)
				{
					primitive.AddPoint(itm->dat.pnt[i],mapMode);

					primitive.AddFlashVtxPoint(itm->dat.pnt[i],mapMode);
					primitive.AddFlashVtxColor(primitive.color);
					primitive.AddFlashPointSize(1.0);
				}
			}
			break;

		case Ys2DDrawingElement::LINESEGMENTS:
		case Ys2DDrawingElement::LINES:
			{
				graphicCache->primitiveArray.Increment();
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.GetEnd();
				primitive.objType=itm->dat.t;
				primitive.specular=itm->dat.GetSpecular();
				primitive.vtxArray.Clear();
				primitive.colArray.Clear();

				switch(itm->dat.t)
				{
				default:
					break;
				case Ys2DDrawingElement::LINESEGMENTS:
					primitive.color=YsScaleColor(itm->dat.c,linColorScale);
					break;
				case Ys2DDrawingElement::LINES:
					primitive.color=YsScaleColor(itm->dat.c,linColorScale);
					break;
				}

				for(YSSIZE_T i=0; i<itm->dat.pnt.GetN(); i++)
				{
					primitive.AddPoint(itm->dat.pnt[i],mapMode);
				}
			}
			break;
		case Ys2DDrawingElement::POLYGON:
			{
				const Ys2DDrawingElement::OBJTYPE objType=Ys2DDrawingElement::TRIANGLES; // Must be converted to all triangles in the de-evlolved world of OpenGL ES and Direct 3D.
				const YsColor color=YsScaleColor(itm->dat.c,plgColorScale);
				if(0==graphicCache->primitiveArray.GetN() ||
				   graphicCache->primitiveArray.Last().objType!=objType ||
				   graphicCache->primitiveArray.Last().color!=color ||
				   graphicCache->primitiveArray.Last().specular!=itm->dat.GetSpecular())
				{
					graphicCache->primitiveArray.Increment();
					graphicCache->primitiveArray.Last().objType=objType;
					graphicCache->primitiveArray.Last().color=color;;
					graphicCache->primitiveArray.Last().specular=itm->dat.GetSpecular();
					graphicCache->primitiveArray.Last().vtxArray.Clear();
					graphicCache->primitiveArray.Last().texCoordArray.Clear();
					graphicCache->primitiveArray.Last().colArray.Clear();
				}
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.Last();

				if(itm->dat.cvx==YSTRUE)
				{
					for(int i=1; i<itm->dat.pnt.GetN()-1; i++)
					{
						primitive.AddPoint(itm->dat.pnt[0  ],mapMode);
						primitive.AddPoint(itm->dat.pnt[i  ],mapMode);
						primitive.AddPoint(itm->dat.pnt[i+1],mapMode);
					}

					if(itm->dat.pnt.GetN()==itm->dat.texCoord.GetN())
					{
						for(int i=1; i<itm->dat.texCoord.GetN(); ++i)
						{
							primitive.AddTexCoord(itm->dat.texCoord[0  ]);
							primitive.AddTexCoord(itm->dat.texCoord[i  ]);
							primitive.AddTexCoord(itm->dat.texCoord[i+1]);
						}
					}
				}
				else
				{
					YsShell2dTessellator tess;
					YsArray <YsShell2dVertexHandle> v2HdArray;
					tess.SetDomain(v2HdArray,itm->dat.pnt.GetN(),itm->dat.pnt);

					YsHashTable <YSSIZE_T> v2KeyToPntIdx;
					for(YSSIZE_T idx=0; idx<v2HdArray.GetN(); ++idx)
					{
						v2KeyToPntIdx.AddElement(tess.GetSearchKey(v2HdArray[idx]),idx);
					}

					for(;;)
					{
						YSBOOL repeat;
						YsShell2dEdgeHandle edHd;
						tess.GetShell2d().RewindEdgePtr();
						repeat=YSFALSE;
						while(NULL!=(edHd=tess.GetShell2d().StepEdgePtr()))
						{
							if(tess.RemoveEdge(edHd,100,YSFALSE)==YSOK)
							{
								repeat=YSTRUE;
							}
						}
						if(repeat!=YSTRUE)
						{
							break;
						}
					}

					YsListItem <YsShell2dTessTriangle> *ptr;
					tess.triList.RewindPointer();
					while(NULL!=(ptr=tess.triList.StepPointer()))
					{
						YSSIZE_T pntIdx[3];
						v2KeyToPntIdx.FindElement(pntIdx[0],tess.GetSearchKey(ptr->dat.trVtHd[0]));
						v2KeyToPntIdx.FindElement(pntIdx[1],tess.GetSearchKey(ptr->dat.trVtHd[1]));
						v2KeyToPntIdx.FindElement(pntIdx[2],tess.GetSearchKey(ptr->dat.trVtHd[2]));

						primitive.AddPoint(itm->dat.pnt[pntIdx[0]],mapMode);
						primitive.AddPoint(itm->dat.pnt[pntIdx[1]],mapMode);
						primitive.AddPoint(itm->dat.pnt[pntIdx[2]],mapMode);

						if(itm->dat.pnt.GetN()==itm->dat.texCoord.GetN())
						{
							primitive.AddTexCoord(itm->dat.texCoord[pntIdx[0]]);
							primitive.AddTexCoord(itm->dat.texCoord[pntIdx[1]]);
							primitive.AddTexCoord(itm->dat.texCoord[pntIdx[2]]);
						}
					}
				}
			}
			break;
		case Ys2DDrawingElement::GRADATIONQUADSTRIP:
			{
				const Ys2DDrawingElement::OBJTYPE objType=itm->dat.t;
				const YsColor color=YsScaleColor(itm->dat.c,plgColorScale);

				graphicCache->primitiveArray.Increment();
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.GetEnd();
				primitive.objType=objType;
				primitive.color=color;
				primitive.specular=itm->dat.GetSpecular();
				primitive.vtxArray.Clear();
				primitive.texCoordArray.Clear();
				primitive.colArray.Clear();

				const YsColor altColor[2]={itm->dat.c,itm->dat.c2};

				for(int i=0; i<itm->dat.pnt.GetN(); i++)
				{
					primitive.AddColor(altColor[i&1]);
					primitive.AddPoint(itm->dat.pnt[i],mapMode);
					if(itm->dat.texCoord.GetN()==itm->dat.pnt.GetN())
					{
						primitive.AddTexCoord(itm->dat.texCoord[i]);
					}
				}
			}
			break;
		case Ys2DDrawingElement::QUADSTRIP:
			{
				const Ys2DDrawingElement::OBJTYPE objType=itm->dat.t;
				const YsColor color=YsScaleColor(itm->dat.c,plgColorScale);

				graphicCache->primitiveArray.Increment();
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.GetEnd();
				primitive.objType=objType;
				primitive.color=color;
				primitive.specular=itm->dat.GetSpecular();
				primitive.vtxArray.Clear();
				primitive.texCoordArray.CleanUp();
				primitive.colArray.Clear();

				for(YSSIZE_T i=0; i<itm->dat.pnt.GetN(); i++)
				{
					primitive.AddPoint(itm->dat.pnt[i],mapMode);
					if(itm->dat.texCoord.GetN()==itm->dat.pnt.GetN())
					{
						primitive.AddTexCoord(itm->dat.texCoord[i]);
					}
				}
			}
			break;
		case Ys2DDrawingElement::QUADS:
			{
				const Ys2DDrawingElement::OBJTYPE objType=Ys2DDrawingElement::TRIANGLES; // Must be converted to all triangles in the de-evlolved world of OpenGL ES and Direct 3D.
				const YsColor color=YsScaleColor(itm->dat.c,plgColorScale);
				if(0==graphicCache->primitiveArray.GetN() ||
				   graphicCache->primitiveArray.Last().objType!=objType ||
				   graphicCache->primitiveArray.Last().color!=color ||
				   graphicCache->primitiveArray.Last().specular!=itm->dat.GetSpecular())
				{
					graphicCache->primitiveArray.Increment();
					graphicCache->primitiveArray.Last().objType=objType;
					graphicCache->primitiveArray.Last().color=color;
					graphicCache->primitiveArray.Last().specular=itm->dat.GetSpecular();
					graphicCache->primitiveArray.Last().vtxArray.Clear();
					graphicCache->primitiveArray.Last().texCoordArray.Clear();
					graphicCache->primitiveArray.Last().colArray.Clear();
				}
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.Last();

				for(YSSIZE_T i=0; i<=itm->dat.pnt.GetN()-4; i+=4)
				{
					primitive.AddPoint(itm->dat.pnt[i  ],mapMode);
					primitive.AddPoint(itm->dat.pnt[i+1],mapMode);
					primitive.AddPoint(itm->dat.pnt[i+2],mapMode);

					primitive.AddPoint(itm->dat.pnt[i+2],mapMode);
					primitive.AddPoint(itm->dat.pnt[i+3],mapMode);
					primitive.AddPoint(itm->dat.pnt[i  ],mapMode);

					if(itm->dat.texCoord.GetN()==itm->dat.pnt.GetN())
					{
						primitive.AddTexCoord(itm->dat.texCoord[i  ]);
						primitive.AddTexCoord(itm->dat.texCoord[i+1]);
						primitive.AddTexCoord(itm->dat.texCoord[i+2]);

						primitive.AddTexCoord(itm->dat.texCoord[i+2]);
						primitive.AddTexCoord(itm->dat.texCoord[i+3]);
						primitive.AddTexCoord(itm->dat.texCoord[i  ]);
					}
				}
			}
			break;

		case Ys2DDrawingElement::TRIANGLES:
			{
				const Ys2DDrawingElement::OBJTYPE objType=itm->dat.t;
				const YsColor color=YsScaleColor(itm->dat.c,plgColorScale);
				if(0==graphicCache->primitiveArray.GetN() ||
				   graphicCache->primitiveArray.Last().objType!=objType ||
				   graphicCache->primitiveArray.Last().specular!=itm->dat.GetSpecular() ||
				   graphicCache->primitiveArray.Last().color!=color)
				{
					graphicCache->primitiveArray.Increment();
					graphicCache->primitiveArray.Last().objType=objType;
					graphicCache->primitiveArray.Last().color=color;
					graphicCache->primitiveArray.Last().specular=itm->dat.GetSpecular();
					graphicCache->primitiveArray.Last().vtxArray.Clear();
					graphicCache->primitiveArray.Last().texCoordArray.Clear();
					graphicCache->primitiveArray.Last().colArray.Clear();
				}
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.Last();

				for(YSSIZE_T i=0; i<=itm->dat.pnt.GetN()-3; i+=3)
				{
					primitive.AddPoint(itm->dat.pnt[i  ],mapMode);
					primitive.AddPoint(itm->dat.pnt[i+1],mapMode);
					primitive.AddPoint(itm->dat.pnt[i+2],mapMode);

					if(itm->dat.texCoord.GetN()==itm->dat.pnt.GetN())
					{
						primitive.AddTexCoord(itm->dat.texCoord[i  ]);
						primitive.AddTexCoord(itm->dat.texCoord[i+1]);
						primitive.AddTexCoord(itm->dat.texCoord[i+2]);
					}
				}
			}
			break;
		}
	NEXTOBJ:
		if(0<itm->dat.texLabel.Strlen() && NULL!=owner)
		{
			auto scn=owner->GetOwner();
			auto texHd=scn->textureManager.FindTextureFromLabel(itm->dat.texLabel);
			if(NULL!=texHd)
			{
				itm->dat.texManCache=&scn->textureManager;
				itm->dat.texHdCache=texHd;
			}
		}
		if(NULL!=itm->dat.texManCache && NULL!=itm->dat.texHdCache)
		{
			auto tex=itm->dat.texManCache->GetTexture(itm->dat.texHdCache);
			if(YSTRUE!=tex->IsActualTextureReady())
			{
				tex->MakeActualTexture();
			}
			if(YSTRUE==tex->IsActualTextureReady())
			{
				Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray.Last();
				primitive.textureId=tex->GetActualTexture()->texId;
				if(YsTextureManager::Unit::FILTERTYPE_NEAREST==tex->GetFilterType())
				{
					primitive.filterType=GL_NEAREST;
				}
				else
				{
					primitive.filterType=GL_LINEAR;
				}
			}
		}
	}

	for(auto &prim : graphicCache->primitiveArray)
	{
		prim.MakeVbo();
	}

	printf("Making map cache! %s %d\n",__FUNCTION__,__LINE__);
}

void Ys2DDrawing::Draw(
    const double &plgColorScale,const double &linColorScale,const double &pntColorScale,
    YSBOOL drawPset,
    YSBOOL mapMode,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL name2DElem,
    const double &currentTime,
    YsMatrix4x4 * /*viewModelTfm*/ )
{
	YsDisregardVariable(currentTime);
	YsDisregardVariable(fill);
	YsDisregardVariable(wire);
	YsDisregardVariable(name2DElem);

	if(YSTRUE==IsCached() && YSTRUE==graphicCache->ColorScaleChanged(plgColorScale,linColorScale,pntColorScale))
	{
		DeleteCache();
	}

	if(YSTRUE!=IsCached())
	{
		MakeCache(plgColorScale,linColorScale,pntColorScale,mapMode);
	}


	// if(ysScnGlUseRwLightTexture==YSTRUE && mapMode==YSTRUE)
	// Use flash renderer


	// Approach light: needs to be drawn on the fly
	// Ground texture needs to be turned on and off, anyway my YSGL library doesn't support it yet.
	// GL_POINTS must not be drawn when drawPset is off.

	// Homework: For future use in the Scenery Editor, support wireframe mode.


	if(YSTRUE==IsCached())
	{
		YsGLSL3DRenderer *currentRenderer=NULL;

		for(YSSIZE_T i=0; i<graphicCache->primitiveArray.GetN(); ++i)
		{
			YsGLSL3DRenderer *renderer=NULL;
			GLenum glPrimitive=GL_POINTS;

			if(YSTRUE!=drawPset && 
			   (graphicCache->primitiveArray[i].objType==Ys2DDrawingElement::POINTS || 
			    graphicCache->primitiveArray[i].objType==Ys2DDrawingElement::APPROACHLIGHT))
			{
				continue;
			}

			YSBOOL useOwnTexture=(0<graphicCache->primitiveArray[i].textureId ? YSTRUE : YSFALSE);
			YSBOOL variColor=YSFALSE;
			int rendererType=YSGLSL_RENDERER_TYPE_NONE;

			switch(graphicCache->primitiveArray[i].objType)
			{
			case Ys2DDrawingElement::POINTS:
			case Ys2DDrawingElement::APPROACHLIGHT:
				renderer=YsGLSLSharedFlat3DRenderer();
				glPrimitive=GL_POINTS;  // Will be replaced with YsGLSLFlash3DRenderer
				variColor=YSFALSE;
				rendererType=YSGLSL_RENDERER_TYPE_PLAIN3D;
				break;

			case Ys2DDrawingElement::LINESEGMENTS:
				renderer=YsGLSLSharedMonoColorShaded3DRenderer();
				glPrimitive=GL_LINE_STRIP;
				variColor=YSFALSE;
				rendererType=YSGLSL_RENDERER_TYPE_SHADED3D;
				break;

			case Ys2DDrawingElement::LINES:
				renderer=YsGLSLSharedMonoColorShaded3DRenderer();
				glPrimitive=GL_LINES;
				variColor=YSFALSE;
				rendererType=YSGLSL_RENDERER_TYPE_SHADED3D;
				break;

			case Ys2DDrawingElement::POLYGON:
			case Ys2DDrawingElement::QUADS:
			case Ys2DDrawingElement::TRIANGLES:
				if(YSTRUE!=useOwnTexture)
				{
					renderer=YsGLSLSharedMonoColorShaded3DRenderer();
					variColor=YSFALSE;
					rendererType=YSGLSL_RENDERER_TYPE_SHADED3D;
				}
				else
				{
					renderer=YsGLSLSharedMonoColorShadedWithTexCoord3DRenderer();
					variColor=YSFALSE;
					rendererType=YSGLSL_RENDERER_TYPE_SHADED3D;
				}
				glPrimitive=GL_TRIANGLES;
				break;

			case Ys2DDrawingElement::QUADSTRIP:
				if(YSTRUE!=useOwnTexture)
				{
					renderer=YsGLSLSharedMonoColorShaded3DRenderer();
					variColor=YSFALSE;
					rendererType=YSGLSL_RENDERER_TYPE_SHADED3D;
				}
				else
				{
					renderer=YsGLSLSharedMonoColorShadedWithTexCoord3DRenderer();
					variColor=YSFALSE;
					rendererType=YSGLSL_RENDERER_TYPE_SHADED3D;
				}
				glPrimitive=GL_TRIANGLE_STRIP;
				break;

			case Ys2DDrawingElement::GRADATIONQUADSTRIP:
				renderer=YsGLSLSharedVariColorShaded3DRenderer();
				glPrimitive=GL_TRIANGLE_STRIP;
				variColor=YSTRUE;
				rendererType=YSGLSL_RENDERER_TYPE_SHADED3D;
				break;
			}

			if(NULL!=renderer)
			{
				if(currentRenderer!=renderer)
				{
					YsGLSLEndUse3DRenderer(currentRenderer);
					YsGLSLUse3DRenderer(renderer);
					currentRenderer=renderer;
				}

				const Ys2DDrawingGraphicCache::Primitive &primitive=graphicCache->primitiveArray[i];


				GLfloat savedSpecular[3];
				YsGLSLGet3DRendererSpecularColor(savedSpecular,renderer);
				const GLfloat specularOn[3]={1.0F,1.0F,1.0F};
				const GLfloat specularOff[3]={0.0F,0.0F,0.0F};
				YsGLSLSet3DRendererSpecularColor(renderer,(YSTRUE==primitive.specular ? specularOn : specularOff));


				if(/*rendererType==YSGLSL_RENDERER_TYPE_SHADED3D && */YSTRUE!=useOwnTexture)
				{
					glActiveTexture(GL_TEXTURE0);
					if(nullptr!=YsScenery::commonTexManPtr && YSTRUE==YsScenery::commonTexManPtr->IsReady(YsScenery::commonGroundTexHd))
					{
						auto unitPtr=YsScenery::commonTexManPtr->GetTexture(YsScenery::commonGroundTexHd);
						unitPtr->Bind();
						YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_TILING);
						YsGLSLSet3DRendererTextureIdentifier(renderer,0);
					}
					else
					{
						YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_NONE);
					}
				}
				else if(/*rendererType==YSGLSL_RENDERER_TYPE_SHADED3D && */YSTRUE==useOwnTexture)
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D,primitive.textureId);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,primitive.filterType);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,primitive.filterType);
					YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_ATTRIBUTE);
					YsGLSLSet3DRendererTextureIdentifier(renderer,0);
				}

				if(YSTRUE!=variColor)
				{
					GLfloat uniformColor[4]=
					{
						(GLfloat)primitive.color.Rd(),
						(GLfloat)primitive.color.Gd(),
						(GLfloat)primitive.color.Bd(),
						1.0f
					};
					YsGLSLSet3DRendererUniformColorfv(renderer,uniformColor);
				}

				if(rendererType==YSGLSL_RENDERER_TYPE_PLAIN3D && YSTRUE!=useOwnTexture && YSTRUE!=variColor)
				{
					if(graphicCache->primitiveArray[i].objType!=Ys2DDrawingElement::APPROACHLIGHT)
					{
						graphicCache->primitiveArray[i].DrawPrimitiveVtx(renderer,glPrimitive);
					}
					else
					{
						if(currentTime>=0.0)
						{
							const double t=fmod(currentTime,0.5)/0.5;
							const int nPnt=(int)primitive.flashVtxArray.GetN()/9;
							if(nPnt>2)
							{
								int idx=(int)((double)nPnt*t);
								idx=YsBound <int> (idx,0,(int)nPnt-1);

								const GLfloat *vtx=primitive.vtxArray.GetArray()+idx*3;
								YsGLSLDrawPrimitiveVtxfv(renderer,glPrimitive,1,vtx);
							}
						}
					}
				}
				else if(rendererType==YSGLSL_RENDERER_TYPE_SHADED3D && YSTRUE!=useOwnTexture && YSTRUE!=variColor)
				{
					graphicCache->primitiveArray[i].DrawPrimitiveVtxNomfv(renderer,glPrimitive);
				}
				else if(rendererType==YSGLSL_RENDERER_TYPE_SHADED3D && YSTRUE!=useOwnTexture && YSTRUE==variColor)
				{
					graphicCache->primitiveArray[i].DrawPrimitiveVtxNomColfv(renderer,glPrimitive);
				}
				else if(rendererType==YSGLSL_RENDERER_TYPE_SHADED3D && YSTRUE==useOwnTexture && YSTRUE!=variColor)
				{
					YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_ATTRIBUTE);
					graphicCache->primitiveArray[i].DrawPrimitiveVtxTexCoordNomfv(renderer,glPrimitive);
					YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_NONE);
				}
				else if(rendererType==YSGLSL_RENDERER_TYPE_SHADED3D && YSTRUE==useOwnTexture && YSTRUE==variColor)
				{
					YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_ATTRIBUTE);
					graphicCache->primitiveArray[i].DrawPrimitiveVtxTexCoordNomColfv(renderer,glPrimitive);
					YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_NONE);
				}

				if(graphicCache->primitiveArray[i].objType==Ys2DDrawingElement::POINTS)
				{
					renderer=YsGLSLSharedFlashByPointSprite3DRenderer();
					YsGLSLEndUse3DRenderer(currentRenderer);
					YsGLSLUse3DRenderer(renderer);
					currentRenderer=renderer;

					int viewport[4];
					glGetIntegerv(GL_VIEWPORT,viewport);
					YsGLSLSet3DRendererViewportDimensionf(renderer,(float)viewport[2],(float)viewport[3]);
					YsGLSLSet3DRendererPointSizeMode(renderer,YSGLSL_POINTSPRITE_SIZE_IN_3DSPACE);

					YsGLSLSet3DRendererZOffsetEnabled(renderer,1);
					YsGLSLSet3DRendererZOffset(renderer,-0.01f);

#ifdef GL_PROGRAM_POINT_SIZE
					glEnable(GL_PROGRAM_POINT_SIZE);  // Needed for enabling gl_PointSize in the vertex shader.
					glEnable(GL_POINT_SPRITE);        // Needed for enabling gl_PointCoord in the fragment shader.
#endif

					YsGLSLDrawPrimitiveVtxColPointSizefv(renderer,GL_POINTS,(int)primitive.flashVtxArray.GetN()/3,primitive.flashVtxArray,primitive.flashColArray,primitive.flashPointSizeArray);

#ifdef GL_PROGRAM_POINT_SIZE
					glDisable(GL_PROGRAM_POINT_SIZE);  // Needed for enabling gl_PointSize in the vertex shader.
					glDisable(GL_POINT_SPRITE);        // Needed for enabling gl_PointCoord in the fragment shader.
#endif
				}
				else if(graphicCache->primitiveArray[i].objType==Ys2DDrawingElement::APPROACHLIGHT)
				{
					renderer=YsGLSLSharedFlashByPointSprite3DRenderer();
					YsGLSLEndUse3DRenderer(currentRenderer);
					YsGLSLUse3DRenderer(renderer);
					currentRenderer=renderer;

					int viewport[4];
					glGetIntegerv(GL_VIEWPORT,viewport);
					YsGLSLSet3DRendererViewportDimensionf(renderer,(float)viewport[2],(float)viewport[3]);
					YsGLSLSet3DRendererPointSizeMode(renderer,YSGLSL_POINTSPRITE_SIZE_IN_3DSPACE);

					YsGLSLSet3DRendererZOffsetEnabled(renderer,1);
					YsGLSLSet3DRendererZOffset(renderer,-0.01f);

#ifdef GL_PROGRAM_POINT_SIZE
					glEnable(GL_PROGRAM_POINT_SIZE);  // Needed for enabling gl_PointSize in the vertex shader.
					glEnable(GL_POINT_SPRITE);        // Needed for enabling gl_PointCoord in the fragment shader.
#endif

					if(currentTime>=0.0)
					{
						const double t=fmod(currentTime,0.5)/0.5;
						const int nPnt=(int)primitive.flashVtxArray.GetN()/3;
						if(nPnt>2)
						{
							int idx=(int)((double)nPnt*t);
							idx=YsBound <int> (idx,0,(int)nPnt-1);

							const GLfloat *vtx=primitive.flashVtxArray.GetArray()+idx*3;
							const GLfloat *col=primitive.flashColArray.GetArray()+idx*4;
							const GLfloat *psize=primitive.flashPointSizeArray.GetArray()+idx;
							YsGLSLDrawPrimitiveVtxColPointSizefv(renderer,GL_POINTS,1,vtx,col,psize);
						}
					}

#ifdef GL_PROGRAM_POINT_SIZE
					glDisable(GL_PROGRAM_POINT_SIZE);  // Needed for enabling gl_PointSize in the vertex shader.
					glDisable(GL_POINT_SPRITE);        // Needed for enabling gl_PointCoord in the fragment shader.
#endif
				}

				YsGLSLSet3DRendererSpecularColor(renderer,savedSpecular);
			}
		}
		YsGLSLEndUse3DRenderer(currentRenderer);

		if(drawBbx==YSTRUE)
		{
			DrawBoundingBox(mapMode);
		}
	}

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

void Ys2DDrawing::DrawBoundingBox(YSBOOL mapMode)
{
	YsDisregardVariable(mapMode);
/*	YsVec2 inflatedBbx[2],dgn,XZ;
	double l;
	dgn=bbx[1]-bbx[0];
	l=YsGreater(dgn.x(),dgn.y());
	XZ.Set(1.0,1.0);
	inflatedBbx[0]=bbx[0]-l*0.025*XZ;
	inflatedBbx[1]=bbx[1]+l*0.025*XZ;

	glDisable(GL_LIGHTING);
	YsScenery::GlSetColor(YsMagenta());
	if(mapMode==YSTRUE)
	{
		glBegin(GL_LINE_LOOP);
		glVertex3d(inflatedBbx[0].x(),0.0,inflatedBbx[0].y());
		glVertex3d(inflatedBbx[0].x(),0.0,inflatedBbx[1].y());
		glVertex3d(inflatedBbx[1].x(),0.0,inflatedBbx[1].y());
		glVertex3d(inflatedBbx[1].x(),0.0,inflatedBbx[0].y());
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_LOOP);
		glVertex2d(inflatedBbx[0].x(),inflatedBbx[0].y());
		glVertex2d(inflatedBbx[0].x(),inflatedBbx[1].y());
		glVertex2d(inflatedBbx[1].x(),inflatedBbx[1].y());
		glVertex2d(inflatedBbx[1].x(),inflatedBbx[0].y());
		glEnd();
	} */
}

////////////////////////////////////////////////////////////

class YsElevationGridGraphicCache
{
public:
	double plgColorScale;
	class Primitive : public YsSceneryGraphicCachePrimitiveTemplate
	{
	};

	Primitive meshCache,wallCache[4],protectPlgCache;
};

void YsElevationGrid::AllocCache(void) const
{
	DeleteCache();
	graphicCache=new YsElevationGridGraphicCache;
}

void YsElevationGrid::DeleteCache(void) const
{
	if(NULL!=graphicCache)
	{
		delete graphicCache;
		graphicCache=NULL;
	}
}

YSBOOL YsElevationGrid::IsCached(void) const
{
	if(NULL!=graphicCache)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void YsElevationGrid::MakeCache(const double &plgColScale,YSBOOL invert)
{
	YsDisregardVariable(invert);

	AllocCache();
	graphicCache->plgColorScale=plgColScale;

	for(int j=0; j<nz; j++)
	{
		const double t0=(double)j/(double)nz;
		const double t1=(double)(j+1)/(double)nz;

		const double z=double(j)*zWid;
		double x=0.0;
		const int baseIdx=(nx+1)*j;
		YsVec3 rc[4],*tri[6];

		//  1  3
		//
		//  0  2
		rc[2].Set(x,node[baseIdx     ].y,z);
		rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
		for(int i=0; i<nx; i++)
		{
			const double s0=(double)i/(double)nx;
			const double s1=(double)(i+1)/(double)nx;

			rc[0]=rc[2];
			rc[1]=rc[3];
			rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
			rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

			const double texCoord[4][2]=
			{
				{s0,t0},{s0,t1},{s1,t0},{s1,t1}
			};
			YsVec2 triTexCoord[6];

			if(node[baseIdx+i].lup==YSTRUE)
			{
				// (3,1,2),(0,2,1)
				tri[0]=&rc[3];
				tri[1]=&rc[1];
				tri[2]=&rc[2];
				tri[3]=&rc[0];
				tri[4]=&rc[2];
				tri[5]=&rc[1];

				triTexCoord[0].Set(texCoord[3][0],texCoord[3][1]);
				triTexCoord[1].Set(texCoord[1][0],texCoord[1][1]);
				triTexCoord[2].Set(texCoord[2][0],texCoord[2][1]);
				triTexCoord[3].Set(texCoord[0][0],texCoord[0][1]);
				triTexCoord[4].Set(texCoord[2][0],texCoord[2][1]);
				triTexCoord[5].Set(texCoord[1][0],texCoord[1][1]);
			}
			else
			{
				// (1,0,3),(2,3,0)
				tri[0]=&rc[1];
				tri[1]=&rc[0];
				tri[2]=&rc[3];
				tri[3]=&rc[2];
				tri[4]=&rc[3];
				tri[5]=&rc[0];

				triTexCoord[0].Set(texCoord[1][0],texCoord[1][1]);
				triTexCoord[1].Set(texCoord[0][0],texCoord[0][1]);
				triTexCoord[2].Set(texCoord[3][0],texCoord[3][1]);
				triTexCoord[3].Set(texCoord[2][0],texCoord[2][1]);
				triTexCoord[4].Set(texCoord[3][0],texCoord[3][1]);
				triTexCoord[5].Set(texCoord[0][0],texCoord[0][1]);
			}

			for(int k=0; k<2; k++)
			{
				YsVec3 nom[3];
				YsElvGridFaceId nodId[3];
				GetTriangleNodeId(nodId,i,j,k);
				nom[0]=node[(nx+1)*nodId[0].z+nodId[0].x].nomOfNode;
				nom[1]=node[(nx+1)*nodId[1].z+nodId[1].x].nomOfNode;
				nom[2]=node[(nx+1)*nodId[2].z+nodId[2].x].nomOfNode;

				if(node[baseIdx+i].visible[k]==YSTRUE)
				{
					graphicCache->meshCache.AddTexCoord(triTexCoord[k*3]);
					graphicCache->meshCache.AddTexCoord(triTexCoord[k*3+1]);
					graphicCache->meshCache.AddTexCoord(triTexCoord[k*3+2]);
					if(colorByElevation!=YSTRUE)
					{
						graphicCache->meshCache.AddPoint(*tri[k*3]  ,nom[0],node[baseIdx+i].c[k]);
						graphicCache->meshCache.AddPoint(*tri[k*3+1],nom[1],node[baseIdx+i].c[k]);
						graphicCache->meshCache.AddPoint(*tri[k*3+2],nom[2],node[baseIdx+i].c[k]);
					}
					else
					{
						graphicCache->meshCache.AddPoint(*tri[k*3]  ,nom[0],ColorByElevation(tri[k*3]->y()));
						graphicCache->meshCache.AddPoint(*tri[k*3+1],nom[1],ColorByElevation(tri[k*3+1]->y()));
						graphicCache->meshCache.AddPoint(*tri[k*3+2],nom[2],ColorByElevation(tri[k*3+2]->y()));
					}
				}
			}
			x+=xWid;
		}
	}

	if(sideWall[0]==YSTRUE) // Bottom
	{
		const YsVec3 nom(0.0,0.0,-1.0);

		double x=0.0;
		int baseIdx=0;
		for(int i=0; i<nx; i++)
		{
			const double y0=node[baseIdx  ].y;
			const double y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[0].AddPoint(YsVec3(x     ,0.0,0.0),nom,sideWallColor[0]);
					graphicCache->wallCache[0].AddPoint(YsVec3(x+xWid,0.0,0.0),nom,sideWallColor[0]);
					graphicCache->wallCache[0].AddPoint(YsVec3(x     ,y0 ,0.0),nom,sideWallColor[0]);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[0].AddPoint(YsVec3(x+xWid,0.0,0.0),nom,sideWallColor[0]);
					graphicCache->wallCache[0].AddPoint(YsVec3(x+xWid,y1 ,0.0),nom,sideWallColor[0]);
					graphicCache->wallCache[0].AddPoint(YsVec3(x     ,y0 ,0.0),nom,sideWallColor[0]);
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[0].AddPoint(YsVec3(x     ,0.0,0.0),nom,ColorByElevation(0.0));
					graphicCache->wallCache[0].AddPoint(YsVec3(x+xWid,0.0,0.0),nom,ColorByElevation(0.0));
					graphicCache->wallCache[0].AddPoint(YsVec3(x     ,y0 ,0.0),nom,ColorByElevation(y0));
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[0].AddPoint(YsVec3(x+xWid,0.0,0.0),nom,ColorByElevation(0.0));
					graphicCache->wallCache[0].AddPoint(YsVec3(x+xWid,y1 ,0.0),nom,ColorByElevation(y1));
					graphicCache->wallCache[0].AddPoint(YsVec3(x     ,y0 ,0.0),nom,ColorByElevation(y0));
				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[1]==YSTRUE) // Right
	{
		const YsVec3 nom(1.0,0.0,0.0);

		double z=0.0;
		int baseIdx=nx;
		for(int i=0; i<nz; i++)
		{
			const double y0=node[baseIdx       ].y;
			const double y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,0.0,z),     nom,sideWallColor[1]);
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,0.0,z+zWid),nom,sideWallColor[1]);
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,y0 ,z),     nom,sideWallColor[1]);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,0.0,z+zWid),nom,sideWallColor[1]);
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,y1 ,z+zWid),nom,sideWallColor[1]);
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,y0 ,z),     nom,sideWallColor[1]);
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,0.0,z),     nom,ColorByElevation(0.0));
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,0.0,z+zWid),nom,ColorByElevation(0.0));
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,y0 ,z),     nom,ColorByElevation(y0));
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,0.0,z+zWid),nom,ColorByElevation(0.0));
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,y1 ,z+zWid),nom,ColorByElevation(y1));
					graphicCache->wallCache[1].AddPoint(YsVec3(double(nx)*xWid,y0 ,z),     nom,ColorByElevation(y0));
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}
	if(sideWall[2]==YSTRUE) // Top
	{
		const YsVec3 nom(0.0,0.0,1.0);

		double x=0.0;
		int baseIdx=nz*(nx+1);
		for(int i=0; i<nx; i++)
		{
			const double y0=node[baseIdx  ].y;
			const double y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[2].AddPoint(YsVec3(x     ,0.0,double(nz)*zWid),nom,sideWallColor[2]);
					graphicCache->wallCache[2].AddPoint(YsVec3(x     ,y0 ,double(nz)*zWid),nom,sideWallColor[2]);
					graphicCache->wallCache[2].AddPoint(YsVec3(x+xWid,0.0,double(nz)*zWid),nom,sideWallColor[2]);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[2].AddPoint(YsVec3(x+xWid,0.0,double(nz)*zWid),nom,sideWallColor[2]);
					graphicCache->wallCache[2].AddPoint(YsVec3(x     ,y0 ,double(nz)*zWid),nom,sideWallColor[2]);
					graphicCache->wallCache[2].AddPoint(YsVec3(x+xWid,y1 ,double(nz)*zWid),nom,sideWallColor[2]);
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[2].AddPoint(YsVec3(x     ,0.0,double(nz)*zWid),nom,ColorByElevation(0.0));
					graphicCache->wallCache[2].AddPoint(YsVec3(x     ,y0 ,double(nz)*zWid),nom,ColorByElevation(y0));
					graphicCache->wallCache[2].AddPoint(YsVec3(x+xWid,0.0,double(nz)*zWid),nom,ColorByElevation(0.0));
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[2].AddPoint(YsVec3(x+xWid,0.0,double(nz)*zWid),nom,ColorByElevation(0.0));
					graphicCache->wallCache[2].AddPoint(YsVec3(x     ,y0 ,double(nz)*zWid),nom,ColorByElevation(y0));
					graphicCache->wallCache[2].AddPoint(YsVec3(x+xWid,y1 ,double(nz)*zWid),nom,ColorByElevation(y1));
				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[3]==YSTRUE) // Left
	{
		const YsVec3 nom(-1.0,0.0,0.0);

		double z=0.0;
		int baseIdx=0;
		for(int i=0; i<nz; i++)
		{
			const double y0=node[baseIdx       ].y;
			const double y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,0.0,z)     ,nom,sideWallColor[3]);
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,y0 ,z)     ,nom,sideWallColor[3]);
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,0.0,z+zWid),nom,sideWallColor[3]);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,0.0,z+zWid),nom,sideWallColor[3]);
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,y0 ,z)     ,nom,sideWallColor[3]);
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,y1 ,z+zWid),nom,sideWallColor[3]);
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,0.0,z),     nom,ColorByElevation(0.0));
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,y0 ,z),     nom,ColorByElevation(y0));
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,0.0,z+zWid),nom,ColorByElevation(0.0));
				}
				if(YsZero(y1)!=YSTRUE)
				{
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,0.0,z+zWid),nom,ColorByElevation(0.0));
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,y0 ,z),     nom,ColorByElevation(y0));
					graphicCache->wallCache[3].AddPoint(YsVec3(0.0,y1 ,z+zWid),nom,ColorByElevation(y1));
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}


	for(int j=0; j<nz; j++)
	{
		const double z=double(j)*zWid;
		double x=0.0;
		const int baseIdx=(nx+1)*j;

		YsVec3 rc[4],*tri[6];

		//  1  3
		//
		//  0  2
		rc[2].Set(x,node[baseIdx     ].y,z);
		rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
		for(int i=0; i<nx; i++)
		{
			rc[0]=rc[2];
			rc[1]=rc[3];
			rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
			rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

			if(node[baseIdx+i].lup==YSTRUE)
			{
				// (3,1,2),(0,2,1)
				tri[0]=&rc[3];
				tri[1]=&rc[1];
				tri[2]=&rc[2];
				tri[3]=&rc[0];
				tri[4]=&rc[2];
				tri[5]=&rc[1];
			}
			else
			{
				// (1,0,3),(2,3,0)
				tri[0]=&rc[1];
				tri[1]=&rc[0];
				tri[2]=&rc[3];
				tri[3]=&rc[2];
				tri[4]=&rc[3];
				tri[5]=&rc[0];
			}
			for(int k=0; k<2; k++)
			{
				YsElvGridFaceId nodId[3];
				GetTriangleNodeId(nodId,i,j,k);
				YsVec3 cen,cenBtm,btm[3];

				if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
				{
					graphicCache->protectPlgCache.AddProtectTri(*tri[k*3],*tri[k*3+1],*tri[k*3+2]);

					// Side walls
					btm[0].Set(tri[k*3  ]->x(),0.0,tri[k*3  ]->z());
					btm[1].Set(tri[k*3+1]->x(),0.0,tri[k*3+1]->z());
					btm[2].Set(tri[k*3+2]->x(),0.0,tri[k*3+2]->z());

					cen=(*tri[k*3]+*tri[k*3+1]+*tri[k*3+2])/3.0;
					cenBtm.Set(cen.x(),0.0,cen.z());

					graphicCache->protectPlgCache.AddProtectQuad(cenBtm,cen,*tri[k*3  ],btm[0]);
					graphicCache->protectPlgCache.AddProtectQuad(cenBtm,cen,*tri[k*3+1],btm[1]);
					graphicCache->protectPlgCache.AddProtectQuad(cenBtm,cen,*tri[k*3+2],btm[2]);
				}
			}
			x+=xWid;
		}
	}

	graphicCache->meshCache.MakeVbo();
	graphicCache->wallCache[0].MakeVbo();
	graphicCache->wallCache[1].MakeVbo();
	graphicCache->wallCache[2].MakeVbo();
	graphicCache->wallCache[3].MakeVbo();
	graphicCache->protectPlgCache.MakeVbo();
}

void YsElevationGrid::Draw
   (const double &plgColorScale,
    YSBOOL invert,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL /*shrinkTriangle*/,
    YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
	YsDisregardVariable(nameElvGridFace);
	YsDisregardVariable(nameElvGridNode);
	YsDisregardVariable(fill);
	YsDisregardVariable(wire);

	if(YSTRUE!=IsCached())
	{
		MakeCache(plgColorScale,invert);
	}

	DrawFastFillOnly(plgColorScale);

	if(drawBbx==YSTRUE)
	{
		DrawBoundingBox();
	}
}

void YsElevationGrid::DrawFastFillOnly(const double &plgColorScale)
{
	if(YSTRUE!=IsCached())
	{
		MakeCache(plgColorScale,YSFALSE);
	}

	if(YSTRUE==IsCached())
	{
		YSBOOL useOwnTexture=YSFALSE;
		if(0<texLabel.Strlen() && YSTRUE!=texLabelNotFound && NULL==texManCache && NULL!=owner)
		{
			TryCacheTexture(owner->GetOwner());
		}
		if(NULL!=texManCache && NULL!=texHdCache)
		{
			auto tex=texManCache->GetTexture(texHdCache);
			useOwnTexture=(YSOK==tex->Bind() ? YSTRUE : YSFALSE);
		}

		if(YSTRUE==useOwnTexture)
		{
			auto renderer=YsGLSLSharedVariColorShaded3DRenderer();
			YsGLSLUse3DRenderer(renderer);

			GLfloat savedSpecular[3];
			YsGLSLGet3DRendererSpecularColor(savedSpecular,renderer);
			const GLfloat specularOn[3]={1.0F,1.0F,1.0F};
			const GLfloat specularOff[3]={0.0F,0.0F,0.0F};
			YsGLSLSet3DRendererSpecularColor(renderer,(YSTRUE==GetSpecular() ? specularOn : specularOff));
			YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_ATTRIBUTE);

			graphicCache->meshCache.DrawPrimitiveVtxTexCoordNomColfv(renderer,GL_TRIANGLES);

			YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_NONE);
			YsGLSLSet3DRendererSpecularColor(renderer,savedSpecular);

			YsGLSLEndUse3DRenderer(renderer);
			glDisable(GL_TEXTURE_2D);
		}
		else
		{
			auto renderer=YsGLSLSharedVariColorShaded3DRenderer();
			YsGLSLUse3DRenderer(renderer);

			GLfloat savedSpecular[3];
			YsGLSLGet3DRendererSpecularColor(savedSpecular,renderer);
			const GLfloat specularOn[3]={1.0F,1.0F,1.0F};
			const GLfloat specularOff[3]={0.0F,0.0F,0.0F};
			YsGLSLSet3DRendererSpecularColor(renderer,(YSTRUE==GetSpecular() ? specularOn : specularOff));

			graphicCache->meshCache.DrawPrimitiveVtxNomColfv(renderer,GL_TRIANGLES);

			YsGLSLSet3DRendererSpecularColor(renderer,savedSpecular);

			YsGLSLEndUse3DRenderer(renderer);
		}
		{
			auto renderer=YsGLSLSharedVariColorShaded3DRenderer();
			YsGLSLUse3DRenderer(renderer);

			GLfloat savedSpecular[3];
			YsGLSLGet3DRendererSpecularColor(savedSpecular,renderer);
			const GLfloat specularOn[3]={1.0F,1.0F,1.0F};
			const GLfloat specularOff[3]={0.0F,0.0F,0.0F};
			YsGLSLSet3DRendererSpecularColor(renderer,(YSTRUE==GetSpecular() ? specularOn : specularOff));

			graphicCache->wallCache[0].DrawPrimitiveVtxNomColfv(renderer,GL_TRIANGLES);
			graphicCache->wallCache[1].DrawPrimitiveVtxNomColfv(renderer,GL_TRIANGLES);
			graphicCache->wallCache[2].DrawPrimitiveVtxNomColfv(renderer,GL_TRIANGLES);
			graphicCache->wallCache[3].DrawPrimitiveVtxNomColfv(renderer,GL_TRIANGLES);

			YsGLSLSet3DRendererSpecularColor(renderer,savedSpecular);

			YsGLSLEndUse3DRenderer(renderer);
		}
	}
}

//static void DrawQuadAsTwoTri(const YsVec3 &q0,const YsVec3 &q1,const YsVec3 &q2,const YsVec3 &q3)
//{
//	glVertex3dv(q0);
//	glVertex3dv(q1);
//	glVertex3dv(q2);
//
//	glVertex3dv(q2);
//	glVertex3dv(q3);
//	glVertex3dv(q0);
//}

void YsElevationGrid::DrawProtectPolygon(void)
{
	if(YSTRUE!=IsCached())
	{
		MakeCache(1.0,YSFALSE);
	}

//	if(hasProtectPolygon!=YSFALSE)  // Can be YSTFUNKNOWN
//	{
//		int i,j,baseIdx;
//		double x,z;
//		YsVec3 rc[4],*tri[6];
//
//		glPushAttrib(GL_ENABLE_BIT);
//		glDisable(GL_CULL_FACE);
//		glEnable(GL_LIGHTING);
//
//		// Adding polygon offset to terrain was not such a good idea.  It makes artifacts.
//		// glEnable(GL_POLYGON_OFFSET_FILL);
//		// glPolygonOffset(1,1);
//
//
//		glBegin(GL_TRIANGLES);
//
//		for(j=0; j<nz; j++)
//		{
//			z=double(j)*zWid;
//			x=0.0;
//			baseIdx=(nx+1)*j;
//
//			//  1  3
//			//
//			//  0  2
//			rc[2].Set(x,node[baseIdx     ].y,z);
//			rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
//			for(i=0; i<nx; i++)
//			{
//				rc[0]=rc[2];
//				rc[1]=rc[3];
//				rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
//				rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);
//
//				if(node[baseIdx+i].lup==YSTRUE)
//				{
//					// (3,1,2),(0,2,1)
//					tri[0]=&rc[3];
//					tri[1]=&rc[1];
//					tri[2]=&rc[2];
//					tri[3]=&rc[0];
//					tri[4]=&rc[2];
//					tri[5]=&rc[1];
//				}
//				else
//				{
//					// (1,0,3),(2,3,0)
//					tri[0]=&rc[1];
//					tri[1]=&rc[0];
//					tri[2]=&rc[3];
//					tri[3]=&rc[2];
//					tri[4]=&rc[3];
//					tri[5]=&rc[0];
//				}
//				int k;
//				for(k=0; k<2; k++)
//				{
//					YsElvGridFaceId nodId[3];
//					GetTriangleNodeId(nodId,i,j,k);
//					YsVec3 cen,cenBtm,btm[3];
//
//					if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
//					{
//						hasProtectPolygon=YSTRUE;
//
//						glVertex3dv(*tri[k*3  ]);
//						glVertex3dv(*tri[k*3+1]);
//						glVertex3dv(*tri[k*3+2]);
//
//						// Side walls
//						btm[0].Set(tri[k*3  ]->x(),0.0,tri[k*3  ]->z());
//						btm[1].Set(tri[k*3+1]->x(),0.0,tri[k*3+1]->z());
//						btm[2].Set(tri[k*3+2]->x(),0.0,tri[k*3+2]->z());
//
//						cen=(*tri[k*3]+*tri[k*3+1]+*tri[k*3+2])/3.0;
//						cenBtm.Set(cen.x(),0.0,cen.z());
//
//						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3  ],btm[0]);
//						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+1],btm[1]);
//						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+2],btm[2]);
//					}
//				}
//				x+=xWid;
//			}
//		}
//		glEnd();
//
//		glPopAttrib();
//
//		if(hasProtectPolygon==YSTFUNKNOWN)
//		{
//			hasProtectPolygon=YSFALSE;
//		}
//	}
}


// See YSFLIGHT/memo/technical/20081116-clippedProtectPolygon.jnt
void YsElevationGrid::DrawClippedProtectPolygon(
    const YsVec3 &cameraPos,const YsPlane &clipPln,const YsPlane &nearPln,const YsVec3 &t0,const YsVec3 &t1,const YsVec3 &t2)

{
//	int nClp1,nClp2;
//	YsVec3 clp1[5],clp2[5];
//	YsVec3 const *tri[4];
//	int i;
//
//	tri[0]=&t0;
//	tri[1]=&t1;
//	tri[2]=&t2;
//	tri[3]=&t0;
//
//	// Clipping the polygon by clipPln (Extract only forward part)
//	nClp1=0;
//	for(i=0; i<3 && nClp1<4; i++)
//	{
//		if(clipPln.GetSideOfPlane(*tri[i])>=0)
//		{
//			clp1[nClp1++]=*tri[i];
//			if(clipPln.GetSideOfPlane(*tri[i+1])<0)  // Going out
//			{
//				if(clipPln.GetIntersection(clp1[nClp1],*tri[i],*tri[i+1]-*tri[i])==YSOK)
//				{
//					nClp1++;
//				}
//			}
//		}
//		else if(clipPln.GetSideOfPlane(*tri[i+1])>=0)  // Coming in
//		{
//			if(clipPln.GetIntersection(clp1[nClp1],*tri[i],*tri[i+1]-*tri[i])==YSOK)
//			{
//				nClp1++;
//			}
//		}
//	}
//	if(nClp1==0)
//	{
//		return;
//	}
//	clp1[nClp1]=clp1[0];
//
//	// Clipping the polygon by nearPln (Extract only rear part)
//	nClp2=0;
//	for(i=0; i<nClp1 && nClp2<5; i++)
//	{
//		if(nearPln.GetSideOfPlane(clp1[i])<0)
//		{
//			clp2[nClp2++]=clp1[i];
//			if(nearPln.GetSideOfPlane(clp1[i+1])>=0)  // Going out
//			{
//				if(nearPln.GetIntersection(clp2[nClp2],clp1[i],clp1[i+1]-clp1[i])==YSOK)
//				{
//					nClp2++;
//				}
//			}
//		}
//		else if(nearPln.GetSideOfPlane(clp1[i+1])<0)  // Coming in
//		{
//			if(nearPln.GetIntersection(clp2[nClp2],clp1[i],clp1[i+1]-clp1[i])==YSOK)
//			{
//				nClp2++;
//			}
//		}
//	}
//
//
//	// Extend the polygon to nearPln
//	for(i=0; i<nClp2; i++)
//	{
//		if(nearPln.GetSideOfPlane(clp2[i])<0)
//		{
//			nearPln.GetIntersection(clp2[i],cameraPos,clp2[i]-cameraPos);
//		}
//	}
//
//	// Then draw
//	if(nClp2>=3)
//	{
//		glVertex3dv(clp2[0]);
//		glVertex3dv(clp2[1]);
//		glVertex3dv(clp2[2]);
//		if(nClp2>=4)
//		{
//			glVertex3dv(clp2[0]);
//			glVertex3dv(clp2[2]);
//			glVertex3dv(clp2[3]);
//		}
//		if(nClp2>=5)
//		{
//			glVertex3dv(clp2[0]);
//			glVertex3dv(clp2[3]);
//			glVertex3dv(clp2[4]);
//		}
//	}
}

void YsElevationGrid::DrawProtectPolygonAccurate(const YsMatrix4x4 &viewMdlMat,const double &nearZ)
{
//	if(hasProtectPolygon!=YSFALSE)  // Can be YSTFUNKNOWN
//	{
//		int i,j,baseIdx;
//		double x,z;
//		YsVec3 rc[4],*tri[6];
//
//
//		// See YSFLIGHT/document/20081116.jnt for why cameraPos, nearPln, and clipPln are needed.
//		// posInCameraCoord=viewMdlMat*posInEvgCoord
//		YsMatrix4x4 camToEvg(viewMdlMat);
//		YsPlane nearPln,clipPln;
//		YsVec3 cameraPos,org,nom;
//
//		camToEvg.Invert();  // Now posInEvgCoord=camToEvg*posInCameraCoord
//
//		camToEvg.Mul(cameraPos,YsOrigin(),1.0);
//		org.Set(0.0,0.0,nearZ*1.001);
//		nom.Set(0.0,0.0,1.0);
//		camToEvg.Mul(org,org,1.0);
//		camToEvg.Mul(nom,nom,0.0);
//		nearPln.Set(org,nom);  // Side<0 means a point is closer than the near plane.
//
//		org.Set(0.0,0.0,0.5);
//		camToEvg.Mul(org,org,1.0);
//		clipPln.Set(org,nom);
//
//
//
//		glPushAttrib(GL_ENABLE_BIT);
//		glDisable(GL_CULL_FACE);
//		glDisable(GL_LIGHTING);
//
//
//		glBegin(GL_TRIANGLES);
//
//		for(j=0; j<nz; j++)
//		{
//			z=double(j)*zWid;
//			x=0.0;
//			baseIdx=(nx+1)*j;
//
//			//  1  3
//			//
//			//  0  2
//			rc[2].Set(x,node[baseIdx     ].y,z);
//			rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
//			for(i=0; i<nx; i++)
//			{
//				rc[0]=rc[2];
//				rc[1]=rc[3];
//				rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
//				rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);
//
//				if(node[baseIdx+i].protectPolygon[0]==YSTRUE ||
//				   node[baseIdx+i].protectPolygon[1]==YSTRUE)
//				{
//					if(node[baseIdx+i].lup==YSTRUE)
//					{
//						// (3,1,2),(0,2,1)
//						tri[0]=&rc[3];
//						tri[1]=&rc[1];
//						tri[2]=&rc[2];
//						tri[3]=&rc[0];
//						tri[4]=&rc[2];
//						tri[5]=&rc[1];
//					}
//					else
//					{
//						// (1,0,3),(2,3,0)
//						tri[0]=&rc[1];
//						tri[1]=&rc[0];
//						tri[2]=&rc[3];
//						tri[3]=&rc[2];
//						tri[4]=&rc[3];
//						tri[5]=&rc[0];
//					}
//					int k;
//					for(k=0; k<2; k++)
//					{
//						YsElvGridFaceId nodId[3];
//						GetTriangleNodeId(nodId,i,j,k);
//						YsVec3 cen,cenBtm,btm[3];
//
//						if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
//						{
//							hasProtectPolygon=YSTRUE;
//
//							glVertex3dv(*tri[k*3  ]);
//							glVertex3dv(*tri[k*3+1]);
//							glVertex3dv(*tri[k*3+2]);
//
//							if(nearPln.GetSideOfPlane(*tri[k*3  ])<0 ||
//							   nearPln.GetSideOfPlane(*tri[k*3+1])<0 ||
//							   nearPln.GetSideOfPlane(*tri[k*3+2])<0)
//							{
//								// See YSFLIGHT/document/20081116.jnt
//								DrawClippedProtectPolygon(cameraPos,clipPln,nearPln,*tri[k*3],*tri[k*3+1],*tri[k*3+2]);
//							}
//
//							// Side walls
//							btm[0].Set(tri[k*3  ]->x(),0.0,tri[k*3  ]->z());
//							btm[1].Set(tri[k*3+1]->x(),0.0,tri[k*3+1]->z());
//							btm[2].Set(tri[k*3+2]->x(),0.0,tri[k*3+2]->z());
//
//							cen=(*tri[k*3]+*tri[k*3+1]+*tri[k*3+2])/3.0;
//							cenBtm.Set(cen.x(),0.0,cen.z());
//
//							DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3  ],btm[0]);
//							DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+1],btm[1]);
//							DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+2],btm[2]);
//						}
//					}
//				}
//				x+=xWid;
//			}
//		}
//		glEnd();
//
//		glPopAttrib();
//
//		if(hasProtectPolygon==YSTFUNKNOWN)
//		{
//			hasProtectPolygon=YSFALSE;
//		}
//	}
}

void YsElevationGrid::DrawBoundingBox(void)
{
//	glDisable(GL_LIGHTING);
//	YsScenery::GlSetColor(YsCyan());
//	glBegin(GL_LINE_LOOP);
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//	glEnd();
//
//	glBegin(GL_LINES);
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//	glEnd();
//
//	glBegin(GL_LINE_LOOP);
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//	glEnd();
}

void YsSceneryShell::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
//	if(drawBbx==YSTRUE)
//	{
//		YsVec3 bbx[2];
//		shl.GetBoundingBox(bbx[0],bbx[1]);
//
//		glDisable(GL_LIGHTING);
//		YsScenery::GlSetColor(YsWhite());
//		glBegin(GL_LINE_LOOP);
//		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//		glEnd();
//
//		glBegin(GL_LINES);
//		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//		glEnd();
//
//		glBegin(GL_LINE_LOOP);
//		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//		glEnd();
//	}
//
//	shl.Draw();
//	/* if(fill==YSTRUE)
//	{
//		YsGlDrawShellPolygon(shl,YSTRUE,YsBlack(),YSTRUE);
//	}
//	if(wire==YSTRUE)
//	{
//		YsGlDrawShellWireframe(shl,YsWhite());
//	} */
}

void YsSceneryRectRegion::Draw(void)
{
//	glBegin(GL_LINE_LOOP);
//	glVertex3d(min.x(),0.0,min.y());
//	glVertex3d(max.x(),0.0,min.y());
//	glVertex3d(max.x(),0.0,max.y());
//	glVertex3d(min.x(),0.0,max.y());
//	glEnd();
//
//	glRasterPos3d(0.0,20.0,0.0);
//	char idStr[256],tagStr[256];
//	idStr[0]=0;
//	tagStr[0]=0;
//
//	switch(id)
//	{
//	case 1:
//		strcpy(idStr,"ID=1(Runway)");
//		break;
//	case 2:
//		strcpy(idStr,"ID=1(Taxiway)");
//		break;
//	case 10:
//		strcpy(idStr,"ID=10(View Point)");
//		break;
//	default:
//		sprintf(idStr,"ID=%d",id);
//		break;
//	}
//
//	if(tag[0]!=0)
//	{
//		sprintf(tagStr,",TAG=%s",tag);
//	}
//
//	YsString all;
//	all.Set("RGN ");
//	all.Append(idStr);
//	all.Append(" ");
//	all.Append(tagStr);
//	ysGlPrint(all);
}

void YsSceneryGndObj::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
//	if(gndObjTemplate!=NULL)
//	{
//		if(drawBbx==YSTRUE)
//		{
//			glDisable(GL_LIGHTING);
//
//			YsVec3 bbx[2];
//			gndObjTemplate->dnm.GetBoundingBox(bbx);
//
//			YsScenery::GlSetColor(YsBlue());
//			glBegin(GL_LINE_LOOP);
//			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//			glEnd();
//
//			glBegin(GL_LINES);			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//			glEnd();
//
//			glBegin(GL_LINE_LOOP);
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//			glEnd();
//		}
//
//		gndObjTemplate->dnm.Draw(YsOrigin(),YsZeroAtt());
//		/* if(fill==YSTRUE)
//		{
//			YsGlDrawShellPolygon(gndObjTemplate->shl,YSTRUE,YsBlack(),YSTRUE);
//		}
//		if(wire==YSTRUE)
//		{
//			YsGlDrawShellWireframe(gndObjTemplate->shl,YsBlack());
//		} */
//	}
//
//	YsVec3 bbx[2],dgn;
//	double l;
//
//	GetBoundingBox(bbx);
//	dgn=bbx[1]-bbx[0];
//	l=dgn.GetLength();
//
//	glDisable(GL_LIGHTING);  // glCallList ignores color unless something is drawn before.
//	YsScenery::GlSetColor(YsThreeBitColor(iff+1));
//	glBegin(GL_LINES);
//	glVertex3dv(YsOrigin());
//	glVertex3d(0.0,l,0.0);
//	glEnd();
//
//	glRasterPos3d(0.0,l,0.0);
//	char str[256],add[256];
//
//	sprintf(str,"[%s] IFF=%d",(char *)objName,iff+1);
//
//	if(gndFlag!=0)
//	{
//		sprintf(add," F=0x%02x",gndFlag);
//		strcat(str,add);
//	}
//
//	if(primaryTarget==YSTRUE)
//	{
//		strcat(str," (P)");
//	}
//
//	if(id!=0)
//	{
//		sprintf(add," ID=%d",id);
//		strcat(str,add);
//	}
//
//	if(tag[0]!=0)
//	{
//		strcat(str," TAG=\"");
//		strcat(str,tag);
//		strcat(str,"\"");
//	}
//
//	ysGlPrint(str);
}

void YsSceneryAir::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
//	if(airTemplate!=NULL)
//	{
//		if(drawBbx==YSTRUE)
//		{
//			glDisable(GL_LIGHTING);
//
//			YsVec3 bbx[2];
//			airTemplate->dnm.GetBoundingBox(bbx);
//
//			YsScenery::GlSetColor(YsGreen());
//			glBegin(GL_LINE_LOOP);
//			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//			glEnd();
//
//			glBegin(GL_LINES);			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//			glEnd();
//
//			glBegin(GL_LINE_LOOP);
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//			glEnd();
//		}
//
//		if(ldg==YSTRUE)
//		{
//			airTemplate->dnm.SetClassState(0,1);
//		}
//		else
//		{
//			airTemplate->dnm.SetClassState(0,0);
//		}
//
//		airTemplate->dnm.Draw(YsOrigin(),YsZeroAtt());
//		/* if(fill==YSTRUE)
//		{
//			YsGlDrawShellPolygon(airTemplate->shl,YSTRUE,YsBlack(),YSTRUE);
//		}
//		if(wire==YSTRUE)
//		{
//			YsGlDrawShellWireframe(airTemplate->shl,YsBlack());
//		} */
//	}
//
//	YsVec3 bbx[2],dgn;
//	double l;
//
//	GetBoundingBox(bbx);
//	dgn=bbx[1]-bbx[0];
//	l=dgn.GetLength();
//
//	glDisable(GL_LIGHTING);  // glCallList ignores color unless something is drawn before.
//	YsScenery::GlSetColor(YsThreeBitColor(iff+1));
//	glBegin(GL_LINES);
//	glVertex3dv(YsOrigin());
//	glVertex3d(0.0,l,0.0);
//	glEnd();
//
//	glRasterPos3d(0.0,l,0.0);
//	char str[256];
//
//	sprintf(str,"[%s] IFF=%d",(const char *)objName,iff+1);
//	if(ldg==YSTRUE)
//	{
//		strcat(str," (LDG)");
//	}
//
//	if(id!=0)
//	{
//		char add[256];
//		sprintf(add," ID=%d",id);
//		strcat(str,add);
//	}
//
//	if(tag[0]!=0)
//	{
//		strcat(str," TAG=\"");
//		strcat(str,tag);
//		strcat(str,"\"");
//	}
//
//	ysGlPrint(str);
}

void YsSceneryPointSet::Draw(void)
{
//	int i;
//
//	glDisable(GL_LIGHTING);
//
//	if(GetAreaType()==YSSCNAREA_NOAREA)
//	{
//		glLineWidth(2);
//
//		double r,g;
//		glBegin(GL_LINE_STRIP);
//		if(pnt.GetN()>1)
//		{
//			for(i=0; i<pnt.GetN(); i++)
//			{
//				r=(double)i/(double)(pnt.GetN()-1);
//				g=(double)(pnt.GetN()-1-i)/(double)(pnt.GetN()-1);
//				glColor3d(r,g,0.0);
//				glVertex3dv(pnt[i]);
//			}
//			if(isLoop==YSTRUE)
//			{
//				glColor3d(1.0,0.0,0.0);
//				glVertex3dv(pnt[0]);
//			}
//		}
//		glEnd();
//		glLineWidth(1);
//	}
//	else
//	{
//		switch(GetAreaType())
//		{
//		case YSSCNAREA_LAND:
//			glColor3d(1.0,0.0,0.0);
//			break;
//		default:
//		case YSSCNAREA_WATER:
//			glColor3d(0.0,1.0,1.0);
//			break;
//		}
//
//		glLineWidth(1);
//		glBegin(GL_LINE_LOOP);
//		forYsArray(i,pnt)
//		{
//			glVertex3dv(pnt[i]);
//		}
//		glEnd();
//	}
//
//	glPointSize(5);
//	glBegin(GL_POINTS);
//	for(i=0; i<pnt.GetN(); i++)
//	{
//		glVertex3dv(pnt[i]);
//	}
//	glEnd();
//	glPointSize(1);
//
//
//	if(pnt.GetN()>0)
//	{
//		glRasterPos3d(pnt[0].x(),pnt[0].y(),pnt[0].z());
//	}
//	else
//	{
//		glRasterPos3d(0.0,0.0,0.0);
//	}
//
//	char all[256];
//	if(GetAreaType()==YSSCNAREA_NOAREA)
//	{
//		sprintf(all,"ID=%d TAG=%s",id,tag);
//	}
//	else
//	{
//		sprintf(all,"ID=%d TAG=%s AREA=%s",id,tag,YsScenery::GetAreaTypeString(GetAreaType()));
//	}
//	ysGlPrint(all);
//
//
//	glEnable(GL_LIGHTING);
}

void YsSceneryPointSet::DrawStar(void)
{
//	int i;
//	glPointSize(8);
//	YsGlSetColor(YsGreen());
//	glBegin(GL_POINTS);
//	for(i=0; i<pnt.GetN(); i++)
//	{
//		glVertex3dv(pnt[i]);
//	}
//	glEnd();
//	glPointSize(1);
//
//	glLineWidth(5);
//	glBegin(GL_LINE_STRIP);
//	if(pnt.GetN()>1)
//	{
//		for(i=0; i<pnt.GetN(); i++)
//		{
//			glVertex3dv(pnt[i]);
//		}
//		if(isLoop==YSTRUE)
//		{
//			glVertex3dv(pnt[0]);
//		}
//	}
//	glEnd();
//	glLineWidth(1);
}

void YsScenery::GlSetColor(const YsColor &col)
{
	YsDisregardVariable(col);
//	glColor3d(col.Rd(),col.Gd(),col.Bd());
}

void YsScenery::DrawBoundingBox(void)
{
//	YsVec3 bbx[2],dgn,XZ;
//	double l;
//	GetBoundingBox(bbx);
//	dgn=bbx[1]-bbx[0];
//	l=YsGreater(YsGreater(dgn.x(),dgn.y()),dgn.z());
//	XZ.Set(1.0,0.0,1.0);
//	bbx[0]-=l*0.05*XZ;
//	bbx[1]+=l*0.05*XZ;
//
//	glDisable(GL_LIGHTING);
//	YsScenery::GlSetColor(YsDarkYellow());
//	glBegin(GL_LINE_LOOP);
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//	glEnd();
//
//	glBegin(GL_LINES);
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//	glEnd();
//
//	glBegin(GL_LINE_LOOP);
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
//	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
//	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
//	glEnd();
}

void YsScenery::Draw
	   (YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryAir> *air;
	YsListItem <YsSceneryPointSet> *pst;
	YsListItem <YsScenery> *scn;


	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());

	YsMatrix4x4 tfm(YSFALSE);
	tfm.CreateFromOpenGlCompatibleMatrix(glMatPush);
	tfm.Multiply(pos,att);



	if(drawScn==YSTRUE && drawBbx==YSTRUE)
	{
		double glMat[16];
		tfm.GetOpenGlCompatibleMatrix(glMat);
		SetUpMatrix::SetDirect(glMat);
		DrawBoundingBox();
	}


	if(drawShl==YSTRUE)
	{
		shl=NULL;
		while((shl=shlList.FindNext(shl))!=NULL)
		{
			SetUpMatrix::SetUp(shl->dat.pos,shl->dat.att,tfm);
			shl->dat.Draw(wire,fill,drawBbx);
		}
	}

	if(drawEvg==YSTRUE)
	{
		evg=NULL;
		while((evg=evgList.FindNext(evg))!=NULL)
		{
			SetUpMatrix::SetUp(evg->dat.pos,evg->dat.att,tfm);
			evg->dat.evg.Draw(plgColorScale,YSFALSE,wire,fill,drawBbx,drawShrink,YSFALSE,YSFALSE);
		}
	}

	if(drawSbd==YSTRUE)
	{
		drw=NULL;
		while((drw=sbdList.FindNext(drw))!=NULL)
		{
			SetUpMatrix::SetUp(drw->dat.pos,drw->dat.att,tfm);
			drw->dat.drw.Draw(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,YSFALSE,-1.0);
		}
	}

	if(drawRgn==YSTRUE)
	{
		rgn=NULL;
		while((rgn=rgnList.FindNext(rgn))!=NULL)
		{
			SetUpMatrix::SetUp(rgn->dat.pos,rgn->dat.att,tfm);
			YsScenery::GlSetColor(YsThreeBitColor(rgn->dat.id%8));
			rgn->dat.Draw();
		}
	}

	if(drawGndObj==YSTRUE)
	{
		gnd=NULL;
		while((gnd=gndList.FindNext(gnd))!=NULL)
		{
			SetUpMatrix::SetUp(gnd->dat.pos,gnd->dat.att,tfm);
			gnd->dat.Draw(wire,fill,drawBbx);
		}
	}

	if(drawAir==YSTRUE)
	{
		air=NULL;
		while(NULL!=(air=airList.FindNext(air)))
		{
			SetUpMatrix::SetUp(air->dat.pos,air->dat.att,tfm);
			air->dat.Draw(wire,fill,drawBbx);
		}
	}

	if(drawPst==YSTRUE)
	{
		pst=NULL;
		while((pst=pstList.FindNext(pst))!=NULL)
		{
			SetUpMatrix::SetUp(pst->dat.pos,pst->dat.att,tfm);
			pst->dat.Draw();
		}
	}



	double glMat[16];
	tfm.GetOpenGlCompatibleMatrix(glMat);

	YsGLSLUse3DRenderer(YsGLSLSharedVariColor3DRenderer());
	YsGLSLSet3DRendererModelViewdv(YsGLSLSharedVariColor3DRenderer(),glMat);
	YsGLSLEndUse3DRenderer(YsGLSLSharedVariColor3DRenderer());

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.Draw(wire,fill,drawBbx,drawShrink,drawShl,drawEvg,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);
	}

	SetUpMatrix::SetDirect(glMatPush);
}

void YsScenery::DrawMap(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
	YsGLSL3DRenderer *renderer[]=
	{
		YsGLSLSharedFlat3DRenderer(),
		YsGLSLSharedMonoColorShaded3DRenderer(),
		YsGLSLSharedVariColorShaded3DRenderer(),
		NULL
	};

	glActiveTexture(GL_TEXTURE0);
	if(nullptr!=commonTexManPtr && YSTRUE==commonTexManPtr->IsReady(commonGroundTexHd))
	{
		auto unitPtr=commonTexManPtr->GetTexture(commonGroundTexHd);
		unitPtr->Bind();
	}

	for(int i=0; NULL!=renderer[i]; ++i)
	{
		const GLfloat texTfm[]=
		{
			0.025f,0.0f  ,0.0f  ,0,
			0.0f  ,0.0f  ,0.025f,0,
			0.0f  ,0.025f,0.0f  ,0,
			0.0f  ,0.0f  ,0.0f  ,1
		};
		YsGLSLUse3DRenderer(renderer[i]);
		if(nullptr!=commonTexManPtr && YSTRUE==commonTexManPtr->IsReady(commonGroundTexHd))
		{
			YsGLSLSet3DRendererTextureType(renderer[i],YSGLSL_TEX_TYPE_TILING);
		}
		else
		{
			YsGLSLSet3DRendererTextureType(renderer[i],YSGLSL_TEX_TYPE_NONE);
		}
		YsGLSLSet3DRendererTextureIdentifier(renderer[i],0); // 0 for GL_TEXTURE0 apparently
		YsGLSLSet3DRendererUniformTextureTilingMatrixfv(renderer[i],texTfm);
		YsGLSLEndUse3DRenderer(renderer[i]);
	}



	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());

	YsMatrix4x4 tfm(YSFALSE);
	tfm.CreateFromOpenGlCompatibleMatrix(glMatPush);
	tfm.Multiply(pos,att);



	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsScenery> *scn;


	drw=NULL;
	while((drw=mapList.FindNext(drw))!=NULL)
	{
		SetUpMatrix::SetUp(drw->dat.pos,drw->dat.att,tfm);
		drw->dat.drw.Draw(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSTRUE,wire,fill,drawBbx,YSFALSE,-1.0);
	}



	double glMat[16];
	tfm.GetOpenGlCompatibleMatrix(glMat);

	YsGLSLUse3DRenderer(YsGLSLSharedVariColor3DRenderer());
	YsGLSLSet3DRendererModelViewdv(YsGLSLSharedVariColor3DRenderer(),glMat);
	YsGLSLEndUse3DRenderer(YsGLSLSharedVariColor3DRenderer());

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.DrawMap(wire,fill,drawBbx);
	}

	SetUpMatrix::SetDirect(glMatPush);

	for(int i=0; NULL!=renderer[i]; ++i)
	{
		YsGLSLUse3DRenderer(renderer[i]);
		YsGLSLSet3DRendererTextureType(renderer[i],YSGLSL_TEX_TYPE_NONE);
		YsGLSLEndUse3DRenderer(renderer[i]);
	}
}

void YsScenery::DrawProtectPolygon(const YsMatrix4x4 &modelTfm) // OpenGL Only for SceneryEdit
{
	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());



	glDepthMask(1);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery> *scn;
	YsMatrix4x4 newModelTfm(YSFALSE),shlTfm(YSFALSE);


	newModelTfm=modelTfm;
	newModelTfm.Translate(pos);
	newModelTfm.RotateXZ(att.h());
	newModelTfm.RotateZY(att.p());
	newModelTfm.RotateXY(att.b());

	evg=NULL;
	while((evg=evgList.FindNext(evg))!=NULL)
	{
		SetUpMatrix::SetUp(evg->dat.pos,evg->dat.att,newModelTfm);
		evg->dat.evg.DrawProtectPolygon();
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.DrawProtectPolygon(newModelTfm);
	}


	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	SetUpMatrix::SetDirect(glMatPush);
}

void YsScenery::DrawProtectPolygon(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,const double nearZ,
    const double &currentTime)
{
	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());


	glDepthMask(1);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery> *scn;
	YsMatrix4x4 viewModelTfm(YSFALSE),newModelTfm(YSFALSE),evgTfm(YSFALSE);


	newModelTfm=modelTfm;
	newModelTfm.Translate(pos);
	newModelTfm.RotateXZ(att.h());
	newModelTfm.RotateZY(att.p());
	newModelTfm.RotateXY(att.b());

	viewModelTfm=viewTfm*newModelTfm;

	evg=NULL;
	while((evg=evgList.FindNext(evg))!=NULL)
	{
		if(evg->dat.evg.hasProtectPolygon!=YSFALSE &&
		   IsItemVisible(viewModelTfm,projTfm,&evg->dat)==YSTRUE)
		{
			evgTfm=viewModelTfm;
			evgTfm.Translate(evg->dat.pos);
			evgTfm.RotateXZ(evg->dat.att.h());
			evgTfm.RotateZY(evg->dat.att.p());
			evgTfm.RotateXY(evg->dat.att.b());

			SetUpMatrix::SetUp(evg->dat.pos,evg->dat.att,viewModelTfm);
			evg->dat.evg.DrawProtectPolygonAccurate(evgTfm,nearZ);
		}
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&scn->dat)==YSTRUE)
		{
			scn->dat.DrawProtectPolygon(viewTfm,newModelTfm,projTfm,nearZ,currentTime);
		}
	}


	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

	SetUpMatrix::SetDirect(glMatPush);
}


int YsScenery::numSceneryDrawn;

void YsScenery::DrawVisual(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,const double &currentTime,YSBOOL forShadowMap)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsScenery> *scn;
	const YSBOOL wire=YSTRUE,fill=YSTRUE,drawBbx=YSFALSE /*,drawShrink=YSFALSE*/;
	YsMatrix4x4 viewModelTfm(YSFALSE),newModelTfm(YSFALSE),shlTfm(YSFALSE);

	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());



	newModelTfm=modelTfm;
	newModelTfm.Translate(pos);
	newModelTfm.RotateXZ(att.h());
	newModelTfm.RotateZY(att.p());
	newModelTfm.RotateXY(att.b());

	viewModelTfm=viewTfm*newModelTfm;


	numSceneryDrawn++;

	shl=NULL;
	while((shl=shlList.FindNext(shl))!=NULL)
	{
		shl->dat.shl.Draw(viewModelTfm,projTfm,shl->dat.pos,shl->dat.att,YsVisual::DRAWALL);
	}

	if(NULL!=evgList.FindNext(NULL))
	{
		if(YSTRUE!=forShadowMap)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glFrontFace(GL_CW);
		}

		glActiveTexture(GL_TEXTURE0);
		if(nullptr!=commonTexManPtr && YSTRUE==commonTexManPtr->IsReady(commonGroundTexHd))
		{
			auto unitPtr=commonTexManPtr->GetTexture(commonGroundTexHd);
			unitPtr->Bind();
		}

		const GLfloat texTfm[]=
		{
			0.025f,0.0f,  0.0f,  0,
			0.0f,  0.0f,  0.025f,0,
			0.0f,  0.025f,0.0f,  0,
			0.0f,  0.0f,  0.0f,  1
		};
		YsGLSLUse3DRenderer(YsGLSLSharedVariColorShaded3DRenderer());
		if(nullptr!=commonTexManPtr && YSTRUE==commonTexManPtr->IsReady(commonGroundTexHd))
		{
			YsGLSLSet3DRendererTextureType(YsGLSLSharedVariColorShaded3DRenderer(),YSGLSL_TEX_TYPE_TILING);
		}
		else
		{
			YsGLSLSet3DRendererTextureType(YsGLSLSharedVariColorShaded3DRenderer(),YSGLSL_TEX_TYPE_NONE);
		}
		YsGLSLSet3DRendererTextureIdentifier(YsGLSLSharedVariColorShaded3DRenderer(),0); // 0 for GL_TEXTURE0 apparently
		YsGLSLSet3DRendererUniformTextureTilingMatrixfv(YsGLSLSharedVariColorShaded3DRenderer(),texTfm);
		YsGLSLEndUse3DRenderer(YsGLSLSharedVariColorShaded3DRenderer());

		evg=NULL;
		while((evg=evgList.FindNext(evg))!=NULL)
		{
			if(IsItemVisible(viewModelTfm,projTfm,&evg->dat)==YSTRUE)
			{
				SetUpMatrix::SetUpLF(evg->dat.pos,evg->dat.att,viewModelTfm);
				evg->dat.evg.DrawFastFillOnly(plgColorScale);
			}
		}

		YsGLSLUse3DRenderer(YsGLSLSharedVariColorShaded3DRenderer());
		YsGLSLSet3DRendererTextureType(YsGLSLSharedVariColorShaded3DRenderer(),YSGLSL_TEX_TYPE_NONE);
		YsGLSLEndUse3DRenderer(YsGLSLSharedVariColorShaded3DRenderer());
	}
	glFrontFace(GL_CCW);

	drw=NULL;
	while((drw=sbdList.FindNext(drw))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&drw->dat)==YSTRUE)
		{
			shlTfm=viewModelTfm;
			shlTfm.Translate(drw->dat.pos);
			shlTfm.RotateXZ(drw->dat.att.h());
			shlTfm.RotateZY(drw->dat.att.p());
			shlTfm.RotateXY(drw->dat.att.b());

			SetUpMatrix::SetUpLF(drw->dat.pos,drw->dat.att,viewModelTfm);
			drw->dat.drw.Draw(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,YSFALSE,currentTime,&shlTfm);
		}
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&scn->dat)==YSTRUE)
		{
			scn->dat.DrawVisual(viewTfm,newModelTfm,projTfm,currentTime,forShadowMap);
		}
	}

	SetUpMatrix::SetDirect(glMatPush);
}

void YsScenery::DrawMapVisual
    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &,const YsMatrix4x4 &projTfm,
     const double &,const double &,YSBOOL drawPset,const double &currentTime)
{
	YsGLSL3DRenderer *renderer[]=
	{
		YsGLSLSharedFlat3DRenderer(),
		YsGLSLSharedMonoColorShaded3DRenderer(),
		YsGLSLSharedVariColorShaded3DRenderer(),
		NULL
	};

	glActiveTexture(GL_TEXTURE0);
	if(nullptr!=commonTexManPtr && YSTRUE==commonTexManPtr->IsReady(commonGroundTexHd))
	{
		auto unitPtr=commonTexManPtr->GetTexture(commonGroundTexHd);
		unitPtr->Bind();
	}

	for(int i=0; NULL!=renderer[i]; ++i)
	{
		GLfloat texTfm[]=
		{
			0.025f,0.0f  ,0.0f  ,0,
			0.0f,  0.0f  ,0.025f,0,
			0.0f,  0.025f,0.0f,  0,
			0.0f,  0.0f  ,0.0f,  1
		};
		YsGLSLUse3DRenderer(renderer[i]);
		if(nullptr!=commonTexManPtr && YSTRUE==commonTexManPtr->IsReady(commonGroundTexHd))
		{
			YsGLSLSet3DRendererTextureType(renderer[i],YSGLSL_TEX_TYPE_TILING);
		}
		else
		{
			YsGLSLSet3DRendererTextureType(renderer[i],YSGLSL_TEX_TYPE_NONE);
		}
		YsGLSLSet3DRendererTextureIdentifier(renderer[i],0); // 0 for GL_TEXTURE0 apparently
		YsGLSLSet3DRendererUniformTextureTilingMatrixfv(renderer[i],texTfm);
		YsGLSLEndUse3DRenderer(renderer[i]);
	}



	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());


	YsMatrix4x4 shlTfm(YSFALSE);

	glDisable(GL_CULL_FACE);


	const YSBOOL wire=YSFALSE,fill=YSTRUE,drawBbx=YSFALSE;

	// Kamigotoh Runway Map at (-27899.6120179042, 80.0000000000, -35024.0065930447)
	// Kamigotoh Field Map at (-27896.9900000000, 80.0000000000, -35060.8300000000)
	// Do not seem to be in the same group.

	for(auto &samePlaneMapGroup : mapDrawingOrderCache.samePlaneMapGroup)
	{
		for(int i=0; i<2; ++i)
		{
			if(0==i)
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
				glDepthMask(GL_FALSE);
			}
			else
			{
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
				glDepthMask(GL_TRUE);
			}

			for(auto &mapDrawingInfo : samePlaneMapGroup.mapDrawingInfo)
			{
				auto viewModelTfm=viewTfm*mapDrawingInfo.mapOwnerToWorldTfm;
				if(IsItemVisible(viewModelTfm,projTfm,mapDrawingInfo.mapPtr)==YSTRUE)
				{
					shlTfm=viewModelTfm;
					shlTfm.Translate(mapDrawingInfo.mapPtr->pos);
					shlTfm.RotateXZ(mapDrawingInfo.mapPtr->att.h());
					shlTfm.RotateZY(mapDrawingInfo.mapPtr->att.p());
					shlTfm.RotateXY(mapDrawingInfo.mapPtr->att.b());

					SetUpMatrix::SetUpLF(mapDrawingInfo.mapPtr->pos,mapDrawingInfo.mapPtr->att,viewModelTfm);
					mapDrawingInfo.mapPtr->GetDrawing().Draw
					    (plgColorScale,linColorScale,pntColorScale,drawPset,YSTRUE,wire,fill,drawBbx,YSFALSE,
					     currentTime,&shlTfm);
				}
			}
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	SetUpMatrix::SetDirect(glMatPush);

	for(int i=0; NULL!=renderer[i]; ++i)
	{
		GLfloat texTfm[]=
		{
			0.025f,0.0f,  0.0f,  0,
			0.0f,  0.0f,  0.025f,0,
			0.0f,  0.025f,0.0f,  0,
			0.0f,  0.0f,  0.0f,  1
		};
		YsGLSLUse3DRenderer(renderer[i]);
		YsGLSLSet3DRendererTextureType(renderer[i],YSGLSL_TEX_TYPE_NONE);
		YsGLSLEndUse3DRenderer(renderer[i]);
	}
}


void YsScenery::DrawAxis
   (const double &axsSize,
    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsDisregardVariable(axsSize);
    YsDisregardVariable(drawShl);
    YsDisregardVariable(drawEvg);
    YsDisregardVariable(drawMap);
    YsDisregardVariable(drawSbd);
    YsDisregardVariable(drawRgn);
    YsDisregardVariable(drawGndObj);
    YsDisregardVariable(drawAir);
    YsDisregardVariable(drawPst);
    YsDisregardVariable(drawScn);

//	YsListItem <YsSceneryShell> *shl;
//	YsListItem <YsSceneryElevationGrid> *evg;
//	YsListItem <YsScenery2DDrawing> *drw;
//	YsListItem <YsSceneryRectRegion> *rgn;
//	YsListItem <YsScenery> *scn;
//	YsListItem <YsSceneryGndObj> *gnd;
//	YsListItem <YsSceneryAir> *air;
//	YsListItem <YsSceneryPointSet> *pst;
//
//
//	if(drawMap==YSTRUE)
//	{
//		YsScenery::GlSetColor(YsMagenta());
//		drw=NULL;
//		while((drw=mapList.FindNext(drw))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(drw->dat.searchKey);
//			YsGlMulMatrix(drw->dat.pos,drw->dat.att);
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//
//
//	if(drawShl==YSTRUE)
//	{
//		YsScenery::GlSetColor(YsWhite());
//		shl=NULL;
//		while((shl=shlList.FindNext(shl))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(shl->dat.searchKey);
//			YsGlMulMatrix(shl->dat.pos,shl->dat.att);
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//	if(drawEvg==YSTRUE)
//	{
//		YsScenery::GlSetColor(YsCyan());
//		evg=NULL;
//		while((evg=evgList.FindNext(evg))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(evg->dat.searchKey);
//			YsGlMulMatrix(evg->dat.pos,evg->dat.att);
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//	if(drawSbd==YSTRUE)
//	{
//		YsScenery::GlSetColor(YsMagenta());
//		drw=NULL;
//		while((drw=sbdList.FindNext(drw))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(drw->dat.searchKey);
//			YsGlMulMatrix(drw->dat.pos,drw->dat.att);
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//	if(drawRgn==YSTRUE)
//	{
//		YsScenery::GlSetColor(YsGreen());
//		rgn=NULL;
//		while((rgn=rgnList.FindNext(rgn))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(rgn->dat.searchKey);
//			YsGlMulMatrix(rgn->dat.pos,rgn->dat.att);
//			YsScenery::GlSetColor(YsThreeBitColor(rgn->dat.id%8));
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//	if(drawGndObj==YSTRUE)
//	{
//		gnd=NULL;
//		while((gnd=gndList.FindNext(gnd))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(gnd->dat.searchKey);
//			YsGlMulMatrix(gnd->dat.pos,gnd->dat.att);
//			YsScenery::GlSetColor(YsThreeBitColor(gnd->dat.iff+1));
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//	if(drawAir==YSTRUE)
//	{
//		air=NULL;
//		while((air=airList.FindNext(air))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(air->dat.searchKey);
//			YsGlMulMatrix(air->dat.pos,air->dat.att);
//			YsScenery::GlSetColor(YsThreeBitColor(air->dat.iff+1));
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//	if(drawPst==YSTRUE)
//	{
//		pst=NULL;
//		while((pst=pstList.FindNext(pst))!=NULL)
//		{
//			glPushMatrix();
//			glLoadName(pst->dat.searchKey);
//			YsGlMulMatrix(pst->dat.pos,pst->dat.att);
//			YsScenery::GlSetColor(YsGreen());
//			YsGlDrawAxis(axsSize);
//			glPopMatrix();
//		}
//	}
//
//	scn=NULL;
//	while((scn=scnList.FindNext(scn))!=NULL)
//	{
//		YsScenery::GlSetColor(YsDarkYellow());
//		glPushMatrix();
//		YsGlMulMatrix(scn->dat.pos,scn->dat.att);
//		glLoadName(scn->dat.searchKey);
//		if(drawScn==YSTRUE)
//		{
//			YsGlDrawAxis(axsSize);
//		}
//		scn->dat.DrawAxis(axsSize,drawShl,drawEvg,drawMap,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);
//		glPopMatrix();
//	}
}

void YsScenery::DrawItemAxis(const YsSceneryItem *itm,const double &axsSize)
{
	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());


	YsArray <YsScenery *,16> parentLink;


	YsScenery *scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}

	YsMatrix4x4 tfm;
	for(auto i=parentLink.GetN()-1; i>=0; i--)
	{
		tfm.Multiply(parentLink[i]->pos,parentLink[i]->att);
	}
	tfm.Multiply(itm->pos,itm->att);

	double glMat[16];
	tfm.GetOpenGlCompatibleMatrix(glMat);
	SetUpMatrix::SetDirect(glMat);

	YsGlDrawAxis(axsSize);

	SetUpMatrix::SetDirect(glMatPush);
}

void YsScenery::DrawItem
   (const YsSceneryItem *itm,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
    YSBOOL name2DElem,YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());

	YsArray <YsScenery *,16> parentLink;


	YsScenery *scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}

	YsMatrix4x4 tfm;
	for(auto i=parentLink.GetN()-1; i>=0; i--)
	{
		tfm.Multiply(parentLink[i]->pos,parentLink[i]->att);
	}
	tfm.Multiply(itm->pos,itm->att);

	double glMat[16];
	tfm.GetOpenGlCompatibleMatrix(glMat);
	SetUpMatrix::SetDirect(glMat);

	switch(itm->objType)
	{
	case YsSceneryItem::SHELL:
		((YsSceneryShell *)itm)->Draw(wire,fill,drawBbx);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		((YsSceneryElevationGrid *)itm)->evg.Draw
		    (plgColorScale,YSFALSE,wire,fill,drawBbx,drawShrink,nameElvGridFace,nameElvGridNode);
		break;
	case YsSceneryItem::MAP:
		((YsScenery2DDrawing *)itm)->drw.Draw
		    (plgColorScale,linColorScale,pntColorScale,YSTRUE,YSTRUE,wire,fill,drawBbx,name2DElem,-1.0);
		break;
	case YsSceneryItem::SIGNBOARD:
		((YsScenery2DDrawing *)itm)->drw.Draw
		    (plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,name2DElem,-1.0);
		break;
	case YsSceneryItem::RECTREGION:
		YsScenery::GlSetColor(YsThreeBitColor(((YsSceneryRectRegion *)itm)->id%8));
		((YsSceneryRectRegion *)itm)->Draw();
		break;
	case YsSceneryItem::GROUNDOBJECT:
		((YsSceneryGndObj *)itm)->Draw(wire,fill,drawBbx);
		break;
	case YsSceneryItem::AIRCRAFT:
		((YsSceneryAir *)itm)->Draw(wire,fill,drawBbx);
		break;
	case YsSceneryItem::SUBSCENARY:
		if(drawBbx==YSTRUE)
		{
			((YsScenery *)itm)->DrawBoundingBox();
		}
		break;
	case YsSceneryItem::POINTSET:
		((YsSceneryPointSet *)itm)->Draw();
		break;
	}

	SetUpMatrix::SetDirect(glMatPush);
}

void YsScenery::DrawItemStar(const YsSceneryItem *itm)
{
	double glMatPush[16];
	YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());

	YsArray <YsScenery *,16> parentLink;

	YsScenery *scn;
	scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}

	YsMatrix4x4 tfm;
	for(auto i=parentLink.GetN()-1; i>=0; i--)
	{
		tfm.Multiply(parentLink[i]->pos,parentLink[i]->att);
	}
	tfm.Multiply(itm->pos,itm->att);

	double glMat[16];
	tfm.GetOpenGlCompatibleMatrix(glMat);
	SetUpMatrix::SetDirect(glMat);

	if(itm->objType==YsSceneryItem::POINTSET)
	{
		((YsSceneryPointSet *)itm)->DrawStar();
	}

	SetUpMatrix::SetDirect(glMatPush);
}

void YsScenery::Draw2DDrawingElement
   (YsScenery2DDrawing *drw,const Ys2DDrawingElement *itm,
    YSBOOL nameVtId,YSBOOL wire,YSBOOL points)
{
	YsMatrix4x4 mat;
	YSBOOL mapMode;
	if(GetTransformation(mat,drw)==YSOK)
	{
		int i;


		double glMatPush[16];
		YsGLSLGet3DRendererModelViewdv(glMatPush,YsGLSLSharedVariColor3DRenderer());

		YsMatrix4x4 tfm;
		tfm.CreateFromOpenGlCompatibleMatrix(glMatPush);
		tfm*=mat;

		double glMat[16];
		tfm.GetOpenGlCompatibleMatrix(glMat);

		SetUpMatrix::SetDirect(glMat);

		if(drw->objType==YsScenery2DDrawing::MAP)
		{
			mapMode=YSTRUE;
		}
		else
		{
			mapMode=YSFALSE;
		}

		if(wire==YSTRUE)
		{
			int primType=0;

			switch(itm->t)
			{
			case Ys2DDrawingElement::POINTS:
				primType=GL_POINTS;
				break;
			case Ys2DDrawingElement::LINESEGMENTS:
			case Ys2DDrawingElement::GRADATIONQUADSTRIP:
			case Ys2DDrawingElement::QUADSTRIP:
			case Ys2DDrawingElement::QUADS:
			case Ys2DDrawingElement::TRIANGLES:
			case Ys2DDrawingElement::APPROACHLIGHT:
				primType=GL_LINE_STRIP;
				break;
			case Ys2DDrawingElement::LINES:
				primType=GL_LINES;
				break;
			default:
			case Ys2DDrawingElement::POLYGON:
				primType=GL_LINE_LOOP;
				break;
			}

			std::vector <float> pnt;
			for(i=0; i<itm->pnt.GetN(); i++)
			{
				if(mapMode==YSTRUE)
				{
					pnt.push_back(itm->pnt[i].xf());
					pnt.push_back(0);
					pnt.push_back(itm->pnt[i].yf());
				}
				else
				{
					pnt.push_back(itm->pnt[i].xf());
					pnt.push_back(itm->pnt[i].yf());
					pnt.push_back(0);
				}
			}

			auto renderer=YsGLSLSharedVariColor3DRenderer();
			YsGLSLUse3DRenderer(renderer);
			YsGLSLDrawPrimitiveVtxfv(renderer,primType,pnt.size()/3,pnt.data());
			YsGLSLEndUse3DRenderer(renderer);
		}

		if(points==YSTRUE)
		{
			std::vector <float> pnt;
			for(i=0; i<itm->pnt.GetN(); i++)
			{
				if(mapMode==YSTRUE)
				{
					pnt.push_back(itm->pnt[i].xf());
					pnt.push_back(0);
					pnt.push_back(itm->pnt[i].yf());
				}
				else
				{
					pnt.push_back(itm->pnt[i].xf());
					pnt.push_back(itm->pnt[i].yf());
					pnt.push_back(0);
				}
			}
			auto renderer=YsGLSLSharedVariColor3DRenderer();
			YsGLSLUse3DRenderer(renderer);
			YsGLSLDrawPrimitiveVtxfv(renderer,GL_POINTS,pnt.size()/3,pnt.data());
			YsGLSLEndUse3DRenderer(renderer);
		}

		if(nameVtId==YSTRUE)
		{
		}

		SetUpMatrix::SetDirect(glMatPush);
	}
}

void YsScenery::DrawILSGuideline(void)
{
//	YsListItem <YsSceneryGndObj> *gnd;
//
//	YsGlSetColor(YsGreen());
//
//	glPushMatrix();
//
//	gnd=NULL;
//	while((gnd=gndList.FindNext(gnd))!=NULL)
//	{
//		if(gnd->dat.gndObjTemplate!=NULL && gnd->dat.gndObjTemplate->ilsRange>YsTolerance)
//		{
//			glPushMatrix();
//			YsGlMulMatrix(gnd->dat.pos,gnd->dat.att);
//			YsGlMulMatrix(gnd->dat.gndObjTemplate->ilsPos,gnd->dat.gndObjTemplate->ilsAtt);
//
//			YsVec3 p1,p2;
//			p1.Set(0.0,0.0,1000.0);
//			p2.Set(0.0,0.0,500.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			p1.Set(0.0,0.0,500.0);
//			p2.Set(0.0,0.0,0.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			p1.Set(0.0,0.0,500.0);
//			p2.Set(0.0,0.0,0.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			glEnable(GL_LINE_STIPPLE);
//			glLineStipple(1,0xf0f0);
//
//			p1.Set(0.0,0.0,0.0);
//			p2.Set(0.0,0.0,-500.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			p1.Set(0.0,0.0,-500.0);
//			p2.Set(0.0,0.0,-1000.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			p1.Set(0.0,0.0,-1000.0);
//			p2.Set(0.0,0.0,-1500.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			p1.Set(0.0,0.0,-1500.0);
//			p2.Set(0.0,0.0,-2000.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			p1.Set(0.0,0.0,-2000.0);
//			p2.Set(0.0,0.0,-2500.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			p1.Set(0.0,0.0,-2500.0);
//			p2.Set(0.0,0.0,-3000.0);
//			YsGlDrawArrow(p1,p2,0.05);
//
//			glDisable(GL_LINE_STIPPLE);
//
//			glPopMatrix();
//		}
//	}
//
//	YsListItem <YsScenery> *scn;
//	scn=NULL;
//	while((scn=scnList.FindNext(scn))!=NULL)
//	{
//		glPushMatrix();
//		YsGlMulMatrix(scn->dat.pos,scn->dat.att);
//		scn->dat.DrawILSGuideline();
//		glPopMatrix();
//	}
//
//	glPopMatrix();
}

void YsScenery::DrawItemILSGuideline(YsSceneryGndObj *gnd)
{
	YsDisregardVariable(gnd);
//	if(gnd->gndObjTemplate!=NULL && gnd->gndObjTemplate->ilsRange>YsTolerance)
//	{
//		YsArray <YsScenery *,16> parentLink;
//		int i;
//
//		glPushMatrix();
//
//		YsScenery *scn;
//		scn=gnd->owner;
//		while(scn!=NULL && scn->owner!=NULL)
//		{
//			parentLink.Append(scn);
//			scn=scn->owner;
//		}
//		for(i=parentLink.GetN()-1; i>=0; i--)
//		{
//			YsGlMulMatrix(parentLink[i]->pos,parentLink[i]->att);
//		}
//
//		YsGlMulMatrix(gnd->pos,gnd->att);
//		YsGlMulMatrix(gnd->gndObjTemplate->ilsPos,gnd->gndObjTemplate->ilsAtt);
//
//		YsVec3 p1,p2;
//		p1.Set(0.0,0.0,1000.0);
//		p2.Set(0.0,0.0,500.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		p1.Set(0.0,0.0,500.0);
//		p2.Set(0.0,0.0,0.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		p1.Set(0.0,0.0,500.0);
//		p2.Set(0.0,0.0,0.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		glEnable(GL_LINE_STIPPLE);
//		glLineStipple(1,0xf0f0);
//
//		p1.Set(0.0,0.0,0.0);
//		p2.Set(0.0,0.0,-500.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		p1.Set(0.0,0.0,-500.0);
//		p2.Set(0.0,0.0,-1000.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		p1.Set(0.0,0.0,-1000.0);
//		p2.Set(0.0,0.0,-1500.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		p1.Set(0.0,0.0,-1500.0);
//		p2.Set(0.0,0.0,-2000.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		p1.Set(0.0,0.0,-2000.0);
//		p2.Set(0.0,0.0,-2500.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		p1.Set(0.0,0.0,-2500.0);
//		p2.Set(0.0,0.0,-3000.0);
//		YsGlDrawArrow(p1,p2,0.05);
//
//		glDisable(GL_LINE_STIPPLE);
//
//		glPopMatrix();
//	}
}
