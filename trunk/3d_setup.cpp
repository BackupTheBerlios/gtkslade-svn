
#include "main.h"
#include "struct_3d.h"
#include "camera.h"
#include "map.h"
#include "bsp.h"
#include "render.h"
#include "splash.h"

sectinfo_t	*sector_info;

vector<line3d_t>	lines_3d;
vector<ssect3d_t>	ssects_3d;

extern Camera camera;
extern vector<wallrect_t*> wallrects;
extern vector<flatpoly_t*> flatpolys;
extern vector<thing3d_t*> things_3d;
extern Map map;

extern gl_vertex_t	*gl_verts;
extern gl_seg_t		*gl_segs;
extern gl_ssect_t	*gl_ssects;
extern gl_node_t	*gl_nodes;
extern DWORD		n_gl_verts;
extern DWORD		n_gl_segs;
extern DWORD		n_gl_ssects;

extern int map_changed;

void setup_wallrect(wallrect_t *wall)
{
	wall->verts.clear();

	rect_t rect = map.l_getrect(wall->line);
	sidedef_t* side1;
	sidedef_t* side2;

	if (wall->side == 1)
	{
		side1 = map.l_getside(wall->line, 1);
		side2 = map.l_getside(wall->line, 2);
	}
	else
	{
		side1 = map.l_getside(wall->line, 2);
		side2 = map.l_getside(wall->line, 1);
		point2_t point(rect.tl.x, rect.tl.y);
		rect.tl.set(rect.br.x, rect.br.y);
		rect.br.set(point.x, point.y);
	}

	float x1 = rect.x1() * SCALE_3D;
	float y1 = rect.y1() * SCALE_3D;
	float x2 = rect.x2() * SCALE_3D;
	float y2 = rect.y2() * SCALE_3D;

	if (wall->sector == -1)
		wall->sector = side1->sector;

	wall->light = map.sectors[wall->sector]->light;

	if (wall->part == PART_MIDDLE)
	{
		int f = map.sectors[side1->sector]->f_height;
		int c = map.sectors[side1->sector]->c_height;
		plane_t f_plane = sector_info[side1->sector].f_plane;
		plane_t c_plane = sector_info[side1->sector].c_plane;

		Texture* tex = get_texture(side1->tex_middle, 1);
		rect_t t_c;

		if (map.lines[wall->line]->flags & LINE_LOWERUNPEGGED)
		{
			t_c.tl.set(side1->x_offset, side1->y_offset - (c-f));
			t_c.br.set(side1->x_offset + rect.length(), side1->y_offset);
		}
		else
		{
			t_c.tl.set(side1->x_offset, side1->y_offset);
			t_c.br.set(side1->x_offset + rect.length(), side1->y_offset + (c-f));
		}

		float z = plane_height(c_plane, x1, y1);
		float s_m = (float(c * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x1, y1, z,
			(float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height) + s_m));

		z = plane_height(c_plane, x2, y2);
		s_m = (float(c * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x2, y2, z,
			(float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height) + s_m));

		z = plane_height(f_plane, x2, y2);
		s_m = (float(f * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x2, y2, z,
			(float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height) + s_m));

		z = plane_height(f_plane, x1, y1);
		s_m = (float(f * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x1, y1, z,
			(float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height) + s_m));

		wall->tex = tex;
	}

	if (wall->part == PART_UPPER)
	{
		int f = map.sectors[side2->sector]->c_height;
		int c = map.sectors[side1->sector]->c_height;
		plane_t f_plane = sector_info[side2->sector].c_plane;
		plane_t c_plane = sector_info[side1->sector].c_plane;

		Texture* tex = get_texture(side1->tex_upper, 1);
		rect_t t_c;

		// Check for skywall
		if (map.sectors[side1->sector]->c_tex == "F_SKY1" &&
			map.sectors[side2->sector]->c_tex == "F_SKY1")
		{
			wall->light = 255;
			tex = get_texture("SKY1", 1);
		}

		if (map.lines[wall->line]->flags & LINE_UPPERUNPEGGED)
		{
			t_c.tl.set(side1->x_offset, side1->y_offset);
			t_c.br.set(side1->x_offset + rect.length(), side1->y_offset + (c-f));
		}
		else
		{
			t_c.tl.set(side1->x_offset, (tex->height + side1->y_offset) - (c-f));
			t_c.br.set(side1->x_offset + rect.length(), tex->height + side1->y_offset);
		}

		float z = plane_height(c_plane, x1, y1);
		float s_m = (float(c * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x1, y1, z,
			(float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height) + s_m));

		z = plane_height(c_plane, x2, y2);
		s_m = (float(c * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x2, y2, z,
			(float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height) + s_m));

		z = plane_height(f_plane, x2, y2);
		s_m = (float(f * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x2, y2, z,
			(float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height) + s_m));

		z = plane_height(f_plane, x1, y1);
		s_m = (float(f * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x1, y1, z,
			(float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height) + s_m));

		wall->tex = tex;
	}

	if (wall->part == PART_LOWER)
	{
		int f = map.sectors[side1->sector]->f_height;
		int c = map.sectors[side2->sector]->f_height;
		plane_t f_plane = sector_info[side1->sector].f_plane;
		plane_t c_plane = sector_info[side2->sector].f_plane;

		Texture* tex = get_texture(side1->tex_lower, 1);
		rect_t t_c;

		int h2 = map.sectors[side1->sector]->c_height
			- map.sectors[side2->sector]->f_height;

		if (map.lines[wall->line]->flags & LINE_LOWERUNPEGGED)
		{
			t_c.tl.set(side1->x_offset, h2 + side1->y_offset);
			t_c.br.set(side1->x_offset + rect.length(), h2 + side1->y_offset + (c-f));
		}
		else
		{
			t_c.tl.set(side1->x_offset, side1->y_offset);
			t_c.br.set(side1->x_offset + rect.length(), side1->y_offset + (c-f));
		}

		float z = plane_height(c_plane, x1, y1);
		float s_m = (float(c * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x1, y1, z,
			(float)t_c.x1() / (float)tex->width, ((float)t_c.y1() / (float)tex->height) + s_m));

		z = plane_height(c_plane, x2, y2);
		s_m = (float(c * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x2, y2, z,
			(float)t_c.x2() / (float)tex->width, ((float)t_c.y1() / (float)tex->height) + s_m));

		z = plane_height(f_plane, x2, y2);
		s_m = (float(f * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x2, y2, z,
			(float)t_c.x2() / (float)tex->width, ((float)t_c.y2() / (float)tex->height) + s_m));

		z = plane_height(f_plane, x1, y1);
		s_m = (float(f * SCALE_3D) - z) / (tex->height * SCALE_3D);
		wall->verts.push_back(point3_t(x1, y1, z,
			(float)t_c.x1() / (float)tex->width, ((float)t_c.y2() / (float)tex->height) + s_m));

		wall->tex = tex;
	}

	if (wall->part == PART_TRANS)
	{
		int t = min(map.sectors[side1->sector]->c_height, map.sectors[side2->sector]->c_height);
		int b = max(map.sectors[side1->sector]->f_height, map.sectors[side2->sector]->f_height);
		int rt = 0;
		int rb = 0;

		Texture* tex = get_texture(side1->tex_middle, 1);
		rect_t t_c;

		if (map.lines[wall->line]->flags & LINE_LOWERUNPEGGED)
		{
			rb = b + side1->y_offset;
			rt = rb + tex->height;
		}
		else
		{
			rt = t + side1->y_offset;
			rb = rt - tex->height;
		}

		int h = min(rt, t) - max(rb, b);

		t_c.tl.set(side1->x_offset, 0);
		t_c.br.set(side1->x_offset + rect.length(), h);

		if (rt > t)
		{
			t_c.tl.y = rt - t;
			t_c.br.y = t_c.tl.y + h;
			rt = t;
		}

		if (rb < b)
			rb = b;

		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, rt * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, rt * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, rb * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, rb * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->tex = tex;
	}
}

void setup_3d_line(int line)
{
	if (lines_3d.size() <= line)
		lines_3d.push_back(line3d_t());

	rect_t rect = map.l_getrect(line);
	line3d_t* ldat = &lines_3d[line];

	for (int a = 0; a < ldat->rects.size(); a++)
		delete ldat->rects[a];
	ldat->rects.clear();

	int floor1 = 0;
	int ceil1 = 0;
	int floor2 = 0;
	int ceil2 = 0;

	int light = 0;

	// First side
	sidedef_t *side = map.l_getside(line, 1);

	if (side)
	{
		floor1 = map.sectors[side->sector]->f_height;
		ceil1 = map.sectors[side->sector]->c_height;
		light = map.sectors[side->sector]->light;

		// Middle
		side = map.l_getside(line, 2);
		if (!side)
		{
			wallrect_t *mid = new wallrect_t();
			mid->line = line;
			mid->side = 1;
			mid->part = PART_MIDDLE;
			mid->light = light;
			setup_wallrect(mid);
			ldat->rects.push_back(mid);
		}

		if (side)
		{
			floor2 = map.sectors[side->sector]->f_height;
			ceil2 = map.sectors[side->sector]->c_height;

			// Middle trans
			if (map.l_getside(line, 1)->tex_middle != "-")
			{
				wallrect_t *mid = new wallrect_t();
				mid->line = line;
				mid->side = 1;
				mid->part = PART_TRANS;
				mid->light = light;
				setup_wallrect(mid);
				ldat->rects.push_back(mid);
			}

			// Upper
			if (ceil1 > ceil2)
			{
				wallrect_t *upper = new wallrect_t();
				upper->line = line;
				upper->side = 1;
				upper->part = PART_UPPER;
				upper->light = light;
				setup_wallrect(upper);
				ldat->rects.push_back(upper);
			}

			// Lower
			if (floor2 > floor1)
			{
				wallrect_t *lower = new wallrect_t();
				lower->line = line;
				lower->side = 1;
				lower->part = PART_LOWER;
				lower->light = light;
				setup_wallrect(lower);
				ldat->rects.push_back(lower);
			}
		}
	}

	// Second side
	side = map.l_getside(line, 2);

	if (side)
	{
		light = map.sectors[side->sector]->light;

		// Middle trans
		if (side->tex_middle != "-")
		{
			wallrect_t *mid = new wallrect_t();
			mid->line = line;
			mid->side = 2;
			mid->part = PART_TRANS;
			mid->light = light;
			setup_wallrect(mid);
			ldat->rects.push_back(mid);
		}

		// Upper
		if (ceil2 > ceil1)
		{
			wallrect_t *upper = new wallrect_t();
			upper->line = line;
			upper->side = 2;
			upper->part = PART_UPPER;
			upper->light = light;
			setup_wallrect(upper);
			ldat->rects.push_back(upper);
		}

		// Lower
		if (floor1 > floor2)
		{
			wallrect_t *lower = new wallrect_t();
			lower->line = line;
			lower->side = 2;
			lower->part = PART_LOWER;
			lower->light = light;
			setup_wallrect(lower);
			ldat->rects.push_back(lower);
		}
	}

	ldat->alpha = 255;
	//lines_3d.push_back(ldat);
}

void setup_slopes_3d(int sector)
{
	// Check lines
	for (int a = 0; a < map.n_lines; a++)
	{
		if (sector != -1 &&
			map.l_getsector1(a) != sector &&
			map.l_getsector2(a) != sector)
			continue;
		
		// Plane_Align
		if (map.zdoom && map.lines[a]->type == 181)
		{
			// Get start height and end height depending on the type of slope
			float f_start_height = 0;
			float f_end_height = 0;
			float c_start_height = 0;
			float c_end_height = 0;
			bool valid = false;
			int sector = -1;

			// Slope front side floor
			if (map.lines[a]->args[0] == 1 && map.lines[a]->side1 != -1 && map.lines[a]->side2 != -1)
			{
				f_start_height = map.sectors[map.l_getsector2(a)]->f_height * SCALE_3D;
				f_end_height = map.sectors[map.l_getsector1(a)]->f_height * SCALE_3D;
				sector = map.l_getsector1(a);
			}

			// Slope back side floor
			if (map.lines[a]->args[0] == 2 && map.lines[a]->side1 != -1 && map.lines[a]->side2 != -1)
			{
				f_start_height = map.sectors[map.l_getsector1(a)]->f_height * SCALE_3D;
				f_end_height = map.sectors[map.l_getsector2(a)]->f_height * SCALE_3D;
				sector = map.l_getsector2(a);
			}

			// Slope front side ceiling
			if (map.lines[a]->args[1] == 1 && map.lines[a]->side1 != -1 && map.lines[a]->side2 != -1)
			{
				c_start_height = map.sectors[map.l_getsector2(a)]->c_height * SCALE_3D;
				c_end_height = map.sectors[map.l_getsector1(a)]->c_height * SCALE_3D;
				sector = map.l_getsector1(a);
			}

			// Slope back side ceiling
			if (map.lines[a]->args[1] == 2 && map.lines[a]->side1 != -1 && map.lines[a]->side2 != -1)
			{
				c_start_height = map.sectors[map.l_getsector1(a)]->c_height * SCALE_3D;
				c_end_height = map.sectors[map.l_getsector2(a)]->c_height * SCALE_3D;
				sector = map.l_getsector2(a);
			}

			if (sector != -1)
			{
				bool c_sloped = false;
				bool f_sloped = false;
				point3_t c_1, c_2, c_3;
				point3_t f_1, f_2, f_3;

				// Set start point
				if (map.lines[a]->args[0] > 0)
				{
					f_1.set(map.l_getrect(a).x1() * SCALE_3D, map.l_getrect(a).y1() * SCALE_3D, f_start_height);
					f_2.set(map.l_getrect(a).x2() * SCALE_3D, map.l_getrect(a).y2() * SCALE_3D, f_start_height);
					f_sloped = true;
				}

				if (map.lines[a]->args[1] > 0)
				{
					c_1.set(map.l_getrect(a).x1() * SCALE_3D, map.l_getrect(a).y1() * SCALE_3D, c_start_height);
					c_2.set(map.l_getrect(a).x2() * SCALE_3D, map.l_getrect(a).y2() * SCALE_3D, c_start_height);
					c_sloped = true;
				}

				// Get end point
				rect_t s_line = map.l_getrect(a);
				point3_t mid(float(s_line.x1()) + ((s_line.x2() - s_line.x1()) / 2.0f),
					float(s_line.y1()) + ((s_line.y2() - s_line.y1()) / 2.0f), 0.0f);
				point3_t dir(float(s_line.y2() - s_line.y1()), -float(s_line.x2() - s_line.x1()), 0.1f);
				point3_t side(mid.x + dir.x, mid.y + dir.y, mid.z + dir.z);
				float max_dist = 0.0f;
				float min_dist = 0.0f;

				// Run through all lines in the sector and get their vertices
				//numlist_t verts;
				vector<int> verts;
				for (DWORD l2 = 0; l2 < map.n_lines; l2++)
				{
					if ((map.l_getsector1(l2) == sector ||
						map.l_getsector2(l2) == sector) &&
						l2 != a)
					{
						verts.push_back(map.lines[l2]->vertex1);
						verts.push_back(map.lines[l2]->vertex2);
					}
				}

				for (DWORD b = 0; b < verts.size(); b++)
				{
					int v = verts[b];
					float x1 = (float)map.verts[v]->x;
					float x2 = x1 + float(s_line.x2() - s_line.x1());
					float y1 = (float)map.verts[v]->y;
					float y2 = y1 + float(s_line.y2() - s_line.y1());

					if ((y2 - y1) * (side.x - mid.x) - (x2 - x1) * (side.y - mid.y) != 0.0f)
					{
						float dist = ((x2 - x1) * (mid.y - y1) - (y2 - y1) * (mid.x - (x1))) /
							((y2 - y1) * (side.x - mid.x) - (x2 - x1) * (side.y - mid.y));

						if (dist > max_dist)
							max_dist = dist;

						if (dist < min_dist)
							min_dist = dist;
					}
				}

				// Get 2 intersection points: max for side1 and min for side2
				float max_x = float(mid.x) + (float(dir.x) * max_dist);
				float max_y = float(mid.y) + (float(dir.y) * max_dist);
				float min_x = float(mid.x) + (float(dir.x) * min_dist);
				float min_y = float(mid.y) + (float(dir.y) * min_dist);

				if (map.lines[a]->args[0] == 1)
					f_3.set(max_x * SCALE_3D, max_y * SCALE_3D, f_end_height);

				if (map.lines[a]->args[0] == 2)
					f_3.set(min_x * SCALE_3D, min_y * SCALE_3D, f_end_height);

				if (map.lines[a]->args[1] == 1)
					c_3.set(max_x * SCALE_3D, max_y * SCALE_3D, c_end_height);

				if (map.lines[a]->args[1] == 2)
					c_3.set(min_x * SCALE_3D, min_y * SCALE_3D, c_end_height);

				if (f_sloped)
					sector_info[sector].f_plane.from_triangle(f_3, f_1, f_2);

				if (c_sloped)
					sector_info[sector].c_plane.from_triangle(c_3, c_1, c_2);
			}
		}
	}

	// Check things
	for (int t = 0; t < map.n_things; t++)
	{
		// 'Slope floor to here' thing
		if (map.things[t]->type == 9500)
		{
			// Get a list of lines with associated lineid
			vector<int> list;
			map.l_getfromid(map.things[t]->args[0], &list);

			for (int a = 0; a < list.size(); a++)
			{
				int line = list[a];
				point3_t p1, p2, p3;
				float x, y, z;
				int s;

				if (determine_line_side(line, map.things[t]->x, map.things[t]->y))
					s = map.l_getsector1(line);
				else
					s = map.l_getsector2(line);

				if (sector != -1 && sector != s)
					continue;

				if (s != -1)
				{
					rect_t rect = map.l_getrect(line);

					// Get line endpoints
					x = rect.x1() * SCALE_3D;
					y = rect.y1() * SCALE_3D;
					z = plane_height(sector_info[s].f_plane, x, y);
					p2.set(x, y, z);

					x = rect.x2() * SCALE_3D;
					y = rect.y2() * SCALE_3D;
					z = plane_height(sector_info[s].f_plane, x, y);
					p3.set(x, y, z);

					// Get point to slope to
					x = map.things[t]->x * SCALE_3D;
					y = map.things[t]->y * SCALE_3D;
					//sector = determine_sector(x / scale, y / scale);
					z = p2.z + (map.things[t]->z * SCALE_3D);
					p1.set(x, y, z);

					sector_info[s].f_plane.from_triangle(p1, p2, p3);
				}
			}
		}

		// 'Slope ceiling to here' thing
		if (map.things[t]->type == 9501)
		{
			// Get a list of lines with associated lineid
			//numlist_t list;
			vector<int> list;
			map.l_getfromid(map.things[t]->args[0], &list);

			for (int a = 0; a < list.size(); a++)
			{
				int line = list[a];
				point3_t p1, p2, p3;
				float x, y, z;
				int s;

				// Get line endpoints
				if (determine_line_side(line, map.things[t]->x, map.things[t]->y))
					s = map.l_getsector1(line);
				else
					s = map.l_getsector2(line);

				if (sector != -1 && sector != s)
					continue;

				if (s != -1)
				{
					rect_t rect = map.l_getrect(line);

					// Get line endpoints
					x = rect.x1() * SCALE_3D;
					y = rect.y1() * SCALE_3D;
					z = plane_height(sector_info[s].c_plane, x, y);
					p2.set(x, y, z);

					x = rect.x2() * SCALE_3D;
					y = rect.y2() * SCALE_3D;
					z = plane_height(sector_info[s].c_plane, x, y);
					p3.set(x, y, z);

					// Get point to slope to
					x = map.things[t]->x * SCALE_3D;
					y = map.things[t]->y * SCALE_3D;
					//sector = determine_sector(x / scale, y / scale);
					z = (map.things[t]->z * SCALE_3D) - p2.z;
					p1.set(x, y, z);

					sector_info[s].c_plane.from_triangle(p1, p2, p3);
				}
			}
		}
	}
}


void setup_specials_3d(int sector)
{
	for (int a = 0; a < map.n_lines; a++)
	{
		if (sector != -1 &&
			map.l_getsector1(a) != sector &&
			map.l_getsector2(a) != sector)
			continue;

		// Translucentline
		if (map.zdoom && map.lines[a]->type == 208)
		{
			if (map.lines[a]->args[0] != 0)
			{
				vector<int> lines;
				map.l_getfromid(map.lines[a]->args[0], &lines);

				for (int b = 0; b < lines.size(); b++)
					lines_3d[b].alpha = map.lines[a]->args[1];
			}
			else
				lines_3d[a].alpha = map.lines[a]->args[1];
		}
	}
}

// setup_sector: Sets up a sector
// --------------------------- >>
void setup_sector(int s)
{
	sector_info[s].f_plane.set(0.0f, 0.0f, 1.0f, map.sectors[s]->f_height * SCALE_3D);
	sector_info[s].c_plane.set(0.0f, 0.0f, 1.0f, map.sectors[s]->c_height * SCALE_3D);
}

void setup_flatpoly(flatpoly_t *poly, int ssector)
{
	poly->verts.clear();
	int startseg = gl_ssects[ssector].startseg;
	int endseg = startseg + gl_ssects[ssector].n_segs;

	// Get parent sector & what segs are lines
	if (poly->parent_sector == -1)
	{
		for (DWORD s = startseg; s < endseg; s++)
		{
			// If the seg runs along a line
			if (gl_segs[s].line != -1)
			{
				int sector = 0;

				if (gl_segs[s].flags & SEG_FLAGS_SIDE)
					sector = map.l_getsector2(gl_segs[s].line);
				else
					sector = map.l_getsector1(gl_segs[s].line);

				poly->parent_sector = sector;
				poly->lines.push_back(true);
			}
			else
				poly->lines.push_back(false);
		}
	}

	Texture* tex;
	if (poly->part == PART_FLOOR)
		tex = get_texture(map.sectors[poly->parent_sector]->f_tex, 2);
	else
		tex = get_texture(map.sectors[poly->parent_sector]->c_tex, 2);

	if (tex->name != "F_SKY1")
	{
		poly->tex = tex->get_gl_id();
		poly->light = map.sectors[poly->parent_sector]->light;
	}
	else
	{
		tex = get_texture("SKY1", 1);
		poly->light = 255;
		poly->tex = tex->get_gl_id();
	}

	float x, y, z, u, v;
	plane_t p;

	if (poly->part == PART_FLOOR)
		p = sector_info[poly->parent_sector].f_plane;
	else
		p = sector_info[poly->parent_sector].c_plane;

	// Add first vertex
	int vert = gl_segs[startseg].vertex1;
	if (vert & SEG_GLVERTEX)
	{
		x = gl_verts[vert & ~SEG_GLVERTEX].x * SCALE_3D;
		y = gl_verts[vert & ~SEG_GLVERTEX].y * SCALE_3D;
	}
	else
	{
		x = map.verts[vert]->x * SCALE_3D;
		y = map.verts[vert]->y * SCALE_3D;
	}

	/*
	if (poly->part == PART_FLOOR)
		z = map.sectors[poly->parent_sector]->f_height * SCALE_3D;
	else
		z = map.sectors[poly->parent_sector]->c_height * SCALE_3D;
		*/
	z = plane_height(p, x, y);

	u = (x / SCALE_3D) * (1.0f / tex->width);
	v = (-y / SCALE_3D) * (1.0f / tex->height);

	poly->verts.push_back(point3_t(x, y, z, u, v));
	//log_message("%1.2f %1.2f %1.2f\n", x, y, z);

	// Add each seg's second vertex
	for (DWORD s = startseg; s < endseg; s++)
	{
		vert = gl_segs[s].vertex2;

		if (vert & SEG_GLVERTEX)
		{
			x = gl_verts[vert & ~SEG_GLVERTEX].x * SCALE_3D;
			y = gl_verts[vert & ~SEG_GLVERTEX].y * SCALE_3D;
		}
		else
		{
			x = map.verts[vert]->x * SCALE_3D;
			y = map.verts[vert]->y * SCALE_3D;
		}

		z = plane_height(p, x, y);

		u = (x / SCALE_3D) * (1.0f / tex->width);
		v = (-y / SCALE_3D) * (1.0f / tex->height);
		
		poly->verts.push_back(point3_t(x, y, z, u, v));
		//log_message("%1.2f %1.2f %1.2f\n", x, y, z);
	}

	//log_message("\n");
}

void setup_ssector(int ssect)
{
	ssect3d_t ssinfo;

	// Setup floor
	flatpoly_t *poly = new flatpoly_t();
	//poly->ssector = ssect;
	poly->part = PART_FLOOR;
	setup_flatpoly(poly, ssects_3d.size());
	ssinfo.flats.push_back(poly);

	// Setup ceiling
	poly = new flatpoly_t();
	//poly->ssector = ssect;
	poly->part = PART_CEIL;
	setup_flatpoly(poly, ssects_3d.size());
	ssinfo.flats.push_back(poly);

	ssects_3d.push_back(ssinfo);
}

void setup_3d_things()
{
	for (int a = 0; a < map.n_things; a++)
	{
		thing3d_t* t = new thing3d_t();
		t->parent_sector = determine_sector(map.things[a]->x, map.things[a]->y);
		t->sprite = get_texture(map.things[a]->ttype->spritename, 3);
	}
}

void setup_3d_data()
{
	if (map.changed & MC_NODE_REBUILD)
	{
		// Build gl nodes
		splash("Building GL Nodes");
		build_gl_nodes();

		map.changed = (map.changed & ~MC_NODE_REBUILD);
	}

	// Init lines
	if (map.changed & MC_LINES)
	{
		while (wallrects.size() != 0)
			delete wallrects[0];

		lines_3d.clear();
	}

	if (map.changed & MC_SSECTS)
	{
		ssects_3d.clear();

		// Init flats
		for (int a = 0; a < flatpolys.size(); a++)
			delete flatpolys[a];
		flatpolys.clear();
	}

	if (map.changed & MC_THINGS)
	{
		// Init things
		for (int a = 0; a < things_3d.size(); a++)
			delete things_3d[a];
		things_3d.clear();
	}

	// Setup sectors
	splash("Setup Sectors");
	sector_info = (sectinfo_t *)realloc(sector_info, map.n_sectors * sizeof(sectinfo_t));
	for (int a = 0; a < map.n_sectors; a++)
		setup_sector(a);

	setup_slopes_3d();

	if (map.changed & MC_LINES)
	{
		// Setup walls
		splash("Setup Walls");
		for (int a = 0; a < map.n_lines; a++)
		{
			if (a % 10 == 0)
				splash_progress((double)a / (double)map.n_lines);

			setup_3d_line(a);
		}

		map.changed = (map.changed & ~MC_LINES);
	}

	if (map.changed & MC_SSECTS)
	{
		// Setup subsectors
		splash("Setup Flats");
		for (int a = 0; a < n_gl_ssects; a++)
		{
			if (a % 10 == 0)
				splash_progress((double)a / (double)n_gl_ssects);

			setup_ssector(a);
		}

		map.changed = (map.changed & ~MC_SSECTS);
	}

	// Setup specials
	setup_specials_3d();

	if (map.changed & MC_THINGS)
	{
		// Setup things
		splash("Setup Things");
		setup_3d_things();

		map.changed = (map.changed & ~MC_THINGS);
	}

	splash_hide();

	map_changed = 1;
}
