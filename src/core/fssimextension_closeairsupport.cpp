#include <ysglfontdata.h>

#include "fssimextension_closeairsupport.h"
#include "fsworld.h"
#include "fssimulation.h"
#include "fsexistence.h"



FsSimExtension_CloseAirSupport::FsSimExtension_CloseAirSupport() : initRemainTime(15.0*60.0), maxNumTank(10)
{
	gLimit=4.0;
	remainTime=initRemainTime;
	nEnemy=0;
}

/* static */ const char *FsSimExtension_CloseAirSupport::Ident(void)
{
	return "CLOSEAIRSUPPORT";
}

/* static */ std::shared_ptr <FsSimExtensionBase> FsSimExtension_CloseAirSupport::Generate(void)
{
	std::shared_ptr <FsSimExtensionBase> ptr;
	ptr.reset(new THISCLASS);
	return ptr;
}

/* static */ std::function <std::shared_ptr <FsSimExtensionBase>(void)> FsSimExtension_CloseAirSupport::Generator(void)
{
	std::function <std::shared_ptr <FsSimExtensionBase>(void)> func;
	func=std::bind(&THISCLASS::Generate);
	return func;
}

/* virtual */ const char *FsSimExtension_CloseAirSupport::GetIdent(void) const
{
	return THISCLASS::Ident();
}
/* virtual */ YsArray <YsString> FsSimExtension_CloseAirSupport::Serialize(const FsSimulation *sim)
{
	YsArray <YsString> encoded;
	YsString param;
	YsString header;

	header.Append(FirstCommand());
	header.Append(" ");
	header.Append(Ident());
	header.Append(" ");

	encoded.Increment();
	encoded.Last().Append(header);
	encoded.Last().Append(param);


	return encoded;
}
/* virtual */ YSRESULT FsSimExtension_CloseAirSupport::ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv)
{
	return YSOK;
}

/* virtual */ void FsSimExtension_CloseAirSupport::StartSimulation(FsSimulation *sim)
{
	if(nullptr!=sim->GetPlayerAirplane())
	{
		auto playerPlane=sim->GetPlayerAirplane();
		FsGround *gnd=nullptr;
		while((gnd=sim->FindNextGround(gnd))!=NULL)
		{
			if(gnd->iff!=playerPlane->iff)
			{
				gnd->SendCommand("INITIGUN 0");
				gnd->SendCommand("INITISAM 0");
			}
		}
	}

	srand((unsigned int)time(NULL));

	gLimit=4.0;
	remainTime=initRemainTime;
	sim->GenerateTank(10,info,YSFALSE,YSFALSE,YSTRUE,FS_IFF0,7,-1);
	nEnemy=0;
}

/* virtual */ void FsSimExtension_CloseAirSupport::EndSimulation(FsSimulation *sim)
{
}

/* virtual */ void FsSimExtension_CloseAirSupport::OnInterval(FsSimulation *sim,double dt)
{
	if(YSTRUE!=sim->Paused())
	{
		remainTime-=dt;

		const double elapsedTime=initRemainTime-remainTime;

		if(remainTime>0.0)
		{
			int nTank=int(elapsedTime/60.0)+3;
			nTank=YsBound(nTank,3,maxNumTank);
			sim->GenerateTank(nTank,info,YSTRUE,YSTRUE,YSTRUE,FS_IFF3,6,4);
		}
	}
}

/* virtual */ YsArray <FsSimExtensionBase::LowerLeftAndBitmap,0> FsSimExtension_CloseAirSupport::Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const
{
	YsArray <LowerLeftAndBitmap,0> overlay;
	if(0==windowId)
	{
		YsString msg;
		if(remainTime>0.0)
		{
			msg.Printf("TIME REMAIN : %.1lf",remainTime);
		}
		else
		{
			msg="TIME IS UP. RETURN TO BASE.";
		}

		const int fontWid=12;
		const int fontHei=16;
		auto fontPtr=YsFont12x16;

		YsBitmap bmp;
		const int bmpWid=(int)msg.Strlen()*fontWid;
		const int bmpHei=fontHei;
		bmp.PrepareBitmap(bmpWid,bmpHei);
		bmp.Clear(0,0,255,0);

		YsGlWriteStringToRGBA8Bitmap(
			msg.data(),
			0,255,0,255,
			bmp.GetEditableRGBABitmapPointer(),bmp.GetWidth(),bmp.GetHeight(),
			0,0,
			fontPtr,fontWid,fontHei);

		overlay.Increment();
		overlay.back().bmp.MoveFrom(bmp);
		overlay.back().pos.Set(winWid/2-bmpWid/2,bmpHei*2);
	}
	return overlay;
}
