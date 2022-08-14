import subprocess
import os
import sys

import common
import file_location



def EnableNetworkClientFlightRecord():
	binFName=file_location.GetTestBinary()
	subprocess.Popen([binFName[0],"-script","netconfig_client_enable_recording.txt"]).wait()



def RunNetworkTest_Basic():
	binFName=file_location.GetTestBinary()
	svrFName=file_location.GetConsoleServerBinary()

	svrProc=subprocess.Popen([svrFName,"-server","ServerUser","-closeserverlogoff"])

	THISDIR=os.path.dirname(os.path.realpath(__file__))
	clientScript=[
		os.path.join(THISDIR,"menu_network_client1.txt"),
		os.path.join(THISDIR,"menu_network_client2.txt"),
		os.path.join(THISDIR,"menu_network_client3.txt"),
	]
	clientProc=[]
	idx=0
	for fn in binFName:
		clientProc.append(
			subprocess.Popen([fn,"-script",clientScript[idx]])
		)
		idx=idx+1
		if 3<=idx:
			break

	svrProc.communicate()

	for proc in clientProc:
		proc.communicate()




def RunNetworkTest_Combat():
	binFName=file_location.GetTestBinary()
	svrFName=file_location.GetConsoleServerBinary()

	svrProc=subprocess.Popen([svrFName,"-server","ServerUser","-closeserverlogoff"])

	THISDIR=os.path.dirname(os.path.realpath(__file__))
	clientScript=[
		os.path.join(THISDIR,"menu_network_client_shootdown1.txt"),
		os.path.join(THISDIR,"menu_network_client_shootdown2.txt"),
	]
	clientProc=[]
	idx=0
	while idx<2:
		fn=binFName[idx%len(binFName)]
		clientProc.append(
			subprocess.Popen([fn,"-script",clientScript[idx%2]])
		)
		idx=idx+1

	svrProc.communicate()

	for proc in clientProc:
		proc.communicate()




def main():
	common.MakeDataDir()

	RunNetworkTest_Basic()
	RunNetworkTest_Combat()



if __name__=="__main__":
	main()
