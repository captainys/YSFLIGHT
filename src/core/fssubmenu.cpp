#include <ysclass.h>
#include <ysunitconv.h>
#include <ysport.h>

#include "fsconfig.h"

#include "fs.h"
#include "fsfilename.h"
#include "fsinstpanel.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"

#include "fspluginmgr.h"


#include "fstextresource.h"
#include "ysbitmap.h"


#ifndef WIN32  // Assuming UNIX

#include <sys/time.h>
#endif

#include <time.h>



#include "fsgui.h"
#include "fsguiselectiondialogbase.h"
#include <fsguifiledialog.h>

#include "fschoose.h"

#include "graphics/common/fsfontrenderer.h"

#include "fspersona.h"
#include "fsautodrive.h"


FsSubMenuItem::FsSubMenuItem()
{
	Initialize();
}

void FsSubMenuItem::Initialize(void)
{
	itemType=FSSUBMENUITEM_NULL;;
	appendix.Set("");
	bmp.PrepareBitmap(0,0);
}


////////////////////////////////////////////////////////////

FsSubMenu::FsSubMenu() : menuItemList(menuItemAlloc)
{
	CleanUp();
}

FsSubMenu::~FsSubMenu()
{
}

void FsSubMenu::CleanUp(void)
{
	subMenu=FSSUBMENU_NONE;
	subMenuBase=0;
	ctlNavId=0;

	menuItemList.CleanUp();

	availableComTarget.Clear();
	fuelTruckOnCallKey=-1;
}

FSSUBMENU FsSubMenu::GetSubMenu(void) const
{
	return subMenu;
}

void FsSubMenu::SetSubMenu(class FsSimulation *sim,FSSUBMENU subMenuIn)
{
	if(subMenu!=subMenuIn)
	{
		subMenu=FSSUBMENU_WAITKEYRELEASE; // tentative
		subMenuBase=0;
		PrepareSubMenu(sim,subMenuIn);
	}
}

void FsSubMenu::PrepareSubMenu(class FsSimulation *,FSSUBMENU subMenuIn)
{
	subMenu=subMenuIn;
}

void FsSubMenu::SelectNav(int navIdIn)
{
	ctlNavId=navIdIn;
}




void FsSubMenu::CheckKeyRelease(void)
{
	if(subMenu==FSSUBMENU_WAITKEYRELEASE)
	{
		if(FsGetKeyState(FSKEY_1)==YSFALSE &&
		   FsGetKeyState(FSKEY_2)==YSFALSE &&
		   FsGetKeyState(FSKEY_3)==YSFALSE &&
		   FsGetKeyState(FSKEY_4)==YSFALSE &&
		   FsGetKeyState(FSKEY_5)==YSFALSE &&
		   FsGetKeyState(FSKEY_6)==YSFALSE &&
		   FsGetKeyState(FSKEY_7)==YSFALSE &&
		   FsGetKeyState(FSKEY_8)==YSFALSE &&
		   FsGetKeyState(FSKEY_9)==YSFALSE &&
		   FsGetKeyState(FSKEY_0)==YSFALSE &&
		   FsGetKeyState(FSKEY_F)==YSFALSE &&
		   FsGetKeyState(FSKEY_BS)==YSFALSE &&
		   FsGetKeyState(FSKEY_ENTER)==YSFALSE)
		{
			subMenu=FSSUBMENU_NONE;
		}
	}
}

YSBOOL FsSubMenu::SubMenuEatRawKey(FSSUBMENU subMenu,int rawKey)
{
	if(subMenu==FSSUBMENU_INFLTCONFIG)
	   // 2005/03/19 In Flight Config captures all keys.
	   // 2005/03/22 In Flight Message captures all keys.
	{
		return YSTRUE;
	}

	if(subMenu!=FSSUBMENU_NONE && FSKEY_0<=rawKey && rawKey<=FSKEY_9)
	{
		return YSTRUE;
	}

	return YSFALSE;
}

void FsSubMenu::ProcessSubMenu(class FsSimulation *sim,class FsFlightConfig &cfg,int rawKey)
{
	FsAirplane *playerPlane=sim->GetPlayerAirplane();
	if(playerPlane!=NULL && playerPlane->isPlayingRecord!=YSTRUE)
	{
		switch(subMenu)
		{
		default:
			break;
		case FSSUBMENU_SELECTVOR:
			if(rawKey==FSKEY_ESC)
			{
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
			}
			else
			{
				switch(ctlNavId)
				{
				case 0:
				case 1:
					if(rawKey==FSKEY_0)
					{
						playerPlane->Prop().SetVorStation(ctlNavId,YSNULLHASHKEY);
						SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
					}
					else if(FSKEY_1<=rawKey && rawKey<=FSKEY_9)
					{
						YsArray <double,64> vorDist;
						YsArray <FsGround *,64> vorInRange;
						sim->MakeSortedVorList(vorInRange,vorDist,playerPlane->GetPosition());

						if(rawKey==FSKEY_9)
						{
							subMenuBase+=NSHOWMAX;
							if(vorInRange.GetN()<=subMenuBase)
							{
								subMenuBase=0;
							}
						}
						else
						{
							const int sel=subMenuBase+rawKey-FSKEY_1;
							if(0<=sel && sel<vorInRange.GetN())
							{
								unsigned gndKey=(NULL!=vorInRange[sel] ? FsExistence::GetSearchKey(vorInRange[sel]) : YSNULLHASHKEY);
								playerPlane->Prop().SetVorStation(ctlNavId,gndKey);
								SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
							}
						}
					}
					break;
				case 100:
					if(rawKey==FSKEY_0)
					{
						playerPlane->Prop().SetNdbStation(YSNULLHASHKEY);
						SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
					}
					else if(FSKEY_1<=rawKey && rawKey<=FSKEY_9)
					{
						int sel;
						YsArray <double,64> ndbDist;
						YsArray <FsGround *,64> ndbInRange;
						sim->MakeSortedNdbList(ndbInRange,ndbDist,playerPlane->GetPosition());

						if(rawKey==FSKEY_9)
						{
							subMenuBase+=NSHOWMAX;
							if(ndbInRange.GetN()<=subMenuBase)
							{
								subMenuBase=0;
							}
						}
						else
						{
							sel=subMenuBase+rawKey-FSKEY_1;
							if(0<=sel && sel<ndbInRange.GetN())
							{
								playerPlane->Prop().SetNdbStation(FsExistence::GetSearchKey(ndbInRange[sel]));
								SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
							}
						}
					}
					break;
				}
			}
			break;
		case FSSUBMENU_GUNNER:
			{
				if(rawKey==FSKEY_1)
				{
					playerPlane->Prop().SetGunnerFirePermission(YSTRUE);
					SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				}
				else if(rawKey==FSKEY_2)
				{
					playerPlane->Prop().SetGunnerFirePermission(YSFALSE);
					SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				}
				else if(rawKey==FSKEY_ESC)
				{
					SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				}
			}
			break;
		}
	}

	switch(subMenu)
	{
	default:
		break;
	case FSSUBMENU_OPENSUBWINDOW:
		if(rawKey==FSKEY_ENTER)
		{
			SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
		}
		else if(rawKey==FSKEY_1)
		{
			subMenu=FSSUBMENU_OPENLEFTSUBWINDOW;
		}
		else if(rawKey==FSKEY_2)
		{
			subMenu=FSSUBMENU_OPENRIGHTSUBWINDOW;
		}
		break;
	case FSSUBMENU_OPENLEFTSUBWINDOW:
	case FSSUBMENU_OPENRIGHTSUBWINDOW:
		{
			int windowId;
			FSSUBMENU nextViewSubmenu;
			if(subMenu==FSSUBMENU_OPENLEFTSUBWINDOW)
			{
				windowId=0;
				nextViewSubmenu=FSSUBMENU_OPENLEFTSUBWINDOWCOCKPITVIEW;
			}
		 	else // if(subMenu==FSSUBMENU_OPENRIGHTSUBWINDOW)
		 	{
				windowId=1;
				nextViewSubmenu=FSSUBMENU_OPENRIGHTSUBWINDOWCOCKPITVIEW;
			}

			YSBOOL needOpen;
			needOpen=YSFALSE;

			switch(rawKey)
			{
			case FSKEY_ENTER:
				subMenu=FSSUBMENU_OPENSUBWINDOW;
				break;
			case FSKEY_1:
				if(FsIsSubWindowOpen(windowId)==YSTRUE)
				{
					FsCloseSubWindow(windowId);
					FsSelectMainWindow();
				}
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_2:
				subMenu=nextViewSubmenu;
				break;
			case FSKEY_3:
				sim->SetSubWindowViewMode(windowId,FsSimulation::FSMYWEAPONVIEW_OLD);
				needOpen=YSTRUE;
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_4:
				sim->SetSubWindowViewMode(windowId,FsSimulation::FSMYWEAPONVIEW_NEW);
				needOpen=YSTRUE;
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_5:
				sim->SetSubWindowViewMode(windowId,FsSimulation::FSLOCKEDTARGETVIEW);
				needOpen=YSTRUE;
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			}

			if(needOpen==YSTRUE && FsIsSubWindowOpen(windowId)!=YSTRUE)
			{
				FsOpenSubWindow(windowId);
			}
		}
		break;
	case FSSUBMENU_OPENLEFTSUBWINDOWCOCKPITVIEW:
	case FSSUBMENU_OPENRIGHTSUBWINDOWCOCKPITVIEW:
		{
			int windowId;
			FSSUBMENU prevViewSubmenu;
			if(subMenu==FSSUBMENU_OPENLEFTSUBWINDOWCOCKPITVIEW)
			{
				windowId=0;
				prevViewSubmenu=FSSUBMENU_OPENLEFTSUBWINDOW;
			}
		 	else // if(subMenu==FSSUBMENU_OPENRIGHTSUBWINDOWCOCPITVIEW)
		 	{
				windowId=1;
				prevViewSubmenu=FSSUBMENU_OPENRIGHTSUBWINDOW;
			}

			YSBOOL needOpen;
			needOpen=YSFALSE;

			switch(rawKey)
			{
			case FSKEY_ENTER:
				subMenu=prevViewSubmenu;
				break;
			case FSKEY_1:
				needOpen=YSTRUE;
				sim->SetSubWindowViewMode(windowId,FsSimulation::FSBACKMIRRORVIEW);
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_2:
				needOpen=YSTRUE;
				sim->SetSubWindowViewMode(windowId,FsSimulation::FS45DEGREELEFTVIEW);
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_3:
				needOpen=YSTRUE;
				sim->SetSubWindowViewMode(windowId,FsSimulation::FS45DEGREERIGHTVIEW);
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_4:
				needOpen=YSTRUE;
				sim->SetSubWindowViewMode(windowId,FsSimulation::FS90DEGREELEFTVIEW);
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_5:
				needOpen=YSTRUE;
				sim->SetSubWindowViewMode(windowId,FsSimulation::FS90DEGREERIGHTVIEW);
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			case FSKEY_6:
				{
					FsAirplane *playerPlane=sim->GetPlayerAirplane();
					if(playerPlane!=NULL && playerPlane->Prop().FreeFallBombIsLoaded()==YSTRUE)
					{
						needOpen=YSTRUE;
						sim->SetSubWindowViewMode(windowId,FsSimulation::FSBOMBINGVIEW);
						SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
					}
				}
				break;
			case FSKEY_7:
				needOpen=YSTRUE;
				sim->SetSubWindowViewMode(windowId,FsSimulation::FSCOCKPITVIEW);
				SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
				break;
			}

			if(needOpen==YSTRUE && FsIsSubWindowOpen(windowId)!=YSTRUE)
			{
				FsOpenSubWindow(windowId);
			}
		}
		break;
	case FSSUBMENU_INFLTCONFIG:
		switch(rawKey)
		{
		case FSKEY_ENTER:
		case FSKEY_ESC:
			SetSubMenu(sim,FSSUBMENU_WAITKEYRELEASE);
			break;
		case FSKEY_1:
			cfg.smkType=FSSMKCIRCLE;
			cfg.smkRemainTime=15.0;
			cfg.smkStep=8;
			cfg.drawShadow=YSFALSE;
			cfg.drawOrdinance=YSFALSE;
			cfg.drawTransparency=YSFALSE;
			cfg.drawTransparentVapor=YSFALSE;
			cfg.drawTransparentSmoke=YSFALSE;
			cfg.drawTransparentLater=YSFALSE;
			cfg.airLod=2;
			cfg.gndLod=2;
			cfg.shadowOfDeadAirplane=YSFALSE;
			cfg.drawCoarseOrdinance=YSFALSE;
			cfg.zbuffQuality=0;
			break;
		case FSKEY_2:
			cfg.smkType=FSSMKTOWEL;
			cfg.smkRemainTime=30.0;
			cfg.smkStep=4;
			cfg.drawShadow=YSTRUE;
			cfg.drawOrdinance=YSTRUE;
			cfg.drawTransparency=YSTRUE;
			cfg.drawTransparentVapor=YSFALSE;
			cfg.drawTransparentSmoke=YSFALSE;
			cfg.drawTransparentLater=YSFALSE;
			cfg.airLod=0;
			cfg.gndLod=0;
			cfg.shadowOfDeadAirplane=YSTRUE;
			cfg.drawCoarseOrdinance=YSFALSE;
			cfg.zbuffQuality=1;
			break;
		case FSKEY_3:
			cfg.smkType=FSSMKSOLID;
			cfg.smkRemainTime=60.0;
			cfg.smkStep=1;
			cfg.drawShadow=YSTRUE;
			cfg.drawOrdinance=YSTRUE;
			cfg.drawTransparency=YSTRUE;
			cfg.drawTransparentVapor=YSTRUE;
			cfg.drawTransparentSmoke=YSTRUE;
			cfg.drawTransparentLater=YSTRUE;
			cfg.airLod=1;
			cfg.gndLod=1;
			cfg.shadowOfDeadAirplane=YSTRUE;
			cfg.drawCoarseOrdinance=YSFALSE;
			cfg.zbuffQuality=2;
			break;
		case FSKEY_D:
			YsFlip(cfg.shadowOfDeadAirplane);
			break;
		case FSKEY_A:
			cfg.airLod=(cfg.airLod+1)%4;
			break;
		case FSKEY_G:
			cfg.gndLod=(cfg.gndLod+1)%3;
			break;
		case FSKEY_W:
			YsFlip(cfg.drawCoarseOrdinance);
			break;
		case FSKEY_S:
			YsFlip(cfg.drawShadow);
			break;
		case FSKEY_L:
			YsFlip(cfg.drawLightsInDaylight);
			break;
		case FSKEY_Z:
			cfg.zbuffQuality=(cfg.zbuffQuality+1)&3;
			break;
		case FSKEY_J:
			YsFlip(cfg.drawVirtualJoystick);
			break;
		case FSKEY_T:
			YsFlip(cfg.drawTransparency);
			cfg.drawTransparentSmoke=cfg.drawTransparency;
			cfg.drawTransparentVapor=cfg.drawTransparency;
			break;
		case FSKEY_P:
			YsFlip(cfg.drawTransparentLater);
			break;
		case FSKEY_Q:
			YsFlip(cfg.useParticle);
			break;
		case FSKEY_M:
			switch(cfg.smkType)
			{
			default:
				break;
			case FSSMKCIRCLE:
				cfg.smkType=FSSMKNOODLE;
				cfg.useParticle=YSFALSE;
				break;
			case FSSMKNOODLE:
				cfg.smkType=FSSMKTOWEL;
				cfg.useParticle=YSFALSE;
				break;
			case FSSMKTOWEL:
				cfg.smkType=FSSMKSOLID;
				cfg.useParticle=YSFALSE;
				break;
			case FSSMKSOLID:
				cfg.smkType=FSSMKCIRCLE;
				cfg.useParticle=YSFALSE;
				break;
			}
			break;
		case FSKEY_H:
			YsFlip(cfg.useHudAlways);
			break;
		case FSKEY_U:
			YsFlip(cfg.useSimpleHud);
			break;
		case FSKEY_C:
			sim->FlipShowUserNameMasterSwitch();
			break;
		case FSKEY_F:
			YsFlip(cfg.showFps);
			break;
		case FSKEY_I:
			YsFlip(cfg.showIAS);
			break;
		case FSKEY_R:
			YsFlip(cfg.displayTextWarnings);
			break;
		}
		break;
	}
}

void FsSubMenu::Draw(const class FsSimulation *sim,class FsFlightConfig &cfg,int &sx,int &sy) const
{
	const FsAirplane *playerPlane=sim->GetPlayerAirplane();

	switch(subMenu)
	{
	case FSSUBMENU_OPENSUBWINDOW:
		FsDrawString(sx,sy,"Sub-Window",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"1:Left Sub-Window  2:Right Sub-Window",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"Enter: Back",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		break;
	case FSSUBMENU_OPENLEFTSUBWINDOW:
	case FSSUBMENU_OPENRIGHTSUBWINDOW:
		int windowId;
		if(subMenu==FSSUBMENU_OPENLEFTSUBWINDOW)
		{
			FsDrawString(sx,sy,"Left Sub-Window",YsWhite());
			windowId=0;
			sy+=fsAsciiRenderer.GetFontHeight();
		}
		else // if(subMenu==FSSUBMENU_OPENRIGHTSUBWINDOW)
		{
			FsDrawString(sx,sy,"Right Sub-Window",YsWhite());
			windowId=1;
			sy+=fsAsciiRenderer.GetFontHeight();
		}
		if(FsIsSubWindowOpen(windowId)==YSTRUE)
		{
			FsDrawString(sx,sy,"1. Close",YsWhite());
		}
		else
		{
			FsDrawString(sx,sy,"1. ------------",YsWhite());
		}
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"2. Cockpit View",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"3. Player Weapon View (Oldest)",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"4. Player Weapon View (Newest)",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"5. Locked Target View",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"Enter: Back",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		break;
	case FSSUBMENU_OPENLEFTSUBWINDOWCOCKPITVIEW:
	case FSSUBMENU_OPENRIGHTSUBWINDOWCOCKPITVIEW:
		if(subMenu==FSSUBMENU_OPENLEFTSUBWINDOWCOCKPITVIEW)
		{
			FsDrawString(sx,sy,"Cockpit View in Left Sub-Window",YsWhite());
			sy+=fsAsciiRenderer.GetFontHeight();
		}
		else if(subMenu==FSSUBMENU_OPENRIGHTSUBWINDOWCOCKPITVIEW)
		{
			FsDrawString(sx,sy,"Cockpit View in Right Sub-Window",YsWhite());
			sy+=fsAsciiRenderer.GetFontHeight();
		}
		FsDrawString(sx,sy,"1. Rear Camera",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"2. 45 Degree Left",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"3. 45 Degree Right",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"4. 90 Degree Left",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"5. 90 Degree Right",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();

		if(playerPlane!=NULL && playerPlane->Prop().FreeFallBombIsLoaded()>0)
		{
			FsDrawString(sx,sy,"6. Bombing View",YsWhite());
		}
		else
		{
			FsDrawString(sx,sy,"6. ------------",YsWhite());
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		FsDrawString(sx,sy,"7. Forward View",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();

		FsDrawString(sx,sy,"Enter: Back",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		break;
	case FSSUBMENU_SELECTVOR:
		if(playerPlane!=NULL)
		{
			int i;
			char buf[256];
			YsArray <double,64> stationDist;
			YsArray <FsGround *,64> stationInRange;
			switch(ctlNavId)
			{
			case 0:
			case 1:
				sprintf(buf,"STATION FOR NAV %d",ctlNavId+1);
				FsDrawString(sx,sy,buf,YsYellow());
				sy+=fsAsciiRenderer.GetFontHeight();

				FsDrawString(sx,sy,"0. Select None",YsWhite());
				sy+=fsAsciiRenderer.GetFontHeight();

				sim->MakeSortedVorList(stationInRange,stationDist,playerPlane->GetPosition());

				for(i=0; subMenuBase+i<stationInRange.GetN() && i<NSHOWMAX; i++)
				{
					int vorId;
					vorId=subMenuBase+i;
					if(stationInRange[vorId]->Prop().GetAircraftCarrierProperty()!=NULL)
					{
						sprintf(buf,"%d. ILS [%s] (%.0lf nautical miles)",i+1,(const char *)stationInRange[vorId]->name,YsUnitConv::MtoNM(stationDist[vorId]));
					}
					else
					{
						sprintf(buf,"%d. [%s] (%.0lf nautical miles)",i+1,(const char *)stationInRange[vorId]->name,YsUnitConv::MtoNM(stationDist[vorId]));
					}
					FsDrawString(sx,sy,buf,YsWhite());
					sy+=fsAsciiRenderer.GetFontHeight();
				}

				if(subMenuBase+NSHOWMAX<stationInRange.GetN())
				{
					FsDrawString(sx,sy,"9. Next Page",YsWhite());
					sy+=fsAsciiRenderer.GetFontHeight();
				}

				break;
			case 100:
				FsDrawString(sx,sy,"NDB STATION",YsYellow());
				sy+=fsAsciiRenderer.GetFontHeight();

				FsDrawString(sx,sy,"0. Select None",YsWhite());
				sy+=fsAsciiRenderer.GetFontHeight();

				sim->MakeSortedNdbList(stationInRange,stationDist,playerPlane->GetPosition());

				for(i=0; i<stationInRange.GetN() && i<NSHOWMAX; i++)
				{
					int ndbId;
					ndbId=subMenuBase+i;
					sprintf(buf,"%d. [%s]",i+1,(const char *)stationInRange[ndbId]->name);
					FsDrawString(sx,sy,buf,YsWhite());
					sy+=fsAsciiRenderer.GetFontHeight();
				}

				if(subMenuBase+NSHOWMAX<stationInRange.GetN())
				{
					FsDrawString(sx,sy,"9. Next Page",YsWhite());
					sy+=fsAsciiRenderer.GetFontHeight();
				}

				break;
			case 200:
				FsDrawString(sx,sy,"HEADING BUG",YsYellow());
				sy+=fsAsciiRenderer.GetFontHeight();
				break;
			}
		}
		break;
	case FSSUBMENU_INFLTCONFIG:
		FsDrawString(sx,sy,"Enter: Back",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();

		FsDrawString(sx,sy,"1: Fast Mode",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"2: Default Mode",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"3: Detailed Mode",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"----------------",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.airLod)
		{
		default:
			FsDrawString(sx,sy,"A: Airplane Graphics Quality",YsWhite());
			break;
		case 0:
			FsDrawString(sx,sy,"A: Airplane Graphics Quality (Now:AUTO)",YsWhite());
			break;
		case 1:
			FsDrawString(sx,sy,"A: Airplane Graphics Quality (Now:DETAILED)",YsWhite());
			break;
		case 2:
			FsDrawString(sx,sy,"A: Airplane Graphics Quality (Now:FAST)",YsWhite());
			break;
		case 3:
			FsDrawString(sx,sy,"A: Airplane Graphics Quality (Now:VERY FAST)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.gndLod)
		{
		default:
			FsDrawString(sx,sy,"G: Ground Object Graphics Quality",YsWhite());
			break;
		case 0:
			FsDrawString(sx,sy,"G: Ground Object Graphics Quality (Now:AUTO)",YsWhite());
			break;
		case 1:
			FsDrawString(sx,sy,"G: Ground Object Graphics Quality (Now:DETAILED)",YsWhite());
			break;
		case 2:
			FsDrawString(sx,sy,"G: Ground Object Graphics Quality (Now:FAST)",YsWhite());
			break;
		case 3:
			FsDrawString(sx,sy,"G: Ground Object Graphics Quality (Now:VERY FAST)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.drawCoarseOrdinance)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"W: Weapon Graphics Quality (Now:FAST)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"W: Weapon Graphics Quality (Now:NORMAL)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.drawShadow)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"S: Shadow Graphics (Now:AUTO)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"S: Shadow Graphics (Now:FAST)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.shadowOfDeadAirplane)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"D: Shadow of Dead Airplanes (Now:Draw)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"D: Shadow of Dead Airplanes (Now:Don't Draw)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.drawLightsInDaylight)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"L: Runway/City Lights in Daylight (Now: Draw)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"L: Runway/City Lights in Daylight (Now: Don't Draw)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		if(YSTRUE==cfg.drawLightsInDaylight)
		{
			YsString str;
			str.Printf("   When visibility is lower than %.1lf miles.",
			    YsUnitConv::MtoSM(cfg.drawLightsInDaylightVisibilityThr));
			FsDrawString(sx,sy,str,YsWhite());
			sy+=fsAsciiRenderer.GetFontHeight();
		}

		switch(cfg.useHudAlways)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"H: Always Use Head Up Display (Now: On)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"H: Always Use Head Up Display (Now: Off)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.useSimpleHud)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"U: Use Simple Head Up Display (Now: On)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"U: Use Simple Head Up Display (Now: Off)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch(cfg.drawVirtualJoystick)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"J: Virtual Joystick (Now: Draw)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"J: Virtual Joystick (Now: Don't Draw)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();


		switch(cfg.useParticle)
		{
		default:
			break;
		case YSTRUE:
			FsDrawString(sx,sy,"Q: Use Particle Rendering (Now: On)",YsWhite());
			break;
		case YSFALSE:
			FsDrawString(sx,sy,"Q: Use Particle Rendering (Now: Off)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();


		if(YSTRUE==cfg.useParticle)
		{
			FsDrawString(sx,sy,"M: Smoke Type (Now: Particle)",YsWhite());
		}
		else
		{
			switch(cfg.smkType)
			{
			default:
				FsDrawString(sx,sy,"M: Smoke Type",YsWhite());
				break;
			case FSSMKCIRCLE:
				FsDrawString(sx,sy,"M: Smoke Type (Now: DOT)",YsWhite());
				break;
			case FSSMKNOODLE:
				FsDrawString(sx,sy,"M: Smoke Type (Now: NOODLE)",YsWhite());
				break;
			case FSSMKTOWEL:
				FsDrawString(sx,sy,"M: Smoke Type (Now: TOWEL)",YsWhite());
				break;
			case FSSMKSOLID:
				FsDrawString(sx,sy,"M: Smoke Type (Now: SOLID)",YsWhite());
				break;
			}
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		{
			switch(cfg.zbuffQuality)
			{
			default:
				FsDrawString(sx,sy,"Z: Z-Buffer Quality",YsWhite());
				break;
			case 0:
				FsDrawString(sx,sy,"Z: Z-Buffer Quality (Now: FAST)",YsWhite());
				break;
			case 1:
				FsDrawString(sx,sy,"Z: Z-Buffer Quality (Now: MEDIUM)",YsWhite());
				break;
			case 2:
				FsDrawString(sx,sy,"Z: Z-Buffer Quality (Now: DETAILED)",YsWhite());
				break;
			case 3:
				FsDrawString(sx,sy,"Z: Z-Buffer Quality (Now: VERY DETAILED)",YsWhite());
				break;
			}
			sy+=fsAsciiRenderer.GetFontHeight();

			switch(cfg.drawTransparency)
			{
			default:
				break;
			case YSTRUE:
				FsDrawString(sx,sy,"T: Transparency (Now: ON)",YsWhite());
				break;
			case YSFALSE:
				FsDrawString(sx,sy,"T: Transparency (Now: OFF)",YsWhite());
				break;
			}
			sy+=fsAsciiRenderer.GetFontHeight();

			switch(cfg.drawTransparentLater)
			{
			default:
				break;
			case YSTRUE:
				FsDrawString(sx,sy,"P: Reduce Peep Hole Effect by Transparent Polygon (Now: ON)",YsWhite());
				break;
			case YSFALSE:
				FsDrawString(sx,sy,"P: Reduce Peep Hole Effect by Transparent Polygon  (Now: OFF)",YsWhite());
				break;
			}
			sy+=fsAsciiRenderer.GetFontHeight();

			switch(cfg.showFps)
			{
			default:
				break;
			case YSTRUE:
				FsDrawString(sx,sy,"F: Show Frames Per Second (Now: ON)",YsWhite());
				break;
			case YSFALSE:
				FsDrawString(sx,sy,"F: Show Frames Per Second (Now: OFF)",YsWhite());
				break;
			}
			sy+=fsAsciiRenderer.GetFontHeight();
		}

		if(cfg.showUserName!=2)
		{
			switch(sim->GetShowUserNameMasterSwitch())
			{
			default:
				break;
			case YSTRUE:
				FsDrawString(sx,sy,"C: Show User Name and Circle (Now: ON)",YsWhite());
				break;
			case YSFALSE:
				FsDrawString(sx,sy,"C: Show User Name and Circle (Now: OFF)",YsWhite());
				break;
			}
			sy+=fsAsciiRenderer.GetFontHeight();
		}

		switch(cfg.showIAS)
		{
		case YSTRUE:
			FsDrawString(sx,sy,"I: Speed Indication (Now: IAS)",YsWhite());
			break;
		default:
		case YSFALSE:
			FsDrawString(sx,sy,"I: Speed Indication (Now: TAS)",YsWhite());
			break;
		}
		sy+=fsAsciiRenderer.GetFontHeight();

		switch (cfg.displayTextWarnings)
		{
		case YSTRUE:
			FsDrawString(sx, sy, "R: Display Text Warnings (Now: ON)", YsWhite());
			break;
		default:
		case YSFALSE:
			FsDrawString(sx, sy, "R: Display Text Warnings (Now: OFF)", YsWhite());
			break;
		}
		sy += fsAsciiRenderer.GetFontHeight();

		break;
	case FSSUBMENU_GUNNER:
		FsDrawString(sx,sy,"1..... Fire at will",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"2..... Ceasefire",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		FsDrawString(sx,sy,"ESC... (Say nothing to the gunner)",YsWhite());
		sy+=fsAsciiRenderer.GetFontHeight();
		break;
	default:
		break;
	}
}
