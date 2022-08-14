#ifndef FSGL_BITMAP_IS_INCLUDED
#define FSGL_BITMAP_IS_INCLUDED
/* { */

#include <ysgl.h>

class FsGL2Bitmap
{
private:
	int w,h;
	GLfloat *rgba;
public:
	FsGL2Bitmap();
	FsGL2Bitmap(const FsGL2Bitmap &from);
	~FsGL2Bitmap();
	void CleanUp();

	const FsGL2Bitmap &operator=(const FsGL2Bitmap &from);

	int GetWidth(void) const;
	int GetHeight(void) const;
	const GLfloat *GetRGBAPointer(void) const;

	void MakeEmpty(int w,int h);
	void MakeFromYsBitmap(const class YsBitmap &bmp);
	void CopyFrom(const FsGL2Bitmap &from);
	void ApplySphereLighting(struct YsGLSL3DRenderer *rendererWithLight);
};


/* } */
#endif
