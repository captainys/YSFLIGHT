import os
import array
import shutil
import common
import file_location



def main():
	byteList=bytearray([0xE3,0x83,0x86,0x20,0xE3,0x82,0xB9,0x20,0xE3,0x83,0x88,0x20,0x59,0x53,0x20,0x46,0x4C,0x49,0x47,0x48,0x54])
	yfsByteList=bytearray([0xE6,0x97,0xA5,0x20,0xE6,0x9C,0xAC,0x20,0xE8,0xAA,0x9E,0x2E,0x79,0x66,0x73])

	fName_utf8=bytes(byteList)
	fName=fName_utf8.decode('utf8')
	yfsFName=bytes(yfsByteList).decode('utf8')

	print(fName_utf8)
	print(fName)
	print(yfsFName)



	testDirName=os.path.join('~',fName)
	testDirName=os.path.expanduser(testDirName)
	print(testDirName)



	if os.path.isdir(testDirName):
		shutil.rmtree(testDirName)
	os.makedirs(testDirName)



	common.MakeDataDir()

	for binFName in file_location.GetTestBinary():
		# Normal Window, Maximize Window, Full Screen
		common.RunOneSession("Save to Unicode File Name",binFName,"unicode_file_name.txt",["-userdir",testDirName,"-configdir",testDirName])

		# Try misc config items
		common.RunOneSession("Write Config to Unicode Dir",binFName,"config_reset.txt",["-userdir",testDirName,"-configdir",testDirName])

		# Normal Window, Maximize Window, Full Screen
		common.RunOneSession("Write Option to Unicode Dir",binFName,"option_reset.txt",["-userdir",testDirName,"-configdir",testDirName])



	flightCfg=os.path.join(testDirName,"flight.cfg")
	optionCfg=os.path.join(testDirName,"option.cfg")
	yfs=os.path.join(testDirName,yfsFName)

	if not os.path.isfile(flightCfg) or not os.path.isfile(optionCfg):
		print("Configufation/Option files are not created in the config dir.")
		raise

	if not os.path.isfile(yfs):
		print("YFS file not created.")
		raise


if __name__=="__main__":
	main()
