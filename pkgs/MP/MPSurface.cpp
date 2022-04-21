////////////////////////////////////////////////////////////////////////
// Source file for MP surface
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "MP.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {




////////////////////////////////////////////////////////////////////////
// Member functions
////////////////////////////////////////////////////////////////////////

MPSurface::
MPSurface(void)
  : house(NULL),
    house_index(-1),
    region(NULL),
    region_index(-1),
    vertices(),
    position(0,0,0),
    normal(0,0,0),
    bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX),
    label(NULL)
{
}



MPSurface::
~MPSurface(void)
{
  // Delete all vertices
  while (vertices.NEntries() > 0) delete vertices.Tail();

  // Remove from region and house
  if (region) region->RemoveSurface(this);
  if (house) house->RemoveSurface(this);

  // Delete label
  if (label) free(label);
}



void MPSurface::
InsertVertex(MPVertex *vertex, RNBoolean search_for_best_index)
{
  // Find location for vertex
  int best_index = vertices.NEntries();
  if (search_for_best_index) {
    RNLength best_d = FLT_MAX;
    for (int i = 0; i < vertices.NEntries(); i++) {
      MPVertex *v0 = vertices.Kth(i);
      MPVertex *v1 = vertices.Kth((i+1)%vertices.NEntries());
      R3Span span(v0->position, v1->position);
      RNLength d = R3Distance(vertex->position, span);
      if (d < best_d) { best_d = d; best_index = (i+1)%vertices.NEntries(); }
    }
  }

  // Insert vertex
  vertex->surface = this;
  vertex->surface_index = best_index;
  vertices.InsertKth(vertex, best_index);

  // Recompute bounding box
  RecomputeBBox();
}



void MPSurface::
RemoveVertex(MPVertex *vertex)
{
  // Remove vertex
  vertices.Remove(vertex);
  vertex->surface = NULL;
  vertex->surface_index = -1;

  // Recompute bounding box
  RecomputeBBox();
}



void MPSurface::
FlipOrientation(void)
{
  // Reverse order of vertices
  vertices.Reverse();
  for (int i = 0; i < vertices.NEntries(); i++) {
    MPVertex *vertex = vertices.Kth(i);
    vertex->surface_index = i;
  }

  // Reverse direction of normal (normals should always be up)
  // normal.Flip();
}



void MPSurface::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_REGION) && region)
    LoadColor(region->label);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_SURFACE))
    LoadColor(house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_REGION) && region)
    LoadColor(region->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL) && region && region->level)
    LoadColor(region->level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_SURFACE_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_SURFACES] && draw_flags[MP_DRAW_FACES | MP_DRAW_EDGES | MP_DRAW_VERTICES]) DrawPolygon(draw_flags);
}



void MPSurface::
DrawPolygon(RNFlags draw_flags) const
{
  // Draw faces
  if (draw_flags & MP_DRAW_FACES) {
    if (draw_flags & MP_COLOR_FOR_PICK) glDisable(GL_LIGHTING);
    else glEnable(GL_LIGHTING);
    R3LoadNormal(normal);
    GLUtesselator *tess = gluNewTess();
    gluTessCallback(tess, GLU_TESS_BEGIN, (void (*)()) glBegin);
    gluTessCallback(tess, GLU_TESS_VERTEX, (void (*)()) glVertex3dv);
    gluTessCallback(tess, GLU_TESS_END, (void (*)()) glEnd);
    gluTessBeginPolygon(tess, NULL);
    gluTessBeginContour(tess);
    for (int i = 0; i < vertices.NEntries(); i++) {
      GLdouble *coords = (GLdouble *) vertices[i]->position.Coords();
      gluTessVertex(tess, coords, coords);
    }
    gluTessEndContour(tess);
    gluTessEndPolygon(tess);
    gluDeleteTess(tess);
  }

  // Draw edges
  if (draw_flags & MP_DRAW_EDGES) {
    glDisable(GL_LIGHTING);
    RNGrfxBegin(RN_GRFX_LINE_LOOP);
    for (int i = 0; i < vertices.NEntries(); i++) {
      MPVertex *vertex = vertices.Kth(i);
      R3LoadPoint(vertex->position);
    }
    RNGrfxEnd();
  }

  // Draw vertices
  if (draw_flags & MP_DRAW_VERTICES) {
    for (int i = 0; i < vertices.NEntries(); i++) {
      MPVertex *vertex = vertices.Kth(i);
      vertex->DrawPosition(draw_flags);
    }
  }
}



void MPSurface::
RecomputeBBox(void)
{
  // Update bounding box
  bbox = R3null_box;
  for (int i = 0; i < vertices.NEntries(); i++) {
    MPVertex *vertex = vertices.Kth(i);
    vertex->surface_index = i;
    bbox.Union(vertex->position);
  }

  // Update region
  if (region) region->RecomputeBBox(TRUE);
}

  

// End namespace
};
