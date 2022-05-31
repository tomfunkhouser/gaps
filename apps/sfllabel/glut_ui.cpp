/* Source file for GLUT UI */
#ifndef USE_GLFW



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils/R3Utils.h"
#include "fglut/fglut.h"
#include "io.h"
#include "ui.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {
  


////////////////////////////////////////////////////////////////////////
// GLUT variables
////////////////////////////////////////////////////////////////////////

static int GLUTwindow = 0;
static int GLUTwindow_width = 1024;
static int GLUTwindow_height = 768;
static int GLUTmodifiers = 0;
static R3SurfelLabeler *GLUTlabeler = NULL;



////////////////////////////////////////////////////////////////////////
// GLUT callback functions
////////////////////////////////////////////////////////////////////////

void GLUTStop(void)
{
  // Terminate labeler
  if (GLUTlabeler) GLUTlabeler->Terminate();

  // Close scene
  R3SurfelScene *scene = GLUTlabeler->Scene();
  if (scene) CloseScene(scene, TRUE);

  // Exit
  exit(0);
}



void GLUTRedraw(void)
{
  // Redraw with GLUTlabeler 
  if (GLUTlabeler->Redraw()) {
    glutPostRedisplay();
  }

  // Swap buffers 
  glutSwapBuffers();

  // Check for error
  int status = glGetError();
  if (status != GL_NO_ERROR) {
    printf("GL Error: %d\n", status);
  }
}    



void GLUTResize(int w, int h)
{
  // Remember window dimensions
  GLUTwindow_width = w;
  GLUTwindow_height = h;

  // Resize GLUTlabeler
  if (GLUTlabeler->Resize(w, h)) {
    glutPostRedisplay();
  }
}



void GLUTMouseMotion(int x, int y)
{
  // Invert y coordinate
  y = GLUTwindow_height - y;
  
  // Send mouse motion to GLUTlabeler
  if (GLUTlabeler->MouseMotion(x, y)) {
    glutPostRedisplay();
  }
}



void GLUTMouseButton(int button, int state, int x, int y)
{
  // Invert y coordinate
  y = GLUTwindow_height - y;
  
  // Determine button
  int b = button;
  if (button == GLUT_LEFT_BUTTON) b = 0;
  else if (button == GLUT_MIDDLE_BUTTON) b = 1;
  else if (button == GLUT_RIGHT_BUTTON) b = 2;

  // Determine state
  int s = (state == GLUT_DOWN) ? 1 : 0;

  // Determine modifiers
  GLUTmodifiers = glutGetModifiers();
  int shift = (GLUTmodifiers & GLUT_ACTIVE_SHIFT);
  int ctrl = ( GLUTmodifiers & GLUT_ACTIVE_CTRL);
  int alt = ( GLUTmodifiers & GLUT_ACTIVE_ALT);

  // Send mouse event to GLUTlabeler
  if (GLUTlabeler->MouseButton(x, y, b, s, shift, ctrl, alt)) {
    glutPostRedisplay();
  }
}



void GLUTSpecial(int key, int x, int y)
{
  // Invert y coordinate
  y = GLUTwindow_height - y;

  // Determine modifiers
  GLUTmodifiers = glutGetModifiers();
  int shift = (GLUTmodifiers & GLUT_ACTIVE_SHIFT);
  int ctrl = (GLUTmodifiers & GLUT_ACTIVE_CTRL);
  int alt = (GLUTmodifiers & GLUT_ACTIVE_ALT);

  // Translate key
  int translated_key = key;
  switch (key) {
    case GLUT_KEY_DOWN: translated_key = R3_SURFEL_VIEWER_DOWN_KEY; break;
    case GLUT_KEY_UP: translated_key = R3_SURFEL_VIEWER_UP_KEY; break;
    case GLUT_KEY_LEFT: translated_key = R3_SURFEL_VIEWER_LEFT_KEY; break;
    case GLUT_KEY_RIGHT: translated_key = R3_SURFEL_VIEWER_RIGHT_KEY; break;
    case GLUT_KEY_PAGE_DOWN: translated_key = R3_SURFEL_VIEWER_PAGE_DOWN_KEY; break;
    case GLUT_KEY_PAGE_UP: translated_key = R3_SURFEL_VIEWER_PAGE_UP_KEY; break;
    case GLUT_KEY_HOME: translated_key = R3_SURFEL_VIEWER_HOME_KEY; break;
    case GLUT_KEY_END: translated_key = R3_SURFEL_VIEWER_END_KEY; break;
    case GLUT_KEY_INSERT: translated_key = R3_SURFEL_VIEWER_INSERT_KEY; break;
    case GLUT_KEY_F1: translated_key = R3_SURFEL_VIEWER_F1_KEY; break;
    case GLUT_KEY_F2: translated_key = R3_SURFEL_VIEWER_F2_KEY; break;
    case GLUT_KEY_F3: translated_key = R3_SURFEL_VIEWER_F3_KEY; break;
    case GLUT_KEY_F4: translated_key = R3_SURFEL_VIEWER_F4_KEY; break;
    case GLUT_KEY_F5: translated_key = R3_SURFEL_VIEWER_F5_KEY; break;
    case GLUT_KEY_F6: translated_key = R3_SURFEL_VIEWER_F6_KEY; break;
    case GLUT_KEY_F7: translated_key = R3_SURFEL_VIEWER_F7_KEY; break;
    case GLUT_KEY_F8: translated_key = R3_SURFEL_VIEWER_F8_KEY; break;
    case GLUT_KEY_F9: translated_key = R3_SURFEL_VIEWER_F9_KEY; break;
    case GLUT_KEY_F10: translated_key = R3_SURFEL_VIEWER_F10_KEY; break;
    case GLUT_KEY_F11: translated_key = R3_SURFEL_VIEWER_F11_KEY; break;
    case GLUT_KEY_F12: translated_key = R3_SURFEL_VIEWER_F12_KEY; break;
  }

  // Send keyboard event to GLUTlabeler
  if (GLUTlabeler->Keyboard(x, y, translated_key, shift, ctrl, alt)) {
    glutPostRedisplay();
  }
}



void GLUTKeyboard(unsigned char key, int x, int y)
{
  // Invert y coordinate
  y = GLUTwindow_height - y;

  // Determine modifiers
  GLUTmodifiers = glutGetModifiers();
  int shift = (GLUTmodifiers & GLUT_ACTIVE_SHIFT);
  int ctrl = (GLUTmodifiers & GLUT_ACTIVE_CTRL);
  int alt = (GLUTmodifiers & GLUT_ACTIVE_ALT);

  // Convert control keys 
  int translated_key = key;
  if ((key >= 1) && (key <= 26)) {
    if (shift) translated_key += 64;
    else translated_key += 96;
    ctrl = GLUT_ACTIVE_CTRL;
  }
  
  // Send keyboard event to GLUTlabeler
  if (GLUTlabeler->Keyboard(x, y, translated_key, shift, ctrl, alt)) {
    glutPostRedisplay();
  }
}



////////////////////////////////////////////////////////////////////////
// Top-level UI function
////////////////////////////////////////////////////////////////////////

void UIInterface(R3SurfelLabeler *labeler)
{
  // Remember labeler so can access from global callback functions
  GLUTlabeler = labeler;
  
  // Open window
  int argc = 1;
  char *argv[1];
  argv[0] = strdup("sfllabel");
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(GLUTwindow_width, GLUTwindow_height);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_ALPHA);
  GLUTwindow = glutCreateWindow("Surfel Labeler");

  // Initialize grfx (after create context because calls glewInit)
  RNInitGrfx();

  // Initialize GLUT callback functions
  glutDisplayFunc(GLUTRedraw);
  glutReshapeFunc(GLUTResize);
  glutKeyboardFunc(GLUTKeyboard);
  glutSpecialFunc(GLUTSpecial);
  glutMouseFunc(GLUTMouseButton);
  glutMotionFunc(GLUTMouseMotion);
  glutPassiveMotionFunc(GLUTMouseMotion);
  atexit(GLUTStop);

  // Initialize labeler
  labeler->Initialize();

  // Run main loop -- never returns
  glutMainLoop();
}



}; // end namespace



#endif
