/* Include file for RN graphics module */
#ifndef __RN__GRFX__H__
#define __RN__GRFX__H__



/* Turn off warnings about deprecated functions (for GLU and GLUT) */

#if (RN_OS == RN_MAC) 
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif



/* Graphics library include files */

#if (RN_2D_GRFX == RN_XLIB)
#   ifndef __RN_XLIB__
#       include "X11/Xlib.h"
#       include "X11/X.h"
#       define __RN_XLIB__
#   endif
#endif

#if (RN_2D_GRFX == RN_OPENGL) || (RN_3D_GRFX == RN_OPENGL)
#   ifndef __RN_OPENGL__
#       if (RN_OS == RN_MAC) 
#           include <OpenGL/gl.h>
#           include <OpenGL/glu.h>
#       elif (RN_OS == RN_WINDOWS)
#           include <GL/gl.h>
#           include <GL/glu.h>
#       else
#           define GLEW_STATIC 1
#           include <GL/glew.h>
#           include <GL/gl.h>
#           include <GL/glu.h>
#           if defined(USE_MESA) && !defined(GLAPI)
#               define GLAPI extern
#           endif
#       endif
#       define __RN_OPENGL__
#   endif
#endif



/* Graphics windowing/drawing include files */
/* Used only for drawing text in pkgs */

#if (RN_2D_GRFX == RN_OPENGL) || (RN_3D_GRFX == RN_OPENGL)
#  include "fglut/fglut.h"
#endif



/* Drawing modes */

#define RN_GRFX_POINTS         GL_POINTS
#define RN_GRFX_LINES          GL_LINES
#define RN_GRFX_LINE_STRIP     GL_LINE_STRIP
#define RN_GRFX_LINE_LOOP      GL_LINE_LOOP
#define RN_GRFX_TRIANGLES      GL_TRIANGLES
#define RN_GRFX_TRIANGLE_STRIP GL_TRIANGLE_STRIP
#define RN_GRFX_TRIANGLE_FAN   GL_TRIANGLE_FAN
#define RN_GRFX_QUADS          GL_QUADS
#define RN_GRFX_QUAD_STRIP     GL_QUAD_STRIP
#define RN_GRFX_POLYGON        GL_POLYGON        



/* Fonts */

#ifdef __FGLUT__H__
#   define  RN_GRFX_BITMAP_TIMES_ROMAN_10   GLUT_BITMAP_TIMES_ROMAN_10
#   define  RN_GRFX_BITMAP_TIMES_ROMAN_24   GLUT_BITMAP_TIMES_ROMAN_24
#   define  RN_GRFX_BITMAP_HELVETICA_10     GLUT_BITMAP_HELVETICA_10  
#   define  RN_GRFX_BITMAP_HELVETICA_12     GLUT_BITMAP_HELVETICA_12  
#   define  RN_GRFX_BITMAP_HELVETICA_18     GLUT_BITMAP_HELVETICA_18  
#else
#   define  RN_GRFX_BITMAP_TIMES_ROMAN_10      ((void *)0x0004)
#   define  RN_GRFX_BITMAP_TIMES_ROMAN_24      ((void *)0x0005)
#   define  RN_GRFX_BITMAP_HELVETICA_10        ((void *)0x0006)
#   define  RN_GRFX_BITMAP_HELVETICA_12        ((void *)0x0007)
#   define  RN_GRFX_BITMAP_HELVETICA_18        ((void *)0x0008)
#endif



/* Compatibility definitions -- not sure about this */

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif



/* Begin namespace */
namespace gaps {


  
/* Initialization functions */

int RNInitGrfx(void);
void RNStopGrfx(void);



/* 2D viewing functions */

void R2SetViewport(int xmin, int ymin, int xmax, int ymax);
void R2SetWindow(float xmin, float ymin, float xmax, float ymax);
void R2ScaleWindow(float xscale, float yscale);
void R2TranslateWindow(float xtranslate, float ytranslate);
void R2WindowToViewport(float wx, float wy, int *vx, int *vy);
void R2WindowToViewport(double wx, double wy, int *vx, int *vy);
void R2WindowToViewport(float wx, float wy, short *vx, short *vy);
void R2WindowToViewport(double wx, double wy, short *vx, short *vy);
void R2ViewportToWindow(int vx, int vy, float *wx, float *wy);
void R2ViewportToWindow(int vx, int vy, double *wx, double *wy);



/* "Immediate mode" drawing begin/end functions */

void RNGrfxBegin(int mode);
void RNGrfxEnd(void);

void R2BeginPolygon(void);
void R2EndPolygon(void);
void R2BeginLine(void);
void R2EndLine(void);
void R2BeginLoop(void);
void R2EndLoop(void);

void R3BeginPolygon(void);
void R3EndPolygon(void);
void R3BeginLine(void);
void R3EndLine(void);
void R3BeginLoop(void);
void R3EndLoop(void);



/* "Immediate mode" data loading functions (called between RNGrfxBegin and RNGrfxEnd) */
  
void R2LoadPoint(int x, int y);
void R2LoadPoint(float x, float y);
void R2LoadPoint(double x, double y);
void R2LoadPoint(const int point[2]);
void R2LoadPoint(const float point[2]);
void R2LoadPoint(const double point[2]);

void R3LoadPoint(int x, int y, int z);
void R3LoadPoint(float x, float y, float z);
void R3LoadPoint(double x, double y, double z);
void R3LoadPoint(const int point[3]);
void R3LoadPoint(const float point[3]);
void R3LoadPoint(const double point[3]);

void R3LoadNormal(float x, float y, float z);
void R3LoadNormal(double x, double y, float z);
void R3LoadNormal(const float normal[3]);
void R3LoadNormal(const double normal[3]);

void R3LoadTextureCoords(int x, int y);
void R3LoadTextureCoords(float x, float y);
void R3LoadTextureCoords(double x, double y);
void R3LoadTextureCoords(const int texcoords[2]);
void R3LoadTextureCoords(const float texcoords[2]);
void R3LoadTextureCoords(const double texcoords[2]);

void RNLoadRgb(unsigned char r, unsigned char g, unsigned char b);
void RNLoadRgb(float r, float g, float b);
void RNLoadRgb(double r, double g, double b);
void RNLoadRgb(const unsigned char rgb[3]);
void RNLoadRgb(const float rgb[3]);
void RNLoadRgb(const double rgb[3]);

void RNLoadRgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void RNLoadRgba(float r, float g, float b, float a);
void RNLoadRgba(double r, double g, double b, double a);
void RNLoadRgba(unsigned char rgba[4]);
void RNLoadRgba(const float rgba[4]);
void RNLoadRgba(const double rgba[4]);
void RNLoadRgba(unsigned int value);


 
/* Text drawing functions */

void R2DrawText(double x, double y, const char *str, void *font = NULL);
void R3DrawText(double x, double y, double z, const char *str, void *font = NULL);
int RNTextWidth(const char *str, void *font = NULL);

  

/* Image drawing functions */

void R2DrawImage(int x, int y, int width, int height, int depth, void *data);



/* Error handling functions */

void RNGrfxError(const char *message);



/* Graphics library specific functions */

#if (RN_2D_GRFX == RN_XLIB)
    int RNInitGrfx(Display *display, Window window, GC gc);
#endif



/* Inline function implementations */

#include "RNGrfx.hh"



/* To disambiguate type resolution in overloaded functions */
  
inline void R2LoadPoint(int x, float y) { R2LoadPoint((float) x, y); }
inline void R2LoadPoint(int x, double y) { R2LoadPoint((double) x, y); }
inline void R2LoadPoint(float x, int y) { R2LoadPoint(x, (float) y); }
inline void R2LoadPoint(float x, double y) { R2LoadPoint((double) x, y); }
inline void R2LoadPoint(double x, int y) { R2LoadPoint(x, (double) y); }
inline void R2LoadPoint(double x, float y) { R2LoadPoint(x, (double) y); }

inline void R3LoadPoint(int x, int y, float z) { R3LoadPoint((float) x, (float) y, z); }
inline void R3LoadPoint(int x, int y, double z) { R3LoadPoint((double) x, (double) y, z); }
inline void R3LoadPoint(int x, float y, int z) { R3LoadPoint((float) x, y, (float) z); }
inline void R3LoadPoint(int x, float y, float z) { R3LoadPoint((float) x, y, z); }
inline void R3LoadPoint(int x, float y, double z) { R3LoadPoint((double) x, (double) y, z); }
inline void R3LoadPoint(int x, double y, int z) { R3LoadPoint((double) x, y, (double) z); }
inline void R3LoadPoint(int x, double y, float z) { R3LoadPoint((double) x, y, (double) z); }
inline void R3LoadPoint(int x, double y, double z) { R3LoadPoint((double) x, y, z); }

inline void R3LoadPoint(float x, int y, int z) { R3LoadPoint(x, (float) y, (float) z); }
inline void R3LoadPoint(float x, int y, float z) { R3LoadPoint(x, (float) y, z); }
inline void R3LoadPoint(float x, int y, double z) { R3LoadPoint((double) x, (double) y, z); }
inline void R3LoadPoint(float x, float y, int z) { R3LoadPoint(x, y, (float) z); }
inline void R3LoadPoint(float x, float y, double z) { R3LoadPoint((double) x, (double) y, z); }
inline void R3LoadPoint(float x, double y, int z) { R3LoadPoint((double) x, y, (double) z); }
inline void R3LoadPoint(float x, double y, float z) { R3LoadPoint((double) x, y, (double) z); }
inline void R3LoadPoint(float x, double y, double z) { R3LoadPoint((double) x, y, z); }

inline void R3LoadPoint(double x, int y, int z) { R3LoadPoint(x, (double) y, (double) z); }
inline void R3LoadPoint(double x, int y, float z) { R3LoadPoint(x, (double) y, (double) z); }
inline void R3LoadPoint(double x, int y, double z) { R3LoadPoint(x, (double) y, z); }
inline void R3LoadPoint(double x, float y, int z) { R3LoadPoint(x, (double) y, (double) z); }
inline void R3LoadPoint(double x, float y, float z) { R3LoadPoint(x, (double) y, (double) z); }
inline void R3LoadPoint(double x, float y, double z) { R3LoadPoint(x, (double) y, z); }
inline void R3LoadPoint(double x, double y, int z) { R3LoadPoint(x, y, (double) z); }
inline void R3LoadPoint(double x, double y, float z) { R3LoadPoint(x, y, (double) z); }


inline void R3LoadNormal(int x, int y, int z) { R3LoadNormal((double) x, (double) y, (double) z); }
inline void R3LoadNormal(int x, int y, float z) { R3LoadNormal((float) x, (float) y, z); }
inline void R3LoadNormal(int x, int y, double z) { R3LoadNormal((double) x, (double) y, z); }
inline void R3LoadNormal(int x, float y, int z) { R3LoadNormal((float) x, y, (float) z); }
inline void R3LoadNormal(int x, float y, float z) { R3LoadNormal((float) x, y, z); }
inline void R3LoadNormal(int x, float y, double z) { R3LoadNormal((double) x, (double) y, z); }
inline void R3LoadNormal(int x, double y, int z) { R3LoadNormal((double) x, y, (double) z); }
inline void R3LoadNormal(int x, double y, float z) { R3LoadNormal((double) x, y, (double) z); }
inline void R3LoadNormal(int x, double y, double z) { R3LoadNormal((double) x, y, z); }

inline void R3LoadNormal(float x, int y, int z) { R3LoadNormal(x, (float) y, (float) z); }
inline void R3LoadNormal(float x, int y, float z) { R3LoadNormal(x, (float) y, z); }
inline void R3LoadNormal(float x, int y, double z) { R3LoadNormal((double) x, (double) y, z); }
inline void R3LoadNormal(float x, float y, int z) { R3LoadNormal(x, y, (float) z); }
inline void R3LoadNormal(float x, float y, double z) { R3LoadNormal((double) x, (double) y, z); }
inline void R3LoadNormal(float x, double y, int z) { R3LoadNormal((double) x, y, (double) z); }
inline void R3LoadNormal(float x, double y, float z) { R3LoadNormal((double) x, y, (double) z); }
inline void R3LoadNormal(float x, double y, double z) { R3LoadNormal((double) x, y, z); }

inline void R3LoadNormal(double x, int y, int z) { R3LoadNormal(x, (double) y, (double) z); }
inline void R3LoadNormal(double x, int y, float z) { R3LoadNormal(x, (double) y, (double) z); }
inline void R3LoadNormal(double x, int y, double z) { R3LoadNormal(x, (double) y, z); }
inline void R3LoadNormal(double x, float y, int z) { R3LoadNormal(x, (double) y, (double) z); }
inline void R3LoadNormal(double x, float y, float z) { R3LoadNormal(x, (double) y, (double) z); }
inline void R3LoadNormal(double x, float y, double z) { R3LoadNormal(x, (double) y, z); }
inline void R3LoadNormal(double x, double y, int z) { R3LoadNormal(x, y, (double) z); }
inline void R3LoadNormal(double x, double y, float z) { R3LoadNormal(x, y, (double) z); }


inline void R3LoadTextureCoords(int x, float y) { R3LoadTextureCoords((float) x, y); }
inline void R3LoadTextureCoords(int x, double y) { R3LoadTextureCoords((double) x, y); }
inline void R3LoadTextureCoords(float x, int y) { R3LoadTextureCoords(x, (float) y); }
inline void R3LoadTextureCoords(float x, double y) { R3LoadTextureCoords((double) x, y); }
inline void R3LoadTextureCoords(double x, int y) { R3LoadTextureCoords(x, (double) y); }
inline void R3LoadTextureCoords(double x, float y) { R3LoadTextureCoords(x, (double) y); }


inline void RNLoadRgb(int r, int g, int b) { RNLoadRgb((float) r, (float) g, (float) b); }
inline void RNLoadRgb(int r, int g, float b) { RNLoadRgb((float) r, (float) g, b); }
inline void RNLoadRgb(int r, int g, double b) { RNLoadRgb((double) r, (double) g, b); }
inline void RNLoadRgb(int r, float g, int b) { RNLoadRgb((float) r, g, (float) b); }
inline void RNLoadRgb(int r, float g, float b) { RNLoadRgb((float) r, g, b); }
inline void RNLoadRgb(int r, float g, double b) { RNLoadRgb((double) r, (double) g, b); }
inline void RNLoadRgb(int r, double g, int b) { RNLoadRgb((double) r, g, (double) b); }
inline void RNLoadRgb(int r, double g, float b) { RNLoadRgb((double) r, g, (double) b); }
inline void RNLoadRgb(int r, double g, double b) { RNLoadRgb((double) r, g, b); }

inline void RNLoadRgb(float r, int g, int b) { RNLoadRgb(r, (float) g, (float) b); }
inline void RNLoadRgb(float r, int g, float b) { RNLoadRgb(r, (float) g, b); }
inline void RNLoadRgb(float r, int g, double b) { RNLoadRgb((double) r, (double) g, b); }
inline void RNLoadRgb(float r, float g, int b) { RNLoadRgb(r, g, (float) b); }
inline void RNLoadRgb(float r, float g, double b) { RNLoadRgb((double) r, (double) g, b); }
inline void RNLoadRgb(float r, double g, int b) { RNLoadRgb((double) r, g, (double) b); }
inline void RNLoadRgb(float r, double g, float b) { RNLoadRgb((double) r, g, (double) b); }
inline void RNLoadRgb(float r, double g, double b) { RNLoadRgb((double) r, g, b); }

inline void RNLoadRgb(double r, int g, int b) { RNLoadRgb(r, (double) g, (double) b); }
inline void RNLoadRgb(double r, int g, float b) { RNLoadRgb(r, (double) g, (double) b); }
inline void RNLoadRgb(double r, int g, double b) { RNLoadRgb(r, (double) g, b); }
inline void RNLoadRgb(double r, float g, int b) { RNLoadRgb(r, (double) g, (double) b); }
inline void RNLoadRgb(double r, float g, float b) { RNLoadRgb(r, (double) g, (double) b); }
inline void RNLoadRgb(double r, float g, double b) { RNLoadRgb(r, (double) g, b); }
inline void RNLoadRgb(double r, double g, int b) { RNLoadRgb(r, g, (double) b); }
inline void RNLoadRgb(double r, double g, float b) { RNLoadRgb(r, g, (double) b); }


inline void RNLoadRgba(int r, int g, int b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(int r, int g, float b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(int r, int g, double b, double a) { RNLoadRgba((double) r, (double) g, b, a); }
inline void RNLoadRgba(int r, float g, int b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(int r, float g, float b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(int r, float g, double b, double a) { RNLoadRgba((double) r, (double) g, b, a); }
inline void RNLoadRgba(int r, double g, int b, double a) { RNLoadRgba((double) r, g, (double) b, a); }
inline void RNLoadRgba(int r, double g, float b, double a) { RNLoadRgba((double) r, g, (double) b, a); }
inline void RNLoadRgba(int r, double g, double b, double a) { RNLoadRgba((double) r, g, b, a); }

inline void RNLoadRgba(float r, int g, int b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(float r, int g, float b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(float r, int g, double b, double a) { RNLoadRgba((double) r, (double) g, b, a); }
inline void RNLoadRgba(float r, float g, int b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(float r, float g, float b, double a) { RNLoadRgba((double) r, (double) g, (double) b, a); }
inline void RNLoadRgba(float r, float g, double b, double a) { RNLoadRgba((double) r, (double) g, b, a); }
inline void RNLoadRgba(float r, double g, int b, double a) { RNLoadRgba((double) r, g, (double) b, a); }
inline void RNLoadRgba(float r, double g, float b, double a) { RNLoadRgba((double) r, g, (double) b, a); }
inline void RNLoadRgba(float r, double g, double b, double a) { RNLoadRgba((double) r, g, b, a); }

inline void RNLoadRgba(double r, int g, int b, double a) { RNLoadRgba(r, (double) g, (double) b, a); }
inline void RNLoadRgba(double r, int g, float b, double a) { RNLoadRgba(r, (double) g, (double) b, a); }
inline void RNLoadRgba(double r, int g, double b, double a) { RNLoadRgba(r, (double) g, b, a); }
inline void RNLoadRgba(double r, float g, int b, double a) { RNLoadRgba(r, (double) g, (double) b, a); }
inline void RNLoadRgba(double r, float g, float b, double a) { RNLoadRgba(r, (double) g, (double) b, a); }
inline void RNLoadRgba(double r, float g, double b, double a) { RNLoadRgba(r, (double) g, b, a); }
inline void RNLoadRgba(double r, double g, int b, double a) { RNLoadRgba(r, g, (double) b, a); }
inline void RNLoadRgba(double r, double g, float b, double a) { RNLoadRgba(r, g, (double) b, a); }



// End namespace
}


// End include guard
#endif
