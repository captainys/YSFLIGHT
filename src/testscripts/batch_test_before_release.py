# This script tests a package in ~/Packaging.
# Should work for test release and stable release.

import os
import sys

import batch_test
import file_location


if sys.platform.startswith('darwin'):
	appPath=os.path.join("~","Packaging","MacOSX","Ysflight")
elif sys.platform.startswith('linux'):
	appPath=os.path.join("~","Packaging","MacOSX","Ysflight","ysflight64_gl1.app","Contents","Resources")
elif sys.platform.startswith('win') or sys.platform.startswith('cygwin'):
	appPath=os.path.join("~","Packaging","Windows","Ysflight")
	file_location.bitness="32"
else:
	print("Unsupported platform.")
	raise



file_location.appLocation=os.path.expanduser(appPath)
file_location.winAppConfig=""
batch_test.main()
