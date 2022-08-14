#ifndef FSGUICOMMONDIALOG_IS_INCLUDED
#define FSGUICOMMONDIALOG_IS_INCLUDED
/* { */

#include <fsgui.h>

#include "fsguinewflightdialog.h"

class FsGuiConfirmDeleteFlightDialog : public FsGuiDialog
{
private:
	FsGuiStatic *label;
	FsGuiDialogItem *yesBtn,*noBtn,*btn;
	int nextActionCode;

public:
	YSRESULT res;
	YsWString whatToOpenNext;

	void Make(int dialogId,int nextActionCode);
	virtual void OnButtonClick(FsGuiButton *btn);
};



class FsGuiNoJoystickWarningDialogClass : public FsGuiDialog
{
public:
	int nextActionCode;
	FsGuiButton *okBtn,*dontShowItAgainBtn;
	void Make(int nextActionCode);
	virtual void OnKeyDown(int fsKey,YSBOOL shift,YSBOOL ctrl,YSBOOL alt);
	virtual void OnButtonClick(FsGuiButton *btn);
};

class FsGuiAboutDialog : public FsGuiDialog
{
public:
	FsGuiButton *openURLBtn,*bannerBtn;
	FsGuiButton *okBtn;
	void Make(void);
	virtual void OnKeyDown(int fsKey,YSBOOL shift,YSBOOL ctrl,YSBOOL alt);
	virtual void OnButtonClick(FsGuiButton *btn);
};

////////////////////////////////////////////////////////////

class FsGuiFirstDialogClass : public FsGuiDialog
{
public:
	int nextActionCode;

	FsGuiButton *ysflightComBtn;
	FsGuiButton *downloadPageBtn;
	FsGuiButton *okBtn1,*okBtn2;
	FsGuiButton *bannerBtn;

	void Make(int nextActionCode);
	virtual void OnButtonClick(FsGuiButton *btn);
};

#ifdef _WIN32
class FsGuiExplainJWordDialog : public FsGuiDialog
{
public:
	FsGuiButton *okBtn,*urlBtn;
	void Make(void);
	void OnButtonClick(FsGuiButton *btn);
};

class FsGuiAskInstallJWordOnFirstStart : public FsGuiDialog
{
public:
	FsGuiExplainJWordDialog explainJWordDlg;
	FsGuiCanvas *canvas;
	FsGuiButton *installBtn,*explainJWordBtn,*deleteBtn,*closeBtn;
	void Make(void);
	void OnButtonClick(FsGuiButton *btn);
};

#endif

class FsGuiSupportYsflightDialogClass : public FsGuiDialog
{
public:
	YSBOOL firstStart;
	YSBOOL needClose;
	int nextActionCode;

	FsGuiButton *downloadPageBtn;
	FsGuiButton *ysflightComBtn1,*ysflightComBtn2;
	FsGuiButton *okBtn1,*okBtn2;

	void Make(YSBOOL firstStart,int nextActioNCode);

	void OnButtonClick(FsGuiButton *btn);
	virtual void OnModalDialogClosed(int,class FsGuiDialog *closedModalDialog,int);
};

class FsGuiVoteYsflightDialogClass : public FsGuiDialog
{
public:
	FsGuiButton *votePageBtn;
	FsGuiButton *okBtn;

	void Make(void);
};

////////////////////////////////////////////////////////////

class FsGuiAirCombatDialog : public FsGuiDialogWithFieldAndAircraft
{
public:
	enum ERRORCODE
	{
		ACMDLG_ERROR_NOERROR,
		ACMDLG_ERROR_STARTPOS_UNSELECTED,
		ACMDLG_ERROR_STARTPOS_COLLISION,
		ACMDLG_ERROR_PLAYER_NOTSELECTED
	};

	enum
	{
		MAX_NUM_WINGMAN=2,
		MAX_NUM_ENEMY=5
	};

	YsString fieldName;
	FsNewFlightAirplaneData player;
	FsNewFlightAirplaneData wingman[MAX_NUM_WINGMAN];
	FsNewFlightAirplaneData enemy[MAX_NUM_ENEMY];
	FsWorld *world;
	YSRESULT res;
	ERRORCODE errCode;
	YSBOOL flyNow;


	FsGuiDropList *fieldList;
	FsGuiDropList *playerAir,*playerStp;
	FsGuiDropList *wingmanAir[MAX_NUM_WINGMAN],*wingmanStp[MAX_NUM_WINGMAN],*wingmanLevel[MAX_NUM_WINGMAN];
	FsGuiDropList *enemyAir[MAX_NUM_ENEMY],*enemyStp[MAX_NUM_ENEMY],*enemyLevel[MAX_NUM_ENEMY];
	FsGuiButton *useMissileBtn;

	FsGuiButton *okBtn,*cancelBtn,*flyNowBtn;

	void Make(class FsWorld *world);
	void Initialize(class FsWorld *world,FsFlightConfig &cfg);
	void ResetStartPos(class FsWorld *world);
	void ReloadPlayerStartPos(class FsWorld *world);
	void DrawSelectedFieldAndAircraft(void);
	YSRESULT CreateFlight(void);

	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
	virtual void OnButtonClick(FsGuiButton *btn);
};

////////////////////////////////////////////////////////////

class FsGuiSelectMissionDialog : public FsGuiDialog
{
public:
	FsGuiListBox *missionList;
	FsGuiButton *okBtn,*cancelBtn,*flyNowBtn;

	FsGuiButton *lastClicked;

	YsArray <YsWString> missionFile;

private:
	YsWString TryToGetMissionTitle(const YsWString &subDir,const YsWString &file) const;

public:
	void Make(const YsWString &subDir,YsArray <YsWString> &fileList);
	virtual void OnButtonClick(FsGuiButton *btn);

	YSRESULT GetSelectedMission(YsWString &selMissionFile) const;
};



/* } */
#endif
