#include <ysclass.h>
#include <ysport.h>

#include "fsconfig.h"

#include "fs.h"
#include "fsfilename.h"
#include "fsinstpanel.h"
#include "platform/common/fswindow.h"
#include "graphics/common/fsopengl.h"

#include "fspluginmgr.h"


#include "fstextresource.h"
#include "ysbitmap.h"


#ifndef WIN32  // Assuming UNIX

#include <sys/time.h>
#endif

#include <time.h>



#include "fsgui.h"
#include "fsguiselectiondialogbase.h"
#include <fsguifiledialog.h>

#include "fschoose.h"

#include "graphics/common/fsfontrenderer.h"

#include "fspersona.h"


////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

FsSimInfo::ObjRef::ObjRef()
{
	Initialize();
}

void FsSimInfo::ObjRef::Initialize(void)
{
	objType=FSEX_NULL;
	yfsLabel.Set("");
	yfsIdx=-1;
	objKeyCache=YSNULLHASHKEY;
}

void FsSimInfo::ObjRef::SetObject(const FsExistence *obj)
{
	objKeyCache=obj->SearchKey();
}

void FsSimInfo::ObjRef::SetYfsIdent(FSEXISTENCETYPE objType,int yfsIdent)
{
	this->objType=objType;
	this->yfsIdx=yfsIdent;
	this->yfsLabel="";
	this->objKeyCache=YSNULLHASHKEY;
}

void FsSimInfo::ObjRef::SetYfsLabel(FSEXISTENCETYPE objType,const char yfsLabel[])
{
	this->objType=objType;
	this->yfsIdx=-1;
	this->yfsLabel=yfsLabel;
	this->objKeyCache=YSNULLHASHKEY;
}

FsExistence *FsSimInfo::ObjRef::GetObject(FsSimulation *sim) const
{
	if(YSNULLHASHKEY==objKeyCache)
	{
		FsExistence *obj=NULL;
		if(0<yfsLabel.Strlen())
		{
			if(FSEX_AIRPLANE==objType)
			{
				obj=sim->FindAirplaneByName(yfsLabel);
			}
			else if(FSEX_GROUND==objType)
			{
				obj=sim->FindGroundByTag(yfsLabel);
			}
			else // Unspecified
			{
				obj=sim->FindAirplaneByName(yfsLabel);
				if(NULL==obj)
				{
					obj=sim->FindGroundByTag(yfsLabel);
				}
			}
		}

		if(NULL==obj && 0<=yfsIdx)
		{
			if(FSEX_AIRPLANE==objType)
			{
				obj=sim->FindAirplaneByYsfId(yfsIdx);
			}
			else if(FSEX_GROUND==objType)
			{
				obj=sim->FindGroundByYsfId(yfsIdx);
			}
			else
			{
				obj=sim->FindAirplaneByYsfId(yfsIdx);
				if(NULL==obj)
				{
					obj=sim->FindGroundByYsfId(yfsIdx);
				}
			}
		}

		if(NULL!=obj)
		{
			objKeyCache=obj->SearchKey();
		}

		return obj;
	}
	else
	{
		return sim->FindObject(objKeyCache);
	}
}

const FsExistence *FsSimInfo::ObjRef::GetObject(const FsSimulation *sim) const
{
	if(YSNULLHASHKEY==objKeyCache)
	{
		FsExistence *obj=NULL;
		if(0<yfsLabel.Strlen())
		{
			if(FSEX_AIRPLANE==objType)
			{
				obj=sim->FindAirplaneByName(yfsLabel);
			}
			else if(FSEX_GROUND==objType)
			{
				obj=sim->FindGroundByTag(yfsLabel);
			}
			else // Unspecified
			{
				obj=sim->FindAirplaneByName(yfsLabel);
				if(NULL==obj)
				{
					obj=sim->FindGroundByTag(yfsLabel);
				}
			}
		}

		if(NULL==obj && 0<=yfsIdx)
		{
			if(FSEX_AIRPLANE==objType)
			{
				obj=sim->FindAirplaneByYsfId(yfsIdx);
			}
			else if(FSEX_GROUND==objType)
			{
				obj=sim->FindGroundByYsfId(yfsIdx);
			}
			else
			{
				obj=sim->FindAirplaneByYsfId(yfsIdx);
				if(NULL==obj)
				{
					obj=sim->FindGroundByYsfId(yfsIdx);
				}
			}
		}

		if(NULL!=obj)
		{
			objKeyCache=obj->SearchKey();
		}

		return obj;
	}
	else
	{
		return sim->FindObject(objKeyCache);
	}
}

FsAirplane *FsSimInfo::AirRef::GetAircraft(FsSimulation *sim) const
{
	if(objKeyCache==YSNULLHASHKEY)
	{
		FsAirplane *air=NULL;
		if(0<yfsLabel.Strlen())
		{
			air=sim->FindAirplaneByName(yfsLabel);
		}
		else if(0<=yfsIdx)
		{
			air=sim->FindAirplaneByYsfId(yfsIdx);
		}
		if(NULL!=air)
		{
			objKeyCache=FsExistence::GetSearchKey(air);
		}
		return air;
	}
	else
	{
		return sim->FindAirplane(objKeyCache);
	}
}



////////////////////////////////////////////////////////////

FsMissionGoal::FsMissionGoal()
{
	Initialize();
}

void FsMissionGoal::Initialize()
{
	isActiveMission=YSTRUE;  // Default on, but it is turned off in the default goal of FsSimulation.

	goalFlag=0;
	duration=0.0;
	numPrmGndMustSurvive=0;
	numPrmGndMustBeDestroyed=0;
	defendGndName.Clear();
	defendAirName.Clear();
	destroyGndName.Clear();
	destroyAirName.Clear();
	mustLandAirName.Clear();
	text.Set(0,NULL);
	iText.CleanUp();
	pngFn.Set(NULL);
	base64png.CleanUp();
}

const YsArray <YsString> &FsMissionGoal::GetText(void) const
{
	return text;
}
const YsArray <FsInternationalText> &FsMissionGoal::GetInternationalText(void) const
{
	return iText;
}

const YsArray <YsString> &FsMissionGoal::GetBase64Png(int pngIdx) const
{
	if(0==pngIdx)
	{
		return base64png;
	}
	else
	{
		return emptyBase64png;
	}
}

void FsMissionGoal::SetIsActiveMission(YSBOOL isActiveMissionIn)
{
	isActiveMission=isActiveMissionIn;
}

const YSBOOL FsMissionGoal::IsActiveMission(void) const
{
	return isActiveMission;
}

YSRESULT FsMissionGoal::WriteFile(FILE *fp)
{
	int i;
	for(i=0; i<text.GetN(); i++)
	{
		fprintf(fp,"MSSNGOAL TEXT \"%s\"\n",(const char *)text[i]);
	}

	for(auto &msg : iText)
	{
		YsString utf8;
		utf8.EncodeUTF8(msg.GetText());
		fprintf(fp,"MSSNGOAL ITEXT %s \"%s\"\n",(const char *)msg.GetLanguageCode(),(const char *)utf8);
	}

	if(pngFn[0]!=0)
	{
		YsString utf8;
		utf8.EncodeUTF8 <wchar_t> (pngFn);
		fprintf(fp,"MSSNGOAL STARTPNG \"%s\"\n",utf8.Txt());
	}

	for(auto &str : base64png)
	{
		fprintf(fp,"MSSNGOAL BASE64PNG 0 %s\n",str.Txt());
	}

	if(goalFlag&FSGOAL_SURVIVE)
	{
		fprintf(fp,"MSSNGOAL SURVIVE\n");
	}
	if(goalFlag&FSGOAL_LAND)
	{
		fprintf(fp,"MSSNGOAL LAND");
		if(landRegionName.Strlen()>0)
		{
			fprintf(fp," RGN \"%s\"",(const char *)landRegionName);
		}
		if(landCarrierName.Strlen()>0)
		{
			fprintf(fp," CARRIER \"%s\"",(const char *)landCarrierName);
		}
		fprintf(fp,"\n");
	}

	if(goalFlag&FSGOAL_DEFENDPRMGND)
	{
		fprintf(fp,"MSSNGOAL DEFPRMGND %d\n",numPrmGndMustSurvive);
	}

	if(goalFlag&FSGOAL_DEFENDGND)
	{
		forYsArray(i,defendGndName)
		{
			if(defendGndName[i].Strlen()>0)
			{
				fprintf(fp,"MSSNGOAL DEFGND");
				fprintf(fp," \"%s\"",(const char *)defendGndName[i]);
				fprintf(fp,"\n");
			}
		}
	}

	if(goalFlag&FSGOAL_DEFENDAIR)
	{
		forYsArray(i,defendAirName)
		{
			if(defendAirName[i].Strlen()>0)
			{
				fprintf(fp,"MSSNGOAL DEFAIR");
				fprintf(fp," \"%s\"",(const char *)defendAirName[i]);
				fprintf(fp,"\n");
			}
		}
	}

	if(goalFlag&FSGOAL_DESTROYPRMGND)
	{
		fprintf(fp,"MSSNGOAL DESTROYPRMGND %d\n",numPrmGndMustBeDestroyed);
	}

	if(goalFlag&FSGOAL_DESTROYGND)
	{
		forYsArray(i,destroyGndName)
		{
			if(destroyGndName[i].Strlen()>0)
			{
				fprintf(fp,"MSSNGOAL DESTROYGND");
				fprintf(fp," \"%s\"",(const char *)destroyGndName[i]);
				fprintf(fp,"\n");
			}
		}
	}

	if(goalFlag&FSGOAL_DESTROYAIR)
	{
		forYsArray(i,destroyAirName)
		{
			if(destroyAirName[i].Strlen()>0)
			{
				fprintf(fp,"MSSNGOAL DESTROYAIR");
				fprintf(fp," \"%s\"",(const char *)destroyAirName[i]);
				fprintf(fp,"\n");
			}
		}
	}

	if(goalFlag&FSGOAL_MUSTLAND)
	{
		forYsArray(i,mustLandAirName)
		{
			if(mustLandAirName[i].airLabel.Strlen()>0)
			{
				fprintf(fp,"MSSNGOAL MUSTLAND \"%s\"",mustLandAirName[i].airLabel.Txt());
				if(0<mustLandAirName[i].base.GetName().Strlen())
				{
					auto &base=mustLandAirName[i].base;
					fprintf(fp," %s \"%s\"",FsSimInfo::BaseTypeToString(base.GetType()),base.GetName().Txt());
				}
				fprintf(fp,"\n");
			}
		}
	}

	if(goalFlag&FSGOAL_DESTROYALLAIR)
	{
		fprintf(fp,"MSSNGOAL DESTROYALLAIR\n");
	}

	if(duration>YsTolerance)
	{
		fprintf(fp,"MSSNGOAL DURATION %.2lf\n",duration);
	}

	return YSOK;
}

YSRESULT FsMissionGoal::SendCommand(const char str[],const wchar_t relPath[])
{
	YsString buf;
	YsArray <YsString,16> args;
	buf.Set(str);
	if(buf.Arguments(args)==YSOK)
	{
		return SendCommand(args.GetN(),args,relPath);
	}
	return YSERR;
}

YSRESULT FsMissionGoal::SendCommand(YSSIZE_T ac,YsString av[],const wchar_t relPath[])
{
	av[0].Capitalize();
	if(ac>=2 && strcmp(av[0],"MSSNGOAL")==0)
	{
		isActiveMission=YSTRUE;

		av[1].Capitalize();
		if(strcmp(av[1],"SURVIVE")==0)
		{
			goalFlag|=FSGOAL_SURVIVE;
			return YSOK;
		}
		else if(strcmp(av[1],"LAND")==0)
		{
			int i;
			goalFlag|=FSGOAL_LAND;
			landRegionName.Set("");
			landCarrierName.Set("");
			for(i=2; i<ac-1; i+=2)
			{
				av[i].Capitalize();
				if(strcmp(av[i],"RGN")==0)
				{
					landRegionName.Set(av[i+1]);
				}
				else if(strcmp(av[i],"CARRIER")==0)
				{
					landCarrierName.Set(av[i+1]);
				}
			}
			return YSOK;
		}
		else if(strcmp(av[1],"DEFPRMGND")==0)
		{
			if(ac>=3)
			{
				goalFlag|=FSGOAL_DEFENDPRMGND;
				numPrmGndMustSurvive=atoi(av[2]);
				return YSOK;
			}
		}
		else if(strcmp(av[1],"DEFGND")==0)
		{
			if(ac>=3)
			{
				goalFlag|=FSGOAL_DEFENDGND;
				defendGndName.Increment();
				defendGndName.GetEnd().Set(av[2]);
				return YSOK;
			}
		}
		else if(strcmp(av[1],"DEFAIR")==0)
		{
			if(ac>=3)
			{
				goalFlag|=FSGOAL_DEFENDAIR;
				defendAirName.Increment();
				defendAirName.GetEnd().Set(av[2]);
				return YSOK;
			}
		}
		else if(strcmp(av[1],"DESTROYPRMGND")==0)
		{
			if(ac>=3)
			{
				goalFlag|=FSGOAL_DESTROYPRMGND;
				numPrmGndMustBeDestroyed=atoi(av[2]);
				return YSOK;
			}
		}
		else if(strcmp(av[1],"DESTROYGND")==0)
		{
			if(ac>=3)
			{
				goalFlag|=FSGOAL_DESTROYGND;
				destroyGndName.Increment();
				destroyGndName.GetEnd().Set(av[2]);
				return YSOK;
			}
		}
		else if(strcmp(av[1],"DESTROYAIR")==0)
		{
			if(ac>=3)
			{
				goalFlag|=FSGOAL_DESTROYAIR;
				destroyAirName.Increment();
				destroyAirName.GetEnd().Set(av[2]);
				return YSOK;
			}
		}
		else if(strcmp(av[1],"MUSTLAND")==0)
		{
			if(ac>=3)
			{
				goalFlag|=FSGOAL_MUSTLAND;
				mustLandAirName.Increment();
				mustLandAirName.Last().airLabel=av[2];
				mustLandAirName.Last().base.CleanUp();
				if(5<=ac)
				{
					mustLandAirName.Last().base.SetType(FsSimInfo::StringToBaseType(av[3]));
					mustLandAirName.Last().base.SetName(av[4]);
				}
				return YSOK;
			}
		}
		else if(strcmp(av[1],"DESTROYALLAIR")==0)
		{
			goalFlag|=FSGOAL_DESTROYALLAIR;
			return YSOK;
		}
		else if(strcmp(av[1],"DURATION")==0)
		{
			if(ac>=3)
			{
				duration=atof(av[2]);
				return YSOK;
			}
		}
		else if(strcmp(av[1],"TEXT")==0)
		{
			YsString str;
			str.Set(av[2]);
			text.Append(str);
			return YSOK;
		}
		else if(strcmp(av[1],"ITEXT")==0)
		{
			YsWString wMsg;
			wMsg.SetUTF8String(av[3]);
			iText.Increment();
			iText.Last().SetText(av[2],wMsg);
			return YSOK;
		}
		else if(strcmp(av[1],"STARTPNG")==0)
		{
			if(av[2][0]!=0 && (av[2][0]=='/' || av[2][0]=='\\' || av[2][1]==':'))
			{
				pngFn.SetUTF8String(av[2]);
			}
			else
			{
				pngFn.Set(relPath);
				if(pngFn.LastChar()!='/' && pngFn.LastChar()!='\\' && pngFn.LastChar()!=':')
				{
					pngFn.Append('/');
				}

				YsWString fn;
				fn.SetUTF8String(av[2]);
				pngFn.Append(fn);
			}

			YsString cStr;
			YsUnicodeToSystemEncoding(cStr,pngFn);
			printf("STARTPNG=%s\n",cStr.Txt());
			return YSOK;
		}
		else if(0==strcmp(av[1],"BASE64PNG") && 4<=ac)
		{
			const int pngId=atoi(av[2]);
			if(0==pngId)
			{
				base64png.Append(av[3]);
			}
			return YSOK;
		}
	}
	return YSERR;
}

YSBOOL FsMissionGoal::TestAllMissionGoalIsSatisfied(const FsSimulation *sim) const
{
	if(TestMissionDuration(sim)==YSTRUE)
	{
		if((goalFlag&FSGOAL_SURVIVE)!=0 && TestSurvive(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		if((goalFlag&FSGOAL_LAND)!=0 && TestLanding(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		int nGndSurvive;
		if((goalFlag&FSGOAL_DEFENDPRMGND)!=0 && TestDefendPrimaryGround(nGndSurvive,sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		if((goalFlag&FSGOAL_DEFENDGND)!=0 && TestDefendGround(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		if((goalFlag&FSGOAL_DEFENDAIR)!=0 && TestDefendAir(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		int nGndKill;
		if((goalFlag&FSGOAL_DESTROYPRMGND)!=0 && TestDestroyPrimaryGround(nGndKill,sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		if((goalFlag&FSGOAL_DESTROYGND)!=0 && TestDestroyGround(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		if((goalFlag&FSGOAL_DESTROYAIR)!=0 && TestDestroyAir(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		if((goalFlag&FSGOAL_DESTROYALLAIR)!=0 && TestDestroyAllAir(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		if((goalFlag&FSGOAL_MUSTLAND)!=0 && TestMustLandAir(sim)!=YSTRUE)
		{
			return YSFALSE;
		}

		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsMissionGoal::TestMissionDuration(const FsSimulation *sim) const
{
	double lastRecTime;
	lastRecTime=sim->LastRecordedTime();

	// printf("Needed Duration %lf Last Recorded Time %lf\n",duration,lastRecTime);

	if(duration<=lastRecTime)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsMissionGoal::TestSurvive(const FsSimulation *sim) const
{
	const FsAirplane *air;
	air=sim->GetPlayerAirplane();
	if(air!=NULL)
	{
		FSFLIGHTSTATE sta;
		sta=air->GetFinalState();
		if(sta!=FSDEAD && sta!=FSDEADFLATSPIN && sta!=FSDEADSPIN)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsMissionGoal::TestLanding(const FsSimulation *sim) const
{
	const FsAirplane *air;
	air=sim->GetPlayerAirplane();
	if(air!=NULL && air->GetFinalState()==FSGROUNDSTATIC)
	{
		YSBOOL region,carrier;
		region=YSFALSE;
		carrier=YSFALSE;
		if(landRegionName[0]!=0)
		{
			int i;
			const FsField *ptr;
			YsVec3 pos;
			YsArray <const YsSceneryRectRegion *,16> rgn;

			air->GetFinalPosition(pos);

			ptr=sim->GetField();
			if(ptr->GetFieldRegion(rgn,pos.x(),pos.z())==YSOK)  // 2003/01/04
			{
				for(i=0; i<rgn.GetN(); i++)
				{
					if(strcmp(rgn[i]->GetTag(),landRegionName)==0)
					{
						region=YSTRUE;
						break;
					}
				}
			}
		}
		else
		{
			region=YSTRUE;
		}

		if(landCarrierName[0]!=0)
		{
			int i,nCarrier;
			const FsGround *gnd;
			YsVec3 airPos;
			air->GetFinalPosition(airPos);

			nCarrier=sim->GetNumAircraftCarrier();
			for(i=0; i<nCarrier; i++)
			{
				gnd=sim->GetAircraftCarrier(i);
				if(strcmp(gnd->name,landCarrierName)==0)
				{
					const FsAircraftCarrierProperty *carProp;

					carProp=gnd->Prop().GetAircraftCarrierProperty();
					if(carProp!=NULL)
					{
						YsVec3 loc;
						YsVec3 carPos;
						YsAtt3 carAtt;
						gnd->GetFinalPosition(carPos);
						gnd->GetFinalAttitude(carAtt);
						loc=airPos-carPos;
						carAtt.MulInverse(loc,loc);
						if(carProp->IsOnDeckLocal(loc)==YSTRUE)
						{
							carrier=YSTRUE;
							break;
						}
					}
				}
			}
		}
		else
		{
			carrier=YSTRUE;
		}

		if(region==YSTRUE && carrier==YSTRUE)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsMissionGoal::TestDefendPrimaryGround(int &nSurvive,const FsSimulation *sim) const
{
	const FsGround *gnd;
	const FsExistence *player;

	player=sim->GetPlayerObject();
	gnd=NULL;
	nSurvive=0;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->iff==player->iff && gnd->primaryTarget==YSTRUE)
		{
			FSGNDSTATE sta;
			sta=gnd->GetFinalState();
			if(sta!=FSGNDDEAD)
			{
				nSurvive++;
			}
		}
	}

	if(nSurvive>=numPrmGndMustSurvive)
	{
		return YSTRUE;
	}

	return YSFALSE;
}

YSBOOL FsMissionGoal::TestDefendGround(const FsSimulation *sim) const
{
	const FsGround *gnd;
	int i,nTest;

	nTest=0;
	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		forYsArray(i,defendGndName)
		{
			if(strcmp(gnd->name,defendGndName[i])==0)
			{
				nTest++;
				if(gnd->GetFinalState()==FSGNDDEAD)
				{
					return YSFALSE;
				}
			}
		}
	}

	if(nTest>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsMissionGoal::TestDefendAir(const FsSimulation *sim) const
{
	const FsAirplane *air;
	int i,nTest;

	nTest=0;
	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		forYsArray(i,defendAirName)
		{
			if(strcmp(air->name,defendAirName[i])==0)
			{
				FSFLIGHTSTATE sta;
				sta=air->GetFinalState();
				nTest++;
				if(sta==FSDEAD || sta==FSDEADFLATSPIN || sta==FSDEADSPIN)
				{
					return YSFALSE;
				}
			}
		}
	}

	if(nTest>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsMissionGoal::TestDestroyPrimaryGround(int &nKill,const FsSimulation *sim) const
{
	const FsGround *gnd;
	const FsExistence *player;

	player=sim->GetPlayerObject();
	gnd=NULL;
	nKill=0;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->iff!=player->iff && gnd->primaryTarget==YSTRUE)
		{
			FSGNDSTATE sta;
			sta=gnd->GetFinalState();
			if(sta==FSGNDDEAD)
			{
				nKill++;
			}
		}
	}

	if(nKill>=numPrmGndMustBeDestroyed)
	{
		return YSTRUE;
	}

	return YSFALSE;
}

YSBOOL FsMissionGoal::TestDestroyGround(const FsSimulation *sim) const
{
	const FsGround *gnd;

	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		int i;
		forYsArray(i,destroyGndName)
		{
			if(strcmp(gnd->name,destroyGndName[i])==0 && gnd->GetFinalState()!=FSGNDDEAD)
			{
				return YSFALSE;
			}
		}
	}

	return YSTRUE;
}

YSBOOL FsMissionGoal::TestDestroyAir(const FsSimulation *sim) const
{
	const FsAirplane *air;

	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		int i;
		forYsArray(i,destroyAirName)
		{
			if(strcmp(air->name,destroyAirName[i])==0)
			{
				FSFLIGHTSTATE sta;
				sta=air->GetFinalState();
				if(sta!=FSDEAD && sta!=FSDEADFLATSPIN && sta!=FSDEADSPIN)
				{
					return YSFALSE;
				}
			}
		}
	}
	return YSTRUE;
}

YSBOOL FsMissionGoal::TestDestroyAllAir(const FsSimulation *sim) const
{
	const FsAirplane *air;
	const FsExistence *player;

	player=sim->GetPlayerObject();
	air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		if(air->iff!=player->iff)
		{
			FSFLIGHTSTATE sta;
			sta=air->GetFinalState();
			if(sta!=FSDEAD && sta!=FSDEADFLATSPIN && sta!=FSDEADSPIN)
			{
				return YSFALSE;
			}
		}
	}

	return YSTRUE;
}

YSBOOL FsMissionGoal::TestMustLandAir(const FsSimulation *sim) const
{
	auto scn=sim->GetField()->GetFieldPtr();

	const FsAirplane *air=NULL;
	while((air=sim->FindNextAirplane(air))!=NULL)
	{
		int i;
		forYsArray(i,mustLandAirName)
		{
			if(strcmp(air->name,mustLandAirName[i].airLabel)==0)
			{
				if(air->GetFinalState()!=FSGROUNDSTATIC && air->GetFinalState()!=FSGROUND)
				{
					return YSFALSE;
				}

				if(0<mustLandAirName[i].base.GetName().Strlen())
				{
					YsVec3 finalPos;
					air->GetFinalPosition(finalPos);

					auto &base=mustLandAirName[i].base;
					if(base.GetType()==FsSimInfo::AIRPORT)
					{
						YsArray <const YsSceneryRectRegion *> rgnArray;
						if(YSOK==scn->SearchRegionByTag(rgnArray,base.GetName()) && 0<rgnArray.GetN())
						{
							YSBOOL isInside=YSFALSE;
							for(auto rgn : rgnArray)
							{
								if(YSTRUE==scn->IsInsideRectRegion(finalPos,rgn))
								{
									isInside=YSTRUE;
									break;
								}
							}
							if(YSTRUE!=isInside)
							{
								return YSFALSE;
							}
						}
					}
					else if(base.GetType()==FsSimInfo::CARRIER)
					{
						auto *carrier=air->Prop().OnThisCarrier();
						if(NULL==carrier || 0!=strcmp(carrier->name,base.GetName()))
						{
							return YSFALSE;
						}
					}
				}
			}
		}
	}
	return YSTRUE;
}

////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////

FsSimInfo::AirBase::AirBase()
{
	baseType=AIRPORT;
}
void FsSimInfo::AirBase::CleanUp(void)
{
	baseType=AIRPORT;
	baseName.Set("");
	Decache();
}
void FsSimInfo::AirBase::SetName(const YsString &baseName)
{
	this->baseName=baseName;
}
void FsSimInfo::AirBase::SetType(BASE_TYPE baseType)
{
	this->baseType=baseType;
}
const YsString &FsSimInfo::AirBase::GetName(void) const
{
	return this->baseName;
}
FsSimInfo::BASE_TYPE FsSimInfo::AirBase::GetType(void) const
{
	return baseType;
}

YSBOOL FsSimInfo::AirBase::IsCached(void) const
{
	return cached;
}
YSRESULT FsSimInfo::AirBase::Encache(const FsSimulation *sim) const
{
	if(YSTRUE==cached)
	{
		return YSOK;
	}
	if(YSOK!=cacheError)
	{
		return YSERR;
	}

	cacheError=YSOK;
	if(FsSimInfo::AIRPORT==baseType)
	{
		auto fld=sim->GetField();
		if(NULL!=fld)
		{
			auto scn=fld->GetFieldPtr();
			YsArray <const YsSceneryRectRegion *> rgnArray;
			if(NULL!=scn && YSOK==scn->SearchRegionByTag(rgnArray,GetName()) && 0<rgnArray.GetN())
			{
				airportCache=rgnArray[0];
				carrierKeyCache=YSNULLHASHKEY;
				cached=YSTRUE;
				cacheError=YSOK;
				return YSOK;
			}
		}
	}
	else if(FsSimInfo::CARRIER==baseType)
	{
		auto carrierPtr=sim->FindIlsOrCarrierByTag(GetName());
		if(NULL!=carrierPtr)
		{
			airportCache=NULL;
			carrierKeyCache=carrierPtr->SearchKey();
			cached=YSTRUE;
			cacheError=YSOK;
			return YSOK;
		}
	}

	cached=YSFALSE;
	cacheError=YSERR;
	return YSERR;
}
void FsSimInfo::AirBase::Decache(void) const
{
	cached=YSFALSE;
	cacheError=YSOK;
	carrierKeyCache=YSNULLHASHKEY;
	airportCache=NULL;
}
FsGround *FsSimInfo::AirBase::GetCachedCarrier(const FsSimulation *sim) const
{
	if(YSTRUE==cached)
	{
		return sim->FindGround(carrierKeyCache);
	}
	return NULL;
}
const YsSceneryRectRegion *FsSimInfo::AirBase::GetCachedAirportRect(const FsSimulation *) const
{
	if(YSTRUE==cached)
	{
		return airportCache;
	}
	return NULL;
}


////////////////////////////////////////////////////////////

FsSimulationEvent::FsSimulationEvent()
{
	Initialize();
}

void FsSimulationEvent::Initialize(void)
{
	eventTime=0.0;
	eventType=FSEVENT_NULLEVENT;
	eventFlag=0;

	objKey=0;
	str.Set("");
	weaponCfg.Set(0,NULL);
	cloudLayer.Set(0,NULL);

	env=FSDAYLIGHT;

	visibility=20000.0;
	wind=YsOrigin();
}

void FsSimulationEvent::Save(FILE *fp,const class FsSimulation *sim) const
{
	int i;

	switch(eventType)
	{
	case FSEVENT_TEXTMESSAGE:
		fprintf(fp,"TXTEVT %lf %d\n",eventTime,eventFlag);
		fprintf(fp,"TXT %s\n",str.Txt());
		fprintf(fp,"ENDEVT\n");
		break;
	case FSEVENT_PLAYEROBJCHANGE:
		{
			const FsExistence *player=sim->FindObject(objKey);
			if(NULL!=player)
			{
				fprintf(fp,"PLRAIR %lf %d\n",eventTime,eventFlag);
				fprintf(fp,"OBJID %d\n",player->ysfId);
				fprintf(fp,"ENDEVT\n");
			}
		}
		break;
	case FSEVENT_AIRCOMMAND:
		{
			const FsAirplane *air=sim->FindAirplane(objKey);
			if(air!=NULL)
			{
				fprintf(fp,"AIRCMD %lf %d\n",eventTime,eventFlag);
				fprintf(fp,"AIRID %d\n",air->ysfId);
				fprintf(fp,"TXT %s\n",str.Txt());
				fprintf(fp,"ENDEVT\n");
			}
		}
		break;
	case FSEVENT_SETWEAPONCONFIG:
		{
			const FsAirplane *air=sim->FindAirplane(objKey);
			if(air!=NULL)
			{
				fprintf(fp,"WPNCFG %lf %d\n",eventTime,eventFlag);
				fprintf(fp,"AIRID %d\n",air->ysfId);
				for(i=0; i<=weaponCfg.GetN()-2; i+=2)
				{
					const char *wpnName;
					wpnName=FsGetWeaponString((FSWEAPONTYPE)weaponCfg[i]);
					if(wpnName!=NULL)
					{
						fprintf(fp,"CFG %s %d\n",wpnName,weaponCfg[i+1]);
					}
				}
				fprintf(fp,"ENDEVT\n");
			}
		}
		break;
	case FSEVENT_ENVIRONMENTCHANGE:
		fprintf(fp,"ENVCHG %lf %d\n",eventTime,eventFlag);
		switch(env)
		{
		case FSDAYLIGHT:
			fprintf(fp,"ENV DAY\n");
			break;
		case FSNIGHT:
			fprintf(fp,"ENV NIGHT\n");
			break;
		}
		fprintf(fp,"ENDEVT\n");
		break;
	case FSEVENT_VISIBILITYCHANGE:
		fprintf(fp,"VISCHG %lf %d\n",eventTime,eventFlag);
		fprintf(fp,"VISI %lfm\n",visibility);
		fprintf(fp,"ENDEVT\n");
		break;
	case FSEVENT_WINDCHANGE:
		fprintf(fp,"WNDCHG %lf %d\n",eventTime,eventFlag);
		fprintf(fp,"WIND %lfm/s %lfm/s %lfm/s\n",wind.x(),wind.y(),wind.z());
		fprintf(fp,"ENDEVT\n");
		break;
	case FSEVENT_CLOUDLAYERCHANGE:
		fprintf(fp,"CLDCHG %lf %d\n",eventTime,eventFlag);
		forYsArray(i,cloudLayer)
		{
			fprintf(fp,"CLDLYR %s %lfm %lfm\n",
			    FsWeatherCloudLayer::CloudLayerTypeString(cloudLayer[i].cloudLayerType),
			    cloudLayer[i].y0,
			    cloudLayer[i].y1);
		}
		fprintf(fp,"ENDEVT\n");
		break;
	case FSEVENT_GNDCOMMAND:
		{
			const FsGround *gnd=sim->FindGround(objKey);
			if(gnd!=NULL)
			{
				fprintf(fp,"GNDCMD %lf %d\n",eventTime,eventFlag);
				fprintf(fp,"OBJID %d\n",gnd->ysfId);
				fprintf(fp,"TXT %s\n",str.Txt());
				fprintf(fp,"ENDEVT\n");
			}
		}
		break;
	}
}

void FsSimulationEvent::Load(FILE *fp,const FsSimulation * /*sim*/)
{
	YsString str;
	YsArray <YsString,16> args;
	int i;

	while(str.Fgets(fp)!=NULL)
	{
		str.Arguments(args);

		if(args.GetN()>0)
		{
			args[0].Capitalize();
			if(strcmp(args[0],"ENDEVT")==0)
			{
				break;
			}
			else if(strcmp(args[0],"TXT")==0 && str.Strlen()>=4)
			{
				this->str.Set(str.Txt()+4);
			}
			else if((0==strcmp(args[0],"AIRID") || 0==strcmp(args[0],"OBJID")) && args.GetN()>=2)
			{
				objKey=atoi(args[1]);  // Tentative.  YsfId should be re-assigned in MatchAirGndYfsId function.
			}
			else if(strcmp(args[0],"CFG")==0 && args.GetN()>=3)
			{
				for(i=1; i<=args.GetN()-2; i+=2)
				{
					FSWEAPONTYPE wpnType;
					wpnType=FsGetWeaponTypeByString(args[i]);
					if(wpnType!=FSWEAPON_NULL)
					{
						int cfg[2];
						cfg[0]=(int)wpnType;
						cfg[1]=atoi(args[i+1]);
						weaponCfg.Append(2,cfg);
					}
				}
			}
			else if(strcmp(args[0],"ENV")==0 && args.GetN()>=2)
			{
				args[1].Capitalize();
				if(strcmp(args[1],"DAY")==0)
				{
					env=FSDAYLIGHT;
				}
				else if(strcmp(args[1],"NIGHT")==0)
				{
					env=FSNIGHT;
				}
			}
			else if(strcmp(args[0],"WIND")==0 && args.GetN()>=4)
			{
				double x,y,z;
				if(FsGetSpeed(x,args[1])==YSOK && FsGetSpeed(y,args[2])==YSOK && FsGetSpeed(z,args[3])==YSOK)
				{
					wind.Set(x,y,z);
				}
			}
			else if(strcmp(args[0],"VISI")==0 && args.GetN()>=2)
			{
				double v;
				if(FsGetLength(v,args[1])==YSOK)
				{
					visibility=v;
				}
			}
			else if(strcmp(args[0],"CLDLYR")==0 && args.GetN()>=4)
			{
				FsWeatherCloudLayer lyr;
				lyr.cloudLayerType=FsWeatherCloudLayer::CloudLayerTypeFromString(args[1]);
				FsGetLength(lyr.y0,args[2]);
				FsGetLength(lyr.y1,args[3]);
				cloudLayer.Append(lyr);
			}
		}
	}
}

////////////////////////////////////////////////////////////

FsSimulationEventStore::FsSimulationEventStore()
{
	nextEvent=0;
}

void FsSimulationEventStore::CleanUp(void)
{
	nextEvent=0;
	eventList.Set(0,NULL);
}

void FsSimulationEventStore::Rewind(void)
{
	nextEvent=0;
}

void FsSimulationEventStore::AddEvent(const double &ctime,const FsSimulationEvent &newEvt)
{


	eventList.Append(newEvt);
	eventList.Last().eventTime=ctime;
	eventList.Last().eventFlag|=FSEVENTFLAG_UNSORTED;
}

void FsSimulationEventStore::SortEventByTime(void)
{
	if(eventList.GetN()>0)
	{
		int i;
		YsArray <double> timeList;
		timeList.Set(eventList.GetN(),NULL);

		forYsArray(i,eventList)
		{
			timeList[i]=eventList[i].eventTime;
			eventList[i].eventFlag&=(~FSEVENTFLAG_UNSORTED);
		}

		YsQuickSort <double,FsSimulationEvent> (timeList.GetN(),timeList,eventList);

		Rewind();
	}
}

void FsSimulationEventStore::DeleteFutureEventForResume(const double currentTime)
{
	for(YSSIZE_T evtIdx=0; evtIdx<eventList.GetN(); ++evtIdx)
	{
		if(currentTime<=eventList[evtIdx].eventTime &&
		   FSEVENT_PLAYEROBJCHANGE==eventList[evtIdx].eventType)
		{
			eventList[evtIdx].eventType=FSEVENT_NULLEVENT;
		}
	}
}

void FsSimulationEventStore::DeleteEventByTypeAll(int eventType)
{
	for(YSSIZE_T evtIdx=0; evtIdx<eventList.GetN(); ++evtIdx)
	{
		if(eventType==eventList[evtIdx].eventType)
		{
			eventList[evtIdx].eventType=FSEVENT_NULLEVENT;
		}
	}
}

void FsSimulationEventStore::SeekNextEventPointereToCurrentTime(const double currentTime)
{
	for(nextEvent=0; eventList[nextEvent].eventTime<currentTime; ++nextEvent)
	{
	}
}

void FsSimulationEventStore::Save(FILE *fp,const FsSimulation *sim) const
{
	int i;
	if(eventList.GetN()>0)
	{
		fprintf(fp,"EVTBLOCK\n");
		forYsArray(i,eventList)
		{
			eventList[i].Save(fp,sim);
		}
		fprintf(fp,"EDEVTBLK\n");
	}
}

void FsSimulationEventStore::Load(FILE *fp,const FsSimulation *sim)
{
	YsString str;
	FsSimulationEvent toAdd;
	YsArray <YsString,16> args;

	while(str.Fgets(fp)!=NULL)
	{
		str.Arguments(args);
		if(args.GetN()>0)
		{
			if(strcmp(args[0],"EDEVTBLK")==0)
			{
				break;
			}
			else if(strcmp(args[0],"TXTEVT")==0)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_TEXTMESSAGE;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if(strcmp(args[0],"PLRAIR")==0)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_PLAYEROBJCHANGE;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if(strcmp(args[0],"AIRCMD")==0)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_AIRCOMMAND;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if(strcmp(args[0],"GNDCMD")==0)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_GNDCOMMAND;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if(strcmp(args[0],"WPNCFG")==0 && args.GetN()>=3)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_SETWEAPONCONFIG;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if(strcmp(args[0],"ENVCHG")==0 && args.GetN()>=3)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_ENVIRONMENTCHANGE;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if(strcmp(args[0],"VISCHG")==0 && args.GetN()>=3)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_VISIBILITYCHANGE;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if(strcmp(args[0],"WNDCHG")==0 && args.GetN()>=3)
			{
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_WINDCHANGE;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
			else if((strcmp(args[0],"CLDCHD")==0 || strcmp(args[0],"CLDCHG")==0) && args.GetN()>=3)
			{
				// Misspelled as CLDCHD until 2012/05/03
				toAdd.Initialize();
				toAdd.eventType=FSEVENT_CLOUDLAYERCHANGE;
				toAdd.eventTime=atof(args[1]);
				toAdd.eventFlag=atoi(args[2]);
				toAdd.Load(fp,sim);
				eventList.Append(toAdd);
			}
		}
	}
}

void FsSimulationEventStore::MatchAirGndYfsId(const FsSimulation *sim)
{
	const FsAirplane *air;
	const FsGround *gnd;
	YsHashTable <const FsExistence *> ysfIdToObjPtr((sim->GetNumAirplane()/4)+1);

	air=NULL;
	while(NULL!=(air=sim->FindNextAirplane(air)))
	{
		if(air->ysfId>0)
		{
			ysfIdToObjPtr.AddElement(air->ysfId,air);
		}
	}
	gnd=NULL;
	while(NULL!=(gnd=sim->FindNextGround(gnd)))
	{
		if(0<gnd->ysfId)
		{
			ysfIdToObjPtr.AddElement(gnd->ysfId,gnd);
		}
	}

	int i;
	forYsArray(i,eventList)
	{
		switch(eventList[i].eventType)
		{
		case FSEVENT_AIRCOMMAND:
		case FSEVENT_GNDCOMMAND:
		case FSEVENT_SETWEAPONCONFIG:
		case FSEVENT_PLAYEROBJCHANGE:
			{
				const FsExistence *obj;
				if(ysfIdToObjPtr.FindElement(obj,eventList[i].objKey)==YSOK)
				{
					eventList[i].objKey=obj->SearchKey();
				}
				else
				{
					eventList[i].eventType=FSEVENT_NULLEVENT;
				}
			}
			break;
		}
	}
}

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

FsDemoModeInfo::FsDemoModeInfo(FSDEMOMODETYPE t)
{
	Initialize(t);
}

void FsDemoModeInfo::Initialize(FSDEMOMODETYPE t)
{
	type=t;

	useCockpitView=YSTRUE;
	useOutsideView=YSTRUE;
	useCarrierView=YSTRUE;
	usePlayerToGndView=YSTRUE;
	useGndToPlayerView=YSTRUE;
	useAirToAirView=YSTRUE;
	useAirFromAirView=YSTRUE;
	changeViewPointInterval=20.0;

	showCenter=YsOrigin();
	refObj=NULL;
	acroType=FSACRO_NONE;
	int i;
	for(i=0; i<MAXNUMFORMATION; i++)
	{
		formation[i]=NULL;
	}
	for(i=0; i<MAXNUMSOLO; i++)
	{
		solo[i]=NULL;
	}

	concFlyByType=0;
	nextActionTime=0.0;
}

FsAirplane *FsDemoModeInfo::PickRandomAirplane(int &fomPosition,YSBOOL searchFom,YSBOOL searchSolo)
{
	int n,i,inc;
	FsAirplane *air;

	n=1+rand()%(MAXNUMFORMATION+MAXNUMSOLO);
	air=NULL;
	fomPosition=0;

	while(n>0)
	{
		inc=0;
		if(searchFom==YSTRUE)
		{
			for(i=0; n>0 && i<MAXNUMFORMATION; i++)
			{
				if(formation[i]!=NULL)
				{
					air=formation[i];
					fomPosition=i+1;
					n--;
					inc++;
				}
			}
		}
		if(searchSolo==YSTRUE)
		{
			for(i=0; n>0 && i<MAXNUMSOLO; i++)
			{
				if(solo[i]!=NULL)
				{
					air=solo[i];
					fomPosition=-(i+1);
					n--;
					inc++;
				}
			}
		}

		if(inc==0)
		{
			fomPosition=0;
			return NULL;
		}
	}
	return air;
}

int FsDemoModeInfo::GetFormationPosition(FsAirplane *air)  // Position   >0:Formation  <0:Solo
{
	int i;

	for(i=0; i<MAXNUMFORMATION; i++)
	{
		if(air==formation[i])
		{
			return i+1;
		}
	}
	for(i=0; i<MAXNUMSOLO; i++)
	{
		if(air==solo[i])
		{
			return -(i+1);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////

void FsNewFlightAirplaneData::Initialize(void)
{
	iff=0;
	typeName.Set("");
	weaponConfig.Set(0,NULL);
	startPos.Set("");
	fuel=75;
}

YSRESULT FsNewFlightAirplaneData::Save(FILE *fp) const
{
	fprintf(fp,"NEWFLTAPDATA\n");

	fprintf(fp,"IFF %d\n",iff);
	fprintf(fp,"FUEL %d%%\n",fuel);
	if(0!=typeName[0])
	{
		fprintf(fp,"AIRCRAFT \"%s\"\n",typeName.Txt());
	}
	if(0!=startPos[0])
	{
		fprintf(fp,"STARTPOS \"%s\"\n",startPos.Txt());
	}

	int i;
	for(i=0; i<weaponConfig.GetN()-1; i+=2)
	{
		fprintf(fp,"WPNCONFIG %s %d\n",FsGetWeaponString((FSWEAPONTYPE)weaponConfig[i]),weaponConfig[i+1]);
	}

	fprintf(fp,"ENDNEWFLTAPDATA\n");

	return YSOK;
}

YSRESULT FsNewFlightAirplaneData::Load(FILE *fp)
{
	Initialize();

	char str[256],buf[256];
	int ac;
	char *av[16];

	while(fgets(str,256,fp)!=NULL)
	{
		strcpy(buf,str);
		if(YsArguments(&ac,av,16,buf)==YSOK && ac>0)
		{
			if(strcmp(av[0],"NEWFLTAPDATA")==0)
			{
				// Ignore identifier
			}
			else if(strcmp(av[0],"IFF")==0 && ac==2)
			{
				iff=atoi(av[1]);
			}
			else if(strcmp(av[0],"FUEL")==0 && ac==2)
			{
				fuel=atoi(av[1]);
			}
			else if(strcmp(av[0],"AIRCRAFT")==0 && ac==2)
			{
				typeName.Set(av[1]);
			}
			else if(strcmp(av[0],"STARTPOS")==0 && ac==2)
			{
				startPos.Set(av[1]);
			}
			else if(strcmp(av[0],"WPNCONFIG")==0 && ac==3)
			{
				weaponConfig.Append((int)FsGetWeaponTypeByString(av[1]));
				weaponConfig.Append(atoi(av[2]));
			}
			else if(strcmp(av[0],"ENDNEWFLTAPDATA")==0)
			{
				break;
			}
			else
			{
				fsStderr.Printf("Cannot recognize \"%s\"\n",str);
				return YSERR;
			}
		}
	}
	return YSOK;
}

////////////////////////////////////////////////////////////

void FsNewFlightDialogInfo::Initialize(void)
{
	playerAirInfo.Initialize();
	nextStartPos=YSFALSE;
	for(int i=0; i<FsNewFlightDialogInfo::MaxNumWingman; i++)
	{
		wingman[i]=YSFALSE;
		wingmanInfo[i].Initialize();
	}

	addComputerAircraft=YSTRUE;

	envInfo.Initialize();

	fomType=FSFOM_NONE;

	flyImmediately=YSFALSE;
}

void FsNewFlightEnvironmentInfo::Initialize(void)
{
	specifyEnvironment=YSTRUE;

	windDir=0.0;
	windSpd=0.0;
	dayOrNight=FSDAYLIGHT;
	fog=YSTRUE;
	fogVisibility=20000.0;
	cloudLayer.Clear();
}

void FsNewFlightDialogInfo::SetFromConfig(const class FsWorld *world,const class FsFlightConfig &cfg)
{
	Initialize();
	fieldName.Set(cfg.defField);
	playerAirInfo.typeName.Set(cfg.defAirplane);
	playerAirInfo.startPos.Set(cfg.defStartPos);

	const FsAirplaneTemplate *tmpl;
	if((tmpl=world->GetAirplaneTemplate(cfg.defAirplane))!=NULL)
	{
		tmpl->GetProperty()->GetWeaponConfig(playerAirInfo.weaponConfig);
	}
	playerAirInfo.fuel=75;
}

void FsNewFlightDialogInfo::SetFromOption(const FsNewFlightDialogOption &opt)
{
	if(0<opt.fieldName.Strlen())
	{
		fieldName=opt.fieldName;
	}
	if(0<opt.aircraftName.Strlen())
	{
		playerAirInfo.typeName=opt.aircraftName;
	}
	if(0<opt.startPosName.Strlen())
	{
		playerAirInfo.startPos=opt.startPosName;
	}
	nextStartPos=opt.nextStartPos;
}

void FsNewFlightDialogInfo::AutoSetStartPosition(FsWorld *world,const char fieldName[])
{
	int i;
	char startPos[256];
	for(i=0; world->GetFieldStartPositionName(startPos,fieldName,i)==YSOK; i++)
	{
		if(i==0)
		{
			playerAirInfo.startPos.Set(startPos);
		}
		else if(1<=i && i<FsNewFlightDialogInfo::MaxNumWingman+1)
		{
			wingmanInfo[i-1].startPos.Set(startPos);
		}
	}
}

void FsNewFlightDialogInfo::AutoSetWingmanStartPosition(
    FsWorld *world,const char fieldName[],const char playerStartPos[])
{
	int i,wingmanId;
	char startPos[256];
	wingmanId=0;
	for(i=0; world->GetFieldStartPositionName(startPos,fieldName,i)==YSOK; i++)
	{
		if(strcmp(startPos,playerStartPos)!=0 && wingmanId<FsNewFlightDialogInfo::MaxNumWingman)
		{
			wingmanInfo[wingmanId].startPos.Set(startPos);
			wingmanId++;
		}
	}
}

void FsNewDriveDialogInfo::Initialize(void)
{
	fieldName.Set("");
	gobId=0;
	driveNow=YSFALSE;
}

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

FsMissionEnemyGroundAttackerInfo::FsMissionEnemyGroundAttackerInfo()
{
	Initialize();
}

void FsMissionEnemyGroundAttackerInfo::Initialize(void)
{
	allowAirCover=YSFALSE;
	allowStealth=YSFALSE;
	allowBomb=YSFALSE;
	allowAGM=YSFALSE;
	allowHeavyBomber=YSFALSE;
	maxNumAttacker=3;

	jet=YSTRUE;
	ww2=YSFALSE;
}

////////////////////////////////////////////////////////////

FsInterceptMissionInfo::FsInterceptMissionInfo()
{
	attackerInfo.Initialize();

	attackerInfo.allowAirCover=YSTRUE;
	attackerInfo.allowStealth=YSTRUE;
	attackerInfo.allowBomb=YSTRUE;
	attackerInfo.allowHeavyBomber=YSFALSE;
	attackerInfo.jet=YSTRUE;
	attackerInfo.ww2=YSFALSE;

	attackerInfo.maxNumAttacker=3;

	numWingman=1;
}

YsArray <YsString> FsInterceptMissionInfo::Encode() const
{
	YsArray <YsString> encode;

	encode.Increment();
	encode.Last().Printf("AIRCOVER %s\n",FsTrueFalseString(attackerInfo.allowAirCover));

	encode.Increment();
	encode.Last().Printf("STEALTH %s\n",FsTrueFalseString(attackerInfo.allowStealth));

	encode.Increment();
	encode.Last().Printf("BOMB %s\n",FsTrueFalseString(attackerInfo.allowBomb));

	encode.Increment();
	encode.Last().Printf("HEAVYBOMBER %s\n",FsTrueFalseString(attackerInfo.allowHeavyBomber));

	encode.Increment();
	encode.Last().Printf("NUMWINGMAN %d\n",numWingman);

	encode.Increment();
	encode.Last().Printf("NUMATTACKER %d\n",attackerInfo.maxNumAttacker);

	encode.Increment();
	encode.Last().Printf("JET %s\n",FsTrueFalseString(attackerInfo.jet));

	encode.Increment();
	encode.Last().Printf("WW2 %s\n",FsTrueFalseString(attackerInfo.ww2));

	return encode;
}

YSRESULT FsInterceptMissionInfo::Decode(const YsConstArrayMask <YsString> &argv)
{
	YSRESULT res=YSOK;
	if(0<argv.size())
	{
		if(0==argv[0].STRCMP("INTERCEPT3"))
		{
			// Ignore identifier
			return YSOK;
		}
		else if(0==argv[0].STRCMP("FIELD") && argv.size()==2)
		{
			fieldName.Set(argv[1]);
			return YSOK;
		}
		else if(0==argv[0].STRCMP("AIRCOVER") && argv.size()==2)
		{
			FsGetBool(attackerInfo.allowAirCover,argv[1]);
			return YSOK;
		}
		else if(0==argv[0].STRCMP("STEALTH") && argv.size()==2)
		{
			FsGetBool(attackerInfo.allowStealth,argv[1]);
			return YSOK;
		}
		else if(0==argv[0].STRCMP("BOMB") && argv.size()==2)
		{
			FsGetBool(attackerInfo.allowBomb,argv[1]);
			return YSOK;
		}
		else if(0==argv[0].STRCMP("HEAVYBOMBER") && argv.size()==2)
		{
			FsGetBool(attackerInfo.allowHeavyBomber,argv[1]);
			return YSOK;
		}
		else if(0==argv[0].STRCMP("NUMWINGMAN") && argv.size()==2)
		{
			numWingman=atoi(argv[1]);
			return YSOK;
		}
		else if(0==argv[0].STRCMP("NUMATTACKER") && argv.size()==2)
		{
			attackerInfo.maxNumAttacker=YsGreater(1,atoi(argv[1]));
			return YSOK;
		}
		else if(0==argv[0].STRCMP("JET") && argv.size()==2)
		{
			FsGetBool(attackerInfo.jet,argv[1]);
			return YSOK;
		}
		else if(0==argv[0].STRCMP("WW2") && argv.size()==2)
		{
			FsGetBool(attackerInfo.ww2,argv[1]);
			return YSOK;
		}
	}
	return res;
}

YSRESULT FsInterceptMissionInfo::Save(const wchar_t fn[]) const
{
	FILE *fp=YsFileIO::Fopen(fn,"w");
	if(fp!=NULL)
	{
		Save(fp);
		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsInterceptMissionInfo::Save(FILE *fp) const
{
	fprintf(fp,"INTERCEPT3\n"); // Identifier

	if(0!=fieldName[0])
	{
		fprintf(fp,"FIELD \"%s\"\n",fieldName.Txt());
	}

	playerAirInfo.Save(fp);

	for(auto &str : Encode())
	{
		fprintf(fp,"%s\n",str.Txt());
	}

	fprintf(fp,"ENDINTERCEPTMISSIONINFO\n");

	return YSOK;
}

YSRESULT FsInterceptMissionInfo::Load(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(NULL!=fp)
	{
		if(YSOK==Load(fp))
		{
			fclose(fp);
			return YSOK;
		}
	}

	if(NULL!=fp)
	{
		fclose(fp);
	}
	return YSERR;
}

YSRESULT FsInterceptMissionInfo::Load(FILE *fp)
{
	YsString str;
	while(nullptr!=str.Fgets(fp))
	{
		YsArray <YsString> argv=str.Argv();
		if(0<argv.size() && 0==argv[0].STRCMP("NEWFLTAPDATA"))
		{
			playerAirInfo.Load(fp);
		}
		else if(0<argv.size() && 0==argv[0].STRCMP("ENDINTERCEPTMISSIONINFO"))
		{
			break;
		}
		else if(YSOK!=Decode(argv))
		{
			fsStderr.Printf("Cannot recognize \"%s\"\n",str.Txt());
			return YSERR;
		}
	}
	return YSOK;
}

////////////////////////////////////////////////////////////

FsGroundToAirDefenseMissionInfo::FsGroundToAirDefenseMissionInfo()
{
	Initialize();
}

void FsGroundToAirDefenseMissionInfo::Initialize(void)
{
	FsNewDriveDialogInfo::Initialize();
	attackerInfo.Initialize();

	numIntercepter=0;
	intercepter.Set("F-16_FIGHTING_FALCON");
	infiniteGun=YSTRUE;

	attackerInfo.allowAirCover=YSFALSE;
	attackerInfo.allowStealth=YSFALSE;
	attackerInfo.allowBomb=YSFALSE;
	attackerInfo.allowAGM=YSFALSE;
	attackerInfo.allowHeavyBomber=YSFALSE;
	attackerInfo.maxNumAttacker=3;

	attackerInfo.jet=YSTRUE;
	attackerInfo.ww2=YSFALSE;
}

YsArray <YsString> FsGroundToAirDefenseMissionInfo::Encode(void) const
{
	YsArray <YsString> encoded;

	encoded.Increment();
	encoded.Last().Printf("NINTERCEPTER %d\n",numIntercepter);

	encoded.Increment();
	encoded.Last().Printf("INTERCEPTER \"%s\"\n",intercepter.Txt());

	encoded.Increment();
	encoded.Last().Printf("INFINITEGUN %s\n",FsTrueFalseString(infiniteGun));

	encoded.Increment();
	encoded.Last().Printf("STEALTH %s\n",FsTrueFalseString(attackerInfo.allowStealth));

	encoded.Increment();
	encoded.Last().Printf("BOMB %s\n",FsTrueFalseString(attackerInfo.allowBomb));

	encoded.Increment();
	encoded.Last().Printf("HEAVYBOMBER %s\n",FsTrueFalseString(attackerInfo.allowHeavyBomber));

	encoded.Increment();
	encoded.Last().Printf("JET %s\n",FsTrueFalseString(attackerInfo.jet));

	encoded.Increment();
	encoded.Last().Printf("WW2 %s\n",FsTrueFalseString(attackerInfo.ww2));

	encoded.Increment();
	encoded.Last().Printf("NUMATTACKER %d\n",attackerInfo.maxNumAttacker);

	return encoded;
}
YSRESULT FsGroundToAirDefenseMissionInfo::Decode(const YsConstArrayMask <YsString> &argv)
{
	YSRESULT res=YSOK;
	if(0<argv.size())
	{
		if(0==argv[0].STRCMP("GOBID") && argv.size()>=2)
		{
			gobId=atoi(argv[1]);
		}
		else if(0==argv[0].STRCMP("NINTERCEPTER") && argv.size()>=2)
		{
			numIntercepter=atoi(argv[1]);
		}
		else if(0==argv[0].STRCMP("INTERCEPTER") && argv.size()==2)
		{
			intercepter=argv[1];
		}
		else if(0==argv[0].STRCMP("STEALTH") && argv.size()==2)
		{
			FsGetBool(attackerInfo.allowStealth,argv[1]);
		}
		else if(0==argv[0].STRCMP("BOMB") && argv.size()==2)
		{
			FsGetBool(attackerInfo.allowBomb,argv[1]);
		}
		else if(0==argv[0].STRCMP("HEAVYBOMBER") && argv.size()==2)
		{
			FsGetBool(attackerInfo.allowHeavyBomber,argv[1]);
		}
		else if(0==argv[0].STRCMP("NUMATTACKER") && argv.size()==2)
		{
			attackerInfo.maxNumAttacker=YsGreater(1,atoi(argv[1]));
		}
		else if(0==argv[0].STRCMP("JET") && argv.size()==2)
		{
			FsGetBool(attackerInfo.jet,argv[1]);
		}
		else if(0==argv[0].STRCMP("WW2") && argv.size()==2)
		{
			FsGetBool(attackerInfo.ww2,argv[1]);
		}
		else if(0==argv[0].STRCMP("INFINITEGUN") && argv.size()==2)
		{
			FsGetBool(infiniteGun,argv[1]);
		}
		else
		{
			return YSERR;
		}
	}
	return res;
}

YSRESULT FsGroundToAirDefenseMissionInfo::Save(const wchar_t fn[]) const
{
	FILE *fp=YsFileIO::Fopen(fn,"w");
	if(NULL!=fp)
	{
		YSRESULT res=Save(fp);
		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT FsGroundToAirDefenseMissionInfo::Save(FILE *fp) const
{
	fprintf(fp,"GNDTOAIR1\n");
	fprintf(fp,"FIELD \"%s\"\n",fieldName.Txt());
	fprintf(fp,"GOBID %d\n",gobId);
	for(auto &s : Encode())
	{
		fprintf(fp,"%s\n",s.data());
	}
	fprintf(fp,"ENDGNDTOAIR\n");

	return YSOK;
}

YSRESULT FsGroundToAirDefenseMissionInfo::Load(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(NULL!=fp)
	{
		YSRESULT res=Load(fp);
		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT FsGroundToAirDefenseMissionInfo::Load(FILE *fp)
{
	YsString str,buf;
	YsArray <YsString,16> args;

	while(str.Fgets(fp)!=NULL)
	{
		buf=str;
		if(YSOK==buf.Arguments(args) && 0<args.GetN())
		{
			if(strcmp(args[0],"GNDTOAIR1")==0)
			{
				// Ignore identifier
			}
			else if(strcmp(args[0],"FIELD")==0 && args.GetN()<=2)
			{
				fieldName.Set(args[1]);
			}
			else if(strcmp(args[0],"ENDGNDTOAIR")==0)
			{
				break;
			}
			else if(YSOK!=Decode(args))
			{
				fsStderr.Printf("Cannot recognize \"%s\"\n",str.Txt());
				return YSERR;
			}
		}
	}
	return YSOK;
}

