#include <stdio.h>

#include "funcs.h"
#define GLFW_DLL
#include "glfw/glfw3.h"

#include "attr_loc.h"
#include "input.h"
#include "cam3.h"

static void error_callback(int error, const char *description)
{
    fputs(description, stderr);
}

static void resize_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    cam3_on_window_resize(width, height);
}
/*
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
*/
void basic_cleanup(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

GLFWwindow * basic_init(int width, int height, const char *title)
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
        glfwSetWindowSizeCallback(window, &resize_callback);
        return window;
    }
    fputs("Too many shader attribute locations used.\n", stderr);

load_failed:
    basic_cleanup(window);
    return NULL;
}

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
