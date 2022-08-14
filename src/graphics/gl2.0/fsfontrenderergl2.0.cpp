#include "graphics/common/fsfontrenderer.h"
#include "graphics/common/fsopengl.h"

#include <ysglfontdata.h>
#include <yssystemfont.h>

#include <ysgl.h>
#include <ysglsldrawfontbitmap.h>

#include "fsopengl2.0.h"

YSRESULT FsDirectFixedFontRenderer::RequestDefaultFontWithPixelHeight(unsigned int fontHeight)
{
	auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
	fontPtr=YsGlSelectFontBitmapPointerByHeight(&fontWid,&fontHei,fontHeight);
	YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
	YsGLSLBitmapFontRendererRequestFontSize(fsBitmapFontRenderer,fontWid,fontHei);
	YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);
	return YSERR;
}

YSRESULT FsDirectFixedFontRenderer::RenderAsciiStringSingleLine(int leftX,int bottomY,const char str[],const YsColor &col)
{
	auto fsBitmapFontRenderer=YsGLSLSharedBitmapFontRenderer();
	YsGLSLUseBitmapFontRenderer(fsBitmapFontRenderer);
	YsGLSLSetBitmapFontRendererColor3f(fsBitmapFontRenderer,col.Rf(),col.Gf(),col.Bf());
	YsGLSLRenderBitmapFontString2D(fsBitmapFontRenderer,leftX,bottomY,str);
	YsGLSLEndUseBitmapFontRenderer(fsBitmapFontRenderer);
	return YSOK;
}
