#include <memory>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <fsgui.h>
#include <fsguifiledialog.h>

#include "graphics/common/fsopengl.h"

#include "fsconfig.h"
#include "fsoption.h"
#include "fsapplyoption.h"
#include "fsnetconfig.h"

#include "fsworld.h"
#include "fspluginmgr.h"
#include "fsfilename.h"
#include "graphics/common/fsfontrenderer.h"

#include "fstextresource.h"

#include "fsdialog.h"

#include "fsautopilot.h" // Needed for FSACRO_*

#include "fsexistence.h"
#include "fscontrol.h"

#include "fsrunloop.h"

#include "fssimextension_endurance.h"
#include "fssimextension_intercept.h"
#include "fssimextension_closeairsupport.h"
#include "fssimextension_groundtoair.h"


extern FsScreenMessage fsConsole;

// #define CRASHINVESTIGATION

////////////////////////////////////////////////////////////

FsShowLandingPracticeInfo::FsShowLandingPracticeInfo()
{
}

FsShowLandingPracticeInfo::~FsShowLandingPracticeInfo()
{
}

void FsShowLandingPracticeInfo::CleanUp(void)
{
	leftTraffic=YSTRUE;
	leg=FSLEG_BASE;
	state=STATE_RUNNING;
	wind.Set(0.0,0.0);
	lowCloud=YSFALSE;
	lowVisibility=YSFALSE;

	path.CleanUp();;
	pathPtr=0;
	windPtr=0;
}

void FsShowLandingPracticeInfo::SetUp(YSBOOL leftTraffic,FSTRAFFICPATTERNLEG leg,const YsVec3 &wind,YSBOOL lowCloud,YSBOOL lowVisibility)
{
	CleanUp();

	state=STATE_RUNNING;

	this->leftTraffic=leftTraffic;
	this->leg=leg;
	this->wind.GetXY(wind);
	this->wind.Normalize();
	this->lowCloud=lowCloud;
	this->lowVisibility=lowVisibility;


	YsVec2 p2d,prevP2d;
	for(int i=0; i<20; i++)
	{
		p2d.Set(0.0,(double)i);
		path.Append(p2d);
		prevP2d=p2d;
	}
	switch(leg)
	{
	default:
		break;
	case FSLEG_FINAL:
		for(int i=1; i<20; i++)
		{
			p2d.Set(0.0,(double)i);
			p2d+=prevP2d;
			path.Append(p2d);
		}
		break;
	case FSLEG_DOG:
		for(int i=0; i<=5; i++)
		{
			double a;
			a=(YsPi/6.0)*(double)i/5.0;
			p2d.Set(sin(a),cos(a));
			p2d+=prevP2d;
			path.Append(p2d);
			prevP2d=p2d;
		}
		for(int i=0; i<15; i++)
		{
			p2d.Set(0.5,1.7320508/2.0);
			p2d+=prevP2d;
			path.Append(p2d);
			prevP2d=p2d;
		}
		break;
	case FSLEG_BASE:
		for(int i=0; i<=10; i++)
		{
			double a;
			a=(YsPi/2.0)*(double)i/10.0;
			p2d.Set(sin(a),cos(a));
			p2d+=prevP2d;
			path.Append(p2d);
			prevP2d=p2d;
		}
		for(int i=0; i<15; i++)
		{
			p2d.Set(1.0,0.0);
			p2d+=prevP2d;
			path.Append(p2d);
			prevP2d=p2d;
		}
		break;
	}

	pathPtr=path.GetN()-1;
	windPtr=0;


	int mx,my;
	YSBOOL lb,mb,rb;

	lb=YSTRUE;
	mb=YSTRUE;
	rb=YSTRUE;
	while(FsInkey()!=FSKEY_NULL || FsInkeyChar()!=0 || FSMOUSEEVENT_NONE!=FsGetMouseEvent(lb,mb,rb,mx,my) || lb==YSTRUE || rb==YSTRUE || mb==YSTRUE)
	{
		// 2010/12/04 lb,mb,rb must be checked after FsGetMouseEvent.  Otherwise, the test
		//            doesn't reach FSMOUSEEVENT_NONE!=FsGetMouseEvent, and causes infinite loop.
		FsPollDevice();
	}
}

void FsShowLandingPracticeInfo::RunOneStep(void)
{
	int i,j,mx,my;
	YSBOOL lb,mb,rb;
	YSBOOL joyTrigger;
	FsJoystick joy;

	const int key=FsInkey();

	FsGetMouseEvent(lb,mb,rb,mx,my);

	pathPtr--;
	if(pathPtr<1)
	{
		pathPtr=path.GetN()-1;
	}

	if(wind!=YsVec2::Origin())
	{
		windPtr+=3;
		if(windPtr>=40)
		{
			windPtr=-40;
		}
	}

	joyTrigger=YSFALSE;
	for(i=0; i<FsMaxNumJoystick; i++)
	{
		FsPollJoystick(joy,i);
		for(j=0; j<FsMaxNumJoyTrig; j++)
		{
			if(joy.trg[j]==YSTRUE)
			{
				joyTrigger=YSTRUE;
			}
		}
	}
	if(joyTrigger==YSTRUE || key==FSKEY_SPACE || key==FSKEY_ESC || lb==YSTRUE || rb==YSTRUE)
	{
		state=STATE_OVER;
	}
}

void FsShowLandingPracticeInfo::Draw(void) const
{
	int wid,hei,sx,sy;

	FsGetWindowSize(wid,hei);

	sx=80;
	sy=100;

	FsClearScreenAndZBuffer(YsBlack());
	FsSet2DDrawing();


	FsDrawRect(32,32,wid-31,hei-31,YsYellow(),YSFALSE);
	FsDrawRect(24,24,wid-23,hei-23,YsYellow(),YSFALSE);


	FsDrawString(sx,sy,"<< LANDING PRACTICE >>",YsGreen());
	sy+=16;

	switch(leg)
	{
	default:
		break;
	case FSLEG_FINAL:
		FsDrawString(sx,sy,"START ON FINAL",YsWhite());
		break;
	case FSLEG_DOG:
		if(leftTraffic==YSTRUE)
		{
			FsDrawString(sx,sy,"START ON LEFT DOG-LEG",YsWhite());
		}
		else
		{
			FsDrawString(sx,sy,"START ON RIGHT DOG-LEG",YsWhite());
		}
		break;
	case FSLEG_BASE:
		if(leftTraffic==YSTRUE)
		{
			FsDrawString(sx,sy,"START ON LEFT BASE",YsWhite());
		}
		else
		{
			FsDrawString(sx,sy,"START ON RIGHT BASE",YsWhite());
		}
		break;
	}
	sy+=20;

	if(lowCloud==YSTRUE)
	{
		FsDrawString(sx,sy,"CEILING 250FT OVERCAST",YsYellow());
		sy+=16;
	}
	if(lowVisibility==YSTRUE)
	{
		FsDrawString(sx,sy,"VISIBILITY 0.5 MILE",YsYellow());
		sy+=16;
	}
	if(lowCloud==YSTRUE || lowVisibility==YSTRUE)
	{
		FsDrawString(sx,sy,"ILS APPROACH",YsGreen());
		sy+=20;
	}
	else
	{
		FsDrawString(sx,sy,"VISUAL APPROACH",YsGreen());
		sy+=20;
	}


	FsDrawString(sx,sy,"CHECK GEAR DOWN!",YsWhite());
	sy+=16;



	int rwCx,rwCy,rwLx,rwLy;
	double unit;
	if(leftTraffic==YSTRUE)
	{
		rwCx=wid*4/5;
	}
	else
	{
		rwCx=wid/2;
	}
	rwCy=hei*2/10;
	rwLx=wid/80;
	rwLy=hei/10;
	unit=(double)rwLy/5.0;

	FsDrawRect(rwCx-rwLx,rwCy-rwLy,rwCx+rwLx,rwCy+rwLy,YsWhite(),YSFALSE);
	for(auto i : path.AllIndex())
	{
		double u,v;
		int x,y;
		u=path[i].x();
		v=path[i].y();
		if(leftTraffic==YSTRUE)
		{
			x=rwCx-(int)(u*unit);
		}
		else
		{
			x=rwCx+(int)(u*unit);
		}
		y=rwCy+(int)(v*unit);
		FsDrawPoint2Pix(x,y,YsWhite());
	}

	if(YSTRUE==path.IsInRange(pathPtr))
	{
		int tri[6];
		YsVec2 p,p1,p2,v,w;
		p1=path[pathPtr-1];
		p2=path[pathPtr];
		if(leftTraffic==YSTRUE)
		{
			p1.MulX(-1.0);
			p2.MulX(-1.0);
		}
		v=(p1-p2)*0.5;
		w.Set(-v.y(),v.x());

		p=p2+v;
		tri[0]=rwCx+(int)(p.x()*unit);
		tri[1]=rwCy+(int)(p.y()*unit);
		p=p2-v+w;
		tri[2]=rwCx+(int)(p.x()*unit);
		tri[3]=rwCy+(int)(p.y()*unit);
		p=p2-v-w;
		tri[4]=rwCx+(int)(p.x()*unit);
		tri[5]=rwCy+(int)(p.y()*unit);

		FsDrawPolygon(3,tri,YsGreen());
	}


	if(wind!=YsVec2::Origin())
	{
		for(int i=1; i<=3; i++)
		{
			YsVec2 c,p1,p2,v,p3;
			int cx,cy;
			cx=rwCx;
			cy=rwCy-rwLy+(rwLy*2*i)/4;

			c.Set(cx,cy);

			v=wind;
			v.Rotate(YsPi/2.0);

			p1=c-wind*(double)windPtr;
			p2=c-wind*(double)windPtr-wind*20.0;
			FsDrawLine((int)p1.x(),(int)p1.y(),(int)p2.x(),(int)p2.y(),YsWhite());

			p3=p2+(wind-v)*4.0;
			FsDrawLine((int)p3.x(),(int)p3.y(),(int)p2.x(),(int)p2.y(),YsWhite());

			p3=p2+(wind+v)*4.0;
			FsDrawLine((int)p3.x(),(int)p3.y(),(int)p2.x(),(int)p2.y(),YsWhite());
		}
	}


	FsSwapBuffers();
}

////////////////////////////////////////////////////////////

FsRunLoop::FsRunLoop()
{
	RealInitialize();
	Initialize();
}

FsRunLoop::FsRunLoop(StepByStepInitializationOption opt)
{
	RealInitialize();
}

FsRunLoop::FsRunLoop(FsWorld::InitializationOption worldOpt)
{
	RealInitialize();
	Initialize(worldOpt);
}

void FsRunLoop::RealInitialize(void)
{
	initializationCounter=0;

	netcfgPtr=nullptr;

	terminate=YSFALSE;
	terminateWhenNoModalDialogIsOpenAndBackToMenu=YSFALSE;
	autoDemoForever=YSFALSE;

	needRedraw=YSTRUE;
	needReloadOption=YSFALSE;

	world=NULL;
	opt=NULL;

	mainCanvas=NULL;
	showLandingPracticeInfo=NULL;

	demoModeInfo=NULL;

	clientModeRunLoop=NULL;
	serverModeRunLoop=NULL;

}

FsRunLoop::~FsRunLoop()
{
	Free();
}

/* static */ const char *FsRunLoop::RunModeToString(RUNMODE runMode)
{
	switch(runMode)
	{
	default:
		break;
	case YSRUNMODE_MENU:
		return "YSRUNMODE_MENU";
	case YSRUNMODE_SHOWLANDINGPRACTICEINFO:
		return "YSRUNMODE_SHOWLANDINGPRACTICEINFO";
	case YSRUNMODE_FLY_REGULAR:
		return "YSRUNMODE_FLY_REGULAR";
	case YSRUNMODE_FLY_DEMOMODE:
		return "YSRUNMODE_FLY_DEMOMODE";
	case YSRUNMODE_REPLAYRECORD:
		return "YSRUNMODE_REPLAYRECORD";
	case YSRUNMODE_FLY_CLIENTMODE:
		return "YSRUNMODE_FLY_CLIENTMODE";
	case YSRUNMODE_FLY_SERVERMODE:
		return "YSRUNMODE_FLY_SERVERMODE";
	}
	return "YSRUNMODE_?";
}

void FsRunLoop::Free(void)
{
printf("%s %d\n",__FUNCTION__,__LINE__);
	if(NULL!=clientModeRunLoop)
	{
		delete clientModeRunLoop;
		clientModeRunLoop=NULL;
	}
printf("%s %d\n",__FUNCTION__,__LINE__);
	if(NULL!=serverModeRunLoop)
	{
		delete serverModeRunLoop;
		serverModeRunLoop=NULL;
	}
printf("%s %d\n",__FUNCTION__,__LINE__);
	if(NULL!=demoModeInfo)
	{
		delete demoModeInfo;
		demoModeInfo=NULL;
	}
printf("%s %d\n",__FUNCTION__,__LINE__);
	if(NULL!=showLandingPracticeInfo)
	{
		delete showLandingPracticeInfo;
		showLandingPracticeInfo=NULL;
	}
printf("%s %d\n",__FUNCTION__,__LINE__);
	if(NULL!=world)
	{
		delete world;
		world=NULL;
	}
printf("%s %d\n",__FUNCTION__,__LINE__);
	if(NULL!=opt)
	{
		delete opt;
		opt=NULL;
	}
printf("%s %d\n",__FUNCTION__,__LINE__);

	if(nullptr!=netcfgPtr)
	{
		delete netcfgPtr;
		netcfgPtr=nullptr;
	}
}

FsRunLoop::RUNMODE FsRunLoop::GetCurrentRunMode(void) const
{
	if(0<runModeStack.GetN())
	{
		return runModeStack.Last().runMode;
	}
	return YSRUNMODE_NONE;
}

void FsRunLoop::SetCommandParameter(FsCommandParameter fscp)
{
	this->fscp=fscp;
}

const FsCommandParameter &FsRunLoop::GetCommandParameter(void) const
{
	return fscp;
}

void FsRunLoop::SetAutoExit(YSBOOL autoExit)
{
	terminateWhenNoModalDialogIsOpenAndBackToMenu=autoExit;
}

void FsRunLoop::SetAutoDemoForever(YSBOOL autoDemoForever) // 2013/11/27 For cracking down the freeze problem.
{
	this->autoDemoForever=autoDemoForever;
}

void FsRunLoop::SetTerminateFlag(YSBOOL term)
{
	this->terminate=term;
}

void FsRunLoop::SetNeedReloadOption(YSBOOL nrlo)
{
	this->needReloadOption=nrlo;
}

void FsRunLoop::SetMainCanvas(GuiCanvasBase *canvasPtr) // <- Later should be made GuiCanvasBase *
{
	this->mainCanvas=canvasPtr;
	canvasPtr->runLoop=this;
}

class FsRunLoop::GuiCanvasBase *FsRunLoop::GetCanvas(void)
{
	return mainCanvas;
}

void FsRunLoop::AttachGuiMessageBox(const YsWString &title,const YsWString &msg,const YsWString &okBtn)
{
	if(nullptr!=mainCanvas)
	{
		auto dlg=FsGuiDialog::CreateSelfDestructiveDialog<FsGuiMessageBoxDialog>();
		dlg->Make(title,msg,okBtn,nullptr);
		mainCanvas->AttachModalDialog(dlg);
	}
}

class FsWorld *FsRunLoop::GetWorld(void)
{
	return world;
}

const class FsWorld *FsRunLoop::GetWorld(void) const
{
	return world;
}

void FsRunLoop::Initialize(void)
{
	FsWorld::InitializationOption worldOpt;
	Initialize(worldOpt);
}

void FsRunLoop::Initialize(FsWorld::InitializationOption worldOpt)
{
	while(YSTRUE!=InitializeOneStep(worldOpt))
	{
	}
}

YSBOOL FsRunLoop::InitializeOneStep(FsWorld::InitializationOption worldOpt)
{
	switch(initializationCounter)
	{
	case 0:
		ChangeRunMode(YSRUNMODE_MENU);
		Free();
		world=new FsWorld;
		world->LoadAirplaneTemplate(worldOpt);
		break;
	case 1:
		world->LoadGroundTemplate(worldOpt);
		break;
	case 2:
		world->LoadFieldTemplate(worldOpt);
		break;
	case 3:
		FsPlugInCallInitialize(world);

		opt=new FsOption;
		opt->Load(FsGetOptionFile());

		netcfgPtr=new FsNetConfig;

		if(0==strcmp(FsOption::GetLanguageString(),FsJapaneseLanguageCode))
		{
			newFltMsgBmp.LoadPng("misc/newfltmsg.png");
			simFlyMsgBmp.LoadPng("misc/simflymsg.png");
			simRepMsgBmp.LoadPng("misc/simrepmsg.png");
		}
		break;
	case 4:
		nTitleBmp=0;
		for(;;)
		{
			YsString fn;
			fn.Printf("misc/title%02d.png",nTitleBmp);
			if(YsFileExist(fn)!=YSTRUE)
			{
				break;
			}
			else
			{
				nTitleBmp++;
			}
		}

		FsGuiObject::defActiveBgCol.SetDoubleRGB(0.0,0.0,0.5);
		FsGuiObject::defActiveFgCol.SetDoubleRGB(1.0,1.0,1.0);

		lastChangePicture=0;
		titleBmpPtr=NULL;
		break;
	default:
		return YSTRUE;
	}
	++initializationCounter;
	return YSFALSE;
}

void FsRunLoop::StartShowLandingPracticeInfoMode( // Next run mode is always fixed.
    YSBOOL leftTraffic,FSTRAFFICPATTERNLEG leg,const YsVec3 &wind,YSBOOL lowCloud,YSBOOL lowVisibility)
{
	if(NULL==showLandingPracticeInfo)
	{
		showLandingPracticeInfo=new FsShowLandingPracticeInfo;
	}

	ChangeRunMode(YSRUNMODE_SHOWLANDINGPRACTICEINFO);
	showLandingPracticeInfo->SetUp(leftTraffic,leg,wind,lowCloud,lowVisibility);
}

void FsRunLoop::StartReplayRecord(YSBOOL editMode)
{
	replayInfo.Initialize(world->GetSimulation()->GetFirstRecordTime(),editMode);
	simState=FsSimulation::FSSIMSTATE_INITIALIZE;
	ChangeRunMode(YSRUNMODE_REPLAYRECORD);
}

void FsRunLoop::StartOpeningDemo(void)
{
	int demoType=rand()&1;  // 0:Dogfight  1:Acro  2:Landing
	// Currently landing demo is excluded from Opening demo because it is too long.

	switch(demoType)
	{
	case 0:
		StartDogFightDemo();
		break;
	case 1:
		{
			const char *const acroPlane[]=
			{
				"F-16C_THUNDERBIRDS",
				"F-18C_BLUEANGELS",
				"T-4_BLUEIMPULSE",
				"T-2_BLUEIMPULSE",
				"HAWK_REDARROWS",
				"KFIR",
				NULL
			};
			const int tbAcroType[]=
			{
				FSACRO_SLOWROLL,
				FSACRO_DELTALOOP,
				FSACRO_DELTAROLL,
				FSACRO_TIGHTTURN,
				FSACRO_BOMBBURST4SHIP,
				FSACRO_CHANGEOVERTURN,
				FSACRO_TRAILTODIAMONDROLL,
				FSACRO_CUBANEIGHT,
				FSACRO_BOMBBURSTDOWN6SHIP,
				FSACRO_DIAMONDTAKEOFF,
				FSACRO_CONTINUOUSROLL,
				FSACRO_TACKCROSSANDVERTICALCLIMBROLL,
				FSACRO_PITCHUPBREAK,
				FSACRO_LEVELOPENER,
				FSACRO_LINEABREASTLOOP,
				-1
			};
			const int baAcroType[]=
			{
				FSACRO_SLOWROLL,
				FSACRO_DELTALOOP,
				FSACRO_DELTAROLL,
				FSACRO_TIGHTTURN,
				FSACRO_TRAILTODIAMONDROLL,
				FSACRO_CUBANEIGHT,
				FSACRO_BOMBBURSTDOWN6SHIP,
				FSACRO_DIAMONDTAKEOFF,
				FSACRO_CONTINUOUSROLL,
				FSACRO_TACKCROSSANDVERTICALCLIMBROLL,
				FSACRO_LEVELBREAK,
				FSACRO_PITCHUPBREAK,
				FSACRO_BOMBBURST6SHIP,
				FSACRO_LEVELOPENER,
				FSACRO_LINEABREASTLOOP,
				FSACRO_DOUBLEFARVEL,
				-1
			};
			const int t4AcroType[]=
			{
				FSACRO_CORKSCREW,
				FSACRO_SLOWROLL,
				FSACRO_DELTALOOP,
				FSACRO_DELTAROLL,
				FSACRO_TIGHTTURN,
				FSACRO_360ANDLOOP,
				FSACRO_CHANGEOVERTURN,
				FSACRO_TRAILTODIAMONDROLL,
				FSACRO_CUBANEIGHT,
				FSACRO_DELTALOOPANDBONTON,
				FSACRO_ROLLINGCOMBATPITCH,
				FSACRO_DIAMONDTAKEOFF,
				FSACRO_CONTINUOUSROLL,
				FSACRO_ROLLONTAKEOFFANDHALFCUBAN,
				FSACRO_TACKCROSSANDVERTICALCLIMBROLL,
				FSACRO_RAINFALL,
				FSACRO_LETTEREIGHT,
				FSACRO_STARCROSS,
				FSACRO_LEVELOPENER,
				FSACRO_LINEABREASTLOOP,
				FSACRO_LINEABREASTROLL,
				FSACRO_DOUBLEFARVEL,
				-1
			};
			const int t2AcroType[]=
			{
				FSACRO_SLOWROLL,
				FSACRO_DELTALOOP,
				FSACRO_DELTAROLL,
				FSACRO_TIGHTTURN,
				FSACRO_BOMBBURST4SHIP,
				FSACRO_CHANGEOVERTURN,
				FSACRO_TRAILTODIAMONDROLL,
				FSACRO_CUBANEIGHT,
				FSACRO_ROLLINGCOMBATPITCH,
				FSACRO_DIAMONDTAKEOFF,
				FSACRO_CONTINUOUSROLL,
				FSACRO_LEVELOPENER,
				FSACRO_LINEABREASTLOOP,
				-1
			};
			const int redArwAcroType[]=
			{
				FSACRO_CORKSCREW,
				FSACRO_DELTALOOP,
				FSACRO_DELTAROLL,
				FSACRO_BOMBBURSTDOWN6SHIP,
				FSACRO_CONTINUOUSROLL,
				FSACRO_BIGHEART,
				FSACRO_BIGHEART,
				FSACRO_DIAMOND9TOSWANBEND,
				FSACRO_SWANTOAPOLLOROLL,
				FSACRO_LANCASTERTO5_4SPLIT,
				FSACRO_CHAMPAIGNSPLIT,
				FSACRO_VIXENBREAK,
				FSACRO_BIGBATTLETOSHORTDIAMONDLOOP,
				-1
			};

			int airId,acroType=FSACRO_ROLLINGCOMBATPITCH;
			int n,r,i;

			airId=rand()%6;
			r=rand()&63;
			n=0;
			const char *field="AOMORI";
			switch(airId)
			{
			case 0:
				for(i=0; i<r; i++)
				{
					n++;
					if(tbAcroType[n]<0)
					{
						n=0;
					}
				}
				acroType=tbAcroType[n];
				break;
			case 1:
				for(i=0; i<r; i++)
				{
					n++;
					if(baAcroType[n]<0)
					{
						n=0;
					}
				}
				acroType=baAcroType[n];
				break;
			case 2:
				for(i=0; i<r; i++)
				{
					n++;
					if(t4AcroType[n]<0)
					{
						n=0;
					}
				}
				acroType=t4AcroType[n];
				break;
			case 3:
				for(i=0; i<r; i++)
				{
					n++;
					if(t2AcroType[n]<0)
					{
						n=0;
					}
				}
				acroType=t2AcroType[n];
				break;
			case 4:
				field="HEATHROW";
				for(i=0; i<r; i++)
				{
					n++;
					if(redArwAcroType[n]<0)
					{
						n=0;
					}
				}
				acroType=redArwAcroType[n];
				break;
			case 5:
				acroType=FSACRO_STAROFDAVID;
				break;
			}
			printf("%s %d\n",__FUNCTION__,__LINE__);
			printf("Field: %s\n",field);
			printf("Airplane: %s\n",acroPlane[airId]);
			printf("Acro: %d\n",acroType);
			StartAirShowDemo(field,acroPlane[airId],acroType);
		}
		break;
	case 2:
		{
			int ldgType=rand()&1;  // 0:Normal landing  1:Carrier landing

			const char *const normalCandidate[]=
			{
				"F-16C_THUNDERBIRDS",
				"F-18C_BLUEANGELS",
				"T-4_BLUEIMPULSE",
				"F-4EJ_PHANTOM",
				"F-16C_FIGHTINGFALCON",
				"F-18C_HORNET",
				"F-22_RAPTOR",
				"F-15J_EAGLE",
				"F-16C_FIGHTINGFALCON",
				"F-18C_HORNET",
				NULL
			};

			const char *const carrierCandidate[]=
			{
				"A-4_SKYHAWK",
				"A-6_INTRUDER",
				"EA-6B_PROWLER",
				"E-2C_HAWKEYE",
				"F-4E_PHANTOM",
				"F-14_TOMCAT",
				"F-18C_HORNET",
				"S-3_VIKING",
				NULL
			};

			int i,r,n;
			r=rand()&63;
			n=0;
			switch(ldgType)
			{
			case 0:
				for(i=0; i<r; i++)
				{
					n++;
					if(normalCandidate[n]==NULL)
					{
						n=0;
					}
				}
				StartLandingDemo("AOMORI",normalCandidate[n]);
				break;
			case 1:
				for(i=0; i<r; i++)
				{
					n++;
					if(carrierCandidate[n]==NULL)
					{
						n=0;
					}
				}
				StartCarrierLandingDemo("AOMORI",carrierCandidate[n]);
				break;
			}
		}
		break;
	}

}

void FsRunLoop::StartLandingDemo(const char fldName[],const char airName[])
{
	if(YSOK==SetUpLandingDemo(fldName,airName))
	{
		demoModeRecordFlight=YSFALSE;
		demoModeDrawSmoke=YSFALSE;
		StartDemoModeInGeneral(FSDEMO_LANDING,"Press key to exit demonstration.",0.0);
	}
}

YSRESULT FsRunLoop::SetUpLandingDemo(const char fldName[],const char airName[])
{
	world->TerminateSimulation();
	world->PrepareSimulation();


	world->AddField(NULL,fldName,YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0),YSTRUE,YSFALSE);
	world->DisableGroundFire();


	FsAirplane *air=world->AddAirplane(airName,YSTRUE);
	air->iff=FS_IFF0;


	double bankLimitOverride;
	bankLimitOverride=YsPi/2.0;
	// if(formationLanding==YSTRUE)
	{
		bankLimitOverride=YsPi/6.0;
	}


	if(world->SettleAirplaneForLandingDemo(*air,2300.0,bankLimitOverride)==YSOK)
	{
		FsLandingAutopilot *ap;
		ap=(FsLandingAutopilot *)air->GetAutopilot();
		ap->autoClearRunway=YSFALSE;


		if(air->Prop().GetAircraftClass()==FSAC_AEROBATIC ||
		   air->Prop().GetAircraftClass()==FSAC_FIGHTER ||
		   air->Prop().GetAircraftClass()==FSAC_ATTACKER ||
		   air->Prop().GetAircraftClass()==FSAC_TRAINER ||
		   air->Prop().GetAircraftClass()==FSAC_WW2FIGHTER ||
		   air->Prop().GetAircraftClass()==FSAC_WW2ATTACKER ||
		   air->Prop().GetAircraftClass()==FSAC_WW2DIVEBOMBER)
		{
			YsVec3 bbx[2],ofst[2],wmPos[2];

			double dx,dz;
			air->UntransformedCollisionShell().GetBoundingBox(bbx[0],bbx[1]);
			dx=bbx[1].x()-bbx[0].x();
			dz=bbx[1].z()-bbx[0].z();
			ofst[0].Set( dx*1.1,-dz*0.1,-dz*0.9);
			ofst[1].Set(-dx*1.1,-dz*0.1,-dz*0.9);

			YsVec3 parPos;
			YsAtt3 parAtt;
			YsVec3 parVel;
			parPos=air->GetPosition();
			parAtt=air->GetAttitude();
			air->Prop().GetVelocity(parVel);

			parAtt.Mul(wmPos[0],ofst[0]);
			wmPos[0]+=parPos;
			parAtt.Mul(wmPos[1],ofst[1]);
			wmPos[1]+=parPos;

			int i;
			for(i=0; i<2; i++)
			{
				FsAirplane *wingman;
				wingman=world->AddAirplane(airName,YSFALSE);
				wingman->iff=FS_IFF0;

				wingman->Prop().SendCommand("CTLLDGEA FALSE");
				wingman->Prop().SetPosition(wmPos[i]);
				wingman->Prop().SetAttitude(parAtt);
				wingman->Prop().SetVelocity(parVel);
				wingman->Prop().SendCommand("INITFUEL 15%");

				YsArray <int,64> loading;
				air->Prop().GetWeaponConfig(loading);
				wingman->Prop().ApplyWeaponConfig(loading.GetN(),loading);

				FsFormation *fom;
				fom=FsFormation::Create();
				fom->minAlt=0.0;
				fom->leader=air;
				fom->shouldBe=ofst[i];
				fom->synchronizeTrigger=YSTRUE;
				fom->autoSpacingForLanding=YSFALSE;
				wingman->SetAutopilot(fom);
			}
		}

		// if(extraAirplane==YSTRUE)
		{
			FsAirplane *ext;
			ext=world->AddAirplane(airName,YSFALSE);
			if(ext!=NULL)
			{
				YsVec3 pos,vel;
				YsAtt3 att;
				att=air->GetAttitude();
				pos.Set(0.0,0.0,-800.0);
				att.Mul(pos,pos);
				pos+=air->GetPosition();
				air->Prop().GetVelocity(vel);

				ext->Prop().SendCommand("CTLLDGEA FALSE");
				ext->Prop().SetPosition(pos);
				ext->Prop().SetAttitude(att);
				ext->Prop().SetVelocity(vel);
				ext->Prop().SendCommand("INITFUEL 15%");

				YsArray <int,64> loading;
				air->Prop().GetWeaponConfig(loading);
				ext->Prop().ApplyWeaponConfig(loading.GetN(),loading);

				FsLandingAutopilot *ldg;
				ldg=FsLandingAutopilot::Create();
				ldg->landingSpeedCorrection=ap->landingSpeedCorrection;  // <- must come before SetAirplaneInfo
				ldg->SetAirplaneInfo(*ext,ap->bankLimit);
				ldg->glideSlope=ap->glideSlope;
				ldg->flareAlt=ap->flareAlt;
				ldg->alwaysGoAround=YSTRUE;
				ldg->wheelControlOnGround=YSFALSE;
				ldg->SetIls(*ext,world->GetSimulation(),ap->ils);
				ext->SetAutopilot(ldg);
			}
		}
		return YSOK;
	}
	return YSERR;
}

void FsRunLoop::StartCarrierLandingDemo(const char fldName[],const char airName[])
{
	if(YSOK==SetUpCarrierLandingDemo(fldName,airName))
	{
		demoModeRecordFlight=YSFALSE;
		demoModeDrawSmoke=YSFALSE;
		StartDemoModeInGeneral(FSDEMO_LANDING,"Press key to exit demonstration.",0.0);
	}
}

YSRESULT FsRunLoop::SetUpCarrierLandingDemo(const char fldName[],const char airName[])
{
	world->TerminateSimulation();
	world->PrepareSimulation();


	world->AddField(NULL,fldName,YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0),YSTRUE,YSFALSE);
	world->DisableGroundFire();


	FsAirplane *air;

	air=world->AddAirplane(airName,YSTRUE);
	air->iff=FS_IFF0;

	// No formation landing in carrier landing

	if(world->SettleAirplaneForCarrierLandingDemo(*air)==YSOK)
	{
		FsLandingAutopilot *ap;
		ap=(FsLandingAutopilot *)air->GetAutopilot();
		ap->autoClearRunway=YSFALSE;

		// No formation landing in carrier landing

		// if(extraAirplane==YSTRUE)
		{
			FsAirplane *ext;
			ext=world->AddAirplane(airName,YSFALSE);
			if(ext!=NULL)
			{
				YsVec3 pos,vel;
				YsAtt3 att;
				att=air->GetAttitude();
				pos.Set(0.0,0.0,-800.0);
				att.Mul(pos,pos);
				pos+=air->GetPosition();
				air->Prop().GetVelocity(vel);

				ext->Prop().SendCommand("CTLLDGEA FALSE");
				ext->Prop().SetPosition(pos);
				ext->Prop().SetAttitude(att);
				ext->Prop().SetVelocity(vel);
				ext->Prop().SendCommand("INITFUEL 15%");

				YsArray <int,64> loading;
				air->Prop().GetWeaponConfig(loading);
				ext->Prop().ApplyWeaponConfig(loading.GetN(),loading);

				FsLandingAutopilot *ldg;
				ldg=FsLandingAutopilot::Create();
				ldg->landingSpeedCorrection=ap->landingSpeedCorrection;  // <- must come before SetAirplaneInfo
				ldg->SetAirplaneInfo(*ext,YsPi/2.0);
				ldg->glideSlope=ap->glideSlope;
				ldg->flareAlt=ap->flareAlt;
				ldg->alwaysGoAround=YSTRUE;
				ldg->wheelControlOnGround=YSFALSE;
				ldg->SetIls(*ext,world->GetSimulation(),ap->ils);
				ext->SetAutopilot(ldg);
			}


			FsAirplane *e2c;
			e2c=world->AddAirplane("E-2C_HAWKEYE",YSFALSE);
			if(e2c!=NULL)
			{
				double elv,x,z;
				YsVec3 gotoPos,initPos,lead;
				YsAtt3 carrierAtt;
				carrierAtt=ap->ils->GetAttitude();
				carrierAtt.Mul(lead,YsZVec());
				lead*=1000.0;
				lead.SetY(0.0);

				elv=air->GetPosition().y()+180.0;
				x=ap->ils->GetPosition().x();
				z=ap->ils->GetPosition().z();

				gotoPos.Set(x,elv,z);
				gotoPos+=lead;

				initPos=gotoPos;
				initPos.SubZ(300.0);

				e2c->Prop().SetPosition(initPos);
				e2c->Prop().SetAttitude(YsZeroAtt());
				e2c->SendCommand("INITSPED 100m/s");

				FsGotoPosition *gp;
				gp=FsGotoPosition::Create();
				gp->SetSingleDestination(gotoPos);
				gp->speed=100.0;
				gp->minAlt=0.0;
				gp->straightFlightMode=YSFALSE;
				e2c->SetAutopilot(gp);
			}
		}
		return YSOK;
	}
	return YSERR;
}

void FsRunLoop::StartDogFightDemo(void)
{
	if(YSOK==SetUpDogFightDemo())
	{
		demoModeRecordFlight=YSFALSE;
		demoModeDrawSmoke=YSFALSE;
		StartDemoModeInGeneral(FSDEMO_DOGFIGHT,"Press key to exit demonstration.",0.0);
	}
}

YSRESULT FsRunLoop::SetUpDogFightDemo(void)
{
	int winner,nIff0,nIff1;
	double gLimit0,gLimit1,altLimit0,altLimit1;

	int nAirCandidate;
	char *airCandidate[256];
	world->GetFighterList(nAirCandidate,airCandidate,256);
	if(0>=nAirCandidate)
	{
		return YSERR;
	}

	world->TerminateSimulation();
	world->PrepareSimulation();


	if(NULL==world->AddField(NULL,"ATSUGI_AIRBASE",YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0)))
	{
		return YSERR;
	}


	winner=((rand()%100)>50 ? 0 : 1);
	if(winner==0)
	{
		nIff0=3;
		nIff1=1;
		gLimit0=5.0;
		gLimit1=9.0;
		altLimit0=1500.0;
		altLimit1=1000.0;
	}
	else
	{
		nIff0=2;
		nIff1=2;
		gLimit0=9.0;
		gLimit1=7.0;
		altLimit0=1000.0;
		altLimit1=1500.0;
	}


	// nIff0=1;//  1 by 1 dogfight test
	// nIff1=1;//  1 by 1 dogfight test


	int i,id;
	char stp[256];
	FsAirplane *air;
	FsDogfight *df;

	for(i=0; i<nIff0; i++)
	{
		id=rand()%nAirCandidate;
		sprintf(stp,"NORTH10000_%02d",i+1);

		air=world->AddAirplane(airCandidate[id],(i==0 ? YSTRUE : YSFALSE));
		air->iff=FS_IFF0;

		df=FsDogfight::Create();
		df->gLimit=gLimit0;
		df->minAlt=altLimit0;

		air->SetAutopilot(df);
		air->gLimit=gLimit0;
		if(i==0 && winner==1)
		{
			air->SendCommand("INITIAAM 1");
		}
		else
		{
			air->SendCommand("INITIAAM 0");
		}

		air->SendCommand("INITIAGM 0");
		air->SendCommand("INITBOMB 0");

		// air->SendCommand("INITIAAM 0");  // Machine gun dogfight test
		world->SettleAirplane(*air,stp);
	}


	for(i=0; i<nIff1; i++)
	{
		id=rand()%nAirCandidate;
		sprintf(stp,"SOUTH10000_%02d",i+3);

		air=world->AddAirplane(airCandidate[id],YSFALSE);
		air->iff=FS_IFF1;

		df=FsDogfight::Create();
		df->gLimit=gLimit1;
		df->minAlt=altLimit1;
		air->SetAutopilot(df);
		air->gLimit=gLimit1;
		air->Prop().SendCommand("INITIAAM 0");

		if(i==0 && winner==0)
		{
			air->SendCommand("INITIAAM 2");
		}
		else
		{
			air->SendCommand("INITIAAM 0");
		}
		air->SendCommand("INITIAGM 0");
		air->SendCommand("INITBOMB 0");

		// air->SendCommand("INITIAAM 0");  // Machine gun dogfight test

		world->SettleAirplane(*air,stp);
	}

	return YSOK;
}

void FsRunLoop::StartAirShowDemo(const char fldName[],const char airName[],int acroType)
{
	world->TerminateSimulation();
	world->PrepareSimulation();
	if(NULL==world->AddField(NULL,fldName,YsOrigin(),YsZeroAtt(),YSTRUE,YSFALSE))
	{
		return;
	}

	MakeFreshDemoModeInfo(FSDEMO_ACROBAT);
	if(world->PrepareAcrobat(*demoModeInfo,airName,acroType)==YSOK)
	{
		demoModeRecordFlight=YSFALSE;
		demoModeDrawSmoke=YSTRUE;
		demoModeInfo->changeViewPointInterval=6.0+(double)(rand()%400)/100.0;

	#ifndef ANDROID
		world->PrepareRunDemoMode(*demoModeInfo,"Press key to exit demonstration.",0.0);
	#else
		world->PrepareRunDemoMode(*demoModeInfo,"Touch 2 fingers to exit demonstration.",0.0);
	#endif
		ChangeRunMode(YSRUNMODE_FLY_DEMOMODE);
	}
}

void FsRunLoop::StartDemoModeInGeneral(
    FSDEMOMODETYPE demoType,const char sysMsg[],const double maxTime)
{
	MakeFreshDemoModeInfo(demoType);
	world->PrepareRunDemoMode(*demoModeInfo,sysMsg,maxTime);
	ChangeRunMode(YSRUNMODE_FLY_DEMOMODE);
}

FsDemoModeInfo *FsRunLoop::MakeFreshDemoModeInfo(FSDEMOMODETYPE demoType)
{
	if(NULL!=demoModeInfo)
	{
		delete demoModeInfo;
		demoModeInfo=NULL;
	}
	demoModeInfo=new FsDemoModeInfo(demoType);
	return demoModeInfo;
}

void FsRunLoop::AfterDemoAction(void)
{
	if(YSTRUE==autoDemoForever)
	{
		return;
	}
}

void FsRunLoop::ChangeRunMode(RUNMODE runMode)
{
	FsDisableIME();  // Just in case.

	if(0==runModeStack.GetN())
	{
		runModeStack.Increment();
	}

	runModeStack.Last().runMode=runMode;
	runModeStack.Last().runModeCounter=0;

	// Actually wait until all buttons, keys are clear.
	FsPollDevice();
	while(FsInkey()!=FSKEY_NULL)
	{
		FsPollDevice();
	}
}

void FsRunLoop::ChangeSimulationState(FsSimulation::FSSIMULATIONSTATE simState)
{
	this->simState=simState;
}

void FsRunLoop::PushRunMode(void)
{
	FsDisableIME();  // Just in case.

	runModeStack.Increment();
	if(1<runModeStack.GetN())
	{
		runModeStack[runModeStack.GetN()-1]=runModeStack[runModeStack.GetN()-2];
	}
}

void FsRunLoop::PopRunMode(void)
{
	FsDisableIME();  // Just in case.

	if(0<runModeStack.GetN())
	{
		runModeStack.DeleteLast();
	}
}

void FsRunLoop::TakeOff(RUNMODE nextRunMode)
{
	ChangeSimulationState(FsSimulation::FSSIMSTATE_CENTERJOYSTICK);
	ChangeRunMode(nextRunMode);
}

YSRESULT FsRunLoop::SetUpEnduranceMode(
    const char playerPlane[],
    YSBOOL jet,YSBOOL ww2,
    const char fieldName[],int numWingman,int wingmanLevel,YSBOOL allowAAM,
    YSSIZE_T nWeaponConfigInput,const int weaponConfigInput[],int fuel)
{
	YsArray <int,64> weaponConfig;
	char startPosition[256];
	strcpy(startPosition,"NORTH10000_01");
	world->TerminateSimulation();
	world->PrepareSimulation();

	YsVec3 vec(0.0,0.0,0.0);
	YsAtt3 att(0.0,0.0,0.0);
	if(NULL==world->AddField(NULL,fieldName,vec,att,YSTRUE,YSFALSE))  // gnd=TRUE, air=FALSE
	{
		AttachGuiMessageBox(
		    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
		    FSGUI_COMMON_OK);
		world->TerminateSimulation();
		return YSERR;
	}
	world->DisableGroundFire();
	world->AllowAAM(allowAAM);

	weaponConfig.Set(nWeaponConfigInput,weaponConfigInput);

	FsAirplane *air=world->AddAirplane(playerPlane,YSTRUE);
	air->iff=FS_IFF0;
	world->SettleAirplane(*air,startPosition);
	if(allowAAM!=YSTRUE)
	{
		FsAirplaneProperty::RemoveWeaponFromWeaponConfig
		    (weaponConfig.GetN(),weaponConfig.GetEditableArray(),FSWEAPON_AIM9);
		FsAirplaneProperty::RemoveWeaponFromWeaponConfig
		    (weaponConfig.GetN(),weaponConfig.GetEditableArray(),FSWEAPON_AIM120);
	}

	air->AutoSendCommand(weaponConfig.GetN(),weaponConfig,fuel);
	air->SendCommand("INITIAGM 0");
	air->SendCommand("INITBOMB 0");
	air->Prop().LoadFuel();

	for(int i=0; i<numWingman; i++)
	{
		FsAirplane *wingman;
		wingman=world->AddAirplane(playerPlane,YSFALSE);  // Same type as player airplane
		wingman->iff=FS_IFF0;
		sprintf(startPosition,"NORTH10000_%02d",i+2);
		world->SettleAirplane(*wingman,startPosition);
		wingman->AutoSendCommand(weaponConfig.GetN(),weaponConfig,fuel);
		wingman->SendCommand("INITIAGM 0");
		wingman->SendCommand("INITBOMB 0");
		wingman->Prop().LoadFuel();

		FsDogfight *df;
		df=FsDogfight::Create();
		df->gLimit=5.0+double(wingmanLevel*2);
		df->minAlt=0.0;
		wingman->SetAutopilot(df);
		wingman->gLimit=5.0+double(wingmanLevel*2);
	}

	air->Prop().TurnOnRadar();

	world->AllowAAM(allowAAM);

	std::shared_ptr <FsSimExtension_EnduranceMode> addOnPtr;
	addOnPtr.reset(new FsSimExtension_EnduranceMode);
	addOnPtr->jet=jet;
	addOnPtr->ww2=ww2;
	addOnPtr->allowAAM=allowAAM;
	world->RegisterExtension(addOnPtr);

	return YSOK;
}

YSRESULT FsRunLoop::SetUpCloseAirSupportMission(const char airName[],const char fldName[],YSSIZE_T nWeaponConfig,const int weaponConfig[],int fuel)
{
	char startPosition[256];;
	strcpy(startPosition,"NORTH10000_01");

	world->TerminateSimulation();
	world->PrepareSimulation();

	YsVec3 vec(0.0,0.0,0.0);
	YsAtt3 att(0.0,0.0,0.0);
	if(NULL==world->AddField(NULL,fldName,vec,att,YSTRUE,YSFALSE,0x01))  // 0x01 -> Only IFF1
	{
		AttachGuiMessageBox(
		    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
		    FSGUI_COMMON_OK);
		world->TerminateSimulation();
		return YSERR;
	}


	if(world->CheckCloseAirSupportMissionAvailable()!=YSTRUE)
	{
		world->TerminateSimulation();
		AttachGuiMessageBox(
		    L"ERROR",
			L"Close Air Support Mission is not available for this map\n"
			L"because ground target information is missing",
			FSGUI_COMMON_OK);
		return YSERR;
	}

	char stpName[256];
	for(int i=0; world->GetFieldStartPositionName(stpName,fldName,i)==YSOK; i++)
	{
		if(stpName[0]=='R' && stpName[1]=='W')
		{
			strcpy(startPosition,stpName);
			break;
		}
	}

	FsAirplane *air=world->AddAirplane(airName,YSTRUE);
	air->iff=FS_IFF0;
	world->SettleAirplane(*air,startPosition);
	air->AutoSendCommand(nWeaponConfig,weaponConfig,fuel);
	air->Prop().LoadFuel();

	air->Prop().TurnOnRadar();



	FsMissionGoal goal;
	goal.goalFlag|=FSGOAL_DEFENDPRMGND;
	goal.numPrmGndMustSurvive=YsGreater(1,world->GetNumPrimaryGroundTarget(FS_IFF0)/2);
	goal.duration=60.0*15.0;
	world->SetMissionGoal(goal);



	std::shared_ptr <FsSimExtension_CloseAirSupport> extPtr;
	extPtr.reset(new FsSimExtension_CloseAirSupport);
	// extPtr->info=info;  Not used now.
	world->RegisterExtension(extPtr);


	return YSOK;
}

YSRESULT FsRunLoop::SetUpInterceptMission(const class FsInterceptMissionInfo &info)
{
	const char *playerPlane=info.playerAirInfo.typeName;
	const char *fieldName=info.fieldName;
	const int fuel=info.playerAirInfo.fuel;

	YsArray <int,64> weaponConfig;
	char startPosition[256];;
	strcpy(startPosition,"NORTH10000_01");

	weaponConfig.Set(info.playerAirInfo.weaponConfig.GetN(),info.playerAirInfo.weaponConfig);

	FsAirplaneProperty::RemoveWeaponFromWeaponConfig
	    ((int)weaponConfig.GetN(),weaponConfig.GetEditableArray(),FSWEAPON_BOMB);
	FsAirplaneProperty::RemoveWeaponFromWeaponConfig
	    ((int)weaponConfig.GetN(),weaponConfig.GetEditableArray(),FSWEAPON_AGM65);
	FsAirplaneProperty::RemoveWeaponFromWeaponConfig
	    ((int)weaponConfig.GetN(),weaponConfig.GetEditableArray(),FSWEAPON_BOMB250);

	world->TerminateSimulation();
	world->PrepareSimulation();

	YsVec3 vec(0.0,0.0,0.0);
	YsAtt3 att(0.0,0.0,0.0);
	if(NULL==world->AddField(NULL,fieldName,vec,att,YSTRUE,YSFALSE,0x01)) // loadYfs=YSTRUE,loadAir=YSFALSE,iffControl=1
	{
		AttachGuiMessageBox(
		    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
		    FSGUI_COMMON_OK);
		world->TerminateSimulation();
		return YSERR;
	}
	world->DisableGroundFire();

	if(world->CheckInterceptMissionAvailable()!=YSTRUE)
	{
		AttachGuiMessageBox(
			L"ERROR!",
			L"Intercept Mission is not available for this map\n"
			L"because ground target information is missing",
			FSGUI_COMMON_OK);
		world->TerminateSimulation();
		return YSERR;
	}



	std::shared_ptr <FsSimExtension_InterceptMission> addOnPtr;
	addOnPtr.reset(new FsSimExtension_InterceptMission);
	addOnPtr->info=info;
	world->RegisterExtension(addOnPtr);



	char stpName[256];
	for(int i=0; world->GetFieldStartPositionName(stpName,fieldName,i)==YSOK; i++)
	{
		if(stpName[0]=='R' && stpName[1]=='W')
		{
			strcpy(startPosition,stpName);
			break;
		}
	}

	FsAirplane *air=world->AddAirplane(playerPlane,YSTRUE);
	air->iff=FS_IFF0;
	world->SettleAirplane(*air,startPosition);
	air->AutoSendCommand(weaponConfig.GetN(),weaponConfig,fuel);
	air->Prop().LoadFuel();
	air->Prop().TurnOnRadar();



	FsMissionGoal goal;
	goal.goalFlag|=FSGOAL_DEFENDPRMGND;
	goal.numPrmGndMustSurvive=YsGreater(1,world->GetNumPrimaryGroundTarget(FS_IFF0)/2);
	goal.duration=60.0*15.0;
	world->SetMissionGoal(goal);

	return YSOK;
}

YSRESULT FsRunLoop::SetUpGroundToAirMission(const FsGroundToAirDefenseMissionInfo &info)
{
	FsAddedFieldInfo addedFieldInfo;

	world->TerminateSimulation();
	world->PrepareSimulation();
	if(NULL==world->AddField(&addedFieldInfo,info.fieldName,YsOrigin(),YsZeroAtt())) // This function needs to return gobId's object
	{
		AttachGuiMessageBox(
		    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
		    FSGUI_COMMON_OK);
		world->TerminateSimulation();
		return YSERR;
	}
	world->SetEnvironment(FSDAYLIGHT);


	FsMissionGoal goal;
	goal.goalFlag|=FSGOAL_DEFENDPRMGND;
	goal.numPrmGndMustSurvive=YsGreater(1,world->GetNumPrimaryGroundTarget(FS_IFF0)/2);
	goal.duration=60.0*15.0;
	world->SetMissionGoal(goal);


	if(YSTRUE==addedFieldInfo.gndArray.IsInRange(info.gobId) &&
	   NULL!=addedFieldInfo.gndArray[info.gobId])
	{
		world->SetPlayerGround(addedFieldInfo.gndArray[info.gobId],YSFALSE); // It is an initial condition.  No need for recording a player-change event.

		std::shared_ptr <FsSimExtension_GroundToAir> extPtr;
		extPtr.reset(new FsSimExtension_GroundToAir);
		extPtr->info=info;
		world->RegisterExtension(extPtr);

		return YSOK;
	}

	return YSERR;
}

YSRESULT FsRunLoop::SetUpLandingPracticeMode(
    const char fldName[],const char airName[],
    YSBOOL &leftTraffic, // Need to return to the caller....
    FSTRAFFICPATTERNLEG leg,const double &crossWind,YSBOOL lowCloud,YSBOOL lowVisibility)
{
	world->TerminateSimulation();
	world->PrepareSimulation();

	if(NULL==world->AddField(NULL,fldName,YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0),YSTRUE,YSFALSE)) // gnd=TRUE, air=FALSE
	{
		AttachGuiMessageBox(
		    FSGUI_COMMON_ERROR,world->GetLastErrorMessage(),
		    FSGUI_COMMON_OK);
		world->TerminateSimulation();
		return YSERR;
	}
	world->DisableGroundFire();
	world->AssignUniqueYsfId();
	world->SetCanContinue(YSFALSE); // No continue for landing-practice mode.


	FsAirplane *air;
	air=world->AddAirplane(airName,YSTRUE);
	air->iff=FS_IFF0;


	YsVec3 pos,tdPos;
	YsAtt3 att,tdAtt;
	double initSpd;
	FsGround *ils;
	double obs;
	if(world->GetLandingPracticeStartPosition(leftTraffic,ils,obs,*air,pos,att,tdPos,tdAtt,initSpd,leg)==YSOK)
	{
		char cmd[256];
		YsVec3 wind;
		FsFlightPlanFix::FIXTYPE firstFix=FsFlightPlanFix::FIX_NULL;

		wind.Set(crossWind,0.0,0.0);
		if(rand()%100<50)
		{
			wind=-wind;
		}

		const double reqAOA=air->Prop().ComputeAOAForRequiredG(1.0,initSpd);
		const double elvTrim=YsBound(air->Prop().ComputeElevatorTrimForAOA(reqAOA),-1.0,1.0);

		air->Prop().SendCommand("CTLLDGEA FALSE");

		sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm",pos.x(),pos.y(),pos.z());
		air->SendCommand(cmd);
		sprintf(cmd,"ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
		    YsRadToDeg(att.h()),YsRadToDeg(att.p()),YsRadToDeg(att.b()));
		air->SendCommand(cmd);

		sprintf(cmd,"INITSPED %lfm/s",initSpd);
		air->SendCommand(cmd);

		att.SetP(reqAOA);
		sprintf(cmd,"ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
		    YsRadToDeg(att.h()),YsRadToDeg(att.p()),YsRadToDeg(att.b()));
		air->SendCommand(cmd);

		switch(leg)
		{
		case FSLEG_FINAL: // On Final
			air->SendCommand("INITFUEL 2%%");
			firstFix=FsFlightPlanFix::FIX_GLIDESLOPEINTERCEPT;  // On final
			break;
		case FSLEG_DOG: // On Dog-leg
			air->SendCommand("INITFUEL 4%%");
			firstFix=FsFlightPlanFix::FIX_LOCALIZERINTERCEPT;  // Dogleg to final
			break;
		case FSLEG_BASE: // On Base
			air->SendCommand("INITFUEL 6%%");
			firstFix=FsFlightPlanFix::FIX_DOGLEGENTRY;  // Base to dogleg
			break;
		}


		// >> SETCNTRL must be sent before PrepareRunSimulation
		char ctlCmd[512];
		sprintf(ctlCmd,"SETCNTRL TRIM:%.2lf",elvTrim);
		if((lowCloud==YSTRUE || lowVisibility==YSTRUE) && ils!=NULL)
		{
			char add[256];
			sprintf(add," NAV1:%d OBS1:%.2lf",ils->ysfId,obs);
			strcat(ctlCmd,add);
		}
		air->SendCommand(ctlCmd);
		// << SETCNTRL must be sent before PrepareRunSimulation


		// Deciding Weather
		if(lowCloud==YSTRUE)
		{
			world->MakeSolidCloud(16,tdPos,15000.0,12000.0,80.0,120.0);
			world->AddOvercastLayer(220.0,260.0);
			world->MakeSolidCloud(16,tdPos,15000.0,4500.0,1000.0,1200.0);
		}
		if(lowVisibility==YSTRUE)
		{
			world->SetFog(YSTRUE,800.0);
		}

		if(YSTRUE==lowCloud || YSTRUE==lowVisibility)
		{
			FsSimulation *sim=world->GetSimulation();
			class FsAirTrafficController *atc=sim->FindAirTrafficController(FsAirTrafficController::PrimaryAirTrafficControllerKey);
			if(NULL!=atc)
			{
				atc->AirplaneCheckIn(sim,air);
				atc->AirplaneRequestApproachByILS(sim,*air,ils,"");
				air->GetAirTrafficInfo().SelectFirstFixForApproach(firstFix);

				YsVec3 pos;
				YsAtt3 att;
				if(YSOK==air->GetAirTrafficInfo().FindFixBefore(pos,att,firstFix))
				{
					// Override if possible.
					YsString cmd;
					cmd.Printf("POSITION %.2lfm %.2lfm %.2lfm",pos.x(),pos.y(),pos.z());
					air->SendCommand(cmd);
					cmd.Printf("ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
					YsRadToDeg(att.h()),YsRadToDeg(att.p()),YsRadToDeg(att.b()));
					air->SendCommand(cmd);

					pos.SetY(0.0);
					world->MakeSolidCloud(16,pos,15000.0,12000.0,80.0,120.0);
				}
			}
		}

		tdAtt.Mul(wind,wind);
		world->SetWind(wind);

		// Save prevflight.dat
		world->Save(FsGetPrevFlightFile(),3,4,2,2,2,2,0.0);

		return YSOK;
	}
	else
	{
		AttachGuiMessageBox(FSGUI_COMMON_ERROR,FSERRMSG_LANDINGPRACTICE_NOTAVAILABL,FSGUI_COMMON_OK);
	}
	return YSERR;
}

void FsRunLoop::StartNetServerMode(const char username[],const char fldName[],int netPort)
{
	world->TerminateSimulation();
	world->PrepareSimulation();
	world->CheckJoystickAssignmentAndFixIfNecessary();

	netcfgPtr->Load(FsGetNetConfigFile());

	if(fldName!=NULL && fldName[0]==0)
	{
		fldName=NULL;
	}

	if(NULL!=serverModeRunLoop)
	{
		delete serverModeRunLoop;
	}
	serverModeRunLoop=new FsServerRunLoop(username,fldName,netPort,world->GetSimulation(),netcfgPtr);
	ChangeRunMode(YSRUNMODE_FLY_SERVERMODE);
}

void FsRunLoop::StartNetClientMode(const char username[],const char hostname[],int netPort)
{
	netcfgPtr->Load(FsGetNetConfigFile());
	if(0>netPort)
	{
		netPort=netcfgPtr->portNumber;
	}

	world->TerminateSimulation();
	world->PrepareSimulation();
	world->CheckJoystickAssignmentAndFixIfNecessary();

	if(NULL!=clientModeRunLoop)
	{
		delete clientModeRunLoop;
	}
	clientModeRunLoop=new FsClientRunLoop(username,netPort,world->GetSimulation(),netcfgPtr);
	clientModeRunLoop->hostname=hostname;
	clientModeRunLoop->username=username;

	ChangeRunMode(YSRUNMODE_FLY_CLIENTMODE);
}



YSBOOL FsRunLoop::RunOneStep(void)
{
#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	if(0<runModeStack.GetN())
	{
		YSBOOL res=YSFALSE;
		const RUNMODE prevRunMode=runModeStack.GetEnd().runMode;

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		switch(runModeStack.GetEnd().runMode)
		{
		case YSRUNMODE_MENU:
			res=RunMenuOneStep();
			break;
		case YSRUNMODE_SHOWLANDINGPRACTICEINFO:
			res=RunShowLandingPracticeInfoOneStep();
			FsSleep(50);
			break;
		case YSRUNMODE_FLY_REGULAR:
		case YSRUNMODE_FLY_DEMOMODE:
		case YSRUNMODE_REPLAYRECORD:
		case YSRUNMODE_FLY_CLIENTMODE:
		case YSRUNMODE_FLY_SERVERMODE:
			res=RunSimulationOneStep();
			break;
		default:
			ChangeRunMode(YSRUNMODE_MENU);
			AttachGuiMessageBox(
			   L"INTERNAL ERROR",
			   L"ENTERED UNDEFINED RUN MODE",
			   FSGUI_COMMON_OK);
			break;
		}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		if(0<runModeStack.GetN() && prevRunMode==runModeStack.GetEnd().runMode)
		{
			++runModeStack.GetEnd().runModeCounter;
		}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		return res;
	}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	return YSFALSE;
}

void FsRunLoop::ProcessGuiCommand(const char *cmd)
{
	if(0<runModeStack.GetN())
	{
		switch(runModeStack.GetEnd().runMode)
		{
		case YSRUNMODE_MENU:
			if(nullptr!=mainCanvas)
			{
				mainCanvas->ProcessCommand(cmd);
			}
			break;
		case YSRUNMODE_FLY_CLIENTMODE:
			if(nullptr!=clientModeRunLoop)
			{
				clientModeRunLoop->canvasPtr->ProcessCommand(cmd);
			}
			break;
		}
	}
}

void FsRunLoop::ProcessFileCommand(const char *cmdIn)
{
	YsString cmd(cmdIn);
	auto argv=cmd.Argv();
	if(0<argv.size() && 
	   GetCurrentRunMode()==YSRUNMODE_MENU &&
	   (nullptr==mainCanvas || nullptr==mainCanvas->GetActiveModalDialog()))
	{
		if(0==argv[0].STRCMP("OPEN") && 2<=argv.size())
		{
			YsWString fNameIn;
			fNameIn.SetUTF8String(argv[1]);
			auto fName=YsSpecialPath::ExpandUser(fNameIn);
			world->Load(fName);
		}
		else if((0==argv[0].STRCMP("SAVE") || 0==argv[0].STRCMP("SAVEAS")) && 2<=argv.size())
		{
			YsWString fNameIn;
			fNameIn.SetUTF8String(argv[1]);
			auto fName=YsSpecialPath::ExpandUser(fNameIn);
			world->Save(fName,4,4,4,4,4,4,0.05);
		}
	}
}

void FsRunLoop::ProcessFlightCommand(const char *cmd)
{
	if(YSTRUE==Flying())
	{
		auto sim=world->GetSimulation();
		YsString str(cmd);
		auto argv=str.Argv();
		if(0<argv.size())
		{
			if(0==argv[0].STRCMP("AUTOPILOT") && 3<=argv.size())
			{
				FsAirplane *air=nullptr;
				if(0==argv[1].STRCMP("$PLAYER"))
				{
					air=world->GetPlayerAirplane();
				}
				if(nullptr!=air)
				{
					auto ap=FsAutopilot::ReadIntentionEZ(argv.Subset(2));
					if(nullptr!=ap)
					{
						air->SetAutopilot(ap);
					}
				}
			}
		}
	}
}

void FsRunLoop::ProcessButtonFunction(const char *cmd)
{
	if(YSTRUE==Flying())
	{
		auto sim=world->GetSimulation();
		auto btf=FsGetButtonFuncFromString(cmd);
		if(FSBTF_NULL!=btf)
		{
			sim->SimProcessButtonFunction(btf,FSUSC_SCRIPT);
		}
	}
}

void FsRunLoop::ProcessAssertCommand(const char *cmdIn)
{
	YsString cmd(cmdIn);
	auto argv=cmd.Argv();
	if(2<=argv.size())
	{
		if(0==argv[0].STRCMP("FINALSTATE"))
		{
			auto sim=world->GetSimulation();
			auto playerAir=sim->GetPlayerAirplane();
			if(nullptr!=playerAir)
			{
				auto sta=playerAir->GetFinalState();
				auto staStr=FsFlightStateToStr(sta);

				if(0==argv[1].STRCMP(staStr))
				{
					return;
				}
				else
				{
					fprintf(stderr,"ACTUAL FINALSTATE=%s\n",staStr);
				}
			}
		}

		fprintf(stderr,"%s\n",__FUNCTION__);
		fprintf(stderr,"  Assertion Error.\n");
		fprintf(stderr,"  CMD: %s\n",cmdIn);
		exit(1);
	}
	else
	{
		fprintf(stderr,"%s\n",__FUNCTION__);
		fprintf(stderr,"  Fatal Error: Too few arguments.\n");
		fprintf(stderr,"  CMD: %s\n",cmdIn);
		exit(1);
	}
}

YSBOOL FsRunLoop::NetReady(void) const
{
	switch(runModeStack.GetEnd().runMode)
	{
	default:
		break;
	case YSRUNMODE_FLY_CLIENTMODE:
		if(nullptr!=clientModeRunLoop)
		{
			return clientModeRunLoop->NetReady();
		}
		break;
	case YSRUNMODE_FLY_SERVERMODE:
		if(nullptr!=serverModeRunLoop)
		{
		}
		break;
	}
	return YSFALSE;
}

YSBOOL FsRunLoop::Flying(void) const
{
	if(GetCurrentRunMode()==YSRUNMODE_FLY_REGULAR ||
	   GetCurrentRunMode()==YSRUNMODE_FLY_DEMOMODE)
	{
		return YSTRUE;
	}
    else if(GetCurrentRunMode()==YSRUNMODE_FLY_CLIENTMODE &&
            nullptr!=clientModeRunLoop &&
            FsClientVariable::CLISTATE_FLYING==clientModeRunLoop->cli.clientState)
	{
		return YSTRUE;
	}
    else if(GetCurrentRunMode()==FsRunLoop::YSRUNMODE_FLY_SERVERMODE &&
            nullptr!=serverModeRunLoop &&
            FsServerVariable::SVRSTATE_FLYING==serverModeRunLoop->svr.serverState)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsRunLoop::RunSimulationOneStep(void)
{
	switch(runModeStack.GetEnd().runMode)
	{
	default:
		ChangeRunMode(YSRUNMODE_MENU);
		AttachGuiMessageBox(
		   L"INTERNAL ERROR",
		   L"ENTERED UNDEFINED RUN MODE IN RunSimulationOneStep",
		   FSGUI_COMMON_OK);
		break;
	case YSRUNMODE_FLY_REGULAR:
		world->RunSimulationOneStep(simState);
		if(FsSimulation::FSSIMSTATE_OVER==simState)
		{
			EndSimulationMode();
		}
		break;
	case YSRUNMODE_REPLAYRECORD:
		world->RunReplayOneStep(simState,replayInfo);
		if(FsSimulation::FSSIMSTATE_OVER==simState)
		{
			world->PrepareReplaySimulation();
			if(nullptr!=mainCanvas)
			{
				mainCanvas->OnTerminateReplay();
			}
			ChangeRunMode(YSRUNMODE_MENU);
		}
		break;
	case YSRUNMODE_FLY_DEMOMODE:
		simState=FsSimulation::FSSIMSTATE_RUNNING;
		if(YSTRUE!=world->DemoModeOneStep(*demoModeInfo,demoModeDrawSmoke,demoModeRecordFlight))
		{
			world->AfterDemoMode(*demoModeInfo);
			if(YSTRUE!=demoModeRecordFlight)
			{
				world->TerminateSimulation();
				world->UnprepareAllTemplate();
				YsShell::CleanUpVertexPolygonStore();
			}
			AfterDemoAction();
			if(YSTRUE!=autoDemoForever)
			{
				ChangeRunMode(YSRUNMODE_MENU);
			}
			else
			{
				StartOpeningDemo();
			}
		}
		break;
	case YSRUNMODE_FLY_CLIENTMODE:
		if(NULL!=clientModeRunLoop)
		{
			world->RunClientModeOneStep(*clientModeRunLoop);
			if(FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED==clientModeRunLoop->runState)
			{
				world->PrepareReplaySimulation();

				if(clientModeRunLoop->netcfg.recordWhenClientMode!=YSTRUE ||
				   FsClientRunLoop::CLIENT_FATAL_NOERROR!=clientModeRunLoop->fatalError)
				{
					world->TerminateSimulation();
					world->UnprepareAllTemplate();
					YsShell::CleanUpVertexPolygonStore();
				}

				ChangeRunMode(YSRUNMODE_MENU);

				switch(clientModeRunLoop->fatalError)
				{
				default:
					break;
				case FsClientRunLoop::CLIENT_FATAL_NO_NETWORK:
					AttachGuiMessageBox(L"ERROR",L"Cannot start network",FSGUI_COMMON_CLOSE);
					break;
				case FsClientRunLoop::CLIENT_FATAL_CANNOT_CONNECT:
					AttachGuiMessageBox(L"ERROR",L"Cannot connect to the host",FSGUI_COMMON_CLOSE);
					break;
				case FsClientRunLoop::CLIENT_FATAL_CANNOT_LOGON:
					AttachGuiMessageBox(L"ERROR",L"Cannot logon to the host",FSGUI_COMMON_CLOSE);
					break;
				case FsClientRunLoop::CLIENT_FATAL_FIELD_UNAVAILABLE:
					{
						YsString msg;
						msg.Set("Field \"");
						msg.Append(clientModeRunLoop->cli.fieldName);
						msg.Append("\" is not installed on this client.");

						YsWString wMsg;
						wMsg.SetUTF8String(msg);

						AttachGuiMessageBox(wMsg,FSGUI_NET_ERROR_SCENERYNOTAVAILABLE,FSGUI_COMMON_CLOSE);
					}
					break;
				case FsClientRunLoop::CLIENT_FATAL_VERSIONCONFLICT:
					{
						YsWString wMsg=FSGUI_NET_ERROR_VERSIONCONFLICT;

						YsString versionInfo;
						versionInfo.Printf("\nServer Net Version: %d\nClient Net Version: %d\n",
						    clientModeRunLoop->reportedServerVersion,
						    YSFLIGHT_NETVERSION);

						YsWString wVersionInfo;
						wVersionInfo.SetUTF8String(versionInfo);

						wMsg.Append(wVersionInfo);

						AttachGuiMessageBox(L"ERROR",wMsg,FSGUI_COMMON_CLOSE);
					}
					break;
				case FsClientRunLoop::CLIENT_FATAL_NO_COMMON_AIRPLANE:
					AttachGuiMessageBox(
						L"ERROR",
					    FSGUI_NET_ERROR_NOAVAILABLEAIRPLANE,
						FSGUI_COMMON_CLOSE);
					break;
				}
			}
		}
		else
		{
			ChangeRunMode(YSRUNMODE_MENU);
		}
		break;
	case YSRUNMODE_FLY_SERVERMODE:
		if(NULL!=serverModeRunLoop)
		{
			world->RunServerModeOneStep(*serverModeRunLoop);
			if(FsServerRunLoop::SERVER_RUNSTATE_TERMINATED==serverModeRunLoop->runState)
			{
				const YSBOOL resetServer=serverModeRunLoop->resetServer;
				world->PrepareReplaySimulation();

				if(netcfgPtr->recordWhenServerMode!=YSTRUE)
				{
					world->TerminateSimulation();
					world->UnprepareAllTemplate();
					YsShell::CleanUpVertexPolygonStore();
				}

				++serverModeRunLoop->resetCounter;

				if(resetServer==YSTRUE && (netcfgPtr->endSvrAfterResetNTimes==0 || serverModeRunLoop->resetCounter<netcfgPtr->endSvrAfterResetNTimes))
				{
					printf("Resetting Server %d\n",(int)time(NULL));

					world->TerminateSimulation();
					world->PrepareSimulation();
					serverModeRunLoop->runState=FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE1;
				}
				else
				{
					ChangeRunMode(YSRUNMODE_MENU);
				}
			}
		}
		else
		{
			ChangeRunMode(YSRUNMODE_MENU);
		}
		break;
	}
	return YSTRUE;
}

void FsRunLoop::DrawInSimulationMode(void)
{
	if(0==runModeStack.GetN())
	{
		return;
	}

	YSBOOL demoMode=YSFALSE;
	YSBOOL showTimer=YSFALSE;
	YSBOOL showTimeMarker=YSFALSE;

	switch(runModeStack.GetEnd().runMode)
	{
	default:
		break;
	case YSRUNMODE_FLY_REGULAR:
		demoMode=YSFALSE;
		showTimer=YSFALSE;
		showTimeMarker=YSFALSE;
		world->DrawInNormalSimulationMode(simState,demoMode,showTimer,showTimeMarker);
		break;
	case YSRUNMODE_REPLAYRECORD:
		demoMode=YSFALSE;
		showTimer=YSTRUE;
		showTimeMarker=YSFALSE;
		world->DrawInNormalSimulationMode(simState,demoMode,showTimer,showTimeMarker);
		break;
	case YSRUNMODE_FLY_DEMOMODE:
		demoMode=YSTRUE;
		showTimer=YSFALSE;
		showTimeMarker=YSFALSE;
		world->DrawInNormalSimulationMode(simState,demoMode,showTimer,showTimeMarker);
		break;
	case YSRUNMODE_FLY_CLIENTMODE:
		if(NULL!=clientModeRunLoop)
		{
			world->DrawInClientMode(*clientModeRunLoop);
		}
		break;
	case YSRUNMODE_FLY_SERVERMODE:
		if(NULL!=serverModeRunLoop)
		{
			world->DrawInServerMode(*serverModeRunLoop);
		}
		break;
	}
}

void FsRunLoop::EndSimulationMode(void)
{
	world->PrepareReplaySimulation();
	if(nullptr!=mainCanvas)
	{
		mainCanvas->OnTerminateSimulation(GetCurrentRunMode());
	}
	ChangeRunMode(YSRUNMODE_MENU);
}

YSBOOL FsRunLoop::RunShowLandingPracticeInfoOneStep(void)
{
	showLandingPracticeInfo->RunOneStep();
	if(FsShowLandingPracticeInfo::STATE_OVER==showLandingPracticeInfo->state)
	{
		// This sequence is same as regular simulation after mission-goal dialog.
		ChangeRunMode(YSRUNMODE_MENU);  // 2014/09/07 Was missing and added.  Why was it working until now?
		TakeOff();
	}
	return YSTRUE;
}

YSBOOL FsRunLoop::RunMenuOneStep(void)
{
#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	if(nullptr!=mainCanvas)
	{
		mainCanvas->Interval();
		if(nullptr==mainCanvas->GetModalDialog() && YSTRUE==terminateWhenNoModalDialogIsOpenAndBackToMenu)
		{
			this->terminate=YSTRUE;
		}
		if(0==runModeStack.GetEnd().runModeCounter)
		{
			mainCanvas->RemoveAllNonPermanentDialog();
		}
	}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	{
		if(YSTRUE==this->needReloadOption)
		{
			opt->Load(FsGetOptionFile());
			FsApplyNonScreenOption(*opt);
			if(nullptr!=mainCanvas)
			{
				mainCanvas->AfterReloadOption(*opt);
			}
			needReloadOption=YSFALSE;
		}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		if(opt->backPicture==YSTRUE)
		{
			if(lastChangePicture==0 || YsAbs(time(NULL)-lastChangePicture)>8)
			{
				int retry;
				titleBmpPtr=NULL;
				for(retry=0; retry<5; retry++)
				{
					if(nTitleBmp>0)
					{
						int n;
						char fn[256];
						n=rand()%nTitleBmp;
						sprintf(fn,"misc/title%02d.png",n);
						if(titleBmp.LoadPng(fn)==YSOK && titleBmp.GetWidth()>0 && titleBmp.GetHeight()>0)
						{
							titleBmpPtr=&titleBmp;
							SetNeedRedraw(YSTRUE);
							break;
						}
					}
				}
				lastChangePicture=time(NULL);
			}
		}
		else
		{
			titleBmpPtr=NULL;
		}

		attentionClock=FsSubSecondTimer()/300;
		if(prevAttentionClock!=attentionClock)
		{
			auto textColor=((attentionClock&1)!=0 ? YsGreen() : YsBlack());
			if(simFlyMsgBmp.GetWidth()>0 && simFlyMsgBmp.GetHeight()>0)
			{
				ChangeBitmapColor(newFltMsgBmp,textColor);
			}
			if(simRepMsgBmp.GetWidth()>0 && simRepMsgBmp.GetHeight()>0)
			{
				ChangeBitmapColor(simRepMsgBmp,textColor);
			}
			prevAttentionClock=attentionClock;
			SetNeedRedraw(YSTRUE);
		}


	    FsSelectMainWindow();
	    for(int i=0; i<FsMaxNumSubWindow; i++)
	    {
			if(FsIsSubWindowOpen(i)==YSTRUE)
			{
				FsCloseSubWindow(i);
				FsSelectMainWindow();
			}
		}

		const int key=FsInkey();
		const YSBOOL shift=(YSBOOL)FsGetKeyState(FSKEY_SHIFT);
		const YSBOOL ctrl=(YSBOOL)FsGetKeyState(FSKEY_CTRL);
		const YSBOOL alt=(YSBOOL)FsGetKeyState(FSKEY_ALT);

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		for(;;)
		{
			int mx,my;
			YSBOOL lb,mb,rb;
			
			int eventType=FsGetMouseEvent(lb,mb,rb,mx,my);
			mainCanvas->SetMouseState(lb,mb,rb,mx,my);
			mainCanvas->SetTouchState(FsGetNumCurrentTouch(),FsGetCurrentTouch());

			if(FSMOUSEEVENT_NONE==eventType)
			{
				break;
			}
		}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		if(FSKEY_NULL!=key)
		{
			mainCanvas->KeyIn(key,shift,ctrl,alt);
		}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

		int c;
		while(0!=(c=FsInkeyChar()))
		{
			mainCanvas->CharIn(c);
		}

		FsSleep(25);
	}
#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	return YsReverseBool(this->terminate);
}

void FsRunLoop::DrawMenu(void) const
{
	if(0==runModeStack.GetEnd().runModeCounter)
	{
		// Menu may not be ready.
		return;
	}

	FsClearScreenAndZBuffer(YsGrayScale(0.25));
	FsSet2DDrawing();
	if((nullptr==mainCanvas || YSTRUE!=mainCanvas->ShowConsole()) &&
	   titleBmpPtr!=NULL && titleBmpPtr->GetWidth()>0 && titleBmpPtr->GetHeight()>0)
	{
		FsDrawTitleBmp(*titleBmpPtr,YSTRUE);
	}
	else
	{
		fsConsole.Draw(NULL);
	}


	int wid,hei;
	FsGetWindowSize(wid,hei);
	if(world->SimulationIsPrepared()!=YSTRUE)
	{
		if(0==strcmp(FsOption::GetLanguageString(),FsJapaneseLanguageCode) &&
		   newFltMsgBmp.GetWidth()>0 && 
		   newFltMsgBmp.GetHeight()>0)
		{
			FsDrawBmp(newFltMsgBmp,0,hei/2+fsAsciiRenderer.GetFontHeight());
		}
	}
	else
	{
		if(YSTRUE==world->PlayerPlaneIsReady() || YSTRUE==world->PlayerGroundIsReady())
		{
			if(0==strcmp(FsOption::GetLanguageString(),FsJapaneseLanguageCode) &&
			   simFlyMsgBmp.GetWidth()>0 && 
			   simFlyMsgBmp.GetHeight()>0)
			{
				FsDrawBmp(simFlyMsgBmp,0,hei/2+fsAsciiRenderer.GetFontHeight());
			}
		}
		else
		{
			if(0==strcmp(FsOption::GetLanguageString(),FsJapaneseLanguageCode) &&
			   simRepMsgBmp.GetWidth()>0 && 
			   simRepMsgBmp.GetHeight()>0)
			{
				FsDrawBmp(simRepMsgBmp,0,hei/2+fsAsciiRenderer.GetFontHeight());
			}
		}
	}

	if(nullptr!=mainCanvas)
	{
		mainCanvas->Show();
		mainCanvas->SetNeedRedraw(YSFALSE);
	}
	FsSwapBuffers();
}

void FsRunLoop::ChangeBitmapColor(YsBitmap &bmp,const YsColor &set)
{
	unsigned char value[4];
	value[0]=(unsigned char)set.Ri();
	value[1]=(unsigned char)set.Gi();
	value[2]=(unsigned char)set.Bi();
	value[3]=(unsigned char)255;

	for(int y=0; y<bmp.GetHeight(); y++)
	{
		for(int x=0; x<bmp.GetWidth(); x++)
		{
			unsigned char *rgba;
			rgba=(unsigned char *)bmp.GetRGBAPixelPointer(x,y);
			if(rgba[3]==255)
			{
				rgba[0]=value[0];
				rgba[1]=value[1];
				rgba[2]=value[2];
			}
		}
	}
}

////////////////////////////////////////////////////////////

YSBOOL FsRunLoop::NeedRedraw(void) const
{
#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	if(0<runModeStack.GetN())
	{
		switch(runModeStack.GetEnd().runMode)
		{
		case YSRUNMODE_MENU:
			if((nullptr!=mainCanvas && YSTRUE==mainCanvas->NeedRedraw()) ||
			   YSTRUE==FsCheckWindowExposure() ||
			   YSTRUE==needRedraw)
			{
				return YSTRUE;
			}
			break;
		case YSRUNMODE_SHOWLANDINGPRACTICEINFO:
		case YSRUNMODE_FLY_REGULAR:
		case YSRUNMODE_FLY_DEMOMODE:
		case YSRUNMODE_REPLAYRECORD:
		case YSRUNMODE_FLY_CLIENTMODE:
		case YSRUNMODE_FLY_SERVERMODE:
			return YSTRUE;  // Always redraw.
		default:
			return YSTRUE;  // I don't know.  Maybe it's safe to rdraw.
		}
	}

#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	return YSFALSE;
}

void FsRunLoop::SetNeedRedraw(YSBOOL needRedraw)
{
	this->needRedraw=needRedraw;
}

void FsRunLoop::Draw(void)
{
#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	if(0<runModeStack.GetN())
	{
		switch(runModeStack.GetEnd().runMode)
		{
		case YSRUNMODE_MENU:
			DrawMenu();
			break;
		case YSRUNMODE_SHOWLANDINGPRACTICEINFO:
			if(nullptr!=showLandingPracticeInfo)
			{
				showLandingPracticeInfo->Draw();
			}
			break;
		case YSRUNMODE_FLY_REGULAR:
		case YSRUNMODE_FLY_DEMOMODE:
		case YSRUNMODE_REPLAYRECORD:
		case YSRUNMODE_FLY_CLIENTMODE:
		case YSRUNMODE_FLY_SERVERMODE:
			DrawInSimulationMode();
			break;
		default:
			break;
		}
	}
	SetNeedRedraw(YSFALSE);
#ifdef CRASHINVESTIGATION
	printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
}

