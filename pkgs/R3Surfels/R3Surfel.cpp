/* Source file for the R3 surfel class */



/* Include files */

#include "R3Surfels.h"



// Namespace

namespace gaps {



/* Public functions */

R3Surfel::
R3Surfel(void)
  : timestamp(0),
    identifier(0),
    attribute(0),
    depth(0),
    elevation(0),
    flags(0)
{
  // Set everything
  SetPosition(0, 0, 0);
  SetNormal(0, 0, 0);
  SetTangent(0, 0, 0);
  SetRadius(0);
  SetColor(0, 0, 0);
}



R3Surfel::
R3Surfel(float x, float y, float z, 
  unsigned char r, unsigned char g, unsigned char b, 
  RNBoolean aerial)
  : timestamp(0),
    identifier(0),
    attribute(0),
    depth(0),
    elevation(0),
    flags(0)
{
  // Set everything
  SetPosition(x, y, z);
  SetNormal(0, 0, 0);
  SetTangent(0, 0, 0);
  SetRadius(0);
  SetColor(r, g, b);
  SetAerial(aerial);
}



R3Surfel::
R3Surfel(float x, float y, float z, float nx, float ny, float nz,
  float radius, unsigned char r, unsigned char g, unsigned char b, 
  unsigned char flags)
  : timestamp(0),
    identifier(0),
    attribute(0),
    depth(0),
    elevation(0),
    flags(flags)
{
  // Set everything
  SetPosition(x, y, z);
  SetNormal(nx, ny, nz);
  SetTangent(0, 0, 0);
  SetRadius(radius);
  SetColor(r, g, b);
}



R3Surfel::
R3Surfel(float x, float y, float z,
  float nx, float ny, float nz,
  float tx, float ty, float tz,
  float radius0, float radius1,
  unsigned char r, unsigned char g, unsigned char b, 
  float timestamp, unsigned int identifier,
  unsigned char flags)
  : timestamp(timestamp),
    identifier(identifier),
    attribute(0),
    depth(0),
    elevation(0),
    flags(flags)
{
  // Set everything
  SetPosition(x, y, z);
  SetNormal(nx, ny, nz);
  SetTangent(tx, ty, tz);
  SetRadius(0, radius0);
  SetRadius(1, radius1);
  SetColor(r, g, b);
}



R3Surfel::
R3Surfel(float x, float y, float z,
  float nx, float ny, float nz,
  float tx, float ty, float tz,
  float radius0, float radius1,
  float depth, float elevation,
  unsigned char r, unsigned char g, unsigned char b, 
  float timestamp, unsigned int identifier,
  unsigned char flags)
  : timestamp(timestamp),
    identifier(identifier),
    attribute(0),
    depth(0),
    elevation(0),
    flags(flags)
{
  // Set everything
  SetPosition(x, y, z);
  SetNormal(nx, ny, nz);
  SetTangent(tx, ty, tz);
  SetRadius(0, radius0);
  SetRadius(1, radius1);
  SetDepth(depth);
  SetElevation(elevation);
  SetColor(r, g, b);
}



} // namespace gaps
