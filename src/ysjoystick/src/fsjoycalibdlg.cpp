#include <stdio.h>
#include <ysclass.h>


#include <ysbitmap.h>

#ifndef YSFLIGHT
	#include <fssimplewindow.h>
#else
	#include "fsdef.h"
	#include "fswindow.h"
	#include <fssimplewindow.h>
#endif


#include <fsgui.h>


#include "ysjoyreader.h"
#include "fsjoycalibdlg.h"


////////////////////////////////////////////////////////////

class FsGuiCalibrateJoystickAxisDialog : public FsGuiDialog
{
public:
	FsGuiButton *nextBtn,*cancelBtn;

	YsJoyReader *joyPtr;
	int axisType[YsJoyReaderMaxNumAxis];

	enum
	{
		PHASE_NONE,
		PHASE_FIRST_CENTER,
		PHASE_LIMIT,
		PHASE_SECOND_CENTER,
	};

	int axis;
	int phase;

	void Make(int axis);
	void Make(int axis,int phase);
	int FetchFirstPhase(int axis);
	virtual void Interval(void);
	virtual void OnButtonClick(FsGuiButton *btn);
};

////////////////////////////////////////////////////////////

FsCalibrationDialog::FsCalibrationDialog()
{
}

FsCalibrationDialog::FsCalibrationDialog(int nJoystick,YsJoyReader *joystick)
{
	SetJoystick(nJoystick,joystick);
}

void FsCalibrationDialog::SetJoystick(int nJoystick,YsJoyReader *joystick)
{
	this->nJoystick=nJoystick;
	this->joystick=joystick;
}

void FsCalibrationDialog::MakeDialog(void)
{
	const char *const joyIdString[]=
	{
		"Joystick 1",
		"Joystick 2",
		"Joystick 3",
		"Joystick 4",
		"Joystick 5",
		"Joystick 6"
	};

	const char *const joyTypeString[]=
	{
		"Only One Stick/Yoke",
		"1:Stick 2:Throttle",
		"1:Stick 2:Rudder",
		"1:Stick 2:Throttle 3:Rudder",
		"1:Stick 2:Rudder 3:Throttle",
		"All Quadrant",
		"All Sliders with Center",
	};

#ifdef GLX
	const char *const povAxsString[]=
	{
		"No Hat Switch",
		"Axis 1",
		"Axis 2",
		"Axis 3",
		"Axis 4",
		"Axis 5",
		"Axis 6",
		"Axis 7",
		"Axis 8",
		NULL
	};
#endif


	SetSize(800,600);
	SetTopLeftCorner(0,24);

	calibrateBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_CALIBAXES,YSTRUE);
#ifdef YSJOYREADER_USE_HAT_CALIBRATION
	calibrateHatBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_CALIBHAT,YSFALSE);
#endif

	eraseInfoBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_ERASE,YSFALSE);
	exitBtn=AddTextButton(0,FSKEY_ESC,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_EXIT,YSFALSE);

	joyId=AddDropList(0,FSKEY_NULL,"",6,joyIdString,10,20,20,YSFALSE);
	joyId->Select(0);

	joyType=AddDropList(0,FSKEY_NULL,"",7,joyTypeString,10,30,30,YSFALSE);
	joyType->Select(0);

#ifdef GLX
	AddStaticText(0,FSKEY_NULL,"Hat Axes",10,1,YSTRUE);
	povXAxis=AddDropList(0,FSKEY_NULL,"",9,povAxsString,10,30,30,YSFALSE);
	povXAxis->Select(0);
	povYAxis=AddDropList(0,FSKEY_NULL,"",9,povAxsString,10,30,30,YSFALSE);
	povYAxis->Select(0);
#endif

	int i;
	for(i=0; i<YsJoyReaderMaxNumAxis; i++)
	{
		axisRaw[i]=AddStaticText(0,FSKEY_NULL,"AX# Raw:",16,1,(i%2==0 ? YSTRUE : YSFALSE));
		axisRaw[i]->SetFill(YSFALSE);
		axisRaw[i]->SetDrawFrame(YSFALSE);
		axisCalib[i]=AddStaticText(0,FSKEY_NULL,"AX# Calibrated:",32,1,YSFALSE);
		axisCalib[i]->SetFill(YSFALSE);
		axisCalib[i]->SetDrawFrame(YSFALSE);
	}

	button=AddStaticText(0,FSKEY_NULL,"Button:",48,1,YSTRUE);
	button->SetFill(YSFALSE);
	button->SetDrawFrame(YSFALSE);

	hatSwitchRaw=AddStaticText(0,FSKEY_NULL,"HatSwitchRaw:",16,1,YSTRUE);
	hatSwitchRaw->SetFill(YSFALSE);
	hatSwitchRaw->SetDrawFrame(YSFALSE);

	hatSwitchCalibrated=AddStaticText(0,FSKEY_NULL,"HatSwitchInterpreted:",48,1,YSFALSE);
	hatSwitchCalibrated->SetFill(YSFALSE);
	hatSwitchCalibrated->SetDrawFrame(YSFALSE);

	SetTransparency(YSFALSE);

	Fit();

	exitDlg=YSFALSE;
}

void FsCalibrationDialog::InitialSetup(void)
{
#ifdef GLX
  int joyId=this->joyId->GetSelection();
  if(0<=joyId && joyId<nJoystick)
    {
      povXAxis->Select(joystick[joyId].povXAxis+1);
      povYAxis->Select(joystick[joyId].povYAxis+1);
    }
#endif
}

void FsCalibrationDialog::SetValueText(const YsJoyReader &joystick) const
{
	int i;
	char str[256];
	for(i=0; i<YsJoyReaderMaxNumAxis; i++)
	{
		if(joystick.axis[i].exist!=0)
		{
			sprintf(str,"AX%d Raw:%d",i+1,joystick.axis[i].value);
			axisRaw[i]->SetText(str);
			sprintf(str,"AX%d Calibrated:%.3lf",i+1,joystick.axis[i].GetCalibratedValue());
			axisCalib[i]->SetText(str);
		}
		else
		{
			sprintf(str,"AX%d Raw:---",i);
			axisRaw[i]->SetText(str);
			sprintf(str,"AX%d Calibrated:---",i);
			axisCalib[i]->SetText(str);
		}
	}

	strcpy(str,"BUTTON:");
	for(i=0; i<YsJoyReaderMaxNumButton; i++)
	{
		if(joystick.button[YsJoyReaderMaxNumButton-1-i].exist!=0)
		{
			if(joystick.button[YsJoyReaderMaxNumButton-1-i].value==0)
			{
				str[7+i]='0';
			}
			else
			{
				str[7+i]='1';
			}
		}
		else
		{
			str[7+i]='-';
		}
	}
	str[7+YsJoyReaderMaxNumButton]=0;
	button->SetText(str);

	if(joystick.hatSwitch[0].exist!=0)
	{
	    sprintf(str,"HAT:%d",joystick.hatSwitch[0].value);
	}
	else
	{
	    strcpy(str,"HAT:---");
	}
	hatSwitchRaw->SetText(str);

	if(joystick.hatSwitch[0].exist!=0)
	{
		switch(joystick.hatSwitch[0].GetDiscreteValue())
		{
		case 0:
			hatSwitchCalibrated->SetText("HAT(Interpreted):Neutral");
			break;
		case 1:
			hatSwitchCalibrated->SetText("HAT(Interpreted):0deg");
			break;
		case 2:
			hatSwitchCalibrated->SetText("HAT(Interpreted):45deg");
			break;
		case 3:
			hatSwitchCalibrated->SetText("HAT(Interpreted):90deg");
			break;
		case 4:
			hatSwitchCalibrated->SetText("HAT(Interpreted):135deg");
			break;
		case 5:
			hatSwitchCalibrated->SetText("HAT(Interpreted):180deg");
			break;
		case 6:
			hatSwitchCalibrated->SetText("HAT(Interpreted):225deg");
			break;
		case 7:
			hatSwitchCalibrated->SetText("HAT(Interpreted):270deg");
			break;
		case 8:
			hatSwitchCalibrated->SetText("HAT(Interpreted):315deg");
			break;
		}
	}
	else
	{
		hatSwitchCalibrated->SetText("HAT(Interpreted):---");
	}

}

void FsCalibrationDialog::GetAxisType(int axisType[YsJoyReaderMaxNumAxis]) const
{
	int i;
	for(i=0; i<YsJoyReaderMaxNumAxis; i++)
	{
		axisType[i]=FSJOY_AXISTYPE_AXISWITHCENTER;
	}
	switch(joyType->GetSelection())
	{
	case 0: // "Only one stick",
		axisType[0]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[1]=FSJOY_AXISTYPE_STICKORYOKE;
		break;
	case 1: // "1:Stick 2:Throttle",
		axisType[0]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[1]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[2]=FSJOY_AXISTYPE_QUADRANT;
		break;
	case 2: // "1:Stick 2:Rudder",
		axisType[0]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[1]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[2]=FSJOY_AXISTYPE_AXISWITHCENTER;
		break;
	case 3: // "1:Stick 2:Throttle 3:Rudder",
		axisType[0]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[1]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[2]=FSJOY_AXISTYPE_QUADRANT;
		axisType[3]=FSJOY_AXISTYPE_AXISWITHCENTER;
		break;
	case 4: // "1:Stick 2:Rudder 3:Throttle",
		axisType[0]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[1]=FSJOY_AXISTYPE_STICKORYOKE;
		axisType[2]=FSJOY_AXISTYPE_AXISWITHCENTER;
		axisType[3]=FSJOY_AXISTYPE_QUADRANT;
		break;
	case 5: // "All Quadrant",
		for(i=0; i<YsJoyReaderMaxNumAxis; i++)
		{
			axisType[i]=FSJOY_AXISTYPE_QUADRANT;
		}
		break;
	default:
	case 6: // "All Sliders with Center",
		// Default
		break;
	}
}

/* virtual */ void FsCalibrationDialog::Interval(void)
{
	SetNeedRedraw(YSTRUE);
}

void FsCalibrationDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn==calibrateBtn)
	{
		int joyId=this->joyId->GetSelection();
		if(0<=joyId && joyId<nJoystick)
		{
			for(int axis=0; axis<YsJoyReaderMaxNumAxis; ++axis)
			{
				if(0!=joystick[joyId].axis[axis].exist)
				{
					auto dlg=FsGuiDialog::CreateSelfDestructiveDialog <FsGuiCalibrateJoystickAxisDialog>();
					dlg->joyPtr=&joystick[joyId];
					GetAxisType(dlg->axisType);
					dlg->Make(axis);
					dlg->BindCloseModalCallBack(&FsCalibrationDialog::OnCalibrateAxisDialogClosed,this);
					AttachModalDialog(dlg);
					break;
				}
			}
		}
	}
#ifdef YSJOYREADER_USE_HAT_CALIBRATION
	else if(btn==calibrateHatBtn)
	{
		int joyId;
		joyId=this->joyId->GetSelection();
		CalibrateHatSwitch(joystick[joyId]);
		YsJoyReaderSaveJoystickCalibrationInfo(nJoystick,joystick);
	}
#endif
	else if(btn==eraseInfoBtn)
	{
		FILE *fp;
		fp=YsJoyReaderOpenJoystickCalibrationFile("w");
		if(fp!=NULL)
		{
			fclose(fp);
		}
	}
	else if(btn==exitBtn)
	{
		exitDlg=YSTRUE;
		CloseModalDialog(0);
	}
}

void FsCalibrationDialog::OnCalibrateAxisDialogClosed(FsGuiDialog *dlg,int returnCode)
{
	auto calDlg=dynamic_cast <FsGuiCalibrateJoystickAxisDialog *>(dlg);
	if(nullptr!=calDlg && (int)YSOK==returnCode)
	{
printf("%s %d\n",__FUNCTION__,__LINE__);
		YsJoyReaderSaveJoystickCalibrationInfo(nJoystick,joystick);
printf("%s %d\n",__FUNCTION__,__LINE__);
	}
}

#ifdef GLX
void FsCalibrationDialog::OnDropListSelChange(FsGuiDropList *drp,int prevSel)
{
	if(drp==joyId)
	{
		int sel;
		sel=drp->GetSelection();
		if(0<=sel && sel<nJoystick)
		{
			InitialSetup();
		}
	}
	else if(drp==povXAxis)
	{
		int sel;
		sel=joyId->GetSelection();
		if(0<=sel && sel<nJoystick)
		{
			joystick[sel].povXAxis=povXAxis->GetSelection()-1;
			YsJoyReaderSaveJoystickCalibrationInfo(nJoystick,joystick);
		}
	}
	else if(drp==povYAxis)
	{
		int sel;
		sel=joyId->GetSelection();
		if(0<=sel && sel<nJoystick)
		{
			joystick[sel].povYAxis=povYAxis->GetSelection()-1;
			YsJoyReaderSaveJoystickCalibrationInfo(nJoystick,joystick);
		}
	}
}
#endif

/* virtual */ void FsCalibrationDialog::Show(const FsGuiDialog *excludeFromDrawing) const
{
	FsGuiDialog::Show(excludeFromDrawing);

	auto selJoyId=joyId->GetSelection();
	if(0<=selJoyId && selJoyId<nJoystick)
	{
		int axisType[YsJoyReaderMaxNumAxis];
		joystick[selJoyId].Read();
		SetValueText(joystick[selJoyId]);
		GetAxisType(axisType);
		ShowJoystickAxes(joystick[selJoyId],axisType);
	}
}

void FsCalibrationDialog::ShowJoystickAxes(const YsJoyReader &joy,const int axisType[]) const
{
	int i;
	int x,y;
	int wid,hei;
	FsGetWindowSize(wid,hei);

	x=20;
	y=hei-320;

	for(i=0; i<YsJoyReaderMaxNumAxis; i++)
	{
		if(joy.axis[i].exist==1)
		{
			ShowJoystickAxis(joy,axisType,i,x,y);
			switch(axisType[i])
			{
			case FSJOY_AXISTYPE_STICKORYOKE:
				x+=310;
				i++;
				break;
			case FSJOY_AXISTYPE_QUADRANT:
				x+=40;
				break;
			case FSJOY_AXISTYPE_AXISWITHCENTER:
				x+=40;
				break;
			}
		}
	}

	for(i=0; i<YsJoyReaderMaxNumHatSwitch; i++)
	{
		if(joy.hatSwitch[i].exist==1)
		{
			ShowHatSwitch(joy,i,x,y);
			y+=64;
		}
	}
}

void FsCalibrationDialog::CaptureHat(YsJoyReader &joystick,int hatId,int &hatValueCapture,FsGuiDialog *dlg,FsGuiButton *nextBtn,FsGuiStatic *hatState)
{
	int key,chr;
	int lb,mb,rb,mx,my;

	for(;;)
	{
		joystick.Read();
		hatValueCapture=joystick.hatSwitch[hatId].value;

		FsPollDevice();

		key=FsInkey();
		chr=FsInkeyChar();
		FsGetMouseState(lb,mb,rb,mx,my);
		dlg->SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
		dlg->KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
		dlg->CharIn(chr);

		if(dlg->GetClickedItem()==nextBtn)
		{
			break;
		}

		ClearScreen();

		char str[256];
		sprintf(str,"HAT: %d\n",joystick.hatSwitch[hatId].value);
		hatState->SetText(str);
		dlg->Show();

		FsSwapBuffers();
		FsSleep(10);
	}
}

////////////////////////////////////////////////////////////

void FsGuiCalibrateJoystickAxisDialog::Make(int axis)
{
	phase=FetchFirstPhase(axis);
	Make(axis,phase);
}

void FsGuiCalibrateJoystickAxisDialog::Make(int axis,int phase)
{
	auto &joystick=*joyPtr;
	FsGuiDialog::RemoveAllItem();
	if(0!=joyPtr->axis[axis].exist)
	{
		this->axis=axis;
		this->phase=phase;
		switch(axisType[axis])
		{
		case FSJOY_AXISTYPE_STICKORYOKE:
			switch(phase)
			{
			case PHASE_FIRST_CENTER:
			case PHASE_SECOND_CENTER:
				SetSize(800,600);
				SetTopLeftCorner(0,32);
				AddStaticText(0,0,FSJC_BMP_CALIB_STICKNEUTRAL,40,1,YSTRUE);
				nextBtn=AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
				cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_EXIT,YSTRUE);
				SetTransparency(YSFALSE);
				Fit();
				break;
			case PHASE_LIMIT:
				SetSize(800,600);
				SetTopLeftCorner(0,32);
				AddStaticText(0,0,FSJC_BMP_CALIB_STICKCIRCLE,80,1,YSTRUE);
				nextBtn=AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
				cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_EXIT,YSTRUE);
				SetTransparency(YSFALSE);

				joystick.axis[axis  ].BeginCaptureMinMax();
				joystick.axis[axis+1].BeginCaptureMinMax();

				Fit();
				break;
			}
			break;
		case FSJOY_AXISTYPE_QUADRANT:
			{
				phase=PHASE_NONE; // No phase for Quadrant

				char str[256];
				sprintf(str,"AXIS:%d",axis);

				SetSize(800,600);
				SetTopLeftCorner(0,32);
				AddStaticText(0,0,str,16,1,YSTRUE);
				AddStaticText(0,0,FSJC_BMP_CALIB_AXISCYCLE,80,1,YSTRUE);
				nextBtn=AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
				cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_EXIT,YSTRUE);
				SetTransparency(YSFALSE);
				Fit();

				joystick.Read();
				joystick.axis[axis].CaptureCenter();
				joystick.axis[axis].BeginCaptureMinMax();
			}
			break;
		case FSJOY_AXISTYPE_AXISWITHCENTER:
			{
				char str[256];
				sprintf(str,"AXIS:%d",axis);

				switch(phase)
				{
				case PHASE_FIRST_CENTER:
				case PHASE_SECOND_CENTER:
					SetSize(800,600);
					SetTopLeftCorner(0,32);
					AddStaticText(0,0,str,16,1,YSTRUE);
					AddStaticText(0,0,FSJC_BMP_CALIB_AXISNEUTRAL,40,1,YSTRUE);
					nextBtn=AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
					cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_EXIT,YSTRUE);
					SetTransparency(YSFALSE);
					Fit();
					break;
				case PHASE_LIMIT:
					SetSize(800,600);
					SetTopLeftCorner(0,32);
					AddStaticText(0,0,str,16,1,YSTRUE);
					AddStaticText(0,0,FSJC_BMP_CALIB_AXISCYCLE,80,1,YSTRUE);
					nextBtn=AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
					cancelBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_EXIT,YSTRUE);
					SetTransparency(YSFALSE);

					joystick.axis[axis].BeginCaptureMinMax();

					Fit();
					break;
				}
			}
			break;
		}
	}
}

/* virtual */ void FsGuiCalibrateJoystickAxisDialog::Interval(void)
{
	auto &joystick=*joyPtr;
	if(joystick.axis[axis].exist==1)
	{
		switch(axisType[axis])
		{
		case FSJOY_AXISTYPE_STICKORYOKE:
			switch(phase)
			{
			case PHASE_FIRST_CENTER:
			case PHASE_SECOND_CENTER:
				joystick.Read();
				joystick.axis[axis  ].CaptureCenter();
				joystick.axis[axis+1].CaptureCenter();
				break;
			case PHASE_LIMIT:
				joystick.Read();
				joystick.axis[axis  ].CaptureMinMax();
				joystick.axis[axis+1].CaptureMinMax();
				break;
			}
			break;
		case FSJOY_AXISTYPE_QUADRANT:
			joystick.Read();
			joystick.axis[axis].CaptureMinMax();
			joystick.axis[axis].CenterFromMinMax();
			break;
		case FSJOY_AXISTYPE_AXISWITHCENTER:
			switch(phase)
			{
			case PHASE_FIRST_CENTER:
			case PHASE_SECOND_CENTER:
				joystick.Read();
				joystick.axis[axis].CaptureCenter();
				break;
			case PHASE_LIMIT:
				joystick.Read();
				joystick.axis[axis].CaptureMinMax();
				break;
			}
			break;
		}
	}
	SetNeedRedraw(YSTRUE);
}

int FsGuiCalibrateJoystickAxisDialog::FetchFirstPhase(int axis)
{
	switch(axisType[axis])
	{
	case FSJOY_AXISTYPE_STICKORYOKE:
	case FSJOY_AXISTYPE_AXISWITHCENTER:
		return PHASE_FIRST_CENTER;
	case FSJOY_AXISTYPE_QUADRANT:
		return PHASE_NONE;
	}
	return PHASE_NONE;
}

/* virtual */ void FsGuiCalibrateJoystickAxisDialog::OnButtonClick(FsGuiButton *btn)
{
	if(nextBtn==btn)
	{
		auto prevAxis=axis;
		auto &joystick=*joyPtr;
		if(0==joystick.axis[axis].exist)
		{
			++axis;
		}
		else
		{
			switch(axisType[axis])
			{
			case FSJOY_AXISTYPE_STICKORYOKE:
			case FSJOY_AXISTYPE_AXISWITHCENTER:
				switch(phase)
				{
				case PHASE_FIRST_CENTER:
				case PHASE_LIMIT:
					++phase;
					Make(axis,phase);
					break;
				case PHASE_SECOND_CENTER:
					axis+=2;
					break;
				}
				break;
			case FSJOY_AXISTYPE_QUADRANT:
				++axis;
				break;
			}
		}

		if(prevAxis!=axis)
		{
			while(axis<YsJoyReaderMaxNumAxis)
			{
				if(0!=joystick.axis[axis].exist)
				{
					Make(axis);
					break;
				}
				++axis;
			}
		}
		if(YsJoyReaderMaxNumAxis<=axis)
		{
			CloseModalDialog(YSOK);
		}
	}
	else if(cancelBtn==btn)
	{
		CloseModalDialog(YSERR);
	}
}


//void FsCalibrationDialog::CalibrateJoystick(YsJoyReader &joystick,int axisType[])
//{
//	int axis;
//	int key,chr;
//	int lb,mb,rb,mx,my;
//
//	for(axis=0; axis<YsJoyReaderMaxNumAxis; axis++)
//	{
//		if(joystick.axis[axis].exist==1)
//		{
//			switch(axisType[axis])
//			{
//			case FSJOY_AXISTYPE_STICKORYOKE:
//				{
//					FsGuiDialog centerDlg;
//					FsGuiButton *centerNextBtn;
//					centerDlg.SetSize(800,600);
//					centerDlg.SetTopLeftCorner(0,32);
//					centerDlg.AddStaticText(0,0,FSJC_BMP_CALIB_STICKNEUTRAL,40,1,YSTRUE);
//					centerNextBtn=centerDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
//					centerDlg.SetTransparency(YSTRUE);
//					centerDlg.Fit();
//
//					FsGuiDialog limitDlg;
//					FsGuiButton *limitNextBtn;
//					limitDlg.SetSize(800,600);
//					limitDlg.SetTopLeftCorner(0,32);
//					limitDlg.AddStaticText(0,0,FSJC_BMP_CALIB_STICKCIRCLE,80,1,YSTRUE);
//					limitNextBtn=limitDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
//					limitDlg.SetTransparency(YSTRUE);
//					limitDlg.Fit();
//
//					for(;;)  // First centering
//					{
//						joystick.Read();
//						joystick.axis[axis  ].CaptureCenter();
//						joystick.axis[axis+1].CaptureCenter();
//
//						FsPollDevice();
//
//						key=FsInkey();
//						chr=FsInkeyChar();
//						FsGetMouseState(lb,mb,rb,mx,my);
//						centerDlg.SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
//						centerDlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
//						centerDlg.CharIn(chr);
//
//						if(centerDlg.GetClickedItem()==centerNextBtn)
//						{
//							break;
//						}
//
//						ClearScreen();
//
//						ShowJoystickAxis(joystick,axisType,axis,20,280);
//						centerDlg.Show();
//
//						FsSwapBuffers();
//						FsSleep(10);
//					}
//
//					joystick.axis[axis  ].BeginCaptureMinMax();
//					joystick.axis[axis+1].BeginCaptureMinMax();
//
//					for(;;)
//					{
//						joystick.Read();
//						joystick.axis[axis  ].CaptureMinMax();
//						joystick.axis[axis+1].CaptureMinMax();
//
//						FsPollDevice();
//
//						key=FsInkey();
//						chr=FsInkeyChar();
//						FsGetMouseState(lb,mb,rb,mx,my);
//						limitDlg.SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
//						limitDlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
//						limitDlg.CharIn(chr);
//
//						if(limitDlg.GetClickedItem()==limitNextBtn)
//						{
//							break;
//						}
//
//						ClearScreen();
//
//						ShowJoystickAxis(joystick,axisType,axis,20,280);
//						limitDlg.Show();
//
//						FsSwapBuffers();
//						FsSleep(10);
//					}
//
//					for(;;)  // First centering
//					{
//						joystick.Read();
//						joystick.axis[axis  ].CaptureCenter();
//						joystick.axis[axis+1].CaptureCenter();
//
//						FsPollDevice();
//
//						key=FsInkey();
//						chr=FsInkeyChar();
//						FsGetMouseState(lb,mb,rb,mx,my);
//						centerDlg.SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
//						centerDlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
//						centerDlg.CharIn(chr);
//
//						if(centerDlg.GetClickedItem()==centerNextBtn)
//						{
//							break;
//						}
//
//						ClearScreen();
//
//						ShowJoystickAxis(joystick,axisType,axis,20,280);
//						centerDlg.Show();
//
//						FsSwapBuffers();
//						FsSleep(10);
//					}
//				}
//				axis++;
//				break;
//			case FSJOY_AXISTYPE_QUADRANT:
//				{
//					char str[256];
//					sprintf(str,"AXIS:%d",axis);
//
//					FsGuiDialog limitDlg;
//					FsGuiButton *limitNextBtn;
//					limitDlg.SetSize(800,600);
//					limitDlg.SetTopLeftCorner(0,32);
//					limitDlg.AddStaticText(0,0,str,16,1,YSTRUE);
//					limitDlg.AddStaticText(0,0,FSJC_BMP_CALIB_AXISCYCLE,80,1,YSTRUE);
//					limitNextBtn=limitDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
//					limitDlg.SetTransparency(YSTRUE);
//					limitDlg.Fit();
//
//					joystick.Read();
//					joystick.axis[axis].CaptureCenter();
//					joystick.axis[axis].BeginCaptureMinMax();
//
//					for(;;)
//					{
//						joystick.Read();
//						joystick.axis[axis].CaptureMinMax();
//						joystick.axis[axis].CenterFromMinMax();
//
//						FsPollDevice();
//
//						key=FsInkey();
//						chr=FsInkeyChar();
//						FsGetMouseState(lb,mb,rb,mx,my);
//						limitDlg.SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
//						limitDlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
//						limitDlg.CharIn(chr);
//
//						if(limitDlg.GetClickedItem()==limitNextBtn)
//						{
//							break;
//						}
//
//						ClearScreen();
//
//						ShowJoystickAxis(joystick,axisType,axis,20,280);
//						limitDlg.Show();
//
//						FsSwapBuffers();
//						FsSleep(10);
//					}
//				}
//				break;
//			case FSJOY_AXISTYPE_AXISWITHCENTER:
//				{
//					char str[256];
//					sprintf(str,"AXIS:%d",axis);
//
//					FsGuiDialog centerDlg;
//					FsGuiButton *centerNextBtn;
//					centerDlg.SetSize(800,600);
//					centerDlg.SetTopLeftCorner(0,32);
//					centerDlg.AddStaticText(0,0,str,16,1,YSTRUE);
//					centerDlg.AddStaticText(0,0,FSJC_BMP_CALIB_AXISNEUTRAL,40,1,YSTRUE);
//					centerNextBtn=centerDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
//					centerDlg.SetTransparency(YSTRUE);
//					centerDlg.Fit();
//
//					FsGuiDialog limitDlg;
//					FsGuiButton *limitNextBtn;
//					limitDlg.SetSize(800,600);
//					limitDlg.SetTopLeftCorner(0,32);
//					limitDlg.AddStaticText(0,0,str,16,1,YSTRUE);
//					limitDlg.AddStaticText(0,0,FSJC_BMP_CALIB_AXISCYCLE,80,1,YSTRUE);
//					limitNextBtn=limitDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
//					limitDlg.SetTransparency(YSTRUE);
//					limitDlg.Fit();
//
//					for(;;)
//					{
//						joystick.Read();
//						joystick.axis[axis].CaptureCenter();
//
//						FsPollDevice();
//
//						key=FsInkey();
//						chr=FsInkeyChar();
//						FsGetMouseState(lb,mb,rb,mx,my);
//						centerDlg.SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
//						centerDlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
//						centerDlg.CharIn(chr);
//
//						if(centerDlg.GetClickedItem()==centerNextBtn)
//						{
//							break;
//						}
//
//						ClearScreen();
//
//						ShowJoystickAxis(joystick,axisType,axis,20,280);
//						centerDlg.Show();
//
//						FsSwapBuffers();
//						FsSleep(10);
//					}
//
//					joystick.axis[axis].BeginCaptureMinMax();
//
//					for(;;)
//					{
//						joystick.Read();
//						joystick.axis[axis].CaptureMinMax();
//
//						FsPollDevice();
//
//						key=FsInkey();
//						chr=FsInkeyChar();
//						FsGetMouseState(lb,mb,rb,mx,my);
//						limitDlg.SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
//						limitDlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
//						limitDlg.CharIn(chr);
//
//						if(limitDlg.GetClickedItem()==limitNextBtn)
//						{
//							break;
//						}
//
//						ClearScreen();
//
//						ShowJoystickAxis(joystick,axisType,axis,20,280);
//						limitDlg.Show();
//
//						FsSwapBuffers();
//						FsSleep(10);
//					}
//
//					for(;;)
//					{
//						joystick.Read();
//						joystick.axis[axis].CaptureCenter();
//
//						FsPollDevice();
//
//						key=FsInkey();
//						chr=FsInkeyChar();
//						FsGetMouseState(lb,mb,rb,mx,my);
//						centerDlg.SetMouseState((YSBOOL)lb,(YSBOOL)mb,(YSBOOL)rb,mx,my);
//						centerDlg.KeyIn(key,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
//						centerDlg.CharIn(chr);
//
//						if(centerDlg.GetClickedItem()==centerNextBtn)
//						{
//							break;
//						}
//
//						ClearScreen();
//
//						ShowJoystickAxis(joystick,axisType,axis,20,280);
//						centerDlg.Show();
//
//						FsSwapBuffers();
//						FsSleep(10);
//					}
//				}
//				break;
//			}
//		}
//	}
//}

void FsCalibrationDialog::CalibrateHatSwitch(YsJoyReader &joystick)
{
	int hatId;
	for(hatId=0; hatId<YsJoyReaderMaxNumHatSwitch; hatId++)
	{
		if(0!=joystick.hatSwitch[hatId].exist)
		{
			char str[256];
			sprintf(str,"HatSwitch: %d\n",hatId);

			FsGuiDialog hatNeutDlg;
			FsGuiStatic *hatNeutText;
			FsGuiStatic *hatNeutState;
			FsGuiButton *hatNeutNextBtn;
			hatNeutDlg.SetSize(800,600);
			hatNeutDlg.SetTopLeftCorner(0,32);
			hatNeutDlg.AddStaticText(0,0,str,16,1,YSTRUE);
			hatNeutText=hatNeutDlg.AddStaticText(0,0,FSJC_BMP_CALIB_HATNEUTRAL,60,1,YSTRUE);
			hatNeutState=hatNeutDlg.AddStaticText(0,0,"HAT:---",16,1,YSTRUE);
			hatNeutNextBtn=hatNeutDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
			hatNeutDlg.SetTransparency(YSTRUE);
			hatNeutDlg.Fit();

			FsGuiDialog hat0DegDlg;
			FsGuiStatic *hat0DegText;
			FsGuiStatic *hat0DegState;
			FsGuiButton *hat0DegNextBtn;
			hat0DegDlg.SetSize(800,600);
			hat0DegDlg.SetTopLeftCorner(0,32);
			hat0DegDlg.AddStaticText(0,0,str,16,1,YSTRUE);
			hat0DegText=hat0DegDlg.AddStaticText(0,0,FSJC_BMP_CALIB_HAT0DEG,60,1,YSTRUE);
			hat0DegState=hat0DegDlg.AddStaticText(0,0,"HAT:---",16,1,YSTRUE);
			hat0DegNextBtn=hat0DegDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
			hat0DegDlg.SetTransparency(YSTRUE);
			hat0DegDlg.Fit();

			FsGuiDialog hat90DegDlg;
			FsGuiStatic *hat90DegText;
			FsGuiStatic *hat90DegState;
			FsGuiButton *hat90DegNextBtn;
			hat90DegDlg.SetSize(800,600);
			hat90DegDlg.SetTopLeftCorner(0,32);
			hat90DegDlg.AddStaticText(0,0,str,16,1,YSTRUE);
			hat90DegText=hat90DegDlg.AddStaticText(0,0,FSJC_BMP_CALIB_HAT90DEG,60,1,YSTRUE);
			hat90DegState=hat90DegDlg.AddStaticText(0,0,"HAT:---",16,1,YSTRUE);
			hat90DegNextBtn=hat90DegDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
			hat90DegDlg.SetTransparency(YSTRUE);
			hat90DegDlg.Fit();

			FsGuiDialog hat180DegDlg;
			FsGuiStatic *hat180DegText;
			FsGuiStatic *hat180DegState;
			FsGuiButton *hat180DegNextBtn;
			hat180DegDlg.SetSize(800,600);
			hat180DegDlg.SetTopLeftCorner(0,32);
			hat180DegDlg.AddStaticText(0,0,str,16,1,YSTRUE);
			hat180DegText=hat180DegDlg.AddStaticText(0,0,FSJC_BMP_CALIB_HAT180DEG,60,1,YSTRUE);
			hat180DegState=hat180DegDlg.AddStaticText(0,0,"HAT:---",16,1,YSTRUE);
			hat180DegNextBtn=hat180DegDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
			hat180DegDlg.SetTransparency(YSTRUE);
			hat180DegDlg.Fit();

			FsGuiDialog hat270DegDlg;
			FsGuiStatic *hat270DegText;
			FsGuiStatic *hat270DegState;
			FsGuiButton *hat270DegNextBtn;
			hat270DegDlg.SetSize(800,600);
			hat270DegDlg.SetTopLeftCorner(0,32);
			hat270DegDlg.AddStaticText(0,0,str,16,1,YSTRUE);
			hat270DegText=hat270DegDlg.AddStaticText(0,0,FSJC_BMP_CALIB_HAT270DEG,60,1,YSTRUE);
			hat270DegState=hat270DegDlg.AddStaticText(0,0,"HAT:---",16,1,YSTRUE);
			hat270DegNextBtn=hat270DegDlg.AddTextButton(0,FSKEY_SPACE,FSGUI_PUSHBUTTON,FSJC_BMP_CALIB_NEXT,YSTRUE);
			hat270DegDlg.SetTransparency(YSTRUE);
			hat270DegDlg.Fit();

			CaptureHat(joystick,hatId,joystick.hatSwitch[hatId].valueNeutral,&hatNeutDlg,hatNeutNextBtn,hatNeutState);
			CaptureHat(joystick,hatId,joystick.hatSwitch[hatId].value0Deg,&hat0DegDlg,hat0DegNextBtn,hat0DegState);
			CaptureHat(joystick,hatId,joystick.hatSwitch[hatId].value90Deg,&hat90DegDlg,hat90DegNextBtn,hat90DegState);
			CaptureHat(joystick,hatId,joystick.hatSwitch[hatId].value180Deg,&hat180DegDlg,hat180DegNextBtn,hat180DegState);
			CaptureHat(joystick,hatId,joystick.hatSwitch[hatId].value270Deg,&hat270DegDlg,hat270DegNextBtn,hat270DegState);
		}
	}
}


