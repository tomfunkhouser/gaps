////////////////////////////////////////////////////////////////////////
// Source file for RGBD coordinate transformation functions
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "RGBD.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// Transformation Functions
////////////////////////////////////////////////////////////////////////

int RGBDTransformTextureToSurface(const R2Point& texture_position, R2Point& surface_position, const RGBDSurface *surface)
{
  // Transform from position in texture coordinates to surface coordinates
  return surface->TransformTextureToSurface(texture_position, surface_position);
}



int RGBDTransformTextureToWorld(const R2Point& texture_position, R3Point& world_position, const RGBDSurface *surface)
{
  // Transform from position in texture coordinates to world coordinates
  R2Point surface_position;
  if (!RGBDTransformTextureToSurface(texture_position, surface_position, surface)) return 0;
  if (!RGBDTransformSurfaceToWorld(surface_position, world_position, surface)) return 0;
  return 1;
}



int RGBDTransformTextureToCamera(const R2Point& texture_position, R3Point& camera_position, const RGBDSurface *surface, const RGBDImage *image)
{
  // Transform from position in texture coordinates to camera coordinates
  R3Point world_position;
  if (!RGBDTransformTextureToWorld(texture_position, world_position, surface)) return 0;
  if (!RGBDTransformWorldToCamera(world_position, camera_position, image)) return 0;
  return 1;
}



int RGBDTransformTextureToImage(const R2Point& texture_position, R2Point& image_position, const RGBDSurface *surface, const RGBDImage *image)
{
  // Transform from position in texture coordinates to image coordinates
  R3Point world_position;
  if (!RGBDTransformTextureToWorld(texture_position, world_position, surface)) return 0;
  if (!RGBDTransformWorldToImage(world_position, image_position, image)) return 0;
  return 1;
}



int RGBDTransformSurfaceToTexture(const R2Point& surface_position, R2Point& texture_position, const RGBDSurface *surface)
{
  // Transform from position in surface coordinates to texture coordinates
  return surface->TransformSurfaceToTexture(surface_position, texture_position);
}



int RGBDTransformSurfaceToWorld(const R2Point& surface_position, R3Point& world_position, const RGBDSurface *surface)
{
  // Transform from position in surface coordinates to world coordinates
  return surface->TransformSurfaceToWorld(surface_position, world_position);
}



int RGBDTransformSurfaceToCamera(const R2Point& surface_position, R3Point& camera_position, const RGBDSurface *surface, const RGBDImage *image)
{
  // Transform from position in surface coordinates to camera coordinates
  R3Point world_position;
  if (!RGBDTransformSurfaceToWorld(surface_position, world_position, surface)) return 0;
  if (!RGBDTransformWorldToCamera(world_position, camera_position, image)) return 0;
  return 1;
}



int RGBDTransformSurfaceToImage(const R2Point& surface_position, R2Point& image_position, const RGBDSurface *surface, const RGBDImage *image)
{
  // Transform from position in surface coordinates to image coordinates
  R3Point world_position;
  if (!RGBDTransformSurfaceToWorld(surface_position, world_position, surface)) return 0;
  if (!RGBDTransformWorldToImage(world_position, image_position, image)) return 0;
  return 1;
}



int RGBDTransformWorldToTexture(const R3Point& world_position, R2Point& texture_position, const RGBDSurface *surface)
{
  // Transform from position in world coordinates to texture coordinates
  R2Point surface_position;
  if (!RGBDTransformWorldToSurface(world_position, surface_position, surface)) return 0;
  if (!RGBDTransformSurfaceToTexture(surface_position, texture_position, surface)) return 0;
  return 1;

  // Return success
  return 1;
}



int RGBDTransformWorldToSurface(const R3Point& world_position, R2Point& surface_position, const RGBDSurface *surface)
{
  // Transform from position in world coordinates to surface coordinates
  return surface->TransformWorldToSurface(world_position, surface_position);
}



int RGBDTransformWorldToCamera(const R3Point& world_position, R3Point& camera_position, const RGBDImage *image)
{
  // Transform from position in world coordinates to camera coordinates
  return image->TransformWorldToCamera(world_position, camera_position);
}



int RGBDTransformWorldToImage(const R3Point& world_position, R2Point& image_position, const RGBDImage *image)
{
  // Transform from position in world coordinates to image coordinates
  R3Point camera_position;
  if (!RGBDTransformWorldToCamera(world_position, camera_position, image)) return 0;
  if (!RGBDTransformCameraToImage(camera_position, image_position, image)) return 0;
  return 1;

  // Return success
  return 1;
}



int RGBDTransformCameraToTexture(const R3Point& camera_position, R2Point& texture_position, const RGBDImage *image, const RGBDSurface *surface)
{
  // Transform from position in camera coordinates to texture coordinates
  R3Point world_position;
  if (!RGBDTransformCameraToWorld(camera_position, world_position, image)) return 0;
  if (!RGBDTransformWorldToTexture(world_position, texture_position, surface)) return 0;
  return 1;
}



int RGBDTransformCameraToSurface(const R3Point& camera_position, R2Point& surface_position, const RGBDImage *image, const RGBDSurface *surface)
{
  // Transform from position in camera coordinates to surface coordinates
  R3Point world_position;
  if (!RGBDTransformCameraToWorld(camera_position, world_position, image)) return 0;
  if (!RGBDTransformWorldToSurface(world_position, surface_position, surface)) return 0;
  return 1;
}



int RGBDTransformCameraToWorld(const R3Point& camera_position, R3Point& world_position, const RGBDImage *image)
{
  // Transform from camera coordinates to world coordinates
  return image->TransformCameraToWorld(camera_position, world_position);
}



int RGBDTransformCameraToImage(const R3Point& camera_position, R2Point& image_position, const RGBDImage *image)
{
  // Transform from position in camera coordinates to image coordinates
  if (!image->TransformCameraToImage(camera_position, image_position)) return 0;

  // Check pixel position
  int image_ix = (int) (image_position.X() + 0.5);
  if ((image_ix < 0) || (image_ix >= image->NPixels(RN_X))) return 0;
  int image_iy = (int) (image_position.Y() + 0.5);
  if ((image_iy < 0) || (image_iy >= image->NPixels(RN_Y))) return 0;

#if 0
  // Get/check point depth
  RNScalar point_depth = -camera_position[2];
  if (RNIsNegativeOrZero(point_depth)) return 0;

  // Check depth
  RNScalar image_depth = image->PixelDepth(image_ix, image_iy);
  if ((image_depth == R2_GRID_UNKNOWN_VALUE) || (image_depth == 0)) return 0;

  // If depth is not within 10% of image depth, then probably not 
  if (image_depth < 0.9 * point_depth) return 0;
  if (image_depth > 1.1 * point_depth) return 0;
#endif
  
  // Return success
  return 1;
}




int RGBDTransformImageToTexture(const R2Point& image_position, R2Point& texture_position, const RGBDImage *image, const RGBDSurface *surface)
{
  // Transform from position in image coordinates to texture coordinates
  R3Point world_position;
  if (!RGBDTransformImageToWorld(image_position, world_position, image)) return 0;
  if (!RGBDTransformWorldToTexture(world_position, texture_position, surface)) return 0;
  return 1;
}



int RGBDTransformImageToSurface(const R2Point& image_position, R2Point& surface_position, const RGBDImage *image, const RGBDSurface *surface)
{
  // Transform from position in image coordinates to surface coordinates
  R3Point world_position;
  if (!RGBDTransformImageToWorld(image_position, world_position, image)) return 0;
  if (!RGBDTransformWorldToSurface(world_position, surface_position, surface)) return 0;
  return 1;
}



int RGBDTransformImageToWorld(const R2Point& image_position, R3Point& world_position, const RGBDImage *image)
{
  // Transform from position in image coordinates to world coordinates
  R3Point camera_position;
  if (!RGBDTransformImageToCamera(image_position, camera_position, image)) return 0;
  if (!RGBDTransformCameraToWorld(camera_position, world_position, image)) return 0;
  return 1;
}



int RGBDTransformImageToCamera(const R2Point& image_position, R3Point& camera_position, const RGBDImage *image)
{
  // Transform from position in image coordinates to camera coordinates (where camera is looking down -Z, up is +Y, right is +X)
  return image->TransformImageToCamera(image_position, camera_position);
}



} // namespace gaps
