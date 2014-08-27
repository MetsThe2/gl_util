#version 440
precision highp float;

in vec2  vert_pos;
in vec2  inst_pos;
in float inst_rad;
in vec3  inst_clr;

layout(shared) uniform Cam2 {
    vec2 cam_pos;
    float cam_scale; // 1.0 / camera radius
};

out vec3 vary_clr;

void main(void)
{
    const vec2 model = vert_pos * inst_rad;
    const vec2 view  = inst_pos - cam_pos;
    const vec2 proj  = (model + view) * cam_scale - 1.0;
    gl_Position = vec4(proj, 0.0, 1.0);
    
    vary_clr = inst_clr;
}