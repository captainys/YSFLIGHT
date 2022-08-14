#ifndef FSSIMEXTENSION_RACING_IS_INCLUDED
#define FSSIMEXTENSION_RACING_IS_INCLUDED
/* { */

#include "fssimextension.h"

class FsSimExtension_RacingMode : public FsSimExtensionBase
{
public:
	class CheckPointTime
	{
	public:
		int nCheckPointLeft;
		double ctime;
	};


	typedef FsSimExtension_RacingMode THISCLASS;

	int nCheckPoint;
	int prevNumCheckPoint;
	YsArray <CheckPointTime> currentRun;
	double goalTime;

	FsSimExtension_RacingMode();

	static const char *Ident(void);
	static std::shared_ptr <FsSimExtensionBase> Generate(void);
	static std::function <std::shared_ptr <FsSimExtensionBase>(void)> Generator(void);

	virtual const char *GetIdent(void) const;
	virtual YsArray <YsString> Serialize(const FsSimulation *sim);
	virtual YSRESULT ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv);

	virtual YSBOOL CanContinue(FsSimulation *sim) const;
	virtual YSBOOL CanResume(FsSimulation *sim) const;

	virtual void StartSimulation(FsSimulation *sim);
	virtual void EndSimulation(FsSimulation *sim);

	virtual void StartReplay(FsSimulation *sim);
	virtual void EndReplay(FsSimulation *sim);

	virtual void OnInterval(FsSimulation *sim,double dt);
	virtual YsArray <LowerLeftAndBitmap,0> Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const;
};


/* } */
#endif
