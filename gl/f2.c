#include <stdio.h>

#include "../pp_sak.h"   // ARRAY_LEN
#include "../math_def.h" // M_PI
#include "f2.h"

glf2 get_side_rot(unsigned side_num) {
#define GEN(n) \
    {.x = cos(2.0f * M_PI / (GLfloat)(n)), .y = sin(2.0f * M_PI / (GLfloat)(n))}
    _Static_assert(MIN_SIDE_ROT_NUM == 3 && MAX_SIDE_ROT_NUM == 8,
                   "get_side_rot lookup table needs to be updated.");
    const glf2 rots[] = {GEN(MIN_SIDE_ROT_NUM), GEN(4), GEN(5), GEN(6), GEN(7),
                         GEN(MAX_SIDE_ROT_NUM)};
#undef GEN
    if (side_num > ARRAY_LEN(rots) + MIN_SIDE_ROT_NUM) {
        fprintf(stderr, "Can't get rotation vector for %u sides.\n", side_num);
        return (glf2){0.f, 0.f};
    }
    return rots[side_num - MIN_SIDE_ROT_NUM];
/* // TODO: Test whether the above or below method is faster.
#define GEN(n) \
case (n): \
    side_angle = 2.0f * M_PI / (GLfloat)(n); \
    rot.x = cos(side_angle); \
    rot.y = sin(side_angle); \
    break

    GLfloat side_angle;
    glf2 rot;
    switch (side_num) {
        GEN(MIN_SIDE_ROT_NUM);
        GEN(4);
        GEN(5);
        GEN(6);
        GEN(7);
        GEN(MAX_SIDE_ROT_NUM);
    default:
        fprintf(stderr, "Can't get rotation vector for %u sides.\n", side_num);
        rot.x = rot.y = 0.0;
    }
#undef GEN
    return rot;
*/
}
