#include "fswirefont.h"

extern int *FsFontSet[];
extern int FsWireFontWidth;
extern int FsWireFontHeight;

void FsMakeWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei)
{
	lineBuf.CleanUp();
	triBuf.CleanUp();
	FsAddWireFontVertexBuffer(lineBuf,lineColBuf,triBuf,triColBuf,x,y,z,str,col,fontWid,fontHei);
}

void FsAddWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei)
{
	const auto x0=x,y0=y;
	for(int i=0; 0<str[i]; i++)
	{
		if(str[i]=='\n' || str[i]=='\a' || str[i]=='\r')
		{
			x=x0;
			y+=fontHei;
			continue;
		}

		auto ptr=FsFontSet[str[i]];
		if(nullptr!=ptr)
		{
			ptr++;  // Skip charactor code
			while(ptr[0]!=-1)
			{
				switch(ptr[0])
				{
				default:
				case 0:
					if(3<=ptr[1])
					{
						float tx0=x+fontWid*(float)ptr[2]/((float)FsWireFontWidth*1.1F);
						float ty0=y+fontHei*(float)ptr[3]/((float)FsWireFontHeight*1.1F);
						for(int j=1; j<ptr[1]-1; j++)
						{
							triBuf.Add(tx0,ty0,z);
							triBuf.Add(
								(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F)),
								z);
							triBuf.Add(
								(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F)),
								z);
							triColBuf.Add(col);
							triColBuf.Add(col);
							triColBuf.Add(col);
						}
					}
					break;
				case 1:
					{
						for(int j=0; j<ptr[1]-1; j++)
						{
							lineBuf.Add(
								(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F)),
								z);
							lineBuf.Add(
								(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F)),
								z);
							lineColBuf.Add(col);
							lineColBuf.Add(col);
						}
					}
					break;
				case 2:
					{
						for(int j=0; j<=ptr[1]-2; j+=2)
						{
							lineBuf.Add(
								(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F)),
								z);
							lineBuf.Add(
								(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F)),
								z);
							lineColBuf.Add(col);
							lineColBuf.Add(col);
						}
					}
					break;
				}
				ptr=ptr+2+ptr[1]*2;
			}
		}
		x+=fontWid;
	}
}

////////////////////////////////////////////////////////////

void FsMakeWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    const YsMatrix4x4 &tfm,const char str[],YsColor col)
{
	lineBuf.CleanUp();
	triBuf.CleanUp();
	FsAddWireFontVertexBuffer(lineBuf,lineColBuf,triBuf,triColBuf,tfm,str,col);
}

void FsAddWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    const YsMatrix4x4 &_tfm,const char str[],YsColor col)
{
	auto tfm=_tfm;
	int lineNum=0;
	for(int i=0; 0<str[i]; i++)
	{
		if(str[i]=='\n' || str[i]=='\a' || str[i]=='\r')
		{
			++lineNum;
			tfm=_tfm;
			for(int j=0; j<lineNum; ++j)
			{
				tfm.Translate(0,1,0);
			}
			continue;
		}

		auto ptr=FsFontSet[str[i]];
		if(nullptr!=ptr)
		{
			ptr++;  // Skip charactor code
			while(ptr[0]!=-1)
			{
				switch(ptr[0])
				{
				default:
				case 0:
					if(3<=ptr[1])
					{
						const YsVec3 vtx0=tfm*YsVec3((float)ptr[2]/((float)FsWireFontWidth*1.1F),(float)ptr[3]/((float)FsWireFontHeight*1.1F),0);
						for(int j=1; j<ptr[1]-1; j++)
						{
							const YsVec3 triVtPos[2]=
							{
								tfm*YsVec3((float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F),(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F),0),
								tfm*YsVec3((float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F),(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F),0)
							};
							triBuf.Add(vtx0);
							triBuf.Add(triVtPos[0]);
							triBuf.Add(triVtPos[1]);
							triColBuf.Add(col);
							triColBuf.Add(col);
							triColBuf.Add(col);
						}
					}
					break;
				case 1:
					{
						for(int j=0; j<ptr[1]-1; j++)
						{
							const YsVec3 lineVtPos[2]=
							{
								tfm*YsVec3((float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F),(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F),0),
								tfm*YsVec3((float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F),(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F),0)
							};
							lineBuf.Add(lineVtPos[0]);
							lineBuf.Add(lineVtPos[1]);
							lineColBuf.Add(col);
							lineColBuf.Add(col);
						}
					}
					break;
				case 2:
					{
						for(int j=0; j<=ptr[1]-2; j+=2)
						{
							const YsVec3 lineVtPos[2]=
							{
								tfm*YsVec3((float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F),(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F),0),
								tfm*YsVec3((float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F),(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F),0)
							};
							lineBuf.Add(lineVtPos[0]);
							lineBuf.Add(lineVtPos[1]);
							lineColBuf.Add(col);
							lineColBuf.Add(col);
						}
					}
					break;
				}
				ptr=ptr+2+ptr[1]*2;
			}
		}
		tfm.Translate(1.0,0.0,0.0);
	}
}

////////////////////////////////////////////////////////////

void FsMakeWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei,
    YSSIDE sideToDraw,YsConstArrayMask <YsVec2> trimPlg,YsRect2 trimPlgBbx)
{
	lineBuf.CleanUp();
	lineColBuf.CleanUp();
	triBuf.CleanUp();
	triColBuf.CleanUp();
	FsAddWireFontVertexBuffer(
	    lineBuf,lineColBuf,
	    triBuf,triColBuf,
	    x,y,z,str,col,fontWid,fontHei,
	    sideToDraw,trimPlg,trimPlgBbx);
}

static void FsAddWireFontVertexBufferLineTrim(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    float x0,float y0,float z0,float x1,float y1,float z1,YsColor col,
    YSSIDE sideToDraw,YsConstArrayMask <YsVec2> trimPlg,YsRect2 trimPlgBbx)
{
	YsArray <YsVec2,2> inside,outside,toDraw;
	YsClipLineByPolygon(inside,outside,YsVec2(x0,y0),YsVec2(x1,y1),trimPlg,trimPlgBbx);
	if(YSINSIDE==sideToDraw)
	{
		toDraw.MoveFrom(inside);
	}
	else if(YSOUTSIDE==sideToDraw)
	{
		toDraw.MoveFrom(outside);
	}
	for(int i=0; i<toDraw.GetN()-1; i+=2)
	{
		lineBuf.Add<double>(toDraw[i  ].x(),toDraw[i  ].y(),z0);
		lineBuf.Add<double>(toDraw[i+1].x(),toDraw[i+1].y(),z1);
		lineColBuf.Add(col);
		lineColBuf.Add(col);
	}
}

void FsAddWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei,
    YSSIDE sideToDraw,YsConstArrayMask <YsVec2> trimPlg,YsRect2 trimPlgBbx)
{
	const auto x0=x,y0=y;
	for(int i=0; 0<str[i]; i++)
	{
		if(str[i]=='\n' || str[i]=='\a' || str[i]=='\r')
		{
			x=x0;
			y+=fontHei;
			continue;
		}

		auto ptr=FsFontSet[str[i]];
		if(nullptr!=ptr)
		{
			ptr++;  // Skip charactor code
			while(ptr[0]!=-1)
			{
				switch(ptr[0])
				{
				default:
				case 0:
					if(3<=ptr[1])
					{
						float tx0,ty0,lastX,lastY;
						for(int j=0; j<ptr[1]; j++)
						{
							float xx=(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F));
							float yy=(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F));
							if(0==j)
							{
								tx0=xx;
								ty0=yy;
							}
							else
							{
								FsAddWireFontVertexBufferLineTrim(
								    lineBuf,lineColBuf,
								    lastX,lastY,z,xx,yy,z,col,
								    sideToDraw,trimPlg,trimPlgBbx);
							}
							lastX=xx;
							lastY=yy;
						}
						FsAddWireFontVertexBufferLineTrim(
						    lineBuf,lineColBuf,
						    lastX,lastY,z,tx0,ty0,z,col,
						    sideToDraw,trimPlg,trimPlgBbx);
					}
					break;
				case 1:
					{
						for(int j=0; j<ptr[1]-1; j++)
						{
							float x0=(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F));
							float y0=(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F));
							float x1=(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F));
							float y1=(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F));

							FsAddWireFontVertexBufferLineTrim(
							    lineBuf,lineColBuf,
							    x0,y0,z,x1,y1,z,col,
							    sideToDraw,trimPlg,trimPlgBbx);
						}
					}
					break;
				case 2:
					{
						for(int j=0; j<=ptr[1]-2; j+=2)
						{
							float x0=(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F));
							float y0=(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F));
							float x1=(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F));
							float y1=(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F));

							FsAddWireFontVertexBufferLineTrim(
							    lineBuf,lineColBuf,
							    x0,y0,z,x1,y1,z,col,
							    sideToDraw,trimPlg,trimPlgBbx);
						}
					}
					break;
				}
				ptr=ptr+2+ptr[1]*2;
			}
		}
		x+=fontWid;
	}
}

////////////////////////////////////////////////////////////

void FsMakeWireFontVertexBuffer2D(
    YsGLVertexBuffer2D &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer2D &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei)
{
	lineBuf.CleanUp();
	triBuf.CleanUp();
	FsAddWireFontVertexBuffer2D(lineBuf,lineColBuf,triBuf,triColBuf,x,y,z,str,col,fontWid,fontHei);
}

void FsAddWireFontVertexBuffer2D(
    YsGLVertexBuffer2D &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer2D &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei)
{
	const auto x0=x,y0=y;
	for(int i=0; 0<str[i]; i++)
	{
		if(str[i]=='\n' || str[i]=='\a' || str[i]=='\r')
		{
			x=x0;
			y+=fontHei;
			continue;
		}

		auto ptr=FsFontSet[str[i]];
		if(nullptr!=ptr)
		{
			ptr++;  // Skip charactor code
			while(ptr[0]!=-1)
			{
				switch(ptr[0])
				{
				default:
				case 0:
					if(3<=ptr[1])
					{
						float tx0=x+fontWid*(float)ptr[2]/((float)FsWireFontWidth*1.1F);
						float ty0=y+fontHei*(float)ptr[3]/((float)FsWireFontHeight*1.1F);
						for(int j=1; j<ptr[1]-1; j++)
						{
							triBuf.Add(tx0,ty0);
							triBuf.Add(
								(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F))
								);
							triBuf.Add(
								(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F))
								);
							triColBuf.Add(col);
							triColBuf.Add(col);
							triColBuf.Add(col);
						}
					}
					break;
				case 1:
					{
						for(int j=0; j<ptr[1]-1; j++)
						{
							lineBuf.Add(
								(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F))
								);
							lineBuf.Add(
								(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F))
								);
							lineColBuf.Add(col);
							lineColBuf.Add(col);
						}
					}
					break;
				case 2:
					{
						for(int j=0; j<=ptr[1]-2; j+=2)
						{
							lineBuf.Add(
								(x+fontWid*(float)ptr[2+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[3+j*2]/((float)FsWireFontHeight*1.1F))
								);
							lineBuf.Add(
								(x+fontWid*(float)ptr[4+j*2]/((float)FsWireFontWidth*1.1F)),
								(y+fontHei*(float)ptr[5+j*2]/((float)FsWireFontHeight*1.1F))
								);
							lineColBuf.Add(col);
							lineColBuf.Add(col);
						}
					}
					break;
				}
				ptr=ptr+2+ptr[1]*2;
			}
		}
		x+=fontWid;
	}
}
