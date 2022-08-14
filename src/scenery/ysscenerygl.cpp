#include <ysclass.h>

#include <ysshelldnmident.h>

#ifdef WIN32
#include <windows.h>
#else
#define CALLBACK
#endif

#include <ysgl.h>

#include "ysscenery.h"
#include "ysgltess.h"

#include <ystexturemanager_gl.h>

// YSBOOL ysScnGlUseRwLightTexture=YSFALSE;
// GLuint ysScnGlRwLightTex=~(unsigned int)0;
// 
// YSBOOL ysScnGlUseMapTexture=YSFALSE;
// GLuint ysScnGlMapTex=~(unsigned int)0;

int ysScnFontBitmapBase=-1;

static inline void FsResetMaterial(void)
{
// Ubuntu 11.1's OpenGL driver bugged.  It forgets material very often.
#if !defined(__APPLE__) && !defined(_WIN32)
	float zero[4]={0,0,0,0};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,zero);
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,zero);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,zero);

	int shininess[1]={0};
	glMaterialiv(GL_FRONT_AND_BACK,GL_SHININESS,shininess);
#endif
}

static void ysGlPrint(const char str[])
{
	if(ysScnFontBitmapBase<0)
	{
		fprintf(stderr,"Error!\n");
		fprintf(stderr,"ysScnFontBitmapBase is not set.\n");
	}

	size_t l;
	GLint listbase;

	l=strlen(str);

	glGetIntegerv(GL_LIST_BASE,&listbase);
	if(listbase!=ysScnFontBitmapBase)
	{
		glListBase(ysScnFontBitmapBase);
	}

	glPushMatrix();
	glCallLists(l,GL_UNSIGNED_BYTE,str);
	glPopMatrix();
	if(listbase!=ysScnFontBitmapBase)
	{
		glListBase(listbase);
	}
};

static void YsGlDrawArrow(const YsVec3 &p1,const YsVec3 &p2,const double &arrowSize)
{
	glBegin(GL_LINES);
	glVertex3dv(p1.GetValue());
	glVertex3dv(p2.GetValue());
	glEnd();

	YsVec3 a1,a2,a3,a4,v,w,o;
	double arrowLen;

	arrowLen=(p2-p1).GetLength();

	v=p2-p1;
	v*=arrowSize;

	o=p2-v;
	w=v.GetArbitraryPerpendicularVector();
	w.Normalize();
	w*=arrowLen*arrowSize*0.5;
	a1=o+w;
	a2=o-w;

	v.Normalize();
	w=w^v;
	w.Normalize();
	w*=arrowLen*arrowSize*0.5;
	a3=o+w;
	a4=o-w;

	glBegin(GL_POLYGON);
	glVertex3dv(a1.GetValue());
	glVertex3dv(a2.GetValue());
	glVertex3dv(p2.GetValue());
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3dv(a3.GetValue());
	glVertex3dv(a4.GetValue());
	glVertex3dv(p2.GetValue());
	glEnd();
}

static void YsGlDrawArrowAbsoluteSize(const YsVec3 &p1,const YsVec3 &p2,const double &arrowSize)
{
	glBegin(GL_LINES);
	glVertex3dv(p1.GetValue());
	glVertex3dv(p2.GetValue());
	glEnd();

	YsVec3 a1,a2,a3,a4,v,w,o;

	v=YsUnitVector(p2-p1);

	o=p2-v*arrowSize;
	w=v.GetArbitraryPerpendicularVector();
	w.Normalize();
	w*=arrowSize*0.5;
	a1=o+w;
	a2=o-w;

	v.Normalize();
	w=w^v;
	w.Normalize();
	w*=arrowSize*0.5;
	a3=o+w;
	a4=o-w;

	glBegin(GL_POLYGON);
	glVertex3dv(a1.GetValue());
	glVertex3dv(a2.GetValue());
	glVertex3dv(p2.GetValue());
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3dv(a3.GetValue());
	glVertex3dv(a4.GetValue());
	glVertex3dv(p2.GetValue());
	glEnd();
}

static void YsGlSetColor(const YsColor &col,const double &alpha=1.0)
{
	glDisable(GL_LIGHTING);
	// glDisable(GL_LIGHT0);

	glColor4d(col.Rd(),col.Gd(),col.Bd(),alpha);
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


void YsGlMulMatrix(const YsVec3 &tra,const YsAtt3 &att)
{
	glTranslated(tra.x(),tra.y(),tra.z());
	glRotated(-YsRadToDeg(att.h()),0.0,1.0,0.0);
	glRotated(-YsRadToDeg(att.p()),1.0,0.0,0.0);
	glRotated( YsRadToDeg(att.b()),0.0,0.0,1.0);

	// YsMatrix4x4 tfm;
	// double mat[16];
	// tfm.Initialize();
	// tfm.Translate(tra);
	// tfm.RotateXZ(att.h());
	// tfm.RotateZY(att.p());
	// tfm.RotateXY(att.b());
	// tfm.GetOpenGlCompatibleMatrix(mat);
	// glMultMatrixd(mat);
}

void YsGlDrawAxis(const double &axsSize)
{
	YsVec3 p;
	p.Set(axsSize,0.0,0.0);
	YsGlDrawArrow(YsOrigin(),p,0.1);
	p.Set(0.0,axsSize,0.0);
	YsGlDrawArrow(YsOrigin(),p,0.05);
	p.Set(0.0,0.0,axsSize);
	YsGlDrawArrow(YsOrigin(),p,0.05);

	glPointSize(5.0);
	glBegin(GL_POINTS);
	glVertex3dv(YsOrigin());
	glEnd();
	glPointSize(1.0);
}

static const YsColor YsScaleColor(const YsColor &col,const double &scl)
{
	YsColor newCol;
	newCol.SetDoubleRGB(col.Rd()*scl,col.Gd()*scl,col.Bd()*scl);
	return newCol;
}

static const int maxNumTessVtx=1024;
static GLdouble GlTessVtx[maxNumTessVtx*3];

void Ys2DDrawing::AllocCache(void) const
{
}

void Ys2DDrawing::DeleteCache(void) const
{
}

YSBOOL Ys2DDrawing::IsCached(void) const
{
	return YSFALSE;
}


void Ys2DDrawing::Draw
    (const double &plgColScale,const double &linColorScale,const double &pntColorScale,
     YSBOOL drawPset,
     YSBOOL mapMode,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL name2DElem,
     const double &currentTime,
     YsMatrix4x4 *viewModelTfm)
{
	const YsTextureManager *commonTexManPtr=YsScenery::commonTexManPtr;
	const YsTextureManager::Unit *commonGndTexUnitPtr=nullptr;
	const YsTextureManager::Unit *commonRwLightTexUnitPtr=nullptr;
	if(nullptr!=commonTexManPtr)
	{
		commonGndTexUnitPtr=commonTexManPtr->GetTextureReady(YsScenery::commonGroundTexHd);
		commonRwLightTexUnitPtr=commonTexManPtr->GetTextureReady(YsScenery::commonRunwayLightTexHd);
	}

	YsVec3 viewX=YsVec3::Origin(),viewY=YsVec3::Origin();
	YsColor color,color2,prevColor;
	YsListItem <Ys2DDrawingElement> *itm;
	itm=NULL;
	GLenum curShadeMode;

	glShadeModel(GL_FLAT);
	curShadeMode=GL_FLAT;

	prevColor=YsBlue();
	glDisable(GL_LIGHTING);
	YsScenery::GlSetColor(YsBlue());

	if(mapMode==YSTRUE)
	{
		if(nullptr!=viewModelTfm)
		{
			YsMatrix3x3 inv(YSFALSE);
			inv.Set(1,1,viewModelTfm->v(1,1));
			inv.Set(1,2,viewModelTfm->v(2,1));
			inv.Set(1,3,viewModelTfm->v(3,1));
			inv.Set(2,1,viewModelTfm->v(1,2));
			inv.Set(2,2,viewModelTfm->v(2,2));
			inv.Set(2,3,viewModelTfm->v(3,2));
			inv.Set(3,1,viewModelTfm->v(1,3));
			inv.Set(3,2,viewModelTfm->v(2,3));
			inv.Set(3,3,viewModelTfm->v(3,3));
			inv.Mul(viewX,YsXVec()*0.22);
			inv.Mul(viewY,YsYVec()*0.22);
		}
	}
	while((itm=elemList.FindNext(itm))!=NULL)
	{
		int i;
		double t;
		YSBOOL useTextureCoord=YSFALSE;

		if(name2DElem==YSTRUE)
		{
			glLoadName(itm->dat.searchKey);
		}

		if(drawPset!=YSTRUE && itm->dat.t==Ys2DDrawingElement::POINTS)
		{
			continue;
		}

		if(viewModelTfm!=NULL && YsTolerance<itm->dat.VisibleDist())
		{
			YsVec3 cen;
			if(mapMode==YSTRUE)
			{
				cen.SetXZ((itm->dat.bbx[0]+itm->dat.bbx[1])/2.0);
				cen.SetY(0.0);
			}
			else
			{
				cen.SetXY((itm->dat.bbx[0]+itm->dat.bbx[1])/2.0);
				cen.SetZ(0.0);
			}
			viewModelTfm->Mul(cen,cen,1.0);
			if(YsSqr(itm->dat.VisibleDist())<cen.GetSquareLength())
			{
				continue;
			}
		}

		if(itm->dat.t==Ys2DDrawingElement::GRADATIONQUADSTRIP && curShadeMode==GL_FLAT)
		{
			glShadeModel(GL_SMOOTH);
			curShadeMode=GL_SMOOTH;
		}
		else if(itm->dat.t!=Ys2DDrawingElement::GRADATIONQUADSTRIP && curShadeMode!=GL_FLAT)
		{
			glShadeModel(GL_FLAT);
			curShadeMode=GL_FLAT;
		}

		if(0<itm->dat.texLabel.Strlen() && YSTRUE!=itm->dat.texLabelNotFound && NULL==itm->dat.texManCache && NULL!=owner)
		{
			itm->dat.TryCacheTexture(owner->GetOwner());
		}

		const YsTextureManager::Unit *ownTexUnitPtr=nullptr;
		if(NULL!=itm->dat.texManCache && NULL!=itm->dat.texHdCache)
		{
			ownTexUnitPtr=itm->dat.texManCache->GetTextureReady(itm->dat.texHdCache);
		}

		if(nullptr!=ownTexUnitPtr)
		{
			auto tex=itm->dat.texManCache->GetTexture(itm->dat.texHdCache);
			useTextureCoord=(YSOK==tex->Bind() ? YSTRUE : YSFALSE);
		}
		else if(nullptr!=commonGndTexUnitPtr)
		{
			switch(itm->dat.t)
			{
			default:
				break;
			case Ys2DDrawingElement::POLYGON:
			case Ys2DDrawingElement::GRADATIONQUADSTRIP:
			case Ys2DDrawingElement::QUADSTRIP:
			case Ys2DDrawingElement::QUADS:
			case Ys2DDrawingElement::TRIANGLES:
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

				commonGndTexUnitPtr->Bind();

				// 2016/11/19 Bind() is disabling GL_TEXTURE_GEN.  Did I do it on purpose?  Re-enable here for now.
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				glEnable(GL_TEXTURE_2D);

				break;
			}
		}


		const float ps=YsScenery::lightPointSize3d;


		switch(itm->dat.t)
		{
		default:
			goto NEXTOBJ;

		case Ys2DDrawingElement::APPROACHLIGHT:
			color=YsScaleColor(itm->dat.c,pntColorScale);
			if(color.Ri()!=prevColor.Ri() || color.Gi()!=prevColor.Gi() || color.Bi()!=prevColor.Bi())
			{
				YsScenery::GlSetColor(color);
				prevColor=color;
			}

			if(currentTime>=0.0)
			{
				glBegin(GL_POINTS);
				t=fmod(currentTime,0.5)/0.5;
				if(itm->dat.pnt.GetN()>2)
				{
					i=(int)((double)itm->dat.pnt.GetN()*t);
					i=YsBound <int> (i,0,(int)itm->dat.pnt.GetN()-1);
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
						glVertex3d(itm->dat.pnt[i].x()+ps , 0.0,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()-ps , 0.0,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()    , ps ,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()    ,-ps ,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()    , 0.0,itm->dat.pnt[i].y()+ps );
						glVertex3d(itm->dat.pnt[i].x()    , 0.0,itm->dat.pnt[i].y()-ps );
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				else if(t<0.1)
				{
					for(i=0; i<itm->dat.pnt.GetN(); i++)
					{
						if(mapMode==YSTRUE)
						{
							glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
							glVertex3d(itm->dat.pnt[i].x()+ps , 0.0,itm->dat.pnt[i].y()+0.0);
							glVertex3d(itm->dat.pnt[i].x()-ps , 0.0,itm->dat.pnt[i].y()+0.0);
							glVertex3d(itm->dat.pnt[i].x()    , ps ,itm->dat.pnt[i].y()+0.0);
							glVertex3d(itm->dat.pnt[i].x()    ,-ps ,itm->dat.pnt[i].y()+0.0);
							glVertex3d(itm->dat.pnt[i].x()    , 0.0,itm->dat.pnt[i].y()+ps );
							glVertex3d(itm->dat.pnt[i].x()    , 0.0,itm->dat.pnt[i].y()-ps );
						}
						else
						{
							glVertex2dv(itm->dat.pnt[i]);
						}
					}
				}
				glEnd();
			}
			else
			{
				glBegin(GL_POINTS);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
						glVertex3d(itm->dat.pnt[i].x()+ps , 0.0,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()-ps , 0.0,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()    , ps ,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()    ,-ps ,itm->dat.pnt[i].y()+0.0);
						glVertex3d(itm->dat.pnt[i].x()    , 0.0,itm->dat.pnt[i].y()+ps );
						glVertex3d(itm->dat.pnt[i].x()    , 0.0,itm->dat.pnt[i].y()-ps );
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}

			break;

		case Ys2DDrawingElement::POINTS:
		case Ys2DDrawingElement::LINESEGMENTS:
		case Ys2DDrawingElement::LINES:
			switch(itm->dat.t)
			{
			case Ys2DDrawingElement::POINTS:
				color=YsScaleColor(itm->dat.c,pntColorScale);
				glBegin(GL_POINTS);
				break;
			case Ys2DDrawingElement::LINESEGMENTS:
				color=YsScaleColor(itm->dat.c,linColorScale);
				glBegin(GL_LINE_STRIP);
				break;
			case Ys2DDrawingElement::LINES:
				color=YsScaleColor(itm->dat.c,linColorScale);
				glBegin(GL_LINES);
				break;
			}

			if(color.Ri()!=prevColor.Ri() || color.Gi()!=prevColor.Gi() || color.Bi()!=prevColor.Bi())
			{
				YsScenery::GlSetColor(color);
				prevColor=color;
			}

			for(i=0; i<itm->dat.pnt.GetN(); i++)
			{
				if(mapMode==YSTRUE)
				{
					glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
				}
				else
				{
					glVertex2dv(itm->dat.pnt[i]);
				}
			}
			glEnd();


			if(mapMode==YSTRUE && 
			   itm->dat.t==Ys2DDrawingElement::POINTS &&
			   nullptr!=commonRwLightTexUnitPtr)
			{
				glEnable(GL_TEXTURE_2D);
				//glEnable(GL_BLEND);
				//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

				commonRwLightTexUnitPtr->Bind();

				glBegin(GL_QUADS);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					YsVec3 p(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					glTexCoord2d(0.0,0.0);
					glVertex3dv(p-viewX-viewY);
					glTexCoord2d(1.0,0.0);
					glVertex3dv(p+viewX-viewY);
					glTexCoord2d(1.0,1.0);
					glVertex3dv(p+viewX+viewY);
					glTexCoord2d(0.0,1.0);
					glVertex3dv(p-viewX+viewY);
				}
				glEnd();


				//glDisable(GL_BLEND);
				glDisable(GL_TEXTURE_2D);
			}
			break;
		case Ys2DDrawingElement::POLYGON:
			color=YsScaleColor(itm->dat.c,plgColScale);
			if(color.Ri()!=prevColor.Ri() || color.Gi()!=prevColor.Gi() || color.Bi()!=prevColor.Bi())
			{
				YsScenery::GlSetColor(color);
				prevColor=color;
			}

			if(wire==YSTRUE)
			{
				glBegin(GL_LINE_LOOP);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}

			if(fill==YSTRUE)
			{
				if(itm->dat.cvx==YSTRUE)
				{
					glBegin(GL_POLYGON);
					for(i=0; i<itm->dat.pnt.GetN(); i++)
					{
						if(YSTRUE==useTextureCoord && i<itm->dat.texCoord.GetN())
						{
							glTexCoord2dv(itm->dat.texCoord[i]);
						}

						if(mapMode==YSTRUE)
						{
							glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
						}
						else
						{
							glVertex2dv(itm->dat.pnt[i]);
						}
					}
					glEnd();
				}
				else
				{
					GLUtriangulatorObj *GlTessObj;
					GlTessObj=YsGlGetTessObj();
					if(GlTessObj!=NULL)
					{
						gluTessCallback(GlTessObj,GLU_BEGIN, (void (CALLBACK*)())glBegin);
						gluTessCallback(GlTessObj,GLU_VERTEX,(void (CALLBACK*)())glVertex3dv);
						gluTessCallback(GlTessObj,GLU_END,   (void (CALLBACK*)())glEnd);

						gluBeginPolygon(GlTessObj);
						for(i=0; i<itm->dat.pnt.GetN() && i<maxNumTessVtx; i++)
						{
							if(mapMode==YSTRUE)
							{
								GlTessVtx[i*3  ]=itm->dat.pnt[i].x();
								GlTessVtx[i*3+1]=0.0;
								GlTessVtx[i*3+2]=itm->dat.pnt[i].y();
							}
							else
							{
								GlTessVtx[i*3  ]=itm->dat.pnt[i].x();
								GlTessVtx[i*3+1]=itm->dat.pnt[i].y();
								GlTessVtx[i*3+2]=0.0;
							}
							gluTessVertex(GlTessObj,&GlTessVtx[i*3],&GlTessVtx[i*3]);
						}
						gluEndPolygon(GlTessObj);
					}
				}
			}
			break;
		case Ys2DDrawingElement::GRADATIONQUADSTRIP:
			color=YsScaleColor(itm->dat.c,plgColScale);
			color2=YsScaleColor(itm->dat.c2,plgColScale);

			if(wire==YSTRUE)
			{
				YsScenery::GlSetColor(color);
				glBegin(GL_LINE_STRIP);
				for(i=0; i<itm->dat.pnt.GetN()-1; i+=2)
				{
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();

				YsScenery::GlSetColor(color2);
				glBegin(GL_LINE_STRIP);
				for(i=1; i<itm->dat.pnt.GetN(); i+=2)
				{
					YsScenery::GlSetColor((i&1) ? color2 : color);
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();

				glBegin(GL_LINES);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					YsScenery::GlSetColor((i&1) ? color2 : color);
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
				YsScenery::GlSetColor(color2);
				prevColor=color2;
			}

			if(fill==YSTRUE)
			{
				YsScenery::GlSetColor(color);
				glBegin(GL_QUAD_STRIP);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					YsScenery::GlSetColor((i&1) ? color2 : color);
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
				YsScenery::GlSetColor(color2);
				prevColor=color2;
			}
			break;
		case Ys2DDrawingElement::QUADSTRIP:
			color=YsScaleColor(itm->dat.c,plgColScale);
			if(color.Ri()!=prevColor.Ri() || color.Gi()!=prevColor.Gi() || color.Bi()!=prevColor.Bi())
			{
				YsScenery::GlSetColor(color);
				prevColor=color;
			}

			if(wire==YSTRUE)
			{
				glBegin(GL_LINE_LOOP);
				for(i=0; i<itm->dat.pnt.GetN()-1; i+=2)
				{
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				for(i=(itm->dat.pnt.GetN()&~1)-1; i>=0; i-=2)
				{
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}

			if(fill==YSTRUE)
			{
				glBegin(GL_QUAD_STRIP);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}
			break;
		case Ys2DDrawingElement::QUADS:
			color=YsScaleColor(itm->dat.c,plgColScale);
			if(color.Ri()!=prevColor.Ri() || color.Gi()!=prevColor.Gi() || color.Bi()!=prevColor.Bi())
			{
				YsScenery::GlSetColor(color);
				prevColor=color;
			}

			if(wire==YSTRUE)
			{
				glBegin(GL_LINE_LOOP);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					if(i>0 && (i&3)==0)
					{
						glEnd();
						glBegin(GL_LINE_LOOP);
					}

					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}

			if(fill==YSTRUE)
			{
				glBegin(GL_QUADS);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					if(YSTRUE==useTextureCoord && i<itm->dat.texCoord.GetN())
					{
						glTexCoord2dv(itm->dat.texCoord[i]);
					}

					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}
			break;


		case Ys2DDrawingElement::TRIANGLES:
			color=YsScaleColor(itm->dat.c,plgColScale);
			if(color.Ri()!=prevColor.Ri() || color.Gi()!=prevColor.Gi() || color.Bi()!=prevColor.Bi())
			{
				YsScenery::GlSetColor(color);
				prevColor=color;
			}

			if(wire==YSTRUE)
			{
				glBegin(GL_LINE_LOOP);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					if(i>0 && (i%3)==0)
					{
						glEnd();
						glBegin(GL_LINE_LOOP);
					}

					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}

			if(fill==YSTRUE)
			{
				glBegin(GL_TRIANGLES);
				for(i=0; i<itm->dat.pnt.GetN(); i++)
				{
					if(YSTRUE==useTextureCoord && i<itm->dat.texCoord.GetN())
					{
						glTexCoord2dv(itm->dat.texCoord[i]);
					}

					if(mapMode==YSTRUE)
					{
						glVertex3d(itm->dat.pnt[i].x(),0.0,itm->dat.pnt[i].y());
					}
					else
					{
						glVertex2dv(itm->dat.pnt[i]);
					}
				}
				glEnd();
			}
			break;
		}

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);

	NEXTOBJ:
		;
	}
	if(drawBbx==YSTRUE)
	{
		DrawBoundingBox(mapMode);
	}
	if(mapMode==YSTRUE)
	{
	}

	glShadeModel(GL_SMOOTH);
}

void Ys2DDrawing::DrawBoundingBox(YSBOOL mapMode)
{
	YsVec2 inflatedBbx[2],dgn,XZ;
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
	}
}

void YsElevationGrid::AllocCache(void) const
{
}

void YsElevationGrid::DeleteCache(void) const
{
}

YSBOOL YsElevationGrid::IsCached(void) const
{
	return YSFALSE;
}

void YsElevationGrid::MakeCache(const double &,YSBOOL)
{
}

void YsElevationGrid::Draw(
    const double & /*plgColorScale*/,
    YSBOOL invert,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL shrinkTriangle,
    YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
	int i,j,baseIdx;
	double x,z;
	YsVec3 rc[4],*tri[6];


	glEnable(GL_LIGHTING);



	YSBOOL useTexture=YSFALSE;
	if(0<texLabel.Strlen() && YSTRUE!=texLabelNotFound && NULL==texManCache && NULL!=owner)
	{
		TryCacheTexture(owner->GetOwner());
	}
	if(NULL!=texManCache && NULL!=texHdCache)
	{
		auto tex=texManCache->GetTexture(texHdCache);
		useTexture=(YSOK==tex->Bind() ? YSTRUE : YSFALSE);
	}



	for(j=0; j<nz; j++)
	{
		const double t0=(double)j/(double)nz;
		const double t1=(double)(j+1)/(double)nz;

		z=double(j)*zWid;
		x=0.0;
		baseIdx=(nx+1)*j;

		//  1  3
		//
		//  0  2
		rc[2].Set(x,node[baseIdx     ].y,z);
		rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
		for(i=0; i<nx; i++)
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
			double triTexCoord[6][2];

			if(node[baseIdx+i].lup==YSTRUE)
			{
				// (3,1,2),(0,2,1)
				tri[0]=&rc[3];
				tri[1]=&rc[1];
				tri[2]=&rc[2];
				tri[3]=&rc[0];
				tri[4]=&rc[2];
				tri[5]=&rc[1];

				triTexCoord[0][0]=texCoord[3][0];
				triTexCoord[0][1]=texCoord[3][1];
				triTexCoord[1][0]=texCoord[1][0];
				triTexCoord[1][1]=texCoord[1][1];
				triTexCoord[2][0]=texCoord[2][0];
				triTexCoord[2][1]=texCoord[2][1];
				triTexCoord[3][0]=texCoord[0][0];
				triTexCoord[3][1]=texCoord[0][1];
				triTexCoord[4][0]=texCoord[2][0];
				triTexCoord[4][1]=texCoord[2][1];
				triTexCoord[5][0]=texCoord[1][0];
				triTexCoord[5][1]=texCoord[1][1];
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

				triTexCoord[0][0]=texCoord[1][0];
				triTexCoord[0][1]=texCoord[1][1];
				triTexCoord[1][0]=texCoord[0][0];
				triTexCoord[1][1]=texCoord[0][1];
				triTexCoord[2][0]=texCoord[3][0];
				triTexCoord[2][1]=texCoord[3][1];
				triTexCoord[3][0]=texCoord[2][0];
				triTexCoord[3][1]=texCoord[2][1];
				triTexCoord[4][0]=texCoord[3][0];
				triTexCoord[4][1]=texCoord[3][1];
				triTexCoord[5][0]=texCoord[0][0];
				triTexCoord[5][1]=texCoord[0][1];
			}
			int k;
			for(k=0; k<2; k++)
			{
				if(nameElvGridFace==YSTRUE)
				{
					unsigned name;
					name=(j*(nx+1)+i)*2+k;
					glLoadName(name);
				}

				YsVec3 nom[3];
				YsElvGridFaceId nodId[3];
				GetTriangleNodeId(nodId,i,j,k);
				nom[0]=node[(nx+1)*nodId[0].z+nodId[0].x].nomOfNode;
				nom[1]=node[(nx+1)*nodId[1].z+nodId[1].x].nomOfNode;
				nom[2]=node[(nx+1)*nodId[2].z+nodId[2].x].nomOfNode;


				if(node[baseIdx+i].visible[k]==YSTRUE)
				{
					if(fill==YSTRUE)
					{
						glNormal3dv(node[baseIdx+i].nom[k]);
						if(colorByElevation!=YSTRUE)
						{
							YsScenery::GlSetColor(node[baseIdx+i].c[k]);
							glBegin(GL_POLYGON);
							if(invert!=YSTRUE)
							{
								glTexCoord2d(triTexCoord[k*3][0],triTexCoord[k*3][1]);
								glNormal3dv(nom[0]);
								glVertex3dv(*tri[k*3]);
								glTexCoord2d(triTexCoord[k*3+1][0],triTexCoord[k*3+1][1]);
								glNormal3dv(nom[1]);
								glVertex3dv(*tri[k*3+1]);
								glTexCoord2d(triTexCoord[k*3+2][0],triTexCoord[k*3+2][1]);
								glNormal3dv(nom[2]);
								glVertex3dv(*tri[k*3+2]);
							}
							else
							{
								glTexCoord2d(triTexCoord[k*3+2][0],triTexCoord[k*3+2][1]);
								glNormal3dv(nom[2]);
								glVertex3dv(*tri[k*3+2]);
								glTexCoord2d(triTexCoord[k*3+1][0],triTexCoord[k*3+1][1]);
								glNormal3dv(nom[1]);
								glVertex3dv(*tri[k*3+1]);
								glTexCoord2d(triTexCoord[k*3+0][0],triTexCoord[k*3+0][1]);
								glNormal3dv(nom[0]);
								glVertex3dv(*tri[k*3]);
							}
							glEnd();
						}
						else
						{
							if(invert!=YSTRUE)
							{
								YsScenery::GlSetColor(ColorByElevation(tri[k*3]->y()));
								glBegin(GL_POLYGON);  // Call color material before glBegin
								glTexCoord2d(triTexCoord[k*3][0],triTexCoord[k*3][1]);
								glNormal3dv(nom[0]);
								glVertex3dv(*tri[k*3]);
								YsScenery::GlSetColor(ColorByElevation(tri[k*3+1]->y()));
								glTexCoord2d(triTexCoord[k*3+1][0],triTexCoord[k*3+1][1]);
								glNormal3dv(nom[1]);
								glVertex3dv(*tri[k*3+1]);
								YsScenery::GlSetColor(ColorByElevation(tri[k*3+2]->y()));
								glTexCoord2d(triTexCoord[k*3+2][0],triTexCoord[k*3+2][1]);
								glNormal3dv(nom[2]);
								glVertex3dv(*tri[k*3+2]);
								glEnd();
							}
							else
							{
								YsScenery::GlSetColor(ColorByElevation(tri[k*3+2]->y()));
								glBegin(GL_POLYGON);  // Call color material before glBegin
								glTexCoord2d(triTexCoord[k*3+2][0],triTexCoord[k*3+2][1]);
								glNormal3dv(nom[2]);
								glVertex3dv(*tri[k*3+2]);
								YsScenery::GlSetColor(ColorByElevation(tri[k*3+1]->y()));
								glTexCoord2d(triTexCoord[k*3+1][0],triTexCoord[k*3+1][1]);
								glNormal3dv(nom[1]);
								glVertex3dv(*tri[k*3+1]);
								YsScenery::GlSetColor(ColorByElevation(tri[k*3]->y()));
								glTexCoord2d(triTexCoord[k*3][0],triTexCoord[k*3][1]);
								glNormal3dv(nom[0]);
								glVertex3dv(*tri[k*3]);
								glEnd();
							}
						}
					}

					if(wire==YSTRUE)
					{
						if(YSTRUE==useTexture)
						{
							glDisable(GL_TEXTURE_2D);
						}

						YsScenery::GlSetColor(YsBlack()); // YsScenery::GlSetColor(node[baseIdx+i].c[k]);
						glBegin(GL_LINE_LOOP);
						glVertex3dv(*tri[k*3]);
						glVertex3dv(*tri[k*3+1]);
						glVertex3dv(*tri[k*3+2]);
						glEnd();

						if(YSTRUE==useTexture)
						{
							glEnable(GL_TEXTURE_2D);
						}
					}
				}

				if(shrinkTriangle==YSTRUE)
				{
					if(YSTRUE==useTexture)
					{
						glDisable(GL_TEXTURE_2D);
					}

					if(node[baseIdx+i].visible[k]!=YSTRUE)
					{
						glEnable(GL_LINE_STIPPLE);
						glLineStipple(1,0x3333);
					}

					YsVec3 shrink[3],cen;
					cen=((*tri[k*3])+(*tri[k*3+1])+(*tri[k*3+2]))/3.0;
					shrink[0]=cen+((*tri[k*3  ])-cen)*0.7;
					shrink[1]=cen+((*tri[k*3+1])-cen)*0.7;
					shrink[2]=cen+((*tri[k*3+2])-cen)*0.7;

					glDisable(GL_LIGHTING);
					glColor3i(0,0,0);

					glBegin(GL_LINE_LOOP);
					glVertex3dv(shrink[0]);
					glVertex3dv(shrink[1]);
					glVertex3dv(shrink[2]);
					glEnd();


					if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
					{
						shrink[0]=cen+((*tri[k*3  ])-cen)*0.4;
						shrink[1]=cen+((*tri[k*3+1])-cen)*0.4;
						shrink[2]=cen+((*tri[k*3+2])-cen)*0.4;

						glBegin(GL_LINE_LOOP);
						glVertex3dv(shrink[0]);
						glVertex3dv(shrink[1]);
						glVertex3dv(shrink[2]);
						glEnd();
					}


					glEnable(GL_LIGHTING);

					if(node[baseIdx+i].visible[k]!=YSTRUE)
					{
						glDisable(GL_LINE_STIPPLE);
					}

					if(YSTRUE==useTexture)
					{
						glEnable(GL_TEXTURE_2D);
					}
				}
			}
			x+=xWid;
		}
	}

	// Side-wall texture not supported yet.
	if(YSTRUE==useTexture)
	{
		glDisable(GL_TEXTURE_2D);
		useTexture=YSFALSE;
	}

	if(sideWall[0]==YSTRUE) // Bottom
	{
		YsScenery::GlSetColor(sideWallColor[0]);
		glNormal3d(0.0,0.0,-1.0);

		double x,y0,y1;
		x=0.0;
		baseIdx=0;
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(x     ,0.0,0.0);
					glVertex3d(x+xWid,0.0,0.0);
					glVertex3d(x     ,y0 ,0.0);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(x+xWid,0.0,0.0);
					glVertex3d(x+xWid,y1 ,0.0);
					glVertex3d(x     ,y0 ,0.0);
					glEnd();
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x     ,0.0,0.0);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,0.0);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,0.0);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,0.0);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(x+xWid,y1 ,0.0);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,0.0);
					glEnd();
				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[1]==YSTRUE) // Right
	{
		YsScenery::GlSetColor(sideWallColor[1]);
		glNormal3d(1.0,0.0,0.0);

		double z,y0,y1;
		z=0.0;
		baseIdx=nx;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(double(nx)*xWid,0.0,z);
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					glVertex3d(double(nx)*xWid,y0 ,z);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					glVertex3d(double(nx)*xWid,y1 ,z+zWid);
					glVertex3d(double(nx)*xWid,y0 ,z);
					glEnd();
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(double(nx)*xWid,0.0,z);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(double(nx)*xWid,y0 ,z);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(double(nx)*xWid,y1 ,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(double(nx)*xWid,y0 ,z);
					glEnd();
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}
	if(sideWall[2]==YSTRUE) // Top
	{
		YsScenery::GlSetColor(sideWallColor[2]);
		glNormal3d(0.0,0.0,1.0);

		double x,y0,y1;
		x=0.0;
		baseIdx=nz*(nx+1);
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(x     ,0.0,double(nz)*zWid);
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					glVertex3d(x+xWid,0.0,double(nz)*zWid);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(x+xWid,0.0,double(nz)*zWid);
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					glVertex3d(x+xWid,y1 ,double(nz)*zWid);
					glEnd();
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x     ,0.0,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,double(nz)*zWid);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(x+xWid,y1 ,double(nz)*zWid);
					glEnd();
				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[3]==YSTRUE) // Left
	{
		YsScenery::GlSetColor(sideWallColor[3]);
		glNormal3d(-1.0,0.0,0.0);

		double z,y0,y1;
		z=0.0;
		baseIdx=0;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(0.0,0.0,z);
					glVertex3d(0.0,y0 ,z);
					glVertex3d(0.0,0.0,z+zWid);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					glVertex3d(0.0,0.0,z+zWid);
					glVertex3d(0.0,y0 ,z);
					glVertex3d(0.0,y1 ,z+zWid);
					glEnd();
				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(0.0,0.0,z);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(0.0,y0 ,z);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(0.0,0.0,z+zWid);
					glEnd();
				}
				if(YsZero(y1)!=YSTRUE)
				{
					glBegin(GL_POLYGON);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(0.0,0.0,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(0.0,y0 ,z);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(0.0,y1 ,z+zWid);
					glEnd();
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}

	if(nameElvGridNode==YSTRUE)
	{
		for(j=0; j<nz+1; j++)
		{
			for(i=0; i<nx+1; i++)
			{
				int name;
				double x,z;
				name=j*(nx+1)+i;
				x=double(i)*xWid;
				z=double(j)*zWid;
				glLoadName(name);
				glBegin(GL_POINTS);
				glVertex3d(x,node[name].y,z);
				glEnd();
			}
		}
	}

	if(nameElvGridFace==YSTRUE || nameElvGridNode==YSTRUE)
	{
		glLoadName(0xffffffff);
	}

	if(drawBbx==YSTRUE)
	{
		DrawBoundingBox();
	}
}

void YsElevationGrid::DrawFastFillOnly(const double & /*plgColorScale*/)
{
	int i,j,baseIdx;
	double x,z;
	YsVec3 rc[4],*tri[6];

	glPushAttrib(GL_ENABLE_BIT);
	// glEnable(GL_CULL_FACE);  // Am I drawing in the reverse orientation?
	glEnable(GL_LIGHTING);

	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2,2);


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


	if(YSTRUE!=useOwnTexture &&
	   nullptr!=YsScenery::commonTexManPtr && YSTRUE==YsScenery::commonTexManPtr->IsReady(YsScenery::commonGroundTexHd))  // 2007/02/05
	{
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

		auto unitPtr=YsScenery::commonTexManPtr->GetTexture(YsScenery::commonGroundTexHd);
		unitPtr->Bind();

		// 2016/11/19 Bind() is disabling GL_TEXTURE_GEN.  Did I do it on purpose?  Re-enable here for now.
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_2D);
	}


	glBegin(GL_TRIANGLES);

	for(j=0; j<nz; j++)
	{
		const double t0=(double)j/(double)nz;
		const double t1=(double)(j+1)/(double)nz;

		z=double(j)*zWid;
		x=0.0;
		baseIdx=(nx+1)*j;

		//  1  3
		//
		//  0  2
		rc[2].Set(x,node[baseIdx     ].y,z);
		rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
		for(i=0; i<nx; i++)
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
			double triTexCoord[6][2];

			if(node[baseIdx+i].lup==YSTRUE)
			{
				// (3,1,2),(0,2,1)
				tri[0]=&rc[3];
				tri[1]=&rc[1];
				tri[2]=&rc[2];
				tri[3]=&rc[0];
				tri[4]=&rc[2];
				tri[5]=&rc[1];

				triTexCoord[0][0]=texCoord[3][0];
				triTexCoord[0][1]=texCoord[3][1];
				triTexCoord[1][0]=texCoord[1][0];
				triTexCoord[1][1]=texCoord[1][1];
				triTexCoord[2][0]=texCoord[2][0];
				triTexCoord[2][1]=texCoord[2][1];
				triTexCoord[3][0]=texCoord[0][0];
				triTexCoord[3][1]=texCoord[0][1];
				triTexCoord[4][0]=texCoord[2][0];
				triTexCoord[4][1]=texCoord[2][1];
				triTexCoord[5][0]=texCoord[1][0];
				triTexCoord[5][1]=texCoord[1][1];
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

				triTexCoord[0][0]=texCoord[1][0];
				triTexCoord[0][1]=texCoord[1][1];
				triTexCoord[1][0]=texCoord[0][0];
				triTexCoord[1][1]=texCoord[0][1];
				triTexCoord[2][0]=texCoord[3][0];
				triTexCoord[2][1]=texCoord[3][1];
				triTexCoord[3][0]=texCoord[2][0];
				triTexCoord[3][1]=texCoord[2][1];
				triTexCoord[4][0]=texCoord[3][0];
				triTexCoord[4][1]=texCoord[3][1];
				triTexCoord[5][0]=texCoord[0][0];
				triTexCoord[5][1]=texCoord[0][1];
			}
			int k;
			for(k=0; k<2; k++)
			{
				YsVec3 nom[3];
				YsElvGridFaceId nodId[3];
				GetTriangleNodeId(nodId,i,j,k);
				nom[0]=node[(nx+1)*nodId[0].z+nodId[0].x].nomOfNode;
				nom[1]=node[(nx+1)*nodId[1].z+nodId[1].x].nomOfNode;
				nom[2]=node[(nx+1)*nodId[2].z+nodId[2].x].nomOfNode;

				if(node[baseIdx+i].visible[k]==YSTRUE)
				{
					glNormal3dv(node[baseIdx+i].nom[k]);
					if(colorByElevation!=YSTRUE)
					{
						YsScenery::GlSetColor(node[baseIdx+i].c[k]);

						if(YSTRUE==useOwnTexture)
						{
							glTexCoord2d(triTexCoord[k*3][0],triTexCoord[k*3][1]);
						}
						glNormal3dv(nom[0]);
						glVertex3dv(*tri[k*3]);
						if(YSTRUE==useOwnTexture)
						{
							glTexCoord2d(triTexCoord[k*3+1][0],triTexCoord[k*3+1][1]);
						}
						glNormal3dv(nom[1]);
						glVertex3dv(*tri[k*3+1]);
						if(YSTRUE==useOwnTexture)
						{
							glTexCoord2d(triTexCoord[k*3+2][0],triTexCoord[k*3+2][1]);
						}
						glNormal3dv(nom[2]);
						glVertex3dv(*tri[k*3+2]);
					}
					else
					{
						YsScenery::GlSetColor(ColorByElevation(tri[k*3]->y()));
						if(YSTRUE==useOwnTexture)
						{
							glTexCoord2d(triTexCoord[k*3][0],triTexCoord[k*3][1]);
						}
						glNormal3dv(nom[0]);
						glVertex3dv(*tri[k*3]);
						YsScenery::GlSetColor(ColorByElevation(tri[k*3+1]->y()));
						if(YSTRUE==useOwnTexture)
						{
							glTexCoord2d(triTexCoord[k*3+1][0],triTexCoord[k*3+1][1]);
						}
						glNormal3dv(nom[1]);
						glVertex3dv(*tri[k*3+1]);
						YsScenery::GlSetColor(ColorByElevation(tri[k*3+2]->y()));
						if(YSTRUE==useOwnTexture)
						{
							glTexCoord2d(triTexCoord[k*3+2][0],triTexCoord[k*3+2][1]);
						}
						glNormal3dv(nom[2]);
						glVertex3dv(*tri[k*3+2]);
					}
				}
			}
			x+=xWid;
		}
	}

	if(YSTRUE==useOwnTexture)
	{
		glDisable(GL_TEXTURE_2D);
	}

	if(sideWall[0]==YSTRUE) // Bottom
	{
		YsScenery::GlSetColor(sideWallColor[0]);
		glNormal3d(0.0,0.0,-1.0);

		double x,y0,y1;
		x=0.0;
		baseIdx=0;
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glVertex3d(x     ,0.0,0.0);
					glVertex3d(x+xWid,0.0,0.0);
					glVertex3d(x     ,y0 ,0.0);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					glVertex3d(x+xWid,0.0,0.0);
					glVertex3d(x+xWid,y1 ,0.0);
					glVertex3d(x     ,y0 ,0.0);

				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x     ,0.0,0.0);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,0.0);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,0.0);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,0.0);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(x+xWid,y1 ,0.0);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,0.0);

				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[1]==YSTRUE) // Right
	{
		YsScenery::GlSetColor(sideWallColor[1]);
		glNormal3d(1.0,0.0,0.0);

		double z,y0,y1;
		z=0.0;
		baseIdx=nx;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glVertex3d(double(nx)*xWid,0.0,z);
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					glVertex3d(double(nx)*xWid,y0 ,z);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					glVertex3d(double(nx)*xWid,y1 ,z+zWid);
					glVertex3d(double(nx)*xWid,y0 ,z);

				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(double(nx)*xWid,0.0,z);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(double(nx)*xWid,y0 ,z);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(double(nx)*xWid,0.0,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(double(nx)*xWid,y1 ,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(double(nx)*xWid,y0 ,z);

				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}
	if(sideWall[2]==YSTRUE) // Top
	{
		YsScenery::GlSetColor(sideWallColor[2]);
		glNormal3d(0.0,0.0,1.0);

		double x,y0,y1;
		x=0.0;
		baseIdx=nz*(nx+1);
		for(i=0; i<nx; i++)
		{
			y0=node[baseIdx  ].y;
			y1=node[baseIdx+1].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glVertex3d(x     ,0.0,double(nz)*zWid);
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					glVertex3d(x+xWid,0.0,double(nz)*zWid);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					glVertex3d(x+xWid,0.0,double(nz)*zWid);
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					glVertex3d(x+xWid,y1 ,double(nz)*zWid);

				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x     ,0.0,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,double(nz)*zWid);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(x+xWid,0.0,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(x     ,y0 ,double(nz)*zWid);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(x+xWid,y1 ,double(nz)*zWid);

				}
			}
			x+=xWid;
			baseIdx++;
		}
	}
	if(sideWall[3]==YSTRUE) // Left
	{
		YsScenery::GlSetColor(sideWallColor[3]);
		glNormal3d(-1.0,0.0,0.0);

		double z,y0,y1;
		z=0.0;
		baseIdx=0;
		for(i=0; i<nz; i++)
		{
			y0=node[baseIdx       ].y;
			y1=node[baseIdx+(nx+1)].y;
			if(colorByElevation!=YSTRUE)
			{
				if(YsZero(y0)!=YSTRUE)
				{
					glVertex3d(0.0,0.0,z);
					glVertex3d(0.0,y0 ,z);
					glVertex3d(0.0,0.0,z+zWid);

				}
				if(YsZero(y1)!=YSTRUE)
				{
					glVertex3d(0.0,0.0,z+zWid);
					glVertex3d(0.0,y0 ,z);
					glVertex3d(0.0,y1 ,z+zWid);

				}
			}
			else
			{
				if(YsZero(y0)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(0.0,0.0,z);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(0.0,y0 ,z);
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(0.0,0.0,z+zWid);
				}
				if(YsZero(y1)!=YSTRUE)
				{
					YsScenery::GlSetColor(ColorByElevation(0.0));
					glVertex3d(0.0,0.0,z+zWid);
					YsScenery::GlSetColor(ColorByElevation(y0));
					glVertex3d(0.0,y0 ,z);
					YsScenery::GlSetColor(ColorByElevation(y1));
					glVertex3d(0.0,y1 ,z+zWid);
				}
			}
			z+=zWid;
			baseIdx+=(nx+1);
		}
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glPopAttrib();
}

static void DrawQuadAsTwoTri(const YsVec3 &q0,const YsVec3 &q1,const YsVec3 &q2,const YsVec3 &q3)
{
	glVertex3dv(q0);
	glVertex3dv(q1);
	glVertex3dv(q2);

	glVertex3dv(q2);
	glVertex3dv(q3);
	glVertex3dv(q0);
}

void YsElevationGrid::DrawProtectPolygon(void)
{
	if(hasProtectPolygon!=YSFALSE)  // Can be YSTFUNKNOWN
	{
		int i,j,baseIdx;
		double x,z;
		YsVec3 rc[4],*tri[6];

		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_CULL_FACE);
		glEnable(GL_LIGHTING);

		// Adding polygon offset to terrain was not such a good idea.  It makes artifacts.
		// glEnable(GL_POLYGON_OFFSET_FILL);
		// glPolygonOffset(1,1);


		glBegin(GL_TRIANGLES);

		for(j=0; j<nz; j++)
		{
			z=double(j)*zWid;
			x=0.0;
			baseIdx=(nx+1)*j;

			//  1  3
			//
			//  0  2
			rc[2].Set(x,node[baseIdx     ].y,z);
			rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
			for(i=0; i<nx; i++)
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
				int k;
				for(k=0; k<2; k++)
				{
					YsElvGridFaceId nodId[3];
					GetTriangleNodeId(nodId,i,j,k);
					YsVec3 cen,cenBtm,btm[3];

					if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
					{
						hasProtectPolygon=YSTRUE;

						glVertex3dv(*tri[k*3  ]);
						glVertex3dv(*tri[k*3+1]);
						glVertex3dv(*tri[k*3+2]);

						// Side walls
						btm[0].Set(tri[k*3  ]->x(),0.0,tri[k*3  ]->z());
						btm[1].Set(tri[k*3+1]->x(),0.0,tri[k*3+1]->z());
						btm[2].Set(tri[k*3+2]->x(),0.0,tri[k*3+2]->z());

						cen=(*tri[k*3]+*tri[k*3+1]+*tri[k*3+2])/3.0;
						cenBtm.Set(cen.x(),0.0,cen.z());

						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3  ],btm[0]);
						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+1],btm[1]);
						DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+2],btm[2]);
					}
				}
				x+=xWid;
			}
		}
		glEnd();

		glPopAttrib();

		if(hasProtectPolygon==YSTFUNKNOWN)
		{
			hasProtectPolygon=YSFALSE;
		}
	}
}


// See YSFLIGHT/document/20081116.jnt
void YsElevationGrid::DrawClippedProtectPolygon(
    const YsVec3 &cameraPos,const YsPlane &clipPln,const YsPlane &nearPln,const YsVec3 &t0,const YsVec3 &t1,const YsVec3 &t2)

{
	int nClp1,nClp2;
	YsVec3 clp1[5],clp2[5];
	YsVec3 const *tri[4];
	int i;

	tri[0]=&t0;
	tri[1]=&t1;
	tri[2]=&t2;
	tri[3]=&t0;

	// Clipping the polygon by clipPln (Extract only forward part)
	nClp1=0;
	for(i=0; i<3 && nClp1<4; i++)
	{
		if(clipPln.GetSideOfPlane(*tri[i])>=0)
		{
			clp1[nClp1++]=*tri[i];
			if(clipPln.GetSideOfPlane(*tri[i+1])<0)  // Going out
			{
				if(clipPln.GetIntersection(clp1[nClp1],*tri[i],*tri[i+1]-*tri[i])==YSOK)
				{
					nClp1++;
				}
			}
		}
		else if(clipPln.GetSideOfPlane(*tri[i+1])>=0)  // Coming in
		{
			if(clipPln.GetIntersection(clp1[nClp1],*tri[i],*tri[i+1]-*tri[i])==YSOK)
			{
				nClp1++;
			}
		}
	}
	if(nClp1==0)
	{
		return;
	}
	clp1[nClp1]=clp1[0];

	// Clipping the polygon by nearPln (Extract only rear part)
	nClp2=0;
	for(i=0; i<nClp1 && nClp2<5; i++)
	{
		if(nearPln.GetSideOfPlane(clp1[i])<0)
		{
			clp2[nClp2++]=clp1[i];
			if(nearPln.GetSideOfPlane(clp1[i+1])>=0)  // Going out
			{
				if(nearPln.GetIntersection(clp2[nClp2],clp1[i],clp1[i+1]-clp1[i])==YSOK)
				{
					nClp2++;
				}
			}
		}
		else if(nearPln.GetSideOfPlane(clp1[i+1])<0)  // Coming in
		{
			if(nearPln.GetIntersection(clp2[nClp2],clp1[i],clp1[i+1]-clp1[i])==YSOK)
			{
				nClp2++;
			}
		}
	}


	// Extend the polygon to nearPln
	for(i=0; i<nClp2; i++)
	{
		if(nearPln.GetSideOfPlane(clp2[i])<0)
		{
			nearPln.GetIntersection(clp2[i],cameraPos,clp2[i]-cameraPos);
		}
	}

	// Then draw
	if(nClp2>=3)
	{
		glVertex3dv(clp2[0]);
		glVertex3dv(clp2[1]);
		glVertex3dv(clp2[2]);
		if(nClp2>=4)
		{
			glVertex3dv(clp2[0]);
			glVertex3dv(clp2[2]);
			glVertex3dv(clp2[3]);
		}
		if(nClp2>=5)
		{
			glVertex3dv(clp2[0]);
			glVertex3dv(clp2[3]);
			glVertex3dv(clp2[4]);
		}
	}
}

void YsElevationGrid::DrawProtectPolygonAccurate(const YsMatrix4x4 &viewMdlMat,const double &nearZ)
{
	if(hasProtectPolygon!=YSFALSE)  // Can be YSTFUNKNOWN
	{
		int i,j,baseIdx;
		double x,z;
		YsVec3 rc[4],*tri[6];


		// See YSFLIGHT/document/20081116.jnt for why cameraPos, nearPln, and clipPln are needed.
		// posInCameraCoord=viewMdlMat*posInEvgCoord
		YsMatrix4x4 camToEvg(viewMdlMat);
		YsPlane nearPln,clipPln;
		YsVec3 cameraPos,org,nom;

		camToEvg.Invert();  // Now posInEvgCoord=camToEvg*posInCameraCoord

		camToEvg.Mul(cameraPos,YsOrigin(),1.0);
		org.Set(0.0,0.0,nearZ*1.001);
		nom.Set(0.0,0.0,1.0);
		camToEvg.Mul(org,org,1.0);
		camToEvg.Mul(nom,nom,0.0);
		nearPln.Set(org,nom);  // Side<0 means a point is closer than the near plane.

		org.Set(0.0,0.0,0.5);
		camToEvg.Mul(org,org,1.0);
		clipPln.Set(org,nom);



		glPushAttrib(GL_ENABLE_BIT);
		glDisable(GL_CULL_FACE);
		glDisable(GL_LIGHTING);


		glBegin(GL_TRIANGLES);

		for(j=0; j<nz; j++)
		{
			z=double(j)*zWid;
			x=0.0;
			baseIdx=(nx+1)*j;

			//  1  3
			//
			//  0  2
			rc[2].Set(x,node[baseIdx     ].y,z);
			rc[3].Set(x,node[baseIdx+nx+1].y,z+zWid);
			for(i=0; i<nx; i++)
			{
				rc[0]=rc[2];
				rc[1]=rc[3];
				rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
				rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

				if(node[baseIdx+i].protectPolygon[0]==YSTRUE ||
				   node[baseIdx+i].protectPolygon[1]==YSTRUE)
				{
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
					int k;
					for(k=0; k<2; k++)
					{
						YsElvGridFaceId nodId[3];
						GetTriangleNodeId(nodId,i,j,k);
						YsVec3 cen,cenBtm,btm[3];

						if(node[baseIdx+i].protectPolygon[k]==YSTRUE)
						{
							hasProtectPolygon=YSTRUE;

							glVertex3dv(*tri[k*3  ]);
							glVertex3dv(*tri[k*3+1]);
							glVertex3dv(*tri[k*3+2]);

							if(nearPln.GetSideOfPlane(*tri[k*3  ])<0 ||
							   nearPln.GetSideOfPlane(*tri[k*3+1])<0 ||
							   nearPln.GetSideOfPlane(*tri[k*3+2])<0)
							{
								// See YSFLIGHT/document/20081116.jnt
								DrawClippedProtectPolygon(cameraPos,clipPln,nearPln,*tri[k*3],*tri[k*3+1],*tri[k*3+2]);
							}

							// Side walls
							btm[0].Set(tri[k*3  ]->x(),0.0,tri[k*3  ]->z());
							btm[1].Set(tri[k*3+1]->x(),0.0,tri[k*3+1]->z());
							btm[2].Set(tri[k*3+2]->x(),0.0,tri[k*3+2]->z());

							cen=(*tri[k*3]+*tri[k*3+1]+*tri[k*3+2])/3.0;
							cenBtm.Set(cen.x(),0.0,cen.z());

							DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3  ],btm[0]);
							DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+1],btm[1]);
							DrawQuadAsTwoTri(cenBtm,cen,*tri[k*3+2],btm[2]);
						}
					}
				}
				x+=xWid;
			}
		}
		glEnd();

		glPopAttrib();

		if(hasProtectPolygon==YSTFUNKNOWN)
		{
			hasProtectPolygon=YSFALSE;
		}
	}
}

void YsElevationGrid::DrawBoundingBox(void)
{
	glDisable(GL_LIGHTING);
	YsScenery::GlSetColor(YsCyan());
	glBegin(GL_LINE_LOOP);
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
	glEnd();

	glBegin(GL_LINES);
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
	glEnd();
}

void YsSceneryShell::Draw(YSBOOL /*wire*/,YSBOOL /*fill*/,YSBOOL drawBbx)
{
	if(drawBbx==YSTRUE)
	{
		YsVec3 bbx[2];
		shl.GetBoundingBox(bbx[0],bbx[1]);

		glDisable(GL_LIGHTING);
		YsScenery::GlSetColor(YsGray());
		glBegin(GL_LINE_LOOP);
		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
		glEnd();

		glBegin(GL_LINES);
		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
		glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
		glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
		glEnd();

		glBegin(GL_LINE_LOOP);
		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
		glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
		glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
		glEnd();
	}


	// Modelview matrix already takes left-hand coordinate system into account; it multiplies -1 to z.
	// YsVisualSrf::Draw also multiplies -1 to z, so the multiplications cancel each other.
	double glMat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,glMat);
	YsMatrix4x4 glModelview;
	glModelview.CreateFromOpenGlCompatibleMatrix(glMat);

	YsMatrix4x4 modelview;
	modelview.Scale(1.0,1.0,-1.0);
	modelview*=glModelview;
	shl.Draw(modelview,YsVisualSrf::DRAWOPAQUE);
	shl.Draw(modelview,YsVisualSrf::DRAWTRANSPARENT);

	// For subsequent axis drawing.
	YsScenery::GlSetColor(YsGray());
}

void YsSceneryRectRegion::Draw(void)
{
	glDisable(GL_LIGHTING);

	glBegin(GL_LINE_LOOP);
	glVertex3d(min.x(),0.0,min.y());
	glVertex3d(max.x(),0.0,min.y());
	glVertex3d(max.x(),0.0,max.y());
	glVertex3d(min.x(),0.0,max.y());
	glEnd();

	glRasterPos3d(0.0,20.0,0.0);
	YsString idMsg,tagMsg;

	switch(id)
	{
	case 1:
		idMsg="ID=1(Runway)";
		break;
	case 2:
		idMsg="ID=1(Taxiway)";
		break;
	case 10:
		idMsg="ID=10(View Point)";
		break;
	default:
		idMsg.Printf("ID=%d",id);
		break;
	}

	if(0!=this->tagStr[0])
	{
		tagMsg.Printf(",TAG=%s",tagStr.c_str());
	}

	YsString all;
	all.Set("RGN ");
	all.Append(idMsg);
	all.Append(" ");
	all.Append(tagMsg);
	ysGlPrint(all);
}

void YsSceneryGndObj::Draw(YSBOOL /*wire*/,YSBOOL /*fill*/,YSBOOL drawBbx)
{
	if(gndObjTemplate!=NULL)
	{
		if(drawBbx==YSTRUE)
		{
			glDisable(GL_LIGHTING);

			YsVec3 bbx[2];
			gndObjTemplate->GetDnm().GetBoundingBox(bbx);

			YsScenery::GlSetColor(YsBlue());
			glBegin(GL_LINE_LOOP);
			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
			glEnd();

			glBegin(GL_LINES);			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
			glEnd();

			glBegin(GL_LINE_LOOP);
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
			glEnd();
		}

		// Modelview matrix already takes left-hand coordinate system into account; it multiplies -1 to z.
		// YsVisualSrf::Draw also multiplies -1 to z, so the multiplications cancel each other.
		double glMat[16];
		glGetDoublev(GL_MODELVIEW_MATRIX,glMat);
		YsMatrix4x4 glModelview;
		glModelview.CreateFromOpenGlCompatibleMatrix(glMat);
		YsMatrix4x4 modelview;
		modelview.Scale(1.0,1.0,-1.0);
		modelview*=glModelview;
		gndObjTemplate->GetDnm().Draw(modelview,YsVisualSrf::DRAWOPAQUE);
		gndObjTemplate->GetDnm().Draw(modelview,YsVisualSrf::DRAWTRANSPARENT);
	}

	if(YSTRUE==drawBbx)
	{
		YsVec3 bbx[2],dgn;
		double l;

		GetBoundingBox(bbx);
		dgn=bbx[1]-bbx[0];
		l=dgn.GetLength();

		glDisable(GL_LIGHTING);  // glCallList ignores color unless something is drawn before.
		YsScenery::GlSetColor(YsThreeBitColor(iff+1));
		glBegin(GL_LINES);
		glVertex3dv(YsOrigin());
		glVertex3d(0.0,l,0.0);
		glEnd();

		glRasterPos3d(0.0,l,0.0);
		YsString msg,add;

		msg.Printf("[%s] IFF=%d",objName.data(),iff+1);

		if(gndFlag!=0)
		{
			add.Printf(" F=0x%02x",gndFlag);
			msg+=add;
		}

		if(primaryTarget==YSTRUE)
		{
			msg+=" (P)";
		}

		if(id!=0)
		{
			add.Printf(" ID=%d",id);
			msg+=add;
		}

		if(0!=tagStr[0])
		{
			msg+=" TAG=\"";
			msg+=tagStr;
			msg+="\"";
		}

		ysGlPrint(msg);
	}
}

void YsSceneryAir::Draw(YSBOOL /*wire*/,YSBOOL /*fill*/,YSBOOL drawBbx)
{
	if(airTemplate!=NULL)
	{
		if(drawBbx==YSTRUE)
		{
			glDisable(GL_LIGHTING);

			YsVec3 bbx[2];
			airTemplate->GetDnm().GetBoundingBox(bbx);

			YsScenery::GlSetColor(YsGreen());
			glBegin(GL_LINE_LOOP);
			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
			glEnd();

			glBegin(GL_LINES);			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
			glEnd();

			glBegin(GL_LINE_LOOP);
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
			glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
			glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
			glEnd();
		}

		if(ldg==YSTRUE)
		{
			airTemplate->GetDnm().SetState(YSDNM_CLASSID_LANDINGDEVICE,1);
		}
		else
		{
			airTemplate->GetDnm().SetState(YSDNM_CLASSID_LANDINGDEVICE,0);
		}


		// Modelview matrix already takes left-hand coordinate system into account; it multiplies -1 to z.
		// YsVisualSrf::Draw also multiplies -1 to z, so the multiplications cancel each other.
		double glMat[16];
		glGetDoublev(GL_MODELVIEW_MATRIX,glMat);
		YsMatrix4x4 glModelview;
		glModelview.CreateFromOpenGlCompatibleMatrix(glMat);
		YsMatrix4x4 modelview;
		modelview.Scale(1.0,1.0,-1.0);
		modelview*=glModelview;
		airTemplate->GetDnm().Draw(modelview,YsVisualSrf::DRAWOPAQUE);
		airTemplate->GetDnm().Draw(modelview,YsVisualSrf::DRAWTRANSPARENT);
	}

	if(YSTRUE==drawBbx)
	{
		YsVec3 bbx[2],dgn;
		double l;

		GetBoundingBox(bbx);
		dgn=bbx[1]-bbx[0];
		l=dgn.GetLength();

		glDisable(GL_LIGHTING);  // glCallList ignores color unless something is drawn before.
		YsScenery::GlSetColor(YsThreeBitColor(iff+1));
		glBegin(GL_LINES);
		glVertex3dv(YsOrigin());
		glVertex3d(0.0,l,0.0);
		glEnd();

		glRasterPos3d(0.0,l,0.0);
		YsString msg;

		msg.Printf("[%s] IFF=%d",objName.data(),iff+1);
		if(ldg==YSTRUE)
		{
			msg+=" (LDG)";
		}

		if(id!=0)
		{
			YsString add;
			add.Printf(" ID=%d",id);
			msg+=add;
		}

		if(tagStr[0]!=0)
		{
			msg+=" TAG=\"";
			msg+=tagStr;
			msg+="\"";
		}

		ysGlPrint(msg);
	}
}

void YsSceneryPointSet::Draw(void)
{
	int i;

	glDisable(GL_LIGHTING);

	if(GetAreaType()==YSSCNAREA_NOAREA)
	{
		glLineWidth(2);

		double r,g;
		glBegin(GL_LINE_STRIP);
		if(pnt.GetN()>1)
		{
			for(i=0; i<pnt.GetN(); i++)
			{
				r=(double)i/(double)(pnt.GetN()-1);
				g=(double)(pnt.GetN()-1-i)/(double)(pnt.GetN()-1);
				glColor3d(r,g,0.0);
				glVertex3dv(pnt[i]);
			}
			if(isLoop==YSTRUE)
			{
				glColor3d(1.0,0.0,0.0);
				glVertex3dv(pnt[0]);
			}
		}
		glEnd();
		glLineWidth(1);

		for(int i=0; i<pnt.GetN()-1; ++i)
		{
			YsGlDrawArrowAbsoluteSize(pnt[i],pnt[i+1],6.0);
		}
		if(YSTRUE==isLoop)
		{
			YsGlDrawArrowAbsoluteSize(pnt[pnt.GetN()-1],pnt[0],6.0);
		}
	}
	else
	{
		switch(GetAreaType())
		{
		case YSSCNAREA_LAND:
			glColor3d(1.0,0.0,0.0);
			break;
		default:
		case YSSCNAREA_WATER:
			glColor3d(0.0,1.0,1.0);
			break;
		}

		glLineWidth(1);
		glBegin(GL_LINE_LOOP);
		forYsArray(i,pnt)
		{
			glVertex3dv(pnt[i]);
		}
		glEnd();
	}

	glPointSize(5);
	glBegin(GL_POINTS);
	for(i=0; i<pnt.GetN(); i++)
	{
		glVertex3dv(pnt[i]);
	}
	glEnd();
	glPointSize(1);


	if(pnt.GetN()>0)
	{
		glRasterPos3d(pnt[0].x(),pnt[0].y(),pnt[0].z());
	}
	else
	{
		glRasterPos3d(0.0,0.0,0.0);
	}

	YsString msg;
	if(GetAreaType()==YSSCNAREA_NOAREA)
	{
		msg.Printf("ID=%d TAG=%s",id,tagStr.data());
	}
	else
	{
		msg.Printf("ID=%d TAG=%s AREA=%s",id,tagStr.data(),YsScenery::GetAreaTypeString(GetAreaType()));
	}
	ysGlPrint(msg);


	glEnable(GL_LIGHTING);
}

void YsSceneryPointSet::DrawStar(void)
{
	int i;
	glPointSize(8);
	YsGlSetColor(YsGreen());
	glBegin(GL_POINTS);
	for(i=0; i<pnt.GetN(); i++)
	{
		glVertex3dv(pnt[i]);
	}
	glEnd();
	glPointSize(1);

	glLineWidth(5);
	glBegin(GL_LINE_STRIP);
	if(pnt.GetN()>1)
	{
		for(i=0; i<pnt.GetN(); i++)
		{
			glVertex3dv(pnt[i]);
		}
		if(isLoop==YSTRUE)
		{
			glVertex3dv(pnt[0]);
		}
	}
	glEnd();
	glLineWidth(1);
}

void YsScenery::GlSetColor(const YsColor &col)
{
	glColor3d(col.Rd(),col.Gd(),col.Bd());
}

/* void YsScenery::GlSetColorMaterial(const YsColor &col)
{
	const double alpha=1.0;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	float specular[4],diffuse[4],ambient[4];

	specular[0]=0.0F;
	specular[1]=0.0F;
	specular[2]=0.0F;
	specular[3]=float(alpha);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,specular);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,30.0);

	diffuse[0]=float(col.Rd());
	diffuse[1]=float(col.Gd());
	diffuse[2]=float(col.Bd());
	diffuse[3]=float(alpha);
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,diffuse);

	ambient[0]=0.04F;
	ambient[1]=0.04F;
	ambient[2]=0.04F;
	ambient[3]=float(alpha);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,ambient);
} */

void YsScenery::DrawBoundingBox(void)
{
	YsVec3 bbx[2],dgn,XZ;
	double l;
	GetBoundingBox(bbx);
	dgn=bbx[1]-bbx[0];
	l=YsGreater(YsGreater(dgn.x(),dgn.y()),dgn.z());
	XZ.Set(1.0,0.0,1.0);
	bbx[0]-=l*0.05*XZ;
	bbx[1]+=l*0.05*XZ;

	glDisable(GL_LIGHTING);
	YsScenery::GlSetColor(YsDarkYellow());
	glBegin(GL_LINE_LOOP);
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
	glEnd();

	glBegin(GL_LINES);
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[0].y(),bbx[0].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
	glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[0].z());
	glVertex3d(bbx[0].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[1].z());
	glVertex3d(bbx[1].x(),bbx[1].y(),bbx[0].z());
	glEnd();
}

void YsScenery::Draw
	   (YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	glPushMatrix();
	YsGlMulMatrix(pos,att);

	DrawSingle(wire,fill,drawBbx,drawShrink,drawShl,drawEvg,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);

	YsListItem <YsScenery> *scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		glLoadName(scn->dat.searchKey);
		scn->dat.Draw(wire,fill,drawBbx,drawShrink,drawShl,drawEvg,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);
	}
	glPopMatrix();
}

void YsScenery::DrawThisSceneryOnly(
	    YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,
	    YSBOOL drawGndObj,YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsMatrix4x4 tfm(YSFALSE);
	GetTransformation(tfm,this);

	double mat[16];
	tfm.GetOpenGlCompatibleMatrix(mat);

	glPushMatrix();
	glMultMatrixd(mat);

	DrawSingle(wire,fill,drawBbx,drawShrink,drawShl,drawEvg,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);

	glPopMatrix();
}

void YsScenery::DrawSingle(
    YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,
    YSBOOL drawGndObj,YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryAir> *air;
	YsListItem <YsSceneryPointSet> *pst;

	if(drawScn==YSTRUE)
	{
		if(drawBbx==YSTRUE)
		{
			DrawBoundingBox();
		}
	}


	if(drawShl==YSTRUE)
	{
		shl=NULL;
		while((shl=shlList.FindNext(shl))!=NULL)
		{
			glPushMatrix();
			glLoadName(shl->dat.searchKey);
			YsGlMulMatrix(shl->dat.pos,shl->dat.att);
			shl->dat.Draw(wire,fill,drawBbx);
			glPopMatrix();
		}
	}

	if(drawEvg==YSTRUE)
	{
		evg=NULL;
		while((evg=evgList.FindNext(evg))!=NULL)
		{
			glPushMatrix();
			glLoadName(evg->dat.searchKey);
			YsGlMulMatrix(evg->dat.pos,evg->dat.att);
			evg->dat.evg.Draw(plgColorScale,YSFALSE,wire,fill,drawBbx,drawShrink,YSFALSE,YSFALSE);
			glPopMatrix();
		}
	}

	if(drawSbd==YSTRUE)
	{
		drw=NULL;
		while((drw=sbdList.FindNext(drw))!=NULL)
		{
			glPushMatrix();
			glLoadName(drw->dat.searchKey);
			YsGlMulMatrix(drw->dat.pos,drw->dat.att);
			drw->dat.drw.Draw(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,YSFALSE,-1.0);
			glPopMatrix();
		}
	}

	if(drawRgn==YSTRUE)
	{
		rgn=NULL;
		while((rgn=rgnList.FindNext(rgn))!=NULL)
		{
			glPushMatrix();
			glLoadName(rgn->dat.searchKey);
			YsGlMulMatrix(rgn->dat.pos,rgn->dat.att);
			YsScenery::GlSetColor(YsThreeBitColor(rgn->dat.id%8));
			rgn->dat.Draw();
			glPopMatrix();
		}
	}

	if(drawGndObj==YSTRUE)
	{
		gnd=NULL;
		while((gnd=gndList.FindNext(gnd))!=NULL)
		{
			glPushMatrix();
			glLoadName(gnd->dat.searchKey);
			YsGlMulMatrix(gnd->dat.pos,gnd->dat.att);
			gnd->dat.Draw(wire,fill,drawBbx);
			glPopMatrix();
		}
	}

	if(drawAir==YSTRUE)
	{
		air=NULL;
		while(NULL!=(air=airList.FindNext(air)))
		{
			glPushMatrix();
			glLoadName(air->dat.searchKey);
			YsGlMulMatrix(air->dat.pos,air->dat.att);
			air->dat.Draw(wire,fill,drawBbx);
			glPopMatrix();
		}
	}

	if(drawPst==YSTRUE)
	{
		pst=NULL;
		while((pst=pstList.FindNext(pst))!=NULL)
		{
			glPushMatrix();
			glLoadName(pst->dat.searchKey);
			YsGlMulMatrix(pst->dat.pos,pst->dat.att);
			pst->dat.Draw();
			glPopMatrix();
		}
	}
}

void YsScenery::DrawMap(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
	glPushMatrix();
	YsGlMulMatrix(pos,att);

	DrawMapSingle(wire,fill,drawBbx);

	YsListItem <YsScenery> *scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.DrawMap(wire,fill,drawBbx);
	}
	glPopMatrix();
}

void YsScenery::DrawMapThisSceneryOnly(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
	YsMatrix4x4 tfm(YSFALSE);
	GetTransformation(tfm,this);

	double mat[16];
	tfm.GetOpenGlCompatibleMatrix(mat);

	glPushMatrix();
	glMultMatrixd(mat);

	DrawMapSingle(wire,fill,drawBbx);

	glPopMatrix();
}

void YsScenery::DrawMapSingle(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
	YsListItem <YsScenery2DDrawing> *drw=NULL;
	while((drw=mapList.FindNext(drw))!=NULL)
	{
		glPushMatrix();
		glLoadName(drw->dat.searchKey);
		YsGlMulMatrix(drw->dat.pos,drw->dat.att);
		drw->dat.drw.Draw(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSTRUE,wire,fill,drawBbx,YSFALSE,-1.0);
		glPopMatrix();
	}
}

void YsScenery::DrawProtectPolygon(const YsMatrix4x4 &modelTfm) // OpenGL Only for SceneryEdit
{
	glDepthMask(1);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery> *scn;
	YsMatrix4x4 newModelTfm(YSFALSE),shlTfm(YSFALSE);

	glPushMatrix();
	YsGlMulMatrix(pos,att);

	newModelTfm=modelTfm;
	newModelTfm.Translate(pos);
	newModelTfm.RotateXZ(att.h());
	newModelTfm.RotateZY(att.p());
	newModelTfm.RotateXY(att.b());

	evg=NULL;
	while((evg=evgList.FindNext(evg))!=NULL)
	{
		glPushMatrix();
		YsGlMulMatrix(evg->dat.pos,evg->dat.att);
		evg->dat.evg.DrawProtectPolygon();
		glPopMatrix();
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.DrawProtectPolygon(newModelTfm);
	}

	glPopMatrix();

	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
}

void YsScenery::DrawProtectPolygon(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,const double nearZ,
    const double &currentTime)
{
	glDepthMask(1);
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);

	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery> *scn;
	YsMatrix4x4 viewModelTfm(YSFALSE),newModelTfm(YSFALSE),evgTfm(YSFALSE);

	glPushMatrix();
	YsGlMulMatrix(pos,att);

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

			glPushMatrix();
			YsGlMulMatrix(evg->dat.pos,evg->dat.att);
			evg->dat.evg.DrawProtectPolygonAccurate(evgTfm,nearZ);
			glPopMatrix();
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

	glPopMatrix();

	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
}


int YsScenery::numSceneryDrawn;

void YsScenery::DrawVisual(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,const double &currentTime,YSBOOL forShadowMap)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsScenery> *scn;
	const YSBOOL wire=YSTRUE,fill=YSTRUE,drawBbx=YSFALSE/*,drawShrink=YSFALSE*/;
	YsMatrix4x4 viewModelTfm(YSFALSE),newModelTfm(YSFALSE),shlTfm(YSFALSE);

	FsResetMaterial();

	glPushMatrix();
	YsGlMulMatrix(pos,att);

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

	evg=NULL;
	while((evg=evgList.FindNext(evg))!=NULL)
	{
		if(IsItemVisible(viewModelTfm,projTfm,&evg->dat)==YSTRUE)
		{
			glPushMatrix();
			YsGlMulMatrix(evg->dat.pos,evg->dat.att);
			evg->dat.evg.DrawFastFillOnly(plgColorScale);
			glPopMatrix();
		}
	}

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

			glPushMatrix();
			YsGlMulMatrix(drw->dat.pos,drw->dat.att);
			drw->dat.drw.Draw
			(plgColorScale,linColorScale,pntColorScale,YSTRUE,YSFALSE,wire,fill,drawBbx,YSFALSE,currentTime,&shlTfm);
			glPopMatrix();
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

	glPopMatrix();
}

void YsScenery::DrawMapVisual
    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &,const YsMatrix4x4 &projTfm,
     const double &,const double &,YSBOOL drawPset,const double &currentTime)
{
	YsMatrix4x4 shlTfm(YSFALSE);

	FsResetMaterial();

	numSceneryDrawn++;

	glDisable(GL_CULL_FACE);

	glPushMatrix();

	const YSBOOL wire=YSFALSE,fill=YSTRUE,drawBbx=YSFALSE;

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

					glLoadName(mapDrawingInfo.mapPtr->searchKey);

					GLfloat glMat[16];
					shlTfm.GetOpenGlCompatibleMatrix(glMat);
					glLoadIdentity();
					glScaled(1.0,1.0,-1.0);
					glMultMatrixf(glMat);

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

	glPopMatrix();
}


void YsScenery::DrawAxis(
    const double &axsSize,
    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	DrawAxisSingle(axsSize,drawShl,drawEvg,drawMap,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);

	YsListItem <YsScenery> *scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		YsScenery::GlSetColor(YsDarkYellow());
		glPushMatrix();
		YsGlMulMatrix(scn->dat.pos,scn->dat.att);
		glLoadName(scn->dat.searchKey);
		if(drawScn==YSTRUE)
		{
			YsGlDrawAxis(axsSize);
		}
		scn->dat.DrawAxis(axsSize,drawShl,drawEvg,drawMap,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);
		glPopMatrix();
	}
}

void YsScenery::DrawAxisThisSceneryOnly(
	    const double &axsSize,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsMatrix4x4 tfm(YSFALSE);
	GetTransformation(tfm,this);

	double mat[16];
	tfm.GetOpenGlCompatibleMatrix(mat);

	glPushMatrix();
	glMultMatrixd(mat);

	DrawAxisSingle(axsSize,drawShl,drawEvg,drawMap,drawSbd,drawRgn,drawGndObj,drawAir,drawPst,drawScn);

	glPopMatrix();
}

void YsScenery::DrawAxisSingle(
	    const double &axsSize,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryAir> *air;
	YsListItem <YsSceneryPointSet> *pst;


	if(drawMap==YSTRUE)
	{
		YsScenery::GlSetColor(YsMagenta());
		drw=NULL;
		while((drw=mapList.FindNext(drw))!=NULL)
		{
			glPushMatrix();
			glLoadName(drw->dat.searchKey);
			YsGlMulMatrix(drw->dat.pos,drw->dat.att);
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}



	if(drawShl==YSTRUE)
	{
		YsScenery::GlSetColor(YsGray());
		shl=NULL;
		while((shl=shlList.FindNext(shl))!=NULL)
		{
			glPushMatrix();
			glLoadName(shl->dat.searchKey);
			YsGlMulMatrix(shl->dat.pos,shl->dat.att);
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}

	if(drawEvg==YSTRUE)
	{
		YsScenery::GlSetColor(YsCyan());
		evg=NULL;
		while((evg=evgList.FindNext(evg))!=NULL)
		{
			glPushMatrix();
			glLoadName(evg->dat.searchKey);
			YsGlMulMatrix(evg->dat.pos,evg->dat.att);
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}

	if(drawSbd==YSTRUE)
	{
		YsScenery::GlSetColor(YsMagenta());
		drw=NULL;
		while((drw=sbdList.FindNext(drw))!=NULL)
		{
			glPushMatrix();
			glLoadName(drw->dat.searchKey);
			YsGlMulMatrix(drw->dat.pos,drw->dat.att);
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}

	if(drawRgn==YSTRUE)
	{
		YsScenery::GlSetColor(YsGreen());
		rgn=NULL;
		while((rgn=rgnList.FindNext(rgn))!=NULL)
		{
			glPushMatrix();
			glLoadName(rgn->dat.searchKey);
			YsGlMulMatrix(rgn->dat.pos,rgn->dat.att);
			YsScenery::GlSetColor(YsThreeBitColor(rgn->dat.id%8));
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}

	if(drawGndObj==YSTRUE)
	{
		gnd=NULL;
		while((gnd=gndList.FindNext(gnd))!=NULL)
		{
			glPushMatrix();
			glLoadName(gnd->dat.searchKey);
			YsGlMulMatrix(gnd->dat.pos,gnd->dat.att);
			YsScenery::GlSetColor(YsThreeBitColor(gnd->dat.iff+1));
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}

	if(drawAir==YSTRUE)
	{
		air=NULL;
		while((air=airList.FindNext(air))!=NULL)
		{
			glPushMatrix();
			glLoadName(air->dat.searchKey);
			YsGlMulMatrix(air->dat.pos,air->dat.att);
			YsScenery::GlSetColor(YsThreeBitColor(air->dat.iff+1));
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}

	if(drawPst==YSTRUE)
	{
		pst=NULL;
		while((pst=pstList.FindNext(pst))!=NULL)
		{
			glPushMatrix();
			glLoadName(pst->dat.searchKey);
			YsGlMulMatrix(pst->dat.pos,pst->dat.att);
			YsScenery::GlSetColor(YsGreen());
			YsGlDrawAxis(axsSize);
			glPopMatrix();
		}
	}
}

void YsScenery::DrawItemAxis(const YsSceneryItem *itm,const double &axsSize)
{
	YsScenery *scn;
	YsArray <YsScenery *,16> parentLink;

	glPushMatrix();
	glDisable(GL_LIGHTING);

	scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}
	for(YSSIZE_T i=parentLink.GetN()-1; i>=0; i--)
	{
		YsGlMulMatrix(parentLink[i]->pos,parentLink[i]->att);
	}
	YsGlMulMatrix(itm->pos,itm->att);

	YsGlDrawAxis(axsSize);

	glPopMatrix();
}

void YsScenery::DrawItem
   (const YsSceneryItem *itm,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
    YSBOOL name2DElem,YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
	YsArray <YsScenery *,16> parentLink;

	glPushMatrix();

	YsScenery *scn;
	scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}
	for(YSSIZE_T i=parentLink.GetN()-1; i>=0; i--)
	{
		YsGlMulMatrix(parentLink[i]->pos,parentLink[i]->att);
	}
	YsGlMulMatrix(itm->pos,itm->att);

	switch(itm->objType)
	{
	default:
		break;
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

	glPopMatrix();
}

void YsScenery::DrawItemStar(const YsSceneryItem *itm)
{
	YsArray <YsScenery *,16> parentLink;

	glPushMatrix();

	YsScenery *scn;
	scn=itm->owner;
	while(scn!=NULL && scn->owner!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}
	for(YSSIZE_T i=parentLink.GetN()-1; i>=0; i--)
	{
		YsGlMulMatrix(parentLink[i]->pos,parentLink[i]->att);
	}
	YsGlMulMatrix(itm->pos,itm->att);

	if(itm->objType==YsSceneryItem::POINTSET)
	{
		((YsSceneryPointSet *)itm)->DrawStar();
	}

	glPopMatrix();
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
		double matArray[16];
		mat.GetOpenGlCompatibleMatrix(matArray);
		glPushMatrix();
		glMultMatrixd(matArray);

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
			switch(itm->t)
			{
			case Ys2DDrawingElement::POINTS:
				glBegin(GL_POINTS);
				break;
			case Ys2DDrawingElement::LINESEGMENTS:
			case Ys2DDrawingElement::GRADATIONQUADSTRIP:
			case Ys2DDrawingElement::QUADSTRIP:
			case Ys2DDrawingElement::QUADS:
			case Ys2DDrawingElement::TRIANGLES:
			case Ys2DDrawingElement::APPROACHLIGHT:
				glBegin(GL_LINE_STRIP);
				break;
			case Ys2DDrawingElement::LINES:
				glBegin(GL_LINES);
				break;
			default:
			case Ys2DDrawingElement::POLYGON:
				glBegin(GL_LINE_LOOP);
				break;
			}

			for(i=0; i<itm->pnt.GetN(); i++)
			{
				if(mapMode==YSTRUE)
				{
					glVertex3d(itm->pnt[i].x(),0.0,itm->pnt[i].y());
				}
				else
				{
					glVertex2dv(itm->pnt[i]);
				}
			}
			glEnd();
		}

		if(points==YSTRUE)
		{
			glBegin(GL_POINTS);
			for(i=0; i<itm->pnt.GetN(); i++)
			{
				if(mapMode==YSTRUE)
				{
					glVertex3d(itm->pnt[i].x(),0.0,itm->pnt[i].y());
				}
				else
				{
					glVertex2dv(itm->pnt[i]);
				}
			}
			glEnd();
		}

		if(nameVtId==YSTRUE)
		{
			for(i=0; i<itm->pnt.GetN(); i++)
			{
				glLoadName(i);
				glBegin(GL_POINTS);
				if(mapMode==YSTRUE)
				{
					glVertex3d(itm->pnt[i].x(),0.0,itm->pnt[i].y());
				}
				else
				{
					glVertex2dv(itm->pnt[i]);
				}
				glEnd();
			}
		}

		glPopMatrix();
	}
}

void YsScenery::DrawILSGuideline(void)
{
	YsListItem <YsSceneryGndObj> *gnd;

	YsGlSetColor(YsGreen());

	glPushMatrix();

	gnd=NULL;
	while((gnd=gndList.FindNext(gnd))!=NULL)
	{
		if(gnd->dat.gndObjTemplate!=NULL && gnd->dat.gndObjTemplate->ilsRange>YsTolerance)
		{
			glPushMatrix();
			YsGlMulMatrix(gnd->dat.pos,gnd->dat.att);
			YsGlMulMatrix(gnd->dat.gndObjTemplate->ilsPos,gnd->dat.gndObjTemplate->ilsAtt);

			YsVec3 p1,p2;
			p1.Set(0.0,0.0,1000.0);
			p2.Set(0.0,0.0,500.0);
			YsGlDrawArrow(p1,p2,0.05);

			p1.Set(0.0,0.0,500.0);
			p2.Set(0.0,0.0,0.0);
			YsGlDrawArrow(p1,p2,0.05);

			p1.Set(0.0,0.0,500.0);
			p2.Set(0.0,0.0,0.0);
			YsGlDrawArrow(p1,p2,0.05);

			glEnable(GL_LINE_STIPPLE);
			glLineStipple(1,0xf0f0);

			p1.Set(0.0,0.0,0.0);
			p2.Set(0.0,0.0,-500.0);
			YsGlDrawArrow(p1,p2,0.05);

			p1.Set(0.0,0.0,-500.0);
			p2.Set(0.0,0.0,-1000.0);
			YsGlDrawArrow(p1,p2,0.05);

			p1.Set(0.0,0.0,-1000.0);
			p2.Set(0.0,0.0,-1500.0);
			YsGlDrawArrow(p1,p2,0.05);

			p1.Set(0.0,0.0,-1500.0);
			p2.Set(0.0,0.0,-2000.0);
			YsGlDrawArrow(p1,p2,0.05);

			p1.Set(0.0,0.0,-2000.0);
			p2.Set(0.0,0.0,-2500.0);
			YsGlDrawArrow(p1,p2,0.05);

			p1.Set(0.0,0.0,-2500.0);
			p2.Set(0.0,0.0,-3000.0);
			YsGlDrawArrow(p1,p2,0.05);

			glDisable(GL_LINE_STIPPLE);

			glPopMatrix();
		}
	}

	YsListItem <YsScenery> *scn;
	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		glPushMatrix();
		YsGlMulMatrix(scn->dat.pos,scn->dat.att);
		scn->dat.DrawILSGuideline();
		glPopMatrix();
	}

	glPopMatrix();
}

void YsScenery::DrawItemILSGuideline(YsSceneryGndObj *gnd)
{
	if(gnd->gndObjTemplate!=NULL && gnd->gndObjTemplate->ilsRange>YsTolerance)
	{
		YsArray <YsScenery *,16> parentLink;

		glPushMatrix();

		YsScenery *scn;
		scn=gnd->owner;
		while(scn!=NULL && scn->owner!=NULL)
		{
			parentLink.Append(scn);
			scn=scn->owner;
		}
		for(YSSIZE_T i=parentLink.GetN()-1; i>=0; i--)
		{
			YsGlMulMatrix(parentLink[i]->pos,parentLink[i]->att);
		}

		YsGlMulMatrix(gnd->pos,gnd->att);
		YsGlMulMatrix(gnd->gndObjTemplate->ilsPos,gnd->gndObjTemplate->ilsAtt);

		YsVec3 p1,p2;
		p1.Set(0.0,0.0,1000.0);
		p2.Set(0.0,0.0,500.0);
		YsGlDrawArrow(p1,p2,0.05);

		p1.Set(0.0,0.0,500.0);
		p2.Set(0.0,0.0,0.0);
		YsGlDrawArrow(p1,p2,0.05);

		p1.Set(0.0,0.0,500.0);
		p2.Set(0.0,0.0,0.0);
		YsGlDrawArrow(p1,p2,0.05);

		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0xf0f0);

		p1.Set(0.0,0.0,0.0);
		p2.Set(0.0,0.0,-500.0);
		YsGlDrawArrow(p1,p2,0.05);

		p1.Set(0.0,0.0,-500.0);
		p2.Set(0.0,0.0,-1000.0);
		YsGlDrawArrow(p1,p2,0.05);

		p1.Set(0.0,0.0,-1000.0);
		p2.Set(0.0,0.0,-1500.0);
		YsGlDrawArrow(p1,p2,0.05);

		p1.Set(0.0,0.0,-1500.0);
		p2.Set(0.0,0.0,-2000.0);
		YsGlDrawArrow(p1,p2,0.05);

		p1.Set(0.0,0.0,-2000.0);
		p2.Set(0.0,0.0,-2500.0);
		YsGlDrawArrow(p1,p2,0.05);

		p1.Set(0.0,0.0,-2500.0);
		p2.Set(0.0,0.0,-3000.0);
		YsGlDrawArrow(p1,p2,0.05);

		glDisable(GL_LINE_STIPPLE);

		glPopMatrix();
	}
}
