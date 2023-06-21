#ifndef YSKEYMENU_IS_INCLUDED
#define YSKEYMENU_IS_INCLUDED
/* { */

#include <fsgui.h>

class FsGuiMainMenu : public FsGuiPopUpMenu
{
public:
	class FsWorld *world; // <- This should not be cached.  Always must use runLoop->GetWorld()
	class FsGuiMainCanvas *canvas;
	class FsRunLoop *runLoop;

	FsGuiPopUpMenu *fileRecent;
	FsGuiRecentFiles recent;



	enum
	{
		NUM_LANDING_PRACTICE_LEVEL=15
	};
	FsGuiPopUpMenuItem *simLdgPracticeLevel[NUM_LANDING_PRACTICE_LEVEL];  // This need to stay.

	FsGuiPopUpMenuItem *acroStarOfDavid;
	FsGuiPopUpMenuItem *acroCorkScrew,*acroSlowRoll,*acroDeltaLoop,*acroDeltaRoll,*acroTightTurn,*acro360Loop;
	FsGuiPopUpMenuItem *acroBombBurstUpward4,*acroBombBurstUpward6,*acroChangeOverTurn,*acroTrailtoDiamondRoll;
	FsGuiPopUpMenuItem *acroCubanEight,*acroDeltaLoopandBonton,*acroBontolRoll,*acroBombBurstDownward4;
	FsGuiPopUpMenuItem *acroBombBurstDownward6,*acroRailFall,*acroRollingCombatPitch,*acroDiamondTakeOff;
	FsGuiPopUpMenuItem *acroContinuousRoll,*acroRollOnTakeOff,*acroTackCross,*acroBigHeart,*acroLevelBreak;
	FsGuiPopUpMenuItem *acroRollBacktoArrowhead,*acroPitchUpBreak,*acroRockWingGearDown,*acroRockWingClean;
	FsGuiPopUpMenuItem *acroLetterEight,*acroStarCross,*acroLevelOpener,*acroFormationBreak,*acroLineAbreastRoll;
	FsGuiPopUpMenuItem *acroLineAbreastLoop,*acroDoubleFarvel,*acroDiamond9toSwanBend,*acroSwantoApolloRoll;
	FsGuiPopUpMenuItem *acroLancastertoSplit,*acroChampaignSplit,*acroVixenBreak,*acroBigBtlToDiamondLoop;

	FsGuiPopUpMenuItem *helpJ,*helpE,*helpDefKeyJ,*helpDefKeyE,*helpAbout,*helpSupport;

	void Make(void);
	void RefreshRecentlyUsedFileList(void);
	void AddRecentlyUsedFile(const wchar_t wfn[]);

	virtual void OnSelectMenuItem(FsGuiPopUpMenuItem *item);
};

class FsGuiMainDialog : public FsGuiDialog
{
public:
	FsGuiStatic *statusMsg;
	FsGuiButton *newFlightBtn,*loadFlightBtn,*saveFlightBtn;
	FsGuiButton *flyNowBtn,*replayRecordBtn,*retryPrevMissionBtn;
	FsGuiButton *englishBtn,*languageBtn;
	FsGuiButton *showConsoleBtn;

	FsGuiButton *votePageBtn;

	class FsGuiMainCanvas *canvas;
	class FsRunLoop *runLoop;

	void Make(void);
	virtual void OnButtonClick(FsGuiButton *btn);
};

char FsTranslateKey(int fskey);
char FsTranslateKeyShift(int fskey,YSBOOL shift);

YSBOOL FsKeyMessageBox(const char title[],const char msg[],const char *yesBtnTxt="(Y)es",const char *noBtnTxt="(N)o");

/* } */
#endif
