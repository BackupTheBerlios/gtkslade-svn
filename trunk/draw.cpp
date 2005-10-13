// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004                   >>
// << ------------------------------------- >>
// << draw.cpp - Drawing functions          >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"
#include "map.h"
#include "draw.h"
#include "edit.h"
#include "edit_move.h"
#include "textures.h"

// Variables ------------------------------ >>
rgba_t	col_hilight(255, 255, 0, 160, 1);
rgba_t	col_selection(0, 140, 220, 255, 1);
rgba_t	col_moving(255, 0, 0, 255, 1);
rgba_t	col_tagged(0, 255, 0, 255, 1);
rgba_t	col_grid(0, 0, 255, 80, 0);
rgba_t	col_64grid(200, 200, 200, 65, 0);
rgba_t	col_vertex(0, 180, 0, 255, 1);
rgba_t	col_background(0, 0, 0, 255, 0);
rgba_t	col_selbox(0, 140, 220, 100, 1);
rgba_t	col_selbox_line(100, 180, 220, 200, 1);
rgba_t	col_linedraw(0, 200, 0, 255, 0);

CVAR(Float, line_size, 1.5, CVAR_SAVE)
CVAR(Bool, thing_sprites, false, CVAR_SAVE)
CVAR(Bool, grid_dashed, false, CVAR_SAVE)
CVAR(Bool, line_aa, true, CVAR_SAVE)

/*
CVAR(Float, hilight_size, 3.0, CVAR_SAVE)
CVAR(Float, selection_size, 5.0f, CVAR_SAVE)
CVAR(Float, moving_size, 5.0f, CVAR_SAVE)
*/

float hilight_size = 3.0f;
float selection_size = 5.0f;
float moving_size = 5.0f;

GLuint map_list;
GLuint grid_list;

// External Variables --------------------- >>
extern Map map;
extern int vid_width, vid_height, hilight_item, edit_mode, gridsize;
extern float zoom;
extern vector<int> selected_items;
extern pointlist_t ldraw_points;
extern rect_t sel_box;
extern point2_t mouse;
extern bool line_draw;
extern movelist_t move_list;

// -------------------------------------------------------------
// BASIC DRAWING STUFF -----------------------------------------
// -------------------------------------------------------------



// set_colour: Sets the current OpenGL colour
// ------------------------------------------ >>
void set_colour(rgba_t col)
{
	glColor4f(col.fr(), col.fg(), col.fb(), col.fa());
}

// draw_point: Draws a point of given size at (x, y)
// ---------------------------------------------- >>
void draw_point(int x, int y, int size, rgba_t col)
{
	glDisable(GL_TEXTURE_2D);

	set_colour(col);
	col.set_blend();
	glPointSize((float)size);

	glBegin(GL_POINTS);
		glVertex2d(x, y);
	glEnd();
}

// draw_line: Draws a line from a rect's top left to it's bottom right
// ---------------------------------------------------------------- >>
void draw_line(rect_t rect, rgba_t col, bool aa)
{
	glDisable(GL_TEXTURE_2D);

	if (aa)
		glEnable(GL_LINE_SMOOTH);
	else
		glDisable(GL_LINE_SMOOTH);

	set_colour(col);
	col.set_blend();
	
	glBegin(GL_LINES);
		glVertex2d(rect.tl.x, rect.tl.y);
		glVertex2d(rect.br.x, rect.br.y);
	glEnd();
}

// draw_rect: Draws a rectangle
// ------------------------- >>
void draw_rect(rect_t rect, rgba_t col, bool fill)
{
	glDisable(GL_TEXTURE_2D);

	set_colour(col);
	col.set_blend();

	if (fill)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBegin(GL_QUADS);
	glVertex2d(rect.x1(), rect.y1());
	glVertex2d(rect.x2(), rect.y1());
	glVertex2d(rect.x2(), rect.y2());
	glVertex2d(rect.x1(), rect.y2());
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// draw_texture: Draws a texture at x, y, resized to the given width & height
// not resized if width or height = -1
// ----------------------------------------------------------------------- >>
void draw_texture(int x, int y, int width, int height, string texname, int textype, rgba_t col)
{
	Texture* tex = get_texture(texname, textype);

	if (!tex)
		return;

	if (width == -1)
		width = tex->width;

	if (height == -1)
		height = tex->height;

	set_colour(col);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(x, y);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(x + width, y);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(x + width, y + height);
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(x, y + height);
	glEnd();
}

// draw_texture_scale: Draws a texture to fit within a certain area, scaled correctly
// ------------------------------------------------------------------------------- >>
void draw_texture_scale(rect_t rect, string texname, int textype, rgba_t col, float scalef)
{
	Texture* tex = get_texture(texname, textype);

	if (!tex)
		return;

	int width = rect.width();
	int height = rect.height();
	int texwidth = tex->width * scalef;
	int texheight = tex->height * scalef;

	int mid_x = rect.x1() + (width / 2);
	int mid_y = rect.y1() + (height / 2);

	int nx1 = rect.x1();
	int nx2 = rect.x1() + width;
	int ny1 = rect.y1();
	int ny2 = rect.y1() + height;

	if (texwidth > width)
	{
		float multiplier = (float)rect.width() / (float)texwidth;
		
		ny1 = mid_y - (float(texheight / 2.0f) * multiplier);
		ny2 = mid_y + (float(texheight / 2.0f) * multiplier);
		height = texheight * multiplier;
	}
	else
	{
		nx1 = mid_x - (texwidth / 2);
		nx2 = mid_x + (texwidth / 2);
	}

	if (texheight > height)
	{
		float multiplier = (float)rect.height() / (float)texheight;
		
		nx1 = mid_x - (float(texwidth / 2.0f) * multiplier);
		nx2 = mid_x + (float(texwidth / 2.0f) * multiplier);
	}
	else
	{
		ny1 = mid_y - (texheight / 2);
		ny2 = mid_y + (texheight / 2);
	}
	
	set_colour(col);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex->get_gl_id());
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);	glVertex2d(nx1, ny1);
		glTexCoord2f(1.0f, 0.0f);	glVertex2d(nx2, ny1);
		glTexCoord2f(1.0f, 1.0f);	glVertex2d(nx2, ny2);
		glTexCoord2f(0.0f, 1.0f);	glVertex2d(nx1, ny2);
	glEnd();
}


// -------------------------------------------------------------
// MAP DRAWING STUFF -------------------------------------------
// -------------------------------------------------------------



// draw_basic_lines: Draws lines without selections/hilights or side markers
// ---------------------------------------------------------------------- >>
void draw_basic_lines()
{
	rgba_t colour(0, 0, 0, 255);

	for (DWORD l = 0; l < map.n_lines; l++)
	{
		rect_t rect = map.l_getsrect(l);

		if (!((rect.x1() < 0 && rect.x2() < 0) ||
			(rect.x1() > vid_width && rect.x2() > vid_width) ||
			(rect.y1() < 0 && rect.y2() < 0) ||
			(rect.y1() > vid_height && rect.y2() > vid_height)))
		{
			glLineWidth(line_size);
			get_line_colour(l, &colour);
			draw_line(rect, colour, line_aa);
		}
	}
}

// draw_lines: Draws lines with selections, hilights and side_markers
// --------------------------------------------------------------- >>
void draw_lines()
{
	rgba_t colour;

	draw_basic_lines();

	for (DWORD l = 0; l < map.n_lines; l++)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(line_size);

		rect_t rect = map.l_getsrect(l);
		rect_t rect2;

		if (!((rect.x1() < 0 && rect.x2() < 0) ||
			(rect.x1() > vid_width && rect.x2() > vid_width) ||
			(rect.y1() < 0 && rect.y2() < 0) ||
			(rect.y1() > vid_height && rect.y2() > vid_height)))
		{
			point2_t mid = midpoint(rect.tl, rect.br);
			point2_t vec(rect.x2() - rect.x1(), rect.y2() - rect.y1());
			int x = -vec.y / 8;
			int y = vec.x / 8;
			rect2.set(mid.x, mid.y, mid.x + x, mid.y + y);

			get_line_colour(l, &colour);
			draw_line(rect2, colour, line_aa);

			if (MOVING(map.lines[l]->vertex1) && MOVING(map.lines[l]->vertex2))
			{
				glLineWidth(moving_size * line_size);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				draw_line(rect, col_moving, true);
				draw_line(rect2, col_moving, true);
			}
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw selected lines if nothing is moving
	if (move_list.n_items == 0)
	{
		glLineWidth(selection_size * line_size);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		for (DWORD a = 0; a < selected_items.size(); a++)
		{
			DWORD l = selected_items[a];

			rect_t rect = map.l_getsrect(l);
			rect_t rect2;

			if (!((rect.x1() < 0 && rect.x2() < 0) ||
				(rect.x1() > vid_width && rect.x2() > vid_width) ||
				(rect.y1() < 0 && rect.y2() < 0) ||
				(rect.y1() > vid_height && rect.y2() > vid_height)))
			{
				point2_t mid = midpoint(rect.tl, rect.br);
				point2_t vec(rect.x2() - rect.x1(), rect.y2() - rect.y1());
				int x = -vec.y / 8;
				int y = vec.x / 8;
				rect2.set(mid.x, mid.y, mid.x + x, mid.y + y);

				draw_line(rect, col_selection, line_aa);
				draw_line(rect2, col_selection, line_aa);
			}
		}
	}

	glLineWidth(line_size);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// draw_vertices: Draws map vertices
// ------------------------------ >>
void draw_vertices()
{
	int size = 6;

	glEnable(GL_POINT_SMOOTH);

	for(DWORD v = 0; v < map.n_verts; v++)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		point2_t vert;
		vert.set(map.v_getspoint(v));

		// Deafults
		size = 6;

		if (vert.x >= 0 && vert.x <= vid_width && vert.y >= 0 && vert.y <= vid_height)
			draw_point(vert.x, vert.y, size, col_vertex);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// Draw moving vertices
	for (DWORD a = 0; a < move_list.n_items; a++)
	{
		DWORD v = move_list.items[a];
		point2_t vert(map.v_getspoint(v));
		draw_point(vert.x, vert.y, 10, col_moving);
	}

	// Draw selected vertices
	for (DWORD a = 0; a < selected_items.size(); a++)
	{
		DWORD v = selected_items[a];

		if (!move_list.exists(v))
		{
			point2_t vert(map.v_getspoint(v));
			draw_point(vert.x, vert.y, 10, col_selection);
		}
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// draw_sectors: Draws the map sectors (hilight/selections)
// ----------------------------------------------------- >>
void draw_sectors()
{
	rgba_t colour;

	for(DWORD l = 0; l < map.n_lines; l++)
	{
		short sector1 = map.l_getsector1(l);
		short sector2 = map.l_getsector2(l);

		rect_t rect = map.l_getsrect(l);

		if (move_list.exists(map.lines[l]->vertex1)
			&&move_list.exists(map.lines[l]->vertex2))
		{
			glLineWidth(moving_size * line_size);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			draw_line(rect, col_moving, true);
		}
		else
		{
			//if (selected_items.exists(sector1)
			//	|| selected_items.exists(sector2))
			if (vector_exists(selected_items, sector1)
				|| vector_exists(selected_items, sector2))
			{
				glLineWidth(selection_size * line_size);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				draw_line(rect, col_selection, true);
			}
		}
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// draw_things: Draws the map things
// ------------------------------ >>
void draw_things()
{
	rgba_t colour;

	for (DWORD t = 0; t < map.n_things; t++)
	{
		// Get thing type definition (for colour, radius and sprite)
		thing_type_t* ttype = map.things[t]->ttype;
		if (!ttype) ttype = get_thing_type(-1);

		// Setup the colour
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		colour.set(ttype->colour);

		// Setup the radius
		int r = (ttype->radius * zoom) / MAJOR_UNIT;
		if (r < 0) r = 8;

		// Draw thing rect
		rect_t t_rect(s_x(map.things[t]->x), s_y(-map.things[t]->y), r*2, r*2, RECT_CENTER);
		draw_texture(t_rect.x1(), t_rect.y1(), t_rect.width(), t_rect.height(), "_thing", 0, colour);

		// Draw the angle (if needed)
		if (map.things[t]->ttype->show_angle)
		{
			point2_t p(s_x(map.things[t]->x), s_y(-map.things[t]->y));
			int x2, y2;

			// east
			if (map.things[t]->angle == 0)			{ x2 = p.x + r; y2 = p.y; }
			// northeast
			else if (map.things[t]->angle == 45)	{ x2 = p.x + (r*0.75); y2 = p.y - (r*0.75); }
			// north
			else if (map.things[t]->angle == 90)	{ x2 = p.x; y2 = p.y - r; }
			// northwest
			else if (map.things[t]->angle == 135)	{ x2 = p.x - (r*0.75); y2 = p.y - (r*0.75); }
			// west
			else if (map.things[t]->angle == 180)	{ x2 = p.x - r; y2 = p.y; }
			// southwest
			else if (map.things[t]->angle == 225)	{ x2 = p.x - (r*0.75); y2 = p.y + (r*0.75); }
			// south
			else if (map.things[t]->angle == 270)	{ x2 = p.x; y2 = p.y + r; }
			// southeast
			else if (map.things[t]->angle == 315)	{ x2 = p.x + (r*0.75); y2 = p.y + (r*0.75); }
			// Invalid angle
			else	{ x2 = p.x; y2 = p.y; }

			//if (thing_sprites)
			//{
			//	draw_point(p.x, p.y, 8 * zoom / MAJOR_UNIT, COL_WHITE);
			//	draw_line(rect_t(p.x, p.y, x2, y2), rgba_t(255, 255, 255, 200), line_aa);
			//}
			//else

			glLineWidth(2.0f);
			draw_line(rect_t(p.x, p.y, x2, y2), rgba_t(0, 0, 0, 200), line_aa);
			glLineWidth(1.0f);
		}

		// Draw moving hilight
		if (move_list.exists(t))
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			r += 4;
			draw_rect(rect_t(s_x(map.things[t]->x), s_y(-map.things[t]->y), r*2, r*2, RECT_CENTER), col_moving, true);
		}
		else if (vector_exists(selected_items, t)) // Draw selected hilight
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			r += 4;
			draw_rect(rect_t(s_x(map.things[t]->x), s_y(-map.things[t]->y), r*2, r*2, RECT_CENTER), col_selection, true);
		}
	}
}

// draw_hilight: Draws the currently hilighted item
// --------------------------------------------- >>
void draw_hilight()
{
	if (edit_mode == 0)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		draw_point(s_x(map.verts[hilight_item]->x), s_y(-map.verts[hilight_item]->y), 10, col_hilight);
	}

	if (edit_mode == 1)
	{
		rect_t rect = map.l_getsrect(hilight_item);
		rect_t rect2;

		int xm = rect.x1() + ((rect.x2() - rect.x1()) / 2);
		int ym = rect.y1() + ((rect.y2() - rect.y1()) / 2);
		int xm2, ym2;

		xm2 = xm - ((rect.y2() - rect.y1()) / 8);
		ym2 = ym + ((rect.x2() - rect.x1()) / 8);

		rect2.set(xm, ym, xm2, ym2);

		glLineWidth(hilight_size * line_size);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		draw_line(rect, col_hilight, true);
		draw_line(rect2, col_hilight, true);
	}

	if (edit_mode == 2)
	{
		for(DWORD l = 0; l < map.n_lines; l++)
		{
			short sector1 = map.l_getsector1(l);
			short sector2 = map.l_getsector2(l);

			rect_t rect = map.l_getsrect(l);

			if (hilight_item == sector1
				|| hilight_item == sector2
				&& hilight_item != -1)
			{
				glLineWidth(hilight_size * line_size);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				draw_line(rect, col_hilight, true);
			}
		}
	}

	if (edit_mode == 3)
	{
		int r = ((map.things[hilight_item]->ttype->radius * zoom) / MAJOR_UNIT);
		if (r < 0) r = 14;

		rgba_t col(col_hilight.r, col_hilight.g, col_hilight.b, 150, col_hilight.blend);
		draw_rect(rect_t(s_x(map.things[hilight_item]->x), s_y(-map.things[hilight_item]->y), r*2, r*2, RECT_CENTER), col, true);
	}
}

// draw_tagged: Draws any tagged items associated with the current hilighted item
// --------------------------------------------------------------------------- >>
void draw_tagged()
{
	// Lines mode (check sectors)
	if (edit_mode == 1)
	{
		int tag = map.lines[hilight_item]->sector_tag;

		if (tag == 0)
			return;

		for(DWORD l = 0; l < map.n_lines; l++)
		{
			short sector1 = map.l_getsector1(l);
			short sector2 = map.l_getsector2(l);
			bool tagged = false;

			rect_t rect = map.l_getsrect(l);

			if (sector1 != -1)
			{
				if (map.sectors[sector1]->tag == tag)
					tagged = true;
			}

			if (sector2 != -1)
			{
				if (map.sectors[sector2]->tag == tag)
					tagged = true;
			}

			if (tagged)
			{
				glLineWidth(3.0f * line_size);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				draw_line(rect, col_tagged, true);
			}
		}
	}

	// Sectors mode (check lines)
	if (edit_mode == 2)
	{
		int tag = map.sectors[hilight_item]->tag;

		if (tag == 0)
			return;

		for(DWORD l = 0; l < map.n_lines; l++)
		{
			if (map.lines[l]->sector_tag == tag)
			{
				rect_t rect = map.l_getsrect(l);
				rect_t rect2;

				int xm = rect.x1() + ((rect.x2() - rect.x1()) / 2);
				int ym = rect.y1() + ((rect.y2() - rect.y1()) / 2);
				int xm2, ym2;

				xm2 = xm - ((rect.y2() - rect.y1()) / 8);
				ym2 = ym + ((rect.x2() - rect.x1()) / 8);

				rect2.set(xm, ym, xm2, ym2);

				glLineWidth(3.0f * line_size);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				draw_line(rect, col_tagged, true);
				draw_line(rect2, col_tagged, true);
			}
		}
	}
}

// draw_drawlines: Draws dashed lines for line drawing
// ------------------------------------------------ >>
void draw_drawlines()
{
	glEnable(GL_POINT_SMOOTH);

	if (ldraw_points.n_points == 0)
	{
		draw_point(s_x(snap_to_grid(m_x(mouse.x))), s_y(snap_to_grid(m_y(mouse.y))), 8, col_linedraw);
		return;
	}
	else
	{
		glLineWidth(line_size);
		glLineStipple(4, 0xBBBB);
		glEnable(GL_LINE_STIPPLE);
		point2_t p1, p2;

		for (int p = 1; p < ldraw_points.n_points; p++)
		{
			p1.set(s_x(ldraw_points.points[p - 1].x), s_y(-ldraw_points.points[p - 1].y));
			p2.set(s_x(ldraw_points.points[p].x), s_y(-ldraw_points.points[p].y));

			draw_line(rect_t(p1, p2), col_linedraw, true);
			draw_point(p1.x, p1.y, 6, col_linedraw);
			//draw_text(midpoint(p1, p2).x, midpoint(p1, p2).y - 4, col_linedraw, 1, "%d", (int)distance(m_x(p1.x), m_y(p1.y), m_x(p2.x), m_y(p2.y)));
		}

		if (ldraw_points.n_points == 1)
			p2.set(s_x(ldraw_points.points[0].x), s_y(-ldraw_points.points[0].y));

		if (sel_box.x1() == -1)
		{
			p1.set(snap_to_grid(m_x(mouse.x)), snap_to_grid(m_y(mouse.y)));
			draw_line(rect_t(s_p(p1), p2), col_linedraw, true);
			draw_point(s_x(p1.x), s_y(p1.y), 8, col_linedraw);
			draw_point(p2.x, p2.y, 6, col_linedraw);
			//draw_text(midpoint(s_p(p1), p2).x, midpoint(s_p(p1), p2).y, col_linedraw, 1, "%d", (int)distance(p1.x, p1.y, m_x(p2.x), m_y(p2.y)));
		}

		glDisable(GL_LINE_STIPPLE);
	}
}

// update_grid: Updates the grid display list
// --------------------------------------- >>
void update_grid()
{
	int grid_hidelevel;

	glDeleteLists(grid_list, 1);
	grid_list = glGenLists(1);
	glNewList(grid_list, GL_COMPILE);

	glDisable(GL_LINE_SMOOTH); // AA is unnecessary (grid lines are not diagonal)
	glLineWidth(1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	grid_hidelevel = 64;

	int start_x = m_x(0);
	int end_x = m_x(vid_width);
	int start_y = m_y(0);
	int end_y = m_y(vid_height);

	if (grid_dashed)
	{
		glLineStipple(2, 0xAAAA);
		glEnable(GL_LINE_STIPPLE);
	}

	// If the grid should be drawn (not to small)
	if (gridsize >= (grid_hidelevel / zoom) && gridsize > 0)
	{
		for (int x = start_x; x < end_x; x++)
		{
			if (x % gridsize == 0)
				draw_line(rect_t(s_x(x), 0, s_x(x), vid_height), col_grid, false);
		}

		for (int y = start_y; y < end_y; y++)
		{
			if (y % gridsize == 0)
				draw_line(rect_t(0, s_y(y), vid_width, s_y(y)), col_grid, false);
		}
	}

	glDisable(GL_LINE_STIPPLE);

	// Don't draw 64x64 grid if it'll be too small
	if (64 < (grid_hidelevel / zoom))
	{
		glEndList();
		return;
	}

	// Don't draw 64x64 grid if the actual grid size is >=
	if (gridsize >= 64)
	{
		glEndList();
		return;
	}

	for (int x = start_x; x < end_x; x++)
	{
		if (x % 64 == 0)
			draw_line(rect_t(s_x(x), 0, s_x(x), vid_height), col_64grid, false);
	}

	for (int y = start_y; y < end_y; y++)
	{
		if (y % 64 == 0)
			draw_line(rect_t(0, s_y(y), vid_width, s_y(y)), col_64grid, false);
	}

	glEndList();
}

// update_map: Updates the map display list
// ------------------------------------- >>
void update_map()
{
	glDeleteLists(map_list, 1);
	map_list = glGenLists(1);
	glNewList(map_list, GL_COMPILE);

	// Draw map objects
	if (edit_mode == 0)	// Vertices
	{
		draw_basic_lines();
		draw_vertices();
	}

	if (edit_mode == 1)	// Lines
		draw_lines();

	if (edit_mode == 2) // Sectors
	{
		draw_basic_lines();
		draw_sectors();
	}

	if (edit_mode == 3) // Things
	{
		draw_basic_lines();
		draw_things();
	}

	//draw_texture(64, 64, 64, 64, "_thing", 0, rgba_t(255, 0, 0, 255, 0));

	glEndList();
}

// draw_map: Draws the map
// -------------------- >>
void draw_map()
{
	glClearColor(col_background.fr(), col_background.fg(), col_background.fb(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (map.opened)
	{
		glCallList(grid_list);
		glCallList(map_list);

		if (line_draw)
			draw_drawlines();
		else
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			glLineWidth(2.0f);
			draw_rect(sel_box, col_selbox, true);
			draw_rect(sel_box, col_selbox_line, false);

			if (hilight_item != -1)
			{
				draw_hilight();
				draw_tagged();
			}

			if (sel_box.x1() != -1)
			{
				glLineWidth(2.0f);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				draw_rect(sel_box, col_selbox, true);
				draw_rect(sel_box, col_selbox_line, false);
			}
		}
	}
}
