#include "funcs.h"
#include "f2_type.h"
#include "f3_type.h"
#include "clr3_type.h"
#include "buf_index_type.h"

#define DEL_BUF(b) do { glDeleteBuffers(1, &(b)); (b) = 0; } while (0)

inline void attr_src(GLuint loc, GLint comp_num, GLsizei stride, unsigned offset,
                     unsigned div)
{
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, comp_num, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
    glVertexAttribDivisor(loc, div);
}

// buf_* functions use the currently bound buffer unless otherwise specified.

inline void buf_orphan_f(size_t num, GLenum usage) {
    glBufferData(GL_ARRAY_BUFFER, num * sizeof(GLfloat), NULL, usage);
}
inline void buf_orphan_f2(size_t num, GLenum usage) {
    buf_orphan_f(num * 2, usage);
}
inline void buf_orphan_f3(size_t num, GLenum usage) {
    buf_orphan_f(num * 3, usage);
}
inline void buf_orphan_i(size_t num, GLenum usage) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num * sizeof(BufIndex), NULL, usage);
}

int buf_put_f(   const GLfloat  *data, size_t start, size_t num);
int buf_put_f2(  const glf2     *data, size_t start, size_t num);
int buf_put_f3(  const glf3     *data, size_t start, size_t num);
int buf_put_clr3(const clr3     *data, size_t start, size_t num);
int buf_put_i(   const BufIndex *data, size_t start, size_t num);
