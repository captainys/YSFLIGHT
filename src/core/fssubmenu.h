#ifndef FSSUBMENU_IS_INCLUDED
#define FSSUBMENU_IS_INCLUDED
/* { */

#include <ysclass.h>
#include <ysbitmap.h>

enum FSSUBMENUITEMTYPE
{
	FSSUBMENUITEM_NULL,

	FSSUBMENUITEM_BACK,

	FSSUBMENUITEM_ATC_REQUESTAPPROACH,
	FSSUBMENUITEM_ATC_CANCELIFR,
	FSSUBMENUITEM_ATC_REQUESTVECTOR,
	FSSUBMENUITEM_ATC_MISSEDAPPROACH,
	FSSUBMENUITEM_ATC_REQUESTNEWVECTOR
};

class FsSubMenuItem
{
public:
	FSSUBMENUITEMTYPE itemType;
	YsString message;
	YsString appendix;
	YsBitmap bmp;

	FsSubMenuItem();
	void Initialize(void);
};

enum FSSUBMENU
{
	FSSUBMENU_NONE,
	FSSUBMENU_WAITKEYRELEASE, // <- Used for waiting for key release.
	FSSUBMENU_NOLONGERUSED1,
	FSSUBMENU_NOLONGERUSED2,
	FSSUBMENU_NOLONGERUSED3,
	FSSUBMENU_NOLONGERUSED8,
	FSSUBMENU_NOLONGERUSED5,
	FSSUBMENU_OPENSUBWINDOW,
	FSSUBMENU_OPENLEFTSUBWINDOW,
	FSSUBMENU_OPENRIGHTSUBWINDOW,
	FSSUBMENU_OPENLEFTSUBWINDOWCOCKPITVIEW,
	FSSUBMENU_OPENRIGHTSUBWINDOWCOCKPITVIEW,
	FSSUBMENU_SELECTVOR,
	FSSUBMENU_SELECTILS,
	FSSUBMENU_INFLTCONFIG,
	FSSUBMENU_GUNNER,
	FSSUBMENU_NOLONGERUSED7,
	FSSUBMENU_NOLONGERUSED6,
	FSSUBMENU_NOLONGERUSED4,
};

class FsSubMenu
{
private:
	YsListAllocator <FsSubMenuItem> menuItemAlloc;
	YsListContainer <FsSubMenuItem> menuItemList;

	FSSUBMENU subMenu;
	int subMenuBase;
	YsArray <class FsAirplane *> availableComTarget,comTarget;
	int fuelTruckOnCallKey;
	int ctlNavId;

	enum
	{
		NSHOWMAX=8
	};

public:
	FsSubMenu();
	virtual ~FsSubMenu();

	void CleanUp(void);

	FSSUBMENU GetSubMenu(void) const;
	void SetSubMenu(class FsSimulation *sim,FSSUBMENU subMenuIn);
private:
	void PrepareSubMenu(class FsSimulation *sim,FSSUBMENU subMenuIn);

public:
	void SelectNav(int navIdIn);

	void CheckKeyRelease(void);
	YSBOOL SubMenuEatRawKey(FSSUBMENU subMenu,int rawKey);
	void ProcessSubMenu(class FsSimulation *sim,class FsFlightConfig &cfg,int rawKey);

public:
	void Draw(const class FsSimulation *sim,class FsFlightConfig &cfg,int &sx,int &sy) const;
};

/* } */
#endif
