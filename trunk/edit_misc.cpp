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
#include "struct_3d.h"
#include "editor_window.h"

// Variables ----------------------------- >>

// External Variables -------------------- >>
extern Map map;
extern vector<int> selected_items;
extern int edit_mode;

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

	map_changelevel(2);
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
	map_changelevel(2);
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
	map_changelevel(3);
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
	map_changelevel(3);
	force_map_redraw(true, false);
}
