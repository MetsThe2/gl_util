#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "funcs.h"
#include "../pp_sak.h" // ARRAY_LEN
#include "input.h"
#include "buf.h"
#include "bind_index.h"
#include "cam3.h"

// DEFAULT_FOV = 1 / tan(rad_from_deg(a)), where 0 < a < 180, and usually 30 < a < 60.
#define DEFAULT_FOV 1.8f

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
    cam->dir = (glf3){1.f, 0.f, 0.f};
    cam->mat[PLANE_D] = -1.f;
}

void cam3_reset(Cam3 *cam)
{
    reset_zoom_dir_pos(cam);
    cam3_fov(cam, DEFAULT_FOV);
}

static Cam3 cam3_init_uni_buf(const GLchar *uni_block_name,
                              GLuint *progs, unsigned prog_num)
{
    assert(prog_num > 0);

    Cam3 cam;

    GLuint block_index;
    int i = prog_num - 1;
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
    } while (--i >= 0);

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
    assert(uni_num == 2); // cam_mat and cam_pos

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
    cam.pos_offset = uni_offsets[1];

    return cam;
}

Cam3 cam3_init(glf2 window_size, GLfloat z_near, GLfloat z_far,
               const GLchar *uni_block_name, GLuint *progs, unsigned prog_num)
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

static Cam3 *callback_cam = NULL;
void set_callback_cam3(Cam3 *cam)
{
    callback_cam = cam;
}

void cam3_on_window_resize(int width, int height)
{
    static int warned = 0;
    if (!(callback_cam || warned)) {
        fputs("Warning: the callback cam3_on_window_resize has been called without "
              "having a callback cam set. This can result in an incorrect aspect "
              "ratio.\n", stderr);
        warned = 1; // Don't print the warning every frame the window is resized.
        return;
    } // Reciprocal of aspect ratio
    const GLfloat rec_aspect_ratio = ((GLfloat)height) / (GLfloat)width;
    callback_cam->mat[SCALE_X] = callback_cam->mat[SCALE_Y] * rec_aspect_ratio;
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

static void mat_mul(GLfloat *a, GLfloat *b, GLfloat *r, unsigned n)
{
#define column(i) (i * n)
#define row(j) j
    assert(n > 0);
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

static void rot_mat(GLfloat *m, GLfloat x, GLfloat y, GLfloat z, GLfloat c, GLfloat s)
{
    const GLfloat qx = x * x;
    m[0] = qx + c - (qx * c);
    const GLfloat ic = 1.f - c;
    const GLfloat icxy = ic * x * y;
    const GLfloat zs = z * s;
    m[1] = icxy + zs;
    const GLfloat icxz = ic * x * z;
    const GLfloat ys = y * s;
    m[2] = icxz - ys;
    m[3] = 0.f;

    m[4] = icxy - zs;
    const GLfloat qy = y * y;
    m[5] = qy + c - (qy * c);
    const GLfloat icyz = ic * y * z;
    const GLfloat xs = x * s;
    m[6] = icyz + xs;
    m[7] = 0.f;

    m[8] = icxz + ys;
    m[9] = icyz - xs;
    const GLfloat qz = z * z;
    m[10] = qz + c - (qz * c);
    m[11] = 0.f;

    m[12] = m[13] = m[14] = 0.f;
    m[15] = 1.f;
}

static void id_mat(GLfloat *r, unsigned n)
{
    memset(r, 0, n * n);
    for (unsigned i = 0; i < n; ++i) r[i * n + i] = 1.f;
}

void cam3_upload(Cam3 *cam)
{
    static GLfloat tmpa[16], tmpb[16], tmpc[16];
    rot_mat(tmpa, 0.f,        1.f, 0.f, cam->dir.x, cam->dir.z); // Horizontal
    rot_mat(tmpb, cam->dir.x, 0.f, 0.f, 1.f,        cam->dir.y); // Vertical
    mat_mul(tmpa, tmpb, tmpc, 4);     // All rotations
    mat_mul(cam->mat, tmpc, tmpa, 4); // Combined projection and rotation matrix
    for (unsigned i = 0; i < 0 * ARRAY_LEN(tmpa); ++i) {
        printf("%u %f %f\n", i, cam->mat[i], tmpa[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, cam->uni_buf);
    glBufferSubData(GL_UNIFORM_BUFFER, cam->pos_offset, sizeof(cam->pos), &cam->pos);
    glBufferSubData(GL_UNIFORM_BUFFER, cam->mat_offset, sizeof(tmpa), tmpa);
}

void cam3_del(Cam3 *cam)
{
    DEL_BUF(cam->uni_buf);
}
