#include "fsatc.h"
#include "fsexistence.h"
#include "fssimulation.h"
#include "fsnavaid.h"
#include "ysunitconv.h"

YSRESULT FsApproach::Save(FILE *fp,const FsSimulation *sim) const
{
	fprintf(fp,"APPROACH\n");
	fprintf(fp,"TYPE \"%s\"\n",GetApproachTypeString());

	fprintf(fp,"DEST \"%s\"\n",dstName.Txt());

	FsGround *gnd=sim->FindGround(gndKey);
	if(NULL!=gnd)
	{
		fprintf(fp,"GROUND \"%s\"\n",gnd->GetName());
	}
	fprintf(fp,"ENDAPPROACH\n");

	return YSOK;
}

YSRESULT FsApproach::Load(FsSimulation *,FILE *fp)
{
	YsString str;
	while(NULL!=str.Fgets(fp))
	{
		YsArray <YsString,16> args;
		str.Arguments(args);
		if(0<args.GetN())
		{
			if(0==strcmp("APPROACH",args[0]))
			{
				// Skip signature
			}
			if(0==strcmp("TYPE",args[0]) && 2<=args.GetN())
			{
				appType=StringToAppType(args[1]);
			}
			else if(0==strcmp("DEST",args[0]) && 2<=args.GetN())
			{
				dstName=args[1];
			}
			else if(0==strcmp("GROUND",args[0]) && 2<=args.GetN())
			{
				gndNameCacheForFileIO=args[1];
			}
			else if(0==strcmp("ENDAPPROACH",args[0]))
			{
				return YSOK;
			}
		}
	}
	return YSERR;
}

YSRESULT FsApproach::ReconnectGroundFacility(FsSimulation *sim)  // Called from FsAirTrafficInfo
{
	if(0<gndNameCacheForFileIO.Strlen())
	{
		FsGround *gnd=sim->FindGroundByTag(gndNameCacheForFileIO);
		if(NULL!=gnd)
		{
			gndKey=gnd->SearchKey();
			return YSOK;
		}
		else
		{
			return YSERR;
		}
	}
	else
	{
		return YSOK;
	}
}



YSRESULT FsAirTrafficInfo::Save(FILE *fp,const FsSimulation *sim,const FsAirplane &) const
{
	if(YSTRUE==talkingWithAtc)
	{
		fprintf(fp,"BEGINATC\n");

		const FsAirTrafficController *atc=GetAirTrafficController(sim);
		if(atcKey==FsAirTrafficController::PrimaryAirTrafficControllerKey)
		{
			fprintf(fp,"DEFATC\n");
		}
		else if(NULL!=atc)
		{
			fprintf(fp,"ATCNAME \"%s\"\n",atc->GetName());
		}

		for(int i=0; i<path.GetN(); i++)
		{
			fprintf(fp,"FIX %s %.1lfm %.1lfm %.1lfm\n",
			    path[i].GetFixString(),
			    path[i].GetPosition().x(),
			    path[i].GetPosition().y(),
			    path[i].GetPosition().z());
		}

		fprintf(fp,"NEXTFIX %d\n",currentPathIndex);

		approach.Save(fp,sim);

		fprintf(fp,"ENDATC\n");
	}
	return YSOK;
}

YSRESULT FsAirTrafficInfo::Load(FsSimulation *sim,FsAirplane &air,FILE *fp)
{
	CleanUp();

	YsString str;
	while(NULL!=str.Fgets(fp))
	{
		YsArray <YsString,16> args;
		str.Arguments(args);
		if(0<args.GetN())
		{
			if(0==strcmp(args[0],"BEGINATC"))
			{
				// Skip signature
			}
			else if(0==strcmp(args[0],"DEFATC"))
			{
				atcKey=FsAirTrafficController::PrimaryAirTrafficControllerKey;
				// atc->AirplaneCheckIn will be done in ReconnectAtcAndApproach
			}
			else if(0==strcmp(args[0],"ATCNAME") && 2<=args.GetN())
			{
				atcNameCacheForFileIO=args[1];
				// atc->AirplaneCheckIn will be done in ReconnectAtcAndApproach
			}
			else if(0==strcmp(args[0],"FIX") && 5<=args.GetN())
			{
				path.Increment();
				const FsFlightPlanFix::FIXTYPE fixType=FsFlightPlanFix::StringToFixType(args[1]);

				double x,y,z;
				FsGetLength(x,args[2]);
				FsGetLength(y,args[3]);
				FsGetLength(z,args[4]);
				path.GetEnd().SetPosition(YsVec3(x,y,z),fixType);
			}
			else if(0==strcmp(args[0],"NEXTFIX") && 2<=args.GetN())
			{
				currentPathIndex=atoi(args[1]);
			}
			else if(0==strcmp(args[0],"APPROACH"))
			{
				if(YSOK!=approach.Load(sim,fp))
				{
					return YSERR;
				}
			}
			else if(0==strcmp(args[0],"ENDATC"))
			{
				return YSOK;
			}
		}
	}
	return YSERR;
}

YSRESULT FsAirTrafficInfo::ReconnectAtcAndApproach(FsSimulation *sim,FsAirplane *air)  // Used after Load
{
	YSRESULT res=YSOK;

	approach.ReconnectGroundFacility(sim);

	FsAirTrafficController *atc=NULL;
	if(0<atcNameCacheForFileIO.Strlen())
	{
		// In place for:
		// FsAirTrafficController *atc=sim->FindAirTrafficControllerByName(atcNameCacheForFileIO);
		// if(NULL!=atc)
		// {
		// 	atc->AirplaneCheckIn(sim,air);
		// }
		// else
		// {
		// 	res=YSERR;
		// }
	}
	else if(FsAirTrafficController::NullAirTrafficControllerKey!=atcKey)
	{
		atc=sim->FindAirTrafficController(atcKey);
	}

	if(NULL!=atc && FsApproach::APP_NULL!=approach.GetApproachType())
	{
		if(YSTRUE!=IsApproachAlreadySetUp())
		{
			auto *gndFacility=approach.GetFacility(sim);
			if(NULL!=gndFacility)
			{
				switch(approach.GetApproachType())
				{
				case FsApproach::APP_ILS:
					air->AtcRequestIlsApproach(sim,atc,gndFacility);
					return YSOK;
				default:
					break;
				}
			}
		}

		atc->AirplaneCheckIn(sim,air);
		return YSOK;
	}
	else
	{
		res=YSERR;
	}

	return res;
}
