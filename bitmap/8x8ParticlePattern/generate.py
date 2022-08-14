#!/usr/bin/python

import os
import sys
import subprocess

import ysmake
import yscmake
import yssvn


def ExeExt():
	if sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
		return ".exe"
	else:
		return ""

def ResizeBmpCmd():
	EXE64DIR=yscmake.FindExe64Dir()
	return os.path.join(EXE64DIR,"ysresizebmp"+ExeExt())

def MakeBlankBmpCmd():
	EXE64DIR=yscmake.FindExe64Dir()
	return os.path.join(EXE64DIR,"ysmakeblankbmp"+ExeExt())

def MakeBmpTileCmd():
	EXE64DIR=yscmake.FindExe64Dir()
	return os.path.join(EXE64DIR,"ysmakebmptile"+ExeExt())



TRUNK_ROOT=yssvn.TrunkDir()
SRC_ROOT=os.path.join(TRUNK_ROOT,"src")
BUILD32DIR=yscmake.FindBuild32Dir()
BUILD64DIR=yscmake.FindBuild64Dir()


if sys.platform.startswith('linux') or sys.platform.startswith('darwin'):
	ysmake.Make(BUILD64DIR,[
		"ysmakebmptile",
		"ysmakeblackwhite",
		"ysresizebmp",
		"ysmakewhitetransparent",
		"ysnegatebmp",
		"ysmakeblankbmp",
	])
elif sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
	ysmake.Msbuild(os.path.join(BUILD64DIR,"YS.sln"),ysmake.VCReleaseConfig(64),[
		"ysmakebmptile",
		"ysmakeblackwhite",
		"ysresizebmp",
		"ysmakewhitetransparent",
		"ysnegatebmp",
		"ysmakeblankbmp",
	])
	exeExt=".exe"
else:
	print("What is this system?  ",sys.platform)



subprocess.Popen([ResizeBmpCmd(),"explosion01.png","ptn00.png","128","128"]).wait()
subprocess.Popen([ResizeBmpCmd(),"explosion02.png","ptn10.png","128","128"]).wait()
subprocess.Popen([ResizeBmpCmd(),"explosion03.png","ptn20.png","128","128"]).wait()
subprocess.Popen([ResizeBmpCmd(),"explosion04.png","ptn30.png","128","128"]).wait()
subprocess.Popen([ResizeBmpCmd(),"explosion05.png","ptn40.png","128","128"]).wait()
subprocess.Popen([ResizeBmpCmd(),"flash01.png","ptn01.png","128","128"]).wait()
subprocess.Popen([ResizeBmpCmd(),"cloud01.png","ptn02.png","128","128"]).wait()
subprocess.Popen([ResizeBmpCmd(),"cloud02.png","ptn12.png","128","128"]).wait()

subprocess.Popen([MakeBlankBmpCmd(),"empty.png","128","128"]).wait()


subprocess.Popen([MakeBmpTileCmd(),
	"../../runtime/misc/particle_tile.png","1024","1024",
	"ptn00.png","ptn10.png","ptn20.png","ptn30.png","ptn40.png","empty.png","empty.png","empty.png",
	"ptn01.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png",
	"ptn02.png","ptn12.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png",
	"empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png",
	"empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png",
	"empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png",
	"empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png","empty.png",
]).wait()



os.remove("ptn00.png")
os.remove("ptn10.png")
os.remove("ptn20.png")
os.remove("ptn30.png")
os.remove("ptn40.png")
os.remove("ptn01.png")
os.remove("ptn02.png")
os.remove("ptn12.png")

os.remove("empty.png")
