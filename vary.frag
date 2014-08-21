#version 440
precision highp float;

in vec3 vary_clr;

out vec4 frag_clr;

void main(void)
{
    frag_clr = vec4(vary_clr, 1.0);
}