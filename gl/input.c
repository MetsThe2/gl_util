#include "input.h"

static int scroll_dir = 0; // -1 == down, 0 == none, 1 == up

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    scroll_dir = yoffset < 0.0 ? -1 : 1;
}

int get_scroll_dir(void)
{
    return scroll_dir;
}

void poll_events(void)
{
    scroll_dir = 0; // Reset global variable each frame after use.
    glfwPollEvents();
}

INPUT_NO_REPEAT(toggle_pause, IS_KEY(P))

glf2 get_cursor_pos(GLFWwindow *window, Cam2 cam)
{
    double cursor_pos_x_d, cursor_pos_y_d;
    glfwGetCursorPos(window, &cursor_pos_x_d, &cursor_pos_y_d);
    glf2 cursor_pos = {(GLfloat)cursor_pos_x_d, (GLfloat)cursor_pos_y_d};

    int window_width_i, window_height_i;
    glfwGetWindowSize(window, &window_width_i, &window_height_i);
    const glf2 window_size  = {(GLfloat)window_width_i, (GLfloat)window_height_i};

    cursor_pos.y = window_size.y - cursor_pos.y;

    const GLfloat diam = 2.f * cam.rad;
    cursor_pos.x *= diam / window_size.x;
    cursor_pos.y *= diam / window_size.y;

    cursor_pos.x += cam.pos.x;
    cursor_pos.y += cam.pos.y;

    return cursor_pos;
}
