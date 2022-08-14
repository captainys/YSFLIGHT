import os
import install_addon
import batch_test_allfield_light
import batch_test_allaircraft_light



# Add-ons must be stored in ~/ysflightextra



def main():
	zipDir=os.path.join("~","ysflightextra")
	zipDir=os.path.expanduser(zipDir)
	testEnvDir=os.path.join("~","ysflight_testenv")
	testEnvDir=os.path.expanduser(testEnvDir)
	additional_param=["-userdir",testEnvDir,"-configdir",testEnvDir]
	if os.path.isdir(zipDir):
		install_addon.InstallMultiAddOn(zipDir,testEnvDir)
		batch_test_allaircraft_light.main(additional_param)
		batch_test_allfield_light.main(additional_param)
	else:
		print("No add-on package found.  Skipping.")



if __name__=="__main__":
	main()
