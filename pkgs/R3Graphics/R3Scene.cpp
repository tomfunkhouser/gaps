/* Source file for the R3 scene class */



/* Include files */

#include "R3Graphics.h"



/* Member functions */

int 
R3InitScene()
{
  /* Return success */
  return TRUE;
}



void 
R3StopScene()
{
}



R3Scene::
R3Scene(void)
  : nodes(),
    lights(),
    materials(),
    brdfs(),
    textures(),
    ambient(0, 0, 0),
    background(0, 0, 0)
{
  // Create root node
  root = new R3SceneNode(this);

  // Initialize viewer
  viewer.SetCamera(R3default_camera);
  viewer.SetViewport(R2default_viewport);
}



R3Scene::
~R3Scene(void)
{
  // Delete everything
  // ???
}



R3SceneNode *R3Scene::
Node(const char *name) const
{
  // Search nodes for matching name
  for (int i = 0; i < NNodes(); i++) {
    R3SceneNode *node = Node(i);
    if (!node->Name()) continue;
    if (!strcmp(node->Name(), name)) return node;
  }

  // Node not found
  return NULL;
}



R3Material *R3Scene::
Material(const char *name) const
{
  // Search materials for matching name
  for (int i = 0; i < NMaterials(); i++) {
    R3Material *material = Material(i);
    if (!material->Name()) continue;
    if (!strcmp(material->Name(), name)) return material;
  }

  // Material not found
  return NULL;
}



R3Light *R3Scene::
Light(const char *name) const
{
  // Search lights for matching name
  for (int i = 0; i < NLights(); i++) {
    R3Light *light = Light(i);
    if (!light->Name()) continue;
    if (!strcmp(light->Name(), name)) return light;
  }

  // Light not found
  return NULL;
}



R3Brdf *R3Scene::
Brdf(const char *name) const
{
  // Search brdfs for matching name
  for (int i = 0; i < NBrdfs(); i++) {
    R3Brdf *brdf = Brdf(i);
    if (!brdf->Name()) continue;
    if (!strcmp(brdf->Name(), name)) return brdf;
  }

  // Brdf not found
  return NULL;
}



R2Texture *R3Scene::
Texture(const char *name) const
{
  // Search textures for matching name
  for (int i = 0; i < NTextures(); i++) {
    R2Texture *texture = Texture(i);
    if (!texture->Name()) continue;
    if (!strcmp(texture->Name(), name)) return texture;
  }

  // Texture not found
  return NULL;
}



void R3Scene::
InsertNode(R3SceneNode *node) 
{
  // Insert node
  assert(!node->scene);
  assert(node->scene_index == -1);
  node->scene = this;
  node->scene_index = nodes.NEntries();
  nodes.Insert(node);
}



void R3Scene::
RemoveNode(R3SceneNode *node) 
{
  // Remove node
  assert(node->scene == this);
  assert(node->scene_index >= 0);
  RNArrayEntry *entry = nodes.KthEntry(node->scene_index);
  assert(nodes.EntryContents(entry) == node);
  R3SceneNode *tail = nodes.Tail();
  nodes.EntryContents(entry) = tail;
  tail->scene_index = node->scene_index;
  nodes.RemoveTail();
  node->scene_index = -1;
  node->scene = NULL;
}



void R3Scene::
InsertLight(R3Light *light) 
{
  // Insert light
  assert(!light->scene);
  assert(light->scene_index == -1);
  light->scene = this;
  light->scene_index = lights.NEntries();
  lights.Insert(light);
}



void R3Scene::
RemoveLight(R3Light *light) 
{
  // Remove light
  assert(light->scene == this);
  assert(light->scene_index >= 0);
  RNArrayEntry *entry = lights.KthEntry(light->scene_index);
  assert(lights.EntryContents(entry) == light);
  R3Light *tail = lights.Tail();
  lights.EntryContents(entry) = tail;
  tail->scene_index = light->scene_index;
  lights.RemoveTail();
  light->scene_index = -1;
  light->scene = NULL;
}



void R3Scene::
InsertMaterial(R3Material *material) 
{
  // Insert material
  assert(!material->scene);
  assert(material->scene_index == -1);
  material->scene = this;
  material->scene_index = materials.NEntries();
  materials.Insert(material);
}



void R3Scene::
RemoveMaterial(R3Material *material) 
{
  // Remove material
  assert(material->scene == this);
  assert(material->scene_index >= 0);
  RNArrayEntry *entry = materials.KthEntry(material->scene_index);
  assert(materials.EntryContents(entry) == material);
  R3Material *tail = materials.Tail();
  materials.EntryContents(entry) = tail;
  tail->scene_index = material->scene_index;
  materials.RemoveTail();
  material->scene_index = -1;
  material->scene = NULL;
}



void R3Scene::
InsertBrdf(R3Brdf *brdf) 
{
  // Insert brdf
  assert(!brdf->scene);
  assert(brdf->scene_index == -1);
  brdf->scene = this;
  brdf->scene_index = brdfs.NEntries();
  brdfs.Insert(brdf);
}



void R3Scene::
RemoveBrdf(R3Brdf *brdf) 
{
  // Remove brdf
  assert(brdf->scene == this);
  assert(brdf->scene_index >= 0);
  RNArrayEntry *entry = brdfs.KthEntry(brdf->scene_index);
  assert(brdfs.EntryContents(entry) == brdf);
  R3Brdf *tail = brdfs.Tail();
  brdfs.EntryContents(entry) = tail;
  tail->scene_index = brdf->scene_index;
  brdfs.RemoveTail();
  brdf->scene_index = -1;
  brdf->scene = NULL;
}



void R3Scene::
InsertTexture(R2Texture *texture) 
{
  // Insert texture
  assert(!texture->scene);
  assert(texture->scene_index == -1);
  texture->scene = this;
  texture->scene_index = textures.NEntries();
  textures.Insert(texture);
}



void R3Scene::
RemoveTexture(R2Texture *texture) 
{
  // Remove texture
  assert(texture->scene == this);
  assert(texture->scene_index >= 0);
  RNArrayEntry *entry = textures.KthEntry(texture->scene_index);
  assert(textures.EntryContents(entry) == texture);
  R2Texture *tail = textures.Tail();
  textures.EntryContents(entry) = tail;
  tail->scene_index = texture->scene_index;
  textures.RemoveTail();
  texture->scene_index = -1;
  texture->scene = NULL;
}



void R3Scene::
SetCamera(const R3Camera& camera) 
{
  // Remember camera
  viewer.SetCamera(camera);
}



void R3Scene::
SetViewport(const R2Viewport& viewport) 
{
  // Remember viewport
  viewer.SetViewport(viewport);
}



void R3Scene::
SetViewer(const R3Viewer& viewer) 
{
  // Remember viewer
  this->viewer = viewer;
}



static void
R3SceneRemoveHierarchy(R3Scene *scene, R3SceneNode *node, const R3Affine& parent_transformation)
{
  // Compute transformation
  R3Affine transformation = R3identity_affine;
  transformation.Transform(parent_transformation);
  transformation.Transform(node->Transformation());

  // Recurse to children
  for (int i = 0; i < node->NChildren(); i++) {
    R3SceneNode *child = node->Child(i);
    R3SceneRemoveHierarchy(scene, child, transformation);
  }

  // Check if should move node to root
  R3SceneNode *root = scene->Root();
  if (node != root) {
    R3SceneNode *parent = node->Parent();
    if (parent && (parent != root) && (node->NElements() > 0)) {
      // Assign transformation
      node->SetTransformation(transformation);

      // Move node to be child of root
      parent->RemoveChild(node);
      root->InsertChild(node);
    }
    else {
      // Delete node
      assert(node->NChildren() == 0);
      parent->RemoveChild(node);
      delete node;
    }
  }
}



void R3Scene::
RemoveHierarchy(void)
{
  // Maintain topology of scene, but set all node transformations to identity
  R3SceneRemoveHierarchy(this, root, R3identity_affine);

  // Set root node transformation to identity
  root->SetTransformation(R3identity_affine);
}



static void
R3SceneRemoveTransformations(R3Scene *scene, R3SceneNode *node, const R3Affine& parent_transformation)
{
  // Compute transformation
  R3Affine transformation = R3identity_affine;
  transformation.Transform(parent_transformation);
  transformation.Transform(node->Transformation());
  node->SetTransformation(R3identity_affine);

  // Check if leaf
  if (node->NChildren() == 0) {
    // Transform shapes
    for (int i = 0; i < node->NElements(); i++) {
      R3SceneElement *element = node->Element(i);
      element->Transform(transformation);
    }
  }
  else {
    // Recurse to children
    for (int i = 0; i < node->NChildren(); i++) {
      R3SceneNode *child = node->Child(i);
      R3SceneRemoveTransformations(scene, child, transformation);
    }
  }

  // Remember that should update node bounding box
  node->InvalidateBBox();
}



void R3Scene::
RemoveTransformations(void)
{
  // Maintain topology of scene, but set all node transformations to identity
  R3SceneRemoveTransformations(this, root, R3identity_affine);
}



static void
R3SceneSubdivideTriangles(R3Scene *scene, R3SceneNode *node, RNLength max_edge_length)
{
  // Check max edge length
  if (RNIsNegativeOrZero(max_edge_length)) return;

  // Transform max_edge_length
  RNScalar scale = node->Transformation().ScaleFactor();
  if (RNIsNotZero(scale)) max_edge_length /= scale;

  // Subdivide triangles
  for (int i = 0; i < node->NElements(); i++) {
    R3SceneElement *element = node->Element(i);
    for (int j = 0; j < element->NShapes(); j++) {
      R3Shape *shape = element->Shape(j);
      if (shape->ClassID() == R3TriangleArray::CLASS_ID()) {
        R3TriangleArray *triangles = (R3TriangleArray *) shape;
        triangles->Subdivide(max_edge_length);
      }
    }
  }

  // Subdivide children
  for (int i = 0; i < node->NChildren(); i++) {
    R3SceneNode *child = node->Child(i);
    R3SceneSubdivideTriangles(scene, child, max_edge_length);
  }
}



void R3Scene::
SubdivideTriangles(RNLength max_edge_length)
{
  // Subdivide triangles until none is longer than max edge length
  R3SceneSubdivideTriangles(this, root, max_edge_length);
}




RNLength R3Scene::
Distance(const R3Point& point) const
{
  // Find distance to root node
  return root->Distance(point);
}



RNBoolean R3Scene::
FindClosest(const R3Point& point,
  R3SceneNode **hit_node, R3SceneElement **hit_element, R3Shape **hit_shape,
  R3Point *hit_point, R3Vector *hit_normal, RNScalar *hit_d,
  RNScalar min_d, RNScalar max_d) const
{
  // Find closest point in root node
  return root->FindClosest(point, hit_node, hit_element, hit_shape, hit_point, hit_normal, hit_d, min_d, max_d);
}



RNBoolean R3Scene::
Intersects(const R3Ray& ray,
  R3SceneNode **hit_node, R3SceneElement **hit_element, R3Shape **hit_shape,
  R3Point *hit_point, R3Vector *hit_normal, RNScalar *hit_t,
  RNScalar min_t, RNScalar max_t) const
{
  // Intersect with root node
  return root->Intersects(ray, hit_node, hit_element, hit_shape, hit_point, hit_normal, hit_t, min_t, max_t);
}



void R3Scene::
Draw(const R3DrawFlags draw_flags, RNBoolean set_camera, RNBoolean set_lights) const
{
  // Draw null material
  R3null_material.Draw();

  // Set camera
  if (set_camera) {
    viewer.Camera().Load(); 
  }

  // Set lights
  if (set_lights) {
    for (int i = 0; i < lights.NEntries(); i++) {
      lights.Kth(i)->Draw(i);
    }
  }

  // Draw nodes
  root->Draw(draw_flags);

  // Draw null material
  R3null_material.Draw();
}



////////////////////////////////////////////////////////////////////////
// I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3Scene::
ReadFile(const char *filename)
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    fprintf(stderr, "Filename %s has no extension (e.g., .txt)\n", filename);
    return 0;
  }

  // Read file of appropriate type
  if (!strncmp(extension, ".scn", 4)) {
    if (!ReadPrincetonFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".ssc", 4)) {
    if (!ReadParseFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".txt", 4)) {
    if (!ReadSupportHierarchyFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".hier", 5)) {
    if (!ReadGrammarHierarchyFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".obj", 4)) {
    if (!ReadObjFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".off", 4)) {
    if (!ReadMeshFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".ply", 4)) {
    if (!ReadMeshFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".rct", 4)) {
    if (!ReadRectangleFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".json", 5)) {
    if (!ReadPlanner5DFile(filename)) return 0;
  }
  else {
    fprintf(stderr, "Unable to read file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Provide default camera
  if (Camera() == R3default_camera) {
    double scene_radius = BBox().DiagonalRadius();
    R3Point scene_center = BBox().Centroid();
    R3Vector towards = R3Vector(0, 0, -1);
    R3Vector up = R3Vector(0, 1, 0);
    R3Point eye = scene_center - 3 * scene_radius * towards;
    R3Camera camera(eye, towards, up, 0.25, 0.25, 0.01 * scene_radius, 100 * scene_radius);
    SetCamera(camera);
  }

  // Provide default lights
  if (NLights() == 0) {
    // Create first directional light
    RNRgb color1(1,1,1);
    R3Vector direction1(-3,-4,-5);
    direction1.Normalize();
    R3DirectionalLight *light1 = new R3DirectionalLight(direction1, color1);
    InsertLight(light1);

    // Create second directional light
    RNRgb color2(0.5, 0.5, 0.5);
    R3Vector direction2(3,2,3);
    direction2.Normalize();
    R3DirectionalLight *light2 = new R3DirectionalLight(direction2, color2);
    InsertLight(light2);
  }

  // Return success
  return 1;
}



int R3Scene::
WriteFile(const char *filename) const
{
  // Parse input filename extension
  const char *extension;
  if (!(extension = strrchr(filename, '.'))) {
    fprintf(stderr, "Filename %s has no extension (e.g., .txt)\n", filename);
    return 0;
  }

  // Write file of appropriate type
  if (!strncmp(extension, ".txt", 4)) {
    if (!WriteSupportHierarchyFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".scn", 4)) {
    if (!WritePrincetonFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".ssc", 4)) {
    if (!WriteParseFile(filename)) return 0;
  }
  else if (!strncmp(extension, ".obj", 4)) {
    if (!WriteObjFile(filename)) return 0;
  }
  else {
    fprintf(stderr, "Unable to write file %s (unrecognized extension: %s)\n", filename, extension);
    return 0;
  }

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// OBJ FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

#include <string>
#include <map>

static int
InsertSceneElement(R3Scene *scene, R3SceneNode *node, const char *object_name, R3Material *material, 
  const RNArray<R3TriangleVertex *>& verts, const RNArray<R3Triangle *>& tris)
{
  // Create material if none
  if (!material) {
    R3Brdf *brdf = new R3Brdf("Default");
    scene->InsertBrdf(brdf);
    material = new R3Material(brdf, "Default");
    scene->InsertMaterial(material);
  }
  
  // Create child node if object name is set
  if (object_name) {
    R3SceneNode *child = new R3SceneNode(scene);
    child->SetName(object_name);
    node->InsertChild(child);
    node = child;
  }

  // Find element
  R3SceneElement *element = NULL;
  for (int i = 0; i < node->NElements(); i++) {
    R3SceneElement *g = node->Element(i);
    if (g->Material() == material) { element = g; break; }
  }

  // Create element
  if (!element) {
    element = new R3SceneElement(material);
  }

  // Mark vertices used by tris
  for (int i = 0; i < verts.NEntries(); i++) {
    R3TriangleVertex *vertex = verts.Kth(i);
    vertex->SetMark(0);
  }

  // Create array of verts used by tris
  RNArray<R3TriangleVertex *> tri_verts;
  for (int i = 0; i < tris.NEntries(); i++) {
    R3Triangle *triangle = tris.Kth(i);
    for (int j = 0; j < 3; j++) {
      R3TriangleVertex *vertex = triangle->Vertex(j);
      if (vertex->Mark() == 0) {
        tri_verts.Insert(vertex);
        vertex->SetMark(1);
      }
    }
  }

  // Create shape (triangle array)
  R3TriangleArray *shape = new R3TriangleArray(tri_verts, tris);
        
  // Insert shape
  element->InsertShape(shape);
        
  // Insert element
  node->InsertElement(element);

  // Return success
  return 1;
}



static int
ReadObjMtlFile(R3Scene *scene, const char *dirname, const char *mtlname, RNArray<R3Material *> *returned_materials)
{
  // Open file
  char filename[1024];
  if (dirname) sprintf(filename, "%s/%s", dirname, mtlname);
  else strncpy(filename, mtlname, 1024);
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Initialize returned materials
  if (returned_materials) returned_materials->Empty();

  // Parse file
  char buffer[1024];
  int line_count = 0;
  R3Brdf *brdf = NULL;
  R3Material *material = NULL;
  std::map<std::string, R2Texture *> texture_symbol_table;
  while (fgets(buffer, 1023, fp)) {
    // Increment line counter
    line_count++;

    // Skip white space
    char *bufferp = buffer;
    while (isspace(*bufferp)) bufferp++;

    // Skip blank lines and comments
    if (*bufferp == '#') continue;
    if (*bufferp == '\0') continue;

    // Get keyword
    char keyword[80];
    if (sscanf(bufferp, "%s", keyword) != 1) {
      fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
      return 0;
    }

    // Check keyword
    if (!strcmp(keyword, "newmtl")) {
      // Parse line
      char name[1024];
      if (sscanf(bufferp, "%s%s", keyword, name) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Create new material
      brdf = new R3Brdf(name);
      scene->InsertBrdf(brdf);
      material = new R3Material(brdf, NULL, name);
      scene->InsertMaterial(material);
      if (returned_materials) returned_materials->Insert(material);
    }
    else if (!strcmp(keyword, "Ka")) {
      // Parse line
      double r, g, b;
      if (sscanf(bufferp, "%s%lf%lf%lf", keyword, &r, &g, &b) != (unsigned int) 4) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set ambient reflectance 
      if (material && brdf) {
        brdf->SetAmbient(RNRgb(r, g, b));
        material->Update();
      }
    }
    else if (!strcmp(keyword, "Kd")) {
      // Parse line
      double r, g, b;
      if (sscanf(bufferp, "%s%lf%lf%lf", keyword, &r, &g, &b) != (unsigned int) 4) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set diffuse reflectance 
      if (material && brdf) {
        brdf->SetDiffuse(RNRgb(r, g, b));
        material->Update();
      }
    }
    else if (!strcmp(keyword, "Ks")) {
      // Parse line
      double r, g, b;
      if (sscanf(bufferp, "%s%lf%lf%lf", keyword, &r, &g, &b) != (unsigned int) 4) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set specular reflectance 
      if (material && brdf) {
        brdf->SetSpecular(RNRgb(r, g, b));
        material->Update();
      }
    }
    else if (!strcmp(keyword, "Ke")) {
      // Parse line
      double r, g, b;
      if (sscanf(bufferp, "%s%lf%lf%lf", keyword, &r, &g, &b) != (unsigned int) 4) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set emission
      if (material && brdf) {
        brdf->SetEmission(RNRgb(r, g, b));
        material->Update();
      }
    }
    else if (!strcmp(keyword, "Tf")) {
      // Parse line
      double r, g, b;
      if (sscanf(bufferp, "%s%lf%lf%lf", keyword, &r, &g, &b) != (unsigned int) 4) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set transmission
      if (material && brdf) {
        brdf->SetTransmission(RNRgb(r, g, b));
        material->Update();
      }
    }
    else if (!strcmp(keyword, "Tr")) {
      // Parse line
      double transparency;
      if (sscanf(bufferp, "%s%lf", keyword, &transparency) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set transmission
      if (material && brdf) {
        brdf->SetOpacity(1 - transparency);
        material->Update();
      }
    }
    else if (!strcmp(keyword, "d")) {
      // Parse line
      double opacity;
      if (sscanf(bufferp, "%s%lf", keyword, &opacity) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set transmission
      if (material && brdf) {
        brdf->SetOpacity(opacity);
        material->Update();
      }
    }
    else if (!strcmp(keyword, "Ns")) {
      // Parse line
      double ns;
      if (sscanf(bufferp, "%s%lf", keyword, &ns) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set shininess
      if (material && brdf) {
        brdf->SetShininess(ns);
        material->Update();
      }
    }
    else if (!strcmp(keyword, "Ni")) {
      // Parse line
      double index_of_refraction;
      if (sscanf(bufferp, "%s%lf", keyword, &index_of_refraction) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set index of refraction
      if (material && brdf) {
        brdf->SetIndexOfRefraction(index_of_refraction);
        material->Update();
      }
    }
    else if (!strcmp(keyword, "map_Kd")) {
      // Parse line
      char texture_name[1024];
      if (sscanf(bufferp, "%s%s", keyword, texture_name) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Set texture
      if (material) {
        char texture_filename[1024];
        if (dirname) sprintf(texture_filename, "%s/%s", dirname, texture_name);
        else strncpy(texture_filename, texture_name, 1024);
        std::map<std::string, R2Texture *>::iterator it = texture_symbol_table.find(texture_filename);
        R2Texture *texture = (it != texture_symbol_table.end()) ? it->second : NULL;
        if (!texture) {
          R2Image *image = new R2Image();
          if (!image->Read(texture_filename)) return 0;
          texture = new R2Texture(image);
          texture_symbol_table[texture_filename] = texture;
          texture->SetName(texture_filename);
          scene->InsertTexture(texture);
        }
        material->SetTexture(texture);
        material->Update();
      }
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int
ReadObj(R3Scene *scene, R3SceneNode *node, const char *dirname, FILE *fp, RNArray<R3Material *> *returned_materials = NULL)
{
  // Read body
  char buffer[1024];
  int line_count = 0;
  char *object_name = NULL;
  R3Material *material = NULL;
  std::map<std::string, R3Material *> material_symbol_table;
  RNArray<R2Point *> texture_coords;
  RNArray<R3TriangleVertex *> verts;
  RNArray<R3Triangle *> tris;
  while (fgets(buffer, 1023, fp)) {
    // Increment line counter
    line_count++;

    // Skip white space
    char *bufferp = buffer;
    while (isspace(*bufferp)) bufferp++;

    // Skip blank lines and comments
    if (*bufferp == '#') continue;
    if (*bufferp == '\0') continue;

    // Get keyword
    char keyword[80];
    if (sscanf(bufferp, "%s", keyword) != 1) {
      fprintf(stderr, "Syntax error on line %d in OBJ file", line_count);
      return 0;
    }

    // Check keyword
    if (!strcmp(keyword, "v")) {
      // Read vertex coordinates
      double x, y, z;
      if (sscanf(bufferp, "%s%lf%lf%lf", keyword, &x, &y, &z) != 4) {
        fprintf(stderr, "Syntax error on line %d in OBJ file", line_count);
        return 0;
      }

      // Create vertex
      R3TriangleVertex *vertex = new R3TriangleVertex(R3Point(x, y, z));
      verts.Insert(vertex);
    }
    else if (!strcmp(keyword, "vt")) {
      // Read texture coordinates
      double u, v;
      if (sscanf(bufferp, "%s%lf%lf", keyword, &u, &v) != 3) {
        fprintf(stderr, "Syntax error on line %d in OBJ file", line_count);
        return 0;
      }

      // Create texture coordinates
      R2Point *vt = new R2Point(u, v);
      texture_coords.Insert(vt);
    }
    else if (!strcmp(keyword, "f")) {
      // Read vertex indices
      int quad = 1;
      char s1[128], s2[128], s3[128], s4[128] = { '\0' };
      if (sscanf(bufferp, "%s%s%s%s%s", keyword, s1, s2, s3, s4) != 5) {
        quad = 0;;
        if (sscanf(bufferp, "%s%s%s%s", keyword, s1, s2, s3) != 4) {
          fprintf(stderr, "Syntax error on line %d in OBJ file", line_count);
          return 0;
        }
      }

      // Parse vertex indices
      int vi1 = -1, vi2 = -1, vi3 = -1, vi4 = -1;
      int ti1 = -1, ti2 = -1, ti3 = -1, ti4 = -1;
      char *p1 = strchr(s1, '/'); 
      if (p1) { *p1 = 0; vi1 = atoi(s1); p1++; if (*p1) ti1 = atoi(p1); }
      else { vi1 = atoi(s1); ti1 = vi1; }
      char *p2 = strchr(s2, '/'); 
      if (p2) { *p2 = 0; vi2 = atoi(s2); p2++; if (*p2) ti2 = atoi(p2); }
      else { vi2 = atoi(s2); ti2 = vi2; }
      char *p3 = strchr(s3, '/'); 
      if (p3) { *p3 = 0; vi3 = atoi(s3); p3++; if (*p3) ti3 = atoi(p3); }
      else { vi3 = atoi(s3); ti3 = vi3; }
      if (quad) {
        char *p4 = strchr(s4, '/'); 
        if (p4) { *p4 = 0; vi4 = atoi(s4); p4++; if (*p4) ti4 = atoi(p4); }
        else { vi4 = atoi(s4); ti4 = vi4; }
      }

      // Get vertices
      R3TriangleVertex *v1 = verts.Kth(vi1-1);
      R3TriangleVertex *v2 = verts.Kth(vi2-1);
      R3TriangleVertex *v3 = verts.Kth(vi3-1);
      R3TriangleVertex *v4 = (quad) ? verts.Kth(vi4-1) : NULL;
      
      // Assign texture coordinates
      if ((ti1 > 0) && ((ti1-1) < texture_coords.NEntries())) v1->SetTextureCoords(*(texture_coords.Kth(ti1-1)));
      if ((ti2 > 0) && ((ti2-1) < texture_coords.NEntries())) v2->SetTextureCoords(*(texture_coords.Kth(ti2-1)));
      if ((ti3 > 0) && ((ti3-1) < texture_coords.NEntries())) v3->SetTextureCoords(*(texture_coords.Kth(ti3-1)));
      if (quad) {
        if ((ti4 > 0) && ((ti4-1) < texture_coords.NEntries())) v4->SetTextureCoords(*(texture_coords.Kth(ti4-1)));
      }

      // Check vertices
      if ((v1 == v2) || (v2 == v3) || (v1 == v3)) continue;
      if ((quad) && ((v4 == v1) || (v4 == v2) || (v4 == v3))) quad = 0;

      // Create first triangle
      if (RNIsPositive(R3Distance(v1->Position(), v2->Position())) &&
          RNIsPositive(R3Distance(v2->Position(), v3->Position())) &&
          RNIsPositive(R3Distance(v1->Position(), v3->Position()))) {
        R3Triangle *triangle = new R3Triangle(v1, v2, v3);
        tris.Insert(triangle);
      }

      // Create second triangle
      if (quad) {
        if (RNIsPositive(R3Distance(v1->Position(), v3->Position())) &&
            RNIsPositive(R3Distance(v3->Position(), v4->Position())) &&
            RNIsPositive(R3Distance(v1->Position(), v4->Position()))) {
          R3Triangle *triangle = new R3Triangle(v1, v3, v4);
          tris.Insert(triangle);
        }
      }
    }
    else if (!strcmp(keyword, "mtllib")) {
      // Read fields
      char mtlname[1024];
      if (sscanf(bufferp, "%s%s", keyword, mtlname) != 2) {
        fprintf(stderr, "Syntax error on line %d in OBJ file", line_count);
        return 0;
      }

      // Read materials
      RNArray<R3Material *> parsed_materials;
      if (!ReadObjMtlFile(scene, dirname, mtlname, &parsed_materials)) return 0;
      if (returned_materials) *returned_materials = parsed_materials;

      // Fill symbol table
      material_symbol_table.clear();
      for (int i = 0; i < parsed_materials.NEntries(); i++) {
        R3Material *material = parsed_materials.Kth(i);
        if (!material->Name()) continue;
        material_symbol_table[material->Name()] = material;
      }
    }
    else if (!strcmp(keyword, "usemtl")) {
      // Read fields
      char mtlname[1024];
      if (sscanf(bufferp, "%s%s", keyword, mtlname) != 2) {
        fprintf(stderr, "Syntax error on line %d in OBJ file", line_count);
        return 0;
      }

      // Process triangles from previous material
      if ((verts.NEntries() > 0) && (tris.NEntries() > 0)) {
        InsertSceneElement(scene, node, object_name, material, verts, tris);
        tris.Empty();
      }

      // Find material
      std::map<std::string, R3Material *>::iterator it = material_symbol_table.find(mtlname);
      material = (it != material_symbol_table.end()) ? it->second : NULL;
      if (!material) {
        fprintf(stderr, "Unable to find material %s at on line %d in OBJ file\n", mtlname, line_count);
        return 0;
      }
    }
    else if (!strcmp(keyword, "g") || !strcmp(keyword, "o")) {
      // Read name
      char name[1024];
      if (sscanf(bufferp, "%s%s", keyword, name) != 2) {
        fprintf(stderr, "Syntax error on line %d in OBJ file", line_count);
        return 0;
      }

      // Process triangles from previous object
      if ((verts.NEntries() > 0) && (tris.NEntries() > 0)) {
        InsertSceneElement(scene, node, object_name, material, verts, tris);
        tris.Empty();
      }

      // Remember object name
      if (object_name) free(object_name);
      object_name = strdup(name);
    }
  }

  // Process triangles from previous material
  if ((verts.NEntries() > 0) && (tris.NEntries() > 0)) {
    InsertSceneElement(scene, node, object_name, material, verts, tris);
    tris.Empty();
  }

  // Delete texture coordinates
  for (int i = 0; i < texture_coords.NEntries(); i++) {
    R2Point *vt = texture_coords.Kth(i);
    delete vt;
  }

  // Return success
  return 1;
}



int
ReadObj(R3Scene *scene, R3SceneNode *node, const char *filename, RNArray<R3Material *> *returned_materials)
{
  // Open file
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Determine directory name (for texture image files)
  char *dirname = NULL;
  char buffer[1024];
  strncpy(buffer, filename, 1024);
  char *endp = strrchr(buffer, '/');
  if (!endp) endp = strrchr(buffer, '\\');
  if (endp) { *endp = '\0'; dirname = buffer; }

  // Read file
  if (!ReadObj(scene, node, dirname, fp, returned_materials)) {
    fprintf(stderr, "Unable to read OBJ file %s\n", filename);
    fclose(fp);
    return 0;
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3Scene::
ReadObjFile(const char *filename)
{
  // Read obj file, and put contents in root node
  return ReadObj(this, root, filename);
}



////



static int
WriteObjMtlFile(const R3Scene *scene, const char *dirname, const char *mtlname)
{
  // Open file
  char filename[1024];
  if (dirname) sprintf(filename, "%s/%s", dirname, mtlname);
  else strncpy(filename, mtlname, 1024);
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Write default material
  fprintf(fp, "newmtl _DEFAULT_\n");
  fprintf(fp, "Kd 0 0 0\n");
  fprintf(fp, "\n");

  // Write materials
  for (int i = 0; i < scene->NMaterials(); i++) {
    R3Material *material = scene->Material(i);  

    // ??? CHANGE NAME OF MATERIAL SO THAT IT IS UNIQUE ???
    char matname[1024];
    if (!material->Name()) sprintf(matname, "m%d", i);
    else sprintf(matname, "m%d_%s", i, material->Name());
    material->SetName(matname);
    
    // Write new material command
    fprintf(fp, "newmtl %s\n", material->Name());

    // Write brdf
    if (material->Brdf()) {
      const R3Brdf *brdf = material->Brdf();
      if (brdf->IsAmbient()) fprintf(fp, "Ka %g %g %g\n", brdf->Ambient().R(), brdf->Ambient().G(), brdf->Ambient().B());
      if (brdf->IsDiffuse()) fprintf(fp, "Kd %g %g %g\n", brdf->Diffuse().R(), brdf->Diffuse().G(), brdf->Diffuse().B());
      if (brdf->IsSpecular()) fprintf(fp, "Ks %g %g %g\n", brdf->Specular().R(), brdf->Specular().G(), brdf->Specular().B());
      if (brdf->IsEmissive()) fprintf(fp, "Ke %g %g %g\n", brdf->Emission().R(), brdf->Emission().G(), brdf->Emission().B());
      if (brdf->IsTransparent()) fprintf(fp, "Tf %g %g %g\n", brdf->Transmission().R(), brdf->Transmission().G(), brdf->Transmission().B());
      if (RNIsNotEqual(brdf->Opacity(), 1.0)) fprintf(fp, "Tr %g\n", 1.0 - brdf->Opacity());
      if (RNIsNotEqual(brdf->Opacity(), 1.0)) fprintf(fp, "d %g\n", brdf->Opacity());
      if (RNIsNotZero(brdf->Shininess())) fprintf(fp, "Ns %g\n", brdf->Shininess());
      if (RNIsNotZero(brdf->IndexOfRefraction())) fprintf(fp, "Ni %g\n", brdf->IndexOfRefraction());
    }

    // Write texture
    if (material->Texture()) {
      const R2Texture *texture = material->Texture();

      // Check if texture has a filename associated with it 
      if (texture->Name()) {
        // Write texture command to material file (USE SAME FILE AS READ WITHOUT RE-WRITING IT)
        // fprintf(fp, "map_Ka %s\n", texture->Name());
        fprintf(fp, "map_Kd %s\n", texture->Name());
      }
      else {
        // Get texture filename
        char texture_filename[1024];
        const char *texture_extension = "jpg";
        if (dirname) sprintf(texture_filename, "%s/%s.%s", dirname, material->Name(), texture_extension);
        else sprintf(texture_filename, "%s.%s", material->Name(), texture_extension);
        
        // Write texture file
        const R2Image *texture_image = texture->Image();
        texture_image->Write(texture_filename);
        
        // Write texture command to material file
        // fprintf(fp, "map_Ka %s.%s\n", material->Name(), texture_extension);
        fprintf(fp, "map_Kd %s.%s\n", material->Name(), texture_extension);
      }
    }

    // Write new line after each material
    fprintf(fp, "\n");
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int
WriteObj(const R3Scene *scene, R3SceneNode *node, const R3Affine& transformation, int &ngroups, int& nvertices, int& ntexture_coords, FILE *fp)
{
  // Write group name
  if (node->Name()) fprintf(fp, "g %s\n", node->Name());
  else fprintf(fp, "g GROUP_%d\n", ++ngroups);
  
  // Write elements
  for (int i = 0; i < node->NElements(); i++) {
    R3SceneElement *element = node->Element(i);

    // Write material
    R3Material *material = element->Material();
    if (material) fprintf(fp, "usemtl %s\n", material->Name());
    else fprintf(fp, "usemtl _DEFAULT_\n");
      
    // Write shapes
    for (int j = 0; j < element->NShapes(); j++) {
      R3Shape *shape = element->Shape(j);
      if (shape->ClassID() == R3TriangleArray::CLASS_ID()) {
        R3TriangleArray *triangles = (R3TriangleArray *) shape;

        // Write vertices
        for (int k = 0; k < triangles->NVertices(); k++) {
          R3TriangleVertex *v = triangles->Vertex(k);
          R3Point p = v->Position();
          R2Point t = v->TextureCoords();
          p.Transform(transformation);
          fprintf(fp, "v %g %g %g\n", p.X(), p.Y(), p.Z());
          fprintf(fp, "vt %g %g\n", t.X(), t.Y());
          v->SetMark(++nvertices); // Store index of this vertex in whole file (starting at index=1)
        }
       

        // Write triangles
        for (int k = 0; k < triangles->NTriangles(); k++) {
          R3Triangle *triangle = triangles->Triangle(k);
          R3TriangleVertex *v0 = triangle->V0();
          R3TriangleVertex *v1 = triangle->V1();
          R3TriangleVertex *v2 = triangle->V2();
          unsigned int i0 = v0->Mark();
          unsigned int i1 = v1->Mark();
          unsigned int i2 = v2->Mark();
          if (triangle->Flags()[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG]) {
            if (transformation.IsMirrored()) fprintf(fp, "f %u/%u %u/%u %u/%u\n", i2, i2, i1, i1, i0, i0);
            else fprintf(fp, "f %u/%u %u/%u %u/%u\n", i0, i0, i1, i1, i2, i2);
          }
          else {
            if (transformation.IsMirrored()) fprintf(fp, "f %u %u %u\n", i2, i1, i0);
            else fprintf(fp, "f %u %u %u\n", i0, i1, i2);
          }
        }
      }
    }
  }

  // Write children
  for (int i = 0; i < node->NChildren(); i++) {
    R3SceneNode *child = node->Child(i);

    // Update transformation
    R3Affine child_transformation = R3identity_affine;
    child_transformation.Transform(transformation);
    child_transformation.Transform(child->Transformation());

    // Write child
    if (!WriteObj(scene, child, child_transformation, ngroups, nvertices, ntexture_coords, fp)) return 0;
  }

  // Return success
  return 1;
}



int 
WriteObj(const R3Scene *scene, R3SceneNode *node, const char *filename) 
{
  // Determine directory name (for texture image files)
  char *dirname = NULL;
  char buffer[1024];
  strncpy(buffer, filename, 1024);
  char *endp = strrchr(buffer, '/');
  if (!endp) endp = strrchr(buffer, '\\');
  if (!endp) strcpy(buffer, ".");
  else { *endp = '\0'; dirname = buffer; }

  // Determine material filename
  char mtl_filename[1024];
  const char *startp = strrchr(filename, '/');
  startp = (startp) ? startp+1 : filename;
  strncpy(mtl_filename, startp, 1024);
  int slen = strlen(mtl_filename);
  if (slen > 4) mtl_filename[slen-4] = '\0';
  strncat(mtl_filename, ".mtl", 1024);

  // Create directory
  if (dirname) {
    char mkdir_cmd[1024];
    sprintf(mkdir_cmd, "mkdir -p %s", dirname);
    system(mkdir_cmd);
  }

  // Open file
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Write material file
  fprintf(fp, "mtllib %s\n", mtl_filename);
  if (!WriteObjMtlFile(scene, dirname, mtl_filename)) {
    fprintf(stderr, "Unable to write OBJ material file %s\n", mtl_filename);
    fclose(fp);
    return 0;
  }

  // Write nodes 
  int ngroups = 0;
  int nvertices = 0;
  int ntexture_coords = 0;
  if (!WriteObj(scene, node, node->Transformation(), ngroups, nvertices, ntexture_coords, fp)) {
    fprintf(stderr, "Unable to write OBJ file %s\n", filename);
    fclose(fp);
    return 0;
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3Scene::
WriteObjFile(const char *filename) const
{
  // Write obj file
  return WriteObj(this, root, filename);
}



////////////////////////////////////////////////////////////////////////
// MESH FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

static R3TriangleArray *
ReadMesh(const char *filename)
{
  R3Mesh mesh;
  if (!mesh.ReadFile(filename)) {
    fprintf(stderr, "Unable to read mesh %s\n", filename);
    return NULL;
  }
  
  // Create array of vertices
  RNArray<R3TriangleVertex *> vertices;
  for (int i = 0; i < mesh.NVertices(); i++) {
    R3MeshVertex *mesh_vertex = mesh.Vertex(i);
    const R3Point& position = mesh.VertexPosition(mesh_vertex);
    const R3Vector& normal = mesh.VertexNormal(mesh_vertex);
    R3TriangleVertex *triangle_vertex = new R3TriangleVertex(position, normal);
    vertices.Insert(triangle_vertex);
  }

  // Create array of triangles
  RNArray<R3Triangle *> triangles;
  for (int i = 0; i < mesh.NFaces(); i++) {
    R3MeshFace *mesh_face = mesh.Face(i);
    int i0 = mesh.VertexID(mesh.VertexOnFace(mesh_face, 0));
    int i1 = mesh.VertexID(mesh.VertexOnFace(mesh_face, 1));
    int i2 = mesh.VertexID(mesh.VertexOnFace(mesh_face, 2));
    R3TriangleVertex *v0 = vertices.Kth(i0);
    R3TriangleVertex *v1 = vertices.Kth(i1);
    R3TriangleVertex *v2 = vertices.Kth(i2);
    R3Triangle *triangle = new R3Triangle(v0, v1, v2);
    triangles.Insert(triangle);
  }

  // Return triangle array
  return new R3TriangleArray(vertices, triangles);
}

 

int R3Scene::
ReadMeshFile(const char *filename)
{
  // Load triangles into scene
  R3TriangleArray *shape = ReadMesh(filename);
  if (!shape) return 0;
  R3SceneElement *element = new R3SceneElement();
  element->InsertShape(shape);
  R3SceneNode *node = new R3SceneNode(this);
  node->InsertElement(element);
  root->InsertChild(node);
  
  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// PRINCETON SCENE FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

static int
FindPrincetonMaterialAndElement(R3Scene *scene, R3SceneNode *node,
  const RNArray<R3Material *>& materials, int m, R3Material *&default_material,
  R3Material *& material, R3SceneElement *& element)
{
  // Find material from m
  material = NULL;
  if (m >= 0) {
    if (m < materials.NEntries()) material = materials[m];
    else return 0;
  }
  else {
    // Get material from 
    material = default_material;
    if (!material) {
      R3Brdf *brdf = new R3Brdf(R3default_brdf);
      scene->InsertBrdf(brdf);
      material = new R3Material(brdf);
      scene->InsertMaterial(material);
      default_material = material;
    }
  }

  // Find element with that material
  element = NULL;
  for (int i = 0; i < node->NElements(); i++) {
    R3SceneElement *e = node->Element(i);
    if (e->Material() == material) { element = e; break; }
  }

  // Create element if none found
  if (!element) {
    element = new R3SceneElement(material);
    node->InsertElement(element);
  }

  // Return success
  return 1;
}



static int
ReadPrinceton(R3Scene *scene, R3SceneNode *node, const char *filename)
{
  // Open file
  FILE *fp;
  if (!(fp = fopen(filename, "r"))) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Create array of materials
  RNArray<R3Material *> parsed_materials;
  R3Material *material = NULL;
  R3SceneElement *element = NULL;

  // Create stack of group information
  const int max_depth = 1024;
  R3SceneNode *group_nodes[max_depth] = { NULL };
  R3Material *group_materials[max_depth] = { NULL };
  group_nodes[0] = (node) ? node : scene->Root();
  int depth = 0;

  // Read body
  char cmd[128];
  int command_number = 1;
  while (fscanf(fp, "%s", cmd) == 1) {
    if (cmd[0] == '#') {
      // Comment -- read everything until end of line
      do { cmd[0] = fgetc(fp); } while ((cmd[0] >= 0) && (cmd[0] != '\n'));
    }
    else if (!strcmp(cmd, "tri")) {
      // Read data
      int m;
      R3Point p1, p2, p3;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf", &m, 
        &p1[0], &p1[1], &p1[2], &p2[0], &p2[1], &p2[2], &p3[0], &p3[1], &p3[2]) != 10) {
        fprintf(stderr, "Unable to read triangle at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create triangle
      R3TriangleVertex *v1 = new R3TriangleVertex(p1);
      R3TriangleVertex *v2 = new R3TriangleVertex(p2);
      R3TriangleVertex *v3 = new R3TriangleVertex(p3);
      R3Triangle *triangle = new R3Triangle(v1, v2, v3);

      // Get material and element from m
      if (!FindPrincetonMaterialAndElement(scene, group_nodes[depth], parsed_materials, m, group_materials[depth], material, element)) {
        fprintf(stderr, "Invalid material id at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Insert triangle into element
      element->InsertShape(triangle);
    }
    else if (!strcmp(cmd, "box")) {
      // Read data
      int m;
      R3Point p1, p2;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf", &m, &p1[0], &p1[1], &p1[2], &p2[0], &p2[1], &p2[2]) != 7) {
        fprintf(stderr, "Unable to read box at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Sort coordinates
      if (p1[0] > p2[0]) { RNCoord swap = p1[0]; p1[0] = p2[0]; p2[0] = swap; }
      if (p1[1] > p2[1]) { RNCoord swap = p1[1]; p1[1] = p2[1]; p2[1] = swap; }
      if (p1[2] > p2[2]) { RNCoord swap = p1[2]; p1[2] = p2[2]; p2[2] = swap; }

      // Create box
      R3Box *box = new R3Box(p1, p2);

      // Get material and element from m
      if (!FindPrincetonMaterialAndElement(scene, group_nodes[depth], parsed_materials, m, group_materials[depth], material, element)) {
        fprintf(stderr, "Invalid material id at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Insert shape into element
      element->InsertShape(box);
    }
    else if (!strcmp(cmd, "sphere")) {
      // Read data
      int m;
      R3Point c;
      double r;
      if (fscanf(fp, "%d%lf%lf%lf%lf", &m, &c[0], &c[1], &c[2], &r) != 5) {
        fprintf(stderr, "Unable to read sphere at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create sphere
      R3Sphere *sphere = new R3Sphere(c, r);

      // Get material and element from m
      if (!FindPrincetonMaterialAndElement(scene, group_nodes[depth], parsed_materials, m, group_materials[depth], material, element)) {
        fprintf(stderr, "Invalid material id at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Insert shape into element
      element->InsertShape(sphere);
    }
    else if (!strcmp(cmd, "cylinder")) {
      // Read data
      int m;
      R3Point c;
      double r, h;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf", &m, &c[0], &c[1], &c[2], &r, &h) != 6) {
        fprintf(stderr, "Unable to read cylinder at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create cylinder
      R3Point p1 = c - 0.5 * h * R3posy_vector;
      R3Point p2 = c + 0.5 * h * R3posy_vector;
      R3Cylinder *cylinder = new R3Cylinder(p1, p2, r);

      // Get material and element from m
      if (!FindPrincetonMaterialAndElement(scene, group_nodes[depth], parsed_materials, m, group_materials[depth], material, element)) {
        fprintf(stderr, "Invalid material id at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Insert shape into element
      element->InsertShape(cylinder);
    }
    else if (!strcmp(cmd, "cone")) {
      // Read data
      int m;
      R3Point c;
      double r, h;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf", &m, &c[0], &c[1], &c[2], &r, &h) != 6) {
        fprintf(stderr, "Unable to read cone at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create cone
      R3Point p1 = c - 0.5 * h * R3posy_vector;
      R3Point p2 = c + 0.5 * h * R3posy_vector;
      R3Cone *cone = new R3Cone(p1, p2, r);

      // Get material and element from m
      if (!FindPrincetonMaterialAndElement(scene, group_nodes[depth], parsed_materials, m, group_materials[depth], material, element)) {
        fprintf(stderr, "Invalid material id at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Insert shape into element
      element->InsertShape(cone);
    }
    else if (!strcmp(cmd, "mesh")) {
      // Read data
      int m;
      char meshname[256];
      if (fscanf(fp, "%d%s", &m, meshname) != 2) {
        fprintf(stderr, "Unable to parse mesh command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get mesh filename
      char buffer[2048];
      strcpy(buffer, filename);
      char *bufferp = strrchr(buffer, '/');
      if (bufferp) *(bufferp+1) = '\0';
      else buffer[0] = '\0';
      strcat(buffer, meshname);

      // Read mesh
      R3TriangleArray *mesh = ReadMesh(buffer);
      if (!mesh) return 0;

      // Get material and element from m
      if (!FindPrincetonMaterialAndElement(scene, group_nodes[depth], parsed_materials, m, group_materials[depth], material, element)) {
        fprintf(stderr, "Invalid material id at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Insert shape into element
      element->InsertShape(mesh);
    }
    else if (!strcmp(cmd, "line")) {
      // Read data
      int m;
      double x1, y1, z1, x2, y2, z2;
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf", &m, &x1, &y1, &z1, &x2, &y2, &z2) != 7) {
        fprintf(stderr, "Unable to read line at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create cylinder representing line
      R3Point p1(x1, y1, z1);
      R3Point p2(x2, y2, z2);
      R3Cylinder *cylinder = new R3Cylinder(p1, p2, RN_BIG_EPSILON);

      // Get material and element from m
      if (!FindPrincetonMaterialAndElement(scene, group_nodes[depth], parsed_materials, m, group_materials[depth], material, element)) {
        fprintf(stderr, "Invalid material id at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Insert shape into element
      element->InsertShape(cylinder);
    }
    else if (!strcmp(cmd, "begin")) {
      // Read data
      int m;
      double matrix[16];
      if (fscanf(fp, "%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &m, 
        &matrix[0], &matrix[1], &matrix[2], &matrix[3], 
        &matrix[4], &matrix[5], &matrix[6], &matrix[7], 
        &matrix[8], &matrix[9], &matrix[10], &matrix[11], 
        &matrix[12], &matrix[13], &matrix[14], &matrix[15]) != 17) {
        fprintf(stderr, "Unable to read begin at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get material from m
      if (m >= 0) {
        if (m < parsed_materials.NEntries()) material = parsed_materials[m];
        else material = NULL;
      }

      // Create new node
      R3SceneNode *node = new R3SceneNode(scene);
      node->SetTransformation(R3Affine(R4Matrix(matrix)));
      group_nodes[depth]->InsertChild(node);

      // Push node onto stack
      depth++;
      group_nodes[depth] = node;
      group_materials[depth] = material;
    }
    else if (!strcmp(cmd, "end")) {
      // Check stack depth
      if (depth <= 0) {
        fprintf(stderr, "Extra end statement at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Pop node from stack
      depth--;
    }
    else if (!strcmp(cmd, "material")) {
      // Read data
      RNRgb ka, kd, ks, kt, e;
      double n, ir;
      char texture_name[256];
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%s", 
          &ka[0], &ka[1], &ka[2], &kd[0], &kd[1], &kd[2], &ks[0], &ks[1], &ks[2], &kt[0], &kt[1], &kt[2], 
          &e[0], &e[1], &e[2], &n, &ir, texture_name) != 18) {
        fprintf(stderr, "Unable to read material at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create brdf
      R3Brdf *brdf = new R3Brdf(ka, kd, ks, kt, e, n, ir);
      scene->InsertBrdf(brdf);

      // Create texture
      R2Texture *texture = NULL;
      if (strcmp(texture_name, "0")) {
        // Get texture filename
        char buffer[2048];
        strcpy(buffer, filename);
        char *bufferp = strrchr(buffer, '/');
        if (bufferp) *(bufferp+1) = '\0';
        else buffer[0] = '\0';
        strcat(buffer, texture_name);

        // Read texture file
        R2Image *image = new R2Image();
        if (!image->Read(buffer)) {
          fprintf(stderr, "Unable to read texture from %s at command %d in file %s\n", buffer, command_number, filename);
          return 0;
        }
        
        // Create texture
        texture = new R2Texture(image);
        scene->InsertTexture(texture);
      }

      // Create material
      R3Material *material = new R3Material(brdf, texture);
      scene->InsertMaterial(material);
      parsed_materials.Insert(material);
    }
    else if (!strcmp(cmd, "dir_light")) {
      // Read data
      RNRgb c;
      R3Vector d;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf", 
        &c[0], &c[1], &c[2], &d[0], &d[1], &d[2]) != 6) {
        fprintf(stderr, "Unable to read directional light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Normalize direction
      d.Normalize();

      // Create directional light
      R3DirectionalLight *light = new R3DirectionalLight(d, c);
      scene->InsertLight(light);
    }
    else if (!strcmp(cmd, "point_light")) {
      // Read data
      RNRgb c;
      R3Point p;
      double ca, la, qa;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &c[0], &c[1], &c[2], &p[0], &p[1], &p[2], &ca, &la, &qa) != 9) {
        fprintf(stderr, "Unable to read point light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Create point light
      R3PointLight *light = new R3PointLight(p, c, 1, TRUE, ca, la, qa);
      scene->InsertLight(light);
    }
    else if (!strcmp(cmd, "spot_light")) {
      // Read data
      RNRgb c;
      R3Point p;
      R3Vector d;
      double ca, la, qa, sc, sd;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
        &c[0], &c[1], &c[2], &p[0], &p[1], &p[2], &d[0], &d[1], &d[2], &ca, &la, &qa, &sc, &sd) != 14) {
        fprintf(stderr, "Unable to read point light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Normalize direction
      d.Normalize();

      // Create spot light
      R3SpotLight *light = new R3SpotLight(p, d, c, sd, sc, 1, TRUE, ca, la, qa);
      scene->InsertLight(light);
    }
    else if (!strcmp(cmd, "area_light")) {
      // Read data
      RNRgb c;
      R3Point p;
      R3Vector d;
      double radius, ca, la, qa;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
        &c[0], &c[1], &c[2], &p[0], &p[1], &p[2], &d[0], &d[1], &d[2], &radius, &ca, &la, &qa) != 13) {
        fprintf(stderr, "Unable to read area light at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Normalize direction
      d.Normalize();

      // Create spot light
      R3AreaLight *light = new R3AreaLight(p, radius, d, c, 1, TRUE, ca, la, qa);
      scene->InsertLight(light);
    }
    else if (!strcmp(cmd, "camera")) {
      // Read data
      R3Point e;
      R3Vector t, u;
      RNScalar xfov, neardist, fardist;
      if (fscanf(fp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", &e[0], &e[1], &e[2], &t[0], &t[1], &t[2], &u[0], &u[1], &u[2], &xfov, &neardist, &fardist) != 12) {
        fprintf(stderr, "Unable to read camera at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Assign camera
      R3Camera camera(e, t, u, xfov, xfov, neardist, fardist);
      scene->SetCamera(camera);
    }
    else if (!strcmp(cmd, "include")) {
      // Read data
      char scenename[256];
      if (fscanf(fp, "%s", scenename) != 1) {
        fprintf(stderr, "Unable to read include command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Get scene filename
      char buffer[2048];
      strcpy(buffer, filename);
      char *bufferp = strrchr(buffer, '/');
      if (bufferp) *(bufferp+1) = '\0';
      else buffer[0] = '\0';
      strcat(buffer, scenename);

      // Read scene from included file
      if (!ReadPrinceton(scene, group_nodes[depth], buffer)) {
        fprintf(stderr, "Unable to read included scene: %s\n", buffer);
        return 0;
      }
    }
    else if (!strcmp(cmd, "background")) {
      // Read data
      double r, g, b;
      if (fscanf(fp, "%lf%lf%lf", &r, &g, &b) != 3) {
        fprintf(stderr, "Unable to read background at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Assign background color
      scene->SetBackground(RNRgb(r, g, b));
    }
    else if (!strcmp(cmd, "ambient")) {
      // Read data
      double r, g, b;
      if (fscanf(fp, "%lf%lf%lf", &r, &g, &b) != 3) {
        fprintf(stderr, "Unable to read ambient at command %d in file %s\n", command_number, filename);
        return 0;
      }

      // Assign ambient color
      scene->SetAmbient(RNRgb(r, g, b));
    }
    else {
      fprintf(stderr, "Unrecognized command %d in file %s: %s\n", command_number, filename, cmd);
      return 0;
    }
	
    // Increment command number
    command_number++;
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3Scene::
ReadPrincetonFile(const char *filename)
{
  // Read princeton file and insert contents into root node
  return ReadPrinceton(this, root, filename);
}



//////////////

static int
CreateTextureFile(const R3Scene *scene, const R2Texture *texture, const char *output_directory)
{
  // Return success
  return 1;
}



static int
WritePrincetonMaterial(const R3Scene *scene, const R3Material *material, FILE *fp)
{
  // Write material
  const R3Brdf *brdf = material->Brdf();
  const R2Texture *texture = material->Texture();
  RNRgb ambient = (brdf) ? brdf->Ambient() : RNblack_rgb;
  RNRgb diffuse = (brdf) ? brdf->Diffuse() : RNblack_rgb;
  RNRgb specular = (brdf) ? brdf->Specular() : RNblack_rgb;
  RNRgb transmission = (brdf) ? brdf->Transmission() : RNblack_rgb;
  RNRgb emission = (brdf) ? brdf->Emission() : RNblack_rgb;
  RNScalar shininess = brdf->Shininess();
  RNScalar indexofref = brdf->IndexOfRefraction();
  const char *texture_name = (texture && texture->Name()) ? texture->Name() : "0";
  fprintf(fp, "material %g %g %g  %g %g %g  %g %g %g  %g %g %g  %g %g %g  %g %g %s\n",
    ambient.R(), ambient.G(), ambient.B(),
    diffuse.R(), diffuse.G(), diffuse.B(),
    specular.R(), specular.G(), specular.B(),
    transmission.R(), transmission.G(), transmission.B(),
    emission.R(), emission.G(), emission.B(),
    shininess, indexofref, texture_name);

  // Return success
  return 1;
}



static int
WritePrincetonLight(const R3Scene *scene, const R3Light *light, FILE *fp)
{
  // Get light color
  RNRgb color = light->Color();
  
  // Write light in format appropriate for type
  if (light->ClassID() == R3DirectionalLight::CLASS_ID()) {
    R3DirectionalLight *directional_light = (R3DirectionalLight *) light;
    const R3Vector& direction = directional_light->Direction();
    fprintf(fp, "dir_light  %g %g %g  %g %g %g\n",
            color.R(), color.G(), color.B(),
            direction.X(), direction.Y(), direction.Z());
  }
  else if (light->ClassID() == R3PointLight::CLASS_ID()) {
    R3PointLight *point_light = (R3PointLight *) light;
    const R3Point& position = point_light->Position();
    RNScalar ca = point_light->ConstantAttenuation();
    RNScalar la = point_light->LinearAttenuation();
    RNScalar qa = point_light->QuadraticAttenuation();
    fprintf(fp, "point_light  %g %g %g  %g %g %g  %g %g %g\n",
            color.R(), color.G(), color.B(),
            position.X(), position.Y(), position.Z(),
            ca, la, qa);
  }
  else if (light->ClassID() == R3SpotLight::CLASS_ID()) {
    R3SpotLight *spot_light = (R3SpotLight *) light;
    const R3Point& position = spot_light->Position();
    const R3Vector& direction = spot_light->Direction();
    RNScalar ca = spot_light->ConstantAttenuation();
    RNScalar la = spot_light->LinearAttenuation();
    RNScalar qa = spot_light->QuadraticAttenuation();
    RNScalar sc = spot_light->CutOffAngle();
    RNScalar sd = spot_light->DropOffRate();
    fprintf(fp, "spot_light  %g %g %g  %g %g %g  %g %g %g  %g %g %g  %g %g\n",
            color.R(), color.G(), color.B(),
            position.X(), position.Y(), position.Z(),
            direction.X(), direction.Y(), direction.Z(),
            ca, la, qa, sc, sd);
  }
  else if (light->ClassID() == R3AreaLight::CLASS_ID()) {
    R3AreaLight *area_light = (R3AreaLight *) light;
    const R3Point& position = area_light->Position();
    const R3Vector& direction = area_light->Direction();
    RNScalar radius = area_light->Radius();
    RNScalar ca = area_light->ConstantAttenuation();
    RNScalar la = area_light->LinearAttenuation();
    RNScalar qa = area_light->QuadraticAttenuation();
    fprintf(fp, "area_light  %g %g %g  %g %g %g  %g %g %g  %g  %g %g %g\n",
            color.R(), color.G(), color.B(),
            position.X(), position.Y(), position.Z(),
            direction.X(), direction.Y(), direction.Z(),
            radius, ca, la, qa);
  }
  else {
    // fprintf("Unrecognized light type\n");
    // return 0;
  }

  // Return success
  return 1;
}



static int
WritePrincetonElement(const R3Scene *scene, R3SceneElement *element,
  const R3Affine& transformation, FILE *fp, const char *indent)
{
  // Get material index
  R3Material *material = element->Material();
  int material_index = (material) ? material->SceneIndex() : -1;
  
  // Write shapes
  for (int i = 0; i < element->NShapes(); i++) {
    R3Shape *shape = element->Shape(i);
    if (shape->ClassID() == R3Box::CLASS_ID()) {
      R3Box *box = (R3Box *) shape;
      fprintf(fp, "%sbox  %d   %g %g %g   %g %g %g\n", indent, material_index,
        box->XMin(), box->YMin(), box->ZMin(), box->XMax(), box->YMax(), box->ZMax());
    }
    else if (shape->ClassID() == R3Sphere::CLASS_ID()) {
      R3Sphere *sphere = (R3Sphere *) shape;
      R3Point center = sphere->Center();
      fprintf(fp, "%ssphere  %d   %g %g %g   %g\n", indent, material_index,
        center.X(), center.Y(), center.Z(), sphere->Radius());
    }
    else if (shape->ClassID() == R3Cone::CLASS_ID()) {
      R3Cone *cone = (R3Cone *) shape;
      R3Point center = cone->Axis().Midpoint();
      if (RNIsNotEqual(cone->Axis().Vector().Dot(R3posz_vector), 1.0)) fprintf(stderr, "Warning: cone not axis aligned\n");
      fprintf(fp, "%scone  %d   %g %g %g   %g %g\n", indent, material_index,
        center.X(), center.Y(), center.Z(), cone->Radius(), cone->Height());
    }
    else if (shape->ClassID() == R3Cylinder::CLASS_ID()) {
      R3Cylinder *cylinder = (R3Cylinder *) shape;
      R3Point center = cylinder->Axis().Midpoint();
      if (RNIsNotEqual(cylinder->Axis().Vector().Dot(R3posz_vector), 1.0)) fprintf(stderr, "Warning: cylinder not axis aligned\n");
      fprintf(fp, "%scylinder  %d   %g %g %g   %g %g\n", indent, material_index,
        center.X(), center.Y(), center.Z(), cylinder->Radius(), cylinder->Height());
    }
    else if (shape->ClassID() == R3Triangle::CLASS_ID()) {
      R3Triangle *triangle = (R3Triangle *) shape;
      R3TriangleVertex *v0 = (transformation.IsMirrored()) ? triangle->V2() : triangle->V0();
      R3TriangleVertex *v1 = triangle->V1();
      R3TriangleVertex *v2 = (transformation.IsMirrored()) ? triangle->V0() : triangle->V2();
      const R3Point& p0 = v0->Position();
      const R3Point& p1 = v1->Position();
      const R3Point& p2 = v2->Position();
      fprintf(fp, "%stri  %d   %g %g %g   %g %g %g   %g %g %g\n", indent, material_index,
        p0.X(), p0.Y(), p0.Z(), p1.X(), p1.Y(), p1.Z(), p2.X(), p2.Y(), p2.Z());
    }
    else if (shape->ClassID() == R3TriangleArray::CLASS_ID()) {
      R3TriangleArray *array = (R3TriangleArray *) shape;
      for (int j = 0; j < array->NTriangles(); j++) {
        R3Triangle *triangle = array->Triangle(j);
        R3TriangleVertex *v0 = (transformation.IsMirrored()) ? triangle->V2() : triangle->V0();
        R3TriangleVertex *v1 = triangle->V1();
        R3TriangleVertex *v2 = (transformation.IsMirrored()) ? triangle->V0() : triangle->V2();
        const R3Point& p0 = v0->Position();
        const R3Point& p1 = v1->Position();
        const R3Point& p2 = v2->Position();
        fprintf(fp, "%stri  %d   %g %g %g   %g %g %g   %g %g %g\n", indent, material_index,
          p0.X(), p0.Y(), p0.Z(), p1.X(), p1.Y(), p1.Z(), p2.X(), p2.Y(), p2.Z());
      }
    }
    else {
      fprintf(stderr, "Warning: unrecognized shape type %d\n", shape->ClassID());
      // return 0;
    }
  }
  
  // Return success
  return 1;
}



static int
WritePrincetonNode(const R3Scene *scene, const R3SceneNode *node,
  const R3Transformation& parent_transformation,
  FILE *fp, const char *indent)
{
  // Compute indent
  int nindent = strlen(indent);
  char *child_indent = new char [nindent + 3];
  strcpy(child_indent, indent);
  child_indent[nindent] = ' ';
  child_indent[nindent+1] = ' ';
  child_indent[nindent+2] = '\0';

  // Compute transformation
  R3Affine transformation = R3identity_affine;
  transformation.Transform(parent_transformation);
  transformation.Transform(node->Transformation());

  // Don't write root separately, unless transformed
  RNBoolean print_node = (node != scene->Root()) || (!node->Transformation().IsIdentity());
  if (print_node) {
    // Write begin
    fprintf(fp, "\n");
    fprintf(fp, "%sbegin -1\n", indent);

    // Write node transformation
    const R4Matrix& m = node->Transformation().Matrix();
    fprintf(fp, "%s%g %g %g %g\n", child_indent, m[0][0], m[0][1], m[0][2], m[0][3]);
    fprintf(fp, "%s%g %g %g %g\n", child_indent, m[1][0], m[1][1], m[1][2], m[1][3]);
    fprintf(fp, "%s%g %g %g %g\n", child_indent, m[2][0], m[2][1], m[2][2], m[2][3]);
    fprintf(fp, "%s%g %g %g %g\n", child_indent, m[3][0], m[3][1], m[3][2], m[3][3]);
  }
  
  // Write elements
  for (int i = 0; i < node->NElements(); i++) {
    R3SceneElement *element = node->Element(i);
    if (!WritePrincetonElement(scene, element, transformation, fp, child_indent)) return 0;
  }

  // Write children
  for (int i = 0; i < node->NChildren(); i++) {
    R3SceneNode *child = node->Child(i);
    if (!WritePrincetonNode(scene, child, transformation, fp, child_indent)) return 0;
  }

  // Write end
  if (print_node) fprintf(fp, "%send\n", indent);

  // Delete child indent buffer
  delete [] child_indent;

  // Return success
  return 1;
}



int R3Scene::
WritePrincetonFile(const char *filename) const
{
  // Open file
  FILE *fp;
  if (!(fp = fopen(filename, "w"))) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Determine output directory
  const char *output_directory = ".";

  // Write color and camera info
  fprintf(fp, "background %g %g %g\n", Background().R(), Background().G(), Background().B());
  fprintf(fp, "ambient %g %g %g\n", Ambient().R(), Ambient().G(), Ambient().B());
  fprintf(fp, "camera %g %g %g  %g %g %g  %g %g %g  %g   %g %g  \n",
    Camera().Origin().X(), Camera().Origin().Y(), Camera().Origin().Z(),
    Camera().Towards().X(), Camera().Towards().Y(), Camera().Towards().Z(),
    Camera().Up().X(), Camera().Up().Y(), Camera().Up().Z(),
    Camera().XFOV(), Camera().Near(), Camera().Far());

  // Create textures files
  for (int i = 0; i < NTextures(); i++) {
    R2Texture *texture = Texture(i);
    if (!CreateTextureFile(this, texture, output_directory)) return 0;
  }

  // Write lights
  fprintf(fp, "\n");
  for (int i = 0; i < NLights(); i++) {
    R3Light *light = Light(i);
    if (!WritePrincetonLight(this, light, fp)) return 0;
  }

  // Write materials
  fprintf(fp, "\n");
  for (int i = 0; i < NMaterials(); i++) {
    R3Material *material = Material(i);
    if (!WritePrincetonMaterial(this, material, fp)) return 0;
  }

  // Write nodes recursively
  int status = WritePrincetonNode(this, root, R3identity_affine, fp, "");
  
  // Close file
  fclose(fp);

  // Return status
  return status;
}



////////////////////////////////////////////////////////////////////////
// SUPPORT HIERARCHY FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3Scene::
ReadSupportHierarchyFile(const char *filename)
{
  // Open file
  FILE *fp;
  if (!(fp = fopen(filename, "r"))) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Read scene
  char buffer[1024];
  int line_count = 0;
  RNArray<R3SceneNode *> nodes;
  while (fgets(buffer, 1023, fp)) {
    // Increment line counter
    line_count++;

    // Skip white space
    char *bufferp = buffer;
    while (isspace(*bufferp)) bufferp++;

    // Skip blank lines and comments
    if (*bufferp == '#') continue;
    if (*bufferp == '\0') continue;

    // Get keyword
    char keyword[256];
    if (sscanf(bufferp, "%s", keyword) != 1) {
      fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
      return 0;
    }

    // Check keyword
    if (!strcmp(keyword, "newModel")) {
      // Read fields
      int model_index;
      char model_name[1024];
      if (sscanf(bufferp, "%s%d%s", keyword, &model_index, model_name) != (unsigned int) 3) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Create node
      assert(nodes.NEntries() == model_index);
      R3SceneNode *node = new R3SceneNode(this);
      node->SetName(model_name);
      root->InsertChild(node);
      nodes.Insert(node);

      // Read obj file
      char model_filename[1024];
      sprintf(model_filename, "models/%s.obj", model_name);
      if (!ReadObj(this, node, model_filename)) return 0;
    }
    else if (!strcmp(keyword, "parentIndex")) {
      // Read fields
      int parent_index;
      if (sscanf(bufferp, "%s%d", keyword, &parent_index) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Check parent index
      if (parent_index < 0) {
        if (nodes.NEntries() != 1) {
          fprintf(stderr, "Root node was not first in file %s", filename);
          return 0;
        }
      }
      else {
        // Just checking
        if (parent_index >= nodes.NEntries()) {
          fprintf(stderr, "Invalid parent node index %d on line %d in file %s", parent_index, line_count, filename);
          return 0;
        }

        // Set last node's parent
        R3SceneNode *node = nodes.Tail();
        R3SceneNode *parent = nodes.Kth(parent_index);
        R3SceneNode *previous_parent = node->Parent();
        if (parent != previous_parent) {
          if (previous_parent) previous_parent->RemoveChild(node);
          if (parent) parent->InsertChild(node);
        }
      }
    }
    else if (!strcmp(keyword, "transform")) {
      // Read fields
      double m[16];
      if (sscanf(bufferp, "%s%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf", 
        keyword, &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], 
        &m[8], &m[9], &m[10], &m[11], &m[12], &m[13], &m[14], &m[15]) != (unsigned int) 17) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // NOTE: Our transformation is stored relative to parent's coordinate system
      // So, we must compute transformation from inverse of transform from parent node to world coordinates so that can 
      // convert file's absolute transform (which goes from node's coordinates to world coordinates)
      // to our transform (which goes from node's coordinates to parent node's coordinates)
      R3Affine transformation = R3identity_affine;
      R3SceneNode *node = nodes.Tail();
      R3SceneNode *ancestor = node->Parent();
      while (ancestor) {
        transformation.InverseTransform(ancestor->Transformation());
        ancestor = ancestor->Parent();
      }

      // Set last node's transformation
      // Note that file's matrix is for post-multiplication, while ours is for pre-multiplication, so need flip
      R4Matrix matrix(m); matrix.Flip();
      transformation.Transform(R3Affine(matrix, 0));
      node->SetTransformation(transformation);
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3Scene::
WriteSupportHierarchyFile(const char *filename) const
{
  // Not implemented yet
  RNAbort("Not implemented");

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// GRAMMAR HIERARCHY FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3Scene::
ReadGrammarHierarchyFile(const char *filename)
{
  // Open file
  FILE *fp;
  if (!(fp = fopen(filename, "r"))) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Extract base name
  char basename[4096] = { '\0' };
  const char *startp = strrchr(filename, '/');
  if (!startp) startp = filename;
  else startp++;
  const char *endp = strrchr(filename, '.');
  if (!endp) endp = startp + strlen(startp);
  int basename_length = endp - startp;
  if (basename_length > 4095) basename_length = 4095;
  strncpy(basename, startp, basename_length);

  // Read scene
  char buffer[1024];
  int line_count = 0;
  RNBoolean leaf = TRUE;
  RNArray<R3SceneNode *> parsed_nodes;
  R3SceneNode *current_node = NULL;
  while (fgets(buffer, 1023, fp)) {
    // Increment line counter
    line_count++;

    // Skip white space
    char *bufferp = buffer;
    while (isspace(*bufferp)) bufferp++;

    // Skip blank lines and comments
    if (*bufferp == '#') continue;
    if (*bufferp == '\0') continue;

    // Get keyword
    char keyword[256];
    if (sscanf(bufferp, "%s", keyword) != 1) {
      fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
      return 0;
    }

    // Check keyword
    if (!strcmp(keyword, "newModel")) {
      // Read fields
      int model_index;
      if (sscanf(bufferp, "%s%d", keyword, &model_index) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Create node
      while (parsed_nodes.NEntries() <= model_index) {
        char node_name[4096];
        sprintf(node_name, "%d", parsed_nodes.NEntries());
        R3SceneNode *node = new R3SceneNode(this);
        node->SetName(node_name);
        parsed_nodes.Insert(node);
      }

      // Remember current node
      current_node = parsed_nodes.Kth(model_index);
    }
    else if (!strcmp(keyword, "root")) {
      // Read fields
      int root_index;
      if (sscanf(bufferp, "%s%d", keyword, &root_index) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Create node
      while (parsed_nodes.NEntries() <= root_index) {
        char node_name[4096];
        sprintf(node_name, "%d", parsed_nodes.NEntries());
        R3SceneNode *node = new R3SceneNode(this);
        node->SetName(node_name);
        parsed_nodes.Insert(node);
      }

      // Insert root of this parse as child of root
      R3SceneNode *node = parsed_nodes.Kth(root_index);
      root->InsertChild(node);
    }
    else if (!strcmp(keyword, "parent")) {
      // Read fields
      int parent_index;
      if (sscanf(bufferp, "%s%d", keyword, &parent_index) != (unsigned int) 2) {
        fprintf(stderr, "Syntax error on line %d in file %s", line_count, filename);
        return 0;
      }

      // Check parent index
      if (parent_index >= 0) {
        // Create parent node
        while (parsed_nodes.NEntries() <= parent_index) {
          char node_name[4096];
          sprintf(node_name, "I%d", parsed_nodes.NEntries());
          R3SceneNode *node = new R3SceneNode(this);
          node->SetName(node_name);
          parsed_nodes.Insert(node);
        }
      
        // Set last node's parent
        R3SceneNode *parent = parsed_nodes.Kth(parent_index);
        parent->InsertChild(current_node);
      }
    }
    else if (!strcmp(keyword, "children")) {
      const char *token = strtok(bufferp, " \n\t");
      assert(token && (!strcmp(token, "children")));
      token = strtok(NULL, " \n\t");
      leaf = (token) ? FALSE : TRUE;
    }
    else if (!strcmp(keyword, "leaf_group")) {
      // Check current node
      if (!current_node) {
        fprintf(stderr, "leaf_group before first newModel at line %d in %s\n", line_count, filename);
        return 0;
      }

      // Read models
      if (leaf) {
        const char *token = strtok(bufferp, " \n\t");
        assert(token && (!strcmp(token, "leaf_group")));
        while (TRUE) {
          token = strtok(NULL, " \n\t");
          if (!token) break;
          int model_index = atoi(token);
          // char node_name[4096];
          // sprintf(node_name, "L%d", model_index);
          // R3SceneNode *node = new R3SceneNode(this);
          // node->SetName(node_name);
          // current_node->InsertChild(node);
          char model_filename[1024];
          sprintf(model_filename, "models/%s/%d.off", basename, model_index);
          R3TriangleArray *shape = ReadMesh(model_filename);
          if (shape) {
            R3SceneElement *element = new R3SceneElement();
            element->SetMaterial(&R3default_material);
            element->InsertShape(shape);
            current_node->InsertElement(element);
          }
        }    
      }    
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3Scene::
WriteGrammarHierarchyFile(const char *filename) const
{
  // Not implemented yet
  RNAbort("Not implemented");

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// PARSE FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3Scene::
ReadParseFile(const char *filename)
{
  // Open file
  FILE *fp;
  if (!(fp = fopen(filename, "r"))) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Read header
  char buffer[4096];
  if (!fgets(buffer, 4096, fp)) {
    fprintf(stderr, "Unable to read object parse file %s\n", filename);
    return 0;
  }

  // Check header
  if (strncmp(buffer, "OBJECT PARSE 1.0", 16)) {
    fprintf(stderr, "Error in header of oject parse file %s\n", filename);
    return 0;
  }

  // Read file
  int line_number = 0;
  int assignment_index = 0;
  RNArray<R3Shape *> shapes;
  char mesh_directory[4096] = { '.', '\0' };
  while (fgets(buffer, 4096, fp)) {
    // Check line
    line_number++;
    char *bufferp = buffer;
    while (*bufferp && isspace(*bufferp)) bufferp++;
    if (!bufferp) continue;
    if (*bufferp == '#') continue;

    // Parse line
    char keyword[4096];
    if (sscanf(bufferp, "%s", keyword) == (unsigned int) 1) {
      if (!strcmp(keyword, "A")) {
        // Parse assignment
        double score, m[16];
        int segmentation_index, model_index, dummy;
        if (sscanf(bufferp, "%s%d%d%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%d%d%d%d", keyword, 
          &segmentation_index, &model_index, 
          &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7], 
          &m[8], &m[9], &m[10], &m[11], &m[12], &m[13], &m[14], &m[15], 
          &score, &dummy, &dummy, &dummy, &dummy) != (unsigned int) 24) {
          fprintf(stderr, "Error parsing assignment at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Create node
        R3SceneNode *node = new R3SceneNode(this);
        root->InsertChild(node);

        // Create shape element
        R3Shape *shape = shapes.Kth(model_index);
        R3Brdf *brdf = new R3Brdf(RNRgb(0, 0.25 + score, 0), 0.0, 0.25 + score);
        InsertBrdf(brdf);
        R3Material *material = new R3Material(brdf);
        InsertMaterial(material);
        R3SceneElement *element = new R3SceneElement();
        element->SetMaterial(material);
        element->InsertShape(shape);
        node->InsertElement(element);

        // Set node name
        char node_name[1024];
        sprintf(node_name, "A%d_M%d_S%03d", assignment_index++, model_index, (int) (1000*score));
        node->SetName(node_name);

        // Set node transformation
        R4Matrix matrix(m); 
        R3Affine affine(matrix, 0);
        node->SetTransformation(affine);
      }
      else if (!strcmp(keyword, "M")) {
        // Parse model
        int dummy;
        double cx, cy, cz, r, h;
        char model_name[4096], mesh_name[4096];
        if (sscanf(bufferp, "%s%d%lf%lf%lf%lf%lf%s%s%d%d%d%d%d", keyword, 
          &dummy, &cx, &cy, &cz, &r, &h, model_name, mesh_name, &dummy, &dummy, &dummy, &dummy, &dummy) != (unsigned int) 14) {
          fprintf(stderr, "Error parsing model at line %d of %s\n", line_number, filename);
          return 0;
        }

        // Read mesh
        char mesh_filename[4096];
        R3Shape *shape = NULL;
        if (strcmp(mesh_name, "None")) {
          sprintf(mesh_filename, "%s/%s", mesh_directory, mesh_name);
          shape = ReadMesh(mesh_filename);
          if (!shape) return 0;
        }
        else {
          shape = new R3Sphere(R3Point(0,0,0), 0);
        }

        // Insert shape into list
        shapes.Insert(shape);
      }
      else if (!strcmp(keyword, "MD")) {
        // Parse model directory
        if (sscanf(bufferp, "%s%s", keyword, mesh_directory) != (unsigned int) 2) {
          fprintf(stderr, "Error parsing model directory at line %d of %s\n", line_number, filename);
          return 0;
        }
      }
    }
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}



int R3Scene::
WriteParseFile(const char *filename) const
{
  // Not implemented yet
  RNAbort("Not implemented");

  // Return success
  return 1;
}



////////////////////////////////////////////////////////////////////////
// RECTANGLE FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

int R3Scene::
ReadRectangleFile(const char *filename)
{
  // Open file
  FILE *fp;
  if (!(fp = fopen(filename, "r"))) {
    fprintf(stderr, "Unable to open file %s", filename);
    return 0;
  }

  // Read file
  char buffer[4096];
  int line_number = 0;
  int assignment_index = 0;
  while (fgets(buffer, 4096, fp)) {
    // Check line
    line_number++;
    char *bufferp = buffer;
    while (*bufferp && isspace(*bufferp)) bufferp++;
    if (!bufferp) continue;
    if (*bufferp == '#') continue;

    // Parse line
    char name[4096];
    double x1, y1, x2, y2, x3, y3, x4, y4, zmin, zmax, score;
    if (sscanf(bufferp, "%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%s", 
       &x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4, &zmin, &zmax, &score, name) != (unsigned int) 12) {
      fprintf(stderr, "Error parsing line %d of %s\n", line_number, filename);
      return 0;
    }

    // Create node
    R3SceneNode *node = new R3SceneNode(this);
    root->InsertChild(node);

    // Create shape element
    RNArray<R3Triangle *> triangles;
    RNArray<R3TriangleVertex *> vertices;
    R3TriangleVertex *v1 = new R3TriangleVertex(R3Point(x1, y1, zmin)); vertices.Insert(v1);
    R3TriangleVertex *v2 = new R3TriangleVertex(R3Point(x2, y2, zmin)); vertices.Insert(v2);
    R3TriangleVertex *v3 = new R3TriangleVertex(R3Point(x3, y3, zmin)); vertices.Insert(v3);
    R3TriangleVertex *v4 = new R3TriangleVertex(R3Point(x4, y4, zmin)); vertices.Insert(v4);
    R3Triangle *t1 = new R3Triangle(v1, v2, v3); triangles.Insert(t1);
    R3Triangle *t2 = new R3Triangle(v1, v3, v4); triangles.Insert(t2);
    R3TriangleArray *base = new R3TriangleArray(vertices, triangles);
    R3Point base_centroid = base->BBox().Centroid();
    R3Point top_centroid = base_centroid + (zmax - zmin) * R3posz_vector;
    R3Cylinder *marker = new R3Cylinder(base_centroid, top_centroid, 0.01 * base->BBox().DiagonalRadius());
    R3Brdf *brdf = new R3Brdf(RNRgb(0, 0.25 + score, 0));
    InsertBrdf(brdf);
    R3Material *material = new R3Material(brdf);
    InsertMaterial(material);
    R3SceneElement *element = new R3SceneElement();
    element->SetMaterial(material);
    element->InsertShape(base);
    element->InsertShape(marker);
    node->InsertElement(element);
    
    // Set node name
    char node_name[1024];
    sprintf(node_name, "A%d_S%03d", assignment_index++, (int) (1000*score));
    node->SetName(node_name);
  }

  // Close file
  fclose(fp);

  // Return success
  return 1;
}




////////////////////////////////////////////////////////////////////////
// PLANNER5D FILE I/O FUNCTIONS
////////////////////////////////////////////////////////////////////////

#include "p5d.h"
#if (RN_OS != RN_WINDOWS)
#   include <unistd.h>
#endif



static int
ParseP5DColor(const char *str, RNRgb& rgb)
{
  // Parse color
  char buffer[1024];
  strncpy(buffer, str, 1024);
  if (buffer[0] == '#') {
    long int b = strtol(&buffer[5], NULL, 16); buffer[5] = '\0';
    long int g = strtol(&buffer[3], NULL, 16); buffer[3] = '\0';
    long int r = strtol(&buffer[1], NULL, 16); buffer[1] = '\0';
    rgb.Reset(r / 255.0, g / 255.0, b / 255.0);
    return 1;
  }
  else if (!strncmp(buffer, "rgb(", 4)) {
    char *bufferp = strtok(buffer, "(,)");
    bufferp = strtok(NULL, "(,)");
    int r = (bufferp) ? atoi(bufferp) : 127;
    bufferp = strtok(NULL, "(,)");
    int g = (bufferp) ? atoi(bufferp) : 127;
    bufferp = strtok(NULL, "(,)");
    int b = (bufferp) ? atoi(bufferp) : 127;
    rgb.Reset(r / 255.0, g / 255.0, b / 255.0);
    return 1;
  }

  // Color was not parsed
  return 0;
}



static int
ReplaceP5DObjectMaterials(R3Scene *scene, R3SceneNode *node, const char *objname,
  const RNArray<R3Material *>& materials, P5DObject *object)
{
#if 0
  // Determine directory name (for texture image files)
  char dirname[1024] = { '.', '/', '\0' };
  if (objname) {
    // Get base directory name
    strncpy(dirname, objname, 1024);
    for (int i = 0; i < 3; i++) {
      char *endp = strrchr(dirname, '/');
      if (!endp) endp = strrchr(dirname, '\\');
      if (!endp) { strcpy(dirname, "."); break; }
      else *endp = '\0';
    }

    // Get texture directory
    strncat(dirname, "/texture", 1024);
  }
#else
  const char *dirname = "../../texture";
#endif
  
  // Process every material associated with object
  for (unsigned int i = 0; i < object->materials.size(); i++) {
    P5DMaterial *p5d_material = object->materials[i];
    if (!p5d_material->name) continue;
    if ((int) i >= materials.NEntries()) break;
    R3Material *material = materials.Kth(i);

    // Replace color
    if (p5d_material->color) {
      RNRgb rgb(0.5, 0.5, 0.5);
      if (ParseP5DColor(p5d_material->color, rgb)) {
        R3Brdf *brdf = (R3Brdf *) material->Brdf();
        if (!brdf) { brdf = new R3Brdf(rgb); scene->InsertBrdf(brdf); material->SetBrdf(brdf); }
        else { brdf->SetAmbient(rgb); brdf->SetDiffuse(rgb); }
      }
    }
    
    // Replace texture image
    if (p5d_material->texture_name) {
      // Replace color
      if (p5d_material->tcolor) {
        RNRgb rgb(0.5, 0.5, 0.5);
        if (ParseP5DColor(p5d_material->tcolor, rgb)) {
          R3Brdf *brdf = (R3Brdf *) material->Brdf();
          if (!brdf) { brdf = new R3Brdf(rgb); scene->InsertBrdf(brdf); material->SetBrdf(brdf); }
          else { brdf->SetAmbient(rgb); brdf->SetDiffuse(rgb); }
        }
      }
    
      // Get texture image file
      char texture_filename[1024];
      if (dirname) sprintf(texture_filename, "%s/%s.jpg", dirname, p5d_material->texture_name);
      else sprintf(texture_filename, "%s.jpg", p5d_material->texture_name);
      if (!RNFileExists(texture_filename)) {
        if (dirname) sprintf(texture_filename, "%s/%s.png", dirname, p5d_material->texture_name);
        else sprintf(texture_filename, "%s.png", p5d_material->texture_name);
      }

      // Update texture image
      R2Texture *texture = (R2Texture *) material->Texture();
      R2Image *image = (texture) ? (R2Image *) texture->Image() : NULL; 
      if (!texture) { texture = new R2Texture(); scene->InsertTexture(texture); material->SetTexture(texture); }
      if (!image) { image = new R2Image(); texture->SetImage(image); }
      if (!image->Read(texture_filename)) return 0; 
      texture->SetName(texture_filename);
      texture->SetImage(image);
    }

    // Update material
    material->Update();
  }

  // Return success
  return 1;
}
  




int R3Scene::
ReadPlanner5DFile(const char *filename)
{
  // Get current working directory
  char current_working_directory[1024];
  if (!getcwd(current_working_directory, 1024)) {
    fprintf(stderr, "Unable to get current directory\n");
    return 0;
  }

  // Get/parse full path name to json file
  char full_path_name[1024];
  if (filename[0] == '/') strncpy(full_path_name, filename, 1024);
  else sprintf(full_path_name, "%s/%s", current_working_directory, filename);

  // Get data directory and 
  char input_data_directory[1024];
  strncpy(input_data_directory, full_path_name, 1024);
  char *endp = strrchr(input_data_directory, '/');
  if (endp) *endp = '\0';
  endp = strrchr(input_data_directory, '/');
  if (endp) *endp = '\0';
  endp = strrchr(input_data_directory, '/');
  if (endp) *endp = '\0';

  // Get project name
  char input_project_name[1024], tmp[1024];
  strncpy(tmp, full_path_name, 1024);
  endp = strrchr(tmp, '/');
  if (endp) *endp = '\0';
  char *startp = strrchr(tmp, '/');
  if (startp) startp++;
  else startp = tmp;
  strncpy(input_project_name, startp, 1024);
  
  // Allocate project
  P5DProject *project = new P5DProject();
  if (!project) {
    fprintf(stderr, "Unable to allocate project for %s\n", filename);
    return 0;
  }

  // Set project name
  project->name = strdup(input_project_name);

  // Read project from file
  if (!project->ReadFile(filename)) {
    delete project;
    return 0;
  }

  // Get root node 
  char project_name[1024];
  sprintf(project_name, "Project#%s", (project->name) ? project->name : "");
  R3SceneNode *root_node = Root();
  root_node->SetName(project_name);
  root_node->SetData(project);

  // Mirror the world to compensate for swap of Y and Z axes in code than generates the json and obj files
  R4Matrix xmirror_matrix(-1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
  R3Affine xmirror_transformation(xmirror_matrix, TRUE);
  root_node->SetTransformation(xmirror_transformation);
  
  // Create nodes for floors
  RNScalar floor_z = 0;
  for (int i = 0; i < project->NFloors(); i++) {
    P5DFloor *floor = project->Floor(i);

    // Create floor node
    char floor_name[1024];
    sprintf(floor_name, "Floor#%d_%d", i+1, floor->idx_index+1);
    R3SceneNode *floor_node = new R3SceneNode(this);
    floor_node->SetName(floor_name);
    floor_node->SetData(floor);
    floor->data = floor_node;
    root_node->InsertChild(floor_node);

    // Set floor transformation
    R3Affine floor_transformation(R3identity_affine);
    floor_transformation.Translate(R3Vector(0, 0, floor_z));
    floor_node->SetTransformation(floor_transformation);
    floor_z += floor->h;

    // Create nodes for rooms
    for (int j = 0; j < floor->NRooms(); j++) {
      P5DRoom *room = floor->Room(j);

      // Create room node
      char room_name[1024];
      sprintf(room_name, "Room#%d_%d_%d", i+1, j+1, room->idx_index+1);
      R3SceneNode *room_node = new R3SceneNode(this);
      room_node->SetName(room_name);
      room_node->SetData(room);
      room->data = room_node;
      floor_node->InsertChild(room_node);

      // Check if enclosed room
      if (!strcmp(room->className, "Room")) {
        // Read walls
        char rm_name[4096], node_name[4096];
        sprintf(rm_name, "%s/roomfiles/%s/fr_%drm_%d.obj", input_data_directory, project->name, i+1, room->idx_index+1); 
        if (RNFileExists(rm_name)) {
          R3SceneNode *wall_node = new R3SceneNode(this);
          sprintf(node_name, "Walls#%d_%d_%d", i+1, j+1, room->idx_index+1);
          wall_node->SetName(node_name);
          if (!ReadObj(this, wall_node, rm_name)) return 0;
          room_node->InsertChild(wall_node);
        }

        // Read floor
        sprintf(rm_name, "%s/roomfiles/%s/fr_%drm_%df.obj", input_data_directory, project->name, i+1, room->idx_index+1); 
        if (RNFileExists(rm_name)) {
          R3SceneNode *rmfloor_node = new R3SceneNode(this);
          sprintf(node_name, "Floors#%d_%d_%d", i+1, j+1, room->idx_index+1);
          rmfloor_node->SetName(node_name);
          if (!ReadObj(this, rmfloor_node, rm_name)) return 0;
          room_node->InsertChild(rmfloor_node);
        }
        
        // Read ceiling
        sprintf(rm_name, "%s/roomfiles/%s/fr_%drm_%dc.obj", input_data_directory, project->name, i+1, room->idx_index+1); 
        if (RNFileExists(rm_name)) {
          R3SceneNode *rmceil_node = new R3SceneNode(this);
          sprintf(node_name, "Ceilings#%d_%d_%d", i+1, j+1, room->idx_index+1);
          rmceil_node->SetName(node_name);
          if (!ReadObj(this, rmceil_node, rm_name)) return 0;
          room_node->InsertChild(rmceil_node);
        }
      }
    }

    // Create nodes for objects
    for (int j = 0; j < floor->NObjects(); j++) {
      P5DObject *object = floor->Object(j);
      if (!object->id || !(*(object->id))) continue;
      P5DRoom *room = object->room;
      
      // Create object node
      char object_name[1024];
      int ri = (room) ? room->floor_index+1 : -1;
      int oi = (room) ? object->room_index+1 : -1;
      sprintf(object_name, "Object#%d_%d_%d_%d_%s_%s", i+1, ri, oi, object->idx_index+1, object->className, object->id);
      R3SceneNode *object_node = new R3SceneNode(this);
      object_node->SetName(object_name);
      object_node->SetData(object);
      object->data = object_node;

      // Insert into room or floor parent node
      R3SceneNode *parent_node = (object->room) ? floor_node->Child(object->room->floor_index) : floor_node;
      parent_node->InsertChild(object_node);

      // Set object transformation
      R3Affine object_transformation(R3identity_affine);
      object_transformation.Translate(R3Vector(object->x, object->y, object->z));
      object_transformation.ZRotate(object->a);
      object_transformation.Scale(R3Vector(object->sX, object->sY, object->sZ));
      if (!strcmp(object->className, "Door")) object_transformation.ZRotate(RN_PI_OVER_TWO);
      else if (!strcmp(object->className, "Window")) object_transformation.ZRotate(RN_PI_OVER_TWO);
      if (object->fX) object_transformation.XMirror();
      if (object->fY) object_transformation.YMirror();
      object_node->SetTransformation(object_transformation);

      // Read obj file
      char obj_name[4096];
      RNArray<R3Material *> materials;
      if (object->aframe) sprintf(obj_name, "%s/objects/%s/%s_0.obj", input_data_directory, object->id, object->id);
      else sprintf(obj_name, "%s/objects/%s/%s.obj", input_data_directory, object->id, object->id);
      if (!ReadObj(this, object_node, obj_name, &materials)) return 0;

      // Replace materials with ones stored in P5D object
      if (!ReplaceP5DObjectMaterials(this, object_node, obj_name, materials, object)) return 0;
    }
  }
    
  // Return success
  return 1;
}




