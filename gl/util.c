#include <stdio.h>

#include "funcs.h"
#define GLFW_DLL
#include "glfw/glfw3.h"

#include "attr_loc.h"
#include "input.h"
#include "f2.h"
#include "cam3.h"

static void error_callback(int error, const char *description)
{
    fputs(description, stderr);
}

static void adjust_viewport(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

static void resize_callback(GLFWwindow *window, int width, int height)
{
    adjust_viewport(window, width, height);
    cam3_on_window_resize(width, height);
}

static void cursor_pos_callback(GLFWwindow *window, double x_d, double y_d)
{
    int width_i, height_i;
    glfwGetWindowSize(window, &width_i, &height_i);
    const glf2 size = {width_i, height_i};

    const glf2 center = scale_glf2(size, 0.5f);
    const glf2 cursor = {x_d, size.y - y_d};

    cam3_rot(cursor, center, size);
    glfwSetCursorPos(window, center.x, center.y);
}

static void key_callback(GLFWwindow *window,
                         int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void basic_cleanup(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

GLFWwindow * basic_init(int width, int height, const char *title, int use_cam3)
{
    glfwSetErrorCallback(&error_callback);
    if (!glfwInit()) return NULL;

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        fputs("glfwCreateWindow returned NULL.\n", stderr);
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    int loaded = ogl_LoadFunctions();
    if (loaded == ogl_LOAD_FAILED) {
        fprintf(stderr, "%d OpenGL functions didn't load.\n",
                loaded - ogl_LOAD_SUCCEEDED);
        goto load_failed;
    }

    int max_loc_num;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_loc_num);
    if (shader_attrib_loc_num <= max_loc_num) {
        glfwSetScrollCallback(window, &scroll_callback);
        glfwSetKeyCallback(window, &key_callback);
        if (use_cam3) {
            glfwSetCursorPosCallback(window, &cursor_pos_callback);
            glfwSetWindowSizeCallback(window, &resize_callback);
        } else {
            glfwSetWindowSizeCallback(window, &adjust_viewport);
        }
        return window;
    }
    fputs("Too many shader attribute locations used.\n", stderr);

load_failed:
    basic_cleanup(window);
    return NULL;
}

void maybe_complete_pause(GLFWwindow *window)
{
    /* In a finished app, there's usually still things to update and render when
    paused, but this function is good for initial prototyping / debugging. */
    if (toggle_pause(window)) {
        while (!(glfwWindowShouldClose(window) || toggle_pause(window))) {
            glfwWaitEvents();
        }
    }
}
