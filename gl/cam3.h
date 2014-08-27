#ifndef CAM3_H_INCLUDED
#define CAM3_H_INCLUDED

#include "types.h"
#include "f2_type.h"
#include "f3_type.h"

typedef struct Cam3 {
    glf3 pos;
    glf3 dir;
    GLfloat mat[16]; // 4x4 camera- to clip-space matrix
    GLuint uni_buf;
    GLuint mat_offset;
    GLuint pos_offset;
} Cam3;

Cam3 cam3_init(glf2 window_size, GLfloat z_near, GLfloat z_far,
               const GLchar *uni_block_name, GLuint *progs, unsigned prog_num);
inline int cam3_is_null(Cam3 *cam) { return !cam->uni_buf; }
void cam3_del(Cam3 *cam);

void set_callback_cam3(Cam3 *cam); // Sets pointer to camera to use in callbacks.
void cam3_on_window_resize(int width, int height);

void cam3_reset(Cam3 *cam);
void cam3_fov(Cam3 *cam, GLfloat fov);
void cam3_zoom(Cam3 *cam, double dt);

void cam3_upload(Cam3 *cam);

#endif // CAM3_H_INCLUDED
