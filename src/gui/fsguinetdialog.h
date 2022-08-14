#ifndef FSGUINETDIALOG_IS_INCLUDED
#define FSGUINETDIALOG_IS_INCLUDED
/* { */

#include <fsgui.h>

////////////////////////////////////////////////////////////

class FsGuiStartServerDialog : public FsGuiDialog
{
public:
	const class FsWorld *world;
	FsGuiButton *okBtn,*cancelBtn;
	FsGuiTextBox *portTxt;
	FsGuiTextBox *userNameTxt;
	FsGuiListBox *fldListBox;
	FsGuiTextBox *fldSearch;
	YSRESULT res;

	void Make(const class FsWorld *world,const class FsNetConfig &netcfg);
	void ResetFieldListBySearchKeyword(const FsWorld *world,FsGuiListBox *fieldList,FsGuiTextBox *fieldSearch);

	virtual void FieldSelectionChanged(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnListBoxSelChange(FsGuiListBox *lbx,int prevSel);
	virtual void OnTextBoxChange(FsGuiTextBox *txt);
};

class FsGuiStartClientDialog : public FsGuiDialog
{
public:
	FsGuiButton *okBtn,*cancelBtn;
	FsGuiTextBox *userNameTxt,*hostAddrTxt,*portTxt;
	FsGuiDropList *hostHist;
	YSRESULT res;

	void Make(const class FsNetConfig &netcfg);
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
	virtual void OnButtonClick(FsGuiButton *btn);
};

////////////////////////////////////////////////////////////

class FsGuiNetConfigDialog : public FsGuiDialog
{
public:
	const FsWorld *world;

	FsGuiButton *okBtn,*cancelBtn,*setDefaultBtn;

	FsGuiTabControl *mainTab;
	int generalTabId,clientTabId,server1TabId,server2TabId,server3TabId;


	FsGuiTextBox *networkPort;
	FsGuiTextBox *userName;
	FsGuiButton *saveChatLog;

	FsGuiTextBox *cliHostAddr;
	FsGuiDropList *cliStartPos;
	FsGuiDropList *cliIff;
	FsGuiButton *cliRecordFlight,*cliFreeMemory;

	FsGuiDropList *svrField;
	FsGuiDropList *svrStartPos;
	FsGuiButton *svrConfigInterceptMission;
	FsGuiButton *svrRecordFlight,*svrFreeMemory;
	FsGuiButton *svrDisableChat;
	FsGuiDropList *svrLogOnTimeOut;
	FsGuiDropList *svrMaxMultiConnection;
	FsGuiDropList *svrResetInterval;
	FsGuiDropList *svrStopAfterNTimes;
	FsGuiButton *svrSendWelcomeMessage;

	FsGuiDropList *svrIff;
	FsGuiButton *svrGroundFire;
	FsGuiButton *svrUseGuidedWeapon;
	FsGuiButton *svrUseUnguidedWeapon;
	FsGuiButton *svrDisableRadarGunSight;
	FsGuiButton *svrDisableThirdAirplaneView;
	FsGuiButton *svrNotifyTakeOffAndLeave;
	FsGuiDropList *svrShowUserName;
	FsGuiDropList *svrMidAirCollision;
	FsGuiDropList *svrBlackOut;
	FsGuiDropList *svrCanLandAnywhere;
	FsGuiDropList *svrRadarAltLimit;

	FsGuiListBox *svrBlockedIpAddr;
	FsGuiTextBox *svrBlockedIpAddrToAdd;
	FsGuiButton *svrDeleteBlockedIpAddr;
	FsGuiButton *svrAddBlockedIpAddr;


	void MakeDialog(void);
	void MakeGeneralDialog(void);
	void MakeClientDialog(void);
	void MakeServerDialog1(void);
	void MakeServerDialog2(void);
	void MakeServerDialog3(void);
	void MakeStartPosList(FsGuiDropList *lbx,const FsWorld *world,const char fldName[]);
	void MakeFieldList(FsGuiDropList *lbx,const FsWorld *world);
	void MakeBlockedIpList(FsGuiListBox *lbx);
	void SaveBlockedIpList(void);

	void InitializeDialog(const FsWorld *world,const class FsNetConfig &cfg);
	void RetrieveConfig(class FsNetConfig &cfg,const FsWorld *world);

	virtual void OnButtonClick(FsGuiButton *btn);
};

/* } */
#endif
