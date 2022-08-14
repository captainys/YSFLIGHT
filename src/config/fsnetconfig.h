

#ifndef FSNETCONFIG_IS_INCLUDED
#define FSNETCONFIG_IS_INCLUDED
/* { */


////////////////////////////////////////////////////////////


class FsNetConfig
{
public:
	YSBOOL useMissile,useUnguidedWeapon;
	YSBOOL disableRadarGunSight;
	YSBOOL serverWait;
	char defHost[256];
	char defUser[256];
	YSBOOL recordWhenClientMode;
	YSBOOL freeMemoryWhenPossibleClientMode;
	YSBOOL recordWhenServerMode;
	YSBOOL freeMemoryWhenPossibleServerMode;

	YSBOOL saveChatLog;

	int serverControlBlackOut;        // 0:Dont' Control  1:Enable  2:Disable  3:Same as server
	int serverControlCanLandAnywhere; // 0:Dont' Control  1:Enable  2:Disable  3:Same as server
	int serverControlMidAirCollision; // 0:Dont' Control  1:Enable  2:Disable  3:Same as server
	int serverControlShowUserName;    // 0:Don't Control  1:Enable  2:Disable  3:Same as server  n>3:Show when within n meters
	YSBOOL serverControlRadarAlt;     // FALSE:Don't Control  TRUE:Same as Server
	YSBOOL disableThirdAirplaneView;
	YSBOOL sendJoinLeaveMessage;
	YSBOOL sendWelcomeMessage;
	int logOnTimeOut;
	int multiConnLimit;


	YSBOOL serverAcceptSameVersionOnly;
	YSBOOL serverDisableChat;
	unsigned int serverResetTime;   // Minutes
	int endSvrAfterResetNTimes;

	char defField[256];
	char defStartPosClient[256];
	char defStartPosServer[256];
	int defIFFWhenClient;
	int defIFFWhenServer;
	YSBOOL groundFire;

	int portNumber;



	FsNetConfig();
	void SetDefault(void);
	YSRESULT Load(const wchar_t fn[]);
	YSRESULT Save(const wchar_t fn[]);
};



/* } */
#endif
