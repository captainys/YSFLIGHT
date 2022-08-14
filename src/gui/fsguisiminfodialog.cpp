#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>
#include <ysport.h>
#include <ysunitconv.h>
#include <ysbase64.h>

#include "fsguisiminfodialog.h"
#include "fsprintf.h"
#include "fstextresource.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"
#include "fsfilename.h"
#include "fsexistence.h"
#include "fsoption.h"

//	FsGuiButton *okBtn;
//	FsGuiTabControl *mainTab;

YSRESULT FsGuiMissionGoalDialogClass::Create(const class FsSimulation *sim,int nextActionCode)
{
	Initialize();

	SetIdent("simInfo");

	SetTransparency(YSFALSE);

	SetTextMessage("== Your Mission ==");

	this->nextActionCode=nextActionCode;

	okBtn=AddTextButton(MkId("ok"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);

	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);
	AddDescriptionTab(sim);
	AddBitmapTab(sim);
	AddDetailTab(sim);

	mainTab->SelectFirstTab();

	ExpandTab(mainTab);
	Fit();

	SetFocus(okBtn);

	return YSOK;
}

void FsGuiMissionGoalDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		CloseModalDialog(nextActionCode);
	}
}

int FsGuiMissionGoalDialogClass::AddDescriptionTab(const class FsSimulation *sim)
{
	const FsMissionGoal &goal=sim->GetMissionGoal();
	auto &text=goal.GetText();
	auto &iText=goal.GetInternationalText();

	if(YSTRUE==goal.IsActiveMission() && (0<text.GetN() || 0<iText.GetN()))
	{
		const int tabId=AddTab(mainTab,"Description");
		BeginAddTabItem(mainTab,tabId);

		YsLocale locale;
		auto langStr=FsOption::GetLanguageString();
		YSBOOL isMatchingLanguage=YSFALSE;
		for(auto &msg : iText)
		{
			if(YSTRUE==msg.MatchLanguage(FsOption::GetLanguageString()))
			{
				isMatchingLanguage=YSTRUE;
				break;
			}
		}
		if(YSTRUE!=isMatchingLanguage)
		{
			langStr="en";
		}


		for(int i=0; i<text.GetN(); i++)
		{
			AddStaticText(1,FSKEY_NULL,goal.text[i],YSTRUE);
		}
		for(auto &msg : iText)
		{
			if(YSTRUE==msg.MatchLanguage(langStr))
			{
				AddStaticText(1,FSKEY_NULL,msg.GetText(),YSTRUE);
			}
		}

		auto bmp=LoadBitmap(sim);
		if(0<bmp.GetWidth() && 0<bmp.GetHeight())
		{
			StepToNextColumn();
			AddStaticBmp(0,FSKEY_NULL,bmp,YSTRUE);
		}

		EndAddTabItem();
		return tabId;
	}
	return -1;
}

int FsGuiMissionGoalDialogClass::AddBitmapTab(const class FsSimulation *sim)
{
	auto bmp=LoadBitmap(sim);
	if(0<bmp.GetWidth() && 0<bmp.GetHeight())
	{
		const int tabId=AddTab(mainTab,"Supplimental Diagram");
		BeginAddTabItem(mainTab,tabId);

		AddStaticBmp(0,FSKEY_NULL,bmp,YSTRUE);

		EndAddTabItem();

		return tabId;
	}
	return -1;
}

int FsGuiMissionGoalDialogClass::AddDetailTab(const class FsSimulation *sim)
{
	const FsMissionGoal &goal=sim->GetMissionGoal();

	if(0!=goal.goalFlag)
	{
		const int tabId=AddTab(mainTab,"Mission Detail");
		BeginAddTabItem(mainTab,tabId);


		if(goal.goalFlag&(FSGOAL_DEFENDPRMGND|FSGOAL_DEFENDGND))
		{
			FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,"[Defend Ground Force]",YSTRUE);
			txt->SetFgColor(YsYellow());
		}
		if(goal.goalFlag&FSGOAL_DEFENDPRMGND)
		{
			YsString str;
			str.Printf("AT LEAST %d FACILITIES/VEHICLES MUST SURVIVE",goal.numPrmGndMustSurvive);
			AddStaticText(1,FSKEY_NULL,str,YSTRUE);
		}
		if(goal.goalFlag&FSGOAL_DEFENDGND)
		{
			for(int i=0; i<goal.defendGndName.GetN(); i++)
			{
				YsString str;
				str.Printf("DEFEND %s",(const char *)goal.defendGndName[i]);
				AddStaticText(1,FSKEY_NULL,str,YSTRUE);
			}
		}


		if((goal.goalFlag&FSGOAL_DEFENDAIR)!=0)
		{
			FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,"[Defend Aircraft]",YSTRUE);
			txt->SetFgColor(YsYellow());

			for(int i=0; i<goal.defendAirName.GetN(); i++)
			{
				YsString str;
				str.Printf("DEFEND %s\n",goal.defendAirName[i].Txt());
				AddStaticText(1,FSKEY_NULL,str,YSTRUE);
			}
		}


		if(goal.goalFlag&(FSGOAL_DESTROYPRMGND|FSGOAL_DESTROYGND))
		{
			FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,"[Air Strike]",YSTRUE);
			txt->SetFgColor(YsYellow());
		}
		if(goal.goalFlag&FSGOAL_DESTROYPRMGND)
		{
			YsString str;
			str.Printf("DESTROY AT LEAST %d ENEMY GROUND FACILITIES/VEHICLES",goal.numPrmGndMustBeDestroyed);
			AddStaticText(1,FSKEY_NULL,str,YSTRUE);
		}
		if(goal.goalFlag&FSGOAL_DESTROYGND)
		{
			for(int i=0; i<goal.destroyGndName.GetN(); i++)
			{
				YsString str;
				str.Printf("DESTROY %s",(const char *)goal.destroyGndName[i]);
				AddStaticText(1,FSKEY_NULL,str,YSTRUE);
			}
		}


		if(goal.goalFlag&(FSGOAL_DESTROYAIR|FSGOAL_DESTROYALLAIR))
		{
			FsGuiStatic *txt=AddStaticText(0,FSKEY_NULL,"[Intercept]",YSTRUE);
			txt->SetFgColor(YsYellow());

			if(goal.goalFlag&FSGOAL_DESTROYAIR)
			{
				for(int i=0; i<goal.destroyAirName.GetN(); i++)
				{
					YsString str;
					str.Printf("SHOOT DOWN %s",(const char *)goal.destroyAirName[i]);
					AddStaticText(0,FSKEY_NULL,str,YSTRUE);
				}
			}
			if(goal.goalFlag&FSGOAL_DESTROYALLAIR)
			{
				AddStaticText(0,FSKEY_NULL,"SHOOT DOWN ALL ENEMY AIRPLANES",YSTRUE);
			}
		}


		if(goal.goalFlag&FSGOAL_LAND)
		{
			FsGuiStatic *txt=AddStaticText(0,FSKEY_NULL,"[Landing]",YSTRUE);
			txt->SetFgColor(YsYellow());

			YsString str;
			if(goal.landRegionName[0]!=0 && goal.landCarrierName[0]!=0)
			{
				str.Printf("LAND AT %s, OR CARRIER %s (MAKE A COMPLETE STOP)",
				   (const char *)goal.landRegionName,(const char *)goal.landCarrierName);
			}
			else if(goal.landRegionName[0]!=0)
			{
				str.Printf("LAND AT %s (MAKE A COMPLETE STOP)",(const char *)goal.landRegionName);
			}
			else if(goal.landCarrierName[0]!=0)
			{
				str.Printf("LAND AT CARRIER %s (MAKE A COMPLETE STOP)",(const char *)goal.landCarrierName);
			}
			else
			{
				str.Set("LAND AND MAKE A COMPLETE STOP");
			}
			if(0<str.Strlen())
			{
				AddStaticText(0,FSKEY_NULL,str,YSTRUE);
			}
		}


		if(goal.goalFlag&FSGOAL_SURVIVE)
		{
			if(goal.goalFlag&(~FSGOAL_SURVIVE))
			{
				AddStaticText(0,FSKEY_NULL,"AND, COME BACK ALIVE.",YSTRUE);
			}
			else
			{
				AddStaticText(0,FSKEY_NULL,"COME BACK ALIVE.",YSTRUE);
			}
		}

		EndAddTabItem();
		return tabId;
	}
	return -1;
}

YsBitmap FsGuiMissionGoalDialogClass::LoadBitmap(const FsSimulation *sim)
{
	const FsMissionGoal &goal=sim->GetMissionGoal();
	auto &base64png=goal.GetBase64Png(0);

	if(0<base64png.GetN())
	{
		YsBase64Decoder decoder;
		YsArray <unsigned char> outBuf;
		for(auto &str : base64png)
		{
			decoder.DecodeString(outBuf,str);
		}
		decoder.Flush(outBuf);

		YsBitmap bmp;
		if(YSOK==bmp.LoadPng(outBuf.GetN(),outBuf) && 0<bmp.GetWidth() && 0<bmp.GetHeight())
		{
			return bmp;
		}
	}
	if(0!=goal.pngFn[0])
	{
		YsWString pth,fil,guessFn;
		goal.pngFn.SeparatePathFile(pth,fil);
		guessFn.MakeFullPathName(L"mission",fil);

		FILE *fp=NULL;


		fp=YsFileIO::Fopen(goal.pngFn,"rb");
		if(NULL==fp)
		{
			YsWString ful;
			ful.MakeFullPathName(FsGetUserYsflightDir(),goal.pngFn);
			fp=YsFileIO::Fopen(ful,"rb");

			YsString str;
			str.EncodeUTF8 <wchar_t> (ful);
			printf("Trying... %s\n",str.Txt());
		}
		if(NULL==fp)
		{
			YsWString progPath;
			YsSpecialPath::GetProgramBaseDir(progPath);

			YsWString ful;
			ful.MakeFullPathName(progPath,guessFn);
			fp=YsFileIO::Fopen(ful,"rb");

			YsString str;
			str.EncodeUTF8 <wchar_t> (ful);
			printf("Trying... %s\n",str.Txt());
		}

		if(NULL!=fp)
		{
			YsBitmap bmp;
			bmp.LoadPng(fp);
			fclose(fp);
			return bmp;
		}
	}

	YsBitmap empty;
	return empty;
}

////////////////////////////////////////////////////////////

FsGuiResultDialogClass::FsGuiResultDialogClass()
{
	landingProfileTDIndexCache=-1;
	landingProfileThresholdIndexCache=-1;
	rwRectCacheValid=YSFALSE;
}

FsGuiResultDialogClass::~FsGuiResultDialogClass()
{
}

FsGuiResultDialogClass *FsGuiResultDialogClass::Create(void)
{
	return new FsGuiResultDialogClass;
}

void FsGuiResultDialogClass::Delete(FsGuiResultDialogClass *ptr)
{
	delete ptr;
}

YSRESULT FsGuiResultDialogClass::Make(const class FsSimulation *sim,int nextActionCode)
{
	Initialize();

	this->sim=sim;

	this->landingDropList=NULL;
	this->landingVerticalSpeed=NULL;
	this->landingHeadingDifference=NULL;

	SetTransparency(YSFALSE);

	this->nextActionCode=nextActionCode;

	SetTextMessage("== Simulation Result ==");

	okBtn1=AddTextButton(MakeIdent("close"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSTRUE);

	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);
	missionGoalTabId=AddMissionGoalTab(sim);
	summaryTabId=AddSummaryTab(sim);
	scoreDetailTabId=AddScoreDetailTab(sim);
	landingAnalysisTabId=AddLandingAnalysisTab(sim);

	mainTab->SelectFirstTab();


	okBtn2=NULL; // AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CLOSE,YSTRUE);

	SetFocus(okBtn1);

	ExpandTab(mainTab);
	Fit();

	return YSOK;
}

void FsGuiResultDialogClass::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn1)
	{
		CloseModalDialog(nextActionCode);
	}
	if(NULL!=okBtn2 && btn==okBtn1)
	{
		CloseModalDialog(nextActionCode);
	}
}

int FsGuiResultDialogClass::AddMissionGoalTab(const class FsSimulation *sim)
{
	const FsMissionGoal &goal=sim->GetMissionGoal();

	if(YSTRUE==goal.IsActiveMission())
	{
		const int tabId=AddTab(mainTab,FSGUI_RESULTDLG_MISSIONGOAL);
		BeginAddTabItem(mainTab,tabId);


		YSBOOL overAllSucceed=goal.TestAllMissionGoalIsSatisfied(sim);
		if(YSTRUE==overAllSucceed)
		{
			FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_SUCCESS,YSTRUE);
			txt->SetFgColor(YsDarkGreen());
		}
		else
		{
			FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_FAILURE,YSTRUE);
			txt->SetFgColor(YsRed());
		}


		if(goal.TestMissionDuration(sim)!=YSTRUE && goal.TestSurvive(sim)==YSTRUE)
		{
			FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_FAILURE_EARLYTERMINATION,YSTRUE);
			txt->SetFgColor(YsRed());
		}

		if((goal.goalFlag&FSGOAL_SURVIVE)!=0 && goal.TestSurvive(sim)!=YSTRUE)
		{
			FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_FAILURE_KIA,YSTRUE);
			txt->SetFgColor(YsRed());
		}

		if((goal.goalFlag&FSGOAL_LAND)!=0)
		{
			if(goal.TestLanding(sim)!=YSTRUE)
			{
				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_FAILURE_MISSEDDESTINATION,YSTRUE);
				txt->SetFgColor(YsRed());
			}
			else if(overAllSucceed==YSTRUE)
			{
				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_SUCCESS_ARRIVED,YSTRUE);
				txt->SetFgColor(YsDarkGreen());
			}
		}

		if((goal.goalFlag&FSGOAL_DEFENDPRMGND)!=0)
		{
			int nGndSurvive;
			if(goal.TestDefendPrimaryGround(nGndSurvive,sim)!=YSTRUE)
			{
				YsWString str(FSGUI_RESULTDLG_FAILURE_GROUNDLOSS);
				YsFormatFirstInteger(str,nGndSurvive);
				YsFormatFirstInteger(str,goal.numPrmGndMustSurvive);

				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
				txt->SetFgColor(YsRed());
			}
			else if(overAllSucceed==YSTRUE)
			{
				YsWString str(FSGUI_RESULTDLG_SUCCEED_GROUNDSURVIVED);
				YsFormatFirstInteger(str,nGndSurvive);

				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
				txt->SetFgColor(YsDarkGreen());
			}
		}

		if((goal.goalFlag&FSGOAL_DEFENDGND)!=0)
		{
			if(goal.TestDefendGround(sim)!=YSTRUE)
			{
				for(int i=0; i<goal.defendGndName.GetN(); i++)
				{
					FsGround *gnd=NULL;
					while(NULL!=(gnd=sim->FindNextGround(gnd)))
					{
						if(strcmp(gnd->name,goal.defendGndName[i])==0 && gnd->GetFinalState()==FSGNDDEAD)
						{
							YsWString str(FSGUI_RESULTDLG_FAILURE_FAILEDPROTECTION);
							YsFormatFirstString <YsWString,char> (str,goal.defendGndName[i]);

							FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
							txt->SetFgColor(YsRed());
						}
					}
				}
			}
			else if(overAllSucceed==YSTRUE)
			{
				for(int i=0; i<goal.defendGndName.GetN(); i++)
				{
					YsWString str(FSGUI_RESULTDLG_SUCCESS_PROTECTED);
					YsFormatFirstString <YsWString,char> (str,goal.defendGndName[i]);

					FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
					txt->SetFgColor(YsDarkGreen());
				}
			}
		}

		if((goal.goalFlag&FSGOAL_DEFENDAIR)!=0)
		{
			if(goal.TestDefendAir(sim)!=YSTRUE)
			{
				for(int i=0; i<goal.defendAirName.GetN(); i++)
				{
					FsAirplane *air;
					air=NULL;
					while(NULL!=(air=sim->FindNextAirplane(air)))
					{
						FSFLIGHTSTATE sta;
						sta=air->GetFinalState();
						if(strcmp(air->name,goal.defendAirName[i])==0 && (sta==FSDEAD || sta==FSDEADFLATSPIN || sta==FSDEADSPIN))
						{
							YsWString str(FSGUI_RESULTDLG_FAILURE_FAILEDPROTECTION);
							YsFormatFirstString <YsWString,char> (str,goal.defendAirName[i]);

							FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
							txt->SetFgColor(YsRed());
						}
					}
				}
			}
			else if(overAllSucceed==YSTRUE)
			{
				for(int i=0; i<goal.defendAirName.GetN(); i++)
				{
					YsWString str(FSGUI_RESULTDLG_SUCCESS_PROTECTED);
					YsFormatFirstString <YsWString,char> (str,goal.defendAirName[i]);

					FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
					txt->SetFgColor(YsDarkGreen());
				}
			}
		}

		if((goal.goalFlag&FSGOAL_DESTROYPRMGND)!=0)
		{
			int nGndKill;
			if(goal.TestDestroyPrimaryGround(nGndKill,sim)!=YSTRUE)
			{
				YsWString str(FSGUI_RESULTDLG_FAILURE_AIRSTRIKEFAILURE);
				YsFormatFirstInteger(str,nGndKill);
				YsFormatFirstInteger(str,goal.numPrmGndMustBeDestroyed);

				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
				txt->SetFgColor(YsRed());
			}
			else if(overAllSucceed==YSTRUE)
			{
				YsWString str(FSGUI_RESULTDLG_SUCCESS_AIRSTRIKE);
				YsFormatFirstInteger(str,nGndKill);

				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
				txt->SetFgColor(YsDarkGreen());
			}
		}

		if((goal.goalFlag&FSGOAL_DESTROYGND)!=0)
		{
			if(goal.TestDestroyGround(sim)!=YSTRUE)
			{
				for(int i=0; i<goal.destroyGndName.GetN(); i++)
				{
					FsGround *gnd=NULL;
					while(NULL!=(gnd=sim->FindNextGround(gnd)))
					{
						if(strcmp(gnd->name,goal.destroyGndName[i])==0 && gnd->GetFinalState()!=FSGNDDEAD)
						{
							YsWString str(FSGUI_RESULTDLG_FAILURE_FAILEDTODESTROY);
							YsFormatFirstString <YsWString,char> (str,goal.destroyGndName[i]);

							FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
							txt->SetFgColor(YsRed());
						}
					}
				}
			}
			else if(overAllSucceed==YSTRUE)
			{
				for(int i=0; i<goal.destroyGndName.GetN(); i++)
				{
					YsWString str(FSGUI_RESULTDLG_SUCCESS_DESTROYED);
					YsFormatFirstString <YsWString,char> (str,goal.destroyGndName[i]);

					FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
					txt->SetFgColor(YsDarkGreen());
				}
			}
		}

		if((goal.goalFlag&FSGOAL_DESTROYAIR)!=0)
		{
			if(goal.TestDestroyAir(sim)!=YSTRUE)
			{
				for(int i=0; i<goal.destroyAirName.GetN(); i++)
				{
					FsAirplane *air;
					air=NULL;
					while(NULL!=(air=sim->FindNextAirplane(air)))
					{
						if(strcmp(air->name,goal.destroyAirName[i])==0)
						{
							FSFLIGHTSTATE sta;
							sta=air->GetFinalState();
							if(sta!=FSDEAD && sta!=FSDEADFLATSPIN && sta!=FSDEADSPIN)
							{
								YsWString str(FSGUI_RESULTDLG_FAILURE_FAILEDTOKILLAIR);
								YsFormatFirstString <YsWString,char> (str,goal.destroyAirName[i]);

								FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
								txt->SetFgColor(YsRed());
							}
						}
					}
				}
			}
			else if(overAllSucceed==YSTRUE)
			{
				for(int i=0; i<goal.destroyAirName.GetN(); i++)
				{
					YsWString str(FSGUI_RESULTDLG_SUCCESS_AIRKILL);
					YsFormatFirstString <YsWString,char> (str,goal.destroyAirName[i]);

					FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
					txt->SetFgColor(YsDarkGreen());
				}
			}
		}

		if((goal.goalFlag&FSGOAL_DESTROYALLAIR)!=0)
		{
			if(goal.TestDestroyAllAir(sim)!=YSTRUE)
			{
				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_FAILURE_FAILEDTOKILLALLAIR,YSTRUE);
				txt->SetFgColor(YsRed());
			}
			else
			{
				FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_SUCCESS_AIRKILLALL,YSTRUE);
				txt->SetFgColor(YsDarkGreen());
			}
		}


		if((goal.goalFlag&FSGOAL_MUSTLAND)!=0)
		{
			if(goal.TestMustLandAir(sim)!=YSTRUE)
			{
				const FsAirplane *air;
				air=NULL;
				while((air=sim->FindNextAirplane(air))!=NULL)
				{
					int i;
					forYsArray(i,goal.mustLandAirName)
					{
						if(strcmp(air->name,goal.mustLandAirName[i].airLabel)==0)
						{
							if(air->GetFinalState()!=FSGROUNDSTATIC && air->GetFinalState()!=FSGROUND)
							{
								YsWString str(FSGUI_RESULTDLG_FAILURE_FAILEDESCORT);
								YsFormatFirstString <YsWString,char> (str,goal.mustLandAirName[i].airLabel);

								FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
								txt->SetFgColor(YsRed());
							}
						}
					}
				}
			}
			else
			{
				for(int i=0; i<goal.mustLandAirName.GetN(); ++i)
				{
					YsWString str(FSGUI_RESULTDLG_SUCCESS_ESCORTED);
					YsFormatFirstString <YsWString,char> (str,goal.mustLandAirName[i].airLabel);

					FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
					txt->SetFgColor(YsDarkGreen());
				}
			}
		}


		EndAddTabItem();
		return tabId;
	}
	return -1;
}

const wchar_t *FsGuiResultDialogClass::FinalAirplaneStateString(YsColor &col,YsWString &str,FSFLIGHTSTATE sta) const
{
	switch(sta)
	{
	case FSFLYING:
	case FSSTALL:
		col=YsWhite();
		str.Set(FSGUI_RESULTDLG_FINALSTATE_FLYING);
		break;
	case FSGROUND:
		col=YsWhite();
		str.Set(FSGUI_RESULTDLG_FINALSTATE_LANDED);
		break;
	case FSGROUNDSTATIC:
		col=YsDarkGreen();
		str.Set(FSGUI_RESULTDLG_FINALSTATE_LANDEDSTATIC);
		break;
	case FSDEAD:
	case FSDEADSPIN:
	case FSDEADFLATSPIN:
		col=YsRed();
		str.Set(FSGUI_RESULTDLG_FINALSTATE_DEAD);
		break;
	case FSOVERRUN:
		str.Set(FSGUI_RESULTDLG_FINALSTATE_OVERRUN);
		col=YsRed();
		break;
	default:
		col=YsWhite();
		str.Set(FSGUI_RESULTDLG_FINALSTATE_UNKNOWN);
		break;
	}
	return str;
}

int FsGuiResultDialogClass::AddSummaryTab(const class FsSimulation *sim)
{
	const int tabId=AddTab(mainTab,FSGUI_RESULTDLG_SUMMARY);
	BeginAddTabItem(mainTab,tabId);


	if(NULL!=sim->GetPlayerAirplane())
	{
		const FsAirplane *playerPlane=sim->GetPlayerAirplane();

		YsWString str(FSGUI_RESULTDLG_USEDAIRCRAFT);
		YsFormatFirstString(str,playerPlane->Prop().GetIdentifier());
		AddStaticText(1,FSKEY_NULL,str,YSTRUE);


		YsWString finalStateStr;
		YsColor col;
		FinalAirplaneStateString(col,finalStateStr,playerPlane->GetFinalState());

		str.Set(FSGUI_RESULTDLG_FINALSTATE);
		YsFormatFirstString(str,finalStateStr.Txt());
		FsGuiStatic *txt=AddStaticText(1,FSKEY_NULL,str,YSTRUE);
		txt->SetFgColor(col);
	}
	else if(NULL!=sim->GetPlayerGround())
	{
		//const FsGround *playerGround=GetPlayerGround();
	}
	else
	{
		// No player vehicle?
	}

	if(NULL!=sim->GetPlayerObject())
	{
		int nAliveFacility=0;
		int nDestroyedFacility=0;
		FSIFF playerIff=sim->GetPlayerObject()->iff;

		FsGround *gndSeeker=NULL;
		while((gndSeeker=sim->FindNextGround(gndSeeker))!=NULL)
		{
			if(gndSeeker->primaryTarget==YSTRUE && gndSeeker->iff==playerIff)
			{
				if(gndSeeker->GetFinalState()==FSGNDALIVE)
				{
					nAliveFacility++;
				}
				else
				{
					nDestroyedFacility++;
				}
			}
		}

		AddStaticText(1,FSKEY_NULL,FSGUI_RESULTDLG_FACILITIES,YSTRUE);

		YsWString str(FSGUI_RESULTDLG_ALIVEDEAD);
		YsFormatFirstInteger(str,nAliveFacility);
		YsFormatFirstInteger(str,nDestroyedFacility);
		AddStaticText(1,FSKEY_NULL,str,YSFALSE);
	}

	{
		const FsWeaponHolder &bulletHolder=sim->GetWeaponStore();

		YsList <FsKillCredit> *kill;

		int airToGnd=0;
		int nGndKill=0;
		int airToAir=0;
		int nAirKill=0;
		int total=0;

		YSBOOL airCombat=YSFALSE;
		YSBOOL airStrike=YSFALSE;
		for(kill=bulletHolder.killCredit; kill!=NULL; kill=kill->Next())
		{
			int x=0;
			YsString msg;
			if((x=kill->dat.GetAntiAirScore(msg,sim,sim->GetPlayerObject()))!=0)
			{
				airToAir+=x;
				airCombat=YSTRUE;
				if(x>0)
				{
					nAirKill++;
				}
			}
			else if((x=kill->dat.GetAntiGroundScore(msg,sim,sim->GetPlayerObject()))!=0)
			{
				airToGnd+=x;
				airStrike=YSTRUE;
				if(x>0)
				{
					nGndKill++;
				}
			}
		}

		total=airToAir+airToGnd;

		if(airCombat==YSTRUE)
		{
			if(airToAir>=0)
			{
				YsWString str(FSGUI_RESULTDLG_AIRTOAIRSCORE);
				YsFormatFirstInteger(str,airToAir);
				AddStaticText(0,FSKEY_NULL,str,YSTRUE);
			}
			else
			{
				YsWString str(FSGUI_RESULTDLG_AIRTOAIRPENALTY);
				YsFormatFirstInteger(str,-airToAir);
				FsGuiStatic *txt=AddStaticText(0,FSKEY_NULL,str,YSTRUE);
				txt->SetFgColor(YsRed());
			}

			YsWString str(FSGUI_RESULTDLG_AIRTOAIRKILL);
			YsFormatFirstInteger(str,nAirKill);
			AddStaticText(0,FSKEY_NULL,str,YSTRUE);
		}
		else
		{
			AddStaticText(0,FSKEY_NULL,FSGUI_RESULTDLG_NOAIRCOMBAT,YSTRUE);
		}

		if(airStrike==YSTRUE)
		{
			if(airToGnd>=0)
			{
				YsWString str(FSGUI_RESULTDLG_AIRTOGNDSCORE);
				YsFormatFirstInteger(str,airToGnd);
				AddStaticText(0,FSKEY_NULL,str,YSTRUE);
			}
			else
			{
				YsWString str(FSGUI_RESULTDLG_AIRTOGNDPENALTY);
				YsFormatFirstInteger(str,-airToGnd);
				FsGuiStatic *txt=AddStaticText(0,FSKEY_NULL,str,YSTRUE);
				txt->SetFgColor(YsRed());
			}

			YsWString str(FSGUI_RESULTDLG_AIRTOGNDKILL);
			YsFormatFirstInteger(str,nGndKill);
			AddStaticText(0,FSKEY_NULL,str,YSTRUE);
		}
		else
		{
			AddStaticText(0,FSKEY_NULL,FSGUI_RESULTDLG_NOAIRSTRIKE,YSTRUE);
		}

		{
			YsWString str(FSGUI_RESULTDLG_TOTALSCORE);
			YsFormatFirstInteger(str,YsGreater(0,total));
			AddStaticText(0,FSKEY_NULL,str,YSTRUE);
		}
	}

	EndAddTabItem();
	return tabId;
}

int FsGuiResultDialogClass::AddScoreDetailTab(const class FsSimulation *sim)
{
	const int tabId=AddTab(mainTab,FSGUI_RESULTDLG_SCOREDETAIL);
	BeginAddTabItem(mainTab,tabId);

	const wchar_t *empty[]={NULL};
	FsGuiListBox *lbx=AddListBox(1,FSKEY_NULL,"",0,empty,16,48,YSTRUE);


	const FsWeaponHolder &bulletHolder=sim->GetWeaponStore();
	for(YsList <FsKillCredit> *kill=bulletHolder.killCredit; kill!=NULL; kill=kill->Next())
	{
		YsString msg;
		int x=0;
		if((x=kill->dat.GetAntiAirScore(msg,sim,sim->GetPlayerObject()))!=0)
		{
		}
		else if((x=kill->dat.GetAntiGroundScore(msg,sim,sim->GetPlayerObject()))!=0)
		{
		}

		if(x!=0)
		{
			const int id=lbx->AddString(msg,YSFALSE);
			if(0<x)
			{
				lbx->SetStringFgColor(id,YsDarkGreen());
			}
			else
			{
				lbx->SetStringFgColor(id,YsRed());
			}
		}
	}

	EndAddTabItem();
	return tabId;
}

int FsGuiResultDialogClass::AddLandingAnalysisTab(const class FsSimulation *sim)
{
	MakeLandingArray(sim);
	if(0<landingArray.GetN())
	{
		const int tabId=AddTab(mainTab,FSGUI_RESULTDLG_LANDINGANALYSIS);
		BeginAddTabItem(mainTab,tabId);
		AddLandingAnalysis(sim);
		EndAddTabItem();
		return tabId;
	}
	else
	{
		return -1;
	}
}

void FsGuiResultDialogClass::MakeLandingArray(const class FsSimulation *sim)
{
	landingArray.Clear();

	YsArray <FsSimulation::PlayerVehicleHistoryInfo> playerHist;
	sim->GetPlayerVehicleHistory(playerHist);
	for(YSSIZE_T histIdx=0; histIdx<playerHist.GetN(); ++histIdx)
	{
		if(FSEX_AIRPLANE==playerHist[histIdx].vehicle->GetType())
		{
			printf("[%d]\n",(int)histIdx);
			printf("%s from %.2lf sec to %.2lf sec\n",playerHist[histIdx].vehicle->GetIdentifier(),playerHist[histIdx].tStart,playerHist[histIdx].tEnd);

			const FsAirplane *air=(const FsAirplane *)playerHist[histIdx].vehicle;
			if(NULL==air->rec)
			{
				continue;
			}

			YsArray <YSSIZE_T> recordIndexArray;
			air->FindLandingWithinTimeRange(recordIndexArray,playerHist[histIdx].tStart,playerHist[histIdx].tEnd);
			printf("%d landings.\n",(int)recordIndexArray.GetN());

			for(YSSIZE_T idx=0; idx<recordIndexArray.GetN(); ++idx)
			{
				const YSSIZE_T recIdx=recordIndexArray[idx];

				double t1,t2;
				const FsFlightRecord *rec1=air->rec->GetElement(t1,recIdx-1);
				const FsFlightRecord *rec2=air->rec->GetElement(t2,recIdx);
				// Need to measure between recIdx-1 to recIdx.  As soon as the airplane lands, Y may be forced to the ground level, and the vertical speed may appear lower.
				if(NULL!=rec1 && NULL!=rec2)
				{
					YsVec3 pos1,pos2;
					pos1=rec1->pos;
					pos2=rec2->pos;

					const double dt=t2-t1;

					const YsVec3 vel=(pos2-pos1)/dt;
					printf("Time %.2lf Vertical Speed at touch down: %.2lf m/s\n",t1,vel.y());

					landingArray.Increment();
					landingArray.GetEnd().airSearchKey=air->SearchKey();
					landingArray.GetEnd().t=t1;
					landingArray.GetEnd().airIdentifier=air->GetIdentifier();
					landingArray.GetEnd().p=pos1;
					landingArray.GetEnd().v=vel;
					landingArray.GetEnd().hdg=rec1->h;
				}
			}
		}
	}
}

void FsGuiResultDialogClass::AddLandingAnalysis(const class FsSimulation *sim)
{
	landingDropList=AddEmptyDropList(0,FSKEY_NULL,"Landings",10,32,32,YSTRUE);
	AddStaticText(0,FSKEY_NULL,FSGUI_RESULTDLG_VERTICALSPEEDATTOUCHDOWN,32,1,YSTRUE);
	landingVerticalSpeed=AddStaticText(0,FSKEY_NULL,L"",16,1,YSFALSE);
	AddStaticText(0,FSKEY_NULL,FSGUI_RESULTDLG_CRABANGLEATTOUCHDOWN,32,1,YSTRUE);
	landingHeadingDifference=AddStaticText(0,FSKEY_NULL,L"",16,1,YSFALSE);

	for(YSSIZE_T ldgIdx=0; ldgIdx<landingArray.GetN(); ++ldgIdx)
	{
		YsString msg;
		msg.Printf("#%d at %.1lf sec in %s",ldgIdx+1,landingArray[ldgIdx].t,landingArray[ldgIdx].airIdentifier.Txt());
		landingDropList->AddString(msg,YSFALSE);
	}

	landingProfileViewSwitch[0]=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,L"Vertical Profile",YSTRUE);
	landingProfileViewSwitch[1]=AddTextButton(0,FSKEY_NULL,FSGUI_RADIOBUTTON,L"Horizontal Profile",YSFALSE);
	SetRadioButtonGroup(2,landingProfileViewSwitch);
	landingProfileViewSwitch[0]->SetCheck(YSTRUE);

	SelectLanding(sim,0);
}

void FsGuiResultDialogClass::SelectLanding(const FsSimulation *sim,int ldgIdx)
{
	if(YSTRUE==landingArray.IsInRange(ldgIdx))
	{
		landingDropList->Select(ldgIdx);

		YsString msg;
		msg.Printf(" : %.0lf ft/min",YsUnitConv::MtoFT(landingArray[ldgIdx].v.y()*60.0));
		landingVerticalSpeed->SetText(msg);

		YsVec2 gndTrk(landingArray[ldgIdx].v.x(),landingArray[ldgIdx].v.z());
		gndTrk.Normalize();
		YsVec2 hdgVec(-sin(landingArray[ldgIdx].hdg),cos(landingArray[ldgIdx].hdg));

		const double trkDiff=acos(gndTrk*hdgVec);
		msg.Printf(" : %.1lf deg",YsRadToDeg(trkDiff));

		landingHeadingDifference->SetText(msg);

		CacheVerticalProfile(sim,ldgIdx);
		CacheHorizontalProfileTransformation();
	}
}

void FsGuiResultDialogClass::CacheVerticalProfile(const FsSimulation *sim,int ldgIdx)
{
	landingProfileTDIndexCache=-1;
	landingProfileThresholdIndexCache=-1;
	landingVerticalProfileCache.Clear();
	landingHorizontalProfileCache.Clear();

	rwRectCacheValid=YSFALSE;

	if(YSTRUE==landingArray.IsInRange(ldgIdx))
	{
		const FsLandingAnalysis &ldgInfo=landingArray[ldgIdx];

		const FsAirplane *air=sim->FindAirplane(ldgInfo.airSearchKey);
		YSSIZE_T recIdx0;
		const double t0=ldgInfo.t;
		if(NULL!=air && YSOK==air->rec->GetIndexByTime(recIdx0,t0))
		{
			YSSIZE_T recBegin;
			for(recBegin=recIdx0; 0<recBegin; --recBegin)
			{
				double t;
				if(NULL==air->rec->GetElement(t,recBegin) || t<t0-40.0)
				{
					break;
				}
			}

			YSSIZE_T recEnd;
			for(recEnd=recIdx0; recEnd<air->rec->GetNumRecord(); ++recEnd)
			{
				double t;
				if(NULL==air->rec->GetElement(t,recEnd) || t0+10.0<t)
				{
					break;
				}
			}


			YsVec3 runwayRect3d[4];
			const YsSceneryRectRegion *rectRgnPtr;
			const YSRESULT useRunwayRect=sim->GetRunwayRectFromPosition(rectRgnPtr,runwayRect3d,ldgInfo.p);
			if(YSOK==useRunwayRect)
			{
				rwRectCacheValid=YSTRUE;
				rwRectCache[0].GetXZ(runwayRect3d[0]);
				rwRectCache[1].GetXZ(runwayRect3d[1]);
				rwRectCache[2].GetXZ(runwayRect3d[2]);
				rwRectCache[3].GetXZ(runwayRect3d[3]);
			}


			double accum=0.0;
			for(YSSIZE_T recIdx=recBegin; recIdx<recEnd-1; ++recIdx)
			{
				double t,u;
				const FsFlightRecord *rec0=air->rec->GetElement(t,recIdx);
				const FsFlightRecord *rec1=air->rec->GetElement(u,recIdx+1);
				if(NULL!=rec0)
				{
					if(recIdx0==recIdx)
					{
						landingProfileTDIndexCache=landingVerticalProfileCache.GetN();
					}
					const YsVec2 recPos=rec0->pos.xz(); // (rec0->x,rec0->z);
					if(0>=landingProfileThresholdIndexCache && YSOK==useRunwayRect)
					{
						if(YSINSIDE==YsCheckInsidePolygon2(recPos,4,rwRectCache))
						{
							landingProfileThresholdIndexCache=landingVerticalProfileCache.GetN();
						}
					}
					landingVerticalProfileCache.Increment();
					landingVerticalProfileCache.GetEnd().Set(accum,rec0->pos.y());

					landingHorizontalProfileCache.Append(recPos);
				}
				if(NULL!=rec0 && NULL!=rec1)
				{
					const double dx=rec1->pos.x()-rec0->pos.x();
					const double dz=rec1->pos.z()-rec0->pos.z();
					const double l=sqrt(dx*dx+dz*dz);
					accum+=l;
				}
			}
		}
	}
}

void FsGuiResultDialogClass::CacheHorizontalProfileTransformation(void)
{
	double minAngle=0.0;
	YsVec2 minDim=CalculateHorizontalProfileDimension(landingHorizontalProfileCache.GetN(),landingHorizontalProfileCache);

	YsArray <YsVec2> tmpPnt(landingHorizontalProfileCache.GetN(),NULL);

	for(int ang=0; ang<360; ang+=10)
	{
		const double radian=YsDegToRad((double)ang);
		for(YSSIZE_T idx=0; idx<tmpPnt.GetN(); ++idx)
		{
			tmpPnt[idx]=landingHorizontalProfileCache[idx];
			tmpPnt[idx].Rotate(radian);
		}

		const YsVec2 dim=CalculateHorizontalProfileDimension(tmpPnt.GetN(),tmpPnt);

		if(dim.y()<minDim.y())
		{
			minAngle=radian;
			minDim=dim;
		}
	}

	landingHorizontalProfileRotation=minAngle;
	landingHorizontalProfileDimension=minDim;

	landingHorizontalProfileTranslation.Set(0.0,0.0);
	for(YSSIZE_T idx=0; idx<landingHorizontalProfileCache.GetN(); ++idx)
	{
		landingHorizontalProfileTranslation+=landingHorizontalProfileCache[idx];
	}
	landingHorizontalProfileTranslation/=(double)landingHorizontalProfileCache.GetN();
}

YsVec2 FsGuiResultDialogClass::CalculateHorizontalProfileDimension(YSSIZE_T np,const YsVec2 p[])
{
	YsBoundingBoxMaker2 mkBbx;
	for(YSSIZE_T idx=0; idx<np; ++idx)
	{
		mkBbx.Add(p[idx]);
	}

	YsVec2 bbx[2];
	mkBbx.Get(bbx[0],bbx[1]);
	return bbx[1]-bbx[0];
}

void FsGuiResultDialogClass::OnDropListSelChange(FsGuiDropList *drp,int /*prevSel*/)
{
	if(drp==landingDropList)
	{
		SelectLanding(sim,drp->GetSelection());
	}
}

void FsGuiResultDialogClass::Show(const FsGuiDialog *excludeFromDrawing) const
{
	if(landingAnalysisTabId==mainTab->GetCurrentTab() && 0<landingVerticalProfileCache.GetN())
	{
		int wid,hei;
		FsGetWindowSize(wid,hei);

		const int x0=32;
		const int y0=this->dlgY0+this->GetHeight()+16;
		const int x1=wid-32;
		const int y1=hei-32;

		FsDrawRect(x0-16,y0-16,x1+16,y1+16,YsBlack(),YSTRUE);

		YsColor darkGray;
		darkGray.SetIntRGB(40,40,40);
		for(int x=0; x<=10; ++x)
		{
			int xx=x0+(x1-x0)*x/10;
			FsDrawLine(xx,y0,xx,y1,darkGray);
		}
		for(int y=0; y<=10; ++y)
		{
			int yy=y0+(y1-y0)*y/10;
			FsDrawLine(x0,yy,x1,yy,darkGray);
		}


		if(YSTRUE==landingProfileViewSwitch[0]->GetCheck())
		{
			double tdY0=0.0;
			if(YSTRUE==landingVerticalProfileCache.IsInRange(landingProfileTDIndexCache))
			{
				tdY0=landingVerticalProfileCache[landingProfileTDIndexCache].y();
			}


			const double lTotal=landingVerticalProfileCache.GetEnd().x();
			// lTotal maps to x1-x0
			const double xScale=(double)(x1-x0)/lTotal;

			const double yScale=xScale*3.0;


			if(YSTRUE==landingVerticalProfileCache.IsInRange(landingProfileThresholdIndexCache))
			{
				const double thrX0=landingVerticalProfileCache[landingProfileThresholdIndexCache].x();
				const double thrY0=YsUnitConv::FTtoM(30.0); // landingVerticalProfileCache[landingProfileThresholdIndexCache].y()-tdY0;

				const int lineX1=x0+(int)(thrX0*xScale);
				const int lineY1=y1-(int)(thrY0*yScale);

				double glideSlopeY0=thrY0+thrX0*YsTan3deg;

				const int lineX0=x0;
				const int lineY0=y1-(int)(glideSlopeY0*yScale);

				FsDrawLine(lineX0,lineY0,lineX1,lineY1,YsBlue());


				const double glideSlopeXExt=thrY0/YsTan3deg;
				const int lineX2=x0+(int)((thrX0+glideSlopeXExt)*xScale);
				const int lineY2=y1-(int)(tdY0*yScale);

				FsDrawLine(lineX1,lineY1,lineX2,lineY2,YsBlue());


				// Virtual runway rect
				const int rwX0=lineX1;
				const int rwY0=lineY2;
				FsDrawLine(rwX0-10,rwY0-10,rwX0+10,rwY0+10,YsDarkGreen());
				FsDrawLine(rwX0-10,rwY0-10,x1,rwY0-10,YsDarkGreen());
				FsDrawLine(rwX0+10,rwY0+10,x1,rwY0+10,YsDarkGreen());
			}
			else if(YSTRUE==landingVerticalProfileCache.IsInRange(landingProfileTDIndexCache)) // Use touch-down point as reference instead
			{
				const double thrX0=landingVerticalProfileCache[landingProfileTDIndexCache].x();
				const double thrY0=landingVerticalProfileCache[landingProfileTDIndexCache].y()-tdY0;

				const int lineX1=x0+(int)(thrX0*xScale);
				const int lineY1=y1-(int)(thrY0*yScale);

				double glideSlopeY0=thrY0+thrX0*YsTan3deg;

				const int lineX0=x0;
				const int lineY0=y1-(int)(glideSlopeY0*yScale);

				FsDrawLine(lineX0,lineY0,lineX1,lineY1,YsBlue());
			}

			int lineX1=x0+(int)(landingVerticalProfileCache[0].x()*xScale);
			int lineY1=y1-(int)((landingVerticalProfileCache[0].y()-tdY0)*yScale);
			for(YSSIZE_T idx=1; idx<landingVerticalProfileCache.GetN()-1; ++idx)
			{
				int lineX0=lineX1;
				int lineY0=lineY1;

				lineX1=x0+(int)(landingVerticalProfileCache[idx].x()*xScale);
				lineY1=y1-(int)((landingVerticalProfileCache[idx].y()-tdY0)*yScale);

				FsDrawLine(lineX0,lineY0,lineX1,lineY1,YsWhite());
			}

			if(YSTRUE==landingVerticalProfileCache.IsInRange(landingProfileTDIndexCache))
			{
				int triX0=x0+(int)(landingVerticalProfileCache[landingProfileTDIndexCache].x()*xScale);
				int triY0=y1-(int)((landingVerticalProfileCache[landingProfileTDIndexCache].y()-tdY0)*yScale);

				int plg[6]=
				{
					triX0,triY0,
					triX0-10,triY0-17,
					triX0+10,triY0-17
				};
				FsDrawPolygon(3,plg,YsWhite());
			}
		}
		else
		{
			double scale=1.0;

			// profAspect=landingHorizontalProfileDimension.x()/landingHorizontalProfileDimension.y()
			// windowAspect=(double)(x1-x0)/(double)(y1-y0)
			// if(windowAspect<profAspect)
			if((double)(x1-x0)*landingHorizontalProfileDimension.y()<landingHorizontalProfileDimension.x()*(double)(y1-y0))
			{
				scale=(double)(x1-x0)/landingHorizontalProfileDimension.x();
			}
			else
			{
				scale=(double)(y1-y0)/landingHorizontalProfileDimension.y();
			}

			const int cx=(x0+x1)/2;
			const int cy=(y0+y1)/2;

			if(YSTRUE==rwRectCacheValid)
			{
				YsVec2 rwRect[4];
				for(YSSIZE_T idx=0; idx<4; ++idx)
				{
					rwRect[idx]=(rwRectCache[idx]-landingHorizontalProfileTranslation)*scale;
					rwRect[idx].Rotate(landingHorizontalProfileRotation);
				}
				FsDrawLine(cx+(int)rwRect[0].x(),cy-(int)rwRect[0].y(),cx+(int)rwRect[1].x(),cy-(int)rwRect[1].y(),YsDarkGreen());
				FsDrawLine(cx+(int)rwRect[1].x(),cy-(int)rwRect[1].y(),cx+(int)rwRect[2].x(),cy-(int)rwRect[2].y(),YsDarkGreen());
				FsDrawLine(cx+(int)rwRect[2].x(),cy-(int)rwRect[2].y(),cx+(int)rwRect[3].x(),cy-(int)rwRect[3].y(),YsDarkGreen());
				FsDrawLine(cx+(int)rwRect[3].x(),cy-(int)rwRect[3].y(),cx+(int)rwRect[0].x(),cy-(int)rwRect[0].y(),YsDarkGreen());
			}

			for(YSSIZE_T idx=0; idx<landingHorizontalProfileCache.GetN()-1; ++idx)
			{
				YsVec2 p1=(landingHorizontalProfileCache[idx  ]-landingHorizontalProfileTranslation)*scale;
				YsVec2 p2=(landingHorizontalProfileCache[idx+1]-landingHorizontalProfileTranslation)*scale;

				p1.Rotate(landingHorizontalProfileRotation);
				p2.Rotate(landingHorizontalProfileRotation);

				FsDrawLine(cx+(int)p1.x(),cy-(int)p1.y(),cx+(int)p2.x(),cy-(int)p2.y(),YsWhite());
			}

			if(YSTRUE==landingVerticalProfileCache.IsInRange(landingProfileTDIndexCache))
			{
				YsVec2 p=(landingHorizontalProfileCache[landingProfileTDIndexCache]-landingHorizontalProfileTranslation)*scale;

				p.Rotate(landingHorizontalProfileRotation);

				int triX0=cx+(int)p.x();
				int triY0=cy-(int)p.y();

				int plg[6]=
				{
					triX0,triY0,
					triX0-10,triY0-17,
					triX0+10,triY0-17
				};
				FsDrawPolygon(3,plg,YsWhite());
			}
		}
	}

	FsGuiDialog::Show(excludeFromDrawing);
}

