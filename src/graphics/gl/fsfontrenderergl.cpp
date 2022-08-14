#include "graphics/common/fsfontrenderer.h"
#include "graphics/common/fsopengl.h"

#include <ysglfontdata.h>
#include <yssystemfont.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif



extern int ysScnFontBitmapBase;  // defined in ysscenerygl.cpp

YSRESULT FsDirectFixedFontRenderer::RequestDefaultFontWithPixelHeight(unsigned int fontHeight)
{
	ysScnFontBitmapBase=FS_GL_FONT_BITMAP_BASE;

	fontPtr=YsGlSelectFontBitmapPointerByHeight(&fontWid,&fontHei,fontHeight);
	if(NULL!=fontPtr)
	{
		YsGlMakeFontBitmapDisplayList(FS_GL_FONT_BITMAP_BASE,fontPtr,fontWid,fontHei);
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsDirectFixedFontRenderer::RenderAsciiStringSingleLine(int leftX,int bottomY,const char str[],const YsColor &col)
{
	glColor3d(col.Rd(),col.Gd(),col.Bd());
	glRasterPos2i(leftX,bottomY);

	glListBase(FS_GL_FONT_BITMAP_BASE);
	glCallLists((GLsizei)strlen(str),GL_UNSIGNED_BYTE,str);
	glListBase(0);

	return YSOK;
}
