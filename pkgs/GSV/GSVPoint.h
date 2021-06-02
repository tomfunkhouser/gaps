// Include file for the GSV point class 
#ifndef __GSV__POINT__H__
#define __GSV__POINT__H__



////////////////////////////////////////////////////////////////////////
// NAMESPACE
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// CLASS DEFINITION
////////////////////////////////////////////////////////////////////////

class GSVPoint {
public:
  //////////////////////////////////////////
  //// CONSTRUCTOR/DESTRUCTOR FUNCTIONS ////
  //////////////////////////////////////////

  // Constructor functions
  GSVPoint(void);


  ////////////////////////////
  //// PROPERTY FUNCTIONS ////
  ////////////////////////////

  // Geometric property functions
  R3Point Position(void) const;
  R3Vector Tangent(void) const;
  R3Vector Normal(void) const;
  RNLength Radius(int k) const;
  RNLength Radius(void) const;

  // Other property functions
  RNScalar Timestamp(void) const;
  RNRgb Color(void) const;
  RNScalar Elevation(void) const;
  int ReturnType(void) const;
  int BeamIndex(void) const;
  int Reflectivity(void) const;
  int PointIdentifier(void) const;
  int ClusterIdentifier(void) const;
  int CategoryIdentifier(void) const;
  RNScalar CategoryConfidence(void) const;
  RNBoolean IsOnGround(void) const;
  

  ////////////////////////////////
  //// MANIPULATION FUNCTIONS ////
  ////////////////////////////////

  // Geometric manipulation functions
  void SetPosition(const R3Point& position);
  void SetNormal(const R3Vector& normal);
  void SetTangent(const R3Vector& tangent);
  void SetRadius(int k, RNLength radius);
  void SetRadius(RNLength radius);
  
  // Other manipulation functions
  void SetReturnType(int return_type);
  void SetColor(const RNRgb& color);
  void SetColor(unsigned char r, unsigned char g, unsigned char b);
  void SetTimestamp(RNScalar timestamp);
  void SetElevation(RNScalar elevation);
  void SetReflectivity(int reflectivity);
  void SetBeamIndex(int beam_index);
  void SetPointIdentifier(int point_identifier);
  void SetClusterIdentifier(int cluster_identifier);
  void SetCategoryIdentifier(int category_identifier);
  void SetCategoryConfidence(RNScalar category_confidence);

  // Transformation functions
  void Transform(const R3Transformation& transformation);
  
  
////////////////////////////////////////////////////////////////////////
// INTERNAL STUFF BELOW HERE
////////////////////////////////////////////////////////////////////////

public:
  // Lower level access functions 
  const double *PositionCoords(void) const;
  const float *NormalCoords(void) const;
  const float *TangentCoords(void) const;
  unsigned char Flags(void) const;
  
  // Lower level manipulation functions
  void SetPositionCoords(const double position[3]);
  void SetNormalCoords(const float normal[3]);
  void SetTangentCoords(const float tangent[3]);
  void SetFlags(unsigned char flags);

protected:
  friend class GSVScanline;
  double position[3];
  double timestamp; 
  float normal[3];
  float tangent[3];
  int point_identifier;
  int cluster_identifier;
  unsigned short radius[2];
  short elevation;
  unsigned char beam_index;
  unsigned char reflectivity;
  unsigned char color[3];
  unsigned char category_identifier;
  unsigned char category_confidence;
  unsigned char flags;
};



////////////////////////////////////////////////////////////////////////
// RETURN TYPE CONSTANTS
////////////////////////////////////////////////////////////////////////

#define GSV_POINT_STRONGEST_RETURN_TYPE                 0x0001
#define GSV_POINT_LAST_RETURN_TYPE                      0x0002
#define GSV_POINT_STRONGEST_OF_NONLAST_RETURN_TYPE      0x0003



////////////////////////////////////////////////////////////////////////
// FLAGS CONSTANTS
////////////////////////////////////////////////////////////////////////

#define GSV_POINT_RETURN_TYPE_FLAGS                     0x0003
#define GSV_POINT_ON_GROUND_FLAG                        0x0004


 
////////////////////////////////////////////////////////////////////////
// INLINE FUNCTION DEFINITIONS
////////////////////////////////////////////////////////////////////////

inline R3Point GSVPoint::
Position(void) const
{
  // Return position
  return R3Point(position[0], position[1], position[2]);
}



inline const double *GSVPoint::
PositionCoords(void) const
{
  // Return position
  return position;
}



inline R3Vector GSVPoint::
Normal(void) const
{
  // Return position
  return R3Vector(normal[0], normal[1], normal[2]);
}



inline const float *GSVPoint::
NormalCoords(void) const
{
  // Return position
  return normal;
}



inline R3Vector GSVPoint::
Tangent(void) const
{
  // Return position
  return R3Vector(tangent[0], tangent[1], tangent[2]);
}



inline const float *GSVPoint::
TangentCoords(void) const
{
  // Return position
  return tangent;
}



inline RNLength GSVPoint::
Radius(int k) const
{
  // Return radius (0 is tangent, 1 is orthogonal)
  assert((k >= 0) && (k <= 1));
  return 1E-3 * radius[k];
}



inline RNLength GSVPoint::
Radius(void) const
{
  // Return average radius
  return 0.5 * (Radius(0) + Radius(1));
}



inline int GSVPoint::
ReturnType(void) const
{
  // Return return type
  return (flags & GSV_POINT_RETURN_TYPE_FLAGS);
}



inline RNRgb GSVPoint::
Color(void) const
{
  // Return color
  return RNRgb(color[0]/255.0, color[1]/255.0, color[2]/255.0);
}



inline RNLength GSVPoint::
Timestamp(void) const
{
  // Return timestamp
  return timestamp;
}



inline RNScalar GSVPoint::
Elevation(void) const
{
  // Return elevation
  return elevation / 400.0;
}



inline int GSVPoint::
Reflectivity(void) const
{
  // Return reflectivity
  return reflectivity;
}



inline int GSVPoint::
BeamIndex(void) const
{
  // Return beam index
  return beam_index;
}



inline int GSVPoint::
PointIdentifier(void) const
{
  // Return point identifier
  return point_identifier;
}



inline int GSVPoint::
ClusterIdentifier(void) const
{
  // Return cluster identifier
  return cluster_identifier;
}



inline int GSVPoint::
CategoryIdentifier(void) const
{
  // Return category identifier
  return category_identifier;
}



inline RNScalar GSVPoint::
CategoryConfidence(void) const
{
  // Return category identifier
  if (category_confidence <= 0) return 0;
  else if (category_confidence > 255) return 1;
  return (RNScalar) category_confidence / 255.0;
}



inline unsigned char GSVPoint::
Flags(void) const
{
  // Return flags
  return flags;
}



inline RNBoolean GSVPoint::
IsOnGround(void) const
{
  // Return whether point is on ground
  return (Flags() & GSV_POINT_ON_GROUND_FLAG);
}



////////////////////////////////////////////////////////////////////////
// Manipulation functions
////////////////////////////////////////////////////////////////////////

inline void GSVPoint::
SetPosition(const R3Point& position)
{
  // Set position
  this->position[0] = position[0];
  this->position[1] = position[1];
  this->position[2] = position[2];
}


  
inline void GSVPoint::
SetPositionCoords(const double position[3])
{
  // Set position
  this->position[0] = position[0];
  this->position[1] = position[1];
  this->position[2] = position[2];
}



inline void GSVPoint::
SetNormal(const R3Vector& normal)
{
  // Set normal
  this->normal[0] = normal[0];
  this->normal[1] = normal[1];
  this->normal[2] = normal[2];
}

  

inline void GSVPoint::
SetNormalCoords(const float normal[3])
{
  // Set normal
  this->normal[0] = normal[0];
  this->normal[1] = normal[1];
  this->normal[2] = normal[2];
}



inline void GSVPoint::
SetTangent(const R3Vector& tangent)
{
  // Set tangent
  this->tangent[0] = tangent[0];
  this->tangent[1] = tangent[1];
  this->tangent[2] = tangent[2];
}

  

inline void GSVPoint::
SetTangentCoords(const float tangent[3])
{
  // Set tangent
  this->tangent[0] = tangent[0];
  this->tangent[1] = tangent[1];
  this->tangent[2] = tangent[2];
}



inline void GSVPoint::
SetRadius(int k, RNLength radius)
{
  // Set radius
  if (radius <= 0) this->radius[k] = 0;
  else if (radius > 64) this->radius[k] = 1e3 * 64;
  else this->radius[k] = 1e3 * radius + 0.5;
}

  

inline void GSVPoint::
SetRadius(RNLength radius)
{
  // Set both radii
  SetRadius(0, radius);
  SetRadius(1, radius);
}

  

inline void GSVPoint::
SetColor(unsigned char r, unsigned char g, unsigned char b)
{
  // Set color
  this->color[0] = r;
  this->color[1] = g;
  this->color[2] = b;
}



inline void GSVPoint::
SetColor(const RNRgb& color)
{
  // Set color
  SetColor(255*color[0], 255*color[1], 255*color[2]);
}



inline void GSVPoint::
SetReturnType(int return_type)
{
  // Set return type
  this->flags =
    (this->flags & ~GSV_POINT_RETURN_TYPE_FLAGS) |
    (return_type & GSV_POINT_RETURN_TYPE_FLAGS);
}



inline void GSVPoint::
SetTimestamp(RNScalar timestamp)
{
  // Set timestamp 
  this->timestamp = timestamp;
}

  

inline void GSVPoint::
SetElevation(RNScalar elevation)
{
  // Set elevation
  double encoded = 400.0 * elevation;
  if (encoded < SHRT_MIN) encoded = SHRT_MIN;
  else if (encoded > SHRT_MAX) encoded = SHRT_MAX;
  this->elevation = encoded;
}

  

inline void GSVPoint::
SetReflectivity(int reflectivity)
{
  // Set reflectivity
  if (reflectivity <= 0) reflectivity = 0;
  if (reflectivity > 255) reflectivity = 255;
  this->reflectivity = reflectivity;
}

  

inline void GSVPoint::
SetBeamIndex(int beam_index)
{
  // Set beam_index
  if (beam_index <= 0) beam_index = 0;
  if (beam_index > 255) beam_index = 255;
  this->beam_index = beam_index;
}


 
inline void GSVPoint::
SetPointIdentifier(int point_identifier)
{
  // Set point_identifier
  this->point_identifier = point_identifier;
}



inline void GSVPoint::
SetClusterIdentifier(int cluster_identifier)
{
  // Set cluster_identifier
  this->cluster_identifier = cluster_identifier;
}



inline void GSVPoint::
SetCategoryIdentifier(int category_identifier)
{
  // Set category_identifier
  if (category_identifier <= 0) category_identifier = 0;
  if (category_identifier > 255) category_identifier = 0;
  this->category_identifier = category_identifier;
}


 
inline void GSVPoint::
SetCategoryConfidence(RNScalar category_confidence)
{
  // Set category_identifier
  if (category_confidence <= 0) this->category_confidence = 0;
  else if (category_confidence >= 1.0) this->category_confidence = 255;
  else this->category_confidence = 255 * category_confidence;
}


 
inline void GSVPoint::
SetFlags(unsigned char flags)
{
  // Set flags 
  this->flags = flags;
}

  

inline void GSVPoint::
Transform(const R3Transformation& transformation)
{
  // Transform position and normal
  R3Point position = Position();
  R3Vector normal = Normal();
  R3Vector tangent = Tangent();
  position.Transform(transformation);
  normal.Transform(transformation);
  tangent.Transform(transformation);
  SetPosition(position);
  SetNormal(normal);
  SetTangent(tangent);

  // Transform radii
  double scale = transformation.ScaleFactor();
  if (RNIsNotEqual(scale, 1.0)) {
    for (int i = 0; i < 2; i++) {
      double r = this->radius[i];
      r *= fabs(scale);
      if (r > 64000) r = 64000;
      this->radius[i] = r;
    }
  }
}



// End namespace
}


// End include guard
#endif
