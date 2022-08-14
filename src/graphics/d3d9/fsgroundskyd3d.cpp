#include <ysclass.h>
#include "fs.h"

#include <fstexturemanager.h>
#include <ystexturemanager_d3d9.h>

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"






////////////////////////////////////////////////////////////

// Implementation //////////////////////////////////////////
FsGroundSky::FsGroundSky()
{
	nLayer=8;
	nDeg=30.0;

	int i,j;
	double h,p,x,y,z;

	for(i=0; i<nLayer; i++)
	{
		p=nDeg*double(i*i)/(nLayer*nLayer);
		p=p*YsPi/180.0;
		y=sin(p);
		for(j=0; j<FsGndSkyDivX; j++)
		{
			h=double(j)*YsPi*2.0/double(FsGndSkyDivX-1);
			x=cos(p)*sin(h);
			z=cos(p)*cos(h);
			FsGndSkyHalfSphere[i][j].Set(x,y,z);
		}
	}

	for(i=nLayer; i<FsGndSkyDivY; i++)
	{
		p=nDeg+double(i-nLayer)*(90.0-nDeg)/double(FsGndSkyDivY-1-nLayer);
		p=p*YsPi/180.0;
		y=sin(p);
		for(j=0; j<FsGndSkyDivX; j++)
		{
			h=double(j)*YsPi*2.0/double(FsGndSkyDivX-1);
			x=cos(p)*sin(h);
			z=cos(p)*cos(h);
			FsGndSkyHalfSphere[i][j].Set(x,y,z);
		}
	}
}

FsGroundSky::~FsGroundSky()
{
}

void FsGroundSky::DrawByFog
    (const YsVec3 &pos,const YsAtt3 &viewAtt,const YsColor &ignd,const YsColor &isky,const YsColor &horizon,
     const double &farZ,YSBOOL specular)
{
	int i;
	YsAtt3 att;

	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast <FsD3dDevice *>(ysD3dDev);

	att.Set(viewAtt.h(),0.0,0.0);

	const double cylRad=farZ/10.0;
	double x0,x1;
	double a,y0,y,z;
	x0=-cylRad*3.0;  // Actually, I have to draw a infinitely long cylinder.
	x1= cylRad*3.0;  // So, x0 and x1 must be sufficiently long, but not too long to cause numerical shit.

	if(fsD3dDev->skyCol!=isky || 
	   fsD3dDev->gndCol!=ignd || 
	   fsD3dDev->gndPlgByFog==NULL || 
	   fsD3dDev->skyPlgByFog==NULL ||
	   fsD3dDev->gndSkyMode!=0)
	{
		if(fsD3dDev->gndPlgByFog!=NULL)
		{
			fsD3dDev->gndPlgByFog->ReleaseBuffer();
			delete fsD3dDev->gndPlgByFog;
			fsD3dDev->gndPlgByFog=NULL;
		}
		if(fsD3dDev->skyPlgByFog!=NULL)
		{
			fsD3dDev->skyPlgByFog->ReleaseBuffer();
			delete fsD3dDev->skyPlgByFog;
			fsD3dDev->skyPlgByFog=NULL;
		}

		fsD3dDev->gndSkyMode=0;
		fsD3dDev->skyCol=isky;
		fsD3dDev->gndCol=ignd;

		fsD3dDev->gndPlgByFog=new YsXyzColBuffer;
		fsD3dDev->gndPlgByFog->PrepareBuffer(*fsD3dDev,30);  // 30 is sufficient, but to prevent auto-flush, I get 32.

		fsD3dDev->skyPlgByFog=new YsXyzColBuffer;
		fsD3dDev->skyPlgByFog->PrepareBuffer(*fsD3dDev,(nLayer*2+1)*2);

		fsD3dDev->skyPlgByFog->LockBuffer();
		for(i=0; i<=nLayer*2; i++)
		{
			a=(YsPi/2.0)*double(i)/double(nLayer);
			y=cylRad*sin(a)/10.0;
			z=cylRad*cos(a);

			fsD3dDev->skyPlgByFog->lockedBuf[i*2].x=(float)x0;
			fsD3dDev->skyPlgByFog->lockedBuf[i*2].y=(float)y;
			fsD3dDev->skyPlgByFog->lockedBuf[i*2].z=(float)z;
			fsD3dDev->skyPlgByFog->lockedBuf[i*2].color=D3DCOLOR_ARGB(255,isky.Ri(),isky.Gi(),isky.Bi());

			fsD3dDev->skyPlgByFog->lockedBuf[i*2+1].x=(float)x1;
			fsD3dDev->skyPlgByFog->lockedBuf[i*2+1].y=(float)y;
			fsD3dDev->skyPlgByFog->lockedBuf[i*2+1].z=(float)z;
			fsD3dDev->skyPlgByFog->lockedBuf[i*2+1].color=D3DCOLOR_ARGB(255,isky.Ri(),isky.Gi(),isky.Bi());
		}
		fsD3dDev->skyPlgByFog->nBufUsed=(nLayer*2+1)*2;
		fsD3dDev->skyPlgByFog->UnlockBuffer();
	}



	// Ground polygons need to be re-made every time.  (Function of viewPos.y())
	// In total
	// (-64 -32 -16 -8 -4 -2 -1 0 1 2 4 8 16 32 64)x2 = 30 points in total.

	int ctr;
	double groundFadeAngle;
	if(YsEqual(viewAtt.p(),YsPi/2.0)!=YSTRUE && YsEqual(viewAtt.p(),-YsPi/2.0)!=YSTRUE)
	{
		double groundFadeZ;  // Distance that maps fade
		groundFadeZ=farZ*cos(viewAtt.p())+(farZ*sin(viewAtt.p())+pos.y())*tan(viewAtt.p());
		groundFadeAngle=atan2(pos.y(),groundFadeZ);
	}
	else
	{
		groundFadeAngle=0.0;
	}

	y0=-cylRad*sin(groundFadeAngle);
	ctr=0;

	fsD3dDev->gndPlgByFog->LockBuffer();

	fsD3dDev->gndPlgByFog->Set(ctr++,x0,0.0,cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
	fsD3dDev->gndPlgByFog->Set(ctr++,x1,0.0,cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

	for(i=32; i>=1; i/=2)
	{
		z=cylRad*(double)i/32.0;
		fsD3dDev->gndPlgByFog->Set(ctr++,x0,y0,z,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndPlgByFog->Set(ctr++,x1,y0,z,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
	}

	fsD3dDev->gndPlgByFog->Set(ctr++,x0,y0,0.0,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
	fsD3dDev->gndPlgByFog->Set(ctr++,x1,y0,0.0,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

	for(i=1; i<=32; i*=2)
	{
		z=cylRad*(double)i/32.0;
		fsD3dDev->gndPlgByFog->Set(ctr++,x0,y0,-z,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndPlgByFog->Set(ctr++,x1,y0,-z,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
	}

	fsD3dDev->gndPlgByFog->Set(ctr++,x0,0.0,-cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
	fsD3dDev->gndPlgByFog->Set(ctr++,x1,0.0,-cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

	fsD3dDev->gndPlgByFog->nBufUsed=30;
	fsD3dDev->gndPlgByFog->UnlockBuffer();



	D3DMATRIX pushMatrix,tra,rot;
	fsD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

	YsD3dMakeTranslation(tra,(float)pos.x(),(float)pos.y(),(float)pos.z());
	fsD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tra);
	YsD3dMakeRotationY(rot,(float)-att.h());
	fsD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&rot);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_STENCILWRITEMASK,0);

	float fogDensity,fogDensity10;
	fsD3dDev->d3dDev->GetRenderState(D3DRS_FOGDENSITY,(DWORD *)&fogDensity);
	fogDensity10=fogDensity*10.0F;
	fsD3dDev->d3dDev->SetRenderState(D3DRS_FOGDENSITY,*(DWORD *)(&fogDensity10));

	fsD3dDev->gndPlgByFog->DrawPrimitive(D3DPT_TRIANGLESTRIP);
	fsD3dDev->skyPlgByFog->DrawPrimitive(D3DPT_TRIANGLESTRIP);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_FOGDENSITY,*(DWORD *)(&fogDensity));

	fsD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_STENCILWRITEMASK,0xffffffff);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
}

void FsGroundSky::DrawGradation
    (const YsVec3 &pos,const YsAtt3 &att,const YsColor &ignd,const YsColor &isky,const YsColor &horizon,
     const double &farZ,YSBOOL specular)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast<FsD3dDevice *>(ysD3dDev);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_STENCILWRITEMASK,0);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);


	const double cylRad=5000.0;
	double x0,x1;
	x0=-cylRad*3.0;  // Actually, I have to draw a infinitely long cylinder.
	x1= cylRad*3.0;  // So, x0 and x1 must be sufficiently long, but not too long to cause numerical shit.


	D3DMATRIX pushMatrix,tra,rot;
	fsD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

	YsD3dMakeTranslation(tra,(float)pos.x(),(float)pos.y(),(float)pos.z());
	fsD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tra);
	YsD3dMakeRotationY(rot,(float)-att.h());
	fsD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&rot);

	if(fsD3dDev->gndSkyGradation==NULL || fsD3dDev->skyCol!=isky || fsD3dDev->gndCol!=ignd || fsD3dDev->gndSkyMode!=1)
	{
		if(fsD3dDev->gndSkyGradation!=NULL)
		{
			fsD3dDev->gndSkyGradation->ReleaseBuffer();
			delete fsD3dDev->gndSkyGradation;
		}


		int vtId;

		fsD3dDev->skyCol=isky;
		fsD3dDev->gndCol=ignd;
		fsD3dDev->gndSkyMode=1;

		fsD3dDev->gndSkyGradation=new YsXyzColBuffer;
		fsD3dDev->gndSkyGradation->PrepareBuffer(*fsD3dDev,(nLayer+1)*4+6);

		fsD3dDev->gndSkyGradation->LockBuffer();

		vtId=0;

		fsD3dDev->gndSkyGradation->Set(vtId++,x0, 0.0   ,-cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(vtId++,x1, 0.0   ,-cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(vtId++,x0,-cylRad, 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(vtId++,x1,-cylRad, 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(vtId++,x0, 0.0   , cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(vtId++,x1, 0.0   , cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

		int i;
		double a;
		YsColor col;
		for(i=0; i<=nLayer; i++)
		{
			a=YsDegToRad(nDeg*double(i)/double(nLayer));

			col.SetDoubleRGB
			  ((isky.Rd()*double(i)+horizon.Rd()*double(nLayer-i))/double(nLayer),
			   (isky.Gd()*double(i)+horizon.Gd()*double(nLayer-i))/double(nLayer),
			   (isky.Bd()*double(i)+horizon.Bd()*double(nLayer-i))/double(nLayer));

			fsD3dDev->gndSkyGradation->Set(vtId++,x0,cylRad*sin(a),cylRad*(cos(a)),col.Ri(),col.Gi(),col.Bi(),255);
			fsD3dDev->gndSkyGradation->Set(vtId++,x1,cylRad*sin(a),cylRad*(cos(a)),col.Ri(),col.Gi(),col.Bi(),255);

		}

		for(i=nLayer; i>=0; i--)
		{
			a=YsDegToRad(nDeg*double(i)/double(nLayer));

			col.SetDoubleRGB
			  ((isky.Rd()*double(i)+horizon.Rd()*double(nLayer-i))/double(nLayer),
			   (isky.Gd()*double(i)+horizon.Gd()*double(nLayer-i))/double(nLayer),
			   (isky.Bd()*double(i)+horizon.Bd()*double(nLayer-i))/double(nLayer));

			fsD3dDev->gndSkyGradation->Set(vtId++,x0,cylRad*sin(a),-cylRad*(cos(a)),col.Ri(),col.Gi(),col.Bi(),255);
			fsD3dDev->gndSkyGradation->Set(vtId++,x1,cylRad*sin(a),-cylRad*(cos(a)),col.Ri(),col.Gi(),col.Bi(),255);
		}

		if(vtId!=(nLayer+1)*4+6)
		{
			printf("Vertex Count Error %d %d\n",vtId,(nLayer+1)*4+6);
			exit(1);
		}

		fsD3dDev->gndSkyGradation->UnlockBuffer();
		fsD3dDev->gndSkyGradation->nBufUsed=vtId;
	}

	fsD3dDev->gndSkyGradation->DrawPrimitive(D3DPT_TRIANGLESTRIP);

	fsD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_STENCILWRITEMASK,0xffffffff);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
}

void FsGroundSky::DrawGroundMesh(const YsVec3 &pos,const YsAtt3 &att,const YsColor &ignd,int div,YSBOOL)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_EQUAL);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_STENCILREF,0);


	auto &commonTexture=FsCommonTexture::GetCommonTexture();
	commonTexture.LoadGroundTileTexture();
	auto texUnitPtr=commonTexture.GetGroundTileTexture();

	if(nullptr!=texUnitPtr)
	{
		// Setting up texture >>
		double invView[16];
		YsMatrix4x4 invViewTfm;
		invViewTfm.Translate(pos);
		invViewTfm.RotateXZ(att.h());
		invViewTfm.RotateZY(att.p());
		invViewTfm.RotateXY(att.b());
		invViewTfm.GetOpenGlCompatibleMatrix(invView);

		D3DMATRIX texTfm;
		YsD3dSetMatrix(texTfm,
			0.02F,0.00F,0.00F,0.00F,
			0.00F,0.00F,0.02F,0.00F,
			0.00F,0.02F,0.00F,0.00F,
			0.00F,0.00F,0.00F,0.02F
		);
		D3DMATRIX autoTexTfm;
		YsD3dSetMatrix(autoTexTfm,
			(FLOAT)invView[ 0],(FLOAT)invView[ 1],(FLOAT)invView[ 2],(FLOAT)invView[ 3],
			(FLOAT)invView[ 4],(FLOAT)invView[ 5],(FLOAT)invView[ 6],(FLOAT)invView[ 7],
			(FLOAT)invView[ 8],(FLOAT)invView[ 9],(FLOAT)invView[10],(FLOAT)invView[11],
			(FLOAT)invView[12],(FLOAT)invView[13],(FLOAT)invView[14],(FLOAT)invView[15]
		);

		ysD3dDev->d3dDev->SetTransform(D3DTS_TEXTURE0,&texTfm);
		ysD3dDev->d3dDev->MultiplyTransform(D3DTS_TEXTURE0,&autoTexTfm);
		ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2);
		ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_CAMERASPACEPOSITION);

		texUnitPtr->Bind();
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);
		// Setting up texture <<



		int xx,zz,xp,zp;
		int x1,z1,x2,z2;
		const int scaleFactor=2000;
		const int nGrid=6;


		YsVec3 org;
		xx=(int)(pos.x())/scaleFactor;
		zz=(int)(pos.z())/scaleFactor;

		xx*=scaleFactor;
		zz*=scaleFactor;

		for(zp=-nGrid; zp<nGrid; zp++)
		{
			for(xp=-nGrid; xp<=nGrid; xp++)
			{
				x1=xp;
				z1=zp;
				x1=xx+x1*scaleFactor;
				z1=zz+z1*scaleFactor;

				ysD3dDev->AddXyzCol(D3DPT_TRIANGLESTRIP,x1,0,z1,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

				x2=xp;
				z2=zp+1;
				x2=xx+x2*scaleFactor;
				z2=zz+z2*scaleFactor;

				ysD3dDev->AddXyzCol(D3DPT_TRIANGLESTRIP,x2,0,z2,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
			}
			ysD3dDev->FlushXyzCol(D3DPT_TRIANGLESTRIP);
		}



		// Turning off texture >>
		ysD3dDev->d3dDev->SetTexture(0,NULL);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_POINTSPRITEENABLE,FALSE);

		ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
		ysD3dDev->d3dDev->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU);
		// Turning off texture <<
	}
	else
	{
		// Boundary condition:
		//   Out most points: same as ignd
		//
		// Grid sizes:
		//   2^5  2^8  2^11  2^14

		int xx,zz,xp,zp;
		int x1,z1,x2,z2;
		const int scaleFactor=div;
		const int nGrid=6;
		int intensity;

		intensity=YsGreatestOf(ignd.Ri(),ignd.Gi(),ignd.Bi())/8;

		YsVec3 org;
		xx=(int)(pos.x())/scaleFactor;
		zz=(int)(pos.z())/scaleFactor;

		xx*=scaleFactor;
		zz*=scaleFactor;

		for(zp=-nGrid; zp<nGrid; zp++)
		{
			for(xp=-nGrid; xp<=nGrid; xp++)
			{
				int r,g,b;
				YSBOOL boundary;

				x1=xp;
				z1=zp;
				boundary=(x1!=-nGrid && z1!=-nGrid && x1!=nGrid && z1!=nGrid ? YSFALSE : YSTRUE);
				x1=xx+x1*scaleFactor;
				z1=zz+z1*scaleFactor;

				r=ignd.Ri();
				g=ignd.Gi();
				b=ignd.Bi();
				if(boundary!=YSTRUE)
				{
					r=YsBound(r+((x1&31)-16)*intensity/16,0,255);
					g=YsBound(g+((z1&31)-16)*intensity/16,0,255);
					b=YsBound(b+((x1&31)+(z1&31)-32)*intensity/32,0,255);
				}

				ysD3dDev->AddXyzCol(D3DPT_TRIANGLESTRIP,x1,0,z1,r,g,b,255);

				x2=xp;
				z2=zp+1;
				boundary=(x2!=-nGrid && z2!=-nGrid && x2!=nGrid && z2!=nGrid ? YSFALSE : YSTRUE);
				x2=xx+x2*scaleFactor;
				z2=zz+z2*scaleFactor;

				r=ignd.Ri();
				g=ignd.Gi();
				b=ignd.Bi();
				if(boundary!=YSTRUE)
				{
					r=YsBound(r+((x2&31)-16)*intensity/16,0,255);
					g=YsBound(g+((z2&31)-16)*intensity/16,0,255);
					b=YsBound(b+((x2&31)+(z2&31)-32)*intensity/32,0,255);
				}

				ysD3dDev->AddXyzCol(D3DPT_TRIANGLESTRIP,x2,0,z2,r,g,b,255);
			}
			ysD3dDev->FlushXyzCol(D3DPT_TRIANGLESTRIP);
		}
	}

	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	ysD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);

	ysD3dDev->d3dDev->SetRenderState(D3DRS_STENCILFUNC,D3DCMP_ALWAYS);
}

void FsGroundSky::DrawCrappy(const YsVec3 &pos,const YsColor &ignd,const YsColor &isky,const double &farZ,YSBOOL)
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	auto fsD3dDev=dynamic_cast<FsD3dDevice *>(ysD3dDev);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,FALSE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_ALWAYS);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_STENCILWRITEMASK,0);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);


	const double cylRad=5000.0;

	D3DMATRIX pushMatrix,tra; // rot;
	fsD3dDev->d3dDev->GetTransform(D3DTS_WORLD,&pushMatrix);

	YsD3dMakeTranslation(tra,(float)pos.x(),(float)pos.y(),(float)pos.z());
	fsD3dDev->d3dDev->MultiplyTransform(D3DTS_WORLD,&tra);

	if(fsD3dDev->gndSkyGradation==NULL || fsD3dDev->skyCol!=isky || fsD3dDev->gndCol!=ignd || fsD3dDev->gndSkyMode!=2)
	{
		if(fsD3dDev->gndSkyGradation!=NULL)
		{
			fsD3dDev->gndSkyGradation->ReleaseBuffer();
			delete fsD3dDev->gndSkyGradation;
		}

		fsD3dDev->skyCol=isky;
		fsD3dDev->gndCol=ignd;
		fsD3dDev->gndSkyMode=2;

		fsD3dDev->gndSkyGradation=new YsXyzColBuffer;
		fsD3dDev->gndSkyGradation->PrepareBuffer(*fsD3dDev,24);

		fsD3dDev->gndSkyGradation->LockBuffer();

		fsD3dDev->gndSkyGradation->Set( 0, cylRad, 0.0   , 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set( 1, 0.0   , 0.0   ,cylRad ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set( 2, 0.0   ,-cylRad, 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

		fsD3dDev->gndSkyGradation->Set( 3, 0.0   , 0.0   ,cylRad ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set( 4,-cylRad, 0.0   , 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set( 5, 0.0   ,-cylRad, 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

		fsD3dDev->gndSkyGradation->Set( 6,-cylRad, 0.0   , 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set( 7, 0.0   , 0.0   ,-cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set( 8, 0.0   ,-cylRad, 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);

		fsD3dDev->gndSkyGradation->Set( 9, 0.0   , 0.0   ,-cylRad,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(10, cylRad, 0.0   , 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(11, 0.0   ,-cylRad, 0.0   ,ignd.Ri(),ignd.Gi(),ignd.Bi(),255);


		fsD3dDev->gndSkyGradation->Set(12, 0.0   , 0.0   ,cylRad ,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(13, cylRad, 0.0   , 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(14, 0.0   , cylRad, 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);

		fsD3dDev->gndSkyGradation->Set(15,-cylRad, 0.0   , 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(16, 0.0   , 0.0   ,cylRad ,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(17, 0.0   , cylRad, 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);

		fsD3dDev->gndSkyGradation->Set(18, 0.0   , 0.0   ,-cylRad,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(19,-cylRad, 0.0   , 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(20, 0.0   , cylRad, 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);

		fsD3dDev->gndSkyGradation->Set(21, cylRad, 0.0   , 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(22, 0.0   , 0.0   ,-cylRad,isky.Ri(),isky.Gi(),isky.Bi(),255);
		fsD3dDev->gndSkyGradation->Set(23, 0.0   , cylRad, 0.0   ,isky.Ri(),isky.Gi(),isky.Bi(),255);


		fsD3dDev->gndSkyGradation->UnlockBuffer();
		fsD3dDev->gndSkyGradation->nBufUsed=24;
	}

	fsD3dDev->gndSkyGradation->DrawPrimitive(D3DPT_TRIANGLELIST);

	fsD3dDev->d3dDev->SetTransform(D3DTS_WORLD,&pushMatrix);

	fsD3dDev->d3dDev->SetRenderState(D3DRS_STENCILWRITEMASK,0xffffffff);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZENABLE,TRUE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
	fsD3dDev->d3dDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);
}
