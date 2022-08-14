#ifndef FSRESULTDIALOG_IS_INCLUDED
#define FSRESULTDIALOG_IS_INCLUDED
/* { */

#include <fsgui.h>
#include <ysclass.h>
#include "fssimulation.h"
#include "fsguiselectiondialogbase.h"

class FsGuiMissionGoalDialogClass : public FsGuiDialogWithFieldAndAircraft
{
public:
	int nextActionCode;
	FsGuiButton *okBtn;
	FsGuiTabControl *mainTab;

	YSRESULT Create(const class FsSimulation *sim,int nextActionCode);
	virtual void OnButtonClick(FsGuiButton *btn);
private:
	int AddDescriptionTab(const class FsSimulation *sim);
	int AddBitmapTab(const class FsSimulation *sim);
	int AddDetailTab(const class FsSimulation *sim);
	YsBitmap LoadBitmap(const FsSimulation *sim);
};



class FsGuiResultDialogClass : public FsGuiDialogWithFieldAndAircraft
{
private:
	FsGuiTabControl *mainTab;
	int missionGoalTabId;
	int summaryTabId;
	int scoreDetailTabId;
	int landingAnalysisTabId;

	const FsSimulation *sim;
	int nextActionCode;
	YsSegmentedArray <FsLandingAnalysis,4> landingArray;

	YSSIZE_T landingProfileTDIndexCache;
	YSSIZE_T landingProfileThresholdIndexCache;
	YsArray <YsVec2> landingVerticalProfileCache;
	YsArray <YsVec2> landingHorizontalProfileCache;
	double landingHorizontalProfileRotation;
	YsVec2 landingHorizontalProfileTranslation;
	YsVec2 landingHorizontalProfileDimension;

	YSBOOL rwRectCacheValid;
	YsVec2 rwRectCache[4];

	FsGuiDropList *landingDropList;
	FsGuiStatic *landingVerticalSpeed;
	FsGuiStatic *landingHeadingDifference;
	FsGuiButton *landingProfileViewSwitch[2];

public: // Temporarily public
	FsGuiButton *okBtn1,*okBtn2;

private:
	FsGuiResultDialogClass();
	~FsGuiResultDialogClass();

public:
	static FsGuiResultDialogClass *Create(void);
	static void Delete(FsGuiResultDialogClass *);

	YSRESULT Make(const class FsSimulation *sim,int nextActionCode);
	virtual void OnButtonClick(FsGuiButton *btn);
private:
	int AddMissionGoalTab(const class FsSimulation *sim);
	int AddSummaryTab(const class FsSimulation *sim);
	int AddScoreDetailTab(const class FsSimulation *sim);
	int AddLandingAnalysisTab(const class FsSimulation *sim);
	void MakeLandingArray(const class FsSimulation *sim);
	void AddLandingAnalysis(const class FsSimulation *sim);
	void SelectLanding(const FsSimulation *sim,int ldgIdx);
	void CacheVerticalProfile(const FsSimulation *sim,int ldgIdx);
	void CacheHorizontalProfileTransformation(void);
	YsVec2 CalculateHorizontalProfileDimension(YSSIZE_T np,const YsVec2 p[]);
	const wchar_t *FinalAirplaneStateString(YsColor &col,YsWString &str,FSFLIGHTSTATE sta) const;
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
	virtual void Show(const FsGuiDialog *excludeFromDrawing) const;
};

/* } */
#endif
