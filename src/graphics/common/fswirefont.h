#ifndef FSWIREFONT_IS_INCLUDED
#define FSWIREFONT_IS_INCLUDED
/* { */

#include <ysglcpp.h>
#include <ysclass.h>



/*! Make vertex arrays for lines and triangles for drawing the given string.
    It first clears lineBuf and triBuf, and call FsAddWireFontVertexBuffer.
*/
void FsMakeWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei);

/*! Add vertex arrays for lines and triangles for drawing the given string.
    It does not clear lineBuf and triBuf.  It adds vertices to the buffers.
*/
void FsAddWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei);



/*! Make vertex arrays for lines and triangles for drawing the given string with given transformation.
    It first clears lineBuf and triBuf, and call FsAddWireFontVertexBuffer.
*/
void FsMakeWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    const YsMatrix4x4 &tfm,const char str[],YsColor col);

/*! Add vertex arrays for lines and triangles for drawing the given string.
    It does not clear lineBuf and triBuf.  It adds vertices to the buffers.
*/
void FsAddWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    const YsMatrix4x4 &tfm,const char str[],YsColor col);



/*! Make vertex arrays for lines and triangles for drawing the given string.
    It first clears lineBuf and triBuf, and call FsAddWireFontVertexBuffer.
    It trims the lines with trimPlg.
*/
void FsMakeWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei,
    YSSIDE sideToDraw,YsConstArrayMask <YsVec2> trimPlg,YsRect2 trimPlgBbx);

/*! Add vertex arrays for lines and triangles for drawing the given string.
    It does not clear lineBuf and triBuf.  It adds vertices to the buffers.
    It trims the lines with trimPlg.
*/
void FsAddWireFontVertexBuffer(
    YsGLVertexBuffer &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei,
    YSSIDE sideToDraw,YsConstArrayMask <YsVec2> trimPlg,YsRect2 trimPlgBbx);






/*! Make vertex arrays for lines and triangles for drawing the given string.
    It first clears lineBuf and triBuf, and call FsAddWireFontVertexBuffer.
*/
void FsMakeWireFontVertexBuffer2D(
    YsGLVertexBuffer2D &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer2D &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei);

/*! Add vertex arrays for lines and triangles for drawing the given string.
    It does not clear lineBuf and triBuf.  It adds vertices to the buffers.
*/
void FsAddWireFontVertexBuffer2D(
    YsGLVertexBuffer2D &lineBuf,YsGLColorBuffer &lineColBuf,
    YsGLVertexBuffer2D &triBuf,YsGLColorBuffer &triColBuf,
    float x,float y,float z,const char str[],YsColor col,float fontWid,float fontHei);



/* } */
#endif
