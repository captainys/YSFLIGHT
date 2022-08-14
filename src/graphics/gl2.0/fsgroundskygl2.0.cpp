#include <ysclass.h>
#include "fs.h"

#include <ysgl.h>
#include <fstexturemanager.h>
#include <ystexturemanager_gl.h>
#include "fsgl2.0util.h"

////////////////////////////////////////////////////////////

extern YSBOOL ysScnGlUseMapTexture;
extern GLuint ysScnGlMapTex;

class FsGroundSkyGraphicCache
{
public:
	FsGL2VariableVertexStorage vtxBuf;
};

// Implementation //////////////////////////////////////////
FsGroundSky::FsGroundSky()
{
	res=new FsGroundSkyGraphicCache;

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
	delete res;
}

void FsGroundSky::DrawByFog(
    const YsVec3 &pos,const YsAtt3 &viewAtt,const YsColor &ignd,const YsColor &isky,const YsColor & /*horizon*/,
    const double &farZ,YSBOOL specular)
{
	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);

	YsGLSL3DRenderer *renderer=YsGLSLSharedVariColor3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat prevTfm[16];
	YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);

	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);


	YsAtt3 att;
	att.Set(viewAtt.h(),0.0,0.0);


	const GLfloat fogDensity=YsGLSLGet3DRendererFogDensity(renderer);
	YsGLSLSet3DRendererUniformFogDensity(renderer,fogDensity*10.0F);

	GLfloat groundFadeAngle=0.0f;
	if(YsEqual(viewAtt.p(),YsPi/2.0)!=YSTRUE && YsEqual(viewAtt.p(),-YsPi/2.0)!=YSTRUE)
	{
		GLfloat groundFadeZ;  // Distance that maps fade
		groundFadeZ=(GLfloat)(farZ*cos(viewAtt.p())+(farZ*sin(viewAtt.p())+pos.y())*tan(viewAtt.p()));
		groundFadeAngle=(GLfloat)atan2((GLfloat)pos.y(),groundFadeZ);
	}

	const GLfloat cylRad=(GLfloat)farZ/10.0f;
	const GLfloat x0=-cylRad*3.0f;  // Actually, I have to draw a infinitely long cylinder.
	const GLfloat x1= cylRad*3.0f;  // So, x0 and x1 must be sufficiently long, but not too long to cause numerical shit.

	GLfloat tfm[16];
	YsGLCopyMatrixfv(tfm,prevTfm);
	YsGLMultMatrixTranslationfv(tfm,(GLfloat)pos.x(),(GLfloat)pos.y(),(GLfloat)pos.z());
	YsGLMultMatrixRotationXZfv(tfm,(GLfloat)att.h());
	YsGLSLSet3DRendererModelViewfv(renderer,tfm);

	res->vtxBuf.CleanUp();

	const GLfloat y0=-cylRad*sinf(groundFadeAngle);

	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x0, 0.0f, -cylRad);
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x1, 0.0f, -cylRad);
	for(int i=-32; i>=1; i/=2)
	{
		const GLfloat z=cylRad*cosf(groundFadeAngle)*(GLfloat)i/32.0f;
		res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
		res->vtxBuf.AddVertex(x0,y0,z);
		res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
		res->vtxBuf.AddVertex(x1,y0,z);
	}
	for(int i=1; i<=32; i*=2)
	{
		const GLfloat z=cylRad*cosf(groundFadeAngle)*(GLfloat)i/32.0f;
		res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
		res->vtxBuf.AddVertex(x0,y0,z);
		res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
		res->vtxBuf.AddVertex(x1,y0,z);
	}
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0, 0.0f, cylRad);
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1, 0.0f, cylRad);


	for(int i=0; i<nLayer; ++i)
	{
		const double a0=(YsPi/2.0)*double(i)/double(nLayer);

		res->vtxBuf.AddColor(isky.Rf(),isky.Gf(),isky.Bf(),1.0f);
		res->vtxBuf.AddVertex(x0,cylRad*(GLfloat)sin(a0)/10.0f,cylRad*(GLfloat)cos(a0));
		res->vtxBuf.AddColor(isky.Rf(),isky.Gf(),isky.Bf(),1.0f);
		res->vtxBuf.AddVertex(x1,cylRad*(GLfloat)sin(a0)/10.0f,cylRad*(GLfloat)cos(a0));
	}

	for(int i=nLayer; 0<=i; --i)
	{
		const double a0=(YsPi/2.0)*double(i)/double(nLayer);

		res->vtxBuf.AddColor(isky.Rf(),isky.Gf(),isky.Bf(),1.0f);
		res->vtxBuf.AddVertex(x0,cylRad*(GLfloat)sin(a0)/10.0f,-cylRad*(GLfloat)cos(a0));
		res->vtxBuf.AddColor(isky.Rf(),isky.Gf(),isky.Bf(),1.0f);
		res->vtxBuf.AddVertex(x1,cylRad*(GLfloat)sin(a0)/10.0f,-cylRad*(GLfloat)cos(a0));
	}

	YsGLSLDrawPrimitiveVtxColfv(renderer,GL_TRIANGLE_STRIP,res->vtxBuf.nVtx,res->vtxBuf.vtxArray,res->vtxBuf.colArray);

	YsGLSLSet3DRendererUniformFogDensity(renderer,fogDensity);
	YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);
	YsGLSLEndUse3DRenderer(renderer);

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
}

void FsGroundSky::DrawGradation
    (const YsVec3 &pos,const YsAtt3 &att,const YsColor &ignd,const YsColor &isky,const YsColor &horizon,
     const double & /*farZ*/,YSBOOL specular)
{
	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);
	glEnable(GL_CULL_FACE);



	GLfloat groundFadeAngle=0.0f;
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



	const GLfloat cylRad=5000.0f;
	const GLfloat x0=-cylRad*3.0f;  // Actually, I have to draw a infinitely long cylinder.
	const GLfloat x1= cylRad*3.0f;  // So, x0 and x1 must be sufficiently long, but not too long to cause numerical shit.


	YsGLSL3DRenderer *renderer=YsGLSLSharedVariColor3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat prevTfm[16];
	YsGLSLGet3DRendererModelViewfv(prevTfm,renderer);

	GLfloat tfm[16];
	YsGLCopyMatrixfv(tfm,prevTfm);
	YsGLMultMatrixTranslationfv(tfm,(GLfloat)pos.x(),(GLfloat)pos.y(),(GLfloat)pos.z());
	YsGLMultMatrixRotationXZfv(tfm,(GLfloat)att.h());
	YsGLSLSet3DRendererModelViewfv(renderer,tfm);

	res->vtxBuf.CleanUp();

	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,-cylRad*sinf(groundFadeAngle),-cylRad*cosf(groundFadeAngle));
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,-cylRad*sinf(groundFadeAngle),-cylRad*cosf(groundFadeAngle));
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,-cylRad                     , 0.0f);

	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,-cylRad                     , 0.0f);
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,-cylRad                     , 0.0f);
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,-cylRad*sinf(groundFadeAngle),-cylRad*cosf(groundFadeAngle));

	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,-cylRad                     , 0.0f);
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,-cylRad                     , 0.0f);
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,-cylRad*sinf(groundFadeAngle), cylRad*cosf(groundFadeAngle));

	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,-cylRad*sinf(groundFadeAngle), cylRad*cosf(groundFadeAngle));
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,-cylRad*sinf(groundFadeAngle), cylRad*cosf(groundFadeAngle));
	res->vtxBuf.AddColor(ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,-cylRad                     , 0.0f);

 	res->vtxBuf.AddColor(horizon.Rf(),horizon.Gf(),horizon.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x0,-cylRad*sinf(groundFadeAngle), cylRad*cosf(groundFadeAngle));
 	res->vtxBuf.AddColor(horizon.Rf(),horizon.Gf(),horizon.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x1,-cylRad*sinf(groundFadeAngle), cylRad*cosf(groundFadeAngle));
 	res->vtxBuf.AddColor(horizon.Rf(),horizon.Gf(),horizon.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x1, 0.0f, cylRad);

 	res->vtxBuf.AddColor(horizon.Rf(),horizon.Gf(),horizon.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x1, 0.0f, cylRad);
 	res->vtxBuf.AddColor(horizon.Rf(),horizon.Gf(),horizon.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x0, 0.0f, cylRad);
 	res->vtxBuf.AddColor(horizon.Rf(),horizon.Gf(),horizon.Bf(),1.0f);
 	res->vtxBuf.AddVertex(x0,-cylRad*sinf(groundFadeAngle), cylRad*cosf(groundFadeAngle));


	int k;
	YsColor col0,col1;
	GLfloat a0,a1;
	const GLfloat nRadianf=(GLfloat)YsDegToRad(nDeg);
	for(int i=0; i<nLayer; i++)
	{
		a0=(nRadianf*GLfloat(i  )/GLfloat(nLayer));
		a1=(nRadianf*GLfloat(i+1)/GLfloat(nLayer));

		k=i+1;
		col0.SetDoubleRGB
		  ((isky.Rd()*double(i)+horizon.Rd()*double(nLayer-i))/double(nLayer),
		   (isky.Gd()*double(i)+horizon.Gd()*double(nLayer-i))/double(nLayer),
		   (isky.Bd()*double(i)+horizon.Bd()*double(nLayer-i))/double(nLayer));
		col1.SetDoubleRGB
		  ((isky.Rd()*double(k)+horizon.Rd()*double(nLayer-k))/double(nLayer),
		   (isky.Gd()*double(k)+horizon.Gd()*double(nLayer-k))/double(nLayer),
		   (isky.Bd()*double(k)+horizon.Bd()*double(nLayer-k))/double(nLayer));

		res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
		res->vtxBuf.AddVertex(x0,cylRad*sinf(a0),cylRad*(cosf(a0)));
		res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
		res->vtxBuf.AddVertex(x1,cylRad*sinf(a0),cylRad*(cosf(a0)));
		res->vtxBuf.AddColor(col1.Rf(),col1.Gf(),col1.Bf(),1.0f);
		res->vtxBuf.AddVertex(x1,cylRad*sinf(a1),cylRad*(cosf(a1)));

		res->vtxBuf.AddColor(col1.Rf(),col1.Gf(),col1.Bf(),1.0f);
		res->vtxBuf.AddVertex(x1,cylRad*sinf(a1),cylRad*(cosf(a1)));
		res->vtxBuf.AddColor(col1.Rf(),col1.Gf(),col1.Bf(),1.0f);
		res->vtxBuf.AddVertex(x0,cylRad*sinf(a1),cylRad*(cosf(a1)));
		res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
		res->vtxBuf.AddVertex(x0,cylRad*sinf(a0),cylRad*(cosf(a0)));
	}

	a1=nRadianf;
	res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,cylRad*sinf(a1), cylRad*cosf(a1));
	res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,cylRad*sinf(a1), cylRad*cosf(a1));
	res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,cylRad*sinf(a1),-cylRad*cosf(a1));

	res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
	res->vtxBuf.AddVertex(x1,cylRad*sinf(a1),-cylRad*cosf(a1));
	res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,cylRad*sinf(a1),-cylRad*cosf(a1));
	res->vtxBuf.AddColor(col0.Rf(),col0.Gf(),col0.Bf(),1.0f);
	res->vtxBuf.AddVertex(x0,cylRad*sinf(a1), cylRad*cosf(a1));

	YsGLSLDrawPrimitiveVtxColfv(renderer,GL_TRIANGLES,res->vtxBuf.nVtx,res->vtxBuf.vtxArray,res->vtxBuf.colArray);

	YsGLSLSet3DRendererModelViewfv(renderer,prevTfm);
	YsGLSLEndUse3DRenderer(renderer);
	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
}

void FsGroundSky::DrawGroundMesh(const YsVec3 &pos,const YsAtt3 &att,const YsColor &ignd,int div,YSBOOL specular)
{
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);

	glStencilFunc(GL_EQUAL,0,255);

	auto &commonTexture=FsCommonTexture::GetCommonTexture();
	auto gndTileUnitPtr=commonTexture.GetGroundTileTexture();

	if(nullptr!=gndTileUnitPtr)
	{
		int xx,zz,xp,zp;
		int x1,z1,x2,z2;
		const int scaleFactor=4050;
		const int nGrid=6;

		glActiveTexture(GL_TEXTURE0);

		gndTileUnitPtr->Bind();

		glEnable(GL_CULL_FACE);

		YsGLSL3DRenderer *renderer=YsGLSLSharedMonoColorShaded3DRenderer();

		const GLfloat texTfm[]=
		{
			0.025f,0.0f,  0.0f,  0,
			0.0f,  0.0f,  0.025f,0,
			0.0f,  0.025f,0.0f,  0,
			0.0f,  0.0f,  0.0f,  1
		};
		YsGLSLUse3DRenderer(renderer);


		GLfloat savedSpecularColor[3];
		YsGLSLGet3DRendererSpecularColor(savedSpecularColor,renderer);
		const GLfloat specularOn[3]={1.0F,1.0F,1.0F};
		const GLfloat specularOff[3]={0.0F,0.0F,0.0F};
		YsGLSLSet3DRendererSpecularColor(renderer,(YSTRUE==specular ? specularOn : specularOff));


		YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_TILING);
		YsGLSLSet3DRendererTextureIdentifier(renderer,0); // 0 for GL_TEXTURE0 apparently
		YsGLSLSet3DRendererUniformTextureTilingMatrixfv(renderer,texTfm);

		YsVec3 org;
		xx=(int)(pos.x())/scaleFactor;
		zz=(int)(pos.z())/scaleFactor;

		xx*=scaleFactor;
		zz*=scaleFactor;

		const GLfloat gndColor[4]={ignd.Rf(),ignd.Gf(),ignd.Bf(),1.0f};
		YsGLSLSet3DRendererUniformColorfv(renderer,gndColor);

		for(zp=-nGrid; zp<nGrid; zp++)
		{
			res->vtxBuf.CleanUp();
			for(xp=-nGrid; xp<=nGrid; xp++)
			{
				x2=xp;
				z2=zp+1;
				x2=xx+x2*scaleFactor;
				z2=zz+z2*scaleFactor;

				res->vtxBuf.AddVertex<int>(x2,0,z2);
				res->vtxBuf.AddNormal<int>(0,1,0);

				x1=xp;
				z1=zp;
				x1=xx+x1*scaleFactor;
				z1=zz+z1*scaleFactor;

				res->vtxBuf.AddVertex<int>(x1,0,z1);
				res->vtxBuf.AddNormal<int>(0,1,0);
			}
			YsGLSLDrawPrimitiveVtxNomfv(renderer,GL_TRIANGLE_STRIP,res->vtxBuf.nVtx,res->vtxBuf.vtxArray,res->vtxBuf.nomArray);
		}

		YsGLSLSet3DRendererSpecularColor(renderer,savedSpecularColor);

		YsGLSLSet3DRendererTextureType(renderer,YSGLSL_TEX_TYPE_NONE);

		YsGLSLEndUse3DRenderer(renderer);
	}
	else
	{
		// Boundary condition:
		//   Out most points: same as ignd
		//
		// Grid sizes:
		//   2^5  2^8  2^11  2^14

		YsGLSL3DRenderer *renderer=YsGLSLSharedVariColor3DRenderer();
		YsGLSLUse3DRenderer(renderer);

		int xx,zz,xp,zp; // ,i,r,g,b,roff,goff,boff
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
			res->vtxBuf.CleanUp();

			for(xp=-nGrid; xp<=nGrid; xp++)
			{
				// int scale;
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

				res->vtxBuf.AddColor((GLfloat)r/256.0f,(GLfloat)g/256.0f,(GLfloat)b/256.0f,1.0f);
				res->vtxBuf.AddVertex<int>(x1,0,z1);

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

				res->vtxBuf.AddColor((GLfloat)r/256.0f,(GLfloat)g/256.0f,(GLfloat)b/256.0f,1.0f);
				res->vtxBuf.AddVertex<int>(x2,0,z2);
			}
			YsGLSLDrawPrimitiveVtxColfv(renderer,GL_TRIANGLE_STRIP,res->vtxBuf.nVtx,res->vtxBuf.vtxArray,res->vtxBuf.colArray);
		}

		YsGLSLEndUse3DRenderer(renderer);
	}

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glStencilFunc(GL_ALWAYS,0,255);
}

void FsGroundSky::DrawCrappy(const YsVec3 &pos,const YsColor &ignd,const YsColor &isky,const double &farZ,YSBOOL specular)
{
	int i,j;
	YsColor col1,col2;

	glClearColor((float)ignd.Rd(),(float)ignd.Gd(),(float)ignd.Bd(),1.0F);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0F,0.0F,0.0F,1.0F);
	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);


	col2.SetDoubleRGB(1.0,1.0,1.0);

	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_FALSE);


	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	res->vtxBuf.CleanUp();

	const GLfloat color[4]={isky.Rf(),isky.Gf(),isky.Bf(),1.0f};
	YsGLSLSet3DRendererUniformColorfv(renderer,color);

	for(i=0; i<FsGndSkyDivY-1; i++)
	{
		for(j=0; j<FsGndSkyDivX-1; j++)
		{
			res->vtxBuf.AddVertex(FsGndSkyHalfSphere[i+1][j  ]*farZ*0.5+pos);
			res->vtxBuf.AddVertex(FsGndSkyHalfSphere[i  ][j  ]*farZ*0.5+pos);
			res->vtxBuf.AddVertex(FsGndSkyHalfSphere[i+1][j+1]*farZ*0.5+pos);

			res->vtxBuf.AddVertex(FsGndSkyHalfSphere[i  ][j+1]*farZ*0.5+pos);
			res->vtxBuf.AddVertex(FsGndSkyHalfSphere[i+1][j+1]*farZ*0.5+pos);
			res->vtxBuf.AddVertex(FsGndSkyHalfSphere[i  ][j  ]*farZ*0.5+pos);
		}
	}

	YsGLSLDrawPrimitiveVtxfv(renderer,GL_TRIANGLES,res->vtxBuf.nVtx,res->vtxBuf.vtxArray);
	YsGLSLEndUse3DRenderer(renderer);

	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
}
