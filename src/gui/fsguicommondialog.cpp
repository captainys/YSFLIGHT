#include <ysport.h>

#include <fstextresource.h>
#include <fsconfig.h>
#include <fsoption.h>
#include <fsfilename.h>
#include <fsworld.h>

#include "fsguicommondialog.h"

#include "../../svnrevision/yssvnrevision.h"

void FsGuiConfirmDeleteFlightDialog::Make(int dialogIdent,int nextActionCode)
{
	Initialize();

	modalDialogIdent=dialogIdent;  // modalDialogIdent is a protected member of FsGuiDialog
	this->nextActionCode=nextActionCode;

	SetTransparency(YSFALSE);

	SetTopLeftCorner(16,16);
	SetTextMessage("");

	label=AddStaticText(0,FSKEY_NULL,FSGUI_DELFLTDLG_MESSAGE,24,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	yesBtn=AddTextButton(MakeIdent("ok"),FSKEY_Y,FSGUI_PUSHBUTTON,FSGUI_DELFLTDLG_DELETE,YSTRUE);

	noBtn=AddTextButton(MakeIdent("cancel"),FSKEY_N,FSGUI_PUSHBUTTON,FSGUI_DELFLTDLG_NODELETE,YSFALSE);

	whatToOpenNext=L"";

	Fit();
}

void FsGuiConfirmDeleteFlightDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==yesBtn)
	{
		res=YSOK;
		CloseModalDialog(nextActionCode);
	}
	else if(btn==noBtn)
	{
		res=YSERR;
		CloseModalDialog(0);
	}
}

////////////////////////////////////////////////////////////

void FsGuiNoJoystickWarningDialogClass::Make(int nextActionCode)
{
	Initialize();

	SetTopLeftCorner(32,32);

	this->nextActionCode=nextActionCode;

	SetIdent("noJoystickDialog");

	AddStaticText(0,FSKEY_NULL,FSGUI_NOJOYSTICK_MESSAGE,YSTRUE);
	dontShowItAgainBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NOJOYSTICK_DONTSHOWAGAIN,YSTRUE);
	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSTRUE);

	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiNoJoystickWarningDialogClass::OnKeyDown(int fsKey,YSBOOL /*shift*/,YSBOOL /*ctrl*/,YSBOOL /*alt*/)
{
	if(FSKEY_ESC==fsKey)
	{
		CloseModalDialog(nextActionCode);
	}
}

void FsGuiNoJoystickWarningDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(NULL!=btn && btn==okBtn)
	{
		if(YSTRUE==dontShowItAgainBtn->GetCheck())
		{
			FsFlightConfig cfg;
			cfg.Load(FsGetConfigFile());
			cfg.showJoystickUnpluggedWarning=YSFALSE;
			cfg.Save(FsGetConfigFile());
		}
		CloseModalDialog(nextActionCode);
	}
}


extern const char *FsProgramTitle;

void FsGuiAboutDialog::Make(void)
{
	SetTopLeftCorner(32,32);

	Initialize();

	SetTextMessage(FSGUI_ABOUTDLG_TITLE);

	AddStaticText(0,FSKEY_NULL,FsProgramTitle,32,1,YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_ABOUTDLG_VERSION,YSTRUE);
	YsString str;
	str.Printf(" %d (Rev.%s)",YSFLIGHT_VERSION,YsSvnRevisionString);
	AddStaticText(0,FSKEY_NULL,str,YSFALSE);

	AddStaticText(0,FSKEY_NULL,"By CaptainYS, and the YSFlight Community Edition Team",YSTRUE);
	AddStaticText(0,FSKEY_NULL,"Please visit YSFQ.COM for more information",48,1,YSTRUE);
	openURLBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Open YSFHQ.COM",YSTRUE);
	okBtn=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSFALSE);

	YsBitmap bmp;
	bmp.LoadPng(FSGUI_ABOUTDLG_BANNER);
	bannerBtn=AddBmpButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,bmp,L"YSFHQ.COM",YSTRUE);

	SetFocus(openURLBtn);

	Fit();

	SetTransparency(YSFALSE);
}

void FsGuiAboutDialog::OnKeyDown(int keyCode,YSBOOL /*shift*/,YSBOOL /*ctrl*/,YSBOOL /*alt*/)
{
	if(FSKEY_ESC==keyCode || FSKEY_ENTER==keyCode)
	{
		CloseModalDialog(0);
	}
}

void FsGuiAboutDialog::OnButtonClick(FsGuiButton *btn)
{
	if(NULL!=btn)
	{
		if(btn==okBtn)
		{
			CloseModalDialog(0);
		}
		else if(btn==openURLBtn || btn==bannerBtn)
		{
		    YsOpenURL("https://ysfhq.com/");
		}
	}
}

////////////////////////////////////////////////////////////

void FsGuiFirstDialogClass::Make(int nextActionCode)
{
	Initialize();

	this->nextActionCode=nextActionCode;

	SetTopLeftCorner(8,8);

	AddStaticText(0,FSKEY_NULL,FSGUI_FIRSTDLG_1STLINE,67,2,YSTRUE);

	okBtn1=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_FIRSTDLG_2NDLINE,57,2,YSTRUE);

	ysflightComBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"https://forum.ysfhq.com/",YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_FIRSTDLG_3RDLINE,60,2,YSTRUE);

	downloadPageBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"http://download.en.ysflight.com",YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_FIRSTDLG_4THLINE,32,2,YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_FIRSTDLG_5THLINE,20,2,YSTRUE);

	okBtn2=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSTRUE);

	YsBitmap bmp;
	bmp.LoadPng(FSGUI_ABOUTDLG_BANNER);
	bannerBtn=AddBmpButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,bmp,L"YSFLIGHT.COM",YSFALSE);

	SetFocus(ysflightComBtn);

	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiFirstDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(NULL!=btn)
	{
		if(okBtn1==btn || okBtn2==btn)
		{
			CloseModalDialog(nextActionCode);
		}
		else if(btn==ysflightComBtn || btn==bannerBtn)
		{
		    YsOpenURL("http://www.ysflight.com");
		}
		else if(btn==downloadPageBtn)
		{
			if(0==strcmp(FsOption::GetLanguageString(),FsJapaneseLanguageCode))
			{
				YsOpenURL("http://download.jp.ysflight.com");
			}
			else
			{
				YsOpenURL("http://download.en.ysflight.com");
			}
		}
	}
}

#ifdef _WIN32
extern void FsWin32InstallJWordPlugin(void);
extern void FsWin32DeleteJWordPlugin(void);

void FsGuiAskInstallJWordOnFirstStart::Make(void)
{
	Initialize();

	SetTopLeftCorner(24,24);
	if(0==strcmp(FsOption::GetLanguageString(),FsJapaneseLanguageCode))
	{
		AddStaticText(0,FSKEY_NULL,FSGUI_INSTALLJWORD_LINE1,YSTRUE);
		installBtn=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_INSTALLJWORD_INSTALL,YSTRUE);
		explainJWordBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_INSTALLJWORD_EXPLAIN,YSFALSE);
		closeBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_INSTALLJWORD_DONTINSTALL,YSTRUE);
		AddStaticText(0,FSKEY_NULL,FSGUI_INSTALLJWORD_COMMENT,YSTRUE);
		AddStaticText(0,FSKEY_NULL,FSGUI_INSTALLJWORD_FIRSTSTART,YSTRUE);

		SetFocus(installBtn);

		explainJWordDlg.Make();
	}
	else
	{
		AddStaticText(0,FSKEY_NULL,FSGUI_INSTALLJWORD_LINE1,YSTRUE);
		AddStaticText(0,FSKEY_NULL,FSGUI_INSTALLJWORD_LINE2,YSTRUE);
		AddStaticText(0,FSKEY_NULL,FSGUI_INSTALLJWORD_LINE3,YSTRUE);
		AddStaticText(0,FSKEY_NULL,FSGUI_INSTALLJWORD_LINE4,YSTRUE);

		deleteBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_INSTALLJWORD_DELETE,YSTRUE);
		closeBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSFALSE);

		SetFocus(deleteBtn);
	}

	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiAskInstallJWordOnFirstStart::OnButtonClick(FsGuiButton *btn)
{
	if(NULL!=btn)
	{
		if(btn==closeBtn)
		{
			CloseModalDialog(0);
		}
		else if(btn==installBtn)
		{
			FsWin32InstallJWordPlugin();
			CloseModalDialog(0);
		}
		else if(btn==explainJWordBtn)
		{
			AttachModalDialog(&explainJWordDlg);
		}
		else if(btn==deleteBtn)
		{
			FsWin32DeleteJWordPlugin();
			CloseModalDialog(0);
		}
	}
}

void FsGuiExplainJWordDialog::Make(void)
{
	Initialize();

	SetTopLeftCorner(16,16);

	AddStaticText(0,FSKEY_NULL,FSGUI_EXPLAINJWORD_LINE1,YSTRUE);
	AddStaticText(0,FSKEY_NULL,FSGUI_EXPLAINJWORD_LINE2,YSTRUE);
	AddStaticText(0,FSKEY_NULL,FSGUI_EXPLAINJWORD_LINE3,YSTRUE);
	AddStaticText(0,FSKEY_NULL,FSGUI_EXPLAINJWORD_LINE4,YSTRUE);
	AddStaticText(0,FSKEY_NULL,FSGUI_EXPLAINJWORD_LINE5,YSTRUE);
	urlBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,L"http://www.jword.jp/intro",YSFALSE);
	okBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSTRUE);

	SetFocus(okBtn);
	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiExplainJWordDialog::OnButtonClick(FsGuiButton *btn)
{
	if(NULL!=btn)
	{
		if(btn==urlBtn)
		{
			YsOpenURL("http://www.jword.jp/intro");
		}
		else if(btn==okBtn)
		{
			CloseModalDialog(0);
		}
	}
}
#endif

void FsGuiSupportYsflightDialogClass::Make(YSBOOL firstStart,int nextActionCode)
{
	Initialize();

	this->nextActionCode=nextActionCode;

	this->firstStart=firstStart;
	needClose=YSFALSE;

	SetTopLeftCorner(8,8);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_1STLINE,YSTRUE);

	okBtn1=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_2NDLINE,YSTRUE);

	AddStaticText(0,FSKEY_NULL,"",1,1,YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_3RDLINE,YSTRUE);

	AddStaticText(0,FSKEY_NULL,"        ",YSTRUE);
	downloadPageBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"http://download.en.ysflight.com",YSFALSE);

	AddStaticText(0,FSKEY_NULL,"",1,1,YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_4THLINE1,YSTRUE);

	AddStaticText(0,FSKEY_NULL,"        ",YSTRUE);
	ysflightComBtn1=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"http://www.ysflight.com",YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_4THLINE2,YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_4THLINE3,YSTRUE);

	AddStaticText(0,FSKEY_NULL,"",1,1,YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_5THLINE1,YSTRUE);

	ysflightComBtn2=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"http://www.ysflight.com",YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_5THLINE2,1,1,YSFALSE);

	AddStaticText(0,FSKEY_NULL,FSGUI_SUPPORTYSFLIGHTDLG_6THLINE,54,1,YSTRUE);

	okBtn2=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSTRUE);

	SetFocus(ysflightComBtn1);
	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiSupportYsflightDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(NULL!=btn)
	{
		if(NULL!=btn && (btn==okBtn1 || btn==okBtn2))
		{
			CloseModalDialog(nextActionCode);
			needClose=YSTRUE;
		}
		else if(btn==ysflightComBtn1 || btn==ysflightComBtn2)
		{
		    YsOpenURL("http://www.ysflight.com");
		}
		else if(btn==downloadPageBtn)
		{
			if(0==strcmp(FsOption::GetLanguageString(),FsJapaneseLanguageCode))
			{
				YsOpenURL("http://download.jp.ysflight.com");
			}
			else
			{
				YsOpenURL("http://download.en.ysflight.com");
			}
		}
	}
}

void FsGuiSupportYsflightDialogClass::OnModalDialogClosed(int,class FsGuiDialog *closedModalDialog,int)
{
}

void FsGuiVoteYsflightDialogClass::Make(void)
{
	SetTopLeftCorner(8,8);

	AddStaticText(0,FSKEY_NULL,FSGUI_VOTEYSFLIGHTDLG_MESSAGE,YSTRUE);

	votePageBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,
	    "http://www.vector.co.jp/award/vote.html?no=se121250&vasts=vote",YSTRUE);


	okBtn=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSTRUE);


	SetFocus(votePageBtn);
	Fit();
	SetTransparency(YSFALSE);
}

////////////////////////////////////////////////////////////

void FsGuiAirCombatDialog::Make(FsWorld *world)
{
	FsGuiDialog::Initialize();

	this->world=world;

	SetTextMessage("-- Create Air Combat --");

	int i;
	YsArray <const char *> nameLst;
	const char *name;

	for(i=0; (name=world->GetFieldTemplateName(i))!=NULL; i++)
	{
		nameLst.Append(name);
	}

	AddStaticText(0,FSKEY_NULL,"Field",7,1,YSTRUE);
	fieldList=AddDropList(0,FSKEY_NULL,"",nameLst.GetN(),nameLst,16,24,32,YSFALSE);

	nameLst.Set(0,NULL);
	nameLst.Append("NONE");
	for(i=0; (name=world->GetAirplaneTemplateName(i))!=NULL; i++)
	{
		nameLst.Append(name);
	}
	AddStaticText(0,FSKEY_NULL,"Player",10,1,YSTRUE);
	playerAir=AddDropList(0,FSKEY_NULL,"",nameLst.GetN(),nameLst,16,24,32,YSFALSE);
	playerStp=AddEmptyDropList(0,FSKEY_NULL,"",16,24,32,YSFALSE);


	const char *const levelStr[]={"NOVICE","VETERAN","ACE","DEADLY","LETHAL"};

	for(i=0; i<MAX_NUM_WINGMAN; i++)
	{
		char label[256];
		sprintf(label,"Wingman%d",i+1);
		AddStaticText(0,FSKEY_NULL,label,10,1,YSTRUE);
		wingmanAir[i]=AddDropList(0,FSKEY_NULL,"",nameLst.GetN(),nameLst,16,24,32,YSFALSE);
		wingmanStp[i]=AddEmptyDropList(0,FSKEY_NULL,"",16,24,32,YSFALSE);
		wingmanLevel[i]=AddDropList(0,FSKEY_NULL,"",5,levelStr,16,24,32,YSFALSE);
		wingmanLevel[i]->Select(2);
	}


	for(i=0; i<MAX_NUM_ENEMY; i++)
	{
		char label[256];
		sprintf(label,"Enemy%d",i+1);
		AddStaticText(0,FSKEY_NULL,label,10,1,YSTRUE);
		enemyAir[i]=AddDropList(0,FSKEY_NULL,"",nameLst.GetN(),nameLst,16,24,32,YSFALSE);
		enemyStp[i]=AddEmptyDropList(0,FSKEY_NULL,"",16,24,32,YSFALSE);
		enemyLevel[i]=AddDropList(0,FSKEY_NULL,"",5,levelStr,16,24,32,YSFALSE);
		enemyLevel[i]->Select(2);
	}

	useMissileBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,"Use Missile",YSTRUE);
	useMissileBtn->SetCheck(YSTRUE);

	okBtn=AddTextButton(MakeIdent("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,"OK",YSTRUE);
	cancelBtn=AddTextButton(MakeIdent("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,"Cancel",YSFALSE);
	flyNowBtn=AddTextButton(MakeIdent("flynow"),FSKEY_ESC,FSGUI_PUSHBUTTON,"Fly Now!",YSFALSE);

	Fit();
}

void FsGuiAirCombatDialog::Initialize(FsWorld *world,FsFlightConfig &cfg)
{
	fieldList->SelectByString(cfg.defField);
	ResetStartPos(world);

	playerAir->SelectByString(cfg.defAirplane);

	enemyAir[0]->SelectByString("SU-27_FLANKER");
	enemyAir[1]->SelectByString("MIG-29_FULCRUM");
}

void FsGuiAirCombatDialog::ResetStartPos(FsWorld *world)
{
	YsString fieldName;
	fieldList->GetSelectedString(fieldName);
	if(0<fieldName.Strlen())
	{
		ReloadField(world,fieldName);

		int i,selNorth,selSouth;
		YsString stp;
		YsArray <YsString> stpList;

		selNorth=-1;
		selSouth=-1;
		for(i=0; world->GetFieldStartPositionName(stp,fieldName,i)==YSOK; i++)
		{
			stpList.Append(stp);

			if(selNorth<0 && strncmp(stp,"NORTH",5)==0)
			{
				selNorth=i;
			}
			if(selSouth<0 && strncmp(stp,"SOUTH",5)==0)
			{
				selSouth=i;
			}
		}

		playerStp->SetChoice(stpList.GetN(),stpList);
		for(i=0; i<MAX_NUM_WINGMAN; i++)
		{
			wingmanStp[i]->SetChoice(stpList.GetN(),stpList);
		}
		for(i=0; i<MAX_NUM_ENEMY; i++)
		{
			enemyStp[i]->SetChoice(stpList.GetN(),stpList);
		}


		if(selNorth>=0)
		{
			playerStp->Select(selNorth);
			for(i=0; i<MAX_NUM_WINGMAN; i++)
			{
				wingmanStp[i]->Select(selNorth+i+1);
			}
		}

		if(selSouth>=0)
		{
			for(i=0; i<MAX_NUM_ENEMY; i++)
			{
				enemyStp[i]->Select(selSouth+i);
			}
		}
	}

	ReloadPlayerStartPos(world);
}

void FsGuiAirCombatDialog::ReloadPlayerStartPos(FsWorld *world)
{
	YsString fieldName,stpName;
	fieldList->GetSelectedString(fieldName);
	playerStp->GetSelectedString(stpName);

	if(0<fieldName.Strlen() && 0<stpName.Strlen())
	{
		ReloadStartPos(world,fieldName,stpName);
	}
}

void FsGuiAirCombatDialog::DrawSelectedFieldAndAircraft(void)
{
	DrawField(world,YSTRUE,YSTRUE,YSFALSE,YsOrigin());

	YsString typeName;
	if(YSOK==playerAir->GetSelectedString(typeName))
	{
		YsAtt3 att;
		att.Set(YsPi/2.0,0.0,YsPi/2.0);
		DrawAirplane(world,typeName,att,0,NULL,YSTRUE);
	}
}

YSRESULT FsGuiAirCombatDialog::CreateFlight(void)
{
	int i,j;
	FsGuiDropList *airList[1+MAX_NUM_WINGMAN+MAX_NUM_ENEMY];
	FsGuiDropList *posList[1+MAX_NUM_WINGMAN+MAX_NUM_ENEMY];
	FsGuiDropList *lvlList[1+MAX_NUM_WINGMAN+MAX_NUM_ENEMY];
	FSIFF iffList[1+MAX_NUM_WINGMAN+MAX_NUM_ENEMY];

	j=0;
	airList[j]=playerAir;
	lvlList[j]=NULL;
	iffList[j]=FS_IFF0;
	posList[j++]=playerStp;
	for(i=0; i<MAX_NUM_WINGMAN; i++)
	{
		airList[j]=wingmanAir[i];
		lvlList[j]=wingmanLevel[i];
		iffList[j]=FS_IFF0;
		posList[j++]=wingmanStp[i];
	}
	for(i=0; i<MAX_NUM_ENEMY; i++)
	{
		airList[j]=enemyAir[i];
		lvlList[j]=enemyLevel[i];
		iffList[j]=FS_IFF1;
		posList[j++]=enemyStp[i];
	}

	double gLimit[]=
	{
		 3.0,
		 5.0,
		 7.0,
		 9.0,
		11.0
	};
	YsString air1,air2;
	YsString pos1,pos2;
	double holdPosition[][3]=
	{
		{-4000.0, 3000.0,-4000.0},
		{    0.0, 4000.0,-4000.0},
		{ 4000.0, 5000.0,-4000.0},
		{-4000.0, 5000.0,    0.0},
		{    0.0, 4000.0,    0.0},
		{ 4000.0, 3000.0,    0.0},
		{-4000.0, 3000.0, 4000.0},
		{    0.0, 4000.0, 4000.0},
		{ 4000.0, 5000.0, 4000.0},
		{-4000.0, 5000.0, 8000.0},
		{    0.0, 4000.0, 8000.0},
		{ 4000.0, 3000.0, 8000.0},
	};

	for(i=0; i<1+MAX_NUM_WINGMAN+MAX_NUM_ENEMY; i++)
	{
		airList[i]->GetSelectedString(air1);
		if(0<air1.Strlen() && strcmp(air1,"NONE")!=0)
		{
			posList[i]->GetSelectedString(pos1);
			if(0==pos1.Strlen() || strcmp(pos1,"NONE")==0)
			{
				errCode=ACMDLG_ERROR_STARTPOS_UNSELECTED;
				return YSERR;
			}

			for(j=i+1; j<1+MAX_NUM_WINGMAN+MAX_NUM_ENEMY; j++)
			{
				airList[j]->GetSelectedString(air2);
				posList[j]->GetSelectedString(pos2);

				if(0<air2.Strlen() && strcmp(air2,"NONE")!=0 && 0<pos2.Strlen())
				{
					if(strcmp(pos1,pos2)==0)
					{
						errCode=ACMDLG_ERROR_STARTPOS_COLLISION;
						return YSERR;
					}
				}
			}
		}
	}


	world->TerminateSimulation();
	world->PrepareSimulation();
	FsAirplane *air;
	int prevEnemyKey;
	FsDogfight *prevEnemyDf;
	YsString fld,pln,pos;
	int lvl;

	fieldList->GetSelectedString(fld);
	world->AddField(NULL,fld,YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0));
	prevEnemyDf=NULL;
	prevEnemyKey=-1;

	playerAir->GetSelectedString(pln);
	playerStp->GetSelectedString(pos);
	air=world->AddAirplane(pln,YSTRUE);
	if(NULL==air)
	{
		errCode=ACMDLG_ERROR_PLAYER_NOTSELECTED;
		return YSERR;
	}

	air->iff=iffList[0];
	world->SettleAirplane(*air,pos);

	for(i=1; i<1+MAX_NUM_WINGMAN+MAX_NUM_ENEMY; i++)
	{
		airList[i]->GetSelectedString(pln);
		posList[i]->GetSelectedString(pos);
		lvl=lvlList[i]->GetSelection();

		if(0<pln.Strlen() && strcmp(pln,"NONE")!=0 && 0<pos.Strlen() && strcmp(pos,"NONE")!=0)
		{
			air=world->AddAirplane(pln,YSFALSE);
			air->iff=iffList[i];
			world->SettleAirplane(*air,pos);

			if(air->Prop().GetAirplaneCategory()==FSAC_AEROBATIC ||
			   air->Prop().GetAirplaneCategory()==FSAC_FIGHTER ||
			   air->Prop().GetAirplaneCategory()==FSAC_ATTACKER ||
			   air->Prop().GetAirplaneCategory()==FSAC_TRAINER ||
			   air->Prop().GetAirplaneCategory()==FSAC_WW2FIGHTER)
			{
				FsDogfight *df;
				df=FsDogfight::Create();
				df->gLimit=gLimit[lvl];
				if(air->Prop().IsJet()!=YSTRUE)
				{
					df->gLimit=1.0+(df->gLimit-1.0)/1.6;
				}
				df->minAlt=1000.0;
				air->SetAutopilot(df);
				air->gLimit=gLimit[lvl];

				if(iffList[i]!=FS_IFF0)
				{
					if(prevEnemyDf!=NULL)
					{
						prevEnemyDf->wingmanAirplaneKey=FsExistence::GetSearchKey(air);
						df->wingmanAirplaneKey=prevEnemyKey;
						prevEnemyDf=NULL;
					}
					else
					{
						prevEnemyDf=df;
						prevEnemyKey=FsExistence::GetSearchKey(air);
					}
				}
			}
			else
			{
				YsVec3 pos;
				pos=air->GetPosition();
				pos.SetY(holdPosition[i-1][1]);

				// air->Prop().SetPosition(pos);

				FsGotoPosition *gp;
				gp=FsGotoPosition::Create();
				gp->SetSingleDestination(pos);
				gp->minAlt=2000.0;
				gp->speed=air->Prop().GetEstimatedCruiseSpeed();
				air->SetAutopilot(gp);
			}
		}
	}

	if(useMissileBtn->GetCheck()==YSTRUE)
	{
		world->AllowAAM(YSTRUE);
	}
	else
	{
		world->AllowAAM(YSFALSE);
	}

	errCode=ACMDLG_ERROR_NOERROR;
	return YSOK;
}

void FsGuiAirCombatDialog::OnDropListSelChange(FsGuiDropList *drp,int prevSel)
{
	prevSel;

	if(drp==fieldList)
	{
		ResetStartPos(world);
	}
	if(drp==playerStp)
	{
		ReloadPlayerStartPos(world);
	}
}

void FsGuiAirCombatDialog::OnButtonClick(FsGuiButton *btn)
{
	errCode=ACMDLG_ERROR_NOERROR;
	if(btn==okBtn)
	{
		flyNow=YSFALSE;
		res=CreateFlight();
		CloseModalDialog((int)res);
	}
	else if(btn==flyNowBtn)
	{
		flyNow=YSTRUE;
		res=CreateFlight();
		CloseModalDialog((int)res);
	}
	else if(btn==cancelBtn)
	{
		res=YSERR;
		CloseModalDialog((int)res);
	}
}

////////////////////////////////////////////////////////////

void FsGuiSelectMissionDialog::Make(const YsWString &subDir,YsArray <YsWString> &fileList)
{
	SetTopLeftCorner(0,0);
	SetSize(800,400);

	SetTextMessage(FSGUI_COMMON_MISSION);

	missionFile.CleanUp();

	YsArray <YsWString> titleArray;
	for(auto &file : fileList)
	{
		titleArray.Append(TryToGetMissionTitle(subDir,file));
		missionFile.Append(file);
	}
	YsQuickSortString <YsWString,YsWString> (titleArray.GetN(),titleArray.GetEditableArray(),missionFile.GetEditableArray());

	YsArray <const wchar_t *> titlePtr;
	for(auto &title : titleArray)
	{
		titlePtr.Append(title.Txt());
	}

	missionList=AddListBox(0,FSKEY_NULL,"Mission",titlePtr.GetN(),titlePtr,16,48,YSTRUE);
	okBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);
	flyNowBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NEWFLTDLG_FLYNOW,YSTRUE);

	lastClicked=NULL;

	Fit();
	SetTransparency(YSFALSE);
}

YsWString FsGuiSelectMissionDialog::TryToGetMissionTitle(const YsWString &subDir,const YsWString &file) const
{
	YsWString ful,title=file;;
	ful.MakeFullPathName(subDir,file);

	FILE *fp=YsFileIO::Fopen(ful,"r");
	if(NULL!=fp)
	{
		for(int i=0; i<20; ++i) // Read only up to first 20 lines.
		{
			YsString str;
			if(NULL!=str.Fgets(fp))
			{
				YsArray <YsString> args;
				str.Arguments(args);
				if(0<args.GetN() && 0==strcmp("SIMTITLE",args[0]))
				{
					title.SetUTF8String(args[1]);
					break;
				}
			}
		}
		fclose(fp);
	}
	return title;
}

void FsGuiSelectMissionDialog::OnButtonClick(FsGuiButton *btn)
{
	lastClicked=btn;
	if(okBtn==btn || cancelBtn==btn || flyNowBtn==btn)
	{
		CloseModalDialog(0);
	}
}

YSRESULT FsGuiSelectMissionDialog::GetSelectedMission(YsWString &selMissionFile) const
{
	auto sel=missionList->GetSelection();
	if(YSTRUE==missionFile.IsInRange(sel))
	{
		selMissionFile=missionFile[sel];
		return YSOK;
	}
	return YSERR;
}
