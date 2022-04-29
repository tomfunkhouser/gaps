////////////////////////////////////////////////////////////////////////
// Source file for MP utilities
////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "MP.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {



////////////////////////////////////////////////////////////////////////
// UTILITY COLORING FUNCTIONS
////////////////////////////////////////////////////////////////////////

void
LoadIndex(int index, int tag)
{
  // Set color to represent an integer (24 bits)
  int k = index + 1;
  unsigned char color[4];
  color[0] = (k >> 16) & 0xFF;
  color[1] = (k >>  8) & 0xFF;
  color[2] = (k      ) & 0xFF;
  color[3] = tag;
  RNLoadRgba(color);
}



void
LoadColor(int k)
{
  // Make array of colors
  const int ncolors = 72;
  const RNRgb colors[ncolors] = {
    RNRgb(0.5, 0.5, 0.5), RNRgb(1, 0, 0), RNRgb(0, 0, 1), 
    RNRgb(0, 1, 0), RNRgb(0, 1, 1), RNRgb(1, 0, 1), 
    RNRgb(1, 0.5, 0), RNRgb(0, 1, 0.5), RNRgb(0.5, 0, 1), 
    RNRgb(0.5, 1, 0), RNRgb(0, 0.5, 1), RNRgb(1, 0, 0.5), 
    RNRgb(0.5, 0, 0), RNRgb(0, 0.5, 0), RNRgb(0, 0, 0.5), 
    RNRgb(0.5, 0.5, 0), RNRgb(0, 0.5, 0.5), RNRgb(0.5, 0, 0.5),
    RNRgb(0.7, 0, 0), RNRgb(0, 0.7, 0), RNRgb(0, 0, 0.7), 
    RNRgb(0.7, 0.7, 0), RNRgb(0, 0.7, 0.7), RNRgb(0.7, 0, 0.7), 
    RNRgb(0.7, 0.3, 0), RNRgb(0, 0.7, 0.3), RNRgb(0.3, 0, 0.7), 
    RNRgb(0.3, 0.7, 0), RNRgb(0, 0.3, 0.7), RNRgb(0.7, 0, 0.3), 
    RNRgb(0.3, 0, 0), RNRgb(0, 0.3, 0), RNRgb(0, 0, 0.3), 
    RNRgb(0.3, 0.3, 0), RNRgb(0, 0.3, 0.3), RNRgb(0.3, 0, 0.3),
    RNRgb(1, 0.3, 0.3), RNRgb(0.3, 1, 0.3), RNRgb(0.3, 0.3, 1), 
    RNRgb(1, 1, 0.3), RNRgb(0.3, 1, 1), RNRgb(1, 0.3, 1), 
    RNRgb(1, 0.5, 0.3), RNRgb(0.3, 1, 0.5), RNRgb(0.5, 0.3, 1), 
    RNRgb(0.5, 1, 0.3), RNRgb(0.3, 0.5, 1), RNRgb(1, 0.3, 0.5), 
    RNRgb(0.5, 0.3, 0.3), RNRgb(0.3, 0.5, 0.3), RNRgb(0.3, 0.3, 0.5), 
    RNRgb(0.5, 0.5, 0.3), RNRgb(0.3, 0.5, 0.5), RNRgb(0.5, 0.3, 0.5),
    RNRgb(0.3, 0.5, 0.5), RNRgb(0.5, 0.3, 0.5), RNRgb(0.5, 0.5, 0.3), 
    RNRgb(0.3, 0.3, 0.5), RNRgb(0.5, 0.3, 0.3), RNRgb(0.3, 0.5, 0.3), 
    RNRgb(0.3, 0.8, 0.5), RNRgb(0.5, 0.3, 0.8), RNRgb(0.8, 0.5, 0.3), 
    RNRgb(0.8, 0.3, 0.5), RNRgb(0.5, 0.8, 0.3), RNRgb(0.3, 0.5, 0.8), 
    RNRgb(0.8, 0.5, 0.5), RNRgb(0.5, 0.8, 0.5), RNRgb(0.5, 0.5, 0.8), 
    RNRgb(0.8, 0.8, 0.5), RNRgb(0.5, 0.8, 0.8), RNRgb(0.8, 0.5, 0.8)
  };

  // Load color
  if (k == -1) RNLoadRgb(0.8, 0.8, 0.8);
  else if (k == 0) RNLoadRgb(colors[0]);
  else RNLoadRgb(colors[1 + (k % (ncolors-1))]);
}



void
LoadColor(const char *s)
{
  // Load color based on first character of string
  LoadColor((int) ((s) ? s[0] : 0));
}


    
// End namespace
};
