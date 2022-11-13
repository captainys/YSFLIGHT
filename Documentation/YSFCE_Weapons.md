# YSFCE Weapon Performance

This document contains specifications for YSFlight Comunity Edition Weapon performance.


# Types of Weapons

| Weapon Type | DAT Name | Description | 
| -- | -- | -- |
| Bomb | B250 | 250lb Bomb that explodes on impact |
| Bomb | B500 | 500lb Bomb that explodes on impact | 
| Bomb | B500HD | 500lb Bomb that has a $C_D A$ of 0.75 | 
| A2A Missile | AIM9 | Short-range missile that does not retarget after launch |
| A2A Missile | AIM9X | Short-range missile that retargets after launch | 
| A2A Missile | AIM120 | Medium-range missile that retargets after launch if the launch platform's nose is pointed at target. | 
| A2G Missile | AGM65 | Short-range missile that aims towards target selected before launch. |
| Misc | FUEL | External fuel tank (Default 800kg of fuel) | 
| Misc | FLR | Flarepod with extra flares. Depletes this stock before flares on the aircraft. |
| Misc | IFLR | Internal Flares | 
| A2G Rocket | RKT | Short-range rocket | 
| Gun | MACHNGUN | Gun that fires at DAT-specified rate of speed and damage. | 





# Weapon Weights

| Weapon | Weight (kg) | 
| -- | -- |
| B250 | 120  |
| B500 |  250 |
| B500HD | 250  |
| AIM9 |  90 |
| AIM9X | 90  |
| AIM120 | 150  |
| RKT |  10  |
| FUEL (Empty) |  150  |
| AGM65 | 300  |
| FLR (Empty) |  150 |




# Missile Properties

| Weapon | Unguided Time (s) | Max Speed (m/s) | Explosive Radius (m) | Range (m) |
| -- | -- | -- | -- | -- |
| AIM9 |    |  | 18 |  |
| AIM9X | 0.5 |  | 23 |  |
| AIM120 | 3 |  | 25 |  |
| AGM65 | 1.6 |  |  |  |


# Weapon Falling Properties





# Explosion Damage vs Distance From Epicenter



$$dmg = 1.0 + destructivePower \times \frac{range - \sqrt{distance}}{range}$$

Where:
- destructivePower = damage that weapon can cause
- distance = distance between weapon and object centers
- range = fusing range of the weapon

## B500HD

High drag bombs use the following equation to simulate the flight profile of a high-drag bomb.

In the drag equation, the drag coefficient and the area are combined.

$$C_D S = 0.8$$

<br>

$$Drag = 0.5 \times C_D S \times V^2 \times \rho$$

and the acceleration is as follows:

$$a = \frac{Drag}{m}$$

where:
- $m$ = 226.8 kg