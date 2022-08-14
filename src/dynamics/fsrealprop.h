#ifndef FSREALPROP_H_IS_INCLUDED
#define FSREALPROP_H_IS_INCLUDED
/* { */

#include <ysclass.h>
#include "fsengine.h"

class FsPropellerEngine : public FsAircraftEngine
{
public:
	class Blade
	{
	public:
		class State
		{
		public:
			double angle;  // 0 deg is up.
			double pitch;  // Can be constant
		};

		// Constants
		double area;
		double kCl,ClZero;
		double kCd,CdMin,minCdAOA;

		double pitchChangeSpeed;
		double minPitch,maxPitch;  // For constant-speed propeller
		double kGoverner;          // Proportional controller constant  pitch+=(radianPerSec-desiredRadianPerSec)*kGoverner

		double gravityCenter;   // From the spinning axis to the center of gravity.
		double liftCenter;      // From the spinning axis to the center of lift.

		double weight;

		YSBOOL clockwise;  // Direction of rotation must be the property of the blade.  Imagine co-axial counter-rotating props.

		// Variable
		State state;

		double torque;
		double aoa;

		// Output
		YsVec3 lift,drag; // In spinner coordinate

		Blade();
		void Initialize(void);

		/*! Incoming parameter relVel is the velocity relative to the airframe.
		    rotSpeed is radian per sec.
			Return value is a force vector relative to the airframe.
		    */
		YsVec3 CalculateForce(const double radianPerSec,const double rho,const YsVec3 &relVelInPropCoord);
		YsVec3 GetForceCenterInPropCoord(void) const;
		const double GetMomentOfInertia(void) const;
	};


	class State
	{
	public:
		YsArray <Blade::State,8> bladeStateArray;
		double rho;
		double radianPerSec;
	};


	YsArray <Blade> bladeArray;

	// What an approximation! :-P
	double engineBrakeTorquePerRpm;    // Torque for excess 100rpm
	double engineBrakeZeroThrottleRpm; // RPM at which the engine brake kicks in at 0 throttle
	double engineBrakeMaxThrottleRpm;  // RPM at which the engine brake kicks in at max throttle

	double maxJoulePerSec;
	double idleJoulePerSec;
	double rho;

	double radianPerSec;  // Always positive.  There may be two counter-rotating blade per engine.

	double ctlRadianPerSecMin,ctlRadianPerSecMax;

	double sndRPMMin,sndRPMMax;

	static const char * const keyWordSource[];
	static YsKeyWordList keyWordList;

	FsPropellerEngine();

	void Initialize(void);

	/*! 
	\p relVelAirframe is the relative velocity in the airframe coordinate system.
	\p throttle is the current throttle (0.0 to 1.0).
	\p dt is the time step.
	\p engineOut should be true if the engine is not running (engine failure or fuel exhaustion)
	*/
	void Move(const YsVec3 &relVelAirframe,const double throttle,const double dt,YSBOOL engineOut);

	void ControlPitch(double propLeverPosition,double dt);

	/*! This function sets the air density. */
	void SetRho(const double rho);

	/*! This function returns the convergent thrust for the given throttle setting. */
	double GetConvergentThrust(double &radianPerSec,const double throttle,const double rho,const YsVec3 &relVelAirframe,int maxIter,const double dt);

	/*! This function returns the total force in the airframe coordinate. */
	const YsVec3 GetForce(void) const;

	void GetRPMRangeForSoundEffect(double &min,double &max) const;

	void SaveState(State &state) const;
	void RestoreState(const State &state);

	static YsString MakeShortFormat(const YsString &str);

	YSRESULT SendCommand(YSSIZE_T argc,const YsString argv[]);
	YSRESULT SendCommand(YSSIZE_T argc,const char *const argv[]);
};

/* } */
#endif
