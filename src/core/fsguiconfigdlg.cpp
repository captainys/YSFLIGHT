#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include <ysclass.h>
#include <ysport.h>
#include <ysunitconv.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <fsguifiledialog.h>

#include "fsconfig.h"
#include "fsoption.h"
#include "fsapplyoption.h"

#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"

#include "graphics/common/fsopengl.h"
#include "fswirefont.h"

#include "fstextresource.h"
#include "fsnetconfig.h"


#ifdef WIN32
#include <float.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <ysbitmap.h>
#include <yscompilerwarning.h>

#include "fsguiconfigdlg.h"



////////////////////////////////////////////////////////////

void FsGuiConfigDialog::MakeDialog(FsWorld *world,FsFlightConfig &cfg)
{
	SetTextMessage("-- CONFIG --");


	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);

	int tabId;

	tabId=AddTab(mainTab,FSGUI_CFGDLG_DEFAULT);
	BeginAddTabItem(mainTab,tabId);
	MakeDefaultsDialog(world,cfg);
	EndAddTabItem();


	tabId=AddTab(mainTab,FSGUI_CFGDLG_GAMERULE);
	BeginAddTabItem(mainTab,tabId);
	MakeGameDialog(world,cfg);
	EndAddTabItem();


	tabId=AddTab(mainTab,FSGUI_CFGDLG_GRAPHICS);
	BeginAddTabItem(mainTab,tabId);
	MakeGraphicDialog(world,cfg);
	EndAddTabItem();


	tabId=AddTab(mainTab,FSGUI_CFGDLG_OPENGL);
	BeginAddTabItem(mainTab,tabId);
	MakeOpenGLDialog(world,cfg);
	EndAddTabItem();


	okBtn      =AddTextButton(MkId("ok"),    FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,      YSTRUE);
	cancelBtn  =AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,  FSGUI_COMMON_CANCEL,  YSFALSE);
	resetBtn   =AddTextButton(MkId("reset"), FSKEY_NULL,FSGUI_PUSHBUTTON, FSGUI_CFGDLG_SETDEFAULT,YSFALSE);

	mainTab->SelectFirstTab();

	ExpandTab(mainTab);
	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiConfigDialog::MakeDefaultsDialog(FsWorld *world,FsFlightConfig &cfg)
{
	int i;

	const char *str;
	YsArray <const char *> strList;



	FsGuiStatic *label;
	FsGuiGroupBox *grpBox;



	label=AddStaticText(1,FSKEY_NULL,FSGUI_CFGDLG_ENVIRONMENT,16,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	dayOrNightBtn[0]=AddTextButton(MkId("day")  ,FSKEY_NULL,FSGUI_RADIOBUTTON,L"DAY",YSTRUE);
	dayOrNightBtn[1]=AddTextButton(MkId("night"),FSKEY_NULL,FSGUI_RADIOBUTTON,L"NIGHT",YSFALSE);
	SetRadioButtonGroup(2,dayOrNightBtn);

	grpBox=AddGroupBox();
	grpBox->AddGuiItem(label);
	grpBox->AddGuiItem(dayOrNightBtn[0]);
	grpBox->AddGuiItem(dayOrNightBtn[1]);



	label=AddStaticText(1,FSKEY_NULL,FSGUI_CFGDLG_LIGHT,16,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	lightSrcBtn[0]=AddTextButton(MkId("lightUp"),   FSKEY_NULL,FSGUI_RADIOBUTTON,"UP",YSTRUE);
	lightSrcBtn[1]=AddTextButton(MkId("lightSouth"),FSKEY_NULL,FSGUI_RADIOBUTTON,"SOUTH",YSFALSE);
	lightSrcBtn[2]=AddTextButton(MkId("lightEast") ,FSKEY_NULL,FSGUI_RADIOBUTTON,"EAST",YSFALSE);
	lightSrcBtn[3]=AddTextButton(MkId("lightWest") ,FSKEY_NULL,FSGUI_RADIOBUTTON,"WEST",YSFALSE);
	lightSrcBtn[4]=AddTextButton(MkId("lightNorth"),FSKEY_NULL,FSGUI_RADIOBUTTON,"NORTH",YSFALSE);
	SetRadioButtonGroup(5,lightSrcBtn);

	grpBox=AddGroupBox();
	grpBox->AddGuiItem(label);
	grpBox->AddGuiItem(lightSrcBtn[0]);
	grpBox->AddGuiItem(lightSrcBtn[1]);
	grpBox->AddGuiItem(lightSrcBtn[2]);
	grpBox->AddGuiItem(lightSrcBtn[3]);
	grpBox->AddGuiItem(lightSrcBtn[4]);



	label=AddStaticText(1,FSKEY_NULL,FSGUI_CFGDLG_AIRCRAFT,16,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	strList.Set(0,NULL);
	for(i=0; NULL!=(str=world->GetAirplaneTemplateName(i)); i++)
	{
		strList.Append(str);
	}
	airLst=AddDropList(MkId("aircraft"),FSKEY_NULL,"Airplane",strList.GetN(),strList,16,32,32,YSTRUE);

	grpBox=AddGroupBox();
	grpBox->AddGuiItem(label);
	grpBox->AddGuiItem(airLst);



	label=AddStaticText(1,FSKEY_NULL,FSGUI_CFGDLG_FIELD,16,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	strList.Set(0,NULL);
	for(i=0; NULL!=(str=world->GetFieldTemplateName(i)); i++)
	{
		strList.Append(str);
	}
	fldLst=AddDropList(MkId("field"),FSKEY_NULL,"Field",strList.GetN(),strList,16,32,32,YSTRUE);

	grpBox=AddGroupBox();
	grpBox->AddGuiItem(label);
	grpBox->AddGuiItem(fldLst);




	label=AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_STP,16,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	const char *const dmyStpLst[]={"RW28"};
	stpLst=AddDropList(MkId("startPos"),FSKEY_NULL,"Start Position",1,dmyStpLst,16,32,32,YSTRUE);
	ResetStartPositionList(world,cfg.defField);

	grpBox=AddGroupBox();
	grpBox->AddGuiItem(label);
	grpBox->AddGuiItem(stpLst);
}

void FsGuiConfigDialog::MakeGameDialog(FsWorld *,FsFlightConfig &)
{
	blackOutBtn            =AddTextButton(MkId("blackOut"),    FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_BLACKOUT       ,YSTRUE);
	midAirCollisionBtn     =AddTextButton(MkId("midAirColl"),  FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_MIDAIR         ,YSFALSE);
	noTailStrikeBtn        =AddTextButton(MkId("noTailStrike"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_NOTAILSTRIKE   ,YSTRUE);
	canLandAnywhereBtn     =AddTextButton(MkId("landAnywhere"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_CANLANDANYWHERE,YSFALSE);
	autoRudderBtn          =AddTextButton(MkId("autoRudder"),  FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_AUTORUDDER     ,YSTRUE);
	preciseSimulationBtn   =AddTextButton(MkId("preciseSim"),  FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_PRECISION      ,YSFALSE);
	alwaysShowHudBtn       =AddTextButton(MkId("alwaysHud"),   FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_SHOWHUDALWAYS  ,YSTRUE);
	doNotUseInstPanelBtn   =AddTextButton(MkId("noInstPanel"), FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_NOINSTPANEL    ,YSFALSE);

	simpleHudBtn           =AddTextButton(MkId("simpleHud"),FSKEY_NULL,FSGUI_RADIOBUTTON,FSGUI_CFGDLG_USESIMPLEHUD,YSTRUE);
	threeDHudBtn           =AddTextButton(MkId("3dHud"),    FSKEY_NULL,FSGUI_RADIOBUTTON,FSGUI_CFGDLG_USE3DHUD,YSFALSE);
	FsGuiButton *hudTypeRadioButtonGroup[2]={simpleHudBtn,threeDHudBtn};
	SetRadioButtonGroup(2,hudTypeRadioButtonGroup);

	alwaysDrawPlayerNameBtn=AddTextButton(MkId("drawName"),    FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_SHOWNAMEALWAYS ,YSTRUE);
	drawVirtualJoystickBtn =AddTextButton(MkId("drawJoystick"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_SHOWJOYSTICK   ,YSFALSE);
	f8CameraDelayBtn       =AddTextButton(MkId("f8Delay"),     FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_F8CAMERADELAY  ,YSTRUE);
	showIASBtn             =AddTextButton(MkId("showIAS"),     FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_SHOWIAS        ,YSFALSE);

	YsArray <YsArray <FsGuiDialogItem *> > dlgItemMatrix;
	dlgItemMatrix.Increment();
	dlgItemMatrix.GetEnd().Append(blackOutBtn);
	dlgItemMatrix.GetEnd().Append(midAirCollisionBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.GetEnd().Append(noTailStrikeBtn);
	dlgItemMatrix.GetEnd().Append(canLandAnywhereBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.GetEnd().Append(autoRudderBtn);
	dlgItemMatrix.GetEnd().Append(preciseSimulationBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.GetEnd().Append(alwaysShowHudBtn);
	dlgItemMatrix.GetEnd().Append(doNotUseInstPanelBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.GetEnd().Append(simpleHudBtn);
	dlgItemMatrix.GetEnd().Append(threeDHudBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.GetEnd().Append(alwaysDrawPlayerNameBtn);
	dlgItemMatrix.GetEnd().Append(drawVirtualJoystickBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.GetEnd().Append(f8CameraDelayBtn);
	dlgItemMatrix.GetEnd().Append(showIASBtn);
	AlignLeftMiddle(dlgItemMatrix);

	radarAltLimitTxt       =AddTextBox(12,FSKEY_NULL,FSGUI_CFGDLG_MINRADARALT,"",8,YSTRUE);
	radarAltLimitTxt->SetTextType(FSGUI_INTEGER);

	AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_TROUBLEREQUENCY,YSTRUE);
	troubleFrequencyText=AddStaticText(0,FSKEY_NULL,"0.00000",YSFALSE);
	troubleFrequency=AddHorizontalSlider(0,FSKEY_NULL,32,0.01,10,YSTRUE);
	AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_AIRCRAFTRELIABILITY,YSTRUE);
	aircraftReliabilityText=AddStaticText(0,FSKEY_NULL,"0.00000",YSFALSE);
	aircraftReliability=AddHorizontalSlider(0,FSKEY_NULL,32,1,100,YSTRUE);;
}

void FsGuiConfigDialog::MakeGraphicDialog(FsWorld *,FsFlightConfig &)
{
	drawShadowBtn              =AddTextButton(MkId("drawShadow"),  FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_DRAWSHADOW   ,YSTRUE);
	drawShadowOfDeadAirplaneBtn=AddTextButton(MkId("deadShadow"),  FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_DEADPLNSHADOW,YSFALSE);
	drawOrdinanceBtn           =AddTextButton(MkId("drawWeapon"),  FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_DRAWORDINANCE,YSTRUE);
	drawCoarseWeaponBtn        =AddTextButton(MkId("coarseWeapon"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_COARSEWEAPON ,YSFALSE);
	horizonGradationBtn        =AddTextButton(MkId("horizonGrad"), FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_HORIZONGRAD  ,YSTRUE);

	YsArray <YsArray <FsGuiDialogItem *> > dlgItemMatrix;
	dlgItemMatrix.Increment();
	dlgItemMatrix.Last().Append(drawShadowBtn);
	dlgItemMatrix.Last().Append(drawShadowOfDeadAirplaneBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.Last().Append(drawOrdinanceBtn);
	dlgItemMatrix.Last().Append(drawCoarseWeaponBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.Last().Append(horizonGradationBtn);
	AlignLeftMiddle(dlgItemMatrix);

	drawLightInDaylightBtn     =AddTextButton(MkId("drawLightDay"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_DRAWLIGHTALWAYS,YSTRUE);

	AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_IFVISIBILITYISLESSTHAN,YSFALSE);
	drawLightInDaylightVisibilityThrTxt=AddTextBox(0,FSKEY_NULL,L"",L"",8,YSFALSE);
	drawLightInDaylightVisibilityThrTxt->SetTextType(FSGUI_REALNUMBER);
	AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_THISMILES,YSFALSE);


	FsGuiStatic *label;


	label=AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_CLOUDTYPE,8,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	const char *const cloudOptionStr[]={"No Cloud","Solid Cloud","Flat Cloud"};
	cloudLbx=AddDropList(MkId("cloudType"),FSKEY_NULL,"Cloud",3,cloudOptionStr,3,16,16,YSFALSE);



	label=AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_AIRGRAPHQUAL,18,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	const char *const airGraphStr[]={"Automatic","Always High Quality","Always Coarse"};
	airplaneGraphicsLbx=AddDropList(MkId("airGraphics"),FSKEY_NULL,"Airplane Graphics",3,airGraphStr,3,20,20,YSFALSE);



	label=AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_SMOKETYPE,10,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	const char *const smkTypeStr[]={"Towel","Solid","NOSMOKE"};
	smokeTypeLbx=AddDropList(MkId("smokeType"),FSKEY_NULL,"Smoke Type",3,smkTypeStr,4,16,16,YSFALSE);

	smokeRemainTime=AddTextBox(MkId("smokeTime"),FSKEY_NULL,FSGUI_CFGDLG_SMOKEREMAIN,"",8,YSTRUE);
	smokeRemainTime->SetTextType(FSGUI_INTEGER);

	smokeDrawEveryNStep=AddTextBox(MkId("smokeStep"),FSKEY_NULL,FSGUI_CFGDLG_SMOKESTEP,"",8,YSTRUE);
	smokeDrawEveryNStep->SetTextType(FSGUI_INTEGER);

	drawBurningSmokeByParticle=AddTextButton(MkId("smokeParticle"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_SMKPARTICLE,YSTRUE);

	showFpsBtn=AddTextButton(MkId("showFps"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_SHOWFPS,YSFALSE);
}

void FsGuiConfigDialog::MakeOpenGLDialog(FsWorld *,FsFlightConfig &)
{
	fogBtn=AddTextButton( 0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_FOG,YSTRUE);
	fogVisibility=AddTextBox( 1,FSKEY_NULL,FSGUI_CFGDLG_VISIBILITY,"",10,YSTRUE);
	fogVisibility->SetTextType(FSGUI_REALNUMBER);

	FsGuiStatic *label;

	label=AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_ZBUFQUAL,16,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);

	const char *const zBufQualStr[]={"Low(Fast)","High","Very High","Super High"};
	zBufQualityLbx=AddDropList( 1,FSKEY_NULL,"Z-Buffer Quality",4,zBufQualStr,4,12,12,YSFALSE);

	trspObjBtn              =AddTextButton( 2,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_TRSPOBJ        ,YSTRUE);
	trspSmkBtn              =AddTextButton( 3,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_TRSPSMOKE      ,YSFALSE);
	trspVaporBtn            =AddTextButton( 4,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_TRSPVAPOR      ,YSTRUE);
	reducePeepHoleBtn       =AddTextButton( 5,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_REDUCEPEEPHOLE ,YSFALSE);
	useDlistCloudBtn        =AddTextButton( 6,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_DISPLISTCLOUD  ,YSTRUE);
	useDlistExplosionBtn    =AddTextButton( 7,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_DISPLISTEXPLODE,YSFALSE);
	useDlistWeaponBtn       =AddTextButton( 8,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_DISPLISTWEAPON ,YSTRUE);
	useGroundTextureBtn     =AddTextButton( 9,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_GNDTEXTURE     ,YSFALSE);
	useRunwayLightTextureBtn=AddTextButton(10,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_RWLIGHTTEXTURE ,YSTRUE);
	msaaButton			    =AddTextButton(11, FSKEY_NULL, FSGUI_CHECKBOX, FSGUI_CFGDLG_ANTIALIASING, YSFALSE);

	YsArray <YsArray <FsGuiDialogItem *> > dlgItemMatrix;
	dlgItemMatrix.Increment();
	dlgItemMatrix.Last().Append(trspObjBtn);
	dlgItemMatrix.Last().Append(trspSmkBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.Last().Append(trspVaporBtn);
	dlgItemMatrix.Last().Append(reducePeepHoleBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.Last().Append(useDlistCloudBtn);
	dlgItemMatrix.Last().Append(useDlistExplosionBtn);
	dlgItemMatrix.Increment();
	dlgItemMatrix.Last().Append(useDlistWeaponBtn);
	dlgItemMatrix.Last().Append(useGroundTextureBtn);
	AlignLeftMiddle(dlgItemMatrix);



#ifdef __APPLE__
	useOpenGlAntiAliasing=AddTextButton(11,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_CFGDLG_ANTIALIASING,YSTRUE);
	AddStaticText(0,FSKEY_NULL,FSGUI_CFGDLG_ANTIALIASNEEDRESTART,24,2,YSTRUE);
#endif
}

void FsGuiConfigDialog::CheckSelectionChange(FsWorld *world)
{
	if(fldLst!=NULL && fldLst->CheckAndClearSelectionChange()==YSTRUE)
	{
		YsString fldSel;
		fldLst->GetSelectedString(fldSel);
		ResetStartPositionList(world,fldSel);
	}
}

void FsGuiConfigDialog::ResetStartPositionList(FsWorld *world,const char fldName[])
{
	int i;
	YsArray <const char *> strList;
	YsString ysStr,curStp;
	YsArray <YsString> ysStrList;

	stpLst->GetSelectedString(curStp);

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
	stpLst->SetChoice(strList.GetN(),strList);

	stpLst->SelectByString(curStp);
	if(stpLst->GetSelection()<0)
	{
		stpLst->Select(0);
	}
}

void FsGuiConfigDialog::InitializeDialog(FsWorld *,FsFlightConfig &cfg)
{
	blackOutBtn->SetCheck(cfg.blackOut);
	midAirCollisionBtn->SetCheck(cfg.midAirCollision);
	noTailStrikeBtn->SetCheck(cfg.noTailStrike);
	canLandAnywhereBtn->SetCheck(cfg.canLandAnywhere);
	autoRudderBtn->SetCheck(cfg.autoCoordination);
	preciseSimulationBtn->SetCheck(cfg.accurateTime);
	alwaysShowHudBtn->SetCheck(cfg.showHudAlways);
	doNotUseInstPanelBtn->SetCheck(cfg.useHudAlways);

	if(YSTRUE==cfg.useSimpleHud)
	{
		simpleHudBtn->SetCheck(YSTRUE);
		threeDHudBtn->SetCheck(YSFALSE);
	}
	else
	{
		simpleHudBtn->SetCheck(YSFALSE);
		threeDHudBtn->SetCheck(YSTRUE);
	}

	alwaysDrawPlayerNameBtn->SetCheck(cfg.drawPlayerNameAlways);
	drawVirtualJoystickBtn->SetCheck(cfg.drawVirtualJoystick);
	f8CameraDelayBtn->SetCheck(cfg.externalCameraDelay);
	radarAltLimitTxt->SetInteger((int)(cfg.radarAltitudeLimit/0.3048));
	showFpsBtn->SetCheck(cfg.showFps);
	showIASBtn->SetCheck(cfg.showIAS);

	drawBurningSmokeByParticle->SetCheck(cfg.useParticle);
#ifdef __APPLE__
	useOpenGlAntiAliasing->SetCheck(cfg.useOpenGlAntiAliasing);
#endif
	switch(cfg.env)
	{
	case FSDAYLIGHT:
		dayOrNightBtn[0]->SetCheck(YSTRUE);
		dayOrNightBtn[1]->SetCheck(YSFALSE);
		break;
	case FSNIGHT:
		dayOrNightBtn[0]->SetCheck(YSFALSE);
		dayOrNightBtn[1]->SetCheck(YSTRUE);
		break;
	}

	YsVec3 tst;
	int lightSrcId=0;
	tst=cfg.lightSourceDirection;
	tst.SetY(0.0);
	if(tst==YsOrigin())
	{
		lightSrcId=0;
	}
	else if(fabs(tst.x())>fabs(tst.z()))
	{
		if(tst.x()>0.0)
		{
			lightSrcId=2;
		}
		else
		{
			lightSrcId=3;
		}
	}
	else
	{
		if(tst.z()<0.0)
		{
			lightSrcId=1;
		}
		else
		{
			lightSrcId=4;
		}
	}
	for(int i=0; i<5; ++i)
	{
		lightSrcBtn[i]->SetCheck(i==lightSrcId ? YSTRUE : YSFALSE);
	}

	airLst->SelectByString(cfg.defAirplane);
	fldLst->SelectByString(cfg.defField);
	stpLst->SelectByString(cfg.defStartPos);



	drawShadowBtn->SetCheck(cfg.drawShadow);
	drawShadowOfDeadAirplaneBtn->SetCheck(cfg.shadowOfDeadAirplane);
	drawOrdinanceBtn->SetCheck(cfg.drawOrdinance);
	drawCoarseWeaponBtn->SetCheck(cfg.drawCoarseOrdinance);
	horizonGradationBtn->SetCheck(cfg.horizonGradation);
	drawLightInDaylightBtn->SetCheck(cfg.drawLightsInDaylight);
	drawLightInDaylightVisibilityThrTxt->SetRealNumber(YsUnitConv::MtoSM(cfg.drawLightsInDaylightVisibilityThr),2);

	if(cfg.drawCloud!=YSTRUE)
	{
		cloudLbx->Select(0);
	}
	else
	{
		switch(cfg.cloudType)
		{
		case FSNOCLOUD:
			cloudLbx->Select(0);
			break;
		case FSCLOUDSOLID:
			cloudLbx->Select(1);
			break;
		case FSCLOUDFLAT:
			cloudLbx->Select(2);
			break;
		}
	}
	airplaneGraphicsLbx->Select(cfg.airLod);
	switch(cfg.smkType)
	{
	case FSSMKNOODLE:
	case FSSMKCIRCLE:
	case FSSMKTOWEL:
		smokeTypeLbx->Select(0);
		break;
	case FSSMKSOLID:
		smokeTypeLbx->Select(1);
		break;
	case FSSMKNULL:
		smokeTypeLbx->Select(2);
		break;
	}
	smokeTypeLbx->SetEnabled(YsReverseBool(cfg.useParticle));


	smokeRemainTime->SetInteger((int)cfg.smkRemainTime);
	smokeDrawEveryNStep->SetInteger(cfg.smkStep);



	fogBtn->SetCheck(cfg.drawFog);
	fogVisibility->SetRealNumber(cfg.fogVisibility/1600.0,1);

	zBufQualityLbx->Select(cfg.zbuffQuality);
	trspObjBtn->SetCheck(cfg.drawTransparency);
	trspSmkBtn->SetCheck(cfg.drawTransparentSmoke);
	trspVaporBtn->SetCheck(cfg.drawTransparentVapor);
	reducePeepHoleBtn->SetCheck(cfg.drawTransparentLater);
	useDlistCloudBtn->SetCheck(cfg.useOpenGlListForCloud);
	useDlistExplosionBtn->SetCheck(cfg.useOpenGlListForExplosion);
	useDlistWeaponBtn->SetCheck(cfg.useOpenGlListForWeapon);
	useGroundTextureBtn->SetCheck(cfg.useOpenGlGroundTexture);
	useRunwayLightTextureBtn->SetCheck(cfg.useOpenGlRunwayLightTexture);
	msaaButton->SetCheck(cfg.useOpenGlAntiAliasing);

	troubleFrequency->SetPositionByScaledValue(cfg.aircraftTroubleFrequency);
	aircraftReliability->SetPositionByScaledValue(cfg.aircraftReliability);

	YsString str;
	str.Printf("%d",cfg.aircraftReliability);
	aircraftReliabilityText->SetText(str);
	str.Printf("%.3lf",cfg.aircraftTroubleFrequency);
	troubleFrequencyText->SetText(str);
}

void FsGuiConfigDialog::RetrieveConfig(FsFlightConfig &cfg)
{
	cfg.blackOut=blackOutBtn->GetCheck();
	cfg.midAirCollision=midAirCollisionBtn->GetCheck();
	cfg.noTailStrike=noTailStrikeBtn->GetCheck();
	cfg.canLandAnywhere=canLandAnywhereBtn->GetCheck();
	cfg.autoCoordination=autoRudderBtn->GetCheck();
	cfg.accurateTime=preciseSimulationBtn->GetCheck();
	cfg.showHudAlways=alwaysShowHudBtn->GetCheck();
	cfg.useHudAlways=doNotUseInstPanelBtn->GetCheck();
	cfg.useSimpleHud=simpleHudBtn->GetCheck();
	cfg.drawPlayerNameAlways=alwaysDrawPlayerNameBtn->GetCheck();
	cfg.drawVirtualJoystick=drawVirtualJoystickBtn->GetCheck();
	cfg.externalCameraDelay=f8CameraDelayBtn->GetCheck();
	cfg.showIAS=showIASBtn->GetCheck();
	cfg.radarAltitudeLimit=radarAltLimitTxt->GetRealNumber()*0.3048;
	cfg.showFps=showFpsBtn->GetCheck();
	cfg.useParticle=drawBurningSmokeByParticle->GetCheck();
#ifdef __APPLE__
	cfg.useOpenGlAntiAliasing=useOpenGlAntiAliasing->GetCheck();
#endif

	if(YSTRUE==dayOrNightBtn[0]->GetCheck())
	{
		cfg.env=FSDAYLIGHT;
	}
	else
	{
		cfg.env=FSNIGHT;
	}



	int lightSrcId=0;
	for(int i=0; i<5; ++i)
	{
		if(YSTRUE==lightSrcBtn[i]->GetCheck())
		{
			lightSrcId=i;
			break;
		}
	}
	switch(lightSrcId)
	{
	case 0:  // Up
		cfg.lightSourceDirection=YsYVec();
		break;
	case 1:  // South
		cfg.lightSourceDirection.Set(0.0,sqrt(3.0)/2.0,-0.5);
		break;
	case 2:  // East
		cfg.lightSourceDirection.Set(0.5,sqrt(3.0)/2.0,0.0);
		break;
	case 3:  // West
		cfg.lightSourceDirection.Set(-0.5,sqrt(3.0)/2.0,0.0);
		break;
	case 4:  // North
		cfg.lightSourceDirection.Set(0.0,sqrt(3.0)/2.0,0.5);
		break;
	}



	{
		YsString airSel;
		if(YSOK==airLst->GetSelectedString(airSel))
		{
			strcpy(cfg.defAirplane,airSel);
		}
	}
	{
		YsString fldSel;
		if(YSOK==fldLst->GetSelectedString(fldSel))
		{
			strcpy(cfg.defField,fldSel);
		}
	}
	{
		YsString stpSel;
		if(YSOK==stpLst->GetSelectedString(stpSel))
		{
			strcpy(cfg.defStartPos,stpSel);
		}
	}



	cfg.drawShadow=drawShadowBtn->GetCheck();
	cfg.shadowOfDeadAirplane=drawShadowOfDeadAirplaneBtn->GetCheck();
	cfg.drawOrdinance=drawOrdinanceBtn->GetCheck();
	cfg.drawCoarseOrdinance=drawCoarseWeaponBtn->GetCheck();
	cfg.horizonGradation=horizonGradationBtn->GetCheck();
	cfg.drawLightsInDaylight=drawLightInDaylightBtn->GetCheck();
	cfg.drawLightsInDaylightVisibilityThr=YsUnitConv::SMtoM(drawLightInDaylightVisibilityThrTxt->GetRealNumber());


	switch(cloudLbx->GetSelection())
	{
	case 0:
		cfg.drawCloud=YSFALSE;
		cfg.cloudType=FSNOCLOUD;
		break;
	case 1:
		cfg.drawCloud=YSTRUE;
		cfg.cloudType=FSCLOUDSOLID;
		break;
	case 2:
		cfg.drawCloud=YSTRUE;
		cfg.cloudType=FSCLOUDFLAT;
		break;
	}
	cfg.airLod=airplaneGraphicsLbx->GetSelection();
	switch(smokeTypeLbx->GetSelection())
	{
	default:
	case 0:
		cfg.smkType=FSSMKTOWEL;
		break;
	case 1:
		cfg.smkType=FSSMKSOLID;
		break;
	case 2:
		cfg.smkType=FSSMKNULL;
		break;
	}
	cfg.smkRemainTime=YsBound(smokeRemainTime->GetRealNumber(),10.0,160.0);
	cfg.smkStep=YsBound(smokeDrawEveryNStep->GetInteger(),1,16);



	cfg.drawFog=fogBtn->GetCheck();
	cfg.fogVisibility=YsBound(fogVisibility->GetRealNumber(),0.1,12.5)*1600.0;
	cfg.zbuffQuality=zBufQualityLbx->GetSelection();
	cfg.drawTransparency=trspObjBtn->GetCheck();;
	cfg.drawTransparentSmoke=trspSmkBtn->GetCheck();
	cfg.drawTransparentVapor=trspVaporBtn->GetCheck();
	cfg.drawTransparentLater=reducePeepHoleBtn->GetCheck();
	cfg.useOpenGlListForCloud=useDlistCloudBtn->GetCheck();
	cfg.useOpenGlListForExplosion=useDlistExplosionBtn->GetCheck();
	cfg.useOpenGlListForWeapon=useDlistWeaponBtn->GetCheck();
	cfg.useOpenGlGroundTexture=useGroundTextureBtn->GetCheck();
	cfg.useOpenGlRunwayLightTexture=useRunwayLightTextureBtn->GetCheck();
	cfg.useOpenGlAntiAliasing=msaaButton->GetCheck();

	cfg.aircraftTroubleFrequency=troubleFrequency->GetScaledValue();
	cfg.aircraftReliability=(int)aircraftReliability->GetScaledValue();
}

void FsGuiConfigDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		FsFlightConfig cfg;
		RetrieveConfig(cfg);
		cfg.Save(FsGetConfigFile());
		CloseModalDialog(0);
	}
	else if(btn==cancelBtn)
	{
		CloseModalDialog(0);
	}
	else if(btn==resetBtn)
	{
		FsFlightConfig cfg;
		cfg.SetDefault();
		InitializeDialog(world,cfg);
	}
	else if(btn==drawBurningSmokeByParticle)
	{
		smokeTypeLbx->SetEnabled(YsReverseBool(drawBurningSmokeByParticle->GetCheck()));
	}
}

void FsGuiConfigDialog::OnSliderPositionChange(FsGuiSlider *slider,const double & /*prevPos*/,const double & /*prevValue*/)
{
	if(slider==aircraftReliability)
	{
		YsString str;
		str.Printf("%d",(int)slider->GetScaledValue());
		aircraftReliabilityText->SetText(str);
	}
	if(slider==troubleFrequency)
	{
		YsString str;
		str.Printf("%.3lf",slider->GetScaledValue());
		troubleFrequencyText->SetText(str);
	}
}

/* virtual */ void FsGuiConfigDialog::OnDropListSelChange(FsGuiDropList *drp,int prevSel)
{
	if(drp==fldLst)
	{
		ResetStartPositionList(world,drp->GetSelectedString());
	}
}

////////////////////////////////////////////////////////////


FsGuiOptionDialog::FsGuiOptionDialog()
{
	scrnMode=NULL;
	rememberWindowPos=NULL;
	alwaysOnTop=NULL;
	fontSize=NULL;
	soundBtn=NULL;
	openingDemoBtn=NULL;
	useTaskBarIcon=NULL;
	okBtn=NULL;
	cancelBtn=NULL;
	setDefaultBtn=NULL;
	useMapPreview=NULL;

	langType=NULL;
	langFile=NULL;
	langFileButton=NULL;

	fdlg=new FsGuiFileDialog;

#ifdef _WIN32
	direct3DSucksButton=NULL;
#endif

	option.SetDefault();
}

FsGuiOptionDialog::~FsGuiOptionDialog()
{
	delete fdlg;
}

void FsGuiOptionDialog::Make(void)
{
	int winWid,winHei;

	FsGuiStatic *label;


	FsGetWindowSize(winWid,winHei);

	SetSize(winWid,winHei);
	SetTextMessage("--- Option ---");

#ifdef _WIN32
	const char *const scrnModeStr[]=
	{
		"Normal Window",
		"Maximize Window",
		"Full Screen"
	};
	label=AddStaticText(0,FSKEY_NULL,FSGUI_OPTDLG_SCRNMODE,12,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);
	scrnMode=AddDropList(MkId("scrnMode"),FSKEY_NULL,"Screen Mode",3,scrnModeStr,8,24,24,YSFALSE);
#endif
	rememberWindowPos=AddTextButton(MkId("windowPos"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_REMEMBERWNDPOS,YSTRUE);

#ifdef _WIN32
	alwaysOnTop=AddTextButton(MkId("onTop"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_ALWAYSONTOP,YSTRUE);
#endif

	const char *const fontSizeStr[]=
	{
		"8 pixel",
		"10 pixel",
		"12 pixel",
		"14 pixel",
		"16 pixel",
		"18 pixel",
		"20 pixel",
		"22 pixel",
		"24 pixel",
		"26 pixel",
		"28 pixel",
		"30 pixel",
		"32 pixel"
	};
	label=AddStaticText(0,FSKEY_NULL,FSGUI_OPTDLG_FONTSIZE,10,1,YSTRUE);
	label->SetFill(YSFALSE);
	label->SetDrawFrame(YSFALSE);
	fontSize=AddDropList(MkId("fontSize"),FSKEY_NULL,"Font Size",13,fontSizeStr,16,24,24,YSFALSE);

	char const *langTypeStr[]={"FORCE ENGLISH","AUTOMATIC","SPECIFY FILE"};
	AddStaticText(0,FSKEY_NULL,FSGUI_OPTDLG_LANGTYPE,YSTRUE);
	langType=AddDropList(MkId("languageType"),FSKEY_NULL,"",3,langTypeStr,9,24,24,YSFALSE);
	AddStaticText(0,FSKEY_NULL,FSGUI_OPTDLG_LANGFILE,YSTRUE);
	langFile=AddStaticText(0,FSKEY_NULL,"",80,1,YSFALSE);
	langFileButton=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_OPTDLG_LANGFILEBUTTON,YSTRUE);


	AddStaticText(0,FSKEY_NULL,FSGUI_OPTDLG_FONTSIZEWARNING,10,1,YSTRUE)->SetFill(YSFALSE);

	soundBtn=AddTextButton(MkId("sound"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_SOUND,YSTRUE);
	openingDemoBtn=AddTextButton(MkId("openingDemo"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_OPENINGDEMO,YSTRUE);

	useMapPreview=AddTextButton(MkId("mapPreview"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_USEMAPPREVIEW,YSTRUE);

#ifdef _WIN32
	useTaskBarIcon=AddTextButton(1,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_USETASKBAR,YSTRUE);
#endif

	backPicture=AddTextButton(MkId("backgroundPicture"),FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_BACPICTURE,YSTRUE);

#ifdef _WIN32
	direct3DSucksButton=AddTextButton(1,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_OPTDLG_DIRECT3DSUCKS,YSTRUE);
#endif

	setDefaultBtn=AddTextButton(MkId("reset"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_SETDEFAULT,YSTRUE);
	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);

	Fit();

	SetBackgroundAlpha(0.9);
}

void FsGuiOptionDialog::SelectLanguageFile(void)
{
	fdlg->Initialize();
	fdlg->mode=FsGuiFileDialog::MODE_OPEN;
	fdlg->title.Set(L"Open Language File");
	fdlg->fileExtensionArray.Append(L".uitxt");
	fdlg->defaultFileName.Set(L"en.uitst");
	AttachModalDialog(fdlg);
}

void FsGuiOptionDialog::Initialize(const FsOption &option)
{
	if(scrnMode!=NULL)
	{
		scrnMode->Select(option.scrnMode);
	}
	if(rememberWindowPos!=NULL)
	{
		rememberWindowPos->SetCheck(option.rememberWindowSize);
	}
	if(alwaysOnTop!=NULL)
	{
		alwaysOnTop->SetCheck(option.alwaysOnTop);
	}
	if(fontSize!=NULL)
	{
		fontSize->Select((option.fontHeight-8)/2);
	}
	if(soundBtn!=NULL)
	{
		soundBtn->SetCheck(option.sound);
	}
	if(openingDemoBtn!=NULL)
	{
		openingDemoBtn->SetCheck(option.openingDemo);
	}
	if(useTaskBarIcon!=NULL)
	{
		useTaskBarIcon->SetCheck(option.useTaskBarIcon);
	}
	if(useMapPreview!=NULL)
	{
		useMapPreview->SetCheck(option.useMapPreview);
	}
	if(backPicture!=NULL)
	{
		backPicture->SetCheck(option.backPicture);
	}
#ifdef _WIN32
	if(NULL!=direct3DSucksButton)
	{
		direct3DSucksButton->SetCheck(option.myD3dDriverSucks);
	}
#endif

	if(NULL!=langType)
	{
		switch(option.languageType)
		{
		case FsOption::FORCEENGLISH:
			langType->Select(0);
			langFileButton->Disable();
			break;
		case FsOption::AUTOMATIC:
			langType->Select(1);
			langFileButton->Disable();
			break;
		case FsOption::SPECIFYFILE:
			langType->Select(2);
			langFileButton->Enable();
			break;
		}
	}
	if(NULL!=langFile)
	{
		langFile->SetText(option.languageFile);
	}
}

void FsGuiOptionDialog::Retrieve(void)
{
	option.SetDefault();
	if(scrnMode!=NULL && scrnMode->GetSelection()>=0)
	{
		option.scrnMode=scrnMode->GetSelection();
	}
	if(rememberWindowPos!=NULL)
	{
		option.rememberWindowSize=rememberWindowPos->GetCheck();
	}
	if(alwaysOnTop!=NULL)
	{
		option.alwaysOnTop=alwaysOnTop->GetCheck();
	}
	if(fontSize!=NULL && fontSize->GetSelection()>=0)
	{
		option.fontHeight=8+2*fontSize->GetSelection();
	}
	if(soundBtn!=NULL)
	{
		option.sound=soundBtn->GetCheck();
	}
	if(openingDemoBtn!=NULL)
	{
		option.openingDemo=openingDemoBtn->GetCheck();
	}
	if(useTaskBarIcon!=NULL)
	{
		option.useTaskBarIcon=useTaskBarIcon->GetCheck();
	}
	if(useMapPreview!=NULL)
	{
		option.useMapPreview=useMapPreview->GetCheck();
	}
	if(backPicture!=NULL)
	{
		option.backPicture=backPicture->GetCheck();
	}
#ifdef _WIN32
	if(NULL!=direct3DSucksButton)
	{
		option.myD3dDriverSucks=direct3DSucksButton->GetCheck();
	}
#endif

	if(NULL!=langType)
	{
		switch(langType->GetSelection())
		{
		case 0:
			option.languageType=FsOption::FORCEENGLISH;
			break;
		case 1:
			option.languageType=FsOption::AUTOMATIC;
			break;
		case 2:
			option.languageType=FsOption::SPECIFYFILE;
			break;
		}
	}

	if(NULL!=langFile)
	{
		langFile->GetText(option.languageFile);
	}
}

void FsGuiOptionDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		Retrieve();
		option.Save(FsGetOptionFile());
		FsApplyOption(option);
		CloseModalDialog(YSOK);
	}
	else if(btn==cancelBtn)
	{
		CloseModalDialog(YSERR);
	}
	else if(btn==setDefaultBtn)
	{
		option.SetDefault();
		Initialize(option);
	}
	else if(btn==langFileButton)
	{
		SelectLanguageFile();
	}
}

void FsGuiOptionDialog::OnDropListSelChange(FsGuiDropList *drp,int prevSel)
{
	int sel=drp->GetSelection();
	if(prevSel!=sel)
	{
		if(drp==langType)
		{
			switch(sel)
			{
			case 0:
				langFileButton->Disable();
				break;
			case 1:
				langFileButton->Disable();
				break;
			case 2:
				langFileButton->Enable();
				{
					YsWString langFileStr;
					langFile->GetText(langFileStr);
					if(0==langFileStr.Strlen())
					{
						SelectLanguageFile();
					}
				}
				break;
			}
		}
	}
}

void FsGuiOptionDialog::OnModalDialogClosed(int,class FsGuiDialog *closedModalDialog,int)
{
	if(fdlg==closedModalDialog && YSOK==fdlg->res && 0<fdlg->selectedFileArray.GetN())
	{
		langFile->SetText(fdlg->selectedFileArray[0]);
	}
}



////////////////////////////////////////////////////////////

class FsGuiFuncAssignDialog : public FsGuiDialog
{
public:
	FsJoystick joyMax[FsMaxNumJoystick],joyMin[FsMaxNumJoystick],prevJoy[FsMaxNumJoystick];

	int assignType; // 0:Axis  1:Trigger  2:Key
	int funcType;

	// Output from Axis-assignment
	int newJoy,newAxs;
	YSBOOL newRev;
	int newFuncType;

	// Output from Trigger-assignment
	int newTrig;

	// Output from Key-assignment
	int newKey;

	FsGuiStatic *funcLabel;
	FsGuiListBox *assignList;
	FsGuiButton *reverseBtn;
	FsGuiButton *okBtn,*cancelBtn;

	void MakeAxisAssign(int funcType,int defJoyId,int defJoyAxis,YSBOOL defReverse);
	void MakeTrigAssign(int funcType,int defJoyId,int defJoyTrig);
	void MakeKeyAssign(int funcType,int defKey);
	void Initialize(void);
	void UpdateJoystickState(void);

	virtual void Interval(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnKeyDown(int fsKey,YSBOOL shift,YSBOOL ctrl,YSBOOL alt);
};

void FsGuiFuncAssignDialog::MakeAxisAssign(int funcType,int defJoyId,int defJoyAxis,YSBOOL defReverse)
{
	FsGuiDialog::Initialize();

	int winWid,winHei;
	FsGetWindowSize(winWid,winHei);

	SetSize(winWid,winHei);

	int i,j,index;
	const char *label;
	label=FsGetAxisFuncLabel((FSAXISFUNCTION)funcType);

	funcLabel=AddStaticText(1,FSKEY_NULL,label,32,1,YSTRUE);
	funcLabel->SetDrawFrame(YSFALSE);

	assignList=AddEmptyListBox(1,FSKEY_NULL,"Assignment",12,32,YSTRUE);

	assignList->AddString("----",YSFALSE);

	for(i=0; i<FsMaxNumJoystick; i++)
	{
		if(i!=FsMouseJoyId)
		{
			FsPollJoystick(joyMax[i],i);
			joyMin[i]=joyMax[i];
			prevJoy[i]=joyMax[i];

			for(j=0; j<FsMaxNumJoyAxis; j++)
			{
				char str[256];
				sprintf(str,"JOYSTICK %d AXIS %d",i+1,j+1);
				index=assignList->AddString(str,YSFALSE);
				if(defJoyId==i && defJoyAxis==j)
				{
					assignList->Select(index);
				}
			}
		}
		else
		{
			index=assignList->AddString("MOUSE-X",YSFALSE);
			if(defJoyId==i && defJoyAxis==0)
			{
				assignList->Select(index);
			}
			index=assignList->AddString("MOUSE-Y",YSFALSE);
			if(defJoyId==i && defJoyAxis==1)
			{
				assignList->Select(index);
			}
		}
	}

	reverseBtn=AddTextButton(1,FSKEY_NULL,FSGUI_CHECKBOX,"Reverse",YSTRUE);
	reverseBtn->SetCheck(defReverse);

	okBtn=AddTextButton(1,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);

	assignType=0;
	this->funcType=funcType;

	Fit();
}

void FsGuiFuncAssignDialog::MakeTrigAssign(int funcType,int defJoyId,int defJoyTrig)
{
	FsGuiDialog::Initialize();

	int winWid,winHei;
	FsGetWindowSize(winWid,winHei);

	SetSize(winWid,winHei);

	int i,j,index;
	const char *label;
	label=FsGetButtonFuncLabel((FSBUTTONFUNCTION)funcType);

	funcLabel=AddStaticText(1,FSKEY_NULL,label,32,1,YSTRUE);
	funcLabel->SetDrawFrame(YSFALSE);

	assignList=AddEmptyListBox(1,FSKEY_NULL,"Assignment",12,32,YSTRUE);

	assignList->AddString("----",YSFALSE);

	for(i=0; i<FsMaxNumJoystick; i++)
	{
		if(i!=FsMouseJoyId)
		{
			FsPollJoystick(joyMax[i],i);
			joyMin[i]=joyMax[i];
			prevJoy[i]=joyMax[i];

			for(j=0; j<FsMaxNumJoyTrig; j++)
			{
				char str[256];
				sprintf(str,"JOYSTICK %d BUTTON %d",i+1,j+1);
				index=assignList->AddString(str,YSFALSE);
				if(defJoyId==i && defJoyTrig==j)
				{
					assignList->Select(index);
				}
			}
		}
		else
		{
			index=assignList->AddString("MOUSE LEFT BUTTON",YSFALSE);
			if(defJoyId==i && defJoyTrig==0)
			{
				assignList->Select(index);
			}
			index=assignList->AddString("MOUSE RIGHT BUTTON",YSFALSE);
			if(defJoyId==i && defJoyTrig==1)
			{
				assignList->Select(index);
			}
			index=assignList->AddString("MOUSE MIDDLE BUTTON",YSFALSE);
			if(defJoyId==i && defJoyTrig==2)
			{
				assignList->Select(index);
			}
		}
	}

	okBtn=AddTextButton(1,FSKEY_ENTER,FSGUI_PUSHBUTTON,"OK",YSTRUE);
	cancelBtn=AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,"Cancel",YSFALSE);

	assignType=1;
	this->funcType=funcType;

	Fit();
}

void FsGuiFuncAssignDialog::MakeKeyAssign(int funcType,int defKey)
{
	FsGuiDialog::Initialize();

	int winWid,winHei;
	FsGetWindowSize(winWid,winHei);

	SetSize(winWid,winHei);

	int i,index;
	const char *label;

	label=FsGetButtonFuncLabel((FSBUTTONFUNCTION)funcType);

	funcLabel=AddStaticText(1,FSKEY_NULL,label,32,1,YSTRUE);
	funcLabel->SetDrawFrame(YSFALSE);

	assignList=AddEmptyListBox(1,FSKEY_NULL,"Assignment",12,32,YSTRUE);

	for(i=0; i<FSKEY_NUM_KEYCODE; i++)
	{
		const char *keyLabel;
		keyLabel=FsGetKeyLabel(i);
		if(keyLabel!=NULL)
		{
			index=assignList->AddString(keyLabel,YSFALSE);
		}
		else
		{
			index=assignList->AddString("----",YSFALSE);
		}
		if(defKey==i)
		{
			assignList->Select(index);
		}
	}

	okBtn=AddTextButton(1,FSKEY_ENTER,FSGUI_PUSHBUTTON,"OK",YSTRUE);
	cancelBtn=AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,"Cancel",YSFALSE);

	assignType=2;
	this->funcType=funcType;

	Fit();
}

void FsGuiFuncAssignDialog::Initialize(void)
{
	for(int i=0; i<FsMaxNumJoystick; i++)
	{
		for(int j=0; j<FsMaxNumJoyAxis; j++)
		{
			FsIsJoystickAxisAvailable(i,j); // Dummy call to initialize joy caps
		}
	}
}

void FsGuiFuncAssignDialog::UpdateJoystickState(void)
{
	int i,j;
	for(i=0; i<FsMaxNumJoystick; i++)
	{
		if(i!=FsMouseJoyId)
		{
			FsJoystick joy;
			FsPollJoystick(joy,i);

			if(assignType==0)
			{
				for(j=0; j<FsMaxNumJoyAxis; j++)
				{
					joyMax[i].axs[j]=YsGreater(joyMax[i].axs[j],joy.axs[j]);
					joyMin[i].axs[j]=YsSmaller(joyMin[i].axs[j],joy.axs[j]);

					if(joyMin[i].axs[j]<0.15 && 0.85<joyMax[i].axs[j])
					{
						int sel;
						sel=1+i*FsMaxNumJoyAxis+j;
						assignList->Select(sel);
						joyMax[i].axs[j]=0.5;
						joyMin[i].axs[j]=0.5;
					}
				}
			}
			else if(assignType==1)
			{
				for(j=0; j<FsMaxNumJoyTrig; j++)
				{
					if(prevJoy[i].trg[j]==YSFALSE && joy.trg[j]==YSTRUE)
					{
						int sel;
						sel=1+i*FsMaxNumJoyTrig+j;
						assignList->Select(sel);
					}
				}
			}
			prevJoy[i]=joy;
		}
	}
}

/* virtual */ void FsGuiFuncAssignDialog::Interval(void)
{
	UpdateJoystickState();
}
/* virtual */ void FsGuiFuncAssignDialog::OnButtonClick(FsGuiButton *btn)
{
	if(okBtn==btn)
	{
		if(0==assignType)
		{
			newJoy=(assignList->GetSelection()-1)/FsMaxNumJoyAxis;
			newAxs=(assignList->GetSelection()-1)%FsMaxNumJoyAxis;
			newRev=reverseBtn->GetCheck();
		}
		else if(1==assignType)
		{
			newJoy=(assignList->GetSelection()-1)/FsMaxNumJoyTrig;
			newTrig=(assignList->GetSelection()-1)%FsMaxNumJoyTrig;
		}
		else if(2==assignType)
		{
			newKey=assignList->GetSelection();
		}
		CloseModalDialog(YSOK);
	}
	else if(cancelBtn==btn)
	{
		CloseModalDialog(YSERR);
	}
}

/* virtual */ void FsGuiFuncAssignDialog::OnKeyDown(int fsKey,YSBOOL shift,YSBOOL ctrl,YSBOOL alt)
{
	if(nullptr!=assignList && 2==assignType)
	{
		assignList->Select(fsKey);
		assignList->SetShowTop(fsKey-1);
	}
}



////////////////////////////////////////////////////////////



class FsGuiPrimaryJoystickDialogClass : public FsGuiDialog
{
public:
	FsGuiNumberBox *joyId;
	FsGuiButton *okBtn,*cancelBtn;
	FsGuiStatic *joyStickState[FsMaxNumJoystick];

	double min[FsMaxNumJoystick][2],max[FsMaxNumJoystick][2];

public:
	int selectedJoyId;
	void Make(void);
	virtual void Interval(void);
	virtual void OnButtonClick(FsGuiButton *btn);
};

void FsGuiPrimaryJoystickDialogClass::Make(void)
{
	SetTextMessage("--- Select Primary Joystick ID ---");
	AddStaticText(0,FSKEY_NULL,FSGUI_ASSIGNDLG_PRIMARYAXISMESSAGE,YSFALSE);
	joyId=AddNumberBox(0,FSKEY_NULL,"Primary Joystick",22,1,1,FsMaxNumJoystick,1,YSTRUE);
	okBtn=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);
	selectedJoyId=-1;

	for(int i=0; i<FsMaxNumJoystick; i++)
	{
		joyStickState[i]=AddStaticText(0,FSKEY_NULL,"JOYSTICK: # X:# Y:#",YSTRUE);
	}

	for(int i=0; i<FsMaxNumJoystick; i++)
	{
		min[i][0]=0.5;
		min[i][1]=0.5;
		max[i][0]=0.5;
		max[i][1]=0.5;
	}

	Fit();
}

/* virtual */ void FsGuiPrimaryJoystickDialogClass::Interval(void)
{
	for(int i=0; i<FsMaxNumJoystick; i++)
	{
		if(i!=FsMouseJoyId)
		{
			FsJoystick joy;
			FsPollJoystick(joy,i);

			YsString str;
			str.Printf("JOYSTICK:%d X:%.2lf Y:%.2lf",i+1,joy.axs[0],joy.axs[1]);
			joyStickState[i]->SetText(str);

			for(int axis=0; axis<2; axis++)
			{
				max[i][axis]=YsGreater(max[i][axis],joy.axs[axis]);
				min[i][axis]=YsSmaller(min[i][axis],joy.axs[axis]);
			}

			if(0.75<max[i][0] && 0.75<max[i][1] && 0.75>min[i][0] && 0.75>min[i][1])
			{
				joyId->SetNumber(i+1);
				min[i][0]=0.5;
				max[i][0]=0.5;
				min[i][1]=0.5;
				max[i][1]=0.5;
			}
		}
	}
}

/* virtual */ void FsGuiPrimaryJoystickDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(okBtn==btn)
	{
		selectedJoyId=joyId->GetNumber()-1;
		CloseModalDialog(YSOK);
	}
	else if(cancelBtn==btn)
	{
		CloseModalDialog(YSERR);
	}
}



////////////////////////////////////////////////////////////



FsGuiKeyAssignDialogClass::FsGuiKeyAssignDialogClass()
{
	fdlg=new FsGuiFileDialog;
}

FsGuiKeyAssignDialogClass::~FsGuiKeyAssignDialogClass()
{
	delete fdlg;
}

void FsGuiKeyAssignDialogClass::Make(void)
{
	SetTextMessage("--- Joystick/Mouse/Key Assignment ---");

	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);

	int tabId=AddTab(mainTab,FSGUI_ASSIGNDLG_AXIS);
	BeginAddTabItem(mainTab,tabId);

	joyAxisFunc=AddEmptyListBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_AXISFUNCTION,14,24,YSTRUE);
	joyAxis=AddEmptyListBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_JOYSTICKAXIS,14,24,YSFALSE);
	changeJoyAxisFuncBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_CHANGE,YSFALSE);
	deadZoneElevator=AddNumberBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_ELVDEADZONE,22,3,0,50,1,YSTRUE);
	deadZoneAileron=AddNumberBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_AILDEADZONE,22,3,0,50,1,YSFALSE);
	deadZoneRudder=AddNumberBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_RUDDEADZONE,22,3,0,50,1,YSTRUE);
	usePovHatSwitch=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_ASSIGNDLG_USEPOV,YSFALSE);

	EndAddTabItem();


	tabId=AddTab(mainTab,FSGUI_ASSIGNDLG_TRIGGER);
	BeginAddTabItem(mainTab,tabId);

	joyTrigFunc=AddEmptyListBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_TRIGGERFUNCTION,18,24,YSTRUE);
	joyTrig=AddEmptyListBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_JOYSTICKBUTTON,18,24,YSFALSE);
	changeJoyTrigFuncBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_CHANGE,YSFALSE);

	EndAddTabItem();


	tabId=AddTab(mainTab,FSGUI_ASSIGNDLG_KEYBOARD);
	BeginAddTabItem(mainTab,tabId);

	keyFunc=AddEmptyListBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_KEYFUNCTION,18,24,YSTRUE);
	key=AddEmptyListBox(0,FSKEY_NULL,FSGUI_ASSIGNDLG_KEY,18,24,YSFALSE);
	changeKeyFuncBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_CHANGE,YSFALSE);

	EndAddTabItem();

	mainTab->SelectFirstTab();


	setDefaultBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_SETDEFAULT,YSTRUE);
	setDefaultMouseAsStickBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_SETMOUSEASJOYSTICK,YSFALSE);
	setDefaultKeyboadAsStickBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_SETKEYBOARDASJOYSITKC,YSFALSE);
	setDefaultButPrimaryJoystickIsNotJoystick1=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,
	    FSGUI_ASSIGNDLG_SETDEFAULTWITHPRIMARYJOYSTICK,YSTRUE);
	setDefaultGamePad=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_SETDEFAULTGAMEPAD,YSTRUE);
	setDefaultKeyAssignKeepMouseAndStickBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_SETDEFAULTKEYASSIGN,YSTRUE);

	saveBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_SAVE,YSTRUE);
	loadBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_ASSIGNDLG_LOAD,YSFALSE);
	okBtn=AddTextButton(1,FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSFALSE);
	cancelBtn=AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);

	Fit();
	SetTransparency(YSFALSE);
}


void FsGuiKeyAssignDialogClass::Initialize(void)
{
	int i;

	joyAxisFunc->ClearChoice();
	joyAxis->ClearChoice();
	joyTrigFunc->ClearChoice();
	joyTrig->ClearChoice();
	keyFunc->ClearChoice();
	key->ClearChoice();

	for(i=1; i<(int)FSAXF_NUMAXISFUNCTION; i++)
	{
		const char *label;
		label=FsGetAxisFuncLabel((FSAXISFUNCTION)i);
		if(label!=NULL)
		{
			joyAxisFunc->AddString(label,YSFALSE);

			int joyId,joyAxs;
			YSBOOL rev;
			if(ctlAssign.FindAxisByFunction(joyId,joyAxs,rev,(FSAXISFUNCTION)i)==YSOK)
			{
				char str[256];
				if(joyId!=FsMouseJoyId)
				{
					sprintf(str,"JOY %d AXIS %d ",joyId+1,joyAxs+1);
				}
				else
				{
					sprintf(str,"MOUSE %s",(joyAxs==0 ? "X-AXIS" : "Y-AXIS"));
				}
				if(rev==YSTRUE)
				{
					strcat(str," REVERSE");
				}
				joyAxis->AddString(str,YSFALSE);
			}
			else
			{
				joyAxis->AddString("----",YSFALSE);
			}
		}
	}

	for(i=1; i<(int)FSBTF_NUMBUTTONFUNCTION; i++)
	{
		const char *label;
		label=FsGetButtonFuncLabel((FSBUTTONFUNCTION)i);
		if(label!=NULL)
		{
			joyTrigFunc->AddString(label,YSFALSE);

			int joyId,joyTrg;
			if(ctlAssign.FindTriggerByFunction(joyId,joyTrg,(FSBUTTONFUNCTION)i)==YSOK)
			{
				char str[256];
				if(joyId!=FsMouseJoyId)
				{
					sprintf(str,"JOY %d BTN %d ",joyId+1,joyTrg+1);
				}
				else
				{
					switch(joyTrg)
					{
					case 0:
						strcpy(str,"MOUSE LEFT BUTTON");
						break;
					case 1:
						strcpy(str,"MOUSE RIGHT BUTTON");
						break;
					case 2:
						strcpy(str,"MOUSE MID BUTTON");
						break;
					default:
						strcpy(str,"MOUSE ? BUTTON");
						break;
					}
				}
				joyTrig->AddString(str,YSFALSE);
			}
			else
			{
				joyTrig->AddString("----",YSFALSE);
			}


			keyFunc->AddString(label,YSFALSE);

			int keyCode;
			keyCode=ctlAssign.FindKeyByFunction((FSBUTTONFUNCTION)i);
			if(keyCode!=FSKEY_NULL)
			{
				const char *label;
				label=FsGetKeyLabel(keyCode);
				if(label!=NULL)
				{
					key->AddString(label,YSFALSE);
				}
				else
				{
					key->AddString("???",YSFALSE);
				}
			}
			else
			{
				key->AddString("----",YSFALSE);
			}
		}
	}

	deadZoneElevator->SetNumber((int)(ctlAssign.deadZoneElevator*100.0+0.499));
	deadZoneAileron->SetNumber((int)(ctlAssign.deadZoneAileron*100.0+0.499));
	deadZoneRudder->SetNumber((int)(ctlAssign.deadZoneRudder*100.0+0.499));
	usePovHatSwitch->SetCheck(ctlAssign.usePovHatSwitch);
}

void FsGuiKeyAssignDialogClass::SaveListBoxPos(void)
{
	axisShowTop=joyAxis->GetShowTop();
	axisSel=joyAxis->GetSelection();

	trigShowTop=joyTrig->GetShowTop();
	trigSel=joyTrig->GetSelection();
	
	keyShowTop=key->GetShowTop();
	keySel=key->GetSelection();
}

void FsGuiKeyAssignDialogClass::RestoreListBoxPos(void)
{
	joyAxis->SetShowTop(axisShowTop);
	joyAxis->Select(axisSel);
	joyAxisFunc->SetShowTop(axisShowTop);
	joyAxisFunc->Select(axisSel);

	joyTrig->SetShowTop(trigShowTop);
	joyTrig->Select(trigSel);
	joyTrigFunc->SetShowTop(trigShowTop);
	joyTrigFunc->Select(trigSel);

	key->SetShowTop(keyShowTop);
	key->Select(keySel);
	keyFunc->SetShowTop(keyShowTop);
	keyFunc->Select(keySel);
}

void FsGuiKeyAssignDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(okBtn==btn)
	{
		ctlAssign.deadZoneElevator=((double)deadZoneElevator->GetNumber())/100.0;
		ctlAssign.deadZoneAileron=((double)deadZoneAileron->GetNumber())/100.0;
		ctlAssign.deadZoneRudder=((double)deadZoneRudder->GetNumber())/100.0;
		ctlAssign.usePovHatSwitch=usePovHatSwitch->GetCheck();
		ctlAssign.Save(FsGetControlAssignFile());
		CloseModalDialog(YSOK);
	}
	if(cancelBtn==btn)
	{
		CloseModalDialog(YSERR);
	}

	if(btn==setDefaultBtn)
	{
		ctlAssign.SetDefault(0);
		ctlAssign.BuildMapping();
		Initialize();
	}
	if(btn==setDefaultGamePad)
	{
		auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiPrimaryJoystickDialogClass>();
		dlg->Make();
		dlg->BindCloseModalCallBack(&FsGuiKeyAssignDialogClass::OnCloseGamePadDialog,this);
		AttachModalDialog(dlg);
	}
	if(btn==setDefaultMouseAsStickBtn)
	{
		ctlAssign.SetDefaultMouseAsStick();
		ctlAssign.BuildMapping();
		Initialize();
	}
	if(btn==setDefaultKeyboadAsStickBtn)
	{
		ctlAssign.SetDefaultKeyboardAsStick();
		ctlAssign.BuildMapping();
		Initialize();
	}
	if(btn==setDefaultButPrimaryJoystickIsNotJoystick1)
	{
		auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiPrimaryJoystickDialogClass>();
		dlg->Make();
		dlg->BindCloseModalCallBack(&FsGuiKeyAssignDialogClass::OnClosePrimaryJoystickDialog,this);
		AttachModalDialog(dlg);
	}
	if(btn==setDefaultKeyAssignKeepMouseAndStickBtn)
	{
		ctlAssign.SetDefaultKeyAssign();
		ctlAssign.BuildMapping();
		Initialize();
	}


	if(btn==saveBtn)
	{
		fdlg->Initialize();
		fdlg->mode=FsGuiFileDialog::MODE_SAVE;
		fdlg->title.Set(L"Save Assignment");
		fdlg->fileExtensionArray.Append(L".asn");
		fdlg->defaultFileName.Set(L"ysflightKeyAssign.asn");
		AttachModalDialog(fdlg);
	}

	if(btn==loadBtn)
	{
		fdlg->Initialize();
		fdlg->mode=FsGuiFileDialog::MODE_OPEN;
		fdlg->title.Set(L"Open Assignment");
		fdlg->fileExtensionArray.Append(L".asn");
		fdlg->defaultFileName.Set(L"ysflightKeyAssign.asn");
		AttachModalDialog(fdlg);
	}


	if(btn==changeJoyAxisFuncBtn)
	{
		int newJoy,newAxis,defJoy,defAxis;
		YSBOOL newRev,defRev;
		int funcType;

		SaveListBoxPos();

		funcType=joyAxisFunc->GetSelection()+1;
		if(0<=funcType && funcType<FSAXF_NUMAXISFUNCTION)
		{
			defJoy=-1;
			defAxis=-1;
			defRev=YSFALSE;
			ctlAssign.FindAxisByFunction(defJoy,defAxis,defRev,(FSAXISFUNCTION)funcType);

			auto *dlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiFuncAssignDialog>();
			dlg->MakeAxisAssign(funcType,defJoy,defAxis,defRev);
			dlg->Initialize();
			dlg->BindCloseModalCallBack(&FsGuiKeyAssignDialogClass::OnCloseAxisAssignDialog,this);
			AttachModalDialog(dlg);
		}

		RestoreListBoxPos();
	}
	if(btn==changeJoyTrigFuncBtn)
	{
		int newJoy,newTrig,defJoy,defTrig;
		int funcType;

		SaveListBoxPos();

		funcType=joyTrigFunc->GetSelection()+1;
		if(0<=funcType && funcType<FSBTF_NUMBUTTONFUNCTION)
		{
			defJoy=-1;
			defTrig=-1;
			ctlAssign.FindTriggerByFunction(defJoy,defTrig,(FSBUTTONFUNCTION)funcType);

			auto *dlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiFuncAssignDialog>();
			dlg->MakeTrigAssign(funcType,defJoy,defTrig);
			dlg->Initialize();
			dlg->BindCloseModalCallBack(&FsGuiKeyAssignDialogClass::OnCloseTriggerAssignDialog,this);
			AttachModalDialog(dlg);
		}

		RestoreListBoxPos();
	}
	if(btn==changeKeyFuncBtn)
	{
		int newKey,defKey;
		int funcType;

		SaveListBoxPos();

		funcType=keyFunc->GetSelection()+1;
		if(0<=funcType && funcType<FSBTF_NUMBUTTONFUNCTION)
		{
			defKey=ctlAssign.FindKeyByFunction((FSBUTTONFUNCTION)funcType);

			auto *dlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiFuncAssignDialog>();
			dlg->MakeKeyAssign(funcType,defKey);
			dlg->Initialize();
			dlg->BindCloseModalCallBack(&FsGuiKeyAssignDialogClass::OnCloseKeyAssignDialog,this);
			AttachModalDialog(dlg);
		}

		RestoreListBoxPos();
	}
}

void FsGuiKeyAssignDialogClass::OnCloseAxisAssignDialog(FsGuiDialog *dlg,int returnCode)
{
	auto assignDlg=dynamic_cast <FsGuiFuncAssignDialog *>(dlg);
	if(nullptr!=assignDlg && (int)YSOK==returnCode)
	{
		auto newJoy=assignDlg->newJoy;
		auto newAxis=assignDlg->newAxs;
		auto newRev=assignDlg->newRev;
		auto funcType=assignDlg->funcType;
		printf("%d %d %d\n",newJoy,newAxis,newRev);
		if(newJoy>=0)
		{
			ctlAssign.AddAxisAssignment(newJoy,newAxis,(FSAXISFUNCTION)funcType,newRev);
		}
		else
		{
			ctlAssign.DeleteAxisFunction((FSAXISFUNCTION)funcType);
		}
		ctlAssign.BuildMapping();
		Initialize();
	}
}

void FsGuiKeyAssignDialogClass::OnCloseTriggerAssignDialog(FsGuiDialog *dlg,int returnCode)
{
	auto assignDlg=dynamic_cast <FsGuiFuncAssignDialog *>(dlg);
	if(nullptr!=assignDlg && (int)YSOK==returnCode)
	{
		auto newJoy=assignDlg->newJoy;
		auto newTrig=assignDlg->newTrig;
		auto funcType=assignDlg->funcType;
		if(newJoy>=0)
		{
			ctlAssign.AddTriggerAssignment(newJoy,newTrig,(FSBUTTONFUNCTION)funcType);
		}
		else
		{
			ctlAssign.DeleteTriggerFunction((FSBUTTONFUNCTION)funcType);
		}
		ctlAssign.BuildMapping();
		Initialize();
	}
}

void FsGuiKeyAssignDialogClass::OnCloseKeyAssignDialog(FsGuiDialog *dlg,int returnCode)
{
	auto assignDlg=dynamic_cast <FsGuiFuncAssignDialog *>(dlg);
	if(nullptr!=assignDlg && (int)YSOK==returnCode)
	{
		auto newKey=assignDlg->newKey;
		auto funcType=assignDlg->funcType;
		if(newKey>=0)
		{
			ctlAssign.AddKeyAssignment(newKey,(FSBUTTONFUNCTION)funcType);
		}
		else
		{
			ctlAssign.DeleteKeyFunction((FSBUTTONFUNCTION)funcType);
		}
		ctlAssign.BuildMapping();
		Initialize();
	}
}
void FsGuiKeyAssignDialogClass::OnClosePrimaryJoystickDialog(FsGuiDialog *dlg,int returnCode)
{
	auto primaryJoystickDlg=dynamic_cast <FsGuiPrimaryJoystickDialogClass *>(dlg);
	if(nullptr!=primaryJoystickDlg && (int)YSOK==returnCode)
	{
		const int primaryJoyId=primaryJoystickDlg->selectedJoyId;
		if(0<=primaryJoyId)
		{
			ctlAssign.SetDefault(primaryJoyId);
			ctlAssign.BuildMapping();
			Initialize();
		}
	}
}

void FsGuiKeyAssignDialogClass::OnCloseGamePadDialog(FsGuiDialog *dlg,int returnCode)
{
	auto primaryJoystickDlg=dynamic_cast <FsGuiPrimaryJoystickDialogClass *>(dlg);
	if(nullptr!=primaryJoystickDlg && (int)YSOK==returnCode)
	{
		const int primaryJoyId=primaryJoystickDlg->selectedJoyId;
		if(0<=primaryJoyId)
		{
			ctlAssign.SetDefaultGamePad(primaryJoyId);
			ctlAssign.BuildMapping();
			Initialize();
		}
	}
}

void FsGuiKeyAssignDialogClass::OnListBoxSelChange(FsGuiListBox *lbx,int prevSel)
{
	YsDisregardVariable(prevSel);

	const int sel=lbx->GetSelection();
	if(lbx==joyAxis || lbx==joyAxisFunc)
	{
		joyAxis->Select(sel);
		joyAxisFunc->Select(sel);
	}
	if(lbx==joyTrig || lbx==joyTrigFunc)
	{
		joyTrig->Select(sel);
		joyTrigFunc->Select(sel);
	}
	if(lbx==key || lbx==keyFunc)
	{
		key->Select(sel);
		keyFunc->Select(sel);
	}
}

void FsGuiKeyAssignDialogClass::OnListBoxScroll(FsGuiListBox *lbx,int prevShowTop)
{
	YsDisregardVariable(prevShowTop);

	const int showTop=lbx->GetShowTop();
	if(lbx==joyAxis || lbx==joyAxisFunc)
	{
		joyAxis->SetShowTop(showTop);
		joyAxisFunc->SetShowTop(showTop);
	}
	if(lbx==joyTrig || lbx==joyTrigFunc)
	{
		joyTrig->SetShowTop(showTop);
		joyTrigFunc->SetShowTop(showTop);
	}
	if(lbx==key || lbx==keyFunc)
	{
		key->SetShowTop(showTop);
		keyFunc->SetShowTop(showTop);
	}
}

void FsGuiKeyAssignDialogClass::OnModalDialogClosed(int,class FsGuiDialog *closedModalDialog,int)
{
	if(fdlg==closedModalDialog && YSOK==fdlg->res && 0<fdlg->selectedFileArray.GetN())
	{
		if(FsGuiFileDialog::MODE_SAVE==fdlg->mode)
		{
			ctlAssign.deadZoneElevator=((double)deadZoneElevator->GetNumber())/100.0;
			ctlAssign.deadZoneAileron=((double)deadZoneAileron->GetNumber())/100.0;
			ctlAssign.deadZoneRudder=((double)deadZoneRudder->GetNumber())/100.0;
			ctlAssign.usePovHatSwitch=usePovHatSwitch->GetCheck();

			ctlAssign.Save(fdlg->selectedFileArray[0]);
		}
		else if(FsGuiFileDialog::MODE_OPEN==fdlg->mode)
		{
			ctlAssign.Load(fdlg->selectedFileArray[0]);
			Initialize();
		}
	}
}

////////////////////////////////////////////////////////////

static void FsDecodeHostNameAndPort(YsString &hostname,int &port,const char str[])
{
	port=-1;

	int i;
	for(i=0; 0!=str[i]; i++)
	{
		if('('==str[i])
		{
			port=atoi(str+i+1);
			hostname.Set(str);
			hostname.SetLength(i);
			hostname.DeleteTailSpace();
			return;
		}
	}

	hostname.Set(str);
	hostname.DeleteTailSpace();
}

