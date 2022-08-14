#include <ysport.h>

#include "fscmdparaminfo.h"
#include "fsfilename.h"
#include "fsoption.h"

FsCommandParameter::FsCommandParameter()
{
#ifdef WIN32
	keyMenu=YSFALSE;
#else
	keyMenu=YSTRUE;
#endif
	autoExit=YSFALSE;
	executionMode=0;
	showAirList=YSFALSE;
	showFldList=YSFALSE;
	showStpList=YSFALSE;
	deleteFirstStartFile=YSFALSE;
	autoSave=YSFALSE;
	netPort=-1;
	netPlayerName.Set("Nameless");
	netServerName.Set("");
	netCloseServerWhenAllLogOut=YSTFUNKNOWN;
	yfsFilename.SetLength(0);
	airName.Set("");
	fldName.Set("");
	startPos.Set("");

	endModeNumWingman=0;
	endModeWingmanLevel=0;
	endModeAllowAAM=YSFALSE;

	setDefConfig=YSFALSE;
	setDefNetConfig=YSFALSE;
	setDefKey=YSFALSE;
	setDefOption=YSFALSE;

	prepareRelease=YSFALSE;
}

YSRESULT FsCommandParameter::RecognizeCommandParameter(int ac,char *av[])
{
	int i;
	i=1;
	while(i<ac)
	{
		YsString cmd(av[i]);
		cmd.Uncapitalize();
		if(0==cmd.Strlen())
		{
			++i;
			continue;
		}
		else if(0==cmd.STRCMP("-keymenu"))
		{
			keyMenu=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-configdir") && i+1<ac)
		{
			YsWString wStr;
			YsSystemEncodingToUnicode(wStr,av[i+1]);
			FsOverrideUserConfigDir(wStr);
			i+=2;
		}
		else if(0==cmd.STRCMP("-userdir") && i+1<ac)
		{
			YsWString wStr;
			YsSystemEncodingToUnicode(wStr,av[i+1]);
			FsOverrideUserDir(wStr);
			i+=2;
		}
		else if(0==cmd.STRCMP("-english"))
		{
			FsOption::SetLanguageStringOverrideFromCommandLine("en");
			i++;
		}
		else if(0==cmd.STRCMP("-language") && i+1<ac)
		{
			FsOption::SetLanguageStringOverrideFromCommandLine(av[i+1]);
			i+=2;
		}
		else if(0==cmd.STRCMP("-replayrecord") && i+1<ac)
		{
			executionMode=100;
			YsWString wStr;
			YsSystemEncodingToUnicode(wStr,av[i+1]);
			yfsFilename.Set(wStr);
			i+=2;
		}
		else if(0==cmd.STRCMP("-server") && i+1<ac)
		{
			executionMode=1;
			netPlayerName.Set(av[i+1]);
			i+=2;
			if(i<ac && av[i][0]!='-')
			{
				fldName.Set(av[i]);
				i++;
			}
		}
		else if(0==cmd.STRCMP("-closeserverlogoff"))
		{
			netCloseServerWhenAllLogOut=YSTRUE;
			++i;
		}
		else if(0==cmd.STRCMP("-client") && i+2<ac)
		{
			executionMode=2;
			netPlayerName.Set(av[i+1]);
			netServerName.Set(av[i+2]);
			i+=3;
		}
		else if(0==cmd.STRCMP("-freeflight") && i+3<ac)
		{
			executionMode=3;
			airName.Set(av[i+1]);
			fldName.Set(av[i+2]);
			startPos.Set(av[i+3]);
			i+=4;
		}
		else if(0==cmd.STRCMP("-flyyfs") && i+1<ac)
		{
			executionMode=4;
			YsWString wStr;
			YsSystemEncodingToUnicode(wStr,av[i+1]);
			yfsFilename.Set(wStr);
			i+=2;
		}
		else if(0==cmd.STRCMP("-demoforever"))
		{
			executionMode=EXEMODE_OPENINGDEMOFOREVER;
			++i;
		}
		else if(0==cmd.STRCMP("-netport") && i+1<ac)
		{
			netPort=atoi(av[i+1]);
			i+=2;
		}
		else if(0==cmd.STRCMP("-autoexit"))
		{
			autoExit=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-saveflight") && i+1<ac)
		{
			autoSave=YSTRUE;
			YsWString wStr;
			YsSystemEncodingToUnicode(wStr,av[i+1]);
			yfsFilename.Set(wStr);
			i+=2;
		}
		else if(0==cmd.STRCMP("-listairplane"))
		{
			showAirList=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-listfield"))
		{
			showFldList=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-liststartpos") && i+1<ac)
		{
			showStpList=YSTRUE;
			fldName.Set(av[i+1]);
			i+=2;
		}
		else if(0==cmd.STRCMP("-endurance") && i+5<ac)
		{
			executionMode=5;
			airName.Set(strlen(av[i+1])+1,av[i+1]);
			fldName.Set(strlen(av[i+2])+1,av[i+2]);
			endModeNumWingman=YsBound(atoi(av[i+3]),0,2);
			endModeWingmanLevel=YsBound(atoi(av[i+4]),1,5);
			endModeAllowAAM=(YSBOOL)atoi(av[i+5]);
			i+=6;
		}
		else if(0==cmd.STRCMP("-intercept") && i+8<ac)
		{
			executionMode=6;
			airName.Set(strlen(av[i+1])+1,av[i+1]);
			fldName.Set(strlen(av[i+2])+1,av[i+2]);
			interceptMissionInfo.attackerInfo.allowStealth=(YSBOOL)atoi(av[3]);
			interceptMissionInfo.attackerInfo.allowAirCover=(YSBOOL)atoi(av[4]);
			interceptMissionInfo.attackerInfo.allowHeavyBomber=(YSBOOL)atoi(av[5]);
			interceptMissionInfo.attackerInfo.allowBomb=(YSBOOL)atoi(av[6]);
			interceptMissionInfo.attackerInfo.maxNumAttacker=YsBound(atoi(av[7]),1,5);
			interceptMissionInfo.numWingman=YsBound(atoi(av[8]),0,2);
			i+=9;
		}
		else if(0==cmd.STRCMP("-setdefaultconfig"))
		{
			setDefConfig=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-setdefaultnetconfig"))
		{
			setDefNetConfig=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-setdefaultkeyassign"))
		{
			setDefKey=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-setdefaultoption"))
		{
			setDefOption=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-preparerelease"))
		{
			FsUseLocalFolderSetting();
			setDefConfig=YSTRUE;
			setDefNetConfig=YSTRUE;
			setDefKey=YSTRUE;
			setDefOption=YSTRUE;
			prepareRelease=YSTRUE;
			autoExit=YSTRUE;
			i++;
		}
		else if(0==cmd.STRCMP("-deletefirststart"))
		{
			deleteFirstStartFile=YSTRUE;
			autoExit=YSTRUE;
			++i;
		}
		else if(0==cmd.STRCMP("-script") && i+1<ac)
		{
			// Test script file name must be a full-path name.
			// Working directory will be the YSFLIGHT directory.
			YsSystemEncodingToUnicode(testScriptFilename,av[i+1]);
			i+=2;
		}
		else if(0==cmd.STRCMP("-h") || 0==cmd.STRCMP("-help"))
		{
			ShowHelp();
			return YSERR;
		}
		else
		{
			ShowHelp();

			printf("\n");
			printf("\n");
			printf("ERROR: Unrecognized parameter \"%s\"\n",cmd.GetArray());

		#ifndef __APPLE__
			// macOS can throw misterious command parameters.
			// Just ignore them.
			return YSERR;
		#endif
		}
	}
	return YSOK;
}

void FsCommandParameter::ShowHelp(void)
{
	printf("YSFLIGHT Command Parameters.\n");

	printf("  -h\n");
	printf("  -help\n");
	printf("   Show help.\n");
	printf("\n");

	printf("  -keymenu\n");
	printf("   Use key menu.\n");
	printf("\n");

	printf("  -configdir Path\n");
	printf("   Override user-configuration directory.\n");

	printf("  -userdir Path\n");
	printf("   Override user YSFLIGHT directory.\n");

	printf("  -replayrecord Filename\n");
	printf("   Replay flight record.\n");
	printf("\n");

	printf("  -freeflight Airplane Field Position\n");
	printf("   Fly Free Flight.\n");
	printf("\n");

	printf("  -flyyfs Filename\n");
	printf("   Load .YFS and fly.\n");
	printf("\n");

	printf("  -endurance Airplane Field NWingmen WingmenLvl UseMissile\n");
	printf("   Endurance Mode (15 minutes dogfight)\n");
	printf("     NWingmen    : 0 to 2\n");
	printf("     WingmenLvl  : 1 to 5\n");
	printf("     UseMissile  : 0(not use) or 1(use)\n");
	printf("\n");

	printf("  -intercept Airplane Field Stealth Escort HeavyBomber Bomb NEnemy NWingmen\n");
	printf("   Intercept Mission (15 minutes base defense)\n");
	printf("     Stealth     : 0(not allow stealth) or 1(allow stealth)\n");
	printf("     Escort      : 0(not allow fighter escort) or 1(allow fighter escort)\n");
	printf("     Bomb        : 0(not allow bomb) or 1(allow bomb)\n");
	printf("     NEnemy      : 1 to 5\n");
	printf("     NWingmen    : 0 to 2\n");
	printf("\n");

	printf("  -server Username\n");
	printf("   Start server mode.\n");
	printf("\n");

	printf("  -closeserverlogoff\n");
	printf("   Close server when everyone logs off.\n");
	printf("\n");

	printf("  -client Username ServerHostName\n");
	printf("   Start client mode.\n");
	printf("\n");

	printf("  -netport portNumber\n");
	printf("   Specify port number.\n");
	printf("\n");

	printf("  -autoexit\n");
	printf("   Exit after flight. (Ignores -keymenu.)\n");
	printf("\n");

	printf("  -saveflight Filename\n");
	printf("   Save after flight.\n");
	printf("\n");

	printf("  -listairplane\n");
	printf("   Show airplane list.\n");
	printf("\n");

	printf("  -listfield\n");
	printf("   Show field list.\n");
	printf("\n");

	printf("  -liststartpos Fieldname\n");
	printf("   Show start position list.\n");
	printf("\n");

	printf("  -setdefaultconfig\n");
	printf("   Set default configuration.\n");
	printf("\n");
	printf("  -setdefaultnetconfig\n");
	printf("   Set default network configuration.\n");
	printf("\n");
	printf("  -setdefaultkeyassign\n");
	printf("   Set default keyboard assignment.\n");
	printf("\n");
	printf("  -setdefaultoption\n");
	printf("   Set default option.\n");
	printf("\n");
	printf("  -english\n");
	printf("   Force English mode.\n");
	printf("\n");
	printf("  -language [languageString]\n");
	printf("   Force to use given language string (eg. en for English, ja for Japanese.)\n");
}

