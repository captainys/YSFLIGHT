#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <ysclass.h>
#include <ysport.h>
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

// Standard precision -> 3,4,2,2,2,2

YSRESULT FsSimulation::Save(
    FILE *fp,
    int airPosPrecision,int airAttPrecision,
    int gndPosPrecision,int gndAttPrecision,
    int wpnPosPrecision,int wpnAttPrecision,
    const double & /*timeStep*/)
{
	int i;
	const FsField *fld;
	char format[256],turretFormat[256];


	AssignUniqueYsfId();  // 2009/03/27 Always assign a unique YFS ID.


	if(0<simTitle.Strlen())
	{
		fprintf(fp,"SIMTITLE \"%s\"\n",simTitle.Txt());
	}


	if(NULL!=(fld=GetField()))
	{
		fprintf(fp,"FIELDNAM %s %g %g %g %g %g %g FALSE LOADAIR:FALSE\n",
		    // last "FALSE" prevent double loading of grd objs
		    fld->GetIdName(),
		    fld->GetPosition().x(),fld->GetPosition().y(),fld->GetPosition().z(),
		    fld->GetAttitude().h(),fld->GetAttitude().p(),fld->GetAttitude().b());
	}

	{
		const FsExistence *obj=firstPlayer.GetObject(this);
		if(NULL!=obj)
		{
			fprintf(fp,"INITPLYR %s %d\n",FsExistence::TypeToStr(obj->GetType()),obj->ysfId);
		}
	}

	fprintf(fp,"CANCONTI %s\n",YsBoolToStr(canContinue));

	if(env==FSDAYLIGHT)  // 2003/01/19
	{
		fprintf(fp,"ENVIRONM DAY\n");
	}
	else if(env==FSNIGHT)
	{
		fprintf(fp,"ENVIRONM NIGHT\n");
	}

	fprintf(fp,"ALLOWGUN %s\n",((allowedWeaponType & FSWEAPON_ALLOWGUN) ? "TRUE" : "FALSE"));
	fprintf(fp,"ALLOWAAM %s\n",((allowedWeaponType & FSWEAPON_ALLOWAAM) ? "TRUE" : "FALSE"));
	fprintf(fp,"ALLOWAGM %s\n",((allowedWeaponType & FSWEAPON_ALLOWAGM) ? "TRUE" : "FALSE"));
	fprintf(fp,"ALLOWBOM %s\n",((allowedWeaponType & FSWEAPON_ALLOWBOMB) ? "TRUE" : "FALSE"));
	fprintf(fp,"ALLOWRKT %s\n",((allowedWeaponType & FSWEAPON_ALLOWROCKET) ? "TRUE" : "FALSE"));

	if(goal!=NULL)
	{
		goal->WriteFile(fp);
	}

	for(auto &addOnPtr : addOnList)
	{
		for(auto &s : addOnPtr->Serialize(this))
		{
			if('\n'!=s.LastChar())
			{
				s.push_back('\n');
			}
			fprintf(fp,"%s",s.data());
		}
	}

	simEvent->Save(fp,this);


	FsAirplane *air;
	air=NULL;
	while((air=FindNextAirplane(air))!=NULL)
	{
		if(air->actualIdName[0]==0)
		{
			fprintf(fp,"AIRPLANE %s ",air->Prop().GetIdentifier());
			fprintf(fp,"%s",FsTrueFalseString(IsPlayerAirplane(air)));

			const char *substId;
			substId=air->Prop().GetSubstIdName();
			if(substId!=NULL && substId[0]!=0)
			{
				fprintf(fp," SUBST:%s",substId);
			}
			fprintf(fp,"\n");
		}
		else
		{
			fprintf(fp,"AIRPLANE %s ",(const char *)air->actualIdName);
			fprintf(fp,"%s",FsTrueFalseString(IsPlayerAirplane(air)));

			fprintf(fp," SUBST:%s",air->Prop().GetIdentifier());

			fprintf(fp,"\n");
		}

		if(air->_startPosition[0]!=0)
		{
			fprintf(fp,"STARTPOS NA %s\n",air->_startPosition.Txt());
		}
		switch(air->iff)
		{
		case FS_IFF0:
			fprintf(fp,"IDENTIFY 0\n");
			break;
		case FS_IFF1:
			fprintf(fp,"IDENTIFY 1\n");
			break;
		case FS_IFF2:
			fprintf(fp,"IDENTIFY 2\n");
			break;
		case FS_IFF3:
			fprintf(fp,"IDENTIFY 3\n");
			break;
		}


		{
			const int nSmokeGen=air->Prop().GetNumSmokeGenerator();
			YSBOOL smokeChanged=YSFALSE;
			for(int smkIdx=0; smkIdx<nSmokeGen; ++smkIdx)
			{
				if(air->Prop().GetSmokeColor(smkIdx)!=air->Prop().GetDefaultSmokeColor(smkIdx))
				{
					smokeChanged=YSTRUE;
					break;
				}
			}
			if(YSTRUE==smokeChanged)
			{
				YSBOOL allSameColor=YSTRUE;
				for(int smkIdx=1; smkIdx<nSmokeGen; ++smkIdx)
				{
					if(air->Prop().GetSmokeColor(0)!=air->Prop().GetSmokeColor(smkIdx))
					{
						allSameColor=YSFALSE;
						break;
					}
				}
				if(YSTRUE==allSameColor)
				{
					auto smkCol=air->Prop().GetSmokeColor(0);
					fprintf(fp,"AIRPCMND SMOKECOL ALL %d %d %d\n",
					    smkCol.Ri(),smkCol.Gi(),smkCol.Bi());
				}
				else
				{
					for(int smkIdx=0; smkIdx<nSmokeGen; ++smkIdx)
					{
						auto smkCol=air->Prop().GetSmokeColor(smkIdx);
						fprintf(fp,"AIRPCMND SMOKECOL %d %d %d %d\n",
						    smkIdx,smkCol.Ri(),smkCol.Gi(),smkCol.Bi());
					}
				}
			}
		}


		if(air->ysfId!=0 || air->name[0]!=0)
		{
			fprintf(fp,"IDANDTAG %d \"%s\"\n",air->ysfId,(const char *)air->name);
		}

		if(air->airFlag!=0)
		{
			fprintf(fp,"AIRFLAGS %u\n",air->airFlag);
		}

		if(air->name[0]!=0)
		{
			int i;
			char username[256];
			strcpy(username,air->name);
			for(i=0; username[i]!=0; i++)
			{
				if(username[i]=='\t' || username[i]=='\"')
				{
					username[i]='_';
				}
			}
			fprintf(fp,"USERNAME \"%s\"\n",username);
		}

		air->GetAirTrafficInfo().Save(fp,this,*air);

		if(air->primaryTarget==YSTRUE)
		{
			fprintf(fp,"ARTARGET\n");
		}

		for(i=0; i<air->cmdLog.GetN(); i++)
		{
			fprintf(fp,"AIRPCMND %s\n",(const char *)air->cmdLog[i]);
		}

		auto reloadCmd=air->GetReloadCommand();
		for(auto &cmd : reloadCmd)
		{
			fprintf(fp,"RELDCMND %s\n",cmd.Txt());
		}

		if(air->motionPath!=NULL)
		{
			fprintf(fp,"MOTNPATH \"%s\" %s\n",air->motionPath->GetTag(),FsTrueFalseString(air->useMotionPathOffset));
		}
		else if(air->motionPathPnt.GetN()>0)
		{
			forYsArray(i,air->motionPathPnt)
			{
				fprintf(fp,"MPATHPNT %lf %lf %lf\n",air->motionPathPnt[i].x(),air->motionPathPnt[i].y(),air->motionPathPnt[i].z());
			}
			fprintf(fp,"MPATHLOP %s\n",(air->motionPathIsLoop==YSTRUE ? "TRUE" : "FALSE"));
			fprintf(fp,"MPATHOFS %s\n",(air->useMotionPathOffset==YSTRUE ? "TRUE" : "FALSE"));
		}

		if(air->GetLandWhenLowFuel()>YsTolerance)
		{
			fprintf(fp,"LANDLWFL %.2lf\n",air->GetLandWhenLowFuel());
		}

		if(0<air->GetHomeBaseName().Strlen())
		{
			fprintf(fp,"HOMEBASE %s \"%s\"\n",(const char *)FsSimInfo::BaseTypeToString(air->GetHomeBaseType()),air->GetHomeBaseName().Txt());
		}

		air->SaveAutoPilot(fp,this);

		if(air->rec!=NULL)
		{
			int i,j,nr;
			FsFlightRecord *r;
			double t;

			nr=air->rec->GetNumRecord();
			fprintf(fp,"NUMRECOR %d 4\n",nr);
			// Version 3
			//   Includes Turret Position and States
			// Version 4 almost same as version 3 except smoke is 0-255 rather than 0/1

			sprintf(format,"%s%d%s %s%d%s %s%d%s %s%d%s %s%d%s %s%d%s\n",
			    "%.",airPosPrecision,"lf",
                "%.",airPosPrecision,"lf",
                "%.",airPosPrecision,"lf",
                "%.",airAttPrecision,"lf",
                "%.",airAttPrecision,"lf",
                "%.",airAttPrecision,"lf %.1lf");

			sprintf(turretFormat," %s%d%s %s%d%s %s",
                "%.",airAttPrecision,"lf",
                "%.",airAttPrecision,"lf",
                "%d");

			for(i=0; i<nr; i++)
			{
				r=air->rec->GetElement(t,i);
				fprintf(fp,"%g\n",t);

				char buf[1024];
				sprintf(buf,format,    // "%.3lf %.3lf %.3lf %.4lf %.4lf %.4lf %.1lf\n"
				    double(r->pos.x()),double(r->pos.y()),double(r->pos.z()),
				    double(r->h),double(r->p),double(r->b),
				    double(r->g));
				FsCompressNumberInString(buf);
				fprintf(fp,"%s",buf);

				fprintf(fp,"%d %d %d %d ",r->state      ,r->vgw         ,r->spoiler,r->gear);
				fprintf(fp,"%d %d %d %d ",r->flap       ,r->brake       ,r->smoke  ,r->vapor);
				fprintf(fp,"%d %d %d %d ",r->flags      ,r->dmgTolerance,r->thr    ,r->elv);
				fprintf(fp,"%d %d %d %d ",r->ail        ,r->rud         ,r->elvTrim,r->thrVector);
				fprintf(fp,"%d %d\n"     ,r->thrReverser,r->bombBay);

				fprintf(fp,"%d",r->turret.GetN());
				for(j=0; j<r->turret.GetN(); j++)
				{
					fprintf(fp,turretFormat,r->turret[j].h,r->turret[j].p,r->turret[j].turretState);
				}
				fprintf(fp,"\n");
			}
		}
	}

	FsGround *gnd;
	gnd=NULL;
	while((gnd=FindNextGround(gnd))!=NULL)
	{
		fprintf(fp,"GROUNDOB %s",gnd->Prop().GetIdentifier());

		if(gnd==GetPlayerGround())
		{
			fprintf(fp," TRUE");
		}
		else
		{
			fprintf(fp," FALSE");
		}

		const char *substId;
		substId=gnd->Prop().GetSubstIdName();
		if(substId!=NULL && substId[0]!=0)
		{
			fprintf(fp," SUBST:%s",substId);
		}
		fprintf(fp,"\n");


		switch(gnd->iff)
		{
		case FS_IFF0:
			fprintf(fp,"IDENTIFY 0\n");
			break;
		case FS_IFF1:
			fprintf(fp,"IDENTIFY 1\n");
			break;
		case FS_IFF2:
			fprintf(fp,"IDENTIFY 2\n");
			break;
		case FS_IFF3:
			fprintf(fp,"IDENTIFY 3\n");
			break;
		}

		if(gnd->ysfId!=0 || gnd->name[0]!=0)
		{
			fprintf(fp,"IDANDTAG %d \"%s\"\n",gnd->ysfId,(const char *)gnd->name);
		}

		for(i=0; i<gnd->cmdLog.GetN(); i++)
		{
			fprintf(fp,"GRNDCMND %s\n",(const char *)gnd->cmdLog[i]);
		}

		if(gnd->motionPath!=NULL)
		{
			fprintf(fp,"MOTNPATH \"%s\" %s\n",gnd->motionPath->GetTag(),FsTrueFalseString(gnd->useMotionPathOffset));
		}
		else if(gnd->motionPathPnt.GetN()>0)
		{
			forYsArray(i,gnd->motionPathPnt)
			{
				fprintf(fp,"MPATHPNT %lf %lf %lf\n",gnd->motionPathPnt[i].x(),gnd->motionPathPnt[i].y(),gnd->motionPathPnt[i].z());
			}
			fprintf(fp,"MPATHLOP %s\n",(gnd->motionPathIsLoop==YSTRUE ? "TRUE" : "FALSE"));
			fprintf(fp,"MPATHOFS %s\n",(gnd->useMotionPathOffset==YSTRUE ? "TRUE" : "FALSE"));
		}

		fprintf(fp,"GNDPOSIT %gm %gm %gm\n",
		     gnd->initPosition.x(),
		     gnd->initPosition.y(),
		     gnd->initPosition.z());
		fprintf(fp,"GNDATTIT %grad %grad %grad\n",
		     gnd->initAttitude.h(),
		     gnd->initAttitude.p(),
		     gnd->initAttitude.b());

		if(gnd->primaryTarget==YSTRUE)
		{
			fprintf(fp,"PRTARGET\n");
		}
		fprintf(fp,"GNDFLAGS %u\n",gnd->gndFlag);

		// Ground Intention ....
		// in the future

		if(gnd->rec!=NULL)
		{
			int i,j,nr;
			FsGroundRecord *r;
			double t;

			nr=gnd->rec->GetNumRecord();
			fprintf(fp,"NUMGDREC %d 3\n",nr);
			// Version 1
			//   aim is now separated into aaaAim, samAim, and canAim
			//   for future extension, three zeros are also added after that.

			sprintf(format,"%s%d%s %s%d%s %s%d%s %s%d%s %s%d%s %s%d%s\n",
			    "%.",gndPosPrecision,"lf",
                "%.",gndPosPrecision,"lf",
                "%.",gndPosPrecision,"lf",
                "%.",gndAttPrecision,"f",
                "%.",gndAttPrecision,"f",
                "%.",gndAttPrecision,"f");

			sprintf(turretFormat," %s%d%s %s%d%s %s",
                "%.",gndAttPrecision,"lf",
                "%.",gndAttPrecision,"lf",
                "%d");

			for(i=0; i<nr; i++)
			{
				r=gnd->rec->GetElement(t,i);
				fprintf(fp,"%g\n",t);

				char buf[1024];
				sprintf(buf,format, //"%.2f %.2f %.2f %.2f %.2f %.2f\n"
				    r->pos.x(),r->pos.y(),r->pos.z(),r->h,r->p,r->b);
				FsCompressNumberInString(buf);
				fprintf(fp,"%s",buf);

				fprintf(fp,"%d %d\n",
				    r->state,
				    r->dmgTolerance);
				fprintf(fp,"%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f 0 0 0\n",
				    r->aaaAimh,r->aaaAimp,r->aaaAimb,
				    r->samAimh,r->samAimp,r->samAimb,
				    r->canAimh,r->canAimp,r->canAimb);
				fprintf(fp,"%d %d %d %d %d %d\n",
				    r->steering,
				    r->leftDoor,
				    r->rightDoor,
				    r->rearDoor,
				    r->brake,
				    r->lightState);


				fprintf(fp,"%d",r->turret.GetN());
				for(j=0; j<r->turret.GetN(); j++)
				{
					fprintf(fp,turretFormat,r->turret[j].h,r->turret[j].p,r->turret[j].turretState);
				}
				fprintf(fp,"\n");
			}
		}
	}

	bulletHolder.Save(fp,this,wpnPosPrecision,wpnAttPrecision);
	explosionHolder.Save(fp,this);
	weather->Save(fp);
	if(cloud->IsReady()==YSTRUE)  // 2003/01/14
	{
		cloud->Save(fp);
	}
	if(solidCloud->IsReady()==YSTRUE) // 2005/09/06
	{
		solidCloud->Save(fp);
	}
	return YSOK;
}

YSRESULT FsSimulation::LoadWeaponRecord(FILE *fp)
{
	return bulletHolder.Load(fp,this);
}

YSRESULT FsSimulation::LoadExplosionRecord(FILE *fp)
{
	return explosionHolder.Load(fp,this);
}

YSRESULT FsSimulation::LoadCloud(FILE *fp)
{
	return cloud->Load(fp);
}

YSRESULT FsSimulation::LoadSolidCloud(FILE *fp)
{
	return solidCloud->Load(fp);
}

YSRESULT FsSimulation::LoadWeather(FILE *fp)
{
	return weather->Load(fp);
}

YSRESULT FsSimulation::LoadSimulationEvent(FILE *fp)
{
	simEvent->Load(fp,this);
	return YSOK;
}

