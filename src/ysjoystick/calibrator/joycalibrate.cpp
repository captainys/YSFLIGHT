#include <stdio.h>
#include <ysclass.h>
#include <ysport.h>
#include <ysfontrenderer.h>

#include <ysgl.h>
#include <ysbitmap.h>

#include <fssimplewindow.h>
#include <ysglfontdata.h>
#include <fsgui.h>



YsTextResource fsTextResource;



#include "ysjoyreader.h"
#include "fsjoycalibdlg.h"


inline const wchar_t *FsGetTextResource(const char *key,const wchar_t *alternative)
{
	const wchar_t *primary=fsTextResource.FindWString(key);
	if(NULL!=primary)
	{
		return primary;
	}
	return alternative;
}


#define FSJC_BMP_FILEMENU           FsGetTextResource("joycalib_filemenu"         ,L"File")
#define FSJC_BMP_FILE_EXITMENU      FsGetTextResource("joycalib_fileexitmenu"     ,L"Exit")
#define FSJC_BMP_CALIBMENU          FsGetTextResource("joycalib_calibmenu"        ,L"Calibrate")
#define FSJC_BMP_CALIB_CALIBMENU    FsGetTextResource("joycalib_calibcalibmenu"   ,L"Calibrate")
#define FSJC_BMP_CALIB_CALIBHATMENU FsGetTextResource("joycalib_calibcalibhatmenu",L"Calibrate Hat Switch")
#define FSJC_BMP_CALIB_ERASEMENU    FsGetTextResource("joycalib_caliberasemenu"   ,L"Erase Calibration Information")







class FsMainMenu : public FsGuiPopUpMenu
{
public:
	FsGuiPopUpMenuItem *fileMenu;
	FsGuiPopUpMenuItem *fileOpen,*fileSave,*fileExit;

	FsGuiPopUpMenuItem *calibMenu;
	FsGuiPopUpMenuItem *calibWizard,*calibHat,*calibEraseAll;

	class FsCalibrationDialog *calibDialog;

	FsMainMenu(class FsCalibrationDialog *dlg);
	void MakeMenu(void);
	virtual void OnSelectMenuItem(FsGuiPopUpMenuItem *item);
};

FsMainMenu::FsMainMenu(class FsCalibrationDialog *dlg)
{
	this->calibDialog=dlg;
}

void FsMainMenu::MakeMenu(void)
{
	pullDown=YSTRUE;
	fileMenu=AddTextItem(0,FSKEY_F,FSJC_BMP_FILEMENU);
	fileExit=fileMenu->GetSubMenu()->AddTextItem(0,FSKEY_X,FSJC_BMP_FILE_EXITMENU);

	calibMenu=AddTextItem(0,FSKEY_C,FSJC_BMP_CALIBMENU);
	calibWizard=calibMenu->GetSubMenu()->AddTextItem(0,FSKEY_C,FSJC_BMP_CALIB_CALIBMENU);
#ifdef YSJOYREADER_USE_HAT_CALIBRATION
	calibHat=calibMenu->GetSubMenu()->AddTextItem(0,FSKEY_C,FSJC_BMP_CALIB_CALIBHATMENU);
#endif
	calibEraseAll=calibMenu->GetSubMenu()->AddTextItem(0,FSKEY_E,FSJC_BMP_CALIB_ERASEMENU);
}

void FsMainMenu::OnSelectMenuItem(FsGuiPopUpMenuItem *item)
{
	if(item==fileExit)
	{
		exit(0);
	}
	if(item==calibWizard)
	{
		int joyId,axisType[YsJoyReaderMaxNumAxis];
		joyId=calibDialog->joyId->GetSelection();
		if(0<=joyId && joyId<calibDialog->nJoystick)
		{
			calibDialog->GetAxisType(axisType);
			calibDialog->CalibrateJoystick(calibDialog->joystick[joyId],axisType);
			YsJoyReaderSaveJoystickCalibrationInfo(calibDialog->nJoystick,calibDialog->joystick);
		}
	}
#ifdef YSJOYREADER_USE_HAT_CALIBRATION
	if(item==calibHat)
	{
		int joyId,axisType[YsJoyReaderMaxNumAxis];
		joyId=calibDialog->joyId->GetSelection();
		if(0<=joyId && joyId<calibDialog->nJoystick)
		{
			calibDialog->GetAxisType(axisType);
			calibDialog->CalibrateHatSwitch(calibDialog->joystick[joyId]);
			YsJoyReaderSaveJoystickCalibrationInfo(calibDialog->nJoystick,calibDialog->joystick);
		}
	}
#endif
	if(item==calibEraseAll)
	{
		FILE *fp;
		fp=YsJoyReaderOpenJoystickCalibrationFile("w");
		if(fp!=NULL)
		{
			fclose(fp);
		}
	}
}

#ifdef MACOSX
extern "C" void FsChangeToProgramDirC(void);
#endif

int main(void)
{
	FsChangeToProgramDir();


	YsSystemFontRenderer *sysFont=new YsSystemFontRenderer;
	FsGuiObject::defUnicodeRenderer=sysFont;


	const int FsMaxNumJoystick=6;
	int nJoystick;
	YsJoyReader joystick[FsMaxNumJoystick];

	FsMainMenu *mainMenu;
	FsCalibrationDialog *calibDialog;

	YsJoyReaderSetUpJoystick(nJoystick,joystick,FsMaxNumJoystick);
	YsJoyReaderLoadJoystickCalibrationInfo(nJoystick,joystick);


	YsLocale locale;
	if(NULL!=locale.GetLanguagePart())
	{
		YsString uifn(locale.GetLanguagePart());
		uifn.Append(".uitxt");

		YsString ful;
		ful.MakeFullPathName("language",uifn);

		fsTextResource.LoadFile(ful);
	}


	FsOpenWindow(32,32,800,600,1);

	FsGuiObject::defUnicodeRenderer->RequestDefaultFontWithPixelHeight(14);
	FsGuiObject::defAsciiRenderer->RequestDefaultFontWithPixelHeight(12);


	calibDialog=new FsCalibrationDialog(nJoystick,joystick);
	mainMenu=new FsMainMenu(calibDialog);


	mainMenu->MakeMenu();
	calibDialog->MakeDialog();
	calibDialog->InitialSetup();

	FsPollDevice();
	while(YSTRUE!=calibDialog->exitDlg)
	{
		int key,chr;
		int lb,mb,rb,mx,my;

		FsPollDevice();

		while(0!=FsGetMouseEvent(lb,mb,rb,mx,my))
		{
			mainMenu->SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
			calibDialog->SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
		}

		while(FSKEY_NULL!=(key=FsInkey()))
		{
			mainMenu->KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
			calibDialog->KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
		}

		while(0!=(chr=FsInkeyChar()))
		{
		}


		glClearColor(0.7,0.7,0.7,1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		int wid,hei;
		FsGetWindowSize(wid,hei);
		glViewport(0,0,wid,hei);

		glDepthFunc(GL_ALWAYS);
		glDepthMask(0);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-0.5,(GLdouble)wid-0.5,(GLdouble)hei-0.5,-0.5,-1,1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		int selJoyId,axisType[YsJoyReaderMaxNumAxis];
		selJoyId=calibDialog->joyId->GetSelection();
		if(selJoyId<nJoystick)
		{
			joystick[selJoyId].Read();
			calibDialog->SetValueText(joystick[selJoyId]);
			calibDialog->GetAxisType(axisType);

			calibDialog->ShowJoystickAxes(joystick[selJoyId],axisType);
		}

		calibDialog->Show();
		mainMenu->Show();

		glFlush();
		FsSwapBuffers();

		FsSleep(10);
	}

	return 0;
}

