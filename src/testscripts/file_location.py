import subprocess
import os
import sys



appLocation=os.path.expanduser(os.path.join("~","ysbin","build64","trunk","src","ysflight","src","main"))
winAppConfig="Release"
bitness="64"



def ExeExt():
	if sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
		return ".exe"
	return ""



def GetTestBinary():
	dirName=appLocation
	if sys.platform.startswith('darwin'):
		return [
			os.path.join(dirName,"ysflight"+bitness+"_gl1.app","Contents","MacOS","ysflight"+bitness+"_gl1"),
			os.path.join(dirName,"ysflight"+bitness+"_gl2.app","Contents","MacOS","ysflight"+bitness+"_gl2"),
		]
	elif sys.platform.startswith('linux'):
		return [
			os.path.join(dirName,"ysflight"+bitness+"_gl1"),
			os.path.join(dirName,"ysflight"+bitness+"_gl2"),
		]
	elif sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
		return [
			os.path.join(dirName,winAppConfig,"ysflight"+bitness+"_gl1"+ExeExt()),
			os.path.join(dirName,winAppConfig,"ysflight"+bitness+"_gl2"+ExeExt()),
			os.path.join(dirName,winAppConfig,"ysflight"+bitness+"_d3d9"+ExeExt()),
		]



def GetConsoleServerBinary():
	if sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
		return os.path.expanduser(os.path.join("~","ysbin","build"+bitness+"","trunk","src","ysflight","src","main_consvr","Release","ysflight"+bitness+"_nownd"+ExeExt()))
	else:
		return os.path.expanduser(os.path.join("~","ysbin","build"+bitness+"","trunk","src","ysflight","src","main_consvr","ysflight"+bitness+"_nownd"+ExeExt()))



