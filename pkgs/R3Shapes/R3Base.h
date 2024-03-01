/* Include file for GAPS basic stuff */
#ifndef __R3__BASE__H__
#define __R3__BASE__H__



/* Begin namespace */
namespace gaps {



/* Initialization functions */

int R3InitBase();
void R3StopBase();



/* Class definition */

class R3Base {};



/* Draw flags definitions */

typedef RNFlags R3DrawFlags;
#define R3_NULL_DRAW_FLAGS                    (0x0000)
#define R3_EDGES_DRAW_FLAG                    (0x0001)
#define R3_SURFACES_DRAW_FLAG                 (0x0002)
#define R3_VERTICES_DRAW_FLAG                 (0x0004)
#define R3_SURFACE_COLORS_DRAW_FLAG           (0x0010)
#define R3_SURFACE_NORMALS_DRAW_FLAG          (0x0020)
#define R3_SURFACE_TEXTURE_DRAW_FLAG          (0x0030)
#define R3_SURFACE_MATERIAL_DRAW_FLAG         (0x0040)
#define R3_SURFACE_CATEGORY_DRAW_FLAG         (0x1000)
#define R3_SURFACE_INSTANCE_DRAW_FLAG         (0x2000)
#define R3_SURFACE_PICK_DRAW_FLAG             (0x4000)
#define R3_VERTEX_NORMALS_DRAW_FLAG           (0x0100)
#define R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG    (0x0200)
#define R3_VERTEX_COLORS_DRAW_FLAG            (0x0400)
#define R3_VERTEX_PICK_DRAW_FLAG              (0x0800)
#define R3_EVERYTHING_DRAW_FLAGS              (0x7777)
#define R3_DEFAULT_DRAW_FLAGS                 (R3_SURFACES_DRAW_FLAG | \
                                               R3_SURFACE_COLORS_DRAW_FLAG | R3_SURFACE_NORMALS_DRAW_FLAG | \
                                               R3_SURFACE_MATERIAL_DRAW_FLAG | R3_SURFACE_TEXTURE_DRAW_FLAG | \
                                               R3_VERTEX_NORMALS_DRAW_FLAG | R3_VERTEX_TEXTURE_COORDS_DRAW_FLAG | \
                                               R3_VERTEX_COLORS_DRAW_FLAG)

  
/* State flags definitions */

#define R3_VERTEX_SHARED_FLAG                (0x10000)



// End namespace
}


// End include guard
#endif
