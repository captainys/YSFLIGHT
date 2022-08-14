#ifndef FSGL2UTIL_IS_INCLUDED
#define FSGL2UTIL_IS_INCLUDED
/* { */

template <class T>
static inline void FsGLAddVertex3(int &nVtx,T vtx[],T x,T y,T z)
{
	vtx[nVtx*3  ]=x;
	vtx[nVtx*3+1]=y;
	vtx[nVtx*3+2]=z;
	++nVtx;
}

template <class T>
static inline void FsGLAddVertex3(int &nVtx,T vtx[],const YsVec3 &pos)
{
	vtx[nVtx*3  ]=(T)pos.x();
	vtx[nVtx*3+1]=(T)pos.y();
	vtx[nVtx*3+2]=(T)pos.z();
	++nVtx;
}

template <class T>
static inline void FsGLAddVertex3(int &nVtx,T vtx[],const YsVec2 &pos)
{
	vtx[nVtx*3  ]=(T)pos.x();
	vtx[nVtx*3+1]=(T)pos.y();
	vtx[nVtx*3+2]=(T)0.0;
	++nVtx;
}

template <class T>
static inline void FsGLSetColor(int nVtx,T col[],T r,T g,T b,T a)
{
	col[nVtx*4  ]=r;
	col[nVtx*4+1]=g;
	col[nVtx*4+2]=b;
	col[nVtx*4+3]=a;
}


class FsGL2VariableVertexStorage
{
public:
	int nVtx;
	YsArray <GLfloat> vtxArray;
	int nNom;
	YsArray <GLfloat> nomArray;
	int nCol;
	YsArray <GLfloat> colArray;
	int nVOffset;
	YsArray <GLfloat> vOffsetArray;
	int nTexCoord;
	YsArray <GLfloat> texCoordArray;

	inline void CleanUp(void)
	{
		nNom=0;
		nCol=0;
		nVtx=0;
		nVOffset=0;
		nTexCoord=0;
	};
	template <class T>
	inline void AddVertex(T x,T y,T z)
	{
		if(nVtx*3+3>vtxArray.GetN())
		{
			vtxArray.Resize(nVtx*3+3);
		}
		vtxArray[nVtx*3  ]=(GLfloat)x;
		vtxArray[nVtx*3+1]=(GLfloat)y;
		vtxArray[nVtx*3+2]=(GLfloat)z;
		++nVtx;
	}
	inline void AddVertex(const YsVec3 &pos)
	{
		if(nVtx*3+3>vtxArray.GetN())
		{
			vtxArray.Resize(nVtx*3+3);
		}
		vtxArray[nVtx*3  ]=(GLfloat)pos.x();
		vtxArray[nVtx*3+1]=(GLfloat)pos.y();
		vtxArray[nVtx*3+2]=(GLfloat)pos.z();
		++nVtx;
	}

	template <class T>
	inline void AddNormal(T x,T y,T z)
	{
		if(nNom*3+3>nomArray.GetN())
		{
			nomArray.Resize(nNom*3+3);
		}
		nomArray[nNom*3  ]=(GLfloat)x;
		nomArray[nNom*3+1]=(GLfloat)y;
		nomArray[nNom*3+2]=(GLfloat)z;
		++nNom;
	}
	inline void AddNormal(const YsVec3 &nom)
	{
		if(nNom*3+3>nomArray.GetN())
		{
			nomArray.Resize(nNom*3+3);
		}
		nomArray[nNom*3  ]=(GLfloat)nom.x();
		nomArray[nNom*3+1]=(GLfloat)nom.y();
		nomArray[nNom*3+2]=(GLfloat)nom.z();
		++nNom;
	}

	template <class T>
	inline void AddViewOffset3(T x,T y,T z)
	{
		if(nVOffset*3+3>vOffsetArray.GetN())
		{
			vOffsetArray.Resize(nVOffset*3+3);
		}
		vOffsetArray[nVOffset*3  ]=x;
		vOffsetArray[nVOffset*3+1]=y;
		vOffsetArray[nVOffset*3+2]=z;
		++nVOffset;
	}
	inline void AddViewOffset3(const YsVec3 &vOffset)
	{
		if(nVOffset*3+3>vOffsetArray.GetN())
		{
			vOffsetArray.Resize(nVOffset*3+3);
		}
		vOffsetArray[nVOffset*3  ]=(GLfloat)vOffset.x();
		vOffsetArray[nVOffset*3+1]=(GLfloat)vOffset.y();
		vOffsetArray[nVOffset*3+2]=(GLfloat)vOffset.z();
		++nVOffset;
	}

	template <class T>
	inline void AddTexCoord2(T x,T y)
	{
		if(nTexCoord*2+2>texCoordArray.GetN())
		{
			texCoordArray.Resize(nTexCoord*2+2);
		}
		texCoordArray[nTexCoord*2  ]=x;
		texCoordArray[nTexCoord*2+1]=y;
		++nTexCoord;
	}
	inline void AddTexCoord2(const YsVec2 &texCoord)
	{
		if(nTexCoord*2+2>texCoordArray.GetN())
		{
			texCoordArray.Resize(nTexCoord*2+2);
		}
		texCoordArray[nTexCoord*2  ]=(GLfloat)texCoord.x();
		texCoordArray[nTexCoord*2+1]=(GLfloat)texCoord.y();
		++nTexCoord;
	}


	template <class T>
	inline void AddColor(T r,T g,T b,T a)
	{
		if(nCol*4+4>colArray.GetN())
		{
			colArray.Resize(nCol*4+4);
		}
		colArray[nCol*4  ]=(GLfloat)r;
		colArray[nCol*4+1]=(GLfloat)g;
		colArray[nCol*4+2]=(GLfloat)b;
		colArray[nCol*4+3]=(GLfloat)a;
		++nCol;
	}
};


class FsGL2DisableCulling
{
public:
	inline FsGL2DisableCulling()
	{
		glDisable(GL_CULL_FACE);
	}
	inline ~FsGL2DisableCulling()
	{
		glEnable(GL_CULL_FACE);
	}
};


/* } */
#endif
