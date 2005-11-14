
#include "main.h"
#include "struct_3d.h"
#include "input.h"
#include "keybind.h"
#include "render.h"
#include "camera.h"
#include "edit.h"
#include "map.h"
#include "mathstuff.h"
#include "bsp.h"
#include "tex_browser.h"
#include "misc.h"

float grav = 0.01f;

wallrect_t*	hl_wrect = NULL;
flatpoly_t* hl_fpoly = NULL;

string copy_wtex = "-";
string copy_ftex = "";

extern Camera camera;
extern Map map;

extern sectinfo_t *sector_info;
extern vector<line3d_t> lines_3d;
extern vector<ssect3d_t> ssects_3d;
extern vector<wallrect_t*> wallrects;
extern gl_ssect_t *gl_ssects;
extern bool mix_tex;

// line_intersect: Determines if the camera is looking at a line
// (borrowed from Doom Builder, thanks CodeImp :D)
// ---------------------------------------------------------- >>
float line_intersect(point3_t r1, point3_t r2, float x1, float y1, float x2, float y2)
{
	// Calculate the intersection distance from the ray
	float u_ray = ((x2 - x1) * (r1.y - y1) - (y2 - y1) * (r1.x - x1)) /
				  ((y2 - y1) * (r2.x - r1.x) - (x2 - x1) * (r2.y - r1.y));
	
	// Calculate the intersection distance from the line
	float u_line = ((r2.x - r1.x) * (r1.y - y1) - (r2.y - r1.y) * (r1.x - x1)) /
				   ((y2 - y1) * (r2.x - r1.x) - (x2 - x1) * (r2.y - r1.y));
	
	// Return the distance on the ray if intersecting, or return -1
	if((u_ray >= 0)/* && (u_ray <= 1024) */&& (u_line >= 0) && (u_line <= 1)) return u_ray; else return -1;
}

float rays_intersect_dist(point3_t r1_o, point3_t r1_d, point3_t r2_o, point3_t r2_d)
{
	return ((r2_d.x - r2_o.x) * (r1_o.y - r2_o.y) - (r2_d.y - r2_o.y) * (r1_o.x - r2_o.x)) /
			((r2_d.y - r2_o.y) * (r1_d.x - r1_o.x) - (r2_d.x - r2_o.x) * (r1_d.y - r1_o.y));
}

float get_slope_height_point(point3_t slope_start, point3_t slope_dir, point3_t point)
{
	point3_t slope_vec(slope_dir.x - slope_start.x, slope_dir.y - slope_start.y, slope_dir.z - slope_start.z);
	slope_vec.normalize();

	// Get the ray from the point, in a direction perpendicular to the slope ray
	point3_t perp(point.x + slope_vec.y, point.y - slope_vec.x, point.z);

	// Get distance of intersection between the rays
	float dist = rays_intersect_dist(slope_start, slope_dir, point, perp);

	return slope_start.z + (slope_vec.z * dist);
}

// dist_ray_plane: Returns the distance between a ray and a plane
// ----------------------------------------------------------- >>
float dist_ray_plane(point3_t r_o, point3_t r_v, plane_t plane)
{
    float cos_a;
    float delta_d;

	point3_t p_normal(-plane.a, -plane.b, plane.c);

    cos_a = dot_product(r_v, p_normal.normalize());

    // parallel to the plane (alpha=90)
    if (cos_a == 0)
		return -1.0f;

    delta_d = plane.d - dot_product(r_o, p_normal.normalize());
    
    return (delta_d / cos_a);
}

void determine_hilight()
{
	float min_dist = 1024;
	hl_wrect = NULL;
	hl_fpoly = NULL;

	// List of sectors the camera view ray passes through
	vector<int> sectors;

	// Eventually: Check Things

	// Check Lines
	for (int a = 0; a < map.n_lines; a++)
	{
		if (!lines_3d[a].visible)
			continue;

		rect_t lrect = map.l_getrect(a);
		float dist = line_intersect(camera.position, camera.view,
									(float)lrect.x1() * SCALE_3D, (float)lrect.y1() * SCALE_3D,
									(float)lrect.x2() * SCALE_3D, (float)lrect.y2() * SCALE_3D);

		if (dist != -1 && dist < min_dist)
		{
			point3_t direction = camera.view - camera.position;
			point3_t hit_point(camera.position.x + (direction.x * dist),
								camera.position.y + (direction.y * dist),
								camera.position.z + (direction.z * dist));

			sectors.push_back(map.l_getsector1(a));
			sectors.push_back(map.l_getsector2(a));

			// For all wallrects on the line
			for (int r = 0; r < lines_3d[a].rects.size(); r++)
			{
				if (!determine_line_side(lines_3d[a].rects[r]->verts[0].x, lines_3d[a].rects[r]->verts[0].y,
										lines_3d[a].rects[r]->verts[1].x, lines_3d[a].rects[r]->verts[1].y,
										camera.position.x, camera.position.y))
					continue;

				float up = get_slope_height_point(lines_3d[a].rects[r]->verts[0], lines_3d[a].rects[r]->verts[1], hit_point);
				float lo = get_slope_height_point(lines_3d[a].rects[r]->verts[3], lines_3d[a].rects[r]->verts[2], hit_point);

				if (up >= hit_point.z && lo <= hit_point.z)
				{
					hl_wrect = lines_3d[a].rects[r];
					min_dist = dist;
					break;
				}
			}
		}
	}

	// Check sectors
	for (int a = 0; a < ssects_3d.size(); a++)
	{
		if (!ssects_3d[a].visible)
			continue;

		point3_t direction = camera.view - camera.position;
		for (int b = 0; b < ssects_3d[a].flats.size(); b++)
		{
			// Get flat plane
			flatpoly_t *poly = ssects_3d[a].flats[b];

			if (!(vector_exists(sectors, poly->parent_sector)))
				continue;

			plane_t plane;

			if (poly->part == PART_CEIL)
				plane = sector_info[poly->parent_sector].c_plane;
			else
				plane = sector_info[poly->parent_sector].f_plane;

			float dist = dist_ray_plane(camera.position, direction, plane);
			if (dist < min_dist && dist > 0.0f)
			{
				point3_t intersection(camera.position.x + (direction.x * dist),
								camera.position.y + (direction.y * dist),
								camera.position.z + (direction.z * dist));

				bool in = true;
				DWORD start = gl_ssects[a].startseg;
				DWORD end = start + gl_ssects[a].n_segs;
				for (DWORD seg = start; seg < end; seg++)
				{
					if (!determine_seg_side(seg, intersection.x, intersection.y))
					{
						in = false;
						break;
					}
				}

				if (in)
				{
					hl_wrect = NULL;
					hl_fpoly = poly;
					min_dist = dist;
				}
			}
		}
	}
}

// apply_gravity: Applies gravity to the camera
// ----------------------------------------- >>
void apply_gravity()
{
	int sector = determine_sector((int)(camera.position.x / SCALE_3D), (int)(camera.position.y / SCALE_3D));

	if (sector != -1)
	{
		float floor_height = 0.0f;
		bool moved = false;

		if (sector_info[sector].f_plane.c != 1.0f)
		{
			floor_height = plane_height(sector_info[sector].f_plane, camera.position.x, camera.position.y) + (40 * SCALE_3D);
			float diff = floor_height - camera.position.z;
			camera.position.z = floor_height;
			camera.view.z += diff;
			return;
		}

		if (map.sectors[sector]->f_height + 40 < map.sectors[sector]->c_height)
			floor_height = (float)(map.sectors[sector]->f_height + 40) * SCALE_3D;
		else
		{
			floor_height = (float)(map.sectors[sector]->c_height - 4) * SCALE_3D;
			float diff = floor_height - camera.position.z;
			camera.position.z = floor_height;
			camera.view.z += diff;
			grav = 0.01f;
			return;
		}

		if (camera.position.z > floor_height + 0.05f)
		{
			grav = grav * 2;
			camera.position.z -= grav;
			camera.view.z -= grav;
			moved = true;
		}

		if (camera.position.z < floor_height - 0.05f)
		{
			grav = grav * 2;
			camera.position.z += grav;
			camera.view.z += grav;
			moved = true;
		}

		if (camera.position.z > floor_height - grav && camera.position.z < floor_height + grav && moved)
		{
			float diff = floor_height - camera.position.z;
			camera.position.z = floor_height;
			camera.view.z += diff;
			grav = 0.01f;
		}
	}
}

void change_sector_height_3d(int amount, bool floor)
{
	int sector = -1;
	BYTE part = PART_FLOOR;

	if (hl_fpoly)
	{
		sector = hl_fpoly->parent_sector;
		part = hl_fpoly->part;
	}
	else if (hl_wrect)
	{
		sector = hl_wrect->sector;
		if (floor)
			part = PART_FLOOR;
		else
			part = PART_CEIL;
	}

	if (sector == -1)
		return;

	if (part == PART_FLOOR)
	{
		map.sectors[sector]->f_height += amount;
		add_3d_message(parse_string("Floor height: %d", map.sectors[sector]->f_height));
	}
	else
	{
		map.sectors[sector]->c_height += amount;
		add_3d_message(parse_string("Ceiling height: %d", map.sectors[sector]->c_height));
	}

	setup_sector(sector);

	for (int a = 0; a < ssects_3d.size(); a++)
	{
		for (int b = 0; b < ssects_3d[a].flats.size(); b++)
		{
			if (ssects_3d[a].flats[b]->parent_sector == sector &&
				ssects_3d[a].flats[b]->part == part)
				setup_flatpoly(ssects_3d[a].flats[b], a);
		}
	}

	for (int a = 0; a < lines_3d.size(); a++)
	{
		if (map.l_getsector1(a) == sector ||
			map.l_getsector2(a) == sector)
			setup_3d_line(a);
	}

	determine_hilight();
}

void change_texture_3d(bool paint)
{
	// Change flat texture
	if (hl_fpoly)
	{
		int sector = hl_fpoly->parent_sector;
		BYTE part = hl_fpoly->part;
		string otex;

		if (sector == -1)
			return;

		if (part == PART_FLOOR)
			otex = map.sectors[sector]->f_tex;
		else
			otex = map.sectors[sector]->c_tex;

		string ntex = open_texture_browser(false, true, false, otex, true);

		if (ntex != otex)
		{
			if (part == PART_FLOOR)
				map.sectors[sector]->f_tex = ntex;
			else
				map.sectors[sector]->c_tex = ntex;

			for (int a = 0; a < ssects_3d.size(); a++)
			{
				for (int b = 0; b < ssects_3d[a].flats.size(); b++)
				{
					if (ssects_3d[a].flats[b]->parent_sector == sector &&
						ssects_3d[a].flats[b]->part == part)
						setup_flatpoly(ssects_3d[a].flats[b], a);
				}
			}
		}
	}

	// Change line texture
	else if (hl_wrect)
	{
		string otex = hl_wrect->tex->name;
		string ntex = open_texture_browser(true, false, false, otex, true);

		if (ntex != otex)
		{
			if (hl_wrect->part == PART_MIDDLE || hl_wrect->part == PART_TRANS)
				map.l_getside(hl_wrect->line, hl_wrect->side)->tex_middle = ntex;
			else if (hl_wrect->part == PART_UPPER)
				map.l_getside(hl_wrect->line, hl_wrect->side)->tex_upper = ntex;
			else if (hl_wrect->part == PART_LOWER)
				map.l_getside(hl_wrect->line, hl_wrect->side)->tex_lower = ntex;

			setup_3d_line(hl_wrect->line);
		}
	}
}

void change_offsets_3d(int x, int y)
{
	if (hl_wrect)
	{
		sidedef_t* side = map.l_getside(hl_wrect->line, hl_wrect->side);
		side->x_offset += x;
		side->y_offset += y;
		setup_3d_line(hl_wrect->line);
		add_3d_message(parse_string("Offsets: (%d, %d)", side->x_offset, side->y_offset));
	}
}

void toggle_texture_peg_3d(bool upper)
{
	if (hl_wrect)
	{
		string msg;
		if (upper)
		{
			map.lines[hl_wrect->line]->toggle_flag(LINE_UPPERUNPEGGED);
			msg = "Upper texture unpegged: ";
			if (map.lines[hl_wrect->line]->flags & LINE_UPPERUNPEGGED)
				msg += "On";
			else
				msg += "Off";
		}
		else
		{
			map.lines[hl_wrect->line]->toggle_flag(LINE_LOWERUNPEGGED);
			msg = "Lower texture unpegged: ";
			if (map.lines[hl_wrect->line]->flags & LINE_LOWERUNPEGGED)
				msg += "On";
			else
				msg += "Off";
		}

		setup_3d_line(hl_wrect->line);
		add_3d_message(msg);
	}
}

void copy_texture_3d()
{
	if (hl_wrect)
	{
		copy_wtex = hl_wrect->tex->name;
		add_3d_message(parse_string("Copied texture \"%s\"", copy_wtex.c_str()));
	}

	else if (hl_fpoly)
	{
		string ct;

		if (hl_fpoly->part == PART_FLOOR)
			ct = map.sectors[hl_fpoly->parent_sector]->f_tex;
		else
			ct = map.sectors[hl_fpoly->parent_sector]->c_tex;

		if (mix_tex)
			copy_wtex = ct;
		else
			copy_ftex = ct;

		add_3d_message(parse_string("Copied texture \"%s\"", ct.c_str()));
	}
}

void paste_texture_3d(bool paint)
{
	if (hl_wrect)
	{
		if (hl_wrect->part == PART_MIDDLE || hl_wrect->part == PART_TRANS)
			map.l_getside(hl_wrect->line, hl_wrect->side)->tex_middle = copy_wtex;
		else if (hl_wrect->part == PART_UPPER)
			map.l_getside(hl_wrect->line, hl_wrect->side)->tex_upper = copy_wtex;
		else if (hl_wrect->part == PART_LOWER)
			map.l_getside(hl_wrect->line, hl_wrect->side)->tex_lower = copy_wtex;

		setup_3d_line(hl_wrect->line);
		add_3d_message(parse_string("Pasted texture \"%s\"", copy_wtex.c_str()));
	}
	else if (hl_fpoly)
	{
		string tex;
		if (mix_tex)
			tex = copy_wtex;
		else
			tex = copy_ftex;

		if (hl_fpoly->part == PART_FLOOR)
			map.sectors[hl_fpoly->parent_sector]->f_tex = tex;
		else
			map.sectors[hl_fpoly->parent_sector]->c_tex = tex;

		for (int a = 0; a < ssects_3d.size(); a++)
		{
			for (int b = 0; b < ssects_3d[a].flats.size(); b++)
			{
				if (ssects_3d[a].flats[b]->parent_sector == hl_fpoly->parent_sector &&
					ssects_3d[a].flats[b]->part == hl_fpoly->part)
					setup_flatpoly(ssects_3d[a].flats[b], a);
			}
		}

		add_3d_message(parse_string("Pasted texture \"%s\"", tex.c_str()));
	}
}

void change_light_3d(int amount)
{
	int sector;

	if (hl_wrect)
		sector = hl_wrect->sector;
	else if (hl_fpoly)
		sector = hl_fpoly->parent_sector;

	int light = map.sectors[sector]->light;
	
	if (light == 255)
		light = 256;

	light += amount;

	if (light > 255)
		light = 255;
	if (light < 0)
		light = 0;

	map.sectors[sector]->light = light;

	add_3d_message(parse_string("Light level: %d", light));
}
