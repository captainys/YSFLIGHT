# YS Flight Simulator

by CaptainYS
http://www.ysflight.com

## Introduction
This is the source code of YS Flight Simulator.  I wanted to keep it to myself a bit longer, but in reality my hands are full with too many projects now and haven't been able to work on YSFLIGHT for too long.  I admit that it would be the best for long-time YSFLIGHT fans to have access to the source code.  Please feel free to fork it and make your own version.

Microsoft Flight Simulator is a great piece of work, but I also believe it is nice to have a flight simulator that everyone can casually play during the lunch break.  That has been the concept of YSFLIGHT.  But, I put many elements that I learned from my flight training in YSFLIGHT.  I do use YSFLIGHT for practicing IFR approaches in a Cessna for myself (of course I'm not logging time for it though.)  I hope YSFLIGHT serves you well for the future!

By the way, although I haven't been able to add features to YSFLIGHT, I still have plans.  Eventually, I'll get back to YSFLIGHT development, and finish those features.



## Compile Instruction
Prerequisite is a compiler and cmake.

YSFLIGHT requires my public libraries to build.  Follow the steps below:

```
(In your working directory)
git clone https://github.com/captainys/public.git
git clone https://github.com/captainys/YSFLIGHT.git
cd YSFLIGHT
mkdir build
cd build
cmake ../src
cmake --build . --config Release --parallel
```

The first line of code was written in 1998.  I re-used lines from my C library that I wrote between 1998 and 1996.  I also had a distrust on C++ Standard Template Library then.  So, you see many C-like code in C++.  Also my public library has many functionality that can be covered by C++ Standard Template Library.  I'm gradually making my public library inter-operable with Standard Template Library, but not fully done, and that's not my priority now.

Also bigger mess came from the experimental support for iOS.  I had to re-write my GUI library for fully concurrent mode.

By the way, I ditched iOS support because I believe it is wrong to be forced to pay money to run my own code.  I was able to test-run my iOS app on my iPhone from XCode, but it is valid for only several days.  When I tried to show my code to my class, it was expired and was useless.  AppStore is the worst idea, probably one of the biggest threat, from the freedom of programming point of view.  So, what needs to be done?  There should be a choice of application stores.  If Apple, Google, and Microsoft want to kill all free software, the user should be allowed to choose another application store that is more open.  Sure, there is a danger of malicious software.  Then, have two computers, one for fun, keep all sensitive information out, and the other for your banking, shopping, and work-related communications.

I included iOS project in this repository, but I think it doesn't compile any more.

I wanted to clean the source code more before releasing, but it is what it is.
