#include <ysclass.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <fs.h>
#include <fswindow.h>
#include <fsgui.h>

#include <fsopengl.h>
#include <fsfontrenderer.h>

#include <ysglfontdata.h>

#include <windows.h>
#include <mmsystem.h>


#include <GL/gl.h>
#include <GL/glu.h>

#include <windowsx.h>





static void FsWin32SetPixelFormat(HDC dc);
static unsigned char FsWin32PalVal(unsigned long n,unsigned bit,unsigned sft);
static HPALETTE FsWin32CreatePalette(HDC dc);
static void FsWin32InitOpenGL(HWND wnd);


// This file includes functions called from fswin32.cpp

bool FsWin32CreateGraphicContext(HWND hWnd,HDC hDc)
{
	return false;  // This will let FsSimpleWindow framework create OpenGL context.
}

void FsWin32InitializeGraphicEngine(HWND hWnd,HDC)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	FsSwapBuffers();

	FsWin32InitOpenGL(hWnd);
	fsDirectFixedFontRenderer.RemakeFont();
}

void FsWin32UninitializeGraphicEngine(HWND,HDC)
{
	// wglMakeCurrent(NULL,NULL);
}

void FsWin32AfterResize(HWND hWnd,HDC hDc,int winX,int winY)
{
    RECT rect;
	float aspect;

    GetClientRect(hWnd,&rect);

    glMatrixMode(GL_PROJECTION);
    aspect=(float)rect.right/rect.bottom;;
	glLoadIdentity();
    gluPerspective(45.0F,aspect,0.5,17.0F);

	glMatrixMode(GL_MODELVIEW);
    glViewport(0,0,rect.right,rect.bottom);
}

bool FsWin32SwapBuffers(HWND,HDC)
{
	return false;  // FsSimpleWindow framework will take care of it.
}



////////////////////////////////////////////////////////////

void FsWin32SetPixelFormat(HDC dc)
{
	static PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0,0,0,0,0,0,
		0,
		0,
		0,
		0,0,0,0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,0,0
	};
	int pfm;
	pfm=ChoosePixelFormat(dc,&pfd);
	if(pfm!=0)
	{
		if(SetPixelFormat(dc,pfm,&pfd)!=FALSE)
		{
			HPALETTE hPlt;
			hPlt=FsWin32CreatePalette(dc);
			SelectPalette(dc,hPlt,FALSE);
			RealizePalette(dc);
			return;
		}
	}
	fprintf(stderr,"YSWIN32 : Error In SetPixelFormat.");
}

unsigned char FsWin32PalVal(unsigned long n,unsigned bit,unsigned sft)
{
	unsigned long msk;
	n>>=sft;
	msk=(1<<bit)-1;
	n&=msk;
	return (unsigned char)(n*255/msk);
}

/* ? lp=LocalAlloc(LMEM_FIXED,sizeof(LOGPALETTE)+n*sizeof(PALETTEENTRY)); */
/* ? LocalFree(lp); */

HPALETTE FsWin32CreatePalette(HDC dc)
{
	HPALETTE neo;
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *lp;
    int n,i;

    n=GetPixelFormat(dc);
    DescribePixelFormat(dc,n,sizeof(PIXELFORMATDESCRIPTOR),&pfd);

	neo=NULL; // 10/17/2000
    if(pfd.dwFlags & PFD_NEED_PALETTE)
    {
        // LocalAlloc, LocalFree : Difference between BI & this

        n=1<<pfd.cColorBits;
        lp=(LOGPALETTE *)LocalAlloc(LMEM_FIXED,sizeof(LOGPALETTE)+n*sizeof(PALETTEENTRY));
        lp->palVersion=0x300;
        lp->palNumEntries=n;
        for (i=0; i<n; i++)
        {
            lp->palPalEntry[i].peRed  =FsWin32PalVal(i,pfd.cRedBits,pfd.cRedShift);
            lp->palPalEntry[i].peGreen=FsWin32PalVal(i,pfd.cGreenBits,pfd.cGreenShift);
            lp->palPalEntry[i].peBlue =FsWin32PalVal(i,pfd.cBlueBits,pfd.cBlueShift);
            lp->palPalEntry[i].peFlags=0;
        }

        neo=::CreatePalette(lp);
		LocalFree(lp);
    }
    return neo;
}

void FsWin32InitOpenGL(HWND wnd)
{
	RECT rect;
	float aspect;

    GetClientRect(wnd,&rect);

    glClearColor(1.0F,1.0F,1.0F, 1.0F);
    glClearDepth(1.0F);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    aspect=(float)rect.right/rect.bottom;;
    gluPerspective(45.0F,aspect,0.5,17.0F);
	/* glFrustum(-20.0F,20.0F,-20.0F,20.0F,  50,200.0F); <- Super version of Perspective */

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0,0,rect.right,rect.bottom);

	glShadeModel(GL_SMOOTH);



	GLfloat dif[]={0.8F,0.8F,0.8F,1.0F};
	GLfloat amb[]={0.0F,0.0F,0.0F,1.0F};
	GLfloat spc[]={0.9F,0.9F,0.9F,1.0F};
	GLfloat shininess[]={50.0,50.0,50.0,0.0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,dif);
	glLightfv(GL_LIGHT0,GL_SPECULAR,spc);
	glMaterialfv(GL_FRONT|GL_BACK,GL_SHININESS,shininess);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,amb);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);


	int depthBits;
	glGetIntegerv(GL_DEPTH_BITS,&depthBits);
	printf("Bit Depth=%d\n",depthBits);
}

void FsWin32HidePartOfScreenForSharewareMessage(void)
{
	int viewport[4],wid,hei;
	glGetIntegerv(GL_VIEWPORT,viewport);
	wid=viewport[2];
	hei=viewport[3];

	glDepthFunc(GL_ALWAYS);
	glDepthMask(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-0.5,(GLdouble)wid-0.5,(GLdouble)hei-0.5,-0.5,-1,1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glDisable(GL_COLOR_MATERIAL);

	glColor3d(0.0,0.0,0.0);

	glBegin(GL_POLYGON);
	glVertex2i(0    ,0);
	glVertex2i(wid-1,0);
	glVertex2i(wid-1,hei/3);
	glVertex2i(0    ,hei/3);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2i(0    ,hei*2/3);
	glVertex2i(wid-1,hei*2/3);
	glVertex2i(wid-1,hei-1);
	glVertex2i(0    ,hei-1);
	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
