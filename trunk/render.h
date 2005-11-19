
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
	//rgba_t				col;
	BYTE				light;

	wallrect_t();
	~wallrect_t();

	void draw();
	void draw_hilight();
};

struct flatpoly_t
{
	int		parent_sector;
	BYTE	part;			// 0 = floor, 1 = ceiling

	// Draw info
	GLuint				tex;
	vector<point3_t>	verts;
	vector<bool>		lines;
	//rgba_t				col;
	BYTE				light;

	flatpoly_t();
	~flatpoly_t();

	void draw();
	void draw_hilight();
};

struct thing3d_t
{
	Texture*	sprite;
	int			parent_sector;

	thing3d_t();
};

struct sectinfo_t
{
	plane_t	f_plane;
	plane_t	c_plane;
	bool visible;
};

struct line3d_t
{
	vector<wallrect_t*> rects;
	bool				visible;
	BYTE				alpha;
};

struct ssect3d_t
{
	vector<flatpoly_t*> flats;
	bool visible;
};

void setup_wallrect(wallrect_t *wall);
void setup_flatpoly(flatpoly_t *poly, int ssector);
void setup_sector(int s);
void setup_3d_line(int line);
void render_3d_view();
void setup_3d_data();
void setup_specials_3d(int sector = -1);
void setup_slopes_3d(int sector = -1);
float plane_height(plane_t plane, float x, float y);
void add_3d_message(string message);
