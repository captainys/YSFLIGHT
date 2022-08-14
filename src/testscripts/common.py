import subprocess
import os
import sys



def RunOneSession(testLabel,binFName,scriptFName,additional_param=[]):
	THISDIR=os.path.dirname(os.path.realpath(__file__))

	print("SESSION:"+testLabel)
	proc=subprocess.Popen([binFName,"-script",os.path.join(THISDIR,scriptFName)]+additional_param)
	proc.communicate()
	if 0!=proc.returncode:
		print("ERROR!")
		print("RETURN:"+str(proc.returncode))
		print("SESSION:"+testLabel)
		print("BINARY:"+binFName)
		print("SCRIPT:"+scriptFName)
		raise



def MakeDataDir():
	dir=os.path.join("~","YSFLIGHT_TEST_DATA")
	dir=os.path.expanduser(dir)
	if not os.path.isdir(dir):
		os.makedirs(dir)
