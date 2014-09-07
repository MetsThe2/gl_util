#version 440
precision highp float;

in vec3 vert_pos;

layout(shared) uniform Cam3 { mat4 cam_mat; };

void main(void)
{
    gl_Position = cam_mat * vec4(vert_pos, 1.0);
}