#include <stdlib.h>
#include <ysglfontdata.h>

#include "fssimextension_intercept.h"
#include "fssimulation.h"
#include "fssiminfo.h"
#include "fsexistence.h"

FsSimExtension_InterceptMission::FsSimExtension_InterceptMission() : initRemainTime(15.0*60.0)
{
	gLimit=4.0;
	remainTime=initRemainTime;
	nEnemy=0;
}



/* static */ const char *FsSimExtension_InterceptMission::Ident(void)
{
	return "INTERCEPT";
}

/* static */ std::shared_ptr <FsSimExtensionBase> FsSimExtension_InterceptMission::Generate(void)
{
	std::shared_ptr <FsSimExtensionBase> ptr;
	ptr.reset(new THISCLASS);
	return ptr;
}

/* static */ std::function <std::shared_ptr <FsSimExtensionBase>(void)> FsSimExtension_InterceptMission::Generator(void)
{
	std::function <std::shared_ptr <FsSimExtensionBase>(void)> func;
	func=std::bind(&THISCLASS::Generate);
	return func;
}

/* virtual */ const char *FsSimExtension_InterceptMission::GetIdent(void) const
{
	return THISCLASS::Ident();
}

/* virtual */ YsArray <YsString> FsSimExtension_InterceptMission::Serialize(const FsSimulation *sim)
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

	for(auto &param : info.Encode())
	{
		encoded.Increment();
		encoded.Last().Append(header);
		encoded.Last().Append(param);
	}

	return encoded;
}
/* virtual */ YSRESULT FsSimExtension_InterceptMission::ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv)
{
	return info.Decode(argv.Subset(2));
}

/* virtual */ void FsSimExtension_InterceptMission::StartSimulation(FsSimulation *sim)
{
	srand((unsigned int)time(NULL));

	for(FsGround *gndSeeker=NULL; NULL!=(gndSeeker=sim->FindNextGround(gndSeeker)); )
	{
		gndSeeker->SendCommand("INITIGUN 0");
		gndSeeker->SendCommand("INITISAM 0");
	}

	gLimit=4.0;
	remainTime=initRemainTime;
	nEnemy=0;
}
/* virtual */ void FsSimExtension_InterceptMission::EndSimulation(FsSimulation *sim)
{
}

/* virtual */ void FsSimExtension_InterceptMission::OnInterval(FsSimulation *sim,double dt)
{
	if(YSTRUE!=sim->Paused())
	{
		remainTime-=dt;
	}

	const double timeElapse=initRemainTime-remainTime;
	if(remainTime>0.0)
	{
		int nAttacker=int(timeElapse/120.0)+1;
		nAttacker=YsBound(nAttacker,1,info.attackerInfo.maxNumAttacker);
		sim->GenerateAttackerAirplane(nAttacker,info.attackerInfo,20000.0,8000.0,YSFALSE,300.0);
	}

	if(timeElapse>10.0)
	{
		sim->GenerateFriendlyAirplane(info);
	}

}

/* virtual */ YsArray <FsSimExtensionBase::LowerLeftAndBitmap,0> FsSimExtension_InterceptMission::Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const
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
