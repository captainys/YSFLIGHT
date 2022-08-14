#include <stdio.h>
#include <ysclass.h>

#include "fspersona.h"

#include "fsexistence.h"


FsPersona::FsPersona()
{
}

void FsPersona::Initialize(void)
{
}

////////////////////////////////////////////////////////////

FsUser::FsUser()
{
	Initialize();
}

void FsUser::Initialize(void)
{
	FsPersona::Initialize();
	airPtr=NULL;
	gndPtr=NULL;
}

void FsUser::BoardAircraft(class FsAirplane *airPtr)
{
	this->airPtr=airPtr;
	this->gndPtr=NULL;
}

void FsUser::BoardGround(class FsGround *gndPtr)
{
	this->airPtr=NULL;
	this->gndPtr=gndPtr;
	if(NULL!=gndPtr)
	{
		gndPtr->Prop().PrepareUserWeaponofChoice();
	}
}

FsAirplane *FsUser::GetAircraft(void) const
{
	return airPtr;
}

FsGround *FsUser::GetGround(void) const
{
	return gndPtr;
}

////////////////////////////////////////////////////////////

FsLocalUser::FsLocalUser()
{
}

void FsLocalUser::Initialize(void)
{
	FsUser::Initialize();
}

////////////////////////////////////////////////////////////

FsRemoteUser::FsRemoteUser()
{
}

void FsRemoteUser::Initialize(void)
{
	FsUser::Initialize();
}

