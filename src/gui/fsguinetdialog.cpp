#include <ysclass.h>
#include <ysport.h>

#ifdef _WIN32
#define _WINSOCKAPI_
#endif

#include <fsgui.h>
#include <fssimplewindow.h>
#include <fswindow.h>

#include <fstextresource.h>

#include <fsfilename.h>
#include <fsopengl.h>

#include <fsconfig.h>
#include <fsoption.h>

#include <fssimulation.h>
#include <fsworld.h>
#include <fsexistence.h>

#include <fsnetconfig.h>
#include <fsnetwork.h>

#include "fsguinetdialog.h"

#include <curl/curl.h>

#include <nlohmann/json.hpp>

#include <string>
#include <iostream>
#include <thread>


void FsGuiStartServerDialog::Make(const class FsWorld *world,const FsNetConfig &netcfg)
{
	int i;
	const char *fldName;
	YsArray <const char *> fldList;
	for(i=0; NULL!=(fldName=world->GetFieldTemplateName(i)); i++)
	{
		fldList.Append(fldName);
	}
	YsQuickSortString <int> (fldList.GetN(),fldList,NULL);

	Initialize();

	this->world=world;

	SetTransparency(YSFALSE);

	SetTextMessage("-- STARTING NETWORK SERVER --");

	okBtn=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);
	userNameTxt=AddTextBox(0,FSKEY_NULL,FSGUI_NET_USERNAME,netcfg.defUser,32,YSTRUE);
	userNameTxt->SetText(netcfg.defUser);
	userNameTxt->SelectAll();

	YsString portStr;
	portStr.Printf("%d",netcfg.portNumber);
	portTxt=AddTextBox(0,FSKEY_NULL,FSGUI_NET_HOSTPORT,portStr,16,YSTRUE);
	portTxt->SetTextType(FSGUI_INTEGER);
	portTxt->SelectAll();

	fldListBox=AddListBox(3,FSKEY_NULL,FSGUI_NET_FIELD,fldList.GetN(),fldList,5,32,YSTRUE);
	fldListBox->SelectByString(netcfg.defField);

	fldSearch=AddTextBox(0,FSKEY_NULL,"Search","",32,YSTRUE);
	Fit();
}

void FsGuiStartServerDialog::ResetFieldListBySearchKeyword(const FsWorld *world,FsGuiListBox *fieldList,FsGuiTextBox *fieldSearch)
{
	YsString searchTxt,curSel;
	YsArray <YsString,16> searchKeyWord;

	if(fieldList!=NULL)
	{
		fieldList->GetSelectedString(curSel);
	}

	if(fieldSearch!=NULL)
	{
		YsString str;
		fieldSearch->GetText(str);
		if(0<str.Strlen())
		{
			searchTxt.Set(str);
			searchTxt.Capitalize();
			searchTxt.Arguments(searchKeyWord);
		}
	}


	YsArray <const char *> nameLst;
	const char *name;
	int i;
	for(i=0; (name=world->GetFieldTemplateName(i))!=NULL; i++)
	{
		if(searchKeyWord.GetN()>0)
		{
			YsString cap(name);
			cap.Capitalize();
			if(FsTestKeyWordMatch(cap,searchKeyWord.GetN(),searchKeyWord)!=YSTRUE)
			{
				continue;
			}
		}

		nameLst.Append(name);
	}

	if(0<nameLst.GetN())
	{
		if(NULL!=fieldList)
		{
			fieldList->SetChoice(nameLst.GetN(),nameLst);
			if(0<curSel.Strlen())
			{
				YsString newCurSel;
				fieldList->SelectByString(curSel,YSTRUE);
				if(YSOK!=fieldList->GetSelectedString(newCurSel) || strcmp(newCurSel,curSel)!=0)
				{
					FieldSelectionChanged();
				}
			}
		}
	}
}

void FsGuiStartServerDialog::FieldSelectionChanged(void)
{
}

void FsGuiStartServerDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		res=YSOK;
		CloseModalDialog(0);
	}
	else if(btn==cancelBtn)
	{
		res=YSERR;
		CloseModalDialog(0);
	}
}

void FsGuiStartServerDialog::OnListBoxSelChange(FsGuiListBox *,int)
{
}

void FsGuiStartServerDialog::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==fldSearch)
	{
		ResetFieldListBySearchKeyword(world,fldListBox,fldSearch);
	}
}

////////////////////////////////////////////////////////////

void FsGuiStartClientDialog::Make(const FsNetConfig &netcfg, const class FsWorld *world)
{	
	//Build a field list
	int i;
	const char *fldName;
	for(i=0; NULL!=(fldName=world->GetFieldTemplateName(i)); i++)
	{
		fldList.Append(fldName);
	}
	YsQuickSortString <int> (fldList.GetN(),fldList,NULL);

	SetTransparency(YSFALSE);
	
	SetTextMessage("-- STARTING NETWORK CLIENT --");

	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);
	userNameTxt=AddTextBox(MkId("username"),FSKEY_NULL,FSGUI_NET_USERNAME,netcfg.defUser,32,YSTRUE);
	userNameTxt->SetLengthLimit(200);
	userNameTxt->SelectAll();
	hostAddrTxt=AddTextBox(MkId("hostname"),FSKEY_NULL,FSGUI_NET_HOSTNAME,netcfg.defHost,32,YSTRUE);
	hostAddrTxt->SelectAll();

	YsString portStr;
	portStr.Printf("%d",netcfg.portNumber);
	portTxt=AddTextBox(MkId("port"),FSKEY_NULL,FSGUI_NET_HOSTPORT,portStr,16,YSTRUE);
	portTxt->SetTextType(FSGUI_INTEGER);
	portTxt->SelectAll();

	FsGuiStatic *histLabel;
	histLabel=AddStaticText(0,FSKEY_NULL,FSGUI_NET_HISTORY,26,1,YSTRUE);
	histLabel->SetFill(YSFALSE);
	histLabel->SetDrawFrame(YSFALSE);

	FsServerAddressLog hist;
	YsArray <const char *> histStr;
	hist.ReadAddressFromFile();
	histStr.Set(hist.addrLog.GetN(),NULL);
	for(int i=0; i<hist.addrLog.GetN(); ++i)
	{
		histStr[i]=hist.addrLog[i];
	}
	hostHist=AddDropList(4,FSKEY_NULL,"",histStr.GetN(),histStr,12,32,32,YSTRUE);
	serverListBtn = AddTextButton(MkId("serverlist"), FSKEY_NULL, FSGUI_PUSHBUTTON, "Server List", YSTRUE);
	Fit();
}

void FsGuiStartClientDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		res=YSOK;
		CloseModalDialog(0);
	}
	if(btn==cancelBtn)
	{
		res=YSERR;
		CloseModalDialog(0);
	}
	if(btn==serverListBtn)
	{
		std::string url = "https://ysflight.org/serverlist/?json=1";
		auto callback = [this](std::string jsonData){
			UpdateGUI(jsonData);
			};

		std::thread threadObj(&FsGuiStartClientDialog::FetchJsonServerList, this, url, callback);
		threadObj.detach();
	}
}

void FsGuiStartClientDialog::OnDropListSelChange(FsGuiDropList *drp,int )
{
	if(drp==hostHist)
	{
		YsString str;
		hostHist->GetSelectedString(str);

		YsString port;
		for(int i=0; i<str.Strlen(); ++i)
		{
			if('('==str[i])
			{
				port.Set(str.Txt()+i+1);
				str.SetLength(i);
				break;
			}
		}

		hostAddrTxt->SetText(str);

		if(0<port.Strlen())
		{
			for(int i=0; i<port.Strlen(); ++i)
			{
				if(')'==port[i])
				{
					port.SetLength(i);
					break;
				}
			}
			if(0<port.Strlen())
			{
				portTxt->SetText(port);
			}
		}
	}
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

void FsGuiStartClientDialog::FetchJsonServerList(const std::string url, std::function<void(std::string)> callback)
{
	// Fetch the JSON from the URL string:
	CURL *curl=curl_easy_init();
	CURLcode res;
	long resCode;
	std::string json;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);



	res = curl_easy_perform(curl);
	
	curl_easy_strerror(res);

	if (res != CURLE_OK)
	{
		printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		
		
	}
	else
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resCode);
		curl_easy_cleanup(curl);
		printf("Response code: %d\n", resCode);
		if (resCode == 200)
		{
			callback(json);
		}
		else
		{
			auto dlg = FsGuiDialog::CreateSelfDestructiveDialog <FsGuiNetServerListOfflineDialog>();
			dlg->Make();
			AttachModalDialog(dlg);
		}
	}
	
}

void FsGuiStartClientDialog::UpdateGUI(const std::string& json)
{
	
	auto jsonServerList = nlohmann::json::parse(json);
	if (jsonServerList.contains("servers") && jsonServerList["servers"].is_array())
	{
		
		auto servers = jsonServerList["servers"];
		std::vector<ServerListItem> serverListItems;

		for (auto server : servers)
		{
			if (server.contains("name") && server["name"].is_string() &&
				server.contains("ip") && server["ip"].is_string() &&
				server.contains("port") && server["port"].is_number_integer())
			{
				auto game = server["game"];
				if (game["status"] == "Online"){
					ServerListItem item;
					item.name = server["name"];
					item.address = server["ip"];
					int port = server["port"];
					item.port = std::to_string(port);
					item.map = game["map"];
					item.players = game["players"]["total"];
					item.website = server["website"];
					item.version = game["version"];

					serverListItems.push_back(item);

				}
			}
		}
		
		auto dlg = FsGuiDialog::CreateSelfDestructiveDialog<FsGuiNetServerListDialog>();
		dlg->Make(serverListItems, fldList);
		dlg->BindCloseModalCallBack(&FsGuiStartClientDialog::OnServerListDialogClosed,this);
		AttachModalDialog(dlg);

	}
}

void FsGuiStartClientDialog::OnServerListDialogClosed(FsGuiDialog *dlg, int returnCode){
	auto serverListDlg = dynamic_cast<FsGuiNetServerListDialog*>(dlg);
	if (serverListDlg != nullptr){
		if (YSOK == serverListDlg->res){
			auto server = serverListDlg->GetSelectedServer();
			if (!server.name.empty()){
				hostAddrTxt->SetText(server.address.c_str());
				portTxt->SetText(server.port.c_str());
				res = YSOK;
				CloseModalDialog(0);
			}
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiNetConfigDialog::MakeDialog(void)
{
	Initialize();

	YsArray <FsGuiButton *> btnGroup;

	SetTopLeftCorner(0,0);
	SetTextMessage("-- Network Config --");

	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);

	generalTabId=AddTab(mainTab,FSGUI_NETCFG_GENERAL);
	BeginAddTabItem(mainTab,generalTabId);
	MakeGeneralDialog();
	EndAddTabItem();

	clientTabId=AddTab(mainTab,FSGUI_NETCFG_CLIENT);
	BeginAddTabItem(mainTab,clientTabId);
	MakeClientDialog();
	EndAddTabItem();

	server1TabId=AddTab(mainTab,FSGUI_NETCFG_SERVER1);
	BeginAddTabItem(mainTab,server1TabId);
	MakeServerDialog1();
	EndAddTabItem();

	server2TabId=AddTab(mainTab,FSGUI_NETCFG_SERVER2);
	BeginAddTabItem(mainTab,server2TabId);
	MakeServerDialog2();
	EndAddTabItem();

	server3TabId=AddTab(mainTab,FSGUI_NETCFG_SERVER3);
	BeginAddTabItem(mainTab,server3TabId);
	MakeServerDialog3();
	EndAddTabItem();

	mainTab->SelectFirstTab();


	okBtn        =AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn    =AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);
	setDefaultBtn=AddTextButton(MkId("setDefault"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_SETDEFAULT,YSFALSE);


	ExpandTab(mainTab);
	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiNetConfigDialog::MakeGeneralDialog(void)
{
	networkPort=AddTextBox(0,FSKEY_NULL,FSGUI_NETCFG_PORT,"",32,YSTRUE);
	networkPort->SetTextType(FSGUI_INTEGER);
	userName=AddTextBox(0,FSKEY_NULL,FSGUI_NETCFG_USERNAME,"",32,YSTRUE);
	saveChatLog=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_CHATLOG,YSTRUE);
}

void FsGuiNetConfigDialog::MakeClientDialog(void)
{
	cliHostAddr=AddTextBox(0,FSKEY_NULL,FSGUI_NETCFG_DEFSVR,"",32,YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_DEFSTP,YSTRUE);
	cliStartPos=AddEmptyDropList(2,FSKEY_NULL,"Default Start Position",5,24,32,YSFALSE);

	const char *const iffList[]={"IFF 0","IFF 1","IFF 2","IFF 3",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_DEFIFF,YSTRUE);
	cliIff=AddDropList(0,FSKEY_NULL,"Default IFF",4,iffList,4,10,10,YSFALSE);

	cliRecordFlight=AddTextButton(MkId("cliRecord"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_RECFLT,YSTRUE);
	cliFreeMemory=AddTextButton(MkId("cliFreeMem"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_FREEMEM,YSTRUE);
}

void FsGuiNetConfigDialog::MakeServerDialog1(void)
{
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_DEFFLD,YSTRUE);
	svrField=AddEmptyDropList(MkId("svrDefField"),FSKEY_NULL,"Default Field",4,24,32,YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_DEFSTP,YSTRUE);
	svrStartPos=AddEmptyDropList(0,FSKEY_NULL,"Default Starting Position",4,24,32,YSFALSE);

	svrConfigInterceptMission=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCFG_INTERCEPT,YSTRUE);
	svrRecordFlight=AddTextButton(MkId("svrRecord"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_RECFLT,YSTRUE);
	svrFreeMemory=AddTextButton(MkId("svrFreeMem"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_FREEMEM,YSTRUE);
	svrDisableChat=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_DISABLECHAT,YSTRUE);

	const char *const logOnTimeOutLst[]={"No Time Out","15 seconds","30 seconds","45 seconds","60 seconds","75 seconds","90 seconds","105 seconds","120 seconds",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_LOGONTIMEOUT,YSTRUE);
	svrLogOnTimeOut=AddDropList(0,FSKEY_NULL,"Log-On Time Out",9,logOnTimeOutLst,4,20,20,YSFALSE);

	const char *const multiConnList[]={
	    "As many as available","1 connection","2 connections","3 connections",
	    "4 connections","5 connections","6 connections","7 connections","8 connections",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_MAXCONN,YSTRUE);
	svrMaxMultiConnection=AddDropList(0,FSKEY_NULL,"Max Connection",9,multiConnList,4,24,32,YSFALSE);

	const char *const resetItvlList[]={"Never Reset","1 Hour","3 Hours","6 Hours","12 Hours","24 Hours","3 Days","7 Days",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_SVRRESET,YSTRUE);
	svrResetInterval=AddDropList(0,FSKEY_NULL,"Reset Server Every:",8,resetItvlList,4,20,20,YSFALSE);

	const char *const stopSvrAfter[]={"Never","1","2","3","4","5","6","7","8","9","10",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_SVRSTOP,YSTRUE);
	svrStopAfterNTimes=AddDropList(0,FSKEY_NULL,"Stop Server After ? Times:",11,stopSvrAfter,4,16,16,YSFALSE);

	svrSendWelcomeMessage=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_WELCOMEMSG,YSTRUE);
}

void FsGuiNetConfigDialog::MakeServerDialog2(void)
{
	const char *const iffList[]={"IFF 0","IFF 1","IFF 2","IFF 3",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_DEFIFF,YSTRUE);
	svrIff=AddDropList(0,FSKEY_NULL,"Default IFF",4,iffList,4,10,10,YSFALSE);

	svrGroundFire=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_GROUNDFIRE,YSTRUE);
	svrUseGuidedWeapon=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_USEMISSILE,YSTRUE);
	svrUseUnguidedWeapon=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_USEGUN,YSTRUE);
	svrDisableRadarGunSight=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_RADARGUNSIGHT,YSTRUE);
	svrDisableThirdAirplaneView=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_NO3RDPLANEVIEW,YSTRUE);
	svrNotifyTakeOffAndLeave=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCFG_NOTIFYJOINLEAVE,YSTRUE);

	const char *const showUserNameLst[]={"Always","Never","Within 1000m","Within 2000m","Within 4000m",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_SHOWUSERNAME,YSTRUE);
	svrShowUserName=AddDropList(0,FSKEY_NULL,"Show User Name",5,showUserNameLst,5,20,20,YSFALSE);

	const char *const svrControlStr1[]={"Don't Control","Enable","Disable","Same as Server",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_MIDAIR,YSTRUE);
	svrMidAirCollision=AddDropList(0,FSKEY_NULL,"Mid-Air Collision",4,svrControlStr1,4,20,20,YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_BLACKOUT,YSTRUE);
	svrBlackOut=AddDropList(0,FSKEY_NULL,"Black Out",4,svrControlStr1,4,20,20,YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_CANLANDANYWHERE,YSTRUE);
	svrCanLandAnywhere=AddDropList(0,FSKEY_NULL,"Can Land Anywhere",4,svrControlStr1,4,20,20,YSFALSE);

	const char *const svrControlStr2[]={"Don't Control","Same as Server",NULL};
	AddStaticText(0,FSKEY_NULL,FSGUI_NETCFG_MINRADARALT,YSTRUE);
	svrRadarAltLimit=AddDropList(0,FSKEY_NULL,"Minimum Radar Alt",2,svrControlStr2,2,20,20,YSFALSE);
}

void FsGuiNetConfigDialog::MakeServerDialog3(void)
{
	svrBlockedIpAddr=AddEmptyListBox(0,FSKEY_NULL,FSGUI_NETCFG_BLOCKEDIP,5,32,YSTRUE);
	svrDeleteBlockedIpAddr=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCFG_DELETEBLOCKEDIP,YSTRUE);
	svrBlockedIpAddrToAdd=AddTextBox(0,FSKEY_NULL,FSGUI_NETCFG_NEWBLOCKEDIP,"",32,YSTRUE);
	svrAddBlockedIpAddr=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCFG_ADDBLOCKEDIP,YSTRUE);
}

void FsGuiNetConfigDialog::MakeStartPosList(FsGuiDropList *lbx,const FsWorld *world,const char fldName[])
{
	int i;
	YsArray <const char *> strList;
	YsString ysStr,curStp;
	YsArray <YsString> ysStrList;

	lbx->GetSelectedString(curStp);

	ysStrList.Set(0,NULL);
	for(i=0; world->GetFieldStartPositionName(ysStr,fldName,i)==YSOK; i++)
	{
		ysStrList.Append(ysStr);
	}
	strList.Set(ysStrList.GetN(),NULL);
	forYsArray(i,ysStrList)
	{
		strList[i]=ysStrList[i];
	}
	lbx->SetChoice(strList.GetN(),strList);

	lbx->SelectByString(curStp,YSTRUE);  // YSTRUE -> force selection
	if(lbx->GetSelection()<0)
	{
		lbx->Select(0);
	}
}

void FsGuiNetConfigDialog::MakeBlockedIpList(FsGuiListBox *lbx)
{
	int i;
	YsArray <const char *> strList;
	YsArray <YsString> ysStrList;

	lbx->ClearChoice();

	FILE *fp=YsFileIO::Fopen(FsGetIpBlockFile(),"r");
	if(fp!=NULL)
	{
		YsString str;
		while(str.Fgets(fp)!=NULL)
		{
			ysStrList.Append(str);
		}

		strList.Set(ysStrList.GetN(),NULL);
		forYsArray(i,ysStrList)
		{
			strList[i]=ysStrList[i];
		}
		lbx->SetChoice(strList.GetN(),strList);

		fclose(fp);
	}
}

void FsGuiNetConfigDialog::SaveBlockedIpList(void)
{
	FILE *fp=YsFileIO::Fopen(FsGetIpBlockFile(),"w");
	if(fp!=NULL)
	{
		int i;
		YsString str;
		for(i=0; YSOK==svrBlockedIpAddr->GetString(str,i); i++)
		{
			fprintf(fp,"%s\n",str.Txt());
		}

		fclose(fp);
	}
}

void FsGuiNetConfigDialog::MakeFieldList(FsGuiDropList *lbx,const FsWorld *world)
{
	int i;
	const char *str;
	YsArray <const char *> strList;
	strList.Set(0,NULL);
	for(i=0; NULL!=(str=world->GetFieldTemplateName(i)); i++)
	{
		strList.Append(str);
	}
	lbx->SetChoice(strList.GetN(),strList);
}


void FsGuiNetConfigDialog::InitializeDialog(const FsWorld *world,const FsNetConfig &cfg)
{
	this->world=world;

	networkPort->SetInteger(cfg.portNumber);
	userName->SetText(cfg.defUser);
	saveChatLog->SetCheck(cfg.saveChatLog);

	cliHostAddr->SetText(cfg.defHost);

	MakeStartPosList(cliStartPos,world,cfg.defField);
	cliStartPos->SelectByString(cfg.defStartPosClient,YSTRUE);  // YSTRUE -> force selection

	cliIff->Select(cfg.defIFFWhenClient);
	cliRecordFlight->SetCheck(cfg.recordWhenClientMode);
	cliFreeMemory->SetCheck(cfg.freeMemoryWhenPossibleClientMode);


	MakeFieldList(svrField,world);
	svrField->SelectByString(cfg.defField,YSTRUE);

	MakeStartPosList(svrStartPos,world,cfg.defField);
	svrStartPos->SelectByString(cfg.defStartPosServer,YSTRUE);

	svrRecordFlight->SetCheck(cfg.recordWhenServerMode);
	svrFreeMemory->SetCheck(cfg.freeMemoryWhenPossibleServerMode);
	svrDisableChat->SetCheck(cfg.serverDisableChat);

	svrLogOnTimeOut->Select(YsBound((cfg.logOnTimeOut+14)/15,0,8));

	svrMaxMultiConnection->Select(cfg.multiConnLimit);

	if(cfg.serverResetTime==0)
	{
		svrResetInterval->Select(0);
	}
	else if(cfg.serverResetTime<180)
	{
		svrResetInterval->Select(1);
	}
	else if(cfg.serverResetTime<360)
	{
		svrResetInterval->Select(2);
	}
	else if(cfg.serverResetTime<720)
	{
		svrResetInterval->Select(3);
	}
	else if(cfg.serverResetTime<1440)
	{
		svrResetInterval->Select(4);
	}
	else if(cfg.serverResetTime<1440*3)
	{
		svrResetInterval->Select(5);
	}
	else if(cfg.serverResetTime<1440*7)
	{
		svrResetInterval->Select(6);
	}
	else
	{
		svrResetInterval->Select(7);
	}

	svrStopAfterNTimes->Select(cfg.endSvrAfterResetNTimes);
	svrSendWelcomeMessage->SetCheck(cfg.sendWelcomeMessage);

	svrIff->Select(cfg.defIFFWhenServer);
	svrGroundFire->SetCheck(cfg.groundFire);
	svrUseGuidedWeapon->SetCheck(cfg.useMissile);
	svrUseUnguidedWeapon->SetCheck(cfg.useUnguidedWeapon);
	svrDisableRadarGunSight->SetCheck(cfg.disableRadarGunSight);
	svrDisableThirdAirplaneView->SetCheck(cfg.disableThirdAirplaneView);
	svrNotifyTakeOffAndLeave->SetCheck(cfg.sendJoinLeaveMessage);


	if(cfg.serverControlShowUserName==1)
	{
		svrShowUserName->Select(0);
	}
	else if(cfg.serverControlShowUserName==2)
	{
		svrShowUserName->Select(1);
	}
	else if(cfg.serverControlShowUserName>=4000)
	{
		svrShowUserName->Select(4);
	}
	else if(cfg.serverControlShowUserName>=2000)
	{
		svrShowUserName->Select(3);
	}
	else if(cfg.serverControlShowUserName>=1000)
	{
		svrShowUserName->Select(2);
	}


	svrMidAirCollision->Select(cfg.serverControlMidAirCollision);
	svrBlackOut->Select(cfg.serverControlBlackOut);
	svrCanLandAnywhere->Select(cfg.serverControlCanLandAnywhere);

	if(cfg.serverControlRadarAlt==YSTRUE)
	{
		svrRadarAltLimit->Select(1);
	}
	else
	{
		svrRadarAltLimit->Select(0);
	}
}

void FsGuiNetConfigDialog::RetrieveConfig(FsNetConfig &cfg,const FsWorld *)
{
	YsString username;
	this->userName->GetText(username);

	cfg.portNumber=networkPort->GetInteger();
	strcpy(cfg.defUser,username);
	cfg.saveChatLog=saveChatLog->GetCheck();

	YsString clihostaddr;
	cliHostAddr->GetText(clihostaddr);
	strcpy(cfg.defHost,clihostaddr);

	{
		YsString stpSel;
		if(YSOK==cliStartPos->GetSelectedString(stpSel))
		{
			strcpy(cfg.defStartPosClient,stpSel);
		}
	}

	cfg.defIFFWhenClient=cliIff->GetSelection();
	cfg.recordWhenClientMode=cliRecordFlight->GetCheck();
	cfg.freeMemoryWhenPossibleClientMode=cliFreeMemory->GetCheck();

	{
		YsString fldSel;
		if(YSOK==svrField->GetSelectedString(fldSel))
		{
			strcpy(cfg.defField,fldSel);
		}
	}

	{
		YsString stpSel;
		if(YSOK==svrStartPos->GetSelectedString(stpSel))
		{
			strcpy(cfg.defStartPosServer,stpSel);
		}
	}

	cfg.recordWhenServerMode=svrRecordFlight->GetCheck();
	cfg.freeMemoryWhenPossibleServerMode=svrFreeMemory->GetCheck();
	cfg.serverDisableChat=svrDisableChat->GetCheck();

	cfg.logOnTimeOut=svrLogOnTimeOut->GetSelection()*15;

	cfg.multiConnLimit=svrMaxMultiConnection->GetSelection();


	switch(svrResetInterval->GetSelection())
	{
	case 0:
		cfg.serverResetTime=0;
		break;
	case 1:
		cfg.serverResetTime=60;
		break;
	case 2:
		cfg.serverResetTime=180;
		break;
	case 3:
		cfg.serverResetTime=360;
		break;
	case 4:
		cfg.serverResetTime=720;
		break;
	case 5:
		cfg.serverResetTime=1440;
		break;
	case 6:
		cfg.serverResetTime=1440*3;
		break;
	case 7:
		cfg.serverResetTime=1440*7;
		break;
	}


	cfg.endSvrAfterResetNTimes=svrStopAfterNTimes->GetSelection();
	cfg.sendWelcomeMessage=svrSendWelcomeMessage->GetCheck();

	cfg.defIFFWhenServer=svrIff->GetSelection();
	cfg.groundFire=svrGroundFire->GetCheck();
	cfg.useMissile=svrUseGuidedWeapon->GetCheck();
	cfg.useUnguidedWeapon=svrUseUnguidedWeapon->GetCheck();
	cfg.disableRadarGunSight=svrDisableRadarGunSight->GetCheck();
	cfg.disableThirdAirplaneView=svrDisableThirdAirplaneView->GetCheck();
	cfg.sendJoinLeaveMessage=svrNotifyTakeOffAndLeave->GetCheck();


	switch(svrShowUserName->GetSelection())
	{
	case 0:
		cfg.serverControlShowUserName=1;
		break;
	case 1:
		cfg.serverControlShowUserName=2;
		break;
	case 4:
		cfg.serverControlShowUserName=4000;
		break;
	case 3:
		cfg.serverControlShowUserName=2000;
		break;
	case 2:
		cfg.serverControlShowUserName=1000;
		break;
	}


	cfg.serverControlMidAirCollision=svrMidAirCollision->GetSelection();
	cfg.serverControlBlackOut=svrBlackOut->GetSelection();
	cfg.serverControlCanLandAnywhere=svrCanLandAnywhere->GetSelection();

	switch(svrRadarAltLimit->GetSelection())
	{
	case 1:
		cfg.serverControlRadarAlt=YSTRUE;
		break;
	case 0:
		cfg.serverControlRadarAlt=YSFALSE;
		break;
	}
}

void FsGuiNetConfigDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		FsNetConfig cfg;
		RetrieveConfig(cfg,world);
		cfg.Save(FsGetNetConfigFile());
		SaveBlockedIpList();
		CloseModalDialog(0);
	}
	else if(btn==cancelBtn)
	{
		CloseModalDialog(0);
	}
	else if(btn==setDefaultBtn)
	{
		FsNetConfig cfg;
		cfg.SetDefault();
		InitializeDialog(world,cfg);
	}
	else if(btn==svrDeleteBlockedIpAddr)
	{
		svrBlockedIpAddr->DeleteString(svrBlockedIpAddr->GetSelection());
	}
	else if(btn==svrAddBlockedIpAddr)
	{
		YsString blockedIp;
		svrBlockedIpAddrToAdd->GetText(blockedIp);
		if(0<blockedIp.Strlen())
		{
			svrBlockedIpAddr->AddString(blockedIp,YSTRUE);
			svrBlockedIpAddrToAdd->SetText("");
		}
	}
	else if(btn==svrConfigInterceptMission)
	{
	}
}

////////////////////////////////////////////////////////////

void FsGuiNetServerListOfflineDialog::Make(){
	Initialize();

	SetTopLeftCorner(32,32);
	AddStaticText(0, FSKEY_NULL,"Server list offline",YSTRUE);
	okBtn=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,"OK",YSTRUE);
	Fit();
}

void FsGuiNetServerListOfflineDialog::OnButtonClick(FsGuiButton *btn){
	if(btn==okBtn){
		CloseModalDialog(0);
	}
}

////////////////////////////////////////////////////////////

void FsGuiNetServerListDialog::Make(const std::vector<ServerListItem>& servers, YsArray <const char *> &fldList){
	this->servers= servers;
	this->fldList= fldList;
	
	GetUnicodeCharacterBitmapSize(wid,hei);
	
	Initialize();
	SetBackgroundColor(FsGuiObject::defTabBgCol);
	SetTextMessage("-- Server Browser --");
	YsArray <const char *> serverListBoxItems;
	
	green.SetIntRGB(34,120,57);
	red.SetIntRGB(120,34,34);

	// Iterate through the servers, and add buttons.
	for(int i =0; i < servers.size(); i++){
		const char *nameYs = servers[i].name.c_str();
		serverListBoxItems.Append(nameYs);
	}
	YsQuickSortString <int> (serverListBoxItems.GetN(), serverListBoxItems, NULL);

	serverList=AddListBox(MkId("serverList"),FSKEY_NULL,"Server list", serverListBoxItems.GetN(), serverListBoxItems, 10, boxSpacing * 2 - (defHSpaceUnit/hei),YSTRUE);

	AddStaticText(0,FSKEY_NULL,"",16,2,YSTRUE);

	serverNameLabel = AddStaticText(0,FSKEY_NULL,"Server Name",boxSpacing,2,YSTRUE);
	serverNameLabel->SetDrawFrame(YSTRUE);
	serverNameLabel->SetFill(YSTRUE);

	serverMapLabel = AddStaticText(0,FSKEY_NULL,"Map",boxSpacing,2,YSFALSE);
	serverMapLabel->SetDrawFrame(YSTRUE);
	serverMapLabel->SetFill(YSTRUE);

	serverName = AddStaticText(0,FSKEY_NULL,"",boxSpacing,2,YSTRUE);
	serverName->SetDrawFrame(YSTRUE);

	serverMap = AddStaticText(0,FSKEY_NULL,"",boxSpacing,2,YSFALSE);
	serverMap->SetDrawFrame(YSTRUE);

	serverPlayersLabel = AddStaticText(0,FSKEY_NULL,"Players",boxSpacing,2,YSTRUE);
	serverPlayersLabel->SetDrawFrame(YSTRUE);
	serverPlayersLabel->SetFill(YSTRUE);

	serverVersionLabel = AddStaticText(0,FSKEY_NULL,"Version",boxSpacing,2,YSFALSE);
	serverVersionLabel->SetDrawFrame(YSTRUE);
	serverVersionLabel->SetFill(YSTRUE);

	serverPlayers = AddStaticText(0,FSKEY_NULL,"",boxSpacing,2,YSTRUE);
	serverPlayers->SetDrawFrame(YSTRUE);

	serverVersion = AddStaticText(0,FSKEY_NULL,"",boxSpacing,2,YSFALSE);
	serverVersion->SetDrawFrame(YSTRUE);
		
	
	okBtn = AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);
	
	Fit();

}

void FsGuiNetServerListDialog::OnButtonClick(FsGuiButton *btn){
	int id = btn->GetIdent();	

	if(btn==cancelBtn){
		res = YSERR;
		CloseModalDialog(0);
	}
	else if(btn==okBtn){
		res = YSOK;
		CloseModalDialog(0);
	}

}

void FsGuiNetServerListDialog::OnListBoxSelChange(FsGuiListBox *lbx,int prevSel){
	YsString selectedName;
	
	if (YSOK== lbx->GetSelectedString(selectedName)){
		for(int i =0; i < servers.size(); i++){
			if(servers[i].name == selectedName.Txt()){
				selectedServer = servers[i];
				YsString serverNameStr = selectedServer.name.c_str();
				YsString serverMapStr = selectedServer.map.c_str();
				
				
				YsString serverPlayersStr = std::to_string(selectedServer.players).c_str();
				
				serverNameStr.resize(boxSpacing, ' ');
				serverMapStr.resize(boxSpacing-12, ' ');

				serverName->SetText(serverNameStr);
				serverName->wid=boxSpacing*wid+defHSpaceUnit;
				
				YSRESULT mapInstalled = YSERR;

				for (int i=0; i<fldList.GetN(); ++i)
				{
					if (0==strcmp(fldList[i],selectedServer.map.c_str()))
					{
						mapInstalled=YSOK;
						break;
					}
				}

				if (mapInstalled==YSOK){
					serverMap->SetFgColor(green);
					serverMap->SetText(serverMapStr + " ✓ - (Installed)");
				}
				else{
					serverMap->SetFgColor(red);
					serverMap->SetText(serverMapStr + " ✗ - (Not Installed)");
				}
				serverMap->wid=boxSpacing*wid+defHSpaceUnit;
				serverPlayers->SetText(serverPlayersStr);
				

				if (selectedServer.players==0){
					serverPlayers->SetFgColor(red);

				}
				else{
					serverPlayers->SetFgColor(green);

				
				}
				serverPlayers->wid=boxSpacing*wid+defHSpaceUnit;

				if(selectedServer.version == YSFLIGHT_NETVERSION){
					serverVersion->SetText("✓");
					serverVersion->SetFgColor(green);
				}
				else{
					serverVersion->SetText("✗");
					serverVersion->SetFgColor(red);
				}
				serverVersion->wid=boxSpacing*wid+defHSpaceUnit;
				break;
			}
		}

	}
}

ServerListItem FsGuiNetServerListDialog::GetSelectedServer(){
	return selectedServer;
}
