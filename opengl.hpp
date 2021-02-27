#ifdef __APPLE__
#include <sys/time.h>
#else
#include <windows.h>
#endif

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif