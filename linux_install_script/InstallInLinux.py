#!/usr/bin/python

import os
import sys
import shutil
import subprocess
import stat


def WaitForEnter():
	try:
		input("Press ENTER>>")
	except:
		print(".")


def FileCheck(fn):
	if not os.path.isfile(fn):
		print(fn+" not found")
		WaitForEnter()



def MakeDesktopIcon(fn,label,exe):
#	try:
		ful=os.path.expanduser(os.path.join("~","Desktop",fn))
		fp=open(ful,"w")
		fp.write("[Desktop Entry]\n")
		fp.write("Name="+label+"\n")
		fp.write("Exec="+exe+"\n")
		fp.write("Type=Application\n")
		fp.write("StartupNotify=true\n")
		fp.write("Comment=YS FLIGHT SIMULATOR\n")
		fp.write("Path="+os.path.dirname(exe)+"\n")
		iconfile=os.path.join(os.path.dirname(exe),"misc","desktopicon.png")
		if not os.path.isfile(iconfile):
			print("Warning!  Icon file missing.")
			print("("+iconfile+")")
		if not os.path.isfile(ful):
			print("Error!  Executable file missing.")
			WaitForEnter()
			quit()
		fp.write("Icon="+iconfile+"\n")
		fp.close()
		os.chmod(ful,stat.S_IRWXU|stat.S_IRGRP|stat.S_IXGRP|stat.S_IROTH|stat.S_IXOTH)
#	except:
#		print("Error! Cannot place a desktop icon!")
#		WaitForEnter();


def FindYsflightFileSourceDir():
	THISSCRIPT=os.path.realpath(__file__)
	THISDIR=os.path.dirname(THISSCRIPT)
	candidateDir=[
		THISDIR,
		os.path.join(THISDIR,"ysflight64_gl1.app","Contents","Resources"),
		os.path.join(THISDIR,"ysflight64_gl2.app","Contents","Resources"),
		os.path.join(THISDIR,"YS FLIGHT SIMULATOR.app","Contents","Resources"),
		os.path.join(THISDIR,"YS FLIGHT SIMULATOR(GL1.x).app","Contents","Resources"),
		os.path.join(THISDIR,"YS FLIGHT SIMULATOR(GL2.x).app","Contents","Resources"),
	]
	for cd in candidateDir:
		if os.path.isfile(os.path.join(cd,"ysflight64_gl2")):
			return cd

	testPackageDir=os.path.expanduser(os.path.join("~","Packaging","Linux","Ysflight"))
	if os.path.isfile(os.path.join(testPackageDir,"ysflight64_gl2")):
		print("*** It is a dry run. ***")
		WaitForEnter()
		return testPackageDir

	print("YS FLIGHT SIMULATOR files are not found.")
	print("Please unzip the contents correctlly and then run this script.")
	WaitForEnter()
	quit()



################################################################################
# main
################################################################################

if sys.platform.startswith('darwin'):
	print("This installer is for Linux operating system.")
	print("In MacOSX, please double click YS FLIGHT SIMULATOR icon to start the program.")
	WaitForEnter()
	quit()
elif not sys.platform.startswith('linux'):
	print("This installer is for Linux operating system.")
	WaitForEnter()
	quit()


YSFLIGHTPATH=os.path.expanduser(os.path.join("~","YSFLIGHT.COM","ysflight"))

print("This script will copy the YSFLIGHT program to ~/YSFLIGHT.COM/ysflight and then")
print("Create desktop icons for the programs.  Older installation will be deleted.")
print("")
WaitForEnter()

if os.path.isdir(YSFLIGHTPATH):
	print("A program is already installed.")
	print("")
	print("If you need some files in the older installation, ")
	print("Press Ctrl+C and make a copy of ~/YSFLIGHT.COM/ysflight to a different location")
	print("and run this script again.")
	print("")
	print("If you are ok with overwriting the older version, press Enter to continue.")
	WaitForEnter()
	print("Please press Enter again to confirm you delete the previous installation.")
	WaitForEnter()
	shutil.rmtree(YSFLIGHTPATH)


shutil.copytree(FindYsflightFileSourceDir(),YSFLIGHTPATH)



print("YSFLIGHT copied to:"+YSFLIGHTPATH)



YSFLIGHT32_GL1=os.path.join(YSFLIGHTPATH,"ysflight32_gl1")
YSFLIGHT32_GL2=os.path.join(YSFLIGHTPATH,"ysflight32_gl2")
YSFLIGHT32_CONSVR=os.path.join(YSFLIGHTPATH,"ysflight32_nownd")

YSFLIGHT64_GL1=os.path.join(YSFLIGHTPATH,"ysflight64_gl1")
YSFLIGHT64_GL2=os.path.join(YSFLIGHTPATH,"ysflight64_gl2")
YSFLIGHT64_CONSVR=os.path.join(YSFLIGHTPATH,"ysflight64_nownd")

#FileCheck(YSFLIGHT32_GL1)
#FileCheck(YSFLIGHT32_GL2)
#FileCheck(YSFLIGHT32_CONSVR)
FileCheck(YSFLIGHT64_GL1)
FileCheck(YSFLIGHT64_GL2)
FileCheck(YSFLIGHT64_CONSVR)



if sys.maxsize>2**32:
	bitness=64
else:
	bitness=32

print("Detected "+str(bitness)+"-bit environment")



if 64==bitness:
	MakeDesktopIcon("YSFLIGHT1.desktop","YSFLIGHT\\n(OpenGL 1.1)",YSFLIGHT64_GL1)
	MakeDesktopIcon("YSFLIGHT2.desktop","YSFLIGHT\\n(OpenGL 2.x)",YSFLIGHT64_GL2)
	MakeDesktopIcon("YSFLIGHT3.desktop","YSFLIGHT\\n(Console Server)",YSFLIGHT64_CONSVR)
else:
	MakeDesktopIcon("YSFLIGHT1.desktop","YSFLIGHT\\n(OpenGL 1.1)",YSFLIGHT32_GL1)
	MakeDesktopIcon("YSFLIGHT2.desktop","YSFLIGHT\\n(OpenGL 2.x)",YSFLIGHT32_GL2)
	MakeDesktopIcon("YSFLIGHT3.desktop","YSFLIGHT\\n(Console Server)",YSFLIGHT32_CONSVR)

print("You can start YSFLIGHT by double-clicking an icon on the desktop")

WaitForEnter()
