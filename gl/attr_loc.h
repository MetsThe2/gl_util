#ifndef ATTR_LOC_H_INCLUDED
#define ATTR_LOC_H_INCLUDED

enum ShaderAttrLocations {
    loc_vert_pos, loc_vert_clr,
    loc_inst_pos, loc_inst_rad, loc_inst_clr,
    shader_attrib_loc_num
};
/* Add attributes to the enum as needed, but don't remove any unless *no* program
including this header is using them. Some attributes are used in dots.c:
vert_pos, inst_pos, inst_rad and inst_clr. */

#define BIND_ATTR_LOC(p, a) glBindAttribLocation(p, loc_ ## a, #a)

#endif // ATTR_LOC_H_INCLUDED
