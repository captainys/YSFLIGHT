#include <ysclass.h>
#include <ysport.h>
#include <ysunitconv.h>

#include "fs.h"
#include "graphics/common/fsopengl.h"
#include "fsutil.h"

#include "fsinstpanel.h"
#include "fswirefont.h"

#include "fsinstreading.h"


FsInstrumentPanel::FsInstrumentPanel()
{
	// panelPlg.Append(YsVec2(-0.92,-1.0));
	// panelPlg.Append(YsVec2(-0.85,-0.37));
	// panelPlg.Append(YsVec2(-0.82,-0.35));
	// panelPlg.Append(YsVec2( 0.82,-0.35));
	// panelPlg.Append(YsVec2( 0.85,-0.37));
	// panelPlg.Append(YsVec2( 0.92,-1.0));

	panelRange[0].Set(-1.00,-1.0);
	panelRange[1].Set( 0.95,-0.27);

	drawSpeed=YSTRUE;
	speedPos.Set(-0.64,-0.52);
	speedRad=0.15;
	speedShowMachNumber=YSFALSE;
	speedVfe=-1.0;
	speedVno=-1.0;
	speedVne=-1.0;
	speedIndicatorRange=-1.0;
	speedArc=-1.0;

	drawHdg=YSTRUE;
	hdgPos.Set(-0.32,-0.84);
	hdgRad=0.15;

	drawHsi=YSFALSE;

	drawAlt=YSTRUE;
	altPos.Set( 0.0,-0.52);
	altRad=0.15;

	drawClimb=YSTRUE;
	climbPos.Set(0.0,-0.84);
	climbRad=0.15;

	drawAtt=YSTRUE;
	attPos.Set(-0.32,-0.52);
	attRad=0.15;

	drawEngine=YSTRUE;
	enginePos.Set(0.64,-0.84);
	engineRad=0.15;

	drawRPM=YSFALSE;

	drawIls=YSTRUE;
	ilsPos.Set( 0.32,-0.52);
	ilsRad=0.15;

	drawVor=YSTRUE;
	vorPos.Set(0.32,-0.84);
	vorRad=0.15;

	drawAdf=YSTRUE;
	adfPos.Set(0.96,-0.52);
	adfRad=0.15;

	drawTurnCoordinator=YSTRUE;
	turnCoordinatorPos.Set(-0.64,-0.84);
	turnCoordinatorRad=0.15;

	drawG=YSTRUE;
	g1.Set(-0.91,-0.99); // (0.20,-0.67);
	g2.Set(-0.81,-0.69); // (0.32,-0.37);

	drawTrim=YSTRUE;
	trim1.Set(0.94,-0.99);
	trim2.Set(1.02,-0.69);

	drawGear=YSTRUE;
	gear1.Set(-0.05,-0.35); // (0.35,-0.67);   // w=0.12  h=14
	gear2.Set( 0.05,-0.27); // (0.47,-0.53);

	drawBrake=YSTRUE;
	brake1.Set(-0.17,-0.35); // (0.35,-0.51);
	brake2.Set(-0.07,-0.27); // (0.47,-0.37);

	drawSpoiler=YSTRUE;
	spoil1.Set(0.50,-0.67);
	spoil2.Set(0.62,-0.37);

	drawFuel=YSTRUE;
	fuel1.Set(0.65,-0.67);
	fuel2.Set(0.77,-0.37);

	drawFlap=YSTRUE;
	flap1.Set(0.80,-0.99);
	flap2.Set(0.92,-0.69);

	drawAmmo=YSFALSE;
	ammoPos.Set(0.0,0.0);
	ammoRad=0.1;

	drawCrossHair=YSFALSE;
	crossHairPos.Set(0.0,0.0);
	crossHairRad=0.05;
}

FsInstrumentPanel::~FsInstrumentPanel()
{
}

static char const *FsInstPanelCmd[]=
{
	"REM",
	"PNLRANGE",
	"AIRSPEED",
	"ALTITUDE",
	"ATTITUDE",
	"HEADING_",
	"VERTICAL",
	"THROTTLE",
	"ILS_____",
	"VOR_____",
	"GLOADING",
	"FUEL____",
	"GEAR____",
	"FLAP____",
	"SPOILER_",
	"BRAKE___",
	"AMMO____",
	"CROSHAIR",
	"TRNCOORD",

	"SPEEDVFE",
	"SPEEDVNO",
	"SPEEDVNE",
	"SPEEDRNG",
	"SPEEDMCH",
	"SPEEDARC",

	"ADF_____",

	// 2010/07/05
	"HSI_____",

	// 2011/12/28
	"ELEVTRIM",

	// 2014/06/20
	"NUMTACHO",
	"TACHOPOS",
	"TACHORPM",
	"TACHOGRN",
	"TACHORED",
	"TACHOTCK",
	"TACHOANG",

	NULL
};

YSRESULT FsInstrumentPanel::LoadIsp(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		YsString str;
		YsArray <YsString,16> args;

		drawSpeed=YSFALSE;
		drawHdg=YSFALSE;
		drawHsi=YSFALSE;
		drawAlt=YSFALSE;
		drawClimb=YSFALSE;
		drawAtt=YSFALSE;
		drawEngine=YSFALSE;
		drawIls=YSFALSE;
		drawVor=YSFALSE;
		drawG=YSFALSE;
		drawTrim=YSFALSE;
		drawGear=YSFALSE;
		drawBrake=YSFALSE;
		drawSpoiler=YSFALSE;
		drawFuel=YSFALSE;
		drawFlap=YSFALSE;
		drawAmmo=YSFALSE;
		drawAdf=YSFALSE;
		drawTurnCoordinator=YSFALSE;

		while(str.Fgets(fp)!=NULL)
		{
			str.Arguments(args);

			if(args.GetN()>0)
			{
				YSRESULT res;
				int i,cmd;
				cmd=-1;
				for(i=0; FsInstPanelCmd[i]!=NULL; i++)
				{
					if(strcmp(args[0],FsInstPanelCmd[i])==0)
					{
						cmd=i;
						break;
					}
				}

				res=YSERR;
				switch(cmd)
				{
				default:
					break;
				case 0: // "REM",
					res=YSOK;
					break;
				case 1: // "PNLRANGE",
					if(args.GetN()>=5)
					{
						panelRange[0].Set(atof(args[1]),atof(args[2]));
						panelRange[1].Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 2: // "AIRSPEED",
					if(args.GetN()>=4)
					{
						drawSpeed=YSTRUE;
						speedPos.Set(atof(args[1]),atof(args[2]));
						speedRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 3: // "ALTITUDE",
					if(args.GetN()>=4)
					{
						drawAlt=YSTRUE;
						altPos.Set(atof(args[1]),atof(args[2]));
						altRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 4: // "ATTITUDE",
					if(args.GetN()>=4)
					{
						drawAtt=YSTRUE;
						attPos.Set(atof(args[1]),atof(args[2]));
						attRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 5: // "HEADING_",
					if(args.GetN()>=4)
					{
						drawHdg=YSTRUE;
						hdgPos.Set(atof(args[1]),atof(args[2]));
						hdgRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 6: // "VERTICAL",
					if(args.GetN()>=4)
					{
						drawClimb=YSTRUE;
						climbPos.Set(atof(args[1]),atof(args[2]));
						climbRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 7: // "THROTTLE",
					if(args.GetN()>=4)
					{
						drawEngine=YSTRUE;
						enginePos.Set(atof(args[1]),atof(args[2]));
						engineRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 8: // "ILS_____",
					if(args.GetN()>=4)
					{
						drawIls=YSTRUE;
						ilsPos.Set(atof(args[1]),atof(args[2]));
						ilsRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 9: // "VOR_____",
					if(args.GetN()>=4)
					{
						drawVor=YSTRUE;
						vorPos.Set(atof(args[1]),atof(args[2]));
						vorRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 10: // "GLOADING",
					if(args.GetN()>=5)
					{
						drawG=YSTRUE;
						g1.Set(atof(args[1]),atof(args[2]));
						g2.Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 11: // "FUEL____",
					if(args.GetN()>=5)
					{
						drawFuel=YSTRUE;
						fuel1.Set(atof(args[1]),atof(args[2]));
						fuel2.Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 12: // "GEAR____",
					if(args.GetN()>=5)
					{
						drawGear=YSTRUE;
						gear1.Set(atof(args[1]),atof(args[2]));
						gear2.Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 13: // "FLAP____",
					if(args.GetN()>=5)
					{
						drawFlap=YSTRUE;
						flap1.Set(atof(args[1]),atof(args[2]));
						flap2.Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 14: // "SPOILER_",
					if(args.GetN()>=5)
					{
						drawSpoiler=YSTRUE;
						spoil1.Set(atof(args[1]),atof(args[2]));
						spoil2.Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 15: // "BRAKE___",
					if(args.GetN()>=5)
					{
						drawBrake=YSTRUE;
						brake1.Set(atof(args[1]),atof(args[2]));
						brake2.Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 16: // "AMMO____",
					if(args.GetN()>=4)
					{
						drawAmmo=YSTRUE;
						ammoPos.Set(atof(args[1]),atof(args[2]));
						ammoRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 17: // "CROSHAIR",
					if(args.GetN()>=4)
					{
						drawCrossHair=YSTRUE;
						crossHairPos.Set(atof(args[1]),atof(args[2]));
						crossHairRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 18: // "TRNCOORD",
					if(args.GetN()>=4)
					{
						drawTurnCoordinator=YSTRUE;
						turnCoordinatorPos.Set(atof(args[1]),atof(args[2]));
						turnCoordinatorRad=atof(args[3]);
						res=YSOK;
					}
					break;

				case 19: // "SPEEDVFE",
					if(args.GetN()>=2)
					{
						res=FsGetSpeed(speedVfe,args[1]);
					}
					break;
				case 20: // "SPEEDVNO",
					if(args.GetN()>=2)
					{
						res=FsGetSpeed(speedVno,args[1]);
					}
					break;
				case 21: // "SPEEDVNE",
					if(args.GetN()>=2)
					{
						res=FsGetSpeed(speedVne,args[1]);
					}
					break;
				case 22: // "SPEEDRNG",
					if(args.GetN()>=2)
					{
						res=FsGetSpeed(speedIndicatorRange,args[1]);
					}
					break;
				case 23: // "SPEEDMCH"
					if(args.GetN()>=2)
					{
						res=FsGetBool(speedShowMachNumber,args[1]);
					}
					break;
				case 24: // "SPEEDARC"
					if(args.GetN()>=2)
					{
						res=FsGetAngle(speedArc,args[1]);
					}
					break;
				case 25: // "ADF_____"
					if(args.GetN()>=4)
					{
						drawAdf=YSTRUE;
						adfPos.Set(atof(args[1]),atof(args[2]));
						adfRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 26: // "HSI_____",
					if(args.GetN()>=4)
					{
						drawHsi=YSTRUE;
						hsiPos.Set(atof(args[1]),atof(args[2]));
						hsiRad=atof(args[3]);
						res=YSOK;
					}
					break;
				case 27: // "ELEVTRIM",
					if(args.GetN()>=5)
					{
						drawTrim=YSTRUE;
						trim1.Set(atof(args[1]),atof(args[2]));
						trim2.Set(atof(args[3]),atof(args[4]));
						res=YSOK;
					}
					break;
				case 28: // "NUMTACHO",
					if(2<=args.GetN())
					{
						tachometerArray.Resize(atoi(args[1]));
						if(0<tachometerArray.GetN())
						{
							drawRPM=YSTRUE;
						}
						res=YSOK;
					}
					break;
				case 29: // "TACHOPOS", # x y rad
					if(5<=args.GetN())
					{
						const int tachId=atoi(args[1]);
						if(YSTRUE==tachometerArray.IsInRange(tachId))
						{
							auto &tach=tachometerArray[tachId];
							tach.pos.Set(atof(args[2]),atof(args[3]));
							tach.rad=atof(args[4]);
							res=YSOK;
						}
					}
					break;
				case 30: // "TACHORPM", # rpmMin rpmMax
					if(4<=args.GetN())
					{
						const int tachId=atoi(args[1]);
						if(YSTRUE==tachometerArray.IsInRange(tachId))
						{
							auto &tach=tachometerArray[tachId];
							tach.rpmMin=atoi(args[2]);
							tach.rpmMax=atoi(args[3]);
							res=YSOK;
						}
					}
					break;
				case 31: // "TACHOGRN", # grnRpmMin grnRpmMax
					if(4<=args.GetN())
					{
						const int tachId=atoi(args[1]);
						if(YSTRUE==tachometerArray.IsInRange(tachId))
						{
							auto &tach=tachometerArray[tachId];
							tach.greenArcMin=atoi(args[2]);
							tach.greenArcMax=atoi(args[3]);
							res=YSOK;
						}
					}
					break;
				case 32: // "TACHORED", # redRpm
					if(3<=args.GetN())
					{
						const int tachId=atoi(args[1]);
						if(YSTRUE==tachometerArray.IsInRange(tachId))
						{
							auto &tach=tachometerArray[tachId];
							tach.redLine=atoi(args[2]);
							res=YSOK;
						}
					}
					break;
				case 33: // "TACHOTCK", # tick tick tick tick ...
					if(2<=args.GetN())
					{
						const int tachId=atoi(args[1]);
						if(YSTRUE==tachometerArray.IsInRange(tachId))
						{
							auto &tach=tachometerArray[tachId];
							tach.tickLocationArray.Set(args.GetN()-2,NULL);
							for(int idx=2; idx<args.GetN(); ++idx)
							{
								tach.tickLocationArray[idx-2]=atoi(args[idx]);
							}
							res=YSOK;
						}
					}
					break;
				case 34: // "TACHOANG",
					if(4<=args.GetN())
					{
						const int tachId=atoi(args[1]);
						if(YSTRUE==tachometerArray.IsInRange(tachId))
						{
							auto &tach=tachometerArray[tachId];
							if(YSOK==FsGetAngle(tach.startAngle,args[2]) &&
							   YSOK==FsGetAngle(tach.endAngle,args[3]))
							{
								res=YSOK;
							}
						}
					}
					break;
				}

				if(res!=YSOK)
				{
					fsStderr.Printf("Error: %s",(const char *)str);
				}
			}
		}
		fclose(fp);
		return YSOK;
	}
	return YSERR;
}

////////////////////////////////////////////////////////////

void FsInstrumentPanel::BeginDraw3d(const YsVec3 &viewPos,const YsVec3 &localViewPos,const class FsAirplaneProperty &prop)
{
	pointVtxBuf.CleanUp();
	lineVtxBuf.CleanUp();
	triVtxBuf.CleanUp();
	pointColBuf.CleanUp();
	lineColBuf.CleanUp();
	triColBuf.CleanUp();

	ovLineVtxBuf.CleanUp();
	ovTriVtxBuf.CleanUp();
	ovLineColBuf.CleanUp();
	ovTriColBuf.CleanUp();

	viewPosCache=viewPos;
	localViewPosCache=localViewPos;
	airPropCache=&prop;
}

void FsInstrumentPanel::Draw3d(const FsAirplaneProperty &prop,const FsCockpitIndicationSet &cockpitIndicationSet)
{
	const FsInstrumentIndication &inst=cockpitIndicationSet.inst;

	DrawBackground();

	if(drawSpeed==YSTRUE)
	{
		double Vfe,Vno,Vne,VindicatorRange,Varc;

		Vfe=(speedVfe>=-0.5 ? speedVfe : inst.Vfe);
		Vno=(speedVno>=-0.5 ? speedVno : inst.Vno);
		Vne=(speedVne>=-0.5 ? speedVne : inst.Vne);
		VindicatorRange=(speedIndicatorRange>=-0.5 ? speedIndicatorRange : inst.VindicatorRange);
		Varc=(speedArc>=-YsTolerance ? speedArc : YsPi*1.8);

		DrawSpeed3d(inst.airSpeed,Vfe,Vno,Vne,VindicatorRange,inst.mach,Varc);
	}

	if(drawAlt==YSTRUE)
	{
		DrawAltitude3d(inst.altitude);
	}

	if(drawAtt==YSTRUE)
	{
		DrawAttitude3d(inst.pitch,inst.bank);
	}

	if(drawHdg==YSTRUE)
	{
		DrawHeading3d(inst.heading,YSTRUE,inst.headingBug,inst.headingBugSelected);
	}

	if(drawClimb==YSTRUE)
	{
		DrawVSI3d(inst.verticalSpeed);
	}

	if(drawAmmo==YSTRUE)
	{
		const int gun=prop.GetNumWeapon(FSWEAPON_GUN)+prop.GetNumPilotControlledTurretBullet();
		const int maxGun=prop.GetMaxNumWeapon(FSWEAPON_GUN)+prop.GetMaxNumPilotControlledTurretBullet();
		FSWEAPONTYPE wpnChoice=prop.GetWeaponOfChoice();
		const int wpnChoiceLeft=prop.GetNumWeapon(wpnChoice);

		DrawAmmo3d(gun,maxGun,wpnChoice,wpnChoiceLeft);
	}

	if(drawEngine==YSTRUE)
	{
		DrawEngine3d(inst.engineOutput[0],inst.afterBurner[0]);
	}

	if(YSTRUE==drawRPM)
	{
		for(auto tachIdx : tachometerArray.AllIndex())
		{
			auto &tach=tachometerArray[tachIdx];
			DrawTach3d((int)tachIdx,prop.GetRealPropRPM(tachIdx));
		}
	}

	if(drawTurnCoordinator==YSTRUE)
	{
		DrawTurnCoordinator3d(inst.sideSlip,-inst.turnRate);
	}

	if(drawG==YSTRUE)
	{
		DrawG3d(inst.gForce);
	}

	if(drawFuel==YSTRUE)
	{
		DrawSimpleRectInstrument3d(fuel1,fuel2,"FUEL","EMPTY","FULL",inst.fuelRemain[0]/inst.fuelCapacity[0]);
	}

	if(drawGear==YSTRUE)
	{
		DrawGear3d(inst.gearPos);
	}

	if(drawFlap==YSTRUE)
	{
		DrawSimpleRectInstrument3d(flap1,flap2,"FLAPS","DOWN","UP",1.0-inst.flaps);
	}

	if(YSTRUE==drawTrim)
	{
		DrawSimpleRectInstrument3d(trim1,trim2,"TRIM","DOWN","UP",(1.0+inst.elevatorTrim)/2.0);
	}

	if(drawSpoiler==YSTRUE)
	{
		DrawSimpleRectInstrument3d(spoil1,spoil2,"SPOIL","","EXTND",inst.spoiler);
	}

	if(drawBrake==YSTRUE)
	{
		DrawBrake3d(inst.brake);
	}
}

YSBOOL FsInstrumentPanel::HasHSI(void) const
{
	return drawHsi;
}

////////////////////////////////////////////////////////////

/* static */ void FsInstrumentPanel::AddNeedleVertexArray(
   	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
		YsColor lineCol,YsColor fillCol,
	    const YsMatrix4x4 &tfm,
	    const double &wid,const double &lng,const double &ang,const double &tailLng)
{
	const float c=(float)cos(ang);
	const float s=(float)sin(ang);
	const float w=(float)wid;
	const float t=(float)tailLng;

	const float needle[5*3]=
	{
		-(-c*w+s*t),
		 (-s*w-c*t),
		 0.0f,

		-(c*w+s*t),
		 (s*w-c*t),
		 0.0f,

		-(c*w-s*(float)(lng-wid*2.0)),
		 (s*w+c*(float)(lng-wid*2.0)),
		 0.0f,

		-(c*0.0f-s*(float)lng),
		 (s*0.0f+c*(float)lng),
		 0.0f,

		-(-c*w-s*(float)(lng-wid*2.0)),
		 (-s*w+c*(float)(lng-wid*2.0)),
		 0.0f,
	};
	YsVec3 plg[6];
	for(int i=0; i<5; ++i)
	{
		plg[i]=tfm*YsVec3(needle[i*3],needle[i*3+1],needle[i*3+2]);
	}
	plg[5]=plg[0];

	for(int i=1; i<4; ++i)
	{
		triVtxBuf.Add(plg[0]);
		triVtxBuf.Add(plg[i]);
		triVtxBuf.Add(plg[i+1]);
		triColBuf.Add(fillCol);
		triColBuf.Add(fillCol);
		triColBuf.Add(fillCol);
	}
	for(int i=0; i<5; ++i)
	{
		lineVtxBuf.Add(plg[i]);
		lineVtxBuf.Add(plg[i+1]);
		lineColBuf.Add(lineCol);
		lineColBuf.Add(lineCol);
	}
}

/* static */ void FsInstrumentPanel::AddDirectionVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
	    const YsColor col,
	    const YsMatrix4x4 &tfm)
{
	const float lineVtx[72*2*3]=
	{
		  0.000000f,    0.950000f,0.0f,
		  0.000000f,    0.850000f,0.0f,
		  0.082798f,    0.946385f,0.0f,
		  0.078440f,    0.896575f,0.0f,
		  0.164966f,    0.935567f,0.0f,
		  0.147601f,    0.837087f,0.0f,
		  0.245878f,    0.917630f,0.0f,
		  0.232937f,    0.869333f,0.0f,
		  0.324919f,    0.892708f,0.0f,
		  0.290717f,    0.798739f,0.0f,
		  0.401487f,    0.860992f,0.0f,
		  0.380356f,    0.815677f,0.0f,
		  0.475000f,    0.822724f,0.0f,
		  0.425000f,    0.736122f,0.0f,
		  0.544898f,    0.778194f,0.0f,
		  0.516219f,    0.737237f,0.0f,
		  0.610648f,    0.727742f,0.0f,
		  0.546369f,    0.651138f,0.0f,
		  0.671751f,    0.671751f,0.0f,
		  0.636396f,    0.636396f,0.0f,
		  0.727742f,    0.610648f,0.0f,
		  0.651138f,    0.546369f,0.0f,
		  0.778194f,    0.544898f,0.0f,
		  0.737237f,    0.516219f,0.0f,
		  0.822724f,    0.475000f,0.0f,
		  0.736122f,    0.425000f,0.0f,
		  0.860992f,    0.401487f,0.0f,
		  0.815677f,    0.380356f,0.0f,
		  0.892708f,    0.324919f,0.0f,
		  0.798739f,    0.290717f,0.0f,
		  0.917630f,    0.245878f,0.0f,
		  0.869333f,    0.232937f,0.0f,
		  0.935567f,    0.164966f,0.0f,
		  0.837087f,    0.147601f,0.0f,
		  0.946385f,    0.082798f,0.0f,
		  0.896575f,    0.078440f,0.0f,
		  0.950000f,    0.000000f,0.0f,
		  0.850000f,    0.000000f,0.0f,
		  0.946385f,   -0.082798f,0.0f,
		  0.896575f,   -0.078440f,0.0f,
		  0.935567f,   -0.164966f,0.0f,
		  0.837087f,   -0.147601f,0.0f,
		  0.917630f,   -0.245878f,0.0f,
		  0.869333f,   -0.232937f,0.0f,
		  0.892708f,   -0.324919f,0.0f,
		  0.798739f,   -0.290717f,0.0f,
		  0.860992f,   -0.401487f,0.0f,
		  0.815677f,   -0.380356f,0.0f,
		  0.822724f,   -0.475000f,0.0f,
		  0.736122f,   -0.425000f,0.0f,
		  0.778194f,   -0.544898f,0.0f,
		  0.737237f,   -0.516219f,0.0f,
		  0.727742f,   -0.610648f,0.0f,
		  0.651138f,   -0.546369f,0.0f,
		  0.671751f,   -0.671751f,0.0f,
		  0.636396f,   -0.636396f,0.0f,
		  0.610648f,   -0.727742f,0.0f,
		  0.546369f,   -0.651138f,0.0f,
		  0.544898f,   -0.778194f,0.0f,
		  0.516219f,   -0.737237f,0.0f,
		  0.475000f,   -0.822724f,0.0f,
		  0.425000f,   -0.736122f,0.0f,
		  0.401487f,   -0.860992f,0.0f,
		  0.380356f,   -0.815677f,0.0f,
		  0.324919f,   -0.892708f,0.0f,
		  0.290717f,   -0.798739f,0.0f,
		  0.245878f,   -0.917630f,0.0f,
		  0.232937f,   -0.869333f,0.0f,
		  0.164966f,   -0.935567f,0.0f,
		  0.147601f,   -0.837087f,0.0f,
		  0.082798f,   -0.946385f,0.0f,
		  0.078440f,   -0.896575f,0.0f,
		  0.000000f,   -0.950000f,0.0f,
		  0.000000f,   -0.850000f,0.0f,
		 -0.082798f,   -0.946385f,0.0f,
		 -0.078440f,   -0.896575f,0.0f,
		 -0.164966f,   -0.935567f,0.0f,
		 -0.147601f,   -0.837087f,0.0f,
		 -0.245878f,   -0.917630f,0.0f,
		 -0.232937f,   -0.869333f,0.0f,
		 -0.324919f,   -0.892708f,0.0f,
		 -0.290717f,   -0.798739f,0.0f,
		 -0.401487f,   -0.860992f,0.0f,
		 -0.380356f,   -0.815677f,0.0f,
		 -0.475000f,   -0.822724f,0.0f,
		 -0.425000f,   -0.736122f,0.0f,
		 -0.544898f,   -0.778194f,0.0f,
		 -0.516219f,   -0.737237f,0.0f,
		 -0.610648f,   -0.727742f,0.0f,
		 -0.546369f,   -0.651138f,0.0f,
		 -0.671751f,   -0.671751f,0.0f,
		 -0.636396f,   -0.636396f,0.0f,
		 -0.727742f,   -0.610648f,0.0f,
		 -0.651138f,   -0.546369f,0.0f,
		 -0.778194f,   -0.544898f,0.0f,
		 -0.737237f,   -0.516219f,0.0f,
		 -0.822724f,   -0.475000f,0.0f,
		 -0.736122f,   -0.425000f,0.0f,
		 -0.860992f,   -0.401487f,0.0f,
		 -0.815677f,   -0.380356f,0.0f,
		 -0.892708f,   -0.324919f,0.0f,
		 -0.798739f,   -0.290717f,0.0f,
		 -0.917630f,   -0.245878f,0.0f,
		 -0.869333f,   -0.232937f,0.0f,
		 -0.935567f,   -0.164966f,0.0f,
		 -0.837087f,   -0.147601f,0.0f,
		 -0.946385f,   -0.082798f,0.0f,
		 -0.896575f,   -0.078440f,0.0f,
		 -0.950000f,   -0.000000f,0.0f,
		 -0.850000f,   -0.000000f,0.0f,
		 -0.946385f,    0.082798f,0.0f,
		 -0.896575f,    0.078440f,0.0f,
		 -0.935567f,    0.164966f,0.0f,
		 -0.837087f,    0.147601f,0.0f,
		 -0.917630f,    0.245878f,0.0f,
		 -0.869333f,    0.232937f,0.0f,
		 -0.892708f,    0.324919f,0.0f,
		 -0.798739f,    0.290717f,0.0f,
		 -0.860992f,    0.401487f,0.0f,
		 -0.815677f,    0.380356f,0.0f,
		 -0.822724f,    0.475000f,0.0f,
		 -0.736122f,    0.425000f,0.0f,
		 -0.778194f,    0.544898f,0.0f,
		 -0.737237f,    0.516219f,0.0f,
		 -0.727742f,    0.610648f,0.0f,
		 -0.651138f,    0.546369f,0.0f,
		 -0.671751f,    0.671751f,0.0f,
		 -0.636396f,    0.636396f,0.0f,
		 -0.610648f,    0.727742f,0.0f,
		 -0.546369f,    0.651138f,0.0f,
		 -0.544898f,    0.778194f,0.0f,
		 -0.516219f,    0.737237f,0.0f,
		 -0.475000f,    0.822724f,0.0f,
		 -0.425000f,    0.736122f,0.0f,
		 -0.401487f,    0.860992f,0.0f,
		 -0.380356f,    0.815677f,0.0f,
		 -0.324919f,    0.892708f,0.0f,
		 -0.290717f,    0.798739f,0.0f,
		 -0.245878f,    0.917630f,0.0f,
		 -0.232937f,    0.869333f,0.0f,
		 -0.164966f,    0.935567f,0.0f,
		 -0.147601f,    0.837087f,0.0f,
		 -0.082798f,    0.946385f,0.0f,
		 -0.078440f,    0.896575f,0.0f,
	};
	for(int i=0; i<144; i++)
	{
		lineVtxBuf.Add(tfm*YsVec3(lineVtx[i*3],lineVtx[i*3+1],lineVtx[i*3+2]));
		lineColBuf.Add(col);
	}

	for(int i=0; i<36; i+=3)
	{
		const char *str;
		char buf[16];
		switch(i)
		{
		default:
			sprintf(buf,"%d",i);
			str=buf;
			break;
		case 0:
			str="N";
			break;
		case 9:
			str="E";
			break;
		case 18:
			str="S";
			break;
		case 27:
			str="W";
			break;
		}

		const float a=(float)i*(float)YsPi*2.0f/36.0f;
		const float x=(float)sin(a)*0.74f;
		const float y=(float)cos(a)*0.74f;
		const float fontWid=0.10f;
		const float fontHei=0.12f;
		const float centerize=(float)strlen(str)*fontWid/2.0f;

		auto fontTfm=tfm;
		fontTfm.Translate(x,y,0.0f);
		fontTfm.RotateXY(-a);
		fontTfm.Translate(-centerize,0.0f,0.0f);
		fontTfm.Scale(fontWid,fontHei,1.0f);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,str,col);
	}
}

/* static */ void FsInstrumentPanel::AddReverseDirectionVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
		const YsColor col,
	    const YsMatrix4x4 &tfm)
{
	const float lineVtx[72*2*3]=  // 0.74
	{
		  0.000000f,    0.740000f,0.0f,
		  0.000000f,    0.840000f,0.0f,
		  0.064495f,    0.737184f,0.0f,
		  0.068853f,    0.786994f,0.0f,
		  0.128500f,    0.728758f,0.0f,
		  0.145864f,    0.827239f,0.0f,
		  0.191526f,    0.714785f,0.0f,
		  0.204467f,    0.763081f,0.0f,
		  0.253095f,    0.695373f,0.0f,
		  0.287297f,    0.789342f,0.0f,
		  0.312738f,    0.670668f,0.0f,
		  0.333868f,    0.715983f,0.0f,
		  0.370000f,    0.640859f,0.0f,
		  0.420000f,    0.727461f,0.0f,
		  0.424447f,    0.606173f,0.0f,
		  0.453125f,    0.647130f,0.0f,
		  0.475663f,    0.566873f,0.0f,
		  0.539942f,    0.643477f,0.0f,
		  0.523259f,    0.523259f,0.0f,
		  0.558614f,    0.558614f,0.0f,
		  0.566873f,    0.475663f,0.0f,
		  0.643477f,    0.539942f,0.0f,
		  0.606173f,    0.424447f,0.0f,
		  0.647130f,    0.453125f,0.0f,
		  0.640859f,    0.370000f,0.0f,
		  0.727461f,    0.420000f,0.0f,
		  0.670668f,    0.312738f,0.0f,
		  0.715983f,    0.333868f,0.0f,
		  0.695373f,    0.253095f,0.0f,
		  0.789342f,    0.287297f,0.0f,
		  0.714785f,    0.191526f,0.0f,
		  0.763081f,    0.204467f,0.0f,
		  0.728758f,    0.128500f,0.0f,
		  0.827239f,    0.145864f,0.0f,
		  0.737184f,    0.064495f,0.0f,
		  0.786994f,    0.068853f,0.0f,
		  0.740000f,    0.000000f,0.0f,
		  0.840000f,    0.000000f,0.0f,
		  0.737184f,   -0.064495f,0.0f,
		  0.786994f,   -0.068853f,0.0f,
		  0.728758f,   -0.128500f,0.0f,
		  0.827239f,   -0.145864f,0.0f,
		  0.714785f,   -0.191526f,0.0f,
		  0.763081f,   -0.204467f,0.0f,
		  0.695373f,   -0.253095f,0.0f,
		  0.789342f,   -0.287297f,0.0f,
		  0.670668f,   -0.312738f,0.0f,
		  0.715983f,   -0.333868f,0.0f,
		  0.640859f,   -0.370000f,0.0f,
		  0.727461f,   -0.420000f,0.0f,
		  0.606173f,   -0.424447f,0.0f,
		  0.647130f,   -0.453125f,0.0f,
		  0.566873f,   -0.475663f,0.0f,
		  0.643477f,   -0.539942f,0.0f,
		  0.523259f,   -0.523259f,0.0f,
		  0.558614f,   -0.558614f,0.0f,
		  0.475663f,   -0.566873f,0.0f,
		  0.539942f,   -0.643477f,0.0f,
		  0.424447f,   -0.606173f,0.0f,
		  0.453125f,   -0.647130f,0.0f,
		  0.370000f,   -0.640859f,0.0f,
		  0.420000f,   -0.727461f,0.0f,
		  0.312738f,   -0.670668f,0.0f,
		  0.333868f,   -0.715983f,0.0f,
		  0.253095f,   -0.695373f,0.0f,
		  0.287297f,   -0.789342f,0.0f,
		  0.191526f,   -0.714785f,0.0f,
		  0.204467f,   -0.763081f,0.0f,
		  0.128500f,   -0.728758f,0.0f,
		  0.145864f,   -0.827239f,0.0f,
		  0.064495f,   -0.737184f,0.0f,
		  0.068853f,   -0.786994f,0.0f,
		  0.000000f,   -0.740000f,0.0f,
		  0.000000f,   -0.840000f,0.0f,
		 -0.064495f,   -0.737184f,0.0f,
		 -0.068853f,   -0.786994f,0.0f,
		 -0.128500f,   -0.728758f,0.0f,
		 -0.145864f,   -0.827239f,0.0f,
		 -0.191526f,   -0.714785f,0.0f,
		 -0.204467f,   -0.763081f,0.0f,
		 -0.253095f,   -0.695373f,0.0f,
		 -0.287297f,   -0.789342f,0.0f,
		 -0.312738f,   -0.670668f,0.0f,
		 -0.333868f,   -0.715983f,0.0f,
		 -0.370000f,   -0.640859f,0.0f,
		 -0.420000f,   -0.727461f,0.0f,
		 -0.424447f,   -0.606173f,0.0f,
		 -0.453125f,   -0.647130f,0.0f,
		 -0.475663f,   -0.566873f,0.0f,
		 -0.539942f,   -0.643477f,0.0f,
		 -0.523259f,   -0.523259f,0.0f,
		 -0.558614f,   -0.558614f,0.0f,
		 -0.566873f,   -0.475663f,0.0f,
		 -0.643477f,   -0.539942f,0.0f,
		 -0.606173f,   -0.424447f,0.0f,
		 -0.647130f,   -0.453125f,0.0f,
		 -0.640859f,   -0.370000f,0.0f,
		 -0.727461f,   -0.420000f,0.0f,
		 -0.670668f,   -0.312738f,0.0f,
		 -0.715983f,   -0.333868f,0.0f,
		 -0.695373f,   -0.253095f,0.0f,
		 -0.789342f,   -0.287297f,0.0f,
		 -0.714785f,   -0.191526f,0.0f,
		 -0.763081f,   -0.204467f,0.0f,
		 -0.728758f,   -0.128500f,0.0f,
		 -0.827239f,   -0.145864f,0.0f,
		 -0.737184f,   -0.064495f,0.0f,
		 -0.786994f,   -0.068853f,0.0f,
		 -0.740000f,   -0.000000f,0.0f,
		 -0.840000f,   -0.000000f,0.0f,
		 -0.737184f,    0.064495f,0.0f,
		 -0.786994f,    0.068853f,0.0f,
		 -0.728758f,    0.128500f,0.0f,
		 -0.827239f,    0.145864f,0.0f,
		 -0.714785f,    0.191526f,0.0f,
		 -0.763081f,    0.204467f,0.0f,
		 -0.695373f,    0.253095f,0.0f,
		 -0.789342f,    0.287297f,0.0f,
		 -0.670668f,    0.312738f,0.0f,
		 -0.715983f,    0.333868f,0.0f,
		 -0.640859f,    0.370000f,0.0f,
		 -0.727461f,    0.420000f,0.0f,
		 -0.606173f,    0.424447f,0.0f,
		 -0.647130f,    0.453125f,0.0f,
		 -0.566873f,    0.475663f,0.0f,
		 -0.643477f,    0.539942f,0.0f,
		 -0.523259f,    0.523259f,0.0f,
		 -0.558614f,    0.558614f,0.0f,
		 -0.475663f,    0.566873f,0.0f,
		 -0.539942f,    0.643477f,0.0f,
		 -0.424447f,    0.606173f,0.0f,
		 -0.453125f,    0.647130f,0.0f,
		 -0.370000f,    0.640859f,0.0f,
		 -0.420000f,    0.727461f,0.0f,
		 -0.312738f,    0.670668f,0.0f,
		 -0.333868f,    0.715983f,0.0f,
		 -0.253095f,    0.695373f,0.0f,
		 -0.287297f,    0.789342f,0.0f,
		 -0.191526f,    0.714785f,0.0f,
		 -0.204467f,    0.763081f,0.0f,
		 -0.128500f,    0.728758f,0.0f,
		 -0.145864f,    0.827239f,0.0f,
		 -0.064495f,    0.737184f,0.0f,
		 -0.068853f,    0.786994f,0.0f,
	};
	for(int i=0; i<144; i++)
	{
		lineVtxBuf.Add(tfm*YsVec3(lineVtx[i*3],lineVtx[i*3+1],lineVtx[i*3+2]));
		lineColBuf.Add(col);
	}

	for(int i=0; i<36; i+=3)
	{
		const char *str;
		char buf[16];
		switch(i)
		{
		default:
			sprintf(buf,"%d",i);
			str=buf;
			break;
		case 0:
			str="N";
			break;
		case 9:
			str="E";
			break;
		case 18:
			str="S";
			break;
		case 27:
			str="W";
			break;
		}

		const float a=(float)i*(float)YsPi*2.0f/36.0f;
		const float x=(float)sin(a)*0.87f;
		const float y=(float)cos(a)*0.87f;
		const float fontWid=0.09f;
		const float fontHei=0.11f;
		const float centerize=(float)strlen(str)*fontWid/2.0f;

		auto fontTfm=tfm;
		fontTfm.Translate(x,y,0.0f);
		fontTfm.RotateXY(-a);
		fontTfm.Translate(-centerize,0.0f,0.0f);
		fontTfm.Scale(fontWid,fontHei,1.0f);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,str,col);
	}
}

/* static */ void FsInstrumentPanel::AddHsiVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,
	    YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
	    const double hdg,
	    YsColor col,
	    const YsMatrix4x4 &tfmIn,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selectedObs,
	    YSBOOL showHdgBug,const double &hdgBug,YSBOOL selectedBug,YSBOOL vorInop)
{
	const double fontWid=0.1;
	const double fontHei=0.12;
	if(YSTRUE==isDme && YSTRUE!=vorInop)
	{
		char str[256];
		sprintf(str,"%.1lf",dme/1852.0);

		auto fontTfm=tfmIn;
		fontTfm.Translate(-0.6,0.1,0.0f);
		fontTfm.Scale(fontWid,fontHei,1.0);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,str,col);
	}

	{
		auto fontTfm=tfmIn;
		fontTfm.Translate(-0.6,-0.1-fontHei,0);
		fontTfm.Scale(fontWid,fontHei,1);
		if(YSTRUE!=vorInop)
		{
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,vorId,col);
		}
		else
		{
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,"VOR INOP",col);
		}
	}



	{
		const float lineVtx[30*3]=
		{
			 0.95f,            0.0f,             0.0f,    1.00f,       0.0f,       0.0f,
			 0.0f,             0.95f,            0.0f,    0.0f,        1.0f,       0.0f,
			-0.95f,            0.0f,             0.0f,   -1.00f,       0.0f,       0.0f,
			 0.0f,            -0.95f,            0.0f,    0.0f,       -1.0f,       0.0f,
			 (float)YsCos45deg*0.95f,  (float)YsSin45deg*0.95f,  0.0f,    (float)YsCos45deg,  (float)YsSin45deg, 0.0f,
			 (float)YsCos135deg*0.95f, (float)YsSin135deg*0.95f, 0.0f,    (float)YsCos135deg, (float)YsSin135deg,0.0f,
			-(float)YsCos45deg*0.95f, -(float)YsSin45deg*0.95f,  0.0f,   -(float)YsCos45deg, -(float)YsSin45deg, 0.0f,
			-(float)YsCos135deg*0.95f,-(float)YsSin135deg*0.95f, 0.0f,   -(float)YsCos135deg,-(float)YsSin135deg,0.0f,

			 0.0f, 0.7f,0.0f,  0.0f, 1.0f,0.0f,
			 0.0f,-0.7f,0.0f,  0.0f,-1.0f,0.0f,
			 0.7f, 0.0f,0.0f,  1.0f, 0.0f,0.0f,
			-0.7f, 0.0f,0.0f,  -1.0f, 0.0f,0.0f,

			-0.2f,  0.1f,0.0f,   0.2f,  0.1f,0.0f,
			-0.15f,-0.07f,0.0f,  0.15f,-0.07f,0.0f,
			 0.0f,  0.25f,0.0f,  0.0f,- 0.07f,0.0f,
		};
		for(int i=0; i<30; ++i)
		{
			lineVtxBuf.Add(tfmIn*YsVec3(lineVtx[i*3],lineVtx[i*3+1],lineVtx[i*3+2]));
			lineColBuf.Add(col);
		}

		const float triVtx[18*3]=
		{
			-0.2f,0.09f,0.0f,
			 0.2f,0.09f,0.0f,
			 0.2f,0.11f,0.0f,

			 0.2f,0.11f,0.0f,
			-0.2f,0.11f,0.0f,
			-0.2f,0.09f,0.0f,

			-0.15f,-0.06f,0.0f,
			 0.15f,-0.06f,0.0f,
			 0.15f,-0.08f,0.0f,

			 0.15f,-0.08f,0.0f,
			-0.15f,-0.08f,0.0f,
			-0.15f,-0.06f,0.0f,

			 0.01f, 0.25f,0.0f,
			 0.01f,-0.07f,0.0f,
			-0.01f,-0.07f,0.0f,

			-0.01f,-0.07f,0.0f,
			-0.01f, 0.25f,0.0f,
			 0.01f, 0.25f,0.0f
		};
		for(int i=0; i<18; ++i)
		{
			triVtxBuf.Add(tfmIn*YsVec3(triVtx[i*3],triVtx[i*3+1],triVtx[i*3+2]));
			triColBuf.Add(col);
		}
	}



	const double inRad=0.7;
	const double dot5=0.6;
	const double localizerNeedleLength=0.35;
	const double gsLimit=0.4;
	const double glideSlopeNeedleSeparation=0.6;

	auto tfm=tfmIn;
	tfm.RotateXY(-hdg);
	AddDirectionVertexArray(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,col,tfm);

	if(YSTRUE==showHdgBug)
	{
		AddHeadingBugVertexArray(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    hdgBug,selectedBug,
			col,
		    tfm);  // Not tfmIn.  It should rotate with the direction.
	}


	{
		auto obsTfm=tfm;  // Not tfmIn.  It should rotate with the direction.
		obsTfm.RotateXY(obs);

		if(YSTRUE!=vorInop)
		{
			lineVtxBuf.Add(obsTfm*YsVec3(0.0f,-localizerNeedleLength,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(0.0f,-inRad,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(0.0f, inRad,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(0.0f, localizerNeedleLength,0.0f));
			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);
		}

		{
			float dx;
			if(YSTRUE!=isIls)
			{
				dx=dot5*(float)(lc/(YsPi/18.0));
			}
			else
			{
				dx=dot5*(float)(lc/YsDegToRad(2.5));
			}

			dx=YsBound<double>(dx,-inRad+0.02f,inRad-0.02);

			for(int i=1; i<=5; i++) // 4vtxs * 5

			{
				const float x=dot5*(float)i/5.0f;
				lineVtxBuf.Add(obsTfm*YsVec3( x, 0.03f,0.0f));
				lineVtxBuf.Add(obsTfm*YsVec3( x,-0.03f,0.0f));
				lineVtxBuf.Add(obsTfm*YsVec3( -x, 0.03f,0.0f));
				lineVtxBuf.Add(obsTfm*YsVec3( -x,-0.03f,0.0f));

				lineColBuf.Add(col);
				lineColBuf.Add(col);
				lineColBuf.Add(col);
				lineColBuf.Add(col);
			}

			lineVtxBuf.Add(obsTfm*YsVec3( -0.04f, 0.00f,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(  0.00f, 0.04f,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(  0.00f, 0.04f,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(  0.04f, 0.00f,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(  0.04f, 0.00f,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(  0.00f,-0.04f,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3(  0.00f,-0.04f,0.0f));
			lineVtxBuf.Add(obsTfm*YsVec3( -0.04f, 0.00f,0.0f));

			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);
			lineColBuf.Add(col);


			if(YSTRUE!=vorInop)
			{
				float y=inRad*inRad-dx*dx;
				if(y>0.0)
				{
					y=(float)sqrt(y);
					if(y>localizerNeedleLength)
					{
						y=localizerNeedleLength;
					}
					lineVtxBuf.Add(obsTfm*YsVec3(dx,-y,0.0f));
					lineVtxBuf.Add(obsTfm*YsVec3(dx, y,0.0f));
					lineColBuf.Add(col);
					lineColBuf.Add(col);
				}
			}

			triVtxBuf.Add(obsTfm*YsVec3( 0.0f ,inRad,0.0f));
			triVtxBuf.Add(obsTfm*YsVec3( 0.06f,inRad-0.12f,0.0f));
			triVtxBuf.Add(obsTfm*YsVec3(-0.06f,inRad-0.12f,0.0f));
			triColBuf.Add(col);
			triColBuf.Add(col);
			triColBuf.Add(col);

			if(YSTRUE!=vorInop)
			{
				float y=inRad*inRad-dx*dx;
				if(y>0.0)
				{
					y=sqrt(y);
					if(y>localizerNeedleLength)
					{
						y=localizerNeedleLength;
					}
					triVtxBuf.Add(obsTfm*YsVec3(dx-0.01f,-y,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(dx-0.01f, y,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(dx+0.01f, y,0.0f));

					triVtxBuf.Add(obsTfm*YsVec3(dx+0.01f, y,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(dx+0.01f,-y,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(dx-0.01f,-y,0.0f));

					triColBuf.Add(col);
					triColBuf.Add(col);
					triColBuf.Add(col);
					triColBuf.Add(col);
					triColBuf.Add(col);
					triColBuf.Add(col);
				}

				if(0<toFrom)
				{
					triVtxBuf.Add(obsTfm*YsVec3(0.2f,0.1f,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(0.4f,0.1f,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(0.3f,0.25f,0.0f));
					triColBuf.Add(col);
					triColBuf.Add(col);
					triColBuf.Add(col);
				}
				else if(0>toFrom)
				{
					triVtxBuf.Add(obsTfm*YsVec3(0.2f,-0.1f,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(0.4f,-0.1f,0.0f));
					triVtxBuf.Add(obsTfm*YsVec3(0.3f,-0.25f,0.0f));
					triColBuf.Add(col);
					triColBuf.Add(col);
					triColBuf.Add(col);
				}
			}
		}
	}

	if(YSTRUE!=vorInop && YSTRUE==isIls)  // Glide Slope
	{
		float dy;
		dy=(float)gsLimit*(float)gs/(float)YsDegToRad(0.7);
		dy=(float)YsBound<double>(dy,-gsLimit+0.02f,gsLimit-0.02);

		const double triVtx[6*3]=
		{
			-glideSlopeNeedleSeparation      ,dy-0.08 ,0.0 ,
			-glideSlopeNeedleSeparation      ,dy+0.08 ,0.0 ,
			-glideSlopeNeedleSeparation+0.08 ,dy,      0.0 ,

			 glideSlopeNeedleSeparation      ,dy-0.08 ,0.0 ,
			 glideSlopeNeedleSeparation      ,dy+0.08 ,0.0 ,
			 glideSlopeNeedleSeparation-0.08 ,dy,      0.0 
		};

		for(int i=0; i<6; ++i)
		{
			triVtxBuf.Add(tfmIn*YsVec3(triVtx[i*3],triVtx[i*3+1],triVtx[i*3+2]));
			triColBuf.Add(col);
		}
	}

	if(YSTRUE==selectedObs)
	{
		float knobVtx[13*3]=
		{
			-0.9f+0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
			-0.9f+0.1f*(float)YsCos30deg,-0.9f+0.1f*(float)YsSin30deg,0.0f,
			-0.9f+0.1f*(float)YsCos60deg,-0.9f+0.1f*(float)YsSin60deg,0.0f,
			-0.9f+0.1f*(float)0.0,       -0.9f+0.1f*(float)1.0,       0.0f,
			-0.9f-0.1f*(float)YsCos60deg,-0.9f+0.1f*(float)YsSin60deg,0.0f,
			-0.9f-0.1f*(float)YsCos30deg,-0.9f+0.1f*(float)YsSin30deg,0.0f,
			-0.9f-0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
			-0.9f-0.1f*(float)YsCos30deg,-0.9f-0.1f*(float)YsSin30deg,0.0f,
			-0.9f-0.1f*(float)YsCos60deg,-0.9f-0.1f*(float)YsSin60deg,0.0f,
			-0.9f+0.1f*(float)0.0,       -0.9f-0.1f*(float)1.0,       0.0f,
			-0.9f+0.1f*(float)YsCos60deg,-0.9f-0.1f*(float)YsSin60deg,0.0f,
			-0.9f+0.1f*(float)YsCos30deg,-0.9f-0.1f*(float)YsSin30deg,0.0f,

			-0.9f+0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
		};
		for(int i=0; i<12; ++i)
		{
			lineVtxBuf.Add(tfmIn*YsVec3(knobVtx[i*3  ],knobVtx[i*3+1],knobVtx[i*3+2]));
			lineVtxBuf.Add(tfmIn*YsVec3(knobVtx[i*3+3],knobVtx[i*3+4],knobVtx[i*3+5]));
			lineColBuf.Add(col);
			lineColBuf.Add(col);
		}
	}

	if(YSTRUE==selectedBug)
	{
		float knobVtx[13*3]=
		{
			 0.9f+0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
			 0.9f+0.1f*(float)YsCos30deg,-0.9f+0.1f*(float)YsSin30deg,0.0f,
			 0.9f+0.1f*(float)YsCos60deg,-0.9f+0.1f*(float)YsSin60deg,0.0f,
			 0.9f+0.1f*(float)0.0,       -0.9f+0.1f*(float)1.0,       0.0f,
			 0.9f-0.1f*(float)YsCos60deg,-0.9f+0.1f*(float)YsSin60deg,0.0f,
			 0.9f-0.1f*(float)YsCos30deg,-0.9f+0.1f*(float)YsSin30deg,0.0f,
			 0.9f-0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
			 0.9f-0.1f*(float)YsCos30deg,-0.9f-0.1f*(float)YsSin30deg,0.0f,
			 0.9f-0.1f*(float)YsCos60deg,-0.9f-0.1f*(float)YsSin60deg,0.0f,
			 0.9f+0.1f*(float)0.0,       -0.9f-0.1f*(float)1.0,       0.0f,
			 0.9f+0.1f*(float)YsCos60deg,-0.9f-0.1f*(float)YsSin60deg,0.0f,
			 0.9f+0.1f*(float)YsCos30deg,-0.9f-0.1f*(float)YsSin30deg,0.0f,

			 0.9f+0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
		};

		for(int i=0; i<12; ++i)
		{
			lineVtxBuf.Add(tfmIn*YsVec3(knobVtx[i*3  ],knobVtx[i*3+1],knobVtx[i*3+2]));
			lineVtxBuf.Add(tfmIn*YsVec3(knobVtx[i*3+3],knobVtx[i*3+4],knobVtx[i*3+5]));
			lineColBuf.Add(col);
			lineColBuf.Add(col);
		}
	}
}

/*static*/ void FsInstrumentPanel::AddHeadingBugVertexArray(
    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
    double hdgBug,YSBOOL selected,
	const YsColor col,
    const YsMatrix4x4 &instTfm)
{
	auto bugTfm=instTfm;
	bugTfm.RotateXY(hdgBug);

	const float hdgBugVtx[9*3]=
	{
		 0.0f, 0.96f,0.0f,
		 0.08f,0.96f,0.0f,
		 0.08f,0.88f,0.0f,
		 0.03f,0.88f,0.0f,
		 0.0f ,0.82f,0.0f,
		-0.03f,0.88f,0.0f,
		-0.08f,0.88f,0.0f,
		-0.08f,0.96f,0.0f,

		 0.0f, 0.96f,0.0f,
	};

	if(YSTRUE==selected)
	{
		YsVec3 v0=bugTfm*YsVec3(hdgBugVtx[0],hdgBugVtx[1],hdgBugVtx[2]);
		for(int i=1; i<8; ++i)
		{
			triVtxBuf.Add(v0);
			triVtxBuf.Add(bugTfm*YsVec3(hdgBugVtx[i*3  ],hdgBugVtx[i*3+1],hdgBugVtx[i*3+2]));
			triVtxBuf.Add(bugTfm*YsVec3(hdgBugVtx[i*3+3],hdgBugVtx[i*3+4],hdgBugVtx[i*3+5]));
			triColBuf.Add(col);
			triColBuf.Add(col);
			triColBuf.Add(col);
		}
	}
	else
	{
		for(int i=0; i<8; ++i)
		{
			lineVtxBuf.Add(bugTfm*YsVec3(hdgBugVtx[i*3  ],hdgBugVtx[i*3+1],hdgBugVtx[i*3+2]));
			lineVtxBuf.Add(bugTfm*YsVec3(hdgBugVtx[i*3+3],hdgBugVtx[i*3+4],hdgBugVtx[i*3+5]));
			lineColBuf.Add(col);
			lineColBuf.Add(col);
		}
	}
}


void FsInstrumentPanel::DrawBackground(void)
{
	if(panelPlg.GetN()>=3)
	{
		for(int i=1; i<panelPlg.GetN()-1; i++)
		{
			triVtxBuf.Add(YsVec3(panelPlg[0],0));
			triVtxBuf.Add(YsVec3(panelPlg[i],0));
			triVtxBuf.Add(YsVec3(panelPlg[i+1],0));
			triColBuf.Add(YsBlack());
			triColBuf.Add(YsBlack());
			triColBuf.Add(YsBlack());
		}
		for(int i=0; i<panelPlg.GetN(); ++i)
		{
			lineVtxBuf.Add(YsVec3(panelPlg[i],0));
			lineVtxBuf.Add(YsVec3(panelPlg.GetCyclic(i+1),0));
			lineColBuf.Add(YsWhite());
			lineColBuf.Add(YsWhite());
		}
	}
}


void FsInstrumentPanel::DrawHeading3d(const double &hdg,YSBOOL showHdgBug,const double &hdgBug,YSBOOL selected)
{
	YsMatrix4x4 mat;
	mat.Translate(YsVec3(hdgPos,0));
	mat.Scale(hdgRad,hdgRad,hdgRad);

	AddCircularInstrumentFrameVertexArray(hdgPos,hdgRad);

	const float lineVtx[16*3]=
	{
		 0.95f,            0.0f,             0.0f,    1.00f,       0.0f,       0.0f,
		 0.0f,             0.95f,            0.0f,    0.0f,        1.0f,       0.0f,
		-0.95f,            0.0f,             0.0f,   -1.00f,       0.0f,       0.0f,
		 0.0f,            -0.95f,            0.0f,    0.0f,       -1.0f,       0.0f,
		 (float)YsCos45deg*0.95f,  (float)YsSin45deg*0.95f,  0.0f,    (float)YsCos45deg,  (float)YsSin45deg, 0.0f,
		 (float)YsCos135deg*0.95f, (float)YsSin135deg*0.95f, 0.0f,    (float)YsCos135deg, (float)YsSin135deg,0.0f,
		-(float)YsCos45deg*0.95f, -(float)YsSin45deg*0.95f,  0.0f,   -(float)YsCos45deg, -(float)YsSin45deg, 0.0f,
		-(float)YsCos135deg*0.95f,-(float)YsSin135deg*0.95f, 0.0f,   -(float)YsCos135deg,-(float)YsSin135deg,0.0f,
	};
	for(int i=0; i<16; ++i)
	{
		lineVtxBuf.Add(mat*YsVec3(lineVtx[i*3],lineVtx[i*3+1],lineVtx[i*3+2]));
		lineColBuf.Add(YsWhite());
	}

	const float triVtx[57*3]=
	{
		 0.0f,      0.88f,    0.0f,
		-0.05f,     1.0f,     0.0f,
		 0.05f,     1.0f,     0.0f,
		-0.02f,     0.5f,     0.0f,
		 0.02f,     0.5f,     0.0f,
		 0.0f,      0.8f,     0.0f,
		-0.057053f, 0.336797f,0.0f,
		 0.000000f, 0.465627f,0.0f,
		 0.057053f, 0.336797f,0.0f,
		 0.057053f, 0.336797f,0.0f,
		-0.093862f, 0.149074f,0.0f,
		-0.057053f, 0.336797f,0.0f,
		 0.057053f, 0.336797f,0.0f,
		 0.093862f, 0.149074f,0.0f,
		-0.093862f, 0.149074f,0.0f,
		 0.093862f, 0.149074f,0.0f,
		 0.347840f, 0.016564f,0.0f,
		 0.082819f,-0.057053f,0.0f,
		 0.347840f, 0.016564f,0.0f,
		 0.347840f,-0.086500f,0.0f,
		 0.082819f,-0.057053f,0.0f,
		-0.082819f,-0.057053f,0.0f,
		-0.347840f, 0.016564f,0.0f,
		-0.093862f, 0.149074f,0.0f,
		-0.082819f,-0.057053f,0.0f,
		-0.347840f,-0.086500f,0.0f,
		-0.347840f, 0.016564f,0.0f,
		 0.082819f,-0.057053f,0.0f,
		-0.082819f,-0.171159f,0.0f,
		-0.082819f,-0.057053f,0.0f,
		 0.082819f,-0.057053f,0.0f,
		 0.082819f,-0.171159f,0.0f,
		-0.082819f,-0.171159f,0.0f,
		 0.093862f, 0.149074f,0.0f,
		 0.082819f,-0.057053f,0.0f,
		-0.093862f, 0.149074f,0.0f,
		 0.082819f,-0.057053f,0.0f,
		-0.082819f,-0.057053f,0.0f,
		-0.093862f, 0.149074f,0.0f,
		 0.082819f,-0.171159f,0.0f,
		-0.060734f,-0.270542f,0.0f,
		-0.082819f,-0.171159f,0.0f,
		 0.082819f,-0.171159f,0.0f,
		 0.060734f,-0.270542f,0.0f,
		-0.060734f,-0.270542f,0.0f,
		 0.082819f,-0.171159f,0.0f,
		 0.189563f,-0.241095f,0.0f,
		 0.060734f,-0.270542f,0.0f,
		 0.189563f,-0.241095f,0.0f,
		 0.189563f,-0.325755f,0.0f,
		 0.060734f,-0.270542f,0.0f,
		-0.060734f,-0.270542f,0.0f,
		-0.189563f,-0.241095f,0.0f,
		-0.082819f,-0.171159f,0.0f,
		-0.060734f,-0.270542f,0.0f,
		-0.189563f,-0.325755f,0.0f,
		-0.189563f,-0.241095f,0.0f
	};
	for(int i=0; i<57; ++i)
	{
		triVtxBuf.Add(mat*YsVec3(triVtx[i*3],triVtx[i*3+1],triVtx[i*3+2]));
		triColBuf.Add(YsWhite());
	}

	if(YSTRUE==selected)
	{
		auto knobTfm=mat;
		knobTfm.Translate(-0.9,-0.9,0.0);
		knobTfm.Scale(0.1,0.1,1.0);

		const float loopVtx[13*3]=
		{
			 1.0f,                0.0f,               0.0f,
			 (float)YsCos30deg, (float)YsSin30deg,0.0f,
			 (float)YsCos60deg, (float)YsSin60deg,0.0f,
			 0.0,                 1.0,                0.0f,
			-(float)YsCos60deg, (float)YsSin60deg,0.0f,
			-(float)YsCos30deg, (float)YsSin30deg,0.0f,
			-1.0,                 0.0,                0.0f,
			-(float)YsCos30deg,-(float)YsSin30deg,0.0f,
			-(float)YsCos60deg,-(float)YsSin60deg,0.0f,
			 0.0,                -1.0,                0.0f,
			 (float)YsCos60deg,-(float)YsSin60deg,0.0f,
			 (float)YsCos30deg,-(float)YsSin30deg,0.0f,

			 1.0f,                0.0f,               0.0f,
		};
		for(int i=0; i<12; ++i)
		{
			lineVtxBuf.Add(mat*YsVec3(loopVtx[i*3  ],loopVtx[i*3+1],loopVtx[i*3+2]));
			lineVtxBuf.Add(mat*YsVec3(loopVtx[i*3+3],loopVtx[i*3+4],loopVtx[i*3+5]));
			lineColBuf.Add(YsWhite());
			lineColBuf.Add(YsWhite());
		}
	}

	auto rotMat=mat;
	rotMat.RotateXY(hdg);
	AddDirectionVertexArray(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,YsWhite(),rotMat);

	if(YSTRUE==showHdgBug)
	{
		AddHeadingBugVertexArray(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    -hdgBug,selected,
			YsWhite(),
		    rotMat);
	}
}

/* static */ void FsInstrumentPanel::AddAdfVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,
	    YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
	    YsColor col,YsColor fillCol,
	    const YsMatrix4x4 &tfmIn,
	    YSRESULT inRange,
	    const char adfId[],YSBOOL tuned,const double &obs,const double &bearing,YSBOOL selected,YSBOOL inop)
{
	{
		auto obsTfm=tfmIn;
		obsTfm.RotateXY(-obs);
		AddReverseDirectionVertexArray(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,col,obsTfm);
	}

	const float fontWid=0.1f;
	const float fontHei=0.12f;
	const float inRad=0.7f;

	{
		auto fontTfm=tfmIn;
		fontTfm.Translate(-0.6f,-0.1f-fontHei,0.0f);
		fontTfm.Scale(fontWid,fontHei,1.0f);
		if(YSTRUE!=inop)
		{
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,adfId,col);
		}
		else
		{
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,"INOP",col);
		}
	}

	if(YSTRUE==selected)
	{
		float knobVtx[13*3]=
		{
			-0.9f+0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
			-0.9f+0.1f*(float)YsCos30deg,-0.9f+0.1f*(float)YsSin30deg,0.0f,
			-0.9f+0.1f*(float)YsCos60deg,-0.9f+0.1f*(float)YsSin60deg,0.0f,
			-0.9f+0.1f*(float)0.0,       -0.9f+0.1f*(float)1.0,       0.0f,
			-0.9f-0.1f*(float)YsCos60deg,-0.9f+0.1f*(float)YsSin60deg,0.0f,
			-0.9f-0.1f*(float)YsCos30deg,-0.9f+0.1f*(float)YsSin30deg,0.0f,
			-0.9f-0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
			-0.9f-0.1f*(float)YsCos30deg,-0.9f-0.1f*(float)YsSin30deg,0.0f,
			-0.9f-0.1f*(float)YsCos60deg,-0.9f-0.1f*(float)YsSin60deg,0.0f,
			-0.9f+0.1f*(float)0.0,       -0.9f-0.1f*(float)1.0,       0.0f,
			-0.9f+0.1f*(float)YsCos60deg,-0.9f-0.1f*(float)YsSin60deg,0.0f,
			-0.9f+0.1f*(float)YsCos30deg,-0.9f-0.1f*(float)YsSin30deg,0.0f,

			-0.9f+0.1f*(float)1.0,       -0.9f+0.1f*(float)0.0,       0.0f,
		};
		for(int i=0; i<12; ++i)
		{
			lineVtxBuf.Add(tfmIn*YsVec3(knobVtx[i*3  ],knobVtx[i*3+1],knobVtx[i*3+2]));
			lineVtxBuf.Add(tfmIn*YsVec3(knobVtx[i*3+3],knobVtx[i*3+4],knobVtx[i*3+5]));
			lineColBuf.Add(col);
			lineColBuf.Add(col);
		}
	}
	if(YSTRUE!=inop)
	{
		AddNeedleVertexArray(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,col,fillCol,tfmIn,0.05,inRad,-bearing,inRad);
	}
}

/* static */ void FsInstrumentPanel::AddNavVertexArray(
	    YsGLVertexBuffer &lineVtxBuf,YsGLColorBuffer &lineColBuf,
	    YsGLVertexBuffer &triVtxBuf,YsGLColorBuffer &triColBuf,
	    YsColor lineCol,YsColor fillCol,
	    const YsMatrix4x4 &tfmIn,

	    int navId,
	    YSRESULT inRange,
	    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
	    YSBOOL isDme,const double &dme,
	    YSBOOL selected,YSBOOL inop)
{
	{
		auto obsTfm=tfmIn;
		obsTfm.RotateXY(-obs);
		AddReverseDirectionVertexArray(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,lineCol,obsTfm);
	}



	const float fontWid=0.1f;
	const float fontHei=0.12f;
	if(YSTRUE==isDme && YSTRUE!=inop)
	{
		char str[256];
		sprintf(str,"%.1lf",dme/1852.0);

		auto fontTfm=tfmIn;
		fontTfm.Translate(-0.6f,0.1f,0.0f);
		fontTfm.Scale(fontWid,fontHei,1.0f);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,str,lineCol);
	}



	{
		auto fontTfm=tfmIn;
		fontTfm.Translate(-0.6f,-0.1f-fontHei,0.0f);
		fontTfm.Scale(fontWid,fontHei,1.0f);
		if(YSTRUE!=inop)
		{
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,vorId,lineCol);
		}
		else
		{
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontTfm,"INOP",lineCol);
		}
	}



	const float inRad=0.65f;
	{
		float triVtx[9]=
		{
			 0.0f ,0.75f     ,0.0f,
			-0.05f,0.75f-0.1f,0.0f,
			 0.05f,0.75f-0.1f,0.0f
		};
		for(int i=0; i<3; ++i)
		{
			triVtxBuf.Add(tfmIn*YsVec3(triVtx[i*3],triVtx[i*3+1],triVtx[i*3+2]));
			triColBuf.Add(lineCol);
		}
	}

	if(YSTRUE==selected)
	{
		auto tfm=tfmIn;
		tfm.Translate(-0.9f,-0.9f,0.0f);
		tfm.Scale(0.1f,0.1f,1.0f);

		const float knobVtx[13*3]=
		{
			 (float)1.0       , (float)0.0       ,0.0f,
			 (float)YsCos30deg, (float)YsSin30deg,0.0f,
			 (float)YsCos60deg, (float)YsSin60deg,0.0f,
			 (float)0.0       , (float)1.0       ,0.0f,
			-(float)YsCos60deg, (float)YsSin60deg,0.0f,
			-(float)YsCos30deg, (float)YsSin30deg,0.0f,
			-(float)1.0       , (float)0.0       ,0.0f,
			-(float)YsCos30deg,-(float)YsSin30deg,0.0f,
			-(float)YsCos60deg,-(float)YsSin60deg,0.0f,
			 (float)0.0       ,-(float)1.0       ,0.0f,
			 (float)YsCos60deg,-(float)YsSin60deg,0.0f,
			 (float)YsCos30deg,-(float)YsSin30deg,0.0f,

			 (float)1.0       , (float)0.0       ,0.0f,
		};
		for(int i=0; i<12; ++i)
		{
			lineVtxBuf.Add(tfm*YsVec3(knobVtx[i*3  ],knobVtx[i*3+1],knobVtx[i*3+2]));
			lineVtxBuf.Add(tfm*YsVec3(knobVtx[i*3+3],knobVtx[i*3+4],knobVtx[i*3+5]));
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);
		}
	}

	switch(navId)
	{
	case 0:
		{
			const float dot5=0.6f;
			float dx,dy;
			if(YSTRUE!=isIls)
			{
				dx=dot5*(float)lc/(float)(YsPi/18.0);
				dy=0.0f;
			}
			else
			{
				dx=dot5*(float)lc/(float)YsDegToRad(2.5);
				dy=dot5*(float)gs/(float)YsDegToRad(0.7);
			}

			dx=YsBound <float> (dx,-inRad+0.02f,inRad-0.02f);
			dy=YsBound <float> (dy,-inRad+0.02f,inRad-0.02f);

			for(int i=1; i<=5; i++)   // 8*5 vtxs
			{
				const float x=dot5*(float)i/5.0f;
				lineVtxBuf.Add(tfmIn*YsVec3( x, 0.03f, 0.0f));
				lineVtxBuf.Add(tfmIn*YsVec3( x,-0.03f, 0.0f));
				lineVtxBuf.Add(tfmIn*YsVec3(-x, 0.03f, 0.0f));
				lineVtxBuf.Add(tfmIn*YsVec3(-x,-0.03f, 0.0f));
				lineVtxBuf.Add(tfmIn*YsVec3( 0.03f, x, 0.0f));
				lineVtxBuf.Add(tfmIn*YsVec3(-0.03f, x, 0.0f));
				lineVtxBuf.Add(tfmIn*YsVec3( 0.03f,-x, 0.0f));
				lineVtxBuf.Add(tfmIn*YsVec3(-0.03f,-x, 0.0f));
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
			}

			lineVtxBuf.Add(tfmIn*YsVec3(-0.04f, 0.00f, 0.0f));
			lineVtxBuf.Add(tfmIn*YsVec3( 0.00f, 0.04f, 0.0f));
			lineVtxBuf.Add(tfmIn*YsVec3( 0.00f, 0.04f, 0.0f));
			lineVtxBuf.Add(tfmIn*YsVec3( 0.04f, 0.00f, 0.0f));
			lineVtxBuf.Add(tfmIn*YsVec3( 0.04f, 0.00f, 0.0f));
			lineVtxBuf.Add(tfmIn*YsVec3( 0.00f,-0.04f, 0.0f));
			lineVtxBuf.Add(tfmIn*YsVec3( 0.00f,-0.04f, 0.0f));
			lineVtxBuf.Add(tfmIn*YsVec3(-0.04f, 0.00f, 0.0f));
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);
			lineColBuf.Add(lineCol);

			if(YSTRUE!=inop)
			{
				float y;

				y=inRad*inRad-dx*dx;  // 2vtxs
				if(y>0.0)
				{
					y=sqrt(y);
					lineVtxBuf.Add(tfmIn*YsVec3( dx,-y, 0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3( dx, y, 0.0f));
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
				}

				y=inRad*inRad-dy*dy;  // 2vtxs
				if(y>0.0)
				{
					y=sqrt(y);
					lineVtxBuf.Add(tfmIn*YsVec3(-y, dy, 0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3( y, dy, 0.0f));
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
				}
			}

			if(YSTRUE!=inop)
			{
				float y=inRad*inRad-dx*dx;
				if(y>0.0)
				{
					y=sqrt(y);
					lineVtxBuf.Add(tfmIn*YsVec3(dx-0.01f,-y,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3(dx-0.01f, y,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3(dx+0.01f, y,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3(dx+0.01f, y,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3(dx+0.01f,-y,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3(dx-0.01f,-y,0.0f));
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
				}

				y=inRad*inRad-dy*dy;
				if(y>0.0)
				{
					y=sqrt(y);
					lineVtxBuf.Add(tfmIn*YsVec3(-y,dy-0.01f,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3( y,dy-0.01f,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3( y,dy+0.01f,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3( y,dy+0.01f,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3(-y,dy+0.01f,0.0f));
					lineVtxBuf.Add(tfmIn*YsVec3(-y,dy-0.01f,0.0f));
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);
				}

				if(0<toFrom)
				{
					triVtxBuf.Add(tfmIn*YsVec3(0.2f,0.3f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.4f,0.3f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.3f,0.4f,0.0f));
					triColBuf.Add(lineCol);
					triColBuf.Add(lineCol);
					triColBuf.Add(lineCol);
				}
				else if(0>toFrom)
				{
					triVtxBuf.Add(tfmIn*YsVec3(0.2f,0.2f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.4f,0.2f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.3f,0.1f,0.0f));
					triColBuf.Add(lineCol);
					triColBuf.Add(lineCol);
					triColBuf.Add(lineCol);
				}
			}
		}
		break;
	case 1:
		{
			const float cx=0.0f,cy=1.0f;
			const float dotRad=1.4f;

			const float windowRectVtx[5*3]=
			{
				-0.04f, 0.00f,0.0f,
				 0.00f, 0.04f,0.0f,
				 0.04f, 0.00f,0.0f,
				 0.00f,-0.04f,0.0f,

				-0.04f, 0.00f,0.0f,
			}; 
			for(int i=0; i<4; ++i)
			{
				lineVtxBuf.Add(tfmIn*YsVec3(windowRectVtx[i*3  ],windowRectVtx[i*3+1],windowRectVtx[i*3+2]));
				lineVtxBuf.Add(tfmIn*YsVec3(windowRectVtx[i*3+3],windowRectVtx[i*3+4],windowRectVtx[i*3+5]));
				lineColBuf.Add(lineCol);
				lineColBuf.Add(lineCol);
			}


			for(int i=0; i<=5; i++)  // 6*4 tri *3=72 vtxs
			{
				const float a=(float)i*(float)YsDegToRad(4.0);  // 2deg separation
				const float s=sin(a);
				const float c=cos(a);
				const float x=dotRad*s;
				const float y=dotRad*c;

				triVtxBuf.Add(tfmIn*YsVec3(cx+x-0.03f,cy-y      ,0.0f));
				triVtxBuf.Add(tfmIn*YsVec3(cx+x      ,cy-y+0.03f,0.0f));
				triVtxBuf.Add(tfmIn*YsVec3(cx+x+0.03f,cy-y      ,0.0f));
				triVtxBuf.Add(tfmIn*YsVec3(cx+x+0.03f,cy-y      ,0.0f));
				triVtxBuf.Add(tfmIn*YsVec3(cx+x      ,cy-y-0.03f,0.0f));
				triVtxBuf.Add(tfmIn*YsVec3(cx+x-0.03f,cy-y      ,0.0f));
				triColBuf.Add(fillCol);
				triColBuf.Add(fillCol);
				triColBuf.Add(fillCol);
				triColBuf.Add(fillCol);
				triColBuf.Add(fillCol);
				triColBuf.Add(fillCol);

				if(0!=i)
				{
					triVtxBuf.Add(tfmIn*YsVec3(cx-x-0.03f,cy-y,      0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(cx-x      ,cy-y+0.03f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(cx-x+0.03f,cy-y,      0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(cx-x+0.03f,cy-y,      0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(cx-x      ,cy-y-0.03f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(cx-x-0.03f,cy-y,      0.0f));
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
				}
			}

			if(YSTRUE!=inop)
			{
				double deflection;  // 5dots=20deg
				if(YSTRUE!=isIls)
				{
					deflection=YsDegToRad(20.0)*lc/(YsPi/18.0);;
				}
				else
				{
					deflection=YsDegToRad(20.0)*lc/YsDegToRad(2.5);
				}
				deflection=YsBound(deflection,-YsDegToRad(24.0),YsDegToRad(24.0));

				YsVec2 o,v;
				o.Set(0.0,1.0);
				v.Set(sin(deflection),-cos(deflection));

				// Want to clip line o+tv by a circle x^2+y^2=inRad^2
				// (o+tv)^2=inRad^2
				// (v^2)t^2+2t(ov)+o^2-inRad^2=0
				double a,b,c;
				a=v.GetSquareLength();
				b=o*v;
				c=o.GetSquareLength()-inRad*inRad;

				double det=b*b-a*c;
				if(det>0.0)
				{
					double t1,t2;
					t1=(-b+sqrt(det))/a;
					t2=(-b-sqrt(det))/a;

					YsVec2 _p1,_p2,_n;
					_p1=o+t1*v;
					_p2=o+t2*v;

					_n.Set(-v.y(),v.x());
					_n*=0.01;

					YsVec3 p1,p2,n;
					p1.Set(_p1.x(),_p1.y(),0.0);
					p2.Set(_p2.x(),_p2.y(),0.0);
					n.Set(_n.x(),_n.y(),0.0);

					lineVtxBuf.Add(tfmIn*p1);
					lineVtxBuf.Add(tfmIn*p2);
					lineColBuf.Add(lineCol);
					lineColBuf.Add(lineCol);

					triVtxBuf.Add(tfmIn*(p1+n));
					triVtxBuf.Add(tfmIn*(p2+n));
					triVtxBuf.Add(tfmIn*(p2-n));
					triVtxBuf.Add(tfmIn*(p2-n));
					triVtxBuf.Add(tfmIn*(p1-n));
					triVtxBuf.Add(tfmIn*(p1+n));
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
				}

				if(0<toFrom)
				{
					triVtxBuf.Add(tfmIn*YsVec3(0.2f,0.3f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.4f,0.3f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.3f,0.4f,0.0f));
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
				}
				else if(0>toFrom)
				{
					triVtxBuf.Add(tfmIn*YsVec3(0.2f,0.2f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.4f,0.2f,0.0f));
					triVtxBuf.Add(tfmIn*YsVec3(0.3f,0.1f,0.0f));
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
					triColBuf.Add(fillCol);
				}
			}
		}
		break;
	}
}

void FsInstrumentPanel::AddCircularInstrumentFrameVertexArray(const YsVec2 &cen,const double &rad)
{
	const float unitCircleVtx[37*3]=
	{
		 (float)1.0,        (float)0.0,       0.0f,
		 (float)YsCos10deg, (float)YsSin10deg,0.0f,
		 (float)YsCos20deg, (float)YsSin20deg,0.0f,
		 (float)YsCos30deg, (float)YsSin30deg,0.0f,
		 (float)YsCos40deg, (float)YsSin40deg,0.0f,
		 (float)YsCos50deg, (float)YsSin50deg,0.0f,
		 (float)YsCos60deg, (float)YsSin60deg,0.0f,
		 (float)YsCos70deg, (float)YsSin70deg,0.0f,
		 (float)YsCos80deg, (float)YsSin80deg,0.0f,
		 (float)0.0,        (float)1.0,       0.0f,
		-(float)YsCos80deg, (float)YsSin80deg,0.0f,
		-(float)YsCos70deg, (float)YsSin70deg,0.0f,
		-(float)YsCos60deg, (float)YsSin60deg,0.0f,
		-(float)YsCos50deg, (float)YsSin50deg,0.0f,
		-(float)YsCos40deg, (float)YsSin40deg,0.0f,
		-(float)YsCos30deg, (float)YsSin30deg,0.0f,
		-(float)YsCos20deg, (float)YsSin20deg,0.0f,
		-(float)YsCos10deg, (float)YsSin10deg,0.0f,
		-(float)1.0,        (float)0.0,       0.0f,
		-(float)YsCos10deg,-(float)YsSin10deg,0.0f,
		-(float)YsCos20deg,-(float)YsSin20deg,0.0f,
		-(float)YsCos30deg,-(float)YsSin30deg,0.0f,
		-(float)YsCos40deg,-(float)YsSin40deg,0.0f,
		-(float)YsCos50deg,-(float)YsSin50deg,0.0f,
		-(float)YsCos60deg,-(float)YsSin60deg,0.0f,
		-(float)YsCos70deg,-(float)YsSin70deg,0.0f,
		-(float)YsCos80deg,-(float)YsSin80deg,0.0f,
		 (float)0.0,       -(float)1.0,       0.0f,
		 (float)YsCos80deg,-(float)YsSin80deg,0.0f,
		 (float)YsCos70deg,-(float)YsSin70deg,0.0f,
		 (float)YsCos60deg,-(float)YsSin60deg,0.0f,
		 (float)YsCos50deg,-(float)YsSin50deg,0.0f,
		 (float)YsCos40deg,-(float)YsSin40deg,0.0f,
		 (float)YsCos30deg,-(float)YsSin30deg,0.0f,
		 (float)YsCos20deg,-(float)YsSin20deg,0.0f,
		 (float)YsCos10deg,-(float)YsSin10deg,0.0f,

		 (float)1.0,        (float)0.0,       0.0f,
	};

	float x0=unitCircleVtx[0]*rad+cen.x();
	float y0=unitCircleVtx[1]*rad+cen.y();
	float z0=unitCircleVtx[2];
	for(int i=1; i<36; ++i)
	{
		triVtxBuf.Add<float>(x0,y0,z0);
		triVtxBuf.Add<float>(unitCircleVtx[i*3  ]*rad+cen.x(),unitCircleVtx[i*3+1]*rad+cen.y(),unitCircleVtx[i*3+2]);
		triVtxBuf.Add<float>(unitCircleVtx[i*3+3]*rad+cen.x(),unitCircleVtx[i*3+4]*rad+cen.y(),unitCircleVtx[i*3+5]);
		triColBuf.Add(YsBlack());
		triColBuf.Add(YsBlack());
		triColBuf.Add(YsBlack());
	}

	for(int i=0; i<36; ++i)
	{
		lineVtxBuf.Add<float>(unitCircleVtx[i*3  ]*rad+cen.x(),unitCircleVtx[i*3+1]*rad+cen.y(),unitCircleVtx[i*3+2]);
		lineVtxBuf.Add<float>(unitCircleVtx[i*3+3]*rad+cen.x(),unitCircleVtx[i*3+4]*rad+cen.y(),unitCircleVtx[i*3+5]);
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
	}
}
void FsInstrumentPanel::AddRectangularInstrumentFrameVertexArray(const YsVec2 &g1,const YsVec2 &g2)
{
	const double lineVtx[5*3]=
	{
		g1.x(),g1.y(),0.0,
		g2.x(),g1.y(),0.0,
		g2.x(),g2.y(),0.0,
		g1.x(),g2.y(),0.0,

		g1.x(),g1.y(),0.0f,
	};
	const double triVtx[6*3]=
	{
		g1.x(),g1.y(),0.0,
		g2.x(),g1.y(),0.0,
		g2.x(),g2.y(),0.0,

		g2.x(),g2.y(),0.0,
		g1.x(),g2.y(),0.0,
		g1.x(),g1.y(),0.0,
	};

	for(int i=0; i<6; ++i)
	{
		triVtxBuf.Add(triVtx[i*3  ],triVtx[i*3+1],triVtx[i*3+2]);
		triColBuf.Add(YsBlack());
	}
	for(int i=0; i<4; ++i)
	{
		lineVtxBuf.Add(lineVtx[i*3  ],lineVtx[i*3+1],lineVtx[i*3+2]);
		lineVtxBuf.Add(lineVtx[i*3+3],lineVtx[i*3+4],lineVtx[i*3+5]);
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
	}

}

void FsInstrumentPanel::DrawNav3d(
    int navId,
    YSRESULT inRange,
    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
    YSBOOL isDme,const double &dme,
    YSBOOL selected,YSBOOL inop)
{
	YsVec2 pos;
	double rad;
	if(navId==0 && drawIls==YSTRUE)
	{
		pos=ilsPos;
		rad=ilsRad;
	}
	else if(navId==1 && drawVor==YSTRUE)
	{
		pos=vorPos;
		rad=vorRad;
	}
	else
	{
		return;
	}
	AddCircularInstrumentFrameVertexArray(pos,rad);
	YsMatrix4x4 tfm;
	tfm.Translate(YsVec3(pos,0));
	tfm.Scale(rad,rad,rad);
	FsInstrumentPanel::AddNavVertexArray(
	    lineVtxBuf,lineColBuf,
	    triVtxBuf,triColBuf,
	    YsWhite(),YsGrayScale(0.3),
	    tfm,

	    navId,
	    inRange,
	    vorId,tuned,isIls,toFrom,obs,lc,gs,
	    isDme,dme,
	    selected,inop);
}

void FsInstrumentPanel::DrawAdf3d(
    YSRESULT inRange,
    const char adfId[],YSBOOL tuned,const double &obs,const double &bearing,YSBOOL selected,YSBOOL inop)
{
	if(drawAdf==YSTRUE)
	{
		AddCircularInstrumentFrameVertexArray(adfPos,adfRad);

		YsMatrix4x4 tfm;
		tfm.Translate(YsVec3(adfPos,0));
		tfm.Scale(adfRad,adfRad,adfRad);
		FsInstrumentPanel::AddAdfVertexArray(
		    lineVtxBuf,lineColBuf,
		    triVtxBuf,triColBuf,
		    YsWhite(),YsGrayScale(0.3),
		    tfm,
		    inRange,
		    adfId,tuned,obs,bearing,selected,inop);
	}
}

void FsInstrumentPanel::DrawHsi3d(
    const double &hdg,
    YSRESULT inRange,
    const char vorId[],YSBOOL tuned,YSBOOL isIls,int toFrom,const double &obs,const double &lc,const double &gs,
    YSBOOL isDme,const double &dme,
    YSBOOL selectedObs,
    YSBOOL showHdgBug,const double &hdgBug,YSBOOL selectedBug,YSBOOL vorInop)
{
	if(drawHsi==YSTRUE)
	{
		AddCircularInstrumentFrameVertexArray(hsiPos,hsiRad);

		YsMatrix4x4 tfm;
		tfm.Translate(YsVec3(hsiPos,0));
		tfm.Scale(hsiRad,hsiRad,hsiRad);
		AddHsiVertexArray(
		    lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,
		    -hdg,
		    YsWhite(),
		    tfm,
		    inRange,
		    vorId,tuned,isIls,toFrom,obs,lc,gs,
		    isDme,dme,
		    selectedObs,
		    showHdgBug,-hdgBug,selectedBug,vorInop);
	}
}

void FsInstrumentPanel::DrawTach3d(int tachIdx,const double currentRPM)
{
	if(YSTRUE==tachometerArray.IsInRange(tachIdx))
	{
		auto &tach=tachometerArray[tachIdx];

		AddCircularInstrumentFrameVertexArray(tach.pos,tach.rad);

		YsMatrix4x4 mat;
		mat.Translate(YsVec3(tach.pos,0));
		mat.Scale(tach.rad,tach.rad,tach.rad);

		// 0-> YsPi*3/4   100->-YsPi/3/4

		const float outRad=1.0f;
		const float inRad=0.8f;

		const float fontWid=inRad/8.0f;
		const float fontHei=inRad/6.0f;



		float prevX1,prevY1,prevX2,prevY2;
		YSBOOL first=YSTRUE;
		for(int rpm=tach.greenArcMin; rpm<=tach.greenArcMax; rpm+=100)
		{
			const float t=(float)(rpm-tach.rpmMin)/(float)(tach.rpmMax-tach.rpmMin);
			const float a=(float)tach.startAngle*(float)(1.0-t)+(float)tach.endAngle*(float)t;

			const float c=(float)cos(a);
			const float s=(float)sin(a);

			const float x1=s*outRad;
			const float y1=c*outRad;
			const float x2=s*inRad;
			const float y2=c*inRad;

			if(YSTRUE!=first)
			{
				triVtxBuf.Add(mat*YsVec3(prevX1,prevY1,0.0f));
				triVtxBuf.Add(mat*YsVec3(prevX2,prevY2,0.0f));
				triVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));

				triVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
				triVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
				triVtxBuf.Add(mat*YsVec3(prevX1,prevY1,0.0f));

				triColBuf.Add(YsGreen());
				triColBuf.Add(YsGreen());
				triColBuf.Add(YsGreen());
				triColBuf.Add(YsGreen());
				triColBuf.Add(YsGreen());
				triColBuf.Add(YsGreen());
			}

			first=YSFALSE;
			prevX1=x1;
			prevY1=y1;
			prevX2=x2;
			prevY2=y2;
		}



		for(int rpm=tach.rpmMin; rpm<=tach.rpmMax; rpm+=100)
		{
			YSBOOL tick=tach.tickLocationArray.IsIncluded(rpm);

			const float t=(float)(rpm-tach.rpmMin)/(float)(tach.rpmMax-tach.rpmMin);
			const float a=(float)tach.startAngle*(float)(1.0-t)+(float)tach.endAngle*(float)t;

			const float c=(float)cos(a);
			const float s=(float)sin(a);

			const float x1=s*outRad;
			const float y1=c*outRad;
			const float x2=s*(YSTRUE!=tick ? inRad : inRad*0.9f);
			const float y2=c*(YSTRUE!=tick ? inRad : inRad*0.9f);

			lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
			lineColBuf.Add(YsWhite());
			lineColBuf.Add(YsWhite());
		}



		{
			const float t=(float)(tach.redLine-tach.rpmMin)/(float)(tach.rpmMax-tach.rpmMin);
			const float a=(float)tach.startAngle*(float)(1.0-t)+(float)tach.endAngle*(float)t;

			const float c1=(float)cos(a);
			const float s1=(float)sin(a);
			const float c2=(float)cos(a+YsPi/120.0);
			const float s2=(float)sin(a+YsPi/120.0);

			const float x1=s1*outRad;
			const float y1=c1*outRad;
			const float x2=s1*inRad*0.9f;
			const float y2=c1*inRad*0.9f;
			const float x3=s2*outRad;
			const float y3=c2*outRad;
			const float x4=s2*inRad*0.9f;
			const float y4=c2*inRad*0.9f;

			triVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			triVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
			triVtxBuf.Add(mat*YsVec3(x4,y4,0.0f));
			triVtxBuf.Add(mat*YsVec3(x4,y4,0.0f));
			triVtxBuf.Add(mat*YsVec3(x3,y3,0.0f));
			triVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			triColBuf.Add(YsRed());
			triColBuf.Add(YsRed());
			triColBuf.Add(YsRed());
			triColBuf.Add(YsRed());
			triColBuf.Add(YsRed());
			triColBuf.Add(YsRed());

			lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
			lineVtxBuf.Add(mat*YsVec3(x4,y4,0.0f));
			lineVtxBuf.Add(mat*YsVec3(x3,y3,0.0f));
			lineColBuf.Add(YsRed());
			lineColBuf.Add(YsRed());
			lineColBuf.Add(YsRed());
			lineColBuf.Add(YsRed());
		}



		for(auto rpm : tach.tickLocationArray)
		{
			const float t=(float)(rpm-tach.rpmMin)/(float)(tach.rpmMax-tach.rpmMin);
			const float a=(float)tach.startAngle*(float)(1.0-t)+(float)tach.endAngle*(float)t;

			const float c=(float)cos(a);
			const float s=(float)sin(a);

			auto fontMat=mat;

			const float x=s*inRad*0.75f;
			const float y=c*inRad*0.75f;

			fontMat.Translate(x-fontWid*1.0f,y-fontHei/2.0f,0.0f);
			fontMat.Scale(fontWid,fontHei,1.0f);

			char num[4];
			if(10000<=rpm)
			{
				num[0]='0'+(rpm/10000)%10;
				num[1]='0'+(rpm/1000)%10;
				num[2]='0'+(rpm/100)%10;
				num[3]=0;
			}
			else if(1000<=rpm)
			{
				num[0]='0'+(rpm/1000)%10;
				num[1]='0'+(rpm/100)%10;
				num[2]=0;
			}
			else
			{
				num[0]='0'+(rpm/100)%10;
				num[1]=0;
			}
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,num,YsWhite());
		}

		{
			const float t=(float)(currentRPM-(double)tach.rpmMin)/(float)(tach.rpmMax-tach.rpmMin);
			const float a=(float)tach.startAngle*(float)(1.0-t)+(float)tach.endAngle*(float)t;
			AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),mat,0.02,inRad,a,0.02);
			// DrawNeedle3d(0.02,inRad,a,0.02);
		}
	}
}

void FsInstrumentPanel::DrawSpeed3d(
    const double &spd,const double &vfe,const double &vno,const double &vne,const double &vRng,const double &mach,
    const double &vArc)
{
	AddCircularInstrumentFrameVertexArray(speedPos,speedRad);

	YsMatrix4x4 mat;
	mat.Translate(YsVec3(speedPos,0));
	mat.Scale(speedRad,speedRad,speedRad);

	const float spdKt=(float)spd*3600.0f/1852.0f;
	const float vfeKt=(float)vfe*3600.0f/1852.0f;
	const float vnoKt=(float)vno*3600.0f/1852.0f;
	const float vneKt=(float)vne*3600.0f/1852.0f;
	const float vmaxKt=(float)vRng*3600.0f/1852.0f; // vneKt*1.1;   // vmaxKt->270degree

	const float outRad=0.95f;
	const float inRad=0.80f;
	const float needleRad=0.70f;

	char str[256];

	int i,numStep;

	if(vmaxKt<200.0)
	{
		numStep=20;
	}
	else if(vmaxKt<300.0)
	{
		numStep=40;
	}
	else if(vmaxKt<400.0)
	{
		numStep=60;
	}
	else if(vmaxKt<500.0)
	{
		numStep=80;
	}
	else
	{
		numStep=100;
	}


	const float fontWid=inRad/9.0f;
	const float fontHei=inRad/7.0f;


	if(speedShowMachNumber==YSTRUE)
	{
		const float rectVtx[5*3]=
		{
			-fontWid*3.2f,-fontHei*0.8f,0.0f,
			-fontWid*3.2f,-fontHei*2.2f,0.0f,
			 fontWid*3.2f,-fontHei*2.2f,0.0f,
			 fontWid*3.2f,-fontHei*0.8f,0.0f,

			-fontWid*3.2f,-fontHei*0.8f,0.0f,
		};
		for(int i=0; i<4; ++i)
		{
			lineVtxBuf.Add(mat*YsVec3(rectVtx[i*3  ],rectVtx[i*3+1],rectVtx[i*3+2]));
			lineVtxBuf.Add(mat*YsVec3(rectVtx[i*3+1],rectVtx[i*3+1],rectVtx[i*3+2]));
			lineColBuf.Add(YsWhite());
			lineColBuf.Add(YsWhite());
		}

		sprintf(str,"M %3.1lf",mach);

		auto fontMat=mat;
		fontMat.Translate(-fontWid*3.0f,-fontHei*2.0f,0.0f);
		fontMat.Scale(fontWid,fontHei,1.0f);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,YsWhite());
	}


	YsColor color;

	int arcSegment;
	if(vneKt/vmaxKt<=YsTolerance)
	{
		arcSegment=3;
		color=YsRed();
	}
	else if(vnoKt/vmaxKt<=YsTolerance)
	{
		arcSegment=2;
		color=YsYellow();
	}
	else if(vfeKt/vmaxKt<=YsTolerance)
	{
		arcSegment=1;
		color=YsGreen();
	}
	else
	{
		arcSegment=0;
		color=YsWhite();
	}

	float prevX1,prevY1,prevX2,prevY2;
	for(i=1; i<=48; i++)
	{
		float a=(float)i*(float)vArc/48.0f;

		const float x1=0.95*outRad*(float)sin(a);
		const float y1=0.95*outRad*(float)cos(a);
		const float x2=     outRad*(float)sin(a);
		const float y2=     outRad*(float)cos(a);

		if(1<i)
		{
			triVtxBuf.Add(mat*YsVec3(prevX2,prevY2,0.0f));
			triVtxBuf.Add(mat*YsVec3(prevX1,prevY1,0.0f));
			triVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			triVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			triVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
			triVtxBuf.Add(mat*YsVec3(prevX2,prevY2,0.0f));
			triColBuf.Add(color);
			triColBuf.Add(color);
			triColBuf.Add(color);
			triColBuf.Add(color);
			triColBuf.Add(color);
			triColBuf.Add(color);
		}

		if(arcSegment==0 && vfeKt/vmaxKt<=(double)i/48.0)
		{
			arcSegment=1;
			color=YsGreen();
		}
		if(arcSegment==1 && vnoKt/vmaxKt<=(double)i/48.0)
		{
			arcSegment=2;
			color=YsYellow();
		}
		if(arcSegment==2 && vneKt/vmaxKt<=(double)i/48.0)
		{
			arcSegment=3;
			color=YsRed();
		}

		prevX1=x1;
		prevY1=y1;
		prevX2=x2;
		prevY2=y2;
	}

	for(int i=0; i<(int)vmaxKt; i+=20)
	{
		float a=(float)vArc*(float)i/vmaxKt;
		float c=(float)cos(a);
		float s=(float)sin(a);

		if(i%numStep==0)
		{
			const float x1=(s*outRad);
			const float y1=(c*outRad);
			const float x2=(s*inRad*0.9f);
			const float y2=(c*inRad*0.9f);
			lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
			lineColBuf.Add(YsWhite());
			lineColBuf.Add(YsWhite());
		}
		else
		{
			const float x1=(s*outRad);
			const float y1=(c*outRad);
			const float x2=(s*inRad);
			const float y2=(c*inRad);
			lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
			lineColBuf.Add(YsWhite());
			lineColBuf.Add(YsWhite());
		}
	}


	for(i=0; i<(int)vmaxKt; i+=20)
	{
		if(i%numStep==0)
		{
			float a=(float)vArc*(float)i/vmaxKt;
			float c=(float)cos(a);
			float s=(float)sin(a);

			sprintf(str,"%d",i);

			const float x2=(s*inRad*0.8f);
			const float y2=(c*inRad*0.8f);

			float tfm[16];
			auto fontMat=mat;
			fontMat.Translate(x2-fontWid,y2,0.0f);
			fontMat.Scale(fontWid,fontHei,1.0);
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,YsWhite());
		}
	}

	float a=(float)(vArc*(spdKt/vmaxKt));
	AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),mat,0.05,needleRad,a,0.02);
	// DrawNeedle3d(0.05,needleRad,a,0.05);
}

void FsInstrumentPanel::DrawBrake3d(const double &brake)
{
	AddRectangularInstrumentFrameVertexArray(brake1,brake2);

	auto g1=brake1;
	auto g2=brake2;
	YsMatrix4x4 mat;
	mat.Translate(g1.x(),g1.y(),0);
	mat.Scale((g2.x()-g1.x()),(g2.y()-g1.y()),1);

	if(brake>0.5)
	{
		const double fontWid=0.2;
		const double fontHei=0.24;

		auto fontMat=mat;
		fontMat.Translate(0.5-fontWid*1.5f,0.5-fontHei/2.0,0.0);
		fontMat.Scale(fontWid,fontHei,1.0);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"BRK",YsWhite());
	}
}

void FsInstrumentPanel::DrawTurnCoordinator3d(const double &ssa,const double &turn)
{
	AddCircularInstrumentFrameVertexArray(turnCoordinatorPos,turnCoordinatorRad);

	YsMatrix4x4 mat;
	mat.Translate(YsVec3(turnCoordinatorPos,0));
	mat.Scale(turnCoordinatorRad,turnCoordinatorRad,turnCoordinatorRad);

	const float outRad=1.0f;
	const float inRad=outRad*0.8f;

	// Ball
	const float ballCirCx=0.0f;
	const float ballCirCy=outRad*0.6f;
	const float ballCirRad=outRad;


	float prevX1,prevY1,prevX2,prevY2;
	for(int a=-30; a<=30; a+=10)  // 7 steps total * 2 vtx each
	{
		const float c=(float)cos(YsDegToRad(a));
		const float s=(float)sin(YsDegToRad(a));
		float x1,y1,x2,y2;

		x1=ballCirCx+s*ballCirRad*0.9f;
		y1=ballCirCy-c*ballCirRad*0.9f;
		x2=ballCirCx+s*ballCirRad*1.1f;
		y2=ballCirCy-c*ballCirRad*1.1f;

		if(-30<a)
		{
			triVtxBuf.Add(mat*YsVec3(prevX2,prevY2,0.0f));
			triVtxBuf.Add(mat*YsVec3(prevX1,prevY1,0.0f));
			triVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			triVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
			triVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));
			triVtxBuf.Add(mat*YsVec3(prevX2,prevY2,0.0f));
			triColBuf.Add(YsWhite());
			triColBuf.Add(YsWhite());
			triColBuf.Add(YsWhite());
			triColBuf.Add(YsWhite());
			triColBuf.Add(YsWhite());
			triColBuf.Add(YsWhite());
		}

		prevX1=x1;
		prevY1=y1;
		prevX2=x2;
		prevY2=y2;
	}


	const float lineVtx[8*3]=
	{
		-inRad,                      0.0f,                       0.0f,
		-outRad,                     0.0f,                       0.0f,
		 inRad,                      0.0f,                       0.0f,
		 outRad,                     0.0f,                       0.0f,

		-inRad*(float)YsCos20deg, -inRad*(float)YsSin20deg,  0.0f,
		-outRad*(float)YsCos20deg,-outRad*(float)YsSin20deg, 0.0f,
		 inRad*(float)YsCos20deg, -inRad*(float)YsSin20deg,  0.0f,
		 outRad*(float)YsCos20deg,-outRad*(float)YsSin20deg, 0.0f
	};
	for(int i=0; i<8; ++i)
	{
		lineVtxBuf.Add(mat*YsVec3(lineVtx[i*3],lineVtx[i*3+1],lineVtx[i*3+2]));
		lineColBuf.Add(YsWhite());
	}




	float tilt,c,s,x,y;

	tilt=YsBound <float> ((float)ssa*5.0f,-(float)YsPi/9.0f,(float)YsPi/9.0f);
	c=(float)cos(tilt);
	s=(float)sin(tilt);
	x=ballCirCx+s*ballCirRad;
	y=ballCirCy-c*ballCirRad;

	const float scl=ballCirRad*0.1f;
	const float ballVtx[8*3]=
	{
		x+scl*1.0f,               y+scl*0.0f,               0.0f,
		x+scl*(float)YsCos45deg,y+scl*(float)YsSin45deg,0.0f,
		x+scl*0.0f,               y+scl*1.0f,               0.0f,
		x-scl*(float)YsCos45deg,y+scl*(float)YsSin45deg,0.0f,
		x-scl*1.0f,               y+scl*0.0f,               0.0f,
		x-scl*(float)YsCos45deg,y-scl*(float)YsSin45deg,0.0f,
		x+scl*0.0f,               y-scl*1.0f,               0.0f,
		x+scl*(float)YsCos45deg,y-scl*(float)YsSin45deg,0.0f,
	};
	YsVec3 vtx0=mat*YsVec3(ballVtx[0],ballVtx[1],ballVtx[2]);
	for(int i=1; i<7; ++i)
	{
		triVtxBuf.Add(vtx0);
		triVtxBuf.Add(mat*YsVec3(ballVtx[i*3  ],ballVtx[i*3+1],ballVtx[i*3+2]));
		triVtxBuf.Add(mat*YsVec3(ballVtx[i*3+3],ballVtx[i*3+4],ballVtx[i*3+5]));
		triColBuf.Add(YsBlack());
		triColBuf.Add(YsBlack());
		triColBuf.Add(YsBlack());
	}



	// 360deg/2min -> 20 deg tilt
	// 3deg/sec -> 20 deg tilt
	// 1deg/sec -> 20/3 deg tilt

	tilt=(float)turn*20.0f/3.0f;
	tilt=YsBound <float> (tilt,-(float)YsPi/3.0f,(float)YsPi/3.0f);

	auto tiltMat=mat;
	tiltMat.RotateXY(-tilt);

	const float miniatureAirplaneVtx[25*3]=
	{
		 0.0f,      0.0f,     0.0f,
		-0.700000f,-0.010000f,0.0f,
		-0.700000f, 0.010000f,0.0f,
		-0.117694f, 0.023411f,0.0f,
		-0.099776f, 0.066668f,0.0f,
		-0.066668f, 0.099776f,0.0f,
		-0.023411f, 0.117694f,0.0f,
		-0.010000f, 0.300000f,0.0f,
		 0.010000f, 0.300000f,0.0f,
		 0.023411f, 0.117694f,0.0f,
		 0.066668f, 0.099776f,0.0f,
		 0.099776f, 0.066668f,0.0f,
		 0.117694f, 0.023411f,0.0f,
		 0.700000f, 0.010000f,0.0f,
		 0.700000f,-0.010000f,0.0f,
		 0.117694f,-0.023411f,0.0f,
		 0.110866f,-0.045922f,0.0f,
		 0.084853f,-0.084853f,0.0f,
		 0.045922f,-0.110866f,0.0f,
		 0.000000f,-0.120000f,0.0f,
		-0.045922f,-0.110866f,0.0f,
		-0.084853f,-0.084853f,0.0f,
		-0.110866f,-0.045922f,0.0f,
		-0.117694f,-0.023411f,0.0f,
		-0.700000f,-0.010000f,0.0f
	};
	vtx0=tiltMat*YsVec3(miniatureAirplaneVtx[0],miniatureAirplaneVtx[1],miniatureAirplaneVtx[2]);
	for(int i=1; i<24; ++i)
	{
		triVtxBuf.Add(vtx0);
		triVtxBuf.Add(tiltMat*YsVec3(miniatureAirplaneVtx[i*3+0],miniatureAirplaneVtx[i*3+1],miniatureAirplaneVtx[i*3+2]));
		triVtxBuf.Add(tiltMat*YsVec3(miniatureAirplaneVtx[i*3+3],miniatureAirplaneVtx[i*3+4],miniatureAirplaneVtx[i*3+5]));
		triColBuf.Add(YsWhite());
		triColBuf.Add(YsWhite());
		triColBuf.Add(YsWhite());
	}
}

void FsInstrumentPanel::DrawAltitude3d(const double &y)
{
	AddCircularInstrumentFrameVertexArray(altPos,altRad);

	YsMatrix4x4 mat;
	mat.Translate(YsVec3(altPos,0));
	mat.Scale(altRad,altRad,altRad);


	const double feet=YsUnitConv::MtoFT(y);

	char str[256];


	const float outRad=1.0f;
	const float inRad=0.80f;

	const float fontWid=inRad/8.0f;
	const float fontHei=inRad/6.0f;

	double a,c,s;
	int n=0;

	//const float white[4]={1.0f,1.0f,1.0f,1.0f};
	//YsGLSLSet3DRendererUniformColorfv(renderer,white);

	for(int i=0; i<100; i+=2)  // 50 steps, 2 vtx each
	{
		a=(YsPi*2.0)*(double)i/100.0;

		c=cos(a);
		s=sin(a);

		const float x1=(float)(s*outRad);
		const float y1=(float)(c*outRad);
		float x2,y2;

		if(0==n)
		{
			x2=(float)(s*inRad);
			y2=(float)(c*inRad);
			n=5;
		}
		else
		{
			x2=(float)(s*inRad*1.1);
			y2=(float)(c*inRad*1.1);
		}
		lineVtxBuf.Add(mat*YsVec3(x1,y1,0));
		lineVtxBuf.Add(mat*YsVec3(x2,y2,0));
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
		n--;
	}



	n=0;
	for(int i=0; i<100; i+=2)
	{
		if(n==0)
		{
			a=(YsPi*2.0)*(double)i/100.0;

			c=cos(a);
			s=sin(a);

			const float x2=(float)(s*inRad*0.8-fontWid/2.0);
			const float y2=(float)(c*inRad*0.8-fontHei/2.0);
			sprintf(str,"%d",i/10);

			auto fontMat=mat;
			fontMat.Translate(x2,y2,0);
			fontMat.Scale(fontWid,fontHei,1);
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,YsWhite());
	
			n=5;
		}
		n--;
	}

	a=(YsPi*2.0)*(fmod(feet,1000.0)/1000.0);
	AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),mat,0.05,inRad,a,0.05);
	// DrawNeedle3d(0.05,inRad,a,0.05);

	a=(YsPi*2.0)*(fmod(feet/1000.0,10.0)/10.0);
	AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),mat,0.05,inRad*0.75,a,0.05);
	// DrawNeedle3d(0.05,inRad*0.75,a,0.05);

	a=(YsPi*2.0)*(fmod(feet/10000.0,10.0)/10.0);
	AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),mat,0.05,inRad*0.5,a,0.05);
	// DrawNeedle3d(0.05,inRad*0.5,a,0.05);
}

void FsInstrumentPanel::DrawVSI3d(const double &dy)
{
	AddCircularInstrumentFrameVertexArray(climbPos,climbRad);

	YsMatrix4x4 mat;
	mat.Translate(YsVec3(climbPos,0));
	mat.Scale(climbRad,climbRad,climbRad);



	// YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();

	float feetPerMin=(float)((dy*60.0)/0.30479);

	char str[256];

	const float outRad=1.0f;
	float inRad=0.90f;

	const float fontWid=inRad/8.0f;
	const float fontHei=inRad/6.0f;

	// 180deg -> 2000ft/min

	// const float white[4]={1.0f,1.0f,1.0f,1.0f};

	// YsGLSLSet3DRendererUniformColorfv(renderer,white);

	for(int i=0; i<1000; i+=100)
	{
		const float a=(float)i*(float)YsPi/2000.0f;
		const float s=(float)sin(a);
		const float c=(float)cos(a);

		const float x1=-c*outRad;
		const float y1= s*outRad;
		const float x2=-c*inRad;
		const float y2= s*inRad;

		lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
		lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));

		lineVtxBuf.Add(mat*YsVec3(x1,-y1,0.0f));
		lineVtxBuf.Add(mat*YsVec3(x2,-y2,0.0f));

		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
	}

	inRad=0.8f;

	for(int i=0; i<=2000; i+=500)
	{
		const int lmt=(i<1900 ? i : 1900);
		const float a=(float)lmt*(float)YsPi/2000.0f;
		const float s=(float)sin(a);
		const float c=(float)cos(a);

		const float x1=-c*outRad;
		const float y1= s*outRad;
		const float x2=-c*inRad;
		const float y2= s*inRad;

		lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0f));
		lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0f));

		lineVtxBuf.Add(mat*YsVec3(x1,-y1,0.0f));
		lineVtxBuf.Add(mat*YsVec3(x2,-y2,0.0f));

		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
	}



	for(int i=0; i<=2000; i+=500)
	{
		const float a=(float)i*(float)YsPi/2000.0f;
		const float s=(float)sin(a);
		const float c=(float)cos(a);
		const float x=-c*inRad*8.0f/9.0f;
		const float y= s*inRad*8.0f/9.0f;

		sprintf(str,"%d",i/100);


		auto fontMat=mat;
		fontMat.Translate(x-fontWid,y-fontHei/2.0f,0.0f);
		fontMat.Scale(fontWid,fontHei,1.0f);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,YsWhite());

		if(0!=i && 2000!=i)
		{
			sprintf(str,"%d",i/100);

			auto fontMat=mat;
			fontMat.Translate(x-fontWid,-y-fontHei/2.0f,0.0f);
			fontMat.Scale(fontWid,fontHei,1.0f);
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,YsWhite());
		}
	}



	feetPerMin=YsBound <float> (feetPerMin,-1900.0f,1900.0f);
	const double a=-YsPi/2.0+YsPi*feetPerMin/2000.0;
	AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),mat,0.05,inRad*0.8,a,0.05);
	// DrawNeedle3d(0.05,inRad*0.8,a,0.05);
}

void FsInstrumentPanel::DrawAttitude3d(const double &p,const double &b)
{
	AddCircularInstrumentFrameVertexArray(attPos,attRad);
	YsMatrix4x4 mat;
	mat.Translate(YsVec3(attPos,0));
	mat.Scale(attRad,attRad,attRad);


	const float inRad=0.8f,outRad=0.98f;
	YsColor lightCyan;
	lightCyan.SetFloatRGBA(0.0f,0.83f,0.83f,1.0f);
	YsColor lightBrown;
	lightBrown.SetFloatRGBA(0.51f,0.2f,0.2f,1.0f);
	YsColor brown;
	brown.SetFloatRGBA(0.43f,0.12f,0.12f,1.0f);



	// Pitch=40deg -> Displacement reaches inRad
	// k=1.0/40deg
	{
		const double k=1.0/(YsPi*4.0/18.0);

		auto bankMat=mat;
		bankMat.RotateXY(-b);

		const double y0=-k*p;
		if(y0<=-0.99999 || 0.99999<=y0) // All ground
		{
			YsColor col=(1.0<y0 ? lightBrown : lightCyan);

			double prevX,prevY;
			for(int i=0; i<=64; ++i)
			{
				const double a=(double)i*YsPi/32.0;
				const double c=cos(a);
				const double s=sin(a);
				const double x=inRad*c;
				const double y=inRad*s;
				if(0<i)
				{
					triVtxBuf.Add(bankMat*YsOrigin());
					triVtxBuf.Add(bankMat*YsVec3(prevX,prevY,0));
					triVtxBuf.Add(bankMat*YsVec3(x,y,0));
					triColBuf.Add(col);
					triColBuf.Add(col);
					triColBuf.Add(col);
				}
				prevX=x;
				prevY=y;
			}
		}
		else
		{
			const double a0=asin(y0);
			const double a1=YsPi-a0;
			const double a2=a0+YsPi*2.0;

			YsVec3 mid=bankMat*YsVec3(0,inRad*y0,0);
			double prevX,prevY;
			for(int i=0; i<=64; ++i)
			{
				const double t=(double)i/64.0;
				const double a=a0*(1.0-t)+a1*t;
				const double c=cos(a);
				const double s=sin(a);
				const double x=inRad*c;
				const double y=inRad*s;
				if(0<i)
				{
					triVtxBuf.Add(mid);
					triVtxBuf.Add(bankMat*YsVec3(prevX,prevY,0));
					triVtxBuf.Add(bankMat*YsVec3(x,y,0));
					triColBuf.Add(lightCyan);
					triColBuf.Add(lightCyan);
					triColBuf.Add(lightCyan);
				}
				prevX=x;
				prevY=y;
			}
			for(int i=0; i<=64; ++i)
			{
				const double t=(double)i/64.0;
				const double a=a1*(1.0-t)+a2*t;
				const double c=cos(a);
				const double s=sin(a);
				const double x=inRad*c;
				const double y=inRad*s;
				if(0<i)
				{
					triVtxBuf.Add(mid);
					triVtxBuf.Add(bankMat*YsVec3(prevX,prevY,0));
					triVtxBuf.Add(bankMat*YsVec3(x,y,0));
					triColBuf.Add(lightBrown);
					triColBuf.Add(lightBrown);
					triColBuf.Add(lightBrown);
				}
				prevX=x;
				prevY=y;
			}
		}

		for(int i=-40; i<=40; i+=5)
		{
			const double y=-k*(p-YsDegToRad((double)i));
			if(-0.99999<=y && y<=0.99999)
			{
				const double xMax=sqrt(1-y*y);
				double l;
				if(0==i)
				{
					l=0.6;
				}
				else if(0==i%10)
				{
					l=0.3;
				}
				else
				{
					l=0.2;
				}
				YsMakeSmaller(l,xMax);
				lineVtxBuf.Add(bankMat*YsVec3(-l*inRad,y*inRad,0));
				lineVtxBuf.Add(bankMat*YsVec3( l*inRad,y*inRad,0));
				lineColBuf.Add(YsWhite());
				lineColBuf.Add(YsWhite());
			}
		}

		double prevX1,prevY1,prevX2,prevY2;
		for(int i=0; i<=64; ++i)
		{
			const double a=i*YsPi/64.0f;
			const double s=sin(a);
			const double c=cos(a);

			const double x1=inRad*c;
			const double y1=inRad*s;
			const double x2=outRad*c;
			const double y2=outRad*s;

			if(0<i)
			{
				triVtxBuf.Add(bankMat*YsVec3(x1,y1,0));
				triVtxBuf.Add(bankMat*YsVec3(x2,y2,0));
				triVtxBuf.Add(bankMat*YsVec3(prevX2,prevY2,0));
				triVtxBuf.Add(bankMat*YsVec3(prevX2,prevY2,0));
				triVtxBuf.Add(bankMat*YsVec3(prevX1,prevY1,0));
				triVtxBuf.Add(bankMat*YsVec3(x1,y1,0));
				triColBuf.Add(YsCyan());
				triColBuf.Add(YsCyan());
				triColBuf.Add(YsCyan());
				triColBuf.Add(YsCyan());
				triColBuf.Add(YsCyan());
				triColBuf.Add(YsCyan());
				triVtxBuf.Add(bankMat*YsVec3(x2,-y2,0));
				triVtxBuf.Add(bankMat*YsVec3(x1,-y1,0));
				triVtxBuf.Add(bankMat*YsVec3(prevX1,-prevY1,0));
				triVtxBuf.Add(bankMat*YsVec3(prevX1,-prevY1,0));
				triVtxBuf.Add(bankMat*YsVec3(prevX2,-prevY2,0));
				triVtxBuf.Add(bankMat*YsVec3(x2,-y2,0));
				triColBuf.Add(brown);
				triColBuf.Add(brown);
				triColBuf.Add(brown);
				triColBuf.Add(brown);
				triColBuf.Add(brown);
				triColBuf.Add(brown);
			}

			prevX1=x1;
			prevY1=y1;
			prevX2=x2;
			prevY2=y2;
		}

		for(int i=0; i<=180; i+=30)  // 2 tri * 7 = 14 triangless
		{
			if(90!=i)
			{
				const float a=(float)i*(float)YsPi/180.0f;
				const float s=(float)sin(a);
				const float c=(float)cos(a);

				const float x1=inRad*c;
				const float y1=inRad*s;
				const float x2=outRad*c;
				const float y2=outRad*s;
				const float tx=-s*0.02f;
				const float ty= c*0.02f;

				triVtxBuf.Add(bankMat*YsVec3(x1+tx,y1+ty,0.0f));
				triVtxBuf.Add(bankMat*YsVec3(x2+tx,y2+ty,0.0f));
				triVtxBuf.Add(bankMat*YsVec3(x2-tx,y2-ty,0.0f));
				triColBuf.Add(YsWhite());
				triColBuf.Add(YsWhite());
				triColBuf.Add(YsWhite());

				triVtxBuf.Add(bankMat*YsVec3(x2-tx,y2-ty,0.0f));
				triVtxBuf.Add(bankMat*YsVec3(x1-tx,y1-ty,0.0f));
				triVtxBuf.Add(bankMat*YsVec3(x1+tx,y1+ty,0.0f));
				triColBuf.Add(YsWhite());
				triColBuf.Add(YsWhite());
				triColBuf.Add(YsWhite());
			}
		}

		triVtxBuf.Add(bankMat*YsVec3( 0.09f,outRad,0.0f));
		triVtxBuf.Add(bankMat*YsVec3(-0.09f,outRad,0.0f));
		triVtxBuf.Add(bankMat*YsVec3(0.0f,inRad,0.0f));
		triColBuf.Add(YsWhite());
		triColBuf.Add(YsWhite());
		triColBuf.Add(YsWhite());



		const float lineVtx2[12*3]=
		{
			(float)YsCos45deg*( inRad      ),(float)YsSin45deg*(inRad      ),0.0f,
			(float)YsCos45deg*( inRad+0.12f),(float)YsSin45deg*(inRad+0.12f),0.0f,
			(float)YsCos45deg*(-inRad      ),(float)YsSin45deg*(inRad      ),0.0f,
			(float)YsCos45deg*(-inRad-0.12f),(float)YsSin45deg*(inRad+0.12f),0.0f,

			(float)YsCos80deg*( inRad      ),(float)YsSin80deg*(inRad      ),0.0f,
			(float)YsCos80deg*( inRad+0.12f),(float)YsSin80deg*(inRad+0.12f),0.0f,
			(float)YsCos80deg*(-inRad      ),(float)YsSin80deg*(inRad      ),0.0f,
			(float)YsCos80deg*(-inRad-0.12f),(float)YsSin80deg*(inRad+0.12f),0.0f,

			(float)YsCos70deg*( inRad      ),(float)YsSin70deg*(inRad      ),0.0f,
			(float)YsCos70deg*( inRad+0.12f),(float)YsSin70deg*(inRad+0.12f),0.0f,
			(float)YsCos70deg*(-inRad      ),(float)YsSin70deg*(inRad      ),0.0f,
			(float)YsCos70deg*(-inRad-0.12f),(float)YsSin70deg*(inRad+0.12f),0.0f
		};
		for(int i=0; i<12; ++i)
		{
			lineVtxBuf.Add(bankMat*YsVec3(lineVtx2[i*3],lineVtx2[i*3+1],lineVtx2[i*3+2]));
			lineColBuf.Add(YsWhite());
		}
	}

	YsColor red;
	red.SetFloatRGBA(0.71f,0.31f,0.31f,1.0f);

	const double zeroBankTriVtx[9]=
	{
		 0.09,inRad-0.2,0.0,
		-0.09,inRad-0.2,0.0,
		 0.0 ,inRad    ,0.0
	};
	ovTriVtxBuf.Add(mat*YsVec3(zeroBankTriVtx[0],zeroBankTriVtx[1],zeroBankTriVtx[2]));
	ovTriVtxBuf.Add(mat*YsVec3(zeroBankTriVtx[3],zeroBankTriVtx[4],zeroBankTriVtx[5]));
	ovTriVtxBuf.Add(mat*YsVec3(zeroBankTriVtx[6],zeroBankTriVtx[7],zeroBankTriVtx[8]));
	ovTriColBuf.Add(red);
	ovTriColBuf.Add(red);
	ovTriColBuf.Add(red);

	{
		const double triStripVtx[22*3]=
		{
			 -0.600000,   0.020000, 0.0,
			 -0.600000,  -0.020000, 0.0,
			 -0.200000,   0.020000, 0.0,
			 -0.232043,  -0.020000, 0.0,
			 -0.184776,  -0.056537, 0.0,
			 -0.221731,  -0.071844, 0.0,
			 -0.141421,  -0.121421, 0.0,
			 -0.169706,  -0.149706, 0.0,
			 -0.076537,  -0.164776, 0.0,
			 -0.091844,  -0.201731, 0.0,
			 -0.000000,  -0.180000, 0.0,
			  0.000000,  -0.220000, 0.0,
			  0.076537,  -0.164776, 0.0,
			  0.091844,  -0.201731, 0.0,
			  0.141421,  -0.121421, 0.0,
			  0.169706,  -0.149706, 0.0,
			  0.184776,  -0.056537, 0.0,
			  0.221731,  -0.071844, 0.0,
			  0.200000,   0.020000, 0.0,
			  0.232043,  -0.020000, 0.0,
			  0.600000,   0.020000, 0.0,
			  0.600000,  -0.020000, 0.0
		};
		YsVec3 prev[2];
		for(int i=0; i<22; i+=2)
		{
			YsVec3 p0=mat*YsVec3(triStripVtx[i*3  ],triStripVtx[i*3+1],triStripVtx[i*3+2]);
			YsVec3 p1=mat*YsVec3(triStripVtx[i*3+3],triStripVtx[i*3+4],triStripVtx[i*3+5]);

			if(0<i)
			{
				ovTriVtxBuf.Add(prev[0]);
				ovTriVtxBuf.Add(prev[1]);
				ovTriVtxBuf.Add(p1);
				ovTriVtxBuf.Add(p1);
				ovTriVtxBuf.Add(p0);
				ovTriVtxBuf.Add(prev[0]);
				ovTriColBuf.Add(red);
				ovTriColBuf.Add(red);
				ovTriColBuf.Add(red);
				ovTriColBuf.Add(red);
				ovTriColBuf.Add(red);
				ovTriColBuf.Add(red);
			}

			prev[0]=p0;
			prev[1]=p1;
		}
	}

	const double centerCircleVtx[8*3]=
	{
		  0.035000,   0.000000, 0.0,
		  0.024749,   0.024749, 0.0,
		 -0.000000,   0.035000, 0.0,
		 -0.024749,   0.024749, 0.0,
		 -0.035000,   0.000000, 0.0,
		 -0.024749,  -0.024749, 0.0,
		  0.000000,  -0.035000, 0.0,
		  0.024749,  -0.024749, 0.0,
	};
	for(int i=1; i<7; ++i)
	{
		ovTriVtxBuf.Add(mat*YsVec3(centerCircleVtx[0],centerCircleVtx[1],centerCircleVtx[2]));
		ovTriVtxBuf.Add(mat*YsVec3(centerCircleVtx[i*3  ],centerCircleVtx[i*3+1],centerCircleVtx[i*3+2]));
		ovTriVtxBuf.Add(mat*YsVec3(centerCircleVtx[i*3+3],centerCircleVtx[i*3+4],centerCircleVtx[i*3+5]));
		ovTriColBuf.Add(red);
		ovTriColBuf.Add(red);
		ovTriColBuf.Add(red);
	}
}

void FsInstrumentPanel::DrawSimpleRectInstrument3d(
    const YsVec2 &g1,const YsVec2 &g2,
    const char caption[],const char captionBtm[],const char captionTop[],const double &fuelPercent)
{
	AddRectangularInstrumentFrameVertexArray(g1,g2);

	YsMatrix4x4 mat;
	mat.Translate(g1.x(),g1.y(),0.0);
	mat.Scale(g2.x()-g1.x(),g2.y()-g1.y(),1.0);

	const double fontWid=0.15;
	const double fontHei=0.08;

	auto fontMat=mat;
	fontMat.Translate(0.35,0.25,0.0);
	fontMat.RotateXY(YsPi/2.0);
	fontMat.Scale(0.1,0.25,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,caption,YsWhite());


	const double gy1=fontHei*2.0;
	const double gy2=0.95-fontHei;


	fontMat=mat;
	fontMat.Translate(0.1,0.05,0.0);
	fontMat.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,captionBtm,YsWhite());

	fontMat=mat;
	fontMat.Translate(0.1,1.0-fontHei-0.05,0.0);
	fontMat.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,captionTop,YsWhite());



	for(int i=0; i<=10; ++i)
	{
		const double y=gy1+(gy2-gy1)*(i*10.0)/100.0;
		lineVtxBuf.Add(mat*YsVec3(0.5,y,0.0));
		lineVtxBuf.Add(mat*YsVec3(0.7,y,0.0));
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
	}

	const double y=gy1+(gy2-gy1)*(double)fuelPercent;
	double triVtx[9]=
	{
		0.75,y     ,0.0,
		0.9, y-0.05,0.0,
		0.9, y+0.05,0.0,
	};
	triVtxBuf.Add(mat*YsVec3(triVtx[0],triVtx[1],triVtx[2]));
	triVtxBuf.Add(mat*YsVec3(triVtx[3],triVtx[4],triVtx[5]));
	triVtxBuf.Add(mat*YsVec3(triVtx[6],triVtx[7],triVtx[8]));
	triColBuf.Add(YsWhite());
	triColBuf.Add(YsWhite());
	triColBuf.Add(YsWhite());
}

void FsInstrumentPanel::DrawGear3d(const double &gear)
{
	auto g1=gear1;
	auto g2=gear2;

	AddRectangularInstrumentFrameVertexArray(g1,g2);

	YsMatrix4x4 mat;
	mat.Translate(g1.x(),g1.y(),0.0);
	mat.Scale(g2.x()-g1.x(),g2.y()-g1.y(),1.0);

	if(0.99<gear)
	{
		const double fontWid=0.18;
		const double fontHei=0.22;

		auto fontMat=mat;
		fontMat.Translate(0.5-fontWid*2.0,0.51,0.0);
		fontMat.Scale(fontWid,fontHei,1.0);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"GEAR",YsGreen());

		fontMat.Translate(0.0,-1.2,0.0);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"DOWN",YsGreen());
	}
}

void FsInstrumentPanel::DrawEngine3d(const double &thr,YSBOOL ab)
{
	AddCircularInstrumentFrameVertexArray(enginePos,engineRad);

	YsMatrix4x4 mat;
	mat.Translate(YsVec3(enginePos,0));
	mat.Scale(engineRad,engineRad,1.0);

	// 0-> YsPi*3/4   100->-YsPi/3/4

	const double outRad=1.0;
	const double inRad=0.9;

	const double fontWid=inRad/8.0;
	const double fontHei=inRad/6.0;


	for(int i=0; i<=100; i+=2)
	{
		const double a=-(double)YsPi*0.75+(double)(YsPi*1.5)*(double)i/100.0;
		const double c=(double)cos(a);
		const double s=(double)sin(a);

		const double x1=s*outRad;
		const double y1=c*outRad;
		double x2,y2;

		if(0==i%5)
		{
			x2=s*inRad*9.0/10.0;
			y2=c*inRad*9.0/10.0;
		}
		else
		{
			x2=s*inRad;
			y2=c*inRad;
		}

		lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0));
		lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0));
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());
	}


	for(int i=0; i<=100; i+=100)
	{
		const double a=-(double)YsPi*0.75+(double)(YsPi*1.5)*(double)i/100.0;
		const double c=(double)cos(a);
		const double s=(double)sin(a);

		if(i==0)
		{
			const double x=s*inRad*8.0/10.0;
			const double y=c*inRad*8.0/10.0;
			auto fontMat=mat;
			fontMat.Translate(x-fontWid*2.0,y-fontHei/2.0,0.0);
			fontMat.Scale(fontWid,fontHei,1.0);
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"IDLE",YsWhite());
		}
		else if(i==100)
		{
			const double x=s*inRad*8.0f/10.0f;
			const double y=c*inRad*8.0f/10.0f;
			auto fontMat=mat;
			fontMat.Translate(x-fontWid*2.0,y-fontHei/2.0,0.0);
			fontMat.Scale(fontWid,fontHei,1.0);
			FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"MAX",YsWhite());
		}
	}

	AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),mat,0.02,inRad,-YsPi*3/4+(YsPi*6/4)*thr,0.02);

	if(ab==YSTRUE)
	{
		const double rectVtx[5*3]=
		{
			-fontWid*2.0,-inRad+fontHei/2.0    ,0.0,
			-fontWid*2.0,-inRad+fontHei*5.0/2.0,0.0,
			 fontWid*2.0,-inRad+fontHei*5.0/2.0,0.0,
			 fontWid*2.0,-inRad+fontHei/2.0    ,0.0,

			-fontWid*2.0,-inRad+fontHei/2.0    ,0.0,
		};
		for(int i=1; i<3; ++i)
		{
			triVtxBuf.Add(mat*YsVec3(rectVtx[0],rectVtx[1],rectVtx[2]));
			triVtxBuf.Add(mat*YsVec3(rectVtx[i*3  ],rectVtx[i*3+1],rectVtx[i*3+2]));
			triVtxBuf.Add(mat*YsVec3(rectVtx[i*3+3],rectVtx[i*3+4],rectVtx[i*3+5]));
			triColBuf.Add(YsRed());
			triColBuf.Add(YsRed());
			triColBuf.Add(YsRed());
		}
		for(int i=0; i<4; ++i)
		{
			lineVtxBuf.Add(mat*YsVec3(rectVtx[i*3  ],rectVtx[i*3+1],rectVtx[i*3+2]));
			lineVtxBuf.Add(mat*YsVec3(rectVtx[i*3+3],rectVtx[i*3+4],rectVtx[i*3+5]));
			lineColBuf.Add(YsWhite());
			lineColBuf.Add(YsWhite());
		}

		auto fontMat=mat;
		fontMat.Translate(-fontWid,-inRad+fontHei,0.0);
		fontMat.Scale(fontWid,fontHei,1.0);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"AB",YsWhite());
	}
}

void FsInstrumentPanel::DrawG3d(const double &g)
{
	AddRectangularInstrumentFrameVertexArray(g1,g2);

	YsMatrix4x4 mat;
	mat.Translate(g1.x(),g1.y(),0.0);
	mat.Scale(g2.x()-g1.x(),g2.y()-g1.y(),1.0);


	double gBracket;
	char str[256];

	gBracket=YsBound <double> (g,-3.0,9.0);

	const double fontWid=0.2;
	const double fontHei=0.1;

	// Rect inst (0,0)-(1,1)

	//const double white[4]={1.0f,1.0f,1.0f,1.0f};
	//YsGLSLSet3DRendererUniformColorfv(renderer,white);


	//double prevTfm[16];
	//YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);


	//double tfm[16];
	//YsGLCopyMatrixfv(tfm,prevTfm);
	//YsGLMultMatrixTranslationfv(tfm,0.1f,0.25f-fontHei/2.0f,0.0f);
	//YsGLMultMatrixScalingfv(tfm,fontWid,fontHei,1.0f);
	//YsGLSLSet3DRendererModelViewfv(renderer,tfm);
	//FsDrawWireFont("G");
	auto fontMat=mat;
	fontMat.Translate(0.1,0.25-fontHei/2.0,0.0);
	fontMat.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"G",YsWhite());



	const double gy1=    fontHei/2.0;
	const double gy2=1.0-fontHei/2.0;

	for(int i=-3; i<=9; i+=3)
	{
		const double y=gy1+(gy2-gy1)*(double)(i+3)/12.0;

		const double x=1.0f/3.0f;
		sprintf(str,"% d",i);

		//YsGLCopyMatrixfv(tfm,prevTfm);
		//YsGLMultMatrixTranslationfv(tfm,x,y-fontHei/3.0f,0.0f);
		//YsGLMultMatrixScalingfv(tfm,fontWid*2.0f/3.0f,fontHei*2.0f/3.0f,1.0f);
		//YsGLSLSet3DRendererModelViewfv(renderer,tfm);
		//FsDrawWireFont(str);

		fontMat=mat;
		fontMat.Translate(x,y-fontHei/3.0,0.0);
		fontMat.Scale(fontWid*2.0/3.0,fontHei*2.0/3.0,1.0);
		FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,YsWhite());
	}


	//YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);

	//int nYellowLineVtx=0;
	//double yellowLineVtx[13*3*2];
	//int nRedLineVtx=0;
	//double redLineVtx[13*3*2];
	//int nWhiteLineVtx=0;
	//double whiteLineVtx[13*3*2];

	for(int i=-3; i<=9; i++) // 13 lines
	{
		const double y=gy1+(gy2-gy1)*(double)(i+3)/12.0f;
		const double x1=2.0f/3.0f;
		const double x2=0.75f;

		YsColor col;
		if(i<=-2 || 8<=i)
		{
			col=YsRed();
			//FsGLAddVertex3 <double> (nRedLineVtx,redLineVtx,x1,y,0.0f);
			//FsGLAddVertex3 <double> (nRedLineVtx,redLineVtx,x2,y,0.0f);
		}
		else if(6<=i)
		{
			col=YsYellow();
			//FsGLAddVertex3 <double> (nYellowLineVtx,yellowLineVtx,x1,y,0.0f);
			//FsGLAddVertex3 <double> (nYellowLineVtx,yellowLineVtx,x2,y,0.0f);
		}
		else
		{
			col=YsWhite();
			//FsGLAddVertex3 <double> (nWhiteLineVtx,whiteLineVtx,x1,y,0.0f);
			//FsGLAddVertex3 <double> (nWhiteLineVtx,whiteLineVtx,x2,y,0.0f);
		}
		lineVtxBuf.Add(mat*YsVec3(x1,y,0.0));
		lineVtxBuf.Add(mat*YsVec3(x2,y,0.0));
		lineColBuf.Add(col);
		lineColBuf.Add(col);
	}

	//const double yellow[4]={1.0f,1.0f,0.0f,1.0f};
	//YsGLSLSet3DRendererUniformColorfv(renderer,yellow);
	//YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,nYellowLineVtx,yellowLineVtx);

	//const double red[4]={1.0f,0.0f,0.0f,1.0f};
	//YsGLSLSet3DRendererUniformColorfv(renderer,red);
	//YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,nRedLineVtx,redLineVtx);

	//YsGLSLSet3DRendererUniformColorfv(renderer,white);
	//YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,nWhiteLineVtx,whiteLineVtx);


	YsColor col;
	if((4.0<g && g<6.0) || (-1.0<g && g<-0.5))
	{
		//YsGLSLSet3DRendererUniformColorfv(renderer,yellow);
		col=YsYellow();
	}
	else if(g<-1.0 || 6.0<g)
	{
		//YsGLSLSet3DRendererUniformColorfv(renderer,red);
		col=YsRed();
	}
	else
	{
		//YsGLSLSet3DRendererUniformColorfv(renderer,white);
		col=YsWhite();
	}
	
	const double y=gy1+(gy2-gy1)*(gBracket+3.0)/12.0;

	const double triVtx[3*3]=
	{
		0.75,y    ,0.0,
		0.9,y-0.05,0.0,
		0.9,y+0.05,0.0,
	};
	triVtxBuf.Add(mat*YsVec3(triVtx[0],triVtx[1],triVtx[2]));
	triVtxBuf.Add(mat*YsVec3(triVtx[3],triVtx[4],triVtx[5]));
	triVtxBuf.Add(mat*YsVec3(triVtx[6],triVtx[7],triVtx[8]));
	triColBuf.Add(col);
	triColBuf.Add(col);
	triColBuf.Add(col);
}

void FsInstrumentPanel::DrawAmmo3d(int nGun,int maxNGun,FSWEAPONTYPE wpnType,int wpnLeft)
{
	AddCircularInstrumentFrameVertexArray(ammoPos,ammoRad);

	YsMatrix4x4 mat;
	mat.Translate(YsVec3(ammoPos,0));
	mat.Scale(ammoRad,ammoRad,1.0);


	// const double outRad=1.0f;
	const double inRad=0.90f;

	const double fontWid=inRad/7.0f;
	const double fontHei=inRad/5.0f;

	//double prevTfm[16];
	//YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);


	//double tfm[16];
	//YsGLCopyMatrixfv(tfm,prevTfm);
	//YsGLMultMatrixTranslationfv(tfm,-fontWid*2.0f,-inRad+fontHei,0.0f);
	//YsGLMultMatrixScalingfv(tfm,fontWid,fontHei,1.0f);
	//YsGLSLSet3DRendererModelViewfv(renderer,tfm);
	//FsDrawWireFont("AMMO");
	auto fontMat=mat;
	fontMat.Translate(-fontWid*2.0,-inRad+fontHei,0.0);
	fontMat.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,"AMMO",YsWhite());




	char str[256];
	switch(wpnType)
	{
	default:
		str[0]=0;
		break;
	case FSWEAPON_GUN:
		strcpy(str,"GUN");
		break;
	case FSWEAPON_AIM9:
		sprintf(str,"AAM(S) %d",wpnLeft);
		break;
	case FSWEAPON_AGM65:
		sprintf(str,"AGM %d",wpnLeft);
		break;
	case FSWEAPON_BOMB:
		sprintf(str,"BOM %d",wpnLeft);
		break;
	case FSWEAPON_ROCKET:
		sprintf(str,"RKT %d",wpnLeft);
		break;
	case FSWEAPON_FLARE:
		sprintf(str,"FLR %d",wpnLeft);
		break;
	case FSWEAPON_AIM120:
		sprintf(str,"AIM(M) %d",wpnLeft);
		break;
	case FSWEAPON_BOMB250:
		sprintf(str,"BOM250 %d",wpnLeft);
		break;
	case FSWEAPON_SMOKE:
		sprintf(str,"SMK %d",wpnLeft);
		break;
	case FSWEAPON_FUELTANK:
		sprintf(str,"TANK%d",wpnLeft);
		break;
	}

	//YsGLCopyMatrixfv(tfm,prevTfm);
	//YsGLMultMatrixTranslationfv(tfm,-fontWid*0.5f*(double)strlen(str),inRad-fontHei,0.0f);
	//YsGLMultMatrixScalingfv(tfm,fontWid,fontHei,1.0f);
	//YsGLSLSet3DRendererModelViewfv(renderer,tfm);
	//FsDrawWireFont(str);
	fontMat=mat;
	fontMat.Translate(-fontWid*0.5*(double)strlen(str),inRad-fontHei,0.0);
	fontMat.Scale(fontWid,fontHei,1.0);
	FsAddWireFontVertexBuffer(lineVtxBuf,lineColBuf,triVtxBuf,triColBuf,fontMat,str,YsWhite());


	//YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);


	int needle;
	if(maxNGun>0)
	{
		needle=nGun*90/maxNGun;
	}
	else
	{
		needle=0;
	}

	//const int maxNLineVtx=8;
	//int nLineVtx=0;
	//double lineVtx[maxNLineVtx*3];
	for(int i=0; i<=90; i+=30)
	{
		const double a=(double)YsPi*3.0f/4.0f-(double)YsDegToRad(i);
		const double s=(double)sin(a);
		const double c=(double)cos(a);

		const double x1=              +(c*inRad*1.2);
		const double y1=-inRad+fontHei+(s*inRad*1.2);
		const double x2=              +(c*inRad);
		const double y2=-inRad+fontHei+(s*inRad);

		//FsGLAddVertex3 <double> (nLineVtx,lineVtx,x1,y1,0.0);
		//FsGLAddVertex3 <double> (nLineVtx,lineVtx,x2,y2,0.0);
		lineVtxBuf.Add(mat*YsVec3(x1,y1,0.0));
		lineVtxBuf.Add(mat*YsVec3(x2,y2,0.0));
		lineColBuf.Add(YsWhite());
		lineColBuf.Add(YsWhite());

		// if(i==0)
		// {
		// 	x2=cx      +(int)(c*(double)inRad*1.3);
		// 	y2=cy+inRad-fontHei-(int)(s*(double)inRad*1.3);
		// 	FsDrawWireFont2D(cx,cy+inRad-fontHei,"0",YsWhite(),fontWid,fontHei);
		// }
	}


	//YsGLCopyMatrixfv(tfm,prevTfm);
	//YsGLMultMatrixTranslationfv(tfm,0.0f,-inRad+fontHei,0.0f);
	//YsGLSLSet3DRendererModelViewfv(renderer,tfm);
	//DrawNeedle3d(0.02,inRad,-YsPi/4.0+YsDegToRad(needle),0.02);
	auto needleMat=mat;
	needleMat.Translate(0.0,-inRad+fontHei,0.0);
	AddNeedleVertexArray(ovLineVtxBuf,ovLineColBuf,ovTriVtxBuf,ovTriColBuf,YsWhite(),YsGrayScale(0.3),needleMat,0.02,inRad,-YsPi/4.0+YsDegToRad(needle),0.02);
}


