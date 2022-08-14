#include <ysclass.h>
#include <time.h>

#ifdef WIN32  // Assuming UNIX
#define _WINSOCKAPI_
#include <windows.h>
#else
#include <sys/time.h>
#endif



#include <ysbitmap.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fsoption.h"

#include "fs.h"
#include "fsgui.h"
#include "graphics/common/fsopengl.h"
#include "platform/common/fswindow.h"

#include "graphics/common/fstexturemanager.h"



#include "fsguiselectiondialogbase.h"

#include "fsfilename.h"

FsGuiDialogWithFieldAndAircraft::FsGuiDialogWithFieldAndAircraft()
{
	scn=new YsScenery;
	stpLoaded=YSFALSE;

	FsOption opt;
	opt.Load(FsGetOptionFile());
	useMapPreview=opt.useMapPreview;
}

FsGuiDialogWithFieldAndAircraft::~FsGuiDialogWithFieldAndAircraft()
{
	delete scn;
}


void FsGuiDialogWithFieldAndAircraft::ReloadField(FsWorld *world,const char fieldName[])
{
	if(fieldName!=NULL && fieldName[0]!=0)
	{
		scn->Initialize();
		world->GetFieldVisual(*scn,fieldName);
		MakeMapVisibleDistanceAllInfinity();
	}
}

void FsGuiDialogWithFieldAndAircraft::ReloadStartPos(FsWorld *world,const char fieldName[],const char stpName[])
{
	if(fieldName!=NULL && fieldName[0]!=0 && stpName!=NULL && stpName[0]!=0)
	{
		FsStartPosInfo stpInfo;

		stpLoaded=YSFALSE;
		if(world->GetStartPositionInfo(stpInfo,fieldName,stpName)==YSOK)
		{
			if(stpInfo.onCarrier==YSTRUE)
			{
				if(scn!=NULL)
				{
					const YsSceneryGndObj *gob;
					YsMatrix4x4 mat(YSFALSE);

					gob=scn->SearchGndObjByTag(stpInfo.carrierTag);
					if(gob!=NULL && scn->GetTransformation(mat,gob)==YSOK)
					{
						stpPos=mat*stpInfo.relPosCarrier;

						YsVec3 ev,uv;
						ev=stpInfo.relAttCarrier.GetForwardVector();
						uv=stpInfo.relAttCarrier.GetUpVector();

						mat.Mul(ev,ev,0.0);
						mat.Mul(uv,uv,0.0);

						stpAtt.SetTwoVector(ev,uv);
						stpLoaded=YSTRUE;
					}
				}
			}
			else
			{
				if(stpInfo.InterpretPosition(stpPos,stpAtt)==YSOK)
				{
					stpLoaded=YSTRUE;
				}
			}
		}
	}
}

void FsGuiDialogWithFieldAndAircraft::MakeMapVisibleDistanceAllInfinity(void)
{
	YsArray <YsScenery *,16> todo;
	todo.Append(scn);
	for(int i=0; i<todo.GetN(); i++)
	{
		YsListItem <YsScenery2DDrawing> *map=NULL;
		while(NULL!=(map=todo[i]->FindNextMap(map)))
		{
			Ys2DDrawing *drw=&map->dat.GetDrawing();

			YsListItem <Ys2DDrawingElement> *elm=NULL;
			while(NULL!=(elm=drw->FindNextElem(elm)))
			{
				elm->dat.SetVisibleDist(YsInfinity);
			}
		}

		YsListItem <YsScenery> *chd=NULL;
		while(NULL!=(chd=todo[i]->FindNextChildScenery(chd)))
		{
			todo.Append(&chd->dat);
		}
	}
}

void FsGuiDialogWithFieldAndAircraft::DrawAirplane(FsWorld *world,const char typeName[],const YsAtt3 &att,YSSIZE_T nWpnCfg,const int *wpnCfg,YSBOOL smallMode) const
{
	const FsAirplaneTemplate *tmpl;
	FsVisualDnm vis;

	if((tmpl=world->GetAirplaneTemplate(typeName))!=NULL &&
	   (vis=world->GetAirplaneVisual(typeName))!=NULL)
	{
		FsAirplaneProperty prop;   // Be careful!  prop.belongTo will be NULL.
		YsVec3 pos;

		FsVisualDnm weaponShapeOverrideStatic[(int)FSWEAPON_NUMWEAPONTYPE];
		FsVisualDnm weaponShapeOverrideFlying[(int)FSWEAPON_NUMWEAPONTYPE];
		for(int i=0; i<(int)FSWEAPON_NUMWEAPONTYPE; i++)
		{
			weaponShapeOverrideStatic[i]=world->GetAirplaneWeaponShapeOverride(typeName,(FSWEAPONTYPE)i,0);
			weaponShapeOverrideFlying[i]=world->GetAirplaneWeaponShapeOverride(typeName,(FSWEAPONTYPE)i,1);
		}

		int wid,hei;
		FsGetWindowSize(wid,hei);


		for(int i=0; i<256; i++)
		{
			vis.SetState(i,0,0,0.0);
		}

		prop=*tmpl->GetProperty();

		if(smallMode!=YSTRUE)
		{
			pos.Set(0.0,0.0,prop.GetOutsideRadius()*2.2);
		}
		else
		{
			pos.Set(0.0,0.0,60.0);
		}
		prop.SetPosition(pos);
		prop.SetAttitude(att);
		prop.ApplyWeaponConfig(nWpnCfg,wpnCfg);
		prop.SetupVisual(vis);

		YsVec3 lightEv(-2.0,-10.0,10.0);
		lightEv.Normalize();
		if(YSTRUE==FsIsShadowMapAvailable())
		{
			auto &commonTexture=FsCommonTexture::GetCommonTexture();
			commonTexture.ReadyShadowMap();

			for(int i=0; i<commonTexture.GetMaxNumShadowMap(); ++i)
			{
				auto texUnit=commonTexture.GetShadowMapTexture(i);
				if(nullptr!=texUnit)
				{
					// For mysterious reason, the following block doesn't even erase
					// the shadow map. >>
					YsProjectionTransformation proj;
					proj.SetProjectionMode(YsProjectionTransformation::ORTHOGONAL);
					proj.SetAspectRatio(1.0);
					proj.SetOrthogonalProjectionHeight((double)(i+1)*prop.GetOutsideRadius()*2.2);
					proj.SetNearFar(0.0,(double)(i+4)*prop.GetOutsideRadius()*2.2); // Dirty trick to reduce depth fighting problem in shadow buffer.
					auto projMat=proj.GetProjectionMatrix();

					YsAtt3 lightViewAtt;
					lightViewAtt.SetForwardVector(lightEv);
					lightViewAtt.SetB(0.0);
					auto lightOrigin=pos-lightEv*prop.GetOutsideRadius()*1.1;

					YsMatrix4x4 viewMat;
					viewMat.RotateXY(-lightViewAtt.b());
					viewMat.RotateZY(-lightViewAtt.p());
					viewMat.RotateXZ(-lightViewAtt.h());
					viewMat.Translate(-lightOrigin);

					auto texWid=texUnit->GetWidth();
					auto texHei=texUnit->GetHeight();

					texUnit->BindFrameBuffer();

					FsBeginRenderShadowMap(projMat,viewMat,texWid,texHei);

					// It was a tough problem to fix:
					//  (1) Draw(pos,att) was ignoring camera matrix.  Need to use DrawShadow and give viewMat.
					//      Left-Hand coordinate transformation is taken care all in the DrawShadow function.
					//  (2) FsBeginRenderShadowMap was not enabling DepthMask and DepthFunc.
					vis.DrawShadow(viewMat,projMat,pos,att,YsIdentity4x4());
					prop.DrawOrdinanceShadow(YSFALSE,weaponShapeOverrideStatic,viewMat,projMat,YsIdentity4x4());

					FsEndRenderShadowMap();

					texUnit->Bind(5+i);
					FsEnableShadowMap(YsIdentity4x4(),projMat,viewMat,5+i,0+i);
					// For mysterious reason, the above block doesn't even erase
					// the shadow map. <<
				}
			}
		}


		FsProjection prj;
		FsSimulation::GetStandardProjection(prj);
		if(smallMode!=YSTRUE)
		{
			prj.cx=prj.cx*4/3;
			prj.cy=prj.cy*5/4;
		}
		else
		{
			prj.cx=wid*4/5;
			prj.cy=hei*3/10;
		}
		FsSetSceneProjection(prj);

		FsSetCameraPosition(YsOrigin(),YsZeroAtt(),YSTRUE);
		FsSetDirectionalLight(YsOrigin(),-lightEv,FSDAYLIGHT);

		vis.Draw(YsIdentity4x4(),prj.GetMatrix(),pos,att,YsVisual::DRAWOPAQUE);
		prop.DrawOrdinanceVisual(YSFALSE,weaponShapeOverrideStatic,YsIdentity4x4(),prj.GetMatrix(),YsVisual::DRAWOPAQUE);

		vis.Draw(YsIdentity4x4(),prj.GetMatrix(),pos,att,YsVisual::DRAWTRANSPARENT);
		prop.DrawOrdinanceVisual(YSFALSE,weaponShapeOverrideStatic,YsIdentity4x4(),prj.GetMatrix(),YsVisual::DRAWTRANSPARENT);

		FsFlushScene(); // BiFlushBuffer();

		FsSet2DDrawing();
	}
}

void FsGuiDialogWithFieldAndAircraft::DrawField(FsWorld * /*world*/,YSBOOL /*drawStp*/,YSBOOL smallMode,YSBOOL drawCursor,const YsVec3 &cursorPos) const
{
	if(useMapPreview==YSTRUE && scn!=NULL)
	{
		int wid,hei;
		FsGetWindowSize(wid,hei);

		double l;
		YsVec3 bbx[2],satellite;
		YsAtt3 att;
		scn->GetBoundingBox(bbx);

		satellite=(bbx[0]+bbx[1])/2.0;
		l=(bbx[1]-bbx[0]).L0Norm();
		if(smallMode==YSTRUE)
		{
			l*=1.2;
		}

		satellite.SetY(l*1.4);
		att.Set(0.0,-YsPi/2.0,0.0);


		YsMatrix4x4 viewMat;
		viewMat.RotateXY(-att.b());
		viewMat.RotateZY(-att.p());
		viewMat.RotateXZ(-att.h());
		viewMat.Translate(-satellite);


		FsDrawRect(0,0,wid-1,hei-1,scn->GetGroundColor(),YSTRUE);

		FsProjection prj;
		FsSimulation::GetStandardProjection(prj);
		prj.nearz=l;
		prj.farz=l*1.5;
		if(smallMode!=YSTRUE)
		{
			prj.cx=prj.cx*4/3;
		}
		else
		{
			prj.cx=wid*3/5;
			prj.cy=hei*3/5;
		}
		FsSetSceneProjection(prj);

		FsSetCameraPosition(satellite,att,YSTRUE);
		FsSetDirectionalLight(satellite,YsYVec(),FSDAYLIGHT);

		scn->DrawMapVisual(viewMat,YsIdentity4x4(),prj.GetMatrix(),-YsInfinity,YsInfinity,YSTRUE,0.0);
		scn->DrawVisual(viewMat,YsIdentity4x4(),prj.GetMatrix(),0.0,YSFALSE); // forShadowMap=YSFALSE

		if(stpLoaded==YSTRUE)
		{
			YsVec3 p1,p2;
			p1=stpPos;
			p2=stpPos;
			p1.SubX(20000.0);
			p2.AddX(20000.0);
			FsDrawLine3d(p1,p2,YsWhite());

			p1=stpPos;
			p2=stpPos;
			p1.SubZ(20000.0);
			p2.AddZ(20000.0);
			FsDrawLine3d(p1,p2,YsWhite());
		}

		if(YSTRUE==drawCursor)
		{
			YsVec3 p1,p2;
			p1=cursorPos;
			p2=cursorPos;
			p1.SubX(20000.0);
			p2.AddX(20000.0);
			FsDrawLine3d(p1,p2,YsWhite());

			p1=cursorPos;
			p2=cursorPos;
			p1.SubZ(20000.0);
			p2.AddZ(20000.0);
			FsDrawLine3d(p1,p2,YsWhite());
		}

		FsFlushScene();

		FsSet2DDrawing();
	}
}

void FsGuiDialogWithFieldAndAircraft::ResetFieldListBySearchKeyword(FsWorld *world,FsGuiListBox *fieldList,FsGuiTextBox *fieldSearch,YSBOOL forRacingMode)
{
	YsString curSel;
	YsArray <YsString,16> searchKeyWord;

	if(fieldList!=NULL)
	{
		fieldList->GetSelectedString(curSel);
	}

	YsString searchTxt;
	if(nullptr!=fieldSearch)
	{
		fieldSearch->GetText(searchTxt);
	}

	if(nullptr!=fieldList)
	{
		PopulateFieldList(world,fieldList,searchTxt,forRacingMode);
		if(0<curSel.Strlen())
		{
			YsString newCurSel;
			fieldList->SelectByString(curSel,YSTRUE);
			if(YSOK!=fieldList->GetSelectedString(newCurSel) || 0!=newCurSel.Strcmp(curSel))
			{
				FieldSelectionChanged();
			}
		}
	}
}

void FsGuiDialogWithFieldAndAircraft::FieldSelectionChanged(void)
{
}

void FsGuiDialogWithFieldAndAircraft::PopulateFieldList(FsWorld *world,FsGuiListBox *fieldList,const char keyword[],YSBOOL forRacingMode)
{
	YsArray <YsString> searchKeyWord;

	if(nullptr!=keyword && 0!=keyword[0])
	{
		YsString str(keyword);
		str.Capitalize();
		searchKeyWord=str.Argv();
	}


	YsArray <const char *> nameLst;
	const char *name;
	int i;
	for(i=0; (name=world->GetFieldTemplateName(i))!=NULL; i++)
	{
		if(0<searchKeyWord.size())
		{
			YsString cap(name);
			cap.Capitalize();
			if(FsTestKeyWordMatch(cap,searchKeyWord.GetN(),searchKeyWord)!=YSTRUE)
			{
				continue;
			}
		}
		if(YSTRUE==forRacingMode && YSTRUE!=world->IsFieldTemplateRaceCourseAvailable(i))
		{
			continue;
		}

		nameLst.Append(name);
	}

	if(0<nameLst.GetN() && nullptr!=fieldList)
	{
		fieldList->SetChoice(nameLst.GetN(),nameLst);
		fieldList->Select(0);
	}
}
