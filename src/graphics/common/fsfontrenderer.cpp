#include "graphics/common/fsfontrenderer.h"
#include <yssystemfont.h>


YsSystemFontRenderer fsUnicodeRenderer;
YsFixedFontRenderer fsAsciiRenderer;
FsDirectFixedFontRenderer fsDirectFixedFontRenderer;



////////////////////////////////////////////////////////////



int FsDirectFixedFontRenderer::GetFontHeight(void) const
{
	return fontHei;
}

int FsDirectFixedFontRenderer::GetFontWidth(void) const
{
	return fontWid;
}

YSRESULT FsDirectFixedFontRenderer::RemakeFont(void)
{
	return RequestDefaultFontWithPixelHeight(fontHei);
}

YSRESULT FsDirectFixedFontRenderer::RenderAsciiString(int leftX,int bottomY,const char str[],const YsColor &col)
{
	int nWid,nHei;
	YsFontRenderer::CountStringDimension(nWid,nHei,str);

	if(1==nHei)
	{
		return RenderAsciiStringSingleLine(leftX,bottomY,str,col);
	}

	const int fontHei=GetFontHeight();

	int i=0,y=bottomY-(nHei-1)*fontHei;
	YsString buf;
	for(;;)
	{
		if(0==str[i] || '\n'==str[i])
		{
			RenderAsciiStringSingleLine(leftX,y,buf,col);
			y+=fontHei;
			buf.Set("");
		}
		else
		{
			buf.Append(str[i]);
		}

		if(0==str[i])
		{
			break;
		}
		i++;
	}

	return YSOK;
}

YSRESULT FsSetFont(const char /*fontName*/[],int fontHeight)
{
	fsUnicodeRenderer.RequestDefaultFontWithPixelHeight(fontHeight);
	fsAsciiRenderer.RequestDefaultFontWithPixelHeight(fontHeight);
	fsDirectFixedFontRenderer.RequestDefaultFontWithPixelHeight(fontHeight);
	return YSOK;
}

