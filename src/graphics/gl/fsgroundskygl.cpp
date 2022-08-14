#include <ysclass.h>
#include "fs.h"

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <wingdi.h>
#endif

////////////////////////////////////////////////////////////

#include <fstexturemanager.h>
#include <ystexturemanager_gl.h>

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

	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(0);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	att.Set(viewAtt.h(),0.0,0.0);


	float fogDensity;
	glGetFloatv(GL_FOG_DENSITY,&fogDensity);
	glFogf(GL_FOG_DENSITY,fogDensity*10.0F);  // <- Increase fog density to draw miniature horizon cylinder.

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

	const double cylRad=farZ/10.0;
	double x0,x1;
	x0=-cylRad*3.0;  // Actually, I have to draw a infinitely long cylinder.
	x1= cylRad*3.0;  // So, x0 and x1 must be sufficiently long, but not too long to cause numerical shit.

	glPushMatrix();
	glTranslated(pos.x(),pos.y(),pos.z());
	glRotated(-YsRadToDeg(att.h()),0.0,1.0,0.0);

	glBegin(GL_TRIANGLE_STRIP);

	glColor3d(ignd.Rd(),ignd.Gd(),ignd.Bd());
	const GLfloat y0=-cylRad*sin(groundFadeAngle);
	const GLfloat z0= cylRad*cos(groundFadeAngle);

 	glVertex3d(x0, 0.0, -cylRad);
 	glVertex3d(x1, 0.0, -cylRad);
	for(i=-32; i>=1; i/=2)
	{
		const GLfloat z=cylRad*cos(groundFadeAngle)*(double)i/32.0;
		glVertex3d(x0,y0,z);
		glVertex3d(x1,y0,z);
	}
	for(i=1; i<=32; i*=2)
	{
		const GLfloat z=cylRad*cos(groundFadeAngle)*(double)i/32.0;
		glVertex3d(x0,y0,z);
		glVertex3d(x1,y0,z);
	}
 	glVertex3d(x0, 0.0, cylRad);
 	glVertex3d(x1, 0.0, cylRad);


	glColor3d(isky.Rd(),isky.Gd(),isky.Bd());
	for(i=0; i<nLayer; ++i)
	{
		double a0=(YsPi/2.0)*double(i  )/double(nLayer);

		glVertex3d(x0,cylRad*sin(a0)/10.0,cylRad*(cos(a0)));
		glVertex3d(x1,cylRad*sin(a0)/10.0,cylRad*(cos(a0)));
	}

	for(i=nLayer; 0<=i; --i)
	{
		double a0=(YsPi/2.0)*double(i  )/double(nLayer);

		glVertex3d(x0,cylRad*sin(a0)/10.0,-cylRad*(cos(a0)));
		glVertex3d(x1,cylRad*sin(a0)/10.0,-cylRad*(cos(a0)));
	}

	glEnd();

	glFogf(GL_FOG_DENSITY,fogDensity);
	glPopMatrix();

	glPopAttrib();
}

void FsGroundSky::DrawGradation
    (const YsVec3 &pos,const YsAtt3 &att,const YsColor &ignd,const YsColor &isky,const YsColor &horizon,
     const double &farZ,YSBOOL specular)
{
	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(0);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_CULL_FACE);



	double groundFadeAngle;

	// if(YsEqual(att.p(),YsPi/2.0)!=YSTRUE && YsEqual(att.p(),-YsPi/2.0)!=YSTRUE)
	// {
	// 	double groundFadeZ;  // Distance that maps fade
	// 	groundFadeZ=farZ*cos(att.p())+(farZ*sin(att.p())+pos.y())*tan(att.p());
	// 	groundFadeAngle=atan2(pos.y(),groundFadeZ);
	// }
	// else
	// {
	// 	groundFadeAngle=0.0;
	// }
	groundFadeAngle=0.0;    // What was in my mind when I made this thing?



	const double cylRad=5000.0;
	double x0,x1;
	x0=-cylRad*3.0;  // Actually, I have to draw a infinitely long cylinder.
	x1= cylRad*3.0;  // So, x0 and x1 must be sufficiently long, but not too long to cause numerical shit.

	glPushMatrix();
	glTranslated(pos.x(),pos.y(),pos.z());
	glRotated(-YsRadToDeg(att.h()),0.0,1.0,0.0);

	glBegin(GL_QUADS);

	glColor3d(ignd.Rd(),ignd.Gd(),ignd.Bd());
	glVertex3d(x0,-cylRad*sin(groundFadeAngle),-cylRad*cos(groundFadeAngle));
	glVertex3d(x1,-cylRad*sin(groundFadeAngle),-cylRad*cos(groundFadeAngle));
	glVertex3d(x1,-cylRad                     , 0.0);
	glVertex3d(x0,-cylRad                     , 0.0);

	glVertex3d(x0,-cylRad                     , 0.0);
	glVertex3d(x1,-cylRad                     , 0.0);
	glVertex3d(x1,-cylRad*sin(groundFadeAngle), cylRad*cos(groundFadeAngle));
	glVertex3d(x0,-cylRad*sin(groundFadeAngle), cylRad*cos(groundFadeAngle));

 	glColor3d(horizon.Rd(),horizon.Gd(),horizon.Bd());
 	glVertex3d(x0,-cylRad*sin(groundFadeAngle), cylRad*cos(groundFadeAngle));
 	glVertex3d(x1,-cylRad*sin(groundFadeAngle), cylRad*cos(groundFadeAngle));
 	glVertex3d(x1, 0.0, cylRad);
 	glVertex3d(x0, 0.0, cylRad);

 	//glVertex3d(x0,-cylRad*sin(groundFadeAngle),-cylRad*cos(groundFadeAngle));
 	//glVertex3d(x1,-cylRad*sin(groundFadeAngle),-cylRad*cos(groundFadeAngle));
 	//glVertex3d(x1, 0.0,-cylRad);
 	//glVertex3d(x0, 0.0,-cylRad);

	int i,k;
	double a0,a1;
	YsColor col0,col1;
	for(i=0; i<nLayer; i++)
	{
		a0=YsDegToRad(nDeg*double(i  )/double(nLayer));
		a1=YsDegToRad(nDeg*double(i+1)/double(nLayer));

		k=i+1;
		col0.SetDoubleRGB
		  ((isky.Rd()*double(i)+horizon.Rd()*double(nLayer-i))/double(nLayer),
		   (isky.Gd()*double(i)+horizon.Gd()*double(nLayer-i))/double(nLayer),
		   (isky.Bd()*double(i)+horizon.Bd()*double(nLayer-i))/double(nLayer));
		col1.SetDoubleRGB
		  ((isky.Rd()*double(k)+horizon.Rd()*double(nLayer-k))/double(nLayer),
		   (isky.Gd()*double(k)+horizon.Gd()*double(nLayer-k))/double(nLayer),
		   (isky.Bd()*double(k)+horizon.Bd()*double(nLayer-k))/double(nLayer));

		glColor3d(col0.Rd(),col0.Gd(),col0.Bd());
		glVertex3d(x0,cylRad*sin(a0),cylRad*(cos(a0)));
		glVertex3d(x1,cylRad*sin(a0),cylRad*(cos(a0)));
		glColor3d(col1.Rd(),col1.Gd(),col1.Bd());
		glVertex3d(x1,cylRad*sin(a1),cylRad*(cos(a1)));
		glVertex3d(x0,cylRad*sin(a1),cylRad*(cos(a1)));
	}

	a1=YsDegToRad(nDeg);
	glVertex3d(x0,cylRad*sin(a1), cylRad*cos(a1));
	glVertex3d(x1,cylRad*sin(a1), cylRad*cos(a1));
	glVertex3d(x1,cylRad*sin(a1),-cylRad*cos(a1));
	glVertex3d(x0,cylRad*sin(a1),-cylRad*cos(a1));

	glEnd();  // GL_QUADS
	glPopMatrix();

	glPopAttrib();
}

void FsGroundSky::DrawGroundMesh(const YsVec3 &pos,const YsAtt3 &att,const YsColor &ignd,int div,YSBOOL)
{
	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(0);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	glStencilFunc(GL_EQUAL,0,255);

	auto &commonTexture=FsCommonTexture::GetCommonTexture();
	commonTexture.LoadGroundTileTexture();
	auto texUnitPtr=commonTexture.GetGroundTileTexture();

	if(nullptr!=texUnitPtr)
	{
		int xx,zz,xp,zp;
		int x1,z1,x2,z2;
		const int scaleFactor=4050;
		const int nGrid=6;

		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

		texUnitPtr->Bind();

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_2D);

		YsVec3 org;
		xx=(int)(pos.x())/scaleFactor;
		zz=(int)(pos.z())/scaleFactor;

		xx*=scaleFactor;
		zz*=scaleFactor;

		glColor3ub(ignd.Ri(),ignd.Gi(),ignd.Bi());

		for(zp=-nGrid; zp<nGrid; zp++)
		{
			glBegin(GL_QUAD_STRIP);
			for(xp=-nGrid; xp<=nGrid; xp++)
			{
				x1=xp;
				z1=zp;
				x1=xx+x1*scaleFactor;
				z1=zz+z1*scaleFactor;

				glVertex3i(x1,0,z1);

				x2=xp;
				z2=zp+1;
				x2=xx+x2*scaleFactor;
				z2=zz+z2*scaleFactor;

				glVertex3i(x2,0,z2);
			}
			glEnd();
		}


		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
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

		glColor3ub(255,255,255);

		for(zp=-nGrid; zp<nGrid; zp++)
		{
			glBegin(GL_QUAD_STRIP);
			for(xp=-nGrid; xp<=nGrid; xp++)
			{
				int scale;
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

				glColor3ub(r,g,b);
				glVertex3i(x1,0,z1);

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

				glColor3ub(r,g,b);
				glVertex3i(x2,0,z2);
			}
			glEnd();
		}



	//	int x,z,cx,cz;
	//
	//	cx=int(pos.x());
	//	cz=int(pos.z());
	//	cx=cx/div;
	//	cz=cz/div;
	//	cx=cx*div;
	//	cz=cz*div;
	//
	//	glColor3d(1.0,1.0,1.0);
	//
	//	for(z=-4; z<=4; z++)
	//	{
	//		YsVec3 p;
	//		glBegin(GL_QUAD_STRIP);
	//		for(x=-4; x<=4; x++)
	//		{
	//			int xx,zz,r,g,b;
	//			xx=cx+x*div;
	//			zz=cz+z*div;
	//
	//			r=ignd.Ri();
	//			g=ignd.Gi();
	//			b=ignd.Bi();
	//
	//			r=YsBound(r+(xx&15)-8,0,255);
	//			g=YsBound(g+(zz&15)-8,0,255);
	//			b=YsBound(b+(xx&15)+(zz&15)-16,0,255);
	//
	//			glColor3ub(r,g,b);
	//			glVertex3i(xx,0,zz);
	//
	//			xx=cx+x*div;
	//			zz=cz+z*div+div;
	//
	//			r=ignd.Ri();
	//			g=ignd.Gi();
	//			b=ignd.Bi();
	//
	//			r=YsBound(r+(xx&15)-8,0,255);
	//			g=YsBound(g+(zz&15)-8,0,255);
	//			b=YsBound(b+(xx&15)+(zz&15)-16,0,255);
	//
	//			glColor3ub(r,g,b);
	//			glVertex3i(xx,0,zz);
	//		}
	//		glEnd();
	//	}
	}

	glStencilFunc(GL_ALWAYS,0,255);
	glPopAttrib();
}

void FsGroundSky::DrawCrappy(const YsVec3 &pos,const YsColor &ignd,const YsColor &isky,const double &farZ,YSBOOL)
{
	int i,j;
	YsColor col1,col2;

	glClearColor((float)ignd.Rd(),(float)ignd.Gd(),(float)ignd.Bd(),1.0F);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0F,0.0F,0.0F,1.0F);


	col2.SetDoubleRGB(1.0,1.0,1.0);

	glPushAttrib(GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_ALWAYS);
	glDepthMask(0);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_CULL_FACE);

	glColor3d(isky.Rd(),isky.Gd(),isky.Bd());
	glBegin(GL_QUADS);
	for(i=0; i<FsGndSkyDivY-1; i++)
	{
		for(j=0; j<FsGndSkyDivX-1; j++)
		{
			glVertex3dv(FsGndSkyHalfSphere[i  ][j  ]*farZ*0.5+pos);
			glVertex3dv(FsGndSkyHalfSphere[i+1][j  ]*farZ*0.5+pos);
			glVertex3dv(FsGndSkyHalfSphere[i+1][j+1]*farZ*0.5+pos);
			glVertex3dv(FsGndSkyHalfSphere[i  ][j+1]*farZ*0.5+pos);
		}
	}
	glEnd();

	glPopAttrib();
}
