#include "funcs.h"
#include "f2.h"          // rot_glf2
#include "../pp_sak.h"   // PP_FOUR
#include "../math_def.h" // sin, cos, M_PI
#include "gen.h"

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
        /* The IF's condition should be !(CIRCLE_BUF_SIZE % 2). */ \
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

#define I4(n) (n), ((n) + 1), ((n) + 2), (n), ((n) + 2), ((n) + 3)
#define F4(k) I4(k * 4)
static void gen_cuboid_index(BufIndex *bi)
{
    static const BufIndex i[CUBOID_INDEX_NUM] = {
        F4(0), F4(1), F4(2), F4(3), F4(4), F4(5)
    };
    memcpy(bi, i, sizeof(i));
}

#define I3(n) (n), ((n) + 1), ((n) + 2)
#define F3(k) I3(k * 3)
static void gen_pyramid_index(BufIndex *bi)
{
    static const BufIndex i[PYRAMID_INDEX_NUM] = {
        F3(0), F3(1), F3(2), F3(3), I4(12)
    };
    memcpy(bi, i, sizeof(i));
}
#undef F4
#undef I4

#define UNIT3(a, b, c) \
    vert[v++] = (glf3){pos.x a size.x, pos.y b size.y, pos.z c size.z}
// pos.c o size.c = pos.c + ( o 1.f * size.c), where o is operator '-' or '+' and
// c is some component.

static void gen_cuboid_vert(glf3 *vert, glf3 pos, glf3 size)
{
    unsigned v = 0;

    UNIT3(-, -, -); // Left
    UNIT3(-, +, -);
    UNIT3(-, +, +);
    UNIT3(-, -, +);

    UNIT3(+, -, -); // Front
    UNIT3(+, +, -);
    UNIT3(-, +, -);
    UNIT3(-, -, -);

    UNIT3(+, -, +); // Right
    UNIT3(+, +, +);
    UNIT3(+, +, -);
    UNIT3(+, -, -);

    UNIT3(-, -, +); // Back
    UNIT3(-, +, +);
    UNIT3(+, +, +);
    UNIT3(+, -, +);

    UNIT3(+, -, +); // Bottom
    UNIT3(+, -, -);
    UNIT3(-, -, -);
    UNIT3(-, -, +);

    UNIT3(+, +, -); // Top
    UNIT3(+, +, +);
    UNIT3(-, +, +);
    UNIT3(-, +, -);
}

#define VERT3(x, y, z) vert[v++] = (glf3){x, y, z}
static void gen_pyramid_vert(glf3 *vert, glf3 pos, glf3 size)
{
#define TRI(a1, b1, c1, a2, b2, c2) \
    UNIT3(a1, b1, c1); VERT3(0, pos.y + size.y, 0); UNIT3(a2, b2, c2)

#define TOP(a1, b1, c1, a2, b2, c2, a3, b3, c3, a4, b4, c4) \
    TRI(a1, b1, c1, a2, b2, c2); /* Left */ \
    TRI(a3, b3, c3, a1, b1, c1); /* Front */ \
    TRI(a4, b4, c4, a3, b3, c3); /* Right */ \
    TRI(a2, b2, c2, a4, b4, c4); /* Back */

    unsigned v = 0;

    TOP(-, -, -,   -, -, +,   +, -, -,   +, -, +);

#undef TOP
#undef TRI

    UNIT3(+, -, +);
    UNIT3(-, -, +);
    UNIT3(-, -, -);
    UNIT3(+, -, -);
}
#undef VERT3
#undef UNIT3

static void gen_cuboid_norm(glf3 *norm)
{
#define T (glf3)
#define px T { 1.f,  0.f,  0.f}
#define nx T {-1.f,  0.f,  0.f}
#define py T { 0.f,  1.f,  0.f}
#define ny T { 0.f, -1.f,  0.f}
#define pz T { 0.f,  0.f,  1.f}
#define nz T { 0.f,  0.f, -1.f}
    const glf3 n[CUBOID_VERT_NUM] = {
        PP_FOUR(nx), PP_FOUR(nz), PP_FOUR(px), PP_FOUR(pz), PP_FOUR(ny), PP_FOUR(py)
    };
#undef T
#undef px
#undef nx
#undef py
#undef ny
#undef pz
#undef nz
    memcpy(norm, n, sizeof(n));
}

static void gen_pyramid_norm(glf3 *norm)
{

}

static void gen_cuboid_clr(clr3 *clr, const clr3 *face_clrs)
{
    unsigned fc = 0, c = 0;
    for (unsigned i = 0; i < CUBOID_FACE_NUM; ++i) {
        for (unsigned j = 0; j < CUBOID_VERT_NUM / CUBOID_FACE_NUM; ++j) {
            clr[c++] = face_clrs[fc];
        }
        ++fc;
    }
}


static void gen_pyramid_clr(clr3 *clr, const clr3 *face_clrs)
{

}

void gen_cuboid(BufIndex *bi, glf3 *vert, glf3 *norm, clr3 *clr,
                const clr3 *face_clrs, glf3 pos, glf3 size)
{
    if (bi)               gen_cuboid_index(bi);
    if (norm)             gen_cuboid_norm(norm);
    if (vert)             gen_cuboid_vert(vert, pos, size);
    if (face_clrs && clr) gen_cuboid_clr(clr, face_clrs);
}

void gen_pyramid(BufIndex *bi, glf3 *vert, glf3 *norm, clr3 *clr,
                 const clr3 *face_clrs, glf3 pos, glf3 size)
{
    if (bi)               gen_pyramid_index(bi);
    if (vert)             gen_pyramid_vert(vert, pos, size);
    if (norm)             gen_pyramid_norm(norm);
    if (face_clrs && clr) gen_pyramid_clr(clr, face_clrs);
}
