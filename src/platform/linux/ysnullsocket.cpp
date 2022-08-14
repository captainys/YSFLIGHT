// ioctlsocket can set socket to non-blocking mode
//


#include "../../socket/yssocket.h"


YsSocketServer::YsSocketServer(int listen,int maxNumCli)
{
}


YsSocketServer::~YsSocketServer()
{
}


YSRESULT YsSocketServer::Start(void)
{
	return YSERR;
}

YSRESULT YsSocketServer::Terminate(void)
{
	return YSERR;
}


YSRESULT YsSocketServer::CheckAndAcceptConnection(void)
{
	return YSERR;
}

YSRESULT YsSocketServer::CheckReceive(void)
{
	return YSERR;
}


int YsSocketServer::GetNumClient(void)
{
	return maxNumClient;
}

YSBOOL YsSocketServer::IsClientActive(int clientId)
{
	return YSFALSE;
}

YSRESULT YsSocketServer::Disconnect(int clientId)   /* clientId=-1 to close all connections */
{
	return YSERR;
}

YSRESULT YsSocketServer::Send(int clientId,int nBytes,unsigned char dat[],unsigned timeout)
      /* clientId=-1 to broadcast */
{
	return YSERR;
}

YSRESULT YsSocketServer::ReceivedFrom(int clientId,int nBytes,unsigned char dat[])
{
	return YSOK;
}

YSRESULT YsSocketServer::SendTerminateMessage(int clientId,unsigned timeout)
{
	return YSOK;
}

YSRESULT YsSocketServer::ConnectionAccepted(int clientId)
{
	return YSOK;
}

YSRESULT YsSocketServer::ConnectionClosedByClient(int clientId)
{
	return YSOK;
}


////////////////////////////////////////////////////////////



YsSocketClient::YsSocketClient(int p)
{
}

YsSocketClient::~YsSocketClient()
{
}

YSRESULT YsSocketClient::Start(void)
{
	return YSERR;
}

YSRESULT YsSocketClient::Terminate(void)
{
	return YSERR;
}


YSRESULT YsSocketClient::Connect(const char host[])
{
	return YSERR;
}

YSRESULT YsSocketClient::Disconnect(void)
{
	return YSERR;
}


YSBOOL YsSocketClient::IsConnected(void)
{
	return connected;
}


YSRESULT YsSocketClient::Send(int nBytes,unsigned char dat[],unsigned timeout)
{
	return YSERR;
}

YSRESULT YsSocketClient::CheckReceive(void)
{
	return YSERR;
}


YSRESULT YsSocketClient::Received(int nBytes,unsigned char dat[])
{
	return YSOK;
}

YSRESULT YsSocketClient::ConnectionClosedByServer(void)
{
	return YSOK;
}

