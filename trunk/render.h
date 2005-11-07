
void render_3d_view();
void setup_3d_data();
float plane_height(plane_t plane, float x, float y);

#include "textures.h"

#define SCALE_3D	0.01f

#define PART_UPPER	0	// Upper 'texture'
#define PART_MIDDLE	1	// Middle (solid, 1-sided) 'texture'
#define PART_LOWER	2	// Lower 'texture'
#define PART_TRANS	3	// Transparent middle (bars, etc)

#define PART_FLOOR	0
#define PART_CEIL	1

struct wallrect_t
{
	// Parent info
	int					sector;
	int					line;
	BYTE				side;
	BYTE				part;

	// Draw info
	Texture*			tex;
	vector<point3_t>	verts;
	rgba_t				col;
	BYTE				light;

	wallrect_t();
	~wallrect_t();

	void draw();
};

struct flatpoly_t
{
	int		parent_sector;
	BYTE	part;			// 0 = floor, 1 = ceiling
	int		ssector;

	// Draw info
	//Texture*			tex;
	GLuint				tex;
	vector<point3_t>	verts;
	rgba_t				col;
	BYTE				light;

	flatpoly_t();

	void draw();
};

struct sectinfo_t
{
	plane_t	f_plane;
	plane_t	c_plane;
	bool visible;

	/*
	BYTE light;
	Texture *f_tex;
	Texture *c_tex;
	*/
};
