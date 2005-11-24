
#include "main.h"
#include "struct_3d.h"
#include "camera.h"
#include "map.h"
#include "bsp.h"
#include "render.h"
#include "draw.h"
#include "mathstuff.h"

struct fademsg_t
{
	string	message;
	int		fade;
};

Camera camera;
vector<wallrect_t*> wallrects;
vector<flatpoly_t*> flatpolys;
vector<thing3d_t*>	things_3d;

vector<fademsg_t> messages_3d;

CVAR(Bool, render_fog, true, CVAR_SAVE)
CVAR(Bool, render_fullbright, false, CVAR_SAVE)
CVAR(Bool, render_wireframe, false, 0)
CVAR(Bool, render_hilight, true, CVAR_SAVE)
CVAR(Bool, render_testinfo, false, CVAR_SAVE)
CVAR(Int, render_things, 2, CVAR_SAVE)

rgba_t col_3d_crosshair(100, 180, 255, 180);
rgba_t col_3d_hilight(0, 0, 0, 0);
rgba_t col_3d_hilight_line(0, 200, 255, 220);

// Test stuff
int wall_count = 0;
int flat_count = 0;
int thing_count = 0;

extern Map map;
extern int n_gl_nodes;
extern GtkWidget *draw_3d_area;

extern vector<line3d_t> lines_3d;
extern vector<ssect3d_t> ssects_3d;

extern wallrect_t* hl_wrect;
extern flatpoly_t* hl_fpoly;
extern thing3d_t* hl_thing;
extern sectinfo_t* sector_info;

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

	wall_count++;

	glEnd();
}

void wallrect_t::draw_hilight()
{
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(4.0f);

	// Outline
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	set_gl_colour(col_3d_hilight_line);

	glBegin(GL_QUADS);
	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(false);
	glEnd();

	// Fill
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	set_gl_colour(col_3d_hilight);

	glBegin(GL_QUADS);
	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(false);
	glEnd();

	glLineWidth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
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
	if (tex) glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());
	glBegin(GL_TRIANGLE_FAN);

	for (int a = 0; a < verts.size(); a++)
		verts[a].gl_draw(true);

	flat_count++;

	glEnd();
}

void flatpoly_t::draw_hilight()
{
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glLineWidth(4.0f);

	// Outline
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	set_gl_colour(col_3d_hilight_line);

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

	// Fill
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	set_gl_colour(col_3d_hilight);

	glBegin(GL_TRIANGLE_FAN);
	for (int a = 0; a < verts.size() - 1; a++)
		verts[a].gl_draw(false);
	glEnd();

	glLineWidth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

thing3d_t::thing3d_t()
{
	things_3d.push_back(this);
}

float plane_height(plane_t plane, float x, float y)
{
	return ((-plane.a * x) + (-plane.b * y) + plane.d) / plane.c;
}

void set_light(rgba_t col, int light)
{
	if ((render_fullbright || light == 255) && col.r == col.g && col.r == col.b)
	{
		glDisable(GL_FOG);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		return;
	}

	if (render_fog)
	{
		glEnable(GL_FOG);
		glFogf(GL_FOG_END, ((light/200.0f) * (light/200.0f) * 40.0f));
	}
	else
	{
		glDisable(GL_FOG);
	}

	// If we have a non-coloured light, darken it a bit to 
	// closer resemble the software renderer light level
	if (col.r == col.g && col.r == col.b)
		glColor4f(col.fr()*(col.fr()*1.3f), col.fg()*(col.fg()*1.3f), col.fb()*(col.fb()*1.3f), col.fa());
	else
		glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
}

rgba_t sector_col(int sector)
{
	BYTE l = map.sectors[sector]->light;
	return rgba_t(l, l, l, 255);
}

void render_3d_things(bool boxes)
{
	if (render_things == 0)
		return;

	if (!boxes)
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.8f);
	}
	else
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);
	}

	thing_count = 0;
	float shrink = 0.001f;

	for (int a = 0; a < things_3d.size(); a++)
	{
		int sector = things_3d[a]->parent_sector;

		if (sector == -1)
			continue;

		if (sector_info[sector].visible)
		{
			float x = map.things[a]->x * SCALE_3D;
			float y = map.things[a]->y * SCALE_3D;
			float r = (things_3d[a]->sprite->width / 2) * SCALE_3D;
			float h = things_3d[a]->sprite->height * SCALE_3D;
			float f;

			if (map.things[a]->ttype->hanging)
				f = plane_height(sector_info[sector].c_plane, x, y) - h;
			else
				f = plane_height(sector_info[sector].f_plane, x, y);

			if (map.hexen && map.things[a]->type != 9500 && map.things[a]->type != 9501)
				f += (map.things[a]->z * SCALE_3D);

			float x1 = x - camera.strafe.x * r;
			float y1 = y - camera.strafe.y * r;
			float x2 = x + camera.strafe.x * r;
			float y2 = y + camera.strafe.y * r;

			set_light(sector_col(sector), map.sectors[sector]->light);

			if (!boxes)
			{
				glCullFace(GL_FRONT);

				// Bind texture
				Texture* tex = things_3d[a]->sprite;
				glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

				// Draw sprite
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(x1, y1, f + h);
				glTexCoord2f(tex->t_x, 0.0f);
				glVertex3f(x2, y2, f + h);
				glTexCoord2f(tex->t_x, tex->t_y);
				glVertex3f(x2, y2, f);
				glTexCoord2f(0.0f, tex->t_y);
				glVertex3f(x1, y1, f);
				glEnd();

				// Draw selection rectangle if selected
				if (hl_thing == things_3d[a])
				{
					glDisable(GL_DEPTH_TEST);
					glDisable(GL_TEXTURE_2D);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					glLineWidth(4.0f);
					set_gl_colour(col_3d_hilight_line);

					glBegin(GL_QUADS);
					glVertex3f(x1, y1, f + h);
					glVertex3f(x2, y2, f + h);
					glVertex3f(x2, y2, f);
					glVertex3f(x1, y1, f);
					glEnd();

					glEnable(GL_TEXTURE_2D);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_DEPTH_TEST);
				}

				continue;
			}

			// Draw floor box
			if (render_things >= 2)
			{
				x1 = (map.things[a]->x - map.things[a]->ttype->radius) * SCALE_3D;
				y1 = (map.things[a]->y - map.things[a]->ttype->radius) * SCALE_3D;
				x2 = (map.things[a]->x + map.things[a]->ttype->radius) * SCALE_3D;
				y2 = (map.things[a]->y + map.things[a]->ttype->radius) * SCALE_3D;

				rgba_t col = map.things[a]->ttype->colour;
				glColor4f(col.fr(), col.fg(), col.fb(), 0.8f);

				// Outline
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glLineWidth(4.0f);
				glBegin(GL_QUADS);
				glVertex3f(x1, y1, f + shrink);
				glVertex3f(x2, y1, f + shrink);
				glVertex3f(x2, y2, f + shrink);
				glVertex3f(x1, y2, f + shrink);
				glEnd();

				// Direction
				if (map.things[a]->ttype->show_angle)
				{
					glBegin(GL_LINES);
					glLineWidth(10.0f);
					glVertex3f(x, y, f + shrink);

					glLineWidth(4.0f);
					if (map.things[a]->angle == 0)
						glVertex3f(x2, y, f + shrink);
					if (map.things[a]->angle == 45)
						glVertex3f(x2, y2, f + shrink);
					if (map.things[a]->angle == 90)
						glVertex3f(x, y2, f + shrink);
					if (map.things[a]->angle == 135)
						glVertex3f(x1, y2, f + shrink);
					if (map.things[a]->angle == 180)
						glVertex3f(x1, y, f + shrink);
					if (map.things[a]->angle == 225)
						glVertex3f(x1, y1, f + shrink);
					if (map.things[a]->angle == 270)
						glVertex3f(x, y1, f + shrink);
					if (map.things[a]->angle == 315)
						glVertex3f(x2, y1, f + shrink);
					else
						glVertex3f(x, y, f + shrink);

					glEnd();
				}

				// Fill
				glColor4f(col.fr(), col.fg(), col.fb(), 0.3f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glBegin(GL_QUADS);
				glVertex3f(x1, y1, f + shrink);
				glVertex3f(x2, y1, f + shrink);
				glVertex3f(x2, y2, f + shrink);
				glVertex3f(x1, y2, f + shrink);
				glEnd();
			}

			// Draw full box
			if (render_things >= 3)
			{
				float t = f + h;
				rgba_t col = map.things[a]->ttype->colour;

				// OUTLINES
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glColor4f(col.fr(), col.fg(), col.fb(), 0.8f);

				// Top
				glBegin(GL_QUADS);
				glVertex3f(x1, y1, t - shrink);
				glVertex3f(x2, y1, t - shrink);
				glVertex3f(x2, y2, t - shrink);
				glVertex3f(x1, y2, t - shrink);
				glEnd();

				// Left
				glBegin(GL_QUADS);
				glVertex3f(x1, y1, t - shrink);
				glVertex3f(x1, y2, t - shrink);
				glVertex3f(x1, y2, f + shrink);
				glVertex3f(x1, y1, f + shrink);
				glEnd();

				// Right
				glBegin(GL_QUADS);
				glVertex3f(x2, y2, t - shrink);
				glVertex3f(x2, y1, t - shrink);
				glVertex3f(x2, y1, f + shrink);
				glVertex3f(x2, y2, f + shrink);
				glEnd();

				// Front
				glBegin(GL_QUADS);
				glVertex3f(x1, y2, t - shrink);
				glVertex3f(x2, y2, t - shrink);
				glVertex3f(x2, y2, f + shrink);
				glVertex3f(x1, y2, f + shrink);
				glEnd();

				// Back
				glBegin(GL_QUADS);
				glVertex3f(x2, y1, t - shrink);
				glVertex3f(x1, y1, t - shrink);
				glVertex3f(x1, y1, f + shrink);
				glVertex3f(x2, y1, f + shrink);
				glEnd();


				// FILLS
				glColor4f(col.fr(), col.fg(), col.fb(), 0.3f);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				// Top
				glBegin(GL_QUADS);
				glVertex3f(x1, y1, t - shrink);
				glVertex3f(x2, y1, t - shrink);
				glVertex3f(x2, y2, t - shrink);
				glVertex3f(x1, y2, t - shrink);
				glEnd();

				// Left
				glBegin(GL_QUADS);
				glVertex3f(x1, y1, t - shrink);
				glVertex3f(x1, y2, t - shrink);
				glVertex3f(x1, y2, f + shrink);
				glVertex3f(x1, y1, f + shrink);
				glEnd();

				// Right
				glBegin(GL_QUADS);
				glVertex3f(x2, y2, t - shrink);
				glVertex3f(x2, y1, t - shrink);
				glVertex3f(x2, y1, f + shrink);
				glVertex3f(x2, y2, f + shrink);
				glEnd();

				// Front
				glBegin(GL_QUADS);
				glVertex3f(x1, y2, t - shrink);
				glVertex3f(x2, y2, t - shrink);
				glVertex3f(x2, y2, f + shrink);
				glVertex3f(x1, y2, f + shrink);
				glEnd();

				// Back
				glBegin(GL_QUADS);
				glVertex3f(x2, y1, t - shrink);
				glVertex3f(x1, y1, t - shrink);
				glVertex3f(x1, y1, f + shrink);
				glVertex3f(x2, y1, f + shrink);
				glEnd();
			}

			thing_count++;
		}
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void render_3d_view()
{
	wall_count = 0;
	flat_count = 0;

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

	// Draw non-transparent walls
	vector<wallrect_t*> trans_walls;
	glCullFace(GL_FRONT);
	for (int a = 0; a < map.n_lines; a++)
	{
		if (lines_3d[a].visible)
		{
			for (int b = 0; b < lines_3d[a].rects.size(); b++)
			{
				if (lines_3d[a].rects[b]->part == PART_TRANS)
					trans_walls.push_back(lines_3d[a].rects[b]);
				else
				{
					rgba_t col = sector_col(lines_3d[a].rects[b]->sector);

					if (lines_3d[a].rects[b]->flags & WRECT_SKY)
						set_light(col, 255);
					else
						set_light(col, map.sectors[lines_3d[a].rects[b]->sector]->light);

					lines_3d[a].rects[b]->draw();
				}
			}
		}
	}

	glDisable(GL_ALPHA_TEST);

	// Get hilighted flats (if any)
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

	// Draw flats
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

				sector_info[ssects_3d[a].flats[b]->parent_sector].visible = true;

				if (ssects_3d[a].flats[b]->flags & FPOLY_SKY)
					set_light(sector_col(ssects_3d[a].flats[b]->parent_sector), 255);
				else
					set_light(sector_col(ssects_3d[a].flats[b]->parent_sector), map.sectors[ssects_3d[a].flats[b]->parent_sector]->light);
				
				ssects_3d[a].flats[b]->draw();
			}
		}
	}

	// Draw transparent walls
	glCullFace(GL_FRONT);
	for (int a = 0; a < trans_walls.size(); a++)
	{
		rgba_t col = sector_col(trans_walls[a]->sector);
		col.a = lines_3d[trans_walls[a]->line].alpha;

		if (col.a == 255)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.8f);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
			glDepthMask(GL_FALSE);
		}

		set_light(col, map.sectors[trans_walls[a]->sector]->light);
		trans_walls[a]->draw();
	}

	glEnable(GL_ALPHA_TEST);
	glDepthMask(GL_TRUE);
	render_3d_things(false);
	render_3d_things(true);

	glDisable(GL_ALPHA_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw hilighted wallrect/flatpoly
	if (render_hilight)
	{
		if (hl_wrect)
			hl_wrect->draw_hilight();

		if (hl_fpoly)
		{
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

		messages_3d[a].fade -= 7;
	}

	// Test info
	if (render_testinfo)
	{
		int bottom = draw_3d_area->allocation.height;
		draw_text(0, bottom - 10, rgba_t(255, 255, 255, 255), 0, "Wallrects: %d (%d)", wallrects.size(), wall_count);
		draw_text(0, bottom - 20, rgba_t(255, 255, 255, 255), 0, "Flatpolys: %d (%d)", flatpolys.size(), flat_count);
		draw_text(0, bottom - 32, rgba_t(255, 255, 255, 255), 0, "Things: %d (%d)", things_3d.size(), thing_count);
	}
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
