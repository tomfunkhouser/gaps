/* Source file for the R3 triangle class */



/* Include files */

#include "R3Shapes.h"



// Namespace

namespace gaps {



/* Class type definitions */

RN_CLASS_TYPE_DEFINITIONS(R3TriangleArray);



/* Public functions */

int 
R3InitTriangleArray()
{
    /* Return success */
    return TRUE;
}



void 
R3StopTriangleArray()
{
}



R3TriangleArray::
R3TriangleArray(void)
  : bbox(R3null_box),
    flags(0),
    vbo_id(0),
    vbo_size(0)
{
}



R3TriangleArray::
R3TriangleArray(const R3TriangleArray& array)
  : vertices(),
    triangles(),
    bbox(array.bbox),
    flags(array.flags),
    vbo_id(0),
    vbo_size(0)
{
    // Copy vertices
    for (int i = 0; i < array.vertices.NEntries(); i++) {
      R3TriangleVertex *oldv = array.vertices.Kth(i);
      R3TriangleVertex *newv = new R3TriangleVertex(*oldv);
      vertices.Insert(newv);
      newv->SetSharedFlag();
      oldv->SetMark(i);
    }

    // Copy triangles
    for (int i = 0; i < array.triangles.NEntries(); i++) {
      R3Triangle *oldt = array.triangles.Kth(i);
      R3TriangleVertex *v0 = vertices.Kth(oldt->V0()->Mark());
      R3TriangleVertex *v1 = vertices.Kth(oldt->V1()->Mark());
      R3TriangleVertex *v2 = vertices.Kth(oldt->V2()->Mark());
      R3Triangle *newt = new R3Triangle(v0, v1, v2);
      triangles.Insert(newt);
    }

    // Update everything
    Update();
}



R3TriangleArray::
R3TriangleArray(const RNArray<R3TriangleVertex *>& vertices, const RNArray<R3Triangle *>& triangles)
  : vertices(vertices),
    triangles(triangles),
    bbox(R3null_box),
    vbo_id(0),
    vbo_size(0)
{
    // Update everything
    Update();
}



R3TriangleArray::
~R3TriangleArray(void)
{
    // Delete triangles and vertices
    for (int i = 0; i < triangles.NEntries(); i++) delete triangles[i];
    for (int i = 0; i < vertices.NEntries(); i++) delete vertices[i];

    // Delete vbo
    if (vbo_id > 0) glDeleteBuffers(1, &vbo_id);
}



const RNBoolean R3TriangleArray::
IsPoint (void) const
{
  // Check if all vertices are at same position
  if (vertices.NEntries() > 1) {
    const R3Point& p0 = vertices[0]->Position();
    for (int i = 1; i < vertices.NEntries(); i++) 
      if (!R3Contains(p0, vertices[i]->Position())) return FALSE;
  }
  return TRUE;
}



const RNBoolean R3TriangleArray::
IsLinear (void) const
{
    // Assume not ???
    return FALSE;
}



const RNBoolean R3TriangleArray::
IsPlanar(void) const
{
  // Check if all triangles are on same plane
  if (triangles.NEntries() > 1) {
    const R3Plane& p0 = triangles[0]->Plane();
    for (int i = 1; i < triangles.NEntries(); i++) 
      if (!R3Contains(p0, triangles[i]->Plane())) return FALSE;
  }
  return TRUE;
}



const RNBoolean R3TriangleArray::
IsConvex(void) const
{
    // If planar - may still be concave ???
    return IsPlanar();
}



const RNInterval R3TriangleArray::
NFacets(void) const
{
    // Return number of trianglets (triangles)
    return RNInterval(triangles.NEntries(), triangles.NEntries());
}



const RNLength R3TriangleArray::
Length (void) const
{
    // Return cumulative perimeter of triangles
    RNLength length = 0.0;
    for (int i = 0; i < triangles.NEntries(); i++)
      length += triangles[i]->Length();
    return length;
}



const RNArea R3TriangleArray::
Area(void) const
{
    // Return cumulative area of triangles
    RNArea area = 0.0;
    for (int i = 0; i < triangles.NEntries(); i++)
      area += triangles[i]->Area();
    return area;
}



const R3Point R3TriangleArray::
Centroid(void) const
{
    // Return centroid of vertices
    R3Point centroid = R3zero_point;
    for (int i = 0; i < vertices.NEntries(); i++) 
      centroid += vertices[i]->Position();
    centroid /= (RNScalar) vertices.NEntries();
    return centroid;
}



const R3Point R3TriangleArray::
ClosestPoint(const R3Point& point) const
{
    // Find closest point
    R3Point closest_point = R3zero_point;
    RNLength closest_dd = RN_INFINITY * RN_INFINITY;
    for (int i = 0; i < triangles.NEntries(); i++) {
      R3Triangle *triangle = triangles.Kth(i);
      R3Point p = triangle->ClosestPoint(point);
      RNLength dd = R3SquaredDistance(p, point);
      if (dd < closest_dd) {
        closest_point = p;
        closest_dd = dd;
      }
    }
        
    // Return closest point
    return closest_point;
}



const R3Point R3TriangleArray::
FurthestPoint(const R3Point& point) const
{
    // Find furthest point
    R3Point furthest_point = R3zero_point;
    RNLength furthest_dd = 0;
    for (int i = 0; i < vertices.NEntries(); i++) {
      R3TriangleVertex *vertex = vertices.Kth(i);
      R3Point p = vertex->Position();
      RNLength dd = R3SquaredDistance(p, point);
      if (dd > furthest_dd) {
        furthest_point = p;
        furthest_dd = dd;
      }
    }

    // Return furthest point
    return furthest_point;
}



const R3Shape& R3TriangleArray::
BShape(void) const
{
    // Return bounding shape
    return bbox;
}



const R3Box R3TriangleArray::
BBox(void) const
{
    // Return bounding box
    return bbox;
}



const R3Sphere R3TriangleArray::
BSphere(void) const
{
    // Return bounding sphere
    return bbox.BSphere();
}



R3TriangleArray& R3TriangleArray::
operator=(const R3TriangleArray& array)
{
    // Delete previous triangles and vertices
    for (int i = 0; i < triangles.NEntries(); i++) delete triangles[i];
    for (int i = 0; i < vertices.NEntries(); i++) delete vertices[i];
    triangles.Empty();
    vertices.Empty();

    // Copy vertices
    for (int i = 0; i < array.vertices.NEntries(); i++) {
      R3TriangleVertex *oldv = array.vertices.Kth(i);
      R3TriangleVertex *newv = new R3TriangleVertex(*oldv);
      vertices.Insert(newv);
      newv->SetSharedFlag();
      oldv->SetMark(i);
    }

    // Copy triangles
    for (int i = 0; i < array.triangles.NEntries(); i++) {
      R3Triangle *oldt = array.triangles.Kth(i);
      R3TriangleVertex *v0 = vertices.Kth(oldt->V0()->Mark());
      R3TriangleVertex *v1 = vertices.Kth(oldt->V1()->Mark());
      R3TriangleVertex *v2 = vertices.Kth(oldt->V2()->Mark());
      R3Triangle *newt = new R3Triangle(v0, v1, v2);
      triangles.Insert(newt);
    }

    // Copy bbox
    bbox = array.bbox;

    // Update everything
    Update();

    // Initialize vbo variables
    vbo_id = 0;
    vbo_size = 0;
    
    // Return this
    return *this;
}



void R3TriangleArray::
Flip (void)
{
    // Flip the triangles
    for (int i = 0; i < triangles.NEntries(); i++)
      triangles[i]->Flip();

    // Update
    Update();
}



void R3TriangleArray::
Mirror (const R3Plane& plane)
{
    // Mirror vertices
    for (int i = 0; i < vertices.NEntries(); i++) 
      vertices[i]->Mirror(plane);

    // Update the triangles
    for (int i = 0; i < triangles.NEntries(); i++)
      triangles[i]->Update();

    // Update the bounding box
    Update();
}



void R3TriangleArray::
Transform (const R3Transformation& transformation) 
{
    // Transform vertices
    for (int i = 0; i < vertices.NEntries(); i++) 
      vertices[i]->Transform(transformation);

    // Update the triangles
    for (int i = 0; i < triangles.NEntries(); i++)
      triangles[i]->Update();

    // Check if need to flip triangles
    if (transformation.IsMirrored()) Flip();

    // Update the bounding box
    Update();
}



void R3TriangleArray::
Subdivide(RNLength max_edge_length) 
{
  // Get convenient variables
  RNLength max_edge_length_squared = max_edge_length * max_edge_length;
  
  // Subdivide triangles with edges that are too long
  RNArray<R3Triangle *> stack = triangles;
  while (!stack.IsEmpty()) {
    // Get triangle from stack
    R3Triangle *triangle = stack.Tail();
    stack.RemoveTail();

    // Get vertices
    R3TriangleVertex *v0 = triangle->V0();
    R3TriangleVertex *v1 = triangle->V1();
    R3TriangleVertex *v2 = triangle->V2();
    const R3Point& p0 = v0->Position();
    const R3Point& p1 = v1->Position();
    const R3Point& p2 = v2->Position();
    RNLength dd01 = R3SquaredDistance(p0, p1);
    RNLength dd12 = R3SquaredDistance(p1, p2);
    RNLength dd20 = R3SquaredDistance(p2, p0);

    // Check if edge is too long
    if ((dd01 > max_edge_length_squared) && (dd01 >= dd12) && (dd01 >= dd20)) {
      // Create new vertex at midpoint of edge between v0 and v1
      R3Point position = 0.5*(p0 + p1);
      R3Vector normal = 0.5*(v0->Normal() + v1->Normal()); normal.Normalize();
      R3TriangleVertex *v = new R3TriangleVertex(position, normal);
      if (v0->flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG] && v1->flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG])
        v->SetTextureCoords(0.5*(v0->TextureCoords() + v1->TextureCoords()));
      vertices.Insert(v);

      // Adjust existing triangle
      triangle->Reset(v0, v, v2);
      stack.Insert(triangle);

      // Create new triangle
      R3Triangle *t = new R3Triangle(v1, v2, v);
      triangles.Insert(t);
      stack.Insert(t);
    }
    else if ((dd12 > max_edge_length_squared) && (dd12 >= dd01) && (dd12 >= dd20)) {
      // Create new vertex at midpoint of edge between v1 and v2
      R3Point position = 0.5*(p1 + p2);
      R3Vector normal = 0.5*(v1->Normal() + v2->Normal()); normal.Normalize();
      R3TriangleVertex *v = new R3TriangleVertex(position, normal);
      if (v1->flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG] && v2->flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG])
        v->SetTextureCoords(0.5*(v1->TextureCoords() + v2->TextureCoords()));
      vertices.Insert(v);

      // Adjust existing triangle
      triangle->Reset(v1, v, v0);
      stack.Insert(triangle);

      // Create new triangle
      R3Triangle *t = new R3Triangle(v2, v0, v);
      triangles.Insert(t);
      stack.Insert(t);
    }
    else if ((dd20 > max_edge_length_squared) && (dd20 >= dd01) && (dd20 >= dd12)) {
      // Create new vertex at midpoint of edge between v2 and v0
      R3Point position = 0.5*(p2 + p0);
      R3Vector normal = 0.5*(v2->Normal() + v0->Normal()); normal.Normalize();
      R3TriangleVertex *v = new R3TriangleVertex(position, normal);
      if (v2->flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG] && v0->flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG])
        v->SetTextureCoords(0.5*(v2->TextureCoords() + v0->TextureCoords()));
      vertices.Insert(v);

      // Adjust existing triangle
      triangle->Reset(v2, v, v1);
      stack.Insert(triangle);

      // Create new triangle
      R3Triangle *t = new R3Triangle(v0, v1, v);
      triangles.Insert(t);
      stack.Insert(t);
    }
  }

  // Update
  Update();
}



void R3TriangleArray::
CreateVertexNormals(RNAngle max_angle)
{
  // Create data for computing vertex normals
  R3Vector *normals = new R3Vector [ vertices.NEntries() ];
  RNBoolean *smooth = new RNBoolean [ vertices.NEntries() ];
  for (int i = 0; i < vertices.NEntries(); i++) {
    R3TriangleVertex *vertex = vertices.Kth(i);
    normals[i] = R3zero_vector;
    smooth[i] = TRUE;
    vertex->SetMark(i);
  }

  // Aggregate info for vertex normals
  for (int i = 0; i < triangles.NEntries(); i++) {
    R3Triangle *triangle = triangles.Kth(i);
    RNArea weight = triangle->Area();
    const R3Vector& normal = triangle->Normal();
    for (int j = 0; j < 3; j++) {
      R3TriangleVertex *vertex = triangle->Vertex(j);
      int index = vertex->Mark();
      normals[index] += weight * normal;
    }
  }

  // Normalize vertex normals
  for (int i = 0; i < vertices.NEntries(); i++) {
    normals[i].Normalize();
  }

  // Mark smooth vertices
  RNScalar min_dot = (max_angle < RN_PI) ? cos(max_angle) : -1;
  for (int i = 0; i < triangles.NEntries(); i++) {
    R3Triangle *triangle = triangles.Kth(i);
    const R3Vector& normal = triangle->Normal();
    for (int j = 0; j < 3; j++) {
      R3TriangleVertex *vertex = triangle->Vertex(j);
      int index = vertex->Mark();
      if (normal.Dot(normals[index]) > min_dot) continue;
      smooth[index] = FALSE;
    }
  }
        
  // Update vertex normals
  for (int i = 0; i < vertices.NEntries(); i++) {
    R3TriangleVertex *vertex = vertices.Kth(i);
    if (vertex->HasNormal()) continue;
    if (normals[i].IsZero()) continue;
    if (!smooth[i]) continue;
    vertex->SetNormal(normals[i]);
  }

  // Update triangle flags
  for (int i = 0; i < triangles.NEntries(); i++) {
    R3Triangle *triangle = triangles.Kth(i);
    triangle->UpdateFlags();
  }

  // Delete temporary data
  delete [] normals;
  delete [] smooth;

  // Update
  Update();
}



void R3TriangleArray::
MoveVertex(R3TriangleVertex *vertex, const R3Point& position)
{
    // Move vertex
    vertex->SetPosition(position);

    // Update triangles
    for (int i = 0; i < triangles.NEntries(); i++) {
      R3Triangle *triangle = triangles.Kth(i);
      for (int j = 0; j < 3; j++) {
        if (triangle->Vertex(i) == vertex) {
          triangle->Update();
          break;
        }
      }
    }

    // Update
    Update();
}



void R3TriangleArray::
Update(void)
{
  // Mark vertices as shared
  for (int i = 0; i < vertices.NEntries(); i++) {
    R3TriangleVertex *v = vertices.Kth(i);
    v->SetSharedFlag();
  }
    
  // Recompute bounding box
  bbox = R3null_box;
  for (int i = 0; i < vertices.NEntries(); i++) {
    R3TriangleVertex *v = vertices.Kth(i);
    bbox.Union(v->Position());
  }

  // Recompute flags
  flags.Add(R3_EVERYTHING_DRAW_FLAGS);
  for (int i = 0; i < vertices.NEntries(); i++) {
    R3TriangleVertex *v = vertices.Kth(i);
    RNFlags vertex_flags = v->Flags();
    if (!(vertex_flags[R3_VERTEX_NORMALS_DRAW_FLAG]))
      flags.Remove(R3_VERTEX_NORMALS_DRAW_FLAG);
    if (!(vertex_flags[R3_VERTEX_COLORS_DRAW_FLAG]))
      flags.Remove(R3_VERTEX_COLORS_DRAW_FLAG);
    if (!(vertex_flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG]))
      flags.Remove(R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG);
  }

  // Invalidate VBO
  InvalidateVBO();
}



void R3TriangleArray::
Draw(const R3DrawFlags draw_flags) const
{
#if 0 && (RN_3D_GRFX == RN_OPENGL)
  // Draw surfaces
  if (draw_flags[R3_SURFACES_DRAW_FLAG]) {
    DrawVBO(draw_flags);
  }

  // Draw edges
  if (draw_flags[R3_EDGES_DRAW_FLAG]) {
    RNGrfxBegin(RN_GRFX_LINES);
    for (int i = 0; i < NTriangles(); i++) {
      R3Triangle *triangle = Triangle(i);
      for (int j = 0; j < 3; j++) {
        R3TriangleVertex *v0 = triangle->Vertex(j);
        R3TriangleVertex *v1 = triangle->Vertex((j+1)%3);
        R3LoadPoint(v0->Position());
        R3LoadPoint(v1->Position());
      }
    }
    RNGrfxEnd();
  }
#else
    // Draw all triangles
    for (int i = 0; i < triangles.NEntries(); i++)
        triangles.Kth(i)->Draw(draw_flags);
#endif
}



void R3TriangleArray::
InvalidateVBO(void)
{
  // Mark vbo as out of date
  vbo_size = 0;
}



void R3TriangleArray::
UpdateVBO(void)
{
#if (RN_3D_GRFX == RN_OPENGL)
  // Check if VBO is uptodate
  if (vbo_size > 0) return;
  
  // Check triangle array
  if (NVertices() == 0) return;
  if (NTriangles() == 0) return;

  // Allocate buffer data
  static const unsigned int vertex_size = 12 * sizeof(GLdouble);
  unsigned int buffer_size = 3 * NTriangles() * vertex_size;
  GLdouble *buffer_data = new GLdouble [ buffer_size ];
  if (!buffer_data) return;
  
  // Fill vertex data 
  GLdouble *buffer_datap = buffer_data;
  for (int i = 0; i < NTriangles(); i++) {
    R3Triangle *triangle = Triangle(i);
    int dim = triangle->Normal().MaxDimension();
    int dim1 = (dim + 1) % 3;
    int dim2 = (dim + 2) % 3;
    for (int j = 0; j < 3; j++) {
      R3TriangleVertex *vertex = triangle->Vertex(j);

      // Load position
      *(buffer_datap++) = vertex->Position().X();
      *(buffer_datap++) = vertex->Position().Y();
      *(buffer_datap++) = vertex->Position().Z();

      // Load normal
      if (triangle->HasNormals() && vertex->HasNormal()) {
        R3Vector normal = vertex->Normal();
        RNScalar dot = normal.Dot(triangle->Normal());
        if (dot < -0.25) normal = -normal;
        else if (dot < 0.25) normal = triangle->Normal();
        *(buffer_datap++) = normal.X();
        *(buffer_datap++) = normal.Y();
        *(buffer_datap++) = normal.Z();
      }
      else {
        *(buffer_datap++) = triangle->Normal().X();
        *(buffer_datap++) = triangle->Normal().Y();
        *(buffer_datap++) = triangle->Normal().Z();
      }

      // Load color      
      if (triangle->HasColors() && vertex->HasColor()) {
        *(buffer_datap++) = vertex->Color().R();
        *(buffer_datap++) = vertex->Color().G();
        *(buffer_datap++) = vertex->Color().B();
      }
      else {
        *(buffer_datap++) = 0.5;
        *(buffer_datap++) = 0.5;
        *(buffer_datap++) = 0.5;
      }

      // Load texture coordinates
      if (triangle->HasTextureCoords() && vertex->HasTextureCoords()) {
        *(buffer_datap++) = vertex->TextureCoords().X();
        *(buffer_datap++) = vertex->TextureCoords().Y();
        *(buffer_datap++) = 0;
      }
      else {
        *(buffer_datap++) = vertex->Position()[dim1];
        *(buffer_datap++) = vertex->Position()[dim2];
        *(buffer_datap++) = 0;
      }
    }
  }

  // Just checking
  assert((long int) (buffer_datap - buffer_data) == (long int) (buffer_size / sizeof(GLdouble)));

  // Generate VBO buffer (first time only)
  if (vbo_id == 0) glGenBuffers(1, &vbo_id);

  // Load VBO buffer
  if (vbo_id && buffer_data && buffer_size) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, buffer_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    vbo_size = buffer_size;
  }

  // Delete in-memory data
  if (buffer_data) delete [] buffer_data;
#endif
}



void R3TriangleArray::
DrawVBO(const R3DrawFlags draw_flags) const
{
#if (RN_3D_GRFX == RN_OPENGL)
  // Update vbo
  ((R3TriangleArray *) this)->UpdateVBO();

  // Check vbo
  if (vbo_id == 0) return;
  if (vbo_size == 0) return;

  // Bind vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

  // Set pointers
  static const unsigned int vertex_size = 12 * sizeof(GLdouble);
  glVertexPointer(3, GL_DOUBLE, vertex_size, (char *) NULL);
  glNormalPointer(GL_DOUBLE, vertex_size, (char *) NULL + 3 * sizeof(GLdouble));
  glColorPointer(3, GL_DOUBLE, vertex_size, (char *) NULL + 6 * sizeof(GLdouble));
  glTexCoordPointer(2, GL_DOUBLE, vertex_size, (char *) NULL + 9 * sizeof(GLdouble));

  // Enable client states
  R3DrawFlags flags(Flags() & draw_flags);
  glEnableClientState(GL_VERTEX_ARRAY);
  if (flags[R3_SURFACE_NORMALS_DRAW_FLAG] || flags[R3_VERTEX_NORMALS_DRAW_FLAG])
    glEnableClientState(GL_NORMAL_ARRAY);
  else glDisable(GL_NORMAL_ARRAY);
  if (flags[R3_VERTEX_COLORS_DRAW_FLAG])
    glEnableClientState(GL_COLOR_ARRAY);
  else glDisable(GL_COLOR_ARRAY);
  if (flags[R3_SURFACE_TEXTURE_DRAW_FLAG] || flags[R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG])
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  else glDisable(GL_TEXTURE_COORD_ARRAY);
  
  // Draw triangles
  int nvertices = vbo_size / vertex_size;
  assert(nvertices == 3 * NTriangles());
  glDrawArrays(GL_TRIANGLES, 0, nvertices);
  
  // Disable client states
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

#endif
}



} // namespace gaps
