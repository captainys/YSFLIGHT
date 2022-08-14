import subprocess
import file_location


def GetFieldList(additional_param=[]):
	allBinFName=file_location.GetTestBinary()

	print(allBinFName[0])

	proc=subprocess.Popen([allBinFName[0],"-listfield","-autoexit"]+additional_param,stdout=subprocess.PIPE)
	res=proc.communicate()

	utf8=res[0].decode('utf-8').splitlines()

	fieldList=[]

	for line in utf8:
		if line.startswith("FLD:"):
			fieldList.append(line.replace("FLD:",""))

	print(fieldList)
	
	return fieldList



def GetAirList(additional_param=[]):
	allBinFName=file_location.GetTestBinary()

	print(allBinFName[0])

	proc=subprocess.Popen([allBinFName[0],"-listairplane","-autoexit"]+additional_param,stdout=subprocess.PIPE)
	res=proc.communicate()

	utf8=res[0].decode('utf-8').splitlines()

	airList=[]

	for line in utf8:
		if line.startswith("AIR:"):
			airList.append(line.replace("AIR:",""))

	print(airList)
	
	return airList



def GetStpList(fld,additional_param=[]):
	allBinFName=file_location.GetTestBinary()

	print(allBinFName[0])

	proc=subprocess.Popen([allBinFName[0],"-liststartpos",fld,"-autoexit"]+additional_param,stdout=subprocess.PIPE)
	res=proc.communicate()

	utf8=res[0].decode('utf-8').splitlines()

	stpList=[]

	for line in utf8:
		if line.startswith("STP:"):
			stpList.append(line.replace("STP:",""))

	print(stpList)

	return stpList



def main():
	fieldList=GetFieldList()
	airList=GetAirList()
	for f in fieldList:
		stpList=GetStpList(f)



if __name__=="__main__":
	main()
