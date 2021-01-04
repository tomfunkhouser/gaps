////////////////////////////////////////////////////////////////////////
// Text drawing utility functions
////////////////////////////////////////////////////////////////////////

#include "fglut/fglut.h"

inline void 
DrawText(const R2Point& p, const char *s, void *font = GLUT_BITMAP_HELVETICA_12)
{
  // Draw text string s and position p
  glRasterPos2d(p[0], p[1]);
  while (*s) glutBitmapCharacter(font, *(s++));
}
  


#if 0

inline void 
DrawText(const R3Point& p, const char *s, void *font = GLUT_BITMAP_HELVETICA_12)
{
  // Draw text string s and position p
  glRasterPos3d(p[0], p[1], p[2]);
  while (*s) glutBitmapCharacter(font, *(s++));
}
  
#endif


