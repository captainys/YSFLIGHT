#ifndef FSOPENGL_2_0_LOCAL_IS_INCLUDED
#define FSOPENGL_2_0_LOCAL_IS_INCLUDED
/* { */

#include <ysgl.h>
#include <ysglsldrawfontbitmap.h>

#include "fsgl2bitmap.h"

extern int fsNumExplosionTex;
extern GLuint fsExplosionTex[];

extern int fsNumCloudParticleTex;
extern GLuint fsCloudParticleTex[];
extern FsGL2Bitmap cloudParticleTexSrc[];
extern FsGL2Bitmap cloudParticleTexBuf[];

extern GLuint fsParticleTexture;

extern int fsNumFlashTex;
extern GLuint fsFlashTex[];

extern int fsNumCloudTex;
extern GLuint fsCloudTex[];



// In fswirefontgl2.0.cpp >>
// extern void FsGlMakeWireFontList(void);
// In fswirefontgl2.0.cpp <<

/* } */
#endif
