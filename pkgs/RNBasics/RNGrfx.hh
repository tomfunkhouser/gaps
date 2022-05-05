// This file has inline functions declared in RNGrfx.h



////////////////////////////////////////////////////////////
// Global Xlib variables
////////////////////////////////////////////////////////////

#if (RN_2D_GRFX == RN_XLIB)
    extern Display *RNgrfx_xdisplay;
    extern Window RNgrfx_xwindow;
    extern GC RNgrfx_xgc;
    extern XPoint RNgrfx_xpoints[];
    extern int RNgrfx_xnpoints;
#endif




////////////////////////////////////////////////////////////
// Window-viewport transformation functions 
////////////////////////////////////////////////////////////

extern float RNgrfx_window_xscale;
extern float RNgrfx_window_yscale;
extern float RNgrfx_window_width;
extern float RNgrfx_window_height;
extern float RNgrfx_window_xcenter;
extern float RNgrfx_window_ycenter;
extern float RNgrfx_window_xmin;
extern float RNgrfx_window_ymin;
extern float RNgrfx_window_xmax;
extern float RNgrfx_window_ymax;
extern int RNgrfx_viewport_width;
extern int RNgrfx_viewport_height;
extern int RNgrfx_viewport_xcenter;
extern int RNgrfx_viewport_ycenter;
extern int RNgrfx_viewport_xmin;
extern int RNgrfx_viewport_ymin;
extern int RNgrfx_viewport_xmax;
extern int RNgrfx_viewport_ymax;



inline void
R2WindowToViewport(float wx, float wy, int *vx, int *vy)
{
    // Transform x coordinate by window/viewport
    *vx = RNgrfx_viewport_xmin + (int) ((wx - RNgrfx_window_xmin) * RNgrfx_window_xscale);
    *vy = RNgrfx_viewport_ymin + (int) ((wy - RNgrfx_window_ymin) * RNgrfx_window_yscale);
    // *vy = RNgrfx_viewport_ymax - *vy;
}
	


inline void
R2WindowToViewport(double wx, double wy, int *vx, int *vy)
{
    // Transform x coordinate by window/viewport
    *vx = RNgrfx_viewport_xmin + (int) ((wx - RNgrfx_window_xmin) * RNgrfx_window_xscale);
    *vy = RNgrfx_viewport_ymin + (int) ((wy - RNgrfx_window_ymin) * RNgrfx_window_yscale);
    // *vy = RNgrfx_viewport_ymax - *vy;
}
	


inline void
R2WindowToViewport(float wx, float wy, short *vx, short *vy)
{
    // Transform x coordinate by window/viewport
    *vx = (short) (RNgrfx_viewport_xmin + (wx - RNgrfx_window_xmin) * RNgrfx_window_xscale);
    *vy = (short) (RNgrfx_viewport_ymin + (wy - RNgrfx_window_ymin) * RNgrfx_window_yscale);
    // *vy = RNgrfx_viewport_ymax - *vy;
}
	


inline void
R2WindowToViewport(double wx, double wy, short *vx, short *vy)
{
    // Transform x coordinate by window/viewport
    *vx = (short) (RNgrfx_viewport_xmin + (wx - RNgrfx_window_xmin) * RNgrfx_window_xscale);
    *vy = (short) (RNgrfx_viewport_ymin + (wy - RNgrfx_window_ymin) * RNgrfx_window_yscale);
    // *vy = RNgrfx_viewport_ymax - *vy;
}
	


inline void
R2ViewportToWindow(int vx, int vy, float *wx, float *wy)
{
    // Transform x coordinate by window/viewport
    *wx = RNgrfx_window_xmin + (float) ((vx - RNgrfx_viewport_xmin) / RNgrfx_window_xscale);
    *wy = RNgrfx_window_ymin + (float) ((vy - RNgrfx_viewport_ymin) / RNgrfx_window_yscale);
    // *wy = RNgrfx_window_ymax - (*wy - RNgrfx_window_ymin);
}
	


inline void
R2ViewportToWindow(int vx, int vy, double *wx, double *wy)
{
    // Transform x coordinate by window/viewport
    *wx = RNgrfx_window_xmin + (double) ((vx - RNgrfx_viewport_xmin) / RNgrfx_window_xscale);
    *wy = RNgrfx_window_ymin + (double) ((vy - RNgrfx_viewport_ymin) / RNgrfx_window_yscale);
    // *wy = RNgrfx_window_ymax - (*wy - RNgrfx_window_ymin);
}
	


////////////////////////////////////////////////////////////
// Primitive drawing functions
////////////////////////////////////////////////////////////

inline int
RNGrfxModeToOpenGLMode(int grfx_mode)
{
    // GRFX and OpenGL modes are the same
    return grfx_mode;
}



inline void 
RNGrfxBegin(int mode)
{
    // Begin drawing primitive (follow with R2LoadPoint* and R2EndBuffer)
#if (RN_2D_GRFX == RN_OPENGL)
    glBegin(RNGrfxModeToOpenGLMode(mode));
#elif (RN_2D_GRFX == RN_XLIB)
    RNgrfx_xnpoints = 0;
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void
RNGrfxEnd(void)
{
#if (RN_2D_GRFX == RN_OPENGL)
    glEnd();
#elif (RN_2D_GRFX == RN_XLIB)
    XFillPolygon(RNgrfx_xdisplay, RNgrfx_xwindow, RNgrfx_xgc,
	RNgrfx_xpoints, RNgrfx_xnpoints, Convex, CoordModeOrigin);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R2BeginPolygon(void)
{
    // Begin drawing polygon
    RNGrfxBegin(RN_GRFX_POLYGON);
}



inline void 
R2EndPolygon(void)
{
    // End drawing polygon
    RNGrfxEnd();
}
	


inline void 
R2BeginLine(void)
{
    // Begin drawing line
    RNGrfxBegin(RN_GRFX_LINE_STRIP);
}



inline void 
R2EndLine(void)
{
    // End drawing line
    RNGrfxEnd();
}
	


inline void 
R2BeginLoop(void)
{
    // Begin drawing closed line
    RNGrfxBegin(RN_GRFX_LINE_LOOP);
}



inline void 
R2EndLoop(void)
{
    // End drawing closed line
    RNGrfxEnd();
}



inline void 
R3BeginPolygon(void)
{
    // Begin drawing polygon
    RNGrfxBegin(RN_GRFX_POLYGON);
}



inline void 
R3EndPolygon(void)
{
    // End drawing polygon
    RNGrfxEnd();
}
	


inline void 
R3BeginLine(void)
{
    // Begin drawing line
    RNGrfxBegin(RN_GRFX_LINE_STRIP);
}



inline void 
R3EndLine(void)
{
    // End drawing line
    RNGrfxEnd();
}
	


inline void 
R3BeginLoop(void)
{
    // Begin drawing closed line
    RNGrfxBegin(RN_GRFX_LINE_LOOP);
}



inline void 
R3EndLoop(void)
{
    // End drawing closed line
    RNGrfxEnd();
}
	


////////////////////////////////////////////////////////////
// Data loading functions (between Begin and End)
////////////////////////////////////////////////////////////

inline void 
R2LoadPoint(int x, int y)
{
    // Load vertex (within R2BeginXXX and R2EndXXX)
#if (RN_2D_GRFX == RN_OPENGL)
    glVertex2i(x, y);
#elif (RN_2D_GRFX == RN_XLIB)
    R2WindowToViewport(x, y, &RNgrfx_xpoints[RNgrfx_xnpoints].x, &RNgrfx_xpoints[RNgrfx_xnpoints].y);
    RNgrfx_xnpoints++;
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R2LoadPoint(float x, float y)
{
    // Load vertex (within R2BeginXXX and R2EndXXX)
#if (RN_2D_GRFX == RN_OPENGL)
    glVertex2f(x, y);
#elif (RN_2D_GRFX == RN_XLIB)
    R2WindowToViewport(x, y, &RNgrfx_xpoints[RNgrfx_xnpoints].x, &RNgrfx_xpoints[RNgrfx_xnpoints].y);
    RNgrfx_xnpoints++;
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R2LoadPoint(double x, double y)
{
    // Load vertex (within R2BeginXXX and R2EndXXX)
#if (RN_2D_GRFX == RN_OPENGL)
    glVertex2d(x, y);
#elif (RN_2D_GRFX == RN_XLIB)
    R2WindowToViewport(x, y, &RNgrfx_xpoints[RNgrfx_xnpoints].x, &RNgrfx_xpoints[RNgrfx_xnpoints].y);
    RNgrfx_xnpoints++;
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R2LoadPoint(const int point[2])
{
    // Load vertex (within R2BeginXXX and R2EndXXX)
#if (RN_2D_GRFX == RN_OPENGL)
    glVertex2iv(point);
#elif (RN_2D_GRFX == RN_XLIB)
    R2WindowToViewport(point[0], point[1], &RNgrfx_xpoints[RNgrfx_xnpoints].x, &RNgrfx_xpoints[RNgrfx_xnpoints].y);
    RNgrfx_xnpoints++;
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R2LoadPoint(const float point[2])
{
    // Load vertex (within R2BeginXXX and R2EndXXX)
#if (RN_2D_GRFX == RN_OPENGL)
    glVertex2fv(point);
#elif (RN_2D_GRFX == RN_XLIB)
    R2WindowToViewport(point[0], point[1], &RNgrfx_xpoints[RNgrfx_xnpoints].x, &RNgrfx_xpoints[RNgrfx_xnpoints].y);
    RNgrfx_xnpoints++;
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R2LoadPoint(const double point[2])
{
    // Load vertex (within R2BeginXXX and R2EndXXX)
#if (RN_2D_GRFX == RN_OPENGL)
    glVertex2dv(point);
#elif (RN_2D_GRFX == RN_XLIB)
    R2WindowToViewport(point[0], point[1], &RNgrfx_xpoints[RNgrfx_xnpoints].x, &RNgrfx_xpoints[RNgrfx_xnpoints].y);
    RNgrfx_xnpoints++;
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadPoint(int x, int y, int z)
{
    // Load vertex (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glVertex3i(x, y, z);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadPoint(float x, float y, float z)
{
    // Load vertex (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glVertex3f(x, y, z);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadPoint(double x, double y, double z)
{
    // Load vertex (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glVertex3d(x, y, z);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadPoint(const int point[3])
{
    // Load vertex (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glVertex3iv(point);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadPoint(const float point[3])
{
    // Load vertex (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glVertex3fv(point);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadPoint(const double point[3])
{
    // Load vertex (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glVertex3dv(point);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadNormal(float x, float y, float z)
{
    // Load normal vector 
#if (RN_3D_GRFX == RN_OPENGL)
    glNormal3f(x, y, z);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadNormal(double x, double y, double z)
{
    // Load normal vector 
#if (RN_3D_GRFX == RN_OPENGL)
    glNormal3d(x, y, z);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadNormal(const float normal[3])
{
    // Load normal vector 
#if (RN_3D_GRFX == RN_OPENGL)
    glNormal3fv(normal);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadNormal(const double normal[3])
{
    // Load normal vector 
#if (RN_3D_GRFX == RN_OPENGL)
    glNormal3dv(normal);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadTextureCoords(int x, int y)
{
    // Load texture coordinate (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glTexCoord2i(x, y);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadTextureCoords(float x, float y)
{
    // Load texture coordinate (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glTexCoord2f(x, y);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadTextureCoords(double x, double y)
{
    // Load texture coordinate (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glTexCoord2d(x, y);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadTextureCoords(const int texcoords[2])
{
    // Load texture coordinate (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glTexCoord2iv(texcoords);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadTextureCoords(const float texcoords[2])
{
    // Load texture coordinate (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glTexCoord2fv(texcoords);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
R3LoadTextureCoords(const double texcoords[2])
{
    // Set texture coordinate (within RNGrfxBegin and RNGrfxEnd)
#if (RN_3D_GRFX == RN_OPENGL)
    glTexCoord2dv(texcoords);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgb(unsigned char r, unsigned char g, unsigned char b)
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor3ub(r, g, b);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgb(float r, float g, float b)
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor3f(r, g, b);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgb(double r, double g, double b)
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor3d(r, g, b);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgb(const unsigned char rgb[3])
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor3ubv(rgb);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgb(const float rgb[3])
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor3fv(rgb);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgb(const double rgb[3])
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor3dv(rgb);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4ub(r, g, b, a);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(int r, int g, int b, int a)
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4d(r, g, b, a);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(float r, float g, float b, float a)
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4f(r, g, b, a);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(double r, double g, double b, double a)
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4d(r, g, b, a);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(unsigned char rgba[4])
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4ubv(rgba);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(int rgba[4])
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4d(rgba[0], rgba[1], rgba[2], rgba[3]);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(float rgba[4])
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4fv(rgba);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(double rgba[4])
{
    // Load rgb
#if (RN_3D_GRFX == RN_OPENGL)
    glColor4dv(rgba);
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void 
RNLoadRgba(unsigned int value)
{
  // Load identifer
  unsigned char rgba[4];
  rgba[0] = value & 0xFF;
  rgba[1] = (value >> 8) & 0xFF;
  rgba[2] = (value >> 16) & 0xFF;
  rgba[3] = (value >> 24) & 0xFF;
  RNLoadRgba(rgba);
}


  
////////////////////////////////////////////////////////////
// Image drawing functions
////////////////////////////////////////////////////////////

inline void
R2DrawImage(int x, int y, int width, int height, int depth, const unsigned char *data)
{
#if (RN_3D_GRFX == RN_OPENGL)
    // Set projection matrix
    glMatrixMode(GL_PROJECTION);  
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);

    // Set model view matrix
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Set position for image
    glRasterPos2i(x, y);

    // Determine image format
    GLenum format = GL_LUMINANCE;
    if (depth == 1) format = GL_LUMINANCE;
    else if (depth == 2) format = GL_LUMINANCE_ALPHA;
    else if (depth == 3) format = GL_RGB;
    else if (depth == 4) format = GL_RGBA;
    else RNAbort("Illegal image image");

    // Draw pixels
    glDrawPixels(width, height, format, GL_UNSIGNED_BYTE, data);
  
    // Reset model view matrix
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // Reset projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
#else
    RNGrfxError("Not Implemented");
#endif
}



////////////////////////////////////////////////////////////
// Text drawing functions
////////////////////////////////////////////////////////////

inline void
R2DrawText(double x, double y, const char *str, void *font)
{
#if (RN_2D_GRFX == RN_OPENGL)
#   ifdef __FGLUT__H__
        glRasterPos2d(x, y);
        if (!font) font = GLUT_BITMAP_HELVETICA_12;
        while (*str) glutBitmapCharacter(font, *(str++));
#   endif
#else
    RNGrfxError("Not Implemented");
#endif
}



inline void
R3DrawText(double x, double y, double z, const char *str, void *font)
{
#if (RN_3D_GRFX == RN_OPENGL)
#   ifdef __FGLUT__H__
        glRasterPos3d(x, y, z);
        if (!font) font = GLUT_BITMAP_HELVETICA_12;
        while (*str) glutBitmapCharacter(font, *(str++));
#   endif
#else
    RNGrfxError("Not Implemented");
#endif
}



inline int
RNTextWidth(const char *str, void *font)
{
#if (RN_3D_GRFX == RN_OPENGL)
#   ifdef __FGLUT__H__
        // Return width of text in pixels
        return glutBitmapLength(font, (const unsigned char *) str);
#   else
        // Just a guess (could do better by switching on font)
        return 12 * strlen(str);
#   endif
#else
    RNGrfxError("Not Implemented");
#endif
}
