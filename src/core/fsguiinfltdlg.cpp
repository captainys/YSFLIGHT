#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <ysclass.h>
#include <ysunitconv.h>

#ifdef _WIN32
// Visual C++ stupidity.
#define _WINSOCKAPI_
#endif

#include <fsgui.h>
#include <fssimplewindow.h>
#include "platform/common/fswindow.h"
#include "fsguiinfltdlg.h"

#include "fstextresource.h"

#include "fsfilename.h"
#include "graphics/common/fsopengl.h"

#include "fsconfig.h"

#include "fsexistence.h"
#include "fssimulation.h"
#include "fsworld.h"
#include "fsautopilot.h"
#include "fsautodrive.h"

/* virtual */ YSBOOL FsGuiInFlightDialog::ProcessRawKeyInput(int)
{
	return YSFALSE;
}
/* virtual */ YSBOOL FsGuiInFlightDialog::WouldProcessNumberKey(void) const
{
	return YSFALSE;
}
/* virtual */ YSBOOL FsGuiInFlightDialog::WouldProcessThisRawKey(int ) const
{
	return YSFALSE;
}
/* virtual */ void FsGuiInFlightDialog::UpdateDialog(void)
{
}

////////////////////////////////////////////////////////////

/* virtual */ YSBOOL FsGuiInFlightDialogThatProcessNumberKey::WouldProcessNumberKey(void) const
{
	return YSTRUE;
}

////////////////////////////////////////////////////////////

/* virtual */ YSBOOL FsGuiInFlightDialogThatProcessNumberKeyAndEsc::WouldProcessNumberKey(void) const
{
	return YSTRUE;
}
/* virtual */ YSBOOL FsGuiInFlightDialogThatProcessNumberKeyAndEsc::WouldProcessThisRawKey(int rawKey) const
{
	if(FSKEY_ESC==rawKey)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

////////////////////////////////////////////////////////////

FsGuiInFlightDialogWithNonConstantTextButton::FsGuiInFlightDialogWithNonConstantTextButton()
{
	prevPageBtn=NULL;
	nextPageBtn=NULL;
}
void FsGuiInFlightDialogWithNonConstantTextButton::Reset(void)
{
	nChoice=0;
	baseIndex=0;
}
void FsGuiInFlightDialogWithNonConstantTextButton::SetNumChoice(YSSIZE_T nc)
{
	nChoice=nc;
}
void FsGuiInFlightDialogWithNonConstantTextButton::MakeVariableTextButton(YSSIZE_T nBtn)
{
	varBtnArray.CleanUp();
	for(decltype(nBtn) i=0; i<nBtn; ++i)
	{
		varBtnArray.Append(AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"...",YSTRUE));
	}
}
void FsGuiInFlightDialogWithNonConstantTextButton::MakePageButton(void)
{
	prevPageBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"<<Prev",YSTRUE);
	nextPageBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Next>>",YSFALSE);
}
YSBOOL FsGuiInFlightDialogWithNonConstantTextButton::UpdateText(YSSIZE_T btnIdx,const char str[])
{
	if(YSTRUE==varBtnArray.IsInRange(btnIdx))
	{
		auto &curLabel=varBtnArray[btnIdx]->GetLabel();
		YsString utf8;
		utf8.EncodeUTF8(curLabel.Txt());

		if(0!=strcmp(str,utf8))
		{
			varBtnArray[btnIdx]->SetText(str);
			return YSTRUE;
		}
	}
	return YSFALSE;
}

void FsGuiInFlightDialogWithNonConstantTextButton::NextPage(void)
{
	if(nChoice<=varBtnArray.GetN())
	{
		baseIndex=0;
	}
	else
	{
		baseIndex+=varBtnArray.GetN();
		if(nChoice<=baseIndex)
		{
			baseIndex=0;
		}
	}
}
void FsGuiInFlightDialogWithNonConstantTextButton::PrevPage(void)
{
	if(nChoice<=varBtnArray.GetN())
	{
		baseIndex=0;
	}
	else if(0==baseIndex)
	{
		if(0<varBtnArray.GetN())
		{
			baseIndex=((nChoice-1)/varBtnArray.GetN())*varBtnArray.GetN();
		}
	}
	else if(baseIndex<varBtnArray.GetN())
	{
		baseIndex=0;
	}
	else
	{
		baseIndex-=varBtnArray.GetN();
	}
}
int FsGuiInFlightDialogWithNonConstantTextButton::NShow(void) const
{
	return (int)varBtnArray.GetN();
}
FsGuiButton *FsGuiInFlightDialogWithNonConstantTextButton::NextPageButton(void)
{
	return nextPageBtn;
}
const FsGuiButton *FsGuiInFlightDialogWithNonConstantTextButton::NextPageButton(void) const
{
	return nextPageBtn;
}
FsGuiButton *FsGuiInFlightDialogWithNonConstantTextButton::PrevPageButton(void)
{
	return prevPageBtn;
}
const FsGuiButton *FsGuiInFlightDialogWithNonConstantTextButton::PrevPageButton(void) const
{
	return prevPageBtn;
}

int FsGuiInFlightDialogWithNonConstantTextButton::ClickedSelection(FsGuiButton *btn) const
{
	for(auto idx : varBtnArray.AllIndex())
	{
		if(btn==varBtnArray[idx])
		{
			return (int)idx;
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////

FsReplayDialog::FsReplayDialog()
{
}

FsReplayDialog::~FsReplayDialog()
{
}

FsReplayDialog *FsReplayDialog::Create(void)
{
	return new FsReplayDialog;
}

void FsReplayDialog::Delete(FsReplayDialog *toDel)
{
	delete toDel;
}

void FsReplayDialog::MakeForReplay(FsSimulation *sim)
{
	// [Time] [Jump]
	// <<< << < <0.1 || 0.1> > >> >>>
	// Hide

	this->sim=sim;
	resume=YSFALSE;

	timeText[0]=AddTextBox(0,FSKEY_NULL,"TIME","0",6,YSTRUE);
	timeText[0]->SetTextType(FSGUI_REALNUMBER);
	jumpButton[0]=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"JUMP",YSFALSE);
	captureButton[0]=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"CAPTURE",YSFALSE);

	timeText[1]=NULL;
	jumpButton[1]=NULL;
	captureButton[1]=NULL;

	trimButton=NULL;
	deleteButton=NULL;

	rewindAllTheWay=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"|<",YSTRUE);
	veryFastRewind=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"<<<",YSFALSE);
	fastRewind=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON," <<",YSFALSE);
	backward=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"  <",YSFALSE);
	stepBack=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"<0.1",YSFALSE);
	pause=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"||",YSFALSE);
	stepPlay=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"0.1>",YSFALSE);
	play=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">  ",YSFALSE);
	fastForward=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">> ",YSFALSE);
	veryFastForward=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">>>",YSFALSE);
	forwardAllTheWay=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">|",YSFALSE);

	resumeBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Resume Flight",YSTRUE);
	confirmResumeBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Confirm Resume",YSFALSE);
	confirmResumeBtn->Disable();

	hideBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"HIDE (ENTER key to show)",YSTRUE);

	Fit();
	SetTransparency(YSTRUE);

	int wid,hei;
	FsGetWindowSize(wid,hei);

	Move(0,hei-GetHeight());
}

void FsReplayDialog::MakeForEdit(FsSimulation *sim)
{
	// [Time] [Jump]
	// <<< << < <0.1 || 0.1> > >> >>>
	// Hide

	this->sim=sim;
	resume=YSFALSE;

	timeText[0]=AddTextBox(0,FSKEY_NULL,"TIME0","0",6,YSTRUE);
	timeText[0]->SetTextType(FSGUI_REALNUMBER);
	jumpButton[0]=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"JUMP",YSFALSE);
	captureButton[0]=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"CAPTURE",YSFALSE);

	timeText[1]=AddTextBox(0,FSKEY_NULL,"TIME1","0",6,YSTRUE);
	timeText[1]->SetTextType(FSGUI_REALNUMBER);
	jumpButton[1]=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"JUMP",YSFALSE);;
	captureButton[1]=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"CAPTURE",YSFALSE);

	trimButton=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"TRIM",YSTRUE);
	deleteButton=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"DELETE",YSFALSE);;

	rewindAllTheWay=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"|<",YSTRUE);
	veryFastRewind=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"<<<",YSFALSE);
	fastRewind=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON," <<",YSFALSE);
	backward=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"  <",YSFALSE);
	stepBack=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"<0.05",YSFALSE);
	pause=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"||",YSFALSE);
	stepPlay=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"0.05>",YSFALSE);
	play=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">  ",YSFALSE);
	fastForward=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">> ",YSFALSE);
	veryFastForward=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">>>",YSFALSE);
	forwardAllTheWay=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,">|",YSFALSE);

	resumeBtn=NULL;
	confirmResumeBtn=NULL;

	hideBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"HIDE (ENTER key to show)",YSTRUE);

	Fit();
	SetTransparency(YSTRUE);

	int wid,hei;
	FsGetWindowSize(wid,hei);

	Move(0,hei-GetHeight());
}

YSBOOL FsReplayDialog::IsTakingNumberKey(void) const
{
	if((NULL!=timeText[0] && GetFocus()==timeText[0]) ||
	   (NULL!=timeText[1] && GetFocus()==timeText[1]))
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void FsReplayDialog::OnButtonClick(FsGuiButton *btn)
{
	int i;
	for(i=0; i<2; i++)
	{
		if(btn==jumpButton[i])
		{
			double t;
			t=timeText[i]->GetRealNumber();
			sim->world->Jump(t);
		}
		else if(btn==captureButton[i])
		{
			double t;
			t=sim->GetClock();
			timeText[i]->SetRealNumber(t,2);
		}
	}

	if(btn==trimButton)
	{
		double ct;
		ct=sim->GetClock();

		char msg[256];
		double t1,t2;
		t1=timeText[0]->GetRealNumber();
		t2=timeText[1]->GetRealNumber();;
		if(t2<t1)
		{
			YsSwapSomething <double> (t1,t2);
		}
		sprintf(msg,"Delete beginning to %.2lf and %.2lf to end?",t1,t2);
		if(FsYesNoDialog(msg,"")==YSTRUE)
		{
			sim->world->DeleteFlightRecord(t2,YsInfinity);
			sim->world->DeleteFlightRecord(0.0,t1);
			if(ct<t1)
			{
				sim->world->Jump(0.0);
			}
			else if(ct<t2)
			{
				sim->world->Jump(ct-t1);
			}
			else if(t2<ct)
			{
				sim->world->Jump(t2-t1);
			}
		}
	}
	else if(btn==deleteButton)
	{
		double ct;
		ct=sim->GetClock();

		char msg[256];
		double t1,t2;
		t1=timeText[0]->GetRealNumber();
		t2=timeText[1]->GetRealNumber();;
		if(t2<t1)
		{
			YsSwapSomething <double> (t1,t2);
		}
		sprintf(msg,"Delete %.2lf to %.2lf?",t1,t2);
		if(FsYesNoDialog(msg,"")==YSTRUE)
		{
			sim->world->DeleteFlightRecord(t1,t2);
			if(t2<=ct)
			{
				ct-=(t2-t1);
				sim->world->Jump(ct);
			}
			else if(t1<=ct)
			{
				sim->world->Jump(ct);
			}
		}
	}
	else if(btn==rewindAllTheWay)
	{
		sim->world->JumpToFirstRecordTime();
	}
	else if(btn==veryFastRewind)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_VERYFASTREWIND);
	}
	else if(btn==fastRewind)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_FASTREWIND);
	}
	else if(btn==backward)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_BACKWARD);
	}
	else if(btn==stepBack)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_STEPBACK);
	}
	else if(btn==pause)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_PAUSE);
	}
	else if(btn==stepPlay)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_STEPFORWARD);
	}
	else if(btn==play)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_PLAY);
	}
	else if(btn==fastForward)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_FASTFORWARD);
	}
	else if(btn==veryFastForward)
	{
		sim->SetReplayMode(FsSimulation::FSREPLAY_VERYFASTFORWARD);
	}
	else if(btn==forwardAllTheWay)
	{
		sim->world->JumpToLastRecordTime();
	}
	else if(btn==hideBtn)
	{
		sim->showReplayDlg=YSFALSE;
	}

	else if(btn==resumeBtn)
	{
		confirmResumeBtn->Enable();
	}
	else if(btn==confirmResumeBtn)
	{
		resume=YSTRUE;
	}
	else
	{
		if(NULL!=confirmResumeBtn)
		{
			confirmResumeBtn->Disable();
		}
	}
}



////////////////////////////////////////////////////////////



FsContinueDialog::FsContinueDialog()
{
}

FsContinueDialog::~FsContinueDialog()
{
}

FsContinueDialog *FsContinueDialog::Create(void)
{
	return new FsContinueDialog;
}

void FsContinueDialog::Delete(FsContinueDialog *toDel)
{
	delete toDel;
}

void FsContinueDialog::OnButtonClick(FsGuiButton *)
{
}

void FsContinueDialog::MakeDialog(FsSimulation *sim)
{
	this->sim=sim;

	SetTextMessage("CONTINUE FLIGHT?");
	

	char str[256];
	sprintf(str,"TIME: %.2lf",sim->currentTime);
	timeTxt=AddStaticText(0,FSKEY_NULL,str,60,1,YSTRUE);
	timeTxt->SetDrawFrame(YSFALSE);
	timeTxt->SetFill(YSFALSE);

	YsString plrState;
	plrState.Set("PLAYER STATE: ");
	if(sim->GetPlayerAirplane()!=NULL)
	{
		plrState.Append(sim->FinalAirplaneStateString(sim->GetPlayerAirplane()->Prop().GetFlightState()));
	}
	playerStateTxt=AddStaticText(0,FSKEY_NULL,plrState,60,1,YSTRUE);
	playerStateTxt->SetDrawFrame(YSFALSE);
	playerStateTxt->SetFill(YSFALSE);

	queryTxt=AddStaticText(0,FSKEY_NULL,FSGUI_CONT_MESSAGE,YSTRUE);

	contButton=AddTextButton(1,FSKEY_Y,FSGUI_PUSHBUTTON,FSGUI_CONT_CONTINUE,YSTRUE);
	endButton=AddTextButton(1,FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_CONT_TERMINATE,YSFALSE);

	SetBackgroundAlpha(0.5);

	Fit();
}



////////////////////////////////////////////////////////////



FsGuiChatDialog::FsGuiChatDialog()
{
}

FsGuiChatDialog::~FsGuiChatDialog()
{
}

FsGuiChatDialog *FsGuiChatDialog::Create(void)
{
	return new FsGuiChatDialog;
}

void FsGuiChatDialog::Delete(FsGuiChatDialog *toDel)
{
	delete toDel;
}

void FsGuiChatDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==sendBtn)
	{
		YsString str;
		chatMsg->GetText(str);
		sim->SendNetChatMessage(str);
		chatMsg->SetText("");
		sim->CloseChatDialog();
	}
	else if(btn==cancelBtn)
	{
		sim->CloseChatDialog();
	}
	else if(btn==clearBtn)
	{
		chatMsg->SetText("");
	}
}

void FsGuiChatDialog::MakeDialog(FsSimulation *sim)
{
	this->sim=sim;
	chatMsg=AddTextBox(0,FSKEY_NULL,"","",40,YSTRUE);
	chatMsg->SetLengthLimit(140);
	chatMsg->SetEatEnterKey(YSFALSE); // Don't take enter key.
	sendBtn=AddTextButton(0,FSKEY_ENTER,FSGUI_PUSHBUTTON,"Send",YSFALSE);
	cancelBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,"Cancel",YSFALSE);
	clearBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,"Clear",YSFALSE);
	Fit();
	SetBackgroundAlpha(0.5);
}



////////////////////////////////////////////////////////////



FsGuiStationaryDialog::FsGuiStationaryDialog()
{
	endButton=NULL;
	confirmEndButton=NULL;
	supplyButton=NULL;
	changeVehicleButton=NULL;

	confirmEndButtonPressed=YSFALSE;
	supplyButtonPressed=YSFALSE;
	changeVehicleButtonPressed=YSFALSE;
}

FsGuiStationaryDialog::~FsGuiStationaryDialog()
{
}

FsGuiStationaryDialog *FsGuiStationaryDialog::Create(void)
{
	return new FsGuiStationaryDialog;
}

void FsGuiStationaryDialog::Delete(FsGuiStationaryDialog *toDel)
{
	delete toDel;
}

void FsGuiStationaryDialog::Make(class FsSimulation *sim)
{
	this->sim=sim;

	endButton=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_STATIONARYDLG_ENDFLIGHT,YSTRUE);
	confirmEndButton=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_STATIONARYDLG_CONFIRMENDFLIGHT,YSFALSE);

	AddStaticText(0,FSKEY_NULL,L"                ",YSFALSE);

	supplyButton=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_STATIONARYDLG_SUPPLY,YSFALSE);
	changeVehicleButton=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_STATIONARYDLG_CHANGEVEHICLE,YSFALSE);

	Fit();
	SetBackgroundAlpha(0.5);
}

void FsGuiStationaryDialog::SetUp(YSBOOL canSupply,YSBOOL canChangeVehicle)
{
	confirmEndButtonPressed=YSFALSE;
	supplyButtonPressed=YSFALSE;
	changeVehicleButtonPressed=YSFALSE;

	endButton->Enable();
	confirmEndButton->Disable();
	supplyButton->SetEnabled(canSupply);
	changeVehicleButton->SetEnabled(canChangeVehicle);

	int wid,hei;
	FsGetWindowSize(wid,hei);

	Move(wid/2-GetWidth()/2,0);
}

YSBOOL FsGuiStationaryDialog::CheckConfirmEndButtonPressed(void) const
{
	return confirmEndButtonPressed;
}

YSBOOL FsGuiStationaryDialog::CheckSupplyButtonPressed(void) const
{
	return supplyButtonPressed;
}

YSBOOL FsGuiStationaryDialog::CheckChangeVehicleButtonPressed(void) const
{
	return changeVehicleButtonPressed;
}

void FsGuiStationaryDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==endButton)
	{
		confirmEndButton->Enable();
	}
	if(btn==confirmEndButton)
	{
		confirmEndButtonPressed=YSTRUE;
		sim->SetTerminate(YSTRUE);
	}
	if(btn==supplyButton)
	{
		supplyButtonPressed=YSTRUE;

		const FsAirplane *playerAir=sim->GetPlayerAirplane();
		if(NULL!=playerAir)
		{
			YSBOOL fuel,ammo;
			if(NULL!=sim->FindNearbySupplyTruck(fuel,ammo,*playerAir))
			{
				sim->OpenLoadingDialog(fuel,ammo,*playerAir);
			}
		}
	}
	if(btn==changeVehicleButton)
	{
		changeVehicleButtonPressed=YSTRUE;
		sim->OpenVehicleChangeDialog();
	}
}



////////////////////////////////////////////////////////////



FsGuiVehicleChangeDialog::FsGuiVehicleChangeDialog()
{
	okBtn=NULL;
	cancelBtn=NULL;
	vehicleList=NULL;
	sim=NULL;
}

FsGuiVehicleChangeDialog::~FsGuiVehicleChangeDialog()
{
}

FsGuiVehicleChangeDialog *FsGuiVehicleChangeDialog::Create(void)
{
	return new FsGuiVehicleChangeDialog;
}

void FsGuiVehicleChangeDialog::Delete(FsGuiVehicleChangeDialog *toDel)
{
	delete toDel;
}

void FsGuiVehicleChangeDialog::Make(class FsSimulation *sim)
{
	this->sim=sim;

	okBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);
	vehicleList=AddEmptyListBox(0,FSKEY_NULL,"Vehicle",20,20,YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}

void FsGuiVehicleChangeDialog::SetUp(YSSIZE_T nObj,const FsExistence * const obj[])
{
	YsArray <const char *> nameArray(nObj,NULL);
	for(int idx=0; idx<nObj; ++idx)
	{
		nameArray[idx]=obj[idx]->GetIdentifier();
	}

	vehicleList->SetChoice(nObj,nameArray);
	for(int idx=0; idx<nObj; ++idx)
	{
		vehicleList->SetIntAttrib(idx,obj[idx]->SearchKey());
	}

	if(0<nObj)
	{
		vehicleList->Select(0);
	}
}

void FsGuiVehicleChangeDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==okBtn)
	{
		const int sel=vehicleList->GetSelection();
		if(0<=sel)
		{
			const int key=vehicleList->GetIntAttrib(sel);
			sim->PlayerChangeVehicleIfPossible(key);
			sim->CloseVehicleChangeDialog();
		}
	}
	else if(btn==cancelBtn)
	{
		sim->CloseVehicleChangeDialog();
	}
}

////////////////////////////////////////////////////////////

FsGuiAutoPilotDialog::FsGuiAutoPilotDialog()
{
	sim=NULL;
}
FsGuiAutoPilotDialog::~FsGuiAutoPilotDialog()
{
}
/* static */ FsGuiAutoPilotDialog *FsGuiAutoPilotDialog::Create(void)
{
	return new FsGuiAutoPilotDialog;
}
/* static */ void FsGuiAutoPilotDialog::Delete(FsGuiAutoPilotDialog *ptr)
{
	delete ptr;
}
void FsGuiAutoPilotDialog::Make(FsSimulation *sim)
{
	FsGuiDialog::Initialize();

	this->sim=sim;

	YsWString msg;

	msg=L"1...";
	msg.Append(FSGUI_DLG_AUTOPILOT_CIRCLE);
	circleBtn=AddTextButton(0,FSKEY_1,FSGUI_PUSHBUTTON,msg,YSTRUE);

	msg=L"2...";
	msg.Append(FSGUI_DLG_AUTOPILOT_STRAIGHT_LEVEL);
	straightBtn=AddTextButton(0,FSKEY_2,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"3...";
	msg.Append(FSGUI_DLG_AUTOPILOT_LANDING);
	landingBtn=AddTextButton(0,FSKEY_3,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"4...";
	msg.Append(FSGUI_DLG_AUTOPILOT_TAKEOFF);
	takeoffBtn=AddTextButton(0,FSKEY_4,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"5...";
	msg.Append(FSGUI_DLG_AUTOPILOT_FLY_HDGBUG);
	hdgBugBtn=AddTextButton(0,FSKEY_5,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"0...";
	msg.Append(FSGUI_DLG_AUTOPILOT_DISENGAGE);
	disengageBtn=AddTextButton(0,FSKEY_0,FSGUI_PUSHBUTTON,msg,YSTRUE);

	msg=L"ESC:";
	msg.Append(FSGUI_COMMON_CANCEL);
	cancelBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,msg,YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}
/* virtual */ void FsGuiAutoPilotDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==circleBtn)
	{
		Circle();
	}
	else if(btn==straightBtn)
	{
		Straight();
	}
	else if(btn==landingBtn)
	{
		Landing();
	}
	else if(btn==takeoffBtn)
	{
		TakeOff();
	}
	else if(btn==hdgBugBtn)
	{
		FlyHeadingBug();
	}
	else if(btn==disengageBtn)
	{
		Disengage();
	}
	else if(btn==cancelBtn)
	{
		sim->SetCurrentInFlightDialog(NULL);
	}
}
/* virtual */ YSBOOL FsGuiAutoPilotDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_ESC:
		sim->SetCurrentInFlightDialog(NULL);
		return YSTRUE;
	case FSKEY_0:
		Disengage();
		return YSTRUE;

	case FSKEY_1:
		Circle();
		return YSTRUE;
	case FSKEY_2:
		Straight();
		return YSTRUE;
	case FSKEY_3:
		Landing();
		return YSTRUE;
	case FSKEY_4:
		TakeOff();
		return YSTRUE;
	case FSKEY_5:
		FlyHeadingBug();
		return YSTRUE;
	}

	return YSFALSE;
}

void FsGuiAutoPilotDialog::Circle(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto gp=FsGotoPosition::Create();
		auto holdPos=playerPlane->GetPosition();
		gp->SetSingleDestination(holdPos);
		gp->speed=YsGreater(playerPlane->Prop().GetEstimatedLandingSpeed()*1.2,
		                    playerPlane->Prop().GetVelocity());
		gp->minAlt=0.0;
		gp->straightFlightMode=YSFALSE;
		playerPlane->SetAutopilot(gp);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotDialog::Straight(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto gp=FsGotoPosition::Create();
		auto holdPos=playerPlane->GetPosition();
		gp->SetSingleDestination(holdPos);
		gp->speed=YsGreater(playerPlane->Prop().GetEstimatedLandingSpeed()*1.2,
		                    playerPlane->Prop().GetVelocity());
		gp->minAlt=0.0;
		gp->straightFlightMode=YSTRUE;
		playerPlane->SetAutopilot(gp);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotDialog::Landing(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto ap=FsLandingAutopilot::Create();
		playerPlane->SetAutopilot(ap);
		FsGround *vor;
		if((vor=sim->FindGround(playerPlane->Prop().GetVorStationKey(0)))!=NULL &&
			vor->Prop().GetAircraftCarrierProperty()!=NULL)
		{
			ap->SetAirplaneInfo(*playerPlane,YsPi/2.0);
			ap->SetIls(*playerPlane,sim,vor);
		}
		else if((vor=sim->FindGround(playerPlane->Prop().GetVorStationKey(1)))!=NULL &&
				 vor->Prop().GetAircraftCarrierProperty()!=NULL)
		{
			ap->SetAirplaneInfo(*playerPlane,YsPi/2.0);
			ap->SetIls(*playerPlane,sim,vor);
		}
		ap->useRunwayClearingPathIfAvailable=YSTRUE;
		ap->taxi.SetFastTaxiSpeed(11.0);
		ap->taxi.SetSlowTaxiSpeed(5.5);
		ap->taxi.SetLastTaxiSpeed(4.0);
		ap->dontStopAtFarEnoughPosition=YSTRUE;
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotDialog::TakeOff(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto to=FsTakeOffAutopilot::Create();
		to->desigAlt=playerPlane->GetFieldElevation()+YsUnitConv::FTtoM(1500.0);

		YsVec3 o,v;
		if(YSOK==FsTakeOffAutopilot::FindRunwayCenterLine(o,v,*playerPlane,sim))
		{
			to->UseRunwayCenterLine(o,v);
		}

		playerPlane->SetAutopilot(to);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotDialog::FlyHeadingBug(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto gp=FsGotoPosition::Create();
		auto holdPos=playerPlane->GetPosition();
		gp->SetSingleDestination(holdPos);
		gp->speed=YsGreater(playerPlane->Prop().GetEstimatedLandingSpeed()*1.2,
		                    playerPlane->Prop().GetVelocity());
		gp->minAlt=0.0;
		gp->flyHeadingBugMode=YSTRUE;
		playerPlane->SetAutopilot(gp);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotDialog::Disengage(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		playerPlane->SetAutopilot(NULL);
		sim->ReadControlBackFromAirplane(playerPlane);
	}
	sim->SetCurrentInFlightDialog(NULL);
}

////////////////////////////////////////////////////////////

FsGuiAutoPilotVTOLDialog::FsGuiAutoPilotVTOLDialog()
{
	sim=NULL;
}
FsGuiAutoPilotVTOLDialog::~FsGuiAutoPilotVTOLDialog()
{
}
/* static */ FsGuiAutoPilotVTOLDialog *FsGuiAutoPilotVTOLDialog::Create(void)
{
	return new FsGuiAutoPilotVTOLDialog;
}
/* static */ void FsGuiAutoPilotVTOLDialog::Delete(FsGuiAutoPilotVTOLDialog *ptr)
{
	delete ptr;
}
void FsGuiAutoPilotVTOLDialog::Make(FsSimulation *sim)
{
	FsGuiDialog::Initialize();

	this->sim=sim;

	hoverBtn=AddTextButton(0,FSKEY_1,FSGUI_PUSHBUTTON,"1:Hover",YSTRUE);
	verticalLandingBtn=AddTextButton(0,FSKEY_2,FSGUI_PUSHBUTTON,"2:Vertical Landing",YSFALSE);
	disengageBtn=AddTextButton(0,FSKEY_0,FSGUI_PUSHBUTTON,"0:Disengage Autopilot",YSTRUE);
	cancelBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,"ESC:Cancel",YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}
/* virtual */ void FsGuiAutoPilotVTOLDialog::OnButtonClick(FsGuiButton *btn)
{
	if(hoverBtn==btn)
	{
		Hover();
	}
	else if(verticalLandingBtn==btn)
	{
		VerticalLanding();
	}
	else if(disengageBtn==btn)
	{
		Disengage();
	}
	else if(cancelBtn==btn)
	{
		sim->SetCurrentInFlightDialog(NULL);
	}
}
/* virtual */ YSBOOL FsGuiAutoPilotVTOLDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_ESC:
		sim->SetCurrentInFlightDialog(NULL);
		return YSTRUE;
	case FSKEY_0:
		Disengage();
		return YSTRUE;

	case FSKEY_1:
		Hover();
		return YSTRUE;
	case FSKEY_2:
		VerticalLanding();
		return YSTRUE;
	}
	return YSFALSE;
}

void FsGuiAutoPilotVTOLDialog::Hover(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto hv=FsHoveringAutopilot::Create();
		playerPlane->SetAutopilot(hv);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotVTOLDialog::VerticalLanding(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto vl=FsVerticalLandingAutopilot::Create();
		playerPlane->SetAutopilot(vl);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotVTOLDialog::Disengage(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		playerPlane->SetAutopilot(NULL);
		sim->ReadControlBackFromAirplane(playerPlane);
	}
	sim->SetCurrentInFlightDialog(NULL);
}

////////////////////////////////////////////////////////////

FsGuiAutoPilotHelicopterDialog::FsGuiAutoPilotHelicopterDialog()
{
	sim=NULL;
}
FsGuiAutoPilotHelicopterDialog::~FsGuiAutoPilotHelicopterDialog()
{
}
/* static */ FsGuiAutoPilotHelicopterDialog *FsGuiAutoPilotHelicopterDialog::Create(void)
{
	return new FsGuiAutoPilotHelicopterDialog;
}
/* static */ void FsGuiAutoPilotHelicopterDialog::Delete(FsGuiAutoPilotHelicopterDialog *ptr)
{
	delete ptr;
}
void FsGuiAutoPilotHelicopterDialog::Make(FsSimulation *sim)
{
	FsGuiDialog::Initialize();

	this->sim=sim;

	hoverBtn=AddTextButton(0,FSKEY_1,FSGUI_PUSHBUTTON,"1:Hover",YSTRUE);
	verticalLandingBtn=AddTextButton(0,FSKEY_2,FSGUI_PUSHBUTTON,"2:Vertical Landing",YSFALSE);
	disengageBtn=AddTextButton(0,FSKEY_0,FSGUI_PUSHBUTTON,"0:Disengage Autopilot",YSTRUE);
	cancelBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,"ESC:Cancel",YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}

/* virtual */ void FsGuiAutoPilotHelicopterDialog::OnButtonClick(FsGuiButton *btn)
{
	if(hoverBtn==btn)
	{
		Hover();
	}
	else if(verticalLandingBtn==btn)
	{
		VerticalLanding();
	}
	else if(disengageBtn==btn)
	{
		Disengage();
	}
	else if(cancelBtn==btn)
	{
		sim->SetCurrentInFlightDialog(NULL);
	}
}
/* virtual */ YSBOOL FsGuiAutoPilotHelicopterDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_ESC:
		sim->SetCurrentInFlightDialog(NULL);
		return YSTRUE;
	case FSKEY_0:
		Disengage();
		return YSTRUE;

	case FSKEY_1:
		Hover();
		return YSTRUE;
	case FSKEY_2:
		VerticalLanding();
		return YSTRUE;
	}
	return YSFALSE;
}

void FsGuiAutoPilotHelicopterDialog::Hover(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto hv=FsHoveringAutopilot::Create();
		playerPlane->SetAutopilot(hv);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotHelicopterDialog::VerticalLanding(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		auto vl=FsVerticalLandingAutopilot::Create();
		playerPlane->SetAutopilot(vl);
	}
	sim->SetCurrentInFlightDialog(NULL);
}
void FsGuiAutoPilotHelicopterDialog::Disengage(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		playerPlane->SetAutopilot(NULL);
		sim->ReadControlBackFromAirplane(playerPlane);
	}
	sim->SetCurrentInFlightDialog(NULL);
}

////////////////////////////////////////////////////////////

/*static */YsArray <FsGuiRadioCommDialogBase::Callable,16> FsGuiRadioCommDialogBase::GetCallable(FsSimulation *sim,FsExistence *player)
{
	YsArray <FsGuiRadioCommDialogBase::Callable,16> allCallable;

	auto playerPlane=dynamic_cast <FsAirplane *> (player);
	if(NULL!=playerPlane)
	{
		int nCallableAir=0;
		FsAirplane *air=NULL;
		while((air=sim->FindNextAirplane(air))!=NULL)
		{
			if(NULL!=air->GetAutopilot() &&
			   YSTRUE!=air->GetAutopilot()->DoesRespondToRadioCall())
			{
				continue;
			}
			if(air!=playerPlane &&
			   (air->airFlag&FSAIRFLAG_INDEPENDENT)==0 &&
			   air->iff==playerPlane->iff &&
			   air->Prop().IsActive()==YSTRUE &&
			   air->Prop().GetAircraftClass()!=FSCL_HELICOPTER &&  // 2003/05/05
			   air->netType==FSNET_LOCAL &&
			   air->isPlayingRecord!=YSTRUE)
			{
				allCallable.Increment();
				allCallable.Last().callableType=AIRCRAFT;
				allCallable.Last().callableKey=air->SearchKey();
				++nCallableAir;
			}
		}

		if(0<nCallableAir)
		{
			allCallable.Increment();
			allCallable.Last().callableType=ALLAIRCRAFT;
			allCallable.Last().callableKey=YSNULLHASHKEY;
		}

		YsArray <YsPair <FsAirplane *,YsVec3>,16> airInFormation;
		sim->GetAircraftInFormation(airInFormation,playerPlane);  // airInFormation[0] is always the wing leader.
		if(1<airInFormation.GetN())
		{
			allCallable.Increment();
			allCallable.Last().callableType=FORMATION;
			allCallable.Last().callableKey=YSNULLHASHKEY;
		}


		if(YSTRUE==playerPlane->Prop().IsOnGround())
		{
			YSHASHKEY fuelTruckOnCallKey=YSNULLHASHKEY;
			if(YSNULLHASHKEY==fuelTruckOnCallKey)
			{
				for(int i=0; i<sim->GetNumSupplyVehicle(); ++i)
				{
					FsGround *supply=sim->GetSupplyVehicle(i);
					if(NULL!=supply &&
					   (FSVEHICLE==supply->Prop().GetGroundType() || FSTANK==supply->Prop().GetGroundType()) &&
					   YSTRUE==supply->IsAutoDrivingTo(playerPlane))
					{
						fuelTruckOnCallKey=supply->SearchKey();
					}
				}
			}
			if(YSNULLHASHKEY==fuelTruckOnCallKey)
			{
				YSBOOL fuel,ammo;
				const FsGround *supply=sim->FindNearbySupplyTruck(fuel,ammo,*playerPlane);
				if(NULL!=supply &&
				  (FSVEHICLE==supply->Prop().GetGroundType() || FSTANK==supply->Prop().GetGroundType()))
				{
					fuelTruckOnCallKey=supply->SearchKey();
				}
			}
			if(YSNULLHASHKEY==fuelTruckOnCallKey && FSGROUNDSTATIC==playerPlane->Prop().GetFlightState())
			{
				FsGround *supply=sim->FindNearestSupplyTruckInTheSameRamp(playerPlane->GetPosition());
				if(NULL!=supply &&
				  (FSVEHICLE==supply->Prop().GetGroundType() || FSTANK==supply->Prop().GetGroundType()))
				{
					fuelTruckOnCallKey=supply->SearchKey();
				}
			}

			if(YSNULLHASHKEY!=fuelTruckOnCallKey)
			{
				allCallable.Increment();
				allCallable.Last().callableType=FUELTRUCK;
				allCallable.Last().callableKey=fuelTruckOnCallKey;
			}
		}

		if(YSTRUE==playerPlane->Prop().GetHasGunnerControlledTurret())
		{
			allCallable.Increment();
			allCallable.Last().callableType=GUNNER;
			allCallable.Last().callableKey=YSNULLHASHKEY;
		}

		allCallable.Increment();
		allCallable.Last().callableType=ATC;
		allCallable.Last().callableKey=YSNULLHASHKEY;
	}

	return allCallable;
}

////////////////////////////////////////////////////////////

FsGuiRadioCommTargetDialog::FsGuiRadioCommTargetDialog()
{
}
FsGuiRadioCommTargetDialog::~FsGuiRadioCommTargetDialog()
{
}
/* static */ FsGuiRadioCommTargetDialog *FsGuiRadioCommTargetDialog::Create(void)
{
	return new FsGuiRadioCommTargetDialog;
}
/* static */ void FsGuiRadioCommTargetDialog::Delete(FsGuiRadioCommTargetDialog *ptr)
{
	delete ptr;
}
void FsGuiRadioCommTargetDialog::Make(class FsSimulation *sim,YSSIZE_T nCallable,const Callable callable[])
{
	this->sim=sim;

	FsGuiDialog::Initialize();
	callableAir.CleanUp();
	keyBtnCallablePairArray.CleanUp();

	YsConstArrayMask <Callable> allCallable(nCallable,callable);

	FsGuiInFlightDialogWithNonConstantTextButton::Reset();
	for(auto &callable : allCallable)
	{
		if(callable.callableType==AIRCRAFT)
		{
			callableAir.Append(callable);
		}
	}
	const auto nAirTarget=callableAir.GetN();
	FsGuiInFlightDialogWithNonConstantTextButton::SetNumChoice(nAirTarget);
	YSSIZE_T nAirButton=0;
	if(NSHOW<nAirTarget)
	{
		nAirButton=NSHOW;
		MakeVariableTextButton(NSHOW);
		MakePageButton();
	}
	else
	{
		nAirButton=nAirTarget;
		MakeVariableTextButton(nAirTarget);
	}



	int nVarTxtButtonUsed=0;
	for(auto callable : allCallable)
	{
		switch(callable.callableType)
		{
		case AIRCRAFT:
			if(nVarTxtButtonUsed<nAirButton)
			{
				// Tentative.  Later set by ResetVariableTextButton()
				keyBtnCallablePairArray.Increment();
				keyBtnCallablePairArray.Last().fsKey=FSKEY_1+nVarTxtButtonUsed;
				keyBtnCallablePairArray.Last().btn=varBtnArray[nVarTxtButtonUsed];
				keyBtnCallablePairArray.Last().callable=callable;
				++nVarTxtButtonUsed;
			}
			break;
		case ALLAIRCRAFT:
			{
				YsWString msg=L"0...";
				msg.Append(FSGUI_DLG_RADIOCOMM_TARGET_ALLWINGMAN);
				keyBtnCallablePairArray.Increment();
				keyBtnCallablePairArray.Last().btn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,msg,YSTRUE);
				keyBtnCallablePairArray.Last().callable=callable;
				keyBtnCallablePairArray.Last().fsKey=FSKEY_0;
			}
			break;
		case FORMATION:
			{
				YsWString msg=L"W...";
				msg.Append(FSGUI_DLG_RADIOCOMM_TARGET_ALL_IN_FORMATION);
				keyBtnCallablePairArray.Increment();
				keyBtnCallablePairArray.Last().btn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,msg,YSTRUE);
				keyBtnCallablePairArray.Last().callable=callable;
				keyBtnCallablePairArray.Last().fsKey=FSKEY_W;
			}
			break;
		case GUNNER:
			{
				YsWString msg=L"G...";
				msg.Append(FSGUI_DLG_RADIOCOMM_TARGET_GUNNER);
				keyBtnCallablePairArray.Increment();
				keyBtnCallablePairArray.Last().btn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,msg,YSTRUE);
				keyBtnCallablePairArray.Last().callable=callable;
				keyBtnCallablePairArray.Last().fsKey=FSKEY_G;
			}
			break;
		case ATC:
			{
				YsWString msg=L"A...";
				msg.Append(FSGUI_DLG_RADIOCOMM_TARGET_ATC);
				keyBtnCallablePairArray.Increment();
				keyBtnCallablePairArray.Last().btn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,msg,YSTRUE);
				keyBtnCallablePairArray.Last().callable=callable;
				keyBtnCallablePairArray.Last().fsKey=FSKEY_A;
			}
			break;
		case FUELTRUCK:
			{
				YsWString msg=L"F...";
				msg.Append(FSGUI_DLG_RADIOCOMM_TARGET_FUELTRUCK);
				keyBtnCallablePairArray.Increment();
				keyBtnCallablePairArray.Last().btn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,msg,YSTRUE);
				keyBtnCallablePairArray.Last().callable=callable;
				keyBtnCallablePairArray.Last().fsKey=FSKEY_F;
			}
			break;
		}
	}

	{
		YsWString msg=L"ESC:";
		msg.Append(FSGUI_DLG_RADIOCOMM_TARGET_DONTCALL);
		cancelBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,msg,YSTRUE);
	}

	ResetVariableTextButton();

	Fit();
	SetBackgroundAlpha(0.5);
}
/* virtual */ void FsGuiRadioCommTargetDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==cancelBtn)
	{
		sim->SetCurrentInFlightDialog(NULL);
	}
	if(btn==NextPageButton())
	{
		NextPage();
		ResetVariableTextButton();
	}
	else if(btn==PrevPageButton())
	{
		PrevPage();
		ResetVariableTextButton();
	}

	for(auto &keyBtnCallablePair : keyBtnCallablePairArray)
	{
		if(keyBtnCallablePair.btn==btn)
		{
			CallableSelected(keyBtnCallablePair.callable);
		}
	}
}

void FsGuiRadioCommTargetDialog::ResetVariableTextButton(void)
{
	int airCounter=0;
	for(auto &keyBtnCallablePair : keyBtnCallablePairArray)
	{
		if(AIRCRAFT==keyBtnCallablePair.callable.callableType)
		{
			const int wingmanIndex=(int)(baseIndex+airCounter);
			if(YSTRUE==callableAir.IsInRange(wingmanIndex))
			{
				auto air=sim->FindAirplane(callableAir[wingmanIndex].callableKey);
				if(NULL!=air)
				{
					const YsString airName(air->GetName());
					YsWString wStr;
					if(0<airName.Strlen())
					{
						YsString str;
						str.Printf("%d...%s[%s]",1+airCounter,airName.Txt(),air->Prop().GetIdentifier());
						wStr.SetUTF8String(str);
					}
					else
					{
						YsString left,right;
						left.Printf("%d...",1+airCounter);
						right.Printf(" #%d[%s]",1+wingmanIndex,air->Prop().GetIdentifier());

						YsWString wLeft,wRight;
						wLeft.SetUTF8String(left);
						wRight.SetUTF8String(right);

						wStr=wLeft;
						wStr.Append(FSGUI_COMMON_WINGMAN);
						wStr.Append(wRight);
					}
					keyBtnCallablePair.callable.callableKey=air->SearchKey();
					keyBtnCallablePair.btn->SetText(wStr);
					keyBtnCallablePair.fsKey=FSKEY_1+airCounter;
				}
				else
				{
					keyBtnCallablePair.callable.callableKey=YSNULLHASHKEY;
					keyBtnCallablePair.btn->SetText("...");
					keyBtnCallablePair.fsKey=FSKEY_NULL;
				}
			}
			else
			{
				keyBtnCallablePair.callable.callableKey=YSNULLHASHKEY;
				keyBtnCallablePair.btn->SetText("...");
				keyBtnCallablePair.fsKey=FSKEY_NULL;
			}
			++airCounter;
		}
	}
}

/* virtual */ YSBOOL FsGuiRadioCommTargetDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_ENTER:
	case FSKEY_ESC:
	case FSKEY_BS:
		sim->SetCurrentInFlightDialog(NULL);
		return YSTRUE;
	case FSKEY_RIGHT:
	case FSKEY_N:
	case FSKEY_PAGEUP:
		NextPage();
		ResetVariableTextButton();
		return YSTRUE;
	case FSKEY_LEFT:
	case FSKEY_P:
	case FSKEY_PAGEDOWN:
		PrevPage();
		ResetVariableTextButton();
		return YSTRUE;
	}
	for(auto &keyBtnCallablePair : keyBtnCallablePairArray)
	{
		if(keyBtnCallablePair.fsKey==rawKey)
		{
			sim->SetCurrentInFlightDialog(NULL); // Do it before CallableSelected, or it would cancel dialog set in CallableSelected.
			CallableSelected(keyBtnCallablePair.callable);
			return YSTRUE;
		}
	}
	return YSFALSE;
}

void FsGuiRadioCommTargetDialog::CallableSelected(const Callable &callable)
{
	switch(callable.callableType)
{
	case ATC:
		sim->SetUpAtcRequestDialog();
		break;
	case FUELTRUCK:
		sim->SetUpCallFuelTruckDialog();
		break;
	case FORMATION:
		sim->SetUpRadioCommToFormationDialog();
		break;

	case GUNNER:
		sim->SetUpGunnerSubMenu();
		break;
	case AIRCRAFT:
		{
			auto air=sim->FindAirplane(callable.callableKey);
			if(NULL!=air)
			{
				YsArray <FsAirplane *> comTarget;
				comTarget.Set(1,NULL);
				comTarget[0]=air;
				sim->SetUpRadioCommCommandDialog(comTarget);
			}
		}
		break;
	case ALLAIRCRAFT:
		{
			YsArray <FsAirplane *,16> comTarget;
			for(auto &callable : callableAir)
			{
				if(AIRCRAFT==callable.callableType)
				{
					auto air=sim->FindAirplane(callable.callableKey);
					if(NULL!=air)
					{
						comTarget.Append(air);
					}
				}
			}
			sim->SetUpRadioCommCommandDialog(comTarget);
		}
		break;
	}
}

////////////////////////////////////////////////////////////

FsGuiRadioCommFuelTruckDialog::FsGuiRadioCommFuelTruckDialog()
{
	fuelTruckOnCallKey=YSNULLHASHKEY;
}
FsGuiRadioCommFuelTruckDialog::~FsGuiRadioCommFuelTruckDialog()
{
}
/* static */ FsGuiRadioCommFuelTruckDialog *FsGuiRadioCommFuelTruckDialog::Create(void)
{
	return new FsGuiRadioCommFuelTruckDialog;
}
/* static */ void FsGuiRadioCommFuelTruckDialog::Delete(FsGuiRadioCommFuelTruckDialog *ptr)
{
	delete ptr;
}
void FsGuiRadioCommFuelTruckDialog::CacheCallableFuelTruck(void)
{
	auto *playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane && YSTRUE==playerPlane->Prop().IsOnGround())
	{
		if(YSNULLHASHKEY==fuelTruckOnCallKey)
		{
			for(int i=0; i<sim->GetNumSupplyVehicle(); ++i)
			{
				FsGround *supply=sim->GetSupplyVehicle(i);
				if(NULL!=supply &&
				   (FSVEHICLE==supply->Prop().GetGroundType() || FSTANK==supply->Prop().GetGroundType()) &&
				   YSTRUE==supply->IsAutoDrivingTo(playerPlane))
				{
					fuelTruckOnCallKey=supply->SearchKey();
				}
			}
		}
		if(YSNULLHASHKEY==fuelTruckOnCallKey)
		{
			YSBOOL fuel,ammo;
			const FsGround *supply=sim->FindNearbySupplyTruck(fuel,ammo,*playerPlane);
			if(NULL!=supply &&
			  (FSVEHICLE==supply->Prop().GetGroundType() || FSTANK==supply->Prop().GetGroundType()))
			{
				fuelTruckOnCallKey=supply->SearchKey();
			}
		}
		if(YSNULLHASHKEY==fuelTruckOnCallKey && FSGROUNDSTATIC==playerPlane->Prop().GetFlightState())
		{
			FsGround *supply=sim->FindNearestSupplyTruckInTheSameRamp(playerPlane->GetPosition());
			if(NULL!=supply &&
			  (FSVEHICLE==supply->Prop().GetGroundType() || FSTANK==supply->Prop().GetGroundType()))
			{
				fuelTruckOnCallKey=supply->SearchKey();
			}
		}
	}
	else
	{
		fuelTruckOnCallKey=YSNULLHASHKEY;
	}
}
void FsGuiRadioCommFuelTruckDialog::Make(FsSimulation *sim)
{
	FsGuiDialog::Initialize();

	this->sim=sim;

	AddStaticText(0,FSKEY_NULL,FSGUI_DLG_RADIOCOMM_FUELTRUCK_TITLE,YSTRUE);

	YsWString msg;

	msg=L"1...";
	msg.Append(FSGUI_DLG_RADIOCOMM_FUELTRUCK_CALL);
	callBtn=AddTextButton(0,FSKEY_1,FSGUI_PUSHBUTTON,msg,YSTRUE);

	msg=L"2...";
	msg.Append(FSGUI_DLG_RADIOCOMM_FUELTRUCK_DISMISS);
	dismissBtn=AddTextButton(0,FSKEY_2,FSGUI_PUSHBUTTON,msg,YSTRUE);

	msg=L"ESC:";
	msg.Append(FSGUI_COMMON_CANCEL);
	neverMindBtn=AddTextButton(0,FSKEY_3,FSGUI_PUSHBUTTON,msg,YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}
/* virtual */ void FsGuiRadioCommFuelTruckDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==callBtn)
	{
		Call();
	}
	else if(btn==callBtn)
	{
		Dismiss();
	}
	else if(btn==neverMindBtn)
	{
		Close();
	}
}
/* virtual */ YSBOOL FsGuiRadioCommFuelTruckDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_ESC:
	case FSKEY_0:
		Close();
		return YSTRUE;
	case FSKEY_1:
		Call();
		return YSTRUE;
	case FSKEY_2:
		Dismiss();
		return YSTRUE;
	}
	return YSFALSE;
}
void FsGuiRadioCommFuelTruckDialog::Call(void)
{
	FsGround *fuelTruck=sim->FindGround(fuelTruckOnCallKey);
	FsExistence *player=sim->GetPlayerObject();
	if(NULL!=fuelTruck && NULL!=player)
	{
		if(YSTRUE!=fuelTruck->IsAutoDrivingTo(player))
		{
			// autoDrive1 will be used first, and when it is accomplished,
			// autoDrive2 will hold until the player goes away, and drive it back
			// to the initial position;
			FsAutoDriveToExactPosition *autoDrive2=new FsAutoDriveToExactPosition;
			autoDrive2->SetUp(sim,fuelTruck,fuelTruck->initPosition,fuelTruck->initAttitude.h());
			autoDrive2->holdUntil.SetHoldUntilThisObjectGoesAway(player);
			fuelTruck->PushAutoDrive(autoDrive2);

			FsAutoDriveToObject *autoDrive1=new FsAutoDriveToObject;
			autoDrive1->SetUp(sim,fuelTruck,player);
			fuelTruck->PushAutoDrive(autoDrive1);
			sim->AddTimedMessage("Fuel Truck: I'll be right with you.");
			Close();
		}
		else
		{
			sim->AddTimedMessage("Fuel Truck: I'm on the way.");
			Close();
		}
	}
	else
	{
		sim->AddTimedMessage("Lost communication with the fuel truck.");
		fuelTruckOnCallKey=YSNULLHASHKEY;
		Close();
	}
}
void FsGuiRadioCommFuelTruckDialog::Dismiss(void)
{
	FsGround *fuelTruck=sim->FindGround(fuelTruckOnCallKey);
	FsExistence *player=sim->GetPlayerObject();
	if(NULL!=fuelTruck && NULL!=player)
	{
		YSBOOL fuel,ammo,dismissed=YSFALSE;
		FsGround *fuelTruck=sim->FindNearbySupplyTruck(fuel,ammo,*player);
		if(NULL!=fuelTruck)
		{
			FsAutoDriveToExactPosition *autoDrive=new FsAutoDriveToExactPosition;
			autoDrive->SetUp(sim,fuelTruck,fuelTruck->initPosition,fuelTruck->initAttitude.h());
			autoDrive->StartWithThreePointTurn(sim,fuelTruck);
			autoDrive->ExcludeFromCollisionTest(player); // This will prevent the dead lock at the beginning.
			fuelTruck->SetAutoDrive(autoDrive);
			dismissed=YSTRUE;
		}
		for(int i=0; i<sim->GetNumSupplyVehicle(); ++i)
		{
			FsGround *fuelTruck=sim->GetSupplyVehicle(i);
			if(NULL!=fuelTruck &&
			   YSTRUE==fuelTruck->IsAlive() &&
			   YSTRUE==fuelTruck->IsAutoDrivingTo(player))
			{
				FsAutoDriveToExactPosition *autoDrive=new FsAutoDriveToExactPosition;
				autoDrive->SetUp(sim,fuelTruck,fuelTruck->initPosition,fuelTruck->initAttitude.h());
				autoDrive->StartWithThreePointTurn(sim,fuelTruck);
				autoDrive->ExcludeFromCollisionTest(player); // This will prevent the dead lock at the beginning.
				fuelTruck->SetAutoDrive(autoDrive);
				dismissed=YSTRUE;
			}
		}
		if(YSTRUE==dismissed)
		{
			Close();
		}
		else
		{
			sim->AddTimedMessage("No fuel truck to dismiss.");
			fuelTruckOnCallKey=YSNULLHASHKEY;
			Close();
		}
	}
	else
	{
		sim->AddTimedMessage("Lost communication with the fuel truck.");
		fuelTruckOnCallKey=YSNULLHASHKEY;
		Close();
	}
}
void FsGuiRadioCommFuelTruckDialog::Close(void)
{
	sim->SetCurrentInFlightDialog(NULL);
	fuelTruckOnCallKey=YSNULLHASHKEY;
}

////////////////////////////////////////////////////////////

FsGuiRadioCommCommandDialog::FsGuiRadioCommCommandDialog()
{
}
FsGuiRadioCommCommandDialog::~FsGuiRadioCommCommandDialog()
{
}
/* static */ FsGuiRadioCommCommandDialog *FsGuiRadioCommCommandDialog::Create(void)
{
	return new FsGuiRadioCommCommandDialog;
}
/* static */ void FsGuiRadioCommCommandDialog::Delete(FsGuiRadioCommCommandDialog *ptr)
{
	delete ptr;
}
void FsGuiRadioCommCommandDialog::GetComTarget(YsArray <FsAirplane *,16> &target) const
{
	target.CleanUp();
	for(auto &key : comTargetKey)
	{
		auto ptr=sim->FindAirplane(key);
		if(NULL!=ptr)
		{
			target.Append(ptr);
		}
	}
}

void FsGuiRadioCommCommandDialog::Make(FsSimulation *sim)
{
	FsGuiDialog::Initialize();

	this->sim=sim;


	YsWString msg;

	AddStaticText(0,FSKEY_NULL,FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_TITLE,YSTRUE);

	msg=L"1...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_BREAK_AND_ATTACK);
	breakAndAttackBtn=AddTextButton(0,FSKEY_1,FSGUI_PUSHBUTTON,msg,YSTRUE);

	msg=L"2...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_ATTACK_GROUND);
	attackGroundTargetBtn=AddTextButton(0,FSKEY_2,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"3...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_COVER_ME);
	helpMeBtn=AddTextButton(0,FSKEY_3,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"4...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_FORM_ON_MY_WING);
	formOnMyWingBtn=AddTextButton(0,FSKEY_4,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"5...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_RTB);
	returnToBaseBtn=AddTextButton(0,FSKEY_5,FSGUI_PUSHBUTTON,msg,YSTRUE);

	msg=L"6...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_HOLD);
	stayInHoldingBtn=AddTextButton(0,FSKEY_6,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"7...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_LAND_REFUEL_TAKEOFF);
	landRefuelAndTakeOffBtn=AddTextButton(0,FSKEY_7,FSGUI_PUSHBUTTON,msg,YSFALSE);

	msg=L"0...";
	msg.Append(FSGUI_DLG_RADIOCOMM_WINGMAN_COMMAND_DONTSEND);
	cancelBtn=AddTextButton(0,FSKEY_3,FSGUI_PUSHBUTTON,msg,YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}
void FsGuiRadioCommCommandDialog::SetCommTarget(YSSIZE_T n,FsAirplane *const air[])
{
	comTargetKey.Set(n,NULL);
	for(YSSIZE_T i=0; i<n; ++i)
	{
		comTargetKey[i]=air[i]->SearchKey();
	}
}
/* virtual */ void FsGuiRadioCommCommandDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==breakAndAttackBtn)
	{
		BreakAndAttack();
	}
	else if(btn==attackGroundTargetBtn)
	{
		AttackGroundTarget();
	}
	else if(btn==helpMeBtn)
	{
		HelpMe();
	}
	else if(btn==formOnMyWingBtn)
	{
		FormOnMyWing();
	}
	else if(btn==returnToBaseBtn)
	{
		ReturnToBase();
	}
	else if(btn==stayInHoldingBtn)
	{
		StayInHoldingPattern();
	}
	else if(btn==landRefuelAndTakeOffBtn)
	{
		LandRefuelAndTakeOff();
	}
	else if(btn==cancelBtn)
	{
		Close();
	}
}
/* virtual */ YSBOOL FsGuiRadioCommCommandDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_ENTER:
	case FSKEY_ESC:
	case FSKEY_0:
		Close();
		return YSTRUE;
	case FSKEY_1:
		BreakAndAttack();
		return YSTRUE;
	case FSKEY_2:
		AttackGroundTarget();
		return YSTRUE;
	case FSKEY_3:
		HelpMe();
		return YSTRUE;
	case FSKEY_4:
		FormOnMyWing();
		return YSTRUE;
	case FSKEY_5:
		ReturnToBase();
		return YSTRUE;
	case FSKEY_6:
		StayInHoldingPattern();
		return YSTRUE;
	case FSKEY_7:
		LandRefuelAndTakeOff();
		return YSTRUE;
	}
	return YSFALSE;
}
void FsGuiRadioCommCommandDialog::BreakAndAttack(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	YsArray <FsAirplane *,16> comTarget;
	GetComTarget(comTarget);
	for(auto target : comTarget)
	{
		sim->RadioCommSendBreakAndAttack(target,playerPlane);
	}
	Close();
}
void FsGuiRadioCommCommandDialog::AttackGroundTarget(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	YsArray <FsAirplane *,16> comTarget;
	GetComTarget(comTarget);
	for(auto target : comTarget)
	{
		sim->RadioCommSendAttackGround(target,playerPlane);
	}
	Close();
}
void FsGuiRadioCommCommandDialog::HelpMe(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	YsArray <FsAirplane *,16> comTarget;
	GetComTarget(comTarget);
	for(auto target : comTarget)
	{
		sim->RadioCommSendCoverMe(target,playerPlane);
	}
	Close();
}
void FsGuiRadioCommCommandDialog::FormOnMyWing(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	YsArray <FsAirplane *,16> comTarget;
	GetComTarget(comTarget);
	for(auto target : comTarget)
	{
		sim->RadioCommSendFormOnMyWing(target,playerPlane,YSTRUE);
	}
	Close();
}
void FsGuiRadioCommCommandDialog::ReturnToBase(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	YsArray <FsAirplane *,16> comTarget;
	GetComTarget(comTarget);
	for(auto target : comTarget)
	{
		sim->RadioCommSendReturnToBase(target,playerPlane);
	}
	Close();
}
void FsGuiRadioCommCommandDialog::StayInHoldingPattern(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	YsArray <FsAirplane *,16> comTarget;
	GetComTarget(comTarget);
	for(auto target : comTarget)
	{
		sim->RadioCommSendHoldingPattern(target,playerPlane);
	}
	Close();
}
void FsGuiRadioCommCommandDialog::LandRefuelAndTakeOff(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	YsArray <FsAirplane *,16> comTarget;
	GetComTarget(comTarget);
	for(auto target : comTarget)
	{
		sim->RadioCommSendLandRefuelAndTakeOff(target,playerPlane);
	}
	Close();
}
void FsGuiRadioCommCommandDialog::Close(void)
{
	sim->SetCurrentInFlightDialog(NULL);
	comTargetKey.CleanUp();
}

////////////////////////////////////////////////////////////

FsGuiSelectApproachDialog::FsGuiSelectApproachDialog()
{
}
FsGuiSelectApproachDialog::~FsGuiSelectApproachDialog()
{
}
/* static */ FsGuiSelectApproachDialog *FsGuiSelectApproachDialog::Create(void)
{
	return new FsGuiSelectApproachDialog;
}
/* static */ void FsGuiSelectApproachDialog::Delete(FsGuiSelectApproachDialog *ptr)
{
	delete ptr;
}
void FsGuiSelectApproachDialog::Make(FsSimulation *sim)
{
	Reset();
	FsGuiDialog::Initialize();

	this->sim=sim;

	AddStaticText(0,FSKEY_NULL,"Request Approach",YSTRUE);

	MakeVariableTextButton(5);
	MakePageButton();
	cancelBtn=AddTextButton(0,FSKEY_3,FSGUI_PUSHBUTTON,"ESC: (Don't request approach.)",YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}
/* virtual */ void FsGuiSelectApproachDialog::OnButtonClick(FsGuiButton *btn)
{
	const int clicked=ClickedSelection(btn);
	if(0<=clicked)
	{
		Select(clicked);
	}
	else if(NextPageButton()==btn)
	{
		NextPage();
	}
	else if(PrevPageButton()==btn)
	{
		PrevPage();
	}
	else if(cancelBtn==btn)
	{
		Close();
	}
}
/* virtual */ YSBOOL FsGuiSelectApproachDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_ENTER: // Must take it o prevent another ATC menu to pop up.
	case FSKEY_ESC:
	case FSKEY_0:
		Close();
		return YSTRUE;
	case FSKEY_1:
	case FSKEY_2:
	case FSKEY_3:
	case FSKEY_4:
	case FSKEY_5:
	case FSKEY_6:
	case FSKEY_7:
	case FSKEY_8:
	case FSKEY_9:
		Select(rawKey-FSKEY_1);
		return YSTRUE;
	case FSKEY_RIGHT:
	case FSKEY_N:
	case FSKEY_PAGEUP:
		NextPage();
		return YSTRUE;
	case FSKEY_LEFT:
	case FSKEY_P:
	case FSKEY_PAGEDOWN:
		PrevPage();
		return YSTRUE;
	}
	return YSFALSE;
}
/* virtual */ void FsGuiSelectApproachDialog::UpdateDialog(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		YsArray <FsGround *,64> ilsList;
		YsArray <double,64> ilsDist;
		YsString buf;
		sim->MakeSortedIlsList(ilsList,ilsDist,playerPlane->GetPosition());

		if(0==ilsList.GetN())
		{
			sim->AddTimedMessage("No approach available.");
			Close();
			return;
		}

		SetNumChoice(ilsList.GetN());

		YSBOOL needFit=YSFALSE;
		for(YSSIZE_T i=0; i<NShow(); i++)
		{
			if(baseIndex+i<ilsList.GetN())
			{
				buf.Printf("%d...ILS [%s] (%.0lf miles)",i+1,(const char *)ilsList[baseIndex+i]->name,YsUnitConv::MtoNM(ilsDist[baseIndex+i]));
			}
			else
			{
				buf="...";
			}
			if(YSTRUE==UpdateText(i,buf))
			{
				needFit=YSTRUE;
			}
		}
		if(YSTRUE==needFit)
		{
			Fit();
		}

		if(NShow()<ilsList.GetN())
		{
			NextPageButton()->Enable();
			PrevPageButton()->Enable();
		}
		else
		{
			NextPageButton()->Disable();
			PrevPageButton()->Disable();
		}
	}
}
void FsGuiSelectApproachDialog::Select(int sel)
{
	if(0<=sel && sel<NShow())
	{
		auto playerPlane=sim->GetPlayerAirplane();
		if(NULL!=playerPlane)
		{
			YSSIZE_T selIndex=baseIndex+sel;

			YsArray <FsGround *,64> ilsList;
			YsArray <double,64> ilsDist;
			sim->MakeSortedIlsList(ilsList,ilsDist,playerPlane->GetPosition());

			if(YSTRUE==ilsList.IsInRange(selIndex))
			{
				auto atc=sim->FindAirTrafficController(FsAirTrafficController::PrimaryAirTrafficControllerKey);
				if(NULL!=playerPlane && NULL!=atc)
				{
					playerPlane->AtcRequestIlsApproach(sim,atc,ilsList[selIndex]);
				}
			}
		}
		Close();
	}
}
void FsGuiSelectApproachDialog::Close(void)
{
	Reset();
	sim->SetCurrentInFlightDialog(NULL);
}

////////////////////////////////////////////////////////////

FsGuiAtcRequestDialog::FsGuiAtcRequestDialog()
{
}
FsGuiAtcRequestDialog::~FsGuiAtcRequestDialog()
{
}
/* static */ FsGuiAtcRequestDialog *FsGuiAtcRequestDialog::Create(void)
{
	return new FsGuiAtcRequestDialog;
}
/* static */ void FsGuiAtcRequestDialog::Delete(FsGuiAtcRequestDialog *ptr)
{
	delete ptr;
}
void FsGuiAtcRequestDialog::Make(FsSimulation *sim)
{
	FsGuiDialog::Initialize();

	this->sim=sim;

	activeBtn.CleanUp();

	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		YsString label;
		int index=1;
		const FsAirTrafficInfo &airTrafficInfo=playerPlane->GetAirTrafficInfo();
		const FsAirTrafficController *atc=airTrafficInfo.GetAirTrafficController(sim);

		if(NULL==atc)
		{
			label.Printf("%d...Request Approach",index++);
			requestApproachBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,label,YSTRUE);
			activeBtn.Append(requestApproachBtn);
		}
		else
		{
			label.Printf("%d...Request Another Approach",index++);
			requestApproachBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,label,YSTRUE);
			activeBtn.Append(requestApproachBtn);

			const FsApproach &app=airTrafficInfo.GetApproach();
			if(FsApproach::APP_NULL!=app.GetApproachType() &&
			   FsApproach::APP_VISUAL!=app.GetApproachType())
			{
				if(YSTRUE==airTrafficInfo.IsHeadingForFinalFix())
				{
					label.Printf("%d...Declare Missed Approach",index++);
					declareMissedApproachBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,label,YSTRUE);
					activeBtn.Append(declareMissedApproachBtn);
				}
				else
				{
					label.Printf("%d...Request New Vector",index++);
					requestNewVectorBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,label,YSTRUE);
					activeBtn.Append(requestNewVectorBtn);
				}
			}
			if(YSTRUE==airTrafficInfo.IsGuidedByATC())
			{
				label.Printf("%d...Request Heading",index++);
				requestVectorBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,label,YSTRUE);
				activeBtn.Append(requestVectorBtn);
			}

			label.Printf("%d...Cancel IFR",index++);
			cancelIFRBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,label,YSTRUE);
			activeBtn.Append(cancelIFRBtn);
		}
	}

	cancelBtn=AddTextButton(0,FSKEY_3,FSGUI_PUSHBUTTON,"ESC: (Don't request approach.)",YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);

}
/* virtual */ void FsGuiAtcRequestDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==cancelBtn)
	{
		Close();
	}
	else if(btn==requestApproachBtn)
	{
		RequestApproach();
	}
	else if(btn==declareMissedApproachBtn)
	{
		DeclareMissedApproach();
	}
	else if(btn==requestNewVectorBtn)
	{
		RequestNewVector();
	}
	else if(btn==requestVectorBtn)
	{
		RequestVector();
	}
	else if(btn==cancelIFRBtn)
	{
		CancelIFR();
	}

}
/* virtual */ YSBOOL FsGuiAtcRequestDialog::ProcessRawKeyInput(int rawKey)
{
	if(FSKEY_ESC==rawKey || FSKEY_0==rawKey || FSKEY_ENTER==rawKey)
	{
		Close();
		return YSTRUE;
	}
	else if(FSKEY_1<=rawKey && rawKey<=FSKEY_9)
	{
		const int sel=rawKey-FSKEY_1;
		if(YSTRUE==activeBtn.IsInRange(sel))
		{
			OnButtonClick(activeBtn[sel]);
			return YSTRUE;
		}
	}
	return YSFALSE;
}
void FsGuiAtcRequestDialog::RequestApproach(void)
{
	sim->SetUpRequestApproachDialog();
	// Don't Close().  It will close RequestApproachDialog, not this dialog.
}
void FsGuiAtcRequestDialog::DeclareMissedApproach(void)
{
	Close();
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		FsAirTrafficInfo &airTrafficInfo=playerPlane->GetAirTrafficInfo();
		FsAirTrafficController *atc=airTrafficInfo.GetAirTrafficController(sim);
		if(NULL!=atc)
		{
			atc->AirplaneDeclareMissedApproach(sim,*playerPlane);
		}
	}
}
void FsGuiAtcRequestDialog::RequestNewVector(void)
{
	Close();
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		FsAirTrafficInfo &airTrafficInfo=playerPlane->GetAirTrafficInfo();
		FsAirTrafficController *atc=airTrafficInfo.GetAirTrafficController(sim);
		if(NULL!=atc)
		{
			atc->AirplaneRequestNewVector(sim,*playerPlane);
		}
	}
}
void FsGuiAtcRequestDialog::RequestVector(void)
{
	Close();
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		FsAirTrafficInfo &airTrafficInfo=playerPlane->GetAirTrafficInfo();
		FsAirTrafficController *atc=airTrafficInfo.GetAirTrafficController(sim);
		if(NULL!=atc)
		{
			atc->AirplaneRequestHeading(sim,*playerPlane);
		}
	}
}
void FsGuiAtcRequestDialog::CancelIFR(void)
{
	Close();
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		FsAirTrafficInfo &airTrafficInfo=playerPlane->GetAirTrafficInfo();
		FsAirTrafficController *atc=airTrafficInfo.GetAirTrafficController(sim);
		if(NULL!=atc)
		{
			atc->AirplaneCancelIFR(sim,*playerPlane);
		}
	}
}
void FsGuiAtcRequestDialog::Close(void)
{
	sim->SetCurrentInFlightDialog(NULL);
}

////////////////////////////////////////////////////////////

FsGuiRadioCommToFormationDialog::FsGuiRadioCommToFormationDialog()
{
}
FsGuiRadioCommToFormationDialog::~FsGuiRadioCommToFormationDialog()
{
}
/* static */ FsGuiRadioCommToFormationDialog *FsGuiRadioCommToFormationDialog::Create(void)
{
	return new FsGuiRadioCommToFormationDialog;
}
/* static */ void FsGuiRadioCommToFormationDialog::Delete(FsGuiRadioCommToFormationDialog *ptr)
{
	delete ptr;
}
void FsGuiRadioCommToFormationDialog::Make(FsSimulation *sim)
{
	FsGuiDialog::Initialize();

	this->sim=sim;

	YsWString wStr;

	wStr=L"1...";
	wStr.Append(FSGUI_DLG_RADIOCOMM_FORMATION_SPREAD);
	spreadBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,wStr,YSTRUE);

	wStr=L"2...";
	wStr.Append(FSGUI_DLG_RADIOCOMM_FORMATION_TIGHTEN);
	tightenBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,wStr,YSTRUE);

	wStr=L"ESC:";
	wStr.Append(FSGUI_DLG_RADIOCOMM_FORMATION_DONTCALL);
	cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,wStr,YSTRUE);

	Fit();
	SetBackgroundAlpha(0.5);
}
/* virtual */ void FsGuiRadioCommToFormationDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==spreadBtn)
	{
		SpreadFormation();
	}
	else if(btn==tightenBtn)
	{
		TightenFormation();
	}
	else if(btn==cancelBtn)
	{
		Close();
	}
}
/* virtual */ YSBOOL FsGuiRadioCommToFormationDialog::ProcessRawKeyInput(int rawKey)
{
	switch(rawKey)
	{
	case FSKEY_0:
	case FSKEY_ESC:
	case FSKEY_ENTER:
		Close();
		return YSTRUE;
	case FSKEY_1:
		SpreadFormation();
		return YSTRUE;
	case FSKEY_2:
		TightenFormation();
		return YSTRUE;
	default:
		break;
	}
	return YSFALSE;
}
void FsGuiRadioCommToFormationDialog::SpreadFormation(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		sim->RadioCommSendSpreadFormation(playerPlane,1.5);
	}
	Close();
}
void FsGuiRadioCommToFormationDialog::TightenFormation(void)
{
	auto playerPlane=sim->GetPlayerAirplane();
	if(NULL!=playerPlane)
	{
		sim->RadioCommSendTightenFormation(playerPlane,1.2);
	}
	Close();
}
void FsGuiRadioCommToFormationDialog::Close(void)
{
	sim->SetCurrentInFlightDialog(NULL);
}
