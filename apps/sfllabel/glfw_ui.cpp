/* Source file for GLUT UI */
#ifdef USE_GLFW



////////////////////////////////////////////////////////////////////////
// Include files
////////////////////////////////////////////////////////////////////////

#include "R3Utils/R3Utils.h"
#include <GLFW/glfw3.h>
#include "io.h"
#include "ui.h"



////////////////////////////////////////////////////////////////////////
// Namespace
////////////////////////////////////////////////////////////////////////

namespace gaps {
  



/////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////

static R3SurfelLabeler *GLFWlabeler = NULL;
static bool GLFWneeds_redraw = true;



/////////////////////////////////////////////////////////////////////////
// Callback functions
////////////////////////////////////////////////////////////////////////

static void
GLFWError(int err, const char *str)
{
  printf("GLFW Error: %d %s\n", err, str);
}



static void
GLFWPostRedisplay(void)
{
  // Remember that needs redraw
  GLFWneeds_redraw = true;
}



static void
GLFWRedraw(GLFWwindow *window)
{
  // Check if need redraw
  if (!GLFWneeds_redraw) return;
  
  // Redraw with labeler 
  if (GLFWlabeler->Redraw()) {
    GLFWPostRedisplay();
  }

  // Swap buffers
  glfwSwapBuffers(window);
  
  // Check for error
  int status = glGetError();
  if (status != GL_NO_ERROR) {
    printf("GL Error: %d\n", status);
  }

  // Remember that redraw is done
  GLFWneeds_redraw = false;
}




static void
GLFWResize(GLFWwindow *window,
  int width, int height)
{
  // Resize labeler
  if (GLFWlabeler->Resize(width, height)) {
    GLFWPostRedisplay();
  }
}



static void
GLFWMotion(GLFWwindow *window,
  double xpos, double ypos)
{
  // Get cursor position
  int window_width, window_height;
  int framebuffer_width, framebuffer_height;
  glfwGetCursorPos(window, &xpos, &ypos);
  glfwGetWindowSize(window, &window_width, &window_height);
  glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
  xpos *= (double) framebuffer_width / (double) window_width;
  ypos *= (double) framebuffer_height / (double) window_height;
  int x = xpos + 0.5;
  int y = ypos + 0.5;
  y = framebuffer_height - y;
  
  // Send mouse motion to labeler
  if (GLFWlabeler->MouseMotion(x, y)) {
    GLFWPostRedisplay();
  }
}



static void
GLFWButton(GLFWwindow *window,
int button, int action, int mods)
{
  // Get state
  int state = -1;
  if (action == GLFW_PRESS) state = 1;
  else if (action == GLFW_RELEASE) state = 0;
  else return;

  // Get button
  if (button == 1) button = 2;
  else if (button == 2) button = 1;
  
  // Get modifiers
  int shift = (mods & GLFW_MOD_SHIFT);
  int ctrl = (mods & GLFW_MOD_CONTROL);
  int alt = (mods & GLFW_MOD_ALT);
  // if (mods & GLFW_MOD_CAPS_LOCK) shift = !shift;
  
  // Map space -> alt
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) alt = 1;

  // Get cursor position
  double xpos, ypos;
  int window_width, window_height;
  int framebuffer_width, framebuffer_height;
  glfwGetCursorPos(window, &xpos, &ypos);
  glfwGetWindowSize(window, &window_width, &window_height);
  glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
  xpos *= (double) framebuffer_width / (double) window_width;
  ypos *= (double) framebuffer_height / (double) window_height;
  int x = xpos + 0.5;
  int y = ypos + 0.5;
  y = framebuffer_height - y;

  // Call callback
  // Send mouse event to labeler
  if (GLFWlabeler->MouseButton(x, y, button, state, shift, ctrl, alt)) {
    GLFWPostRedisplay();
  }
}


         
static void
GLFWScroll(GLFWwindow *window,
  double xoffset, double yoffset)
{
  // Get button
  int button = (yoffset < 0) ? 3 : 4;

  // Get cursor position
  double xpos, ypos;
  int window_width, window_height;
  int framebuffer_width, framebuffer_height;
  glfwGetCursorPos(window, &xpos, &ypos);
  glfwGetWindowSize(window, &window_width, &window_height);
  glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
  xpos *= (double) framebuffer_width / (double) window_width;
  ypos *= (double) framebuffer_height / (double) window_height;
  int x = xpos + 0.5;
  int y = ypos + 0.5;
  y = framebuffer_height - y;
  
  // Set event to labeler
  if (GLFWlabeler->MouseButton(x, y, button, 1, 0, 0, 0)) {
    GLFWPostRedisplay();
  }
}


         
static void
GLFWKeyboard(GLFWwindow *window,
  int key, int scancode, int action, int mods)
{
  // Check action
  if (action == GLFW_RELEASE) return;

  // Get modifiers
  int shift = (mods & GLFW_MOD_SHIFT);
  int ctrl = (mods & GLFW_MOD_CONTROL);
  int alt = (mods & GLFW_MOD_ALT);
  // if (mods & GLFW_MOD_CAPS_LOCK) shift = !shift;
  
  // Map space -> alt
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) alt = 1;
  if (key == GLFW_KEY_SPACE) return;
  
  // Get tab key status
  int tab =(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) ? 1 : 0;
  
  // Get cursor position
  double xpos, ypos;
  int window_width, window_height;
  int framebuffer_width, framebuffer_height;
  glfwGetCursorPos(window, &xpos, &ypos);
  glfwGetWindowSize(window, &window_width, &window_height);
  glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
  xpos *= (double) framebuffer_width / (double) window_width;
  ypos *= (double) framebuffer_height / (double) window_height;
  int x = xpos + 0.5;
  int y = ypos + 0.5;
  y = framebuffer_height - y;

  // Translated key
  int translated_key = key;
  switch (key) {
  case GLFW_KEY_A: translated_key = (shift) ? 'A' : 'a'; break;
  case GLFW_KEY_B: translated_key = (shift) ? 'B' : 'b'; break;
  case GLFW_KEY_C: translated_key = (shift) ? 'C' : 'c'; break;
  case GLFW_KEY_D: translated_key = (shift) ? 'D' : 'd'; break;
  case GLFW_KEY_E: translated_key = (shift) ? 'E' : 'e'; break;
  case GLFW_KEY_F: translated_key = (shift) ? 'F' : 'f'; break;
  case GLFW_KEY_G: translated_key = (shift) ? 'G' : 'g'; break;
  case GLFW_KEY_H: translated_key = (shift) ? 'H' : 'h'; break;
  case GLFW_KEY_I: translated_key = (shift) ? 'I' : 'i'; break;
  case GLFW_KEY_J: translated_key = (shift) ? 'J' : 'j'; break;
  case GLFW_KEY_K: translated_key = (shift) ? 'K' : 'k'; break;
  case GLFW_KEY_L: translated_key = (shift) ? 'L' : 'l'; break;
  case GLFW_KEY_M: translated_key = (shift) ? 'M' : 'm'; break;
  case GLFW_KEY_N: translated_key = (shift) ? 'N' : 'n'; break;
  case GLFW_KEY_O: translated_key = (shift) ? 'O' : 'o'; break;
  case GLFW_KEY_P: translated_key = (shift) ? 'P' : 'p'; break;
  case GLFW_KEY_Q: translated_key = (shift) ? 'Q' : 'q'; break;
  case GLFW_KEY_R: translated_key = (shift) ? 'R' : 'r'; break;
  case GLFW_KEY_S: translated_key = (shift) ? 'S' : 's'; break;
  case GLFW_KEY_T: translated_key = (shift) ? 'T' : 't'; break;
  case GLFW_KEY_U: translated_key = (shift) ? 'U' : 'u'; break;
  case GLFW_KEY_V: translated_key = (shift) ? 'V' : 'v'; break;
  case GLFW_KEY_W: translated_key = (shift) ? 'W' : 'w'; break;
  case GLFW_KEY_X: translated_key = (shift) ? 'X' : 'x'; break;
  case GLFW_KEY_Y: translated_key = (shift) ? 'Y' : 'y'; break;
  case GLFW_KEY_Z: translated_key = (shift) ? 'Z' : 'z'; break;
  case GLFW_KEY_1: translated_key = (shift) ? '1' : '!'; break;
  case GLFW_KEY_2: translated_key = (shift) ? '2' : '@'; break;
  case GLFW_KEY_3: translated_key = (shift) ? '3' : '#'; break;
  case GLFW_KEY_4: translated_key = (shift) ? '4' : '$'; break;
  case GLFW_KEY_5: translated_key = (shift) ? '5' : '%'; break;
  case GLFW_KEY_6: translated_key = (shift) ? '6' : '^'; break;
  case GLFW_KEY_7: translated_key = (shift) ? '7' : '&'; break;
  case GLFW_KEY_8: translated_key = (shift) ? '8' : '*'; break;
  case GLFW_KEY_9: translated_key = (shift) ? '9' : '('; break;
  case GLFW_KEY_0: translated_key = (shift) ? '0' : ')'; break;
  case GLFW_KEY_GRAVE_ACCENT: translated_key = (shift) ? '~' : '`'; break;
  case GLFW_KEY_MINUS: translated_key = (shift) ? '_' : '-'; break;
  case GLFW_KEY_EQUAL: translated_key = (shift) ? '+' : '='; break;
  case GLFW_KEY_LEFT_BRACKET: translated_key = (shift) ? '{' : '['; break;
  case GLFW_KEY_RIGHT_BRACKET: translated_key = (shift) ? '}' : ']'; break;
  case GLFW_KEY_SEMICOLON: translated_key = (shift) ? ':' : ';'; break;
  case GLFW_KEY_APOSTROPHE: translated_key = (shift) ? '\"' : '\''; break;
  case GLFW_KEY_COMMA: translated_key = (shift) ? '<' : ','; break;
  case GLFW_KEY_PERIOD: translated_key = (shift) ? '>' : '.'; break;
  case GLFW_KEY_SLASH: translated_key = (shift) ? '?' : '/'; break;
  case GLFW_KEY_DOWN: translated_key = R3_SURFEL_VIEWER_DOWN_KEY; break;
  case GLFW_KEY_UP: translated_key = R3_SURFEL_VIEWER_UP_KEY; break;
  case GLFW_KEY_LEFT: translated_key = R3_SURFEL_VIEWER_LEFT_KEY; break;
  case GLFW_KEY_RIGHT: translated_key = R3_SURFEL_VIEWER_RIGHT_KEY; break;
  case GLFW_KEY_PAGE_DOWN: translated_key = R3_SURFEL_VIEWER_PAGE_DOWN_KEY; break;
  case GLFW_KEY_PAGE_UP: translated_key = R3_SURFEL_VIEWER_PAGE_UP_KEY; break;
  case GLFW_KEY_HOME: translated_key = R3_SURFEL_VIEWER_HOME_KEY; break;
  case GLFW_KEY_END: translated_key = R3_SURFEL_VIEWER_END_KEY; break;
  case GLFW_KEY_INSERT: translated_key = R3_SURFEL_VIEWER_INSERT_KEY; break;
  case GLFW_KEY_ESCAPE: translated_key = R3_SURFEL_VIEWER_ESC_KEY; break;
  case GLFW_KEY_SPACE: translated_key = R3_SURFEL_VIEWER_SPACE_KEY; break;
  case GLFW_KEY_DELETE: translated_key = R3_SURFEL_VIEWER_DEL_KEY; break;
  case GLFW_KEY_ENTER: translated_key = 'M'; ctrl = 1; break; 
  case GLFW_KEY_F1: translated_key = R3_SURFEL_VIEWER_F1_KEY; break;
  case GLFW_KEY_F2: translated_key = R3_SURFEL_VIEWER_F2_KEY; break;
  case GLFW_KEY_F3: translated_key = R3_SURFEL_VIEWER_F3_KEY; break;
  case GLFW_KEY_F4: translated_key = R3_SURFEL_VIEWER_F4_KEY; break;
  case GLFW_KEY_F5: translated_key = R3_SURFEL_VIEWER_F5_KEY; break;
  case GLFW_KEY_F6: translated_key = R3_SURFEL_VIEWER_F6_KEY; break;
  case GLFW_KEY_F7: translated_key = R3_SURFEL_VIEWER_F7_KEY; break;
  case GLFW_KEY_F8: translated_key = R3_SURFEL_VIEWER_F8_KEY; break;
  case GLFW_KEY_F9: translated_key = R3_SURFEL_VIEWER_F9_KEY; break;
  case GLFW_KEY_F10: translated_key = R3_SURFEL_VIEWER_F10_KEY; break;
  case GLFW_KEY_F11: translated_key = R3_SURFEL_VIEWER_F11_KEY; break;
  case GLFW_KEY_F12: translated_key = R3_SURFEL_VIEWER_F12_KEY; break;
  }

  // Send keyboard event to labeler
  if (GLFWlabeler->Keyboard(x, y, translated_key, shift, ctrl, alt, tab)) {
    GLFWPostRedisplay();
  }
}



/////////////////////////////////////////////////////////////////////////
// Top-level UI function
////////////////////////////////////////////////////////////////////////

void
UIInterface(R3SurfelLabeler *labeler)
{
  // Remember labeler so that can access from global callback functions
  GLFWlabeler = labeler;
  
  // Initialize the library
  if (!glfwInit()) {
    printf("Unable to initialize GLFW\n");
    return;
  }
  
  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window = glfwCreateWindow(1024, 768, "sfllabel", NULL, NULL);
  if (!window) {
    printf("Unable to create GLFW window\n");
    glfwTerminate();
    return;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Initialize grfx (after create context because calls glewInit)
  RNInitGrfx();

  // Set callbacks
  glfwSetErrorCallback(GLFWError);
  glfwSetFramebufferSizeCallback(window, GLFWResize);
  glfwSetScrollCallback(window, GLFWScroll);
  glfwSetCursorPosCallback(window, GLFWMotion);
  glfwSetMouseButtonCallback(window, GLFWButton);
  glfwSetKeyCallback(window, GLFWKeyboard);
  glfwSetWindowRefreshCallback(window, GLFWRedraw);

  // Initialize labeler
  labeler->Initialize();

  // Size window
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  GLFWResize(window, width, height);

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    GLFWRedraw(window);
    glfwWaitEvents();
  }

  // Terminate GLFW
  glfwTerminate();

  // Stop packages
  R3StopGraphics();
  R3StopSurfels();
}



}; // end namespace



#endif

