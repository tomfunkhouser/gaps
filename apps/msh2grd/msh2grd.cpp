// Source file for the mesh to grid conversion program



// Include files 

#include "R3Shapes/R3Shapes.h"



// Program variables

static char *mesh_name = NULL;
static char *grid_name = NULL;
static int grid_resolution[3] = { 64, 64, 64 };
static int print_verbose = 0;



static R3Mesh *
ReadMesh(char *mesh_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate mesh
  R3Mesh *mesh = new R3Mesh();
  assert(mesh);

  // Read mesh from file
  if (!mesh->ReadFile(mesh_name)) {
    delete mesh;
    return NULL;
  }

  // Print statistics
  if (print_verbose) {
    printf("Read mesh ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Faces = %d\n", mesh->NFaces());
    printf("  # Edges = %d\n", mesh->NEdges());
    printf("  # Vertices = %d\n", mesh->NVertices());
    fflush(stdout);
  }

  // Return success
  return mesh;
}



static R3Grid *
CreateGrid(R3Mesh *mesh)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Allocate grid
  R3Grid *grid = new R3Grid(grid_resolution[0], grid_resolution[1], grid_resolution[2]);
  if (!grid) {
    fprintf(stderr, "Unable to allocate grid\n");
    exit(-1);
  }

  // Compute mesh center of mass
  RNScalar mesh_area = 0.0;
  R3Point mesh_centroid = R3zero_point;
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    RNArea area = mesh->FaceArea(face);
    mesh_centroid += area * mesh->FaceCentroid(face);
    mesh_area += area;
  }
  mesh_centroid /= mesh_area;

  // Compute average distance to center of mass
  RNScalar avg_distance = 0.0;
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    RNArea area = mesh->FaceArea(face);
    avg_distance += area * R3Distance(mesh->FaceCentroid(face), mesh_centroid);
  }
  avg_distance /= mesh_area;

  // Compute grid center
  int res = grid_resolution[0];
  if (res < grid_resolution[1]) res = grid_resolution[1];
  if (res < grid_resolution[2]) res = grid_resolution[2];
  R3Vector grid_diagonal(grid_resolution[0]-1, grid_resolution[1]-1, grid_resolution[2]-1);
  R3Vector grid_centroid = 0.5 * grid_diagonal;

  // Compute scale factor
  RNScalar scale = (0.5 * (res-1)) / (2.0 * avg_distance);

  // Set affine transformation
  R3Affine world_to_grid = R3identity_affine;
  world_to_grid.Translate(grid_centroid);
  world_to_grid.Scale(scale);
  world_to_grid.Translate(-(mesh_centroid.Vector()));
  grid->SetWorldToGridTransformation(world_to_grid);

  // Rasterize each triangle into grid
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    const R3Point& p0 = mesh->VertexPosition(mesh->VertexOnFace(face, 0));
    const R3Point& p1 = mesh->VertexPosition(mesh->VertexOnFace(face, 1));
    const R3Point& p2 = mesh->VertexPosition(mesh->VertexOnFace(face, 2));
    grid->RasterizeWorldTriangle(p0, p1, p2, 1.0);
  }

  // Print statistics
  if (print_verbose) {
    printf("Created grid ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Voxels = %d %d% d\n", grid->XResolution(), grid->YResolution(), grid->ZResolution());
    printf("  Grid Center = %g %g %g\n", grid_centroid[0], grid_centroid[1], grid_centroid[2]);
    printf("  World Center = %g %g %g\n", mesh_centroid[0], mesh_centroid[1], mesh_centroid[2]);
    printf("  World Area = %g\n", mesh_area);
    printf("  Average Distance = %g\n", avg_distance);
    printf("  World To Grid Scale Factor = %g\n", scale);
    printf("  Maximum = %g\n", grid->Maximum());
    printf("  L2Norm = %g\n", grid->L2Norm());
    fflush(stdout);
  }

  // Return grid
  return grid;
}



static int 
WriteGrid(R3Grid *grid, const char *grid_name)
{
  // Start statistics
  RNTime start_time;
  start_time.Read();

  // Write grid
  int status = grid->WriteFile(grid_name);

  // Print statistics
  if (print_verbose) {
    printf("Wrote grid ...\n");
    printf("  Time = %.2f seconds\n", start_time.Elapsed());
    printf("  # Bytes = %d\n", status * (int) sizeof(RNScalar));
    fflush(stdout);
  }

  // Return status
  return status;
}



static int 
ParseArgs(int argc, char **argv)
{
  // Check number of arguments
  if (argc < 3) {
    printf("Usage: off2grd offfile gridfile [-resolution x y z] [-v]\n");
    exit(0);
  }

  // Parse arguments
  argc--; argv++;
  while (argc > 0) {
    if ((*argv)[0] == '-') {
      if (!strcmp(*argv, "-v")) { 
        print_verbose = 1; 
      }
      else if (!strcmp(*argv, "-resolution")) { 
        argc--; argv++; grid_resolution[0] = atoi(*argv); 
        argc--; argv++; grid_resolution[1] = atoi(*argv); 
        argc--; argv++; grid_resolution[2] = atoi(*argv); 
      }
      else { 
        fprintf(stderr, "Invalid program argument: %s", *argv); 
        exit(1); 
      }
    }
    else {
      if (!mesh_name) mesh_name = *argv;
      else if (!grid_name) grid_name = *argv;
      else { 
        fprintf(stderr, "Invalid program argument: %s", *argv); 
        exit(1); 
      }
    }
    argv++; argc--;
  }

  // Check mesh filename
  if (!mesh_name) {
    fprintf(stderr, "You did not specify a mesh file.\n");
    return 0;
  }

  // Check grid filename
  if (!grid_name) {
    fprintf(stderr, "You did not specify a grid file.\n");
    return 0;
  }

  // Check grid resolution
  if ((grid_resolution[0] <= 0) || (grid_resolution[1] <= 0) || (grid_resolution[2] <= 0)) {
    fprintf(stderr, "Invalid grid resolution: %d %d %d\n", grid_resolution[0], grid_resolution[1], grid_resolution[2]);
    return 0;
  }

  // Return OK status 
  return 1;
}



int 
main(int argc, char **argv)
{
  // Parse program arguments
  if (!ParseArgs(argc, argv)) exit(-1);

  // Read mesh file
  R3Mesh *mesh = ReadMesh(mesh_name);
  if (!mesh) exit(-1);

  // Create grid from mesh
  R3Grid *grid = CreateGrid(mesh);
  if (!grid) exit(-1);

  // Write grid
  int status = WriteGrid(grid, grid_name);
  if (!status) exit(-1);

  // Return success
  return 0;
}
