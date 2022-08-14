#ifndef FSEXTERNALCONSOLECOMMAND_IS_INCLUDED
#define FSEXTERNALCONSOLECOMMAND_IS_INCLUDED
/* { */


enum FSEXTERNALCONSOLECOMMAND
{
	FSNETCMD_LOADFIELD                        =0x00004,

	FSEXCCMD_CAPS_INSTPANEL                   =0x10000,   // Server(Player) <-  Client(Console)
	FSEXCCMD_CAPS_ELEVATOR                    =0x10001,   // Server(Player) <-  Client(Console)
	FSEXCCMD_CAPS_AILERON                     =0x10002,   // Server(Player) <-  Client(Console)
	FSEXCCMD_CAPS_RUDDER                      =0x10003,   // Server(Player) <-  Client(Console)

	FSEXCCMD_INSTRUMENT_INDICATION            =0x10100,   // Server(Player)  -> Client(Console)
	FSEXCCMD_NAV_INDICATION                   =0x10101,   // Server(Player)  -> Client(Console)
	FSEXCCMD_ADF_INDICATION                   =0x10102,   // Server(Player)  -> Client(Console)
	FSEXCCMD_RADAR_INDICATION                 =0x10103,   // Server(Player)  -> Client(Console)
	FSEXCCMD_AMMUNITION_INDICATION            =0x10104,   // Server(Player)  -> Client(Console)
	FSEXCCMD_PLAYER_POSITION                  =0x10105,   // Server(Player)  -> Client(Console)
	FSEXCCMD_PLAYER_COCKPIT_POSITION          =0x10106,   // Server(Player)  -> Client(Console)

	FSEXCCMD_MALFUNCTION_ATTITUDE_INDICATOR   =0x10200,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_REPAIR_ATTITUDE_INDICATOR        =0x10201,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_MALFUNCTION_RAMAIR_FROZEN        =0x10202,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_REPAIR_RAMAIR_FROZEN             =0x10203,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_MALFUNCTION_AIRSPEED_INDICATOR   =0x10204,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_REPAIR_AIRSPEED_INDICATOR        =0x10205,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_MALFUNCTION_VSI                  =0x10206,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_REPAIR_VSI                       =0x10207,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_MALFUNCTION_ENGINE_FAILURE       =0x10208,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_REPAIR_ENGINE_FAILURE            =0x10209,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_MALFUNCTION_LOOSE_GEAR_LOCK      =0x1020a,   // Server(Player) <-  Client(Console) (*1)
	FSEXCCMD_REPAIR_LOOSE_GEAR_LOCK           =0x1020b,   // Server(Player) <-  Client(Console) (*1)

	FSEXCCMD_ENVIRON_SETWIND                  =0x10300,   // Server(Player) <-> Client(Console) (*1)
	FSEXCCMD_ENVIRON_ADDCLOUD                 =0x10301,   // Server(Player) <-> Client(Console) (*1)
	FSEXCCMD_ENVIRON_CLEARCLOUD               =0x10302,   // Server(Player) <-> Client(Console) (*1)
	FSEXCCMD_ENVIRON_SETVISIBILITY            =0x10303,   // Server(Player) <-> Client(Console) (*1)
};

// (*1) Server(Player) ignores if a mission goal(s) is specified or in the network mode.


/* } */
#endif
