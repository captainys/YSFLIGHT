#include <ysclass.h>
#include "fs.h"

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

void FsAircraftCarrierProperty::BeginDrawArrestingWire(void)
{
}

void FsAircraftCarrierProperty::EndDrawArrestingWire(void)
{
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
					glDisable(GL_LIGHTING);
					glColor3d(1.0,1.0,1.0);
					glBegin(GL_LINE_STRIP);
					glVertex3dv(wire[0]);
					glVertex3dv(hook);
					glVertex3dv(wire[1]);
					glEnd();
					return YSOK;
				}
			}
		}

		glDisable(GL_LIGHTING);
		glColor3d(1.0,1.0,1.0);
		glBegin(GL_LINES);
		glVertex3dv(wire[0]);
		glVertex3dv(wire[1]);
		glEnd();
	}
	return YSOK;
}

