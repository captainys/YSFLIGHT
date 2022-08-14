import subprocess
import os
import sys

import common
import file_location


def RunTestForBinary(binFName):
	THISDIR=os.path.dirname(os.path.realpath(__file__))

	# 1. Turn off auto demo and close
	# 2. Try all Simulation menu items
	# 3. Turn on auto demo and close
	# 4. Try all Simulation menu items
	# 5. Reset option
	print("BINARY:"+binFName)
	common.RunOneSession("Auto Demo On/Off",binFName,"option_demo_off.txt")
	common.RunOneSession("Auto Demo On/Off",binFName,"menu_sim.txt")
	common.RunOneSession("Auto Demo On/Off",binFName,"option_demo_on.txt")
	common.RunOneSession("Auto Demo On/Off",binFName,"menu_sim.txt")
	common.RunOneSession("Auto Demo On/Off",binFName,"option_reset.txt")

	# Default aircrafts are better covered by batch_test_allaircraft.py
	# # Flip through all default aircrafts
	# common.RunOneSession("Flip Through Aircrafts",binFName,"aircraft.txt")

	# Default field are better covered by batch_test_allfield.py
	# # Flip through all default fields
	# common.RunOneSession("Flip Through Fields",binFName,"field.txt")

	# Normal Window, Maximize Window, Full Screen
	common.RunOneSession("Landings",binFName,"screen_mode.txt")

	# Try landings
	common.RunOneSession("Landings",binFName,"landing.txt")

	# Turn on/off particle-based rendering
	common.RunOneSession("Particle Rendering",binFName,"smoke_particle.txt")

	# Try misc config items
	common.RunOneSession("Misc Config",binFName,"misc_config.txt")

	# Try misc config items
	common.RunOneSession("Weapon Record",binFName,"weapon_record.txt")

	# Try some of the acro demo
	common.RunOneSession("Acro Demo",binFName,"acro_demo.txt")

	# Try some of the acro demo
	common.RunOneSession("Language",binFName,"language.txt")



def main():
	common.MakeDataDir()

	for binFName in file_location.GetTestBinary():
		RunTestForBinary(binFName)



if __name__=="__main__":
	main()
