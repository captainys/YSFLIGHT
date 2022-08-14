
BEGINAIRROUTE
LABEL AOMORI_LOOP
CYCLE LOOP or REVERSE
AIRPORT LABEL:MISAWA ILS:28-MISAWA ILS:10-MISAWA APPROACH:15nm
DIRECT ALT:5000ft 
AIRPORT LABEL:HACHINOHE ILS:25-HACHINOHE ILS:07-HACHINOHE
DIRECT ALT:8000ft
AIRPORT LABEL:AOMORI ILS:24-AOMORI ILS:06-AOMORI
DIRECT ALT:9000ft APPROACH:15nm
ENDAIRROUTE


class YsSceneryAirRoute
{
public:
	class RouteSegment
	{
	public:
		enum ROUTE_SEGMENT_TYPE
		{
			ROUTE_SEGMENT_DIRECT,
			ROUTE_SEGMENT_FIX,
			ROUTE_SEGMENT_AIRPORT
		};

		ROUTE_SEGMENT_TYPE segType;
		YsString label;
		double altitude;
		YsArray <YsString> ilsArray;

		// Cache
		mutable YsVec3 pos;
		mutable unsigned int ilsKeyArray;
	};

	YsString label;
	YsSegmentedArray <RouteSegment> airportStore;
};




For each airport should be able to make a list of
  Arriving airplane - FsLandingAutopilot coming in to one of the ILSs.  An airplane talking with FsATC and guided to one of the ILSs.
  Airplane standing by - Airplane stationary at the ramp
  Departing airplane - Airplane with FsTaxiingAutopilot in MODE_TAKEOFF to the runway.

  Question: How to avoid dead lock?
  Goal: Preventing an arriving or stationary airplane from blocking a departing airplane will cause a dead lock.
      Solution A, only allow a single airplane per airport.  Not fun.
      Solution B?




é¿ÇÕç°ÇÃAutopilotÇÃògëgÇ›Ç≈Ç≈Ç´ÇÈåè



class FsAirlinerAutopilot
{
public:
	enum AIRLINER_STATE
	{
		STATE_INITIAL, // <- Assess the situation and appropriately initialize
		STATE_STATIONARY,
		STATE_WAIT_FOR_FUELTRUCK,
		STATE_REFUELING,
		STATE_DISMISS_FUELTRUCK,
		STATE_TAXI_FOR_TAKEOFF,
		STATE_TAKEOFF,
		STATE_ENROUTE,
		STATE_HOLDING,
		STATE_APPROACH,
		STATE_TAXI_AFTER_LANDING,
	};

	int airRouteIdx;
	class YsSceneryAirRoute *airRoute;


	// STATE_STATIONARY >>
	YSBOOL needToDismissFuelTruck;
	// STATE_STATIONARY <<

	// STATE_HOLDING >>
	YsVec3 holdingFix;
	// STATE_HOLDING <<

	FsLandingAutopilot *landingAP;
	FsGoToPosition *cruiseAP;
	FsTaxiingAutopilot *taxiAP;
};

