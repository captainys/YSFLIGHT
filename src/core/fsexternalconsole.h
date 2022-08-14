#ifndef FSEXTERNALCONSOLE_IS_INCLUDED
#define FSEXTERNALCONSOLE_IS_INCLUDED
/* { */

#include "fsexternalconsolecommand.h"
#include "socket/yssocket.h"

class FsExternalConsoleServer : public YsSocketServer
{
public:
	enum
	{
		MAX_NUM_CLIENT=16
	};

	class Client
	{
	public:
		enum
		{
			COMBUFSIZE=8192,
			SENDQUEUESIZE=8192
		};
		unsigned nComBuf;
		unsigned char comBuf[COMBUFSIZE];   // Store commands that are not completed in "Received"

		YSBOOL canBeInstPanel;
		YSBOOL controlElevator,controlAileron,controlRudder,controlThrottle;
		YSBOOL controlWeapon;

		void CleanUp(void);
	};

	YSBOOL active;
	int listeningPort;
	Client cli[MAX_NUM_CLIENT];

	virtual YSRESULT ReceivedFrom(int clientId,int nBytes,unsigned char dat[]);
	virtual YSRESULT SendTerminateMessage(int clientId,unsigned timeout);
	virtual YSRESULT ConnectionAccepted(int clientId,unsigned int ipAddr[4]);
	virtual YSRESULT ConnectionClosedByClient(int clientId);
};

/* } */
#endif
