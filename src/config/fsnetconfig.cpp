#include <ysclass.h>
#include <ysport.h>
#include <time.h>


#include "fsnetconfig.h"
#include "fsdef.h"


const char *const fsNetCmd[]=
{
	"USEMISSILE",
	"SERVERWAIT",
	"DEFAULTHST",
	"DEFAULTUSR",
	"RECORDCLNT",
	"RECORDSRVR",
	"DEFLTFIELD",
	"DFAIRPLANE",
	"DFSTPOSSVR",
	"DEFIFFSRVR",
	"DEFIFFCLNT",
	"DFSTPOSCLI",
	"GROUNDFIRE",
	"PORTNUMBER",
	"FREECLIENT",
	"FREESERVER",

	"SENDBLKOUT",
	"SENDLDAWHR",
	"SENDMIDAIR",

	"NOGUNSIGHT",
	"SAMEVERSIN",

	"SVRRESETTM",
	"ENTSVRAFTR",

	"USEUGWEAPN",
	"DISABLCHAT",

	"SHWUSRNAME",

	"SENDRDRALT", // Server Control Radar Altitude
	"NOEXTAIRVW", // Disable Third Airplane View

	"NOTIFYJOIN", // Send Join/Leave Message

	"WELCOMEMSG", // Send Welcome Message

	"SAVCHATLOG", // Save Chat Log

	"LOGONTMOUT", // Log on time out  2007/09/12
	"MLTCONNLMT", // Multi-Connection Limit  2007/09/12

	NULL
};

FsNetConfig::FsNetConfig()
{
	SetDefault();
};

void FsNetConfig::SetDefault(void)
{
	useMissile=YSTRUE;
	useUnguidedWeapon=YSTRUE;
	disableRadarGunSight=YSFALSE;
	serverWait=YSTRUE;
	defHost[0]=0;
	strcpy(defUser,"USERNAME");
	recordWhenClientMode=YSTRUE;
	freeMemoryWhenPossibleClientMode=YSFALSE;
	recordWhenServerMode=YSFALSE;
	freeMemoryWhenPossibleServerMode=YSFALSE;
	strcpy(defField,"AOMORI");

	strcpy(defStartPosClient,"SOUTH10000_01");
	strcpy(defStartPosServer,"NORTH10000_01");
	defIFFWhenClient=FS_IFF0;
	defIFFWhenServer=FS_IFF1;
	groundFire=YSTRUE;

	serverControlBlackOut=3;
	serverControlCanLandAnywhere=3;
	serverControlMidAirCollision=3;
	serverControlShowUserName=4000;

	serverAcceptSameVersionOnly=YSFALSE;
	serverDisableChat=YSFALSE;
	serverResetTime=0;
	endSvrAfterResetNTimes=0;

	logOnTimeOut=0;   // 0 -> No time out
	multiConnLimit=0; // 0 -> No connection limit from the same IP

	portNumber=FS_DEFAULT_NETWORK_PORT;

	serverControlRadarAlt=YSTRUE;     // FALSE:Don't Control  TRUE:Same as Server
	disableThirdAirplaneView=YSFALSE;

	sendJoinLeaveMessage=YSTRUE;
	sendWelcomeMessage=YSTRUE;

	saveChatLog=YSFALSE;
}

YSRESULT FsNetConfig::Load(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		char str[256],buf[256];
		int ac;
		char *av[16];
		YSRESULT res;

		res=YSOK;
		while(fgets(buf,256,fp)!=NULL)
		{
			strcpy(str,buf);
			if(YsArguments(&ac,av,16,buf)==YSOK && ac>0)
			{
				int cmd;
				if(YsCommandNumber(&cmd,av[0],fsNetCmd)==YSOK)
				{
					switch(cmd)
					{
					case 0: // "USEMISSILE",
						res=YSOK;
						useMissile=YsStrToBool(av[1]);
						break;
					case 1: // "SERVERWAIT",
						res=YSOK;
						serverWait=YsStrToBool(av[1]);
						break;
					case 2: // "DEFAULTHST",
						strcpy(defHost,av[1]);
						break;
					case 3: // "DEFAULTUSR",
						strcpy(defUser,av[1]);
						break;
					case 4: // "RECORDCLNT",
						res=YSOK;
						recordWhenClientMode=YsStrToBool(av[1]);
						break;
					case 5: // "RECORDSRVR",
						res=YSOK;
						recordWhenServerMode=YsStrToBool(av[1]);
						break;
					case 6: // "DEFLTFIELD",
						strcpy(defField,av[1]);
						break;
					case 7: // "DFAIRPLANE",
						// strcpy(defAirplane,av[1]);
						break;
					case 8: // "DFSTARTPOS",
						strcpy(defStartPosServer,av[1]);
						break;
					case 9: // "DEFIFFSVR",
						defIFFWhenServer=atoi(av[1]);
						break;
					case 10: // "DEFIFFCLI",
						defIFFWhenClient=atoi(av[1]);
						break;
					case 11: // "DFSTPOSCLI"
						strcpy(defStartPosClient,av[1]);
						break;
					case 12: // "GROUNDFIRE"
						res=YSOK;
						groundFire=YsStrToBool(av[1]);
						break;
					case 13: // "PORTNUMBER"
						res=YSOK;
						portNumber=atoi(av[1]);
						break;
					case 14: // "FREECLIENT",
						res=YSOK;
						freeMemoryWhenPossibleClientMode=YsStrToBool(av[1]);
						break;
					case 15: // "FREESERVER",
						res=YSOK;
						freeMemoryWhenPossibleServerMode=YsStrToBool(av[1]);
						break;

					case 16: // "SENDBLKOUT",
						if('0'<=av[1][0] && av[1][0]<='9')
						{
							serverControlBlackOut=atoi(av[1]);
						}
						else
						{
							YSBOOL tf;
							res=YSOK;
							tf=YsStrToBool(av[1]);
							if(res==YSOK)
							{
								serverControlBlackOut=(tf==YSTRUE ? 3 : 0);
							}
						}
						break;
					case 17: // "SENDLDAWHR",
						if('0'<=av[1][0] && av[1][0]<='9')
						{
							serverControlCanLandAnywhere=atoi(av[1]);
						}
						else
						{
							YSBOOL tf;
							res=YSOK;
							tf=YsStrToBool(av[1]);
							if(res==YSOK)
							{
								serverControlCanLandAnywhere=(tf==YSTRUE ? 3 : 0);
							}
						}
						break;
					case 18: // "SENDMIDAIR",
						if('0'<=av[1][0] && av[1][0]<='9')
						{
							serverControlMidAirCollision=atoi(av[1]);
						}
						else
						{
							YSBOOL tf;
							res=YSOK;
							tf=YsStrToBool(av[1]);
							if(res==YSOK)
							{
								serverControlMidAirCollision=(tf==YSTRUE ? 3 : 0);
							}
						}
						break;

					case 19: // "NOGUNSIGHT",
						res=YSOK;
						disableRadarGunSight=YsStrToBool(av[1]);
						break;
					case 20: //"SAMEVERSIN",
						res=YSOK;
						serverAcceptSameVersionOnly=YsStrToBool(av[1]);
						break;

					case 21: //	"SVRRESETTM",
						serverResetTime=atoi(av[1]);
						break;

					case 22: // "ENTSVRAFTR",
						endSvrAfterResetNTimes=atoi(av[1]);
						break;

					case 23: // "USEUGWEAPN",
						res=YSOK;
						useUnguidedWeapon=YsStrToBool(av[1]);
						break;

					case 24: // "DISABLCHAT",
						res=YSOK;
						serverDisableChat=YsStrToBool(av[1]);
						break;

					case 25: // "SHWUSRNAME",
						serverControlShowUserName=atoi(av[1]);
						break;

					case 26: // "SENDRDRALT", // Server Control Radar Altitude
						res=YSOK;
						serverControlRadarAlt=YsStrToBool(av[1]);
						break;

					case 27: // "NOEXTAIRVW", // Disable Third Airplane View
						res=YSOK;
						disableThirdAirplaneView=YsStrToBool(av[1]);
						break;

					case 28: // "NOTIFYJOIN"
						res=YSOK;
						sendJoinLeaveMessage=YsStrToBool(av[1]);
						break;

					case 29: // "WELCOMEMSG"
						res=YSOK;
						sendWelcomeMessage=YsStrToBool(av[1]);
						break;

					case 30: // "SAVCHATLOG"
						res=YSOK;
						saveChatLog=YsStrToBool(av[1]);
						break;

					case 31: // "LOGONTMOUT"
						logOnTimeOut=atoi(av[1]);
						res=YSOK;
						break;
					case 32: // "MLTCONNLMT", // Multi-Connection Limit
						multiConnLimit=atoi(av[1]);
						res=YSOK;
						break;

					default:
						res=YSERR;
						break;
					}

					if(res!=YSOK)
					{
						//fsStderr.Printf("Unrecognized command [%s]\n",str);
						fclose(fp);
						return YSERR;
					}
				}
			}
		}
		fclose(fp);
		return YSOK;
	}
	else
	{
		SetDefault();
		return YSERR;
	}
}

YSRESULT FsNetConfig::Save(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"w");
	if(fp!=NULL)
	{
		fprintf(fp,"USEMISSILE %s\n",YsBoolToStr(useMissile));
		fprintf(fp,"SERVERWAIT %s\n",YsBoolToStr(serverWait));
		fprintf(fp,"DEFAULTHST \"%s\"\n",defHost);
		fprintf(fp,"DEFAULTUSR \"%s\"\n",defUser);
		fprintf(fp,"RECORDCLNT %s\n",YsBoolToStr(recordWhenClientMode));
		fprintf(fp,"RECORDSRVR %s\n",YsBoolToStr(recordWhenServerMode));
		fprintf(fp,"DEFLTFIELD \"%s\"\n",defField);
		// fprintf(fp,"DFAIRPLANE \"%s\"\n",defAirplane);
		fprintf(fp,"DFSTPOSSVR \"%s\"\n",defStartPosServer);
		fprintf(fp,"DEFIFFSRVR %d\n",defIFFWhenServer);
		fprintf(fp,"DEFIFFCLNT %d\n",defIFFWhenClient);
		fprintf(fp,"DFSTPOSCLI \"%s\"\n",defStartPosClient);
		fprintf(fp,"GROUNDFIRE %s\n",YsBoolToStr(groundFire));
		fprintf(fp,"PORTNUMBER %d\n",portNumber);

		fprintf(fp,"FREECLIENT %s\n",YsBoolToStr(freeMemoryWhenPossibleClientMode));
		fprintf(fp,"FREESERVER %s\n",YsBoolToStr(freeMemoryWhenPossibleServerMode));

		fprintf(fp,"SENDBLKOUT %d\n",serverControlBlackOut);
		fprintf(fp,"SENDLDAWHR %d\n",serverControlCanLandAnywhere);
		fprintf(fp,"SENDMIDAIR %d\n",serverControlMidAirCollision);

		fprintf(fp,"NOGUNSIGHT %s\n",YsBoolToStr(disableRadarGunSight));
		fprintf(fp,"SAMEVERSIN %s\n",YsBoolToStr(serverAcceptSameVersionOnly));

		fprintf(fp,"SVRRESETTM %dmin\n",serverResetTime);
		fprintf(fp,"ENTSVRAFTR %d\n",endSvrAfterResetNTimes);

		fprintf(fp,"USEUGWEAPN %s\n",YsBoolToStr(useUnguidedWeapon));
		fprintf(fp,"DISABLCHAT %s\n",YsBoolToStr(serverDisableChat));

		fprintf(fp,"SHWUSRNAME %d\n",serverControlShowUserName);

		fprintf(fp,"SENDRDRALT %s\n",YsBoolToStr(serverControlRadarAlt));
		fprintf(fp,"NOEXTAIRVW %s\n",YsBoolToStr(disableThirdAirplaneView));

		fprintf(fp,"NOTIFYJOIN %s\n",YsBoolToStr(sendJoinLeaveMessage));

		fprintf(fp,"WELCOMEMSG %s\n",YsBoolToStr(sendWelcomeMessage));

		fprintf(fp,"SAVCHATLOG %s\n",YsBoolToStr(saveChatLog));

		fprintf(fp,"LOGONTMOUT %d\n",logOnTimeOut);
		fprintf(fp,"MLTCONNLMT %d\n",multiConnLimit);

		fclose(fp);
		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

