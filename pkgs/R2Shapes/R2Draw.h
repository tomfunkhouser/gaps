/* Include file for R2 draw utility */
#ifndef __R2__DRAW__H__
#define __R2__DRAW__H__



/* Begin namespace */
namespace gaps {


  
inline void 
R2LoadPoint(const R2Point& point)
{
    // Load vertex 
    R2LoadPoint(point.Coords());
}



inline void 
R2DrawText(const R2Point& p, const char *str, void *font = NULL)
{
    // Draw text at point
    R2DrawText(p.X(), p.Y(), str, font);
}



// End namespace
}


// End include guard
#endif
