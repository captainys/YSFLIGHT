#ifndef FSPERSONA_IS_INCLUDED
#define FSPERSONA_IS_INCLUDED
/* { */

class FsPersona
{
public:
	FsPersona();
	void Initialize(void);
};

class FsUser : public FsPersona
{
private:
	class FsAirplane *airPtr;
	class FsGround *gndPtr;
public:
	FsUser();
	void Initialize(void);
	void BoardAircraft(class FsAirplane *airPtr);
	void BoardGround(class FsGround *gndPtr);
	class FsAirplane *GetAircraft(void) const;
	class FsGround *GetGround(void) const;
};

class FsLocalUser : public FsUser
{
public:
	FsLocalUser();
	void Initialize(void);
};

class FsRemoteUser : public FsUser
{
public:
	FsRemoteUser();
	void Initialize(void);
};
/* } */
#endif
