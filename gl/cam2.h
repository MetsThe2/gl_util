#include "types.h"
#include "f2_type.h"
#include "cam2_type.h"

Cam2 cam2_init(GLfloat pos_x, GLfloat pos_y, GLfloat rad,
               const GLchar *uni_block_name, GLuint *progs, unsigned prog_num);
inline int cam2_is_null(const Cam2 *cam) { return !cam->uni_buf; }
void cam2_del(Cam2 *cam);

Cam2 cam2_pin(Cam2 cam, glf2 pin, GLfloat tightness, glf2 level_size);
Cam2 cam2_zoom(Cam2 cam, glf2 pin, glf2 level_size);

void cam2_upload(Cam2 cam);

// Pins, zooms and uploads camera.
void cam2_adjust(Cam2 *cam, glf2 pin, GLfloat tightness, glf2 level_size);
