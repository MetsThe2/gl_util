#ifndef F2_H_INCLUDED
#define F2_H_INCLUDED

#include <math.h> // sqrt

#include "f2_type.h"

inline int eq_glf2(glf2 a, glf2 b) {
    return a.x == b.x && a.y == b.y;
}
inline int neq_glf2(glf2 a, glf2 b) {
    return a.x != b.x || a.y != b.y;
}

inline glf2 add_glf2(glf2 a, glf2 b) {
    glf2 r = {.x = a.x + b.x, .y = a.y + b.y};
    return r;
}
inline glf2 sub_glf2(glf2 a, glf2 b) {
    glf2 r = {.x = a.x - b.x, .y = a.y - b.y};
    return r;
}
inline glf2 mul_glf2(glf2 a, glf2 b) {
    glf2 r = {.x = a.x * b.x, .y = a.y * b.y};
    return r;
}
inline glf2 div_glf2(glf2 a, glf2 b) { return (glf2){a.x / b.x, a.y / b.y}; }

inline glf2 scale_glf2(glf2 v, GLfloat s) {
    v.x *= s;
    v.y *= s;
    return v;
}

inline GLfloat dot_glf2(glf2 a, glf2 b) {
    return (a.x * b.x) + (a.y * b.y);
}
inline  GLfloat sqr_len_glf2(glf2 v) {
    return dot_glf2(v, v);
}
inline GLfloat sqr_dist_glf2(glf2 a, glf2 b) {
    return sqr_len_glf2(sub_glf2(a, b));
}

inline glf2 unit_glf2(glf2 v) {
    return scale_glf2(v, 1.f / sqrt(sqr_len_glf2(v)));
}

// a and b are the endpoints of the line.
inline glf2 line_nearest(glf2 a, glf2 b, glf2 v) {
    const glf2 ab = sub_glf2(b, a); // ab = a -> b
    const glf2 av = sub_glf2(v, a); // av = a -> v
    const GLfloat s = dot_glf2(av, ab) / sqr_len_glf2(ab); // s = (av . ab) / |ab|**2
    if (s <= 0.f) return a;
    if (s >= 1.f) return b;
    return add_glf2(a, scale_glf2(ab, s)); // a + (ab * s)
}

inline glf2 rot_glf2(glf2 v, GLfloat rot_cos, GLfloat rot_sin) {
    glf2 r;
    r.x = (v.x * rot_cos) - (v.y * rot_sin);
    r.y = (v.x * rot_sin) + (v.y * rot_cos);
    return r;
}

#define MAX_SIDE_ROT_NUM 8 // The (inclusive) range of valid inputs to get_side_rot
#define MIN_SIDE_ROT_NUM 3 // 8 is the maximum because > 8 sides ~= circle.
glf2 get_side_rot(unsigned side_num);

#endif // F2_H_INCLUDED
