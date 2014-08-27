#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include "f2_type.h"
#include "cam2_type.h"
#define GLFW_DLL
#include "glfw/glfw3.h"

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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
int get_scroll_dir(void);
void poll_events(void);
glf2 get_cursor_pos(GLFWwindow *window, Cam2 cam);

#endif // INPUT_H_INCLUDED
