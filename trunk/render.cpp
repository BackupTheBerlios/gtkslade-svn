
#include "main.h"
#include "struct_3d.h"
#include "camera.h"
#include "map.h"
#include "bsp.h"
#include "render.h"
#include "draw.h"

Camera camera;
vector<wallrect_t*> wallrects;
vector<flatpoly_t*> flatpolys;

CVAR(Bool, render_wireframe, false, CVAR_SAVE)

extern Map map;
extern bool vis_lines[65535];
extern bool vis_ssects[65535];
extern int n_gl_nodes;
extern GtkWidget *draw_3d_area;

extern rgba_t col_3d_crosshair;

wallrect_t::wallrect_t()
{
	wallrects.push_back(this);
	this->tex = NULL;
	this->sector = -1;
}

wallrect_t::~wallrect_t()
{
}

void wallrect_t::draw()
{
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
	if (tex) glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());
	glBegin(GL_TRIANGLE_FAN);

	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(true);

	glEnd();
}

flatpoly_t::flatpoly_t()
{
	flatpolys.push_back(this);
	this->parent_sector = -1;
	this->tex = NULL;
}

void flatpoly_t::draw()
{
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
	if (tex) glBindTexture(GL_TEXTURE_2D, tex);//->get_gl_id());

	glBegin(GL_TRIANGLE_FAN);

	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(true);

	glEnd();
}

float plane_height(plane_t plane, float x, float y)
{
	return ((plane.a * x) + (plane.b * y) + plane.d) / plane.c;
}

void set_light(rgba_t col, int light)
{
	//int light = map.sectors[sector]->light;
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
	glFogf(GL_FOG_DENSITY, 1.0f - light/500.0f);
	glFogf(GL_FOG_END, ((light/255.0f) * (light/255.0f) * 40.0f));
}

void render_3d_view()
{
	// Set 3d screen
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (GLfloat)320/(GLfloat)200, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Clear screen
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);

	camera.update();
	camera.look();

	// Determine visibility
	clear_visibility();
	set_visbuffer(1);
	open_view();
	walk_bsp_tree(n_gl_nodes - 1);

	if (render_wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glAlphaFunc(GL_GREATER, 0.8);
	glCullFace(GL_FRONT);
	for (int a = 0; a < wallrects.size(); a++)
	{
		if (vis_lines[wallrects[a]->line])
		{
			if (wallrects[a]->part == PART_TRANS)
				glEnable(GL_ALPHA_TEST);
			else
				glDisable(GL_ALPHA_TEST);

			set_light(wallrects[a]->col, wallrects[a]->light);
			wallrects[a]->draw();
		}
	}

	for (int a = 0; a < flatpolys.size(); a++)
	{
		if (vis_ssects[flatpolys[a]->ssector])
		{
			if (flatpolys[a]->part == PART_CEIL)
				glCullFace(GL_BACK);
			else
				glCullFace(GL_FRONT);

			set_light(flatpolys[a]->col, flatpolys[a]->light);
			flatpolys[a]->draw();
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 2d Stuff
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, draw_3d_area->allocation.width, draw_3d_area->allocation.height, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	draw_texture((draw_3d_area->allocation.width / 2) - 16,
					(draw_3d_area->allocation.height / 2) - 16, 32, 32, "_xhair", 4, col_3d_crosshair);
}
