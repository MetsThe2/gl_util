#ifndef GL_TYPES_H_INCLUDED
#define GL_TYPES_H_INCLUDED

#define GLEW_STATIC
#include <gl/glew.h>

typedef GLuint BufIndex; // Make sure the typedef and the define match.
#define BUF_INDEX_TYPE GL_UNSIGNED_INT

typedef struct glf2 {
    GLfloat x;
    GLfloat y;
} glf2;

typedef struct glf3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
} glf3;

typedef struct clr3 {
    GLfloat r;
    GLfloat g;
    GLfloat b;
} clr3;

typedef struct clr4 {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
} clr4;

#endif // GL_TYPES_H_INCLUDED
