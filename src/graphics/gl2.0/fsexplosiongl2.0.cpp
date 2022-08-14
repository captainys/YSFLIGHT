#include <ysclass.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"

#include <ysgl.h>
#include <ysglcpp.h>
#include <ysglslcpp.h>
#include <ysglbuffermanager_gl2.h>
#include "fsgl2.0util.h"
#include "fsopengl2.0.h"

void FsExplosion::Draw(const YsVec3 &viewPos,YSBOOL /*transparency*/ ,YSBOOL useOpenGlDisplayList) const
{
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(timeRemain>=YsTolerance)
	{
		switch(expType)
		{
		case FSEXPLOSION_FIREBALL:
			{
				GLfloat r,alpha;
				GLfloat slowTimer=YsSmaller <GLfloat> ((GLfloat)timePassed,1.0f);
				GLfloat fastTimer=YsSmaller <GLfloat> ((GLfloat)timePassed,0.6f)/0.6f;

				int texId=-1;

				{
					const double d=timeRemain+timePassed;
					if(d>YsTolerance)
					{
						alpha=(GLfloat)(0.8+0.6*(timeRemain/d));
						if(1.0f<alpha)
						{
							alpha=1.0f;
						}
					}
					else
					{
						alpha=1.0f;
					}
				}

				GLfloat color[4]={1.0f,1.0f,1.0f,alpha};
				if(flash==YSTRUE && timePassed<=0.05)
				{
					r=(GLfloat)radius/2.0f;
					if(0<fsNumFlashTex)
					{
						texId=fsFlashTex[random%fsNumFlashTex];
					}
				}
				else
				{
					r=(GLfloat)(iniRadius+(radius-iniRadius)*fastTimer);
					if(0<fsNumExplosionTex)
					{
						texId=fsExplosionTex[random%fsNumExplosionTex];
					}
					// Starting from red go dark
					GLfloat decay=slowTimer;
					if(YSTRUE==flash)
					{
						decay=(decay-0.05f)/0.95f;
					}
					decay=decay*decay*decay;
					color[0]=(float)(1.0-decay*0.78);
					color[1]=(float)(decay*0.22);
					color[2]=(float)(decay*0.22);
				}


				YsGLSLShaded3DRenderer renderer;
				renderer.SetUniformColor(color);

				if(0<=texId)
				{
					glActiveTexture(GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D,texId);
					renderer.SetTextureIdent(0);
					renderer.SetTextureType(YSGLSL_TEX_TYPE_BILLBOARD);
				}
				else
				{
					renderer.SetTextureType(YSGLSL_TEX_TYPE_NONE);
				}

				YSBOOL inside=YSFALSE;
				GLfloat billBoardRadiusCorrection=1.0;
				if((viewPos-pos).GetSquareLength()<r*r)
				{
					inside=YSTRUE;
					billBoardRadiusCorrection=2.0;
					// 2012/12/26
					// If the viewpoint is inside the fireball, the area on which texture covers
					// will be reduced because the far-polygons will be visible.  As a result
					// the apparent area will be even smaller, and only fraction of the area
					// will be texture-mapped.  To prevent this, billBoard must cover larger 
					// area when the viewpoint is inside.
				}
				else
				{
					inside=YSFALSE;
					billBoardRadiusCorrection=0.8f;
				}

				if((nullptr==fireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(fireballVboHd)->GetState()) ||
				   (nullptr==halfFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(halfFireballVboHd)->GetState()) ||
				   (nullptr==reverseFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(reverseFireballVboHd)->GetState()) ||
				   (nullptr==reverseHalfFireballVboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(reverseHalfFireballVboHd)->GetState()))
				{
					PrepareFireballVertexArray();
				}

				YsGLBufferManager::Handle vboHd=nullptr;

				GLfloat y;
				if(pos.y()>r)
				{
					y=(GLfloat)pos.y();
					if(YSTRUE!=inside)
					{
						vboHd=reverseFireballVboHd;
					}
					else
					{
						vboHd=fireballVboHd;
					}
				}
				else
				{
					y=0.0f;
					if(YSTRUE!=inside)
					{
						vboHd=reverseHalfFireballVboHd;
					}
					else
					{
						vboHd=halfFireballVboHd;
					}
				}

				GLfloat prevTfm[16];
				renderer.GetModelView(prevTfm);

				const GLfloat center[3]={0,0,0};
				const GLfloat dimension[2]={r*billBoardRadiusCorrection,r*billBoardRadiusCorrection};
				YsGLSLSet3DRendererBillBoardfv(renderer,center,dimension);

				GLfloat tfm[16];
				YsGLCopyMatrixfv(tfm,prevTfm);
				YsGLMultMatrixTranslationfv(tfm,pos.xf(),y,pos.zf());
				YsGLMultMatrixScalingfv(tfm,r,r,r);
				renderer.SetModelView(tfm);

				if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
				{
					auto unitPtr=bufMan.GetBufferUnit(vboHd);
					if(nullptr!=unitPtr)
					{
						unitPtr->GetActualBuffer()->DrawPrimitiveVtxNom(renderer,GL_TRIANGLES);
					}
				}

				renderer.SetModelView(prevTfm);

				renderer.SetTextureType(YSGLSL_TEX_TYPE_NONE);
			}
			break;
		case FSEXPLOSION_WATERPLUME:
			{
				GLfloat alpha;
				GLfloat t=YsSmaller((GLfloat)timePassed,0.6f)/0.6f;

				GLfloat d=(GLfloat)(timeRemain+timePassed);
				if(d>YsTolerance)
				{
					alpha=(GLfloat)(0.4+0.6*(timeRemain/d));
				}
				else
				{
					alpha=1.0;
				}

				GLfloat color[4]={1.0f,1.0f,1.0f,alpha};

				YsGLSLShaded3DRenderer renderer;

				renderer.SetUniformColor(color);

				GLfloat prevTfm[16];
				renderer.GetModelView(prevTfm);

				GLfloat tfm[16];
				YsGLCopyMatrixfv(tfm,prevTfm);
				YsGLMultMatrixTranslationfv(tfm,pos.xf(),pos.yf(),pos.zf());
				YsGLMultMatrixScalingfv(tfm,(GLfloat)radius,(GLfloat)height,(GLfloat)radius);
				renderer.SetModelView(tfm);

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
						unitPtr->GetActualBuffer()->DrawPrimitiveVtxNom(renderer,GL_TRIANGLE_STRIP);
					}
				}

				renderer.SetModelView(prevTfm);
			}
			break;
		}
	}
}

void FsExplosionHolder::Draw(const YsVec3 &viewPos,YSBOOL /*transparency*/ ,YSBOOL useOpenGlDisplayList) const
{
	glEnable(GL_CULL_FACE);

	FsExplosion *seeker,*nxt;
	for(seeker=activeList; seeker!=NULL; seeker=nxt)
	{
		nxt=seeker->next;
		seeker->Draw(viewPos,/* transparency is always true */YSTRUE,useOpenGlDisplayList);
	}
}

