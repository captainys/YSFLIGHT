import os
import sys



def GetNewestMTime(path):
	newest=0
	for fName in os.listdir(path):
		ful=os.path.join(path,fName)

		mtime=os.path.getmtime(ful)
		print(fName,mtime)
		if newest<mtime:
			newest=mtime

		if os.path.isdir(ful):
			mtime=GetNewestMTime(ful)
			if newest<mtime:
				newest=mtime

	return newest



# THISDIR=os.path.dirname(os.path.realpath(__file__))
# newest=GetNewestMTime(os.path.join(THISDIR,"..","runtime"))

if len(sys.argv)<2:
	newest=GetNewestMTime(".")
	dir="."
else:
	newest=GetNewestMTime(sys.argv[1])
	dir=sys.argv[1]

if os.path.isfile(dir):
	dir=os.path.dirname(dir)


print("Newest="+str(newest))



fName=os.path.join(dir,"ys_timestamp.txt")
fp=open(fName,"w")
fp.write(str(newest))
fp.close()
