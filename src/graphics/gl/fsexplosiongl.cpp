#include <ysclass.h>
#include <ysglbuffermanager.h>
#include <ysglbuffermanager_gl1.h>

#include "fs.h"
#include "graphics/common/fsopengl.h"

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

void FsExplosion::Draw(const YsVec3 &viewPos,YSBOOL transparency,YSBOOL useOpenGlDisplayList) const
{
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(timeRemain>=YsTolerance)
	{
		switch(expType)
		{
		case FSEXPLOSION_FIREBALL:
			{
				double r,t,alpha;
				t=YsSmaller(timePassed,0.6)/0.6;


				if(transparency==YSTRUE)
				{
					double d;
					d=timeRemain+timePassed;
					if(d>YsTolerance)
					{
						alpha=0.4+0.6*(timeRemain/d);
					}
					else
					{
						alpha=1.0;
					}
				}
				else
				{
					alpha=1.0;
				}


				if(flash==YSTRUE && timePassed<=0.05)
				{
					glDisable(GL_LIGHTING);
					r=radius/2.0;
					glColor3d(1.0,1.0,1.0);
				}
				else
				{
					glEnable(GL_LIGHTING);
					r=iniRadius+(radius-iniRadius)*t;
					float diffuseAmbient[4];
					diffuseAmbient[0]=(float)(1.0-t*t*0.78);
					diffuseAmbient[1]=(float)(t*t*0.22);
					diffuseAmbient[2]=(float)(t*t*0.22);
					diffuseAmbient[3]=(float)alpha;
					glColor4fv(diffuseAmbient);
				}

				if((nullptr==fireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(fireballVboHd)->GetState()) ||
				   (nullptr==halfFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(halfFireballVboHd)->GetState()) ||
				   (nullptr==reverseFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(reverseFireballVboHd)->GetState()) ||
				   (nullptr==reverseHalfFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(reverseHalfFireballVboHd)->GetState()))
				{
					PrepareFireballVertexArray();
				}

				auto fullVboHd=fireballVboHd;
				auto halfVboHd=halfFireballVboHd;
				if(transparency==YSTRUE)
				{
					if((viewPos-pos).GetSquareLength()<r*r)
					{
						fullVboHd=reverseFireballVboHd;
						halfVboHd=reverseHalfFireballVboHd;
					}
				}

				if(pos.y()>r)
				{
					glPushMatrix();
					glTranslated(pos.x(),pos.y(),pos.z());
					glScaled(r,r,r);

					auto vboHd=fullVboHd;
					if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
					{
						auto unitPtr=bufMan.GetBufferUnit(vboHd);
						if(nullptr!=unitPtr)
						{
							unitPtr->GetActualBuffer()->DrawPrimitiveVtxNom(GL_TRIANGLES);
						}
					}
					glPopMatrix();
				}
				else
				{
					glPushMatrix();
					glTranslated(pos.x(),0.0,pos.z());
					glScaled(r,r,r);

					auto vboHd=halfVboHd;
					if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
					{
						auto unitPtr=bufMan.GetBufferUnit(vboHd);
						if(nullptr!=unitPtr)
						{
							unitPtr->GetActualBuffer()->DrawPrimitiveVtxNom(GL_TRIANGLES);
						}
					}
					glPopMatrix();
				}
			}
			break;
		case FSEXPLOSION_WATERPLUME:
			{
				double t,d,alpha;
				t=YsSmaller(timePassed,0.6)/0.6;

				d=timeRemain+timePassed;
				if(d>YsTolerance && transparency==YSTRUE)
				{
					alpha=0.4+0.6*(timeRemain/d);
				}
				else
				{
					alpha=1.0;
				}

				glPushMatrix();
				glTranslated(pos.x(),pos.y(),pos.z());
				glScaled(radius,height,radius);

				glEnable(GL_LIGHTING);
				float diffuseAmbient[4];
				diffuseAmbient[0]=1.0F;
				diffuseAmbient[1]=1.0F;
				diffuseAmbient[2]=1.0F;
				diffuseAmbient[3]=1.0F;

				if(nullptr==waterPlumeVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(waterPlumeVboHd)->GetState())
				{
					PrepareWaterPlumeVertexArray();
				}

				auto vboHd=waterPlumeVboHd;
				if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
				{
					auto unitPtr=bufMan.GetBufferUnit(vboHd);
					if(nullptr!=unitPtr)
					{
						unitPtr->GetActualBuffer()->DrawPrimitiveVtxNom(GL_TRIANGLE_STRIP);
					}
				}

				glPopMatrix();
			}
			break;
		}
	}
}

void FsExplosionHolder::Draw(const YsVec3 &viewPos,YSBOOL transparency,YSBOOL useOpenGlDisplayList) const
{
	if(transparency==YSTRUE)
	{
		glEnable(GL_CULL_FACE);
	}

	FsExplosion *seeker,*nxt;
	for(seeker=activeList; seeker!=NULL; seeker=nxt)
	{
		nxt=seeker->next;
		seeker->Draw(viewPos,transparency,useOpenGlDisplayList);
	}

	if(transparency==YSTRUE)
	{
		//glDisable(GL_CULL_FACE);
	}
}

