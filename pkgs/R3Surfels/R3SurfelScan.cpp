/* Source file for the R3 surfel scan class */



////////////////////////////////////////////////////////////////////////
// INCLUDE FILES
////////////////////////////////////////////////////////////////////////

#include "R3Surfels.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS/DESTRUCTORS
////////////////////////////////////////////////////////////////////////

R3SurfelScan::
R3SurfelScan(const char *name)
  : scene(NULL),
    scene_index(-1),
    node(NULL),
    image(NULL),
    pose(R3Point(0,0,0), R3Triad(R3Vector(0,0,0), R3Vector(0,0,0))),
    timestamp(0),
    image_width(0), 
    image_height(0),
    image_center(0,0),
    xfocal(0),
    yfocal(0),
    name((name) ? RNStrdup(name) : NULL),
    flags(0),
    data(NULL)
{
}



R3SurfelScan::
R3SurfelScan(const R3SurfelScan& scan)
  : scene(NULL),
    scene_index(-1),
    node(NULL),
    image(NULL),
    pose(scan.pose),
    timestamp(scan.timestamp),
    image_width(scan.image_width), 
    image_height(scan.image_height),
    image_center(scan.image_center),
    xfocal(scan.xfocal),
    yfocal(scan.yfocal),
    name((scan.name) ? RNStrdup(scan.name) : NULL),
    flags(0),
    data(NULL)
{
}



R3SurfelScan::
~R3SurfelScan(void)
{
  // Remove node
  if (node) SetNode(NULL);

  // Remove image
  if (image) SetImage(NULL);

  // Delete scan from scene
  if (scene) scene->RemoveScan(this);

  // Delete name
  if (name) free(name);
}



////////////////////////////////////////////////////////////////////////
// POINT ACCESS FUNCTIONS
////////////////////////////////////////////////////////////////////////

R3SurfelPointSet *R3SurfelScan::
PointSet(RNBoolean leaf_level) const
{
  // Return node pointset
  R3SurfelNode *node = Node();
  if (!node) return NULL;
  return node->PointSet(leaf_level);
}



////////////////////////////////////////////////////////////////////////
// PROPERTY MANIPULATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelScan::
SetPose(const R3CoordSystem& pose) 
{
  // Set pose
  this->pose = pose;
}



void R3SurfelScan::
SetViewpoint(const R3Point& viewpoint) 
{
  // Set viewpoint
  this->pose.SetOrigin(viewpoint);
}



void R3SurfelScan::
SetOrientation(const R3Vector& towards, const R3Vector& up) 
{
  // Set orientation
  this->pose.SetAxes(R3Triad(towards, up));
}



void R3SurfelScan::
SetFocalLengths(RNLength focal_length) 
{
  // Set focal lengths
  this->xfocal = focal_length;
  this->yfocal = focal_length;
}



void R3SurfelScan::
SetXFocal(RNLength focal_length) 
{
  // Set horizontal focal length
  this->xfocal = focal_length;
}



void R3SurfelScan::
SetYFocal(RNLength focal_length) 
{
  // Set horizontal focal length
  this->yfocal = focal_length;
}



void R3SurfelScan::
SetTimestamp(RNScalar timestamp) 
{
  // Set timestamp
  this->timestamp = timestamp;
}



void R3SurfelScan::
SetName(const char *name)
{
  // Delete previous name
  if (this->name) free(this->name);
  this->name = (name) ? RNStrdup(name) : NULL;

  // Mark scene as dirty
  if (Scene()) Scene()->SetDirty();
}



void R3SurfelScan::
SetImageDimensions(int width, int height) 
{
  // Set resolution
  this->image_width = width;
  this->image_height = height;
}



void R3SurfelScan::
SetImageCenter(const R2Point& center) 
{
  // Set image center
  this->image_center = center;
}



void R3SurfelScan::
SetFlags(RNFlags flags)
{
  // Set flags
  this->flags = flags;
}



void R3SurfelScan::
SetData(void *data) 
{
  // Set user data
  this->data = data;
}



////////////////////////////////////////////////////////////////////////
// BLOCK MANIPULATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelScan::
SetNode(R3SurfelNode *node)
{
  // Check if the same
  if (this->node == node) return;

  // Update old node
  if (this->node) {
    assert(this->node->scan == this);
    this->node->scan = NULL;
  }

  // Update new node
  if (node) {
    assert(node->scan == NULL);
    node->scan = this;
  }

  // Assign node
  this->node = node;

  // Mark scene as dirty
  if (Scene()) Scene()->SetDirty();
}



void R3SurfelScan::
SetImage(R3SurfelImage *image)
{
  // Check if the same
  if (this->image == image) return;

  // Update old image
  if (this->image) {
    assert(this->image->scan == this);
    this->image->scan = NULL;
  }

  // Update new image
  if (image) {
    assert(image->scan == NULL);
    image->scan = this;
  }

  // Assign image
  this->image = image;

  // Mark scene as dirty
  if (Scene()) Scene()->SetDirty();
}



////////////////////////////////////////////////////////////////////////
// DISPLAY FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelScan::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print scan
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "%d %s %d %d",
          SceneIndex(), (Name()) ? Name() : "-",
          (Node()) ? node->TreeIndex() : -1,
          (Image()) ? image->SceneIndex() : -1);
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");
}




void R3SurfelScan::
Draw(RNFlags flags) const
{
  // Draw node
  if (node) node->Draw(flags);
}



////////////////////////////////////////////////////////////////////////
// IMAGE GENERATION FUNCTIONS
// USE R3SurfelImage INSTEAD -- MAINLY FOR BACKWARD COMPATIBILITY
////////////////////////////////////////////////////////////////////////

R2Point R3SurfelScan::
ImagePosition(const R3Point& world_position) const
{
  // Transform 3D point into canonical coordinate system
  R3Point p = Pose().InverseMatrix() * world_position;
  if (RNIsPositiveOrZero(p.Z())) return R2infinite_point;

  // Compute 2D point projected onto viewport
  const R2Point c = ImageCenter();
  RNCoord x = c.X() + xfocal * p.X() / -p.Z();
  RNCoord y = c.Y() + yfocal * p.Y() / -p.Z();

  // Return point projected onto viewport
  return R2Point(x, y);
}



int R3SurfelScan::
RenderImage(R2Image *color_image, R2Grid *depth_image,
  R2Grid *xnormal_image, R2Grid *ynormal_image, R2Grid *znormal_image,
  R2Grid *label_image, R2Grid *object_image,
  R2Grid *node_image, R2Grid *block_image) const
{
  // Initialize images
  R2Grid tmp_image(ImageWidth(), ImageHeight());
  tmp_image.Clear(R2_GRID_UNKNOWN_VALUE);
  if (color_image) *color_image = R2Image(ImageWidth(), ImageHeight());
  if (depth_image) *depth_image = tmp_image;
  if (xnormal_image) *xnormal_image = tmp_image;
  if (ynormal_image) *ynormal_image = tmp_image;
  if (znormal_image) *znormal_image = tmp_image;
  if (label_image) *label_image = tmp_image;
  if (object_image) *object_image = tmp_image;
  if (node_image) *node_image = tmp_image;
  if (block_image) *block_image = tmp_image;

  // Check stuff
  if (!scene || !node) return 0;

  // Visit leaf nodes
  RNArray<const R3SurfelNode *> stack;
  stack.Insert(node);
  while (!stack.IsEmpty()) {
    const R3SurfelNode *decendent = stack.Tail();
    stack.RemoveTail();
    if (decendent->NParts() > 0) {
      // Visit decendent nodes
      for (int i = 0; i < decendent->NParts(); i++) {
        R3SurfelNode *part = decendent->Part(i);
        stack.Insert(part);
      }
    }
    else {
      // Render all blocks of leaf node
      for (int i = 0; i < decendent->NBlocks(); i++) {
        R3SurfelBlock *block = decendent->Block(i);
        R3SurfelNode *node = (block) ? block->Node() : NULL;
        R3SurfelObject *object = (node) ? node->Object(TRUE) : NULL;
        R3SurfelLabel *label = (object) ? object->CurrentLabel() : NULL;

        // Read block
        scene->Tree()->Database()->ReadBlock(block);

        // Render block
        for (int j = 0; j < block->NSurfels(); j++) {
          const R3Surfel *surfel = block->Surfel(j);

          // Get image position
          R2Point image_position = ImagePosition(block->SurfelPosition(j));
          int ix = (int) (image_position.X() + 0.5);
          int iy = (int) (image_position.Y() + 0.5);

          // Fill pixel at image index
          if (color_image) color_image->SetPixelRGB(ix, iy, block->SurfelColor(j));
          if (depth_image) depth_image->SetGridValue(ix, iy, (block->SurfelPosition(j) - Viewpoint()).Dot(Towards()));
          if (xnormal_image) xnormal_image->SetGridValue(ix, iy, surfel->NX());
          if (ynormal_image) ynormal_image->SetGridValue(ix, iy, surfel->NY());
          if (znormal_image) znormal_image->SetGridValue(ix, iy, surfel->NZ());
          if (block_image) block_image->SetGridValue(ix, iy, block->DatabaseIndex());
          if (node_image && node) node_image->SetGridValue(ix, iy, node->TreeIndex());
          if (object_image && object) object_image->SetGridValue(ix, iy, object->SceneIndex());
          if (label_image && label) label_image->SetGridValue(ix, iy, label->SceneIndex());
        }

        // Release block
        scene->Tree()->Database()->ReleaseBlock(block);
      }
    }
  }

  // Return success
  return 1;
}

  

} // namespace gaps
