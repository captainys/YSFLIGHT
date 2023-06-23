#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <ysclass.h>
#include <ysport.h>

#include "fs.h"
#include "fsfilename.h"
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#include <time.h>

// Implementation //////////////////////////////////////////



FsFlightControl::FsFlightControl()
{
	Initialize();
}

FsFlightControl::FsFlightControl(const FsFlightControl &from)
{
	*this=from;
}

void FsFlightControl::Initialize(void)
{
	ctlGear=0.0;
	ctlGearTrouble=YSFALSE;
	ctlBrake=0.0;
	ctlSpoiler=0.0;
	ctlAb=YSFALSE;
	hasAb=YSFALSE;
	ctlThrottle=0.0;
	ctlPropeller=1.0; // Full forward
	ctlFlap=0.0;
	ctlVgw=0.0;
	ctlAutoVgw=YSTRUE;
	ctlThrVec=0.0;
	ctlThrRev=0.0;

	ctlElevator=0.0;
	ctlElvTrim=0.0;
	ctlRudder=0.0;
	ctlAileron=0.0;

	ctlFireWeaponButtonExt=YSFALSE;
	ctlFireGunButtonExt=YSFALSE;
	ctlFireAAMButtonExt=YSFALSE;
	ctlFireAGMButtonExt=YSFALSE;
	ctlFireRocketButtonExt=YSFALSE;
	ctlDropBombButtonExt=YSFALSE;
	ctlDispenseFlareButtonExt=YSFALSE;
	ctlCycleWeaponButtonExt=YSFALSE;
	ctlSmokeButtonExt=YSFALSE;
	ctlCycleSmokeSelectorButtonExt=YSFALSE;

	ctlFireWeaponButton=YSFALSE;
	ctlFireGunButton=YSFALSE;
	ctlFireAAMButton=YSFALSE;
	ctlFireAGMButton=YSFALSE;
	ctlFireRocketButton=YSFALSE;
	ctlDropBombButton=YSFALSE;
	ctlDispenseFlareButton=YSFALSE;
	ctlCycleWeaponButton=YSFALSE;
	ctlSmokeButton=YSFALSE;
	ctlBombBayDoor=YSFALSE;

	ctlLeftDoor=0.0;
	ctlRightDoor=0.0;
	ctlRearDoor=0.0;

	ctlSensitivity=1.0;

	ctlNavId=0;
	ctlVectorMarker=YSTRUE;

	brakeHold=YSFALSE;
	spoilerHold=YSFALSE;

	pov=0;
	viewHdg=0.0;
	viewPch=0.0;

	ctlTurretHdg=0.0;
	ctlTurretPch=0.0;
}

YSRESULT FsFlightControl::ProcessButtonFunction(const double &/*cTime*/,FsExistence *existence,FSBUTTONFUNCTION fnc)
{
	FsAirplane *air=NULL;
	FsGround *gnd=NULL;
	if(NULL!=existence)
	{
		switch(existence->GetType())
		{
		case FSEX_AIRPLANE:
			air=(FsAirplane *)existence;
			break;
		case FSEX_GROUND:
			gnd=(FsGround *)existence;
			break;
		}
	}

	switch(fnc)
	{
	case FSBTF_ELEVATORUP:                    //  Elevator Up
		ctlElevator=YsSmaller(ctlElevator+0.04,1.0);
		return YSOK;
	case FSBTF_ELEVATORNEUTRAL:               //  Elevator Neutral
		ctlElevator=0.0;
		return YSOK;
	case FSBTF_ELEVATORDOWN:                  //  Elevator Down
		ctlElevator=YsGreater(ctlElevator-0.04,-1.0);
		return YSOK;
	case FSBTF_AILERONLEFT:                   //  Aileron Left
		if(ctlAileron<0.0)
		{
			ctlAileron=0.0;
		}
		else
		{
			ctlAileron=YsSmaller(ctlAileron+0.1,1.0);
		}
		return YSOK;
	case FSBTF_AILERONNEUTRAL:                //  Aileron Neutral
		ctlAileron=0.0;
		return YSOK;
	case FSBTF_AILERONRIGHT:                  //  Aileron Right
		if(ctlAileron>0.0)
		{
			ctlAileron=0.0;
		}
		else
		{
			ctlAileron=YsGreater(ctlAileron-0.1,-1.0);
		}
		return YSOK;
	case FSBTF_RUDDERLEFT:                    //  Rudder Left
		ctlRudder=YsSmaller(ctlRudder+0.25,1.0);
		return YSOK;
	case FSBTF_RUDDERCENTER:                  //  Rudder Center
		ctlRudder=0.0;
		return YSOK;
	case FSBTF_RUDDERRIGHT:                   //  Rudder Right
		ctlRudder=YsGreater(ctlRudder-0.25,-1.0);
		return YSOK;
	// case FSBTF_TRIMUP:                        //  Elevator Trim Up  Processed in SimControlByUser.  Effect depends on dt.
	//	break;
	// case FSBTF_TRIMDOWN:                      //  Elevator Trim Down  Processed in SimControlByUser.  Effect depends on dt.
	//	breauk;
	case FSBTF_AUTOTRIM:                      //  Auto Trim
		ctlElvTrim=YsBound(ctlElvTrim+ctlElevator,-1.0,1.0);
		return YSOK;
	case FSBTF_THROTTLEUP:                    //  Throttle Add Power
		ctlThrottle=YsSmaller(ctlThrottle+0.05,1.0);
		return YSOK;
	case FSBTF_THROTTLEUP_HALF:
		ctlThrottle=YsSmaller(ctlThrottle+0.025,1.0);
		return YSOK;
	case FSBTF_THROTTLEDOWN:                  //  Throttle Reduce Power
		ctlThrottle=YsGreater(ctlThrottle-0.05,0.0);
		if(ctlThrottle<0.6)
		{
			ctlAb=YSFALSE;
		}
		return YSOK;
	case FSBTF_THROTTLEDOWN_HALF:             //  Throttle Reduce Power
		ctlThrottle=YsGreater(ctlThrottle-0.025,0.0);
		if(ctlThrottle<0.6)
		{
			ctlAb=YSFALSE;
		}
		return YSOK;
	case FSBTF_THROTTLEMAX:                   //  Throttle Max
		ctlThrottle=1.0;
		ctlAb=YSTRUE;
		return YSOK;
	case FSBTF_THROTTLEIDLE:                  //  Throttle Min (Idle)
		ctlThrottle=0.0;
		ctlAb=YSFALSE;
		return YSOK;
	case FSBTF_AFTERBURNER:                   //  Afterburner
		ctlAb=(ctlAb==YSTRUE ? YSFALSE : YSTRUE);
		if(ctlAb==YSTRUE && ctlThrottle<0.6)
		{
			ctlThrottle=0.6;
		}
		return YSOK;
	case FSBTF_PROPFORWARD:
		ctlPropeller+=0.1;
		if(1.0<ctlPropeller)
		{
			ctlPropeller=1.0;
		}
		return YSOK;
	case FSBTF_PROPBACKWARD:
		ctlPropeller-=0.1;
		if(0.0>ctlPropeller)
		{
			ctlPropeller=0.0;
		}
		return YSOK;
	case FSBTF_NOZZLEUP:                      //  Nozzle Up (Nose Up for Concorde)
		ctlThrVec=YsGreater(ctlThrVec-0.1,0.0);
		return YSOK;
	case FSBTF_NOZZLEDOWN:                    //  Nozzle Down (Nose Down for Concorde)
		ctlThrVec=YsSmaller(ctlThrVec+0.1,1.0);
		return YSOK;
	case FSBTF_LANDINGGEAR:                   //  Landing Gear Extend/Retract
		if(YSTRUE!=ctlGearTrouble)
		{
			ctlGear=(ctlGear<0.5 ? 1.0 : 0.0);
		}
		else
		{
			ctlGear=(ctlGear<0.5 ? 0.8 : 0.2);
			ctlGearTrouble=YSFALSE;
		}
		return YSOK;
	case FSBTF_FLAP:                          //  Flap Up/Down
		ctlFlap=(ctlFlap<0.5 ? 1.0 : 0.0);
		return YSOK;
	case FSBTF_FLAPUP:                        //  Flap Up
		if(NULL!=air)
		{
			const int nFlpPos=air->Prop().GetNumFlapPosition();
			int i;
			YSBOOL set=YSFALSE;
			for(i=0; i<nFlpPos; i++)
			{
				if(YSTRUE==YsEqual(ctlFlap,air->Prop().GetFlapPosition(i)))
				{
					if(0<i)
					{
						ctlFlap=air->Prop().GetFlapPosition(i-1);
					}
					set=YSTRUE;
					break;
				}
			}
			if(YSTRUE!=set)
			{
				for(i=0; i<nFlpPos-1; i++)
				{
					if(air->Prop().GetFlapPosition(i)<=ctlFlap && ctlFlap<air->Prop().GetFlapPosition(i+1))
					{
						ctlFlap=air->Prop().GetFlapPosition(i);
						set=YSTRUE;
						break;
					}
				}
			}
			if(YSTRUE!=set)
			{
				if(0<nFlpPos)
				{
					ctlFlap=air->Prop().GetFlapPosition(0);
				}
				else
				{
					ctlFlap=0.0;
				}
			}
		}
		// ctlFlap=YsGreater(ctlFlap-0.25,0.0);
		return YSOK;
	case FSBTF_FLAPDOWN:                      //  Flap Down
		if(NULL!=air)
		{
			const int nFlpPos=air->Prop().GetNumFlapPosition();
			int i;
			YSBOOL set=YSFALSE;
			for(i=0; i<nFlpPos; i++)
			{
				if(YSTRUE==YsEqual(ctlFlap,air->Prop().GetFlapPosition(i)))
				{
					if(i<nFlpPos-1)
					{
						ctlFlap=air->Prop().GetFlapPosition(i+1);
					}
					set=YSTRUE;
					break;
				}
			}
			if(YSTRUE!=set)
			{
				for(i=0; i<nFlpPos-1; i++)
				{
					if(air->Prop().GetFlapPosition(i)<ctlFlap && ctlFlap<=air->Prop().GetFlapPosition(i+1))
					{
						ctlFlap=air->Prop().GetFlapPosition(i+1);
						set=YSTRUE;
						break;
					}
				}
			}
			if(YSTRUE!=set)
			{
				if(0<nFlpPos)
				{
					ctlFlap=air->Prop().GetFlapPosition(nFlpPos-1);
				}
				else
				{
					ctlFlap=1.0;
				}
			}
		}
		// ctlFlap=YsSmaller(ctlFlap+0.25,1.0);
		return YSOK;
	case FSBTF_FLAPFULLUP:                    //  Flap Full Up
		ctlFlap=0.0;
		return YSOK;
	case FSBTF_FLAPFULLDOWN:                  //  Flap Full Down
		ctlFlap=1.0;
		return YSOK;
	case FSBTF_SPOILERBRAKE:                  //  Spoiler and Brake On/Off
		ctlBrake=(ctlBrake<0.5 ? 1.0 : 0.0);
		ctlSpoiler=ctlBrake;
		return YSOK;
	case FSBTF_SPOILER:                       //  Spoiler Extend/Retract
		ctlSpoiler=(ctlSpoiler<0.5 ? 1.0 : 0.0);
		return YSOK;
	case FSBTF_SPOILEREXTEND:                 //  Spoiler Extend
		ctlSpoiler=YsSmaller(ctlSpoiler+0.25,1.0);
		return YSOK;
	case FSBTF_SPOILERRETRACT:                //  Spoiler Retract
		ctlSpoiler=YsGreater(ctlSpoiler-0.25,0.0);
		return YSOK;
	case FSBTF_BRAKEONOFF:                    //  Brake On/Off
		ctlBrake=(ctlBrake<0.5 ? 1.0 : 0.0);
		return YSOK;

	case FSBTF_RADAR:                         //  Radar
		if(NULL!=air)
		{
			const int dir=(YSTRUE!=FsGetKeyState(FSKEY_SHIFT) ? 1 : -1);
			air->Prop().ToggleRadarRange(dir);
		}
		return YSOK;
	case FSBTF_RADARRANGEUP:                  //  Radar Range Up
		if(air!=NULL)
		{
			air->Prop().IncreaseRadarRange();
		}
		return YSOK;
	case FSBTF_RADARRANGEDOWN:                //  Radar Range Down
		if(air!=NULL)
		{
			air->Prop().ReduceRadarRange();
		}
		return YSOK;


	case FSBTF_ILS:                           //  ILS On/Off
		switch(ctlNavId)
		{
		case 0:
			ctlNavId=1;
			break;
		case 1:
			ctlNavId=100;   // <- ADF
			break;
		case 100:
			ctlNavId=200;   // <- Heading Bug
			break;
		default:
		case 200:
			ctlNavId=0;
			break;
		}
		return YSOK;
	case FSBTF_VELOCITYINDICATOR:             //  Velocity Indicator On/Off
		YsFlip(ctlVectorMarker);
		return YSOK;

	case FSBTF_BOMBBAYDOOR:                   // Open/Close Bomb Bay Door
		YsFlip(ctlBombBayDoor);
		return YSOK;


	/* Turret Motion is moved to FsSimulation::SimControlByUser
	case FSBTF_TURRETLEFT:
		ctlTurretHdg+=0.0125;
		if(ctlTurretHdg>=1.0)
		{
			ctlTurretHdg=1.0;
		}
		return YSOK;
	case FSBTF_TURRETRIGHT:
		ctlTurretHdg-=0.0125;
		if(ctlTurretHdg<=-1.0)
		{
			ctlTurretHdg=-1.0;
		}
		return YSOK;
	case FSBTF_TURRETUP:
		ctlTurretPch+=0.0125;
		if(ctlTurretPch>=1.0)
		{
			ctlTurretPch=1.0;
		}
		return YSOK;
	case FSBTF_TURRETDOWN:
		ctlTurretPch-=0.0125;
		if(ctlTurretPch<=-1.0)
		{
			ctlTurretPch=-1.0;
		}
		return YSOK; */
	case FSBTF_TURRETNEUTRAL:
		ctlTurretPch=0.0;
		ctlTurretHdg=0.0;
		return YSOK;

	case FSBTF_TOGGLELIGHT:
		if(NULL!=air)
		{
			air->Prop().ToggleLight();
		}
		if(NULL!=gnd)
		{
			gnd->Prop().ToggleLight();
		}
		break;
	case FSBTF_TOGGLENAVLIGHT:
		if(NULL!=air)
		{
			air->Prop().ToggleNavLight();
		}
		break;
	case FSBTF_TOGGLEBEACON:
		if(NULL!=air)
		{
			air->Prop().ToggleBeacon();
		}
		break;
	case FSBTF_TOGGLESTROBE:
		if(NULL!=air)
		{
			air->Prop().ToggleStrobe();
		}
		break;
	case FSBTF_TOGGLELANDINGLIGHT:
		if(NULL!=air)
		{
			air->Prop().ToggleLandingLight();
		}
		break;


	// The following keys are implemented through virtual buttons of FsAirplaneProperty
	case FSBTF_FIREWEAPON:                    //  Fire Selected Weapon
	case FSBTF_FIREAAM:                       //  Fire AAM
	case FSBTF_FIREAGM:                       //  Fire AAM
	case FSBTF_FIREROCKET:                    //  Fire Rocket
	case FSBTF_DROPBOMB:                      //  Drop Bomb
	case FSBTF_DISPENSEFLARE:                 //  Dispense Flare
	case FSBTF_SELECTWEAPON:                  //  Select Weapon
	case FSBTF_BRAKEHOLD:                     //  Brake On While Holding
	case FSBTF_FIREGUN:                       //  Fire Machine Gun
	case FSBTF_SMOKE:                         //  Smoke
		break;

	case FSBTF_TOGGLEALLDOOR:
		ctlLeftDoor=(0.5<ctlLeftDoor ? 0.0 : 1.0);
		ctlRightDoor=ctlLeftDoor;
		ctlRearDoor=ctlLeftDoor;
		break;
	case FSBTF_TOGGLELEFTDOOR:
		ctlLeftDoor=(0.5<ctlLeftDoor ? 0.0 : 1.0);
		break;
	case FSBTF_TOGGLERIGHTDOOR:
		ctlRightDoor=(0.5<ctlRightDoor ? 0.0 : 1.0);
		break;
	case FSBTF_TOGGLEREARDOOR:
		ctlRearDoor=(0.5<ctlRearDoor ? 0.0 : 1.0);
		break;
	}
	return YSERR;
}

void FsFlightControl::CycleNav(void)
{
	if(ctlNavId==0)
	{
		ctlNavId=1;
	}
	else if(ctlNavId==1)
	{
		ctlNavId=100;
	}
	else if(ctlNavId==100)
	{
		ctlNavId=200;
	}
	else
	{
		ctlNavId=0;
	}
}

void FsFlightControl::SelectNav(int navIdIn)
{
	ctlNavId=navIdIn;
}

int FsFlightControl::Nav(void) const
{
	return ctlNavId;
}

void FsFlightControl::SensitivityDown(void)
{
	if(0.501<=ctlSensitivity)
	{
		ctlSensitivity=0.5;
	}
	else if(0.251<=ctlSensitivity)
	{
		ctlSensitivity=0.25;
	}
}

void FsFlightControl::SensitivityUp(void)
{
	if(0.499>=ctlSensitivity)
	{
		ctlSensitivity=0.5;
	}
	else if(0.99>=ctlSensitivity)
	{
		ctlSensitivity=1.0;
	}
}

void FsFlightControl::CycleSensitivity(void)
{
	if(0.999<=ctlSensitivity)
	{
		ctlSensitivity=0.25;
	}
	else if(0.749<=ctlSensitivity)
	{
		ctlSensitivity=1.0;
	}
	else if(0.499<=ctlSensitivity)
	{
		ctlSensitivity=0.75;
	}
	else
	{
		ctlSensitivity=0.5;
	}

}

const double FsFlightControl::GetSensitivity(void) const
{
	return ctlSensitivity;
}

YSRESULT FsFlightControl::ReadControl(const FsControlAssignment &ctlAssign,FsJoystick joy[FsMaxNumJoystick])
{
	FsJoystick pJoy[FsMaxNumJoystick];
	int i,j;
	for(i=0; i<FsMaxNumJoystick; i++)
	{
		pJoy[i]=joy[i];
		for(j=0; j<FsMaxNumJoyAxis; j++)
		{
			pJoy[i].axs[j]+=1.0;
		}
		for(j=0; j<FsMaxNumJoyTrig; j++)
		{
			YsFlip(pJoy[i].trg[j]);
		}
	}
	return ReadControl(ctlAssign,pJoy,joy);
}

YSRESULT FsFlightControl::ReadControl
    (const FsControlAssignment &ctlAssign,FsJoystick pJoy[FsMaxNumJoystick],FsJoystick joy[FsMaxNumJoystick])
{
	// int wid,hei;
	int i;

	// FsGetWindowSize(wid,hei);

	// >> Following Values must be zero unless otherwise something is held.
	pov=0;
	viewHdg=0.0;
	viewPch=0.0;
	// <<

	ctlFireWeaponButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_FIREWEAPON,joy),ctlFireWeaponButtonExt);
	ctlFireGunButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_FIREGUN,joy),ctlFireGunButtonExt);
	ctlFireAAMButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_FIREAAM,joy),ctlFireAAMButtonExt);
	ctlFireAGMButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_FIREAGM,joy),ctlFireAGMButtonExt);
	ctlFireRocketButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_FIREROCKET,joy),ctlFireRocketButtonExt);
	ctlDropBombButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_DROPBOMB,joy),ctlDropBombButtonExt);
	ctlDispenseFlareButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_DISPENSEFLARE,joy),ctlDispenseFlareButtonExt);
	ctlCycleWeaponButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_SELECTWEAPON,joy),ctlCycleWeaponButtonExt);
	ctlSmokeButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_SMOKE,joy),ctlSmokeButtonExt);
	ctlCycleSmokeSelectorButton=YsOr(ctlAssign.IsButtonPressed(FSBTF_CYCLESMOKESELECTOR,joy),ctlCycleSmokeSelectorButtonExt);

	ctlFireWeaponButtonExt=YSFALSE;
	ctlFireGunButtonExt=YSFALSE;
	ctlFireAAMButtonExt=YSFALSE;
	ctlFireAGMButtonExt=YSFALSE;
	ctlFireRocketButtonExt=YSFALSE;
	ctlDropBombButtonExt=YSFALSE;
	ctlDispenseFlareButtonExt=YSFALSE;
	ctlCycleWeaponButtonExt=YSFALSE;
	ctlSmokeButtonExt=YSFALSE;
	ctlCycleSmokeSelectorButtonExt=YSFALSE;


	// 2005/09/20 If mouse is assigned to turret, and left button is not assigned to anything,
	// use it as a fire-gun button.
	if(ctlFireGunButton==YSFALSE)
	{
		int joyId,joyAxs;
		YSBOOL reverse;
		if(ctlAssign.FindAxisByFunction(joyId,joyAxs,reverse,FSAXF_TURRETH)==YSOK &&
		   ctlAssign.TranslateTrigger(joyId,0)==FSBTF_NULL)
		{
			ctlFireGunButton=joy[joyId].trg[0];
		}
	}


	for(i=0; i<FsMaxNumJoystick; i++)
	{
		YSBOOL atLeastSomethingIsAssigned=YSFALSE;
		for(int j=0; j<FsMaxNumJoyAxis; j++)
		{
			if(FSAXF_NULL!=ctlAssign.TranslateAxis(i,j))
			{
				atLeastSomethingIsAssigned=YSTRUE;
				break;
			}
		}

		if(YSTRUE==atLeastSomethingIsAssigned && YSTRUE==ctlAssign.usePovHatSwitch && YSTRUE==joy[i].pov)
		{
			if(YsAbs(joy[i].povAngle)<YsTolerance)
			{
				viewHdg=0.0;
				viewPch=YsPi/2.0;
			}
			else
			{
				viewHdg=-joy[i].povAngle;
			}
			pov=(int)((joy[i].povAngle+YsPi/8.0)/(YsPi/4.0))&7;
			pov++;
		}
	}

	// viewHdg, viewPch may be overridden in SetControlAxis
	// It may happen if POV is explicitly specified in ctlassign.cfg
	double ctlViewX=0.0;
	double ctlViewY=0.0;
	if(PollControlAxis(ctlAssign,ctlViewX,YSTRUE,YSTRUE,FSAXF_POVX,joy)==YSTRUE)
	{
		viewHdg=ctlViewX*YsPi/2.0;
	}
	if(PollControlAxis(ctlAssign,ctlViewX,YSTRUE,YSTRUE,FSAXF_POVX_180DEG,joy)==YSTRUE)
	{
		viewHdg=ctlViewX*YsPi;
	}
	if(PollControlAxis(ctlAssign,ctlViewY,YSTRUE,YSTRUE,FSAXF_POVY,joy)==YSTRUE)
	{
		viewPch=ctlViewY*YsPi/2.0;
	}

                                      // defRev  2side
	SetControlAxis(ctlAssign,ctlElevator,YSFALSE,YSTRUE ,ctlAssign.deadZoneElevator,FSAXF_ELEVATOR,pJoy,joy);
	SetControlAxis(ctlAssign,ctlAileron, YSTRUE, YSTRUE ,ctlAssign.deadZoneAileron,FSAXF_AILERON, pJoy,joy);
	if(SetControlAxis(ctlAssign,ctlThrottle,YSTRUE,YSFALSE,0.0,FSAXF_THROTTLE,pJoy,joy)==YSTRUE)
	{
		if(hasAb==YSTRUE)
		{
			if(ctlThrottle<0.6)
			{
				ctlThrottle=ctlThrottle/0.6;
				ctlAb=YSFALSE;
			}
			else
			{
				ctlAb=YSTRUE;
			}
		}
		else
		{
			ctlAb=YSFALSE;
		}
	}
	SetControlAxis(ctlAssign,ctlPropeller,YSTRUE,YSFALSE,0.0,FSAXF_PROPELLER,pJoy,joy);
	SetControlAxis(ctlAssign,ctlRudder, YSTRUE, YSTRUE ,ctlAssign.deadZoneRudder, FSAXF_RUDDER,pJoy,joy);
	SetControlAxis(ctlAssign,ctlFlap,   YSFALSE,YSFALSE,0.0,FSAXF_FLAP,  pJoy,joy);
	SetControlAxis(ctlAssign,ctlGear,   YSFALSE,YSFALSE,0.0,FSAXF_LANDINGGEAR,pJoy,joy);
	SetControlAxis(ctlAssign,ctlElvTrim,YSFALSE,YSTRUE ,0.0,FSAXF_TRIM,pJoy,joy);
	SetControlAxis(ctlAssign,ctlThrVec ,YSFALSE,YSFALSE,0.0,FSAXF_NOZZLE,pJoy,joy);
	SetControlAxis(ctlAssign,ctlVgw    ,YSFALSE,YSFALSE,0.0,FSAXF_VGW,pJoy,joy);

	SetControlAxis(ctlAssign,ctlSpoiler,YSTRUE ,YSFALSE,0.0,FSAXF_SPOILERBRAKE,pJoy,joy);
	SetControlAxis(ctlAssign,ctlBrake  ,YSTRUE ,YSFALSE,0.0,FSAXF_SPOILERBRAKE,pJoy,joy);
	SetControlAxis(ctlAssign,ctlSpoiler,YSTRUE ,YSFALSE,0.0,FSAXF_SPOILER,pJoy,joy);
	SetControlAxis(ctlAssign,ctlBrake  ,YSTRUE ,YSFALSE,0.0,FSAXF_BRAKE,pJoy,joy);

	SetControlAxis(ctlAssign,ctlTurretHdg,YSTRUE,YSTRUE,0.0,FSAXF_TURRETH,pJoy,joy);
	SetControlAxis(ctlAssign,ctlTurretPch,YSTRUE,YSTRUE,0.0,FSAXF_TURRETP,pJoy,joy);


	if(ctlAssign.IsButtonPressed(FSBTF_VGWEXTEND,joy)==YSTRUE)
	{
		ctlVgw=1.0;
		ctlAutoVgw=YSFALSE;
	}
	else if(ctlAssign.IsButtonPressed(FSBTF_VGWRETRACT,joy)==YSTRUE)
	{
		ctlVgw=0.0;
		ctlAutoVgw=YSFALSE;
	}
	else
	{
		ctlAutoVgw=YSTRUE;
	}

	if(ctlAssign.IsButtonPressed(FSBTF_REVERSETHRUST,joy)==YSTRUE)
	{
		ctlThrRev=1.0;
	}
	else
	{
		ctlThrRev=0.0;
	}

	if(brakeHold!=ctlAssign.IsButtonPressed(FSBTF_BRAKEHOLD,joy))
	{
		YsFlip(brakeHold);
		ctlBrake=(brakeHold==YSTRUE ? 1.0 : 0.0);
	}
	if(spoilerHold!=ctlAssign.IsButtonPressed(FSBTF_SPOILERHOLD,joy))
	{
		YsFlip(spoilerHold);
		ctlSpoiler=(spoilerHold==YSTRUE ? 1.0 : 0.0);
	}


	int vx,vy,vz;
	YsVec3 viewVec;
	YsAtt3 viewAtt;
	vx=0;
	vy=0;
	vz=0;

	if(ctlAssign.IsButtonPressed(FSBTF_LOOKFORWARD,joy)==YSTRUE)
	{
		vz+=1;
		pov=1;
	}
	if(ctlAssign.IsButtonPressed(FSBTF_LOOKBACK,joy)==YSTRUE)
	{
		vz-=1;
		pov=5;
	}
	if(ctlAssign.IsButtonPressed(FSBTF_LOOKLEFT,joy)==YSTRUE)
	{
		vx-=1;
		pov=7;
	}
	if(ctlAssign.IsButtonPressed(FSBTF_LOOKRIGHT,joy)==YSTRUE)
	{
		vx+=1;
		pov=3;
	}
	if(ctlAssign.IsButtonPressed(FSBTF_LOOKUP,joy)==YSTRUE)
	{
		vy+=1;
	}
	if(ctlAssign.IsButtonPressed(FSBTF_LOOKDOWN,joy)==YSTRUE)
	{
		vy-=1;
	}

	if(vx!=0 || vy!=0 || vz!=0)
	{
		viewVec.Set(vx,vy,vz);
		viewAtt.Set(0.0,0.0,0.0);
		viewAtt.SetForwardVector(viewVec);
		viewHdg=viewAtt.h();
		viewPch=viewAtt.p();
	}

	return YSOK;
}

YSBOOL FsFlightControl::SetControlAxis
   (const FsControlAssignment &ctlAssign,
    double &newValue,YSBOOL defReverse,YSBOOL twoSide,const double &deadZone,FSAXISFUNCTION fnc,
    FsJoystick pJoy[FsMaxNumJoystick],FsJoystick joy[FsMaxNumJoystick])
{
	int joyId,joyAxs;
	YSBOOL reverse;

	if(ctlAssign.FindAxisByFunction(joyId,joyAxs,reverse,fnc)==YSOK)
	{
		double pAxs,axs;
		pAxs=pJoy[joyId].axs[joyAxs];
		axs=joy[joyId].axs[joyAxs];

		if(YsAbs(pAxs-axs)>YsTolerance)
		{
			if(reverse!=defReverse)
			{
				axs=1.0-axs;
			}

			if(fnc!=FSAXF_TURRETH && fnc!=FSAXF_TURRETP)
			{
				if(twoSide!=YSTRUE)
				{
					axs=YsBound(Margin(axs,deadZone),0.0,1.0);
				}
				else
				{
					axs=YsBound(Margin((axs-0.5)*2.0,deadZone),-1.0,1.0);
				}
			}
			else
			{
				if(twoSide!=YSTRUE)
				{
					axs=YsBound(axs,0.0,1.0);
				}
				else
				{
					axs=YsBound((axs-0.5)*2.0,-1.0,1.0);
				}
			}

			newValue=axs;

			/* if(useDeadZone==YSTRUE)
			{
				if(newValue>0.05)
				{
					newValue=(newValue-0.05)/0.95;
					newValue=newValue*newValue;
				}
				else  if(newValue<-0.05)
				{
					newValue=(newValue+0.05)/0.95;
					newValue=-newValue*newValue;
				}
				else
				{
					newValue=0.0;
				}
			} */

			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsFlightControl::PollControlAxis
   (const FsControlAssignment &ctlAssign,
    double &newValue,YSBOOL defReverse,YSBOOL twoSide,FSAXISFUNCTION fnc,
    const FsJoystick joy[FsMaxNumJoystick]) const
{
	int joyId,joyAxs;
	YSBOOL reverse;

	if(ctlAssign.FindAxisByFunction(joyId,joyAxs,reverse,fnc)==YSOK)
	{
		double axs;
		axs=joy[joyId].axs[joyAxs];

		if(reverse!=defReverse)
		{
			axs=1.0-axs;
		}

		if(twoSide!=YSTRUE)
		{
			axs=YsBound(Margin(axs,0.03),0.0,1.0);
		}
		else
		{
			axs=YsBound(Margin((axs-0.5)*2.0,0.03),-1.0,1.0);
		}
		newValue=axs;
		return YSTRUE;
	}
	return YSFALSE;
}

void FsFlightControl::Move(FsControlAssignment &ctlAssign,const FsJoystick joy[FsMaxNumJoystick],const double &dt)
{
	int joyId,joyAxs;
	YSBOOL reverse;
	if(ctlAssign.FindAxisByFunction(joyId,joyAxs,reverse,FSAXF_AILERON)!=YSOK)
	{
		if(ctlAileron>0.0)
		{
			ctlAileron-=0.1*dt;
			if(ctlAileron<0.0)
			{
				ctlAileron=0.0;
			}
		}
		else if(ctlAileron<0.0)
		{
			ctlAileron+=0.1*dt;
			if(ctlAileron>0.0)
			{
				ctlAileron=0.0;
			}
		}
	}

	double axs;
	if(YSTRUE==PollControlAxis(ctlAssign,axs,YSFALSE,YSTRUE,FSAXF_THROTTLE_UPDOWN,joy))
	{
		ctlThrottle+=axs*dt;
		if(YSTRUE==hasAb)
		{
			if(1.0<ctlThrottle)
			{
				if(YSTRUE!=ctlAb)
				{
					ctlThrottle=0.6;
					ctlAb=YSTRUE;
				}
				else
				{
					ctlThrottle=1.0;
				}
			}
			else if(ctlThrottle<0.6 && YSTRUE==ctlAb)
			{
				ctlThrottle=1.0;
				ctlAb=YSFALSE;
			}
			else if(ctlThrottle<0.0)
			{
				ctlThrottle=0.0;
			}
		}
		else
		{
			if(1.0<ctlThrottle)
			{
				ctlThrottle=1.0;
			}
			else if(ctlThrottle<0.0)
			{
				ctlThrottle=0.0;
			}
		}
	}
}

double FsFlightControl::Margin(double org,const double deadZone) const
{
	if(org>deadZone)
	{
		org=(org-deadZone)/(1.0-deadZone);
	}
	else if(org<-deadZone)
	{
		org=(org+deadZone)/(1.0-deadZone);
	}
	else
	{
		org=0.0;
	}
	return org*fabs(org);
}



YSRESULT FsFlightControl::VerifyAndFixJoystickAxisAssignment(FsControlAssignment &ctlAssign)
{
	YSRESULT res=YSOK;  // Return YSERR if joystick axis is missing.

	int joyId,joyAxs;
	YSBOOL needRemap=YSFALSE;
	YSBOOL reverse;

	if(ctlAssign.FindAxisByFunction(joyId,joyAxs,reverse,FSAXF_ELEVATOR)==YSOK && joyId!=FsMouseJoyId)
	{
		if(FsIsJoystickAxisAvailable(joyId,joyAxs)!=YSTRUE)
		{
			res=YSERR;
			if(ctlAssign.FindKeyByFunction(FSBTF_ELEVATORUP)==FSKEY_NULL ||
			   ctlAssign.FindKeyByFunction(FSBTF_ELEVATORDOWN)==FSKEY_NULL)
			{
				ctlAssign.DeleteAxis(joyId,joyAxs);
				ctlAssign.DeleteAxis(FsMouseJoyId,1);  // 2005/06/29

				ctlAssign.AddAxisAssignment(FsMouseJoyId,1,FSAXF_ELEVATOR,YSFALSE);
				ctlAssign.AddTriggerAssignment(FsMouseJoyId,0,FSBTF_FIREWEAPON);
				ctlAssign.AddTriggerAssignment(FsMouseJoyId,1,FSBTF_SELECTWEAPON);
				ctlAssign.AddTriggerAssignment(FsMouseJoyId,2,FSBTF_RADAR);

				ctlAssign.AddKeyAssignment(FSKEY_LEFT, FSBTF_TURRETLEFT);
				ctlAssign.AddKeyAssignment(FSKEY_RIGHT,FSBTF_TURRETRIGHT);
				ctlAssign.AddKeyAssignment(FSKEY_UP,   FSBTF_TURRETUP);
				ctlAssign.AddKeyAssignment(FSKEY_DOWN, FSBTF_TURRETDOWN);

				needRemap=YSTRUE;
			}
		}
	}

	if(ctlAssign.FindAxisByFunction(joyId,joyAxs,reverse,FSAXF_AILERON)==YSOK && joyId!=FsMouseJoyId)
	{
		if(FsIsJoystickAxisAvailable(joyId,joyAxs)!=YSTRUE)
		{
			res=YSERR;
			if(ctlAssign.FindKeyByFunction(FSBTF_AILERONLEFT)==FSKEY_NULL ||
			   ctlAssign.FindKeyByFunction(FSBTF_AILERONRIGHT)==FSKEY_NULL)
			{
				ctlAssign.DeleteAxis(joyId,joyAxs);
				ctlAssign.DeleteAxis(FsMouseJoyId,0);  // 2005/06/29

				ctlAssign.AddAxisAssignment(FsMouseJoyId,0,FSAXF_AILERON,YSFALSE);
				ctlAssign.AddTriggerAssignment(FsMouseJoyId,0,FSBTF_FIREWEAPON);
				ctlAssign.AddTriggerAssignment(FsMouseJoyId,1,FSBTF_SELECTWEAPON);
				ctlAssign.AddTriggerAssignment(FsMouseJoyId,2,FSBTF_RADAR);

				needRemap=YSTRUE;
			}
		}
	}

	for(int i=0; i<(int)FSAXF_NUMAXISFUNCTION; i++)
	{
		if(i!=(int)FSAXF_ELEVATOR &&
		   i!=(int)FSAXF_AILERON &&
		   ctlAssign.FindAxisByFunction(joyId,joyAxs,reverse,(FSAXISFUNCTION)i)==YSOK)
		{
			if(FsIsJoystickAxisAvailable(joyId,joyAxs)!=YSTRUE)
			{
				ctlAssign.DeleteAxisFunction((FSAXISFUNCTION)i);
				ctlAssign.DeleteAxis(joyId,joyAxs);
				needRemap=YSTRUE;
			}
		}
	}

	if(needRemap==YSTRUE)
	{
		ctlAssign.BuildMapping();
	}

	return res;
}

////////////////////////////////////////////////////////////



static FsVisualDnm *stickDnm=NULL;
static FsVisualDnm *throttleDnm=NULL;
static FsVisualDnm *rudderDnm=NULL;

void FsFlightControl::PrepareJoystickPolygonModel(void)
{
	if(stickDnm==NULL)
	{
		stickDnm=new FsVisualDnm;
		if(stickDnm->Load(L"misc/stick.dnm")!=YSOK)
		{
			delete stickDnm;
			stickDnm=NULL;
		}
	}

	if(throttleDnm==NULL)
	{
		throttleDnm=new FsVisualDnm;
		if(throttleDnm->Load(L"misc/throttle.dnm")!=YSOK)
		{
			delete throttleDnm;
			throttleDnm=NULL;
		}
	}

	if(rudderDnm==NULL)
	{
		rudderDnm=new FsVisualDnm;
		if(rudderDnm->Load(L"misc/rudder.dnm")!=YSOK)
		{
			delete rudderDnm;
			rudderDnm=NULL;
		}
	}
}

void FsFlightControl::DrawJoystick(const YsVec3 &pos,const YsAtt3 &att) const
{
	PrepareJoystickPolygonModel();
	if(stickDnm!=NULL)
	{
		if(ctlElevator>0.0)
		{
			stickDnm->SetState(6,0,1,ctlElevator);
		}
		else
		{
			stickDnm->SetState(6,0,2,-ctlElevator);
		}
		if(ctlAileron>0.0)
		{
			stickDnm->SetState(7,0,1,ctlAileron);
		}
		else
		{
			stickDnm->SetState(7,0,2,-ctlAileron);
		}
		stickDnm->CacheTransformation();
		stickDnm->Draw(pos,att);
	}
}

void FsFlightControl::DrawThrottle(const YsVec3 &pos,const YsAtt3 &att) const
{
	PrepareJoystickPolygonModel();
	if(throttleDnm!=NULL)
	{
		throttleDnm->SetState(0,0,1,ctlThrottle);
		throttleDnm->CacheTransformation();;
		throttleDnm->Draw(pos,att);
	}
}

void FsFlightControl::DrawRudder(const YsVec3 &pos,const YsAtt3 &att) const
{
	PrepareJoystickPolygonModel();
	if(rudderDnm!=NULL)
	{
		if(ctlRudder>0.0)
		{
			rudderDnm->SetState(8,0,1,ctlRudder);
		}
		else
		{
			rudderDnm->SetState(8,0,2,-ctlRudder);
		}
		rudderDnm->CacheTransformation();
		rudderDnm->Draw(pos,att);
	}
}

YSRESULT FsFlightControl::CenterJoystick(FsControlAssignment &ctlAssign)
{
	FsCenterJoystick centerJoystick;
	centerJoystick.Initialize(this,&ctlAssign,0);

	while(FsCenterJoystick::OVER!=centerJoystick.state)
	{
		centerJoystick.RunOneStep();
	}

	return centerJoystick.res;
}

////////////////////////////////////////////////////////////

struct FsAxisFunctionString
{
	FSAXISFUNCTION fnc;
	const char *str;
	const char *label;
};

struct FsButtonFunctionString
{
	FSBUTTONFUNCTION fnc;
	const char *str;
	const char *label;
};

struct FsKeyString
{
	int keyCode;
	const char *str;
	const char *label;
};

static struct FsAxisFunctionString fsAxisFuncStr[]=
{
	{FSAXF_NULL,                 "NULL",                "NULL"},
	{FSAXF_AILERON,              "AILERON",             "Aileron"},
	{FSAXF_ELEVATOR,             "ELEVATOR",            "Elevator"},
	{FSAXF_THROTTLE,             "THROTTLE",            "Throttle"},
	{FSAXF_PROPELLER,            "PROPELLER",           "Propeller Control"},
	{FSAXF_RUDDER,               "RUDDER",              "Rudder"},
	{FSAXF_FLAP,                 "FLAP",                "Flap"},
	{FSAXF_LANDINGGEAR,          "LANDINGGEAR",         "Landing Gear"},
	{FSAXF_TRIM,                 "TRIM",                "Elevator Trim"},
	{FSAXF_NOZZLE,               "NOZZLE",              "VTOL Nozzle Up/Down (Nose for Concorde)"},
	{FSAXF_VGW,                  "VGW",                 "Variable Geometry Wing"},
	{FSAXF_SPOILERBRAKE,         "SPOILERBRAKE",        "AirBrake + Brake"},
	{FSAXF_SPOILER,              "SPOILER",             "AirBrake"},
	{FSAXF_BRAKE,                "BRAKE",               "Brake"},
	{FSAXF_POVX,                 "POVX",                "X Axis of Point of View (+-90deg)"},
	{FSAXF_POVX_180DEG,          "POVX180",             "X Axis of Point of View (+-180deg)"},
	{FSAXF_POVY,                 "POVY",                "Y Axis of Point of View"},
	{FSAXF_TURRETH,              "TURRETHDG",           "Rotating Turret Heading"},
	{FSAXF_TURRETP,              "TURRETPCH",           "Rotating Turret Pitch"},
	{FSAXF_THROTTLE_UPDOWN,      "THROTTLEUPDOWN",      "Increase/Decrease Throttle"},
};

static struct FsButtonFunctionString fsButtonFuncStr[]=
{
	{FSBTF_NULL,                 "NULL",                 "NULL"},
	{FSBTF_ELEVATORUP,           "ELEVATORUP",           "Elevator Up"},
	{FSBTF_ELEVATORNEUTRAL,      "ELEVATORNEUTRAL",      "Elevator Neutral"},
	{FSBTF_ELEVATORDOWN,         "ELEVATORDOWN",         "Elevator Down"},
	{FSBTF_AILERONLEFT,          "AILERONLEFT",          "Aileron Left"},
	{FSBTF_AILERONNEUTRAL,       "AILERONNEUTRAL",       "Aileron Neutral"},
	{FSBTF_AILERONRIGHT,         "AILERONRIGHT",         "Aileron Right"},
	{FSBTF_RUDDERLEFT,           "RUDDERLEFT",           "Rudder Left"},
	{FSBTF_RUDDERCENTER,         "RUDDERCENTER",         "Rudder Center"},
	{FSBTF_RUDDERRIGHT,          "RUDDERRIGHT",          "Rudder Right"},
	{FSBTF_TRIMUP,               "TRIMUP",               "Elevator Trim Up"},
	{FSBTF_TRIMDOWN,             "TRIMDOWN",             "Elevator Trim Down"},
	{FSBTF_AUTOTRIM,             "AUTOTRIM",             "Auto Trim"},
	{FSBTF_THROTTLEUP,           "THROTTLEUP",           "Throttle Add Power(5%)"},
	{FSBTF_THROTTLEUP_HALF,      "THROTTLEUPHALF",       "Throttle Add Power(2.5%)"},
	{FSBTF_THROTTLEDOWN,         "THROTTLEDOWN",         "Throttle Reduce Power(5%)"},
	{FSBTF_THROTTLEDOWN_HALF,    "THROTTLEDOWNHALF",     "Throttle Reduce Power(2.5%)"},
	{FSBTF_THROTTLEMAX,          "THROTTLEMAX",          "Throttle Max"},
	{FSBTF_THROTTLEIDLE,         "THROTTLEIDLE",         "Throttle Min (Idle)"},
	{FSBTF_PROPFORWARD,          "PROPFORWARD",          "Increase RPM"},
	{FSBTF_PROPBACKWARD,         "PROPBACKWARD",         "Decrease RPM"},
	{FSBTF_CYCLESMOKESELECTOR,   "CYCLESMOKESELECTOR",   "Cycle Smoke Selector"},
	{FSBTF_AFTERBURNER,          "AFTERBURNER",          "Afterburner"},
	{FSBTF_NOZZLEUP,             "NOZZLEUP",             "Nozzle Up (Nose Up for Concorde)"},
	{FSBTF_NOZZLEDOWN,           "NOZZLEDOWN",           "Nozzle Down (Nose Down for Concorde)"},
	{FSBTF_VGWEXTEND,            "VGWEXTEND",            "Extend Variable Geometry Wing"},
	{FSBTF_VGWRETRACT,           "VGWRETRACT",           "Retract Variable Geometry Wing"},
	{FSBTF_LANDINGGEAR,          "LANDINGGEAR",          "Landing Gear Extend/Retract"},
	{FSBTF_FLAP,                 "FLAP",                 "Flap Up/Down"},
	{FSBTF_FLAPUP,               "FLAPUP",               "Flap Up"},
	{FSBTF_FLAPDOWN,             "FLAPDOWN",             "Flap Down"},
	{FSBTF_FLAPFULLUP,           "FLAPFULLUP",           "Flap Full Up"},
	{FSBTF_FLAPFULLDOWN,         "FLAPFULLDOWN",         "Flap Full Down"},
	{FSBTF_SPOILERBRAKE,         "SPOILERBRAKE",         "Spoiler and Brake On/Off"},
	{FSBTF_SPOILER,              "SPOILER",              "Spoiler Extend/Retract"},
	{FSBTF_SPOILEREXTEND,        "SPOILEREXTEND",        "Spoiler Extend"},
	{FSBTF_SPOILERRETRACT,       "SPOILERRETRACT",       "Spoiler Retract"},
	{FSBTF_SPOILERHOLD,          "SPOILERHOLD",          "Spoiler On When Pressed"},
	{FSBTF_BRAKEONOFF,           "BRAKEONOFF",           "Brake On/Off"},
	{FSBTF_BRAKEHOLD,            "BRAKEHOLD",            "Brake On When Pressed"},
	{FSBTF_FIREWEAPON,           "FIREWEAPON",           "Fire Selected Weapon"},
	{FSBTF_FIREGUN,              "FIREGUN",              "Fire Machine Gun"},
	{FSBTF_FIREAAM,              "FIREAAM",              "Fire AAM"},
	{FSBTF_FIREAGM,              "FIREAGM",              "Fire AGM"},
	{FSBTF_FIREROCKET,           "FIREROCKET",           "Fire Rocket"},
	{FSBTF_DROPBOMB,             "DROPBOMB",             "Drop Bomb"},
	{FSBTF_DISPENSEFLARE,        "DISPENSEFLARE",        "Dispense Flare"},
	{FSBTF_SMOKE,                "SMOKE",                "Smoke"},
	{FSBTF_SELECTWEAPON,         "SELECTWEAPON",         "Select Weapon"},
	{FSBTF_RADAR,                "RADAR",                "Radar"},
	{FSBTF_RADARRANGEUP,         "RADARRANGEUP",         "Radar Range Up"},
	{FSBTF_RADARRANGEDOWN,       "RADARRANGEDOWN",       "Radar Range Down"},
	{FSBTF_ILS,                  "ILS",                  "Switch NAV 1/2"},
	{FSBTF_VELOCITYINDICATOR,    "VELOCITYINDICATOR",    "Velocity Indicator On/Off"},
	{FSBTF_OPENAUTOPILOTMENU,    "OPENAUTOPILOTMENU",    "Open Autopilot Menu"},
	{FSBTF_OPENRADIOCOMMMENU,    "OPENRADIOCOMMMENU",    "Open Radio Comm Menu"},

	{FSBTF_COCKPITVIEW,          "COCKPITVIEW",          "Cockpit View"},
	{FSBTF_OUTSIDEPLAYERVIEW,    "OUTSIDEPLAYERVIEW",    "Outside Player View"},
	{FSBTF_COMPUTERAIRPLANEVIEW, "COMPUTERAIRPLANEVIEW", "Computer Airplane View"},
	{FSBTF_WEAPONVIEW,           "WEAPONVIEW",           "Weapon View"},
	{FSBTF_ILSVIEW,              "ILSVIEW",              "ILS/Control-Tower View"},
	{FSBTF_OUTSIDEPLAYERVIEW2,   "OUTSIDEPLAYERVIEW2",   "Outside-Aircraft View"},
	{FSBTF_OUTSIDEPLAYERVIEW3,   "OUTSIDEPLAYERVIEW3",   "Outside-Aircraft View"},
	{FSBTF_CONTROLTOWERVIEW,     "CONTROLTOWERVIEW",     "ILS/Control-Tower View"},
	{FSBTF_CHANGEAIRPLANE,       "CHANGEAIRPLANE",       "Change Airplane in Replay Record Mode"},
	{FSBTF_GHOSTVIEW,            "GHOSTVIEW",            "Ghost View"},

	{FSBTF_LOOKFORWARD,          "LOOKFORWARD",          "Look Forward"},
	{FSBTF_LOOKRIGHT,            "LOOKRIGHT",            "Look Right"},
	{FSBTF_LOOKLEFT,             "LOOKLEFT",             "Look Left"},
	{FSBTF_LOOKBACK,             "LOOKBACK",             "Look Back"},
	{FSBTF_LOOKUP,               "LOOKUP",               "Look Up"},
	{FSBTF_LOOKDOWN,             "LOOKDOWN",             "Look Down"},

	{FSBTF_REVERSETHRUST,        "REVERSETHRUST",        "Reverse Thrust"},
	{FSBTF_OPENSUBWINDOWMENU,    "OPENSUBWINDOWMENU",    "Open Sub-Window Menu"},

	{FSBTF_CHANGEHUDCOLOR,       "CHANGEHUDCOLOR",       "Change HUD Color"},
	{FSBTF_PAUSE,                "PAUSESIMULATION",      "Pause Simulation"},

	{FSBTF_OPENVORMENU,          "OPENVORMENU",          "Select VOR"},
	{FSBTF_ROTATEVORLEFT,        "ROTATEVORLEFT",        "Rotate VOR to the Left"},
	{FSBTF_ROTATEVORRIGHT,       "ROTATEVORRIGHT",       "Rotate VOR to the Right"},

	{FSBTF_BOMBBAYDOOR,          "BOMBBAYDOOR",          "Open/Close Bomb Bay Door"},

	{FSBTF_INFLTCONFIG,          "INFLIGHTCFG",          "In-Flight Config Change"},
	{FSBTF_INFLTMESSAGE,         "INFLTMESSAGE",         "Send Message Over Network"},

	{FSBTF_TURRETRIGHT,          "TURRETRIGHT",          "Rotate Turret Right"},
	{FSBTF_TURRETLEFT,           "TURRETLEFT",           "Rotate Turret Left"},
	{FSBTF_TURRETUP,             "TURRETUP",             "Rotate Turret Up"},
	{FSBTF_TURRETDOWN,           "TURRETDOWN",           "Rotate Turret Down"},
	{FSBTF_TURRETNEUTRAL,        "TURRETNEUTRAL",        "Move Turret to the Neutral Position"},

	{FSBTF_VIEWZOOM,             "VIEWZOOM",             "Zoom In"},
	{FSBTF_VIEWMOOZ,             "VIEWMOOZ",             "Zoom Out"},

	{FSBTF_OPENADFMENU,          "OPENADFMENU",          "Open ADF Menu"},

	{FSBTF_SUPPLYDIALOG,         "OPENSUPPLYDLG",        "Open Supply Dialog"},

	{FSBTF_TOGGLELIGHT,          "TOGGLELIGHT",          "Turn On/Off Lights"},
	{FSBTF_TOGGLENAVLIGHT,       "TOGGLENAVLIGHT",       "Turn On/Off Nav Lights"},
	{FSBTF_TOGGLEBEACON,         "TOGGLEBEACON",         "Turn On/Off Beacon"},
	{FSBTF_TOGGLESTROBE,         "TOGGLESTROBE",         "Turn On/Off Strobe"},
	{FSBTF_TOGGLELANDINGLIGHT,   "TOGGLELANDINGLIGHT",   "Turn On/Off Landing Lights"},

	{FSBTF_TOGGLEALLDOOR,        "TOGGLEALLDOOR",        "Open/Close All Doors"},
	{FSBTF_TOGGLELEFTDOOR,       "TOGGLELEFTDOOR",       "Open/Close Left Door"},
	{FSBTF_TOGGLERIGHTDOOR,      "TOGGLERIGHTDOOR",      "Open/Close Right Door"},
	{FSBTF_TOGGLEREARDOOR,       "TOGGLEREARDOOR",       "Open/Close Rear Door"},

	{FSBTF_CYCLESENSITIVITY,     "CYCLESENSITIVITY",     "Cycle Sensitivity"},
	{FSBTF_SENSITIVITYUP,        "SENSITIVITYUP",        "Increase Sensitivity"},
	{FSBTF_SENSITIVITYDOWN,      "SENSITIVITYDOWN",      "Decrease Sensitivity"},

	{FSBTF_SWITCHVIEWTARGET,     "SWITCHVIEWTARGET",     "Switch View Target"}
};

static struct FsKeyString fsKeyString[]=
{
	{FSKEY_SPACE,        "SPACE",        "Space"},
	{FSKEY_0,            "0",            "0"},
	{FSKEY_1,            "1",            "1"},
	{FSKEY_2,            "2",            "2"},
	{FSKEY_3,            "3",            "3"},
	{FSKEY_4,            "4",            "4"},
	{FSKEY_5,            "5",            "5"},
	{FSKEY_6,            "6",            "6"},
	{FSKEY_7,            "7",            "7"},
	{FSKEY_8,            "8",            "8"},
	{FSKEY_9,            "9",            "9"},
	{FSKEY_A,            "A",            "A"},
	{FSKEY_B,            "B",            "B"},
	{FSKEY_C,            "C",            "C"},
	{FSKEY_D,            "D",            "D"},
	{FSKEY_E,            "E",            "E"},
	{FSKEY_F,            "F",            "F"},
	{FSKEY_G,            "G",            "G"},
	{FSKEY_H,            "H",            "H"},
	{FSKEY_I,            "I",            "I"},
	{FSKEY_J,            "J",            "J"},
	{FSKEY_K,            "K",            "K"},
	{FSKEY_L,            "L",            "L"},
	{FSKEY_M,            "M",            "M"},
	{FSKEY_N,            "N",            "N"},
	{FSKEY_O,            "O",            "O"},
	{FSKEY_P,            "P",            "P"},
	{FSKEY_Q,            "Q",            "Q"},
	{FSKEY_R,            "R",            "R"},
	{FSKEY_S,            "S",            "S"},
	{FSKEY_T,            "T",            "T"},
	{FSKEY_U,            "U",            "U"},
	{FSKEY_V,            "V",            "V"},
	{FSKEY_W,            "W",            "W"},
	{FSKEY_X,            "X",            "X"},
	{FSKEY_Y,            "Y",            "Y"},
	{FSKEY_Z,            "Z",            "Z"},
	{FSKEY_ESC,          "ESC",          "Esc"},
	{FSKEY_F1,           "F1",           "F1"},
	{FSKEY_F2,           "F2",           "F2"},
	{FSKEY_F3,           "F3",           "F3"},
	{FSKEY_F4,           "F4",           "F4"},
	{FSKEY_F5,           "F5",           "F5"},
	{FSKEY_F6,           "F6",           "F6"},
	{FSKEY_F7,           "F7",           "F7"},
	{FSKEY_F8,           "F8",           "F8"},
	{FSKEY_F9,           "F9",           "F9"},
	{FSKEY_F10,          "F10",          "F10"},
	{FSKEY_F11,          "F11",          "F11"},
	{FSKEY_F12,          "F12",          "F12"},
	{FSKEY_CAPSLOCK,     "CAPSLOCK",     "Caps Lock"},
	{FSKEY_SCROLLLOCK,   "SCROLLLOCK",   "Scroll Lock"},
	{FSKEY_PAUSEBREAK,   "PAUSEBREAK",   "Pause"},
	{FSKEY_BS,           "BS",           "Backspace"},
	{FSKEY_TAB,          "TAB",          "Tab"},
	{FSKEY_ENTER,        "ENTER",        "Enter"},
	{FSKEY_SHIFT,        "SHIFT",        "Shift"},
	{FSKEY_CTRL,         "CTRL",         "Ctrl"},
	{FSKEY_INS,          "INS",          "Insert"},
	{FSKEY_DEL,          "DEL",          "Delete"},
	{FSKEY_HOME,         "HOME",         "Home"},
	{FSKEY_END,          "END",          "End"},
	{FSKEY_PAGEUP,       "PAGEUP",       "Page Up"},
	{FSKEY_PAGEDOWN,     "PAGEDOWN",     "Page Down"},
	{FSKEY_UP,           "UP",           "Up Arrow"},
	{FSKEY_DOWN,         "DOWN",         "Down Arrow"},
	{FSKEY_LEFT,         "LEFT",         "Left Arrow"},
	{FSKEY_RIGHT,        "RIGHT",        "Right Arrow"},
	{FSKEY_NUMLOCK,      "NUMLOCK",      "Num Lock"},
	{FSKEY_TILDA,        "TILDA",        "~"},
	{FSKEY_MINUS,        "MINUS",        "-"},
	{FSKEY_PLUS,         "PLUS",         "+"},
	{FSKEY_LBRACKET,     "LBRACKET",     "["},
	{FSKEY_RBRACKET,     "RBRACKET",     "]"},
	{FSKEY_BACKSLASH,    "BACKSLASH",    "\\"},
	{FSKEY_SEMICOLON,    "SEMICOLON",    ";"},
	{FSKEY_SINGLEQUOTE,  "SINGLEQUOTE",  "'"},
	{FSKEY_COMMA,        "COMMA",        ","},
	{FSKEY_DOT,          "DOT",          "."},
	{FSKEY_SLASH,        "SLASH",        "/"},
	{FSKEY_TEN0,         "TEN0",         "0 (NUMPAD)"},
	{FSKEY_TEN1,         "TEN1",         "1 (NUMPAD)"},
	{FSKEY_TEN2,         "TEN2",         "2 (NUMPAD)"},
	{FSKEY_TEN3,         "TEN3",         "3 (NUMPAD)"},
	{FSKEY_TEN4,         "TEN4",         "4 (NUMPAD)"},
	{FSKEY_TEN5,         "TEN5",         "5 (NUMPAD)"},
	{FSKEY_TEN6,         "TEN6",         "6 (NUMPAD)"},
	{FSKEY_TEN7,         "TEN7",         "7 (NUMPAD)"},
	{FSKEY_TEN8,         "TEN8",         "8 (NUMPAD)"},
	{FSKEY_TEN9,         "TEN9",         "9 (NUMPAD)"},
	{FSKEY_TENDOT,       "TENDOT",       ". (NUMPAD)"},
	{FSKEY_TENSLASH,     "TENSLASH",     "/ (NUMPAD)"},
	{FSKEY_TENSTAR,      "TENSTAR",      "* (NUMPAD)"},
	{FSKEY_TENMINUS,     "TENMINUS",     "- (NUMPAD)"},
	{FSKEY_TENPLUS,      "TENPLUS",      "+ (NUMPAD)"},
	{FSKEY_TENENTER,     "TENENTER",     "Enter (NUMPAD)"},
	{FSKEY_WHEELUP,      "WHEELUP",      "Mouse Wheel Up"},
	{FSKEY_WHEELDOWN,    "WHEELDOWN",    "Mouse Wheel Down"}
};

const char *FsGetAxisFuncLabel(FSAXISFUNCTION fnc)
{
	int i;
	for(i=0; i<sizeof(fsAxisFuncStr)/sizeof(struct FsAxisFunctionString); i++)
	{
		if(fsAxisFuncStr[i].fnc==fnc)
		{
			return fsAxisFuncStr[i].label;
		}
	}
	return NULL;
}

const char *FsGetAxisFuncString(FSAXISFUNCTION fnc)
{
	int i;
	for(i=0; i<sizeof(fsAxisFuncStr)/sizeof(struct FsAxisFunctionString); i++)
	{
		if(fsAxisFuncStr[i].fnc==fnc)
		{
			return fsAxisFuncStr[i].str;
		}
	}
	return NULL;
}

const char *FsGetButtonFuncLabel(FSBUTTONFUNCTION fnc)
{
	int i;
	for(i=0; i<sizeof(fsButtonFuncStr)/sizeof(struct FsButtonFunctionString); i++)
	{
		if(fsButtonFuncStr[i].fnc==fnc)
		{
			return fsButtonFuncStr[i].label;
		}
	}
	return NULL;
}

const char *FsGetButtonFuncString(FSBUTTONFUNCTION fnc)
{
	int i;
	for(i=0; i<sizeof(fsButtonFuncStr)/sizeof(struct FsButtonFunctionString); i++)
	{
		if(fsButtonFuncStr[i].fnc==fnc)
		{
			return fsButtonFuncStr[i].str;
		}
	}
	return NULL;
}

const char *FsGetKeyLabel(int keyCode)
{
	int i;
	for(i=0; i<sizeof(fsKeyString)/sizeof(struct FsKeyString); i++)
	{
		if(fsKeyString[i].keyCode==keyCode)
		{
			return fsKeyString[i].label;
		}
	}
	return NULL;
}

const char *FsGetKeyString(int keyCode)
{
	int i;
	for(i=0; i<sizeof(fsKeyString)/sizeof(struct FsKeyString); i++)
	{
		if(fsKeyString[i].keyCode==keyCode)
		{
			return fsKeyString[i].str;
		}
	}
	return NULL;
}

FSAXISFUNCTION FsGetAxisFuncFromString(const char str[])
{
	int i;
	for(i=0; i<sizeof(fsAxisFuncStr)/sizeof(struct FsAxisFunctionString); i++)
	{
		if(strcmp(fsAxisFuncStr[i].str,str)==0)
		{
			return fsAxisFuncStr[i].fnc;
		}
	}
	return FSAXF_NULL;
}

FSBUTTONFUNCTION FsGetButtonFuncFromString(const char str[])
{
	int i;
	for(i=0; i<sizeof(fsButtonFuncStr)/sizeof(struct FsButtonFunctionString); i++)
	{
		if(strcmp(fsButtonFuncStr[i].str,str)==0)
		{
			return fsButtonFuncStr[i].fnc;
		}
	}
	return FSBTF_NULL;
}

int FsGetKeycodeFromString(const char str[])
{
	int i;
	for(i=0; i<sizeof(fsKeyString)/sizeof(struct FsKeyString); i++)
	{
		if(strcmp(fsKeyString[i].str,str)==0)
		{
			return fsKeyString[i].keyCode;
		}
	}
	return FSKEY_NULL;
}


FsControlAssignment::FsControlAssignment() :
	axsAssignAllocator(16),
	trgAssignAllocator(16),
	keyAssignAllocator(16),
	axsAssignList(axsAssignAllocator),
	trgAssignList(trgAssignAllocator),
	keyAssignList(keyAssignAllocator)
{
	processNumberKey=YSTRUE;
	checkKeyHolding=YSTRUE;
	ignoreThisKeyHolding=FSKEY_NULL;
	SetDefault(0);
	BuildMapping();
}

void FsControlAssignment::CleanUp(void)
{
	axsAssignList.CleanUp();
	trgAssignList.CleanUp();
	keyAssignList.CleanUp();

	axsAssignAllocator.CollectGarbage();
	trgAssignAllocator.CollectGarbage();
	keyAssignAllocator.CollectGarbage();

	processNumberKey=YSTRUE;
	checkKeyHolding=YSTRUE;
	ignoreThisKeyHolding=FSKEY_NULL;
}

void FsControlAssignment::CleanUpAxisAndButtonAssignment(void)
{
	axsAssignList.CleanUp();
	trgAssignList.CleanUp();
	axsAssignAllocator.CollectGarbage();
	trgAssignAllocator.CollectGarbage();
	checkKeyHolding=YSTRUE;
	ignoreThisKeyHolding=FSKEY_NULL;
}

void FsControlAssignment::SetDefault(int primaryJoyId)
{
	CleanUp();

	SetDefaultKeyAssign();

	AddAxisAssignment(primaryJoyId,0,FSAXF_AILERON,YSFALSE);
	AddAxisAssignment(primaryJoyId,1,FSAXF_ELEVATOR,YSFALSE);
	AddAxisAssignment(primaryJoyId,2,FSAXF_THROTTLE,YSFALSE);
	AddAxisAssignment(primaryJoyId,3,FSAXF_RUDDER,YSFALSE);

	AddAxisAssignment(FsMouseJoyId,0,FSAXF_TURRETH,YSFALSE);
	AddAxisAssignment(FsMouseJoyId,1,FSAXF_TURRETP,YSFALSE);

	AddTriggerAssignment(primaryJoyId,0,FSBTF_FIREWEAPON);
	AddTriggerAssignment(primaryJoyId,1,FSBTF_SELECTWEAPON);
	AddTriggerAssignment(primaryJoyId,2,FSBTF_RADAR);
	AddTriggerAssignment(primaryJoyId,3,FSBTF_DISPENSEFLARE);

	deadZoneElevator=0.03;
	deadZoneAileron=0.03;
	deadZoneRudder=0.03;
	usePovHatSwitch=YSTRUE;
}

void FsControlAssignment::SetDefaultGamePad(int primaryJoyId)
{
	CleanUpAxisAndButtonAssignment();

	AddAxisAssignment(primaryJoyId,0,FSAXF_AILERON,YSFALSE);
	AddAxisAssignment(primaryJoyId,1,FSAXF_ELEVATOR,YSFALSE);
	AddAxisAssignment(primaryJoyId,2,FSAXF_RUDDER,YSFALSE);
	AddAxisAssignment(primaryJoyId,3,FSAXF_THROTTLE_UPDOWN,YSTRUE);

	AddAxisAssignment(FsMouseJoyId,0,FSAXF_TURRETH,YSFALSE);
	AddAxisAssignment(FsMouseJoyId,1,FSAXF_TURRETP,YSFALSE);

	AddTriggerAssignment(primaryJoyId,0,FSBTF_FIREWEAPON);
	AddTriggerAssignment(primaryJoyId,1,FSBTF_SELECTWEAPON);
	AddTriggerAssignment(primaryJoyId,2,FSBTF_RADAR);
	AddTriggerAssignment(primaryJoyId,3,FSBTF_DISPENSEFLARE);

	AddTriggerAssignment(primaryJoyId,4,FSBTF_LANDINGGEAR);
	AddTriggerAssignment(primaryJoyId,5,FSBTF_FLAP);
	AddTriggerAssignment(primaryJoyId,6,FSBTF_SPOILERBRAKE);
	AddTriggerAssignment(primaryJoyId,7,FSBTF_AUTOTRIM);

	deadZoneElevator=0.03;
	deadZoneAileron=0.03;
	deadZoneRudder=0.03;
	usePovHatSwitch=YSTRUE;
}

void FsControlAssignment::SetDefaultMouseAsStick(void)
{
	// Note:
	//   When joystick is not connected and joystick axes are assigned
	//   to some functions, VerifyAndFixJoystickAxisAssignment will be
	//   called instead of this function.

	CleanUp();

	SetDefaultKeyAssign();

	AddAxisAssignment(FsMouseJoyId,0,FSAXF_AILERON,YSFALSE);
	AddAxisAssignment(FsMouseJoyId,1,FSAXF_ELEVATOR,YSFALSE);

	AddTriggerAssignment(FsMouseJoyId,0,FSBTF_FIREWEAPON);
	AddTriggerAssignment(FsMouseJoyId,1,FSBTF_SELECTWEAPON);
	AddTriggerAssignment(FsMouseJoyId,2,FSBTF_RADAR);

	AddKeyAssignment(FSKEY_LEFT, FSBTF_TURRETLEFT);
	AddKeyAssignment(FSKEY_RIGHT,FSBTF_TURRETRIGHT);
	AddKeyAssignment(FSKEY_UP,   FSBTF_TURRETUP);
	AddKeyAssignment(FSKEY_DOWN, FSBTF_TURRETDOWN);

	deadZoneElevator=0.0;
	deadZoneAileron=0.05;
	deadZoneRudder=0.05;
}

void FsControlAssignment::SetDefaultKeyboardAsStick(void)
{
	CleanUp();

	SetDefaultKeyAssign();

	AddKeyAssignment(FSKEY_UP   ,FSBTF_ELEVATORDOWN);
	AddKeyAssignment(FSKEY_DOWN ,FSBTF_ELEVATORUP);
	AddKeyAssignment(FSKEY_LEFT ,FSBTF_AILERONLEFT);
	AddKeyAssignment(FSKEY_RIGHT,FSBTF_AILERONRIGHT);

	deadZoneElevator=0.0;
	deadZoneAileron=0.03;
	deadZoneRudder=0.03;
}

void FsControlAssignment::SetDefaultKeyAssign(void)
{
#ifndef __APPLE__
	AddKeyAssignment(FSKEY_INS,     FSBTF_TRIMDOWN);
	AddKeyAssignment(FSKEY_DEL,     FSBTF_TRIMUP);
#else
	AddKeyAssignment(FSKEY_UP,      FSBTF_TRIMDOWN);
	AddKeyAssignment(FSKEY_DOWN,    FSBTF_TRIMUP);
#endif
	AddKeyAssignment(FSKEY_T,       FSBTF_AUTOTRIM);
	AddKeyAssignment(FSKEY_Q,       FSBTF_THROTTLEUP);
	AddKeyAssignment(FSKEY_A,       FSBTF_THROTTLEDOWN);
	AddKeyAssignment(FSKEY_TAB,     FSBTF_AFTERBURNER);
	AddKeyAssignment(FSKEY_PAGEUP,  FSBTF_NOZZLEUP);
	AddKeyAssignment(FSKEY_PAGEDOWN,FSBTF_NOZZLEDOWN);
	AddKeyAssignment(FSKEY_COMMA,   FSBTF_CYCLESENSITIVITY);
	AddKeyAssignment(FSKEY_G,       FSBTF_LANDINGGEAR);
	AddKeyAssignment(FSKEY_B,       FSBTF_SPOILERBRAKE);
	AddKeyAssignment(FSKEY_Z,       FSBTF_RUDDERLEFT);
	AddKeyAssignment(FSKEY_X,       FSBTF_RUDDERCENTER);
	AddKeyAssignment(FSKEY_C,       FSBTF_RUDDERRIGHT);
	AddKeyAssignment(FSKEY_SPACE,   FSBTF_FIREWEAPON);
	AddKeyAssignment(FSKEY_2,       FSBTF_SELECTWEAPON);
	AddKeyAssignment(FSKEY_P,       FSBTF_CYCLESMOKESELECTOR);
	AddKeyAssignment(FSKEY_3,       FSBTF_RADAR);
	AddKeyAssignment(FSKEY_4,       FSBTF_DISPENSEFLARE);
	AddKeyAssignment(FSKEY_I,       FSBTF_TOGGLELIGHT);
	AddKeyAssignment(FSKEY_V,       FSBTF_VELOCITYINDICATOR);
	AddKeyAssignment(FSKEY_BS,      FSBTF_OPENAUTOPILOTMENU);
	AddKeyAssignment(FSKEY_ENTER,   FSBTF_OPENRADIOCOMMMENU);
	AddKeyAssignment(FSKEY_F1,      FSBTF_COCKPITVIEW);
	AddKeyAssignment(FSKEY_F2,      FSBTF_OUTSIDEPLAYERVIEW);
	AddKeyAssignment(FSKEY_F3,      FSBTF_COMPUTERAIRPLANEVIEW);
	AddKeyAssignment(FSKEY_F4,      FSBTF_WEAPONVIEW);
	AddKeyAssignment(FSKEY_F5,      FSBTF_CHANGEAIRPLANE);
	AddKeyAssignment(FSKEY_F6,      FSBTF_ILSVIEW);
	AddKeyAssignment(FSKEY_F7,      FSBTF_OUTSIDEPLAYERVIEW2);
	AddKeyAssignment(FSKEY_F8,      FSBTF_CONTROLTOWERVIEW);
	AddKeyAssignment(FSKEY_F9,      FSBTF_SWITCHVIEWTARGET);
	AddKeyAssignment(FSKEY_U,       FSBTF_LOOKFORWARD);
	AddKeyAssignment(FSKEY_K,       FSBTF_LOOKRIGHT);
	AddKeyAssignment(FSKEY_H,       FSBTF_LOOKLEFT);
	AddKeyAssignment(FSKEY_M,       FSBTF_LOOKBACK);
	AddKeyAssignment(FSKEY_J,       FSBTF_LOOKUP);
	AddKeyAssignment(FSKEY_N,       FSBTF_LOOKDOWN);
	AddKeyAssignment(FSKEY_DOT,     FSBTF_REVERSETHRUST);
	AddKeyAssignment(FSKEY_W,       FSBTF_PROPFORWARD);
	AddKeyAssignment(FSKEY_S,       FSBTF_PROPBACKWARD);
	AddKeyAssignment(FSKEY_R,       FSBTF_FLAPUP);
	AddKeyAssignment(FSKEY_F,       FSBTF_FLAPDOWN);
	AddKeyAssignment(FSKEY_O,       FSBTF_OPENSUBWINDOWMENU);
	AddKeyAssignment(FSKEY_9,       FSBTF_CHANGEHUDCOLOR);
	AddKeyAssignment(FSKEY_PAUSEBREAK,FSBTF_PAUSE);
	AddKeyAssignment(FSKEY_F10,     FSBTF_GHOSTVIEW);
	AddKeyAssignment(FSKEY_L,       FSBTF_OPENVORMENU);
	AddKeyAssignment(FSKEY_7,       FSBTF_ROTATEVORLEFT);
	AddKeyAssignment(FSKEY_8,       FSBTF_ROTATEVORRIGHT);
	AddKeyAssignment(FSKEY_1,       FSBTF_BOMBBAYDOOR);
	AddKeyAssignment(FSKEY_CTRL,    FSBTF_INFLTCONFIG);
	AddKeyAssignment(FSKEY_SEMICOLON, FSBTF_TOGGLEALLDOOR);
#ifndef __APPLE__
	AddKeyAssignment(FSKEY_F12,     FSBTF_INFLTMESSAGE);
#else
	AddKeyAssignment(FSKEY_0,       FSBTF_INFLTMESSAGE);
#endif
	AddKeyAssignment(FSKEY_WHEELUP, FSBTF_VIEWZOOM);
	AddKeyAssignment(FSKEY_WHEELDOWN,FSBTF_VIEWMOOZ);
	AddKeyAssignment(FSKEY_HOME,    FSBTF_SUPPLYDIALOG);
}

void FsControlAssignment::BuildMapping(void)
{
	int i,j;
	for(i=0; i<FsMaxNumJoystick; i++)
	{
		for(j=0; j<FsMaxNumJoyAxis; j++)
		{
			axisToFuncMap[i][j]=NULL;
		}
		for(j=0; j<FsMaxNumJoyTrig; j++)
		{
			trgToFuncMap[i][j]=NULL;
		}
	}

	for(i=0; i<FSAXF_NUMAXISFUNCTION; i++)
	{
		funcToAxisMap[i]=NULL;
	}

	for(i=0; i<FSBTF_NUMBUTTONFUNCTION; i++)
	{
		funcToTrgMap[i]=NULL;
		funcToKeyMap[i]=NULL;
	}

	for(i=0; i<FSKEY_NUM_KEYCODE; i++)
	{
		keyToFuncMap[i]=NULL;
	}


	YsListItem <FsAxisAssignment> *axsPtr;
	axsPtr=NULL;
	while((axsPtr=axsAssignList.FindNext(axsPtr))!=NULL)
	{
		if(0<=axsPtr->dat.joyId && axsPtr->dat.joyId<FsMaxNumJoystick &&
		   0<=axsPtr->dat.joyAxs && axsPtr->dat.joyAxs<FsMaxNumJoyAxis)
		{
			axisToFuncMap[axsPtr->dat.joyId][axsPtr->dat.joyAxs]=&axsPtr->dat;
		}
		funcToAxisMap[(int)axsPtr->dat.fnc]=&axsPtr->dat;
	}

	YsListItem <FsTriggerAssignment> *trgPtr;
	trgPtr=NULL;
	while((trgPtr=trgAssignList.FindNext(trgPtr))!=NULL)
	{
		if(0<=trgPtr->dat.joyId && trgPtr->dat.joyId<FsMaxNumJoystick &&
		   0<=trgPtr->dat.joyTrg && trgPtr->dat.joyTrg<FsMaxNumJoyTrig)
		{
			trgToFuncMap[trgPtr->dat.joyId][trgPtr->dat.joyTrg]=&trgPtr->dat;
		}
		funcToTrgMap[(int)trgPtr->dat.fnc]=&trgPtr->dat;
	}

	YsListItem <FsKeyAssignment> *keyPtr;
	keyPtr=NULL;
	while((keyPtr=keyAssignList.FindNext(keyPtr))!=NULL)
	{
		keyToFuncMap[keyPtr->dat.keyCode]=&keyPtr->dat;
		funcToKeyMap[(int)keyPtr->dat.fnc]=&keyPtr->dat;
	}
}

YSRESULT FsControlAssignment::AddAxisAssignment(int joyId,int joyAxs,FSAXISFUNCTION fnc,YSBOOL reverse)
{
	if(0<=joyId && joyId<FsMaxNumJoystick && 0<=joyAxs && joyAxs<FsMaxNumJoyAxis)
	{
		YsListItem <FsAxisAssignment> *axsPtr;

		DeleteAxis(joyId,joyAxs);
		DeleteAxisFunction(fnc);

		axsPtr=axsAssignList.Create();
		axsPtr->dat.joyId=joyId;
		axsPtr->dat.joyAxs=joyAxs;
		axsPtr->dat.fnc=fnc;
		axsPtr->dat.reverse=reverse;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsControlAssignment::DeleteAxisFunction(FSAXISFUNCTION fnc)
{
	YsListItem <FsAxisAssignment> *axsPtr,*nextAxsPtr;
	axsPtr=NULL;
	nextAxsPtr=axsAssignList.FindNext(axsPtr);
	while((axsPtr=nextAxsPtr)!=NULL)
	{
		nextAxsPtr=axsAssignList.FindNext(axsPtr);
		if(axsPtr->dat.fnc==fnc)
		{
			axisToFuncMap[axsPtr->dat.joyId][axsPtr->dat.joyAxs]=NULL;
			funcToAxisMap[(int)fnc]=NULL;
			axsAssignList.Delete(axsPtr);
		}
	}
	return YSOK;
}

YSRESULT FsControlAssignment::DeleteAxis(int joyId,int joyAxs)
{
	YsListItem <FsAxisAssignment> *axsPtr,*nextAxsPtr;
	axsPtr=NULL;
	nextAxsPtr=axsAssignList.FindNext(axsPtr);
	while((axsPtr=nextAxsPtr)!=NULL)
	{
		nextAxsPtr=axsAssignList.FindNext(axsPtr);
		if(axsPtr->dat.joyId==joyId && axsPtr->dat.joyAxs==joyAxs)
		{
			axisToFuncMap[axsPtr->dat.joyId][axsPtr->dat.joyAxs]=NULL;
			funcToAxisMap[(int)axsPtr->dat.fnc]=NULL;
			axsAssignList.Delete(axsPtr);
		}
	}
	return YSOK;
}

YSRESULT FsControlAssignment::AddTriggerAssignment(int joyId,int joyTrg,FSBUTTONFUNCTION fnc)
{
	if(0<=joyId && joyId<FsMaxNumJoystick && 0<=joyTrg && joyTrg<FsMaxNumJoyTrig)
	{
		YsListItem <FsTriggerAssignment> *trgPtr;

		DeleteTriggerFunction(fnc);
		DeleteTrigger(joyId,joyTrg);

		trgPtr=trgAssignList.Create();
		trgPtr->dat.joyId=joyId;
		trgPtr->dat.joyTrg=joyTrg;
		trgPtr->dat.fnc=fnc;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsControlAssignment::DeleteTriggerFunction(FSBUTTONFUNCTION fnc)
{
	YsListItem <FsTriggerAssignment> *trgPtr,*nextTrgPtr;
	trgPtr=NULL;
	nextTrgPtr=trgAssignList.FindNext(trgPtr);
	while((trgPtr=nextTrgPtr)!=NULL)
	{
		nextTrgPtr=trgAssignList.FindNext(trgPtr);
		if(trgPtr->dat.fnc==fnc)
		{
			trgToFuncMap[trgPtr->dat.joyId][trgPtr->dat.joyTrg]=NULL;
			funcToTrgMap[(int)fnc]=NULL;
			trgAssignList.Delete(trgPtr);
		}
	}
	return YSOK;
}

YSRESULT FsControlAssignment::DeleteTrigger(int joyId,int joyTrg)
{
	YsListItem <FsTriggerAssignment> *trgPtr,*nextTrgPtr;
	trgPtr=NULL;
	nextTrgPtr=trgAssignList.FindNext(trgPtr);
	while((trgPtr=nextTrgPtr)!=NULL)
	{
		nextTrgPtr=trgAssignList.FindNext(trgPtr);
		if(trgPtr->dat.joyId==joyId && trgPtr->dat.joyTrg==joyTrg)
		{
			trgToFuncMap[trgPtr->dat.joyId][trgPtr->dat.joyTrg]=NULL;
			funcToTrgMap[(int)trgPtr->dat.fnc]=NULL;
			trgAssignList.Delete(trgPtr);
		}
	}
	return YSOK;
}

YSRESULT FsControlAssignment::AddKeyAssignment(int keyCode,FSBUTTONFUNCTION fnc)
{
	if(0<=keyCode && keyCode<FSKEY_NUM_KEYCODE)
	{
		YsListItem <FsKeyAssignment> *keyPtr;

		DeleteKeyFunction(fnc);
		DeleteKey(keyCode);

		keyPtr=keyAssignList.Create();
		keyPtr->dat.keyCode=keyCode;
		keyPtr->dat.fnc=fnc;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsControlAssignment::DeleteKeyFunction(FSBUTTONFUNCTION fnc)
{
	YsListItem <FsKeyAssignment> *keyPtr,*nextKeyPtr;
	keyPtr=NULL;
	nextKeyPtr=keyAssignList.FindNext(keyPtr);
	while((keyPtr=nextKeyPtr)!=NULL)
	{
		nextKeyPtr=keyAssignList.FindNext(keyPtr);
		if(keyPtr->dat.fnc==fnc)
		{
			keyToFuncMap[keyPtr->dat.keyCode]=NULL;
			funcToKeyMap[(int)fnc]=NULL;
			keyAssignList.Delete(keyPtr);
		}
	}
	return YSOK;
}

YSRESULT FsControlAssignment::DeleteKey(int keyCode)
{
	YsListItem <FsKeyAssignment> *keyPtr,*nextKeyPtr;
	keyPtr=NULL;
	nextKeyPtr=keyAssignList.FindNext(keyPtr);
	while((keyPtr=nextKeyPtr)!=NULL)
	{
		nextKeyPtr=keyAssignList.FindNext(keyPtr);
		if(keyPtr->dat.keyCode==keyCode)
		{
			keyToFuncMap[keyCode]=NULL;
			funcToKeyMap[(int)keyPtr->dat.fnc]=NULL;
			keyAssignList.Delete(keyPtr);
		}
	}
	return YSOK;
}

void FsControlAssignment::SetIgnoreThisKeyHolding(int rawKey)
{
	ignoreThisKeyHolding=rawKey;
}

void FsControlAssignment::CheckIgnoredKeyRelease(void)
{
	if(FSKEY_NULL!=ignoreThisKeyHolding && YSTRUE!=FsGetKeyState(ignoreThisKeyHolding))
	{
		ignoreThisKeyHolding=FSKEY_NULL;
	}
}

FSBUTTONFUNCTION FsControlAssignment::TranslateKeyStroke(int keyCode) const
{
	if(processNumberKey!=YSTRUE && FSKEY_0<=keyCode && keyCode<=FSKEY_9)
	{
		return FSBTF_NULL;
	}

	if(0<keyCode && keyCode<FSKEY_NUM_KEYCODE && keyToFuncMap[keyCode]!=NULL)
	{
		return keyToFuncMap[keyCode]->fnc;
	}
	return FSBTF_NULL;
}

FSAXISFUNCTION FsControlAssignment::TranslateAxis(int joyId,int joyAxs) const
{
	if(0<=joyId && joyId<FsMaxNumJoystick && 0<=joyAxs && joyAxs<FsMaxNumJoyAxis && NULL!=axisToFuncMap[joyId][joyAxs])
	{
		return axisToFuncMap[joyId][joyAxs]->fnc;
	}
	return FSAXF_NULL;
}

FSBUTTONFUNCTION FsControlAssignment::TranslateTrigger(int joyId,int joyTrg) const
{
	if(0<=joyId && joyId<FsMaxNumJoystick && 0<=joyTrg && joyTrg<FsMaxNumJoyTrig && trgToFuncMap[joyId][joyTrg]!=NULL)
	{
		return trgToFuncMap[joyId][joyTrg]->fnc;
	}
	return FSBTF_NULL;
}

int FsControlAssignment::FindKeyByFunction(FSBUTTONFUNCTION fnc) const
{
	if(funcToKeyMap[(int)fnc]!=NULL)
	{
		return funcToKeyMap[(int)fnc]->keyCode;
	}
	return FSKEY_NULL;
}

YSRESULT FsControlAssignment::FindTriggerByFunction(int &joyId,int &joyTrg,FSBUTTONFUNCTION fnc) const
{
	if(funcToTrgMap[(int)fnc]!=NULL)
	{
		joyId=funcToTrgMap[(int)fnc]->joyId;
		joyTrg=funcToTrgMap[(int)fnc]->joyTrg;
		return YSOK;
	}
	joyId=-1;
	joyTrg=-1;
	return YSERR;
}

YSRESULT FsControlAssignment::FindAxisByFunction(int &joyId,int &joyAxs,YSBOOL &reverse,FSAXISFUNCTION fnc) const
{
	if(funcToAxisMap[(int)fnc]!=NULL)
	{
		joyId=funcToAxisMap[(int)fnc]->joyId;
		joyAxs=funcToAxisMap[(int)fnc]->joyAxs;
		reverse=funcToAxisMap[(int)fnc]->reverse;
		return YSOK;
	}
	joyId=-1;
	joyAxs=-1;
	return YSERR;
}

YSBOOL FsControlAssignment::IsButtonPressed(FSBUTTONFUNCTION fnc,class FsJoystick joy[FsMaxNumJoystick]) const
{
	int key,joyId,joyTrg;

	key=FindKeyByFunction(fnc);
	if(ignoreThisKeyHolding!=key)
	{
		if(processNumberKey==YSTRUE || key<FSKEY_0 || FSKEY_9<key)
		{
			if(checkKeyHolding==YSTRUE)   // <- Added on 2005/03/31
			{
				if(FsGetKeyState(key)==YSTRUE)
				{
					return YSTRUE;
				}
			}
		}
	}

	if(FindTriggerByFunction(joyId,joyTrg,fnc)==YSOK && joy[joyId].trg[joyTrg]==YSTRUE)
	{
		return YSTRUE;
	}

	return YSFALSE;
}

YSRESULT FsControlAssignment::Load(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		int ac;
		char *av[16];
		char str[256],err[256];
		int version;

		version=0;

		CleanUp();
		while(fgets(str,255,fp)!=NULL)
		{
			strcpy(err,str);
			if(YsArguments(&ac,av,16,str)==YSOK && ac>0)
			{
				YSRESULT err;

				err=YSOK;
				if(strcmp(av[0],"END")==0)
				{
					break;
				}
				else if(strcmp(av[0],"REM")==0)
				{
				}
				else if(strcmp(av[0],"VER")==0)
				{
					version=atoi(av[1]);
				}
				else if(strcmp(av[0],"AXS")==0 && ac>=4)
				{
					int joyId,joyAxs;
					FSAXISFUNCTION fnc;
					YSBOOL reverse;
					if(av[1][0]=='M')
					{
						joyId=FsMouseJoyId;
					}
					else
					{
						joyId=atoi(av[1]);
					}

					joyAxs=atoi(av[2]);
					fnc=FsGetAxisFuncFromString(av[3]);

					if(ac>=5 && strcmp(av[4],"REV")==0)
					{
						reverse=YSTRUE;
					}
					else
					{
						reverse=YSFALSE;
					}

					if(fnc!=FSAXF_NULL)
					{
						AddAxisAssignment(joyId,joyAxs,fnc,reverse);
					}
				}
				else if(strcmp(av[0],"TRG")==0 && ac>=4)
				{
					int joyId,joyTrg;
					FSBUTTONFUNCTION fnc;
					if(av[1][0]=='M')
					{
						joyId=FsMouseJoyId;
					}
					else
					{
						joyId=atoi(av[1]);
					}
					joyTrg=atoi(av[2]);
					fnc=FsGetButtonFuncFromString(av[3]);
					AddTriggerAssignment(joyId,joyTrg,fnc);
				}
				else if(strcmp(av[0],"KEY")==0)
				{
					int keyCode;
					FSBUTTONFUNCTION fnc;
					keyCode=FsGetKeycodeFromString(av[1]);
					fnc=FsGetButtonFuncFromString(av[2]);
					if(keyCode!=FSKEY_NULL && fnc!=FSBTF_NULL)
					{
						AddKeyAssignment(keyCode,fnc);
					}
				}
				else if(strcmp(av[0],"DZELV2")==0)
				{
					deadZoneElevator=atof(av[1]);
				}
				else if(strcmp(av[0],"DZAIL2")==0)
				{
					deadZoneAileron=atof(av[1]);
				}
				else if(strcmp(av[0],"DZRUD2")==0)
				{
					deadZoneRudder=atof(av[1]);
				}
				else if(strcmp(av[0],"DZELV")==0)
				{
					YSBOOL useDeadZone;
					FsGetBool(useDeadZone,av[1]);
					deadZoneElevator=(YSTRUE==useDeadZone ? 0.03 : 0.0);
				}
				else if(strcmp(av[0],"DZAIL")==0)
				{
					YSBOOL useDeadZone;
					FsGetBool(useDeadZone,av[1]);
					deadZoneAileron=(YSTRUE==useDeadZone ? 0.03 : 0.0);
				}
				else if(strcmp(av[0],"DZRUD")==0)
				{
					YSBOOL useDeadZone;
					FsGetBool(useDeadZone,av[1]);
					deadZoneRudder=(YSTRUE==useDeadZone ? 0.03 : 0.0);
				}
				else if(strcmp(av[0],"HATSW")==0)
				{
					FsGetBool(usePovHatSwitch,av[1]);
				}
				else
				{
					err=YSERR;
				}

				if(err!=YSOK)
				{
					fsStderr.Printf("Error: %s %d\n",av[0],err);
				}
			}
		}
		fclose(fp);

		if(version<=20070907)
		{
			DeleteKeyFunction(FSBTF_ILS);
			DeleteKeyFunction(FSBTF_OPENADFMENU);
		}

		BuildMapping();
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsControlAssignment::Save(const wchar_t fn[])
{
	int i,j;
	FILE *fp=YsFileIO::Fopen(fn,"w");
	if(fp!=NULL)
	{
		BuildMapping();

		fprintf(fp,"VER %d\n",YSFLIGHT_VERSION);

		for(i=0; i<FsMaxNumJoystick; i++)
		{
			for(j=0; j<FsMaxNumJoyAxis; j++)
			{
				FSAXISFUNCTION fnc;
				if(axisToFuncMap[i][j]!=NULL)
				{
					const char *str,*revStr;
					fnc=axisToFuncMap[i][j]->fnc;
					str=FsGetAxisFuncString(fnc);

					if(axisToFuncMap[i][j]->reverse==YSTRUE)
					{
						revStr="REV";
					}
					else
					{
						revStr="";
					}

					if(str!=NULL)
					{
						if(i==FsMouseJoyId)
						{
							fprintf(fp,"AXS M %d %s %s\n",j,str,revStr);
						}
						else
						{
							fprintf(fp,"AXS %d %d %s %s\n",i,j,str,revStr);
						}
					}
				}
			}
		}

		for(i=0; i<FsMaxNumJoystick; i++)
		{
			for(j=0; j<FsMaxNumJoyTrig; j++)
			{
				FSBUTTONFUNCTION fnc;
				if(trgToFuncMap[i][j]!=NULL)
				{
					const char *str;
					fnc=trgToFuncMap[i][j]->fnc;
					str=FsGetButtonFuncString(fnc);
					if(str!=NULL)
					{
						if(i==FsMouseJoyId)
						{
							fprintf(fp,"TRG M %d %s\n",j,str);
						}
						else
						{
							fprintf(fp,"TRG %d %d %s\n",i,j,str);
						}
					}
				}
			}
		}

		for(i=0; i<FSKEY_NUM_KEYCODE; i++)
		{
			FSBUTTONFUNCTION fnc;
			if(keyToFuncMap[i]!=NULL)
			{
				const char *str,*label;
				fnc=keyToFuncMap[i]->fnc;
				str=FsGetButtonFuncString(fnc);
				label=FsGetKeyString(i);
				if(str!=NULL && label!=NULL)
				{
					fprintf(fp,"KEY %s %s\n",label,str);
				}
			}
		}

		fprintf(fp,"DZELV2 %.3lf\n",deadZoneElevator);
		fprintf(fp,"DZAIL2 %.3lf\n",deadZoneAileron);
		fprintf(fp,"DZRUD2 %.3lf\n",deadZoneRudder);
		fprintf(fp,"HATSW %s\n",FsTrueFalseString(usePovHatSwitch));

		fprintf(fp,"END\n");

		fprintf(fp,"REM Joystick Axis Assignment\n");
		fprintf(fp,"REM   AXS joyId joyAxs AxisFunction\n");
		fprintf(fp,"REM       or\n");
		fprintf(fp,"REM   AXS joyId joyAxs AxisFunction REV\n");
		fprintf(fp,"REM \n");
		fprintf(fp,"REM   joyId='M' means mouse\n");
		fprintf(fp,"REM   joyId must be 0<=joyId<=%d\n",FsMaxNumJoystick-1);
		fprintf(fp,"REM   joyAxs must be 0<=joyAxs<=%d\n",FsMaxNumJoyAxis-1);
		fprintf(fp,"REM   joyId=%d also means mouse, but it may change in the future versions.\n",FsMouseJoyId);
		fprintf(fp,"REM   \"REV\" at the end of line means reverse.\n");
		fprintf(fp,"REM \n");
		fprintf(fp,"REM Joystick Trigger Assignment\n");
		fprintf(fp,"REM   TRG joyId joyTrg ButtonFunction\n");
		fprintf(fp,"REM \n");
		fprintf(fp,"REM   joyTrg must be 0<=joyTrg<=%d\n",FsMaxNumJoyTrig-1);
		fprintf(fp,"REM \n");
		fprintf(fp,"REM Keyboard Assignment\n");
		fprintf(fp,"REM   KEY keyCode ButtonFunction\n");
		fprintf(fp,"REM \n");

		fprintf(fp,"REM List of AxisFunctions\n");
		for(i=0; i<(int)FSAXF_NUMAXISFUNCTION; i++)
		{
			const char *label,*str;
			label=FsGetAxisFuncLabel((FSAXISFUNCTION)i);
			str=FsGetAxisFuncString((FSAXISFUNCTION)i);
			if(label!=NULL && str!=NULL)
			{
				fprintf(fp,"REM   %-16s %s\n",str,label);
			}
		}
		fprintf(fp,"REM Use dead zone (for overly sensitive joysticks)\n");
		fprintf(fp,"REM   DZELV TRUE/FALSE   <- Elevator axis\n");
		fprintf(fp,"REM   DZAIL TRUE/FALSE   <- Aileron axis\n");
		fprintf(fp,"REM   DZRUD TRUE/FALSE   <- Rudder axis\n");
		fprintf(fp,"REM\n");

		fprintf(fp,"REM List of ButtonFunctions \n");
		for(i=0; i<(int)FSBTF_NUMBUTTONFUNCTION; i++)
		{
			const char *label,*str;
			label=FsGetButtonFuncLabel((FSBUTTONFUNCTION)i);
			str=FsGetButtonFuncString((FSBUTTONFUNCTION)i);
			if(label!=NULL && str!=NULL)
			{
				fprintf(fp,"REM   %-16s %s\n",str,label);
			}
		}
		fprintf(fp,"REM\n");

		fprintf(fp,"REM List of KeyCodes\n");
		for(i=0; i<(int)FSKEY_NUM_KEYCODE; i++)
		{
			const char *label,*str;
			label=FsGetKeyLabel(i);
			str=FsGetKeyString(i);
			if(label!=NULL && str!=NULL)
			{
				fprintf(fp,"REM   %-16s %s\n",str,label);
			}
		}
		fprintf(fp,"REM\n");

		fclose(fp);
		return YSOK;
	}

	return YSERR;
}


YSRESULT FsControlAssignment::MergeDefaultControl(void)
{
	FsControlAssignment defCtl,userCtl;
	YSBOOL update;

	defCtl.CleanUp();
	defCtl.SetDefault(0);
	defCtl.BuildMapping();

	userCtl.CleanUp();
	if(userCtl.Load(FsGetControlAssignFile())==YSOK)
	{
		int i;
		update=YSFALSE;
		for(i=0; i<FSBTF_NUMBUTTONFUNCTION; i++)
		{
			FSBUTTONFUNCTION fnc;

			fnc=(FSBUTTONFUNCTION)i;

			if((defCtl.funcToTrgMap[i]!=NULL || defCtl.funcToKeyMap[i]!=NULL) && // A key or trigger is assigned in default assignment and
			   (userCtl.funcToTrgMap[i]==NULL && userCtl.funcToKeyMap[i]==NULL)) // No key nor trigger is assigned in user assignment.
			{
				if(defCtl.funcToTrgMap[i]!=NULL)  // If a trigger is assigned in default assginment
				{
					int joyId,joyTrg;
					joyId=defCtl.funcToTrgMap[i]->joyId;
					joyTrg=defCtl.funcToTrgMap[i]->joyTrg;
					if(userCtl.trgToFuncMap[joyId][joyTrg]==NULL)
					{
						printf("No trigger was assigned to %s\n",FsGetButtonFuncLabel((FSBUTTONFUNCTION)i));
						printf("JS:%d TRG:%d is now assigned.\n",joyId,joyTrg);

						userCtl.AddTriggerAssignment(joyId,joyTrg,fnc);
						update=YSTRUE;
					}
				}
				else if(defCtl.funcToKeyMap[i]!=NULL)
				{
					int keyCode;
					keyCode=defCtl.funcToKeyMap[i]->keyCode;
					if(userCtl.keyToFuncMap[keyCode]==NULL)
					{
						printf("No key was assigned to %s\n",FsGetButtonFuncLabel((FSBUTTONFUNCTION)i));
						printf("%s-key is now assigned.\n",FsGetKeyLabel(keyCode));

						userCtl.AddKeyAssignment(keyCode,fnc);
						update=YSTRUE;
					}
				}
			}
		}

		for(i=0; i<FSAXF_NUMAXISFUNCTION; i++)
		{
			FSAXISFUNCTION fnc;

			fnc=(FSAXISFUNCTION)i;

			if(defCtl.funcToAxisMap[i]!=NULL && userCtl.funcToAxisMap[i]==NULL)
			{
				int joyId,joyAxs;
				joyId=defCtl.funcToAxisMap[i]->joyId;
				joyAxs=defCtl.funcToAxisMap[i]->joyAxs;
				if(userCtl.axisToFuncMap[joyId][joyAxs]==NULL)
				{
					printf("No axis was assigned to %s\n",FsGetAxisFuncLabel((FSAXISFUNCTION)i));
					printf("JS:%d AXS:%d is now assigned.\n",joyId,joyAxs);

					userCtl.AddAxisAssignment(joyId,joyAxs,fnc,defCtl.funcToAxisMap[i]->reverse);
					update=YSTRUE;
				}
			}
		}

		if(update==YSTRUE)
		{
			userCtl.BuildMapping();
			return userCtl.Save(FsGetControlAssignFile());
		}
	}

	return YSERR;
}

FsCenterJoystick::FsCenterJoystick()
{
	pJoy=new FsJoystick[FsMaxNumJoystick];
	joy=new FsJoystick[FsMaxNumJoystick];
}

FsCenterJoystick::~FsCenterJoystick()
{
	if(NULL!=pJoy)
	{
		delete [] pJoy;
		pJoy=NULL;
	}
	if(NULL!=joy)
	{
		delete [] joy;
		joy=NULL;
	}
}

void FsCenterJoystick::Initialize(FsFlightControl *ctl,const FsControlAssignment *ctlAssign,int nextActionCode)
{
	this->ctl=ctl;
	this->ctlAssign=ctlAssign;

	state=INITIAL;
	this->nextActionCode=nextActionCode;
}

void FsCenterJoystick::RunOneStep(void)
{
	YSBOOL lb,mb,rb;
	int mx,my;

	if(INITIAL==state)
	{
		for(int i=0; i<FsMaxNumJoystick; i++)
		{
			FsPollJoystick(joy[i],i);
		}
		ctl->ReadControl(*ctlAssign,joy);
		state=WAITING_FOR_BUTTON;
	}
	else if(WAITING_FOR_BUTTON==state)
	{
		int key;

		while((key=FsInkey())!=FSKEY_NULL)
		{
			FSBUTTONFUNCTION fnc;
			fnc=ctlAssign->TranslateKeyStroke(key);
			ctl->ProcessButtonFunction(0.0,NULL,fnc);
			if(key==FSKEY_SPACE || key==FSKEY_ENTER) // or trigger is pulled
			{
				waitStart=time(NULL);
				state=WAITING_FOR_RELEASE;
				return;
			}
			if(key==FSKEY_ESC)
			{
				res=YSERR;
				state=OVER;
				return;
			}
		}

		for(int i=0; i<FsMaxNumJoystick; i++)
		{
			pJoy[i]=joy[i];
			FsPollJoystick(joy[i],i);
			for(int j=0; j<FsMaxNumJoyTrig; j++)
			{
				FSBUTTONFUNCTION fnc;
				fnc=ctlAssign->TranslateTrigger(i,j);
				if(fnc!=FSBTF_NULL && joy[i].trg[j]==YSTRUE)
				{
					waitStart=time(NULL);
					state=WAITING_FOR_RELEASE;
					return;
				}
			}
		}

		FsGetMouseEvent(lb,mb,rb,mx,my);
		if(lb==YSTRUE || mb==YSTRUE || rb==YSTRUE)
		{
			waitStart=time(NULL);
			state=WAITING_FOR_RELEASE;
			return;
		}

		ctl->ReadControl(*ctlAssign,pJoy,joy);
	}
	else if(WAITING_FOR_RELEASE==state)
	{
		int x,y;
		const time_t waitTime=2;

		x=32;
		y=48;

		waiting=time(NULL)-waitStart;

		while(FsInkey()!=FSKEY_NULL)
		{
		}

		int c;
		c=0;
		if(FsGetKeyState(FSKEY_SPACE)!=YSFALSE)
		{
			c++;
		}
		for(int i=0; i<FsMaxNumJoystick; i++)
		{
			FsPollJoystick(joy[i],i);
			for(int j=0; j<FsMaxNumJoyTrig; j++)
			{
				auto fnc=ctlAssign->TranslateTrigger(i,j);
				if(fnc!=FSBTF_NULL && joy[i].trg[j]==YSTRUE)
				{
					c++;
				}
			}
		}

		FsGetMouseEvent(lb,mb,rb,mx,my);
		if(lb==YSTRUE || mb==YSTRUE || rb==YSTRUE)
		{
			c++;
		}

		if(c==0 || 5<waiting)
		{
			state=OVER;
			return;
		}
	}
	else if(OVER==state)
	{
	}
}

void FsCenterJoystick::Draw(void) const
{
	if(INITIAL==state)
	{
	}
	else if(WAITING_FOR_BUTTON==state)
	{
		FsClearScreenAndZBuffer(YsBlue());

		FsProjection prj;
		FsSimulation::GetStandardProjection(prj);
		prj.nearz=0.1;
		prj.farz=50.0;
		FsSetSceneProjection(prj);


		YsVec3 pos(0.0,0.0,-1.0);
		YsAtt3 att(0.0,-YsPi/4.1,0.0);
		att.Mul(pos,pos);
		pos.AddY(0.2);
		FsSetCameraPosition(pos,att,YSTRUE);
		FsSetDirectionalLight(YsVec3(0.0,0.2,-1.0),YsYVec(),FSDAYLIGHT);



		YsVec3 joyPos,thrPos,rudPos;
		joyPos.Set( 0.4,0.1,0.2);
		thrPos.Set(-0.4,0.1,0.2);
		rudPos.Set( 0.0,-0.4,-0.05);

		att.MulInverse(joyPos,joyPos-pos);
		att.MulInverse(thrPos,thrPos-pos);
		att.MulInverse(rudPos,rudPos-pos);

		YsVec3 ev=YsZVec(),uv=YsYVec();
		att.MulInverse(ev,ev);
		att.MulInverse(uv,uv);

		YsAtt3 joyAtt;
		joyAtt.SetTwoVector(ev,uv);

		ctl->DrawJoystick(joyPos,joyAtt);
		ctl->DrawThrottle(thrPos,joyAtt);
		ctl->DrawRudder(rudPos,joyAtt);



		FsSet2DDrawing();
		FsDrawString(48,48,"CENTER JOYSTICK. PRESS SPACE KEY or TRIGGER TO GO!",YsWhite());

		FsFlushScene(); // BiFlushBuffer();
		FsSwapBuffers();
	}
	else if(WAITING_FOR_RELEASE==state)
	{
		int x,y;
		const time_t waitTime=2;

		x=32;
		y=48;

		auto waiting=time(NULL)-waitStart;
		if(waiting>=waitTime)
		{
			FsClearScreenAndZBuffer(YsBlue());
			FsSet2DDrawing();

			if(FsGetKeyState(FSKEY_SPACE)!=YSFALSE)
			{
				if(waiting>=waitTime)
				{
					FsDrawString(x,y,"Please Release Space Key",YsWhite());
					y+=24;
				}
			}
			for(int i=0; i<FsMaxNumJoystick; i++)
			{
				FsPollJoystick(joy[i],i);
				for(int j=0; j<FsMaxNumJoyTrig; j++)
				{
					FSBUTTONFUNCTION fnc;
					fnc=ctlAssign->TranslateTrigger(i,j);
					if(fnc!=FSBTF_NULL && joy[i].trg[j]==YSTRUE)
					{
						if(waiting>=waitTime)
						{
							char str[256];
							sprintf(str,"Please Release Joystick %d Button %d",i,j);
							FsDrawString(x,y,str,YsWhite());
							y+=24;
						}
					}
				}
			}

			int mx,my;
			YSBOOL lb,mb,rb;
			FsMouse(lb,mb,rb,mx,my);
			if(lb==YSTRUE || mb==YSTRUE || rb==YSTRUE)
			{
				if(waiting>=waitTime)
				{
					FsDrawString(x,y,"Please Release Mouse Button",YsWhite());
					y+=24;
				}
			}

			FsFlushScene(); // BiFlushBuffer();
			FsSwapBuffers();
		}
	}
	else if(OVER==state)
	{
	}
}

