#ifndef FSSIMEXTENSION_ENDURANCE_IS_INCLUDED
#define FSSIMEXTENSION_ENDURANCE_IS_INCLUDED
/* { */

#include "fssimextension.h"

class FsSimExtension_EnduranceMode : public FsSimExtensionBase
{
public:
	typedef FsSimExtension_EnduranceMode THISCLASS;

    int nEnemyMax;
    double remainTime;
    double gLimit;
    YSBOOL jet;
    YSBOOL ww2;
	YSBOOL allowAAM;

	FsSimExtension_EnduranceMode();

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
