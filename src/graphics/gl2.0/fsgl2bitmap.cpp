#include <stdio.h>
#include <ysgl.h>
#include <ysbitmap.h>

#include "fsgl2bitmap.h"

FsGL2Bitmap::FsGL2Bitmap()
{
	rgba=NULL;
	CleanUp();
}

FsGL2Bitmap::FsGL2Bitmap(const FsGL2Bitmap &from)
{
	rgba=NULL;
	CopyFrom(from);
}

FsGL2Bitmap::~FsGL2Bitmap()
{
	CleanUp();
}

void FsGL2Bitmap::CleanUp()
{
	if(NULL!=rgba)
	{
		delete [] rgba;
		rgba=NULL;
	}
	w=0;
	h=0;
}


const FsGL2Bitmap &FsGL2Bitmap::operator=(const FsGL2Bitmap &from)
{
	CopyFrom(from);
	return *this;
}

int FsGL2Bitmap::GetWidth(void) const
{
	return w;
}

int FsGL2Bitmap::GetHeight(void) const
{
	return h;
}

const GLfloat *FsGL2Bitmap::GetRGBAPointer(void) const
{
	return rgba;
}

void FsGL2Bitmap::MakeEmpty(int w,int h)
{
	CleanUp();
	this->rgba=new GLfloat [w*h*4];
	this->w=w;
	this->h=h;
}

void FsGL2Bitmap::MakeFromYsBitmap(const class YsBitmap &bmp)
{
	MakeEmpty(bmp.GetWidth(),bmp.GetHeight());
	YsGLCopyUCharBitmapToGLFloatBitmap(bmp.GetWidth(),bmp.GetHeight(),rgba,bmp.GetRGBABitmapPointer());
}

void FsGL2Bitmap::CopyFrom(const FsGL2Bitmap &from)
{
	if(from.w!=w || from.h!=h)
	{
		MakeEmpty(from.w,from.h);
	}
	for(int i=0; i<w*h*4; ++i)
	{
		rgba[i]=from.rgba[i];
	}
}

void FsGL2Bitmap::ApplySphereLighting(struct YsGLSL3DRenderer *rendererWithLight)
{
	YsGLSL3DRendererApplySphereMap(w,h,rgba,rendererWithLight,0);
}



