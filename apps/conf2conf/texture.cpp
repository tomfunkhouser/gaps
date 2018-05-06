////////////////////////////////////////////////////////////////////////
// Source file for computing surface textures from a configuration of images
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Support functions
////////////////////////////////////////////////////////////////////////

static RNScalar
Score(RGBDImage *image, RGBDSurface *surface,
  const R2Point& image_position, const R2Point& texture_position,
  const RNRgb& color)
{
  // Don't allow purely black pixels ???
  if (color == RNblack_rgb) return 0.0;

  // Get convenient variables
  R3Point E = image->WorldViewpoint();
  R3Point P = surface->TexelWorldPosition(texture_position);
  R3Vector N = surface->TexelWorldNormal(texture_position);
  R3Vector T = -(image->WorldTowards());
  R3Vector V = E - P; 
  RNLength d = V.Length();
  if (RNIsZero(d)) return 0.0;
  else V /= d;

  // Consider viewing angles
  RNScalar ndotv = N.Dot(V);
  if (RNIsNegativeOrZero(ndotv)) return 0;
  RNScalar tdotv = T.Dot(V);
  if (RNIsNegativeOrZero(tdotv)) return 0;
  RNScalar invdd = (d > 1.0) ? 1.0 / (d*d) : 1.0;
  
  // Compute score
  RNScalar score = 1.0;
  score *= ndotv;
  score *= tdotv;
  score *= invdd;

  // Return score
  return score * score;
}



////////////////////////////////////////////////////////////////////////
// Support functions
////////////////////////////////////////////////////////////////////////

static int
RGBDComputeColorTexture(RGBDSurface *surface)
{
  // Get/check variables
  RGBDConfiguration *configuration = surface->Configuration();
  if (!configuration) return 0;
  int width = surface->NTexels(RN_X);
  if (width == 0) return 0;
  int height = surface->NTexels(RN_Y);
  if (height == 0) return 0;

  // Allocate images for texture RGB channels (initialized with zeroes)
  RNLength world_texel_spacing = surface->WorldTexelSpacing();
  R2Box surface_bbox(0.0, 0.0, world_texel_spacing*width, world_texel_spacing*height);
  R2Grid red_channel(width, height, surface_bbox);
  R2Grid green_channel(width, height, surface_bbox);
  R2Grid blue_channel(width, height, surface_bbox);

  // Allocate storage for accumulating statistics
  if (configuration->NImages() == 0) return 1;
  RNRgb *colors = new RNRgb [ configuration->NImages() ];
  if (!colors) { fprintf(stderr, "Unable to allocate memory\n"); return 0; }
  RNScalar *weights = new RNScalar [ configuration->NImages() ];
  if (!weights) { fprintf(stderr, "Unable to allocate memory\n"); delete [] colors; return 0; }

  // Fill the texture RGB channels
  for (int ix = 0; ix < width; ix++) {
    for (int iy = 0; iy < height; iy++) {
      // Check if (ix,iy) maps to a surface
      if (surface->mesh_face_index) {
        RNScalar face_index_value = surface->mesh_face_index->GridValue(ix, iy);
        if (face_index_value == R2_GRID_UNKNOWN_VALUE) continue;
      }

      // Get position in texture
      R2Point texture_position(ix+0.5, iy+0.5);

      // Initialize statistics
      int count = 0;
      RNScalar weight_sum = 0;
      RNRgb weighted_color_sum(0,0,0);

      // Gather sample from every image
      for (int i = 0; i < configuration->NImages(); i++) {
        // Get image
        RGBDImage *image = configuration->Image(i);

        // Get position in image
        R2Point image_position;
        if (!RGBDTransformTextureToImage(texture_position, image_position, surface, image)) continue;

        // Get rgb from image
        RNScalar r = image->PixelChannelValue(image_position, RGBD_RED_CHANNEL);
        RNScalar g = image->PixelChannelValue(image_position, RGBD_GREEN_CHANNEL);
        RNScalar b = image->PixelChannelValue(image_position, RGBD_BLUE_CHANNEL);

        // Check if valid image pixel
        if (r == R2_GRID_UNKNOWN_VALUE) continue;
        if (g == R2_GRID_UNKNOWN_VALUE) continue;
        if (b == R2_GRID_UNKNOWN_VALUE) continue;

        // Compute weight
        RNRgb color(r, g, b);
        RNScalar weight = Score(image, surface, image_position, texture_position, color);
        if (RNIsNegativeOrZero(weight)) continue;
        weight = 0.1;
        
        // Update statistics
        weight_sum += weight;
        weighted_color_sum += weight * color;
        weights[count] = weight;
        colors[count] = color;
        count++;
      }

      // Check statistics
      if (count == 0) continue;
      if (weight_sum == 0) continue;

#if 1
      // Compute weighted mean
      RNRgb color = weighted_color_sum / weight_sum;
#else
      // Compute weighted median
      RNRgb color = RNblack_rgb;
      RNScalar best_dd = FLT_MAX;
      for (int i = 0; i < count; i++) {
        RNScalar dd = 0;
        for (int j = 0; j < count; j++) {
          if (i != j) {
            RNScalar dr = colors[i].R() - colors[j].R();
            RNScalar dg = colors[i].G() - colors[j].G();
            RNScalar db = colors[i].B() - colors[j].B();
            dd += weights[j]*weights[j] * (dr*dr + dg*dg + db*db);
          }
        }
        if (dd < best_dd) {
          color = colors[i];
          best_dd = dd;
        }
      }
#endif
      
      // Update the texture channels
      red_channel.SetGridValue(ix, iy, color.R());
      green_channel.SetGridValue(ix, iy, color.G());
      blue_channel.SetGridValue(ix, iy, color.B());
    }
  }

  // Delete temporary memory
  delete [] colors;
  delete [] weights;

  // Create color channels
  R2Image tmp(width, height);
  surface->CreateColorChannels(tmp);

  // Update the texture RGB channels
  surface->SetChannel(RGBD_RED_CHANNEL, red_channel);
  surface->SetChannel(RGBD_GREEN_CHANNEL, green_channel);
  surface->SetChannel(RGBD_BLUE_CHANNEL, blue_channel);

  // Write color channels
  surface->WriteColorChannels();

  // Return success
  return 1;
}



static int
RGBDComputeSemanticTexture(RGBDSurface *surface)
{
  // Get/check variables
  RGBDConfiguration *configuration = surface->Configuration();
  if (!configuration) return 0;
  int width = surface->NTexels(RN_X);
  if (width == 0) return 0;
  int height = surface->NTexels(RN_Y);
  if (height == 0) return 0;
  R3Mesh *mesh = surface->mesh;
  if (!mesh) return 1;
  
  // Allocate images for texture RGB channels (initialized with zeroes)
  RNLength world_texel_spacing = surface->WorldTexelSpacing();
  R2Box surface_bbox(0.0, 0.0, world_texel_spacing*width, world_texel_spacing*height);
  R2Grid red_channel(width, height, surface_bbox);
  R2Grid green_channel(width, height, surface_bbox);
  R2Grid blue_channel(width, height, surface_bbox);

  // Rasterize mesh face categories into texture
  for (int i = 0; i < mesh->NFaces(); i++) {
    R3MeshFace *face = mesh->Face(i);
    // int category = mesh->FaceCategory(face);
    // int r = (category >> 0) && 0xFF;
    // int g = (category >> 8) && 0xFF;
    // int b = (category >> 16) && 0xFF;
    R3MeshVertex *v0 = mesh->VertexOnFace(face, 0);
    R3MeshVertex *v1 = mesh->VertexOnFace(face, 1);
    R3MeshVertex *v2 = mesh->VertexOnFace(face, 2);
    R2Point s0 = mesh->VertexTextureCoords(v0);
    R2Point s1 = mesh->VertexTextureCoords(v1);
    R2Point s2 = mesh->VertexTextureCoords(v2);
    R2Point t0, t1, t2;
    surface->TransformSurfaceToTexture(s0, t0);
    surface->TransformSurfaceToTexture(s1, t1);
    surface->TransformSurfaceToTexture(s2, t2);
    const RNRgb& color = mesh->VertexColor(v0);
    red_channel.RasterizeGridTriangle(t0, t1, t2, 255*color.R(), R2_GRID_REPLACE_OPERATION);
    green_channel.RasterizeGridTriangle(t0, t1, t2, 255*color.G(), R2_GRID_REPLACE_OPERATION);
    blue_channel.RasterizeGridTriangle(t0, t1, t2, 255*color.B(), R2_GRID_REPLACE_OPERATION);
  }

  // Create color channels
  R2Image tmp(width, height);
  surface->CreateColorChannels(tmp);

  // Update the texture RGB channels
  surface->SetChannel(RGBD_RED_CHANNEL, red_channel);
  surface->SetChannel(RGBD_GREEN_CHANNEL, green_channel);
  surface->SetChannel(RGBD_BLUE_CHANNEL, blue_channel);

  // Write color channels
  surface->WriteColorChannels();

  // Return success
  return 1;
}



static int
RGBDComputeSurfaceTextures(RGBDConfiguration *configuration, int texture_type)
{
  // Check if should bother
  if (configuration->NSurfaces() == 0) return 1;

  if (texture_type == 0) {
    // Compute color textures for all surfaces
    configuration->ReadChannels();
    for (int i = 0; i < configuration->NSurfaces(); i++) {
      RGBDSurface *surface = configuration->Surface(i);
      if (!RGBDComputeColorTexture(surface)) return 0;
    }
    configuration->ReleaseChannels();
  }
  else if (texture_type == 1) {
    // Compute semantic textures for all surfaces
    for (int i = 0; i < configuration->NSurfaces(); i++) {
      RGBDSurface *surface = configuration->Surface(i);
      if (!RGBDComputeSemanticTexture(surface)) return 0;
    }
  }

  // Return success
  return 1;
}


