#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <ysclass.h>
#include <ysport.h>
#include <fssimplewindow.h>

#include "fsconfig.h"

#include "fs.h"
#include "fsradar.h"
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



YSRESULT FsSimulation::PrepareRunDemoMode(FsDemoModeInfo &info,const char sysMsg[],const double &maxTime)
{
	info.maxTime=maxTime;

	strcpy(systemMessage,sysMsg);

	ClearTimedMessage();

	PrepareRunSimulation();

	if(GetPlayerAirplane()==NULL)
	{
		SetPlayerAirplane(0);
	}

	if(info.type==FSDEMO_DOGFIGHT)
	{
		mainWindowViewmode=FSAIRTOAIRVIEW;
		DemoModeReconsiderViewTarget(focusAir,focusAir2);
		DemoModeReconsiderPlayerAirplane(focusAir,focusAir2);
		info.nextReconsiderViewTargetTime=0.0;
	}
	else if(info.type==FSDEMO_LANDING)
	{
		DemoModeReconsiderLandingViewMode(info);
		info.nextReconsiderViewTargetTime=info.changeViewPointInterval;
	}
	else if(info.type==FSDEMO_ACROBAT)
	{
		DemoModeReconsiderAcrobatViewMode(info);
		info.nextReconsiderViewTargetTime=info.changeViewPointInterval+(double)(rand()%400-200)/100.0;
	}
	else if(info.type==FSDEMO_CONCORDEFLYBY)
	{
		DemoModeReconsiderConcordeFlyByViewMode(info);
		info.nextReconsiderViewTargetTime=info.changeViewPointInterval;
	}

	info.endTime=0.0;

	info.terminateByUser=YSFALSE;

	return YSOK;
}

YSBOOL FsSimulation::DemoModeOneStep(FsDemoModeInfo &info,YSBOOL drawSmokeAndVapor,YSBOOL preserveFlightRecord)
{
	cfgPtr->externalCameraDelay=YSTRUE;  // 2005/03/18

	if(info.type==FSDEMO_ACROBAT)
	{
		cfgPtr->drawPlayerNameAlways=YSFALSE;
	}

	if((info.endTime<YsTolerance || currentTime<info.endTime) &&
	   (info.maxTime<YsTolerance || currentTime<info.maxTime))
	{
		int fskey;

		fskey=FsInkey();
		if(fskey!=FSKEY_NULL && fskey!=FSKEY_ALT)
		{
			info.terminateByUser=YSTRUE;
			return YSFALSE;
		}

		int mx,my;
		YSBOOL lb,mb,rb;
		FsGetMouseEvent(lb,mb,rb,mx,my);
		if(lb==YSTRUE || rb==YSTRUE)
		{
			info.terminateByUser=YSTRUE;
			return YSFALSE;
		}

		if(2<=FsGetNumCurrentTouch())
		{
			info.terminateByUser=YSTRUE;
			return YSFALSE;
		}



		double passedTime;
		passedTime=PassedTime();

		if(info.nextReconsiderViewTargetTime>YsTolerance &&
		   info.nextReconsiderViewTargetTime<currentTime)
		{
			switch(info.type)
			{
			case FSDEMO_DOGFIGHT:
				if(mainWindowViewmode==FSAIRTOAIRVIEW)
				{
					mainWindowViewmode=FSAIRFROMAIRVIEW;
					if(DemoModeReconsiderViewTarget(focusAir,focusAir2)!=YSOK)
					{
						return YSFALSE;
					}
					DemoModeReconsiderPlayerAirplane(focusAir,focusAir2);
					info.nextReconsiderViewTargetTime=currentTime+5.0;
				}
				else
				{
					mainWindowViewmode=FSAIRTOAIRVIEW;
					info.nextReconsiderViewTargetTime=0.0;
				}
				break;
			case FSDEMO_LANDING:
				DemoModeReconsiderLandingViewMode(info);
				info.nextReconsiderViewTargetTime=currentTime+info.changeViewPointInterval;
				break;
			case FSDEMO_ACROBAT:
				DemoModeReconsiderAcrobatViewMode(info);
				info.nextReconsiderViewTargetTime=currentTime+info.changeViewPointInterval;
				break;
			case FSDEMO_CONCORDEFLYBY:
				DemoModeReconsiderConcordeFlyByViewMode(info);
				info.nextReconsiderViewTargetTime=currentTime+info.changeViewPointInterval;
				break;
			}
		}

		SimulateOneStep(passedTime,YSTRUE,drawSmokeAndVapor,YSFALSE,YSFALSE,FSUSC_ENABLE/*YSTRUE*/,YSFALSE);

		if(preserveFlightRecord!=YSTRUE)
		{
			DemoModeRipOffEarlyPartOfRecord();
		}

		if(info.type==FSDEMO_DOGFIGHT)
		{
			const FsAirplane *air1,*air2;
			air1=focusAir;
			air2=focusAir2;

			if(info.nextReconsiderViewTargetTime<YsTolerance &&
			   (air1->Prop().IsActive()!=YSTRUE || air2->Prop().IsActive()!=YSTRUE))
			{
				info.nextReconsiderViewTargetTime=currentTime+8.0;
			}

			if(DemoModeOneSideWon()==YSTRUE && info.endTime<YsTolerance)
			{
				info.endTime=currentTime+10.0;
			}
		}
		else if(info.type==FSDEMO_LANDING)
		{
			FsAirplane *player;
			player=GetPlayerAirplane();
			if(player==NULL)
			{
				info.terminateByUser=YSFALSE;
				return YSFALSE;
			}

			FsLandingAutopilot *ap;
			ap=(FsLandingAutopilot *)player->GetAutopilot();
			if(ap==NULL || ap->Type()!=FSAUTOPILOT_LANDING)
			{
				info.terminateByUser=YSFALSE;
				return YSFALSE;
			}

			if(ap->landingPhase==FsLandingAutopilot::PHASE_CLEARINGRUNWAY_WITH_TAXIPATH /*10*/ || player->Prop().GetVelocity()<=0.5)
			{
				info.terminateByUser=YSFALSE;
				return YSFALSE;
			}

			if(info.endTime<=YsTolerance)
			{
				if(player->Prop().IsActive()!=YSTRUE)
				{
					info.endTime=currentTime+3.0;
				}
			}
		}
		else if(info.type==FSDEMO_ACROBAT)
		{
			FsAirplane *air;
			FsAirshowControl *ap;
			air=FindNextAirplane(NULL);
			if(air!=NULL && (ap=FsAirshowControl::GetAirshowAP(air))!=NULL && ap->endOfAction==YSTRUE)
			{
				info.terminateByUser=YSFALSE;
				return YSFALSE;
			}

			if(info.endTime<YsTolerance)
			{
				air=NULL;
				while((air=FindNextAirplane(air))!=NULL)
				{
					if(air->Prop().IsActive()!=YSTRUE)
					{
						info.endTime=currentTime+3.0;
						AddTimedMessage("An airplane lost control! Aborting the maneuver!");
					}
				}
			}
		}
		else if(info.type==FSDEMO_CONCORDEFLYBY)
		{
			cfgPtr->neverDrawAirplaneContainer=YSTRUE;
			ConcordeFlyByOneStep(info,preserveFlightRecord);
		}

		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsSimulation::ConcordeFlyByOneStep(FsDemoModeInfo &info,YSBOOL /*record*/)
{
	if(info.formation[0]->Prop().IsActive()!=YSTRUE)
	{
		info.endTime=currentTime+0.5;
		return YSFALSE;
	}


	FsTakeOffAutopilot *to;
	FsGotoPosition *gp;
	FsLandingAutopilot *ld;
	YsVec3 dif;
	int i;

	FsAirplane *playerPlane=GetPlayerAirplane();
	switch(info.concFlyByType)
	{
	case 0:  // LHR->London->LHR
		switch(info.concFlyByStep)
		{
		case 0:  // Start
			to=FsTakeOffAutopilot::Create();
			to->desigAlt=300.0;

			if(GetRegionCenterByTag(info.showCenter,"BUCKINGHAM_SHOWCENTER")==YSOK)
			{
				info.showCenter.SetY(500.0);
			}
			else
			{
				return YSFALSE;
			}

			info.formation[0]->SetAutopilot(to);
			info.concFlyByStep++;
			break;
		case 1:  // Taking off
			if(info.formation[0]->GetPosition().y()>=150.0)
			{
				gp=FsGotoPosition::Create();
				gp->SetSingleDestination(info.showCenter);
				gp->speed=playerPlane->Prop().GetEstimatedLandingSpeed()*2.5;
				gp->minAlt=0.0;
				gp->straightFlightMode=YSFALSE;
				info.formation[0]->SetAutopilot(gp);
				info.concFlyByStep++;
			}
			break;
		case 2:  // Buckingham Fly-By
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);
			if(dif.GetLength()<12000.0)
			{
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				if(gp->destination.GetN()>0)
				{
					gp->destination[0].SetY(200.0);
				}
				gp->speed=playerPlane->Prop().GetEstimatedLandingSpeed()*2.0;
			}
			if(dif.GetLength()<200.0)
			{
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				gp->straightFlightMode=YSTRUE;
				info.concFlyByStep++;
			}
			break;
		case 3:  // Clear the region
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);
			if(dif.GetLength()>500.0)
			{
				if(GetRegionCenterByTag(info.showCenter,"LHR-TOWER")==YSOK)
				{
					info.showCenter.SetY(450.0); // info.showCenter.SetY(900.0);
				}
				else
				{
					return YSFALSE;
				}
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				gp->straightFlightMode=YSFALSE;
				gp->speed=playerPlane->Prop().GetEstimatedLandingSpeed()*3.0;
				gp->SetSingleDestination(info.showCenter);
				info.concFlyByStep++;
			}
			break;
		case 4:  // Landing
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);
			if(info.formation[0]->GetPosition().y()>=400.0)
			{
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				gp->speed=playerPlane->Prop().GetEstimatedLandingSpeed()*2.0;
			}
			if(dif.GetLength()<20000.0)
			{
				ld=FsLandingAutopilot::Create();
				ld->autoClearRunway=YSFALSE;
				info.formation[0]->SetAutopilot(ld);
				info.concFlyByStep++;
			}
			break;
		case 5:
			if(info.formation[0]->Prop().GetFlightState()==FSGROUNDSTATIC &&
			   info.endTime<YsTolerance)
			{
				info.endTime=currentTime+1.0;
			}
			break;
		}
		break;

	case 1:
		if(info.formation[0]->Prop().GetFlightState()==FSGROUNDSTATIC &&
		   info.endTime<YsTolerance)
		{
			info.endTime=currentTime+1.0;
		}
		break;

	case 2:
		switch(info.concFlyByStep)
		{
		case 0:
		case 2:
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);
			if(dif.GetSquareLength()<2000.0*2000.0)
			{
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				gp->straightFlightMode=YSTRUE;
				info.concFlyByStep++;
			}
			break;
		case 1:
		case 3:
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);

			if(dif.GetSquareLength()<1200.0*1200.0)
			{
				for(i=0; i<9; i++)
				{
					info.solo[i]->Prop().SetSmokeButton(YSTRUE);
				}
			}
			else
			{
				for(i=0; i<9; i++)
				{
					info.solo[i]->Prop().SetSmokeButton(YSFALSE);
				}
			}
			if(dif.GetSquareLength()>5000.0*5000.0)
			{
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				gp->straightFlightMode=YSFALSE;
				info.concFlyByStep++;
			}
			if(info.concFlyByStep==3 &&
			   dif.GetSquareLength()>2000.0*2000.0 &&
			   info.endTime<YsTolerance)
			{
				info.endTime=currentTime+1.0;
			}
			break;
		}
		break;

	case 3:  // Triple landing to LHR
		switch(info.concFlyByStep)
		{
		case 0:
			info.nextActionTime=currentTime+5.0;
			info.concFlyByStep++;
			break;
		case 1:
		case 2:
		case 3:
			if(info.nextActionTime<=currentTime)
			{
				YsVec3 dst,ilsPos;
				FsAirplane *air;
				FsGotoPosition *gp;
				air=info.formation[info.concFlyByStep-1];
				gp=(FsGotoPosition *)air->GetAutopilot();

				ilsPos=info.refObj->GetPosition()-air->GetPosition();
				air->GetAttitude().MulInverse(ilsPos,ilsPos);
				if(ilsPos.x()>0.0)
				{
					dst.Set(100000.0,0.0,0.0);
				}
				else
				{
					dst.Set(-100000.0,0.0,0.0);
				}
				air->GetAttitude().Mul(dst,dst);
				dst+=air->GetPosition();
				gp->SetSingleDestination(dst);
				gp->forcedTurn=0;  // <- Because I'm re-using the autopilot, I need to reset it.
				gp->straightFlightMode=YSFALSE;

				info.concFlyByStep++;

				info.nextActionTime=currentTime+30.0;
			}
			// Don't break and fall down to case 4
		case 4:
			for(i=0; i<3; i++)
			{
				FsAutopilot *ap;
				FsGotoPosition *gp;
				ap=info.formation[i]->GetAutopilot();
				if(ap->Type()==FSAUTOPILOT_GOTO)
				{
					YsVec3 relPos;
					relPos=info.formation[i]->GetPosition()-info.refObj->GetPosition();
					info.refObj->GetAttitude().MulInverse(relPos,relPos);
					if(fabs(relPos.x())<1800.0)
					{
						gp=(FsGotoPosition *)ap;
						if(gp->destination.GetN()>0)
						{
							gp->destination[0].SetX(info.refObj->GetPosition().x());
							gp->destination[0].SetZ(info.refObj->GetPosition().z());
						}
						gp->forcedTurn=0;
					}

					YsVec3 ilsPos;
					YsAtt3 att;
					att=info.formation[i]->GetAttitude();
					att.SetB(0.0);
					ilsPos=info.refObj->GetPosition()-info.formation[i]->GetPosition();
					att.MulInverse(ilsPos,ilsPos);
					if(ilsPos.z()>0.0 && fabs(ilsPos.x()/ilsPos.z())<tan(YsPi/6.0))
					{
						FsLandingAutopilot *ld;
						ld=FsLandingAutopilot::Create();
						ld->SetAirplaneInfo(*info.formation[i],YsPi/2.0);
						ld->SetIls(*info.formation[i],this,info.refObj);
						info.formation[i]->SetAutopilot(ld);
					}
				}
			}

			if(info.endTime<=YsTolerance &&
			   info.formation[0]->Prop().GetFlightState()==FSGROUNDSTATIC)
			{
				info.endTime=currentTime+1.0;
			}

			break;
		}
		break;

	case 4:  // Gatwick -> Heathrow
		switch(info.concFlyByStep)
		{
		case 0:
			if(info.formation[0]->GetPosition().y()>=1300.0)
			{
				FsGotoPosition *gp;
				gp=FsGotoPosition::Create();
				gp->SetSingleDestination(info.showCenter);
				gp->speed=info.formation[0]->Prop().GetEstimatedLandingSpeed()*3.0;
				gp->SetSingleDestination(info.showCenter);
				gp->destination[0].SetY(1500.0);
				gp->minAlt=0.0;
				gp->straightFlightMode=YSFALSE;
				info.formation[0]->SetAutopilot(gp);
				info.formation[0]->Prop().SetThrustVector(0.0);

				info.concFlyByStep++;
			}
			break;
		case 1:
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);
			if(dif.GetSquareLength()<YsSqr(30000.0))
			{
				FsGotoPosition *gp;
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				gp->speed=info.formation[0]->Prop().GetEstimatedLandingSpeed()*2.0;
				if(gp->destination.GetN()>0)
				{
					gp->destination[0].SetY(915.0);
				}
				info.concFlyByStep++;
			}
			break;
		case 2:
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);
			if(dif.GetSquareLength()<YsSqr(22000.0))
			{
				FsGotoPosition *gp;
				gp=(FsGotoPosition *)info.formation[0]->GetAutopilot();
				gp->speed=info.formation[0]->Prop().GetEstimatedLandingSpeed()*1.2;
				info.concFlyByStep++;
			}
			break;
		case 3:
			dif=info.formation[0]->GetPosition()-info.showCenter;
			dif.SetY(0.0);
			if(dif.GetSquareLength()<YsSqr(15000.0))
			{
				FsLandingAutopilot *ld;
				ld=FsLandingAutopilot::Create();
				ld->SetAirplaneInfo(*info.formation[0],YsPi/2.0);
				ld->SetIls(*info.formation[0],this,info.refObj);
				ld->autoClearRunway=YSFALSE;
				info.formation[0]->SetAutopilot(ld);
				info.concFlyByStep++;
				info.formation[0]->Prop().SetThrustVector(0.4);
			}
			break;
		case 4:
			if(info.endTime<=YsTolerance &&
			   info.formation[0]->Prop().GetFlightState()==FSGROUNDSTATIC)
			{
				info.endTime=currentTime+1.0;
			}
			break;
		}
		break;

	case 5:  // High altitude supersonic cruise
	case 6:  // Low altitude supersonic cruise
		if(info.endTime<=YsTolerance)
		{
			info.endTime=currentTime+90.0;
		}
		break;

	// case 7,8,9 are for landing and rock wing passes

	case 10:  // Formation with Spitfire + Hurricane
	case 11:  // Formation with Spitfire + Hurricane + Lancaster
		if(info.endTime<=YsTolerance)
		{
			info.endTime=currentTime+50.0;
		}
		break;
	}

	return YSTRUE;
}

YSRESULT FsSimulation::AfterDemoMode(FsDemoModeInfo &)
{
	AfterSimulation();

	// 2001/07/22 Reset key buffer
	// Let it be at the end of demo mode.  The user
	// may want to press ESC key before opening demo starts,
	// in order to skip it.
	ClearKeyBuffer();

	return YSOK;
}

YSRESULT FsSimulation::DemoModeReconsiderViewTarget(const FsAirplane *&fromAirplane,const FsAirplane *&toAirplane)
{
	FsAirplane *winnerPtr,*victimPtr,*air;
	FsAutopilot *ap;
	FsDogfight *df;
	double maxg;


	winnerPtr=NULL;
	victimPtr=NULL;

	// Find Active and Highest-G capable airplane
	maxg=0.0;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->Prop().IsActive()==YSTRUE &&
		   (ap=air->GetAutopilot())!=NULL &&
		   ap->Type()==FSAUTOPILOT_DOGFIGHT)
		{
			df=(FsDogfight *)ap;
			if(df->gLimit>maxg || (df->gLimit==maxg && (rand()%100)>50))
			{
				winnerPtr=air;
				maxg=df->gLimit;
			}
		}
	}

	if(winnerPtr==NULL)
	{
		return YSERR;
	}


	// Find target, if no target, find an alive && enemy of winner airplane
	df=(FsDogfight *)winnerPtr->GetAutopilot();
	air=df->GetTarget(this);
	if(air!=NULL && air->IsAlive()==YSTRUE)
	{
		victimPtr=air;
	}
	else
	{
		air=NULL;
		while((air=FindNextAirplane(air))!=NULL)
		{
			if(air->Prop().IsAlive()==YSTRUE && air->iff!=winnerPtr->iff)
			{
				victimPtr=air;
				break;
			}
		}
	}

	if(victimPtr==NULL)
	{
		return YSERR;
	}

	// Decide victim view or winner view
	YSBOOL victimView;
	victimView=((rand()%100)>50 ? YSTRUE : YSFALSE);

	if(victimView==YSTRUE)
	{
		fromAirplane=victimPtr;
		toAirplane=winnerPtr;
	}
	else
	{
		fromAirplane=winnerPtr;
		toAirplane=victimPtr;
	}
	return YSOK;
}

YSRESULT FsSimulation::DemoModeReconsiderPlayerAirplane(const FsAirplane *fromAir,const FsAirplane *toAir)
{
	const FsAirplane *air;
	air=fromAir;
	if(air!=NULL && air->Prop().IsActive()==YSTRUE)
	{
		SetPlayerAirplane(fromAir);
	}
	else
	{
		air=toAir;
		if(air!=NULL && air->Prop().IsAlive()==YSTRUE)
		{
			SetPlayerAirplane(toAir);
		}
	}
	return YSOK;
}

YSRESULT FsSimulation::DemoModeReconsiderLandingViewMode(FsDemoModeInfo &info)
{
	// Possible view mode
	//  FSCOCKPITVIEW      (Cockpit View)
	//  FSCARRIERVIEW      (ILS)
	//  FSPLAYERTOGNDVIEW  (Air to Ground)
	//  FSGNDTOPLAYERVIEW  (Ground to Air)
	//  FSAIRTOAIRVIEW     (#1 Air to #2 Air)
	//  FSAIRTOAIRVIEW     (#2 Air to #1 Air)
	//  FSSPOTPLANEVIEW    (Spot Plane)

	YsArray <FSVIEWMODE,256> candidate;
	FsAirplane *player;

	player=GetPlayerAirplane();
	if(player!=NULL)
	{
		FsLandingAutopilot *ap=NULL;
		const FsGround *ils=NULL;
		FsAirplane *spotPlane=NULL;
		YsVec3 tdPos;
		YsAtt3 rwAtt;

		if(info.useCockpitView==YSTRUE)
		{
			candidate.Append(FSCOCKPITVIEW);
		}
		if(info.useOutsideView==YSTRUE)
		{
			candidate.Append(FSSPOTPLANEVIEW);
		}

		ap=(FsLandingAutopilot *)player->GetAutopilot();
		if(ap!=NULL && ap->Type()==FSAUTOPILOT_LANDING)
		{
			ils=ap->ils;
			ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
			rwAtt.SetP(0.0);
			if(info.useCarrierView==YSTRUE)
			{
				candidate.Append(FSCARRIERVIEW);
			}
			if(info.usePlayerToGndView==YSTRUE)
			{
				candidate.Append(FSPLAYERTOGNDVIEW);
			}
			if(info.useGndToPlayerView==YSTRUE)
			{
				candidate.Append(FSGNDTOPLAYERVIEW);
			}
		}

		if(GetNumAirplane()>1)
		{
			YsArray <FsAirplane *> spotPlaneCandidate;
			FsAirplane *air;
			air=NULL;
			while((air=FindNextAirplane(air))!=NULL)
			{
				if(air!=player)
				{
					spotPlaneCandidate.Append(air);
				}
			}
			if(spotPlaneCandidate.GetN()>0)  //  2004/06/26
			{
				spotPlane=spotPlaneCandidate[rand()%spotPlaneCandidate.GetN()];
				if(info.useAirToAirView==YSTRUE)
				{
					candidate.Append(FSAIRTOAIRVIEW);
				}
				if(info.useAirFromAirView==YSTRUE)
				{
					candidate.Append(FSAIRFROMAIRVIEW);
				}
			}
		}

		if(ap!=NULL && ap->Type()==FSAUTOPILOT_LANDING)
		{
			int landingPhase=ap->landingPhase;

			// Mode -2:  STALL
			// Mode -1:  ILS not found.  I don't know what to do.
			// Mode 0:  Head to the entry point
			// Mode 1:  Hold in pattern until the airspeed reaches 110% of the landing speed
			// Mode 2:  I cannot enter the pattern because I'm too close to the pattern.  Go around.
			// Mode 3:  Turning to downwind leg.
			// Mode 4:  Downwind leg.
			// Mode 5:  Begin descend
			// Mode 6:  Turn to base
			// Mode 7:  Base leg
			// Mode 8:  Turn to final
			// Mode 9:
			// Mode 10:  Hey, clear the runway!

			for(auto i=candidate.GetN()-1; i>1; i--)  // i>1 this way at least one will survive.
			{
				// AIR TO AIR (player to another) invalid after base
				if(candidate[i]==FSAIRTOAIRVIEW && landingPhase>=FsLandingAutopilot::PHASE_BASE /*7*/)
				{
					candidate.DeleteBySwapping(i);
				}

				// AIR FROM AIR (another to player) invalid before base
				if(candidate[i]==FSAIRFROMAIRVIEW && landingPhase<FsLandingAutopilot::PHASE_BASE /*7*/)
				{
					candidate.DeleteBySwapping(i);
				}

				// GND TO AIR invalid before final
				// ILS invalid before final
				if((candidate[i]==FSGNDTOPLAYERVIEW || candidate[i]==FSCARRIERVIEW) && landingPhase<FsLandingAutopilot::PHASE_BASE_TO_FINAL /*8*/)
				{
					candidate.DeleteBySwapping(i);
				}
			}
		}

		if(candidate.GetN()==0)
		{
			candidate.Append(FSCOCKPITVIEW);
			candidate.Append(FSSPOTPLANEVIEW);
		}


		int n=rand()%candidate.GetN();

		double sgn;

		FSVIEWMODE prevViewmode;
		prevViewmode=mainWindowViewmode;

		mainWindowViewmode=candidate[n];
		switch(mainWindowViewmode)
		{
		case FSCOCKPITVIEW:
			YsPrintf("Cockpit View\n");
			break;
		case FSSPOTPLANEVIEW:
			YsPrintf("Spot Plane\n");
			if(prevViewmode!=mainWindowViewmode)
			{
				relViewDist=2.0;
				// relViewAtt.SetH(YsPi*2.0*double(rand()%360)/360.0);
				sgn=(rand()%100<50 ? -1.0 : 1.0);
				relViewAtt.SetH(rwAtt.h()+YsPi+YsDegToRad(3.0)*sgn);
				relViewAtt.SetP(-YsPi/18.0);
				relViewAtt.SetB(0.0);
			}
			break;
		case FSCARRIERVIEW:
			YsPrintf("Carrier View\n");
			focusGnd=ils;
			focusAir=player;
			break;
		case FSPLAYERTOGNDVIEW:
		case FSGNDTOPLAYERVIEW:
			focusGnd=ils;
			break;
		case FSAIRTOAIRVIEW:
		case FSAIRFROMAIRVIEW:
			focusAir=player;
			focusAir2=spotPlane;
			break;
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::DemoModeReconsiderAcrobatViewMode(FsDemoModeInfo &info)
{
	int n,i,j;
	FsAirplane *air;
	FsAirshowControl *ap;
	switch(info.acroType)
	{
	default:
		n=rand()%4;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			break;
		}
		return YSOK;

	case FSACRO_DOUBLEFARVEL:
		n=rand()%8;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,info.formation[0]);
			break;
		case 3:
			DemoModeSetAcrobatViewModeCockpitView(info,info.formation[3]);
			break;
		case 4:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[0]);
			break;
		case 5:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[3]);
			break;
		case 6:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[1]);
			relViewAtt.Set(-YsPi/2.0,-YsPi/6.0,0.0);
			break;
		case 7:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[2]);
			relViewAtt.Set(YsPi/2.0,-YsPi/6.0,0.0);
			break;
		}
		return YSOK;

	case FSACRO_LINEABREASTLOOP:
		n=rand()%6;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,info.formation[1]);
			mainWindowViewmode=FS90DEGREERIGHTVIEW;
			break;
		case 3:
			DemoModeSetAcrobatViewModeCockpitView(info,info.formation[2]);
			mainWindowViewmode=FS90DEGREELEFTVIEW;
			break;
		case 4:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[1]);
			relViewAtt.Set(-YsPi/2.0,-YsPi/18.0,0.0);
			break;
		case 5:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[2]);
			relViewAtt.Set(YsPi/2.0,-YsPi/18.0,0.0);
			break;
		}
		return YSOK;

	case FSACRO_LINEABREASTROLL:
		n=rand()%6;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,info.formation[0]);
			mainWindowViewmode=FS90DEGREERIGHTVIEW;
			break;
		case 3:
			DemoModeSetAcrobatViewModeCockpitView(info,info.formation[2]);
			mainWindowViewmode=FS90DEGREELEFTVIEW;
			break;
		case 4:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[0]);
			relViewAtt.Set(-YsPi/2.0,-YsPi/18.0,0.0);
			break;
		case 5:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[2]);
			relViewAtt.Set(YsPi/2.0,-YsPi/18.0,0.0);
			break;
		}
		return YSOK;

	case FSACRO_LEVELOPENER:
		n=rand()%4;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			if(GetPlayerAirplane()==info.formation[0])
			{
				mainWindowViewmode=FSBACKMIRRORVIEW;
			}
			else if(GetPlayerAirplane()==info.formation[1])
			{
				mainWindowViewmode=FS90DEGREERIGHTVIEW;
			}
			else if(GetPlayerAirplane()==info.formation[2])
			{
				mainWindowViewmode=FS90DEGREELEFTVIEW;
			}
			else if(GetPlayerAirplane()==info.formation[3])
			{
				mainWindowViewmode=FSCOCKPITVIEW;
			}
			else if(GetPlayerAirplane()==info.formation[4])
			{
				mainWindowViewmode=FSCOCKPITVIEW;
			}
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			if(GetPlayerAirplane()==info.formation[0])
			{
				relViewAtt.Set(YsPi,-YsPi/17.0,0.0);
			}
			else if(GetPlayerAirplane()==info.formation[1])
			{
				relViewAtt.Set(-YsPi/2.0,-YsPi/18.0,0.0);
			}
			else if(GetPlayerAirplane()==info.formation[2])
			{
				relViewAtt.Set(YsPi/2.0,-YsPi/18.0,0.0);
			}
			else if(GetPlayerAirplane()==info.formation[3])
			{
				relViewAtt.Set(0.0,-YsPi/18.0,0.0);
			}
			else if(GetPlayerAirplane()==info.formation[4])
			{
				relViewAtt.Set(0.0,-YsPi/18.0,0.0);
			}
			break;
		}
		return YSOK;

	case FSACRO_STARCROSS:
	case FSACRO_STAROFDAVID:
	case FSACRO_BOMBBURST6SHIP:
		ap=FsAirshowControl::GetAirshowAP(info.formation[0]);
		n=rand()%5;

		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			if(ap!=NULL && ap->bombBurstMode>=8)
			{
				mainWindowViewmode=FSTOWERVIEW_NOMAGNIFY;
			}
			break;
		case 1:
			if(info.acroType==FSACRO_STARCROSS && ap!=NULL && ap->bombBurstMode>=8)
			{
				mainWindowViewmode=FSTOWERVIEW_NOMAGNIFY;
				towerViewPos=ap->bombBurstBreakPoint;
				towerViewPos.AddX(100.0);
				towerViewPos.AddY(2500.0);
			}
			else
			{
				DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			}
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			if(ap!=NULL && ap->bombBurstMode>=3)
			{
				mainWindowViewmode=FSCOCKPITVIEW;
			}
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			if(ap!=NULL && ap->bombBurstMode>=5)
			{
				relViewAtt.Set(0.0,-YsPi/18.0,0.0);
			}
			else if(ap!=NULL && ap->bombBurstMode>=3)
			{
				relViewAtt.Set(YsPi,-YsPi/18.0,0.0);
			}
			break;
		case 4:
			if(info.refObj!=NULL)
			{
				mainWindowViewmode=FSCARRIERVIEW;
				focusGnd=info.refObj;
			}
			else
			{
				DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			}
			break;
		}
		return YSOK;

	case FSACRO_LETTEREIGHT:
		n=rand()%9;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,info.formation[0]);
			break;
		case 1:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,info.formation[3]);
			break;
		case 2:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,info.formation[0]);
			break;
		case 3:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,info.formation[3]);
			break;
		case 4:
			DemoModeSetAcrobatViewModeCockpitView(info,info.formation[3]);
			break;
		case 5:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[0]);
			break;
		case 6:
			DemoModeSetAcrobatViewModeOutsideView3(info,info.formation[3]);
			break;
		case 7:
			mainWindowViewmode=FSAIRTOAIRVIEW;
			focusAir=info.formation[0];
			focusAir2=info.formation[3];
			break;
		case 8:
			mainWindowViewmode=FSAIRTOAIRVIEW;
			focusAir=info.formation[3];
			focusAir2=info.formation[0];
			break;
		}
		return YSOK;

	case FSACRO_TIGHTTURN:
	case FSACRO_360ANDLOOP:
	case FSACRO_SLOWROLL:
		n=rand()%4;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			mainWindowViewmode=FSAIRTOTOWERVIEWSOLO;
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			break;
		}
		return YSOK;

	case FSACRO_CORKSCREW:
		n=rand()%5;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,info.solo[1]);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,info.solo[1]);
			break;
		case 3:
			SetPlayerAirplane(info.solo[1]);
			mainWindowViewmode=FSOUTSIDEPLAYER3;
			relViewAtt.Set(-YsPi/4.0,-YsPi/18.0,0.0);
			break;
		case 4:
			mainWindowViewmode=FSAIRTOAIRVIEW;
			focusAir=info.solo[1];
			focusAir2=info.solo[0];
			break;
		}
		return YSOK;
	case FSACRO_BOMBBURST4SHIP:
		n=rand()%5;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			break;
		default:  // Special view mode for Bomb Burst
		case 4:
			{
				FsAirshowControl *leadAp,*soloAp;

				leadAp=FsAirshowControl::GetAirshowAP(info.formation[0]);
				soloAp=FsAirshowControl::GetAirshowAP(info.solo[0]);

				if(soloAp!=NULL && soloAp->bombBurstMode<=1)  // Until vertical climb roll
				{
					SetPlayerAirplane(info.solo[0]);
					mainWindowViewmode=FSOUTSIDEPLAYER3;
					relViewAtt.Set(YsPi/18.0,-YsPi/18.0,0.0);
				}
				else if(soloAp!=NULL && soloAp->bombBurstMode==2)
				{
					i=rand()%90;
					if(i<30)
					{
						SetPlayerAirplane(info.solo[0]);
						mainWindowViewmode=FSOUTSIDEPLAYER3;
						relViewAtt.Set(YsPi/18.0,-YsPi/18.0,0.0);
					}
					else if(i<60)
					{
						SetPlayerAirplane(info.solo[0]);
						mainWindowViewmode=FSFROMTOPOFPLAYERPLANE;
					}
					else
					{
						SetPlayerAirplane(info.solo[0]);
						mainWindowViewmode=FSOUTSIDEPLAYER2;
						relViewAtt.Set(YsPi*(double)(rand()%200)/100.0,0.0,0.0);
					}
				}
				else if(soloAp!=NULL && soloAp->bombBurstMode==3)
				{
					SetPlayerAirplane(info.solo[0]);
					mainWindowViewmode=FSFROMTOPOFPLAYERPLANE;
				}
				else
				{
					FsAirplane *shuffle[4],*swap;
					mainWindowViewmode=FSAIRTOAIRVIEW;
					shuffle[0]=info.formation[0];
					shuffle[1]=info.formation[1];
					shuffle[2]=info.formation[2];
					shuffle[3]=info.formation[3];
					for(i=0; i<4; i++)
					{
						j=rand()%4;
						swap=shuffle[i];
						shuffle[i]=shuffle[j];
						shuffle[j]=swap;
					}
					focusAir=shuffle[0];
					focusAir2=shuffle[1];
				}
			}
			break;
		}
		return YSOK;
	case FSACRO_BOMBBURSTDOWN4SHIP:
	case FSACRO_BOMBBURSTDOWN6SHIP:
	case FSACRO_RAINFALL:
		ap=FsAirshowControl::GetAirshowAP(info.formation[0]);
		n=rand()%4;

		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			if(ap!=NULL && ap->bombBurstMode>=2)
			{
				mainWindowViewmode=FSBACKMIRRORVIEW;
			}
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			if(ap!=NULL && ap->bombBurstMode>=2)
			{
				relViewAtt.Set(-YsPi*17.0/18.0,-YsPi/18.0,0.0);
			}
			break;
		}
		return YSOK;
	case FSACRO_CHANGEOVERTURN:
		air=info.formation[1+rand()%5];
		n=rand()%4;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,air);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,air);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,air);
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,air);
			break;
		}
		return YSOK;
	case FSACRO_TRAILTODIAMONDROLL:
		n=rand()%4;
		ap=FsAirshowControl::GetAirshowAP(info.formation[0]);
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			if(ap!=NULL && ap->brlMode<2 && GetPlayerAirplane()!=info.formation[0])
			{
				mainWindowViewmode=FSCOCKPITVIEW;
			}
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			if(ap!=NULL && ap->brlMode<2 && GetPlayerAirplane()!=info.formation[0])
			{
				relViewAtt.SetH(0.0);
			}
			break;
		}
		return YSOK;
	case FSACRO_ROLLINGCOMBATPITCH:
		n=rand()%4;
		ap=FsAirshowControl::GetAirshowAP(info.formation[3]);
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,info.formation[3]);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,info.formation[3]);
			break;
		case 2:
			if(ap!=NULL && ap->cpMode==0)
			{
				SetPlayerAirplane(info.formation[3]);
				mainWindowViewmode=FS45DEGREERIGHTVIEW;
			}
			else
			{
				SetPlayerAirplane(info.formation[3]);
				mainWindowViewmode=FSFROMTOPOFPLAYERPLANE;
			}
			break;
		case 3:
			SetPlayerAirplane(info.formation[3]);
			mainWindowViewmode=FSOUTSIDEPLAYER3;
			relViewAtt.Set(-YsPi/4.0,-YsPi/18.0,0.0);
			break;
		}
		return YSOK;
	case FSACRO_ROLLONTAKEOFFANDHALFCUBAN:
		n=rand()%6;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			break;
		case 4:
			mainWindowViewmode=FSAIRTOAIRVIEW;
			focusAir=info.solo[0];
			focusAir2=info.solo[1];
			break;
		case 5:
			SetPlayerAirplane(info.solo[1]);
			mainWindowViewmode=FSOUTSIDEPLAYER3;
			relViewAtt.Set(YsPi,-YsPi/18.0,0.0);
			break;
		}
		return YSOK;
	case FSACRO_BIGHEART:
		ap=FsAirshowControl::GetAirshowAP(info.solo[0]);
		if(ap!=NULL && ap->bombBurstMode>=3)
		{
			n=rand()%6;
		}
		else
		{
			n=rand()%4;
		}
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,NULL);
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			break;
		case 4:
			mainWindowViewmode=FSTOWERVIEW_NOMAGNIFY;
			towerViewPos.Set(1500.0,0.0,0.0);
			info.solo[0]->GetAttitude().Mul(towerViewPos,towerViewPos);
			towerViewPos+=info.solo[0]->GetPosition();
			break;
		case 5:
			mainWindowViewmode=FSTOWERVIEW_NOMAGNIFY;
			towerViewPos.Set(-1500.0,0.0,0.0);
			info.solo[0]->GetAttitude().Mul(towerViewPos,towerViewPos);
			towerViewPos+=info.solo[0]->GetPosition();
			break;
		}
		return YSOK;
	case FSACRO_PITCHUPBREAK:
		air=info.formation[3+rand()%3];
		ap=FsAirshowControl::GetAirshowAP(air);
		n=rand()%4;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,air);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,air);
			break;
		case 2:
			DemoModeSetAcrobatViewModeCockpitView(info,air);
			if(ap!=NULL && ap->cpMode!=0)
			{
				mainWindowViewmode=FSCOCKPITVIEW;
			}
			break;
		case 3:
			DemoModeSetAcrobatViewModeOutsideView3(info,air);
			break;
		}
		return YSOK;
	case FSACRO_ROCKWINGCLEAN:
	case FSACRO_ROCKWINGDIRTY:
		n=rand()%3;
		switch(n)
		{
		case 0:
			DemoModeSetAcrobatViewModeTowerToAirplane(info,NULL);
			break;
		case 1:
			DemoModeSetAcrobatViewModeAirplaneToTower(info,NULL);
			mainWindowViewmode=FSAIRTOTOWERVIEWSOLO;
			break;
		case 2:
			DemoModeSetAcrobatViewModeOutsideView3(info,NULL);
			break;
		}
		return YSOK;
	}
	// Unreachable return YSERR;
}

YSRESULT FsSimulation::DemoModeReconsiderConcordeFlyByViewMode(FsDemoModeInfo &info)
{
	YsArray <FSVIEWMODE,16> available;
	int i;

	YsArray <FsAirplane *,16> candidate;
	for(i=0; i<3; i++)
	{
		if(info.formation[i]!=NULL &&
		   info.formation[i]->Prop().GetFlightState()!=FSGROUNDSTATIC &&
		   info.formation[i]->Prop().IsActive()==YSTRUE)
		{
			candidate.Append(info.formation[i]);
		}
	}
	if(candidate.GetN()==0)
	{
		for(i=0; i<3; i++)
		{
			if(info.formation[i]!=NULL &&
			   info.formation[i]->Prop().IsActive()==YSTRUE)
			{
				candidate.Append(info.formation[i]);
			}
		}
	}

	if(candidate.GetN()>0)
	{
		focusAir=candidate[rand()%candidate.GetN()];
		SetPlayerAirplane(focusAir);
	}
	else
	{
		return YSERR;
	}


	// May switch focus air with another Concorde


	if(info.useCockpitView==YSTRUE)
	{
		available.Append(FSCOCKPITVIEW);
	}

	if(info.useOutsideView==YSTRUE)
	{
		double h,p,b;
		h= (YsPi*2.0)*(double)(rand()%1000)/1000.0;
		p=-(YsPi/6.0)*(double)(rand()%1000)/1000.0;
		b=0.0;
		relViewAtt.Set(h,p,b);

		available.Append(FSTURNVIEW);
		available.Append(FSOUTSIDEPLAYER2);
		available.Append(FSOUTSIDEPLAYER3);

		if(focusAir->GetPosition().y()>=150.0)
		{
			available.Append(FSVERTICALORBITINGVIEW);
			available.Append(FSHORIZONTALORBITINGVIEW);
		}
	}

	if(info.useGndToPlayerView==YSTRUE || info.usePlayerToGndView==YSTRUE)
	{
		YsVec3 d;
		if(info.refObj!=NULL)
		{
			d=info.refObj->GetPosition()-focusAir->GetPosition();
			if(d.GetSquareLength()<YsSqr(2500.0))
			{
				focusGnd=info.refObj;
				if(info.useGndToPlayerView==YSTRUE)
				{
					available.Append(FSGNDTOPLAYERVIEW); // Wasnt' it FSCARRIERVIEW?
				}
				if(info.usePlayerToGndView==YSTRUE)
				{
					available.Append(FSPLAYERTOGNDVIEW); // Wasnt' it FSCARRIERVIEW?
				}
			}
		}

		for(i=0; i<towerPosition.GetN(); i++)
		{
			d=towerPosition[i]-focusAir->GetPosition();
			if(d.GetSquareLength()<YsSqr(5000.0))
			{
				towerViewId=i;
				towerViewPos=towerPosition[i];
				if(info.useGndToPlayerView==YSTRUE)
				{
					available.Append(FSTOWERVIEW);
				}
				if(info.usePlayerToGndView==YSTRUE)
				{
					available.Append(FSAIRTOTOWERVIEW);
					available.Append(FSAIRTOTOWERVIEWSOLO);
				}
				break;
			}
		}
	}

	if(info.formation[0]!=NULL && info.formation[1]!=NULL)
	{
		if(focusAir==info.formation[0])
		{
			focusAir2=info.formation[1];
		}
		else
		{
			focusAir2=info.formation[0];
		}

		if(info.useAirToAirView==YSTRUE)
		{
			available.Append(FSAIRTOAIRVIEW);
		}
		if(info.useAirFromAirView==YSTRUE)
		{
			available.Append(FSAIRFROMAIRVIEW);
		}
	}

	if(available.GetN()>0)
	{
		int n;
		n=rand()%available.GetN();
		mainWindowViewmode=available[n];
		if(mainWindowViewmode==FSVERTICALORBITINGVIEW)
		{
			relViewAtt.SetB(YsPi/6.0);
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::DemoModeSetAcrobatViewModeTowerToAirplane(FsDemoModeInfo &info,FsAirplane *air)
{
	if(air==NULL)
	{
		if(info.formation[3]!=NULL)
		{
			air=info.formation[3];
		}
		else if(info.formation[0]!=NULL)
		{
			air=info.formation[0];
		}
		else
		{
			int fomPosition;
			air=info.PickRandomAirplane(fomPosition,YSTRUE,YSTRUE);
		}

		if(air==NULL)
		{
			return YSERR;
		}
	}

	mainWindowViewmode=FSTOWERVIEW;
	towerViewPos=info.showCenter;
	focusAir=air;
	return YSOK;
}

YSRESULT FsSimulation::DemoModeSetAcrobatViewModeAirplaneToTower(FsDemoModeInfo &info,FsAirplane *air)
{
	if(DemoModeSetAcrobatViewModeTowerToAirplane(info,air)==YSOK)
	{
		mainWindowViewmode=FSAIRTOTOWERVIEW;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::DemoModeSetAcrobatViewModeCockpitView(FsDemoModeInfo &info,FsAirplane *air)
{
	int fomPosition;
	fomPosition=-1;
	if(air==NULL)
	{
		air=info.PickRandomAirplane(fomPosition,YSTRUE,YSFALSE);  // formation=YSTRUE, solo=YSFALSE
		if(air==NULL)
		{
			air=info.PickRandomAirplane(fomPosition,YSFALSE,YSTRUE);
		}
	}
	else
	{
		fomPosition=info.GetFormationPosition(air);
	}

	if(air!=NULL)
	{
		SetPlayerAirplane(air);
		switch(fomPosition)
		{
		default:
		case 4:
			mainWindowViewmode=FSCOCKPITVIEW;
			break;
		case 1:
			mainWindowViewmode=FSBACKMIRRORVIEW;
			break;
		case 2:
		case 5:
			mainWindowViewmode=FS45DEGREERIGHTVIEW;
			break;
		case 3:
		case 6:
			mainWindowViewmode=FS45DEGREELEFTVIEW;
			break;
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::DemoModeSetAcrobatViewModeOutsideView3(FsDemoModeInfo &info,FsAirplane *air)
{
	int fomPosition;
	fomPosition=-1;
	if(air==NULL)
	{
		air=info.PickRandomAirplane(fomPosition,YSTRUE,YSFALSE);  // formation=YSTRUE, solo=YSFALSE
		if(air==NULL)
		{
			air=info.PickRandomAirplane(fomPosition,YSFALSE,YSTRUE);
		}
	}
	else
	{
		fomPosition=info.GetFormationPosition(air);
	}

	if(air!=NULL)
	{
		SetPlayerAirplane(air);
		mainWindowViewmode=FSOUTSIDEPLAYER3;
		switch(fomPosition)
		{
		default:
		case 4:
			relViewDist=2.0;
			relViewAtt.Set(0.0,-YsPi/18.0,0.0);
			break;
		case 1:
			relViewDist=2.0;
			relViewAtt.Set(YsPi,-YsPi/10.0,0.0);
			break;
		case 2:
		case 5:
			relViewDist=2.0;
			relViewAtt.Set(-YsPi/4.0,-YsPi/18.0,0.0);
			break;
		case 3:
		case 6:
			relViewDist=2.0;
			relViewAtt.Set(YsPi/4.0,-YsPi/18.0,0.0);
			break;
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSimulation::DemoModeRipOffEarlyPartOfRecord(void)
{
	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		air->RipOffEarlyPartOfRecord();
	}

	FsGround *gnd;
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		gnd->RipOffEarlyPartOfRecord();
	}
	explosionHolder.RipOffEarlyPartOfRecord();
	bulletHolder.RipOffEarlyPartOfRecord();
	return YSOK;
}

YSBOOL FsSimulation::DemoModeOneSideWon(void)
{
	int iff0,iff1;
	FsAirplane *air;
	iff0=YSFALSE;
	iff1=YSFALSE;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->Prop().IsActive()==YSTRUE && air->iff==FS_IFF0)
		{
			iff0=YSTRUE;
		}
		if(air->Prop().IsActive()==YSTRUE && air->iff==FS_IFF1)
		{
			iff1=YSTRUE;
		}
	}
	if(iff0!=YSTRUE || iff1!=YSTRUE)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

