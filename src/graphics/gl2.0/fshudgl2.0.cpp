#include <ysclass.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"

#include <ysgl.h>
#include <ysglsldrawfontbitmap.h>

#include "fsgl2.0util.h"
#include "fsopengl2.0.h"

void FsHeadUpDisplay::DrawVelocityVectorIndicator(const YsVec3 &viewPos,const YsAtt3 &viewAtt,const YsVec3 &v)
{
	YsVec3 vel,target,ev,uv,rv,p1,p2;
	ev=viewAtt.GetForwardVector();
	uv=viewAtt.GetUpVector();
	rv=ev^uv;

	glDisable(GL_DEPTH_TEST);


	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();

	const GLfloat color[4]={hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f};

	YsGLSLUse3DRenderer(renderer);
	YsGLSLSet3DRendererUniformColorfv(renderer,color);

	vel=v;

	viewAtt.MulInverse(vel,vel);
	if(vel.z()>YsTolerance)
	{
		vel.DivX(vel.z());
		vel.MulX(5.0);
		vel.DivY(vel.z());
		vel.MulY(5.0);
		vel.SetZ(5.0);
		viewAtt.Mul(vel,vel);
		target=viewPos+vel;

		const double rad=0.1;

		GLfloat circleVtx[12*3];
		for(int a=0; a<12; ++a)
		{
			const int i=a*30;
			const YsVec3 p=(target+(rv*trigonomyTable[i*2]+uv*trigonomyTable[i*2+1])*rad);
			circleVtx[a*3  ]=(GLfloat)p.x();
			circleVtx[a*3+1]=(GLfloat)p.y();
			circleVtx[a*3+2]=(GLfloat)p.z();
		}
		YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINE_LOOP,12,circleVtx);

		int nLineVtx=0;
		GLfloat lineVtx[6*3];

		p1=target+rv*rad;
		p2=target+rv*rad*1.8;
		FsGLAddVertex3(nLineVtx,lineVtx,(GLfloat)p1.x(),(GLfloat)p1.y(),(GLfloat)p1.z());
		FsGLAddVertex3(nLineVtx,lineVtx,(GLfloat)p2.x(),(GLfloat)p2.y(),(GLfloat)p2.z());

		p1=target-rv*rad;
		p2=target-rv*rad*1.8;
		FsGLAddVertex3(nLineVtx,lineVtx,(GLfloat)p1.x(),(GLfloat)p1.y(),(GLfloat)p1.z());
		FsGLAddVertex3(nLineVtx,lineVtx,(GLfloat)p2.x(),(GLfloat)p2.y(),(GLfloat)p2.z());

		p1=target+uv*rad;
		p2=target+uv*rad*1.8;
		FsGLAddVertex3(nLineVtx,lineVtx,(GLfloat)p1.x(),(GLfloat)p1.y(),(GLfloat)p1.z());
		FsGLAddVertex3(nLineVtx,lineVtx,(GLfloat)p2.x(),(GLfloat)p2.y(),(GLfloat)p2.z());

		YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,6,lineVtx);
	}

	glEnable(GL_DEPTH_TEST);
	YsGLSLEndUse3DRenderer(renderer);
}

void FsHeadUpDisplay::DrawCircleContainer
	   (const YsMatrix4x4 &viewpoint,
	    const YsAtt3 &viewAtt,
	    const YsVec3 &target,
	    const YsVec3 &from,
	    const YsColor &col,
	    const char caption[],
	    const char caption2[],
	    YSBOOL dot,
	    int begin,int step)
{
	int i;
	YsVec3 p1,p2;
	YsVec3 rel,tgt;

	rel=viewpoint*target;
	if(rel.z()>5000.0)
	{
		rel.MulX(5000.0/rel.z());
		rel.MulY(5000.0/rel.z());
		rel.SetZ(5000.0);
		viewpoint.MulInverse(tgt,rel,1.0);
	}
	else
	{
		tgt=target;
	}


	glDisable(GL_DEPTH_TEST);


	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat prevTfm[16];
	YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);

	GLfloat tfm[16];
	YsGLCopyMatrixfv(tfm,prevTfm);
	YsGLMultMatrixTranslationfv(tfm,(GLfloat)tgt.x(),(GLfloat)tgt.y(),(GLfloat)tgt.z());
	YsGLMultMatrixRotationXZfv(tfm,(GLfloat)viewAtt.h());
	YsGLMultMatrixRotationZYfv(tfm,(GLfloat)viewAtt.p());
	YsGLMultMatrixRotationXYfv(tfm,(GLfloat)viewAtt.b());


	YsGLSLSet3DRendererModelViewfv(renderer,tfm);

	const GLfloat color[4]={col.Rf(),col.Gf(),col.Bf(),1.0f};
	YsGLSLSet3DRendererUniformColorfv(renderer,color);

	int nVtx=0;
	GLfloat vtx[360*3];
	for(i=begin; i<=begin+360; i+=step)
	{
		vtx[nVtx*3  ]=(GLfloat)(rel.z()/18.0*trigonomyTable[i*2]);
		vtx[nVtx*3+1]=(GLfloat)(rel.z()/18.0*trigonomyTable[i*2+1]);
		vtx[nVtx*3+2]=0.0f;
		++nVtx;
	}
	YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINE_LOOP,nVtx,vtx);

	YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);
	if(dot==YSTRUE)
	{
		GLfloat vtx[3]={(GLfloat)tgt.x(),(GLfloat)tgt.y(),(GLfloat)tgt.z()};
		YsGLSLDrawPrimitiveVtxfv(renderer,GL_POINTS,1,vtx);
	}

	if(tgt!=from)
	{
		GLfloat vtx[6]=
		{
			(GLfloat)tgt.x(),(GLfloat)tgt.y(),(GLfloat)tgt.z(),
			(GLfloat)from.x(),(GLfloat)from.y(),(GLfloat)from.z(),
		};
		YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,2,vtx);
	}

	YsGLSLEndUse3DRenderer(renderer);



	auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
	YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
	YsGLSLSetBitmapFontRendererModelViewfv(fsBitmapFontRenderer,tfm);
	YsGLSLSetBitmapFontRendererColor3f(fsBitmapFontRenderer,color[0],color[1],color[2]);

	YsGLSLRenderBitmapFontString3D(fsBitmapFontRenderer,(GLfloat)rel.z()/18.0f,-(GLfloat)rel.z()/18.0f,0.0f,caption);
	if(caption2!=NULL)
	{
		YsGLSLRenderBitmapFontString3D(fsBitmapFontRenderer,(GLfloat)rel.z()/18.0f,(GLfloat)rel.z()/18.0f,0.0f,caption2);
	}

	YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);

	glEnable(GL_DEPTH_TEST);
}


void FsHeadUpDisplay::DrawCrossDesignator
	   (const YsMatrix4x4 &viewpoint,
	    const YsAtt3 &viewAtt,
	    const YsVec3 &target,
	    const YsVec3 &from,
	    const YsColor &col,
	    YSBOOL dot)
{
	YsVec3 p1,p2;
	YsVec3 rel,tgt;

	rel=viewpoint*target;
	if(rel.z()>5000.0)
	{
		rel.MulX(5000.0/rel.z());
		rel.MulY(5000.0/rel.z());
		rel.SetZ(5000.0);
		viewpoint.MulInverse(tgt,rel,1.0);
	}
	else
	{
		tgt=target;
	}


	glDisable(GL_DEPTH_TEST);


	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat prevTfm[16];
	YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);

	GLfloat tfm[16];
	YsGLCopyMatrixfv(tfm,prevTfm);
	YsGLMultMatrixTranslationfv(tfm,(GLfloat)tgt.x(),(GLfloat)tgt.y(),(GLfloat)tgt.z());
	YsGLMultMatrixRotationXZfv(tfm,(GLfloat)viewAtt.h());
	YsGLMultMatrixRotationZYfv(tfm,(GLfloat)viewAtt.p());
	YsGLMultMatrixRotationXYfv(tfm,(GLfloat)viewAtt.b());


	YsGLSLSet3DRendererModelViewfv(renderer,tfm);

	const GLfloat color[4]={col.Rf(),col.Gf(),col.Bf(),1.0f};
	YsGLSLSet3DRendererUniformColorfv(renderer,color);


	const GLfloat vtx[8*3]=
	{
		(GLfloat)rel.z()/32.0f,0.0f,0.0f,
		(GLfloat)rel.z()/96.0f,0.0f,0.0f,

		-(GLfloat)rel.z()/32.0f,0.0f,0.0f,
		-(GLfloat)rel.z()/96.0f,0.0f,0.0f,

		0.0f,(GLfloat)rel.z()/32.0f,0.0f,
		0.0f,(GLfloat)rel.z()/96.0f,0.0f,

		0.0f,-(GLfloat)rel.z()/32.0f,0.0f,
		0.0f,-(GLfloat)rel.z()/96.0f,0.0f,
	};
	YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,8,vtx);

	YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);

	if(dot==YSTRUE)
	{
		const GLfloat vtx[3]={(GLfloat)tgt.x(),(GLfloat)tgt.y(),(GLfloat)tgt.z()};
		YsGLSLDrawPrimitiveVtxfv(renderer,GL_POINTS,1,vtx);
	}

	YsGLSLEndUse3DRenderer(renderer);

	glEnable(GL_DEPTH_TEST);
}


void FsHeadUpDisplay::DrawCrossDesignator2
	   (const YsMatrix4x4 &viewpoint,
	    const YsAtt3 &viewAtt,
	    const YsVec3 &target,
	    const YsVec3 &from,
	    const YsColor &col,
	    YSBOOL dot)
{
	YsVec3 p1,p2;
	YsVec3 rel,tgt;

	rel=viewpoint*target;
	if(rel.z()>5000.0)
	{
		rel.MulX(5000.0/rel.z());
		rel.MulY(5000.0/rel.z());
		rel.SetZ(5000.0);
		viewpoint.MulInverse(tgt,rel,1.0);
	}
	else
	{
		tgt=target;
	}



	glDisable(GL_DEPTH_TEST);


	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat prevTfm[16];
	YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);

	GLfloat tfm[16];
	YsGLCopyMatrixfv(tfm,prevTfm);
	YsGLMultMatrixTranslationfv(tfm,(GLfloat)tgt.x(),(GLfloat)tgt.y(),(GLfloat)tgt.z());
	YsGLMultMatrixRotationXZfv(tfm,(GLfloat)viewAtt.h());
	YsGLMultMatrixRotationZYfv(tfm,(GLfloat)viewAtt.p());
	YsGLMultMatrixRotationXYfv(tfm,(GLfloat)viewAtt.b());


	YsGLSLSet3DRendererModelViewfv(renderer,tfm);

	const GLfloat color[4]={col.Rf(),col.Gf(),col.Bf(),1.0f};
	YsGLSLSet3DRendererUniformColorfv(renderer,color);



	const GLfloat l1=(GLfloat)rel.z()/32.0f;
	const GLfloat l2=(GLfloat)rel.z()/96.0f;

	const GLfloat vtx[8*3]=
	{
		l1,l1,0.0f,
		l2,l2,0.0f,

		-l1,l1,0.0f,
		-l2,l2,0.0f,

		l1,-l1,0.0f,
		l2,-l2,0.0f,

		-l1,-l1,0.0f,
		-l2,-l2,0.0f,
	};
	YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,8,vtx);


	YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);

	if(dot==YSTRUE)
	{
		const GLfloat vtx[3]={(GLfloat)tgt.x(),(GLfloat)tgt.y(),(GLfloat)tgt.z()};
		YsGLSLDrawPrimitiveVtxfv(renderer,GL_POINTS,1,vtx);
	}

	YsGLSLEndUse3DRenderer(renderer);

	glEnable(GL_DEPTH_TEST);
}



void FsHeadUpDisplay::DrawCrossHair(void)
{
	const GLfloat cenX=(GLfloat)(lupX+wid/2);
	const GLfloat cenY=(GLfloat)(lupY+hei/2);

	const GLfloat lat=(GLfloat)(wid/20);
	const GLfloat lng=(GLfloat)(lat*2/3);

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	const GLfloat colorBuf[8*4]=
	{
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f
	};
	const GLfloat vertexBuf[8*2]=
	{
		(GLfloat)(cenX-lat),(GLfloat)cenY,
		(GLfloat)(cenX-4  ),(GLfloat)cenY,

		(GLfloat)(cenX+lat),(GLfloat)cenY,
		(GLfloat)(cenX+4  ),(GLfloat)cenY,

		(GLfloat)cenX,      (GLfloat)(cenY-lng),
		(GLfloat)cenX,      (GLfloat)(cenY-4),

		(GLfloat)cenX,      (GLfloat)(cenY+lng),
		(GLfloat)cenX,      (GLfloat)(cenY+4)
	};

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,8,vertexBuf,colorBuf);

	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawHeading(const YsAtt3 &a,YSBOOL showHdgBug,const double &hdgBug,YSBOOL selected)
{
	long cenX,cenY;
	long deg,num;
	int s1x,s1y,s2x,s2y;
	double hdg,interval;

	cenX=lupX+wid/2;
	cenY=lupY+hei/6;

	interval=(double)wid/64;

	hdg=-YsRadToDeg(a.h());
	deg= (long)(hdg-12.0F);


	const int nVtxPerBunch=32;
	GLfloat vtxBuf[nVtxPerBunch*2];
	GLfloat colBuf[nVtxPerBunch*4];
	int nVtxBufUsed=0;


	vtxBuf[0]=(GLfloat)cenX;
	vtxBuf[1]=(GLfloat)(cenY+1);
	vtxBuf[2]=(GLfloat)cenX;
	vtxBuf[3]=(GLfloat)(cenY+8);
	nVtxBufUsed=2;

	colBuf[0]=hudCol.Rf();
	colBuf[1]=hudCol.Gf();
	colBuf[2]=hudCol.Bf();
	colBuf[3]=1.0f;
	colBuf[4]=hudCol.Rf();
	colBuf[5]=hudCol.Gf();
	colBuf[6]=hudCol.Bf();
	colBuf[7]=1.0f;

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	s1y=cenY;
	while(deg<(long)hdg+12)
	{
		s2y=(deg%10==0 ? cenY-8 : cenY-2);

		s1x=cenX+(long)(((double)deg-hdg)*interval);
		s2x=s1x;

		vtxBuf[nVtxBufUsed*2  ]=(GLfloat)s1x;
		vtxBuf[nVtxBufUsed*2+1]=(GLfloat)s1y;
		vtxBuf[nVtxBufUsed*2+2]=(GLfloat)s2x;
		vtxBuf[nVtxBufUsed*2+3]=(GLfloat)s2y;

		colBuf[nVtxBufUsed*4  ]=hudCol.Rf();
		colBuf[nVtxBufUsed*4+1]=hudCol.Gf();
		colBuf[nVtxBufUsed*4+2]=hudCol.Bf();
		colBuf[nVtxBufUsed*4+3]=1.0f;
		colBuf[nVtxBufUsed*4+4]=hudCol.Rf();
		colBuf[nVtxBufUsed*4+5]=hudCol.Gf();
		colBuf[nVtxBufUsed*4+6]=hudCol.Bf();
		colBuf[nVtxBufUsed*4+7]=1.0f;

		nVtxBufUsed+=2;
		if(nVtxPerBunch-4<nVtxBufUsed)
		{
			YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf,colBuf);
			nVtxBufUsed=0;
		}

		deg++;
	}
	if(0<nVtxBufUsed)
	{
		YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf,colBuf);
		nVtxBufUsed=0;
	}

	if(YSTRUE==showHdgBug)
	{
		double relHdgBug=-YsRadToDeg(hdgBug-a.h());
		while(relHdgBug<-180.0)
		{
			relHdgBug+=360.0;
		}
		while(180.0<relHdgBug)
		{
			relHdgBug-=360.0;
		}

		relHdgBug=YsBound(relHdgBug,-12.0,12.0);
		int cx=cenX+(long)(relHdgBug*interval);
		int cy=cenY;

		if(YSTRUE!=selected)
		{
			const GLfloat vtxBuf[7*2]=
			{
				(GLfloat)(cx-12),(GLfloat)(cy-10),
				(GLfloat)(cx-12),(GLfloat)(cy),
				(GLfloat)(cx-4) ,(GLfloat)(cy),
				(GLfloat)(cx)   ,(GLfloat)(cy+4),
				(GLfloat)(cx+4) ,(GLfloat)(cy),
				(GLfloat)(cx+12),(GLfloat)(cy),
				(GLfloat)(cx+12),(GLfloat)(cy-10)
			};
			const GLfloat colBuf[7*4]=
			{
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f
			};
			YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINE_LOOP,7,vtxBuf,colBuf);
		}
		else
		{
			const GLfloat vtxBuf[8*2]=
			{
				(GLfloat)(cx),   (GLfloat)(cy-10),
				(GLfloat)(cx-12),(GLfloat)(cy-10),
				(GLfloat)(cx-12),(GLfloat)(cy),
				(GLfloat)(cx-4), (GLfloat)(cy),
				(GLfloat)(cx),   (GLfloat)(cy+4),
				(GLfloat)(cx+4), (GLfloat)(cy),
				(GLfloat)(cx+12),(GLfloat)(cy),
				(GLfloat)(cx+12),(GLfloat)(cy-10)
			};
			const GLfloat colBuf[8*4]=
			{
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
				hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f
			};
			YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_TRIANGLE_FAN,8,vtxBuf,colBuf);
		}
	}

	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());



	deg= (long)(hdg-12.0F);
	s1y=cenY;
	while(deg<(long)hdg+12)
	{
		s2y=(deg%10==0 ? cenY-8 : cenY-2);

		s1x=cenX+(long)(((double)deg-hdg)*interval);
		s2x=s1x;

		if(deg%10==0)
		{
			s2y=cenY-8;

			num=(deg>0 ? deg/10 : (deg+360)/10);
			char str[3];
			str[0]=(char)('0'+num/10);
			str[1]=(char)('0'+num%10);
			str[2]=0;

			FsDrawString(s2x-12,s2y-12,str,hudCol);
		}

		++deg;
	}

	if(YSTRUE==showHdgBug)
	{
		double relHdgBug=-YsRadToDeg(hdgBug-a.h());
		while(relHdgBug<-180.0)
		{
			relHdgBug+=360.0;
		}
		while(180.0<relHdgBug)
		{
			relHdgBug-=360.0;
		}

		relHdgBug=YsBound(relHdgBug,-12.0,12.0);
		int cx=cenX+(long)(relHdgBug*interval);
		int cy=cenY;

		YsString bugTxt;
		int digit=(int)YsRadToDeg(-hdgBug);
		while(digit<0)
		{
			digit+=360;
		}
		while(360<digit)
		{
			digit-=360;
		}
		bugTxt.Printf("%03d",digit);
		FsDrawString(cx-14,cy+20,bugTxt,hudCol);
	}
}

void FsHeadUpDisplay::DrawThrottle(int nEng,const double thr[],const YSBOOL ab[])
{
	int i;
	YSBOOL abr;
	long cenX,cenY,sizX,sizY,barY;
	double deg;



	cenX=lupX+wid/12;
	cenY=lupY+hei*10/12;

	sizX=wid/64;
	sizY=wid/16;

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	for(i=0; i<nEng; i++)
	{
		deg=thr[i];
		abr=ab[i];

		barY=(long)(((double)sizY)*deg);

		GLfloat r,g,b;

		if(abr==YSTRUE)
		{
			r=1.0f;
			g=0.0f;
			b=0.0f;
		}
		else
		{
			r=hudCol.Rf();
			g=hudCol.Gf();
			b=hudCol.Bf();
		}

		const GLfloat colBuf[4*4]=
		{
			r,g,b,1.0f,
			r,g,b,1.0f,
			r,g,b,1.0f,
			r,g,b,1.0f
		};
		const GLfloat barVtxBuf[4*2]=
		{
			(GLfloat)(cenX+1)     ,(GLfloat)(cenY),
			(GLfloat)(cenX+sizX-1),(GLfloat)(cenY),
			(GLfloat)(cenX+sizX-1),(GLfloat)(cenY-barY),
			(GLfloat)(cenX+1)     ,(GLfloat)(cenY-barY),
		};
		YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_TRIANGLE_FAN,4,barVtxBuf,colBuf);

		const GLfloat frameVtxBuf[4*2]=
		{
			(GLfloat)(cenX+1)     ,(GLfloat)(cenY),
			(GLfloat)(cenX+sizX-1),(GLfloat)(cenY),
			(GLfloat)(cenX+sizX-1),(GLfloat)(cenY-sizY),
			(GLfloat)(cenX+1)     ,(GLfloat)(cenY-sizY),
		};

		YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINE_LOOP,4,frameVtxBuf,colBuf);
		YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_POINTS,4,frameVtxBuf,colBuf);

		cenX+=sizX;
	}

	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawNozzle(const YsVec3 &vec)
{
	long cenX,cenY,sizX,dirX,dirY;

	cenX=lupX;
	cenY=lupY+hei*10/12-wid/64;

	sizX=wid/32;

	FsDrawString(cenX-24,cenY,"NZL",hudCol);

	const GLfloat colBuf[4*4]=
	{
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
	};


	dirX=int(vec.z()*double(sizX));
	dirY=int(vec.y()*double(sizX));

	const GLfloat vtxBuf[4*2]=
	{
		(GLfloat)(cenX)     ,     (GLfloat)(cenY),
		(GLfloat)(cenX+sizX),     (GLfloat)(cenY),
		(GLfloat)(cenX+sizX)     ,(GLfloat)(cenY),
		(GLfloat)(cenX+sizX+dirX),(GLfloat)(cenY+dirY)
	};

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,4,vtxBuf,colBuf);
	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawElevator(double elv,double trim,YSBOOL au)
{
	long cenX,cenY,sizX,sizY;

	if(elv>=0.0)
	{
		elv=sqrt(elv);
	}
	else
	{
		elv=-sqrt(-elv);
	}

	if(trim>=0.0)
	{
		trim=sqrt(trim);
	}
	else
	{
		trim=-sqrt(-trim);
	}

	cenX=lupX+wid*2/12;
	cenY=lupY+hei*10/12;

	sizX=wid/48;
	sizY=wid/16;

	cenY-=sizY/2;

	int s1x,s1y,s2x,s2y;
	s1x=cenX+sizX;
	s1y=cenY;
	s2x=cenX+sizX*2;
	s2y=cenY;


	const GLfloat pointColBuf[4*2]=
	{
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
	};


	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	const GLfloat pointVtxBuf[2*2]=
	{
		(GLfloat)(s1x),(GLfloat)(s1y),
		(GLfloat)(s2x),(GLfloat)(s2y)
	};
	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_POINTS,2,pointVtxBuf,pointColBuf);



	GLfloat auColor[3];
	if(au==YSTRUE)
	{
		auColor[0]=1.0f;
		auColor[1]=1.0f;
		auColor[2]=0.0f;
	}
	else
	{
		auColor[0]=hudCol.Rf();
		auColor[1]=hudCol.Gf();
		auColor[2]=hudCol.Bf();
	}

	const GLfloat colBuf[6*4]=
	{
		auColor[0],auColor[1],auColor[2],1.0f,
		auColor[0],auColor[1],auColor[2],1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
	};

	GLfloat vtxBuf[6*2];

	s1y=cenY-(long)((double)sizY*elv/2.0);
	s2y=s1y;
	vtxBuf[0]=(GLfloat)s1x;
	vtxBuf[1]=(GLfloat)s1y;
	vtxBuf[2]=(GLfloat)s2x;
	vtxBuf[3]=(GLfloat)s2y;

	s1y=cenY-(long)(trim*double(sizY)/2.0);
	s2y=s1y;
	vtxBuf[4]=(GLfloat)s1x;
	vtxBuf[5]=(GLfloat)s1y;
	vtxBuf[6]=(GLfloat)s2x;
	vtxBuf[7]=(GLfloat)s2y;

	s1x=cenX+sizX*3/2;
	s1y=cenY-(long)(trim*double(sizY)/2.0);
	s2x=s1x;
	s2y=cenY;
	vtxBuf[ 8]=(GLfloat)s1x;
	vtxBuf[ 9]=(GLfloat)s1y;
	vtxBuf[10]=(GLfloat)s2x;
	vtxBuf[11]=(GLfloat)s2y;

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,6,vtxBuf,colBuf);

	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawAileron(double ail,YSBOOL au)
{
	long cenX,cenY,sizX,sizY;
	int s1x,s1y,s2x,s2y;

	GLfloat vtxBuf[4];
	GLfloat colBuf[8];

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	if(ail>=0.0)
	{
		ail=sqrt(ail);
	}
	else
	{
		ail=-sqrt(-ail);
	}

	cenX=lupX+wid*2/12;
	cenY=lupY+hei*10/12;

	sizX=wid/48;
	sizY=wid/16;

	cenY-=sizY/2;

	s1x=cenX;
	s1y=cenY;
	s2x=cenX+sizX;
	s2y=cenY;

	colBuf[0]=hudCol.Rf();
	colBuf[1]=hudCol.Gf();
	colBuf[2]=hudCol.Bf();
	colBuf[3]=1.0f;
	colBuf[4]=hudCol.Rf();
	colBuf[5]=hudCol.Gf();
	colBuf[6]=hudCol.Bf();
	colBuf[7]=1.0f;

	vtxBuf[0]=(GLfloat)s1x;
	vtxBuf[1]=(GLfloat)s1y;
	vtxBuf[2]=(GLfloat)s2x;
	vtxBuf[3]=(GLfloat)s2y;

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_POINTS,2,vtxBuf,colBuf);

	if(au==YSTRUE)
	{
		colBuf[0]=1.0f;
		colBuf[1]=1.0f;
		colBuf[2]=0.0f;
		colBuf[3]=1.0f;
		colBuf[4]=1.0f;
		colBuf[5]=1.0f;
		colBuf[6]=0.0f;
		colBuf[7]=1.0f;
	}
	else
	{
		colBuf[0]=hudCol.Rf();
		colBuf[1]=hudCol.Gf();
		colBuf[2]=hudCol.Bf();
		colBuf[3]=1.0f;
		colBuf[4]=hudCol.Rf();
		colBuf[5]=hudCol.Gf();
		colBuf[6]=hudCol.Bf();
		colBuf[7]=1.0f;
	}

	s1y=cenY-(long)((double)sizY*ail/2.0F);
	s2y=s1y;

	vtxBuf[0]=(GLfloat)s1x;
	vtxBuf[1]=(GLfloat)s1y;
	vtxBuf[2]=(GLfloat)s2x;
	vtxBuf[3]=(GLfloat)s2y;

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,2,vtxBuf,colBuf);

	s1x=cenX+sizX*2;
	s1y=cenY;
	s2x=cenX+sizX*3;
	s2y=cenY;

	colBuf[0]=hudCol.Rf();
	colBuf[1]=hudCol.Gf();
	colBuf[2]=hudCol.Bf();
	colBuf[3]=1.0f;
	colBuf[4]=hudCol.Rf();
	colBuf[5]=hudCol.Gf();
	colBuf[6]=hudCol.Bf();
	colBuf[7]=1.0f;

	vtxBuf[0]=(GLfloat)s1x;
	vtxBuf[1]=(GLfloat)s1y;
	vtxBuf[2]=(GLfloat)s2x;
	vtxBuf[3]=(GLfloat)s2y;

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_POINTS,2,vtxBuf,colBuf);

	if(au==YSTRUE)
	{
		colBuf[0]=1.0f;
		colBuf[1]=1.0f;
		colBuf[2]=0.0f;
		colBuf[3]=1.0f;
		colBuf[4]=1.0f;
		colBuf[5]=1.0f;
		colBuf[6]=0.0f;
		colBuf[7]=1.0f;
	}
	else
	{
		colBuf[0]=hudCol.Rf();
		colBuf[1]=hudCol.Gf();
		colBuf[2]=hudCol.Bf();
		colBuf[3]=1.0f;
		colBuf[4]=hudCol.Rf();
		colBuf[5]=hudCol.Gf();
		colBuf[6]=hudCol.Bf();
		colBuf[7]=1.0f;
	}

	s1y=cenY+(long)((double)sizY*ail/2.0F);
	s2y=s1y;

	vtxBuf[0]=(GLfloat)s1x;
	vtxBuf[1]=(GLfloat)s1y;
	vtxBuf[2]=(GLfloat)s2x;
	vtxBuf[3]=(GLfloat)s2y;

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,2,vtxBuf,colBuf);

	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawRudder(double rud)
{
	long cenX,cenY,sizX;

	GLfloat vtxBuf[8];
	const GLfloat colBuf[16]=
	{
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
	};

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	cenX=lupX+wid*2/12;
	cenY=lupY+hei*10/12;

	sizX=wid/16;

	cenX+=sizX/2;

	vtxBuf[0]=(GLfloat)cenX;
	vtxBuf[1]=(GLfloat)(cenY+2);
	vtxBuf[2]=(GLfloat)cenX;
	vtxBuf[3]=(GLfloat)(cenY+8);

	sizX=(long)((double)sizX*rud);
	vtxBuf[4]=(GLfloat)(cenX-sizX);
	vtxBuf[5]=(GLfloat)(cenY+2);
	vtxBuf[6]=(GLfloat)(cenX-sizX);
	vtxBuf[7]=(GLfloat)(cenY+8+14);

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,4,vtxBuf,colBuf);

	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawSpeed(const double &spd)
{
	long cenX,cenY;
	long i,deg;
	double y;


	cenX=lupX+wid/5;
	cenY=lupY+hei/2;

	int s1x,s1y,s2x,s2y;
	s1x=cenX;
	s1y=cenY;
	s2x=cenX+4;
	s2y=cenY;

	double interval;
	interval=(double)wid/64;


	const int nVtxPerBunch=32;
	GLfloat vtxBuf[nVtxPerBunch*2];
	GLfloat colBuf[nVtxPerBunch*4];
	int nVtxBufUsed=0;

	for(int i=0; i<nVtxPerBunch; ++i)
	{
		colBuf[i*4  ]=hudCol.Rf();
		colBuf[i*4+1]=hudCol.Gf();
		colBuf[i*4+2]=hudCol.Bf();
		colBuf[i*4+3]=1.0f;
	}

	vtxBuf[nVtxBufUsed*2  ]=(GLfloat)s1x;
	vtxBuf[nVtxBufUsed*2+1]=(GLfloat)s1y;
	vtxBuf[nVtxBufUsed*2+2]=(GLfloat)s2x;
	vtxBuf[nVtxBufUsed*2+3]=(GLfloat)s2y;
	nVtxBufUsed+=2;

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	s1x=cenX-5;
	s2x=cenX-1;
	for(i=-10; i<=10; i++)
	{
		deg=(long)spd+i;

		if(deg>=0)
		{
			y=(spd-(double)deg)*interval;
			s1y=(long)(cenY+y);
			s2y=(long)(cenY+y);

			s1x=cenX-2;

			vtxBuf[nVtxBufUsed*2  ]=(GLfloat)s1x;
			vtxBuf[nVtxBufUsed*2+1]=(GLfloat)s1y;
			vtxBuf[nVtxBufUsed*2+2]=(GLfloat)s2x;
			vtxBuf[nVtxBufUsed*2+3]=(GLfloat)s2y;
			nVtxBufUsed+=2;

			if(nVtxPerBunch-4<=nVtxBufUsed)
			{
				YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf,colBuf);
				nVtxBufUsed=0;
			}
		}
	}

	if(0<nVtxBufUsed)
	{
		YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf,colBuf);
		nVtxBufUsed=0;
	}
	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());


	s1x=cenX-5;
	s2x=cenX-1;
	for(i=-10; i<=10; i++)
	{
		deg=(long)spd+i;

		if(deg>=0)
		{
			y=(spd-(double)deg)*interval;
			s1y=(long)(cenY+y);
			s2y=(long)(cenY+y);

			if(deg%10==0)
			{
				s1x=cenX-5;

				char str[16];
				sprintf(str,"%-4d",(int)deg);
				FsDrawString(s1x-40,s1y,str,hudCol);
			}
		}
	}
}

void FsHeadUpDisplay::DrawAltitude(double alt)
{
	long cenX,cenY;
	long i,deg;
	double y;
	int s1x,s1y,s2x,s2y;

	const int nVtxPerBunch=32;
	GLfloat vtxBuf[nVtxPerBunch*2];
	GLfloat colBuf[nVtxPerBunch*4];
	int nVtxBufUsed=0;

	for(int i=0; i<nVtxPerBunch; ++i)
	{
		colBuf[i*4  ]=hudCol.Rf();
		colBuf[i*4+1]=hudCol.Gf();
		colBuf[i*4+2]=hudCol.Bf();
		colBuf[i*4+3]=1.0f;
	}

	cenX=lupX+wid*4/5;
	cenY=lupY+hei/2;

	s1x=cenX;
	s1y=cenY;
	s2x=cenX-4;
	s2y=cenY;

	vtxBuf[nVtxBufUsed*2  ]=(GLfloat)s1x;
	vtxBuf[nVtxBufUsed*2+1]=(GLfloat)s1y;
	vtxBuf[nVtxBufUsed*2+2]=(GLfloat)s2x;
	vtxBuf[nVtxBufUsed*2+3]=(GLfloat)s2y;
	nVtxBufUsed+=2;


	double interval;
	interval=(double)wid/64;


	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	alt/=10.0F;

	s1x=cenX+5;
	s2x=cenX+1;
	for(i=-10; i<=10; i++)
	{
		deg=(long)alt+i;

		if(deg>=0)
		{
			y=(alt-(double)deg)*interval;
			s1y=(long)(cenY+y);
			s2y=(long)(cenY+y);

			s1x=cenX+2;
			vtxBuf[nVtxBufUsed*2  ]=(GLfloat)s1x;
			vtxBuf[nVtxBufUsed*2+1]=(GLfloat)s1y;
			vtxBuf[nVtxBufUsed*2+2]=(GLfloat)s2x;
			vtxBuf[nVtxBufUsed*2+3]=(GLfloat)s2y;
			nVtxBufUsed+=2;

			if(nVtxPerBunch-4<=nVtxBufUsed)
			{
				YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf,colBuf);
				nVtxBufUsed=0;
			}
		}
	}
	if(0<nVtxBufUsed)
	{
		YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf,colBuf);
		nVtxBufUsed=0;
	}
	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());



	s1x=cenX+5;
	s2x=cenX+1;
	for(i=-10; i<=10; i++)
	{
		deg=(long)alt+i;

		if(deg>=0)
		{
			y=(alt-(double)deg)*interval;
			s1y=(long)(cenY+y);
			s2y=(long)(cenY+y);

			if(deg%10==0)
			{
				s1x=cenX+5;

				char str[16];
				sprintf(str,"%-5d",(int)deg*10);
				FsDrawString(s1x+6,s1y,str,hudCol);
			}
		}
	}
}

void FsHeadUpDisplay::DrawClimbRatio(const double &climbRatio)
{
	int i,y,d;
	char str[128];
	int sx,sy,s1x,s1y,s2x,s2y;

	sx=lupX+wid*11/12;
	sy=lupY+hei*2/12;

	FsDrawString(sx,sy-FONTPITCH,"VSI",hudCol);

	const int y0=sy;

	d=hei/4;
	i=0;
	for(y=15; y>=-15; y-=5)
	{
		sy=y0+(d*i)/6;

		s1x=sx+6;
		s1y=sy;

		s2x=sx+11;
		s2y=sy;

		if(y==-15 || y==15 || y==0)
		{
			s2x+=3;
		}

		s2x+=4;

		if(y==-15 || y==15)
		{
			sprintf(str,"%+d",y);
			FsDrawString(s2x,s2y,str,hudCol);
		}
		else if(y==0)
		{
			FsDrawString(s2x,s2y,"0",hudCol);
		}

		i++;
	}



	int nVtxBufUsed=0;
	GLfloat vtxBuf[20*2];
	GLfloat colBuf[20*4];

	d=hei/4;
	sy=y0;
	i=0;
	for(y=15; y>=-15; y-=5)
	{
		sy=y0+(d*i)/6;

		s1x=sx+6;
		s1y=sy;

		s2x=sx+11;
		s2y=sy;

		if(y==-15 || y==15 || y==0)
		{
			s2x+=3;
		}

		vtxBuf[nVtxBufUsed*2  ]=(GLfloat)s1x;
		vtxBuf[nVtxBufUsed*2+1]=(GLfloat)s1y;
		vtxBuf[nVtxBufUsed*2+2]=(GLfloat)s2x;
		vtxBuf[nVtxBufUsed*2+3]=(GLfloat)s2y;

		colBuf[nVtxBufUsed*4  ]=(GLfloat)hudCol.Rf();
		colBuf[nVtxBufUsed*4+1]=(GLfloat)hudCol.Gf();
		colBuf[nVtxBufUsed*4+2]=(GLfloat)hudCol.Bf();
		colBuf[nVtxBufUsed*4+3]=(GLfloat)1.0f;
		colBuf[nVtxBufUsed*4+4]=(GLfloat)hudCol.Rf();
		colBuf[nVtxBufUsed*4+5]=(GLfloat)hudCol.Gf();
		colBuf[nVtxBufUsed*4+6]=(GLfloat)hudCol.Bf();
		colBuf[nVtxBufUsed*4+7]=(GLfloat)1.0f;

		nVtxBufUsed+=2;

		i++;
	}



	int r;
	r=int(YsBound(climbRatio,-1500.0,1500.0));
	r=((1500-r)*d)/3000;

	s1x=sx;
	s1y=y0+r;
	s2x=s1x+4;
	s2y=s1y;

	vtxBuf[nVtxBufUsed*2  ]=(GLfloat)s1x;
	vtxBuf[nVtxBufUsed*2+1]=(GLfloat)s1y;
	vtxBuf[nVtxBufUsed*2+2]=(GLfloat)s2x;
	vtxBuf[nVtxBufUsed*2+3]=(GLfloat)s2y;

	colBuf[nVtxBufUsed*4  ]=(GLfloat)hudCol.Rf();
	colBuf[nVtxBufUsed*4+1]=(GLfloat)hudCol.Gf();
	colBuf[nVtxBufUsed*4+2]=(GLfloat)hudCol.Bf();
	colBuf[nVtxBufUsed*4+3]=(GLfloat)1.0f;
	colBuf[nVtxBufUsed*4+4]=(GLfloat)hudCol.Rf();
	colBuf[nVtxBufUsed*4+5]=(GLfloat)hudCol.Gf();
	colBuf[nVtxBufUsed*4+6]=(GLfloat)hudCol.Bf();
	colBuf[nVtxBufUsed*4+7]=(GLfloat)1.0f;

	nVtxBufUsed+=2;

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf,colBuf);
	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawBank(const double &b)
{
	int i;
	long cenX,cenY;
	const double s[]={-0.866025,-0.707107,-0.5,0.0,0.5,0.707107,0.866025};
	const double c[]={0.5,0.707107,0.866025,1.0,0.866025,0.707107,0.5};
	double x,y,rad;

	cenX=lupX+wid/2;
	cenY=lupY+hei/6;
	rad=double(hei/3);


	const GLfloat colBuf[16*4]=
	{
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
		hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f,
	};


	int nVtxUsed=0;
	GLfloat vtxBuf[16*2];

	for(i=0; i<=6; i++)
	{
		x=s[i];
		y=c[i];

		vtxBuf[nVtxUsed*2  ]=(GLfloat)(cenX+int(x*(rad+9)));
		vtxBuf[nVtxUsed*2+1]=(GLfloat)(cenY-int(y*(rad+9)));
		vtxBuf[nVtxUsed*2+2]=(GLfloat)(cenX+int(x*(rad+14)));
		vtxBuf[nVtxUsed*2+3]=(GLfloat)(cenY-int(y*(rad+14)));
		nVtxUsed+=2;
	}

	if(-60.0<YsRadToDeg(b) && YsRadToDeg(b)<60.0)
	{
		x=sin(b);
		y=cos(b);

		vtxBuf[nVtxUsed*2  ]=(GLfloat)(cenX+int(x*(rad)));
		vtxBuf[nVtxUsed*2+1]=(GLfloat)(cenY-int(y*(rad)));
		vtxBuf[nVtxUsed*2+2]=(GLfloat)(cenX+int(x*(rad+8)));
		vtxBuf[nVtxUsed*2+3]=(GLfloat)(cenY-int(y*(rad+8)));
		nVtxUsed+=2;
	}

	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINES,nVtxUsed,vtxBuf,colBuf);
	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}

void FsHeadUpDisplay::DrawAttitude(const YsVec3 &pos,const YsAtt3 &att,const YsVec3 &viewPos,const YsAtt3 &viewAtt)
{
	YsVec3 trans,p1,p2;
	YsAtt3 attH;
	int i,j,s,w,pitch;

	attH.Set(0.0,att.p(),att.b());

	YsGLSLUse3DRenderer(YsGLSLSharedFlat3DRenderer());

	glDisable(GL_DEPTH_TEST);

	const GLfloat colBuf[4]={hudCol.Rf(),hudCol.Gf(),hudCol.Bf(),1.0f};
	YsGLSLSet3DRendererUniformColorfv(YsGLSLSharedFlat3DRenderer(),colBuf);

	const int nVtxPerBunch=32;
	GLfloat vtxBuf[nVtxPerBunch*3];
	int nVtxBufUsed=0;

	pitch=(int)YsRadToDeg(att.p());
	for(i=-90; i<=90; i+=10)
	{
		if(pitch-30<=i && i<pitch+30)
		{
			double a,y,z;
			a=double(i);
			z=100.0*cos(YsDegToRad(a));
			y=100.0*sin(YsDegToRad(a));

			if(i>=0)
			{
				s=6;
				w=6;
			}
			else
			{
				s=2;
				w=1;
			}

			for(j=20; j<32; j+=s)
			{
				p1.Set(j,y,z);
				attH.MulInverse(p1,p1);
				viewAtt.Mul(p1,p1);
				p1+=viewPos;

				p2.Set(j+w,y,z);
				attH.MulInverse(p2,p2);
				viewAtt.Mul(p2,p2);
				p2+=viewPos;

				vtxBuf[nVtxBufUsed*3  ]=(GLfloat)p1.x();
				vtxBuf[nVtxBufUsed*3+1]=(GLfloat)p1.y();
				vtxBuf[nVtxBufUsed*3+2]=(GLfloat)p1.z();
				vtxBuf[nVtxBufUsed*3+3]=(GLfloat)p2.x();
				vtxBuf[nVtxBufUsed*3+4]=(GLfloat)p2.y();
				vtxBuf[nVtxBufUsed*3+5]=(GLfloat)p2.z();
				nVtxBufUsed+=2;
				if(nVtxPerBunch-2<=nVtxBufUsed)
				{
					YsGLSLDrawPrimitiveVtxfv(YsGLSLSharedFlat3DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf);
					nVtxBufUsed=0;
				}


				p1.Set(-j,y,z);
				attH.MulInverse(p1,p1);
				viewAtt.Mul(p1,p1);
				p1+=viewPos;

				p2.Set(-j-w,y,z);
				attH.MulInverse(p2,p2);
				viewAtt.Mul(p2,p2);
				p2+=viewPos;

				vtxBuf[nVtxBufUsed*3  ]=(GLfloat)p1.x();
				vtxBuf[nVtxBufUsed*3+1]=(GLfloat)p1.y();
				vtxBuf[nVtxBufUsed*3+2]=(GLfloat)p1.z();
				vtxBuf[nVtxBufUsed*3+3]=(GLfloat)p2.x();
				vtxBuf[nVtxBufUsed*3+4]=(GLfloat)p2.y();
				vtxBuf[nVtxBufUsed*3+5]=(GLfloat)p2.z();
				nVtxBufUsed+=2;
				if(nVtxPerBunch-2<=nVtxBufUsed)
				{
					YsGLSLDrawPrimitiveVtxfv(YsGLSLSharedFlat3DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf);
					nVtxBufUsed=0;
				}
			}
		}
	}

	vtxBuf[nVtxBufUsed*3  ]=(GLfloat)(pos.x()-30.0);
	vtxBuf[nVtxBufUsed*3+1]=(GLfloat)(pos.y()+100.0);
	vtxBuf[nVtxBufUsed*3+2]=(GLfloat)pos.z();
	vtxBuf[nVtxBufUsed*3+3]=(GLfloat)(pos.x()+30.0);
	vtxBuf[nVtxBufUsed*3+4]=(GLfloat)(pos.y()+100.0);
	vtxBuf[nVtxBufUsed*3+5]=(GLfloat)pos.z();
	nVtxBufUsed+=2;
	if(nVtxPerBunch-2<=nVtxBufUsed)
	{
		YsGLSLDrawPrimitiveVtxfv(YsGLSLSharedFlat3DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf);
		nVtxBufUsed=0;
	}

	vtxBuf[nVtxBufUsed*3  ]=(GLfloat)(pos.x()-30.0);
	vtxBuf[nVtxBufUsed*3+1]=(GLfloat)(pos.y()-100.0);
	vtxBuf[nVtxBufUsed*3+2]=(GLfloat)pos.z();
	vtxBuf[nVtxBufUsed*3+3]=(GLfloat)(pos.x()+30.0);
	vtxBuf[nVtxBufUsed*3+4]=(GLfloat)(pos.y()-100.0);
	vtxBuf[nVtxBufUsed*3+5]=(GLfloat)pos.z();
	nVtxBufUsed+=2;
	if(nVtxPerBunch-2<=nVtxBufUsed)
	{
		YsGLSLDrawPrimitiveVtxfv(YsGLSLSharedFlat3DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf);
		nVtxBufUsed=0;
	}

	vtxBuf[nVtxBufUsed*3  ]=(GLfloat)pos.x();
	vtxBuf[nVtxBufUsed*3+1]=(GLfloat)(pos.y()+100.0);
	vtxBuf[nVtxBufUsed*3+2]=(GLfloat)(pos.z()-30.0);
	vtxBuf[nVtxBufUsed*3+3]=(GLfloat)pos.x();
	vtxBuf[nVtxBufUsed*3+4]=(GLfloat)(pos.y()+100.0);
	vtxBuf[nVtxBufUsed*3+5]=(GLfloat)(pos.z()+30.0);
	nVtxBufUsed+=2;
	if(nVtxPerBunch-2<=nVtxBufUsed)
	{
		YsGLSLDrawPrimitiveVtxfv(YsGLSLSharedFlat3DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf);
		nVtxBufUsed=0;
	}

	vtxBuf[nVtxBufUsed*3  ]=(GLfloat)pos.x();
	vtxBuf[nVtxBufUsed*3+1]=(GLfloat)(pos.y()-100.0);
	vtxBuf[nVtxBufUsed*3+2]=(GLfloat)(pos.z()-30.0);
	vtxBuf[nVtxBufUsed*3+3]=(GLfloat)pos.x();
	vtxBuf[nVtxBufUsed*3+4]=(GLfloat)(pos.y()-100.0);
	vtxBuf[nVtxBufUsed*3+5]=(GLfloat)(pos.z()+30.0);
	nVtxBufUsed+=2;
	YsGLSLDrawPrimitiveVtxfv(YsGLSLSharedFlat3DRenderer(),GL_LINES,nVtxBufUsed,vtxBuf);

	YsGLSLEndUse3DRenderer(YsGLSLSharedFlat3DRenderer());



	auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
	YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
	YsGLSLSetBitmapFontRendererColor4f(fsBitmapFontRenderer,colBuf[0],colBuf[1],colBuf[2],colBuf[3]);

	for(i=-90; i<=90; i+=10)
	{
		if(pitch-30<=i && i<pitch+30)
		{
			double a,y,z;
			a=double(i);
			z=100.0*cos(YsDegToRad(a));
			y=100.0*sin(YsDegToRad(a));

			if(i>=0)
			{
				s=6;
				w=6;
			}
			else
			{
				s=2;
				w=1;
			}

			for(j=20; j<32; j+=s)
			{
				p2.Set(-j-w,y,z);
				attH.MulInverse(p2,p2);
				viewAtt.Mul(p2,p2);
				p2+=viewPos;
			}

			char str[256];
			sprintf(str,"%3d",i);

			YsGLSLRenderBitmapFontString3D(fsBitmapFontRenderer,(GLfloat)p2.x(),(GLfloat)p2.y(),(GLfloat)p2.z(),str);
			//glRasterPos3dv(p2);
			//FsGlSetListBase(FS_GL_FONT_BITMAP_BASE);
			//glCallLists(strlen(str),GL_UNSIGNED_BYTE,str);
		}
	}

	YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);
}

void FsHeadUpDisplay::DrawNeedle(int cx,int cy,int wid,int lng,const double &ang,int tailLng)
{
	YsGLSLUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());

	const GLfloat c=(GLfloat)cos(ang);
	const GLfloat s=(GLfloat)sin(ang);
	const GLfloat w=(GLfloat)wid;
	const GLfloat t=(GLfloat)tailLng;

	GLfloat needle[10];
	needle[0]=(GLfloat)cx-(GLfloat)(c*(-w)-s*(-t));
	needle[1]=(GLfloat)cy-(GLfloat)(s*(-w)+c*(-t));

	needle[2]=(GLfloat)cx-(GLfloat)(c*( w)-s*(-t));
	needle[3]=(GLfloat)cy-(GLfloat)(s*( w)+c*(-t));

	needle[4]=(GLfloat)cx-(GLfloat)(c*( w)-s*(lng-wid*2));
	needle[5]=(GLfloat)cy-(GLfloat)(s*( w)+c*(lng-wid*2));

	needle[6]=(GLfloat)cx-(GLfloat)(c*( 0.0)-s*(lng));
	needle[7]=(GLfloat)cy-(GLfloat)(s*( 0.0)+c*(lng));

	needle[8]=(GLfloat)cx-(GLfloat)(c*(-w)-s*(lng-wid*2));
	needle[9]=(GLfloat)cy-(GLfloat)(s*(-w)+c*(lng-wid*2));

	const GLfloat gray[40]=
	{
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f,
		0.3f,0.3f,0.3f,1.0f
	};
	const GLfloat white[40]=
	{
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,1.0f
	};

	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_TRIANGLE_FAN,5,needle,gray);
	YsGLSLDrawPlain2DPrimitivefv(YsGLSLSharedPlain2DRenderer(),GL_LINE_LOOP,5,needle,white);

	YsGLSLEndUsePlain2DRenderer(YsGLSLSharedPlain2DRenderer());
}
