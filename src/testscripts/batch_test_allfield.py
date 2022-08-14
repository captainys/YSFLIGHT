import common
import file_location
import list




def WriteHeader(fp):
	fp.write("REM No space between GUI and : and following command.\n")
	fp.write("GUI:CLOSEMODAL\n")
	fp.write("GUI:CLOSEMODAL\n")
	fp.write("GUI:CLOSEMODAL\n")
	fp.write("GUI:CLOSEMODAL\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("REM Get out of demo mode.\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("\n")
	fp.write("REM Simulation -> Create Flight\n")
	fp.write("GUI:CLICK $mainMenu sim/create\n")
	fp.write("WAITFOR:MODAL\n")
	fp.write("GUI:CLICK $activeModalDialog ok\n")
	fp.write("WAITFOR:NO_MODAL\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")



def WriteField(fp,field,stp):
	fp.write("REM ###########################################################################\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("REM Create Flight -> Fly\n")
	fp.write("\n")
	fp.write("GUI:CLICK $mainMenu sim/create\n")
	fp.write("REM Delete current flight?\n")
	fp.write("GUI:CLICK $activeModalDialog ok\n")
	fp.write("\n")
	fp.write("GUI:SELECT $activeModalDialog fieldList "+field+"\n")
	fp.write("\n")
	# fp.write("GUI:CLICK $activeModalDialog playerStpList\n")
	# fp.write("GUI:TYPE N\n")
	fp.write("GUI:SELECT $activeModalDialog playerStpList "+stp+"\n")
	fp.write("\n")
	fp.write("GUI:CLICK $activeModalDialog flynow\n")
	fp.write("REM Joystick warning (may or may not)\n")
	fp.write("GUI:CLOSEMODAL\n")
	fp.write("\n")
	fp.write("REM Center Joystick\n")
	fp.write("RAWKEY:SPACE\n")
	fp.write("WAITFOR:FLYING\n")
	fp.write("\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("RAWKEY:ESC\n")
	fp.write("WAITFOR:GUI\n")
	fp.write("\n")
	fp.write("REM Result Dialog\n")
	fp.write("GUI:CLICK $activeModalDialog close\n")
	fp.write("WAITFOR:NO_MODAL\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")



def WriteFooter(fp):
	fp.write("REM ###########################################################################\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("REM File -> Exit\n")
	fp.write("\n")
	fp.write("GUI:CLICK $mainMenu file/exit\n")
	fp.write("\n")
	fp.write("WAITFOR:MODAL\n")
	fp.write("REM Delete current flight?\n")
	fp.write("GUI:CLICK $activeModalDialog ok\n")
	fp.write("REM Really exit?\n")
	fp.write("GUI:CLICK $activeModalDialog ok\n")



def main(additional_param=[]):
	common.MakeDataDir()

	fieldList=list.GetFieldList(additional_param)



	fieldStpPair=[]
	batch=[]
	for f in fieldList:
		stpList=list.GetStpList(f,additional_param)
		for s in stpList:
			batch.append([f,s])
			if 16<=len(batch):
				fieldStpPair.append(batch)
				batch=[]
	if 0<len(batch):
		fieldStpPair.append(batch)
		batch=[]

	for b in fieldStpPair:
		fp=open("allfield_generated.txt","w")
		WriteHeader(fp)

		stpList=list.GetStpList(f,additional_param)
		for fs in b:
			WriteField(fp,fs[0],fs[1])

		WriteFooter(fp)

		fp.close()

		for binFName in file_location.GetTestBinary():
			print("BINARY:"+binFName)
			common.RunOneSession("All-Field-Generated",binFName,"allfield_generated.txt",additional_param)



if __name__=="__main__":
	main()


