#include <ysclass.h>
#include <ysbase64.h>
#include <yseditarray.h>
#include <ysunitconv.h>
#include "ysscenery.h"

#include <ysshellextio.h>


#include <errno.h>


const unsigned char YsScenery::groundTileTexture[16*16]=
{
	247,235,252,241,238,246,232,247,244,240,245,238,236,230,239,248,
	249,255,237,241,239,239,248,248,237,245,237,239,236,248,237,238,
	233,237,254,236,237,239,245,238,248,236,243,248,251,233,241,251,
	238,240,240,235,250,234,253,238,236,246,251,234,236,238,233,240,
	231,244,240,240,244,253,237,244,234,244,247,244,255,246,242,254,
	237,252,230,235,230,233,242,240,232,242,246,255,232,241,240,251,
	245,254,237,233,251,231,248,246,239,245,232,246,238,254,243,243,
	246,249,247,242,246,252,245,250,254,246,252,242,238,243,236,231,
	255,238,236,244,240,249,232,241,245,253,252,246,249,249,249,241,
	249,237,241,245,231,244,230,249,255,242,239,230,252,255,234,232,
	241,255,253,247,240,248,242,246,241,231,243,253,251,232,250,238,
	250,232,231,231,235,247,238,230,237,233,250,241,255,233,244,240,
	255,246,248,242,232,247,247,237,241,233,254,237,241,241,238,244,
	237,232,232,233,237,239,234,244,251,239,253,255,252,238,232,244,
	231,242,241,246,244,250,253,231,236,254,236,233,251,247,250,240,
	242,252,244,243,234,240,232,244,255,243,234,252,251,245,237,249
};

////////////////////////////////////////////////////////////

static unsigned YsScenerySearchKeySeed=0x80000;
int YsScenery::lightPointSizePix=3;
float YsScenery::lightPointSize3d=0.4f;
unsigned YsScenery::globalControl_transparency=255;  // 0:No transparency  255:Use transparency
YSBOOL YsScenery::globalControl_antiAliasing=YSFALSE;

YsTextureManager *YsScenery::commonTexManPtr=nullptr;
YsTextureManager::TexHandle YsScenery::commonGroundTexHd=nullptr;
YsTextureManager::TexHandle YsScenery::commonRunwayLightTexHd=nullptr;

////////////////////////////////////////////////////////////

YsTextFile::YsTextFile() : text(textAllocator)
{
}

YsTextFile::~YsTextFile()
{
	text.CleanUp();
	textAllocator.CollectGarbage();
}

const YsListContainer <YsString> &YsTextFile::GetText(void) const
{
	return text;
}

YsListItem <YsString> *YsTextFile::CreateNewLine(void)
{
	YsListItem <YsString> *newLine;
	newLine=text.Create();
	newLine->dat="";
	return newLine;
}

void YsTextFile::Initialize(void)
{
	text.CleanUp();
}

////////////////////////////////////////////////////////////

YSRESULT YsSceneryTexturable::TryCacheTexture(class YsScenery *scn) const
{
	auto texHd=scn->textureManager.FindTextureFromLabel(texLabel);
	if(NULL!=texHd)
	{
		texManCache=&scn->textureManager;
		texHdCache=texHd;
		return YSOK;
	}
	else
	{
		texLabelNotFound=YSTRUE;
		texManCache=NULL;
		texHdCache=NULL;
		return YSERR;
	}
}

////////////////////////////////////////////////////////////

Ys2DDrawingElement::OBJTYPE Ys2DDrawingElement::GetElemType(void) const
{
	return t;
}

void Ys2DDrawingElement::SetElemType(Ys2DDrawingElement::OBJTYPE t)
{
	this->t=t;
}

const YsColor &Ys2DDrawingElement::GetColor(void) const
{
	return c;
}

const YsColor &Ys2DDrawingElement::GetSecondColor(void) const
{
	return c2;
}

const double &Ys2DDrawingElement::VisibleDist(void) const
{
	return visibleDist;
}

const YsArray <YsVec2> &Ys2DDrawingElement::GetPointList(void) const
{
	return pnt;
}

const YsArray <YsVec2> &Ys2DDrawingElement::GetTexCoordArray(void) const
{
	return texCoord;
}

const YsTextureManager::Unit *Ys2DDrawingElement::GetTextureUnit(void) const
{
	if(0==texLabel.Strlen())
	{
		return NULL;
	}

	if(YSTRUE==texLabelNotFound)
	{
		return NULL;
	}

	if(NULL!=texManCache)
	{
		return texManCache->GetTexture(texHdCache);
	}

	auto drw=owner;
	auto scnItm=drw->owner;
	if(NULL!=scnItm)
	{
		auto scn=scnItm->GetOwner();
		if(NULL!=scn)
		{
			texManCache=&scn->textureManager;
			texHdCache=texManCache->FindTextureFromLabel(texLabel);
			if(NULL!=texHdCache)
			{
				return texManCache->GetTexture(texHdCache);
			}
			else
			{
				texLabelNotFound=YSTRUE;
			}
		}
	}
	return NULL;
}

void Ys2DDrawingElement::SetVisibleDist(const double dist)
{
	visibleDist=dist;
}

void Ys2DDrawingElement::SetSpecular(YSBOOL s)
{
	specular=s;
}

YSBOOL Ys2DDrawingElement::GetSpecular(void) const
{
	return specular;
}

void Ys2DDrawingElement::Initialize(void)
{
	visibleDist=0.0;
	c=YsBlack();
	cvx=YSTRUE;
	pnt.CleanUp();
	texCoord.CleanUp();

	texLabel.Set("");
	texLabelNotFound=YSFALSE;
	texManCache=NULL;
	texHdCache=NULL;

	specular=YSFALSE;
}

const YSBOOL Ys2DDrawingElement::IsConvex(void) const
{
	return cvx;
}

void Ys2DDrawingElement::SetPointList(const YsArray <YsVec2> &newPntArray)
{
	pnt=newPntArray;
}

void Ys2DDrawingElement::SetTexCoordArray(const YsArray <YsVec2> &newTexCoordArray)
{
	texCoord=newTexCoordArray;
}

YsListAllocator <Ys2DDrawingElement> Ys2DDrawing::elemAllocator(16);

Ys2DDrawing::Ys2DDrawing() : elemList(elemAllocator),frozenList(elemAllocator),graphicCache(NULL)
{
	owner=NULL;
	Initialize();
}

Ys2DDrawing::~Ys2DDrawing()
{
	Initialize();
}

void Ys2DDrawing::Initialize(void)
{
	elemList.CleanUp();
	DeleteCache();
}

void Ys2DDrawing::RecomputeBoundingBox(void)
{
	YsListItem <Ys2DDrawingElement> *ptr;
	YsBoundingBoxMaker2 makeBbx;
	makeBbx.Begin(YsVec2(0.0,0.0));
	ptr=NULL;
	while((ptr=elemList.FindNext(ptr))!=NULL)
	{
		int i;
		YsBoundingBoxMaker2 makeLocalBbx;
		makeLocalBbx.Begin();
		for(i=0; i<ptr->dat.pnt.GetN(); i++)
		{
			makeBbx.Add(ptr->dat.pnt[i]);
			makeLocalBbx.Add(ptr->dat.pnt[i]);
		}
		makeLocalBbx.Get(ptr->dat.bbx[0],ptr->dat.bbx[1]);
	}
	makeBbx.Get(bbx[0],bbx[1]);
}

YsListItem <Ys2DDrawingElement> *Ys2DDrawing::CreateElement(Ys2DDrawingElement::OBJTYPE t)
{
	YsListItem <Ys2DDrawingElement> *newElem;
	newElem=elemList.Create();
	newElem->dat.Initialize();
	newElem->dat.t=t;
	newElem->dat.searchKey=YsScenerySearchKeySeed++;
	newElem->dat.owner=this;
	newElem->dat.thisInTheList=newElem;

	DeleteCache();

	return newElem;
}

YSRESULT Ys2DDrawing::DeleteElement(YsListItem <Ys2DDrawingElement> *toDel)
{
	if(toDel->GetContainer()==&elemList)
	{
		DeleteCache();
		return elemList.Delete(toDel);
	}
	else if(toDel->GetContainer()==&frozenList)
	{
		DeleteCache();
		return frozenList.Delete(toDel);
	}
	else
	{
		return YSERR;
	}
}

YSRESULT Ys2DDrawing::FreezeElement(YsListItem <Ys2DDrawingElement> *toFreeze)
{
	if(toFreeze->GetContainer()==&elemList)
	{
		DeleteCache();
		return elemList.Transfer(toFreeze,frozenList);
	}
	else
	{
		return YSERR;
	}
}

YSRESULT Ys2DDrawing::MeltElement(YsListItem <Ys2DDrawingElement> *toMelt)
{
	if(toMelt->GetContainer()==&frozenList)
	{
		DeleteCache();
		return frozenList.Transfer(toMelt,elemList);
	}
	else
	{
		return YSERR;
	}
}

YSRESULT Ys2DDrawing::DeleteFrozenElement(void)
{
	return frozenList.CleanUp();
}

YSBOOL Ys2DDrawing::IsFrozen(YsListItem <Ys2DDrawingElement> *elem)
{
	if(elem->GetContainer()==&frozenList)
	{
		return YSTRUE;
	}
	else if(elem->GetContainer()==&elemList)
	{
		return YSFALSE;
	}
	else
	{
		YsPrintf("Ys2DDrawing::IsFrozen()\n");
		YsPrintf("  Attempted to access an item that does not belong to this drawing.\n");
		return YSFALSE;
	}
}

YSRESULT Ys2DDrawing::TriangulateAll(void)
{
	YsShell2dTessellator tess;
	YsListItem <Ys2DDrawingElement> *elm;
	elm=NULL;
	while((elm=FindNextElem(elm))!=NULL)
	{
		Triangulate(tess,&elm->dat);
	}
	DeleteCache();
	return YSOK;
}

YSRESULT Ys2DDrawing::Triangulate(YsShell2dTessellator &tess,Ys2DDrawingElement *elm)
{
	// if(elm->IsConvex()!=YSTRUE && elm->GetElemType()==Ys2DDrawingElement::POLYGON)
	if(elm->GetPointList().GetN()>3 && elm->GetElemType()==Ys2DDrawingElement::POLYGON)
	{
		YsArray <YsVec2> pntList=elm->GetPointList();;
		YsArray <YsShell2dVertexHandle> v2HdArray;

		tess.SetDomain(v2HdArray,pntList.GetN(),pntList);
		YsHashTable <YSSIZE_T> v2KeyToPntIdx;
		for(auto idx : v2HdArray.AllIndex())
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

		YsArray <YsVec2> texCoordArray=elm->GetTexCoordArray();

		YsArray <YsVec2> newPosArray,newTexCoordArray;

		YSBOOL first;
		YsListItem <YsShell2dTessTriangle> *ptr;
		tess.triList.RewindPointer();
		first=YSTRUE;
		while(NULL!=(ptr=tess.triList.StepPointer()))
		{
			YSSIZE_T triIdx[3];
			v2KeyToPntIdx.FindElement(triIdx[0],tess.GetSearchKey(ptr->dat.trVtHd[0]));
			v2KeyToPntIdx.FindElement(triIdx[1],tess.GetSearchKey(ptr->dat.trVtHd[1]));
			v2KeyToPntIdx.FindElement(triIdx[2],tess.GetSearchKey(ptr->dat.trVtHd[2]));

			if(YSTRUE==pntList.IsInRange(triIdx[0]) && YSTRUE==pntList.IsInRange(triIdx[1]) && YSTRUE==pntList.IsInRange(triIdx[2]))
			{
				newPosArray.Append(pntList[triIdx[0]]);
				newPosArray.Append(pntList[triIdx[1]]);
				newPosArray.Append(pntList[triIdx[2]]);

				if(pntList.GetN()==texCoordArray.GetN())
				{
					newTexCoordArray.Append(texCoordArray[triIdx[0]]);
					newTexCoordArray.Append(texCoordArray[triIdx[1]]);
					newTexCoordArray.Append(texCoordArray[triIdx[2]]);
				}
			}
		}
		elm->SetElemType(Ys2DDrawingElement::TRIANGLES);
		elm->SetPointList(newPosArray);
		elm->SetTexCoordArray(newTexCoordArray);

		/* 
		sword.SetInitialPolygon(pntList.GetN(),pntList);
		// if(sword.Convexnize()==YSOK)
		if(sword.Triangulate()==YSOK)
		{
			int k;
			YsPrintf("SUBDIVIDED FROM NP=%d TO...\n",pntList.GetN());
			for(k=0; k<sword.GetNumPolygon(); k++)
			{
				int np;
				np=sword.GetNumVertexOfPolygon(k);
				pntList.Set(np,NULL);
				sword.GetVertexListOfPolygon(pntList,np,k);

				if(k==0)
				{
					SetPointList(elm,pntList);
				}
				else
				{
					YsListItem <Ys2DDrawingElement> *newElm;
					newElm=CreateElement(Ys2DDrawingElement::POLYGON);
					SetColor(&newElm->dat,elm->GetColor());
					SetPointList(&newElm->dat,pntList);
					SetVisibleDistance(&newElm->dat,elm->VisibleDist());
					elemList.MoveItemBefore(newElm,elm->thisInTheList);
				}
			}
		} */
	}
	DeleteCache();
	return YSOK;
}

YSRESULT Ys2DDrawing::SetColor(Ys2DDrawingElement *elm,const YsColor &c)
{
	elm->c=c;
	DeleteCache();
	return YSOK;
}

YSRESULT Ys2DDrawing::SetSecondColor(Ys2DDrawingElement *elm,const YsColor &c)
{
	elm->c2=c;
	DeleteCache();
	return YSOK;
}

YSRESULT Ys2DDrawing::SetVisibleDistance(Ys2DDrawingElement *elm,const double &dist)
{
	elm->visibleDist=dist;
	return YSOK;
}

YSRESULT Ys2DDrawing::SetPointList(Ys2DDrawingElement *elm,const YsArray <YsVec2> &pnt)
{
	elm->pnt=pnt;
	elm->cvx=YsCheckConvexByAngle2(pnt.GetN(),pnt);
	DeleteCache();
	return YSOK;
}

YSRESULT Ys2DDrawing::SetPointList(Ys2DDrawingElement *elm,int np,const YsVec2 pnt[])
{
	elm->pnt.Set(np,pnt);
	elm->cvx=YsCheckConvexByAngle2(np,pnt);
	DeleteCache();
	return YSOK;
}

YsListItem <Ys2DDrawingElement> *Ys2DDrawing::FindNextElem(const YsListItem <Ys2DDrawingElement> *ptr)
{
	return elemList.FindNext(ptr);
}

YsListItem <Ys2DDrawingElement> *Ys2DDrawing::FindPrevElem(const YsListItem <Ys2DDrawingElement> *ptr)
{
	return elemList.FindPrev(ptr);
}

const YsListItem <Ys2DDrawingElement> *Ys2DDrawing::FindNextElem(const YsListItem <Ys2DDrawingElement> *ptr) const
{
	return elemList.FindNext(ptr);
}

const YsListItem <Ys2DDrawingElement> *Ys2DDrawing::FindPrevElem(const YsListItem <Ys2DDrawingElement> *ptr) const
{
	return elemList.FindPrev(ptr);
}

////////////////////////////////////////////////////////////

YsElevationGrid::YsElevationGrid()
{
	owner=NULL;
	graphicCache=NULL;
	Initialize();
}

YsElevationGrid::~YsElevationGrid()
{
	Initialize();
}

void YsElevationGrid::RecomputeBoundingBox(void)
{
	YsBoundingBoxMaker3 makeBbx;
	int i;
	YsVec3 p;

	makeBbx.Begin(YsOrigin());
	for(i=0; i<node.GetN(); i++)
	{
		p.Set(0.0,node[i].y,0.0);
		makeBbx.Add(p);
	}
	p.Set(0.0,0.0,0.0);
	makeBbx.Add(p);
	p.Set(double(nx)*xWid,0.0,double(nz)*zWid);

	makeBbx.Add(p);

	makeBbx.Get(bbx[0],bbx[1]);
}

YSRESULT YsElevationGrid::RecomputeNormal(int x,int z)
{
	YsVec3 tri[6];
	if(GetTriangle(tri,x,z,0)==YSOK && GetTriangle(tri+3,x,z,1)==YSOK)
	{
		node[z*(nx+1)+x].nom[0]=(tri[2]-tri[0])^(tri[1]-tri[0]);
		node[z*(nx+1)+x].nom[0].Normalize();
		node[z*(nx+1)+x].cen[0]=(tri[0]+tri[1]+tri[2])/3.0;
		node[z*(nx+1)+x].nom[1]=(tri[5]-tri[3])^(tri[4]-tri[3]);
		node[z*(nx+1)+x].nom[1].Normalize();
		node[z*(nx+1)+x].cen[1]=(tri[3]+tri[4]+tri[5])/3.0;

		int i,j,k,m;
		for(i=x-1; i<=x+1; i++)
		{
			for(j=z-1; j<=z+1; j++)
			{
				int idx;
				idx=(nx+1)*j+i;
				if(0<=idx && idx<(nx+1)*(nz+1))
				{
					node[idx].nomOfNode=YsOrigin();
				}
			}
		}


		for(i=x-2; i<=x+2; i++)
		{
			for(j=z-2; j<=z+2; j++)
			{
				for(k=0; k<2; k++)
				{
					YsVec3 nom;
					YsElvGridFaceId tri[3];
					if(GetTriangleNodeId(tri,i,j,k)==YSOK)
					{
						nom=node[(nx+1)*j+i].nom[k];
						for(m=0; m<3; m++)
						{
							if(x-1<=tri[m].x && tri[m].x<=x+1 && z-1<=tri[m].z && tri[m].z<=z+1)
							{
								int idx;
								idx=(nx+1)*tri[m].z+tri[m].x;
								if(0<=idx && idx<(nx+1)*(nz+1))
								{
									node[idx].nomOfNode+=nom;
								}
							}
						}
					}
				}
			}
		}


		for(i=x-1; i<=x+1; i++)
		{
			for(j=z-1; j<=z+1; j++)
			{
				int idx;
				idx=(nx+1)*j+i;
				if(0<=idx && idx<(nx+1)*(nz+1))
				{
					node[idx].nomOfNode.Normalize();
				}
			}
		}

		return YSOK;
	}
	return YSERR;
}

void YsElevationGrid::RecomputeNormal(void)
{
	int i,j,baseIdx;
	double x,z;
	YsVec3 rc[4];

	for(j=0; j<=nz; j++)
	{
		for(i=0; i<=nx; i++)
		{
			node[(nx+1)*j+i].nomOfNode=YsOrigin();
		}
	}

	for(j=0; j<nz; j++)
	{
		z=double(j)*zWid;
		baseIdx=(nx+1)*j;

		for(i=0; i<nx; i++)
		{
			x=double(i)*xWid;

			rc[0].Set(x     ,node[baseIdx+i       ].y,z);
			rc[1].Set(x     ,node[baseIdx+i+nx+1  ].y,z+zWid);
			rc[2].Set(x+xWid,node[baseIdx+i+1     ].y,z);
			rc[3].Set(x+xWid,node[baseIdx+i+1+nx+1].y,z+zWid);

			//  1  3
			//
			//  0  2
			if(node[baseIdx+i].lup==YSTRUE)
			{
				// (2,3,1),(0,2,1)
				node[baseIdx+i].nom[0]=(rc[1]-rc[2])^(rc[3]-rc[2]);
				node[baseIdx+i].nom[1]=(rc[1]-rc[0])^(rc[2]-rc[0]);
				node[baseIdx+i].nom[0].Normalize();
				node[baseIdx+i].nom[1].Normalize();

				node[baseIdx+i].cen[0]=(rc[1]+rc[2]+rc[3])/3.0;
				node[baseIdx+i].cen[1]=(rc[1]+rc[0]+rc[2])/3.0;
			}
			else
			{
				// (0,3,1),(0,2,3)
				node[baseIdx+i].nom[0]=(rc[1]-rc[0])^(rc[3]-rc[0]);
				node[baseIdx+i].nom[1]=(rc[3]-rc[0])^(rc[2]-rc[0]);
				node[baseIdx+i].nom[0].Normalize();
				node[baseIdx+i].nom[1].Normalize();

				node[baseIdx+i].cen[0]=(rc[1]+rc[0]+rc[3])/3.0;
				node[baseIdx+i].cen[1]=(rc[3]+rc[0]+rc[2])/3.0;
			}

			int k;
			for(k=0; k<2; k++)
			{
				YsElvGridFaceId tri[3];
				GetTriangleNodeId(tri,i,j,k);
				node[(nx+1)*tri[0].z+tri[0].x].nomOfNode+=node[baseIdx+i].nom[k];
				node[(nx+1)*tri[1].z+tri[1].x].nomOfNode+=node[baseIdx+i].nom[k];
				node[(nx+1)*tri[2].z+tri[2].x].nomOfNode+=node[baseIdx+i].nom[k];
			}
		}
	}

	for(j=0; j<=nz; j++)
	{
		for(i=0; i<=nx; i++)
		{
			node[(nx+1)*j+i].nomOfNode.Normalize();
		}
	}
}

void YsElevationGrid::Initialize(void)
{
	nx=0;
	nz=0;
	xWid=0;
	zWid=0;
	sideWall[0]=YSFALSE;
	sideWall[1]=YSFALSE;
	sideWall[2]=YSFALSE;
	sideWall[3]=YSFALSE;
	node.Set(0,NULL);

	colorByElevation=YSFALSE;
	colorByElevation_Elevation[0]=0.0;
	colorByElevation_Elevation[1]=0.0;
	colorByElevation_Color[0]=YsWhite();
	colorByElevation_Color[1]=YsWhite();

	hasProtectPolygon=YSTFUNKNOWN;

	texLabel.Set("");
	texLabelNotFound=YSFALSE;
	texManCache=NULL;
	texHdCache=NULL;

	specular=YSFALSE;

	DeleteCache();
}

void YsElevationGrid::Create(int nBlkX,int nBlkZ,const double &xw,const double &zw,const YsColor &col)
{
	nx=nBlkX;
	nz=nBlkZ;
	xWid=xw;
	zWid=zw;
	sideWall[0]=YSFALSE;
	sideWall[1]=YSFALSE;
	sideWall[2]=YSFALSE;
	sideWall[3]=YSFALSE;
	sideWallColor[0]=YsWhite();
	sideWallColor[1]=YsWhite();
	sideWallColor[2]=YsWhite();
	sideWallColor[3]=YsWhite();
	node.Set((nx+1)*(nz+1),NULL);

	int i;
	for(i=0; i<(nx+1)*(nz+1); i++)
	{
		node[i].y=0.0;
		node[i].lup=YSFALSE;
		node[i].c[0]=col;
		node[i].c[1]=col;
		node[i].visible[0]=YSTRUE;
		node[i].visible[1]=YSTRUE;
		node[i].protectPolygon[0]=YSFALSE;
		node[i].protectPolygon[1]=YSFALSE;
	}

	RecomputeBoundingBox();
	RecomputeNormal();
}

void YsElevationGrid::SetTextureLabel(const YsString &texLabel)
{
	this->texLabel=texLabel;
	texLabelNotFound=YSFALSE;
	texManCache=NULL;
	texHdCache=NULL;
}

YsVec2i YsElevationGrid::GetNumBlock(void) const
{
	return YsVec2i(nx,nz);
}

YSRESULT YsElevationGrid::GetGridPosition(YsVec3 &pos,int x,int z) const
{
	if(0<=x && x<=nx && 0<=z && z<=nz)
	{
		pos.Set(double(x)*xWid,node[z*(nx+1)+x].y,double(z)*zWid);
		return YSOK;
	}
	pos=YsOrigin();
	return YSERR;
}

YSRESULT YsElevationGrid::GetTriangle(YsVec3 tri[3],int x,int z,int f) const
{
	if(0<=x && x<nx && 0<=z && z<nz && 0<=f && f<2)
	{
		int baseIdx;
		double zcoord,xcoord;
		YsVec3 rc[4],*t[6];

		zcoord=double(z)*zWid;
		xcoord=double(x)*xWid;
		baseIdx=z*(nx+1)+x;

		//  1  3
		//
		//  0  2
		rc[0].Set(xcoord     ,node[baseIdx       ].y,zcoord);
		rc[1].Set(xcoord     ,node[baseIdx  +nx+1].y,zcoord+zWid);
		rc[2].Set(xcoord+xWid,node[baseIdx+1     ].y,zcoord);
		rc[3].Set(xcoord+xWid,node[baseIdx+1+nx+1].y,zcoord+zWid);

		// Some functions are assuming that last two are diagonal nodes.
		if(node[baseIdx].lup==YSTRUE)
		{
			// (3,1,2),(0,2,1)  // Guarantee that last two are diagonal nodes.
			t[0]=&rc[3];
			t[1]=&rc[1];
			t[2]=&rc[2];
			t[3]=&rc[0];
			t[4]=&rc[2];
			t[5]=&rc[1];
		}
		else
		{
			// (1,0,3),(2,3,0)  // Guarantee that last two are diagonal nodes.
			t[0]=&rc[1];
			t[1]=&rc[0];
			t[2]=&rc[3];
			t[3]=&rc[2];
			t[4]=&rc[3];
			t[5]=&rc[0];
		}

		tri[0]=*t[f*3];
		tri[1]=*t[f*3+1];
		tri[2]=*t[f*3+2];

		return YSOK;
	}
	return YSERR;
}

YSRESULT YsElevationGrid::GetTriangleNormal(YsVec3 &nom,int x,int z,int f) const
{
	if(0<=x && x<nx && 0<=z && z<nz && 0<=f && f<2)
	{
		int baseIdx;
		baseIdx=z*(nx+1)+x;
		nom=node[baseIdx].nom[f];
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsElevationGrid::GetTriangleNodeId(YsElvGridFaceId tri[3],int x,int z,int f) const
{
	if(0<=x && x<nx && 0<=z && z<nz && 0<=f && f<2)
	{
		int baseIdx;
		double zcoord,xcoord;
		YsElvGridFaceId rc[4],*t[6];

		zcoord=double(z)*zWid;
		xcoord=double(x)*xWid;
		baseIdx=z*(nx+1)+x;

		//  Z
		//  1  3
		//
		//  0  2 X
		rc[0].x=x;
		rc[0].z=z;
		rc[0].tri=0;

		rc[1].x=x;
		rc[1].z=z+1;
		rc[1].tri=0;

		rc[2].x=x+1;
		rc[2].z=z;
		rc[2].tri=0;

		rc[3].x=x+1;
		rc[3].z=z+1;
		rc[3].tri=0;

		// Some functions are assuming that last two are diagonal nodes.
		if(node[baseIdx].lup==YSTRUE)
		{
			// (3,1,2),(0,2,1)  // Guarantee that last two are diagonal nodes.
			t[0]=&rc[3];
			t[1]=&rc[1];
			t[2]=&rc[2];
			t[3]=&rc[0];
			t[4]=&rc[2];
			t[5]=&rc[1];
		}
		else
		{
			// (1,0,3),(2,3,0)  // Guarantee that last two are diagonal nodes.
			t[0]=&rc[1];
			t[1]=&rc[0];
			t[2]=&rc[3];
			t[3]=&rc[2];
			t[4]=&rc[3];
			t[5]=&rc[0];
		}

		tri[0]=*t[f*3];
		tri[1]=*t[f*3+1];
		tri[2]=*t[f*3+2];

		return YSOK;
	}
	return YSERR;
}

YSRESULT YsElevationGrid::GetElevation(double &elv,int &ix,int &iz,int &f,const YsVec3 &pos) const
{
	int x,z;

	x=int(pos.x()/xWid);
	z=int(pos.z()/zWid);

	if(0<=x && x<nx && 0<=z && z<nz)
	{
		YsVec3 tri[2][3];
		YsVec2 tri2[2][3],tst;
		if(GetTriangle(tri[0],x,z,0)==YSOK && GetTriangle(tri[1],x,z,1)==YSOK)
		{
			int triId;
			YsVec3 o,n;

			tri2[0][0].GetXZ(tri[0][0]);
			tri2[0][1].GetXZ(tri[0][1]);
			tri2[0][2].GetXZ(tri[0][2]);
			tri2[1][0].GetXZ(tri[1][0]);
			tri2[1][1].GetXZ(tri[1][1]);
			tri2[1][2].GetXZ(tri[1][2]);

			tst.GetXZ(pos);

			if(YsCheckInsidePolygon2(tst,3,tri2[0])==YSINSIDE)
			{
				triId=0;
			}
			else
			{
				triId=1;
			}
			o=tri[triId][0];
			n=(tri[triId][1]-tri[triId][0])^(tri[triId][2]-tri[triId][0]);

			double a,b,c,d;
			a=n.x();
			b=n.y();
			c=n.z();
			d=o*n;

			// Now, the equation is ax+by+cz-d=0  -> y=(d-ax-cz)/b
			if(YsZero(b)!=YSTRUE)  // <- Shouldn't be YSTRUE, but just for safety.
			{
				ix=x;
				iz=z;
				f=triId;
				elv=(d-a*pos.x()-c*pos.z())/b;
				return YSOK;
			}
		}
	}
	return YSERR;
}

YSRESULT YsElevationGrid::GetNodeListFromFaceList(YsArray <YsElvGridFaceId> &nodeId,int nFace,const YsElvGridFaceId fcId[]) const
{
	int i,j;
	YsKeyStore visited;
	unsigned int ndId;
	YsElvGridFaceId tri[3];

	nodeId.Set(0,NULL);
	for(i=0; i<nFace; i++)
	{
		GetTriangleNodeId(tri,fcId[i].x,fcId[i].z,fcId[i].tri);
		for(j=0; j<3; j++)
		{
			ndId=LinearizeFaceId(tri[j].x,tri[j].z,tri[j].tri);
			if(visited.IsIncluded(ndId)!=YSTRUE)
			{
				visited.AddKey(ndId);
				nodeId.Append(tri[j]);
			}
		}
	}

	return YSOK;
}

const YsColor YsElevationGrid::ColorByElevation(const double &y) const
{
	YsColor col;
	if(y>colorByElevation_Elevation[1])
	{
		col=colorByElevation_Color[1];
	}
	else if(y<colorByElevation_Elevation[0])
	{
		col=colorByElevation_Color[0];
	}
	else
	{
		double t,r0,g0,b0,r1,g1,b1,r,g,b;
		t=(y-colorByElevation_Elevation[0])/(colorByElevation_Elevation[1]-colorByElevation_Elevation[0]);
		colorByElevation_Color[0].GetDoubleRGB(r0,g0,b0);
		colorByElevation_Color[1].GetDoubleRGB(r1,g1,b1);
		r=r0*(1.0-t)+r1*t;
		g=g0*(1.0-t)+g1*t;
		b=b0*(1.0-t)+b1*t;
		col.SetDoubleRGB(r,g,b);
	}
	return col;
}

const YsColor YsElevationGrid::GetTriangleColor(int x,int z,int f) const
{
	if(colorByElevation==YSTRUE)
	{
		double y;
		YsVec3 tri[3];
		GetTriangle(tri,x,z,f);
		y=(tri[0].y()+tri[1].y()+tri[2].y())/3.0;
		return ColorByElevation(y);
	}
	else if(0<=x && x<nx && 0<=z && z<nz)
	{
		return node[(nx+1)*z+x].c[f];
	}
	return YsBlack();
}

YSRESULT YsElevationGrid::GetNeighborTriangle(int &neix,int &neiz,int &neif,int x,int z,int f,int side) const
{
	if(0<=x && x<nx && 0<=z && z<nz && 0<=f && f<2)
	{
		if(side==0)
		{
			neix=x;
			neiz=z;
			neif=1-f;
			return YSOK;
		}

		int baseIdx;
		baseIdx=z*(nx+1)+x;

		if(node[baseIdx].lup==YSTRUE)
		{
			if(f==0)
			{
				if(side==1)
				{
					goto BOTTOMOFUPPERSQUARE;
				}
				else if(side==2)
				{
					goto LEFTOFRIGHTSQUARE;
				}
			}
			else
			{
				if(side==1)
				{
					goto TOPOFLOWERSQUARE;
				}
				else if(side==2)
				{
					goto RIGHTOFLEFTSQUARE;
				}
			}
		}
		else // lup==YSFALSE
		{
			if(f==0)
			{
				if(side==1)
				{
					goto RIGHTOFLEFTSQUARE;
				}
				else if(side==2)
				{
					goto BOTTOMOFUPPERSQUARE;
				}
			}
			else
			{
				if(side==1)
				{
					goto LEFTOFRIGHTSQUARE;
				}
				else if(side==2)
				{
					goto TOPOFLOWERSQUARE;
				}
			}
		}
	}
	return YSERR;

TOPOFLOWERSQUARE:
	neix=x;
	neiz=z-1;
	if(0<=neiz && neiz<nz)
	{
		neif=0;
		return YSOK;
	}
	return YSERR;
BOTTOMOFUPPERSQUARE:
	neix=x;
	neiz=z+1;
	if(0<=neiz && neiz<nz)
	{
		neif=1;
		return YSOK;
	}
	return YSERR;
LEFTOFRIGHTSQUARE:
	neix=x+1;
	neiz=z;
	if(0<=neix && neix<nx)
	{
		if(node[neiz*(nx+1)+neix].lup==YSTRUE)
		{
			neif=1;
		}
		else
		{
			neif=0;
		}
		return YSOK;
	}
	return YSERR;
RIGHTOFLEFTSQUARE:
	neix=x-1;
	neiz=z;
	if(0<=neix && neix<nx)
	{
		if(node[neiz*(nx+1)+neix].lup==YSTRUE)
		{
			neif=0;
		}
		else
		{
			neif=1;
		}
		return YSOK;
	}
	return YSERR;
}

unsigned int YsElevationGrid::LinearizeFaceId(int x,int z,int f) const
{
	return z*(nx+1)*2+x*2+f;
}

YSRESULT YsElevationGrid::UnlinearizeFaceId(int &x,int &z,int &f,unsigned int id) const
{
	x=(id%((nx+1)*2))/2;
	z= id/((nx+1)*2);
	f= id&1;
	return YSOK;
}

////////////////////////////////////////////////////////////

YsSceneryItem::YsSceneryItem()
{
	Initialize();
}

YsSceneryItem::OBJTYPE YsSceneryItem::GetObjType(void) const
{
	return objType;
}

YsScenery *YsSceneryItem::GetOwner(void) const
{
	return owner;
}

const char *YsSceneryItem::GetObjTypeString(OBJTYPE t)
{
	switch(t)
	{
	default:
		break;
	case GENERIC:
		return "GENERIC";
	case SHELL:
		return "SHELL";
	case ELEVATIONGRID:
		return "ELEVATIONGRID";
	case MAP:
		return "MAP";
	case SIGNBOARD:
		return "SIGNBOARD";
	case RECTREGION:
		return "RECTREGION";
	case SUBSCENARY:
		return "SUBSCENARY";
	}
	return "UNKNOWN";
}

const YsVec3 &YsSceneryItem::GetPosition(void) const
{
	return pos;
}

const YsAtt3 &YsSceneryItem::GetAttitude(void) const
{
	return att;
}

unsigned YsSceneryItem::GetSearchKey(void) const
{
	return searchKey;
}

void YsSceneryItem::Initialize()
{
	att.Set(0.0,0.0,0.0);
	pos=YsOrigin();

	owner=NULL;

	fName="";
	visibleDist=0.0;
	id=0;
	tagStr="";
}

void YsSceneryItem::GetBoundingBox(YsVec3 bbx[2]) const
{
	bbx[0]=YsOrigin();
	bbx[1]=YsOrigin();
}

int YsSceneryItem::GetId(void) const
{
	return id;
}

const char *YsSceneryItem::GetTag(void) const
{
	return tagStr.c_str();
}

YsScenery2DDrawing *YsSceneryItem::Get2DDrawing(void)
{
	if(MAP==GetObjType() || SIGNBOARD==GetObjType())
	{
		return (YsScenery2DDrawing *)this;
	}
	return NULL;
}

const YsScenery2DDrawing *YsSceneryItem::Get2DDrawing(void) const
{
	if(MAP==GetObjType() || SIGNBOARD==GetObjType())
	{
		return (const YsScenery2DDrawing *)this;
	}
	return NULL;
}

YsSceneryElevationGrid *YsSceneryItem::GetElevationGrid(void)
{
	if(ELEVATIONGRID==GetObjType())
	{
		return (YsSceneryElevationGrid *)this;
	}
	return NULL;
}

const YsSceneryElevationGrid *YsSceneryItem::GetElevationGrid(void) const
{
	if(ELEVATIONGRID==GetObjType())
	{
		return (const YsSceneryElevationGrid *)this;
	}
	return NULL;
}

YsSceneryRectRegion *YsSceneryItem::GetRectRegion(void)
{
	if(RECTREGION==GetObjType())
	{
		return (YsSceneryRectRegion *)this;
	}
	return NULL;
}

const YsSceneryRectRegion *YsSceneryItem::GetRectRegion(void) const
{
	if(RECTREGION==GetObjType())
	{
		return (const YsSceneryRectRegion *)this;
	}
	return NULL;
}

YsSceneryGndObj *YsSceneryItem::GetGndObj(void)
{
	if(GROUNDOBJECT==GetObjType())
	{
		return (YsSceneryGndObj *)this;
	}
	return NULL;
}

const YsSceneryGndObj *YsSceneryItem::GetGndObj(void) const
{
	if(GROUNDOBJECT==GetObjType())
	{
		return (const YsSceneryGndObj *)this;
	}
	return NULL;
}

YsSceneryAir *YsSceneryItem::GetAirObj(void)
{
	if(AIRCRAFT==GetObjType())
	{
		return (YsSceneryAir *)this;
	}
	return NULL;
}

const YsSceneryAir *YsSceneryItem::GetAirObj(void) const
{
	if(AIRCRAFT==GetObjType())
	{
		return (const YsSceneryAir *)this;
	}
	return NULL;
}

YsSceneryPointSet *YsSceneryItem::GetPointSet(void)
{
	if(POINTSET==GetObjType())
	{
		return (YsSceneryPointSet *)this;
	}
	return NULL;
}

const YsSceneryPointSet *YsSceneryItem::GetPointSet(void) const
{
	if(POINTSET==GetObjType())
	{
		return (const YsSceneryPointSet *)this;
	}
	return NULL;
}



////////////////////////////////////////////////////////////

void YsSceneryShell::Initialize(void)
{
	YsSceneryItem::Initialize();
	thisInTheList=NULL;
	shl.CleanUp();
	collShl.CleanUp();
	collLtc.CleanUp();
}

YSRESULT YsSceneryShell::Save(const char fn[]) const
{
	YsShellExtWriter writer;
	auto res=writer.SaveSrf(fn,shl);
	return res;
}

YSRESULT YsSceneryShell::Save(YsTextOutputStream &textOut) const
{
	YsShellExtWriter writer;
	auto res=writer.SaveSrf(textOut,shl);
	return res;
}

YSRESULT YsSceneryShell::LoadSrf(const YsTextFile &txtFile)
{
	YsShellExtReader reader;

	YsListItem <YsString> *str;
	reader.StartMergeSrf(shl);
	str=NULL;
	while((str=txtFile.GetText().FindNext(str))!=NULL)
	{
		if(reader.ReadSrfOneLine(shl,str->dat)!=YSOK)
		{
			return YSERR;
		}
	}
	reader.EndMergeSrf();
	return YSOK;
}

YSRESULT YsSceneryShell::SaveCollSrf(YsTextOutputStream &textOut) const
{
	YsShellExtWriter writer;
	auto res=writer.SaveSrf(textOut,collShl);
	return res;
}
YSRESULT YsSceneryShell::LoadCollSrf(const YsTextFile &txtFile)
{
	YsShellExtReader reader;

	YsListItem <YsString> *str;
	reader.StartMergeSrf(collShl);
	str=NULL;
	while((str=txtFile.GetText().FindNext(str))!=NULL)
	{
		if(reader.ReadSrfOneLine(collShl,str->dat)!=YSOK)
		{
			return YSERR;
		}
	}
	reader.EndMergeSrf();
	return YSOK;
}
YSRESULT YsSceneryShell::CacheCollLattice(void) const
{
	collLtc.CleanUp();
	auto &shl=GetCollisionShell();
	if(0<shl.GetNumPolygon())
	{
		collLtc.SetDomain(shl.Conv(),shl.GetNumPolygon()+1);
		return YSOK;
	}
	return YSERR;
}

const YsVisualSrf &YsSceneryShell::GetCollisionShell(void) const
{
	if(0<collShl.GetNumPolygon())
	{
		return collShl;
	}
	return shl;
}

void YsSceneryShell::GetBoundingBox(YsVec3 bbx[2]) const
{
	shl.GetBoundingBox(bbx[0],bbx[1]);
}





////////////////////////////////////////////////////////////





void YsSceneryElevationGrid::Initialize(void)
{
	YsSceneryItem::Initialize();
	thisInTheList=NULL;
	evg.Initialize();
}

YsSceneryElevationGrid::YsSceneryElevationGrid()
{
	evg.owner=this;
}

YSRESULT YsSceneryElevationGrid::Save(const char fn[]) const
{
	return evg.SaveTer(fn);
}

YSRESULT YsSceneryElevationGrid::Save(YsTextOutputStream &textOut) const
{
	return evg.SaveTer(textOut);
}

void YsSceneryElevationGrid::GetSideWallConfiguration(YSBOOL sw[4],YsColor swc[4]) const
{
	sw[0]=evg.sideWall[0];
	sw[1]=evg.sideWall[1];
	sw[2]=evg.sideWall[2];
	sw[3]=evg.sideWall[3];
	swc[0]=evg.sideWallColor[0];
	swc[1]=evg.sideWallColor[1];
	swc[2]=evg.sideWallColor[2];
	swc[3]=evg.sideWallColor[3];
}

void YsSceneryElevationGrid::GetBoundingBox(YsVec3 bbx[2]) const
{
	bbx[0]=evg.bbx[0];
	bbx[1]=evg.bbx[1];
}

void YsSceneryElevationGrid::RecomputeBoundingBox(void)
{
	evg.RecomputeBoundingBox();
}

void YsSceneryElevationGrid::RecomputeNormal(void)
{
	evg.RecomputeNormal();
}

YsVec2i YsSceneryElevationGrid::GetNumBlock(void) const
{
	return evg.GetNumBlock();
}

YSRESULT YsSceneryElevationGrid::GetNumBlock(int &nx,int &nz) const
{
	nx=evg.nx;
	nz=evg.nz;
	return YSOK;
}

YsVec3 YsSceneryElevationGrid::GetNodePosition(const YsVec2i &idx) const
{
	YsVec3 r;
	if(YSOK==GetNodePosition(r,idx.x(),idx.y()))
	{
		return r;
	}
	return YsOrigin();
}

YSRESULT YsSceneryElevationGrid::GetNodePosition(YsVec3 &pos,int x,int z) const
{
	if(0<=x && x<=evg.nx && 0<=z && z<=evg.nz)
	{
		int n;
		n=z*(evg.nx+1)+x;
		pos.Set(evg.xWid*double(x),evg.node[n].y,evg.zWid*double(z));
		return YSOK;
	}
	return YSERR;
}

const double &YsSceneryElevationGrid::GetNodeElevation(int x,int z) const
{
	if(0<=x && x<=evg.nx && 0<=z && z<=evg.nz)
	{
		int n;
		n=z*(evg.nx+1)+x;
		return evg.node[n].y;
	}
	return evg.node[0].y;
}

YSRESULT YsSceneryElevationGrid::GetTriangle(YsVec3 tri[3],int x,int z,int f) const
{
	return evg.GetTriangle(tri,x,z,f);
}

YSRESULT YsSceneryElevationGrid::GetTriangleNormal(YsVec3 &nom,int x,int z,int f) const
{
	return evg.GetTriangleNormal(nom,x,z,f);
}

YSRESULT YsSceneryElevationGrid::GetTriangleNodeId(YsElvGridFaceId idx[3],int x,int z,int f) const
{
	if(0<=x && x<evg.nx && 0<=z && z<evg.nz && 0<=f && f<2)
	{
		int baseIdx;
		YsElvGridFaceId rc[4],*t[6];

		baseIdx=z*(evg.nx+1)+x;

		//  1  3
		//
		//  0  2
		rc[0].x=x;
		rc[0].z=z;

		rc[1].x=x;;
		rc[1].z=z+1;

		rc[2].x=x+1;
		rc[2].z=z;

		rc[3].x=x+1;
		rc[3].z=z+1;

		if(evg.node[baseIdx].lup==YSTRUE)
		{
			// (2,3,1),(0,2,1)
			t[0]=&rc[2];
			t[1]=&rc[3];
			t[2]=&rc[1];
			t[3]=&rc[0];
			t[4]=&rc[2];
			t[5]=&rc[1];
		}
		else
		{
			// (0,3,1),(0,2,3)
			t[0]=&rc[0];
			t[1]=&rc[3];
			t[2]=&rc[1];
			t[3]=&rc[0];
			t[4]=&rc[2];
			t[5]=&rc[3];
		}

		idx[0]=*t[f*3];
		idx[1]=*t[f*3+1];
		idx[2]=*t[f*3+2];

		return YSOK;
	}
	return YSERR;
}

YSBOOL YsSceneryElevationGrid::IsTriangleVisible(int x,int z,int f) const
{
	if(0<=x && x<evg.nx && 0<=z && z<evg.nz && 0<=f && f<2)
	{
		return evg.node[z*(evg.nx+1)+x].visible[f];
	}
	return YSFALSE;
}

YSBOOL YsSceneryElevationGrid::IsTriangleProtectPolygon(int x,int z,int f) const
{
	if(0<=x && x<evg.nx && 0<=z && z<evg.nz && 0<=f && f<2)
	{
		return evg.node[z*(evg.nx+1)+x].protectPolygon[f];
	}
	return YSFALSE;
}

YSBOOL YsSceneryElevationGrid::GetTriangulation(int x,int z) const
{
	if(0<=x && x<evg.nx && 0<=z && z<evg.nz)
	{
		return evg.node[z*(evg.nx+1)+x].lup;
	}
	return YSFALSE;
}

YSBOOL YsSceneryElevationGrid::GetColorByElevation(double elv[2],YsColor col[2])  const
{
	elv[0]=evg.colorByElevation_Elevation[0];
	elv[1]=evg.colorByElevation_Elevation[1];
	col[0]=evg.colorByElevation_Color[0];
	col[1]=evg.colorByElevation_Color[1];
	return evg.colorByElevation;
}

const YsColor YsSceneryElevationGrid::GetTriangleColor(int x,int z,int f) const
{
	return evg.GetTriangleColor(x,z,f);
}

YSRESULT YsSceneryElevationGrid::GetNeighborTriangle(int &neix,int &neiz,int &neif,int x,int z,int f,int side) const
{
	return evg.GetNeighborTriangle(neix,neiz,neif,x,z,f,side);
}

unsigned int YsSceneryElevationGrid::LinearizeFaceId(int x,int z,int f) const
{
	return evg.LinearizeFaceId(x,z,f);
}

YSRESULT YsSceneryElevationGrid::UnlinearlizeFaceId(int &x,int &z,int &f,unsigned int id) const
{
	return evg.UnlinearizeFaceId(x,z,f,id);
}

void YsElevationGrid::SetSpecular(YSBOOL s)
{
	specular=s;
}

YSBOOL YsElevationGrid::GetSpecular(void) const
{
	return specular;
}

YSRESULT YsSceneryElevationGrid::GetNodeListFromFaceList(YsArray <YsElvGridFaceId> &nodeId,int nFace,const YsElvGridFaceId fcId[]) const
{
	return evg.GetNodeListFromFaceList(nodeId,nFace,fcId);
}


void YsScenery2DDrawing::Initialize(void)
{
	YsSceneryItem::Initialize();
	thisInTheList=NULL;
	drw.Initialize();
	drw.owner=this;
}

YSRESULT YsScenery2DDrawing::Save(const char fn[]) const
{
	return drw.SavePc2(fn);
}

YSRESULT YsScenery2DDrawing::Save(YsTextOutputStream &textOut) const
{
	return drw.SavePc2(textOut);
}

void YsScenery2DDrawing::GetBoundingBox(YsVec3 bbx[2]) const
{
	if(objType==MAP)
	{
		bbx[0].Set(drw.bbx[0].x(),0.0,drw.bbx[0].y());
		bbx[1].Set(drw.bbx[1].x(),0.0,drw.bbx[1].y());
	}
	else
	{
		bbx[0].Set(drw.bbx[0].x(),drw.bbx[0].y(),0.0);
		bbx[1].Set(drw.bbx[1].x(),drw.bbx[1].y(),0.0);
	}
}

void YsScenery2DDrawing::RecomputeBoundingBox(void)
{
	drw.RecomputeBoundingBox();
}

const Ys2DDrawing &YsScenery2DDrawing::GetDrawing(void) const
{
	return drw;
}

Ys2DDrawing &YsScenery2DDrawing::GetDrawing(void)
{
	return drw;
}

void YsSceneryRectRegion::Initialize(void)
{
	YsSceneryItem::Initialize();
	thisInTheList=NULL;
	min.Set(0.0,0.0);
	max.Set(0.0,0.0);
	subClassType=SUBCLASS_NONE;
}

void YsSceneryRectRegion::GetBoundingBox(YsVec3 bbx[2]) const
{
	bbx[0].SetXZ(min);
	bbx[0].SetY(0.0);
	bbx[1].SetXZ(max);
	bbx[1].SetY(0.0);
}

void YsSceneryRectRegion::GetMinMax(YsVec3 &min,YsVec3 &max) const
{
	min.Set(this->min.x(),0.0,this->min.y());
	max.Set(this->max.x(),0.0,this->max.y());
}

void YsSceneryRectRegion::GetMinMax(YsVec2 &min,YsVec2 &max) const
{
	min=this->min;
	max=this->max;
}

YsSceneryRectRegion::SUBCLASSTYPE YsSceneryRectRegion::GetSubClassType(void) const
{
	return subClassType;
}

void YsSceneryRectRegion::SetSubClassType(YsSceneryRectRegion::SUBCLASSTYPE subClassType)
{
	this->subClassType=subClassType;
}

/*static*/ const char *YsSceneryRectRegion::GetSubClassTypeString(YsSceneryRectRegion::SUBCLASSTYPE subClassType)
{
	switch(subClassType)
	{
	default:
	case SUBCLASS_NONE:
		return "NONE";
	case SUBCLASS_DEADLOCK_FREE_AIRPORT:
		return "DEADLOCKFREEAP";
	}
}

/*static*/ YsSceneryRectRegion::SUBCLASSTYPE YsSceneryRectRegion::GetSubClassTypeFromString(const char str[])
{
	if(0==strcmp("DEADLOCKFREEAP",str))
	{
		return SUBCLASS_DEADLOCK_FREE_AIRPORT;
	}
	return SUBCLASS_NONE;
}


////////////////////////////////////////////////////////////

void YsSceneryObjTemplate::Initialize(void)
{
	identName.Set("");
	dnmFn.Set("");
	dnmReady=YSFALSE;
	dnm.CleanUp();
}

const char *YsSceneryObjTemplate::ObjName(void) const
{
	return identName;
}

void YsSceneryObjTemplate::SetObjName(const char nm[])
{
	identName.Set(nm);
}

void YsSceneryObjTemplate::SetDnmFileName(const char fn[])
{
	dnmFn.Set(fn);
}

YsVisualDnm &YsSceneryObjTemplate::GetDnm(void) const
{
	if(dnmReady!=YSTRUE)
	{
		FILE *fp=fopen(dnmFn,"r");
		if(nullptr!=fp)
		{
			YsTextFileInputStream inStream(fp);
			dnm.CleanUp();
			if(dnm.Load(inStream,dnmFn)==YSOK)
			{
				dnm.SetStateOfAllPart(0);
			}
			else
			{
				dnm.CleanUp();
				YsPrintf("Warning: Cannot Load %s (Is it a dynamic model?)\n",dnmFn.Txt());
			}
			dnmReady=YSTRUE;

			fclose(fp);
		}
	}
	return dnm;
}

////////////////////////////////////////////////////////////

YsSceneryObjTemplate::YsSceneryObjTemplate()
{
	Initialize();
}

void YsSceneryGndObjTemplate::Initialize(void)
{
	YsSceneryObjTemplate::Initialize();

	ilsPos=YsOrigin();
	ilsAtt=YsZeroAtt();
	ilsRange=0.0;

	vorRange=0.0;
	ndbRange=0.0;
	gndType=FSSTATIC;

}

void YsSceneryGndObj::Initialize(void)
{
	gndObjTemplate=NULL;
	objName="";
	iff=0;
	gndFlag=0;
	primaryTarget=YSFALSE;

	motionPathName="";
	motionPathOffset=YSFALSE;
}

const YsSceneryGndObjTemplate *YsSceneryGndObj::GetLinkedObjTemplate(void) const
{
	return gndObjTemplate;
}

const char *YsSceneryGndObj::GetObjName(void) const
{
	return objName.c_str();
}

const int YsSceneryGndObj::GetIFF(void) const
{
	return iff;
}

const YSBOOL YsSceneryGndObj::IsPrimaryTarget(void) const
{
	return primaryTarget;
}

unsigned YsSceneryGndObj::GetFlag(void) const
{
	return gndFlag;
}

const char *YsSceneryGndObj::GetMotionPathName(void) const
{
	return motionPathName.c_str();
}

YSBOOL YsSceneryGndObj::GetMotionPathOffset(void) const
{
	return motionPathOffset;
}

void YsSceneryGndObj::GetBoundingBox(YsVec3 bbx[2]) const
{
	if(gndObjTemplate!=NULL)
	{
		gndObjTemplate->GetDnm().GetBoundingBox(bbx);
	}
	else
	{
		bbx[0]=YsOrigin();
		bbx[1]=YsOrigin();
	}
}


YsSceneryAirTemplate::YsSceneryAirTemplate()
{
	Initialize();
}

void YsSceneryAirTemplate::Initialize(void)
{
	YsSceneryObjTemplate::Initialize();
}

void YsSceneryAirAction::Initialize(void)
{
	actType=YsSceneryAirAction::DONOTHING;
	actParam[0]="";
	customIntention.CleanUp();
}

void YsSceneryAir::Initialize(void)
{
	airTemplate=NULL;
	objName.Set("");
	iff=0;
	ldg=YSTRUE;
	isPlayer=YSFALSE;
	airFlag=0;
	action.Initialize();
	otherCommand.CleanUp();
	landWhenLowFuel=0.0;
}

const int YsSceneryAir::GetIFF(void) const
{
	return iff;
}

const char *YsSceneryAir::GetObjName(void) const
{
	return objName;
}

YSBOOL YsSceneryAir::GetLandingGear(void) const
{
	return ldg;
}

unsigned int YsSceneryAir::GetFlag(void) const
{
	return airFlag;
}

unsigned int YsSceneryAir::GetFuel(void) const
{
	return fuel;
}

const double &YsSceneryAir::GetSpeed(void) const
{
	return speed;
}

const double &YsSceneryAir::GetLandWhenLowFuel(void) const
{
	return landWhenLowFuel;
}

const YsSceneryAirAction &YsSceneryAir::GetAction(void) const
{
	return action;
}

void YsSceneryAir::GetBoundingBox(YsVec3 bbx[2]) const
{
	if(airTemplate!=NULL)
	{
		airTemplate->GetDnm().GetBoundingBox(bbx);
	}
	else
	{
		bbx[0]=YsOrigin();
		bbx[1]=YsOrigin();
	}
}


////////////////////////////////////////////////////////////

YsSceneryPointSet::YsSceneryPointSet()
{
	shl2d=NULL;
	ltc=NULL;
	Initialize();
}

YsSceneryPointSet::~YsSceneryPointSet()
{
	if(ltc!=NULL)
	{
		delete ltc;
	}
	if(shl2d!=NULL)
	{
		delete shl2d;
	}
}

void YsSceneryPointSet::Initialize(void)
{
	isLoop=YSFALSE;
	areaType=YSSCNAREA_NOAREA;
	pnt.Set(0,NULL);

	shl2dReady=YSFALSE;

	if(ltc!=NULL)
	{
		delete ltc;
		ltc=NULL;
	}
	if(shl2d!=NULL)
	{
		delete shl2d;
		shl2d=NULL;
	}

	bbxValid=YSFALSE;
}

int YsSceneryPointSet::GetNumPoint(void) const
{
	return (int)pnt.GetN();
}

const YsVec3 *YsSceneryPointSet::GetPointArray(void) const
{
	return pnt.GetArray();
}

const YsVec3 YsSceneryPointSet::GetTransformedPoint(int idx) const
{
	YsVec3 pnt=this->pnt[idx];

	const YsSceneryItem *itm=this;
	while(itm!=NULL)
	{
		itm->GetAttitude().Mul(pnt,pnt);
		pnt+=itm->GetPosition();
		itm=itm->GetOwner();
	}

	return pnt;
}

YSBOOL YsSceneryPointSet::IsLoop(void) const
{
	return isLoop;
}

YSSCNAREATYPE YsSceneryPointSet::GetAreaType(void) const
{
	return areaType;
}

YSRESULT YsSceneryPointSet::SetPointList(int np,const YsVec3 p[],YSBOOL islp)
{
	isLoop=islp;
	pnt.Set(np,p);
	shl2dReady=YSFALSE;
	bbxValid=YSFALSE;
	return YSOK;
}

YSRESULT YsSceneryPointSet::SetPointList(int np,const YsVec3 p[])
{
	pnt.Set(np,p);
	shl2dReady=YSFALSE;
	bbxValid=YSFALSE;
	return YSOK;
}

void YsSceneryPointSet::SetAreaType(YSSCNAREATYPE areaType)
{
	this->areaType=areaType;
}

void YsSceneryPointSet::PrepareShell2d(void) const
{
	if(pnt.GetN()>=3)
	{
		int i,prvIdx;
		if(shl2d==NULL)
		{
			shl2d=new YsShell2d;
		}
		if(ltc==NULL)
		{
			ltc=new YsShell2dLattice;
		}

		shl2d->CleanUp();

		YsArray <YsShell2dVertexHandle> vtHd;
		vtHd.Set(pnt.GetN(),NULL);
		forYsArray(i,pnt)
		{
			YsVec2 p;
			p.GetXZ(pnt[i]);
			vtHd[i]=shl2d->AddVertexH(p);
		}

		prvIdx=(int)pnt.GetN()-1;
		forYsArray(i,vtHd)
		{
			shl2d->AddEdgeH(vtHd[prvIdx],vtHd[i]);
			prvIdx=i;
		}

		ltc->SetDomain(*shl2d,YsGreater(64,shl2d->GetNumEdge()+1));

		shl2dReady=YSTRUE;
	}
}

YSSIDE YsSceneryPointSet::CheckInsidePolygon(const double &x,const double &z) const
{
	if(pnt.GetN()>=3)
	{
		YsVec3 bbx[2];
		GetBoundingBox(bbx);
		if(bbx[0].x()<=x && x<=bbx[1].x() && bbx[0].z()<=z && z<=bbx[1].z())
		{
			if(shl2dReady!=YSTRUE)
			{
				PrepareShell2d();
			}

			if(shl2dReady==YSTRUE && ltc!=NULL)
			{
				YsVec2 p;
				p.Set(x,z);
				return ltc->CheckInsidePolygon(p);
			}
		}
	}
	return YSOUTSIDE;
}

void YsSceneryPointSet::GetBoundingBox(YsVec3 bbx[2]) const
{
	if(bbxValid==YSTRUE)
	{
		bbx[0]=bbxCache[0];
		bbx[1]=bbxCache[1];
	}
	else
	{
		YsBoundingBoxMaker3 makeBbx;
		makeBbx.Make(pnt.GetN(),pnt);
		makeBbx.Get(bbx[0],bbx[1]);

		bbxCache[0]=bbx[0];
		bbxCache[1]=bbx[1];
		bbxValid=YSTRUE;
	}
}

////////////////////////////////////////////////////////////

YsSceneryAirRoute::YsSceneryAirRoute()
{
	Initialize();
}

void YsSceneryAirRoute::Initialize(void)
{
	label.Set("");
	routeSequence.Clear();
}

int YsSceneryAirRoute::AddSequence(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType)
{
	const int seqIdx=(int)routeSequence.GetN();
	routeSequence.Increment();
	routeSequence.Last().Initialize();
	routeSequence.Last().segType=segType;
	return seqIdx;
}

void YsSceneryAirRoute::SetSequenceAltitude(int seqIdx,const double alt)
{
	routeSequence[seqIdx].altitude=alt;
}

void YsSceneryAirRoute::SetTakeOffClimbAltitude(int seqIdx,const double alt)
{
	routeSequence[seqIdx].takeOffClimbAlt=alt;
}

void YsSceneryAirRoute::SetSequenceLabel(int seqIdx,const char label[])
{
	routeSequence[seqIdx].label.Set(label);
}

void YsSceneryAirRoute::SetSequenceBeginApproachDist(int seqIdx,const double dist)
{
	routeSequence[seqIdx].beginApproachAt=dist;
}

void YsSceneryAirRoute::AddSequenceILS(int seqIdx,const char ilsName[])
{
	routeSequence[seqIdx].ilsArray.Append(YsString(ilsName));
}

void YsSceneryAirRoute::RouteSegment::Initialize(void)
{
	segType=ROUTE_SEGMENT_NULL;
	label.Set("");
	altitude=0.0;
	takeOffClimbAlt=YsUnitConv::FTtoM(2000.1);
	trafficPatternAlt=YsUnitConv::FTtoM(2000.1);
	beginApproachAt=36000.0;
	ilsArray.Clear();
	gndObjKey=-1;

	ClearCache();
}

void YsSceneryAirRoute::RouteSegment::ClearCache(void)
{
	pos=YsOrigin();
	ilsKeyArray.Clear();
	vfrApproachArray.Clear();
}


////////////////////////////////////////////////////////////

YsListAllocator <YsSceneryShell> YsScenery::shlAllocator(16);
YsListAllocator <YsSceneryElevationGrid> YsScenery::evgAllocator(16);
YsListAllocator <YsScenery2DDrawing> YsScenery::drwAllocator(16);
YsListAllocator <YsSceneryRectRegion> YsScenery::rgnAllocator(16);
YsListAllocator <YsScenery> YsScenery::scnAllocator(16);
YsListAllocator <YsSceneryGndObj> YsScenery::gndAllocator(16);
YsListAllocator <YsSceneryAir> YsScenery::airAllocator(16);
YsListAllocator <YsSceneryPointSet> YsScenery::pstAllocator(16);
YsListAllocator <YsTextFile> YsScenery::pckFileAllocator(16);


YsScenery::YsScenery() :
    shlList(shlAllocator),evgList(evgAllocator),mapList(drwAllocator),
    sbdList(drwAllocator),rgnList(rgnAllocator),scnList(scnAllocator),
    gndList(gndAllocator),airList(airAllocator),pstList(pstAllocator),
    pckFileList(pckFileAllocator)
{
	airRouteList.UseSharedSearchKeySeed(YsScenerySearchKeySeed);
	Initialize();
}

YsScenery::~YsScenery()
{
	DeleteList(this);
}

/* static */ const char *YsScenery::ErrorCodeToString(ERRORCODE err)
{
	switch(err)
	{
	case ERROR_NOERROR:
		return "ERROR_NOERROR";
	case ERROR_IO_FILE_NOT_FOUND:
		return "ERROR_IO_FILE_NOT_FOUND";
	case ERROR_IO_WRITE_ERROR:
		return "ERROR_IO_WRITE_ERROR";
	case ERROR_IO_NEED_NEW_VERSION:
		return "ERROR_IO_NEED_NEW_VERSION";
	case ERROR_IO_INVALID_FILE:
		return "ERROR_IO_INVALID_FILE";
	}
	return "Unknown Error";
}

void YsScenery::SelfDiagnostic(void)
{
	shlAllocator.SelfDiagnostic();
	evgAllocator.SelfDiagnostic();
	drwAllocator.SelfDiagnostic();
	rgnAllocator.SelfDiagnostic();
	scnAllocator.SelfDiagnostic();
	gndAllocator.SelfDiagnostic();
	airAllocator.SelfDiagnostic();  // 2008/01/13
	pstAllocator.SelfDiagnostic();
	pckFileAllocator.SelfDiagnostic();
}

void YsScenery::DeleteList(YsScenery *scn)
{
	YsListItem <YsScenery> *chd;
	chd=NULL;
	while((chd=scn->scnList.FindNext(chd))!=NULL)
	{
		DeleteList(&chd->dat);
	}
	scn->shlList.CleanUp();
	scn->evgList.CleanUp();
	scn->mapList.CleanUp();
	scn->sbdList.CleanUp();
	scn->rgnList.CleanUp();
	scn->gndList.CleanUp();
	scn->airList.CleanUp();  // 2008/01/13
	scn->pstList.CleanUp();
	scn->scnList.CleanUp();
}

void YsScenery::Initialize(void)
{
	YsSceneryItem::Initialize();

	mapDrawingOrderCache.CleanUp();
	mapElevationCache.CleanUp();

	idName.Set("");

	yfsHeader.CleanUp();

	lastError=ERROR_NOERROR;

	lastErrorNo=0;
	lastErrorStr.Set("");

	skyColor.SetIntRGB(23,106,189);
	gndColor.SetIntRGB(0,0,215);
	gndSpecular=YSFALSE;

	shlList.CleanUp();
	evgList.CleanUp();
	mapList.CleanUp();
	sbdList.CleanUp();
	rgnList.CleanUp();
	gndList.CleanUp();
	airList.CleanUp();  // 2008/01/13
	pstList.CleanUp();
	scnList.CleanUp();
	pckFileList.CleanUp();

	bbx[0]=YsOrigin();
	bbx[1]=YsOrigin();

	plgColorScale=1.0;
	linColorScale=1.0;
	pntColorScale=1.0;

	thisInTheList=NULL;

	areaType=YSSCNAREA_NOAREA;
	baseElevation=0.0;
	magneticVariation=0.0;

	canResume=YSTRUE;
	canContinue=YSTRUE;

	airRouteList.CleanUp();

	textureManager.CleanUp();

	CollectGarbage();

	// printf("shlAllocator\n");
	// shlAllocator.SelfDiagnostic();
	// printf("evgAllocator\n");
	// evgAllocator.SelfDiagnostic();
	// printf("drwAllocator\n");
	// drwAllocator.SelfDiagnostic();
	// printf("rgnAllocator\n");
	// rgnAllocator.SelfDiagnostic();
	// printf("scnAllocator\n");
	// scnAllocator.SelfDiagnostic();
	// printf("gndAllocator\n");
	// gndAllocator.SelfDiagnostic();
	// printf("pstAllocator\n");
	// pstAllocator.SelfDiagnostic();
	// printf("pckFileAllocator\n");
	// pckFileAllocator.SelfDiagnostic();
}

YsScenery::ERRORCODE YsScenery::GetLastError(void) const
{
	auto r=lastError;
	lastError=ERROR_NOERROR;
	return r;
}

void YsScenery::SetLastError(ERRORCODE err)
{
	lastError=err;
}

void YsScenery::ResetPosition(void)  // 2005/01/10
{
	pos=YsOrigin();
	att=YsZeroAtt();
}

void YsScenery::CollectGarbage(void)
{
	shlAllocator.CollectGarbage();
	evgAllocator.CollectGarbage();
	drwAllocator.CollectGarbage();
	rgnAllocator.CollectGarbage();
	scnAllocator.CollectGarbage();
	gndAllocator.CollectGarbage();
	airAllocator.CollectGarbage();  // 2008/01/13
	pstAllocator.CollectGarbage();
	pckFileAllocator.CollectGarbage();

	Ys2DDrawing::elemAllocator.CollectGarbage();
}

void YsScenery::RecomputeBoundingBox(void)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsScenery> *scn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryAir> *air;
	YsListItem <YsSceneryPointSet> *pst;

	YsVec3 itmBbx[2];
	YsBoundingBoxMaker3 makeBbx;
	makeBbx.Begin(YsOrigin());

	shl=NULL;
	while((shl=shlList.FindNext(shl))!=NULL)
	{
		shl->dat.shl.GetBoundingBox(itmBbx[0],itmBbx[1]);
		TransformBoundingBox(itmBbx,shl->dat.pos,shl->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	evg=NULL;
	while((evg=evgList.FindNext(evg))!=NULL)
	{
		evg->dat.evg.RecomputeBoundingBox();
		itmBbx[0]=evg->dat.evg.bbx[0];
		itmBbx[1]=evg->dat.evg.bbx[1];
		TransformBoundingBox(itmBbx,evg->dat.pos,evg->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	drw=NULL;
	while((drw=mapList.FindNext(drw))!=NULL)
	{
		drw->dat.drw.RecomputeBoundingBox();
		itmBbx[0].SetXZ(drw->dat.drw.bbx[0]);
		itmBbx[1].SetXZ(drw->dat.drw.bbx[1]);
		TransformBoundingBox(itmBbx,drw->dat.pos,drw->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	drw=NULL;
	while((drw=sbdList.FindNext(drw))!=NULL)
	{
		drw->dat.drw.RecomputeBoundingBox();
		itmBbx[0].SetXY(drw->dat.drw.bbx[0]);
		itmBbx[1].SetXY(drw->dat.drw.bbx[1]);
		TransformBoundingBox(itmBbx,drw->dat.pos,drw->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	rgn=NULL;
	while((rgn=rgnList.FindNext(rgn))!=NULL)
	{
		itmBbx[0].SetXZ(rgn->dat.min);
		itmBbx[1].SetXZ(rgn->dat.max);
		TransformBoundingBox(itmBbx,rgn->dat.pos,rgn->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	gnd=NULL;
	while((gnd=gndList.FindNext(gnd))!=NULL)
	{
		gnd->dat.GetBoundingBox(itmBbx);
		TransformBoundingBox(itmBbx,gnd->dat.pos,gnd->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	air=NULL;
	while(NULL!=(air=airList.FindNext(air)))
	{
		air->dat.GetBoundingBox(itmBbx);
		TransformBoundingBox(itmBbx,air->dat.pos,air->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	pst=NULL;
	while((pst=pstList.FindNext(pst))!=NULL)
	{
		pst->dat.GetBoundingBox(itmBbx);
		TransformBoundingBox(itmBbx,pst->dat.pos,pst->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.RecomputeBoundingBox();
		itmBbx[0]=scn->dat.bbx[0];
		itmBbx[1]=scn->dat.bbx[1];
		TransformBoundingBox(itmBbx,scn->dat.pos,scn->dat.att);
		makeBbx.Add(itmBbx[0]);
		makeBbx.Add(itmBbx[1]);
	}

	makeBbx.Get(bbx[0],bbx[1]);
}

void YsScenery::UnlinkGndObjTemplate(void)
{
	YsListItem <YsSceneryGndObj> *gnd;
	gnd=NULL;
	while((gnd=gndList.FindNext(gnd))!=NULL)
	{
		gnd->dat.gndObjTemplate=NULL;
	}

	YsListItem <YsScenery> *scn;
	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.UnlinkGndObjTemplate();
	}
}

void YsScenery::UnlinkAirTemplate(void)
{
	YsListItem <YsSceneryAir> *air;
	air=NULL;
	while((air=airList.FindNext(air))!=NULL)
	{
		air->dat.airTemplate=NULL;
	}

	YsListItem <YsScenery> *scn;
	scn=NULL;
	while(NULL!=(scn=scnList.FindNext(scn)))
	{
		scn->dat.UnlinkAirTemplate();
	}
}

void YsScenery::LinkGndObjTemplate(const YsListContainer <YsSceneryGndObjTemplate> &gndObjTemplateList)
{
	YsListItem <YsSceneryGndObj> *gnd;
	gnd=NULL;
	while((gnd=gndList.FindNext(gnd))!=NULL)
	{
		YsListItem <YsSceneryGndObjTemplate> *gndTmp;
		gndTmp=NULL;
		while((gndTmp=gndObjTemplateList.FindNext(gndTmp))!=NULL)
		{
			if(strcmp(gnd->dat.objName,gndTmp->dat.ObjName())==0)
			{
				gnd->dat.gndObjTemplate=&gndTmp->dat;
				break;
			}
		}
	}

	YsListItem <YsScenery> *scn;
	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.LinkGndObjTemplate(gndObjTemplateList);
	}
}

void YsScenery::LinkAirTemplate(YsSceneryAir *air,const YsListContainer <YsSceneryAirTemplate> &airTemplateList)
{
	YsListItem <YsSceneryAirTemplate> *airTmp;
	airTmp=NULL;
	while((airTmp=airTemplateList.FindNext(airTmp))!=NULL)
	{
		if(strcmp(air->objName,airTmp->dat.ObjName())==0)
		{
			air->airTemplate=&airTmp->dat;
			break;
		}
	}
}

void YsScenery::LinkAirTemplate(const YsListContainer <YsSceneryAirTemplate> &airTemplateList)
{
	YsListItem <YsSceneryAir> *air;
	air=NULL;
	while((air=airList.FindNext(air))!=NULL)
	{
		LinkAirTemplate(&air->dat,airTemplateList);
	}

	YsListItem <YsScenery> *scn;
	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.LinkAirTemplate(airTemplateList);
	}
}

void YsScenery::GetBoundingBox(YsVec3 b[2]) const
{
	b[0]=bbx[0];
	b[1]=bbx[1];
}

YSSCNAREATYPE YsScenery::GetAreaTypeFromString(const char str[])
{
	if(strcmp(str,"NOAREA")==0)
	{
		return YSSCNAREA_NOAREA;
	}
	if(strcmp(str,"WATER")==0)
	{
		return YSSCNAREA_WATER;
	}
	if(strcmp(str,"LAND")==0)
	{
		return YSSCNAREA_LAND;
	}
	return YSSCNAREA_NOAREA;
}

const char *YsScenery::GetAreaTypeString(YSSCNAREATYPE areaType)
{
	switch(areaType)
	{
	default:
	case YSSCNAREA_NOAREA:
		return "NOAREA";
	case YSSCNAREA_WATER:
		return "WATER";
	case YSSCNAREA_LAND:
		return "LAND";
	}
}

void YsScenery::SetColorScale(const double &plgScale,const double &linScale,const double &pntScale)
{
	plgColorScale=plgScale;
	linColorScale=linScale;
	pntColorScale=pntScale;
	YsListItem <YsScenery> *scn;
	scn=NULL;
	while((scn=FindNextChildScenery(scn))!=NULL)
	{
		scn->dat.SetColorScale(plgScale,linScale,pntScale);
	}
}

void YsScenery::CalculateMapElevationCache(void)
{

	mapElevationCache.Set(0,NULL);
	CalculateMapElevationCache(mapElevationCache,YsIdentity4x4());

	YsQuickSort <double,int> (mapElevationCache.GetN(),mapElevationCache,NULL);
	for(auto i=mapElevationCache.GetN()-1; i>0; i--)
	{
		if(mapElevationCache[i]-mapElevationCache[i-1]<YsTolerance)
		{
			mapElevationCache.Delete(i);
		}
	}
}

void YsScenery::CalculateMapElevationCache(YsArray <double> &cache,const YsMatrix4x4 &modelTfm)
{
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsScenery> *scn;
	YsMatrix4x4 newModelTfm(YSFALSE),shlTfm(YSFALSE);

	newModelTfm=modelTfm;
	newModelTfm.Translate(pos);
	newModelTfm.RotateXZ(att.h());
	newModelTfm.RotateZY(att.p());
	newModelTfm.RotateXY(att.b());

	drw=NULL;
	while((drw=mapList.FindNext(drw))!=NULL)
	{
		YsVec3 pos;
		newModelTfm.Mul(pos,drw->dat.pos,1.0);
		cache.Append(pos.y());
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.CalculateMapElevationCache(cache,newModelTfm);
	}
}

void YsScenery::CacheMapDrawingOrder(const double tol)
{
	mapDrawingOrderCache=MakeMapDrawingOrder(YsIdentity4x4(),tol);
	mapDrawingOrderCache.cached=YSTRUE;
}

YSBOOL YsScenery::MapDrawingOrderCached(void) const
{
	return mapDrawingOrderCache.cached;
}

YsScenery::MapDrawingOrder YsScenery::MakeMapDrawingOrder(const YsMatrix4x4 &sceneryToWorldTfm,const double tol) const
{
	MapDrawingOrder mdo;
	MakeMapDrawingOrder(mdo,sceneryToWorldTfm,tol);
	printf("%d same-plane map groups.\n",(int)mdo.samePlaneMapGroup.GetN());
	return mdo;
}
void YsScenery::MakeMapDrawingOrder(MapDrawingOrder &mdo,const YsMatrix4x4 &sceneryToWorldTfm,const double tol) const
{
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsScenery> *scn;
	YsMatrix4x4 nextSceneryToWorldTfm(YSFALSE),shlTfm(YSFALSE);

	nextSceneryToWorldTfm=sceneryToWorldTfm;
	nextSceneryToWorldTfm.Translate(pos);
	nextSceneryToWorldTfm.RotateXZ(att.h());
	nextSceneryToWorldTfm.RotateZY(att.p());
	nextSceneryToWorldTfm.RotateXY(att.b());

	drw=NULL;
	while((drw=mapList.FindNext(drw))!=NULL)
	{
		YsVec3 pos,nom;
		YsMatrix4x4 mapToWorldTfm=nextSceneryToWorldTfm;
		mapToWorldTfm.Translate(drw->dat.pos);
		mapToWorldTfm.RotateXZ(drw->dat.att.h());
		mapToWorldTfm.RotateZY(drw->dat.att.p());
		mapToWorldTfm.RotateXY(drw->dat.att.b());

		mapToWorldTfm.Mul(pos,YsVec3::Origin(),1.0);
		mapToWorldTfm.Mul(nom,YsYVec(),0.0);
		// Find place in mdo, and add.
		YSBOOL added=YSFALSE;
		for(auto &samePlaneMapGroup : mdo.samePlaneMapGroup)
		{
			if(samePlaneMapGroup.nom==nom)
			{
				double d=(pos-samePlaneMapGroup.org)*samePlaneMapGroup.nom;
				if(fabs(d)<tol)
				{
					samePlaneMapGroup.mapDrawingInfo.Increment();
					samePlaneMapGroup.mapDrawingInfo.Last().mapPtr=&drw->dat;
					samePlaneMapGroup.mapDrawingInfo.Last().mapOwnerToWorldTfm=nextSceneryToWorldTfm;
					added=YSTRUE;
				}
			}
		}
		if(YSTRUE!=added)
		{
			mdo.samePlaneMapGroup.Increment();
			auto &samePlaneMapGroup=mdo.samePlaneMapGroup.Last();
			samePlaneMapGroup.Initialize();
			samePlaneMapGroup.nom=nom;
			samePlaneMapGroup.org=pos;
			samePlaneMapGroup.mapDrawingInfo.Increment();
			samePlaneMapGroup.mapDrawingInfo.Last().mapPtr=&drw->dat;
			samePlaneMapGroup.mapDrawingInfo.Last().mapOwnerToWorldTfm=nextSceneryToWorldTfm;
		}
	}

	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		scn->dat.MakeMapDrawingOrder(mdo,nextSceneryToWorldTfm,tol);
	}
}

YSRESULT YsScenery::RecursivelyUpdateBoundingBox(const YsSceneryItem *itm)
{
	YsVec3 itmBbx[2];
	if(itm->owner!=NULL)
	{
		if(itm->objType==YsSceneryItem::SHELL)
		{
			YsSceneryShell *shl;
			shl=(YsSceneryShell *)itm;
			shl->shl.GetBoundingBox(itmBbx[0],itmBbx[1]);
		}
		else if(itm->objType==YsSceneryItem::ELEVATIONGRID)
		{
			YsSceneryElevationGrid *evg;
			evg=(YsSceneryElevationGrid *)itm;
			itmBbx[0]=evg->evg.bbx[0];
			itmBbx[1]=evg->evg.bbx[1];
		}
		else if(itm->objType==YsSceneryItem::MAP)
		{
			YsScenery2DDrawing *drw;
			drw=(YsScenery2DDrawing *)itm;
			itmBbx[0].SetXZ(drw->drw.bbx[0]);
			itmBbx[1].SetXZ(drw->drw.bbx[1]);
		}
		else if(itm->objType==YsSceneryItem::SIGNBOARD)
		{
			YsScenery2DDrawing *drw;
			drw=(YsScenery2DDrawing *)itm;
			itmBbx[0].SetXY(drw->drw.bbx[0]);
			itmBbx[1].SetXY(drw->drw.bbx[1]);
		}

		else if(itm->objType==YsSceneryItem::RECTREGION)
		{
			YsSceneryRectRegion *rgn;
			rgn=(YsSceneryRectRegion *)itm;
			itmBbx[0].SetXY(rgn->min);
			itmBbx[1].SetXY(rgn->max);
		}
		else if(itm->objType==YsSceneryItem::SUBSCENARY)
		{
			YsScenery *scn;
			scn=(YsScenery *)itm;
			itmBbx[0]=scn->bbx[0];
			itmBbx[1]=scn->bbx[1];
		}
		else if(itm->objType==YsSceneryItem::GROUNDOBJECT)
		{
			YsSceneryGndObj *gnd;
			gnd=(YsSceneryGndObj *)itm;
			gnd->GetBoundingBox(itmBbx);
		}
		else if(itm->objType==YsSceneryItem::AIRCRAFT)
		{
			YsSceneryAir *air;
			air=(YsSceneryAir *)itm;
			air->GetBoundingBox(itmBbx);
		}
		else
		{
			itmBbx[0]=YsOrigin();
			itmBbx[1]=YsOrigin();
		}
		TransformBoundingBox(itmBbx,itm->pos,itm->att);
		YsScenery *owner;
		owner=(YsScenery *)itm->owner;
		if(YsCheckInsideBoundingBox3(itmBbx[0],owner->bbx[0],owner->bbx[1])!=YSTRUE ||
		   YsCheckInsideBoundingBox3(itmBbx[1],owner->bbx[0],owner->bbx[1])!=YSTRUE)
		{
			YsBoundingBoxMaker3 makeBbx;
			makeBbx.Begin(owner->bbx[0]);
			makeBbx.Add(owner->bbx[1]);
			makeBbx.Add(itmBbx[0]);
			makeBbx.Add(itmBbx[1]);
			makeBbx.Get(owner->bbx[0],owner->bbx[1]);
			RecursivelyUpdateBoundingBox(owner);
		}
	}
	return YSOK;
}

YSRESULT YsScenery::GetTransformation(YsMatrix4x4 &mat,const YsSceneryItem *itm) const
{
	if(itm==NULL)
	{
		ComputeTransformation(mat,pos,att);
	}
	else
	{
		YsMatrix4x4 localTfm;
		GetParentTransformation(mat,itm);
		mat*=ComputeTransformation(localTfm,itm->pos,itm->att);
	}
	return YSOK;
}

YsMatrix4x4 YsScenery::GetTransformation(const YsSceneryItem *itm) const
{
	YsMatrix4x4 mat;
	GetTransformation(mat,itm);
	return mat;
}

YSRESULT YsScenery::GetParentTransformation(YsMatrix4x4 &mat,const YsSceneryItem *itm) const
{
	int i;
	YsArray <YsScenery *,16> parentLink;
	YsMatrix4x4 localTfm;

	mat.Initialize();
	if(itm==NULL)
	{
		return YSOK;
	}

	YsScenery *scn;
	scn=itm->owner;
	while(scn!=NULL)
	{
		parentLink.Append(scn);
		scn=scn->owner;
	}
	for(i=(int)parentLink.GetN()-1; i>=0; i--)
	{
		mat*=ComputeTransformation(localTfm,parentLink[i]->pos,parentLink[i]->att);
	}
	return YSOK;
}

const YsMatrix4x4 &YsScenery::ComputeTransformation(YsMatrix4x4 &tfm,const YsVec3 &tra,const YsAtt3 &att) const
{
	tfm.Initialize();
	tfm.Translate(tra);
	tfm.RotateXZ(att.h());
	tfm.RotateZY(att.p());
	tfm.RotateXY(att.b());
	return tfm;
}

YSRESULT YsScenery::GetOrigin(YsVec3 &pos,const YsSceneryItem *itm) const
{
	YsMatrix4x4 mat(YSFALSE);
	if(GetTransformation(mat,itm)==YSOK)
	{
		mat.Mul(pos,YsOrigin(),1.0);
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsScenery::GetTransformedBoundingBox(YsVec3 bbx[8],const YsSceneryItem *itm) const
{
	YsMatrix4x4 mat(YSFALSE);
	if(GetTransformation(mat,itm)==YSOK)
	{
		int i;
		YsVec3 itmBbx[2];
		itm->GetBoundingBox(itmBbx);
		for(i=0; i<8; i++)
		{
			bbx[i].Set(itmBbx[(i/4)&1].x(),itmBbx[(i/2)&1].y(),itmBbx[i&1].z());
			mat.Mul(bbx[i],bbx[i],1.0);
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT YsScenery::GetTransformedBoundingBox(YsVec3 bbx[8],const YsSceneryItem *itm,const YsMatrix4x4 &tfm) const
{
	int i;
	YsVec3 itmBbx[2];
	itm->GetBoundingBox(itmBbx);
	for(i=0; i<8; i++)
	{
		bbx[i].Set(itmBbx[(i/4)&1].x(),itmBbx[(i/2)&1].y(),itmBbx[i&1].z());
		bbx[i].RotateXY(itm->att.b());
		bbx[i].RotateZY(itm->att.p());
		bbx[i].RotateXZ(itm->att.h());
		bbx[i]+=itm->pos;
		tfm.Mul(bbx[i],bbx[i],1.0);
	}
	return YSOK;
}

// tanFov : (X or Y)/Z
//       Z
//       |      /
//       |     /
//       |    /
//       |FOV/
//       |  /
//       | /
//       |/
//       +-------X
YSBOOL YsScenery::IsItemVisible
	    (const YsMatrix4x4 &viewModelTfm,const YsMatrix4x4 &projTfm,
	     const YsSceneryItem *itm) const
{
	YsVec3 minmax[2];
	itm->GetBoundingBox(minmax);

	if(YsTolerance<itm->visibleDist)
	{
		auto cen=(minmax[0]+minmax[1])/2.0;
		cen.RotateXY(itm->att.b());
		cen.RotateZY(itm->att.p());
		cen.RotateXZ(itm->att.h());
		cen+=itm->pos;
		viewModelTfm.Mul(cen,cen,1.0);
		if(YsSqr(itm->visibleDist)<cen.GetSquareLength())
		{
			return YSFALSE;
		}
	}

	auto viewModelItemTfm=viewModelTfm;
	viewModelItemTfm.Translate(itm->pos);
	viewModelItemTfm.RotateXZ(itm->att.h());
	viewModelItemTfm.RotateZY(itm->att.p());
	viewModelItemTfm.RotateXY(itm->att.b());

	// printf("####\n");
	// projTfm.Print();
	// for(int i=0; i<8; i++)
	// {
	// 	YsVec3 tfm(minmax[(i/4)&1].x(),minmax[(i/2)&1].y(),minmax[i&1].z());
	// 	tfm=viewModelItemTfm*tfm;
	// 	auto proj=projTfm*tfm;
	// 	printf("%s|%s\n",tfm.Txt(),proj.Txt());
	// }

	return YsIsBoundingBoxVisible(minmax,viewModelItemTfm,projTfm);
}

void YsScenery::TransformBoundingBox(YsVec3 bbx[2],const YsVec3 &tra,const YsAtt3 &rot) const
{
	int i;
	YsBoundingBoxMaker3 makeBbx;
	YsVec3 corner[8];

	makeBbx.Begin();
	for(i=0; i<8; i++)
	{
		corner[i].Set(bbx[(i/4)&1].x(),bbx[(i/2)&1].y(),bbx[i&1].z());
		corner[i].RotateXY(rot.b());
		corner[i].RotateZY(rot.p());
		corner[i].RotateXZ(rot.h());
		corner[i]+=tra;
		makeBbx.Add(corner[i]);
	}

	makeBbx.Get(bbx[0],bbx[1]);
}

YSRESULT YsScenery::GetRegionRect(YsVec3 rec[4],const YsSceneryRectRegion *rgn) const
{
	rec[0].Set(rgn->min.x(),0.0,rgn->min.y());
	rec[1].Set(rgn->max.x(),0.0,rgn->min.y());
	rec[2].Set(rgn->max.x(),0.0,rgn->max.y());
	rec[3].Set(rgn->min.x(),0.0,rgn->max.y());

	YsMatrix4x4 tfm(YSFALSE);
	GetTransformation(tfm,rgn);

	tfm.Mul(rec[0],rec[0],1.0);
	tfm.Mul(rec[1],rec[1],1.0);
	tfm.Mul(rec[2],rec[2],1.0);
	tfm.Mul(rec[3],rec[3],1.0);

	return YSOK;
}

YsVec3 YsScenery::GetRectRegionCenter(const YsSceneryRectRegion *rgn) const
{
	YsVec3 rect[4];
	if(YSOK==GetRegionRect(rect,rgn))
	{
		return YsGetCenter(4,rect);
	}
	return YsOrigin();
}

YSBOOL YsScenery::IsInsideRectRegion(const YsVec3 &pos,const YsSceneryRectRegion *rgn) const
{
	YsVec3 tPos;
	YsVec2 tPos2;
	YsMatrix4x4 tfm(YSFALSE);
	GetTransformation(tfm,rgn);

	tfm.MulInverse(tPos,pos,1.0);

	tPos2.GetXZ(tPos);
	return YsCheckInsideBoundingBox2(tPos2,rgn->min,rgn->max);
}

YSSCNAREATYPE YsScenery::GetAreaTypeFromPoint(const YsVec3 &pos) const
{
	YSSCNAREATYPE areaType;
	areaType=GetAreaTypeFromPoint_Recursion(pos);
	if(areaType!=YSSCNAREA_NOAREA)
	{
		return areaType;
	}
	return this->areaType;
}

YSSCNAREATYPE YsScenery::GetAreaTypeFromPoint_Recursion(const YsVec3 &posOutside) const
{
	YsVec2 tst2,bbx2[2];
	YsVec3 tst,pos;
	const YsListItem <YsSceneryPointSet> *pst;

	pos=posOutside;
	pos-=this->pos;
	pos.RotateXZ(-att.h());
	pos.RotateZY(-att.p());
	pos.RotateXY(-att.b());

	tst2.GetXZ(pos);

	bbx2[0].GetXZ(bbx[0]);
	bbx2[1].GetXZ(bbx[1]);

	if(YsCheckInsideBoundingBox2(tst2,bbx2[0],bbx2[1])==YSTRUE)
	{
		YSSCNAREATYPE areaType;

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			areaType=scn->dat.GetAreaTypeFromPoint_Recursion(pos);
			if(areaType!=YSSCNAREA_NOAREA)
			{
				return areaType;
			}
		}


		pst=NULL;
		while((pst=FindPrevPointSet(pst))!=NULL)
		{
			tst=pos;
			tst-=pst->dat.pos;
			tst.RotateXZ(-pst->dat.att.h());
			tst.RotateZY(-pst->dat.att.p());
			tst.RotateXY(-pst->dat.att.b());

			if(pst->dat.GetAreaType()!=YSSCNAREA_NOAREA && pst->dat.CheckInsidePolygon(tst.x(),tst.z())==YSINSIDE)
			{
				return pst->dat.GetAreaType();
			}
		}
	}
	return YSSCNAREA_NOAREA;
}

YSRESULT YsScenery::GetFirstPointOfPointSet(YsVec3 &point,const YsSceneryPointSet *pst) const
{
	if(0<pst->GetNumPoint())
	{
		YsMatrix4x4 tfm(YSFALSE);
		GetTransformation(tfm,pst);
		tfm.Mul(point,pst->GetPointArray()[0],1.0);
		return YSOK;
	}
	return YSERR;
}

double YsScenery::GetElevation(const YsSceneryItem *&evg,const YsVec3 &pos) const
{
	double elv;
	elv=0.0;
	evg=NULL;
	if(GetElevation_Recursion(evg,elv,pos)==YSOK)
	{
		return elv;
	}
	else
	{
		return 0.0;
	}
}

YSRESULT YsScenery::GetElevation_Recursion(const YsSceneryItem *&itm,double &elv,const YsVec3 &posOutside) const
{
	YsVec2 tst2,bbx2[2];
	YsVec3 tst,pos;
	const YsListItem <YsSceneryElevationGrid> *evg;

	pos=posOutside;
	pos-=this->pos;
	pos.RotateXZ(-att.h());
	pos.RotateZY(-att.p());
	pos.RotateXY(-att.b());

	tst2.GetXZ(pos);
	bbx2[0].GetXZ(bbx[0]);
	bbx2[1].GetXZ(bbx[1]);

	if(YsCheckInsideBoundingBox2(tst2,bbx2[0],bbx2[1])==YSTRUE)
	{
		evg=NULL;
		while((evg=FindNextElevationGrid(evg))!=NULL)
		{
			tst=pos;
			tst-=evg->dat.pos;
			tst.RotateXZ(-evg->dat.att.h());
			tst.RotateZY(-evg->dat.att.p());
			tst.RotateXY(-evg->dat.att.b());

			tst2.GetXZ(tst);

			YsVec3 bbx[2];
			evg->dat.GetBoundingBox(bbx);
			bbx2[0].GetXZ(bbx[0]);
			bbx2[1].GetXZ(bbx[1]);

			if(YsCheckInsideBoundingBox2(tst2,bbx2[0],bbx2[1])==YSTRUE)
			{
				int x,z,f;
				double tstElv;
				if(evg->dat.evg.GetElevation(tstElv,x,z,f,tst)==YSOK && tstElv>elv)
				{
					itm=&evg->dat;
					elv=tstElv;
				}
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			scn->dat.GetElevation_Recursion(itm,elv,pos);
		}
	}
	return YSOK;
}

void YsScenery::GetElevationAndNormal(const YsSceneryItem *&evg,double &elv,YsVec3 &nom,const YsVec3 &pos)
{
	elv=0.0;
	evg=NULL;
	nom=YsYVec();
	GetElevationAndNormal_Recursion(evg,elv,nom,pos);
}

YSRESULT YsScenery::GetElevationAndNormal_Recursion(const YsSceneryItem *&itm,double &elv,YsVec3 &nom,const YsVec3 &posOutside) const
{
	YsVec2 tst2,bbx2[2];
	YsVec3 tst,pos;
	const YsListItem <YsSceneryElevationGrid> *evg;

	pos=posOutside;
	pos-=this->pos;
	pos.RotateXZ(-att.h());
	pos.RotateZY(-att.p());
	pos.RotateXY(-att.b());

	tst2.GetXZ(pos);
	bbx2[0].GetXZ(bbx[0]);
	bbx2[1].GetXZ(bbx[1]);

	if(YsCheckInsideBoundingBox2(tst2,bbx2[0],bbx2[1])==YSTRUE)
	{
		evg=NULL;
		while((evg=FindNextElevationGrid(evg))!=NULL)
		{
			tst=pos;
			tst-=evg->dat.pos;
			tst.RotateXZ(-evg->dat.att.h());
			tst.RotateZY(-evg->dat.att.p());
			tst.RotateXY(-evg->dat.att.b());

			tst2.GetXZ(tst);

			YsVec3 bbx[2];
			evg->dat.GetBoundingBox(bbx);
			bbx2[0].GetXZ(bbx[0]);
			bbx2[1].GetXZ(bbx[1]);

			if(YsCheckInsideBoundingBox2(tst2,bbx2[0],bbx2[1])==YSTRUE)
			{
				int x,z,f;
				double tstElv;
				YsVec3 tri[3];
				const YsSceneryItem *itm;
				if(evg->dat.evg.GetElevation(tstElv,x,z,f,tst)==YSOK && tstElv>elv)
				{
					itm=&evg->dat;
					elv=tstElv;
					evg->dat.GetTriangleNormal(nom,x,z,f); // evg->dat.evg.GetTriangle(tri,x,z,f);
					                                   // nom=(tri[1]-tri[0])^(tri[2]-tri[0]);
					itm=&evg->dat;
					while(itm!=NULL)
					{
						itm->GetAttitude().Mul(nom,nom);
						itm=itm->GetOwner();
					}
					nom.Normalize();
				}
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			scn->dat.GetElevationAndNormal_Recursion(itm,elv,nom,pos);
		}
	}
	return YSOK;
}

YSBOOL YsScenery::GetShellCollisionByBoundingBox(const YsVec3 &posOutside,const double &buff) const
{
	const YsListItem <YsSceneryShell> *shl;
	YsVec3 pos;

	pos=posOutside-this->pos;
	pos.RotateXZ(-att.h());
	pos.RotateZY(-att.p());
	pos.RotateXY(-att.b());

	if(YsCheckInsideBoundingBox3(pos,bbx[0],bbx[1])==YSTRUE)
	{
		shl=NULL;
		while((shl=FindNextShell(shl))!=NULL)
		{
			YsVec3 tst,bbx[2];

			tst=pos-shl->dat.pos;
			tst.RotateXZ(-shl->dat.att.h());
			tst.RotateZY(-shl->dat.att.p());
			tst.RotateXY(-shl->dat.att.b());

			shl->dat.GetBoundingBox(bbx);
			bbx[0]-=YsXYZ()*buff;
			bbx[1]+=YsXYZ()*buff;

			if(YsCheckInsideBoundingBox3(tst,bbx[0],bbx[1])==YSTRUE)
			{
				return YSTRUE;
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			if(scn->dat.GetShellCollisionByBoundingBox(pos,buff)==YSTRUE)
			{
				return YSTRUE;
			}
		}
	}

	return YSFALSE;
}

const YsSceneryShell *YsScenery::CheckShellCollision(const YsShell &shl1,const YsMatrix4x4 &modelTfm) const
{
	YsMatrix4x4 fieldTfm;
	fieldTfm.MultiplyInverse(this->pos,this->att);
	fieldTfm*=modelTfm;

	YsVec3 bbx1[2];
	shl1.GetBoundingBox(bbx1);
	{
		const YsVec3 bbx1Corner[8]=
		{
			fieldTfm*bbx1[0],
			fieldTfm*YsVec3(bbx1[1].x(),bbx1[0].x(),bbx1[0].x()),
			fieldTfm*YsVec3(bbx1[1].x(),bbx1[1].x(),bbx1[0].x()),
			fieldTfm*YsVec3(bbx1[0].x(),bbx1[1].x(),bbx1[0].x()),

			fieldTfm*YsVec3(bbx1[0].x(),bbx1[0].x(),bbx1[1].x()),
			fieldTfm*YsVec3(bbx1[1].x(),bbx1[0].x(),bbx1[1].x()),
			fieldTfm*bbx1[1],
			fieldTfm*YsVec3(bbx1[0].x(),bbx1[1].x(),bbx1[1].x()),
		};
		if(YSTRUE==YsCheckOutsideBoundingBox(this->bbx,8,bbx1Corner))
		{
			return nullptr;
		}
	}

	{
		const YsListItem <YsSceneryShell> *shl2=NULL;
		while((shl2=FindNextShell(shl2))!=NULL)
		{
			YsMatrix4x4 overallTfm;
			overallTfm.MultiplyInverse(shl2->dat.pos,shl2->dat.att);
			overallTfm*=fieldTfm;

			shl1.GetBoundingBox(bbx1);
			const YsVec3 bbx1Corner[8]=
			{
				overallTfm*bbx1[0],
				overallTfm*YsVec3(bbx1[1].x(),bbx1[0].x(),bbx1[0].x()),
				overallTfm*YsVec3(bbx1[1].x(),bbx1[1].x(),bbx1[0].x()),
				overallTfm*YsVec3(bbx1[0].x(),bbx1[1].x(),bbx1[0].x()),

				overallTfm*YsVec3(bbx1[0].x(),bbx1[0].x(),bbx1[1].x()),
				overallTfm*YsVec3(bbx1[1].x(),bbx1[0].x(),bbx1[1].x()),
				overallTfm*bbx1[1],
				overallTfm*YsVec3(bbx1[0].x(),bbx1[1].x(),bbx1[1].x()),
			};

			YsVec3 shl2Bbx[2];
			shl2->dat.GetCollisionShell().GetBoundingBox(shl2Bbx);
			if(YSTRUE==YsCheckOutsideBoundingBox(shl2Bbx,8,bbx1Corner))
			{
				continue;
			}

			YsCollisionOfPolygon collCheck;
			for(auto plHd : shl1.AllPolygon())
			{
				auto plVtPos1=shl1.GetPolygonVertexPosition(plHd);
				for(auto &p : plVtPos1)
				{
					p=overallTfm*p;
				}
				collCheck.SetPolygon1(plVtPos1.size(),plVtPos1.data());
				for(auto plHd2 : shl2->dat.collLtc.GetPolygonIntersectionCandidate(plVtPos1.size(),plVtPos1.data()))
				{
					// Check for real intersection.
					auto plVtPos2=shl2->dat.GetCollisionShell().GetPolygonVertexPosition(plHd2);
					collCheck.SetPolygon2(plVtPos2.size(),plVtPos2.data());
					if(YSTRUE==collCheck.CheckCollision())
					{
						return &shl2->dat;
					}
				}
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			auto collPtr=scn->dat.CheckShellCollision(shl1,fieldTfm);
			if(nullptr!=collPtr)
			{
				return collPtr;
			}
		}
	}

	return nullptr;
}

const YsSceneryShell *YsScenery::CheckShellCollision(YsVec3 &itscPos,const YsVec3 &p1,const YsVec3 &p2,const YsMatrix4x4 &fieldTfmIn) const
{
	YsMatrix4x4 fieldTfm;
	fieldTfm.MultiplyInverse(this->pos,this->att);
	fieldTfm*=fieldTfmIn;

	const YsVec3 lineTfm[2]=
	{
		fieldTfm*p1,
		fieldTfm*p2
	};
	if(YSTRUE==YsCheckOutsideBoundingBox(this->bbx,2,lineTfm))
	{
		return nullptr;
	}

	{
		const YsListItem <YsSceneryShell> *shl2=NULL;
		while((shl2=FindNextShell(shl2))!=NULL)
		{
			YsMatrix4x4 overallTfm;
			overallTfm.MultiplyInverse(shl2->dat.pos,shl2->dat.att);
			overallTfm*=fieldTfm;

			const YsVec3 lineTfm[2]=
			{
				overallTfm*p1,
				overallTfm*p2
			};
			YsVec3 shl2Bbx[2];
			shl2->dat.GetCollisionShell().GetBoundingBox(shl2Bbx);
			if(YSTRUE==YsCheckOutsideBoundingBox(shl2Bbx,2,lineTfm))
			{
				continue;
			}

			YsArray <YsShellPolygonHandle,16> allItscPlHd;
			YsArray <YsVec3,16> allItscPos;
			if(YSOK==shl2->dat.collLtc.ShootFiniteRay(allItscPos,allItscPlHd,lineTfm[0],lineTfm[1]) &&
			   0<allItscPos.size())
			{
				overallTfm.MulInverse(itscPos,allItscPos[0],1.0);
				return &shl2->dat;
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			auto collPtr=scn->dat.CheckShellCollision(itscPos,p1,p2,fieldTfm);
			if(nullptr!=collPtr)
			{
				return collPtr;
			}
		}
	}

	return nullptr;
}

const char *YsScenery::GetIdName(void) const
{
	return idName.Txt();
}

const YsColor &YsScenery::GetGroundColor(void) const
{
	return gndColor;
}

const YsColor &YsScenery::GetSkyColor(void) const
{
	return skyColor;
}

void YsScenery::SetSpecular(YSBOOL s)
{
	gndSpecular=s;
}

YSBOOL YsScenery::GetSpecular(void) const
{
	return gndSpecular;
}

YSSCNAREATYPE YsScenery::GetDefaultAreaType(void) const
{
	return areaType;
}

const double &YsScenery::GetBaseElevation(void) const
{
	return baseElevation;
}

const double &YsScenery::GetMagneticVariation(void) const
{
	return magneticVariation;
}

YSBOOL YsScenery::GetCanResume(void) const
{
	return canResume;
}

YSBOOL YsScenery::GetCanContinue(void) const
{
	return canContinue;
}

void YsScenery::MakeListOfMap(YsArray <YsScenery2DDrawing *,16> &drwLst) const
{
	YsArray <const YsScenery *> todo;

	drwLst.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsScenery2DDrawing> *drw;
		drw=NULL;
		while((drw=todo[0]->mapList.FindNext(drw))!=NULL)
		{
			drwLst.Append(&drw->dat);
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}
}

void YsScenery::MakeListOfGndObj(YsArray <YsSceneryGndObj *> & gobLst) const
{
	YsArray <const YsScenery *> todo;

	gobLst.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsSceneryGndObj> *gob;
		gob=NULL;
		while((gob=todo[0]->gndList.FindNext(gob))!=NULL)
		{
			gobLst.Append(&gob->dat);
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}
}

void YsScenery::MakeListOfAir(YsArray <YsSceneryAir *> &airList) const
{
	YsArray <const YsScenery *> todo;

	airList.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsSceneryAir> *air;
		air=NULL;

		while((air=todo[0]->airList.FindNext(air))!=NULL)
		{
			airList.Append(&air->dat);
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}
}

void YsScenery::MakeListOfPointSet(YsArray <YsSceneryPointSet *> & pstLst) const
{
	YsArray <const YsScenery *> todo;

	pstLst.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsSceneryPointSet> *pst;
		pst=NULL;
		while((pst=todo[0]->pstList.FindNext(pst))!=NULL)
		{
			pstLst.Append(&pst->dat);
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}
}

const YsSceneryGndObj *YsScenery::SearchGndObjByTag(const char tag[]) const
{
	YsListItem <YsSceneryGndObj> *gob;
	gob=NULL;
	while((gob=gndList.FindNext(gob))!=NULL)
	{
		if(strcmp(gob->dat.GetTag(),tag)==0)
		{
			return &gob->dat;
		}
	}

	const YsListItem <YsScenery> *scn;
	scn=NULL;
	while((scn=scnList.FindNext(scn))!=NULL)
	{
		const YsSceneryGndObj *gndObj;
		gndObj=scn->dat.SearchGndObjByTag(tag);
		if(gndObj!=NULL)
		{
			return gndObj;
		}
	}

	return NULL;
}

YsListItem <YsSceneryShell> *YsScenery::FindNextShell(const YsListItem <YsSceneryShell> *ptr)
{
	return shlList.FindNext(ptr);
}

YsListItem <YsSceneryElevationGrid> *YsScenery::FindNextElevationGrid(const YsListItem <YsSceneryElevationGrid> *ptr)
{
	return evgList.FindNext(ptr);
}

YsListItem <YsScenery2DDrawing> *YsScenery::FindNextMap(const YsListItem <YsScenery2DDrawing> *ptr)
{
	return mapList.FindNext(ptr);
}

YsListItem <YsScenery2DDrawing> *YsScenery::FindNextSignBoard(const YsListItem <YsScenery2DDrawing> *ptr)
{
	return sbdList.FindNext(ptr);
}

YsListItem <YsSceneryRectRegion> *YsScenery::FindNextRectRegion(const YsListItem <YsSceneryRectRegion> *ptr)
{
	return rgnList.FindNext(ptr);
}

YsListItem <YsScenery> *YsScenery::FindNextChildScenery(const YsListItem <YsScenery> *ptr)
{
	return scnList.FindNext(ptr);
}

YsListItem <YsSceneryGndObj> *YsScenery::FindNextGndObj(const YsListItem <YsSceneryGndObj> *ptr)
{
	return gndList.FindNext(ptr);
}

YsListItem <YsSceneryAir> *YsScenery::FindNextAir(const YsListItem <YsSceneryAir> *ptr)
{
	return airList.FindNext(ptr);
}

YsListItem <YsSceneryPointSet> *YsScenery::FindNextPointSet(const YsListItem <YsSceneryPointSet> *ptr)
{
	return pstList.FindNext(ptr);
}


YsListItem <YsSceneryShell> *YsScenery::FindPrevShell(const YsListItem <YsSceneryShell> *ptr)
{
	return shlList.FindPrev(ptr);
}

YsListItem <YsSceneryElevationGrid> *YsScenery::FindPrevElevationGrid(const YsListItem <YsSceneryElevationGrid> *ptr)
{
	return evgList.FindPrev(ptr);
}

YsListItem <YsScenery2DDrawing> *YsScenery::FindPrevMap(const YsListItem <YsScenery2DDrawing> *ptr)
{
	return mapList.FindPrev(ptr);
}

YsListItem <YsScenery2DDrawing> *YsScenery::FindPrevSignBoard(const YsListItem <YsScenery2DDrawing> *ptr)
{
	return sbdList.FindPrev(ptr);
}

YsListItem <YsSceneryRectRegion> *YsScenery::FindPrevRectRegion(const YsListItem <YsSceneryRectRegion> *ptr)
{
	return rgnList.FindPrev(ptr);
}

YsListItem <YsScenery> *YsScenery::FindPrevChildScenery(const YsListItem <YsScenery> *ptr)
{
	return scnList.FindPrev(ptr);
}

YsListItem <YsSceneryGndObj> *YsScenery::FindPrevGndObj(const YsListItem <YsSceneryGndObj> *ptr)
{
	return gndList.FindPrev(ptr);
}

YsListItem <YsSceneryAir> *YsScenery::FindPrevAir(const YsListItem <YsSceneryAir> *ptr)
{
	return airList.FindPrev(ptr);
}

YsListItem <YsSceneryPointSet> *YsScenery::FindPrevPointSet(const YsListItem <YsSceneryPointSet> *ptr)
{
	return pstList.FindPrev(ptr);
}


////////////////////////////////////////////////////////////

const YsListItem <YsSceneryShell> *YsScenery::FindNextShell(const YsListItem <YsSceneryShell> *ptr) const
{
	return shlList.FindNext(ptr);
}

const YsListItem <YsSceneryElevationGrid> *YsScenery::FindNextElevationGrid(const YsListItem <YsSceneryElevationGrid> *ptr) const
{
	return evgList.FindNext(ptr);
}

const YsListItem <YsScenery2DDrawing> *YsScenery::FindNextMap(const YsListItem <YsScenery2DDrawing> *ptr) const
{
	return mapList.FindNext(ptr);
}

const YsListItem <YsScenery2DDrawing> *YsScenery::FindNextSignBoard(const YsListItem <YsScenery2DDrawing> *ptr) const
{
	return sbdList.FindNext(ptr);
}

const YsListItem <YsSceneryRectRegion> *YsScenery::FindNextRectRegion(const YsListItem <YsSceneryRectRegion> *ptr) const
{
	return rgnList.FindNext(ptr);
}

const YsListItem <YsScenery> *YsScenery::FindNextChildScenery(const YsListItem <YsScenery> *ptr) const
{
	return scnList.FindNext(ptr);
}

const YsListItem <YsSceneryGndObj> *YsScenery::FindNextGndObj(const YsListItem <YsSceneryGndObj> *ptr) const
{
	return gndList.FindNext(ptr);
}

const YsListItem <YsSceneryAir> *YsScenery::FindNextAir(const YsListItem <YsSceneryAir> *ptr) const
{
	return airList.FindNext(ptr);
}

const YsListItem <YsSceneryPointSet> *YsScenery::FindNextPointSet(const YsListItem <YsSceneryPointSet> *ptr) const
{
	return pstList.FindNext(ptr);
}


const YsListItem <YsSceneryShell> *YsScenery::FindPrevShell(const YsListItem <YsSceneryShell> *ptr) const
{
	return shlList.FindPrev(ptr);
}

const YsListItem <YsSceneryElevationGrid> *YsScenery::FindPrevElevationGrid(const YsListItem <YsSceneryElevationGrid> *ptr) const
{
	return evgList.FindPrev(ptr);
}

const YsListItem <YsScenery2DDrawing> *YsScenery::FindPrevMap(const YsListItem <YsScenery2DDrawing> *ptr) const
{
	return mapList.FindPrev(ptr);
}

const YsListItem <YsScenery2DDrawing> *YsScenery::FindPrevSignBoard(const YsListItem <YsScenery2DDrawing> *ptr) const
{
	return sbdList.FindPrev(ptr);
}

const YsListItem <YsSceneryRectRegion> *YsScenery::FindPrevRectRegion(const YsListItem <YsSceneryRectRegion> *ptr) const
{
	return rgnList.FindPrev(ptr);
}

const YsListItem <YsScenery> *YsScenery::FindPrevChildScenery(const YsListItem <YsScenery> *ptr) const
{
	return scnList.FindPrev(ptr);
}

const YsListItem <YsSceneryGndObj> *YsScenery::FindPrevGndObj(const YsListItem <YsSceneryGndObj> *ptr) const
{
	return gndList.FindPrev(ptr);
}

const YsListItem <YsSceneryAir> *YsScenery::FindPrevAir(const YsListItem <YsSceneryAir> *ptr) const
{
	return airList.FindPrev(ptr);
}

const YsListItem <YsSceneryPointSet> *YsScenery::FindPrevPointSet(const YsListItem <YsSceneryPointSet> *ptr) const
{
	return pstList.FindPrev(ptr);
}

////////////////////////////////////////////////////////////



YsListItem <YsSceneryShell> *YsScenery::CreateShell(void)
{
	YsListItem <YsSceneryShell> *newItem;
	newItem=shlList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::SHELL;
	return newItem;
}

YsListItem <YsSceneryElevationGrid> *YsScenery::CreateElevationGrid(void)
{
	YsListItem <YsSceneryElevationGrid> *newItem;
	newItem=evgList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::ELEVATIONGRID;
	newItem->dat.RecomputeBoundingBox();
	return newItem;
}

YsListItem <YsScenery2DDrawing> *YsScenery::CreateMap(void)
{
	YsListItem <YsScenery2DDrawing> *newItem;
	newItem=mapList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::MAP;
	newItem->dat.RecomputeBoundingBox();
	return newItem;
}

YsListItem <YsScenery2DDrawing> *YsScenery::CreateSignBoard(void)
{
	YsListItem <YsScenery2DDrawing> *newItem;
	newItem=sbdList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::SIGNBOARD;
	newItem->dat.RecomputeBoundingBox();
	return newItem;
}

YsListItem <YsSceneryRectRegion> *YsScenery::CreateRectRegion(void)
{
	YsListItem <YsSceneryRectRegion> *newItem;
	newItem=rgnList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::RECTREGION;
	return newItem;
}

YsListItem <YsScenery> *YsScenery::CreateChildScenery(void)
{
	YsListItem <YsScenery> *newItem;
	newItem=scnList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::SUBSCENARY;
	newItem->dat.RecomputeBoundingBox();
	return newItem;
}

YsListItem <YsSceneryGndObj> *YsScenery::CreateGndObj(void)
{
	YsListItem <YsSceneryGndObj> *newItem;
	newItem=gndList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::GROUNDOBJECT;
	return newItem;
}

YsListItem <YsSceneryAir> *YsScenery::CreateAir(void)
{
	YsListItem <YsSceneryAir> *newItem;
	newItem=airList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::AIRCRAFT;
	return newItem;
}

YsListItem <YsSceneryPointSet> *YsScenery::CreatePointSet(void)
{
	YsListItem <YsSceneryPointSet> *newItem;
	newItem=pstList.Create();
	newItem->dat.Initialize();
	newItem->dat.thisInTheList=newItem;
	newItem->dat.searchKey=YsScenerySearchKeySeed++;
	newItem->dat.owner=this;
	newItem->dat.objType=YsSceneryItem::POINTSET;
	return newItem;
}

YsSceneryAirRouteHandle YsScenery::CreateAirRoute(void)
{
	YsSceneryAirRouteHandle newArHd=airRouteList.Create();
	if(NULL!=newArHd)
	{
		YsSceneryAirRoute *airRoute=GetAirRoute(newArHd);
		airRoute->Initialize();
		return newArHd;
	}
	return NULL;
}

YSRESULT YsScenery::FindNextAirRoute(YsSceneryAirRouteHandle &arHd) const
{
	return airRouteList.MoveToNext(arHd);
}

YSRESULT YsScenery::FindPrevAirRoute(YsSceneryAirRouteHandle &arHd) const
{
	return airRouteList.MoveToPrev(arHd);
}

const YsSceneryAirRoute *YsScenery::GetAirRoute(YsSceneryAirRouteHandle arHd) const
{
	return airRouteList[arHd];
}

YsSceneryAirRoute *YsScenery::GetAirRoute(YsSceneryAirRouteHandle arHd)
{
	return airRouteList[arHd];
}

const YsSceneryAirRoute *YsScenery::FindAirRouteByTag(const char tag[]) const
{
	const YsListItem <YsScenery> *subScn=NULL;
	while(NULL!=(subScn=FindNextChildScenery(subScn)))
	{
		const YsSceneryAirRoute *airRoute=subScn->dat.FindAirRouteByTag(tag);
		if(NULL!=airRoute)
		{
			return airRoute;
		}
	}

	YsSceneryAirRouteHandle arHd=NULL;
	while(YSOK==FindNextAirRoute(arHd))
	{
		const YsSceneryAirRoute *airRoute=GetAirRoute(arHd);
		if(0==strcmp(tag,airRoute->label))
		{
			return airRoute;
		}
	}

	return NULL;
}

void YsScenery::FindAllAirRoute(YsArray <const YsSceneryAirRoute *> &airRouteArray) const
{
	airRouteArray.Clear();
	FindAllAirRoute_Recursion(airRouteArray);
}

void YsScenery::FindAllAirRoute_Recursion(YsArray <const YsSceneryAirRoute *> &airRouteArray) const
{
	const YsListItem <YsScenery> *subScn=NULL;
	while(NULL!=(subScn=FindNextChildScenery(subScn)))
	{
		subScn->dat.FindAllAirRoute_Recursion(airRouteArray);
	}

	YsSceneryAirRouteHandle arHd=NULL;
	while(YSOK==FindNextAirRoute(arHd))
	{
		airRouteArray.Append(GetAirRoute(arHd));
	}
}

void YsScenery::FindAllSceneryAndAirRoutePair(YsArray <YsSceneryAndAirRoutePair> &scnRoutePairArray) const
{
	scnRoutePairArray.Clear();
	FindAllSceneryAndAirRoutePair_Recursion(scnRoutePairArray);
}

void YsScenery::FindAllSceneryAndAirRoutePair_Recursion(YsArray <YsSceneryAndAirRoutePair> &scnRoutePairArray) const
{
	const YsListItem <YsScenery> *subScn=NULL;
	while(NULL!=(subScn=FindNextChildScenery(subScn)))
	{
		subScn->dat.FindAllSceneryAndAirRoutePair_Recursion(scnRoutePairArray);
	}

	YsSceneryAirRouteHandle arHd=NULL;
	while(YSOK==FindNextAirRoute(arHd))
	{
		scnRoutePairArray.Increment();
		scnRoutePairArray.GetEnd().scn=this;
		scnRoutePairArray.GetEnd().route=*GetAirRoute(arHd);
	}
}

void YsScenery::ClearAllAirRoute(void)
{
	YsListItem <YsScenery> *subScn=NULL;
	while(NULL!=(subScn=FindNextChildScenery(subScn)))
	{
		subScn->dat.ClearAllAirRoute();
	}

	airRouteList.CleanUp();
}

void YsScenery::ApplySceneryAndAirRoutePair(int nRoute,const YsSceneryAndAirRoutePair route[])
{
	ClearAllAirRoute();

	for(int routeIdx=0; routeIdx<nRoute; ++routeIdx)
	{
		YsScenery *scn=(YsScenery *)(route[routeIdx].scn);
		scn->airRouteList.Append(route[routeIdx].route);
	}
}
