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



def WriteFieldList(fp,fieldList):
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
	for f in fieldList:
		fp.write("GUI:SELECT $activeModalDialog fieldList "+f+"\n")
		fp.write("SLEEP:100\n")
		fp.write("\n")
	fp.write("GUI:CLICK $activeModalDialog ok\n")
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
	session=[]
	oneSession=[]
	for f in fieldList:
		oneSession=oneSession+[f]
		if 10==len(oneSession):
			session=session+[oneSession]
			oneSession=[]
	if 0<len(oneSession):
		session=session+[oneSession]

	for s in session:
		fp=open("allfield_generated.txt","w")
		WriteHeader(fp)
		WriteFieldList(fp,s)
		WriteFooter(fp)

		fp.close()

		for binFName in file_location.GetTestBinary():
			print("BINARY:"+binFName)
			common.RunOneSession("All-Field-Generated",binFName,"allfield_generated.txt",additional_param)



if __name__=="__main__":
	main()


