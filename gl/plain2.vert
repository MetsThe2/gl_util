#version 440
precision highp float;

in vec2 vert_pos;

layout(shared) uniform Cam2 {
    vec2 cam_pos;
    float cam_scale; // 1.0 / camera.radius
};

void main(void)
{
    gl_Position = vec4((vert_pos - cam_pos) * cam_scale - 1.0, 0.0, 1.0);
}