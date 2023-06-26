// Memo about Chat Dialog
//  EM_SETSEL for setting selection (bottom of the text)
//  EM_REPLACESEL must be able to add message.

#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1



#include <ysclass.h>
#include <ysport.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fsconfig.h"
#include "fsnetconfig.h"

#include "fs.h"
#include "fsnetutil.h"
#include "fsfilename.h"
#include "fspluginmgr.h"

#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
typedef int SOCKET;
#endif

#include <time.h>

#include <ysbitmap.h>

#include "fsgui.h"
#include "fsguiselectiondialogbase.h"
#include "fschoose.h"


#include <yssocket.h>

#include "fstextresource.h"

YSBOOL FsVerboseMode=YSFALSE;



//#define CRASHINVESTIGATION

#ifdef CRASHINVESTIGATION
#define FSSVRLOGFILE "c:/ysflightsvr.log"

// Memo
//   Distinguish "connected" and "logged in"
//   Broadcast messages must be sent to "logged in" users. Not "connected" users
//   "logged in" user is expected to know field and existing objects

static void FsLogOutput(const char fn[],const char msg[])
{
	FILE *fp;
	fp=fopen(fn,"a");
	if(fp!=NULL)
	{
		time_t gtime;
		struct tm *ltime;
		gtime=time(NULL);
		ltime=localtime(&gtime);
		fprintf
		   (fp,"%04d/%02d/%02d %02d:%02d %02d\n",
		    ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday,ltime->tm_hour,ltime->tm_min,ltime->tm_sec);
		if(msg!=NULL)
		{
			fprintf(fp,"%s\n",msg);
		}
		else
		{
			fprintf(fp,"(null)\n");
		}
		fclose(fp);
	}
}
#endif



////////////////////////////////////////////////////////////


void FsServerAddressLog::ReadAddressFromFile(void)
{

	YsString str;

	addrLog.Set(0,NULL);

	FILE *fp=YsFileIO::Fopen(FsGetNetServerAddressHistoryFile(),"r");
	if(fp!=NULL)
	{
		while(str.Fgets(fp)!=NULL)
		{
			if(str.Strlen()>0)
			{
				addrLog.Append(str);
			}
		}
		fclose(fp);
	}
}

void FsServerAddressLog::AddAddressToFile(const char addr[],int port)
{

	YsString str,newEntry;

	newEntry.Printf("%s(%d)",addr,port);
	FILE *fp=YsFileIO::Fopen(FsGetNetServerAddressHistoryFile(),"r");
	if(fp!=NULL)
	{
		while(str.Fgets(fp)!=NULL)
		{
			if(strcmp(str,newEntry)==0)
			{
				fclose(fp);
				return;
			}
		}
		fclose(fp);
	}

	fp=YsFileIO::Fopen(FsGetNetServerAddressHistoryFile(),"a");
	if(fp!=NULL)
	{
		fprintf(fp,"%s\n",newEntry.Txt());
		fclose(fp);
	}
}


////////////////////////////////////////////////////////////

static YSSIZE_T FsEncodeWeaponConfig(unsigned char dat[],int airId,YSSIZE_T nWeaponConfig,const int weaponConfig[])
{
	int i;
	YSSIZE_T packetLength;
	unsigned char *ptr;

	// 4 bytes  FSNETCMD_WEAPONCONFIG
	// 4 bytes  AirId
	// 2 bytes  nWeaponConfig  <- 10 bytes
	// 244 bytes left. -> 2 bytes code+2 bytes num=4 bytes per config.  Take 4 bytes margine -> 240/4=60 configs max.

	nWeaponConfig=YsSmaller <int> ((int)nWeaponConfig,120);  // nWeaponConfig = num config times 2
	nWeaponConfig&=(~1);

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_WEAPONCONFIG);
	FsPushInt(ptr,airId);
	FsPushShort(ptr,(short)nWeaponConfig);
	for(i=0; i<nWeaponConfig-1; i+=2)
	{
		if(weaponConfig[i]==FSWEAPON_SMOKE ||
		   weaponConfig[i]==FSWEAPON_SMOKE0 ||
		   weaponConfig[i]==FSWEAPON_SMOKE1 ||
		   weaponConfig[i]==FSWEAPON_SMOKE2 ||
		   weaponConfig[i]==FSWEAPON_SMOKE3 ||
		   weaponConfig[i]==FSWEAPON_SMOKE4 ||
		   weaponConfig[i]==FSWEAPON_SMOKE5 ||
		   weaponConfig[i]==FSWEAPON_SMOKE6 ||
		   weaponConfig[i]==FSWEAPON_SMOKE7)
		{
			FsPushShort(ptr,(short)weaponConfig[i]);

			int ccode,r,g,b;
			r=(weaponConfig[i+1]&0xff0000)>>19;
			g=(weaponConfig[i+1]&0x00ff00)>>11;
			b=(weaponConfig[i+1]&0x0000ff)>>3;
			ccode=(r<<10)+(g<<5)+b;

			FsPushShort(ptr,(short)ccode);
		}
		else
		{
			FsPushShort(ptr,(short)weaponConfig[i]);
			FsPushShort(ptr,(short)weaponConfig[i+1]);
		}
	}

	packetLength=ptr-dat;
	return packetLength;
}

static void FsDecodeWeaponConfig(int &airId,YsArray <int,64> &weaponConfig,unsigned char dat[])
{
	int i,n;
	const unsigned char *ptr;

	ptr=dat;
	FsPopInt(ptr);  // Skip FSNETCMD_WEAPONCONFIG
	airId=FsPopInt(ptr);
	n=FsPopShort(ptr);
	n&=(~1);

	weaponConfig.Set(0,NULL);
	for(i=0; i<n; i+=2)
	{
		int typ,num;

		typ=FsPopShort(ptr);
		num=FsPopShort(ptr);

		if(typ==FSWEAPON_SMOKE ||
		   typ==FSWEAPON_SMOKE0 ||
		   typ==FSWEAPON_SMOKE1 ||
		   typ==FSWEAPON_SMOKE2 ||
		   typ==FSWEAPON_SMOKE3 ||
		   typ==FSWEAPON_SMOKE4 ||
		   typ==FSWEAPON_SMOKE5 ||
		   typ==FSWEAPON_SMOKE6 ||
		   typ==FSWEAPON_SMOKE7)
		{
			weaponConfig.Append(typ);

			int r,g,b;
			r=(num>>10)&31;
			g=(num>>5)&31;
			b=num&31;

			r=(r>>2)+(r<<3);
			g=(g>>2)+(g<<3);
			b=(b>>2)+(b<<3);
			weaponConfig.Append((r<<16)+(g<<8)+b);
		}
		else
		{
			weaponConfig.Append(typ);
			weaponConfig.Append(num);
		}
	}
}

static void FsDeletePacket(YSSIZE_T &nSendQueue,unsigned char sendQueue[],unsigned pckPtr)
{
	unsigned i;
	unsigned pckLng;
	pckLng=FsGetUnsignedInt(sendQueue+pckPtr);

	for(i=pckPtr; i+(pckLng+4)<nSendQueue; i++)
	{
		sendQueue[i]=sendQueue[i+pckLng+4];
	}

	YSSIZE_T prevNSendQueue;
	prevNSendQueue=nSendQueue;

	nSendQueue=nSendQueue-(pckLng+4);

	if(prevNSendQueue<nSendQueue)
	{
		nSendQueue=0;
		fsConsole.Printf("Buffer underflow!\n");
	}
}

//static void FsDietSendBufferByDeletingOldState(unsigned &nSendQueue,unsigned char sendQueue[])
//{
//	unsigned char *gndPtr,*airPtr;
//	unsigned i;
//	i=0;
//	gndPtr=NULL;
//	airPtr=NULL;
//	while(i<nSendQueue)
//	{
//		int cmd1;
//		unsigned pckLng1;
//		unsigned char *pckTop1,*cmdTop1;
//
//		pckTop1=sendQueue+i;
//		cmdTop1=sendQueue+i+4;
//
//		pckLng1=FsGetUnsignedInt(pckTop1);
//		cmd1=FsGetInt(cmdTop1);
//
//		if(cmd1==FSNETCMD_AIRPLANESTATE || cmd1==FSNETCMD_GROUNDSTATE)
//		{
//			unsigned j;
//			int cmd2,idOnSvr1;
//			unsigned pckLng2;
//			unsigned char *pckTop2,*cmdTop2;
//
//			idOnSvr1=FsGetInt(cmdTop1+8);
//			j=i+pckLng1+4;
//			while(j<nSendQueue)
//			{
//				pckTop2=sendQueue+j;
//				cmdTop2=sendQueue+j+4;
//
//				pckLng2=FsGetUnsignedInt(pckTop2);
//				cmd2=FsGetInt(cmdTop2);
//
//				if(cmd1==cmd2 && FsGetInt(cmdTop2+8)==idOnSvr1)
//				{
//					FsDeletePacket(nSendQueue,sendQueue,i);
//					break;
//				}
//				j=j+FsGetUnsignedInt(sendQueue+j)+4;
//			}
//		}
//
//		pckLng1=FsGetUnsignedInt(pckTop1);  // <- Don't forget refreshing it
//		i=i+pckLng1+4;
//	}
//}

static void FsDeleteOldStatePacket(YSSIZE_T &nSendQueue,unsigned char sendQueue[],int cmd0,int idOnSvr0)
{
	unsigned i;
	i=0;

	while(i+8<=nSendQueue)
	{
		int cmd;
		unsigned char *pckTop,*cmdTop;

		pckTop=sendQueue+i;
		cmdTop=sendQueue+i+4;

		cmd=FsGetInt(cmdTop);

		if(cmd==cmd0 && FsGetInt(cmdTop+8)==idOnSvr0)
		{
			FsDeletePacket(nSendQueue,sendQueue,i);
		}
		else // 2003/01/07
		{
			unsigned pckLng;
			pckLng=FsGetUnsignedInt(pckTop);
			i=i+pckLng+4;
		}
	}
}

YSBOOL FsIsLowPriorityPacket(int cmd)
{
	if(cmd==FSNETCMD_REMOVEGROUND ||
	   cmd==FSNETCMD_GROUNDSTATE ||
	   cmd==FSNETCMD_RESENDGNDREQUEST ||
	   cmd==FSNETCMD_ERROR ||
	   cmd==FSNETCMD_NOP)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsDeleteLowPriorityPacket(YSSIZE_T &nSendQueue,unsigned char sendQueue[])
{
	unsigned i;
	i=0;

	while(i+8<=nSendQueue)
	{
		int cmd;
		unsigned char *pckTop,*cmdTop;

		pckTop=sendQueue+i;
		cmdTop=sendQueue+i+4;

		cmd=FsGetInt(cmdTop);

		if(FsIsLowPriorityPacket(cmd)==YSTRUE)
		{
			FsDeletePacket(nSendQueue,sendQueue,i);
		}
		else // 2003/01/07
		{
			unsigned pckLng;
			pckLng=FsGetUnsignedInt(pckTop);
			i=i+pckLng+4;
		}
	}
}




////////////////////////////////////////////////////////////


void FsNetworkScoreLog::FormatScoredMessage(YsString &msg)
{
	msg.Set("You killed ");
	msg.Append(killedUsername);
	msg.Append(" [");
	msg.Append(killedVehicleType);
	msg.Append("]");

	msg.Append(" with ");
	msg.Append(FsGetWeaponString(weaponType));
	msg.Append(".");
}

void FsNetworkScoreLog::FormatKilledMessage(YsString &msg)
{
	msg.Set("You are killed by ");
	msg.Append(scoredUsername);
	msg.Append(" [");
	msg.Append(scoredVehicleType);
	msg.Append("]");

	msg.Append(" with ");
	msg.Append(FsGetWeaponString(weaponType));
	msg.Append(".");
}


////////////////////////////////////////////////////////////


void FsNetworkUser::AddToSendQueue(YSSIZE_T nByte,unsigned char dat[])
{

	for(decltype(nByte) i=0; i<nByte; i++)
	{
		sendQueue[nSendQueueFilled+i]=dat[i];
	}
	nSendQueueFilled+=nByte;
}

void FsNetworkUser::Initialize(void)
{
	state=FSUSERSTATE_NOTCONNECTED; // Will be set to PENDING in ReceiveLogOnUser

	noActivityTime=0.0;
	username.Set("");
	ipAddr[0]=ipAddr[0];
	ipAddr[1]=ipAddr[1];
	ipAddr[2]=ipAddr[2];
	ipAddr[3]=ipAddr[3];
	air=NULL;
	nComBuf=0;
	nSendQueueFilled=0;

	sendCriticalInfoTimer=0.0;
	configStringToSend.Set(0,NULL);
	fldToSend.Set(0,NULL);
	gndToSend.Set(0,NULL);
	airToSend.Set(0,NULL);

	gndRmvToSend.Set(0,NULL);
	airRmvToSend.Set(0,NULL);
	joinSequence=0;

	useMissileReadBack=YSFALSE;
	useUnguidedWeaponReadBack=YSFALSE;
	controlShowUserNameReadBack=YSFALSE;
	environmentReadBack=YSFALSE;
	preparationReadBack=YSFALSE;
}


////////////////////////////////////////////////////////////


void FsNetReceivedAirplaneState::Decode(const unsigned char dat[],const double &tl)
{
	int version;
	const unsigned char *ptr=dat;

	FsPopInt(ptr);   // FSNETCMD_AIRPLANESTATE  skip


	// May not be included >>
	thrVector=0.0;
	thrReverser=0.0;
	bombBay=0.0;
	// << May not be included



	tRemote=(double)FsPopFloat(ptr);
	tLocal=tl;

	FsPopInt(ptr);   // idOnSvr   skip
	version=FsPopShort(ptr);
	if(version==4 || version==5)
	{
		x=FsPopFloat(ptr); // FsPushFloat(ptr,(float)sta.Position.x());
		y=FsPopFloat(ptr); // FsPushFloat(ptr,(float)sta.Position.y());
		z=FsPopFloat(ptr); // FsPushFloat(ptr,(float)sta.Position.z());

		h=double(FsPopShort(ptr))*YsPi/32768.0; // FsPushShort(ptr,h);
		p=double(FsPopShort(ptr))*YsPi/32768.0; // FsPushShort(ptr,p);
		b=double(FsPopShort(ptr))*YsPi/32768.0; // FsPushShort(ptr,b);

		vx=double(FsPopShort(ptr))/10.0; // FsPushShort(ptr,(short)(sta.Velocity.x()*10.0));
		vy=double(FsPopShort(ptr))/10.0; // FsPushShort(ptr,(short)(sta.Velocity.y()*10.0));
		vz=double(FsPopShort(ptr))/10.0; // FsPushShort(ptr,(short)(sta.Velocity.z()*10.0));

		vp=double(FsPopShort(ptr))*YsPi/32768.0; // FsPushShort(ptr,vp);
		vh=double(FsPopShort(ptr))*YsPi/32768.0; // FsPushShort(ptr,vy);
		vb=double(FsPopShort(ptr))*YsPi/32768.0; // FsPushShort(ptr,YsBound(vr,-32767,32767));

		smokeOil=double(FsPopShort(ptr)); // FsPushShort(ptr,(short)GetSmokeOil());
		fuel=double(FsPopInt(ptr));     // FsPushInt(ptr,(short)GetFuelLeft());
		payload=double(FsPopShort(ptr));  // FsPushShort(ptr,(short)sta.Payload);

		state=FSFLIGHTSTATE(FsPopUnsignedChar(ptr)); // FsPushUnsignedChar(ptr,(unsigned char)GetFlightState());
		vgw=double(FsPopUnsignedChar(ptr))/255.0;    // FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetVariableGeometryWingState()*255.0),0,255));

		unsigned int c;
		c=FsPopUnsignedChar(ptr);
		spoiler=double((c>>4)&15)/15.0;
		ldg=double(c&15)/15.0;

		c=FsPopUnsignedChar(ptr);
		flap=double((c>>4)&15)/15.0;
		brake=double(c&15)/15.0;

		flags=FsPopShort(ptr); // FsPushShort(ptr,flag);
		if((flags&1)!=0)
		{
			ab=YSTRUE;
		}
		else
		{
			ab=YSFALSE;
		}

		firingGun=YSFALSE;
		trailingSmokeFlag=0;
		if((flags&8)!=0)
		{
			firingGun=YSTRUE;
		}

		if((flags&2)!=0)  // Older client sends just bit 2, and zero smoke selection.
		{
			trailingSmokeFlag=((flags>>8)&255);
			if(0==trailingSmokeFlag)
			{
				trailingSmokeFlag=255;
			}
		}


		navLight=YSFALSE;
		beacon=YSFALSE;
		strobe=YSFALSE;
		landingLight=YSFALSE;
		if(flags&16)
		{
			beacon=YSTRUE;
		}
		if(flags&32)
		{
			navLight=YSTRUE;
		}
		if(flags&64)
		{
			strobe=YSTRUE;
		}
		if(flags&128)
		{
			landingLight=YSTRUE;
		}


		gun=FsPopShort(ptr);    // FsPushUnsignedChar(ptr,(unsigned char)gun);
		rocket=FsPopShort(ptr); // FsPushUnsignedChar(ptr,(unsigned char)rocket);

		aam=FsPopUnsignedChar(ptr);    // FsPushUnsignedChar(ptr,(unsigned char)aam);
		agm=FsPopUnsignedChar(ptr);    // FsPushUnsignedChar(ptr,(unsigned char)agm);
		bomb=FsPopUnsignedChar(ptr);   // FsPushUnsignedChar(ptr,(unsigned char)bom);
		life=FsPopUnsignedChar(ptr);   // FsPushUnsignedChar(ptr,(unsigned char)GetDamageTolerance());

		g=double(FsPopChar(ptr))/10.0; // FsPushChar(ptr,(char)g);

		throttle=double(FsPopUnsignedChar(ptr)/99.0); // FsPushUnsignedChar(ptr,(unsigned char)YsBound(int(GetThrottle()*99.0),0,99));
		elevator=double(FsPopChar(ptr)/99.0);         // FsPushChar(ptr,(char)YsBound(int(GetElevator()*99.0),-99,99));
		aileron= double(FsPopChar(ptr)/99.0);         // FsPushChar(ptr,(char)YsBound(int(GetAileron()*99.0),-99,99));
		rudder=  double(FsPopChar(ptr)/99.0);         // FsPushChar(ptr,(char)YsBound(int(GetRudder()*99.0),-99,99));
		elvTrim= double(FsPopChar(ptr)/99.0);         // FsPushChar(ptr,(char)YsBound(int(GetElvTrim()*99.0),-99,99));

		if(version==4)
		{
			c=FsPopUnsignedChar(ptr);
			thrVector=double((c>>4)&15)/15.0;
			thrReverser=double(c&15)/15.0;

			c=FsPopUnsignedChar(ptr);
			bombBay=double((c>>4)&15)/15.0;
		}
	}
	else
	{
		FsPopShort(ptr); //padding

		x=FsPopFloat(ptr);
		y=FsPopFloat(ptr);
		z=FsPopFloat(ptr);

		h=double(FsPopShort(ptr))*YsPi/32768.0;
		p=double(FsPopShort(ptr))*YsPi/32768.0;
		b=double(FsPopShort(ptr))*YsPi/32768.0;

		vx=double(FsPopShort(ptr))/10.0;
		vy=double(FsPopShort(ptr))/10.0;
		vz=double(FsPopShort(ptr))/10.0;

		vp=double(FsPopShort(ptr))*YsPi/32768.0;  // This will be discarded if version>=1
		vh=double(FsPopShort(ptr))*YsPi/32768.0;
		vb=double(FsPopShort(ptr))*YsPi/32768.0;

		g=double(FsPopShort(ptr))/100.0;

		gun=FsPopShort(ptr);
		aam=FsPopShort(ptr);
		agm=FsPopShort(ptr);
		bomb=FsPopShort(ptr);
		smokeOil=double(FsPopShort(ptr));
		fuel=double(FsPopFloat(ptr));
		payload= double(FsPopFloat(ptr));
		life=FsPopShort(ptr);

		state=FSFLIGHTSTATE(FsPopUnsignedChar(ptr));
		vgw=double(FsPopUnsignedChar(ptr))/255.0;
		spoiler=double(FsPopUnsignedChar(ptr))/255.0;
		ldg=double(FsPopUnsignedChar(ptr))/255.0;
		flap=double(FsPopUnsignedChar(ptr))/255.0;
		brake=double(FsPopUnsignedChar(ptr))/255.0;

		flags=FsPopShort(ptr);

		if((flags&1)!=0)
		{
			ab=YSTRUE;
		}
		else
		{
			ab=YSFALSE;
		}

		firingGun=YSFALSE;
		trailingSmokeFlag=0;
		if((flags&8)!=0)
		{
			firingGun=YSTRUE;
		}

		if((flags&2)!=0)  // Older client sends just bit 2, and zero smoke selection.
		{
			trailingSmokeFlag=((flags>>8)&255);
			if(0==trailingSmokeFlag)
			{
				trailingSmokeFlag=255;
			}
		}

		throttle=double(FsPopUnsignedChar(ptr)/99.0);
		elevator=double(FsPopChar(ptr)/99.0);
		aileron= double(FsPopChar(ptr)/99.0);
		rudder=  double(FsPopChar(ptr)/99.0);
		elvTrim= double(FsPopChar(ptr)/99.0);

		rocket=FsPopShort(ptr);

		if(version>=1)
		{
			vp=double(FsPopFloat(ptr));
			vh=double(FsPopFloat(ptr));
			vb=double(FsPopFloat(ptr));
		}

		if(version>=2)
		{
			thrVector=double(FsPopUnsignedChar(ptr))/255.0;
			thrReverser=double(FsPopUnsignedChar(ptr))/255.0;
			bombBay=double(FsPopUnsignedChar(ptr))/255.0;
		}
	}
};


////////////////////////////////////////////////////////////


FsSocketServer::FsSocketServer(const char username[],FsSimulation *assocSim,int netPort,class FsNetConfig *cfg) :
    YsSocketServer((0<=netPort ? netPort : cfg->portNumber),FS_MAX_NUM_USER),
    FsServerVariable(username,assocSim,cfg)
{
	sim->SetNetServer(this);
}

FsSocketServer::~FsSocketServer()
{
}

YSRESULT FsSocketServer::ReceivedFrom(int clientId,YSSIZE_T nBytes,unsigned char dat[])
{
	// printf("Server:Received from %d, %d bytes\n",clientId,nBytes);
	static YSBOOL caution=YSFALSE;

	while(nBytes>0)   // Continue until all bytes are stored or processed
	{
		int i,stored;
		auto &nComBuf=user[clientId].nComBuf;

		stored=0;
		for(i=0; i<nBytes && i+nComBuf<FsNetworkUser::COMBUFSIZE; i++)
		{
			user[clientId].comBuf[nComBuf+i]=dat[i];
			stored++;
		}

		nComBuf+=stored;
		nBytes-=stored;
		dat+=stored;

		unsigned int comBufPtr;
		comBufPtr=0;
		for(;;)
		{
			unsigned packetLength;
			if(comBufPtr+4<=nComBuf)
			{
				packetLength=FsGetUnsignedInt(user[clientId].comBuf+comBufPtr);
				if(packetLength>FsNetworkUser::COMBUFSIZE-4)
				{
					// Fatal Error!  I can do nothing.
					YsString str;
					str.Set("User ");
					str.Append(user[clientId].username);
					str.Append(" has been disconnected due to fatal data loss or corruption");
					AddMessage(str);

					SendTextMessage(clientId,"Fatal data loss or corruption.\n");
					SendError(clientId,FSNETERR_CANNOTSUSTAIN);
					FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
					DisconnectUser(clientId);
					return YSERR;
				}
			}
			else
			{
				packetLength=0;
			}


			if(nComBuf<=comBufPtr+4 || nComBuf-comBufPtr<packetLength+4)
				// False of the first condition guarantees the validity of packetLength.
				// Thus, the second one never gives false positive.
			{
				unsigned i;
				for(i=comBufPtr; i<nComBuf; i++)   // Shift unprocessed chunk of data to top.
				{
					user[clientId].comBuf[i-comBufPtr]=user[clientId].comBuf[i];
				}
				nComBuf-=comBufPtr;
				break;
			}
			else
			{
				int cmd;
				unsigned char *cmdTop;

				cmdTop=user[clientId].comBuf+comBufPtr+4;
				cmd=FsGetInt(cmdTop);
			#ifdef CRASHINVESTIGATION
				printf("CMD %d IN\n",cmd);
			#endif

				if(FsVerboseMode==YSTRUE && cmd!=FSNETCMD_AIRPLANESTATE && cmd!=FSNETCMD_GROUNDSTATE)
				{
					printf("CMD %d IN\n",cmd);
				}

				user[clientId].noActivityTime=0.0;  // Reset no-activity timer

				switch(cmd)
				{
				case FSNETCMD_TESTPACKET:
					// printf("Packet Size=%d\n",packetLength);
					break;
				case FSNETCMD_LOGON:
					if(24>=packetLength)
					{
						ReceiveLogOnUser(clientId,FsGetInt(cmdTop+20),(char *)cmdTop+4);
					}
					else
					{
						char username[204];
						strncpy(username,(char *)(cmdTop+24),200);
						ReceiveLogOnUser(clientId,FsGetInt(cmdTop+20),username);
					}
					break;
				case FSNETCMD_ERROR:
					ReceiveError(clientId,FsGetInt(cmdTop+4));
					break;

				case FSNETCMD_LOADFIELD:  // 2007/01/06
					ReceiveLoadFieldReadBack(clientId,cmdTop);
					break;
				case FSNETCMD_CONFIGSTRING:  // 2007/01/07
					ReceiveConfigStringReadBack(clientId,cmdTop);
					break;
				case FSNETCMD_LIST:                    //  44
					ReceiveListReadBack(clientId,cmdTop);
					break;
				case FSNETCMD_READBACK:
					ReceiveReadBack(clientId,cmdTop);
					break;

				case FSNETCMD_SMOKECOLOR:
					ReceiveSmokeColor(clientId,cmdTop,packetLength);
					break;

				case FSNETCMD_JOINREQUEST:
					ReceiveJoinRequest(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_AIRPLANESTATE:
					ReceiveAirplaneState(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_UNJOIN:
					ReceiveUnjoin(clientId,cmdTop);
					break;
				case FSNETCMD_REQUESTTESTAIRPLANE:
					ReceiveRequestTestAirplane(clientId,cmdTop);
					break;
				// case FSNETCMD_KILLSERVER:
				// 	ReceiveKillServer(clientId,cmdTop);
				// 	break;
				case FSNETCMD_LOCKON:
					ReceiveLockOn(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_REMOVEAIRPLANE:
					ReceiveRemoveAirplane(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_REMOVEGROUND:
					ReceiveRemoveGround(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_MISSILELAUNCH:
					ReceiveMissileLaunch(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_GROUNDSTATE:
					ReceiveGroundState(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_GETDAMAGE:
					ReceiveGetDamage(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_GNDTURRETSTATE:          //  40
					ReceiveGndTurretState(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_REQTOBESIDEWINDOWOFSVR:
					ReceiveRequestToBeSideWindow(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_RESENDAIRREQUEST:
					ReceiveResendAirRequest(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_RESENDGNDREQUEST:
					ReceiveResendGndRequest(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_AIRCMD:
					ReceiveAirCmd(clientId,cmdTop,packetLength);
					break;

				case FSNETCMD_TEXTMESSAGE:             //  32
					ReceiveTextMessage(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_ENVIRONMENT:              //  33
					ReceiveEnvironmentRequest(clientId,cmdTop,packetLength);
					break;

				case FSNETCMD_NEEDRESENDJOINAPPROVAL:  //  34
					ReceivedResendJoinApproval(clientId,cmdTop,packetLength);
					break;

				case FSNETCMD_REVIVEGROUND:            //  35
					break;
				case FSNETCMD_WEAPONCONFIG:            //  36
					ReceiveWeaponConfig(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_LISTUSER:                //  37
					ReceiveListUser(clientId);
					break;
				case FSNETCMD_QUERYAIRSTATE:           //  38
					ReceiveQueryAirState(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_AIRTURRETSTATE:             //  40
					ReceiveAirTurretState(clientId,cmdTop,packetLength);
					break;
				case FSNETCMD_GNDCMD:                 //  45
					ReceiveGndCmd(clientId,cmdTop,packetLength);
					break;

				case FSNETCMD_CONFIRMEXISTENCE:              //  42
				case FSNETCMD_SERVER_FORCE_JOIN:       //  47
				case FSNETCMD_FOGCOLOR:              //  48
				case FSNETCMD_SKYCOLOR:              //  49
				case FSNETCMD_GNDCOLOR:              //  50
				case FSNETCMD_RESERVED_FOR_LIGHTCOLOR:              //  51
				case FSNETCMD_RESERVED21:              //  52
				case FSNETCMD_RESERVED22:             //  53
				case FSNETCMD_RESERVED23:             //  54
				case FSNETCMD_RESERVED24:             //  55
				case FSNETCMD_RESERVED25:             //  56
				case FSNETCMD_RESERVED26:             //  57
				case FSNETCMD_RESERVED27:             //  58
				case FSNETCMD_RESERVED28:             //  59
				case FSNETCMD_RESERVED29:             //  60
				case FSNETCMD_RESERVED30:             //  61
				case FSNETCMD_RESERVED31:             //  62
				case FSNETCMD_RESERVED32:             //  63
				case FSNETCMD_OPENYSF_RESERVED33:     //  64 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED34:     //  65 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED35:     //  66 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED36:     //  67 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED37:     //  68 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED38:     //  69 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED39:     //  70 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED40:     //  71 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED41:     //  72 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED42:     //  73 Reserved for OpenYSF
				case FSNETCMD_RESERVED43:             //  74
				case FSNETCMD_RESERVED44:             //  75
				case FSNETCMD_RESERVED45:             //  76
				case FSNETCMD_RESERVED46:             //  77
				case FSNETCMD_RESERVED47:             //  78
				case FSNETCMD_RESERVED48:             //  79
				case FSNETCMD_RESERVED49:             //  80
					break;

				default:
					caution=YSTRUE;
					{
						char str[256];
						sprintf(str,"Unknown or Unimplemented command [%d] is received",cmd);
						AddMessage(str);
					}
					SendError(clientId,FSNETERR_CANNOTSUSTAIN);
					FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
					DisconnectUser(clientId);
					return YSERR;
				}
				comBufPtr=comBufPtr+packetLength+4;

			#ifdef CRASHINVESTIGATION
				printf("CMD %d O\n",cmd);
			#endif
			}
		}
	}

	return YSOK;
}

FsAirplane *FsSocketServer::ServerPlayerPlane()
{
	return sim->GetPlayerAirplane();
}

YSRESULT FsSocketServer::CheckPendingUser(const double &currentTime)
{
	const int unitNum=32;

	int clientId;
	for(clientId=0; clientId<FS_MAX_NUM_USER; clientId++)
	{
		if(user[clientId].state!=FSUSERSTATE_NOTCONNECTED)
		{
			double interval;
			if(user[clientId].state==FSUSERSTATE_PENDING ||
			   user[clientId].airToSend.GetN()+user[clientId].gndToSend.GetN()>=10)
			{
				interval=0.5;  // Rush delivery.
			}
			else
			{
				interval=1.5;
			}

			if(user[clientId].state==FSUSERSTATE_PENDING) // Do not combine the inside if-statement with this if-statement.
			{                                             // SendCriticalInfoTimer is be updated if nothing is to be re-sent (in the last else block).
				CheckAndSendPendingData(clientId,currentTime,interval);
			}
			else if(user[clientId].airToSend.GetN()>0 ||  // Do not combine the inside if-statement with this if-statement.
			        user[clientId].gndToSend.GetN()>0)    // SendCriticalInfoTimer is be updated if nothing is to be re-sent (in the last else block).
			{
				if(user[clientId].sendCriticalInfoTimer<currentTime)
				{
					user[clientId].sendCriticalInfoTimer=currentTime+interval;

					int i;
					for(i=0; i<user[clientId].gndToSend.GetN() && i<unitNum; i++)
					{
						FsGround *gnd;
						gnd=sim->FindGround(user[clientId].gndToSend[i]);
						if(gnd!=NULL && gnd->Prop().IsAlive()==YSTRUE)
						{
							SendAddGround(clientId,user[clientId].gndToSend[i],FSNET_REMOTE);
						}
						else
						{
							user[clientId].gndToSend.DeleteBySwapping(i);
						}
					}
					if(user[clientId].gndToSend.GetN()>unitNum)
					{
						YsArray <unsigned int,unitNum> buf;
						buf.Set(unitNum,user[clientId].gndToSend);
						user[clientId].gndToSend.Delete(0,unitNum);
						user[clientId].gndToSend.Append(unitNum,buf);
					}


					// 2018/04/07
					// Wait until all ground objects have been sent.
					// Otherwise the airplane may not be loaded on a carrier.
					if(0==user[clientId].gndToSend.size())
					{
						for(i=0; i<user[clientId].airToSend.GetN() && i<unitNum; i++)
						{
							FsAirplane *air;
							air=sim->FindAirplane(user[clientId].airToSend[i]);
							if(air!=NULL && air->Prop().IsAlive()==YSTRUE)
							{
								if(user[clientId].air==air)
								{
									unsigned int airDatPacketLength;
									unsigned char airDat[256];
									SendAddAirplane(clientId,user[clientId].airToSend[i],FSNET_LOCAL,YSTRUE);
									airDatPacketLength=air->Prop().NetworkEncode(airDat,FsExistence::GetSearchKey(air),sim->currentTime,YSFALSE);
									SendPacket(i,airDatPacketLength,airDat);
								}
								else
								{
									SendAddAirplane(clientId,user[clientId].airToSend[i],FSNET_REMOTE,YSFALSE);
								}
								YsArray <int,64> weaponConfig;
								air->Prop().GetWeaponConfig(weaponConfig);
								SendWeaponConfig(clientId,user[clientId].airToSend[i],weaponConfig.GetN(),weaponConfig);
							}
							else
							{
								user[clientId].airToSend.DeleteBySwapping(i);
							}
						}
						if(user[clientId].airToSend.GetN()>unitNum)
						{
							YsArray <unsigned int,unitNum> buf;
							buf.Set(unitNum,user[clientId].airToSend);
							user[clientId].airToSend.Delete(0,unitNum);
							user[clientId].airToSend.Append(unitNum,buf);
						}
					}
				}
			}
			else if(user[clientId].airRmvToSend.GetN()>0 ||  // Do not combine the inside if-statement with this if-statement.
			        user[clientId].gndRmvToSend.GetN()>0)    // SendCriticalInfoTimer is be updated if nothing is to be re-sent (in the last else block).
			{
				if(user[clientId].sendCriticalInfoTimer<currentTime)
				{
					user[clientId].sendCriticalInfoTimer=currentTime+interval;

					int i;
					for(i=0; i<user[clientId].airRmvToSend.GetN() && i<unitNum; i++)
					{
						SendRemoveAirplane(clientId,user[clientId].airRmvToSend[i],YSFALSE);
					}
					for(i=0; i<user[clientId].gndRmvToSend.GetN() && i<unitNum; i++)
					{
						SendRemoveGround(clientId,user[clientId].gndRmvToSend[i],YSFALSE);
					}
				}
			}
			else if(user[clientId].joinSequence!=0) // Do not combine the inside if-statement with this if-statement.
			{                                       // SendCriticalInfoTimer is be updated if nothing is to be re-sent (in the last else block).
				if(user[clientId].sendCriticalInfoTimer<currentTime)
				{
					user[clientId].sendCriticalInfoTimer=currentTime+interval;

					switch(user[clientId].joinSequence)
					{
					case 0: // Nothing to do
						break;
					case 1: // Waiting for AddAirplaneReadBack (Processed in ReceiveAddAirplaneReadBack)
						break;
					case 2: // Waiting for Join Approval Read Back
						SendJoinApproval(clientId);
						break;
					}
				}
			}
			else
			{
				// The program falls into this else block if no information needs to be re-sent.
				// In this case, always push the timer 3 seconds ahead of time.
				user[clientId].sendCriticalInfoTimer=currentTime+interval;
			}
		}
		else if(user[clientId].state==FSUSERSTATE_NOTCONNECTED &&
		        IsClientActive(clientId)==YSTRUE &&
		        netcfg->logOnTimeOut!=0)
		{
			// This can happen if the client connects and does not send a log on request.
			if(user[clientId].logOnTimeOut<time(NULL))
			{
				char str[256];
				sprintf(str,"User %d disconnected due to time out.\n",clientId);
				AddMessage(str);
				SendTextMessage(clientId,"Connection Timed Out.\n");
				FlushSendQueue(clientId,FS_NETTIMEOUT);
				DisconnectUser(clientId);
			}
		}
	}

	return YSERR;
}

YSRESULT FsSocketServer::CheckAndSendPendingData(int clientId,const double &currentTime,const double &interval)
{
	int j;
	const int unitNum=32;
	if(user[clientId].sendCriticalInfoTimer<currentTime)
	{
		user[clientId].sendCriticalInfoTimer=currentTime+interval;

		if(user[clientId].configStringToSend.GetN()>0)
		{
			for(j=0; j<user[clientId].configStringToSend.GetN() && j<unitNum; j++)
			{
				SendConfigString(clientId,user[clientId].configStringToSend[j]);
			}
		}
		else if(user[clientId].fldToSend.GetN()>0)
		{
			for(j=0; j<user[clientId].fldToSend.GetN() && j<8; j++)
			{
				SendLoadField(
				    clientId,
				    user[clientId].fldToSend[j].fldName,
				    YSFALSE,
				    user[clientId].fldToSend[j].pos,
				    user[clientId].fldToSend[j].att);
			}
		}
		else if(user[clientId].airTypeToSend.GetN()>0)
		{
			SendAirplaneList(clientId,unitNum);
		}
		else if(user[clientId].useMissileReadBack!=YSTRUE)
		{
			SendUseMissile(clientId,netcfg->useMissile);
		}
		else if(user[clientId].useUnguidedWeaponReadBack!=YSTRUE)
		{
			SendUseUnguidedWeapon(clientId,netcfg->useUnguidedWeapon);
		}
		else if(user[clientId].controlShowUserNameReadBack!=YSTRUE)
		{
			SendControlShowUserName(clientId,netcfg->serverControlShowUserName);
		}
		else if(user[clientId].environmentReadBack!=YSTRUE)
		{
			SendEnvironment(clientId);
		}
		else
		{
			if(user[clientId].preparationReadBack!=YSTRUE)
			{
				SendPrepare(clientId);
			}
			else
			{
				user[clientId].state=FSUSERSTATE_LOGGEDON;

				YsString str;
				str.Set("User ");
				str.Append(user[clientId].username);
				str.Append(" log-on process completed.");
				AddMessage(str);

				SendTextMessage(clientId,"** Log-on process completed **");
				SendLogOnComplete(clientId);

				if(netcfg->sendWelcomeMessage==YSTRUE && welcomeMessage.Strlen()>0)
				{
					SendTextMessage(clientId,welcomeMessage);
				}
			}
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::DisconnectUser(int clientId)
{
	if(0<=clientId && clientId<FS_MAX_NUM_USER)  // Don't check user[clientId].loggedOn==YSTRUE
	{                                            // This function is also used in ReceiveLogInRequest
		// Added 2002/05/05 >>
		if(user[clientId].state==FSUSERSTATE_LOGGEDON && user[clientId].air!=NULL)
		{
			int airId;
			airId=FsExistence::GetSearchKey(user[clientId].air);
			BroadcastRemoveAirplane(airId,YSTRUE);
		}
		// Added 2002/05/05 <<

		Disconnect(clientId);
		user[clientId].state=FSUSERSTATE_NOTCONNECTED;
		user[clientId].air=NULL;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketServer::SendTerminateMessage(int /*clientId*/,unsigned /*timeout*/)
{
	return YSOK;
}

YSRESULT FsSocketServer::ConnectionAccepted(int clientId,unsigned int ipAddr[4])
{
	char str[256];
	sprintf(str,"Connection Request By #%d",clientId);
	AddMessage(str);
	sprintf(str,"IP-Address %d.%d.%d.%d\n",ipAddr[0],ipAddr[1],ipAddr[2],ipAddr[3]);
	AddMessage(str);


	if(netcfg->multiConnLimit!=0)
	{
		int i,nSameIp;
		nSameIp=0;
		for(i=0; i<FS_MAX_NUM_USER; i++)
		{
			if(i!=clientId &&
			   IsClientActive(i)==YSTRUE &&
			   user[i].ipAddr[0]==ipAddr[0] &&
			   user[i].ipAddr[1]==ipAddr[1] &&
			   user[i].ipAddr[2]==ipAddr[2] &&
			   user[i].ipAddr[3]==ipAddr[3])
			{
				nSameIp++;
			}
		}
		if(netcfg->multiConnLimit<=nSameIp)
		{
			user[clientId].state=FSUSERSTATE_PENDING;
			AddMessage("Disconnected due to too many access from the same IP.");
			SendTextMessage(clientId,"Too many access from the same IP.\n");
			FlushSendQueue(clientId,FS_NETTIMEOUT);

			user[clientId].state=FSUSERSTATE_NOTCONNECTED; // 2007/12/19
			Disconnect(clientId);
			return YSOK;
		}
	}



	FILE *fp=YsFileIO::Fopen(FsGetIpBlockFile(),"r");
	if(fp!=NULL)
	{
		YsString str,argv[4];
		while(str.Fgets(fp)!=NULL)
		{
			int i,argc;
			argc=1;
			for(i=0; str[i]!=0 && argc<=4; i++)
			{
				if(str[i]=='.')
				{
					argc++;
				}
				else if(str[i]=='#')
				{
					break;
				}
				else
				{
					argv[argc-1].Append(str[i]);
				}
			}

			if(argc==4)
			{
				for(i=0; i<argc; i++)
				{
					if(argv[i][0]!='*' && (unsigned int)atoi(argv[i])!=ipAddr[i])
					{
						goto NEXTLINE;
					}
				}
				// All four matched.
				AddMessage("Matched IP-Blocking List.  Disconnecting.\n");
				Disconnect(clientId);
				fclose(fp);
				return YSOK;
			}
		NEXTLINE:
			;
		}
		fclose(fp);
	}

	AddMessage("Waiting for login signal");

	// Clear user-store to prepare for login request
	user[clientId].Initialize();
	user[clientId].logOnTimeOut=time(NULL)+netcfg->logOnTimeOut;

	user[clientId].ipAddr[0]=ipAddr[0];
	user[clientId].ipAddr[1]=ipAddr[1];
	user[clientId].ipAddr[2]=ipAddr[2];
	user[clientId].ipAddr[3]=ipAddr[3];

	return YSOK;
}

YSRESULT FsSocketServer::BroadcastAirplaneState(void)
{
	static int broadcastCounter=0;
	int n;
	unsigned int packetSize,packetSizeShort,turretPackSize;
	unsigned char dat[256],datShort[256],turretCmd[256];

	n=sim->GetNumAirplane();
	if(n>0)
	{
		int idOnSvr;
		FsAirplane *air;

		air=NULL;
		while((air=sim->FindNextAirplane(air))!=NULL)
		{
			idOnSvr=FsExistence::GetSearchKey(air);

			if(air->Prop().IsAlive()==YSTRUE)
			{
				packetSize=air->Prop().NetworkEncode(dat,idOnSvr,sim->currentTime,YSFALSE);
				packetSizeShort=air->Prop().NetworkEncode(datShort,idOnSvr,sim->currentTime,YSTRUE);

				if(air->Prop().TurretStateChanged()==YSTRUE)
				{
					air->Prop().SaveTurretState();
					turretPackSize=air->Prop().EncodeTurretState(turretCmd,idOnSvr);
				}
				else
				{
					turretPackSize=0;
				}
			

				int userId;
				for(userId=0; userId<FS_MAX_NUM_USER; userId++)
				{
					if(user[userId].state==FSUSERSTATE_LOGGEDON && air!=user[userId].air)
					{
						if(/* user[userId].version>=20060805 && */packetSizeShort>0)
						{
							SendPacket(userId,packetSizeShort,datShort);
						}
						else
						{
							SendPacket(userId,packetSize,dat);
						}

						if(turretPackSize>0 /* && user[userId].version>=20050701 */)
						{
							SendPacket(userId,turretPackSize,turretCmd);
						}
					}
				}
			}
		}
		broadcastCounter=(broadcastCounter+1)%n;
	}

	return YSOK;
}

YSRESULT FsSocketServer::RectifyIllegalMissiles(void)
{
	int userId;
	for(userId=0; userId<FS_MAX_NUM_USER; userId++)
	{
		if(user[userId].state==FSUSERSTATE_LOGGEDON &&
		   user[userId].air!=NULL &&
		   user[userId].air->IsAlive()==YSTRUE)
		{
			if(user[userId].air->Prop().GetNumWeapon(FSWEAPON_AIM9)>0 ||
			   user[userId].air->Prop().GetNumWeapon(FSWEAPON_AIM120)>0 ||
			   user[userId].air->Prop().GetNumWeapon(FSWEAPON_AIM9X)>0 ||
			   user[userId].air->Prop().GetNumWeapon(FSWEAPON_AGM65)>0)
			{
				int idOnSvr;
				idOnSvr=FsExistence::GetSearchKey(user[userId].air);
				if(user[userId].version>=20101211)
				{
					SendAirCmd(userId,idOnSvr,"ULOADAAM");
					SendAirCmd(userId,idOnSvr,"ULOADAGM");
				}
				else
				{
					SendAirCmd(userId,idOnSvr,"INITIAAM 0");
					SendAirCmd(userId,idOnSvr,"INITIAGM 0");
					if(user[userId].version>=20050207)
					{
						SendAirCmd(userId,idOnSvr,"INITAAMM 0");
					}
				}
				SendUseMissile(userId,YSFALSE);
			}
		}
	}

	FsAirplane *playerPlane;
	playerPlane=sim->GetPlayerAirplane();
	if(playerPlane!=NULL && 
	   (playerPlane->Prop().GetNumWeapon(FSWEAPON_AIM9)>0 ||
	    playerPlane->Prop().GetNumWeapon(FSWEAPON_AIM9X)>0))
	{
		playerPlane->SendCommand("ULOADAAM");

		int idOnSvr;
		idOnSvr=FsExistence::GetSearchKey(playerPlane);
		BroadcastAirCmd(idOnSvr,"ULOADAAM");
	}

	return YSOK;
}

YSRESULT FsSocketServer::BroadcastGroundState(void)
{
	unsigned int packetSize;
	unsigned char dat[256] /* ,confirmGndCmd[256],*confirmGndCmdPtr */;
	FsGround *gnd;
	int i,gndId;

	// confirmGndCmdPtr=confirmGndCmd;
	// FsPushInt(confirmGndCmdPtr,FSNETCMD_CONFIRMEXISTENCE);
	// FsPushInt(confirmGndCmdPtr,1);
	// FsPushInt(confirmGndCmdPtr,0);
	// nConfirm=0;

	gnd=NULL;
	gndId=0;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->netType==FSNET_LOCAL && gnd->Prop().IsAlive()==YSTRUE)
		{
			if(gnd->GetPosition()!=gnd->netPos ||
			   YsEqual(gnd->GetAttitude().h(),gnd->netAtt.h())!=YSTRUE ||
			   YsEqual(gnd->GetAttitude().p(),gnd->netAtt.p())!=YSTRUE ||
			   YsEqual(gnd->GetAttitude().b(),gnd->netAtt.b())!=YSTRUE ||
			   gnd->Prop().IsFiringAaa()!=gnd->netShootingAaa ||
			   gnd->Prop().IsFiringCannon()!=gnd->netShootingCannon ||
			   gnd->Prop().GetDamageTolerance()!=gnd->netDamageTolerance)
			{
				unsigned int packetSizeShort;
				unsigned char datShort[256];

				packetSize=gnd->Prop().NetworkEncode(dat,FsExistence::GetSearchKey(gnd),sim->currentTime,YSFALSE);
				packetSizeShort=gnd->Prop().NetworkEncode(datShort,FsExistence::GetSearchKey(gnd),sim->currentTime,YSTRUE);

				for(i=0; i<FS_MAX_NUM_USER; i++)
				{
					if(user[i].state==FSUSERSTATE_LOGGEDON)
					{
						if(packetSizeShort>0 /* && user[i].version>=20060805 */)
						{
							SendPacket(i,packetSizeShort,datShort);
						}
						else
						{
							SendPacket(i,packetSize,dat);
						}
					}
				}

				gnd->netPos=gnd->GetPosition();
				gnd->netAtt=gnd->GetAttitude();
				gnd->netShootingAaa=gnd->Prop().IsFiringAaa();
				gnd->netShootingCannon=gnd->Prop().IsFiringCannon();
				gnd->netDamageTolerance=gnd->Prop().GetDamageTolerance();
			}
			else
			{
				// if((gndId&63)==nBroadcastGndCount)
				// {
				// 	FsPushInt(confirmGndCmdPtr,gnd->GetSearchKey());
				// 	nConfirm++;
				// 	if(nConfirm>=56)  // Up to 61 is supposed to be ok.
				// 	{
				// 		packetSize=confirmGndCmdPtr-confirmGndCmd;
				// 		FsSetInt(confirmGndCmd+8,nConfirm);
				// 		BroadcastPacket(packetSize,confirmGndCmd,20050701);
				// 		confirmGndCmdPtr=confirmGndCmd+12;
				// 		nConfirm=0;
				// 	}
				// }
			}

			if(gnd->Prop().HasTurret()==YSTRUE && gnd->Prop().TurretStateChanged()==YSTRUE)
			{
				unsigned int turretPackSize;
				unsigned char turretCmd[256];
				gnd->Prop().SaveTurretState();
				turretPackSize=gnd->Prop().EncodeTurretState(turretCmd,FsExistence::GetSearchKey(gnd));
				if(turretPackSize>0)
				{
					for(i=0; i<FS_MAX_NUM_USER; i++)
					{
						if(user[i].state==FSUSERSTATE_LOGGEDON)
						{
							SendPacket(i,turretPackSize,turretCmd);
						}
					}
				}
			}
		}
		gndId++;
	}

	// if(nConfirm>0)
	// {
	// 	packetSize=confirmGndCmdPtr-confirmGndCmd;
	// 	FsSetInt(confirmGndCmd+8,nConfirm);
	// 	BroadcastPacket(packetSize,confirmGndCmd,20050701);
	// }

	nBroadcastGndCount=(nBroadcastGndCount+1)&63;

	return YSOK;
}

YSRESULT FsSocketServer::BroadcastReviveGround(void)
{
	unsigned char dat[256];
	FsSetInt(dat,FSNETCMD_REVIVEGROUND);  // 2004/09/04  <- Wrongfully written as FsSetShort
	BroadcastPacket(4,dat,20040520);

	FsGround *gnd;

	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		gnd->netDamageTolerance=gnd->Prop().GetDamageTolerance()+1;  // <- This will trigger BroadcastGroundState
		gnd->Settle(gnd->initPosition);  // 2004/09/03
		gnd->Settle(gnd->initAttitude);  // 2004/09/03
		gnd->motionPathIndex=0;  // 2004/09/03
	}

	return YSOK;
}

YSRESULT FsSocketServer::BroadcastAddAirplane(const FsAirplane *air,FSNETTYPE netType)
{
	int i,airId;
	unsigned packLength;
	unsigned char dat[256];

	airId=FsExistence::GetSearchKey(air);
	packLength=air->Prop().NetworkEncode(dat,airId,sim->currentTime,YSFALSE);

	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			YSBOOL isPlayer;
			isPlayer=(user[i].air==air ? YSTRUE : YSFALSE);
			SendAddAirplane(i,airId,netType,isPlayer);
			SendPacket(i,packLength,dat);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::BroadcastRemoveAirplane(int airId,YSBOOL explosion)
{
	int i;
	FsAirplane *air;

	air=sim->FindAirplane(airId);
	if(air!=NULL)  // 2003/12/11 for cleaning up.
	               //            This message may come after airplanes are deleted.
	{
		air->Prop().SetFlightState(FSDEAD,FSDIEDOF_NULL);
		air->netAlive=YSFALSE;   // 2002/11/13 anti-ghost
	}

	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			SendRemoveAirplane(i,airId,explosion);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::BroadcastAddGround(FsGround *gnd,FSNETTYPE netType)
{
	int i,gndId;

	gndId=FsExistence::GetSearchKey(gnd);
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			SendAddGround(i,gndId,netType);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::BroadcastRemoveGround(int gndId,YSBOOL explosion)
{
	int i;
	FsGround *gnd;

	gnd=sim->FindGround(gndId);
	gnd->Prop().SetState(FSGNDDEAD);

	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			SendRemoveGround(i,gndId,explosion);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::BroadcastStateChange(void)
{
	FsAirplane *air;

	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		unsigned airKey=FsExistence::GetSearchKey(air);

		if(air->netType==FSNET_LOCAL)
		{
			// Is this dead?
			if(air->netAlive==YSTRUE && air->IsAlive()!=YSTRUE)
			{
				BroadcastRemoveAirplane(airKey,YSFALSE);
				air->netAlive=air->IsAlive();
			}

			// Which airplane locked on which?
			if(sim->FindAirplane(air->Prop().GetAirTargetKey())!=air->netAirTarget)
			{
				if(sim->FindAirplane(air->Prop().GetAirTargetKey())!=NULL)
				{
					BroadcastLockOn(airKey,YSTRUE,air->Prop().GetAirTargetKey(),YSTRUE);
				}
				else
				{
					BroadcastLockOn(airKey,YSTRUE,-1,YSTRUE);
				}
				air->netAirTarget=sim->FindAirplane(air->Prop().GetAirTargetKey());
			}
			if(sim->FindGround(air->Prop().GetGroundTargetKey())!=air->netGndTarget)
			{
				if(sim->FindGround(air->Prop().GetGroundTargetKey())!=NULL)
				{
					BroadcastLockOn(airKey,YSTRUE,air->Prop().GetGroundTargetKey(),YSFALSE);
				}
				else
				{
					BroadcastLockOn(airKey,YSTRUE,-1,YSFALSE);
				}
				air->netGndTarget=sim->FindGround(air->Prop().GetGroundTargetKey());
			}
		}
	}

	FsGround *gnd;
	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		unsigned gndKey;
		gndKey=FsExistence::GetSearchKey(gnd);

		if(gnd->netType==FSNET_LOCAL)
		{
			// Is this dead?
			if(gnd->Prop().IsAlive()!=YSTRUE && gnd->netAlive==YSTRUE)
			{
				BroadcastRemoveGround(gndKey,YSFALSE);
				gnd->netAlive=gnd->Prop().IsAlive();
			}
		}
	}

	return YSOK;
}

YSRESULT FsSocketServer::BroadcastLockOn(int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir)
{
	int i;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state==FSUSERSTATE_LOGGEDON)
		{
			SendLockOn(i,id1,id1isAir,id2,id2isAir);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::BroadcastMissileLaunch(FsWeaponRecord &rec)
{
	int i;
	unsigned length;
	unsigned char dat[512];
	length=FsEncodeWeaponRecord(dat,sim,rec);
	if(length>0)
	{
		if(rec.type==FSWEAPON_AIM120 || rec.type==FSWEAPON_BOMB250)
		{
			BroadcastPacket(length,dat,20050207);

			length=0;
			for(i=0; i<FS_MAX_NUM_USER; i++)
			{
				if(user[i].state==FSUSERSTATE_LOGGEDON && user[i].version<20050207)
				{
					if(length==0)
					{
						FsWeaponRecord oldRec;
						oldRec=rec;
						if(oldRec.type==FSWEAPON_AIM120)
						{
							oldRec.type=FSWEAPON_AIM9;
						}
						if(oldRec.type==FSWEAPON_BOMB250)
						{
							oldRec.type=FSWEAPON_BOMB;
						}

						length=FsEncodeWeaponRecord(dat,sim,oldRec);
					}
					if(length>0)
					{
						SendPacket(i,length,dat);
					}
				}
			}
		}
		else if(rec.type==FSWEAPON_ROCKET)  // Rocket is supported after 20010624
		{
			BroadcastPacket(length,dat,20010624);
		}
		else
		{
			BroadcastPacket(length,dat,0);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::BroadcastGetDamage
    (FsExistence *victim,FsExistence *firedBy,int power,enum FSDIEDOF diedOf)
{
	unsigned char dat[256],*ptr;
	ptr=dat;
	FsPushInt(ptr,FSNETCMD_GETDAMAGE);

	int isAir,idOnSvr;
	EncodeObject(isAir,idOnSvr,victim);
	FsPushInt(ptr,isAir);
	FsPushInt(ptr,idOnSvr);

	EncodeObject(isAir,idOnSvr,firedBy);
	FsPushInt(ptr,isAir);
	FsPushInt(ptr,idOnSvr);

	FsPushShort(ptr,(short)power);
	FsPushShort(ptr,(short)diedOf);

	return BroadcastPacket(ptr-dat,dat,0);
}

// BroadcastAirCmd is available after 20010624
YSRESULT FsSocketServer::BroadcastAirCmd(int airId,const char cmd[])
{
	YSSIZE_T packetLength;
	unsigned char dat[256],*ptr;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_AIRCMD);
	FsPushInt(ptr,airId);
	strcpy((char *)(dat+8),cmd);

	packetLength=8+strlen(cmd)+1;
	return BroadcastPacket(packetLength,dat,20010624);
}

YSRESULT FsSocketServer::BroadcastChatTextMessage(const char txt[])
{
	if(chatLogFp.Fp()!=NULL)
	{
		fprintf(chatLogFp.Fp(),"%s\n",txt);
	}
	return BroadcastTextMessage(txt);
}

YSRESULT FsSocketServer::BroadcastTextMessage(const char txt[])
{
	YSSIZE_T packetLength;
	unsigned char dat[256];
	FsSetInt(dat  ,FSNETCMD_TEXTMESSAGE);
	FsSetInt(dat+4,0);  // Will be used for destination selection
	FsSetInt(dat+8,0);  // Will be used for source indication
	strncpy((char *)dat+12,txt,244);
	dat[255]=0;

	packetLength=YsSmaller <YSSIZE_T> ((12+strlen(txt)+1 +3)&~3,256);

	int i;
	YSBOOL oldVersion;
	oldVersion=YSFALSE;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			SendPacket(i,packetLength,dat);
		}
	}

	return YSOK;

//	return BroadcastPacket(packetLength,dat,20031211);
}

YSRESULT FsSocketServer::BroadcastWeaponConfig(int airId,YSSIZE_T nWeaponConfig,const int weaponConfig[])
{
	YSRESULT res;


	unsigned char dat[256];
	YSSIZE_T packetLength=FsEncodeWeaponConfig(dat,airId,nWeaponConfig,weaponConfig);
	res=BroadcastPacket(packetLength,dat,20040618);


	// Gun
	if(netcfg->useUnguidedWeapon!=YSTRUE)
	{
		BroadcastAirCmd(airId,"INITIGUN 0");
	}



	// For old versions:  Try its best.
	int i,aam,agm,bom,rkt;
	aam=0;
	agm=0;
	bom=0;
	rkt=0;
	for(i=0; i<nWeaponConfig-1; i+=2)
	{
		switch(weaponConfig[i])
		{
		case FSWEAPON_AIM9:
			aam++;
			break;
		case FSWEAPON_AGM65:
			agm++;
			break;
		case FSWEAPON_BOMB:
			bom++;
			break;
		case FSWEAPON_ROCKET:
			rkt++;
			break;
		}

	}

	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED && user[i].version<20040618)
		{
			char airCmd[256];
			sprintf(airCmd,"INITIAAM %d",aam);
			SendAirCmd(i,airId,airCmd);

			sprintf(airCmd,"INITIAGM %d",agm);
			SendAirCmd(i,airId,airCmd);

			sprintf(airCmd,"INITBOMB %d",bom);
			SendAirCmd(i,airId,airCmd);

			sprintf(airCmd,"INITRCKT %d",rkt);
			SendAirCmd(i,airId,airCmd);
		}
	}

	return res;
}

YSRESULT FsSocketServer::BroadcastSkyColor(YsColor col)
{
	unsigned char dat[16];
	FsSetInt(dat  ,FSNETCMD_SKYCOLOR);
	dat[4]=(unsigned char)col.Ri();
	dat[5]=(unsigned char)col.Gi();
	dat[6]=(unsigned char)col.Bi();
	return BroadcastPacket(7,dat,20040618);
}
YSRESULT FsSocketServer::BroadcastGroundColor(YsColor col)
{
	unsigned char dat[16];
	FsSetInt(dat  ,FSNETCMD_GNDCOLOR);
	dat[4]=(unsigned char)col.Ri();
	dat[5]=(unsigned char)col.Gi();
	dat[6]=(unsigned char)col.Bi();
	return BroadcastPacket(7,dat,20040618);
}
YSRESULT FsSocketServer::BroadcastFogColor(YsColor col)
{
	unsigned char dat[16];
	FsSetInt(dat  ,FSNETCMD_FOGCOLOR);
	dat[4]=(unsigned char)col.Ri();
	dat[5]=(unsigned char)col.Gi();
	dat[6]=(unsigned char)col.Bi();
	return BroadcastPacket(7,dat,20040618);
}

YSRESULT FsSocketServer::BroadcastForceJoin(void)
{
	unsigned char dat[16];
	FsSetInt(dat,FSNETCMD_SERVER_FORCE_JOIN);
	return BroadcastPacket(4,dat,2040618);
}

YSBOOL FsSocketServer::ReceivedKillServer(void)
{
	return receivedKillServer;
}

YSRESULT FsSocketServer::ReceiveLockOn(int clientId,unsigned char dat[],unsigned packetLength)
{
	int i;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state==FSUSERSTATE_LOGGEDON && i!=clientId)
		{
			SendPacket(i,packetLength,dat);
		}
	}

	int id1,id2;
	YSBOOL id1isAir,id2isAir;
	const unsigned char *ptr=dat;
	FsPopInt(ptr);
	id1=             FsPopInt(ptr);
	id1isAir=(YSBOOL)FsPopInt(ptr);
	id2=             FsPopInt(ptr);
	id2isAir=(YSBOOL)FsPopInt(ptr);

	if(id1isAir==YSTRUE && id2isAir==YSTRUE)
	{
		FsAirplane *air1,*air2;
		air1=sim->FindAirplane(id1);
		air2=sim->FindAirplane(id2);
		if(air1!=NULL)
		{
			if(id2>=0)
			{
				air1->Prop().SetAirTargetKey(FsExistence::GetSearchKey(air2));
			}
			else
			{
				air1->Prop().SetAirTargetKey(YSNULLHASHKEY);
			}
		}
	}
	else if(id1isAir==YSTRUE && id2isAir==YSFALSE)
	{
		FsAirplane *air1;
		FsGround *gnd2;
		air1=sim->FindAirplane(id1);
		gnd2=sim->FindGround(id2);
		if(air1!=NULL)
		{
			if(id2>=0)
			{
				air1->Prop().SetGroundTargetKey(FsExistence::GetSearchKey(gnd2));
			}
			else
			{
				air1->Prop().SetGroundTargetKey(YSNULLHASHKEY);
			}
		}
	}
	else if(id1isAir==YSFALSE && id2isAir==YSTRUE)
	{
		FsGround *gnd1;
		FsAirplane *air2;
		gnd1=sim->FindGround(id1);
		air2=sim->FindAirplane(id2);
		if(gnd1!=NULL)
		{
			if(id2>=0)
			{
				gnd1->Prop().SetAirTarget(air2);
			}
			else
			{
				gnd1->Prop().SetAirTarget(NULL);
			}
		}
	}
	else if(id1isAir==YSFALSE && id2isAir==YSFALSE)
	{
		// Ground to Ground, not supported
		// FsGround *gnd1,*gnd2;
		// gnd1=sim->GetGround(id1);
		// gnd2=sim->GetGround(id2);
		// if(gnd1!=NULL)
		// {
		// 	gnd1->Prop().SetGroundTarget(air2);
		// }
	}

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveRemoveAirplane(int clientId,unsigned char dat[],unsigned packetLength)
{
	int idOnSvr;
	int flags;
	FsAirplane *air;

	idOnSvr=FsGetInt(dat+4);
	flags=FsGetShort(dat+8);

	SendRemoveAirplaneReadBack(clientId,idOnSvr);

	air=sim->FindAirplane(idOnSvr);

	if(air!=NULL)
	{
		if((flags&1)!=0)
		{
			// Add explosion here
		}

		air->Prop().SetFlightState(FSDEAD,FSDIEDOF_NULL);
		air->netAlive=YSFALSE;  // 2002/11/13 anti-ghost
		ForwardPacket(clientId,packetLength,dat);
		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsSocketServer::ReceiveRemoveGround(int clientId,unsigned char dat[],unsigned)
{
	int idOnSvr;
	int flags;
	FsGround *gnd;

	idOnSvr=FsGetInt(dat+4);
	flags=FsGetShort(dat+8);

	SendRemoveGroundReadBack(clientId,idOnSvr);

	gnd=sim->FindGround(idOnSvr);
	if(gnd!=NULL)
	{
		if((flags&1)!=0)
		{
			// Add explosion here
		}

		gnd->Prop().SetState(FSGNDDEAD);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsSocketServer::ReceiveMissileLaunch(int clientId,unsigned char dat[],unsigned packetLength)
{
	int firedBy;
	YSBOOL firedByAirplane;
	int firedAt;
	YSBOOL firedAtAirplane;
	FsWeaponRecord rec;

	// printf("Receive Missile Launch\n");

	if(FsDecodeWeaponRecord(rec,firedBy,firedByAirplane,firedAt,firedAtAirplane,dat,sim)==YSOK)
	{
		FsExistence *owner,*target;
		if(firedBy>=0)
		{
			if(firedByAirplane==YSTRUE)
			{
				owner=sim->FindAirplane(firedBy);
			}
			else
			{
				owner=sim->FindGround(firedBy);
			}
		}
		else
		{
			owner=NULL;
		}

		if(firedAt>=0)
		{
			if(firedAtAirplane==YSTRUE)
			{
				target=sim->FindAirplane(firedAt);
			}
			else
			{
				target=sim->FindGround(firedAt);
			}
		}
		else
		{
			target=NULL;
		}

		rec.firedBy=owner;
		rec.target=target;
		sim->NetWeaponLaunch(rec);


		if(rec.type!=FSWEAPON_AIM120 && rec.type!=FSWEAPON_BOMB250)
		   // AIM120 and BOMB250 becomes available only after 20050207 net-version.
		{
			return ForwardPacket(clientId,packetLength,dat);
		}
		else
		{
			int i;
			unsigned packLng20040618;
			unsigned char pack20040618[512];

			packLng20040618=0;

			for(i=0; i<FS_MAX_NUM_USER; i++)
			{
				if(user[i].state==FSUSERSTATE_LOGGEDON && i!=clientId)
				{
					if(user[i].version>=20050207)
					{
						SendPacket(i,packetLength,dat);
					}
					else
					{
						if(packLng20040618==0)
						{
							FsWeaponRecord oldRec;
							oldRec=rec;
							if(oldRec.type==FSWEAPON_AIM120)
							{
								oldRec.type=FSWEAPON_AIM9;
							}
							else if(oldRec.type==FSWEAPON_BOMB250)
							{
								oldRec.type=FSWEAPON_BOMB;
							}
							packLng20040618=FsEncodeWeaponRecord(pack20040618,sim,oldRec);
						}
						if(packLng20040618>0)
						{
							SendPacket(i,packLng20040618,pack20040618);
						}
					}
				}
			}
		}
	}
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveGroundState(int clientId,unsigned char dat[],unsigned packetLength)
{
	int idOnSvr;
	FsGround *gnd;

	idOnSvr=FsGetInt(dat+8);
	gnd=sim->FindGround(idOnSvr);
	if(gnd!=NULL)
	{
		gnd->Prop().NetworkDecode(packetLength,dat,gnd->netClockRemote,gnd->netClockLocal,sim->currentTime);
		return ForwardPacket(clientId,packetLength,dat);
	}
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveGetDamage(int clientId,unsigned char dat[],unsigned packetLength)
{
	FsExistence *victim,*firedBy;
	int victimIsAir,firedByAir;
	int victimId,firedById;
	int damage;
	FSDIEDOF diedOf;
	YSBOOL killed;

	FSWEAPONTYPE weaponType;

	const unsigned char *ptr=dat;
	FsPopInt(ptr);   // Skip FSNETCMD
	victimIsAir=FsPopInt(ptr);
	victimId=FsPopInt(ptr);
	DecodeObject(&victim,victimIsAir,victimId);

	firedByAir=FsPopInt(ptr);
	firedById=FsPopInt(ptr);
	DecodeObject(&firedBy,firedByAir,firedById);

	damage=FsPopShort(ptr);
	diedOf=(FSDIEDOF)FsPopShort(ptr);

	if(20100630<=user[clientId].version)
	{
		weaponType=(FSWEAPONTYPE)FsPopShort(ptr);
	}
	else
	{
		weaponType=FSWEAPON_NULL;
	}

	if(victim!=NULL)
	{
		if(victim->GetDamage(killed,damage,diedOf)==YSTRUE)
		{
			double rad;
			rad=victim->GetApproximatedCollideRadius();
			if(killed==YSTRUE)
			{
				ReportKill(victim,firedBy,weaponType);
				// Add explosion and sound
			}
			else
			{
				// Add small explosion and sound
			}
		}
	}

	return ForwardPacket(clientId,packetLength,dat);
}

YSRESULT FsSocketServer::ReceiveRequestToBeSideWindow(int clientId,unsigned char dat[],unsigned /*packetLength*/)
{
	double hdg,pch;
	FsAirplane *playerPlane;

	playerPlane=sim->GetPlayerAirplane();
	if(playerPlane!=NULL)
	{
		const unsigned char *ptr=dat;
		FsPopInt(ptr);
		hdg=FsPopFloat(ptr);
		pch=FsPopFloat(ptr);

		SendAssignSideWindow(clientId,FsExistence::GetSearchKey(playerPlane),hdg,pch);
	}
	else
	{
		SendError(clientId,FSNETERR_REJECT);
	}

	return YSOK;
}

YSRESULT FsSocketServer::ConnectionClosedByClient(int clientId)
{
	char str[256];
	sprintf(str,"Connection Closed By #%d",clientId);
	AddMessage(str);
#ifdef CRASHINVESTIGATION
	FsLogOutput(FSSVRLOGFILE,str);
#endif

	if(user[clientId].air!=NULL)
	{
		BroadcastRemoveAirplane(FsExistence::GetSearchKey(user[clientId].air),YSTRUE);
	}

	user[clientId].state=FSUSERSTATE_NOTCONNECTED;
	user[clientId].air=NULL;

	return YSOK;
}

YSRESULT FsSocketServer::DisconnectInactiveUser(const double &passedTime,const double &timeOut)
{
	int i;

	// 2009/07/10 Additional protection from mass disconnection >>
	if(passedTime>10.0)
	{
		return DisconnectInactiveUser(10.0,timeOut);
	}
	// 2009/07/10 Additional protection from mass disconnection <<

	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			double prevNoActiveTime;
			prevNoActiveTime=user[i].noActivityTime;
			user[i].noActivityTime+=passedTime;

			if(prevNoActiveTime<60.0 && 60.0<=user[i].noActivityTime)
			{
				user[i].noActivityTime=60.1;  // 2009/07/06 Preventive

				SendTextMessage(i,"No activity for 60 seconds.");
				SendTextMessage(i,"Please do something, or you will be disconnected in 120 seconds.");

				SendControlShowUserName(i,netcfg->serverControlShowUserName);  // 2009/07/06 Test: Let client read back
			}
			else if(prevNoActiveTime<120.0 && 120.0<=user[i].noActivityTime)
			{
				user[i].noActivityTime=120.1;  // 2009/07/06 Preventive

				SendTextMessage(i,"No activity for 120 seconds.");
				SendTextMessage(i,"Please do something, or you will be disconnected in 60 seconds.");

				SendControlShowUserName(i,netcfg->serverControlShowUserName);  // 2009/07/06 Test: Let client read back
			}
			else if(180.0<user[i].noActivityTime)
			{
				char str[256];
				sprintf(str,"User #%d has been disconnected due to no activity.",i);
				AddMessage(str);
				user[i].noActivityTime=0.0;
				DisconnectUser(i);
			}
		}
	}
	return YSOK;
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
	if(nMsg<MAXNUMMESSAGE)
	{
		strcpy(msg[nMsg],txt);
		nMsg++;
		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

void FsSocketServer::ReportKill(FsExistence *killed,FsExistence *scored,FSWEAPONTYPE wpnType)
{
	FsNetworkScoreLog newKill;

	newKill.weaponType=wpnType;
	newKill.scoreTime=sim->currentTime;
	newKill.killedClientId=-2;   // -1 -> Server   -2 -> Non-Player
	newKill.killedKey=YSNULLHASHKEY;
	newKill.scoredClientId=-2;   // -1 -> Server   -2 -> Non-Player
	newKill.scoredKey=YSNULLHASHKEY;

	if(NULL!=killed)
	{
		newKill.killedKey=FsExistence::GetSearchKey(killed);
		switch(killed->GetType())
		{
		default:
			break;
		case FSEX_AIRPLANE:
			{
				FsAirplane *air=(FsAirplane *)killed;
				newKill.killedVehicleType.Set(air->Prop().GetIdentifier());
			}
			break;
		case FSEX_GROUND:
			{
				FsGround *gnd=(FsGround *)killed;
				newKill.killedVehicleType.Set(gnd->Prop().GetIdentifier());
			}
			break;
		}
	}
	if(NULL!=scored)
	{
		newKill.scoredKey=FsExistence::GetSearchKey(scored);
		switch(scored->GetType())
		{
		default:
			break;
		case FSEX_AIRPLANE:
			{
				FsAirplane *air=(FsAirplane *)scored;
				newKill.scoredVehicleType.Set(air->Prop().GetIdentifier());
			}
			break;
		case FSEX_GROUND:
			{
				FsGround *gnd=(FsGround *)scored;
				newKill.scoredVehicleType.Set(gnd->Prop().GetIdentifier());
			}
			break;
		}
	}



	int clientId;
	for(clientId=0; clientId<FS_MAX_NUM_USER; clientId++)
	{
		if(NULL!=killed && user[clientId].state==FSUSERSTATE_LOGGEDON && user[clientId].air==killed)
		{
			newKill.killedClientId=clientId;
			newKill.killedUsername=user[clientId].username;
		}
		if(NULL!=scored && user[clientId].state==FSUSERSTATE_LOGGEDON && user[clientId].air==scored)
		{
			newKill.scoredClientId=clientId;
			newKill.scoredUsername=user[clientId].username;
		}
	}
	if(NULL!=killed && -2==newKill.killedClientId && killed->netType==FSNET_LOCAL && sim->GetPlayerAirplane()==killed)
	{
		newKill.killedClientId=-1;
		newKill.killedUsername=username;
	}
	if(NULL!=scored && -2==newKill.scoredClientId && scored->netType==FSNET_LOCAL && sim->GetPlayerAirplane()==scored)
	{
		newKill.scoredClientId=-1;
		newKill.scoredUsername=username;
	}



	if(0<=newKill.killedClientId)
	{
		SendReportScore(newKill.killedClientId,YSFALSE,newKill);
	}
	else if(-1==newKill.killedClientId)
	{
		this->killed.Append(newKill);

		YsString msg;
		newKill.FormatKilledMessage(msg);
		AddMessage(msg);
		sim->AddTimedMessage(msg);
	}

	if(0<=newKill.scoredClientId)
	{
		SendReportScore(newKill.scoredClientId,YSTRUE,newKill);
	}
	else if(-1==newKill.scoredClientId)
	{
		this->scored.Append(newKill);

		YsString msg;
		newKill.FormatScoredMessage(msg);
		AddMessage(msg);
		sim->AddTimedMessage(msg);
	}

	printf("Client %d is killed by Client %d\n",newKill.killedClientId,newKill.scoredClientId);
}

YSRESULT FsSocketServer::ReceiveLogOnUser(int clientId,int version,const char recvUsername[])
{
	char username[256];

	if(strcmp(recvUsername,"USERNAME")==0 || recvUsername[0]==0)
	{
		strcpy(username,"nameless");
	}
	else
	{
		strncpy(username,recvUsername,248);
		username[248]=0;
	}

	if(locked==YSTRUE)
	{
		char str[256];
		sprintf(str,"User %s is rejected because the server is locked.",username);
		AddMessage(str);

		SendTextMessage(clientId,"The server is locked.\n");
		FlushSendQueue(clientId,FS_NETTIMEOUT);

		DisconnectUser(clientId);
		return YSERR;
	}


	YSRESULT versionCheck;
	versionCheck=YSERR;
	if(version==YSFLIGHT_NETVERSION)
	{
		versionCheck=YSOK;
	}
	else if(netcfg->serverAcceptSameVersionOnly!=YSTRUE)
	{
		if(version==20080220)
		{
			versionCheck=YSOK;
		}
	}


	if(versionCheck==YSOK)
	{
		user[clientId].state=FSUSERSTATE_PENDING;
		user[clientId].username.Set(username);
		if(user[clientId].username.Strlen()>200)
		{
			user[clientId].username.SetLength(200);
		}
		user[clientId].air=NULL;
		user[clientId].version=version;

		user[clientId].sendCriticalInfoTimer=sim->currentTime+0.5;

		FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);

		SendVersionNotify(clientId);
		SendUseMissile(clientId,netcfg->useMissile);
		SendUseUnguidedWeapon(clientId,netcfg->useUnguidedWeapon);
		SendControlShowUserName(clientId,netcfg->serverControlShowUserName);

		char str[256];
		YsString strBuf;
		if(netcfg->serverControlRadarAlt==YSTRUE)
		{
			sprintf(str,"RADARALTI %.2lfm",sim->GetConfig().radarAltitudeLimit);
			strBuf.Set(str);
			user[clientId].configStringToSend.Append(strBuf);
			SendConfigString(clientId,str);
		}
		if(netcfg->disableThirdAirplaneView==YSTRUE)
		{
			strBuf.Set("NOEXAIRVW TRUE");
			user[clientId].configStringToSend.Append(strBuf);
			SendConfigString(clientId,"NOEXAIRVW TRUE");
		}



		sprintf(str,"User %s logged on.",username);
		AddMessage(str);
	#ifdef CRASHINVESTIGATION
		FsLogOutput(FSSVRLOGFILE,str);
	#endif

		// Let client Load Fields
		YsString fieldNameBuf;
		const char *fieldName;
		YsVec3 vec;
		YsAtt3 att;

		if(sim->GetLoadedField(fieldNameBuf,vec,att)==YSOK)
		{
			fieldName=fieldNameBuf;

			FsNetworkFldToSend fldToSend;
			fldToSend.fldName.Set(fieldName);
			fldToSend.pos=vec;
			fldToSend.att=att;
			user[clientId].fldToSend.Append(fldToSend);

			if(SendLoadField(clientId,fieldName,YSFALSE,vec,att)!=YSOK)
			{
				AddMessage("Failed to send field list.");
				DisconnectUser(clientId);
				return YSERR;
			}
		}



		YsString airName;
		int id;
		for(id=0; sim->world->GetAirplaneTemplateName(airName,id)==YSOK; id++)
		{
			user[clientId].airTypeToSend.Append(airName);
		}
		SendAirplaneList(clientId,32);  // <- Version check is done inside.



		// Let client know the state of all air objects
		FsGround *gnd;
		gnd=NULL;
		while((gnd=sim->FindNextGround(gnd))!=NULL)
		{
			if(gnd->Prop().IsAlive()==YSTRUE)
			{
				user[clientId].gndToSend.Append(FsExistence::GetSearchKey(gnd));

//				SendAddGround(clientId,gnd->GetSearchKey(),FSNET_REMOTE);
				// if(gnd->motionPath!=NULL)
				// {
				// 	SendAssignMotionPath(clientId,gnd);
				// }
			}
		}


		FsAirplane *air;
		air=NULL;
		while((air=sim->FindNextAirplane(air))!=NULL)
		{
			if(air->Prop().IsAlive()==YSTRUE)
			{
				user[clientId].airToSend.Append(FsExistence::GetSearchKey(air));

//				SendAddAirplane(clientId,air->GetSearchKey(),FSNET_REMOTE,YSFALSE);
//				YsArray <int,64> weaponConfig;
//				air->Prop().GetWeaponLoadingConfiguration(weaponConfig);
//				SendWeaponConfig(clientId,air->GetSearchKey(),weaponConfig.GetN(),weaponConfig);
			}
		}

		return YSOK;
	}
	else
	{
		user[clientId].state=FSUSERSTATE_PENDING;

		SendVersionNotify(clientId);

		SendError(clientId,FSNETERR_VERSIONCONFLICT);

		AddMessage("Connection is rejected because of versoin conflict.");
		char str[256];
		sprintf(str,"  SERVER NET-VERSION : %d",YSFLIGHT_NETVERSION);
		AddMessage(str);
		sprintf(str,"  CLIENT NET-VERSION : %d",version);
		AddMessage(str);

		FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
		DisconnectUser(clientId);
		return YSERR;
	}
}

YSRESULT FsSocketServer::ReceiveError(int clientId,int errorCode)
{
	char msg[256];
	sprintf(msg,"ERROR code received from client [%d]",clientId);

	switch(errorCode)
	{
	case FSNETERR_VERSIONCONFLICT:
		AddMessage("ERROR : Version Conflict.");
		break;
	case FSNETERR_CANNOTADDOBJECT:
		AddMessage("ERROR : Cannot add object.");
		break;
	case FSNETERR_REJECT:
		AddMessage("ERROR : Request is rejected.");
		break;
	case FSNETERR_CANNOTSUSTAIN:
		sprintf(msg,"ERROR : Fatal error in the client %d.",clientId);
		AddMessage(msg);
		break;
	default:
		sprintf(msg,"ERROR : Eror code %d",errorCode);
		AddMessage(msg);
		break;
	}
	return YSOK;
}

YSRESULT FsSocketServer::ReceiveLoadFieldReadBack(int clientId,unsigned char dat[])
{
	YSBOOL loadYFS;
	double x,y,z,h,p,b;
	unsigned flag;
	YsVec3 pos;
	YsAtt3 att;

	flag=FsGetUnsignedInt(dat+36);
	x=FsGetFloat(dat+40);
	y=FsGetFloat(dat+44);
	z=FsGetFloat(dat+48);
	h=FsGetFloat(dat+52);
	p=FsGetFloat(dat+56);
	b=FsGetFloat(dat+60);
	loadYFS=((flag&1)!=0 ? YSTRUE : YSFALSE);

	pos.Set(x,y,z);
	att.Set(h,p,b);

	if(FsVerboseMode==YSTRUE)
	{
		printf("Received Field Read Back From #%d\n",clientId);
		printf("[%s]\n",dat+4);
		printf("%d -> ",(int)user[clientId].fldToSend.GetN());
	}

	int i;
	forYsArray(i,user[clientId].fldToSend)
	{
		if(strcmp(user[clientId].fldToSend[i].fldName,(char *)dat+4)==0 &&
		   pos==user[clientId].fldToSend[i].pos &&
		   att==user[clientId].fldToSend[i].att)
		{
			user[clientId].fldToSend.DeleteBySwapping(i);
		}
	}

	if(FsVerboseMode==YSTRUE)
	{
		printf("%d\n",(int)user[clientId].fldToSend.GetN());
	}

	CheckAndSendPendingData(clientId,sim->currentTime,1.5);

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveConfigStringReadBack(int clientId,unsigned char dat[])
{
	char *str;
	str=(char *)dat+8;

	if(FsVerboseMode==YSTRUE)
	{
		printf("Received Config String Read Back from #%d\n",clientId);
		printf("[%s]\n",str);
		printf("%d -> ",(int)user[clientId].configStringToSend.GetN());
	}

	int i;
	forYsArray(i,user[clientId].configStringToSend)
	{
		if(strcmp(str,user[clientId].configStringToSend[i])==0)
		{
			user[clientId].configStringToSend.DeleteBySwapping(i);
		}
	}

	if(FsVerboseMode==YSTRUE)
	{
		printf("%d\n",(int)user[clientId].configStringToSend.GetN());
	}

	// CheckAndSendPendingData(clientId,sim->currentTime,1.5);

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveListReadBack(int clientId,unsigned char dat[])
{
	int nList,listType;
	int i;
	char *ptr;
	YsString str;

	listType=dat[4];
	nList=dat[5];
	ptr=(char *)(dat+8);

	if(FsVerboseMode==YSTRUE)
	{
		printf("Receive List Read Back\n");
		printf("List Type=%d\n",listType);
		printf("Num Item=%d\n",nList);
	}

	switch(listType)
	{
	case 1:
		if(FsVerboseMode==YSTRUE)
		{
			printf("Air To Send %d -> ",(int)user[clientId].airTypeToSend.GetN());
		}

		for(i=0; i<nList; i++)
		{
			int l,j;
			l=(int)strlen(ptr);
			// printf("%s\n",ptr);

			str.Set(ptr);
			for(j=(int)user[clientId].airTypeToSend.GetN()-1; j>=0; j--)
			{
				if(strcmp(user[clientId].airTypeToSend[j],ptr)==0)
				{
					user[clientId].airTypeToSend.Delete(j);
				}
			}

			ptr+=(l+1);
		}

		if(FsVerboseMode==YSTRUE)
		{
			printf("%d\n",(int)user[clientId].airTypeToSend.GetN());
		}

		CheckAndSendPendingData(clientId,sim->currentTime,1.5);

		break;
	case 2:
		break;
	}

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveReadBack(int clientId,unsigned char dat[])
{
	YSSIZE_T i;
	int readBackType,readBackParam;
	const unsigned char *ptr=dat;

	FsPopInt(ptr);  // Skip FSNETCMD
	readBackType=FsPopShort(ptr);
	FsPopShort(ptr);  // Skip Padding
	readBackParam=FsPopInt(ptr);

	switch(readBackType)
	{
	case FSNETREADBACK_ADDAIRPLANE:
		{
			int idOnSvr;
			idOnSvr=readBackParam;
			forYsArrayRev(i,user[clientId].airToSend)
			{
				if(user[clientId].airToSend[i]==idOnSvr)
				{
					user[clientId].airToSend.DeleteBySwapping(i);
					if(FsVerboseMode==YSTRUE)
					{
						printf("Receive Add Airplane Read Back from #%d (%d remain)\n",
						    clientId,(int)user[clientId].airToSend.GetN());
					}
				}

				if(user[clientId].joinSequence==1 &&
				   user[clientId].air!=NULL &&
				   FsExistence::GetSearchKey(user[clientId].air)==idOnSvr)
				{
					user[clientId].joinSequence++;
					SendJoinApproval(clientId);
					if(FsVerboseMode==YSTRUE)
					{
						printf("Client #%d Join Sequence %d\n",clientId,user[clientId].joinSequence);
					}
				}
			}
		}
		break;
	case FSNETREADBACK_ADDGROUND:
		{
			int idOnSvr;
			idOnSvr=readBackParam;
			forYsArrayRev(i,user[clientId].gndToSend)
			{
				if(user[clientId].gndToSend[i]==idOnSvr)
				{
					user[clientId].gndToSend.DeleteBySwapping(i);
					if(FsVerboseMode==YSTRUE)
					{
						printf("Receive Add Ground Read Back from #%d (%d remain)\n",
						    clientId,(int)user[clientId].gndToSend.GetN());
					}
				}
			}
		}
		break;
	case FSNETREADBACK_REMOVEAIRPLANE:
		{
			int idOnSvr;
			idOnSvr=readBackParam;
			if(FsVerboseMode==YSTRUE)
			{
				printf("Receive Remove Object (Air) Read Back from #%d\n",clientId);
			}
			forYsArrayRev(i,user[clientId].airRmvToSend)
			{
				if(user[clientId].airRmvToSend[i]==idOnSvr)
				{
					user[clientId].airRmvToSend.DeleteBySwapping(i);
				}
			}
		}
		break;
	case FSNETREADBACK_REMOVEGROUND:
		{
			int idOnSvr;
			idOnSvr=readBackParam;
			if(FsVerboseMode==YSTRUE)
			{
				printf("Receive Remove Object (Gnd) Read Back from #%d\n",clientId);
			}
			forYsArrayRev(i,user[clientId].gndRmvToSend)
			{
				if(user[clientId].gndRmvToSend[i]==idOnSvr)
				{
					user[clientId].gndRmvToSend.DeleteBySwapping(i);
				}
			}
		}
		break;
	case FSNETREADBACK_ENVIRONMENT:
		if(FsVerboseMode==YSTRUE)
		{
			printf("Received Environment Read Back from #%d\n",clientId);
		}
		user[clientId].environmentReadBack=YSTRUE;
		break;
	case FSNETREADBACK_JOINREQUEST:
		printf("Fatal Error:  FSNETREADBACK_JOINREQUEST is sent to the server.\n");
		DisconnectUser(clientId);
		break;
	case FSNETREADBACK_JOINAPPROVAL:
		user[clientId].joinSequence=0;  // Concludes join sequence.
		if(FsVerboseMode==YSTRUE)
		{
			printf("Client #%d Join Sequence Completed\n",clientId);
		}
		break;
	case FSNETREADBACK_PREPARE:
		if(FsVerboseMode==YSTRUE)
		{
			printf("Received Preparation Read Back from #%d\n",clientId);
		}
		user[clientId].preparationReadBack=YSTRUE;
		break;
	case FSNETREADBACK_USEMISSILE:
		if(FsVerboseMode==YSTRUE)
		{
			printf("Received UseMissile Read Back from #%d\n",clientId);
		}
		user[clientId].useMissileReadBack=YSTRUE;
		break;
	case FSNETREADBACK_USEUNGUIDEDWEAPON:
		if(FsVerboseMode==YSTRUE)
		{
			printf("Received UseUnguidedWeapon Read Back from #%d\n",clientId);
		}
		user[clientId].useUnguidedWeaponReadBack=YSTRUE;
		break;
	case FSNETREADBACK_CTRLSHOWUSERNAME:
		if(FsVerboseMode==YSTRUE)
		{
			printf("Received ControlShowUserName Read Back from #%d\n",clientId);
		}
		user[clientId].controlShowUserNameReadBack=YSTRUE;
		break;
	default:
		printf("Fatal Error: Unrecognized read back is sent to the server.\n");
		DisconnectUser(clientId);;
		break;
	}

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveSmokeColor(int clientId,unsigned char cmdTop[],unsigned int packetLength)
{
	int idOnSvr,smkNum,r,g,b;
	const unsigned char *ptr=cmdTop;
	FsAirplane *air;

	FsPopInt(ptr);  // Skip FSNETCMD
	idOnSvr=FsPopInt(ptr);
	smkNum=FsPopUnsignedChar(ptr);
	r=FsPopUnsignedChar(ptr);
	g=FsPopUnsignedChar(ptr);
	b=FsPopUnsignedChar(ptr);

	if(NULL!=(air=sim->FindAirplane(idOnSvr)))
	{
		char str[256];
		sprintf(str,"SMOKECOL %d %d %d %d",smkNum,r,g,b);
		air->SendCommand(str);

		int i;
		for(i=0; i<FS_MAX_NUM_USER; i++)
		{
			if(user[i].state!=FSUSERSTATE_NOTCONNECTED && i!=clientId)
			{
				SendPacket(i,packetLength,cmdTop);
			}
		}

		return YSOK;
	}

	return YSERR;
}

static void MakeTakeOffMessage(YsString &msg,const char username[],const char airname[])
{
	char unm[256],typ[256];

	strncpy(unm,username,32);
	unm[32]=0;

	strncpy(typ,airname,32);
	typ[32]=0;

	msg.Set(unm);
	msg.Append(" took off (");
	msg.Append(typ);
	msg.Append(")");
}

static void MakeUnjoinMessage(YsString &msg,const char username[])
{
	char unm[256];
	strncpy(unm,username,32);
	unm[32]=0;

	msg.Set(unm);
	msg.Append(" has left the airplane.");
}

YSRESULT FsSocketServer::ReceiveJoinRequest(int clientId,unsigned char dat[],unsigned packetLength)
{
	if(user[clientId].state==FSUSERSTATE_LOGGEDON)
	{
		FSIFF iff;
		char airId[32],stp[32];
		unsigned int airDatPacketLength;
		unsigned char airDat[256];

		// Read Back >>
		SendReadBack(clientId,FSNETREADBACK_JOINREQUEST,0);
		// Read Back <<


		if(user[clientId].joinSequence!=0)  // Probably re-send packet from the client.
		{
			return YSOK;
		}

		if(FsVerboseMode==YSTRUE)
		{
			printf("Received Join Signal from [%d]%s\n",clientId,user[clientId].username.Txt());
		}

		if(joinLocked==YSTRUE)
		{
			SendReject(clientId);
			SendTextMessage(clientId,"** The server is currently not accepting join requests. **");
			return YSERR;
		}


		iff=(FSIFF)FsGetShort(dat+4);
		strncpy(airId,(char *)(dat+8),31);
		airId[31]=0;
		strncpy(stp,(char *)(dat+40),31);
		stp[31]=0;


		FsStartPosInfo info;
		if(sim->world->GetStartPositionInfo(info,stp)==YSOK)
		{
			if(info.iff!=FsStartPosInfo::IFF_ANY && info.iff!=iff)
			{
				char str[1024];
				SendReject(clientId);

				sprintf(str,"** You need to select IFF %d to start from %s **",info.iff+1,stp);
				SendTextMessage(clientId,str);
				return YSERR;
			}

			if(info.onCarrier==YSTRUE)
			{
				FsGround *gnd;
				gnd=sim->FindGroundByTag(info.carrierTag);
				if(gnd==NULL || gnd->IsAlive()!=YSTRUE)
				{
					SendReject(clientId);
					SendTextMessage(clientId,"** The carrier you chose has been destroyed **");
					return YSERR;
				}
			}

			YsVec3 pos;
			YsAtt3 att;
			auto collPtr=sim->world->GetAirplaneCollision(airId);
			if(collPtr!=NULL)
			{
				info.InterpretPosition(pos,att);
				if(sim->SimTestCollision(*collPtr,pos,att)==YSTRUE)
				{
					SendReject(clientId);
					SendTextMessage(clientId,"** An airplane is already taking the start position. **");
					return YSERR;
				}
			}
		}



		FsAirplane *air;
		int idOnSvr;
		air=sim->world->AddAirplane(airId,YSFALSE);

		if(air==NULL)
		{
			char str[512];
			sprintf(str,"User %s requests unknown airplane %s.",user[clientId].username.Txt(),airId);
			AddMessage(str);
			AddMessage("The request has been rejected.");
			SendReject(clientId);
			return YSERR;

			// AddMessage("Unknown Airplane is specified.");
			// char idName[256];
			// if(sim->world->GetAirplaneTemplateName(idName,0)==YSOK)
			// {
			// 	char str[256];
			// 	sprintf(str,"Assuming it is %s.",idName);
			// 	AddMessage(str);
			// 	air=sim->world->AddAirplane(idName,YSFALSE);
			// }
		}

		if(air!=NULL)
		{
			if(packetLength>0x48)  // if packetLength==0x48, it is from an older version program.
			{                      // This block must come before SettleAirplane to reflect fuel setting in the .STP file.
				int ver,fuel;
				YSBOOL smoke;
				ver=FsGetShort(dat+0x48);
				if(ver==1)
				{
					fuel=FsGetShort(dat+0x4a);

					smoke=(FsGetShort(dat+0x4c)==1 ? YSTRUE : YSFALSE);
					if(smoke==YSTRUE)
					{
						air->SendCommand("SMOKEOIL 100.0kg");
					}
					else
					{
						air->SendCommand("SMOKEOIL 0.0kg");
					}


					char cmd[256];
					sprintf(cmd,"INITFUEL %d%%",fuel);
					air->SendCommand(cmd);
				}
			}


			air->iff=iff;
			sim->world->SettleAirplane(*air,stp);
			int i;
			for(i=0; stp[i]!=0; i++)
			{
				if(strncmp(&stp[i],"CARRIER",7)==0)
				{
					air->SendCommand("INITSPED 0kt");
					break;
				}
			}


			air->netType=FSNET_REMOTE;
			air->name.Set(user[clientId].username);

			if(netcfg!=NULL && netcfg->useMissile!=YSTRUE)
			{
				air->Prop().UnloadGuidedAAM();
				air->Prop().UnloadGuidedAGM();
			}
			if(netcfg!=NULL && netcfg->useUnguidedWeapon!=YSTRUE)
			{
				air->Prop().UnloadUnguidedWeapon();
			}


			if(air->name[0]!=0 && netcfg->sendJoinLeaveMessage==YSTRUE)
			{
				YsString msg;
				MakeTakeOffMessage(msg,air->name,air->Prop().GetIdentifier());
				AddMessage(msg);
				sim->AddTimedMessage(msg);  // 2006/07/18
				BroadcastChatTextMessage(msg);

			#ifdef CRASHINVESTIGATION
				printf("%\n",msg);
				FsLogOutput(FSSVRLOGFILE,msg);
			#endif
			}



			user[clientId].air=air;

			airDatPacketLength=air->Prop().NetworkEncode(airDat,FsExistence::GetSearchKey(air),sim->currentTime,YSFALSE);
			if(FsVerboseMode==YSTRUE)
			{
				printf("AIRDATPACKETLENGTH=%d\n",airDatPacketLength);
			}

			idOnSvr=FsExistence::GetSearchKey(air);

			for(i=0; i<FS_MAX_NUM_USER; i++)
			{
				if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
				{
					if(i!=clientId)
					{
						SendAddAirplane(i,idOnSvr,FSNET_REMOTE,YSFALSE);
						SendPacket(i,airDatPacketLength,airDat);
					}
					else
					{
#ifndef SIMULATEDPACKETDELAY
						SendAddAirplane(i,idOnSvr,FSNET_LOCAL,YSTRUE);
						SendPacket(i,airDatPacketLength,airDat);
#else
						printf("Simulated Packet Delay ####\n");
						user[i].airToSend.Append(air->GetSearchKey());
#endif
					}
				}
			}

			user[clientId].joinSequence=1;
			if(FsVerboseMode==YSTRUE)
			{
				printf("Client #%d Join Sequence %d\n",clientId,user[clientId].joinSequence);
			}

			return YSOK;
		}
		else
		{
			SendReject(clientId);
			SendError(clientId,FSNETERR_REJECT);
			return YSERR;
		}
	}
	else if(user[clientId].state!=FSUSERSTATE_NOTCONNECTED)
	{
		SendReject(clientId);
		SendTextMessage(clientId,"** Not Ready to Join (Wait until log-on process completes) **");
	}
	else
	{
		if(FsVerboseMode==YSTRUE)
		{
			printf("Received Join Signal from unconnected user ??? [%d]%s\n",clientId,user[clientId].username.Txt());
		}
	}
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveAirplaneState(int clientId,unsigned char dat[],unsigned)
{
	if(0<=clientId && clientId<FS_MAX_NUM_USER && user[clientId].state!=FSUSERSTATE_PENDING)
	{
		FsAirplane *air;
		int idx;
		idx=FsGetInt(dat+8);

		air=sim->FindAirplane(idx);

		if(air!=NULL && air->netAlive==YSTRUE)  // <- air->netAlive==YSTRUE : 2002/11/13 anti-ghost
		{
			FsNetReceivedAirplaneState state;
			state.Decode(dat,sim->currentTime);

			// << Airplane State must be reflected immediately 2002/11/12
			// ?
			// >> Airplane State must be reflected immediately 2002/11/12

			if(air->netPrevState.tRemote<state.tRemote && air->netPrevState.tLocal<state.tLocal)
			{
				if(air->netNextState.tRemote<0.0)
				{
					air->netNextState=state;
				}
				else
				{
					double dtLocal,dtRemote,ratio1,ratio2;
					dtLocal=air->netNextState.tLocal-air->netPrevState.tLocal;
					dtRemote=air->netNextState.tRemote-air->netPrevState.tRemote;
					ratio1=dtRemote/dtLocal;

					dtLocal=state.tLocal-air->netPrevState.tLocal;
					dtRemote=state.tRemote-air->netPrevState.tRemote;
					ratio2=dtRemote/dtLocal;

					if(YsAbs(1.0-ratio2)<YsAbs(1.0-ratio1))
					{
						air->netNextState=state;
					}
				}
			}

			// air->Prop().NetworkDecode(air->netPrevState,state);
		}

		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveUnjoin(int,unsigned char dat[])
{
	int idOnSvr;
	short flags;
	idOnSvr=FsGetInt(dat+4);
	flags=FsGetShort(dat+8);


	FsAirplane *air;
	air=sim->FindAirplane(idOnSvr);

	if(air!=NULL)   // 2003/12/19 Crash Problem?
	{
		if((flags&1)!=0)
		{
			YsVec3 pos;
			pos=air->GetPosition();
			// Add explosion here.
		}

		int i;
		for(i=0; i<FS_MAX_NUM_USER; i++)   // 2003/12/19 Crash Problem?
		{
			if(air==user[i].air)
			{
				user[i].air=NULL;
			}
		}

		if(netcfg->sendJoinLeaveMessage==YSTRUE)
		{
			YsString msg;
			MakeUnjoinMessage(msg,air->name);
			AddMessage(msg);
			sim->AddTimedMessage(msg);
			BroadcastChatTextMessage(msg);
		}
	}

	if(FsVerboseMode==YSTRUE)
	{
		printf("Server:Received FSNETCMD_UNJOIN\n");
	}

	return BroadcastRemoveAirplane(idOnSvr,((flags&1)!=0 ? YSTRUE : YSFALSE));
}

YSRESULT FsSocketServer::ReceiveRequestTestAirplane(int clientId,unsigned char [])
{
	// Test Airplane : Su-27 (0,0,3000)

	FsAirplane *air;

	air=sim->world->AddAirplane("F-15C_EAGLE",YSFALSE);
	if(air!=NULL)
	{
		air->iff=FS_IFF3;
		sim->world->SettleAirplane(*air,"NORTH10000_01");
		air->netType=FSNET_LOCAL;
		air->name.Set("TEST AIRPLANE");


		if(netcfg!=NULL && netcfg->useMissile!=YSTRUE)
		{
			air->Prop().UnloadGuidedAAM();
			air->Prop().UnloadGuidedAGM();
		}


		BroadcastAddAirplane(air,FSNET_REMOTE);  // Local in the server==Remove in clients

		//FsGotoPosition *gp;
		//gp=new FsGotoPosition;
		//gp->destination=air->GetPosition();
		//air->SetAutopilot(gp);

		FsDogfight *df;
		df=FsDogfight::Create();
		df->gLimit=7.0;
		df->minAlt=100.0;
		air->SetAutopilot(df);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsSocketServer::ReceiveKillServer(int clientId,unsigned char [])
{
	receivedKillServer=YSTRUE;
	return YSOK;
}

YSRESULT FsSocketServer::ReceiveResendAirRequest(int clientId,unsigned char dat[],unsigned )
{
	int idOnSvr;
	idOnSvr=FsGetInt(dat+4);
	if(0<=clientId && clientId<FS_MAX_NUM_USER)
	{
		FsAirplane *air;
		air=sim->FindAirplane(idOnSvr);
		if(air!=NULL && air->Prop().IsActive()==YSTRUE)
		{
			if(air==user[clientId].air)
			{
				SendAddAirplane(clientId,idOnSvr,FSNET_LOCAL,YSTRUE);
			}
			else
			{
				SendAddAirplane(clientId,idOnSvr,FSNET_REMOTE,YSFALSE);
			}
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveResendGndRequest(int clientId,unsigned char dat[],unsigned )
{
	int idOnSvr;
	idOnSvr=FsGetInt(dat+4);
	if(0<=clientId && clientId<FS_MAX_NUM_USER)
	{
		FsGround *gnd;
		gnd=sim->FindGround(idOnSvr);
		if(gnd!=NULL && gnd->IsAlive()==YSTRUE)
		{
			SendAddGround(clientId,idOnSvr,FSNET_REMOTE);
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketServer::ReceiveWeaponConfig(int ,unsigned char cmdTop[],unsigned )
{
	YsArray <int,64> weaponConfig;
	int airId;
	FsAirplane *air;

	FsDecodeWeaponConfig(airId,weaponConfig,cmdTop);

	air=sim->FindAirplane(airId);
	if(air!=NULL)
	{
		air->AutoSendCommand(weaponConfig.GetN(),weaponConfig);
		air->Prop().GetWeaponConfig(weaponConfig);
	}

	if(netcfg->useMissile!=YSTRUE)
	{
		air->Prop().UnloadGuidedAAM();
		air->Prop().UnloadGuidedAGM();
		air->Prop().GetWeaponConfig(weaponConfig);
	}
	if(netcfg->useUnguidedWeapon!=YSTRUE)
	{
		air->Prop().UnloadUnguidedWeapon();
		air->Prop().GetWeaponConfig(weaponConfig);
	}

	if(netcfg->recordWhenServerMode==YSTRUE)
	{
		FsSimulationEvent evt;
		evt.Initialize();
		evt.eventType=FSEVENT_SETWEAPONCONFIG;
		evt.objKey=airId;
		evt.weaponCfg.Set(weaponConfig.GetN(),weaponConfig);
		sim->AddEvent(evt);
	}

	return BroadcastWeaponConfig(airId,weaponConfig.GetN(),weaponConfig);
}

YSRESULT FsSocketServer::ReceiveListUser(int clientId)
{
	int i;
	unsigned char cmd[256];
	unsigned flags,airKey;
	YSSIZE_T packetLength;
	flags=0;
	airKey=0;

	flags|=2;  // Means server
	if(ServerPlayerPlane()!=NULL && ServerPlayerPlane()->IsAlive()==YSTRUE)
	{
		flags|=1;
		flags|=((unsigned)ServerPlayerPlane()->iff)*65536;
		airKey=FsExistence::GetSearchKey(ServerPlayerPlane());
	}
	FsSetInt(cmd   ,FSNETCMD_LISTUSER);
	FsSetInt(cmd+ 4,flags);
	FsSetInt(cmd+ 8,airKey);
	FsSetInt(cmd+12,0);
	strncpy((char *)cmd+16,username.Txt(),239);
	cmd[255]=0;
	packetLength=16+strlen((char *)cmd+16)+1;
	SendPacket(clientId,packetLength,cmd);

	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			flags=0;
			airKey=0;
			if(user[i].air!=NULL && user[i].air->IsAlive()==YSTRUE)
			{
				flags|=1;
				flags|=((unsigned)user[i].air->iff)*65536;
				airKey=FsExistence::GetSearchKey(user[i].air);
			}

			FsSetInt(cmd   ,FSNETCMD_LISTUSER);
			FsSetInt(cmd+ 4,flags);
			FsSetInt(cmd+ 8,airKey);
			FsSetInt(cmd+12,0);
			strncpy((char *)cmd+16,user[i].username,239);
			cmd[255]=0;
			packetLength=16+strlen((char *)cmd+16)+1;
			SendPacket(clientId,packetLength,cmd);
		}
	}

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveQueryAirState(int clientId,unsigned char cmdTop[],unsigned )
{
	printf("ReceiveQueryAirState\n"); 
	if(clientId<FS_MAX_NUM_USER && user[clientId].state!=FSUSERSTATE_NOTCONNECTED)
	{
		const unsigned char *ptr=cmdTop;
		int i,n,airId;
		FsAirplane *air;

		FsPopInt(ptr);  // Skip FSCMD_*
		n=FsPopInt(ptr);  // # of airplanes;
		for(i=0; i<n; i++)
		{
			airId=FsPopInt(ptr);
			air=sim->FindAirplane(airId);
			if(air!=NULL)
			{
				printf("Sending: %d %s\n",airId,air->name.GetArray());

				unsigned packetLength;
				unsigned char packet[256];

				packetLength=air->Prop().NetworkEncode(packet,airId,sim->currentTime,YSFALSE);
				SendPacket(clientId,packetLength,packet);

// Never use Short Format fore replying to QueryAirState.
// In reply to QueryAirState, the server may resend local airplane's state to the client,
// and the client's airplane's ammo may be rounded off to 255.
//				unsigned packetLengthShort;
//				unsigned char packetShort[256];
//
//				packetLength=air->Prop().NetworkEncode(packet,airId,sim->currentTime,YSFALSE);
//				packetLengthShort=air->Prop().NetworkEncode(packetShort,airId,sim->currentTime,YSTRUE);
//				if(user[clientId].version>=20060805 && packetLengthShort>0)
//				{
//					SendPacket(clientId,packetLengthShort,packetShort);
//				}
//				else
//				{
//					SendPacket(clientId,packetLength,packet);
//				}
			}
		}
	}

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveAirTurretState(int clientId,unsigned char cmdTop[],unsigned packetLength)
{
	if(clientId<FS_MAX_NUM_USER && user[clientId].state!=FSUSERSTATE_NOTCONNECTED)
	{
		const unsigned char *ptr=cmdTop;;
		int airId;
		FsAirplane *air;

		FsPopInt(ptr);  // Skip FSCMD_*
		airId=FsPopInt(ptr);
		air=sim->FindAirplane(airId);
		if(air!=NULL)
		{
			if(air->Prop().DecodeTurretState(cmdTop,packetLength)!=YSOK)
			{
				// Illegally modified airplane?
				SendAirCmd(clientId,airId,"NMTURRET 0");
			}
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::ReceiveGndTurretState(int clientId,unsigned char cmdTop[],unsigned packetLength)
{
	if(clientId<FS_MAX_NUM_USER && user[clientId].state!=FSUSERSTATE_NOTCONNECTED)
	{
		const unsigned char *ptr=cmdTop;
		int gndId;
		FsGround *gnd;

		FsPopInt(ptr);  // Skip FSCMD_*
		gndId=FsPopInt(ptr);
		gnd=sim->FindGround(gndId);
		if(gnd!=NULL)
		{
			if(gnd->Prop().DecodeTurretState(cmdTop,packetLength)!=YSOK)
			{
				// Illegally modified airplane?
				SendGndCmd(clientId,gndId,"NMTURRET 0");
			}
		}
	}
	return YSOK;
}

// FSNETCMD_AIRCMD is supported after 20010624
YSRESULT FsSocketServer::ReceiveAirCmd(int clientId,unsigned char dat[],unsigned packetLength)
{
	char cmd[256];
	int idOnSvr;

	idOnSvr=FsGetInt(dat+4);
	strcpy(cmd,(char *)(dat+8));
	if(0<=clientId && clientId<FS_MAX_NUM_USER)
	{
		FsAirplane *air;
		air=sim->FindAirplane(idOnSvr);
		if(air!=NULL)
		{
			air->SendCommand(cmd);

			int i;
			for(i=0; i<FS_MAX_NUM_USER; i++)
			{
				if(user[i].state!=FSUSERSTATE_NOTCONNECTED && i!=clientId)
				{
					SendPacket(i,packetLength,dat);
				}
			}
		}
	}

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveGndCmd(int clientId,unsigned char dat[],unsigned packetLength)
{
	char cmd[256];
	int idOnSvr;

	idOnSvr=FsGetInt(dat+4);
	strcpy(cmd,(char *)(dat+8));
	if(0<=clientId && clientId<FS_MAX_NUM_USER)
	{
		FsGround *gnd;
		gnd=sim->FindGround(idOnSvr);
		if(gnd!=NULL)
		{
			gnd->SendCommand(cmd);

			int i;
			for(i=0; i<FS_MAX_NUM_USER; i++)
			{
				if(user[i].state!=FSUSERSTATE_NOTCONNECTED && i!=clientId)
				{
					SendPacket(i,packetLength,dat);
				}
			}
		}
	}

	return YSOK;
}

YSRESULT FsSocketServer::ReceiveTextMessage(int clientId,unsigned char dat[],unsigned )
{
	if(netcfg->serverDisableChat==YSTRUE)  // 2005/03/25
	{
		SendTextMessage(clientId,"** This Server Disables Chat. **");
		SendTextMessage(clientId,"** Cannot Send the Message. **");
		return YSOK;
	}
	else
	{
		if(chatLogFp.Fp()!=NULL)
		{
			fprintf(chatLogFp.Fp(),"%s\n",(char *)dat+12);
		}


		AddMessage((char *)dat+12);
		sim->AddTimedMessage((char *)dat+12);
		BroadcastTextMessage((char *)dat+12);
		return YSOK;
	}
}

YSRESULT FsSocketServer::ReceiveEnvironmentRequest(int clientId,unsigned char [],unsigned )
{
	return SendEnvironment(clientId);
}

YSRESULT FsSocketServer::ReceivedResendJoinApproval(int clientId,unsigned char [],unsigned )
{
	if(user[clientId].air!=NULL && user[clientId].air->IsAlive()==YSTRUE)
	{
		FsAirplane *air;
		unsigned char airDat[256];
		unsigned airDatPacketLength;

		air=user[clientId].air;
		airDatPacketLength=air->Prop().NetworkEncode(airDat,FsExistence::GetSearchKey(air),sim->currentTime,YSFALSE);

		SendAddAirplane(clientId,FsExistence::GetSearchKey(air),FSNET_LOCAL,YSTRUE);
		SendPacket(clientId,airDatPacketLength,airDat);
		SendJoinApproval(clientId);
	}
	else
	{
		SendReject(clientId);
	}
	return YSOK;
}

YSRESULT FsSocketServer::SendError(int clientId,int errorCode)
{
	unsigned char dat[8];
	FsSetInt(dat+0,FSNETCMD_ERROR);
	FsSetInt(dat+4,errorCode);
	return SendPacket(clientId,8,dat);
}

YSRESULT FsSocketServer::SendTextMessage(int clientId,const char txt[])
{
	YSSIZE_T packetLength;
	unsigned char dat[256];
	FsSetInt(dat  ,FSNETCMD_TEXTMESSAGE);
	FsSetInt(dat+4,0);  // Will be used for destination selection
	FsSetInt(dat+8,0);  // Will be used for source indication
	strncpy((char *)dat+12,txt,244);
	dat[255]=0;

	packetLength=YsSmaller <YSSIZE_T> ((12+strlen(txt)+1 +3)&~3,256);


	int i,j;
	if(user[clientId].version<20050701)
	{
		j=12;
		for(i=0; txt[i]!=0 && j<254; i++)
		{
			dat[j++]=txt[i];
			if(txt[i]=='%')
			{
				dat[j++]='%';
			}
		}
		dat[j]=0;
	}


	return SendPacket(clientId,packetLength,dat);
}

YSRESULT FsSocketServer::SendReject(int clientId)
{
	unsigned char dat[8];
	FsSetInt(dat,FSNETCMD_REJECTJOINREQ);
	return SendPacket(clientId,4,dat);
}

YSRESULT FsSocketServer::SendLoadField(
    int clientId,const char fieldName[],YSBOOL loadYFS,const YsVec3 &pos,const YsAtt3 &att)
{
	unsigned char dat[64];
	unsigned flag;

	flag=0;
	if(loadYFS==YSTRUE)
	{
		flag|=1;
	}

	FsSetInt(dat+ 0,FSNETCMD_LOADFIELD);
	strcpy((char *)dat+4,fieldName);
	FsSetInt(dat+36,flag);
	FsSetFloat(dat+40,(float)pos.x());
	FsSetFloat(dat+44,(float)pos.y());
	FsSetFloat(dat+48,(float)pos.z());
	FsSetFloat(dat+52,(float)att.h());
	FsSetFloat(dat+56,(float)att.p());
	FsSetFloat(dat+60,(float)att.b());

	return SendPacket(clientId,64,dat);
}

YSRESULT FsSocketServer::SendAddAirplane(int clientId,int airId,FSNETTYPE netType,YSBOOL setAsPlayer)
{
	unsigned char dat[256],*ptr;
	FsAirplane *air;
	unsigned short flags0;
	// static char *FsMustSendCommandList[]=
	// {
	// 	"AFTBURNR","THRAFTBN","THRMILIT","WEIGHCLN","WEIGFUEL","WEIGLOAD","FUELABRN","FUELMILI",
	// 	"CRITAOAP","CRITAOAM","CRITSPED","MAXSPEED","HASSPOIL","RETRGEAR","VARGEOMW","CLVARGEO",
	// 	"CDVARGEO","CLBYFLAP","CDBYFLAP","CDBYGEAR","CDSPOILR","WINGAREA","MXIPTAOA","MXIPTSSA",
	// 	"MXIPTROL","CPITMANE","CPITSTAB","CYAWMANE","CYAWSTAB","CROLLMAN","REFVCRUS","REFACRUS",
	// 	"REFVLAND","REFAOALD","REFLNRWY","REFTHRLD","REFTCRUS","AUTOCALC","MANESPD1","MANESPD2",
	// 	"HTRADIUS","STRENGTH","PROPELLR","MANESPD3","RADARCRS","MACHNGN2","TRSTVCTR",
	// 	"TRSTDIR0","TRSTDIR1","PSTMPTCH","PSTMYAW_","PSTMROLL","PROPEFCY","PROPVMIN","AIRCLASS",
	// 	"THRSTREV",
	// 	NULL
	// };
	// See bottom of the function for newer versions.


	if(YsIsIncluded <unsigned int> (user[clientId].airToSend.GetN(),user[clientId].airToSend,airId)!=YSTRUE)
	{
		user[clientId].airToSend.Append(airId);
	}


	air=sim->FindAirplane(airId);

	flags0=0;
	if(setAsPlayer==YSTRUE)
	{
		flags0|=1;
	}
	if(air->Prop().IsJet()==YSTRUE)
	{
		flags0|=2;
	}

	double yOffset;  // Cheating:  If it is on carrier, add some GETA.
	yOffset=(air->Prop().IsOnCarrier()==YSTRUE ? 0.3 : 0.0);


	ptr=dat;
	FsPushInt(  ptr       ,FSNETCMD_ADDOBJECT);
	FsPushShort(ptr,0);  // AIR
	FsPushShort(ptr,(short)netType);

	FsPushInt  (ptr,airId);
	FsPushShort(ptr,(short)air->iff);
	FsPushShort(ptr,0);
	FsPushFloat(ptr,(float)air->GetPosition().x());
	FsPushFloat(ptr,(float)air->GetPosition().y()+(float)yOffset);
	FsPushFloat(ptr,(float)air->GetPosition().z());
	FsPushFloat(ptr,(float)air->GetAttitude().h());
	FsPushFloat(ptr,(float)air->GetAttitude().p());
	FsPushFloat(ptr,(float)air->GetAttitude().b());

	strncpy((char *)ptr,air->Prop().GetIdentifier(),31);
	ptr[31]=0;
	ptr+=32;

	strncpy((char *)ptr,air->Prop().GetSubstIdName(),31);
	ptr[31]=0;  // Safety Zero Termination
	ptr+=32;

	FsPushInt(ptr,air->ysfId);                        // 108 bytes (* 92+16)

	FsPushInt(ptr,air->airFlag); // airFlag/gndFlag   112 bytes
	FsPushInt(ptr,flags0);             // Flags0      116 bytes

	FsPushFloat(ptr,(float)air->Prop().GetOutsideRadius());   // 120 bytes << Up to outside radius is minimum requirement for FSNETCMD_ADDOBJECT


	FsPushShort(ptr,(short)air->Prop().GetAircraftClass());    //                      122 bytes
	FsPushShort(ptr,(short)air->Prop().GetAirplaneCategory()); //                      124 bytes
	FsPushShort(ptr,0);                                 // Nationality Info     126 bytes
	FsPushShort(ptr,0);                                 // For future expansion 128 bytes

	if(air->name[0]!=0)
	{
		strncpy((char *)ptr,air->name,47);
		ptr[47]=0;
		ptr+=48;                                       //                       176 bytes
	}


	if(SendPacket(clientId,ptr-dat,dat)==YSOK)
	{
		YsArray <YsString> propCmd,turretCmd;
		int i;
		air->Prop().EncodeProperty(propCmd,turretCmd,user[clientId].version);

		FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
		// printf("PropCmd\n");
		for(i=0; i<propCmd.GetN(); i++)
		{
			SendAirCmd(clientId,airId,propCmd[i]);
			// printf("%s\n",propCmd[i].GetArray());
		}

		FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
		// printf("TurretCmd\n");
		for(i=0; i<turretCmd.GetN(); i++)
		{
			SendAirCmd(clientId,airId,turretCmd[i]);
			// printf("%s\n",turretCmd[i].GetArray());
		}


		if(netcfg->disableRadarGunSight==YSTRUE || air->Prop().GetLeadGunSight()!=YSTRUE)
		{
			SendAirCmd(clientId,airId,"GUNSIGHT FALSE");
		}
		else
		{
			SendAirCmd(clientId,airId,"GUNSIGHT TRUE");
		}


		if(netcfg->useUnguidedWeapon!=YSTRUE)
		{
			SendAirCmd(clientId,airId,"ULOADGUN");
			SendAirCmd(clientId,airId,"ULOADBOM");
			SendAirCmd(clientId,airId,"ULOADRKT");
		}

		if(netType==FSNET_REMOTE)
		{
			for(int smkIdx=0; smkIdx<air->Prop().GetNumSmokeGenerator(); ++smkIdx)
			{
				SendSmokeColor(clientId,FsExistence::GetSearchKey(air),smkIdx,air->Prop().GetSmokeColor(smkIdx));
			}
		}

		// Engine properties must be sent in one packet. >>
		{
			FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
			auto engineCmd=air->Prop().EncodeEngineProperty(user[clientId].version);
			for(auto &str : engineCmd)
			{
				SendAirCmd(clientId,airId,str);
				// printf("%s\n",propCmd[i].GetArray());
			}
		}
		// Engine properties must be sent in one packet. <<

		return YSOK;
	}

	return YSERR;
}

YSRESULT FsSocketServer::SendAddGround(int clientId,int gndId,FSNETTYPE netType)
{
	unsigned char dat[256],*ptr;
	FsGround *gnd;
	gnd=sim->FindGround(gndId);

	unsigned int flags0;
	flags0=0;
	if(gnd->primaryTarget==1)
	{
		flags0|=0x8000;
	}

	if(YsIsIncluded <unsigned int> (user[clientId].gndToSend.GetN(),user[clientId].gndToSend,gndId)!=YSTRUE)
	{
		user[clientId].gndToSend.Append(gndId);
	}

	if(FsVerboseMode==YSTRUE)
	{
		printf("Server:SendAddGround (%d) %s\n",gndId,gnd->Prop().GetIdentifier());
	}

	ptr=dat;
	FsPushInt(  ptr       ,FSNETCMD_ADDOBJECT);
	FsPushShort(ptr,1);  // GROUND
	FsPushShort(ptr,(short)netType);
	FsPushInt(ptr,gndId);
	FsPushShort(ptr,(short)gnd->iff);
	FsPushShort(ptr,0);
	FsPushFloat(ptr,(float)gnd->GetPosition().x());
	FsPushFloat(ptr,(float)gnd->GetPosition().y());
	FsPushFloat(ptr,(float)gnd->GetPosition().z());
	FsPushFloat(ptr,(float)gnd->GetAttitude().h());
	FsPushFloat(ptr,(float)gnd->GetAttitude().p());
	FsPushFloat(ptr,(float)gnd->GetAttitude().b());

	strncpy((char *)ptr,gnd->Prop().GetIdentifier(),31);
	ptr[31]=0;
	ptr+=32;

	strncpy((char *)ptr,gnd->Prop().GetSubstIdName(),31);
	ptr[31]=0;
	ptr+=32;

	FsPushInt(ptr,gnd->ysfId);                         // 108 bytes

	FsPushInt(ptr,gnd->gndFlag);   // airFlag/gndFlag  112 bytes
	FsPushInt(ptr,flags0);         //                  116 bytes

	FsPushFloat(ptr,(float)gnd->Prop().GetOutsideRadius());    // 120 bytes << Up to outside radius is minimum requirement for FSNETCMD_ADDOBJECT

	if(gnd->name[0]!=0)
	{
		FsPushShort(ptr,0);             // Class                122 bytes
		FsPushShort(ptr,0);             // Category             124 bytes
		FsPushShort(ptr,0);             // Nationality Info     126 bytes
		FsPushShort(ptr,0);             // For future expansion 128 bytes

		strncpy((char *)ptr,gnd->name,47);
		ptr[47]=0;
		ptr+=48;                       //                       176 bytes
	}



	if(SendPacket(clientId,ptr-dat,dat)==YSOK)
	{
		return YSOK;
	}
	else
	{
		AddMessage("TIMEOUT while sending object");
		return YSERR;
	}
}

YSRESULT FsSocketServer::SendJoinApproval(int clientId)
{
	unsigned char dat[4];
	FsSetInt(dat,FSNETCMD_JOINAPPROVAL);
	return SendPacket(clientId,4,dat);
}

YSRESULT FsSocketServer::SendRemoveAirplane(int clientId,int airId,YSBOOL explosion)
{
	if(YsIsIncluded <unsigned int> (
	    user[clientId].airRmvToSend.GetN(),user[clientId].airRmvToSend,airId)!=YSTRUE)
	{
		user[clientId].airRmvToSend.Append(airId);
	}

	unsigned char dat[256];
	FsSetInt  (dat   ,FSNETCMD_REMOVEAIRPLANE);
	FsSetInt  (dat+ 4,airId);
	FsSetShort(dat+ 8,(explosion==YSTRUE ? 1 : 0));
	FsSetShort(dat+10,0);
	return SendPacket(clientId,12,dat);
}

YSRESULT FsSocketServer::SendRemoveGround(int clientId,int gndId,YSBOOL explosion)
{
	if(YsIsIncluded <unsigned int> (
	    user[clientId].gndRmvToSend.GetN(),user[clientId].gndRmvToSend,gndId)!=YSTRUE)
	{
		user[clientId].gndRmvToSend.Append(gndId);
	}

	unsigned char dat[256];
	FsSetInt  (dat   ,FSNETCMD_REMOVEGROUND);
	FsSetInt  (dat +4,gndId);
	FsSetShort(dat +8,(explosion==YSTRUE ? 1 : 0));
	FsSetShort(dat+10,0);
	return SendPacket(clientId,12,dat);
}

YSRESULT FsSocketServer::SendPrepare(int clientId)
{
	unsigned char dat[256];
	FsSetInt(dat,FSNETCMD_PREPARESIMULATION);
	return SendPacket(clientId,4,dat);
}

YSRESULT FsSocketServer::SendTestPacket(int clientId)
{
	unsigned char dat[4];
	FsSetInt(dat,FSNETCMD_TESTPACKET);
	SendPacket(clientId,4,dat);
	return YSOK;
}

YSRESULT FsSocketServer::SendTestHugePacket(int )
{
	unsigned char dat[8192];
	FsSetInt(dat,8192);
	Send(0,8192,dat,0);
	AddMessage("Test Packet (Huge) is sent.");
	AddMessage("The client is supposed to disconnect.");
	return YSOK;
}

YSRESULT FsSocketServer::SendLockOn(int clientId,int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir)
{
	unsigned char dat[256],*ptr;
	ptr=dat;
	FsPushInt(ptr,FSNETCMD_LOCKON);
	FsPushInt(ptr,id1);
	FsPushInt(ptr,id1isAir);
	FsPushInt(ptr,id2);
	FsPushInt(ptr,id2isAir);
	return SendPacket(clientId,ptr-dat,dat);
}

YSRESULT FsSocketServer::SendSetTestAutopilot(int clientId,int idOnSvr,int type)
{
	unsigned char dat[256];
	FsSetInt  (dat   ,FSNETCMD_SETTESTAUTOPILOT);
	FsSetInt  (dat +4,idOnSvr);
	FsSetShort(dat +8,(short)type);
	FsSetShort(dat+10,0);
	return SendPacket(clientId,12,dat);
}

YSRESULT FsSocketServer::SendAssignSideWindow(int clientId,int idOnSvr,const double &hdgOffset,const double &pchOffset)
{
	unsigned char dat[256],*ptr;
	ptr=dat;
	FsPushInt(ptr,FSNETCMD_ASSIGNSIDEWINDOW);
	FsPushInt(ptr,idOnSvr);
	FsPushFloat(ptr,float(hdgOffset));
	FsPushFloat(ptr,float(pchOffset));
	return SendPacket(clientId,ptr-dat,dat);
}

YSRESULT FsSocketServer::SendVersionNotify(int clientId)
{
	if(user[clientId].version>=20010120)
	{
		unsigned char dat[256],*ptr;
		ptr=dat;
		FsPushInt(ptr,FSNETCMD_VERSIONNOTIFY);
		FsPushInt(ptr,YSFLIGHT_NETVERSION);
		return SendPacket(clientId,ptr-dat,dat);
	}
	return YSOK;
}

// BroadcastAirCmd is available after 20010624
YSRESULT FsSocketServer::SendAirCmd(int clientId,int airId,const char cmd[])
{
	YSSIZE_T packetLength;
	unsigned char dat[256],*ptr;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_AIRCMD);
	FsPushInt(ptr,airId);
	strcpy((char *)(dat+8),cmd);

	packetLength=8+strlen(cmd)+1;
	return SendPacket(clientId,packetLength,dat);
}

YSRESULT FsSocketServer::SendGndCmd(int clientId,int gndId,const char cmd[])
{
	YSSIZE_T packetLength;
	unsigned char dat[256],*ptr;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_GNDCMD);
	FsPushInt(ptr,gndId);
	strcpy((char *)(dat+8),cmd);

	packetLength=8+strlen(cmd)+1;
	return SendPacket(clientId,packetLength,dat);
}

YSRESULT FsSocketServer::SendUseMissile(int clientId,YSBOOL useMissile)
{
	unsigned char dat[256],*ptr;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_USEMISSILE);
	FsPushInt(ptr,(useMissile==YSTRUE ? 1 : 0));
	return SendPacket(clientId,ptr-dat,dat);
}
YSRESULT FsSocketServer::SendUseUnguidedWeapon(int clientId,YSBOOL useUnguidedWeapon)
{
	if(user[clientId].version>=20050323)
	{
		unsigned char dat[256],*ptr;

		ptr=dat;
		FsPushInt(ptr,FSNETCMD_USEUNGUIDEDWEAPON);
		FsPushInt(ptr,(useUnguidedWeapon==YSTRUE ? 1 : 0));
		return SendPacket(clientId,ptr-dat,dat);
	}
	else if(user[clientId].version>=20040618)
	{
		if(useUnguidedWeapon!=YSTRUE)
		{
			SendTextMessage(clientId,"Unguided weapons are not allowed on this server.");
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::SendEnvironment(int clientId)
{
	unsigned char dat[256],*ptr;
	YsVec3 wind;
	YSBOOL fog;
	double visibility;
	unsigned flags;

	wind=sim->GetWeather().GetWind();
	fog=sim->GetWeather().GetFog();
	visibility=sim->GetWeather().GetFogVisibility();

	flags=0;
	if(fog==YSTRUE)
	{
		flags|=1;
	}
	// Leave bit1 unused
	if(netcfg->serverControlBlackOut!=0)
	{
		flags|=8;

		if(netcfg->serverControlBlackOut==1)
		{
			sim->SetBlackOut(YSTRUE);
		}
		else if(netcfg->serverControlBlackOut==2)
		{
			sim->SetBlackOut(YSFALSE);
		}

		if(sim->GetConfig().blackOut==YSTRUE)
		{
			flags|=4;
		}
	}

	if(netcfg->serverControlMidAirCollision!=0)
	{
		flags|=32;

		if(netcfg->serverControlMidAirCollision==1)
		{
			sim->SetMidAirCollision(YSTRUE);
		}
		else if(netcfg->serverControlMidAirCollision==2)
		{
			sim->SetMidAirCollision(YSFALSE);
		}

		if(sim->GetConfig().midAirCollision==YSTRUE)
		{
			flags|=16;
		}
	}

	if(netcfg->serverControlCanLandAnywhere!=0)
	{
		flags|=128;

		if(netcfg->serverControlCanLandAnywhere==1)
		{
			sim->SetCanLandAnywhere(YSTRUE);
		}
		else if(netcfg->serverControlCanLandAnywhere==2)
		{
			sim->SetCanLandAnywhere(YSFALSE);
		}

		if(sim->GetConfig().canLandAnywhere==YSTRUE)
		{
			flags|=64;
		}
	}

	ptr=dat;
	FsPushInt  (ptr,FSNETCMD_ENVIRONMENT);
	FsPushShort(ptr,1);                              // Version 1:Env, Flags, Wind, Visibility
	FsPushShort(ptr,(short)sim->GetEnvironment());
	FsPushInt  (ptr,flags);
	FsPushFloat(ptr,(float)wind.x());
	FsPushFloat(ptr,(float)wind.y());
	FsPushFloat(ptr,(float)wind.z());
	FsPushFloat(ptr,(float)visibility);
	return SendPacket(clientId,ptr-dat,dat);
}

YSRESULT FsSocketServer::SendLogOnComplete(int clientId)
{
	unsigned char dat[256],*ptr;
	ptr=dat;
	FsPushInt(ptr,FSNETCMD_LOGON);

	SendPacket(clientId,ptr-dat,dat);
	FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
	SendPacket(clientId,ptr-dat,dat);
	FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);
	SendPacket(clientId,ptr-dat,dat);
	return YSOK;
}

YSRESULT FsSocketServer::SendWeaponConfig(int clientId,int airId,YSSIZE_T nWeaponConfig,const int weaponConfig[])
{
	if(user[clientId].state!=FSUSERSTATE_NOTCONNECTED && user[clientId].version>=20040618)
	{

		unsigned char dat[256];
		YSSIZE_T packetLength=FsEncodeWeaponConfig(dat,airId,nWeaponConfig,weaponConfig);
		return SendPacket(clientId,packetLength,dat);
	}
	return YSERR;
}

YSRESULT FsSocketServer::SendControlShowUserName(int clientId,int showUserName)
{
	if(user[clientId].state!=FSUSERSTATE_NOTCONNECTED && user[clientId].version>=20040701)
	{
		unsigned char dat[256];

		FsSetInt(dat  ,FSNETCMD_CTRLSHOWUSERNAME);
		FsSetInt(dat+4,showUserName);

		return SendPacket(clientId,8,dat);
	}
	return YSERR;
}

YSRESULT FsSocketServer::SendConfigString(int clientId,const char configStr[])
{
	if(user[clientId].state!=FSUSERSTATE_NOTCONNECTED && user[clientId].version>=20060514)
	{

		unsigned char dat[256];

		FsSetInt(dat  ,FSNETCMD_CONFIGSTRING);
		FsSetInt(dat+4,0);  // For future use

		strncpy((char *)dat+8,configStr,247);
		dat[255]=0;

		YSSIZE_T n=8+strlen((char *)dat+8)+1;
		return SendPacket(clientId,n,dat);
	}
	return YSERR;
}

YSRESULT FsSocketServer::SendAirplaneList(int clientId,int numSend)
{
	if(user[clientId].state!=FSUSERSTATE_NOTCONNECTED)
	{
		int id;
		YsArray <unsigned char> dat;

		// 4 FSNETCMD_LIST
		// 1 TYPE (1:Airplane 2:Start Pos)
		// 1 Number of items
		// 2 0 (Reserved)

		dat.Set(8,NULL);
		FsSetInt(dat  ,FSNETCMD_LIST);
		dat[4]=1;
		dat[5]=0;
		dat[6]=0;
		dat[7]=0;

		for(id=0; id<user[clientId].airTypeToSend.GetN() && id<numSend; id++)
		{


			YSSIZE_T curPos=dat.GetN();

			dat[5]++;
			dat.Resize(dat.GetN()+user[clientId].airTypeToSend[id].Strlen()+1);

			strcpy((char *)dat.GetArray()+curPos,user[clientId].airTypeToSend[id]);

			if(dat[5]==255 || dat.GetN()>=1024)
			{
				AddMessage("Sending Airplane List...\n");
				return SendPacket(clientId,dat.GetN(),dat);
			}
		}

		if(dat[5]>0)
		{
			return SendPacket(clientId,dat.GetN(),dat);
		}

		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketServer::SendRemoveAirplaneReadBack(int clientId,int idOnSvr)
{
	return SendReadBack(clientId,FSNETREADBACK_REMOVEAIRPLANE,idOnSvr);
}

YSRESULT FsSocketServer::SendRemoveGroundReadBack(int clientId,int idOnSvr)
{
	return SendReadBack(clientId,FSNETREADBACK_REMOVEGROUND,idOnSvr);
}

YSRESULT FsSocketServer::SendReadBack(int clientId,int readBackType,int readBackParam)
{
	unsigned char dat[256],*ptr;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_READBACK);
	FsPushShort(ptr,(short)readBackType);
	FsPushShort(ptr,0); // Padding
	FsPushInt(ptr,readBackParam);

	return SendPacket(clientId,ptr-dat,dat);
}

YSRESULT FsSocketServer::SendSmokeColor(int clientId,int idOnSvr,int smkIdx,const YsColor &col)
{
	unsigned char dat[256],*ptr;

	ptr=dat;

	FsPushInt(ptr,FSNETCMD_SMOKECOLOR);
	FsPushInt(ptr,idOnSvr);
	FsPushUnsignedChar(ptr,smkIdx);
	FsPushUnsignedChar(ptr,(char)col.Ri());
	FsPushUnsignedChar(ptr,(char)col.Gi());
	FsPushUnsignedChar(ptr,(char)col.Bi());

	return SendPacket(clientId,ptr-dat,dat);
}

YSRESULT FsSocketServer::SendReportScore(int clientId,YSBOOL scored,FsNetworkScoreLog &score)
{
	int i;
	unsigned char dat[256],*ptr;
	ptr=dat;
	FsPushInt(ptr,FSNETCMD_REPORTSCORE);                                //  4
	FsPushShort(ptr,(short)scored);                                       //  6
	FsPushShort(ptr,(short)score.weaponType);                             //  8
	FsPushFloat(ptr,(float)score.pos.x());                              // 12
	FsPushFloat(ptr,(float)score.pos.y());                              // 16
	FsPushFloat(ptr,(float)score.pos.z());                              // 20
	FsPushFloat(ptr,(float)score.scoreTime);                            // 24

	FsPushInt(ptr,0);                                                   // 28 Reserve
	FsPushInt(ptr,score.scoredKey);                                     // 32
	for(i=0; i<32; i++)
	{
		ptr[i]=0;
	}
	if(-1<=score.scoredClientId)
	{
		strncpy((char *)ptr,score.scoredUsername,31);
	}
	else
	{
		strncpy((char *)ptr,"a Non-Player Object",31);
	}
	ptr[31]=0;
	ptr+=32;                                                            // 64

	for(i=0; i<32; i++)
	{
		ptr[i]=0;
	}
	strncpy((char *)ptr,score.scoredVehicleType,31);
	ptr[31]=0;
	ptr+=32;                                                            // 96


	FsPushInt(ptr,0);                                                   // 100
	FsPushInt(ptr,score.killedKey);                                     // 132
	for(i=0; i<32; i++)
	{
		ptr[i]=0;
	}
	if(-1<=score.killedClientId)
	{
		strncpy((char *)ptr,score.killedUsername,31);
	}
	else
	{
		strncpy((char *)ptr,"a Non-Player Object",31);
	}
	ptr[31]=0;
	ptr+=32;                                                            // 164

	for(i=0; i<32; i++)
	{
		ptr[i]=0;
	}
	strncpy((char *)ptr,score.killedVehicleType,31);
	ptr[31]=0;
	ptr+=32;                                                            // 160



	return SendPacket(clientId,ptr-dat,dat);
}

YSRESULT FsSocketServer::SendForceJoin(int clientId)
{
	unsigned char dat[4];
	FsSetInt(dat,FSNETCMD_SERVER_FORCE_JOIN);
	return SendPacket(clientId,4,dat);
}

// YSRESULT FsSocketServer::SendAssignMotionPath(int clientId,FsExistence *obj)
// {
// 	if(user[clientId].loggedOn==YSTRUE && user[clientId].version>=20050323)
// 	{
// 		int len;
// 		int isAir,idOnSvr;
// 
// 		if(obj!=NULL && EncodeObject(isAir,idOnSvr,obj)==YSOK && obj->motionPath!=NULL)
// 		{
// 			len=strlen(obj->motionPath->GetTag());
// 			if(len<128)
// 			{
// 				unsigned int packetLength;
// 				unsigned char *ptr,dat[256];
// 
// 				ptr=dat;
// 				FsPushInt(ptr,FSNETCMD_ASSIGNMOTIONPATH);
// 				FsPushInt(ptr,0);    // Reserved
// 				FsPushShort(ptr,0);  // Reserved
// 				FsPushShort(ptr,isAir);
// 				FsPushInt(ptr,idOnSvr);
// 
// 				FsPushShort(ptr,(int)obj->useMotionPathOffset);
// 				FsPushShort(ptr,(int)obj->motionPathIndex);
// 				FsPushFloat(ptr,(float)obj->motionPathOffset.x());
// 				FsPushFloat(ptr,(float)obj->motionPathOffset.y());
// 				FsPushFloat(ptr,(float)obj->motionPathOffset.z());
// 
// 				strcpy((char *)ptr,obj->motionPath->GetTag());
// 
// 				packetLength=(ptr-dat)+len+1;
// 				packetLength=(packetLength+3)&~3;
// 
// 				return SendPacket(clientId,packetLength,dat);
// 			}
// 		}
// 	}
// 	return YSERR;
// }

YSRESULT FsSocketServer::FlushSendQueue(int clientId,unsigned timeout)
{
	if(user[clientId].state!=FSUSERSTATE_NOTCONNECTED && user[clientId].nSendQueueFilled>0)
	{
		YSRESULT res;
		res=Send(clientId,user[clientId].nSendQueueFilled,user[clientId].sendQueue,timeout);
		if(res==YSOK)
		{
			user[clientId].nSendQueueFilled=0;
			return YSOK;
		}
	}
	return YSERR;
}

void FsSocketServer::AddTestAirplaneInClient(int clientId,int type)
{
	int idOnSvr;
	FsAirplane *air;

	air=sim->world->AddAirplane("YS-11",YSFALSE);
	if(air!=NULL)
	{
		air->iff=FS_IFF0;
		sim->world->SettleAirplane(*air,"NORTH10000_01");
		air->netType=FSNET_REMOTE;
		air->name.Set("COMPUTER");


		if(netcfg!=NULL && netcfg->useMissile!=YSTRUE)
		{
			air->Prop().UnloadGuidedAAM();
			air->Prop().UnloadGuidedAGM();
		}


		idOnSvr=FsExistence::GetSearchKey(air);

		BroadcastAddAirplane(air,FSNET_LOCAL);  // Remote in the server==Local in clients

		SendSetTestAutopilot(clientId,idOnSvr,type);
	}
}

void FsSocketServer::PrintPlayerList(void)
{
	if(ServerPlayerPlane()!=NULL && ServerPlayerPlane()->IsAlive()==YSTRUE)
	{
		fsConsole.Printf("[%04d] (SERVER) (IFF=%d) %s",
		    FsExistence::GetSearchKey(ServerPlayerPlane()),
		    ServerPlayerPlane()->iff,
		    username.Txt());
	}
	else
	{
		fsConsole.Printf("[****] (SERVER) %s",username.Txt());
	}

	int i;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			if(user[i].air!=NULL && user[i].air->IsAlive()==YSTRUE)
			{
				YsString ipAddrString;
				fsConsole.Printf("[%04d] (IFF=%d) IP-ADDR=%s %s",
				    FsExistence::GetSearchKey(user[i].air),
				    user[i].air->iff,
				    GetUserIpAddressString(i,ipAddrString),
				    user[i].username.Txt());
			}
			else
			{
				YsString ipAddrString;
				fsConsole.Printf("[****] (IFF=*)  IP-ADDR=%s %s",
				    GetUserIpAddressString(i,ipAddrString),
				    user[i].username.Txt());
			}
		}
	}
}

int FsSocketServer::GetMaxNumUser(void) const
{
	return FS_MAX_NUM_USER;
}

YSBOOL FsSocketServer::IsUserLoggedOn(int id) const
{
	if(0<=id && id<FS_MAX_NUM_USER && user[id].state!=FSUSERSTATE_NOTCONNECTED)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsSocketServer::UserHasAirplane(int id) const
{
	if(0<=id && id<FS_MAX_NUM_USER && user[id].air!=NULL)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

const char *FsSocketServer::GetUserName(int id) const
{
	if(0<=id && id<FS_MAX_NUM_USER && user[id].state!=FSUSERSTATE_NOTCONNECTED) // loggedOn==YSTRUE)
	{
		return user[id].username;
	}
	return NULL;
}

const char *FsSocketServer::GetUserIpAddressString(int id,YsString &buf) const
{
	char str[256];
	if(0<=id && id<FS_MAX_NUM_USER && user[id].state!=FSUSERSTATE_NOTCONNECTED) // loggedOn==YSTRUE)
	{
		sprintf(str,"%d.%d.%d.%d",
		    user[id].ipAddr[0],
		    user[id].ipAddr[1],
		    user[id].ipAddr[2],
		    user[id].ipAddr[3]);
		buf.Set(str);
		return buf;

	}
	return "";
}

void FsSocketServer::ClearUserAirplaneForMemoryCleanUpPurpose(void)
{
	int id;
	for(id=0; id<FS_MAX_NUM_USER; id++)
	{
		user[id].air=NULL;
	}
}

YSRESULT FsSocketServer::FlushAllSendQueue(unsigned timeout)
{
	int i;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED)
		{
			FlushSendQueue(i,timeout);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::BroadcastPacket(YSSIZE_T nDat,unsigned char dat[],unsigned version)
{
	int i;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state==FSUSERSTATE_LOGGEDON && user[i].version>=version)
		{
			SendPacket(i,nDat,dat);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::SendPacket(int clientId,YSSIZE_T nDat,unsigned char dat[])
{
	YSSIZE_T total;
	total=nDat+4;

	if(user[clientId].state==FSUSERSTATE_NOTCONNECTED)
	{
		printf("Tried to send a packet to unlogged-on user.\n");
		AddMessage("Tried to send a packet to unlogged-on user.");
		return YSERR;
	}

	if(FsNetworkUser::SENDQUEUESIZE<total)
	{
		AddMessage("Tried to send huge packet. (larger than queue)");
		return YSERR;
	}


	// if sending AIRPLANESTATE or GROUNDSTATE, delete old ones.
	if(FsGetInt(dat)==FSNETCMD_AIRPLANESTATE ||
	   FsGetInt(dat)==FSNETCMD_GROUNDSTATE)
	{
		FsDeleteOldStatePacket
		    (user[clientId].nSendQueueFilled,user[clientId].sendQueue,
		     FsGetInt(dat),FsGetInt(dat+8));
	}


	if(FsNetworkUser::SENDQUEUESIZE<user[clientId].nSendQueueFilled+total)
	{
		// Buffer overflow?
		FlushSendQueue(clientId,FS_NETEMERGENCYTIMEOUT);  // Try flushing send queue

		// If couldn't flush, delete low priority packets.
		if(user[clientId].nSendQueueFilled!=0)
		{
			FsDeleteLowPriorityPacket(user[clientId].nSendQueueFilled,user[clientId].sendQueue);
			printf("X1\n");
		}

		// Even if it's failed, I abundon old packets. What else can I do?
		if(FsNetworkUser::SENDQUEUESIZE<user[clientId].nSendQueueFilled+total)
		{
			if(FsIsLowPriorityPacket(FsGetInt(dat))==YSTRUE)
			{
				printf("X3\n");
				return YSERR;
			}
			printf("X2\n");
			user[clientId].nSendQueueFilled=0;
		}
	}

	unsigned char nByte[4];
	FsSetUnsignedInt(nByte,(unsigned int)nDat);
	user[clientId].AddToSendQueue(4,nByte);
	user[clientId].AddToSendQueue(nDat,dat);

	return YSOK;
}

// YSRESULT FsSocketServer::SendPriorityPacket(int clientId,unsigned int nDat,unsigned char dat[])
// {
// 	unsigned int i,total;
// 	YsArray <unsigned char> buf;
// 	YSRESULT res;
// 
// 	total=nDat+4;
// 	buf.Set(total,NULL);
// 	FsSetUnsignedInt(buf,nDat);
// 	for(i=0; i<nDat; i++)
// 	{
// 		buf.SetItem(4+i,dat[i]);
// 	}
// 
// 	for(i=0; i<FS_NETPRIORITYPACKETRETRY; i++)
// 	{
// 		res=Send(clientId,buf.GetN(),buf,FS_NETPRIORITYPACKETTIMEOUT);
// 		if(res==YSOK)
// 		{
// 			break;
// 		}
// 		else
// 		{
// 			FsSleep(FS_NETPRIORITYPACKETSLEEP);
// 		}
// 	}
// 
// 	if(res!=YSOK)
// 	{
// 		AddMessage("Could not send a priority packet.");
// 	}
// 
// 	return res;
// }

YSRESULT FsSocketServer::ForwardPacket(int clientId,YSSIZE_T packetLength,unsigned char dat[])
{
	int i;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		if(user[i].state!=FSUSERSTATE_NOTCONNECTED && i!=clientId)
		{
			SendPacket(i,packetLength,dat);
		}
	}
	return YSOK;
}

YSRESULT FsSocketServer::EncodeObject(int &isAirplane,int &idOnSvr,FsExistence *obj)
{
	if(obj!=NULL && sim->FindAirplane(FsExistence::GetSearchKey(obj))!=NULL)
	{
		isAirplane=1;
		idOnSvr=FsExistence::GetSearchKey(obj);
		return YSOK;
	}
	else if(obj!=NULL && sim->FindGround(FsExistence::GetSearchKey(obj))!=NULL)
	{
		isAirplane=0;
		idOnSvr=FsExistence::GetSearchKey(obj);
		return YSOK;
	}
	else
	{
		isAirplane=0;
		idOnSvr=-1;
		return YSOK;
	}
}

YSRESULT FsSocketServer::DecodeObject(FsExistence **obj,int isAirplane,int idOnSvr)
{
	if(idOnSvr<0)
	{
		(*obj)=NULL;
		return YSOK;
	}
	else if(isAirplane!=0)
	{
		(*obj)=sim->FindAirplane(idOnSvr);
		return YSOK;
	}
	else
	{
		(*obj)=sim->FindGround(idOnSvr);
		return YSOK;
	}

	(*obj)=NULL;
	return YSOK;
}

////////////////////////////////////////////////////////////


FsSocketClient::FsSocketClient(const char username[],const int port,FsSimulation *sim,FsNetConfig *cfg) : 
    YsSocketClient(port),
    FsClientVariable(username,sim,cfg)
{
	nMsg=0;
	nComBuf=0;
	receivedApproval=YSFALSE;
	receivedRejection=YSFALSE;
	nSendQueueFilled=0;

	sideWindowAssigned=YSFALSE;

	reportedServerVersion=0;

	svrUseMissile=YSTRUE;
	svrUseUnguidedWeapon=YSTRUE;

	fatalError=YSFALSE;
	fatalErrorCode=0;

	sim->SetNetClient(this);
}

YSRESULT FsSocketClient::Received(YSSIZE_T nBytes,unsigned char dat[])
{
	// printf("Client:Received %d bytes\n",nBytes);

	while(nBytes>0)   // Continue until all bytes are stored or processed
	{
		int i,stored;

		stored=0;
		for(i=0; i<nBytes && i+nComBuf<COMBUFSIZE; i++)
		{
			comBuf[nComBuf+i]=dat[i];
			stored++;
		}

		nComBuf+=stored;
		nBytes-=stored;
		dat+=stored;

		unsigned int comBufPtr;
		comBufPtr=0;

		for(;;)
		{
			unsigned packetLength;
			if(comBufPtr+4<=nComBuf)
			{
				packetLength=FsGetUnsignedInt(comBuf+comBufPtr);
				if(packetLength>FsNetworkUser::COMBUFSIZE-4)
				{
					// Fatal Error!  I can do nothing.
					fsConsole.Printf("Fatal data loss or corruption.\n");
					fsConsole.Printf("Cannot sustain the connection.\n");
					Disconnect();
					return YSERR;
				}
			}
			else
			{
				packetLength=0;
			}

			if(nComBuf<=comBufPtr+4 || nComBuf-comBufPtr<packetLength+4)
				// False of the first condition guarantees the validity of packetLength.
				// Thus, the second one never gives false positive.
			{
				unsigned i;
				for(i=comBufPtr; i<nComBuf; i++)   // Shift unprocessed chunk of data to top.
				{
					comBuf[i-comBufPtr]=comBuf[i];
				}
				nComBuf-=comBufPtr;
				break;
			}
			else
			{
				unsigned cmd;
				unsigned char *cmdTop;

				cmdTop=comBuf+comBufPtr+4;
				cmd=FsGetUnsignedInt(cmdTop);

			#ifdef CRASHINVESTIGATION
				printf("CMD %d IN\n",cmd);
			#endif

				if(FsVerboseMode==YSTRUE && cmd!=FSNETCMD_AIRPLANESTATE && cmd!=FSNETCMD_GROUNDSTATE)
				{
					printf("CMD %d IN\n",cmd);
				}

				switch(cmd)
				{
				case FSNETCMD_LOGON:
					logOnProcessCompleted=YSTRUE;
					break;
				case FSNETCMD_ERROR:
					ReceiveError(cmdTop);
					break;
				case FSNETCMD_LOADFIELD:
					ReceiveLoadField(packetLength,cmdTop);
					break;
				case FSNETCMD_ADDOBJECT:
					ReceiveAddObject(packetLength,cmdTop);
					break;
				case FSNETCMD_JOINAPPROVAL:
					ReceiveApproveToJoin(packetLength,cmdTop);
					break;
				case FSNETCMD_REJECTJOINREQ:
					ReceiveRejectToJoin(cmdTop);
					break;
				case FSNETCMD_AIRPLANESTATE:
					ReceiveAirplaneState(cmdTop);
					break;
				case FSNETCMD_REMOVEAIRPLANE:
					ReceiveRemoveAirplane(cmdTop);
					break;
				case FSNETCMD_REMOVEGROUND:
					ReceiveRemoveGround(cmdTop);
					break;
				case FSNETCMD_PREPARESIMULATION:
					ReceivePrepareSimulation(packetLength,cmdTop);
					break;
				case FSNETCMD_LOCKON:
					ReceiveLockOn(cmdTop);
					break;
				case FSNETCMD_MISSILELAUNCH:
					ReceiveMissileLaunch(cmdTop);
					break;
				case FSNETCMD_GROUNDSTATE:
					ReceiveGroundState(packetLength,cmdTop);
					break;
				case FSNETCMD_GETDAMAGE:
					ReceiveGetDamage(cmdTop);
					break;
				case FSNETCMD_GNDTURRETSTATE:
					ReceiveGndTurretState(cmdTop,packetLength);
					break;
				case FSNETCMD_SETTESTAUTOPILOT:
					ReceiveSetTestAutopilot(cmdTop);
					break;
				case FSNETCMD_ASSIGNSIDEWINDOW:
					ReceiveAssignSideWindow(cmdTop);
					break;
				case FSNETCMD_VERSIONNOTIFY:
					ReceiveVersionNotify(cmdTop);
					break;
				case FSNETCMD_AIRCMD:
					ReceiveAirCmd(cmdTop);
					break;
				case FSNETCMD_USEMISSILE:
					ReceiveUseMissile(packetLength,cmdTop);
					break;

				case FSNETCMD_TEXTMESSAGE:             //  32
					ReceiveTextMessage(cmdTop);
					break;
				case FSNETCMD_ENVIRONMENT:              //  33
					ReceiveEnvironment(packetLength,cmdTop);
					break;

				case FSNETCMD_REVIVEGROUND:            //  35
					ReceiveReviveGround();
					break;
				case FSNETCMD_WEAPONCONFIG:            //  36
					ReceiveWeaponConfig(cmdTop);
					break;
				case FSNETCMD_LISTUSER:         //  37
					ReceiveListUser(cmdTop);
					break;
				case FSNETCMD_USEUNGUIDEDWEAPON:       //  39
					ReceiveUseUnguidedWeapon(packetLength,cmdTop);
					break;
				case FSNETCMD_AIRTURRETSTATE:             //  40
					ReceiveAirTurretState(cmdTop,packetLength);
					break;
				case FSNETCMD_CTRLSHOWUSERNAME:        //  41
					ReceiveControlShowUserName(packetLength,cmdTop);
					break;
				case FSNETCMD_CONFIGSTRING:            //  43
					ReceiveConfigString(packetLength,cmdTop);
					break;
				case FSNETCMD_LIST:                    //  44
					ReceiveList(packetLength,cmdTop);
					break;
				case FSNETCMD_READBACK:
					ReceiveReadBack(cmdTop);
					break;
				case FSNETCMD_SMOKECOLOR:
					ReceiveSmokeColor(cmdTop);
					break;
				case FSNETCMD_REPORTSCORE:
					ReceiveScore(cmdTop);
					break;

				case FSNETCMD_GNDCMD:                 //  45
					ReceiveGndCmd(cmdTop);
					break;

				case FSNETCMD_SERVER_FORCE_JOIN:      //  47
					ReceiveForceJoin(cmdTop);
					break;
				case FSNETCMD_FOGCOLOR:             //  48
					ReceiveFogColor(cmdTop);
					break;
				case FSNETCMD_SKYCOLOR:             //  49
					ReceiveSkyColor(cmdTop);
					break;
				case FSNETCMD_GNDCOLOR:             //  50
					ReceiveGroundColor(cmdTop);
					break;
				case FSNETCMD_RESERVED_FOR_LIGHTCOLOR:             //  51
				case FSNETCMD_RESERVED21:             //  52
				case FSNETCMD_RESERVED22:             //  53
				case FSNETCMD_RESERVED23:             //  54
				case FSNETCMD_RESERVED24:             //  55
				case FSNETCMD_RESERVED25:             //  56
				case FSNETCMD_RESERVED26:             //  57
				case FSNETCMD_RESERVED27:             //  58
				case FSNETCMD_RESERVED28:             //  59
				case FSNETCMD_RESERVED29:             //  60
				case FSNETCMD_RESERVED30:             //  61
				case FSNETCMD_RESERVED31:             //  62
				case FSNETCMD_RESERVED32:             //  63
				case FSNETCMD_OPENYSF_RESERVED33:     //  64 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED34:     //  65 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED35:     //  66 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED36:     //  67 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED37:     //  68 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED38:     //  69 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED39:     //  70 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED40:     //  71 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED41:     //  72 Reserved for OpenYSF
				case FSNETCMD_OPENYSF_RESERVED42:     //  73 Reserved for OpenYSF
				case FSNETCMD_RESERVED43:             //  74
				case FSNETCMD_RESERVED44:             //  75
				case FSNETCMD_RESERVED45:             //  76
				case FSNETCMD_RESERVED46:             //  77
				case FSNETCMD_RESERVED47:             //  78
				case FSNETCMD_RESERVED48:             //  79
				case FSNETCMD_RESERVED49:             //  80
					break;

				default:
					AddMessage("Unknown or Unimplemented command is received.");
					AddMessage("Connection closed.");
					printf("Unknown command %d\n",cmd);
					Disconnect();
					return YSERR;
				}
				comBufPtr+=packetLength+4;
			}
		}
	}
	return YSOK;
}

YSRESULT FsSocketClient::ConnectionClosedByServer(void)
{
	int i;
	for(i=0; i<GetNumMessage(); i++)
	{
		fsConsole.Printf("%s",GetMessage(i));
	}
	ClearMessage();
	AddMessage("Connection Closed by the Server.");
	connectionClosedByServer=YSTRUE;
	return YSOK;
}

void FsSocketClient::CheckPendingData(
    const double &currentTime,
    FsGuiChooseAircraft &chooseAirplane,
    FsGuiChooseField &chooseStartPosition)
{
	int i;

	if(airRmvToSend.GetN()>0 ||   // Do not combine the inside if-statement with this if-statement.
	   gndRmvToSend.GetN()>0 ||   // SendCriticalInfoTimer is be updated if nothing is to be re-sent (in the last else block).
	   airUnjoinToSend.GetN()>0)
	{
		if(sendCriticalInfoTimer<currentTime)
		{
			printf("Re-Sending Removal/Unjoin request.\n");

			sendCriticalInfoTimer=currentTime+3.0;

			forYsArray(i,airRmvToSend)
			{
				SendRemoveAirplane(airRmvToSend[i],YSFALSE);
			}
			forYsArray(i,gndRmvToSend)
			{
				SendRemoveGround(gndRmvToSend[i],YSFALSE);
			}
			forYsArray(i,airUnjoinToSend)
			{
				SendUnjoin(airUnjoinToSend[i],YSFALSE);
			}
		}
	}
	else if(waitingForJoiningApproval==YSTRUE &&   // Do not combine the inside if-statement with this if-statement.
	        joinReqReadBack!=YSTRUE &&             // SendCriticalInfoTimer is be updated if nothing is to be re-sent (in the last else block).
	        chooseStartPosition.selStp.Strlen()>0)
	{
		if(sendCriticalInfoTimer<currentTime)
		{
			printf("Re-Sending Join Request.\n");

			sendCriticalInfoTimer=currentTime+3.0;

			SendJoinRequest(
			    iff,chooseAirplane.selAir,
			    chooseStartPosition.selStp,
			    chooseAirplane.selFuel,
			    chooseAirplane.SmokeLoaded());
		}
	}
	else
	{
		// The program falls into this else block if no information needs to be re-sent.
		// In this case, always push the timer 3 seconds ahead of time.
		sendCriticalInfoTimer=currentTime+3.0;
	}
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
	FsAirplane *air;
	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		if(air->netType==FSNET_LOCAL)
		{
			if(air->Prop().IsAlive()==YSTRUE)
			{
				unsigned int packetSize;
				unsigned char dat[512];

				packetSize=air->Prop().NetworkEncode(dat,FsExistence::GetSearchKey(air),sim->currentTime,YSTRUE);
				if(packetSize==0)
				{
					packetSize=air->Prop().NetworkEncode(dat,FsExistence::GetSearchKey(air),sim->currentTime,YSFALSE);
				}
				SendPacket(packetSize,dat);

				if(air->Prop().TurretStateChanged()==YSTRUE)
				{
					air->Prop().SaveTurretState();
					packetSize=air->Prop().EncodeTurretState(dat,FsExistence::GetSearchKey(air));
					if(packetSize>0)
					{
						SendPacket(packetSize,dat);
					}
				}
			}
		}
	}

	return YSOK;
}

YSRESULT FsSocketClient::NotifyGroundState(void)
{
	FsGround *gnd;
	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->netType==FSNET_LOCAL && gnd->Prop().IsAlive()==YSTRUE)
		{
			if(gnd->GetPosition()!=gnd->netPos ||
			   YsEqual(gnd->GetAttitude().h(),gnd->netAtt.h())!=YSTRUE ||
			   YsEqual(gnd->GetAttitude().p(),gnd->netAtt.p())!=YSTRUE ||
			   YsEqual(gnd->GetAttitude().b(),gnd->netAtt.b())!=YSTRUE ||
			   gnd->Prop().IsFiringAaa()!=gnd->netShootingAaa ||
			   gnd->Prop().IsFiringCannon()!=gnd->netShootingCannon ||
			   gnd->Prop().GetDamageTolerance()!=gnd->netDamageTolerance)
			{
				unsigned int packetSize;
				unsigned char dat[512];
				packetSize=gnd->Prop().NetworkEncode(dat,FsExistence::GetSearchKey(gnd),sim->currentTime,YSTRUE);
				if(packetSize==0)
				{
					packetSize=gnd->Prop().NetworkEncode(dat,FsExistence::GetSearchKey(gnd),sim->currentTime,YSFALSE);
				}
				SendPacket(packetSize,dat);

				gnd->netPos=gnd->GetPosition();
				gnd->netAtt=gnd->GetAttitude();
				gnd->netShootingAaa=gnd->Prop().IsFiringAaa();
				gnd->netShootingCannon=gnd->Prop().IsFiringCannon();

				if(gnd->Prop().HasTurret()==YSTRUE && gnd->Prop().TurretStateChanged()==YSTRUE)
				{
					gnd->Prop().SaveTurretState();
					packetSize=gnd->Prop().EncodeTurretState(dat,FsExistence::GetSearchKey(gnd));
					if(packetSize>0)
					{
						SendPacket(packetSize,dat);
					}
				}
			}
		}
	}
	return YSOK;
}

YSRESULT FsSocketClient::AddMessage(const char *txt)
{
	if(nMsg<MAXNUMMESSAGE)
	{
		strcpy(msg[nMsg],txt);
		nMsg++;
		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsSocketClient::SendLogOn(const char username[],int version)
{
	unsigned char dat[256];

	FsSetUnsignedInt(dat,FSNETCMD_LOGON);

	strncpy((char *)dat+4,username,15);
	(dat+4)[15]=0;
	FsSetUnsignedInt(dat+20,version);

	if(15<strlen(username))
	{
		strncpy((char *)dat+24,username,200);
		dat[224]=0;
		dat[225]=0;
		dat[226]=0;
		dat[227]=0;
		return SendPacket(228,dat);
	}
	else
	{
		return SendPacket(24,dat);
	}
}

YSRESULT FsSocketClient::SendError(int errorCode)
{
	unsigned char dat[16];
	FsSetInt(dat+0,FSNETCMD_ERROR);
	FsSetInt(dat+4,errorCode);
	return SendPacket(8,dat);
}

YSRESULT FsSocketClient::SendJoinRequest
    (int iff,const char *air,const char *stp,int fuelPercent,YSBOOL smokeOil)
{
	unsigned char dat[256];
	FsSetInt(dat+0x0000,FSNETCMD_JOINREQUEST);
	FsSetShort(dat+0x0004,(short)iff);
	FsSetShort(dat+0x0006,0);
	strncpy((char *)(dat+0x0008),air,31);
	(dat+0x0008)[31]=0;
	strncpy((char *)(dat+0x0028),stp,31);
	(dat+0x0028)[31]=0;

	FsSetShort(dat+0x0048,1);  //  FSNETCMD_JOINREQUEST version.  // 2004/09/02
	FsSetShort(dat+0x004a,(short)fuelPercent);
	FsSetShort(dat+0x004c,(smokeOil==YSTRUE ? 1 : 0));

	return SendPacket(0x004e,dat);
}

YSRESULT FsSocketClient::SendUnjoin(int airId,YSBOOL explosion)
{
	unsigned char dat[256];

	if(YsIsIncluded <unsigned int> (airUnjoinToSend.GetN(),airUnjoinToSend,airId)!=YSTRUE)
	{
		airUnjoinToSend.Append(airId);  // Expect RemoveAirplane as read back
	}

	FsSetInt  (dat   ,FSNETCMD_UNJOIN);
	FsSetInt  (dat+ 4,airId);
	FsSetShort(dat+ 8,(explosion==YSTRUE ? 1 : 0));
	FsSetShort(dat+10,0);

	return SendPacket(12,dat);
}

YSRESULT FsSocketClient::SendRequestTestAirplane(void)
{
	unsigned char dat[256];
	FsSetInt(dat,FSNETCMD_REQUESTTESTAIRPLANE);
	return SendPacket(4,dat);
}

YSRESULT FsSocketClient::SendKillServer(void)
{
	unsigned char dat[256];
	FsSetInt(dat,FSNETCMD_KILLSERVER);
	return SendPacket(4,dat);
}

YSRESULT FsSocketClient::SendTestPacket(void)
{
	unsigned char dat[4];
	FsSetInt(dat,FSNETCMD_TESTPACKET);
	SendPacket(4,dat);
	return YSOK;
}

YSRESULT FsSocketClient::SendTestHugePacket(void)  // This must make server disconnect the client.
{
	unsigned char dat[8192];
	FsSetInt(dat,8192);
	Send(8192,dat,0);
	AddMessage("Test Packet (Huge) is sent.");
	AddMessage("The server is supposed to disconnect you.");
	return YSOK;
}

YSRESULT FsSocketClient::SendRemoveAirplane(int airId,YSBOOL explosion)
{
	unsigned char dat[256];

	if(YsIsIncluded <unsigned int> (airRmvToSend.GetN(),airRmvToSend,airId)!=YSTRUE)
	{
		airRmvToSend.Append(airId);
	}

	FsSetInt  (dat   ,FSNETCMD_REMOVEAIRPLANE);
	FsSetInt  (dat +4,airId);
	FsSetShort(dat +8,(explosion==YSTRUE ? 1 : 0));
	FsSetShort(dat+10,0);
	return SendPacket(12,dat);
}

YSRESULT FsSocketClient::SendRemoveGround(int gndId,YSBOOL explosion)
{
	unsigned char dat[256];

	if(YsIsIncluded <unsigned int> (gndRmvToSend.GetN(),gndRmvToSend,gndId)!=YSTRUE)
	{
		gndRmvToSend.Append(gndId);
	}

	FsSetInt  (dat   ,FSNETCMD_REMOVEGROUND);
	FsSetInt  (dat +4,gndId);
	FsSetShort(dat +8,(explosion==YSTRUE ? 1 : 0));
	FsSetShort(dat+10,0);
	return SendPacket(12,dat);
}

YSRESULT FsSocketClient::SendStateChange(void)
{
	FsAirplane *air;
	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		if(air->netType==FSNET_LOCAL)
		{
			// Is this dead?
			if(air->netAlive==YSTRUE && air->IsAlive()!=YSTRUE)
			{
				SendRemoveAirplane(FsExistence::GetSearchKey(air),YSFALSE);
				air->netAlive=air->IsAlive();
			}

			// Which airplane locked on which?
			if(sim->FindAirplane(air->Prop().GetAirTargetKey())!=air->netAirTarget)
			{
				if(sim->FindAirplane(air->Prop().GetAirTargetKey())!=NULL)
				{
					SendLockOn(FsExistence::GetSearchKey(air),YSTRUE,air->Prop().GetAirTargetKey(),YSTRUE);
				}
				else
				{
					SendLockOn(FsExistence::GetSearchKey(air),YSTRUE,-1,YSTRUE);
				}
				air->netAirTarget=sim->FindAirplane(air->Prop().GetAirTargetKey());
			}
			if(sim->FindGround(air->Prop().GetGroundTargetKey())!=air->netGndTarget)
			{
				if(sim->FindGround(air->Prop().GetGroundTargetKey())!=NULL)
				{
				 	SendLockOn(FsExistence::GetSearchKey(air),YSTRUE,air->Prop().GetGroundTargetKey(),YSFALSE);
				}
				else
				{
					SendLockOn(FsExistence::GetSearchKey(air),YSTRUE,-1,YSFALSE);
				}
				air->netGndTarget=sim->FindGround(air->Prop().GetGroundTargetKey());
			}
		}
	}

	FsGround *gnd;
	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->netType==FSNET_LOCAL)
		{
			// Is this dead?
			if(gnd->Prop().IsAlive()!=YSTRUE && gnd->netAlive==YSTRUE)
			{
				SendRemoveGround(FsExistence::GetSearchKey(gnd),YSFALSE);
				gnd->netAlive=gnd->Prop().IsAlive();
			}
		}
	}

	return YSOK;
}

YSRESULT FsSocketClient::SendLockOn(int id1,YSBOOL id1isAir,int id2,YSBOOL id2isAir)
{
	unsigned char dat[4096],*ptr;
	ptr=dat;
	FsPushInt(ptr,FSNETCMD_LOCKON);
	FsPushInt(ptr,id1);
	FsPushInt(ptr,id1isAir);
	FsPushInt(ptr,id2);
	FsPushInt(ptr,id2isAir);
	return SendPacket(ptr-dat,dat);
}

YSRESULT FsSocketClient::SendMissileLaunch(FsWeaponRecord &rec)
{
	unsigned length;
	unsigned char dat[512];
	length=FsEncodeWeaponRecord(dat,sim,rec);
	if(length>0)
	{
		SendPacket(length,dat);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketClient::SendGetDamage(FsExistence *victim,FsExistence *firedBy,int power,enum FSDIEDOF diedOf,FSWEAPONTYPE weaponType)
{
	unsigned char dat[256],*ptr;
	int victimIsAir,firedByAir;
	int victimIdOnSvr,firedByIdOnSvr;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_GETDAMAGE);

	EncodeObject(victimIsAir,victimIdOnSvr,victim);
	EncodeObject(firedByAir,firedByIdOnSvr,firedBy);

	FsPushInt(ptr,victimIsAir);
	FsPushInt(ptr,victimIdOnSvr);

	FsPushInt(ptr,firedByAir);
	FsPushInt(ptr,firedByIdOnSvr);

	FsPushShort(ptr,(short)power);
	FsPushShort(ptr,(short)diedOf);

	FsPushShort(ptr,(short)weaponType);  // Since 20100630 net version

	return SendPacket(ptr-dat,dat);
}

YSRESULT FsSocketClient::SendRequestToBeSideWindow(const double &hdgOffset,const double &pchOffset)
{
	unsigned char *ptr;
	unsigned char dat[256];

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_REQTOBESIDEWINDOWOFSVR);
	FsPushFloat(ptr,(float)hdgOffset);
	FsPushFloat(ptr,(float)pchOffset);

	return SendPacket(ptr-dat,dat);
}

YSRESULT FsSocketClient::SendResendAirRequest(int idOnSvr)
{
	unsigned char *ptr,dat[256];

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_RESENDAIRREQUEST);
	FsPushInt(ptr,idOnSvr);

	return SendPacket(ptr-dat,dat);
}

YSRESULT FsSocketClient::SendResendGndRequest(int idOnSvr)
{
	unsigned char *ptr,dat[256];

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_RESENDGNDREQUEST);
	FsPushInt(ptr,idOnSvr);

	return SendPacket(ptr-dat,dat);
}

// SendAirCmd is supported after 20010624
YSRESULT FsSocketClient::SendAirCmd(int idOnSvr,const char cmd[])
{
	unsigned char *ptr,dat[256];

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_AIRCMD);
	FsPushInt(ptr,idOnSvr);
	strcpy((char *)(dat+8),cmd);

	return SendPacket(8+strlen(cmd)+1,dat);
}

YSRESULT FsSocketClient::SendGndCmd(int idOnCli,const char cmd[])
{
	unsigned char *ptr,dat[256];

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_GNDCMD);
	FsPushInt(ptr,idOnCli);
	strcpy((char *)(dat+8),cmd);

	return SendPacket(8+strlen(cmd)+1,dat);
}

YSRESULT FsSocketClient::SendTextMessage(const char txt[])
{

	unsigned char dat[256];
	FsSetInt(dat  ,FSNETCMD_TEXTMESSAGE);
	FsSetInt(dat+4,0);  // Will be used for destination selection
	FsSetInt(dat+8,0);  // Will be used for source indication
	strncpy((char *)dat+12,txt,244);
	dat[255]=0;

	YSSIZE_T packetLength=YsSmaller <YSSIZE_T> ((12+strlen(txt)+1 +3)&~3,256);
	return SendPacket(packetLength,dat);
}

YSRESULT FsSocketClient::SendEnvironmentRequest(void)
{
	unsigned char dat[4];
	FsSetInt(dat,FSNETCMD_ENVIRONMENT);
	return SendPacket(4,dat);
}

YSRESULT FsSocketClient::SendResendJoinApproval(void)
{
	unsigned char dat[4];
	FsSetInt(dat,FSNETCMD_NEEDRESENDJOINAPPROVAL);
	return SendPacket(4,dat);
}

YSRESULT FsSocketClient::SendWeaponConfig(int airId,YSSIZE_T nWeaponConfig,const int weaponConfig[])
{

	unsigned char dat[256];
	YSSIZE_T packetLength=FsEncodeWeaponConfig(dat,airId,nWeaponConfig,weaponConfig);
	return SendPacket(packetLength,dat);
}

YSRESULT FsSocketClient::SendListUser(void)
{
	unsigned char dat[4];
	FsSetInt(dat,FSNETCMD_LISTUSER);
	return SendPacket(4,dat);
}

YSRESULT FsSocketClient::SendQueryAirStateForReducingWarpProblem(void)
{
	unsigned char dat[256],*ptr;
	FsAirplane *air;
	int n;

	printf("SendQueryAirState\n");

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_QUERYAIRSTATE);
	FsPushInt(ptr,0);

	n=0;
	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL && (ptr-dat)<240)
	{
		if(air->netType==FSNET_LOCAL)
		{
			air->netPrevState.tLocal=-1.0;
			air->netPrevState.tRemote=-1.0;
			air->netNextState.tLocal=-1.0;
			air->netNextState.tRemote=-1.0;
			FsPushInt(ptr,FsExistence::GetSearchKey(air));
			n++;
		}
	}

	FsSetInt(dat+4,n);

	return SendPacket(ptr-dat,dat);
}

YSRESULT FsSocketClient::SendEnvironmentReadBack(void)
{
	return SendReadBack(FSNETREADBACK_ENVIRONMENT,0);
}

YSRESULT FsSocketClient::SendRemoveAirplaneReadBack(int idOnSvr)
{
	return SendReadBack(FSNETREADBACK_REMOVEAIRPLANE,idOnSvr);
}

YSRESULT FsSocketClient::SendRemoveGroundReadBack(int idOnSvr)
{
	return SendReadBack(FSNETREADBACK_REMOVEGROUND,idOnSvr);
}

YSRESULT FsSocketClient::SendReadBack(int readBackType,int readBackParam)
{
	unsigned char dat[256],*ptr;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_READBACK);
	FsPushShort(ptr,(short)readBackType);
	FsPushShort(ptr,0); // Padding
	FsPushInt(ptr,readBackParam);

	return SendPacket(ptr-dat,dat);
}

YSRESULT FsSocketClient::SendSmokeColor(int idOnSvr,int smkIdx,const YsColor &col)
{
	unsigned char dat[256],*ptr;

	ptr=dat;

	FsPushInt(ptr,FSNETCMD_SMOKECOLOR);
	FsPushInt(ptr,idOnSvr);
	FsPushUnsignedChar(ptr,smkIdx);  // Smoke Pos
	FsPushUnsignedChar(ptr,(char)col.Ri());
	FsPushUnsignedChar(ptr,(char)col.Gi());
	FsPushUnsignedChar(ptr,(char)col.Bi());

	return SendPacket(ptr-dat,dat);
}

YSRESULT FsSocketClient::ReceiveError(unsigned char cmdTop[])
{
	int errorCode;
	char msg[256];

	errorCode=FsGetInt(cmdTop+4);
	this->lastErrorFromServer=errorCode;
	switch(errorCode)
	{
	case FSNETERR_VERSIONCONFLICT:
		AddMessage("ERROR : Version Conflict.");
		AddMessage("Client and Server must use the same version.");
		fatalError=YSTRUE;
		fatalErrorCode=FsClientRunLoop::CLIENT_FATAL_VERSIONCONFLICT;
		break;
	case FSNETERR_CANNOTADDOBJECT:
		AddMessage("ERROR : Cannot add object.");
		break;
	case FSNETERR_REJECT:
		AddMessage("ERROR : Request is rejected.");
		break;
	case FSNETERR_CANNOTSUSTAIN:
		AddMessage("ERROR : Cannot sustain connection.");
		break;
	default:
		sprintf(msg,"ERROR : Eror code %d",errorCode);
		AddMessage(msg);
		break;
	}
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveLoadField(int packetLength,unsigned char dat[])
{
	YSBOOL loadYFS;
	double x,y,z,h,p,b;
	char msg[256];
	unsigned flag;
	YsVec3 pos;
	YsAtt3 att;

	flag=FsGetUnsignedInt(dat+36);
	x=FsGetFloat(dat+40);
	y=FsGetFloat(dat+44);
	z=FsGetFloat(dat+48);
	h=FsGetFloat(dat+52);
	p=FsGetFloat(dat+56);
	b=FsGetFloat(dat+60);
	loadYFS=((flag&1)!=0 ? YSTRUE : YSFALSE);

	pos.Set(x,y,z);
	att.Set(h,p,b);

	// Read Back >>
	SendPacket(packetLength,dat);
	// Read Back <<

	// Check field already loaded >>
	YsString loadedFldName;
	YsVec3 loadedFldPos;
	YsAtt3 loadedFldAtt;
	if(sim->GetLoadedField(loadedFldName,loadedFldPos,loadedFldAtt)==YSOK)
	{
		if(strcmp(loadedFldName,(char *)dat+4)==0 && loadedFldPos==pos && loadedFldAtt==att)
		{
			return YSOK;
		}
	}
	// Check field already loaded <<



	sprintf(msg,"Loading Field : %s",dat+4);
	AddMessage(msg);

	fieldName.Set((char *)dat+4);

	SendEnvironmentRequest();

	if(sim->world->AddField(NULL,(char *)dat+4,pos,att,loadYFS,YSFALSE)!=NULL)
	{
		return YSOK;
	}
	else
	{
		AddMessage("ERROR:");
		AddMessage("The server is using a field that is not installed on this client.");
		AddMessage("The server may be running a newer version of YSFLIGHT, or ");
		AddMessage("the server has an add-on field installed.");
		AddMessage("Please contact the server manager for this issue.");

		sprintf(msg,"Field Name=%s\n",dat+4);
		AddMessage(msg);

		fieldNotAvailable=YSTRUE;
		return YSERR;
	}
}

YSRESULT FsSocketClient::ReceiveAddObject(unsigned packetLength,unsigned char dat[])
{
	short airOrGnd;
	FSNETTYPE netType;
	int idOnSvr;
	FSIFF iff;
	unsigned int flags0,airGndFlags;
	float x,y,z,h,p,b;
	YsVec3 pos;
	YsAtt3 att;
	char *identifier,*substIdName;
	const char *username;
	const unsigned char *ptr=dat;;
	int idNumber;
	double outsideRadius;
	int objClass,objCategory,objNationality;
	YSBOOL isJet;

	FsPopInt(ptr);  // Skip FSNETCMD_ADDOBJECT
	airOrGnd=FsPopShort(ptr);
	netType= (FSNETTYPE)FsPopShort(ptr);
	idOnSvr= FsPopInt(ptr);
	iff    = (FSIFF)FsPopShort(ptr);
	FsPopShort(ptr);  // Now it is a dummy
	x      = FsPopFloat(ptr);
	y      = FsPopFloat(ptr);
	z      = FsPopFloat(ptr);
	h      = FsPopFloat(ptr);
	p      = FsPopFloat(ptr);
	b      = FsPopFloat(ptr);

	identifier=(char *)(ptr);
	ptr+=32;

	substIdName=(char *)(ptr);
	ptr+=32;

	// Read Back >>
	switch(airOrGnd)
	{
	case 0:
		SendReadBack(FSNETREADBACK_ADDAIRPLANE,idOnSvr);
		break;
	case 1:
		SendReadBack(FSNETREADBACK_ADDGROUND,idOnSvr);
		break;
	}
	// Read Back <<


	idNumber=FsPopInt(ptr);                 // 108 bytes
	airGndFlags=FsPopInt(ptr);              // 112 bytes
	flags0=FsPopInt(ptr);                   // 116 bytes

	isJet=((flags0&2) ? YSTRUE : YSFALSE);

	outsideRadius=(double)FsPopFloat(ptr);  // 120 bytes << Minimum Requirement

	if(packetLength>=128)
	{
		objClass=FsPopShort(ptr);                       // 122
		objCategory=FsPopShort(ptr);                    // 124
		objNationality=FsPopShort(ptr);                 // 126
		FsPopShort(ptr); // Dummy Padding (future use)  // 128 bytes
	}

	if(packetLength>=176)
	{
		username=(const char *)ptr;
		ptr+=48;                                        // 176 bytes
	}
	else
	{
		username="";
	}


	pos.Set(x,y,z);
	att.Set(h,p,b);


	if(FsVerboseMode==YSTRUE)
	{
		printf("ADDOBJECT:%s(%s) KEY=%d\n",identifier,username,idOnSvr);
		printf("%s (%.2lf %.2lf %.2lf)\n",pos.Txt(),YsRadToDeg(att.h()),YsRadToDeg(att.p()),YsRadToDeg(att.b()));
		printf("AirOrGnd=%d\n",airOrGnd);
	}


	if(airOrGnd==0)
	{
		FsAirplane *air;
		air=sim->FindAirplane((unsigned)idOnSvr);
		if(air==NULL)
		{
			FsAirplane *air;
			YSBOOL isPlayer;

			isPlayer=((flags0&1) ? YSTRUE : YSFALSE);
			air=sim->world->AddAirplane(identifier,isPlayer,(unsigned)idOnSvr);

			// (*20090706)

			if(NULL==air && 0!=substIdName[0])
			{
				air=sim->world->AddAirplane(substIdName,isPlayer,(unsigned)idOnSvr);
				AddMessage("Trying Substitute Airplane.");
				if(YSTRUE==FsVerboseMode)
				{
					printf("Trying Substitute Airplane.\n");
				}
				if(NULL!=air)
				{
					air->actualIdName.Set(identifier);
					air->isNetSubstitute=YSTRUE;
				}
			}

			if(NULL==air)
			{
				AddMessage("Unknown Airplane is specified.");
				if(YSTRUE==FsVerboseMode)
				{
					printf("Unknown Airplane is specified.\n");
				}

				air=sim->world->AddMatchingAirplane(
				    (FSAIRCRAFTCLASS)objClass,(FSAIRPLANECATEGORY)objCategory,objNationality,isJet,outsideRadius,
				    isPlayer,(unsigned int)idOnSvr);

				if(NULL!=air)
				{
					char idName[256],msg[256];
					strncpy(idName,air->Prop().GetIdentifier(),31);
					idName[31]=0;

					air->actualIdName.Set(identifier);

					strcpy(msg,"Substitute ");
					strcat(msg,idName);
					AddMessage(msg);

					if(YSTRUE==FsVerboseMode)
					{
						printf("%s\n",msg);
					}
				}
				else
				{
					AddMessage("Cannot find a matching airplane.\n");
				}
			}

			if(air!=NULL)
			{
				// This block has been moved from (*20090706) >>
				if(isPlayer==YSTRUE && netcfg->recordWhenClientMode==YSTRUE)
				{
					sim->RecordPlayerChange(air);
				}
				// This block has been moved from (*20090706) <<

				air->name.Set(username);
				air->Prop().SetPosition(pos);
				air->Prop().SetAttitude(att);
				air->iff=iff;
				air->netType=netType;

				// Commented out 2006/08/25 This message is now sent from the server.
				// if(username[0]!=0)
				// {
				// 	YsString msg;
				// 	MakeTakeOffMessage(msg,air->name,air->Prop().GetIdentifier());
				// 	AddMessage(msg);
				// 	sim->AddTimedMessage(msg);  // 2006/07/18
				// }

				air->ysfId=idNumber;

				if(ServerAllowAAM()!=YSTRUE)
				{
					air->Prop().UnloadGuidedAAM();
					air->Prop().UnloadGuidedAGM();
				}
				if(ServerAllowUnguidedWeapon()!=YSTRUE)
				{
					air->Prop().UnloadUnguidedWeapon();
				}

				if(FsVerboseMode==YSTRUE)
				{
					printf("AIRSTATE:%d %d\n",air->Prop().GetFlightState(),air->Prop().GetDiedOf());
				}
			}
			else
			{
				goto CANNOTADD;
			}
		}
		else
		{
			printf("NOTICE : Air #%d already exists.\n",idOnSvr);

			// char msg[256];
			// sprintf(msg,"NOTICE : Air #%d already exists.",idOnSvr);
			// AddMessage(msg);

			// if(FsVerboseMode==YSTRUE)
			// {
			// 	printf(msg);
			// }
		}
	}
	else
	{
		FsGround *gnd;
		gnd=sim->FindGround(idOnSvr);
		if(NULL==gnd)
		{
			gnd=sim->world->AddGround(identifier,YSFALSE,idOnSvr);

			if(NULL==gnd && 0!=substIdName[0])
			{
				gnd=sim->world->AddGround(substIdName,YSFALSE,idOnSvr);
				AddMessage("Trying Substitute Ground");

				if(YSTRUE==FsVerboseMode)
				{
					printf("Trying Substitute Ground.\n");
				}
			}

			// In the current version, a client can have only one local airplane,
			// and it should be a player airplane.
			if(NULL!=gnd)
			{
				gnd->name.Set(username);
				gnd->Prop().SetPosition(pos);
				gnd->Prop().SetAttitude(att);
				gnd->iff=iff;
				gnd->gndFlag=airGndFlags;
				gnd->netType=netType;

				gnd->primaryTarget=((flags0&0x8000) ? YSTRUE : YSFALSE);

				gnd->ysfId=idNumber;

				if(gnd->Prop().GetAircraftCarrierProperty()!=NULL)
				{
					gnd->SendCommand("MAXSPEED 0kt");
					gnd->SendCommand("MAXROTAT 0deg");
				}
			}
			else
			{
				goto CANNOTADD;
			}
		}
		else
		{
			printf("NOTICE : Gnd #%d already exists.\n",idOnSvr);

			// char msg[256];
			// sprintf(msg,"NOTICE : Gnd #%d already exists.",idOnSvr);
			// AddMessage(msg);

			// if(FsVerboseMode==YSTRUE)
			// {
			// 	printf(msg);
			// }
		}
	}
	return YSOK;

CANNOTADD:
	SendError(FSNETERR_CANNOTADDOBJECT);
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveApproveToJoin(int ,unsigned char [])
{
	// Read Back >>
	SendReadBack(FSNETREADBACK_JOINAPPROVAL,0);
	// Read Back <<

	receivedApproval=YSTRUE;
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveRejectToJoin(unsigned char [])
{
	receivedRejection=YSTRUE;
	return YSOK;
}

YSBOOL FsSocketClient::ServerAllowAAM(void)
{
	return svrUseMissile;
}

YSBOOL FsSocketClient::ServerAllowUnguidedWeapon(void)
{
	return svrUseUnguidedWeapon;
}

YSBOOL FsSocketClient::ReceivedSideWindowAssignment(void)
{
	YSBOOL r;
	r=sideWindowAssigned;
	sideWindowAssigned=YSFALSE;
	return r;
}

void FsSocketClient::GetSideWindowAssignment(FsAirplane *&air,double &hdg,double &pch)
{
	air=sideWindowAirplane;
	hdg=sideWindowHdg;
	pch=sideWindowPch;
}

void FsSocketClient::ModifySideWindowAssignment(const double &hdg,const double &pch)
{
	sideWindowHdg=hdg;
	sideWindowPch=pch;
}

YSRESULT FsSocketClient::ReceiveAirplaneState(unsigned char dat[])
{
	int idOnSvr;
	FsAirplane *air;

	idOnSvr=FsGetInt(dat+8);
	air=sim->FindAirplane(idOnSvr);

	if(FsVerboseMode==YSTRUE)
	{
		printf("AirplaneState Key=%d\n",idOnSvr);
	}

	if(air!=NULL)
	{
		if(air->netAlive==YSTRUE)    // <- air->netAlive==YSTRUE : 2002/11/13 anti-ghost
		{
			FsNetReceivedAirplaneState state;
			state.Decode(dat,sim->currentTime);

			if(air->netPrevState.tRemote<state.tRemote && air->netPrevState.tLocal<state.tLocal)
			{
				if(air->netNextState.tRemote<0.0)
				{
					air->netNextState=state;
				}
				else
				{
					double dtLocal,dtRemote,ratio1,ratio2;
					dtLocal=air->netNextState.tLocal-air->netPrevState.tLocal;
					dtRemote=air->netNextState.tRemote-air->netPrevState.tRemote;
					ratio1=dtRemote/dtLocal;

					dtLocal=state.tLocal-air->netPrevState.tLocal;
					dtRemote=state.tRemote-air->netPrevState.tRemote;
					ratio2=dtRemote/dtLocal;

					if(YsAbs(1.0-ratio2)<YsAbs(1.0-ratio1))
					{
						air->netNextState=state;
					}
				}
			}
			// air->Prop().NetworkDecode(air->netPrevState,state);
		}
	}
	else
	{
		// 2007/01/07 Send and Read-Back mechanism is introduced.
		// AddMessage("Airplane Not Found.  Sending Re-send request to server.");
		// if(FsVerboseMode==YSTRUE)
		// {
		// 	printf("Airplane Not Found.  Sending Re-send request to server.\n");
		// }
		// SendResendAirRequest(idOnSvr);
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveGroundState(unsigned int packetLength,unsigned char dat[])
{
	int idOnSvr;
	FsGround *gnd;

	idOnSvr=FsGetInt(dat+8);
	gnd=sim->FindGround(idOnSvr);
	if(gnd!=NULL)
	{
		gnd->Prop().NetworkDecode(packetLength,dat,gnd->netClockRemote,gnd->netClockLocal,sim->currentTime);
	}
	else
	{
		// 2007/01/07 Send and Read-Back mechanism is introduced.
		// AddMessage("Ground Object Not Found.  Sending Re-send request to server.");
		// SendResendGndRequest(idOnSvr);
	}
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveGetDamage(unsigned char dat[])
{
	FsExistence *victim,*firedBy;
	int victimIsAir,firedByAir;
	int victimId,firedById;
	int damage;
	FSDIEDOF diedOf;
	const unsigned char *ptr=dat;;
	YSBOOL killed;

	FsPopInt(ptr);   // Skip FSNETCMD
	victimIsAir=FsPopInt(ptr);
	victimId=FsPopInt(ptr);
	DecodeObject(&victim,victimIsAir,victimId);

	if(victim!=NULL)
	{
		firedByAir=FsPopInt(ptr);
		firedById=FsPopInt(ptr);
		DecodeObject(&firedBy,firedByAir,firedById);

		damage=FsPopShort(ptr);
		diedOf=(FSDIEDOF)FsPopShort(ptr);

		if(victim->GetDamage(killed,damage,diedOf)==YSTRUE)
		{
			double rad;
			rad=victim->GetApproximatedCollideRadius();
			if(killed==YSTRUE)
			{
				FsSoundSetOneTime(FSSND_ONETIME_BLAST);
			}
			else
			{
				if(victim==sim->GetPlayerAirplane())
				{
					FsSoundSetOneTime(FSSND_ONETIME_DAMAGE);
				}
				else
				{
					FsSoundSetOneTime(FSSND_ONETIME_HIT);
				}
			}
		}
	}
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveSetTestAutopilot(unsigned char dat[])
{
	int idOnSvr;
	FsAirplane *air;
	idOnSvr=FsGetInt(dat+4);
	if((air=sim->FindAirplane(idOnSvr))!=NULL)
	{
		FsDogfight *df;
		df=FsDogfight::Create();
		df->gLimit=7.0;
		df->minAlt=1000.0;
		air->SetAutopilot(df);
		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsSocketClient::ReceiveAssignSideWindow(unsigned char dat[])
{
	const unsigned char *ptr=dat;
	int idOnSvr;
	double hdg,pch;
	FsAirplane *air;

	FsPopInt(ptr);  // Skip FSNETCMD_ASSIGNSIDEWINDOW
	idOnSvr=FsPopInt(ptr);
	hdg=FsPopFloat(ptr);
	pch=FsPopFloat(ptr);

	if((air=sim->FindAirplane(idOnSvr))!=NULL)
	{
		sideWindowAirplane=air;
		sideWindowHdg=hdg;
		sideWindowPch=pch;
		sideWindowAssigned=YSTRUE;
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveVersionNotify(unsigned char dat[])
{
	int svrVersion;
	char str[256];

	svrVersion=FsGetInt(dat+4);

	AddMessage("Version check");
	sprintf(str,"  SERVER NET-VERSION : %d",svrVersion);
	AddMessage(str);
	sprintf(str,"  CLIENT NET-VERSION : %d",YSFLIGHT_NETVERSION);
	AddMessage(str);

	if(YSFLIGHT_NETVERSION<svrVersion)
	{
		AddMessage("**** VERSION CONFLICT ****");
		AddMessage("Connection MAY BE REJECTED by the server");
		AddMessage("or may experience incompatibilities during");
		AddMessage("the game.");
		AddMessage("I strongly recommend to download");
		AddMessage("the latest version of YSFLIGHT.");
		AddMessage("");
	}
	else if(svrVersion<YSFLIGHT_NETVERSION)
	{
		AddMessage("**** VERSION CONFLICT ****");
		AddMessage("The server is running an older version of");
		AddMessage("YSFLIGHT2000.  Please ask him to download");
		AddMessage("the latest version of YSFLIGHT.");
		AddMessage("You or he may experience glitches during");
		AddMessage("the game, or the connection MAY BE REJECTED.");
		AddMessage("");
	}

	reportedServerVersion=(unsigned)svrVersion;

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveAirCmd(unsigned char dat[])
{
	int idOnSvr;
	FsAirplane *air;

	idOnSvr=FsGetInt(dat+4);
	if(NULL!=(air=sim->FindAirplane(idOnSvr)))
	{
		if(FsVerboseMode==YSTRUE)
		{
			printf("NETCMD:[%s]\n",(char *)dat+8);
		}

		air->SendCommand((char *)(dat+8));
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveGndCmd(unsigned char dat[])
{
	int idOnSvr;
	FsGround *gnd;

	idOnSvr=FsGetInt(dat+4);
	if(NULL!=(gnd=sim->FindGround(idOnSvr)))
	{
		if(FsVerboseMode==YSTRUE)
		{
			printf("NETCMD:%s\n",(char *)dat+8);
		}

		gnd->SendCommand((char *)(dat+8));
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveUseMissile(int ,unsigned char dat[])
{
	int useMissile;
	useMissile=FsGetInt(dat+4);
	svrUseMissile=(useMissile==1 ? YSTRUE : YSFALSE);
	sim->AllowAAM(svrUseMissile);
	sim->AllowAGM(svrUseMissile);


	// Read Back >>
	SendReadBack(FSNETREADBACK_USEMISSILE,0);
	// Read Back <<


	if(svrUseMissile==YSTRUE)
	{
		if(FsVerboseMode==YSTRUE)
		{
			printf("The server allows AAM/AGM.\n");
		}
		AddMessage("The server allows AAM/AGM.");
	}
	else
	{
		if(FsVerboseMode==YSTRUE)
		{
			printf("The server doesn't allow AAM/AGM.\n");
		}
		AddMessage("The server doesn't allow AAM/AGM.");
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveUseUnguidedWeapon(int ,unsigned char dat[])
{
	int useUnguidedWeapon;
	useUnguidedWeapon=FsGetInt(dat+4);
	svrUseUnguidedWeapon=(useUnguidedWeapon==1 ? YSTRUE : YSFALSE);
	sim->AllowGun(svrUseUnguidedWeapon);
	sim->AllowGun(svrUseUnguidedWeapon);
	sim->AllowRocket(svrUseUnguidedWeapon);


	// Read Back >>
	SendReadBack(FSNETREADBACK_USEUNGUIDEDWEAPON,0);
	// Read Back <<

	if(svrUseUnguidedWeapon==YSTRUE)
	{
		if(FsVerboseMode==YSTRUE)
		{
			printf("The server allows unguided weapons.\n");
		}
		AddMessage("The server allows unguided weapons.");
	}
	else
	{
		if(FsVerboseMode==YSTRUE)
		{
			printf("The server doesn't allow unguided weapons.\n");
		}
		AddMessage("The server doesn't allow unguided weapons.");
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveControlShowUserName(int ,unsigned char dat[])
{
	int showUserName;
	showUserName=FsGetInt(dat+4);
	sim->SetShowUserName(showUserName);

	// Read Back >>
	SendReadBack(FSNETREADBACK_CTRLSHOWUSERNAME,0);
	// Read Back <<

	if(FsVerboseMode==YSTRUE)
	{
		printf("Received Control Show User Name (%d)\n",showUserName);
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveConfigString(int packetLength,unsigned char dat[])
{
	char *str;
	str=(char *)dat+8;

	// Read Back >>
	SendPacket(packetLength,dat);
	// Read Back <<

	return sim->SendConfigString(str);
}

YSRESULT FsSocketClient::ReceiveList(int packetLength,unsigned char dat[])
{
	int nList,listType;
	int i;
	char *ptr;
	YsString str;

	listType=dat[4];
	nList=dat[5];
	ptr=(char *)(dat+8);

	// Read Back >>
	SendPacket(packetLength,dat);
	// Read Back <<


	// printf("List Type=%d\n",listType);
	// printf("Num Item=%d\n",nList);
	switch(listType)
	{
	case 1:
		for(i=0; i<nList; i++)
		{
			int l;
			l=(int)strlen(ptr);
			// printf("%s\n",ptr);

			str.Set(ptr);
			airNameFilter.Append(str);
			if(YSTRUE!=airplaneAvailable)
			{
				if(NULL!=sim->world->GetAirplaneTemplate(str))
				{
					airplaneAvailable=YSTRUE;
				}
			}

			ptr+=(l+1);
		}
		break;
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveReadBack(unsigned char cmdTop[])
{
	YSSIZE_T i;
	int readBackType,readBackParam;
	const unsigned char *ptr=cmdTop;

	FsPopInt(ptr);  // Skip FSNETCMD
	readBackType=FsPopShort(ptr);
	FsPopShort(ptr);  // Skip Padding
	readBackParam=FsPopInt(ptr);

	switch(readBackType)
	{
	case FSNETREADBACK_ADDAIRPLANE:
		printf("Fatal Error: FSNETREADBACK_ADDAIRPLANE is sent to the client.\n");
		exit(1);
		break;
	case FSNETREADBACK_ADDGROUND:
		printf("Fatal Error: FSNETREADBACK_ADDGROUND is sent to the client.\n");
		exit(1);
		break;
	case FSNETREADBACK_REMOVEAIRPLANE:
		{
			int idOnSvr;
			idOnSvr=readBackParam;
			printf("Receive Remove Object (Air) Read Back\n");
			forYsArrayRev(i,airRmvToSend)
			{
				if(airRmvToSend[i]==idOnSvr)
				{
					airRmvToSend.DeleteBySwapping(i);
				}
			}
		}
		break;
	case FSNETREADBACK_REMOVEGROUND:
		{
			int idOnSvr;
			idOnSvr=readBackParam;
			printf("Receive Remove Object (Gnd) Read Back\n");
			forYsArrayRev(i,gndRmvToSend)
			{
				if(gndRmvToSend[i]==idOnSvr)
				{
					gndRmvToSend.DeleteBySwapping(i);
				}
			}
		}
		break;
	case FSNETREADBACK_ENVIRONMENT:
		printf("Fatal Error: FSNETREADBACK_ENVIRONMENT is sent to the client.\n");
		exit(1);
		break;
	case FSNETREADBACK_JOINREQUEST:
		if(waitingForJoiningApproval==YSTRUE)
		{
			printf("Received Join Request Read Back.\n");
			joinReqReadBack=YSTRUE;
		}
		break;
	case FSNETREADBACK_JOINAPPROVAL:
		printf("Fatal Error: FSNETREADBACK_JOINAPPROVAL is sent to the client.\n");
		exit(1);
		break;
	case FSNETREADBACK_PREPARE:
		printf("Fatal Error: FSNETREADBACK_PREPARE is sent to the client.\n");
		exit(1);
		break;
	case FSNETREADBACK_USEMISSILE:
		printf("Fatal Error: FSNETREADBACK_USEMISSILE is sent to the client.\n");
		exit(1);
		break;
	case FSNETREADBACK_USEUNGUIDEDWEAPON:
		printf("Fatal Error: FSNETREADBACK_USEUNGUIDEDWEAPON is sent to the client.\n");
		exit(1);
		break;
	case FSNETREADBACK_CTRLSHOWUSERNAME:
		printf("Fatal Error: FSNETREADBACK_CTRLSHOWUSERNAME is sent to the client.\n");
		exit(1);
		break;
	default:
		printf("Fatal Error: Unrecognized read back is sent to the client.\n");
		exit(1);
		break;
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveSmokeColor(unsigned char cmdTop[])
{
	int idOnSvr,smkNum,r,g,b;
	const unsigned char *ptr=cmdTop;
	FsAirplane *air;

	FsPopInt(ptr);  // Skip FSNETCMD
	idOnSvr=FsPopInt(ptr);
	smkNum=FsPopUnsignedChar(ptr);
	r=FsPopUnsignedChar(ptr);
	g=FsPopUnsignedChar(ptr);
	b=FsPopUnsignedChar(ptr);

	if(NULL!=(air=sim->FindAirplane(idOnSvr)))
	{
		char str[256];
		sprintf(str,"SMOKECOL %d %d %d %d",smkNum,r,g,b);
		air->SendCommand(str);

		return YSOK;
	}

	return YSERR;
}

YSRESULT FsSocketClient::ReceiveScore(unsigned char cmdTop[])
{
	FsNetworkScoreLog newKill;
	YSBOOL scored;

	const unsigned char *ptr=cmdTop;

	FsPopInt(ptr);
	scored=(YSBOOL)FsPopShort(ptr);
	newKill.weaponType=(FSWEAPONTYPE)FsPopShort(ptr);

	const float x=FsPopFloat(ptr);
	const float y=FsPopFloat(ptr);
	const float z=FsPopFloat(ptr);
	const float t=FsPopFloat(ptr);

	newKill.pos.Set((double)x,(double)y,(double)z);
	newKill.scoreTime=(double)t;


	char buf[33];

	FsPopInt(ptr);  // Reserve
	newKill.scoredKey=FsPopInt(ptr);
	for(int idx=0; idx<32; ++idx)
	{
		buf[idx]=(char)ptr[idx];
	}
	buf[32]=0;
	newKill.scoredUsername.Set(buf);
	ptr+=32;

	for(int idx=0; idx<32; ++idx)
	{
		buf[idx]=(char)ptr[idx];
	}
	buf[32]=0;
	newKill.scoredVehicleType.Set(buf);
	ptr+=32;


	FsPopInt(ptr);  // Reserve
	newKill.killedKey=FsPopInt(ptr);
	for(int idx=0; idx<32; ++idx)
	{
		buf[idx]=(char)ptr[idx];
	}
	buf[32]=0;
	newKill.killedUsername.Set(buf);
	ptr+=32;

	for(int idx=0; idx<32; ++idx)
	{
		buf[idx]=(char)ptr[idx];
	}
	buf[32]=0;
	newKill.killedVehicleType.Set(buf);
	ptr+=32;


	newKill.scoredClientId=-2;
	newKill.killedClientId=-2;

	YsString msg;
	switch(scored)
	{
	default:
		break;
	case YSTRUE:
		this->scored.Append(newKill);
		newKill.FormatScoredMessage(msg);
		break;
	case YSFALSE:
		this->killed.Append(newKill);
		newKill.FormatKilledMessage(msg);
		break;
	}

	AddMessage(msg);
	sim->AddTimedMessage(msg);

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveSkyColor(unsigned char cmdTop[])
{
	YsColor c;
	c.SetIntRGB(cmdTop[4],cmdTop[5],cmdTop[6]);
	sim->SetSkyColor(c);
	return YSOK;
}
YSRESULT FsSocketClient::ReceiveGroundColor(unsigned char cmdTop[])
{
	YsColor c;
	c.SetIntRGB(cmdTop[4],cmdTop[5],cmdTop[6]);
	sim->SetGroundColor(c);
	return YSOK;
}
YSRESULT FsSocketClient::ReceiveFogColor(unsigned char cmdTop[])
{
	YsColor c;
	c.SetIntRGB(cmdTop[4],cmdTop[5],cmdTop[6]);
	sim->SetFogColor(c);
	return YSOK;
}
YSRESULT FsSocketClient::ReceiveForceJoin(unsigned char [])
{
	FsPushKey(FSKEY_J);
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveEnvironment(int ,unsigned char dat[])
{
	const unsigned char *ptr=dat;

	short version;
	FSENVIRONMENT env;
	unsigned flags;
	YSBOOL fog;
	double visibility,wx,wy,wz;

	FsPopInt(ptr);
	version=FsPopShort(ptr);
	env=(FSENVIRONMENT)FsPopShort(ptr);
	flags=FsPopInt(ptr);
	fog=((flags&1) ? YSTRUE : YSFALSE);
	wx=FsPopFloat(ptr);
	wy=FsPopFloat(ptr);
	wz=FsPopFloat(ptr);
	visibility=FsPopFloat(ptr);

	// if(2<=version)
	// {
	// }

	if(env!=sim->GetEnvironment()) // This check has been added 2015/04/17
	{
		sim->world->SetEnvironment(env);
	}
	sim->world->SetWind(YsVec3(wx,wy,wz));
	sim->world->SetFog(fog);
	sim->world->SetFogVisibility(visibility);


	if((flags&8)!=0)
	{
		sim->SetBlackOut((flags&4)!=0 ? YSTRUE : YSFALSE);
		AddMessage((flags&4)!=0 ? "Server Enables Black Out" : "Server Disables Black Out");
	}
	if((flags&32)!=0)
	{
		sim->SetMidAirCollision((flags&16)!=0 ? YSTRUE : YSFALSE);
		AddMessage((flags&16)!=0 ? "Server Enables Collision" : "Server Disables Collision");
	}
	if((flags&128)!=0)
	{
		sim->SetCanLandAnywhere((flags&64)!=0 ? YSTRUE : YSFALSE);
		AddMessage((flags&64)!=0 ? "Server Enables \"Can Land Anywhere\"" : "Server Disables \"Can Land Anywhere\"");
	}

	SendEnvironmentReadBack();

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveReviveGround(void)
{
	sim->ReviveGround();
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveWeaponConfig(unsigned char dat[])
{
	YsArray <int,64> weaponConfig;
	int airId;
	FsAirplane *air;

	FsDecodeWeaponConfig(airId,weaponConfig,dat);

	air=sim->FindAirplane(airId);
	if(air!=NULL)
	{
		air->Prop().ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);

		if(netcfg->recordWhenClientMode==YSTRUE)
		{
			FsSimulationEvent evt;
			evt.Initialize();
			evt.eventType=FSEVENT_SETWEAPONCONFIG;
			evt.objKey=airId;
			evt.weaponCfg.Set(weaponConfig.GetN(),weaponConfig);
			sim->AddEvent(evt);
		}

		return YSOK;
	}
	else
	{
		return YSERR;
	}

}

YSRESULT FsSocketClient::ReceiveListUser(unsigned char dat[])
{
	unsigned flags,airKey;
	flags=FsGetUnsignedInt(dat+4);
	airKey=FsGetUnsignedInt(dat+8);

	const char *serverStr="";

	if((flags&2)!=0)
	{
		serverStr="(SERVER)";
	}

	if((flags&1)!=0)
	{
		unsigned iff;
		iff=(flags>>16)&255;
		fsConsole.Printf("[%04d] (IFF=%d) %s %s",airKey,iff,dat+16,serverStr);
	}
	else
	{
		fsConsole.Printf("[****] (IFF=*) %s %s",dat+16,serverStr);
	}
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveTextMessage(unsigned char dat[])
{
	int i;
	char *ptr,prv;

	ptr=(char *)dat+12;
	if(chatLogFp.Fp()!=NULL)
	{
		fprintf(chatLogFp.Fp(),"%s\n",ptr);
	}

	prv=0;
	for(i=12; dat[i]!=0; i++)
	{
		if(dat[i]=='\r' || dat[i]=='\n')
		{
			if(dat[i]!=prv && (prv=='\r' || prv=='\n'))  // 0x0d 0x0a
			{
				prv=0;  // Not to skip following 0x0a 0x0d
				ptr=(char *)dat+i+1;
			}
			else
			{
				prv=dat[i];

				dat[i]=0;
				AddMessage(ptr);
				sim->AddTimedMessage(ptr);
				ptr=(char *)dat+i+1;
			}
		}
		else
		{
			prv=dat[i];
		}
	}

	AddMessage(ptr);
	sim->AddTimedMessage(ptr);

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveRemoveAirplane(unsigned char dat[])
{
	YSSIZE_T i;
	int idOnSvr;
	int flags;
	FsAirplane *air;

	idOnSvr=FsGetInt(dat+4);
	flags=FsGetShort(dat+8);


	forYsArrayRev(i,airUnjoinToSend)
	{
		if(airUnjoinToSend[i]==idOnSvr)
		{
			printf("Confirmed unjoin request has been received.\n");
			airUnjoinToSend.DeleteBySwapping(i);
		}
	}


	SendRemoveAirplaneReadBack(idOnSvr);

	if((air=sim->FindAirplane(idOnSvr))!=NULL)
	{
		if((flags&1)!=0)
		{
			// Add explosion here
		}

		air->Prop().SetFlightState(FSDEAD,FSDIEDOF_NULL);
		air->netAlive=YSFALSE;  // 2002/11/13 anti-ghost
		// airSvrToCli.DeleteElement(idOnSvr,air);  Commented out 2002/11/14 anti-ghost

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsSocketClient::ReceiveRemoveGround(unsigned char dat[])
{
	int idOnSvr;
	int flags;
	FsGround *gnd;

	idOnSvr=FsGetInt(dat+4);
	flags=FsGetShort(dat+8);

	SendRemoveGroundReadBack(idOnSvr);

	if((gnd=sim->FindGround(idOnSvr))!=NULL)
	{
		if((flags&1)!=0)
		{
			// Add explosion here
		}

		gnd->Prop().SetState(FSGNDDEAD);

		// gndSvrToCli.DeleteElement(idOnSvr,gnd);  Commented out 2002/11/14 anti-ghost

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT FsSocketClient::ReceivePrepareSimulation(int ,unsigned char dat[])
{
	// Read Back >>
	SendReadBack(FSNETREADBACK_PREPARE,0);
	// Read Back <<

	if(simReady!=YSTRUE)
	{
		sim->EnforceEnvironment();   // 2003/03/26
	}

	if(YSTRUE==airplaneAvailable)
	{
		simReady=YSTRUE;
	}
	else
	{
		fatalError=YSTRUE;
		fatalErrorCode=(int)FsClientRunLoop::CLIENT_FATAL_NO_COMMON_AIRPLANE;
		quit=YSTRUE;
	}

	return YSOK;
}

YSRESULT FsSocketClient::ReceiveLockOn(unsigned char dat[])
{
	int id1OnSvr,id2OnSvr;
	YSBOOL id1isAir,id2isAir;
	const unsigned char *ptr=dat;;
	FsExistence *lockOnBy;

	FsPopInt(ptr);
	id1OnSvr=        FsPopInt(ptr);
	id1isAir=(YSBOOL)FsPopInt(ptr);
	id2OnSvr=        FsPopInt(ptr);
	id2isAir=(YSBOOL)FsPopInt(ptr);

	lockOnBy=FindObject(id1OnSvr,id1isAir);
	if(lockOnBy!=NULL)
	{
		if(id1isAir==YSTRUE && lockOnBy->IsAirplane()==YSTRUE)
		{
			FsAirplane *air;
			FsExistence *target;

			air=(FsAirplane *)lockOnBy;

			target=FindObject(id2OnSvr,id2isAir);
			if(id2isAir==YSTRUE)
			{
				air->Prop().SetAirTargetKey(FsExistence::GetSearchKey(target));
			}
			else
			{
				air->Prop().SetGroundTargetKey(FsExistence::GetSearchKey(target));
			}
		}
		else if(id1isAir!=YSTRUE && lockOnBy->IsAirplane()!=YSTRUE)
		{
			FsGround *gnd;
			FsExistence *target;

			gnd=(FsGround *)lockOnBy;

			target=FindObject(id2OnSvr,id2isAir);
			if(id2isAir==YSTRUE)
			{
				gnd->Prop().SetAirTarget((FsAirplane *)target);
			}
		}
	}
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveMissileLaunch(unsigned char dat[])
{
	int firedBy;
	YSBOOL firedByAirplane;
	int firedAt;
	YSBOOL firedAtAirplane;
	FsWeaponRecord rec;

	// printf("Receive Missile Launch\n");

	if(FsDecodeWeaponRecord(rec,firedBy,firedByAirplane,firedAt,firedAtAirplane,dat,sim)==YSOK)
	{
		FsExistence *owner,*target;
		owner=FindObject(firedBy,firedByAirplane);
		target=FindObject(firedAt,firedAtAirplane);

		rec.firedBy=owner;
		rec.target=target;
		sim->NetWeaponLaunch(rec);

		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketClient::ReceiveAirTurretState(unsigned char dat[],unsigned int packetLength)
{
	const unsigned char *ptr=dat;
	int airId;
	FsAirplane *air;

	FsPopInt(ptr);  // Skip FSCMD_*
	airId=FsPopInt(ptr);
	air=sim->FindAirplane(airId);
	if(air!=NULL)
	{
		air->Prop().DecodeTurretState(dat,packetLength);
	}
	return YSOK;
}

YSRESULT FsSocketClient::ReceiveGndTurretState(unsigned char dat[],unsigned int packetLength)
{
	const unsigned char *ptr=dat;;
	int gndId;
	FsGround *gnd;

	FsPopInt(ptr);  // Skip FSCMD_*
	gndId=FsPopInt(ptr);
	gnd=sim->FindGround(gndId);
	if(gnd!=NULL)
	{
		gnd->Prop().DecodeTurretState(dat,packetLength);
	}
	return YSOK;
}

// YSRESULT FsSocketClient::ReceiveAssignMotionPath(unsigned char dat[])
// {
// 	int idOnSvr,mpathIndex;
// 	YSBOOL isAir;
// 	unsigned char *ptr;
// 	FsExistence *obj;
// 	const YsSceneryPointSet *mpath;
// 	YSBOOL useMotionPathOffset;
// 	float x,y,z;
// 
// 	ptr=dat;
// 
// 	FsPopInt(ptr);  // Skipping command
// 
// 	FsPopInt(ptr);
// 	FsPopShort(ptr);
// 	isAir=(YSBOOL)FsPopShort(ptr);
// 	idOnSvr=      FsPopInt(ptr);
// 
// 	useMotionPathOffset=(YSBOOL)FsPopShort(ptr);
// 	mpathIndex=FsPopShort(ptr);
// 	x=FsPopFloat(ptr);
// 	y=FsPopFloat(ptr);
// 	z=FsPopFloat(ptr);
// 
// 	mpath=sim->SearchMotionPathByTag((char *)ptr);
// 	obj=FindObject(idOnSvr,isAir);
// 	if(obj!=NULL && mpath!=NULL)
// 	{
// 		obj->motionPath=mpath;
// 		obj->useMotionPathOffset=useMotionPathOffset;
// 		obj->motionPathIndex=mpathIndex;
// 		obj->motionPathOffset.Set(x,y,z);
// 
// 		printf("ID %d MPATH %s OFST %d %f %f %f\n",idOnSvr,mpath->GetTag(),useMotionPathOffset,x,y,z);
// 
// 		return YSOK;
// 	}
// 	return YSERR;
// }

YSRESULT FsSocketClient::FlushSendQueue(unsigned int timeout)
{
	YSRESULT res;
	res=Send(nSendQueueFilled,sendQueue,timeout);
	if(res==YSOK)
	{
		nSendQueueFilled=0;
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsSocketClient::SendPacket(YSSIZE_T nDat,unsigned char dat[])
{
	auto total=nDat+4;


	if(SENDQUEUESIZE<total)
	{
		AddMessage("Tried to send huge packet. (larger than queue)");
		return YSERR;
	}


	// if sending AIRPLANESTATE or GROUNDSTATE, delete old ones.
	if(FsGetInt(dat)==FSNETCMD_AIRPLANESTATE ||
	   FsGetInt(dat)==FSNETCMD_GROUNDSTATE)
	{
		FsDeleteOldStatePacket
		    (nSendQueueFilled,sendQueue,FsGetInt(dat),FsGetInt(dat+8));
	}


	if(SENDQUEUESIZE-nSendQueueFilled<total)
	{
		// Buffer overflow?
		FlushSendQueue(FS_NETEMERGENCYTIMEOUT);  // Try flushing send queue

		// If couldn't flush, delete low priority packets.
		if(nSendQueueFilled!=0)
		{
			FsDeleteLowPriorityPacket(nSendQueueFilled,sendQueue);
			printf("Y1\n");
		}

		// Even if it's failed, I abundon old packets. What else can I do?
		if(SENDQUEUESIZE-nSendQueueFilled<total)
		{
			if(FsIsLowPriorityPacket(FsGetInt(dat))==YSTRUE)
			{
				printf("X3\n");
				return YSERR;
			}
			printf("X2\n");
			nSendQueueFilled=0;
		}
	}

	unsigned char nByte[4];
	FsSetUnsignedInt(nByte,(unsigned int)nDat);
	AddToSendQueue(4,nByte);
	AddToSendQueue(nDat,dat);

	return YSOK;

//	unsigned char nByte[4];
//
//	FsSetUnsignedInt(nByte,nDat);
//	if(Send(4,nByte,timeout)==YSOK &&
//	   Send(nDat,dat,timeout)==YSOK)
//	{
//		return YSOK;
//	}
//	else
//	{
//		return YSERR;
//	}
}

void FsSocketClient::AddToSendQueue(YSSIZE_T nByte,unsigned char dat[])
{

	for(decltype(nByte) i=0; i<nByte; i++)
	{
		sendQueue[nSendQueueFilled+i]=dat[i];
	}
	nSendQueueFilled+=nByte;
}

FsExistence *FsSocketClient::FindObject(int idOnSvr,YSBOOL isAirplane)
{
	if(idOnSvr>=0)
	{
		if(isAirplane==YSTRUE)
		{
			FsAirplane *air;
			if((air=sim->FindAirplane(idOnSvr))!=NULL)
			{
				return air;
			}
		}
		else
		{
			FsGround *gnd;
			if((gnd=sim->FindGround(idOnSvr))!=NULL)
			{
				return gnd;
			}
		}
	}
	return NULL;
}

int FsSocketClient::FindIdOnServer(const int idOnCli,YSBOOL )
{
	if(idOnCli>=0)
	{
		return idOnCli;
	}
	return -1;
}

YSRESULT FsSocketClient::EncodeObject(int &isAirplane,int &idOnSvr,FsExistence *obj)
{
	if(sim->FindAirplane(FsExistence::GetSearchKey(obj))!=NULL)
	{
		isAirplane=1;
		idOnSvr=FsExistence::GetSearchKey(obj);
		return YSOK;
	}
	else if(sim->FindGround(FsExistence::GetSearchKey(obj))!=NULL)
	{
		isAirplane=0;
		idOnSvr=FsExistence::GetSearchKey(obj);
		return YSOK;
	}
	else
	{
		isAirplane=1;
		idOnSvr=-1;
		return YSOK;
	}
}

YSRESULT FsSocketClient::DecodeObject(FsExistence **obj,int isAirplane,int idOnSvr)
{
	if(idOnSvr<0)
	{
		(*obj)=NULL;
		return YSOK;
	}
	else if(isAirplane!=0)
	{
		FsAirplane *air;
		if((air=sim->FindAirplane(idOnSvr))!=NULL)
		{
			(*obj)=air;
			return YSOK;
		}
	}
	else
	{
		FsGround *gnd;
		if((gnd=sim->FindGround(idOnSvr))!=NULL)
		{
			(*obj)=gnd;
			return YSOK;
		}
	}

	(*obj)=NULL;
	return YSOK;
}






////////////////////////////////////////////////////////////

FSNET_CONSOLE_COMMAND FsTranslateKeyToCommonCommand(int ky,FsSimulation *sim)
{
	switch(ky)
	{
	default:
		break;
	case FSKEY_SPACE:
	case FSKEY_ENTER:
		return FSNCC_COMMON_PRINTMENU;
	case FSKEY_J:
		return FSNCC_COMMON_JOIN;
	case FSKEY_1:
		return FSNCC_COMMON_IFF1;
	case FSKEY_2:
		return FSNCC_COMMON_IFF2;
	case FSKEY_3:
		return FSNCC_COMMON_IFF3;
	case FSKEY_4:
		return FSNCC_COMMON_IFF4;
	case FSKEY_V:
		return FSNCC_COMMON_OBSERVERMODE;
	case FSKEY_L:
		return FSNCC_COMMON_LISTPLAYER;
	case FSKEY_ESC:
		return FSNCC_COMMON_ESC;
	case FSKEY_A:
		return FSNCC_COMMON_SELECTAIR;
	case FSKEY_S:
		return FSNCC_COMMON_SELECTSTP;
	}

	if(FSKEY_NULL!=ky)
	{
		FSBUTTONFUNCTION buttonFunc=sim->ctlAssign.TranslateKeyStroke(ky);
		if(FSBTF_INFLTMESSAGE==buttonFunc)
		{
			return FSNCC_COMMON_INLINECHAT;
		}
	}

	return FSNCC_COMMON_NULL;
}

FSNET_CONSOLE_COMMAND FsTranslateKeyToServerCommand(int ky)
{
	switch(ky)
	{
	case FSKEY_C:
		return FSNCC_SVR_TOGGLELOCK;
	case FSKEY_K:
		return FSNCC_SVR_TOGGLEJOINLOCK;
	case FSKEY_D:
		return FSNCC_SVR_DISPELUSER;
	case FSKEY_B:
		return FSNCC_SVR_STARTINTERCEPTMISSION;
	case FSKEY_E:
		return FSNCC_SVR_STARTENDURANCEMODE_JET;
	case FSKEY_W:
		return FSNCC_SVR_STARTENDURANCEMODE_WW2;
	case FSKEY_P:
		return FSNCC_SVR_STARTCLOSEAIRSUPPORTMISSION;
	case FSKEY_T:
		return FSNCC_SVR_TERMINATEMISSION;
	case FSKEY_R:
		return FSNCC_SVR_REVIVEGROUND;
	}

	return FSNCC_COMMON_NULL;
}

FSNET_CONSOLE_COMMAND FsTranslateKeyToClientCommand(int ky)
{
	switch(ky)
	{
	case FSKEY_F1:
		return FSNCC_CLI_SVRCOCKPIT;
	case FSKEY_F2:
		return FSNCC_CLI_SVRLEFTVIEW;
	case FSKEY_F3:
		return FSNCC_CLI_SVRRIGHTVIEW;
	case FSKEY_F4:
		return FSNCC_CLI_SVRREARVIEW;
	}

	return FSNCC_COMMON_NULL;
}

////////////////////////////////////////////////////////////

static YSRESULT FsNetworkStandby
    (FsSimulation *,FsNetworkVariable *var,
     FSNET_CONSOLE_COMMAND fsNetConsCmd,int ky,char c,YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my,
     int &escCount,int &choosingMode,
     FsGuiChooseAircraft &chooseAirplane,
     FsGuiChooseField &chooseStartPosition,YSBOOL fieldReady)
{
	switch(choosingMode)
	{
	case 0:
		{
			if(ky!=FSKEY_NULL && ky!=FSKEY_ESC)
			{
				escCount=0;
			}

			switch(fsNetConsCmd)
			{
			default:
				break;
			case FSNCC_COMMON_ESC:
				escCount++;
				if(escCount==1)
				{
					fsConsole.Printf("Press ESC once more to confirm termination.");
				}
				break;
			case FSNCC_COMMON_QUIT:
				escCount=2;
				break;
			case FSNCC_COMMON_SELECTAIR:
				escCount=0;
				if(FsIsConsoleServer()!=YSTRUE)
				{
					chooseAirplane.ResetSelection();
					choosingMode=1;
				}
				break;
			case FSNCC_COMMON_SELECTSTP:
				escCount=0;
				if(FsIsConsoleServer()!=YSTRUE && fieldReady==YSTRUE)
				{
					choosingMode=2;
				}
				break;
			case FSNCC_COMMON_INLINECHAT:
				escCount=0;
				choosingMode=10;  // Typing chat message
				break;
			case FSNCC_COMMON_WHOKILLEDME:
				{
					int i;
					YsString msg;
					fsConsole.Printf("[Loss Report]");
					for(i=0; i<var->killed.GetN(); i++)
					{
						var->killed[i].FormatKilledMessage(msg);
						fsConsole.Printf("%s",msg.Txt());
					}
					fsConsole.Printf("[End of Loss Report]");
				}
				break;
			case FSNCC_COMMON_WHOMHAVEIKILLED:
				{
					int i;
					YsString msg;
					fsConsole.Printf("[Kill Report]");
					for(i=0; i<var->scored.GetN(); i++)
					{
						var->scored[i].FormatScoredMessage(msg);
						fsConsole.Printf("%s",msg.Txt());
					}
					fsConsole.Printf("[End of Kill Report]");
				}
				break;
			}
		}
		break;
	case 1:
		{
			FsGuiDialogItem *itm;

			FsClearScreenAndZBuffer(YsGrayScale(0.25));
			chooseAirplane.Show();
			FsSwapBuffers();

			chooseAirplane.SetMouseState(lb,mb,rb,mx,my);
			chooseAirplane.KeyIn(ky,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
			chooseAirplane.CharIn(c);

			itm=chooseAirplane.GetClickedItem();
			if(itm==chooseAirplane.okBtn)
			{
				chooseAirplane.CaptureSelection();
				choosingMode=0;

			}
			else if(itm==chooseAirplane.cancelBtn)
			{
				chooseAirplane.ResetSelection();
				choosingMode=0;

			}

			// chooseAirplane.Rotate(YsDegToRad(4.0));
		}
		break;
	case 2:
		{
			FsGuiDialogItem *itm;

			FsClearScreenAndZBuffer(YsBlack());
			chooseStartPosition.Show();
			FsSwapBuffers();

			chooseStartPosition.SetMouseState(lb,mb,rb,mx,my);
			chooseStartPosition.KeyIn(ky,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
			chooseStartPosition.CharIn(c);

			itm=chooseStartPosition.GetClickedItem();
			if(itm==chooseStartPosition.okBtn)
			{
				chooseStartPosition.CaptureSelection();
				choosingMode=0;
			}
			else if(itm==chooseStartPosition.cancelBtn)
			{
				chooseStartPosition.ResetSelection();
				choosingMode=0;
			}
		}
		break;
	case 10:
		{
			if(ky==FSKEY_ESC)
			{
				choosingMode=0;
				fsConsole.wInputLine=L"";
			}
			else
			{
				if(fsConsole.wInputLine.Strlen()==0)
				{
					fsConsole.wInputLine.Set(L">");
				}

				if(isprint(c))
				{
					wchar_t s[2];
					s[0]=c;
					s[1]=0;
					fsConsole.wInputLine.Append(s);
				}

				if(ky==FSKEY_BS && fsConsole.wInputLine.Strlen()>1)
				{
					fsConsole.wInputLine.BackSpace();
				}

				fsConsole.Show();
			}
		}
	}

	if(choosingMode==0 || choosingMode==10)
	{
		fsConsole.SetAutoFlush(YSTRUE);
	}
	else
	{
		fsConsole.SetAutoFlush(YSFALSE);
	}

	return YSOK;
}

////////////////////////////////////////////////////////////

YSBOOL FsSimulation::NetActivity(void)
{
	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		// Even the airplane is remote, netAlive must be respected because
		// if a delayed state packet arrives after the airplane crashes into
		// the ground in the server, the airplane must be revived.  Hence,
		// if netAlive is YSTRUE, the airplane must be considered alive.
		// A remote airplane's netResendRemoveCount never becomes non zero.
		if(air->netAlive==YSTRUE || air->IsAlive()==YSTRUE)
		{
			return YSTRUE;  // Alive means there's actitivy.
		}
	}

	if(bulletHolder.FindNextActiveWeapon(NULL)!=NULL)
	{
		return YSTRUE;
	}

	return YSFALSE;
}

void FsSimulation::NetFreeMemoryWhenPossible
	(double &netNoActivityTime,const double &passedTime,FsSocketServer *server)
{
	FsPluginCallNetFreeMemory(currentTime,this);



	double prevNoActivityTime;
	prevNoActivityTime=netNoActivityTime;
	if(NetActivity()==YSTRUE)
	{
		netNoActivityTime=0.0;
	}
	else
	{
		netNoActivityTime+=passedTime;
	}

	if((int)(prevNoActivityTime/20.0)!=(int)(netNoActivityTime/20.0))
	{
		fsConsole.Printf("Idle %d seconds.",(int)netNoActivityTime);
	}

	if(netNoActivityTime>=60.0)  // 1 minutes of no activity
	{
		FsAirplane *air;

		if(FsVerboseMode==YSTRUE)
		{
			air=NULL;
			while((air=FindNextAirplane(air))!=NULL)
			{
				printf("[%d] %s ",FsExistence::GetSearchKey(air),air->GetPosition().Txt());
				printf("(%.2lf %.2lf %.2lf)",
				    YsRadToDeg(air->GetAttitude().h()),
				    YsRadToDeg(air->GetAttitude().p()),
				    YsRadToDeg(air->GetAttitude().b()));
				if(air->netType==FSNET_LOCAL)
				{
					printf("LOCAL %d %d\n",air->netAlive,air->IsAlive());
				}
				else if(air->netType==FSNET_REMOTE)
				{
					printf("REMOTE %d\n",air->IsAlive());
				}
				else
				{
					printf("UNKNOWN\n");
				}
			}
		}

	#ifdef CRASHINVESTIGATION
		printf("Cleaning Up Airplanes.\n");
		FsLogOutput(FSSVRLOGFILE,"Cleaning Up Airplanes and Airplane Templates");
	#endif

		fsConsole.Printf("");
		fsConsole.Printf("Clean Up Airplanes....");
		if(server!=NULL)
		{
			server->ClearUserAirplaneForMemoryCleanUpPurpose();
		}
		while((air=FindNextAirplane(NULL))!=NULL)
		{
			DeleteAirplane(air);
		}
		SetPlayerAirplane(NULL);
		FsAirplaneAllocator.CollectGarbage();  // Enabled again 2004/05/21
		fsConsole.Printf("Done.");
		fsConsole.Printf("");

		world->UnprepareAirplaneTemplate();  // 2005/03/16 Testing Aggressive Cleaning Up


		// Added 2009/06/02 >>
		FsGround *gnd;
		gnd=NULL;
		while(NULL!=(gnd=FindNextGround(gnd)))
		{
			FsAircraftCarrierProperty *carrier;
			carrier=gnd->Prop().GetAircraftCarrierProperty();
			if(carrier!=NULL)
			{
				carrier->ForceClearLoadedObject();
			}
		}
		// Added 2009/06/02 <<


		if(bulletHolder.killCredit!=NULL) // 2004/09/06
		{
			bulletHolder.killCredit->DeleteList();
			bulletHolder.killCredit=NULL;
		}


		mainWindowViewmode=FSCOCKPITVIEW;  // 2004/09/04
		focusAir=NULL;  // 2004/09/04
		focusAir2=NULL; // 2004/09/04

		netNoActivityTime=0.0;
	}
}

////////////////////////////////////////////////////////////

class FsGuiServerDialog : public FsGuiDialog
{
public:
	int prevWid,prevHei;

	FsSimulation *sim;
	FsSocketServer *svr;

	FsGuiStatic *ipAddrTxt;
	FsGuiStatic *portTxt;

	FsGuiButton *iffBtn[4];
	FsGuiButton *disableChatBtn;
	FsGuiButton *svrLockBtn;
	FsGuiButton *joinLockBtn;
	FsGuiButton *selectAirBtn;
	FsGuiButton *selectStpBtn;
	FsGuiButton *joinBtn;
	FsGuiButton *observerModeBtn;
	FsGuiButton *dispelUserBtn;
	FsGuiButton *listUserBtn;
	FsGuiButton *startEnduranceModeJetBtn;
	FsGuiButton *startEnduranceModeWw2Btn;
	FsGuiButton *startInterceptMissionBtn;
	FsGuiButton *startCloseAirSupportBtn;

	FsGuiButton *whoKilledMeBtn,*whomHaveIKilledBtn;

	FsGuiStatic *multiIpWarningTxt;

	FsGuiButton *endServerBtn,*confirmEndServerBtn;
	FsGuiButton *reviveGroundBtn,*confirmReviveGroundBtn;

	FsGuiTextBox *chatTxt;
	FsGuiButton *sendChatMessage;

	void MakeDialog(FsSimulation *sim,FsSocketServer *svr);
	void AutoArrangeDialog(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnTextBoxSpecialKey(FsGuiTextBox *txt,int fskey);
};

void FsGuiServerDialog::MakeDialog(FsSimulation *sim,FsSocketServer *server)
{
	this->sim=sim;
	this->svr=server;

	prevWid=0;
	prevHei=0;

	AddStaticText(0,FSKEY_NULL,"IP Address:",YSTRUE);
	ipAddrTxt=AddStaticText(0,FSKEY_NULL,"255.255.255.255",20,1,YSFALSE);

	char hostname[1024];
	if(gethostname(hostname,1024)==0)
	{
		int i;
		struct hostent *table;
		YsString hostaddr;

		table=gethostbyname(hostname);
		if(table!=NULL && NULL!=table->h_addr_list[0])
		{
			const unsigned char *addr=(unsigned char *)table->h_addr_list[0];
			for(i=0; i<table->h_length; i++)
			{
				unsigned a;
				YsString num;
				a=addr[i];
				num.Printf("%d",a);
				hostaddr.Append(num);
				if(i<table->h_length-1)
				{
					hostaddr.Append(".");
				}
			}

			ipAddrTxt->SetText(hostaddr);
		}
	}




	AddStaticText(0,FSKEY_NULL,"Port:",YSTRUE);
	portTxt=AddStaticText(0,FSKEY_NULL,"9999",5,1,YSFALSE);
	{
		YsString str;
		str.Printf("%d",server->netcfg->portNumber);
		portTxt->SetText(str);
	}


	AddStaticText(0,FSKEY_NULL,"IFF",YSTRUE);
	iffBtn[0]=AddTextButton(MkId("iff1"),FSKEY_NULL,FSGUI_RADIOBUTTON,"1",YSFALSE);
	iffBtn[1]=AddTextButton(MkId("iff2"),FSKEY_NULL,FSGUI_RADIOBUTTON,"2",YSFALSE);
	iffBtn[2]=AddTextButton(MkId("iff3"),FSKEY_NULL,FSGUI_RADIOBUTTON,"3",YSFALSE);
	iffBtn[3]=AddTextButton(MkId("iff4"),FSKEY_NULL,FSGUI_RADIOBUTTON,"4",YSFALSE);
	SetRadioButtonGroup(4,iffBtn);

	disableChatBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCONSOLE_DISABLECHAT,YSTRUE);
	svrLockBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCONSOLE_LOCKSERVER,YSFALSE);
	joinLockBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_NETCONSOLE_JOINLOCK,YSFALSE);

	selectAirBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_SELECTAIR,YSTRUE);
	selectStpBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_SELECTSTP,YSFALSE);
	joinBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_JOIN,YSFALSE);
	observerModeBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_OBSERVERMODE,YSFALSE);

	dispelUserBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_DISPELUSER,YSTRUE);
	listUserBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_LISTUSER,YSFALSE);

	whoKilledMeBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_WHOKILLEDME,YSTRUE);
	whomHaveIKilledBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_WHOMHAVEIKILLED,YSFALSE);

	startEnduranceModeJetBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_ENDURANCE_JET,YSTRUE);
	startEnduranceModeWw2Btn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_ENDURANCE_WW2,YSFALSE);

	startInterceptMissionBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_INTERCEPT,YSTRUE);
	startCloseAirSupportBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_CLOSEAIRSUPPORT,YSFALSE);

	multiIpWarningTxt=AddStaticText(0,FSKEY_NULL,"",16,2,YSTRUE);

	endServerBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_ENDSERVER,YSTRUE);
	confirmEndServerBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_CONFIRMENDSERVER,YSFALSE);
	confirmEndServerBtn->Disable();

	reviveGroundBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_REVIVEGROUND,YSTRUE);
	confirmReviveGroundBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_CONFIRMREVIVEGROUND,YSFALSE);
	confirmReviveGroundBtn->Disable();

	AddStaticText(0,FSKEY_NULL,FSGUI_NETCONSOLE_CHAT,YSTRUE);
	chatTxt=AddTextBox(0,FSKEY_NULL,"","",32,YSFALSE);
	chatTxt->SetLengthLimit(140);
	sendChatMessage=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_SEND,YSFALSE);

	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiServerDialog::AutoArrangeDialog(void)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	if(prevWid!=wid)
	{
		Move(wid-dlgWid,0);
		prevWid=wid;
	}
}

void FsGuiServerDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn!=endServerBtn && btn!=confirmEndServerBtn)
	{
		confirmEndServerBtn->Disable();
	}
	if(btn!=reviveGroundBtn && btn!=confirmReviveGroundBtn)
	{
		confirmReviveGroundBtn->Disable();
	}

	YsString str;
	chatTxt->GetText(str);
	if(btn==sendChatMessage && 0<str.Strlen() && YSTRUE!=disableChatBtn->GetCheck())
	{
		sim->SendNetChatMessage(str);
		chatTxt->SetText("");
	}
	else if(btn==iffBtn[0])
	{
		svr->commandQueue.push(FSNCC_COMMON_IFF1);
	}
	else if(btn==iffBtn[1])
	{
		svr->commandQueue.push(FSNCC_COMMON_IFF2);
	}
	else if(btn==iffBtn[2])
	{
		svr->commandQueue.push(FSNCC_COMMON_IFF3);
	}
	else if(btn==iffBtn[3])
	{
		svr->commandQueue.push(FSNCC_COMMON_IFF4);
	}
	else if(btn==disableChatBtn)
	{
		svr->netcfg->serverDisableChat=btn->GetCheck();
	}
	else if(btn==svrLockBtn)
	{
		svr->locked=btn->GetCheck();
	}
	else if(btn==joinLockBtn)
	{
		svr->joinLocked=btn->GetCheck();
	}
	else if(btn==selectAirBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_SELECTAIR);
	}
	else if(btn==selectStpBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_SELECTSTP);
	}
	else if(btn==joinBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_JOIN);
	}
	else if(btn==observerModeBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_OBSERVERMODE);
	}
	else if(btn==dispelUserBtn)
	{
		svr->commandQueue.push(FSNCC_SVR_DISPELUSER);
	}
	else if(btn==listUserBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_LISTPLAYER);
	}
	else if(btn==startEnduranceModeJetBtn)
	{
		svr->commandQueue.push(FSNCC_SVR_STARTENDURANCEMODE_JET);
	}
	else if(btn==startEnduranceModeWw2Btn)
	{
		svr->commandQueue.push(FSNCC_SVR_STARTENDURANCEMODE_WW2);
	}
	else if(btn==startInterceptMissionBtn)
	{
		svr->commandQueue.push(FSNCC_SVR_STARTINTERCEPTMISSION);
	}
	else if(btn==startCloseAirSupportBtn)
	{
		svr->commandQueue.push(FSNCC_SVR_STARTCLOSEAIRSUPPORTMISSION);
	}
	else if(btn==endServerBtn)
	{
		confirmEndServerBtn->Enable();
	}
	else if(btn==confirmEndServerBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_QUIT);
	}
	else if(btn==reviveGroundBtn)
	{
		confirmReviveGroundBtn->Enable();
	}
	else if(btn==confirmReviveGroundBtn)
	{
		svr->commandQueue.push(FSNCC_SVR_REVIVEGROUND);
		confirmReviveGroundBtn->Disable();
	}
	else if(btn==whoKilledMeBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_WHOKILLEDME);
	}
	else if(btn==whomHaveIKilledBtn)
	{
		svr->commandQueue.push(FSNCC_COMMON_WHOMHAVEIKILLED);
	}
}

void FsGuiServerDialog::OnTextBoxSpecialKey(FsGuiTextBox *txt,int fskey)
{
	YsString str;
	switch(fskey)
	{
	case FSKEY_ENTER:
		chatTxt->GetText(str);
		if(txt==chatTxt && 0<str.Strlen() && YSTRUE!=disableChatBtn->GetCheck())
		{
			sim->SendNetChatMessage(str);
			chatTxt->SetText("");
		}
		break;
	case FSKEY_ESC:
		svr->commandQueue.push(FSNCC_COMMON_ESC);
		break;
	}
}

FsNetworkVariable::FsNetworkVariable()
{
	Initialize();
}

void FsNetworkVariable::Initialize(void)
{
	quit=YSFALSE;
	escCount=0;
	iff=FS_IFF0;
	username.Set("");

	std::queue <FSNET_CONSOLE_COMMAND> emptyQueue;
	commandQueue.swap(emptyQueue);

	sim=NULL;

	killed.Set(0,NULL);
	scored.Set(0,NULL);
}


FsServerVariable::FsServerVariable(const char _name[],FsSimulation *_sim,FsNetConfig *_netcfg)
{
	FsNetworkVariable::Initialize();


	cfg=new FsFlightConfig;
	imInfo=new FsInterceptMissionInfo;


	sim=_sim;
	netcfg=_netcfg;

	quit=YSFALSE;
	escCount=0;

	username.Set(_name);

	cfg->Load(FsGetConfigFile());
	imInfo->Load(FsGetNetInterceptMissionConfigFile());

	iff=(FSIFF)netcfg->defIFFWhenServer;


	nMsg=0;

	user=new FsNetworkUser[FS_MAX_NUM_USER];

	int i;
	for(i=0; i<FS_MAX_NUM_USER; i++)
	{
		user[i].state=FSUSERSTATE_NOTCONNECTED; // 2007/01/05 loggedOn=YSFALSE;
		user[i].username.Set("");
		user[i].air=NULL;
		user[i].nComBuf=0;
	}

	receivedKillServer=YSFALSE;

	locked=YSFALSE;
	joinLocked=YSFALSE;

	nBroadcastGndCount=0;

}

FsServerVariable::~FsServerVariable()
{
	delete cfg;
	delete imInfo;
	delete [] user;
}

YSRESULT FsServerVariable::LoadWelcomeMessage(const wchar_t fn[])
{

	YsString str;

	welcomeMessage.Set("");

	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		while(str.Fgets(fp))
		{
			if(welcomeMessage.Strlen()>0)
			{
				welcomeMessage.Append("\n");
			}
			welcomeMessage.Append(str);

			if(welcomeMessage.Strlen()>256)
			{
				break;
			}
		}

		if(FsVerboseMode==YSTRUE)
		{
			printf("Loaded Welcome Message %d\n",(int)welcomeMessage.Strlen());
		}

		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

static void FsPrintServerMenu(
    const FsSimulation *sim,
    FsGuiChooseAircraft &airplane,FsGuiChooseField &startPosition,FSIFF iff,YSBOOL locked,YSBOOL joinLocked,int port)
{
	const int chatKey=sim->ctlAssign.FindKeyByFunction(FSBTF_INFLTMESSAGE);
	const char *chatKeyTxt=(FsGetKeyString(chatKey)!=NULL ? FsGetKeyString(chatKey) : "NONE");
	char txt[256];
	sprintf(txt,"[%s]...Send Message",chatKeyTxt);

	if(FsIsConsoleServer()!=YSTRUE)
	{
		fsConsole.Printf("------");
		fsConsole.Printf("Airplane : %s          IFF : %d",(const char *)airplane.selAir,iff+1);
		fsConsole.Printf("Starting Position : %s",(const char *)startPosition.selStp);
		fsConsole.Printf("[A]...Choose Airplane            [S].....Choose Start Position");
		fsConsole.Printf("[J]...Join Flight                [ESC]...Terminate Server");
		fsConsole.Printf("[L]...List Players               [D]...Dispell User");
		fsConsole.Printf("[C]...Lock Server                [K]...Join-Lock");
		fsConsole.Printf("[1][2][3][4]...Choose IFF        [V].....Observer mode");
		fsConsole.Printf("[E]...Start Endurance Mode (Jet) [W].....Start Endurance Mode (WwII)");
		fsConsole.Printf("[B].....Start Intercept Mission");
		fsConsole.Printf("[P]...Start Close Air Support Mission");
		fsConsole.Printf("[T]...Terminate Endurance Mode/Intercept Mission");
		fsConsole.Printf("[R]...Revive Ground Objects");
		fsConsole.Printf("%s",txt);
		fsConsole.Printf("The Server is %s",(locked==YSTRUE ? "LOCKED" : "not locked"));
	}
	else
	{
		fsConsole.Printf("------");
		fsConsole.Printf("[ESC]...Terminate Server");
		fsConsole.Printf("[L]...List Players               [D]...Dispell User");
		fsConsole.Printf("[C]...Lock Server                [K]...Join-Lock");
		fsConsole.Printf("[E]...Start Endurance Mode (Jet) [W].....Start Endurance Mode (WwII)");
		fsConsole.Printf("[B].....Start Intercept Mission");
		fsConsole.Printf("[P]...Start Close Air Support Mission");
		fsConsole.Printf("[T]...Terminate Endurance Mode/Intercept Mission");
		fsConsole.Printf("[R]...Revive Ground Objects");
		fsConsole.Printf("%s",txt);
		fsConsole.Printf("The Server is %s",(locked==YSTRUE ? "LOCKED" : "not locked"));
	}

	if(joinLocked!=YSTRUE)
	{
		fsConsole.Printf("The Server is accepting join requests.");
	}
	else // if(joinLocked==YSTRUE)
	{
		fsConsole.Printf("The Server is not accepting join requests. (K-key to change)");
	}
	fsConsole.Printf("");

	char hostname[1024],hostaddr[1024];
	if(gethostname(hostname,1024)==0)
	{
		int i;
		struct hostent *table;

		table=gethostbyname(hostname);
		if(table!=NULL)
		{
			int n;
			for(n=0; table->h_addr_list[n]!=NULL; n++)
			{
				unsigned char *addr;
				strcpy(hostaddr,"SERVER ADDRESS: ");

				addr=(unsigned char *)table->h_addr_list[n];
				for(i=0; i<table->h_length; i++)
				{
					unsigned a;
					char num[256];
					a=addr[i];
					sprintf(num,"%d",a);
					strcat(hostaddr,num);
					if(i<table->h_length-1)
					{
						strcat(hostaddr,".");
					}
				}
				fsConsole.Printf("%s",hostaddr);
			}
			if(n>1)
			{
				fsConsole.Printf("More than one IP addresses are assigned to this server.");
				fsConsole.Printf("Some of them may not work.");
			}
		}
		else
		{
			fsConsole.Printf("Cannot obtain host address. Error in gethostbyname()\n");
		}

		fsConsole.Printf("PORT=%d",port);
	}
	else
	{
		fsConsole.Printf("Cannot obtain host address. Error in gethostname()\n");
	}
}

YSRESULT FsSimulation::ServerJoin(
    class FsSocketServer &svr,FsGuiChooseAircraft &chooseAirplane,const char startPos[])
{
	FsStartPosInfo info;
	world->GetStartPositionInfo(info,startPos);

	if(info.onCarrier==YSTRUE)
	{
		FsGround *gnd;
		gnd=FindGroundByTag(info.carrierTag);
		if(gnd==NULL || gnd->IsAlive()!=YSTRUE)
		{
			fsConsole.Printf("** The aircraft carrier you chose has been destroyed. **");
			return YSERR;
		}
	}

	if(info.iff!=FsStartPosInfo::IFF_ANY && info.iff!=svr.iff)
	{
		fsConsole.Printf("** You need to select IFF %d to start from %s **",info.iff+1,startPos);
		return YSOK;
	}


	int idOnSvr;
	FsAirplane *air;
	char airCmd[256];

	air=world->AddAirplane(chooseAirplane.selAir,YSTRUE);
	if(air==NULL)
	{
		fsConsole.Printf("** Cannot add an airplane **");
		return YSERR;
	}

	air->iff=svr.iff;
	air->name.Set(svr.username);

	for(int smkIdx=0; smkIdx<air->Prop().GetNumSmokeGenerator(); ++smkIdx)
	{
		auto col=chooseAirplane.SmokeColor(smkIdx);
		sprintf(airCmd,"SMOKECOL %d %d %d %d",smkIdx,col.Ri(),col.Gi(),col.Bi());
		air->SendCommand(airCmd);
	}

	if(svr.netcfg->sendJoinLeaveMessage==YSTRUE)
	{
		YsString msg;
		MakeTakeOffMessage(msg,air->name,air->Prop().GetIdentifier());
		svr.BroadcastChatTextMessage(msg);
	}

	idOnSvr=FsExistence::GetSearchKey(air);

	if(svr.netcfg->recordWhenServerMode==YSTRUE)
	{
		RecordPlayerChange(air);
	}

	if(svr.netcfg->useMissile!=YSTRUE)
	{
		air->Prop().UnloadGuidedAAM();
		air->Prop().UnloadGuidedAGM();
	}
	if(svr.netcfg->useUnguidedWeapon!=YSTRUE)
	{
		air->Prop().UnloadUnguidedWeapon();
	}
	if(svr.netcfg->disableRadarGunSight==YSTRUE)
	{
		air->SendCommand("GUNSIGHT FALSE");
	}

	world->SettleAirplane(*air,startPos);
	int i;
	for(i=0; startPos[i]!=0; i++)
	{
		if(strncmp(startPos+i,"CARRIER",7)==0)
		{
			air->SendCommand("INITSPED 0kt");
			break;
		}
	}

	svr.BroadcastAddAirplane(air,FSNET_REMOTE);

	userInput.Initialize();

	air->Prop().ReadBackControl(userInput);
	Gear=air->Prop().GetLandingGear();
	pGear=air->Prop().GetLandingGear();
	ppGear=air->Prop().GetLandingGear();
	userInput.hasAb=air->Prop().GetHasAfterburner();

	YsArray <int,64> weaponConfig;
	air->AutoSendCommand(chooseAirplane.selWeaponConfig.GetN(),chooseAirplane.selWeaponConfig);
	air->Prop().GetWeaponConfig(weaponConfig);
	svr.BroadcastWeaponConfig(idOnSvr,weaponConfig.GetN(),weaponConfig);

	if(svr.netcfg->useMissile!=YSTRUE)
	{
		air->Prop().UnloadGuidedAAM();
		air->Prop().UnloadGuidedAGM();
	}
	if(svr.netcfg->useUnguidedWeapon!=YSTRUE)
	{
		air->Prop().UnloadUnguidedWeapon();
	}


	if(chooseAirplane.SmokeLoaded()==YSTRUE)
	{
		air->SendCommand("SMOKEOIL 100.0kg");
		svr.BroadcastAirCmd(idOnSvr,"SMOKEOIL 100.0kg");
	}
	else
	{
		air->SendCommand("SMOKEOIL 0.0kg");
		svr.BroadcastAirCmd(idOnSvr,"SMOKEOIL 0.0kg");
	}

	char cmd[256];
	sprintf(cmd,"INITFUEL %d%%",chooseAirplane.selFuel);
	air->SendCommand(cmd);
	svr.BroadcastAirCmd(idOnSvr,cmd);

	return YSOK;
}

YSRESULT FsSimulation::ServerState_StandBy(
    const double &passedTime,
    class FsSocketServer &svr,
    FsGuiChooseAircraft &chooseAirplane,
    FsGuiChooseField &chooseStartPosition,FsChoose &dispellUser,class FsGuiServerDialog &svrDlg)
{
	int i;

	escKeyCount=0;  // Don't confuse with escCount

	for(i=0; i<FsMaxNumSubWindow; i++)
	{
		if(FsIsSubWindowOpen(i)==YSTRUE)
		{
			FsCloseSubWindow(i);
			FsSelectMainWindow();
		}
	}


	FsPollDevice();

	int ky=FsInkey();
	int c=FsInkeyChar();


#ifdef YSFS_TESTVERSION
//	if(FSKEY_UP==ky)
//	{
//		svr.BroadcastSkyColor(YsGreen());
//		svr.BroadcastGroundColor(YsCyan());
//		svr.BroadcastFogColor(YsBlack());
//	}
//	if(FSKEY_DOWN==ky)
//	{
//		svr.BroadcastForceJoin();
//	}
#endif


	int mx,my;
	YSBOOL lb,mb,rb;
	FsGetMouseEvent(lb,mb,rb,mx,my);

	if(0==svr.choosingMode)
	{
		svrDlg.AutoArrangeDialog();
		svrDlg.SetMouseState(lb,mb,rb,mx,my);
		svrDlg.KeyIn(ky,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
		svrDlg.CharIn(c);

		if(svrDlg.chatTxt!=svrDlg.GetFocus())
		{
			auto cmd=FsTranslateKeyToCommonCommand(ky,this);
			if(FSNCC_COMMON_NULL==cmd)
			{
				cmd=FsTranslateKeyToServerCommand(ky);
			}
			if(FSNCC_COMMON_NULL!=cmd)
			{
				svr.commandQueue.push(cmd);
			}
		}
	}

	FSNET_CONSOLE_COMMAND consCmd=FSNCC_COMMON_NULL;
	if(true!=svr.commandQueue.empty())
	{
		consCmd=svr.commandQueue.front();
		svr.commandQueue.pop();
	}



	int prevChoosingMode=svr.choosingMode;

	FsNetworkStandby(this,&svr,consCmd,ky,c,lb,mb,rb,mx,my,svr.escCount,svr.choosingMode,chooseAirplane,chooseStartPosition,YSTRUE);

	if((prevChoosingMode!=0 && prevChoosingMode!=10) && (svr.choosingMode==0 || svr.choosingMode==10))
	{
		fsConsole.Printf("");
		FsPrintServerMenu(
		    this,chooseAirplane,chooseStartPosition,svr.iff,svr.locked,svr.joinLocked,svr.netcfg->portNumber);
	}
	else if((prevChoosingMode==0 || prevChoosingMode==10) && (svr.choosingMode!=0 && svr.choosingMode!=10))
	{
	}

	if(svr.choosingMode==0)
	{
		switch(consCmd)
		{
		default:
			break;
		case FSNCC_COMMON_PRINTMENU:
			FsPrintServerMenu(
			    this,chooseAirplane,chooseStartPosition,svr.iff,svr.locked,svr.joinLocked,svr.netcfg->portNumber);
			break;
		case FSNCC_COMMON_JOIN:
			if(FsIsConsoleServer()!=YSTRUE &&
			   ServerJoin(svr,chooseAirplane,chooseStartPosition.selStp)==YSOK)
			{
				ClearUserInterface();    // 2009/03/29
				svr.serverState=1;
				endTime=0.0;
				terminate=YSFALSE;

				std::queue <FSNET_CONSOLE_COMMAND> emptyQueue;
				svr.commandQueue.swap(emptyQueue);
			}
			break;
		case FSNCC_SVR_TOGGLELOCK:
			YsFlip(svr.locked);

			FsPrintServerMenu(
			    this,chooseAirplane,chooseStartPosition,svr.iff,svr.locked,svr.joinLocked,svr.netcfg->portNumber);
			svrDlg.svrLockBtn->SetCheck(svr.locked);
			break;
		case FSNCC_SVR_TOGGLEJOINLOCK:
			YsFlip(svr.joinLocked);

			FsPrintServerMenu(
			    this,chooseAirplane,chooseStartPosition,svr.iff,svr.locked,svr.joinLocked,svr.netcfg->portNumber);
			svrDlg.joinLockBtn->SetCheck(svr.joinLocked);
			break;
		case FSNCC_SVR_DISPELUSER:
			{
				int id;
				dispellUser.Clear();
				for(id=0; id<svr.GetMaxNumUser(); id++)
				{
					if(svr.IsUserLoggedOn(id)==YSTRUE)
					{
						char buf[256];
						YsString ipAddrString;
						sprintf(buf,"%03d %s ADDR=%s",
						    id,
						    svr.GetUserName(id),
						    svr.GetUserIpAddressString(id,ipAddrString));
						dispellUser.AddChoice(buf);
					}
				}

				if(YSTRUE==FsIsConsoleServer())
				{
					dispellUser.PrintInConsoleMode();
				}

				svr.serverState=2;
			}
			break;
		case FSNCC_SVR_STARTINTERCEPTMISSION:
			if(svr.baseDefenseModeRemainingTime<YsTolerance && 
			   svr.enduranceModeRemainingTime<YsTolerance &&
			   svr.closeAirSupportMissionRemainingTime<YsTolerance)
			{
				svr.baseDefenseModeRemainingTime=15.0*60.0;
				fsConsole.Printf("**** INTERCEPT MISSION START ****");
				svr.BroadcastTextMessage("**** INTERCEPT MISSION START ****");
			}
			break;
		case FSNCC_SVR_STARTENDURANCEMODE_JET:
			if(svr.baseDefenseModeRemainingTime<YsTolerance &&
			   svr.enduranceModeRemainingTime<YsTolerance &&
			   svr.closeAirSupportMissionRemainingTime<YsTolerance)
			{
				svr.enduranceModeRemainingTime=15.0*60.0;
				svr.enduranceModeJet=YSTRUE;
				svr.enduranceModeWw2=YSFALSE;

				svr.gLimit=4.0;
				svr.nEnduranceModeEnemyMax=0;
				fsConsole.Printf("**** ENDURANCE MODE START (Jet) ****");
				svr.BroadcastTextMessage("**** ENDURANCE MODE START (Jet) ****");
			}
			break;
		case FSNCC_SVR_STARTENDURANCEMODE_WW2:
			if(svr.baseDefenseModeRemainingTime<YsTolerance &&
			   svr.enduranceModeRemainingTime<YsTolerance &&
			   svr.closeAirSupportMissionRemainingTime<YsTolerance)
			{
				svr.enduranceModeRemainingTime=15.0*60.0;
				svr.enduranceModeJet=YSFALSE;
				svr.enduranceModeWw2=YSTRUE;

				svr.gLimit=4.0;
				svr.nEnduranceModeEnemyMax=0;
				fsConsole.Printf("**** ENDURANCE MODE START (Jet) ****");
				svr.BroadcastTextMessage("**** ENDURANCE MODE START (Jet) ****");
			}
			break;
		case FSNCC_SVR_STARTCLOSEAIRSUPPORTMISSION:
			if(svr.baseDefenseModeRemainingTime<YsTolerance && 
			   svr.enduranceModeRemainingTime<YsTolerance &&
			   svr.closeAirSupportMissionRemainingTime<YsTolerance)
			{
				svr.closeAirSupportMissionRemainingTime=15.0*60.0;
				fsConsole.Printf("**** CLOSE AIR SUPPORT MISSION START ****");
				svr.BroadcastTextMessage("**** CLOSE AIR SUPPORT MISSION START ****");
			}
			break;
		case FSNCC_SVR_TERMINATEMISSION:
			svr.enduranceModeRemainingTime=0.0;
			svr.baseDefenseModeRemainingTime=0.0;
			svr.closeAirSupportMissionRemainingTime=0.0;
			fsConsole.Printf("INTERCEPT MISSION/ENDURANCE MODE/CLOSE AIR SUPPORT MISSION is terminated.");
			svr.BroadcastTextMessage("INTERCEPT MISSION/ENDURANCE MODE/CLOSE AIR SUPPORT MISSION is terminated.");

			DestroyAutoGeneratedAirAndGnd();

			break;
		case FSNCC_SVR_REVIVEGROUND:
			ReviveGround();
			svr.BroadcastReviveGround();
			break;
		case FSNCC_COMMON_OBSERVERMODE:
			if(FsIsConsoleServer()!=YSTRUE)
			{
				svr.serverState=3; // Observer mode
				escKeyCount=0;
				terminate=YSFALSE;
				SetPlayerAirplane(NULL);
				mainWindowViewmode=FSGHOSTVIEW;

				std::queue <FSNET_CONSOLE_COMMAND> emptyQueue;
				svr.commandQueue.swap(emptyQueue);
			}
			break;
		case FSNCC_COMMON_IFF1:
		case FSNCC_COMMON_IFF2:
		case FSNCC_COMMON_IFF3:
		case FSNCC_COMMON_IFF4:
			if(FsIsConsoleServer()!=YSTRUE)
			{
				svr.iff=(FSIFF)(consCmd-FSNCC_COMMON_IFF1);

				FsPrintServerMenu(
				    this,chooseAirplane,chooseStartPosition,svr.iff,svr.locked,svr.joinLocked,svr.netcfg->portNumber);

				svrDlg.iffBtn[0]->SetCheck(YSFALSE);
				svrDlg.iffBtn[1]->SetCheck(YSFALSE);
				svrDlg.iffBtn[2]->SetCheck(YSFALSE);
				svrDlg.iffBtn[3]->SetCheck(YSFALSE);
				svrDlg.iffBtn[svr.iff]->SetCheck(YSTRUE);
			}
			break;
		case FSNCC_COMMON_LISTPLAYER:
			svr.PrintPlayerList();
			break;
		}
	}
	else if(svr.choosingMode==10)
	{
		if(ky==FSKEY_ENTER)
		{
			YsString msgStr;

			msgStr.Set("(");
			msgStr.Append(svr.username);
			msgStr.Append(")");
			msgStr.Append(fsConsole.wInputLine.GetUTF8String().Txt()+1);

			fsConsole.Printf("%s",(const char *)msgStr);
			svr.BroadcastChatTextMessage(msgStr);

			svr.choosingMode=0;
			fsConsole.wInputLine=L"";
		}
	}

	if(svr.escCount==2 || svr.ReceivedKillServer()==YSTRUE)
	{
		svr.quit=YSTRUE;
	}

	for(i=0; i<svr.GetNumMessage(); i++)
	{
		fsConsole.Printf("%s",svr.GetMessage(i));
	}
	svr.ClearMessage();

	svr.nextConsoleUpdateTime-=passedTime;

	FsSleep(25);

	return YSOK;
}

YSRESULT FsSimulation::ServerState_Flying(const double &,class FsSocketServer &svr)
{
	if((endTime>YsTolerance && currentTime>endTime) || terminate==YSTRUE)
	{
		FsAirplane *air;
		air=GetPlayerAirplane();

		if(air!=NULL)
		{
			int airId;
			airId=FsExistence::GetSearchKey(air);
			svr.BroadcastRemoveAirplane(airId,YSFALSE);

			if(svr.netcfg->sendJoinLeaveMessage==YSTRUE)
			{
				YsString msg;
				MakeUnjoinMessage(msg,air->name);
				svr.AddMessage(msg);
				svr.BroadcastChatTextMessage(msg);
			}
		}

		svr.serverState=0;
		FsSoundStopAll();

		ClearUserInterface();    // 2009/03/29
		fsConsole.SetAutoFlush(YSTRUE);
	}
	else
	{
		fsConsole.SetAutoFlush(YSFALSE);
	}
	return YSOK;
}

YSRESULT FsSimulation::ServerState_GhostMode(const double &,class FsSocketServer &svr)
{
	if(terminate==YSTRUE)
	{
		svr.serverState=0;
		SetPlayerAirplane(NULL); // 2006/07/29
		FsSoundStopAll();
	}
	return YSOK;
}

YSRESULT FsSimulation::ServerState_DispellUser
    (const double &passedTime,class FsSocketServer &svr,FsChoose &dispellUser)
{
	int c;
	int ky,mx,my;
	YSBOOL lb,mb,rb;

	FsPollDevice();
	ky=FsInkey();
	c=FsInkeyChar();
	FsGetMouseEvent(lb,mb,rb,mx,my);

	if(YSTRUE!=FsIsConsoleServer())
	{
		FsClearScreenAndZBuffer(YsGrayScale(0.25));
		dispellUser.Draw(32,32);
		FsSwapBuffers();

		if(dispellUser.ProcessMouse(32,32,lb,mb,rb,mx,my)==YSOK)
		{
		}
		else if(dispellUser.KeyIn(ky,c)==YSOK)
		{
		}
		else if(ky==FSKEY_ESC || dispellUser.GetCancelButtonClicked()==YSTRUE)
		{
			svr.serverState=0;
		}
		else if(ky==FSKEY_SPACE || ky==FSKEY_ENTER || dispellUser.GetOkButtonClicked()==YSTRUE)
		{
			int choiceId,userId;
			const char *username;
			choiceId=dispellUser.GetChoiceId();
			username=dispellUser.GetChoice();
			if('0'<=username[0] && username[0]<='9')
			{
				userId=atoi(username);
				svr.DisconnectUser(userId);
				dispellUser.DeleteChoice(choiceId);
			}
		}
	}
	else
	{
		YSBOOL selected,cancelled;
		dispellUser.KeyInInConsoleMode(selected,cancelled,ky,c);
		if(YSTRUE==selected)
		{
			int choiceId,userId;
			const char *username;
			choiceId=dispellUser.GetChoiceId();
			username=dispellUser.GetChoice();
			if(NULL!=username && '0'<=username[0] && username[0]<='9')
			{
				userId=atoi(username);
				svr.DisconnectUser(userId);
				dispellUser.DeleteChoice(choiceId);
			}
			svr.serverState=0;
		}
		if(YSTRUE==cancelled)
		{
			svr.serverState=0;
		}
	}

	FsSleep(25);

	return YSOK;
}

void FsSimulation::RunServerModeTest(const char [],const char [],FsNetConfig &)
{
}

void FsSimulation::RunServerModeOneStep(FsServerRunLoop &svrSta)
{
	FsSocketServer &server=svrSta.svr;
	FsFlightConfig &cfg=*server.cfg;

	switch(svrSta.runState)
	{
	case FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE1:
		svrSta.resetServer=YSFALSE;
		svrSta.serverStartTime=(unsigned long)time(NULL);
		svrSta.prvTime=(unsigned long)time(NULL);

		server.LoadWelcomeMessage(FsGetNetWelcomeMessageFile());

		if(world->AddField(NULL,svrSta.fldName,YsOrigin(),YsZeroAtt())==NULL)
		{
			svrSta.fatalError=FsServerRunLoop::SERVER_FATAL_FIELD_UNAVAILABLE;
			svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_TERMINATED;
			return;
		}
		world->TieDownCarrier();

		svrSta.startServerRetryCount=0;
		svrSta.nextServerStartTryTime=time(NULL);
		svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE2;
		break;

	case FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE2:
		if(svrSta.nextServerStartTryTime<=time(NULL))
		{
			if(server.Start()==YSOK)
			{
				svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE3;
			}
			else
			{
				++svrSta.startServerRetryCount;
				if(6<=svrSta.startServerRetryCount)
				{
					svrSta.fatalError=FsServerRunLoop::SERVER_FATAL_CANNOT_START;
					svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_TERMINATED;
				}
				fsConsole.Printf("Failed to start server.\n");
				fsConsole.Printf("Retry (Count=%d)\n",svrSta.startServerRetryCount);

				svrSta.nextServerStartCountDown=time(NULL)+5;
				svrSta.nextServerStartTryTime=time(NULL)+90;
			}
		}
		else if(svrSta.nextServerStartCountDown<=time(NULL))
		{
			fsConsole.Printf("%d seconds to retry.\n",(int)(svrSta.nextServerStartTryTime-time(NULL)));
			svrSta.nextServerStartCountDown=time(NULL)+5;
		}
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE3:
		{
			svrSta.svrDlg->Initialize();
			svrSta.svrDlg->MakeDialog(this,&server);
			svrSta.svrDlg->AutoArrangeDialog();
			fsConsole.SetDialog(svrSta.svrDlg);

			switch(svrSta.netcfg.serverControlBlackOut)
			{
			case 1:
				SetBlackOut(YSTRUE);
				break;
			case 2:
				SetBlackOut(YSFALSE);
				break;
			}
			switch(svrSta.netcfg.serverControlCanLandAnywhere)
			{
			case 1:
				SetCanLandAnywhere(YSTRUE);
				break;
			case 2:
				SetCanLandAnywhere(YSFALSE);
				break;
			}
			switch(svrSta.netcfg.serverControlMidAirCollision)
			{
			case 1:
				SetMidAirCollision(YSTRUE);
				break;
			case 2:
				SetMidAirCollision(YSFALSE);
				break;
			}
			SetShowUserName(svrSta.netcfg.serverControlShowUserName);

			SetDisableThirdAirplaneView(svrSta.netcfg.disableThirdAirplaneView);

			if(svrSta.netcfg.saveChatLog==YSTRUE)
			{
				YsString dateCStr;
				FsGetTodayFileString(dateCStr);

				YsWString chatLogFile,dateStr;
				dateStr.SetUTF8String(dateCStr);

				chatLogFile.Set(FsGetNetChatLogDir());
				chatLogFile.Append(L"/");
				chatLogFile.Append(dateStr);
				chatLogFile.Append(L"server.txt");
				if(server.chatLogFp.Fopen(chatLogFile,"a")==NULL)
				{
					YsFileIO::MkDir(FsGetNetChatLogDir());
					server.chatLogFp.Fopen(chatLogFile,"a");
				}
				if(server.chatLogFp.Fp()!=NULL)
				{
					YsString timeStr;
					FsGetTodayTimeString(timeStr);
					fprintf(server.chatLogFp.Fp(),"Start Server Mode\n");
					fprintf(server.chatLogFp.Fp(),"%s\n",(const char *)dateCStr);
					fprintf(server.chatLogFp.Fp(),"%s\n",(const char *)timeStr);
				}
			}

			YsString fieldNameBuf;
			const char *fieldName;
			YsVec3 fieldPos;
			YsAtt3 fieldAtt;

			svrSta.dispellUser->showCancelButton=YSTRUE;
			GetLoadedField(fieldNameBuf,fieldPos,fieldAtt);
			fieldName=fieldNameBuf;


		#ifdef CRASHINVESTIGATION
			FsLogOutput(FSSVRLOGFILE,"Starting Server Mode");
		#endif


			server.enduranceModeRemainingTime=0.0;
			server.enduranceModeJet=YSTRUE;
			server.enduranceModeWw2=YSFALSE;

			server.baseDefenseModeRemainingTime=0.0;
			server.closeAirSupportMissionRemainingTime=0.0;
			server.netNoActivityTime=0.0;
			server.gLimit=4.0;
			server.choosingMode=0;

			svrSta.chooseAirplane->allowAam=svrSta.netcfg.useMissile;
			svrSta.chooseAirplane->allowAgm=svrSta.netcfg.useMissile;
			world->AllowAAM(svrSta.netcfg.useMissile);
			world->AllowAGM(svrSta.netcfg.useMissile);

			svrSta.chooseAirplane->allowBomb=svrSta.netcfg.useUnguidedWeapon;
			svrSta.chooseAirplane->allowRocket=svrSta.netcfg.useUnguidedWeapon;
			world->AllowGun(svrSta.netcfg.useMissile);
			world->AllowBomb(svrSta.netcfg.useMissile);
			world->AllowRocket(svrSta.netcfg.useMissile);

			svrSta.chooseAirplane->Initialize();
			svrSta.chooseAirplane->Create(world,*svrSta.opt,0);
			svrSta.chooseAirplane->SetDefault(cfg.defAirplane);
			svrSta.chooseAirplane->ResetOrdinance();

			svrSta.chooseStartPosition->useFldListBox=YSFALSE;
			svrSta.chooseStartPosition->fldNameForStpSelector.Set(fieldName);
			svrSta.chooseStartPosition->Create(world);
			svrSta.chooseStartPosition->ResetStartPosSelection();
			svrSta.chooseStartPosition->Select(fieldName,svrSta.netcfg.defStartPosServer);
			svrSta.chooseStartPosition->CaptureSelection();
			svrSta.chooseStartPosition->ReloadField();
			svrSta.chooseStartPosition->ReloadStartPosition();

			if(svrSta.netcfg.groundFire!=YSTRUE)		// Added on 2002/06/28
			{
				FsGround *gnd=NULL;
				while((gnd=FindNextGround(gnd))!=NULL)
				{
					if(gnd->Prop().GetNumSAM()>0 || gnd->Prop().GetNumAaaBullet()>0)
					{
						gnd->Prop().SetDamageTolerance(0);
						gnd->Prop().SetState(FSGNDDEAD);
					}
				}
			}

			FsPrintServerMenu(this,
			    *svrSta.chooseAirplane,
			    *svrSta.chooseStartPosition,
			    server.iff,
			    server.locked,
			    server.joinLocked,
			    svrSta.netcfg.portNumber);

			PrepareRunSimulation();

			for(int i=0; i<24; i++)
			{
				fsConsole.Printf("");
			}
			fsConsole.Printf("********************");
			fsConsole.Printf("Starting Server Mode");
			fsConsole.Printf("********************");


			server.quit=YSFALSE;
			server.escCount=0;
			server.serverState=0;

			fsConsole.SetAutoFlush(YSTRUE);
			FsPrintServerMenu(
			    this,
			    *svrSta.chooseAirplane,
			    *svrSta.chooseStartPosition,
			    server.iff,
			    server.locked,
			    server.joinLocked,
			    svrSta.netcfg.portNumber);

			PassedTime();
			server.timeToBroadcastAirplane=0.0;
			server.timeToBroadcastGround=0.0;
			server.nextConsoleUpdateTime=0.0;

			svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_LOOP;

			fsConsole.Printf("Ready to go!");

		#ifdef CRASHINVESTIGATION
			svrSta.prevGtime=time(NULL);
		#endif
		}
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_LOOP:
		{
		#ifdef CRASHINVESTIGATION
			static time_t prevGtime=0;
			time_t gtime=time(NULL);
			if(prevGtime!=gtime)
			{
				struct tm *ltime=localtime(&gtime);
				printf
				   ("%04d/%02d/%02d %02d:%02d %02d\n",
				    ltime->tm_year+1900,ltime->tm_mon+1,ltime->tm_mday,ltime->tm_hour,ltime->tm_min,ltime->tm_sec);
				svrSta.prevGtime=gtime;
			}
			printf("N1\n");
		#endif

			double passedTime=PassedTime();

			// 2009/07/10 >>
			if(passedTime>=60.0)
			{
				char str[256];
				sprintf(str,"Warning: Server was stopping for %.2lf seconds.\n",passedTime);
				server.AddMessage(str);
				passedTime=60.0;
			}
			// 2009/07/10 <<

			server.timeToBroadcastAirplane+=passedTime;
			if(server.timeToBroadcastAirplane>0.1)
			{
				server.BroadcastAirplaneState();
				if(svrSta.netcfg.useMissile!=YSTRUE)
				{
					server.RectifyIllegalMissiles();
				}
				server.timeToBroadcastAirplane=0.0;
			}
			server.timeToBroadcastGround+=passedTime;
			if(server.timeToBroadcastGround>0.6)
			{
				server.BroadcastGroundState();
				server.timeToBroadcastGround=0.0;
			}

			const int prevServerState=server.serverState;

			server.CheckPendingUser(currentTime);

			switch(server.serverState)
			{
			case 0:
				ServerState_StandBy(
				    passedTime,server,
				    *svrSta.chooseAirplane,
				    *svrSta.chooseStartPosition,
				    *svrSta.dispellUser,*svrSta.svrDlg);
				break;
			case 1:
				ServerState_Flying(passedTime,server);
				break;
			case 2:
				ServerState_DispellUser(passedTime,server,*svrSta.dispellUser);
				break;
			case 3:
				ServerState_GhostMode(passedTime,server);
				break;
			default:
				fsConsole.Printf("Invalid Server State.");
				fsConsole.Printf("Press Key to terminate.");
				while(FsInkey()!=FSKEY_NULL)
				{
					FsPollDevice();
					FsSleep(50);
				}
				svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_TERMINATING_KEYWAIT;
				svrSta.fatalError=FsServerRunLoop::SERVER_FATAL_UNKNOWN_STATE;
				server.quit=YSTRUE;
				return;
			}

			if(prevServerState!=0 && server.serverState==0)
			{
				FsPrintServerMenu(this,
				   *(svrSta.chooseAirplane),
				   *(svrSta.chooseStartPosition),
				   server.iff,
				   server.locked,
				   server.joinLocked,
				   svrSta.netcfg.portNumber);
			}
			else if(prevServerState==0 && server.serverState!=0)
			{
			}

			/* if(server.dialogInvokeRedraw==YSTRUE && server.serverState==0)
			{
				server.dialogInvokeRedraw=YSFALSE;
				FsPrintServerMenu
				(this,chooseAirplane,chooseStartPosition,server.iff,server.locked,server.joinLocked,netcfg.portNumber);
			} */



			const YSBOOL networkStandby=((server.serverState!=1 && server.serverState!=3) ? YSTRUE : YSFALSE);

			FSUSERCONTROL userControl;
			switch(server.serverState)
			{
			default:
				userControl=FSUSC_ENABLE;
				break;
			case 3:  // Ghost mode
				userControl=FSUSC_VIEWCONTROLONLY;
				break;
			}

			SimulateOneStep(passedTime,YSFALSE,YSTRUE,YSFALSE,networkStandby,userControl,YSFALSE);
			if(svrSta.netcfg.recordWhenServerMode!=YSTRUE)
			{
				DemoModeRipOffEarlyPartOfRecord();
			}

			FsAirplane *air=NULL;
			while((air=FindNextAirplane(air))!=NULL)
			{
				air->netNextState.tLocal=-1.0;
				air->netNextState.tRemote=-1.0;
			}

		#ifdef CRASHINVESTIGATION
			printf("N2\n");
		#endif
			server.CheckAndAcceptConnection();
			server.DisconnectInactiveUser(passedTime,180.0);

		#ifdef CRASHINVESTIGATION
			printf("N3\n");
		#endif
			server.CheckReceive();

		#ifdef CRASHINVESTIGATION
			printf("N4\n");
		#endif
			server.BroadcastStateChange();

		#ifdef CRASHINVESTIGATION
			printf("N5\n");
		#endif
			server.FlushAllSendQueue(FS_NETTIMEOUT);

		#ifdef CRASHINVESTIGATION
			printf("N6\n");
		#endif
			air=NULL;
			while((air=FindNextAirplane(air))!=NULL)
			{
				if(// air->netType==FSNET_REMOTE &&  << I thought it's a good idea, but even
				   //                                   local airplane must receivean airplane
				   //                                   state once to initialize its position
				   air->netAlive==YSTRUE &&       // 2002/11/13 anti-ghost
				   air->netNextState.tLocal>air->netPrevState.tLocal &&
				   air->netNextState.tRemote>air->netPrevState.tRemote)
				{
					air->Prop().NetworkDecode(air->netPrevState,air->netNextState);
				}
			}


			if(svrSta.netcfg.recordWhenServerMode!=YSTRUE &&
			   svrSta.netcfg.freeMemoryWhenPossibleServerMode==YSTRUE &&
			   GetNumAirplane()>0)
			{
				NetFreeMemoryWhenPossible(server.netNoActivityTime,passedTime,&server);
		#ifdef CRASHINVESTIGATION
			printf("Clean Up\n");
		#endif
			}
			if(server.enduranceModeRemainingTime>YsTolerance)
			{
				GenerateEnemyAirplane(
				    server.nEnduranceModeEnemyMax,server.gLimit,server.enduranceModeRemainingTime,svrSta.netcfg.useMissile,
				    server.enduranceModeJet,server.enduranceModeWw2);

				server.enduranceModeRemainingTime-=passedTime;

				if(server.enduranceModeRemainingTime<=YsTolerance)
				{
					server.AddMessage("**** END ENDURANCE MODE ****");
					AddTimedMessage("**** END ENDURANCE MODE ****");
					server.BroadcastTextMessage("**** END ENDURANCE MODE ****");
				#ifdef CRASHINVESTIGATION
					FsLogOutput(FSSVRLOGFILE,"Ending Endurance Mode");
				#endif
				}
			}
			if(server.baseDefenseModeRemainingTime>YsTolerance)
			{
				double timeElapse;
				int nAttacker;
				timeElapse=15.0*60.0-server.baseDefenseModeRemainingTime;
				nAttacker=int(timeElapse/120.0)+1;
				nAttacker=YsBound(nAttacker,1,server.imInfo->attackerInfo.maxNumAttacker);
				GenerateAttackerAirplane(nAttacker,server.imInfo->attackerInfo,20000.0,8000.0,YSFALSE,300.0);

				server.baseDefenseModeRemainingTime-=passedTime;

				if(server.baseDefenseModeRemainingTime<=YsTolerance)
				{
					server.AddMessage("**** END INTERCEPT MISSION ****");
					AddTimedMessage("**** END INTERCEPT MISSION ****");
					server.BroadcastTextMessage("**** END INTERCEPT MISSION ****");
				#ifdef CRASHINVESTIGATION
					FsLogOutput(FSSVRLOGFILE,"Ending Intercept Mission");
				#endif
				}
			}
			if(server.closeAirSupportMissionRemainingTime>YsTolerance)
			{
				const int maxNumTank=10;
				double timeElapse;
				int nTank;
				FsCloseAirSupportMissionInfo info;

				timeElapse=15.0*60.0-server.closeAirSupportMissionRemainingTime;

				nTank=int(timeElapse/60.0)+3;
				nTank=YsBound(nTank,3,maxNumTank);
				GenerateTank(nTank,info,YSTRUE,YSTRUE,YSTRUE,FS_IFF3,6,4);

				server.closeAirSupportMissionRemainingTime-=passedTime;
				if(server.closeAirSupportMissionRemainingTime<=YsTolerance)
				{
					server.AddMessage("**** END CLOSE AIR SUPPORT MISSION ****");
					AddTimedMessage("**** END CLOSE AIR SUPPORT MISSION ****");
					server.BroadcastTextMessage("**** END CLOSE AIR SUPPORT MISSION ****");
				#ifdef CRASHINVESTIGATION
					FsLogOutput(FSSVRLOGFILE,"Ending Close Air Support Mission");
				#endif
				}
			}


			if(svrSta.netcfg.serverResetTime!=0)
			{
				unsigned long curTime,timeElapsed,prvTimeRemain,timeRemain,resetTimer;  // All in seconds

				resetTimer=svrSta.netcfg.serverResetTime*60;
				curTime=(unsigned long)time(NULL);
				timeElapsed=curTime-svrSta.serverStartTime;
				if(timeElapsed>=resetTimer)
				{
					svrSta.resetServer=YSTRUE;
					break;
				}
				if(curTime!=svrSta.prvTime)
				{
					const char *timeMsg;
					char msg[256];

					timeMsg=NULL;

					timeRemain=resetTimer-timeElapsed;
					prvTimeRemain=resetTimer-(svrSta.prvTime-svrSta.serverStartTime);
					if(timeRemain<=3)
					{
						server.AddMessage("**** Resetting Server ****");
						AddTimedMessage("**** Resetting Server ****");
						server.BroadcastTextMessage("**** Resetting Server ****");

					#ifdef CRASHINVESTIGATION
						FsLogOutput(FSSVRLOGFILE,"Resetting Server");
					#endif
					}
					else if(timeRemain<=10 && 10<prvTimeRemain)
					{
						timeMsg="10 seconds";
					}
					else if(timeRemain<=30 && 30<prvTimeRemain)
					{
						timeMsg="30 seconds";
					}
					else if(timeRemain<=60 && 60<prvTimeRemain)
					{
						timeMsg="60 seconds";
					}
					else if(timeRemain<=300 && 300<prvTimeRemain)
					{
						timeMsg="5 minutes";
					}
					else if(timeRemain<=600 && 600<prvTimeRemain)
					{
						timeMsg="10 minutes";
					}
					else if(timeRemain<=900 && 900<prvTimeRemain)
					{
						timeMsg="15 minutes";
					}
					else if(timeRemain<=1200 && 1200<prvTimeRemain)
					{
						timeMsg="20 minutes";
					}
					else if(timeRemain<=1800 && 1800<prvTimeRemain)
					{
						timeMsg="30 minutes";
					}

					if(timeMsg!=NULL)
					{
						sprintf(msg,"**** Server will be reset in %s ****",timeMsg);
						server.AddMessage(msg);
						AddTimedMessage(msg);
						sprintf(msg,"**** Server will be reset in %s (You'll be logged out) ****",timeMsg);
						server.BroadcastTextMessage(msg);
					}
				}
				svrSta.prvTime=curTime;
			}

		#ifdef CRASHINVESTIGATION
			printf("N-1\n");
		#endif
		}
		if(YSTRUE==server.quit)
		{
			svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_TERMINATING;
		}
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_TERMINATING_KEYWAIT:
		{
			FsPollDevice();
			if(FSKEY_NULL!=FsInkey())
			{
				svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_TERMINATING;
			}
			fsConsole.Show();
			FsSleep(50);
		}
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_TERMINATING:
		server.Terminate();

		fsConsole.SetAutoFlush(YSTRUE);

		AfterSimulation();

		if(server.ReceivedKillServer()==YSTRUE)
		{
			fsStderr.Printf("Received FSNETCMD_KILLSERVER from client.");
		}

		if(svrSta.netcfg.saveChatLog==YSTRUE && server.chatLogFp.Fp()!=NULL)
		{
			YsString timeStr;
			FsGetTodayTimeString(timeStr);
			fprintf(server.chatLogFp.Fp(),"End Server Mode\n");
			fprintf(server.chatLogFp.Fp(),"%s\n",(const char *)timeStr);
			fprintf(server.chatLogFp.Fp(),"\n");
			fprintf(server.chatLogFp.Fp(),"\n");
		}

		fsConsole.SetDialog(NULL);

		svrSta.runState=FsServerRunLoop::SERVER_RUNSTATE_TERMINATED;

		break;
	case FsServerRunLoop::SERVER_RUNSTATE_TERMINATED:
		FsSleep(50);
		break;
	}
}

void FsSimulation::DrawInServerMode(const class FsServerRunLoop &svrSta) const
{
	const FsSocketServer &server=svrSta.svr;

	switch(svrSta.runState)
	{
	case FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE1:
	case FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE2:
	case FsServerRunLoop::SERVER_RUNSTATE_INITIALIZE3:
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_LOOP:
		{
			if(0==server.serverState)
			{
				if(server.choosingMode==0 && (server.nextConsoleUpdateTime<0.0 || YSTRUE==svrSta.svrDlg->NeedRedraw()))
				{
					fsConsole.Show();
					server.nextConsoleUpdateTime=0.5;
				}
			}
			else
			{
				SimDrawAllScreen(YSFALSE,YSFALSE,YSFALSE);
			}
		}
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_TERMINATING_KEYWAIT:
		fsConsole.Show();
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_TERMINATING:
		break;
	case FsServerRunLoop::SERVER_RUNSTATE_TERMINATED:
		break;
	}
}


////////////////////////////////////////////////////////////

class FsGuiClientDialog : public FsGuiDialog
{
public:
	int prevWid,prevHei;

	FsSimulation *sim;
	FsSocketClient *cli;

	FsGuiButton *iffBtn[4];
	FsGuiButton *selectAirBtn;
	FsGuiButton *selectStpBtn;
	FsGuiButton *joinBtn;
	FsGuiButton *observerModeBtn;
	FsGuiButton *listUserBtn;

	FsGuiButton *whoKilledMeBtn,*whomHaveIKilledBtn;

	FsGuiStatic *netLimitTxt[3];

	FsGuiButton *logOutBtn;
	FsGuiButton *confirmLogOutBtn;

	FsGuiTextBox *chatTxt;
	FsGuiButton *sendChatMessage;

	void MakeDialog(FsSimulation *sim,FsSocketClient *cli);
	void AutoArrangeDialog(void);
	virtual void OnButtonClick(FsGuiButton *btn);
	virtual void OnTextBoxSpecialKey(FsGuiTextBox *txt,int fskey);
};

void FsGuiClientDialog::MakeDialog(FsSimulation *sim,FsSocketClient *cli)
{
	prevWid=0;
	prevHei=0;

	this->sim=sim;
	this->cli=cli;

	AddStaticText(0,FSKEY_NULL,"IFF",YSTRUE);
	iffBtn[0]=AddTextButton(MkId("iff1"),FSKEY_NULL,FSGUI_RADIOBUTTON,"1",YSFALSE);
	iffBtn[1]=AddTextButton(MkId("iff2"),FSKEY_NULL,FSGUI_RADIOBUTTON,"2",YSFALSE);
	iffBtn[2]=AddTextButton(MkId("iff3"),FSKEY_NULL,FSGUI_RADIOBUTTON,"3",YSFALSE);
	iffBtn[3]=AddTextButton(MkId("iff4"),FSKEY_NULL,FSGUI_RADIOBUTTON,"4",YSFALSE);
	SetRadioButtonGroup(4,iffBtn);

	selectAirBtn=AddTextButton(MkId("selectAir"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_SELECTAIR,YSTRUE);
	selectStpBtn=AddTextButton(MkId("selectStp"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_SELECTSTP,YSFALSE);
	joinBtn=AddTextButton(MkId("join"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_JOIN,YSFALSE);
	observerModeBtn=AddTextButton(MkId("observer"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_OBSERVERMODE,YSFALSE);

	listUserBtn=AddTextButton(MkId("listUser"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_LISTUSER,YSTRUE);

	whoKilledMeBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_WHOKILLEDME,YSTRUE);
	whomHaveIKilledBtn=AddTextButton(0,FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_WHOMHAVEIKILLED,YSFALSE);

	netLimitTxt[0]=AddStaticText(0,FSKEY_NULL,"",20,1,YSTRUE);
	netLimitTxt[1]=AddStaticText(0,FSKEY_NULL,"",20,1,YSTRUE);
	netLimitTxt[2]=AddStaticText(0,FSKEY_NULL,"",20,1,YSTRUE);

	logOutBtn=AddTextButton(MkId("logOut"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_LOGOUT,YSTRUE);
	confirmLogOutBtn=AddTextButton(MkId("confirmLogOut"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_CONFIRMLOGOUT,YSFALSE);
	confirmLogOutBtn->Disable();

	AddStaticText(0,FSKEY_NULL,FSGUI_NETCONSOLE_CHAT,YSTRUE);
	chatTxt=AddTextBox(MkId("chatTxt"),FSKEY_NULL,"","",32,YSFALSE);
	chatTxt->SetLengthLimit(140);
	sendChatMessage=AddTextButton(MkId("sendChatMsg"),FSKEY_NULL,FSGUI_PUSHBUTTON,FSGUI_NETCONSOLE_SEND,YSFALSE);

	Fit();
	SetTransparency(YSFALSE);
}

void FsGuiClientDialog::AutoArrangeDialog(void)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	if(prevWid!=wid)
	{
		Move(wid-dlgWid,0);
		prevWid=wid;
	}
}

void FsGuiClientDialog::OnButtonClick(FsGuiButton *btn)
{
	if(btn!=logOutBtn && btn!=confirmLogOutBtn)
	{
		confirmLogOutBtn->Disable();
	}

	YsString str;
	chatTxt->GetText(str);
	if(btn==sendChatMessage && 0<str.Strlen())
	{
		sim->SendNetChatMessage(str);
		chatTxt->SetText("");
	}
	else if(btn==iffBtn[0])
	{
		cli->commandQueue.push(FSNCC_COMMON_IFF1);
	}
	else if(btn==iffBtn[1])
	{
		cli->commandQueue.push(FSNCC_COMMON_IFF2);
	}
	else if(btn==iffBtn[2])
	{
		cli->commandQueue.push(FSNCC_COMMON_IFF3);
	}
	else if(btn==iffBtn[3])
	{
		cli->commandQueue.push(FSNCC_COMMON_IFF4);
	}
	else if(btn==selectAirBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_SELECTAIR);
	}
	else if(btn==selectStpBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_SELECTSTP);
	}
	else if(btn==joinBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_JOIN);
	}
	else if(btn==observerModeBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_OBSERVERMODE);
	}
	else if(btn==listUserBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_LISTPLAYER);
	}
	else if(btn==logOutBtn)
	{
		confirmLogOutBtn->Enable();
	}
	else if(btn==confirmLogOutBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_QUIT);
	}
	else if(btn==whoKilledMeBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_WHOKILLEDME);
	}
	else if(btn==whomHaveIKilledBtn)
	{
		cli->commandQueue.push(FSNCC_COMMON_WHOMHAVEIKILLED);
	}

}

void FsGuiClientDialog::OnTextBoxSpecialKey(FsGuiTextBox *txt,int fskey)
{
	YsString str;
	switch(fskey)
	{
	case FSKEY_ENTER:
		chatTxt->GetText(str);
		if(txt==chatTxt && 0<str.Strlen())
		{
			sim->SendNetChatMessage(str);
			chatTxt->SetText("");
		}
		break;
	case FSKEY_ESC:
		cli->commandQueue.push(FSNCC_COMMON_ESC);
		break;
	}
}


FsClientVariable::FsClientVariable(const char _name[],FsSimulation *_sim,FsNetConfig *_netcfg)
{
	FsNetworkVariable::Initialize();

	cfg=new FsFlightConfig;

	cfg->Load(FsGetConfigFile());

	username.Set(_name);
	sim=_sim;
	netcfg=_netcfg;

	logOnProcessCompleted=YSFALSE;
	airplaneAvailable=YSFALSE;
}

FsClientVariable::~FsClientVariable()
{
	delete cfg;
}

static void FsPrintClientMenu(FsSimulation *sim,FsGuiChooseAircraft &airplane,FsGuiChooseField &startPosition,FSIFF iff)
{
	const int chatKey=sim->ctlAssign.FindKeyByFunction(FSBTF_INFLTMESSAGE);
	const char *chatKeyTxt=(FsGetKeyString(chatKey)!=NULL ? FsGetKeyString(chatKey) : "NONE");
	char txt[256];
	sprintf(txt,"[%s]...Send Message",chatKeyTxt);

	fsConsole.Printf("------");
	fsConsole.Printf("IFF : %d",iff+1);
	fsConsole.Printf("Airplane : %s",(const char *)airplane.selAir);
	fsConsole.Printf("Starting Position : %s",(const char *)startPosition.selStp);
	fsConsole.Printf("[A]...Choose Airplane     [S].....Choose Start Position");
	fsConsole.Printf("[J]...Join Flight         [ESC]...Terminate Client");
	fsConsole.Printf("[L]...List Flying Players [V].....Observer Mode");
	fsConsole.Printf("[1][2][3][4]...Choose IFF");
	fsConsole.Printf("[F1]...Server airplane's cockpit");
	fsConsole.Printf("[F2]...Server airplane's left view");
	fsConsole.Printf("[F3]...Server airplane's right view");
	fsConsole.Printf("[F4]...Server airplane's rear view");
	fsConsole.Printf("%s",txt);
}

YSRESULT FsSimulation::ClientState_StandBy(
    const double &passedTime,
    class FsSocketClient &cli,FsGuiChooseAircraft &chooseAirplane,FsGuiChooseField &chooseStartPosition,
    FsGuiClientDialog &cliDlg)
{
	int i,prevChoosingMode;

	escKeyCount=0;  // Don't confuse with escCount

	for(i=0; i<FsMaxNumSubWindow; i++)
	{
		if(FsIsSubWindowOpen(i)==YSTRUE)
		{
			FsCloseSubWindow(i);
			FsSelectMainWindow();
		}
	}

	if(cli.fieldReady!=YSTRUE && GetField()!=NULL)
	{
		YsString fieldNameBuf;
		const char *fldName;
		YsVec3 fieldPos;
		YsAtt3 fieldAtt;

		GetLoadedField(fieldNameBuf,fieldPos,fieldAtt);
		fldName=fieldNameBuf;

		chooseStartPosition.useFldListBox=YSFALSE;
		chooseStartPosition.fldNameForStpSelector.Set(fldName);
		chooseStartPosition.Create(world);
		chooseStartPosition.ResetStartPosSelection();
		chooseStartPosition.Select(fldName,cli.netcfg->defStartPosClient);
		chooseStartPosition.CaptureSelection();
		chooseStartPosition.ReloadField();
		chooseStartPosition.ReloadStartPosition();

		cli.fieldReady=YSTRUE;

		for(i=0; i<24; i++)
		{
			fsConsole.Printf("");
		}
		fsConsole.Printf("Received Field");
	}
	else if(cli.fieldNotAvailable==YSTRUE)
	{
		fsConsole.Printf("ERROR: Field Not Available.\n");
		fsConsole.Printf("Disconnecting.\n");
		cli.quit=YSTRUE;
	}



	FsPollDevice();
	int ky=FsInkey();
	int c=FsInkeyChar();

	int mx,my;
	YSBOOL lb,mb,rb;
	FsGetMouseEvent(lb,mb,rb,mx,my);

	if(0==cli.choosingMode)
	{
		cliDlg.AutoArrangeDialog();
		cliDlg.SetMouseState(lb,mb,rb,mx,my);
		cliDlg.KeyIn(ky,(YSBOOL)FsGetKeyState(FSKEY_SHIFT),(YSBOOL)FsGetKeyState(FSKEY_CTRL),(YSBOOL)FsGetKeyState(FSKEY_ALT));
		cliDlg.CharIn(c);

		if(cliDlg.chatTxt!=cliDlg.GetFocus())
		{
			auto cmd=FsTranslateKeyToCommonCommand(ky,this);
			if(FSNCC_COMMON_NULL==cmd)
			{
				cmd=FsTranslateKeyToClientCommand(ky);
			}
			if(FSNCC_COMMON_NULL!=cmd)
			{
				cli.commandQueue.push(cmd);
			}
		}
	}
	FSNET_CONSOLE_COMMAND consCmd=FSNCC_COMMON_NULL;
	if(true!=cli.commandQueue.empty())
	{
		consCmd=cli.commandQueue.front();
		cli.commandQueue.pop();
	}



	prevChoosingMode=cli.choosingMode;
	FsNetworkStandby(this,&cli,consCmd,ky,c,lb,mb,rb,mx,my,cli.escCount,cli.choosingMode,chooseAirplane,chooseStartPosition,cli.fieldReady);
	if((prevChoosingMode!=0 && prevChoosingMode!=10) && (cli.choosingMode==0 || cli.choosingMode==10))
	{
		fsConsole.Printf("");
		FsPrintClientMenu(this,chooseAirplane,chooseStartPosition,cli.iff);
	}
	else if((prevChoosingMode==0 || prevChoosingMode==10) && (cli.choosingMode!=0 && cli.choosingMode!=10))
	{
	}



	if(cli.choosingMode==0)
	{
		if(cli.fieldReady==YSTRUE && cli.simReady==YSTRUE)
		{
			switch(consCmd)
			{
			default:
				break;
			case FSNCC_COMMON_PRINTMENU:
				FsPrintClientMenu(this,chooseAirplane,chooseStartPosition,cli.iff);
				break;
			case FSNCC_COMMON_JOIN:
				if(cli.waitingForJoiningApproval!=YSTRUE &&
				   chooseStartPosition.selStp[0]!=0)
				{
					printf("Join Request.\n");

					fsConsole.Printf("Sent Join Request.");

					cli.waitingForJoiningApproval=YSTRUE;
					cli.joinReqReadBack=YSFALSE;
					cli.ReceivedApproval(); // <- This clears the flag.
					cli.SendJoinRequest
					    (cli.iff,chooseAirplane.selAir,
					     chooseStartPosition.selStp,
					     chooseAirplane.selFuel,
					     chooseAirplane.SmokeLoaded());

					std::queue <FSNET_CONSOLE_COMMAND> emptyQueue;
					cli.commandQueue.swap(emptyQueue);
				}
				break;
			case FSNCC_COMMON_OBSERVERMODE:
				if(cli.waitingForJoiningApproval!=YSTRUE)
				{
					cli.clientState=3; // Ghost mode
					escKeyCount=0;
					terminate=YSFALSE;
					SetPlayerAirplane(NULL);
					mainWindowViewmode=FSGHOSTVIEW;

					std::queue <FSNET_CONSOLE_COMMAND> emptyQueue;
					cli.commandQueue.swap(emptyQueue);
				}
				break;
			case FSNCC_COMMON_IFF1:
			case FSNCC_COMMON_IFF2:
			case FSNCC_COMMON_IFF3:
			case FSNCC_COMMON_IFF4:
				cli.iff=(FSIFF)(consCmd-FSNCC_COMMON_IFF1);
				FsPrintClientMenu(this,chooseAirplane,chooseStartPosition,cli.iff);

				cliDlg.iffBtn[0]->SetCheck(YSFALSE);
				cliDlg.iffBtn[1]->SetCheck(YSFALSE);
				cliDlg.iffBtn[2]->SetCheck(YSFALSE);
				cliDlg.iffBtn[3]->SetCheck(YSFALSE);
				cliDlg.iffBtn[cli.iff]->SetCheck(YSTRUE);
				break;
			case FSNCC_COMMON_LISTPLAYER:
				if(cli.nextListUserTime>=2.0)
				{
					cli.SendListUser();
				}
				break;
			// FSKEY_F1->F4 assign side window of the server airplane
			case FSNCC_CLI_SVRCOCKPIT:
				cli.SendRequestToBeSideWindow( 0.0,0.0);  // HUD
				break;
			case FSNCC_CLI_SVRLEFTVIEW:
				cli.SendRequestToBeSideWindow( YsPi/2.0,0.0);  // Left view
				break;
			case FSNCC_CLI_SVRRIGHTVIEW:
				cli.SendRequestToBeSideWindow(-YsPi/2.0,0.0);  // Right view
				break;
			case FSNCC_CLI_SVRREARVIEW:
				cli.SendRequestToBeSideWindow(-YsPi,0.0);  // Right view
				break;
			}
		}
	}
	else if(cli.choosingMode==10)
	{
		if(ky==FSKEY_ENTER)
		{
			YsString msgStr;

			msgStr.Set("(");
			msgStr.Append(cli.username);
			msgStr.Append(")");
			msgStr.Append(fsConsole.wInputLine.GetUTF8String().Txt()+1);

			cli.SendTextMessage(msgStr);

			cli.choosingMode=0;
			fsConsole.wInputLine=L"";

			FsPollDevice();
			while(0!=FsInkeyChar())
			{
				FsPollDevice();
			}
		}
	}

	if(cli.waitingForJoiningApproval==YSTRUE)
	{
		if(cli.ReceivedApproval()==YSTRUE)
		{
			if(cli.choosingMode==1)
			{

			}
			cli.choosingMode=0;


			if(GetPlayerAirplane()!=NULL && GetPlayerAirplane()->Prop().IsActive()==YSTRUE)
			{
				FsAirplane *air;
				air=GetPlayerAirplane();

				userInput.Initialize();  // 2006/10/01
				ClearUserInterface();    // 2009/03/29

				air->Prop().ReadBackControl(userInput);
				Gear=air->Prop().GetLandingGear();
				pGear=air->Prop().GetLandingGear();
				ppGear=air->Prop().GetLandingGear();
				userInput.hasAb=air->Prop().GetHasAfterburner();

				// 2001/06/24
				int idOnSvr;
				idOnSvr=FsExistence::GetSearchKey(air);

				cli.SendWeaponConfig(idOnSvr,chooseAirplane.selWeaponConfig.GetN(),chooseAirplane.selWeaponConfig);
				air->Prop().ApplyWeaponConfig(chooseAirplane.selWeaponConfig.GetN(),chooseAirplane.selWeaponConfig);

				if(cli.ServerAllowAAM()!=YSTRUE)
				{
					air->Prop().UnloadGuidedAAM();
					air->Prop().UnloadGuidedAGM();
				}
				if(cli.ServerAllowUnguidedWeapon()!=YSTRUE)
				{
					air->Prop().UnloadUnguidedWeapon();
				}

				for(int smkIdx=0; smkIdx<air->Prop().GetNumSmokeGenerator(); ++smkIdx)
				{
					auto col=chooseAirplane.SmokeColor(smkIdx);

					YsString airCmd;
					airCmd.Printf("SMOKECOL %d %d %d %d",smkIdx,col.Ri(),col.Gi(),col.Bi());
					air->SendCommand(airCmd);
					cli.SendSmokeColor(idOnSvr,smkIdx,col);
				}

				// 2004/09/02 if(chooseAirplane.smk==YSTRUE)
				// 2004/09/02 {
				// 2004/09/02 	air->SendCommand("SMOKEOIL 100.0kg");
				// 2004/09/02 	cli.SendAirCmd(idOnSvr,"SMOKEOIL 100.0kg");
				// 2004/09/02 }
				// 2004/09/02 else
				// 2004/09/02 {
				// 2004/09/02 	air->SendCommand("SMOKEOIL 0.0kg");
				// 2004/09/02 	cli.SendAirCmd(idOnSvr,"SMOKEOIL 0.0kg");
				// 2004/09/02 }
				// 2004/09/02
				// 2004/09/02
				// 2004/09/02 char cmd[256];
				// 2004/09/02 sprintf(cmd,"INITFUEL %d%%",chooseAirplane.fuel);
				// 2004/09/02 air->SendCommand(cmd);
				// 2004/09/02 cli.SendAirCmd(idOnSvr,cmd);


				cli.clientState=1;
				endTime=0.0;
				terminate=YSFALSE;

				cli.waitingForJoiningApproval=YSFALSE;
			}
			else // Packet Order is screwed up.
			{
				fsConsole.Printf("Packet Delay Problem.  Re-sending Join Request to the Server.");
				if(FsVerboseMode==YSTRUE)
				{
					printf("Packet Delay Problem.  Re-sending Join Request to the Server.\n");
				}

				cli.SendResendJoinApproval();

				// Not supposed to clear waitingForJoiningApproval
				// The server is supposed to re-send ,FSCMD_ADDOBJECT,
				// FSCMD_SETPLAYERAIRPLANE,FSCMD_APPROVETOJOIN
				//    OR
				// FSCMD_REJECTJOINREQ
			}
		}

		if(cli.ReceivedRejection()==YSTRUE)
		{
			fsConsole.Printf("Request Rejected by the Server.");
			cli.waitingForJoiningApproval=YSFALSE;
		}
	}

	if(cli.ReceivedSideWindowAssignment()==YSTRUE)
	{
		endTime=0.0;
		terminate=YSFALSE;
		cli.clientState=2;
	}

	if(cli.escCount==2)
	{
		cli.quit=YSTRUE;
	}

	for(i=0; i<cli.GetNumMessage(); i++)
	{
		fsConsole.Printf("%s",cli.GetMessage(i));
	}
	cli.ClearMessage();

	cli.nextConsoleUpdateTime-=passedTime;

	FsSleep(25);

	return YSOK;
}

YSRESULT FsSimulation::ClientState_Flying(const double &,class FsSocketClient &cli)
{
	cli.ReceivedSideWindowAssignment();   // Clear (ignore) request

	if((endTime>YsTolerance && currentTime>endTime) || terminate==YSTRUE)
	{
		int airId;
		FsAirplane *player;
		player=GetPlayerAirplane();

		airId=FsExistence::GetSearchKey(player);
		cli.SendUnjoin(airId,YSFALSE);

		player->Prop().SetFlightState(FSDEAD,FSDIEDOF_NULL);  // 2002/11/13 anti-ghost
		player->netAlive=YSFALSE;                             // 2002/11/13 anti-ghost

		cli.clientState=0;
		FsSoundStopAll();
	}

	return YSOK;
}

YSRESULT FsSimulation::ClientState_GhostMode(const double &,class FsSocketClient &cli)
{
	cli.ReceivedSideWindowAssignment();   // Clear (ignore) request

	if(terminate==YSTRUE)
	{
		cli.clientState=0;
		SetPlayerAirplane(NULL); // 2006/07/29
		FsSoundStopAll();
	}

	return YSOK;
}

YSRESULT FsSimulation::ClientState_SideWindow(const double &,class FsSocketClient &cli)
{
	int ky;
	double hdg,pch;
	FsAirplane *air;

	FsPollDevice();
	ky=FsInkey();
	if(ky==FSKEY_ESC)
	{
		escKeyCount++;  // <- Must take over
	}

	cli.ReceivedSideWindowAssignment();   // Clear (ignore) request

	cli.GetSideWindowAssignment(air,hdg,pch);

	mainWindowViewmode=FSCOCKPITVIEW;
	userInput.viewHdg=hdg;
	userInput.viewPch=pch;
	if(air!=NULL && air->IsAlive()==YSTRUE)
	{
		SetPlayerAirplane(air);
	}

	if(air==NULL || air->IsAlive()!=YSTRUE || escKeyCount>=2 ||
	   (endTime>YsTolerance && currentTime>endTime) || terminate==YSTRUE)
	{
		cli.clientState=0;
		FsSoundStopAll();
	}

	return YSOK;
}

void FsSimulation::RunClientModeOneStep(class FsClientRunLoop &cliSta)
{
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
	switch(cliSta.runState)
	{
	case FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE1:
		{
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
			cliSta.prevSimReady=YSFALSE;
			cliSta.cli.clientState=0;
			cliSta.fatalError=FsClientRunLoop::CLIENT_FATAL_NOERROR;
			if(cliSta.cli.Start()!=YSOK)
			{
				cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED;
				cliSta.fatalError=FsClientRunLoop::CLIENT_FATAL_NO_NETWORK;
				return;
			}
			if(cliSta.cli.Connect(cliSta.hostname)!=YSOK)
			{
				cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED;
				cliSta.fatalError=FsClientRunLoop::CLIENT_FATAL_CANNOT_CONNECT;
				return;
			}
			if(cliSta.cli.SendLogOn(cliSta.username,YSFLIGHT_NETVERSION)!=YSOK)
			{
				cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED;
				cliSta.fatalError=FsClientRunLoop::CLIENT_FATAL_CANNOT_LOGON;
				return;
			}
			cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE2;
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		}
		break;


	case FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE2:
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		cliSta.svrAddrLog->AddAddressToFile(cliSta.hostname,cliSta.port);
		cliSta.cliDlg->MakeDialog(this,&cliSta.cli);
		cliSta.cliDlg->AutoArrangeDialog();
		fsConsole.SetDialog(cliSta.cliDlg);

		cliSta.cli.iff=(FSIFF)cliSta.netcfg.defIFFWhenClient;

		PrepareRunSimulation();


		for(int i=0; i<24; i++)
		{
			fsConsole.Printf("");
		}
		fsConsole.Printf("********************");
		fsConsole.Printf("Starting Client Mode");
		fsConsole.Printf("********************");


		cliSta.cli.choosingMode=0;
		cliSta.chooseAirplane->filterEnabled=YSTRUE;
		cliSta.chooseAirplane->Create(world,*(cliSta.opt),0);


		cliSta.cli.escCount=0;
		cliSta.cli.fatalError=YSFALSE;
		cliSta.cli.connectionClosedByServer=YSFALSE;
		cliSta.cli.lastErrorFromServer=FSNETERR_NOERR;
		cliSta.cli.quit=YSFALSE;
		cliSta.cli.simReady=YSFALSE;
		cliSta.cli.fieldReady=YSFALSE;
		cliSta.cli.fieldNotAvailable=YSFALSE;
		cliSta.cli.waitingForJoiningApproval=YSFALSE;

		PassedTime();
		cliSta.cli.nextAirNotifyTime=0.0;
		cliSta.cli.nextGndNotifyTime=0.0;
		cliSta.cli.nextConsoleUpdateTime=0.0;
		cliSta.cli.netNoActivityTime=0.0;
		cliSta.cli.nextListUserTime=0.0;

		cliSta.cli.sendCriticalInfoTimer=0.0;
		cliSta.cli.joinReqReadBack=YSFALSE;

		if(cliSta.netcfg.saveChatLog==YSTRUE)
		{
			YsString dateCStr;
			FsGetTodayFileString(dateCStr);

			YsWString chatLogFile,dateStr;
			dateStr.SetUTF8String(dateCStr);

			chatLogFile.Set(FsGetNetChatLogDir());
			chatLogFile.Append(L"/");
			chatLogFile.Append(dateStr);
			chatLogFile.Append(L"client.txt");
			if(cliSta.cli.chatLogFp.Fopen(chatLogFile,"a")==NULL)
			{
				YsFileIO::MkDir(FsGetNetChatLogDir());
				cliSta.cli.chatLogFp.Fopen(chatLogFile,"a");
			}
			if(cliSta.cli.chatLogFp.Fp()!=NULL)
			{
				YsString timeStr;
				FsGetTodayTimeString(timeStr);
				fprintf(cliSta.cli.chatLogFp.Fp(),"Start Client Mode\n");
				fprintf(cliSta.cli.chatLogFp.Fp(),"%s\n",(const char *)dateCStr);
				fprintf(cliSta.cli.chatLogFp.Fp(),"%s\n",(const char *)timeStr);
			}
		}

		cliSta.nextTestPacketTimeOut=30.0;

		cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE3;
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		break;


	case FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE3:
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_LOOP;
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		break;
	case FsClientRunLoop::CLIENT_RUNSTATE_LOOP:
		{
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

			const double passedTime=PassedTime();

			cliSta.cli.CheckPendingData(currentTime,*cliSta.chooseAirplane,*cliSta.chooseStartPosition);

			cliSta.nextTestPacketTimeOut-=passedTime;
			if(cliSta.nextTestPacketTimeOut<0.0)
			{
				cliSta.nextTestPacketTimeOut=30.0;
				cliSta.cli.SendTestPacket();
			}

			if(FsVerboseMode==YSTRUE)
			{
				printf("LOOP-IN\n");
				if(cliSta.cli.waitingForJoiningApproval==YSTRUE)
				{
					printf("Waiting for Approval to Join.\n");
				}
			}

			if(FsVerboseMode==YSTRUE)
			{
				FsAirplane *air=NULL;
				while((air=FindNextAirplane(air))!=NULL)
				{
					printf("[%d] %s ",FsExistence::GetSearchKey(air),air->GetPosition().Txt());
					printf("(%.2lf %.2lf %.2lf)",
					    YsRadToDeg(air->GetAttitude().h()),
					    YsRadToDeg(air->GetAttitude().p()),
					    YsRadToDeg(air->GetAttitude().b()));
					printf("%lf %lf",air->netPrevState.tLocal,air->netPrevState.tRemote);
					printf("%lf %lf",air->netNextState.tLocal,air->netNextState.tRemote);

					printf("\n");
				}
			}


			cliSta.cli.nextAirNotifyTime+=passedTime;
			if(cliSta.cli.nextAirNotifyTime>0.1)
			{
				cliSta.cli.NotifyAirplaneState();
				cliSta.cli.nextAirNotifyTime=0.0;
			}
			cliSta.cli.nextGndNotifyTime+=passedTime;
			if(cliSta.cli.nextGndNotifyTime>0.3)
			{
				cliSta.cli.NotifyGroundState();
				cliSta.cli.nextGndNotifyTime=0.0;
			}
			cliSta.cli.nextListUserTime+=passedTime;

			if(cliSta.cli.ServerAllowAAM()!=YSTRUE)
			{
				if(cliSta.chooseAirplane->allowAam==YSTRUE || cliSta.chooseAirplane->allowAgm==YSTRUE)
				{
					cliSta.chooseAirplane->allowAam=YSFALSE;
					cliSta.chooseAirplane->allowAgm=YSFALSE;
					cliSta.chooseAirplane->ResetOrdinance();
				}
			}
			if(cliSta.cli.ServerAllowUnguidedWeapon()!=YSTRUE)
			{
				if(cliSta.chooseAirplane->allowBomb==YSTRUE || cliSta.chooseAirplane->allowRocket==YSTRUE)
				{
					cliSta.chooseAirplane->allowBomb=YSFALSE;
					cliSta.chooseAirplane->allowRocket=YSFALSE;
					cliSta.chooseAirplane->ResetOrdinance();
				}
			}

			if(cliSta.cli.airNameFilter.GetN()>0)
			{
				cliSta.chooseAirplane->filter.AddKeyWord(cliSta.cli.airNameFilter.GetN(),cliSta.cli.airNameFilter);
				cliSta.chooseAirplane->ResetAircraftList();

				cliSta.chooseAirplane->SetDefault(cfgPtr->defAirplane);
				cliSta.chooseAirplane->ResetOrdinance();

				cliSta.cli.airNameFilter.Set(0,NULL);
			}


			int prevClientState;
			prevClientState=cliSta.cli.clientState;
			if(cliSta.cli.clientState==0)  // Network Standby
			{
				ClientState_StandBy(passedTime,cliSta.cli,*cliSta.chooseAirplane,*cliSta.chooseStartPosition,*cliSta.cliDlg);
				if(cliSta.prevSimReady!=YSTRUE && cliSta.cli.simReady==YSTRUE)
				{
					FsPrintClientMenu(this,*cliSta.chooseAirplane,*(cliSta.chooseStartPosition),cliSta.cli.iff);
					cliSta.prevSimReady=cliSta.cli.simReady;
				}
			}
			else if(cliSta.cli.clientState==1)  // Flying
			{
				ClientState_Flying(passedTime,cliSta.cli);
			}
			else if(cliSta.cli.clientState==2)
			{
				ClientState_SideWindow(passedTime,cliSta.cli);
			}
			else if(cliSta.cli.clientState==3)
			{
				ClientState_GhostMode(passedTime,cliSta.cli);
			}
			else // Fatal Error (clientState=-1)
			{
				while(FsInkey()!=FSKEY_NULL)
				{
					FsPollDevice();
				}
				cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING_WAIT_KEYRELEASE;
				cliSta.cli.quit=YSTRUE;
				break;  // 2008/01/02
			}

			if(prevClientState!=0 && cliSta.cli.clientState==0)
			{
				FsPrintClientMenu(this,*cliSta.chooseAirplane,*(cliSta.chooseStartPosition),cliSta.cli.iff);
			}
			else if(prevClientState==0 && cliSta.cli.clientState!=0)
			{
			}

			/* if(cli.dialogInvokeRedraw==YSTRUE && cli.clientState==0)
			{
				cli.dialogInvokeRedraw=YSFALSE;
				FsPrintClientMenu(this,*chooseAirplane,*chooseStartPosition,cli.iff);
			} */

			YSBOOL networkStandby=(cliSta.cli.clientState==0 ? YSTRUE : YSFALSE);

			FSUSERCONTROL userControl;
			switch(cliSta.cli.clientState)
			{
			default:
				userControl=FSUSC_ENABLE;
				break;
			case 2:  // Side-Window of the server
				userControl=FSUSC_DISABLE;
				break;
			case 3:  // Ghost mode
				userControl=FSUSC_VIEWCONTROLONLY;
				break;
			}

			if(passedTime<=1.0)
			{
				SimulateOneStep(passedTime,YSFALSE,YSTRUE,YSFALSE,networkStandby,userControl,YSFALSE);
			}
			else
			{

				SimulateOneStep(1.0,YSFALSE,YSTRUE,YSFALSE,networkStandby,userControl,YSFALSE);
				cliSta.cli.SendQueryAirStateForReducingWarpProblem();
			}

			if(cliSta.netcfg.recordWhenClientMode!=YSTRUE)
			{
				DemoModeRipOffEarlyPartOfRecord();
			}

			if(cliSta.netcfg.recordWhenClientMode!=YSTRUE &&
			   cliSta.netcfg.freeMemoryWhenPossibleClientMode==YSTRUE &&
			   GetNumAirplane()>0)
			{
				NetFreeMemoryWhenPossible(cliSta.cli.netNoActivityTime,passedTime,NULL);
			}

			if(cliSta.cli.IsConnected()!=YSTRUE)
			{
				cliSta.cli.clientState=-1;
				fsConsole.Printf("Disconnected.\n");
			}
			else
			{
				FsAirplane *air=NULL;
				while((air=FindNextAirplane(air))!=NULL)
				{
					air->netNextState.tLocal=-1.0;
					air->netNextState.tRemote=-1.0;
				}
				cliSta.cli.CheckReceive();
				cliSta.cli.SendStateChange();
				cliSta.cli.FlushSendQueue(FS_NETTIMEOUT);
				air=NULL;

				while((air=FindNextAirplane(air))!=NULL)
				{
					if(// air->netType==FSNET_REMOTE &&  << I thought it's a good idea, but even
					   //                                   local airplane must receive an airplane
					   //                                   state once to initialize its position
					   // Also, it may receive the state of a local airplane in response to QueryAirState
					   air->netAlive==YSTRUE &&       // 2002/11/13 anti-ghost
					   air->netNextState.tLocal>air->netPrevState.tLocal &&
					   air->netNextState.tRemote>air->netPrevState.tRemote)
					{
						air->Prop().NetworkDecode(air->netPrevState,air->netNextState);
					}
				}
			}
		}
		if(YSTRUE==cliSta.cli.quit)
		{
			cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING_WAIT_KEYRELEASE;
		}
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		break;


	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING_WAIT_KEYRELEASE:
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		FsPollDevice();
		if(FSKEY_NULL==FsInkey())
		{
			cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING;
		}
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		break;


	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING:
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		cliSta.reportedServerVersion=cliSta.cli.reportedServerVersion;
		if(YSTRUE==cliSta.cli.fatalError)
		{
			cliSta.fatalError=(FsClientRunLoop::CLIENT_FATAL_ERROR)cliSta.cli.fatalErrorCode;
		}

		if(cliSta.netcfg.saveChatLog==YSTRUE && cliSta.cli.chatLogFp.Fp()!=NULL)
		{
			YsString timeStr;
			FsGetTodayTimeString(timeStr);
			fprintf(cliSta.cli.chatLogFp.Fp(),"End Client Mode\n");
			fprintf(cliSta.cli.chatLogFp.Fp(),"%s\n",(const char *)timeStr);
			fprintf(cliSta.cli.chatLogFp.Fp(),"\n");
			fprintf(cliSta.cli.chatLogFp.Fp(),"\n");
		}

		cliSta.cli.Disconnect();
		cliSta.cli.Terminate();

		if(cliSta.cli.fieldNotAvailable==YSTRUE)
		{
			cliSta.fatalError=FsClientRunLoop::CLIENT_FATAL_FIELD_UNAVAILABLE;
		}

		AfterSimulation();

		fsConsole.SetDialog(NULL);

		cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED;

		fsConsole.Printf("\n");
//		fsConsole.Printf("<< Press Key to Close. >>\n");
//		cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING_WAIT_KEYPRESS;


#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
		break;


	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING_WAIT_KEYPRESS:
		FsPollDevice();
		if(FSKEY_NULL!=FsInkey())
		{
			cliSta.runState=FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED;
		}
		fsConsole.Show();
		FsSleep(50);
		break;

	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED:
		FsSleep(10);
		break;
	}
#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif

	{
		FsGuiDialog *modalDlgPtr=nullptr;
		switch(cliSta.cli.choosingMode)
		{
		default:
			break;
		case FsNetworkVariable::CHOOSING_NOTHING:
			modalDlgPtr=cliSta.cliDlg;
			break;
		case FsNetworkVariable::CHOOSING_AIRCRAFT:
			modalDlgPtr=cliSta.chooseAirplane;
			break;
		case FsNetworkVariable::CHOOSING_STARTPOS:
			modalDlgPtr=cliSta.chooseStartPosition;
			break;
		};
		if(nullptr!=modalDlgPtr && cliSta.canvasPtr->GetActiveModalDialog()!=modalDlgPtr)
		{
			cliSta.canvasPtr->AttachModalDialog(modalDlgPtr);
		}
	}

#ifdef CRASHINVESTIGATION
printf("%s %d\n",__FUNCTION__,__LINE__);
#endif
}

void FsSimulation::DrawInClientMode(const class FsClientRunLoop &cliSta) const
{
	switch(cliSta.runState)
	{
	case FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE1:
	case FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE2:
	case FsClientRunLoop::CLIENT_RUNSTATE_INITIALIZE3:
		fsConsole.Show();
		break;
	case FsClientRunLoop::CLIENT_RUNSTATE_LOOP:
		{
			if(cliSta.cli.clientState==0)  // Network Standby
			{
				if(cliSta.cli.choosingMode==0 && (cliSta.cli.nextConsoleUpdateTime<0.0 || YSTRUE==cliSta.cliDlg->NeedRedraw()))
				{
					fsConsole.Show();
					cliSta.cli.nextConsoleUpdateTime=0.5;
				}
			}
			else
			{
				SimDrawAllScreen(YSFALSE,YSFALSE,YSFALSE);
			}
		}
		break;
	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING_WAIT_KEYRELEASE:
		break;
	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING:
		break;

	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATING_WAIT_KEYPRESS:
		fsConsole.Show();
		break;

	case FsClientRunLoop::CLIENT_RUNSTATE_TERMINATED:
		break;
	}
}

FsClientRunLoop::FsClientRunLoop(const char username[],const int port,FsSimulation *sim,FsNetConfig *cfg) :
	cli(username,port,sim,cfg),
	netcfg(*cfg)
{
	runState=CLIENT_RUNSTATE_INITIALIZE1;
	fatalError=CLIENT_FATAL_NOERROR;

	svrAddrLog=new FsServerAddressLog;
	canvasPtr.reset(new FsGuiCanvas);
	cliDlg=new FsGuiClientDialog;
	chooseAirplane=new FsGuiChooseAircraft;
	chooseStartPosition=new FsGuiChooseField;
	opt=new FsGuiChooseAircraftOption;
	this->port=port;
}

FsClientRunLoop::~FsClientRunLoop()
{
	delete svrAddrLog;
	delete cliDlg;
	delete chooseAirplane;
	delete chooseStartPosition;
	delete opt;
}

YSBOOL FsClientRunLoop::NetReady(void) const
{
	if(CLIENT_RUNSTATE_LOOP==runState &&
	   YSTRUE==cli.logOnProcessCompleted)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

FsServerRunLoop::FsServerRunLoop(const char username[],const char fldName[],int netPort,FsSimulation *sim,FsNetConfig *cfg) :
	svr(username,sim,netPort,&*cfg),netcfg(*cfg)
{
	resetCounter=0;

	runState=SERVER_RUNSTATE_INITIALIZE1;
	dispellUser=new FsChoose(16);
	chooseAirplane=new FsGuiChooseAircraft;
	chooseStartPosition=new FsGuiChooseField;
	opt=new FsGuiChooseAircraftOption;
	svrDlg=new FsGuiServerDialog;
	fatalError=SERVER_FATAL_NOERROR;

	this->username=username;
	this->netPort=netPort;

	if(NULL!=fldName)
	{
		this->fldName=fldName;
	}
	else
	{
		this->fldName=netcfg.defField;
	}
}

FsServerRunLoop::~FsServerRunLoop()
{
	delete dispellUser;
	delete chooseAirplane;
	delete chooseStartPosition;
	delete opt;
	delete svrDlg;
}

