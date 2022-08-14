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



def WriteAircraft(fp,aircraft):
	fp.write("GUI:CLICK $mainMenu sim/create\n")
	fp.write("REM Delete current flight?\n")
	fp.write("GUI:CLICK confirmDeleteDlg ok\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("GUI:CLICK $activeModalDialog playerAirBtn\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("GUI:SELECT $activeModalDialog airList "+aircraft+"\n")
	fp.write("SLEEP:500\n")
	fp.write("\n")
	fp.write("GUI:CLICK $activeModalDialog ok\n")
	fp.write("\n")
	fp.write("\n")
	fp.write("GUI:CLICK $activeModalDialog flynow\n")
	fp.write("REM Joystick warning (may or may not)\n")
	fp.write("GUI:CLOSEMODAL\n")
	fp.write("\n")
	fp.write("REM Center Joystick\n")
	fp.write("RAWKEY:SPACE\n")
	fp.write("WAITFOR:FLYING\n")
	fp.write("SLEEP:2000\n")
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

	airList=list.GetAirList(additional_param)

	session=[]
	oneSession=[]
	for airName in airList:
		oneSession=oneSession+[airName]
		if 8<=len(oneSession):
			session=session+[oneSession]
			oneSession=[]
	if 0<len(oneSession):
		session=session+[oneSession]

	for s in session:
		fp=open("allaircraft_generated.txt","w")
		WriteHeader(fp)
		for f in s:
			WriteAircraft(fp,f)
		WriteFooter(fp)

		fp.close()

		for binFName in file_location.GetTestBinary():
			print("BINARY:"+binFName)
			common.RunOneSession("All-Aircraft-Generated",binFName,"allaircraft_generated.txt",additional_param)



if __name__=="__main__":
	main()


