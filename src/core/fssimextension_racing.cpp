#include <ysglfontdata.h>

#include "fssimextension_racing.h"
#include "fssimulation.h"
#include "fsexistence.h"


FsSimExtension_RacingMode::FsSimExtension_RacingMode()
{
	nCheckPoint=0;
	prevNumCheckPoint=0;
	goalTime=0.0;
}

/*static*/ const char *FsSimExtension_RacingMode::Ident(void)
{
	return "RACINGMODE";
}
/*static*/ std::shared_ptr <FsSimExtensionBase> FsSimExtension_RacingMode::Generate(void)
{
	std::shared_ptr <FsSimExtensionBase> ptr;
	ptr.reset(new THISCLASS);
	return ptr;
}
/*static*/ std::function <std::shared_ptr <FsSimExtensionBase>(void)> FsSimExtension_RacingMode::Generator(void)
{
	std::function <std::shared_ptr <FsSimExtensionBase>(void)> func;
	func=std::bind(&THISCLASS::Generate);
	return func;
}

/* virtual */ const char *FsSimExtension_RacingMode::GetIdent(void) const
{
	return THISCLASS::Ident();
}
/* virtual */ YsArray <YsString> FsSimExtension_RacingMode::Serialize(const FsSimulation *sim)
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

	if(YsTolerance<goalTime)
	{
		param.Printf("GOALTIME %.2lfsec\n",goalTime);
		encoded.Increment();
		encoded.Last().Append(header);
		encoded.Last().Append(param);

		for(auto c : currentRun)
		{
			param.Printf("CURRENTRUN %d %.2lfsec\n",c.nCheckPointLeft,c.ctime);
			encoded.Increment();
			encoded.Last().Append(header);
			encoded.Last().Append(param);
		}
	}

	return encoded;
}
/* virtual */ YSRESULT FsSimExtension_RacingMode::ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv)
{
	if(4<=argv.size() && 0==argv[2].STRCMP("GOALTIME"))
	{
		goalTime=atof(argv[3]);
	}
	else if(5<=argv.size() && 0==argv[2].STRCMP("CURRENTRUN"))
	{
		currentRun.Increment();
		currentRun.Last().nCheckPointLeft=atoi(argv[3]);
		currentRun.Last().ctime=atof(argv[4]);
	}

	return YSOK;
}

/* virtual */ YSBOOL FsSimExtension_RacingMode::CanContinue(FsSimulation *sim) const
{
	return YSFALSE;
}
/* virtual */ YSBOOL FsSimExtension_RacingMode::CanResume(FsSimulation *sim) const
{
	return YSFALSE;
}

/* virtual */ void FsSimExtension_RacingMode::StartSimulation(FsSimulation *sim)
{
	currentRun.CleanUp();
	nCheckPoint=0;
	prevNumCheckPoint=0;
	goalTime=0.0;

	auto playerAir=sim->GetPlayerAirplane();
	if(nullptr!=playerAir)
	{
		playerAir->SendCommand("UNLOADWP");
		playerAir->SendCommand("INITIGUN 0");
	}
}
/* virtual */ void FsSimExtension_RacingMode::EndSimulation(FsSimulation *sim)
{
}
/* virtual */ void FsSimExtension_RacingMode::StartReplay(FsSimulation *sim)
{
}
/* virtual */ void FsSimExtension_RacingMode::EndReplay(FsSimulation *sim)
{
}
/* virtual */ void FsSimExtension_RacingMode::OnInterval(FsSimulation *sim,double)
{
	nCheckPoint=0;
	for(FsGround *obj=nullptr; nullptr!=(obj=sim->FindNextGround(obj)); )
	{
		if(YSTRUE==obj->Prop().IsAlive() &&
		   YSTRUE==obj->Prop().IsRacingCheckPoint())
		{
			++nCheckPoint;
		}
	}
	for(FsAirplane *obj=nullptr; nullptr!=(obj=sim->FindNextAirplane(obj)); )
	{
		if(YSTRUE==obj->Prop().IsAlive() &&
		   YSTRUE==obj->Prop().IsRacingCheckPoint())
		{
			++nCheckPoint;
		}
	}

	auto playerAir=sim->GetPlayerAirplane();
	if(nullptr!=playerAir && YSTRUE!=playerAir->isPlayingRecord && 0<prevNumCheckPoint)
	{
		if(0==nCheckPoint)
		{
			goalTime=sim->CurrentTime();
		}
		else if(nCheckPoint<prevNumCheckPoint)
		{
			currentRun.Increment();
			currentRun.Last().ctime=sim->CurrentTime();
			currentRun.Last().nCheckPointLeft=nCheckPoint;
		}
	}
	prevNumCheckPoint=nCheckPoint;
}

/* virtual */ YsArray <FsSimExtensionBase::LowerLeftAndBitmap,0> FsSimExtension_RacingMode::Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const
{
	YsArray <LowerLeftAndBitmap,0> overlay;
	if(0==windowId)
	{
		auto ctime=sim->CurrentTime();
		YsString msg;
		if(0<nCheckPoint)
		{
			msg.Printf("%.2lf (%d)",ctime,nCheckPoint);
		}
		else
		{
			double t=ctime-goalTime;
			int i=(int)(t*3.0);
			if(12<t || 0==i%2)
			{
				msg.Printf("GOAL %.2lf",goalTime);
			}
			else
			{
				msg=" ";
			}
		}

		const int fontWid=24;
		const int fontHei=40;
		auto fontPtr=YsFont24x40;

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
