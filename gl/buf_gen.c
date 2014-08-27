#include "funcs.h"
#include "f2.h"      // rot_glf2
#include "../math_def.h" // sin, cos, M_PI
#include "buf_gen.h"

#if 1 // So the macros can be folded in a code editor (e.g. Code::Blocks).
#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

#define IF_0(...)
#define IF_1(...) __VA_ARGS__
#define IF(c, ...) CONCAT(IF_, c)(__VA_ARGS__)

#define GEN_CIRCLE(n) \
    /* Hopefully these will be evaluated at compile time. */ \
    const double side_angle = 2.0f * M_PI / (double)CIRCLE_VERT_NUM; \
    const GLfloat ccw_cos = cos(side_angle), ccw_sin = sin(side_angle); \
 \
    GLfloat dup_first = pos.x + rad; \
    vert[0].x = dup_first; \
    vert[0].y = pos.y; \
    vert[1].x = dup_first; \
    vert[1].y = pos.y; \
 \
    glf2 ccw_dir = {.x = 1.0, .y = 0.0}; \
    IF(n, norm[0] = ccw_dir; norm[1].x = ccw_dir.x; norm[1].y = -ccw_dir.y;) \
 \
    unsigned i = 2; \
    do { \
        ccw_dir = rot_glf2(ccw_dir, ccw_cos, ccw_sin); \
        \
        GLfloat vert_x = pos.x + ccw_dir.x * rad; \
        GLfloat rad_y = ccw_dir.y * rad; \
        \
        vert[i].x = vert_x; \
        vert[i].y = pos.y + rad_y; \
        IF(n, norm[i] = ccw_dir;) \
        ++i; /* The following IF's condition should be CIRCLE_BUF_SIZE % 2,
        but since that doesn't compile, replace it with 1 or 0 manually. */ \
        IF(0, if (i >= CIRCLE_BUF_SIZE) break;) \
 \
        vert[i].x = vert_x; \
        vert[i].y = pos.y - rad_y; \
        IF(n, norm[i].x = ccw_dir.x; norm[i].y = -ccw_dir.y;) \
        /* The IF's condition should be !(CIRCLE_BUF_SIZE % 2) here. */ \
    } while (++i IF(1, < CIRCLE_BUF_SIZE));
#endif // 1

static void gen_circle(glf2 pos, GLfloat rad, glf2 *vert)
{
    GEN_CIRCLE(0);
}
static void gen_circle_with_norm(glf2 pos, GLfloat rad, glf2 *vert, glf2 *norm)
{
    GEN_CIRCLE(1);
}

#undef GEN_CIRCLE

void buf_gen_circles(const glf2 *pos, const GLfloat *rad, unsigned num, GLenum usage)
{
    const size_t circle_byte_num = CIRCLE_BUF_SIZE * sizeof(glf2);
    const size_t  total_byte_num = num * circle_byte_num;

    /* Mapping is slow as hell for some reason (with a Geforce GTX 560 and
       OpenGL 4.4, the latest (non-beta) version on 2014-08-18).
    glf2 *vert = glMapBufferRange(GL_ARRAY_BUFFER, 0, total_byte_num, MAP_FAST_WRITE);
    if (!vert) return 1; // Return type was int */
    glf2 vert[total_byte_num];
    glf2 *gen_here = vert;

    for (unsigned i = 0; i < num; gen_here += circle_byte_num, ++i) {
        gen_circle(pos[i], rad[i], gen_here);
    }
    glBufferData(GL_ARRAY_BUFFER, total_byte_num, vert, usage);
}

void buf_gen_circles_with_norm(const glf2 *pos, const GLfloat *rad, unsigned num,
                               GLuint vert_buf, GLuint norm_buf, GLenum usage)
{
    const size_t circle_byte_num = CIRCLE_BUF_SIZE * sizeof(glf2);
    const size_t  total_byte_num = num * circle_byte_num;

    glf2 vert[total_byte_num];
    glf2 norm[total_byte_num];

    for (unsigned gen_offset = 0, i = 0; i < num; gen_offset += circle_byte_num, ++i) {
        gen_circle_with_norm(pos[i], rad[i], vert + gen_offset, norm + gen_offset);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vert_buf);
    glBufferData(GL_ARRAY_BUFFER, total_byte_num, vert, usage);
    glBindBuffer(GL_ARRAY_BUFFER, norm_buf);
    glBufferData(GL_ARRAY_BUFFER, total_byte_num, norm, usage);
}
