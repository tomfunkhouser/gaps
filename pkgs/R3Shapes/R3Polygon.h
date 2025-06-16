/* Include file for the R3 polygon class */
#ifndef __R3__POLYGON__H__
#define __R3__POLYGON__H__



/* Begin namespace */
namespace gaps {



/* Initialization functions */

int R3InitPolygon();
void R3StopPolygon();



/* Class definition */

class R3Polygon : public R3Solid {
    public:
        // Constructor functions
	R3Polygon(void);
        R3Polygon(const R3Polygon& polygon);
        R3Polygon(const RNArray<R3Point *>& points, RNBoolean clockwise = FALSE);
        R3Polygon(const R3Point *points, int npoints, RNBoolean clockwise = FALSE);
        ~R3Polygon(void);

        // Polygon property functions/operators
        const int NPoints(void) const;
        const R3Point& Point(int k) const;
        const R3Vector& Normal(void) const;
        const R3Plane& Plane(void) const;
	const RNBoolean IsEmpty(void) const;
	const RNBoolean IsFinite(void) const;
	const RNBoolean IsClockwise(void) const;
        const R3Point& operator[](int k) const;
        const R3Point ClosestPoint(const R3Point& point) const;

        // Shape property functions/operators
	virtual const RNBoolean IsPoint(void) const;
	virtual const RNBoolean IsLinear(void) const ;
	virtual const RNBoolean IsPlanar(void) const ;
        virtual const RNInterval NFacets(void) const;
	virtual const RNArea Area(void) const;
	virtual const RNLength Perimeter(void) const;
	virtual const R3Point Centroid(void) const;
	virtual const R3Shape& BShape(void) const;
	virtual const R3Box BBox(void) const;
	virtual const R3Sphere BSphere(void) const;

        // Point properties
        R3Line Tangent(int k, RNLength radius = 0) const;
        RNAngle InteriorAngle(int k, RNLength radius = 0) const;
        RNScalar Curvature(int k, RNLength radius = 0) const;

        // Manipulation functions/operators
        virtual void Empty(void);
        virtual void Clip(const R3Plane& plane);
        virtual void Clip(const R3Box& box);
        virtual void RemovePoint(int k);
        virtual void InsertPoint(int k, const R3Point& position);
        virtual void SetPoint(int k, const R3Point& position);
	virtual void Transform(const R3Transformation& transformation);
        R3Polygon& operator=(const R3Polygon& polygon);

        // Draw functions/operators
        virtual void Draw(const R3DrawFlags draw_flags = R3_DEFAULT_DRAW_FLAGS) const;
        virtual void Print(FILE *fp = stdout) const;

    private:
        R3Point *points;
        int npoints;
        R3Plane plane;
        R3Box bbox;
        RNBoolean clockwise;
};



/* Inline functions */

inline const int R3Polygon::
NPoints(void) const
{
  // Return number of points
  return npoints;
}



inline const R3Point& R3Polygon::
Point(int k) const
{
  // Return Kth point
  return points[k];
}


inline const R3Vector& R3Polygon::
Normal(void) const
{
  // Return normal to plane
  return plane.Normal();
}


  
inline const R3Plane& R3Polygon::
Plane(void) const
{
  // Return plane
  return plane;
}


  
inline const RNBoolean R3Polygon::
IsEmpty(void) const
{
    // Return whether the polygon is null
    return (npoints == 0);
}



inline const RNBoolean R3Polygon::
IsFinite(void) const
{
    // Return whether the polygon has finite extent
    return bbox.IsFinite();
}




inline const RNBoolean R3Polygon::
IsClockwise(void) const
{
    // Return whether the polygon has points in clockwise order
    return clockwise;
}




inline const R3Point& R3Polygon::
operator[](int k) const
{
  // Return Kth point
  return Point(k);
}



// End namespace
}


// End include guard
#endif
