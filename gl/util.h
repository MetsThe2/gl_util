#ifndef GL_UTIL_H_INCLUDED
#define GL_UTIL_H_INCLUDED

#include <assert.h>
#include "../math_def.h"
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
#include "clr3.h"
#include "buf.h"
#include "gen.h"
#include "cam2.h"
#include "cam3.h"
#include "input.h"
#include "dots.h"

GLFWwindow * basic_init(int width, int height, const char *title, int use_cam3);
void basic_cleanup(GLFWwindow *window);

void maybe_complete_pause(GLFWwindow *window);

inline GLfloat rand_float(GLfloat max) {
    return max * (((long double)rand()) / (long double)RAND_MAX);
}
inline glf2 rand_glf2(glf2 max) { return (glf2){rand_float(max.x), rand_float(max.y)}; }

#endif // GL_UTIL_H_INCLUDED
