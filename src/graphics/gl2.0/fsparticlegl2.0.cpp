#include <ysclass.h>

#include <ysbitmap.h>
#include <fssimplewindow.h>

#include <ysgl.h>
#include <ysglslcpp.h>
#include <ysglparticlemanager.h>

#include "fsparticle.h"
#include "fstexturemanager.h"

#include "fsgl2.0util.h"
#include "fsopengl2.0.h"

static FsGL2VariableVertexStorage vtxBuf;

/*
	If, additive transparency is used (glBlendFunc(GL_SRC_ALPHA,GL_ONE);) the order doesn't matter.  Particles can be drawn by:
	  (1) first drawing with DepthTest on, DepthMask GL_FALSE, Color mask all GL_TRUE.
	  (2) then drawing the same particles with DepthTest on, DepthMask GL_TRUE, Color mask all GL_FALSE.

    This way, I can avoid sorting particles in Z direction.  However, it is 'additive'.  Not, converging to the texture color.
    Therefore, it only go brighter with more particles.
    
    It may be ok for day-time cloud and smoke, but not work for night.  Also the color gets too bright even for day-time.
*/



void FsParticleStore::Draw(const class YsGLParticleManager &partMan) const
{
	glDisable(GL_CULL_FACE);

	auto &commonTexture=FsCommonTexture::GetCommonTexture();
	if(nullptr==commonTexture.GetParticleSpriteTextureHd())
	{
		commonTexture.LoadParticleSpriteTexture();
	}

	auto texUnitPtr=commonTexture.GetParticleSpriteTexture();
	if(nullptr!=texUnitPtr)
	{
		texUnitPtr->Bind(0);
	}
	{
		// First draw farther-away particles with point sprites,
		// and then closer particles with triangles.

		int viewport[4];
		glGetIntegerv(GL_VIEWPORT,viewport);

#ifdef GL_PROGRAM_POINT_SIZE
		glEnable(GL_PROGRAM_POINT_SIZE);  // Needed for enabling gl_PointSize in the vertex shader.
#endif
#ifdef GL_POINT_SPRITE
		glEnable(GL_POINT_SPRITE);        // Needed for enabling gl_PointCoord in the fragment shader.
#endif

		YsGLSLPointSprite3DRenderer renderer;
		renderer.EnableFog();
		renderer.SetTextureIdent(0);
		renderer.SetViewportDimension((float)viewport[2],(float)viewport[3]);
		renderer.SetPointSizeMode(YSGLSL_POINTSPRITE_SIZE_IN_3DSPACE);
		renderer.SetPointSpriteClippingType(YSGLSL_POINTSPRITE_CLIPPING_NONE);
		renderer.SetTextureType(YSGLSL_TEX_TYPE_BILLBOARD);
		renderer.SetPointSpriteTextureCoordRange(0.125);
		renderer.DrawVtxTexCoordColPointSize(GL_POINTS,partMan.pntVtxBuf.GetN(),partMan.pntVtxBuf,partMan.pntTexCoordBuf,partMan.pntColBuf,partMan.pntSizeBuf);

#ifdef GL_PROGRAM_POINT_SIZE
		glDisable(GL_PROGRAM_POINT_SIZE);  // Needed for enabling gl_PointSize in the vertex shader.
#endif
#ifdef GL_POINT_SPRITE
		glDisable(GL_POINT_SPRITE);        // Needed for enabling gl_PointCoord in the fragment shader.
#endif
	}
	{
		YsGLSLPlain3DRenderer renderer;
		renderer.SetTextureIdent(0);
		renderer.SetTextureType(YSGLSL_TEX_TYPE_ATTRIBUTE);
		renderer.DrawVtxTexCoordCol(GL_TRIANGLES,partMan.triVtxBuf.GetN(),partMan.triVtxBuf,partMan.triTexCoordBuf,partMan.triColBuf);
		renderer.SetTextureType(YSGLSL_TEX_TYPE_NONE);
	}

	glEnable(GL_CULL_FACE);
}
