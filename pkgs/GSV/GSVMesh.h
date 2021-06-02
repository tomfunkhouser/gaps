// GSV mesh class definitions
#ifndef __GSV__MESH__H__
#define __GSV__MESH__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////////////////

#define GSVMeshEdge R3MeshEdge
#define GSVMeshFace R3MeshFace

class GSVMeshVertex : public R3MeshVertex {
  private:
    GSVMeshVertex(void);
    virtual ~GSVMeshVertex(void);

  private:
    friend class GSVMesh;
    GSVScanline *scanline;
    int point_index;
    int boundary_type;
};



// Mesh class definition

class GSVMesh : public R3Mesh {
  public:
    // Constructor/destructor
    GSVMesh(void);
    virtual ~GSVMesh(void);

    // GSV-specific vertex property functions
    GSVScanline *VertexScanline(R3MeshVertex *vertex) const;
    RNScalar VertexTimestamp(R3MeshVertex *vertex) const;
    RNScalar VertexHeight(R3MeshVertex *vertex) const;
    RNScalar VertexLaserAngle(R3MeshVertex *vertex) const;
    RNScalar VertexLaserDepth(R3MeshVertex *vertex) const;
    RNScalar VertexLaserDistance(R3MeshVertex *vertex) const;
    R3Point VertexLaserViewpoint(R3MeshVertex *vertex) const;
    R3Vector VertexLaserTowards(R3MeshVertex *vertex) const;
    R3Vector VertexLaserUp(R3MeshVertex *vertex) const;
    int VertexScanlineIndex(R3MeshVertex *vertex) const;
    int VertexPointIndex(R3MeshVertex *vertex) const;
    int VertexBoundaryType(R3MeshVertex *vertex) const; 

    // GSV manipulation functions
    int LoadScan(GSVScan *scan, RNLength min_viewpoint_movement = 0.05, RNLength max_depth_discontinuity = 1);

    // File input/output functions
    virtual int ReadPlyFile(GSVScan *scan, const char *filename);
    virtual int WritePlyFile(GSVScan *scan, const char *filename, RNBoolean binary = TRUE) const;
    virtual int WriteARFFFile(GSVScan *scan, const char *filename) const;

public:
    // R3Mesh vertex access functions (overridden to return GSVMeshVertex)
    GSVMeshVertex *Vertex(int k) const;
    GSVMeshVertex *VertexOnVertex(const R3MeshVertex *vertex) const;
    GSVMeshVertex *VertexOnVertex(const R3MeshVertex *vertex, int k) const;
    GSVMeshVertex *VertexOnEdge(const R3MeshEdge *edge) const;
    GSVMeshVertex *VertexOnEdge(const R3MeshEdge *edge, int k) const;
    GSVMeshVertex *VertexAcrossEdge(const R3MeshEdge *edge, const R3MeshVertex *vertex) const;
    GSVMeshVertex *VertexBetweenEdges(const R3MeshEdge *edge1, const R3MeshEdge *edge2) const;
    GSVMeshVertex *VertexOnEdge(const R3MeshEdge *edge, const R3MeshFace *face, RNDirection dir = RN_CCW) const;
    GSVMeshVertex *VertexOnFace(const R3MeshFace *face) const;
    GSVMeshVertex *VertexOnFace(const R3MeshFace *face, int k) const;
    GSVMeshVertex *VertexOnFace(const R3MeshFace *face, const R3MeshVertex *vertex, RNDirection dir = RN_CCW) const;
    GSVMeshVertex *VertexOnFace(const R3MeshFace *face, const R3MeshEdge *edge, RNDirection dir = RN_CCW) const;
    GSVMeshVertex *VertexAcrossFace(const R3MeshFace *face, const R3MeshEdge *edge) const;
    GSVMeshVertex *VertexBetweenFaces(const R3MeshFace *face1, const R3MeshFace *face2, RNDirection dir = RN_CCW) const;

    // R3Mesh manipulation functions (overridden to return GSVMeshVertex)
    virtual GSVMeshVertex *MergeVertex(R3MeshVertex *v1, R3MeshVertex *v2);
    virtual GSVMeshVertex *CollapseEdge(R3MeshEdge *edge, const R3Point& point);
    virtual GSVMeshVertex *CollapseFace(R3MeshFace *face, const R3Point& point);
    virtual GSVMeshVertex *CollapseEdge(R3MeshEdge *edge);
    virtual GSVMeshVertex *CollapseFace(R3MeshFace *face);
    virtual GSVMeshVertex *SplitEdge(R3MeshEdge *edge, const R3Point& point, R3MeshEdge **e0 = NULL, R3MeshEdge **e1 = NULL);
    virtual GSVMeshVertex *SplitEdge(R3MeshEdge *edge, const R3Plane& plane);
    virtual GSVMeshVertex *SubdivideEdge(R3MeshEdge *edge);
    virtual GSVMeshVertex *SplitFace(R3MeshFace *face, const R3Point& point, 
      R3MeshFace **f0 = NULL, R3MeshFace **f1 = NULL, R3MeshFace **f2 = NULL);

    // R3Mesh vertex creation functions (overridden to create GSVMeshVertex)
    virtual R3MeshVertex *CreateVertex(const R3Point& position, R3MeshVertex *vertex = NULL);
    virtual R3MeshVertex *CreateVertex(const R3Point& position, const R3Vector& normal, R3MeshVertex *vertex = NULL);

    // R3Mesh intersection functions (overridden to take advantage of search tree)
    virtual R3MeshType Intersection(const R3Ray& ray, R3MeshIntersection *intersection = NULL) const;
    virtual R3MeshType Intersection(const R3Ray& ray, R3MeshIntersection *intersection, 
      RNScalar min_t, RNScalar max_t = RN_INFINITY,
      int (*IsCompatible)(const R3Point&, const R3Vector&, R3Mesh *, R3MeshFace *, void *) = NULL, 
      void *compatible_data = NULL) const;
    virtual R3MeshType ClosestPoint(const R3Point& query, R3MeshIntersection *closest = NULL,
      RNScalar min_distance = 0, RNScalar max_distance = RN_INFINITY,
      int (*IsCompatible)(const R3Point&, const R3Vector&, R3Mesh *, R3MeshFace *, void *) = NULL, 
      void *compatible_data = NULL);
    virtual R3MeshType Intersection(const R3Ray& ray, GSVMeshFace *face, R3MeshIntersection *intersection = NULL) const;

    // R3Mesh Draw functions (overridden to include texture coordinates)
    virtual void Draw(RNBoolean texture_coordinates) const;
    virtual void Draw(void) const;

    // Input/output files (overridden to include GSVScan)
    virtual int ReadPlyFile(const char *filename);
    virtual int WritePlyFile(const char *filename, RNBoolean binary = TRUE) const;
    virtual int WriteARFFFile(const char *filename) const;

  protected:  
    GSVMeshVertex *gsv_vertex_block;
    R3MeshSearchTree *search_tree;
};



////////////////////////////////////////////////////////////////////////
// Boundary types
////////////////////////////////////////////////////////////////////////

enum {
  GSV_MESH_NO_BOUNDARY,
  GSV_MESH_LEFT_BOUNDARY,
  GSV_MESH_RIGHT_BOUNDARY,
  GSV_MESH_BOTTOM_BOUNDARY,
  GSV_MESH_TOP_BOUNDARY,
  GSV_MESH_SHADOW_BOUNDARY,
  GSV_MESH_SILHOUETTE_BOUNDARY
};



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline GSVMeshVertex *GSVMesh::
Vertex(int k) const
{
  // Return kth vertex
  return (GSVMeshVertex *) R3Mesh::Vertex(k);
}



inline GSVScanline *GSVMesh::
VertexScanline(R3MeshVertex *v) const
{
  // Return vertex point index
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  return vertex->scanline;
}



inline int GSVMesh::
VertexPointIndex(R3MeshVertex *v) const
{
  // Return vertex point index
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  return vertex->point_index;
}



inline int GSVMesh::
VertexBoundaryType(R3MeshVertex *v) const
{
  // Return vertex boundary type
  GSVMeshVertex *vertex = (GSVMeshVertex *) v;
  return vertex->boundary_type;
}


inline GSVMeshVertex *GSVMesh::
VertexOnVertex(const R3MeshVertex *vertex) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnVertex(vertex);
}



inline GSVMeshVertex *GSVMesh::
VertexOnVertex(const R3MeshVertex *vertex, int k) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnVertex(vertex, k);
}



inline GSVMeshVertex *GSVMesh::
VertexOnEdge(const R3MeshEdge *edge) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnEdge(edge);
}



inline GSVMeshVertex *GSVMesh::
VertexOnEdge(const R3MeshEdge *edge, int k) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnEdge(edge, k);
}



inline GSVMeshVertex *GSVMesh::
VertexAcrossEdge(const R3MeshEdge *edge, const R3MeshVertex *vertex) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexAcrossEdge(edge, vertex);
}



inline GSVMeshVertex *GSVMesh::
VertexBetweenEdges(const R3MeshEdge *edge1, const R3MeshEdge *edge2) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexBetweenEdges(edge1, edge2);
}



inline GSVMeshVertex *GSVMesh::
VertexOnEdge(const R3MeshEdge *edge, const R3MeshFace *face, RNDirection dir) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnEdge(edge, face, dir);
}



inline GSVMeshVertex *GSVMesh::
VertexOnFace(const R3MeshFace *face) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnFace(face);
}



inline GSVMeshVertex *GSVMesh::
VertexOnFace(const R3MeshFace *face, int k) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnFace(face, k);
}



inline GSVMeshVertex *GSVMesh::
VertexOnFace(const R3MeshFace *face, const R3MeshVertex *vertex, RNDirection dir) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnFace(face, vertex, dir);
}



inline GSVMeshVertex *GSVMesh::
VertexOnFace(const R3MeshFace *face, const R3MeshEdge *edge, RNDirection dir) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexOnFace(face, edge, dir);
}



inline GSVMeshVertex *GSVMesh::
VertexAcrossFace(const R3MeshFace *face, const R3MeshEdge *edge) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexAcrossFace(face, edge);
}



inline GSVMeshVertex *GSVMesh::
VertexBetweenFaces(const R3MeshFace *face1, const R3MeshFace *face2, RNDirection dir) const
{
  // Call R3Mesh function and cast result to GSVMeshVertex
  return (GSVMeshVertex *) R3Mesh::VertexBetweenFaces(face1, face2, dir);
}



// End namespace
}


// End include guard
#endif
