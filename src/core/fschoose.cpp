#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#ifdef WIN32  // Assuming UNIX
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <sys/time.h>
#endif
#include <time.h>


#include <ysclass.h>
#include <ysbitmap.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>
#include "fsgui.h"


#include "fs.h"

#include "fsfilename.h"

#include "fsguiselectiondialogbase.h"


#include "fsdef.h"
#include "fschoose.h"
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#include "fstextresource.h"


FsChoose::FsChoose(int n) : fullChoice(fullChoiceAllocator), filterList(fullChoiceAllocator)
{
	currentCursorPosition=0;
	currentMouseSelectPosition=-1;
	currentMouseButtonPosition=-1;

	nShow=n;

	choosingAirplane=YSFALSE;
	choosingScenery=YSFALSE;
	scn=NULL;

	enableFilter=YSFALSE;


	showNormal=YSTRUE;
	showUtility=YSTRUE;
	showAerobatic=YSTRUE;
	showFighter=YSTRUE;
	showAttacker=YSTRUE;
	showTrainer=YSTRUE;
	showHeavyBomber=YSTRUE;
	showWW2Fighter=YSTRUE,
	showWW2Bomber=YSTRUE;

	choosingFile=YSFALSE;
	choosingFileForReadOnly=YSFALSE;
	curName.Set("");
	curPath.Set("");

	showCancelButton=YSFALSE;

	searchKeyWord.Set("");
	keyWordSearchMode=YSFALSE;
	keyWordNoMatch=YSFALSE;

	aam=0;
	aim9x=0;
	aim120=0;
	agm=0;
	bomb=0;
	bomb250=0;
	bomb500hd=0;
	rocket=0;
	fuel=75;

	allowAam=YSTRUE;
	allowAgm=YSTRUE;
	allowBomb=YSTRUE;
	allowRocket=YSTRUE;

	prevLb=YSFALSE;
	prevMb=YSFALSE;
	prevRb=YSFALSE;

	okButtonVisible=YSFALSE;
	mouseOnOkButton=YSFALSE;
	okButtonX0=0;
	okButtonY0=0;
	okButtonClicked=YSFALSE;

	mouseOnCancelButton=YSFALSE;
	cancelButtonX0=0;
	cancelButtonY0=0;
	cancelButtonClicked=YSFALSE;

	smk=YSFALSE;
	smkCol=YsWhite();
	smokeColorMode=YSFALSE;
	wld=NULL;

	lastCursorMoveClock=0;

	att.Set(YsDegToRad(170.0),YsDegToRad(7.0),0.0);
}

FsChoose::~FsChoose()
{
	if(scn!=NULL)
	{
		delete scn;
	}
}

YSRESULT FsChoose::NetworkAddFilterWord(int n,const YsString airNameFilter[])
{
	int i;
	YsListItem <YsString> *neo;
	for(i=0; i<n; i++)
	{
		if(filter.GetId(airNameFilter[i])<0) // 2007/01/07
		{
			neo=filterList.Create();
			neo->dat=airNameFilter[i];
		}
	}

	YsArray <const char *> lst;
	neo=NULL;
	while(NULL!=(neo=filterList.FindNext(neo)))
	{
		lst.Append(neo->dat);
	}
	lst.Append(NULL);

	filter.MakeList(lst);

	return YSOK;
}

YSRESULT FsChoose::MakeAirplaneList(FsWorld *world)
{
	int i;
	const char *name;
	YsListItem <YsString> *neoFullChoice;

	choice.Set(0,NULL);
	fullChoice.CleanUp();

	for(i=0; (name=world->GetAirplaneTemplateName(i))!=NULL; i++)
	{
		if(enableFilter==YSTRUE && filter.GetId(name)<0)
		{
			continue;
		}


		FSAIRPLANECATEGORY airCat;
		airCat=world->GetAirplaneTemplateCategory(i);
		if((airCat==FSAC_NORMAL && showNormal==YSTRUE) ||
		   (airCat==FSAC_UTILITY && showUtility==YSTRUE) ||
		   (airCat==FSAC_AEROBATIC && showAerobatic==YSTRUE) ||
		   (airCat==FSAC_FIGHTER && showFighter==YSTRUE) ||
		   (airCat==FSAC_ATTACKER && showAttacker==YSTRUE) ||
		   (airCat==FSAC_TRAINER && showTrainer==YSTRUE) ||
		   (airCat==FSAC_HEAVYBOMBER && showHeavyBomber==YSTRUE) ||
		   (airCat==FSAC_WW2FIGHTER && showWW2Fighter==YSTRUE) ||
		   (airCat==FSAC_WW2BOMBER && showWW2Bomber==YSTRUE))
		{
			neoFullChoice=fullChoice.Create();
			neoFullChoice->dat.Set(name);
			choice.Append(neoFullChoice->dat);
		}
	}

	if(choice.GetN()==0)
	{
		neoFullChoice=fullChoice.Create();
		neoFullChoice->dat.Set(world->GetAirplaneTemplateName(0));
		choice.Append(neoFullChoice->dat);
	}

	choosingAirplane=YSTRUE;
	choosingScenery=YSFALSE;
	wld=world;

	ResetOrdinanceOfAirplane();

	ReduceChoiceByKeyWord();

	return YSOK;
}

YSRESULT FsChoose::MakeStartPositionList(FsWorld *world,const char fieldName[])
{
	int i;
	YsString name;

	choice.Set(0,NULL);
	fullChoice.CleanUp();

	for(i=0; world->GetFieldStartPositionName(name,fieldName,i)==YSOK; i++)
	{
		AddChoice(name);
	}
	wld=world;
	return YSOK;
}

YSRESULT FsChoose::ChooseByName(const char name[])
{
	int i;

	forYsArray(i,choice)
	{
		if(strcmp(choice[i],name)==0)
		{
			currentCursorPosition=i;
			goto OKEND;
		}
	}

	if(choice.GetN()>0 && strcmp(name,choice[0])<0)
	{
		currentCursorPosition=0;
		goto OKEND;
	}

	forYsArray(i,choice)
	{
		if(0<=strcmp(name,choice[i]) && (i==choice.GetN()-1 || strcmp(name,choice[i+1])<=0))
		{
			currentCursorPosition=i;
			goto OKEND;
		}
	}

	return YSERR;

OKEND:
	if(choosingAirplane==YSTRUE)
	{
		ResetOrdinanceOfAirplane();
	}
	return YSOK;
}

YSRESULT FsChoose::ResetOrdinanceOfAirplane(void)
{
	const FsAirplaneTemplate *tmpl;
	if(IsChoiceValid()==YSTRUE &&
	   (tmpl=wld->GetAirplaneTemplate(choice[currentCursorPosition]))!=NULL)
	{
		aam=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_AIM9);
		aim9x=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_AIM9X);
		aim120=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_AIM120);
		agm=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_AGM65);
		bomb=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_BOMB);
		bomb250=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_BOMB250);
		bomb500hd=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_BOMB500HD);
		rocket=tmpl->GetProperty()->GetNumWeapon(FSWEAPON_ROCKET);
		smk=(tmpl->GetProperty()->GetSmokeOil()>YsTolerance ? YSTRUE : YSFALSE);
		tmpl->GetProperty()->GetWeaponConfig(weaponConfig);

		ApplyWeaponLimit(weaponConfig);

		FsAirplaneProperty prop;
		prop=*tmpl->GetProperty();
		prop.ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);
		prop.GetWeaponConfig(weaponConfig);
		aam=prop.GetNumWeapon(FSWEAPON_AIM9);
		aim9x=prop.GetNumWeapon(FSWEAPON_AIM9X);
		aim120=prop.GetNumWeapon(FSWEAPON_AIM120);
		agm=prop.GetNumWeapon(FSWEAPON_AGM65);
		bomb=prop.GetNumWeapon(FSWEAPON_BOMB);
		bomb250=prop.GetNumWeapon(FSWEAPON_BOMB250);
		bomb500hd=prop.GetNumWeapon(FSWEAPON_BOMB500HD);
		rocket=prop.GetNumWeapon(FSWEAPON_ROCKET);

		// prop.ShowSlotConfig();

		return YSOK;
	}
	return YSERR;
}

void FsChoose::ApplyWeaponLimit(YsArray <int,64> &weaponConfig)
{
	int i;
	i=0;
	while(i<weaponConfig.GetN()-1)
	{
		FSWEAPONTYPE wpnType;
		wpnType=(FSWEAPONTYPE)weaponConfig[i];
		if(allowAam!=YSTRUE && (wpnType==FSWEAPON_AIM9 || wpnType==FSWEAPON_AIM9X || wpnType==FSWEAPON_AIM120))
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else if(allowAgm!=YSTRUE && wpnType==FSWEAPON_AGM65)
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else if(allowBomb!=YSTRUE && (wpnType==FSWEAPON_BOMB || wpnType==FSWEAPON_BOMB250 || wpnType==FSWEAPON_BOMB500HD))
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else if(allowRocket!=YSTRUE && (wpnType==FSWEAPON_ROCKET))
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else
		{
			i+=2;
		}
	}
}

YSRESULT FsChoose::KeyIn(int ky,int c)
{
	YSRESULT res;
	int prevCursorPosition;
	prevCursorPosition=currentCursorPosition;

	res=YSERR;

	if(ky==FSKEY_UP)
	{
		if(FsGetKeyState(FSKEY_SHIFT)==YSTRUE || FsGetKeyState(FSKEY_CTRL)==YSTRUE)
		{
			currentCursorPosition-=5;
		}
		else
		{
			currentCursorPosition-=1;
		}
		res=YSOK;
	}
	else if(ky==FSKEY_DOWN)
	{
		if(FsGetKeyState(FSKEY_SHIFT)==YSTRUE || FsGetKeyState(FSKEY_CTRL)==YSTRUE)
		{
			currentCursorPosition+=5;
		}
		else
		{
			currentCursorPosition+=1;
		}
		res=YSOK;
	}
	else if(ky==FSKEY_PAGEUP)
	{
		currentCursorPosition-=nShow;
		res=YSOK;
	}
	else if(ky==FSKEY_PAGEDOWN)
	{
		currentCursorPosition+=nShow;
		res=YSOK;
	}
	else if(ky==FSKEY_HOME)
	{
		currentCursorPosition=0;
		res=YSOK;
	}
	else if(ky==FSKEY_END)
	{
		currentCursorPosition=(int)choice.GetN()-1;
		res=YSOK;
	}

	if(prevCursorPosition!=currentCursorPosition)
	{
#ifdef WIN32
		lastCursorMoveClock=clock();
#else
		struct timeval tm;
		gettimeofday(&tm,NULL);
		lastCursorMoveClock=tm.tv_sec*1000+tm.tv_usec/1000;
#endif
	}

	currentCursorPosition=YsBound <int> (currentCursorPosition,0,(int)choice.GetN()-1);

	if(KeyInForModeChange(ky,c)==YSOK ||
	   KeyInForSearchKeyWord(ky,c)==YSOK || 
	   KeyInForSmokeColorCode(ky,c)==YSOK)
	{
		return YSOK;
	}

	if(choosingAirplane==YSTRUE)
	{
		if(prevCursorPosition!=currentCursorPosition)
		{
			ResetOrdinanceOfAirplane();
		}

		const FsAirplaneTemplate *tmpl;

		tmpl=wld->GetAirplaneTemplate(choice[currentCursorPosition]);
		if(tmpl!=NULL)
		{
			FSWEAPONTYPE wpnType;
			int add;

			wpnType=FSWEAPON_GUN;
			add=0;

			if(ky==FSKEY_Q)
			{
				AddRemoveLoading(FSWEAPON_AIM9,1);
				res=YSOK;
			}
			else if(ky==FSKEY_A)
			{
				AddRemoveLoading(FSWEAPON_AIM9,-1);
				res=YSOK;
			}
			else if(ky==FSKEY_W)
			{
				AddRemoveLoading(FSWEAPON_AIM120,1);
				res=YSOK;
			}
			else if(ky==FSKEY_S)
			{
				AddRemoveLoading(FSWEAPON_AIM120,-1);
				res=YSOK;
			}
			else if(ky==FSKEY_E)
			{
				AddRemoveLoading(FSWEAPON_AGM65,1);
				res=YSOK;
			}
			else if(ky==FSKEY_D)
			{
				AddRemoveLoading(FSWEAPON_AGM65,-1);
				res=YSOK;
			}
			else if(ky==FSKEY_R)
			{
				AddRemoveLoading(FSWEAPON_BOMB,1);
				res=YSOK;
			}
			else if(ky==FSKEY_F)
			{
				AddRemoveLoading(FSWEAPON_BOMB,-1);
				res=YSOK;
			}
			else if(ky==FSKEY_T)
			{
				AddRemoveLoading(FSWEAPON_BOMB250,1);
				res=YSOK;
			}
			else if(ky==FSKEY_G)
			{
				AddRemoveLoading(FSWEAPON_BOMB250,-1);
				res=YSOK;
			}
			else if(ky==FSKEY_Y)
			{
				AddRemoveLoading(FSWEAPON_ROCKET,tmpl->GetProperty()->GetMaxNumWeapon(FSWEAPON_ROCKET));
				res=YSOK;
			}
			else if(ky==FSKEY_H)
			{
				AddRemoveLoading(FSWEAPON_ROCKET,-tmpl->GetProperty()->GetMaxNumWeapon(FSWEAPON_ROCKET));
				res=YSOK;
			}
			else if(ky==FSKEY_U)
			{
				smk=YSTRUE;
				res=YSOK;
			}
			else if(ky==FSKEY_J)
			{
				smk=YSFALSE;
				res=YSOK;
			}
			else if(ky==FSKEY_I)
			{
				fuel=YsSmaller(fuel+5,100);
				res=YSOK;
			}
			else if(ky==FSKEY_K)
			{
				fuel=YsGreater(fuel-5,0);
				res=YSOK;
			}
		}

		YsString curChoice;
		curChoice.Set(choice[currentCursorPosition]);
		switch(ky)
		{
		case FSKEY_F1:
			currentCursorPosition=0;
			YsFlip(showNormal);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F2:
			currentCursorPosition=0;
			YsFlip(showUtility);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F3:
			currentCursorPosition=0;
			YsFlip(showAerobatic);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F4:
			currentCursorPosition=0;
			YsFlip(showFighter);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F5:
			currentCursorPosition=0;
			YsFlip(showAttacker);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F6:
			currentCursorPosition=0;
			YsFlip(showTrainer);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F7:
			currentCursorPosition=0;
			YsFlip(showHeavyBomber);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F8:
			currentCursorPosition=0;
			YsFlip(showWW2Fighter);
			MakeAirplaneList(wld);
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		case FSKEY_F9:
			currentCursorPosition=0;
			YsFlip(showWW2Bomber);
			MakeAirplaneList(wld);
			currentCursorPosition=0;
			ChooseByName(curChoice);
			// ResetOrdinanceOfAirplane();  <- done in ChooseByName

			res=YSOK;
			break;
		}
	}
	else if(choosingFile==YSTRUE)
	{
		if(choice[currentCursorPosition][0]!='*')
		{
			curName.Set(choice[currentCursorPosition]);
		}
		else if(strncmp(choice[currentCursorPosition],"*NEWFILE",8)==0)
		{
			curName.Set((const char *)choice[currentCursorPosition]+10);  // Skipping "*NEWFILE* "
		}
		else
		{
			curName.Set("");
		}
	}

	return res;
}

YSRESULT FsChoose::ProcessMouse(int x0,int y0,YSBOOL lb,YSBOOL mb,YSBOOL rb,int mx,int my)
{
	//  DY:16  DX:240
	YSRESULT res;

	res=YSERR;

	if(x0<=mx && mx<x0+240 && y0-16<=my && my<y0+(nShow+1)*16)
	{
		int y;
		y=(my-y0+16)/16;
		if(0<=y && y<nShow+2)
		{
			currentMouseSelectPosition=y;
		}
	}
	else
	{
		currentMouseSelectPosition=-1;
	}

	if(x0+240<=mx && mx<x0+480 && y0+160-16<=my && my<y0+160+144-16)
	{
		currentMouseButtonPosition=(my-y0-144)/16;
	}
	else
	{
		currentMouseButtonPosition=-1;
	}

	if(okButtonX0<=mx && mx<okButtonX1 && okButtonY0<=my && my<okButtonY1)
	{
		mouseOnOkButton=YSTRUE;
	}
	else
	{
		mouseOnOkButton=YSFALSE;
	}

	if(cancelButtonX0<=mx && mx<cancelButtonX1 && cancelButtonY0<=my && my<cancelButtonY1)
	{
		mouseOnCancelButton=YSTRUE;
	}
	else
	{
		mouseOnCancelButton=YSFALSE;
	}


	if(prevLb!=YSTRUE && lb==YSTRUE)
	{
		if(x0<=mx && mx<x0+240 && y0-16<=my && my<y0+(nShow+1)*16)
		{
			int currentShowPosition,prevCursorPosition;
			prevCursorPosition=currentCursorPosition;

			currentShowPosition=(currentCursorPosition/nShow)*nShow;
			if(currentMouseSelectPosition==0)
			{
				currentCursorPosition-=nShow;
			}
			else if(currentMouseSelectPosition==nShow+1)
			{
				currentCursorPosition+=nShow;
			}
			else if(0<=(currentMouseSelectPosition-1) && (currentMouseSelectPosition-1)<nShow)
			{
				currentCursorPosition=currentShowPosition+(currentMouseSelectPosition-1);
			}

			if(prevCursorPosition!=currentCursorPosition)
			{
#ifdef WIN32
				lastCursorMoveClock=clock();
#else
				struct timeval tm;
				gettimeofday(&tm,NULL);
				lastCursorMoveClock=tm.tv_sec*1000+tm.tv_usec/1000;
#endif
				res=YSOK;
			}
			currentCursorPosition=YsBound <int> (currentCursorPosition,0,(int)choice.GetN()-1);
			if(choosingAirplane==YSTRUE)       // 2006/07/24
			{                                  // 2006/07/24
				ResetOrdinanceOfAirplane();    // 2006/07/24
			}                                  // 2006/07/24
		}
		if(choosingAirplane==YSTRUE && x0+240<=mx && mx<x0+480 && y0+160-16<=my && my<y0+160+144-16)
		{
			YsString curChoice;
			curChoice.Set(choice[currentCursorPosition]);
			switch(currentMouseButtonPosition)
			{
			case 0:
				currentCursorPosition=0;
				YsFlip(showNormal);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 1:
				currentCursorPosition=0;
				YsFlip(showUtility);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 2:
				currentCursorPosition=0;
				YsFlip(showAerobatic);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 3:
				currentCursorPosition=0;
				YsFlip(showFighter);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 4:
				currentCursorPosition=0;
				YsFlip(showAttacker);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 5:
				currentCursorPosition=0;
				YsFlip(showTrainer);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 6:
				currentCursorPosition=0;
				YsFlip(showHeavyBomber);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 7:
				currentCursorPosition=0;
				YsFlip(showWW2Fighter);
				MakeAirplaneList(wld);
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			case 8:
				currentCursorPosition=0;
				YsFlip(showWW2Bomber);
				MakeAirplaneList(wld);
				currentCursorPosition=0;
				ChooseByName(curChoice);
				// ResetOrdinanceOfAirplane();  <- done in ChooseByName
				res=YSOK;
				break;
			}
		}
	}
	else if(prevLb==YSTRUE && lb!=YSTRUE)
	{
		if(mouseOnOkButton==YSTRUE)
		{
			okButtonClicked=YSTRUE;
			// res should not be YSOK.
		}
		else if(mouseOnCancelButton==YSTRUE)
		{
			cancelButtonClicked=YSTRUE;
		}
	}

	prevLb=lb;
	prevMb=mb;
	prevRb=rb;
	return res;
}

YSRESULT FsChoose::KeyInForModeChange(int ky,int /*c*/)
{
	if(ky==FSKEY_F && FsGetKeyState(FSKEY_CTRL)==YSTRUE)
	{
		YsFlip(keyWordSearchMode);
		if(keyWordSearchMode==YSTRUE)
		{
			smokeColorMode=YSFALSE;
		}
		return YSOK;
	}
	if(choosingAirplane==YSTRUE && ky==FSKEY_S && FsGetKeyState(FSKEY_CTRL)==YSTRUE)
	{
		YsFlip(smokeColorMode);
		if(smokeColorMode==YSTRUE)
		{
			smokeColorCode.Set("");
			keyWordSearchMode=YSFALSE;
		}
		return YSOK;
	}
	return YSERR; // 2007/04/05 Symptom: Ctrl+F => type "bluean" => [BS][BS] => type "im" => Down Arrow => Ctrl+S then no key but F & S works.
}

YSRESULT FsChoose::KeyInForSearchKeyWord(int ky,int c)
{
	if(keyWordSearchMode==YSTRUE)
	{


		int prevStrlen=(int)searchKeyWord.Strlen();

		switch(ky)
		{
		case FSKEY_BS:
			searchKeyWord.BackSpace();
			if(searchKeyWord.Strlen()==0)
			{
				keyWordNoMatch=YSFALSE;
			}
			break;
		case FSKEY_ESC:
		case FSKEY_ENTER:
			keyWordSearchMode=YSFALSE;
			return YSOK;
		default:
			if(searchKeyWord.Strlen()<80 && isprint(c))
			{
				char s[2];
				s[0]=(char)c;

				s[1]=0;
				if(isprint(s[0]))
				{
					searchKeyWord.Append(s);
				}
			}
			break;
		}

		if(prevStrlen<searchKeyWord.Strlen())
		{
			ReduceChoiceByKeyWord();
			return YSOK;
		}
		else if(searchKeyWord.Strlen()<prevStrlen)
		{
			AddChoiceByKeyWord();
			return YSOK;
		}
	}

	return YSERR;
}

void FsChoose::ReduceChoiceByKeyWord(void)
{
	YsArray <YsString,16> args;
	YsArray <const char *> prevChoiceList;
	YsString curChoice;

	currentCursorPosition=0;
	if(choice.GetN()>0)
	{
		curChoice.Set(choice[currentCursorPosition]);
	}

	prevChoiceList=choice;


	if(searchKeyWord.Arguments(args)==YSOK && args.GetN()>0)
	{
		YSSIZE_T i;
		YsString cap;
		forYsArray(i,args)
		{
			args[i].Capitalize();
		}
		forYsArrayRev(i,choice)
		{
			cap.Set(choice[i]);
			cap.Capitalize();
			if(FsTestKeyWordMatch(cap,args.GetN(),args)!=YSTRUE)
			{
				choice.Delete(i);
			}
		}
	}

	if(choice.GetN()==0)
	{
		keyWordNoMatch=YSTRUE;
		choice=prevChoiceList;
	}

	ChooseByName(curChoice);
	// ResetOrdinanceOfAirplane();  <- done in ChooseByName
}

void FsChoose::AddChoiceByKeyWord(void)
{
	YsArray <YsString,16> args;
	YsArray <const char *> prevChoiceList;

	YsString curChoice;

	currentCursorPosition=0;
	if(choice.GetN()>0)
	{
		curChoice.Set(choice[currentCursorPosition]);
	}

	prevChoiceList=choice;

	if(searchKeyWord.Arguments(args)==YSOK && args.GetN()>0)
	{
		int i;
		YsString cap;
		YsListItem <YsString> *ptr;

		forYsArray(i,args)
		{
			args[i].Capitalize();
		}

		choice.Set(0,NULL);
		fullChoice.RewindPointer();
		while(NULL!=(ptr=fullChoice.StepPointer()))
		{
			cap=ptr->dat;
			cap.Capitalize();
			if(FsTestKeyWordMatch(cap,args.GetN(),args)==YSTRUE)
			{
				choice.Append(ptr->dat);

				keyWordNoMatch=YSFALSE;
			}
		}
	}
	else
	{
		YsListItem <YsString> *ptr;
		choice.Set(0,NULL);
		fullChoice.RewindPointer();
		while(NULL!=(ptr=fullChoice.StepPointer()))
		{
			choice.Append(ptr->dat);
			keyWordNoMatch=YSFALSE;
		}
	}

	if(choice.GetN()==0)
	{
		keyWordNoMatch=YSTRUE;
		choice=prevChoiceList;
	}

	ChooseByName(curChoice);
	// ResetOrdinanceOfAirplane();  <- done in ChooseByName
}

YSRESULT FsChoose::KeyInForSmokeColorCode(int ky,int c)
{
	if(choosingAirplane!=YSTRUE)
	{
		return YSERR;
	}

	if(smokeColorMode==YSTRUE)
	{


		int prevStrlen=(int)smokeColorCode.Strlen();

		switch(ky)
		{
		case FSKEY_BS:
			smokeColorCode.BackSpace();
			if(smokeColorCode.Strlen()==0)
			{
				keyWordNoMatch=YSFALSE;
			}
			break;
		case FSKEY_ESC:
			smokeColorMode=YSFALSE;
			return YSOK;
		default:
			char s[2];
			s[0]=(char)c;
			s[1]=0;
			if(isprint(s[0]))
			{
				smokeColorCode.Append(s);
				if(smokeColorCode.Strlen()>=6)
				{
					goto SETCOLOR;
				}
			}
			break;
		}
	}
	return YSERR;

SETCOLOR:
	smokeColorMode=YSFALSE;

	int i,ccode,r,g,b;
	ccode=0;
	for(i=0; i<smokeColorCode.Strlen(); i++)
	{
		ccode<<=4;
		if('0'<=smokeColorCode[i] && smokeColorCode[i]<='9')
		{
			ccode+=(smokeColorCode[i]-'0');
		}
		else if('a'<=smokeColorCode[i] && smokeColorCode[i]<='f')
		{
			ccode+=(smokeColorCode[i]-'a'+10);
		}
		else if('A'<=smokeColorCode[i] && smokeColorCode[i]<='F')
		{
			ccode+=(smokeColorCode[i]-'A'+10);
		}
	}

	r=(ccode>>16)&255;
	g=(ccode>>8)&255;
	b=ccode&255;

	smkCol.SetIntRGB(r,g,b);

	return YSOK;
}

void FsChoose::AddRemoveLoading(FSWEAPONTYPE wpnType,int addRemove)
{
	const FsAirplaneTemplate *tmpl;

	tmpl=wld->GetAirplaneTemplate(choice[currentCursorPosition]);
	if(tmpl!=NULL)
	{
		if(wpnType!=FSWEAPON_GUN && addRemove!=0)
		{
			FsAirplaneProperty prop;
			prop=*tmpl->GetProperty();

			if(addRemove>0)
			{
				int nOrg;

				prop.ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);
				// prop.ShowSlotConfig();

				nOrg=prop.GetNumWeapon(wpnType);
				prop.AddWeapon(wpnType,addRemove);

				if(prop.GetNumWeapon(wpnType)==nOrg) // If no increase.
				{
					weaponConfig.Insert(0,addRemove);
					weaponConfig.Insert(0,(int)wpnType);
					prop.ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);
				}
			}
			else
			{
				int i;
				for(i=(int)weaponConfig.GetN()-2; i>=0; i-=2)
				{
					if(weaponConfig[i]==wpnType)
					{
						addRemove+=weaponConfig[i+1];
						weaponConfig[i+1]=0;
						if(addRemove>=0)
						{
							weaponConfig[i+1]=addRemove;
							break;
						}
					}
				}
				prop.ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);
			}

			prop.GetWeaponConfig(weaponConfig);
			ApplyWeaponLimit(weaponConfig);
			prop.ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);

			aam=prop.GetNumWeapon(FSWEAPON_AIM9);
			aim9x=prop.GetNumWeapon(FSWEAPON_AIM9X);
			agm=prop.GetNumWeapon(FSWEAPON_AGM65);
			bomb=prop.GetNumWeapon(FSWEAPON_BOMB);
			rocket=prop.GetNumWeapon(FSWEAPON_ROCKET);
			aim120=prop.GetNumWeapon(FSWEAPON_AIM120);
			bomb250=prop.GetNumWeapon(FSWEAPON_BOMB250);
			bomb500hd=prop.GetNumWeapon(FSWEAPON_BOMB500HD);

			// prop.ShowSlotConfig();
		}
	}
}

YSBOOL FsChoose::IsWeaponAvailable(FSWEAPONTYPE wpnType)
{
	const FsAirplaneTemplate *tmpl;

	tmpl=wld->GetAirplaneTemplate(choice[currentCursorPosition]);
	if(tmpl!=NULL)
	{
		if(tmpl->GetProperty()->GetMaxNumWeapon(wpnType)<=0)
		{
			return YSFALSE;
		}

		switch(wpnType)
		{
		default:
			break;
		case FSWEAPON_AIM9:
		case FSWEAPON_AIM9X:
		case FSWEAPON_AIM120:
			return allowAam;
		case FSWEAPON_AGM65:
			return allowAgm;
		case FSWEAPON_BOMB:
		case FSWEAPON_BOMB250:
		case FSWEAPON_BOMB500HD:
			return allowBomb;
		case FSWEAPON_ROCKET:
			return allowRocket;
		}

		return YSTRUE;
	}
	return YSFALSE;
}

void FsChoose::Rotate(const double &h)
{
	att.SetH(att.h()+h);
}

int FsChoose::GetChoiceId(void)
{
	return currentCursorPosition;
}

const char *FsChoose::GetChoice(void)
{
	if(0<=currentCursorPosition && currentCursorPosition<choice.GetN())
	{
		return choice[currentCursorPosition];
	}
	else
	{
		return "(Not Chosen Yet)";
	}
}

YSBOOL FsChoose::IsChoiceValid(void)
{
	if(0<=currentCursorPosition && currentCursorPosition<choice.GetN())
	{
		return YSTRUE;
	}
	return YSFALSE;
}

int FsChoose::GetNumChoice(void)
{
	return (int)choice.GetN();
}

void FsChoose::DrawChosenAirplane(FsWorld *world)
{
#ifdef WIN32
	int stillTime;
	stillTime=YsAbs(clock()-lastCursorMoveClock);
	if(lastCursorMoveClock==0 || stillTime*3>=CLOCKS_PER_SEC)
#else
	int stillTime;
	struct timeval tm;
	gettimeofday(&tm,NULL);
	stillTime=tm.tv_sec*1000+tm.tv_usec/1000-lastCursorMoveClock;
	if(lastCursorMoveClock==0 || stillTime>300)
#endif
	{
		const char *airName;
		const FsAirplaneTemplate *tmpl;
		FsVisualDnm vis;

		if((airName=GetChoice())!=nullptr &&
		   (tmpl=world->GetAirplaneTemplate(airName))!=nullptr &&
		   (vis=world->GetAirplaneVisual(airName))!=nullptr)
		{
			int i;
			FsAirplaneProperty prop;   // Be careful!  prop.belongTo will be NULL.
			YsVec3 pos;

			FsVisualDnm weaponShapeOverrideStatic[(int)FSWEAPON_NUMWEAPONTYPE];
			FsVisualDnm weaponShapeOverrideFlying[(int)FSWEAPON_NUMWEAPONTYPE];
			for(int i=0; i<(int)FSWEAPON_NUMWEAPONTYPE; i++)
			{
				weaponShapeOverrideStatic[i]=world->GetAirplaneWeaponShapeOverride(airName,(FSWEAPONTYPE)i,0);
				weaponShapeOverrideFlying[i]=world->GetAirplaneWeaponShapeOverride(airName,(FSWEAPONTYPE)i,1);
			}

			FsProjection prj;
			FsSimulation::GetStandardProjection(prj);
			prj.cx=prj.cx*4/3;
			prj.cy=prj.cy*5/4;
			FsSetSceneProjection(prj);

			for(i=0; i<20; i++)
			{
				vis.SetState(i,0,0,0.0);
			}

			prop=*tmpl->GetProperty();

			pos.Set(0.0,0.0,prop.GetOutsideRadius()*2.2);
			prop.SetPosition(pos);
			prop.SetAttitude(att);

			FsSetCameraPosition(YsOrigin(),YsZeroAtt(),YSTRUE);
			FsSetPointLight(YsOrigin(),YsOrigin(),FSDAYLIGHT);

			vis.Draw(pos,att);

			prop.ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);
			prop.DrawOrdinance(YsVisual::DRAWALL,NULL,weaponShapeOverrideStatic);

			FsFlushScene(); // BiFlushBuffer();
		}
	}
}

void FsChoose::DrawChosenField(FsWorld *world)
{
	if(scn!=NULL)
	{
	#ifdef WIN32
		int stillTime;
		stillTime=YsAbs(clock()-lastCursorMoveClock);
		if(lastCursorMoveClock==0 || stillTime*3>=CLOCKS_PER_SEC)
	#else
		int stillTime;
		struct timeval tm;
		gettimeofday(&tm,NULL);
		stillTime=tm.tv_sec*1000+tm.tv_usec/1000-lastCursorMoveClock;
		if(lastCursorMoveClock==0 || stillTime>300)
	#endif
		{
			const char *fldName;
			fldName=GetChoice();
			if(fldName!=NULL && strcmp(loadedField,fldName)!=0)
			{
				scn->Initialize();
				world->GetFieldVisual(*scn,fldName);
				loadedField.Set(fldName);
			}
		}

		int wid,hei;
		FsGetWindowSize(wid,hei);

		double l;
		YsVec3 bbx[2],satellite;
		YsAtt3 att;
		scn->GetBoundingBox(bbx);

		satellite=(bbx[0]+bbx[1])/2.0;
		l=(bbx[1]-bbx[0]).L0Norm();

		satellite.SetY(l*1.4);
		att.Set(0.0,-YsPi/2.0,0.0);


		YsMatrix4x4 viewMat;
		viewMat.RotateXY(-att.b());
		viewMat.RotateZY(-att.p());
		viewMat.RotateXZ(-att.h());
		viewMat.Translate(-satellite);


		FsSet2DDrawing();
		FsDrawRect(0,0,wid-1,hei-1,scn->GetGroundColor(),YSTRUE);

		FsProjection prj;
		FsSimulation::GetStandardProjection(prj);
		prj.nearz=l;
		prj.farz=l*1.5;
		prj.cx=prj.cx*4/3;
		FsSetSceneProjection(prj);

		FsSetCameraPosition(satellite,att,YSTRUE);
		FsSetDirectionalLight(satellite,YsYVec(),FSDAYLIGHT);

		scn->DrawMapVisual(viewMat,YsIdentity4x4(),prj.GetMatrix(),-YsInfinity,YsInfinity,YSTRUE,0.0);
		scn->DrawVisual(viewMat,YsIdentity4x4(),prj.GetMatrix(),0.0,YSFALSE); // forShadowMap=YSFALSE


		FsSet2DDrawing();
		FsDrawRect(0,0,wid/4,hei-1,YsGrayScale(0.25),YSTRUE);
	}
}

void FsChoose::Draw(int x1,int y1)
{
	int i,currentShowPosition;

	currentShowPosition=(currentCursorPosition/nShow)*nShow;

	if(FsIsConsoleServer()!=YSTRUE)
	{
		int x,y;
		YsColor col,darkGreen;
		YsString buf;

		darkGreen.SetIntRGB(0,192,0);

		FsSet2DDrawing();

		x=x1;
		y=y1;

		if(choosingFile==YSTRUE)
		{
			YsString ful;

			ful.Set("FILENAME:");
			ful.Append(curPath);
			ful.Append("/");
			ful.Append(curName);
			FsDrawString(x,y,ful,YsWhite());
			y+=20;
		}


		if(currentShowPosition>0)
		{
			FsDrawString(x,y,"<<<<PREVIOUS PAGE",(currentMouseSelectPosition!=0 ? YsYellow() : YsBlue()));
		}
		else
		{
			FsDrawString(x,y,"-----------------",YsYellow());
		}
		y+=16;

		for(i=0; i<nShow; i++)
		{
			if(currentShowPosition+i<choice.GetN())
			{
				if(currentShowPosition+i==currentCursorPosition)
				{
					char str[256];
					sprintf(str,">>>%-24s<<<",(const char *)choice[currentShowPosition+i]);

					FsDrawString(x-24,y,str,YsWhite());
					FsDrawString(x-23,y,str,YsWhite());
				}
				else if(choosingFile!=YSTRUE)
				{
					FsDrawString
					   (x,y,choice[currentShowPosition+i],(currentMouseSelectPosition-1!=i ? darkGreen : YsGreen()));
				}
				else
				{
					if(strncmp(choice[currentShowPosition+i],"*DIR",4)==0)
					{
						col.SetIntRGB(0,192,0);
					}
					else if(strncmp(choice[currentShowPosition+i],"*NEWFILE",8)==0)
					{
						col.SetIntRGB(255,0,255);
					}
					else
					{
						col.SetIntRGB(0,255,0);
					}
					FsDrawString(x,y,choice[currentShowPosition+i],col);
				}
			}
			y+=16;
		}

		if(currentShowPosition+nShow<choice.GetN())
		{
			FsDrawString(x,y,"NEXT PAGE>>>>",(currentMouseSelectPosition!=nShow+1 ? YsYellow() : YsBlue()));
		}
		y+=16;
		y+=16;
		y+=16;
		y+=16;

		if(choosingFile==YSTRUE)
		{
			FsDrawString(x,y,"BS KEY TO GO UP A DIRECTORY",YsWhite());
			y+=16;
		}
		FsDrawString(x,y,"UP AND DOWN KEYS TO MOVE CURSOR",YsWhite());
		y+=16;
		FsDrawString(x,y,"SHIFT+UP AND SHIFT+DOWN KEYS TO MOVE CURSOR FASTER",YsWhite());
		y+=16;
		FsDrawString(x,y,"PAGEUP AND PAGEDOWN KEYS TO MOVE A PAGE",YsWhite());
		y+=16;
		FsDrawString(x,y,"HOME AND END KEYS TO MOVE TO TOP/END",YsWhite());
		y+=16;
		FsDrawString(x,y,"ENTER KEY TO DECIDE",YsWhite());
		y+=32;

		FsDrawString(x,y,"Search (CTRL+F)",(keyWordSearchMode==YSTRUE ? YsWhite() : YsGreen()));
		y+=16;


		buf.Set(">");
		buf.Append(searchKeyWord);
		if(keyWordSearchMode==YSTRUE)
		{
			buf.Append(((int)time(NULL)&1) ? "_" : "|");
		}

		if(keyWordNoMatch==YSTRUE)
		{
			FsDrawString(x,y,buf,YsRed());
			y+=16;
			FsDrawString(x,y,"(No Match)",YsRed());
			y+=16;
		}
		else
		{
			FsDrawString(x,y,buf,(keyWordSearchMode==YSTRUE ? YsWhite() : YsGreen()));
			y+=16;
			y+=16;
		}



		okButtonX0=x;
		okButtonX1=x+64;
		okButtonY0=y-16;
		okButtonY1=y;
		if(mouseOnOkButton!=YSTRUE)
		{
			FsDrawString(x,y,"[ O K ]",darkGreen);
			FsDrawString(x+1,y,"[ O K ]",darkGreen);
		}
		else
		{
			FsDrawString(x,y,"[ O K ]",(prevLb==YSTRUE ? YsWhite() : YsGreen()));
			FsDrawString(x+1,y,"[ O K ]",(prevLb==YSTRUE ? YsWhite() : YsGreen()));
		}
		okButtonVisible=YSTRUE;



		if(showCancelButton==YSTRUE)
		{
			cancelButtonX0=x+64;
			cancelButtonX1=x+128;
			cancelButtonY0=y-16;
			cancelButtonY1=y;
			if(mouseOnCancelButton!=YSTRUE)
			{
				FsDrawString(x+64,y,"[CANCEL]",darkGreen);
				FsDrawString(x+65,y,"[CANCEL]",darkGreen);
			}
			else
			{
				FsDrawString(x+64,y,"[CANCEL]",(prevLb==YSTRUE ? YsWhite() : YsGreen()));
				FsDrawString(x+65,y,"[CANCEL]",(prevLb==YSTRUE ? YsWhite() : YsGreen()));
			}
		}


		// Starting new column  2001/06/24
		if(choosingAirplane==YSTRUE)
		{
			const char *airName;
			const FsAirplaneTemplate *tmpl;
			char str[256];

			x=x1+240;
			y=y1;

			if((airName=GetChoice())!=NULL && (tmpl=wld->GetAirplaneTemplate(airName))!=NULL)
			{
				const FsAirplaneProperty &prop=*tmpl->GetProperty();   // Be careful!  prop.belongTo will be NULL.

				sprintf(str,"AAM [%d] (Short Range) Q-key:Add  A-key:Remove",aam);
				FsDrawString
				   (x,y,str,((allowAam==YSTRUE && prop.GetMaxNumWeapon(FSWEAPON_AIM9)>0) ? YsGreen() : YsRed()));
				y+=16;

				sprintf(str,"AAM [%d] (Mid Range) W-key:Add  S-key:Remove",aim120);
				FsDrawString
				   (x,y,str,((allowAam==YSTRUE && prop.GetMaxNumWeapon(FSWEAPON_AIM120)>0) ? YsGreen() : YsRed()));
				y+=16;

				sprintf(str,"AGM [%d] E-key:Add  D-key:Remove",agm);
				FsDrawString
				   (x,y,str,((allowAgm==YSTRUE && prop.GetMaxNumWeapon(FSWEAPON_AGM65)>0) ? YsGreen() : YsRed()));
				y+=16;

				sprintf(str,"BOM [%d] (500lb) R-key:Add  F-key:Remove",bomb);
				FsDrawString
				   (x,y,str,((allowBomb==YSTRUE && prop.GetMaxNumWeapon(FSWEAPON_BOMB)>0)  ? YsGreen() : YsRed()));
				y+=16;

				sprintf(str,"BOM [%d] (250lb) T-key:Add  G-key:Remove",bomb250);
				FsDrawString
				   (x,y,str,((allowBomb==YSTRUE && prop.GetMaxNumWeapon(FSWEAPON_BOMB250)>0)? YsGreen() : YsRed()));
				y+=16;

				sprintf(str,"RKT [%d] Y-key:Add  H-key:Remove",rocket);
				FsDrawString
				   (x,y,str,((allowRocket==YSTRUE && prop.GetMaxNumWeapon(FSWEAPON_ROCKET)>0) ? YsGreen() : YsRed()));
				y+=16;

				sprintf(str,"SMK [%s] U-key:Load  J-key:Unload",(smk==YSTRUE ? "Loaded" : "Unloaded"));
				FsDrawString(x,y,str,YsGreen());
				y+=16;

				sprintf(str,"FUEL[%d%%] I-key:Add K-key:Reduce",fuel);
				FsDrawString(x,y,str,YsGreen());
				y+=16;

				FsDrawRect(x,y-11,x+31,y+2,smkCol,YSTRUE);
				if(smokeColorMode==YSTRUE)
				{
					int k;
					YsString buf;
					buf.Set("SMK COL [");
					buf.Append(smokeColorCode);
					buf.Append(((int)time(NULL)&1) ? "_" : "|");
					for(k=6; k>smokeColorCode.Strlen(); k--)
					{
						buf.Append(" ");
					}
					buf.Append("] (Ctrl+S)");
					FsDrawString(x+34,y,buf,YsWhite());
				}
				else
				{
					sprintf(str,"SMK COL [%02X%02X%02X] (Ctrl+S)",smkCol.Ri(),smkCol.Gi(),smkCol.Bi());
					FsDrawString(x+34,y,str,YsGreen());
				}
				y+=16;
			}

			y+=32;



			strcpy(str,"[ ] Show Normal Airplanes (F1-key)");
			str[1]=(showNormal==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==0 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show Utility Airplanes (F2-key)");
			str[1]=(showUtility==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==1 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show Aerobatic Airplanes (F3-key)");
			str[1]=(showAerobatic==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==2 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show Fighters (F4-key)");
			str[1]=(showFighter==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==3 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show Attackers (F5-key)");
			str[1]=(showAttacker==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==4 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show Trainers (F6-key)");
			str[1]=(showTrainer==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==5 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show Heavy Bombers (F7-key)");
			str[1]=(showHeavyBomber==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==6 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show WWII Fighters (F8-key)");
			str[1]=(showWW2Fighter==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==7 ? YsGreen() : darkGreen));
			y+=16;

			strcpy(str,"[ ] Show WWII Bombers (F9-key)");
			str[1]=(showWW2Bomber==YSTRUE ? 'X' : ' ');
			FsDrawString(x,y,str,(currentMouseButtonPosition==8 ? YsGreen() : darkGreen));
			y+=16;
		}
	}
	else
	{
		FsResetConsoleWindowCursorPosition();

		if(currentShowPosition>0)
		{
			printf("<<<<PREVIOUS PAGE\n");
		}
		else
		{
			printf("-----------------\n");
		}

		for(i=0; i<nShow; i++)
		{
			if(currentShowPosition+i<choice.GetN())
			{
				if(currentShowPosition+i==currentCursorPosition)
				{
					printf(">>>%-24s<<<\n",(const char *)choice[currentShowPosition+i]);
				}
				else
				{
					printf("   %-24s   \n",(const char *)choice[currentShowPosition+i]);
				}
			}
			else
			{
				printf("%30s\n"," ");
			}
		}

		if(currentShowPosition+nShow<choice.GetN())
		{
			printf("NEXT PAGE>>>>\n");
		}
		printf("\n");
		printf("\n");
		printf("\n");
		printf("\n");

		if(choosingFile==YSTRUE)
		{
			printf("BS KEY TO GO UP A DIRECTORY\n");
		}
		printf("UP AND DOWN KEYS TO MOVE CURSOR\n");
		printf("SHIFT+UP AND SHIFT+DOWN KEYS TO MOVE CURSOR FASTER\n");
		printf("PAGEUP AND PAGEDOWN KEYS TO MOVE A PAGE\n");
		printf("HOME AND END KEYS TO MOVE TO TOP/END\n");
		printf("ENTER KEY TO DECIDE\n");
		if(showCancelButton==YSTRUE)
		{
			printf("ESC KEY TO CANCEL\n");
		}
		printf("\n");
	}
}

YSRESULT FsChoose::KeyInInConsoleMode(YSBOOL &selected,YSBOOL &cancelled,int ky,int /*c*/)
{
	selected=YSFALSE;
	cancelled=YSFALSE;

	switch(ky)
	{
	case FSKEY_P:
		currentCursorPosition-=8;
		currentCursorPosition=YsBound <int> (currentCursorPosition,0,(int)choice.GetN()-1);
		PrintInConsoleMode();
		break;
	case FSKEY_N:
		currentCursorPosition+=8;
		currentCursorPosition=YsBound <int> (currentCursorPosition,0,(int)choice.GetN()-1);
		PrintInConsoleMode();
		break;
	case FSKEY_1:
	case FSKEY_2:
	case FSKEY_3:
	case FSKEY_4:
	case FSKEY_5:
	case FSKEY_6:
	case FSKEY_7:
	case FSKEY_8:
		{
			const int sel=currentCursorPosition+(ky-FSKEY_1);
			if(YSTRUE==choice.IsInRange(sel))
			{
				currentCursorPosition=sel;
				selected=YSTRUE;
			}
		}
		break;
	case FSKEY_ESC:
		cancelled=YSTRUE;
		break;
	case FSKEY_ENTER:
		PrintInConsoleMode();
		break;
	}
	return YSOK;
}

void FsChoose::PrintInConsoleMode(void)
{
	printf("\n\n");

	int i;
	for(i=0; i<8; i++)
	{
		if(currentCursorPosition+i<choice.GetN())
		{
			printf("[%d]...%-24s",i+1,(const char *)choice[currentCursorPosition+i]);
		}
	}
	printf("P.....Previous Page\n");
	printf("N.....Next Page\n");
	printf("ESC...Cancel\n");
}

void FsChoose::Clear()
{
	fullChoice.CleanUp();
	choice.Set(0,NULL);
	searchKeyWord.Set("");
	currentCursorPosition=0;
}

int FsChoose::AddChoice(const char name[])
{
	YsListItem <YsString> *ptr;
	ptr=fullChoice.Create();
	ptr->dat.Set(name);
	choice.Append(ptr->dat);
	return (int)choice.GetN();
}

void FsChoose::DeleteChoice(int id)
{
	if(0<=id && id<choice.GetN())
	{
		choice.Delete(id);
	}
}

const char *FsChoose::GetCurrentPath(void)
{
	return curPath;
}

const char *FsChoose::GetCurrentFileName(void)
{
	return curName;
}

YSBOOL FsChoose::GetOkButtonClicked(void)
{
	if(okButtonVisible==YSTRUE && okButtonClicked==YSTRUE)
	{
		okButtonClicked=YSFALSE;
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsChoose::GetCancelButtonClicked(void)
{
	if(showCancelButton==YSTRUE && cancelButtonClicked==YSTRUE)
	{
		cancelButtonClicked=YSFALSE;
		return YSTRUE;
	}
	return YSFALSE;
}



////////////////////////////////////////////////////////////


FsGuiChooseField::FsGuiChooseField()
{
// 	scn=new YsScenery;
// 	scn->Initialize();

	useStpListBox=YSTRUE;
	useFldListBox=YSTRUE;
	fldNameForStpSelector.Set("");

	Initialize();
}

FsGuiChooseField::~FsGuiChooseField()
{
// 	delete scn;
}

void FsGuiChooseField::Initialize(void)
{
	FsGuiDialog::Initialize();

	fldLbx=NULL;
	fldSearch=NULL;
	stpLbx=NULL;
	okBtn=NULL;
	cancelBtn=NULL;
	world=NULL;
	lastCursorMoveClock=0;
	stpLoaded=YSFALSE;
}

void FsGuiChooseField::ResetSelection(void)
{
	if(fldLbx!=NULL)
	{
		fldLbx->SelectByString(selFld);
	}
	if(stpLbx!=NULL)
	{
		stpLbx->SelectByString(selStp);
	}
}

void FsGuiChooseField::FieldSelectionChanged(void)
{
	ResetStartPosSelection();
	ReloadStartPosition();
	lastCursorMoveClock=FsGuiClock();
}

void FsGuiChooseField::OnListBoxSelChange(FsGuiListBox *lbx,int /*prevSel*/)
{
	if(lbx==fldLbx)
	{
		ResetStartPosSelection();
		ReloadStartPosition();
		lastCursorMoveClock=FsGuiClock();
	}
	if(lbx==stpLbx)
	{
		ReloadStartPosition();
	}
}

void FsGuiChooseField::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==fldSearch)
	{
		ResetFieldListBySearchKeyword(world,fldLbx,fldSearch,/*forRacingMode=*/YSFALSE);
	}
}

void FsGuiChooseField::Create(FsWorld *world)
{
	SetTopLeftCorner(0,0);
	SetTextMessage("Select Field");

	if(useFldListBox==YSTRUE)
	{
		const char *empty[]={NULL};
		fldLbx=AddListBox(MkId("fieldList"),FSKEY_NULL,"Field",0,empty,12,32,YSTRUE);
		fldSearch=AddTextBox(MkId("fieldSearch"),FSKEY_NULL,"Search","",32,YSTRUE);
	}
	if(useStpListBox==YSTRUE)
	{
		int nLine;
		nLine=(fldLbx!=NULL ? 6 : 18);
		const char *empty[]={NULL};
		stpLbx=AddListBox(MkId("startPosList"),FSKEY_NULL,"Start Pos",0,empty,nLine,32,YSTRUE);
	}
	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,"[Enter]OK",YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,"[ESC]Cancel",YSTRUE);

	this->world=world;

	int i;
	const char *str;
	YsArray <const char *> strList;

	for(i=0; (str=world->GetFieldTemplateName(i))!=NULL; i++)
	{
		strList.Append(str);
	}

	YsQuickSortString <int> (strList.GetN(),strList,NULL);

	if(fldLbx!=NULL)
	{
		fldLbx->SetChoice(strList.GetN(),strList);
		fldLbx->Select(0);
	}

	ResetStartPosSelection();

	if(fldLbx!=NULL)
	{
		SetFocus(fldLbx);
	}
	else if(stpLbx!=NULL)
	{
		SetFocus(stpLbx);
	}
	else
	{
		SetFocus(okBtn);
	}

	Fit();
}

void FsGuiChooseField::Select(const char fldName[],const char stpName[])
{
	if(fldLbx!=NULL)
	{
		fldLbx->SelectByString(fldName,YSTRUE);
	}
	else
	{
		fldNameForStpSelector.Set(fldName);
	}

	if(stpLbx!=NULL && stpName!=NULL)
	{
		ResetStartPosSelection();
		stpLbx->SelectByString(stpName,YSTRUE);
	}
}

void FsGuiChooseField::CaptureSelection(void)
{
	GetSelectedField(selFld);
	if(NULL!=stpLbx)
	{
		stpLbx->GetSelectedString(selStp);
	}
	else
	{
		selStp.Set("");
	}
}

void FsGuiChooseField::Show(const FsGuiDialog *excludeFromDrawing) const
{
	unsigned clk;
	clk=FsGuiClock();
	if(lastCursorMoveClock!=0 && (clk<lastCursorMoveClock || (clk-lastCursorMoveClock)>500))
	{
		FsGuiChooseField *nonConstPtr=(FsGuiChooseField *)this;
		nonConstPtr->ReloadField();
		nonConstPtr->lastCursorMoveClock=0;
	}

	DrawField(world,YSTRUE,YSFALSE,YSFALSE,YsOrigin());
	FsGuiDialog::Show(excludeFromDrawing);
}

void FsGuiChooseField::ResetStartPosSelection(void)
{
	YsString fldName;

	if(fldLbx!=NULL)
	{
		fldLbx->GetSelectedString(fldName);
	}
	else
	{
		fldName=fldNameForStpSelector;
	}

	if(stpLbx!=NULL)
	{
		YsString defStp;

		stpLbx->GetSelectedString(defStp);

		if(0<fldName.Strlen())
		{
			int i;
			YsString stp;
			YsArray <YsString> stpList;
			for(i=0; world->GetFieldStartPositionName(stp,fldName,i)==YSOK; i++)
			{
				stpList.Append(stp);
			}
			stpLbx->SetChoice(stpList.GetN(),stpList);
			stpLbx->SelectByString(defStp,YSTRUE);
		}
	}
}

void FsGuiChooseField::ReloadField(void)
{
	YsString fldName;
	if(fldLbx!=NULL)
	{
		fldLbx->GetSelectedString(fldName);
	}
	else
	{
		fldName=fldNameForStpSelector;
	}

	if(0<fldName.Strlen())
	{
		FsGuiDialogWithFieldAndAircraft::ReloadField(world,fldName);
		// scn->Initialize();
		// world->GetFieldVisual(*scn,fldName);
	}
}

void FsGuiChooseField::ReloadStartPosition(void)
{
	YsString fldName;

	if(fldLbx!=NULL)
	{
		fldLbx->GetSelectedString(fldName);
	}
	else
	{
		fldName=fldNameForStpSelector;
	}

	if(0<fldName.Strlen() && stpLbx!=NULL)
	{
		YsString stpName;

		if(YSOK==stpLbx->GetSelectedString(stpName))
		{
			FsGuiDialogWithFieldAndAircraft::ReloadStartPos(world,fldName,stpName);
			// FsStartPosInfo stpInfo;
			// stpLoaded=YSFALSE;
			// if(world->GetStartPositionInfo(stpInfo,fldName,stpName)==YSOK)
			// {
			// 	if(stpInfo.onCarrier!=YSTRUE)
			// 	{
			// 		if(stpInfo.InterpretPosition(stpPos,stpAtt)==YSOK)
			// 		{
			// 			stpLoaded=YSTRUE;
			// 		}
			// 	}
			// }
		}
	}
}

const char *FsGuiChooseField::GetSelectedField(YsString &fldStr) const
{
	if(fldLbx!=NULL)
	{
		fldLbx->GetSelectedString(fldStr);
	}
	else
	{
		fldStr=fldNameForStpSelector;
	}
	return fldStr;
}

const char *FsGuiChooseField::GetSelectedStartPos(YsString &stpStr) const
{
	stpLbx->GetSelectedString(stpStr);
	return stpStr;
}

// void FsGuiChooseField::DrawSelectedField(void)
// {
// 	if(scn!=NULL)
// 	{
// 		unsigned clk;
// 		clk=FsGuiClock();
// 		if(lastCursorMoveClock!=0 && (clk<lastCursorMoveClock || (clk-lastCursorMoveClock)>500))
// 		{
// 			ReloadField();
// 			lastCursorMoveClock=0;
// 		}
// 
// 		int wid,hei;
// 		FsGetWindowSize(wid,hei);
// 
// 		double l;
// 		YsVec3 bbx[2],satellite;
// 		YsAtt3 att;
// 		scn->GetBoundingBox(bbx);
// 
// 		satellite=(bbx[0]+bbx[1])/2.0;
// 		l=(bbx[1]-bbx[0]).L0Norm();
// 
// 		satellite.SetY(l*1.4);
// 		att.Set(0.0,-YsPi/2.0,0.0);
// 
// 
// 		YsMatrix4x4 viewMat;
// 		viewMat.RotateXY(-att.b());
// 		viewMat.RotateZY(-att.p());
// 		viewMat.RotateXZ(-att.h());
// 		viewMat.Translate(-satellite);
// 
// 
// 		FsSet2DDrawing();
// 		FsDrawRect(0,0,wid-1,hei-1,scn->GetGroundColor(),YSTRUE);
// 
// 		FsProjection prj;
// 		FsSimulation::GetStandardProjection(prj);
// 		prj.nearz=l;
// 		prj.farz=l*1.5;
// 		prj.cx=prj.cx*4/3;
// 		FsSetSceneProjection(prj);
// 
// 		FsSetCameraPosition(satellite,att,YSTRUE);
// 		FsSetDirectionalLight(satellite,YsYVec(),FSDAYLIGHT);
// 
// 		scn->DrawMapVisual(viewMat,YsIdentity4x4(),-YsInfinity,YsInfinity,YSTRUE,0.0,prj.nearz,prj.farz,prj.tanFov);
// 		scn->DrawVisual(viewMat,YsIdentity4x4(),0.0,prj.nearz,prj.farz,prj.tanFov);
// 
// 		if(stpLoaded==YSTRUE)
// 		{
// 			YsVec3 p1,p2;
// 			p1=stpPos;
// 			p2=stpPos;
// 			p1.SubX(20000.0);
// 			p2.AddX(20000.0);
// 			FsDrawLine(p1,p2,YsWhite());
// 
// 			p1=stpPos;
// 			p2=stpPos;
// 			p1.SubZ(20000.0);
// 			p2.AddZ(20000.0);
// 			FsDrawLine(p1,p2,YsWhite());
// 		}
// 
// 		FsFlushScene();
// 
// 		FsSet2DDrawing();
// 	}
// }

void FsGuiChooseField::OnButtonClick(FsGuiButton *btn)
{
	if(okBtn==btn)
	{
		CaptureSelection();
		res=YSOK;
		CloseModalDialog((int)YSOK);
	}
	else if(cancelBtn==btn)
	{
		res=YSERR;
		CloseModalDialog((int)YSERR);
	}
}

////////////////////////////////////////////////////////////////////////////////

FsGuiChooseAircraftOption::FsGuiChooseAircraftOption()
{
	canSelectLoading=YSTRUE;
}


////////////////////////////////////////////////////////////


FsGuiChooseAircraft::FsGuiChooseAircraft()
{
	Initialize();
}

void FsGuiChooseAircraft::Initialize(void)
{
	FsGuiDialog::Initialize();

	filterEnabled=YSFALSE;

	createSearch=YSTRUE;
	selectListBoxRow=10;
	showAirplane=YSTRUE;

	selAir.Set("");
	selWeaponConfig.Set(0,NULL);
	selFuel=100;

	mainTab=NULL;

	airLbx=NULL;
	searchTxt=NULL;
	okBtn=NULL;
	cancelBtn=NULL;

	aamSrNbx=NULL;
	aim9xNbx=NULL;
	aamMrNbx=NULL;
	agmNbx=NULL;
	bom500Nbx=NULL;
	bom250Nbx=NULL;
	bom500HdNbx=NULL;
	rktNbx=NULL;
	fuelNbx=NULL;
	smkBtn=NULL;

	catNormal=NULL;
	catUtility=NULL;
	catAerobatic=NULL;
	catFighter=NULL;
	catAttacker=NULL;
	catTrainer=NULL;
	catBomber=NULL;
	catWw2Fighter=NULL;
	catWw2Attacker=NULL;
	catWw2Bomber=NULL;
	catWw2DiveBomber=NULL;


	att.Set(YsDegToRad(170.0),YsDegToRad(7.0),0.0);

	world=NULL;

	allowAam=YSTRUE;
	allowAgm=YSTRUE;
	allowBomb=YSTRUE;
	allowRocket=YSTRUE;
}

void FsGuiChooseAircraft::OnButtonClick(FsGuiButton *btn)
{
	if(btn==catNormal ||
	   btn==catUtility ||
	   btn==catAerobatic ||
	   btn==catFighter ||
	   btn==catAttacker ||
	   btn==catTrainer ||
	   btn==catBomber ||
	   btn==catWw2Fighter ||
	   btn==catWw2Attacker ||
	   btn==catWw2Bomber ||
	   btn==catWw2DiveBomber)
	{
		ResetAircraftList();
	}
	else if(NULL!=okBtn && btn==okBtn)
	{
		CaptureSelection();
		CloseModalDialog(nextActionCode);
	}
	else if(NULL!=cancelBtn && btn==cancelBtn)
	{
		CloseModalDialog(0);
	}
	else if(btn==sameSmkColBtn && YSTRUE==sameSmkColBtn->GetCheck() && NULL!=smkCpl[0])
	{
		auto col=smkCpl[0]->GetColor();
		for(auto smk : smkCpl)
		{
			smk->SetColor(col);
		}
	}

}

void FsGuiChooseAircraft::OnTextBoxChange(FsGuiTextBox *txt)
{
	if(txt==searchTxt)
	{
		ResetAircraftList();
	}
}

void FsGuiChooseAircraft::OnListBoxSelChange(FsGuiListBox *lbx,int /*prevSel*/)
{
	if(lbx==airLbx)
	{
		ResetOrdinance();
	}
}

void FsGuiChooseAircraft::OnMouseMove(
    YSBOOL lb,YSBOOL /*mb*/,YSBOOL /*rb*/,int mx,int my,
    YSBOOL /*plb*/,YSBOOL /*pmb*/,YSBOOL /*prb*/,int pmx,int pmy,
    FsGuiDialogItem *mouseOver)
{
	if(lb==YSTRUE && mouseOver==NULL)
	{
		double h,p;
		YsVec3 ev,uv;

		h=(double)(mx-pmx)*YsPi/100.0;
		p=(double)(my-pmy)*YsPi/100.0;

		ev=att.GetForwardVector();
		uv=att.GetUpVector();

		ev.RotateXZ(h);
		ev.RotateZY(p);

		uv.RotateXZ(h);
		uv.RotateZY(p);

		att.SetTwoVector(ev,uv);

		SetNeedRedraw(YSTRUE);
	}
}

/* virtual */ void FsGuiChooseAircraft::OnColorPaletteChange(FsGuiColorPalette *plt)
{
	if(NULL!=smkBtn)
	{
		smkBtn->SetCheck(YSTRUE);
	}
	if(YSTRUE==sameSmkColBtn->GetCheck())
	{
		auto col=plt->GetColor();
		for(auto smk : smkCpl)
		{
			smk->SetColor(col);
		}
	}
}


void FsGuiChooseAircraft::OnNumberBoxChange(FsGuiNumberBox *nbx,int /*prevNum*/)
{
	if(aamSrNbx==nbx)
	{
		ChangeLoading(FSWEAPON_AIM9,aamSrNbx->GetNumber());
	}
	if(aim9xNbx==nbx)
	{
		ChangeLoading(FSWEAPON_AIM9X,aim9xNbx->GetNumber());
	}
	if(aamMrNbx==nbx)
	{
		ChangeLoading(FSWEAPON_AIM120,aamMrNbx->GetNumber());
	}
	if(agmNbx==nbx)
	{
		ChangeLoading(FSWEAPON_AGM65,agmNbx->GetNumber());
	}
	if(bom500Nbx==nbx)
	{
		ChangeLoading(FSWEAPON_BOMB,bom500Nbx->GetNumber());
	}
	if(bom250Nbx==nbx)
	{
		ChangeLoading(FSWEAPON_BOMB250,bom250Nbx->GetNumber());
	}
	if(bom500HdNbx==nbx)
	{
		ChangeLoading(FSWEAPON_BOMB500HD,bom500HdNbx->GetNumber());
	}
	if(rktNbx==nbx)
	{
		ChangeLoading(FSWEAPON_ROCKET,rktNbx->GetNumber());
	}
	if(flarePodNbx==nbx)
	{
		ChangeLoading(FSWEAPON_FLARE,flarePodNbx->GetNumber());
	}
	if(fuelTankNbx==nbx)
	{
		ChangeLoading(FSWEAPON_FUELTANK,fuelTankNbx->GetNumber());
	}
}

YSRESULT FsGuiChooseAircraft::Create(FsWorld *world,const FsGuiChooseAircraftOption &option,int nextActionCode)
{
	SetTopLeftCorner(0,0);

	mainTab=AddTabControl(0,FSKEY_NULL,YSTRUE);

	this->nextActionCode=nextActionCode;

	aircraftTabId=AddTab(mainTab,FSGUI_AIRDLG_AIRCRAFTTAB);
	BeginAddTabItem(mainTab,aircraftTabId);

	const char *empty[]={NULL};
	airLbx=AddListBox(MkId("airList"),FSKEY_NULL,FSGUI_AIRDLG_AIRCRAFT,0,empty,selectListBoxRow,48,YSTRUE);

	if(createSearch==YSTRUE)
	{
		searchTxt=AddTextBox(2,FSKEY_NULL,FSGUI_AIRDLG_SEARCH,"",48,YSTRUE);
	}

	EndAddTabItem();



	loadingTabId=AddTab(mainTab,FSGUI_AIRDLG_LOADINGTAB);
	BeginAddTabItem(mainTab,loadingTabId);

	aamSrNbx=AddNumberBox(5,FSKEY_NULL,FSGUI_AIRDLG_AAMSHORT,18,0,0,100,1,YSTRUE);
	aim9xNbx=AddNumberBox(5,FSKEY_NULL,FSGUI_AIRDLG_AAAMSHORT,18,0,0,100,1,YSFALSE);

	aamMrNbx=AddNumberBox(6,FSKEY_NULL,FSGUI_AIRDLG_AAMMID,18,0,0,100,1,YSTRUE);
	agmNbx=AddNumberBox(7,FSKEY_NULL,FSGUI_AIRDLG_AGM,18,0,0,100,1,YSFALSE);

	rktNbx=AddNumberBox(10,FSKEY_NULL,FSGUI_AIRDLG_ROCKET,18,0,0,100,1,YSTRUE);
	flarePodNbx=AddNumberBox(0,FSKEY_NULL,FSGUI_AIRDLG_FLAREPOD,18,0,0,100,1,YSFALSE);

	bom500Nbx=AddNumberBox(8,FSKEY_NULL,FSGUI_AIRDLG_BOMB500,18,0,0,100,1,YSTRUE);
	bom250Nbx=AddNumberBox(9,FSKEY_NULL,FSGUI_AIRDLG_BOMB250,18,0,0,100,1,YSFALSE);

	bom500HdNbx=AddNumberBox(0,FSKEY_NULL,FSGUI_AIRDLG_BOM500HD,18,0,0,100,1,YSTRUE);
	fuelTankNbx=AddNumberBox(0,FSKEY_NULL,FSGUI_AIRDLG_FUELTANK,18,0,0,100,1,YSFALSE);
	
	fuelNbx=AddNumberBox(11,FSKEY_NULL,FSGUI_AIRDLG_FUEL,18,75,0,100,5,YSTRUE);
	smkBtn=AddTextButton(12,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_SMOKE,YSTRUE);
	sameSmkColBtn=AddTextButton(0,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_SMOKE_SAMECOLOR,YSFALSE);

	for(int smkIdx=0; smkIdx<FsAirplaneProperty::MaxNumSmokeGenerator; ++smkIdx)
	{
		YsString label;
		label.Printf("Smoke %d",smkIdx+1);
		smkCpl[smkIdx]=AddCompactColorPalette(13,FSKEY_NULL,label,255,255,255,(0==(smkIdx%4) ? YSTRUE : YSFALSE));
	}

	EndAddTabItem();

	if(YSTRUE!=option.canSelectLoading)
	{
		mainTab->DisableTab(loadingTabId);
	}



	categoryTabId=AddTab(mainTab,FSGUI_AIRDLG_CATEGORYTAB);
	BeginAddTabItem(mainTab,categoryTabId);

	catNormal=       AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_NORMAL,       YSTRUE);
	catUtility=      AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_UTILITY,      YSFALSE);
	catAerobatic=    AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_AEROBATIC,    YSFALSE);
	catFighter=      AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_FIGHTER,      YSTRUE);
	catAttacker=     AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_ATTACKER,     YSFALSE);
	catTrainer=      AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_TRAINER,      YSFALSE);
	catBomber=       AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_BOMBER,       YSFALSE);
	catWw2Fighter=   AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_WW2FIGHTER,   YSTRUE);
	catWw2Attacker=  AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_WW2ATTACKER,  YSFALSE);
	catWw2Bomber=    AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_WW2BOMBER,    YSTRUE);
	catWw2DiveBomber=AddTextButton(14,FSKEY_NULL,FSGUI_CHECKBOX,FSGUI_AIRDLG_WW2DIVEBOMBER,YSFALSE);

	EndAddTabItem();

	mainTab->SelectFirstTab();


	okBtn=AddTextButton(MkId("ok"),FSKEY_ENTER,FSGUI_PUSHBUTTON,FSGUI_COMMON_OK,YSTRUE);
	cancelBtn=AddTextButton(MkId("cancel"),FSKEY_ESC,FSGUI_PUSHBUTTON,FSGUI_COMMON_CANCEL,YSFALSE);



	catNormal->SetCheck(YSTRUE);
	catUtility->SetCheck(YSTRUE);
	catAerobatic->SetCheck(YSTRUE);
	catFighter->SetCheck(YSTRUE);
	catAttacker->SetCheck(YSTRUE);
	catTrainer->SetCheck(YSTRUE);
	catBomber->SetCheck(YSTRUE);
	catWw2Fighter->SetCheck(YSTRUE);
	catWw2Attacker->SetCheck(YSTRUE);
	catWw2Bomber->SetCheck(YSTRUE);
	catWw2DiveBomber->SetCheck(YSTRUE);



	this->world=world;

	ResetAircraftList();

	SetFocus(airLbx);

	ExpandTab(mainTab);
	Fit();
	SetBackgroundAlpha(0.9);

	return YSOK;
}

void FsGuiChooseAircraft::EnableSelectListBox(void)
{
	if(airLbx!=NULL)
	{
		airLbx->Enable();
	}
	if(searchTxt!=NULL)
	{
		searchTxt->Enable();
	}
}

void FsGuiChooseAircraft::DisableSelectListBox(void)
{
	if(airLbx!=NULL)
	{
		airLbx->Disable();
	}
	if(searchTxt!=NULL)
	{
		searchTxt->Disable();
	}
}

void FsGuiChooseAircraft::DisableCategoryButton(void)
{
	if(NULL!=catNormal)
	{
		catNormal->Disable();
	}
	if(NULL!=catUtility)
	{
		catUtility->Disable();
	}
	if(NULL!=catAerobatic)
	{
		catAerobatic->Disable();
	}
	if(NULL!=catFighter)
	{
		catFighter->Disable();
	}
	if(NULL!=catAttacker)
	{
		catAttacker->Disable();
	}
	if(NULL!=catTrainer)
	{
		catTrainer->Disable();
	}
	if(NULL!=catBomber)
	{
		catBomber->Disable();
	}
	if(NULL!=catWw2Fighter)
	{
		catWw2Fighter->Disable();
	}
	if(NULL!=catWw2Attacker)
	{
		catWw2Attacker->Disable();
	}
	if(NULL!=catWw2Bomber)
	{
		catWw2Bomber->Disable();
	}
	if(NULL!=catWw2DiveBomber)
	{
		catWw2DiveBomber->Disable();
	}
}

void FsGuiChooseAircraft::EnableAamButton(void)
{
	if(aamSrNbx!=NULL)
	{
		aamSrNbx->Enable();
	}
	if(aamMrNbx!=NULL)
	{
		aamMrNbx->Enable();
	}
	if(NULL!=aim9xNbx)
	{
		aim9xNbx->Enable();
	}
}

void FsGuiChooseAircraft::DisableAamButton(void)
{
	if(aamSrNbx!=NULL)
	{
		aamSrNbx->Disable();
	}
	if(aamMrNbx!=NULL)
	{
		aamMrNbx->Disable();
	}
	if(NULL!=aim9xNbx)
	{
		aim9xNbx->Disable();
	}
}


void FsGuiChooseAircraft::EnableAgmButton(void)
{
	if(agmNbx!=NULL)
	{
		agmNbx->Enable();
	}
}

void FsGuiChooseAircraft::DisableAgmButton(void)
{
	if(agmNbx!=NULL)
	{
		agmNbx->Disable();
	}
}

void FsGuiChooseAircraft::EnableBombButton(void)
{
	if(bom500Nbx!=NULL)
	{
		bom500Nbx->Enable();
	}
	if(bom250Nbx!=NULL)
	{
		bom250Nbx->Enable();
	}
	if(NULL!=bom500HdNbx)
	{
		bom500HdNbx->Enable();
	}
}

void FsGuiChooseAircraft::DisableBombButton(void)
{
	if(bom500Nbx!=NULL)
	{
		bom500Nbx->Disable();
	}
	if(bom250Nbx!=NULL)
	{
		bom250Nbx->Disable();
	}
	if(NULL!=bom500HdNbx)
	{
		bom500HdNbx->Disable();
	}
}

void FsGuiChooseAircraft::EnableRocketButton(void)
{
	if(rktNbx!=NULL)
	{
		rktNbx->Enable();
	}
}

void FsGuiChooseAircraft::DisableRocketButton(void)
{
	if(rktNbx!=NULL)
	{
		rktNbx->Disable();
	}
}

void FsGuiChooseAircraft::EnableFuelButton(void)
{
	if(fuelNbx!=NULL)
	{
		fuelNbx->Enable();
	}
}

void FsGuiChooseAircraft::DisableFuelButton(void)
{
	if(fuelNbx!=NULL)
	{
		fuelNbx->Disable();
	}
}


void FsGuiChooseAircraft::Show(const FsGuiDialog *excludeFromDrawing) const
{
 	if(showAirplane==YSTRUE && (airLbx->GetLastChangeClock()==0 || airLbx->GetLastChangeClock()+1000<FsGuiClock()))
 	{
		YsString airSel;
		airLbx->GetSelectedString(airSel);
		DrawAirplane(world,airSel,att,tmpWeaponConfig.GetN(),tmpWeaponConfig,YSFALSE);
	}
	FsSet2DDrawing();
	FsGuiDialog::Show(excludeFromDrawing);
}

void FsGuiChooseAircraft::ResetAircraftList(void)
{
	int i;
	const char *name;
	YsArray <const char *> airLst;
	YsString curSel;
	YsString searchTxt,cap;
	YsArray <YsString,16> searchKeyWord;



	if(airLbx!=NULL)
	{
		airLbx->GetSelectedString(curSel);
	}

	if(this->searchTxt!=NULL)
	{
		YsString str;
		this->searchTxt->GetText(str);
		if(0<str.Strlen())
		{
			searchTxt.Set(str);
			searchTxt.Capitalize();
			searchTxt.Arguments(searchKeyWord);
		}
	}

	for(i=0; (name=world->GetAirplaneTemplateName(i))!=NULL; i++)
	{
		if(filterEnabled==YSTRUE && filter.GetId(name)<0)
		{
			continue;
		}

		if(catNormal->GetCheck()==YSTRUE ||
		   catUtility->GetCheck()==YSTRUE ||
		   catAerobatic->GetCheck()==YSTRUE ||
		   catFighter->GetCheck()==YSTRUE ||
		   catAttacker->GetCheck()==YSTRUE ||
		   catTrainer->GetCheck()==YSTRUE ||
		   catBomber->GetCheck()==YSTRUE ||
		   catWw2Fighter->GetCheck()==YSTRUE ||
		   catWw2Attacker->GetCheck()==YSTRUE ||
		   catWw2Bomber->GetCheck()==YSTRUE ||
		   catWw2DiveBomber->GetCheck()==YSTRUE)  // If all buttons are off, just show everything.
		{
			if((world->GetAirplaneTemplateCategory(i)==FSAC_NORMAL        && catNormal->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_UTILITY       && catUtility->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_AEROBATIC     && catAerobatic->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_FIGHTER       && catFighter->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_ATTACKER      && catAttacker->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_TRAINER       && catTrainer->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_HEAVYBOMBER   && catBomber->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_WW2FIGHTER    && catWw2Fighter->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_WW2BOMBER     && catWw2Bomber->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_WW2ATTACKER   && catWw2Attacker->GetCheck()!=YSTRUE) ||
			   (world->GetAirplaneTemplateCategory(i)==FSAC_WW2DIVEBOMBER && catWw2DiveBomber->GetCheck()!=YSTRUE))
			{
				continue;
			}
		}

		if(searchKeyWord.GetN()>0)
		{
			cap.Set(name);
			cap.Capitalize();
			if(FsTestKeyWordMatch(cap,searchKeyWord.GetN(),searchKeyWord)!=YSTRUE)
			{
				continue;
			}
		}

		airLst.Append(name);
	}

	if(airLst.GetN()>0)
	{
		YsString newAirSel;

		airLbx->SetChoice(airLst.GetN(),airLst);
		airLbx->SelectByString(curSel,YSTRUE);
		if(YSOK!=(airLbx->GetSelectedString(newAirSel)) || strcmp(newAirSel,curSel)!=0)
		{
			ResetOrdinance();
		}
	}
}

YSBOOL FsGuiChooseAircraft::SmokeLoaded(void) const
{
	for(int i=0; i<=selWeaponConfig.GetN()-2; i+=2)
	{
		if(selWeaponConfig[i]==FSWEAPON_SMOKE ||
		   ((int)FSWEAPON_SMOKE0<=selWeaponConfig[i] && selWeaponConfig[i]<=(int)FSWEAPON_SMOKE7))
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

const YsColor FsGuiChooseAircraft::SmokeColor(int smkIdx) const
{
	YsColor smkCol=YsWhite();

	int smokeMask=0xffff;
	int smokeBit=(1<<smkIdx);
	for(int i=0; i<=selWeaponConfig.GetN()-2; i+=2)
	{
		if(selWeaponConfig[i]==FSWEAPON_SMOKE)
		{
			const int r=(selWeaponConfig[i+1]&0xff0000)>>16;
			const int g=(selWeaponConfig[i+1]&0x00ff00)>>8;
			const int b= selWeaponConfig[i+1]&0x0000ff;
			smkCol.SetIntRGB(r,g,b);
		}
		else if(FSWEAPON_SMOKE0<=selWeaponConfig[i] && selWeaponConfig[i]<=FSWEAPON_SMOKE7)
		{
			if(smkIdx==selWeaponConfig[i]-(int)FSWEAPON_SMOKE0)
			{
				const int r=(selWeaponConfig[i+1]&0xff0000)>>16;
				const int g=(selWeaponConfig[i+1]&0x00ff00)>>8;
				const int b= selWeaponConfig[i+1]&0x0000ff;
				smkCol.SetIntRGB(r,g,b);
			}
		}
	}
	return smkCol;
}

void FsGuiChooseAircraft::ChangeLoading(FSWEAPONTYPE wpnType,int n)
{
	const FsAirplaneTemplate *tmpl;

	YsString airSel;
	if(airLbx!=NULL &&
	   YSOK==(airLbx->GetSelectedString(airSel)) &&
	   (tmpl=world->GetAirplaneTemplate(airSel))!=NULL)
	{
		int addRemove;
		FsAirplaneProperty prop;
		prop=*tmpl->GetProperty();
		prop.ApplyWeaponConfig(tmpWeaponConfig.GetN(),tmpWeaponConfig);
		addRemove=n-prop.GetNumSlotWeapon(wpnType);

		if(wpnType!=FSWEAPON_GUN && addRemove!=0)
		{
			switch(wpnType)
			{
			case FSWEAPON_FUELTANK:
				if(0<addRemove)
				{
					for(int i=0; i<addRemove; i++)
					{
						tmpWeaponConfig.Insert(0,0x7fffffff); // (int)FsGetExternalFuelTankCapacity(wpnType));
						tmpWeaponConfig.Insert(0,wpnType);
					}
					prop.ApplyWeaponConfig(tmpWeaponConfig.GetN(),tmpWeaponConfig);
				}
				else
				{
					for(int i=(int)tmpWeaponConfig.GetN()-2; i>=0 && addRemove<0; i-=2)
					{
						if(tmpWeaponConfig[i]==FSWEAPON_FUELTANK)
						{
							tmpWeaponConfig[i]=FSWEAPON_GUN;
							tmpWeaponConfig[i+1]=0;
							addRemove++;
						}
					}
					prop.ApplyWeaponConfig(tmpWeaponConfig.GetN(),tmpWeaponConfig);
				}
				break;
			default:
				if(0<addRemove)
				{
					int nOrg;

					// prop.ShowSlotConfig();

					nOrg=prop.GetNumSlotWeapon(wpnType);
					prop.AddWeapon(wpnType,addRemove);

					if(prop.GetNumSlotWeapon(wpnType)==nOrg) // If no increase.
					{
						int i;
						for(i=0; i<=tmpWeaponConfig.GetN()-2; i+=2)
						{
							if(tmpWeaponConfig[i]==wpnType)
							{
								tmpWeaponConfig[i]=FSWEAPON_GUN;
								tmpWeaponConfig[i+1]=0;
							}
						}
						tmpWeaponConfig.Insert(0,n);
						tmpWeaponConfig.Insert(0,(int)wpnType);
						prop.ApplyWeaponConfig(tmpWeaponConfig.GetN(),tmpWeaponConfig);
					}
				}
				else
				{

					for(int i=(int)tmpWeaponConfig.GetN()-2; i>=0; i-=2)
					{
						if(tmpWeaponConfig[i]==wpnType)
						{
							addRemove+=tmpWeaponConfig[i+1];
							tmpWeaponConfig[i+1]=0;
							if(addRemove>=0)
							{
								tmpWeaponConfig[i+1]=addRemove;
								break;
							}
						}
					}
					prop.ApplyWeaponConfig(tmpWeaponConfig.GetN(),tmpWeaponConfig);
				}
				break;
			}

			prop.GetWeaponConfig(tmpWeaponConfig);
			ApplyWeaponLimit(tmpWeaponConfig);
			prop.ApplyWeaponConfig(tmpWeaponConfig.GetN(),tmpWeaponConfig);

			aamSrNbx->SetNumber(prop.GetNumWeapon(FSWEAPON_AIM9));
			aim9xNbx->SetNumber(prop.GetNumWeapon(FSWEAPON_AIM9X));
			aamMrNbx->SetNumber(prop.GetNumWeapon(FSWEAPON_AIM120));
			agmNbx->SetNumber(prop.GetNumWeapon(FSWEAPON_AGM65));
			bom500Nbx->SetNumber(prop.GetNumWeapon(FSWEAPON_BOMB));
			rktNbx->SetNumber(prop.GetNumWeapon(FSWEAPON_ROCKET));
			flarePodNbx->SetNumber(prop.GetNumSlotWeapon(FSWEAPON_FLARE));
			fuelTankNbx->SetNumber(prop.GetNumSlotWeapon(FSWEAPON_FUELTANK));
			bom250Nbx->SetNumber(prop.GetNumWeapon(FSWEAPON_BOMB250));
			bom500HdNbx->SetNumber(prop.GetNumWeapon(FSWEAPON_BOMB500HD));

			// prop.ShowSlotConfig();
		}
	}
}

void FsGuiChooseAircraft::ApplyWeaponLimit(YsArray <int,64> &weaponConfig)
{
	int i;
	i=0;
	while(i<weaponConfig.GetN()-1)
	{
		FSWEAPONTYPE wpnType;
		wpnType=(FSWEAPONTYPE)weaponConfig[i];
		if(allowAam!=YSTRUE && (wpnType==FSWEAPON_AIM9 || wpnType==FSWEAPON_AIM9X || wpnType==FSWEAPON_AIM120))
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else if(allowAgm!=YSTRUE && wpnType==FSWEAPON_AGM65)
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else if(allowBomb!=YSTRUE && (wpnType==FSWEAPON_BOMB || wpnType==FSWEAPON_BOMB250 || wpnType==FSWEAPON_BOMB500HD))
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else if(allowRocket!=YSTRUE && (wpnType==FSWEAPON_ROCKET))
		{
			weaponConfig.Delete(i);
			weaponConfig.Delete(i);
		}
		else
		{
			i+=2;
		}
	}
}

void FsGuiChooseAircraft::ResetOrdinance(void)
{
	const FsAirplaneTemplate *tmpl;
	YsString airSel;
	if(YSOK==airLbx->GetSelectedString(airSel) &&
	   (tmpl=world->GetAirplaneTemplate(airSel))!=NULL)
	{
		if(smkBtn!=NULL)
		{
			smkBtn->SetCheck(tmpl->GetProperty()->GetSmokeOil()>YsTolerance ? YSTRUE : YSFALSE);
		}
		for(int smkIdx=0; smkIdx<tmpl->GetProperty()->GetNumSmokeGenerator(); ++smkIdx)
		{
			if(NULL!=smkCpl[smkIdx])
			{
				smkCpl[smkIdx]->Enable();
				smkCpl[smkIdx]->SetColor(tmpl->GetProperty()->GetSmokeColor(smkIdx));
			}
		}
		for(int smkIdx=tmpl->GetProperty()->GetNumSmokeGenerator(); smkIdx<FsAirplaneProperty::MaxNumSmokeGenerator; ++smkIdx)
		{
			if(NULL!=smkCpl[smkIdx])
			{
				smkCpl[smkIdx]->Disable();
			}
		}

		tmpl->GetProperty()->GetWeaponConfig(tmpWeaponConfig);

		ApplyWeaponLimit(tmpWeaponConfig);

		FsAirplaneProperty prop;
		prop=*tmpl->GetProperty();
		prop.ApplyWeaponConfig(tmpWeaponConfig.GetN(),tmpWeaponConfig);
		prop.GetWeaponConfig(tmpWeaponConfig);

		SetOrdinanceByAirplaneProp(prop);
	}
}

void FsGuiChooseAircraft::SetOrdinance(YsArray <int,64> &weaponConfig)
{
	const FsAirplaneTemplate *tmpl;
	YsString airSel;
	if(YSOK==airLbx->GetSelectedString(airSel) &&
	   (tmpl=world->GetAirplaneTemplate(airSel))!=NULL)
	{
		ApplyWeaponLimit(weaponConfig);

		FsAirplaneProperty prop;
		prop=*tmpl->GetProperty();
		prop.ApplyWeaponConfig(weaponConfig.GetN(),weaponConfig);
		prop.GetWeaponConfig(weaponConfig);

		SetOrdinanceByAirplaneProp(prop);
	}
}

void FsGuiChooseAircraft::SetOrdinanceByAirplaneProp(const FsAirplaneProperty &prop)
{
	int num,max;
	if(aamSrNbx!=NULL)
	{
		num=prop.GetNumWeapon(FSWEAPON_AIM9);
		max=prop.GetMaxNumWeapon(FSWEAPON_AIM9);
		aamSrNbx->SetNumberAndRange(num,0,max,1);
	}

	if(aim9xNbx!=NULL)
	{
		num=prop.GetNumWeapon(FSWEAPON_AIM9X);
		max=prop.GetMaxNumWeapon(FSWEAPON_AIM9X);
		aim9xNbx->SetNumberAndRange(num,0,max,1);
	}

	if(aamMrNbx!=NULL)
	{
		num=prop.GetNumWeapon(FSWEAPON_AIM120);
		max=prop.GetMaxNumWeapon(FSWEAPON_AIM120);
		aamMrNbx->SetNumberAndRange(num,0,max,1);
	}

	if(agmNbx!=NULL)
	{
		num=prop.GetNumWeapon(FSWEAPON_AGM65);
		max=prop.GetMaxNumWeapon(FSWEAPON_AGM65);
		agmNbx->SetNumberAndRange(num,0,max,1);
	}

	if(bom500Nbx!=NULL)
	{
		num=prop.GetNumWeapon(FSWEAPON_BOMB);
		max=prop.GetMaxNumWeapon(FSWEAPON_BOMB);
		bom500Nbx->SetNumberAndRange(num,0,max,1);
	}

	if(bom250Nbx!=NULL)
	{
		num=prop.GetNumWeapon(FSWEAPON_BOMB250);
		max=prop.GetMaxNumWeapon(FSWEAPON_BOMB250);
		bom250Nbx->SetNumberAndRange(num,0,max,1);
	}

	if(NULL!=bom500HdNbx)
	{
		num=prop.GetNumWeapon(FSWEAPON_BOMB500HD);
		max=prop.GetMaxNumWeapon(FSWEAPON_BOMB500HD);
		bom500HdNbx->SetNumberAndRange(num,0,max,1);
	}

	if(rktNbx!=NULL)
	{
		num=prop.GetNumWeapon(FSWEAPON_ROCKET);
		max=prop.GetMaxNumWeapon(FSWEAPON_ROCKET);
		rktNbx->SetNumberAndRange(num,0,max,max);
	}

	if(NULL!=flarePodNbx)
	{
		num=prop.GetNumSlotWeapon(FSWEAPON_FLARE);
		max=prop.GetMaxNumSlotWeapon(FSWEAPON_FLARE);
		flarePodNbx->SetNumberAndRange(num,0,max,FsGetDefaultWeaponLoadingUnit(FSWEAPON_FLARE));
	}

	if(NULL!=fuelTankNbx)
	{
		num=prop.GetNumSlotWeapon(FSWEAPON_FUELTANK);
		max=prop.GetMaxNumSlotWeapon(FSWEAPON_FUELTANK);
		fuelTankNbx->SetNumberAndRange(num,0,max,FsGetDefaultWeaponLoadingUnit(FSWEAPON_FUELTANK));
	}

	if(smkBtn!=NULL)
	{
		smkBtn->SetCheck(prop.GetSmokeOil()>YsTolerance ? YSTRUE : YSFALSE);
	}

	for(int smkIdx=0; smkIdx<prop.GetNumSmokeGenerator(); ++smkIdx)
	{
		if(smkCpl[smkIdx]!=NULL)
		{
			smkCpl[smkIdx]->SetColor(prop.GetSmokeColor(smkIdx));
		}
	}
	for(int smkIdx=prop.GetNumSmokeGenerator(); smkIdx<FsAirplaneProperty::MaxNumSmokeGenerator; ++smkIdx)
	{
		smkCpl[smkIdx]->Disable();
	}


	if(allowAam!=YSTRUE)
	{
		if(aamSrNbx!=NULL)
		{
			aamSrNbx->SetNumberAndRange(0,0,0,0);
		}
		if(aamMrNbx!=NULL)
		{
			aamMrNbx->SetNumberAndRange(0,0,0,0);
		}
		if(NULL!=aim9xNbx)
		{
			aim9xNbx->SetNumberAndRange(0,0,0,0);
		}
	}
	if(allowAgm!=YSTRUE)
	{
		if(agmNbx!=NULL)
		{
			agmNbx->SetNumberAndRange(0,0,0,0);
		}
	}
	if(allowBomb!=YSTRUE)
	{
		if(bom500Nbx!=NULL)
		{
			bom500Nbx->SetNumberAndRange(0,0,0,0);
		}
		if(bom250Nbx!=NULL)
		{
			bom250Nbx->SetNumberAndRange(0,0,0,0);
		}
		if(NULL!=bom500HdNbx)
		{
			bom500HdNbx->SetNumberAndRange(0,0,0,0);
		}
	}
	if(allowRocket!=YSTRUE)
	{
		if(rktNbx!=NULL)
		{
			rktNbx->SetNumberAndRange(0,0,0,0);
		}
	}


	prop.GetWeaponConfig(tmpWeaponConfig);  // 2009/06/07


	// prop.ShowSlotConfig();
}

void FsGuiChooseAircraft::SetDefault(const char airName[])
{
	if(airLbx!=NULL)
	{
		airLbx->SelectByString(airName,YSTRUE);
		ResetOrdinance();
		CaptureSelection();
	}
}

void FsGuiChooseAircraft::ResetSelection(void)
{
	airLbx->SelectByString(selAir,YSTRUE);

	tmpWeaponConfig=selWeaponConfig;
	SetOrdinance(tmpWeaponConfig);

	if(smkBtn!=NULL)
	{
		smkBtn->SetCheck(SmokeLoaded());
	}
	for(int smkIdx=0; smkIdx<FsAirplaneProperty::MaxNumSmokeGenerator; ++smkIdx)
	{
		if(smkCpl[smkIdx]!=NULL)
		{
			smkCpl[smkIdx]->SetColor(SmokeColor(smkIdx));
		}
	}
}


void FsGuiChooseAircraft::CaptureSelection(void)
{
	YsString airSel;
	if(airLbx!=NULL && YSOK==airLbx->GetSelectedString(airSel))
	{
		selAir.Set(airSel);
		selWeaponConfig=tmpWeaponConfig;
		if(smkBtn!=NULL && smkBtn->GetCheck()==YSTRUE)
		{
			for(int smkIdx=0; smkIdx<FsAirplaneProperty::MaxNumSmokeGenerator; ++smkIdx)
			{
				if(NULL!=smkCpl[smkIdx] && YSTRUE==smkCpl[smkIdx]->IsEnabled())
				{
					selWeaponConfig.Append((int)FSWEAPON_SMOKE0+smkIdx);

					YsColor col;
					smkCpl[smkIdx]->GetColor(col);

					const int colCode=((col.Ri()&255)<<16)+((col.Gi()&255)<<8)+(col.Bi()&255);
					selWeaponConfig.Append(colCode);
				}
			}
		}
		selFuel=fuelNbx->GetNumber();
	}
}

