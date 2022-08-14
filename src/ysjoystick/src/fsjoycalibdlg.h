#ifndef FSJOYCALIBDLG_IS_INCLUDED
#define FSJOYCALIBDLG_IS_INCLUDED
/* { */

#include "ysjoyreader.h"

#ifdef YSFLIGHT
#include <fstextresource.h>
#else
extern YsTextResource fsTextResource;

inline const wchar_t *FsJoyCalibGUIMessage(const char msgkey[],const wchar_t altTxt[])
{
	auto msg=fsTextResource.FindWString(msgkey);
	if(NULL!=msg)
	{
		return msg;
	}
	else
	{
		return altTxt;
	}
}

#define FSJC_BMP_CALIB_CALIBAXES    FsJoyCalibGUIMessage("joycalib_calib",     L"Calibrate Axes")
#define FSJC_BMP_CALIB_CALIBHAT     FsJoyCalibGUIMessage("joycalib_calibhat",  L"Calibrate Hat Switch")
#define FSJC_BMP_CALIB_EXIT         FsJoyCalibGUIMessage("joycalib_fileexit",  L"Exit")
#define FSJC_BMP_CALIB_ERASE        FsJoyCalibGUIMessage("joycalib_caliberase",L"Erase All Calibration Data")

#define FSJC_BMP_CALIB_HATNEUTRAL   FsJoyCalibGUIMessage("joycalib_hatneutral",L"Release Hat Switch and Press SPACE key")
#define FSJC_BMP_CALIB_HAT0DEG      FsJoyCalibGUIMessage("joycalib_hat0deg",   L"Push Hat Switch Forward (or Up) and Press SPACE key")
#define FSJC_BMP_CALIB_HAT90DEG     FsJoyCalibGUIMessage("joycalib_hat90deg",  L"Push Hat Switch Right and Press SPACE key")
#define FSJC_BMP_CALIB_HAT180DEG    FsJoyCalibGUIMessage("joycalib_hat180deg", L"Push Hat Switch Backward (or Down) and Press SPACE key")
#define FSJC_BMP_CALIB_HAT270DEG    FsJoyCalibGUIMessage("joycalib_hat270deg", L"Push Hat Switch Left and Press SPACE key")

#define FSJC_BMP_CALIB_AXISCYCLE    FsJoyCalibGUIMessage("joycalib_axiscycle",   L"Move Axis All the Way Up and Down Several Times and Press SPACE key")
#define FSJC_BMP_CALIB_AXISNEUTRAL  FsJoyCalibGUIMessage("joycalib_axisneutral", L"Center Axis and Press SPACE key")
#define FSJC_BMP_CALIB_STICKCIRCLE  FsJoyCalibGUIMessage("joycalib_stickcircle", L"Make Complete Circle with the Stick Several Times and Press SPACE key")
#define FSJC_BMP_CALIB_STICKNEUTRAL FsJoyCalibGUIMessage("joycalib_stickneutral",L"Center Joystick and Press SPACE key")
#define FSJC_BMP_CALIB_NEXT         FsJoyCalibGUIMessage("joycalib_next",        L"Next >>")
#endif

enum 
{
	FSJOY_AXISTYPE_STICKORYOKE,
	FSJOY_AXISTYPE_QUADRANT,
	FSJOY_AXISTYPE_AXISWITHCENTER
};

class FsCalibrationDialog : public FsGuiDialog
{
public:
	int nJoystick;
	YsJoyReader *joystick;
	YSBOOL exitDlg;

	FsGuiButton *calibrateBtn;
#ifdef YSJOYREADER_USE_HAT_CALIBRATION
	FsGuiButton *calibrateHatBtn;
#endif
	FsGuiButton *eraseInfoBtn,*exitBtn;

	FsGuiDropList *joyId;
	FsGuiDropList *joyType;

#ifdef GLX
	FsGuiDropList *povXAxis,*povYAxis;
#endif

	FsGuiStatic *axisRaw[YsJoyReaderMaxNumAxis],*axisCalib[YsJoyReaderMaxNumAxis];
	FsGuiStatic *button;
	FsGuiStatic *hatSwitchRaw,*hatSwitchCalibrated;

	FsCalibrationDialog();
	FsCalibrationDialog(int nJoystick,YsJoyReader *joystick);
	void SetJoystick(int nJoystick,YsJoyReader *joystick);
	void MakeDialog(void);
	void SetValueText(const YsJoyReader &joystick) const;
	void InitialSetup(void);
	void GetAxisType(int axisType[YsJoyReaderMaxNumAxis]) const;

	void CalibrateJoystick(YsJoyReader &joystick,int axisType[]);
	void CalibrateHatSwitch(YsJoyReader &joystick);
	void CaptureHat(YsJoyReader &joystick,int hatId,int &hatValueCapture,FsGuiDialog *dlg,FsGuiButton *nextBtn,FsGuiStatic *hatState);
	void ShowJoystickAxes(const YsJoyReader &joy,const int axisType[]) const;

	// fsjoycalibdlggl.cpp >>
	void ClearScreen(void) const;
	void ShowJoystickAxis(const YsJoyReader &joy,const int axisType[],int axis,int x,int y) const;
	void ShowHatSwitch(const YsJoyReader &joy,int hatId,int x,int y) const;
	// fsjoycalibdlggl.cpp <<

	virtual void Interval(void);
	virtual void OnButtonClick(FsGuiButton *btn);
#ifdef GLX
	virtual void OnDropListSelChange(FsGuiDropList *drp,int prevSel);
#endif
	virtual void Show(const FsGuiDialog *excludeFromDrawing=NULL) const;

	void OnCalibrateAxisDialogClosed(FsGuiDialog *dlg,int returnCode);
};


/* } */
#endif
