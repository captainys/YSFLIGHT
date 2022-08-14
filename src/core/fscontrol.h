#ifndef FSCONTROL_IS_INCLUDED
#define FSCONTROL_IS_INCLUDED
/* { */

#define FSSIMPLEWINDOW_MACRO_ONLY
#include <fssimplewindow.h>
#undef FSSIMPLEWINDOW_MACRO_ONLY


// Declaration /////////////////////////////////////////////
// Functions
enum FSAXISFUNCTION  // <=
{
	FSAXF_NULL,
	FSAXF_AILERON,                       //  Aileron
	FSAXF_ELEVATOR,                      //  Elevator
	FSAXF_THROTTLE,                      //  Throttle
	FSAXF_RUDDER,                        //  Rudder
	FSAXF_FLAP,                          //  Flap
	FSAXF_LANDINGGEAR,                   //  Landing Gear
	FSAXF_TRIM,                          //  Elevator Trim
	FSAXF_NOZZLE,                        //  Nozzle
	FSAXF_VGW,                           //  Variable Geometry Wing
	FSAXF_SPOILERBRAKE,                  //  Spoiler + Brake
	FSAXF_SPOILER,                       //  Spoiler
	FSAXF_BRAKE,                         //  Brake

	FSAXF_POVX,                          // POV-X
	FSAXF_POVY,                          // POV-Y

	FSAXF_TURRETH,                       // TURRET HEADING
	FSAXF_TURRETP,                       // TURRET PITCH

	FSAXF_POVX_180DEG,                   // POV-X Plus/Minus 180 degree

	FSAXF_PROPELLER,                     // Propeller lever.

	FSAXF_THROTTLE_UPDOWN,               // Throttle Increase/Decrease

FSAXF_NUMAXISFUNCTION
};

enum FSBUTTONFUNCTION
{
	FSBTF_NULL,
	FSBTF_ELEVATORUP,                    //  Elevator Up
	FSBTF_ELEVATORNEUTRAL,               //  Elevator Neutral
	FSBTF_ELEVATORDOWN,                  //  Elevator Down
	FSBTF_AILERONLEFT,                   //  Aileron Left
	FSBTF_AILERONNEUTRAL,                //  Aileron Neutral
	FSBTF_AILERONRIGHT,                  //  Aileron Right
	FSBTF_RUDDERLEFT,                    //  Rudder Left
	FSBTF_RUDDERCENTER,                  //  Rudder Center
	FSBTF_RUDDERRIGHT,                   //  Rudder Right
	FSBTF_TRIMUP,                        //  Elevator Trim Up
	FSBTF_TRIMDOWN,                      //  Elevator Trim Down
	FSBTF_AUTOTRIM,                      //  Auto Trim
	FSBTF_THROTTLEUP,                    //  Throttle Add Power
	FSBTF_THROTTLEDOWN,                  //  Throttle Reduce Power
	FSBTF_THROTTLEMAX,                   //  Throttle Max
	FSBTF_THROTTLEIDLE,                  //  Throttle Min (Idle)
	FSBTF_AFTERBURNER,                   //  Afterburner
	FSBTF_NOZZLEUP,                      //  Nozzle Up (Nose Up for Concorde)
	FSBTF_NOZZLEDOWN,                    //  Nozzle Down (Nose Down for Concorde)
	FSBTF_VGWEXTEND,                     //  Extend Variable Geometry Wing
	FSBTF_VGWRETRACT,                    //  Retract Variable Geometry Wing
	FSBTF_LANDINGGEAR,                   //  Landing Gear Extend/Retract
	FSBTF_FLAP,                          //  Flap Up/Down
	FSBTF_FLAPUP,                        //  Flap Up
	FSBTF_FLAPDOWN,                      //  Flap Down
	FSBTF_FLAPFULLUP,                    //  Flap Full Up
	FSBTF_FLAPFULLDOWN,                  //  Flap Full Down
	FSBTF_SPOILERBRAKE,                  //  Spoiler and Brake On/Off
	FSBTF_SPOILER,                       //  Spoiler Extend/Retract
	FSBTF_SPOILEREXTEND,                 //  Spoiler Extend
	FSBTF_SPOILERRETRACT,                //  Spoiler Retract
	FSBTF_SPOILERHOLD,                   //  Spoiler On When Pressed
	FSBTF_BRAKEONOFF,                    //  Brake On/Off
	FSBTF_BRAKEHOLD,                     //  Brake On When Pressed
	FSBTF_FIREWEAPON,                    //  Fire Selected Weapon
	FSBTF_FIREGUN,                       //  Fire Machine Gun
	FSBTF_FIREAAM,                       //  Fire AAM
	FSBTF_FIREAGM,                       //  Fire AAM
	FSBTF_FIREROCKET,                    //  Fire Rocket
	FSBTF_DROPBOMB,                      //  Drop Bomb
	FSBTF_DISPENSEFLARE,                 //  Dispense Flare
	FSBTF_SMOKE,                         //  Smoke
	FSBTF_CYCLESMOKESELECTOR,            // Cycle smoke selector
	FSBTF_SELECTWEAPON,                  //  Select Weapon
	FSBTF_RADAR,                         //  Radar
	FSBTF_RADARRANGEUP,                  //  Radar Range Up
	FSBTF_RADARRANGEDOWN,                //  Radar Range Down
	FSBTF_ILS,                           //  ILS On/Off
	FSBTF_VELOCITYINDICATOR,             //  Velocity Indicator On/Off

	FSBTF_OPENAUTOPILOTMENU,             //  Open Autopilot Menu
	FSBTF_OPENRADIOCOMMMENU,             //  Open Radio Comm Menu

	FSBTF_COCKPITVIEW,                   //  Cockpit View
	FSBTF_OUTSIDEPLAYERVIEW,             //  Outside Player View
	FSBTF_COMPUTERAIRPLANEVIEW,          //  Computer Airplane View
	FSBTF_WEAPONVIEW,                    //  Weapon View
	FSBTF_ILSVIEW,                       //  ILS View
	FSBTF_OUTSIDEPLAYERVIEW2,            //  Outside Player View 2
	FSBTF_OUTSIDEPLAYERVIEW3,            //  Outside Player View 3
	FSBTF_CONTROLTOWERVIEW,              //  Control Tower View
	FSBTF_CHANGEAIRPLANE,                //  Change Airplane in Replay Record Mode

	FSBTF_LOOKFORWARD,                   //  Look Forward
	FSBTF_LOOKRIGHT,                     //  Look Right
	FSBTF_LOOKLEFT,                      //  Look Left
	FSBTF_LOOKBACK,                      //  Look Back
	FSBTF_LOOKUP,                        //  Look Up
	FSBTF_LOOKDOWN,                      //  Look Down


	FSBTF_REVERSETHRUST,                 //  Thrust reverser

	FSBTF_OPENSUBWINDOWMENU,             //  Open Sub Window Menu

	FSBTF_CHANGEHUDCOLOR,                //  Change HUD color
	FSBTF_PAUSE,                         //  Pause

	FSBTF_GHOSTVIEW,                     //  Ghost mode.

	FSBTF_OPENVORMENU,                   //  Open VOR Menu
	FSBTF_ROTATEVORLEFT,                 //  Rotate VOR Heading Left
	FSBTF_ROTATEVORRIGHT,                //  Rotate VOR Heading Right

	FSBTF_BOMBBAYDOOR,

	FSBTF_INFLTCONFIG,                   // In flight config change
	FSBTF_INFLTMESSAGE,                  // In flight chat message

	FSBTF_TURRETLEFT,                    // Rotate Turret left
	FSBTF_TURRETRIGHT,                   // Rotate Turret right
	FSBTF_TURRETUP,                      // Rotate Turret up
	FSBTF_TURRETDOWN,                    // Rotate Turret down

	FSBTF_VIEWZOOM,                      // Zoom In
	FSBTF_VIEWMOOZ,                      // Zoom Out

	FSBTF_TURRETNEUTRAL,                 // Move Turret to the neutral position
	FSBTF_THROTTLEUP_HALF,               // Throttle Up (2.5%)
	FSBTF_THROTTLEDOWN_HALF,             // Throttle Down (2.5%)

	FSBTF_OPENADFMENU,                   // Open ADF Menu (NDB Selection)

	FSBTF_SUPPLYDIALOG,                  // Open supply dialog

	FSBTF_TOGGLELIGHT,                   // Turn on/off all lights
	FSBTF_TOGGLENAVLIGHT,                // Turn on/off Nav lights
	FSBTF_TOGGLEBEACON,                  // Turn on/off Beacon
	FSBTF_TOGGLESTROBE,                  // Turn on/off Strobe
	FSBTF_TOGGLELANDINGLIGHT,            // Turn on/off Landing lights

	FSBTF_TOGGLEALLDOOR,                 // Open/Close All Door
	FSBTF_TOGGLELEFTDOOR,                // Open/Close Left Door
	FSBTF_TOGGLERIGHTDOOR,               // Open/Close Right Door
	FSBTF_TOGGLEREARDOOR,                // Open/Close Rear Door

	FSBTF_SENSITIVITYUP,                 // Increase stick sensitivity
	FSBTF_SENSITIVITYDOWN,               // Descrease stick sensitivity

	FSBTF_CYCLESENSITIVITY,              // Cycle stick sensitivity 1.0->0.25->0.5->0.75
	FSBTF_PROPFORWARD,                   // Increase rpm
	FSBTF_PROPBACKWARD,                  // Decrease rpm

	FSBTF_SWITCHVIEWTARGET,              // Switch view target

FSBTF_NUMBUTTONFUNCTION
};

const char *FsGetAxisFuncLabel(FSAXISFUNCTION fnc);
const char *FsGetAxisFuncString(FSAXISFUNCTION fnc);
const char *FsGetButtonFuncLabel(FSBUTTONFUNCTION fnc);
const char *FsGetButtonFuncString(FSBUTTONFUNCTION fnc);
const char *FsGetKeyLabel(int keyCode);
const char *FsGetKeyString(int keyCode);

FSAXISFUNCTION FsGetAxisFuncFromString(const char str[]);
FSBUTTONFUNCTION FsGetButtonFuncFromString(const char str[]);
int FsGetKeycodeFromString(const char str[]);

////////////////////////////////////////////////////////////


class FsFlightControl
{
public:
	static void PrepareJoystickPolygonModel(void);

	FsFlightControl();
	FsFlightControl(const FsFlightControl &from);
	void Initialize(void);

	// Control
	double ctlGear;
	YSBOOL ctlGearTrouble;
	double ctlBrake;
	double ctlSpoiler;
	YSBOOL hasAb;
	YSBOOL ctlAb;
	double ctlThrottle;
	double ctlPropeller;
	double ctlFlap;
	double ctlVgw;
	YSBOOL ctlAutoVgw;
	double ctlThrVec;
	double ctlThrRev;
	YSBOOL ctlBombBayDoor;

	double ctlLeftDoor,ctlRightDoor,ctlRearDoor;

	double ctlSensitivity;

	YSBOOL brakeHold;
	YSBOOL spoilerHold;

	// Navigation Aid
private:
	int ctlNavId;   // 0:NAV-1  1:NAV-2  100:ADF

public:
	YSBOOL ctlVectorMarker;


	double ctlElevator,ctlElvTrim,ctlRudder,ctlAileron;
	double ctlTurretHdg,ctlTurretPch;

	// Buttn press from external source (script) >>
	YSBOOL ctlFireWeaponButtonExt;
	YSBOOL ctlFireGunButtonExt;
	YSBOOL ctlFireAAMButtonExt;
	YSBOOL ctlFireAGMButtonExt;
	YSBOOL ctlFireRocketButtonExt;
	YSBOOL ctlDropBombButtonExt;
	YSBOOL ctlDispenseFlareButtonExt;
	YSBOOL ctlCycleWeaponButtonExt;
	YSBOOL ctlSmokeButtonExt;
	YSBOOL ctlCycleSmokeSelectorButtonExt;
	// Buttn press from external source (script) <<

	YSBOOL ctlFireWeaponButton;
	YSBOOL ctlFireGunButton;
	YSBOOL ctlFireAAMButton;
	YSBOOL ctlFireAGMButton;
	YSBOOL ctlFireRocketButton;
	YSBOOL ctlDropBombButton;
	YSBOOL ctlDispenseFlareButton;
	YSBOOL ctlCycleWeaponButton;
	YSBOOL ctlSmokeButton;
	YSBOOL ctlCycleSmokeSelectorButton;

	int pov;
	double viewHdg,viewPch;

	YSRESULT ProcessButtonFunction(const double &currentTime,class FsExistence *existence,FSBUTTONFUNCTION fnc);
	void CycleNav(void);
	void SelectNav(int navIdIn);
	int Nav(void) const;

	void SensitivityDown(void);
	void SensitivityUp(void);
	void CycleSensitivity(void);
	const double GetSensitivity(void) const;

	YSRESULT ReadControl(const class FsControlAssignment &ctlAssign,class FsJoystick joy[FsMaxNumJoystick]);
	YSRESULT ReadControl
	    (const class FsControlAssignment &ctlAssign,
	     class FsJoystick pJoy[FsMaxNumJoystick],class FsJoystick joy[FsMaxNumJoystick]);
	YSBOOL SetControlAxis
	    (const class FsControlAssignment &ctlAssign,
	     double &axs,YSBOOL defReverse,YSBOOL twoSide,const double &deadZone,FSAXISFUNCTION fnc,
	     FsJoystick pJoy[FsMaxNumJoystick],FsJoystick joy[FsMaxNumJoystick]);
	YSBOOL PollControlAxis
	    (const class FsControlAssignment &ctlAssign,
	     double &axs,YSBOOL defReverse,YSBOOL twoSide,FSAXISFUNCTION fnc,
	     const FsJoystick joy[FsMaxNumJoystick]) const;
	void Move(class FsControlAssignment &ctlAssign,const FsJoystick joy[FsMaxNumJoystick],const double &dt);


	YSRESULT VerifyAndFixJoystickAxisAssignment(FsControlAssignment &ctlAssign);
	void DrawJoystick(const YsVec3 &pos,const YsAtt3 &att) const;
	void DrawThrottle(const YsVec3 &pos,const YsAtt3 &att) const;
	void DrawRudder(const YsVec3 &pos,const YsAtt3 &att) const;
	YSRESULT CenterJoystick(class FsControlAssignment &ctlAssign);
private:
	double Margin(double org,const double deadZone) const;
};



////////////////////////////////////////////////////////////

const int FsMouseJoyId=FsMaxNumJoystick-1;

class FsAxisAssignment
{
public:
	int joyId,joyAxs;     // joyId==FsMouseJoyId means mouse
	YSBOOL reverse;
	FSAXISFUNCTION fnc;
};

class FsTriggerAssignment
{
public:
	int joyId,joyTrg;
	FSBUTTONFUNCTION fnc;
};

class FsKeyAssignment
{
public:
	int keyCode;
	FSBUTTONFUNCTION fnc;
};

class FsControlAssignment
{
public:
	FsControlAssignment();

	YSBOOL processNumberKey;
	YSBOOL checkKeyHolding;  // YSFALSE -> IsButtonPressed() will not check keys.  For in flight config/chat.
private:
	/* This variable is set when a raw key is processed by an in-flight dialog, submenu, or anything 
	   that must block key stroke from processed as a button function.
	   Key status of this key is checked in ReadControl.  If FsGetKeyState(ignoreThisKeyHolding) is YSFALSE,
	   ignoreThisKeyHolding is set back to FSKEY_NULL.  
	   If ignoreThisKeyHolding is not FSKEY_NULL, IsButtonPressed ignores this key.

	   CheckIgnoredKeyRelease checks if FsGetKeyState(ignoreThisKeyHolding) and updates the key code.
	   CheckIgnoredKeyRelease is called from SimControlByUser
	*/
	int ignoreThisKeyHolding;

public:
	void CleanUp(void);
	void CleanUpAxisAndButtonAssignment(void);
	void SetDefault(int primaryJoyId);
	void SetDefaultGamePad(int primaryJoyId);
	void SetDefaultMouseAsStick(void);
	void SetDefaultKeyboardAsStick(void);
	void SetDefaultKeyAssign(void);
	void BuildMapping(void);
	YSRESULT AddAxisAssignment(int joyId,int joyAxs,FSAXISFUNCTION fnc,YSBOOL reverse);
	YSRESULT DeleteAxisFunction(FSAXISFUNCTION fnc);
	YSRESULT DeleteAxis(int joyId,int joyAxs);
	YSRESULT AddTriggerAssignment(int joyId,int joyTrg,FSBUTTONFUNCTION fnc);
	YSRESULT DeleteTriggerFunction(FSBUTTONFUNCTION fnc);
	YSRESULT DeleteTrigger(int joyId,int joyTrg);
	YSRESULT AddKeyAssignment(int keyCode,FSBUTTONFUNCTION fnc);
	YSRESULT DeleteKeyFunction(FSBUTTONFUNCTION fnc);
	YSRESULT DeleteKey(int keyCode);

	void SetIgnoreThisKeyHolding(int key);
	void CheckIgnoredKeyRelease(void);

	FSAXISFUNCTION TranslateAxis(int joyId,int joyAxs) const;
	FSBUTTONFUNCTION TranslateKeyStroke(int keyCode) const;
	FSBUTTONFUNCTION TranslateTrigger(int joyId,int joyTrg) const;
	int FindKeyByFunction(FSBUTTONFUNCTION fnc) const;
	YSRESULT FindTriggerByFunction(int &joyId,int &joyTrg,FSBUTTONFUNCTION fnc) const;
	YSRESULT FindAxisByFunction(int &joyId,int &joyAxs,YSBOOL &reverse,FSAXISFUNCTION fnc) const;

	YSBOOL IsButtonPressed(FSBUTTONFUNCTION fnc,class FsJoystick joy[FsMaxNumJoystick]) const;

	YSRESULT Load(const wchar_t fn[]);
	YSRESULT Save(const wchar_t fn[]);

	static YSRESULT MergeDefaultControl(void);


	YsListAllocator <FsAxisAssignment> axsAssignAllocator;
	YsListAllocator <FsTriggerAssignment> trgAssignAllocator;
	YsListAllocator <FsKeyAssignment> keyAssignAllocator;

	YsListContainer <FsAxisAssignment> axsAssignList;
	YsListContainer <FsTriggerAssignment> trgAssignList;
	YsListContainer <FsKeyAssignment> keyAssignList;

	double deadZoneElevator,deadZoneAileron,deadZoneRudder;
	YSBOOL usePovHatSwitch;

protected:
	FsAxisAssignment *axisToFuncMap[FsMaxNumJoystick][FsMaxNumJoyAxis];
	FsAxisAssignment *funcToAxisMap[FSAXF_NUMAXISFUNCTION];

	FsTriggerAssignment *trgToFuncMap[FsMaxNumJoystick][FsMaxNumJoyTrig];
	FsTriggerAssignment *funcToTrgMap[FSBTF_NUMBUTTONFUNCTION];

	FsKeyAssignment *keyToFuncMap[FSKEY_NUM_KEYCODE];
	FsKeyAssignment *funcToKeyMap[FSBTF_NUMBUTTONFUNCTION];
};


class FsCenterJoystick
{
public:
	YSRESULT res;

	enum STATE
	{
		INITIAL,
		WAITING_FOR_BUTTON,
		WAITING_FOR_RELEASE,
		OVER
	};

	STATE state;

	int nextActionCode;

	FsJoystick *pJoy,*joy;

	FsFlightControl *ctl;
	const FsControlAssignment *ctlAssign;

	time_t waiting,waitStart;

	FsCenterJoystick();
	~FsCenterJoystick();
	void Initialize(FsFlightControl *ctl,const FsControlAssignment *ctlAssign,int nextAction);
	void RunOneStep(void);
	void Draw(void) const;
};

/* } */
#endif
