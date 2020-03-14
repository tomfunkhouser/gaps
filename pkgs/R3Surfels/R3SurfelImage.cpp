/* Source file for the R3 surfel image class */



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

R3SurfelImage::
R3SurfelImage(const char *name)
  : scene(NULL),
    scene_index(-1),
    scan(NULL),
    pose(R3Point(0,0,0), R3Triad(R3Vector(0,0,0), R3Vector(0,0,0))),
    timestamp(0),
    image_width(0), 
    image_height(0),
    image_center(0,0),
    focal_length(0),
    name((name) ? RNStrdup(name) : NULL),
    flags(0),
    data(NULL)
{
}



R3SurfelImage::
R3SurfelImage(const R3SurfelImage& image)
  : scene(NULL),
    scene_index(-1),
    scan(NULL),
    pose(image.pose),
    timestamp(image.timestamp),
    image_width(image.image_width), 
    image_height(image.image_height),
    image_center(image.image_center),
    focal_length(image.focal_length),
    name((image.name) ? RNStrdup(image.name) : NULL),
    flags(0),
    data(NULL)
{
}



R3SurfelImage::
~R3SurfelImage(void)
{
  // Remove image from scan
  if (scan) SetScan(NULL);
  
  // Delete image from scene
  if (scene) scene->RemoveImage(this);

  // Delete name
  if (name) free(name);
}



////////////////////////////////////////////////////////////////////////
// IMAGE GENERATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

R2Point R3SurfelImage::
ImagePosition(const R3Point& world_position) const
{
  // Transform 3D point into canonical coordinate system
  R3Point p = Pose().InverseMatrix() * world_position;
  if (RNIsPositiveOrZero(p.Z())) return R2infinite_point;

  // Compute 2D point projected onto viewport
  const R2Point c = ImageCenter();
  RNCoord x = c.X() + focal_length * p.X() / -p.Z();
  RNCoord y = c.Y() + focal_length * p.Y() / -p.Z();

  // Return point projected onto viewport
  return R2Point(x, y);
}



int R3SurfelImage::
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
  if (!scene || !scene->Tree()) return 0;

  // Visit leaf nodes
  RNArray<const R3SurfelNode *> stack;
  stack.Insert(scene->Tree()->RootNode());
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



////////////////////////////////////////////////////////////////////////
// PROPERTY MANIPULATION FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelImage::
SetScan(R3SurfelScan *scan) 
{
  // Check if the same
  if (this->scan == scan) return;

  // Update old scan
  if (this->scan) {
    assert(this->scan->image == this);
    this->scan->image = NULL;
  }

  // Update new scan
  if (scan) {
    assert(scan->image == NULL);
    scan->image = this;
  }

  // Assign scan
  this->scan = scan;

  // Mark scene as dirty
  if (Scene()) Scene()->SetDirty();
}



void R3SurfelImage::
SetPose(const R3CoordSystem& pose) 
{
  // Set pose
  this->pose = pose;
}



void R3SurfelImage::
SetViewpoint(const R3Point& viewpoint) 
{
  // Set viewpoint
  this->pose.SetOrigin(viewpoint);
}



void R3SurfelImage::
SetOrientation(const R3Vector& towards, const R3Vector& up) 
{
  // Set orientation
  this->pose.SetAxes(R3Triad(towards, up));
}



void R3SurfelImage::
SetFocalLength(RNLength f) 
{
  // Set timestamp
  this->focal_length = f;
}



void R3SurfelImage::
SetTimestamp(RNScalar timestamp) 
{
  // Set timestamp
  this->timestamp = timestamp;
}



void R3SurfelImage::
SetName(const char *name)
{
  // Delete previous name
  if (this->name) free(this->name);
  this->name = (name) ? RNStrdup(name) : NULL;

  // Mark scene as dirty
  if (Scene()) Scene()->SetDirty();
}



void R3SurfelImage::
SetImageDimensions(int width, int height) 
{
  // Set resolution
  this->image_width = width;
  this->image_height = height;
}



void R3SurfelImage::
SetImageCenter(const R2Point& center) 
{
  // Set image center
  this->image_center = center;
}



void R3SurfelImage::
SetFlags(RNFlags flags)
{
  // Set flags
  this->flags = flags;
}



void R3SurfelImage::
SetData(void *data) 
{
  // Set user data
  this->data = data;
}



////////////////////////////////////////////////////////////////////////
// DISPLAY FUNCTIONS
////////////////////////////////////////////////////////////////////////

void R3SurfelImage::
Print(FILE *fp, const char *prefix, const char *suffix) const
{
  // Check fp
  if (!fp) fp = stdout;

  // Print image
  if (prefix) fprintf(fp, "%s", prefix);
  fprintf(fp, "%d %s", SceneIndex(), (Name()) ? Name() : "-");
  if (suffix) fprintf(fp, "%s", suffix);
  fprintf(fp, "\n");
}



void R3SurfelImage::
Draw(RNFlags flags) const
{
  // Draw camera
  RNScalar scale = 1.0;
  R3Point org = Viewpoint() + Towards() * scale;
  R3Vector dx = Right() * scale * tan(XFOV());
  R3Vector dy = Up() * scale * tan(YFOV());
  R3Point ur = org + dx + dy;
  R3Point lr = org + dx - dy;
  R3Point ul = org - dx + dy;
  R3Point ll = org - dx - dy;
  R3BeginLine();
  R3LoadPoint(ur);
  R3LoadPoint(ul);
  R3LoadPoint(ll);
  R3LoadPoint(lr);
  R3LoadPoint(ur);
  R3LoadPoint(Viewpoint());
  R3LoadPoint(lr);
  R3LoadPoint(ll);
  R3LoadPoint(Viewpoint());
  R3LoadPoint(ul);
  R3EndLine();
}



} // namespace gaps
