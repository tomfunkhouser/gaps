/////////////////////////////////////////////////////////////////////////
// Include file for object pointset class
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Point class definition
////////////////////////////////////////////////////////////////////////

class ObjectPoint {
public:
  // Constructor destructor
  ObjectPoint(void);
  ObjectPoint(const ObjectPoint& point);
  ObjectPoint(const R3Point& position);
  ObjectPoint(const R3Point& position, const R3Vector& normal);
  ObjectPoint(const R3Point& position, const R3Vector& normal, const RNRgb& color);
  ObjectPoint(const R3Point& position, const R3Vector& normal, const RNRgb& color, const ObjectDescriptor& descriptor);

  // Properties
  const R3Point& Position(void) const;
  const R3Vector& Normal(void) const;
  const RNRgb Color(void) const;
  const ObjectDescriptor& Descriptor(void) const;
  const RNScalar Value(void) const;

  // Property manipulation
  ObjectPoint& operator=(const ObjectPoint& point);
  void Transform(const R3Transformation& transformation);
  void SetPosition(const R3Point& position);
  void SetNormal(const R3Vector& normal);
  void SetColor(const RNRgb& color);
  void SetDescriptor(const ObjectDescriptor& descriptor);
  void SetValue(RNScalar value);

public:
  R3Point position;
  R3Vector normal;
  ObjectDescriptor descriptor;
  unsigned char rgba[4];
  RNScalar value;
};



////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////

inline const R3Point& ObjectPoint::
Position(void) const
{
  // Return position
  return position;
}


inline const R3Vector& ObjectPoint::
Normal(void) const
{
  // Return normal
  return normal;
}



inline const RNRgb ObjectPoint::
Color(void) const
{
  // Return color
  RNRgb rgb;
  rgb[0] = rgba[0] / 255.0;
  rgb[1] = rgba[1] / 255.0;
  rgb[2] = rgba[2] / 255.0;
  return rgb;
}



inline const ObjectDescriptor& ObjectPoint::
Descriptor(void) const
{
  // Return descriptor
  return descriptor;
}



inline const RNScalar ObjectPoint::
Value(void) const
{
  // Return value
  return value;
}



inline void ObjectPoint::
SetPosition(const R3Point& position)
{
  // Set position
  this->position = position;
}



inline void ObjectPoint::
SetNormal(const R3Vector& normal)
{
  // Set normal
  this->normal = normal;
}



inline void ObjectPoint::
SetColor(const RNRgb& color)
{
  // Set color
  this->rgba[0] = (unsigned char) (255 * color[0]);
  this->rgba[1] = (unsigned char) (255 * color[1]);
  this->rgba[2] = (unsigned char) (255 * color[2]);
  this->rgba[3] = 1;
}



inline void ObjectPoint::
SetDescriptor(const ObjectDescriptor& descriptor)
{
  // Set descriptor
  this->descriptor = descriptor;
}



inline void ObjectPoint::
SetValue(RNScalar value)
{
  // Set value
  this->value = value;
}



inline void ObjectPoint::
Transform(const R3Transformation& transformation)
{
  // Transform point
  position.Transform(transformation);
  normal.Transform(transformation);
}
