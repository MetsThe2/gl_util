#version 440
precision highp float;

uniform vec3 uni_clr;

out vec4 frag_clr;

void main(void)
{
    frag_clr = vec4(uni_clr, 1.0);
}