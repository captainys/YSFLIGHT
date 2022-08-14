#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <ysclass.h>

#include <ysgl.h>
#include <ysglcpp.h>
#include <ysglslcpp.h>
#include <ysglbuffermanager_gl2.h>

#include <fsdef.h>

#include "graphics/common/fsopengl.h"
#include "fscloud.h"

#include "fsgl2.0util.h"
#include "fsopengl2.0.h"


class FsCloudsGraphicCache
{
public:
	class CloudCache
	{
	public:
		GLfloat color[4];
		int nVtx;
		YsArray <GLfloat> vtx;
	};
	YsArray <CloudCache> cloudCache;
};

void FsCloud::Draw(void)
{
}


void FsClouds::CreateGraphicCache(void)
{
	res=new FsCloudsGraphicCache;
}

void FsClouds::DeleteGraphicCache(void)
{
	delete res;
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
		glDisable(GL_CULL_FACE);

		auto unitPtr=bufMan.GetBufferUnit(vboHd);
		if(nullptr!=unitPtr)
		{
			YsGLSLPlain3DRenderer renderer;
			unitPtr->GetActualBuffer()->DrawPrimitiveVtxCol(renderer,GL_TRIANGLES);
		}

		glEnable(GL_CULL_FACE);
	}
}

////////////////////////////////////////////////////////////

class FsGL2SolidCloudParticleCache
{
public:
	int nVtx;
	YsArray <GLfloat> vtx;
	YsArray <GLfloat> texCoord;
	YsArray <GLfloat> pointSize;

	GLuint vboId;
	GLuint vtxPtr,texCoordPtr,pointSizePtr;

	FsGL2SolidCloudParticleCache();
	~FsGL2SolidCloudParticleCache();

	void Make(int nParticle,const FsSolidCloud::CloudParticle particle[]);
	void Draw(YsGLSLPointSprite3DRenderer &renderer) const;
};

FsGL2SolidCloudParticleCache::FsGL2SolidCloudParticleCache()
{
	vboId=0;
}
FsGL2SolidCloudParticleCache::~FsGL2SolidCloudParticleCache()
{
	if(0<vboId)
	{
		glDeleteBuffers(1,&vboId);
		vboId=0;
	}
}
void FsGL2SolidCloudParticleCache::Make(int nParticle,const FsSolidCloud::CloudParticle particle[])
{
	nVtx=nParticle;

	vtx.Resize(nVtx*3);
	texCoord.Resize(nVtx*2);
	pointSize.Resize(nVtx);

	for(int i=0; i<nParticle; ++i)
	{
		const FsSolidCloud::CloudParticle &part=particle[i];
		const GLfloat rad=(GLfloat)part.rad;

		vtx[i*3  ]=(GLfloat)part.pos.x();
		vtx[i*3+1]=(GLfloat)part.pos.y();
		vtx[i*3+2]=(GLfloat)part.pos.z();

		texCoord[i*2  ]=0;
		texCoord[i*2+1]=0;

		pointSize[i]=rad*2.0f;
	}



	glGenBuffers(1,&vboId);

	glBindBuffer(GL_ARRAY_BUFFER,vboId);

	unsigned int totalBufSize=(int)sizeof(GLfloat)*(int)(vtx.GetN()+texCoord.GetN()+pointSize.GetN());
	glBufferData(GL_ARRAY_BUFFER,totalBufSize,NULL,GL_STATIC_DRAW);

	unsigned int bufPtr=0;

	vtxPtr=bufPtr;
	glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*vtx.GetN(),vtx);

	bufPtr+=(int)sizeof(GLfloat)*(int)vtx.GetN();;

	texCoordPtr=bufPtr;
	glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*texCoord.GetN(),texCoord);

	bufPtr+=(int)sizeof(GLfloat)*(int)texCoord.GetN();;

	pointSizePtr=bufPtr;
	glBufferSubData(GL_ARRAY_BUFFER,bufPtr,sizeof(GLfloat)*pointSize.GetN(),pointSize);

	bufPtr+=(int)sizeof(GLfloat)*(int)pointSize.GetN();

	glBindBuffer(GL_ARRAY_BUFFER,0);
}

void FsGL2SolidCloudParticleCache::Draw(YsGLSLPointSprite3DRenderer &renderer) const
{
	if(0<vboId)
	{
		glBindBuffer(GL_ARRAY_BUFFER,vboId);
		renderer.DrawVtxTexCoordPointSize(
		    GL_POINTS,
		    nVtx,
		    (GLfloat *)((YSGLSL_POINTER_CAST)vtxPtr),
		    (GLfloat *)((YSGLSL_POINTER_CAST)texCoordPtr),
		    (GLfloat *)((YSGLSL_POINTER_CAST)pointSizePtr));
		glBindBuffer(GL_ARRAY_BUFFER,0);
	}
	else
	{
		renderer.DrawVtxTexCoordPointSize(
		    GL_POINTS,
		    nVtx,
		    vtx,
		    texCoord,
		    pointSize);
	}
}

////////////////////////////////////////////////////////////

class FsSolidCloudGraphicCache
{
public:
	GLfloat color[4];
	int nVtx;
	YsArray <GLfloat> vtxArray;
	YsArray <GLfloat> nomArray;

	GLint particleTexId;
	FsGL2SolidCloudParticleCache particleCache;

	void MakeCache(const class FsSolidCloud &cloud,const GLfloat alpha);
};

void FsSolidCloudGraphicCache::MakeCache(const class FsSolidCloud &cloud,const GLfloat alpha)
{
	if(0<fsNumCloudParticleTex)
	{
		particleTexId=fsCloudParticleTex[rand()%fsNumCloudParticleTex];
	}
	else
	{
		particleTexId=-1;
	}

	vtxArray.Clear();
	nomArray.Clear();

	color[0]=1.0f;
	color[1]=1.0f;
	color[2]=1.0f;
	color[3]=alpha;

	YsShellPolygonHandle plHd=NULL;
	while(NULL!=(plHd=cloud.shl.FindNextPolygon(plHd)))
	{
		int nPlVt;
		const YsShellVertexHandle *plVtHd;
		cloud.shl.GetVertexListOfPolygon(nPlVt,plVtHd,plHd);

		for(int i=nPlVt-1; i>=0; i--)
		{
			YsVec3 nom,pos;
			nom=cloud.shl.GetVertex(plVtHd[i])->GetNormal();
			cloud.shl.GetVertexPosition(pos,plVtHd[i]);

			nomArray.Append((GLfloat)nom.x());
			nomArray.Append((GLfloat)nom.y());
			nomArray.Append((GLfloat)nom.z());

			vtxArray.Append((GLfloat)pos.x());
			vtxArray.Append((GLfloat)pos.y());
			vtxArray.Append((GLfloat)pos.z());
		}
	}

	nVtx=(int)vtxArray.GetN()/3;
	particleCache.Make((int)cloud.particle.GetN(),cloud.particle);
}


void FsSolidClouds::ReduceVisibilityByPolygon(const YsMatrix4x4 &viewTfm,const YsColor &col,YSBOOL /*transparency*/ )
{
	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();

	YsGLSLUse3DRenderer(renderer);


	YsMatrix4x4 invViewTfm;
	invViewTfm=viewTfm;
	invViewTfm.Invert();


	const GLfloat color[4]={col.Rf(),col.Gf(),col.Bf(),0.6f};
	YsGLSLSet3DRendererUniformColorfv(renderer,color);

	const int maxNWallVtx=30;
	int nWallVtx=0;
	GLfloat wallVtx[maxNWallVtx*3];

	for(int z=400; z>=200; z-=50)  // 6 vtx * 5 layers
	{
		YsVec3 quad[4];
		quad[0].Set(z*3,z*3,z);
		invViewTfm.Mul(quad[0],quad[0],1.0);

		quad[1].Set(-z*3,z*3,z);
		invViewTfm.Mul(quad[1],quad[1],1.0);

		quad[2].Set(-z*3,-z*3,z);
		invViewTfm.Mul(quad[2],quad[2],1.0);

		quad[3].Set(z*3,-z*3,z);
		invViewTfm.Mul(quad[3],quad[3],1.0);

		FsGLAddVertex3 <GLfloat> (nWallVtx,wallVtx,quad[0]);
		FsGLAddVertex3 <GLfloat> (nWallVtx,wallVtx,quad[1]);
		FsGLAddVertex3 <GLfloat> (nWallVtx,wallVtx,quad[2]);

		FsGLAddVertex3 <GLfloat> (nWallVtx,wallVtx,quad[2]);
		FsGLAddVertex3 <GLfloat> (nWallVtx,wallVtx,quad[3]);
		FsGLAddVertex3 <GLfloat> (nWallVtx,wallVtx,quad[0]);
	}

	if(maxNWallVtx<nWallVtx)
	{
		printf("Boom!\n");
		printf("%s %d\n",__FUNCTION__,__LINE__);
		exit(1);
	}

	YsGLSLDrawPrimitiveVtxfv(renderer,GL_TRIANGLES,nWallVtx,wallVtx);

	YsGLSLEndUse3DRenderer(renderer);
}

void FsSolidCloud::CreateGraphicCache(void)
{
	res=new FsSolidCloudGraphicCache;
}

void FsSolidCloud::DeleteGraphicCache(void)
{
	delete res;
}

void FsSolidCloud::Draw(FSENVIRONMENT env,const FsWeather & /*weather*/)
{
	const GLfloat alpha=(0<fsNumCloudTex ? 1.0f : 0.863f); // May be alpha-mask is used.

	if(0==res->vtxArray.GetN() || 0==res->nomArray.GetN())
	{
		printf("%s %d\n",__FUNCTION__,__LINE__);
		res->MakeCache(*this,alpha);
		printf("%s %d\n",__FUNCTION__,__LINE__);
	}


	// Now particle drawing is integrated in YsGLParticleManager.
	// This function may be called if useParticle is off.

	// Fall back to the poor-quality cloud.

	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==vboHd || YsGLBufferManager::Unit::EMPTY==bufMan.GetBufferUnit(vboHd)->GetState())
	{
		MakeOpenGlList();
	}

	if(nullptr!=vboHd && YsGLBufferManager::Unit::EMPTY!=bufMan.GetBufferUnit(vboHd)->GetState())
	{
		glDisable(GL_CULL_FACE);

		auto unitPtr=bufMan.GetBufferUnit(vboHd);
		if(nullptr!=unitPtr)
		{
			YsGLSLShaded3DRenderer renderer;
			unitPtr->GetActualBuffer()->DrawPrimitiveVtxNomCol(renderer,GL_TRIANGLES);
		}

		glEnable(GL_CULL_FACE);
	}
}

void FsSolidClouds::Test(void)
{
}

void FsSolidClouds::SetUpCloudPerFrame(void)
{
/* It looks to take substantial computational time.
   The lighting calculation can be parallelized.  Maybe it is too early for the next version.
	YsGLSL3DRenderer *renderer=YsGLSLSharedVariColorShaded3DRenderer();
	YsGLSLUse3DRenderer(renderer);
	glActiveTexture(GL_TEXTURE0);
	for(int i=0; i<fsNumCloudParticleTex; ++i)
	{
		cloudParticleTexBuf[i].CopyFrom(cloudParticleTexSrc[i]);
		cloudParticleTexBuf[i].ApplySphereLighting(renderer);

		glBindTexture(GL_TEXTURE_2D,fsCloudParticleTex[i]);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

		glTexImage2D
		    (GL_TEXTURE_2D,
		     0,
		     4,
		     cloudParticleTexBuf[i].GetWidth(),
		     cloudParticleTexBuf[i].GetHeight(),
		     0,
		     GL_RGBA,
		     GL_FLOAT,
		     cloudParticleTexBuf[i].GetRGBAPointer());
	}
	YsGLSLEndUse3DRenderer(renderer); */
}

void FsSolidClouds::BeginDrawCloud(void)
{
}

void FsSolidClouds::EndDrawCloud(void)
{
}
