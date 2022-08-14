#ifndef FSAIBASIC_IS_INCLUDED
#define FSAIBASIC_IS_INCLUDED
/* { */

#include <ysclass.h>

class FsAIObject
{
};

class FsHoldUntil : public FsAIObject
{
protected:
	int thisObjGoesAway;
public:
	FsHoldUntil();
	void Initialize(void);
	void ClearHoldingCondition(void);
	void SetHoldUntilThisObjectGoesAway(const class FsExistence *obj);
	YSBOOL NeedToHold(const class FsSimulation *sim,const class FsExistence *obj);
};


/* } */
#endif
