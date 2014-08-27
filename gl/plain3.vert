#version 440
precision highp float;

in vec3 vert_pos;

layout(shared) uniform Cam3 {
    mat4 cam_mat;
    vec3 cam_pos;
};

void main(void)
{
    vec4 cam_space_pos = vec4(vert_pos - cam_pos, 1.0)
    gl_Position = cam_mat * cam_space_pos;
}