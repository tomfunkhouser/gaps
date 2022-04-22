// OpenGL GLUT include file
#ifndef __FGLUT__H__
#define __FGLUT__H__



// External include files

#ifdef __APPLE__
        // Use the native glut on mac
#       include "GLUT/glut.h"
#else
        // Use this implementation 
#       include "fglut/glut.h"
#endif



// Initialization functions

int R3InitGlut(void);
void R3StopGlut(void);



#endif
