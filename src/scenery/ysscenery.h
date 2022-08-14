#ifndef YSSCENARY_IS_INCLUDED
#define YSSCENARY_IS_INCLUDED
/* { */

#include <ysbase64.h>

#include <ysvisual.h>

#include <yseditarray.h> // Not yet fully integrated in YSCLASS lib.
#include <fsdef.h>
#include <ystexturemanager.h>

// Free attribute 0
#define YSSHL_VTX_ROUND           1

#define YSSHL_PLG_NOSHADING       1
#define YSSHL_PLG_CONCAVE         2


#define YSSCN_VERSION 20150228


typedef enum
{
	YSSCNAREA_NOAREA,
	YSSCNAREA_WATER,
	YSSCNAREA_LAND,

YSSCNAREA_UNSELECTED=-1
} YSSCNAREATYPE;



////////////////////////////////////////////////////////////

class YsTextFile
{
public:
	YsString fName;
	YsTextFile();
	~YsTextFile();

	const YsListContainer <YsString> &GetText(void) const;
	YsListItem <YsString> *CreateNewLine(void);
	void Initialize(void);

protected:
	YsListContainer <YsString> text;
	YsListAllocator <YsString> textAllocator;
};

////////////////////////////////////////////////////////////

class YsSceneryTexturable
{
public:
	YsString texLabel;

	mutable YSBOOL texLabelNotFound;
	mutable YsTextureManager *texManCache;
	mutable YsTextureManager::TexHandle texHdCache;

	YSRESULT TryCacheTexture(class YsScenery *owner) const;
};

////////////////////////////////////////////////////////////

class Ys2DDrawingElement : public YsSceneryTexturable
{
friend class YsScenery;
friend class SeScenery;
friend class Ys2DDrawing;
friend class YsScenery2DDrawing;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;
friend class SeUndoSetDrawElemTexture;
friend class SeUndosetDrawElemTextureCoord;

public:
	enum OBJTYPE
	{
		POINTS,
		LINESEGMENTS,
		LINES,
		POLYGON,
		APPROACHLIGHT,
		GRADATIONQUADSTRIP,
		QUADSTRIP,
		QUADS,
		TRIANGLES
	};

protected:
	OBJTYPE t;
	double visibleDist;
	YsColor c,c2;
	YSBOOL cvx,specular;
	YsArray <YsVec2> pnt,texCoord;
	YsVec2 bbx[2];

	unsigned searchKey;
	class Ys2DDrawing *owner;
	YsListItem <Ys2DDrawingElement> *thisInTheList;

public:
	OBJTYPE GetElemType(void) const;
	void SetElemType(OBJTYPE t);
	const YsColor &GetColor(void) const;
	const YsColor &GetSecondColor(void) const;
	const double &VisibleDist(void) const;
	const YsArray <YsVec2> &GetPointList(void) const;
	const YsArray <YsVec2> &GetTexCoordArray(void) const;
	const YSBOOL IsConvex(void) const;

	void SetPointList(const YsArray <YsVec2> &newPntArray);
	void SetTexCoordArray(const YsArray <YsVec2> &newTexCoordArray);

	const YsTextureManager::Unit *GetTextureUnit(void) const;

	void SetVisibleDist(const double dist);

	void SetSpecular(YSBOOL s);
	YSBOOL GetSpecular(void) const;

public:
	void Initialize(void);
};

class Ys2DDrawing
{
friend class YsScenery;
friend class SeScenery;
friend class YsScenery2DDrawing;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;

public:
	class YsScenery2DDrawing *owner;

	YsVec2 bbx[2];

	YsListContainer <Ys2DDrawingElement> elemList,frozenList;
	static YsListAllocator <Ys2DDrawingElement> elemAllocator;

protected:
	static const char *const state0KeyWordSource[];
	static YsKeyWordList state0KeyWordList;
	static const char *const state1KeyWordSource[];
	static YsKeyWordList state1KeyWordList;

public:
	Ys2DDrawing();
	~Ys2DDrawing();
	void Initialize(void);
private:
	mutable class Ys2DDrawingGraphicCache *graphicCache;
	void AllocCache(void) const;
	void DeleteCache(void) const;
	YSBOOL IsCached(void) const;
	void MakeCache(const double &plgColScale,const double &linColorScale,const double &pntColorScale,YSBOOL mapMode);

public:
	void RecomputeBoundingBox(void);
	YsListItem <Ys2DDrawingElement> *CreateElement(Ys2DDrawingElement::OBJTYPE t);
	YSRESULT DeleteElement(YsListItem <Ys2DDrawingElement> *toDel);
	YSRESULT FreezeElement(YsListItem <Ys2DDrawingElement> *toFreeze);
	YSRESULT MeltElement(YsListItem <Ys2DDrawingElement> *toMelt);
	YSRESULT DeleteFrozenElement(void);
	YSBOOL IsFrozen(YsListItem <Ys2DDrawingElement> *elem);
	YSRESULT TriangulateAll(void);
	YSRESULT Triangulate(YsShell2dTessellator &tess,Ys2DDrawingElement *elm);

	int loadingState; // 0:outside object  1:inside PCK  2-11:inside obj  8:end of file  100:ATC  101:AIRSPACE  102:STP
	YsListItem <Ys2DDrawingElement> *loadingElem;

	YSRESULT BeginLoadPc2(void);
	YSRESULT LoadPc2OneLine(const char cmd[]);
	YSRESULT EndLoadPc2(void);

	YSRESULT LoadPc2(const char fn[]);
	YSRESULT LoadPc2(YsTextInputStream &inStream);
	YSRESULT LoadPc2(const YsTextFile &txtFile);
	YSRESULT SavePc2(const char fn[]) const;
	YSRESULT SavePc2(YsTextOutputStream &textOut) const;
	void Draw
	    (const double &plgColScale,const double &linColorScale,const double &pntColorScale,
	     YSBOOL drawPset,
	     YSBOOL mapMode,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL name2DElem,
	     const double &currentTime,
	     YsMatrix4x4 *viewModelTfm=NULL);  // viewModelTfm!=NULL -> check visiDist of each elem
	void DrawBoundingBox(YSBOOL mapMode);

	YSRESULT SetColor(Ys2DDrawingElement *elm,const YsColor &c);
	YSRESULT SetSecondColor(Ys2DDrawingElement *elm,const YsColor &c);
	YSRESULT SetVisibleDistance(Ys2DDrawingElement *elm,const double &dist);
	YSRESULT SetPointList(Ys2DDrawingElement *elm,const YsArray <YsVec2> &pnt);
	YSRESULT SetPointList(Ys2DDrawingElement *elm,int np,const YsVec2 pnt[]);

	YsListItem <Ys2DDrawingElement> *FindNextElem(const YsListItem <Ys2DDrawingElement> *ptr);
	YsListItem <Ys2DDrawingElement> *FindPrevElem(const YsListItem <Ys2DDrawingElement> *ptr);
	const YsListItem <Ys2DDrawingElement> *FindNextElem(const YsListItem <Ys2DDrawingElement> *ptr) const;
	const YsListItem <Ys2DDrawingElement> *FindPrevElem(const YsListItem <Ys2DDrawingElement> *ptr) const;

};

////////////////////////////////////////////////////////////

class YsElevationGridNode
{
public:
	double y;
	YSBOOL lup,visible[2],protectPolygon[2];
	YsColor c[2];
	YsVec3 nom[2],cen[2];
	YsVec3 nomOfNode;
};

class YsElvGridFaceId
{
public:
	int x,z,tri;

	inline bool operator==(const YsElvGridFaceId &b) const
	{
		return (this->x==b.x && this->z==b.z && this->tri==b.tri);
	}
	inline bool operator!=(const YsElvGridFaceId &b) const
	{
		return (this->x!=b.x || this->z!=b.z || this->tri!=b.tri);
	}
};


class YsElevationGrid : public YsSceneryTexturable
{
friend class YsScenery;
friend class SeScenery;
friend class YsSceneryElevationGrid;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;
friend class EditElvGridPropertyDialog;

private:
	const YsElevationGrid &operator=(const YsElevationGrid &from); // Prohibit copy operator

public:
	YsElevationGrid();
	~YsElevationGrid();

private:
	mutable class YsElevationGridGraphicCache *graphicCache;
	void AllocCache(void) const;
	void DeleteCache(void) const;
	YSBOOL IsCached(void) const;
	void MakeCache(const double &plgColScale,YSBOOL invert);

public:
	class YsSceneryElevationGrid *owner;

	int nx,nz;  // <- # of blocks (# of nodes is (nx+1)*(nz+1)
	double xWid,zWid;
	YsArray <YsElevationGridNode> node;
	YSBOOL sideWall[4];
	YsColor sideWallColor[4];
	YSBOOL hasProtectPolygon;

	YsVec3 bbx[2];


	YSBOOL colorByElevation;
	double colorByElevation_Elevation[2];
	YsColor colorByElevation_Color[2];
private:
	YSBOOL specular;

public:
	void Initialize(void);
	void Create(int nx,int ny,const double &xWid,const double &zWid,const YsColor &col);
	void SetTextureLabel(const YsString &texLabel);
	void RecomputeBoundingBox(void);
	void RecomputeNormal(void);
	YSRESULT RecomputeNormal(int x,int z);

	YSBOOL reachEnd;
	YSRESULT BeginLoadTer(void);
	YSRESULT LoadTerOneLine(const char str[]);
	YSRESULT EndLoadTer(void);

	YSRESULT LoadTer(const char fn[]);
	YSRESULT LoadTer(YsTextInputStream &inStream);
	YSRESULT LoadTer(const YsTextFile &txtFile);
	YSRESULT SaveTer(const char fn[]) const;
	YSRESULT SaveTer(YsTextOutputStream &textOut) const;

	YsVec2i GetNumBlock(void) const;
	YSRESULT GetGridPosition(YsVec3 &pos,int x,int z) const;
	YSRESULT GetTriangle(YsVec3 tri[3],int x,int z,int f) const;
	YSRESULT GetTriangleNormal(YsVec3 &nom,int x,int z,int f) const;
	YSRESULT GetTriangleNodeId(YsElvGridFaceId tri[3],int x,int z,int f) const;
	YSRESULT GetElevation(double &elv,int &ix,int &iz,int &f,const YsVec3 &pos) const;
	YSRESULT GetNodeListFromFaceList(YsArray <YsElvGridFaceId> &nodeId,int nFace,const YsElvGridFaceId fcId[]) const;

	const YsColor ColorByElevation(const double &y) const;
	const YsColor GetTriangleColor(int x,int z,int f) const;// <= If ColorByElevation is true, it will return average.
	YSRESULT GetNeighborTriangle(int &neix,int &neiz,int &neif,int x,int z,int f,int side) const;
	unsigned int LinearizeFaceId(int x,int z,int f) const;
	YSRESULT UnlinearizeFaceId(int &x,int &z,int &f,unsigned int id) const;

	void SetSpecular(YSBOOL s);
	YSBOOL GetSpecular(void) const;


	void Draw
	    (const double &plgColScale,
	     YSBOOL invert,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL shrinkTriangle,
	     YSBOOL nameElvGridFace,YSBOOL nameElvGridNode);
	void DrawFastFillOnly(const double &plgColScale);
	void DrawProtectPolygon(void);
	void DrawClippedProtectPolygon(const YsVec3 &cameraPos,const YsPlane &clipPln,const YsPlane &nearPln,const YsVec3 &t0,const YsVec3 &t1,const YsVec3 &t2);
	void DrawProtectPolygonAccurate(const YsMatrix4x4 &viewMdlMat,const double &nearZ);
	void DrawBoundingBox(void);
};

////////////////////////////////////////////////////////////

class YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;
friend class FsField;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;
friend class SeUndoSetItemTag;
friend class SeUndoSetItemId;

public:
	YsSceneryItem();

	enum OBJTYPE
	{
		GENERIC,
		SHELL,
		ELEVATIONGRID,
		MAP,
		SIGNBOARD,
		RECTREGION,
		SUBSCENARY,
		GROUNDOBJECT,
		AIRCRAFT,
		POINTSET
	};

	OBJTYPE GetObjType(void) const;
	class YsScenery *GetOwner(void) const;
	static const char *GetObjTypeString(OBJTYPE t);
	const YsVec3 &GetPosition(void) const;
	const YsAtt3 &GetAttitude(void) const;
	unsigned GetSearchKey(void) const;

protected:
	void Initialize(void);
	YSRESULT WriteCommonData(YsTextOutputStream &textOut);

	unsigned searchKey;
	class YsScenery *owner;
	OBJTYPE objType;

	YsAtt3 att;
	YsVec3 pos;

	YsString fName;

	double visibleDist;
	int id;
	YsString tagStr;

public:
	virtual void GetBoundingBox(YsVec3 bbx[2]) const;

	int GetId(void) const;
	const char *GetTag(void) const;

	class YsScenery2DDrawing *Get2DDrawing(void);
	const class YsScenery2DDrawing *Get2DDrawing(void) const;
	class YsSceneryElevationGrid *GetElevationGrid(void);
	const class YsSceneryElevationGrid *GetElevationGrid(void) const;
	class YsSceneryRectRegion *GetRectRegion(void);
	const class YsSceneryRectRegion *GetRectRegion(void) const;
	class YsSceneryGndObj *GetGndObj(void);
	const class YsSceneryGndObj *GetGndObj(void) const;
	class YsSceneryAir *GetAirObj(void);
	const class YsSceneryAir *GetAirObj(void) const;
	class YsSceneryPointSet *GetPointSet(void);
	const class YsSceneryPointSet *GetPointSet(void) const;
};

class YsSceneryShell : public YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;

protected:
	YsString collFName; // For serialization only.

	YsVisualSrf shl,collShl;
	mutable YsShellLattice collLtc;

	void Initialize(void);
	YsListItem <YsSceneryShell> *thisInTheList;
	void Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx);
	YSRESULT Save(const char fn[]) const;
	YSRESULT Save(YsTextOutputStream &textOut) const;
	YSRESULT LoadSrf(const YsTextFile &txtFile);

	YSRESULT SaveCollSrf(YsTextOutputStream &textOut) const;
	YSRESULT LoadCollSrf(const YsTextFile &txtFile);

	YSRESULT CacheCollLattice(void) const;

public:
	const YsVisualSrf &GetCollisionShell(void) const;
	virtual void GetBoundingBox(YsVec3 bbx[2]) const;
};

class YsSceneryElevationGrid : public YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;
friend class SeUndoSetElevationGridTexture;
friend class SeUndoSetElevationGridSpecular;

friend class EditElvGridPropertyDialog;

protected:
	void Initialize(void);
	YsListItem <YsSceneryElevationGrid> *thisInTheList;
	YsElevationGrid evg;

public:
	YsSceneryElevationGrid();

	YSRESULT Save(const char fn[]) const;
	YSRESULT Save(YsTextOutputStream &textOut) const;
	void GetSideWallConfiguration(YSBOOL sw[4],YsColor swc[4]) const;
	virtual void GetBoundingBox(YsVec3 bbx[2]) const;
	void RecomputeBoundingBox(void);
	void RecomputeNormal(void);
	YsVec2i GetNumBlock(void) const;
	YSRESULT GetNumBlock(int &nx,int &nz) const;
	YsVec3 GetNodePosition(const YsVec2i &idx) const;
	YSRESULT GetNodePosition(YsVec3 &pos,int x,int z) const;
	const double &GetNodeElevation(int x,int z) const;
	YSRESULT GetTriangle(YsVec3 tr[3],int x,int z,int f) const;
	YSRESULT GetTriangleNormal(YsVec3 &nom,int x,int z,int f) const;
	YSRESULT GetTriangleNodeId(YsElvGridFaceId tr[3],int x,int z,int f) const;
	YSBOOL IsTriangleVisible(int x,int z,int f) const;
	YSBOOL IsTriangleProtectPolygon(int x,int z,int f) const;
	YSBOOL GetTriangulation(int x,int z) const;
	YSBOOL GetColorByElevation(double elv[2],YsColor col[2])  const;
	const YsColor GetTriangleColor(int x,int z,int f) const; // <= If ColorByElevation is true, it will return average
	YSRESULT GetNeighborTriangle(int &neix,int &neiz,int &neif,int x,int z,int f,int side) const;
	unsigned int LinearizeFaceId(int x,int z,int f) const;
	YSRESULT UnlinearlizeFaceId(int &x,int &z,int &f,unsigned int id) const;
	YSRESULT GetNodeListFromFaceList(YsArray <YsElvGridFaceId> &nodeId,int nFace,const YsElvGridFaceId fcId[]) const;
};

class YsScenery2DDrawing : public YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;

protected:
	void Initialize(void);
	YsListItem <YsScenery2DDrawing> *thisInTheList;
	Ys2DDrawing drw;

public:
	YSRESULT Save(const char fn[]) const;
	YSRESULT Save(YsTextOutputStream &textOut) const;
	virtual void GetBoundingBox(YsVec3 bbx[2]) const;
	void RecomputeBoundingBox(void);
	const Ys2DDrawing &GetDrawing(void) const;
	Ys2DDrawing &GetDrawing(void);
};

class YsSceneryRectRegion : public YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyRectRegion;

public:
	enum SUBCLASSTYPE
	{
		SUBCLASS_NONE,
		SUBCLASS_DEADLOCK_FREE_AIRPORT
	};


protected:
	void Initialize(void);
	YsListItem <YsSceneryRectRegion> *thisInTheList;
	void Draw(void);
	YsVec2 min,max;

	SUBCLASSTYPE subClassType;

public:
	virtual void GetBoundingBox(YsVec3 bbx[2]) const;
	void GetMinMax(YsVec2 &min,YsVec2 &max) const;
	void GetMinMax(YsVec3 &min,YsVec3 &max) const;

	SUBCLASSTYPE GetSubClassType(void) const;
	void SetSubClassType(SUBCLASSTYPE subClassType);
	static const char *GetSubClassTypeString(SUBCLASSTYPE subClassType);
	static SUBCLASSTYPE GetSubClassTypeFromString(const char str[]);
};

class YsSceneryObjTemplate
{
private:
	YsString identName;
	YsString dnmFn;
	mutable YSBOOL dnmReady;
	mutable YsVisualDnm dnm;

	YsSceneryObjTemplate(const YsSceneryObjTemplate &);
	YsSceneryObjTemplate &operator=(const YsSceneryObjTemplate &);

public:
	YsSceneryObjTemplate();

	void Initialize(void);

	const char *ObjName(void) const;
	void SetObjName(const char nm[]);

	void SetDnmFileName(const char fn[]);
	YsVisualDnm &GetDnm(void) const;
};

class YsSceneryGndObjTemplate : public YsSceneryObjTemplate
{
public:
	void Initialize(void);

	YsVec3 ilsPos;
	YsAtt3 ilsAtt;
	double ilsRange;

	double vorRange,ndbRange;
	FSGROUNDTYPE gndType;
};


class YsSceneryAirTemplate : public YsSceneryObjTemplate
{
public:
	YsSceneryAirTemplate();
	void Initialize(void);
};


class YsSceneryGndObj : public YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyAir;
friend class SeUndoModifyRectRegion;

protected:
	void Initialize(void);
	void Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx);
	YsListItem <YsSceneryGndObj> *thisInTheList;

	YsSceneryGndObjTemplate *gndObjTemplate;
	YsString objName;
	int iff;
	YSBOOL primaryTarget;
	unsigned int gndFlag;

	YSBOOL motionPathOffset;
	YsString motionPathName;

public:
	const YsSceneryGndObjTemplate *GetLinkedObjTemplate(void) const;
	const char *GetObjName(void) const;
	const int GetIFF(void) const;
	const YSBOOL IsPrimaryTarget(void) const;
	unsigned int GetFlag(void) const;

	const char *GetMotionPathName(void) const;
	YSBOOL GetMotionPathOffset(void) const;

	virtual void GetBoundingBox(YsVec3 bbx[2]) const;
};


class YsSceneryAirAction
{
public:
	enum ACTIONTYPE
	{
		DONOTHING,
		LAND,
		CIRCLE,
		STRAIGHT,
		FLYAIRROUTE,
		CUSTOM
	};

	ACTIONTYPE actType;
	YsString actParam[1];
	YsArray <YsString> customIntention;

	void Initialize(void);
};

inline int operator==(const YsSceneryAirAction &act1,const YsSceneryAirAction &act2)
{
	if(act1.actType==act2.actType)
	{
		if(act1.actType==YsSceneryAirAction::FLYAIRROUTE && 0!=strcmp(act1.actParam[0],act2.actParam[0]))
		{
			return 0;
		}
		return 1;
	}
	return 0;
}

class YsSceneryAir : public YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyAir;
friend class SeUndoModifyRectRegion;

protected:
	YsListItem <YsSceneryAir> *thisInTheList;

	YsSceneryAirTemplate *airTemplate;
	YsString objName;
	int iff;
	YSBOOL ldg;
	YSBOOL isPlayer; // For importing and exporting mission YFS purpose only.
	unsigned int airFlag;
	int fuel;
	double speed;
	double landWhenLowFuel;
	YsSceneryAirAction action;
	YsArray <YsString> otherCommand;

public:
	void Initialize(void);
	void Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx);

	const int GetIFF(void) const;
	const char *GetObjName(void) const;
	YSBOOL GetLandingGear(void) const;
	unsigned int GetFlag(void) const;
	unsigned int GetFuel(void) const;
	const double &GetSpeed(void) const;
	const double &GetLandWhenLowFuel(void) const;

	const YsSceneryAirAction &GetAction(void) const;

	virtual void GetBoundingBox(YsVec3 bbx[2]) const;
};




class YsSceneryPointSet : public YsSceneryItem
{
friend class YsScenery;
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoSetMotionPathIsLoop;
friend class SeUndoSetMotionPathCoord;
friend class SeUndoChangeMotionPathAreaType;

protected:
	YsListItem <YsSceneryPointSet> *thisInTheList;

	YSBOOL isLoop;
	YSSCNAREATYPE areaType;
	YsArray <YsVec3> pnt;

	mutable YsShell2d *shl2d;
	mutable YsShell2dLattice *ltc;
	mutable YSBOOL shl2dReady;

	mutable YsVec3 bbxCache[2];
	mutable YSBOOL bbxValid;

public:
	YsSceneryPointSet();
	~YsSceneryPointSet();

protected:
	void Initialize(void);
	void Draw(void);
	void DrawStar(void);

public:
	int GetNumPoint(void) const;
	const YsVec3 *GetPointArray(void) const;
	const YsVec3 GetTransformedPoint(int idx) const;
	YSBOOL IsLoop(void) const;
	YSSCNAREATYPE GetAreaType(void) const;

	YSRESULT SetPointList(int np,const YsVec3 p[],YSBOOL isLoop);
	YSRESULT SetPointList(int np,const YsVec3 p[]);
	void SetAreaType(YSSCNAREATYPE areaType);

	void PrepareShell2d(void) const;
	YSSIDE CheckInsidePolygon(const double &x,const double &z) const;

	virtual void GetBoundingBox(YsVec3 bbx[2]) const;
};


////////////////////////////////////////////////////////////
// Non-drawable properties

class YsSceneryAirRoute  // Keep it copyable.  Need to make a copy in YsSceneryEditorAirRouteDialog
{
public:
	class RouteSegment
	{
	public:
		enum ROUTE_SEGMENT_TYPE
		{
			ROUTE_SEGMENT_NULL,
			ROUTE_SEGMENT_FIX,
			ROUTE_SEGMENT_VOR,
			ROUTE_SEGMENT_NDB,
			ROUTE_SEGMENT_AIRPORT,
			ROUTE_SEGMENT_CARRIER
		};

		ROUTE_SEGMENT_TYPE segType;
		YsString label;
		double altitude,beginApproachAt;
		double trafficPatternAlt;  // 2013/01/26 not used yet.
		double takeOffClimbAlt;
		YsArray <YsString> ilsArray;

		// Cache (Cached in FsAirRouteAutopilot::FetchAirRoute)
		mutable YsVec3 pos;
		mutable int gndObjKey;
		mutable YsArray <unsigned int> ilsKeyArray;
		mutable YsArray <const YsSceneryPointSet *> vfrApproachArray;

		void Initialize(void);
		void ClearCache(void);
	};

	YsString label;
	YsSegmentedArray <RouteSegment,4> routeSequence;

	YsSceneryAirRoute();
	void Initialize(void);
	int AddSequence(RouteSegment::ROUTE_SEGMENT_TYPE segType);
	void SetSequenceAltitude(int seqIdx,const double alt);
	void SetTakeOffClimbAltitude(int seqIdx,const double alt);
	void SetSequenceLabel(int seqIdx,const char label[]);
	void SetSequenceBeginApproachDist(int seqIdx,const double dist);
	void AddSequenceILS(int seqIdx,const char ilsName[]);
};

class YsSceneryAndAirRoutePair
{
public:
	const class YsScenery *scn;
	YsSceneryAirRoute route;
};

////////////////////////////////////////////////////////////


typedef YsEditArrayObjectHandle <YsSceneryAirRoute,2> YsSceneryAirRouteHandle;


class YsScenery : public YsSceneryItem
{
friend class SeScenery;

friend class SeUndo;
friend class SeUndoCreateItem;
friend class SeUndoDeleteItem;
friend class SeUndoChangeMapPriority;
friend class SeUndoMoveItemPosAtt;
friend class SeUndoSetItemOwner;
friend class SeUndoModifyElevationGridNode;
friend class SeUndoModifyElevationGridSideWall;
friend class SeUndoSetElevationGridColorByElevation;
friend class SeUndoCreate2DDrawElem;
friend class SeUndoDelete2DDrawElem;
friend class SeUndoSetDrawElemTypeColorVisidist;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyAir;
friend class SeUndoModifyRectRegion;
friend class SeUndoSetBaseElevation;
friend class SeUndoSetAirRoute;
friend class SeUndoSetCanResume;
friend class SeUndoSetCanContinue;

enum
{
	LOADINGSTATE_OUTSIDE=0,
	LOADINGSTATE_PCK=1,
	LOADINGSTATE_PC2=2,
	LOADINGSTATE_PLT=3,
	LOADINGSTATE_SRF=4,
	LOADINGSTATE_TER=5,
	LOADINGSTATE_RGN=6,
	LOADINGSTATE_FLD=7,
	LOADINGSTATE_ENDOFFILE=8,
	LOADINGSTATE_GOB=9,
	LOADINGSTATE_PST=10,
	LOADINGSTATE_AOB=11,
	LOADINGSTATE_ATC=100,
	LOADINGSTATE_AIRSPACE=101,
	LOADINGSTATE_STP=102,
	LOADINGSTATE_AIRROUTE=103,
	LOADINGSTATE_APPROACH=104,
	LOADINGSTATE_AIRPORT=105,
	LOADINGSTATE_WEATHER=106,
	LOADINGSTATE_SPECOBJ=107
};

public:
	class MapDrawingInfo
	{
	public:
		YsMatrix4x4 mapOwnerToWorldTfm;
		YsScenery2DDrawing *mapPtr;
	};
	class SamePlaneMapGroup
	{
	public:
		YsVec3 org,nom;
		YsArray <MapDrawingInfo> mapDrawingInfo;
		void Initialize(void)
		{
			org=YsVec3::Origin();
			nom=YsYVec();
			mapDrawingInfo.CleanUp();
		}
	};

	/*! This class is for organize all 2D-drawing maps based on the plane on which the maps are lying.
	    To draw maps correctly, maps needs to be drawn:
	    (1) Draw maps with depth-test on, depth-mask=0
	    (2) Draw maps with depth-test on, depth-mask=1
	    This needs to be done maps on the same-plane at a time.
	    Otherwise, the priority may be incorrect when lower-elevation map is drawn after higher-elevation map.
	    Therefore, this actually needs to be:
	    for-each-maps-grouped-by-the-plane:
	        Draw maps with depth-test on, depth-mask=0
	        Draw maps with depth-test on, depth-mask=1
	    To do this, MapDrawingOrder must be pre-calculated.  (Hopefully not while drawing.)

	    TESTCASE: NORTH_KYUSHU | KAMIGOTOH_RW35 -> Look left
	*/
	class MapDrawingOrder
	{
	public:
		YSBOOL cached;
		YsArray <SamePlaneMapGroup> samePlaneMapGroup;
		MapDrawingOrder()
		{
			CleanUp();
		}
		void CleanUp(void)
		{
			cached=YSFALSE;
			samePlaneMapGroup.CleanUp();
		}
	};

public:
	static int lightPointSizePix;
	static float lightPointSize3d;
	static unsigned globalControl_transparency;  // 0:No transparency  255:Use Trancparency
	static YSBOOL globalControl_antiAliasing;

	static YsTextureManager *commonTexManPtr;
	static YsTextureManager::TexHandle commonGroundTexHd;
	static YsTextureManager::TexHandle commonRunwayLightTexHd;


public:
enum ERRORCODE
{
	ERROR_NOERROR,
	ERROR_IO_FILE_NOT_FOUND,
	ERROR_IO_WRITE_ERROR,
	ERROR_IO_NEED_NEW_VERSION,
	ERROR_IO_INVALID_FILE,
};

	static const char *ErrorCodeToString(ERRORCODE err);


public:
	static const unsigned char groundTileTexture[16*16];

protected:
	static const char *const state0KeyWordSource[];
	static YsKeyWordList state0KeyWordList;
	static const char *const state2KeyWordSource[];
	static YsKeyWordList state2KeyWordList;
	static const char *const airActionKeyWordSource[];
	static YsKeyWordList airActionKeyWordList;
	static const char *const airRouteKeyWordSource[];
	static YsKeyWordList airRouteKeyWordList;

protected:
	// Geometric entities >>
	YsListContainer <YsSceneryShell> shlList;
	YsListContainer <YsSceneryElevationGrid> evgList;
	YsListContainer <YsScenery2DDrawing> mapList;
	YsListContainer <YsScenery2DDrawing> sbdList;
	YsListContainer <YsSceneryRectRegion> rgnList;
	YsListContainer <YsScenery> scnList;
	YsListContainer <YsSceneryGndObj> gndList;
	YsListContainer <YsSceneryAir> airList;
	YsListContainer <YsSceneryPointSet> pstList;

	static YsListAllocator <YsSceneryShell> shlAllocator;
	static YsListAllocator <YsSceneryElevationGrid> evgAllocator;
	static YsListAllocator <YsScenery2DDrawing> drwAllocator;
	static YsListAllocator <YsSceneryRectRegion> rgnAllocator;
	static YsListAllocator <YsScenery> scnAllocator;
	static YsListAllocator <YsSceneryGndObj> gndAllocator;
	static YsListAllocator <YsSceneryAir> airAllocator;
	static YsListAllocator <YsSceneryPointSet> pstAllocator;
	// Geometric entities <<

	// Non-geometric entities >>
	YsEditArray <YsSceneryAirRoute,2> airRouteList;
	// Non-geometric entities <<

private:
	mutable ERRORCODE lastError;
public:
	ERRORCODE GetLastError(void) const;
	void SetLastError(ERRORCODE err);

public:
	static int numSceneryDrawn;

	YsListItem <YsScenery> *thisInTheList;

	YsString idName;
	YsColor skyColor,gndColor;
private:
	YSBOOL gndSpecular;
public:
	/*! yfsHeader is for Mission YFS editing.  Populated in ImportYfs of SeScenery, and referenced in ExportYfs.  
	    Nowhere else should use this member. */
	YsArray <YsString> yfsHeader;

	YsVec3 bbx[2];
	double plgColorScale,linColorScale,pntColorScale;

	int loadingState; // 0:Outside
	                  // 1:Loading pck
	                  // 2:Loading map
	                  // 3:Loading sign board
	                  // 4:Loading srf
	                  // 5:Loading elevation grid
	                  // 6:Loading rect region
	                  // 7:Loading fld
	                  // 8:Reached end of file
	                  // 9:Loading YSFLIGHTGround Object
	                  //10:Loading Point Set
	                  //11:Loading Air Object 

	static YsListAllocator <YsTextFile> pckFileAllocator;
	YsListContainer <YsTextFile> pckFileList;
	int nPckFileRemain;
	YsString curPath;
	YsTextFile *currentPckFile;
	YsSceneryItem *currentItem;
	YsSceneryAirRoute *currentAirRoute;

	YSSCNAREATYPE areaType;
	double baseElevation;
	double magneticVariation;

	YSBOOL canResume,canContinue;

	YsArray <double> mapElevationCache;
private:
	MapDrawingOrder mapDrawingOrderCache;

public:
	// Error code from C standard library >>
	int lastErrorNo;
	YsString lastErrorStr;
	// Error code from C standard library <<

	YsTextureManager textureManager;


	YsScenery();
	~YsScenery();
	static void SelfDiagnostic(void);
	void DeleteList(YsScenery *scn);
	void Initialize(void);
	void ResetPosition(void);  // 2005/01/10
	static void CollectGarbage(void);
	void RecomputeBoundingBox(void);
	void UnlinkGndObjTemplate(void);
	void UnlinkAirTemplate(void);
	void LinkGndObjTemplate(const YsListContainer <YsSceneryGndObjTemplate> &gndObjTemplateList);
	void LinkAirTemplate(YsSceneryAir *air,const YsListContainer <YsSceneryAirTemplate> &airTemplateList);
	void LinkAirTemplate(const YsListContainer <YsSceneryAirTemplate> &airTemplateList);
	virtual void GetBoundingBox(YsVec3 bbx[2]) const;

	static void GlSetColor(const YsColor &col);
	static void GlSetColorMaterial(const YsColor &col);

	static YSSCNAREATYPE GetAreaTypeFromString(const char str[]);
	static const char *GetAreaTypeString(YSSCNAREATYPE areaType);

	void DrawBoundingBox(void);

	void Draw(
	    YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,
	    YSBOOL drawGndObj,YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn);
	void DrawThisSceneryOnly(
	    YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,
	    YSBOOL drawGndObj,YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn);
private:
	void DrawSingle
	   (YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,
	    YSBOOL drawGndObj,YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn);
public:
	void DrawMap(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx);
	void DrawMapThisSceneryOnly(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx);
private:
	void DrawMapSingle(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx);

public:
	void CalculateMapElevationCache(void);
protected:
	void CalculateMapElevationCache(YsArray <double> &cache,const YsMatrix4x4 &modelTfm);

public:
	void CacheMapDrawingOrder(const double tol=0.05);
	YSBOOL MapDrawingOrderCached(void) const;
	MapDrawingOrder MakeMapDrawingOrder(const YsMatrix4x4 &sceneryToWorldTfm,const double tol=0.05) const;
protected:
	void MakeMapDrawingOrder(MapDrawingOrder &mdo,const YsMatrix4x4 &sceneryToWorldTfm,const double tol) const;

public:
	void DrawProtectPolygon(const YsMatrix4x4 &modelTfm); // OpenGL Only for SceneryEdit

	void DrawProtectPolygon
	    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,const double nearZ,
	     const double &currentTime);

public:
	void DrawVisual(const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const YsMatrix4x4 &projTfm,
	     const double &currentTime,YSBOOL forShadowMap);
	void DrawMapVisual
	    (const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &sceneryToWorldTfm,const YsMatrix4x4 &projTfm,
	     const double &elvMin,const double &elvMax,YSBOOL drawPset,const double &currentTime);


	void DrawAxis(
	    const double &axsSize,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn);
	void DrawAxisThisSceneryOnly(
	    const double &axsSize,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn);
private:
	void DrawAxisSingle(
	    const double &axsSize,
	    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
	    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn);

public:
	void DrawItemAxis(const YsSceneryItem *itm,const double &axsSize);
	void DrawItem
	   (const YsSceneryItem *itm,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
	    YSBOOL name2DElem,YSBOOL nameElvGridFace,YSBOOL nameElvGridNode);
	void DrawItemStar(const YsSceneryItem *itm);
	void Draw2DDrawingElement
	   (YsScenery2DDrawing *drw,const Ys2DDrawingElement *toDraw,
	    YSBOOL nameVtId,YSBOOL wire,YSBOOL points);
	void DrawILSGuideline(void);
	void DrawItemILSGuideline(YsSceneryGndObj *gob);

	void SetColorScale(const double &plgScale,const double &linScale,const double &pntScale);


	YSRESULT BeginLoadFld(const char currentPath[]);
	YSRESULT LoadFldOneLine(const char str[]);
	YSRESULT EndLoadFld(void);

	YSRESULT LoadFld(const char fn[]);
	YSRESULT LoadFld(FILE *fp);
	YSRESULT LoadFld(const YsTextFile &txtFile);

	YSRESULT SaveFld(const char fn[]);
	YSRESULT SaveFld(YsTextOutputStream &textOut);
protected:
	YSRESULT AssignUniqueFilename(int &n);
	YSRESULT SaveSubScenery(YsTextOutputStream &textOut);
	YSRESULT MergePackedFile(YsTextOutputStream &textOut,const char packName[],const YsTextMemoryOutputStream &tmpFile);

public:
	YSRESULT ExportYfs(const char fn[]) const;

public:
	YSRESULT RecursivelyUpdateBoundingBox(const YsSceneryItem *itm);

	/*! Calculates a transformation from the item's local coordinate to the world coordinate.
	*/
	YSRESULT GetTransformation(YsMatrix4x4 &mat,const YsSceneryItem *itm) const;
	YsMatrix4x4 GetTransformation(const YsSceneryItem *itm) const;


	YSRESULT GetParentTransformation(YsMatrix4x4 &mat,const YsSceneryItem *itm) const;
	const YsMatrix4x4 &ComputeTransformation(YsMatrix4x4 &mat,const YsVec3 &pos,const YsAtt3 &att) const;
	YSRESULT GetOrigin(YsVec3 &pos,const YsSceneryItem *itm) const;
	YSRESULT GetTransformedBoundingBox(YsVec3 bbx[8],const YsSceneryItem *itm) const;
	YSRESULT GetTransformedBoundingBox(YsVec3 bbx[8],const YsSceneryItem *itm,const YsMatrix4x4 &tfm) const;
	YSBOOL IsItemVisible(
	    const YsMatrix4x4 &viewModelTfm,const YsMatrix4x4 &projTfm,
	    const YsSceneryItem *itm) const;

	void TransformBoundingBox(YsVec3 bbx[2],const YsVec3 &tra,const YsAtt3 &rot) const;

	template <const int N>
	YSRESULT SearchRegionById(YsArray <const YsSceneryRectRegion *,N> &rgnLst,int id) const;
	template <const int N>
	YSRESULT SearchRegionByTag(YsArray <const YsSceneryRectRegion *,N> &rgnLst,const char tag[]) const;
	YSRESULT GetRegionRect(YsVec3 rec[4],const YsSceneryRectRegion *rgn) const;
	YsVec3 GetRectRegionCenter(const YsSceneryRectRegion *rgn) const;

	YSBOOL IsInsideRectRegion(const YsVec3 &pos,const YsSceneryRectRegion *rgn) const;

	template <const int N>
	int GetRectRegionFromPoint(YsArray <const YsSceneryRectRegion *,N> &rgnList,const YsVec3 &pos) const;

	YSSCNAREATYPE GetAreaTypeFromPoint(const YsVec3 &pos) const;
	YSSCNAREATYPE GetAreaTypeFromPoint_Recursion(const YsVec3 &pos) const;

protected:
	template <const int N>
	int GetRectRegionFromPoint_Recursion(YsArray <const YsSceneryRectRegion *,N> &rgnList,const YsVec3 &pos) const;

public:
	template <const int N>
	YSRESULT GetAllPointSet(YsArray <const YsSceneryPointSet *,N> &pstLst) const;
	template <const int N>
	YSRESULT SearchPointSetById(YsArray <const YsSceneryPointSet *,N> &pstLst,int id) const;
	template <const int N>
	YSRESULT SearchPointSetByTag(YsArray <const YsSceneryPointSet *,N> &pstLst,const char *tag) const;
	template <const int N>
	YSRESULT GetPointSet(YsArray <YsVec3,N> &point,const YsSceneryPointSet *pst) const;

	YSRESULT GetFirstPointOfPointSet(YsVec3 &point,const YsSceneryPointSet *pst) const;

public:
	double GetElevation(const YsSceneryItem *&evg,const YsVec3 &pos) const;
protected:
	YSRESULT GetElevation_Recursion(const YsSceneryItem *&evg,double &elv,const YsVec3 &pos) const;

public:
	void GetElevationAndNormal(const YsSceneryItem *&evg,double &elv,YsVec3 &nom,const YsVec3 &pos);
protected:
	YSRESULT GetElevationAndNormal_Recursion(const YsSceneryItem *&evg,double &elv,YsVec3 &nom,const YsVec3 &pos) const;

public:
	YSBOOL GetShellCollisionByBoundingBox(const YsVec3 &pos,const double &buff) const;
	const YsSceneryShell *CheckShellCollision(const YsShell &shl,const YsMatrix4x4 &modelTfm) const;
	const YsSceneryShell *CheckShellCollision(YsVec3 &itscPos,const YsVec3 &p1,const YsVec3 &p2,const YsMatrix4x4 &fieldTfm) const;

	const char *GetIdName(void) const;

	const YsColor &GetGroundColor(void) const;
	const YsColor &GetSkyColor(void) const;

	void SetSpecular(YSBOOL s);
	YSBOOL GetSpecular(void) const;

	YSSCNAREATYPE GetDefaultAreaType(void) const;
	const double &GetBaseElevation(void) const;
	const double &GetMagneticVariation(void) const;
	YSBOOL GetCanResume(void) const;
	YSBOOL GetCanContinue(void) const;

	void MakeListOfMap(YsArray <YsScenery2DDrawing *,16> &drwList) const;
	void MakeListOfGndObj(YsArray <YsSceneryGndObj *> & gobList) const;
	void MakeListOfAir(YsArray <YsSceneryAir *> &airList) const;
	void MakeListOfPointSet(YsArray <YsSceneryPointSet *> & pstList) const;
	const YsSceneryGndObj *SearchGndObjByTag(const char tag[]) const;

	YsListItem <YsSceneryShell> *FindNextShell(const YsListItem <YsSceneryShell> *);
	YsListItem <YsSceneryElevationGrid> *FindNextElevationGrid(const YsListItem <YsSceneryElevationGrid> *);
	YsListItem <YsScenery2DDrawing> *FindNextMap(const YsListItem <YsScenery2DDrawing> *);
	YsListItem <YsScenery2DDrawing> *FindNextSignBoard(const YsListItem <YsScenery2DDrawing> *);
	YsListItem <YsSceneryRectRegion> *FindNextRectRegion(const YsListItem <YsSceneryRectRegion> *);
	YsListItem <YsScenery> *FindNextChildScenery(const YsListItem <YsScenery> *);
	YsListItem <YsSceneryGndObj> *FindNextGndObj(const YsListItem <YsSceneryGndObj> *);
	YsListItem <YsSceneryAir> *FindNextAir(const YsListItem <YsSceneryAir> *);
	YsListItem <YsSceneryPointSet> *FindNextPointSet(const YsListItem <YsSceneryPointSet> *);

	YsListItem <YsSceneryShell> *FindPrevShell(const YsListItem <YsSceneryShell> *);
	YsListItem <YsSceneryElevationGrid> *FindPrevElevationGrid(const YsListItem <YsSceneryElevationGrid> *);
	YsListItem <YsScenery2DDrawing> *FindPrevMap(const YsListItem <YsScenery2DDrawing> *);
	YsListItem <YsScenery2DDrawing> *FindPrevSignBoard(const YsListItem <YsScenery2DDrawing> *);
	YsListItem <YsSceneryRectRegion> *FindPrevRectRegion(const YsListItem <YsSceneryRectRegion> *);
	YsListItem <YsScenery> *FindPrevChildScenery(const YsListItem <YsScenery> *);
	YsListItem <YsSceneryGndObj> *FindPrevGndObj(const YsListItem <YsSceneryGndObj> *);
	YsListItem <YsSceneryAir> *FindPrevAir(const YsListItem <YsSceneryAir> *);
	YsListItem <YsSceneryPointSet> *FindPrevPointSet(const YsListItem <YsSceneryPointSet> *);

	const YsListItem <YsSceneryShell> *FindNextShell(const YsListItem <YsSceneryShell> *) const;
	const YsListItem <YsSceneryElevationGrid> *FindNextElevationGrid(const YsListItem <YsSceneryElevationGrid> *)const;
	const YsListItem <YsScenery2DDrawing> *FindNextMap(const YsListItem <YsScenery2DDrawing> *) const;
	const YsListItem <YsScenery2DDrawing> *FindNextSignBoard(const YsListItem <YsScenery2DDrawing> *) const;
	const YsListItem <YsSceneryRectRegion> *FindNextRectRegion(const YsListItem <YsSceneryRectRegion> *) const;
	const YsListItem <YsScenery> *FindNextChildScenery(const YsListItem <YsScenery> *) const;
	const YsListItem <YsSceneryGndObj> *FindNextGndObj(const YsListItem <YsSceneryGndObj> *) const;
	const YsListItem <YsSceneryAir> *FindNextAir(const YsListItem <YsSceneryAir> *) const;
	const YsListItem <YsSceneryPointSet> *FindNextPointSet(const YsListItem <YsSceneryPointSet> *) const;

	const YsListItem <YsSceneryShell> *FindPrevShell(const YsListItem <YsSceneryShell> *) const;
	const YsListItem <YsSceneryElevationGrid> *FindPrevElevationGrid(const YsListItem <YsSceneryElevationGrid> *)const;
	const YsListItem <YsScenery2DDrawing> *FindPrevMap(const YsListItem <YsScenery2DDrawing> *) const;
	const YsListItem <YsScenery2DDrawing> *FindPrevSignBoard(const YsListItem <YsScenery2DDrawing> *) const;
	const YsListItem <YsSceneryRectRegion> *FindPrevRectRegion(const YsListItem <YsSceneryRectRegion> *) const;
	const YsListItem <YsScenery> *FindPrevChildScenery(const YsListItem <YsScenery> *) const;
	const YsListItem <YsSceneryGndObj> *FindPrevGndObj(const YsListItem <YsSceneryGndObj> *) const;
	const YsListItem <YsSceneryAir> *FindPrevAir(const YsListItem <YsSceneryAir> *) const;
	const YsListItem <YsSceneryPointSet> *FindPrevPointSet(const YsListItem <YsSceneryPointSet> *) const;

	YsListItem <YsSceneryShell> *CreateShell(void);
	YsListItem <YsSceneryElevationGrid> *CreateElevationGrid(void);
	YsListItem <YsScenery2DDrawing> *CreateMap(void);
	YsListItem <YsScenery2DDrawing> *CreateSignBoard(void);
	YsListItem <YsSceneryRectRegion> *CreateRectRegion(void);
	YsListItem <YsScenery> *CreateChildScenery(void);
	YsListItem <YsSceneryGndObj> *CreateGndObj(void);
	YsListItem <YsSceneryAir> *CreateAir(void);
	YsListItem <YsSceneryPointSet> *CreatePointSet(void);


	YsSceneryAirRouteHandle CreateAirRoute(void);
	YSRESULT FindNextAirRoute(YsSceneryAirRouteHandle &arHd) const;
	YSRESULT FindPrevAirRoute(YsSceneryAirRouteHandle &arHd) const;
	const YsSceneryAirRoute *GetAirRoute(YsSceneryAirRouteHandle arHd) const;
	const YsSceneryAirRoute *FindAirRouteByTag(const char tag[]) const;
	YsSceneryAirRoute *GetAirRoute(YsSceneryAirRouteHandle arHd);
	void FindAllAirRoute(YsArray <const YsSceneryAirRoute *> &airRouteArray) const;
private:
	void FindAllAirRoute_Recursion(YsArray <const YsSceneryAirRoute *> &airRouteArray) const;
public:
	void FindAllSceneryAndAirRoutePair(YsArray <YsSceneryAndAirRoutePair> &scnRoutePairArray) const;
private:
	void FindAllSceneryAndAirRoutePair_Recursion(YsArray <YsSceneryAndAirRoutePair> &scnRoutePairArray) const;
public:
	void ClearAllAirRoute(void);
	void ApplySceneryAndAirRoutePair(int nRoute,const YsSceneryAndAirRoutePair route[]);
};

template <const int N>
YSRESULT YsScenery::SearchRegionById(YsArray <const YsSceneryRectRegion *,N> &rgnLst,int id) const
{
	YsArray <const YsScenery *,16> todo;

	rgnLst.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsSceneryRectRegion> *rgn;
		rgn=NULL;
		while((rgn=todo[0]->rgnList.FindNext(rgn))!=NULL)
		{
			if(rgn->dat.GetId()==id)
			{
				rgnLst.Append(&rgn->dat);
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}

	return YSOK;
}

template <const int N>
YSRESULT YsScenery::SearchRegionByTag(YsArray <const YsSceneryRectRegion *,N> &rgnLst,const char tag[]) const
{
	YsArray <const YsScenery *,16> todo;

	rgnLst.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsSceneryRectRegion> *rgn;
		rgn=NULL;
		while((rgn=todo[0]->rgnList.FindNext(rgn))!=NULL)
		{
			if(strcmp(rgn->dat.GetTag(),tag)==0)
			{
				rgnLst.Append(&rgn->dat);
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}

	return YSOK;
}

template <const int N>
int YsScenery::GetRectRegionFromPoint(YsArray <const YsSceneryRectRegion *,N> &rgnList,const YsVec3 &pos) const
{
	rgnList.Set(0,NULL);
	return GetRectRegionFromPoint_Recursion(rgnList,pos);
}

template <const int N>
int YsScenery::GetRectRegionFromPoint_Recursion(YsArray <const YsSceneryRectRegion *,N> &rgnList,const YsVec3 &posOutside) const
{
	YsVec2 tst2,bbx2[2];
	YsVec3 tst,pos;
	const YsListItem <YsSceneryRectRegion> *rgn;

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
		rgn=NULL;
		while((rgn=FindNextRectRegion(rgn))!=NULL)
		{
			tst=pos;
			tst-=rgn->dat.pos;
			tst.RotateXZ(-rgn->dat.att.h());
			tst.RotateZY(-rgn->dat.att.p());
			tst.RotateXY(-rgn->dat.att.b());

			tst2.GetXZ(tst);
			if(YsCheckInsideBoundingBox2(tst2,rgn->dat.min,rgn->dat.max)==YSTRUE)
			{
				rgnList.Append(&rgn->dat);
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=FindNextChildScenery(scn))!=NULL)
		{
			scn->dat.GetRectRegionFromPoint_Recursion(rgnList,pos);
		}
	}
	return (int)rgnList.GetN();
}

template <const int N>
YSRESULT YsScenery::GetAllPointSet(YsArray <const YsSceneryPointSet *,N> &pstLst) const
{
	YsArray <const YsScenery *,16> todo;

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

	return YSOK;
}

template <const int N>
YSRESULT YsScenery::SearchPointSetById(YsArray <const YsSceneryPointSet *,N> &pstLst,int id) const
{
	YsArray <const YsScenery *,16> todo;

	pstLst.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsSceneryPointSet> *pst;
		pst=NULL;
		while((pst=todo[0]->pstList.FindNext(pst))!=NULL)
		{
			if(pst->dat.GetId()==id)
			{
				pstLst.Append(&pst->dat);
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}

	return YSOK;
}

template <const int N>
YSRESULT YsScenery::SearchPointSetByTag(YsArray <const YsSceneryPointSet *,N> &pstLst,const char *tag) const
{
	YsArray <const YsScenery *,16> todo;

	pstLst.Set(0,NULL);
	todo.Append(this);
	while(todo.GetN()>0)
	{
		YsListItem <YsSceneryPointSet> *pst;
		pst=NULL;
		while((pst=todo[0]->pstList.FindNext(pst))!=NULL)
		{
			if(strcmp(pst->dat.GetTag(),tag)==0)
			{
				pstLst.Append(&pst->dat);
			}
		}

		const YsListItem <YsScenery> *scn;
		scn=NULL;
		while((scn=todo[0]->scnList.FindNext(scn))!=NULL)
		{
			todo.Append(&scn->dat);
		}

		todo.DeleteBySwapping(0);
	}

	return YSOK;
}

template <const int N>
YSRESULT YsScenery::GetPointSet(YsArray <YsVec3,N> &point,const YsSceneryPointSet *pst) const
{
	int i;
	YsMatrix4x4 tfm;
	GetTransformation(tfm,pst);

	point.Set(pst->GetNumPoint(),NULL);

	for(i=0; i<pst->GetNumPoint(); i++)
	{
		tfm.Mul(point[i],pst->GetPointArray()[i],1.0);
	}

	return YSOK;

}

////////////////////////////////////////////////////////////

void YsSceneryPrepareGroundTextureTile(void); // Currently works only in OpenGL 2.0

/* } */
#endif
