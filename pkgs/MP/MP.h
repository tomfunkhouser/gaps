////////////////////////////////////////////////////////////////////////
// Include file for MP package
////////////////////////////////////////////////////////////////////////
#ifndef __MP__H__
#define __MP__H__



////////////////////////////////////////////////////////////////////////
// Dependency include files
////////////////////////////////////////////////////////////////////////

#include "R3Graphics/R3Graphics.h"
#include "RGBD/RGBD.h"



////////////////////////////////////////////////////////////////////////
// MP include files
////////////////////////////////////////////////////////////////////////

#define MP_DEFAULT_DRAW_FLAGS    0xFFFFFFFF

#include "MPUtils.h"
#include "MPImage.h"
#include "MPPanorama.h"
#include "MPSegment.h"
#include "MPObject.h"
#include "MPCategory.h"
#include "MPVertex.h"
#include "MPSurface.h"
#include "MPRegion.h"
#include "MPPortal.h"
#include "MPLevel.h"
#include "MPHouse.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {


  
////////////////////////////////////////////////////////////////////////
// Initialization functions
////////////////////////////////////////////////////////////////////////

int MPInit(void);
void MPStop(void);


  
////////////////////////////////////////////////////////////////////////
// Constants for defining drawing modes
////////////////////////////////////////////////////////////////////////

#define MP_DRAW_VERTICES         0x00000001
#define MP_DRAW_EDGES            0x00000002
#define MP_DRAW_FACES            0x00000004
#define MP_DRAW_BBOXES           0x00000008
#define MP_DRAW_DEPICTIONS       0x00000010
#define MP_DRAW_LABELS           0x00000020
#define MP_DRAW_IMAGES           0x00000040
#define MP_DRAW_FLAGS            0x000000FF

#define MP_SHOW_IMAGES           0x00000100
#define MP_SHOW_PANORAMAS        0x00000200
#define MP_SHOW_VERTICES         0x00000400
#define MP_SHOW_SURFACES         0x00000800
#define MP_SHOW_SEGMENTS         0x00001000
#define MP_SHOW_OBJECTS          0x00002000
#define MP_SHOW_REGIONS          0x00004000
#define MP_SHOW_PORTALS          0x00008000
#define MP_SHOW_LEVELS           0x00010000
#define MP_SHOW_MESH             0x00020000
#define MP_SHOW_SCENE            0x00040000
#define MP_SHOW_FLAGS            0x000FFF00

#define MP_COLOR_BY_IMAGE        0x00100000
#define MP_COLOR_BY_PANORAMA     0x00200000
#define MP_COLOR_BY_SURFACE      0x00400000
#define MP_COLOR_BY_SEGMENT      0x00800000
#define MP_COLOR_BY_OBJECT       0x01000000
#define MP_COLOR_BY_REGION       0x02000000
#define MP_COLOR_BY_PORTAL       0x04000000
#define MP_COLOR_BY_LEVEL        0x08000000
#define MP_COLOR_BY_LABEL        0x10000000
#define MP_COLOR_BY_INDEX        0x20000000
#define MP_COLOR_BY_RGB          0x40000000
#define MP_COLOR_FOR_PICK        0x80000000
#define MP_COLOR_FLAGS           0xFFF00000



// Constants for picking tags

#define MP_IMAGE_TAG             0xF0
#define MP_PANORAMA_TAG          0xF1
#define MP_OBJECT_TAG            0xF2
#define MP_SEGMENT_TAG           0xF3
#define MP_VERTEX_TAG            0xF4
#define MP_SURFACE_TAG           0xF5
#define MP_REGION_TAG            0xF6
#define MP_PORTAL_TAG            0xF7
#define MP_LEVEL_TAG             0xF8
#define MP_MESH_TAG              0xF9
#define MP_SCENE_TAG             0xFA



// End of namespace
};



// End of include guard
#endif




