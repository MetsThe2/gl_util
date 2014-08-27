#ifndef CAM2_TYPE_H_INCLUDED
#define CAM2_TYPE_H_INCLUDED

#include "types.h"

typedef struct Cam2 {
    glf2 pos;
    GLfloat rad;
    GLuint uni_buf;
    GLuint pos_offset; // Offsets of cam_pos and cam_scale in uniform buffer
    GLuint scale_offset;
} Cam2;

#endif // CAM2_TYPE_H_INCLUDED
