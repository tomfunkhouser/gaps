////////////////////////////////////////////////////////////////////////
// Source file for MP segment
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

MPSegment::
MPSegment(void)
  : house(NULL),
    house_index(-1),
    object(NULL),
    object_index(-1),
    mesh(NULL),
    faces(),
    area(0),
    position(0,0,0),
    bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX),
    id(-1)
{
}



MPSegment::
~MPSegment(void)
{
  // Remove from object and house
  if (object) object->RemoveSegment(this);
  if (house) house->RemoveSegment(this);
}



void MPSegment::
Draw(RNFlags draw_flags) const
{
  // Set the color
  if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_OBJECT) && object && object->category)
    LoadColor(object->category->mpcat40_id);
  else if ((draw_flags & MP_COLOR_BY_LABEL) && (draw_flags & MP_COLOR_BY_REGION) && object && object->region && object->region->label)
    LoadColor(object->region->label);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_SEGMENT))
    LoadColor(house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_OBJECT) && object)
    LoadColor(object->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_REGION) && object->region)
    LoadColor(object->region->house_index + 1);
  else if ((draw_flags & MP_COLOR_BY_INDEX) && (draw_flags & MP_COLOR_BY_LEVEL) && object && object->region && object->region->level)
    LoadColor(object->region->level->house_index + 1);
  else if (draw_flags & MP_COLOR_FOR_PICK)
    LoadIndex(house_index, MP_SEGMENT_TAG);

  // Draw this
  if (draw_flags[MP_SHOW_SEGMENTS] && draw_flags[MP_DRAW_FACES | MP_DRAW_EDGES | MP_DRAW_VERTICES]) DrawMesh(draw_flags);
  if (draw_flags[MP_SHOW_SEGMENTS] && draw_flags[MP_DRAW_BBOXES]) DrawBBox(draw_flags);
}



void MPSegment::
DrawMesh(RNFlags draw_flags) const
{
  // Check mesh
  if (!mesh) return;

  // Draw faces
  if (draw_flags & MP_DRAW_FACES) {
    if (draw_flags & MP_COLOR_FOR_PICK) glDisable(GL_LIGHTING);
    else glEnable(GL_LIGHTING);
    RNGrfxBegin(RN_GRFX_TRIANGLES);
    for (int i = 0; i < faces.NEntries(); i++) {
      R3MeshFace *face = faces.Kth(i);
      R3LoadNormal(mesh->FaceNormal(face));
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh->VertexOnFace(face, j);
        if (draw_flags[MP_COLOR_BY_RGB]) R3LoadRgb(mesh->VertexColor(vertex));
        R3LoadPoint(mesh->VertexPosition(vertex));
      }
    }
    RNGrfxEnd();
  }

  // Draw edges
  if (draw_flags & MP_DRAW_EDGES) {
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    RNGrfxBegin(RN_GRFX_TRIANGLES);
    for (int i = 0; i < faces.NEntries(); i++) {
      R3MeshFace *face = faces.Kth(i);
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh->VertexOnFace(face, j);
        if (draw_flags[MP_COLOR_BY_RGB]) R3LoadRgb(mesh->VertexColor(vertex));
        R3LoadPoint(mesh->VertexPosition(vertex));
      }
    }
    RNGrfxEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  // Draw vertices
  if (draw_flags & MP_DRAW_VERTICES) {
    R3mesh_mark++;
    glDisable(GL_LIGHTING);
    RNGrfxBegin(RN_GRFX_POINTS);
    for (int i = 0; i < faces.NEntries(); i++) {
      R3MeshFace *face = faces.Kth(i);
      for (int j = 0; j < 3; j++) {
        R3MeshVertex *vertex = mesh->VertexOnFace(face, j);
        if (mesh->VertexMark(vertex) == R3mesh_mark) continue;
        mesh->SetVertexMark(vertex, R3mesh_mark);
        if (draw_flags[MP_COLOR_BY_RGB]) R3LoadRgb(mesh->VertexColor(vertex));
        R3LoadPoint(mesh->VertexPosition(vertex));
      }
    }
    RNGrfxEnd();
  }
}



void MPSegment::
DrawBBox(RNFlags draw_flags) const
{
  // Draw bounding box
  glDisable(GL_LIGHTING);
  bbox.Outline();
}

  

// End namespace
};
