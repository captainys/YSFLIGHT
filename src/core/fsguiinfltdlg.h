#ifndef FSGUIINFLTDLG_IS_INCLUDED
#define FSGUIINFLTDLG_IS_INCLUDED
/* { */

#include <fsgui.h>

/*

Conclusion:  Multiple-inheritance is useless.

FsGuiRadioCommDialogBase has facility for NonConstantTextButton, which may not be used in all the radio comm dialogs.
Also, I don't define WouldProcessNumberKey and WouldProcessThisRawKey in FsGuiInFlightDialogWithNonConstantTextButton,
C++ compiler says it is ambiguous.  The problem is, it isn't.

Only way to get rid of this only-annoying warning that really has zero value is explicitly override
WouldProcessNumberKey and WouldProcessThisRawKey in the child dialogs of FsGuiInFlightDialogWithNonConstantTextButton.
Which loses the whole point of inheriting from FsGuiInFlightDialogWithNonConstantTextButton.

Conclusion:  Multiple-inheritance is useless.

                                 FsGuiInFlightDialog
                                   /              \                                   
FsGuiInFlightDialogThatProcessNumberKey  FsGuiInFlightDialogThatProcessNumberKeyAndEsc
                                                  |
                                         FsGuiInFlightDialogWithNonConstantTextButton
                                                  |
                                         FsGuiRadioCommDialogBase
*/


class FsGuiInFlightDialog : public FsGuiDialog
{
protected:
	class FsSimulation *sim;

public:
	virtual YSBOOL ProcessRawKeyInput(int rawKey);           // Must return YSTRUE if processed.  YSFALSE otherwise.
	virtual YSBOOL WouldProcessNumberKey(void) const;        // Must return YSTRUE if this dialog would take a number key.
	virtual YSBOOL WouldProcessThisRawKey(int rawKey) const; // Must return YSTRUE if this dialog would take this key.
	virtual void UpdateDialog(void);
};

class FsGuiInFlightDialogThatProcessNumberKey : public FsGuiInFlightDialog
{
public:
	virtual YSBOOL WouldProcessNumberKey(void) const;
};

class FsGuiInFlightDialogThatProcessNumberKeyAndEsc : public FsGuiInFlightDialog
{
public:
	virtual YSBOOL WouldProcessNumberKey(void) const;
	virtual YSBOOL WouldProcessThisRawKey(int rawKey) const;
};


////////////////////////////////////////////////////////////


class FsGuiInFlightDialogWithNonConstantTextButton : public FsGuiInFlightDialogThatProcessNumberKeyAndEsc
{
protected:
	YSSIZE_T baseIndex;
	YSSIZE_T nChoice;
	// number of choices per page == varBtnArray.GetN()
	YsArray <FsGuiButton *> varBtnArray;
	FsGuiButton *prevPageBtn,*nextPageBtn;
public:
	FsGuiInFlightDialogWithNonConstantTextButton();

	/*! Reset function only resets baseIndex and nChoice, and do not change buttons. */
	void Reset(void);

	void SetNumChoice(YSSIZE_T nc);
	void MakeVariableTextButton(YSSIZE_T nBtn);
	void MakePageButton(void);
	YSBOOL UpdateText(YSSIZE_T btnIdx,const char str[]); // YSTRUE -> Text updated.
	void NextPage(void);
	void PrevPage(void);
	int NShow(void) const;
	FsGuiButton *NextPageButton(void);
	const FsGuiButton *NextPageButton(void) const;
	FsGuiButton *PrevPageButton(void);
	const FsGuiButton *PrevPageButton(void) const;
	int ClickedSelection(FsGuiButton *btn) const;
};


////////////////////////////////////////////////////////////

class FsReplayDialog : public FsGuiInFlightDialog
{
public:
	FsGuiTextBox *timeText[2];
	FsGuiButton *jumpButton[2],*captureButton[2];
	FsGuiButton *trimButton,*deleteButton;
	FsGuiButton *veryFastRewind,*fastRewind,*backward,*stepBack,*pause,*stepPlay,*play,*fastForward,*veryFastForward;
	FsGuiButton *rewindAllTheWay,*forwardAllTheWay;
	FsGuiButton *hideBtn;
	FsGuiButton *resumeBtn,*confirmResumeBtn;

	YSBOOL resume;

private:
	FsReplayDialog();
	~FsReplayDialog();

public:
	static FsReplayDialog *Create(void);
	static void Delete(FsReplayDialog *toDel);
	virtual void OnButtonClick(FsGuiButton *btn);
	void MakeForReplay(FsSimulation *sim);
	void MakeForEdit(FsSimulation *sim);
	YSBOOL IsTakingNumberKey(void) const;
};



class FsContinueDialog : public FsGuiInFlightDialog
{
public:
	FsGuiStatic *timeTxt,*playerStateTxt,*queryTxt;
	FsGuiButton *endButton,*contButton;

private:
	FsContinueDialog();
	~FsContinueDialog();
public:
	static FsContinueDialog *Create(void);
	static void Delete(FsContinueDialog *toDel);
	virtual void OnButtonClick(FsGuiButton *btn);
	void MakeDialog(FsSimulation *sim);
};



class FsGuiChatDialog : public FsGuiInFlightDialogThatProcessNumberKey
{
public:
	FsGuiTextBox *chatMsg;
	FsGuiButton *sendBtn,*cancelBtn,*clearBtn;

private:
	FsGuiChatDialog();
	~FsGuiChatDialog();

public:
	static FsGuiChatDialog *Create(void);
	static void Delete(FsGuiChatDialog *toDel);
	virtual void OnButtonClick(FsGuiButton *btn);
	void MakeDialog(FsSimulation *sim);
};



class FsGuiStationaryDialog : public FsGuiInFlightDialog
{
private:
	FsGuiButton *endButton;
	FsGuiButton *confirmEndButton;
	FsGuiButton *supplyButton;
	FsGuiButton *changeVehicleButton;

	YSBOOL confirmEndButtonPressed;
	YSBOOL supplyButtonPressed;
	YSBOOL changeVehicleButtonPressed;

private:
	FsGuiStationaryDialog();
	~FsGuiStationaryDialog();
public:
	static FsGuiStationaryDialog *Create(void);
	static void Delete(FsGuiStationaryDialog *toDel);

	void Make(class FsSimulation *sim);
	void SetUp(YSBOOL canSupply,YSBOOL canChangeVehicle);

	YSBOOL CheckConfirmEndButtonPressed(void) const;
	YSBOOL CheckSupplyButtonPressed(void) const;
	YSBOOL CheckChangeVehicleButtonPressed(void) const;

	void OnButtonClick(FsGuiButton *btn);
};

class FsGuiVehicleChangeDialog : public FsGuiInFlightDialog
{
private:
	FsGuiButton *okBtn,*cancelBtn;
	FsGuiListBox *vehicleList;

	FsGuiVehicleChangeDialog();
	~FsGuiVehicleChangeDialog();
public:
	static FsGuiVehicleChangeDialog *Create(void);
	static void Delete(FsGuiVehicleChangeDialog *toDel);

	void Make(class FsSimulation *sim);
	void SetUp(YSSIZE_T nObj,const class FsExistence * const obj[]);

	template <const int N>
	inline void SetUp(YsArray <const FsExistence *,N> &objArray);

	virtual void OnButtonClick(FsGuiButton *btn);
};

template <const int N>
inline void FsGuiVehicleChangeDialog::SetUp(YsArray <const FsExistence *,N> &objArray)
{
	SetUp(objArray.GetN(),objArray);
}

////////////////////////////////////////////////////////////

class FsGuiAutoPilotDialog : public FsGuiInFlightDialogThatProcessNumberKeyAndEsc
{
private:
	FsGuiButton *circleBtn,*straightBtn,*landingBtn,*takeoffBtn,*hdgBugBtn,*disengageBtn;
	FsGuiButton *cancelBtn;

	FsGuiAutoPilotDialog();
	~FsGuiAutoPilotDialog();
public:
	static FsGuiAutoPilotDialog *Create(void);
	static void Delete(FsGuiAutoPilotDialog *ptr);

	void Make(FsSimulation *sim);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void Circle(void);
	void Straight(void);
	void Landing(void);
	void TakeOff(void);
	void FlyHeadingBug(void);
	void Disengage(void);
};

class FsGuiAutoPilotVTOLDialog : public FsGuiInFlightDialogThatProcessNumberKeyAndEsc
{
private:
	FsGuiButton *hoverBtn,*verticalLandingBtn,*disengageBtn;
	FsGuiButton *cancelBtn;

	FsGuiAutoPilotVTOLDialog();
	~FsGuiAutoPilotVTOLDialog();
public:
	static FsGuiAutoPilotVTOLDialog *Create(void);
	static void Delete(FsGuiAutoPilotVTOLDialog *ptr);

	void Make(FsSimulation *sim);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void Hover(void);
	void VerticalLanding(void);
	void Disengage(void);
};

class FsGuiAutoPilotHelicopterDialog : public FsGuiInFlightDialogThatProcessNumberKeyAndEsc
{
private:
	FsGuiButton *hoverBtn,*verticalLandingBtn,*disengageBtn;
	FsGuiButton *cancelBtn;

	FsGuiAutoPilotHelicopterDialog();
	~FsGuiAutoPilotHelicopterDialog();
public:
	static FsGuiAutoPilotHelicopterDialog *Create(void);
	static void Delete(FsGuiAutoPilotHelicopterDialog *ptr);

	void Make(FsSimulation *sim);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void Hover(void);
	void VerticalLanding(void);
	void Disengage(void);
};

class FsGuiRadioCommFuelTruckDialog : public FsGuiInFlightDialogThatProcessNumberKeyAndEsc
{
private:
	FsGuiButton *callBtn,*dismissBtn,*neverMindBtn;

	YSHASHKEY fuelTruckOnCallKey;

	FsGuiRadioCommFuelTruckDialog();
	~FsGuiRadioCommFuelTruckDialog();
public:
	static FsGuiRadioCommFuelTruckDialog *Create(void);
	static void Delete(FsGuiRadioCommFuelTruckDialog *ptr);

	void CacheCallableFuelTruck(void);
	void Make(FsSimulation *sim);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void Call(void);
	void Dismiss(void);
	void Close(void);
};

////////////////////////////////////////////////////////////

class FsGuiRadioCommDialogBase : public FsGuiInFlightDialogWithNonConstantTextButton
{
public:
	enum CALLABLETYPE
	{
		AIRCRAFT,
		ALLAIRCRAFT,
		FORMATION,
		GUNNER,
		ATC,
		FUELTRUCK
	};

	class Callable
	{
	public:
		CALLABLETYPE callableType;
		YSHASHKEY callableKey;
	};

	static YsArray <Callable,16> GetCallable(FsSimulation *sim,FsExistence *player);
};


////////////////////////////////////////////////////////////

class FsGuiRadioCommTargetDialog : public FsGuiRadioCommDialogBase
{
private:
	enum
	{
		NSHOW=5
	};

	class KeyButtonCallablePair
	{
	public:
		int fsKey;
		FsGuiButton *btn;
		Callable callable;
	};

	FsGuiButton *cancelBtn;
	YsArray <Callable,16> callableAir;
	YsArray <KeyButtonCallablePair,16> keyBtnCallablePairArray;

	FsGuiRadioCommTargetDialog();
	~FsGuiRadioCommTargetDialog();
public:
	static FsGuiRadioCommTargetDialog *Create(void);
	static void Delete(FsGuiRadioCommTargetDialog *ptr);

	void Make(class FsSimulation *sim,YSSIZE_T nCallable,const Callable callable[]);
	template <const int N>
	void Make(class FsSimulation *sim,const YsArray <Callable,N> &allCallable)
	{
		Make(sim,allCallable.GetN(),allCallable);
	}

	void ResetVariableTextButton(void);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void CallableSelected(const Callable &callable);
};

////////////////////////////////////////////////////////////

class FsGuiRadioCommCommandDialog : public FsGuiInFlightDialogThatProcessNumberKeyAndEsc
{
private:
	YsArray <YSHASHKEY> comTargetKey;

	FsGuiButton *breakAndAttackBtn,*attackGroundTargetBtn,*helpMeBtn,*formOnMyWingBtn,*returnToBaseBtn;
	FsGuiButton *stayInHoldingBtn,*landRefuelAndTakeOffBtn,*cancelBtn;

	FsGuiRadioCommCommandDialog();
	~FsGuiRadioCommCommandDialog();

	void GetComTarget(YsArray <class FsAirplane *,16> &target) const;
public:
	static FsGuiRadioCommCommandDialog *Create(void);
	static void Delete(FsGuiRadioCommCommandDialog *ptr);

	void Make(FsSimulation *sim);

	void SetCommTarget(YSSIZE_T n,class FsAirplane *const air[]);
	template <const int N>
	void SetCommTarget(const YsArray <class FsAirplane *,N> &air);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void BreakAndAttack(void);
	void AttackGroundTarget(void);
	void HelpMe(void);
	void FormOnMyWing(void);
	void ReturnToBase(void);
	void StayInHoldingPattern(void);
	void LandRefuelAndTakeOff(void);
	void Close(void);
};

template <const int N>
void FsGuiRadioCommCommandDialog::SetCommTarget(const YsArray <FsAirplane *,N> &air)
{
	return SetCommTarget(air.GetN(),air);
}

////////////////////////////////////////////////////////////

class FsGuiSelectApproachDialog : public FsGuiInFlightDialogWithNonConstantTextButton
{
private:
	FsGuiButton *cancelBtn;

	FsGuiSelectApproachDialog();
	~FsGuiSelectApproachDialog();
public:
	static FsGuiSelectApproachDialog *Create(void);
	static void Delete(FsGuiSelectApproachDialog *ptr);

	void Make(FsSimulation *sim);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);
	virtual void UpdateDialog(void);

	void Select(int sel);
	void Close(void);
};

////////////////////////////////////////////////////////////

class FsGuiAtcRequestDialog : public FsGuiInFlightDialogWithNonConstantTextButton
{
private:
	FsGuiButton *requestApproachBtn,*declareMissedApproachBtn,*requestNewVectorBtn,*requestVectorBtn,*cancelIFRBtn;
	FsGuiButton *cancelBtn;

	YsArray <FsGuiButton *,8> activeBtn;

	FsGuiAtcRequestDialog();
	~FsGuiAtcRequestDialog();
public:
	static FsGuiAtcRequestDialog *Create(void);
	static void Delete(FsGuiAtcRequestDialog *ptr);

	void Make(FsSimulation *sim);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void RequestApproach(void);
	void DeclareMissedApproach(void);
	void RequestNewVector(void);
	void RequestVector(void);
	void CancelIFR(void);
	void Close(void);
};

////////////////////////////////////////////////////////////

class FsGuiRadioCommToFormationDialog : public FsGuiInFlightDialogThatProcessNumberKeyAndEsc
{
private:
	FsGuiButton *spreadBtn,*tightenBtn;
	FsGuiButton *cancelBtn;

	FsGuiRadioCommToFormationDialog();
	~FsGuiRadioCommToFormationDialog();
public:
	static FsGuiRadioCommToFormationDialog *Create(void);
	static void Delete(FsGuiRadioCommToFormationDialog *ptr);

	void Make(FsSimulation *sim);

	virtual void OnButtonClick(FsGuiButton *btn);
	virtual YSBOOL ProcessRawKeyInput(int rawKey);

	void SpreadFormation(void);
	void TightenFormation(void);
	void Close(void);
};

////////////////////////////////////////////////////////////


/* } */
#endif
