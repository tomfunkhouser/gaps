// Source file for the surfel scene processing program



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

namespace gaps {}
using namespace gaps;
#include "R3Surfels/R3Surfels.h"
#include "R3Shapes/ply.h"



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

static const char *input_surfel_scene_name = NULL;
static const char *input_surfel_database_name = NULL;
static const char *output_points_name = NULL;
static int load_every_kth_image = 1;
static int load_every_kth_pixel = 1;
static int scale_normals_by_confidence = 1;
static int sample_by_confidence = 0;
static double sample_probability = 1;
static int omit_boundaries = 0;
static int omit_corners = 0;
static int max_points = 0;
static double max_depth = 0;
static int print_verbose = 0;



////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////

typedef struct PlyPoint {
  float x, y, z;
  float nx, ny, nz;
  unsigned char red, green, blue;
  float confidence;
  float radius;
} PlyPoint;



////////////////////////////////////////////////////////////////////////
// I/O Functions
////////////////////////////////////////////////////////////////////////

static R3SurfelScene *
OpenSurfelScene(const char *input_scene_name, const char *input_database_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate scene
  R3SurfelScene *scene = new R3SurfelScene();
  if (!scene) {
    RNFail("Unable to allocate scene\n");
    return NULL;
  }

  // Open scene files
  if (!scene->OpenFile(input_scene_name, input_database_name, "r", "r")) {
    delete scene;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Opened scene ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Scans = %d\n", scene->NScans());
    printf("  # Objects = %d\n", scene->NObjects());
    printf("  # Labels = %d\n", scene->NLabels());
    printf("  # Assignments = %d\n", scene->NLabelAssignments());
    printf("  # Features = %d\n", scene->NFeatures());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Return scene
  return scene;
}



static int
CloseSurfelScene(R3SurfelScene *scene)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Print statistics
  if (print_verbose) {
    printf("Closing scene ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Scans = %d\n", scene->NScans());
    printf("  # Objects = %d\n", scene->NObjects());
    printf("  # Labels = %d\n", scene->NLabels());
    printf("  # Assignments = %d\n", scene->NLabelAssignments());
    printf("  # Features = %d\n", scene->NFeatures());
    printf("  # Nodes = %d\n", scene->Tree()->NNodes());
    printf("  # Blocks = %d\n", scene->Tree()->Database()->NBlocks());
    printf("  # Surfels = %lld\n", scene->Tree()->Database()->NSurfels());
    fflush(stdout);
  }

  // Close scene files
  if (!scene->CloseFile()) {
    delete scene;
    return 0;
  }

  // Return success
  return 1;
}



static int
WritePoints(const RNArray<PlyPoint *>& points, const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    printf("Filename %s has no extension (e.g., .ply)\n", filename);
    return 0;
  }

  // Check file type
  if (!strcmp(extension, ".ply")) {
    // Define ply element names
    char *elem_names[] = { (char *) "vertex" };

    // Define property information for a point 
    static PlyProperty vert_props[] = { 
      {(char *) "x", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,x), 0, 0, 0, 0},
      {(char *) "y", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,y), 0, 0, 0, 0},
      {(char *) "z", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,z), 0, 0, 0, 0},
      {(char *) "nx", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,nx), 0, 0, 0, 0},
      {(char *) "ny", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,ny), 0, 0, 0, 0},
      {(char *) "nz", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,nz), 0, 0, 0, 0},
      {(char *) "red", PLY_UCHAR, PLY_UCHAR, offsetof(PlyPoint,red), 0, 0, 0, 0},
      {(char *) "green", PLY_UCHAR, PLY_UCHAR, offsetof(PlyPoint,green), 0, 0, 0, 0},
      {(char *) "blue", PLY_UCHAR, PLY_UCHAR, offsetof(PlyPoint,blue), 0, 0, 0, 0},
      {(char *) "confidence", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,confidence), 0, 0, 0, 0},
      {(char *) "value", PLY_FLOAT, PLY_FLOAT, offsetof(PlyPoint,radius), 0, 0, 0, 0}
    };

    // Open ply file
    float version;
    PlyFile *ply = ply_open_for_writing((char *) filename, 1, elem_names, PLY_BINARY_NATIVE, &version);
    if (!ply) return -1;

    // Describe point properties
    ply_element_count(ply, (char *) "vertex", points.NEntries());
    ply_describe_property(ply, (char *) "vertex", &vert_props[0]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[1]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[2]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[3]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[4]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[5]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[6]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[7]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[8]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[9]);
    ply_describe_property(ply, (char *) "vertex", &vert_props[10]);

    // Complete header
    ply_header_complete(ply);

    // Write points
    ply_put_element_setup(ply, (char *) "vertex");
    for (int i = 0; i < points.NEntries(); i++) {
      ply_put_element(ply, (void *) points[i]);
    }

    // Close the file 
    ply_close(ply);
  }
  else if (!strcmp(extension, ".xyzn")) {
    // Open file
    FILE *fp = stdout;
    if (filename) {
      fp = fopen(filename, "w");
      if (!fp) {
        RNFail("Unable to open output file %s\n", filename);
        return FALSE;
      }
    }

    // Write points
    for (int i = 0; i < points.NEntries(); i++) {
      PlyPoint *point = points[i];
      fprintf(fp, "%g %g %g   %g %g %g\n",
        point->x, point->y, point->z,
        point->nx, point->ny, point->nz);
    }

    // Close file
    if (filename) fclose(fp);
  }
  else if (!strcmp(extension, ".xyz")) {
    // Open file
    FILE *fp = stdout;
    if (filename) {
      fp = fopen(filename, "w");
      if (!fp) {
        RNFail("Unable to open output file %s\n", filename);
        return FALSE;
      }
    }

    // Write points
    for (int i = 0; i < points.NEntries(); i++) {
      PlyPoint *point = points[i];
      fprintf(fp, "%g %g %g\n", point->x, point->y, point->z);
    }

    // Close file
    if (filename) fclose(fp);
  }
  else if (!strcmp(extension, ".pts")) {
    // Open file
    FILE *fp = stdout;
    if (filename) {
      fp = fopen(filename, "wb");
      if (!fp) {
        RNFail("Unable to open output file %s\n", filename);
        return FALSE;
      }
    }

    // Write points
    float coordinates[6];
    for (int i = 0; i < points.NEntries(); i++) {
      PlyPoint *point = points[i];
      coordinates[0] = point->x;
      coordinates[1] = point->y;
      coordinates[2] = point->z;
      coordinates[3] = point->nx;
      coordinates[4] = point->ny;
      coordinates[5] = point->nz;
      if (fwrite(coordinates, sizeof(float), 6, fp) != (unsigned int) 6) {
        RNFail("Unable to write point to output file %s\n", filename);
        return FALSE;
      }
    }

    // Close file
    if (filename) fclose(fp);
  }
  else {
    // Create mesh
    R3Mesh mesh;
    for (int i = 0; i < points.NEntries(); i++) {
      PlyPoint *point = points[i];
      R3Point position(point->x, point->y, point->z);
      R3Vector normal(point->nx, point->ny, point->nz);
      mesh.CreateVertex(position, normal);
    }

    // Write mesh file
    if (!mesh.WriteFile(filename)) return FALSE;
  }

  // Print statistics
  if (print_verbose) {
    printf("Wrote points to %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Points = %u\n", points.NEntries());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Processing functions
////////////////////////////////////////////////////////////////////////

static int
CreatePoints(R3SurfelScene *scene, RNArray<PlyPoint *>& points)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Get useful variables
  R3SurfelTree *tree = scene->Tree();
  if (!tree) return 0;
  R3SurfelDatabase *database = tree->Database();
  if (!database) return 0;

  // Add point for every surfel in a leaf node
  for (int i = 0; i < tree->NNodes(); i++) {
    R3SurfelNode *node = tree->Node(i);
    if (node->NParts() > 0) continue;
    
    // Get/check scan
    R3SurfelScan *scan = node->Scan(TRUE);
    if (scan && (load_every_kth_image > 1) && ((scan->SceneIndex() % load_every_kth_image) != 0)) {
      continue;
    }

    // Get scan info
    int scan_width = (scan) ? scan->ImageWidth() : 0;      
    int scan_height = (scan) ? scan->ImageHeight() : 0;
    if ((scan_width == 0) || (scan_height == 0)) scan = NULL;
    R3Point E = (scan) ? scan->Viewpoint() : R3zero_point;
    R3Vector T = (scan) ? scan->Towards() : R3zero_vector;
    RNScalar min_dot = (scan) ? ((scan->XFOV() < scan->YFOV()) ? cos(scan->XFOV()) : cos(scan->YFOV())) : 0;
    RNScalar min_dotdot = min_dot * min_dot;

    // Process blocks
    for (int i = 0; i < node->NBlocks(); i++) {
      R3SurfelBlock *block = node->Block(i);

      // Read block
      database->ReadBlock(block);

      // Process surfels
      for (int j = 0; j < block->NSurfels(); j++) {
        const R3Surfel *surfel = block->Surfel(j);

        // Check sample probability
        if (RNIsLess(sample_probability, 1.0)) {
          if (RNRandomScalar() > sample_probability) continue;
        }

        // Check if on boundary
        if (omit_boundaries) {
          if (surfel->IsOnSilhouetteBoundary()) continue;
          if (surfel->IsOnShadowBoundary()) continue;
        }
        
        // Get info
        RNScalar confidence = 1.0;
        R3Point P = block->SurfelPosition(j);
        R3Vector N = block->SurfelNormal(j);

        // Update info based on scan
        if (scan) {
          // Get scan view info
          R3Vector V = P - E;

          // Get/check depth
          RNLength d = V.Dot(T);
          if (d < RN_EPSILON) continue;
          if ((max_depth > 0) && (d > max_depth)) continue;
          V.Normalize();

          // Check if outside elipse centered in image
          if (omit_corners) {
            RNScalar dot = T.Dot(V);
            if (dot*dot < min_dotdot) continue;
          }

          // Update confidence
          // RNScalar NdotV = N.Dot(V);
          // if (NdotV >= 0) continue;
          // confidence *= -NdotV;
          // confidence *= T.Dot(V);
          if (d > 1.0) confidence /= d*d;
          if (confidence < 1E-3F) continue;

          // Update normal
          if (scale_normals_by_confidence) {
            N[0] *= confidence;
            N[1] *= confidence;
            N[2] *= confidence;
          }
        }

        // Allocate ply point
        PlyPoint *v = new PlyPoint();
        
        // Fill in ply point position
        v->x = P.X();
        v->y = P.Y();
        v->z = P.Z();

        // Fill in ply point normal
        v->nx = N.X();
        v->ny = N.Y();
        v->nz = N.Z();

        // Fill in ply point color
        v->red = surfel->R();
        v->green = surfel->G();
        v->blue = surfel->B();

        // Fill in ply point confidence and radius
        assert(confidence <= 1.0);
        v->confidence = confidence;
        v->radius = block->SurfelRadius(j);

        // Insert point
        points.Insert(v);
      }

      // Release block
      database->ReleaseBlock(block);
    }
  }

  // Print debug statistics
  if (print_verbose) {
    printf("Created points ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Points = %u\n", points.NEntries());
    printf("  Sample probability = %g\n", sample_probability);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
ComparePlyPoints(const void *data1, const void *data2)
{
  // Sort by confidence
  PlyPoint *point1 = *((PlyPoint **) data1);
  PlyPoint *point2 = *((PlyPoint **) data2);
  unsigned char c1 = point1->confidence;
  unsigned char c2 = point2->confidence;
  if (c1 > c2) return -1;
  else if (c2 > c1) return 1;
  else return 0;
}



static int
SamplePoints(RNArray<PlyPoint *>& points, int max_points)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Check if need to sample
  if ((max_points == 0) || (points.NEntries() < max_points)) {
    return 1;
  }

  // Sort points
  if (sample_by_confidence) {
    points.Sort(ComparePlyPoints);
  }

  // Mark extra points 
  int n = points.NEntries() - max_points;
  for (int i = 0; i < 8*points.NEntries(); i++) {
    RNScalar r = RNRandomScalar();
    if (sample_by_confidence) r = 1.0 - r*r;
    int j = r * points.NEntries();
    if (j >= points.NEntries()) j = points.NEntries()-1;
    PlyPoint *point = points.Kth(j);
    if (point->confidence == 0) continue;
    point->confidence = 0;
    if (--n == 0) break;
  }

  // Delete extra points
  RNScalar total_confidence = 0;
  RNArray<PlyPoint *> survivors;
  for (int i = 0; i < points.NEntries(); i++) {
    PlyPoint *point = points.Kth(i);
    if (point->confidence == 0) { delete point; continue; }
    total_confidence += point->confidence;
    survivors.Insert(point);
  }

  // Keep only survivors
  points = survivors;

  // Print debug statistics
  if (print_verbose) {
    printf("Sampled points ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Points = %u\n", points.NEntries());
    printf("  Confidence = %g\n", total_confidence / (255.0 * points.NEntries()));
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// PROGRAM ARGUMENT PARSING
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-omit_corners")) omit_corners = 1; 
      else if (!strcmp(*argv, "-dont_omit_corners")) omit_corners = 0; 
      else if (!strcmp(*argv, "-omit_boundaries")) omit_boundaries = 1; 
      else if (!strcmp(*argv, "-dont_omit_boundaries")) omit_boundaries = 0; 
      else if (!strcmp(*argv, "-scale_normals_by_confidence")) scale_normals_by_confidence = 1; 
      else if (!strcmp(*argv, "-dont_scale_normals_by_confidence")) scale_normals_by_confidence = 0; 
      else if (!strcmp(*argv, "-sample_by_confidence")) sample_by_confidence = 1; 
      else if (!strcmp(*argv, "-dont_sample_by_confidence")) sample_by_confidence = 0; 
      else if (!strcmp(*argv, "-sample_probability")) { argc--; argv++; sample_probability = atof(*argv); }
      else if (!strcmp(*argv, "-load_every_kth_image")) { argc--; argv++; load_every_kth_image = atoi(*argv); }
      else if (!strcmp(*argv, "-load_every_kth_pixel")) { argc--; argv++; load_every_kth_pixel = atoi(*argv); }
      else if (!strcmp(*argv, "-max_points")) { argc--; argv++; max_points = atoi(*argv); }
      else if (!strcmp(*argv, "-max_depth")) { argc--; argv++; max_depth = atof(*argv); }
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
    else {
      if (!input_surfel_scene_name) input_surfel_scene_name = *argv;
      else if (!input_surfel_database_name) input_surfel_database_name = *argv;
      else if (!output_points_name) output_points_name = *argv;
      else { RNFail("Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Check file names
  if (!input_surfel_scene_name || !input_surfel_database_name || !output_points_name) {
    RNFail("Usage: sfl2pts scenefile databasefile pointsfile [options]\n");
    return FALSE;
  }
  
  // Return OK status 
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);

  // Open surfel scene
  R3SurfelScene *surfel_scene = OpenSurfelScene(input_surfel_scene_name, input_surfel_database_name);
  if (!surfel_scene) exit(-1);

  // Create points and write to ply file
  if (max_points > 0) {
    if (max_points < surfel_scene->Tree()->Database()->NSurfels()) {
      RNScalar p = (RNScalar) (2*max_points) / (RNScalar) surfel_scene->Tree()->Database()->NSurfels();
      p *= load_every_kth_image;
      p *= load_every_kth_pixel * load_every_kth_pixel;
      if (p < sample_probability) sample_probability = p;
    }
  }

  // Create points
  RNArray<PlyPoint *> points;
  if (!CreatePoints(surfel_scene, points)) exit(-1);

  // Sample points
  if (!SamplePoints(points, max_points)) exit(-1);

  // Write points file
  if (!WritePoints(points, output_points_name)) exit(-1);

  // Delete points
  for (int i = 0; i < points.NEntries(); i++) delete points[i];

  // Close surfel scene
  if (!CloseSurfelScene(surfel_scene)) exit(-1);

  // Return success 
  return 0;
}



