#include <stdlib.h>

#include "funcs.h"
#include "buf.h"
#include "gen.h"
#include "attr_loc.h"
#include "dots.h"

void bind_dots_attr_loc(GLuint prog)
{
    BIND_ATTR_LOC(prog, vert_pos);
    BIND_ATTR_LOC(prog, inst_pos);
    BIND_ATTR_LOC(prog, inst_rad);
    BIND_ATTR_LOC(prog, inst_clr);
}

DebugDots dots_alloc(unsigned num)
{
    DebugDots dots;

    dots.pos = malloc(num * sizeof(*dots.pos));
    if (!dots.pos) {
        dots.rad = NULL;
        dots.clr = NULL;
        return dots;
    }
    dots.rad = malloc(num * sizeof(*dots.rad));
    if (!dots.rad) goto null_rad;
    dots.clr = malloc(num * sizeof(*dots.clr));
    if (!dots.clr) goto null_clr;

    dots.num = num;

    goto no_errors;
null_clr:
    free(dots.rad);
    dots.rad = NULL;
null_rad:
    free(dots.pos);
    dots.clr = NULL;
    dots.pos = NULL;
no_errors:
    return dots;
}

void dots_init_bufs(DebugDots *dots)
{
    glGenVertexArrays(1, &dots->vao);
    glBindVertexArray(dots->vao);

    glGenBuffers(1, &dots->model_buf);

    glBindBuffer(GL_ARRAY_BUFFER, dots->model_buf);
    dots->model_size = CIRCLE_BUF_SIZE;
    buf_orphan_f2(dots->model_size, GL_STATIC_DRAW);
    const glf2 circle_pos = {.x = 0.0, .y = 0.0};
    const GLfloat circle_rad = 1.0;
    buf_gen_circles(&circle_pos, &circle_rad, 1, GL_STATIC_DRAW);
    attr_src(loc_vert_pos, 2, 0, 0, 0);

    glGenBuffers(1, &dots->pos_buf);
    glBindBuffer(GL_ARRAY_BUFFER, dots->pos_buf);
    buf_orphan_f2(dots->num, GL_STREAM_DRAW);
    attr_src(loc_inst_pos, 2, 0, 0, 1);

    glGenBuffers(1, &dots->rad_buf);
    glBindBuffer(GL_ARRAY_BUFFER, dots->rad_buf);
    buf_orphan_f(dots->num, GL_STATIC_DRAW);
    buf_put_f(dots->rad, 0, dots->num);
    attr_src(loc_inst_rad, 1, 0, 0, 1);

    glGenBuffers(1, &dots->clr_buf);
    glBindBuffer(GL_ARRAY_BUFFER, dots->clr_buf);
    buf_orphan_f3(dots->num, GL_STATIC_DRAW);
    buf_put_clr3(dots->clr, 0, dots->num);
    attr_src(loc_inst_clr, 3, 0, 0, 1);

    glBindVertexArray(0);
}

void dots_upload(const DebugDots *dots)
{
    glBindBuffer(GL_ARRAY_BUFFER, dots->pos_buf);
    // Not orphaning is more efficient (at least for small numbers of dots).
    // buf_orphan_f2(dots->num, GL_STREAM_DRAW);
    buf_put_f2(dots->pos, 0, dots->num);
}

void dots_draw(const DebugDots *dots, GLuint dots_prog)
{
    glUseProgram(dots_prog);
    glBindVertexArray(dots->vao);
    // Change TRIANGLE_STRIP to TRIANGLE_FAN to draw a porcupine.
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, dots->model_size, dots->num);
}

void dots_del(DebugDots *dots)
{
    DEL_BUF(dots->model_buf);
    DEL_BUF(dots->pos_buf);
    DEL_BUF(dots->rad_buf);
    DEL_BUF(dots->clr_buf);

    free(dots->pos);
    dots->pos = NULL;

    free(dots->rad);
    dots->rad = NULL;

    free(dots->clr);
    dots->clr = NULL;
}
