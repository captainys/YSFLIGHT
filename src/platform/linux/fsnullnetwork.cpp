#include <ysclass.h>
#include "../../fs.h"

#include "graphics/common/fsopengl.h"
#include "../../fswindow.h"

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#include <time.h>



#include "../../socket/yssocket.h"



// Memo
//   Distinguish "connected" and "logged in"
//   Broadcast messages must be sent to "logged in" users. Not "connected" users
//   "logged in" user is expected to know field and existing objects



////////////////////////////////////////////////////////////

static void FsDeletePacket(unsigned &nSendQueue,unsigned char sendQueue[],unsigned pckPtr)
{
}

static void FsDeleteOldStatePacket(unsigned &nSendQueue,unsigned char sendQueue[],int cmd0,int idOnSvr0)
{
}

YSBOOL FsIsLowPriorityPacket(int cmd)
{
	return YSFALSE;
}

void FsDeleteLowPriorityPacket(unsigned &nSendQueue,unsigned char sendQueue[])
{
}




////////////////////////////////////////////////////////////

// Note:Airsound is loaded in fsoption.cpp
extern FsAirSound *fsAirsound;

////////////////////////////////////////////////////////////


void FsNetworkUser::AddToSendQueue(unsigned nByte,unsigned char dat[])
{
}


////////////////////////////////////////////////////////////


void FsNetReceivedAirplaneState::Decode(const unsigned char dat[],const double &tl)
{
};


////////////////////////////////////////////////////////////


FsSocketServer::FsSocketServer(FsSimulation *assocSim,class FsNetConfig *cfg) :
    YsSocketServer(cfg->portNumber,FS_MAX_NUM_USER)
{
}

YSRESULT FsSocketServer::ReceivedFrom(int clientId,int nBytes,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketServer::DisconnectUser(int clientId)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendTerminateMessage(int clientId,unsigned timeout)
{
	return YSOK;
}

YSRESULT FsSocketServer::ConnectionAccepted(int clientId)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastAirplaneState(void)
{
	return YSERR;
}

YSRESULT FsSocketServer::RectifyIllegalMissiles(void)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastGroundState(void)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastAddAirplane(int airId,FSNETTYPE netType)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastRemoveAirplane(int airId,YSBOOL explosion)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastRemoveGround(int gndId,YSBOOL explosion)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastStateChange(void)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastLockOn(int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastMissileLaunch(FsWeaponRecord &rec)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastGetDamage
    (FsExistence *victim,FsExistence *firedBy,int power,enum FSDIEDOF diedOf)
{
	return YSERR;
}

// BroadcastAirCmd is available after 20010624
YSRESULT FsSocketServer::BroadcastAirCmd(int airId,const char cmd[])
{
	return YSERR;
}

YSBOOL FsSocketServer::ReceivedKillServer(void)
{
	return receivedKillServer;
}

YSRESULT FsSocketServer::ReceiveLockOn(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveRemoveAirplane(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveRemoveGround(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveMissileLaunch(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveGroundState(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveGetDamage(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveRequestToBeSideWindow(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ConnectionClosedByClient(int clientId)
{
	return YSERR;
}

int FsSocketServer::GetNumMessage(void) const
{
	return nMsg;
}

const char *FsSocketServer::GetMessage(int i) const
{
	return msg[i];
}

void FsSocketServer::ClearMessage(void)
{
	nMsg=0;
}

YSRESULT FsSocketServer::AddMessage(const char *txt)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveLogInUser(int clientId,int version,const char recvUsername[])
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveError(int clientId,int errorCode)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveRequestToJoin(int clientId,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveAirplaneState(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveUnjoin(int clientId,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveRequestTestAirplane(int clientId,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveKillServer(int clientId,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveResendAirRequest(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveResendGndRequest(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

// FSNETCMD_AIRCMD is supported after 20010624
YSRESULT FsSocketServer::ReceiveAirCmd(int clientId,unsigned char dat[],unsigned packetLength)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendError(int clientId,int errorCode)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendReject(int clientId)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendLoadField
    (int clientId,char fieldName[],YSBOOL loadYFS,const YsVec3 &pos,const YsAtt3 &att)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendAddAirplane(int clientId,int airId,FSNETTYPE netType)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendAddGround(int clientId,int gndId,FSNETTYPE netType)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendApprovalToJoin(int clientId)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendRemoveAirplane(int clientId,int airId,YSBOOL explosion)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendRemoveGround(int clientId,int gndId,YSBOOL explosion)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendPrepare(int clientId)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendTestPacket(int clientId)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendLockOn(int clientId,int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendSetPlayerAirplane(int clientId,int idOnSvr)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendSetTestAutopilot(int clientId,int idOnSvr,int type)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendAssignSideWindow(int clientId,int idOnSvr,const double &hdgOffset,const double &pchOffset)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendVersionNotify(int clientId)
{
	return YSERR;
}

// BroadcastAirCmd is available after 20010624
YSRESULT FsSocketServer::SendAirCmd(int clientId,int airId,const char cmd[])
{
	return YSERR;
}

YSRESULT FsSocketServer::SendUseMissile(int clientId,YSBOOL useMissile)
{
	return YSERR;
}

YSRESULT FsSocketServer::FlushSendQueue(int clientId,unsigned timeout)
{
	return YSERR;
}

void FsSocketServer::AddTestAirplaneInClient(int clientId,int type)
{
}

void FsSocketServer::PrintPlayerList(void)
{
}

int FsSocketServer::GetMaxNumUser(void) const
{
	return FS_MAX_NUM_USER;
}

YSBOOL FsSocketServer::IsUserLoggedOn(int id) const
{
	return YSFALSE;
}

const char *FsSocketServer::GetUserName(int id) const
{
	return NULL;
}

YSRESULT FsSocketServer::FlushAllSendQueue(unsigned timeout)
{
	return YSERR;
}

YSRESULT FsSocketServer::BroadcastPacket(unsigned nDat,unsigned char dat[],unsigned version)
{
	return YSERR;
}

YSRESULT FsSocketServer::SendPacket(int clientId,unsigned nDat,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketServer::ForwardPacket(int clientId,unsigned int packetLength,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketServer::EncodeObject(int &isAirplane,int &idOnSvr,FsExistence *obj)
{
	return YSERR;
}

YSRESULT FsSocketServer::DecodeObject(FsExistence **obj,int isAirplane,int idOnSvr)
{
	return YSERR;
}

////////////////////////////////////////////////////////////


FsSocketClient::FsSocketClient(FsSimulation *assocSim,class FsNetConfig *cfg) :
    YsSocketClient(cfg->portNumber) ,
	airSvrToCli(128),
	airCliToSvr(128),
	gndSvrToCli(128),
	gndCliToSvr(128)
{
}

YSRESULT FsSocketClient::Received(int nBytes,unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ConnectionClosedByServer(void)
{
	return YSERR;
}

int FsSocketClient::GetNumMessage(void) const
{
	return nMsg;
}

const char *FsSocketClient::GetMessage(int i) const
{
	return msg[i];
}

void FsSocketClient::ClearMessage(void)
{
	nMsg=0;
}

YSBOOL FsSocketClient::ReceivedApproval(void)
{
	YSBOOL r;
	r=receivedApproval;
	receivedApproval=YSFALSE;
	return r;
}

YSBOOL FsSocketClient::ReceivedRejection(void)
{
	YSBOOL r;
	r=receivedRejection;
	receivedRejection=YSFALSE;
	return r;
}

YSRESULT FsSocketClient::NotifyAirplaneState(void)
{
	return YSERR;
}

YSRESULT FsSocketClient::NotifyGroundState(void)
{
	return YSERR;
}

YSRESULT FsSocketClient::AddMessage(const char *txt)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendLogIn(const char username[],int version)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendError(int errorCode)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendRequestToJoin(int iff,const char *air,const char *stp)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendUnjoin(int airId,YSBOOL explosion)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendRequestTestAirplane(void)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendKillServer(void)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendTestPacket(void)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendRemoveAirplane(int airId,YSBOOL explosion)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendRemoveGround(int gndId,YSBOOL explosion)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendStateChange(void)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendLockOn(int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendMissileLaunch(FsWeaponRecord &rec)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendGetDamage(FsExistence *victim,FsExistence *firedBy,int power,enum FSDIEDOF diedOf)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendRequestToBeSideWindow(const double &hdgOffset,const double &pchOffset)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendResendAirRequest(int idOnSvr)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendResendGndRequest(int idOnSvr)
{
	return YSERR;
}

// SendAirCmd is supported after 20010624
YSRESULT FsSocketClient::SendAirCmd(int idOnCli,const char cmd[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveError(unsigned char cmdTop[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveLoadField(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveAddObject(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveApproveToJoin(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveRejectToJoin(unsigned char dat[])
{
	return YSERR;
}

YSBOOL FsSocketClient::ServerAllowAAM(void)
{
	return svrUseMissile;
}

YSBOOL FsSocketClient::ReceivedSideWindowAssignment(void)
{
	YSBOOL r;
	r=sideWindowAssigned;
	sideWindowAssigned=YSFALSE;
	return r;
}

void FsSocketClient::GetSideWindowAssignment(int &idOnSvr,double &hdg,double &pch)
{
}

void FsSocketClient::ModifySideWindowAssignment(const double &hdg,const double &pch)
{
}

YSRESULT FsSocketClient::ReceiveAirplaneState(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveGroundState(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveGetDamage(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveSetPlayerAirplane(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveSetTestAutopilot(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveAssignSideWindow(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveVersionNotify(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveAirCmd(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveUseMissile(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveRemoveAirplane(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveRemoveGround(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceivePrepareSimulation(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveLockOn(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveMissileLaunch(unsigned char dat[])
{
	return YSERR;
}

YSRESULT FsSocketClient::FlushSendQueue(unsigned int timeout)
{
	return YSERR;
}

YSRESULT FsSocketClient::SendPacket(unsigned nDat,unsigned char dat[])
{
	return YSERR;
}

void FsSocketClient::AddToSendQueue(unsigned nByte,unsigned char dat[])
{
}

FsExistence *FsSocketClient::FindObject(int idOnSvr,YSBOOL isAirplane)
{
	return NULL;
}

int FsSocketClient::FindIdOnServer(const int idOnCli,YSBOOL isAirplane)
{
	return -1;
}

YSRESULT FsSocketClient::EncodeObject(int &isAirplane,int &idOnSvr,FsExistence *obj)
{
	return YSERR;
}

YSRESULT FsSocketClient::DecodeObject(FsExistence **obj,int isAirplane,int idOnSvr)
{
	return YSERR;
}






////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////

YSRESULT FsNetworkStandby
    (FsSimulation *sim,
     int ky,char chr,int &escCount,int &choosingMode,
     FsChoose &chooseAirplane,
     FsChoose &chooseStartPosition,YSBOOL fieldReady)
{
	return YSERR;
}

////////////////////////////////////////////////////////////

static void FsPrintServerMenu(FsChoose &airplane,FsChoose &startPosition,FSIFF iff,YSBOOL locked)
{
}

void FsSimulation::RunServerMode(const char name[])
{
}


////////////////////////////////////////////////////////////

static void FsPrintClientMenu(FsChoose &airplane,FsChoose &startPosition,FSIFF iff)
{
}

void FsSimulation::RunClientMode(const char username[],const char hostname[])
{
}


