#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>
#include <ysglbuffermanager.h>
#include <ysglbuffermanager_gl1.h>


#ifdef WIN32
#include <windows.h>
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <fsdef.h>
#include "graphics/common/fsopengl.h"
#include "fscloud.h"


static inline void FsResetMaterial(void)
{
// Ubuntu 11.1's OpenGL driver bugged.  It forgets material very often.
#if !defined(__APPLE__) && !defined(_WIN32)
	float zero[4]={0,0,0,0};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,zero);
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,zero);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,zero);

	int shininess[1]={0};
	glMaterialiv(GL_FRONT_AND_BACK,GL_SHININESS,shininess);
#endif
}


void FsCloud::Draw(void)
{
	glColor3d(col.Rd(),col.Gd(),col.Bd());

	glBegin(GL_TRIANGLE_FAN);
	glVertex3dv(cen);
	int i;
	for(i=0; i<vtx.GetN(); i++)
	{
		glVertex3dv(vtx[i]);
	}
	glVertex3dv(vtx[0]);
	glEnd();
}


void FsClouds::CreateGraphicCache(void)
{
	res=NULL;
}

void FsClouds::DeleteGraphicCache(void)
{
}

void FsClouds::Draw(void)
{
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==vboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(vboHd)->GetState() || YSTRUE==needRemakeVbo)
	{
		MakeOpenGlList();
		needRemakeVbo=YSFALSE;
	}

	if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
	{
		glDisable(GL_LIGHTING);

		auto unitPtr=bufMan.GetBufferUnit(vboHd);
		if(nullptr!=unitPtr)
		{
			unitPtr->GetActualBuffer()->DrawPrimitiveVtxCol(GL_TRIANGLES);
		}
	}
}

void FsSolidClouds::ReduceVisibilityByPolygon(const YsMatrix4x4 &viewTfm,const YsColor &col,YSBOOL transparency)
{
	YsMatrix4x4 invViewTfm;
	invViewTfm=viewTfm;
	invViewTfm.Invert();

	if(transparency==YSTRUE)
	{
		glDisable(GL_LIGHTING);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		glColor4ub(col.Ri(),col.Gi(),col.Bi(),160);
		glBegin(GL_QUADS);
		int z;
		for(z=400; z>=200; z-=50)
		{
			YsVec3 pos;
			pos.Set(z*3,z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			glVertex3dv(pos);

			pos.Set(-z*3,z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			glVertex3dv(pos);

			pos.Set(-z*3,-z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			glVertex3dv(pos);

			pos.Set(z*3,-z*3,z);
			invViewTfm.Mul(pos,pos,1.0);
			glVertex3dv(pos);
		}
		glEnd();

		//glDisable(GL_BLEND);
	}
	else
	{
		glColor3ub(col.Ri(),col.Gi(),col.Bi());
		glBegin(GL_QUADS);

		int z;
		YsVec3 pos;

		z=200;

		pos.Set(z*3,z*3,z);
		invViewTfm.Mul(pos,pos,1.0);
		glVertex3dv(pos);

		pos.Set(-z*3,z*3,z);
		invViewTfm.Mul(pos,pos,1.0);
		glVertex3dv(pos);

		pos.Set(-z*3,-z*3,z);
		invViewTfm.Mul(pos,pos,1.0);
		glVertex3dv(pos);

		pos.Set(z*3,-z*3,z);
		invViewTfm.Mul(pos,pos,1.0);
		glVertex3dv(pos);

		glEnd();

	}
}

void FsSolidCloud::CreateGraphicCache(void)
{
	res=NULL;
}

void FsSolidCloud::DeleteGraphicCache(void)
{
}

void FsSolidCloud::Draw(FSENVIRONMENT env,const FsWeather &weather)
{
	FsResetMaterial();

	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==vboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(vboHd)->GetState())
	{
		MakeOpenGlList();
	}

	if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
	{
		glDisable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);

		auto unitPtr=bufMan.GetBufferUnit(vboHd);
		if(nullptr!=unitPtr)
		{
			unitPtr->GetActualBuffer()->DrawPrimitiveVtxNomCol(GL_TRIANGLES);
		}

		glDisable(GL_CULL_FACE);
	}
}

void FsSolidClouds::Test(void)
{
}

void FsSolidClouds::SetUpCloudPerFrame(void)
{
}

void FsSolidClouds::BeginDrawCloud(void)
{
}

void FsSolidClouds::EndDrawCloud(void)
{
}
