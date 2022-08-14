#include <ysclass.h>
#include "fs.h"

#include <ysgl.h>
#include "fsgl2.0util.h"


class FsWeaponGraphicCache
{
public:
	// Rendered in EndDraw>>
	FsGL2VariableVertexStorage lineVtxColBuf;
	FsGL2VariableVertexStorage pntVtxBuf;
	FsGL2VariableVertexStorage triVtxNomBuf;
	// Rendered in EndDraw<<

	FsGL2VariableVertexStorage flareVtxColBuf;
	FsGL2VariableVertexStorage trailBuf;

	void CleanUp(void);
};

void FsWeaponGraphicCache::CleanUp(void)
{
	lineVtxColBuf.CleanUp();
	pntVtxBuf.CleanUp();
	triVtxNomBuf.CleanUp();
}

static FsWeaponGraphicCache *fsWeaponGraphicCache=NULL;

void FsWeaponSmokeTrail::Draw(const YsVec3 &pos,const YsAtt3 &att,YSBOOL,FSSMOKETYPE smk,const double &cTime) const
// transparency==YSTRUE always.
{
	YsGLVertexBuffer vtxBuf[4];
	YsGLNormalBuffer nomBuf[4];
	YsGLColorBuffer colBuf[4];

	this->MakeVertexArray(
	    vtxBuf,nomBuf,colBuf,
	    pos,att,smk,cTime);


	YsGLSL3DRenderer *renderer=YsGLSLSharedVariColorPerVtxShading3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	for(int i=0; i<4; ++i)
	{
		YsGLSLDrawPrimitiveVtxNomColfv(
		    renderer,
			GL_TRIANGLE_STRIP,
			vtxBuf[i].size(),
			vtxBuf[i].data(),
			nomBuf[i].data(),
			colBuf[i].data());
	}

	YsGLSLEndUse3DRenderer(renderer);

	glEnable(GL_CULL_FACE);
}

void FsWeapon::Draw(
    YSBOOL coarse,const YsMatrix4x4 &viewMat,const YsMatrix4x4 &projMat,
    YSBOOL transparency,FSSMOKETYPE smk,const double &cTime,unsigned int drawFlag) const
{
	if(NULL==fsWeaponGraphicCache)
	{
		fsWeaponGraphicCache=new FsWeaponGraphicCache;
	}

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
		default:
			break;
		case FSWEAPON_GUN:
			fsWeaponGraphicCache->lineVtxColBuf.AddVertex(pos);
			fsWeaponGraphicCache->lineVtxColBuf.AddColor(1.0f,1.0f,0.0f,1.0f);
			fsWeaponGraphicCache->lineVtxColBuf.AddVertex(pos+vec/velocity*10.0);
			fsWeaponGraphicCache->lineVtxColBuf.AddColor(1.0f,1.0f,1.0f,1.0f);
			break;
		case FSWEAPON_AIM9:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && aim9!=NULL)
				{
					aim9.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && aim_coarse!=NULL)
				{
					aim_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AIM9X:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && aim9x!=NULL)
				{
					aim9x.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && aim_coarse!=NULL)
				{
					aim9x_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AIM120:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && aim120!=NULL)
				{
					aim120.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && aim120_coarse!=NULL)
				{
					aim120_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && bomb!=NULL)
				{
					bomb.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && bomb_coarse!=NULL)
				{
					bomb_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB250:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && bomb!=NULL)
				{
					bomb250.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && bomb_coarse!=NULL)
				{
					bomb250_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_BOMB500HD:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && bomb!=NULL)
				{
					bomb500hd.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && bomb_coarse!=NULL)
				{
					bomb250_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_AGM65:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && agm65!=NULL)
				{
					agm65.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && agm_coarse!=NULL)
				{
					agm_coarse.Draw(viewMat,projMat,pos,att,drawFlag);
				}
			}
			break;
		case FSWEAPON_ROCKET:
			if(lifeRemain>YsTolerance)
			{
				// glDisable(GL_CULL_FACE); Do I have to?
				if(coarse!=YSTRUE && rocket!=NULL)
				{
					rocket.Draw(viewMat,projMat,pos,att,drawFlag);
				}
				else if(coarse==YSTRUE && rocket_coarse!=NULL)
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
			{
				fsWeaponGraphicCache->pntVtxBuf.AddVertex(pos);

				YsVec3 u(1.0,0.0,0.0),v(0.0,0.0,1.0),nom(0.0,1.0,0.0);
				att.Mul(u,u);
				att.Mul(v,v);
				att.Mul(nom,nom);

				fsWeaponGraphicCache->triVtxNomBuf.AddVertex(pos+v);
				fsWeaponGraphicCache->triVtxNomBuf.AddNormal(nom);
				fsWeaponGraphicCache->triVtxNomBuf.AddVertex(pos-u);
				fsWeaponGraphicCache->triVtxNomBuf.AddNormal(nom);
				fsWeaponGraphicCache->triVtxNomBuf.AddVertex(pos+u);
				fsWeaponGraphicCache->triVtxNomBuf.AddNormal(nom);

				fsWeaponGraphicCache->triVtxNomBuf.AddVertex(pos-v);
				fsWeaponGraphicCache->triVtxNomBuf.AddNormal(nom);
				fsWeaponGraphicCache->triVtxNomBuf.AddVertex(pos+u);
				fsWeaponGraphicCache->triVtxNomBuf.AddNormal(nom);
				fsWeaponGraphicCache->triVtxNomBuf.AddVertex(pos-u);
				fsWeaponGraphicCache->triVtxNomBuf.AddNormal(nom);
			}
			break;
		}

		if(FSSMKNULL!=smk &&
		   0!=(drawFlag&FSVISUAL_DRAWTRANSPARENT) &&
		   trail!=nullptr)
		{
			trail->Draw(pos,att,transparency,smk,cTime);
		}
	}
}

////////////////////////////////////////////////////////////

void FsWeaponHolder::BeginDraw(void) const
{
	if(NULL==fsWeaponGraphicCache)
	{
		fsWeaponGraphicCache=new FsWeaponGraphicCache;
	}
	fsWeaponGraphicCache->CleanUp();
}

void FsWeaponHolder::EndDraw(void) const
{
	YsGLSL3DRenderer *renderer;

	renderer=YsGLSLSharedVariColor3DRenderer();
	YsGLSLUse3DRenderer(renderer);
	YsGLSLDrawPrimitiveVtxColfv(
	    renderer,GL_LINES,
	    fsWeaponGraphicCache->lineVtxColBuf.nVtx,
	    fsWeaponGraphicCache->lineVtxColBuf.vtxArray,
	    fsWeaponGraphicCache->lineVtxColBuf.colArray);
	YsGLSLEndUse3DRenderer(renderer);

	const GLfloat debrisColor[4]={0.125f,0.125f,0.125f,1.0f};
	renderer=YsGLSLSharedFlat3DRenderer();
	YsGLSLUse3DRenderer(renderer);
	YsGLSLSet3DRendererUniformColorfv(renderer,debrisColor);
	YsGLSLDrawPrimitiveVtxfv(
	    renderer,GL_POINTS,
	    fsWeaponGraphicCache->pntVtxBuf.nVtx,
	    fsWeaponGraphicCache->pntVtxBuf.vtxArray);
	YsGLSLEndUse3DRenderer(renderer);

	glDisable(GL_CULL_FACE);

	renderer=YsGLSLSharedMonoColorPerVtxShading3DRenderer();
	YsGLSLUse3DRenderer(renderer);
	YsGLSLSet3DRendererUniformColorfv(renderer,debrisColor);
	YsGLSLDrawPrimitiveVtxNomfv(
		renderer,GL_TRIANGLES,
		fsWeaponGraphicCache->triVtxNomBuf.nVtx,
		fsWeaponGraphicCache->triVtxNomBuf.vtxArray,
		fsWeaponGraphicCache->triVtxNomBuf.nomArray);
	YsGLSLEndUse3DRenderer(renderer);

	glEnable(GL_CULL_FACE);
}

void FsWeaponHolder::DrawGunCalibrator(void) const
{
	if(0<bulletCalibrator.GetN())
	{
		YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();

#ifdef GL_PROGRAM_POINT_SIZE
		glEnable(GL_PROGRAM_POINT_SIZE);  // Needed for enabling gl_PointSize in the vertex shader.
		glEnable(GL_POINT_SPRITE);        // Needed for enabling gl_PointCoord in the fragment shader.
#endif
		YsGLSLSet3DRendererPointSizeMode(renderer,YSGLSL_POINTSPRITE_SIZE_IN_PIXEL);
		YsGLSLSet3DRendererUniformPointSize(renderer,4);

		YsGLSLUse3DRenderer(renderer);
		const GLfloat color[4]={1.0f,0.0f,0.0f,1.0f};
		YsGLSLSet3DRendererUniformColorfv(renderer,color);

		const int maxNVtx=32;
		int nVtx=0;
		GLfloat vtx[maxNVtx];

		for(int i=0; i<bulletCalibrator.GetN(); i++)
		{
			const YsVec3 &nearPos=bulletCalibrator[i];
			FsGLAddVertex3 <GLfloat> (nVtx,vtx,nearPos);
			if(maxNVtx<=nVtx)
			{
				YsGLSLDrawPrimitiveVtxfv(renderer,GL_POINTS,nVtx,vtx);
				nVtx=0;
			}
		}
		if(0<nVtx)
		{
			YsGLSLDrawPrimitiveVtxfv(renderer,GL_POINTS,nVtx,vtx);
			nVtx=0;
		}

		YsGLSLSet3DRendererUniformPointSize(renderer,1);
		YsGLSLEndUse3DRenderer(renderer);

#ifdef GL_PROGRAM_POINT_SIZE
		glDisable(GL_PROGRAM_POINT_SIZE);
		glDisable(GL_POINT_SPRITE);
#endif
	}
}

