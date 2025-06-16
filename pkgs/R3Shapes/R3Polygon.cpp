/* Source file for the R3 polygon class */



/* Include files */

#include "R3Shapes.h"



// Namespace

namespace gaps {



/* Public functions */

int 
R3InitPolygon()
{
  return TRUE;
}



void 
R3StopPolygon()
{
}



R3Polygon::
R3Polygon(void) 
  : points(NULL),
    npoints(0),
    plane(R3null_plane),
    bbox(R3null_box),
    clockwise(FALSE)
{
}



R3Polygon::
R3Polygon(const R3Polygon& polygon) 
  : points(NULL),
    npoints(polygon.npoints),
    plane(polygon.plane),
    bbox(polygon.bbox),
    clockwise(polygon.clockwise)
{
  // Copy points
  if (polygon.npoints > 0) {
    points = new R3Point [ npoints ];
    for (int i = 0; i < npoints; i++) {
      points[i] = polygon.points[i];
    }
  }
}



R3Polygon::
R3Polygon(const RNArray<R3Point *>& p, RNBoolean clockwise)
  : points(NULL),
    npoints(p.NEntries()),
    plane(p, TRUE),
    bbox(R3null_box),
    clockwise(clockwise)
{
  // Copy points
  if (npoints > 0) {
    points = new R3Point [ npoints ];
    for (int i = 0; i < npoints; i++) {
      points[i] = *(p[i]);
      bbox.Union(*(p[i]));
    }
  }
}



R3Polygon::
R3Polygon(const R3Point *p, int np, RNBoolean clockwise)
  : points(NULL),
    npoints(np),
    plane(p, np, TRUE),
    bbox(R3null_box),
    clockwise(clockwise)
{
  // Copy points
  if (npoints > 0) {
    points = new R3Point [ npoints ];
    for (int i = 0; i < npoints; i++) {
      points[i] = p[i];
      bbox.Union(p[i]);
    }
  }
}



R3Polygon::
~R3Polygon(void) 
{
  // Delete points
  if (points) delete [] points;
}



const R3Point R3Polygon::
ClosestPoint(const R3Point& point) const
{
  // THIS IS NOT RIGHT, SHOULD RETURN INTERIOR AND EDGE POINTS TOO

  // Return closest point on polygon
  R3Point closest_point(0,0,0);
  RNLength closest_squared_distance = FLT_MAX;
  for (int i = 0; i < npoints; i++) {
    const R3Point& position = points[i];
    RNLength squared_distance = R3SquaredDistance(point, position);
    if (squared_distance < closest_squared_distance) {
      closest_point = position;
      closest_squared_distance = squared_distance;
    }
  }

  // Return closest point
  return closest_point;
}



const RNBoolean R3Polygon::
IsPoint(void) const
{
    // A polygon only lies on a single point if it has one point
    return (npoints == 1);
}



const RNBoolean R3Polygon::
IsLinear(void) const
{
    // A polygon only lies within a line if it has two points
    return (npoints == 2);
}



const RNBoolean R3Polygon::
IsPlanar(void) const
{
    // All polygons are planar
    return TRUE;
}



const RNArea R3Polygon::
Area(void) const
{
  // Check number of points
  if (npoints < 3) return 0;

  // Compute twicearea by sum of cross products
  RNLength sum = 0;
  R3Point *p1 = &points[npoints-1];
  for (int i = 0; i < npoints; i++) {
    R3Point *p2 = &points[i];
    sum += p1->X()*p2->Y() - p2->X()*p1->Y();
    p1 = p2;
  }

  // Compute area
  RNArea area = 0.5 * sum;

  // Flip if clockwise
  if (clockwise) area = -area;

  // Return area
  return area;
}



const RNLength R3Polygon::
Perimeter(void) const
{
  // Check number of points
  if (npoints < 2) return 0;

  // Compute perimeter
  RNLength sum = 0;
  R3Point *p1 = &points[npoints-1];
  for (int i = 0; i < npoints; i++) {
    R3Point *p2 = &points[i];
    sum += R3Distance(*p1, *p2);
    p1 = p2;
  }

  // Return perimeter
  return sum;
}



const R3Point R3Polygon::
Centroid(void) const
{
  // Check number of points
  if (npoints == 0) return R3zero_point;

  // Return centroid
  R3Point sum = R3zero_point;
  for (int i = 0; i < npoints; i++) sum += points[i];
  return sum / npoints;
}



const R3Shape& R3Polygon::
BShape(void) const
{
    // Return bounding box
    return bbox;
}



const R3Box R3Polygon::
BBox(void) const
{
    // Return bounding box of polygon
    return bbox;
}



const R3Sphere R3Polygon::
BSphere(void) const
{
    // Return bounding sphere
    return bbox.BSphere();
}



R3Line R3Polygon::
Tangent(int k, RNLength radius) const
{
  // Just checking
  if (npoints < 3) return R3null_line;

  // Create array with points of patch within radius
  RNArray<R3Point *> patch;
  patch.Insert(&points[k]);

  // Add points searching one way
  RNLength distance = 0;
  R3Point *prev = &points[k];
  for (int i = 1; i < npoints/3; i++) {
    R3Point *current = &points[(k - i + npoints) % npoints];
    patch.Insert(current);
    distance += R3Distance(*current, *prev);
    if (distance >= radius) break;
    prev = current;
  }

  // Add points searching other way
  distance = 0;
  prev = &points[k];
  for (int i = 1; i < npoints/3; i++) {
    R3Point *current = &points[(k + i) % npoints];
    patch.Insert(current);
    distance += R3Distance(*current, *prev);
    if (distance >= radius) break;
    prev = current;
  }

  // Compute best fitting line to points in patch
  R3Point centroid = R3Centroid(patch);
  R3Triad triad = R3PrincipleAxes(centroid, patch);
  R3Vector vector = triad.Axis(0);

  // Flip vector if facing wrong direction
  R3Point& p1 = points[(k - 1 + npoints) % npoints];
  R3Point& p2 = points[(k + 1) % npoints];
  if (vector.Dot(p2 - p1) < 0) vector.Flip();

  // Flip if polygon is clockwise
  if (clockwise) vector = -vector;

  // Return tangent line
  return R3Line(points[k], vector);
}



RNAngle R3Polygon::
InteriorAngle(int k, RNLength radius) const
{
  // Just checking
  if (npoints < 3) return 0;

  // Find first point
  R3Point *pointA = &points[(k - 1 + npoints) % npoints];
  if (radius > 0) {
    R3Point *prev = pointA;
    RNLength distance = R3Distance(*pointA, points[k]);
    if (distance < radius) {
      for (int i = 2;  i < npoints/3; i++) {
        pointA = &points[(k - i + npoints) % npoints];
        distance += R3Distance(*pointA, *prev);
        if (distance >= radius) break;
        prev = pointA;
      }
    }
  }

  // Find second point
  R3Point *pointB = &points[(k + 1) % npoints];
  if (radius > 0) {
    R3Point *prev = pointB;
    RNLength distance = R3Distance(*pointB, points[k]);
    if (distance < radius) {
      for (int i = 2;  i < npoints/3; i++) {
        pointB = &points[(k + i) % npoints];
        distance += R3Distance(*pointB, *prev);
        if (distance >= radius) break;
        prev = pointB;
      }
    }
  }

  // Compute interior angle
  R3Vector va = *pointA - points[k];
  R3Vector vb = *pointB - points[k];
  RNLength lena = va.Length();
  RNLength lenb = vb.Length();
  if (RNIsZero(lena) || RNIsZero(lenb)) return RN_UNKNOWN;
  RNAngle angle = R3InteriorAngle(va, vb);
  RNScalar cross = (va[0]*vb[1] - va[1]*vb[0]);
  if (clockwise) { if (cross < 0) angle = RN_TWO_PI - angle; }
  else { if (cross > 0) angle = RN_TWO_PI - angle; }
  return angle;
}



RNScalar R3Polygon::
Curvature(int k, RNLength radius) const
{
  // Just checking
  if (npoints < 3) return 0;

  // Find first point
  R3Point *pointA = &points[(k - 1 + npoints) % npoints];
  if (radius > 0) {
    R3Point *prev = pointA;
    RNLength distance = R3Distance(*pointA, points[k]);
    if (distance < radius) {
      for (int i = 2;  i < npoints/3; i++) {
        pointA = &points[(k - i + npoints) % npoints];
        distance += R3Distance(*pointA, *prev);
        if (distance >= radius) break; 
        prev = pointA;
      }
    }
  }

  // Find second point
  R3Point *pointB = &points[(k + 1) % npoints];
  if (radius > 0) {
    R3Point *prev = pointB;
    RNLength distance = R3Distance(*pointB, points[k]);
    if (distance < radius) {
      for (int i = 2;  i < npoints/3; i++) {
        pointB = &points[(k + i) % npoints];
        distance += R3Distance(*pointB, *prev);
        if (distance >= radius) break;
        prev = pointB;
      }
    }
  }

  // Compute curvature
  R3Vector va = *pointA - points[k];
  R3Vector vb = *pointB - points[k];
  RNLength lena = va.Length();
  RNLength lenb = vb.Length();
  if (RNIsZero(lena) || RNIsZero(lenb)) return RN_UNKNOWN;
  RNScalar angle = R3InteriorAngle(va, vb);
  RNScalar cross = (va[0]*vb[1] - va[1]*vb[0]);
  RNScalar sign = (cross > 0) ? -1 : 1;
  RNScalar curvature = sign * (RN_PI - angle) / (lena + lenb);

  // Flip if polygon is clockwise
  if (clockwise) curvature = -curvature;

  // Return curvature
  return curvature;
}



void R3Polygon::
Empty(void)
{
    // Empty polygon
    if (points) delete [] points;
    points = NULL;
    npoints = 0;
    plane = R3null_plane;
    bbox = R3null_box;
}



void R3Polygon::
RemovePoint(int k)
{
    // Copy points down one
    npoints--;
    for (int i = k; k < npoints; i++) {
      points[i] = points[i+1];
    }

    // Update plane
    plane = R3Plane(points, npoints, TRUE);

    // Update bounding box
    bbox = R3null_box;
    for (int i = 0; i < npoints; i++) {
        bbox.Union(points[i]);
    }
}



void R3Polygon::
InsertPoint(int k, const R3Point& position)
{
    // Add point in position k, moving others starting at k+1 up to make room
    R3Point *p = new R3Point [npoints+1];
    for (int i = 0; i < k; i++) p[i] = points[i];
    p[k] = position;
    for (int i = k; i < npoints; i++) p[i+1] = points[i];
    if (points) delete [] points;
    points = p;
    npoints++;
    
    // Update plane
    plane = R3Plane(points, npoints, TRUE);

    // Update bounding box
    bbox.Union(position);
}



void R3Polygon::
SetPoint(int k, const R3Point& position)
{
    // Set point
    points[k] = position;

    // Update plane
    plane = R3Plane(points, npoints, TRUE);

    // Update bounding box
    bbox = R3null_box;
    for (int i = 0; i < npoints; i++) {
        bbox.Union(points[i]);
    }
}



void R3Polygon::
Clip(const R3Plane& plane) 
{
  // Check number of points
  if (npoints == 0) {
    return;
  }
  else if (npoints == 1) {
    if (R3SignedDistance(plane, points[0]) < 0) {
      bbox = R3null_box;
      delete [] points;
      points = NULL;
      npoints = 0;
    }
  }
  else {
    // Check bounding box
    R3Halfspace halfspace(plane, 0);

    // Check if bounding box is entirely on positive side of plane
    if (R3Contains(halfspace, bbox)) {
      return;
    }

    // Check if bounding box is entirely on negative side of plane
    if (R3Contains(-halfspace, bbox)) {
      bbox = R3null_box;
      delete [] points;
      points = NULL;
      npoints = 0;
      return;
    }

    // Create new array for points
    int nbuffer = 0;
    R3Point *buffer = new R3Point [ 4 * npoints ];
    if (!buffer) RNAbort("Unable to allocate points during clip");

    // Build buffer with clipped points
    const R3Point *p1 = &points[npoints-1];
    RNScalar d1 = R3SignedDistance(plane, *p1);
    for (int i = 0; i < npoints; i++) {
      const R3Point *p2 = &points[i];
      RNScalar d2 = R3SignedDistance(plane, *p2);
      if (d2 >= 0) {
        // Insert crossing from negative to positive
        if (d1 < 0) {
          RNScalar t = d2 / (d2 - d1);
          buffer[nbuffer++] = *p2 + t * (*p1 - *p2);
        }

        // Insert point on positive side
        buffer[nbuffer++] = *p2;
      }
      else {
        // Insert crossing from positive to negative
        if (d1 >= 0) {
          RNScalar t = d1 / (d1 - d2);
          buffer[nbuffer++] = *p1 + t * (*p2 - *p1);
        }
      }

      // Remember previous point
      p1 = p2;
      d1 = d2;
    }

    // Copy points
    bbox = R3null_box;
    npoints = nbuffer;
    delete [] points;
    points = new R3Point [ npoints ];
    for (int i = 0; i < npoints; i++) {
      points[i] = buffer[i];
      bbox.Union(points[i]);
    }

    // Delete the buffer of points
    delete [] buffer;
  }
}



void R3Polygon::
Clip(const R3Box& box) 
{
  // Clip to each side of box
  if (npoints == 0) return;
  Clip(R3Plane(1, 0, 0, -(box.XMin())));
  if (npoints == 0) return;
  Clip(R3Plane(-1, 0, 0, box.XMax()));
  if (npoints == 0) return;
  Clip(R3Plane(0, 1, 0, -(box.YMin())));
  if (npoints == 0) return;
  Clip(R3Plane(0, -1, 0, box.YMax()));
  if (npoints == 0) return;
  Clip(R3Plane(0, 0, 1, -(box.ZMin())));
  if (npoints == 0) return;
  Clip(R3Plane(0, 0, -1, box.ZMax()));
}



void R3Polygon::
Transform(const R3Transformation& transformation) 
{
  // Transform plane
  plane.Transform(transformation);

  // Transform points
  bbox = R3null_box;
  for (int i = 0; i < npoints; i++) {
    points[i].Transform(transformation);
    bbox.Union(points[i]);
  }
}



R3Polygon& R3Polygon::
operator=(const R3Polygon& polygon) 
{
  // Delete previous points
  if (points) {
    delete points;
    points = NULL;
  }
  
  // Copy properties
  clockwise = polygon.clockwise;
  plane = polygon.plane;
  bbox = polygon.bbox;

  // Copy points
  npoints = polygon.npoints;
  if (npoints > 0) {
    points = new R3Point [ npoints ];
    for (int i = 0; i < npoints; i++) {
      points[i] = polygon.points[i];
    }
  }

  // Return this
  return *this;
}



void R3Polygon::
Print(FILE *fp) const
{
  // Print points
  fprintf(fp, "%d\n", npoints);
  for (int i = 0; i < npoints; i++) {
    fprintf(fp, "%12.6f %12.6f\n", points[i].X(), points[i].Y());
  }
}



} // namespace gaps
