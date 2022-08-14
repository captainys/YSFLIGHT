#ifndef FSNEWFLIGHTDIALOG_IS_INCLUDED
#define FSNEWFLIGHTDIALOG_IS_INCLUDED
/* { */

#include "fsexistence.h"
#include "fsdialog.h"
#include "fsguiselectiondialogbase.h"

/*
Field: AOMORI
StartPos: MISAWA_RW10
[Change Start Position]

Airplane: F-4EJ PHANTOM
[Change Airplane]

Wingman1: F-4EJ PHANTOM
StartPos: 
[Change Wingman1]

Wingman2: F-4EJ PHANTOM
Startpos:
[Change Wingman2]

Wingman3: F-4EJ PHANTOM
Startpos:
[Change Wingman2]

Environment
[ ] Day
[ ] Night

Weather
Wind Speed (kt)
Wind Direction
Visibility (mile)
*/

class FsNewSimulationDialogTemplate : public FsGuiDialogWithFieldAndAircraft
{
public:
	FsWorld *world;

	FsGuiStatic *fieldLabel;
	FsGuiListBox *field;
	FsGuiTextBox *fieldSearch;

	FsGuiButton *addComputerAircraft;

	FsGuiButton *specifyEnvironment;
	FsGuiButton *dayNight[2];
	FsGuiNumberBox *windDir;
	FsGuiNumberBox *windSpd;
	FsGuiNumberBox *visibility;

	enum
	{
		MAXNUMCLOUDLAYER=3
	};

	FsGuiButton *overCastLayerSw[MAXNUMCLOUDLAYER];
	FsGuiNumberBox *overCastLayerFloor[MAXNUMCLOUDLAYER];
	FsGuiNumberBox *overCastLayerThickness[MAXNUMCLOUDLAYER];


	FsNewSimulationDialogTemplate();
	void AddFieldSelector(YSBOOL forRacingMode);
	void AddDayNightSelector(void);
	void AddWeatherSelector(void);

	void ResetStpListBySearchKeyword(FsWorld *world,FsGuiDropList *stpList,FsGuiTextBox *stpSearch);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnNumberBoxChange(FsGuiNumberBox *nbx,int prevNum);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void FieldSelectionChanged(void)=0;
};



class FsGuiNewFlightDialogClass : public FsNewSimulationDialogTemplate
{
public:
	enum 
	{
		MAXNUMWINGMAN=5
	};

	enum 
	{
		NEWFLIGHT_NEXTACTION_NULL,
		NEWFLIGHT_NEXTACTION_SELECTPLAYERAIRPLANE,
		NEWFLIGHT_NEXTACTION_SELECTWINGMAN,
	};

	FsNewFlightDialogOption option;

	FsGuiTabControl *mainTab;

	FsGuiStatic *playerAir;
	FsGuiButton *changePlayerAir;

	FsGuiStatic *playerStpLabel;
	FsGuiDropList *playerStp;
	FsGuiTextBox *playerStpFilter;


	FsGuiStatic *wingmanAir[MAXNUMWINGMAN];
	FsGuiStatic *wingmanStpLabel[MAXNUMWINGMAN];
	FsGuiDropList *wingmanStp[MAXNUMWINGMAN];
	FsGuiButton *changeWingman[MAXNUMWINGMAN];
	FsGuiButton *removeWingman[MAXNUMWINGMAN];

	FsGuiListBox *fomType;

	FsGuiButton *okBtn,*cancelBtn,*flyNowButton;

	FsNewFlightDialogInfo info;

	class FsGuiChooseAircraft *chooseAircraftDialog;

	int chooseWingmanIdx;

	FsGuiNewFlightDialogClass();
	FsGuiNewFlightDialogClass(const FsNewFlightDialogOption &opt);
	FsGuiNewFlightDialogClass(YSBOOL canSelectWingmen,YSBOOL canChooseNight,YSBOOL canChooseFomType,YSBOOL flyNowButton);
	~FsGuiNewFlightDialogClass();
	void InitialSetUp(const FsNewFlightDialogOption &opt);
	void Make(FsWorld *world);
	virtual void FieldSelectionChanged(void);
	void ResetStartPosChoice(FsGuiDropList *dropList,FsWorld *world,const char fldName[]);
	void InitializeDialog(FsWorld *world,const FsNewFlightDialogInfo &info);
	void MakeDefaultWingmanPosition(YsString str[]);
	void RefreshCloudLayer(void);

	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
	virtual void OnNumberBoxChange(FsGuiNumberBox *nbx,int prevNum);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);
	virtual void OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int modalDialogEndCode);
};



class FsCreateNewSimulationInGroundObjectDialogClass : public FsNewSimulationDialogTemplate
{
protected:
	YSBOOL forAirDefenseMission;

public:
	FsGuiTabControl *mainTab;
	FsGuiListBox *gndListBox;
	FsGuiButton *okBtn,*cancelBtn,*flyNowButton;
	FsGuiStatic *unavailableMessageStatic;

	FsGuiButton *allowStealth;
	FsGuiButton *allowBomb;
	FsGuiButton *allowAGM;
	FsGuiButton *allowHeavyBomber;

	FsGuiNumberBox *maxNumAttacker;

	FsGuiButton *jet,*ww2;

	YSRESULT res;
	YSBOOL flyNow;


	FsCreateNewSimulationInGroundObjectDialogClass();
	void Make(FsWorld *world,YSBOOL forAirDefenseMissionIn);
	void Initialize(const FsFlightConfig &cfg);
	void Initialize(const FsFlightConfig &cfg,const FsGroundToAirDefenseMissionInfo &info);
	void ReloadUserControllableGroundObject(void);
	YSBOOL NeedToDrawGndObjCursor(YsVec3 &pos) const;

	void Retrieve(class FsNewDriveDialogInfo &info);
	void Retrieve(class FsGroundToAirDefenseMissionInfo &info);

	virtual void FieldSelectionChanged(void);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
	virtual void OnNumberBoxChange(FsGuiNumberBox *nbx,int prevNum);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);
};



class FsGuiEnduranceModeDialog : public FsGuiDialogWithFieldAndAircraft
{
public:
	enum
	{
		ENDURANCEDLG_NEXTACTION_NULL,
		ENDURANCEDLG_NEXTACTION_SELECTPLAYERAIRCRAFT
	};

	FsGuiStatic *playerAirName;
	FsGuiButton *changePlayer;
	FsGuiListBox *fieldList;
	FsGuiTextBox *fieldSearch;
	FsGuiButton *allowAAM;
	FsGuiNumberBox *numWingman;
	FsGuiListBox *wingmanLevel;
	FsGuiButton *jet,*ww2;
	FsGuiButton *okBtn,*cancelBtn;

	FsWorld *world;
	FsEnduranceModeDialogInfo info;

	class FsGuiChooseAircraft *chooseAircraftDialog;

	int nextActionCode;

	mutable unsigned int lastCursorMoveClock;

	FsGuiEnduranceModeDialog();
	~FsGuiEnduranceModeDialog();
	void Make(FsWorld *world,int nextActionCode);
	void ResetFieldList(void);
	virtual void FieldSelectionChanged(void);
	void Initialize(FsWorld *world,FsFlightConfig &cfg);
	void Retrieve(void);
	void DrawSelectedFieldAndAircraft(void) const;

	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);
	virtual void OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int nextActionCode);
};



class FsGuiInterceptMissionDialog : public FsGuiDialogWithFieldAndAircraft
{
public:
	enum
	{
		INTERCEPTDLG_NEXTACTION_NULL,
		INTERCEPTDLG_NEXTACTION_SELECTPLAYERAIRCRAFT
	};

	FsGuiStatic *playerAirName;
	FsGuiButton *changePlayer;
	FsGuiListBox *fieldList;
	FsGuiTextBox *fieldSearch;

	FsGuiButton *allowEnemyFighter;
	FsGuiButton *allowStealth;
	FsGuiButton *allowBomb;
	FsGuiButton *allowHeavyBomber;

	FsGuiNumberBox *numWingman;
	FsGuiNumberBox *maxNumAttacker;

	FsGuiButton *jet,*ww2;

	FsGuiButton *okBtn,*cancelBtn;

	FsWorld *world;
	FsInterceptMissionInfo info;

	FsGuiChooseAircraft *chooseAircraftDialog;

	int nextActionCode;

	mutable unsigned int lastCursorMoveClock;

	FsGuiInterceptMissionDialog();
	~FsGuiInterceptMissionDialog();

	void Make(FsWorld *world,int nextActionCode);
	void Initialize(FsWorld *world,FsFlightConfig &cfg);
	void Retrieve(void);
	void DrawSelectedFieldAndAircraft(void) const;

	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;
	virtual void FieldSelectionChanged(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);
	virtual void OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int nextActionCode);
};



class FsGuiCloseAirSupportMissionDialog : public FsGuiDialogWithFieldAndAircraft
{
public:
	enum
	{
		CLOSEAIRDIALOG_NEXTACTION_NULL,
		CLOSEAIRDIALOG_NEXTACTION_SELECTPLAYERAIRCRAFT
	};

	YsString playerAirNameString;
	YsArray <int,64> playerWeaponConfig;
	int playerFuel;
	YsString fieldString;

	FsGuiStatic *playerAirName;
	FsGuiButton *changePlayer;
	FsGuiListBox *fieldList;
	FsGuiTextBox *fieldSearch;

	FsGuiButton *okBtn,*cancelBtn;

	FsWorld *world;

	FsGuiChooseAircraft *chooseAircraftDialog;

	int nextActionCode;

	mutable unsigned int lastCursorMoveClock;

	FsCloseAirSupportMissionInfo info;

	FsGuiCloseAirSupportMissionDialog();
	~FsGuiCloseAirSupportMissionDialog();

	void Make(FsWorld *world,FsFlightConfig &cfg,int nextActionCode);

	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;
	virtual void FieldSelectionChanged(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);
	virtual void OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int nextActionCode);
};



/* } */
#endif
