#include <assert.h>

#include "funcs.h"
#include "../pp_sak.h"
#include "input.h"
#include "f2.h"
#include "buf.h"
#include "bind_index.h"
#include "cam2.h"

Cam2 cam2_init(GLfloat pos_x, GLfloat pos_y, GLfloat rad,
               const GLchar *uni_block_name, GLuint *progs, unsigned prog_num)
{
    Cam2 cam = {.pos = {.x = pos_x, .y = pos_y}, .rad = rad};

    assert(prog_num > 0);
    GLuint block_index;
    int i = prog_num - 1;
    /* Since prog_num is always at least one, there's no need to check if
    i >= 0 before the first iteration. Using a do-while instead of a for loop
    also gets rid of a compiler warning (with GCC 4.8.1 and all warnings enabled)
    about block_index possibly being used unitialized later in this function. */
    do {
        block_index = glGetUniformBlockIndex(progs[i], uni_block_name);
        if (block_index == GL_INVALID_INDEX) {
            cam.uni_buf = 0;
            return cam;
        }
        glUniformBlockBinding(progs[i], block_index, cam2_bind_index);
    } while (--i >= 0);

    glGenBuffers(1, &cam.uni_buf);
    glBindBufferBase(GL_UNIFORM_BUFFER, cam2_bind_index, cam.uni_buf);
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
    assert(uni_num == 2); // cam_pos and cam_scale

    GLint uni_indices[2];
    glGetActiveUniformBlockiv(progs[0], block_index,
                              GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uni_indices);
    GLuint uint_indices[ARRAY_LEN(uni_indices)];
    for (unsigned i = 0; i < ARRAY_LEN(uint_indices); ++i) {
        uint_indices[i] = uni_indices[i]; // "Cast" to GLuint
    }

    GLint uni_offsets[ARRAY_LEN(uint_indices)];
    glGetActiveUniformsiv(progs[0], ARRAY_LEN(uint_indices), uint_indices,
                          GL_UNIFORM_OFFSET, uni_offsets);
    cam.pos_offset   = uni_offsets[0];
    cam.scale_offset = uni_offsets[1];

    return cam;
}

void cam2_upload(Cam2 cam)
{
    glBindBuffer(GL_UNIFORM_BUFFER, cam.uni_buf);
    glBufferSubData(GL_UNIFORM_BUFFER, cam.pos_offset,   sizeof(cam.pos),   &cam.pos);
    const GLfloat cam_scale = 1.f / cam.rad;
    glBufferSubData(GL_UNIFORM_BUFFER, cam.scale_offset, sizeof(cam_scale), &cam_scale);
}

void cam2_del(Cam2 *cam)
{
    DEL_BUF(cam->uni_buf);
}

// Maximum distance of edge of visible area from the outside of the level border
#define OUTSIDE_LEVEL 2.0

Cam2 cam2_pin(Cam2 cam, glf2 pin, GLfloat tightness, glf2 level_size)
{
    const GLfloat diam = 2.0 * cam.rad;

    assert(tightness >= 0.0 && tightness <= 1.0);
    const GLfloat min_dist = cam.rad * tightness;
    const GLfloat max_dist = diam - min_dist;

    // I have no idea what to name this variable.
    const glf2 o = {.x = level_size.x + OUTSIDE_LEVEL,
                    .y = level_size.y + OUTSIDE_LEVEL};

    if (pin.x < cam.pos.x + min_dist) {
        GLfloat new_pos_x = pin.x - min_dist;
        if (new_pos_x > -OUTSIDE_LEVEL) cam.pos.x = new_pos_x;
    } else if (pin.x > cam.pos.x + max_dist) {
        GLfloat new_pos_x = pin.x - max_dist;
        if (new_pos_x + diam < o.x) cam.pos.x = new_pos_x;
    }
    if (pin.y < cam.pos.y + min_dist) {
        GLfloat new_pos_y = pin.y - min_dist;
        if (new_pos_y > -OUTSIDE_LEVEL) cam.pos.y = new_pos_y;
    } else if (pin.y > cam.pos.y + max_dist) {
        GLfloat new_pos_y = pin.y - max_dist;
        if (new_pos_y + diam < o.y) cam.pos.y = new_pos_y;
    }

    return cam;
}

#define ZOOM_STEP 25.0

Cam2 cam2_zoom(Cam2 cam, glf2 pin, glf2 level_size)
{
    const int scroll_dir = get_scroll_dir();
    if (!scroll_dir || (scroll_dir < 0 && cam.rad <= ZOOM_STEP)) return cam;

    const GLfloat zoom = ZOOM_STEP * (GLfloat)scroll_dir;
    const GLfloat diam = 2.f * (cam.rad -= zoom);

    // Make zooming smoother by moving camera after zooming so that the
    // pinned position remains closer to the center.
    // TODO: Prevent the area > cam.pos + diam from becoming visible
    // when zooming out unless it's unavoidable (when cam.rad > level_size).

    const glf2 d = scale_glf2(sub_glf2(pin, cam.pos), zoom / cam.rad);

    const glf2 max = {.x = level_size.x + OUTSIDE_LEVEL - diam,
                      .y = level_size.y + OUTSIDE_LEVEL - diam};

    if ((cam.pos.x > -OUTSIDE_LEVEL && d.x < 0.f) ||
        (cam.pos.x < max.x && d.x > 0.f)) cam.pos.x += d.x;
    if ((cam.pos.y > -OUTSIDE_LEVEL && d.y < 0.f) ||
        (cam.pos.y < max.y && d.y > 0.f)) cam.pos.y += d.y;

    return cam;
}

void cam2_adjust(Cam2 *cam, glf2 pin, GLfloat tightness, glf2 level_size)
{
    *cam = cam2_zoom(cam2_pin(*cam, pin, tightness, level_size), pin, level_size);
    cam2_upload(*cam);
}
