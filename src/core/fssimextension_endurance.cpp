#include <ysglfontdata.h>

#include "fssimextension_endurance.h"
#include "fsworld.h"
#include "fssimulation.h"
#include "fsexistence.h"



FsSimExtension_EnduranceMode::FsSimExtension_EnduranceMode()
{
	nEnemyMax=0;
	remainTime=15.0*60.0;
	gLimit=4.0;
	jet=YSTRUE;
	ww2=YSTRUE;
	allowAAM=YSTRUE;
}

/* static */ const char *FsSimExtension_EnduranceMode::Ident(void)
{
	return "ENDURANCE";
}

/* static */ std::shared_ptr <FsSimExtensionBase> FsSimExtension_EnduranceMode::Generate(void)
{
	std::shared_ptr <FsSimExtensionBase> ptr;
	ptr.reset(new THISCLASS);
	return ptr;
}

/* static */ std::function <std::shared_ptr <FsSimExtensionBase>(void)> FsSimExtension_EnduranceMode::Generator(void)
{
	std::function <std::shared_ptr <FsSimExtensionBase>(void)> func;
	func=std::bind(&THISCLASS::Generate);
	return func;
}

/* virtual */ const char *FsSimExtension_EnduranceMode::GetIdent(void) const
{
	return THISCLASS::Ident();
}
/* virtual */ YsArray <YsString> FsSimExtension_EnduranceMode::Serialize(const FsSimulation *sim)
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

	param.Printf("JET %s",YsBoolToStr(this->jet));
	encoded.Increment();
	encoded.Last().Append(header);
	encoded.Last().Append(param);

	param.Printf("WW2 %s",YsBoolToStr(this->ww2));
	encoded.Increment();
	encoded.Last().Append(header);
	encoded.Last().Append(param);

	param.Printf("AAM %s",YsBoolToStr(this->allowAAM));
	encoded.Increment();
	encoded.Last().Append(header);
	encoded.Last().Append(param);

	return encoded;
}
/* virtual */ YSRESULT FsSimExtension_EnduranceMode::ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv)
{
	if(4<=argv.size() && 0==argv[2].STRCMP("JET"))
	{
		this->jet=YsStrToBool(argv[3]);
	}
	if(4<=argv.size() && 0==argv[2].STRCMP("WW2"))
	{
		this->ww2=YsStrToBool(argv[3]);
	}
	if(4<=argv.size() && 0==argv[2].STRCMP("WPNTYPE"))
	{
		this->allowAAM=YsStrToBool(argv[3]);
	}
	return YSOK;
}

/* virtual */ void FsSimExtension_EnduranceMode::StartSimulation(FsSimulation *sim)
{
	srand((unsigned int)time(NULL));
	this->gLimit=4.0;
	this->remainTime=15.0*60.0;
	this->nEnemyMax=0;
	sim->AllowAAM(this->allowAAM);
}

/* virtual */ void FsSimExtension_EnduranceMode::EndSimulation(FsSimulation *sim)
{
}

/* virtual */ void FsSimExtension_EnduranceMode::OnInterval(FsSimulation *sim,double dt)
{
	if(YSTRUE!=sim->Paused())
	{
		remainTime-=dt;
		if(remainTime>0.0)
		{
			sim->GenerateEnemyAirplane(nEnemyMax,gLimit,remainTime,allowAAM,jet,ww2);
		}
	}
}

/* virtual */ YsArray <FsSimExtensionBase::LowerLeftAndBitmap,0> FsSimExtension_EnduranceMode::Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const
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
