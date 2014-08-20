#include "gl_util.h"
#include "fileutil.h" // For getting shaders

static void error_callback(int error, const char *description)
{
    fputs(description, stderr);
}

static int last_scroll_dir = 0; // -1 == down, 0 == none, 1 == up

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    last_scroll_dir = yoffset < 0.0 ? -1 : 1;
}

int get_scroll_dir(void)
{
    return last_scroll_dir;
}

void poll_events(void)
{
    last_scroll_dir = 0; // Reset global variable each frame after use.
    glfwPollEvents();
}

double next_frame(GLFWwindow *window)
{
    static double frame_start_time = MAIN_LOOP_START_TIME;

    const double frame_end_time = glfwGetTime();
    const double frame_time = frame_end_time - frame_start_time;
    frame_start_time = frame_end_time;

    glfwSwapBuffers(window);

    return frame_time;
}

INPUT_NO_REPEAT(toggle_pause, IS_KEY(T))

int pause_or_exit(GLFWwindow *window)
{
    /* Complete pause except for exit key. In a finished app, there's usually
    still things to update and render when paused, but this function is good
    for initial prototyping / debugging. */
    if (IS_KEY(ESCAPE)) return 1; // 1 == exit
    if (toggle_pause(window)) {
        do {
            glfwWaitEvents();
            if (IS_KEY(ESCAPE)) return 1;
        } while (!toggle_pause(window));
    }
    return 0; // 0 == !exit == keep playing
}

void basic_cleanup(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

static int init_glew(void)
{
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 1;
    } else {
        printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    }
    return 0;
}

GLFWwindow * basic_init(int width, int height, const char *title)
{
    int max_loc_num;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_loc_num);
    if (shader_attrib_loc_num > max_loc_num) {
        fputs("Too many shader attribute locations used.\n", stderr);
        return NULL;
    }

    glfwSetErrorCallback(&error_callback);
    if (!glfwInit()) return NULL;

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        fputs("glfwCreateWindow returned NULL.\n", stderr);
        glfwTerminate();
        return NULL;
    }
    glfwSetScrollCallback(window, &scroll_callback);
    glfwMakeContextCurrent(window);

    if (!init_glew()) return window; // Success

    basic_cleanup(window);
    return NULL;
}

GLuint get_shader(GLenum type, char *filename)
{
    GLchar *source = str_from_file(filename);
    if (!source) {
        fprintf(stderr, "Couldn't open << %s >>.\n", filename);
        return 0;
    }

    GLuint shader = glCreateShader(type);
    if (!shader) {
        fprintf(stderr, "Couldn't create shader << %s >>.\n", filename);
        return 0;
    }

    glShaderSource(shader, 1, (const GLchar **)&source, NULL);

    free(source);

    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        fprintf(stderr, "The shader << %s >> didn't compile.\n", filename);
        return 0;
    }

    return shader;
}

GLuint make_prog(GLuint vert, GLuint frag, void (*before_linking)(GLuint prog))
{
    GLuint prog = glCreateProgram();
    if (!prog) {
        fputs("Couldn't create shader program.\n", stderr);
        return 0;
    }
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);

    (*before_linking)(prog);

    glLinkProgram(prog);
    GLint linked;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        fputs("Shader program didn't link.\n", stderr);
        glDetachShader(prog, vert);
        glDetachShader(prog, frag);
        glDeleteProgram(prog);
        return 0;
    }

    glDetachShader(prog, vert);
    glDetachShader(prog, frag);
    return prog;
}

GLuint get_prog(char *vert_filename, char *frag_filename,
                void (*before_linking)(GLuint prog))
{
    GLuint prog = 0;

    GLuint vert_shader = get_shader(GL_VERTEX_SHADER,   vert_filename);
    if (!vert_shader) goto null_vert;
    GLuint frag_shader = get_shader(GL_FRAGMENT_SHADER, frag_filename);
    if (!frag_shader) goto null_frag;

    prog = make_prog(vert_shader, frag_shader, before_linking);

    glDeleteShader(frag_shader);
null_frag:
    glDeleteShader(vert_shader);
null_vert:

    return prog;
}

void del_progs(GLuint *progs, unsigned prog_num)
{
    for (unsigned i = 0; i < prog_num; ++i) glDeleteProgram(progs[i]);
    memset(progs, 0, prog_num * sizeof(*progs));
}

void vert_attrib_ptr(GLint loc, unsigned comp_num, unsigned div)
{
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, comp_num, GL_FLOAT, GL_FALSE, 0, NULL);
    glVertexAttribDivisor(loc, div);
}

void buf_orphan_f(size_t num, GLenum usage)
{
    glBufferData(GL_ARRAY_BUFFER, num * sizeof(GLfloat), NULL, usage);
}
void buf_orphan_f2(size_t num, GLenum usage)
{
    buf_orphan_f(num * 2, usage);
}
void buf_orphan_f3(size_t num, GLenum usage)
{
    buf_orphan_f(num * 3, usage);
}
void buf_orphan_i(size_t num, GLenum usage)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num * sizeof(BufIndex), NULL, usage);
}

#define MAP_FAST_WRITE (GL_MAP_WRITE_BIT | \
                        GL_MAP_UNSYNCHRONIZED_BIT | \
                        GL_MAP_INVALIDATE_RANGE_BIT)

int buf_put_f(const GLfloat *data, size_t start, size_t num)
{
    num *= sizeof(GLfloat);

    GLfloat *p = glMapBufferRange(GL_ARRAY_BUFFER,
                                  start * sizeof(GLfloat), num, MAP_FAST_WRITE);
    if (!p) return 1;
    memcpy(p, data, num);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    return 0;
}
int buf_put_f2(const glf2 *data, size_t start, size_t num)
{
    return buf_put_f((const GLfloat *)data, start * 2, num * 2);
}
int buf_put_f3(const glf3 *data, size_t start, size_t num)
{
    return buf_put_f((const GLfloat *)data, start * 3, num * 3);
}
int buf_put_clr3(const clr3 *data, size_t start, size_t num)
{
    return buf_put_f((const GLfloat *)data, start * 3, num * 3);
}

int buf_put_i(const BufIndex *data, size_t start, size_t num)
{
    num *= sizeof(BufIndex);

    BufIndex *p = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,
                                   start * sizeof(BufIndex), num, MAP_FAST_WRITE);
    if (!p) return -1;
    memcpy(p, data, num * sizeof(BufIndex));

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    return 0;
}

clr3 set_clr3(clr3 *c, GLfloat r, GLfloat g, GLfloat b)
{
    c->r = r;
    c->g = g;
    c->b = b;
    return *c;
}

glf3 set_glf3(glf3 *v, GLfloat x, GLfloat y, GLfloat z)
{
    v->x = x;
    v->y = y;
    v->z = z;
    return *v;
}

glf2 set_glf2(glf2 *v, GLfloat x, GLfloat y)
{
    v->x = x;
    v->y = y;
    return *v;
}

int eq_glf2(glf2 a, glf2 b)
{
    return a.x == b.x && a.y == b.y;
}

int neq_glf2(glf2 a, glf2 b)
{
    return a.x != b.x || a.y != b.y;
}

glf2 add_glf2(glf2 a, glf2 b)
{
    glf2 r = {.x = a.x + b.x, .y = a.y + b.y};
    return r;
}

glf2 sub_glf2(glf2 a, glf2 b)
{
    glf2 r = {.x = a.x - b.x, .y = a.y - b.y};
    return r;
}

glf2 mul_glf2(glf2 a, glf2 b)
{
    glf2 r = {.x = a.x * b.x, .y = a.y * b.y};
    return r;
}

glf2 scale_glf2(glf2 a, GLfloat s)
{
    a.x *= s;
    a.y *= s;
    return a;
}

GLfloat dot_glf2(glf2 a, glf2 b)
{
    return (a.x * b.x) + (a.y * b.y);
}

GLfloat sqr_abs_glf2(glf2 v)
{
    return dot_glf2(v, v);
}

GLfloat sqr_dist_glf2(glf2 a, glf2 b)
{
    return sqr_abs_glf2(sub_glf2(a, b));
}

glf2 unit_glf2(glf2 v)
{
    return scale_glf2(v, 1.0 / sqrt(sqr_abs_glf2(v)));
}

glf2 line_nearest(glf2 a, glf2 b, glf2 v) // a and b are the endpoints of the line.
{
    const glf2 ab = sub_glf2(b, a); // ab = a -> b
    const glf2 av = sub_glf2(v, a); // av = a -> v
    const GLfloat s = dot_glf2(av, ab) / sqr_abs_glf2(ab); // s = (av . ab) / |ab|**2
    if (s <= 0.0) return a;
    if (s >= 1.0) return b;
    return add_glf2(a, scale_glf2(ab, s)); // a + (ab * s)
}

glf2 rot_glf2(glf2 v, GLfloat rot_cos, GLfloat rot_sin)
{
    glf2 r;
    r.x = (v.x * rot_cos) - (v.y * rot_sin);
    r.y = (v.x * rot_sin) + (v.y * rot_cos);
    return r;
}

glf2 get_side_rot(unsigned side_num)
{
#define GEN(n) \
    {.x = cos(2.0f * M_PI / (GLfloat)(n)), .y = sin(2.0f * M_PI / (GLfloat)(n))}
    const glf2 rots[] = {GEN(MIN_SIDE_ROT_NUM), GEN(4), GEN(5), GEN(6), GEN(7),
                         GEN(MAX_SIDE_ROT_NUM)};
#undef GEN
    if (side_num > ARRAY_LEN(rots) + MIN_SIDE_ROT_NUM) {
        fprintf(stderr, "Can't get rotation vector for %u sides.\n", side_num);
        exit(3);
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

#if 1 // So the macros can be folded in a code editor (Code::Blocks, Notepad++, etc).
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
        /* The IF's condition should be !(CIRCLE_BUF_SIZE % 2) here. */ \
    } while (++i IF(1, < CIRCLE_BUF_SIZE));
#endif // 1 (always true)

static void gen_circle(glf2 pos, GLfloat rad, glf2 *vert)
{
    GEN_CIRCLE(0);
}

static void gen_circle_with_norm(glf2 pos, GLfloat rad, glf2 *vert, glf2 *norm)
{
    GEN_CIRCLE(1);
}

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

enum BindingIndices {cam_bind_index};

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
        glUniformBlockBinding(progs[i], block_index, cam_bind_index);
    } while (--i >= 0);

    glGenBuffers(1, &cam.uni_buf);
    glBindBufferBase(GL_UNIFORM_BUFFER, cam_bind_index, cam.uni_buf);
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

int cam2_is_null(const Cam2 *cam)
{
    return !cam->uni_buf;
}

void cam2_upload(Cam2 cam)
{
    glBindBuffer(GL_UNIFORM_BUFFER, cam.uni_buf);
    glBufferSubData(GL_UNIFORM_BUFFER, cam.pos_offset, sizeof(cam.pos), &cam.pos);
    const GLfloat cam_scale = 1.0 / cam.rad;
    glBufferSubData(GL_UNIFORM_BUFFER, cam.scale_offset,
                    sizeof(cam_scale), &cam_scale);
}

void cam2_del(Cam2 *cam)
{
    glDeleteBuffers(1, &cam->uni_buf);
    cam->uni_buf = 0;
}

// Maximum distance of edge of visible area from the outside of the level border
#define CAM_OUTSIDE_LEVEL 2.0

Cam2 cam2_pin(Cam2 cam, glf2 pin, GLfloat tightness, glf2 level_size)
{
    GLfloat diam = 2.0 * cam.rad;

    assert(tightness >= 0.0 && tightness <= 1.0);
    GLfloat min_dist = cam.rad * tightness;
    GLfloat max_dist = diam - min_dist;

    // I have no idea what to name this variable.
    glf2 o = {.x = level_size.x + CAM_OUTSIDE_LEVEL,
              .y = level_size.y + CAM_OUTSIDE_LEVEL};

    if (pin.x < cam.pos.x + min_dist) {
        GLfloat new_pos_x = pin.x - min_dist;
        if (new_pos_x > -CAM_OUTSIDE_LEVEL) cam.pos.x = new_pos_x;
    } else if (pin.x > cam.pos.x + max_dist) {
        GLfloat new_pos_x = pin.x - max_dist;
        if (new_pos_x + diam < o.x) cam.pos.x = new_pos_x;
    }
    if (pin.y < cam.pos.y + min_dist) {
        GLfloat new_pos_y = pin.y - min_dist;
        if (new_pos_y > -CAM_OUTSIDE_LEVEL) cam.pos.y = new_pos_y;
    } else if (pin.y > cam.pos.y + max_dist) {
        GLfloat new_pos_y = pin.y - max_dist;
        if (new_pos_y + diam < o.y) cam.pos.y = new_pos_y;
    }

    return cam;
}

#define CAM_ZOOM_STEP 25.0

Cam2 cam2_zoom(Cam2 cam, glf2 pin, glf2 level_size)
{
    if (!last_scroll_dir) return cam;

    const GLfloat zoom = CAM_ZOOM_STEP * (GLfloat)last_scroll_dir;
    const GLfloat diam = 2.0 * (cam.rad -= zoom);

    // Make zooming smoother by moving camera after zooming so that the
    // pinned position remains closer to the center.
    // TODO: Prevent the area > cam.pos + diam from becoming visible
    // when zooming out unless it's unavoidable (when cam.rad > level_size).

    const glf2 d = scale_glf2(sub_glf2(pin, cam.pos), zoom / cam.rad);

    const glf2 max = {.x = level_size.x + CAM_OUTSIDE_LEVEL - diam,
                      .y = level_size.y + CAM_OUTSIDE_LEVEL - diam};

    if ((cam.pos.x > -CAM_OUTSIDE_LEVEL && d.x < 0.0) ||
        (cam.pos.x < max.x && d.x > 0.0)) cam.pos.x += d.x;
    if ((cam.pos.y > -CAM_OUTSIDE_LEVEL && d.y < 0.0) ||
        (cam.pos.y < max.y && d.y > 0.0)) cam.pos.y += d.y;

    return cam;
}

void cam2_adjust(Cam2 *cam, glf2 pin, GLfloat tightness, glf2 level_size)
{
    *cam = cam2_zoom(cam2_pin(*cam, pin, tightness, level_size), pin, level_size);
    cam2_upload(*cam);
}

glf2 get_cursor_pos(GLFWwindow *window, Cam2 cam)
{
    double cursor_pos_x_d, cursor_pos_y_d;
    glfwGetCursorPos(window, &cursor_pos_x_d, &cursor_pos_y_d);
    glf2 cursor_pos = {.x = (GLfloat)cursor_pos_x_d, .y = (GLfloat)cursor_pos_y_d};

    int window_width_i, window_height_i;
    glfwGetWindowSize(window, &window_width_i, &window_height_i);
    glf2 window_size  = {.x = (GLfloat)window_width_i, .y = (GLfloat)window_height_i};

    cursor_pos.y = window_size.y - cursor_pos.y;

    const GLfloat diam = 2.0 * cam.rad;
    cursor_pos.x *= diam / window_size.x;
    cursor_pos.y *= diam / window_size.y;

    cursor_pos.x += cam.pos.x;
    cursor_pos.y += cam.pos.y;

    return cursor_pos;
}

GLfloat rand_float(GLfloat max)
{
    return max * (((long double)rand()) / (long double)RAND_MAX);
}

glf2 rand_glf2(glf2 max)
{
    glf2 r = {.x = rand_float(max.x), .y = rand_float(max.y)};
    return r;
}

static void dots_init_bufs(DebugDots *dots)
{
    glGenVertexArrays(1, &dots->vao);
    glBindVertexArray(dots->vao);

    glGenBuffers(1, &dots->model_buf);

    glBindBuffer(GL_ARRAY_BUFFER, dots->model_buf);
    buf_orphan_f2(dots->model_size, GL_STATIC_DRAW);

    const glf2 circle_pos = {.x = 0.0, .y = 0.0};
    const GLfloat circle_rad = 1.0;
    buf_gen_circles(&circle_pos, &circle_rad, 1, GL_STATIC_DRAW);

    vert_attrib_ptr(loc_vert_pos, 2, 0);

    glGenBuffers(1, &dots->pos_buf);
    glBindBuffer(GL_ARRAY_BUFFER, dots->pos_buf);
    buf_orphan_f2(dots->num, GL_STREAM_DRAW);
    vert_attrib_ptr(loc_inst_pos, 2, 1);

    glGenBuffers(1, &dots->rad_buf);
    glBindBuffer(GL_ARRAY_BUFFER, dots->rad_buf);
    buf_orphan_f(dots->num, GL_STREAM_DRAW);
    vert_attrib_ptr(loc_inst_rad, 1, 1);

    glGenBuffers(1, &dots->clr_buf);
    glBindBuffer(GL_ARRAY_BUFFER, dots->clr_buf);
    buf_orphan_f3(dots->num, GL_STREAM_DRAW);
    vert_attrib_ptr(loc_inst_clr, 3, 1);

    glBindVertexArray(0);
}

DebugDots dots_init(unsigned num)
{
    DebugDots dots;

    dots.pos = malloc(num * sizeof(*dots.pos));
    if (!dots.pos) {
        dots.rad = NULL;
        dots.clr = NULL;
        return dots;
    }
    dots.rad = malloc(num * sizeof(*dots.rad));
    if (!dots.rad) goto null_rad;
    dots.clr = malloc(num * sizeof(*dots.clr));
    if (!dots.clr) goto null_clr;

    dots.num = num;
    dots.model_size = CIRCLE_BUF_SIZE;
    dots_init_bufs(&dots);

    goto no_errors;
null_clr:
    free(dots.rad);
    dots.rad = NULL;
null_rad:
    free(dots.pos);
    dots.clr = NULL;
    dots.pos = NULL;
no_errors:
    return dots;
}

int dots_are_null(const DebugDots *dots)
{
    return dots->pos == NULL;
}

void dots_upload(DebugDots dots)
{
    glBindBuffer(GL_ARRAY_BUFFER, dots.pos_buf);
    buf_put_f2(dots.pos, 0, dots.num);

    glBindBuffer(GL_ARRAY_BUFFER, dots.rad_buf);
    buf_put_f( dots.rad, 0, dots.num);

    glBindBuffer(GL_ARRAY_BUFFER, dots.clr_buf);
    buf_put_clr3(dots.clr, 0, dots.num);
}

void dots_draw(const DebugDots *dots)
{
    glBindVertexArray(dots->vao);
    // Change TRIANGLE_STRIP to TRIANGLE_FAN to draw a porcupine.
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, dots->model_size, dots->num);
}

void dots_del(DebugDots *dots)
{
    DEL_BUF(dots->model_buf);
    DEL_BUF(dots->pos_buf);
    DEL_BUF(dots->rad_buf);
    DEL_BUF(dots->clr_buf);

    free(dots->pos);
    dots->pos = NULL;

    free(dots->rad);
    dots->rad = NULL;

    free(dots->clr);
    dots->clr = NULL;
}

void bind_dots_attrib_loc(GLuint prog)
{
    BIND_ATTR_LOC(prog, vert_pos);
    BIND_ATTR_LOC(prog, inst_pos);
    BIND_ATTR_LOC(prog, inst_rad);
    BIND_ATTR_LOC(prog, inst_clr);
}
#define last_line
