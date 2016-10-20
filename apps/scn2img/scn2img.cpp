// Source file for the scene image capture program



////////////////////////////////////////////////////////////////////////
// Include files 
////////////////////////////////////////////////////////////////////////

#include "R3Graphics/R3Graphics.h"
#include "R3Graphics/p5d.h"
#ifdef USE_MESA
#  include "GL/osmesa.h"
#else
#  include "fglut/fglut.h" 
#  define USE_GLUT
#endif



////////////////////////////////////////////////////////////////////////
// Program arguments
////////////////////////////////////////////////////////////////////////

// Filename program variables

static char *input_scene_name = NULL;
static char *input_cameras_name = NULL;
static char *input_categories_name = NULL;
static char *output_image_directory = NULL;


// Image capture program variables

static int capture_color_images = 0;
static int capture_depth_images = 0;
static int capture_kinect_images = 0;
static int capture_height_images = 0;
static int capture_angle_images = 0;
static int capture_normal_images = 0;
static int capture_ndotv_images = 0;
static int capture_albedo_images = 0;
static int capture_brdf_images = 0;
static int capture_material_images = 0;
static int capture_node_images = 0;
static int capture_category_images = 0;
static int capture_boundary_images = 0;
static int capture_room_surface_images = 0;
static int capture_room_boundary_images = 0;


// Other parameter program variables

static int width = 640;
static int height = 480;
static RNAngle xfov = 0;
static RNRgb background(0,0,0);
static int headlight = 1;
static int glut = 1;
static int mesa = 0;


// Image-specific program variables

static RNScalar kinect_min_depth = 0.5;
static RNScalar kinect_max_depth = 7.0;
static RNScalar kinect_min_reflection = 0.05;
static RNScalar kinect_noise_fraction = 0.05;
static RNScalar kinect_stereo_baseline = 0.075;


// Informational program variables

static int print_verbose = 0;
static int print_debug = 0;



////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////

class Category {
public:
  Category(void) : name(NULL), identifier(-1) {};
  ~Category(void) { if (name) free(name); }
public:
  char *name;
  int identifier;
};


enum {
  NO_COLOR_SCHEME,
  RGB_COLOR_SCHEME,
  DEPTH_COLOR_SCHEME,
  HEIGHT_COLOR_SCHEME,
  ANGLE_COLOR_SCHEME,
  XNORMAL_COLOR_SCHEME,
  YNORMAL_COLOR_SCHEME,
  ZNORMAL_COLOR_SCHEME,
  NDOTV_COLOR_SCHEME,
  ALBEDO_COLOR_SCHEME,
  BRDF_COLOR_SCHEME,
  MATERIAL_COLOR_SCHEME,
  NODE_COLOR_SCHEME,
  CATEGORY_COLOR_SCHEME,
  ROOM_SURFACE_COLOR_SCHEME
};



////////////////////////////////////////////////////////////////////////
// Internal variables
////////////////////////////////////////////////////////////////////////

// State variables

static R3Scene *scene = NULL;
static RNArray<R3Camera *> cameras;
static RNArray<Category *> categories;
static Category **node_category_assignments = NULL;
static int next_image_index = 0;



////////////////////////////////////////////////////////////////////////
// Input/output functions
////////////////////////////////////////////////////////////////////////

static R3Scene *
ReadScene(char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate scene
  scene = new R3Scene();
  if (!scene) {
    fprintf(stderr, "Unable to allocate scene for %s\n", filename);
    return NULL;
  }

  // Read scene from file
  if (!scene->ReadFile(filename)) {
    delete scene;
    return NULL;
  }

  // Remove transformations (makes computing rendered values easier)
  scene->RemoveTransformations();
  
  // Print statistics
  if (print_verbose) {
    printf("Read scene from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Nodes = %d\n", scene->NNodes());
    printf("  # Materials = %d\n", scene->NMaterials());
    printf("  # Brdfs = %d\n", scene->NBrdfs());
    printf("  # Textures = %d\n", scene->NTextures());
    printf("  # Lights = %d\n", scene->NLights());
    fflush(stdout);
  }

  // Return scene
  return scene;
}



static int
ReadCameras(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int camera_count = 0;

  // Get useful variables
  RNScalar neardist = 0.01 * scene->BBox().DiagonalRadius();
  RNScalar fardist = 100 * scene->BBox().DiagonalRadius();
  RNScalar aspect = (RNScalar) height / (RNScalar) width;

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open cameras file %s\n", filename);
    return 0;
  }

  // Read file
  RNScalar vx, vy, vz, tx, ty, tz, ux, uy, uz, xf, yf, value;
  while (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &vx, &vy, &vz, &tx, &ty, &tz, &ux, &uy, &uz, &xf, &yf, &value) == (unsigned int) 12) {
    R3Point viewpoint(vx, vy, vz);
    R3Vector towards(tx, ty, tz);
    R3Vector up(ux, uy, uz);
    R3Vector right = towards % up;
    towards.Normalize();
    up = right % towards;
    up.Normalize();
    if (xfov > 0) xf = xfov;
    yf = atan(aspect * tan(xf));
    R3Camera *camera = new R3Camera(viewpoint, towards, up, xf, yf, neardist, fardist);
    camera->SetValue(value);
    cameras.Insert(camera);
    camera_count++;
  }

  // Close file
  fclose(fp);

  // Print statistics
  if (print_verbose) {
    printf("Read cameras from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Cameras = %d\n", camera_count);
    fflush(stdout);
  }

  // Return success
  return 1;
}



static int
ReadCategories(const char *filename)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open category file %s\n", filename);
    return 0;
  }

  // Read file
  char category_name[4096];
  while (fscanf(fp, "%s", category_name) == (unsigned int) 1) {
    // Create category
    Category *category = new Category();
    category->name = strdup(category_name);
    category->identifier = categories.NEntries();
    categories.Insert(category);
  }

  // Close file
  fclose(fp);

  // Print statistics
  if (print_verbose) {
    printf("Read categories from %s ...\n", filename);
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Categories = %d\n", categories.NEntries());
    fflush(stdout);
  }

  // Return success
  return 1;
} 




////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////

static Category **
AssignNodesToCategories(void)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();
  int assignment_count = 0;

  // Allocate array of category assignments
  Category **assignments = new Category * [ scene->NNodes() ];
  if (!assignments) {
    fprintf(stderr, "Unable to allocate array of category assignments\n");
    return NULL;
  }

  // Initialize category assignments
  for (int i = 0; i < scene->NNodes(); i++) assignments[i] = NULL;

  // Assign nodes to categories (by matching node_name)
  for (int i = 0; i < scene->NNodes(); i++) {
    R3SceneNode *node = scene->Node(i);
    const char *node_name = node->Name();
    if (!node_name) continue;
    int node_name_length = strlen(node_name);
    for (int j = 0; j < categories.NEntries(); j++) {
      Category *category = categories.Kth(j);
      int category_name_length = strlen(category->name);
      if (category_name_length == 0) continue;
      if (!strcmp(category->name, "Wall")) {
        if (strncmp(node_name, "Walls#", 5)) continue;
      }
      else if (!strcmp(category->name, "Floor")) {
        if (strncmp(node_name, "Floors#", 7)) continue;
      }
      else if (!strcmp(category->name, "Ceiling")) {
        if (strncmp(node_name, "Ceilings#", 9)) continue;
      }
      else {
        const char *node_category_name = NULL;
        const char *p = node_name + node_name_length;
        while (p-- > node_name+3) {
          if (isalnum(*(p-2)) && (*(p-1) == '_') && isalnum(*(p))) { node_category_name = p; break; }
        }
        if (!node_category_name) continue;
        if (strcmp(node_category_name, category->name)) continue;
      }
      assignments[i] = category;
      assignment_count++;
      break;
    }
  }

  // Print statistics
  if (print_verbose) {
    printf("Assigned nodes to categories ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Assignments = %d\n", assignment_count);
    fflush(stdout);
  }

  // Return array of category assignments
  return assignments;
} 



static RNScalar
EstimateGroundZ(const R3Camera& camera, R3Scene *scene)
{
  // Get convenient variables
  if (!scene) return 0;
  R3SceneNode *root = scene->Root();
  if (!root) return 0;

  // Check if scene is from P5D and has floors explicitly labeled
  if (root->Name() && !strncmp(root->Name(), "Project#", 8) && root->Data()) {
    // Search P5D project for floor containing camera
    P5DProject *project = (P5DProject *) root->Data();
    RNScalar floor_z = 0;
    for (int i = 0; i < project->NFloors(); i++) {
      P5DFloor *floor = project->Floor(i);
      if (floor_z + floor->h > camera.Origin().Z()) break;
      floor_z += floor->h;
    }
    return floor_z;
  }

#if 0
  // Check first intersection with ray cast straight down
  if (R3Contains(scene->BBox(), camera.Origin())) {
    R3Ray ray(camera.Origin(), R3negz_vector);
    R3Point position;
    R3Vector normal;
    if (scene->Intersects(ray, NULL, NULL, NULL, &position, &normal)) {
      if (normal.Z() > 0.9) return position.Z();
    }
  }
#endif
  
  // If all else fails, return bounding box zmin
  return scene->BBox().ZMin();
}



static int
ComputeBoundaryImage(const R2Grid& depth_image, const R2Grid& node_image,
  const R2Grid& xnormal_image, const R2Grid& ynormal_image, const R2Grid& znormal_image,
  R2Grid& result)
{                  
  // Find node boundaries
  R2Grid node_boundaries(node_image);
  node_boundaries.GradientMagnitude();
  node_boundaries.Threshold(RN_EPSILON, 0, 1);

  // Find silhouette boundaries (where depth difference is > 10% of depth)
  R2Grid silhouette_boundaries(depth_image);
  silhouette_boundaries.Substitute(0, R2_GRID_UNKNOWN_VALUE);
  silhouette_boundaries.GradientMagnitude();
  silhouette_boundaries.Divide(depth_image);
  silhouette_boundaries.Threshold(0.1, 0, 1);
  silhouette_boundaries.Substitute(R2_GRID_UNKNOWN_VALUE, 0);

  // Find crease boundaries
  R2Grid crease_boundaries(width, height);
  R2Grid xcrease_boundaries(xnormal_image);
  xcrease_boundaries.GradientMagnitude();
  R2Grid ycrease_boundaries(ynormal_image);
  ycrease_boundaries.GradientMagnitude();
  R2Grid zcrease_boundaries(znormal_image);
  zcrease_boundaries.GradientMagnitude();
  crease_boundaries.Add(xcrease_boundaries);
  crease_boundaries.Add(ycrease_boundaries);
  crease_boundaries.Add(zcrease_boundaries);
  crease_boundaries.Threshold(1, 0, 1);
  R2Grid silhouette_mask(silhouette_boundaries);
  silhouette_mask.Threshold(0.5, 1, 0);
  crease_boundaries.Mask(silhouette_mask);
    
  // Combine boundaries into one image
  result = R2Grid(width, height);
  node_boundaries.Multiply(2);
  result.Add(node_boundaries);
  silhouette_boundaries.Multiply(4);
  result.Add(silhouette_boundaries);
  crease_boundaries.Multiply(8);
  result.Add(crease_boundaries);

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Image capture functions
////////////////////////////////////////////////////////////////////////

static int
CaptureColor(R2Image& image)
{
  // Capture image 
  image.Capture();

  // Return success
  return 1;
}



static void
LoadInteger(int value)
{
  // Set color to represent an integer (24 bits)
  unsigned char color[4];
  color[0] = (value >> 16) & 0xFF;
  color[1] = (value >>  8) & 0xFF;
  color[2] = (value      ) & 0xFF;
  glColor3ubv(color);
}



static int
CaptureInteger(R2Grid& image)
{
  // Allocate pixel buffer
  unsigned char *pixels = new unsigned char [ 3 * width * height ];

  // Read pixels
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

  // Fill image
  unsigned char *pixelp = pixels;
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      unsigned int red = *pixelp++;
      unsigned int green = *pixelp++;
      unsigned int blue = *pixelp++;
      unsigned int value = 0;
      value |= red << 16;
      value |= green <<  8;
      value |= blue;
      image.SetGridValue(ix, iy, value);
    }
  }

  // Delete pixels
  delete [] pixels;
  
  // Return success
  return 1;
}



static void
LoadScalar(RNScalar value, RNScalar max_value = 65535)
{
  // Set color to represent an scalar value
  if (value > max_value) value = max_value;
  glColor3d(0, 0, value / max_value);
}



static int
CaptureScalar(R2Grid& image, RNScalar max_value = 65535)
{
  // Allocate pixel buffer
  float *pixels = new float [ width * height ];

  // Read blue channel
  glReadPixels(0, 0, width, height, GL_BLUE, GL_FLOAT, pixels);
  
  // Fill image
  float *pixelp = pixels;
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      RNScalar value = *pixelp++;
      image.SetGridValue(ix, iy, max_value * value);
    }
  }

  // Delete pixels
  delete [] pixels;
  
  // Return success
  return 1;
}



static int 
CaptureDepth(R2Grid& image)
{
  // Get viewport dimensions
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  // Get modelview  matrix
  static GLdouble modelview_matrix[16];
  // glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
  for (int i = 0; i < 16; i++) modelview_matrix[i] = 0;
  modelview_matrix[0] = 1.0;
  modelview_matrix[5] = 1.0;
  modelview_matrix[10] = 1.0;
  modelview_matrix[15] = 1.0;
  
  // Get projection matrix
  GLdouble projection_matrix[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);

  // Get viewpoint matrix
  GLint viewport_matrix[16];
  glGetIntegerv(GL_VIEWPORT, viewport_matrix);

  // Allocate pixels
  float *pixels = new float [ image.NEntries() ];

  // Read pixels from frame buffer 
  glReadPixels(0, 0, viewport[2], viewport[3], GL_DEPTH_COMPONENT, GL_FLOAT, pixels); 

  // Resize image
  image.Clear(0.0);
  
  // Convert pixels to depths
  int ix, iy;
  double x, y, z;
  for (int i = 0; i < image.NEntries(); i++) {
    if (RNIsEqual(pixels[i], 1.0)) continue;
    if (RNIsNegativeOrZero(pixels[i])) continue;
    image.IndexToIndices(i, ix, iy);
    gluUnProject(ix, iy, pixels[i], modelview_matrix, projection_matrix, viewport_matrix, &x, &y, &z);
    image.SetGridValue(i, -z);
  }

  // Delete pixels
  delete [] pixels;

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Draw functions
////////////////////////////////////////////////////////////////////////

static void 
DrawNodeWithOpenGL(const R3Camera& camera, R3Scene *scene, R3SceneNode *node, int color_scheme, RNBoolean roomfiles_only = FALSE)
{
  // Check if roomfiles_only
  if (roomfiles_only) {
    if (node->NChildren() == 0) {
      if (!node->Name() || 
          (strncmp(node->Name(), "Walls#", 6) &&
           strncmp(node->Name(), "Floors#", 7) &&
           strncmp(node->Name(), "Ceilings#", 9))) {
        return;
      }
    }
  }
  
  // Draw elements
  if ((color_scheme == RGB_COLOR_SCHEME) || (color_scheme == ALBEDO_COLOR_SCHEME)) {
    // Draw colors
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      element->Draw(R3_DEFAULT_DRAW_FLAGS);
    }
  }
  else if ((color_scheme == NODE_COLOR_SCHEME) || (color_scheme == CATEGORY_COLOR_SCHEME) || (color_scheme == ROOM_SURFACE_COLOR_SCHEME)) {
    // Draw integer values per node
    if (color_scheme == NODE_COLOR_SCHEME) {
      LoadInteger(node->SceneIndex() + 1);
    }
    else if (color_scheme == CATEGORY_COLOR_SCHEME) {
      Category *category = (node_category_assignments) ? node_category_assignments[node->SceneIndex()] : NULL;
      int category_identifier = (category) ? category->identifier : 0;
      LoadInteger(category_identifier);
    }
    else if (color_scheme == ROOM_SURFACE_COLOR_SCHEME) {
      if (!node->Name()) LoadInteger(0);
      else if (!strncmp(node->Name(), "Walls#", 6)) LoadInteger(1);
      else if (!strncmp(node->Name(), "Floors#", 7)) LoadInteger(2);
      else if (!strncmp(node->Name(), "Ceilings#", 9)) LoadInteger(3);
      else LoadInteger(0);
    }
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      element->Draw(R3_SURFACES_DRAW_FLAG);
    }
  }
  else if (color_scheme == MATERIAL_COLOR_SCHEME) {
    // Draw integer values per element
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      R3Material *material = element->Material();
      if (!material) LoadInteger(0);
      else LoadInteger(material->SceneIndex() + 1);
      element->Draw(R3_SURFACES_DRAW_FLAG);
    }
  }
  else if (color_scheme == BRDF_COLOR_SCHEME) {
    // Draw color values per element (NOTE: THIS IGNORES TEXTURE)
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      const R3Material *material = element->Material();
      const R3Brdf *brdf = (material) ? material->Brdf() : NULL;
      if (!brdf) brdf = &R3default_brdf;
      RNScalar kd = brdf->Diffuse().Luminance();
      RNScalar ks = brdf->Specular().Luminance();
      RNScalar kt = brdf->Transmission().Luminance();
      RNLoadRgb(kd, ks, kt);
      element->Draw(R3_SURFACES_DRAW_FLAG);
    }
  }
  else if ((color_scheme == ANGLE_COLOR_SCHEME) || 
    (color_scheme == XNORMAL_COLOR_SCHEME) || (color_scheme == YNORMAL_COLOR_SCHEME) || (color_scheme == ZNORMAL_COLOR_SCHEME)) {
    // Draw integer values per triangle
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      for (int j = 0; j < element->NShapes(); j++) {
        R3Shape *shape = element->Shape(j);
        if (shape->ClassID() == R3TriangleArray::CLASS_ID()) {
          R3TriangleArray *triangles = (R3TriangleArray *) shape;
          for (int k = 0; k < triangles->NTriangles(); k++) {
            R3Triangle *triangle = triangles->Triangle(k);
            const R3Vector& normal = triangle->Normal();
            RNScalar value = 0;
            if (color_scheme == ANGLE_COLOR_SCHEME) value = (RN_PI - acos(normal.Z())) / RN_PI;
            else value = 0.5*normal[color_scheme - XNORMAL_COLOR_SCHEME] + 0.5;
            LoadInteger((int) (65535 * value));
            triangle->Draw(R3_SURFACES_DRAW_FLAG);
          }
        }
      }
    }
  }
  else if ((color_scheme == DEPTH_COLOR_SCHEME) || (color_scheme == NDOTV_COLOR_SCHEME) || (color_scheme == HEIGHT_COLOR_SCHEME)) {
    // Draw scalar values interpolated between triangle vertices
    RNScalar ground_z = (color_scheme == HEIGHT_COLOR_SCHEME) ? EstimateGroundZ(camera, scene) : 0;
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      for (int j = 0; j < element->NShapes(); j++) {
        R3Shape *shape = element->Shape(j);
        if (shape->ClassID() == R3TriangleArray::CLASS_ID()) {
          R3TriangleArray *triangles = (R3TriangleArray *) shape;
          for (int k = 0; k < triangles->NTriangles(); k++) {
            R3Triangle *triangle = triangles->Triangle(k);
            for (int m = 0; m < 3; m++) {
              R3TriangleVertex *vertex = triangle->Vertex(m);
              const R3Point& position = vertex->Position();
              if (color_scheme == HEIGHT_COLOR_SCHEME) LoadScalar(position.Z() - ground_z, 6.5535);
              else if (color_scheme == DEPTH_COLOR_SCHEME) LoadScalar((position - camera.Origin()).Dot(camera.Towards()), 6.5535);
              else if (color_scheme == NDOTV_COLOR_SCHEME) {
                R3Vector v = camera.Origin() - position; v.Normalize();
                R3Vector normal = (vertex->Flags()[R3_VERTEX_NORMALS_DRAW_FLAG]) ? vertex->Normal() : triangle->Normal();
                LoadScalar(fabs(normal.Dot(v)), 1.0);
              }
              R3LoadPoint(position);
            }
          }
        }
      }
    }
    glEnd();
  }
  else {
    // Draw without setting colors
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      element->Draw(R3_SURFACES_DRAW_FLAG);
    }
  }

  // Draw children
  for (int i = 0; i < node->NChildren(); i++) {
    R3SceneNode *child = node->Child(i);
    DrawNodeWithOpenGL(camera, scene, child, color_scheme, roomfiles_only);
  }
}



static int
DrawSceneWithOpenGL(const R3Camera& camera, R3Scene *scene, int color_scheme, RNBoolean roomfiles_only = FALSE)
{
  // Clear window
  glClearColor(background.R(), background.G(), background.B(), 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set up lighting
  if (color_scheme == RGB_COLOR_SCHEME) {
    // Load ambient light
    static GLfloat ambient[4];
    ambient[0] = scene->Ambient().R();
    ambient[1] = scene->Ambient().G();
    ambient[2] = scene->Ambient().B();
    ambient[3] = 1;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // Load scene lights
    for (int i = 0; i < scene->NLights(); i++) {
      R3Light *light = scene->Light(i);
      light->Draw(i);
    }

    // Draw scene
    glEnable(GL_LIGHTING);
    R3null_material.Draw();
    DrawNodeWithOpenGL(camera, scene, scene->Root(), color_scheme, roomfiles_only);
    R3null_material.Draw();
  }
  else if (color_scheme == ALBEDO_COLOR_SCHEME) {
    // Load ambient light
    static GLfloat ambient[4];
    ambient[0] = 1;
    ambient[1] = 1;
    ambient[2] = 1;
    ambient[3] = 1;
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

    // Setup lighting to be only diffuse
    glColor3d(1.0, 1.0, 1.0);
    glDisable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    R3null_material.Draw();
    DrawNodeWithOpenGL(camera, scene, scene->Root(), color_scheme, roomfiles_only);
    R3null_material.Draw();
    glEnable(GL_LIGHT0);
  }
  else {
    // Draw scene
    glDisable(GL_LIGHTING);
    glColor3d(1.0, 1.0, 1.0);
    R3null_material.Draw();
    DrawNodeWithOpenGL(camera, scene, scene->Root(), color_scheme, roomfiles_only);
    R3null_material.Draw();
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Rendering callback functions
////////////////////////////////////////////////////////////////////////

void Redraw(void)
{
  // Check scene
  if (!scene) return;

  // Statistics variables
  static RNTime start_time;
  if (next_image_index == 0) start_time.Read(); 

  // Check if we have captured all images
  if (next_image_index >= cameras.NEntries()) {
    if (print_verbose) {
      printf("  Time = %.2f seconds\n", start_time.Elapsed());
      printf("  # Images = %d\n", next_image_index);
      fflush(stdout);
    }
    exit(0);
  }

  // Get camera, name, and node for next image
  char name[1024];
  sprintf(name, "%06d", next_image_index); 
  R3Camera *camera = cameras.Kth(next_image_index);
  next_image_index++;

  // Initialize lighting
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  static GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glEnable(GL_NORMALIZE);

  // Initialize headlight
  if (headlight) {
    static GLfloat light0_diffuse[] = { 0.5, 0.5, 0.5, 1.0 };
    static GLfloat light0_position[] = { 0.0, 0.0, 1.0, 0.0 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    glEnable(GL_LIGHT0);
  }

  // Initialize graphics modes  
  // glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  // Load camera and viewport
  camera->Load();
  glViewport(0, 0, width, height);

  // Print debug message
  if (print_debug) {
    printf("  Rendering %s ...\n", name);
    fflush(stdout);
  }
  
  // Draw, capture, and write depth image 
  if (capture_depth_images) {
    if (DrawSceneWithOpenGL(*camera, scene, NO_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureDepth(image)) {
        image.Multiply(1000);
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_depth.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

#if 0
  // Draw, capture, and write depth image in a second way
  if (capture_depth_images) {
    if (DrawSceneWithOpenGL(*camera, scene, DEPTH_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureScalar(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_depth2.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }
#endif
  
  // Capture and write height image 
  if (capture_height_images) {
    if (DrawSceneWithOpenGL(*camera, scene, HEIGHT_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureScalar(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_height.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

  // Capture and write angle image 
  if (capture_angle_images) {
    if (DrawSceneWithOpenGL(*camera, scene, ANGLE_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureInteger(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_angle.pfm", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

  // Capture and write ndotv image 
  if (capture_ndotv_images) {
    if (DrawSceneWithOpenGL(*camera, scene, NDOTV_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureInteger(image)) {
        image.Multiply(65535.0/255.0);
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_ndotv.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

  // Draw, capture, and write albedo image 
  if (capture_albedo_images) {
    if (DrawSceneWithOpenGL(*camera, scene, ALBEDO_COLOR_SCHEME)) {
      R2Image image(width, height, 3);
      if (CaptureColor(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_albedo.jpg", output_image_directory, name);
        image.Write(output_image_filename);
      }
    }
  }

  // Draw, capture, and write brdf image 
  if (capture_brdf_images) {
    if (DrawSceneWithOpenGL(*camera, scene, BRDF_COLOR_SCHEME)) {
      R2Image image(width, height, 3);
      if (CaptureColor(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_brdf.jpg", output_image_directory, name);
        image.Write(output_image_filename);
      }
    }
  }

  // Capture and write material image 
  if (capture_material_images) {
    if (DrawSceneWithOpenGL(*camera, scene, MATERIAL_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureInteger(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_material.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

  // Capture and write node image 
  if (capture_node_images) {
    if (DrawSceneWithOpenGL(*camera, scene, NODE_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureInteger(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_node.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

  // Capture and write category image 
  if (capture_category_images) {
    if (DrawSceneWithOpenGL(*camera, scene, CATEGORY_COLOR_SCHEME)) {
      R2Grid image(width, height);
      if (CaptureInteger(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_category.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

  // Capture and write room_surface image 
  if (capture_room_surface_images) {
    if (DrawSceneWithOpenGL(*camera, scene, ROOM_SURFACE_COLOR_SCHEME, TRUE)) {
      R2Grid image(width, height);
      if (CaptureInteger(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_room_surface.png", output_image_directory, name);
        image.WriteFile(output_image_filename);
      }
    }
  }

  // Draw, capture, and write normal images
  if (capture_normal_images) {
    R2Grid grid(width, height);
    R2Image image(width, height);
    char output_image_filename[1024];
    sprintf(output_image_filename, "%s/%s_xnormal.png", output_image_directory, name);
    DrawSceneWithOpenGL(*camera, scene, XNORMAL_COLOR_SCHEME);
    if (!CaptureInteger(grid)) return;
    grid.WriteFile(output_image_filename);
    sprintf(output_image_filename, "%s/%s_ynormal.png", output_image_directory, name);
    DrawSceneWithOpenGL(*camera, scene, YNORMAL_COLOR_SCHEME);
    if (!CaptureInteger(grid)) return;
    grid.WriteFile(output_image_filename);
    sprintf(output_image_filename, "%s/%s_znormal.png", output_image_directory, name);
    DrawSceneWithOpenGL(*camera, scene, ZNORMAL_COLOR_SCHEME);
    if (!CaptureInteger(grid)) return;
    grid.WriteFile(output_image_filename);
  }
  
  // Capture and write boundary image 
  if (capture_boundary_images) {
    R2Grid image(width, height);
    R2Grid node_image(width, height);
    R2Grid depth_image(width, height);
    R2Grid xnormal_image(width, height), ynormal_image(width, height), znormal_image(width, height);
    DrawSceneWithOpenGL(*camera, scene, NODE_COLOR_SCHEME);
    CaptureInteger(node_image);
    CaptureDepth(depth_image);
    DrawSceneWithOpenGL(*camera, scene, XNORMAL_COLOR_SCHEME);
    CaptureInteger(xnormal_image); xnormal_image.Multiply(1.0/65535.0); xnormal_image.Subtract(0.5); xnormal_image.Multiply(2.0); 
    DrawSceneWithOpenGL(*camera, scene, YNORMAL_COLOR_SCHEME);
    CaptureInteger(ynormal_image); ynormal_image.Multiply(1.0/65535.0); ynormal_image.Subtract(0.5); ynormal_image.Multiply(2.0); 
    DrawSceneWithOpenGL(*camera, scene, ZNORMAL_COLOR_SCHEME);
    CaptureInteger(znormal_image); znormal_image.Multiply(1.0/65535.0); znormal_image.Subtract(0.5); znormal_image.Multiply(2.0); 
    if (ComputeBoundaryImage(depth_image, node_image, xnormal_image, ynormal_image, znormal_image, image)) {
      char output_image_filename[1024];
      sprintf(output_image_filename, "%s/%s_boundary.png", output_image_directory, name);
      image.WriteFile(output_image_filename);
    }
  }

  // Capture and write room boundary image 
  if (capture_room_boundary_images) {
    R2Grid image(width, height);
    R2Grid node_image(width, height);
    R2Grid depth_image(width, height);
    R2Grid xnormal_image(width, height), ynormal_image(width, height), znormal_image(width, height);
    DrawSceneWithOpenGL(*camera, scene, NODE_COLOR_SCHEME, TRUE);
    CaptureInteger(node_image);
    CaptureDepth(depth_image);
    DrawSceneWithOpenGL(*camera, scene, XNORMAL_COLOR_SCHEME, TRUE);
    CaptureInteger(xnormal_image); xnormal_image.Multiply(1.0/65535.0); xnormal_image.Subtract(0.5); xnormal_image.Multiply(2.0); 
    DrawSceneWithOpenGL(*camera, scene, YNORMAL_COLOR_SCHEME, TRUE);
    CaptureInteger(ynormal_image); ynormal_image.Multiply(1.0/65535.0); ynormal_image.Subtract(0.5); ynormal_image.Multiply(2.0); 
    DrawSceneWithOpenGL(*camera, scene, ZNORMAL_COLOR_SCHEME, TRUE);
    CaptureInteger(znormal_image); znormal_image.Multiply(1.0/65535.0); znormal_image.Subtract(0.5); znormal_image.Multiply(2.0); 
    if (ComputeBoundaryImage(depth_image, node_image, xnormal_image, ynormal_image, znormal_image, image)) {
      char output_image_filename[1024];
      sprintf(output_image_filename, "%s/%s_room_boundary.png", output_image_directory, name);
      image.WriteFile(output_image_filename);
    }
  }

  // Draw, capture, and write simulated kinect depth image 
  if (capture_kinect_images) {
    // Capture depth and ndotv
    R2Grid depth_image(width, height);
    R2Grid ndotv_image(width, height);
    DrawSceneWithOpenGL(*camera, scene, NDOTV_COLOR_SCHEME);
    if (!CaptureDepth(depth_image)) return;
    if (!CaptureInteger(ndotv_image)) return;
    ndotv_image.Multiply(1.0/255.0);

    // Add noise
    depth_image.AddNoise(kinect_noise_fraction);
    ndotv_image.AddNoise(kinect_noise_fraction);
    
    // Capture brdf information
    R2Grid material_image(width, height);
    DrawSceneWithOpenGL(*camera, scene, MATERIAL_COLOR_SCHEME);
    if (!CaptureInteger(material_image)) return;

    // Get convenient variables for stereo baseline checks
    double ixc = 0.5 * width;  // x coordinate on center of image in image coordinates
    double ixr = 0.5 * width;  // x coordinate on right side of image in image coordinates
    double vxr = tan(camera->XFOV()); // x coordinate on right side of image on view plane at d=1m in camera coordinates
    
    // Create kinect image
    R2Grid kinect_image(width, height);
    for (int ix = 0; ix < width; ix++) {
      for (int iy = 0; iy < height; iy++) {
        // Get/check depth
        RNScalar depth = depth_image.GridValue(ix, iy);
        if (depth == 0) continue;
        if ((kinect_min_depth > 0) && (depth < kinect_min_depth)) continue;
        if ((kinect_max_depth > 0) && (depth > kinect_max_depth)) continue;

        // Get/check material
        if (kinect_min_reflection > 0) {
          // Get/check angle
          RNScalar ndotv = ndotv_image.GridValue(ix, iy);
          if (ndotv < kinect_min_reflection) continue; 

          // Get/check material
          RNScalar material_index_value = material_image.GridValue(ix, iy);
          int material_index = (int) (material_index_value - 1.0 + 0.5);
          if (material_index < 0) continue;
          if (material_index >= scene->NMaterials()) continue; 
          const R3Material *material = scene->Material(material_index);
          const R3Brdf *brdf = material->Brdf();
          if (!brdf) continue;
          RNScalar kd = brdf->Diffuse().Luminance();
          RNScalar ks = brdf->Specular().Luminance();
          RNScalar kt = brdf->Transmission().Luminance();
          if (kd < 0.05) kd = 0.05; // this is a hack to compensate for black kd in materials
          RNScalar sum = kd + ks + kt;
          if (RNIsNegativeOrZero(sum)) continue;
          if (sum > 1) kd = 1.0 - ks - kt;  // this is a hack to compensate for nonphysical BRDFs

          // Get/check reflection of light back to camera
          RNScalar reflection = kd / sum;  // this is a hack to compensate for bad kd in materials
          if (reflection * ndotv < kinect_min_reflection) continue;
        }


        // Set depth value
        kinect_image.SetGridValue(ix, iy, depth);

        // Check whether projection of point towards projector camera (baseline to the right) is occluded
        if (kinect_stereo_baseline > 0) {
          double x = depth * vxr * (ix - ixc) / ixr; // x coordinate in camera coordinates
          R2Halfspace h(R2Point(x, depth), R2Point(kinect_stereo_baseline, 0));
          for (int ix2 = ix+1; ix2 < width; ix2++) {
            RNScalar depth2 = depth_image.GridValue(ix2, iy);
            if (depth2 > 0) {
              double x2 = depth2 * vxr * (ix2 - ixc) / ixr; 
              if (R2Contains(h, R2Point(x2, depth2))) {
                kinect_image.SetGridValue(ix, iy, 0);
                break;
              }
            }
          }
        }
      }
    }

    // Write kinect image
    kinect_image.Multiply(1000);
    char output_image_filename[1024];
    sprintf(output_image_filename, "%s/%s_kinect.png", output_image_directory, name);
    kinect_image.WriteFile(output_image_filename);
  }

  // Draw, capture, and write color image 
  if (capture_color_images) {
    if (DrawSceneWithOpenGL(*camera, scene, RGB_COLOR_SCHEME)) {
      R2Image image(width, height, 3);
      if (CaptureColor(image)) {
        char output_image_filename[1024];
        sprintf(output_image_filename, "%s/%s_color.jpg", output_image_directory, name);
        image.Write(output_image_filename);
      }
    }
  }

#ifdef USE_GLUT
  // Redraw
  glutPostRedisplay();
#endif
}



static int
RenderImagesWithGlut(const char *output_image_directory)
{
#ifdef USE_GLUT
  // Print message
  if (print_verbose) {
    printf("Rendering images with glut to %s\n", output_image_directory);
    fflush(stdout);
  }

  // Create output directory
  char cmd[1024];
  sprintf(cmd, "mkdir -p %s", output_image_directory);
  system(cmd);

  // Open window
  int argc = 1;
  char *argv[1];
  argv[0] = strdup("scn2img");
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(width, height);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH); 
  glutCreateWindow("Scene Image Capture");

  // Initialize GLUT callback functions 
  glutDisplayFunc(Redraw);

  // Run main loop  -- never returns 
  glutMainLoop();

  // Return success -- actually never gets here
  return 1;
#else
  // Not supported
  RNAbort("Program was not compiled with glut.  Recompile with make.\n");
  return 0;
#endif
}



static int
RenderImagesWithMesa(const char *output_image_directory)
{
#ifdef USE_MESA
  // Print message
  if (print_verbose) {
    printf("Rendering images with mesa to %s\n", output_image_directory);
    fflush(stdout);
  }

  // Create output directory
  char cmd[1024];
  sprintf(cmd, "mkdir -p %s", output_image_directory);
  system(cmd);

  // Create mesa context
  OSMesaContext ctx = OSMesaCreateContextExt(OSMESA_RGBA, 32, 0, 0, NULL);
  if (!ctx) {
    fprintf(stderr, "Unable to create mesa context\n");
    return 0;
  }

  // Create frame buffer
  void *frame_buffer = malloc(width * height * 4 * sizeof(GLubyte) );
  if (!frame_buffer) {
    fprintf(stderr, "Unable to allocate mesa frame buffer\n");
    return 0;
  }

  // Assign mesa context
  if (!OSMesaMakeCurrent(ctx, frame_buffer, GL_UNSIGNED_BYTE, width, height)) {
    fprintf(stderr, "Unable to make mesa context current\n");
    return 0;
  }

  // Draw images
  while (TRUE) Redraw();
  
  // Delete mesa context
  OSMesaDestroyContext(ctx);

  // Delete frame buffer
  free(frame_buffer);

  // Return success
  return 1;
#else
  // Not supported
  RNAbort("Program was not compiled with mesa.  Recompile with make mesa.\n");
  return 0;
#endif
}



////////////////////////////////////////////////////////////////////////
// Raycasting
////////////////////////////////////////////////////////////////////////

static int
RenderImagesWithRaycasting(const R3Camera& camera, R3Scene *scene, const char *output_image_directory, int image_index)
{
  // Print debug message
  if (print_debug) {
    printf("  Raycasting %06d ...\n", image_index);
    fflush(stdout);
  }
 
  // Some useful variables
  RNScalar ground_z = EstimateGroundZ(camera, scene);
  R2Viewport viewport(0, 0, width, height);
  R3Viewer viewer(camera, viewport);
  char output_image_filename[1024];
  R3SceneNode *node = NULL;
  R3SceneElement *element = NULL;
  R3Shape *shape = NULL;
  R3Point position;
  R3Vector normal;
  RNScalar t;

  // Create images
  R2Grid depth_image(width, height);
  R2Grid height_image(width, height);
  R2Grid angle_image(width, height);
  R2Grid xnormal_image(width, height);
  R2Grid ynormal_image(width, height);
  R2Grid znormal_image(width, height);
  R2Grid ndotv_image(width, height);
  R2Image albedo_image(width, height, 3);
  R2Image brdf_image(width, height, 3);
  R2Grid material_image(width, height);
  R2Grid node_image(width, height);
  R2Grid category_image(width, height);
  
  // Cast ray for every pixel
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      R3Ray ray = viewer.WorldRay(ix, iy);
      if (scene->Intersects(ray, &node, &element, &shape, &position, &normal, &t)) {
        if (capture_depth_images) {
          RNScalar depth = (position - camera.Origin()).Dot(camera.Towards());
          depth_image.SetGridValue(ix, iy, 1000 * depth);
        }
        if (capture_height_images) {
          RNScalar height = position.Z() - ground_z;
          height_image.SetGridValue(ix, iy, 1000.0 * height);
        }
        if (capture_angle_images) {
          RNScalar value = (RN_PI - acos(normal.Z())) / RN_PI;
          angle_image.SetGridValue(ix, iy, 65535.0 * value);
        }
        if (capture_normal_images) {
          RNScalar xvalue = 0.5*normal.X() + 0.5;
          RNScalar yvalue = 0.5*normal.Y() + 0.5;
          RNScalar zvalue = 0.5*normal.Z() + 0.5;
          xnormal_image.SetGridValue(ix, iy, 65535.0 * xvalue);
          ynormal_image.SetGridValue(ix, iy, 65535.0 * yvalue);
          znormal_image.SetGridValue(ix, iy, 65535.0 * zvalue);
        }
        if (capture_ndotv_images) {
          RNScalar ndotv = fabs(normal.Dot(camera.Towards()));
          xnormal_image.SetGridValue(ix, iy, 65535.0 * ndotv);
        }
        if (capture_brdf_images) {
          const R3Material *material = element->Material();
          const R3Brdf *brdf = (material) ? material->Brdf() : NULL;
          if (!brdf) brdf = &R3default_brdf;
          RNScalar kd = brdf->Diffuse().Luminance();
          RNScalar ks = brdf->Specular().Luminance();
          RNScalar kt = brdf->Transmission().Luminance();
          brdf_image.SetPixelRGB(ix, iy, RNRgb(kd, ks, kt));
        }
        if (capture_material_images) {
          const R3Material *material = element->Material();
          int material_index = (material) ? material->SceneIndex() + 1 : 0;
          material_image.SetGridValue(ix, iy, material_index);
        }
        if (capture_node_images) {
          int node_index = node->SceneIndex() + 1;
          node_image.SetGridValue(ix, iy, node_index);
        }
        if (capture_category_images && node_category_assignments) {
          Category *category = (node_category_assignments) ? node_category_assignments[node->SceneIndex()] : NULL;
          int category_identifier = (category) ? category->identifier : 64535;
          category_image.SetGridValue(ix, iy, category_identifier);
        }
      }
    }
  }
  
  // Write images
  if (capture_depth_images) {
    sprintf(output_image_filename, "%s/%06d_depth.png", output_image_directory, image_index);
    depth_image.WriteFile(output_image_filename);
  }
  if (capture_height_images) {
    sprintf(output_image_filename, "%s/%06d_height.png", output_image_directory, image_index);
    height_image.WriteFile(output_image_filename);
  }
  if (capture_angle_images) {
    sprintf(output_image_filename, "%s/%06d_angle.png", output_image_directory, image_index);
    angle_image.WriteFile(output_image_filename);
  }
  if (capture_normal_images) {
    sprintf(output_image_filename, "%s/%06d_xnormal.png", output_image_directory, image_index);
    xnormal_image.WriteFile(output_image_filename);
    sprintf(output_image_filename, "%s/%06d_ynormal.png", output_image_directory, image_index);
    ynormal_image.WriteFile(output_image_filename);
    sprintf(output_image_filename, "%s/%06d_znormal.png", output_image_directory, image_index);
    znormal_image.WriteFile(output_image_filename);
  }
  if (capture_ndotv_images) {
    sprintf(output_image_filename, "%s/%06d_ndotv.png", output_image_directory, image_index);
    ndotv_image.WriteFile(output_image_filename);
  }
  if (capture_brdf_images) {
    sprintf(output_image_filename, "%s/%06d_brdf.jpg", output_image_directory, image_index);
    brdf_image.Write(output_image_filename);
  }
  if (capture_material_images) {
    sprintf(output_image_filename, "%s/%06d_material.png", output_image_directory, image_index);
    material_image.WriteFile(output_image_filename);
  }
  if (capture_node_images) {
    sprintf(output_image_filename, "%s/%06d_node.png", output_image_directory, image_index);
    node_image.WriteFile(output_image_filename);
  }
  if (capture_category_images && node_category_assignments) {
    sprintf(output_image_filename, "%s/%06d_category.png", output_image_directory, image_index);
    node_image.WriteFile(output_image_filename);
  }

  // Return success
  return 1;
}



static int
RenderImagesWithRaycasting(const char *output_image_directory)
{
  // Statistics variables
  static RNTime start_time;
  start_time.Read(); 
  if (print_verbose) {
    printf("Rendering images with RAYCASTING to %s\n", output_image_directory);
    fflush(stdout);
  }
  
  // Create output directory
  char cmd[1024];
  sprintf(cmd, "mkdir -p %s", output_image_directory);
  system(cmd);

  // Raycast images for every camera
  for (int i = 0; i < cameras.NEntries(); i++) {
    R3Camera *camera = cameras.Kth(i);
    if (!RenderImagesWithRaycasting(*camera, scene, output_image_directory, i)) return 0;
  }

  // Print message
  if (print_verbose) {
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Images = %d\n", cameras.NEntries());
    fflush(stdout);
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// Program argument parsing
////////////////////////////////////////////////////////////////////////

static int 
ParseArgs(int argc, char **argv)
{
  // Initialize variables to track whether to assign defaults
  int capture_images = 0;
  
  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) print_verbose = 1;
      else if (!strcmp(*argv, "-debug")) print_debug = 1;
      else if (!strcmp(*argv, "-glut")) { mesa = 0; glut = 1; }
      else if (!strcmp(*argv, "-mesa")) { mesa = 1; glut = 0; }
      else if (!strcmp(*argv, "-raycast")) { mesa = 0; glut = 0; }
      else if (!strcmp(*argv, "-capture_color_images")) { capture_images = capture_color_images = 1; }
      else if (!strcmp(*argv, "-capture_depth_images")) { capture_images = capture_depth_images = 1; }
      else if (!strcmp(*argv, "-capture_kinect_images")) { capture_images = capture_kinect_images = 1; }
      else if (!strcmp(*argv, "-capture_height_images")) { capture_images = capture_height_images = 1; }
      else if (!strcmp(*argv, "-capture_angle_images")) { capture_images = capture_angle_images = 1; }
      else if (!strcmp(*argv, "-capture_normal_images")) { capture_images = capture_normal_images = 1; }
      else if (!strcmp(*argv, "-capture_ndotv_images")) { capture_images = capture_ndotv_images = 1; }
      else if (!strcmp(*argv, "-capture_albedo_images")) { capture_images = capture_albedo_images = 1; }
      else if (!strcmp(*argv, "-capture_brdf_images")) { capture_images = capture_brdf_images = 1; }
      else if (!strcmp(*argv, "-capture_material_images")) { capture_images = capture_material_images = 1; }
      else if (!strcmp(*argv, "-capture_node_images")) { capture_images = capture_node_images = 1; }
      else if (!strcmp(*argv, "-capture_instance_images")) { capture_images = capture_node_images = 1; }
      else if (!strcmp(*argv, "-capture_category_images")) { capture_images = capture_category_images = 1; }
      else if (!strcmp(*argv, "-capture_boundary_images")) { capture_images = capture_boundary_images = 1; }
      else if (!strcmp(*argv, "-capture_room_surface_images")) { capture_images = capture_room_surface_images = 1; }
      else if (!strcmp(*argv, "-capture_room_boundary_images")) { capture_images = capture_room_boundary_images = 1; }
      else if (!strcmp(*argv, "-categories")) { argc--; argv++; input_categories_name = *argv; capture_category_images = 1; }
      else if (!strcmp(*argv, "-kinect_min_depth")) { argc--; argv++; kinect_min_depth = atof(*argv); }
      else if (!strcmp(*argv, "-kinect_max_depth")) { argc--; argv++; kinect_max_depth = atof(*argv); }
      else if (!strcmp(*argv, "-kinect_min_reflection")) { argc--; argv++; kinect_min_reflection = atof(*argv); }
      else if (!strcmp(*argv, "-kinect_noise_fraction")) { argc--; argv++; kinect_noise_fraction = atof(*argv); }
      else if (!strcmp(*argv, "-kinect_stereo_baseline")) { argc--; argv++; kinect_stereo_baseline = atof(*argv); }
      else if (!strcmp(*argv, "-width")) { argc--; argv++; width = atoi(*argv); }
      else if (!strcmp(*argv, "-height")) { argc--; argv++; height = atoi(*argv); }
      else if (!strcmp(*argv, "-xfov")) { argc--; argv++; xfov = atof(*argv); }
      else if (!strcmp(*argv, "-no_headlight")) { headlight = 0; }
      else if (!strcmp(*argv, "-headlight")) { headlight = 1; }
      else if (!strcmp(*argv, "-background")) {
        argc--; argv++; background[0] = atof(*argv);
        argc--; argv++; background[1] = atof(*argv);
        argc--; argv++; background[2] = atof(*argv);
      }
      else {
        fprintf(stderr, "Invalid program argument: %s", *argv);
        exit(1);
      }
      argv++; argc--;
    }
    else {
      if (!input_scene_name) input_scene_name = *argv;
      else if (!input_cameras_name) input_cameras_name = *argv;
      else if (!output_image_directory) output_image_directory = *argv;
      else { fprintf(stderr, "Invalid program argument: %s", *argv); exit(1); }
      argv++; argc--;
    }
  }

  // Set default capture options
  if (!capture_images) {
    capture_color_images = 1;
    capture_depth_images = 1;
    capture_kinect_images = 1;
    capture_normal_images = 1;
    capture_ndotv_images = 1;
    capture_albedo_images = 1;
    capture_brdf_images = 1;
    capture_material_images = 1;
    capture_node_images = 1;
    capture_category_images = 1;
    capture_boundary_images = 1;
    capture_room_surface_images = 1;
    capture_room_boundary_images = 1;
  }

  // Check filenames
  if (!input_scene_name || !input_cameras_name || !output_image_directory) {
    fprintf(stderr, "Usage: scn2img inputscenefile inputcamerasfile outputimagedirectory\n");
    return 0;
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

  // Read scene
  if (!ReadScene(input_scene_name)) exit(-1);

  // Read cameras 
  if (!ReadCameras(input_cameras_name)) exit(-1);

  // Read and assign categories
  if (input_categories_name) {
    if (!ReadCategories(input_categories_name)) exit(-1);
    node_category_assignments = AssignNodesToCategories();
    if (!node_category_assignments) exit(-1);
  }

  // Render images
  if (glut) { if (!RenderImagesWithGlut(output_image_directory)) exit(-1); }
  else if (mesa) { if (!RenderImagesWithMesa(output_image_directory)) exit(-1); }
  else { if (!RenderImagesWithRaycasting(output_image_directory)) exit(-1); }

  // Return success 
  return 0;
}

















