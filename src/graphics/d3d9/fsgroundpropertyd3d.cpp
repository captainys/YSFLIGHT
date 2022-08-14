#include <ysclass.h>
#include "fs.h"

#include <windows.h>
#include <mmsystem.h>

#include <ysd3d9.h>



void FsAircraftCarrierProperty::BeginDrawArrestingWire(void)
{
}

void FsAircraftCarrierProperty::EndDrawArrestingWire(void)
{
}

YSRESULT FsAircraftCarrierProperty::DrawArrestingWire(void) const
{
	auto ysD3dDev=YsD3dDevice::GetCurrent();
	if(drawArrestingWire==YSTRUE)
	{
		YsVec3 wire[2];
		belongTo->GetAttitude().Mul(wire[0],arrestingWirePos[0]);
		belongTo->GetAttitude().Mul(wire[1],arrestingWirePos[1]);
		wire[0]+=belongTo->GetPosition();
		wire[1]+=belongTo->GetPosition();


		int i;
		for(i=0; i<airList.GetN(); i++)
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
					ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
					ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,wire[0],255,255,255,255);
					ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,hook   ,255,255,255,255);
					ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,wire[1],255,255,255,255);
					ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);

					// glColor3d(1.0,1.0,1.0);
					// glBegin(GL_LINE_STRIP);
					// glVertex3dv(wire[0]);
					// glVertex3dv(hook);
					// glVertex3dv(wire[1]);
					// glEnd();
					return YSOK;
				}
			}
		}

		ysD3dDev->d3dDev->SetRenderState(D3DRS_LIGHTING,FALSE);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,wire[0],255,255,255,255);
		ysD3dDev->AddXyzCol(D3DPT_LINESTRIP,wire[1],255,255,255,255);
		ysD3dDev->FlushXyzCol(D3DPT_LINESTRIP);
		// glDisable(GL_LIGHTING);
		// glColor3d(1.0,1.0,1.0);
		// glBegin(GL_LINES);
		// glVertex3dv(wire[0]);
		// glVertex3dv(wire[1]);
		// glEnd();
	}
	return YSOK;
}

