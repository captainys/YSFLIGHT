#!/usr/bin/python

import sys
import os
import subprocess
import shutil

import yscmd
import yssvn
import yscmake
import ysmake
import yspackaging

import export_fsguilib
import export_ysjoystick



PROJECT_NAME="YsJoystickReader"



#################
# Program entry #
#################

#main

ZIPCMD=yscmd.FindFolderArchiver()
TRUNK_ROOT=yssvn.TrunkDir()
SRC_ROOT=os.path.join(TRUNK_ROOT,"src")
BUILD32DIR=yscmake.FindBuild32Dir()
BUILD64DIR=yscmake.FindBuild64Dir()

EXE32DIR=yscmake.FindExe32Dir()
EXE64DIR=yscmake.FindExe64Dir()

OSLABEL=yspackaging.OSLabel()

PACKAGEDIR=os.path.join(yspackaging.PackagingDir(),PROJECT_NAME)
PACKAGEBINDIR=os.path.join(PACKAGEDIR,"bin")
PACKAGESRCDIR=os.path.join(os.path.join(PACKAGEDIR,"src"),"src")

THISPATH=os.path.realpath(__file__)
THISDIR=os.path.dirname(THISPATH)

ZIP_ROOT=yspackaging.ZipCopyDir()
ZIPFILE=os.path.join(ZIP_ROOT,PROJECT_NAME+'_'+OSLABEL+'_R'+str(yssvn.Revision(TRUNK_ROOT))+'.zip')

copy_to_bin=[]
copy_to_src=[]
copy_to_package=[]

print(ZIPCMD)
print(TRUNK_ROOT)
print(SRC_ROOT)
print(BUILD32DIR)
print(BUILD64DIR)
print(EXE32DIR)
print(EXE64DIR)
print(PACKAGEDIR)
print(THISDIR)
print(ZIPFILE)



# svn update
yssvn.Update()



#cmake
yscmake.Cmake32and64()



#Update svnversion.h
#yssvn.UpdateRevisionHeader(os.path.join(SRC_ROOT,"svnrevision/yssvnrevision.h"))



#Build
if sys.platform.startswith('linux'):
	ysmake.Make(BUILD64DIR,["ysjoycalib"])
	copy_to_bin.append(os.path.join(os.path.join(EXE64DIR,"YsJoystickReader"),"ysjoycalib"))
	copy_to_bin.append(os.path.join(THISDIR,"runtime/language"))
elif sys.platform.startswith('darwin'):
	ysmake.Make(BUILD64DIR,["ysjoycalib"])
	copy_to_bin.append(os.path.join(os.path.join(EXE64DIR,"YsJoystickReader"),"ysjoycalib.app"))
elif sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
	ysmake.Msbuild(os.path.join(BUILD32DIR,"YS.sln"),ysmake.VCReleaseConfig(32),["ysjoystick/ysjoycalib"])
	copy_to_bin.append(os.path.join(os.path.join(EXE32DIR,"YsJoystickReader"),"ysjoycalib.exe"))
	copy_to_bin.append(os.path.join(THISDIR,"runtime/language"))
else:
	print("What is this system?  ",sys.platform)
	quit()



#Make Package


if os.path.isdir(PACKAGEDIR):
	shutil.rmtree(PACKAGEDIR)
os.makedirs(PACKAGESRCDIR)
os.makedirs(PACKAGEBINDIR)

yspackaging.Copy(copy_to_package,PACKAGEDIR)
yspackaging.Copy(copy_to_bin,PACKAGEBINDIR)
yspackaging.Copy(copy_to_src,PACKAGESRCDIR)

export_ysjoystick.Export(PACKAGESRCDIR)

if sys.platform.startswith('darwin'):
	renameFrom=os.path.join(PACKAGEBINDIR,"ysjoycalib.app")
	renameTo=os.path.join(PACKAGEBINDIR,"Ys Joystick Calibrator.app")
	os.rename(renameFrom,renameTo)



#Zip
yscmd.MakeFreshZip(ZIPFILE,PROJECT_NAME,yspackaging.PackagingDir())



print("Packaging Done")
print("Package Contents are in: "+PACKAGEDIR)
print("Zip file is in: "+ZIP_ROOT)
