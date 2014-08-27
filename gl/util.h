#ifndef GL_UTIL_H_INCLUDED
#define GL_UTIL_H_INCLUDED

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "funcs.h"
#define GLFW_DLL
#include "glfw/glfw3.h"

#include "../pp_sak.h"

#include "shader.h"
#include "f2.h"
#include "f3.h"
#include "buf.h"
#include "buf_gen.h"
#include "cam2.h"
#include "cam3.h"
#include "input.h"
#include "dots.h"

// Set time to this with glfwSetTime before starting main loop.
#define MAIN_LOOP_START_TIME 0.0

GLFWwindow * basic_init(int width, int height, const char *title);
void basic_cleanup(GLFWwindow *window);

int pause_or_exit(GLFWwindow *window);

inline GLfloat rand_float(GLfloat max) {
    return max * (((long double)rand()) / (long double)RAND_MAX);
}
inline glf2 rand_glf2(glf2 max) {
    glf2 r = {.x = rand_float(max.x), .y = rand_float(max.y)};
    return r;
}

#endif // GL_UTIL_H_INCLUDED
