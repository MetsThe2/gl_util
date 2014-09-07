const glf3 cube_tri_strip_model[] = {
    {-1, -1, -1}, {-1, -1, -1}, {+1, -1, -1}, {-1, +1, -1},
    {+1, +1, -1}, {+1, +1, +1}, {+1, -1, -1}, {+1, -1, +1},
    {-1, -1, -1}, {-1, -1, +1}, {-1, +1, -1}, {-1, +1, +1},
    {-1, -1, +1}, {+1, +1, +1}, {+1, -1, +1}, {+1, -1, +1}
};

#define RED_COLOR 1.0f, 0.0f, 0.0f, 1.0f
#define GREEN_COLOR 0.0f, 1.0f, 0.0f, 1.0f
#define BLUE_COLOR 	0.0f, 0.0f, 1.0f, 1.0f

#define YELLOW_COLOR 1.0f, 1.0f, 0.0f, 1.0f
#define CYAN_COLOR 0.0f, 1.0f, 1.0f, 1.0f
#define MAGENTA_COLOR 	1.0f, 0.0f, 1.0f, 1.0f

#define PP_FOUR(x) x, x, x, x

const GLfloat cube_indexed_tri_model[] =
{
	//Front
	+1.0f, +1.0f, +1.0f,
	+1.0f, -1.0f, +1.0f,
	-1.0f, -1.0f, +1.0f,
	-1.0f, +1.0f, +1.0f,

	//Top
	+1.0f, +1.0f, +1.0f,
	-1.0f, +1.0f, +1.0f,
	-1.0f, +1.0f, -1.0f,
	+1.0f, +1.0f, -1.0f,

	//Left
	+1.0f, +1.0f, +1.0f,
	+1.0f, +1.0f, -1.0f,
	+1.0f, -1.0f, -1.0f,
	+1.0f, -1.0f, +1.0f,

	//Back
	+1.0f, +1.0f, -1.0f,
	-1.0f, +1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	+1.0f, -1.0f, -1.0f,

	//Bottom
	+1.0f, -1.0f, +1.0f,
	+1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, +1.0f,

	//Right
	-1.0f, +1.0f, +1.0f,
	-1.0f, -1.0f, +1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, +1.0f, -1.0f,

    PP_FOUR(GREEN_COLOR),
    PP_FOUR(BLUE_COLOR),
    PP_FOUR(RED_COLOR),
    PP_FOUR(YELLOW_COLOR),
    PP_FOUR(CYAN_COLOR),
    PP_FOUR(MAGENTA_COLOR),
};

const BufIndex cube_indexed_tri_indices[] =
{
    0, 1, 2,
    2, 3, 0,

	4, 5, 6,
	6, 7, 4,

	8, 9, 10,
	10, 11, 8,

	12, 13, 14,
	14, 15, 12,

	16, 17, 18,
	18, 19, 16,

	20, 21, 22,
	22, 23, 20,
};