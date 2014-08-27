#ifndef DOTS_H_INCLUDED
#define DOTS_H_INCLUDED

#include "types.h"
#include "f2_type.h"
#include "clr3_type.h"

typedef struct DebugDots {
    glf2 *pos;
    GLfloat *rad;
    clr3 *clr;
    GLuint vao;
    GLuint model_buf;
    GLuint pos_buf;
    GLuint rad_buf;
    GLuint clr_buf;
    unsigned model_size;
    unsigned num;
} DebugDots;

// Call this before linking dots_prog (made with dots.vert and vary.frag).
void bind_dots_attr_loc(GLuint prog);

DebugDots dots_alloc(unsigned num); // Allocate pos, rad and clr arrays to initialize.
inline int dots_are_null(const DebugDots *dots) { return !dots->pos; }
/* Creates buffers and uploads static data (unit circle model, colors, radiuses),
which should be initialized before calling this. */
void dots_init_bufs(DebugDots *dots);
void dots_del(DebugDots *dots);

void dots_upload(const DebugDots *dots);
void dots_draw(const DebugDots *dots, GLuint dots_prog);

#endif // DOTS_H_INCLUDED
