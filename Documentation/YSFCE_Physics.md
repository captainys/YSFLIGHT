# YSFlight Community Edition Physics Documentation

## Introduction

This document contains the physics model of the game and describes how different parameters are calculated. YSFCE runs on SI units, but will accept DAT file inputs in Imperial Units and convert them into SI units.


# YSFlight Atmospheric Model

The YSFlight atmospheric model is based on the 1976 U.S. Standard Atmosphere model from https://www.digitaldutch.com/atmoscalc/table.htm. The table below shows the various properties of the YSFCE atmosphere. For all parameters at intermediate altitudes, one-dimensional linear interpolation is used with altitudes beyond the table using the closest altitude's values.

| Altitude (m) | Density (kg/m3)    | Temperature (K) | Pressure (Pa) | Speed of sound (m/s) | Viscosity (Pa/s)  |
|----------|------------|-------------|----------|----------------|-------------|
| -2000 | 1.47808    | 301.15 | 127774  | 347.886 | 0.0000187630 |
| -1000 | 1.347      | 294.65 | 113929  | 344.111 | 0.0000184434 |
| 0     | 1.225      | 288.15 | 101325  | 340.294 | 0.0000181206 |
| 1000  | 1.11164    | 281.65 | 89874.6 | 336.434 | 0.0000177943 |
| 2000  | 1.00649    | 275.15 | 79495.2 | 332.529 | 0.0000174645 |
| 3000  | 0.909122   | 268.65 | 70108.5 | 328.578 | 0.0000171311 |
| 4000  | 0.819129   | 262.15 | 61640.2 | 324.579 | 0.0000167940 |
| 5000  | 0.736116   | 255.65 | 54019.9 | 320.529 | 0.0000164531 |
| 6000  | 0.659697   | 249.15 | 47181   | 316.428 | 0.0000161084 |
| 7000  | 0.589501   | 242.65 | 41060.7 | 312.274 | 0.0000157596 |
| 8000  | 0.525168   | 236.15 | 35599.8 | 308.063 | 0.0000154068 |
| 9000  | 0.466348   | 229.65 | 30742.5 | 303.793 | 0.0000150498 |
| 10000 | 0.412707   | 223.15 | 26436.3 | 299.463 | 0.0000146884 |
| 10000 | 0.412707   | 223.15 | 26436.3 | 299.463 | 0.0000146884 |
| 12000 | 0.310828   | 216.65 | 19330.4 | 295.07  | 0.0000143226 |
| 14000 | 0.226753   | 216.65 | 14101.8 | 295.07  | 0.0000143226 |
| 16000 | 0.16542    | 216.65 | 10287.5 | 295.07  | 0.0000143226 |
| 18000 | 0.120676   | 216.65 | 7504.84 | 295.07  | 0.0000143226 |
| 20000 | 0.0880349  | 216.65 | 5474.89 | 295.07  | 0.0000143226 |
| 20000 | 0.0880349  | 216.65 | 5474.89 | 295.07  | 0.0000143226 |
| 22000 | 0.0637273  | 218.65 | 3999.79 | 296.428 | 0.0000144357 |
| 24000 | 0.0462674  | 220.65 | 2930.49 | 297.781 | 0.0000145483 |
| 26000 | 0.0336882  | 222.65 | 2153.09 | 299.128 | 0.0000146604 |
| 28000 | 0.0245988  | 224.65 | 1586.29 | 300.468 | 0.0000147722 |
| 30000 | 0.0180119  | 226.65 | 1171.87 | 301.803 | 0.0000148835 |
| 32000 | 0.013225   | 228.65 | 868.019 | 303.131 | 0.0000149945 |
| 34000 | 0.00960889 | 234.25 | 646.122 | 306.821 | 0.0000153029 |
| 36000 | 0.00703441 | 239.85 | 484.317 | 310.467 | 0.0000156082 |



# Jet Engine Performance

Jet engine performance is a function of DAT variables and altitude. 

| DAT Variable | Data | Definition | Allowable Units |
| ------------ | ---| ---------- | --------------- |
| AFTBURNR | TRUE/FALSE | Indicates if the aircraft has an afterburning engine. Automatically set to FALSE if propeller DAT Variables are in the DAT file. | N/A |
| THRAFTBN | float | The maximum afterburning thrust output of the engine at sea level | t / kg / lb |
| THRMILIT | float | The maximum non-afterburning thrust output of the engine at sea level | t / kg / lb |
| FUELABRN | float | Amount of fuel burned per second at maximum afterburning thrust setting at sea level | lb / kg |
| FUELMILI | float | Amount of fuel burned per second at maximum non-afterburning thrust setting at sea level | lb / kg |


## Jet Engine Thrust

The thrust of the jet engine is calculated based on the DAT file, throttle setting and altitude (in the form of the jet engine thrust efficiency). This equation does not account for thrust reversers or thrust vectoring.

For non-afterburning operations:

$$T = \eta \, \times \, Thr \, \times \, THRMILIT$$

For afterburning operations: 

$$T = \eta \, \times \, THRMILIT \, \times \, (THRAFTBN \, - \, THRMILIT) \, \times \, Thr$$

Where:
- $\eta$ is the Jet Engine Thrust Efficiency
- $Thr$ is the throttle


## Jet Engine Thrust Efficiency

The Jet Engine Thrust Effiency $\eta$ is a function of altitude. For all altitudes between 0 and 32000m, $\eta$ is a one-dimensional linear interpolation of the table. For all values outside the table, the value of the closest is used.

| Altitude (m) | $\eta$ |
| ------------ | ------ | 
| -2000 | 1 | 
|4000 | 1|
| 12000 | 0.6 | 
| 16000 | 0.3 | 
| 20000 | 0.084991 | 
| 31999 | 0.084991 | 
| 32000 | 0 | 


## Jet Engine Fuel Consumption

The DAT file defines the sea-level fuel consumption at maximum afterburning and non-afterburning throttle settings. 

### YSFlight Fuel Consumption

For Afterburning operations:

$$\dot{F} = dt \, \times \, FUELABRN$$

For non-Afterburning operations:

$$\dot{F} = dt \, \times \, FUELMILI \, \times \, Thr$$

Where:
- $\dot{F}$ is the change in fuel weight
- $dt$ is the change in time
- $Thr$ is the throttle setting

### Future YSFCE Fuel Consumption

For Afterburning Operations:

$$\dot{F} = dt \, \times \, FUELABRN \, \times \, \left( \frac{T}{THRAFTBN} \right)$$

For Non-Afterburning Operations:

$$\dot{F} = dt \, \times \, FUELMILI \, \times \, Thr \times \, \left( \frac{T}{THRAFTBN} \right)$$

Where:
- $\dot{F}$ is the change in fuel weight
- $dt$ is the change in time
- $Thr$ is the throttle setting
- $T$ is the thrust being generated by the engine




# Mach Number

The mach number of the aircraft is calculated by first determining the speed of sound at the aircraft's altitude and then dividing the aircraft's speed by that value:

$$Mach \, = \, \frac{V}{a}$$

Where:
- $V$ is the aircraft's speed
- $a$ is the speed of sound.