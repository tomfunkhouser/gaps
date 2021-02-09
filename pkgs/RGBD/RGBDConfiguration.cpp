////////////////////////////////////////////////////////////////////////
// Source file for RGBDConfiguration class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "RGBD.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Constructors/destructors
////////////////////////////////////////////////////////////////////////

RGBDConfiguration::
RGBDConfiguration(void)
  : images(),
    surfaces(),
    name(NULL),
    color_directory(NULL),
    depth_directory(NULL),
    category_directory(NULL),
    instance_directory(NULL),
    texture_directory(NULL),
    dataset_format(NULL),
    world_bbox(FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}



RGBDConfiguration::
~RGBDConfiguration(void)
{
  // Delete everything
  while (NImages() > 0) delete Image(NImages()-1);
  while (NSurfaces() > 0) delete Surface(NSurfaces()-1);

  // Delete name
  if (name) free(name);

  // Delete directory names
  if (color_directory) free(color_directory);
  if (depth_directory) free(depth_directory);
  if (category_directory) free(category_directory);
  if (instance_directory) free(instance_directory);
  if (texture_directory) free(texture_directory);

  // Delete dataset format
  if (dataset_format) free(dataset_format);
}



////////////////////////////////////////////////////////////////////////
// Property functions
////////////////////////////////////////////////////////////////////////

R3Point RGBDConfiguration::
WorldCentroid(void) const
{
  // Compute weighed sum of surface centroids
  RNArea weight = 0;
  R3Point position = R3zero_point;
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    position += surface->WorldArea() * surface->WorldCentroid();
    weight += surface->WorldArea();
  }

  // Return weighted average
  if (RNIsZero(weight)) return R3zero_point;
  return position / weight;
}



////////////////////////////////////////////////////////////////////////
// Image and surface manipulation functions
////////////////////////////////////////////////////////////////////////

void RGBDConfiguration::
InsertImage(RGBDImage *image)
{
  // Just checking
  assert(image);
  assert(image->configuration == NULL);
  assert(image->configuration_index == -1);

  // Insert image into configuration
  image->configuration = this;
  image->configuration_index = images.NEntries();
  images.Insert(image);

  // Update bounding box
  world_bbox.Union(image->WorldBBox());
}



void RGBDConfiguration::
RemoveImage(RGBDImage *image)
{
  // Just checking
  assert(image);
  assert(image->configuration == this);
  assert(image->configuration_index >= 0);
  assert(images.Kth(image->configuration_index) == image);

  // Remove image from configuration
  RNArrayEntry *entry = images.KthEntry(image->configuration_index);
  RGBDImage *tail = images.Tail();
  tail->configuration_index = image->configuration_index;
  images.EntryContents(entry) = tail;
  images.RemoveTail();
  image->configuration_index = -1;
  image->configuration = NULL;
}



void RGBDConfiguration::
InsertSurface(RGBDSurface *surface)
{
  // Just checking
  assert(surface);
  assert(surface->configuration == NULL);
  assert(surface->configuration_index == -1);

  // Insert surface into configuration
  surface->configuration = this;
  surface->configuration_index = surfaces.NEntries();
  surfaces.Insert(surface);

  // Update bounding box
  world_bbox.Union(surface->WorldBBox());
}



void RGBDConfiguration::
RemoveSurface(RGBDSurface *surface)
{
  // Just checking
  assert(surface);
  assert(surface->configuration == this);
  assert(surface->configuration_index >= 0);
  assert(surfaces.Kth(surface->configuration_index) == surface);

  // Remove surface from configuration
  RNArrayEntry *entry = surfaces.KthEntry(surface->configuration_index);
  RGBDSurface *tail = surfaces.Tail();
  tail->configuration_index = surface->configuration_index;
  surfaces.EntryContents(entry) = tail;
  surfaces.RemoveTail();
  surface->configuration_index = -1;
  surface->configuration = NULL;
}



void RGBDConfiguration::
Transform(const R3Transformation& transformation)
{
  // Transform images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->Transform(transformation);
  }
  
  // Transform surfaces
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    surface->Transform(transformation);
  }
}



////////////////////////////////////////////////////////////////////////
// Directory name manipulation functions
////////////////////////////////////////////////////////////////////////

void RGBDConfiguration::
SetName(const char *name)
{
  // Set directory name
  if (this->name) free(this->name);
  if (name && strcmp(name, "-")) this->name = RNStrdup(name);
  else this->name = NULL;
}



void RGBDConfiguration::
SetColorDirectory(const char *directory)
{
  // Set directory name
  if (color_directory) free(color_directory);
  if (directory && strcmp(directory, "-")) color_directory = RNStrdup(directory);
  else color_directory = NULL;
}



void RGBDConfiguration::
SetDepthDirectory(const char *directory)
{
  // Set directory name
  if (depth_directory) free(depth_directory);
  if (directory && strcmp(directory, "-")) depth_directory = RNStrdup(directory);
  else depth_directory = NULL;
}



void RGBDConfiguration::
SetCategoryDirectory(const char *directory)
{
  // Set directory name
  if (category_directory) free(category_directory);
  if (directory && strcmp(directory, "-")) category_directory = RNStrdup(directory);
  else category_directory = NULL;
}



void RGBDConfiguration::
SetInstanceDirectory(const char *directory)
{
  // Set directory name
  if (instance_directory) free(instance_directory);
  if (directory && strcmp(directory, "-")) instance_directory = RNStrdup(directory);
  else instance_directory = NULL;
}



void RGBDConfiguration::
SetTextureDirectory(const char *directory)
{
  // Set directory name
  if (texture_directory) free(texture_directory);
  if (directory && strcmp(directory, "-")) texture_directory = RNStrdup(directory);
  else texture_directory = NULL;
}



void RGBDConfiguration::
SetDatasetFormat(const char *format)
{
  // Set format name
  if (dataset_format) free(dataset_format);
  if (format && strcmp(format, "-")) dataset_format = RNStrdup(format);
  else dataset_format = NULL;
}



////////////////////////////////////////////////////////////////////////
// File input/output functions
////////////////////////////////////////////////////////////////////////

int RGBDConfiguration::
ReadFile(const char *filename, int read_every_kth_image)
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .conf)\n", filename);
    return 0;
  }

  // Read file of appropriate type
  if (!strncmp(extension, ".conf", 5)) {
    if (!ReadConfigurationFile(filename, read_every_kth_image)) return 0;
  }
  else if (!strncmp(extension, ".json", 5)) {
    if (!ReadNeRFFile(filename, read_every_kth_image)) return 0;
  }
  else if (!strncmp(extension, ".seg", 4)) {
    if (!ReadSegmentationFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".obb", 4)) {
    if (!ReadOrientedBoxFile(filename)) return 0;
  }
  else {
    RNFail("Unable to read file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Return success
  return 1;
}


int RGBDConfiguration::
WriteFile(const char *filename, int write_every_kth_image) const
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .conf)\n", filename);
    return 0;
  }

  // Write file of appropriate type
  if (!strncmp(extension, ".conf", 5)) {
    if (!WriteConfigurationFile(filename, write_every_kth_image)) return 0;
  }
  else if (!strncmp(extension, ".json", 5)) {
    if (!WriteNeRFFile(filename, write_every_kth_image)) return 0;
  }
  else if (!strncmp(extension, ".cam", 4)) {
    if (!WriteCameraFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".obj", 4)) {
    if (!WriteObjFile(filename)) return 0;
  }
  else {
    RNFail("Unable to write file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Return success
  return 1;
}


////////////////////////////////////////////////////////////////////////
// Configuration file input/output functions
////////////////////////////////////////////////////////////////////////

int RGBDConfiguration::
ReadConfigurationFile(const char *filename, int read_every_kth_image)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open configuration file %s\n", filename);
    return 0;
  }

  // Read file
  if (!ReadConfigurationStream(fp, read_every_kth_image)) return 0;
  
  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int RGBDConfiguration::
WriteConfigurationFile(const char *filename, int write_every_kth_image) const
{
#if 0
  // Create directories
  char cmd[4096];
  if (color_directory) { sprintf(cmd, "mkdir -p %s", color_directory); system(cmd); }
  if (depth_directory) { sprintf(cmd, "mkdir -p %s", depth_directory); system(cmd); }
  if (category_directory) { sprintf(cmd, "mkdir -p %s", category_directory); system(cmd); }
  if (instance_directory) { sprintf(cmd, "mkdir -p %s", instance_directory); system(cmd); }
  if (texture_directory) { sprintf(cmd, "mkdir -p %s", texture_directory); system(cmd); }
#endif

  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open configuration file %s\n", filename);
    return 0;
  }

  // Write file
  if (!WriteConfigurationStream(fp, write_every_kth_image)) return 0;

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int RGBDConfiguration::
ReadConfigurationStream(FILE *fp, int read_every_kth_image)
{
  // Initialize stuff
  int image_count = 0;
  R3Matrix intrinsics_matrix = R3identity_matrix;
  int image_width = 0;
  int image_height = 0;
  
  // Parse file
  char buffer[4096];
  int line_number = 0;
  while (fgets(buffer, 4096, fp)) {
    char cmd[1024];
    line_number++;
    if (sscanf(buffer, "%s", cmd) != (unsigned int) 1) continue;
    if (cmd[0] == '#') continue;

    // Check cmd
    if (!strcmp(cmd, "dataset")) {
      // Parse dataset format
      char name[1024] = { '\0' };
      if (sscanf(buffer, "%s%s", cmd, name) != (unsigned int) 2) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Set dataset format
      SetDatasetFormat(name);
    }
    else if (!strcmp(cmd, "sequence") || !strcmp(cmd, "name")) {
      // Parse name
      char name[1024] = { '\0' };
      if (sscanf(buffer, "%s%s", cmd, name) != (unsigned int) 2) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Set dataset format
      SetName(name);
    }
    else if (!strcmp(cmd, "depth_resolution")) {
      // Parse dimensions
      if (sscanf(buffer, "%s%d%d", cmd, &image_width, &image_height) != (unsigned int) 3) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }
    }
    else if (!strcmp(cmd, "color_resolution")) {
      // Parse dimensions
      if ((image_width == 0) || (image_height == 0)) {
        if (sscanf(buffer, "%s%d%d", cmd, &image_width, &image_height) != (unsigned int) 3) {
          RNFail("Error parsing line %d of configuration file\n", line_number);
          return 0;
        }
      }
    }
    else if (!strcmp(cmd, "depth_directory") || !strcmp(cmd, "color_directory") ||
             !strcmp(cmd, "category_directory") || !strcmp(cmd, "instance_directory") ||
             !strcmp(cmd, "texture_directory") || !strcmp(cmd, "image_directory")) {
      // Parse directory name
      char dirname[1024];
      if (sscanf(buffer, "%s%s", cmd, dirname) != (unsigned int) 2) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Assign directory name (this leaks memory, but who cares)
      if (!strcmp(cmd, "color_directory")) SetColorDirectory(dirname);
      else if (!strcmp(cmd, "depth_directory")) SetDepthDirectory(dirname);
      else if (!strcmp(cmd, "category_directory")) SetCategoryDirectory(dirname);
      else if (!strcmp(cmd, "instance_directory")) SetInstanceDirectory(dirname);
      else if (!strcmp(cmd, "image_directory")) SetColorDirectory(dirname);
      else if (!strcmp(cmd, "texture_directory")) SetTextureDirectory(dirname);
    }
    else if (!strcmp(cmd, "intrinsics") || !strcmp(cmd, "depth_intrinsics")) { // || !strcmp(cmd, "color_intrinsics")) {
      // Parse intrinsics filename
      char intrinsics_filename[2048];
      if (sscanf(buffer, "%s%s", cmd, intrinsics_filename) != (unsigned int) 2) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Open intrinsics file
      FILE *intrinsics_fp = fopen(intrinsics_filename, "r");
      if (!intrinsics_fp) {
        RNFail("Unable to open intrinsics file %s\n", intrinsics_filename);
        return 0;
      }

      // Read intrinsics file
      if (dataset_format && !strcmp(dataset_format, "matterport")) {
        // Read matterport intrinsics
        double width, height, fx, fy, cx, cy, k1, k2, p1, p2, k3;
        if (fscanf(intrinsics_fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &width, &height,
          &fx, &fy, &cx, &cy, &k1, &k2, &p1, &p2, &k3) != (unsigned int) 11) {
          RNFail("Unable to read Matterport intrinsics matrix.\n");
          return 0;
        }

        // Assign intrinsics matrix
        intrinsics_matrix = R3Matrix(fx, 0, cx,   0, fy, cy,   0, 0, 1);
      }
      else {
        // Read matrix
        RNScalar m[9];
        if (fscanf(intrinsics_fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8]) != (unsigned int) 9) {
          RNFail("Unable to read intrinsics file %s\n", intrinsics_filename);
          return 0;
        }

        // Assign intrinsics matrix
        intrinsics_matrix = R3Matrix(m);
      }

      // Close intrinsics file
      fclose(intrinsics_fp);
    }
    else if (!strcmp(cmd, "intrinsics_matrix") || !strcmp(cmd, "depth_intrinsics_matrix")) { // || !strcmp(cmd, "color_intrinsics_matrix")) {
      // Parse matrix
      double m[9];
      if (sscanf(buffer, "%s%lf%lf%lf%lf%lf%lf%lf%lf%lf", cmd, &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], &m[8]) != (unsigned int) 10) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Assign intrinsics matrix
      intrinsics_matrix = R3Matrix(m);
    }
    else if (!strcmp(cmd, "scan") || !strcmp(cmd, "image")) {
      // Update/check image count
      if ((read_every_kth_image > 1) && ((++image_count % read_every_kth_image) != 1)) continue;

      // Check intrinsics matrix
      if (intrinsics_matrix.IsIdentity()) {
        RNFail("Unable to process scan without prior setting of intrinsics matrix in configuration file\n");
        return 0;
      }
      
      // Parse image names and alignment transformation
      RNScalar m[16];
      char depth_filename[2048], color_filename[2048];
      if (sscanf(buffer, "%s%s%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", cmd, 
         depth_filename, color_filename,
         &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], 
         &m[8], &m[9], &m[10], &m[11], &m[12], &m[13], &m[14], &m[15]) != (unsigned int) 19) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Create RGBD image
      // RGBDImage *image = new RGBDImage(color_filename, depth_filename, intrinsics_matrix, R4Matrix(m), image_width, image_height);
      RGBDImage *image = AllocateImage();
      image->SetNPixels(image_width, image_height);
      image->SetColorFilename(color_filename);
      image->SetDepthFilename(depth_filename);
      image->SetIntrinsics(intrinsics_matrix);
      image->SetCameraToWorld(R3Affine(R4Matrix(m), 0));
      InsertImage(image);
    }
    else if (!strcmp(cmd, "labeled_image")) {
      // Update/check image count
      if ((read_every_kth_image > 1) && ((++image_count % read_every_kth_image) != 1)) continue;

      // Check intrinsics matrix
      if (intrinsics_matrix.IsIdentity()) {
        RNFail("Unable to process scan without prior setting of intrinsics matrix in configuration file\n");
        return 0;
      }
      
      // Parse image names and alignment transformation
      RNScalar m[16];
      char depth_filename[2048], color_filename[2048], category_filename[2048], instance_filename[2048];
      if (sscanf(buffer, "%s%s%s%s%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", cmd, 
         depth_filename, color_filename, category_filename, instance_filename,
         &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], 
         &m[8], &m[9], &m[10], &m[11], &m[12], &m[13], &m[14], &m[15]) != (unsigned int) 21) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Create RGBD image
      // RGBDImage *image = new RGBDImage(color_filename, depth_filename, intrinsics_matrix, R4Matrix(m), image_width, image_height);
      RGBDImage *image = AllocateImage();
      image->SetNPixels(image_width, image_height);
      image->SetColorFilename(color_filename);
      image->SetDepthFilename(depth_filename);
      image->SetCategoryFilename(category_filename);
      image->SetInstanceFilename(instance_filename);
      image->SetIntrinsics(intrinsics_matrix);
      image->SetCameraToWorld(R3Affine(R4Matrix(m), 0));
      InsertImage(image);
    }
    else if (!strcmp(cmd, "frame")) {
      // Update/check image count
      if ((read_every_kth_image > 1) && ((++image_count % read_every_kth_image) != 1)) continue;

      // Check intrinsics matrix
      if (intrinsics_matrix.IsIdentity()) {
        RNFail("Unable to process scan without prior setting of intrinsics matrix in configuration file\n");
        return 0;
      }
      
      // Parse image names and alignment transformation
      RNScalar m[16];
      double depth_timestamp, color_timestamp;
      char depth_filename[2048], color_filename[2048];
      if (sscanf(buffer, "%s%s%lf%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", cmd, 
         depth_filename, &depth_timestamp, color_filename, &color_timestamp,
         &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], 
         &m[8], &m[9], &m[10], &m[11], &m[12], &m[13], &m[14], &m[15]) != (unsigned int) 21) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Create RGBD image
      // RGBDImage *image = new RGBDImage(color_filename, depth_filename, intrinsics_matrix, R4Matrix(m), image_width, image_height);
      RGBDImage *image = AllocateImage();
      image->SetNPixels(image_width, image_height);
      image->SetColorFilename(color_filename);
      image->SetDepthFilename(depth_filename);
      image->SetIntrinsics(intrinsics_matrix);
      image->SetCameraToWorld(R3Affine(R4Matrix(m), 0));
      InsertImage(image);
    }
    else if (!strcmp(cmd, "rectangle")) {
      // Parse surface
      char texture_filename[2048];
      RNScalar pixel_spacing, c[3], n[3], u[3], r[3];
      if (sscanf(buffer, "%s%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", cmd, texture_filename, &pixel_spacing, 
        &c[0], &c[1], &c[2], &n[0], &n[1], &n[2], &u[0], &u[1], &u[2], &r[0], &r[1], &r[2]) != (unsigned int) 15) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Create RGBD surface
      R3Point center(c);
      R3Vector axis1(u); axis1.Normalize();
      R3Vector normal(n); normal.Normalize();
      R3Vector axis0 = axis1 % normal; axis0.Normalize();
      // R3Mesh *mesh = new R3Mesh();
      // R3MeshVertex *v00 = mesh->CreateVertex(center - r[0]*axis0 - r[1]*axis1, normal, RNblack_rgb, R2Point(0,0));
      // R3MeshVertex *v10 = mesh->CreateVertex(center + r[0]*axis0 - r[1]*axis1, normal, RNblack_rgb, R2Point(r[0],0));
      // R3MeshVertex *v01 = mesh->CreateVertex(center - r[0]*axis0 + r[1]*axis1, normal, RNblack_rgb, R2Point(0,r[1]));
      // R3MeshVertex *v11 = mesh->CreateVertex(center + r[0]*axis0 + r[1]*axis1, normal, RNblack_rgb, R2Point(r[0],r[1]));
      // mesh->CreateFace(v00, v10, v11);
      // mesh->CreateFace(v00, v11, v01);
      // RGBDSurface *surface = new RGBDSurface(texture_filename, mesh, pixel_spacing);
      R3Rectangle *rectangle = new R3Rectangle(center, axis0, axis1, r[0], r[1]);
      RGBDSurface *surface = new RGBDSurface(texture_filename, rectangle, pixel_spacing);
      surface->thickness = r[2];
      InsertSurface(surface);
    }
    else if (!strcmp(cmd, "mesh")) {
      // Parse surface
      char texture_filename[2048], mesh_filename[2048];
      RNScalar pixel_spacing;
      if (sscanf(buffer, "%s%s%s%lf", cmd, texture_filename, mesh_filename, &pixel_spacing) != (unsigned int) 4) {
        RNFail("Error parsing line %d of configuration file\n", line_number);
        return 0;
      }

      // Read mesh
      R3Mesh *mesh = new R3Mesh();
      if (!mesh->ReadFile(mesh_filename)) {
        RNFail("Unable to read mesh file %s at line %d of configuration file\n", mesh_filename, line_number);
        return 0;
      }

      // Create surface      
      RGBDSurface *surface = new RGBDSurface(texture_filename, mesh, pixel_spacing);
      surface->SetMeshFilename(mesh_filename);
      InsertSurface(surface);
    }
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
WriteConfigurationStream(FILE *fp, int write_every_kth_image) const
{
  // Write header
  fprintf(fp, "n_images %d\n", NImages());
  if (Name()) fprintf(fp, "sequence %s\n", Name());
  if (DatasetFormat()) fprintf(fp, "dataset %s\n", DatasetFormat());
  if (color_directory) fprintf(fp, "color_directory %s\n", color_directory);
  if (depth_directory) fprintf(fp, "depth_directory %s\n", depth_directory);
  if (category_directory) fprintf(fp, "category_directory %s\n", category_directory);
  if (instance_directory) fprintf(fp, "instance_directory %s\n", instance_directory);
  if (texture_directory) fprintf(fp, "texture_directory %s\n", texture_directory);
  
  // Write blank line
  fprintf(fp, "\n");

  // Write surfaces
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    // surface->WriteChannels();
    if (surface->rectangle) {
      R3Point centroid = surface->rectangle->Centroid();
      R3Vector normal = surface->rectangle->Normal();
      R3Vector up = surface->rectangle->Axis(1);
      RNLength rx = surface->rectangle->Radius(0);
      RNLength ry = surface->rectangle->Radius(1);
      const char *texture_filename = (surface->TextureFilename()) ? surface->TextureFilename() : "-";
      fprintf(fp, "rectangle  %s  %g   %g %g %g   %g %g %g   %g %g %g   %g %g %g\n",
        texture_filename, surface->WorldTexelSpacing(),
        centroid.X(), centroid.Y(), centroid.Z(),
        normal.X(), normal.Y(), normal.Z(),
        up.X(), up.Y(), up.Z(), rx, ry, surface->thickness);
    }
    else if (surface->mesh) {
      const char *texture_filename = (surface->TextureFilename()) ? surface->TextureFilename() : "-";
      const char *mesh_filename = (surface->MeshFilename()) ? surface->MeshFilename() : "-";
      fprintf(fp, "mesh  %s  %s  %g\n", texture_filename, mesh_filename, surface->WorldTexelSpacing());
    }
  }
  
  // Write blank line
  fprintf(fp, "\n");

  // Write images
  int image_width = 0;
  int image_height = 0;
  R3Matrix intrinsics_matrix = R3identity_matrix;
  for (int i = 0; i < NImages(); i++) {
    if ((write_every_kth_image > 1) && ((i % write_every_kth_image) != 0)) continue;
    RGBDImage *image = Image(i);

    // Write image dimensions
    if ((image->NPixels(RN_X) > 0) && (image->NPixels(RN_Y) > 0)) {
      if ((image->NPixels(RN_X) != image_width) || (image->NPixels(RN_Y) != image_height)) {
        fprintf(fp, "depth_resolution %d %d\n", image->NPixels(RN_X), image->NPixels(RN_Y));
        image_width = image->NPixels(RN_X);
        image_height = image->NPixels(RN_Y);
      }
    }

    // Write intrinsics matrix
    if (!image->Intrinsics().IsZero()) {
      if (image->Intrinsics() != intrinsics_matrix) {
        intrinsics_matrix = image->Intrinsics();
        fprintf(fp, "intrinsics_matrix  %g %g %g  %g %g %g  %g %g %g\n",
          intrinsics_matrix[0][0], intrinsics_matrix[0][1], intrinsics_matrix[0][2],
          intrinsics_matrix[1][0], intrinsics_matrix[1][1], intrinsics_matrix[1][2],
          intrinsics_matrix[2][0], intrinsics_matrix[2][1], intrinsics_matrix[2][2]);
      }
    }

    // Write images
    // if (!image->WriteChannels()) return 0;

    // Write image with extrinsics
    R4Matrix m = image->CameraToWorld().Matrix();
    const char *depth_filename = (image->DepthFilename()) ? image->DepthFilename() : "-";
    const char *color_filename = (image->ColorFilename()) ? image->ColorFilename() : "-";
    fprintf(fp, "image %s %s  %g %g %g %g  %g %g %g %g  %g %g %g %g  %g %g %g %g\n",
       depth_filename, color_filename, 
       m[0][0], m[0][1], m[0][2], m[0][3],
       m[1][0], m[1][1], m[1][2], m[1][3], 
       m[2][0], m[2][1], m[2][2], m[2][3],
       m[3][0], m[3][1], m[3][2], m[3][3]);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Segmentation input functions
////////////////////////////////////////////////////////////////////////

int RGBDConfiguration::
ReadSegmentationFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open segmentation file %s\n", filename);
    return 0;
  }

  // Read file
  int id, npoints, primitive_type;
  double area, total_affinity, possible_affinity, timestamp;
  R3Point centroid, center;
  R3Plane plane;
  RNRgb color;
  R3Vector axes[3];
  double variances[3];
  R3Box extent;
  while (fscanf(fp, "%d%d%lf%lf%lf%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
    &id, &npoints, &area,
    &total_affinity, &possible_affinity, &primitive_type,
    &centroid[0], &centroid[1], &centroid[2],
    &plane[0], &plane[1], &plane[2], &plane[3], 
    &color[0], &color[1], &color[2],
    &timestamp,
    &center[0], &center[1], &center[2],
    &axes[0][0], &axes[0][1], &axes[0][2],
    &axes[1][0], &axes[1][1], &axes[1][2],
    &axes[2][0], &axes[2][1], &axes[2][2],
    &variances[0], &variances[1], &variances[2],
    &extent[0][0], &extent[0][1], &extent[0][2],
    &extent[1][0], &extent[1][1], &extent[1][2]) == (unsigned int) 38) {

    // Determine radii
    RNLength radius0 = 0.5 * (extent[1][0] - extent[0][0]);
    if (RNIsZero(radius0)) continue;
    RNLength radius1 = 0.5 * (extent[1][1] - extent[0][1]);
    if (RNIsZero(radius1)) continue;
    RNLength radius2 = 0.5 * (extent[1][2] - extent[0][2]);

    // Determine center of extent
    center += (radius0 + extent[0][0]) * axes[0];
    center += (radius1 + extent[0][1]) * axes[1];

    // Create rectangle
    R3Rectangle *rectangle = new R3Rectangle(center, axes[0], axes[1], radius0, radius1);

    // Create RGBD surface
    double texel_spacing = 0.01;
    RGBDSurface *rgbd_surface = new RGBDSurface(NULL, rectangle, texel_spacing);
    rgbd_surface->thickness = radius2;

    // Insert RGBD surface
    InsertSurface(rgbd_surface);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Oriented box input functions
////////////////////////////////////////////////////////////////////////

int RGBDConfiguration::
ReadOrientedBoxFile(const char *filename)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open obb file %s\n", filename);
    return 0;
  }

  // Read file
  R3Point center;
  R3Vector axis[2];
  double radius[3];
  while (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
    &center[0], &center[1], &center[2],
    &axis[0][0], &axis[0][1], &axis[0][2],
    &axis[1][0], &axis[1][1], &axis[1][2],
    &radius[0], &radius[1], &radius[2]) == (unsigned int) 12) {
    // Make sure axes are orthogonal
    axis[0].Normalize();
    R3Vector axis2 = axis[0] % axis[1];
    axis[1] = axis2 % axis[0];
    axis[1].Normalize();
    
    // Create rectangle
    R3Rectangle *rectangle = new R3Rectangle(center,
      axis[0], axis[1], radius[0], radius[1]);

    // Create RGBD surface
    RGBDSurface *rgbd_surface = new RGBDSurface(NULL, rectangle);
    rgbd_surface->thickness = radius[2];

    // Insert RGBD surface
    InsertSurface(rgbd_surface);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// NeRF input/output functions
////////////////////////////////////////////////////////////////////////

static int
GetJsonObjectMember(Json::Value *&result, Json::Value *object, const char *str, int expected_type = 0)
{
  // Check object type
  if (object->type() != Json::objectValue) {
    // RNFail("JSON: not an object\n");
    return 0;
  }

  // Check object member
  if (!object->isMember(str)) {
    // RNFail("JSON object has no member named %s\n", str);
    return 0;
  }

  // Get object member
  result = &((*object)[str]);
  if (result->type() == Json::nullValue) {
    // RNFail("JSON object has null member named %s\n", str);
    return 0;
  }

  // Check member type
  if (expected_type > 0) {
    if (result->type() != expected_type) {
      // RNFail("JSON object member %s has unexpected type %d (rather than %d)\n", str, result->type(), expected_type);
      return 0;
    }
  }
  
  // Check for empty strings
  if (result->type() == Json::stringValue) {
    if (result->asString().length() == 0) {
      // RNFail("JSON object has zero length string named %s\n", str);
      return 0;
    }
  }

  // Return success
  return 1;
}



static int
GetJsonArrayEntry(Json::Value *&result, Json::Value *array, unsigned int k, int expected_type = -1)
{
  // Check array type
  if (array->type() != Json::arrayValue) {
    RNFail("JSON: not an array\n");
    return 0;
  }

  // Check array size
  if (array->size() <= k) {
    // RNFail("JSON array has no member %d\n", k);
    return 0;
  }

  // Get entry
  result = &((*array)[k]);
  if (result->type() == Json::nullValue) {
    // RNFail("JSON array has null member %d\n", k);
    return 0;
  }

  // Check entry type
  if (expected_type > 0) {
    if (result->type() != expected_type) {
      // RNFail("JSON array entry %d has unexpected type %d (rather than %d)\n", k, result->type(), expected_type);
      return 0;
    }
  }
  
  // Return success
  return 1;
}



int RGBDConfiguration::
ReadNeRFFile(const char *filename, int read_every_kth_image)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    RNFail("Unable to open NeRF file %s\n", filename);
    return 0;
  }

  // Read file 
  std::string text;
  fseek(fp, 0, SEEK_END);
  long const size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char* buffer = new char[size + 1];
  unsigned long const usize = static_cast<unsigned long const>(size);
  if (fread(buffer, 1, usize, fp) != usize) { RNFail("Unable to read %s\n", filename); return 0; }
  else { buffer[size] = 0; text = buffer; }
  delete[] buffer;

  // Close file
  fclose(fp);

  // Parse file
  Json::Value json_root;
  Json::Reader json_reader;
  if (!json_reader.parse(text, json_root, false)) {
    RNFail("Unable to parse %s\n", filename);
    return 0;
  }

  // Get top level stuff
  Json::Value *json_camera_angle_x = NULL;
  Json::Value *json_frames = NULL;
  if (!json_root.isObject() ||
      !GetJsonObjectMember(json_camera_angle_x, &json_root, "camera_angle_x") ||
      !GetJsonObjectMember(json_frames, &json_root, "frames")) {
    RNFail("NeRF file is not expected format\n", filename);
    return 0;
  }

  // Parse x fov 
  int width = 0;  // Gets filled from first image
  int height = 0; // Gets filled from first image
  double xfov = 0.5 * json_camera_angle_x->asDouble();
  if (xfov <= 0) {
    RNFail("Error parsing camera_angle_x in %s\n", filename);
    return 0;
  }

  // Parse frames
  for (Json::ArrayIndex i = 0; i < json_frames->size(); i++) {
    // Check if skipping images
    if ((read_every_kth_image > 1) && ((i % read_every_kth_image) != 0)) continue;
    
    // Get frame
    Json::Value *json_frame = NULL;
    GetJsonArrayEntry(json_frame, json_frames, i);
    if (!json_frame->isObject()) continue;

    // Parse filepath
    std::string file_path;
    Json::Value *json_file_path = NULL;
    Json::Value *json_transform = NULL;
    if (!GetJsonObjectMember(json_file_path, json_frame, "file_path") ||
        !GetJsonObjectMember(json_transform, json_frame, "transform_matrix") ||
        !json_transform->isArray() || (json_transform->size() != 4)) {
      RNFail("Error parsing frame %d\n", i);
      return 0;
    }

    // Parse image filenames
    std::string depth_filename = "-";
    std::string color_filename = json_file_path->asString() + ".png";
    if (!RNFileExists(color_filename.c_str())) {
      color_filename = json_file_path->asString() + ".jpg";
    }

    // Get width and height
    if ((width <= 0) || (height <= 0)) {
      R2Image color_image;
      if (!color_image.ReadFile(color_filename.c_str())) return 0;
      width = color_image.Width();
      height = color_image.Height();
      if ((width <= 0) || (height <= 0)) {
        RNFail("Error parsing image width and height in %s\n", filename);
        return 0;
      }
    }

    // Get intrinsics matrix
    R3Matrix intrinsics = R3identity_matrix;
    double focal = 0.5 * width / tan(xfov);
    intrinsics[0][0] = focal;
    intrinsics[1][1] = focal;
    intrinsics[0][2] = 0.5 * width;
    intrinsics[1][2] = 0.5 * height;
      
    // Parse rotation (what is this?)
    // XXX
    
    // Parse transformation matrix
    R4Matrix matrix = R4identity_matrix;
    for (int j = 0; j < 4; j++) {
      Json::Value *json_column = NULL;
      GetJsonArrayEntry(json_column, json_transform, j);
      if (!json_transform->isArray() || (json_column->size() != 4)) {
        RNFail("Error parsing tranform %d\n", i);
        return 0;
      }
      for (int k = 0; k < 4; k++) {
        Json::Value *json_value = NULL;
        GetJsonArrayEntry(json_value, json_column, k);
        matrix[j][k] = json_value->asDouble();
      }
    }

    // Compute camera-to-world matrix
    R4Matrix camera_to_world = R4identity_matrix;
    camera_to_world.XRotate(-RN_PI_OVER_TWO);
    camera_to_world = camera_to_world * matrix;

    // Create image
    RGBDImage *image = new RGBDImage(color_filename.c_str(), depth_filename.c_str(),
      intrinsics, camera_to_world, width, height);

    // Insert image
    InsertImage(image);
  }
  
  // Return success
  return 1;
}



int RGBDConfiguration::
WriteNeRFFile(const char *filename, int write_every_kth_image) const
{
  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open NeRF file %s\n", filename);
    return 0;
  }

  // Write header
  double xfov = (NImages() > 0) ? Image(0)->XFov() : 0;
  fprintf(fp, "{\n");
  fprintf(fp, "  \"camera_angle_x\": %.12f,\n", 2 * xfov);
  fprintf(fp, "  \"frames\": [\n");

  // Write images
  for (int i = 0; i < NImages(); i++) {
    if ((write_every_kth_image > 1) && ((i % write_every_kth_image) != 0)) continue;
    RGBDImage *image = Image(i);
    if (!image->ColorFilename()) continue;

    // Get filename
    char filename[1024] = { '\0' };
    strncpy(filename, image->ColorFilename(), 1023);
    char *endp = strrchr(filename, '.');
    if (endp) *endp = '\0';

    // Print opening curly bracket for image
    fprintf(fp, "    {\n");

    // Print file path
    if (!color_directory) fprintf(fp, "      \"file_path\": \"%s\",\n", filename);
    else fprintf(fp, "      \"file_path\": \"%s/%s\",\n", color_directory, filename);

    // Print rotation (what is this?)
    // XXX
    
    // Print transformation matrix
    R4Matrix m = R4identity_matrix;
    m.XRotate(RN_PI_OVER_TWO);
    m = m * image->CameraToWorld().Matrix();
    fprintf(fp, "      \"transform_matrix\":\n");
    fprintf(fp, "        [");
    for (int j = 0; j < 4; j++) {
      fprintf(fp, "          [\n");
      for (int k = 0; k < 4; k++) {
        fprintf(fp, "            %.12f", m[j][k]);
        if (k < 3) fprintf(fp, ",");
        fprintf(fp, "\n");
      }
      fprintf(fp, "          ]");
      if (j < 3) fprintf(fp, ",");
      fprintf(fp, "\n");
    }
    fprintf(fp, "        ]");

    // Print trailing curly bracket for image
    if (i < NImages()-1) fprintf(fp, "    },\n");
    else fprintf(fp, "    }\n");
  }
  
  // Write trailer
  fprintf(fp, "  ]\n");
  fprintf(fp, "}\n");
  
  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Camera input/output functions
////////////////////////////////////////////////////////////////////////

int RGBDConfiguration::
WriteCameraFile(const char *filename, int write_every_kth_image) const
{
  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open camera file %s\n", filename);
    return 0;
  }

  // Write images
  for (int i = 0; i < NImages(); i++) {
    if ((write_every_kth_image > 1) && ((i % write_every_kth_image) != 0)) continue;
    RGBDImage *image = Image(i);
    R3Point viewpoint = image->WorldViewpoint();
    R3Vector towards = image->WorldTowards();
    R3Vector up = image->WorldUp();
    RNAngle xfov = image->XFov();
    RNAngle yfov = image->YFov();
    fprintf(fp, "%f %f %f   %f %f %f   %f %f %f  %f %f  1\n",
      viewpoint[0], viewpoint[1], viewpoint[2],
      towards[0], towards[1], towards[2],
      up[0], up[1], up[2],
      xfov, yfov);
  }
  
  // Close file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// OBJ output functions
////////////////////////////////////////////////////////////////////////

static int
WriteMtlFile(const RGBDConfiguration *configuration, const char *filename)
{
  // Check texture directory name
  if (!configuration->TextureDirectory()) return 1;

  // Open mtl file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open file %s", filename);
    return 0;
  }

  // Write materials
  for (int i = 0; i < configuration->NSurfaces(); i++) {
    RGBDSurface *surface = configuration->Surface(i);
    if (!surface->TextureFilename()) continue;
    fprintf(fp, "newmtl surface%d\n", i);
    fprintf(fp, "Kd 1 1 1\n");
    fprintf(fp, "map_Kd %s/%s\n", configuration->TextureDirectory(), surface->TextureFilename());
    fprintf(fp, "\n");
  }
  
  // Close mtl file
  fclose(fp);

  // Return success
  return 1;
}



int RGBDConfiguration::
WriteObjFile(const char *filename) const
{
  // Write mtl file
  char mtl_filename[1024];
  strncpy(mtl_filename, filename, 1020);
  char *endp = strrchr(mtl_filename, '.');
  if (endp) { *endp = '\0'; strcat(mtl_filename, ".mtl"); }
  if (!WriteMtlFile(this, mtl_filename)) return 0;

  // Open obj file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    RNFail("Unable to open file %s", filename);
    return 0;
  }

  // Write material library
  char *startp = strrchr(mtl_filename, '/');
  if (startp) startp++;
  else startp = mtl_filename;
  fprintf(fp, "mtllib %s\n", startp);

  // Write surfaces
  for (int s = 0; s < NSurfaces(); s++) {
    RGBDSurface *surface = Surface(s);
    int width = surface->NTexels(RN_X);
    int height = surface->NTexels(RN_Y);
    if ((width == 0) || (height == 0)) continue;
    
    // Write mesh
    R3Mesh *mesh = surface->mesh;
    if (mesh) {
      // Write material reference
      fprintf(fp, "g surface%d\n", s);
      fprintf(fp, "usemtl surface%d\n", s);

      // Write vertices
      for (int i = 0; i < mesh->NVertices(); i++) {
        R3MeshVertex *vertex = mesh->Vertex(i);
        const R3Point& p = mesh->VertexPosition(vertex);
        R2Point t, s = mesh->VertexTextureCoords(vertex);
        surface->TransformSurfaceToTexture(s, t);
        fprintf(fp, "vt %g %g\n", t.X() / width, t.Y() / height);
        fprintf(fp, "v %g %g %g\n", p.X(), p.Y(), p.Z());
      }

      // Write faces
      for (int i = 0; i < mesh->NFaces(); i++) {
        R3MeshFace *face = mesh->Face(i);
        R3MeshVertex *v0 = mesh->VertexOnFace(face, 0);
        R3MeshVertex *v1 = mesh->VertexOnFace(face, 1);
        R3MeshVertex *v2 = mesh->VertexOnFace(face, 2);
        int i0 = mesh->VertexID(v0) + 1;
        int i1 = mesh->VertexID(v1) + 1;
        int i2 = mesh->VertexID(v2) + 1;
        fprintf(fp, "f %d/%d %d/%d %d/%d\n", i0, i0, i1, i1, i2, i2);
      }
    }
  }
  
  // Close obj file
  fclose(fp);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Read/release functions
////////////////////////////////////////////////////////////////////////

int RGBDConfiguration::
ReadChannels(void)
{
  // Read images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReadChannels();
  }

  // Read surfaces
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    surface->ReadChannels();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReleaseChannels(void)
{
  // Release images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReleaseChannels();
  }

  // Release surfaces
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    surface->ReleaseChannels();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReadColorChannels(void)
{
  // Read images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReadColorChannels();
  }

  // Read surfaces
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    surface->ReadColorChannels();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReleaseColorChannels(void)
{
  // Release images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReleaseColorChannels();
  }

  // Release surfaces
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    surface->ReleaseColorChannels();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReadDepthChannels(void)
{
  // Read images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReadDepthChannel();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReleaseDepthChannels(void)
{
  // Release images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReleaseDepthChannel();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReadCategoryChannels(void)
{
  // Read images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReadCategoryChannel();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReleaseCategoryChannels(void)
{
  // Release images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReleaseCategoryChannel();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReadInstanceChannels(void)
{
  // Read images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReadInstanceChannel();
  }

  // Return success
  return 1;
}



int RGBDConfiguration::
ReleaseInstanceChannels(void)
{
  // Release images
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    image->ReleaseInstanceChannel();
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Update functions
////////////////////////////////////////////////////////////////////////

void RGBDConfiguration::
InvalidateWorldBBox(void)
{
  // Mark bounding box for recomputation
  world_bbox.Reset(R3Point(FLT_MAX, FLT_MAX, FLT_MAX), R3Point(-FLT_MAX, -FLT_MAX, -FLT_MAX));
}



void RGBDConfiguration::
UpdateWorldBBox(void)
{
  // Initialize bounding box
  world_bbox = R3null_box;

  // Union image bounding boxes
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    world_bbox.Union(image->WorldViewpoint());
    if (!image->world_bbox.IsEmpty()) {
      world_bbox.Union(image->world_bbox);
    }
  }

  // Union surface bounding boxes
  for (int i = 0; i < NSurfaces(); i++) {
    RGBDSurface *surface = Surface(i);
    world_bbox.Union(surface->WorldBBox());
  }
}



RGBDImage *RGBDConfiguration::
AllocateImage(void) const
{
  // Allocate image (can be over-ridden by derived class)
  return new RGBDImage();
}



RGBDImage *RGBDConfiguration::
FindImage(const char *name) const
{
  // Find image with matching name
  for (int i = 0; i < NImages(); i++) {
    RGBDImage *image = Image(i);
    if (image->Name() && !strcmp(image->Name(), name)) return image;
  }

  // Image not found
  return NULL;
}



} // namespace gaps
