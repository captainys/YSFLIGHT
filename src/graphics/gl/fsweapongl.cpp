#include <ysclass.h>
#include "fs.h"

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif



void FsWeaponSmokeTrail::Draw(const YsVec3 &pos,const YsAtt3 &att,YSBOOL transparency,FSSMOKETYPE smk,const double &cTime) const
{
	YsGLVertexBuffer vtxBuf[4];
	YsGLNormalBuffer nomBuf[4];
	YsGLColorBuffer colBuf[4];

	this->MakeVertexArray(
	    vtxBuf,nomBuf,colBuf,
	    pos,att,smk,cTime);

	glDisable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);

	glShadeModel(GL_SMOOTH);
	glColor4d(0.7,0.7,0.7,0.8);

	for(int i=0; i<4; ++i)
	{
		glBegin(GL_TRIANGLE_STRIP);

		for(YSSIZE_T j=0; j<vtxBuf[i].size(); ++j)
		{
			glColor4fv(colBuf[i][j]);
			glNormal3fv(nomBuf[i][j]);
			glVertex3fv(vtxBuf[i][j]);
		}
		glEnd();
	}

	glEnable(GL_CULL_FACE);
}

void FsWeapon::Draw(
    YSBOOL coarse,const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,
    YSBOOL transparency,FSSMOKETYPE smk,const double &cTime,unsigned int drawFlag) const
{
	if(lifeRemain>YsTolerance || timeRemain>YsTolerance)
	{
		if(lifeRemain>YsTolerance &&
		   YSTRUE!=coarse &&
		   FSWEAPON_NUMWEAPONTYPE>type &&
		   NULL!=firedBy &&
		   NULL!=firedBy->weaponShapeOverrideFlying[(int)type])
		{
			firedBy->weaponShapeOverrideFlying[(int)type].Draw(viewMat,projMat,pos,att,drawFlag);
		}
		else switch(type)
		{
		case FSWEAPON_GUN:
			glDisable(GL_LIGHTING);

			glBegin(GL_LINES);
			glColor3d(1.0,1.0,0.0);
			glVertex3dv(pos);
			glColor3d(1.0,1.0,1.0);
			glVertex3dv(pos+vec/velocity*10.0);
			glEnd();
			break;
		case FSWEAPON_AIM9:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && aim9!=nullptr)
				{
					aim9.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && aim_coarse!=nullptr)
				{
					aim_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AIM9X:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && aim9x!=nullptr)
				{
					aim9x.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && aim_coarse!=nullptr)
				{
					aim9x_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AIM120:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && aim120!=nullptr)
				{
					aim120.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && aim120_coarse!=nullptr)
				{
					aim120_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && bomb!=nullptr)
				{
					bomb.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && bomb_coarse!=nullptr)
				{
					bomb_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB250:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && bomb!=nullptr)
				{
					bomb250.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && bomb_coarse!=nullptr)
				{
					bomb250_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB500HD:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && bomb!=nullptr)
				{
					bomb500hd.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && bomb_coarse!=nullptr)
				{
					bomb250_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AGM65:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && agm65!=nullptr)
				{
					agm65.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && agm_coarse!=nullptr)
				{
					agm_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_ROCKET:
			if(lifeRemain>YsTolerance)
			{
				glDisable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
				if(coarse!=YSTRUE && rocket!=nullptr)
				{
					rocket.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && rocket_coarse!=nullptr)
				{
					rocket_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_FUELTANK:
			if(lifeRemain>YsTolerance)
			{
				auto &vis=fuelTank;
				if(vis!=nullptr)
				{
					vis.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_FLARE:
			break;

		case FSWEAPON_DEBRIS:
			glDisable(GL_LIGHTING);
			glColor3ub(32,32,32);

			glBegin(GL_POINTS);
			glVertex3dv(pos);
			glEnd();

			glPushMatrix();
			glTranslated(pos.x(),pos.y(),pos.z());
			glRotated(YsRadToDeg(att.b()),0.0,0.0,1.0);
			glRotated(YsRadToDeg(att.p()),1.0,0.0,0.0);
			glRotated(YsRadToDeg(att.h()),0.0,1.0,0.0);

			glNormal3d(0.0,1.0,0.0);

			glBegin(GL_QUADS);
			glVertex3d(-0.8,0.0,-0.8);
			glVertex3d(-0.8,0.0, 0.8);
			glVertex3d( 0.8,0.0, 0.8);
			glVertex3d( 0.8,0.0,-0.8);
			glEnd();

			glPopMatrix();

			break;
		}

		if(trail!=nullptr &&
		   FSSMKNULL!=smk &&
		   0!=(drawFlag&FSVISUAL_DRAWTRANSPARENT))
		{
			trail->Draw(pos,att,transparency,smk,cTime);
		}
	}
}

////////////////////////////////////////////////////////////

void FsWeaponHolder::BeginDraw(void) const
{
}

void FsWeaponHolder::EndDraw(void) const
{
}

void FsWeaponHolder::DrawGunCalibrator(void) const
{
	if(0<bulletCalibrator.GetN())
	{
		glPointSize(4);
		glDisable(GL_LIGHTING);
		glColor3ub(255,0,0);
		glBegin(GL_POINTS);

		for(int i=0; i<bulletCalibrator.GetN(); i++)
		{
			const YsVec3 &nearPos=bulletCalibrator[i];
			glVertex3dv(nearPos);
		}

		glEnd();
		glPointSize(1);
	}
}

