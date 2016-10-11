// Source file for debug visualization

#include "R3Graphics/R3Graphics.h"
#include "debug.h"


//////////////////////////////////////

static R3Affine
NormalizationTransformation(R3Scene *scene, R3SceneNode *node)
{
  // Get bounding boxes
  const R3Box& scene_bbox = scene->BBox();
  const R3Box& node_bbox = node->BBox();
  
  // Find the side of the node bounding box closest to the "wall"
  int closest_dim = 0;
  int closest_dir = 0;
  RNLength closest_distance = FLT_MAX;
  for (int dim = RN_X; dim <= RN_Y; dim++) {
    for (int dir = RN_LO; dir <= RN_HI; dir++) {
      RNLength distance = fabs(node_bbox[dir][dim] - scene_bbox[dir][dim]);
      if (distance < closest_distance) {
        closest_distance = distance;
        closest_dir = dir;
        closest_dim = dim;
      }
    }
  }

  // Get transformation parameters
  RNAngle zrotation = 0;
  R3Point origin = node_bbox.Centroid();
  origin[2] = node_bbox.ZMin();
  if ((closest_dir == RN_HI) && (closest_dim == RN_X)) { origin[0] = node_bbox.XMax(); zrotation = -0.5 * RN_PI; }
  else if ((closest_dir == RN_HI) && (closest_dim == RN_Y)) { origin[1] = node_bbox.YMax(); zrotation = RN_PI; }
  else if ((closest_dir == RN_LO) && (closest_dim == RN_X)) { origin[0] = node_bbox.XMin(); zrotation = 0.5 * RN_PI; }
  else if ((closest_dir == RN_LO) && (closest_dim == RN_Y)) { origin[1] = node_bbox.YMin(); zrotation = 0.0; }

  // Compute transformation
  // Puts center of bbox closest to wall at origin
  // Aligns side of object closest to wall with the x axis
  R3Affine result = R3identity_affine;
  if (zrotation != 0.0) result.ZRotate(zrotation);
  result.Translate(-(origin.Vector()));

  // Return result
  return result;
}



void DrawNormalizationTransformation(R3Scene *scene, R3Viewer *viewer, R3SceneNode *selected_node)
{
  if (selected_node) {
    glDisable(GL_LIGHTING);
    R3Affine affine = NormalizationTransformation(scene, selected_node);
    affine.Invert();
    affine.Push();
    RNScalar d = selected_node->BBox().DiagonalRadius();
    glDisable(GL_LIGHTING);
    glLineWidth(3);
    R3BeginLine();
    glColor3f(1, 0, 0);
    R3LoadPoint(R3zero_point + d * R3negx_vector);
    glColor3f(1, 0.5, 0.5);
    R3LoadPoint(R3zero_point + d * R3posx_vector);
    R3EndLine();
    R3BeginLine();
    glColor3f(0, 1, 0);
    R3LoadPoint(R3zero_point + d * R3negy_vector);
    glColor3f(0.5, 1, 0.5);
    R3LoadPoint(R3zero_point + d * R3posy_vector);
    R3EndLine();
    R3BeginLine();
    glColor3f(0, 0, 1);
    R3LoadPoint(R3zero_point + d * R3negz_vector);
    glColor3f(0.5, 0.5, 1);
    R3LoadPoint(R3zero_point + d * R3posz_vector);
    R3EndLine();
    glLineWidth(1);
    affine.Pop();
  }
}



void DrawDebug(R3Scene *scene, R3Viewer *viewer, R3SceneNode *selected_node)
{
  DrawNormalizationTransformation(scene, viewer, selected_node);
}
