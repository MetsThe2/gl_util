#ifndef BUF_GEN_H_INCLUDED
#define BUF_GEN_H_INCLUDED

#include "types.h"
#include "f2_type.h"

#define CIRCLE_VERT_NUM 16
#define CIRCLE_BUF_SIZE (CIRCLE_VERT_NUM + 2) // This is the
// number of vertices stored in a buffer per circle. The first and last vertex is
// duplicated to create degenerate triangles (and avoid linking all triangle strips
// together), so CIRCLE_BUF_SIZE > CIRCLE_VERT_NUM.
void buf_gen_circles(const glf2 *pos, const GLfloat *rad, unsigned num, GLenum usage);
// buf_gen_circles_with_norm generates into two different GL_ARRAY_BUFFERs, so it
// has to bind the buffers itself.
void buf_gen_circles_with_norm(const glf2 *pos, const GLfloat *rad, unsigned num,
                               GLuint vert_buf, GLuint norm_buf, GLenum usage);

#endif // BUF_GEN_H_INCLUDED
