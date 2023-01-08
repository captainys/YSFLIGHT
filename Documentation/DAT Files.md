# Introduction

The DAT file (.dat) tells YSFlight how an aircraft or ground object is supposed to behave. This page details all of the different DAT Variables, their required inputs, and in general what they are used for.


# Line And In-Line Comments in DAT Files

All DAT Files allow lines to be commented out if the row begins with "REM". Additionally comments can be added in-line if preceded with "#".

# Aircraft DAT Variables

All of the aircraft DAT Variables are defined in sub-sections below in no particular order. In addition to the constraints and information in the sections below, also see the [table below](#Aircraft-DAT-Variable-Summary) for specific requirements in terms of the units and inputs.

<br>







## Hardpoint Definitions

> HRDPOINT -0.893m -1.141m 1.169m B500\*2 AGM65\*2 AIM120\*2 AIM9\*2 AIM9X\*2 $INTERNAL<br> 
> HRDPOINT 0m -1.465m 0.675m FUEL&1200

There are three parts to the Hardpoint definition lines: the location, allowed weapons, and the Internal/External status of the hardpoint.

### Hardpoint location

The position of the hardpoint defines where the weapon SRF model origin (0,0,0) will render.

See the [Coordinate System Definition](#-YSFlight-Coordinate-System) for more about the order of inputs.

### Permitted Weapons

Only one weapon type is allowed on a hardpoint at a time. For all weapons besides external fuel tanks, multiple weapons of the same type may be loaded on a hardpoint. For external fuel tanks, the quantity of fuel (in kg) is defined as shown in the second example, but if not defined, YSFlight will load 800kg as a default. For all other weapons the maximum quantity of the weapon that can be loaded is defined with an asterix * and then the number. If no quantity is defined, YSFlight will default to only allowing one weapon on the hardpoint.

### Internal Weapons

Some hardpoints can be defined as INTERNAL which means that for all bombs on that hardpoint, the bomb bay doors need to be open before the bomb can be dropped. If the weapon bay is closed when the player tries to drop a bomb, the weapon bay will be automatically opened and then the player must command another bomb drop.


<br>


# Turret Definitions

Turrets and all of their functionality are defined within the DAT file. Each turret will need to have a complete set of DAT variables defined. With multiple turrets this can get complicated, but each turret DAT variable also will indicate which turret it is controlling. These variables are different from typical DAT file parameters in that the same names are used for each turret defined, so a turret index is used to tie specific performance to specific turrets.

## NMTURRET

> NMTURRET 6

Indicates the number of turrets defined within the DAT File.

## TURRETPO

> TURRETPO 0 0m 1.82m 10.91m 0deg 0deg 0deg <br> TURRETPO 1 0m -1.67m 10.46m 0deg 0deg 0deg

The position and neutral orientation for turret #0 and #1. Note that the orientation is relative to the turret's visual model in the DNM file. So if the turret's object is parented to an empty and only the empty is rotated 180 degrees, then the neutral position of this turret would face backwards. The turret will return to the neutral orientation when there are no targets within the targeting range and the turret is controlled by the computer.

See the [Coordinate System Definition](#-YSFlight-Coordinate-System) for more about the order of inputs. 

## TURRETPT

> TURRETPT 0 0deg 90deg 0deg  <br> TURRETPT 1 -90deg 0deg 0deg 

Defines the minimum, maximum and neutral turret pitch angles, with positive values above the horizon, and negative values below the horizon.

## TURRETHD

> TURRETHD 0 -180deg 180deg 0deg  <br> TURRETHD 1 -180deg 180deg 0deg 

Defines the minimum, maximum and neutral heading angles, with positive values to the right and negative values to the left.

## TURRETAM

> TURRETAM 0 5000 <br> TURRETAM 4 50  

Defines how much ammunition (bullets or missiles) that this specific turret has available to use.

## TURRETIV

> TURRETIV 0 0.2sec <br> TURRETIV 4 24sec 

The firing interval of the turret's weapon. 

## TURRETNM

> TURRETNM 3 TURRET4 <br> TURRETNP 2 TURRET3GUN  

The name of the DNM object that should move in both pitch and heading with this turret.

**NOTE:** This variable will control a DNM object in both pitch and heading. If you wish to separate out the visual effect see [TURRETNP](#TURRETNP) and [TURRETNH](#TURRETNH).

## TURRETAR

> TURRETAR 0 <br> TURRETAR 4

Including this variable will allow the turret to target enemy IFF aircraft.

## TURRETGD

> TURRETGD 0 <br> TURRETGD 4

Including this variable will allow the turret to target enemy IFF ground objects.

## TURRETCT

> TURRETCT 0 GUNNER <br> TURRETCT 4 PILOT

If "GUNNER", then the computer will control the turret with no pilot/player control. If set to "PILOT", then the aircraft's pilot will be able to manually control the heading, pitch and firing of the turret if their keybinding is properly setup to do so.

## TURRETRG

> TURRETRG 0 1500m <br> TURRETRG 1 5000m

Controls how far the computer can shoot at a target. Must be between 0 and 50000m.

## TURRETNH

> TURRETNH 1 TURRET2BASE <br> TURRETNH 0 TURRET1BASE 




<br><br><br><br><br><br>






# Aircraft DAT Variable Summary

| Variable        | Required? | Input                                | Unit Type |
|-----------------|-----------|--------------------------------------|-----------|
| IDENTIFY        | Yes       | String in quotation marks            | N/A       |
| SUBSTNAM        | No        | String in quotation marks            | N/A       |
| CATEGORY        | Yes       | Pre-defined String                   | N/A       |
| AFTBURNR        | Yes       | TRUE/FALSE                           | Boolean   |
| THRAFTBN        | Yes*      | Number                               | Weight    |
| THRMILIT        | Yes*      | Number                               | Weight    |
| PROPELLR        | Yes**     | Number                               | Power     |
| PROPVMIN        | Yes**     | Number                               | Speed     |
|        PROPEFCY | No**      | Number                               | N/A       |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
|                 |           |                                      |           |
| *               |           | For Jet Engine Aircraft              |           |
| **              |           | For simple propeller engine aircraft |           |



# YSFlight Coordinate System

For dimensions, the following order is used for coordinates:

- Left / Right, Positive Right
- Up / Down, Positive Up
- Fore / Aft, Positive Forward


For angles, the following order is used:

- Rotate about the vertical axis, Positive ==TBD==
- Rotate about the hotizontal axis, Positive Down
- Rotate about the logitudinal axis, Positive ==TBD==


For Vectors, the following order is used. 
- Left / Right, Positive Right
- Up / Down, Positive Up
- Fore / Aft, Positive Forward

Note: Most vectors defined in DAT files are [unit vectors](https://en.wikipedia.org/wiki/Unit_vector), but some are used to define velocity.