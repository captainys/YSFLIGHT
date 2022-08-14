#include <ysclass.h>
#include "fs.h"
#include "graphics/common/fsopengl.h"

#include "fshud2.h"

void FsHud2::TakeSpecificResource(void)
{
}

void FsHud2::ReleaseSpecificResource(void)
{
}



void FsHud2::BeginDrawHud(const YsVec3 &viewPos,const YsAtt3 &airAtt)
{
	pointVtxBuf.CleanUp();
	pointColBuf.CleanUp();
	lineVtxBuf.CleanUp();
	lineColBuf.CleanUp();
	triVtxBuf.CleanUp();
	triColBuf.CleanUp();
}

void FsHud2::EndDrawHud(void)
{
}

