
#include "main.h"
#include "map.h"
#include "checks.h"
#include "edit_move.h"
#include "undoredo.h"
#include "editor_window.h"
#include "info_bar.h"
#include "struct_3d.h"
#include "mathstuff.h"
#include "camera.h"
#include "render.h"
#include "edit_misc.h"

// VARIABLES ----------------------------- >>
float	zoom = 16;			// Zoom level (pixels per 32 map units)
int		xoff = 0;			// View x offset
int		yoff = 0;			// View y offset
int		gridsize = 32;		// Grid size
int		edit_mode = 1;		// Edit mode: 0=vertices, 1=lines, 2=sectors, 3=things
int		hilight_item = -1;	// The currently hilighted item (-1 for none)

bool lock_hilight = false;	// Wether to 'lock' hilighting


int map_changed = 3;	// 0 = no change
						// 1 = changed, no node rebuild or struct rebuild needed
						// 2 = changed, no node rebuild needed
						// 3 = changed, node rebuild needed

bool browser = false;	// Is the texture browser active?
//bool mix_tex = false;	// Mix textures & flats (zdoom)?

thing_t	last_thing;

movelist_t	move_list;
vector<int>	select_order;

vector<int>	selected_items;

CVAR(Bool, edit_snap_grid, true, CVAR_SAVE)

// Colours
rgba_t	col_line_solid(200, 200, 200, 255);
rgba_t	col_line_2s(110, 120, 130, 255);
rgba_t	col_line_monster(0, 120, 0, 255);
rgba_t	col_line_nofirst(255, 0, 0, 255);
rgba_t	col_line_special(100, 115, 180, 255);

extern int vid_width, vid_height;
extern Map map;
extern point2_t mouse, down_pos;
extern Camera camera;

// increase_grid: Increases the grid size
// ----------------------------------- >>
void increase_grid()
{
	gridsize *= 2;

	if (gridsize > 1024)
		gridsize = 1024;
}

// decrease_grid: Decreases the grid size
// ----------------------------------- >>
void decrease_grid()
{
	gridsize /= 2;

	if (gridsize < 1)
		gridsize = 1;
}

// snap_to_grid: Finds the nearest grid line to a point
// ------------------------------------------------- >>
int snap_to_grid(double pos)
{
	if (!edit_snap_grid)
		return pos;

	int upper, lower;

	for (int i = pos; i >= (pos - gridsize); i--)
	{
		if ((i % gridsize) == 0)
		{
			lower = i;
			break;
		}
	}

	for (int i = pos; i < (pos + gridsize); i++)
	{
		if ((i % gridsize) == 0)
		{
			upper = i;
			break;
		}
	}

	double mid = lower + ((upper - lower) / 2.0);

	if (pos > mid)
		return upper;
	else
		return lower;

	return (int)pos;
}

// snap_to_grid_custom: Finds the nearest specified grid line to a point
// ------------------------------------------------------------------ >>
int snap_to_grid_custom(int pos, int grid)
{
	short upper, lower;

	for (int i = pos; i >= (pos - grid); i--)
	{
		if ((i % grid) == 0)
		{
			lower = i;
			break;
		}
	}

	for (int i = pos; i < (pos + grid); i++)
	{
		if ((i % grid) == 0)
		{
			upper = i;
			break;
		}
	}

	if ((upper - pos) < (pos - lower))
		return upper;
	else
		return lower;

	return pos;
}

// s_x: Returns the actual screen position of an x coordinate on the map
// ------------------------------------------------------------------ >>
double s_x(double x)
{
	return ((double)vid_width / 2.0) + (((double)xoff * zoom) + (x * zoom) / MAJOR_UNIT);
}

// s_y: Returns the actual screen position of a y coordinate on the map
// ----------------------------------------------------------------- >>
double s_y(double y)
{
	return ((double)vid_height / 2) + (((double)yoff * zoom) + (y * zoom) / MAJOR_UNIT);
}

point2_t s_p(point2_t point)
{
	return point2_t(s_x(point.x), s_y(point.y));
}

// m_x: Returns the map coordiate of an x position on the screen
// ---------------------------------------------------------- >>
double m_x(double x)
{
	return (((float)x / (float)zoom) * 32.0f) - ((float)xoff * 32.0f) - ((((float)vid_width / 2.0f) / (float)zoom) * MAJOR_UNIT);
}

// m_y: Returns the map coordiate of a y position on the screen
// --------------------------------------------------------- >>
double m_y(double y)
{
	return (((float)y / (float)zoom) * 32.0f) - ((float)yoff * 32.0f) - ((((float)vid_height / 2.0f) / (float)zoom) * MAJOR_UNIT);
}

point2_t m_p(point2_t point)
{
	return point2_t(m_x(point.x), m_y(point.y));
}

// get_line_colour: Returns a colour depending on a line's flags
// ---------------------------------------------------------- >>
void get_line_colour(WORD l, rgba_t *colour)
{
	if (map.lines[l]->flags & LINE_IMPASSIBLE || map.lines[l]->side2 == -1)
		colour->set(col_line_solid);
	else
	{
		colour->set(col_line_2s);

		if (map.lines[l]->flags & LINE_BLOCKMONSTERS)
			colour->set(col_line_monster);
	}

	if (map.lines[l]->type != 0)
	{
		rgba_t spec2s(col_line_special.r * 0.8, col_line_special.g * 0.8,
					col_line_special.b * 0.8, col_line_special.a, col_line_special.blend);

		if (map.lines[l]->flags & LINE_TWOSIDED)
			colour->set(spec2s);
		else
			colour->set(col_line_special);
	}

	if (map.lines[l]->flags & LINE_TWOSIDED && map.lines[l]->side2 == -1)
		colour->set(120, 0, 0, 255);

	if (map.lines[l]->side1 == -1)
		colour->set(col_line_nofirst);
}

// get_nearest_vertex: Gets the nearest vertex to a point
// --------------------------------------------------- >>
int get_nearest_vertex(int x, int y)
{
	double min_dist = 32;
	int vertex = -1;

	for (WORD v = 0; v < map.n_verts; v++)
	{
		double dist = distance(map.v_getspoint(v).x, map.v_getspoint(v).y, x, y);

		if (dist < min_dist)
		{
			min_dist = dist;
			vertex = v;
		}
	}

	return vertex;
}

// get_nearest_thing: Gets the nearest vertex to a point
// --------------------------------------------------- >>
int get_nearest_thing(double x, double y)
{
	double min_dist = 64;
	int thing = -1;

	for (WORD t = 0; t < map.n_things; t++)
	{
		double dist = distance(map.things[t]->x, map.things[t]->y, x, y);

		if (dist < min_dist)
		{
			min_dist = dist;
			thing = t;
		}
	}

	return thing;
}

// get_nearest_line: Gets the nearest line to a point
// (if any are closer than 64 units)
// ----------------------------------------------- >>
int get_nearest_line(double x, double y)
{
	double min_dist = 64;
	int line = -1;

	for (WORD l = 0; l < map.n_lines; l++)
	{
		rect_t r = map.l_getrect(l);

		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), x, y);

		if (dist < min_dist)
		{
			min_dist = dist;
			line = l;
		}
	}

	return line;
}

// get_nearest_line_2: Gets the nearest line to a point
// ------------------------------------------------- >>
int get_nearest_line_2(double x, double y)
{
	double min_dist = -1;
	int line = -1;

	for (WORD l = 0; l < map.n_lines; l++)
	{
		rect_t r = map.l_getrect(l);

		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), x, y);

		if (min_dist == -1 && map.l_getsector1(l) != -1)
		{
			min_dist = dist;
			line = l;
		}
		else if (dist < min_dist && map.l_getsector1(l) != -1)
		{
			min_dist = dist;
			line = l;
		}
	}

	return line;
}

// clear_hilights: Clears hilighted map items depending on the editing mode
// --------------------------------------------------------------------- >>
void clear_hilights()
{
	if (!lock_hilight)
		hilight_item = -1;
}

// clear_selection: Clears the selected items
// --------------------------------------- >>
void clear_selection()
{
	selected_items.clear();
	select_order.clear();
}

bool selection()
{
	if (selected_items.size() == 0)
		return false;
	else
		return true;
}

// determine_line_side: Determines what side of a line a certain point is on
// ---------------------------------------------------------------------- >>
bool determine_line_side(int line, float x, float y)
{
	rect_t r;
	r.set(map.l_getrect(line));
	r.tl.y = -r.tl.y;
	r.br.y = -r.br.y;

	float side = (-y - (float)r.y1()) * float(r.x2() - r.x1()) - (x - (float)r.x1()) * float(r.y2() - r.y1());

	if (side > 0)
		return true;
	else
		return false;
}

// determine_line_side: Determines what side of a line a certain point is on
// ---------------------------------------------------------------------- >>
bool determine_line_side(rect_t r, float x, float y)
{
	r.tl.y = -r.tl.y;
	r.br.y = -r.br.y;

	float side = (-y - (float)r.y1()) * float(r.x2() - r.x1()) - (x - (float)r.x1()) * float(r.y2() - r.y1());

	if (side >= 0)
		return true;
	else
		return false;
}

// determine_line_side_f: Determines what side of a line a certain point is on
// (returns the actual value so we can test for being exactly on the line)
// ------------------------------------------------------------------------ >>
float determine_line_side_f(rect_t r, float x, float y)
{
	r.tl.y = -r.tl.y;
	r.br.y = -r.br.y;

	return (-y - (float)r.y1()) * float(r.x2() - r.x1()) - (x - (float)r.x1()) * float(r.y2() - r.y1());
}

// determine_line_side: Determines what side of a line a certain point is on
// ---------------------------------------------------------------------- >>
bool determine_line_side(float x1, float y1, float x2, float y2, float x, float y)
{
	float side = ((-y + y1) * (x2 - x1)) - ((x - x1) * (-y2 + y1));

	if (side > 0)
		return true;
	else
		return false;
}

// determine_sector: Determines what sector a point lies in
// (returns -1 if point is in the 'void')
int determine_sector(double x, double y)
{
	//draw_text(16, 16, COL_WHITE, 0, "%d %d", x, y);
	int line = get_nearest_line_2(x, y);

	if (line == -1)
		return -1;

	if (determine_line_side(line, x, y))
		return map.l_getsector1(line);
	else
		return map.l_getsector2(line);
}

// get_hilight_item: Hilights the nearest item to the mouse pointer
// (if any items are close enough)
// ------------------------------------------------------------- >>
void get_hilight_item(int x, int y)
{
	if (lock_hilight)
		return;

	int old_hilight = hilight_item;

	clear_hilights();

	if (edit_mode == 0)
	{
		hilight_item = get_nearest_vertex(x, y);

		if (old_hilight != hilight_item)
			update_vertex_info_bar(hilight_item);
	}

	if (edit_mode == 1)
	{
		hilight_item = get_nearest_line(m_x(x), -m_y(y));

		if (old_hilight != hilight_item)
			update_line_info_bar(hilight_item);
	}
	
	if (edit_mode == 2)
	{
		hilight_item = determine_sector(m_x(x), -m_y(y));

		if (old_hilight != hilight_item)
			update_sector_info_bar(hilight_item);
	}

	if (edit_mode == 3)
	{
		hilight_item = get_nearest_thing(m_x(x), -m_y(y));

		if (old_hilight != hilight_item)
			update_thing_info_bar(hilight_item);
	}
}

// get_side_sector: Attempts to find what sector a line's side is in
// -------------------------------------------------------------- >>
int get_side_sector(int line, int side)
{
	rect_t linerect = map.l_getrect(line);
	point2_t mid = midpoint(linerect.tl, linerect.br);
	point2_t vec(linerect.x2() - linerect.x1(), linerect.y2() - linerect.y1());

	if (side == 2)
	{
		vec.x = -vec.x;
		vec.y = -vec.y;
	}

	int x = vec.y;
	int y = vec.x;
	x = -x;

	point2_t side_p(mid.x - x, mid.y - y);

	//printf("side %d\n", side);

	float min_dist = -1;
	int nearest_line = -1;
	point2_t nearest_midpoint;

	for (DWORD l = 0; l < map.n_lines; l++)
	{
		if (l != line)
		{
			rect_t line_r = map.l_getrect(l);
			int x1 = line_r.x1();
			int x2 = line_r.x2();
			int y1 = line_r.y1();
			int y2 = line_r.y2();

			point2_t r1 = mid;
			point2_t r2 = side_p;

			float u_ray = (float(x2 - x1) * float(r1.y - y1) - float(y2 - y1) * float(r1.x - x1)) /
							(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));
				
			float u_line = (float(r2.x - r1.x) * float(r1.y - y1) - float(r2.y - r1.y) * float(r1.x - x1)) /
							(float(y2 - y1) * float(r2.x - r1.x) - float(x2 - x1) * float(r2.y - r1.y));


			if ((u_ray >= 0) &&/* (u_ray < 1) && */(u_line >= 0) && (u_line <= 1))
			{
				if (u_ray < min_dist || min_dist == -1)
				{
					nearest_line = l;
					min_dist = u_ray;
					nearest_midpoint.set(midpoint(line_r.tl, line_r.br));
				}

				if (u_ray == min_dist)
				{
					double nearest_mid_dist = distance_to_line(linerect.x1(), linerect.y1(), linerect.x2(), linerect.y2(),
																nearest_midpoint.x, nearest_midpoint.y);

					point2_t this_midpoint(midpoint(line_r.tl, line_r.br));
					double this_mid_dist = distance_to_line(linerect.x1(), linerect.y1(), linerect.x2(), linerect.y2(),
															this_midpoint.x, this_midpoint.y);

					if (this_mid_dist < nearest_mid_dist)
					{
						nearest_line = l;
						min_dist = u_ray;
						nearest_midpoint.set(midpoint(line_r.tl, line_r.br));
					}
				}
			}
		}
	}

	if (nearest_line == -1)
	{
		//printf("No intersection\n");
		return -1;
	}
	else
	{
		if (determine_line_side(nearest_line, mid.x, mid.y))
			return map.l_getsector1(nearest_line);
		else
			return map.l_getsector2(nearest_line);
	}
}

// select_item: Selects/Deselects the currently highlighted item
// ---------------------------------------------------------- >>
void select_item()
{
	int old_hl = hilight_item;
	get_hilight_item(down_pos.x, down_pos.y);

	if (hilight_item == -1)
	{
		hilight_item = old_hl;
		return;
	}

	if (vector_exists(selected_items, hilight_item))
	{
		if (edit_mode == 0)
			select_order.erase(find(select_order.begin(), select_order.end(), hilight_item));

		selected_items.erase(find(selected_items.begin(), selected_items.end(), hilight_item));
	}
	else
	{
		if (edit_mode == 0)
			select_order.push_back(hilight_item);

		selected_items.push_back(hilight_item);
	}

	hilight_item = old_hl;
}

// select_items_box: Selects items within the selection box
// ----------------------------------------------------- >>
void select_items_box(rect_t box)
{
	int x1 = m_x(box.left());
	int x2 = m_x(box.right());
	int y1 = m_y(box.top());
	int y2 = m_y(box.bottom());

	// Vertices
	if (edit_mode == 0)
	{
		for (DWORD v = 0; v < map.n_verts; v++)
		{
			if (point_in_rect(x1, y1, x2, y2, map.verts[v]->x, -map.verts[v]->y))
			{
				selected_items.push_back(v);
				select_order.push_back(v);
			}
		}
	}
	
	// Lines
	if (edit_mode == 1)
	{
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (point_in_rect(x1, y1, x2, y2, map.l_getrect(l).x1(), -map.l_getrect(l).y1())
				&& point_in_rect(x1, y1, x2, y2, map.l_getrect(l).x2(), -map.l_getrect(l).y2()))
				selected_items.push_back(l);
		}
	}
	
	// Sectors
	if (edit_mode == 2)
	{
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			int sector1 = map.l_getsector1(l);
			int sector2 = map.l_getsector2(l);

			if (point_in_rect(x1, y1, x2, y2, map.l_getrect(l).x1(), -map.l_getrect(l).y1())
				&& point_in_rect(x1, y1, x2, y2, map.l_getrect(l).x2(), -map.l_getrect(l).y2()))
			{
				if (sector1 != -1)
					vector_add_nodup(selected_items, sector1);
					//selected_items.push_back(sector1);

				if (sector2 != -1)
					vector_add_nodup(selected_items, sector2);
					//selected_items.push_back(sector2);
			}
		}
	}
	
	// Things
	if (edit_mode == 3)
	{
		for (DWORD t = 0; t < map.n_things; t++)
		{
			if (point_in_rect(x1, y1, x2, y2, map.things[t]->x, -map.things[t]->y))
				selected_items.push_back(t);
		}
	}
}

// line_flip: Flips any hilighted/selected lines
// ------------------------------------------ >>
void line_flip(bool verts, bool sides)
{
	if (selected_items.size() != 0)
	{
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (vector_exists(selected_items, l))
			{
				if (verts)
					map.l_flip(l);
				if (sides)
					map.l_flipsides(l);
			}
		}
	}
	else if (hilight_item != -1)
	{
		if (verts)
			map.l_flip(hilight_item);
		if (sides)
			map.l_flipsides(hilight_item);
	}

	if (verts)
		map.change_level(MC_NODE_REBUILD);
		//map_changelevel(3);
	else
		map.change_level(MC_LINES);
		//map_changelevel(2);
}

// sector_changeheight: Changes floor/ceiling height of hilighted or selected sector(s)
// --------------------------------------------------------------------------------- >>
void sector_changeheight(bool floor, int amount)
{
	if (selected_items.size() == 0)
	{
		if (hilight_item != -1)
			map.s_changeheight(hilight_item, floor, amount);
	}
	else
	{
		for (DWORD s = 0; s < map.n_sectors; s++)
		{
			if (vector_exists(selected_items, s))
				map.s_changeheight(s, floor, amount);
		}
	}

	if (hilight_item != -1)
		update_sector_info_bar(hilight_item);

	//map_changelevel(2);
	map.change_level(MC_LINES|MC_SSECTS);
}

// delete_vertex: Deletes the currently hilighted vertex or all selected vertices
// --------------------------------------------------------------------------- >>
void delete_vertex()
{
	make_backup(true, false, true, false, false);

	if (selected_items.size() != 0)
	{
		vector<vertex_t *> del_verts;

		for (int a = 0; a < selected_items.size(); a++)
			del_verts.push_back(map.verts[selected_items[a]]);

		for (int a = 0; a < del_verts.size(); a++)
			map.delete_vertex(del_verts[a]);

		selected_items.clear();
	}
	else
	{
		if (hilight_item != -1)
			map.delete_vertex(hilight_item);
	}

	hilight_item = -1;
	//remove_free_verts();
	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
}

// delete_line: Deletes the currently hilighted line or all selected lines
// -------------------------------------------------------------------- >>
void delete_line()
{
	make_backup(true, false, true, false, false);

	if (selected_items.size() != 0)
	{
		vector<linedef_t *> del_lines;

		for (int a = 0; a < selected_items.size(); a++)
			del_lines.push_back(map.lines[selected_items[a]]);

		for (int a = 0; a < del_lines.size(); a++)
			map.delete_line(del_lines[a]);

		selected_items.clear();
	}
	else
	{
		if (hilight_item != -1)
			map.delete_line(hilight_item);
	}

	hilight_item = -1;
	remove_free_verts();
	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
}

// delete_thing: Deletes the currently hilighted thing or all selected things
// ----------------------------------------------------------------------- >>
void delete_thing()
{
	make_backup(false, false, false, false, true);

	if (selected_items.size() != 0)
	{
		vector<thing_t *> del_things;

		for (int a = 0; a < selected_items.size(); a++)
			del_things.push_back(map.things[selected_items[a]]);

		for (int a = 0; a < del_things.size(); a++)
			map.delete_thing(del_things[a]);

		selected_items.clear();
	}
	else
	{
		if (hilight_item != -1)
			map.delete_thing(hilight_item);
	}

	hilight_item = -1;
	//map_changelevel(2);
	map.change_level(MC_THINGS);
}

// delete_sector: Deletes the currently hilighted sector or all selected sectors
// -------------------------------------------------------------------------- >>
void delete_sector()
{
	make_backup(true, true, false, true, false);

	if (selected_items.size() != 0)
	{
		vector<sector_t *> del_sectors;

		for (int a = 0; a < selected_items.size(); a++)
			del_sectors.push_back(map.sectors[selected_items[a]]);

		for (int a = 0; a < del_sectors.size(); a++)
			map.delete_sector(del_sectors[a]);

		selected_items.clear();
	}
	else
	{
		if (hilight_item != -1)
			map.delete_sector(hilight_item);
	}

	hilight_item = -1;
	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
}

// map_changelevel: Sets the map_changed variable, ignores the change if not applicable
// --------------------------------------------------------------------------------- >>
void map_changelevel(int level)
{
	if (level > map_changed)
		map_changed = level;
}

// create_vertex: Adds a vertex at the mouse pointer and splits a line if it's close enough
// ------------------------------------------------------------------------------------- >>
void create_vertex()
{
	point2_t point(snap_to_grid(m_x(down_pos.x)), snap_to_grid(-m_y(down_pos.y)));

	if (map.v_checkspot(point.x, point.y))
	{
		int split_line = check_vertex_split(point);

		if (split_line != -1)
		{
			make_backup(true, true, true, false, false);
			map.add_vertex(point.x, point.y);
			map.l_split(split_line, map.n_verts - 1);
		}
		else
		{
			make_backup(false, false, true, false, false);
			map.add_vertex(point.x, point.y);
		}
	}
	else
		return;

	//map_changelevel(2);
	map.change_level(MC_LINES);
}

// create_lines: Creates lines between vertices in the select_order array
// (joins the last vertex to the first if close is true)
// ------------------------------------------------------------------- >>
void create_lines(bool close)
{
	if (select_order.size() < 2)
		return;

	make_backup(true, false, false, false, false);
	vector<int> lines;

	for (WORD p = 0; p < select_order.size() - 1; p++)
		lines.push_back(map.add_line(select_order[p], select_order[p + 1]));

	if (close)
	{
		if (select_order.size() > 2)
		{
			lines.push_back(map.add_line(select_order[select_order.size() - 1], select_order[0]));

			//if (check_overlapping_lines(&lines))
			//	merge_overlapping_lines(&lines);
		}
		/*
		else if (select_order.n_numbers == 2)
		{
			ldraw_points.add(map.v_getpoint(select_order.numbers[0]), true);
			ldraw_points.add(map.v_getpoint(select_order.numbers[1]), true);
			ldraw_points.add(map.v_getpoint(select_order.numbers[1]), false);
			end_linedraw();
			edit_mode = 1;
			clear_selection();
			return;
		}
		*/
	}

	clear_selection();
	edit_mode = 1;

	for (WORD l = 0; l < lines.size(); l++)
		selected_items.push_back(lines[l]);

	lines.clear();

	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
}

// create_sector: Creates a sector out of selected lines
// -------------------------------------------------- >>
void create_sector()
{
	int sector = map.add_sector();
	int side = -1;
	make_backup(true, true, false, true, false);

	for (DWORD l = 0; l < map.n_lines; l++)
	{
		if (vector_exists(selected_items, l))
		{
			side = map.add_side();
			map.sides[side]->sector = sector;

			if (map.lines[l]->side1 == -1)
				map.lines[l]->side1 = side;
			else if (map.lines[l]->side2 == -1)
			{
				map.lines[l]->side2 = side;
				map.lines[l]->flags = LINE_TWOSIDED;

				memcpy(map.sectors[sector], map.sectors[map.sides[map.lines[l]->side1]->sector], sizeof(sector_t));
			}

			if (map.lines[l]->side2 == -1)
				map.lines[l]->flags = LINE_IMPASSIBLE;

			map.l_setdeftextures(l);
		}
	}

	clear_selection();
	selected_items.push_back(sector);
	edit_mode = 2;
	get_hilight_item(mouse.x, mouse.y);
	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
}

// create_thing: Creates a thing at the mouse pointer, with properties taken from the last thing edited
// ------------------------------------------------------------------------------------------------- >>
void create_thing()
{
	make_backup(false, false, false, false, true);
	map.add_thing(snap_to_grid(m_x(down_pos.x)), snap_to_grid(-m_y(down_pos.y)), last_thing);
	map.things[map.n_things - 1]->ttype = get_thing_type(last_thing.type);
	//map_changelevel(2);
	map.change_level(MC_THINGS);
}

// check_vertex_split: Checks if a vertex is close enough to a line to split it
// ------------------------------------------------------------------------- >>
int check_vertex_split(DWORD vertex)
{
	int split_line = -1;
	short vx = map.verts[vertex]->x;
	short vy = map.verts[vertex]->y;

	if (gridsize == 1)
		return -1;
	
	for (DWORD l = 0; l < map.n_lines; l++)
	{
		rect_t r = map.l_getrect(l);
		
		if (distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), vx, vy) < 2
			&& map.lines[l]->vertex1 != vertex
			&& map.lines[l]->vertex2 != vertex)
			return l;
	}
	
	return -1;
}

// check_vertex_split: Checks if a point is close enough to a line to split it
// ------------------------------------------------------------------------ >>
int check_vertex_split(point2_t p)
{
	int split_line = -1;
	int vertex = map.v_getvertatpoint(p);

	if (gridsize == 1)
		return -1;
	
	for (DWORD l = 0; l < map.n_lines; l++)
	{
		rect_t r = map.l_getrect(l);
		
		if (distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), p.x, p.y) < 2
			&& map.lines[l]->vertex1 != vertex
			&& map.lines[l]->vertex2 != vertex)
			return l;
	}
	
	return -1;
}

// merge_verts: Merges all vertices in the map
// ---------------------------------------- >>
void merge_verts()
{
	for (DWORD v = 0; v < map.n_verts; v++)
		map.v_mergespot(map.verts[v]->x, map.verts[v]->y);

	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
	//remove_free_verts();
}

// check_overlapping_lines: Checks if lines in a list are overlapping any map lines
// ----------------------------------------------------------------------------- >>
bool check_overlapping_lines(vector<int> lines)
{
	for (DWORD a = 0; a < lines.size(); a++)
	{
		int l2 = lines[a];

		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (l != l2)
			{
				if (map.lines[l]->vertex1 == map.lines[l2]->vertex1 &&
					map.lines[l]->vertex2 == map.lines[l2]->vertex2)
					return true;

				if (map.lines[l]->vertex1 == map.lines[l2]->vertex2 &&
					map.lines[l]->vertex2 == map.lines[l2]->vertex1)
					return true;

				//if (check_vertex_split(map.lines[l]->vertex1) != -1 ||
					//check_vertex_split(map.lines[l]->vertex2) != -1)
					//return true;
			}
		}
	}

	return false;
}

// merge_overlapping_lines: Merges any overlapping lines (only if they are in the list given)
// --------------------------------------------------------------------------------------- >>
vector<linedef_t*> merge_lines;
bool merge_overlapping_lines(vector<int> lines)
{
	for (DWORD a = 0; a < lines.size(); a++)
	{
		int l2 = lines[a];

		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (l == l2)
				continue;

			if (map.lines[l]->vertex1 == map.lines[l2]->vertex1 &&
				map.lines[l]->vertex2 == map.lines[l2]->vertex2)
			{
				merge_lines.push_back(map.lines[l]);
				map.delete_line(l2);
				return true;
			}

			if (map.lines[l]->vertex1 == map.lines[l2]->vertex2 &&
				map.lines[l]->vertex2 == map.lines[l2]->vertex1)
			{
				merge_lines.push_back(map.lines[l]);
				map.delete_line(l2);
				return true;
			}
		}
	}

	for (int a = 0; a < map.n_lines; a++)
	{
		if (vector_exists(merge_lines, map.lines[a]))
			line_correct_references(a);
	}

	merge_lines.clear();

	map.change_level(MC_NODE_REBUILD);
	
	return false;
}

void change_edit_mode(int mode)
{
	edit_mode = mode;
	hilight_item = -1;
	clear_selection();
	force_map_redraw(true, false);
	change_infobar_page();
}

// init_map: Focuses on the center of the map & inits 3d camera
// --------------------------------------------------------- >>
void init_map()
{
	int max_x = 0;
	int min_x = 0;
	int max_y = 0;
	int min_y = 0;
	
	for (DWORD v = 0; v < map.n_verts; v++)
	{
		if (map.verts[v]->x < min_x)
			min_x = map.verts[v]->x;
		
		if (map.verts[v]->x > max_x)
			max_x = map.verts[v]->x;
		
		if (map.verts[v]->y < min_y)
			min_y = map.verts[v]->y;
		
		if (map.verts[v]->y > max_y)
			max_y = map.verts[v]->y;
	}
	
	xoff = -(min_x + ((max_x - min_x) / 2)) / MAJOR_UNIT;
	yoff = (min_y + ((max_y - min_y) / 2)) / MAJOR_UNIT;

	// Init camera
	point3_t pos(0.0f, 0.0f, 0.0f);
	point3_t view(0.0f, 1.0f, 0.0f);
	pos.z = 0.0f;

	for (int t = 0; t < map.n_things; t++)
	{
		if (map.things[t]->type == 1)
		{
			pos.x = map.things[t]->x * SCALE_3D;
			pos.y = map.things[t]->y * SCALE_3D;

			int sector = determine_sector(map.things[t]->x, map.things[t]->y);
			
			if (sector != -1)
				pos.z = (map.sectors[sector]->f_height + 40) * SCALE_3D;

			if (map.things[t]->angle == 0)			// east
				view.set(1.0f, 0.0f, 0.0f);
			else if (map.things[t]->angle == 45)	// northeast
				view.set(0.7f, 0.7f, 0.0f);
			else if (map.things[t]->angle == 90)	// north
				view.set(0.0f, 1.0f, 0.0f);
			else if (map.things[t]->angle == 135)	// northwest
				view.set(-0.7f, 0.7f, 0.0f);
			else if (map.things[t]->angle == 180)	// west
				view.set(-1.0f, 0.0f, 0.0f);
			else if (map.things[t]->angle == 225)	// southwest
				view.set(-0.7f, -0.7f, 0.0f);
			else if (map.things[t]->angle == 270)	// south
				view.set(0.0f, -1.0f, 0.0f);
			else if (map.things[t]->angle == 315)	// southeast
				view.set(0.7f, -0.7f, 0.0f);
			else
				view.set(1.0f, 0.0f, 0.0f);
		}
	}
	
	camera.position.set(pos);
	camera.view.set(pos + view);
	//map_changelevel(3);
	map.change_level(255);
}

// get_angle: Gets a doom thing angle from two points
// ----------------------------------------------- >>
short get_angle(point2_t origin, point2_t point)
{
	// Get a direction vector
	point2_t vec(point.x - origin.x, point.y - origin.y);
	float mag = (float)sqrt(float((vec.x * vec.x) + (vec.y * vec.y)));
	float x = vec.x / mag;
	float y = vec.y / mag;

	//draw_text(s_x(origin.x), s_y(origin.y), COL_WHITE, 0, "%1.2f, %1.2f", x, y);
	//draw_text(s_x(point.x), s_y(point.y), COL_WHITE, 0, "%d, %d", vec.x, vec.y);

	if (x > 0.89)	// east
		return 0;
	if (x < -0.89)	// west
		return 180;
	if (y > 0.89)	// north
		return 90;
	if (y < -0.89)	// south
		return 270;
	if (x > 0 && y > 0)	// northeast
		return 45;
	if (x < 0 && y > 0)	// northwest
		return 135;
	if (x < 0 && y < 0)	// southwest
		return 225;
	if (x > 0 && y < 0)	// southeast
		return 315;

	return 0;
}

void thing_setquickangle()
{
	if (edit_mode != 3)
		return;

	point2_t m_mouse(m_x(mouse.x), -m_y(mouse.y));

	if (selected_items.size() != 0)
	{
		for (DWORD a = 0; a < selected_items.size(); a++)
		{
			int t = selected_items[a];
			map.things[t]->angle = get_angle(point2_t(map.things[t]->x, map.things[t]->y), m_mouse);
		}
	}
	else if (hilight_item != -1)
	{
		map.things[hilight_item]->angle = get_angle(point2_t(map.things[hilight_item]->x, map.things[hilight_item]->y), m_mouse);
		last_thing.angle = get_angle(point2_t(map.things[hilight_item]->x, map.things[hilight_item]->y), m_mouse);
	}

	//update_map();
}
