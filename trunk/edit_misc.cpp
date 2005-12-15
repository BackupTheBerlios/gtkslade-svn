// << ------------------------------------- >>
// << SLADE - SlayeR's LeetAss Doom Editor  >>
// << By Simon Judd, 2004-05                >>
// << ------------------------------------- >>
// << edit_misc.cpp - Various editing tools >>
// << (for the 'edit' menu)                 >>
// << ------------------------------------- >>

// INCLUDES ------------------------------ >>
#include "main.h"
#include "map.h"
#include "undoredo.h"
#include "editor_window.h"
#include "struct_3d.h"
#include "mathstuff.h"
#include "textures.h"
#include "info_bar.h"

// Variables ----------------------------- >>

// External Variables -------------------- >>
extern Map map;
extern vector<int> selected_items;
extern int edit_mode;
extern int hilight_item;

// SECTORS

// sector_create_stairs: Creates stairs from a group of sectors
// --------------------------------------------------------- >>
void sector_create_stairs(int floor_step, int ceil_step)
{
	if (selected_items.size() <= 1 || edit_mode != 2)
		return;

	make_backup(false, false, false, true, false);

	// Floor
	int height = map.sectors[selected_items[0]]->f_height;
	for (DWORD a = 0; a < selected_items.size(); a++)
	{
		height += floor_step;
		map.sectors[selected_items[a]]->f_height = height;
	}

	// Ceiling
	height = map.sectors[selected_items[0]]->c_height;
	for (DWORD a = 0; a < selected_items.size(); a++)
	{
		height += ceil_step;
		map.sectors[selected_items[a]]->c_height = height;
	}

	//map_changelevel(2);
	map.change_level(MC_SSECTS|MC_LINES);
}

// sector_create_door: Creates a door from any selected sectors
// --------------------------------------------------------- >>
void sector_create_door(string texture)
{
	if (selected_items.size() == 0 || edit_mode != 2)
		return;

	make_backup(true, true, false, true, false);

	for (DWORD a = 0; a < selected_items.size(); a++)
	{
		int s = selected_items[a];

		// Set ceiling to floor
		map.sectors[s]->c_height = map.sectors[s]->f_height;
		
		// Go through lines in the sector, set their texture and type accordingly
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (map.lines[l]->side2 != -1)
			{
				// Any 2-sided lines
				if (map.sides[map.lines[l]->side2]->sector == s)
				{
					map.sides[map.lines[l]->side1]->tex_upper = texture;
					
					if (!map.zdoom)
						map.lines[l]->type = 1;
					else
					{
						map.lines[l]->type = 12;
						map.lines[l]->args[1] = 24;
						map.lines[l]->args[2] = 128;
						map.lines[l]->set_flag(LINE_REPEATABLE);
						map.lines[l]->set_trigger(LINE_SPAC_USE);
					}
				}

				if (map.sides[map.lines[l]->side1]->sector == s)
				{
					map.sides[map.lines[l]->side2]->tex_upper = texture;

					if (!map.zdoom)
						map.lines[l]->type = 1;
					else
					{
						map.lines[l]->type = 12;
						map.lines[l]->args[1] = 24;
						map.lines[l]->args[2] = 128;
						map.lines[l]->set_flag(LINE_REPEATABLE);
						map.lines[l]->set_trigger(LINE_SPAC_USE);
					}
				}
			}
			else // 1-sided lines
			{
				if (map.sides[map.lines[l]->side1]->sector == s)
				{
					map.sides[map.lines[l]->side1]->tex_middle = "DOORTRAK";
					map.lines[l]->set_flag(LINE_LOWERUNPEGGED);
				}
			}
		}
	}

	force_map_redraw(true, false);
	map.change_level(MC_SSECTS|MC_LINES);
	//map_changelevel(2);
}

// sector_merge: Merges selected sectors to the first selected sector
// --------------------------------------------------------------- >>
void sector_merge(bool remove_lines)
{
	if (selected_items.size() == 0 || edit_mode != 2)
		return;

	make_backup(true, true, false, true, false);

	int sector = selected_items[0];

	for (int l = 0; l < map.n_lines; l++)
	{
		if (remove_lines)
		{
			// If both sides of the line are selected, delete the line
			if (vector_exists(selected_items, map.l_getsector1(l)) &&
				vector_exists(selected_items, map.l_getsector2(l)))
				map.delete_line(l);
		}

		if (vector_exists(selected_items, map.l_getsector1(l)))
			map.l_setsector(l, 1, sector);

		if (vector_exists(selected_items, map.l_getsector2(l)))
			map.l_setsector(l, 2, sector);
	}

	selected_items.clear();
	force_map_redraw(true, false);
	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
}

void sector_changelight(int amount)
{
	if (edit_mode != 2 || (selected_items.size() == 0 && hilight_item == -1))
		return;

	make_backup(false, false, false, true, false);

	vector<int> sectors;

	// Get sectors to change
	if (selected_items.size() == 0)
		sectors.push_back(hilight_item);
	else
	{
		for (int a = 0; a < selected_items.size(); a++)
			vector_add_nodup(sectors, selected_items[a]);
			//sectors.push_back(selected_items[a]);
	}

	// Change light levels
	for (int a = 0; a < sectors.size(); a++)
	{
		int light = map.sectors[sectors[a]]->light;

		if (light == 255)
			light = 256;

		light += amount;

		if (light < 0)
			light = 0;
		if (light > 255)
			light = 255;

		map.sectors[sectors[a]]->light = light;
	}

	if (hilight_item != -1)
		update_sector_info_bar(hilight_item);
}

void sector_create(point2_t point)
{
	double min_dist = -1;
	int line = -1;

	for (WORD l = 0; l < map.n_lines; l++)
	{
		rect_t r = map.l_getrect(l);

		double dist = distance_to_line(r.x1(), r.y1(), r.x2(), r.y2(), point.x, point.y);

		if (min_dist == -1)
		{
			min_dist = dist;
			line = l;
		}
		else if (dist < min_dist)
		{
			min_dist = dist;
			line = l;
		}
	}

	if (line == -1)
		return;

	vector<int> processed_lines;
	bool done = false;
	while (!done)
	{
		int vertex;
		if (determine_line_side(line, point.x, point.y))
			vertex = map.lines[line]->vertex2;
		else
			vertex = map.lines[line]->vertex1;

		vector<int> a_lines;
		a_lines = map.v_getattachedlines(vertex);
		

		// Get line with lowest angle from the current line
		for (int a = 0; a < a_lines.size(); a++)
		{
			if (a_lines[a] == line)
				continue;

			point3_t l_ray2;
		}
	}
}

// LINES

// line_extrude: 'extrudes' selected lines by a specified amount
// ---------------------------------------------------------- >>
void line_extrude(int amount)
{
	if (selected_items.size() == 0 || edit_mode != 1)
		return;

	make_backup(true, true, true, false, false);

	for (int a = 0; a < selected_items.size(); a++)
	{
		int l = selected_items[a];

		// Get a perpendicular vector to the line
		point2_t line(map.l_getrect(l).x2() - map.l_getrect(l).x1(), map.l_getrect(l).y2() - map.l_getrect(l).y1());
		point3_t vec(-line.y, line.x, 0.0f);
		vec = vec.normalize();

		// Add the new vertices
		int v1 = map.add_vertex(map.l_getrect(l).x1() - (vec.x * amount), map.l_getrect(l).y1() - (vec.y * amount));
		int v2 = map.add_vertex(map.l_getrect(l).x2() - (vec.x * amount), map.l_getrect(l).y2() - (vec.y * amount));

		// Split the line through the new vertices
		map.l_split(l, v2);
		map.l_split(l, v1);
	}

	selected_items.clear();
	//map_changelevel(3);
	map.change_level(MC_NODE_REBUILD);
	force_map_redraw(true, false);
}

void line_align_x()
{
	if (selected_items.size() <= 1 || edit_mode != 1)
		return;

	int offset = map.l_getrect(selected_items[0]).length();
	for (int a = 1; a < selected_items.size(); a++)
	{
		sidedef_t *side = map.l_getside(selected_items[a], 1);
		if (side) side->x_offset = offset;

		side = map.l_getside(selected_items[a], 2);
		if (side) side->x_offset = offset;

		offset += map.l_getrect(selected_items[a]).length();
	}
}

void line_align_y()
{
}

void line_correct_references(int line)
{
	if (line != -1)
	{
		map.l_setsector(line, 1, get_side_sector(line, 1));
		map.l_setsector(line, 2, get_side_sector(line, 2));

		return;
	}

	if (edit_mode != 1)
		return;

	if (selected_items.size() == 0 && hilight_item != -1)
	{
		map.l_setsector(hilight_item, 1, get_side_sector(hilight_item, 1));
		map.l_setsector(hilight_item, 2, get_side_sector(hilight_item, 2));
	}
	else if (selected_items.size() > 0)
	{
		for (int a = 0; a < selected_items.size(); a++)
		{
			map.l_setsector(selected_items[a], 1, get_side_sector(selected_items[a], 1));
			map.l_setsector(selected_items[a], 2, get_side_sector(selected_items[a], 2));
		}
	}
}

void line_auto_align_x(int line, int offset, int side, string tex, int texwidth, vector<int> *processed_lines)
{
	if (offset == -51403)
		offset = map.l_getxoff(line, side);
	else
		map.l_getside(line, side)->x_offset = offset;

	offset += map.l_getrect(line).length();
	processed_lines->push_back(line);

	if (offset >= texwidth)
		offset -= texwidth;

	numlist_t a_lines;

	if (side == 1)
		map.v_getattachedlines(map.lines[line]->vertex2, &a_lines);
	else
		map.v_getattachedlines(map.lines[line]->vertex1, &a_lines);

	for (int a = 0; a < a_lines.n_numbers; a++)
	{
		if (find(processed_lines->begin(), processed_lines->end(), a_lines.numbers[a]) == processed_lines->end())
		{
			if (map.l_getside(a_lines.numbers[a], 1))
			{
				if (map.l_getside(a_lines.numbers[a], 1)->tex_lower == tex ||
					map.l_getside(a_lines.numbers[a], 1)->tex_middle == tex ||
					map.l_getside(a_lines.numbers[a], 1)->tex_upper == tex)
					line_auto_align_x(a_lines.numbers[a], offset, 1, tex, texwidth, processed_lines);
			}

			if (map.l_getside(a_lines.numbers[a], 2))
			{
				if (map.l_getside(a_lines.numbers[a], 2)->tex_lower == tex ||
					map.l_getside(a_lines.numbers[a], 2)->tex_middle == tex ||
					map.l_getside(a_lines.numbers[a], 2)->tex_upper == tex)
					line_auto_align_x(a_lines.numbers[a], offset, 2, tex, texwidth, processed_lines);
			}
		}
	}
}

bool line_hastex(int line, int side, string tex)
{
	sidedef_t *sdef = map.l_getside(line, side);

	if (!sdef)
		return false;

	if (map.l_needsuptex(line, side) && sdef->tex_upper == tex)
		return true;
	if (map.l_needsmidtex(line, side) && sdef->tex_middle == tex)
		return true;
	if (map.l_needslotex(line, side) && sdef->tex_lower == tex)
		return true;

	return false;
}

void line_paint_tex(int line, int side, string otex, string ntex, vector<int> *processed_lines)
{
	sidedef_t *sdef = map.l_getside(line, side);

	if (sdef)
	{
		if (map.l_needslotex(line, side) && sdef->tex_lower == otex)
			sdef->tex_lower = ntex;
		if (map.l_needsmidtex(line, side) && sdef->tex_middle == otex)
			sdef->tex_middle = ntex;
		if (map.l_needsuptex(line, side) && sdef->tex_upper == otex)
			sdef->tex_upper = ntex;
	}

	processed_lines->push_back(line);

	numlist_t a_lines;

	map.v_getattachedlines(map.lines[line]->vertex2, &a_lines);
	map.v_getattachedlines(map.lines[line]->vertex1, &a_lines);

	for (int a = 0; a < a_lines.n_numbers; a++)
	{
		if (find(processed_lines->begin(), processed_lines->end(), a_lines.numbers[a]) == processed_lines->end())
		{
			if (line_hastex(a_lines.numbers[a], 1, otex))
				line_paint_tex(a_lines.numbers[a], 1, otex, ntex, processed_lines);

			if (line_hastex(a_lines.numbers[a], 2, otex))
				line_paint_tex(a_lines.numbers[a], 2, otex, ntex, processed_lines);
		}
	}
}

void line_split(int splits)
{
	if (selected_items.size() == 0 && hilight_item == -1)
		return;

	make_backup(true, true, true, false, false);

	if (selected_items.size() == 0)
	{
		rect_t rect = map.l_getrect(hilight_item);

		float x_split = float(rect.x2() - rect.x1()) / (float)splits;
		float y_split = float(rect.y2() - rect.y1()) / (float)splits;

		for (int a = 1; a < splits; a++)
		{
			int x = rect.x1() + int(a * x_split);
			int y = rect.y1() + int(a * y_split);

			int v = map.add_vertex(x, y);
			map.l_split(hilight_item, v);
		}
	}
	else
	{
		for (int a = 0; a < selected_items.size(); a++)
		{
			rect_t rect = map.l_getrect(selected_items[a]);

			float x_split = float(rect.x2() - rect.x1()) / (float)splits;
			float y_split = float(rect.y2() - rect.y1()) / (float)splits;

			int line = selected_items[a];
			for (int a = 1; a < splits; a++)
			{
				int x = rect.x1() + int(a * x_split);
				int y = rect.y1() + int(a * y_split);

				int v = map.add_vertex(x, y);
				line = map.l_split(line, v);
			}
		}
	}

	force_map_redraw(true, false);
}
