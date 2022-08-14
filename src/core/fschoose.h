#ifndef FSCHOOSE_IS_INCLUDED
#define FSCHOOSE_IS_INCLUDED
/* { */

#include "fswindow.h"
#include "graphics/common/fsopengl.h"


class FsChoose   // Now only used from RunServerModeOneStep
{
public:
	FsChoose(int nShow);
	~FsChoose();

	YsArray <const char *> choice;
	YsListContainer <YsString> fullChoice;
	YsListAllocator <YsString> fullChoiceAllocator;


	// For choosing airplane
	int aam,aim9x,aim120,agm,bomb,bomb250,bomb500hd,rocket;
	YSBOOL allowAam,allowAgm,allowBomb,allowRocket;
	YSBOOL smk;
	YsColor smkCol;
	int fuel;
	YsArray <int,64> weaponConfig;

	YSBOOL enableFilter;
	YsListContainer <YsString> filterList;
	YsKeyWordList filter;
	YSRESULT NetworkAddFilterWord(int n,const YsString airNameFilter[]);


	YSBOOL showNormal;
	YSBOOL showUtility;
	YSBOOL showAerobatic;
	YSBOOL showFighter;
	YSBOOL showAttacker;
	YSBOOL showTrainer;
	YSBOOL showHeavyBomber;
	YSBOOL showWW2Fighter;
	YSBOOL showWW2Bomber;

	YSBOOL keyWordSearchMode,keyWordNoMatch;
	YsString searchKeyWord;
	YSBOOL smokeColorMode;
	YsString smokeColorCode;

	YSBOOL showCancelButton;


	YSRESULT MakeAirplaneList(FsWorld *world);
	YSRESULT MakeStartPositionList(FsWorld *world,const char fieldName[]);
	YSRESULT ChooseByName(const char name[]);

	YSRESULT ResetOrdinanceOfAirplane(void);
	void ApplyWeaponLimit(YsArray <int,64> &weaponConfig);
	YSRESULT KeyIn(int ky,int c);
	YSRESULT ProcessMouse(int x0,int y0,YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my);

	YSRESULT KeyInForModeChange(int ky,int c);
	YSRESULT KeyInForSearchKeyWord(int ky,int c);
	void ReduceChoiceByKeyWord(void);
	void AddChoiceByKeyWord(void);
	YSRESULT KeyInForSmokeColorCode(int ky,int c);

	void AddRemoveLoading(FSWEAPONTYPE wpnType,int addRemove);
	YSBOOL IsWeaponAvailable(FSWEAPONTYPE wpnType);
	void Rotate(const double &h);

	int GetChoiceId(void);
	const char *GetChoice(void);
	YSBOOL IsChoiceValid(void);
	int GetNumChoice(void);

	void DrawChosenAirplane(FsWorld *world);
	void DrawChosenField(FsWorld *world);
	void Draw(int x1,int y1);

	YSRESULT KeyInInConsoleMode(YSBOOL &selected,YSBOOL &cancelled,int ky,int c);
	void PrintInConsoleMode(void);

	void Clear();
	int AddChoice(const char name[]);
	void DeleteChoice(int id);

	const char *GetCurrentPath(void);
	const char *GetCurrentFileName(void);

	YSBOOL GetOkButtonClicked(void);
	YSBOOL GetCancelButtonClicked(void);

protected:
	FsWorld *wld;
	int nShow;
	int currentCursorPosition;
	int currentMouseSelectPosition; // selection=currentShowPosition+currentMouseSelectPosition
	int currentMouseButtonPosition;
	YSBOOL choosingAirplane;
	int lastCursorMoveClock;
	YsAtt3 att; // For drawing an airplane


	YSBOOL prevLb,prevMb,prevRb;


	YSBOOL choosingScenery;
	YsScenery *scn;
	YsString loadedField;


	YSBOOL choosingFile;
	YSBOOL choosingFileForReadOnly;
	YsString curName,curPath;


	YSBOOL okButtonVisible,mouseOnOkButton;
	int okButtonX0,okButtonY0,okButtonX1,okButtonY1;
	YSBOOL okButtonClicked;

	YSBOOL mouseOnCancelButton;
	int cancelButtonX0,cancelButtonY0,cancelButtonX1,cancelButtonY1;
	YSBOOL cancelButtonClicked;
};



#ifndef FS_NOGUIDIALOG

#include "ysbitmap.h"
#include "fsgui.h"

class FsGuiChooseField : public FsGuiDialogWithFieldAndAircraft
{
public:
	YsString selFld,selStp;

	FsGuiListBox *fldLbx,*stpLbx;
	FsGuiTextBox *fldSearch;
	FsGuiButton *okBtn,*cancelBtn;
	class FsWorld *world;
	// YsScenery *scn;
	unsigned int lastCursorMoveClock;

	YSBOOL useFldListBox;
	YsString fldNameForStpSelector;

	YSBOOL useStpListBox;

	YSRESULT res;

	// YSBOOL stpLoaded;
	// YsVec3 stpPos;
	// YsAtt3 stpAtt;


	FsGuiChooseField();
	~FsGuiChooseField();
	void Initialize(void);
	void ResetSelection(void);

	virtual void FieldSelectionChanged(void);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);

	void Create(FsWorld *world);
	void Select(const char fldName[],const char stpName[]);
	void CaptureSelection(void);

	const char *GetSelectedField(YsString &fldStr) const;
	const char *GetSelectedStartPos(YsString &stpStr) const;

	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;
	// void DrawSelectedField(void);

	void ResetStartPosSelection(void);
	void ReloadField(void);
	void ReloadStartPosition(void);
};


class FsGuiChooseAircraftOption
{
public:
	YSBOOL canSelectLoading;

	FsGuiChooseAircraftOption();
};

class FsGuiChooseAircraft : public FsGuiDialogWithFieldAndAircraft
{
public:
	YsString selAir;
	YsArray <int,64> selWeaponConfig;
	int selFuel;

	YSBOOL createSearch;
	YSBOOL showAirplane;
	int selectListBoxRow;

	YSBOOL filterEnabled;
	YsKeyWordList filter;


	FsGuiTabControl *mainTab;
	int aircraftTabId,loadingTabId,categoryTabId;


	FsGuiListBox *airLbx;
	FsGuiTextBox *searchTxt;
	FsGuiButton *okBtn,*cancelBtn;
	FsGuiNumberBox *aamSrNbx,*aim9xNbx,*aamMrNbx,*agmNbx,*bom500Nbx,*bom250Nbx,*bom500HdNbx,*rktNbx,*fuelNbx;
	FsGuiNumberBox *flarePodNbx,*fuelTankNbx;
	FsGuiButton *smkBtn,*sameSmkColBtn;
	FsGuiColorPalette *smkCpl[FsAirplaneProperty::MaxNumSmokeGenerator];
	FsGuiButton *catNormal,*catUtility,*catAerobatic;
	FsGuiButton *catFighter,*catAttacker,*catTrainer,*catBomber;
	FsGuiButton *catWw2Fighter,*catWw2Attacker,*catWw2Bomber,*catWw2DiveBomber;


	YSBOOL allowAam,allowAgm,allowBomb,allowRocket;

	YsArray <int,64> tmpWeaponConfig;

	YsAtt3 att; // For drawing an airplane

	FsWorld *world;

	int nextActionCode;

	FsGuiChooseAircraft();
	void Initialize(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnNumberBoxChange(FsGuiNumberBox *nbx,int prevNum);
	virtual void OnMouseMove(
	    YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my,
	    YSBOOL plb,YSBOOL pmb,YSBOOL prb,int pmx,int pmy,
	    FsGuiDialogItem *mouseOver);
	virtual void OnColorPaletteChange(FsGuiColorPalette *plt);


	YSRESULT Create(FsWorld *world,const FsGuiChooseAircraftOption &option,int nextActionCode);
	void EnableSelectListBox(void);
	void DisableSelectListBox(void);
	void DisableCategoryButton(void);
	void EnableAamButton(void);
	void DisableAamButton(void);
	void EnableAgmButton(void);
	void DisableAgmButton(void);
	void EnableBombButton(void);
	void DisableBombButton(void);
	void EnableRocketButton(void);
	void DisableRocketButton(void);
	void EnableFuelButton(void);
	void DisableFuelButton(void);
	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;
	void CheckLoadingChange(void);
	void ApplyWeaponLimit(YsArray <int,64> &weaponConfig);
	void ResetOrdinance(void);
	void SetOrdinance(YsArray <int,64> &weaponConfig);
	void SetOrdinanceByAirplaneProp(const FsAirplaneProperty &prop);

	void SetDefault(const char airName[]);
	void CaptureSelection(void);
	void ResetSelection(void);

	void ResetAircraftList(void);

	YSBOOL SmokeLoaded(void) const;
	const YsColor SmokeColor(int smkIdx) const;

protected:
	void ChangeLoading(FSWEAPONTYPE wpnType,int n);
	// void DrawChosenAirplane(void);
};

#endif // #ifndef FS_NOGUIDIALOG


/* } */
#endif
