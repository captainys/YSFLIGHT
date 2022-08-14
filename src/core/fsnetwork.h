#ifndef FSNETWORK_IS_INCLUDED
#define FSNETWORK_IS_INCLUDED
/* { */

#include "queue"

#include "fsdef.h"
#include "yssocket.h"
#include "fsutil.h"
#include "fsweapon.h"

enum
{
	FSNETERR_NOERR,
	FSNETERR_VERSIONCONFLICT,
	FSNETERR_CANNOTADDOBJECT,
	FSNETERR_REJECT,
	FSNETERR_CANNOTSUSTAIN
};

enum
{
	FSNETREADBACK_ADDAIRPLANE,
	FSNETREADBACK_ADDGROUND,
	FSNETREADBACK_REMOVEAIRPLANE,
	FSNETREADBACK_REMOVEGROUND,
	FSNETREADBACK_ENVIRONMENT,
	FSNETREADBACK_JOINREQUEST,
	FSNETREADBACK_JOINAPPROVAL,
	FSNETREADBACK_PREPARE,
	FSNETREADBACK____UNUSED____,
	FSNETREADBACK_USEMISSILE,
	FSNETREADBACK_USEUNGUIDEDWEAPON,
	FSNETREADBACK_CTRLSHOWUSERNAME
};

enum
{
	FSNETCMD_NULL,                   //   0
	FSNETCMD_LOGON,                  //   1 Cli ->Svr,  (Svr->Cli for log-on complete acknowledgement.)
	FSNETCMD_LOGOFF,                 //   2
	FSNETCMD_ERROR,                  //   3
	FSNETCMD_LOADFIELD,              //   4 Svr ->Cli,   Cli->Svr for read back
	FSNETCMD_ADDOBJECT,              //   5 Svr ->Cli
	FSNETCMD_READBACK,               //   6 Svr<->Cli
	FSNETCMD_SMOKECOLOR,             //   7 Svr<->Cli
	FSNETCMD_JOINREQUEST,            //   8 Svr<- Cli
	FSNETCMD_JOINAPPROVAL,           //   9 Svr ->Cli
	FSNETCMD_REJECTJOINREQ,          //  10
	FSNETCMD_AIRPLANESTATE,          //  11 Svr<->Cli   // Be careful in FsDeleteOldStatePacket when modify
	FSNETCMD_UNJOIN,                 //  12 Svr<- Cli
	FSNETCMD_REMOVEAIRPLANE,         //  13 Svr<->Cli
	FSNETCMD_REQUESTTESTAIRPLANE,    //  14
	FSNETCMD_KILLSERVER,             //  15 Svr<- Cli
	FSNETCMD_PREPARESIMULATION,      //  16 Svr ->Cli
	FSNETCMD_TESTPACKET,             //  17
	FSNETCMD_LOCKON,                 //  18 Svr<->Cli
	FSNETCMD_REMOVEGROUND,           //  19 Svr<->Cli
	FSNETCMD_MISSILELAUNCH,          //  20 Svr<->Cli   // fsweapon.cpp is responsible for encoding/decoding
	FSNETCMD_GROUNDSTATE,            //  21 Svr<->Cli   // Be careful in FsDeleteOldStatePacket when modify
	FSNETCMD_GETDAMAGE,              //  22 Svr<->Cli
	FSNETCMD_GNDTURRETSTATE,         //  23 Svr<->Cli
	FSNETCMD_SETTESTAUTOPILOT,       //  24 Svr ->Cli
	FSNETCMD_REQTOBESIDEWINDOWOFSVR, //  25 Svr<- Cli
	FSNETCMD_ASSIGNSIDEWINDOW,       //  26 Svr ->Cli
	FSNETCMD_RESENDAIRREQUEST,       //  27 Svr<- Cli
	FSNETCMD_RESENDGNDREQUEST,       //  28 Svr<- Cli
	FSNETCMD_VERSIONNOTIFY,          //  29 Svr ->Cli
	FSNETCMD_AIRCMD,                 //  30 Svr<->Cli   // After 2001/06/24
	FSNETCMD_USEMISSILE,             //  31 Svr ->Cli   // After 2001/06/24
	FSNETCMD_TEXTMESSAGE,            //  32 Svr<->Cli
	FSNETCMD_ENVIRONMENT,            //  33 Svr<->Cli  (*1)
	FSNETCMD_NEEDRESENDJOINAPPROVAL, //  34 Svr<- Cli
	FSNETCMD_REVIVEGROUND,           //  35 Svr ->Cli   // After 2004
	FSNETCMD_WEAPONCONFIG,           //  36 Svr<->Cli   // After 20040618
	FSNETCMD_LISTUSER,               //  37 Svr<->Cli   // After 20040726
	FSNETCMD_QUERYAIRSTATE,          //  38 Cli ->Svr   // After 20050207
	FSNETCMD_USEUNGUIDEDWEAPON,      //  39 Svr ->Cli   // After 20050323
	FSNETCMD_AIRTURRETSTATE,         //  40 Svr<->Cli   // After 20050701
	FSNETCMD_CTRLSHOWUSERNAME,       //  41 Svr ->Cli   // After 20050914
	FSNETCMD_CONFIRMEXISTENCE,       //  42 Not Used
	FSNETCMD_CONFIGSTRING,           //  43 Svr ->Cli   // After 20060514    Cli->Svr for read back
	FSNETCMD_LIST,                   //  44 Svr ->Cli   // After 20060514    Cli->Svr for read back
	FSNETCMD_GNDCMD,                 //  45 Svr<->Cli
	FSNETCMD_REPORTSCORE,            //  46 Svr -> Cli  // After 20100630    (Older version will ignore)
	FSNETCMD_SERVER_FORCE_JOIN,      //  47 Svr -> Cli
	FSNETCMD_FOGCOLOR,               //  48 Svr -> Cli
	FSNETCMD_SKYCOLOR,               //  49 Svr -> Cli
	FSNETCMD_GNDCOLOR,               //  50 Svr -> Cli
	FSNETCMD_RESERVED_FOR_LIGHTCOLOR,//  51 Svr -> Cli
	FSNETCMD_RESERVED21,             //  52
	FSNETCMD_RESERVED22,             //  53
	FSNETCMD_RESERVED23,             //  54
	FSNETCMD_RESERVED24,             //  55
	FSNETCMD_RESERVED25,             //  56
	FSNETCMD_RESERVED26,             //  57
	FSNETCMD_RESERVED27,             //  58
	FSNETCMD_RESERVED28,             //  59
	FSNETCMD_RESERVED29,             //  60
	FSNETCMD_RESERVED30,             //  61
	FSNETCMD_RESERVED31,             //  62
	FSNETCMD_RESERVED32,             //  63
	FSNETCMD_OPENYSF_RESERVED33,     //  64 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED34,     //  65 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED35,     //  66 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED36,     //  67 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED37,     //  68 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED38,     //  69 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED39,     //  70 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED40,     //  71 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED41,     //  72 Reserved for OpenYSF
	FSNETCMD_OPENYSF_RESERVED42,     //  73 Reserved for OpenYSF
	FSNETCMD_RESERVED43,             //  74
	FSNETCMD_RESERVED44,             //  75
	FSNETCMD_RESERVED45,             //  76
	FSNETCMD_RESERVED46,             //  77
	FSNETCMD_RESERVED47,             //  78
	FSNETCMD_RESERVED48,             //  79
	FSNETCMD_RESERVED49,             //  80
	FSNETCMD_NOP
};

// (*1)
// FSNETCMD_ENVIRONMENT
//   Cli->Svr Request environment
//   Svr->Cli Report environment
//     The request will be sent whenever a client receives FSNETCMD_LOADFIELD



class FsNetworkFldToSend
{
public:
	YsVec3 pos;
	YsAtt3 att;
	YsString fldName;
};

class FsNetworkScoreLog
{
public:
	int scoredClientId;  // -2:Non-Player  -1:Server
	unsigned int scoredKey;
	YsString scoredUsername;
	YsString scoredVehicleType;
	int killedClientId;  // -2:Non-Player  -1:Server
	unsigned int killedKey;
	YsString killedUsername;
	YsString killedVehicleType;
	FSWEAPONTYPE weaponType;
	YsVec3 pos;
	double scoreTime;

	void FormatScoredMessage(YsString &msg);
	void FormatKilledMessage(YsString &msg);
};

class FsNetworkUser
{
public:
	FSUSERSTATE state;
	time_t logOnTimeOut;
	double noActivityTime;

	YsString username;
	unsigned int ipAddr[4];
	class FsAirplane *air;

	enum
	{
		COMBUFSIZE=8192,
		SENDQUEUESIZE=8192
	};
	YSSIZE_T nComBuf;
	unsigned char comBuf[COMBUFSIZE];   // Store commands that are not completed in "Received"

	unsigned version;

	YSSIZE_T nSendQueueFilled;
	unsigned char sendQueue[SENDQUEUESIZE];

	// Variables should be initialized in ConnectionAccepted()
	double sendCriticalInfoTimer;
	YsArray <YsString> configStringToSend;
	YsArray <FsNetworkFldToSend> fldToSend;
	YsArray <YsString> airTypeToSend;
	YsArray <unsigned int> gndToSend,airToSend;
	YSBOOL useMissileReadBack,useUnguidedWeaponReadBack,controlShowUserNameReadBack;
	YSBOOL environmentReadBack,preparationReadBack;

	YsArray <unsigned int> gndRmvToSend,airRmvToSend;
	int joinSequence;  // 0:Not in process  1:Waiting for Airplane Read Back  2:Waiting for SetPlayer Read Back
	                   // 3:Waiting for Approval Read Back
	                   // For Future extension
	                   // 11:Waiting for Ground Read Back  12:Waiting for SetPlayer Ground Read Back
	                   // 13:Waiting for Approval Read Back

	void Initialize(void);
	void AddToSendQueue(YSSIZE_T nByte,unsigned char dat[]);
};



class FsNetReceivedAirplaneState
{
public:
	void Decode(const unsigned char dat[],const double &tLocal);

	double tLocal;
	double tRemote;

	double x,y,z,h,p,b;
	double vx,vy,vz,vh,vp,vb;
	double g;
	unsigned gun,aam,agm,bomb;
	double smokeOil;
	double fuel,payload;
	unsigned life;
	FSFLIGHTSTATE state;
	double vgw,spoiler,ldg,flap,brake;
	unsigned flags;
	YSBOOL ab,firingGun;
	unsigned char trailingSmokeFlag;
	YSBOOL navLight,beacon,strobe,landingLight;
	double throttle,elevator,aileron,rudder,elvTrim;
	unsigned rocket;
	double thrVector,thrReverser,bombBay;
};

enum FSNET_CONSOLE_COMMAND
{
	FSNCC_COMMON_NULL,
	FSNCC_COMMON_PRINTMENU,
	FSNCC_COMMON_JOIN,
	FSNCC_COMMON_IFF1,
	FSNCC_COMMON_IFF2,
	FSNCC_COMMON_IFF3,
	FSNCC_COMMON_IFF4,
	FSNCC_COMMON_OBSERVERMODE,
	FSNCC_COMMON_LISTPLAYER,
	FSNCC_COMMON_ESC,
	FSNCC_COMMON_QUIT,
	FSNCC_COMMON_SELECTAIR,
	FSNCC_COMMON_SELECTSTP,
	FSNCC_COMMON_INLINECHAT,
	FSNCC_COMMON_WHOKILLEDME,
	FSNCC_COMMON_WHOMHAVEIKILLED,

	FSNCC_SVR_TOGGLELOCK,
	FSNCC_SVR_TOGGLEJOINLOCK,
	FSNCC_SVR_DISPELUSER,
	FSNCC_SVR_STARTINTERCEPTMISSION,
	FSNCC_SVR_STARTENDURANCEMODE_JET,
	FSNCC_SVR_STARTENDURANCEMODE_WW2,
	FSNCC_SVR_STARTCLOSEAIRSUPPORTMISSION,
	FSNCC_SVR_TERMINATEMISSION,
	FSNCC_SVR_REVIVEGROUND,

	FSNCC_CLI_SVRCOCKPIT,
	FSNCC_CLI_SVRLEFTVIEW,
	FSNCC_CLI_SVRRIGHTVIEW,
	FSNCC_CLI_SVRREARVIEW
};

class FsNetworkVariable
{
public:
	enum
	{
		CHOOSING_NOTHING,
		CHOOSING_AIRCRAFT,
		CHOOSING_STARTPOS,
	};

	YSBOOL quit;
	int escCount;
	FSIFF iff;
	YsString username;
	std::queue <FSNET_CONSOLE_COMMAND> commandQueue;

	class FsSimulation *sim;

	YsArray <FsNetworkScoreLog> killed,scored;  // Score log player.

	FsNetworkVariable();
	void Initialize(void);
};

class FsServerVariable : public FsNetworkVariable
{
public:
	enum
	{
		SVRSTATE_STANDBY,
		SVRSTATE_FLYING,
		SVRSTATE_DISPELLUSER,
		SVRSTATE_GHOSTVIEW,
	};

	int serverState;     // 0:Stand by    1:Flying   2:Dispell User   3:Ghost mode
	int choosingMode;    // 0:Not choosing anything    1:Choose airplane    2:Choose start position
	class FsFlightConfig *cfg;
	class FsInterceptMissionInfo *imInfo;

	double enduranceModeRemainingTime,gLimit;
	int nEnduranceModeEnemyMax;
	YSBOOL enduranceModeJet,enduranceModeWw2;

	double baseDefenseModeRemainingTime;
	double closeAirSupportMissionRemainingTime;
	double netNoActivityTime;

	YsString welcomeMessage;

	double timeToBroadcastAirplane,timeToBroadcastGround;
	mutable double nextConsoleUpdateTime;

	class FsNetConfig *netcfg;

	class FsAutoCloseFile chatLogFp;

	int nBroadcastGndCount;

	YSBOOL locked; //  YSTRUE->Not allow to log on   YSFALSE->Allow to log on
	YSBOOL joinLocked;  // YSTRUE->Not allow to join  YSFALSE->Allow to join
	YSBOOL receivedKillServer;

protected:
	enum
	{
		MAXNUMMESSAGE=16
	};
	int nMsg;
	char msg[MAXNUMMESSAGE][256];

	FsNetworkUser *user;

public:
	FsServerVariable(const char name[],FsSimulation *sim,class FsNetConfig *netcfg);
	~FsServerVariable();
	YSRESULT LoadWelcomeMessage(const wchar_t fn[]);
	YSRESULT Join(const char startPos[]);
};

class FsSocketServer : public YsSocketServer, public FsServerVariable
{
public:
	FsSocketServer(const char username[],class FsSimulation *sim,int netPort,class FsNetConfig *cfg);
	~FsSocketServer();

	class FsAirplane *ServerPlayerPlane();

	YSRESULT CheckPendingUser(const double &currentTime);
	YSRESULT CheckAndSendPendingData(int clientId,const double &currentTime,const double &interval);

	YSRESULT DisconnectUser(int clientId);

	YSRESULT ReceivedFrom(int clientId,YSSIZE_T nBytes,unsigned char dat[]);
	YSRESULT SendTerminateMessage(int clientId,unsigned timeout);
	YSRESULT ConnectionAccepted(int clientId,unsigned int ipAddr[]);
	YSRESULT ConnectionClosedByClient(int clientId);
	YSRESULT DisconnectInactiveUser(const double &passedTime,const double &timeOut);

	YSRESULT BroadcastAirplaneState(void);
	YSRESULT BroadcastGroundState(void);
	YSRESULT BroadcastReviveGround(void);
	YSRESULT BroadcastAddAirplane(const FsAirplane *air,FSNETTYPE netType);
	YSRESULT BroadcastRemoveAirplane(int airId,YSBOOL explosion);
	YSRESULT BroadcastAddGround(class FsGround *air,FSNETTYPE netType);
	YSRESULT BroadcastRemoveGround(int airId,YSBOOL explosion);
	YSRESULT BroadcastStateChange(void);
	YSRESULT BroadcastLockOn(int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir);
	YSRESULT BroadcastMissileLaunch(FsWeaponRecord &rec);
	YSRESULT BroadcastGetDamage(class FsExistence *victim,FsExistence *firedBy,int power,enum FSDIEDOF diedOf);
	YSRESULT BroadcastAirCmd(int airId,const char cmd[]);
	YSRESULT BroadcastChatTextMessage(const char txt[]);
	YSRESULT BroadcastTextMessage(const char txt[]);
	YSRESULT BroadcastWeaponConfig(int airId,YSSIZE_T nWeaponConfig,const int weaponConfig[]);
	YSRESULT BroadcastSkyColor(YsColor col);
	YSRESULT BroadcastGroundColor(YsColor col);
	YSRESULT BroadcastFogColor(YsColor col);
	YSRESULT BroadcastForceJoin(void);

	YSRESULT RectifyIllegalMissiles(void);

	YSBOOL ReceivedKillServer(void);

	int GetNumMessage(void) const;
	const char *GetMessage(int i) const;
	void ClearMessage(void);

	YSRESULT AddMessage(const char *msg);
	void ReportKill(FsExistence *killed,FsExistence *scored,FSWEAPONTYPE wpnType);

	YSRESULT ReceiveLogOnUser(int clientId,int version,const char username[]);
	YSRESULT ReceiveError(int clientId,int errorCode);
	YSRESULT ReceiveLoadFieldReadBack(int clientId,unsigned char dat[]);
	YSRESULT ReceiveConfigStringReadBack(int clientId,unsigned char dat[]);
	YSRESULT ReceiveListReadBack(int clientId,unsigned char dat[]);
	YSRESULT ReceiveJoinRequest(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveAirplaneState(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveUnjoin(int clientId,unsigned char dat[]);
	YSRESULT ReceiveRequestTestAirplane(int clientId,unsigned char dat[]);
	YSRESULT ReceiveKillServer(int clientId,unsigned char dat[]);
	YSRESULT ReceiveLockOn(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveRemoveAirplane(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveRemoveGround(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveMissileLaunch(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveGroundState(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveGetDamage(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveRequestToBeSideWindow(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveResendAirRequest(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveResendGndRequest(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveAirCmd(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveGndCmd(int clientId,unsigned char dat[],unsigned packetLength);
	YSRESULT ReceiveTextMessage(int clientId,unsigned char cmdTop[],unsigned packetLength);
	YSRESULT ReceiveEnvironmentRequest(int clientId,unsigned char cmdTop[],unsigned packetLength);
	YSRESULT ReceivedResendJoinApproval(int clientId,unsigned char cmdTop[],unsigned packetLength);
	YSRESULT ReceiveReviveGround(void);
	YSRESULT ReceiveWeaponConfig(int clientId,unsigned char cmdTop[],unsigned packetLength);
	YSRESULT ReceiveListUser(int clientId);
	YSRESULT ReceiveQueryAirState(int clientId,unsigned char cmdTop[],unsigned packetLength);
	YSRESULT ReceiveAirTurretState(int clientId,unsigned char cmdTop[],unsigned packetLength);
	YSRESULT ReceiveGndTurretState(int clientId,unsigned char cmdTop[],unsigned packetLength);
	YSRESULT ReceiveReadBack(int clientId,unsigned char cmdTop[]);
	YSRESULT ReceiveSmokeColor(int clientId,unsigned char cmdTop[],unsigned int packetLength);


	YSRESULT SendReject(int clientId);
	YSRESULT SendError(int clientId,int errorCode);
	YSRESULT SendTextMessage(int clientId,const char txt[]);
	YSRESULT SendLoadField(
	    int clientId,const char fieldName[],YSBOOL loadYFS,const YsVec3 &pos,const YsAtt3 &att);
	YSRESULT SendAddAirplane(int clientId,int airId,FSNETTYPE netType,YSBOOL setAsPlayer);
	YSRESULT SendAddGround(int clientId,int airId,FSNETTYPE netType);
	YSRESULT SendJoinApproval(int clientId);
	YSRESULT SendRemoveAirplane(int clientId,int airId,YSBOOL explosion);
	YSRESULT SendRemoveGround(int clientId,int airId,YSBOOL explosion);
	YSRESULT SendPrepare(int clientId);
	YSRESULT SendTestPacket(int clientId);
	YSRESULT SendTestHugePacket(int clientId);
	YSRESULT SendLockOn(int clientId,int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir);
	YSRESULT SendSetTestAutopilot(int clientId,int idOnSvr,int type);
	YSRESULT SendAssignSideWindow(int clientId,int idOnSvr,const double &hdgOffset,const double &pchOffset);
	YSRESULT SendVersionNotify(int clientId);
	YSRESULT SendAirCmd(int clientId,int airIdOnSvr,const char cmd[]);
	YSRESULT SendGndCmd(int clientId,int gndIdOnSvr,const char cmd[]);
	YSRESULT SendUseMissile(int clientId,YSBOOL useMissile);
	YSRESULT SendUseUnguidedWeapon(int clientId,YSBOOL useUnguidedWeapon);
	YSRESULT SendEnvironment(int clientId);
	YSRESULT SendLogOnComplete(int clientId);
	YSRESULT SendWeaponConfig(int clientId,int airId,YSSIZE_T nWeaponConfig,const int weaponConfig[]);
	YSRESULT SendControlShowUserName(int clientId,int showUserName);
	YSRESULT SendConfigString(int clientId,const char configStr[]);
	YSRESULT SendAirplaneList(int clientId,int numSend);
	YSRESULT SendRemoveAirplaneReadBack(int clientId,int idOnSvr);
	YSRESULT SendRemoveGroundReadBack(int clientId,int idOnSvr);
	YSRESULT SendReadBack(int clientId,int readBackType,int readBackParam);
	YSRESULT SendSmokeColor(int clientId,int idOnSvr,int smkIdx,const YsColor &col);
	YSRESULT SendReportScore(int clientId,YSBOOL scored,FsNetworkScoreLog &score);
	YSRESULT SendForceJoin(int clientId);

	YSRESULT FlushSendQueue(int clientId,unsigned timeout);
	YSRESULT FlushAllSendQueue(unsigned timeout);

	void AddTestAirplaneInClient(int clientId,int type);

	void PrintPlayerList(void);
	int GetMaxNumUser(void) const;
	YSBOOL IsUserLoggedOn(int id) const;
	YSBOOL UserHasAirplane(int id) const;
	const char *GetUserName(int id) const;
	const char *GetUserIpAddressString(int id,YsString &buf) const;

	void ClearUserAirplaneForMemoryCleanUpPurpose(void);

protected:
	YSRESULT BroadcastPacket(YSSIZE_T nDat,unsigned char dat[],unsigned version);
	YSRESULT SendPacket(int clientId,YSSIZE_T nDat,unsigned char dat[]);
	YSRESULT ForwardPacket(int clientId,YSSIZE_T packetLength,unsigned char dat[]);

	YSRESULT EncodeObject(int &isAirplane,int &idOnSvr,FsExistence *obj);
	YSRESULT DecodeObject(FsExistence **obj,int isAirplane,int idOnSvr);
};


class FsClientVariable : public FsNetworkVariable
{
public:
	enum
	{
		CLISTATE_STANDBY,
		CLISTATE_FLYING,
		CLISTATE_SIDEWINDOW,
		CLISTATE_GHOSTVIEW,
	};

	FsClientVariable(const char name[],FsSimulation *sim,FsNetConfig *netcfg);
	~FsClientVariable();

	int clientState;     // 0:Standing by  1:Flying  2:Side Window  3:Ghost
	int choosingMode;    // 0:Not choosing anything    1:Choose airplane    2:Choose start position
	class FsFlightConfig *cfg;
	YSBOOL logOnProcessCompleted;
	YSBOOL waitingForJoiningApproval;
	mutable double nextConsoleUpdateTime;
	double netNoActivityTime;
	double nextListUserTime;

	YsString fieldName;
	YSBOOL fieldReady,fieldNotAvailable,simReady;
	double nextAirNotifyTime,nextGndNotifyTime;
	YSBOOL airplaneAvailable;

	class FsNetConfig *netcfg;

	YsArray <YsString,16> airNameFilter;

	double sendCriticalInfoTimer;
	YsArray <unsigned int> airRmvToSend,gndRmvToSend;
	YsArray <unsigned int> airUnjoinToSend;
	YSBOOL joinReqReadBack;

	class FsAutoCloseFile chatLogFp;
};

class FsSocketClient : public YsSocketClient, public FsClientVariable
{
public:
	YSBOOL fatalError;
	int fatalErrorCode;

	YSBOOL connectionClosedByServer; // Will be set YSTRUE if the connection was closed from the server side.
	int lastErrorFromServer;
	unsigned int reportedServerVersion;

	FsSocketClient(const char username[],const int port,class FsSimulation *associatedSimulation,class FsNetConfig *cfg);

	YSRESULT Received(YSSIZE_T nBytes,unsigned char dat[]);
	YSRESULT ConnectionClosedByServer(void);

	void CheckPendingData(
	    const double &currentTime,
	    class FsGuiChooseAircraft &chooseAirplane,
	    class FsGuiChooseField &chooseStartPosition);

	int GetNumMessage(void) const;
	const char *GetMessage(int i) const;
	void ClearMessage(void);

	YSBOOL ReceivedApproval(void);
	YSBOOL ReceivedRejection(void);
	YSBOOL ServerAllowAAM(void);
	YSBOOL ServerAllowUnguidedWeapon(void);

	YSBOOL ReceivedSideWindowAssignment(void);
	void GetSideWindowAssignment(FsAirplane *&air,double &hdg,double &pch);
	void ModifySideWindowAssignment(const double &hdg,const double &pch);

	YSRESULT NotifyAirplaneState(void);
	YSRESULT NotifyGroundState(void);

	YSRESULT AddMessage(const char *msg);

	YSRESULT SendLogOn(const char username[],int version);
	YSRESULT SendError(int errorCode);
	YSRESULT SendJoinRequest(int iff,const char *air,const char *stp,int fuelPercent,YSBOOL smokeOil);
	YSRESULT SendUnjoin(int airId,YSBOOL explosion);
	YSRESULT SendRequestTestAirplane(void);
	YSRESULT SendKillServer(void);
	YSRESULT SendTestPacket(void);
	YSRESULT SendTestHugePacket(void);  // This must make server disconnect the client.
	YSRESULT SendRemoveAirplane(int airId,YSBOOL explosion);
	YSRESULT SendRemoveGround(int gndId,YSBOOL explosion);
	YSRESULT SendStateChange(void);
	YSRESULT SendLockOn(int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir);
	YSRESULT SendMissileLaunch(FsWeaponRecord &rec);
	YSRESULT SendGetDamage(FsExistence *victim,FsExistence *firedBy,int power,enum FSDIEDOF diedOf,FSWEAPONTYPE weaponType);
	YSRESULT SendRequestToBeSideWindow(const double &hdgOffset,const double &pchOffset);
	YSRESULT SendResendAirRequest(int idOnSvr);
	YSRESULT SendResendGndRequest(int idOnSvr);
	YSRESULT SendAirCmd(int idOnCli,const char cmd[]);
	YSRESULT SendGndCmd(int idOnCli,const char cmd[]);
	YSRESULT SendTextMessage(const char txt[]);
	YSRESULT SendEnvironmentRequest(void);
	YSRESULT SendResendJoinApproval(void);
	YSRESULT SendWeaponConfig(int airId,YSSIZE_T nWeaponConfig,const int weaponConfig[]);
	YSRESULT SendListUser(void);
	YSRESULT SendQueryAirStateForReducingWarpProblem(void);
	YSRESULT SendEnvironmentReadBack(void);
	YSRESULT SendRemoveAirplaneReadBack(int idOnSvr);
	YSRESULT SendRemoveGroundReadBack(int idOnSvr);
	YSRESULT SendReadBack(int readBackType,int readBackParam);
	YSRESULT SendSmokeColor(int idOnSvr,int smkIdx,const YsColor &col);

	YSRESULT ReceiveError(unsigned char dat[]);
	YSRESULT ReceiveLoadField(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveAddObject(unsigned packetLength,unsigned char dat[]);
	YSRESULT ReceiveApproveToJoin(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveRejectToJoin(unsigned char dat[]);
	YSRESULT ReceiveAirplaneState(unsigned char dat[]);
	YSRESULT ReceiveRemoveAirplane(unsigned char dat[]);
	YSRESULT ReceiveRemoveGround(unsigned char dat[]);
	YSRESULT ReceivePrepareSimulation(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveLockOn(unsigned char dat[]);
	YSRESULT ReceiveMissileLaunch(unsigned char dat[]);
	YSRESULT ReceiveGroundState(unsigned int packetLength,unsigned char dat[]);
	YSRESULT ReceiveGetDamage(unsigned char dat[]);
	YSRESULT ReceiveSetTestAutopilot(unsigned char dat[]);
	YSRESULT ReceiveAssignSideWindow(unsigned char dat[]);
	YSRESULT ReceiveVersionNotify(unsigned char dat[]);
	YSRESULT ReceiveAirCmd(unsigned char dat[]);
	YSRESULT ReceiveGndCmd(unsigned char dat[]);
	YSRESULT ReceiveTextMessage(unsigned char dat[]);
	YSRESULT ReceiveUseMissile(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveUseUnguidedWeapon(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveEnvironment(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveReviveGround(void);
	YSRESULT ReceiveWeaponConfig(unsigned char dat[]);
	YSRESULT ReceiveListUser(unsigned char dat[]);
	YSRESULT ReceiveControlShowUserName(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveAirTurretState(unsigned char dat[],unsigned int packetLength);
	YSRESULT ReceiveGndTurretState(unsigned char dat[],unsigned int packetLength);
	YSRESULT ReceiveConfirmExistence(unsigned char dat[]);
	YSRESULT ReceiveConfigString(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveList(int packetLength,unsigned char dat[]);
	YSRESULT ReceiveReadBack(unsigned char cmdTop[]);
	YSRESULT ReceiveSmokeColor(unsigned char cmdTop[]);
	YSRESULT ReceiveScore(unsigned char cmdTop[]);
	YSRESULT ReceiveSkyColor(unsigned char cmdTop[]);
	YSRESULT ReceiveGroundColor(unsigned char cmdTop[]);
	YSRESULT ReceiveFogColor(unsigned char cmdTop[]);
	YSRESULT ReceiveForceJoin(unsigned char cmdTop[]);

	YSRESULT FlushSendQueue(unsigned int timeout);
protected:
	enum
	{
		MAXNUMMESSAGE=16,
		COMBUFSIZE=8192,
		SENDQUEUESIZE=8192
	};
	int nMsg;
	char msg[MAXNUMMESSAGE][256];

	YSBOOL svrUseMissile,svrUseUnguidedWeapon;

	YSSIZE_T nSendQueueFilled;
	unsigned char sendQueue[SENDQUEUESIZE];

	unsigned nComBuf;
	unsigned char comBuf[COMBUFSIZE];

	YSRESULT SendPacket(YSSIZE_T nDat,unsigned char dat[]);

	YSBOOL receivedApproval,receivedRejection;

	YSBOOL sideWindowAssigned;
	FsAirplane *sideWindowAirplane;
	double sideWindowHdg,sideWindowPch;

	void AddToSendQueue(YSSIZE_T nByte,unsigned char dat[]);

	FsExistence *FindObject(int idOnSvr,YSBOOL isAirplane);
	int FindIdOnServer(int idOnCli,YSBOOL isAirplane);

	YSRESULT EncodeObject(int &isAirplane,int &idOnSvr,FsExistence *obj);
	YSRESULT DecodeObject(FsExistence **obj,int isAirplane,int idOnSvr);
};



class FsServerAddressLog
{
public:
	YsArray <YsString> addrLog;
	void ReadAddressFromFile(void);
	void AddAddressToFile(const char addr[],int port);
};

////////////////////////////////////////////////////////////

class FsClientRunLoop
{
public:
	enum CLIENT_RUNSTATE
	{
		CLIENT_RUNSTATE_INITIALIZE1,
		CLIENT_RUNSTATE_INITIALIZE2,
		CLIENT_RUNSTATE_INITIALIZE3,
		CLIENT_RUNSTATE_LOOP,
		CLIENT_RUNSTATE_TERMINATING_WAIT_KEYRELEASE,
		CLIENT_RUNSTATE_TERMINATING,
		CLIENT_RUNSTATE_TERMINATING_WAIT_KEYPRESS,
		CLIENT_RUNSTATE_TERMINATED
	};

	enum CLIENT_FATAL_ERROR
	{
		CLIENT_FATAL_NOERROR,
		CLIENT_FATAL_NO_NETWORK,
		CLIENT_FATAL_CANNOT_CONNECT,
		CLIENT_FATAL_CANNOT_LOGON,
		CLIENT_FATAL_FIELD_UNAVAILABLE,
		CLIENT_FATAL_NO_COMMON_AIRPLANE,
		CLIENT_FATAL_VERSIONCONFLICT
	};

	CLIENT_RUNSTATE runState;
	CLIENT_FATAL_ERROR fatalError;
	int reportedServerVersion;
	FsSocketClient cli;
	YSBOOL prevSimReady;
	class FsServerAddressLog *svrAddrLog;
	std::unique_ptr <class FsGuiCanvas> canvasPtr;
	class FsGuiClientDialog *cliDlg;
	class FsGuiChooseAircraft *chooseAirplane;
	class FsGuiChooseField *chooseStartPosition;
	class FsGuiChooseAircraftOption *opt;
	class FsNetConfig &netcfg;
	double nextTestPacketTimeOut;
	YsString hostname;
	YsString username;
	int port;

	FsClientRunLoop(const char username[],const int port,FsSimulation *sim,FsNetConfig *cfg);
	~FsClientRunLoop();
private:
	const FsClientRunLoop &operator=(const FsClientRunLoop &from);

public:
	/*! Returns YSTRUE if the client has connected to the server and can take commands from the user.
	*/
	YSBOOL NetReady(void) const;
};

class FsServerRunLoop
{
public:
	enum SERVER_RUNSTATE
	{
		SERVER_RUNSTATE_INITIALIZE1,
		SERVER_RUNSTATE_INITIALIZE2,
		SERVER_RUNSTATE_INITIALIZE3,
		SERVER_RUNSTATE_LOOP,
		SERVER_RUNSTATE_TERMINATING_KEYWAIT,
		SERVER_RUNSTATE_TERMINATING,
		SERVER_RUNSTATE_TERMINATED
	};
	enum SERVER_FATAL_ERROR
	{
		SERVER_FATAL_NOERROR,
		SERVER_FATAL_CANNOT_START,
		SERVER_FATAL_FIELD_UNAVAILABLE,
		SERVER_FATAL_UNKNOWN_STATE,
	};

	YsString username,fldName;
	int netPort;

	int resetCounter;
	SERVER_RUNSTATE runState;
	SERVER_FATAL_ERROR fatalError;
	FsSocketServer svr;
	unsigned long serverStartTime,prvTime;
	YSBOOL resetServer;
	class FsGuiServerDialog *svrDlg;
	class FsNetConfig &netcfg;

	class FsGuiChooseAircraft *chooseAirplane;
	class FsGuiChooseField *chooseStartPosition;
	class FsChoose *dispellUser;
	class FsGuiChooseAircraftOption *opt;

	time_t prevGtime;  // For debugging purpose only.

	int startServerRetryCount;
	time_t nextServerStartTryTime,nextServerStartCountDown;

	FsServerRunLoop(const char username[],const char fldName[],int netPort,FsSimulation *sim,FsNetConfig *cfg);
	~FsServerRunLoop();

private:
	const FsServerRunLoop &operator=(const FsServerRunLoop &);
};

/* } */
#endif
