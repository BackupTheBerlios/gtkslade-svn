
#include "main.h"
#include "struct_3d.h"
#include "camera.h"
#include "map.h"
#include "bsp.h"
#include "render.h"
#include "draw.h"

struct fademsg_t
{
	string	message;
	int		fade;
};

Camera camera;
vector<wallrect_t*> wallrects;
vector<flatpoly_t*> flatpolys;

vector<fademsg_t> messages_3d;

CVAR(Bool, render_fog, true, CVAR_SAVE)
CVAR(Bool, render_fullbright, false, CVAR_SAVE)
CVAR(Bool, render_wireframe, false, 0)
CVAR(Bool, render_hilight, true, CVAR_SAVE)

rgba_t col_3d_crosshair;
rgba_t col_3d_hilight;
rgba_t col_3d_hilight_line;

extern Map map;
//extern bool vis_lines[65535];
extern bool vis_ssects[65535];
extern int n_gl_nodes;
extern GtkWidget *draw_3d_area;

extern vector<line3d_t> lines_3d;
extern vector<ssect3d_t> ssects_3d;

extern wallrect_t* hl_wrect;
extern flatpoly_t* hl_fpoly;

wallrect_t::wallrect_t()
{
	wallrects.push_back(this);
	this->tex = NULL;
	this->sector = -1;
}

wallrect_t::~wallrect_t()
{
	for (int a = 0; a < wallrects.size(); a++)
	{
		if (wallrects[a] == this)
		{
			wallrects[a] = wallrects.back();
			wallrects.pop_back();
		}
	}
}

void wallrect_t::draw()
{
	if (tex) glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());
	glBegin(GL_TRIANGLE_FAN);

	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(true);

	glEnd();
}

void wallrect_t::draw_hilight()
{
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(4.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glColor4f(col_3d_hilight_line.fr(), col_3d_hilight_line.fg(),
				col_3d_hilight_line.fb(), col_3d_hilight_line.fa());

	glBegin(GL_QUADS);

	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(false);

	glEnd();

	glLineWidth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

flatpoly_t::flatpoly_t()
{
	flatpolys.push_back(this);
	this->parent_sector = -1;
	this->tex = NULL;
}

flatpoly_t::~flatpoly_t()
{
	/*
	for (int a = 0; a < flatpolys.size(); a++)
	{
		if (flatpolys[a] == this)
		{
			flatpolys[a] = flatpolys.back();
			flatpolys.pop_back();
		}
	}
	*/
}

void flatpoly_t::draw()
{
	if (tex) glBindTexture(GL_TEXTURE_2D, tex);
	glBegin(GL_TRIANGLE_FAN);

	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(true);

	glEnd();
}

void flatpoly_t::draw_hilight()
{
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(4.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glColor4f(col_3d_hilight_line.fr(), col_3d_hilight_line.fg(),
				col_3d_hilight_line.fb(), col_3d_hilight_line.fa());

	for (int a = 0; a < verts.size() - 1; a++)
	{
		if (lines[a])
		{
			glBegin(GL_LINES);
			verts[a].gl_draw(false);
			verts[a+1].gl_draw(false);
			glEnd();
		}
	}

	glLineWidth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

float plane_height(plane_t plane, float x, float y)
{
	return ((plane.a * x) + (plane.b * y) + plane.d) / plane.c;
}

void set_light(rgba_t col, int light)
{
	if (render_fullbright && col.r == col.g && col.r == col.b)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		return;
	}

	if (render_fog)
	{
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
		//glFogf(GL_FOG_DENSITY, 1.0f - light/500.0f);
		glFogf(GL_FOG_END, ((light/255.0f) * (light/255.0f) * 40.0f));
	}
	else
	{
		// If we have a non-coloured light, darken it a bit to closer resemble
		// the depth-fogged light level (still not very accurate, but oh well)
		if (col.r == col.g && col.r == col.b)
			glColor4f(col.fr()*col.fr(), col.fg()*col.fg(), col.fb()*col.fb(), col.fa());
		else
			glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
	}
}

rgba_t sector_col(int sector)
{
	BYTE l = map.sectors[sector]->light;
	return rgba_t(l, l, l, 255);
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
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
	}
	else
		glEnable(GL_TEXTURE_2D);

	if (render_fog && !render_fullbright)
		glEnable(GL_FOG);
	else
		glDisable(GL_FOG);

	glAlphaFunc(GL_GREATER, 0.8);
	glCullFace(GL_FRONT);
	for (int a = 0; a < map.n_lines; a++)
	{
		if (lines_3d[a].visible)
		{
			for (int b = 0; b < lines_3d[a].rects.size(); b++)
			{
				if (lines_3d[a].rects[b]->part == PART_TRANS)
					glEnable(GL_ALPHA_TEST);
				else
					glDisable(GL_ALPHA_TEST);

				set_light(sector_col(lines_3d[a].rects[b]->sector), lines_3d[a].rects[b]->light);
				lines_3d[a].rects[b]->draw();
			}
		}
	}

	glDisable(GL_ALPHA_TEST);

	vector<flatpoly_t*> hl_polys;
	if (hl_fpoly)
	{
		for (int a = 0; a < flatpolys.size(); a++)
		{
			if (flatpolys[a]->parent_sector == hl_fpoly->parent_sector &&
				flatpolys[a]->part == hl_fpoly->part)
				hl_polys.push_back(flatpolys[a]);
		}
	}

	for (int a = 0; a < ssects_3d.size(); a++)
	{
		if (ssects_3d[a].visible)
		{
			for (int b = 0; b < ssects_3d[a].flats.size(); b++)
			{
				if (ssects_3d[a].flats[b]->part == PART_CEIL)
					glCullFace(GL_BACK);
				else
					glCullFace(GL_FRONT);

				set_light(sector_col(ssects_3d[a].flats[b]->parent_sector), ssects_3d[a].flats[b]->light);
				ssects_3d[a].flats[b]->draw();
			}
		}
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (render_hilight)
	{
		if (hl_wrect)
			hl_wrect->draw_hilight();

		if (hl_fpoly)
		{
			hl_fpoly->draw_hilight();
			for (int a = 0; a < hl_polys.size(); a++)
				hl_polys[a]->draw_hilight();
		}
	}

	// 2d Stuff
	glDisable(GL_CULL_FACE);
	glCullFace(GL_NONE);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, draw_3d_area->allocation.width, draw_3d_area->allocation.height, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Crosshair
	draw_texture((draw_3d_area->allocation.width / 2) - 16,
					(draw_3d_area->allocation.height / 2) - 16, 32, 32, "_xhair", 4, col_3d_crosshair);

	// Messages
	for (int a = 0; a < messages_3d.size(); a++)
	{
		if (messages_3d[a].fade <= 0)
		{
			messages_3d.erase(messages_3d.begin() + a);
			a--;
			continue;
		}

		BYTE al = 255;
		if (messages_3d[a].fade < 255)
			al = messages_3d[a].fade;

		draw_text(0, a*10, rgba_t(255, 255, 255, al), 0, messages_3d[a].message.c_str());

		messages_3d[a].fade -= 10;
	}

	// Test info
	int bottom = draw_3d_area->allocation.height;
	draw_text(0, bottom - 10, rgba_t(255, 255, 255, 255), 0, "Wallrects: %d", wallrects.size());
	draw_text(0, bottom - 20, rgba_t(255, 255, 255, 255), 0, "Flatpolys: %d", flatpolys.size());
}

void add_3d_message(string message)
{
	if (messages_3d.size() > 6)
		messages_3d.erase(messages_3d.begin());

	fademsg_t msg;
	msg.message = message;
	msg.fade = 1024;
	messages_3d.push_back(msg);
}
