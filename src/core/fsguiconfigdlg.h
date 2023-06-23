#ifndef FSGUICONFIGDLG_IS_INCLUDED
#define FSGUICONFIGDLG_IS_INCLUDED
/* { */

#include <fsgui.h>

#include "fsoption.h"
#include "fscontrol.h" // FsControlAssignment used in the key-assign dialog

class FsGuiConfigDialog : public FsGuiDialog
{
public:
	class FsWorld *world;

	FsGuiTabControl *mainTab;

	FsGuiButton *okBtn,*cancelBtn,*resetBtn;

	FsGuiButton *dayOrNightBtn[2];
	FsGuiButton *lightSrcBtn[5];

	FsGuiDropList *airLst,*fldLst,*stpLst;

	FsGuiButton *blackOutBtn,*midAirCollisionBtn,*noTailStrikeBtn,*canLandAnywhereBtn;
	FsGuiButton *autoRudderBtn,*preciseSimulationBtn,*alwaysShowHudBtn,*doNotUseInstPanelBtn;
	FsGuiButton *showIASBtn;
	FsGuiButton *simpleHudBtn,*threeDHudBtn;
	FsGuiButton *alwaysDrawPlayerNameBtn,*drawVirtualJoystickBtn,*f8CameraDelayBtn;
	FsGuiTextBox *radarAltLimitTxt;
	FsGuiStatic *troubleFrequencyText;
	FsGuiSlider *troubleFrequency;
	FsGuiStatic *aircraftReliabilityText;
	FsGuiSlider *aircraftReliability;

	FsGuiButton *drawShadowBtn,*drawShadowOfDeadAirplaneBtn,*drawOrdinanceBtn;
	FsGuiButton *drawCoarseWeaponBtn,*horizonGradationBtn;
	FsGuiDropList *cloudLbx;
	FsGuiButton *drawLightInDaylightBtn;
	FsGuiTextBox *drawLightInDaylightVisibilityThrTxt;
	FsGuiDropList *airplaneGraphicsLbx;
	FsGuiDropList *smokeTypeLbx;
	FsGuiTextBox *smokeRemainTime;
	FsGuiTextBox *smokeDrawEveryNStep;
	FsGuiButton *showFpsBtn;
	FsGuiButton *drawBurningSmokeByParticle;

	FsGuiButton *fogBtn;
	FsGuiTextBox *fogVisibility;
	FsGuiDropList *zBufQualityLbx;
	FsGuiButton *trspObjBtn,*trspSmkBtn,*trspVaporBtn,*reducePeepHoleBtn;
	FsGuiButton *useDlistCloudBtn,*useDlistExplosionBtn,*useDlistWeaponBtn;
	FsGuiButton *useGroundTextureBtn,*useRunwayLightTextureBtn;
	FsGuiButton* msaaButton;

#ifdef __APPLE__
	FsGuiButton *useOpenGlAntiAliasing;
#endif



	void MakeDialog(FsWorld *world,FsFlightConfig &cfg);
	void MakeDefaultsDialog(FsWorld *world,FsFlightConfig &cfg);
	void MakeGameDialog(FsWorld *world,FsFlightConfig &cfg);
	void MakeGraphicDialog(FsWorld *world,FsFlightConfig &cfg);
	void MakeOpenGLDialog(FsWorld *world,FsFlightConfig &cfg);
	void CheckSelectionChange(FsWorld *world);
	void ResetStartPositionList(FsWorld *world,const char fldName[]);

	void InitializeDialog(FsWorld *world,FsFlightConfig &cfg);
	void RetrieveConfig(FsFlightConfig &cfg);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnSliderPositionChange(FsGuiSlider * /*slider*/,const double & /*prevPos*/,const double & /*prevValue*/);
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
};



class FsGuiOptionDialog : public FsGuiDialog
{
public:
	FsGuiDropList *scrnMode;  // Windows only
	FsGuiButton *rememberWindowPos;
	FsGuiButton *alwaysOnTop;  // Windows only

	FsGuiDropList *fontSize;  // Windows 8, 10, 12, 14, to 22

	FsGuiButton *useMapPreview;

	FsGuiButton *soundBtn,*openingDemoBtn;
	FsGuiButton *useTaskBarIcon;

	FsGuiButton *backPicture;

	FsGuiDropList *langType;
	FsGuiStatic *langFile;
	FsGuiButton *langFileButton;

#ifdef _WIN32
	FsGuiButton *direct3DSucksButton;
#endif

	FsGuiButton *okBtn;
	FsGuiButton *cancelBtn;
	FsGuiButton *setDefaultBtn;

	class FsGuiFileDialog *fdlg;

	FsOption option;

	FsGuiOptionDialog();
	~FsGuiOptionDialog();
	void Make(void);
	void Initialize(const FsOption &option);
	void Retrieve(void);
	void SelectLanguageFile(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
    virtual void OnModalDialogClosed(int dialogIdent,class FsGuiDialog *closedModalDialog,int modalDialogEndCode);
};



class FsGuiKeyAssignDialogClass : public FsGuiDialog
{
public:
	FsGuiTabControl *mainTab;


	FsGuiListBox *joyAxis,*joyAxisFunc;
	FsGuiNumberBox *deadZoneElevator;
	FsGuiNumberBox *deadZoneAileron;
	FsGuiNumberBox *deadZoneRudder;
	FsGuiButton *usePovHatSwitch;
	FsGuiButton *changeJoyAxisFuncBtn;

	FsGuiListBox *joyTrig,*joyTrigFunc;
	FsGuiButton *changeJoyTrigFuncBtn;

	FsGuiListBox *key,*keyFunc;
	FsGuiButton *changeKeyFuncBtn;

	FsGuiButton *setDefaultBtn;
	FsGuiButton *setDefaultButPrimaryJoystickIsNotJoystick1;
	FsGuiButton *setDefaultMouseAsStickBtn;
	FsGuiButton *setDefaultKeyboadAsStickBtn;
	FsGuiButton *setDefaultKeyAssignKeepMouseAndStickBtn;
	FsGuiButton *setDefaultGamePad;

	FsGuiButton *saveBtn;
	FsGuiButton *loadBtn;
	FsGuiButton *okBtn,*cancelBtn;

	class FsGuiFileDialog *fdlg;

	FsControlAssignment ctlAssign;

	int axisShowTop,trigShowTop,keyShowTop;
	int axisSel,trigSel,keySel;

	FsGuiKeyAssignDialogClass();
	~FsGuiKeyAssignDialogClass();

	void Make(void);
	void Initialize(void);

	void SaveListBoxPos(void);
	void RestoreListBoxPos(void);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnListBoxScroll(FsGuiListBox *lbx,int prevShowTop);
    virtual void OnModalDialogClosed(int dialogIdent,class FsGuiDialog *closedModalDialog,int modalDialogEndCode);

	void OnCloseAxisAssignDialog(FsGuiDialog *dlg,int returnCode);
	void OnCloseTriggerAssignDialog(FsGuiDialog *dlg,int returnCode);
	void OnCloseKeyAssignDialog(FsGuiDialog *dlg,int returnCode);
	void OnClosePrimaryJoystickDialog(FsGuiDialog *dlg,int returnCode);
	void OnCloseGamePadDialog(FsGuiDialog *dlg,int returnCode);
};



/* } */
#endif
