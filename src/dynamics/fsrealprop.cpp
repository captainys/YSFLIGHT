// cl propsim.cpp ..\..\src\fssimplewindow\src\windows\fswin32wrapper.cpp ..\..\src\fssimplewindow\src\windows\fswin32keymap.cpp ..\..\src\ysbitmapfont\src\ysglfontdata.c ..\..\src\ysbitmapfont\src\ysglfontbitmaputil.c ..\..\src\ysbitmapfont\src\ysglusefontbitmap.c -I ..\..\src\fssimplewindow\src ysclass.lib -I ..\..\src\ysbitmapfont\src


#include <stdio.h>
#include <ysclass.h>

#include <fsdef.h>

#include "fsrealprop.h"
#include "fsairproperty.h"
#include "fsutil.h"

FsPropellerEngine::Blade::Blade()
{
	Initialize();
}

void FsPropellerEngine::Blade::Initialize(void)
{
	area=0.0;
	kCl=0.0;
	ClZero=0.0;
	kCd=0.0;
	CdMin=0.0;
	minCdAOA=0.0;

	pitchChangeSpeed=0.0;
	minPitch=0.0;
	maxPitch=0.0;
	kGoverner=0.0;

	gravityCenter=0.0;
	liftCenter=0.0;

	weight=0.0;

	clockwise=YSTRUE;  // Direction of rotation must be the property of the blade.  Imagine co-axial counter-rotating props.



	state.angle=0.0;
	state.pitch=0.0;
	torque=0.0;
	aoa=0.0;
}

YsVec3 FsPropellerEngine::Blade::CalculateForce(const double omega,const double rho,const YsVec3 &relVelInPropCoord)
{
	YsVec3 propVel;

	const double propSpeed=liftCenter*omega;
	const double directionOfRotation=(YSTRUE==clockwise ? -1.0 : 1.0);

	propVel.Set(0.0,propSpeed*directionOfRotation,0.0);
	propVel.RotateXY(state.angle);

	propVel+=relVelInPropCoord;

	YsVec3 relVelBlade;
	relVelBlade=propVel;
	relVelBlade.RotateXY(-state.angle);

	aoa=state.pitch-atan2(relVelBlade.z(),-relVelBlade.y());

	const double Cl=kCl*aoa+ClZero;
	const double Cd=kCd*YsSqr(aoa-minCdAOA)+CdMin;

	const double vv=relVelBlade.GetSquareLength();
	const double L=0.5*Cl*rho*vv*area;
	const double D=0.5*Cd*rho*vv*area;

	// printf("%lf %lf\n",L,D);

	drag=-D*YsUnitVector(propVel);

	lift=YsUnitVector(relVelBlade);
	lift.RotateZY(YsPi/2.0);
	lift.RotateXY(state.angle);
	lift*=L;

	// printf("Prop Position=%6.2lfdeg  AOA=%6.2lf  %s\n",YsRadToDeg(state.angle),YsRadToDeg(aoa),lift.Txt());

	return lift+drag;
}

YsVec3 FsPropellerEngine::Blade::GetForceCenterInPropCoord(void) const
{
	YsVec3 cen(liftCenter,0.0,0.0);
	cen.RotateXY(state.angle);
	return cen;
}

const double FsPropellerEngine::Blade::GetMomentOfInertia(void) const
{
	return weight*gravityCenter*gravityCenter;
}


////////////////////////////////////////////////////////////


FsPropellerEngine::FsPropellerEngine()
{
	Initialize();
}

void FsPropellerEngine::Initialize(void)
{
	bladeArray.CleanUp();

	engineBrakeTorquePerRpm=0.0;
	engineBrakeZeroThrottleRpm=700.0;
	engineBrakeMaxThrottleRpm=3000.0;

	maxJoulePerSec=0.0;
	idleJoulePerSec=0.0;
	radianPerSec=0.0;

	ctlRadianPerSecMin=YsPi*2.0*500.0/60.0;  // 500RPM
	ctlRadianPerSecMax=YsPi*2.0*3000.0/60.0; // 3000RPM

	sndRPMMin=0.0;
	sndRPMMax=0.0;

	rho=1.25;   // Standard air at mean sea level.
}

void FsPropellerEngine::Move(const YsVec3 &relVelAirframe,const double throttle,const double dt,YSBOOL engineOut)
{
	const double airDensityBias=rho/FsGetZeroAirDensity();
	const double joulePerSec=(YSTRUE==engineOut ? 0.0 : this->idleJoulePerSec*(1.0-throttle)+this->maxJoulePerSec*throttle)*airDensityBias;

	YsVec3 relVelInPropCoord;
	relAtt.MulInverse(relVelInPropCoord,relVelAirframe);

	double Tsum=0.0;
	double Isum=0.0;

	for(int bladeIdx=0; bladeIdx<bladeArray.GetN(); ++bladeIdx)
	{
		YsVec3 f=bladeArray[bladeIdx].CalculateForce(radianPerSec,rho,relVelInPropCoord);

		if(YsPi*2.0<bladeArray[bladeIdx].state.angle)
		{
			bladeArray[bladeIdx].state.angle-=YsPi*2.0;
		}
		else if(0.0>bladeArray[bladeIdx].state.angle)
		{
			bladeArray[bladeIdx].state.angle+=YsPi*2.0;
		}

		const YsVec3 Fxy(f.x(),f.y(),0.0);
		const YsVec3 Fcen=bladeArray[bladeIdx].GetForceCenterInPropCoord();

		const YsVec3 torqueVec=Fcen^Fxy;
		const double torquePerBlade=(YSTRUE==bladeArray[bladeIdx].clockwise ? -torqueVec.z() : torqueVec.z());

		Tsum+=torquePerBlade;
		Isum+=bladeArray[bladeIdx].GetMomentOfInertia();

		bladeArray[bladeIdx].torque=torquePerBlade;
	}

	if(YsTolerance>Isum) // 2015/04/25 Prevent infinity.
	{
		Tsum=0.0;
		Isum=1.0;
	}

	if(YsTolerance<engineBrakeTorquePerRpm)
	{
		const double rpm=fabs(radianPerSec*30.0/YsPi);
		const double engineBrakeRpm0=engineBrakeZeroThrottleRpm+(engineBrakeMaxThrottleRpm-engineBrakeZeroThrottleRpm)*throttle;
		if(engineBrakeRpm0<rpm)
		{
			const double excessRpm=rpm-engineBrakeRpm0;
			const double engineBrakeTorque=engineBrakeTorquePerRpm*excessRpm;
			if(0.0<radianPerSec)
			{
				Tsum-=engineBrakeTorque;
			}
			else
			{
				Tsum+=engineBrakeTorque;
			}
		}
	}


	double a=Tsum/Isum;
	radianPerSec+=a*dt;


	// If the blade happens to be rotating reverse, the engine should descelerate the rotation.

	const double E0=0.5*Isum*radianPerSec*radianPerSec;
	const double sign=(0.0<radianPerSec ? 1.0 : -1.0);
	const double E1=E0+sign*joulePerSec*dt;
	const double radianPerSec2=sqrt(fabs(E1)/(0.5*Isum));

	if(0.0<radianPerSec*E1)  // If E1 sign inverts, it means the direction of rotation changed from negative to positive.
	{
		radianPerSec=radianPerSec2;
	}
	else
	{
		radianPerSec=-radianPerSec2;
	}

	// 2013/10/15
	// Make sure the propeller rotates after torque is calculated.
	// Torque must be based on the force and the blade angle at the time when force is calculated.
	for(int bladeIdx=0; bladeIdx<bladeArray.GetN(); ++bladeIdx)
	{
		bladeArray[bladeIdx].state.angle+=radianPerSec*dt;
	}
}

void FsPropellerEngine::ControlPitch(double propLeverPosition,double dt)
{
	const double desiredRadianPerSec=ctlRadianPerSecMin*(1.0-propLeverPosition)+ctlRadianPerSecMax*propLeverPosition;
	const double speedDiff=radianPerSec-desiredRadianPerSec;
	for(int idx=0; idx<bladeArray.GetN(); ++idx)
	{
		const double pitchChange=bladeArray[idx].kGoverner*speedDiff*dt;
		bladeArray[idx].state.pitch+=pitchChange;
		if(bladeArray[idx].state.pitch<bladeArray[idx].minPitch)
		{
			bladeArray[idx].state.pitch=bladeArray[idx].minPitch;
		}
		else if(bladeArray[idx].state.pitch>bladeArray[idx].maxPitch)
		{
			bladeArray[idx].state.pitch=bladeArray[idx].maxPitch;
		}
	}
}

void FsPropellerEngine::SetRho(const double rho)
{
	this->rho=rho;
}

double FsPropellerEngine::GetConvergentThrust(double &radianPerSecOut,const double throttle,const double rho,const YsVec3 &relVelAirframe,int maxIter,const double dt)
{
	State stateSave;
	SaveState(stateSave);



	SetRho(rho);

	// Let's start from 1500RPM=1500*YsPi*2.0 radian per min=1500*YsPi*2.0/60.0 radian per sec
	radianPerSec=1500.0*YsPi*2.0/60.0;

	double thrust=0.0,prevThrust=0.0;
	for(int iter=0; iter<maxIter; ++iter)
	{
		Move(relVelAirframe,throttle,dt,YSFALSE);

		thrust=0.0;
		for(auto &blade : bladeArray)
		{
			thrust+=blade.lift.z()+blade.drag.z();
		}

		const double diff=fabs(thrust-prevThrust)/YsGreater(fabs(thrust),fabs(prevThrust));
		if(0.01>diff)
		{
			// printf("Converged at %d iterations.\n",iter);
			break;
		}

		prevThrust=thrust;
	}

	radianPerSecOut=radianPerSec;
	// printf("%d rpm\n",(int)(radianPerSec*60.0/(YsPi*2.0)));


	RestoreState(stateSave);

	return thrust;
}

const YsVec3 FsPropellerEngine::GetForce(void) const
{
	YsVec3 force=YsOrigin();
	for(auto &blade : bladeArray)
	{
		force+=blade.lift+blade.drag;
	}
	relAtt.Mul(force,force);
	return force;
}

void FsPropellerEngine::GetRPMRangeForSoundEffect(double &min,double &max) const
{
	min=sndRPMMin;
	max=sndRPMMax;
}



const char * const FsPropellerEngine::keyWordSource[]=
{
	"NBLADE",          //  0 1 argument.  Number of blades.
	"AREAPERBLADE",    //  1 1 argument.  Blade area.
	"CL",              //  2 4 argument.  aoa Cl1 aoa2 Cl2
	"CD",              //  3 4 argument.  aoaMinCd minCd aoa2 Cd2
	"PITCHCHGRATE",    //  4 1 argument.  Maximum angular velocity that the blade can change pitch.  (per sec)
	"MINPITCH",        //  5 1 argument.  Minimum propeller pitch.
	"MAXPITCH",        //  6 1 argument.  Maximum propeller pitch.
	"KGOVERNER",       //  7 1 argument.  Governer const.  Defines the reaction speed of the propeller governer.
	"GRAVITYCENTER",   //  8 1 argument.  Distance from the rotation axis
	"LIFTCENTER",      //  9 1 argument.  Distance from the rotation axis
	"WEIGHTPERBLADE",  // 10 1 argument.  The weight of one blade.
	"CLOCKWISE",       // 11 1 argument.  This engine rotates clockwise. Argument is the zero-based blade index.
	"COUNTERCLOCKWISE",// 12 1 argument.  This engine rotates counter-clockwise. Argument is the zero-based blade index.
	"MAXPOWER",        // 13 1 argument.  Maximum Power Output.
	"IDLEPOWER",       // 14 1 argument.  Idle Power Output.
	"RPMCTLRANGE",     // 15 2 arguments. RPM minimum and maximum for propeller control.
	"SNDRPMRANGE",     // 16 2 arguments. RPM minimum and maximum for propeller sound effect.
	"ENGBRKTRQRPM",    // 17 1 argment.   Torque (Nm) from engine brake per excess RPM.
	"ENGBRK0THRRPM",   // 18 1 argument.  RPM at which the engine wants to spin at zero throttle.
	"ENGBRKMAXTHRRPM", // 19 1 argument.  RPM at which the engine wants to spin at max throttle.

NULL
};

YsKeyWordList FsPropellerEngine::keyWordList;

YsString FsPropellerEngine::MakeShortFormat(const YsString &str)
{
	if(keyWordList.GetN()==0)
	{
		keyWordList.MakeList(keyWordSource);
	}

	YsString shortFormat;
	for(YSSIZE_T ptr=0; ptr<str.Strlen(); ++ptr)
	{
		if('A'<=str[ptr] && str[ptr]<='Z')
		{
			YsString nextWord;
			YSSIZE_T wordLen=0;
			while(wordLen+ptr<str.Strlen() && str[ptr+wordLen]!=' ' && str[ptr+wordLen]!='\t')
			{
				nextWord.Append(str[ptr+wordLen]);
				++wordLen;
			}
			ptr=ptr+wordLen-1;

			auto cmdIdx=keyWordList.GetId(nextWord);
			if(0<=cmdIdx)
			{
				nextWord.Printf("*%d",(int)cmdIdx);
			}

			shortFormat.Append(nextWord);
		}
		else
		{
			shortFormat.Append(str[ptr]);
		}
	}

	return shortFormat;
}

YSRESULT FsPropellerEngine::SendCommand(YSSIZE_T ac,const YsString av[])
{
	YsArray <const char *> argv(ac,NULL);
	for(YSSIZE_T idx=0; idx<ac; ++idx)
	{
		argv[idx]=av[idx];
	}
	return SendCommand(ac,argv);
}

YSRESULT FsPropellerEngine::SendCommand(YSSIZE_T ac,const char *const av[])
{
	if(0>=ac)
	{
		return YSOK;
	}

	if(keyWordList.GetN()==0)
	{
		keyWordList.MakeList(keyWordSource);
	}

	int cmd=-1;
	if('*'==av[0][0])
	{
		cmd=atoi(av[0]+1);
	}
	else
	{
		cmd=keyWordList.GetId(av[0]);
	}
	if(0>cmd)
	{
		return YSERR;
	}

	switch(cmd)
	{
	case 0:  // "NBLADE",
		if(2<=ac)
		{
			const int nBlade=atoi(av[1]);
			bladeArray.Set(nBlade,NULL);
			int n=0;
			for(auto &blade : bladeArray)
			{
				blade.Initialize();
				blade.state.angle=YsPi*2.0*(double)n/(double)nBlade;
				++n;
			}
		}
		else
		{
			YsPrintf("Too few arguments for %s\n",(const char *)av[0]);
			return YSERR;
		}
		break;
	case 1:  // "AREAPERBLADE",
		if(2<=ac)
		{
			double bladeArea;
			if(YSOK==FsGetArea(bladeArea,av[1]))
			{
				for(auto &blade : bladeArray)
				{
					blade.area=bladeArea;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 2:  // "CL",
		if(5<=ac)
		{
			double aoa0;
			double aoa1;

			if(YSOK==FsGetAngle(aoa0,av[1]) && YSOK==FsGetAngle(aoa1,av[3]))
			{
				const double cl0=atof(av[2]);
				const double cl1=atof(av[4]);

				const double kCl=(cl1-cl0)/(aoa1-aoa0);
				const double ClZero=cl0-kCl*aoa0;

				for(auto &blade : bladeArray)
				{
					blade.kCl=kCl;
					blade.ClZero=ClZero;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 3:  // "CD",
		if(5<=ac)
		{
			double minCdAOA;
			double aoa1;

			if(YSOK==FsGetAngle(minCdAOA,av[1]) && YSOK==FsGetAngle(aoa1,av[3]))
			{
				const double CdMin=atof(av[2]);
				const double cd1=atof(av[4]);

				// cd=CdMin+kCd*(aoa-minCdAOA)^2

				const double dAOA=aoa1-minCdAOA;
				const double kCd=(cd1-CdMin)/(dAOA*dAOA);

				for(auto &blade : bladeArray)
				{
					blade.kCd=kCd;
					blade.CdMin=CdMin;
					blade.minCdAOA=minCdAOA;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 4:  // "PITCHCHGRATE",
		if(2<=ac)
		{
			double pitchChangeSpeed;
			if(YSOK==FsGetAngle(pitchChangeSpeed,av[1]))
			{
				for(auto &blade : bladeArray)
				{
					blade.pitchChangeSpeed=pitchChangeSpeed;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 5:  // "MINPITCH",
		if(2<=ac)
		{
			double minPitch;
			if(YSOK==FsGetAngle(minPitch,av[1]))
			{
				for(auto &blade : bladeArray)
				{
					blade.minPitch=minPitch;
					blade.state.pitch=(blade.minPitch+blade.maxPitch)/2.0;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 6:  // "MAXPITCH",
		if(2<=ac)
		{
			double maxPitch;
			if(YSOK==FsGetAngle(maxPitch,av[1]))
			{
				for(auto &blade : bladeArray)
				{
					blade.maxPitch=maxPitch;
					blade.state.pitch=(blade.minPitch+blade.maxPitch)/2.0;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 7: // "KGOVERNER",
		if(2<=ac)
		{
			double kGoverner=atof(av[1]);
			for(auto &blade : bladeArray)
			{
				blade.kGoverner=kGoverner;
			}
		}
		break;
	case 8: // "GRAVITYCENTER",  // Distance from the rotation axis
		if(2<=ac)
		{
			double gravityCenter;
			if(YSOK==FsGetLength(gravityCenter,av[1]))
			{
				for(auto &blade : bladeArray)
				{
					blade.gravityCenter=gravityCenter;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 9: // "LIFTCENTER",     // Distance from the rotation axis
		if(2<=ac)
		{
			double liftCenter;
			if(YSOK==FsGetLength(liftCenter,av[1]))
			{
				for(auto &blade : bladeArray)
				{
					blade.liftCenter=liftCenter;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 10: // "WEIGHTPERBLADE",
		if(2<=ac)
		{
			double weight;
			if(YSOK==FsGetWeight(weight,av[1]))
			{
				for(auto &blade : bladeArray)
				{
					blade.weight=weight;
				}
			}
			else
			{
				return YSERR;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 11: // "CLOCKWISE",
		if(2<=ac)
		{
			const int bladeIndex=atoi(av[1]);
			if(YSTRUE==bladeArray.IsInRange(bladeIndex))
			{
				bladeArray[bladeIndex].clockwise=YSTRUE;
			}
		}
		break;
	case 12: // "COUNTERCLOCKWISE",
		if(2<=ac)
		{
			const int bladeIndex=atoi(av[1]);
			if(YSTRUE==bladeArray.IsInRange(bladeIndex))
			{
				bladeArray[bladeIndex].clockwise=YSFALSE;
			}
		}
		break;
	case 13: // "MAXPOWER"
		if(2<=ac)
		{
			double power;
			if(YSOK==FsGetJoulePerSecond(power,av[1]))
			{
				maxJoulePerSec=power;
			}
			else
			{
				return YSERR;
			}
		}
		break;
	case 14: // "IDLEPOWER"
		if(2<=ac)
		{
			double power;
			if(YSOK==FsGetJoulePerSecond(power,av[1]))
			{
				idleJoulePerSec=power;
			}
			else
			{
				return YSERR;
			}
		}
		break;
	case 15: // "RPMCTLRANGE",     // 2 arguments. RPM minimum and maximum for propeller control.
		if(3<=ac)
		{
			ctlRadianPerSecMin=atof(av[1])*YsPi*2.0/60.0;
			ctlRadianPerSecMax=atof(av[2])*YsPi*2.0/60.0;
		}
		break;
	case 16: // 	"SNDRPMRANGE",     // 2 arguments. RPM minimum and maximum for propeller sound effect.
		if(3<=ac)
		{
			sndRPMMin=atof(av[1]);
			sndRPMMax=atof(av[2]);
		}
		break;

	case 17: // "ENGBRKTRQRPM",    // 17 1 argment.   Torque from engine brake per excess RPM.
		if(2<=ac)
		{
			engineBrakeTorquePerRpm=atof(av[1]);
		}
		break;
	case 18: // "ENGBRK0THRRPM",   // 18 1 argument.  RPM at which the engine wants to spin at zero throttle.
		if(2<=ac)
		{
			engineBrakeZeroThrottleRpm=atof(av[1]);
		}
		break;
	case 19: // "ENGBRKMAXTHRRPM", // 19 1 argument.  RPM at which the engine wants to spin at max throttle.
		if(2<=ac)
		{
			engineBrakeMaxThrottleRpm=atof(av[1]);
		}
		break;

	default:
		YsPrintf("Unrecognized command sent to FsPropellerEngine.\n");
		return YSERR;
	}

	return YSOK;
}

void FsPropellerEngine::SaveState(State &state) const
{
	state.rho=rho;
	state.radianPerSec=radianPerSec;

	state.bladeStateArray.Set(bladeArray.GetN(),NULL);
	for(YSSIZE_T bladeIdx=0; bladeIdx<bladeArray.GetN(); ++bladeIdx)
	{
		state.bladeStateArray[bladeIdx]=bladeArray[bladeIdx].state;
	}
}

void FsPropellerEngine::RestoreState(const State &state)
{
	rho=state.rho;
	radianPerSec=state.radianPerSec;

	for(YSSIZE_T bladeIdx=0; bladeIdx<bladeArray.GetN(); ++bladeIdx)
	{
		bladeArray[bladeIdx].state=state.bladeStateArray[bladeIdx];
	}
}


/* int main(void)
{
	FsPropellerEngine prop;
	prop.relAtt=YsZeroAtt();

	for(int i=0; i<2; ++i)
	{
		prop.bladeArray.Increment();

		prop.bladeArray.GetEnd().clockwise=YSTRUE;

		prop.bladeArray.GetEnd().kCl=1.4/YsDegToRad(20.0);   // Slope Cl 1.4 per 20 deg, Clark-Y
		prop.bladeArray.GetEnd().ClZero=0.35;
		prop.bladeArray.GetEnd().minCdAOA=YsDegToRad(-5.0);
		prop.bladeArray.GetEnd().CdMin=0.012;
		prop.bladeArray.GetEnd().kCd=0.213/(YsSqr(YsDegToRad(25.0)));

		prop.bladeArray.GetEnd().pitchChangeSpeed=YsPi;  // 180 degree per sec.  It moves pretty quickly.
		prop.bladeArray.GetEnd().minPitch=YsPi/36.0;
		prop.bladeArray.GetEnd().maxPitch=YsPi/4.0;
		prop.bladeArray.GetEnd().kGoverner=0.05;

		prop.bladeArray.GetEnd().area=0.25;           // Something like 0.25m wide 1.0m long
		prop.bladeArray.GetEnd().gravityCenter=0.6;   // 1m length, probably 60cm off.
		prop.bladeArray.GetEnd().liftCenter=0.6;      // 1m length, probably 60cm off.
		prop.bladeArray.GetEnd().weight=10.0;         // Let's say 10kg

		prop.bladeArray.GetEnd().state.angle=YsPi*(double)i;
		prop.bladeArray.GetEnd().state.pitch=YsDegToRad(18.0);
	}

	prop.joulePerSec=

	double rpm=1000.0;
	prop.radianPerSec=(rpm/60.0)*YsPi*2.0;

	double desiredRadianPerSec=(1500.0/60.0)*YsPi*2.0;

	const double horsePower=160.0;


	FsOpenWindow(256,16,800,600,1);

	double thr=0.1; // 1.0;
	double vel=0.0; // 55.0;  // roughly 106kt

	
	for(;;)
	{
		int key=FsInkey();
		if(FSKEY_ESC==key)
		{
			break;
		}
		else if(FSKEY_Q==key)
		{
			thr+=0.05;
		}
		else if(FSKEY_A==key)
		{
			thr-=0.05;
		}
		else if(FSKEY_W==key)
		{
			vel+=5.0;
		}
		else if(FSKEY_S==key)
		{
			vel-=5.0;
		}
		else if(FSKEY_E==key)
		{
			desiredRadianPerSec+=(100/60.0)*YsPi*2.0;
		}
		else if(FSKEY_D==key)
		{
			desiredRadianPerSec-=(100/60.0)*YsPi*2.0;
		}


		FsPollDevice();

		double dt=1.0/60.0;

		const double JperS=horsePower*745.7;
		prop.Move(YsVec3(0.0,0.0,vel),thr*JperS,dt);
		prop.ControlPitch(desiredRadianPerSec,dt);

		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		int y=32;
		YsString str;

		str.Printf("%6.2lf deg/sec  %8.2lf RPM",YsRadToDeg(prop.radianPerSec),fabs(prop.radianPerSec*60.0/(YsPi*2.0)));
		glRasterPos2d(32,y);
		YsGlDrawFontBitmap16x24(str);
		y+=24;

		str.Printf("Desired %6.2lf RPM",desiredRadianPerSec*60.0/(YsPi*2.0));
		glRasterPos2d(32,y);
		YsGlDrawFontBitmap16x24(str);
		y+=24;

		for(int i=0; i<prop.bladeArray.GetN(); ++i)
		{
			str.Printf("Prop %d  Torque %lf  Pitch %6.2lf deg\n",i,prop.bladeArray[i].torque,YsRadToDeg(prop.bladeArray[i].state.pitch));
			glRasterPos2d(32,y);
			YsGlDrawFontBitmap16x24(str);
			y+=24;
		}

		str.Printf("Power %6.2lf\n",thr);
		glRasterPos2d(32,y);
		YsGlDrawFontBitmap16x24(str);
		y+=24;

		str.Printf("%6.2lf kt\n",vel*3600/1800);
		glRasterPos2d(32,y);
		YsGlDrawFontBitmap16x24(str);
		y+=24;

		FsSwapBuffers();

		FsSleep(10);
	}

	return 0;
} */


