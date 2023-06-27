#include <memory>

#ifdef WIN32  // Assuming UNIX
#else
#include <sys/time.h>
#endif

#include <ysclass.h>
#include <ysport.h>
#include <yscompilerwarning.h>

#define YSBITMAP_NO_WIN32_INTERFACE
#include <ysbitmap.h>


#define FSSIMPLEWINDOW_MACRO_ONLY
#include <fssimplewindow.h>
#undef FSSIMPLEWINDOW_MACRO_ONLY

#include <ysshellext_orientationutil.h>


#include "fssimextension.h"

#include <fsairproperty.h>

#include "fs.h"

#include "fsgui.h"
#include "fsguiselectiondialogbase.h"
#include "fschoose.h"

#include "fsfilename.h"
#include "platform/common/fswindow.h"

#include "graphics/common/fsopengl.h"

#include "fsgenmdl.h"



static YSRESULT YsLoadFld(YsScenery &scn,const wchar_t fn[])
{
	YsFileIO::File fp(fn,"r"); // File pointer will be closed in the destructor.
	if(NULL!=fp)
	{
		auto curPath=YsFileIO::Getcwd();

		YsWString ful(fn),pth,fil;
		ful.SeparatePathFile(pth,fil);

		YsFileIO::ChDir(pth);
		YSRESULT res=scn.LoadFld(fp);
		scn.CacheMapDrawingOrder();

		YsFileIO::ChDir(curPath);

		return res;
	}
	return YSERR;
}

////////////////////////////////////////////////////////////

YsListAllocator <FsAirplaneTemplate> FsAirplaneTemplateAllocator(16);
YsListAllocator <FsGroundTemplate> FsGroundTemplateAllocator(16);
YsListAllocator <FsFieldTemplate> FsFieldTemplateAllocator(16);

// Implementation //////////////////////////////////////////

FsAirplaneTemplate::FsAirplaneTemplate()
{
	idName.Set("");
	airCat=FSAC_UNKNOWN;

	prop=NULL;
	vis=NULL;
	cockpit=NULL;
	lod=NULL;
	coll=NULL;

	for(int i=0; i<FSWEAPON_NUMWEAPONTYPE; i++)
	{
		weaponShapeOverride[0][i]=NULL;
		weaponShapeOverride[1][i]=NULL;
	}

	SetPropFileName(L"");
	SetVisualFileName(L"");
	SetCockpitFileName(L"");
	SetLodFileName(L"");
	SetCollisionFileName(L"");
}

void FsAirplaneTemplate::Initialize(void)
{
	tmplRootDir.Set(L"");

	idName.Set("");
	airCat=FSAC_UNKNOWN;

	Unprepare();

	SetPropFileName(L"");
	SetVisualFileName(L"");
	SetCockpitFileName(L"");
	SetLodFileName(L"");
	SetCollisionFileName(L"");
}

void FsAirplaneTemplate::Unprepare(void)
{
	vis.CleanUp();
	cockpit.CleanUp();
	lod.CleanUp();
	if(coll!=NULL)
	{
		delete coll;
		coll=NULL;
	}
	if(prop!=NULL)
	{
		delete prop;
		prop=NULL;
	}
	for(int i=0; i<(int)FSWEAPON_NUMWEAPONTYPE; i++)
	{
		weaponShapeOverride[0][i].CleanUp();
		weaponShapeOverride[1][i].CleanUp();
	}
}

FsAirplaneTemplate::~FsAirplaneTemplate()
{
	// fsConsole.Printf("~FsAirplaneTemplate %s %s %s\n",visFileName,cockFileName,lodFileName);
	// fsConsole.Printf("Deleting Airplane Template [%s]\n",prop.GetIdentifier());

	Initialize();
}

static int YsDeleteShrunkPolygon(FsVisualSrf &shl)
{
	int nDel;
	YsShellPolygonHandle plHd;
	YsArray <YsShellVertexHandle> plVtHd;
	nDel=0;
	plHd=NULL;
	while(YSOK==shl.MoveToNextPolygon(plHd))
	{
		if(shl.GetVertexListOfPolygon(plVtHd,plHd)==YSOK)
		{
			int nOrg=(int)plVtHd.GetN();

		STARTOVERAGAIN:
			for(int i=0; i<plVtHd.GetN() && 3<=plVtHd.GetN(); i++)
			{
				if(plVtHd[i]==plVtHd[(i+1)%plVtHd.GetN()])
				{
					plVtHd.Delete(i);
					goto STARTOVERAGAIN;
				}
				else if(plVtHd[i]==plVtHd[(i+2)%plVtHd.GetN()])
				{
					plVtHd.Delete(i);
					plVtHd.Delete(i%plVtHd.GetN());
					goto STARTOVERAGAIN;
				}
			}

			if(plVtHd.GetN()<3)
			{
				shl.DeletePolygon(plHd);
				nDel++;
			}
			else if(plVtHd.GetN()!=nOrg)
			{
				shl.SetPolygonVertex(plHd,plVtHd);
				nDel++;
			}
		}
	}
	return nDel;
}

const wchar_t *FsAirplaneTemplate::GetTemplateRootDirectory(void) const
{
	return tmplRootDir;
}

const wchar_t *FsAirplaneTemplate::GetPropFileName(void) const
{
	if(0!=_propFileName[0])
	{
		return propFullPathName.MakeFullPathName(tmplRootDir,_propFileName);
	}
	return L"";
}

const wchar_t *FsAirplaneTemplate::GetVisualFileName(void) const
{
	if(0!=_visFileName[0])
	{
		return visFullPathName.MakeFullPathName(tmplRootDir,_visFileName);
	}
	return L"";
}

const wchar_t *FsAirplaneTemplate::GetCockpitFileName(void) const
{
	if(0!=_cockFileName[0])
	{
		return cockFullPathName.MakeFullPathName(tmplRootDir,_cockFileName);
	}
	return L"";
}

const wchar_t *FsAirplaneTemplate::GetLodFileName(void) const
{
	if(0!=_lodFileName[0])
	{
		return lodFullPathName.MakeFullPathName(tmplRootDir,_lodFileName);
	}
	return L"";
}

const wchar_t *FsAirplaneTemplate::GetCollisionFileName(void) const
{
	if(0!=_collFileName[0])
	{
		return collFullPathName.MakeFullPathName(tmplRootDir,_collFileName);
	}
	return L"";
}

FsVisualDnm FsAirplaneTemplate::GetVisual(void) const
{
	if(FsIsConsoleServer()==YSTRUE)
	{
		return NULL;
	}

	if(nullptr==vis && GetVisualFileName()[0]!=0)
	{
		vis.Load(GetVisualFileName());
		if(nullptr==vis)
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (GetVisualFileName());
			fsStderr.Printf("Load Error (VISUAL):%s\n",utf8.Txt());
		}
	}
	return vis;
}

FsVisualDnm FsAirplaneTemplate::GetLod(void) const
{
	if(FsIsConsoleServer()==YSTRUE)
	{
		return nullptr;
	}

	if(nullptr==lod && GetLodFileName()[0]!=0)
	{
		lod.Load(GetLodFileName());
		if(nullptr==lod)
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (GetLodFileName());
			fsStderr.Printf("Load Error (LOD):%s\n",utf8.Txt());
		}
	}
	return lod;
}

FsVisualDnm FsAirplaneTemplate::GetCockpit(void) const
{
	if(FsIsConsoleServer()==YSTRUE)
	{
		return nullptr;
	}

	if(nullptr==cockpit && GetCockpitFileName()[0]!=0)
	{
		cockpit.Load(GetCockpitFileName());
		if(nullptr==cockpit)
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (GetCockpitFileName());
			fsStderr.Printf
			    ("Load Error (COCKPIT):%s\n",utf8.Txt());
		}
	}
	return cockpit;
}

const FsVisualSrf *FsAirplaneTemplate::GetCollision(void) const
{
	if(coll==NULL && GetCollisionFileName()[0]!=0)
	{
		YSRESULT res=YSERR;

		coll=new FsVisualSrf;
		if(coll!=NULL)
		{
			YsFileIO::File fp(GetCollisionFileName(),"r");
			if(nullptr!=fp.Fp())
			{
				YsTextFileInputStream inStream(fp);
				YsShellExtReader reader;
				res=reader.MergeSrf(*coll,inStream);
			}
			else
			{
				YsFileIO::File fp(_collFileName,"r"); // Found some add-on airplanes referring to a collision SRF of a default airplanes.
				if(nullptr!=fp)
				{
					YsTextFileInputStream inStream(fp);
					YsShellExtReader reader;
					res=reader.MergeSrf(*coll,inStream);
				}
			}



			YsDeleteShrunkPolygon(*coll);
			YsShellExt_OrientationUtil util;
			if(YSOK==util.RecalculateNormalFromCurrentOrientation(coll->Conv()))
			{
				for(auto plNom : util.GetPolygonNormalPair())
				{
					coll->SetPolygonNormal(plNom.plHd,plNom.nom);
				}
				coll->SetTrustPolygonNormal(YSTRUE);
			}
			else
			{
				coll->SetTrustPolygonNormal(YSFALSE);
			}

			if(prop!=NULL)
			{
				// YsPrintf("Tail Strike Angle is Computed in GetCollision()\n");
				prop->AutoComputeTailStrikePitchAngle(coll->Conv());
			}

			// coll->CheckLeak();
		}

		if(res!=YSOK)
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (GetCollisionFileName());
			fsStderr.Printf("Load Error (COLLISION):%s\n",utf8.Txt());
			if(coll!=NULL)
			{
				delete coll;
				coll=NULL;
			}
		}
	}
	return coll;
}

FsVisualDnm FsAirplaneTemplate::GetWeaponVisual(FSWEAPONTYPE wpnType,int state) const
{
	const FsAirplaneProperty *prop=GetProperty();
	if(wpnType<FSWEAPON_NUMWEAPONTYPE && 0<=state && state<2)
	{
		if(nullptr==weaponShapeOverride[state][(int)wpnType])
		{
			const wchar_t *fn=prop->GetWeaponShapeFile(wpnType,state);
			if(NULL!=fn && 0!=fn[0])
			{
				YsWString ful;
				ful.MakeFullPathName(tmplRootDir,fn);
				weaponShapeOverride[state][(int)wpnType].Load(ful);
				weaponShapeOverride[state][(int)wpnType].SetZeroTransformation();
				weaponShapeOverride[state][(int)wpnType].SetStateOfAllPart(0);
				weaponShapeOverride[state][(int)wpnType].CacheTransformation();
			}
		}
		if(nullptr!=weaponShapeOverride[state][(int)wpnType]) // This check is needed because weaponShapeOverride is not a pointer now.
		{
			return weaponShapeOverride[state][(int)wpnType];
		}
	}
	return nullptr;
}

const FsAirplaneProperty *FsAirplaneTemplate::GetProperty(void) const
{
	if(prop==NULL && GetPropFileName()[0]!=0)
	{
		prop=new FsAirplaneProperty;
		if(prop->LoadProperty(GetPropFileName())!=YSOK)
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (GetPropFileName());
			fsStderr.Printf("Load Error :%s\n",utf8.Txt());
		}

		if(coll!=NULL)
		{
			// YsPrintf("Tail Strike Angle is Computed in GetProperty()\n");
			prop->AutoComputeTailStrikePitchAngle(coll->Conv());
		}
	}
	return prop;
}

void FsAirplaneTemplate::SetRootDir(const wchar_t fn[])
{
	tmplRootDir.Set(fn);
}

void FsAirplaneTemplate::SetPropFileName(const wchar_t fn[])
{
	_propFileName.Set(fn);
}

void FsAirplaneTemplate::SetVisualFileName(const wchar_t fn[])
{
	_visFileName.Set(fn);
}

void FsAirplaneTemplate::SetCockpitFileName(const wchar_t fn[])
{
	_cockFileName.Set(fn);
}

void FsAirplaneTemplate::SetLodFileName(const wchar_t fn[])
{
	_lodFileName.Set(fn);
}

void FsAirplaneTemplate::SetCollisionFileName(const wchar_t fn[])
{
	_collFileName.Set(fn);
}

FsVisualSrf *FsAirplaneTemplate::GetCollision(void)
{
	const FsAirplaneTemplate *templ;
	templ=this;
	templ->GetCollision();
	return coll;
}

FsAirplaneProperty *FsAirplaneTemplate::GetProperty(void)
{
	const FsAirplaneTemplate *templ;
	templ=this;
	templ->GetProperty();
	return prop;
}




// Implementation //////////////////////////////////////////
FsGroundTemplate::FsGroundTemplate()
{
	vis=nullptr;
	lod=nullptr;
	coll=NULL;
	cockpit=nullptr;
	isAircraftCarrier=YSFALSE;

	tmplRootDir.Set(L"");

	SetAircraftCarrierFileName(L"");
	SetVisualFileName(L"");
	SetLodFileName(L"");
	SetCollisionFileName(L"");
}

void FsGroundTemplate::Initialize()
{
	vis.CleanUp();
	lod.CleanUp();
	cockpit.CleanUp();
	if(coll!=NULL)
	{
		delete coll;
		coll=NULL;
	}
	isAircraftCarrier=YSFALSE;
	SetAircraftCarrierFileName(L"");
	SetVisualFileName(L"");
	SetLodFileName(L"");
	SetCollisionFileName(L"");
	// I wasn't initializing collFileList here.  Was it on purpose? > me  2004/07/22
}

const wchar_t *FsGroundTemplate::GetVisualFileName(void) const
{
	if(0!=_visFileName[0])
	{
		return visFullPathName.MakeFullPathName(tmplRootDir,_visFileName);
	}
	return L"";
}

const wchar_t *FsGroundTemplate::GetLodFileName(void) const
{
	if(0!=_lodFileName[0])
	{
		return lodFullPathName.MakeFullPathName(tmplRootDir,_lodFileName);
	}
	return L"";
}

const wchar_t *FsGroundTemplate::GetCollisionFileName(void) const
{
	if(0!=_collFileName[0])
	{
		return collFullPathName.MakeFullPathName(tmplRootDir,_collFileName);
	}
	return L"";
}

const wchar_t *FsGroundTemplate::GetRawCollisionFileName(void) const
{
	if(0!=_collFileName[0])
	{
		return _collFileName;
	}
	return L"";
}

const wchar_t *FsGroundTemplate::GetCockpitFileName(void) const
{
	if(0!=cockpitFileName[0])
	{
		return cockpitFullPathName.MakeFullPathName(tmplRootDir,cockpitFileName);
	}
	return L"";
}

const wchar_t *FsGroundTemplate::GetAircraftCarrierFileName(void) const
{
	if(0!=_aircraftCarrierFileName[0])
	{
		return _aircraftCarrierFileName;
	}
	return L"";
}

const wchar_t *FsGroundTemplate::GetTemplateRootDirectory(void) const
{
	return tmplRootDir;
}

void FsGroundTemplate::SetRootDir(const wchar_t fn[])
{
	tmplRootDir.Set(fn);
}

void FsGroundTemplate::SetVisualFileName(const wchar_t fn[])
{
	_visFileName.Set(fn);
}

void FsGroundTemplate::SetLodFileName(const wchar_t fn[])
{
	_lodFileName.Set(fn);
}

void FsGroundTemplate::SetCollisionFileName(const wchar_t fn[])
{
	_collFileName.Set(fn);
}

void FsGroundTemplate::SetCockpitFileName(const wchar_t fn[])
{
	cockpitFileName.Set(fn);
}

void FsGroundTemplate::SetAircraftCarrierFileName(const wchar_t fn[])
{
	_aircraftCarrierFileName.Set(fn);
}

FsGroundTemplate::~FsGroundTemplate()
{
	vis.CleanUp();
	lod.CleanUp();
	cockpit.CleanUp();
	if(coll!=NULL)
	{
		delete coll;
		coll=NULL;
	}
}




// Implementation //////////////////////////////////////////
FsFieldTemplate::FsFieldTemplate()
{
	raceCourseAvailable=YSFALSE;

	fld=NULL;

	tmplRootDir.Set(L"");

	idName.Set("");
	visFileName.Set(L"");
	yfsFileName.Set(L"");
	startPosFile.Set(0,NULL);
}

void FsFieldTemplate::Initialize(void)
{
	if(fld!=NULL)
	{
		delete fld;
		fld=NULL;
	}
	raceCourseAvailable=YSFALSE;
}

FsFieldTemplate::~FsFieldTemplate()
{
	if(fld!=NULL)
	{
		delete fld;
		fld=NULL;
	}
}

void FsFieldTemplate::MakeEmptyTemplate(void)
{
	if(nullptr!=fld)
	{
		delete fld;
	}
	fld=new YsScenery;
}

class YsScenery *FsFieldTemplate::GetField(void) const
{
	return fld;
}

YSRESULT FsFieldTemplate::LoadField(void)
{
	fld=new YsScenery;
	if(fld!=NULL)
	{
		if(YSOK==YsLoadFld(*fld,GetVisualFileName()))
		{
			fldError=YsScenery::ERROR_NOERROR;
			return YSOK;
		}
		else
		{
			fldError=fld->GetLastError();
			DeleteField();
		}
	}
	return YSERR;
}

void FsFieldTemplate::DeleteField(void)
{
	delete fld;
	fld=NULL;
}

void FsFieldTemplate::SetRootDir(const wchar_t dir[])
{
	tmplRootDir.Set(dir);
}

void FsFieldTemplate::SetIdName(const char name[])
{
	idName.Set(name);
}

void FsFieldTemplate::SetVisualFileName(const wchar_t fn[])
{
	visFileName.Set(fn);
}

void FsFieldTemplate::SetYfsFileName(const wchar_t fn[])
{
	yfsFileName.Set(fn);
}

void FsFieldTemplate::AddStartPosFileLine(const char str[])
{
	startPosFile.Increment();
	startPosFile.GetEnd().Set(str);
}

void FsFieldTemplate::SetRaceCourseFlag(YSBOOL flg)
{
	raceCourseAvailable=flg;
}

YSBOOL FsFieldTemplate::IsRaceCourseAvailable(void) const
{
	return raceCourseAvailable;
}

const char *FsFieldTemplate::GetIdName(void) const
{
	return idName;
}

const wchar_t *FsFieldTemplate::GetVisualFileName(void) const
{
	if(0!=visFileName[0])
	{
		return visFullPathName.MakeFullPathName(tmplRootDir,visFileName);
	}
	return L"";
}

const wchar_t *FsFieldTemplate::GetYfsFileName(void) const
{
	if(0!=yfsFileName[0])
	{
		return yfsFullPathName.MakeFullPathName(tmplRootDir,yfsFileName);
	}
	return L"";
}


int FsFieldTemplate::GetNumStartPosFileLine(void) const
{
	return (int)startPosFile.GetN();
}

const char *FsFieldTemplate::GetStartPosFileLine(int i) const
{
	if(YSTRUE==startPosFile.IsInRange(i))
	{
		return startPosFile[i];
	}
	else
	{
		return NULL;
	}
}

YsScenery::ERRORCODE FsFieldTemplate::GetLastFieldError(void) const
{
	return fldError;
}


////////////////////////////////////////////////////////////

FsStartPosInfo::FsStartPosInfo()
{
	Initialize();
}

void FsStartPosInfo::Initialize()
{
	iff=IFF_ANY;
	idName.Set(1,"");
	cmd.Set(0,NULL);

	onCarrier=YSFALSE;
	carrierTag.Set(1,"");
	relPosCarrier=YsOrigin();
	relAttCarrier=YsZeroAtt();
}

YSRESULT FsStartPosInfo::InterpretPosition(YsVec3 &pos,YsAtt3 &att)
{
	YsString str;
	YsArray <YsString,16> args;
	int i;
	YSRESULT res;

	res=YSERR;
	pos=YsOrigin();
	att=YsZeroAtt();

	for(i=0; i<cmd.GetN(); i++)
	{
		str.Set(cmd[i]);
		if(str.Arguments(args)==YSOK && args.GetN()>=4 && strcmp(args[0],"POSITION")==0)
		{
			double x,y,z;
			FsGetLength(x,args[1]);
			FsGetLength(y,args[2]);
			FsGetLength(z,args[3]);
			pos.Set(x,y,z);
			res=YSOK;
		}
		if(str.Arguments(args)==YSOK && args.GetN()>=4 && strcmp(args[0],"ATTITUDE")==0)
		{
			double h,p,b;
			FsGetAngle(h,args[1]);
			FsGetAngle(p,args[2]);
			FsGetAngle(b,args[3]);
			att.Set(h,p,b);
		}
	}

	return res;
}

////////////////////////////////////////////////////////////

FsAddedFieldInfo::FsAddedFieldInfo()
{
	Initialize();
}

void FsAddedFieldInfo::Initialize(void)
{
	gndArray.ClearDeep();
}

// Implementation //////////////////////////////////////////

FsWorld::FsWorld() :
    airplaneTemplate(FsAirplaneTemplateAllocator),groundTemplate(FsGroundTemplateAllocator),
    fieldTemplate(FsFieldTemplateAllocator)
{
	sim=NULL;
	lastError=ERROR_NOERROR;
	extensionRegistry.reset(new FsSimExtensionRegistry);
	extensionRegistry->RegisterKnownExtension();
}

FsWorld::~FsWorld()
{
	airplaneTemplate.CleanUp();
	groundTemplate.CleanUp();
	fieldTemplate.CleanUp();

	FsAirplaneTemplateAllocator.CollectGarbage();  // CollectGarbage is needed because
	FsGroundTemplateAllocator.CollectGarbage();    // fsConsole may be destroyed earlier than
	FsFieldTemplateAllocator.CollectGarbage();     // Allocators, and cause failure.

	if(NULL!=sim)
	{
		delete sim;
	}
}

FsWorld::ERRORCODE FsWorld::GetLastError(void) const
{
	auto returnCode=lastError;
	lastError=ERROR_NOERROR;
	return returnCode;
}

const wchar_t *FsWorld::GetLastErrorMessage(void) const
{
	return lastErrorMessage;
}

class FsSimulation *FsWorld::GetSimulation(void)
{
	return sim;
}

const class FsSimulation *FsWorld::GetSimulation(void) const
{
	return sim;
}

YSRESULT FsWorld::Save(const wchar_t fn[],
     int airPosPrecision,int airAttPrecision,
     int gndPosPrecision,int gndAttPrecision,
     int wpnPosPrecision,int wpnAttPrecision,
     const double &timeStep)
{
	FILE *fp=YsFileIO::Fopen(fn,"w");
	if(fp!=NULL)
	{
		fprintf(fp,"YFSVERSI %d\n",YSFLIGHT_YFSVERSION);

		if(sim->Save(fp,
		   airPosPrecision,airAttPrecision,
		   gndPosPrecision,gndAttPrecision,
		   wpnPosPrecision,wpnAttPrecision,
		   timeStep)!=YSOK)
		{
			goto ERRTRAP;
		}

		fclose(fp);
		return YSOK;
	}

ERRTRAP:
	if(fp!=NULL)
	{
		fclose(fp);
	}
	return YSERR;
}


static const char *const ysflightCommand[]=
{
	"REM",
	"PLAYRPLN",
	"PLAYRPOS",
	"FIELDNAM",
	"AIRPLANE",
	"STARTPOS",
	"IDENTIFY",
	"INTENTIO",
	"NUMRECOR",
	"BULRECOR",
	"EXPRECOR",
	"AIRPCMND",
	"GROUNDOB",  // Ground Object
	"GRNDCMND",  // Ground Command
	"NUMGDREC",  // Number of Ground Record
	"GDINTENT",  // Ground Object Intention
	"GNDPOSIT",  // Ground Position
	"GNDATTIT",  // Ground Attitude
	"ALLOWAAM",  // ALlow using AAM
	"CLOUDCFG",  // Cloud Config
	"WEATHERX",  // Weather (Wind, etc...)
	"PRTARGET",  // Specify it is a Primary Ground Target
	"ARTARGET",  // Specify it is a Primary Air Target
	"YFSVERSI",  // Version check (added 2001/06/24)

	// Added 2001/06/24
	"MESSAGE_",
	"CAMERA__",
	"SHOWINFO",
	"USERNAME",

	// Added 2002/01/19
	"ENVIRONM",

	// Added 2002/01/22
	"GNDFLAGS",  // Ground Flags

	// Added 2003/11/15
	"MOTNPATH",  // Motion Path

	// Added 2004/05/20
	"IDANDTAG",

	// Added 2004/09/13
	"EXTRAFNC",

	// Added 2005/02/08
	"MSSNGOAL",

	// Added 2005/09/07
	"SLDCLOUD",

	// Added 2007/09/13
	"AIRFLAGS",

	// Added 2007/09/14
	"LANDLWFL",

	// Added 2009/03/27
	"EVTBLOCK",

	// Added 2009/03/29
	"ALLOWGUN",
	"ALLOWAGM",
	"ALLOWBOM",
	"ALLOWRKT",

	// Added 2009/04/16
	"MPATHPNT", // Direct definition of motion path on an object
	"MPATHLOP", // Direct definition of motion path on an object
	"MPATHOFS", // Direct definition of motion path on an object

	// Added 2010/03/31
	"DSABLGND",

	// Added 2012/01/31
	"BEGINATC", // Start ATC-Related definition

	// Added 2014/09/20
	"RELDCMND", // Command for re-supplying weapons.

	// Added 2014/09/27
	"HOMEBASE", // Setting home base.

	// Added 2014/10/26
	"INITPLYR", // First player object  INITPLYR AIR/GND YFSID
	"CANCONTI", // Can continue

	// Added 2014/11/01
	"SIMTITLE", // Simulation Title

	// Added 2017/12/17
	"EXTENSIO",

	NULL
};

YSRESULT FsWorld::Load(const wchar_t fn[])
{
	if(PrepareSimulation()!=YSOK)
	{
		fsStderr.Printf("Cannot Create Simulation.\n");
	}

	if(LoadInternal(fn,YsVec3(0.0,0.0,0.0),YsAtt3(0.0,0.0,0.0))==YSOK)
	{
		return YSOK;
	}
	else
	{
		TerminateSimulation();
		return YSERR;
	}
}

void FsWorld::SetCanContinue(YSBOOL canContinue)
{
	if(NULL!=sim)
	{
		sim->SetCanContinue(canContinue);
	}
}

void FsWorld::RegisterExtension(std::shared_ptr <FsSimExtensionBase> addOnPtr)
{
	if(nullptr!=sim)
	{
		sim->RegisterExtension(addOnPtr);
	}
}

// fieldPos,fieldAtt are only for defining ground object.
// Don't use for other purposes. Don't screw up!
YSRESULT FsWorld::LoadInternal(const wchar_t fn[],const YsVec3 &fieldPos,const YsAtt3 &fieldAtt)
{
	FSIFF iffTab[]=
	{
		FS_IFF0,
		FS_IFF1,
		FS_IFF2,
		FS_IFF3
	};

	YsMatrix4x4 fieldMat;
	fieldMat.Translate(fieldPos);
	fieldMat.Rotate(fieldAtt);


	YsWString relPath,fil;
	YsWString(fn).SeparatePathFile(relPath,fil);

	FsMissionGoal goal;


	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		YsVec3 pos,ev,uv;
		YsAtt3 att;

		int i,nr,version;
		FsFlightRecord record;
		FsGroundRecord gdRecord;
		FsAirplane *air;
		FsGround *gnd;
		const YsSceneryPointSet *mpa;

		char playerPlane[256],playerStart[256],fieldName[256];
		playerPlane[0]=0;
		playerStart[0]=0;
		fieldName[0]=0;

		double t;
		double x,y,z,h,p,b,g;
		int state,vgw,spoiler,gear,flap,brake,smoke,vapor,dmgTolerance,thr,elv,ail,rud,elvTrim;
		int thrVector,thrReverser,bombBay;
		unsigned flags;

		YsString readBuf;
		YsArray <YsString,16> args;

		air=NULL;
		gnd=NULL;
		while(readBuf.Fgets(fp)!=NULL)
		{
			if(YSOK==readBuf.Arguments(args," \t","") && 0<args.GetN())
			{
				int cmd;
				if(YsCommandNumber(&cmd,args[0],ysflightCommand)==YSOK)
				{
					switch(cmd)
					{
					case 0: //"REM",
						break;
					case 1: //"PLAYRPLN",
						fsStderr.Printf("The command PLAYRPLN is no longer supported.");
						break;
					case 2: //"PLAYRPOS"
						fsStderr.Printf("The command PLAYRPOS is no longer supported.");
						break;
					case 3: //"FIELDNAM",
						pos.Set(atof(args[2]),atof(args[3]),atof(args[4]));
						att.Set(atof(args[7]),atof(args[6]),atof(args[7]));
						if(args.GetN()<=8)
						{
							AddField(NULL,args[1],pos,att,YSTRUE,YSTRUE);
						}
						else
						{
							int i,j;


							unsigned int iffControl=~(unsigned int)0;
							YSBOOL gndFire=YSTRUE;
							YSBOOL loadAir=YSTRUE;
							for(i=9; i<args.GetN(); i++)
							{
								if(strncmp(args[i],"IFF:",4)==0)
								{
									iffControl=0;
									for(j=4; args[i][j]!=0; j++)
									{
										if('0'<=args[i][j] && args[i][j]<='9')
										{
											iffControl|=(1<<(args[i][j]-'0'));
										}
									}
								}
								else if(strncmp(args[i],"GNDFIRE:",8)==0)
								{
									FsGetBool(gndFire,((const char *)args[i])+8);
								}
								else if(strncmp(args[i],"LOADAIR:",8)==0)
								{
									FsGetBool(loadAir,((const char *)args[i])+8);
								}
							}
							AddField(NULL,args[1],pos,att,YsStrToBool(args[8]),loadAir,iffControl);
							if(gndFire!=YSTRUE)
							{
								DisableGroundFire();
							}
						}
						strcpy(fieldName,args[1]);
						break;
					case 4: //"AIRPLANE",
						air=AddAirplane(args[1],YsStrToBool(args[2]));
						if(air==NULL)
						{
							int i;
							for(i=3; i<args.GetN(); i++)
							{
								if(strncmp(args[i],"SUBST:",6)==0)
								{
									air=AddAirplane(((const char *)args[i])+6,YsStrToBool(args[2]));
									if(air!=NULL)
									{
										air->isNetSubstitute=YSTRUE;
										air->actualIdName.Set(args[1]);
										break;
									}
								}
							}

							if(air==NULL)
							{
								fsStderr.Printf("Cannot create an airplane %s\n",args[1].Txt());
							}
						}
						gnd=NULL;
						break;
					case 5: //"STARTPOS",
						if(air!=NULL)
						{
							// 2004/05/20 Airplane is not settled here.
							// Just store fieldId and startPosName, and the airplane
							// will be settled everything is loaded. (To link with aircraft carrier)
							// air->startFieldId=atoi(args[1]); 2007/08/16 no longer used.
							air->_startPosition.Set(args[2]);
						}
						break;
					case 6: //"IDENTIFY",
						if(air!=NULL)
						{
							air->iff=iffTab[atoi(args[1])];
						}
						if(gnd!=NULL)
						{
							gnd->iff=iffTab[atoi(args[1])];
						}
						break;
					case 7: //"INTENTIO",
						if(air!=NULL)
						{
							FsAutopilot *ap;
							ap=FsReadIntention(fp);
							air->AddAutoPilot(ap);
							// #### <- Not very good way.  I gotta come up with a better way....
							if(ap!=NULL && ap->Type()==FSAUTOPILOT_DOGFIGHT) // strcmp(ap->WhatItIs(),FsDogfight::ClassName)==0)
							{
								FsDogfight *df;
								df=(FsDogfight *)ap;
								air->gLimit=df->gLimit;
							}
						}
						break;
					case 8: //"NUMRECOR",
						nr=atoi(args[1]);
						version=atoi(args[2]);
						if(3==version || 4==version)
						{
							YsString readBuf;
							YsArray <YsString,16> args;
							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b,&g);
								readBuf.Fgets(fp);
								elvTrim=0;  // To compensate a bug found in 2001/22/24
								rud=0;      // To compensate a bug found in 2001/22/24
								sscanf(readBuf,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
								    &state,
								    &vgw,
								    &spoiler,
								    &gear,
								    &flap,
								    &brake,
								    &smoke,
								    &vapor,
								    &flags,
								    &dmgTolerance,
								    &thr,
								    &elv,
								    &ail,
								    &rud,
								    &elvTrim,
								    &thrVector,
								    &thrReverser,
								    &bombBay);

								if(3==version)
								{
									smoke=(0!=smoke ? 255 : 0);
								}

								readBuf.Fgets(fp);
								if(YSOK==readBuf.Arguments(args) && 0<args.GetN())
								{
									int j,n;
									n=atoi(args[0]);
									record.turret.Alloc(n);
									for(j=0; j<n; j++)
									{
										record.turret[j].h=(float)atof(args[1+j*3]);
										record.turret[j].p=(float)atof(args[2+j*3]);
										record.turret[j].turretState=atoi(args[3+j*3]);
									}
								}

								record.pos.Set(x,y,z);
								record.h=float(h);
								record.p=float(p);
								record.b=float(b);
								record.g=float(g);
								record.state=(unsigned char)state;
								record.vgw=(unsigned char)vgw;
								record.spoiler=(unsigned char)spoiler;
								record.gear=(unsigned char)gear;
								record.flap=(unsigned char)flap;
								record.brake=(unsigned char)brake;
								record.smoke=(unsigned char)smoke;
								record.vapor=(unsigned char)vapor;
								record.flags=(unsigned short)flags;
								record.dmgTolerance=(unsigned char)dmgTolerance;
								record.thr=(unsigned char)thr;
								record.elv=(char)elv;
								record.ail=(char)ail;
								record.rud=(char)rud;
								record.elvTrim=(char)elvTrim;
								record.thrVector=(unsigned char)thrVector;
								record.thrReverser=(unsigned char)thrReverser;
								record.bombBay=(unsigned char)bombBay;
								air->Record(t,record,YSTRUE);
							}
						}
						else if(version==2)
						{
							YsString readBuf;
							YsArray <YsString,16> args;

							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b,&g);
								readBuf.Fgets(fp);
								elvTrim=0;  // To compensate a bug found in 2001/22/24
								rud=0;      // To compensate a bug found in 2001/22/24
								sscanf(readBuf,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
								    &state,
								    &vgw,
								    &spoiler,
								    &gear,
								    &flap,
								    &brake,
								    &smoke,
								    &vapor,
								    &flags,
								    &dmgTolerance,
								    &thr,
								    &elv,
								    &ail,
								    &rud,
								    &elvTrim,
								    &thrVector,
								    &thrReverser,
								    &bombBay);
								record.pos.Set(x,y,z);
								record.h=float(h);
								record.p=float(p);
								record.b=float(b);
								record.g=float(g);
								record.state=(unsigned char)state;
								record.vgw=(unsigned char)vgw;
								record.spoiler=(unsigned char)spoiler;
								record.gear=(unsigned char)gear;
								record.flap=(unsigned char)flap;
								record.brake=(unsigned char)brake;
								record.smoke=(unsigned char)smoke;
								record.vapor=(unsigned char)vapor;
								record.flags=(unsigned short)flags;
								record.dmgTolerance=(unsigned char)dmgTolerance;
								record.thr=(unsigned char)thr;
								record.elv=(char)elv;
								record.ail=(char)ail;
								record.rud=(char)rud;
								record.elvTrim=(char)elvTrim;
								record.thrVector=(unsigned char)thrVector;
								record.thrReverser=(unsigned char)thrReverser;
								record.bombBay=(unsigned char)bombBay;
								air->Record(t,record,YSTRUE);
							}
						}
						else if(version==1)
						{
							YsString readBuf;
							YsArray <YsString,16> args;

							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b,&g);
								readBuf.Fgets(fp);
								elvTrim=0;  // To compensate a bug found in 2001/22/24
								rud=0;      // To compensate a bug found in 2001/22/24
								sscanf(readBuf,"%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
								    &state,
								    &vgw,
								    &spoiler,
								    &gear,
								    &flap,
								    &brake,
								    &smoke,
								    &vapor,
								    &flags,
								    &dmgTolerance,
								    &thr,
								    &elv,
								    &ail,
								    &rud,
								    &elvTrim);
								record.pos.Set(x,y,z);
								record.h=float(h);
								record.p=float(p);
								record.b=float(b);
								record.g=float(g);
								record.state=(unsigned char)state;
								record.vgw=(unsigned char)vgw;
								record.spoiler=(unsigned char)spoiler;
								record.gear=(unsigned char)gear;
								record.flap=(unsigned char)flap;
								record.brake=(unsigned char)brake;
								record.smoke=(unsigned char)smoke;
								record.vapor=(unsigned char)vapor;
								record.flags=(unsigned short)flags;
								record.dmgTolerance=(unsigned char)dmgTolerance;
								record.thr=(unsigned char)thr;
								record.elv=(char)elv;
								record.ail=(char)ail;
								record.rud=(char)rud;
								record.elvTrim=(char)elvTrim;
								record.thrVector=0;
								record.thrReverser=0;
								record.bombBay=0;
								air->Record(t,record,YSTRUE);
							}
						}
						else if(version==0) // Old Version
						{
							YsString readBuf;
							YsArray <YsString,16> args;

							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%d%d%d%d%d%d%d%d%d%d",
								    &state,
								    &vgw,
								    &spoiler,
								    &gear,
								    &flap,
								    &brake,
								    &smoke,
								    &vapor,
								    &flags,
								    &dmgTolerance);
								record.pos.Set(x,y,z);
								record.h=float(h);
								record.p=float(p);
								record.b=float(b);
								record.g=10000.0F;
								record.state=(unsigned char)state;
								record.vgw=(unsigned char)vgw;
								record.spoiler=(unsigned char)spoiler;
								record.gear=(unsigned char)gear;
								record.flap=(unsigned char)flap;
								record.brake=(unsigned char)brake;
								record.smoke=(unsigned char)smoke;
								record.vapor=(unsigned char)vapor;
								record.flags=(flags!=0 ? (FsFlightRecord::FLAGS_AB) : 0);
								record.dmgTolerance=(unsigned char)dmgTolerance;
								record.thr=0;
								record.elv=0;
								record.ail=0;
								record.rud=0;
								record.elvTrim=0;
								record.thrVector=0;
								record.thrReverser=0;
								record.bombBay=0;
								air->Record(t,record,YSTRUE);
							}
						}
						break;
					case 9: // "BULRECOR"
						sim->LoadWeaponRecord(fp);
						break;
					case 10: // "EXPRECOR"
						sim->LoadExplosionRecord(fp);
						break;
					case 11: // "AIRPCMND"
						// 2004/05/20
						// Commands are not sent here.  Just store, and recall them
						// after everything is loaded.
						{
							YsString cmd;
							cmd.Set(readBuf.Txt()+9);
							air->cmdLog.Append(cmd);
						}
						break;
					case 12: //"GROUNDOB",  // Ground Object
						{
							YSBOOL isPlayer=(3<=args.GetN() ? YsStrToBool(args[2]) : YSFALSE);

							gnd=AddGround(args[1],isPlayer);
							if(gnd==NULL)
							{
								fsStderr.Printf("Cannot create a ground object %s\n",(const char *)args[1]);
							}
							air=NULL;
						}
						break;
					case 13: //"GRNDCMND",  // Ground Command
						gnd->SendCommand(readBuf.Txt()+9);
						break;
					case 14: //"NUMGDREC",  // Number of Ground Record
						nr=atoi(args[1]);
						version=atoi(args[2]);
						if(3==version)
						{
							YsString readBuf;
							YsArray <YsString,16> args;

							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%d%d",&state,&dmgTolerance);
								gdRecord.pos.Set(x,y,z);
								gdRecord.h=float(h);
								gdRecord.p=float(p);
								gdRecord.b=float(b);
								gdRecord.state=(unsigned char)state;
								gdRecord.dmgTolerance=(unsigned char)dmgTolerance;

								float h1,p1,b1,h2,p2,b2,h3,p3,b3;
								readBuf.Fgets(fp);
								sscanf(readBuf,"%f%f%f%f%f%f%f%f%f",&h1,&p1,&b1,&h2,&p2,&b2,&h3,&p3,&b3);
								gdRecord.aaaAimh=float(h1);
								gdRecord.aaaAimp=float(p1);
								gdRecord.aaaAimb=float(b1);

								gdRecord.samAimh=float(h1);
								gdRecord.samAimp=float(p1);
								gdRecord.samAimb=float(b1);

								gdRecord.canAimh=float(h1);
								gdRecord.canAimp=float(p1);
								gdRecord.canAimb=float(b1);

								readBuf.Fgets(fp);
								int steering;
								unsigned int leftDoor,rightDoor,rearDoor,brake,lightState;
								sscanf(readBuf,"%d%d%d%d%d%d",&steering,&leftDoor,&rightDoor,&rearDoor,&brake,&lightState);
								gdRecord.steering=(char)steering;
								gdRecord.leftDoor=(unsigned char)leftDoor;
								gdRecord.rightDoor=(unsigned char)rightDoor;
								gdRecord.rearDoor=(unsigned char)rearDoor;
								gdRecord.brake=(unsigned char)brake;
								gdRecord.lightState=(unsigned char)lightState;

								readBuf.Fgets(fp);
								if(YSOK==readBuf.Arguments(args)==YSOK && 0<args.GetN())
								{
									int j,n;
									n=atoi(args[0]);
									gdRecord.turret.Alloc(n);
									for(j=0; j<n; j++)
									{
										gdRecord.turret[j].h=(float)atof(args[1+j*3]);
										gdRecord.turret[j].p=(float)atof(args[2+j*3]);
										gdRecord.turret[j].turretState=atoi(args[3+j*3]);
									}
								}

								gnd->Record(t,gdRecord,YSTRUE);
							}
						}
						else if(version==2)
						{
							YsString readBuf;
							YsArray <YsString,16> args;

							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%d%d",&state,&dmgTolerance);
								gdRecord.pos.Set(x,y,z);
								gdRecord.h=float(h);
								gdRecord.p=float(p);
								gdRecord.b=float(b);
								gdRecord.state=(unsigned char)state;
								gdRecord.dmgTolerance=(unsigned char)dmgTolerance;

								float h1,p1,b1,h2,p2,b2,h3,p3,b3;
								readBuf.Fgets(fp);
								sscanf(readBuf,"%f%f%f%f%f%f%f%f%f",&h1,&p1,&b1,&h2,&p2,&b2,&h3,&p3,&b3);
								gdRecord.aaaAimh=float(h1);
								gdRecord.aaaAimp=float(p1);
								gdRecord.aaaAimb=float(b1);

								gdRecord.samAimh=float(h1);
								gdRecord.samAimp=float(p1);
								gdRecord.samAimb=float(b1);

								gdRecord.canAimh=float(h1);
								gdRecord.canAimp=float(p1);
								gdRecord.canAimb=float(b1);


								readBuf.Fgets(fp);
								if(YSOK==readBuf.Arguments(args)==YSOK && 0<args.GetN())
								{
									int j,n;
									n=atoi(args[0]);
									gdRecord.turret.Alloc(n);
									for(j=0; j<n; j++)
									{
										gdRecord.turret[j].h=(float)atof(args[1+j*3]);
										gdRecord.turret[j].p=(float)atof(args[2+j*3]);
										gdRecord.turret[j].turretState=atoi(args[3+j*3]);
									}
								}

								gdRecord.steering=0;
								gdRecord.leftDoor=0;
								gdRecord.rightDoor=0;
								gdRecord.rearDoor=0;
								gdRecord.brake=0;
								gdRecord.lightState=0;

								gnd->Record(t,gdRecord,YSTRUE);
							}
						}
						else if(version==1)
						{
							YsString readBuf;
							YsArray <YsString,16> args;

							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%d%d",&state,&dmgTolerance);
								gdRecord.pos.Set(x,y,z);
								gdRecord.h=float(h);
								gdRecord.p=float(p);
								gdRecord.b=float(b);
								gdRecord.state=(unsigned char)state;
								gdRecord.dmgTolerance=(unsigned char)dmgTolerance;

								float h1,p1,b1,h2,p2,b2,h3,p3,b3;
								readBuf.Fgets(fp);
								sscanf(readBuf,"%f%f%f%f%f%f%f%f%f",&h1,&p1,&b1,&h2,&p2,&b2,&h3,&p3,&b3);
								gdRecord.aaaAimh=float(h1);
								gdRecord.aaaAimp=float(p1);
								gdRecord.aaaAimb=float(b1);

								gdRecord.samAimh=float(h1);
								gdRecord.samAimp=float(p1);
								gdRecord.samAimb=float(b1);

								gdRecord.canAimh=float(h1);
								gdRecord.canAimp=float(p1);
								gdRecord.canAimb=float(b1);

								gdRecord.steering=0;
								gdRecord.leftDoor=0;
								gdRecord.rightDoor=0;
								gdRecord.rearDoor=0;
								gdRecord.brake=0;
								gdRecord.lightState=0;

								gnd->Record(t,gdRecord,YSTRUE);
							}
						}
						else if(version==0)
						{
							YsString readBuf;
							YsArray <YsString,16> args;

							for(i=0; i<nr; i++)
							{
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf",&t);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf%lf%lf%lf",&x,&y,&z,&h,&p,&b);
								readBuf.Fgets(fp);
								sscanf(readBuf,"%d%d",&state,&dmgTolerance);
								gdRecord.pos.Set(x,y,z);
								gdRecord.h=float(h);
								gdRecord.p=float(p);
								gdRecord.b=float(b);
								gdRecord.state=(unsigned char)state;
								gdRecord.dmgTolerance=(unsigned char)dmgTolerance;

								readBuf.Fgets(fp);
								sscanf(readBuf,"%lf%lf%lf",&h,&p,&b);
								gdRecord.aaaAimh=float(h);
								gdRecord.aaaAimp=float(p);
								gdRecord.aaaAimb=float(b);

								gdRecord.samAimh=gdRecord.aaaAimh;
								gdRecord.samAimp=gdRecord.aaaAimp;
								gdRecord.samAimb=gdRecord.aaaAimb;

								gdRecord.canAimh=gdRecord.aaaAimh;
								gdRecord.canAimp=gdRecord.aaaAimp;
								gdRecord.canAimb=gdRecord.aaaAimb;

								gdRecord.steering=0;
								gdRecord.leftDoor=0;
								gdRecord.rightDoor=0;
								gdRecord.rearDoor=0;
								gdRecord.brake=0;
								gdRecord.lightState=0;

								gnd->Record(t,gdRecord,YSTRUE);
							}
						}
						break;
					case 15: //"GDINTENT",  // Ground Object Intention
						printf("fsworld.cpp\n");
						printf("GDINTENT is not implemented yet.\n");
						break;
					case 16: //"GNDPOSIT"
						FsGetVec3(pos,args.GetN()-1,args.GetArray()+1);
						pos=fieldMat*pos;
						SettleGround(*gnd,pos);
						break;
					case 17: //"GNDATTIT",  // Ground Attitude
						FsGetAtt3(att,args.GetN()-1,args.GetArray()+1);
						ev=att.GetForwardVector();
						uv=att.GetUpVector();
						fieldMat.Mul(ev,ev,0.0);
						fieldMat.Mul(uv,uv,0.0);
						att.SetTwoVector(ev,uv);
						SettleGround(*gnd,att);
						break;
					case 18: //	"ALLOWAAM",  // ALlow using AAM
						sim->AllowAAM(YsStrToBool(args[1]));
						break;

					case 19: //	"CLOUDCFG",  // Cloud Config
						sim->LoadCloud(fp);
						break;

					case 20: //	"WEATHERX",   // Weather (Wind, etc...)
						sim->LoadWeather(fp);
						break;

					case 21: // "PRTARGET",  // Specify it is a Primary Target
						if(gnd!=NULL)
						{
							gnd->primaryTarget=YSTRUE;
						}
						break;

					case 22: // "ARTARGET",  // Specify it is a Primary Air Target
						if(air!=NULL)
						{
							air->primaryTarget=YSTRUE;
						}
						break;

					case 23: // "YFSVERSI"   Version check
						if(atoi(args[1])>YSFLIGHT_YFSVERSION)
						{
							fsStderr.Printf(
							   "YFS version is newer than the version\n"
							   "of this executable.\n"
							   "Please download the newest version.\n");
							   goto ERRTRAP;
						}
						break;
					case 24: // "MESSAGE_"   Radio message (for future compatibility)
						break;
					case 25: // "CAMERA__"   Camera info (for future compatibility)
						break;
					case 26: // "SHOWINFO"   Acrobat info (for future compatibility)
						break;

					case 27: // "USERNAME"
						if(air!=NULL)
						{
							air->name.Set(args[1]);
						}
						break;

					case 28: // 	"ENVIRONM",
						if(strcmp(args[1],"DAY")==0)
						{
							sim->SetEnvironment(FSDAYLIGHT);
						}
						else if(strcmp(args[1],"NIGHT")==0)
						{
							sim->SetEnvironment(FSNIGHT);
						}
						break;

					case 29: //     "GNDFLAGS"
						if(gnd!=NULL)
						{
							gnd->gndFlag=atoi(args[1]);
						}
						break;


					case 30: // "MOTNPATH",  // Motion Path
						mpa=sim->SearchMotionPathByTag(args[1]);
						if(mpa!=NULL)
						{
							if(air!=NULL)
							{
								air->SetMotionPath(mpa);
								air->useMotionPathOffset=YsStrToBool(args[2]);
							}
							if(gnd!=NULL)
							{
								gnd->SetMotionPath(mpa);
								gnd->useMotionPathOffset=YsStrToBool(args[2]);
							}
						}
						break;
					case 31: // "IDANDTAG",
						if(gnd!=NULL)
						{
							gnd->ysfId=atoi(args[1]);
							gnd->name.Set(args[2]);
						}
						if(air!=NULL)
						{
							air->ysfId=atoi(args[1]);
							air->name.Set(args[2]);
						}
						break;
					case 32: // "EXTRAFNC",
						break;


					case 33: // "MSSNGOAL"
						if(goal.SendCommand(readBuf,relPath)!=YSOK)
						{
							fsStderr.Printf("Error in mission goal description:%s\n",readBuf.Txt());
							goto ERRTRAP;
						}
						break;

					case 34: // "SLDCLOUD",
						if(sim->LoadSolidCloud(fp)!=YSOK)
						{
							fsStderr.Printf("Error in reading solid clouds\n");
							goto ERRTRAP;
						}
						break;
					case 35: // "AIRFLAGS ",
						if(air!=NULL)
						{
							air->airFlag=atoi(args[1]);
						}
						break;
					case 36: //"LANDLWFL",
						if(air!=NULL)
						{
							air->landWhenLowFuelThr=atof(args[1]);
						}
						break;
					case 37: //"EVTBLOCK",
						sim->LoadSimulationEvent(fp);  // <- Tentative IDs will be assigned in there.  See below (*1)  2009/03/27
						break;

					case 38: // "ALLOWGUN",
						sim->AllowGun(YsStrToBool(args[1]));
						break;
					case 39: // "ALLOWAGM",
						sim->AllowAGM(YsStrToBool(args[1]));
						break;
					case 40: // "ALLOWBOM",
						sim->AllowBomb(YsStrToBool(args[1]));
						break;
					case 41: // "ALLOWRKT",
						sim->AllowRocket(YsStrToBool(args[1]));
						break;

					case 42: // "MPATHPNT", // Direct definition of motion path on an object
						if(air!=NULL)
						{
							YsVec3 pnt;
							pnt.Set(atof(args[1]),atof(args[2]),atof(args[3]));
							air->motionPathPnt.Append(pnt);
							air->motionPath=NULL;
						}
						if(gnd!=NULL)
						{
							YsVec3 pnt;
							pnt.Set(atof(args[1]),atof(args[2]),atof(args[3]));
							gnd->motionPathPnt.Append(pnt);
							gnd->motionPath=NULL;
						}
						break;
					case 43: // "MPATHLOP", // Direct definition of motion path on an object
						if(air!=NULL)
						{
							air->motionPathIsLoop=YsStrToBool(args[1]);
						}
						if(gnd!=NULL)
						{
							gnd->motionPathIsLoop=YsStrToBool(args[1]);
						}
						break;
					case 44: // "MPATHOFS", // Direct definition of motion path on an object
						if(air!=NULL)
						{
							air->useMotionPathOffset=YsStrToBool(args[1]);
						}
						if(gnd!=NULL)
						{
							gnd->useMotionPathOffset=YsStrToBool(args[1]);
						}
						break;
					case 45: // "DSABLGND",
						DisableGroundFire();
						break;
					case 46: // "BEGINATC", // Start ATC-Related definition
						if(NULL!=air)
						{
							air->GetAirTrafficInfo().Load(sim,*air,fp);
						}
						break;
					case 47: // "RELDCMND", // Command for re-supplying weapons.
						if(NULL!=air)
						{
							air->AddReloadCommand(readBuf.Txt()+9);
						}
						break;
					case 48: // "HOMEBASE", // Setting home base.
						if(3<=args.GetN())
						{
							if(NULL!=air)
							{
								air->SetHomeBaseName(FsSimInfo::StringToBaseType(args[1]),args[2]);
							}
							else if(NULL!=gnd)
							{
								gnd->SetHomeBaseName(FsSimInfo::StringToBaseType(args[1]),args[2]);
							}
						}
						else
						{
							fsStderr.Printf("HOMEBASE tag needs 3 arguments. (eg. HOMEBASE CARRIER MAIN_CARRIER or HOMEBASE AIRPORT MISAWA");
							goto ERRTRAP;
						}
						break;
					case 49: // "INITPLYR", // First player object  INITPLYR AIR/GND YFSID
						if(3<=args.GetN())
						{
							auto objType=FsExistence::StrToType(args[1]);
							int yfsIdent=atoi(args[2]);
							sim->SetFirstPlayerYfsIdent(objType,yfsIdent);
						}
						break;
					case 50: // "CANCONTI" 
						if(2<=args.GetN())
						{
							auto canContinue=YsStrToBool(args[1]);
							sim->SetCanContinue(canContinue);
						}
						break;
					case 51: //	"SIMTITLE", // Simulation Title
						if(2<=args.GetN())
						{
							sim->SetSimulationTitle(args[1]);
						}
						break;

					case 52: // "EXTENSIO"
						if(2<=args.size())
						{
							auto extPtr=sim->FindExtension(args[1]);
							if(nullptr==extPtr)
							{
								extPtr=extensionRegistry->Create(args[1]);
								if(nullptr!=extPtr)
								{
									sim->RegisterExtension(extPtr);
								}
							}
							if(nullptr!=extPtr)
							{
								if(YSOK!=extPtr->ProcessCommand(sim,args))
								{
									fsStderr.Printf("Error from AddOn/Extension\n");
									goto ERRTRAP;
								}
							}
						}
						break;

					default:
						fsStderr.Printf("Unrecognized:%s\n",readBuf.Txt());
						goto ERRTRAP;
					}
				}
				else
				{
					fsStderr.Printf("Unrecognized:%s\n",readBuf.Txt());
					goto ERRTRAP;
				}
			}
		}

		// 2004/05/20
		// Settle all airplanes and recall command after everything is loaded.
		// 2012/02/01
		// Also re-connect ATC & AirTrafficInfo
		// >>
		air=NULL;
		while((air=sim->FindNextAirplane(air))!=NULL)
		{
			if(air->_startPosition[0]!=0)
			{
				SettleAirplane(*air,air->_startPosition);
			}
			air->RecallCommand();
			air->GetAirTrafficInfo().ReconnectAtcAndApproach(sim,air);
		}
		// <<

		// 2009/03/27 >>
		sim->PrepareSimulationEvent();
		// 2009/03/27 <<

		// 2005/02/08
		if(goal.goalFlag!=0)
		{
			SetMissionGoal(goal);
		}

		// 2014/10/26 >>
		// It may make sense to reset player object to the recorded first player object.
		// However, the player must be set to the last player object when the flight record is loaded.
		// It should only be reset to the first player at the beginning of the flight record replay.
		// For the earlier flight record with a player object change, not to worry.  It won't include INITPLYR tag anyway.
		// 2014/10/26 <<
		// 2018/04/15 >>
		// When loading for Replay Previous Flight, the player must not be reset to the first player, and
		// player-change events must be deleted.  Thought about adding a LoadYfsOption class for controlling
		// the behavior, but the correct solution is to do it in "Select Player Aircraft".
		// 2018/04/15 <<

		sim->PrepareReplaySimulation();

		fclose(fp);





		return YSOK;
	}

ERRTRAP:
	if(fp!=NULL)
	{
		fclose(fp);
	}
	return YSERR;
}

const char *FsWorld::GetAirplaneTemplateName(int id) const
{
	if(0<=id && id<airplaneTemplate.GetN())
	{
		YsListItem <FsAirplaneTemplate> *seeker;
		seeker=airplaneTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			return seeker->dat.idName;  // return seeker->dat.GetProperty()->GetIdentifier();  2005/01/23
		}
	}
	return NULL;
}

YSRESULT FsWorld::GetAirplaneTemplateName(char idName[],int id)
{
	if(0<=id && id<airplaneTemplate.GetN())
	{
		YsListItem <FsAirplaneTemplate> *seeker;
		seeker=airplaneTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			strcpy(idName,seeker->dat.idName); // 2005/01/23
			// strcpy(idName,seeker->dat.GetProperty()->GetIdentifier()); 2005/01/23
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsWorld::GetAirplaneTemplateName(YsString &idName,int id)
{
	if(0<=id && id<airplaneTemplate.GetN())
	{
		YsListItem <FsAirplaneTemplate> *seeker;
		seeker=airplaneTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			idName.Set(seeker->dat.idName);
			return YSOK;
		}
	}
	return YSERR;
}

FSAIRPLANECATEGORY FsWorld::GetAirplaneTemplateCategory(int id) const
{
	if(0<=id && id<airplaneTemplate.GetN())
	{
		YsListItem <FsAirplaneTemplate> *seeker;
		seeker=airplaneTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			return seeker->dat.airCat;
		}
	}
	return FSAC_UNKNOWN;
}

FSAIRPLANECATEGORY FsWorld::GetAirplaneTemplateCategory(const char idName[]) const
{
	const FsAirplaneTemplate *tmp;
	tmp=GetAirplaneTemplate(idName);
	if(tmp!=NULL)
	{
		return tmp->airCat;
	}
	return FSAC_UNKNOWN;
}

const wchar_t *FsWorld::GetAirplanePropFileName(const char idName[]) const
{
	const FsAirplaneTemplate *templ;
	templ=GetAirplaneTemplate(idName);
	if(templ!=NULL)
	{
		return templ->GetPropFileName();
	}
	else
	{
		return NULL;
	}
}

YSRESULT FsWorld::LoadAirplaneTemplate(
    const wchar_t rootDir[],const wchar_t prop[],const wchar_t vis[],const wchar_t coll[],const wchar_t cock[],const wchar_t lod[])
{
	YsListItem <FsAirplaneTemplate> *neo;
	// #### YsShell collTest;

	neo=airplaneTemplate.Create();
	if(neo!=NULL)
	{
		neo->dat.Initialize();

		// #### if(neo->dat.prop.LoadProperty(prop)!=YSOK)
		// #### {
		// #### 	fsStderr.Printf("Load Error :%s\n",prop);
		// #### 	delete neo;
		// #### 	return YSERR;
		// #### }

		YsWString propFullPath;
		propFullPath.MakeFullPathName(rootDir,prop);

		FILE *fp=YsFileIO::Fopen(propFullPath,"r");
		if(fp!=NULL)
		{
			YsString str;
			int ac;
			char *av[16];
			char buf[256];
			while(str.Fgets(fp)!=NULL)
			{
				if(strncmp(str,"IDENTIFY",8)==0)
				{
					strncpy(buf,str,255);
					buf[255]=0;
					if(YsArguments(&ac,av,16,buf)==YSOK && ac>=2)
					{
						neo->dat.SetRootDir(rootDir);

						neo->dat.idName.Set(av[1]);
						neo->dat.idName.Capitalize(); // 2004/08/18

						int i;
						for(i=0; neo->dat.idName[i]!=0; i++)  // for-loop 2004/08/18
						{
							if(neo->dat.idName[i]==' ' || neo->dat.idName[i]=='\t')
							{
								neo->dat.idName.Set(i,'_');
							}
						}
					}
				}
				else if(strncmp(str,"CATEGORY",8)==0)
				{
					strncpy(buf,str,255);
					buf[255]=0;
					if(YsArguments(&ac,av,16,buf)==YSOK && ac>=2)
					{
						neo->dat.airCat=FsGetAirplaneCategoryFromString(av[1]);
					}
				}

				if(neo->dat.idName[0]!=0 && neo->dat.airCat!=FSAC_UNKNOWN)
				{
					break;
				}
			}
			fclose(fp);

			if(neo->dat.idName[0]==0 || neo->dat.airCat==FSAC_UNKNOWN)
			{
				airplaneTemplate.Delete(neo);
				return YSERR;
			}
		}
		else
		{
			airplaneTemplate.Delete(neo);
			return YSERR;
		}


		neo->dat.SetPropFileName(prop);

		neo->dat.SetVisualFileName(vis);

		neo->dat.SetCollisionFileName(coll);

		// #### if(collTest.LoadSrf(coll)!=YSOK)
		// #### {
		// #### 	fsStderr.Printf("Load Error :%s\n",coll);
		// #### 	delete neo;
		// #### 	return YSERR;
		// #### }

		// #### int nDel;
		// #### nDel=YsDeleteShrunkPolygon(collTest);
		// #### if(nDel>0)
		// #### {
		// #### 	fsConsole.Printf("[%s] %d polygons are modified or deleted because they are shrunk or flat.\n",coll,nDel);
		// #### }

		// #### YsSetDoubleSidedFlag(collTest);
		// #### if(collTest.AutoComputeNormalAll(YSTRUE,YSTRUE)!=YSOK)
		// #### {
		// #### 	fsConsole.Printf("[%s]\n",coll);
		// #### 	fsConsole.Printf("  Cannot pre-compute normal vector of collision model!\n");
		// #### 	fsConsole.Printf("  At least one bad (flat/shrunk/etc.) polygon is included.\n");
		// #### }

		if(cock!=NULL && cock[0]!=0)
		{
			neo->dat.SetCockpitFileName(cock);
		}

		if(lod!=NULL && lod[0]!=0)
		{
			neo->dat.SetLodFileName(lod);
		}

		// fsConsole.Printf("%s\n",neo->dat.prop.GetIdentifier());

		return YSOK;
	}
	return YSERR;
}

// Added 2001/06/06
const FsAirplaneTemplate *FsWorld::GetAirplaneTemplate(const char idName[]) const
{
	YsListItem <FsAirplaneTemplate> *found;
	found=FindAirplaneTemplate(idName);
	if(found!=NULL)
	{
		return &found->dat;
	}
	else
	{
		return NULL;
	}
}

YsListItem <FsAirplaneTemplate> *FsWorld::FindAirplaneTemplate(const char iIdName[]) const
{
	int i;
	YsListItem <FsAirplaneTemplate> *ptr;
	char idName[256];

	strcpy(idName,iIdName);

	for(i=0; idName[i]!=0; i++)
	{
		if(idName[i]==' ' || idName[i]=='\t')
		{
			idName[i]='_';
		}
	}

	YsCapitalize(idName);

	ptr=NULL;
	while((ptr=airplaneTemplate.FindNext(ptr))!=NULL)
	{
		if(strncmp(ptr->dat.idName,idName,31)==0)
		{
			return ptr;
		}
	}
	return NULL;
}

FsVisualDnm FsWorld::GetAirplaneVisual(const char idName[]) const
{
	YsListItem <FsAirplaneTemplate> *templ;
	templ=FindAirplaneTemplate(idName);
	if(templ!=NULL)
	{
		return templ->dat.GetVisual();
	}
	return nullptr;
}

const FsVisualSrf *FsWorld::GetAirplaneCollision(const char idName[]) const
{
	YsListItem <FsAirplaneTemplate> *templ;
	templ=FindAirplaneTemplate(idName);
	if(templ!=NULL)
	{
		return templ->dat.GetCollision();
	}
	return NULL;
}

const FsVisualDnm FsWorld::GetAirplaneWeaponShapeOverride(const char idName[],FSWEAPONTYPE wpnType,int state) const
{
	YsListItem <FsAirplaneTemplate> *templ;
	templ=FindAirplaneTemplate(idName);
	if(templ!=NULL)
	{
		return templ->dat.GetWeaponVisual(wpnType,state);
	}
	return nullptr;
}

YSRESULT FsWorld::GetFighterList(int &nFig,char *fig[],int maxn) const
{
	return GetAirplaneListByCategory(nFig,fig,maxn,FSAC_FIGHTER);
}

YSRESULT FsWorld::GetAttackerList(int &nFig,char *fig[],int maxn) const
{
	return GetAirplaneListByCategory(nFig,fig,maxn,FSAC_ATTACKER);
}

YSRESULT FsWorld::GetBomberList(int &nFig,char *fig[],int maxn) const
{
	return GetAirplaneListByCategory(nFig,fig,maxn,FSAC_HEAVYBOMBER);
}

YSRESULT FsWorld::GetJetAirlinerList(int &nAir,char *air[],int maxn) const
{
	if(GetAirplaneListByCategory(nAir,air,maxn,FSAC_NORMAL)==YSOK)
	{
		int i;
		for(i=nAir-1; i>=0; i--)
		{
			const FsAirplaneProperty *prop;
			const FsAirplaneTemplate *tmpl;
			tmpl=GetAirplaneTemplate(air[i]);
			prop=tmpl->GetProperty();
			if(prop==NULL || prop->IsJet()!=YSTRUE)
			{
				air[i]=air[nAir-1];
				nAir--;
			}
		}
		if(nAir>0)
		{
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsWorld::GetAirplaneListByCategory(int &nFig,char *fig[],int maxn,FSAIRPLANECATEGORY cat) const
{
	int i;
	YsListItem <FsAirplaneTemplate> *templ;

	i=0;
	templ=NULL;
	while((templ=airplaneTemplate.FindNext(templ))!=NULL)
	{
		if(templ->dat.airCat==cat)
		{
			if(i<maxn-1)
			{
				fig[i]=(char *)templ->dat.idName.GetArray();
				i++;
			}
		}
	}
	nFig=i;
	fig[i]=NULL;
	return YSOK;
}

YSRESULT FsWorld::GetAirplaneListByMultiCategory(int &nFig,char *fig[],int maxn,int nCat,FSAIRPLANECATEGORY cat[]) const
{
	int i;
	YsListItem <FsAirplaneTemplate> *templ;

	i=0;
	templ=NULL;
	while((templ=airplaneTemplate.FindNext(templ))!=NULL)
	{
		if(YsIsIncluded <FSAIRPLANECATEGORY> (nCat,cat,templ->dat.airCat)==YSTRUE)
		{
			if(i<maxn-1)
			{
				fig[i]=(char *)templ->dat.idName.GetArray();
				i++;
			}
		}
	}
	nFig=i;
	fig[i]=NULL;
	return YSOK;
}

YSRESULT FsWorld::GetGroundListByType(YsArray <const char *> &gnd,FSGROUNDTYPE gndType) const
{
	YsListItem <FsGroundTemplate> *templ;

	gnd.Clear();

	templ=NULL;
	while((templ=groundTemplate.FindNext(templ))!=NULL)
	{
		if(templ->dat.prop.chType==gndType)
		{
			gnd.Append(templ->dat.prop.GetIdentifier());
		}
	}
	return YSOK;
}

YSRESULT FsWorld::LoadTemplateAll(void)
{
	InitializationOption opt;
	return LoadTemplate(opt);
}

YSRESULT FsWorld::LoadTemplate(InitializationOption opt)
{
	LoadAirplaneTemplate(opt);
	LoadGroundTemplate(opt);
	LoadFieldTemplate(opt);
	return YSOK;
}

YSRESULT FsWorld::LoadAirplaneTemplate(InitializationOption opt)
{
	fsConsole.Printf("Loading Airplane Templates (aircraft/air*.lst)");
	const wchar_t *userYsflightDir=FsGetUserYsflightDir();
	if(YSTRUE==opt.loadDefAir)
	{
		LoadAirplaneTemplateList(L".",L"aircraft",L"air",L"lst");
	}
	if(YSTRUE==opt.loadUserAir)
	{
		LoadAirplaneTemplateList(userYsflightDir,L"aircraft",L"air",L"lst");
	}
	return YSOK;
}

YSRESULT FsWorld::LoadAirplaneTemplateList(const wchar_t rootDir[],const wchar_t subDir[],const wchar_t prefix[],const wchar_t ext[])
{
	int i;
	YsWString ful;
	YsArray <YsWString> filelist;
	YSRESULT res;

	YsWString dir;
	dir.MakeFullPathName(rootDir,subDir);

	res=YSOK;

	FsFindFileList(filelist,dir,prefix,ext);
	for(i=0; i<filelist.GetN(); i++)
	{
		ful.MakeFullPathName(dir,filelist[i]);

		YsString cStr;
		cStr.EncodeUTF8 <wchar_t> (ful);
		fsConsole.Printf("%s",cStr.Txt());

		FILE *fp=YsFileIO::Fopen(ful,"r");
		if(fp!=NULL)
		{
			char str[256];
			int ac;
			char *av[32];

			// fsConsole.Printf("[Airplane Template]\n");

			res=YSOK;
			while(fgets(str,256,fp)!=NULL)
			{
				if(YsArguments(&ac,av,32,str)==YSOK && ac>=3)
				{
					YsWString propFileName,visFileName,collFileName,cockFileName,lodFileName;
					propFileName.SetUTF8String(av[0]);
					visFileName.SetUTF8String(av[1]);
					collFileName.SetUTF8String(av[2]);
					cockFileName.SetUTF8String(ac>=4 ? av[3] : NULL);
					lodFileName.SetUTF8String(ac>=5 ? av[4] : NULL);
					if(LoadAirplaneTemplate(rootDir,propFileName,visFileName,collFileName,cockFileName,lodFileName)!=YSOK)
					{
						fsStderr.Printf("Cannot Load %s\n",av[0]);
						res=YSERR;
					}
				}
			}
			fclose(fp);

			// Check if all categories are set.
			YsListItem <FsAirplaneTemplate> *seeker;
			seeker=NULL;
			while((seeker=airplaneTemplate.FindNext(seeker))!=NULL)
			{
				if(seeker->dat.airCat==FSAC_UNKNOWN)
				{
					fsStderr.Printf("Category of %s is not set.\n",seeker->dat.GetProperty()->GetIdentifier());
				}
			}
		}
		else
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (ful.GetArray());
			fsStderr.Printf("Cannot Open %s\n",utf8.Txt());
			res=YSERR;
		}
	}

	return res;
}

YSRESULT FsWorld::GetGroundTemplateName(char idName[],int id) const
{
	YsListItem <FsGroundTemplate> *seeker;
	seeker=groundTemplate.GetItemFromId(id);
	if(seeker!=NULL)
	{
		strcpy(idName,seeker->dat.prop.GetIdentifier());
		return YSOK;
	}
	return YSERR;
}

const char *FsWorld::GetGroundTemplateName(int id) const
{
	YsListItem <FsGroundTemplate> *seeker;
	seeker=groundTemplate.GetItemFromId(id);
	if(seeker!=NULL)
	{
		return seeker->dat.prop.GetIdentifier();
	}
	return NULL;
}

YSRESULT FsWorld::LoadGroundTemplate(
    const wchar_t rootDir[],const wchar_t prop[],const wchar_t vis[],const wchar_t coll[],const wchar_t cockFileName[],const wchar_t lodFileName[])
{
	const wchar_t *userYsflightDir=FsGetUserYsflightDir();

	YsListItem <FsGroundTemplate> *neo;
	// YsShell collTest;  Commented out 2004/07/22

	neo=groundTemplate.Create();
	if(neo!=NULL)
	{
		YsWString propFullPath;
		propFullPath.MakeFullPathName(rootDir,prop);

		YsWString aircraftCarrier;
		neo->dat.Initialize();
		if(neo->dat.prop.LoadProperty(propFullPath,aircraftCarrier)!=YSOK)
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (prop);
			fsStderr.Printf("Load Error :%s\n",utf8.Txt());
			delete neo;
			return YSERR;
		}

		if(aircraftCarrier[0]!=0)
		{
			neo->dat.isAircraftCarrier=YSTRUE;
			neo->dat.SetAircraftCarrierFileName(aircraftCarrier);
		}

		neo->dat.SetRootDir(rootDir);

		neo->dat.SetVisualFileName(vis);

		if(lodFileName!=NULL)
		{
			neo->dat.SetLodFileName(lodFileName);
		}
		else
		{
			neo->dat.SetLodFileName(L"");
		}

		if(cockFileName!=NULL)
		{
			neo->dat.SetCockpitFileName(cockFileName);
		}

		neo->dat.SetCollisionFileName(coll);

		// if(collTest.LoadSrf(coll)!=YSOK)   Commented out 2004/07/22
		// {
		// 	fsStderr.Printf("Load Error :%s\n",coll);
		// 	delete neo;
		// 	return YSERR;
		// }
		//
		// int nDel;
		// nDel=YsDeleteShrunkPolygon(collTest);
		// if(nDel>0)
		// {
		// 	fsConsole.Printf("[%s] %d polygons are modified or deleted because they are shrunk or flat.\n",coll,nDel);
		// }
		//
		// YsSetDoubleSidedFlag(collTest);
		// if(collTest.AutoComputeNormalAll(YSTRUE,YSTRUE)!=YSOK)
		// {
		// 	fsConsole.Printf("[%s]\n",coll);
		// 	fsConsole.Printf("  Cannot pre-compute normal vector of collision model!\n");
		// 	fsConsole.Printf("  At least one bad (flat/shrunk/etc.) polygon is included.\n");
		// }

		return YSOK;
	}
	return YSERR;
}

YsListItem <FsGroundTemplate> *FsWorld::FindGroundTemplate(const char iIdName[]) const
{
	int i;
	YsListItem <FsGroundTemplate> *ptr;
	char idName[256];

	strcpy(idName,iIdName);

	for(i=0; idName[i]!=0; i++)
	{
		if(idName[i]==' ' || idName[i]=='\t')
		{
			idName[i]='_';
		}
	}

	YsCapitalize(idName);

	ptr=NULL;
	while((ptr=groundTemplate.FindNext(ptr))!=NULL)
	{
		if(strncmp(ptr->dat.prop.GetIdentifier(),idName,31)==0)
		{
			return ptr;
		}
	}
	return NULL;
}

YSRESULT FsWorld::PrepareGroundVisual(YsListItem <FsGroundTemplate> *templ) const
{
	if(FsIsConsoleServer()!=YSTRUE && templ->dat.lod==nullptr && templ->dat.GetLodFileName()[0]!=0)
	{
		templ->dat.lod.Load(templ->dat.GetLodFileName());
		// Ignore fault
	}

	if(templ->dat.coll==NULL && templ->dat.GetCollisionFileName()[0]!=0)
	{
		YSRESULT res=YSERR;

		templ->dat.coll=new FsVisualSrf;
		if(templ->dat.coll!=NULL)
		{
			YsFileIO::File fp(templ->dat.GetCollisionFileName(),"r"); // fp.Fp() will be closed in the destructor.
			if(nullptr!=fp)
			{
				YsTextFileInputStream inStream(fp);
				YsShellExtReader reader;
				res=reader.MergeSrf(*templ->dat.coll,inStream);
			}
			else
			{
				YsFileIO::File fp(templ->dat.GetRawCollisionFileName(),"r"); // Found some add-on airplanes referring to a collision SRF of a default airplanes.
				if(nullptr!=fp)
				{
					YsTextFileInputStream inStream(fp);
					YsShellExtReader reader;
					res=reader.MergeSrf(*templ->dat.coll,inStream);
				}
			}


			YsDeleteShrunkPolygon(*templ->dat.coll);
			YsShellExt_OrientationUtil util;
			if(YSOK==util.RecalculateNormalFromCurrentOrientation(templ->dat.coll->Conv()))
			{
				for(auto plNom : util.GetPolygonNormalPair())
				{
					templ->dat.coll->SetPolygonNormal(plNom.plHd,plNom.nom);
				}
				templ->dat.coll->SetTrustPolygonNormal(YSTRUE);
			}
			else
			{
				templ->dat.coll->SetTrustPolygonNormal(YSFALSE);
			}
		}

		if(res!=YSOK)
		{
			YsString utf8;
			utf8.EncodeUTF8 <wchar_t> (templ->dat.GetCollisionFileName());
			fsStderr.Printf("Load Error (COLLISION):%s\n",utf8.Txt());
			if(templ->dat.coll!=NULL)
			{
				delete templ->dat.coll;
				templ->dat.coll=NULL;
			}
		}
	}

	if(FsIsConsoleServer()==YSTRUE)
	{
		return YSOK;  // ----------------------Terminator ----------------------
	}

	if(nullptr==templ->dat.cockpit && templ->dat.GetCockpitFileName()[0]!=0)
	{
		templ->dat.cockpit.Load(templ->dat.GetCockpitFileName());
	}

	if(nullptr==templ->dat.vis)
	{
		templ->dat.vis.Load(templ->dat.GetVisualFileName());
	}

	if(nullptr!=templ->dat.vis)
	{
		return YSOK;
	}

	YsString utf8;
	utf8.EncodeUTF8 <wchar_t> (templ->dat.GetVisualFileName());
	fsStderr.Printf("Load Error :%s\n",utf8.Txt());
	return YSERR;
}

YSRESULT FsWorld::LoadGroundTemplate(InitializationOption opt)
{
	fsConsole.Printf("Loading Ground-object Templates (ground/gro*.lst)");
	const wchar_t *userYsflightDir=FsGetUserYsflightDir();
	if(YSTRUE==opt.loadDefGnd)
	{
		LoadGroundTemplateList(L".",L"ground",L"gro",L"lst");
	}
	if(YSTRUE==opt.loadUserGnd)
	{
		LoadGroundTemplateList(userYsflightDir,L"ground",L"gro",L"lst");
	}
	return YSOK;
}

YSRESULT FsWorld::LoadGroundTemplateList(const wchar_t rootDir[],const wchar_t subDir[],const wchar_t prefix[],const wchar_t ext[])
{
	int i;
	YsWString ful;
	YsArray <YsWString> filelist;
	YSRESULT res;

	res=YSOK;

	YsWString dir;
	dir.MakeFullPathName(rootDir,subDir);

	FsFindFileList(filelist,dir,prefix,ext);
	for(i=0; i<filelist.GetN(); i++)
	{
		ful.MakeFullPathName(dir,filelist[i]);

		YsString cStr;
		cStr.EncodeUTF8 <wchar_t> (ful);
		fsConsole.Printf("%s",cStr.Txt());

		FILE *fp=YsFileIO::Fopen(ful,"r");
		if(fp!=NULL)
		{
			char str[256];
			int ac;
			char *av[32];
			YSRESULT res;
			res=YSOK;
			while(fgets(str,256,fp)!=NULL)
			{
				if(YsArguments(&ac,av,32,str)==YSOK && ac>=3)
				{
					YsWString propFileName,visFileName,collFileName,cockFileName,lodFileName;

					propFileName.SetUTF8String(av[0]);
					visFileName.SetUTF8String(av[1]);
					collFileName.SetUTF8String(av[2]);
					cockFileName.SetUTF8String(ac>=4 ? av[3] : NULL);
					lodFileName.SetUTF8String(ac>=5 ? av[4] : NULL);

					if(LoadGroundTemplate(rootDir,propFileName,visFileName,collFileName,cockFileName,lodFileName)!=YSOK)
					{
						fsStderr.Printf("Cannot Load %s\n",av[0]);
						res=YSERR;
					}
				}
			}
			fclose(fp);
		}
		else
		{
			YsString str;
			str.EncodeUTF8 <wchar_t> (ful.GetArray());
			fsStderr.Printf("Cannot load %s\n",str.Txt());
			res=YSERR;
		}
	}
	return res;
}

YSRESULT FsWorld::LoadFieldTemplate(InitializationOption opt)
{
	fsConsole.Printf("Loading Field Templates (scenery/sce*.lst)");

	const wchar_t *userYsflightDir=FsGetUserYsflightDir();
	if(YSTRUE==opt.loadDefField)
	{
		LoadFieldTemplateList(L".",L"scenery",L"sce",L"lst");
		if(GetFieldTemplateName(0)==NULL)
		{
			fsConsole.Printf("Field Not Found. (Old version directory structure?)");
			fsConsole.Printf("Trying scenary/sce*.lst <- Sorry for misspelling.");
			LoadFieldTemplateList(L".",L"scenary",L"sce",L"lst");
		}
	}
	if(YSTRUE==opt.loadUserField)
	{
		LoadFieldTemplateList(userYsflightDir,L"scenery",L"sce",L"lst");
	}
	return YSOK;
}

YSRESULT FsWorld::LoadFieldTemplateList(const wchar_t rootDir[],const wchar_t subDir[],const wchar_t prefix[],const wchar_t ext[])
{
	int i;
	YsWString ful;
	YsArray <YsWString> filelist;
	YSRESULT res;

	res=YSOK;

	YsWString dir;
	dir.MakeFullPathName(rootDir,subDir);

	FsFindFileList(filelist,dir,prefix,ext);
	for(i=0; i<filelist.GetN(); i++)
	{
		ful.MakeFullPathName(dir,filelist[i]);

		YsString cStr;
		cStr.EncodeUTF8 <wchar_t> (ful);
		fsConsole.Printf("%s",(const char *)cStr.Txt());

		FILE *fp=YsFileIO::Fopen(ful,"r");

		// fsConsole.Printf("[Field Template]\n");

		if(fp!=NULL)
		{
			YsString str;
			YsArray <YsString> argv;
			YSRESULT res;
			res=YSOK;
			while(nullptr!=str.Fgets(fp))
			{
				argv=str.Argv();
				if(3<=argv.size())
				{
					YSBOOL raceCourse=YSFALSE;
					YsWString visFileName,stpFileName,yfsFileName;
					visFileName.SetUTF8String(argv[1]);
					stpFileName.SetUTF8String(argv[2]);
					yfsFileName.SetUTF8String(4<=argv.size() ? argv[3].data() : NULL);
					for(auto &arg : argv.Subset(4))
					{
						if(0==arg.STRCMP("AIRRACE"))
						{
							raceCourse=YSTRUE;
						}
					}
					if(LoadFieldTemplate(rootDir,argv[0],visFileName,stpFileName,yfsFileName,raceCourse)!=YSOK)
					{
						fsStderr.Printf("Cannot Load %s\n",argv[0].data());
						res=YSERR;
					}
					// fsConsole.Printf("%s\n",argv[0].data());
				}
			}
			fclose(fp);
		}
	}
	return res;
}

YSRESULT FsWorld::LoadFieldTemplate(const wchar_t rootDir[],const char idName[],const wchar_t vis[],const wchar_t stp[],const wchar_t yfs[],YSBOOL raceCourse)
{
	YsListItem <FsFieldTemplate> *neo;

	neo=fieldTemplate.Create();
	if(neo!=NULL)
	{
		neo->dat.Initialize();

		neo->dat.SetRootDir(rootDir);
		neo->dat.SetRaceCourseFlag(raceCourse);

		neo->dat.SetIdName(idName);
		neo->dat.SetVisualFileName(vis);
		if(NULL!=yfs)
		{
			neo->dat.SetYfsFileName(yfs);
		}

		YsWString stpFullPathName;


		YsString fileLine;
		FILE *fp=YsFileIO::Fopen(stpFullPathName.MakeFullPathName(rootDir,stp),"r");
		// Some add-ons link to the default file.
		if(nullptr==fp)
		{
			fp=YsFileIO::Fopen(stpFullPathName.MakeFullPathName(L".",stp),"r");
		}

		if(fp!=NULL)
		{
			while(fileLine.Fgets(fp)!=NULL)
			{
				fileLine.DeleteTailSpace();
				neo->dat.AddStartPosFileLine(fileLine);
			}
			fclose(fp);
		}
		else
		{
			fsStderr.Printf("Cannot load STP.");
			return YSERR;
		}

		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::GetFieldTemplateName(char idName[],int id) const
{
	if(0<=id && id<fieldTemplate.GetN())
	{
		YsListItem <FsFieldTemplate> *seeker;
		seeker=fieldTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			strcpy(idName,seeker->dat.GetIdName());
			return YSOK;
		}
	}
	return YSERR;
}

YSRESULT FsWorld::GetFieldTemplateName(YsString &idName,int id) const
{
	if(0<=id && id<fieldTemplate.GetN())
	{
		YsListItem <FsFieldTemplate> *seeker;
		seeker=fieldTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			idName.Set(seeker->dat.GetIdName());
			return YSOK;
		}
	}
	return YSERR;
}

const char *FsWorld::GetFieldTemplateName(int id) const
{
	if(0<=id && id<fieldTemplate.GetN())
	{
		YsListItem <FsFieldTemplate> *seeker;
		seeker=fieldTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			return seeker->dat.GetIdName();
		}
	}
	return NULL;
}

YSBOOL FsWorld::IsFieldTemplateRaceCourseAvailable(int id) const
{
	if(0<=id && id<fieldTemplate.GetN())
	{
		YsListItem <FsFieldTemplate> *seeker;
		seeker=fieldTemplate.GetItemFromId(id);
		if(seeker!=NULL)
		{
			return seeker->dat.IsRaceCourseAvailable();
		}
	}
	return YSFALSE;
}

YSRESULT FsWorld::GetFieldStartPositionName(char idName[],const char fld[],int id) const
{
	YsListItem <FsFieldTemplate> *fldTemplate;
	fldTemplate=FindFieldTemplate(fld);
	if(fldTemplate!=NULL)
	{
		YsString buf;
		int i;
		for(i=0; i<fldTemplate->dat.GetNumStartPosFileLine(); i++)
		{
			buf.Set(fldTemplate->dat.GetStartPosFileLine(i));
			buf.DeleteTailSpace();
			if(buf[0]=='N')
			{
				if(id==0)
				{
					strcpy(idName,buf.Txt()+2);
					return YSOK;    // This is OK case.
				}
				else
				{
					id--;
				}
			}
		}
		return YSERR;  // Because the start position is not found
	}
	return YSERR;
}

YSRESULT FsWorld::GetFieldStartPositionName(YsString &idName,const char fld[],int id) const
{
	YsListItem <FsFieldTemplate> *fldTemplate;

	fldTemplate=FindFieldTemplate(fld);
	if(fldTemplate!=NULL)
	{
		YsString buf;
		int i;
		for(i=0; i<fldTemplate->dat.GetNumStartPosFileLine(); i++)
		{
			buf.Set(fldTemplate->dat.GetStartPosFileLine(i));
			buf.DeleteTailSpace();
			if(buf[0]=='N')
			{
				if(id==0)
				{
					idName.Set(buf.Txt()+2);
					return YSOK;    // This is OK case.
				}
				else
				{
					id--;
				}
			}
		}
		return YSERR;  // Because the start position is not found
	}
	return YSERR;
}

YSRESULT FsWorld::CheckStartPositionIsAvailable(int fieldId,const char str[])
{
	if(NULL!=sim)
	{
		return sim->CheckStartPositionIsAvailable(fieldId,str);
	}
	else
	{
		fsStderr.Printf(
		    "FsWorld::CheckStartPositionIsUsed is called before"
		    "FsSimulation is prepared.\n");
		return YSERR;
	}
}

YSRESULT FsWorld::GetStartPositionInfo(FsStartPosInfo &info,const char sta[])
{
	YsString fieldName;
	YsVec3 fieldPos;
	YsAtt3 fieldAtt;

	info.Initialize();

	if(GetLoadedField(fieldName,fieldPos,fieldAtt)==YSOK)
	{
		return GetStartPositionInfo(info,fieldName,sta);
	}
	return YSERR;
}

YSRESULT FsWorld::GetStartPositionInfo(FsStartPosInfo &info,const char fldIdName[],const char stpName[])
{
	YsListItem <FsFieldTemplate> *fldTemplate;
	YsArray <char,256> cmd;

	fldTemplate=FindFieldTemplate(fldIdName);
	if(fldTemplate!=NULL)
	{
		YSRESULT res;
		YsString buf;

		res=YSERR;
		int i;
		for(i=0; i<fldTemplate->dat.GetNumStartPosFileLine(); i++)
		{
			buf.Set(fldTemplate->dat.GetStartPosFileLine(i));
			buf.DeleteTailSpace();
			if(buf[0]=='N' && strcmp(buf.Txt()+2,stpName)==0)
			{
				res=YSOK;
				info.idName.Set(strlen(buf.Txt()+2)+1,buf.Txt()+2);
				break;
			}
		}

		// YsPrintf("STARTPOS: %s\n",info.idName.GetArray());

		if(res!=YSOK)
		{
			return YSERR;
		}

		for(i=i+1; i<fldTemplate->dat.GetNumStartPosFileLine(); i++)
		{
			buf.Set(fldTemplate->dat.GetStartPosFileLine(i));
			if(buf[0]=='N')
			{
				break;
			}
			else if(buf[0]=='C')
			{
				cmd.Set(strlen(buf.Txt()+2)+1,buf.Txt()+2);
				info.cmd.Append(cmd);
				// YsPrintf("COMMAND: %s\n",cmd.GetArray());
			}
			else if(buf[0]=='P')
			{
				int ac;
				char *av[16];
				cmd.Set(strlen(buf.Txt()+2)+1,buf.Txt()+2);
				YsArguments(&ac,av,16,cmd.GetEditableArray());
				if(ac>0)
				{
					if(strcmp(av[0],"IFF")==0)
					{
						info.iff=atoi(av[1]);
						// YsPrintf("IFFLIMIT: %d\n",info.iff);
					}
					if(strcmp(av[0],"CARRIER")==0)
					{
						info.onCarrier=YSTRUE;
						info.carrierTag.Set(strlen(av[1])+1,av[1]);
						FsGetVec3(info.relPosCarrier,3,av+2);
						FsGetAtt3(info.relAttCarrier,3,av+5);
					}
				}
			}
		}

		return YSOK;
	}

	return YSERR;
}

YSRESULT FsWorld::SettleAirplane(FsAirplane &air,const char sta[])
{
	YsString fieldNameBuf;
	const char *fieldName;
	YsVec3 fieldPos;
	YsAtt3 fieldAtt;

	if(GetLoadedField(fieldNameBuf,fieldPos,fieldAtt)==YSOK)
	{
		YsListItem <FsFieldTemplate> *fldTemplate;

		fieldName=fieldNameBuf;

		fldTemplate=FindFieldTemplate(fieldName);
		if(fldTemplate!=NULL)
		{
			int i;
//			YSRESULT res;
//			char buf[256];
//
//			res=YSERR;
//			for(i=0; i<fldTemplate->dat.startPosFile.GetN(); i++)
//			{
//				strcpy(buf,fldTemplate->dat.startPosFile[i].dat);
//				YsStringTail(buf);
//				if(buf[0]=='N' && strcmp(buf+2,sta)==0)
//				{
//					res=YSOK;
//					break;
//				}
//			}
//
//			if(res!=YSOK)
//			{
//				return YSERR;
//			}
//
//			for(i=i+1; i<fldTemplate->dat.startPosFile.GetN(); i++)
//			{
//				strcpy(buf,fldTemplate->dat.startPosFile[i].dat);
//				if(buf[0]=='N')
//				{
//					break;
//				}
//				else if(buf[0]=='C')
//				{
//					if(air.Prop().SendCommand(buf+2)!=YSOK)
//					{
//						res=YSERR;
//					}
//				}
//				else if(buf[0]=='P')
//				{
//				}
//			}

			FsStartPosInfo info;
			if(GetStartPositionInfo(info,sta)==YSOK)
			{
				YSBOOL settledOnCarrier;
				settledOnCarrier=YSFALSE;
				if(info.onCarrier==YSTRUE)
				{
					FsGround *carrier;
					FsAircraftCarrierProperty *carrierProp;
					if((carrier=sim->FindGroundByTag(info.carrierTag))!=NULL &&
					   (carrierProp=carrier->Prop().GetAircraftCarrierProperty())!=NULL)
					{
						YsMatrix4x4 tfm;
						tfm.Translate(carrier->GetPosition());
						tfm.Rotate(carrier->GetAttitude());

						YsVec3 p,ev,uv;
						YsAtt3 a;

						p=info.relPosCarrier;
						ev=info.relAttCarrier.GetForwardVector();
						uv=info.relAttCarrier.GetUpVector();

						tfm.Mul(p,p,1.0);
						tfm.Mul(ev,ev,0.0);
						tfm.Mul(uv,uv,0.0);

						a.SetTwoVector(ev,uv);

						carrierProp->LoadAirplane(&air);
						air.Prop().SetFlightState(FSGROUND,FSDIEDOF_NULL);

						// Force airplane touch the surface.  >> 2005/03/27
						double yStat,yDeck;
						YsVec3 deckNom;
						yDeck=carrierProp->GetDeckHeightAndNormal(deckNom,p);
						yStat=air.Prop().GetGroundStandingHeight();

						p.SetY(yDeck+yStat);
						// << 2005/03/27


						air.Prop().SetPosition(p);
						air.Prop().SetAttitude(a);

						air.Prop().SetVelocity(YsOrigin());
						settledOnCarrier=YSTRUE;
					}
				}
				else if(YSTRUE==air.Prop().IsOnCarrier() && NULL!=air.Prop().OnThisCarrier())
				{
					FsGround *carrier=air.Prop().OnThisCarrier();
					FsAircraftCarrierProperty *carrierProp=carrier->Prop().GetAircraftCarrierProperty();
					if(NULL!=carrierProp)
					{
						carrierProp->UnloadAirplane(&air);
					}
				}

				air.Prop().SetVelocity(YsOrigin());  // 2006/07/24 Band-aid for speed problem.

				for(i=0; i<info.cmd.GetN(); i++)
				{
					if(settledOnCarrier!=YSTRUE || strncmp(info.cmd[i],"POSITION",8)!=0)
					{
						air.Prop().SendCommand(info.cmd[i]);
					}
				}

				YsMatrix4x4 fldMat;
				fldMat.Translate(fieldPos);
				fldMat.Rotate(fieldAtt);

				YsVec3 ev,uv;
				YsVec3 pos;
				YsAtt3 att;
				pos=air.GetPosition();
				att=air.GetAttitude();
				ev=att.GetForwardVector();
				uv=att.GetUpVector();

				pos=fldMat*pos;
				fldMat.Mul(ev,ev,0.0);
				fldMat.Mul(uv,uv,0.0);
				att.SetTwoVector(ev,uv);
				air.Prop().SetPosition(pos);
				air.Prop().SetAttitude(att);

				air._startPosition.Set(sta);
				return YSOK;
			}
		}
	}
	return YSERR;
}

YSRESULT FsWorld::GetLandingPracticeStartPosition(
    YSBOOL &leftTraffic,FsGround *&ils,double &hdg,
    FsAirplane &air,YsVec3 &pos,YsAtt3 &att,YsVec3 &tdPos,YsAtt3 &tdAtt,
    double &initSpd,FSTRAFFICPATTERNLEG leg)
// leg  0:Final 1:Dog 2:Base
{
	double finalLength,reqRwLength,iniAlt;
	YsVec3 rwDir;
	YsArray <FsGround *> ilsCandidate;

	iniAlt=1800.0*0.3048;  // 1800ft AGL
	finalLength=iniAlt/tan(YsTan3deg);  // 1800ft AGL
	reqRwLength=air.Prop().GetEstimatedRequiredRunwayLength();

	ils=NULL;
	while((ils=sim->FindNextGround(ils))!=NULL)
	{
		if(ils->Prop().GetAircraftCarrierProperty()!=NULL)
		{
			if(ils->gndFlag&FSGNDFLAG_PRIMARYLANDINGTARGET)
			{
				ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,tdAtt);
				tdAtt.Mul(rwDir,-YsZVec());
				if(sim->CheckRunwayLength(tdPos,rwDir,reqRwLength)==YSTRUE)
				{
					ilsCandidate.Append(ils);
				}

				ilsCandidate.Append(ils);
			}
		}
	}

	if(ilsCandidate.GetN()==0)
	{
		return YSERR;
	}

	ils=ilsCandidate[rand()%ilsCandidate.GetN()];
	ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,tdAtt);
	tdAtt.Mul(rwDir,-YsZVec());
	rwDir.SetY(0.0);
	rwDir.Normalize();
	hdg=fmod(tdAtt.h()+YsPi,YsPi*2.0);



	double Glat;
	double bankLimit,glideSlope,groundHeight,landingSpeed;
	double turnRadius;

	bankLimit=YsDegToRad(25.0);
	glideSlope=YsDegToRad(3.0);
	groundHeight=air.Prop().GetGroundStandingHeight();
	landingSpeed=air.Prop().GetEstimatedLandingSpeed();

	Glat=tan(bankLimit);
	turnRadius=YsSqr(landingSpeed)/(Glat*FsGravityConst);

	initSpd=landingSpeed*1.4;

	switch(leg)
	{
	default:
		break;
	case FSLEG_FINAL: // On Final
		{
			pos=tdPos-rwDir*finalLength;
			pos.SetY(iniAlt);
			att=YsZeroAtt();
			att.SetForwardVector(rwDir);
			leftTraffic=YSTRUE;
			return YSOK;
		}
		break;
	case FSLEG_DOG: // On Dog-leg
		{
			pos=tdPos-rwDir*finalLength;
			pos.SetY(iniAlt);

			YsVec3 dogLegDir;
			dogLegDir=rwDir;
			if(rand()%100<50)
			{
				dogLegDir.RotateXZ(YsPi/6.0);
				leftTraffic=YSFALSE;
			}
			else
			{
				dogLegDir.RotateXZ(-YsPi/6.0);
				leftTraffic=YSTRUE;
			}

			pos-=dogLegDir*initSpd*30.0;

			att=YsZeroAtt();
			att.SetForwardVector(dogLegDir);
			return YSOK;
		}
		break;
	case FSLEG_BASE: // On Base
		{
			pos=tdPos-rwDir*finalLength*1.5;
			pos.SetY(iniAlt);

			YsVec3 dogLegDir;
			dogLegDir=rwDir;
			if(rand()%100<50)
			{
				dogLegDir.RotateXZ(YsPi/2.0);
				leftTraffic=YSFALSE;
			}
			else
			{
				dogLegDir.RotateXZ(-YsPi/2.0);
				leftTraffic=YSTRUE;
			}

			pos-=dogLegDir*initSpd*60.0;

			att=YsZeroAtt();
			att.SetForwardVector(dogLegDir);
			return YSOK;
		}
		break;
	}
	return YSERR;
}

YSRESULT FsWorld::SettleAirplaneForLandingDemo(FsAirplane &air,const double &rwLength,const double &bankLimitOverride)
{
	FsGround *gnd;
	YsArray <FsGround *> ilsCandidate;

	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->Prop().GetAircraftCarrierProperty()!=NULL)
		{
			if(gnd->gndFlag&FSGNDFLAG_PRIMARYLANDINGTARGET)
			{
				ilsCandidate.Append(gnd);
			}
		}
	}

	if(ilsCandidate.GetN()==0)  // When no candidates is found.
	{
		gnd=NULL;
		while((gnd=sim->FindNextGround(gnd))!=NULL)
		{
			if(gnd->Prop().GetAircraftCarrierProperty()!=NULL)
			{
				YsVec3 tdPos,rwDir;
				YsAtt3 rwAtt;
				gnd->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);

				rwAtt.Mul(rwDir,-YsZVec());
				if(sim->CheckRunwayLength(tdPos,rwDir,rwLength)==YSTRUE)
				{
					ilsCandidate.Append(gnd);
				}
			}
		}
	}

	fsConsole.Printf("%d ils candidates.\n",(int)ilsCandidate.GetN());

	if(ilsCandidate.GetN()>0)
	{
		int i,r;
		FsLandingAutopilot *ap;

		ap=FsLandingAutopilot::Create();
		air.SetAutopilot(ap);
		ap->SetAirplaneInfo(air,bankLimitOverride);

		for(i=0; i<ilsCandidate.GetN(); i++)
		{
			r=rand()%ilsCandidate.GetN();

			YsVec3 initialPos[2];
			YsAtt3 initialAtt[2];
			YsArray <YsVec3,16> path[2];
			double initialSpeed;

			if(ap->GetApproachPath(initialSpeed,initialPos,initialAtt,path,ilsCandidate[r])==YSOK)
			{
				int j;
				double maxElv[2];
				maxElv[0]=0.0;
				maxElv[1]=0.0;
				for(j=0; j<2; j++)
				{
					int k;
					for(k=0; k<path[j].GetN()-1; k++)
					{
						int m;
						for(m=0; m<3; m++)
						{
							YsVec3 tst;
							double t,elv;
							t=double(m)/3.0;
							tst=path[j][k]*(1.0-t)+path[j][k+1]*t;
							elv=sim->GetFieldElevation(tst.x(),tst.z());
							if(maxElv[j]<elv)
							{
								maxElv[j]=elv;
							}
						}
					}
				}

				int take;
				if(YsEqual(maxElv[0],maxElv[1])==YSTRUE)
				{
					if((rand()%100)>50)
					{
						take=1;
					}
					else
					{
						take=0;
					}
				}
				else if(maxElv[0]<maxElv[1])
				{
					take=0;
				}
				else
				{
					take=1;
				}

				air.Prop().SendCommand("CTLLDGEA FALSE");
				air.Prop().SetPosition(initialPos[take]);
				air.Prop().SetAttitude(initialAtt[take]);

				YsVec3 spd;
				spd=YsZVec();
				initialAtt[take].Mul(spd,spd);
				spd*=initialSpeed;
				air.Prop().SetVelocity(spd);

				air.Prop().SendCommand("INITFUEL 15%");


				if(air.Prop().GetNumWeapon(FSWEAPON_BOMB)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_BOMB,2);
				}
				if(air.Prop().GetNumWeapon(FSWEAPON_ROCKET)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_ROCKET,2);
				}
				if(air.Prop().GetNumWeapon(FSWEAPON_AIM9)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_AIM9,2);
				}
				if(air.Prop().GetNumWeapon(FSWEAPON_AGM65)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_AGM65,2);
				}

				ap->SetIls(air,sim,ilsCandidate[r]);

				return YSOK;
			}
			else
			{
				fsConsole.Printf("GetApproachPath returned error!\n");
			}
		}
	}
	return YSERR;
}

YSRESULT FsWorld::SettleAirplaneForCarrierLandingDemo(FsAirplane &air)
{
	FsGround *gnd;
	YsArray <FsGround *> ilsCandidate;
	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->gndFlag&FSGNDFLAG_PRIMARYCARRIER)
		{
			ilsCandidate.Append(gnd);
		}
	}

	if(ilsCandidate.GetN()==0)  // Only when no candidate is found.
	{
		gnd=NULL;
		while((gnd=sim->FindNextGround(gnd))!=NULL)
		{
			if(strcmp(gnd->Prop().GetIdentifier(),"AIRCRAFTCARRIER")==0 &&
			   gnd->Prop().GetAircraftCarrierProperty()!=NULL)
			{
				ilsCandidate.Append(gnd);
			}
		}
	}

	fsConsole.Printf("%d ils candidates.\n",(int)ilsCandidate.GetN());

	if(ilsCandidate.GetN()>0)
	{
		int i,r;
		FsLandingAutopilot *ap;

		ap=FsLandingAutopilot::Create();
		air.SetAutopilot(ap);
		ap->landingSpeedCorrection=1.1;  // <- must come before SetAirplaneInfo
		ap->SetAirplaneInfo(air,YsPi/2.0);
		ap->glideSlope=YsDegToRad(5.0);
		ap->flareAlt=0.1;
		ap->wheelControlOnGround=YSFALSE;

		for(i=0; i<ilsCandidate.GetN(); i++)
		{
			r=rand()%ilsCandidate.GetN();

			YsVec3 initialPos[2];
			YsAtt3 initialAtt[2];
			YsArray <YsVec3,16> path[2];
			double initialSpeed;

			if(ap->GetApproachPath(initialSpeed,initialPos,initialAtt,path,ilsCandidate[r])==YSOK)
			{
				FsGround *ils;
				double initElv;
				YsVec3 tdPos,startPos;
				YsAtt3 rwAtt;

				initElv=initialPos[0].y();
				ils=ilsCandidate[r];
				ils->Prop().GetAircraftCarrierProperty()->GetILS().GetLandingPositionAndAttitude(tdPos,rwAtt);

				rwAtt.SetP(0.0);
				rwAtt.SetH(rwAtt.h()+YsPi);

				startPos.Set(0.0,0.0,-initElv*3.0);
				rwAtt.Mul(startPos,startPos);
				startPos+=tdPos;
				startPos.SetY(initElv);

				air.Prop().SendCommand("CTLLDGEA FALSE");
				air.Prop().SetPosition(startPos);
				air.Prop().SetAttitude(rwAtt);

				YsVec3 spd;
				spd=YsZVec();
				rwAtt.Mul(spd,spd);
				spd*=initialSpeed;
				air.Prop().SetVelocity(spd);

				air.Prop().SendCommand("INITFUEL 15%");
				if(air.Prop().GetNumWeapon(FSWEAPON_BOMB)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_BOMB,2);
				}
				if(air.Prop().GetNumWeapon(FSWEAPON_ROCKET)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_ROCKET,2);
				}
				if(air.Prop().GetNumWeapon(FSWEAPON_AIM9)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_AIM9,2);
				}
				if(air.Prop().GetNumWeapon(FSWEAPON_AGM65)>2)
				{
					air.Prop().SetNumWeapon(FSWEAPON_AGM65,2);
				}

				ap->SetIls(air,sim,ils);

				return YSOK;
			}
			else
			{
				printf("GetApproachPath returned error!\n");
			}
		}
	}
	return YSERR;
}

FsGround *FsWorld::PickRandomPrimaryIls(void)
{
	FsGround *gnd;
	YsArray <FsGround *> ilsCandidate;

	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->Prop().GetAircraftCarrierProperty()!=NULL)
		{
			if(gnd->gndFlag&FSGNDFLAG_PRIMARYLANDINGTARGET)
			{
				ilsCandidate.Append(gnd);
			}
		}
	}

	if(ilsCandidate.GetN()>0)
	{
		return ilsCandidate[rand()%ilsCandidate.GetN()];
	}
	return NULL;
}

YSRESULT FsWorld::SettleGround(FsGround &gnd,const YsVec3 &pos)
{
	return gnd.Settle(pos);
}

YSRESULT FsWorld::SettleGround(FsGround &gnd,const YsAtt3 &att)
{
	return gnd.Settle(att);
}

YsListItem <FsFieldTemplate> *FsWorld::FindFieldTemplate(const char iIdName[]) const
{
	int i;
	YsListItem <FsFieldTemplate> *ptr;
	char idName[256];
	strcpy(idName,iIdName);

	for(i=0; idName[i]!=0; i++)
	{
		if(idName[i]==' ' || idName[i]=='\t')
		{
			idName[i]='_';
		}
	}

	ptr=NULL;
	while((ptr=fieldTemplate.FindNext(ptr))!=NULL)
	{
		if(strcmp(ptr->dat.GetIdName(),idName)==0)
		{
			return ptr;
		}
	}
	return NULL;
}

YSRESULT FsWorld::GetRunwayRectFromPosition(const YsSceneryRectRegion *&rgn,YsVec3 rect[4],const YsVec3 &pos)
{
	if(NULL!=sim)
	{
		if(sim->GetRunwayRectFromPosition(rgn,rect,pos)==YSOK)
		{
			return YSOK;
		}
	}

	rgn=NULL;
	return YSERR;
}

YSRESULT FsWorld::PrepareSimulation(void)
{
	if(sim==NULL)
	{
		sim=new FsSimulation(this);
		if(sim==NULL)
		{
			return YSERR;
		}
	}

	sim->LoadConfigFile(FsGetConfigFile(),YSTRUE); // YSTRUE:ChangeEnvironment
	return YSOK;
}

YSRESULT FsWorld::SetMissionGoal(class FsMissionGoal &goal)
{
	if(NULL!=sim)
	{
		return sim->SetMissionGoal(goal);
	}
	return YSERR;
}

YSRESULT FsWorld::TieDownCarrier(void)
{
	FsGround *gnd;
	gnd=NULL;
	while((gnd=sim->FindNextGround(gnd))!=NULL)
	{
		if(gnd->Prop().GetAircraftCarrierProperty()!=NULL)
		{
			gnd->SendCommand("MAXSPEED 0kt");
			gnd->SendCommand("MAXROTAT 0deg");
		}
	}
	return YSOK;
}

YSRESULT FsWorld::DisableGroundFire(void)
{
	printf("FsWorld::DisableGroundFire()\n");
	if(NULL!=sim)
	{
		FsGround *gnd;
		gnd=NULL;
		while((gnd=sim->FindNextGround(gnd))!=NULL)
		{
			if(YSTRUE==gnd->Prop().HasWeapon())
			{
				gnd->SendCommand("INITIGUN 0");
				gnd->SendCommand("INITISAM 0");
				gnd->SendCommand("MAXSPEED 0kt");
				gnd->SendCommand("MAXROTAT 0deg");
			}
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::SetEnvironment(FSENVIRONMENT env)
{
	if(NULL!=sim)
	{
		sim->SetEnvironment(env);
		return YSERR;
	}
	return YSOK;
}

FSENVIRONMENT FsWorld::GetEnvironment(void)
{
	if(NULL!=sim)
	{
		return sim->GetEnvironment();
	}
	return FSDAYLIGHT;
}

FsAirplane *FsWorld::AddAirplane(const char idName[],YSBOOL isPlayerPlane,unsigned netSearchKey)
{
	if(NULL!=sim)
	{
		FsAirplane neo;
		YsListItem <FsAirplaneTemplate> *ptr;

		ptr=FindAirplaneTemplate(idName);
		if(ptr!=NULL)
		{
			// When adding an airplane, GetCollision must be called before GetProperty,
			// otherwise, chTailStrikePitchAngle and chGroundStatickPitchAngle is not set.

			if(NULL!=ptr->dat.GetCollision())
			{
				neo.SetCollisionShell(*ptr->dat.GetCollision());
			}

			neo.SetProperty(*ptr->dat.GetProperty(),ptr->dat.GetTemplateRootDirectory());
			neo.vis=ptr->dat.GetVisual();
			neo.lod=ptr->dat.GetLod();

			for(int i=0; i<(int)FSWEAPON_NUMWEAPONTYPE; i++)
			{
				neo.weaponShapeOverrideStatic[i]=ptr->dat.GetWeaponVisual((FSWEAPONTYPE)i,0);
				neo.weaponShapeOverrideFlying[i]=ptr->dat.GetWeaponVisual((FSWEAPONTYPE)i,1);
			}

			neo.cockpit=ptr->dat.GetCockpit();

			return sim->AddAirplane(neo,isPlayerPlane,ptr->dat.GetTemplateRootDirectory(),netSearchKey);
		}
	}
	return NULL;
}

FsAirplane *FsWorld::ResetAirplane(FsAirplane *air)
{
	if(NULL!=sim && air!=NULL)
	{
		FsAirplane neo;
		YsListItem <FsAirplaneTemplate> *ptr;

		ptr=FindAirplaneTemplate(air->Prop().GetIdentifier());
		if(ptr!=NULL)
		{
			if(YSTRUE==air->Prop().IsOnCarrier())
			{
				FsGround *onThisCarrier=air->Prop().OnThisCarrier();
				if(NULL!=onThisCarrier)
				{
					FsAircraftCarrierProperty *carrierProp=onThisCarrier->Prop().GetAircraftCarrierProperty();
					if(NULL!=carrierProp)
					{
						carrierProp->UnloadAirplane(air);
					}
				}
			}

			air->SetCollisionShell(*ptr->dat.GetCollision());

			air->SetProperty(*ptr->dat.GetProperty(),ptr->dat.GetTemplateRootDirectory());
			air->vis=ptr->dat.GetVisual();
			air->lod=ptr->dat.GetLod();

			air->cockpit=ptr->dat.GetCockpit();

			return air;
		}
	}
	return NULL;
}

FsAirplane *FsWorld::AddMatchingAirplane(
    FSAIRCRAFTCLASS airClass,FSAIRPLANECATEGORY airCategory,unsigned int /*nationality*/,YSBOOL isJet,const double &dimension,
    YSBOOL isPlayerPlane,unsigned netSearchKey)
{
	if(NULL!=sim)
	{
		YsListItem <FsAirplaneTemplate> *ptr,*match;
		double matchDiff,diff;

		match=NULL;
		matchDiff=0.0;

		ptr=NULL;
		while(NULL!=(ptr=airplaneTemplate.FindNext(ptr)))
		{
			if(ptr->dat.GetProperty()->GetAircraftClass()==airClass &&
			   ptr->dat.GetProperty()->GetAirplaneCategory()==airCategory &&
			   ptr->dat.GetProperty()->IsJet()==isJet)
			{
				diff=fabs(ptr->dat.GetProperty()->GetOutsideRadius()-dimension);
				if(match==NULL || diff<matchDiff)
				{
					match=ptr;
					matchDiff=diff;
				}
			}
		}

		if(match==NULL)  // No template with same class+category
		{
			ptr=NULL;
			while(NULL!=(ptr=airplaneTemplate.FindNext(ptr)))
			{
				if(ptr->dat.GetProperty()->GetAircraftClass()==airClass &&
				   ptr->dat.GetProperty()->IsJet()==isJet)
				{
					diff=fabs(ptr->dat.GetProperty()->GetOutsideRadius()-dimension);
					if(match==NULL || diff<matchDiff)
					{
						match=ptr;
						matchDiff=diff;
					}
				}
			}
		}

		if(match==NULL)  // No template with same class+category+Jet/Prop
		{
			ptr=NULL;
			while(NULL!=(ptr=airplaneTemplate.FindNext(ptr)))
			{
				if(ptr->dat.GetProperty()->GetAircraftClass()==airClass)
				{
					diff=fabs(ptr->dat.GetProperty()->GetOutsideRadius()-dimension);
					if(match==NULL || diff<matchDiff)
					{
						match=ptr;
						matchDiff=diff;
					}
				}
			}
		}

		if(match==NULL) // OK.  Take whatever the similar size.
		{
			ptr=NULL;
			while(NULL!=(ptr=airplaneTemplate.FindNext(ptr)))
			{
				diff=fabs(ptr->dat.GetProperty()->GetOutsideRadius()-dimension);
				if(match==NULL || diff<matchDiff)
				{
					match=ptr;
					matchDiff=diff;
				}
			}
		}



		if(match!=NULL)
		{
			FsAirplane neo,*air;

			const char *const *tmpl;


			switch(airClass)
			{
			case FSCL_AIRPLANE:
				if(isJet==YSTRUE)
				{
					tmpl=FsGenericJet;
				}
				else
				{
					tmpl=FsGenericProp;
				}
				break;
			case FSCL_HELICOPTER:
				tmpl=FsGenericChopper;
				break;
			default:
				tmpl=FsGenericJet;
				break;
			}

			std::unique_ptr <FsVisualSrf> collPtr(new FsVisualSrf);

			YsTextMemoryInputStream inStream(tmpl);
			YsShellExtReader reader;
			reader.MergeSrf(*collPtr,inStream);

			YsVec3 pos;
			for(auto vtHd : collPtr->AllVertex())
			{
				collPtr->GetVertexPosition(pos,vtHd);
				pos*=dimension;
				collPtr->SetVertexPosition(vtHd,pos);
			}

			neo.SetCollisionShell(*collPtr);

			neo.SetProperty(*match->dat.GetProperty(),match->dat.GetTemplateRootDirectory());

			neo.vis=NULL; // match->dat.GetVisual();
			neo.lod=NULL; // match->dat.GetLod();

			neo.cockpit=NULL;

			neo.isNetSubstitute=YSTRUE;

			air=sim->AddAirplane(neo,isPlayerPlane,match->dat.GetTemplateRootDirectory(),netSearchKey);

			return air;
		}
	}
	return NULL;
}

int FsWorld::GetAirplaneIdFromHandle(FsAirplane *air) const
{
	if(NULL!=sim)
	{
		return sim->GetAirplaneIdFromHandle(air);
	}
	return -1;
}

YSRESULT FsWorld::ReplaceAirplane(FsAirplane *air,const char idName[])
{
	if(NULL!=sim)
	{
		YsListItem <FsAirplaneTemplate> *ptr;

		ptr=FindAirplaneTemplate(idName);
		if(ptr!=NULL)
		{
			// When adding an airplane, GetCollision must be called before GetProperty,
			// otherwise, chTailStrikePitchAngle and chGroundStatickPitchAngle is not set.
			if(ptr->dat.GetCollision()!=NULL)
			{
				air->SetCollisionShell(*ptr->dat.GetCollision());
			}

			air->SetProperty(*ptr->dat.GetProperty(),ptr->dat.GetTemplateRootDirectory());
			air->vis=ptr->dat.GetVisual();
			air->lod=ptr->dat.GetLod();

			air->cockpit=ptr->dat.GetCockpit();
			SettleAirplane(*air,air->_startPosition);
			air->RecallCommand();
			return YSOK;
		}
	}
	return YSERR;
}

FsGround *FsWorld::AddGround(const char idName[],YSBOOL isPlayerGround,unsigned netSearchKey)
{
	if(NULL!=sim)
	{
		FsGround neo;
		YsListItem <FsGroundTemplate> *ptr;

		ptr=FindGroundTemplate(idName);
		if(ptr!=NULL)
		{
			PrepareGroundVisual(ptr);
			neo.SetProperty(ptr->dat.prop);

			neo.vis=ptr->dat.vis;
			neo.lod=ptr->dat.lod;
			if(ptr->dat.coll!=NULL)
			{
				neo.SetCollisionShell(*ptr->dat.coll);
			}
			if(nullptr!=ptr->dat.cockpit)
			{
				neo.cockpit=ptr->dat.cockpit;
			}

			FsGround *added;

			added=sim->AddGround(neo,ptr->dat.isAircraftCarrier,ptr->dat.GetTemplateRootDirectory(),ptr->dat.GetAircraftCarrierFileName(),netSearchKey);

			if(added!=NULL)
			{
				if(YSTRUE==isPlayerGround)
				{
					sim->SetPlayerGround(added);
				}

				// 2007/01/15 ACP will be read inside FsSimulation::AddGround
				//            ACP file used to be read here.

				return added;
			}
		}
	}

	return NULL;
}

FsGround *FsWorld::ResetGround(FsGround *gnd)
{
	if(NULL!=sim && gnd!=NULL)
	{
		FsGround neo;
		YsListItem <FsGroundTemplate> *ptr;

		ptr=FindGroundTemplate(gnd->Prop().GetIdentifier());
		if(ptr!=NULL)
		{
			if(YSTRUE==gnd->Prop().IsOnCarrier())
			{
				FsGround *onThisCarrier=gnd->Prop().OnThisCarrier();
				if(NULL!=onThisCarrier)
				{
					FsAircraftCarrierProperty *carrierProp=onThisCarrier->Prop().GetAircraftCarrierProperty();
					if(NULL!=carrierProp)
					{
						carrierProp->UnloadGround(gnd);
					}
				}
			}

			gnd->SetCollisionShell(*ptr->dat.coll);

			gnd->SetProperty(ptr->dat.prop);
			gnd->vis=ptr->dat.vis;
			gnd->lod=ptr->dat.lod;

			gnd->cockpit=ptr->dat.cockpit;

			return gnd;
		}
	}
	return NULL;
}

YSRESULT FsWorld::SetPlayerGround(FsGround *gnd,YSBOOL record)
{
	if(NULL!=sim)
	{
		sim->SetPlayerGround(gnd,record);
		return YSOK;
	}
	return YSERR;
}

void FsWorld::ReviveGround(FsGround *gnd)
{
	if(NULL!=sim)
	{
		YsListItem <FsGroundTemplate> *ptr;

		ptr=FindGroundTemplate(gnd->Prop().GetIdentifier());
		if(ptr!=NULL)
		{
			YsVec3 pos;
			YsAtt3 att;

			pos=gnd->GetPosition();
			att=gnd->GetAttitude();

			gnd->Prop().CopyState(ptr->dat.prop);
			gnd->netAlive=YSTRUE;

			gnd->Prop().SetPosition(pos);
			gnd->Prop().SetAttitude(att);
		}
	}
}

FsField *FsWorld::AddField(
	FsAddedFieldInfo *addedFieldInfo,  // NULL -> Nothing will be returned
    const char idName[],const YsVec3 &pos,const YsAtt3 &att,YSBOOL loadYFS,YSBOOL loadAir,unsigned iffControl)
{
	if(NULL!=sim)
	{
		FsField neo;
		YsListItem <FsFieldTemplate> *ptr;
		int nAirCheck1,nAirCheck2;
		// int nFldCheck1,nFldCheck2;  Field check is no longer valid because only one field can be loaded.

		ptr=FindFieldTemplate(idName);
		if(ptr!=NULL)
		{
			if(NULL!=addedFieldInfo)
			{
				addedFieldInfo->Initialize();
			}

			int i;
			YsScenery *fld;
			YsMatrix4x4 rootMat,objMat;
			YsArray <const YsSceneryPointSet *,16> motionPathList;

			if(YSOK!=PrepareFieldVisual(ptr))
			{
				return NULL;
			}
			neo.SetField(ptr->dat.GetField());

			if(loadYFS==YSTRUE)
			{
				if(ptr->dat.GetField()!=NULL)
				{
					YsArray <YsSceneryGndObj *> gobList;

					fld=ptr->dat.GetField();
					// 2005/01/10 fld->ResetPosition();  // 2004/08/27
					fld->ResetPosition();  // 2005/01/10
					fld->MakeListOfGndObj(gobList);

					rootMat.Translate(pos);
					rootMat.RotateXZ(att.h());
					rootMat.RotateZY(att.p());
					rootMat.RotateXY(att.b());


					for(i=0; i<gobList.GetN(); i++)
					{
						if((iffControl&(1<<gobList[i]->GetIFF()))==0)  // 2005/03/08 Add object with only specific IFF
						{
							continue;
						}

						fld->GetTransformation(objMat,gobList[i]);
						YsVec3 ev,uv,p;
						ev=YsZVec();
						uv=YsYVec();
						p=YsOrigin();

						objMat.Mul(ev,ev,0.0);
						objMat.Mul(uv,uv,0.0);
						objMat.Mul(p,p,1.0);

						rootMat.Mul(ev,ev,0.0);
						rootMat.Mul(uv,uv,0.0);
						rootMat.Mul(p,p,1.0);

						FsGround *const gnd=AddGround(gobList[i]->GetObjName(),YSFALSE);
						if(NULL!=addedFieldInfo)
						{
							addedFieldInfo->gndArray.Append(gnd);  // If it is NULL, store NULL so that gndId stays consistent.
						}
						if(gnd!=NULL)
						{
							YsAtt3 att;
							att.SetTwoVector(ev,uv);


							gnd->name.Set(gobList[i]->GetTag());
							gnd->ysfId=gobList[i]->GetId();


							SettleGround(*gnd,p);
							SettleGround(*gnd,att);
							gnd->iff=(FSIFF)gobList[i]->GetIFF();
							gnd->gndFlag=gobList[i]->GetFlag();
							gnd->primaryTarget=gobList[i]->IsPrimaryTarget();

							if(gnd->gndFlag&FSGNDFLAG_DONTMOVE)
							{
								gnd->SendCommand("MAXSPEED 0kt");
								gnd->SendCommand("MAXROTAT 0deg");
							}

							YsString motionPathName=gobList[i]->GetMotionPathName();
							YSBOOL motionPathOffset=gobList[i]->GetMotionPathOffset();
							if(motionPathName.Strlen()>0)
							{
								fld->SearchPointSetByTag(motionPathList,motionPathName);
								if(motionPathList.GetN()>0)
								{
									gnd->SetMotionPath(motionPathList[0]);
									gnd->useMotionPathOffset=motionPathOffset;
								}
							}
						}
					}
				}

				// This section is left for the compatibility >>
				nAirCheck1=sim->GetNumAirplane();
				// nFldCheck1=sim->GetNumFieldLoaded();  Field check is no longer valid.  Only one field can be loaded.
				LoadInternal(ptr->dat.GetYfsFileName(),pos,att);
				nAirCheck2=sim->GetNumAirplane();
				// nFldCheck2=sim->GetNumFieldLoaded();

				// if(nFldCheck2!=nFldCheck1)
				// {
				// 	fsStderr.Printf
				// 	  ("One or more field is added in the"
				// 	   "static object definition file.\n");
				// }
				if(nAirCheck1!=nAirCheck2)
				{
					fsStderr.Printf
					  ("One or more airplane is added in the"
					   "static object definition file.\n");
				}
				// << This section is left for the compatibility
			}
			else
			{
				// printf("loadYFS=FALSE  Don't load YFS for thie field %s.\n",idName);
			}

			if(loadAir==YSTRUE && ptr->dat.GetField()!=NULL)
			{
				YsArray <YsSceneryAir *> airList;

				fld=ptr->dat.GetField();
				fld->ResetPosition();
				fld->MakeListOfAir(airList);

				rootMat.Translate(pos);
				rootMat.RotateXZ(att.h());
				rootMat.RotateZY(att.p());
				rootMat.RotateXY(att.b());

				for(i=0; i<airList.GetN(); i++)
				{
					if((iffControl&(1<<airList[i]->GetIFF()))==0)  // Add object with only specific IFF
					{
						continue;
					}

					fld->GetTransformation(objMat,airList[i]);
					YsVec3 ev,uv,p;
					ev=YsZVec();
					uv=YsYVec();
					p=YsOrigin();

					objMat.Mul(ev,ev,0.0);
					objMat.Mul(uv,uv,0.0);
					objMat.Mul(p,p,1.0);

					rootMat.Mul(ev,ev,0.0);
					rootMat.Mul(uv,uv,0.0);
					rootMat.Mul(p,p,1.0);

					FsAirplane *air;
					air=AddAirplane(airList[i]->GetObjName(),YSFALSE);
					if(air!=NULL)
					{
						YsAtt3 att;
						char cmd[256];

						att.SetTwoVector(ev,uv);

						air->name.Set(airList[i]->GetTag());
						air->ysfId=airList[i]->GetId();
						air->airFlag=airList[i]->GetFlag();
						air->landWhenLowFuelThr=airList[i]->GetLandWhenLowFuel();

						sprintf(cmd,"POSITION %.2lfm %.2lfm %.2lfm\n",p.x(),p.y(),p.z());
						air->SendCommand(cmd);
						sprintf(cmd,"ATTITUDE %.2lfdeg %.2lfdeg %.2lfdeg",
						    YsRadToDeg(att.h()),YsRadToDeg(att.p()),YsRadToDeg(att.b()));
						air->SendCommand(cmd);

						air->iff=(FSIFF)airList[i]->GetIFF();
						air->SendCommand(airList[i]->GetLandingGear()==YSTRUE ? "CTLLDGEA TRUE" : "CTLLDGEA FALSE");


						sprintf(cmd,"INITSPED %lfm/s",airList[i]->GetSpeed());
						air->SendCommand(cmd);

						sprintf(cmd,"INITFUEL %d%%",airList[i]->GetFuel());
						air->SendCommand(cmd);


						switch(airList[i]->GetAction().actType)
						{
						default:
							break;
						case YsSceneryAirAction::DONOTHING:
							break;
						case YsSceneryAirAction::LAND:
							{
								FsLandingAutopilot *ld=FsLandingAutopilot::Create();
								air->SetAutopilot(ld);
							}
							break;
						case YsSceneryAirAction::CIRCLE:
							{
								FsGotoPosition *gp=FsGotoPosition::Create();
								gp->speed=airList[i]->GetSpeed();
								gp->destination.Set(1,&p);
								air->SetAutopilot(gp);
							}
							break;
						case YsSceneryAirAction::STRAIGHT:
							{
								FsGotoPosition *gp=FsGotoPosition::Create();
								gp->destination.Set(1,&p);
								gp->speed=airList[i]->GetSpeed();
								gp->straightFlightMode=YSTRUE;
								air->SetAutopilot(gp);
							}
							break;
						case YsSceneryAirAction::FLYAIRROUTE:
							{
								FsAirRouteAutopilot *ar=FsAirRouteAutopilot::Create();
								ar->SetAirRouteTag(airList[i]->GetAction().actParam[0]);
								air->SetAutopilot(ar);
							}
							break;
						}
					}
				}
			}


			// Don't care neo.coll
			neo.SetIdName(idName);

			FsField *fsFld;
			fsFld=sim->SetField(neo,pos,att);
			return fsFld;
		}
	}
	return NULL;
}

void FsWorld::SetEmptyField(void)
{
	if(NULL!=sim)
	{
		this->emptyFieldTemplate.MakeEmptyTemplate();
		FsField empty;
		empty.SetField(this->emptyFieldTemplate.GetField());
		sim->SetField(empty,YsVec3::Origin(),YsZeroAtt());
	}
}

YSRESULT FsWorld::GetFieldVisual(YsScenery &scn,const char fldName[])
{
	YsListItem <FsFieldTemplate> *ptr;

	ptr=FindFieldTemplate(fldName);
	if(ptr!=NULL)
	{
		return YsLoadFld(scn,ptr->dat.GetVisualFileName());
	}
	return YSERR;
}

YSRESULT FsWorld::PrepareFieldVisual(YsListItem <FsFieldTemplate> *templ) const
{
	if(templ->dat.GetField()!=NULL)
	{
		return YSOK;
	}

	if(YSOK!=templ->dat.LoadField())
	{
		lastErrorMessage=L"Load Error (FLD) :";
		lastErrorMessage.Append(templ->dat.GetVisualFileName());
		switch(templ->dat.GetLastFieldError())
		{
		case YsScenery::ERROR_IO_FILE_NOT_FOUND:
			lastError=ERROR_IO_FILE_NOT_FOUND;
			lastErrorMessage.Append(L"\n");
			lastErrorMessage.Append(L"File Not Found.");
			break;
		case YsScenery::ERROR_IO_NEED_NEW_VERSION:
			lastError=ERROR_FLD_VERSION;
			lastErrorMessage.Append(L"\n");
			lastErrorMessage.Append(L"Newer Version Required.");
			break;
		case YsScenery::ERROR_IO_INVALID_FILE:
			lastError=ERROR_FLD_READERROR;
			lastErrorMessage.Append(L"\n");
			lastErrorMessage.Append(L"File Corrupted.");
			break;
		default:
			lastError=ERROR_FLD_READERROR;
			lastErrorMessage.Append(L"\n");
			lastErrorMessage.Append(L"General Read Error.");
			break;
		}
		return YSERR;
	}
	return YSOK;
}

YSRESULT FsWorld::MakeCenterJoystickDialog(class FsCenterJoystick &centerJoystickDialog,int nextActionCode)
{
	if(NULL!=sim)
	{
		return sim->MakeCenterJoystickDialog(centerJoystickDialog,nextActionCode);
	}
	return YSERR;
}

YSRESULT FsWorld::CheckJoystickAssignmentAndFixIfNecessary(void)
{
	if(NULL!=sim)
	{
		return sim->CheckJoystickAssignmentAndFixIfNecessary();
	}
	return YSERR;
}

void FsWorld::CenterJoystick(void)
{
	if(NULL!=sim)
	{
		sim->CenterJoystick();
	}
}

YSRESULT FsWorld::RunSimulationOneStep(FsSimulation::FSSIMULATIONSTATE &state)
{
	if(NULL!=sim)
	{
		sim->RunSimulationOneStep(state);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::SimulateOneStep(
    const double &dt,
    YSBOOL demoMode,YSBOOL record,YSBOOL showTimer,YSBOOL networkStandby,FSUSERCONTROL userControl,
    YSBOOL showTimeMarker)
{
	if(NULL!=sim)
	{
		sim->SimulateOneStep(dt,demoMode,record,showTimer,networkStandby,userControl,showTimeMarker);
		return YSOK;
	}
	return YSERR;
}

void FsWorld::AssignUniqueYsfId(void)
{
	if(NULL!=sim)
	{
		sim->AssignUniqueYsfId();
	}
}


void FsWorld::RunServerModeOneStep(class FsServerRunLoop &svrSta)
{
	sim->RunServerModeOneStep(svrSta);
}

void FsWorld::RunClientModeOneStep(class FsClientRunLoop &cliSta)
{
	sim->RunClientModeOneStep(cliSta);
}

YSBOOL FsWorld::CheckInterceptMissionAvailable(void) const
{
	if(NULL!=sim)
	{
		return sim->CheckInterceptMissionAvailable();
	}
	return YSFALSE;
}

YSBOOL FsWorld::CheckCloseAirSupportMissionAvailable(void) const
{
	if(NULL!=sim)
	{
		return sim->CheckCloseAirSupportMissionAvailable();
	}
	return YSFALSE;
}

YSRESULT FsWorld::RunReplayOneStep(FsSimulation::FSSIMULATIONSTATE &state,FsSimulation::ReplayInfo &replayInfo)
{
	if(NULL!=sim)
	{
		sim->RunReplaySimulationOneStep(state,replayInfo);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::PrepareRunDemoMode(FsDemoModeInfo &info,const char sysMsg[],const double &maxTime)
{
	if(NULL!=sim)
	{
		return sim->PrepareRunDemoMode(info,sysMsg,maxTime);
	}
	return YSERR;
}

static YSBOOL IsBlueImpulse(const char airName[])
{
	int i;
	YsString str;
	YSBOOL blue,impulse;

	str.Set(airName);

	str.Capitalize();
	blue=YSFALSE;
	impulse=YSFALSE;
	for(i=0; i<str.Strlen(); i++)
	{
		if(strncmp((const char *)str+i,"BLUE",4)==0)
		{
			blue=YSTRUE;
		}
		if(strncmp((const char *)str+i,"IMPULSE",7)==0)
		{
			impulse=YSTRUE;
		}

		if(blue==YSTRUE && impulse==YSTRUE)
		{
			return YSTRUE;
		}
	}
	return YSFALSE;
}

YSRESULT FsWorld::PrepareAcrobat(FsDemoModeInfo &info,const char airType[],int acroType)
{
	// Formation Leader must always be the first airplane.

	info.Initialize(FSDEMO_ACROBAT);

	if(NULL!=sim)
	{
		YsVec3 startPos,startVel;
		YsAtt3 startAtt;
		YsVec3 showCenter;
		FsGround *refGnd;
		YsArray <FsGround *> ilsCandidate,ilsViewList;
		YsArray <YsVec3> twrCandidate;
		YSBOOL isBlueImpulse;

		showCenter=YsOrigin();
		refGnd=NULL;

		isBlueImpulse=IsBlueImpulse(airType);

		if(sim->GetNumILSFacility()>0 && sim->GetNumTowerView()>0)
		{
			int i,j;
			for(i=0; i<sim->GetNumILSFacility(); i++)
			{
				YsVec3 twr;
				FsGround *ils;
				ils=sim->GetILS(i);
				if(ils->Prop().GetAircraftCarrierProperty()->CanBeViewpoint()==YSTRUE)
				{
					ilsViewList.Append(ils);
					for(j=0; j<sim->GetNumTowerView(); j++)
					{
						twr=sim->GetTowerView(j);
						if((twr-ils->GetPosition()).GetSquareLength()<4000.0*4000.0)
						{
							ilsCandidate.Append(ils);
							twrCandidate.Append(twr);
							break;
						}
					}
				}
			}
		}



		switch(acroType)  // Decide startPos,startAtt,showCenter, and refGnd
		{
		default:
			if(ilsCandidate.GetN()>0)
			{
				int n;
				n=rand()%ilsCandidate.GetN();
				startPos=ilsCandidate[n]->GetPosition();
				startAtt=ilsCandidate[n]->GetAttitude();
				startAtt.SetH(startAtt.h()+YsPi);

				refGnd=ilsCandidate[n];
				showCenter=twrCandidate[n];
			}
			else
			{
				if(ilsViewList.GetN()>0)
				{
					int n;
					n=rand()%ilsViewList.GetN();
					startPos=ilsViewList[n]->GetPosition();
					startAtt=ilsViewList[n]->GetAttitude();
					startAtt.SetH(startAtt.h()+YsPi);

					refGnd=ilsViewList[n];
				}
				else if(sim->GetNumTowerView()>0)
				{
					int n;
					n=rand()%sim->GetNumTowerView();
					startPos=sim->GetTowerView(n)-YsZVec()*1500.0;
					startAtt=YsZeroAtt();

					refGnd=NULL;
				}
				else
				{
					startPos=YsOrigin();
					startAtt=YsZeroAtt();
				}

				showCenter.Set((rand()%100<50 ? 10.0 : -10.0),10,1500.0);
				startAtt.Mul(showCenter,showCenter);
				showCenter+=startPos;
				showCenter.SetY(10.0);
			}

			// Adjust start point so that it comes in straight to the show center
			if(acroType==FSACRO_CORKSCREW ||
			   acroType==FSACRO_DELTAROLL ||
			   acroType==FSACRO_TACKCROSSANDVERTICALCLIMBROLL ||
			   acroType==FSACRO_BIGHEART ||
			   acroType==FSACRO_LETTEREIGHT ||
			   acroType==FSACRO_LEVELOPENER ||
			   acroType==FSACRO_LANCASTERTO5_4SPLIT ||
			   acroType==FSACRO_CHAMPAIGNSPLIT ||
			   acroType==FSACRO_VIXENBREAK)
			{
				YsVec3 rwLineOrg,rwLineDir,rwNearPos,ext;
				rwLineOrg=startPos;
				rwLineOrg.SetY(showCenter.y());
				rwLineDir=startAtt.GetForwardVector();
				if(YsGetNearestPointOnLine3(rwNearPos,rwLineOrg,rwLineOrg+rwLineDir,showCenter)==YSOK)
				{
					ext=rwNearPos-showCenter;
					if(ext.Normalize()==YSOK)
					{
						double dist;
						dist=(showCenter-rwLineOrg).GetLength();
						startPos=showCenter+dist*ext;
						startAtt.SetForwardVector(-ext);
					}
				}
			}

			break;
		case FSACRO_DIAMONDTAKEOFF:
		case FSACRO_ROLLONTAKEOFFANDHALFCUBAN:
			{
				int i,j;
				char idName[256];
				YsString fldNameBuf;
				const char *fldName;
				YsVec3 fldPos;
				YsAtt3 fldAtt;
				YsArray <const YsSceneryRectRegion *> visitedRgn;
				YsArray <YsVec3,16> candidate;
				const YsSceneryRectRegion *rgn;
				YsVec3 rect[4];

				if(GetLoadedField(fldNameBuf,fldPos,fldAtt)==YSOK)
				{
					fldName=fldNameBuf;
					for(i=0; GetFieldStartPositionName(idName,fldName,i)==YSOK; i++)
					{
						FsStartPosInfo info;
						if(idName[0]=='R' && idName[1]=='W' && GetStartPositionInfo(info,idName)==YSOK)
						{
							YsVec3 pos;
							YsString str;
							YsArray <YsString,16> args;
							for(j=0; j<info.cmd.GetN(); j++)
							{
								str.Set(info.cmd[j]);
								if(str.Arguments(args)==YSOK && args.GetN()>=4 && strcmp(args[0],"POSITION")==0)
								{
									pos.Set(atof(args[1]),atof(args[2]),atof(args[3]));
									break;
								}
							}
							if(sim->GetRunwayRectFromPosition(rgn,rect,pos)==YSOK)
							{
								if(YsIsIncluded <const YsSceneryRectRegion *> (visitedRgn.GetN(),visitedRgn,rgn)!=YSTRUE)
								{
									visitedRgn.Append(rgn);
									rect[0].SetY(pos.y());
									rect[1].SetY(pos.y());
									rect[2].SetY(pos.y());
									rect[3].SetY(pos.y());
									candidate.Append(4,rect);
								}
							}
						}
					}
				}

				if(candidate.GetN()/4>0)
				{
					int n;
					n=(int)candidate.GetN()/4;
					i=rand()%n;

					rect[0]=candidate[i*4];
					rect[1]=candidate[i*4+1];
					rect[2]=candidate[i*4+2];
					rect[3]=candidate[i*4+3];
					if((rect[1]-rect[0]).GetSquareLength()>(rect[2]-rect[1]).GetSquareLength())
					{
						YsVec3 a;
						a=rect[3];
						rect[3]=rect[2];
						rect[2]=rect[1];
						rect[1]=rect[0];
						rect[0]=a;
					}
					YsVec3 rwDir,mid,side;
					rwDir=rect[2]-rect[1];
					rwDir.Normalize();
					mid=(rect[0]+rect[1])/2.0;

					startAtt.SetForwardVector(rwDir);
					startAtt.SetB(0.0);
					if(acroType==FSACRO_DIAMONDTAKEOFF)
					{
						side.Set(6.0,0.0,0.0);
					}
					else // if(acroType==FSACRO_ROLLONTAKEOFFANDHALFCUBAN)
					{
						side.Set(11.0,0.0,0.0);
					}
					startAtt.Mul(side,side);
					startPos=mid+side+rwDir*200.0;
					startVel=YsOrigin();

					showCenter.Set((rand()%100<50 ? 10.0 : -10.0),10,1500.0);
					startAtt.Mul(showCenter,showCenter);
					showCenter+=startPos;
					showCenter.SetY(10.0);
				}
				else
				{
					return YSERR;
				}
			}
			break;
		}

		info.acroType=acroType;
		info.showCenter=showCenter;
		info.refObj=refGnd;


		int i,j;
		double speed;
		const int maxNumAir=10;
		FsAirplane *air[maxNumAir];
		double r;

		for(i=0; i<maxNumAir; i++)
		{
			air[i]=NULL;
		}

		air[0]=AddAirplane(airType,YSTRUE);
		if(air[0]==NULL)
		{
			return YSERR;
		}
		air[0]->Prop().UnloadAllWeapon();
		air[0]->Prop().SendCommand("CTLLDGEA FALSE");
		speed=(air[0]->Prop().IsJet()==YSTRUE ? 140.0 : 70);
		r=air[0]->GetRadiusFromCollision()*0.85;

		startVel.Set(0.0,0.0,speed);
		startAtt.Mul(startVel,startVel);


		FsAirshowControl *ap;
		YsVec3 newpos;
		switch(acroType)
		{
		case FSACRO_CORKSCREW:
			{
				startPos.SetY(280.0);
				startPos-=startAtt.GetForwardVector()*speed*25;

				double corkScrewRadius=50.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				air[1]=AddAirplane(airType,YSFALSE);
				air[1]->Prop().UnloadAllWeapon();
				air[1]->Prop().SendCommand("CTLLDGEA FALSE");

				newpos.Set(-corkScrewRadius,0.0,-20.0);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::CORKSCREW;
				ap->cswMode=10;
				ap->cswRadius=corkScrewRadius;
				ap->leader=air[0];
				ap->shouldBe=newpos;
				ap->fomPosition=6;

				startAtt.Mul(newpos,newpos);
				newpos+=air[0]->GetPosition();
				air[1]->Prop().SetPosition(newpos);
				air[1]->Prop().SetAttitude(startAtt);
				air[1]->Prop().SetVelocity(startVel);
				air[1]->SetAutopilot(ap);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::CORKSCREW;
				ap->fomPosition=5;
				ap->cswMode=10;
				ap->formation[5]=air[1];
				ap->waitTimer=2.0;
				air[0]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];
			info.solo[1]=air[1];

			break;

		case FSACRO_SLOWROLL:
			{
				startPos.SetY(150.0);
				startPos-=startAtt.GetForwardVector()*speed*20;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->action=FsAirshowControl::BARRELROLL;
				air[0]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];

			break;

		case FSACRO_DELTALOOP:
			{
				startPos.SetY(170.0);
				startPos-=startAtt.GetForwardVector()*speed*5.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::LOOP;
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				air[0]->SetAutopilot(ap);

				for(i=2; i<=6; i++)
				{
					FsFormation *fom;
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetDeltaPosition(newpos,i,r);
					fom=FsFormation::Create();
					fom->leader=air[0];
					fom->synchronizeTrigger=YSTRUE;
					fom->shouldBe=newpos;
					air[i-1]->SetAutopilot(fom);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;

		case FSACRO_DELTAROLL:
			{
				startPos.SetY(170.0);
				startPos-=startAtt.GetForwardVector()*speed*20.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::BARRELROLL;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				air[0]->SetAutopilot(ap);

				for(i=2; i<=6; i++)
				{
					FsFormation *fom;
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetDeltaPosition(newpos,i,r);
					fom=FsFormation::Create();
					fom->leader=air[0];
					fom->synchronizeTrigger=YSTRUE;
					fom->shouldBe=newpos;
					air[i-1]->SetAutopilot(fom);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;

		case FSACRO_TIGHTTURN:
			{
				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::TURN360;
				ap->turnBankAngle=YsPi*80.0/180.0;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->turnAndLoop=YSFALSE;
				air[0]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];

			break;

		case FSACRO_360ANDLOOP:
			{
				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::TURN360;
				ap->turnBankAngle=YsPi*70.0/180.0;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->turnAndLoop=YSTRUE;
				air[0]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];

			break;

		case FSACRO_BOMBBURST4SHIP:
			{
				startPos.SetY(240.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=5; i++)
				{
					if(i<5)
					{
						FsAirshowControl::GetDeltaPosition(newpos,i,r);
					}
					else
					{
						newpos.Set(0.0,0.0,-1000.0);
					}

					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::BOMBBURST4SHIP;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::BOMBBURST4SHIP;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];

			info.solo[0]=air[4];

			break;
		case FSACRO_CHANGEOVERTURN:
			{
				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=1; i<6; i++)
				{
					air[i]=AddAirplane(airType,YSFALSE);
					air[i]->Prop().UnloadAllWeapon();

					YsVec3 p;
					ap=FsAirshowControl::Create();
					p.Set(0.0,-3.5*(double)i,-r*1.6*(double)i);
					ap->action=FsAirshowControl::CHANGEOVERTURN;
					ap->fomPosition=i+1;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=p;

					startAtt.Mul(p,p);
					p+=startPos;
					air[i]->Prop().SetPosition(p);
					air[i]->Prop().SetAttitude(startAtt);
					air[i]->Prop().SetVelocity(startVel);
					air[i]->Prop().SendCommand("CTLLDGEA FALSE");

					air[i]->SetAutopilot(ap);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::CHANGEOVERTURN;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->fomPosition=1;
				ap->changeOverTurnBankAngle=YsPi/3.0;
				ap->waitTimer=3.0;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;
		case FSACRO_TRAILTODIAMONDROLL:
			{
				startPos.SetY(140.0);
				startPos-=startAtt.GetForwardVector()*speed*5.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=1; i<4; i++)
				{
					FsFormation *fom;

					air[i]=AddAirplane(airType,YSFALSE);
					air[i]->Prop().UnloadAllWeapon();

					YsVec3 p;
					fom=FsFormation::Create();
					p.Set(0.0,-3.5*(double)i,-r*1.6*(double)i);
					fom->leader=air[0];
					fom->synchronizeTrigger=YSTRUE;
					fom->shouldBe=p;

					startAtt.Mul(p,p);
					p+=startPos;
					air[i]->Prop().SetPosition(p);
					air[i]->Prop().SetAttitude(startAtt);
					air[i]->Prop().SetVelocity(startVel);
					air[i]->Prop().SendCommand("CTLLDGEA FALSE");

					air[i]->SetAutopilot(fom);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::TRAILTODIAMONDROLL;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->waitTimer=2.0;
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];

				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];

			break;
		case FSACRO_CUBANEIGHT:
			{
				startPos.SetY(140.0);
				startPos+=startAtt.GetForwardVector()*speed*5.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::CUBANEIGHT;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				air[0]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];

			break;

		case FSACRO_DELTALOOPANDBONTON:
			{
				YsVec3 newpos;

				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);


				for(i=2; i<=6; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetDeltaPosition(newpos,i,r);
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::DELTALOOPANDBONTON;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::DELTALOOPANDBONTON;
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;
		case FSACRO_BONTONROLL:
			{
				YsVec3 newpos;

				startPos.SetY(120.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=4; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetDeltaPosition(newpos,i,r);
					newpos*=3.0;
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::BONTONROLL;
					ap->turnDir=(isBlueImpulse==YSTRUE ? -1.0 : 1.0);
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::BONTONROLL;
				ap->waitTimer=4.0;
				ap->turnDir=(isBlueImpulse==YSTRUE ? -1.0 : 1.0);
				ap->loopG=4.0;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];

			break;
		case FSACRO_BOMBBURSTDOWN4SHIP:
		case FSACRO_BOMBBURSTDOWN6SHIP:
			{
				int n,action;

				n=(acroType==FSACRO_BOMBBURSTDOWN4SHIP ? 4 : 6);
				action=(acroType==FSACRO_BOMBBURSTDOWN4SHIP ? 
				        FsAirshowControl::BOMBBURSTDOWN4SHIP : FsAirshowControl::BOMBBURSTDOWN6SHIP);

				startPos.SetY(330.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=n; i++)
				{
					FsAirshowControl::GetDeltaPosition(newpos,i,r);

					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=action;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=action;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;
		case FSACRO_RAINFALL:
			{
				startPos.SetY(330.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=6; i++)
				{
					if(i==4)
					{
						continue;
					}
					FsAirshowControl::GetDeltaPosition(newpos,i,r);

					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::RAINFALL;;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::RAINFALL;;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;
		case FSACRO_ROLLINGCOMBATPITCH:
			{
				int n;
				n=4;

				YsVec3 rel;
				for(i=1; i<n; i++)
				{
					air[i]=AddAirplane(airType,(i==0 ? YSTRUE : YSFALSE));
				}

				startPos.SetY(330.0);
				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=0; i<n; i++)
				{
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::ROLLINGCOMBATPITCH;
					ap->fomPosition=i+1;
					ap->leader=air[0];
					rel.Set(-r*(double)i,-0.2*(double)i,-r*(double)i);
					ap->shouldBe=rel;
					for(j=0; j<n; j++)
					{
						ap->formation[j]=air[j];
					}
					air[i]->SetAutopilot(ap);

					startAtt.Mul(rel,rel);
					rel+=startPos;
					air[i]->Prop().SetPosition(rel);
					air[i]->Prop().SetAttitude(startAtt);
					air[i]->Prop().SetVelocity(startVel);
					air[i]->Prop().UnloadAllWeapon();
					air[i]->Prop().SendCommand("CTLLDGEA FALSE");
				}
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];

			break;
		case FSACRO_DIAMONDTAKEOFF:
			{
				int n;
				n=4;

				YsVec3 rel;
				for(i=1; i<n; i++)
				{
					air[i]=AddAirplane(airType,(i==0 ? YSTRUE : YSFALSE));
				}

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(YsOrigin());
				air[0]->Prop().SendCommand("CTLLDGEA TRUE");

				for(i=0; i<n; i++)
				{
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::DIAMONDTAKEOFF;
					ap->fomPosition=i+1;
					ap->leader=air[0];
					switch(i)
					{
					case 0:
						rel=YsOrigin();
						break;
					case 1:
						rel.Set(-1.0,-0.2,-1.0);
						break;
					case 2:
						rel.Set( 1.0,-0.2,-1.0);
						break;
					case 3:
						rel.Set(-2.0,-0.4,-2.0);
						break;
					}
					rel*=r;
					ap->shouldBe=rel;
					for(j=0; j<n; j++)
					{
						ap->formation[j]=air[j];
					}

					if(i==3)
					{
						YsVec3 path;
						ap->shouldBe.SubY(4.0);
						path=ap->shouldBe;
						path.DivX(3.0);
						ap->transition[ap->nTransition++]=path;
						path.SetX(0.0);
						ap->transition[ap->nTransition++]=path;
						path.AddY(4.0);
						ap->transition[ap->nTransition++]=path;
					}

					air[i]->SetAutopilot(ap);

					startAtt.Mul(rel,rel);
					rel+=startPos;
					rel.SetY(startPos.y());
					air[i]->Prop().SetPosition(rel);
					air[i]->Prop().SetAttitude(startAtt);
					air[i]->Prop().SetVelocity(YsOrigin());
					air[i]->Prop().UnloadAllWeapon();
					air[i]->Prop().SendCommand("CTLLDGEA TRUE");
				}
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;
		case FSACRO_CONTINUOUSROLL:
			{
				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::CONTINUOUSROLL;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->waitTimer=1.0;
				air[0]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];

			break;

		case FSACRO_ROLLONTAKEOFFANDHALFCUBAN:
			{
				air[1]=AddAirplane(airType,YSFALSE);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(YsOrigin());
				air[0]->Prop().UnloadAllWeapon();
				air[0]->Prop().SendCommand("CTLLDGEA TRUE");

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::ROLLONTAKEOFFANDHALFCUBAN;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->fomPosition=5;
				air[0]->SetAutopilot(ap);

				newpos.Set(r*-2.2,0.0,200.0);
				startAtt.Mul(newpos,newpos);
				newpos+=startPos;
				newpos.SetY(startPos.y());
				air[1]->Prop().SetPosition(newpos);
				air[1]->Prop().SetAttitude(startAtt);
				air[1]->Prop().SetVelocity(YsOrigin());
				air[1]->Prop().UnloadAllWeapon();
				air[1]->Prop().SendCommand("CTLLDGEA TRUE");

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::ROLLONTAKEOFFANDHALFCUBAN;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->fomPosition=6;
				air[1]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];
			info.solo[1]=air[1];

			break;

		case FSACRO_TACKCROSSANDVERTICALCLIMBROLL:
			{
				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);
				air[0]->Prop().SendCommand("CTLLDGEA FALSE");
				air[0]->Prop().UnloadAllWeapon();

				air[1]=AddAirplane(airType,YSFALSE);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::TACKCROSSANDVERTICALCLIMBROLL;
				ap->fomPosition=5;
				ap->waitTimer=1.0;
				ap->formation[4]=air[0];
				ap->formation[5]=air[1];
				air[0]->SetAutopilot(ap);


				newpos.Set(-2.5*r,0.0,-2.7*r);
				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::TACKCROSSANDVERTICALCLIMBROLL;
				ap->fomPosition=6;
				ap->leader=air[0];
				ap->shouldBe=newpos;
				ap->waitTimer=1.0;
				ap->formation[4]=air[0];
				ap->formation[5]=air[1];
				air[1]->SetAutopilot(ap);

				startAtt.Mul(newpos,newpos);
				newpos+=startPos;

				air[1]->Prop().SetPosition(newpos);
				air[1]->Prop().SetAttitude(startAtt);
				air[1]->Prop().SetVelocity(startVel);
				air[1]->Prop().SendCommand("CTLLDGEA FALSE");
				air[1]->Prop().UnloadAllWeapon();

				sim->SetPlayerAirplane(air[1]);
			}

			info.solo[0]=air[0];
			info.solo[1]=air[1];

			break;
		case FSACRO_BIGHEART:
			{
				startPos.SetY(300.0);
				startPos-=startAtt.GetForwardVector()*speed*10.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);
				air[0]->Prop().SendCommand("CTLLDGEA FALSE");
				air[0]->Prop().UnloadAllWeapon();

				air[1]=AddAirplane(airType,YSFALSE);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::BIGHEART;
				ap->fomPosition=5;
				ap->waitTimer=1.0;
				ap->formation[4]=air[0];
				ap->formation[5]=air[1];
				air[0]->SetAutopilot(ap);


				newpos.Set(-r,-1.5,-r);
				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::BIGHEART;
				ap->fomPosition=6;
				ap->leader=air[0];
				ap->shouldBe=newpos;
				ap->waitTimer=1.0;
				ap->formation[4]=air[0];
				ap->formation[5]=air[1];
				air[1]->SetAutopilot(ap);

				startAtt.Mul(newpos,newpos);
				newpos+=startPos;

				air[1]->Prop().SetPosition(newpos);
				air[1]->Prop().SetAttitude(startAtt);
				air[1]->Prop().SetVelocity(startVel);
				air[1]->Prop().SendCommand("CTLLDGEA FALSE");
				air[1]->Prop().UnloadAllWeapon();
			}

			info.solo[0]=air[0];
			info.solo[1]=air[1];

			break;

		case FSACRO_LEVELBREAK:
			{
				startPos.SetY(200.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=6; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::LEVELBREAK;
				ap->waitTimer=4.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);

				for(i=2; i<=6; i++)
				{
					FsAirshowControl::GetDeltaPosition(newpos,i,r);
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::LEVELBREAK;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;
					ap->fomPosition=i;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;

		case FSACRO_ROLLBACKTOARROWHEAD:
			{
				startPos.SetY(100.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=4; i++)
				{
					FsAirshowControl::GetDeltaPosition(newpos,i,r);

					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::ROLLBACKTOARROWHEAD;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::ROLLBACKTOARROWHEAD;
				ap->fomPosition=1;
				ap->waitTimer=1.0;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];

			break;

		case FSACRO_PITCHUPBREAK:
			{
				YsVec3 newpos;

				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=6; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::PITCHUPBREAK;
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);

				for(i=2; i<=6; i++)
				{
					FsAirshowControl::GetDeltaPosition(newpos,i,r);
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::PITCHUPBREAK;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;

					ap->formation[0]=air[0];
					ap->formation[1]=air[1];
					ap->formation[2]=air[2];
					ap->formation[3]=air[3];
					ap->formation[4]=air[4];
					ap->formation[5]=air[5];

					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				sim->SetPlayerAirplane(air[5]);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;

		case FSACRO_ROCKWINGCLEAN:
		case FSACRO_ROCKWINGDIRTY:
			{
				startPos.SetY(100.0);

				if(acroType==FSACRO_ROCKWINGDIRTY)
				{
					double v;
					v=air[0]->Prop().GetEstimatedLandingSpeed();
					startVel=1.5*v*startAtt.GetForwardVector();
					startAtt.SetP(air[0]->Prop().GetCriticalAOA()*0.5);

					if(air[0]->Prop().GetHasThrustVectoring()!=YSTRUE)
					{
						// Thrust vectoring is used for Concorde nose.
						// If the airplane does not have thrust the vectoring capability,
						// it can be Concorde, and thrust vectoring has no effect on
						// other airplanes.
						air[0]->Prop().SetThrustVector(1.0);
					}
				}
				else
				{
					if(air[0]->Prop().GetHasThrustVectoring()!=YSTRUE)
					{
						// Thrust vectoring is used for Concorde nose.
						// If the airplane does not have thrust the vectoring capability,
						// it can be Concorde, and thrust vectoring has no effect on
						// other airplanes.
						air[0]->Prop().SetThrustVector(0.4);
					}
				}

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::ROCKWING;
				ap->rwDirty=(acroType==FSACRO_ROCKWINGDIRTY ? YSTRUE : YSFALSE);
				air[0]->SetAutopilot(ap);
			}

			info.solo[0]=air[0];
			break;

		case FSACRO_LETTEREIGHT:
			{
				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=4; i++)
				{
					FsAirshowControl::GetDeltaPosition(newpos,i,r);

					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::LETTEREIGHT;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::LETTEREIGHT;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;

		case FSACRO_STARCROSS:
		case FSACRO_STAROFDAVID:
		case FSACRO_BOMBBURST6SHIP:
			{
				int action;

				switch(acroType)
				{
				case FSACRO_STARCROSS:
					action=FsAirshowControl::STARCROSS;
					startPos.SetY(330.0);
					break;
				case FSACRO_STAROFDAVID:
					action=FsAirshowControl::STAROFDAVID;
					startPos.SetY(330.0);
					break;
				default:
				case FSACRO_BOMBBURST6SHIP:
					action=FsAirshowControl::BOMBBURST6SHIP;
					startPos.SetY(240.0);
					break;
				}

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=6; i++)
				{
					if(i==4 && acroType==FSACRO_STARCROSS)
					{
						air[i-1]=nullptr;
						continue;
					}
					air[i-1]=AddAirplane(airType,YSFALSE);
				}

				for(i=2; i<=6; i++)
				{
					if(i==4 && acroType==FSACRO_STARCROSS)
					{
						continue;
					}
					FsAirshowControl::GetDeltaPosition(newpos,i,r);

					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=action;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=newpos;
					ap->formation[0]=air[0];
					ap->formation[1]=air[1];
					ap->formation[2]=air[2];
					ap->formation[3]=air[3];
					ap->formation[4]=air[4];
					ap->formation[5]=air[5];
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=action;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			break;

		case FSACRO_LEVELOPENER:
			{
				startPos.SetY(140.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=5; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
				}

				for(i=2; i<=5; i++)
				{
					if(i<=4)
					{
						FsAirshowControl::GetDeltaPosition(newpos,i,r);
					}
					else
					{
						FsAirshowControl::GetDeltaPosition(newpos,4,r);
						newpos*=2.0;
					}

					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::LEVELOPENER;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=newpos;
					ap->formation[0]=air[0];
					ap->formation[1]=air[1];
					ap->formation[2]=air[2];
					ap->formation[3]=air[3];
					ap->formation[4]=air[4];
					ap->formation[5]=air[5];
					ap->turnDir=(isBlueImpulse==YSTRUE ? -1.0 : 1.0);
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::LEVELOPENER;
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->turnDir=(isBlueImpulse==YSTRUE ? -1.0 : 1.0);
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			break;

		case FSACRO_FORMATIONBREAK:
			{
				int n;
				n=4;

				YsVec3 rel;
				for(i=1; i<n; i++)
				{
					air[i]=AddAirplane(airType,(i==0 ? YSTRUE : YSFALSE));
				}

				startPos.SetY(330.0);
				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=0; i<n; i++)
				{
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::FORMATIONBREAK;
					ap->fomPosition=i+1;
					ap->leader=air[0];
					ap->turnDir=(isBlueImpulse==YSTRUE ? -1.0 : 1.0);
					rel.Set(r*(double)i,-0.2*(double)i,-r*(double)i);
					if(isBlueImpulse==YSTRUE)
					{
						rel.MulX(-1.0);
					}
					ap->shouldBe=rel;
					for(j=0; j<n; j++)
					{
						ap->formation[j]=air[j];
					}
					air[i]->SetAutopilot(ap);

					startAtt.Mul(rel,rel);
					rel+=startPos;
					air[i]->Prop().SetPosition(rel);
					air[i]->Prop().SetAttitude(startAtt);
					air[i]->Prop().SetVelocity(startVel);
					air[i]->Prop().UnloadAllWeapon();
					air[i]->Prop().SendCommand("CTLLDGEA FALSE");
				}
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];

			break;

		case FSACRO_LINEABREASTROLL:
			{
				startPos.SetY(170.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::BARRELROLL;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				air[0]->SetAutopilot(ap);

				for(i=2; i<=3; i++)
				{
					FsFormation *fom;
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					newpos.Set(2.0*r*(double)(i-1),0.0,0.0);

					fom=FsFormation::Create();
					fom->leader=air[0];
					fom->synchronizeTrigger=YSTRUE;
					fom->shouldBe=newpos;
					air[i-1]->SetAutopilot(fom);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];

			break;

		case FSACRO_LINEABREASTLOOP:
			{
				startPos.SetY(170.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=4; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
				}

				for(i=2; i<=4; i++)
				{
					air[i-1]->Prop().UnloadAllWeapon();

					switch(i)
					{
					case 2:
						newpos.Set(-2.0*r,0.0,0.0);
						break;
					case 3:
						newpos.Set( 2.0*r,0.0,0.0);
						break;
					case 4:
						newpos.Set( 4.0*r,0.0,0.0);
						break;
					}

					ap=FsAirshowControl::Create();
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;
					ap->action=FsAirshowControl::LINEABREASTLOOP;
					ap->fomPosition=i;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::LINEABREASTLOOP;
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=3.0;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				air[0]->SetAutopilot(ap);

			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			break;

		case FSACRO_DOUBLEFARVEL:
			{
				YsVec3 newpos;
				int action;

				action=FsAirshowControl::DOUBLEFARVEL;

				startPos.SetY(80.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=4; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetDeltaPosition(newpos,i,r);
					newpos.MulX( 3.0);
					newpos.MulZ( 1.5);
					newpos.MulY(-1.0);
					ap=FsAirshowControl::Create();
					ap->action=action;
					ap->turnDir=(isBlueImpulse==YSTRUE ? -1.0 : 1.0);
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap=FsAirshowControl::Create();
				ap->action=action;
				ap->waitTimer=1.0;
				ap->turnDir=(isBlueImpulse==YSTRUE ? -1.0 : 1.0);
				ap->loopG=4.0;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];

			break;

		case FSACRO_DIAMOND9TOSWANBEND:
			{
				startPos.SetY(170.0);
				startPos-=startAtt.GetForwardVector()*speed*5.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::DIAMOND9TOSWANBEND;
				ap->waitTimer=2.0;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->fomPosition=1;
				ap->changeOverTurnBankAngle=YsPi/3.0;
				ap->waitTimer=3.0;

				for(i=2; i<=9; i++)
				{
					FsAirshowControl *fom;
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetDiamond9Position(newpos,i,r);
					fom=FsAirshowControl::Create();
					fom->action=FsAirshowControl::DIAMOND9TOSWANBEND;
					fom->fomPosition=i;

					fom->leader=air[0];
					fom->synchronizeTrigger=YSFALSE;
					fom->shouldBe=newpos;
					air[i-1]->SetAutopilot(fom);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				ap->formation[6]=air[6];
				ap->formation[7]=air[7];
				ap->formation[8]=air[8];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			info.formation[6]=air[6];
			info.formation[7]=air[7];
			info.formation[8]=air[8];

			break;

		case FSACRO_SWANTOAPOLLOROLL:
			{
				startPos.SetY(170.0);
				startPos-=startAtt.GetForwardVector()*speed*5.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);


				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::SWANTOAPOLLOROLL;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;

				for(i=2; i<=9; i++)
				{
					FsAirshowControl *fom;
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetSwan9Position(newpos,i,r);
					fom=FsAirshowControl::Create();
					fom->action=FsAirshowControl::SWANTOAPOLLOROLL;
					fom->fomPosition=i;

					fom->leader=air[0];
					fom->synchronizeTrigger=YSFALSE;
					fom->shouldBe=newpos;
					air[i-1]->SetAutopilot(fom);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				ap->formation[6]=air[6];
				ap->formation[7]=air[7];
				ap->formation[8]=air[8];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			info.formation[6]=air[6];
			info.formation[7]=air[7];
			info.formation[8]=air[8];

			break;

		case FSACRO_LANCASTERTO5_4SPLIT:
			{
				startPos.SetY(170.0);
				startPos-=startAtt.GetForwardVector()*speed*5.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::LANCASTERTO5_4SPLIT;
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				air[0]->SetAutopilot(ap);

				for(i=2; i<=9; i++)
				{
					FsAirshowControl *fom;
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetLancasterPosition(newpos,i,r);
					fom=FsAirshowControl::Create();
					fom->action=FsAirshowControl::LANCASTERTO5_4SPLIT;
					fom->leader=air[0];
					fom->fomPosition=i;
					fom->synchronizeTrigger=YSFALSE;
					fom->shouldBe=newpos;
					air[i-1]->SetAutopilot(fom);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				ap->formation[6]=air[6];
				ap->formation[7]=air[7];
				ap->formation[8]=air[8];
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			info.formation[6]=air[6];
			info.formation[7]=air[7];
			info.formation[8]=air[8];

			break;

		case FSACRO_CHAMPAIGNSPLIT:
			{
				startPos.SetY(170.0);

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				for(i=2; i<=9; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
				}

				for(i=2; i<=9; i++)
				{
					FsAirshowControl::GetChampaignPosition(newpos,i,r);

					air[i-1]->Prop().UnloadAllWeapon();
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");

					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::CHAMPAIGNSPLIT;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSFALSE;
					ap->shouldBe=newpos;
					ap->formation[0]=air[0];
					ap->formation[1]=air[1];
					ap->formation[2]=air[2];
					ap->formation[4]=air[4];
					ap->formation[5]=air[5];
					ap->formation[6]=air[6];
					ap->formation[7]=air[7];
					ap->formation[8]=air[8];
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::CHAMPAIGNSPLIT;
				ap->fomPosition=1;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				ap->formation[6]=air[6];
				ap->formation[7]=air[7];
				ap->formation[8]=air[8];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			info.formation[6]=air[6];
			info.formation[7]=air[7];
			info.formation[8]=air[8];
			break;

		case FSACRO_VIXENBREAK:
			{
				YsVec3 newpos;

				startPos.SetY(170.0);
				startPos-=startAtt.GetForwardVector()*speed*20.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);


				for(i=2; i<=7; i++)
				{
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					FsAirshowControl::GetChampaignPosition(newpos,i,r);
					ap=FsAirshowControl::Create();
					ap->action=FsAirshowControl::VIXENBREAK;
					ap->fomPosition=i;
					ap->leader=air[0];
					ap->synchronizeTrigger=YSTRUE;
					ap->shouldBe=newpos;
					air[i-1]->SetAutopilot(ap);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::VIXENBREAK;
				ap->waitTimer=2.0;
				ap->fomPosition=1;
				ap->loopG=4.0;
				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				ap->formation[6]=air[6];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			info.formation[6]=air[6];

			break;

		case FSACRO_BIGBATTLETOSHORTDIAMONDLOOP:
			{
				startPos.SetY(170.0);
				startPos-=startAtt.GetForwardVector()*speed*5.0;

				air[0]->Prop().SetPosition(startPos);
				air[0]->Prop().SetAttitude(startAtt);
				air[0]->Prop().SetVelocity(startVel);

				ap=FsAirshowControl::Create();
				ap->action=FsAirshowControl::BIGBATTLETOSHORTDIAMONDLOOP;
				ap->waitTimer=2.0;
				if(isBlueImpulse==YSTRUE)
				{
					ap->turnDir=-1.0;
				}
				ap->fomPosition=1;
				ap->changeOverTurnBankAngle=YsPi/3.0;
				ap->waitTimer=3.0;
				ap->loopThr=0.8;


				YsVec3 fomPos[9];
				fomPos[0]=YsOrigin();
				FsAirshowControl::GetDiamond9Position(fomPos[1],2,r);
				FsAirshowControl::GetDiamond9Position(fomPos[2],3,r);
				fomPos[3]=fomPos[2]*2.0;
				fomPos[4]=fomPos[1]*2.0;
				fomPos[5]=fomPos[2]*3.0;
				fomPos[6]=fomPos[1]*3.0;
				fomPos[7]=fomPos[2]*4.0;
				fomPos[8]=fomPos[1]*4.0;

				for(i=2; i<=9; i++)
				{
					FsAirshowControl *fom;
					air[i-1]=AddAirplane(airType,YSFALSE);
					air[i-1]->Prop().UnloadAllWeapon();

					newpos=fomPos[i-1];
					fom=FsAirshowControl::Create();
					fom->action=FsAirshowControl::BIGBATTLETOSHORTDIAMONDLOOP;
					fom->fomPosition=i;

					fom->leader=air[0];
					fom->synchronizeTrigger=YSFALSE;
					fom->shouldBe=newpos;
					air[i-1]->SetAutopilot(fom);

					startAtt.Mul(newpos,newpos);
					newpos+=startPos;
					air[i-1]->Prop().SetPosition(newpos);
					air[i-1]->Prop().SetAttitude(startAtt);
					air[i-1]->Prop().SetVelocity(startVel);
					air[i-1]->Prop().SendCommand("CTLLDGEA FALSE");
				}

				ap->formation[0]=air[0];
				ap->formation[1]=air[1];
				ap->formation[2]=air[2];
				ap->formation[3]=air[3];
				ap->formation[4]=air[4];
				ap->formation[5]=air[5];
				ap->formation[6]=air[6];
				ap->formation[7]=air[7];
				ap->formation[8]=air[8];
				air[0]->SetAutopilot(ap);
			}

			info.formation[0]=air[0];
			info.formation[1]=air[1];
			info.formation[2]=air[2];
			info.formation[3]=air[3];
			info.formation[4]=air[4];
			info.formation[5]=air[5];
			info.formation[6]=air[6];
			info.formation[7]=air[7];
			info.formation[8]=air[8];

			break;

		default:
			return YSERR;
		}

		for(i=0; i<maxNumAir; i++)
		{
			if(air[i]!=NULL)
			{
				air[i]->Prop().SendCommand("INITFUEL 20%");
				air[i]->Prop().SetSmokeOil(100.0);
			}
		}

		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::PrepareConcordeFlyby(FsDemoModeInfo &info,int concFlyByType)
{
	int i,r;
	FsGround *gnd[2],*gndSeek;


	info.Initialize(FSDEMO_CONCORDEFLYBY);
	info.concFlyByType=concFlyByType;
	info.concFlyByStep=0;


	// info.formation[0] <- Concorde
	// info.formation[1] <- 2nd Concorde or NULL
	// info.formation[2] <- 3rd Concorde or NULL
	// info.solo[x]  <- Red Arrows


	switch(concFlyByType)
	{
	case 0:  // Field must be HEATHROW
		r=rand()&3;

		info.formation[0]=AddAirplane("CONCORDE",YSTRUE);
		switch(r)
		{
		case 0:
			SettleAirplane(*(info.formation[0]),"RW9R");
			break;
		case 1:
			SettleAirplane(*(info.formation[0]),"RW9L");
			break;
		case 2:
			SettleAirplane(*(info.formation[0]),"RW27R");
			break;
		case 3:
			SettleAirplane(*(info.formation[0]),"RW27L");
			break;
		}
		info.formation[0]->Prop().SetThrustVector(0.4);
		break;
	case 1:  // Field must be HEATHROW
		gnd[0]=NULL;
		gnd[1]=NULL;
		gndSeek=NULL;
		while(NULL!=(gndSeek=sim->FindNextGround(gndSeek)))
		{
			if(strcmp(gndSeek->name,"LHR_27R")==0)
			{
				gnd[0]=gndSeek;
				break;
			}
		}
		gndSeek=NULL;
		while(NULL!=(gndSeek=sim->FindNextGround(gndSeek)))
		{
			if(strcmp(gndSeek->name,"LHR_27L")==0)
			{
				gnd[1]=gndSeek;
				break;
			}
		}
		if(gnd[0]!=NULL && gnd[1]!=NULL)
		{
			YsVec3 p,v,off;
			YsAtt3 a;
			double dx;
			int i;
			info.formation[0]=AddAirplane("CONCORDE",YSTRUE);
			info.formation[1]=AddAirplane("CONCORDE",YSTRUE);

			dx=100.0;
			for(i=0; i<2; i++)
			{
				p=gnd[i]->GetPosition();
				a=gnd[i]->GetAttitude();
				a.SetH(a.h()+YsPi);
				p.SetY(900.0);
				off.Set(dx,0.0,0.0);
				a.Mul(off,off);
				p+=off;

				v.Set(0.0,0.0,info.formation[i]->Prop().GetEstimatedLandingSpeed());
				a.Mul(v,v);

				info.formation[i]->Prop().SetPosition(p);
				info.formation[i]->Prop().SetAttitude(a);
				info.formation[i]->Prop().SetVelocity(v);
				info.formation[i]->Prop().SetThrustVector(0.4);
				info.formation[i]->SendCommand("CTLLDGEA FALSE");;

				FsLandingAutopilot *ldg;
				ldg=FsLandingAutopilot::Create();
				ldg->SetAirplaneInfo(*info.formation[i],YsPi/2.0);
				ldg->SetIls(*info.formation[i],sim,gnd[i]);
				ldg->autoClearRunway=YSFALSE;
				info.formation[i]->SetAutopilot(ldg);

				// dx=-dx;
			}
		}
		else
		{
			return YSERR;
		}
		break;
	case 2:  // With Red Arrows  Red Arrows #1  (0,-2,-42)
		if(sim->GetRegionCenterByTag(info.showCenter,"BUCKINGHAM_SHOWCENTER")!=YSOK)
		{
			FsGround *gnd;
			gnd=PickRandomPrimaryIls();
			if(gnd==NULL)
			{
				return YSERR;
			}

			info.showCenter=gnd->GetPosition();
		}
		info.showCenter.SetY(300.0);
		{
			int i;
			YsAtt3 att;
			YsVec3 pos,rel,vel;
			double v;
			double redArrowPos[][3]=
			{
				{  0.0, -2.0, - 0.0},
				{-10.0, -2.5, - 7.0},
				{ 10.0, -2.5, - 7.0},
				{-20.0, -2.5, -14.0},
				{ 20.0, -2.5, -14.0},
				{-30.0, -3.0, -21.0},
				{ 30.0, -3.0, -21.0},
				{-40.0, -3.5, -28.0},
				{ 40.0, -3.5, -28.0},
			};


			att=YsZeroAtt();
			att.SetH(YsPi*2.0*(double)(rand()%100)/100.0);
			rel.Set(0.0,0.0,-9000.0);
			att.Mul(rel,rel);

			pos=info.showCenter+rel;
			att.SetH(att.h()+(double)(rand()%20-10)*YsPi/180.0);

			info.formation[0]=AddAirplane("CONCORDE",YSTRUE);
			v=info.formation[0]->Prop().GetEstimatedLandingSpeed()*2.2;
			info.formation[0]->Prop().SetPosition(pos);
			info.formation[0]->Prop().SetAttitude(att);
			vel.Set(0.0,0.0,v);
			att.Mul(vel,vel);
			info.formation[0]->Prop().SetVelocity(vel);
			info.formation[0]->Prop().SetThrustVector(0.4);
			info.formation[0]->SendCommand("CTLLDGEA FALSE");

			FsGotoPosition *gp;
			gp=FsGotoPosition::Create();
			gp->destination.Set(1,&info.showCenter);
			gp->speed=v;
			gp->minAlt=0.0;
			info.formation[0]->SetAutopilot(gp);

			for(i=0; i<9; i++)
			{
				YsVec3 wgPos;
				wgPos.Set(redArrowPos[i][0],redArrowPos[i][1],-42+redArrowPos[i][2]);

				FsFormation *fom;
				fom=FsFormation::Create();
				fom->leader=info.formation[0];
				fom->synchronizeTrigger=YSFALSE;
				fom->shouldBe=wgPos;

				att.Mul(wgPos,wgPos);
				wgPos+=pos;

				info.solo[i]=AddAirplane("HAWK_REDARROWS",YSFALSE);
				info.solo[i]->Prop().SetPosition(wgPos);
				info.solo[i]->Prop().SetAttitude(att);
				info.solo[i]->Prop().SetVelocity(vel);
				info.solo[i]->SendCommand("CTLLDGEA FALSE");
				info.solo[i]->SetAutopilot(fom);
			}
		}
		break;

	case 3:
		{
			YSBOOL right;
			if(rand()%100<50)
			{
				right=YSTRUE;
				if(sim->GetRegionCenterByTag(info.showCenter,"LHR_27R_TURN")!=YSOK)
				{
					return YSERR;
				}
			}
			else
			{
				right=YSFALSE;
				if(sim->GetRegionCenterByTag(info.showCenter,"LHR_27L_TURN")!=YSOK)
				{
					return YSERR;
				}
			}

			gndSeek=NULL;
			info.refObj=NULL;
			while(NULL!=(gndSeek=sim->FindNextGround(gndSeek)))
			{
				if(strcmp(gndSeek->name,"LHR_27R")==0)
				{
					info.refObj=gndSeek;
					break;
				}
			}
			if(info.refObj==NULL)
			{
				return YSERR;
			}

			{
				YsVec3 pos,vel;
				YsAtt3 att;
				double v;
				att.Set(-YsPi/2.0,0.0,0.0);

				for(i=0; i<3; i++)
				{
					pos.Set(-i*48.0,-i,-i*64.0);
					if(right!=YSTRUE)
					{
						pos.MulX(-1.0);
					}

					att.Mul(pos,pos);
					pos+=info.showCenter;
					pos.SetY(330.0);

					info.formation[i]=AddAirplane("CONCORDE",(i==2 ? YSTRUE : YSFALSE));
					info.formation[i]->Prop().SetPosition(pos);
					info.formation[i]->Prop().SetAttitude(att);

					v=info.formation[0]->Prop().GetEstimatedLandingSpeed()*1.9;
					vel.Set(0.0,0.0,v);
					att.Mul(vel,vel);

					info.formation[i]->Prop().SetVelocity(vel);
					info.formation[i]->Prop().SetThrustVector(0.4);
					info.formation[i]->SendCommand("CTLLDGEA FALSE");;

					FsGotoPosition *gp;
					gp=FsGotoPosition::Create();
					gp->destination.Set(1,&info.showCenter);
					gp->destination.GetEnd().SetY(330.0);
					gp->straightFlightMode=YSTRUE;
					gp->speed=v;
					gp->minAlt=0.0;
					info.formation[i]->SetAutopilot(gp);
				}
			}
		}
		break;

	case 4:  // Gatwick -> LHR
		{
			const char *const stp[]=
			{
				"GATWICK_26R",
				"GATWICK_26L",
				"GATWICK_8L",
				"GATWICK_8R",
				NULL
			};
			const char *const dst[]=
			{
				"LHR_27R",
				"LHR_27L",
				"LHR_9R",
				"LHR_9L",
				NULL
			};

			r=rand()&3;

			info.formation[0]=AddAirplane("CONCORDE",YSTRUE);
			SettleAirplane(*(info.formation[0]),stp[r]);
			info.formation[0]->Prop().SetThrustVector(0.4);

			if(sim->GetRegionCenterByTag(info.showCenter,"HEATHROW")!=YSOK)
			{
				return YSERR;
			}

			r=rand()&3;

			gndSeek=NULL;
			info.refObj=NULL;
			while(NULL!=(gndSeek=sim->FindNextGround(gndSeek)))
			{
				if(strcmp(gndSeek->name,dst[r])==0)
				{
					info.refObj=gndSeek;
					break;
				}
			}
			if(info.refObj==NULL)
			{
				return YSERR;
			}

			FsTakeOffAutopilot *to;
			to=FsTakeOffAutopilot::Create();
			to->desigAlt=2000.0;
			info.formation[0]->SetAutopilot(to);
		}
		break;

	case 5:  // Supersonic
		{
			char stpName[256];
			YsString fldNameBuf;
			const char *fldName;
			YsVec3 fldPos;
			YsAtt3 fldAtt;

			info.formation[0]=AddAirplane("CONCORDE",YSTRUE);

			DisableGroundFire();

			if(GetLoadedField(fldNameBuf,fldPos,fldAtt)==YSOK)
			{
				int i,n;
				fldName=fldNameBuf;
				n=0;
				for(n=0; GetFieldStartPositionName(stpName,fldName,n)==YSOK; n++)
				{
				}
				if(n>0)
				{
					YsVec3 pos,vel,cen;
					double v,mach;

					i=rand()%n;
					GetFieldStartPositionName(stpName,fldName,i);
					SettleAirplane(*(info.formation[0]),stpName);
					cen=info.formation[0]->GetPosition();
					pos=cen;
					pos.SetY(16000.0);

					mach=FsGetMachOne(16000.0);
					v=mach*1.95;
					vel.Set(0.0,0.0,v);
					info.formation[0]->GetAttitude().Mul(vel,vel);

					pos-=vel*30.0;

					info.formation[0]->Prop().SetPosition(pos);
					info.formation[0]->Prop().SetVelocity(vel);


					FsGotoPosition *gp;
					gp=FsGotoPosition::Create();
					gp->destination.Set(1,&pos);
					gp->straightFlightMode=YSTRUE;
					gp->speed=mach*2.2;
					gp->minAlt=0.0;
					info.formation[0]->SetAutopilot(gp);

					info.formation[0]->SendCommand("INITFUEL 50%");
					info.formation[0]->SendCommand("CTLLDGEA FALSE");
					return YSOK;
				}
			}
			return YSERR;
		}
		break;
	case 6:  // Low altitude, passing other airliners
		{
			char stpName[256];
			YsString fldNameBuf;
			const char *fldName;
			YsVec3 fldPos;
			YsAtt3 fldAtt;

			info.formation[0]=AddAirplane("CONCORDE",YSTRUE);

			DisableGroundFire();

			if(GetLoadedField(fldNameBuf,fldPos,fldAtt)==YSOK)
			{
				int i,n;
				fldName=fldNameBuf;
				n=0;
				for(n=0; GetFieldStartPositionName(stpName,fldName,n)==YSOK; n++)
				{
				}
				if(n>0)
				{
					YsVec3 pos,vel,cen;
					YsAtt3 att;
					double v,mach;

					i=rand()%n;
					GetFieldStartPositionName(stpName,fldName,i);
					SettleAirplane(*(info.formation[0]),stpName);
					cen=info.formation[0]->GetPosition();
					att=info.formation[0]->GetAttitude();
					att.SetP(0.0);
					pos=cen;
					pos.SetY(12000.0);

					mach=FsGetMachOne(12000.0);
					v=mach*1.95;
					vel.Set(0.0,0.0,v);
					info.formation[0]->GetAttitude().Mul(vel,vel);

					pos-=vel*30.0;

					info.formation[0]->Prop().SetPosition(pos);
					info.formation[0]->Prop().SetVelocity(vel);


					FsGotoPosition *gp;
					gp=FsGotoPosition::Create();
					gp->destination.Set(1,&pos);
					gp->straightFlightMode=YSTRUE;
					gp->speed=mach*2.2;
					gp->minAlt=0.0;
					info.formation[0]->SetAutopilot(gp);

					info.formation[0]->SendCommand("INITFUEL 50%");
					info.formation[0]->SendCommand("CTLLDGEA FALSE");

					int nAirliner;
					char *airliner[256];
					if(GetJetAirlinerList(nAirliner,airliner,256)==YSOK && nAirliner>0)
					{
						int nAdd;
						nAdd=3+(rand()&3);
						for(i=0; i<nAdd; i++)
						{
							FsAirplane *air;
							double x,y,z;
							YsVec3 ofst;

							x=((i&1) ? -600.0 : 600.0);
							y=(double)(rand()%300-150);
							z=4000.0+i*4000.0;

							ofst.Set(x,y,z);
							att.Mul(ofst,ofst);

							n=rand()%nAirliner;
							if(strcmp(airliner[n],"CONCORDE")!=0)
							{
								air=AddAirplane(airliner[n],YSFALSE);

								air->Prop().SetPosition(pos+ofst);
								air->Prop().SetAttitude(att);

								v=air->Prop().GetEstimatedCruiseSpeed();
								vel.Set(0.0,0.0,v);
								air->GetAttitude().Mul(vel,vel);
								air->Prop().SetVelocity(vel);

								air->SendCommand("INITFUEL 50%");
								air->SendCommand("CTLLDGEA FALSE");

								gp=FsGotoPosition::Create();
								gp->destination.Set(1,&pos);
								gp->straightFlightMode=YSTRUE;
								gp->speed=v;
								gp->minAlt=0.0;
								air->SetAutopilot(gp);
							}
						}
					}

					return YSOK;
				}
			}
			return YSERR;
		}
		break;
	case 7:  // Landing
		{
			FsAirplane *air;
			air=AddAirplane("CONCORDE",YSTRUE);
			if(SettleAirplaneForLandingDemo(*air,2200.0,YsPi/6.0)==YSOK)
			{
				FsLandingAutopilot *ap;
				ap=(FsLandingAutopilot *)air->GetAutopilot();
				ap->autoClearRunway=YSFALSE;
				info.Initialize(FSDEMO_LANDING);
			}
			else
			{
				return YSERR;
			}
		}
		break;
	case 8:  // Rock wing pass (Dirty)
		PrepareAcrobat(info,"CONCORDE",FSACRO_ROCKWINGDIRTY);
		break;
	case 9:  // Rock wing pass (Clean)
		PrepareAcrobat(info,"CONCORDE",FSACRO_ROCKWINGCLEAN);
		break;

	case 10:  // Formation with Spitfire
	case 11:
		{
			int n;
			YsVec3 startPos,ofst,vel;
			YsAtt3 startAtt,concAtt;
			YsArray <FsGround *> ilsViewList;
			FsGround *ils;
			double v;
			FsFormation *fom;

			// sim->RefreshAircraftCarrierAndVorList();
			int i;
			for(i=0; i<sim->GetNumILSFacility(); i++)
			{
				FsGround *ils;
				ils=sim->GetILS(i);
				if(ils->Prop().GetAircraftCarrierProperty()->CanBeViewpoint()==YSTRUE)
				{
					ilsViewList.Append(ils);
				}
			}
			if(ilsViewList.GetN()>0)
			{
				n=rand()%ilsViewList.GetN();
				ils=ilsViewList[n];
				startPos=ils->GetPosition();
				startAtt=ils->GetAttitude();

				startAtt.SetH(startAtt.h()+YsPi);

				ofst.Set(0.0,0.0,-1500.0);
				startPos+=startAtt*ofst;

				startPos.SetY(150.0);


				concAtt=startAtt;
				concAtt.SetP(YsPi/20.0);
				info.formation[0]=AddAirplane("CONCORDE",YSTRUE);
				info.formation[0]->Prop().SetPosition(startPos);
				info.formation[0]->Prop().SetAttitude(concAtt);

				v=info.formation[0]->Prop().GetEstimatedLandingSpeed()*1.15;
				vel.Set(0.0,0.0,v);
				startAtt.Mul(vel,vel);

				info.formation[0]->Prop().SetVelocity(vel);
				info.formation[0]->Prop().SetThrustVector(1.0);
				info.formation[0]->SendCommand("CTLLDGEA TRUE");

				FsGotoPosition *gp;
				gp=FsGotoPosition::Create();
				gp->destination.Set(1,&startPos);
				gp->straightFlightMode=YSTRUE;
				gp->speed=v;
				gp->minAlt=0.0;
				info.formation[0]->SetAutopilot(gp);


				info.solo[1]=AddAirplane("SPITFIRE",YSFALSE);
				ofst.Set(20.0,0.0,-29.0);
				info.solo[1]->Prop().SetPosition(startPos+startAtt*ofst);
				info.solo[1]->Prop().SetAttitude(startAtt);
				info.solo[1]->Prop().SetVelocity(vel);
				info.solo[1]->SendCommand("CTLLDGEA FALSE");

				fom=FsFormation::Create();
				fom->leader=info.formation[0];
				fom->synchronizeTrigger=YSFALSE;
				fom->synchronizeLandingGear=YSFALSE;
				fom->shouldBe=ofst;
				info.solo[1]->SetAutopilot(fom);

				ofst.Set(30.0,0.0,-37.0);

				info.solo[2]=AddAirplane("SPITFIRE",YSFALSE);
				info.solo[2]->Prop().SetPosition(startPos+startAtt*ofst);
				info.solo[2]->Prop().SetAttitude(startAtt);
				info.solo[2]->Prop().SetVelocity(vel);
				info.solo[2]->SendCommand("CTLLDGEA FALSE");

				fom=FsFormation::Create();
				fom->leader=info.formation[0];
				fom->synchronizeTrigger=YSFALSE;
				fom->synchronizeLandingGear=YSFALSE;
				fom->shouldBe=ofst;
				info.solo[2]->SetAutopilot(fom);


				info.solo[1]=AddAirplane("HURRICANE",YSFALSE);
				ofst.Set(-20.0,0.0,-29.0);
				info.solo[1]->Prop().SetPosition(startPos+startAtt*ofst);
				info.solo[1]->Prop().SetAttitude(startAtt);
				info.solo[1]->Prop().SetVelocity(vel);
				info.solo[1]->SendCommand("CTLLDGEA FALSE");

				fom=FsFormation::Create();
				fom->leader=info.formation[0];
				fom->synchronizeTrigger=YSFALSE;
				fom->synchronizeLandingGear=YSFALSE;
				fom->shouldBe=ofst;
				info.solo[1]->SetAutopilot(fom);

				ofst.Set(-30.0,0.0,-37.0);

				info.solo[2]=AddAirplane("HURRICANE",YSFALSE);
				info.solo[2]->Prop().SetPosition(startPos+startAtt*ofst);
				info.solo[2]->Prop().SetAttitude(startAtt);
				info.solo[2]->Prop().SetVelocity(vel);
				info.solo[2]->SendCommand("CTLLDGEA FALSE");

				fom=FsFormation::Create();
				fom->leader=info.formation[0];
				fom->synchronizeTrigger=YSFALSE;
				fom->synchronizeLandingGear=YSFALSE;
				fom->shouldBe=ofst;
				info.solo[2]->SetAutopilot(fom);


				if(concFlyByType==11)
				{
					ofst.Set(0.0,0.0,-44.0);

					info.solo[2]=AddAirplane("LANCASTER",YSFALSE);
					info.solo[2]->Prop().SetPosition(startPos+startAtt*ofst);
					info.solo[2]->Prop().SetAttitude(startAtt);
					info.solo[2]->Prop().SetVelocity(vel);
					info.solo[2]->SendCommand("CTLLDGEA FALSE");
					info.solo[2]->Prop().UnloadAllWeapon();

					fom=FsFormation::Create();
					fom->leader=info.formation[0];
					fom->synchronizeTrigger=YSFALSE;
					fom->synchronizeLandingGear=YSFALSE;
					startAtt.Mul(ofst,ofst);
					concAtt.MulInverse(ofst,ofst);
					fom->shouldBe=ofst;
					info.solo[2]->SetAutopilot(fom);
				}
			}
			else
			{
				return YSERR;
			}
		}
		break;
	}

	return YSOK;
}

YSBOOL FsWorld::DemoModeOneStep(FsDemoModeInfo &info,YSBOOL drawSmokeAndVapor,YSBOOL preserveFlightRecord)
{
	if(NULL!=sim)
	{
		return sim->DemoModeOneStep(info,drawSmokeAndVapor,preserveFlightRecord);
	}
	return YSFALSE;
}

YSRESULT FsWorld::AfterDemoMode(FsDemoModeInfo &info)
{
	if(NULL!=sim)
	{
		return sim->AfterDemoMode(info);
	}
	return YSERR;
}

YSRESULT FsWorld::PrepareReplaySimulation(void)
{
	if(NULL!=sim)
	{
		sim->PrepareReplaySimulation();
		return YSOK;
	}
	return YSERR;
}

void FsWorld::DeleteEventByTypeAll(int eventType)
{
	if(NULL!=sim)
	{
		sim->DeleteEventByTypeAll(eventType);
	}
}

void FsWorld::ClearFirstPlayer(void)
{
	if(NULL!=sim)
	{
		sim->ClearFirstPlayer();
	}
}

YSRESULT FsWorld::TerminateSimulation(void)
{
	if(NULL!=sim)
	{
		delete sim;
		sim=NULL;
		return YSOK;
	}
	return YSERR;
}

void FsWorld::TestSolidCloud(void)
{
	sim->TestSolidCloud();
}

YSRESULT FsWorld::MakeSolidCloud(
    int n,const YsVec3 &cen,const double &range,const double &sizeH,const double &y0,const double &y1)
{
	if(NULL!=sim)
	{
		sim->MakeSolidCloud(n,cen,range,sizeH,y0,y1);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::SetFog(YSBOOL drawFog,const double &visibility)
{
	if(NULL!=sim)
	{
		sim->SetFog(drawFog,visibility);
		return YSOK;
	}
	return YSERR;
}

void FsWorld::DrawInNormalSimulationMode(FsSimulation::FSSIMULATIONSTATE simState,YSBOOL demoMode,YSBOOL showTimer,YSBOOL showTimeMarker)
{
	if(nullptr!=sim)
	{
		sim->DrawInNormalSimulationMode(simState,demoMode,showTimer,showTimeMarker);
	}
}
void FsWorld::DrawInClientMode(const FsClientRunLoop &clientModeRunLoop)
{
	if(nullptr!=sim)
	{
		sim->DrawInClientMode(clientModeRunLoop);
	}
}
void FsWorld::DrawInServerMode(const FsServerRunLoop &serverModeRunLoop)
{
	if(nullptr!=sim)
	{
		sim->DrawInServerMode(serverModeRunLoop);
	}
}

YSRESULT FsWorld::SetAllowedWeaponType(unsigned int allowedWeaponType)
{
	if(NULL!=sim)
	{
		return sim->SetAllowedWeaponType(allowedWeaponType);
	}
	return YSERR;
}

void FsWorld::AllowGun(YSBOOL a)
{
	if(NULL!=sim)
	{
		sim->AllowGun(a);
	}
}

void FsWorld::AllowAAM(YSBOOL a)
{
	if(NULL!=sim)
	{
		sim->AllowAAM(a);
	}
}

void FsWorld::AllowAGM(YSBOOL a)
{
	if(NULL!=sim)
	{
		sim->AllowAGM(a);
	}
}

void FsWorld::AllowBomb(YSBOOL a)
{
	if(NULL!=sim)
	{
		sim->AllowBomb(a);
	}
}

void FsWorld::AllowRocket(YSBOOL a)
{
	if(NULL!=sim)
	{
		sim->AllowRocket(a);
	}
}

void FsWorld::SetReplayMode(FsSimulation::FSREPLAYMODE replMode)
{
	if(NULL!=sim)
	{
		sim->SetReplayMode(replMode);
	}
}

const double &FsWorld::GetSimulationTime(void)
{
	if(NULL!=sim)
	{
		return sim->currentTime;
	}
	else
	{
		static double zero=0.0;
		return zero;
	}
}

void FsWorld::Jump(const double &newTime)
{
	if(newTime<sim->currentTime)
	{
		sim->SimResetPlayerObjectFromRecord(newTime,sim->currentTime);
		sim->currentTime=YsGreater(0.0,newTime-30.0);
	}

	const double prevTime=sim->currentTime;

	sim->SimSeekNextEventPointereToCurrentTime(prevTime);
	sim->FastForward(newTime);
	sim->RefreshOrdinanceByWeaponRecord(prevTime);
}

void FsWorld::JumpToFirstRecordTime(void)
{
	if(NULL!=sim)
	{
		double t;
		t=sim->GetFirstRecordTime();
		Jump(t);
	}
}

void FsWorld::JumpToLastRecordTime(void)
{
	if(NULL!=sim)
	{
		double t;
		t=sim->GetLastRecordTime();
		Jump(t);
	}
}

YSRESULT FsWorld::DeleteFlightRecord(const double &t1,const double &t2)
{
	sim->DeleteFlightRecord(t1,t2);
	return YSOK;
}


YSRESULT FsWorld::ApplyConfig(YSBOOL changeEnvironment)
{
	if(this!=NULL && sim!=NULL)
	{
		return sim->LoadConfigFile(FsGetConfigFile(),changeEnvironment);
	}
	return YSERR;
}

YSBOOL FsWorld::SimulationIsPrepared(void)
{
	if(NULL!=sim)
	{
		return YSTRUE;
	}
	else
	{
		return YSFALSE;
	}
}

YSBOOL FsWorld::PlayerPlaneIsReady(void) const
{
	if(sim==NULL)
	{
		return YSFALSE;
	}

	FsAirplane *air=GetPlayerAirplane();
	if(air==NULL)
	{
		return YSFALSE;
	}

	if(air->rec!=NULL)
	{
		return YSFALSE;
	}

	return YSTRUE;
}

YSBOOL FsWorld::PlayerGroundIsReady(void) const
{
	if(sim==NULL)
	{
		return YSFALSE;
	}

	FsGround *gnd=GetPlayerGround();
	if(gnd==NULL)
	{
		return YSFALSE;
	}

	if(gnd->rec!=NULL)
	{
		return YSFALSE;
	}

	return YSTRUE;
}

YSBOOL FsWorld::IsFlightRecord(void)
{
	if(sim==NULL)
	{
		return YSFALSE;
	}
	else
	{
		return sim->AtLeastOneAirplaneIsRecordedAirplane();
	}
}

FsAirplane *FsWorld::GetPlayerAirplane(void) const
{
	if(NULL!=sim)
	{
		return sim->GetPlayerAirplane();
	}
	return NULL;
}

FsGround *FsWorld::GetPlayerGround(void) const
{
	if(NULL!=sim)
	{
		return sim->GetPlayerGround();
	}
	return NULL;
}

int FsWorld::GetNumAirplaneLoaded(void)
{
	if(NULL!=sim)
	{
		return sim->GetNumAirplane();
	}
	return 0;
}

int FsWorld::GetNumPrimaryGroundTarget(FSIFF iff)
{
	if(NULL!=sim)
	{
		return sim->GetNumPrimaryGroundTarget(iff);
	}
	return 0;
}

YSBOOL FsWorld::IsFieldLoaded(void) const
{
	if(NULL!=sim && sim->GetField()!=NULL)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

YSRESULT FsWorld::GetLoadedField(YsString &fieldName,YsVec3 &pos,YsAtt3 &att)
{
	if(NULL!=sim)
	{
		return sim->GetLoadedField(fieldName,pos,att);
	}
	return YSERR;
}

void FsWorld::UnprepareAllTemplate(void)
{
	YsListItem <FsAirplaneTemplate> *air;
	YsListItem <FsGroundTemplate> *gnd;
	YsListItem <FsFieldTemplate> *fld;

	air=NULL;
	while((air=airplaneTemplate.FindNext(air))!=NULL)
	{
		air->dat.Unprepare();
	}
	gnd=NULL;
	while((gnd=groundTemplate.FindNext(gnd))!=NULL)
	{
		gnd->dat.vis.CleanUp();
		gnd->dat.lod.CleanUp();
		gnd->dat.cockpit.CleanUp();
		if(gnd->dat.coll!=NULL)
		{
			delete gnd->dat.coll;
			gnd->dat.coll=NULL;
		}
	}
	fld=NULL;
	while((fld=fieldTemplate.FindNext(fld))!=NULL)
	{
		if(fld->dat.GetField()!=NULL)
		{
			fld->dat.DeleteField();
		}
	}

	FsAirplaneTemplateAllocator.CollectGarbage();
	FsGroundTemplateAllocator.CollectGarbage();
	FsFieldTemplateAllocator.CollectGarbage();

	YsShell::CleanUpVertexPolygonStore();  // <- Due to presence of joystick,missiles this will not delete all.
	YsScenery::CollectGarbage();

	// printf("FsWorld::~UnprepareAllTemplate()\n");
	// YsScenery::SelfDiagnostic();
	// YsTextFile::SelfDiagnostic();
}

void FsWorld::UnprepareAirplaneTemplate(void)
{
	YsListItem <FsAirplaneTemplate> *air;

	air=NULL;
	while((air=airplaneTemplate.FindNext(air))!=NULL)
	{
		air->dat.Unprepare();
	}

	FsAirplaneTemplateAllocator.CollectGarbage();

	YsShell::CleanUpVertexPolygonStore();  // <- Due to presence of joystick,missiles this will not delete all.
	YsScenery::CollectGarbage();
}

YSRESULT FsWorld::SetWind(const YsVec3 &wind)
{
	if(NULL!=sim && sim->AtLeastOneAirplaneIsRecordedAirplane()!=YSTRUE)
	{
		sim->GetWeather().SetWind(wind);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::GetWind(YsVec3 &wind) const
{
	if(NULL!=sim)
	{
		wind=sim->GetWeather().GetWind();
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::SetFogVisibility(const double &visibility)
{
	if(NULL!=sim && sim->AtLeastOneAirplaneIsRecordedAirplane()!=YSTRUE)
	{
		sim->GetWeather().SetFogVisibility(visibility);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsWorld::AddOvercastLayer(const double &flr,const double &top)
{
	if(NULL!=sim && sim->AtLeastOneAirplaneIsRecordedAirplane()!=YSTRUE)
	{
		FsWeatherCloudLayer cloudLayer;
		cloudLayer.cloudLayerType=FSCLOUDLAYER_OVERCAST;
		cloudLayer.y0=flr;
		cloudLayer.y1=top;
		sim->GetWeather().AddCloudLayer(cloudLayer);
		return YSOK;
	}
	return YSERR;
}

double FsWorld::GetFogVisibility(void) const
{
	if(NULL!=sim)
	{
		return sim->GetWeather().GetFogVisibility();
	}
	return 0.0;
}

YSRESULT FsWorld::SetFog(YSBOOL fog)
{
	if(NULL!=sim)
	{
		sim->GetWeather().SetFog(fog);
		return YSOK;
	}
	return YSERR;
}

YSBOOL FsWorld::GetFog(void) const
{
	if(NULL!=sim)
	{
		return sim->GetWeather().GetFog();
	}
	return YSFALSE;
}

double FsWorld::GetFieldElevation(const double &x,const double &z) const
{
	if(NULL!=sim)
	{
		return sim->GetFieldElevation(x,z);
	}
	return 0.0;
}

int FsWorld::RerecordByNewInterval(const double &itvl)
{
	if(NULL!=sim)
	{
		return sim->RerecordByNewInterval(itvl);
	}
	return 0;
}

void FsWorld::AdjustPrecisionOfFlightRecord(const double &precPos,const double &precAng)
{
	if(NULL!=sim)
	{
		sim->AdjustPrecisionOfFlightRecord(precPos,precAng);
	}
}

