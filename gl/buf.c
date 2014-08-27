#include "buf.h"

#define MAP_FAST_WRITE (GL_MAP_WRITE_BIT | \
                        GL_MAP_UNSYNCHRONIZED_BIT | \
                        GL_MAP_INVALIDATE_RANGE_BIT)

int buf_put_f(const GLfloat *data, size_t start, size_t num)
{
    glBufferSubData(GL_ARRAY_BUFFER,
                    start * sizeof(GLfloat), num * sizeof(GLfloat), data);
    /* Mapping is very slow with a Geforce GTX 560 and OpenGL 4.4. If it's much
    faster on other setups, use the commented out implementation. */
    /* num *= sizeof(GLfloat);

    GLfloat *p = glMapBufferRange(GL_ARRAY_BUFFER,
                                  start * sizeof(GLfloat), num, MAP_FAST_WRITE);
    if (!p) return 1;
    memcpy(p, data, num);

    glUnmapBuffer(GL_ARRAY_BUFFER); */
    return 0;
}
int buf_put_f2(const glf2 *data, size_t start, size_t num)
{
    return buf_put_f((const GLfloat *)data, start * 2, num * 2);
}
int buf_put_f3(const glf3 *data, size_t start, size_t num)
{
    return buf_put_f((const GLfloat *)data, start * 3, num * 3);
}
int buf_put_clr3(const clr3 *data, size_t start, size_t num)
{
    return buf_put_f((const GLfloat *)data, start * 3, num * 3);
}

int buf_put_i(const BufIndex *data, size_t start, size_t num)
{
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                    start * sizeof(BufIndex), num * sizeof(BufIndex), data);
    /* num *= sizeof(BufIndex);

    BufIndex *p = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,
                                   start * sizeof(BufIndex), num, MAP_FAST_WRITE);
    if (!p) return 1;
    memcpy(p, data, num * sizeof(BufIndex));

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); */
    return 0;
}
