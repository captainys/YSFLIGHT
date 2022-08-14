#include <ysclass.h>
#include <yscompilerwarning.h>

#include "fs.h"

void FsWeaponSmokeTrail::Draw(const YsVec3 &pos,const YsAtt3 &att,YSBOOL transparency,FSSMOKETYPE smkType,const double &cTime) const
{
	YsDisregardVariable(pos);
	YsDisregardVariable(att);
	YsDisregardVariable(transparency);
	YsDisregardVariable(smkType);
	YsDisregardVariable(cTime);
}

void FsWeapon::Draw(
    YSBOOL coarse,const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,
    YSBOOL transparency,FSSMOKETYPE smkType,const double &cTime,unsigned int drawFlag) const
{
	YsDisregardVariable(coarse);
	YsDisregardVariable(viewMat);
	YsDisregardVariable(projMat);
	YsDisregardVariable(transparency);
	YsDisregardVariable(smkType);
	YsDisregardVariable(cTime);
	YsDisregardVariable(drawFlag);
}

////////////////////////////////////////////////////////////

void FsWeaponHolder::BeginDraw(void) const
{
}

void FsWeaponHolder::EndDraw(void) const
{
}

void FsWeaponHolder::DrawGunCalibrator(void) const
{
}

