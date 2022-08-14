#ifndef SESCENERY_IS_INCLUDED
#define SESCENERY_IS_INCLUDED
/* { */

#include <ystexturemanager.h>

class SeScenery2DDrawingItem
{
public:
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <Ys2DDrawingElement> *elem;
};

// Example
// N RW01_01
// P IFF 0
// P CARRIER "GndObjTag" 0m 0m 0m 0deg 0deg 0deg
// C POSITION -117m 0.5m -663.2m
// C ATTITUDE -10deg 0deg 0deg
// C INITSPED 0.0MACH
// C CTLTHROT 0.0

class YsStartPos
{
friend class SeScenery;
friend class SeUndoCreateStartPos;
friend class SeUndoDeleteStartPos;
friend class SeUndoModifyStartPos;
friend class SeUndoModifyStartPosCarrier;
friend class SeUndoChangeStartPosPriority;

public:
	YsStartPos();
	~YsStartPos();

	enum
	{
		IFF0=0,
		IFF1=1,
		IFF2=2,
		IFF3=3,
		IFF_ALL=99
	};

	const char *GetName(void) const;
	const YsVec3 &GetPosition(void) const;
	const YsAtt3 &GetAttitude(void) const;
	const double &GetSpeed(void) const;
	int GetThrottle(void) const;
	// int GetFuel(void) const;
	YSBOOL GetLandingGear(void) const;
	int GetIFF(void) const;

	YSBOOL GetOnCarrier(void) const;
	const char *GetCarrierTag(void) const;
	const YsVec3 &GetPosOnCarrier(void) const;
	const YsAtt3 &GetAttOnCarrier(void) const;

protected:
	void Initialize(void);

	YsString nameStr;
	YsVec3 pos;
	YsAtt3 att;
	double speed;
	int throttle;
	// int fuel;
	YSBOOL landingGear;
	YsArray <YsString> extraCommand;
	int iff;

	YSBOOL onCarrier;
	YsArray <char> carrierTag;
	YsVec3 posOnCarrier;
	YsAtt3 attOnCarrier;

	YsListItem <YsStartPos> *thisInTheList;
};



class SeUndo;

class SeScenery : protected YsScenery
{
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
friend class SeUndoSetDrawElemSpecular;
friend class SeUndoSetDrawElemPointList;
friend class SeUndoSetDrawElemPriority;
friend class SeUndoSetDrawElemTexture;
friend class SeUndoSetDrawElemTextureCoord;
friend class SeUndoModifyGndObj;
friend class SeUndoModifyAir;
friend class SeUndoModifyRectRegion;
friend class SeUndoSetGroundSkyColor;
friend class SeUndoSetItemId;
friend class SeUndoSetItemTag;
friend class SeUndoAddTexture;
friend class SeUndoDeleteTexture;
friend class SeUndoSetTextureFileName;
friend class SeUndoSetTextureFilterType;
friend class SeUndoSetTextureRandomNoiseLevel;
friend class SeUndoSetTextureFileData;
friend class SeUndoSetElevationGridTexture;
friend class SeUndoSetElevationGridSpecular;

friend class SeUndoCreateStartPos;
friend class SeUndoDeleteStartPos;
friend class SeUndoModifyStartPos;
friend class SeUndoModifyStartPosCarrier;
friend class SeUndoChangeStartPosPriority;

friend class SeUndoChangeDefaultAreaType;
friend class SeUndoChangeGroundSpecular;
friend class SeUndoSetBaseElevation;
friend class SeUndoSetMagneticVariation;
friend class SeUndoSetCanResume;
friend class SeUndoSetCanContinue;

friend class SeUndoSetAirRoute;

protected:
	YsHashTable <YsSceneryItem *> searchItem;
	YsHashTable <Ys2DDrawingElement *> search2DElem;
public:
	class StopIncUndo
	{
	private:
		SeScenery *scn;
		int incUndo;
	public:
		StopIncUndo(SeScenery &scn);
		~StopIncUndo();
	};

	using YsScenery::GetCanResume;
	using YsScenery::GetCanContinue;

	using YsScenery::GetLastError;
	using YsScenery::Draw;
	using YsScenery::DrawMap;
	using YsScenery::DrawVisual;
	using YsScenery::DrawMapVisual;
	using YsScenery::DrawAxis;
	using YsScenery::DrawItemAxis;
	using YsScenery::DrawProtectPolygon;
	using YsScenery::DrawItem;
	using YsScenery::DrawItemStar;
	using YsScenery::Draw2DDrawingElement;
	using YsScenery::DrawILSGuideline;
	using YsScenery::DrawItemILSGuideline;
	using YsScenery::FindNextShell;
	using YsScenery::FindNextElevationGrid;
	using YsScenery::FindNextMap;
	using YsScenery::FindNextSignBoard;
	using YsScenery::FindNextRectRegion;
	using YsScenery::FindNextChildScenery;
	using YsScenery::FindPrevShell;
	using YsScenery::FindPrevElevationGrid;
	using YsScenery::FindPrevMap;
	using YsScenery::FindPrevSignBoard;
	using YsScenery::FindPrevRectRegion;
	using YsScenery::FindPrevChildScenery;
	using YsScenery::GetAllPointSet;
	using YsScenery::GetPointSet;
	using YsScenery::GetRegionRect;
	using YsScenery::GetDefaultAreaType;
	using YsScenery::GetAreaTypeFromPoint;
	using YsScenery::GetBaseElevation;
	using YsScenery::GetMagneticVariation;
	using YsScenery::FindAllAirRoute;
	using YsScenery::FindAllSceneryAndAirRoutePair;
	using YsScenery::SearchRegionById;
	using YsScenery::IsInsideRectRegion;
	using YsScenery::SearchPointSetById;

	using YsScenery::SetColorScale;
	using YsScenery::RecomputeBoundingBox;
	using YsScenery::GetTransformation;
	using YsScenery::GetParentTransformation;
	using YsScenery::ComputeTransformation;
	using YsScenery::RecursivelyUpdateBoundingBox;  // <- Don't take it as a modifier.  BBX is not saved anyway.
	using YsScenery::GetOrigin;
	using YsScenery::GetTransformedBoundingBox;
	using YsScenery::GetElevation;
	using YsScenery::GetIdName;
	using YsScenery::GetGroundColor;
	using YsScenery::GetSkyColor;
	using YsScenery::MakeListOfMap;
	using YsScenery::MakeListOfGndObj;
	using YsScenery::MakeListOfAir;
	using YsScenery::SearchGndObjByTag;

	using YsScenery::ExportYfs;

	using YsScenery::lastErrorNo;
	using YsScenery::lastErrorStr;


	SeScenery();
	YSBOOL modified,stpIsModified;
	YsString lastOpenedFName,lastOpenedStpFName;

	// Non-const functions
	void CleanUp(void);

	void SetIdName(const char nm[]);  // Not an undo target.

	YSRESULT LoadFld(const char fn[],YSBOOL autoLoadStp);
	YSRESULT SaveFld(const char fn[]);
	YSRESULT LoadStp(const char fn[]);
	YSRESULT LoadStp(YsTextInputStream &inStream);
	YSRESULT SaveStp(const char fn[]);
	YsScenery *CreateChildScenery(YsScenery *parentScn);
	YsSceneryShell *CreateShell(YsScenery *parentScn,YsVisualSrf &&srf,YsVisualSrf &&collSrf);
	YsScenery2DDrawing *CreateMap(YsScenery *parentScn);
	YsSceneryElevationGrid *CreateElevationGrid
	    (YsScenery *parentScn,int nx,int nz,const double &xWid,const double &zWid,const YsColor &col);
	YsSceneryRectRegion *CreateRectRegion(YsScenery *parentScn,int id,const char tag[]);
	YsScenery *ImportScenery(YsScenery *parentScn,const char fn[]);
	YsScenery2DDrawing *ImportMap(YsScenery *parentScn,const char fn[]);
	YsScenery2DDrawing *ImportMap(YsScenery *parentScn,YsTextInputStream &inStream);
	YsSceneryElevationGrid *ImportElvGrid(YsScenery *parentScn,const char fn[]);
	YsSceneryElevationGrid *ImportElvGrid(YsScenery *parentScn,YsTextInputStream &inStream);
	YsSceneryGndObj *CreateGndObj(YsScenery *parent,const char objName[]);
	YsSceneryAir *CreateAir(YsScenery *parent,const char objName[]);
	YsSceneryPointSet *CreatePointSet(YsScenery *parent,int np,const YsVec3 p[],YSBOOL isLoop);
	YSRESULT ImportYfs(YsScenery *parentScn,YsTextInputStream &inStream);
	YSRESULT DeleteItem(YsSceneryItem *itm);

	const class YsTextureManager &GetTextureManager(YsScenery *scn) const;
private:
	class YsTextureManager &GetTextureManagerInternal(YsScenery *scn);

public:
	YsTextureManager::TexHandle AddTexture(YsScenery *scn,const YsString &str,YsTextureManager::DATA_FORMAT fType,YSSIZE_T nByte,const unsigned char fileData[]);
	template <const int N>
	YsTextureManager::TexHandle AddTexture(YsScenery *scn,const YsString &str,YsTextureManager::DATA_FORMAT fType,const YsArray <unsigned char,N> &fileData);
	YSRESULT DeleteTexture(YsScenery *scn,YsTextureManager::TexHandle texHd);

	YSRESULT SetTextureFileName(YsScenery *scn,YsTextureManager::TexHandle texHd,const YsWString &fn);
	YSRESULT SetTextureFilterType(YsScenery *scn,YsTextureManager::TexHandle texHd,YsTextureManager::Unit::FILTERTYPE filterType);
	YSRESULT SetTextureRandomNoiseLevel(YsScenery *scn,YsTextureManager::TexHandle texHd,const double randomNoiseLevel);
	YSRESULT SetTextureFileData(YsScenery *scn,YsTextureManager::TexHandle texHd,YsTextureManager::DATA_FORMAT fType,YSSIZE_T length,const unsigned char dat[]);
	template <const int N>
	YSRESULT SetTextureFileData(YsScenery *scn,YsTextureManager::TexHandle texHd,YsTextureManager::DATA_FORMAT fType,const YsArray <unsigned char,N> &dat);

	void SetSkyColor(const YsColor &col);
	void SetGroundColor(const YsColor &col);
	void SetDefaultAreaType(YSSCNAREATYPE areaType);
	void SetGroundSpecular(YSBOOL s);
	YSBOOL GetGroundSpecular(void) const;
	void SetBaseElevation(const double &baseElev);
	void SetMagneticVariation(const double &magneticVariation);
	void SetAirRoute(int nRoute,const YsSceneryAndAirRoutePair route[]);
	void SetCanResume(YSBOOL canResume);
	void SetCanContinue(YSBOOL canContinue);

	YSRESULT MoveMapToTopPriority(YsScenery2DDrawing *map);
	YSRESULT MoveMapToBottomPriority(YsScenery2DDrawing *map);
	YSRESULT MapPriorityUp(YsScenery2DDrawing *map);
	YSRESULT MapPriorityDown(YsScenery2DDrawing *map);

	YSRESULT SetSceneryItemPosition(YsSceneryItem *itm,const YsVec3 &pos);
	YSRESULT SetSceneryItemAttitude(YsSceneryItem *itm,const YsAtt3 &att);
	YSRESULT SetSceneryItemOwner(YsSceneryItem *itm,YsScenery *newOwner);

	YSRESULT SetSceneryItemTag(YsSceneryItem *itm,const char tag[]);
	YSRESULT SetSceneryItemId(YsSceneryItem *itm,int id);

	YSRESULT SetElvGridNodeElevation(YsSceneryElevationGrid *evg,int x,int z,const double &y);
	YSRESULT ShowElvGridFace(YsSceneryElevationGrid *evg,int x,int z,int t,YSBOOL show);
	YSRESULT SetElvGridProtectPolygonFlag(YsSceneryElevationGrid *evg,int x,int z,int t,YSBOOL protPolyg);
	YSRESULT SetElvGridFaceColor(YsSceneryElevationGrid *evg,int x,int z,int t,const YsColor &c);
	YSRESULT SetElvGridTriangulation(YsSceneryElevationGrid *evg,int x,int z,YSBOOL lup);
	YSRESULT SetElvGridSideFaceShow(YsSceneryElevationGrid *evg,int fcId,YSBOOL show);
	YSRESULT SetElvGridSideFaceColor(YsSceneryElevationGrid *evg,int fcId,const YsColor &col);
	YSRESULT SetElvGridColorByElevation(YsSceneryElevationGrid *evg,YSBOOL cve,const double y[2],const YsColor c[2]);
	YSRESULT SetElvGridTexture(YsSceneryElevationGrid *evg,const YsString &texLabel);
	YSRESULT SetElvGridSpecular(YsSceneryElevationGrid *evg,YSBOOL specular);

	Ys2DDrawingElement *CreateDrawElem
	    (YsScenery2DDrawing *drw,Ys2DDrawingElement::OBJTYPE t,int np,const YsVec2 p[]);
	YSRESULT DeleteDrawElem(YsScenery2DDrawing *drw,Ys2DDrawingElement *elem);
	YSRESULT SetDrawElemColor(Ys2DDrawingElement *elem,const YsColor &col);
	YSRESULT SetDrawElemSpecular(YSSIZE_T n,Ys2DDrawingElement *const elem[],YSBOOL specular);
	template <const int N>
	YSRESULT SetDrawElemSpecular(const YsArray <Ys2DDrawingElement *,N> &elem,YSBOOL specular);
	YSRESULT SetDrawElem2ndColor(Ys2DDrawingElement *elem,const YsColor &col);
	YSRESULT SetDrawElemVisibleDistance(Ys2DDrawingElement *elm,const double &dist);
	YSRESULT SetDrawElemPointList(Ys2DDrawingElement *elm,int np,const YsVec2 p[]);
	YSRESULT SetDrawElemTexture(Ys2DDrawingElement *elm,const YsString &texLabel);
	YSRESULT SetDrawElemTextureCoord(Ys2DDrawingElement *elm,int np,const YsVec2 p[]);
	YSRESULT ChangeDrawElemType(Ys2DDrawingElement *elm,Ys2DDrawingElement::OBJTYPE t);
	YSRESULT MoveDrawElemToTopPriority(Ys2DDrawingElement *elm);
	YSRESULT MoveDrawElemToBottomPriority(Ys2DDrawingElement *elm);
	YSRESULT MoveDrawElemBefore(Ys2DDrawingElement *elm,Ys2DDrawingElement *beforeThis);
	YSRESULT DrawElemPriorityUp(Ys2DDrawingElement *elm);
	YSRESULT DrawElemPriorityDown(Ys2DDrawingElement *elm);

	YSRESULT DrawOptimize(void);

	YSRESULT SetRectRegionRect(YsSceneryRectRegion *rgn,const YsVec2 rec[]);
	YSRESULT SetRectRegionSubClassType(YsSceneryRectRegion *rgn,YsSceneryRectRegion::SUBCLASSTYPE subClassType);

	YSRESULT SetMotionPathIsLoop(YsSceneryPointSet *rgn,YSBOOL isLoop);
	YSRESULT SetMotionPathCoord(YsSceneryPointSet *rgn,int np,const YsVec3 p[]);
	YSRESULT SetMotionPathAreaType(YsSceneryPointSet *rgn,YSSCNAREATYPE areaType);

	YSRESULT RelinkGndObjTemplateAll(void);
	YSRESULT SetGndObjIff(YsSceneryGndObj *gndObj,int iff);
	YSRESULT SetGndObjPrimaryTarget(YsSceneryGndObj *gndObj,YSBOOL pmt);
	YSRESULT SetGndObjFlag(YsSceneryGndObj *gndObj,unsigned flag);
	YSRESULT SetGndObjMotionPath(YsSceneryGndObj *gndObj,const char motionPathName[],YSBOOL motionPathOffset);

	YSRESULT RelinkAirTemplateAll(void);
	YSRESULT SetAirType(YsSceneryAir *air,const char name[]);
	YSRESULT SetAirIff(YsSceneryAir *air,int iff);
	YSRESULT SetAirLandingGear(YsSceneryAir *air,YSBOOL ldg);
	YSRESULT SetAirFlag(YsSceneryAir *air,unsigned int airFlag);
	YSRESULT SetAirSpeed(YsSceneryAir *air,const double &speed);
	YSRESULT SetAirFuel(YsSceneryAir *air,int fuel);
	YSRESULT SetAirLandWhenLowFuel(YsSceneryAir *air,const double &fuelThr);
	YSRESULT SetAirAction(YsSceneryAir *air,const YsSceneryAirAction &action);

	int PushStopIncUndo(void);
	void PopIncUndo(int incUndo);

	YSRESULT ClearYsflightGndObjTemplate(void);
	YSRESULT LoadYsflightGndObjTemplate(const char ysfsDir[],const char subDir[],const char prefix[],const char ext[]);
	YSRESULT LoadYsflightGndObjTemplate(const char ysfsDir[],const char fn[]);
	YSRESULT AddYsflightGndObjTemplate(const char ysfsDir[],const char dat[],const char vis[]);

	YSRESULT ClearYsflightAirTemplate(void);
	YSRESULT LoadYsflightAirTemplate(const char ysfsDir[],const char subDir[],const char prefix[],const char ext[]);
	YSRESULT LoadYsflightAirTemplate(const char ysfsDir[],const char fn[]);
	YSRESULT AddYsflightAirTemplate(const char ysfsDir[],const char dat[],const char vis[]);

	YsStartPos *CreateStartPos(const char nam[],const YsVec3 &pos,const YsAtt3 &att);
protected:
	YsStartPos *CreateStartPos(void);
public:
	YSRESULT SetStartPosName(YsStartPos *stp,const char nam[]);
	YSRESULT SetStartPos(YsStartPos *stp,const YsVec3 &pos);
	YSRESULT SetStartPosAttitude(YsStartPos *stp,const YsAtt3 &att);
	YSRESULT SetStartPosSpeed(YsStartPos *stp,const double &speed);
	YSRESULT SetStartPosThrottle(YsStartPos *stp,int thr);
	// YSRESULT SetStartPosFuel(YsStartPos *stp,int fuel);
	YSRESULT SetStartPosLandingGear(YsStartPos *stp,YSBOOL gear);
	YSRESULT SetStartPosIFF(YsStartPos *stp,int iff);
	YSRESULT DeleteStartPos(YsStartPos *stp);
	YSRESULT StartPosPriorityUp(YsStartPos *stp);
	YSRESULT StartPosPriorityDown(YsStartPos *stp);
	YSRESULT SetStartPosOnCarrier
	   (YsStartPos *stp,YSBOOL onCarrier,const char carrierTag[],const YsVec3 &relPos,const YsAtt3 &relAtt);

	// const functions
	void GetBoundingBox(YsVec3 bbx[2]) const;
	YsSceneryItem *FindItem(unsigned searchKey) const;
	Ys2DDrawingElement *Find2DElem(unsigned searchKey) const;
	const YsListContainer <YsSceneryGndObjTemplate> &GetGndObjTemplate(void) const;
	const YsListContainer <YsSceneryAirTemplate> &GetAirTemplate(void) const;
	const YsSceneryGndObjTemplate *FindGndObjTemplate(const char gndObjName[]) const;
	const YsSceneryAirTemplate *FindAirTemplate(const char airName[],YSBOOL prepDnm) const;
	YSBOOL IsModified(void) const;
	YSBOOL StpIsModified(void) const;
	const char *GetLastOpenedFilename(void) const;
	const char *GetLastOpenedStpFilename(void) const;
	void SetLastOpenedFilename(const char fn[]);

	YsStartPos *FindNextStartPos(YsStartPos *stp) const;
	YsStartPos *FindPrevStartPos(YsStartPos *stp) const;

	const YsScenery *GetYsScenery(void) const;

protected:
	YsListContainer <YsStartPos> stpList,frozenStpList;
	YsListAllocator <YsStartPos> stpAllocator;



public:
	YSRESULT Undo(void);
	YSRESULT Redo(void);
	YSRESULT CleanUpUndoLog(void);

protected:
	YSRESULT AddUndo(SeUndo *undo);
	YSRESULT AddUndoStp(SeUndo *undo);

	int incUndo,undoCounter;
	SeUndo *undoLog,*undoPtr,*redoPtr;

	YsListContainer <YsSceneryShell> frozenShlList;
	YsListContainer <YsSceneryElevationGrid> frozenEvgList;
	YsListContainer <YsScenery2DDrawing> frozenMapList;
	YsListContainer <YsScenery2DDrawing> frozenSbdList;
	YsListContainer <YsSceneryRectRegion> frozenRgnList;
	YsListContainer <YsScenery> frozenScnList;
	YsListContainer <YsSceneryGndObj> frozenGndList;
	YsListContainer <YsSceneryAir> frozenAirList;
	YsListContainer <YsSceneryPointSet> frozenPstList;

	YsListContainer <YsSceneryGndObjTemplate> gndObjTemplateList;
	static YsListAllocator <YsSceneryGndObjTemplate> gndObjTemplateAllocator;

	YsListContainer <YsSceneryAirTemplate> airTemplateList;
	static YsListAllocator <YsSceneryAirTemplate> airTemplateAllocator;

	YSRESULT CleanUpSearchTable(void);
	YSRESULT RefreshSearchTable(void);
	YSRESULT RefreshSearchTable(YsScenery *scn);
};

template <const int N>
YSRESULT SeScenery::SetDrawElemSpecular(const YsArray <Ys2DDrawingElement *,N> &elem,YSBOOL specular)
{
	return SetDrawElemSpecular(elem.GetN(),elem,specular);
}

template <const int N>
YsTextureManager::TexHandle SeScenery::AddTexture(YsScenery *scn,const YsString &str,YsTextureManager::DATA_FORMAT fType,const YsArray <unsigned char,N> &fileData)
{
	return AddTexture(scn,str,fType,fileData.GetN(),fileData);
}

template <const int N>
YSRESULT SeScenery::SetTextureFileData(YsScenery *scn,YsTextureManager::TexHandle texHd,YsTextureManager::DATA_FORMAT fType,const YsArray <unsigned char,N> &dat)
{
	return SetTextureFileData(scn,texHd,fType,dat.GetN(),dat);
}


class SeUndo
{
public:
	SeUndo();
	virtual ~SeUndo(){}
	virtual YSRESULT Undo(class SeScenery *scn)=0;
	virtual YSRESULT Redo(class SeScenery *scn)=0;
	virtual YSRESULT WillNeverBeRedone(class SeScenery *scn);
	virtual YSRESULT WillNeverBeUndone(class SeScenery *scn);
	int undoCount;
	SeUndo *prev,*next;
};

class SeUndoCreateItem : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	virtual YSRESULT WillNeverBeRedone(class SeScenery *scn);
	YsScenery *parent;
	YsSceneryItem *itm;
};

class SeUndoDeleteItem : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	virtual YSRESULT WillNeverBeUndone(class SeScenery *scn);
	YsScenery *parent;
	YsSceneryItem *itm,*beforeThisItm;
};

class SeUndoChangeMapPriority : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsScenery *parent;
	YsListItem <YsScenery2DDrawing> *itm,*fromBeforeThis,*toBeforeThis;
};

class SeUndoMoveItemPosAtt : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsSceneryItem *itm;
	YsVec3 orgPos,newPos;
	YsAtt3 orgAtt,newAtt;
};

class SeUndoSetItemOwner : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsSceneryItem *itm;
	YsScenery *orgOwner,*newOwner;
	YsSceneryItem *orgBeforeThis;
};

class SeUndoSetItemTag : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsSceneryItem *itm;
	YsString orgTag,newTag;
};

class SeUndoSetItemId : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsSceneryItem *itm;
	int orgId,newId;
};

class SeUndoModifyElevationGridNode : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsSceneryElevationGrid *evg;
	int x,z;
	YsElevationGridNode orgNode,newNode;
};

class SeUndoModifyElevationGridSideWall : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsSceneryElevationGrid *evg;
	int sideFaceId;
	YSBOOL orgShow,newShow;
	YsColor orgColor, newColor;
};

class SeUndoSetElevationGridColorByElevation : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsSceneryElevationGrid *evg;
	YSBOOL orgColorByElevation;
	double orgColorByElevation_Elevation[2];
	YsColor orgColorByElevation_Color[2];
	YSBOOL newColorByElevation;
	double newColorByElevation_Elevation[2];
	YsColor newColorByElevation_Color[2];
};

class SeUndoCreate2DDrawElem : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	virtual YSRESULT WillNeverBeRedone(class SeScenery *scn);

	YsScenery2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *elm;
};

class SeUndoDelete2DDrawElem : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	virtual YSRESULT WillNeverBeUndone(class SeScenery *scn);

	YsScenery2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *elm,*beforeThis;
};

class SeUndoSetDrawElemTypeColorVisidist : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	void CaptureOrg(Ys2DDrawingElement *elem);
	void CaptureNew(Ys2DDrawingElement *elem);

	Ys2DDrawingElement *elm;
	Ys2DDrawingElement::OBJTYPE orgType,newType;
	YsColor orgColor,newColor;
	YsColor orgColor2,newColor2;
	double orgVisiDist,newVisiDist;
};

class SeUndoSetDrawElemSpecular : public SeUndo
{
public:
	YsArray <Ys2DDrawingElement *> elem;
	YsArray <YSBOOL> orgSpecular;
	YSBOOL newSpecular;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetDrawElemPointList : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	Ys2DDrawingElement *elm;
	YsArray <YsVec2> orgPntList,newPntList;
};

class SeUndoSetDrawElemPriority : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	Ys2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *elm,*orgBeforeThis,*newBeforeThis;
};

class SeUndoModifyGndObj : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
	YsSceneryGndObj *gndObj;
	int orgIff,newIff;
	unsigned orgFlag,newFlag;
	YSBOOL orgPmt,newPmt;
	YsString orgMotionPathName,newMotionPathName;
	YSBOOL orgMotionPathOffset,newMotionPathOffset;
};

class SeUndoModifyAir : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	void CaptureOrg(YsSceneryAir *air);
	void CaptureNew(YsSceneryAir *air);

	YsSceneryAir *air;
	int orgIff,newIff;
	YSBOOL orgLdg,newLdg;
	unsigned int orgFlag,newFlag;
	YsString orgType,newType;
	unsigned int orgFuel,newFuel;
	double orgSpeed,newSpeed;
	double orgLandWhenLowFuel,newLandWhenLowFuel;
	YsSceneryAirAction orgAct,newAct;
};

class SeUndoModifyRectRegion : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	void CaptureOrg(YsSceneryRectRegion *rgn);
	void CaptureNew(YsSceneryRectRegion *rgn);

	YsSceneryRectRegion *rgn;
	YsVec2 orgRgn[2],newRgn[2];
	int orgId,newId;
	YsString orgTag,newTag;
	YsSceneryRectRegion::SUBCLASSTYPE prevSubClassType,newSubClassType;
};

class SeUndoSetGroundSkyColor : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsColor orgSkyColor,orgGndColor;
	YsColor newSkyColor,newGndColor;
};

class SeUndoCreateStartPos : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsStartPos *stp;
};

class SeUndoDeleteStartPos : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsStartPos *stp;
	YsListItem <YsStartPos> *prevBeforeThis;
};

class SeUndoModifyStartPos : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsStartPos *stp;

	YsString orgNameStr;
	YsVec3 orgPos;
	YsAtt3 orgAtt;
	double orgSpeed;
	int orgThrottle;
	// int orgFuel;
	YSBOOL orgLandingGear;
	int orgIFF,newIFF;

	YsString newNameStr;
	YsVec3 newPos;
	YsAtt3 newAtt;
	double newSpeed;
	int newThrottle;
	// int newFuel;
	YSBOOL newLandingGear;
};

class SeUndoModifyStartPosCarrier : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsStartPos *stp;

	YSBOOL orgOnCarrier;
	YsArray <char> orgCarrierTag;
	YsVec3 orgPosOnCarrier;
	YsAtt3 orgAttOnCarrier;

	YSBOOL newOnCarrier;
	YsArray <char> newCarrierTag;
	YsVec3 newPosOnCarrier;
	YsAtt3 newAttOnCarrier;
};

class SeUndoChangeStartPosPriority : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsStartPos *stp;
	YsListItem <YsStartPos> *orgBeforeThis,*newBeforeThis;
};

class SeUndoSetMotionPathIsLoop : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsSceneryPointSet *mpa;
	YSBOOL isLoopOrg,isLoopNew;
};

class SeUndoSetMotionPathCoord : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsSceneryPointSet *mpa;
	YsArray <YsVec3> mpaOrg,mpaNew;
};

class SeUndoChangeDefaultAreaType : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YSSCNAREATYPE prvAreaType,newAreaType;
};

class SeUndoChangeGroundSpecular : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YSBOOL specularBefore,specularAfter;
};

class SeUndoSetBaseElevation : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	double prvBaseElev,newBaseElev;
};

class SeUndoSetMagneticVariation : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	double prvMagneticVariation,newMagneticVariation;
};

class SeUndoChangeMotionPathAreaType : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsSceneryPointSet *mpa;
	YSSCNAREATYPE prvAreaType,newAreaType;
};

class SeUndoSetAirRoute : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YsArray <YsSceneryAndAirRoutePair> prvScnRoutePairArray,newScnRoutePairArray;
};

class SeUndoSetCanResume : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YSBOOL prvCanResume,newCanResume;
};

class SeUndoSetCanContinue : public SeUndo
{
public:
	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);

	YSBOOL prvCanContinue,newCanContinue;
};

class SeUndoAddTexture : public SeUndo
{
public:
	YsScenery *scn;
	YsTextureManager::TexHandle texHd;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoDeleteTexture : public SeUndo
{
public:
	YsScenery *scn;
	YsTextureManager::TexHandle texHd;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetTextureFileName : public SeUndo
{
public:
	YsScenery *scn;
	YsTextureManager::TexHandle texHd;
	YsWString oldFn,newFn;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetDrawElemTexture : public SeUndo
{
public:
	Ys2DDrawingElement *elm;
	YsString texLabelBefore,texLabelAfter;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndosetDrawElemTextureCoord : public SeUndo
{
public:
	Ys2DDrawingElement *elm;
	YsArray <YsVec2> texCoordBefore,texCoordAfter;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetTextureFilterType : public SeUndo
{
public:
	YsScenery *scn;
	YsTextureManager::TexHandle texHd;
	YsTextureManager::Unit::FILTERTYPE filterBefore,filterAfter;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetElevationGridTexture : public SeUndo
{
public:
	YsSceneryElevationGrid *evg;
	YsString texLabelBefore,texLabelAfter;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetElevationGridSpecular : public SeUndo
{
public:
	YsSceneryElevationGrid *evg;
	YSBOOL specularBefore,specularAfter;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetTextureRandomNoiseLevel : public SeUndo
{
public:
	YsScenery *scn;
	YsTextureManager::TexHandle texHd;
	double randomNoiseBefore,randomNoiseAfter;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

class SeUndoSetTextureFileData : public SeUndo
{
public:
	YsScenery *scn;
	YsTextureManager::TexHandle texHd;
	YsTextureManager::DATA_FORMAT fTypeBefore,fTypeAfter;
	YsArray <unsigned char> fileDataBefore,fileDataAfter;

	virtual YSRESULT Undo(class SeScenery *scn);
	virtual YSRESULT Redo(class SeScenery *scn);
};

/* } */
#endif
