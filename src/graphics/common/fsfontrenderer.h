#ifndef FSFONTRENDERER_IS_INCLUDED
#define FSFONTRENDERER_IS_INCLUDED
/* { */

#include <ysglfontdata.h>
#include <ysfontrenderer.h>



class FsDirectFixedFontRenderer
{
private:
	const unsigned char * const *fontPtr;
	int fontWid,fontHei;

public:
	int GetFontHeight(void) const;
	int GetFontWidth(void) const;

	YSRESULT RequestDefaultFontWithPixelHeight(unsigned int heightInPix);
	YSRESULT RemakeFont(void);
	YSRESULT RenderAsciiStringSingleLine(int leftX,int bottomY,const char str[],const YsColor &col);
	YSRESULT RenderAsciiString(int leftX,int bottomY,const char str[],const YsColor &col);
};

extern YsSystemFontRenderer fsUnicodeRenderer;
extern YsFixedFontRenderer fsAsciiRenderer;
extern FsDirectFixedFontRenderer fsDirectFixedFontRenderer;

extern YSRESULT FsSetFont(const char fontName[],int fontHeight);

/* } */
#endif
