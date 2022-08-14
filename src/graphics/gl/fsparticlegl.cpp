#include <ysclass.h>

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#endif


#include <ysbitmap.h>
#include <ysglparticlemanager.h>


#ifndef WIN32
#define GL_GLEXT_PROTOTYPES
#endif


#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#endif

#include "fstexturemanager.h"
#include "fsparticle.h"


// extern unsigned int fsParticleTex;
extern double fsOpenGLVersion;

#ifdef WIN32
static void (WINAPI *glPointParameterf)(GLenum,GLfloat)=NULL;
static void (WINAPI *glPointParameteri)(GLenum,GLint)=NULL;
static void (WINAPI *glPointParameterfv)(GLenum,const GLfloat *)=NULL;
static void (WINAPI *glPointParameteriv)(GLenum,const GLint *)=NULL;
#endif

static YSBOOL FsPointParameterIsAvailable(void)
{
	static YSBOOL funcAvailable=YSTFUNKNOWN;
	if(YSTFUNKNOWN==funcAvailable)
	{
		const double glVer=atof((const char *)glGetString(GL_VERSION));
		if(1.5<=glVer)
		{
			funcAvailable=YSTRUE;
		}
		else
		{
			funcAvailable=YSFALSE;
		}
#ifdef WIN32
		glPointParameteri=(void (WINAPI *)(GLenum,GLint))wglGetProcAddress("glPointParameteri");
		glPointParameterf=(void (WINAPI *)(GLenum,GLfloat))wglGetProcAddress("glPointParameterf");
		glPointParameteriv=(void (WINAPI *)(GLenum,const GLint *))wglGetProcAddress("glPointParameteriv");
		glPointParameterfv=(void (WINAPI *)(GLenum,const GLfloat *))wglGetProcAddress("glPointParameterfv");
		if(NULL==glPointParameteri ||
		   NULL==glPointParameterf ||
		   NULL==glPointParameteriv ||
		   NULL==glPointParameterfv)
		{
			funcAvailable=YSFALSE;
		}
#endif
	}
	return funcAvailable;
}

static YSBOOL FsPointSpriteIsAvailable(void)
{
	static YSBOOL funcAvailable=YSTFUNKNOWN;
	if(YSTFUNKNOWN==funcAvailable)
	{
		const double glVer=atof((const char *)glGetString(GL_VERSION));
		if(2.0<=glVer)
		{
			funcAvailable=YSTRUE;
		}
		else
		{
			funcAvailable=YSFALSE;
		}
	}
	return funcAvailable;
}



void FsParticleStore::Draw(const class YsGLParticleManager &partMan) const
{
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);

	auto &commonTexture=FsCommonTexture::GetCommonTexture();
	if(nullptr==commonTexture.GetParticleSpriteTextureHd())
	{
		commonTexture.LoadParticleSpriteTexture();
	}

	auto texUnitPtr=commonTexture.GetParticleSpriteTexture();
	if(nullptr!=texUnitPtr)
	{
		texUnitPtr->Bind(0);
	}
	{
		glEnable(GL_TEXTURE);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_TRIANGLES);
		for(YSSIZE_T i=0; i<partMan.triVtxBuf.GetN(); ++i)
		{
			glTexCoord2fv(partMan.triTexCoordBuf.data()+i*2);
			glColor4fv(partMan.triColBuf.data()+i*4);
			glVertex3fv(partMan.triVtxBuf.data()+i*3);
		}
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE);
	}

	glEnable(GL_CULL_FACE);


/*	glDisable(GL_LIGHTING);


	if(YSTRUE==FsPointParameterIsAvailable())
	{
		glPointSize(10.0f);

		float distAtten[]={0.0f,0.0f,0.0002f};
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,distAtten);
		glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE,1.0f);
		glPointParameterf(GL_POINT_SIZE_MIN,1.0f);
		glPointParameterf(GL_POINT_SIZE_MAX,30.0f);

		// if(YSTRUE==FsPointSpriteIsAvailable())
		// {
		// 	glEnable(GL_POINT_SPRITE);
		// 	glEnable(GL_BLEND);
		// 	glEnable(GL_TEXTURE_2D);
		// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// 	glTexEnvi(GL_POINT_SPRITE,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		// 	glTexEnvi(GL_POINT_SPRITE,GL_COORD_REPLACE,GL_TRUE);
		// 	glBindTexture(GL_TEXTURE_2D,fsParticleTex);
		//  Also, GL_ALPHA_FUNC may be useful to prevent transparent pixel from hiding solid pixel that is farther away.
		// }
	}
	else
	{
		glPointSize(2);
	}

	glBegin(GL_POINTS);

	YsListItem <FsParticle> *ptr;
	particleList.RewindPointer();
	while(ptr=particleList.StepPointer())
	{
		const YsColor &col=ptr->dat.GetColor();
		const YsVec3 &pos=ptr->dat.GetPosition();
		glColor3ub(col.Ri(),col.Gi(),col.Bi());
		glVertex3dv(pos);
	}
	glEnd();


	if(YSTRUE==FsPointParameterIsAvailable())
	{
		float distAtten[]={1.0f,0.0f,0.0f};
		glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION,distAtten);

		// if(YSTRUE==FsPointSpriteIsAvailable())
		// {
		// 	glDisable(GL_POINT_SPRITE);
		// 	glDisable(GL_BLEND);
		// 	glDisable(GL_TEXTURE_2D);
		// }
	}


	glPointSize(1); */
}
