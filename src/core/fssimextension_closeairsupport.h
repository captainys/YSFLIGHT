#ifndef FSSIMEXTENSION_CLOSEAIRSUPPORT_IS_INCLUDED
#define FSSIMEXTENSION_CLOSEAIRSUPPORT_IS_INCLUDED
/* { */

#include "fssimextension.h"
#include "fssiminfo.h"

class FsSimExtension_CloseAirSupport : public FsSimExtensionBase
{
public:
	typedef FsSimExtension_CloseAirSupport THISCLASS;

	const double initRemainTime;
	const int maxNumTank;

    int nEnemy;
    double remainTime;
    double gLimit;
    FsCloseAirSupportMissionInfo info;

	FsSimExtension_CloseAirSupport();

	static const char *Ident(void);
	static std::shared_ptr <FsSimExtensionBase> Generate(void);
	static std::function <std::shared_ptr <FsSimExtensionBase>(void)> Generator(void);

	virtual const char *GetIdent(void) const;
	virtual YsArray <YsString> Serialize(const FsSimulation *sim);
	virtual YSRESULT ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv);

	virtual void StartSimulation(FsSimulation *sim);
	virtual void EndSimulation(FsSimulation *sim);

	virtual void OnInterval(FsSimulation *sim,double dt);
	virtual YsArray <LowerLeftAndBitmap,0> Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const;
};

/* } */
#endif
