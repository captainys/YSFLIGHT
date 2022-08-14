#include <ysclass.h>
#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include "fsconfig.h"

#include "fs.h"
#include "graphics/common/fsopengl.h"


#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>
#include "fsd3d.h"




void FsSimulation::SimDrawBlackout(const ActualViewMode &actualViewMode) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();

	const FsAirplane *playerPlane;

	playerPlane=GetPlayerAirplane();
	int i,j;
	const double plusGLimit=9.0;
	const double minusGLimit=-5.0;

	if(cfgPtr->blackOut==YSTRUE &&
	   actualViewMode.actualViewMode==FSCOCKPITVIEW &&
	   playerPlane!=NULL &&
	   playerPlane->isPlayingRecord!=YSTRUE &&
	   playerPlane->IsAlive()==YSTRUE &&
	   (playerPlane->Prop().GetG()>plusGLimit ||
	    playerPlane->Prop().GetG()<minusGLimit))
	{
		int cenX,cenY,rad;
		double blackness;
		int r,g,b;
		FsD3dDevice *fsD3dDev;

		fsD3dDev=(FsD3dDevice *)ysD3dDev;
		fsD3dDev->line2d->Flush(D3DPT_LINELIST);

		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);

		if(playerPlane->Prop().GetG()>0.0)
		{
			blackness=(playerPlane->Prop().GetG()-plusGLimit)/6.0;
			r=0;
			g=0;
			b=0;
		}
		else
		{
			blackness=(minusGLimit-playerPlane->Prop().GetG())/6.0;
			r=255;
			g=0;
			b=0;
		}


		D3DVIEWPORT9 d3dViewport;

	    ysD3dDev->d3dDev->GetViewport(&d3dViewport);

		cenX=d3dViewport.X+d3dViewport.Width/2;
		cenY=d3dViewport.Y+d3dViewport.Height/2;
		rad=(int)sqrt((double)(YsSqr(d3dViewport.Width)+YsSqr(d3dViewport.Height)));


		ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
		ysD3dDev->d3dDev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);



		double circle[]=
		{
			 1.0       , 0.0,
			 1.4142/2.0, 1.4142/2.0,
			 0.0       , 1.0,
			-1.4142/2.0, 1.4142/2.0,
			-1.0       , 0.0,
			-1.4142/2.0,-1.4142/2.0,
			 0.0       ,-1.0,
			 1.4142/2.0,-1.4142/2.0,
			 1.0       , 0.0
		};


		for(i=0; i<5; i++)  // i==0:Inner circle    smaller i->lighter black
		{
			int iAlpha[2];
			double alpha[2];
			double radius[2];
			alpha[0]=blackness*YsSqr((double)(1+ i   /5.0));
			alpha[1]=blackness*YsSqr((double)(1+(i+1)/5.0));
			radius[0]=(double)(i*rad/5);
			radius[1]=(double)((i+1)*rad/5);

			iAlpha[0]=YsBound((int)(255.0*alpha[0]),0,255);
			iAlpha[1]=YsBound((int)(255.0*alpha[1]),0,255);

			for(j=0; j<=8; j++)
			{
				ysD3dDev->AddXyzCol
				   (D3DPT_TRIANGLESTRIP,cenX+circle[j*2]*radius[0],cenY+circle[j*2+1]*radius[0],0.5,r,g,b,iAlpha[0]);
				ysD3dDev->AddXyzCol
				   (D3DPT_TRIANGLESTRIP,cenX+circle[j*2]*radius[1],cenY+circle[j*2+1]*radius[1],0.5,r,g,b,iAlpha[1]);

				// glColor4d(r,g,b,alpha[0]);
				// glVertex2d(cenX+circle[ j   *2]*radius[0],cenY+circle[ j   *2+1]*radius[0]);
				// glVertex2d(cenX+circle[(j+1)*2]*radius[0],cenY+circle[(j+1)*2+1]*radius[0]);
				// glColor4d(r,g,b,alpha[1]);
				// glVertex2d(cenX+circle[(j+1)*2]*radius[1],cenY+circle[(j+1)*2+1]*radius[1]);
				// glVertex2d(cenX+circle[ j   *2]*radius[1],cenY+circle[ j   *2+1]*radius[1]);
			}
			ysD3dDev->FlushXyzCol(D3DPT_TRIANGLESTRIP);
		}


		YsString str;
		str.Printf("%.1lfG",playerPlane->Prop().GetG());

		int wid,hei;
		FsGetWindowSize(wid,hei);
		FsDrawString(wid/2-20,hei/2,str,YsRed());

		//ysD3dDev->d3dDev->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	}
}

