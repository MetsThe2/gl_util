#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED

#include "types.h"
#include "attr_loc.h"

#define GET_UNI_LOC(p, u) GET_UNI_LOC2(p, loc_ ## u, #u)
#define GET_UNI_LOC2(p, n, s) const GLint n = glGetUniformLocation(p, s)

GLuint get_shader(GLenum type, char *filename);
#define GET_SHADER_v(s) get_shader(GL_VERTEX_SHADER,   s ".vert")
#define GET_SHADER_f(s) get_shader(GL_FRAGMENT_SHADER, s ".frag")
#define GET_SHADER_g(s) get_shader(GL_GEOMETRY_SHADER, s ".geom")
#define GET_SHADER_c(s) get_shader(GL_COMPUTE_SHADER,  s ".comp")
#define GET_SHADER2(f, s) f(s)
#define GET_SHADER(n, t) GET_SHADER2(GET_SHADER_ ## t, #n)
/* If s isn't one of v (vertex), f (fragment), g (geometry) or c (compute),
concatenating GET_SHADER and t will yield a macro that doesn't exist and
result in a compiler error. */

GLuint make_prog(GLuint vert, GLuint frag, void (*before_linking)(GLuint prog));
GLuint get_prog(char *vert_filename, char *frag_filename,
                void (*before_linking)(GLuint prog));
#define GET_PROG(v, f, b) get_prog(v ".vert", f ".frag", b)

void del_progs(GLuint *progs, unsigned prog_num);

#endif // SHADER_H_INCLUDED
