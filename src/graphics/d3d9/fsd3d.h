#ifndef FSD3D_IS_INCLUDED
#define FSD3D_IS_INCLUDED
/* { */


class FsD3dDevice : public YsD3dDevice
{
public:
	FsD3dDevice();
	~FsD3dDevice();
	virtual void PrepareBuffer(void);
	virtual void ReleaseBuffer(void);

	// Buffers must be released in FsD3dDevice::ReleaseBuffer(void)@fswin32d3d.cpp
	//         must be NULLed in FsD3dDevice::FsD3dDevice()@fswin32d3d.cpp
	YsXyzColBuffer *gunBuffer,*instNeedleBuffer;
	YsXyzNomColBuffer *debrisBuffer;

	int gndSkyMode;  // 0:By Fog   1:Gradation  2:Crappy
	YsColor skyCol,gndCol;
	YsXyzColBuffer *gndPlgByFog,*skyPlgByFog,*gndSkyGradation;

	YsXyzColBuffer *line2d;  // <-Will be flushed at SwapBuffers

	YsMultiXyzNomColBuffer smkFront[4],smkBack[4];

	YsD3dExternalTextureBufferLink bitmapFont[256];

	HWND hWndSub[2];
	IDirect3DSwapChain9 *subWnd[2];
};


/* } */
#endif
