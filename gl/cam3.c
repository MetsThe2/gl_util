#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "funcs.h"
#include "../pp_sak.h"   // ARRAY_LEN
#include "f2.h"
#include "../math_def.h" // M_INV_SQRT_2, cos, sin
#include "input.h"
#include "buf.h"
#include "bind_index.h"
#include "cam3.h"

// DEFAULT_FOV = 1 / tan(rad_from_deg(a)), where 0 < a < 180, and usually 30 < a < 60.
#define DEFAULT_FOV 1.5f

// Indices into camera matrix and what is located at them.
#define SCALE_X 0  // Scale the x components of camera space coordinates by this;
#define SCALE_Y 5  // same for y components to get clip space x and y.
                   // mat[SCALE_X] != mat[SCALE_Y] unless the aspect ratio is 1:1.
#define DEPTH_Z 10 // First additive component of clip space z (depth)
#define DEPTH_W 14 // Second component, put in the w column of the matrix.
#define PLANE_D 11 // Distance of camera (projection plane) from eye.
// mat[any other index] == 0

void cam3_fov(Cam3 *cam, GLfloat fov)
{
    cam->mat[SCALE_X] *= fov / cam->mat[SCALE_Y];
    // Now scale_x == fov / (normalized) aspect ratio
    cam->mat[SCALE_Y] = fov;
}

static void reset_zoom_dir_pos(Cam3 *cam)
{
    cam->pos = (glf3){0.f, 0.f, 0.f};
    cam->hdir = (glf2){0.f, -1.f}; // Straight forward
    cam->vdir = (glf2){1.f,  0.f}; // Halfway between straight up and straight down
    cam->mat[PLANE_D] = -1.f;      // Default zoom
}

void cam3_reset(Cam3 *cam)
{
    reset_zoom_dir_pos(cam);
    cam3_fov(cam, DEFAULT_FOV);
}

static Cam3 cam3_init_uni_buf(const GLchar *uni_block_name,
                              const GLuint *progs, unsigned prog_num)
{
    assert(prog_num > 0);

    Cam3 cam;

    GLuint block_index;
    unsigned i = prog_num - 1;
    /* Since prog_num is always at least one, there's no need to check if
    i >= 0 before the first iteration. Using a do-while instead of a for loop
    also gets rid of a compiler warning (with GCC 4.8.1 and all warnings enabled)
    about block_index possibly being used uninitialized later in this function. */
    do {
        block_index = glGetUniformBlockIndex(progs[i], uni_block_name);
        if (block_index == GL_INVALID_INDEX) {
            cam.uni_buf = 0;
            return cam;
        }
        glUniformBlockBinding(progs[i], block_index, cam3_bind_index);
    } while (--i < prog_num);

    glGenBuffers(1, &cam.uni_buf);
    glBindBufferBase(GL_UNIFORM_BUFFER, cam3_bind_index, cam.uni_buf);
    GLint block_size;
    /* block_index was last gotten from progs[0], so that program is used
    to get block data, but any program could be used with the corresponding
    block_index to get the same data. */
    glGetActiveUniformBlockiv(progs[0], block_index,
                              GL_UNIFORM_BLOCK_DATA_SIZE, &block_size);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_STREAM_DRAW);

    GLint uni_num;
    glGetActiveUniformBlockiv(progs[0], block_index,
                              GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uni_num);
    assert(uni_num == 1); // cam_mat

    GLint uni_indices[uni_num];
    glGetActiveUniformBlockiv(progs[0], block_index,
                              GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uni_indices);
    GLuint uint_indices[ARRAY_LEN(uni_indices)];
    for (unsigned i = 0; i < ARRAY_LEN(uint_indices); ++i) {
        uint_indices[i] = uni_indices[i]; // "Cast" to GLuint
    }

    GLint uni_offsets[ARRAY_LEN(uint_indices)];
    glGetActiveUniformsiv(progs[0], ARRAY_LEN(uint_indices), uint_indices,
                          GL_UNIFORM_OFFSET, uni_offsets);
    cam.mat_offset = uni_offsets[0];

    return cam;
}

Cam3 cam3_init(glf2 window_size, GLfloat z_near, GLfloat z_far,
               const GLchar *uni_block_name, const GLuint *progs, unsigned prog_num)
{
    Cam3 cam = cam3_init_uni_buf(uni_block_name, progs, prog_num);
    if (cam3_is_null(&cam)) return cam;

    memset(cam.mat, 0, sizeof(cam.mat));

    reset_zoom_dir_pos(&cam);

    const GLfloat reciprocal_aspect_ratio = window_size.y / window_size.x;
    cam.mat[SCALE_X] = DEFAULT_FOV * reciprocal_aspect_ratio;
    cam.mat[SCALE_Y] = DEFAULT_FOV;

    const GLfloat z_d = z_near - z_far;
    cam.mat[DEPTH_Z] = (z_far + z_near) / z_d;
    cam.mat[DEPTH_W] = 2.f * z_far * z_near / z_d;

    return cam;
}

static Cam3 *cb_cam = NULL;
void set_callback_cam3(Cam3 *cam)
{
    cb_cam = cam;
}

void cam3_on_window_resize(int width, int height)
{
    // Replace with assert(cb_cam && "cam3_on_window_resize has been...")?
    static int warned = 0;
    if (!(cb_cam || warned)) {
        fputs("Warning: the callback cam3_on_window_resize has been called without "
              "having a callback cam set. This can result in an incorrect aspect "
              "ratio.\n", stderr);
        warned = 1; // Don't print the warning every frame the window is resized.
        return;
    } // Reciprocal of aspect ratio
    const GLfloat rec_aspect_ratio = ((GLfloat)height) / (GLfloat)width;
    cb_cam->mat[SCALE_X] = cb_cam->mat[SCALE_Y] * rec_aspect_ratio;
}

#define ZOOM_STEP 10.f
void cam3_zoom(Cam3 *cam, double dt)
{
    const int scroll_dir = get_scroll_dir();
    if (!scroll_dir) return;

    const GLfloat d = dt * ZOOM_STEP;
    if (scroll_dir > 0) {
        if (cam->mat[PLANE_D] >= -d) return;
        cam->mat[PLANE_D] += d;
    } else {
        cam->mat[PLANE_D] -= d;
    }
}

#define HVEL    6.f
#define VVEL    4.f
#define FWD_VEL 8.f
void cam3_move(Cam3 *cam, double dt, int hkey, int vkey, int fwd_key)
{
    cam->pos.y += VVEL * dt * (GLfloat)vkey;

    GLfloat fwd_offset, hoffset;
    if (fwd_key) {
        if (hkey) {
            fwd_offset = M_INV_SQRT_2 * FWD_VEL * dt * (GLfloat)fwd_key;
            hoffset    = M_INV_SQRT_2 * HVEL    * dt * (GLfloat)hkey;
            cam->pos.x -= cam->hdir.x * fwd_offset;
            cam->pos.z -= cam->hdir.y * fwd_offset;
            cam->pos.x += cam->hdir.y * hoffset;
            cam->pos.z -= cam->hdir.x * hoffset;
        } else {
            fwd_offset = FWD_VEL * dt * (GLfloat)fwd_key;
            cam->pos.x -= cam->hdir.x * fwd_offset;
            cam->pos.z -= cam->hdir.y * fwd_offset;
        }
    } else if (hkey) {
        hoffset = HVEL * dt * (GLfloat)hkey;
        cam->pos.x += cam->hdir.y * hoffset;
        cam->pos.z -= cam->hdir.x * hoffset;
    }
}

#define ROT_VEL 1.f
void cam3_rot(glf2 cursor_pos, glf2 window_center, glf2 window_size)
{
    assert(cb_cam && "cam3_rot was called before set_callback_cam3.");

    const glf2 d = sub_glf2(cursor_pos, window_center);
    const glf2 a = scale_glf2(div_glf2(d, window_size), DEFAULT_FOV * ROT_VEL);

    cb_cam->hdir = rot_glf2(cb_cam->hdir, cos(a.x), sin(a.x));
    cb_cam->vdir = rot_glf2(cb_cam->vdir, cos(a.y), sin(a.y));
}

void cam3_key_rot(Cam3 *cam, int hrot, int vrot, double dt)
{
    if (!(hrot || vrot)) return; // Profile with / without the branch.

    const GLfloat ang = (2.f * ROT_VEL) * dt;
    const GLfloat c = cos(ang), s = sin(ang);

    cam->hdir = rot_glf2(cam->hdir, c, s * (GLfloat)hrot);
    cam->vdir = rot_glf2(cam->vdir, c, s * (GLfloat)vrot);
}

static void mat_mul(GLfloat *r, const GLfloat *a, const GLfloat *b, unsigned n)
{
#define column(i) (i * n)
#define row(j) j
    for (unsigned i = 0; i < n; ++i) {
        for (unsigned j = 0; j < n; ++j) {
            const unsigned i_j = column(i) + row(j);
#define mul(k) a[column(k) + row(j)] * b[column(i) + row(k)]
            r[i_j] = mul(0);
            for (unsigned k = 1; k < n; ++k) r[i_j] += mul(k);
        }
    }
#undef mul
#undef column
#undef row
}

static void rot_mat4(GLfloat *r, GLfloat x, GLfloat y, GLfloat z, GLfloat c, GLfloat s)
{
    const GLfloat qx = x * x;
    r[0] = qx + c - (qx * c);
    const GLfloat ic = 1.f - c;
    const GLfloat icxy = ic * x * y;
    const GLfloat zs = z * s;
    r[1] = icxy + zs;
    const GLfloat icxz = ic * x * z;
    const GLfloat ys = y * s;
    r[2] = icxz - ys;
    r[3] = 0.f;

    r[4] = icxy - zs;
    const GLfloat qy = y * y;
    r[5] = qy + c - (qy * c);
    const GLfloat icyz = ic * y * z;
    const GLfloat xs = x * s;
    r[6] = icyz + xs;
    r[7] = 0.f;

    r[8] = icxz + ys;
    r[9] = icyz - xs;
    const GLfloat qz = z * z;
    r[10] = qz + c - (qz * c);
    r[11] = 0.f;

    r[12] = r[13] = r[14] = 0.f;
    r[15] = 1.f;
}

static void id_mat(GLfloat *r, unsigned n)
{
    memset(r, 0, n * n * sizeof(*r));
    for (unsigned i = 0; i < n; ++i) r[i * (n + 1)] = 1.f;
}

static void trans_mat4(GLfloat *r, glf3 offset)
{
    id_mat(r, 4);
    r[12] = -offset.x;
    r[13] = -offset.y;
    r[14] = -offset.z;
}

void cam3_upload(Cam3 *cam)
{
    GLfloat vrot[16];
    rot_mat4(vrot, -cam->hdir.y, 0.f, cam->hdir.x, cam->vdir.x, cam->vdir.y);

    GLfloat hrot[16];
    rot_mat4(hrot, 0.f, 1.f, 0.f, cam->hdir.y, -cam->hdir.x);

    GLfloat trans[16];
    trans_mat4(trans, cam->pos);

    GLfloat trans_vrot[16];
    mat_mul(trans_vrot, vrot, trans, 4);
    GLfloat trans_rot[16];
    mat_mul(trans_rot, hrot, trans_vrot, 4);
    GLfloat up[16];
    mat_mul(up, cam->mat, trans_rot, 4);

    glBindBuffer(GL_UNIFORM_BUFFER, cam->uni_buf);
    glBufferSubData(GL_UNIFORM_BUFFER, cam->mat_offset, sizeof(up), up);
}

void cam3_del(Cam3 *cam)
{
    DEL_BUF(cam->uni_buf);
}
