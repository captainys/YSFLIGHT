#include <ysclass.h>
#include "fs.h"

#include <ysgl.h>
#include "fsgl2.0util.h"

static FsGL2VariableVertexStorage vtxBuf;

void FsAircraftCarrierProperty::BeginDrawArrestingWire(void)
{
	vtxBuf.CleanUp();
}

void FsAircraftCarrierProperty::EndDrawArrestingWire(void)
{
	YsGLSL3DRenderer *renderer=YsGLSLSharedFlat3DRenderer();
	YsGLSLUse3DRenderer(renderer);

	GLfloat color[4]={1.0f,1.0f,1.0f,1.0f};
	YsGLSLSet3DRendererUniformColorfv(renderer,color);

	YsGLSLDrawPrimitiveVtxfv(renderer,GL_LINES,vtxBuf.nVtx,vtxBuf.vtxArray);
	YsGLSLEndUse3DRenderer(renderer);
}

YSRESULT FsAircraftCarrierProperty::DrawArrestingWire(void) const
{
	if(drawArrestingWire==YSTRUE)
	{
		YsVec3 wire[2];
		belongTo->GetAttitude().Mul(wire[0],arrestingWirePos[0]);
		belongTo->GetAttitude().Mul(wire[1],arrestingWirePos[1]);
		wire[0]+=belongTo->GetPosition();
		wire[1]+=belongTo->GetPosition();


		for(int i=0; i<airList.GetN(); i++)
		{
			if(airList[i]->Prop().IsAlive()==YSTRUE &&
			   airList[i]->Prop().IsOnArrestingWire()==YSTRUE)
			{
				YsVec3 hook,wireCen,ev;
				airList[i]->Prop().GetTransformedArrestingHookPosition(hook);
				ev=airList[i]->GetAttitude().GetForwardVector();
				wireCen=(wire[0]+wire[1])/2.0;
				if(ev*(hook-wireCen)>0.0)
				{
					vtxBuf.AddVertex((GLfloat)wire[0].x(),(GLfloat)wire[0].y(),(GLfloat)wire[0].z());
					vtxBuf.AddVertex((GLfloat)hook.x(),   (GLfloat)hook.y(),   (GLfloat)hook.z());
					vtxBuf.AddVertex((GLfloat)hook.x(),   (GLfloat)hook.y(),   (GLfloat)hook.z());
					vtxBuf.AddVertex((GLfloat)wire[1].x(),(GLfloat)wire[1].y(),(GLfloat)wire[1].z());
					return YSOK;
				}
			}
		}

		vtxBuf.AddVertex((GLfloat)wire[0].x(),(GLfloat)wire[0].y(),(GLfloat)wire[0].z());
		vtxBuf.AddVertex((GLfloat)wire[1].x(),(GLfloat)wire[1].y(),(GLfloat)wire[1].z());
	}
	return YSOK;
}

