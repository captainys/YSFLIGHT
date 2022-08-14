#include <fssimplewindow.h>
#include <ysglbuffer.h>

#include "fsblackout.h"



void FsMakeBlackOutPolygon(YsGLVertexBuffer2D &vtxBuf,YsGLColorBuffer &colBuf,const double G)
{
	vtxBuf.CleanUp();
	colBuf.CleanUp();

	const double plusGLimit=9.0f;
	const double minusGLimit=-5.0f;

	if(G>plusGLimit || G<minusGLimit)
	{
		int cenX,cenY,rad;
		double blackness,r,g,b;

		if(G>0.0)
		{
			blackness=(G-plusGLimit)/6.0f;
			r=0.0f;
			g=0.0f;
			b=0.0f;
		}
		else
		{
			blackness=(minusGLimit-G)/6.0f;
			r=1.0f;
			g=0.0f;
			b=0.0f;
		}


		int wid,hei;
		FsGetWindowSize(wid,hei);

		cenX=wid/2;
		cenY=hei/2;
		rad=(int)sqrt((double)(wid*wid+hei*hei));



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

		for(int i=0; i<5; i++)  // i==0:Inner circle    smaller i->lighter black
		{
			double alpha[2]=
			{
				blackness*YsSqr((double)(1+ i   /5.0)),
				blackness*YsSqr((double)(1+(i+1)/5.0))
			};
			const double radius[2]=
			{
				(double)(i*rad/5),
				(double)((i+1)*rad/5)
			};

			alpha[0]=YsBound(alpha[0],0.0,1.0);
			alpha[1]=YsBound(alpha[1],0.0,1.0);

			for(int j=0; j<8; j++)
			{
				const YsVec2 quad[4]=
				{
					YsVec2(cenX+circle[ j   *2]*radius[0],cenY+circle[ j   *2+1]*radius[0]),
					YsVec2(cenX+circle[(j+1)*2]*radius[0],cenY+circle[(j+1)*2+1]*radius[0]),
					YsVec2(cenX+circle[(j+1)*2]*radius[1],cenY+circle[(j+1)*2+1]*radius[1]),
					YsVec2(cenX+circle[ j   *2]*radius[1],cenY+circle[ j   *2+1]*radius[1])
				};

				colBuf.Add(r,g,b,alpha[0]);
				vtxBuf.Add(quad[0]);
				colBuf.Add(r,g,b,alpha[0]);
				vtxBuf.Add(quad[1]);
				colBuf.Add(r,g,b,alpha[1]);
				vtxBuf.Add(quad[2]);

				colBuf.Add(r,g,b,alpha[1]);
				vtxBuf.Add(quad[2]);
				colBuf.Add(r,g,b,alpha[1]);
				vtxBuf.Add(quad[3]);
				colBuf.Add(r,g,b,alpha[0]);
				vtxBuf.Add(quad[0]);
			}
		}
	}
}


