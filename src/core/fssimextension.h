#ifndef FSSIMEXTENSION_IS_INCLUDED
#define FSSIMEXTENSION_IS_INCLUDED
/* { */

#include <vector>
#include <functional>
#include <memory>

#include <ysclass.h>
#include <ysbitmap.h>

#include "fsdef.h"
#include "fsweapon.h"

class FsSimulation;

// FsWorld needs to know all possible add-ons to load from .YFS file.


class FsSimExtensionBase
{
public:
	class LowerLeftAndBitmap
	{
	public:
		/*! Lower-Left corner window coordinate.
		*/
		YsVec2i pos;

		/*! 24-bit RGBA bitmap.
		*/
		YsBitmap bmp;

		LowerLeftAndBitmap(){};
		LowerLeftAndBitmap(const LowerLeftAndBitmap &incoming);
		LowerLeftAndBitmap(LowerLeftAndBitmap &&incoming);
		LowerLeftAndBitmap &operator=(const LowerLeftAndBitmap &incoming);
		LowerLeftAndBitmap &operator=(LowerLeftAndBitmap &&incoming);
	};

	virtual const char *GetIdent(void) const=0;

	virtual ~FsSimExtensionBase(){}

	/*! Process key stroke.
	    Return YSOK if it processes the key stroke, and it shouldn't be further processed.
	    Return YSERR if it didn't process the key stroke, and it should be processed in the downstream.
	*/
	YSRESULT OnKeyDown(FsSimulation *sim,int fskey){return YSERR;}

	/*! Return non-zero value  (>YsTolerance) to force use time step.
	    If multiple extensions are installed.  The last extension has the priority.
	*/
	virtual double UseCustomTimeStep(const FsSimulation *sim) const{return 0.0;}

	virtual FSUSERCONTROL OverrideUserControl(const FsSimulation *sim) const {return FSUSC_DONTCARE;}

	virtual YSBOOL OverrideRecording(const FsSimulation *sim) const {return YSTFUNKNOWN;}

	/*! The first arg must be "EXTENSIO"
	    The second arg must be identifier for the add-on
	*/
	virtual YsArray <YsString> Serialize(const FsSimulation *sim)=0;

	/*! The first arg will be "EXTENSIO" and the second arg will be the identifier.
	*/
	virtual YSRESULT ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv)=0;

	/*! Return YSTRUE if the simulation must be terminated.
	    It changes the simulation state to CHECKCONTINUE.
	    If the simulation must be terminated right away, return YSTRUE for this function and CanContinue.
	*/
	virtual YSBOOL MustTerminate(const FsSimulation *sim) const{return YSFALSE;}

	/*! Return YSTRUE if the simulation must not terminate when the player vehicle is killed.
	    FsSimulation resets the endTime to zero if one of the extensions returns YSTRUE for MustKeepRunning.
	    If one of the extensions returns YSTRUE for MustTerminate, MustTerminate has the priority.
	    Or, if the user presses ESC twice, the simulation terminates anyway.
	*/
	virtual YSBOOL MustKeepRunning(const FsSimulation *sim) const{return YSFALSE;}

	virtual YSBOOL CanContinue(FsSimulation *sim) const{return YSTRUE;}
	virtual YSBOOL CanResume(FsSimulation *sim) const{return YSTRUE;}

	virtual void StartSimulation(FsSimulation *sim){};
	virtual void EndSimulation(FsSimulation *sim){};

	virtual void StartReplay(FsSimulation *sim){};
	virtual void EndReplay(FsSimulation *sim){};

	virtual void BeforeSimulateOneStep(FsSimulation *sim){};
	virtual void AfterSimulateOneStep(FsSimulation *sim){};


	virtual void OnObjKilledByWeapon(FsSimulation *sim,const class FsExistence &killed,const FsExistence &credited,FSWEAPONTYPE wpnType){};
	virtual void OnObjGetDamage(FsSimulation *sim,const FsExistence &killed,const FsExistence &credited,int damage,FSDIEDOF diedOf){};
	virtual void OnMissileLaunch(const FsWeaponRecord &rec){};
	virtual void OnChatMessage(const char msg[]){};


	/*! This function is called after every movement.
	*/
	virtual void OnInterval(FsSimulation *sim,double dt)
	{
	}

	/*! To override screen-drawing totally, override this function and return YSOK.
	    If the screen should be drawn by FsSimulation, return YSERR.
	*/
	virtual YSRESULT OnDraw(const FsSimulation *sim) const {return YSERR;}

	/*! windowId: 0=main window  1=sub window left  2=sub window right
	*/
	virtual YsArray <LowerLeftAndBitmap,0> Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const
	{
		YsArray <LowerLeftAndBitmap,0> empty;
		return empty;
	}

protected:
	const char *FirstCommand(void) const;
};


class FsSimExtensionRegistry
{
private:
	class IdentAndGenerator
	{
	public:
		YsString ident;
		std::function <std::shared_ptr <FsSimExtensionBase>(void)> generator;
	};

	std::vector <IdentAndGenerator> extensionRegistry;
public:
	void RegisterExtension(const char ident[],std::function <std::shared_ptr <FsSimExtensionBase>(void)> generator);
	std::shared_ptr <FsSimExtensionBase> Create(const char ident[]);
	void RegisterKnownExtension(void); // How can I make it dynamic?
};

/* } */
#endif
