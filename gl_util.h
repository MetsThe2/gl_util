#ifndef GL_UTIL_H_INCLUDED
#define GL_UTIL_H_INCLUDED

#include <math.h>
#ifndef M_PI
#define M_PI     3.14159265358979323846f
#endif
#define M_SQRT_2 1.41421356237309504880f

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// GLEW is in gl_types.h, because with GLEW, to include the OpenGL typedef-s,
// everything else must be included as well.
#include "gl_types.h"
#define GLFW_DLL        // GLFW should be included after gl_types, because GLEW is
#include <glfw/glfw3.h> // included in that header.

GLFWwindow * basic_init(int width, int height, const char *title);
void basic_cleanup(GLFWwindow *window);

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

#define IS_KEY(k) (glfwGetKey(window, GLFW_KEY_ ## k) == GLFW_PRESS)
#define IS_MOUSE_BUTTON(b) \
    (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_ ## b) == GLFW_PRESS)

#define INPUT_NO_REPEAT_DECL(f) int f(GLFWwindow *window)
#define INPUT_NO_REPEAT(f, k) \
int f(GLFWwindow *window) \
{ \
    static int was_down = 0; \
    if (k) { \
        if (was_down) return 0; /* Key is being held down. */ \
        return was_down = 1;    /* Key changed from not-down to down. */ \
    } \
    return was_down = 0; /* Key is up. */ \
}

INPUT_NO_REPEAT_DECL(toggle_pause);

// Set time to this with glfwSetTime before starting main loop.
#define MAIN_LOOP_START_TIME 0.0

int get_scroll_dir(void);
void poll_events(void);
double next_frame(GLFWwindow *window); // Returns frame length in seconds

enum ShaderAttribLocations {
    loc_vert_pos, loc_inst_pos, loc_inst_rad, loc_inst_clr, shader_attrib_loc_num};
// Add attributes to the enum as needed, but don't remove any unless *no* program
// including this header is using them. Some attributes are used in gl_util.c:
// vert_pos, inst_pos, inst_rad and inst_clr (for DebugDots).
#define BIND_ATTR_LOC(p, a) glBindAttribLocation(p, loc_ ## a, #a)

#define GET_UNI_LOC(p, u) GET_UNI_LOC2(p, loc_ ## u, #u)
#define GET_UNI_LOC2(p, l, s) const GLint l = glGetUniformLocation(p, s)

GLuint get_shader(GLenum type, char *filename);
#define GET_SHADER_v(s) get_shader(GL_VERTEX_SHADER,   s ".vert")
#define GET_SHADER_f(s) get_shader(GL_FRAGMENT_SHADER, s ".frag")
#define GET_SHADER_g(s) get_shader(GL_GEOMETRY_SHADER, s ".geom")
#define GET_SHADER_c(s) get_shader(GL_COMPUTE_SHADER,  s ".comp")
// If t isn't one of v (vertex), f (fragment), g (geometry) or c (compute),
// concatenating GET_SHADER and t will yield a macro that doesn't exist and
// result in a compiler error.
#define GET_SHADER2(m, s) m(s)
#define GET_SHADER(n, t) GET_SHADER2(GET_SHADER_ ## t, #n)

GLuint make_prog(GLuint vert, GLuint frag, void (*before_linking)(GLuint prog));
GLuint get_prog(char *vert_filename, char *frag_filename,
                void (*before_linking)(GLuint prog));
void del_progs(GLuint *progs, unsigned prog_num);

void vert_attrib_ptr(GLint loc, unsigned comp_num, unsigned div);

int pause_or_exit(GLFWwindow *window);

// buf_* functions use the currently bound buffer unless otherwise specified.
void buf_orphan_f( size_t num, GLenum usage);
void buf_orphan_f2(size_t num, GLenum usage);
void buf_orphan_f3(size_t num, GLenum usage);
void buf_orphan_i( size_t num, GLenum usage);

int buf_put_f(   const GLfloat  *data, size_t start, size_t num);
int buf_put_f2(  const glf2     *data, size_t start, size_t num);
int buf_put_f3(  const glf3     *data, size_t start, size_t num);
int buf_put_clr3(const clr3     *data, size_t start, size_t num);
int buf_put_i(   const BufIndex *data, size_t start, size_t num);

#define DEL_BUF(b) do { glDeleteBuffers(1, &(b)); (b) = 0; } while (0)

#define CIRCLE_VERT_NUM 17
#define CIRCLE_BUF_SIZE (CIRCLE_VERT_NUM + 2) // This is the
// number of vertices stored in buffer per circle. The first and last vertex is
// duplicated to create degenerate triangles (and avoid linking all triangle strips
// together), so CIRCLE_BUF_SIZE > CIRCLE_VERT_NUM.
void buf_gen_circles(const glf2 *pos, const GLfloat *rad, unsigned num, GLenum usage);
// buf_gen_circles_with_norm generates into two different GL_ARRAY_BUFFERs, so it
// has to bind the buffers itself.
void buf_gen_circles_with_norm(const glf2 *pos, const GLfloat *rad, unsigned num,
                               GLuint vert_buf, GLuint norm_buf, GLenum usage);

clr3 set_clr3(clr3 *c, GLfloat r, GLfloat g, GLfloat b);
glf3 set_glf3(glf3 *v, GLfloat x, GLfloat y, GLfloat z);
glf2 set_glf2(glf2 *v, GLfloat x, GLfloat y);

int eq_glf2( glf2 a, glf2 b);
int neq_glf2(glf2 a, glf2 b);

glf2 add_glf2(glf2 a, glf2 b);
glf2 sub_glf2(glf2 a, glf2 b);
glf2 mul_glf2(glf2 a, glf2 b);

glf2 scale_glf2(glf2 a, GLfloat s);
glf2 unit_glf2(glf2 v);

GLfloat dot_glf2(glf2 a, glf2 b);
GLfloat sqr_abs_glf2(glf2 v);     // Square of absolute value == distance from origin
GLfloat sqr_dist_glf2(glf2 a, glf2 b);

glf2 line_nearest(glf2 a, glf2 b, glf2 v); // a and b are the endpoints of the line.

#define MAX_SIDE_ROT_NUM 8 // The (inclusive) range of valid inputs to get_side_rot
#define MIN_SIDE_ROT_NUM 3 // 8 is the maximum because > 8 sides ~= circle.
glf2 get_side_rot(unsigned side_num);
glf2 rot_glf2(glf2 v, GLfloat rot_cos, GLfloat rot_sin);

typedef struct Cam2 {
    glf2 pos;
    GLfloat rad;
    GLuint uni_buf;
    GLuint pos_offset; // Offsets of cam_pos and cam_scale in uniform buffer
    GLuint scale_offset;
} Cam2;
Cam2 cam2_init(GLfloat pos_x, GLfloat pos_y, GLfloat rad,
               const GLchar *uni_block_name, GLuint *progs, unsigned prog_num);
int cam2_is_null(const Cam2 *cam);
void cam2_del(Cam2 *cam);

Cam2 cam2_pin(Cam2 cam, glf2 pin, GLfloat tightness, glf2 level_size);
Cam2 cam2_zoom(Cam2 cam, glf2 pin, glf2 level_size);

void cam2_upload(Cam2 cam);

// Pins, zooms and uploads camera.
void cam2_adjust(Cam2 *cam, glf2 pin, GLfloat tightness, glf2 level_size);

typedef struct Cam3 {
    glf3 pos;
    glf3 dir;
    GLfloat rad;
    GLuint uni_buf;
    GLuint pos_offset;
    GLuint dir_offset;
    GLuint rad_offset;
} Cam3;

// TODO: cam3_* functions

glf2 get_cursor_pos(GLFWwindow *window, Cam2 cam);

GLfloat rand_float(GLfloat max);
glf2 rand_glf2(glf2 max);

typedef struct DebugDots {
    glf2 *pos;
    GLfloat *rad;
    clr3 *clr;
    GLuint vao;
    GLuint model_buf;
    GLuint pos_buf;
    GLuint rad_buf;
    GLuint clr_buf;
    unsigned model_size;
    unsigned num;
} DebugDots;

// Call this before linking dots_prog (made with dots.vert and vary.frag).
void bind_dots_attrib_loc(GLuint prog);

DebugDots dots_alloc(unsigned num); // Allocate pos, rad and clr arrays to initialize.
int dots_are_null(const DebugDots *dots); // Returns whether *dots is invalid.
// Creates buffers, uploads static data (unit circle model, colors, radiuses).
void dots_init_bufs(DebugDots *dots);
void dots_del(DebugDots *dots);

void dots_upload(const DebugDots *dots);
void dots_draw(const DebugDots *dots, GLuint dots_prog);

#endif // GL_UTIL_H_INCLUDED
