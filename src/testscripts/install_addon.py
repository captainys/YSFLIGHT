import sys
import os
import shutil
import subprocess
import zipfile



# install_addon.py Version 20180726
#
# This script covers majority of the add-on packages I downloaded from the web.
# It assumes that the user data is under 'User' or 'user' sub-directory somewhere in the .zip file.
# Windows doesn't care about capitalization, but Linux and macOS do.
# Therefore, this script looks into .lst files and correct capitalization if the file name in the .lst file does not match
# the capitalization of the actual file.
#
# Now at least I can test that the program does not crash or freeze if I install add-on packages in a batch!
#
# Of course you need to have Python installed on your system to use this script.
# This script has been tested with Python 3.6.  It may run with Python 2.x, but I haven't tested.
#
# BSD License.  Free for re-distribution.  Please feel free to customize for your own add-on package and bundle with package.
#
# Usage:
#    python install_addon.py package_zip_file.zip
#     or
#    python install_addon.py package_zip_file.zip install_directory
#
# If you do not specify the install_directory, the script installs to the default YSFLIGHT user directory (~/Documents/YSFLIGHT.COM/YSFLIGHT)
#
#
#
# By the way, I'm wondering why am I not writing this in C++ despite I am primarily a C++ programmer.  I love C++ much better than Python.
# One reason could be C++ Standard Template Library does not have a function to get file listing from the file system.
# Basic string operations, splitting, replacing etc. are not part of STL either, although it is very easy to write.
# I wish C++ standard takes a file-system and basic string operations in the future into STL.
# I can do the same if I link a set of my own class libraries, but then I cannot say easily "please feel free to customize and bundle."
#
# ... I think I soon need to write it in C++ anyway for iOS and Android.  But, for those purposes I'll use my own class libraries.




def DefaultFileList():
	return [
	]




def IsCommandAvailable(cmd):
	# From http://stackoverflow.com/questions/377017/test-if-executable-exists-in-python
	for path in os.environ["PATH"].split(os.pathsep):
		path=path.strip('"')
		#print("Searching "+cmd+" in "+path)
		exe_file=os.path.join(path,cmd)
		if os.path.isfile(exe_file) and os.access(exe_file,os.X_OK):
			return True
	#try:
	#	subprocess.Popen([cmd]).communicate()
	#except:
	#	return False
	return False



# srcPath/abc will be copied to dstPath/abc
# Also returns a list of copied files.
def ForceCopyTree(srcPath,instDir,dstPath):
	if not os.path.isdir(dstPath):
		os.makedirs(dstPath)

	copiedList=[]
	for fName in os.listdir(srcPath):
		srcFul=os.path.join(srcPath,fName)
		dstFul=os.path.join(instDir,dstPath,fName)

		if os.path.isfile(dstFul):
			os.remove(dstFul)

		if os.path.isdir(srcFul):
			if not os.path.isdir(dstFul):
				os.makedirs(dstFul)
			copiedList=copiedList+ForceCopyTree(srcFul,instDir,os.path.join(dstPath,fName))
		else:
			shutil.copyfile(srcFul,dstFul)
			copiedList.append(os.path.join(dstPath,fName).replace('\\','/'))

	return copiedList



# srcPath will be copied to dstPath.  dstPath must be a complete file name, not a directory name.
def ForceCopyFile(srcPath,dstPath):
	if os.path.isfile(dstPath):
		os.remove(dstPath)
	shutil.copyfile(srcPath,dstPath)



def FindUserDir(dir):
	for fName in os.listdir(dir):
		ful=os.path.join(dir,fName)
		if fName=="User" or fName=='user':
			return [ful,fName]
		elif os.path.isdir(ful):
			found=FindUserDir(ful)
			if []!=found:
				return found
	return []



def FindListFile(dir):
	lstFile=[]
	for fName in os.listdir(dir):
		ful=os.path.join(dir,fName)
		if os.path.isdir(ful):
			lstFile=lstFile+FindListFile(ful)
		else:
			ext=os.path.splitext(fName)[1].lower()
			if ext==".lst":
				lstFile.append([ful,fName])
	return lstFile



# Need to nuke a working directory after installation.
# Also I don't want to make a copied files read-only.
def MakeTreeWritable(dir):
	for fName in os.listdir(dir):
		ful=os.path.join(dir,fName)
		os.chmod(ful,0o777)
		if os.path.isdir(ful):
			MakeTreeWritable(ful)



def ExtractAll(zip):
	for info in zip.infolist():
		try:
			zip.extract(info)
		except:
			print("File: "+info.filename+" could not be extracted.")



def InstallAddOn(zipFName,instDir):
	airDir=os.path.join(instDir,"aircraft")
	gndDir=os.path.join(instDir,"ground")
	scnDir=os.path.join(instDir,"scenery")
	for dir in [airDir,gndDir,scnDir]:
		if not os.path.isdir(dir):
			os.makedirs(dir)


	workDir=os.path.join(instDir,"tempDir")
	if os.path.isdir(workDir):
		shutil.rmtree(workDir)
	os.makedirs(workDir)

	pushd=os.getcwd()


	print("****************************************************************")
	print("Zip File: "+zipFName)
	print("****************************************************************")
	zip=zipfile.ZipFile(zipFName,"r")

	os.chdir(workDir)
	ExtractAll(zip)
	MakeTreeWritable(os.getcwd())

	userDir=FindUserDir(workDir)
	print("Found user dir: ")
	print(userDir)

	lstFile=FindListFile(workDir)
	print("Found list files:")
	print(lstFile)

	if []==userDir:
		print("Cannot find a user directory.  Aborting installation.")
		return

	dataFile=ForceCopyTree(userDir[0],instDir,userDir[1])

	leftUninstalled=[]
	installedAirList=[]
	installedGndList=[]
	installedScnList=[]
	for lst in lstFile:
		if lst[1]=='scenary.lst' or lst[1]=='scenery.lst' or lst[1]=='aircraft.lst' or lst[1]=='ground.lst':
			# Sorry for misspelling 'scenary'!
			print("Skipping a generic .lst name:"+lst[1])
			print("Probably intended to overwrite the default .lst file?")
			continue

		if lst[1].startswith("air"):
			print("Installing "+lst[1]+" to aircraft")
			ForceCopyFile(lst[0],os.path.join(airDir,lst[1]))
			installedAirList.append(os.path.join(airDir,lst[1]))
		elif lst[1].startswith("gro"):
			print("Installing "+lst[1]+" to ground")
			ForceCopyFile(lst[0],os.path.join(gndDir,lst[1]))
			installedGndList.append(os.path.join(gndDir,lst[1]))
		elif lst[1].startswith("sce"):
			print("Installing "+lst[1]+" to scenery")
			ForceCopyFile(lst[0],os.path.join(scnDir,lst[1]))
			installedScnList.append(os.path.join(scnDir,lst[1]))
		else:
			leftUninstalled.append(lst[1])
			print("Warning!  Cannot identify the .lst file type ("+lst[1]+")");
			print("          This .lst file hasn't been installed.")

	if 0<len(leftUninstalled):
		print("Following .lst files haven't been installed.")
		print(leftUninstalled)



	FixCapitalization(instDir,installedAirList,installedGndList,installedScnList,dataFile)



	os.chdir(pushd)
	shutil.rmtree(workDir)



def InstallMultiAddOn(zipDirName,instDir):
	for zipFName in os.listdir(zipDirName):
		ext=os.path.splitext(zipFName)[1].lower()
		if ext=='.zip':
			zipFul=os.path.join(zipDirName,zipFName)
			print("["+zipFName+"]")
			InstallAddOn(zipFul,instDir)



################################################################################


def TryCorrectFileName(fName,lowerToActual):
	actual=lowerToActual.get(fName.lower().replace('"',''))
	if actual==None:
		if fName.startswith("aircraft/") or fName.startswith("ground/") or fName.startswith("scenery/"):
			print("File "+fName+" is probably a reference to a default file.")
		else:
			print("Warning: File "+fName+" does not exist.")
		return (False,fName)

	if None!=actual and fName!=actual:
		print("Correcting Capitalization: "+fName+" to "+actual)
		return (True,actual)
	else:
		return (False,fName)



def FixCapitalizationPerDatFile(datFName,lowerToActual,keywordDict):
	try:
		fp=open(datFName,"r")
	except:
		print("File "+datFName+" does not exist.")
		return

	updated=False
	fileContent=[]
	for s in fp:
		argv=s.split()
		if 0<len(argv) and None!=keywordDict.get(argv[0].upper()):
			(tf,newFName)=TryCorrectFileName(argv[1],lowerToActual)
			if True==tf:
				updated=True;
				s=s.replace(argv[1],newFName)
		fileContent.append(s)

	fp.close()

	if True==updated:
		print("Writing "+datFName)
		fp=open(datFName,"w")
		for s in fileContent:
			fp.write(s+"\n")
		fp.close()



def FixCapitalizationPerGndFile(instDir,datFName,lowerToActual):
	try:
		ifp=open(datFName,"r")
	except:
		print("Cannot open "+fName)
		return

	for s in ifp:
		argv=s.split()
		if 2<=len(argv) and "CARRIER"==argv[0]:
			acpFName=os.path.join(instDir,argv[1])
			FixCapitalizationPerAcpFile(acpFName,lowerToActual)



def FixCapitalizationPerAcpFile(acpFName,lowerToActual):
	try:
		fp=open(acpFName,"r")
		print("Looking into "+acpFName)
	except:
		print("Cannot open "+acpFName)
		return

	updated=False
	lineNum=0
	fileContent=[]
	for s in fp:
		s=s.splitlines()[0]
		if lineNum<4:
			(tf,newFName)=TryCorrectFileName(s,lowerToActual)
			if True==tf:
				updated=True;
				s=newFName
		fileContent.append(s)
		lineNum=lineNum+1
	fp.close()


	if True==updated:
		print("Writing "+acpFName)
		ofp=open(acpFName,"w")
		for s in fileContent:
			ofp.write(s+"\n")
		ofp.close()
		


def FixCapitalizationPerListFile(listFName,lowerToActual,skipFirstArg):
	txt=[]
	ifp=open(listFName,"r")
	for s in ifp:
		txt.append(s)
	ifp.close()

	newTxt=[]
	updated=False

	for s in txt:
		argv=s.split()
		first=True
		for arg in argv:
			if True==first and True==skipFirstArg:
				first=False
				continue

			(tf,actual)=TryCorrectFileName(arg,lowerToActual)
			if True==tf:
				updated=True
				print("Correcting Capitalization: "+arg+" to "+actual)
				s=s.replace(arg,actual)

			first=False

		newTxt.append(s)

	if True==updated:
		print("Updating: "+listFName)
		ofp=open(listFName,"w")
		for s in newTxt:
			ofp.write(s+"\n")
		ofp.close()




def FixCapitalization(instDir,airListFName,gndListFName,scnListFName,dataFile):
	lowerToActual=dict()
	for f in dataFile:
		lowerToActual[f.lower()]=f

	for fName in airListFName:
		FixCapitalizationPerListFile(fName,lowerToActual,False)

	for fName in gndListFName:
		FixCapitalizationPerListFile(fName,lowerToActual,False)

		try:
			fp=open(fName,"r")
		except:
			print("Cannot open "+fName)
			continue

		for s in fp:
			argv=s.split()
			if 0<len(argv):
				datFName=os.path.join(instDir,argv[0])
				datFName=os.path.expanduser(datFName)
				FixCapitalizationPerDatFile(datFName,lowerToActual,{"CARRIER":0})
				FixCapitalizationPerGndFile(instDir,datFName,lowerToActual)
		fp.close()

	for fName in scnListFName:
		FixCapitalizationPerListFile(fName,lowerToActual,True)




################################################################################



def main():
	if len(sys.argv)<2:
		print("Usage: python install_addon.py input_zip_file.zip install_destination_directory")
		print("            or,")
		print("       python install_addon.py input_zip_file.zip")
		print("  If you don't specify the destination, it uses ~/YSFLIGHT.COM/YSFLIGHT as the")
		print("  default YSFLIGHT user-data install location.")
	elif 3<=len(sys.argv):
		if not os.path.isdir(sys.argv[2]):
			os.makedirs(sys.argv[2])
		instDir=sys.argv[2]
	else:
		instDir=os.path.join("~","Documents","YSFLIGHT.COM","YSFLIGHT")
		instDir=os.path.expanduser(instDir)
	InstallAddOn(sys.argv[1],instDir)
	print("Installation done.")


if __name__=="__main__":
	main()
