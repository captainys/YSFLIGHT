#include <ysglfontdata.h>

#include "fssimextension_groundtoair.h"
#include "fsworld.h"
#include "fssimulation.h"
#include "fsexistence.h"



FsSimExtension_GroundToAir::FsSimExtension_GroundToAir() : initRemainTime(15.0*60.0)
{
	gLimit=4.0;
	remainTime=initRemainTime;
	nEnemy=0;
}

/* static */ const char *FsSimExtension_GroundToAir::Ident(void)
{
	return "GROUNDTOAIR";
}

/* static */ std::shared_ptr <FsSimExtensionBase> FsSimExtension_GroundToAir::Generate(void)
{
	std::shared_ptr <FsSimExtensionBase> ptr;
	ptr.reset(new THISCLASS);
	return ptr;
}

/* static */ std::function <std::shared_ptr <FsSimExtensionBase>(void)> FsSimExtension_GroundToAir::Generator(void)
{
	std::function <std::shared_ptr <FsSimExtensionBase>(void)> func;
	func=std::bind(&THISCLASS::Generate);
	return func;
}

/* virtual */ const char *FsSimExtension_GroundToAir::GetIdent(void) const
{
	return THISCLASS::Ident();
}
/* virtual */ YsArray <YsString> FsSimExtension_GroundToAir::Serialize(const FsSimulation *sim)
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
/* virtual */ YSRESULT FsSimExtension_GroundToAir::ProcessCommand(FsSimulation *sim,const YsConstArrayMask <YsString> &argv)
{
	return info.Decode(argv.Subset(2));
}

/* virtual */ void FsSimExtension_GroundToAir::StartSimulation(FsSimulation *sim)
{
	sim->ctlAssign.deadZoneElevator=YsGreater(sim->ctlAssign.deadZoneElevator,0.03);
	sim->ctlAssign.deadZoneAileron=YsGreater(sim->ctlAssign.deadZoneAileron,0.03);

	FsGround *gndSeeker=nullptr;
	while((gndSeeker=sim->FindNextGround(gndSeeker))!=NULL)
	{
		if(sim->GetPlayerGround()!=gndSeeker)
		{
			gndSeeker->SendCommand("INITIGUN 0");
			gndSeeker->SendCommand("INITISAM 0");
		}
	}
	auto gnd=sim->GetPlayerGround();
	if(nullptr!=gnd && FSWEAPON_NULL==gnd->Prop().GetWeaponOfChoice())
	{
		gnd->Prop().CycleWeaponOfChoiceByUser();
	}

	srand((unsigned int)time(NULL));

	FsOpenSubWindow(1);
	sim->SetSubWindowViewMode(1,sim->FSLOCKEDTARGETVIEW);

	gLimit=4.0;
	nEnemy=0;
	remainTime=initRemainTime;
}

/* virtual */ void FsSimExtension_GroundToAir::EndSimulation(FsSimulation *sim)
{
}

/* virtual */ void FsSimExtension_GroundToAir::OnInterval(FsSimulation *sim,double dt)
{
	if(YSTRUE!=sim->Paused())
	{
		remainTime-=dt;

		const double timeElapse=initRemainTime-remainTime;

		FsGround *playerGround=sim->GetPlayerGround();
		if(YSTRUE==info.infiniteGun && nullptr!=playerGround)
		{
			playerGround->Prop().SetNumAaaBullet(9999);
		}

		if(0.0<remainTime)
		{
			int nAttacker=int(timeElapse/120.0)+1;
			nAttacker=YsBound(nAttacker,1,info.attackerInfo.maxNumAttacker);
			sim->GenerateAttackerAirplane(nAttacker,info.attackerInfo,5500.0,4000.0,YSTRUE,90.0);
		}

		/* Not supported yet. if(timeElapse>10.0)
		{
			GenerateFriendlyAirplane(info);
		} */
	}
}

/* virtual */ YsArray <FsSimExtensionBase::LowerLeftAndBitmap,0> FsSimExtension_GroundToAir::Draw2D(const FsSimulation *sim,int windowId,int winWid,int winHei) const
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
