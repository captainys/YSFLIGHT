#ifndef FS_IS_INCLUDED
#define FS_IS_INCLUDED

// Test

/* { */


// #define YSFS_TESTVERSION
// #define FREEEDITION  // If defined, 4 airplane restriction will apply.



#include <ysscenery.h>

#include "fsdef.h"

#include "fsbase.h"
#include "fsutil.h"
#include "fsweather.h"
#include "fscloud.h"
#include "fsrecord.h"
#include "fsexplosion.h"
#include "fsgroundsky.h"
#include "fsproperty.h"
#include "fsairplaneproperty.h"
#include "fsweapon.h"
#include "fsgroundproperty.h"
#include "fsnetwork.h"
#include "fsexistence.h"
#include "fsfield.h"
#include "fsstdout.h"
#include "fshud.h"
#include "fscontrol.h"
#include "fssimulation.h"
#include "fsworld.h"
#include "fsairsound.h"
#include "fsautopilot.h"
#include "fsdialog.h"




// Menu Callbacks

void FlightDisableGroundObject(void);
void FlightSetWind(void);
void FlightCreateAirCombat(void);
void FlightChooseAirplane(void);
void FlightChooseStartPosition(void);
void FlightAddComputerAirplane(void);
void FlightLaunchGroundToAirMission(FsGroundToAirDefenseMissionInfo &info);



void NetworkHostFlight(void);
void NetworkConnectToServer(void);
void NetworkOption(void);

void FsCrackDownMemoryLeak(void);

void FsHelpGeneralHelpEnglish(void);
void FsHelpControlEnglish(void);
void FsHelpGeneralHelpJapanese(void);
void FsHelpControlJapanese(void);
void FsHelpVersionInfo(void);

/* } */
#endif

