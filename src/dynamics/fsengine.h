#ifndef FSENGINE_IS_INCLUDED
#define FSENGINE_IS_INCLUDED
/* { */


#include <ysclass.h>


class FsEngine
{
};


class FsAircraftEngine : public FsEngine
{
public:
	YsAtt3 relAtt;  // Attitude relative to the airframe.
	YsVec3 relPos;

	FsAircraftEngine();
};



/* } */
#endif
