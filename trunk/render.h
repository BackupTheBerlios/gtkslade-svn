
void render_3d_view();
void setup_3d_data();

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

	void draw()
	{
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
		if (tex) glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());
		glBegin(GL_TRIANGLE_FAN);

		for (int a = 0; a < verts.size(); a++)
			verts[a].gl_draw(true);

		glEnd();
	}
};

struct flatpoly_t
{
	int		parent_sector;
	BYTE	part;			// 0 = floor, 1 = ceiling
	int		ssector;

	// Draw info
	Texture*			tex;
	vector<point3_t>	verts;
	rgba_t				col;
	BYTE				light;

	flatpoly_t();

	void draw()
	{
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
		if (tex) glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());
		
		glBegin(GL_TRIANGLE_FAN);

		for (int a = 0; a < verts.size(); a++)
			verts[a].gl_draw(true);

		glEnd();
	}
};
