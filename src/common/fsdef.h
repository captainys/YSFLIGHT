#ifndef FSDEF_IS_INCLUDED
#define FSDEF_IS_INCLUDED
/* { */

#include <ysclass.h>

#define YSFLIGHT_VERSION 20181124
#define YSFLIGHT_YFSVERSION 20180930
#define YSFLIGHT_NETVERSION 20180930

// VERSION 20060514 : 2006 Summer Test Ver.
// VERSION 20060828 : 2006 Summer Official Ver.
// VERSION 20070107 : 2007 Spring Test Ver.
// VERSION 20070415 : 2007 Spring Official
// VERSION 20070907 : 2007 Fall Test
// VERSION 20080220 : 2008 Spring Official
// VERSION 20090327 : 2009 Spring Test
// VERSION 20100331 : 2010 Spring Official
// VERSION 20101124 : 2010 Fall Test
// VERSION 20110207 : 2011 Spring Official
// VERSION 20120201 : 2012 Spring Test
// VERSION 20120701 : 2012 Summer Official
// VERSION 20130416 : 2013 Spring test
// VERSION 20130805 : 2013 Summer Official
// VERSION 20140909 : 2014 Fall test
// VERSION 20170506 : 2017 Spring test

// YFS 20030109 : Flash of explosion, Explosion Version is 2 (see FsExplosionHolde::Save)
// YFS 20060514 : 2006 Summer Test Ver.
// YFS 20060828 : 2006 Summer Official
// YFS 20070107 : 2007 Spring Test Ver.
// YFS 20070415 : 2007 Spring Official
// YFS 20070907 : 2007 Fall Test
// YFS 20080220 : 2008 Spring Official
// YFS 20100331 : 2010 Spring Official
// YFS 20101124 : 2010 Fall Test
// YFS 20110207 : 2011 Spring official
// YFS 20120201 : 2012 Spring Test
// YFS 20120701 : 2012 Summer Official
// YFS 20130416 : 2013 Spring test
// YFS 20130805 : 2013 Summer Official
// YFS 20140920 : 2014 Fall test - Added LOADCMND for FsAirplane
// YFS 20141026 : 2014 Fall test - Added INITPLYR
// YFS 20141101 : 2014 Fall test - Added SIMTITLE
// YFS 20141127 : 2014 Fall test - Smoke 0/1 -> Flags

// NET 20050701 : 200509 Test version
// NET 20051102 : 2005 fall official version
// NET 20060514 : 2006 summer Test version
// NET 20060805 : 2006 summer Test version 2 
//                Can accept short format, NMACHNGN command
// NET 20060828 : 2006 Summer Official
// NET 20070107 : 2007 Spring Test Ver.
// NET 20070415 : 2007 Spring Official Ver.
// NET 20070907 : 2007 Fall Test
// NET 20080220 : 2008 Spring Official
// NET 20100630 : 2010 Summer Test Ver.
// NET 20101211 : 2010 Winter Test Ver.
// NET 20110207 : 2011 Spring official


#define FS_TEXTURE_RWLIGHT "misc/rwlight.png"
#define FS_TEXTURE_GROUNDTILE "misc/groundtile.png"
#define FS_TEXTURE_PARTICLESPRITE "misc/particle_tile.png"


const double FsShortFinalDistance=1600.0;
const double FsCosineRunwayWindThreshold=0.173648; // cos(80deg)


typedef enum
{
	FSVISUAL_NOTYPE,
	FSVISUAL_STATIC,
	FSVISUAL_DYNAMIC,
	FSVISUAL_DNMEXT
} FSVISUALTYPE;

typedef enum
{
	FS_IFF0,
	FS_IFF1,
	FS_IFF2,
	FS_IFF3,
	FS_IFF_NEUTRAL=8  // Cannot be greater than 32.  Bitwise op is used in FsWorld::AddField
} FSIFF;


#define FsJapaneseLanguageCode "ja"
#define FsJapaneseLanguageCodeW L"ja"
#define FsEnglishLanguageCode "en"
#define FsEnglishLanguageCodeW L"en"


const double FsGravityConst=9.807;


const int FsMaxNumJoystick=8;
const int FsMaxNumJoyAxis=6;
const int FsMaxNumJoyTrig=32;


enum FSDIEDOF
{
	FSDIEDOF_NULL,
	FSDIEDOF_STEEPLANDING,            // Upon landing
	FSDIEDOF_LANDINGGEARNOTEXTENDED,  // Upon landing
	FSDIEDOF_BADBANKANGLE,            // Upon landing
	FSDIEDOF_BADPITCHANGLE,           // Upon landing
	FSDIEDOF_LANDEDOUTOFRUNWAY,       // Upon landing
	FSDIEDOF_OVERRUN,
	FSDIEDOF_TERRAIN,
	FSDIEDOF_COLLISION,
	FSDIEDOF_TAILSTRIKE,
	FSDIEDOF_MISSILE,
	FSDIEDOF_GUN,
	FSDIEDOF_BOMB
};

typedef enum
{
	FSWEAPON_GUN,            // The order shall not be changed.
	FSWEAPON_AIM9,           // The order shall not be changed.
	FSWEAPON_AGM65,          // The order shall not be changed.
	FSWEAPON_BOMB,           // The order shall not be changed.
	FSWEAPON_ROCKET,         // The order shall not be changed.
	FSWEAPON_FLARE,          // The order shall not be changed.
	FSWEAPON_AIM120,         // The order shall not be changed.
	FSWEAPON_BOMB250,        // The order shall not be changed.
	FSWEAPON_SMOKE,          // The order shall not be changed.
	FSWEAPON_BOMB500HD,      // The order shall not be changed.
	FSWEAPON_AIM9X,          // The order shall not be changed.
	FSWEAPON_FLAREPOD,       // The order shall not be changed.
	FSWEAPON_FUELTANK,       // The order shall not be changed.
	FSWEAPON_RESERVED13,     // The order shall not be changed.
	FSWEAPON_RESERVED14,     // The order shall not be changed.
	FSWEAPON_RESERVED15,     // The order shall not be changed.
	FSWEAPON_RESERVED16,     // The order shall not be changed.
	FSWEAPON_RESERVED17,     // The order shall not be changed.
	FSWEAPON_RESERVED18,     // The order shall not be changed.
	FSWEAPON_RESERVED19,     // The order shall not be changed.
	FSWEAPON_RESERVED20,     // The order shall not be changed.
	FSWEAPON_RESERVED21,     // The order shall not be changed.
	FSWEAPON_RESERVED22,     // The order shall not be changed.
	FSWEAPON_RESERVED23,     // The order shall not be changed.
	FSWEAPON_RESERVED24,     // The order shall not be changed.
	FSWEAPON_RESERVED25,     // The order shall not be changed.
	FSWEAPON_RESERVED26,     // The order shall not be changed.
	FSWEAPON_RESERVED27,     // The order shall not be changed.
	FSWEAPON_RESERVED28,     // The order shall not be changed.
	FSWEAPON_RESERVED29,     // The order shall not be changed.
	FSWEAPON_RESERVED30,     // The order shall not be changed.
	FSWEAPON_RESERVED31,     // The order shall not be changed.
	FSWEAPON_SMOKE0,
	FSWEAPON_SMOKE1,
	FSWEAPON_SMOKE2,
	FSWEAPON_SMOKE3,
	FSWEAPON_SMOKE4,
	FSWEAPON_SMOKE5,
	FSWEAPON_SMOKE6,
	FSWEAPON_SMOKE7,
	FSWEAPON_RESERVED40,     // The order shall not be changed.
	FSWEAPON_RESERVED41,     // The order shall not be changed.
	FSWEAPON_RESERVED42,     // The order shall not be changed.
	FSWEAPON_RESERVED43,     // The order shall not be changed.
	FSWEAPON_RESERVED44,     // The order shall not be changed.
	FSWEAPON_RESERVED45,     // The order shall not be changed.
	FSWEAPON_RESERVED46,     // The order shall not be changed.
	FSWEAPON_RESERVED47,     // The order shall not be changed.

FSWEAPON_NUMWEAPONTYPE,

FSWEAPON_NULL=127,
FSWEAPON_DEBRIS=128,

FSWEAPON_FLARE_INTERNAL=200
// When adding a new weapon type, 
//   (1) make sure new weapon type indication is added in externalconsole/fsinstreading.h (FsAmmunitionIndication), and
//   (2) add translation in FsWeapon::WeaponTypeToWeaponIndicationType
} FSWEAPONTYPE;

typedef enum
{
	FSWEAPON_CREDIT_OWNER_NOT_RECORDED=0,
	FSWEAPON_CREDIT_OWNER_PLAYER=1,
	FSWEAPON_CREDIT_OWNER_NON_PLAYER=2
} FSWEAPON_CREDIT_OWNER;

enum
{
	FSWEAPON_ALLOWGUN=1,
	FSWEAPON_ALLOWROCKET=2,
	FSWEAPON_ALLOWAAM=4,
	FSWEAPON_ALLOWAGM=8,
	FSWEAPON_ALLOWBOMB=16,


FSWEAPON_ALLOWALLWEAPON=~0
};

YsString FsAllowedWeaponTypeToStr(unsigned int allowedWeaponType);
unsigned int FsArgvToAllowedWeaponType(const YsConstArrayMask <YsString> &argv);

typedef enum
{
	FSVEHICLE_CTRL_BY_NOBODY,
	FSVEHICLE_CTRL_BY_PILOT,
	FSVEHICLE_CTRL_BY_AUTOPILOT,
	FSVEHICLE_CTRL_BY_NETWORK
} FSVEHICLECONTROLLER;

typedef enum
{
	FSTURRET_CTRL_BY_NOBODY,
	FSTURRET_CTRL_BY_PILOT,
	FSTURRET_CTRL_BY_GUNNER,
	FSTURRET_CTRL_BY_NETWORK
} FSTURRETCONTROLLER;

enum FSUSERCONTROL
{
	FSUSC_ENABLE,
	FSUSC_DISABLE,
	FSUSC_VIEWCONTROLONLY,
	FSUSC_SCRIPT, // 2018/04/14 Controlling from script

	FSUSC_DONTCARE=0x7fffffff
};

typedef enum
{
	FSSMKCIRCLE,
	FSSMKNOODLE,
	FSSMKTOWEL,
	FSSMKSOLID,
	FSSMKNULL
} FSSMOKETYPE;

typedef enum
{
	FSNOCLOUD,
	FSCLOUDFLAT,
	FSCLOUDSOLID
} FSCLOUDTYPE;



// MEMO:
// To change the environment, set environment in FsSimulation,
// and apply darker color scale to YsScenery.
enum FSENVIRONMENT
{
	FSDAYLIGHT,
	FSNIGHT
};

const int FsMaxNumSubWindow=2;

enum FSFLIGHTSTATE
{
	FSFLYING,
	FSGROUND,
	FSSTALL,
	FSDEAD,
	FSDEADSPIN,
	FSDEADFLATSPIN,
	FSGROUNDSTATIC,
	FSOVERRUN
	// For the compatibility of the flight record,
	// Always add new state at the bottom.

	// When adding a new state, also update FsAirplaneProperty::StringToState and StateToString.
};

const char *FsFlightStateToStr(FSFLIGHTSTATE sta);

typedef enum
{
	FSGNDALIVE,
	FSGNDDEAD
} FSGNDSTATE;

enum FSEXISTENCETYPE
{
	FSEX_AIRPLANE,
	FSEX_GROUND,

FSEX_NULL=~0
};

enum FSAIRCRAFTCLASS
{
	FSCL_AIRPLANE,
	FSCL_HELICOPTER
};

enum FSAIRPLANECATEGORY
{
	FSAC_UNKNOWN,
	FSAC_NORMAL,
	FSAC_UTILITY,
	FSAC_AEROBATIC,
	FSAC_FIGHTER,
	FSAC_ATTACKER,
	FSAC_TRAINER,
	FSAC_HEAVYBOMBER,
	FSAC_WW2FIGHTER,
	FSAC_WW2BOMBER,
	FSAC_WW2ATTACKER,
	FSAC_WW2DIVEBOMBER
};

enum FSGROUNDTYPE
{
	FSSTATIC,
	FSVEHICLE,
	FSTANK,
	FSSHIP,
	FSNAVYSHIP
};

#define FSGOAL_SURVIVE          1
#define FSGOAL_LAND             2
#define FSGOAL_DEFENDPRMGND     4
#define FSGOAL_DEFENDGND        8
#define FSGOAL_DEFENDAIR       16
#define FSGOAL_DESTROYPRMGND   32
#define FSGOAL_DESTROYGND      64
#define FSGOAL_DESTROYAIR     128
#define FSGOAL_DESTROYALLAIR  256
#define FSGOAL_MUSTLAND      1024
// When a new goal flag is added, make sure it is saved in FsMissionGoal::WriteFile, and 
// loaded in FsMissionGoal::SendCommand
// Modify FsSimulation::ShowMissionGoal / ShowResultOfMissionGoal, too.


#define FSLIGHT_GND_HEADLIGHT   1
#define FSLIGHT_GND_STROBE      2
#define FSLIGHT_GND_LEFTTURN    4
#define FSLIGHT_GND_RIGHTTURN   8


enum FSNETTYPE
{
	FSNET_LOCAL,
	FSNET_REMOTE
};


enum FSLANDINGPRACTICEMODE
{
	FSLDGPRC_STRAIGHTIN_WINDCALM_GOODVISIBILITY,
	FSLDGPRC_DOGLEG_WINDCALM_GOODVISIBILITY,
	FSLDGPRC_BASELEG_WINDCALM_GOODVISIBILITY,
	FSLDGPRC_STRAIGHTIN_WINDCALM_LOWCLOUD,
	FSLDGPRC_DOGLEG_WINDCALM_LOWCLOUD,
	FSLDGPRC_STRAIGHTIN_WINDCALM_LOWVISIBILITY,
	FSLDGPRC_DOGLEG_WINDCALM_LOWVISIBILITY
};

enum FSTRAFFICPATTERNLEG
{
	FSLEG_NOT_IN_PATTERN,
	FSLEG_UPWIND,
	FSLEG_DOWNWIND,
	FSLEG_CROSSWIND,
	FSLEG_BASE,
	FSLEG_DOG,
	FSLEG_FINAL,
	FSLEG_LANDED,
	FSLEG_45DEG
};


enum FSFORMATIONTYPE
{
	FSFOM_NONE,
	FSFOM_DIAMOND,
	FSFOM_DELTA
};

typedef enum
{
	FSAUTOPILOT_NONE,
	FSAUTOPILOT_GOTO,
	FSAUTOPILOT_DOGFIGHT,
	FSAUTOPILOT_GNDATTACK,
	FSAUTOPILOT_FORMATION,
	FSAUTOPILOT_AIRSHOW,
	FSAUTOPILOT_LANDING,
	FSAUTOPILOT_TAKEOFF,
	FSAUTOPILOT_HOVER,
	FSAUTOPILOT_VLANDING,
	FSAUTOPILOT_TAXIING,
	FSAUTOPILOT_AIRWAY,
	FSAUTOPILOT_DEFENDER,
	FSAUTOPILOT_REFUEL_AND_TAKEOFF,

	// Unsavable.
	FSAUTOPILOT_TESTPILOT,
	FSAUTOPILOT_SPEEDONLY,

} FSAUTOPILOTTYPE;

enum FSAIRCRAFTTROUBLE
{
	FSAIRTROUBLE_LOSSGEARLOCK,
	FSAIRTROUBLE_AUTOPILOT,
	FSAIRTROUBLE_FLAPSTUCK,
	FSAIRTROUBLE_RADAR,
	FSAIRTROUBLE_VOR,
	FSAIRTROUBLE_ADF,
	FSAIRTROUBLE_AIRSPEED,
	FSAIRTROUBLE_ALTIMETER,
	FSAIRTROUBLE_VSI,
	FSAIRTROUBLE_ATTITUDE,
	FSAIRTROUBLE_HUDFLICKER,
	FSAIRTROUBLE_PARTIALPOWERLOSS_70,
	FSAIRTROUBLE_PARTIALPOWERLOSS_30,
	FSAIRTROUBLE_TOTALPOWERLOSS
};

const YSHASHKEY FsNullSearchKey=YSNULLHASHKEY;

enum
{
	FS_RGNID_NULL=0,
	FS_RGNID_RUNWAY=1,
	FS_RGNID_TAXIWAY=2,
	FS_RGNID_AIRPORT_AREA=4,
	FS_RGNID_ENEMY_TANK_GENERATOR=6,
	FS_RGNID_FRIENDLY_TANK_GENERATOR=7,
	FS_RGNID_VIEWPOINT=10,
	FS_RGNID_FIX=11
};

enum
{
	FS_MPATHID_NULL=0,
	FS_MPATHID_TAXI_TO_RAMP=10,
	FS_MPATHID_TAXI_FOR_TAKEOFF=11,
	FS_MPATHID_LANDING_RUNWAY=12
};

enum FSFLIGHTRULE
{
	FS_VFR,
	FS_IFR
};

enum
{
	FSEVENT_NULLEVENT,
	FSEVENT_TEXTMESSAGE,
	FSEVENT_PLAYEROBJCHANGE,
	FSEVENT_AIRCOMMAND,
	FSEVENT_SETWEAPONCONFIG,
	FSEVENT_ENVIRONMENTCHANGE,
	FSEVENT_VISIBILITYCHANGE,
	FSEVENT_WINDCHANGE,
	FSEVENT_CLOUDLAYERCHANGE,
	FSEVENT_GNDCOMMAND,
};

enum
{
	FSEVENTFLAG_FIXED=1,
	FSEVENTFLAG_UNSORTED=0x8000
};

enum FSDEMOMODETYPE
{
	FSDEMO_DOGFIGHT,
	FSDEMO_LANDING,
	FSDEMO_ACROBAT,
	FSDEMO_CONCORDEFLYBY
};


#define FSAIRFLAG_INDEPENDENT     1  // Neglect command by radio
#define FSAIRFLAG_RESERVE1        2
#define FSAIRFLAG_RESERVE2        4
#define FSAIRFLAG_RESERVE3        8
#define FSAIRFLAG_RESERVE4       16
#define FSAIRFLAG_DONTATTACKIFGROUNDSTATIC     32
#define FSAIRFLAG_AUTOGENERATED  64  // Automatically Generated for Endurance Mode & Intercept Mission
#define FSAIRFLAG_DONTUSE1      128  // This flag means "can be user object" for ground.  Don't use.  Currently has no effect for an aircraft.

#define FSGNDFLAG_PRIMARYLANDINGTARGET        1
#define FSGNDFLAG_PRIMARYCARRIER              2
#define FSGNDFLAG_MOVETOWARDGNDTARGET         4
#define FSGNDFLAG_TARGETANY                   8  // Attack any enemy
#define FSGNDFLAG_DONTMOVE                   16  // If ON, FsWorld::AddField will send "MAXSPEED 0kt","MAXROTAT 0deg"
#define FSGNDFLAG_DONTREVIVE                 32  // If ON, FsWorld::ReviveGround will not revive the object. 2005/09/20
#define FSGNDFLAG_AUTOGENERATED              64  // Automatically Generated Tank (for Close Air Support Mission etc.)
#define FSGNDFLAG_CANBEUSEROBJECT           128
#define FSGNDFLAG_CANBEUSEDINANTIAIRMISSION 256
#define FSGNDFLAG_DONTMOVEUNLESSUSERISCONTROLLING 512 // Stay static unless the user is controlling

#define FSGNDFLAG_CASMISSIONTANK (FSGNDFLAG_MOVETOWARDGNDTARGET|FSGNDFLAG_TARGETANY|FSGNDFLAG_DONTREVIVE|FSGNDFLAG_AUTOGENERATED)



// Weather >>

// 0.5 mile to 12.5 miles
const double FS_FOG_VISIBILITY_MAX=20000.0;
const double FS_FOG_VISIBILITY_MIN=800.0;

enum
{
	FSCLOUDLAYER_NONE,
	FSCLOUDLAYER_OVERCAST
};

// Weather <<



// Network >>

#define FS_DEFAULT_NETWORK_PORT 7915
#define FS_MAX_NUM_USER 64

#define FS_NETTIMEOUT 50
#define FS_NETEMERGENCYTIMEOUT 200
#define FS_NETPRIORITYPACKETWAIT 1500
#define FS_NETPRIORITYPACKETTIMEOUT 50
#define FS_NETPRIORITYPACKETSLEEP 10
#define FS_NETPRIORITYPACKETRETRY (FS_NETPRIORITYPACKETWAIT/(FS_NETPRIORITYPACKETSLEEP+FS_NETPRIORITYPACKETTIMEOUT))

enum FSUSERSTATE
{
	FSUSERSTATE_NOTCONNECTED,
	FSUSERSTATE_PENDING,
	FSUSERSTATE_LOGGEDON
};

// Network <<

/* } */
#endif
