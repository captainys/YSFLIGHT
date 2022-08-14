#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

#include <ysclass.h>

#include "fsnetconfig.h"

#include "fs.h"
#include "fsfilename.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"
#include "fsconmenu.h"

#define FS_NOGUIDIALOG
#include "fschoose.h"


static int FsWaitForKeyStroke(void)
{
	printf(">");

	FsPollDevice();
	while(FSKEY_NULL!=FsInkey() || 0!=FsInkeyChar())
	{
		FsPollDevice();
	}

	int c;
	while((c=FsInkeyChar())==0)
	{
		FsPollDevice();
		FsSleep(25);
	}

	FsPollDevice();
	while(FSKEY_NULL!=FsInkey() || 0!=FsInkeyChar())
	{
		FsPollDevice();
	}

	if('a'<=c && c<='z')
	{
		c=c+'A'-'a';
	}

	printf("%c\n",c);

	return c;
}

static int FsLineInput(char str[])
{
	FsPollDevice();
	while(FSKEY_NULL!=FsInkey() || 0!=FsInkeyChar())
	{
		FsPollDevice();
	}

	int lStr=0;
	str[0]=0;

	printf(">");

	for(;;)
	{
		FsPollDevice();

		char c;
		while(0!=(c=FsInkeyChar()))
		{
			if(lStr<255)
			{
				str[lStr++]=c;
				str[lStr]=0;
				printf("%c",c);
			}
		}

		int fsKey;
		while(FSKEY_NULL!=(fsKey=FsInkey()))
		{
			switch(fsKey)
			{
			case FSKEY_ENTER:
				return lStr;
			case FSKEY_ESC:
				printf("\r                            ");
				printf("\r>");
				lStr=0;
				str[0]=0;
				break;
			case FSKEY_BS:
			case FSKEY_DEL:
				if(lStr>0)
				{
					lStr--;
					str[lStr]=0;
					printf("\r>%s ",str);
				}
				break;
			}
		}
	}
	return lStr;
}



void FsConMenu(const FsCommandParameter &fscp,FsWorld *world)
{
	for(;;)
	{
		printf("\n\n\n\n\n");
		printf("- Main Menu -\n");
		printf("[1]...Start Server\n");
		printf("[2]...Config Server\n");
		printf("[3]...Exit\n");

		switch(FsWaitForKeyStroke())
		{
		case '1':
			FsConStartServer(fscp,world);
			break;
		case '2':
			FsConConfigServer(world);
			break;
		case '3':
			return;
		}
	}
}

YSRESULT FsConChooseField(char fldName[],FsWorld *world)
{

	int curPos,nPerPage;

	curPos=0;
	nPerPage=9;

	for(;;)
	{
		int i;
		YSBOOL nextPage;
		nextPage=YSTRUE;
		for(i=0; i<nPerPage; i++)
		{
			char nam[256];
			if(world->GetFieldTemplateName(nam,i+curPos)==YSOK)
			{
				printf("[%d]...%s\n",i+1,nam);
			}
			else
			{
				nextPage=YSFALSE;
				break;
			}
		}

		if(curPos>0)
		{
			printf("[P] ...Previous Page\n");
		}
		if(nextPage==YSTRUE)
		{
			printf("[N] ...Next Page\n");
		}
		printf("[X] ...Cancel Selection\n");


		int cmd=FsWaitForKeyStroke();
		if(cmd=='p' || cmd=='P')
		{
			curPos-=nPerPage;
			if(curPos<0)
			{
				curPos=0;
			}
		}
		else if(cmd=='n' || cmd=='N')
		{
			if(world->GetFieldTemplateName(curPos+nPerPage)!=NULL)
			{
				curPos+=nPerPage;
			}
		}
		else if(cmd=='x' || cmd=='X')
		{
			return YSERR;
		}
		else
		{
			cmd-='0';
			if(1<=cmd && cmd<=nPerPage)
			{
				if(world->GetFieldTemplateName(fldName,curPos+cmd-1)==YSOK)
				{
					return YSOK;
				}
			}
		}
	}
	return YSERR;
}

void FsConStartServer(const class FsCommandParameter &fscp,FsWorld *world)
{
	char fieldName[256];
	// FsClearConsoleWindow();
	if(FsConChooseField(fieldName,world)==YSOK)
	{
		// FsClearConsoleWindow();
		FsNetConfig netcfg;
		netcfg.Load(FsGetNetConfigFile());
		printf("Field=%s\n",(const char *)fieldName);
		NetworkHostFlightAuto("Console Server",fieldName,netcfg.portNumber,fscp);
	}
}


static const char *const idxToKey[]=
{
	"[0]","[1]","[2]","[3]","[4]","[5]","[6]","[7]","[8]","[9]",
	"[A]","[B]","[C]","[D]","[E]","[F]","[G]","[H]","[I]","[J]",
	"[K]","[L]","[M]","[N]","[O]","[P]","[Q]","[R]","[S]","[T]",
	"[U]","[V]","[W]","[X]","[Y]","[Z]"
};

static void FsConPrintItemBool(int x,int y,int csr,int idx,const char label[],YSBOOL &tf,int fsKey)
{
	printf("%s...%-30s[%s]\n",idxToKey[idx],label,(tf==YSTRUE ? "X" : " "));
}

static void FsConPrintLogOnTimeOut(int x,int y,int csr,int idx,int &logOnTimeOut,int fsKey)
{
	if(logOnTimeOut==0)
	{
		printf("%s...Log On Time Out             [N/A]\n",idxToKey[idx]);
	}
	else
	{
		printf("%s...Log On Time Out             [%3d]\n",idxToKey[idx],logOnTimeOut);
	}
}

static void FsConPrintMultiConnFromSameIP(int x,int y,int csr,int idx,int &multiConnLimit,int fsKey)
{
	if(multiConnLimit==0)
	{
		printf("%s...Connection from same IP [NoLimit]\n",idxToKey[idx]);
	}
	else
	{
		printf("%s...Connection from same IP     [%3d]\n",idxToKey[idx],multiConnLimit);
	}
}

static void FsConPrintServerControl(int x,int y,int csr,int idx,const char *label,int &ctrl,int fsKey)
{
	const char *fmt="%s...%-19s%[%12s]\n";

	switch(ctrl)
	{
	case 0:
		printf(fmt,idxToKey[idx],label,"Up to Client");
		break;
	case 1:
		printf(fmt,idxToKey[idx],label,"Enable");
		break;
	case 2:
		printf(fmt,idxToKey[idx],label,"Diable");
		break;
	case 3:
		printf(fmt,idxToKey[idx],label,"Same as Svr");
		break;
	}
}

static void FsConPrintShowUserName(int x,int y,int csr,int idx,int &ctrl,int fsKey)
{
	const char *fmt="%s...Show User Name     [%12s]\n";

	if(idx==csr)
	{
		if(fsKey==FSKEY_SPACE || fsKey==FSKEY_ENTER)
		{
			switch(ctrl)
			{
			case 1:
				ctrl=2;
				break;
			case 2:
				ctrl=1000;
				break;
			case 1000:
				ctrl=2000;
				break;
			case 2000:
				ctrl=4000;
				break;
			default:
				ctrl=1;
				break;
			}
		}
	}

	switch(ctrl)
	{
	default:
		printf(fmt,idxToKey[idx],"");
		break;
	case 1:
		printf(fmt,idxToKey[idx],"Always");
		break;
	case 2:
		printf(fmt,idxToKey[idx],"Never");
		break;
	case 1000:
		printf(fmt,idxToKey[idx],"Within 1000m");
		break;
	case 2000:
		printf(fmt,idxToKey[idx],"Within 2000m");
		break;
	case 4000:
		printf(fmt,idxToKey[idx],"Within 4000m");
		break;
	}
}

static void FsConPrintResetServerEvery(int x,int y,int csr,int idx,unsigned int &resetTime,int fsKey)
{
	const char *fmt="%s...Reset Server Every [%12s]\n";

	if(idx==csr)
	{
		if(fsKey==FSKEY_SPACE || fsKey==FSKEY_ENTER)
		{
			if(resetTime==0)
			{
				resetTime=60;
			}
			else if(resetTime<180)
			{
				resetTime=180;
			}
			else if(resetTime<360)
			{
				resetTime=360;
			}
			else if(resetTime<720)
			{
				resetTime=720;
			}
			else if(resetTime<1440)
			{
				resetTime=1440;
			}
			else if(resetTime<1440*3)
			{
				resetTime=1440*3;
			}
			else if(resetTime<1440*7)
			{
				resetTime=1440*7;
			}
			else
			{
				resetTime=0;
			}
		}
	}

	if(resetTime==0)
	{
		printf(fmt,idxToKey[idx],"Never");
	}
	else if(resetTime<180)
	{
		printf(fmt,idxToKey[idx],"1 hour");
	}
	else if(resetTime<360)
	{
		printf(fmt,idxToKey[idx],"3 hours");
	}
	else if(resetTime<720)
	{
		printf(fmt,idxToKey[idx],"6 hours");
	}
	else if(resetTime<1440)
	{
		printf(fmt,idxToKey[idx],"12 hours");
	}
	else if(resetTime<1440*3)
	{
		printf(fmt,idxToKey[idx],"24 hours");
	}
	else if(resetTime<1440*7)
	{
		printf(fmt,idxToKey[idx],"3 days");
	}
	else
	{
		printf(fmt,idxToKey[idx],"7 days");
	}
}

static void FsConPrintTerminateServerAfter(int x,int y,int csr,int idx,int &termSvrAfter,int fsKey)
{
	if(termSvrAfter==0)
	{
		printf("%s...Stop Server After      [   Never]\n",idxToKey[idx]);
	}
	else
	{
		printf("%s...Stop Server After      [%2d times]\n",idxToKey[idx],termSvrAfter);
	}
}

static void FsConPrintActionMenu(int x,int y,int csr,int idx,const char *label,char cmd)
{
	printf("[%c] ...%-33s\n",cmd,label);
}


static void FsInputBoolInConsole(YSBOOL &sw,const char label[])
{
	for(;;)
	{
		printf("Set \"%s\" (Now: %s)\n",label,(sw==YSTRUE ? "ON" : "OFF"));
		printf("[1]...ON\n");
		printf("[2]...OFF\n");
		printf("[Q]...Cancel\n");

		switch(FsWaitForKeyStroke())
		{
		case '1':
			sw=YSTRUE;
			return;
		case '2':
			sw=YSFALSE;
			return;
		case 'q':
		case 'Q':
			return;
		}
	}
}

static void FsInputLogOnTimeOut(int &logOnTimeOut)
{
	for(;;)
	{
		if(logOnTimeOut==0)
		{
			printf("Log-On Time Out (Now: No Time Out)\n");
		}
		else
		{
			printf("Log-On Time Out (Now: %d seconds)\n",logOnTimeOut);
		}

		printf("[0]...No Time Out\n");
		printf("[1]...15 seconds\n");
		printf("[2]...30 seconds\n");
		printf("[3]...60 seconds\n");
		printf("[4]...90 seconds\n");
		printf("[5]...120 seconds\n");
		printf("[Q]...Cancel\n");

		switch(FsWaitForKeyStroke())
		{
		case '0':
			logOnTimeOut=0;
			return;
		case '1':
			logOnTimeOut=15;
			return;
		case '2':
			logOnTimeOut=30;
			return;
		case '3':
			logOnTimeOut=60;
			return;
		case '4':
			logOnTimeOut=90;
			return;
		case '5':
			logOnTimeOut=120;
			return;
		case 'q':
		case 'Q':
			return;
		}
	}
}

void FsInputMultiConnectionLimit(int &multiConnLimit)
{
	for(;;)
	{
		if(multiConnLimit==0)
		{
			printf("Connection from same IP (Now: No Limit)\n");
		}
		else
		{
			printf("Connection from same IP (Now: %d)\n",multiConnLimit);
		}

		printf("[0]...No Limit\n");
		printf("[1]...1 Connection\n");
		printf("[2]...2 Connection\n");
		printf("[3]...3 Connection\n");
		printf("[4]...4 Connection\n");
		printf("[5]...5 Connection\n");
		printf("[Q]...Cancel\n");

		int cmd=FsWaitForKeyStroke();
		switch(cmd)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			multiConnLimit=cmd-'0';
			return;
		case 'q':
		case 'Q':
			return;
		}
	}
}

void FsInputShowUserName(int &ctrl)
{
	for(;;)
	{
		printf("Show User Name (Now: ");
		switch(ctrl)
		{
		case 1:
			printf("Always");
			break;
		case 2:
			printf("Never");
			break;
		case 1000:
			printf("Within 1000m");
			break;
		case 2000:
			printf("Within 2000m");
			break;
		case 4000:
			printf("Within 4000m");
			break;
		}
		printf(")\n");

		printf("[1]...Always\n");
		printf("[2]...Never\n");
		printf("[3]...Within 1000m\n");
		printf("[4]...Within 2000m\n");
		printf("[5]...Within 4000m\n");
		printf("[Q]...Cancel\n");

		switch(FsWaitForKeyStroke())
		{
		case '1':
			ctrl=1;
			return;
		case '2':
			ctrl=2;
			return;
		case '3':
			ctrl=1000;
			return;
		case '4':
			ctrl=2000;
			return;
		case '5':
			ctrl=4000;
			return;
		case 'q':
		case 'Q':
			return;
		}
	}
}

static void FsInputServerControl(int &ctrl,const char *label)
{
	for(;;)
	{
		printf("%s (Now: ",label);
		switch(ctrl)
		{
		case 0:
			printf("Up to Client");
			break;
		case 1:
			printf("Enable");
			break;
		case 2:
			printf("Diable");
			break;
		case 3:
			printf("Same as Server");
			break;
		}
		printf(")\n");

		printf("[1]...Up to Client\n");
		printf("[2]...Enable\n");
		printf("[3]...Disable\n");
		printf("[4]...Same as Server\n");
		printf("[Q]...Cancel\n");

		int cmd=FsWaitForKeyStroke();
		switch(cmd)
		{
		case '1':
		case '2':
		case '3':
		case '4':
			ctrl=cmd-'1';
			return;
		case 'q':
		case 'Q':
			return;
		}
	}
}

static void FsInputResetServerEvery(unsigned int &resetTime)
{
	for(;;)
	{
		printf("Reset Server (Now: ");
		if(resetTime==0)
		{
			printf("Never");
		}
		else if(resetTime<180)
		{
			printf("1 hour");
		}
		else if(resetTime<360)
		{
			printf("3 hours");
		}
		else if(resetTime<720)
		{
			printf("6 hours");
		}
		else if(resetTime<1440)
		{
			printf("12 hours");
		}
		else if(resetTime<1440*3)
		{
			printf("24 hours");
		}
		else if(resetTime<1440*7)
		{
			printf("3 days");
		}
		else
		{
			printf("7 days");
		}
		printf(")\n");

		printf("[0]...Never\n");
		printf("[1]...1 hour\n");
		printf("[2]...3 hours\n");
		printf("[3]...6 hours\n");
		printf("[4]...12 hours\n");
		printf("[5]...24 hours\n");
		printf("[6]...3 days\n");
		printf("[7]...7 days\n");
		printf("[Q]...Cancel\n");

		switch(FsWaitForKeyStroke())
		{
		case '0':
			resetTime=0;
			return;
		case '1':
			resetTime=60;
			return;
		case '2':
			resetTime=180;
			return;
		case '3':
			resetTime=360;
			return;
		case '4':
			resetTime=720;
			return;
		case '5':
			resetTime=1440;
			return;
		case '6':
			resetTime=1440*3;
			return;
		case '7':
			resetTime=1440*7;
			return;
		case 'q':
		case 'Q':
			return;
		}
	}
}

static void FsInputTerminateServerAfter(int &termSvrAfter)
{
	for(;;)
	{
		printf("Terminate Serve After N times (Now: ");
		if(termSvrAfter==0)
		{
			printf("Never");
		}
		else
		{
			printf("%d times",termSvrAfter);
		}
		printf("\n");

		printf("[0]...Never\n");
		printf("[1]...Once\n");
		printf("[2]...Twice\n");
		printf("[3]...3 times\n");
		printf("[4]...4 times\n");
		printf("[5]...5 times\n");
		printf("[6]...6 times\n");
		printf("[7]...7 times\n");
		printf("[8]...8 times\n");
		printf("[9]...9 times\n");
		printf("[Q]...Cancel\n");

		int cmd=FsWaitForKeyStroke();
		switch(cmd)
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			termSvrAfter=cmd-'0';
			return;
		case 'q':
		case 'Q':
			return;
		}
	}
}

void FsConConfigServer(FsWorld *world)
{
	int csr,idx,key;
	FsNetConfig netCfg;

	netCfg.Load(FsGetNetConfigFile());

	csr=0;
	key=0;
	for(;;)
	{
		idx=1;

		printf("- Network Configuration -\n");

		FsConPrintItemBool(0,0,csr,idx,"Record Flight",netCfg.recordWhenServerMode,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Free Memory when Possible",netCfg.freeMemoryWhenPossibleServerMode,key);
		idx++;
		FsConPrintLogOnTimeOut(0,0,csr,idx,netCfg.logOnTimeOut,key);
		idx++;
		FsConPrintMultiConnFromSameIP(0,0,csr,idx,netCfg.multiConnLimit,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Disable Chat",netCfg.serverDisableChat,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Welcome Message",netCfg.sendWelcomeMessage,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Use Missile",netCfg.useMissile,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Use Gun/Bomb/Rocket",netCfg.useUnguidedWeapon,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Disable Radar Gun Sight",netCfg.disableRadarGunSight,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Disable 3rd Airplane View",netCfg.disableThirdAirplaneView,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Notify Take Off/Leave",netCfg.sendJoinLeaveMessage,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Ground Fire",netCfg.groundFire,key);
		idx++;
		FsConPrintShowUserName(0,0,csr,idx,netCfg.serverControlShowUserName,key);
		idx++;
		FsConPrintServerControl(0,0,csr,idx,"Mid Air Collision",netCfg.serverControlMidAirCollision,key);
		idx++;
		FsConPrintServerControl(0,0,csr,idx,"Black Out",netCfg.serverControlBlackOut,key);
		idx++;
		FsConPrintServerControl(0,0,csr,idx,"Can Land Anywhere",netCfg.serverControlCanLandAnywhere,key);
		idx++;
		FsConPrintItemBool(0,0,csr,idx,"Control Radar Alt Limit",netCfg.serverControlRadarAlt,key);
		idx++;
		FsConPrintResetServerEvery(0,0,csr,idx,netCfg.serverResetTime,key);
		idx++;
		FsConPrintTerminateServerAfter(0,0,csr,idx,netCfg.endSvrAfterResetNTimes,key);
		idx++;


		FsConPrintActionMenu(0,0,csr,idx,"Set Default",'R');
		FsConPrintActionMenu(0,0,csr,idx,"Save and Exit",'X');
		FsConPrintActionMenu(0,0,csr,idx,"Exit without Saving Changes",'Q');



		switch(FsWaitForKeyStroke())
		{
		case '1': // RecordFlight
			FsInputBoolInConsole(netCfg.recordWhenServerMode,"Record Flight");
			break;
		case '2': // Free Memory
			FsInputBoolInConsole(netCfg.freeMemoryWhenPossibleServerMode,"Free Memory when Possible");
			break;
		case '3':// Log On Time Out
			FsInputLogOnTimeOut(netCfg.logOnTimeOut);
			break;
		case '4':// Connection from same IP
			FsInputMultiConnectionLimit(netCfg.multiConnLimit);
			break;
		case '5':// Disable chat
			FsInputBoolInConsole(netCfg.serverDisableChat,"Disable Chat");
			break;
		case '6':// Welcome message
			FsInputBoolInConsole(netCfg.sendWelcomeMessage,"Welcome Message");
			break;
		case '7':// Use Missile
			FsInputBoolInConsole(netCfg.useMissile,"Use Missile");
			break;
		case '8':// Use Gun/Bomb/Rocket
			FsInputBoolInConsole(netCfg.useUnguidedWeapon,"Use Gun/Bomb/Rocket");
			break;
		case '9':// Disable Radar Gun Sight
			FsInputBoolInConsole(netCfg.disableRadarGunSight,"Disable Radar Gun Sight");
			break;
		case 'A':// Disable 3rd airplane view
			FsInputBoolInConsole(netCfg.disableThirdAirplaneView,"Disable 3rd Airplane View");
			break;
		case 'B':// Notify Take Off/Leave
			FsInputBoolInConsole(netCfg.sendJoinLeaveMessage,"Notify Take Off/Leave");
			break;
		case 'C':// Ground Fire
			FsInputBoolInConsole(netCfg.groundFire,"Ground Fire");
			break;
		case 'D':// Show User Name
			FsInputShowUserName(netCfg.serverControlShowUserName);
			break;
		case 'E':// Mid Air Collision
			FsInputServerControl(netCfg.serverControlMidAirCollision,"Mid Air Collision");
			break;
		case 'F':// Black out
			FsInputServerControl(netCfg.serverControlBlackOut,"Black Out");
			break;
		case 'G':// Can Land Anywhere
			FsInputServerControl(netCfg.serverControlCanLandAnywhere,"Can Land Anywhere");
			break;
		case 'H':// Control Radar Alt Limit
			FsInputBoolInConsole(netCfg.serverControlRadarAlt,"Control Radar Alt Limit");
			break;
		case 'I':// Reset Server Every
			FsInputResetServerEvery(netCfg.serverResetTime);
			break;
		case 'J':// Stop Server After
			FsInputTerminateServerAfter(netCfg.endSvrAfterResetNTimes);
			break;


		case 'r':
		case 'R':
			netCfg.SetDefault();
			break;
		case 'x':
		case 'X':
			netCfg.Save(FsGetNetConfigFile());
			return;
		case 'q':
		case 'Q':
			return;
		}
	}

	FsClearConsoleWindow();
}
