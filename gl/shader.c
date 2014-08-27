#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "funcs.h"
#include "../fileutil.h"
#include "shader.h"

GLuint get_shader(GLenum type, char *filename)
{
    GLchar *source = str_from_file(filename);
    if (!source) {
        fprintf(stderr, "Couldn't open << %s >>.\n", filename);
        return 0;
    }

    GLuint shader = glCreateShader(type);
    if (!shader) {
        fprintf(stderr, "Couldn't create shader << %s >>.\n", filename);
        return 0;
    }

    glShaderSource(shader, 1, (const GLchar **)&source, NULL);

    free(source);

    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        fprintf(stderr, "The shader << %s >> didn't compile.\n", filename);
        return 0;
    }

    return shader;
}

GLuint make_prog(GLuint vert, GLuint frag, void (*before_linking)(GLuint prog))
{
    GLuint prog = glCreateProgram();
    if (!prog) {
        fputs("Couldn't create shader program.\n", stderr);
        return 0;
    }
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);

    if (before_linking) (*before_linking)(prog);

    glLinkProgram(prog);
    GLint linked;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        fputs("Shader program didn't link.\n", stderr);
        glDetachShader(prog, vert);
        glDetachShader(prog, frag);
        glDeleteProgram(prog);
        return 0;
    }

    glDetachShader(prog, vert);
    glDetachShader(prog, frag);
    return prog;
}

GLuint get_prog(char *vert_filename, char *frag_filename,
                void (*before_linking)(GLuint prog))
{
    GLuint prog = 0;

    const GLuint vert_shader = get_shader(GL_VERTEX_SHADER,   vert_filename);
    if (!vert_shader) goto null_vert;
    const GLuint frag_shader = get_shader(GL_FRAGMENT_SHADER, frag_filename);
    if (!frag_shader) goto null_frag;

    prog = make_prog(vert_shader, frag_shader, before_linking);

    glDeleteShader(frag_shader);
null_frag:
    glDeleteShader(vert_shader);
null_vert:

    return prog;
}

void del_progs(GLuint *progs, unsigned prog_num)
{
    for (unsigned i = 0; i < prog_num; ++i) glDeleteProgram(progs[i]);
    memset(progs, 0, prog_num * sizeof(*progs));
}
