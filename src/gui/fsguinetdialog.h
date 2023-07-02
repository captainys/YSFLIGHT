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
	int fontSize;
	YsArray <const char *> fldList;
	FsGuiButton *okBtn,*cancelBtn, *serverListBtn;
	FsGuiTextBox *userNameTxt,*hostAddrTxt,*portTxt;
	FsGuiDropList *hostHist;
	YSRESULT res;
	
	

	void Make(const class FsNetConfig &netcfg, const class FsWorld *world);
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
	virtual void OnButtonClick(FsGuiButton *btn);
	void FetchJsonServerList(const std::string url, std::function<void(std::string)> callback);
	void UpdateGUI(const std::string& json);
	void OnServerListDialogClosed(FsGuiDialog *dlg, int returnCode);
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

class FsGuiNetServerListOfflineDialog : public FsGuiDialog
{
	public:
	FsGuiButton *okBtn, *cancelBtn;

	void Make();
	void OnButtonClick(FsGuiButton *btn);

};

class ServerListItem
{
public:
	std::string name;
	std::string address;
	std::string website;
	std::string port;
	std::string map;
	int version;
	int players;
	



};

class FsGuiNetServerListDialog : public FsGuiDialog
{
public:
	FsGuiButton *okBtn, *cancelBtn;
	FsGuiListBox *serverList;
	FsGuiStatic *serverName, *serverAddress, *serverWebsite, *serverPort, *serverMap, *serverVersion, *serverPlayers;
	FsGuiStatic *serverNameLabel, *serverAddressLabel, *serverWebsiteLabel, *serverPortLabel, *serverMapLabel, *serverVersionLabel, *serverPlayersLabel;
	std::vector<ServerListItem> servers;
	YsArray <const char *> fldList;
	ServerListItem selectedServer;
	YSRESULT res;
	YsColor green, red;
	int wid, hei;
	int boxSpacing = 32;

	void Make(const std::vector<ServerListItem>& servers, YsArray <const char *> &fldList);
	void OnButtonClick(FsGuiButton *btn);
	void OnListBoxSelChange(FsGuiListBox *lbx, int prevSel);
	ServerListItem GetSelectedServer();

};


/* } */
#endif
