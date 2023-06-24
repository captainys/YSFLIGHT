#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include <ysclass.h>
#include <ysport.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fsconfig.h"

#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"

#include "graphics/common/fsopengl.h"
#include "fswirefont.h"

#include "ysunitconv.h"


#ifdef WIN32
#include <float.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <ysbitmap.h>

#include "fsgui.h"
#include <fsguifiledialog.h>
#include "fsguiselectiondialogbase.h"
#include "fschoose.h"

#include "fstextresource.h"

#include "fsguinewflightdialog.h"


FsNewFlightDialogOption::FsNewFlightDialogOption()
{
	canSelectField=YSTRUE;
	canSelectStartPosition=YSTRUE;
	canSelectLoading=YSTRUE;
	canSelectEnvironment=YSTRUE;
	canSelectWingmen=YSTRUE;
	canChooseNight=YSTRUE;
	canChooseFomType=YSTRUE;
	flyNowButton=YSTRUE;
	forRacingMode=YSFALSE;
	nextStartPos=YSFALSE;
}


////////////////////////////////////////////////////////////



FsNewSimulationDialogTemplate::FsNewSimulationDialogTemplate()
{
	world=NULL;

	fieldLabel=NULL;
	field=NULL;

	addComputerAircraft=NULL;

	specifyEnvironment=NULL;

	dayNight[0]=NULL;
	dayNight[1]=NULL;
	windDir=NULL;
	windSpd=NULL;
	visibility=NULL;
}

void FsNewSimulationDialogTemplate::AddFieldSelector(YSBOOL forRacingMode)
{
	fieldLabel=AddStaticText(1,FSKEY_NULL,FSGUI_COMMON_FIELD,YSTRUE);
	fieldLabel->SetDrawFrame(YSFALSE);
	fieldLabel->SetFill(YSFALSE);

	field=AddEmptyListBox(MkId("fieldList"),FSKEY_NULL,"",6,32,YSTRUE);
	fieldSearch=AddTextBox(MkId("fieldSearch"),FSKEY_NULL,"Search","",32,YSTRUE);

	PopulateFieldList(world,field,"",forRacingMode);

	addComputerAircraft=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NEWFLTDLG_ADDCOMPUTERAIRCRAFT,YSTRUE);
	addComputerAircraft->SetCheck(YSTRUE);
}

void FsNewSimulationDialogTemplate::AddDayNightSelector(void)
{
	if(NULL==specifyEnvironment)
	{
		specifyEnvironment=AddTextButton(1,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NEWFLTDLG_SPECIFYENVIRON,YSTRUE);
	}

	dayNight[0]=AddTextButton(1,FSKEY_NULL,FSGUI_RADIOBUTTON,FSGUI_NEWFLTDLG_DAY  ,YSTRUE);
	dayNight[1]=AddTextButton(1,FSKEY_NULL,FSGUI_RADIOBUTTON,FSGUI_NEWFLTDLG_NIGHT,YSFALSE);
	SetRadioButtonGroup(2,dayNight);
}

void FsNewSimulationDialogTemplate::AddWeatherSelector(void)
{
	if(NULL==specifyEnvironment)
	{
		specifyEnvironment=AddTextButton(1,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NEWFLTDLG_SPECIFYENVIRON,YSTRUE);
	}

	AddStaticText(1,FSKEY_NULL,FSGUI_NEWFLTDLG_WEATHER,YSTRUE);
	windDir=AddNumberBox(1,FSKEY_NULL,FSGUI_NEWFLTDLG_WINDDIR,16,0,-360,360,10,YSTRUE);
	windSpd=AddNumberBox(1,FSKEY_NULL,FSGUI_NEWFLTDLG_WINDSPD,16,0,0,50,1,YSTRUE);
	visibility=AddNumberBox(1,FSKEY_NULL,FSGUI_NEWFLTDLG_VISIBILITY,16,20,0,20,1,YSTRUE);


	AddStaticText(1,FSKEY_NULL,FSGUI_NEWFLTDLG_OVERCASTLAYER,YSTRUE);
	for(int i=0; i<MAXNUMCLOUDLAYER; i++)
	{
		overCastLayerSw[i]=AddTextButton(1,FSKEY_NULL,FSGUI_CHECKBOX,"",YSTRUE);
		AddStaticText(1,FSKEY_NULL,FSGUI_NEWFLTDLG_OVERCASTFLOOR,YSFALSE);
		overCastLayerFloor[i]=AddNumberBox(1,FSKEY_NULL,"",6,0,0,25000,200,YSFALSE);
		overCastLayerFloor[i]->SetNumber(800+i*3000);
		AddStaticText(1,FSKEY_NULL,FSGUI_NEWFLTDLG_OVERCASTTHICKNESS,YSFALSE);
		overCastLayerThickness[i]=AddNumberBox(1,FSKEY_NULL,"",6,0,0,5000,100,YSFALSE);
		overCastLayerThickness[i]->SetNumber(300);
	}
}

void FsNewSimulationDialogTemplate::ResetStpListBySearchKeyword(FsWorld *world,FsGuiDropList *stpList,FsGuiTextBox *stpSearch)
{
	YsString fieldName;
	if(nullptr!=field)
	{
		fieldName=field->GetSelectedString();

		YsString curSel;
		if(nullptr!=stpList)
		{
			curSel=stpList->GetSelectedString();
		}

		YsArray <YsString,16> searchKeyWord;
		if(nullptr!=stpSearch)
		{
			YsString str=stpSearch->GetString();
			if(0<str.Strlen())
			{
				str.Capitalize();
				str.Arguments(searchKeyWord);
			}
		}


		YsString stp;
		YsArray <YsString> stpCandidate;
		for(int i=0; world->GetFieldStartPositionName(stp,fieldName,i)==YSOK; i++)
		{
			for(auto kw : searchKeyWord)
			{
				if(YSTRUE!=stp.FINDWORD(nullptr,kw))
				{
					goto SKIP;
				}
			}
			stpCandidate.Append(stp);

		SKIP:
			;
		}

		if(0<stpCandidate.GetN())
		{
			stpList->SetChoice(stpCandidate);
			stpList->SelectByString(curSel);
			if(0>stpList->GetSelection())
			{
				stpList->Select(0);
			}
		}
	}
}

void FsNewSimulationDialogTemplate::OnListBoxSelChange(FsGuiListBox *lbx,int /*prevSel*/)
{
	if(lbx==field)
	{
		FieldSelectionChanged();
	}
}

void FsNewSimulationDialogTemplate::OnButtonClick(FsGuiButton *btn)
{
	YSBOOL envButton=YSFALSE;

	if(btn==dayNight[0] || btn==dayNight[1])
	{
		envButton=YSTRUE;
	}

	for(int i=0; i<MAXNUMCLOUDLAYER; i++)
	{
		if(btn==overCastLayerSw[i])
		{
			envButton=YSTRUE;
		}
	}
	
	if(YSTRUE==envButton && NULL!=specifyEnvironment)
	{
		specifyEnvironment->SetCheck(YSTRUE);
	}
}

void FsNewSimulationDialogTemplate::OnNumberBoxChange(FsGuiNumberBox *nbx,int /*prevNum*/)
{
	YSBOOL envButton=YSFALSE;

	if(nbx==windDir ||
	   nbx==windSpd ||
	   nbx==visibility)
	{
		envButton=YSTRUE;
	}

	enum
	{
		MAXNUMCLOUDLAYER=3
	};

	for(int i=0; i<MAXNUMCLOUDLAYER; i++)
	{
		if(nbx==overCastLayerFloor[i] || nbx==overCastLayerThickness[i])
		{
			envButton=YSTRUE;
		}
	}
	
	if(YSTRUE==envButton && NULL!=specifyEnvironment)
	{
		specifyEnvironment->SetCheck(YSTRUE);
	}
}



////////////////////////////////////////////////////////////


FsGuiNewFlightDialogClass::FsGuiNewFlightDialogClass()
{
	chooseAircraftDialog=new FsGuiChooseAircraft;
}

FsGuiNewFlightDialogClass::FsGuiNewFlightDialogClass(YSBOOL canSelectWingmen,YSBOOL canChooseNight,YSBOOL canChooseFomType,YSBOOL canChooseFlyNow)
{
	chooseAircraftDialog=new FsGuiChooseAircraft;

	FsNewFlightDialogOption opt;
	opt.canSelectWingmen=canSelectWingmen;
	opt.canChooseNight=canChooseNight;
	opt.canChooseFomType=canChooseFomType;
	opt.flyNowButton=canChooseFlyNow;
	InitialSetUp(opt);
}

FsGuiNewFlightDialogClass::FsGuiNewFlightDialogClass(const FsNewFlightDialogOption &opt)
{
	chooseAircraftDialog=new FsGuiChooseAircraft;

	InitialSetUp(opt);
}

FsGuiNewFlightDialogClass::~FsGuiNewFlightDialogClass()
{
	if(NULL!=chooseAircraftDialog)
	{
		delete chooseAircraftDialog;
		chooseAircraftDialog=NULL;
	}
}

void FsGuiNewFlightDialogClass::InitialSetUp(const FsNewFlightDialogOption &opt)
{
	this->option=opt;

	playerAir=NULL;
	changePlayerAir=NULL;

	playerStpLabel=NULL;
	playerStp=NULL;
	playerStpFilter=nullptr;

	for(int i=0; i<MAXNUMWINGMAN; i++)
	{
		wingmanAir[i]=NULL;
		wingmanStpLabel[i]=NULL;
		wingmanStp[i]=NULL;
		changeWingman[i]=NULL;
		removeWingman[i]=NULL;
	}

	okBtn=NULL;
	cancelBtn=NULL;
	flyNowButton=NULL;

	fomType=NULL;

	info.Initialize();
}

void FsGuiNewFlightDialogClass::Make(FsWorld *world)
{
	int i;
	FsGuiGroupBox *grpBox;

	this->world=world;

	// ? FsGuiDialog dlg;
	SetTopLeftCorner(0,0);

	int wid,hei;
	FsGetWindowSize(wid,hei);

	SetSize(wid,hei);

	autoNewColumn=YSFALSE;


	SetTextMessage(FSGUI_NEWFLTDLG_CREATENEWFLIGHT);


	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);

	int tabId;
	tabId=AddTab(mainTab,FSGUI_COMMON_GENERAL);
	BeginAddTabItem(mainTab,tabId);
	{
		AddFieldSelector(option.forRacingMode);
		if(YSTRUE!=option.canSelectField)
		{
			field->Disable();
			fieldSearch->Disable();
		}

		playerAir=AddStaticText(1,FSKEY_NULL,"Airplane:",32,1,YSTRUE);
		playerAir->SetDrawFrame(YSFALSE);
		playerAir->SetFill(YSFALSE);
		changePlayerAir=AddTextButton(MkId("playerAirBtn"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NEWFLTDLG_SELAIR,YSTRUE);

		playerStpLabel=AddStaticText(1,FSKEY_NULL,FSGUI_COMMON_STP,YSTRUE);
		playerStpLabel->SetDrawFrame(YSFALSE);
		playerStpLabel->SetFill(YSFALSE);
		playerStp=AddEmptyDropList(MkId("playerStpList"),FSKEY_NULL,"",16,16,32,YSFALSE);
		playerStpFilter=AddTextBox(0,FSKEY_NULL,"","",32,YSTRUE);

		if(YSTRUE!=option.canSelectStartPosition)
		{
			playerStp->Disable();
			playerStpFilter->Disable();
		}
	}
	EndAddTabItem();



	if(option.canSelectWingmen==YSTRUE)
	{
		tabId=AddTab(mainTab,FSGUI_NEWFLTDLG_WINGMAN);
		BeginAddTabItem(mainTab,tabId);
		{
			for(i=0; i<MAXNUMWINGMAN; i++)
			{
				YsString buf,uitag;

				buf.Printf("Wingman %d:",i+1);
				wingmanAir[i]=AddStaticText(1,FSKEY_NULL,buf,24,1,YSTRUE);
				wingmanAir[i]->SetDrawFrame(YSFALSE);
				wingmanAir[i]->SetFill(YSFALSE);

				uitag.Printf(FSGUI_NEWFLTDLG_CHGWINGMANSRC,i+1);
				buf.Printf("Change Wingman %d",i+1);

				const wchar_t *wstr=fsTextResource.FindWString(uitag);

				changeWingman[i]=AddTextButton(1,FSKEY_NULL,FSGUI_PUSHBUTTON,wstr,YSTRUE);
				removeWingman[i]=AddTextButton(1,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_DELETE,YSFALSE);

				wingmanStpLabel[i]=AddStaticText(1,FSKEY_NULL,FSGUI_COMMON_STP,YSTRUE);
				wingmanStpLabel[i]->SetDrawFrame(YSFALSE);
				wingmanStpLabel[i]->SetFill(YSFALSE);

				wingmanStp[i]=AddEmptyDropList(1,FSKEY_NULL,"",16,16,32,YSFALSE);

				grpBox=AddGroupBox();
				grpBox->AddGuiItem(wingmanAir[i]);
				grpBox->AddGuiItem(wingmanStp[i]);
				grpBox->AddGuiItem(wingmanStpLabel[i]);
				grpBox->AddGuiItem(changeWingman[i]);
				grpBox->AddGuiItem(removeWingman[i]);
			}
		}
		EndAddTabItem();
	}
	else
	{
		for(i=0; i<MAXNUMWINGMAN; i++)
		{
			wingmanAir[i]=NULL;
			wingmanStp[i]=NULL;
			wingmanStpLabel[i]=NULL;
			changeWingman[i]=NULL;
			removeWingman[i]=NULL;
		}
	}

	if(YSTRUE==option.canSelectWingmen && YSTRUE==option.canChooseFomType)
	{
		tabId=AddTab(mainTab,FSGUI_NEWFLTDLG_FORMATION);
		BeginAddTabItem(mainTab,tabId);
		{
			const char *const fomTypeStr[]={"None","Diamond","Delta"};
			fomType=AddListBox(1,FSKEY_NULL,FSGUI_NEWFLTDLG_FORMATION,3,fomTypeStr,3,10,YSTRUE);
		}
		EndAddTabItem();
	}
	else
	{
		fomType=NULL;
	}



	tabId=AddTab(mainTab,FSGUI_NEWFLTDLG_ENVIRON);
	BeginAddTabItem(mainTab,tabId);
	{
		if(option.canChooseNight==YSTRUE)
		{
			AddDayNightSelector();
			InsertVerticalSpace(4);
		}
		else
		{
			dayNight[0]=NULL;
			dayNight[1]=NULL;
		}

		AddWeatherSelector();
	}
	EndAddTabItem();

	if(YSTRUE!=option.canSelectEnvironment)
	{
		mainTab->DisableTab(tabId);
	}
	mainTab->SelectFirstTab();

	StepToNextColumn();

	okBtn=AddTextButton(MakeIdent("ok"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(MakeIdent("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);

	flyNowButton=AddTextButton(MakeIdent("flynow"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_NEWFLTDLG_FLYNOW,YSTRUE);
	if(YSTRUE!=option.flyNowButton)
	{
		flyNowButton->Disable();
	}

	ExpandTab(mainTab);
	Fit();
	SetBackgroundAlpha(0.9);
}

void FsGuiNewFlightDialogClass::FieldSelectionChanged(void)
{
	int i;
	YsString str;

	field->GetSelectedString(info.fieldName);

	if(playerStp!=NULL)
	{
		ResetStartPosChoice(playerStp,world,info.fieldName);
		playerStp->SelectByString(info.playerAirInfo.startPos,YSTRUE);
		playerStp->GetSelectedString(info.playerAirInfo.startPos);
	}


	YsString wingmanPos[MAXNUMWINGMAN];
	MakeDefaultWingmanPosition(wingmanPos);
	for(i=0; i<MAXNUMWINGMAN; i++)
	{
		info.wingmanInfo[i].startPos=wingmanPos[i];
		if(wingmanStp[i]!=NULL)
		{
			wingmanStp[i]->SelectByString(wingmanPos[i]);
		}
	}


	ReloadField(world,info.fieldName);
	ReloadStartPos(world,info.fieldName,info.playerAirInfo.startPos);
}

void FsGuiNewFlightDialogClass::ResetStartPosChoice(FsGuiDropList *dropList,FsWorld *world,const char fldName[])
{
	YsString stp;
	YsArray <YsString> stpList;

	for(int i=0; world->GetFieldStartPositionName(stp,fldName,i)==YSOK; i++)
	{
		stpList.Append(stp);
	}

	dropList->SetChoice(stpList.GetN(),stpList);
}

void FsGuiNewFlightDialogClass::InitializeDialog(FsWorld *world,const FsNewFlightDialogInfo &info)
{
	int i;
	YsString str;
	char buf[256];


	this->info=info;
	this->world=world;

	this->info.envInfo.specifyEnvironment=YSFALSE;  // By default, turn it off.

	if(field!=NULL)
	{
		field->SelectByString(info.fieldName,YSTRUE);
	}

	YsString selectedFieldName=info.fieldName,selectedStp=info.playerAirInfo.startPos;
	if(nullptr!=field)
	{
		selectedFieldName=field->GetSelectedString();
	}

	if(playerStp!=NULL)
	{
		ResetStartPosChoice(playerStp,world,selectedFieldName);
		playerStp->SelectByString(info.playerAirInfo.startPos,YSTRUE);  // selectFirstIfNoMatch=YSTRUE
		if(YSTRUE==info.nextStartPos)
		{
			const int newSel=playerStp->GetSelection()+1;
			if(newSel<playerStp->GetNumChoice())
			{
				playerStp->Select(newSel);
				playerStp->GetSelectedString(this->info.playerAirInfo.startPos);
			}
		}
	}

	if(playerAir!=NULL)
	{
		str.Set("Aircraft:");
		str.Append(info.playerAirInfo.typeName);
		playerAir->SetText(str);
	}

	if(nullptr!=playerStp)
	{
		selectedStp=playerStp->GetSelectedString();
	}
	ReloadField(world,selectedFieldName);
	ReloadStartPos(world,selectedFieldName,selectedStp);

	for(i=0; i<MAXNUMWINGMAN; i++)
	{
		if(info.wingmanInfo[i].typeName[0]!=0)
		{
			if(wingmanAir[i]!=NULL)
			{
				sprintf(buf,"Wingman %d:%s",i+1,(const char *)info.wingmanInfo[i].typeName);
				wingmanAir[i]->SetText(buf);
			}
			if(wingmanStp[i]!=NULL)
			{
				ResetStartPosChoice(wingmanStp[i],world,info.fieldName);
				wingmanStp[i]->SelectByString(info.wingmanInfo[i].startPos);
				wingmanStp[i]->Enable();
			}
		}
		else
		{
			if(wingmanAir[i]!=NULL)
			{
				sprintf(buf,"Wingman %d:None",i+1);
				wingmanAir[i]->SetText(buf);
			}
			if(wingmanStp[i]!=NULL)
			{
				wingmanStp[i]->Disable();
			}
		}
	}



	if(dayNight[0]!=NULL && dayNight[1]!=NULL)
	{
		if(info.envInfo.dayOrNight==FSDAYLIGHT)
		{
			dayNight[0]->SetCheck(YSTRUE);
			dayNight[1]->SetCheck(YSFALSE);
		}
		else
		{
			dayNight[0]->SetCheck(YSFALSE);
			dayNight[1]->SetCheck(YSTRUE);
		}
	}



	if(windDir!=NULL)
	{
		i=(int)(info.envInfo.windDir*180.0/YsPi);
		if(i<0)
		{
			i+=360;
		}
		windDir->SetNumber(i);
	}
	if(windSpd!=NULL)
	{
		double v;
		v=info.envInfo.windSpd;
		v=v*3600.0/1852.0;
		windSpd->SetNumber((int)v);
	}
	if(visibility!=NULL)
	{
		double v;
		if(info.envInfo.fog==YSTRUE)
		{
			v=info.envInfo.fogVisibility;
			v/=1852.0;
			visibility->SetNumber((int)v);
		}
		else
		{
			visibility->SetNumber(20);
		}
	}
}

void FsGuiNewFlightDialogClass::MakeDefaultWingmanPosition(YsString str[])
{
	int i,j;
	YsString idName;

	for(i=0; i<MAXNUMWINGMAN; i++)
	{
		str[i].Set("");
	}

	for(i=0; world->GetFieldStartPositionName(idName,info.fieldName,i)==YSOK; i++)
	{
		if(strcmp(info.playerAirInfo.startPos,idName)==0)
		{
			break;
		}
	}

	j=0;
	for(i=i+1; j<MAXNUMWINGMAN && world->GetFieldStartPositionName(idName,info.fieldName,i)==YSOK; i++)
	{
		str[j++]=idName;
	}
}

void FsGuiNewFlightDialogClass::RefreshCloudLayer(void)
{
	info.envInfo.cloudLayer.Clear();
	for(int i=0; i<MAXNUMCLOUDLAYER; i++)
	{
		if(YSTRUE==overCastLayerSw[i]->GetCheck())
		{
			double flr,top;
			flr=(double)overCastLayerFloor[i]->GetNumber();
			top=flr+(double)overCastLayerThickness[i]->GetNumber();

			flr=YsUnitConv::FTtoM(flr);
			top=YsUnitConv::FTtoM(top);

			info.envInfo.cloudLayer.Append(flr);
			info.envInfo.cloudLayer.Append(top);
		}
	}
}

void FsGuiNewFlightDialogClass::Show(const FsGuiDialog *excludeFromDrawing) const
{
	YsAtt3 att;
	att.Set(YsPi/2.0,0.0,YsPi/2.0);
	DrawField(world,YSTRUE,YSTRUE,YSFALSE,YsOrigin());
	DrawAirplane(world,info.playerAirInfo.typeName,att,info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,YSTRUE);

	FsNewSimulationDialogTemplate::Show(excludeFromDrawing);
}

void FsGuiNewFlightDialogClass::OnButtonClick(FsGuiButton *btn)
{
	FsNewSimulationDialogTemplate::OnButtonClick(btn);

	/* if(btn!=NULL && btn==changePlayerStp)
	{
		FsGuiChooseField chooseFld;

		YsString fld,stp,str;
		if(chooseFld.SelectFieldAndStp(fld,stp,world,info.fieldName,info.playerAirInfo.startPos)==YSOK)
		{
			info.fieldName.Set(chooseFld.GetSelectedField());
			info.playerAirInfo.startPos.Set(chooseFld.GetSelectedStartPos());

			if(field!=NULL)
			{
				str.Set("Field:");
				str.Append(info.fieldName);
				field->SetText(str);
			}

			if(playerStp!=NULL)
			{
				str.Set("StartPos:");
				str.Append(info.playerAirInfo.startPos);
				playerStp->SetText(str);
			}

			YsString wingmanPos[MAXNUMWINGMAN];
			MakeDefaultWingmanPosition(wingmanPos);
			for(i=0; i<MAXNUMWINGMAN; i++)
			{
				info.wingmanInfo[i].startPos=wingmanPos[i];
				if(wingmanStp[i]!=NULL)
				{
					str.Set("StartPos:");
					str.Append(wingmanPos[i]);
					wingmanStp[i]->SetText(str);
				}
			}

			ReloadField(world,info.fieldName);
			ReloadStartPos(world,info.fieldName,info.playerAirInfo.startPos);
		}
		return;
	} */

	if(btn!=NULL && btn==changePlayerAir)
	{
		FsGuiChooseAircraftOption opt;
		opt.canSelectLoading=this->option.canSelectLoading;

		chooseAircraftDialog->Initialize();
		chooseAircraftDialog->Create(world,opt,NEWFLIGHT_NEXTACTION_SELECTPLAYERAIRPLANE);
		chooseAircraftDialog->SetDefault(info.playerAirInfo.typeName);

		AttachModalDialog(chooseAircraftDialog);

		return;
	}


	info.envInfo.specifyEnvironment=specifyEnvironment->GetCheck();  // Do it all the time.  It may have changed from other buttons/number box.

	if(btn!=NULL && btn==dayNight[0])
	{
		info.envInfo.dayOrNight=FSDAYLIGHT;
		return;
	}
	if(btn!=NULL && btn==dayNight[1])
	{
		info.envInfo.dayOrNight=FSNIGHT;
		return;
	}

	if(NULL!=btn && btn==addComputerAircraft)
	{
		info.addComputerAircraft=addComputerAircraft->GetCheck();
	}

	for(int i=0; i<MAXNUMCLOUDLAYER; i++)
	{
		if(btn==overCastLayerSw[i])
		{
			RefreshCloudLayer();
		}
	}

	for(int i=0; i<MAXNUMWINGMAN; i++)
	{
		FsGuiChooseField chooseFld;
		YsString defStp,defAir;

		if(info.wingmanInfo[i].typeName.Strlen()>0)
		{
			defAir=info.wingmanInfo[i].typeName;
		}
		else
		{
			defAir=info.playerAirInfo.typeName;
		}

		if(info.wingmanInfo[i].startPos.Strlen()>0)
		{
			defStp=info.wingmanInfo[i].startPos;
		}
		else
		{
			YsString wingmanPos[MAXNUMWINGMAN];
			MakeDefaultWingmanPosition(wingmanPos);
			defStp=wingmanPos[i];
		}

		if(btn!=NULL && btn==changeWingman[i])
		{
			FsGuiChooseAircraftOption opt;
			opt.canSelectLoading=this->option.canSelectLoading;

			chooseAircraftDialog->Initialize();
			chooseAircraftDialog->Create(world,opt,NEWFLIGHT_NEXTACTION_SELECTWINGMAN);
			chooseAircraftDialog->SetDefault(defAir);

			chooseWingmanIdx=i;

			AttachModalDialog(chooseAircraftDialog);
		}
		if(btn!=NULL && btn==removeWingman[i])
		{
			info.wingman[i]=YSFALSE;
			if(wingmanAir[i]!=NULL)
			{
				char buf[256];
				sprintf(buf,"Wingman %d: None",i+1);
				wingmanAir[i]->SetText(buf);
				wingmanStp[i]->Disable();
			}
		}
	}

	field->GetSelectedString(info.fieldName);
	playerStp->GetSelectedString(info.playerAirInfo.startPos);

	if(btn!=NULL && btn==flyNowButton)
	{
		info.flyImmediately=YSTRUE;
		CloseModalDialog((int)YSOK);
	}

	if(NULL!=btn && btn==okBtn)
	{
		info.flyImmediately=YSFALSE;
		CloseModalDialog((int)YSOK);
	}

	if(NULL!=btn && btn==cancelBtn)
	{
		CloseModalDialog((int)YSERR);
	}
}

void FsGuiNewFlightDialogClass::OnListBoxSelChange(FsGuiListBox *lbx,int prevSel)
{
	FsNewSimulationDialogTemplate::OnListBoxSelChange(lbx,prevSel);
	if(lbx!=NULL && lbx==fomType)
	{
		int sel;
		sel=lbx->GetSelection();
		switch(sel)
		{
		case 0:
			info.fomType=FSFOM_NONE;
			break;
		case 1:
			info.fomType=FSFOM_DIAMOND;
			break;
		case 2:
			info.fomType=FSFOM_DELTA;
			break;
		}
	}
}

void FsGuiNewFlightDialogClass::OnDropListSelChange(FsGuiDropList *drp,int /*prevSel*/)
{
	if(drp==playerStp)
	{
		drp->GetSelectedString(info.playerAirInfo.startPos);
		ReloadStartPos(world,info.fieldName,info.playerAirInfo.startPos);
	}
	else
	{
		int i;
		for(i=0; i<MAXNUMWINGMAN; i++)
		{
			if(drp==wingmanStp[i])
			{
				wingmanStp[i]->GetSelectedString(info.wingmanInfo[i].startPos);
			}
		}
	}
}

void FsGuiNewFlightDialogClass::OnNumberBoxChange(FsGuiNumberBox *nbx,int prevNum)
{
	FsNewSimulationDialogTemplate::OnNumberBoxChange(nbx,prevNum);

	if(nbx!=NULL)
	{
		if(nbx==windDir)
		{
			info.envInfo.windDir=(double)nbx->GetNumber()*YsPi/180.0;
		}
		if(nbx==windSpd)
		{
			info.envInfo.windSpd=(double)nbx->GetNumber()*1852.0/3600.0;
		}
		if(nbx==visibility)
		{
			info.envInfo.fog=YSTRUE;
			info.envInfo.fogVisibility=(double)nbx->GetNumber()*1852.0;
		}


		for(int i=0; i<MAXNUMCLOUDLAYER; i++)
		{
			if(nbx==overCastLayerFloor[i] || nbx==overCastLayerThickness[i])
			{
				RefreshCloudLayer();
			}
		}
	}
}

void FsGuiNewFlightDialogClass::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==fieldSearch)
	{
		ResetFieldListBySearchKeyword(world,field,fieldSearch,option.forRacingMode);
	}
	if(txt==playerStpFilter)
	{
		ResetStpListBySearchKeyword(world,playerStp,playerStpFilter);
	}
}

void FsGuiNewFlightDialogClass::OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int nextActionCode)
{
	switch(nextActionCode)
	{
	case NEWFLIGHT_NEXTACTION_NULL:
		break;
	case NEWFLIGHT_NEXTACTION_SELECTPLAYERAIRPLANE:
		if(chooseAircraftDialog==closedModalDialog)
		{
			info.playerAirInfo.typeName=chooseAircraftDialog->selAir;
			info.playerAirInfo.weaponConfig=chooseAircraftDialog->selWeaponConfig;
			info.playerAirInfo.fuel=chooseAircraftDialog->selFuel;

			if(playerAir!=NULL)
			{
				YsString buf;
				buf.Printf("Aircraft: %s",(const char *)chooseAircraftDialog->selAir);
				playerAir->SetText(buf);
			}
		}
		break;
	case NEWFLIGHT_NEXTACTION_SELECTWINGMAN:
		if(chooseAircraftDialog==closedModalDialog)
		{
			const int i=chooseWingmanIdx;

			info.wingman[i]=YSTRUE;
			info.wingmanInfo[i].typeName=chooseAircraftDialog->selAir;
			info.wingmanInfo[i].weaponConfig=chooseAircraftDialog->selWeaponConfig;
			info.wingmanInfo[i].fuel=chooseAircraftDialog->selFuel;

			if(wingmanAir[i]!=NULL)
			{
				YsString buf;
				buf.Printf("Wingman %d: %s",i+1,(const char *)chooseAircraftDialog->selAir);
				wingmanAir[i]->SetText(buf);

				if(info.wingmanInfo[i].startPos.Strlen()==0)
				{
					YsString wingmanPos[MAXNUMWINGMAN];
					MakeDefaultWingmanPosition(wingmanPos);
					info.wingmanInfo[i].startPos=wingmanPos[i];
				}

				ResetStartPosChoice(wingmanStp[i],world,info.fieldName);
				wingmanStp[i]->SelectByString(info.wingmanInfo[i].startPos);
				wingmanStp[i]->Enable();
			}
		}
		break;
	}
}

////////////////////////////////////////////////////////////



FsCreateNewSimulationInGroundObjectDialogClass::FsCreateNewSimulationInGroundObjectDialogClass()
{
	gndListBox=NULL;
}

void FsCreateNewSimulationInGroundObjectDialogClass::Make(FsWorld *world,YSBOOL forAirDefenseMissionIn)
{
	FsGuiDialog::Initialize();

	this->world=world;
	this->forAirDefenseMission=forAirDefenseMissionIn;

	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);

	{
		const int tabId=AddTab(mainTab,FSGUI_COMMON_GENERAL);
		BeginAddTabItem(mainTab,tabId);

		AddFieldSelector(/*forRacingMode=*/YSFALSE);
		unavailableMessageStatic=AddStaticText(1,FSKEY_NULL,FSGUI_NEWFLTDLG_NOUSABLEGNDOBJ,YSTRUE);
		unavailableMessageStatic->SetText("");

		gndListBox=AddEmptyListBox(0,FSKEY_NULL,"",6,32,YSTRUE);

		EndAddTabItem();
	}

	if(YSTRUE==forAirDefenseMissionIn)
	{
		const int tabId=AddTab(mainTab,FSGUI_GNDTOAIRDLG_ENEMYTAB);
		BeginAddTabItem(mainTab,tabId);

		allowStealth=AddTextButton(5,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NEWFLTDLG_ALLOWSTEALTH,YSTRUE);
		allowHeavyBomber=AddTextButton(7,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NEWFLTDLG_ALLOWHEAVYBOMBER,YSTRUE);
		allowBomb=AddTextButton(6,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NEWFLTDLG_ALLOWBOMB,YSTRUE);
		allowAGM=AddTextButton(6,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NEWFLTDLG_ALLOWAGM,YSTRUE);


		maxNumAttacker=AddNumberBox(0,FSKEY_NULL,FSGUI_NEWFLTDLG_MAXNUMATTACKER,24,3,1,5,1,YSTRUE);

		jet=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,FSGUI_NEWFLTDLG_JET,YSTRUE);
		ww2=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,FSGUI_NEWFLTDLG_WW2,YSFALSE);

		FsGuiButton *jetww2[2]={jet,ww2};
		SetRadioButtonGroup(2,jetww2);


		EndAddTabItem();
	}

	mainTab->SelectFirstTab();

	okBtn=AddTextButton(1,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);

	flyNowButton=AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_NEWFLTDLG_FLYNOW,YSTRUE);

	Fit();
}

void FsCreateNewSimulationInGroundObjectDialogClass::Initialize(const FsFlightConfig &cfg)
{
	if(field!=NULL)
	{
		field->SelectByString(cfg.defField);
		ReloadField(world,cfg.defField);
		ReloadUserControllableGroundObject();
	}

	okBtn->Disable();
	flyNowButton->Disable();
}

void FsCreateNewSimulationInGroundObjectDialogClass::Initialize(
    const FsFlightConfig &cfg,const FsGroundToAirDefenseMissionInfo &info)
{
	Initialize(cfg);

	allowStealth->SetCheck(info.attackerInfo.allowStealth);
	allowBomb->SetCheck(info.attackerInfo.allowBomb);
	allowAGM->SetCheck(info.attackerInfo.allowAGM);
	allowHeavyBomber->SetCheck(info.attackerInfo.allowHeavyBomber);

	maxNumAttacker->SetNumber(info.attackerInfo.maxNumAttacker);

	jet->SetCheck(info.attackerInfo.jet);
	ww2->SetCheck(info.attackerInfo.ww2);
}

void FsCreateNewSimulationInGroundObjectDialogClass::ReloadUserControllableGroundObject(void)
{
	if(NULL!=scn)
	{
		YsArray <YsSceneryGndObj *> gobList;
		scn->MakeListOfGndObj(gobList);

		gndListBox->ClearChoice();

		int nUsable=0;
		for(int i=0; i<gobList.GetN(); i++)
		{
			if(YSTRUE==forAirDefenseMission && 0==(gobList[i]->GetFlag()&FSGNDFLAG_CANBEUSEDINANTIAIRMISSION))
			{
				continue;
			}
			if(0!=(gobList[i]->GetFlag()&FSGNDFLAG_CANBEUSEROBJECT))
			{
				YsString str;
				str.Printf("%s(%d)",(const char *)gobList[i]->GetObjName(),i);

				int id=gndListBox->AddString(str,YSFALSE);
				gndListBox->SetIntAttrib(id,i);


				YsMatrix4x4 tfm;
				scn->GetTransformation(tfm,gobList[i]);

				YsVec3 pos=YsOrigin();
				tfm.Mul(pos,pos,1.0);

				printf("%s\n",pos.Txt());

				nUsable++;
			}
		}

		if(0<nUsable)
		{
			unavailableMessageStatic->SetText("");
		}
		else
		{
			unavailableMessageStatic->SetText(FSGUI_NEWFLTDLG_NOUSABLEGNDOBJ);
		}
	}
}

void FsCreateNewSimulationInGroundObjectDialogClass::Show(const FsGuiDialog *excludeFromDrawing) const
{
	YsVec3 gobPos;
	YSBOOL needToDrawGndObjCursor=NeedToDrawGndObjCursor(gobPos);

	YsAtt3 att;
	att.Set(YsPi/2.0,0.0,YsPi/2.0);
	DrawField(world,YSTRUE,YSTRUE,needToDrawGndObjCursor,gobPos);

	FsGuiDialogWithFieldAndAircraft::Show(excludeFromDrawing);
}

YSBOOL FsCreateNewSimulationInGroundObjectDialogClass::NeedToDrawGndObjCursor(YsVec3 &pos) const
{
	const int sel=gndListBox->GetSelection();
	if(NULL!=scn && 0<=sel)
	{
		const int gobId=gndListBox->GetIntAttrib(sel);

		YsArray <YsSceneryGndObj *> gobList;
		scn->MakeListOfGndObj(gobList);

		if(YSTRUE==gobList.IsInRange(gobId))
		{
			const YsSceneryGndObj *gob=gobList[gobId];

			YsMatrix4x4 tfm;
			scn->GetTransformation(tfm,gob);

			tfm.Mul(pos,YsOrigin(),1.0);

			return YSTRUE;
		}
	}
	return YSFALSE;
}

void FsCreateNewSimulationInGroundObjectDialogClass::Retrieve(class FsNewDriveDialogInfo &info)
{
	info.Initialize();
	field->GetSelectedString(info.fieldName);
	info.gobId=gndListBox->GetIntAttrib(gndListBox->GetSelection());
	info.driveNow=flyNow;
}

void FsCreateNewSimulationInGroundObjectDialogClass::Retrieve(class FsGroundToAirDefenseMissionInfo &info)
{
	info.Initialize();

	field->GetSelectedString(info.fieldName);
	info.gobId=gndListBox->GetIntAttrib(gndListBox->GetSelection());

	info.attackerInfo.allowStealth=allowStealth->GetCheck();
	info.attackerInfo.allowHeavyBomber=allowHeavyBomber->GetCheck();
	info.attackerInfo.allowBomb=allowBomb->GetCheck();
	info.attackerInfo.allowAGM=allowAGM->GetCheck();

	info.attackerInfo.maxNumAttacker=maxNumAttacker->GetNumber();

	info.attackerInfo.jet=jet->GetCheck();
	info.attackerInfo.ww2=ww2->GetCheck();
}

void FsCreateNewSimulationInGroundObjectDialogClass::FieldSelectionChanged(void)
{
	YsString fldSel;
	field->GetSelectedString(fldSel);
	ReloadField(world,fldSel);
	ReloadUserControllableGroundObject();

	okBtn->Disable();
	flyNowButton->Disable();
}

void FsCreateNewSimulationInGroundObjectDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn || btn==flyNowButton)
	{
		res=YSOK;
		if(0<=field->GetSelection() &&
		   0<=gndListBox->GetSelection())
		{
			if(btn==flyNowButton)
			{
				flyNow=YSTRUE;
			}
			else
			{
				flyNow=YSFALSE;
			}
			CloseModalDialog((int)YSOK);
		}
	}
	else if(btn==cancelBtn)
	{
		res=YSERR;
		CloseModalDialog((int)YSERR);
	}
}

void FsCreateNewSimulationInGroundObjectDialogClass::OnListBoxSelChange(FsGuiListBox *lbx,int prevSel)
{
	FsNewSimulationDialogTemplate::OnListBoxSelChange(lbx,prevSel);

	if(lbx==gndListBox)
	{
		if(0<=gndListBox->GetSelection())
		{
			okBtn->Enable();
			flyNowButton->Enable();
		}
		else
		{
			okBtn->Disable();
			flyNowButton->Disable();
		}
	}
}

void FsCreateNewSimulationInGroundObjectDialogClass::OnDropListSelChange(FsGuiDropList *,int /*prevSel*/)
{
}

void FsCreateNewSimulationInGroundObjectDialogClass::OnNumberBoxChange(FsGuiNumberBox *,int /*prevNum*/)
{
}

void FsCreateNewSimulationInGroundObjectDialogClass::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==fieldSearch)
	{
		ResetFieldListBySearchKeyword(world,field,fieldSearch,/*forRacingMode=*/YSFALSE);
	}
}

////////////////////////////////////////////////////////////

FsGuiEnduranceModeDialog::FsGuiEnduranceModeDialog()
{
	playerAirName=NULL;
	changePlayer=NULL;
	fieldList=NULL;
	fieldSearch=NULL;
	allowAAM=NULL;
	numWingman=NULL;
	wingmanLevel=NULL;
	jet=NULL;
	ww2=NULL;
	okBtn=NULL;
	cancelBtn=NULL;

	lastCursorMoveClock=0;

	chooseAircraftDialog=new FsGuiChooseAircraft;
}

FsGuiEnduranceModeDialog::~FsGuiEnduranceModeDialog()
{
	if(NULL!=chooseAircraftDialog)
	{
		delete chooseAircraftDialog;
		chooseAircraftDialog=NULL;
	}
}

void FsGuiEnduranceModeDialog::Make(FsWorld *world,int nextActionCode)
{
	this->world=world;
	this->nextActionCode=nextActionCode;

	SetTextMessage("--- Endurance Mode ---");

	playerAirName=AddStaticText(0,FSKEY_NULL,"(AirTypeName)",32,1,YSTRUE);
	playerAirName->SetDrawFrame(YSFALSE);
	changePlayer=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Change Aircraft",YSTRUE);


	int i;
	const char *fldName;
	YsArray <const char *> fldList;
	for(i=0; NULL!=(fldName=world->GetFieldTemplateName(i)); i++)
	{
		fldList.Append(fldName);
	}
	fieldList=AddListBox(0,FSKEY_NULL,"Field",fldList.GetN(),fldList,5,32,YSTRUE);
	fieldSearch=AddTextBox(0,FSKEY_NULL,"Search","",32,YSTRUE);


	numWingman=AddNumberBox(0,FSKEY_NULL,"Number of Wingmen",24,0,0,2,1,YSTRUE);

	const char *const wingmanLevelStr[]=
	{
		"VETERAN","ACE","TOP-ACE"
	};
	wingmanLevel=AddListBox(0,FSKEY_NULL,"Wingman Level",3,wingmanLevelStr,5,32,YSTRUE);

	jet=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,"Jet Fighters",YSTRUE);
	ww2=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,"WW2 Fighters",YSFALSE);
	allowAAM=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,"Use AAM",YSFALSE);

	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,"OK",YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,"Cancel",YSFALSE);

	Fit();
}

void FsGuiEnduranceModeDialog::FieldSelectionChanged(void)
{
	lastCursorMoveClock=FsGuiClock();
}

void FsGuiEnduranceModeDialog::Initialize(FsWorld *world,FsFlightConfig &cfg)
{
	YsString str;

	str.Set("Aircraft: ");
	str.Append(cfg.defAirplane);
	playerAirName->SetText(str);

	info.playerAirInfo.typeName.Set(cfg.defAirplane);
	const FsAirplaneTemplate *tmpl;
	if((tmpl=world->GetAirplaneTemplate(cfg.defAirplane))!=NULL)
	{
		tmpl->GetProperty()->GetWeaponConfig(info.playerAirInfo.weaponConfig);
	}
	info.playerAirInfo.fuel=100;

	fieldList->SelectByString(cfg.defField);

	allowAAM->SetCheck(YSTRUE);
	numWingman->SetNumber(0);
	wingmanLevel->Select(0);
	jet->SetCheck(YSTRUE);
	ww2->SetCheck(YSFALSE);

	ReloadField(world,cfg.defField);
}

void FsGuiEnduranceModeDialog::Retrieve(void)
{
	if(fieldList->GetSelection()>=0)
	{
		fieldList->GetSelectedString(info.fieldName);
	}

	info.allowAAM=allowAAM->GetCheck();
	info.numWingman=numWingman->GetNumber();
	info.wingmanLevel=wingmanLevel->GetSelection();

	info.jet=jet->GetCheck();
	info.ww2=ww2->GetCheck();
}

void FsGuiEnduranceModeDialog::Show(const FsGuiDialog *excludeFromDrawing) const
{
	DrawSelectedFieldAndAircraft();
	FsSet2DDrawing();
	FsGuiDialogWithFieldAndAircraft::Show(excludeFromDrawing);
}

void FsGuiEnduranceModeDialog::DrawSelectedFieldAndAircraft(void) const
{
	unsigned clk;
	clk=FsGuiClock();
	if(lastCursorMoveClock!=0 && (clk<lastCursorMoveClock || (clk-lastCursorMoveClock)>500))
	{
		YsString fldName;
		if(YSOK==fieldList->GetSelectedString(fldName))
		{
			FsGuiEnduranceModeDialog *nonConstPtr=(FsGuiEnduranceModeDialog *)this;
			nonConstPtr->ReloadField(world,fldName);
		}
		lastCursorMoveClock=0;
	}

	DrawField(world,YSFALSE,YSTRUE,YSFALSE,YsOrigin());

	YsAtt3 att;
	att.Set(YsPi/2.0,0.0,YsPi/2.0);
	DrawAirplane(world,info.playerAirInfo.typeName,att,info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,YSTRUE);

}

void FsGuiEnduranceModeDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==changePlayer)
	{
		FsGuiChooseAircraftOption opt;
		chooseAircraftDialog->Initialize();

		chooseAircraftDialog->allowAam=allowAAM->GetCheck();
		chooseAircraftDialog->allowAgm=YSFALSE;
		chooseAircraftDialog->allowBomb=YSFALSE;

		chooseAircraftDialog->Create(world,opt,ENDURANCEDLG_NEXTACTION_SELECTPLAYERAIRCRAFT);
		chooseAircraftDialog->SetDefault(info.playerAirInfo.typeName);

		AttachModalDialog(chooseAircraftDialog);
	}
	else if(btn==okBtn)
	{
		Retrieve();
		CloseModalDialog(nextActionCode);
	}
	else if(btn==cancelBtn)
	{
		CloseModalDialog(0);
	}
}

void FsGuiEnduranceModeDialog::OnListBoxSelChange(FsGuiListBox *lbx,int /*prevSel*/)
{
	if(lbx==fieldList)
	{
		lastCursorMoveClock=FsGuiClock();
	}
}

void FsGuiEnduranceModeDialog::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==fieldSearch)
	{
		ResetFieldListBySearchKeyword(world,fieldList,fieldSearch,/*forRacingMode=*/YSFALSE); // Call base-class service function.
	}
}

void FsGuiEnduranceModeDialog::OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int nextActionCode)
{
	if(closedModalDialog==chooseAircraftDialog)
	{
		if(nextActionCode==ENDURANCEDLG_NEXTACTION_SELECTPLAYERAIRCRAFT)
		{
			info.playerAirInfo.typeName=chooseAircraftDialog->selAir;
			info.playerAirInfo.weaponConfig=chooseAircraftDialog->selWeaponConfig;
			info.playerAirInfo.fuel=chooseAircraftDialog->selFuel;

			YsString str;
			str.Set("Aircraft: ");
			str.Append(chooseAircraftDialog->selAir);
			playerAirName->SetText(str);
		}
	}
}

////////////////////////////////////////////////////////////

FsGuiInterceptMissionDialog::FsGuiInterceptMissionDialog()
{
	chooseAircraftDialog=new FsGuiChooseAircraft;
}

FsGuiInterceptMissionDialog::~FsGuiInterceptMissionDialog()
{
	if(NULL!=chooseAircraftDialog)
	{
		delete chooseAircraftDialog;
		chooseAircraftDialog=NULL;
	}
}

void FsGuiInterceptMissionDialog::Make(FsWorld *world,int nextActionCode)
{
	this->world=world;
	this->nextActionCode=nextActionCode;

	SetTextMessage("--- Intercept Mission ---");

	playerAirName=AddStaticText(0,FSKEY_NULL,"(AirTypeName)",32,1,YSTRUE);
	playerAirName->SetDrawFrame(YSFALSE);
	changePlayer=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Change Aircraft",YSTRUE);

	numWingman=AddNumberBox(0,FSKEY_NULL,"Number of Wingmen",24,0,1,3,1,YSTRUE);

	int i;
	const char *fldName;
	YsArray <const char *> fldList;
	for(i=0; NULL!=(fldName=world->GetFieldTemplateName(i)); i++)
	{
		fldList.Append(fldName);
	}
	fieldList=AddListBox(0,FSKEY_NULL,"Field",fldList.GetN(),fldList,5,32,YSTRUE);
	fieldSearch=AddTextBox(0,FSKEY_NULL,"Search","",32,YSTRUE);

	allowEnemyFighter=AddTextButton(4,FSKEY_NULL,FSGUI_CHECKBOX,"Enemy Fighter Escort",YSTRUE);
	allowStealth=AddTextButton(5,FSKEY_NULL,FSGUI_CHECKBOX,"Allow Stealth",YSTRUE);
	allowBomb=AddTextButton(6,FSKEY_NULL,FSGUI_CHECKBOX,"Allow Bomb",YSTRUE);
	allowHeavyBomber=AddTextButton(7,FSKEY_NULL,FSGUI_CHECKBOX,"Allow Heavy Bomber",YSTRUE);


	maxNumAttacker=AddNumberBox(0,FSKEY_NULL,"Max Number of Attackers",24,3,1,5,1,YSTRUE);

	jet=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,"Jet",YSTRUE);
	ww2=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,"Ww2",YSFALSE);

	FsGuiButton *group[2];
	group[0]=jet;
	group[1]=ww2;
	SetRadioButtonGroup(2,group);

	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);

	Fit();
}

void FsGuiInterceptMissionDialog::Show(const FsGuiDialog *excludeFromDrawing) const
{
	DrawSelectedFieldAndAircraft();
	FsSet2DDrawing();
	FsGuiDialogWithFieldAndAircraft::Show(excludeFromDrawing);
}

void FsGuiInterceptMissionDialog::Initialize(FsWorld *world,FsFlightConfig &cfg)
{
	YsString str;

	str.Set("Aircraft: ");
	str.Append(cfg.defAirplane);
	playerAirName->SetText(str);

	info.playerAirInfo.typeName.Set(cfg.defAirplane);
	const FsAirplaneTemplate *tmpl;
	if((tmpl=world->GetAirplaneTemplate(cfg.defAirplane))!=NULL)
	{
		tmpl->GetProperty()->GetWeaponConfig(info.playerAirInfo.weaponConfig);
	}
	info.playerAirInfo.fuel=100;


	fieldList->SelectByString(cfg.defField);

	numWingman->SetNumber(0);
	jet->SetCheck(YSTRUE);
	ww2->SetCheck(YSFALSE);

	ReloadField(world,cfg.defField);
}

void FsGuiInterceptMissionDialog::Retrieve(void)
{
	fieldList->GetSelectedString(info.fieldName);

	info.attackerInfo.allowAirCover=allowEnemyFighter->GetCheck();
	info.attackerInfo.allowStealth=allowStealth->GetCheck();
	info.attackerInfo.allowBomb=allowBomb->GetCheck();
	info.attackerInfo.allowHeavyBomber=allowHeavyBomber->GetCheck();
	info.numWingman=numWingman->GetNumber();
	info.attackerInfo.maxNumAttacker=maxNumAttacker->GetNumber();
	info.attackerInfo.jet=jet->GetCheck();
	info.attackerInfo.ww2=ww2->GetCheck();
}

void FsGuiInterceptMissionDialog::DrawSelectedFieldAndAircraft(void) const
{
	unsigned clk;
	clk=FsGuiClock();
	if(lastCursorMoveClock!=0 && (clk<lastCursorMoveClock || (clk-lastCursorMoveClock)>500))
	{
		YsString fldName;
		if(YSOK==fieldList->GetSelectedString(fldName))
		{
			FsGuiEnduranceModeDialog *nonConstPtr=(FsGuiEnduranceModeDialog *)this;
			nonConstPtr->ReloadField(world,fldName);
		}
		lastCursorMoveClock=0;
	}

	DrawField(world,YSFALSE,YSTRUE,YSFALSE,YsOrigin());

	YsAtt3 att;
	att.Set(YsPi/2.0,0.0,YsPi/2.0);
	DrawAirplane(world,info.playerAirInfo.typeName,att,info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig,YSTRUE);
}

void FsGuiInterceptMissionDialog::FieldSelectionChanged(void)
{
	lastCursorMoveClock=FsGuiClock();
}

void FsGuiInterceptMissionDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==changePlayer)
	{
		FsGuiChooseAircraftOption opt;
		chooseAircraftDialog->Initialize();

		chooseAircraftDialog->allowAam=YSTRUE;
		chooseAircraftDialog->allowAgm=YSFALSE;
		chooseAircraftDialog->allowBomb=YSFALSE;

		chooseAircraftDialog->Create(world,opt,INTERCEPTDLG_NEXTACTION_SELECTPLAYERAIRCRAFT);
		chooseAircraftDialog->SetDefault(info.playerAirInfo.typeName);

		AttachModalDialog(chooseAircraftDialog);
	}
	else if(btn==okBtn)
	{
		Retrieve();
		CloseModalDialog(nextActionCode);
	}
	else if(btn==cancelBtn)
	{
		CloseModalDialog(0);
	}
	else if(btn==ww2 && YSTRUE==ww2->GetCheck())
	{
		allowBomb->SetCheck(YSTRUE);
		allowHeavyBomber->SetCheck(YSTRUE);
	}

}

void FsGuiInterceptMissionDialog::OnListBoxSelChange(FsGuiListBox *lbx,int /*prevSel*/)
{
	if(lbx==fieldList)
	{
		lastCursorMoveClock=FsGuiClock();
	}
}

void FsGuiInterceptMissionDialog::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==fieldSearch)
	{
		ResetFieldListBySearchKeyword(world,fieldList,fieldSearch,/*forRacingMode=*/YSFALSE);
	}
}

void FsGuiInterceptMissionDialog::OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int nextActionCode)
{
	if(closedModalDialog==chooseAircraftDialog)
	{
		if(INTERCEPTDLG_NEXTACTION_SELECTPLAYERAIRCRAFT==nextActionCode)
		{
			info.playerAirInfo.typeName=chooseAircraftDialog->selAir;
			info.playerAirInfo.weaponConfig=chooseAircraftDialog->selWeaponConfig;
			info.playerAirInfo.fuel=chooseAircraftDialog->selFuel;

			YsString str;
			str.Set("Aircraft: ");
			str.Append(chooseAircraftDialog->selAir);
			playerAirName->SetText(str);
		}
	}
}



FsGuiCloseAirSupportMissionDialog::FsGuiCloseAirSupportMissionDialog()
{
	lastCursorMoveClock=0;
	chooseAircraftDialog=new FsGuiChooseAircraft;
}

FsGuiCloseAirSupportMissionDialog::~FsGuiCloseAirSupportMissionDialog()
{
	if(NULL!=chooseAircraftDialog)
	{
		delete chooseAircraftDialog;
		chooseAircraftDialog=NULL;
	}
}

void FsGuiCloseAirSupportMissionDialog::Make(FsWorld *world,FsFlightConfig &cfg,int nextActionCode)
{
	this->world=world;
	this->nextActionCode=nextActionCode;

	SetTextMessage("--- Close Air Support Mission ---");

	playerAirName=AddStaticText(0,FSKEY_NULL,"(AirTypeName)",32,1,YSTRUE);
	playerAirName->SetDrawFrame(YSFALSE);

	YsString str("Aircraft: ");
	str.Append(cfg.defAirplane);
	playerAirName->SetText(str);
	playerAirNameString=cfg.defAirplane;

	changePlayer=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Change Aircraft",YSTRUE);

	const char *fldName;
	YsArray <const char *> fldList;
	for(int i=0; NULL!=(fldName=world->GetFieldTemplateName(i)); i++)
	{
		fldList.Append(fldName);
	}
	fieldList=AddListBox(0,FSKEY_NULL,"Field",fldList.GetN(),fldList,5,32,YSTRUE);
	fieldSearch=AddTextBox(0,FSKEY_NULL,"Search","",32,YSTRUE);

	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);


	const FsAirplaneTemplate *tmpl;
	if((tmpl=world->GetAirplaneTemplate(cfg.defAirplane))!=NULL)
	{
		tmpl->GetProperty()->GetWeaponConfig(playerWeaponConfig);
	}
	playerFuel=100;

	fieldList->SelectByString(cfg.defField);

	ReloadField(world,cfg.defField);

	Fit();
}


void FsGuiCloseAirSupportMissionDialog::Show(const FsGuiDialog *excludeFromDrawing) const
{
	unsigned clk;
	clk=FsGuiClock();
	if(lastCursorMoveClock!=0 && (clk<lastCursorMoveClock || (clk-lastCursorMoveClock)>500))
	{
		YsString fldName;
		if(YSOK==fieldList->GetSelectedString(fldName))
		{
			FsGuiEnduranceModeDialog *nonConstPtr=(FsGuiEnduranceModeDialog *)this;
			nonConstPtr->ReloadField(world,fldName);
		}
		lastCursorMoveClock=0;
	}

	DrawField(world,YSFALSE,YSTRUE,YSFALSE,YsOrigin());

	YsAtt3 att;
	att.Set(YsPi/2.0,0.0,YsPi/2.0);
	DrawAirplane(world,playerAirNameString,att,playerWeaponConfig.GetN(),playerWeaponConfig,YSTRUE);

	FsSet2DDrawing();
	FsGuiDialogWithFieldAndAircraft::Show(excludeFromDrawing);
}

void FsGuiCloseAirSupportMissionDialog::FieldSelectionChanged(void)
{
	lastCursorMoveClock=FsGuiClock();
}

void FsGuiCloseAirSupportMissionDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==changePlayer)
	{
		FsGuiChooseAircraftOption opt;
		chooseAircraftDialog->Initialize();
		chooseAircraftDialog->Create(world,opt,CLOSEAIRDIALOG_NEXTACTION_SELECTPLAYERAIRCRAFT);
		chooseAircraftDialog->SetDefault(playerAirNameString);

		AttachModalDialog(chooseAircraftDialog);
	}
	else if(btn==okBtn)
	{
		// playerAirNameString is updated from OnModalDialogClosed
		fieldList->GetSelectedString(fieldString);
		CloseModalDialog(nextActionCode);
	}
	else if(btn==cancelBtn)
	{
		CloseModalDialog(0);
	}
}

void FsGuiCloseAirSupportMissionDialog::OnListBoxSelChange(FsGuiListBox *lbx,int /*prevSel*/)
{
	if(lbx==fieldList)
	{
		lastCursorMoveClock=FsGuiClock();
	}
}

void FsGuiCloseAirSupportMissionDialog::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==fieldSearch)
	{
		ResetFieldListBySearchKeyword(world,fieldList,fieldSearch,/*forRacingMode=*/YSFALSE);
	}
}

void FsGuiCloseAirSupportMissionDialog::OnModalDialogClosed(int /*dialogIdent*/,class FsGuiDialog *closedModalDialog,int nextActionCode)
{
	if(closedModalDialog==chooseAircraftDialog)
	{
		if(CLOSEAIRDIALOG_NEXTACTION_SELECTPLAYERAIRCRAFT==nextActionCode)
		{
			playerAirNameString=chooseAircraftDialog->selAir;
			playerWeaponConfig=chooseAircraftDialog->selWeaponConfig;
			playerFuel=chooseAircraftDialog->selFuel;

			YsString str;
			str.Set("Aircraft: ");
			str.Append(chooseAircraftDialog->selAir);
			playerAirName->SetText(str);
		}
	}
}

////////////////////////////////////////////////////////////

