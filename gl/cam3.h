#ifndef CAM3_H_INCLUDED
#define CAM3_H_INCLUDED

#include "types.h"
#include "f2_type.h"
#include "f3_type.h"

typedef struct Cam3 {
    glf3 pos;
    glf2 hdir; // Horizontal direction
    glf2 vdir; // Vertical direction
    GLfloat mat[16];   // Combined view and projection matrix
    GLuint uni_buf;    // Uniform buffer object ID
    GLuint mat_offset; // Offset of matrix in uniform block
} Cam3;

Cam3 cam3_init(glf2 window_size, GLfloat z_near, GLfloat z_far,
               const GLchar *uni_block_name, const GLuint *progs, unsigned prog_num);
inline int cam3_is_null(Cam3 *cam) { return !cam->uni_buf; }
void cam3_del(Cam3 *cam);

void set_callback_cam3(Cam3 *cam); // Sets pointer to camera to use in callbacks.
void cam3_on_window_resize(int width, int height);

void cam3_reset(Cam3 *cam);
void cam3_fov(Cam3 *cam, GLfloat fov);
void cam3_zoom(Cam3 *cam, double dt);
void cam3_move(Cam3 *cam, double dt, int hkey, int vkey, int fwd_key);
void cam3_key_rot(Cam3 *cam, int hrot, int vrot, double dt);
void cam3_rot(glf2 cursor_pos, glf2 window_center, glf2 window_size);

void cam3_upload(Cam3 *cam);

#endif // CAM3_H_INCLUDED
