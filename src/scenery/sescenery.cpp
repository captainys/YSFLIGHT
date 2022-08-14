#include <ysclass.h>
#include <ysport.h>

#include "ysscenery.h"
#include "sescenery.h"
#include "fsutil.h"



////////////////////////////////////////////////////////////

YsStartPos::YsStartPos()
{
}

YsStartPos::~YsStartPos()
{
}

const char *YsStartPos::GetName(void) const
{
	return nameStr.c_str();
}

const YsVec3 &YsStartPos::GetPosition(void) const
{
	return pos;
}

const YsAtt3 &YsStartPos::GetAttitude(void) const
{
	return att;
}

int YsStartPos::GetThrottle(void) const
{
	return throttle;
}

// int YsStartPos::GetFuel(void) const
// {
// 	return fuel;
// }

const double &YsStartPos::GetSpeed(void) const
{
	return speed;
}

YSBOOL YsStartPos::GetLandingGear(void) const
{
	return landingGear;
}

int YsStartPos::GetIFF(void) const
{
	return iff;
}

YSBOOL YsStartPos::GetOnCarrier(void) const
{
	return onCarrier;
}

const char *YsStartPos::GetCarrierTag(void) const
{
	return carrierTag;
}

const YsVec3 &YsStartPos::GetPosOnCarrier(void) const
{
	return posOnCarrier;
}

const YsAtt3 &YsStartPos::GetAttOnCarrier(void) const
{
	return attOnCarrier;
}

void YsStartPos::Initialize(void)
{
	nameStr="";
	pos=YsOrigin();
	att=YsZeroAtt();
	throttle=0;
	speed=0.0;
	// fuel=70;
	landingGear=YSTRUE;
	iff=IFF_ALL;
	extraCommand.clear();
	thisInTheList=NULL;

	onCarrier=YSFALSE;
	carrierTag.Set(1,"");
	posOnCarrier=YsOrigin();
	attOnCarrier=YsZeroAtt();
}

////////////////////////////////////////////////////////////

SeScenery::StopIncUndo::StopIncUndo(SeScenery &scn)
{
	incUndo=scn.PushStopIncUndo();
	this->scn=&scn;
}

SeScenery::StopIncUndo::~StopIncUndo()
{
	scn->PopIncUndo(incUndo);
}

YsListAllocator <YsSceneryGndObjTemplate> SeScenery::gndObjTemplateAllocator(16);
YsListAllocator <YsSceneryAirTemplate> SeScenery::airTemplateAllocator(16);

SeScenery::SeScenery() :
    searchItem(256), search2DElem(256),
    frozenMapList(drwAllocator),
    frozenSbdList(drwAllocator),
    frozenShlList(shlAllocator),
    frozenEvgList(evgAllocator),
    frozenScnList(scnAllocator),
    frozenRgnList(rgnAllocator),
    frozenGndList(gndAllocator),
    frozenAirList(airAllocator),
    frozenPstList(pstAllocator),
    gndObjTemplateList(gndObjTemplateAllocator),
    airTemplateList(airTemplateAllocator),
    stpList(stpAllocator),frozenStpList(stpAllocator)
{
	incUndo=1;
	undoCounter=0;
	undoPtr=NULL;
	redoPtr=NULL;
	undoLog=NULL;
	objType=YsSceneryItem::SUBSCENARY;

	modified=YSFALSE;
	stpIsModified=YSFALSE;
	lastOpenedFName="";
	lastOpenedStpFName="";

	searchKey=0xffff0000;
	searchItem.AddElement(searchKey,this);
}

void SeScenery::CleanUp(void)
{
	CleanUpUndoLog();
	Initialize();
	CleanUpSearchTable();
}

void SeScenery::SetIdName(const char nm[])  // Not an undo target
{
	YsScenery::idName.Set(nm);
}

YSRESULT SeScenery::LoadFld(const char fn[],YSBOOL autoLoadStp)
{
	CleanUpSearchTable();
	Initialize();
	CleanUpUndoLog();

	if(YsScenery::LoadFld(fn)==YSOK)
	{
		RefreshSearchTable();
		lastOpenedFName=fn;
		modified=YSFALSE;

		if(autoLoadStp==YSTRUE)
		{
			YsString stpFn;
			stpFn.Set(fn);
			stpFn.ReplaceExtension(".stp");
			LoadStp(stpFn);
		}

		return YSOK;
	}
	else
	{
		CleanUpSearchTable();
		Initialize();
		return YSERR;
	}
}

YSRESULT SeScenery::SaveFld(const char fn[])
{
	if(YsScenery::SaveFld(fn)==YSOK)
	{
		lastOpenedFName=fn;
		modified=YSFALSE;
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::LoadStp(const char fn[])
{
	FILE *fp=fopen(fn,"r");
	if(fp!=NULL)
	{
		YsTextFileInputStream inStream(fp);
		YSRESULT res=LoadStp(inStream);
		fclose(fp);
		return res;
	}
	return YSERR;
}

YSRESULT SeScenery::LoadStp(YsTextInputStream &inStream)
{
	CleanUpUndoLog();
	stpList.CleanUp();

	YsString str;
	YsStartPos *stp=NULL;
	while(NULL!=inStream.Gets(str))
	{
		str.DeleteTailSpace();
		if(str[0]=='N' || str[0]=='n')
		{
			YsArray <YsString,16> args;
			str.Arguments(args);
			stp=CreateStartPos();
			stp->nameStr=args[1];
		}
		else if((str[0]=='C' || str[0]=='c') && stp!=NULL)
		{
			char cmd[256];
			strncpy(cmd,str.Txt()+2,255);

			int ac;
			char *av[16];
			YsArguments(&ac,av,16,cmd);
			if(ac>0)
			{
				if(strcmp(av[0],"POSITION")==0)
				{
					FsGetVec3(stp->pos,3,av+1);
				}
				else if(strcmp(av[0],"ATTITUDE")==0)
				{
					FsGetAtt3(stp->att,3,av+1);
				}
				else if(strcmp(av[0],"INITSPED")==0)
				{
					FsGetSpeed(stp->speed,av[1]);
				}
				else if(strcmp(av[0],"CTLTHROT")==0)
				{
					double thr;
					thr=YsBound(atof(av[1]),0.0,1.0);
					stp->throttle=(int)(thr*100.0);
				}
				// else if(strcmp(av[0],"INITFUEL")==0)
				// {
				// 	int fuel;
				// 	fuel=atoi(av[1]);  // Assume %
				// 	stp->fuel=fuel;
				// }
				else if(strcmp(av[0],"CTLLDGEA")==0)
				{
					if(strcmp(av[1],"true")==0 || strcmp(av[1],"True")==0 || strcmp(av[1],"TRUE")==0)
					{
						stp->landingGear=YSTRUE;
					}
					else
					{
						stp->landingGear=YSFALSE;
					}
				}
				else
				{
					YsString extCmd;
					extCmd=str.substr(2);
					stp->extraCommand.push_back(extCmd);
					YsPrintf("ExtCmd: %s\n",str.substr(2).c_str());
				}
			}
		}
		else if((str[0]=='P' || str[0]=='p') && stp!=NULL)
		{
			char cmd[256];
			strncpy(cmd,str.Txt()+2,255);

			int ac;
			char *av[16];
			YsArguments(&ac,av,16,cmd);
			if(ac>0)
			{
				if(strcmp(av[0],"IFF")==0 && ac>=2)
				{
					stp->iff=atoi(av[1]);
				}
				if(strcmp(av[0],"CARRIER")==0 && ac>=8)
				   // Carrier Pos will overwrite C POSITION/C ATTITUDE after all the start positions are loaded.
				{
					stp->onCarrier=YSTRUE;
					stp->carrierTag.Set(strlen(av[1])+1,av[1]);
					FsGetVec3(stp->posOnCarrier,3,av+2);
					FsGetAtt3(stp->attOnCarrier,3,av+5);
				}
			}
		}
	}

	stp=NULL;
	while((stp=FindNextStartPos(stp))!=NULL)
	{
		if(stp->GetOnCarrier()==YSTRUE)
		{
			const YsSceneryGndObj *carrier;
			carrier=SearchGndObjByTag(stp->GetCarrierTag());
			if(carrier!=NULL)
			{
				YsMatrix4x4 tfm;
				YsVec3 p,uv,ev;

				GetTransformation(tfm,carrier);
				p=stp->GetPosOnCarrier();
				ev=stp->GetAttOnCarrier().GetForwardVector();
				uv=stp->GetAttOnCarrier().GetUpVector();

				tfm.Mul(p,p,1.0);
				tfm.Mul(ev,ev,0.0);
				tfm.Mul(uv,uv,0.0);

				stp->pos=p;
				stp->att.SetTwoVector(ev,uv);
			}
			else
			{
				YsPrintf("Warning!\n");
				YsPrintf("Crrier [%s] not found.\n",stp->GetCarrierTag());
				YsPrintf("Replaced with \"C POSITION\" and \"C ATTITUDE\"\n");
			}
		}
	}

	stpIsModified=YSFALSE;

	return YSOK;
}

YSRESULT SeScenery::SaveStp(const char fn[])
{
	FILE *fp;
	fp=fopen(fn,"w");
	if(fp!=NULL)
	{
		YsStartPos *stp;
		stp=NULL;
		while((stp=FindNextStartPos(stp))!=NULL)
		{
			fprintf(fp,"N %s\n",stp->GetName());

			if(stp->GetIFF()!=YsStartPos::IFF_ALL)
			{
				fprintf(fp,"P IFF %d\n",stp->GetIFF());
			}

			if(stp->GetOnCarrier()==YSTRUE)
			{
				const YsSceneryGndObj *carrier;
				carrier=SearchGndObjByTag(stp->GetCarrierTag());
				if(carrier!=NULL)
				{
					YsVec3 p,ev,uv;
					YsAtt3 a;
					YsMatrix4x4 tfm;

					GetTransformation(tfm,carrier);

					p=stp->GetPosition();
					a=stp->GetAttitude();
					ev=a.GetForwardVector();
					uv=a.GetUpVector();

					tfm.MulInverse(p,p,1.0);
					tfm.MulInverse(ev,ev,0.0);
					tfm.MulInverse(uv,uv,0.0);

					a.SetTwoVector(ev,uv);
					fprintf(fp,"P CARRIER \"%s\" %.2lfm %.2lfm %.2lfm %.2lfdeg %.2lfdeg %.2lfdeg\n",
					    stp->GetCarrierTag(),
					    p.x(),p.y(),p.z(),YsRadToDeg(a.h()),YsRadToDeg(a.p()),YsRadToDeg(a.b()));
				}
				else
				{
					fprintf(fp,"P CARRIER \"%s\" %.2lfm %.2lfm %.2lfm %.2lfdeg %.2lfdeg %.2lfdeg\n",
					    stp->GetCarrierTag(),
					    stp->GetPosOnCarrier().x(),stp->GetPosOnCarrier().y(),stp->GetPosOnCarrier().z(),
					    YsRadToDeg(stp->GetAttOnCarrier().h()),
					    YsRadToDeg(stp->GetAttOnCarrier().p()),
					    YsRadToDeg(stp->GetAttOnCarrier().b()));
				}
			}

			fprintf(fp,"C POSITION %.2lfm %.2lfm %.2lfm\n",
			    stp->GetPosition().x(),
			    stp->GetPosition().y(),
			    stp->GetPosition().z());
			fprintf(fp,"C ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg\n",
			    YsRadToDeg(stp->GetAttitude().h()),
			    YsRadToDeg(stp->GetAttitude().p()),
			    YsRadToDeg(stp->GetAttitude().b()));
			fprintf(fp,"C INITSPED %.2lfm/s\n",stp->GetSpeed());
			fprintf(fp,"C CTLTHROT %.2lf\n",(double)stp->GetThrottle()/100.0);
			// fprintf(fp,"C INITFUEL %.d%%\n",stp->GetFuel());
			if(stp->GetLandingGear()==YSTRUE)
			{
				fprintf(fp,"C CTLLDGEA TRUE\n");
			}
			else
			{
				fprintf(fp,"C CTLLDGEA FALSE\n");
			}

			if(stp->extraCommand.size()>0)
			{
				for(int i=0; i<stp->extraCommand.size(); i++)
				{
					const char *cmd=stp->extraCommand[i].c_str();
					fprintf(fp,"C %s\n",cmd);
				}
			}

			fprintf(fp,"\n");
		}

		stpIsModified=YSFALSE;
		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

YsScenery *SeScenery::CreateChildScenery(YsScenery *parentScn)
{
	YsListItem <YsScenery> *newScn;

	if(parentScn!=NULL)
	{
		newScn=parentScn->CreateChildScenery();
	}
	else
	{
		newScn=YsScenery::CreateChildScenery();
	}

	if(newScn!=NULL)
	{
		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newScn->dat.owner;
		undo->itm=&newScn->dat;
		AddUndo(undo);

		searchItem.AddElement(newScn->dat.searchKey,&newScn->dat);
		return &newScn->dat;
	}
	return NULL;
}

YsSceneryShell *SeScenery::CreateShell(YsScenery *parentScn,YsVisualSrf &&srf,YsVisualSrf &&collSrf)
{
	YsListItem <YsSceneryShell> *newShl;
	if(nullptr!=parentScn)
	{
		newShl=parentScn->CreateShell();
	}
	else
	{
		newShl=YsScenery::CreateShell();
	}

	if(nullptr!=newShl)
	{
		newShl->dat.shl.CleanUp();
		newShl->dat.shl.MoveFrom(srf);
		newShl->dat.collShl.MoveFrom(collSrf);

		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newShl->dat.owner;
		undo->itm=&newShl->dat;
		AddUndo(undo);
		return &newShl->dat;
	}

	return NULL;
}

YsScenery2DDrawing *SeScenery::CreateMap(YsScenery *parentScn)
{
	YsListItem <YsScenery2DDrawing> *newDrw;

	if(parentScn!=NULL)
	{
		newDrw=parentScn->CreateMap();
	}
	else
	{
		newDrw=YsScenery::CreateMap();
	}

	if(newDrw!=NULL)
	{
		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newDrw->dat.owner;
		undo->itm=&newDrw->dat;
		AddUndo(undo);

		searchItem.AddElement(newDrw->dat.searchKey,&newDrw->dat);
		return &newDrw->dat;
	}
	return NULL;
}

YsSceneryElevationGrid *SeScenery::CreateElevationGrid
    (YsScenery *parentScn,int nx,int nz,const double &xWid,const double &zWid,const YsColor &col)
{
	YsListItem <YsSceneryElevationGrid> *newEvg;

	if(parentScn!=NULL)
	{
		newEvg=parentScn->CreateElevationGrid();
	}
	else
	{
		newEvg=YsScenery::CreateElevationGrid();
	}

	if(newEvg!=NULL)
	{
		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newEvg->dat.owner;
		undo->itm=&newEvg->dat;
		AddUndo(undo);

		searchItem.AddElement(newEvg->dat.searchKey,&newEvg->dat);
		newEvg->dat.evg.Create(nx,nz,xWid,zWid,col);
		return &newEvg->dat;
	}
	else
	{
		return NULL;
	}
}

YsSceneryRectRegion *SeScenery::CreateRectRegion(YsScenery *parentScn,int id,const char tag[])
{
	YsListItem <YsSceneryRectRegion> *newRgn;

	if(parentScn==NULL)
	{
		parentScn=this;
	}
	newRgn=parentScn->CreateRectRegion();
	if(newRgn!=NULL)
	{
		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newRgn->dat.owner;
		undo->itm=&newRgn->dat;
		AddUndo(undo);

		newRgn->dat.id=id;
		newRgn->dat.tagStr=tag;

		searchItem.AddElement(newRgn->dat.searchKey,&newRgn->dat);
		return &newRgn->dat;
	}
	return NULL;
}

YsScenery *SeScenery::ImportScenery(YsScenery *parentScn,const char fn[])
{
	YsListItem <YsScenery> *newScn;
	if(parentScn==NULL)
	{
		parentScn=this;
	}
	newScn=parentScn->CreateChildScenery();
	if(newScn!=NULL)
	{
		if(newScn->dat.LoadFld(fn)==YSOK)
		{
			searchItem.AddElement(newScn->dat.searchKey,&newScn->dat);
			RefreshSearchTable(&newScn->dat);

			newScn->dat.RecomputeBoundingBox();
			RecursivelyUpdateBoundingBox(&newScn->dat);

			SeUndoCreateItem *undo;
			undo=new SeUndoCreateItem;
			undo->parent=newScn->dat.owner;
			undo->itm=&newScn->dat;
			AddUndo(undo);

			return &newScn->dat;
		}
		else
		{
			parentScn->scnList.Delete(newScn);
		}
	}
	return NULL;
}

YsScenery2DDrawing *SeScenery::ImportMap(YsScenery *parentScn,const char fn[])
{
	FILE *fp=fopen(fn,"r");
	if(NULL!=fp)
	{
		YsTextFileInputStream inStream(fp);
		YsScenery2DDrawing *drw=ImportMap(parentScn,inStream);
		fclose(fp);
		return drw;
	}
	return NULL;
}

YsScenery2DDrawing *SeScenery::ImportMap(YsScenery *parentScn,YsTextInputStream &inStream)
{
	YsListItem <YsScenery2DDrawing> *newDrw;
	if(parentScn==NULL)
	{
		parentScn=this;
	}
	newDrw=parentScn->CreateMap();
	if(newDrw!=NULL)
	{
		if(newDrw->dat.drw.LoadPc2(inStream)==YSOK)
		{
			searchItem.AddElement(newDrw->dat.searchKey,&newDrw->dat);
			newDrw->dat.RecomputeBoundingBox();

			YsListItem <Ys2DDrawingElement> *elm;
			elm=NULL;
			while((elm=newDrw->dat.drw.FindNextElem(elm))!=NULL)
			{
				search2DElem.AddElement(elm->dat.searchKey,&elm->dat);
			}

			RecursivelyUpdateBoundingBox(&newDrw->dat);

			SeUndoCreateItem *undo;
			undo=new SeUndoCreateItem;
			undo->parent=newDrw->dat.owner;
			undo->itm=&newDrw->dat;
			AddUndo(undo);

			return &newDrw->dat;
		}
		else
		{
			parentScn->mapList.Delete(newDrw);
		}
	}
	return NULL;
}

YsSceneryElevationGrid *SeScenery::ImportElvGrid(YsScenery *parentScn,const char fn[])
{
	FILE *fp=fopen(fn,"r");
	if(NULL!=fp)
	{
		YsTextFileInputStream inStream(fp);
		YsSceneryElevationGrid *evg=ImportElvGrid(parentScn,inStream);
		fclose(fp);
		return evg;
	}
	return NULL;
}

YsSceneryElevationGrid *SeScenery::ImportElvGrid(YsScenery *parentScn,YsTextInputStream &inStream)
{
	YsListItem <YsSceneryElevationGrid> *newEvg;
	if(parentScn==NULL)
	{
		parentScn=this;
	}
	newEvg=parentScn->CreateElevationGrid();
	if(newEvg!=NULL)
	{
		if(newEvg->dat.evg.LoadTer(inStream)==YSOK)
		{
			searchItem.AddElement(newEvg->dat.searchKey,&newEvg->dat);
			newEvg->dat.RecomputeBoundingBox();
			RecursivelyUpdateBoundingBox(&newEvg->dat);

			SeUndoCreateItem *undo;
			undo=new SeUndoCreateItem;
			undo->parent=newEvg->dat.owner;
			undo->itm=&newEvg->dat;
			AddUndo(undo);

			return &newEvg->dat;
		}
		else
		{
			parentScn->evgList.Delete(newEvg);
		}
	}
	return NULL;
}

YSRESULT SeScenery::ImportYfs(YsScenery *parentScn,YsTextInputStream &inStream)
{
	const int STATE_NORMAL=0;
	const int STATE_INTENTION=1;
	int state=STATE_NORMAL;



	YsArray <YsSceneryGndObj *> gobList;
	YsLattice3d <YsArray <YsSceneryGndObj *> > gobLtc;

	YsArray <YsSceneryAir *> airList;
	YsLattice3d <YsArray <YsSceneryAir *> > airLtc;

	YsMatrix4x4 mat;
	YsBoundingBoxMaker3 makeBbx;
	YsVec3 p,d,bbx[2];
	int i,bx,by,bz;

	if(parentScn==NULL)
	{
		parentScn=this;
	}


	MakeListOfGndObj(gobList);
	for(i=0; i<gobList.GetN(); i++)
	{
		GetTransformation(mat,gobList[i]);
		mat.Mul(p,p,1.0);
		makeBbx.Add(p);
	}
	MakeListOfAir(airList);
	for(i=0; i<airList.GetN(); i++)
	{
		GetTransformation(mat,airList[i]);
		mat.Mul(p,p,1.0);
		makeBbx.Add(p);
	}

	makeBbx.Get(bbx[0],bbx[1]);
	bbx[0]-=YsXYZ();
	bbx[1]+=YsXYZ();

	gobLtc.Create(128,1,128,bbx[0],bbx[1]);
	for(i=0; i<gobList.GetN(); i++)
	{
		YsArray <YsSceneryGndObj *> *blk;
		GetTransformation(mat,gobList[i]);
		mat.Mul(p,YsOrigin(),1.0);
		gobLtc.GetBlockIndexAutoBound(bx,by,bz,p);
		blk=gobLtc.GetEditableBlock(bx,by,bz);
		if(blk!=NULL)
		{
			blk->Append(gobList[i]);
		}
	}

	airLtc.Create(128,1,128,bbx[0],bbx[1]);
	for(i=0; i<airList.GetN(); i++)
	{
		YsArray <YsSceneryAir *> *blk;
		GetTransformation(mat,airList[i]);
		mat.Mul(p,YsOrigin(),1.0);
		airLtc.GetBlockIndexAutoBound(bx,by,bz,p);
		blk=airLtc.GetEditableBlock(bx,by,bz);
		if(blk!=NULL)
		{
			blk->Append(airList[i]);
		}
	}



	const int incUndo=PushStopIncUndo();

	YsListItem <YsSceneryGndObj> *newGnd;
	YsListItem <YsSceneryAir> *newAir;

	YsString str;

	newGnd=NULL;
	newAir=NULL;
	while(NULL!=inStream.Gets(str))
	{
		if(1023<str.Strlen())
		{
			str.SetLength(1023);
		}

		str.DeleteTailSpace();
		YsArray <YsString,16> av;
		str.Arguments(av);
		auto ac=av.GetN();

		if(0<ac)
		{
			if(STATE_NORMAL==state)
			{
				if(strcmp(av[0],"GROUNDOB")==0 && ac>=2)
				{
					newGnd=parentScn->CreateGndObj();
					newGnd->dat.objName.Set(strlen(av[1])+1,av[1]);
					newAir=NULL;

					SeUndoCreateItem *undo;
					undo=new SeUndoCreateItem;
					undo->parent=newGnd->dat.owner;
					undo->itm=&newGnd->dat;
					AddUndo(undo);
					searchItem.AddElement(newGnd->dat.searchKey,&newGnd->dat);
				}
				else if(strcmp(av[0],"AIRPLANE")==0 && ac>=2)
				{
					newAir=parentScn->CreateAir();

					newAir->dat.objName.Set(av[1]);
					newGnd=NULL;

					if(3<=ac)
					{
						newAir->dat.isPlayer=YsStrToBool(av[2]);
					}

					SeUndoCreateItem *undo;
					undo=new SeUndoCreateItem;
					undo->parent=newAir->dat.owner;
					undo->itm=&newAir->dat;
					AddUndo(undo);
					searchItem.AddElement(newAir->dat.searchKey,&newAir->dat);
				}
				else if(0==strcmp(av[0],"INTENTIO"))
				{
					state=STATE_INTENTION;
					if(NULL!=newAir)  // INTENTIO must be in customIntention
					{
						newAir->dat.action.actType=YsSceneryAirAction::CUSTOM;
						newAir->dat.action.customIntention.Append(str);
					}
				}
				else if(strcmp(av[0],"AIRPCMND")==0 && newAir!=NULL && ac>=2)
				{
					if(strcmp(av[1],"CTLLDGEA")==0)
					{
						FsGetBool(newAir->dat.ldg,av[2]);
					}
					else if(strcmp(av[1],"POSITION")==0)
					{
						FsGetVec3(newAir->dat.pos,3,av+2);

						YsArray <YsSceneryAir *> *blk;
						airLtc.GetBlockIndexAutoBound(bx,by,bz,newAir->dat.pos);
						blk=airLtc.GetEditableBlock(bx,by,bz);
						if(blk!=NULL)
						{
							int i;
							for(i=blk->GetN()-1; i>=0; i--)
							{
								GetTransformation(mat,(*blk)[i]);
								mat.Mul(p,YsOrigin(),1.0);
								if((p-newAir->dat.pos).GetLength()<=0.01)
								{
									DeleteItem((*blk)[i]);
									blk->DeleteBySwapping(i);
								}
							}
						}
					}
					else if(strcmp(av[1],"ATTITUDE")==0)
					{
						FsGetAtt3(newAir->dat.att,3,av+2);
					}
					else if(strcmp(av[1],"INITSPED")==0)
					{
						FsGetSpeed(newAir->dat.speed,av[2]);
					}
					else if(strcmp(av[1],"INITFUEL")==0)
					{
						newAir->dat.fuel=atoi(av[2]);
					}
				}
				else if(0==strcmp(av[0],"RELDCMND") ||
				        0==strcmp(av[0],"HOMEBASE"))
				{
					if(NULL!=newAir)
					{
						newAir->dat.otherCommand.Append(str);
					}
				}
				else if(strcmp(av[0],"LANDLWFL")==0 && newAir!=NULL && ac>=2)
				{
					newAir->dat.landWhenLowFuel=atof(av[1]);
				}
				else if(strcmp(av[0],"AIRFLAGS")==0 && newAir!=NULL && ac>=2)
				{
					newAir->dat.airFlag=atoi(av[1]);
				}
				else if(strcmp(av[0],"GNDPOSIT")==0 && ac>=4 && newGnd!=NULL)
				{
					FsGetVec3(newGnd->dat.pos,3,av+1);

					YsArray <YsSceneryGndObj *> *blk;
					gobLtc.GetBlockIndexAutoBound(bx,by,bz,newGnd->dat.pos);  // 2005/02/27 Remove overlapping
					blk=gobLtc.GetEditableBlock(bx,by,bz);
					if(blk!=NULL)
					{
						int i;
						for(i=blk->GetN()-1; i>=0; i--)
						{
							GetTransformation(mat,(*blk)[i]);
							mat.Mul(p,YsOrigin(),1.0);
							if((p-newGnd->dat.pos).GetLength()<=0.01)
							{
								DeleteItem((*blk)[i]);
								blk->DeleteBySwapping(i);
							}
						}
					}
				}
				else if(strcmp(av[0],"GNDATTIT")==0 && ac>=4 && newGnd!=NULL)
				{
					FsGetAtt3(newGnd->dat.att,3,av+1);
				}
				else if(strcmp(av[0],"IDENTIFY")==0 && ac>=2)
				{
					if(newGnd!=NULL)
					{
						newGnd->dat.iff=atoi(av[1]);
					}
					else if(newAir!=NULL)
					{
						newAir->dat.iff=atoi(av[1]);
					}
				}
				else if(strcmp(av[0],"PRTARGET")==0 && newGnd!=NULL)
				{
					newGnd->dat.primaryTarget=YSTRUE;
				}
				else if(strcmp(av[0],"MOTNPATH")==0 && newGnd!=NULL)
				{
					newGnd->dat.motionPathName.Set(strlen(av[1])+1,av[1]);
					FsGetBool(newGnd->dat.motionPathOffset,av[2]);
				}
				else if(strcmp(av[0],"IDANDTAG")==0)
				{
					if(newGnd!=NULL)
					{
						newGnd->dat.id=atoi(av[1]);
						newGnd->dat.tagStr=av[2];
					}
					else if(newAir!=NULL)
					{
						newAir->dat.id=atoi(av[1]);
						newAir->dat.tagStr=av[2];
					}
				}
				else if(strcmp(av[0],"GNDFLAGS")==0 && newGnd!=NULL)
				{
					newGnd->dat.gndFlag=atoi(av[1]);
				}
				else if(0==strcmp(av[0],"FIELDNAM"))
				{
					yfsHeader.Append(str);
				}
			}
			else if(STATE_INTENTION==state)
			{
				if(NULL!=newAir)  // ENDINTEN must be stored in customIntention
				{
					newAir->dat.action.actType=YsSceneryAirAction::CUSTOM;
					newAir->dat.action.customIntention.Append(str);
				}

				if(0==strcmp(av[0],"ENDINTEN"))
				{
					state=STATE_NORMAL;
				}
			}
		}
	}

	LinkGndObjTemplate(gndObjTemplateList);
	LinkAirTemplate(airTemplateList);
	PopIncUndo(incUndo);

	return YSOK;
}

YsSceneryGndObj *SeScenery::CreateGndObj(YsScenery *parent,const char objName[])
{
	if(parent==NULL)
	{
		parent=this;
	}

	YsListItem <YsSceneryGndObj> *newGnd;
	newGnd=parent->CreateGndObj();
	if(newGnd!=NULL)
	{
		newGnd->dat.objName=objName;
		newGnd->dat.objName=objName;

		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newGnd->dat.owner;
		undo->itm=&newGnd->dat;
		AddUndo(undo);
		searchItem.AddElement(newGnd->dat.searchKey,&newGnd->dat);

		YsListItem <YsSceneryGndObjTemplate> *gndTmp;
		gndTmp=NULL;
		while((gndTmp=gndObjTemplateList.FindNext(gndTmp))!=NULL)
		{
			if(strcmp(gndTmp->dat.ObjName(),objName)==0)
			{
				newGnd->dat.gndObjTemplate=&gndTmp->dat;
			}
		}
		return &newGnd->dat;
	}
	return NULL;
}

YsSceneryAir *SeScenery::CreateAir(YsScenery *parent,const char objName[])
{
	if(parent==NULL)
	{
		parent=this;
	}

	YsListItem <YsSceneryAir> *newAir;
	newAir=parent->CreateAir();
	if(newAir!=NULL)
	{
		newAir->dat.objName.Set(objName);

		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newAir->dat.owner;
		undo->itm=&newAir->dat;
		AddUndo(undo);
		searchItem.AddElement(newAir->dat.searchKey,&newAir->dat);

		YsListItem <YsSceneryAirTemplate> *airTmp;
		airTmp=NULL;
		while((airTmp=airTemplateList.FindNext(airTmp))!=NULL)
		{
			if(strcmp(airTmp->dat.ObjName(),objName)==0)
			{
				newAir->dat.airTemplate=&airTmp->dat;
			}
		}
		return &newAir->dat;
	}
	return NULL;
}

YsSceneryPointSet *SeScenery::CreatePointSet(YsScenery *parentScn,int np,const YsVec3 p[],YSBOOL isLoop)
{
	YsListItem <YsSceneryPointSet> *newPst;

	if(parentScn==NULL)
	{
		parentScn=this;
	}
	newPst=parentScn->CreatePointSet();
	if(newPst!=NULL)
	{
		SeUndoCreateItem *undo;
		undo=new SeUndoCreateItem;
		undo->parent=newPst->dat.owner;
		undo->itm=&newPst->dat;
		AddUndo(undo);

		newPst->dat.SetPointList(np,p,isLoop);

		searchItem.AddElement(newPst->dat.searchKey,&newPst->dat);
		return &newPst->dat;
	}
	return NULL;
}

YSRESULT SeScenery::DeleteItem(YsSceneryItem *itm)
{
	if(itm->owner!=NULL)
	{
		YsScenery *owner;
		owner=itm->owner;

		SeUndoDeleteItem *undo;
		undo=new SeUndoDeleteItem;
		undo->parent=owner;
		undo->itm=itm;
		undo->beforeThisItm=NULL;  // <- Order is significant only when it is a MAP or a SUBSCENARY.

		switch(itm->GetObjType())
		{
		case YsSceneryItem::MAP:
			if(owner->FindNextMap(((YsScenery2DDrawing *)itm)->thisInTheList)!=NULL)
			{
				undo->beforeThisItm=&owner->FindNextMap(((YsScenery2DDrawing *)itm)->thisInTheList)->dat;
			}
			owner->mapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,frozenMapList);
			break;
		case YsSceneryItem::SIGNBOARD:
			owner->sbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,frozenSbdList);
			break;
		case YsSceneryItem::ELEVATIONGRID:
			owner->evgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,frozenEvgList);
			break;
		case YsSceneryItem::SHELL:
			owner->shlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,frozenShlList);
			break;
		case YsSceneryItem::SUBSCENARY:
			if(owner->FindNextChildScenery(((YsScenery *)itm)->thisInTheList)!=NULL)
			{
				undo->beforeThisItm=&owner->FindNextChildScenery(((YsScenery *)itm)->thisInTheList)->dat;
			}
			owner->scnList.Transfer(((YsScenery *)itm)->thisInTheList,frozenScnList);
			break;
		case YsSceneryItem::RECTREGION:
			owner->rgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,frozenRgnList);
			break;
		case YsSceneryItem::GROUNDOBJECT:
			owner->gndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,frozenGndList);
			break;
		case YsSceneryItem::AIRCRAFT:
			owner->airList.Transfer(((YsSceneryAir *)itm)->thisInTheList,frozenAirList);
			break;
		case YsSceneryItem::POINTSET:
			owner->pstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,frozenPstList);
			break;
		}

		AddUndo(undo);
		searchItem.DeleteElement(itm->searchKey,itm);

		return YSOK;
	}
	return YSERR;
}

const class YsTextureManager &SeScenery::GetTextureManager(YsScenery *scn) const
{
	if(NULL!=scn)
	{
		return scn->textureManager;
	}
	else
	{
		return this->textureManager;
	}
}

class YsTextureManager &SeScenery::GetTextureManagerInternal(YsScenery *scn)
{
	if(NULL!=scn)
	{
		return scn->textureManager;
	}
	else
	{
		return this->textureManager;
	}
}

YsTextureManager::TexHandle SeScenery::AddTexture(YsScenery *scn,const YsString &str,YsTextureManager::DATA_FORMAT fType,YSSIZE_T nByte,const unsigned char fileData[])
{
	auto &texMan=GetTextureManagerInternal(scn);
	auto texHd=texMan.AddTexture(str,fType,0,0,nByte,fileData);
	if(NULL!=texHd)
	{
		auto undo=new SeUndoAddTexture;
		undo->scn=scn;
		undo->texHd=texHd;
		AddUndo(undo);
	}
	return texHd;
}

YSRESULT SeScenery::DeleteTexture(YsScenery *scn,YsTextureManager::TexHandle texHd)
{
	auto &texMan=GetTextureManagerInternal(scn);
	if(YSOK==texMan.FreezeTexture(texHd))
	{
		auto undo=new SeUndoDeleteTexture;
		undo->scn=scn;
		undo->texHd=texHd;
		AddUndo(undo);
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetTextureFileName(YsScenery *scn,YsTextureManager::TexHandle texHd,const YsWString &fn)
{
	auto &texMan=GetTextureManagerInternal(scn);
	auto oldFn=texMan.GetTextureFileName(texHd);
	if(YSOK==texMan.SetTextureFileName(texHd,fn))
	{
		auto undo=new SeUndoSetTextureFileName;
		undo->scn=scn;
		undo->texHd=texHd;
		undo->oldFn=oldFn;
		undo->newFn=fn;
		AddUndo(undo);
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetTextureFilterType(YsScenery *scn,YsTextureManager::TexHandle texHd,YsTextureManager::Unit::FILTERTYPE filterType)
{
	auto &texMan=GetTextureManagerInternal(scn);
	auto filterTypeBefore=texMan.GetTextureFilterType(texHd);
	if(YSOK==texMan.SetTextureFilterType(texHd,filterType))
	{
		auto undo=new SeUndoSetTextureFilterType;
		undo->scn=scn;
		undo->texHd=texHd;
		undo->filterBefore=filterTypeBefore;
		undo->filterAfter=filterType;
		AddUndo(undo);
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetTextureRandomNoiseLevel(YsScenery *scn,YsTextureManager::TexHandle texHd,const double randomNoiseLevel)
{
	auto &texMan=GetTextureManagerInternal(scn);
	auto randomNoiseBefore=texMan.GetRandomNoiseLevel(texHd);
	if(YSOK==texMan.SetRandomNoiseLevel(texHd,randomNoiseLevel))
	{
		auto undo=new SeUndoSetTextureRandomNoiseLevel;
		undo->scn=scn;
		undo->texHd=texHd;
		undo->randomNoiseBefore=randomNoiseBefore;
		undo->randomNoiseAfter=randomNoiseLevel;
		AddUndo(undo);
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetTextureFileData(YsScenery *scn,YsTextureManager::TexHandle texHd,YsTextureManager::DATA_FORMAT fType,YSSIZE_T length,const unsigned char dat[])
{
printf("%s %d\n",__FUNCTION__,__LINE__);
	auto &texMan=GetTextureManagerInternal(scn);
	auto fTypeBefore=texMan.GetTextureFormat(texHd);
	auto fileDataBefore=texMan.GetTextureFileData(texHd);
	if(YSOK==texMan.SetTextureFileData(texHd,fType,0,0,length,dat))
	{
printf("%s %d\n",__FUNCTION__,__LINE__);
		auto undo=new SeUndoSetTextureFileData;
		undo->scn=scn;
		undo->texHd=texHd;
		undo->fTypeBefore=fTypeBefore;
		undo->fTypeAfter=fType;
		undo->fileDataBefore=fileDataBefore;
		undo->fileDataAfter.Set(length,dat);
		AddUndo(undo);
		return YSOK;
	}
	return YSERR;
}

void SeScenery::SetSkyColor(const YsColor &col)
{
	SeUndoSetGroundSkyColor *undo;
	undo=new SeUndoSetGroundSkyColor;
	undo->orgSkyColor=skyColor;
	undo->orgGndColor=gndColor;

	skyColor=col;

	undo->newSkyColor=skyColor;
	undo->newGndColor=gndColor;
	AddUndo(undo);
}

void SeScenery::SetGroundColor(const YsColor &col)
{
	SeUndoSetGroundSkyColor *undo;
	undo=new SeUndoSetGroundSkyColor;
	undo->orgSkyColor=skyColor;
	undo->orgGndColor=gndColor;

	gndColor=col;

	undo->newSkyColor=skyColor;
	undo->newGndColor=gndColor;
	AddUndo(undo);
}

void SeScenery::SetDefaultAreaType(YSSCNAREATYPE areaType)
{
	SeUndoChangeDefaultAreaType *undo=new SeUndoChangeDefaultAreaType;
	undo->prvAreaType=this->areaType;
	undo->newAreaType=areaType;

	this->areaType=areaType;

	AddUndo(undo);
}

void SeScenery::SetGroundSpecular(YSBOOL s)
{
	auto undo=new SeUndoChangeGroundSpecular;
	undo->specularBefore=GetSpecular();
	undo->specularAfter=s;
	this->SetSpecular(s);
	AddUndo(undo);
}

YSBOOL SeScenery::GetGroundSpecular(void) const
{
	return GetSpecular();
}

void SeScenery::SetBaseElevation(const double &baseElev)
{
	SeUndoSetBaseElevation *undo=new SeUndoSetBaseElevation;
	undo->prvBaseElev=baseElevation;
	undo->newBaseElev=baseElev;

	this->baseElevation=baseElev;

	AddUndo(undo);
}

void SeScenery::SetMagneticVariation(const double &magneticVariation)
{
	SeUndoSetMagneticVariation *undo=new SeUndoSetMagneticVariation;
	undo->prvMagneticVariation=GetMagneticVariation();
	undo->newMagneticVariation=magneticVariation;

	this->magneticVariation=magneticVariation;

	AddUndo(undo);
}

void SeScenery::SetAirRoute(int nRoute,const YsSceneryAndAirRoutePair route[])
{
	SeUndoSetAirRoute *undo=new SeUndoSetAirRoute;
	FindAllSceneryAndAirRoutePair(undo->prvScnRoutePairArray);
	undo->newScnRoutePairArray.Set(nRoute,route);

	ApplySceneryAndAirRoutePair(nRoute,route);

	AddUndo(undo);
}

void SeScenery::SetCanResume(YSBOOL canResume)
{
	if(GetCanResume()!=canResume)
	{
		SeUndoSetCanResume *undo=new SeUndoSetCanResume;
		undo->prvCanResume=this->canResume;
		undo->newCanResume=canResume;

		this->canResume=canResume;

		AddUndo(undo);
	}
}

void SeScenery::SetCanContinue(YSBOOL canContinue)
{
	if(GetCanContinue()!=canContinue)
	{
		SeUndoSetCanContinue *undo=new SeUndoSetCanContinue;
		undo->prvCanContinue=this->canContinue;
		undo->newCanContinue=canContinue;

		this->canContinue=canContinue;

		AddUndo(undo);
	}
}

YSRESULT SeScenery::MoveMapToTopPriority(YsScenery2DDrawing *map)
{
	if(map->objType==YsSceneryItem::MAP)
	{
		YsScenery *owner;
		owner=map->owner;

		SeUndoChangeMapPriority *undo;
		undo=new SeUndoChangeMapPriority;
		undo->parent=map->owner;
		undo->itm=map->thisInTheList;
		undo->fromBeforeThis=owner->FindNextMap(map->thisInTheList);

		owner->mapList.MoveItemToEnd(map->thisInTheList);

		undo->toBeforeThis=owner->FindNextMap(map->thisInTheList);
		AddUndo(undo);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT SeScenery::MoveMapToBottomPriority(YsScenery2DDrawing *map)
{
	if(map->objType==YsSceneryItem::MAP)
	{
		YsScenery *owner;
		owner=map->owner;

		SeUndoChangeMapPriority *undo;
		undo=new SeUndoChangeMapPriority;
		undo->parent=map->owner;
		undo->itm=map->thisInTheList;
		undo->fromBeforeThis=owner->FindNextMap(map->thisInTheList);

		owner->mapList.MoveItemToTop(map->thisInTheList);

		undo->toBeforeThis=owner->FindNextMap(map->thisInTheList);
		AddUndo(undo);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT SeScenery::MapPriorityUp(YsScenery2DDrawing *map)
{
	if(map->objType==YsSceneryItem::MAP)
	{
		YsScenery *owner;
		YsListItem <YsScenery2DDrawing> *nextMap;
		owner=map->owner;

		SeUndoChangeMapPriority *undo;
		undo=new SeUndoChangeMapPriority;
		undo->parent=map->owner;
		undo->itm=map->thisInTheList;
		undo->fromBeforeThis=owner->FindNextMap(map->thisInTheList);

		nextMap=owner->FindNextMap(map->thisInTheList);
		if(nextMap!=NULL)
		{
			owner->mapList.MoveItemAfter(map->thisInTheList,nextMap);
		}

		undo->toBeforeThis=owner->FindNextMap(map->thisInTheList);
		AddUndo(undo);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT SeScenery::MapPriorityDown(YsScenery2DDrawing *map)
{
	if(map->objType==YsSceneryItem::MAP)
	{
		YsScenery *owner;
		YsListItem <YsScenery2DDrawing> *prevMap;
		owner=map->owner;

		SeUndoChangeMapPriority *undo;
		undo=new SeUndoChangeMapPriority;
		undo->parent=map->owner;
		undo->itm=map->thisInTheList;
		undo->fromBeforeThis=owner->FindNextMap(map->thisInTheList);

		prevMap=owner->FindPrevMap(map->thisInTheList);
		if(prevMap!=NULL)
		{
			owner->mapList.MoveItemBefore(map->thisInTheList,prevMap);
		}

		undo->toBeforeThis=owner->FindNextMap(map->thisInTheList);
		AddUndo(undo);

		return YSOK;
	}
	else
	{
		return YSERR;
	}
}

YSRESULT SeScenery::SetSceneryItemPosition(YsSceneryItem *itm,const YsVec3 &pos)
{
	SeUndoMoveItemPosAtt *undo;
	undo=new SeUndoMoveItemPosAtt;
	undo->itm=itm;
	undo->orgPos=itm->pos;
	undo->orgAtt=itm->att;

	itm->pos=pos;

	undo->newPos=itm->pos;
	undo->newAtt=itm->att;
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetSceneryItemAttitude(YsSceneryItem *itm,const YsAtt3 &att)
{
	SeUndoMoveItemPosAtt *undo;
	undo=new SeUndoMoveItemPosAtt;
	undo->itm=itm;
	undo->orgPos=itm->pos;
	undo->orgAtt=itm->att;

	itm->att=att;

	undo->newPos=itm->pos;
	undo->newAtt=itm->att;
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetSceneryItemOwner(YsSceneryItem *itm,YsScenery *newOwner)
{
	if(newOwner==NULL)
	{
		newOwner=this;
	}

	if(itm!=NULL)
	{
		YsScenery *curOwner;
		curOwner=itm->GetOwner();

		SeUndoSetItemOwner *undo;
		undo=new SeUndoSetItemOwner;
		undo->itm=itm;
		undo->orgOwner=curOwner;
		undo->newOwner=newOwner;
		undo->orgBeforeThis=NULL;

		switch(itm->GetObjType())
		{
		case YsSceneryItem::MAP:
			if(curOwner->FindNextMap(((YsScenery2DDrawing *)itm)->thisInTheList)!=NULL)
			{
				undo->orgBeforeThis=&curOwner->FindNextMap(((YsScenery2DDrawing *)itm)->thisInTheList)->dat;
			}
			curOwner->mapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,newOwner->mapList);
			break;
		case YsSceneryItem::SIGNBOARD:
			curOwner->sbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,newOwner->sbdList);
			break;
		case YsSceneryItem::ELEVATIONGRID:
			curOwner->evgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,newOwner->evgList);
			break;
		case YsSceneryItem::SHELL:
			curOwner->shlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,newOwner->shlList);
			break;
		case YsSceneryItem::SUBSCENARY:
			if(curOwner->FindNextChildScenery(((YsScenery *)itm)->thisInTheList)!=NULL)
			{
				undo->orgBeforeThis=&curOwner->FindNextChildScenery(((YsScenery *)itm)->thisInTheList)->dat;
			}
			curOwner->scnList.Transfer(((YsScenery *)itm)->thisInTheList,newOwner->scnList);
			break;
		case YsSceneryItem::RECTREGION:
			curOwner->rgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,newOwner->rgnList);
			break;
		case YsSceneryItem::GROUNDOBJECT:
			curOwner->gndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,newOwner->gndList);
			break;
		case YsSceneryItem::POINTSET:
			curOwner->pstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,newOwner->pstList);
			break;
		}
		itm->owner=newOwner;
		AddUndo(undo);
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetSceneryItemTag(YsSceneryItem *itm,const char tag[])
{
	SeUndoSetItemTag *undo;
	undo=new SeUndoSetItemTag;

	undo->itm=itm;
	undo->orgTag=itm->tagStr;
	undo->newTag=tag;
	AddUndo(undo);

	itm->tagStr=tag;

	return YSOK;
}

YSRESULT SeScenery::SetSceneryItemId(YsSceneryItem *itm,int id)
{
	SeUndoSetItemId *undo;
	undo=new SeUndoSetItemId;

	undo->itm=itm;
	undo->orgId=itm->id;
	undo->newId=id;
	AddUndo(undo);

	itm->id=id;

	return YSOK;
}

YSRESULT SeScenery::SetElvGridNodeElevation(YsSceneryElevationGrid *evg,int x,int z,const double &y)
{
	if(0<=x && x<=evg->evg.nx && 0<=z && z<=evg->evg.nz)
	{
		SeUndoModifyElevationGridNode *undo;
		undo=new SeUndoModifyElevationGridNode;
		undo->evg=evg;
		undo->x=x;
		undo->z=z;
		undo->orgNode=evg->evg.node[z*(evg->evg.nx+1)+x];

		evg->evg.node[z*(evg->evg.nx+1)+x].y=y;

		undo->newNode=evg->evg.node[z*(evg->evg.nx+1)+x];
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::ShowElvGridFace(YsSceneryElevationGrid *evg,int x,int z,int t,YSBOOL show)
{
	if(0<=x && x<evg->evg.nx && 0<=z && z<evg->evg.nz && 0<=t && t<2)
	{
		SeUndoModifyElevationGridNode *undo;
		undo=new SeUndoModifyElevationGridNode;
		undo->evg=evg;
		undo->x=x;
		undo->z=z;
		undo->orgNode=evg->evg.node[z*(evg->evg.nx+1)+x];

		evg->evg.node[z*(evg->evg.nx+1)+x].visible[t]=show;

		undo->newNode=evg->evg.node[z*(evg->evg.nx+1)+x];
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetElvGridProtectPolygonFlag(YsSceneryElevationGrid *evg,int x,int z,int t,YSBOOL protPolyg)
{
	if(0<=x && x<evg->evg.nx && 0<=z && z<evg->evg.nz && 0<=t && t<2)
	{
		SeUndoModifyElevationGridNode *undo;
		undo=new SeUndoModifyElevationGridNode;
		undo->evg=evg;
		undo->x=x;
		undo->z=z;
		undo->orgNode=evg->evg.node[z*(evg->evg.nx+1)+x];

		evg->evg.node[z*(evg->evg.nx+1)+x].protectPolygon[t]=protPolyg;

		undo->newNode=evg->evg.node[z*(evg->evg.nx+1)+x];
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetElvGridFaceColor(YsSceneryElevationGrid *evg,int x,int z,int t,const YsColor &c)
{
	if(0<=x && x<evg->evg.nx && 0<=z && z<evg->evg.nz && 0<=t && t<2)
	{
		SeUndoModifyElevationGridNode *undo;
		undo=new SeUndoModifyElevationGridNode;
		undo->evg=evg;
		undo->x=x;
		undo->z=z;
		undo->orgNode=evg->evg.node[z*(evg->evg.nx+1)+x];

		evg->evg.node[z*(evg->evg.nx+1)+x].c[t]=c;

		undo->newNode=evg->evg.node[z*(evg->evg.nx+1)+x];
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetElvGridTriangulation(YsSceneryElevationGrid *evg,int x,int z,YSBOOL lup)
{
	if(0<=x && x<evg->evg.nx && 0<=z && z<evg->evg.nz)
	{
		SeUndoModifyElevationGridNode *undo;
		undo=new SeUndoModifyElevationGridNode;
		undo->evg=evg;
		undo->x=x;
		undo->z=z;
		undo->orgNode=evg->evg.node[z*(evg->evg.nx+1)+x];

		evg->evg.node[z*(evg->evg.nx+1)+x].lup=lup;

		undo->newNode=evg->evg.node[z*(evg->evg.nx+1)+x];
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetElvGridSideFaceShow(YsSceneryElevationGrid *evg,int fcId,YSBOOL show)
{
	if(0<=fcId && fcId<4)
	{
		SeUndoModifyElevationGridSideWall *undo;
		undo=new SeUndoModifyElevationGridSideWall;
		undo->evg=evg;
		undo->sideFaceId=fcId;
		undo->orgShow=evg->evg.sideWall[fcId];
		undo->orgColor=evg->evg.sideWallColor[fcId];

		evg->evg.sideWall[fcId]=show;

		undo->newShow=evg->evg.sideWall[fcId];
		undo->newColor=evg->evg.sideWallColor[fcId];
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetElvGridSideFaceColor(YsSceneryElevationGrid *evg,int fcId,const YsColor &col)
{
	if(0<=fcId && fcId<4)
	{
		SeUndoModifyElevationGridSideWall *undo;
		undo=new SeUndoModifyElevationGridSideWall;
		undo->evg=evg;
		undo->sideFaceId=fcId;
		undo->orgShow=evg->evg.sideWall[fcId];
		undo->orgColor=evg->evg.sideWallColor[fcId];

		evg->evg.sideWallColor[fcId]=col;

		undo->newShow=evg->evg.sideWall[fcId];
		undo->newColor=evg->evg.sideWallColor[fcId];
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetElvGridColorByElevation
    (YsSceneryElevationGrid *evg,YSBOOL cve,const double y[2],const YsColor c[2])
{
	SeUndoSetElevationGridColorByElevation *undo;
	undo=new SeUndoSetElevationGridColorByElevation;

	undo->evg=evg;
	undo->orgColorByElevation=evg->evg.colorByElevation;
	undo->orgColorByElevation_Elevation[0]=evg->evg.colorByElevation_Elevation[0];
	undo->orgColorByElevation_Elevation[1]=evg->evg.colorByElevation_Elevation[1];
	undo->orgColorByElevation_Color[0]=evg->evg.colorByElevation_Color[0];
	undo->orgColorByElevation_Color[1]=evg->evg.colorByElevation_Color[1];

	evg->evg.colorByElevation=cve;
	evg->evg.colorByElevation_Elevation[0]=y[0];
	evg->evg.colorByElevation_Elevation[1]=y[1];
	evg->evg.colorByElevation_Color[0]=c[0];
	evg->evg.colorByElevation_Color[1]=c[1];

	undo->newColorByElevation=evg->evg.colorByElevation;
	undo->newColorByElevation_Elevation[0]=evg->evg.colorByElevation_Elevation[0];
	undo->newColorByElevation_Elevation[1]=evg->evg.colorByElevation_Elevation[1];
	undo->newColorByElevation_Color[0]=evg->evg.colorByElevation_Color[0];
	undo->newColorByElevation_Color[1]=evg->evg.colorByElevation_Color[1];
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetElvGridTexture(YsSceneryElevationGrid *evg,const YsString &texLabel)
{
	SeUndoSetElevationGridTexture *undo;
	undo=new SeUndoSetElevationGridTexture;

	undo->evg=evg;
	undo->texLabelBefore=evg->evg.texLabel;
	undo->texLabelAfter=texLabel;
	AddUndo(undo);

	evg->evg.SetTextureLabel(texLabel);

	return YSOK;
}

YSRESULT SeScenery::SetElvGridSpecular(YsSceneryElevationGrid *evg,YSBOOL specular)
{
	auto undo=new SeUndoSetElevationGridSpecular;

	undo->evg=evg;
	undo->specularBefore=evg->evg.GetSpecular();
	undo->specularAfter=specular;

	evg->evg.SetSpecular(specular);
	AddUndo(undo);

	return YSOK;
}

Ys2DDrawingElement *SeScenery::CreateDrawElem
	    (YsScenery2DDrawing *drw,Ys2DDrawingElement::OBJTYPE t,int np,const YsVec2 p[])
{
	YsListItem <Ys2DDrawingElement> *newElem;
	newElem=drw->drw.CreateElement(t);
	newElem->dat.pnt.Set(np,p);
	newElem->dat.cvx=YsCheckConvexByAngle2(np,p);
	search2DElem.AddElement(newElem->dat.searchKey,&newElem->dat);

	SeUndoCreate2DDrawElem *undo;
	undo=new SeUndoCreate2DDrawElem;
	undo->drw=drw;
	undo->elm=newElem;
	AddUndo(undo);

	return &newElem->dat;
}

YSRESULT SeScenery::DeleteDrawElem(YsScenery2DDrawing *drw,Ys2DDrawingElement *elem)
{
	if(elem!=NULL && elem->owner==&drw->drw)
	{
		search2DElem.DeleteElement(elem->searchKey,elem);

		SeUndoDelete2DDrawElem *undo;
		undo=new SeUndoDelete2DDrawElem;
		undo->drw=drw;
		undo->elm=elem->thisInTheList;
		undo->beforeThis=drw->drw.elemList.FindNext(elem->thisInTheList);
		AddUndo(undo);

		return drw->drw.FreezeElement(elem->thisInTheList);
	}
	return YSERR;
}

YSRESULT SeScenery::SetDrawElemColor(Ys2DDrawingElement *elem,const YsColor &col)
{
	SeUndoSetDrawElemTypeColorVisidist *undo;
	undo=new SeUndoSetDrawElemTypeColorVisidist;

	undo->CaptureOrg(elem);
	elem->c=col;
	undo->CaptureNew(elem);
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetDrawElemSpecular(YSSIZE_T n,Ys2DDrawingElement *const elem[],YSBOOL specular)
{
	auto undo=new SeUndoSetDrawElemSpecular;

	undo->elem.Set(n,elem);
	undo->orgSpecular.Set(n,NULL);
	for(int i=0; i<n; ++i)
	{
		undo->orgSpecular[i]=elem[i]->GetSpecular();
		elem[i]->SetSpecular(specular);
	}
	undo->newSpecular=specular;

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetDrawElem2ndColor(Ys2DDrawingElement *elem,const YsColor &col)
{
	SeUndoSetDrawElemTypeColorVisidist *undo;
	undo=new SeUndoSetDrawElemTypeColorVisidist;

	undo->CaptureOrg(elem);
	elem->c2=col;
	undo->CaptureNew(elem);
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetDrawElemVisibleDistance(Ys2DDrawingElement *elm,const double &dist)
{
	SeUndoSetDrawElemTypeColorVisidist *undo;
	undo=new SeUndoSetDrawElemTypeColorVisidist;

	undo->CaptureOrg(elm);
	elm->visibleDist=dist;
	undo->CaptureNew(elm);
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetDrawElemPointList(Ys2DDrawingElement *elm,int np,const YsVec2 p[])
{
	SeUndoSetDrawElemPointList *undo;
	undo=new SeUndoSetDrawElemPointList;
	undo->elm=elm;
	undo->orgPntList=elm->pnt;

	elm->pnt.Set(np,p);
	elm->cvx=YsCheckConvexByAngle2(np,p);

	undo->newPntList=elm->pnt;
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetDrawElemTexture(Ys2DDrawingElement *elm,const YsString &texLabel)
{
	SeUndoSetDrawElemTexture *undo=new SeUndoSetDrawElemTexture;
	undo->elm=elm;
	undo->texLabelBefore=elm->texLabel;
	undo->texLabelAfter=texLabel;
	AddUndo(undo);

	elm->texLabel=texLabel;
	elm->texLabelNotFound=YSFALSE;
	
	return YSOK;
}

YSRESULT SeScenery::SetDrawElemTextureCoord(Ys2DDrawingElement *elm,int np,const YsVec2 p[])
{
	SeUndosetDrawElemTextureCoord *undo=new SeUndosetDrawElemTextureCoord;
	undo->elm=elm;
	undo->texCoordBefore=elm->texCoord;
	undo->texCoordAfter.Set(np,p);
	AddUndo(undo);

	elm->texCoord.Set(np,p);

	return YSOK;
}

YSRESULT SeScenery::ChangeDrawElemType(Ys2DDrawingElement *elm,Ys2DDrawingElement::OBJTYPE t)
{
	SeUndoSetDrawElemPointList *undo;
	undo=new SeUndoSetDrawElemPointList;
	undo->elm=elm;
	undo->orgPntList=elm->pnt;

	elm->t=t;;

	undo->newPntList=elm->pnt;
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::MoveDrawElemToTopPriority(Ys2DDrawingElement *elm)
{
	Ys2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *thisInTheList;
	drw=elm->owner;
	thisInTheList=elm->thisInTheList;

	if(thisInTheList->GetContainer()==&drw->elemList)
	{
		SeUndoSetDrawElemPriority *undo;
		undo=new SeUndoSetDrawElemPriority;
		undo->drw=drw;
		undo->elm=elm->thisInTheList;
		undo->orgBeforeThis=drw->elemList.FindNext(elm->thisInTheList);

		drw->elemList.MoveItemToEnd(thisInTheList);

		undo->newBeforeThis=drw->elemList.FindNext(elm->thisInTheList);
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::MoveDrawElemToBottomPriority(Ys2DDrawingElement *elm)
{
	Ys2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *thisInTheList;
	drw=elm->owner;
	thisInTheList=elm->thisInTheList;

	if(thisInTheList->GetContainer()==&drw->elemList)
	{
		SeUndoSetDrawElemPriority *undo;
		undo=new SeUndoSetDrawElemPriority;
		undo->drw=drw;
		undo->elm=elm->thisInTheList;
		undo->orgBeforeThis=drw->elemList.FindNext(elm->thisInTheList);

		drw->elemList.MoveItemToTop(thisInTheList);

		undo->newBeforeThis=drw->elemList.FindNext(elm->thisInTheList);
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::MoveDrawElemBefore(Ys2DDrawingElement *elm,Ys2DDrawingElement *beforeThis)
{
	Ys2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *thisInTheList;
	drw=elm->owner;
	thisInTheList=elm->thisInTheList;

	if(elm!=beforeThis &&
	   thisInTheList->GetContainer()==&drw->elemList &&
	   (beforeThis==NULL || beforeThis->thisInTheList->GetContainer()==&drw->elemList))
	{
		SeUndoSetDrawElemPriority *undo;
		undo=new SeUndoSetDrawElemPriority;
		undo->drw=drw;
		undo->elm=elm->thisInTheList;
		undo->orgBeforeThis=drw->elemList.FindNext(elm->thisInTheList);

		if(beforeThis!=NULL)
		{
			drw->elemList.MoveItemBefore(thisInTheList,beforeThis->thisInTheList);
		}
		else
		{
			drw->elemList.MoveItemBefore(thisInTheList,NULL);
		}

		undo->newBeforeThis=drw->elemList.FindNext(elm->thisInTheList);
		AddUndo(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::DrawElemPriorityUp(Ys2DDrawingElement *elm)
{
	Ys2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *thisInTheList,*nextElm;
	drw=elm->owner;
	thisInTheList=elm->thisInTheList;

	if(thisInTheList->GetContainer()==&drw->elemList)
	{
		nextElm=drw->elemList.FindNext(thisInTheList);
		if(nextElm!=NULL)
		{
			SeUndoSetDrawElemPriority *undo;
			undo=new SeUndoSetDrawElemPriority;
			undo->drw=drw;
			undo->elm=elm->thisInTheList;
			undo->orgBeforeThis=drw->elemList.FindNext(elm->thisInTheList);

			drw->elemList.MoveItemAfter(thisInTheList,nextElm);

			undo->newBeforeThis=drw->elemList.FindNext(elm->thisInTheList);
			AddUndo(undo);

			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT SeScenery::DrawElemPriorityDown(Ys2DDrawingElement *elm)
{
	Ys2DDrawing *drw;
	YsListItem <Ys2DDrawingElement> *thisInTheList,*prevElm;
	drw=elm->owner;
	thisInTheList=elm->thisInTheList;

	if(thisInTheList->GetContainer()==&drw->elemList)
	{
		prevElm=drw->elemList.FindPrev(thisInTheList);
		if(prevElm!=NULL)
		{
			SeUndoSetDrawElemPriority *undo;
			undo=new SeUndoSetDrawElemPriority;
			undo->drw=drw;
			undo->elm=elm->thisInTheList;
			undo->orgBeforeThis=drw->elemList.FindNext(elm->thisInTheList);

			drw->elemList.MoveItemBefore(thisInTheList,prevElm);

			undo->newBeforeThis=drw->elemList.FindNext(elm->thisInTheList);
			AddUndo(undo);

			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT SeScenery::DrawOptimize(void)
{
	int i,j,incUndo;
	int nQuadOptimized,nTriOptimized,nPntOptimized;
	incUndo=PushStopIncUndo();

	YsArray <YsScenery2DDrawing *,16> drwList;
	YsArray <Ys2DDrawingElement *> toDelete;
	YsArray <YsVec2,16> pntList;
	MakeListOfMap(drwList);

	nQuadOptimized=0;
	nTriOptimized=0;
	nPntOptimized=0;

	for(i=0; i<drwList.GetN(); i++)
	{
		YsListItem <Ys2DDrawingElement> *elm,*ptr,*quadStart,*triStart,*pntStart;
		int nConsec;
		elm=NULL;
		quadStart=NULL;
		triStart=NULL;
		pntStart=NULL;
		toDelete.Set(0,NULL);
		while((elm=drwList[i]->GetDrawing().FindNextElem(elm))!=NULL)
		{
			if(quadStart!=NULL && 
			   (elm->dat.GetElemType()!=Ys2DDrawingElement::POLYGON || elm->dat.GetPointList().GetN()!=4 ||
			    elm->dat.GetColor()!=quadStart->dat.GetColor() || elm->dat.IsConvex()!=YSTRUE ||
			    drwList[i]->GetDrawing().FindNextElem(elm)==NULL ||
			    YsEqual(elm->dat.VisibleDist(),quadStart->dat.VisibleDist())!=YSTRUE))
			{
				if(nConsec>1)
				{
					pntList.Set(0,NULL);
					for(ptr=quadStart; ptr!=elm; ptr=drwList[i]->GetDrawing().FindNextElem(ptr))
					{
						if(ptr!=quadStart)
						{
							toDelete.Append(&ptr->dat);
						}
						pntList.Append(ptr->dat.GetPointList()[0]);
						pntList.Append(ptr->dat.GetPointList()[1]);
						pntList.Append(ptr->dat.GetPointList()[2]);
						pntList.Append(ptr->dat.GetPointList()[3]);
					}
					ChangeDrawElemType(&quadStart->dat,Ys2DDrawingElement::QUADS);
					SetDrawElemPointList(&quadStart->dat,pntList.GetN(),pntList);
					nQuadOptimized++;
				}
				quadStart=NULL;
				nConsec=0;
			}
			if(triStart!=NULL &&
			   (elm->dat.GetElemType()!=Ys2DDrawingElement::POLYGON || elm->dat.GetPointList().GetN()!=3 ||
			    elm->dat.GetColor()!=triStart->dat.GetColor() ||
			    drwList[i]->GetDrawing().FindNextElem(elm)==NULL ||
			    YsEqual(elm->dat.VisibleDist(),triStart->dat.VisibleDist())!=YSTRUE))
			{
				if(nConsec>1)
				{
					pntList.Set(0,NULL);
					for(ptr=triStart; ptr!=elm; ptr=drwList[i]->GetDrawing().FindNextElem(ptr))
					{
						if(ptr!=triStart)
						{
							toDelete.Append(&ptr->dat);
						}
						pntList.Append(ptr->dat.GetPointList()[0]);
						pntList.Append(ptr->dat.GetPointList()[1]);
						pntList.Append(ptr->dat.GetPointList()[2]);
					}
					ChangeDrawElemType(&triStart->dat,Ys2DDrawingElement::TRIANGLES);
					SetDrawElemPointList(&triStart->dat,pntList.GetN(),pntList);
					nTriOptimized++;
				}
				triStart=NULL;
				nConsec=0;
			}
			if(pntStart!=NULL &&
			   (elm->dat.GetElemType()!=Ys2DDrawingElement::POINTS ||
			    elm->dat.GetColor()!=pntStart->dat.GetColor() ||
			    drwList[i]->GetDrawing().FindNextElem(elm)==NULL ||
			    YsEqual(elm->dat.VisibleDist(),pntStart->dat.VisibleDist())!=YSTRUE))
			{
				if(nConsec>1)
				{
					pntList.Set(0,NULL);
					for(ptr=pntStart; ptr!=elm; ptr=drwList[i]->GetDrawing().FindNextElem(ptr))
					{
						if(ptr!=pntStart)
						{
							toDelete.Append(&ptr->dat);
						}
						pntList.Append(ptr->dat.GetPointList().GetN(),ptr->dat.GetPointList());
					}
					SetDrawElemPointList(&pntStart->dat,pntList.GetN(),pntList);
					nPntOptimized++;
				}
				pntStart=NULL;
				nConsec=0;
			}

			if(elm->dat.GetElemType()==Ys2DDrawingElement::POLYGON &&
			   elm->dat.GetPointList().GetN()==4 &&
			   elm->dat.IsConvex()==YSTRUE)
			{
				if(quadStart==NULL)
				{
					quadStart=elm;
					nConsec=0;
				}
				nConsec++;
			}
			if(elm->dat.GetElemType()==Ys2DDrawingElement::POLYGON &&
			   elm->dat.GetPointList().GetN()==3)
			{
				if(triStart==NULL)
				{
					triStart=elm;
					nConsec=0;
				}
				nConsec++;
			}
			if(elm->dat.GetElemType()==Ys2DDrawingElement::POINTS)
			{
				if(pntStart==NULL)
				{
					pntStart=elm;
					nConsec=0;
				}
				nConsec++;
			}
		}

		for(j=0; j<toDelete.GetN(); j++)
		{
			DeleteDrawElem(drwList[i],toDelete[j]);
		}
	}

	printf("%d quad grouops are created.\n",nQuadOptimized);
	printf("%d tri groups are created.\n",nTriOptimized);
	printf("%d pnt groups are created.\n",nPntOptimized);

	PopIncUndo(incUndo);

	return YSOK;
}

YSRESULT SeScenery::SetRectRegionRect(YsSceneryRectRegion *rgn,const YsVec2 rec[])
{
	SeUndoModifyRectRegion *undo;
	undo=new SeUndoModifyRectRegion;
	undo->CaptureOrg(rgn);

	YsBoundingBoxMaker2 makeBbx;
	makeBbx.Make(2,rec);
	makeBbx.Get(rgn->min,rgn->max);

	undo->CaptureNew(rgn);
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetRectRegionSubClassType(YsSceneryRectRegion *rgn,YsSceneryRectRegion::SUBCLASSTYPE subClassType)
{
	SeUndoModifyRectRegion *undo;
	undo=new SeUndoModifyRectRegion;
	undo->CaptureOrg(rgn);

	rgn->SetSubClassType(subClassType);

	undo->CaptureNew(rgn);
	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetMotionPathIsLoop(YsSceneryPointSet *mpa,YSBOOL isLoop)
{
	SeUndoSetMotionPathIsLoop *undo;
	undo=new SeUndoSetMotionPathIsLoop;
	undo->mpa=mpa;
	undo->isLoopOrg=mpa->IsLoop();
	undo->isLoopNew=isLoop;

	mpa->isLoop=isLoop;

	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetMotionPathCoord(YsSceneryPointSet *mpa,int np,const YsVec3 p[])
{
	SeUndoSetMotionPathCoord *undo;
	undo=new SeUndoSetMotionPathCoord;
	undo->mpa=mpa;
	undo->mpaOrg=mpa->pnt;
	undo->mpaNew.Set(np,p);

	mpa->SetPointList(np,p);

	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::SetMotionPathAreaType(YsSceneryPointSet *rgn,YSSCNAREATYPE areaType)
{
	SeUndoChangeMotionPathAreaType *undo;
	undo=new SeUndoChangeMotionPathAreaType;
	undo->mpa=rgn;
	undo->prvAreaType=rgn->areaType;
	undo->newAreaType=areaType;

	rgn->SetAreaType(areaType);

	AddUndo(undo);

	return YSOK;
}

YSRESULT SeScenery::RelinkGndObjTemplateAll(void)
{
	LinkGndObjTemplate(gndObjTemplateList);
	return YSOK;
}

YSRESULT SeScenery::SetGndObjIff(YsSceneryGndObj *gndObj,int iff)
{
	SeUndoModifyGndObj *undo;
	undo=new SeUndoModifyGndObj;
	undo->gndObj=gndObj;
	undo->orgIff=gndObj->iff;
	undo->orgPmt=gndObj->primaryTarget;
	undo->orgFlag=gndObj->gndFlag;
	undo->orgMotionPathName=gndObj->motionPathName;
	undo->orgMotionPathOffset=gndObj->motionPathOffset;

	gndObj->iff=iff;

	undo->newIff=gndObj->iff;
	undo->newPmt=gndObj->primaryTarget;
	undo->newFlag=gndObj->gndFlag;
	undo->newMotionPathName=gndObj->motionPathName;
	undo->newMotionPathOffset=gndObj->motionPathOffset;
	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetGndObjPrimaryTarget(YsSceneryGndObj *gndObj,YSBOOL pmt)
{
	SeUndoModifyGndObj *undo;
	undo=new SeUndoModifyGndObj;
	undo->gndObj=gndObj;
	undo->orgIff=gndObj->iff;
	undo->orgPmt=gndObj->primaryTarget;
	undo->orgFlag=gndObj->gndFlag;
	undo->orgMotionPathName=gndObj->motionPathName;
	undo->orgMotionPathOffset=gndObj->motionPathOffset;

	gndObj->primaryTarget=pmt;

	undo->newIff=gndObj->iff;
	undo->newPmt=gndObj->primaryTarget;
	undo->newFlag=gndObj->gndFlag;
	undo->newMotionPathName=gndObj->motionPathName;
	undo->newMotionPathOffset=gndObj->motionPathOffset;
	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetGndObjFlag(YsSceneryGndObj *gndObj,unsigned flag)
{
	SeUndoModifyGndObj *undo;
	undo=new SeUndoModifyGndObj;
	undo->gndObj=gndObj;
	undo->orgIff=gndObj->iff;
	undo->orgPmt=gndObj->primaryTarget;
	undo->orgFlag=gndObj->gndFlag;
	undo->orgMotionPathName=gndObj->motionPathName;
	undo->orgMotionPathOffset=gndObj->motionPathOffset;

	gndObj->gndFlag=flag;

	undo->newIff=gndObj->iff;
	undo->newPmt=gndObj->primaryTarget;
	undo->newFlag=gndObj->gndFlag;
	undo->newMotionPathName=gndObj->motionPathName;
	undo->newMotionPathOffset=gndObj->motionPathOffset;
	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetGndObjMotionPath(YsSceneryGndObj *gndObj,const char motionPathName[],YSBOOL motionPathOffset)
{
	SeUndoModifyGndObj *undo;
	undo=new SeUndoModifyGndObj;
	undo->gndObj=gndObj;
	undo->orgIff=gndObj->iff;
	undo->orgPmt=gndObj->primaryTarget;
	undo->orgFlag=gndObj->gndFlag;
	undo->orgMotionPathName=gndObj->motionPathName;
	undo->orgMotionPathOffset=gndObj->motionPathOffset;

	if(motionPathName!=NULL)
	{
		gndObj->motionPathName=motionPathName;
	}
	else
	{
		gndObj->motionPathName="";
	}
	gndObj->motionPathOffset=motionPathOffset;

	undo->newIff=gndObj->iff;
	undo->newPmt=gndObj->primaryTarget;
	undo->newFlag=gndObj->gndFlag;
	undo->newMotionPathName=gndObj->motionPathName;
	undo->newMotionPathOffset=gndObj->motionPathOffset;
	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::RelinkAirTemplateAll(void)
{
	LinkAirTemplate(airTemplateList);
	return YSOK;
}

YSRESULT SeScenery::SetAirType(YsSceneryAir *air,const char name[])
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->objName.Set(name);
	LinkAirTemplate(air,airTemplateList);
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetAirIff(YsSceneryAir *air,int iff)
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->iff=iff;
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetAirLandingGear(YsSceneryAir *air,YSBOOL ldg)
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->ldg=ldg;
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetAirFlag(YsSceneryAir *air,unsigned int airFlag)
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->airFlag=airFlag;
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetAirSpeed(YsSceneryAir *air,const double &speed)
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->speed=speed;
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetAirFuel(YsSceneryAir *air,int fuel)
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->fuel=fuel;
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetAirLandWhenLowFuel(YsSceneryAir *air,const double &fuelThr)
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->landWhenLowFuel=fuelThr;
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

YSRESULT SeScenery::SetAirAction(YsSceneryAir *air,const YsSceneryAirAction &action)
{
	SeUndoModifyAir *undo;
	undo=new SeUndoModifyAir;

	undo->CaptureOrg(air);
	air->action=action;
	undo->CaptureNew(air);

	AddUndo(undo);
	return YSOK;
}

int SeScenery::PushStopIncUndo(void)
{
	int savedIncUndo;
	undoCounter++;
	savedIncUndo=incUndo;
	incUndo=0;
	return savedIncUndo;
}

void SeScenery::PopIncUndo(int savedIncUndo)
{
	undoCounter+=savedIncUndo;
	incUndo=savedIncUndo;
}

YSRESULT SeScenery::ClearYsflightGndObjTemplate(void)
{
	gndObjTemplateList.CleanUp();
	UnlinkGndObjTemplate();
	return YSOK;
}

YSRESULT SeScenery::LoadYsflightGndObjTemplate(
    const char ysfsDir[],const char subDir[],const char prefix[],const char ext[])
{
	YsArray <char> dir;
	YsString ful;
	YsArray <YsString> filelist;


	dir.Set(strlen(ysfsDir),ysfsDir);
	if(dir.GetEndItem()!='/' && dir.GetEndItem()!='\\')
	{
		dir.Append('/');
	}

	dir.Append(strlen(subDir),subDir);
	if(dir.GetEndItem()=='/' || dir.GetEndItem()=='\\')
	{
		dir[dir.GetN()-1]=0;
	}
	else
	{
		dir.Append(0);
	}


	YsFileList fList;
	if(YSOK==fList.FindFileList(dir,prefix,ext))
	{
		for(YSSIZE_T i=0; i<fList.GetN(); i++)
		{
			const wchar_t *wfn=fList.GetFileName(i);
			YsString fn;
			fn.EncodeUTF8(wfn);

			ful.Set(subDir);
			ful.Append("/");
			ful.Append(fn);
			if(LoadYsflightGndObjTemplate(ysfsDir,ful)!=YSOK)
			{
				return YSERR;
			}
		}
	}

	return YSOK;
}

YSRESULT SeScenery::LoadYsflightGndObjTemplate(const char ysfsDir[],const char gndLstFn[])
{
	YsPrintf("Loading GndObj Template (%s)\n",gndLstFn);

	YsString fn;
	fn.MakeFullPathName(ysfsDir,gndLstFn);

	FILE *fp;
	fp=fopen(fn,"r");
	if(fp!=NULL)
	{
		char buf[1024];
		int ac;
		char *av[16];
		while(fgets(buf,1024,fp)!=NULL)
		{
			if(YsArguments(&ac,av,16,buf)==YSOK && ac>=2)
			{
				AddYsflightGndObjTemplate(ysfsDir,av[0],av[1]);
			}
		}
		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::AddYsflightGndObjTemplate(const char ysfsDir[],const char dat[],const char vis[])
{
	YsListItem <YsSceneryGndObjTemplate> *newTmp;
	newTmp=gndObjTemplateList.Create();
	if(newTmp!=NULL)
	{
		newTmp->dat.Initialize();

		YsString fn;
		fn.MakeFullPathName(ysfsDir,vis);
		newTmp->dat.SetDnmFileName(fn);

		FILE *fp;
		fn.MakeFullPathName(ysfsDir,dat);
		fp=fopen(fn,"r");
		if(fp!=NULL)
		{
			char buf[256];
			int ac;
			char *av[16];
			while(fgets(buf,256,fp)!=NULL)
			{
				if(YsArguments(&ac,av,16,buf)==YSOK && ac>0)
				{
					if(strcmp(av[0],"IDENTIFY")==0)
					{
						newTmp->dat.SetObjName(av[1]);
					}
					else if(0==strcmp(av[0],"VORRANGE") && 2<=ac)
					{
						FsGetLength(newTmp->dat.vorRange,av[1]);
					}
					else if(0==strcmp(av[0],"NDBRANGE") && 2<=ac)
					{
						FsGetLength(newTmp->dat.ndbRange,av[1]);
					}
					else if(0==strcmp(av[0],"GRNDTYPE") && 2<=ac)
					{
						if(strcmp(av[1],"STATIC")==0)
						{
							newTmp->dat.gndType=FSSTATIC;
						}
						else if(strcmp(av[1],"VEHICLE")==0)
						{
							newTmp->dat.gndType=FSVEHICLE;
						}
						else if(strcmp(av[1],"TANK")==0)
						{
							newTmp->dat.gndType=FSTANK;
						}
						else if(strcmp(av[1],"SHIP")==0)
						{
							newTmp->dat.gndType=FSSHIP;
						}
						else if(strcmp(av[1],"NAVYSHIP")==0)
						{
							newTmp->dat.gndType=FSNAVYSHIP;
						}
					}
					else if(strcmp(av[0],"CARRIER")==0)
					{
						FILE *cfp;
						fn.MakeFullPathName(ysfsDir,av[1]);
						cfp=fopen(fn,"r");
						if(cfp!=NULL)
						{
							char buf[256];
							int ac;
							char *av[16];

							fgets(buf,256,cfp);  // Skip Deck
							fgets(buf,256,cfp);  // Skip Wire
							fgets(buf,256,cfp);  // Skip Catapult
							fgets(buf,256,cfp);  // Skip Bridge
							while(fgets(buf,256,cfp)!=NULL)
							{
								if(YsArguments(&ac,av,16,buf)==YSOK && ac>0)
								{
									if(strcmp(av[0],"ILS")==0 && ac>=8)
									{
										FsGetVec3(newTmp->dat.ilsPos,3,av+1);
										FsGetAtt3(newTmp->dat.ilsAtt,3,av+4);
										FsGetLength(newTmp->dat.ilsRange,av[7]);
									}
								}
							}
							fclose(cfp);
						}
						else
						{
							YsPrintf("Warning:Cannot read Aircraft Carrier or ILS property.\n");
						}
					}
					else
					{
					}
				}
			}
			fclose(fp);
		}
		else
		{
			goto ERREND;
		}

		return YSOK;
	}

ERREND:
	if(newTmp!=NULL)
	{
		gndObjTemplateList.Delete(newTmp);
	}
	return YSERR;
}

YSRESULT SeScenery::ClearYsflightAirTemplate(void)
{
	gndObjTemplateList.CleanUp();
	UnlinkAirTemplate();
	return YSOK;
}

YSRESULT SeScenery::LoadYsflightAirTemplate(
    const char ysfsDir[],const char subDir[],const char prefix[],const char ext[])
{
	int i;
	YsString dir;
	YsString ful;
	YsArray <YsString> filelist;


	dir.Set(ysfsDir);
	if(dir.LastChar()!='/' && dir.LastChar()!='\\')
	{
		dir.Append("/");
	}

	dir.Append(subDir);
	if(dir.LastChar()=='/' || dir.LastChar()=='\\')
	{
		dir.BackSpace();
	}


	YsFileList fList;
	if(YSOK==fList.FindFileList(dir,prefix,ext))
	{
		for(YSSIZE_T idx=0; idx<fList.GetN(); idx++)
		{
			const wchar_t *wfn=fList.GetFileName(idx);
			YsString fn;
			fn.EncodeUTF8(wfn);

			ful.Set(subDir);
			ful.Append("/");
			ful.Append(fn);
			if(LoadYsflightAirTemplate(ysfsDir,ful)!=YSOK)
			{
				return YSERR;
			}
		}
	}

	return YSOK;
}

YSRESULT SeScenery::LoadYsflightAirTemplate(const char ysfsDir[],const char airFn[])
{
	YsString fn;
	fn.MakeFullPathName(ysfsDir,airFn);

	YsPrintf("Loading Air Template (%s)\n",fn.Txt());

	FILE *fp;
	fp=fopen(fn,"r");
	if(fp!=NULL)
	{
		char buf[1024];
		int ac;
		char *av[16];
		while(fgets(buf,1024,fp)!=NULL)
		{
			if(YsArguments(&ac,av,16,buf)==YSOK && ac>=2)
			{
				AddYsflightAirTemplate(ysfsDir,av[0],av[1]);
			}
		}
		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::AddYsflightAirTemplate(const char ysfsDir[],const char dat[],const char vis[])
{
	YsListItem <YsSceneryAirTemplate> *newTmp;
	newTmp=airTemplateList.Create();
	if(newTmp!=NULL)
	{
		newTmp->dat.Initialize();

		YsString fn;
		fn.MakeFullPathName(ysfsDir,vis);
		newTmp->dat.SetDnmFileName(fn);

		// if(newTmp->dat.dnm.LoadDnm(fn)!=YSOK)
		// {
		// 	newTmp->dat.dnm.CleanUp();
		// 	YsPrintf("Warning: Cannot Load %s (Is it a dynamic model?)\n",fn.Txt());
		// }

		FILE *fp;
		fn.MakeFullPathName(ysfsDir,dat);
		fp=fopen(fn,"r");
		if(fp!=NULL)
		{
			char buf[256];
			int ac;
			char *av[16];
			while(fgets(buf,256,fp)!=NULL)
			{
				if(YsArguments(&ac,av,16,buf)==YSOK && ac>0)
				{
					if(strcmp(av[0],"IDENTIFY")==0)
					{
						newTmp->dat.SetObjName(av[1]);
						break;
					}
				}
			}
			fclose(fp);
		}
		else
		{
			goto ERREND;
		}

		return YSOK;
	}

ERREND:
	if(newTmp!=NULL)
	{
		airTemplateList.Delete(newTmp);
	}
	return YSERR;
}

void SeScenery::GetBoundingBox(YsVec3 b[2]) const
{
	b[0]=bbx[0];
	b[1]=bbx[1];
}

YsSceneryItem *SeScenery::FindItem(unsigned searchKey) const
{
	YsSceneryItem *itm;
	if(searchItem.FindElement(itm,searchKey)==YSOK)
	{
		return itm;
	}
	else
	{
		return NULL;
	}
}

Ys2DDrawingElement *SeScenery::Find2DElem(unsigned searchKey) const
{
	Ys2DDrawingElement *elem;
	if(search2DElem.FindElement(elem,searchKey)==YSOK)
	{
		return elem;
	}
	else
	{
		return NULL;
	}
}

const YsListContainer <YsSceneryGndObjTemplate> &SeScenery::GetGndObjTemplate(void) const
{
	return gndObjTemplateList;
}

const YsListContainer <YsSceneryAirTemplate> &SeScenery::GetAirTemplate(void) const
{
	return airTemplateList;
}
	
const YsSceneryGndObjTemplate *SeScenery::FindGndObjTemplate(const char gndObjName[]) const
{
	YsListItem <YsSceneryGndObjTemplate> *gndTmp;
	gndTmp=NULL;
	while((gndTmp=gndObjTemplateList.FindNext(gndTmp))!=NULL)
	{
		if(strcmp(gndObjName,gndTmp->dat.ObjName())==0)
		{
			return &gndTmp->dat;
		}
	}
	return NULL;
}

const YsSceneryAirTemplate *SeScenery::FindAirTemplate(const char airName[],YSBOOL ) const
{
	YsListItem <YsSceneryAirTemplate> *airTmp;
	airTmp=NULL;
	while((airTmp=airTemplateList.FindNext(airTmp)))
	{
		if(strcmp(airName,airTmp->dat.ObjName())==0)
		{
			return &airTmp->dat;
		}
	}
	return NULL;
}

YSBOOL SeScenery::IsModified(void) const
{
	return modified;
}

YSBOOL SeScenery::StpIsModified(void) const
{
	return stpIsModified;
}

const char *SeScenery::GetLastOpenedFilename(void) const
{
	return lastOpenedFName;
}

const char *SeScenery::GetLastOpenedStpFilename(void) const
{
	return lastOpenedStpFName;
}

void SeScenery::SetLastOpenedFilename(const char fn[])
{
	lastOpenedFName=fn;
}


YsStartPos *SeScenery::CreateStartPos(const char nam[],const YsVec3 &pos,const YsAtt3 &att)
{
	YsStartPos *stp;
	stp=CreateStartPos();
	if(stp!=NULL)
	{
		stp->nameStr=nam;

		for(int i=0; i<stp->nameStr.size(); ++i)
		{
			if(stp->nameStr[i]==' ')
			{
				stp->nameStr.Set(i,'_');
			}
		}

		stp->pos=pos;
		stp->att=att;

		SeUndoCreateStartPos *undo;
		undo=new SeUndoCreateStartPos;
		undo->stp=stp;
		AddUndoStp(undo);

		return stp;
	}
	else
	{
		return NULL;
	}
}

YsStartPos *SeScenery::CreateStartPos(void)
{
	YsListItem <YsStartPos> *stp;
	stp=stpList.Create();
	if(stp!=NULL)
	{
		stp->dat.Initialize();
		stp->dat.thisInTheList=stp;
		return &stp->dat;
	}
	else
	{
		return NULL;
	}
}

YSRESULT SeScenery::SetStartPosName(YsStartPos *stp,const char nam[])
{
	SeUndoModifyStartPos *undo;
	undo=new SeUndoModifyStartPos;
	undo->stp=stp;
	undo->orgNameStr=stp->GetName();
	undo->orgPos=stp->GetPosition();
	undo->orgAtt=stp->GetAttitude();
	undo->orgSpeed=stp->GetSpeed();
	undo->orgThrottle=stp->GetThrottle();
	// undo->orgFuel=stp->GetFuel();
	undo->orgLandingGear=stp->GetLandingGear();
	undo->orgIFF=stp->iff;

	stp->nameStr=nam;
	for(int i=0; i<stp->nameStr.size(); ++i)
	{
		if(stp->nameStr[i]==' ')
		{
			stp->nameStr.Set(i,'_');
		}
	}

	undo->newNameStr=stp->GetName();
	undo->newPos=stp->GetPosition();
	undo->newAtt=stp->GetAttitude();
	undo->newSpeed=stp->GetSpeed();
	undo->newThrottle=stp->GetThrottle();
	// undo->newFuel=stp->GetFuel();
	undo->newLandingGear=stp->GetLandingGear();
	undo->newIFF=stp->iff;
	AddUndoStp(undo);

	return YSOK;
}

YSRESULT SeScenery::SetStartPos(YsStartPos *stp,const YsVec3 &pos)
{
	SeUndoModifyStartPos *undo;
	undo=new SeUndoModifyStartPos;
	undo->stp=stp;
	undo->orgNameStr=stp->GetName();
	undo->orgPos=stp->GetPosition();
	undo->orgAtt=stp->GetAttitude();
	undo->orgSpeed=stp->GetSpeed();
	undo->orgThrottle=stp->GetThrottle();
	// undo->orgFuel=stp->GetFuel();
	undo->orgLandingGear=stp->GetLandingGear();
	undo->orgIFF=stp->iff;

	stp->pos=pos;

	undo->newNameStr=stp->GetName();
	undo->newPos=stp->GetPosition();
	undo->newAtt=stp->GetAttitude();
	undo->newSpeed=stp->GetSpeed();
	undo->newThrottle=stp->GetThrottle();
	// undo->newFuel=stp->GetFuel();
	undo->newLandingGear=stp->GetLandingGear();
	undo->newIFF=stp->iff;
	AddUndoStp(undo);

	return YSOK;
}

YSRESULT SeScenery::SetStartPosAttitude(YsStartPos *stp,const YsAtt3 &att)
{
	SeUndoModifyStartPos *undo;
	undo=new SeUndoModifyStartPos;
	undo->stp=stp;
	undo->orgNameStr=stp->GetName();
	undo->orgPos=stp->GetPosition();
	undo->orgAtt=stp->GetAttitude();
	undo->orgSpeed=stp->GetSpeed();
	undo->orgThrottle=stp->GetThrottle();
	// undo->orgFuel=stp->GetFuel();
	undo->orgLandingGear=stp->GetLandingGear();
	undo->orgIFF=stp->iff;

	stp->att=att;

	undo->newNameStr=stp->GetName();
	undo->newPos=stp->GetPosition();
	undo->newAtt=stp->GetAttitude();
	undo->newSpeed=stp->GetSpeed();
	undo->newThrottle=stp->GetThrottle();
	// undo->newFuel=stp->GetFuel();
	undo->newLandingGear=stp->GetLandingGear();
	undo->newIFF=stp->iff;
	AddUndoStp(undo);

	return YSOK;
}

YSRESULT SeScenery::SetStartPosSpeed(YsStartPos *stp,const double &speed)
{
	SeUndoModifyStartPos *undo;
	undo=new SeUndoModifyStartPos;
	undo->stp=stp;
	undo->orgNameStr=stp->GetName();
	undo->orgPos=stp->GetPosition();
	undo->orgAtt=stp->GetAttitude();
	undo->orgSpeed=stp->GetSpeed();
	undo->orgThrottle=stp->GetThrottle();
	// undo->orgFuel=stp->GetFuel();
	undo->orgLandingGear=stp->GetLandingGear();
	undo->orgIFF=stp->iff;

	stp->speed=speed;

	undo->newNameStr=stp->GetName();
	undo->newPos=stp->GetPosition();
	undo->newAtt=stp->GetAttitude();
	undo->newSpeed=stp->GetSpeed();
	undo->newThrottle=stp->GetThrottle();
	// undo->newFuel=stp->GetFuel();
	undo->newLandingGear=stp->GetLandingGear();
	undo->newIFF=stp->iff;
	AddUndoStp(undo);

	return YSOK;
}

YSRESULT SeScenery::SetStartPosThrottle(YsStartPos *stp,int thr)
{
	SeUndoModifyStartPos *undo;
	undo=new SeUndoModifyStartPos;
	undo->stp=stp;
	undo->orgNameStr=stp->GetName();
	undo->orgPos=stp->GetPosition();
	undo->orgAtt=stp->GetAttitude();
	undo->orgSpeed=stp->GetSpeed();
	undo->orgThrottle=stp->GetThrottle();
	// undo->orgFuel=stp->GetFuel();
	undo->orgLandingGear=stp->GetLandingGear();
	undo->orgIFF=stp->iff;

	stp->throttle=thr;

	undo->newNameStr=stp->GetName();
	undo->newPos=stp->GetPosition();
	undo->newAtt=stp->GetAttitude();
	undo->newSpeed=stp->GetSpeed();
	undo->newThrottle=stp->GetThrottle();
	// undo->newFuel=stp->GetFuel();
	undo->newLandingGear=stp->GetLandingGear();
	undo->newIFF=stp->iff;
	AddUndoStp(undo);

	return YSOK;
}

// YSRESULT SeScenery::SetStartPosFuel(YsStartPos *stp,int fuel)
// {
// 	SeUndoModifyStartPos *undo;
// 	undo=new SeUndoModifyStartPos;
// 	undo->stp=stp;
// 	strcpy(undo->orgName,stp->GetName());
// 	undo->orgPos=stp->GetPosition();
// 	undo->orgAtt=stp->GetAttitude();
// 	undo->orgSpeed=stp->GetSpeed();
// 	undo->orgThrottle=stp->GetThrottle();
// 	undo->orgFuel=stp->GetFuel();
// 	undo->orgLandingGear=stp->GetLandingGear();
// 	undo->orgIFF=stp->iff;
//
// 	stp->fuel=fuel;
//
// 	strcpy(undo->newName,stp->GetName());
// 	undo->newPos=stp->GetPosition();
// 	undo->newAtt=stp->GetAttitude();
// 	undo->newSpeed=stp->GetSpeed();
// 	undo->newThrottle=stp->GetThrottle();
// 	undo->newFuel=stp->GetFuel();
// 	undo->newLandingGear=stp->GetLandingGear();
// 	undo->newIFF=stp->iff;
// 	AddUndoStp(undo);
//
// 	return YSOK;
// }

YSRESULT SeScenery::SetStartPosLandingGear(YsStartPos *stp,YSBOOL gear)
{
	SeUndoModifyStartPos *undo;
	undo=new SeUndoModifyStartPos;
	undo->stp=stp;
	undo->orgNameStr=stp->GetName();
	undo->orgPos=stp->GetPosition();
	undo->orgAtt=stp->GetAttitude();
	undo->orgSpeed=stp->GetSpeed();
	undo->orgThrottle=stp->GetThrottle();
//	undo->orgFuel=stp->GetFuel();
	undo->orgLandingGear=stp->GetLandingGear();
	undo->orgIFF=stp->iff;

	stp->landingGear=gear;

	undo->newNameStr=stp->GetName();
	undo->newPos=stp->GetPosition();
	undo->newAtt=stp->GetAttitude();
	undo->newSpeed=stp->GetSpeed();
	undo->newThrottle=stp->GetThrottle();
//	undo->newFuel=stp->GetFuel();
	undo->newLandingGear=stp->GetLandingGear();
	undo->newIFF=stp->iff;
	AddUndoStp(undo);

	return YSOK;
}

YSRESULT SeScenery::SetStartPosIFF(YsStartPos *stp,int iff)
{
	SeUndoModifyStartPos *undo;
	undo=new SeUndoModifyStartPos;
	undo->stp=stp;
	undo->orgNameStr=stp->GetName();
	undo->orgPos=stp->GetPosition();
	undo->orgAtt=stp->GetAttitude();
	undo->orgSpeed=stp->GetSpeed();
	undo->orgThrottle=stp->GetThrottle();
//	undo->orgFuel=stp->GetFuel();
	undo->orgLandingGear=stp->GetLandingGear();
	undo->orgIFF=stp->iff;

	stp->iff=iff;

	undo->newNameStr=stp->GetName();
	undo->newPos=stp->GetPosition();
	undo->newAtt=stp->GetAttitude();
	undo->newSpeed=stp->GetSpeed();
	undo->newThrottle=stp->GetThrottle();
//	undo->newFuel=stp->GetFuel();
	undo->newLandingGear=stp->GetLandingGear();
	undo->newIFF=stp->iff;
	AddUndoStp(undo);

	return YSOK;
}

YSRESULT SeScenery::DeleteStartPos(YsStartPos *stp)
{
	if(stp->thisInTheList->GetContainer()==&stpList)
	{
		SeUndoDeleteStartPos *undo;
		undo=new SeUndoDeleteStartPos;
		undo->stp=stp;
		undo->prevBeforeThis=stpList.FindNext(stp->thisInTheList);
		AddUndoStp(undo);

		stpList.Transfer(stp->thisInTheList,frozenStpList);
	}
	return YSOK;
}

YSRESULT SeScenery::StartPosPriorityUp(YsStartPos *stp)
{
	YsListItem <YsStartPos> *ptr;
	ptr=stpList.FindPrev(stp->thisInTheList);
	if(ptr!=NULL)
	{
		SeUndoChangeStartPosPriority *undo;
		undo=new SeUndoChangeStartPosPriority;
		undo->stp=stp;
		undo->orgBeforeThis=stpList.FindNext(stp->thisInTheList);

		stpList.MoveItemBefore(stp->thisInTheList,ptr);

		undo->newBeforeThis=stpList.FindNext(stp->thisInTheList);
		AddUndoStp(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::StartPosPriorityDown(YsStartPos *stp)
{
	YsListItem <YsStartPos> *ptr;
	ptr=stpList.FindNext(stp->thisInTheList);
	if(ptr!=NULL)
	{
		SeUndoChangeStartPosPriority *undo;
		undo=new SeUndoChangeStartPosPriority;
		undo->stp=stp;
		undo->orgBeforeThis=stpList.FindNext(stp->thisInTheList);

		stpList.MoveItemAfter(stp->thisInTheList,ptr);

		undo->newBeforeThis=stpList.FindNext(stp->thisInTheList);
		AddUndoStp(undo);

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::SetStartPosOnCarrier
   (YsStartPos *stp,YSBOOL onCarrier,const char carrierTag[],const YsVec3 &relPos,const YsAtt3 &relAtt)
{
	SeUndoModifyStartPosCarrier *undo;
	undo=new SeUndoModifyStartPosCarrier;

	undo->orgOnCarrier=stp->onCarrier;
	undo->orgCarrierTag=stp->carrierTag;
	undo->orgPosOnCarrier=stp->posOnCarrier;
	undo->orgAttOnCarrier=stp->attOnCarrier;


	stp->onCarrier=onCarrier;
	stp->carrierTag.Set(strlen(carrierTag)+1,carrierTag);
	stp->posOnCarrier=relPos;
	stp->attOnCarrier=relAtt;


	undo->newOnCarrier=stp->onCarrier;
	undo->newCarrierTag=stp->carrierTag;
	undo->newPosOnCarrier=stp->posOnCarrier;
	undo->newAttOnCarrier=stp->attOnCarrier;

	return YSOK;
}

YsStartPos *SeScenery::FindNextStartPos(YsStartPos *stp) const
{
	YsListItem <YsStartPos> *stpPtr;
	if(stp==NULL)
	{
		stpPtr=NULL;
	}
	else
	{
		stpPtr=stp->thisInTheList;
	}

	stpPtr=stpList.FindNext(stpPtr);
	if(stpPtr!=NULL)
	{
		return &stpPtr->dat;
	}
	else
	{
		return NULL;
	}
}

YsStartPos *SeScenery::FindPrevStartPos(YsStartPos *stp) const
{
	YsListItem <YsStartPos> *stpPtr;
	if(stp==NULL)
	{
		stpPtr=NULL;
	}
	else
	{
		stpPtr=stp->thisInTheList;
	}

	stpPtr=stpList.FindPrev(stpPtr);
	if(stpPtr!=NULL)
	{
		return &stpPtr->dat;
	}
	else
	{
		return NULL;
	}
}

const YsScenery *SeScenery::GetYsScenery(void) const
{
	return this;
}

YSRESULT SeScenery::Undo(void)
{
	if(undoPtr!=NULL)
	{
		int ctr;
		ctr=undoPtr->undoCount;
		while(undoPtr!=NULL && undoPtr->undoCount==ctr)
		{
			undoPtr->Undo(this);
			redoPtr=undoPtr;
			undoPtr=undoPtr->prev;
		}

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::Redo(void)
{
	if(redoPtr!=NULL)
	{
		int ctr;
		ctr=redoPtr->undoCount;
		while(redoPtr!=NULL && redoPtr->undoCount==ctr)
		{
			redoPtr->Redo(this);
			undoPtr=redoPtr;
			redoPtr=redoPtr->next;
		}

		return YSOK;
	}
	return YSERR;
}

YSRESULT SeScenery::CleanUpUndoLog(void)
{
	if(redoPtr!=NULL)
	{
		SeUndo *del,*nxt;
		nxt=redoPtr;
		while((del=nxt)!=NULL)
		{
			nxt=del->next;
			del->WillNeverBeRedone(this);
			delete del;
		}
	}
	if(undoPtr!=NULL)
	{
		SeUndo *del,*prv;
		prv=undoPtr;
		while((del=prv)!=NULL)
		{
			prv=del->prev;
			del->WillNeverBeUndone(this);
			delete del;
		}
	}
	redoPtr=NULL;
	undoPtr=NULL;
	undoLog=NULL;

	return YSOK;
}

YSRESULT SeScenery::AddUndo(SeUndo *undo)
{
	modified=YSTRUE;

	if(undoLog==NULL || undoLog==redoPtr)
	{
		undoLog=undo;
	}

	undo->undoCount=undoCounter;
	undoCounter+=incUndo;

	if(redoPtr!=NULL)
	{
		SeUndo *del,*nxt;
		nxt=redoPtr;
		while((del=nxt)!=NULL)
		{
			nxt=del->next;
			del->WillNeverBeRedone(this);
			delete del;
		}
		if(undoPtr!=NULL)
		{
			undoPtr->next=NULL;
		}
		redoPtr=NULL;
	}

	if(undoPtr!=NULL)
	{
		undoPtr->next=undo;
	}
	undo->prev=undoPtr;

	undoPtr=undo;
	redoPtr=NULL;

	return YSOK;
}

YSRESULT SeScenery::AddUndoStp(SeUndo *undo)
{
	stpIsModified=YSTRUE;

	if(undoLog==NULL || undoLog==redoPtr)
	{
		undoLog=undo;
	}

	undo->undoCount=undoCounter;
	undoCounter+=incUndo;

	if(redoPtr!=NULL)
	{
		SeUndo *del,*nxt;
		nxt=redoPtr;
		while((del=nxt)!=NULL)
		{
			nxt=del->next;
			del->WillNeverBeRedone(this);
			delete del;
		}
		if(undoPtr!=NULL)
		{
			undoPtr->next=NULL;
		}
		redoPtr=NULL;
	}

	if(undoPtr!=NULL)
	{
		undoPtr->next=undo;
	}
	undo->prev=undoPtr;

	undoPtr=undo;
	redoPtr=NULL;

	return YSOK;
}

YSRESULT SeScenery::CleanUpSearchTable(void)
{
	searchItem.PrepareTable();
	search2DElem.PrepareTable();

	searchKey=0xffff0000;
	searchItem.AddElement(searchKey,this);

	return YSOK;
}

YSRESULT SeScenery::RefreshSearchTable(void)
{
	CleanUpSearchTable();
	return RefreshSearchTable(this);
}

YSRESULT SeScenery::RefreshSearchTable(YsScenery *scenary)
{
	YsListItem <YsSceneryShell> *shl;
	YsListItem <YsSceneryElevationGrid> *evg;
	YsListItem <YsScenery2DDrawing> *drw;
	YsListItem <YsSceneryRectRegion> *rgn;
	YsListItem <YsSceneryPointSet> *pst;
	YsListItem <YsScenery> *scn;
	YsListItem <YsSceneryGndObj> *gnd;
	YsListItem <YsSceneryAir> *air;

	drw=NULL;
	while((drw=scenary->FindNextMap(drw))!=NULL)
	{
		searchItem.AddElement(drw->dat.searchKey,&drw->dat);

		YsListItem <Ys2DDrawingElement> *ptr;
		ptr=NULL;
		while((ptr=drw->dat.drw.FindNextElem(ptr))!=NULL)
		{
			search2DElem.AddElement(ptr->dat.searchKey,&ptr->dat);
		}
	}

	shl=NULL;
	while((shl=scenary->FindNextShell(shl))!=NULL)
	{
		searchItem.AddElement(shl->dat.searchKey,&shl->dat);
	}

	evg=NULL;
	while((evg=scenary->FindNextElevationGrid(evg))!=NULL)
	{
		searchItem.AddElement(evg->dat.searchKey,&evg->dat);
	}

	drw=NULL;
	while((drw=scenary->FindNextSignBoard(drw))!=NULL)
	{
		searchItem.AddElement(drw->dat.searchKey,&drw->dat);

		YsListItem <Ys2DDrawingElement> *ptr;
		ptr=NULL;
		while((ptr=drw->dat.drw.FindNextElem(ptr))!=NULL)
		{
			search2DElem.AddElement(ptr->dat.searchKey,&ptr->dat);
		}
	}

	rgn=NULL;
	while((rgn=scenary->FindNextRectRegion(rgn))!=NULL)
	{
		searchItem.AddElement(rgn->dat.searchKey,&rgn->dat);
	}

	pst=NULL;
	while((pst=scenary->FindNextPointSet(pst))!=NULL)
	{
		searchItem.AddElement(pst->dat.searchKey,&pst->dat);
	}

	gnd=NULL;
	while((gnd=scenary->FindNextGndObj(gnd))!=NULL)
	{
		searchItem.AddElement(gnd->dat.searchKey,&gnd->dat);
	}

	air=NULL;
	while((air=scenary->FindNextAir(air))!=NULL)
	{
		searchItem.AddElement(air->dat.searchKey,&air->dat);
	}

	scn=NULL;
	while((scn=scenary->FindNextChildScenery(scn))!=NULL)
	{
		searchItem.AddElement(scn->dat.searchKey,&scn->dat);
		RefreshSearchTable(&scn->dat);
	}


	return YSOK;
}







//  >>Undo  //////////////////////////////////////////////////////////

SeUndo::SeUndo()
{
	prev=NULL;
	next=NULL;
}

YSRESULT SeUndo::WillNeverBeRedone(class SeScenery *scn)
{
	return YSOK;
}

YSRESULT SeUndo::WillNeverBeUndone(class SeScenery *scn)
{
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoCreateItem::Undo(class SeScenery *scn)
{
	scn->searchItem.DeleteElement(itm->searchKey,itm);
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		parent->mapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,scn->frozenMapList);
		break;
	case YsSceneryItem::SIGNBOARD:
		parent->sbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,scn->frozenSbdList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		parent->evgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,scn->frozenEvgList);
		break;
	case YsSceneryItem::SHELL:
		parent->shlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,scn->frozenShlList);
		break;
	case YsSceneryItem::SUBSCENARY:
		parent->scnList.Transfer(((YsScenery *)itm)->thisInTheList,scn->frozenScnList);
		break;
	case YsSceneryItem::RECTREGION:
		parent->rgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,scn->frozenRgnList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		parent->gndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,scn->frozenGndList);
		break;
	case YsSceneryItem::POINTSET:
		parent->pstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,scn->frozenPstList);
		break;
	}
	return YSOK;
}

YSRESULT SeUndoCreateItem::Redo(class SeScenery *scn)
{
	scn->searchItem.AddElement(itm->searchKey,itm);
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		scn->frozenMapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,parent->mapList);
		break;
	case YsSceneryItem::SIGNBOARD:
		scn->frozenSbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,parent->sbdList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		scn->frozenEvgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,parent->evgList);
		break;
	case YsSceneryItem::SHELL:
		scn->frozenShlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,parent->shlList);
		break;
	case YsSceneryItem::SUBSCENARY:
		scn->frozenScnList.Transfer(((YsScenery *)itm)->thisInTheList,parent->scnList);
		break;
	case YsSceneryItem::RECTREGION:
		scn->frozenRgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,parent->rgnList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		scn->frozenGndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,parent->gndList);
		break;
	case YsSceneryItem::POINTSET:
		scn->frozenPstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,parent->pstList);
		break;
	}
	return YSOK;
}

YSRESULT SeUndoCreateItem::WillNeverBeRedone(class SeScenery *scn)
{
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		scn->frozenMapList.Delete(((YsScenery2DDrawing *)itm)->thisInTheList);
		break;
	case YsSceneryItem::SIGNBOARD:
		scn->frozenSbdList.Delete(((YsScenery2DDrawing *)itm)->thisInTheList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		scn->frozenEvgList.Delete(((YsSceneryElevationGrid *)itm)->thisInTheList);
		break;
	case YsSceneryItem::SHELL:
		scn->frozenShlList.Delete(((YsSceneryShell *)itm)->thisInTheList);
		break;
	case YsSceneryItem::SUBSCENARY:
		scn->frozenScnList.Delete(((YsScenery *)itm)->thisInTheList);
		break;
	case YsSceneryItem::RECTREGION:
		scn->frozenRgnList.Delete(((YsSceneryRectRegion *)itm)->thisInTheList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		scn->frozenGndList.Delete(((YsSceneryGndObj *)itm)->thisInTheList);
		break;
	case YsSceneryItem::POINTSET:
		scn->frozenPstList.Delete(((YsSceneryPointSet *)itm)->thisInTheList);
		break;
	}
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoDeleteItem::Undo(class SeScenery *scn)
{
	scn->searchItem.AddElement(itm->searchKey,itm);
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		scn->frozenMapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,parent->mapList);
		if(beforeThisItm!=NULL)
		{
			parent->mapList.MoveItemBefore
			    (((YsScenery2DDrawing *)itm)->thisInTheList,
			     ((YsScenery2DDrawing *)beforeThisItm)->thisInTheList);
		}
		break;
	case YsSceneryItem::SIGNBOARD:
		scn->frozenSbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,parent->sbdList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		scn->frozenEvgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,parent->evgList);
		break;
	case YsSceneryItem::SHELL:
		scn->frozenShlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,parent->shlList);
		break;
	case YsSceneryItem::SUBSCENARY:
		scn->frozenScnList.Transfer(((YsScenery *)itm)->thisInTheList,parent->scnList);
		if(beforeThisItm!=NULL)
		{
			parent->scnList.MoveItemBefore
			    (((YsScenery *)itm)->thisInTheList,
			     ((YsScenery *)beforeThisItm)->thisInTheList);
		}
		break;
	case YsSceneryItem::RECTREGION:
		scn->frozenRgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,parent->rgnList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		scn->frozenGndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,parent->gndList);
		break;
	case YsSceneryItem::POINTSET:
		scn->frozenPstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,parent->pstList);
		break;
	}
	return YSOK;
}

YSRESULT SeUndoDeleteItem::Redo(class SeScenery *scn)
{
	scn->searchItem.DeleteElement(itm->searchKey,itm);
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		parent->mapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,scn->frozenMapList);
		break;
	case YsSceneryItem::SIGNBOARD:
		parent->sbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,scn->frozenSbdList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		parent->evgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,scn->frozenEvgList);
		break;
	case YsSceneryItem::SHELL:
		parent->shlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,scn->frozenShlList);
		break;
	case YsSceneryItem::SUBSCENARY:
		parent->scnList.Transfer(((YsScenery *)itm)->thisInTheList,scn->frozenScnList);
		break;
	case YsSceneryItem::RECTREGION:
		parent->rgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,scn->frozenRgnList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		parent->gndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,scn->frozenGndList);
		break;
	case YsSceneryItem::POINTSET:
		parent->pstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,scn->frozenPstList);
		break;
	}
	return YSOK;
}

YSRESULT SeUndoDeleteItem::WillNeverBeUndone(class SeScenery *scn)
{
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		scn->frozenMapList.Delete(((YsScenery2DDrawing *)itm)->thisInTheList);
		break;
	case YsSceneryItem::SIGNBOARD:
		scn->frozenSbdList.Delete(((YsScenery2DDrawing *)itm)->thisInTheList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		scn->frozenEvgList.Delete(((YsSceneryElevationGrid *)itm)->thisInTheList);
		break;
	case YsSceneryItem::SHELL:
		scn->frozenShlList.Delete(((YsSceneryShell *)itm)->thisInTheList);
		break;
	case YsSceneryItem::SUBSCENARY:
		scn->frozenScnList.Delete(((YsScenery *)itm)->thisInTheList);
		break;
	case YsSceneryItem::RECTREGION:
		scn->frozenRgnList.Delete(((YsSceneryRectRegion *)itm)->thisInTheList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		scn->frozenGndList.Delete(((YsSceneryGndObj *)itm)->thisInTheList);
		break;
	case YsSceneryItem::POINTSET:
		scn->frozenPstList.Delete(((YsSceneryPointSet *)itm)->thisInTheList);
		break;
	}
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoChangeMapPriority::Undo(class SeScenery *scn)
{
	parent->mapList.MoveItemBefore(itm,fromBeforeThis);
	return YSOK;
}

YSRESULT SeUndoChangeMapPriority::Redo(class SeScenery *scn)
{
	parent->mapList.MoveItemBefore(itm,toBeforeThis);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoMoveItemPosAtt::Undo(class SeScenery *scn)
{
	itm->pos=orgPos;
	itm->att=orgAtt;
	return YSOK;
}

YSRESULT SeUndoMoveItemPosAtt::Redo(class SeScenery *scn)
{
	itm->pos=newPos;
	itm->att=newAtt;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetItemOwner::Undo(class SeScenery *scn)
{
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		newOwner->mapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,orgOwner->mapList);
		if(orgBeforeThis!=NULL)
		{
			newOwner->mapList.MoveItemBefore(
			    ((YsScenery2DDrawing *)itm)->thisInTheList,
			    ((YsScenery2DDrawing *)orgBeforeThis)->thisInTheList);
		}
		break;
	case YsSceneryItem::SIGNBOARD:
		newOwner->sbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,orgOwner->sbdList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		newOwner->evgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,orgOwner->evgList);
		break;
	case YsSceneryItem::SHELL:
		newOwner->shlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,orgOwner->shlList);
		break;
	case YsSceneryItem::SUBSCENARY:
		newOwner->scnList.Transfer(((YsScenery *)itm)->thisInTheList,orgOwner->scnList);
		if(orgBeforeThis!=NULL)
		{
			newOwner->scnList.MoveItemBefore(
			    ((YsScenery *)itm)->thisInTheList,
			    ((YsScenery *)orgBeforeThis)->thisInTheList);
		}
		break;
	case YsSceneryItem::RECTREGION:
		newOwner->rgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,orgOwner->rgnList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		newOwner->gndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,orgOwner->gndList);
		break;
	case YsSceneryItem::POINTSET:
		newOwner->pstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,orgOwner->pstList);
		break;
	}
	itm->owner=orgOwner;
	return YSOK;
}

YSRESULT SeUndoSetItemOwner::Redo(class SeScenery *scn)
{
	switch(itm->GetObjType())
	{
	case YsSceneryItem::MAP:
		orgOwner->mapList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,newOwner->mapList);
		break;
	case YsSceneryItem::SIGNBOARD:
		orgOwner->sbdList.Transfer(((YsScenery2DDrawing *)itm)->thisInTheList,newOwner->sbdList);
		break;
	case YsSceneryItem::ELEVATIONGRID:
		orgOwner->evgList.Transfer(((YsSceneryElevationGrid *)itm)->thisInTheList,newOwner->evgList);
		break;
	case YsSceneryItem::SHELL:
		orgOwner->shlList.Transfer(((YsSceneryShell *)itm)->thisInTheList,newOwner->shlList);
		break;
	case YsSceneryItem::SUBSCENARY:
		orgOwner->scnList.Transfer(((YsScenery *)itm)->thisInTheList,newOwner->scnList);
		break;
	case YsSceneryItem::RECTREGION:
		orgOwner->rgnList.Transfer(((YsSceneryRectRegion *)itm)->thisInTheList,newOwner->rgnList);
		break;
	case YsSceneryItem::GROUNDOBJECT:
		orgOwner->gndList.Transfer(((YsSceneryGndObj *)itm)->thisInTheList,newOwner->gndList);
		break;
	case YsSceneryItem::POINTSET:
		orgOwner->pstList.Transfer(((YsSceneryPointSet *)itm)->thisInTheList,newOwner->pstList);
		break;
	}
	itm->owner=newOwner;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetItemTag::Undo(class SeScenery *scn)
{
	itm->tagStr=orgTag;
	return YSOK;
}

YSRESULT SeUndoSetItemTag::Redo(class SeScenery *scn)
{
	itm->tagStr=newTag;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetItemId::Undo(class SeScenery *scn)
{
	itm->id=orgId;
	return YSOK;
}

YSRESULT SeUndoSetItemId::Redo(class SeScenery *scn)
{
	itm->id=newId;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoModifyElevationGridNode::Undo(class SeScenery *scn)
{
	evg->evg.node[z*(evg->evg.nx+1)+x]=orgNode;
	evg->evg.RecomputeNormal(x  ,z  );
	evg->evg.RecomputeNormal(x-1,z  );
	evg->evg.RecomputeNormal(x  ,z-1);
	evg->evg.RecomputeNormal(x-1,z-1);
	return YSOK;
}

YSRESULT SeUndoModifyElevationGridNode::Redo(class SeScenery *scn)
{
	evg->evg.node[z*(evg->evg.nx+1)+x]=newNode;
	evg->evg.RecomputeNormal(x  ,z  );
	evg->evg.RecomputeNormal(x-1,z  );
	evg->evg.RecomputeNormal(x  ,z-1);
	evg->evg.RecomputeNormal(x-1,z-1);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoModifyElevationGridSideWall::Undo(class SeScenery *scn)
{
	evg->evg.sideWall[sideFaceId]=orgShow;
	evg->evg.sideWallColor[sideFaceId]=orgColor;
	return YSOK;
}

YSRESULT SeUndoModifyElevationGridSideWall::Redo(class SeScenery *scn)
{
	evg->evg.sideWall[sideFaceId]=newShow;
	evg->evg.sideWallColor[sideFaceId]=newColor;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetElevationGridColorByElevation::Undo(class SeScenery *scn)
{
	evg->evg.colorByElevation=orgColorByElevation;
	evg->evg.colorByElevation_Elevation[0]=orgColorByElevation_Elevation[0];
	evg->evg.colorByElevation_Elevation[1]=orgColorByElevation_Elevation[1];
	evg->evg.colorByElevation_Color[0]=orgColorByElevation_Color[0];
	evg->evg.colorByElevation_Color[1]=orgColorByElevation_Color[1];
	return YSOK;
}

YSRESULT SeUndoSetElevationGridColorByElevation::Redo(class SeScenery *scn)
{
	evg->evg.colorByElevation=newColorByElevation;
	evg->evg.colorByElevation_Elevation[0]=newColorByElevation_Elevation[0];
	evg->evg.colorByElevation_Elevation[1]=newColorByElevation_Elevation[1];
	evg->evg.colorByElevation_Color[0]=newColorByElevation_Color[0];
	evg->evg.colorByElevation_Color[1]=newColorByElevation_Color[1];
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoCreate2DDrawElem::Undo(class SeScenery *scn)
{
	scn->search2DElem.DeleteElement(elm->dat.searchKey,&elm->dat);
	drw->drw.elemList.Transfer(elm,drw->drw.frozenList);
	return YSOK;
}

YSRESULT SeUndoCreate2DDrawElem::Redo(class SeScenery *scn)
{
	scn->search2DElem.AddElement(elm->dat.searchKey,&elm->dat);
	drw->drw.frozenList.Transfer(elm,drw->drw.elemList);
	return YSOK;
}

YSRESULT SeUndoCreate2DDrawElem::WillNeverBeRedone(class SeScenery *scn)
{
	drw->drw.frozenList.Delete(elm);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoDelete2DDrawElem::Undo(class SeScenery *scn)
{
	drw->drw.frozenList.Transfer(elm,drw->drw.elemList);
	drw->drw.elemList.MoveItemBefore(elm,beforeThis);
	scn->search2DElem.AddElement(elm->dat.searchKey,&elm->dat);
	return YSOK;
}

YSRESULT SeUndoDelete2DDrawElem::Redo(class SeScenery *scn)
{
	drw->drw.elemList.Transfer(elm,drw->drw.frozenList);
	scn->search2DElem.DeleteElement(elm->dat.searchKey,&elm->dat);
	return YSOK;
}

YSRESULT SeUndoDelete2DDrawElem::WillNeverBeUndone(class SeScenery *scn)
{
	drw->drw.frozenList.Delete(elm);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetDrawElemTypeColorVisidist::Undo(class SeScenery *scn)
{
	elm->t=orgType;
	elm->c=orgColor;
	elm->c2=orgColor2;
	elm->visibleDist=orgVisiDist;
	return YSOK;
}

YSRESULT SeUndoSetDrawElemTypeColorVisidist::Redo(class SeScenery *scn)
{
	elm->t=newType;
	elm->c=newColor;
	elm->c2=newColor2;
	elm->visibleDist=newVisiDist;
	return YSOK;
}

void SeUndoSetDrawElemTypeColorVisidist::CaptureOrg(Ys2DDrawingElement *elem)
{
	elm=elem;
	orgType=elem->t;
	orgColor=elem->c;
	orgColor2=elem->c2;
	orgVisiDist=elem->visibleDist;
}

void SeUndoSetDrawElemTypeColorVisidist::CaptureNew(Ys2DDrawingElement *elem)
{
	newType=elem->t;
	newColor=elem->c;
	newColor2=elem->c2;
	newVisiDist=elem->visibleDist;
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetDrawElemSpecular::Undo(class SeScenery *scn)
{
	for(auto idx : elem.AllIndex())
	{
		elem[idx]->SetSpecular(orgSpecular[idx]);
	}
	return YSOK;
}

/* virtual */ YSRESULT SeUndoSetDrawElemSpecular::Redo(class SeScenery *scn)
{
	for(auto idx : elem.AllIndex())
	{
		elem[idx]->SetSpecular(newSpecular);
	}
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetDrawElemPointList::Undo(class SeScenery *scn)
{
	elm->pnt=orgPntList;
	elm->cvx=YsCheckConvexByAngle2(orgPntList.GetN(),orgPntList);
	return YSOK;
}

YSRESULT SeUndoSetDrawElemPointList::Redo(class SeScenery *scn)
{
	elm->pnt=newPntList;
	elm->cvx=YsCheckConvexByAngle2(newPntList.GetN(),newPntList);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetDrawElemPriority::Undo(class SeScenery *scn)
{
	drw->elemList.MoveItemBefore(elm,orgBeforeThis);
	return YSOK;
}

YSRESULT SeUndoSetDrawElemPriority::Redo(class SeScenery *scn)
{
	drw->elemList.MoveItemBefore(elm,newBeforeThis);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoModifyGndObj::Undo(class SeScenery *scn)
{
	gndObj->iff=orgIff;
	gndObj->primaryTarget=orgPmt;
	gndObj->gndFlag=orgFlag;
	gndObj->motionPathName=orgMotionPathName;
	gndObj->motionPathOffset=orgMotionPathOffset;
	return YSOK;
}

YSRESULT SeUndoModifyGndObj::Redo(class SeScenery *scn)
{
	gndObj->iff=newIff;
	gndObj->primaryTarget=newPmt;
	gndObj->gndFlag=newFlag;
	gndObj->motionPathName=newMotionPathName;
	gndObj->motionPathOffset=newMotionPathOffset;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoModifyAir::Undo(class SeScenery *scn)
{
	air->iff=orgIff;
	air->ldg=orgLdg;
	air->airFlag=orgFlag;
	return YSOK;
}

YSRESULT SeUndoModifyAir::Redo(class SeScenery *scn)
{
	air->iff=newIff;
	air->ldg=newLdg;
	air->airFlag=newFlag;
	return YSOK;
}

void SeUndoModifyAir::CaptureOrg(YsSceneryAir *air)
{
	this->air=air;
	orgIff=air->iff;
	orgLdg=air->ldg;
	orgFlag=air->airFlag;
	orgType=air->objName;
	orgFuel=air->fuel;
	orgSpeed=air->speed;
	orgLandWhenLowFuel=air->landWhenLowFuel;
	orgAct=air->action;
}

void SeUndoModifyAir::CaptureNew(YsSceneryAir *air)
{
	this->air=air;
	newIff=air->iff;
	newLdg=air->ldg;
	newFlag=air->airFlag;
	newType=air->objName;
	newFuel=air->fuel;
	newSpeed=air->speed;
	newLandWhenLowFuel=air->landWhenLowFuel;
	newAct=air->action;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoModifyRectRegion::Undo(class SeScenery *scn)
{
	rgn->min=orgRgn[0];
	rgn->max=orgRgn[1];
	rgn->id=orgId;
	rgn->tagStr=orgTag;
	rgn->SetSubClassType(prevSubClassType);
	return YSOK;
}

YSRESULT SeUndoModifyRectRegion::Redo(class SeScenery *scn)
{
	rgn->min=newRgn[0];
	rgn->max=newRgn[1];
	rgn->id=newId;
	rgn->tagStr=newTag;
	rgn->SetSubClassType(newSubClassType);
	return YSOK;
}

void SeUndoModifyRectRegion::CaptureOrg(YsSceneryRectRegion *rgn)
{
	rgn=rgn;
	orgId=rgn->id;
	orgRgn[0]=rgn->min;
	orgRgn[1]=rgn->max;
	orgTag=rgn->tagStr;
	prevSubClassType=rgn->GetSubClassType();
}

void SeUndoModifyRectRegion::CaptureNew(YsSceneryRectRegion *rgn)
{
	rgn=rgn;
	newId=rgn->id;
	newRgn[0]=rgn->min;
	newRgn[1]=rgn->max;
	newTag=rgn->tagStr;
	newSubClassType=rgn->GetSubClassType();
}


////////////////////////////////////////////////////////////

YSRESULT SeUndoSetGroundSkyColor::Undo(class SeScenery *scn)
{
	scn->gndColor=orgGndColor;
	scn->skyColor=orgSkyColor;
	return YSOK;
}

YSRESULT SeUndoSetGroundSkyColor::Redo(class SeScenery *scn)
{
	scn->gndColor=newGndColor;
	scn->skyColor=newSkyColor;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoCreateStartPos::Undo(class SeScenery *scn)
{
	scn->stpList.Transfer(stp->thisInTheList,scn->frozenStpList);
	return YSOK;
}

YSRESULT SeUndoCreateStartPos::Redo(class SeScenery *scn)
{
	scn->frozenStpList.Transfer(stp->thisInTheList,scn->stpList);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoDeleteStartPos::Undo(class SeScenery *scn)
{
	scn->frozenStpList.Transfer(stp->thisInTheList,scn->stpList);
	scn->stpList.MoveItemBefore(stp->thisInTheList,prevBeforeThis);
	return YSOK;
}

YSRESULT SeUndoDeleteStartPos::Redo(class SeScenery *scn)
{
	scn->stpList.Transfer(stp->thisInTheList,scn->frozenStpList);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoModifyStartPos::Undo(class SeScenery *scn)
{
	stp->nameStr=orgNameStr;
	stp->pos=orgPos;
	stp->att=orgAtt;
	stp->speed=orgSpeed;
	stp->throttle=orgThrottle;
	// stp->fuel=orgFuel;
	stp->landingGear=orgLandingGear;
	stp->iff=orgIFF;
	return YSOK;
}

YSRESULT SeUndoModifyStartPos::Redo(class SeScenery *scn)
{
	stp->nameStr=newNameStr;
	stp->pos=newPos;
	stp->att=newAtt;
	stp->speed=newSpeed;
	stp->throttle=newThrottle;
	// stp->fuel=newFuel;
	stp->landingGear=newLandingGear;
	stp->iff=newIFF;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoModifyStartPosCarrier::Undo(class SeScenery *scn)
{
	stp->onCarrier=orgOnCarrier;
	stp->carrierTag=orgCarrierTag;
	stp->posOnCarrier=orgPosOnCarrier;
	stp->attOnCarrier=orgAttOnCarrier;
	return YSOK;
}

YSRESULT SeUndoModifyStartPosCarrier::Redo(class SeScenery *scn)
{
	stp->onCarrier=newOnCarrier;
	stp->carrierTag=newCarrierTag;
	stp->posOnCarrier=newPosOnCarrier;
	stp->attOnCarrier=newAttOnCarrier;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoChangeStartPosPriority::Undo(class SeScenery *scn)
{
	scn->stpList.MoveItemBefore(stp->thisInTheList,orgBeforeThis);
	return YSOK;
}

YSRESULT SeUndoChangeStartPosPriority::Redo(class SeScenery *scn)
{
	scn->stpList.MoveItemBefore(stp->thisInTheList,newBeforeThis);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetMotionPathIsLoop::Undo(class SeScenery *scn)
{
	mpa->isLoop=isLoopOrg;
	return YSOK;
}

YSRESULT SeUndoSetMotionPathIsLoop::Redo(class SeScenery *scn)
{
	mpa->isLoop=isLoopNew;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetMotionPathCoord::Undo(class SeScenery *scn)
{
	mpa->SetPointList(mpaOrg.GetN(),mpaOrg);
	return YSOK;
}

YSRESULT SeUndoSetMotionPathCoord::Redo(class SeScenery *scn)
{
	mpa->SetPointList(mpaNew.GetN(),mpaNew);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoChangeDefaultAreaType::Undo(class SeScenery *scn)
{
	scn->areaType=prvAreaType;
	return YSOK;
}

YSRESULT SeUndoChangeDefaultAreaType::Redo(class SeScenery *scn)
{
	scn->areaType=newAreaType;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoChangeGroundSpecular::Undo(class SeScenery *scn)
{
	scn->YsScenery::SetSpecular(specularBefore);
	return YSOK;
}

YSRESULT SeUndoChangeGroundSpecular::Redo(class SeScenery *scn)
{
	scn->YsScenery::SetSpecular(specularAfter);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoChangeMotionPathAreaType::Undo(class SeScenery *scn)
{
	mpa->SetAreaType(prvAreaType);
	return YSOK;
}

YSRESULT SeUndoChangeMotionPathAreaType::Redo(class SeScenery *scn)
{
	mpa->SetAreaType(newAreaType);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetBaseElevation::Undo(class SeScenery *scn)
{
	scn->baseElevation=prvBaseElev;
	return YSOK;
}

YSRESULT SeUndoSetBaseElevation::Redo(class SeScenery *scn)
{
	scn->baseElevation=newBaseElev;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetMagneticVariation::Undo(class SeScenery *scn)
{
	scn->magneticVariation=prvMagneticVariation;
	return YSOK;
}

YSRESULT SeUndoSetMagneticVariation::Redo(class SeScenery *scn)
{
	scn->magneticVariation=newMagneticVariation;
	return YSOK;
}


////////////////////////////////////////////////////////////

YSRESULT SeUndoSetAirRoute::Undo(class SeScenery *scn)
{
	scn->ApplySceneryAndAirRoutePair(prvScnRoutePairArray.GetN(),prvScnRoutePairArray);
	return YSOK;
}

YSRESULT SeUndoSetAirRoute::Redo(class SeScenery *scn)
{
	scn->ApplySceneryAndAirRoutePair(newScnRoutePairArray.GetN(),newScnRoutePairArray);
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetCanResume::Undo(class SeScenery *scn)
{
	scn->canResume=prvCanResume;
	return YSOK;
}

YSRESULT SeUndoSetCanResume::Redo(class SeScenery *scn)
{
	scn->canResume=newCanResume;
	return YSOK;
}

////////////////////////////////////////////////////////////

YSRESULT SeUndoSetCanContinue::Undo(class SeScenery *scn)
{
	scn->canContinue=prvCanContinue;
	return YSOK;
}

YSRESULT SeUndoSetCanContinue::Redo(class SeScenery *scn)
{
	scn->canContinue=newCanContinue;
	return YSOK;
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoAddTexture::Undo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.FreezeTexture(this->texHd);
}

/* virtual */ YSRESULT SeUndoAddTexture::Redo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.MeltTexture(this->texHd);
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoDeleteTexture::Undo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.MeltTexture(this->texHd);
}

/* virtual */ YSRESULT SeUndoDeleteTexture::Redo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.FreezeTexture(this->texHd);
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetTextureFileName::Undo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetTextureFileName(this->texHd,this->oldFn);
}

/* virtual */ YSRESULT SeUndoSetTextureFileName::Redo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetTextureFileName(this->texHd,this->newFn);
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetDrawElemTexture::Undo(class SeScenery *scn)
{
	elm->texLabel=texLabelBefore;
	elm->texLabelNotFound=YSFALSE;
	return YSOK;
}

/* virtual */ YSRESULT SeUndoSetDrawElemTexture::Redo(class SeScenery *scn)
{
	elm->texLabel=texLabelAfter;
	elm->texLabelNotFound=YSFALSE;
	return YSOK;
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndosetDrawElemTextureCoord::Undo(class SeScenery *scn)
{
	elm->texCoord=texCoordBefore;
	return YSOK;
}

/* virtual */ YSRESULT SeUndosetDrawElemTextureCoord::Redo(class SeScenery *scn)
{
	elm->texCoord=texCoordAfter;
	return YSOK;
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetTextureFilterType::Undo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetTextureFilterType(this->texHd,this->filterBefore);
}

/* virtual */ YSRESULT SeUndoSetTextureFilterType::Redo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetTextureFilterType(this->texHd,this->filterAfter);
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetElevationGridTexture::Undo(class SeScenery *scn)
{
	evg->evg.SetTextureLabel(texLabelBefore);
	return YSOK;
}

/* virtual */ YSRESULT SeUndoSetElevationGridTexture::Redo(class SeScenery *scn)
{
	evg->evg.SetTextureLabel(texLabelBefore);
	return YSOK;
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetElevationGridSpecular::Undo(class SeScenery *scn)
{
	evg->evg.SetSpecular(specularBefore);
	return YSOK;
}

/* virtual */ YSRESULT SeUndoSetElevationGridSpecular::Redo(class SeScenery *scn)
{
	evg->evg.SetSpecular(specularAfter);
	return YSOK;
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetTextureRandomNoiseLevel::Undo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetRandomNoiseLevel(this->texHd,this->randomNoiseBefore);
}

/* virtual */ YSRESULT SeUndoSetTextureRandomNoiseLevel::Redo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetRandomNoiseLevel(this->texHd,this->randomNoiseAfter);
}

////////////////////////////////////////////////////////////

/* virtual */ YSRESULT SeUndoSetTextureFileData::Undo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetTextureFileData(this->texHd,fTypeBefore,0,0,fileDataBefore.GetN(),fileDataBefore);
}

/* virtual */ YSRESULT SeUndoSetTextureFileData::Redo(class SeScenery *scn)
{
	auto &texMan=scn->GetTextureManagerInternal(this->scn);
	return texMan.SetTextureFileData(this->texHd,fTypeAfter,0,0,fileDataAfter.GetN(),fileDataAfter);
}

//  Undo<<  //////////////////////////////////////////////////////////
