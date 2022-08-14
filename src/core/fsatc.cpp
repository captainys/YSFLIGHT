#include <yscompilerwarning.h>
#include "fsatc.h"
#include "fsexistence.h"
#include "fssimulation.h"
#include "fsnavaid.h"
#include "ysunitconv.h"
#include "fsairsound.h"



void FsVoicePhraseAssembly::Clear(void)
{
	sentence.Clear();
}

void FsVoicePhraseAssembly::AddString(FSVOICE_PHRASE_TYPE phraseType,const char phrase[])
{
	sentence.Increment();
	sentence.GetEnd().phraseType=phraseType;
	sentence.GetEnd().english.Set(phrase);
}

void FsVoicePhraseAssembly::AddInt(FSVOICE_PHRASE_TYPE phraseType,int num)
{
	sentence.Increment();
	sentence.GetEnd().phraseType=phraseType;
	if(FSVOICE_HEADING!=phraseType)
	{
		sentence.GetEnd().english.Printf("%d",num);
	}
	else
	{
		sentence.GetEnd().english.Printf("%03d",num);
	}
}

void FsVoicePhraseAssembly::AddDouble(FSVOICE_PHRASE_TYPE phraseType,const double &num)
{
	sentence.Increment();
	sentence.GetEnd().phraseType=phraseType;
	sentence.GetEnd().english.Printf("%.0lf",num);
}

void FsVoicePhraseAssembly::AddCommaAndSpace(void)
{
	sentence.Increment();
	sentence.GetEnd().phraseType=FSVOICE_COMMA;
	sentence.GetEnd().english.Set(", ");
}

void FsVoicePhraseAssembly::AddSpace(void)
{
	sentence.Increment();
	sentence.GetEnd().phraseType=FSVOICE_COMMA;
	sentence.GetEnd().english.Set(" ");
}

void FsVoicePhraseAssembly::AddPeriod(void)
{
	sentence.Increment();
	sentence.GetEnd().phraseType=FSVOICE_COMMA;
	sentence.GetEnd().english.Set(".");
}

void FsVoicePhraseAssembly::AddEndOfSentence(void)
{
	sentence.Increment();
	sentence.GetEnd().phraseType=FSVOICE_END_OF_SENTENCE;
	sentence.GetEnd().english.Set("");
}

YSRESULT FsVoicePhraseAssembly::AssembleString(int &ptr,YsString &str) const
{
	if(sentence.GetN()<=ptr)
	{
		return YSERR;
	}

	str.Set("");
	for(; ptr<sentence.GetN(); ++ptr)
	{
		switch(sentence[ptr].phraseType)
		{
		case FSVOICE_END_OF_SENTENCE:
			++ptr;
			return YSOK;

		default:
		case FSVOICE_GENERAL:
		case FSVOICE_COMMA:  // This should be ignored by the voice dll.
		case FSVOICE_SPACE:  // This should be ignored by the voice dll.
		case FSVOICE_PERIOD: // This should be ignored by the voice dll.
		case FSVOICE_CALLSIGN:
		case FSVOICE_HEADING:
		case FSVOICE_GENERAL_NUMBER:
		case FSVOICE_ALTITUDE_IN_FEET:
		case FSVOICE_APPROACH_GENERAL:
			str.Append(sentence[ptr].english);
			break;
		case FSVOICE_ALTITUDE_IN_FLIGHT_LEVEL:
			str.Append("Flight Level ");
			str.Append(sentence[ptr].english);
			break;
		case FSVOICE_SPEED_IN_KNOT:
			str.Append(sentence[ptr].english);
			str.Append("kt");
			break;
		case FSVOICE_RUNWAY:
			str.Append("runway");
			str.Append(sentence[ptr].english);
			break;
		case FSVOICE_APPROACH_ILS:
			str.Append("ILS ");
			str.Append(sentence[ptr].english);
			break;
		case FSVOICE_APPROACH_VOR:
			str.Append("VOR ");
			str.Append(sentence[ptr].english);
			break;
		case FSVOICE_APPROACH_NDB:
			str.Append("NDB ");
			str.Append(sentence[ptr].english);
			break;
		case FSVOICE_APPROACH_GPS:
			str.Append("GPS ");
			str.Append(sentence[ptr].english);
			break;
		}
	}

	return YSOK;
}

////////////////////////////////////////////////////////////

const YSHASHKEY FsAirTrafficController::PrimaryAirTrafficControllerKey=0x6fffffff;
const YSHASHKEY FsAirTrafficController::NullAirTrafficControllerKey=0x6ffffffe;

const char *FsAirTrafficController::PrimaryAirTrafficControllerName="PrimaryAirTrafficController";

const double FsAirTrafficController::TerrainClearanceAtIAF(void)
{
	return YsUnitConv::FTtoM(2000.0);
}

const double FsAirTrafficController::TerrainClearanceApproach(void)
{
	return YsUnitConv::FTtoM(1500.0);
}

const double FsAirTrafficController::TimeToReduceSpeedBeforeIAF(void)
{
	return 60.0;  // 1 minute
}

const double FsAirTrafficController::GetRadiusBeginDescendFromEnroute(void)
{
	return YsUnitConv::NMtoM(10.0); // nm to the initial approach fix where to begin descend
}

FsAirTrafficController::FsAirTrafficController()
{
	searchKey=NullAirTrafficControllerKey;
	CleanUp();
}

void FsAirTrafficController::AssignSearchKey(unsigned int searchKeyIn)
{
	searchKey=searchKeyIn;
}

void FsAirTrafficController::CleanUp(void)
{
	jurisdictionFlag=0;
	needNoticeSound=YSFALSE;
	name.Set("");
	talkingAirKey.ClearDeep();
}

const char *FsAirTrafficController::GetName(void) const
{
	return name;
}

YSBOOL FsAirTrafficController::GetAndClearNeedNoticeFlag(void)
{
	YSBOOL ret=needNoticeSound;
	needNoticeSound=YSFALSE;
	return ret;
}

void FsAirTrafficController::Process(class FsSimulation *sim,const double &currentTime)
{
	for(YSSIZE_T i=0; i<talkingAirKey.GetN(); i++)
	{
		FsAirplane *air=sim->FindAirplane(talkingAirKey[i]);
		if(NULL!=air && YSTRUE!=air->isPlayingRecord)
		{
			FsAirTrafficInfo &airTrafficInfo=air->GetAirTrafficInfo();

			const FsApproach &app=airTrafficInfo.GetApproach();
			if(app.GetApproachType()!=FsApproach::APP_NULL)
			{
				ProcessApproach(sim,currentTime,air);
			}
		}
	}
}

void FsAirTrafficController::ProcessApproach(class FsSimulation *sim,const double &currentTime,FsAirplane *air)
{
	FsAirTrafficInfo &airTrafficInfo=air->GetAirTrafficInfo();

	if(FsAirTrafficInfo::STAGE_NEEDCALC==airTrafficInfo.GetStage())
	{
		airTrafficInfo.CalculateStage(air);
	}

	switch(airTrafficInfo.GetStage())
	{
	default:
		break;
	case FsAirTrafficInfo::STAGE_ONGROUND:
		ProcessApproach_OnGround(sim,currentTime,air);
		break;
	case FsAirTrafficInfo::STAGE_ENROUTE:
	case FsAirTrafficInfo::STAGE_APPROACH:
		ProcessApproach_Approach(sim,currentTime,air);
		break;
	}
}

void FsAirTrafficController::ProcessApproach_OnGround(class FsSimulation *sim,const double &currentTime,FsAirplane *air)
{
	FsAirTrafficInfo &airTrafficInfo=air->GetAirTrafficInfo();

	if(0.0>airTrafficInfo.GetLastAtcContactTime())
	{
		FsVoicePhraseAssembly phrase;
		SetPlayerCallSignMessage(phrase,*air);

		const double alt=RoundUpAltitude(TerrainClearanceApproach()+air->elevation,sim->GetBaseElevation(),100.0);
		const double mslInFt=YsUnitConv::MtoFT(alt+sim->GetBaseElevation());

		phrase.AddString(FSVOICE_GENERAL,"Fly runway heading, Climb maintain ");
		phrase.AddDouble(FSVOICE_ALTITUDE_IN_FEET,mslInFt);
		phrase.AddCommaAndSpace();
		phrase.AddString(FSVOICE_GENERAL,"Cleared for Take Off.");

		ShowMessage(sim,phrase);

		airTrafficInfo.AssignAltitude(alt);
		airTrafficInfo.SetLastAtcContactTime(currentTime);
	}
	else if(airTrafficInfo.GetAssignedAltitude()<air->GetPosition().y()+YsUnitConv::FTtoM(50.0))
	{
		airTrafficInfo.CalculateStage(air);
		airTrafficInfo.SetLastAtcContactTime(-1.0);
	}
}

void FsAirTrafficController::SetPlayerCallSignMessage(FsVoicePhraseAssembly &phrase,const FsAirplane &air)
{
	if(NULL==air.GetName() || 0==air.GetName()[0])
	{
		phrase.AddString(FSVOICE_CALLSIGN,"Player Aircraft");
		phrase.AddCommaAndSpace();
	}
	else
	{
		phrase.AddString(FSVOICE_CALLSIGN,air.GetName());
		phrase.AddCommaAndSpace();
	}
}

void FsAirTrafficController::ProcessApproach_Approach(class FsSimulation *sim,const double &currentTime,class FsAirplane *air)
{
	FsAirTrafficInfo &airTrafficInfo=air->GetAirTrafficInfo();
	const FsApproach &app=airTrafficInfo.GetApproach();

	const FsFlightPlanFix *currentFix=airTrafficInfo.GetNextWayPoint();  // A fix to which the airplane is currently heading.

	YSBOOL giveHeading=YSFALSE,giveAltitude=YSFALSE,giveAirSpeed=YSFALSE,isInitial=YSFALSE,giveVectorToFinal=YSFALSE;
	const double lastContactTime=airTrafficInfo.GetLastAtcContactTime();
	const double nextHeadingCheckTime=airTrafficInfo.GetNextHeadingCheckTime();
	if(0.0>lastContactTime)
	{
		isInitial=YSTRUE;
		giveHeading=YSTRUE;
		giveAltitude=YSTRUE;
		giveAirSpeed=YSFALSE;
		giveVectorToFinal=YSFALSE;

		printf("Initial Contact\n");
	}
	else if(YSTRUE==airTrafficInfo.AirplaneReachTurnPoint(air))
	{
		airTrafficInfo.IncrementWayPoint();
		giveHeading=YSTRUE;
		giveAltitude=YSTRUE;
		if(airTrafficInfo.GetAssignedAirSpeed()+YsUnitConv::KTtoMPS(10.0)<air->Prop().GetVelocity())
		{
			giveAirSpeed=YSTRUE;
		}

		printf("Turn point\n");
	}
	else if(nextHeadingCheckTime<currentTime && YSTRUE==airTrafficInfo.HeadingDeviationExceed(sim,air,YsPi/18.0))
	{
		giveHeading=YSTRUE;

		printf("Heading deviation\n");
	}
	else if(lastContactTime+20.0<currentTime && YSTRUE==airTrafficInfo.AirplaneMessedUp(air))
	{
		giveHeading=YSTRUE;
		giveAltitude=YSTRUE;
		giveAirSpeed=YSTRUE;

		printf("Messed up\n");
	}

	if(FsAirTrafficInfo::STAGE_ENROUTE==airTrafficInfo.GetStage())
	{
		airTrafficInfo.CalculateStage(air);
		if(FsAirTrafficInfo::STAGE_ENROUTE!=airTrafficInfo.GetStage())
		{
			giveAltitude=YSTRUE;
		}
	}

	const FsFlightPlanFix *nextFix=airTrafficInfo.GetNextWayPoint();

	if((YSTRUE!=giveAirSpeed || YSTRUE!=giveVectorToFinal) &&
	   (NULL==currentFix || YSTRUE!=currentFix->IsApproachFix()) &&
	   (NULL!=nextFix && YSTRUE==nextFix->IsApproachFix()))
	   // If the airplane is approaching the initial approach fix, then assign airspeed.
	{
		YsVec3 nextFixPos=nextFix->GetPosition();
		YsVec3 curPos=air->GetPosition();

		nextFixPos.SetY(0.0);
		curPos.SetY(0.0);

		const double distToNextFix=(nextFixPos-curPos).GetLength();
		const double v=air->Prop().GetVelocity();

		if(YsTolerance<v && distToNextFix/v<TimeToReduceSpeedBeforeIAF())
		{
			const double Vapproach=air->Prop().GetEstimatedLandingSpeed()*1.5;
			if(YSTRUE==YsEqual(0.0,airTrafficInfo.GetAssignedAirSpeed()) ||
			   airTrafficInfo.GetAssignedAirSpeed()>Vapproach+YsTolerance)
			{
				giveAirSpeed=YSTRUE;
				giveVectorToFinal=YSTRUE;
			}
		}
	}


	auto nextAlt=airTrafficInfo.GetAssignedAltitude();
	{
		if(FsAirTrafficInfo::STAGE_ENROUTE==airTrafficInfo.GetStage())
		{
			auto cruiseAltInFt=YsUnitConv::MtoFT(air->Prop().GetReferenceCruisingAltitude());
			double altEast=0.0,altWest=0.0;
			if(35000.0<=cruiseAltInFt)
			{
				altWest=34000.0;
				altEast=35000.0;
			}
			else if(31000.0<=cruiseAltInFt)
			{
				altWest=30000.0;
				altEast=31000.0;
			}
			else if(21000.0<=cruiseAltInFt)
			{
				altWest=20000.0;
				altEast=21000.0;
			}
			else if(18000.0<=cruiseAltInFt)
			{
				altWest=18000.0;
				altEast=19000.0;
			}
			else if(15000.0<=cruiseAltInFt)
			{
				altWest=14000.0;
				altEast=15000.0;
			}
			else
			{
				altWest=10000.0;
				altEast=9000.0;
			}

			double enrouteAlt=0.0;
			if(0.0<=cos(airTrafficInfo.GetAssignedHeading()))  // Odd number
			{
				enrouteAlt=YsUnitConv::FTtoM(altEast);
			}
			else
			{
				enrouteAlt=YsUnitConv::FTtoM(altWest);
			}
			enrouteAlt-=sim->GetBaseElevation();
			if(nextAlt<enrouteAlt)
			{
				nextAlt=enrouteAlt;
			}

			// Assume 1000ft/min descend.
			// If the next fix is an approach fix, calculate the time required to descend from the cruising altitude to the next altitude.
			// Then calculate the distance D.
			// At D, descend half way.
			// At D/2, descend 2000ft above the initial approach fix.
			if((NULL!=currentFix && YSTRUE==currentFix->IsApproachFix()) ||
			   (NULL!=nextFix && YSTRUE==nextFix->IsApproachFix()))
			{
				// Flying to the initial approach fix.
				const double altAtIAP=nextFix->GetPosition().y()+TerrainClearanceAtIAF();
				const double toDescend=fabs(enrouteAlt-altAtIAP);
				const double descendRate=YsUnitConv::FTtoM(1000.0)/60.0;
				const double descendTime=toDescend/descendRate;
				const double descendDist=descendTime*air->Prop().GetEstimatedCruiseSpeed();
				double wantToDescendTo=airTrafficInfo.GetAssignedAltitude();
				const double currentDist=(air->GetPosition().xz()-nextFix->GetPosition().xz()).GetLength();

				if(currentDist<descendDist)
				{
					if(currentDist<descendDist/2.0)
					{
						wantToDescendTo=altAtIAP+YsUnitConv::FTtoM(2000.0);
					}
					else if(currentDist<descendDist)
					{
						wantToDescendTo=(altAtIAP+enrouteAlt)/2.0;
					}

					const double maxElv=airTrafficInfo.GetMaxElevationOfLeg(sim,air->GetPosition(),nextFix->GetPosition(),500.0)+TerrainClearanceAtIAF();
					nextAlt=YsGreater(maxElv,wantToDescendTo);
					nextAlt=RoundUpAltitude(nextAlt,sim->GetBaseElevation(),1000.0);
				}
			}
		}
		else
		{
			if((NULL==currentFix || YSTRUE!=currentFix->IsApproachFix()) &&
			   (NULL!=nextFix && YSTRUE==nextFix->IsApproachFix()))
			{
				// Flying to the initial approach fix.
				const double maxElv=airTrafficInfo.GetMaxElevationOfLeg(sim,air->GetPosition(),nextFix->GetPosition(),500.0);
				nextAlt=maxElv+TerrainClearanceAtIAF();
				nextAlt=RoundUpAltitude(nextAlt,sim->GetBaseElevation(),100.0);
			}
			else
			{
				nextAlt=nextFix->GetPosition().y();
			}
		}

		auto diffAlt=fabs(airTrafficInfo.GetAssignedAltitude()-nextAlt);
		if(500.0<=YsUnitConv::MtoFT(diffAlt))
		{
			giveAltitude=YSTRUE;
		}
	}



	FsVoicePhraseAssembly phrase;


	if(NULL!=nextFix && (YSTRUE==giveHeading || YSTRUE==giveAltitude || YSTRUE==giveAirSpeed))
	{
		SetPlayerCallSignMessage(phrase,*air);

		if(YSTRUE==giveHeading)
		{
			double magHdg,timeForTurn;
			if(YSOK==AddHeadingMessage(magHdg,timeForTurn,phrase,sim,*air,nextFix->GetPosition()))
			{
				airTrafficInfo.AssignHeading(magHdg);
				const double atcForgiveness=YsGreater(timeForTurn,10.0);
				airTrafficInfo.SetNextHeadingCheckTime(currentTime+atcForgiveness);
			}
			else
			{
				// Hold for now.  Need speed reduction, or the turn radius is too big.
				phrase.AddString(FSVOICE_GENERAL,"Reduce Speed to ");
				phrase.AddInt(FSVOICE_SPEED_IN_KNOT,200);
				phrase.AddSpace();
				phrase.AddString(FSVOICE_GENERAL,"or slower.");

				ShowMessage(sim,phrase);
				airTrafficInfo.SetLastAtcContactTime(currentTime);
				return;
			}
		}

		if(YSTRUE==giveAltitude)
		{
			AddAltitudeMessage(nextAlt,phrase,sim,*air,nextAlt);
			airTrafficInfo.AssignAltitude(nextAlt);
		}

		if(YSTRUE==giveAirSpeed)
		{
			const double Vfinal=air->Prop().GetEstimatedLandingSpeed();
			double speedToAssign=0.0;

			switch(nextFix->GetFixType())
			{
			case FsFlightPlanFix::FIX_NULL:
				speedToAssign=0.0;  // Dont' assign
				break;
			case FsFlightPlanFix::FIX_LOCALIZERINTERCEPT:
			case FsFlightPlanFix::FIX_GLIDESLOPEINTERCEPT:
				speedToAssign=Vfinal*1.05;
				break;
			case FsFlightPlanFix::FIX_DOGLEGENTRY:
				speedToAssign=Vfinal*1.2;
				break;
			default:  // For other approach fixes
				speedToAssign=Vfinal*1.5;
				break;
			}

			if(YSTRUE!=YsEqual(0.0,speedToAssign))
			{
				if(air->Prop().GetVelocity()>speedToAssign+YsUnitConv::KTtoMPS(10.0))
				{
					phrase.AddString(FSVOICE_GENERAL,"Reduce speed ");
				}
				else if(air->Prop().GetVelocity()<speedToAssign-YsUnitConv::KTtoMPS(10.0))
				{
					phrase.AddString(FSVOICE_GENERAL,"Increase speed ");
				}
				else
				{
					phrase.AddString(FSVOICE_GENERAL,"Maintain ");
				}

				phrase.AddDouble(FSVOICE_SPEED_IN_KNOT,YsUnitConv::MPStoKT(speedToAssign));
				phrase.AddPeriod();
			}

			airTrafficInfo.AssignAirSpeed(speedToAssign);
		}
		else
		{
			airTrafficInfo.AssignAirSpeed(0.0);
		}

		phrase.AddEndOfSentence();

		if(FsFlightPlanFix::FIX_GLIDESLOPEINTERCEPT==nextFix->GetFixType() ||
		   FsFlightPlanFix::FIX_LOCALIZERINTERCEPT==nextFix->GetFixType())
		{
			switch(app.GetApproachType())
			{
			case FsApproach::APP_VISUAL:
				phrase.AddString(FSVOICE_GENERAL,"Cleared for Visual Approach.");
				phrase.AddEndOfSentence();
				break;
			case FsApproach::APP_ILS:
				phrase.AddString(FSVOICE_GENERAL,"Intercept Localizer.");
				phrase.AddEndOfSentence();
				phrase.AddString(FSVOICE_GENERAL,"Cleared for ");
				if(NULL!=app.GetFacility(sim))
				{
					phrase.AddString(FSVOICE_APPROACH_ILS,app.GetFacility(sim)->GetName());
					phrase.AddSpace();
				}
				phrase.AddString(FSVOICE_GENERAL,"approach.");
				break;
			default:
				break;
			}
			airTrafficInfo.SetStage(FsAirTrafficInfo::STAGE_FINAL);
			isInitial=YSFALSE; // Don't say "Expect ...."
		}

		phrase.AddEndOfSentence();

		airTrafficInfo.SetLastAtcContactTime(currentTime);
	}

	if(YSTRUE==isInitial)
	{
		switch(app.GetApproachType())
		{
		case FsApproach::APP_ILS:
			phrase.AddString(FSVOICE_GENERAL,"Expect ");
			if(NULL!=app.GetFacility(sim))
			{
				phrase.AddString(FSVOICE_APPROACH_ILS,app.GetFacility(sim)->GetName());
				phrase.AddSpace();
			}
			phrase.AddString(FSVOICE_GENERAL,"approach.");
			break;
		default:
			break;
		}
	}

	if(YSTRUE==giveVectorToFinal)
	{
		switch(app.GetApproachType())
		{
		case FsApproach::APP_ILS:
			phrase.AddString(FSVOICE_GENERAL,"Vector for ");
			if(NULL!=app.GetFacility(sim))
			{
				phrase.AddString(FSVOICE_APPROACH_ILS,app.GetFacility(sim)->GetName());
				phrase.AddSpace();
			}
			phrase.AddString(FSVOICE_GENERAL,"approach.");
			break;
		default:
			break;
		}
	}

	ShowMessage(sim,phrase);
}

YSRESULT FsAirTrafficController::AddHeadingMessage(double &magHdg,double &timeForTurn,FsVoicePhraseAssembly &phrase,const class FsSimulation *sim,class FsAirplane &air,const YsVec3 &wayPoint)
{
	YsVec3 newVec,curVel;
	double Rturn;
	air.Prop().GetVelocity(curVel);
	if(YSOK==GetNewVectorToWayPoint(newVec,Rturn,air.GetPosition(),curVel,wayPoint))
	{
		magHdg=VectorToMagneticHeading(sim,newVec);
		printf("New Vector: %s\n",newVec.Txt());

		const char *turn="Fly heading ";

		YsVec3 relNewVec;
		air.GetAttitude().MulInverse(relNewVec,newVec);
		if(relNewVec.x()<0.0)
		{
			turn="turn left heading ";
		}
		else
		{
			turn="turn right heading ";
		}

		const double turnAngle=fabs(atan2(relNewVec.x(),relNewVec.z()));
		const double curV=curVel.GetLength();
		// V=R*Omega
		// Omega=V/R

		// Standard turn=360deg/120seconds=3deg/sec.
		if(0.0<Rturn && 0.0<curV)
		{
			const double omega=curV/Rturn;
			timeForTurn=turnAngle/omega;
		}
		else
		{
			timeForTurn=turnAngle/YsDegToRad(3.0);
		}
		printf("Time for turn=%lf\n",timeForTurn);

		double magHdgInDeg=YsRadToDeg(magHdg);
		if(0.0>magHdgInDeg)
		{
			magHdgInDeg+=360.0;
		}

		int magHdgInDegInt=(int)(magHdgInDeg+2.5);
		magHdgInDegInt/=5;
		magHdgInDegInt*=5;
		if(0==magHdgInDegInt)
		{
			magHdgInDegInt=360;
		}

		phrase.AddString(FSVOICE_GENERAL,turn);
		phrase.AddInt(FSVOICE_HEADING,magHdgInDegInt);
		phrase.AddCommaAndSpace();

		return YSOK;
	}
	return YSERR;
}

YSRESULT FsAirTrafficController::AddAltitudeMessage(double &alt,FsVoicePhraseAssembly &phrase,const class FsSimulation *sim,class FsAirplane &air,const double y)
{
	const double baseElv=sim->GetBaseElevation();

	const char *climb="Maintain ";
	const double airY=air.GetPosition().y();
	if(airY+YsUnitConv::FTtoM(100.0)<y)
	{
		climb="Climb maintain ";
	}
	else if(airY-YsUnitConv::FTtoM(100.0)>y)
	{
		climb="Descend maintain ";
	}

	double altInFt=YsUnitConv::MtoFT(y+baseElv);
	int flightLevel=(int)(altInFt/100.0);
	flightLevel=((flightLevel+4)/10)*10;

	if(flightLevel<180)
	{
		phrase.AddString(FSVOICE_GENERAL,climb);
		phrase.AddDouble(FSVOICE_ALTITUDE_IN_FEET,altInFt);
		phrase.AddCommaAndSpace();
	}
	else
	{
		phrase.AddString(FSVOICE_GENERAL,climb);
		phrase.AddInt(FSVOICE_ALTITUDE_IN_FLIGHT_LEVEL,flightLevel);
		phrase.AddCommaAndSpace();
	}

	alt=y;

	return YSOK;
}

void FsAirTrafficController::ShowMessage(class FsSimulation *sim,const FsVoicePhraseAssembly &phrase)
{
	YsString msg;
	for(int ptr=0; YSOK==phrase.AssembleString(ptr,msg); )
	{
		sim->AddTimedMessage(msg);
		needNoticeSound=YSTRUE;  // Turn on only if at least one message has been added.
	}

	YsArray <struct FsVoicePhrase,32> speechPhrase;
	phrase.AssemblePhraseForSpeech(speechPhrase);
	if(0<speechPhrase.GetN())
	{
		FsVoiceSpeak((int)speechPhrase.GetN(),speechPhrase);
	}
}

unsigned int FsAirTrafficController::SearchKey(void) const
{
	return searchKey;
}

void FsAirTrafficController::SetName(const char str[])
{
	name.Set(str);
}

void FsAirTrafficController::SetAllJurisdiction(void)
{
	jurisdictionFlag=(unsigned int)JURISDICTION_ALL;
}

void FsAirTrafficController::SetIsEnrouteController(YSBOOL tf)
{
	if(YSTRUE==tf)
	{
		jurisdictionFlag|=JURISDICTION_ENROUTE;
	}
	else
	{
		jurisdictionFlag&=~JURISDICTION_ENROUTE;
	}
}

void FsAirTrafficController::SetIsApproachController(YSBOOL tf)
{
	if(YSTRUE==tf)
	{
		jurisdictionFlag|=JURISDICTION_APPROACH;
	}
	else
	{
		jurisdictionFlag&=~JURISDICTION_APPROACH;
	}
}

void FsAirTrafficController::SetIsTowerController(YSBOOL tf)
{
	if(YSTRUE==tf)
	{
		jurisdictionFlag|=JURISDICTION_TOWER;
	}
	else
	{
		jurisdictionFlag&=~JURISDICTION_TOWER;
	}
}

void FsAirTrafficController::SetIsGroundController(YSBOOL tf)
{
	if(YSTRUE==tf)
	{
		jurisdictionFlag|=JURISDICTION_GROUND;
	}
	else
	{
		jurisdictionFlag&=~JURISDICTION_GROUND;
	}
}

YSBOOL FsAirTrafficController::IsEnrouteController(void) const
{
	return (0!=(jurisdictionFlag&JURISDICTION_ENROUTE) ? YSTRUE : YSFALSE);
}

YSBOOL FsAirTrafficController::IsApproachController(void) const
{
	return (0!=(jurisdictionFlag&JURISDICTION_APPROACH) ? YSTRUE : YSFALSE);
}

YSBOOL FsAirTrafficController::IsTowerController(void) const
{
	return (0!=(jurisdictionFlag&JURISDICTION_TOWER) ? YSTRUE : YSFALSE);
}

YSBOOL FsAirTrafficController::IsGroundController(void) const
{
	return (0!=(jurisdictionFlag&JURISDICTION_GROUND) ? YSTRUE : YSFALSE);
}

YSRESULT FsAirTrafficController::AirplaneCheckIn(class FsSimulation *sim,class FsAirplane *air)
{
	if(NULL!=air)
	{
		FsAirTrafficInfo &airTrafficInfo=air->GetAirTrafficInfo();
		FsAirTrafficController *const currentAtc=airTrafficInfo.GetAirTrafficController(sim);
		if(this!=currentAtc)
		{
			if(NULL!=currentAtc)
			{
				currentAtc->RemoveAirplane(sim,air);
			}
			AddAirplane(sim,air);
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsAirTrafficController::AirplaneRequestApproachByILS(class FsSimulation *sim,class FsAirplane &air,const class FsGround *ils,const char dstAirportName[])
{
	FsAirTrafficInfo &airTrafficInfo=air.GetAirTrafficInfo();
	FsAirTrafficController *const currentAtc=airTrafficInfo.GetAirTrafficController(sim);
	if(this==currentAtc)
	{
		return airTrafficInfo.SelectApproachByILS(sim,air,ils,dstAirportName);
	}
	return YSERR;
}

YSRESULT FsAirTrafficController::AirplaneRequestHeading(class FsSimulation *sim,class FsAirplane &air)
{
	FsAirTrafficInfo &airTrafficInfo=air.GetAirTrafficInfo();
	const FsFlightPlanFix *nextFix=airTrafficInfo.GetNextWayPoint();
	if(NULL!=nextFix)
	{
		FsVoicePhraseAssembly phrase;
		double magHdg,timeForTurn;
		if(YSOK==AddHeadingMessage(magHdg,timeForTurn,phrase,sim,air,nextFix->GetPosition()))
		{
			ShowMessage(sim,phrase);
			airTrafficInfo.AssignHeading(magHdg);

			const double atcForgiveness=YsBound(timeForTurn,10.0,45.0);
			airTrafficInfo.SetNextHeadingCheckTime(sim->currentTime+atcForgiveness);

			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsAirTrafficController::AirplaneRequestNewVector(class FsSimulation *sim,class FsAirplane &air)
{
	FsAirTrafficInfo &airTrafficInfo=air.GetAirTrafficInfo();

	const FsFlightPlanFix *nextFix=airTrafficInfo.GetNextWayPoint();
	if(nullptr!=nextFix && YSTRUE==nextFix->IsApproachFix())
	{
		airTrafficInfo.ResetApproach(sim,air);
	}

	nextFix=airTrafficInfo.GetNextWayPoint();
	if(nullptr!=nextFix)
	{
		FsVoicePhraseAssembly phrase;
		double magHdg,timeForTurn;
		if(YSOK==AddHeadingMessage(magHdg,timeForTurn,phrase,sim,air,nextFix->GetPosition()))
		{
			airTrafficInfo.AssignHeading(magHdg);

			const double atcForgiveness=YsBound(timeForTurn,10.0,45.0);
			airTrafficInfo.SetNextHeadingCheckTime(sim->currentTime+atcForgiveness);

			const double maxElv=airTrafficInfo.GetMaxElevationOfLeg(sim,air.GetPosition(),nextFix->GetPosition(),500.0);
			double alt=maxElv+TerrainClearanceAtIAF();
			alt=RoundUpAltitude(alt,sim->GetBaseElevation(),500.0);

			phrase.AddSpace();
			if(YSOK==AddAltitudeMessage(alt,phrase,sim,air,alt))
			{
				airTrafficInfo.AssignAltitude(alt);
			}

			ShowMessage(sim,phrase);
			return YSOK;
		}
	}

	return YSERR;
}

YSRESULT FsAirTrafficController::AirplaneDeclareMissedApproach(class FsSimulation *sim,class FsAirplane &air)
{
	FsAirTrafficInfo &airTrafficInfo=air.GetAirTrafficInfo();
	airTrafficInfo.ResetApproach(sim,air);
	return AirplaneRequestNewVector(sim,air);
}

YSRESULT FsAirTrafficController::AirplaneCancelIFR(class FsSimulation *sim,class FsAirplane &air)
{
	FsVoicePhraseAssembly phrase;
	phrase.AddString(FSVOICE_GENERAL,"IFR Cancellation Received.");
	phrase.AddEndOfSentence();
	phrase.AddString(FSVOICE_GENERAL,"Frequency change approved. Squawk VFR.");
	ShowMessage(sim,phrase);
	RemoveAirplane(sim,&air);

	return YSOK;
}

YSRESULT FsAirTrafficController::RemoveAirplane(class FsSimulation *,class FsAirplane *air)
{
	if(NULL!=air)
	{
		FsAirTrafficInfo &airTrafficInfo=air->GetAirTrafficInfo();

		for(YSSIZE_T i=talkingAirKey.GetN()-1; i>=0; i--)
		{
			if(talkingAirKey[i]==air->SearchKey())
			{
				talkingAirKey.DeleteBySwapping(i);
				airTrafficInfo.StopTalkingWithAirTrafficController();
			}
		}
	}
	return YSOK;
}

YSRESULT FsAirTrafficController::AddAirplane(class FsSimulation *sim,class FsAirplane *air)
{
	if(NULL!=air)
	{
		FsAirTrafficInfo &airTrafficInfo=air->GetAirTrafficInfo();
		FsAirTrafficController *const currentAtc=airTrafficInfo.GetAirTrafficController(sim);

		if(NULL!=currentAtc)
		{
			currentAtc->RemoveAirplane(sim,air);
		}

		for(YSSIZE_T i=talkingAirKey.GetN()-1; i>=0; --i)
		{
			if(talkingAirKey[i]==air->SearchKey())
			{
				return YSOK; // Already talking
			}
		}

		talkingAirKey.Append(air->SearchKey());
		airTrafficInfo.SetAirTrafficController(sim,this);
	}
	return YSOK;
}

YSRESULT FsAirTrafficController::GetNewVectorToWayPoint(
   YsVec3 &newVec,double &Rturn,const YsVec3 &curAirPos,const YsVec3 &curAirVel,const YsVec3 &wayPoint)
{
	// See AirTrafficController.docx
	const double Vair=curAirVel.GetLength();
	Rturn=FsGetLevelTurnRadius(Vair,YsPi/9.0);

	YsVec3 sideVec(-curAirVel.z(),0.0,curAirVel.x());
	if(YSOK==sideVec.Normalize())
	{
		if(0.0>sideVec*(wayPoint-curAirPos))
		{
			sideVec=-sideVec;
		}

		const YsVec3 Cturn=curAirPos+sideVec*Rturn;
		YsVec3 Vcw=wayPoint-Cturn;
		const double d=Vcw.GetLength();
		if(Rturn<d)  // Otherwise, waypoint is within turning radius.  Never be able to make it.
		{
			Vcw/=d;

			const double theata=asin(Rturn/d);
			YsVec3 V[2]={Vcw,Vcw};
			V[0].RotateXZ( theata);
			V[1].RotateXZ(-theata);

			const YsVec3 Pintercept[2]={wayPoint-V[0]*d*cos(theata),wayPoint-V[1]*d*cos(theata)};

			const double crsRef=(curAirVel^(Cturn-curAirPos)).y();
			const double tstRef[2]=
			{
				((Pintercept[0]-Cturn)^V[0]).y(),
				((Pintercept[1]-Cturn)^V[1]).y()
			};

			if(0.0<crsRef*tstRef[0])
			{
				newVec=V[0];
				return YSOK;
			}
			else
			{
				newVec=V[1];
				return YSOK;
			}
		}
	}
	return YSERR;
}

double FsAirTrafficController::RoundUpAltitude(const double &alt,const double &baseElv,const double &denom)
{
	double msl=YsUnitConv::MtoFT(alt+baseElv)+denom-1.0;
	msl=msl-fmod(msl,denom);
	msl=YsUnitConv::FTtoM(msl);
	return msl-baseElv;
}

double FsAirTrafficController::VectorToMagneticHeading(const class FsSimulation *sim,const YsVec3 &vec)
{
	return sim->TrueHeadingToMagneticHeading(atan2(vec.x(),vec.z()));
}

////////////////////////////////////////////////////////////

FsFlightPlanFix::FIXTYPE FsFlightPlanFix::StringToFixType(const char str[])
{
	if(0==strcmp("NULL",str))
	{
		return FIX_NULL;
	}
	if(0==strcmp("GROUND",str))
	{
		return FIX_GROUND;
	}
	if(0==strcmp("DONWINDENTRY",str))
	{
		return FIX_DONWINDENTRY;
	}
	if(0==strcmp("BASEENTRY",str))
	{
		return FIX_BASEENTRY;
	}
	if(0==strcmp("DOGLEGENTRY",str))
	{
		return FIX_DOGLEGENTRY;
	}
	if(0==strcmp("LOCALIZERINTERCEPT",str))
	{
		return FIX_LOCALIZERINTERCEPT;
	}
	if(0==strcmp("GLIDESLOPEINTERCEPT",str))
	{
		return FIX_GLIDESLOPEINTERCEPT;
	}
	if(0==strcmp("WAYPOINT",str))
	{
		return FIX_WAYPOINT;
	}
	return FIX_NULL;
}

const char *FsFlightPlanFix::FixTypeToString(FsFlightPlanFix::FIXTYPE fixType)
{
	switch(fixType)
	{
	case FIX_NULL:
		return "NULL";
	case FIX_GROUND:
		return "GROUND";
	case FIX_DONWINDENTRY:
		return "DONWINDENTRY";
	case FIX_BASEENTRY:
		return "BASEENTRY";
	case FIX_DOGLEGENTRY:
		return "DOGLEGENTRY";
	case FIX_LOCALIZERINTERCEPT:
		return "LOCALIZERINTERCEPT";
	case FIX_GLIDESLOPEINTERCEPT:
		return "GLIDESLOPEINTERCEPT";
	case FIX_WAYPOINT:
		return "WAYPOINT";
	default:
		break;
	}
	return "NULL";
}

FsFlightPlanFix::FsFlightPlanFix()
{
	CleanUp();
}

void FsFlightPlanFix::CleanUp()
{
	fixType=FIX_NULL;
	pos=YsOrigin();
	gndKey=YSNULLHASHKEY;
}

void FsFlightPlanFix::SetPositionFromFacility(const class FsSimulation *,const class FsGround *gnd)
{
	pos=gnd->GetPosition();
	gndKey=gnd->SearchKey();
	fixType=FIX_GROUND;
}

class FsGround *FsFlightPlanFix::GetFacility(const class FsSimulation *sim)
{
	if(FIX_GROUND==fixType)
	{
		return sim->FindGround(gndKey);
	}
	return NULL;
}


void FsFlightPlanFix::SetPosition(const YsVec3 &posIn,FIXTYPE fixTypeIn)
{
	pos=posIn;
	fixType=fixTypeIn;
	gndKey=YSNULLHASHKEY;
}

const YsVec3 &FsFlightPlanFix::GetPosition(void) const
{
	return pos;
}


FsFlightPlanFix::FIXTYPE FsFlightPlanFix::GetFixType(void) const
{
	return fixType;
}

const char *FsFlightPlanFix::GetFixString(void) const
{
	return FixTypeToString(fixType);
}

YSBOOL FsFlightPlanFix::IsApproachFix(void) const
{
	if(fixType==FIX_DONWINDENTRY ||
	   fixType==FIX_BASEENTRY ||
	   fixType==FIX_DOGLEGENTRY ||
	   fixType==FIX_LOCALIZERINTERCEPT ||
	   fixType==FIX_GLIDESLOPEINTERCEPT ||
	   fixType==FIX_CUSTOM_APPROACH_FIX)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

////////////////////////////////////////////////////////////

const char *FsApproach::AppTypeToString(FsApproach::APPROACHTYPE appType)
{
	switch(appType)
	{
	case APP_NULL:
		return "NULL";
	case APP_VISUAL:
		return "VISUAL";
	case APP_ILS:
		return "ILS";
	}
	return "NULL";
}

FsApproach::APPROACHTYPE FsApproach::StringToAppType(const char str[])
{
	if(0==strcmp("NULL",str))
	{
		return APP_NULL;
	}
	if(0==strcmp("VISUAL",str))
	{
		return APP_VISUAL;
	}
	if(0==strcmp("ILS",str))
	{
		return APP_ILS;
	}
	return APP_NULL;
}

FsApproach::FsApproach()
{
	CleanUp();
}

void FsApproach::CleanUp(void)
{
	appType=APP_NULL;
	dstName.Set("");
	gndKey=YSNULLHASHKEY;
	gndNameCacheForFileIO.Set("");
}

YSBOOL FsAirTrafficInfo::IsApproachAlreadySetUp(void) const
{
	if(0<=currentPathIndex && YSTRUE==path.IsInRange(currentPathIndex))
	{
		return YSTRUE;
	}
	return YSFALSE;
}


const char *FsApproach::GetDestinationName(void) const
{
	return dstName;
}

class FsGround *FsApproach::GetFacility(const class FsSimulation *sim) const
{
	return sim->FindGround(gndKey);
}

FsApproach::APPROACHTYPE FsApproach::GetApproachType(void) const
{
	return appType;
}

const char *FsApproach::GetApproachTypeString(void) const
{
	return AppTypeToString(appType);
}

void FsApproach::SetDestinationName(const char dstNameIn[])
{
	dstName.Set(dstNameIn);
}

void FsApproach::SetFacility(const class FsSimulation *,const FsGround *gnd)
{
	gndKey=gnd->SearchKey();
}

void FsApproach::SetApproachType(FsApproach::APPROACHTYPE appTypeIn)
{
	appType=appTypeIn;
}


////////////////////////////////////////////////////////////

FsAirTrafficInfo::FsAirTrafficInfo()
{
	CleanUp();
}

FsAirTrafficInfo::~FsAirTrafficInfo()
{
	CleanUp();
}

void FsAirTrafficInfo::CleanUp(void)
{
	talkingWithAtc=YSFALSE;
	atcKey=FsAirTrafficController::NullAirTrafficControllerKey;
	currentPathIndex=0;
	clockLastInstructionFromAtc=-1.0;
	path.ClearDeep();
	IFRorVFR=FS_VFR;
	stage=STAGE_NEEDCALC;
	approach.CleanUp();

	assignedAltitude=0.0;
	assignedHeading=0.0;
	assignedAirSpeed=0.0;

	atcNameCacheForFileIO.Set("");
}


void FsAirTrafficInfo::SetAirTrafficController(class FsSimulation *,FsAirTrafficController *atc)
{
	atcKey=atc->SearchKey();
	talkingWithAtc=YSTRUE;
}

void FsAirTrafficInfo::CalculateStage(FsAirplane *air)
{
	if(FSGROUND==air->Prop().GetFlightState() || FSGROUNDSTATIC==air->Prop().GetFlightState())
	{
		stage=STAGE_ONGROUND;
	}
	else if(FsApproach::APP_NULL!=approach.GetApproachType())
	{
		const double distToInitialWayPoint=DistanceToNextFix(air);
		if(FsAirTrafficController::GetRadiusBeginDescendFromEnroute()<distToInitialWayPoint)
		{
			stage=STAGE_ENROUTE;
		}
		else
		{
			stage=STAGE_APPROACH;
		}
	}
	else
	{
		stage=STAGE_ENROUTE;
	}
}

void FsAirTrafficInfo::AssignHeading(const double headingIn)
{
	assignedHeading=headingIn;
}

void FsAirTrafficInfo::AssignAltitude(const double altitudeIn)
{
	assignedAltitude=altitudeIn;
}

void FsAirTrafficInfo::AssignAirSpeed(const double airSpeedIn)
{
	assignedAirSpeed=airSpeedIn;
}

YSBOOL FsAirTrafficInfo::AirplaneReachTurnPoint(FsAirplane *air) const
{
	if(currentPathIndex<path.GetN()-1)
	{
		const double Rturn=FsGetLevelTurnRadius(air->Prop().GetVelocity(),YsPi/9.0);

		YsVec3 nextLeg[2]={path[currentPathIndex].GetPosition(),path[currentPathIndex+1].GetPosition()};
		nextLeg[0].SetY(0.0);
		nextLeg[1].SetY(0.0);

		YsVec3 curPos=air->GetPosition();
		curPos.SetY(0.0);

		YsVec3 curVel;
		air->Prop().GetVelocity(curVel);
		curVel.SetY(0.0);

		YsVec3 nextVelNom=nextLeg[1]-nextLeg[0];
		YsVec3 curVelNom=curVel;
		if(YSOK==nextVelNom.Normalize() && YSOK==curVelNom.Normalize())
		{
			const double theata=acos(nextVelNom*curVelNom);
			const double leadDist=Rturn*sin(theata);

			YsVec3 intercept[2];
			if(YSOK==YsGetNearestPointOfTwoLine(intercept[0],intercept[1],nextLeg[0],nextLeg[1],curPos,curPos+curVel))
			{
				intercept[0].SetY(0.0);
				const double distBeforeIntercept=(curPos-intercept[0]).GetLength();
				if(distBeforeIntercept<=leadDist)
				{
					return YSTRUE;
				}
			}
		}
	}
	return YSFALSE;
}

YSBOOL FsAirTrafficInfo::HeadingDeviationExceed(class FsSimulation *sim,FsAirplane *air,const double angle) const
{
	if(YSTRUE==path.IsInRange(currentPathIndex))
	{
		YsVec3 newVec,curPos,curVel;
		double Rturn;
		curPos=air->GetPosition();
		air->Prop().GetVelocity(curVel);
		if(YSOK==FsAirTrafficController::GetNewVectorToWayPoint(newVec,Rturn,curPos,curVel,path[currentPathIndex].GetPosition()))
		{
			double assignedHdg=GetAssignedHeading();
			if(0>assignedHdg)
			{
				assignedHdg+=YsPi*2.0;
			}
			assignedHdg+=YsDegToRad(2.5);
			assignedHdg-=fmod(assignedHdg,YsDegToRad(5.0));

			double magHdg=FsAirTrafficController::VectorToMagneticHeading(sim,newVec);
			if(0>magHdg)
			{
				magHdg+=YsPi*2.0;
			}
			magHdg+=YsDegToRad(2.5);
			magHdg-=fmod(magHdg,YsDegToRad(5.0));

			YsVec2 v1(cos(assignedHdg),sin(assignedHdg)),v2(cos(magHdg),sin(magHdg));
			if(v1*v2<cos(angle))
			{
				printf("Assigned:%.0lf  New:%.0lf\n",YsRadToDeg(assignedHdg),YsRadToDeg(magHdg));
				printf("New Vector: %s\n",newVec.Txt());
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

YSBOOL FsAirTrafficInfo::AirplaneMessedUp(FsAirplane *air) const
{
	YsDisregardVariable(air);  // Will use.
	return YSFALSE;
}

void FsAirTrafficInfo::IncrementWayPoint(void)
{
	if(currentPathIndex<path.GetN()-1)
	{
		currentPathIndex++;
	}
}

const FsFlightPlanFix *FsAirTrafficInfo::GetNextWayPoint(void) const
{
	if(YSTRUE==path.IsInRange(currentPathIndex))
	{
		return &path[currentPathIndex];
	}
	return NULL;
}

const double FsAirTrafficInfo::DistanceToNextFix(FsAirplane *air) const
{
	if(YSTRUE==path.IsInRange(currentPathIndex))
	{
		YsVec3 nextFixPos=path[currentPathIndex].GetPosition();
		nextFixPos.SetY(0.0);

		YsVec3 airPos=air->GetPosition();
		airPos.SetY(0.0);

		return (airPos-nextFixPos).GetLength();
	}
	return 0.0;
}

void FsAirTrafficInfo::RewindFix(void)
{
	currentPathIndex=0;
}

void FsAirTrafficInfo::StopTalkingWithAirTrafficController(void)
{
	talkingWithAtc=YSFALSE;
}

FsAirTrafficController *FsAirTrafficInfo::GetAirTrafficController(class FsSimulation *sim)
{
	if(YSTRUE==talkingWithAtc)
	{
		return sim->FindAirTrafficController(atcKey);
	}
	return NULL;
}

const FsAirTrafficController *FsAirTrafficInfo::GetAirTrafficController(const class FsSimulation *sim) const
{
	if(YSTRUE==talkingWithAtc)
	{
		return sim->FindAirTrafficController(atcKey);
	}
	return NULL;
}

double FsAirTrafficInfo::GetLastAtcContactTime(void) const
{
	return clockLastInstructionFromAtc;
}

void FsAirTrafficInfo::SetLastAtcContactTime(double ctime)
{
	clockLastInstructionFromAtc=ctime;
}

double FsAirTrafficInfo::GetNextHeadingCheckTime(void) const
{
	return clockNextHeadingCheck;
}

void FsAirTrafficInfo::SetNextHeadingCheckTime(double nextCheckTimeIn)
{
	clockNextHeadingCheck=nextCheckTimeIn;
}

FsAirTrafficInfo::FLIGHTSTAGE FsAirTrafficInfo::GetStage(void) const
{
	return stage;
}

void FsAirTrafficInfo::SetStage(FsAirTrafficInfo::FLIGHTSTAGE stageIn)
{
	stage=stageIn;
}

const double FsAirTrafficInfo::GetAssignedHeading(void) const
{
	return assignedHeading;
}

const double FsAirTrafficInfo::GetAssignedAltitude(void) const
{
	return assignedAltitude;
}

const double FsAirTrafficInfo::GetAssignedAirSpeed(void) const
{
	return assignedAirSpeed;
}

YSRESULT FsAirTrafficInfo::SelectFirstFixForApproach(FsFlightPlanFix::FIXTYPE fixType)
{
	while(currentPathIndex<path.GetN())
	{
		if(fixType==path[currentPathIndex].GetFixType())
		{
			return YSOK;
		}
		currentPathIndex++;
	}
	return YSERR;
}

YSRESULT FsAirTrafficInfo::FindFixBefore(YsVec3 &pos,YsAtt3 &att,FsFlightPlanFix::FIXTYPE fixType) const
{
	for(int i=0; i<path.GetN()-1; i++)
	{
		if(fixType==path[i+1].GetFixType())
		{
			YsVec3 v;
			pos=path[i].GetPosition();
			v=path[i+1].GetPosition()-path[i].GetPosition();
			v.SetY(0.0);
			att.SetForwardVector(v);
			att.SetB(0.0);
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsAirTrafficInfo::ResetApproach(class FsSimulation *sim,const class FsAirplane &air)
{
	switch(approach.GetApproachType())
	{
	case FsApproach::APP_ILS:
		{
			auto dstName=approach.GetDestinationName();
			auto ils=approach.GetFacility(sim);
			if(nullptr!=ils)
			{
				return SelectApproachByILS(sim,air,ils,dstName);
			}
		}
		break;
	default:
		return YSERR;
	}
	return YSOK;
}

YSRESULT FsAirTrafficInfo::SelectApproachByAirportName(class FsSimulation *sim,const class FsAirplane &air,const char dstAirportName[])
{
	const FsField *field=sim->GetField();

	YsArray <const class YsSceneryRectRegion *,16> rgnArray;
	if(YSOK!=field->SearchFieldRegionByTag(rgnArray,dstAirportName) && 0>=rgnArray.GetN())
	{
		return YSERR;
	}


	const YsSceneryRectRegion *rgn=NULL;
	for(int i=0; i<rgnArray.GetN(); i++)
	{
		if(rgnArray[i]->GetId()==FS_RGNID_AIRPORT_AREA)
		{
			rgn=rgnArray[i];
			break;
		}
	}
	if(NULL==rgn)
	{
		return YSERR;
	}


	YsVec3 rect[4];
	field->GetFieldRegionRect(rect,rgn);


	YsArray <const FsGround *,16> ilsArray;
	for(int i=0; i<sim->GetNumAircraftCarrier(); i++)
	{
		const FsGround *ils=sim->GetAircraftCarrier(i);
		if(NULL!=ils && YSINSIDE==YsCheckInsidePolygon3(ils->GetPosition(),4,rect))
		{
			ilsArray.Append(ils);
		}
	}


	if(0<ilsArray.GetN())
	{
		const FsWeather &weather=sim->GetWeather();

		const FsGround *ilsPick=NULL;
		double ilsPickDotProd=1.0;

		for(int i=0; i<ilsArray.GetN(); i++)
		{
			const FsAircraftCarrierProperty *acProp=ilsArray[i]->Prop().GetAircraftCarrierProperty();
			if(NULL!=acProp)
			{
				const FsILS &ils=acProp->GetILS();
				if(YsTolerance<ils.GetRange())
				{
					YsVec3 pos;
					YsAtt3 att;
					ils.GetLandingPositionAndAttitude(pos,att);

					YsVec3 ev=att.GetForwardVector();
					ev.SetY(0.0);
					if(NULL==ilsPick || ev*weather.GetWind()<ilsPickDotProd)
					{
						ilsPick=ilsArray[i];
						ilsPickDotProd=ev*weather.GetWind();
					}
				}
			}
		}

		if(NULL!=ilsPick)
		{
			SelectApproachByILS(sim,air,ilsPick,dstAirportName);
			return YSOK;
		}
	}

	return YSERR;
}

YSRESULT FsAirTrafficInfo::SelectApproachByILS(class FsSimulation *sim,const class FsAirplane &air,const class FsGround *ils,const char dstAirportName[])
{
	approach.CleanUp();
	if(YSOK==CalculateFlightPathForILS(sim,air,ils))
	{
		currentPathIndex=0;
		clockLastInstructionFromAtc=-1.0;
		clockNextHeadingCheck=-1.0;
		stage=STAGE_NEEDCALC;

		approach.SetApproachType(FsApproach::APP_ILS);
		approach.SetDestinationName(dstAirportName);
		approach.SetFacility(sim,ils);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsAirTrafficInfo::CalculateFlightPathForILS(class FsSimulation *sim,const class FsAirplane &air,const class FsGround *ils)
{
	const FsAircraftCarrierProperty *ilsProp=ils->Prop().GetAircraftCarrierProperty();
	if(NULL!=ilsProp)
	{
		YsVec3 tdPos;
		YsAtt3 rwAtt;
		ilsProp->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);
		return CalculateFlightPathFromTdPosHeadingAndGlideSlope(sim,air,tdPos,rwAtt,1);
	}
	return YSERR;
}

YSRESULT FsAirTrafficInfo::CalculateFlightPath(const class YsScenery &,const class FsAirplane &,FLIGHTSTAGE )
{
	return YSERR;
}

YSRESULT FsAirTrafficInfo::CalculateFlightPathFromTdPosHeadingAndGlideSlope(class FsSimulation *sim,const class FsAirplane &air,const YsVec3 &tdPos,const YsAtt3 &tdAtt,int sequence)
{
	// See AirTrafficController.docx

	// tdAtt.GetForwardVector() is a vector FROM touch down point toward glide slope upward.

	const double glideSlope=fabs(tdAtt.p());
	const YsAtt3 finalHdg(tdAtt.h()+YsPi,0.0,0.0);
	const YsVec3 fromPos=air.GetPosition();
	const double baseElv=sim->GetBaseElevation();
	const double Vfinal=air.Prop().GetEstimatedLandingSpeed();
	const double Vbase=Vfinal*1.5;

	YsPrintf("Glide Slope Angle %lf\n",YsRadToDeg(tdAtt.p()));

	YsVec3 vecFinal(0.0,0.0,1.0);
	finalHdg.Mul(vecFinal,vecFinal);
	YsPrintf("Vector Final %s\n",vecFinal.Txt());

	const int idLeftTraffic=0,idRightTraffic=1;
	YSBOOL possibleTraffic[2]={YSFALSE,YSFALSE};

	YsPrintf("Final Approach Speed %lfkt\n",YsUnitConv::MPStoKT(Vfinal));
	YsPrintf("Speed before Final %lfkt\n",YsUnitConv::MPStoKT(Vbase));



	const YsAtt3 dogLegHdg[2]={YsAtt3(finalHdg.h()+YsPi/6.0,0.0,0.0),YsAtt3(finalHdg.h()-YsPi/6.0,0.0,0.0)};

	double Ygsi=tdPos.y()+FsAirTrafficController::TerrainClearanceApproach()+YsUnitConv::FTtoM(99.0);
	Ygsi+=baseElv;
	Ygsi-=fmod(Ygsi,YsUnitConv::FTtoM(100.0));
	Ygsi-=baseElv;

	double Lfinal=YsUnitConv::FTtoM(Ygsi)/tan(glideSlope);
	double Lgsi=Vbase*(30.0+40.0*(double)(sequence-1));
	double Ldog=Vbase*45.0;

	YsVec3 Pgsi=tdPos-vecFinal*Lfinal;  // Glide-slope intercept
	YsVec3 Ploc=Pgsi-vecFinal*Lgsi;     // Localizer intercept
	YsVec3 Pdog[2];                     // Dog-leg intercept

	YsPrintf("Lgsi=%.1lfkm\n",Lgsi/1000.0);
	YsPrintf("TdPos=%s\n",tdPos.Txt());

	for(;;) // Until at least 2 minutes of Lgsi is secured.
	{
		Lfinal=Ygsi/tan(glideSlope);

		YsPrintf("Trying: Ygsi=%.1lfm, Lfinal=%.1lfkm\n",Ygsi,Lfinal/1000.0);

		Pgsi=tdPos-vecFinal*Lfinal;
		Pgsi.SetY(Ygsi);

		Ploc=Pgsi-vecFinal*Lgsi;

		const double clearance=GetMinClearanceOfLeg(Ygsi,sim,Pgsi,Ploc,500.0);
		if(YsUnitConv::FTtoM(1000.0)<=clearance)
		{
			YsVec3 vecDog[2];
			dogLegHdg[0].Mul(vecDog[0],YsVec3(0.0,0.0,1.0));
			dogLegHdg[1].Mul(vecDog[1],YsVec3(0.0,0.0,1.0));

			Pdog[0]=Ploc-vecDog[0]*Ldog;
			Pdog[1]=Ploc-vecDog[1]*Ldog;

			const double clearanceDog[2]={
				GetMinClearanceOfLeg(Ygsi,sim,Ploc,Pdog[0],500.0),
				GetMinClearanceOfLeg(Ygsi,sim,Ploc,Pdog[1],500.0)};

			for(int i=0; i<2; i++)
			{
				if(YsUnitConv::FTtoM(1000.0)<=clearanceDog[i])
				{
					possibleTraffic[i]=YSTRUE;
				}
			}

			if(YSTRUE==possibleTraffic[0] || YSTRUE==possibleTraffic[1])
			{
				break;
			}
		}

		Ygsi+=YsUnitConv::FTtoM(500.0);
		if(tdPos.y()+YsUnitConv::FTtoM(5000.0)<=Ygsi)
		{
			YsPrintf("Cannot calculate Lfinal.\n");
			return YSERR;
		}
	}

	YsPrintf("Ygsi=%.1lfm Lfinal=%.1lfkm\n",Ygsi,Lfinal/1000.0);
	YsPrintf("Left Dogleg=%d  Right dogleg=%d\n",(int)possibleTraffic[idLeftTraffic],(int)possibleTraffic[idRightTraffic]);
	YsPrintf("Pgsi=%s Ploc=%s\n",Pgsi.Txt(),Ploc.Txt());


	YsVec3 Pbase[2]; // Base-leg intercept
	const double Lbase=Vbase*90.0;
	const YsAtt3 baseLegHdg[2]={YsAtt3(finalHdg.h()+YsPi/2.0,0.0,0.0),YsAtt3(finalHdg.h()-YsPi/2.0,0.0,0.0)};

	for(int i=0; i<2; i++)
	{
		if(YSTRUE==possibleTraffic[i])
		{
			YsVec3 vecBase;
			baseLegHdg[i].Mul(vecBase,YsZVec());

			Pbase[i]=Pdog[i]-vecBase*Lbase;
			const double maxElv=GetMaxElevationOfLeg(sim,Pdog[i],Pbase[i],500.0);

			double Ybase=tdPos.y()+maxElv;
			Ybase+=baseElv;
			Ybase-=fmod(Ybase,YsUnitConv::FTtoM(100.0));
			Ybase-=baseElv;

			Pdog[i].SetY(YsGreater(Ybase,Ygsi));
			Pbase[i].SetY(YsGreater(Ybase,Ygsi));

			if(Ybase<Ygsi)
			{
				continue;
			}

			const double slope=(Ybase-Ygsi)/Ldog;
			if(slope>=YsTan5deg)
			{
				possibleTraffic[i]=YSFALSE;
			}
		}
	}

	YsPrintf("Left Base=%d  Right Base=%d\n",(int)possibleTraffic[idLeftTraffic],(int)possibleTraffic[idRightTraffic]);



	YsVec3 Pdownwind[2];  // Down-wind intercept
	const double Ldownwind=Vbase*120.0;
	const YsVec3 vecDownwind=-vecFinal;
	for(int i=0; i<2; i++)
	{
		if(YSTRUE==possibleTraffic[i])
		{
			Pdownwind[i]=Pbase[i]-vecDownwind*Ldownwind;
			const double maxElv=GetMaxElevationOfLeg(sim,Pbase[i],Pdownwind[i],500.0);

			double Ydownwind=tdPos.y()+maxElv;
			Ydownwind+=baseElv;
			Ydownwind-=fmod(Ydownwind,YsUnitConv::FTtoM(100.0));
			Ydownwind-=baseElv;

			const double Ybase=Pbase[i].y();

			Pbase[i].SetY(YsGreater(Ydownwind,Ybase));
			Pdownwind[i].SetY(YsGreater(Ydownwind,Ybase));

			if(Ydownwind<Ybase)
			{
				continue;
			}

			const double slope=(Ydownwind-Ybase)/Lbase;
			if(slope>=YsTan10deg)
			{
				possibleTraffic[i]=YSFALSE;
			}
		}
	}

	YsPrintf("Left Downwind=%d  Right downwind=%d\n",(int)possibleTraffic[idLeftTraffic],(int)possibleTraffic[idRightTraffic]);

	int choiceLeftRight=-1;
	if(YSTRUE==possibleTraffic[0] && YSTRUE==possibleTraffic[1])
	{
		const double d[2]={
			(air.GetPosition()-Pdownwind[0]).GetSquareLength(),
			(air.GetPosition()-Pdownwind[1]).GetSquareLength()};
		if(d[0]<d[1])
		{
			choiceLeftRight=0;
		}
		else
		{
			choiceLeftRight=1;
		}
	}
	else if(YSTRUE==possibleTraffic[0])
	{
		choiceLeftRight=0;
	}
	else if(YSTRUE==possibleTraffic[1])
	{
		choiceLeftRight=1;
	}



	if(0<=choiceLeftRight)
	{
		YsPrintf("CurrentPos to DownwindEntry %lf\n",(fromPos-Pdownwind[choiceLeftRight]).GetLength());

		for(YSSIZE_T i=0; i<path.GetN(); ++i)
		{
			if(path[i].GetFixType()==FsFlightPlanFix::FIX_DONWINDENTRY ||
			   path[i].GetFixType()==FsFlightPlanFix::FIX_BASEENTRY ||
			   path[i].GetFixType()==FsFlightPlanFix::FIX_DOGLEGENTRY ||
			   path[i].GetFixType()==FsFlightPlanFix::FIX_LOCALIZERINTERCEPT ||
			   path[i].GetFixType()==FsFlightPlanFix::FIX_GLIDESLOPEINTERCEPT ||
			   path[i].GetFixType()==FsFlightPlanFix::FIX_CUSTOM_APPROACH_FIX)
			{
				path.Resize(i);
				break;
			}
		}

		path.Increment();
		path.GetEnd().SetPosition(Pdownwind[choiceLeftRight],FsFlightPlanFix::FIX_DONWINDENTRY);
		path.Increment();
		path.GetEnd().SetPosition(Pbase[choiceLeftRight],FsFlightPlanFix::FIX_BASEENTRY);
		path.Increment();
		path.GetEnd().SetPosition(Pdog[choiceLeftRight],FsFlightPlanFix::FIX_DOGLEGENTRY);
		path.Increment();
		path.GetEnd().SetPosition(Ploc,FsFlightPlanFix::FIX_LOCALIZERINTERCEPT);
		path.Increment();
		path.GetEnd().SetPosition(Pgsi,FsFlightPlanFix::FIX_GLIDESLOPEINTERCEPT);
		return YSOK;
	}

	return YSERR;
}

const double FsAirTrafficInfo::GetMaxElevationOfLeg(const class FsSimulation *sim,const YsVec3 &p1,const YsVec3 &p2,const double interval) const
{
	const double Lleg=(p2-p1).GetLength();
	const int nDiv=YsGreater <int> ((int)(Lleg/interval),1);
	const YsVec3 v=p2-p1;

	double maxElv=0.0;
	for(int i=0; i<=nDiv; i++)
	{
		const double t=(double)i/(double)nDiv;
		const YsVec3 samplePos=p1+v*t;

		const double elv=sim->GetFieldElevation(samplePos.x(),samplePos.z());
		maxElv=YsGreater(elv,maxElv);
	}

	return maxElv;
}

const double FsAirTrafficInfo::GetMinClearanceOfLeg(const double alt,const class FsSimulation *sim,const YsVec3 &p1,const YsVec3 &p2,const double interval) const
{
	const double maxElv=GetMaxElevationOfLeg(sim,p1,p2,interval);
	return alt-maxElv;
}

const FsApproach &FsAirTrafficInfo::GetApproach(void) const
{
	return approach;
}

YSBOOL FsAirTrafficInfo::IsHeadingForFinalFix(void) const
{
	if(path.GetN()-2<=currentPathIndex)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsAirTrafficInfo::IsGuidedByATC(void) const
{
	if(0<path.GetN())
	{
		return YSTRUE;
	}
	return YSFALSE;
}



////////////////////////////////////////////////////////////

void FsAirTrafficSequence::Slot::Initialize(void)
{
	segType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL;
	label.Set("");
	objSearchKey=-1;
	accommodateMultiple=YSFALSE;
	ClearObject();
}

void FsAirTrafficSequence::Slot::ClearObject(void)
{
	occupyingObjKey.Clear();
	incomingObjKey.Clear();
}

void FsAirTrafficSequence::Slot::RemoveIncomingObject(unsigned int objKey)
{
	for(int idx=(int)incomingObjKey.GetN()-1; 0<=idx; --idx)
	{
		if(incomingObjKey[idx]==objKey)
		{
			incomingObjKey.DeleteBySwapping(idx);
		}
	}
}

void FsAirTrafficSequence::Slot::AddIncomingObject(unsigned int objKey)
{
	incomingObjKey.Append(objKey);
}

void FsAirTrafficSequence::Slot::RemoveOccupyingObject(unsigned int objKey)
{
	for(int idx=(int)occupyingObjKey.GetN()-1; 0<=idx; --idx)
	{
		if(occupyingObjKey[idx]==objKey)
		{
			occupyingObjKey.DeleteBySwapping(idx);
		}
	}
}

void FsAirTrafficSequence::Slot::AddOccupyingObject(unsigned int objKey)
{
	occupyingObjKey.Append(objKey);
}

YSSIZE_T FsAirTrafficSequence::Slot::GetNumObject(void) const
{
	return incomingObjKey.GetN()+occupyingObjKey.GetN();
}

////////////////////////////////////////////////////////////


// See memo/designmen/flightschedule.txt
FsAirTrafficSequence::FsAirTrafficSequence()
{
	Initialize();
}

FsAirTrafficSequence::~FsAirTrafficSequence()
{
}


FsAirTrafficSequence *FsAirTrafficSequence::Create(void)
{
	FsAirTrafficSequence *ats=new FsAirTrafficSequence;
	return ats;
}

void FsAirTrafficSequence::Delete(FsAirTrafficSequence *ats)
{
	delete ats;
}


void FsAirTrafficSequence::Initialize(void)
{
	labelToFixSlotIdx.Initialize();
	labelToVorSlotIdx.Initialize();
	labelToNdbSlotIdx.Initialize();
	labelToAirportSlotIdx.Initialize();
	labelToCarrierSlotIdx.Initialize();
	slotArray.Clear();
	ClearSequence();
	ClearRunwayUsage();
}


void FsAirTrafficSequence::ClearSequence(void)
{
	objKeyToIncomingSlotIdx.PrepareTable();
	objKeyToOccupyingSlotIdx.PrepareTable();
	for(int i=0; i<slotArray.GetN(); ++i)
	{
		slotArray[i].ClearObject();
	}
	lastUpdateTime=0.0;
	updateInterval=3.0;
}

YSRESULT FsAirTrafficSequence::MakeSlotArray(const class FsSimulation *sim)
{
	const FsField *fld=sim->GetField();
	if(NULL==fld)
	{
		return YSERR;
	}
	const YsScenery *scn=fld->GetFieldPtr();
	if(NULL==fld)
	{
		return YSERR;
	}

	Initialize();

	YsArray <const char *> labelArray;
	YsArray <YSSIZE_T> indexArray;
	YsArray <const YsSceneryRectRegion *> rgnArray;

	if(YSOK==scn->SearchRegionById(rgnArray,FS_RGNID_AIRPORT_AREA))
	{
		for(int rgnIdx=0; rgnIdx<rgnArray.GetN(); ++rgnIdx)
		{
			YSSIZE_T nextIndex=slotArray.GetN();
			slotArray.Increment();
			slotArray[nextIndex].Initialize();
			slotArray[nextIndex].label.Set(rgnArray[rgnIdx]->GetTag());
			slotArray[nextIndex].segType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT;
			slotArray[nextIndex].accommodateMultiple=(rgnArray[rgnIdx]->GetSubClassType()==YsSceneryRectRegion::SUBCLASS_DEADLOCK_FREE_AIRPORT ? YSTRUE : YSFALSE);
			labelArray.Append(rgnArray[rgnIdx]->GetTag());
			indexArray.Append(nextIndex);
		}
	}
	labelArray.Append(NULL);
	indexArray.Append(-1);
	labelToAirportSlotIdx.MakeList(labelArray,indexArray);

	labelArray.Clear();
	indexArray.Clear();

	if(YSOK==scn->SearchRegionById(rgnArray,FS_RGNID_FIX))
	{
		for(int rgnIdx=0; rgnIdx<rgnArray.GetN(); ++rgnIdx)
		{
			YSSIZE_T nextIndex=slotArray.GetN();
			slotArray.Increment();
			slotArray[nextIndex].Initialize();
			slotArray[nextIndex].label.Set(rgnArray[rgnIdx]->GetTag());
			slotArray[nextIndex].segType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX;
			labelArray.Append(rgnArray[rgnIdx]->GetTag());
			indexArray.Append(nextIndex);
		}
	}
	labelArray.Append(NULL);
	indexArray.Append(-1);
	labelToFixSlotIdx.MakeList(labelArray,indexArray);


	labelArray.Clear();
	indexArray.Clear();
	for(int idx=0; idx<sim->GetNumVOR(); ++idx)
	{
		const FsGround *vor=sim->GetVOR(idx);

		YSSIZE_T nextIndex=slotArray.GetN();
		slotArray.Increment();
		slotArray[nextIndex].label.Set(vor->GetName());
		slotArray[nextIndex].segType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR;
		slotArray[nextIndex].objSearchKey=vor->SearchKey();

		labelArray.Append(vor->GetName());
		indexArray.Append(nextIndex);
	}
	labelArray.Append(NULL);
	indexArray.Append(-1);
	labelToVorSlotIdx.MakeList(labelArray,indexArray);

	labelArray.Clear();
	indexArray.Clear();
	for(int idx=0; idx<sim->GetNumNDB(); ++idx)
	{
		const FsGround *ndb=sim->GetNDB(idx);

		YSSIZE_T nextIndex=slotArray.GetN();
		slotArray.Increment();
		slotArray[nextIndex].label.Set(ndb->GetName());
		slotArray[nextIndex].segType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB;
		slotArray[nextIndex].objSearchKey=ndb->SearchKey();

		labelArray.Append(ndb->GetName());
		indexArray.Append(nextIndex);
	}
	labelArray.Append(NULL);
	indexArray.Append(-1);
	labelToNdbSlotIdx.MakeList(labelArray,indexArray);
	
	labelArray.Clear();
	indexArray.Clear();
	for(int idx=0; idx<sim->GetNumILSFacility(); ++idx)
	{
		const FsGround *ils=sim->GetILS(idx);

		YSSIZE_T nextIndex=slotArray.GetN();
		slotArray.Increment();
		slotArray[nextIndex].label.Set(ils->GetName());
		slotArray[nextIndex].segType=YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER;
		slotArray[nextIndex].objSearchKey=ils->SearchKey();

		labelArray.Append(ils->GetName());
		indexArray.Append(nextIndex);
	}
	labelArray.Append(NULL);
	indexArray.Append(-1);
	labelToCarrierSlotIdx.MakeList(labelArray,indexArray);


	runwayArray.Clear();
	runwayKeyToRunwayIdx.CleanUp();
	if(YSOK==scn->SearchRegionById(rgnArray,FS_RGNID_RUNWAY))
	{
		for(int rgnIdx=0; rgnIdx<rgnArray.GetN(); ++rgnIdx)
		{
			YSSIZE_T idx=runwayArray.GetN();
			runwayArray.Increment();
			runwayArray[idx].rectRgn=rgnArray[rgnIdx];
			runwayArray[idx].traffic.Clear();
			runwayKeyToRunwayIdx.AddElement(rgnArray[rgnIdx]->GetSearchKey(),idx);
		}
	}

	return YSOK;
}

void FsAirTrafficSequence::RefreshAirTrafficSlot(const class FsSimulation *sim)
{
	ClearSequence();

	for(const FsAirplane *air=NULL; NULL!=(air=sim->FindNextAirplane(air)); )
	{
		if(YSTRUE==air->IsAlive())
		{
			YsString label;
			YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType=air->GetCurrentDestination(label);
			if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL!=segType)
			{
				AddIncomingObject(air->SearchKey(),segType,label);
			}

			segType=air->GetOccupyingAirportOrFix(label);
			if(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NULL!=segType)
			{
				AddOccupyingObject(air->SearchKey(),segType,label);
			}
		}
	}

	lastUpdateTime=sim->currentTime;
}

void FsAirTrafficSequence::ClearRunwayUsage(void)
{
	for(int idx=0; idx<runwayArray.GetN(); ++idx)
	{
		runwayArray[idx].traffic.Clear();
	}
}

void FsAirTrafficSequence::RefreshRunwayUsage(const class FsSimulation *sim)
{
	ClearRunwayUsage();

	for(const FsAirplane *air=NULL; NULL!=(air=sim->FindNextAirplane(air)); )
	{
		if(YSTRUE==air->IsAlive())
		{
			if(YSTRUE==air->Prop().IsOnGround())
			{
				if(YSTRUE==air->rectRgnCached)
				{
					AddLandingOrTakingOffTraffic(FSLEG_LANDED,air,air->GetPosition(),air->rectRgnCache);
				}
			}
			else
			{
				FSTRAFFICPATTERNLEG leg;
				YsArray <const YsSceneryRectRegion *,4> rectRgn;
				YsVec3 tdPos;
				if(YSTRUE==air->IsApproachingRunway(leg,rectRgn,tdPos))
				{
					AddLandingOrTakingOffTraffic(leg,air,tdPos,rectRgn);
				}
			}
		}
	}

	SortRunwayTrafficByDistanceToTouchDown();
}

void FsAirTrafficSequence::SortRunwayTrafficByDistanceToTouchDown(void)
{
	YsArray <double,8> distArray;
	for(YSSIZE_T runwayIdx=0; runwayIdx<runwayArray.GetN(); ++runwayIdx)
	{
		if(1<runwayArray[runwayIdx].traffic.GetN())
		{
			distArray.Set(runwayArray[runwayIdx].traffic.GetN(),NULL);
			for(YSSIZE_T traIdx=0; traIdx<runwayArray[runwayIdx].traffic.GetN(); ++traIdx)
			{
				distArray[traIdx]=runwayArray[runwayIdx].traffic[traIdx].distToTouchDownSq;
			}
			YsQuickSort <double,LandingTraffic> (runwayArray[runwayIdx].traffic.GetN(),distArray,runwayArray[runwayIdx].traffic);
		}
	}
}

YSRESULT FsAirTrafficSequence::RequestProceed(unsigned int objKey,YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[])
{
	const YSSIZE_T slotIndex=FindSlotIndex(segType,label);
	if(0<=slotIndex)
	{
		if(0==slotArray[slotIndex].GetNumObject() || YSTRUE==slotArray[slotIndex].accommodateMultiple)
		{
			AddIncomingObject(objKey,slotIndex);
			return YSOK;
		}
		else
		{
			return YSERR;
		}
	}
	return YSOK; // Not round.  Not this guy's jurisdiction.
}

YSRESULT FsAirTrafficSequence::ClearOccupyingAirportOrFix(unsigned int objKey)
{
	YSSIZE_T currentIndex;
	if(YSOK==objKeyToOccupyingSlotIdx.FindElement(currentIndex,objKey) &&
	   YSTRUE==slotArray.IsInRange(currentIndex))
	{
		slotArray[currentIndex].RemoveOccupyingObject(objKey);
		return YSOK;
	}
	return YSERR;
}

const double FsAirTrafficSequence::GetLastUpdatedTime(void) const
{
	return lastUpdateTime;
}

const double FsAirTrafficSequence::GetNextUpdateTime(void) const
{
	return lastUpdateTime+updateInterval;
}

YSSIZE_T FsAirTrafficSequence::FindSlotIndex(YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[]) const
{
	const YsKeyWordList *dict=NULL;
	switch(segType)
	{
	case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_FIX:
		dict=&labelToFixSlotIdx;
		break;
	case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_VOR:
		dict=&labelToVorSlotIdx;
		break;
	case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_NDB:
		dict=&labelToNdbSlotIdx;
		break;
	case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_AIRPORT:
		dict=&labelToAirportSlotIdx;
		break;
	case YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_CARRIER:
		dict=&labelToCarrierSlotIdx;
		break;
	}

	if(NULL!=dict)
	{
		const int index=dict->FindIndex(label);
		if(0<=index)
		{
			return dict->GetAttribute(index);
		}
	}

	return -1;
}

void FsAirTrafficSequence::AddIncomingObject(unsigned int objKey,YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[])
{
	const YSSIZE_T slotIndex=FindSlotIndex(segType,label);
	if(0<=slotIndex)
	{
		AddIncomingObject(objKey,slotIndex);
	}
}

void FsAirTrafficSequence::AddIncomingObject(unsigned int objKey,YSSIZE_T slotIndex)
{
	YSSIZE_T currentIndex;
	if(YSOK==objKeyToIncomingSlotIdx.FindElement(currentIndex,objKey) &&
	   YSTRUE==slotArray.IsInRange(currentIndex))
	{
		slotArray[currentIndex].RemoveIncomingObject(objKey);
	}
	objKeyToIncomingSlotIdx.AddElement(objKey,slotIndex);
	slotArray[slotIndex].AddIncomingObject(objKey);
}

void FsAirTrafficSequence::AddOccupyingObject(unsigned int objKey,YsSceneryAirRoute::RouteSegment::ROUTE_SEGMENT_TYPE segType,const char label[])
{
	const YSSIZE_T slotIndex=FindSlotIndex(segType,label);
	if(0<=slotIndex)
	{
		AddOccupyingObject(objKey,slotIndex);
	}
}

void FsAirTrafficSequence::AddOccupyingObject(unsigned int objKey,YSSIZE_T slotIndex)
{
	YSSIZE_T currentIndex;
	if(YSOK==objKeyToOccupyingSlotIdx.FindElement(currentIndex,objKey) &&
	   YSTRUE==slotArray.IsInRange(currentIndex))
	{
		slotArray[currentIndex].RemoveOccupyingObject(objKey);
	}
	objKeyToOccupyingSlotIdx.AddElement(objKey,slotIndex);
	slotArray[slotIndex].AddOccupyingObject(objKey);
}

void FsAirTrafficSequence::AddLandingOrTakingOffTraffic(FSTRAFFICPATTERNLEG leg,const FsExistence *obj,const YsVec3 &tdPos,YSSIZE_T nRgn,const YsSceneryRectRegion * const rgnArray[])
{
	for(YSSIZE_T rgnIdx=0; rgnIdx<nRgn; ++rgnIdx)
	{
		YSSIZE_T runwayIdx;
		if(FS_RGNID_RUNWAY==rgnArray[rgnIdx]->GetId() && 
		   YSOK==runwayKeyToRunwayIdx.FindElement(runwayIdx,rgnArray[rgnIdx]->GetSearchKey()))
		{
			runwayArray[runwayIdx].traffic.Increment();
			runwayArray[runwayIdx].traffic.GetEnd().objKey=obj->SearchKey();
			runwayArray[runwayIdx].traffic.GetEnd().tdPos=tdPos;
			runwayArray[runwayIdx].traffic.GetEnd().objPos=obj->GetPosition();
			if(FSLEG_NOT_IN_PATTERN==leg)
			{
				runwayArray[runwayIdx].traffic.GetEnd().distToTouchDownSq=0.0; // Means on the ground.
			}
			else
			{
				runwayArray[runwayIdx].traffic.GetEnd().distToTouchDownSq=(tdPos-obj->GetPosition()).GetSquareLength();
			}
			runwayArray[runwayIdx].traffic.GetEnd().leg=leg;
		}
	}
}

YSRESULT FsAirTrafficSequence::FindLandingAndTakingOffTraffic(YSSIZE_T &nTra,const LandingTraffic *&traArray,const class YsSceneryRectRegion *runwayRect) const
{
	YSSIZE_T runwayIdx;
	if(YSOK==runwayKeyToRunwayIdx.FindElement(runwayIdx,runwayRect->GetSearchKey()))
	{
		nTra=runwayArray[runwayIdx].traffic.GetN();
		traArray=runwayArray[runwayIdx].traffic;
		return YSOK;
	}
	else
	{
		nTra=0;
		traArray=NULL;
		return YSERR;
	}
}

YSBOOL FsAirTrafficSequence::WillCauseRunwayIncursionFromTakeOffTraffic(const class FsAirplane &airTakingOff,const FsSimulation *sim,const class YsSceneryRectRegion *runwayRect) const
{
	YSSIZE_T nTra;
	const LandingTraffic *traArray;
	if(YSOK==FindLandingAndTakingOffTraffic(nTra,traArray,runwayRect) && 1<=nTra)  // 2013/04/08  1<=nTra is a wrong condition.  Two airplanes may be taxiing to the same runway, one is on the runway and the other is just checking from the taxiway.
	{
		for(YSSIZE_T traIdx=0; traIdx<nTra; ++traIdx)
		{
			if(traArray[traIdx].objKey!=airTakingOff.SearchKey())
			{
				if(traArray[traIdx].leg==FSLEG_FINAL &&
				   traArray[traIdx].distToTouchDownSq<FsShortFinalDistance*FsShortFinalDistance)
				{
					return YSTRUE;
				}
				else if(traArray[traIdx].leg==FSLEG_LANDED)
				{
					// 2014/10/19 This traffic may be taking off, and moving away from the airplane.
					//            Then, it doesn't cause runway incursion.  It is possible to line up and wait.
					YSBOOL takingOffAndMovingAway=YSFALSE;
					FsAirplane *airOnRunway=sim->FindAirplane(traArray[traIdx].objKey);
					if(NULL!=airOnRunway)
					{
						auto ap=airOnRunway->GetAutopilot();
						if(NULL!=ap && YSTRUE==ap->IsTakingOff())
						{
							const YsVec3 relPos=airOnRunway->GetPosition()-airTakingOff.GetPosition();
							YsVec3 takeOffDir;
							airOnRunway->Prop().GetVelocity(takeOffDir);
							if(0.0<relPos*takeOffDir)
							{
								takingOffAndMovingAway=YSTRUE;
							}
						}
					}
					if(YSTRUE!=takingOffAndMovingAway)
					{
						return YSTRUE;
					}
				}
			}
		}
	}
	return YSFALSE;
}

YSBOOL FsAirTrafficSequence::WillCauseRunwayIncursionFromLandingTraffic(const class FsAirplane &airLanding,const class YsSceneryRectRegion *runwayRect) const
{
	YSSIZE_T nTra;
	const LandingTraffic *traArray;

	if(YSOK==FindLandingAndTakingOffTraffic(nTra,traArray,runwayRect) && 1<=nTra)
	{
		YSBOOL thisTrafficIsOnShortFinal=YSFALSE;

		const double distToDecisionHeightSq=1147.22*1147.22; // Straight-line distance from tdPos to airPos with 3 degree glide slope.

		YSSIZE_T thisTrafficIdx=-1;
		for(YSSIZE_T traIdx=0; traIdx<nTra; ++traIdx)
		{
			if(airLanding.SearchKey()==traArray[traIdx].objKey)
			{
				thisTrafficIdx=traIdx;
				if(traArray[traIdx].leg==FSLEG_FINAL &&
				   traArray[traIdx].distToTouchDownSq<distToDecisionHeightSq)
				{
					thisTrafficIsOnShortFinal=YSTRUE;
				}
				if(0<traIdx && traArray[traIdx].leg==FSLEG_FINAL && traArray[traIdx-1].leg==FSLEG_FINAL)
				{
					const double distPrevTraffic=sqrt(traArray[traIdx-1].distToTouchDownSq);
					const double distThisTraffic=sqrt(traArray[traIdx].distToTouchDownSq);
					if(distThisTraffic-distPrevTraffic<FsShortFinalDistance)
					{
						return YSTRUE;
					}
				}
				break;
			}
		}

		if(0>thisTrafficIdx)
		{
			return YSFALSE;
		}

		for(YSSIZE_T traIdx=0; traIdx<nTra; ++traIdx)
		{
			if(traIdx!=thisTrafficIdx)
			{
				if(airLanding.GetApproximatedCollideRadius()*5.0>(traArray[traIdx].objPos-airLanding.GetPosition()).GetLength() &&
				   traArray[thisTrafficIdx].distToTouchDownSq>traArray[traIdx].distToTouchDownSq)
				{
					return YSTRUE;
				}
				if(traArray[traIdx].leg==FSLEG_LANDED && YSTRUE==thisTrafficIsOnShortFinal)
				{
					return YSTRUE;
				}
			}
		}
	}
	return YSFALSE;
}
