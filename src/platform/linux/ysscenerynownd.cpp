#include <ysclass.h>

#include <ysopengl.h>

#include <ysscenery.h>



void Ys2DDrawing::Draw(
    const double &plgColScale,const double &linColorScale,const double &pntColorScale,
    YSBOOL drawPset,
    YSBOOL mapMode,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL name2DElem,
    const double &currentTime,
    YsMatrix4x4 *viewModelTfm)
{
}

void Ys2DDrawing::DrawBoundingBox(YSBOOL mapMode)
{
}

void YsElevationGrid::Draw(
    const double &plgColorScale,
    YSBOOL invert,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL shrinkTriangle,
    YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
}

void YsElevationGrid::DrawFastFillOnly(const double &plgColorScale)
{
}

void YsElevationGrid::DrawBoundingBox(void)
{
}

void YsSceneryShell::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
}

void YsSceneryRectRegion::Draw(void)
{
}

void YsSceneryGndObj::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
}

void YsSceneryAir::Draw(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
}

void YsSceneryPointSet::Draw(void)
{
}

void YsSceneryPointSet::DrawStar(void)
{
}

void YsScenery::GlSetColor(const YsColor &col)
{
}

void YsScenery::DrawBoundingBox(void)
{
}

void YsScenery::Draw(
    YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
}

void YsScenery::DrawMap(YSBOOL wire,YSBOOL fill,YSBOOL drawBbx)
{
}

void YsScenery::DrawProtectPolygon(const YsMatrix4x4 &modelTfm) // OpenGL Only for SceneryEdit
{
}

void YsScenery::DrawProtectPolygon(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,
    const double &currentTime,
    const double &nearZ,const double &farZ,const double &tanFov)
{
}


int YsScenery::numSceneryDrawn;

void YsScenery::DrawVisual(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,const double &currentTime,
    const double &nearZ,const double &farZ,const double &tanFov)
{
}

void YsScenery::DrawMapVisual(
    const YsMatrix4x4 &viewTfm,const YsMatrix4x4 &modelTfm,
    const double &elvMin,const double &elvMax,YSBOOL drawPset,const double &currentTime,
    const double &nearZ,const double &farZ,const double &tanFov)
{
}


void YsScenery::DrawAxis(
    const double &axsSize,
    YSBOOL drawShl,YSBOOL drawEvg,YSBOOL drawMap,YSBOOL drawSbd,YSBOOL drawRgn,YSBOOL drawGndObj,
    YSBOOL drawAir,YSBOOL drawPst,YSBOOL drawScn)
{
}

void YsScenery::DrawItemAxis(const YsSceneryItem *itm,const double &axsSize)
{
}

void YsScenery::DrawItem(
    const YsSceneryItem *itm,YSBOOL wire,YSBOOL fill,YSBOOL drawBbx,YSBOOL drawShrink,
    YSBOOL name2DElem,YSBOOL nameElvGridFace,YSBOOL nameElvGridNode)
{
}

void YsScenery::DrawItemStar(const YsSceneryItem *itm)
{
}

void YsScenery::Draw2DDrawingElement(
    YsScenery2DDrawing *drw,const Ys2DDrawingElement *itm,
    YSBOOL nameVtId,YSBOOL wire,YSBOOL points)
{
}

void YsScenery::DrawILSGuideline(void)
{
}

void YsScenery::DrawItemILSGuideline(YsSceneryGndObj *gnd)
{
}
