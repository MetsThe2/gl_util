#ifndef F3_H_INCLUDED
#define F3_H_INCLUDED

#include <math.h>  // sqrt
#include "f3_type.h"

inline int eq_glf3( glf3 a, glf3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
inline int neq_glf3(glf3 a, glf3 b) { return a.x != b.x || a.y != b.y || a.z != b.z; }

inline glf3 add_glf3(glf3 a, glf3 b) {
    glf3 r = {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
    return r;
}
inline glf3 sub_glf3(glf3 a, glf3 b) {
    glf3 r = {.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
    return r;
}
inline glf3 mul_glf3(glf3 a, glf3 b) {
    glf3 r = {.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z};
    return r;
}

inline glf3 scale_glf3(glf3 v, GLfloat s) {
    v.x *= s;
    v.y *= s;
    v.z *= s;
    return v;
}

inline GLfloat dot_glf3(glf3 a, glf3 b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline GLfloat sqr_abs_glf3(glf3 v) {
    return dot_glf3(v, v);
}
inline GLfloat sqr_dist_glf3(glf3 a, glf3 b) {
    return sqr_abs_glf3(sub_glf3(b, a));
}

inline glf3 unit_glf3(glf3 v) {
    return scale_glf3(v, 1.f / sqrt(sqr_abs_glf3(v)));
}

#endif // F3_H_INCLUDED
