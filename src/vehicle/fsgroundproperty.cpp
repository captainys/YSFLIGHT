#include <ysclass.h>
#include <ysport.h>

#include <ysscenery.h>

#include <ysshelldnmident.h>

#include <fsdef.h>
#define FSSIMPLEWINDOW_MACRO_ONLY
#include <fssimplewindow.h>
#undef FSSIMPLEWINDOW_MACRO_ONLY

#include "platform/common/fswindow.h"
#include "fsutil.h"
#include "fsnetutil.h"
#include "fsnavaid.h"
#include "fsrecord.h"
#include "fsvisual.h"
#include "fsweapon.h"
#include "fsproperty.h"
#include "fsairplaneproperty.h"
#include "fsgroundproperty.h"
#include "fsnetwork.h"
#include "fsexistence.h"
#include "fsstdout.h"

#include "fsexplosion.h"
#include "fscontrol.h"
#include "fsweather.h"
#include "fssimulation.h"


////////////////////////////////////////////////////////////

void FsAircraftCarrierDeckPolygonCache::Initialize(void)
{
	plHd=NULL;
	pln.Set(YsOrigin(),YsYVec());
	plVtPos.ClearDeep();
	projPlVtPos.ClearDeep();
}

////////////////////////////////////////////////////////////

FsAircraftCarrierProperty::FsAircraftCarrierProperty()
{
	maneuver=0;
	maneuverTime=600.0;
	drawArrestingWire=YSFALSE;
	arrestingWirePos[0]=YsOrigin();
	arrestingWirePos[1]=YsOrigin();

	canBeViewpoint=YSFALSE;
	deckPos.Set(0.0,50.0,0.0);

	noAutoTaxi=YSFALSE;
}

FsAircraftCarrierProperty::~FsAircraftCarrierProperty()
{
}

YSRESULT FsAircraftCarrierProperty::LoadAircraftCarrier(const wchar_t tmplRootDir[],const wchar_t fn[])
{
	// Carrier data file
	//   1st line : Filename of the deck
	//   2nd line : Filename of the arrester
	//   3rd line : Filename of the catapult
	//   4th line : Designated landing point and ILS direction (hdg,pch,0.0) range

	YsFileIO::File fp(fn,"r");
	if(fp!=NULL)
	{
		char dckUtf8[256],arrUtf8[256],catUtf8[256],briUtf8[256];
		if(fgets(dckUtf8,256,fp)!=NULL &&
		   fgets(arrUtf8,256,fp)!=NULL &&
		   fgets(catUtf8,256,fp)!=NULL &&
		   fgets(briUtf8,256,fp)!=NULL)
		{
			YsStringTail(dckUtf8);
			YsStringTail(arrUtf8);
			YsStringTail(catUtf8);
			YsStringTail(briUtf8);

			YsWString dck,arr,cat,bri;
			dck.SetUTF8String(dckUtf8);
			arr.SetUTF8String(arrUtf8);
			cat.SetUTF8String(catUtf8);
			bri.SetUTF8String(briUtf8);

			FindAircraftCarrierPropertyFile(dck,tmplRootDir);
			FindAircraftCarrierPropertyFile(arr,tmplRootDir);
			FindAircraftCarrierPropertyFile(cat,tmplRootDir);
			FindAircraftCarrierPropertyFile(bri,tmplRootDir);

			YsFileIO::File dckFp(dck,"r");
			YsFileIO::File arrFp(arr,"r");
			YsFileIO::File catFp(cat,"r");

			YsShell shl;
			if(deck.LoadSrf(dckFp)==YSOK &&
			   PrepareDeckShell()==YSOK &&
			   shl.LoadSrf(arrFp)==YSOK &&
			   ConvertShellToArray(arrester,shl)==YSOK &&
			   shl.LoadSrf(catFp)==YSOK &&
			   ConvertShellToArray(catapult,shl)==YSOK)
			{
				bridge.Load(bri);

				YsBoundingBoxMaker3 bbx;
				for(YsShellVertexHandle vtHd=NULL; NULL!=(vtHd=deck.FindNextVertex(vtHd)); )
				{
					YsVec3 pos;
					deck.GetVertexPosition(pos,vtHd);
					bbx.Add(pos);
				}
				for(int i=0; i<arrester.GetN(); i++)
				{
					bbx.Add(arrester[i]);
				}
				for(int i=0; i<catapult.GetN(); i++)
				{
					bbx.Add(catapult[i]);  // 2009/06/02
				}
				bbx.Get(bbx1,bbx2);
				bbxDiagonal=(bbx1-bbx2).GetLength();

				ilsPosition.Set(0.0,0.0,0.0);
				ilsAngle.Set(0.0,0.0,0.0);
				ilsRange=0;

				catapultPos=YsOrigin();
				for(YSSIZE_T catIdx=0; catIdx<catapult.GetN(); ++catIdx)
				{
					catapultPos+=catapult[catIdx];
				}
				if(0<catapult.GetN())
				{
					catapultPos/=(double)catapult.GetN();
				}

				canBeViewpoint=YSTRUE;

				char cmd[256];
				while(fgets(cmd,256,fp)!=NULL)
				{
					int ac;
					char *av[16];

					if(YsArguments(&ac,av,16,cmd)==YSOK && ac>0)
					{
						if(ac==8 && strcmp(av[0],"ILS")==0)
						{
							if(FsGetVec3(ilsPosition,3,av+1)!=YSOK ||
							   FsGetAtt3(ilsAngle,3,av+4)!=YSOK ||
							   FsGetLength(ilsRange,av[7])!=YSOK)
							{
								// Now automatic: fclose(fp);
								return YSERR;
							}
						}
						else if(ac==2 && strcmp(av[0],"VIEWPOINT")==0)
						{
							if(FsGetBool(canBeViewpoint,av[1])!=YSOK)
							{
								// Now automatic: fclose(fp);
								return YSERR;
							}
						}
						else if(ac==7 && strcmp(av[0],"ARRESTINGWIRE")==0)
						{
							FsGetVec3(arrestingWirePos[0],3,av+1);
							FsGetVec3(arrestingWirePos[1],3,av+4);
							drawArrestingWire=YSTRUE;
						}
						else if(ac==4 && strcmp(av[0],"BRIDGEPOS")==0)
						{
							FsGetVec3(deckPos,3,av+1);
							canBeViewpoint=YSTRUE;
						}
						else if(0==strcmp(av[0],"NOAUTOTAXI"))
						{
							noAutoTaxi=YSTRUE;
						}
						else if(strcmp(av[0],"REM")==0)
						{
						}
						else
						{
							// Now automatic: fclose(fp);
							return YSERR;
						}
					}
				}
				// Now automatic: fclose(fp);
				return YSOK;
			}
		}
		else
		{
			// Now automatic: fclose(fp);
		}
	}

	return YSERR;
}

void FsAircraftCarrierProperty::FindAircraftCarrierPropertyFile(YsWString &fn,const wchar_t tmplRootDir[])
{
	YsWString ful;
	ful.MakeFullPathName(tmplRootDir,fn);

	FILE *fp=YsFileIO::Fopen(ful,"r");
	if(NULL!=fp)
	{
		fclose(fp);
		fn=ful;
	}

	// Otherwise, hope to get the file from current working directory.
}

YSRESULT FsAircraftCarrierProperty::LoadAirplane(FsAirplane *air)
{
	if(air->Prop().IsOnCarrier()==YSTRUE && air->Prop().OnThisCarrier()!=belongTo->belongTo)
	{
		FsGround *prevCarrier;
		prevCarrier=air->Prop().OnThisCarrier();
		prevCarrier->Prop().GetAircraftCarrierProperty()->UnloadAirplane(air);
	}


	int i;
	for(i=0; i<airList.GetN(); i++)
	{
		if(air==airList[i])
		{
			// Already loaded
			return YSERR;
		}
	}

	YsVec3 shipSpeed,airSpeed;

	belongTo->GetVelocity(shipSpeed);
	air->Prop().GetVelocity(airSpeed);

	if(airSpeed.GetSquareLength()<shipSpeed.GetSquareLength()*0.25)  // 2005/10/11 Prevent Su-27's sliding off
	{
		airSpeed=YsOrigin();
	}
	else
	{
		airSpeed-=shipSpeed;
	}

	air->Prop().AfterLoadedOnCarrier(belongTo->belongTo);

	air->Prop().SetVelocity(airSpeed);
	airList.AppendItem(air);

	return YSOK;
}

YSRESULT FsAircraftCarrierProperty::UnloadAirplane(FsAirplane *air)
{
	for(int i=0; i<airList.GetN(); i++)
	{
		if(air==airList[i])
		{
			YsVec3 shipSpeed,airSpeed;
			belongTo->GetVelocity(shipSpeed);
			air->Prop().GetVelocity(airSpeed);
			airSpeed+=shipSpeed;
			air->Prop().SetVelocity(airSpeed);
			airList.Delete(i);

			air->Prop().AfterUnloadedFromCarrier();

			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsAircraftCarrierProperty::LoadGround(FsGround *gnd)
{
	// Prevent recursion >>
	{
		const FsGround *prop=belongTo->belongTo;
		while(NULL!=prop)
		{
			if(prop==gnd)
			{
				return YSERR;
			}
			prop=prop->Prop().OnThisCarrier();
		}
	}
	// <<

	for(int i=0; i<gndList.GetN(); i++)
	{
		if(gnd==gndList[i])
		{
			YsPrintf("Error!  FsAircraftCarrierProperty::LoadGround()\n");
			YsPrintf("  [%s](%d) is already loaded.\n",gnd->GetIdentifier(),gnd->GetSearchKey(gnd));
			return YSERR;
		}
	}
	gndList.Append(gnd);
	gnd->Prop().AfterLoadedOnCarrier(belongTo->belongTo);
	return YSOK;
}

YSRESULT FsAircraftCarrierProperty::UnloadGround(FsGround *gnd)
{
	for(int i=0; i<gndList.GetN(); i++)
	{
		if(gnd==gndList[i])
		{
			gndList.DeleteBySwapping(i);
			gnd->Prop().AfterUnloadedFromCarrier();
			return YSOK;
		}
	}
	YsPrintf("Error!  FsAircraftCarrierProperty::UnloadGround()\n");
	YsPrintf("  [%s](%d) is not loaded.\n",gnd->GetIdentifier(),gnd->GetSearchKey(gnd));
	return YSERR;
}

YSBOOL FsAircraftCarrierProperty::IsAirplaneLoaded(const FsAirplane *air) const
{
	for(int i=0; i<airList.GetN(); i++)
	{
		if(air==airList[i])
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsAircraftCarrierProperty::IsGroundLoaded(const FsGround *gnd) const
{
	for(int i=0; i<gndList.GetN(); i++)
	{
		if(gnd==gndList[i])
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSBOOL FsAircraftCarrierProperty::IsOnDeck(const YsVec3 &pos) const
{
	YsVec3 tfmPos;
	belongTo->GetInverseMatrix().Mul(tfmPos,pos,1.0);;

	return IsOnDeckLocal(tfmPos);
}

YSBOOL FsAircraftCarrierProperty::IsOnDeckLocal(const YsVec3 &pos) const
{
	YsVec2 tfmPos2d(pos.x(),pos.z());

	for(int i=0; i<deckCache.GetN(); i++)
	{
		const YSSIDE side=YsCheckInsidePolygon2(tfmPos2d,deckCache[i].projPlVtPos.GetN(),deckCache[i].projPlVtPos);
		if(YSINSIDE==side || YSBOUNDARY==side)
		{
			return YSTRUE;
		}
	}

	return YSFALSE;
}

YSBOOL FsAircraftCarrierProperty::IsOnArrester(const YsVec3 &pos) const
{
	return IsOnSomething(arrester,pos);
}

YSBOOL FsAircraftCarrierProperty::IsOnCatapult(const YsVec3 &pos) const
{
	return IsOnSomething(catapult,pos);
}

YSBOOL FsAircraftCarrierProperty::LandedOnTheDeck
    (const YsVec3 &prv,const YsVec3 &now,
     const YsVec3 &gear1,const YsVec3 &gear2,const YsVec3 &gear3) const
{
	YsVec3 deckNom;
	double deckHeight;
	deckHeight=GetDeckHeightAndNormal(deckNom,now);

	// At least one gear must be touching the deck
	if(gear1.y()<=deckHeight+YsTolerance ||
	   gear2.y()<=deckHeight+YsTolerance ||
	   gear3.y()<=deckHeight+YsTolerance)
	{
		// Did it approach from the top of the deck?
		// (Previously higher than deck and Currently descending)
		if(prv.y()>=deckHeight+YsTolerance && now.y()<=prv.y())
		{
			// Now above the deck
			if(IsOnDeck(now)==YSTRUE)
			{
				return YSTRUE;
			}
		}
	}

	return YSFALSE;
}

double FsAircraftCarrierProperty::GetDeckHeightAndNormal(YsVec3 &nom,const YsVec3 &pos) const
{
	YsVec3 tfmPos;
	belongTo->GetInverseMatrix().Mul(tfmPos,pos,1.0);;
	tfmPos.SetY(-1.0);

	const YsVec2 tfmPos2d(tfmPos.x(),tfmPos.z());

	for(int i=0; i<deckCache.GetN(); i++)
	{
		const YSSIDE side=YsCheckInsidePolygon2(tfmPos2d,deckCache[i].projPlVtPos.GetN(),deckCache[i].projPlVtPos);
		if(YSINSIDE==side || YSBOUNDARY==side)
		{
			YsVec3 itsc;
			if(YSOK==deckCache[i].pln.GetIntersection(itsc,tfmPos,YsYVec()))
			{
				deck.GetNormalOfPolygon(nom,deckCache[i].plHd);

				belongTo->GetMatrix().Mul(nom,nom,0.0);
				belongTo->GetMatrix().Mul(itsc,itsc,1.0);

				return itsc.y()+YsTolerance;
			}
		}
	}

	nom=YsYVec();
	return 0.0;
}

const YsVec3 &FsAircraftCarrierProperty::GetBridgePos(void) const
{
	return deckPos;
}

const YsVec3 FsAircraftCarrierProperty::GetCatapultPos(void) const
{
	return catapultPos;
}

const YsVec3 FsAircraftCarrierProperty::GetCatapultVec(void) const
{
	return YsZVec();
}

YSRESULT FsAircraftCarrierProperty::MoveCargoAndILS(const YsMatrix4x4 &now,const YsMatrix4x4 &prv,const double &/*yaw*/)
{
	YsVec3 pos,ev,uv;
	YsAtt3 att;
	YsMatrix4x4 prvInverse(prv);
	prvInverse.Invert();

	for(int i=0; i<airList.GetNumItem(); i++)
	{
		if(airList[i]->isPlayingRecord!=YSTRUE)
		{
			pos=airList[i]->Prop().GetPosition();
			prvInverse.Mul(pos,pos,1.0);

			now.Mul(pos,pos,1.0);
			airList[i]->Prop().SetPosition(pos);

			att=airList[i]->Prop().GetAttitude();

			ev=att.GetForwardVector();
			uv=att.GetUpVector();
			prvInverse.Mul(ev,ev,0.0);
			prvInverse.Mul(uv,uv,0.0);
			now.Mul(ev,ev,0.0);
			now.Mul(uv,uv,0.0);
			att.SetTwoVector(ev,uv);

			airList[i]->Prop().SetAttitude(att);

			airList[i]->Prop().RemakeMatrix();                         // 2005/03/27
			airList[i]->SetTransformationToCollisionShell(airList[i]->Prop().GetMatrix());// 2005/03/27
		}
	}
	for(int i=0; i<gndList.GetN(); i++)
	{
		if(gndList[i]->isPlayingRecord!=YSTRUE)
		{
			gndList[i]->Prop().GetPosition(pos);
			prvInverse.Mul(pos,pos,1.0);
			now.Mul(pos,pos,1.0);

			att=gndList[i]->Prop().GetAttitude();

			ev=att.GetForwardVector();
			uv=att.GetUpVector();
			prvInverse.Mul(ev,ev,0.0);
			prvInverse.Mul(uv,uv,0.0);
			now.Mul(ev,ev,0.0);
			now.Mul(uv,uv,0.0);
			att.SetTwoVector(ev,uv);

			gndList[i]->Prop().SetPositionAndAttitude(pos,att);
		}
	}

	// Set ILS
	ev=ilsAngle.GetForwardVector();
	uv=ilsAngle.GetUpVector();
	now.Mul(pos,ilsPosition,1.0);
	now.Mul(ev,ev,0.0);
	now.Mul(uv,uv,0.0);
	att.SetTwoVector(ev,uv);
	ils.Set(pos,att,ilsRange);

	return YSOK;
}

YSRESULT FsAircraftCarrierProperty::Maneuver(const double &t)
{
	if(maneuver==0)
	{
		maneuverTime-=t;
		if(maneuverTime<=0.0)
		{
			designatedHeading=belongTo->GetAttitude().h()+YsPi;
			maneuver=1;
		}
	}
	else if(maneuver==1)
	{
		double ip;
		YsVec2 desig,current;
		desig.Set(-sin(designatedHeading),cos(designatedHeading));
		current.Set(-sin(belongTo->GetAttitude().h()),cos(belongTo->GetAttitude().h()));
		ip=desig*current;
		if(ip>=cos(t*belongTo->staRotation*1.5))
		{
			maneuverTime=600.0;
			maneuver=0;
		}
	}

	switch(maneuver)
	{
	case 0:   // Go Straight
		belongTo->staSpeed.Set(0.0,0.0,belongTo->chMaxSpeed);
		belongTo->staRotation=0.0;
		break;
	case 1:
		belongTo->staSpeed.Set(0.0,0.0,belongTo->chMaxSpeed);
		belongTo->staRotation=belongTo->chMaxRotation;
		break;
	}

	return YSOK;
}

YSBOOL FsAircraftCarrierProperty::HasCatapult(void) const
{
	return (3<=catapult.GetN() ? YSTRUE : YSFALSE);
}

YSBOOL FsAircraftCarrierProperty::CanBeViewpoint(void) const
{
	return canBeViewpoint;
}

YSBOOL FsAircraftCarrierProperty::NoAutoTaxi(void) const
{
	return noAutoTaxi;
}

YSRESULT FsAircraftCarrierProperty::DrawBridge(const YsMatrix4x4 &viewMat) const
{
	YsVec3 pos=belongTo->GetPosition();
	YsAtt3 att=belongTo->GetAttitude();
	auto ev=att.GetForwardVector();
	auto uv=att.GetUpVector();

	viewMat.Mul(pos,pos,1.0);
	viewMat.Mul(ev,ev,0.0);
	viewMat.Mul(uv,uv,0.0);
	att.SetTwoVector(ev,uv);

	bridge.Draw(pos,att);
	return YSOK;
}

// YSRESULT FsAircraftCarrierProperty::DrawArrestingWire(void) const
// is moved to fsgroundpropertygl.cpp and fsgroundpropertybi.cpp

const FsILS &FsAircraftCarrierProperty::GetILS(void) const
{
	return ils;
}

void FsAircraftCarrierProperty::ForceClearLoadedObject(void)
{
	airList.Set(0,NULL);
	gndList.Set(0,NULL);
}

YSBOOL FsAircraftCarrierProperty::IsOnSomething(const YsArray <YsVec3> &plg,const YsVec3 &pos) const
{
	if(belongTo->IsAlive()==YSTRUE)
	{
		YsVec3 carrierPos;
		YsAtt3 carrierAtt;
		double outRad;
		belongTo->GetPosition(carrierPos);
		outRad=belongTo->GetOutsideRadius();
		if((carrierPos-pos).GetSquareLength()<=outRad*outRad)
		{
			YsVec3 tfm;

			belongTo->GetAttitude(carrierAtt);

			tfm=pos;
			tfm-=carrierPos;
			carrierAtt.MulInverse(tfm,tfm);

			tfm.SetY(plg[0].y());

			if(YsCheckInsidePolygon3(tfm,plg.GetNumItem(),plg.GetArray())==YSINSIDE)
			{
				return YSTRUE;
			}
		}
	}
	return YSFALSE;
}

YSRESULT FsAircraftCarrierProperty::PrepareDeckShell(void)
{
	deckCache.ClearDeep();
	if(1<=deck.GetNumPolygon())
	{
		deckCache.Set(deck.GetNumPolygon(),NULL);

		int plId=0;
		for(YsShellPolygonHandle plHd=NULL; NULL!=(plHd=deck.FindNextPolygon(plHd)); )
		{
			YsVec3 nom;
			deck.GetNormalOfPolygon(nom,plHd);
			if(nom==YsOrigin())
			{
				nom=YsYVec();
			}
			else
			{
				nom.Normalize();
			}
			deck.SetNormalOfPolygon(plHd,nom); // Make sure a normal is a unit vector.

			deckCache[plId].Initialize();
			deckCache[plId].plHd=plHd;
			deck.GetVertexListOfPolygon(deckCache[plId].plVtPos,plHd);

			deckCache[plId].projPlVtPos.Set(deckCache[plId].plVtPos.GetN(),NULL);
			for(int i=0; i<deckCache[plId].plVtPos.GetN(); i++)
			{
				deckCache[plId].projPlVtPos[i].Set(deckCache[plId].plVtPos[i].x(),deckCache[plId].plVtPos[i].z());
			}

			YsVec3 cen;
			YsGetCenterOfPolygon3(cen,deckCache[plId].plVtPos.GetN(),deckCache[plId].plVtPos);
			deckCache[plId].pln.Set(cen,nom);

			plId++;
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsAircraftCarrierProperty::ConvertShellToArray(YsArray <YsVec3> &ary,const YsShell &shl) const
{
	if(shl.GetNumPolygon()==1)
	{
		ary.Set(shl.GetNumVertexOfPolygon(0),NULL);
		shl.GetVertexListOfPolygon(ary.GetEditableArray(),ary.GetNumItem(),0);
		return YSOK;
	}
	return YSERR;
}





// Implementation //////////////////////////////////////////
FsGroundProperty::FsGroundProperty()
{
	Initialize();
}

FsGroundProperty::~FsGroundProperty()
{
	if(isAircraftCarrier==YSTRUE && aircraftCarrierProperty!=NULL)
	{
		delete aircraftCarrierProperty;
	}
}

void FsGroundProperty::Initialize(void)
{
	InitializeState();

	FsVehicleProperty::InitializeCharacteristic();

	chMaxSpeed=0.0;
	chMaxSpeedRev=0.0;
	chMaxRotation=0.0;
	chConstHeadingRotation=0.0;
	chConstPitchRotation=0.0;
	chConstBankRotation=0.0;
	chManSpeed1=0.0;
	chManSpeed2=YsInfinity;
	chManSpeed3=YsInfinity;
	chTireRadius=0.3;
	chMaxAccel=0.0;
	chMaxAccelRev=0.0;
	chMaxBrake=0.0;


	chSamReloadMax=0;
	chSamReloadTime=0.0;
	chAaaInterval=0.1;

	chAaaMinAimPitch=0.0;
	chAaaMaxAimPitch=YsDegToRad(90.0);
	chSamMinAimPitch=0.0;
	chSamMaxAimPitch=YsDegToRad(90.0);
	chCanMinAimPitch=0.0;
	chCanMaxAimPitch=YsDegToRad(90.0);
	chType=FSSTATIC;
	chCanMoveArea=CANMOVE_ANYWHERE;
	chCanMoveRegionId=0;
	chUserControlLevel=0;

	chAimRotationSpeed=0.0;
	chAimPitchSpeed=0.0;

	chFlags=0;
	chIsVisualLandingAid=YSGP_NOTVISUALLANDINGAID;
	chSkipGroundToGroundCollisionCheck=YSFALSE;

	chGunIniSpeed=340.0*5.0;  // Mach 5.0
	chAaaRange=0.0;
	chCannonRange=0.0;
	chSAMRange=0.0;

	chAaaMount.Set(0.0,2.0,0.0);
	chAaaGunnerPosition.Set(0.0,3.0,0.0);
	chAaaGunnerOffset.Set(0.0,0.0,0.0);
	chNumAaaPosition=0; // 0->Not specified in .DAT
	chAaaPosition[0].Set(0.0,0.0,10.0);

	chGunPrecision=YsPi/360.0;

	chSAMType=FSWEAPON_AIM9;
	chSamMount.Set(0.0,2.0,0.0);
	chSamGunnerPosition.Set(0.0,2.0,0.0);
	chSamGunnerOffset.Set(0.0,0.0,0.0);
	chNumSamPosition=0; // 0->Not specified in .DAT
	chSamPosition[0].Set(1.0,0.0,3.0);
	chSamPosition[1].Set(-1.0,0.0,3.0);

	chCanMount.Set(0.0,2.0,0.0);
	chCanGunnerPosition.Set(0.0,2.0,0.0);
	chCanGunnerOffset.Set(0.0,0.0,0.0);
	chNumCanPosition=0; // 0->Not specfied in .DAT
	chCanPosition[0].Set(0.0,0.0,10.0);

	chSyncAaaSamAim=YSFALSE;

	chAimAt=YsOrigin();

	chVorRange=0.0;
	chNdbRange=0.0;
	chIsDme=YSFALSE;

	chMinimumDamage=0;

	isAircraftCarrier=YSFALSE;
	aircraftCarrierProperty=NULL;

	belongTo=NULL;
}

void FsGroundProperty::InitializeState(void)
{
	FsVehicleProperty::InitializeState();

	staState=FSGNDALIVE;

	staWoc=FSWEAPON_NULL;
	staGunBullet=3000;
	staSAM=8;
	staAirTarget=NULL;
	staGndTarget=NULL;
	staSpeed=YsOrigin();
	staDrift=YSFALSE;
	staRotation=0.0;
	staIsStatic=YSTRUE;
	staWheelRotation=0.0;
	staNextMissileFireTime=0.0;
	staSamReloadCount=0;
	staTimeCounter=0.0;
	staAaaAim.Set(0.0,0.0,0.0);
	staSamAim.Set(0.0,0.0,0.0);
	staCanAim.Set(0.0,0.0,0.0);
	staFiringAaa=YSFALSE;
	staFiringSam=YSFALSE;
	staFiringCannon=YSFALSE;
	staAaaTimer=0.0;
	staCannonTimer=0.0;

	staLightState=0;
	staLightTimer=0.0;

	staWhoIsInControl=FSVEHICLE_CTRL_BY_NOBODY;
	staSteering=0.0;
	staPitchRotation=0.0;
	staHeadingRotation=0.0;
	staAccel=0.0;
	staReverse=0.0;
	staWheelRotation=0.0;
	staBrake=0.0;
	staSetSpeed=0.0;
	staSetVh=0.0;

	staLeftDoor=0.0;
	staRightDoor=0.0;
	staRearDoor=0.0;
	staLeftDoorToBe=0.0;
	staRightDoorToBe=0.0;
	staRearDoorToBe=0.0;

	staSetTurretVh=0.0;
	staSetTurretVp=0.0;
	staSetTurretVb=0.0;
}

void FsGroundProperty::CleanUpAircraftCarrierProperty(void)
{
	if(aircraftCarrierProperty!=NULL)
	{
		delete aircraftCarrierProperty;
	}
	isAircraftCarrier=YSFALSE;
	aircraftCarrierProperty=NULL;
}

// Why aircraftCarrierProperty is needed?
// Because the aircraft carrier property is not loaded when property is loaded from
// file.
YSRESULT FsGroundProperty::LoadProperty(const wchar_t fn[],YsWString &aircraftCarrierProperty)
{
	aircraftCarrierProperty.Set(NULL);

	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		char dat[256];
		while(fgets(dat,256,fp)!=NULL)
		{
			if(strncmp(dat,"CARRIER",7)==0)
			{
				int ac;
				char *av[16];
				if(YsArguments(&ac,av,16,dat)==YSOK && ac>=2)
				{
					aircraftCarrierProperty.SetUTF8String(av[1]);
				}
			}
			else if(SendCommand(dat)!=YSOK)
			{
				fclose(fp);
				return YSERR;
			}
		}
		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

void FsGroundProperty::ApplyControl(const FsFlightControl &ctl,unsigned int /*whatToApply*/)
{
	staWhoIsInControl=FSVEHICLE_CTRL_BY_PILOT;
	if(YsTolerance<ctl.ctlThrRev)
	{
		staSetSpeed=-ctl.ctlThrRev*chMaxSpeedRev;
		staAccel=0.0;
		staReverse=ctl.ctlThrRev;
	}
	else
	{
		staSetSpeed=ctl.ctlThrottle*chMaxSpeed;
		staAccel=ctl.ctlThrottle;
		staReverse=0.0;
	}

	staBrake=ctl.ctlBrake;
	switch(staWoc)
	{
	case FSWEAPON_NULL:
		staSteering=ctl.ctlAileron;
		staPitchRotation=0.0;
		staHeadingRotation=0.0;
		staFiringAaa=YSFALSE;
		staFiringSam=YSFALSE;
		break;
	case FSWEAPON_GUN:
		staSteering=ctl.ctlRudder;
		staPitchRotation=ctl.ctlElevator;
		staHeadingRotation=ctl.ctlAileron;
		staFiringAaa=ctl.ctlFireWeaponButton;
		staFiringSam=YSFALSE;
		break;
	default:
	case FSWEAPON_AIM9:
	case FSWEAPON_AGM65:
		staSteering=ctl.ctlRudder;
		staPitchRotation=ctl.ctlElevator;
		staHeadingRotation=ctl.ctlAileron;
		staFiringAaa=YSFALSE;
		staFiringSam=ctl.ctlFireWeaponButton;
		break;
	}

	staLeftDoorToBe=ctl.ctlLeftDoor;
	staRightDoorToBe=ctl.ctlRightDoor;
	staRearDoorToBe=ctl.ctlRearDoor;
}

void FsGroundProperty::PrepareUserWeaponofChoice(void)
{
	staWoc=FSWEAPON_NULL;
	if(0==(chUserControlLevel&USERCTL_DRIVE))
	{
		CycleWeaponOfChoiceByUser();
	}
}

YSRESULT FsGroundProperty::CycleWeaponOfChoiceByUser(void)
{
	if(0==chUserControlLevel)
	{
		return YSERR;
	}

	const int nCycle=4;
	for(int i=0; i<nCycle; i++)
	{
		switch(staWoc)
		{
		default:
			staWoc=FSWEAPON_GUN;
			break;
		case FSWEAPON_NULL:
			staWoc=FSWEAPON_GUN;
			break;
		case FSWEAPON_GUN:
			staWoc=FSWEAPON_AIM9;
			break;
		case FSWEAPON_AIM9:
			staWoc=FSWEAPON_NULL;
			break;
		}

		if(0!=(chUserControlLevel&USERCTL_DRIVE) && FSWEAPON_NULL==staWoc)
		{
			return YSOK;
		}
		if(0!=(chUserControlLevel&USERCTL_GUN) && FSWEAPON_GUN==staWoc)
		{
			return YSOK;
		}
		if(0!=(chUserControlLevel&USERCTL_SAM) && FSWEAPON_AIM9==staWoc)
		{
			return YSOK;
		}
		if(0!=(chUserControlLevel&USERCTL_ATM) && FSWEAPON_AGM65==staWoc)
		{
			return YSOK;
		}
	}

	staWoc=FSWEAPON_NULL;
	return YSERR;
}

FSGROUNDTYPE FsGroundProperty::GetGroundType(void) const
{
	return chType;
}

void FsGroundProperty::SetWhoIsInControl(FSVEHICLECONTROLLER ctrl)
{
	staWhoIsInControl=ctrl;
}

FSVEHICLECONTROLLER FsGroundProperty::GetWhoIsInControl(void) const
{
	return staWhoIsInControl;
}

void FsGroundProperty::SetSteering(const double steer)
{
	this->staSteering=steer;
}

void FsGroundProperty::SetBrake(const double brake)
{
	this->staBrake=brake;
}

void FsGroundProperty::SetAccel(const double accel)
{
	this->staAccel=accel;
}

void FsGroundProperty::SetDesiredSpeed(const double speed)
{
	this->staSetSpeed=speed;
}

void FsGroundProperty::SetReverse(const double reverse)
{
	this->staReverse=reverse;
}

double FsGroundProperty::CalculateBrakeAcceleration(const double brake) const
{
	return brake*chMaxBrake;
}

double FsGroundProperty::CalculateCurrentTurnRate(const double steer) const
{
	const double v=staSpeed.GetLength();
	return CalculateTurnRate(steer,v);
}

double FsGroundProperty::CalculateTurnRate(const double steer,const double v) const
{
	if(YsTolerance<v)
	{
		const double speedSign=(0.0<staSpeed.z() ? 1.0 : -1.0);
		const double maxRotation=(chManSpeed1<v ? chMaxRotation : chMaxRotation*v/chManSpeed1);

		return speedSign*maxRotation*steer;
	}
	return 0.0;
}

double FsGroundProperty::MoveDevice(
    const double &toBe,double rightNow,const double &howFast,const double &dt)
{
	const double motion=howFast*dt; // (1.0/howFast)*dt;

	if(toBe<rightNow)
	{
		rightNow=rightNow-motion;
		if(rightNow<toBe)
		{
			rightNow=toBe;
		}
	}
	else if(toBe>rightNow)
	{
		rightNow=rightNow+motion;
		if(rightNow>toBe)
		{
			rightNow=toBe;
		}
	}
	return rightNow;
}

void FsGroundProperty::Move(
    YsVec3 &motionPathOffset,YSSIZE_T &motionPathIndex,YSBOOL useMotionPathOffset,
	YSSIZE_T nMpathPnt,const YsVec3 mpathPnt[],YSBOOL mpathIsLoop, /* const YsSceneryPointSet *motionPath, */
	const double &dt)
{
	const YsVec3 prevPos=staPosition;  // Never reference it.  It will be updated in this function, and will be compared in this function.
	const YsAtt3 prevAtt=staAttitude;  // Never reference it.  It will be updated in this function, and will be compared in this function.

	if(FSVEHICLE_CTRL_BY_PILOT==staWhoIsInControl || FSVEHICLE_CTRL_BY_AUTOPILOT==staWhoIsInControl)
	{
		const double v0=staSpeed.z();  // staSpeed is in the local coordinate.
		double v=v0;
		const double accelSign=(v<staSetSpeed ? 1.0 : -1.0);
		const double accel=(0.0<staSetSpeed ? chMaxAccel : chMaxAccelRev);

		if(fabs(v-staSetSpeed)<accel*dt)
		{
			v=staSetSpeed;
		}
		else
		{
			v+=accelSign*accel*dt;
		}

		const double brake=CalculateBrakeAcceleration(staBrake);
		if(fabs(v)<brake*dt)
		{
			v=0.0;
		}
		else
		{
			if(0.0<v)
			{
				v-=brake*dt;
			}
			else
			{
				v+=brake*dt;
			}
		}

		staRotation=CalculateCurrentTurnRate(staSteering);

		const double absv=staSpeed.GetLength();
		const double driftThr=chManSpeed3-(chManSpeed3-chManSpeed2)*fabs(staSteering);
		if(absv<driftThr)
		{
			staSpeed.Set(0.0,0.0,v);
			staDrift=YSFALSE;
		}
		else
		{
			staSpeed.SetZ(v);
			staSpeed.RotateXZ(-staRotation*dt);
			staDrift=YSTRUE;
		}

		switch(staWoc)
		{
		default:
		case FSWEAPON_GUN:
			{
				const double dh=chAimRotationSpeed*dt*staHeadingRotation;
				const double dp=chAimPitchSpeed*dt*staPitchRotation;
				staAaaAim.SetH(staAaaAim.h()+dh);
				const double p=YsBound(staAaaAim.p()+dp,chAaaMinAimPitch,chAaaMaxAimPitch);
				staAaaAim.SetP(p);
				if(YSTRUE==chSyncAaaSamAim)
				{
					staSamAim=staAaaAim;
				}
			}
			break;
		case FSWEAPON_AGM65:
			{
				const double dh=chAimRotationSpeed*dt*staHeadingRotation;
				const double dp=chAimPitchSpeed*dt*staPitchRotation;
				staSamAim.SetH(staSamAim.h()+dh);
				const double p=YsBound(staSamAim.p()+dp,chSamMinAimPitch,chSamMaxAimPitch);
				staSamAim.SetP(p);
				if(YSTRUE==chSyncAaaSamAim)
				{
					staAaaAim=staSamAim;
				}
			}
			break;
		}
	}


	// 2012/03/06
	// If the object is on top of an aircraft carrier, and it shares the same
	// motion path, and motion path offset is on, the intention probably is
	// to let the object move together.
	// Therefore, if it is the case, don't move it.
	const YSBOOL piggyBack=belongTo->PiggyBack();

	if(YSTRUE!=piggyBack &&
	   (staSpeed!=YsOrigin() ||
	    YsEqual(staRotation,0.0)!=YSTRUE ||
	    YsEqual(chConstHeadingRotation,0.0)!=YSTRUE ||
	    YsEqual(chConstPitchRotation,0.0)!=YSTRUE ||
	    YsEqual(chConstBankRotation,0.0)!=YSTRUE))  // Moving object
	{
		YsVec3 v;
		YsMatrix4x4 prvMat(staMatrix);
		double h;


		if(YsTolerance<chTireRadius)
		{
			const double wheelRot=-staSpeed.z()*dt/chTireRadius;
			staWheelRotation+=wheelRot;
		}


		// prv.Translate(staPosition);
		// prv.Rotate(staAttitude);

		staAttitude.Mul(v,staSpeed);
		v*=dt; // v=staAttitude.GetMatrix()*v*staSpeed*t;
		staPosition+=v;

		h=staRotation*dt;
		staAttitude.SetH(staAttitude.h()+h);
		if(YsEqual(chConstHeadingRotation,0.0)!=YSTRUE)
		{
			staAttitude.YawLeft(chConstHeadingRotation*dt);
		}
		if(YsEqual(chConstPitchRotation,0.0)!=YSTRUE)
		{
			staAttitude.NoseUp(chConstPitchRotation*dt);
		}
		staAttitude.SetB(staAttitude.b()+chConstBankRotation*dt);

		if(prevPos!=staPosition && MustStayOnGround()==YSTRUE)
		{
			PutOnGround();
		}

		if(prevPos!=staPosition || prevAtt!=staAttitude)
		{
			RemakeMatrix();
		}


		// Move Aim to Target
		// Default option
		//   In the future, AI will be implemented. Then, AI will take over
		//   the default option.

		if(staAirTarget!=NULL)
		{
		}

		if(isAircraftCarrier==YSTRUE)
		{
			aircraftCarrierProperty->MoveCargoAndILS(staMatrix,prvMat,h);
		}

		if(nMpathPnt>0) // motionPath!=NULL) // Any ground object with motion path
		{
			ManeuverAlongMotionPath(motionPathOffset,motionPathIndex,nMpathPnt,mpathPnt,mpathIsLoop,useMotionPathOffset);
		}
		else if(isAircraftCarrier==YSTRUE)  // An aircraft carrier without motion path
		{
			aircraftCarrierProperty->Maneuver(dt);
		}

		staIsStatic=YSFALSE;
	}
	else  // Static object or an object not yet started moving
	{
		if(isAircraftCarrier==YSTRUE)
		{
			// YsMatrix4x4 now;
			// now.Translate(staPosition);
			// now.Rotate(staAttitude);
			// aircraftCarrierProperty->Move(now,now,0.0);

			aircraftCarrierProperty->MoveCargoAndILS(staMatrix,staMatrix,0.0);
		}

		if(nMpathPnt>0) // motionPath!=NULL)
		{
			ManeuverAlongMotionPath(motionPathOffset,motionPathIndex,nMpathPnt,mpathPnt,mpathIsLoop,useMotionPathOffset);
		}
		else if(isAircraftCarrier==YSTRUE)
		{
			aircraftCarrierProperty->Maneuver(dt);
		}

		if(prevPos!=staPosition || prevAtt!=staAttitude)
		{
			RemakeMatrix();
		}

		staIsStatic=YSTRUE;
	}


	for(int i=0; i<chTurret.GetN(); i++)
	{
		staTurret[i].h=MoveDevice(staTurret[i].ctlH,staTurret[i].h,chTurret[i].vh,dt);
		staTurret[i].p=MoveDevice(staTurret[i].ctlP,staTurret[i].p,chTurret[i].vp,dt);
	}

	staLeftDoor=MoveDevice(staLeftDoorToBe,staLeftDoor,2.0,dt);
	staRightDoor=MoveDevice(staRightDoorToBe,staRightDoor,2.0,dt);
	staRearDoor=MoveDevice(staRearDoorToBe,staRearDoor,2.0,dt);

	staTimeCounter+=dt;
}

void FsGroundProperty::ManeuverAlongMotionPath(
    YsVec3 &motionPathOffset,YSSIZE_T &motionPathIndex,
    YSSIZE_T nMpathPnt,const YsVec3 *mpathPnt,YSBOOL mpathIsLoop, /* const YsSceneryPointSet *motionPath, */
    YSBOOL useMotionPathOffset)
{
	// const YsSceneryItem *itm;
	if(nMpathPnt>0)
	// if(motionPath->GetNumPoint()>0)
	{
		if(useMotionPathOffset==YSTRUE && motionPathOffset==YsOrigin())
		{
			YsVec3 motionPathOrigin;

			motionPathOrigin=mpathPnt[0];
			// motionPathOrigin=motionPath->GetPointArray()[0];
			// itm=motionPath;
			// while(itm!=NULL)
			// {
			// 	itm->GetAttitude().Mul(motionPathOrigin,motionPathOrigin);
			// 	motionPathOrigin+=itm->GetPosition();
			// 	itm=itm->GetOwner();
			// }


			motionPathOffset=staPosition-motionPathOrigin;
			if(CanFloat()==YSFALSE)
			{
				motionPathOffset.SetY(0.0);
			}
		}


		// if(motionPathIndex<motionPath->GetNumPoint())
		if(motionPathIndex<nMpathPnt)
		{
			YsVec3 wayPoint,dist;
			double angle;

			if(motionPathIndex<0)
			{
				motionPathIndex=0;
			}

			// wayPoint=motionPath->GetPointArray()[motionPathIndex];
			// itm=motionPath;
			// while(itm!=NULL)
			// {
			// 	itm->GetAttitude().Mul(wayPoint,wayPoint);
			// 	wayPoint+=itm->GetPosition();
			// 	itm=itm->GetOwner();
			// }

			wayPoint=mpathPnt[motionPathIndex];

			wayPoint+=motionPathOffset;
			dist=wayPoint-staPosition;
			if(CanFloat()==YSFALSE)
			{
				dist.SetY(0.0);

				if(staGndTarget==NULL && staAirTarget==NULL)  // 2005/02/23
				{
					staAaaAim.SetForwardVector(dist);
					staAaaAim.SetB(0.0);

					staSamAim=staAaaAim;
					staCanAim=staAaaAim;
				}

				if(dist.GetSquareLength()<10.0)
				{
					motionPathIndex++;
					if(nMpathPnt<=motionPathIndex && mpathIsLoop==YSTRUE)
					{
						motionPathIndex=0;
					}
				}

				dist.RotateXZ(-staAttitude.h());
				angle=atan2(-dist.x(),dist.z());
				staRotation=YsBound(angle*3.0,-chMaxRotation,chMaxRotation);
				staSpeed.Set(0.0,0.0,chMaxSpeed);
			}
			else
			{
				if(dist.GetSquareLength()<10.0)
				{
					motionPathIndex++;
					if(nMpathPnt<=motionPathIndex && mpathIsLoop==YSTRUE)
					{
						motionPathIndex=0;
					}
				}

				if(StayUpright()!=YSTRUE)
				{
					if(dist.Normalize()==YSOK)
					{
						staAttitude.SetP(atan(dist.y()));

						dist.RotateXZ(-staAttitude.h());
						angle=atan2(-dist.x(),dist.z());
						staRotation=YsBound(angle*3.0,-chMaxRotation,chMaxRotation);

						staSpeed=dist*chMaxSpeed;
					}
					else
					{
						staRotation=0.0;
						staSpeed=YsOrigin();
					}
				}
				else // StayUpRight==TRUE -> No Rotation
				{
					staRotation=0.0;
					if(dist.Normalize()==YSOK)
					{
						staAttitude.MulInverse(dist,dist);
						staSpeed=dist*chMaxSpeed;
					}
					else
					{
						staSpeed=YsOrigin();
					}
				}
			}
		}
		else
		{
			staRotation=0.0;
			staSpeed=YsOrigin();
		}
	}
	else
	{
		staSpeed=YsOrigin();
		staRotation=0.0;
	}
}

void FsGroundProperty::AfterUnloadedFromCarrier(void)
{
	staOnThisCarrier=NULL;
}

void FsGroundProperty::AfterLoadedOnCarrier(FsGround *carrier)
{
	staOnThisCarrier=carrier;
}

YSBOOL FsGroundProperty::FireGun(
    const double &ct,const double &dt,FsSimulation *sim,FsWeaponHolder &bul,class FsExistence *own)
{
	YSBOOL firing;

	firing=YSFALSE;
	if(IsAlive()==YSTRUE)
	{
		int i;
		YsVec3 v;

		staAttitude.Mul(v,staSpeed);

		for(i=0; i<chTurret.GetN(); i++)
		{
			if(staTurret[i].numBullet>0 && (staTurret[i].turretState&FSTURRETSTATE_FIRING)!=0)
			{
				firing=YSTRUE;
			}
			staTurret[i].FireWeapon(chTurret[i],ct,dt,v,staMatrix,sim,bul,own);
		}


		if(staFiringAaa==YSTRUE && staGunBullet>0)
		{
			staAaaTimer-=dt;
			while(staAaaTimer<=0.0)
			{
				for(int i=0; i<GetNumAaaPosition(); i++)
				{
					YsVec3 gun,mount;
					staAttitude.Mul(mount,chAaaMount);
					staAaaAim.Mul(gun,chAaaPosition[i]);
					gun=staPosition+mount+gun;

					YsAtt3 att;
					att=staAaaAim;
					att.SetH(att.h()+chGunPrecision*double(rand()%100-50)/50.0);
					att.SetP(att.p()+chGunPrecision*double(rand()%100-50)/50.0);

					bul.Fire(ct,gun,att,GetBulletSpeed(),chAaaRange,1,own,YSTRUE,YSTRUE);
					staGunBullet--;
				}

				staAaaTimer+=chAaaInterval;
			}
			firing=YSTRUE;
		}

		if(staFiringCannon==YSTRUE && staCannon>0)
		{
			const double shootInterval=1.5;  // 1 bullets / 1.5 second

			staCannonTimer-=dt;
			while(staCannonTimer<=0.0)
			{
				for(int i=0; i<GetNumCannonPosition(); i++)
				{
					YsVec3 gun,mount;
					staAttitude.Mul(mount,chCanMount);
					staCanAim.Mul(gun,chCanPosition[i]);
					gun=staPosition+mount+gun;

					// No random pertubation for cannons
					bul.Fire(ct,gun,staCanAim,GetBulletSpeed(),chCannonRange,3,own,YSTRUE,YSTRUE);
					staCannon--;
				}

				staCannonTimer+=shootInterval;
			}
			firing=YSTRUE;
		}
	}
	return firing;
}

YSBOOL FsGroundProperty::FireMissile
    (const double &ct,class FsWeaponHolder &bul,class FsExistence *own)
{
	FsAirplane *target=NULL;
	YSHASHKEY targetKey=YSNULLHASHKEY;
	if(NULL!=staAirTarget)
	{
		YsVec3 targetDir,aimDir;
		double rcs,range;

		targetDir=(staAirTarget->GetPosition()-staPosition);
		rcs=staAirTarget->Prop().GetRadarCrossSection();
		range=GetSAMRange()*rcs;

		if(targetDir.GetSquareLength()<range*range)
		{
			targetDir.Normalize();
			aimDir.Set(0.0,0.0,1.0);
			staSamAim.Mul(aimDir,aimDir);

			if(aimDir*targetDir>0.998 || (FSVEHICLE_CTRL_BY_PILOT==staWhoIsInControl || FSVEHICLE_CTRL_BY_AUTOPILOT==staWhoIsInControl))
			{
				target=staAirTarget;
				targetKey=staAirTarget->SearchKey();
			}
		}
	}


	YSBOOL firing=YSFALSE;
	if(FSVEHICLE_CTRL_BY_PILOT==staWhoIsInControl || FSVEHICLE_CTRL_BY_AUTOPILOT==staWhoIsInControl)
	{
		firing=staFiringSam;
	}
	else if(NULL!=target)
	{
		firing=YSTRUE;
	}


	if(IsAlive()==YSTRUE && staNextMissileFireTime<=ct && YSTRUE==firing && staSAM>0)
	{
		YsVec3 mis,samMount;

		if(0<GetNumSamPosition())
		{
			mis=chSamPosition[staSAM%GetNumSamPosition()];
		}
		else
		{
			mis=((staSAM%1)!=0 ? chSamPosition[0] : chSamPosition[1]);
		}

		staAttitude.Mul(samMount,chSamMount);

		staSamAim.Mul(mis,mis);
		mis=mis+samMount;
		mis=mis+staPosition;

		bul.Fire(ct,
		         chSAMType,
		         mis,
		         staSamAim,
		         0.0,
		         340.0*3.0,
		         chSAMRange,
		         YsDegToRad(180.0),
		         GetSAMRadarAngle(),
		         12,
		         own,
		         targetKey, // <- Locked On Target
		         YSTRUE,
		         YSTRUE);

		staSAM--;
		if(staSamReloadCount>0)
		{
			staSamReloadCount--;
		}

		if(chSamReloadMax>0 && --staSamReloadCount<=0)
		{
			staNextMissileFireTime=ct+chSamReloadTime;
			staSamReloadCount=chSamReloadMax;
		}
		else
		{
			staNextMissileFireTime=ct+10.0;
		}
		return YSTRUE;
	}
	return YSFALSE;
}

const double FsGroundProperty::GetTimeBeforeNextMissileCanBeShot(const double ctime) const
{
	const double dt=staNextMissileFireTime-ctime;
	if(0.0<dt)
	{
		return dt;
	}
	return 0.0;
}

YSBOOL FsGroundProperty::GetDamage(YSBOOL &killed,int dmg)
{
	killed=YSFALSE;
	if(staState==FSGNDALIVE)
	{
		if(dmg>chMinimumDamage)
		{
			staDamageTolerance-=dmg;
			if(staDamageTolerance<=0)
			{
				staState=FSGNDDEAD;
				staDamageTolerance=0;
				killed=YSTRUE;
			}
			return YSTRUE;
		}
	}
	return YSFALSE;
}

double FsGroundProperty::GetBulletSpeed(void) const
{
	return chGunIniSpeed;
}

double FsGroundProperty::GetSAMRadarAngle(void) const
{
	return YsPi/6.0;
}

double FsGroundProperty::GetSAMRange(void) const
{
	return chSAMRange;
}

double FsGroundProperty::GetAAARange(void) const
{
	return chAaaRange;
}

YSBOOL FsGroundProperty::SetAirTarget(class FsAirplane *trg)
{
	staAirTarget=trg;
	return YSTRUE;
}

FsAirplane *FsGroundProperty::GetAirTarget(void) const
{
	return staAirTarget;
}

unsigned int FsGroundProperty::GetAirTargetKey(void) const
{
	if(NULL!=staAirTarget)
	{
		return staAirTarget->SearchKey();
	}
	return YSNULLHASHKEY;
}

YSBOOL FsGroundProperty::SetGroundTarget(class FsGround *trg)
{
	staGndTarget=trg;
	return YSTRUE;
}

FsGround *FsGroundProperty::GetGroundTarget(void) const
{
	return staGndTarget;
}

const YsVec3 &FsGroundProperty::GetAaaMountPoint(void) const
{
	return chAaaMount;
}

const YsVec3 &FsGroundProperty::GetSamMountPoint(void) const
{
	return chSamMount;
}

const YsVec3 &FsGroundProperty::GetCannonMountPoint(void) const
{
	return chCanMount;
}

YSBOOL FsGroundProperty::IsAlive(void) const
{
	if(staState!=FSGNDDEAD && staDamageTolerance>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsGroundProperty::IsActive(void) const
{
	return IsAlive();  // It is same as IsAlive for now.
}

YSBOOL FsGroundProperty::IsStatic(void) const
{
	return staIsStatic;
}

YSBOOL FsGroundProperty::IsControlledByUser(void) const
{
	return (FSVEHICLE_CTRL_BY_PILOT==staWhoIsInControl ? YSTRUE : YSFALSE);
}

YSBOOL FsGroundProperty::IsAntiGround(void) const
{
	if(chFlags&YSGP_ANTIGROUND)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsGroundProperty::MustStayOnGround(void) const
{
	if(chFlags&YSGP_HUGGROUND)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsGroundProperty::CanFloat(void) const
{
	if(chFlags&YSGP_CANFLOAT)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsGroundProperty::StayUpright(void) const
{
	if(chFlags&YSGP_STAYUPRIGHT)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsGroundProperty::NoShadow(void) const
{
	if(chFlags&YSGP_NOSHADOW)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsGroundProperty::TrackAirTarget(void) const
{
	if(chFlags&YSGP_TRACKAIRTARGET)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsGroundProperty::TrackGndTarget(void) const
{
	return YSFALSE;
}

YSBOOL FsGroundProperty::TargetAny(void) const
{
	if(chFlags&YSGP_TARGETANY)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsGroundProperty::IsPapi(void) const
{
	if(chIsVisualLandingAid==YSGP_PAPI)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsGroundProperty::IsVasi(void) const
{
	if(chIsVisualLandingAid==YSGP_VASI)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsGroundProperty::SkipGroundToGroundCollisionCheck(void) const
{
	return chSkipGroundToGroundCollisionCheck;
}

YSBOOL FsGroundProperty::IsNonGameObject(void) const
{
	if(chFlags&YSGP_NONEGAMEOBJECT)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

void FsGroundProperty::SetupVisual(FsVisualDnm &vis,const YsVec3 &viewPoint,const double &ctime) const
{
	int i;
	YsVec3 ev,uv;
	YsAtt3 relAtt;
	YsMatrix3x3 attInverse(YSFALSE);

	staAttitude.GetMatrix3x3(attInverse);
	attInverse.Transpose();

	ev=staAaaAim.GetForwardVector();
	uv=staAaaAim.GetUpVector();
	attInverse.Mul(ev,ev);
	attInverse.Mul(uv,uv);

	relAtt.SetTwoVector(ev,uv);

	vis.SetHeading(1,relAtt.h());
	vis.SetPitch(2,relAtt.p());



	ev=staSamAim.GetForwardVector();
	uv=staSamAim.GetUpVector();
	attInverse.Mul(ev,ev);
	attInverse.Mul(uv,uv);

	relAtt.SetTwoVector(ev,uv);

	vis.SetHeading(3,relAtt.h());
	vis.SetPitch(4,relAtt.p());


	ev=staCanAim.GetForwardVector();
	uv=staCanAim.GetUpVector();
	attInverse.Mul(ev,ev);
	attInverse.Mul(uv,uv);

	relAtt.SetTwoVector(ev,uv);

	vis.SetHeading(5,relAtt.h());
	vis.SetPitch(6,relAtt.p());

	vis.SetHeading(10,YsPi*0.8*ctime);
	vis.SetHeading(11,YsPi*1.2*ctime);

	vis.SetPitch(YSDNM_CLASSID_TIRE,staWheelRotation);

	if(staSteering>=0.0)
	{
		vis.SetState(YSDNM_CLASSID_STEERING,0,1,staSteering);
	}
	else
	{
		vis.SetState(YSDNM_CLASSID_STEERING,0,2,-staSteering);
	}

	vis.SetState(YSDNM_CLASSID_BRAKE,0,1,staBrake);
	vis.SetState(YSDNM_CLASSID_LANDINGLIGHT,(0!=(staLightState&FSLIGHT_GND_HEADLIGHT) ? 1 : 0));
	vis.SetState(YSDNM_CLASSID_LEFTDOOR,0,1,staLeftDoor);
	vis.SetState(YSDNM_CLASSID_RIGHTDOOR,0,1,staRightDoor);
	vis.SetState(YSDNM_CLASSID_REARDOOR,0,1,staRearDoor);

	if(0.01<=staLeftDoor || 0.01<=staRightDoor || 0.01<=staRearDoor)
	{
		vis.SetState(YSDNM_CLASSID_INTERIOR,1);
	}
	else
	{
		vis.SetState(YSDNM_CLASSID_INTERIOR,0);
	}

	if(IsPapi()==YSTRUE || IsVasi()==YSTRUE)
	{
		double angle;
		YsVec3 relViewPos;
		attInverse.Mul(relViewPos,viewPoint-staPosition);
		if(relViewPos.z()>YsTolerance)
		{
			angle=atan(relViewPos.y()/relViewPos.z());
		}
		else
		{
			angle=0.0;
		}

		if(IsPapi()==YSTRUE)
		{
			vis.SetPapiColor(angle);
		}
		else if(IsVasi()==YSTRUE)
		{
			vis.SetVasiColor(angle);
		}
	}

	for(i=0; i<chTurret.GetN(); i++)
	{
		staTurret[i].SetupVisual(vis,chTurret[i]);
	}

	vis.CacheTransformation();
}


void FsGroundProperty::CopyState(const FsGroundProperty &from)
{
	InitializeState();

	staState=from.staState;

	staPosition=from.staPosition;
	staAttitude=from.staAttitude;
	staMatrix=from.staMatrix;
	staInverseMatrix=from.staInverseMatrix;


	staSAM=from.staSAM;
	staGunBullet=from.staGunBullet;
	staCannon=from.staCannon;
	staAaaTimer=from.staAaaTimer;
	staCannonTimer=from.staCannonTimer;

	staFiringAaa=from.staFiringAaa;
	staFiringCannon=from.staFiringCannon;

	staDamageTolerance=from.staDamageTolerance;

	staSpeed=from.staSpeed;
	staRotation=from.staRotation;
	staIsStatic=from.staIsStatic;
	staNextMissileFireTime=from.staNextMissileFireTime;
	staSamReloadCount=from.staSamReloadCount;

	staTimeCounter=from.staTimeCounter;

	staAaaAim=from.staAaaAim;
	staSamAim=from.staSamAim;
	staCanAim=from.staCanAim;

	staTurret=from.staTurret;
	staSavedTurret=from.staSavedTurret;

	staAirTarget=from.staAirTarget;
	staGndTarget=from.staGndTarget;

	staLightState=from.staLightState;
	staLightTimer=from.staLightTimer;

	staWhoIsInControl=from.staWhoIsInControl;
	staLeftDoor=from.staLeftDoor;
	staRightDoor=from.staRightDoor;
	staRearDoor=from.staRearDoor;
	staLeftDoorToBe=from.staLeftDoorToBe;
	staRightDoorToBe=from.staRightDoorToBe;
	staRearDoorToBe=from.staRearDoorToBe;
	staSteering=from.staSteering;
	staAccel=from.staAccel;
	staReverse=from.staReverse;
	staWheelRotation=from.staWheelRotation;
	staBrake=from.staBrake;
	staSetSpeed=from.staSetSpeed;
	staSetVh=from.staSetVh;

	staSetTurretVh=from.staSetTurretVh;
	staSetTurretVp=from.staSetTurretVp;
	staSetTurretVb=from.staSetTurretVb;
}

void FsGroundProperty::SetPosition(const YsVec3 &vec)
{
	if(isAircraftCarrier==YSTRUE)
	{
		const YsMatrix4x4 prv=GetMatrix();

		staPosition=vec;
		RemakeMatrix();

		aircraftCarrierProperty->MoveCargoAndILS(GetMatrix(),prv,0.0);
	}
	else
	{
		staPosition=vec;
		RemakeMatrix();
	}
}

void FsGroundProperty::SetAttitude(const YsAtt3 &att)
{
	if(isAircraftCarrier==YSTRUE)
	{
		const YsMatrix4x4 prv=GetMatrix();

		staAttitude=att;
		RemakeMatrix();

		aircraftCarrierProperty->MoveCargoAndILS(GetMatrix(),prv,0.0);
	}
	else
	{
		staAttitude=att;
		RemakeMatrix();
	}
}

void FsGroundProperty::SetPositionAndAttitude(const YsVec3 &vec,const YsAtt3 &att)
{
	if(isAircraftCarrier==YSTRUE)
	{
		const YsMatrix4x4 prv=GetMatrix();

		staAttitude=att;
		staPosition=vec;
		RemakeMatrix();

		aircraftCarrierProperty->MoveCargoAndILS(GetMatrix(),prv,0.0);
	}
	else
	{
		staPosition=vec;
		staAttitude=att;
		RemakeMatrix();
	}
}

void FsGroundProperty::PutOnGround(void)
{
	// Change in 2012/01/01 It no longer takes a pointer to FsSimulation.  Terrain info (elevation and terrain normal) must be updated in FsSimulation.

	// const double &elv=belongTo->elevation;
	const YsVec3 &nom=belongTo->terrainNom;

	// YsVec3 nom,ev;
	// sim->GetFieldElevationAndNormal(elv,nom,staPosition.x(),staPosition.z());
	// nom.Normalize();
	// if(nom.y()<0.0)
	// {
	// 	nom=-nom;
	// }

	const YsPlane pln(belongTo->terrainOrg,belongTo->terrainNom);
	pln.GetNearestPoint(staPosition,staPosition); // staPosition.SetY(elv);

	YsVec3 ev=staAttitude.GetForwardVector();
	ev=nom^(ev^nom);
	staAttitude.SetTwoVector(ev,nom);
}

void FsGroundProperty::GetVelocity(YsVec3 &vel) const
{
	staAttitude.Mul(vel,staSpeed);
}

void FsGroundProperty::SetVelocity(const YsVec3 &newVel)
{
	staAttitude.MulInverse(staSpeed,newVel);
}

void FsGroundProperty::SetRelativeVelocity(const YsVec3 &newVel)
{
	staSpeed=newVel;
}

const double FsGroundProperty::GetMaxSpeed(void) const
{
	return chMaxSpeed;
}

const double FsGroundProperty::GetMaxStableSpeed(void) const
{
	return chManSpeed2;
}

const double FsGroundProperty::GetMinimumManeuvableSpeed(void) const
{
	return chManSpeed1;
}

void FsGroundProperty::SetRotation(const double &rot)
{
	staRotation=rot;
}

const double FsGroundProperty::GetRotation(void) const
{
	return staRotation;
}

YSBOOL FsGroundProperty::IsDrifting(void) const
{
	return staDrift;
}

const double FsGroundProperty::GetVelocity(void) const
{
	return staSpeed.GetLength();
}

const double &FsGroundProperty::GetVorRange(void) const
{
	return chVorRange;
}

const double &FsGroundProperty::GetNdbRange(void) const
{
	return chNdbRange;
}

void FsGroundProperty::WriteRecord(FsGroundRecord &rec) const
{
	rec.pos=staPosition;
	rec.h=float(staAttitude.h());
	rec.p=float(staAttitude.p());
	rec.b=float(staAttitude.b());

	rec.state=(unsigned char)staState;
	rec.dmgTolerance=(unsigned char)staDamageTolerance;

	rec.steering=(char)YsBound((int)(staSteering*127.0),-128,127);
	rec.leftDoor=(unsigned char)YsBound <unsigned int> ((unsigned int)(staLeftDoor*255.0),0,255);
	rec.rightDoor=(unsigned char)YsBound <unsigned int> ((unsigned int)(staRightDoor*255.0),0,255);
	rec.rearDoor=(unsigned char)YsBound <unsigned int> ((unsigned int)(staRearDoor*255.0),0,255);
	rec.brake=(unsigned char)YsBound <unsigned int> ((unsigned int)(staBrake*255.0),0,255);
	rec.lightState=(unsigned char)staLightState;

	rec.aaaAimh=float(staAaaAim.h());
	rec.aaaAimp=float(staAaaAim.p());
	rec.aaaAimb=float(staAaaAim.b());

	rec.samAimh=float(staSamAim.h());
	rec.samAimp=float(staSamAim.p());
	rec.samAimb=float(staSamAim.b());

	rec.canAimh=float(staCanAim.h());
	rec.canAimp=float(staCanAim.p());
	rec.canAimb=float(staCanAim.b());

	if(staTurret.GetN()>0)
	{
		rec.turret.Alloc((int)staTurret.GetN());
		if(rec.turret.GetN()==staTurret.GetN())
		{
			int i;
			for(i=0; i<staTurret.GetN(); i++)
			{
				rec.turret[i].h=(float)staTurret[i].h;
				rec.turret[i].p=(float)staTurret[i].p;
				rec.turret[i].turretState=staTurret[i].turretState;
			}
		}
	}
}

void FsGroundProperty::ReadbackRecord(FsGroundRecord &rec,const double &dt,const double &velocity)
{
	YsMatrix4x4 prv;  // Used only if it is an aircraft carrier.
	if(YSTRUE==isAircraftCarrier)
	{
		prv.Translate(staPosition);
		prv.Rotate(staAttitude);
	}

	if(YsTolerance<chTireRadius)
	{
		const double wheelRot=-velocity*dt/chTireRadius;
		staWheelRotation+=wheelRot;
	}

	staPosition=rec.pos;
	staAttitude.Set(rec.h,rec.p,rec.b);
	RemakeMatrix();

	staSteering=(double)rec.steering/128.0;
	staLeftDoor=(double)rec.leftDoor/255.0;
	staRightDoor=(double)rec.rightDoor/255.0;
	staRearDoor=(double)rec.rearDoor/255.0;
	staBrake=(double)rec.brake/255.0;
	staLightState=rec.lightState;


	staState=FSGNDSTATE(rec.state);
	staDamageTolerance=rec.dmgTolerance;
	staSpeed.Set(0.0,0.0,velocity);

	staAaaAim.Set(rec.aaaAimh,rec.aaaAimp,rec.aaaAimb);
	staSamAim.Set(rec.samAimh,rec.samAimp,rec.samAimb);
	staCanAim.Set(rec.canAimh,rec.canAimp,rec.canAimb);

	if(isAircraftCarrier==YSTRUE)
	{
		aircraftCarrierProperty->MoveCargoAndILS(GetMatrix(),prv,0.0);
	}

	if(staTurret.GetN()>0 && rec.turret.GetN()==staTurret.GetN())
	{
		for(int i=0; i<staTurret.GetN(); i++)
		{
			staTurret[i].h=(double)rec.turret[i].h;
			staTurret[i].p=(double)rec.turret[i].p;
			staTurret[i].ctlH=(double)rec.turret[i].h;
			staTurret[i].ctlP=(double)rec.turret[i].p;
			staTurret[i].turretState=rec.turret[i].turretState;
		}
	}
}

void FsGroundProperty::CaptureState(YsArray <YsString> &stateStringArray) const
{
	stateStringArray.Clear();

	stateStringArray.Increment();
	stateStringArray.GetEnd().Printf("POSITION %.2lfm %.2lfm %.2lfm",staPosition.x(),staPosition.y(),staPosition.z());

	stateStringArray.Increment();
	stateStringArray.GetEnd().Printf("ATTITUDE %.2lfrad %.2lfrad %.2lfrad",staAttitude.h(),staAttitude.p(),staAttitude.b());

	stateStringArray.Increment();
	stateStringArray.GetEnd().Printf("STRENGTH %d",staDamageTolerance);

	stateStringArray.Increment();
	stateStringArray.GetEnd().Printf("INITIGUN %d",staGunBullet);

	stateStringArray.Increment();
	stateStringArray.GetEnd().Printf("INITISAM %d",staSAM);

	stateStringArray.Increment();
	stateStringArray.GetEnd().Printf("INITCANO %d",staCannon);
}

void FsGroundProperty::SetState(FSGNDSTATE state)
{
	staState=state;
}

const YsVec3 FsGroundProperty::GetUserViewPoint(void) const
{
	switch(GetWeaponOfChoice())
	{
	default:
	case FSWEAPON_NULL:
		return chCockpit;
	case FSWEAPON_GUN:
		{
			YsVec3 off;
			staAaaAim.Mul(off,chAaaGunnerOffset);
			return off+chAaaGunnerPosition;
		}
	case FSWEAPON_AIM9:
	case FSWEAPON_AGM65:
		{
			YsVec3 off;
			staSamAim.Mul(off,chSamGunnerOffset);
			return off+chSamGunnerPosition;
		}
	}
}

YSBOOL FsGroundProperty::HasTurret(void) const
{
	if(chTurret.GetN()>0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSBOOL FsGroundProperty::GetHasAntiAirTurret(void) const
{
	return chHasAntiAirTurret;
}

YSBOOL FsGroundProperty::GetHasAntiGroundTurret(void) const
{
	return chHasAntiGndTurret;
}

YSBOOL FsGroundProperty::GetHasPilotControlledTurret(void) const
{
	return chHasPilotControlledTurret;
}

YSBOOL FsGroundProperty::GetHasGunnerControlledTurret(void) const
{
	return chHasGunnerControlledTurret;
}

YSRESULT FsGroundProperty::GetFirstPilotControlledTurretDirection(YsVec3 &dir) const
{
	int i;
	for(i=0; i<chTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			YsAtt3 att;

			att.Set(staTurret[i].h,staTurret[i].p,0.0);
			dir=att.GetForwardVector();

			chTurret[i].att.Mul(dir,dir);

			staMatrix.Mul(dir,dir,0.0);
			return YSOK;
		}
	}
	return YSERR;
}

YSBOOL FsGroundProperty::IsFiringAaa(void) const
{
	return staFiringAaa;
}

void FsGroundProperty::StartFiringAaa(void)
{
	staFiringAaa=YSTRUE;
}

void FsGroundProperty::StopFiringAaa(void)
{
	staFiringAaa=YSFALSE;
}

YSBOOL FsGroundProperty::IsFiringCannon(void) const
{
	return staFiringCannon;
}

void FsGroundProperty::StartFiringCannon(void)
{
	staFiringCannon=YSTRUE;
}

void FsGroundProperty::StopFiringCannon(void)
{
	staFiringCannon=YSFALSE;
}

YSRESULT FsGroundProperty::GetFirstPilotControlledTurretPosition(YsVec3 &pos) const
{
	int i;
	for(i=0; i<chTurret.GetN(); i++)
	{
		if(chTurret[i].controlledBy==FSTURRET_CTRL_BY_PILOT)
		{
			staMatrix.Mul(pos,chTurret[i].cen,1.0);
			return YSOK;
		}
	}
	return YSERR;
}


// The command "CARRIER" is not listed here.
// It is processed in the LoadProperty function.
// Means "CARRIER" cannot be used except loading template.
const char *const FsGroundProperty::keyWordSource[]=
{
	"POSITION", //X Y Z [M][IN]
	"ATTITUDE", //H P B [DEG][RAD]
	"HTRADIUS",
	"STRENGTH",
	"INITIGUN",
	"INITISAM",
	"MAXSPEED",
	"MAXROTAT",
	"MINAIMPI",
	"MAXAIMPI",
	"IDENTIFY",
	"AIMROTAT",
	"AIMPITCH",
	"GUNRANGE",
	"SAMRANGE",

	"MINIDAMG",

	"ANTIGRND",

	"CONSTHDG",
	"CONSTPCH",
	"CONSTBNK",

	"HUGROUND",

	"VORRANGE",

	"CANRANGE",
	"INITCANO",

	"GRNDTYPE",
	"GAMEOBJE",

	"AAAMOUNT",
	"SAMMOUNT",
	"CANMOUNT",

	"AAAMAXPI",
	"AAAMINPI",
	"SAMMAXPI",
	"SAMMINPI",
	"CANMAXPI",
	"CANMINPI",

	"CANFLOAT",
	"STYUPRIT",

	"NOSHADOW",

	"USEAIMAT",

	"ASMTARGT",  // 2006/06/01
	"TRACKAIR",  // 2006/06/01
	"TARGTANY",  // 2006/06/01

	"VISLDAID",  // 2006/06/07

	"DISTMESR",  // 2006/07/18
	"SAMRLOAD",  // 2006/07/18

	"NDBRANGE",  // 2007/08/03

	"SUBSTNAM",  // 2007/09/16  46

	// 2007/09/21
	"NMTURRET",  // 47 Number of turret
	"TURRETPO",  // 48 0 0m -0.8m 2.7m 0deg 0deg 0deg      # Number x y z h p b
	"TURRETPT",  // 49 0 -40deg 0deg 0deg                  # Number MinPitch MaxPitch NeutralPitch
	"TURRETHD",  // 50 0 -120deg 120deg 0deg               # Number MinHdg MaxHdg NeutralHdg
	"TURRETAM",  // 51 0 0                                 # Ammo(zero -> staGunBullet will be used)
	"TURRETIV",  // 52 0 0.5sec                            # Number ShootingInterval
	"TURRETNM",  // 53 0 GUN                               # DNM Node Name
	"TURRETAR",  // 54 0 FALSE                             # TRUE -> Anti Air Capable
	"TURRETGD",  // 55 0 TRUE                              # TRUE -> Anti Ground Capable
	"TURRETCT",  // 56 "PILOT" or "GUNNER"
	"TURRETRG",  // 57 Range
	"TURRETNH",  // 58 DNM Node Name (Heading Rotation)
	"TURRETNP",  // 59 DNM Node Name (Pitch Rotation)

	// 2007/09/22
	"TURRETPW",  // 60 Turret Power
	"TURRETWP",  // 61 Weapon Type

	// 2009/03/29
	"SPLYFUEL",  // 62 TRUE/FALSE Supply Fuel
	"SPLYAMMO",  // 63 TRUE/FALSE Supply Ammo

	// 2011/08/05
	"CANMOVON",  // 64 ANYWHERE, WATER, GROUND, PAVED, REGION
	"CANMOVRG",  // 65 Region ID
	"USERCTRL",  // 66 NONE or Combination of DRIVE,GUN, and MISSILE
	"COCKPITP",  // 67 x y z
	"EXCAMERA",  // 68 Extra Camera

	// 2011/12/20
	"MAXSPDRV",  // 69 Maximum Speed Reverse.  Need to be positive to be able to revrse.
	"MANESPD1",  // 70 Normally maneuverable speed
	"MANESPD2",  // 71 Start drifting speed
	"MAXACCEL",  // 72 Maximum acceleration m/ss
	"MAXAXLRV",  // 73 Maximum acceleration in reverse.  Need to be positive to be able to revrse.
	"MAXBRAKE",  // 74 Maximum brake acceleration m/ss
	"MANESPD3",  // 75 Vehicle drifts with slightest stearing

	// 2011/12/25
	"LOOKOFST",  // 76 Look-at Offset

	// 2011/12/27
	"NOGNDCOL",  // 77 Skip Ground to ground collision

	// 2012/01/02
	"AAAGUNNR",  // 78 AAA Gunner Position
	"SAMGUNNR",  // 79 SAM Gunner Position
	"CANGUNNR",  // 80 Cannon Gunner Position
	"AAAOFFST",  // 81 AAA Gunner Offset
	"SAMOFFST",  // 82 AAA Gunner Offset
	"CANOFFST",  // 83 AAA Gunner Offset

	// 2012/01/05 (In American Airline 5)
	"AAAPOSIT",  // 84 AAA Position relative to the turret
	"SAMPOSIT",  // 85 SAM Position relative to the turret
	"CANPOSIT",  // 86 Cannon Position relative to the turret
	"SYNCAASA",  // 87 Synchronize AAA and SAM aim

	// 2012/01/14
	"AAAINTVL",  // 88 AAA Interval

	// 2012/01/16
	"GUNPRECI",  // 89 Gun Precision
	"MSSLTYPE",  // 90 Missile Type

	// 2017/12/08
	"RACECHKP",  // 91 Is racing check point;

	NULL
};

YsKeyWordList FsGroundProperty::keyWordList;

YSRESULT FsGroundProperty::SendCommand(const char in[])
{
	YsString buf(in);
	YsArray <YsString,16> args;

	if(NULL!=in && '#'==in[0])
	{
		return YSOK;
	}

	if(YSOK==buf.Arguments(args) && 0<args.GetN())
	{
		int i,turretId;
		YSBOOL boo;

		if(keyWordList.GetN()==0)
		{
			keyWordList.MakeList(keyWordSource);
		}


		int cmd;
		YSRESULT res;

		if(args[0][0]=='*')
		{
			cmd=atoi(((const char *)(args[0]))+1);
		}
		else
		{
			cmd=keyWordList.GetId(args[0]);
		}

		if(cmd>=0)
		{
			res=YSERR;
			switch(cmd)
			{
			case 0: //"POSITION", //X Y Z [M][IN]
				res=FsGetVec3(staPosition,args.GetN()-1,args.GetArray()+1);
				RemakeMatrix();
				break;
			case 1: //"ATTITUDE", //H P B [DEG][RAD]
				res=FsGetAtt3(staAttitude,args.GetN()-1,args.GetArray()+1);
				RemakeMatrix();
				break;
			case 2: //"HTRADIUS",
				res=FsGetLength(chOutsideRadius,args[1]);
				break;
			case 3: //"STRENGTH",
				res=YSOK;
				staDamageTolerance=atoi(args[1]);
				break;
			case 4: //"INITIGUN",
				staGunBullet=atoi(args[1]);
				res=YSOK;
				break;
			case 5: //"INITISAM",
				staSAM=atoi(args[1]);
				res=YSOK;
				break;
			case 6: //"MAXSPEED",
				res=FsGetSpeed(chMaxSpeed,args[1]);
				break;
			case 7: //"MAXROTAT",
				res=FsGetAngle(chMaxRotation,args[1]);
				break;
			case 8: //"MINAIMPI",
				res=FsGetAngle(chAaaMinAimPitch,args[1]);
				chSamMinAimPitch=chAaaMinAimPitch;
				chCanMinAimPitch=chAaaMinAimPitch;
				break;
			case 9: //"MAXAIMPI",
				res=FsGetAngle(chAaaMaxAimPitch,args[1]);
				chSamMaxAimPitch=chAaaMaxAimPitch;
				chCanMaxAimPitch=chAaaMaxAimPitch;
				break;
			case 10: // "IDENTIFY"
				chIdName.Set(args[1]);
				for(i=0; chIdName[i]!=0; i++)
				{
					if(chIdName[i]==' ' || chIdName[i]=='\t' || chIdName[i]=='\"')
					{
						chIdName.Set(i,'_');
					}
				}
				chIdName.Capitalize();
				res=YSOK;
				break;
			case 11: // "AIMROTATE"
				res=FsGetAngle(chAimRotationSpeed,args[1]);
				break;
			case 12: // "AIMPITCH"
				res=FsGetAngle(chAimPitchSpeed,args[1]);
				break;
			case 13: // "GUNRANGE"
				res=FsGetLength(chAaaRange,args[1]);
				break;
			case 14: // "SAMRANGE"
				res=FsGetLength(chSAMRange,args[1]);
				break;

			case 15:
				chMinimumDamage=atoi(args[1]);
				res=YSOK;
				break;

			case 16:
				res=FsGetBool(boo,args[1]);
				if(boo==YSTRUE)
				{
					chFlags|=YSGP_ANTIGROUND;
				}
				else
				{
					chFlags&=~YSGP_ANTIGROUND;
				}
				break;

			case 17: //	"CONSTHDG",
				res=FsGetAngle(chConstHeadingRotation,args[1]);
				break;
			case 18: // "CONSTPCH",
				res=FsGetAngle(chConstPitchRotation,args[1]);
				break;
			case 19: //"CONSTBNK",
				res=FsGetAngle(chConstBankRotation,args[1]);
				break;

			case 20: //	"HUGROUND",
				res=FsGetBool(boo,args[1]);
				if(boo==YSTRUE)
				{
					chFlags|=YSGP_HUGGROUND;
				}
				else
				{
					chFlags&=~YSGP_HUGGROUND;
				}
				break;

			case 21: // "VORRANGE" // 20040819
				res=FsGetLength(chVorRange,args[1]);
				break;

			case 22: // "CANRANGE", 20050202
				res=FsGetLength(chCannonRange,args[1]);
				break;
			case 23: // "INITCANO", 20050202
				staCannon=atoi(args[1]);
				res=YSOK;
				break;

			case 24: // "GRNDTYPE", 20050205
				res=YSOK;
				args[1].Capitalize();
				if(strcmp(args[1],"STATIC")==0)
				{
					chType=FSSTATIC;
				}
				else if(strcmp(args[1],"VEHICLE")==0)
				{
					chType=FSVEHICLE;
				}
				else if(strcmp(args[1],"TANK")==0)
				{
					chType=FSTANK;
				}
				else if(strcmp(args[1],"SHIP")==0)
				{
					chType=FSSHIP;
				}
				else if(strcmp(args[1],"NAVYSHIP")==0)
				{
					chType=FSNAVYSHIP;
				}
				else
				{
					res=YSERR;
				}
				break;
			case 25: // "GAMEOBJE", 20050206  Not to show trees on radar
				res=FsGetBool(boo,args[1]);
				if(boo==YSFALSE)
				{
					chFlags|=YSGP_NONEGAMEOBJECT;
				}
				else
				{
					chFlags&=~YSGP_NONEGAMEOBJECT;
				}
				break;
			case 26:// "AAAMOUNT",
				res=FsGetVec3(chAaaMount,args.GetN()-1,args.GetArray()+1);
				break;
			case 27:// "SAMMOUNT",
				res=FsGetVec3(chSamMount,args.GetN()-1,args.GetArray()+1);
				break;
			case 28:// "CANMOUNT",
				res=FsGetVec3(chCanMount,args.GetN()-1,args.GetArray()+1);
				break;

			case 29: // "AAAMAXPI",
				res=FsGetAngle(chAaaMaxAimPitch,args[1]);
				break;
			case 30: // "AAAMINPI",
				res=FsGetAngle(chAaaMinAimPitch,args[1]);
				break;
			case 31: // "SAMMAXPI",
				res=FsGetAngle(chSamMaxAimPitch,args[1]);
				break;
			case 32: // "SAMMINPI",
				res=FsGetAngle(chSamMinAimPitch,args[1]);
				break;
			case 33: // "CANMAXPI",
				res=FsGetAngle(chCanMaxAimPitch,args[1]);
				break;
			case 34: // "CANMINPI",
				res=FsGetAngle(chCanMinAimPitch,args[1]);
				break;

			case 35: // "CANFLOAT",
				res=FsGetBool(boo,args[1]);
				if(boo==YSTRUE)
				{
					chFlags|=YSGP_CANFLOAT;
				}
				else
				{
					chFlags&=~YSGP_CANFLOAT;
				}
				break;
			case 36: // "STYUPRIT",
				res=FsGetBool(boo,args[1]);
				if(boo==YSTRUE)
				{
					chFlags|=YSGP_STAYUPRIGHT;
				}
				else
				{
					chFlags&=~YSGP_STAYUPRIGHT;
				}
				break;

			case 37: // "NOSHADOW",
				res=FsGetBool(boo,args[1]);
				if(boo==YSTRUE)
				{
					chFlags|=YSGP_NOSHADOW;
				}
				else
				{
					chFlags&=~YSGP_NOSHADOW;
				}
				break;

			case 38: //  "USEAIMAT",
				res=FsGetVec3(chAimAt,args.GetN()-1,args.GetArray()+1);
				chFlags|=YSGP_USEAIMAT;
				break;

			case 39: //	"ASMTARGT",  // 2006/06/01
				chFlags|=YSGP_ASMTARGET;
				res=YSOK;
				break;
			case 40: //"TRACKAIR",  // 2006/06/01
				chFlags|=YSGP_TRACKAIRTARGET;
				res=YSOK;
				break;
			case 41: //"TARGTANY",  // 2006/06/01
				chFlags|=YSGP_TARGETANY;
				res=YSOK;
				break;

			case 42: // "VISLDAID"  // 2006/06/07
				if(2<=args.GetN())
				{
					args[1].Capitalize();
					if(strcmp(args[1],"NONE")==0)
					{
						chIsVisualLandingAid=YSGP_NOTVISUALLANDINGAID;
						res=YSOK;
					}
					if(strcmp(args[1],"PAPI")==0)
					{
						chIsVisualLandingAid=YSGP_PAPI;
						res=YSOK;
					}
					if(strcmp(args[1],"VASI")==0)
					{
						chIsVisualLandingAid=YSGP_VASI;
						res=YSOK;
					}
				}
				break;
			case 43: // "DISTMESR",  // 2006/07/18
				if(2<=args.GetN())
				{
					res=FsGetBool(chIsDme,args[1]);
				}
				break;
			case 44: // "SAMRLOAD",  // 2006/07/18
				if(3<=args.GetN())
				{
					chSamReloadMax=atoi(args[1]);
					chSamReloadTime=atof(args[2]);
					staSamReloadCount=chSamReloadMax;
					res=YSOK;
				}
				break;

			case 45: // "NDBRANGE" // 20070803
				res=FsGetLength(chNdbRange,args[1]);
				break;

			case 46: // "SUBSTNAM" 20070916
				if(2<=args.GetN())
				{
					chSubstIdName.Set(args[1]);
					res=YSOK;
				}
				else
				{
					res=YSERR;
				}
				break;


			case 47: // "NMTURRET",  // Number of turrets
				if(args.GetN()>=2)
				{
					int i,n;
					n=atoi(args[1]);
					chTurret.Set(n,NULL);
					staTurret.Set(n,NULL);
					for(i=0; i<n; i++)
					{
						chTurret[i].Initialize();
						staTurret[i].Initialize();
						staSavedTurret.Set(0,NULL);
					}
					res=YSOK;
				}
				break;
			case 48: // "TURRETPO",  //   Number x y z h p b
				if(args.GetN()>=8)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && 
					   turretId<chTurret.GetN() &&
					   FsGetVec3(chTurret[turretId].cen,args.GetN()-2,args.GetArray()+2)==YSOK &&
					   FsGetAtt3(chTurret[turretId].att,args.GetN()-5,args.GetArray()+5)==YSOK)
					{
						res=YSOK;
					}
				}
				break;
			case 49: // "TURRETPT",  //   Number MinPitch MaxPitch NeutralPitch
				if(args.GetN()>=5)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && 
					   turretId<chTurret.GetN() &&
					   FsGetAngle(chTurret[turretId].pMin,args[2])==YSOK &&
					   FsGetAngle(chTurret[turretId].pMax,args[3])==YSOK &&
					   FsGetAngle(chTurret[turretId].pZero,args[4])==YSOK)
					{
						res=YSOK;
					}
				}
				break;
			case 50: // "TURRETHD",  //   Number MinHdg MaxHdg NeutralHdg
				if(args.GetN()>=5)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && 
					   turretId<chTurret.GetN() &&
					   FsGetAngle(chTurret[turretId].hMin,args[2])==YSOK &&
					   FsGetAngle(chTurret[turretId].hMax,args[3])==YSOK &&
					   FsGetAngle(chTurret[turretId].hZero,args[4])==YSOK)
					{
						res=YSOK;
					}
				}
				break;
			case 51: // "TURRETAM",  //   Ammo(zero -> staGunBullet will be used)
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						int n;
						n=atoi(args[2]);
						chTurret[turretId].maxNumGunBullet=n;
						staTurret[turretId].numBullet=n;
						res=YSOK;
					}
				}
				break;
			case 52: // "TURRETIV",  //   Number ShootingInterval
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].shootInterval=atof(args[2]);
						res=YSOK;
					}
				}
				break;
			case 53: // "TURRETNM",  //   DNM Node Name
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].dnmNodeName.Set(args[2]);
						res=YSOK;
					}
				}
				break;
			case 54: // "TURRETAR",  //   Anti Air Capable
				if(args.GetN()>=2)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						res=YSOK;
						chTurret[turretId].turretProp|=FSTURRETPROP_ANTIAIR;
						chHasAntiAirTurret=YSTRUE;
					}
				}
				break;
			case 55: // "TURRETGD",  //   Anti Ground Capable
				if(args.GetN()>=2)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						res=YSOK;
						chTurret[turretId].turretProp|=FSTURRETPROP_ANTIGND;
						chHasAntiGndTurret=YSTRUE;
					}
				}
				break;
			case 56: // "TURRETCT",  //  "PILOT" or "GUNNER"
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						if(strcmp(args[2],"PILOT")==0)
						{
							chTurret[turretId].controlledBy=FSTURRET_CTRL_BY_PILOT;
							chHasPilotControlledTurret=YSTRUE;
							res=YSOK;
						}
						else if(strcmp(args[2],"GUNNER")==0)
						{
							chTurret[turretId].controlledBy=FSTURRET_CTRL_BY_GUNNER;
							chHasGunnerControlledTurret=YSTRUE;
							res=YSOK;
						}
					}
				}
				break;
			case 57: // "TURRETRG"
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						res=FsGetLength(chTurret[turretId].range,args[2]);
					}
				}
				break;

			case 58: // "TURRETNH",  // DNM Node Name (Heading Rotation)
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].dnmHdgNodeName.Set(args[2]);
						res=YSOK;
					}
				}
				break;
			case 59: // "TURRETNP",  // DNM Node Name (Pitch Rotation)
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					if(0<=turretId && turretId<chTurret.GetN())
					{
						chTurret[turretId].dnmPchNodeName.Set(args[2]);
						res=YSOK;
					}
				}
				break;
			case 60: // "TURRETPW",
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					chTurret[turretId].destructivePower=atoi(args[2]);
					res=YSOK;
				}
				break;
			case 61: // "TURRETWP",  // 61 Weapon Type
				if(args.GetN()>=3)
				{
					turretId=atoi(args[1]);
					chTurret[turretId].wpnType=FsGetWeaponTypeByString(args[2]);
					res=YSOK;
				}
				break;
			case 62: // "SPLYFUEL",  // 62 TRUE/FALSE Supply Fuel
				if(2<=args.GetN())
				{
					YSBOOL boo;
					res=FsGetBool(boo,args[1]);
					if(boo==YSTRUE)
					{
						chFlags|=YSGP_SUPPLYFUEL;
					}
					else
					{
						chFlags&=(~YSGP_SUPPLYFUEL);
					}
				}
				break;
			case 63: // "SPLYAMMO",  // 63 TRUE/FALSE Supply Ammo
				if(2<=args.GetN())
				{
					YSBOOL boo;
					res=FsGetBool(boo,args[1]);
					if(boo==YSTRUE)
					{
						chFlags|=YSGP_SUPPLYAMMO;
					}
					else
					{
						chFlags&=(~YSGP_SUPPLYAMMO);
					}
				}
				break;

			case 64: // "CANMOVON",  // 64 ANYWHERE, WATER, GROUND, PAVED, REGION
				if(2<=args.GetN())
				{
					args[1].Capitalize();
					if(0==strcmp(args[1],"ANYWHERE"))
					{
						chCanMoveArea=CANMOVE_ANYWHERE;
						res=YSOK;
					}
					else if(0==strcmp(args[1],"WATER"))
					{
						chCanMoveArea=CANMOVE_WATER;
						res=YSOK;
					}
					else if(0==strcmp(args[1],"GROUND"))
					{
						chCanMoveArea=CANMOVE_GROUND;
						res=YSOK;
					}
					else if(0==strcmp(args[1],"PAVED"))
					{
						chCanMoveArea=CANMOVE_PAVED;
						res=YSOK;
					}
					else if(0==strcmp(args[1],"REGION"))
					{
						chCanMoveArea=CANMOVE_SPECIFICREGION;
						res=YSOK;
					}
				}
				break;

			case 65: // "CANMOVRG",  // 65 Region ID
				if(2<=args.GetN())
				{
					chCanMoveRegionId=atoi(args[1]);
					res=YSOK;
				}
				break;

			case 66: // "USERCTRL" 66 NONE or Combination of DRIVE,GUN, and MISSILE
				{
					chUserControlLevel=0;

					int i;
					for(i=1; i<args.GetN(); i++)
					{
						args[i].Capitalize();
						if(0==strcmp(args[i],"NONE"))
						{
							chUserControlLevel=0;
							break;
						}
						else if(0==strcmp(args[i],"DRIVE"))
						{
							chUserControlLevel|=USERCTL_DRIVE;
						}
						else if(0==strcmp(args[i],"GUN"))
						{
							chUserControlLevel|=USERCTL_GUN;
						}
						else if(0==strcmp(args[i],"SAM"))
						{
							chUserControlLevel|=USERCTL_SAM;
						}
						else if(0==strcmp(args[i],"ATM"))
						{
							chUserControlLevel|=USERCTL_ATM;
						}
						else if(0==strcmp(args[i],"CANNON"))
						{
							chUserControlLevel|=USERCTL_CANNON;
						}
					}
					res=YSOK;
				}
				break;
			case 67: // "COCKPITP",  // 67 x y z
				res=FsGetVec3(chCockpit,args.GetN()-1,args.GetArray()+1);
				break;
			case 68: // "EXCAMERA",  // 68 Extra Camera
				if(args.GetN()>=9)
				{
					FsAdditionalViewpoint view;
					view.name.Set(args[1]);
					if(FsGetVec3(view.pos,args.GetN()-2,args.GetArray()+2)==YSOK &&
					   FsGetAtt3(view.att,args.GetN()-5,args.GetArray()+5)==YSOK)
					{
						if(strcmp(args[8],"INSIDE")==0)
						{
							view.vpType=FS_ADVW_INSIDE;
							res=YSOK;
						}
						else if(strcmp(args[8],"OUTSIDE")==0)
						{
							view.vpType=FS_ADVW_OUTSIDE;
							res=YSOK;
						}
						else if(strcmp(args[8],"CABIN")==0)
						{
							view.vpType=FS_ADVW_CABIN;
							res=YSOK;
						}
						chExtraView.Append(view);
					}
				}
				break;
			case 69: // "MAXSPDRV",  // 69 Maximum Speed Reverse.  Non zero -> Can reverse.
				if(2<=args.GetN())
				{
					res=FsGetSpeed(chMaxSpeedRev,args[1]);
				}
				break;
			case 70: // "MANESPD1",  // 70 Normally maneuverable speed
				if(2<=args.GetN())
				{
					res=FsGetSpeed(chManSpeed1,args[1]);
				}
				break;
			case 71: // "MANESPD2",  // 71 Start drifting speed
				if(2<=args.GetN())
				{
					res=FsGetSpeed(chManSpeed2,args[1]);
				}
				break;
			case 72: // "MAXACCEL",  // 72 Maximum acceleration m/ss
				if(2<=args.GetN())
				{
					res=FsGetSpeed(chMaxAccel,args[1]);
				}
				break;
			case 73: // "MAXAXLRV",  // 73 Maximum acceleration in reverse.  Need to be positive to be able to revrse.
				if(2<=args.GetN())
				{
					res=FsGetSpeed(chMaxAccelRev,args[1]);
				}
				break;
			case 74: // "MAXBRAKE",  // 74 Maximum brake acceleration m/ss
				if(2<=args.GetN())
				{
					res=FsGetSpeed(chMaxBrake,args[1]);
				}
				break;
			case 75: // "MANESPD3",  // 75 Start drifting speed
				if(2<=args.GetN())
				{
					res=FsGetSpeed(chManSpeed3,args[1]);
				}
				break;
			case 76: // "LOOKOFST",  // 76 Look-at Offset
				if(4<=args.GetN())
				{
					res=FsGetVec3(chLookAt,args.GetN()-1,args.GetArray()+1);
				}
				break;
			case 77: // "NOGNDCOL",  // 77 Skip Ground to ground collision
				if(2<=args.GetN())
				{
					res=FsGetBool(chSkipGroundToGroundCollisionCheck,args[1]);
				}
				break;
			case 78: // "AAAGUNNR",  // 78 AAA Gunner Position
				if(4<=args.GetN())
				{
					res=FsGetVec3(chAaaGunnerPosition,args.GetN()-1,args.GetArray()+1);
				}
				break;
			case 79: // "SAMGUNNR",  // 79 SAM Gunner Position
				if(4<=args.GetN())
				{
					res=FsGetVec3(chSamGunnerPosition,args.GetN()-1,args.GetArray()+1);
				}
				break;
			case 80: //"CANGUNNR",  // 80 Cannon Gunner Position
				if(4<=args.GetN())
				{
					res=FsGetVec3(chCanGunnerPosition,args.GetN()-1,args.GetArray()+1);
				}
				break;
			case 81: // "AAAOFFST",  // 81 AAA Gunner Offset
				if(4<=args.GetN())
				{
					res=FsGetVec3(chAaaGunnerOffset,args.GetN()-1,args.GetArray()+1);
				}
				break;
			case 82: // "SAMOFFST",  // 82 AAA Gunner Offset
				if(4<=args.GetN())
				{
					res=FsGetVec3(chSamGunnerOffset,args.GetN()-1,args.GetArray()+1);
				}
				break;
			case 83: // "CANOFFST",  // 83 AAA Gunner Offset
				if(4<=args.GetN())
				{
					res=FsGetVec3(chCanGunnerOffset,args.GetN()-1,args.GetArray()+1);
				}
				break;
			case 84: // "AAAPOSIT",  // 84 AAA Position relative to the turret
				if(4<=args.GetN() && chNumAaaPosition<MAXNUMAAAPOSITION)
				{
					res=FsGetVec3(chAaaPosition[chNumAaaPosition],args.GetN()-1,args.GetArray()+1);
					chNumAaaPosition++;
				}
				break;
			case 85: // "SAMPOSIT",  // 85 SAM Position relative to the turret
				if(4<=args.GetN() && chNumSamPosition<MAXNUMSAMPOSITION)
				{
					res=FsGetVec3(chSamPosition[chNumSamPosition],args.GetN()-1,args.GetArray()+1);
					chNumSamPosition++;
				}
				break;
			case 86: // "CANPOSIT",  // 86 Cannon Position relative to the turret
				if(4<=args.GetN() && chNumCanPosition<MAXNUMCANPOSITION)
				{
					res=FsGetVec3(chCanPosition[chNumCanPosition],args.GetN()-1,args.GetArray()+1);
					chNumCanPosition++;
				}
				break;
			case 87: // "SYNCAASA",  // 87 Synchronize AAA and SAM aim
				if(2<=args.GetN())
				{
					res=FsGetBool(chSyncAaaSamAim,args[1]);
				}
				break;
			case 88: // "AAAINTVL",  // 88 AAA Interval
				if(2<=args.GetN())
				{
					chAaaInterval=atof(args[1]);
					res=YSOK;
				}
				break;
			case 89: // "GUNPRECI",  // 89 Gun Precision
				if(2<=args.GetN())
				{
					res=FsGetAngle(chGunPrecision,args[1]);
				}
				break;
			case 90: // "MSSLTYPE",  // 90 Missile Type
				if(2<=args.GetN())
				{
					FSWEAPONTYPE wpnType=FsGetWeaponTypeByString(args[1]);
					if(FSWEAPON_NULL!=wpnType)
					{
						chSAMType=wpnType;
						res=YSOK;
					}
				}
				break;
			case 91: // "RACECHKP"
				if(2<=args.GetN())
				{
					res=FsGetBool(chIsRacingCheckPoint,args[1]);
				}
				break;
			}
			if(res!=YSOK)
			{
				fsStderr.Printf("Error:%s\n",in);
			}
			return res;
		}
		else
		{
			fsStderr.Printf("Error:%s\n",in);
			return YSERR;
		}
	}
	// Ignore Blank line
	return YSOK;
}

YSRESULT FsGroundProperty::LoadAircraftCarrierProperty(const wchar_t tmplRootDir[],const wchar_t fn[])
{
	FsAircraftCarrierProperty *acp;
	acp=new FsAircraftCarrierProperty;
	if(acp!=NULL)
	{
		if(acp->LoadAircraftCarrier(tmplRootDir,fn)==YSOK)
		{
			isAircraftCarrier=YSTRUE;
			aircraftCarrierProperty=acp;
			acp->belongTo=this;
			return YSOK;
		}
		delete acp;
	}
	return YSERR;
}

FsAircraftCarrierProperty *FsGroundProperty::GetAircraftCarrierProperty(void)
{
	if(isAircraftCarrier==YSTRUE)
	{
		return aircraftCarrierProperty;
	}
	return NULL;
}

const FsAircraftCarrierProperty *FsGroundProperty::GetAircraftCarrierProperty(void) const
{
	if(isAircraftCarrier==YSTRUE)
	{
		return aircraftCarrierProperty;
	}
	return NULL;
}

unsigned FsGroundProperty::NetworkEncode(unsigned char dat[],int idOnSvr,const double &currentTime,YSBOOL shortFormat)
{
	unsigned char *ptr;
	unsigned short flags;

	ptr=dat;
	FsPushInt(ptr,FSNETCMD_GROUNDSTATE);             // 4 bytes (Total  4 bytes)
	FsPushFloat(ptr,(float)currentTime);             // 4 bytes (Total  8 bytes)
	FsPushInt  (ptr,idOnSvr);                        // 4 bytes (Total 12 bytes)
	FsPushShort(ptr,(short)staDamageTolerance);     // 2 bytes (Total 14 bytes)

	if(shortFormat==YSTRUE)
	{
		FsPushShort(ptr,1);                          // 2 bytes (Total 16 bytes)

		short h,p,b;
		h=(short)(staAttitude.h()*32768.0/YsPi);
		p=(short)(staAttitude.p()*32768.0/YsPi);
		b=(short)(staAttitude.b()*32768.0/YsPi);

		FsPushFloat(ptr,(float)staPosition.x());    // 4 bytes (Total 20 bytes)
		FsPushFloat(ptr,(float)staPosition.y());    // 4 bytes (Total 24 bytes)
		FsPushFloat(ptr,(float)staPosition.z());    // 4 bytes (Total 28 bytes)
		FsPushShort(ptr,h);                          // 2 bytes (Total 30 bytes)
		FsPushShort(ptr,p);                          // 2 bytes (Total 32 bytes)
		FsPushShort(ptr,b);                          // 2 bytes (Total 34 bytes)

		flags=0;
		if(staFiringAaa==YSTRUE)
		{
			flags|=1;
		}
		if(staFiringCannon==YSTRUE)
		{
			flags|=2;
		}
		FsPushUnsignedShort(ptr,flags);              // 2 bytes (Total 36 bytes)
		// << 36 bytes

		if(staSpeed!=YsOrigin() || fabs(staRotation)>YsTolerance)
		{
			int vx,vy,vz,r;
			vx=(short)(staSpeed.x()*10.0);
			vy=(short)(staSpeed.y()*10.0);
			vz=(short)(staSpeed.z()*10.0);
			r=(short)(staRotation*32768.0/YsPi);
			r=YsBound(r,-32767,32767);

			FsPushShort(ptr,(short)vx);              // 2 bytes (Total 38 bytes)
			FsPushShort(ptr,(short)vy);              // 2 bytes (Total 40 bytes)
			FsPushShort(ptr,(short)vz);              // 2 bytes (Total 42 bytes)
			FsPushShort(ptr,(short)r);               // 2 bytes (Total 44 bytes)
			// << 44 bytes
		}
	}
	else
	{
		FsPushShort(ptr,0);                          // 2 bytes (Total 16 bytes)
		FsPushFloat(ptr,(float)staPosition.x());    // 4 bytes (Total 20 bytes)
		FsPushFloat(ptr,(float)staPosition.y());    // 4 bytes (Total 24 bytes)
		FsPushFloat(ptr,(float)staPosition.z());    // 4 bytes (Total 28 bytes)
		FsPushFloat(ptr,(float)staAttitude.h());    // 4 bytes (Total 32 bytes)
		FsPushFloat(ptr,(float)staAttitude.p());    // 4 bytes (Total 36 bytes)
		FsPushFloat(ptr,(float)staAttitude.b());    // 4 bytes (Total 40 bytes)

		flags=0;
		if(staFiringAaa==YSTRUE)
		{
			flags|=1;
		}
		if(staFiringCannon==YSTRUE)
		{
			flags|=2;
		}

		FsPushUnsignedShort(ptr,flags);              // 2 bytes (Total 42 bytes)
		// << 42 bytes (Net Version 20040207)

		if(staSpeed!=YsOrigin() || fabs(staRotation)>YsTolerance)
		{
			FsPushFloat(ptr,(float)staSpeed.x());   // 4 bytes (Total 46 bytes)
			FsPushFloat(ptr,(float)staSpeed.y());   // 4 bytes (Total 50 bytes)
			FsPushFloat(ptr,(float)staSpeed.z());   // 4 bytes (Total 54 bytes)
			FsPushFloat(ptr,(float)staRotation);    // 4 bytes (Total 58 bytes)
			// << 58 bytes
		}
	}

	return (unsigned int)(ptr-dat);
}

void FsGroundProperty::NetworkDecode(
    unsigned int packetLength,unsigned char dat[],double &t0,double &w0,const double &w)
{
	double x,y,z,h,p,b;
	double t,fra,frb;
	unsigned short flag;

	const unsigned char *ptr=dat;
	FsPopInt  (ptr);   // FSNETCMD_GROUNDSTATE  skip
	t=(double)FsPopFloat(ptr);


	fra=w-w0;
	frb=t-t0;
	if(t0<0.0 || (frb>YsTolerance && 0.8<fra/frb && fra/frb<1.2))
	    // Eliminating delayed-packet
	    // Give negative t0 if it is a first position-notification
	    // See  ../document/netsynchro.txt  for more info.
	{
		short version;

		FsPopInt(ptr); // Skip idOnSvr
		staDamageTolerance=FsPopShort(ptr);
		version=FsPopShort(ptr);

		if(version==1)  // Short Format
		{
			x=FsPopFloat(ptr);
			y=FsPopFloat(ptr);
			z=FsPopFloat(ptr);
			h=(double)FsPopShort(ptr)*YsPi/32768.0;
			p=(double)FsPopShort(ptr)*YsPi/32768.0;
			b=(double)FsPopShort(ptr)*YsPi/32768.0;

			flag=FsPopUnsignedShort(ptr);

			staPosition.Set(x,y,z);
			staAttitude.Set(h,p,b);
			if((flag&1)!=0)
			{
				staFiringAaa=YSTRUE;
			}
			else
			{
				staFiringAaa=YSFALSE;
			}

			if((flag&2)!=0)
			{
				staFiringCannon=YSTRUE;
			}
			else
			{
				staFiringCannon=YSFALSE;
			}

			t0=t;
			w0=w;

			if(packetLength>=44)
			{
				double vx,vy,vz,rot;
				vx=(double)FsPopShort(ptr)/10.0;
				vy=(double)FsPopShort(ptr)/10.0;
				vz=(double)FsPopShort(ptr)/10.0;
				rot=(double)FsPopShort(ptr)*YsPi/32768.0;
				staSpeed.Set(vx,vy,vz);
				staRotation=rot;
			}
			else
			{
				staSpeed=YsOrigin();
				staRotation=0.0;
			}
		}
		else
		{
			x=FsPopFloat(ptr);
			y=FsPopFloat(ptr);
			z=FsPopFloat(ptr);
			h=FsPopFloat(ptr);
			p=FsPopFloat(ptr);
			b=FsPopFloat(ptr);
			flag=FsPopUnsignedShort(ptr);

			staPosition.Set(x,y,z);
			staAttitude.Set(h,p,b);
			if((flag&1)!=0)
			{
				staFiringAaa=YSTRUE;
			}
			else
			{
				staFiringAaa=YSFALSE;
			}

			if((flag&2)!=0)
			{
				staFiringCannon=YSTRUE;
			}
			else
			{
				staFiringCannon=YSFALSE;
			}

			t0=t;
			w0=w;

			if(packetLength>=58)
			{
				float vx,vy,vz,rot;
				vx=FsPopFloat(ptr);
				vy=FsPopFloat(ptr);
				vz=FsPopFloat(ptr);
				rot=FsPopFloat(ptr);
				staSpeed.Set(vx,vy,vz);
				staRotation=rot;
			}
			else
			{
				staSpeed=YsOrigin();
				staRotation=0.0;
			}
		}
	}
}

unsigned FsGroundProperty::EncodeTurretState(unsigned char dat[],int idOnSvr) const
{
	const int netCmd=FSNETCMD_GNDTURRETSTATE;
	return FsVehicleProperty::EncodeTurretState(dat,idOnSvr,netCmd);
}

void FsGroundProperty::ToggleLight(void)
{
	staLightState^=(FSLIGHT_GND_HEADLIGHT|FSLIGHT_GND_STROBE);
}

YSBOOL FsGroundProperty::HasWeapon(void) const
{
	if(0<staSAM || 0<staGunBullet || 0<staCannon)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

FSWEAPONTYPE FsGroundProperty::GetWeaponOfChoice(void) const
{
	return staWoc;
}

int FsGroundProperty::GetNumAaaBullet(void) const
{
	return staGunBullet;
}

int FsGroundProperty::GetNumCannon(void) const
{
	return staCannon;
}

int FsGroundProperty::GetNumSAM(void) const
{
	return staSAM;
}

void FsGroundProperty::SetNumAaaBullet(int numBullet)
{
	staGunBullet=numBullet;
}

const YsAtt3 &FsGroundProperty::GetCannonAim(void) const
{
	return staCanAim;
}

const YsAtt3 &FsGroundProperty::GetAaaAim(void) const
{
	return staAaaAim;
}

const YsAtt3 &FsGroundProperty::GetSamAim(void) const
{
	return staSamAim;
}

void FsGroundProperty::SetCannonAim(const YsAtt3 &aim)
{
	staCanAim=aim;
}

void FsGroundProperty::SetAaaAim(const YsAtt3 &aim)
{
	staAaaAim=aim;
}

void FsGroundProperty::SetSamAim(const YsAtt3 &aim)
{
	staSamAim=aim;
}

int FsGroundProperty::GetNumAaaPosition(void) const
{
	return (0<chNumAaaPosition ? chNumAaaPosition : 1);
}

int FsGroundProperty::GetNumSamPosition(void) const
{
	return (0<chNumSamPosition ? chNumSamPosition : 2);
}

int FsGroundProperty::GetNumCannonPosition(void) const
{
	return (0<chNumCanPosition ? chNumCanPosition : 1);
}

