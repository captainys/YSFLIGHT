#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fs.h"
#include "graphics/common/fsopengl.h"
#include "fsfilename.h"
#include "fsplugin.h"

#ifdef WIN32
#include <windows.h>
typedef const char *(__cdecl *FSPLUGINNAMEFUNC)(void);
typedef FsPlugIn20080220 *(__cdecl *FSLINKTESTPROC20080220)(FsCallBack20080220 *);
#endif



class FsPlugInInfo
{
public:
	FsPlugInInfo();

#ifdef WIN32
	HMODULE hDll;
#endif

	YsString name;
	FsPlugIn20080220 *plugIn20080220;
};


FsPlugInInfo::FsPlugInInfo()
{
	name.Set("");
	plugIn20080220=NULL;
}


static FsCallBack20080220 fsCallBack20080220Ptr;
static YsArray <FsPlugInInfo> fsPlugInList;

void FsLoadPlugIn(void)
{
return;
	YsArray <YsWString> fileList;
	if(FsFindFileList(fileList,FsGetPlugInDir(),L"gen",L"dll")==YSOK)
	{
#ifdef WIN32
		int i;
		YsWString dllFn;
		forYsArray(i,fileList)
		{
			HMODULE hDll;

			dllFn.Set(FsGetPlugInDir());
			dllFn.Append(L"/");
			dllFn.Append(fileList[i]);
			hDll=LoadLibraryExW(dllFn,NULL,0);
			if(hDll!=NULL)
			{
				FsPlugInInfo newPlugIn;

				newPlugIn.hDll=hDll;


				FSPLUGINNAMEFUNC plugInNameProc;
				plugInNameProc=(FSPLUGINNAMEFUNC)GetProcAddress(hDll,"FsGetPlugInName");
				if(plugInNameProc!=NULL)
				{
					newPlugIn.name.Set((plugInNameProc)());
				}
				else
				{
					MessageBox(NULL,"Cannot Get Plug-In Name","Error",MB_OK);
					goto NEXTFILE;
				}


				FSLINKTESTPROC20080220 linkTestProc;
				linkTestProc=(FSLINKTESTPROC20080220)GetProcAddress(hDll,"FsLinkPlugIn20080220");
				if(linkTestProc!=NULL)
				{
					FsPlugIn20080220 *plugIn;
					plugIn=(linkTestProc)(&fsCallBack20080220Ptr);
					if(plugIn!=NULL)
					{
						newPlugIn.plugIn20080220=plugIn;
					}
				}

				if(newPlugIn.plugIn20080220!=NULL)
				{
					fsPlugInList.Append(newPlugIn);
				}
				else
				{
					MessageBox(NULL,"Link Test Failed","Err",MB_OK);
				}
			}

		NEXTFILE:
			;
		}
#else
#endif
	}
}

void FsFreePlugIn(void)
{
#ifdef WIN32
	int i;
	forYsArray(i,fsPlugInList)
	{
		FreeLibrary(fsPlugInList[i].hDll);
	}
#else
#endif
}

void FsPlugInCallInitialize(FsWorld *world)
{
	int i;
	forYsArray(i,fsPlugInList)
	{
		if(fsPlugInList[i].plugIn20080220!=NULL)
		{
			fsPlugInList[i].plugIn20080220->Initialize(world);
		}
	}
}

void FsPlugInCallInterval(const double &ctime,FsSimulation *sim)
{
	int i;
	forYsArray(i,fsPlugInList)
	{
		if(fsPlugInList[i].plugIn20080220!=NULL)
		{
			fsPlugInList[i].plugIn20080220->Interval(ctime,sim->world,sim);
		}
	}
}

void FsPlugInCallDrawForeground(const double &ctime)
{
	int i;
	forYsArray(i,fsPlugInList)
	{
		if(fsPlugInList[i].plugIn20080220!=NULL)
		{
			fsPlugInList[i].plugIn20080220->DrawForeground(ctime);
		}
	}
}

void FsPlugInCallWindowBufferSwapped(const double &ctime,const FsSimulation *sim)
{
	int i;
	forYsArray(i,fsPlugInList)
	{
		if(fsPlugInList[i].plugIn20080220!=NULL)
		{
			fsPlugInList[i].plugIn20080220->WindowBufferSwapped(ctime,sim->world,sim);
		}
	}
}

void FsPluginCallNetFreeMemory(const double &ctime,FsSimulation *sim)
{
	int i;
	forYsArray(i,fsPlugInList)
	{
		if(fsPlugInList[i].plugIn20080220!=NULL)
		{
			fsPlugInList[i].plugIn20080220->NetworkFreeMemory(ctime,sim->world,sim);
		}
	}
}




#ifdef WIN32
extern HWND FsWin32GetMainWindowHandle(void);

HWND FsCallBack20080220::GetMainWindowHandle(void)
{
	return FsWin32GetMainWindowHandle();
}
#endif

void FsCallBack20080220::GetWindowSize(int &width,int &height)
{
	FsGetWindowSize(width,height);
}

YSBOOL FsCallBack20080220::DrawingMainWindow(void)
{
	return FsIsMainWindowActive();
}

YSBOOL FsCallBack20080220::DrawingSubWindow(int subWndId)
{
	return FsIsSubWindowActive(subWndId);
}

class FsAirplane *FsCallBack20080220::GetPlayerAirplane(class FsSimulation *sim)
{
	return sim->GetPlayerAirplane();
}

int FsCallBack20080220::GetNumAirplane(class FsSimulation *sim)
{
	return sim->GetNumAirplane();
}

class FsAirplane *FsCallBack20080220::FindNextAirplane(class FsSimulation *sim,class FsAirplane *air)  // Give air=NULL to get the first airplane
{
	return sim->FindNextAirplane(air);
}

class FsAirplane *FsCallBack20080220::FindPrevAirplane(class FsSimulation *sim,class FsAirplane *air)  // Give air=NULL to get the last airplane
{
	return sim->FindPrevAirplane(air);
}

YSBOOL FsCallBack20080220::IsAirplane(class FsExistence *obj) // True -> obj can be casted to FsAirplane *
{
	if(FSEX_AIRPLANE==obj->GetType())
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsCallBack20080220::IsGround(class FsExistence *obj) // True -> obj can be casted to FsGround *
{
	if(FSEX_GROUND==obj->GetType())
	{
		return YSTRUE;
	}
	return YSFALSE;
}

const char *FsCallBack20080220::GetAirplaneName(class FsAirplane *air) // YFS name.  Not type identifier.
{
	return air->name;
}

const char *FsCallBack20080220::GetAirplaneTypeName(class FsAirplane *air) // Type name (eg. F-15_EAGLE)
{
	return air->Prop().GetIdentifier();
}

FSFLIGHTSTATE FsCallBack20080220::GetAirplaneState(class FsAirplane *air)
{
	return air->Prop().GetFlightState();
}

const YsVec3 &FsCallBack20080220::GetAirplanePosition(class FsAirplane *air)
{
	return air->GetPosition();
}

const YsAtt3 &FsCallBack20080220::GetAirplaneAttitude(class FsAirplane *air)
{
	return air->GetAttitude();
}

const YsVec3 &FsCallBack20080220::GetAirplaneSpeed(YsVec3 &velocity,class FsAirplane *air)
{
	air->Prop().GetVelocity(velocity);
	return velocity;
}

const double &FsCallBack20080220::GetAirplaneFieldElevation(class FsAirplane *air)
{
	return air->Prop().GetGroundElevation();
}

const YsVec3 &FsCallBack20080220::GetAirplaneFieldNormal(class FsAirplane *air)
{
	return air->Prop().GetGroundNormal();
}

const class FsGround *FsCallBack20080220::IsAirplaneOnCarrier(class FsSimulation *,class FsAirplane *air)
{
	return air->Prop().OnThisCarrier();
}

const double &FsCallBack20080220::GetAirplaneElevator(class FsAirplane *air)
{
	return air->Prop().GetElevator();
}

const double &FsCallBack20080220::GetAirplaneElevatorTrim(class FsAirplane *air)
{
	return air->Prop().GetElvTrim();
}

const double &FsCallBack20080220::GetAirplaneAileron(class FsAirplane *air)
{
	return air->Prop().GetAileron();
}

const double FsCallBack20080220::GetAirplaneRudder(class FsAirplane *air)
{
	return air->Prop().GetRudder();
}

const double &FsCallBack20080220::GetAirplaneThrottle(class FsAirplane *air)
{
	return air->Prop().GetThrottle();
}

YSBOOL FsCallBack20080220::GetAirplaneAfterburner(class FsAirplane *air)
{
	return air->Prop().GetAfterBurner();
}

YSBOOL FsCallBack20080220::IsFiringGun(class FsAirplane *air)
{
	if(air->Prop().IsFiringGun()==YSTRUE || air->Prop().IsFiringPilotControlledTurret()==YSTRUE)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsCallBack20080220::IsJet(class FsAirplane *air)
{
	return air->Prop().IsJet();
}

YSBOOL FsCallBack20080220::HasAfterburner(class FsAirplane *air)
{
	return air->Prop().GetHasAfterburner();
}

int FsCallBack20080220::GetNumGround(class FsSimulation *sim)
{
	return sim->GetNumGround();
}
class FsGround *FsCallBack20080220::FindNextGround(class FsSimulation *sim,class FsGround *gnd)
{
	return sim->FindNextGround(gnd);
}
class FsGround *FsCallBack20080220::FindPrevGround(class FsSimulation *sim,class FsGround *gnd)
{
	return sim->FindPrevGround(gnd);
}

const YsVec3 &FsCallBack20080220::GetGroundPosition(class FsGround *gnd)
{
	return gnd->GetPosition();
}

const YsAtt3 &FsCallBack20080220::GetGroundAttitude(class FsGround *gnd)
{
	return gnd->GetAttitude();
}

const YsVec3 &FsCallBack20080220::GetGroundSpeed(YsVec3 &velocity,class FsGround *gnd)
{
	gnd->Prop().GetVelocity(velocity);
	return velocity;
}

const char *FsCallBack20080220::GetGroundName(class FsGround *gnd) // YFS name.  Not type identifier.
{
	return gnd->name;
}

const char *FsCallBack20080220::GetGroundTypeName(class FsGround *gnd) // Type name (eg. F-15_EAGLE)
{
	return gnd->Prop().GetIdentifier();
}

const int FsCallBack20080220::GetFieldRegion(
    YsArray <FsTerrainRegionId20080220,16> &rgnList,class FsSimulation *sim,const double &x,const double &z,unsigned int flags)
{
	const FsField *fld=sim->GetField();

	if(nullptr!=fld)
	{
		YsArray <const YsSceneryRectRegion *,16> rgn;
		if(fld->GetFieldRegion(rgn,x,z)==YSOK)
		{
			int i;
			rgnList.Set(rgn.GetN(),NULL);
			forYsArray(i,rgn)
			{
				rgnList[i].rgnId=rgn[i]->GetId();
				rgnList[i].rgnTag=rgn[i]->GetTag();
				fld->GetFieldPtr()->GetOrigin(rgnList[i].cen,rgn[i]);
				if(flags&FSGETFLDRGN_SETRECT)
				{
					fld->GetFieldPtr()->GetRegionRect(rgnList[i].rect,rgn[i]);
				}
			}
			return (int)rgn.GetN();
		}
	}
	rgnList.Set(0,NULL);
	return 0;
}


void FsCallBack20080220::DrawString(int x,int y,const char str[],const YsColor &col)
{
	FsDrawString(x,y,str,col);
}

void FsCallBack20080220::DrawLine(int x1,int y1,int x2,int y2,const YsColor &col)
{
	FsDrawLine(x1,y1,x2,y2,col);
}

void FsCallBack20080220::DrawRect(int x1,int y1,int x2,int y2,const YsColor &col,YSBOOL fill)
{
	FsDrawRect(x1,y1,x2,y2,col,fill);
}

void FsCallBack20080220::DrawCircle(int x,int y,int rad,const YsColor &col,YSBOOL fill)
{
	FsDrawCircle(x,y,rad,col,fill);
}

void FsCallBack20080220::DrawPolygon(int n,int plg[],const YsColor &col)
{
	FsDrawPolygon(n,plg,col);
}

void FsCallBack20080220::DrawDiamond(int x,int y,int r,const YsColor &col,YSBOOL fill)
{
	FsDrawDiamond(x,y,r,col,fill);
}

void FsCallBack20080220::DrawX(int x,int y,int r,const YsColor &col)
{
	FsDrawX(x,y,r,col);
}

void FsCallBack20080220::DrawPoint(int x,int y,const YsColor &col)
{
	FsDrawPoint2Pix(x,y,col);
}



void FsCallBack20080220::AddMessage(class FsSimulation *sim,const char str[])
{
	sim->AddTimedMessage(str);
}
