
#include "main.h"
#include "struct_3d.h"
#include "camera.h"
#include "map.h"
#include "bsp.h"
#include "render.h"
#include "draw.h"

struct sectinfo_t
{
	plane_t	f_plane;
	plane_t	c_plane;
	//int light;
	Texture *f_tex;
	Texture *c_tex;
};

sectinfo_t	*sector_info;

Camera camera;
vector<wallrect_t*> wallrects;
vector<flatpoly_t*> flatpolys;

CVAR(Bool, render_wireframe, false, CVAR_SAVE)

extern Map map;
//extern bool *vis_lines;
//extern bool *vis_ssects;
extern bool vis_lines[65535];
extern bool vis_ssects[65535];
extern int n_gl_nodes;
extern GtkWidget *draw_3d_area;

extern gl_vertex_t	*gl_verts;
extern gl_seg_t		*gl_segs;
extern gl_ssect_t	*gl_ssects;
extern gl_node_t	*gl_nodes;
extern DWORD		n_gl_verts;
extern DWORD		n_gl_segs;
extern DWORD		n_gl_ssects;

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

flatpoly_t::flatpoly_t()
{
	flatpolys.push_back(this);
	this->parent_sector = -1;
	this->tex = NULL;
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

void setup_wallrect(wallrect_t *wall)
{
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

	if (wall->sector == -1)
		wall->sector = side1->sector;

	if (wall->part == PART_MIDDLE)
	{
		int f = map.sectors[side1->sector]->f_height;
		int c = map.sectors[side1->sector]->c_height;
		int l = map.sectors[side1->sector]->light;

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

		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, c * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, c * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, f * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, f * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->col.set(l, l, l, 255);
		wall->tex = tex;
	}

	if (wall->part == PART_UPPER)
	{
		int f = map.sectors[side2->sector]->c_height;
		int c = map.sectors[side1->sector]->c_height;
		int l = map.sectors[side1->sector]->light;

		Texture* tex = get_texture(side1->tex_upper, 1);
		rect_t t_c;

		// Check for skywall
		if (map.sectors[side1->sector]->c_tex == "F_SKY1" &&
			map.sectors[side2->sector]->c_tex == "F_SKY1")
		{
			wall->light = 255;
			l = 255;
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

		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, c * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, c * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, f * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, f * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->col.set(l, l, l, 255);
		wall->tex = tex;
	}

	if (wall->part == PART_LOWER)
	{
		int f = map.sectors[side1->sector]->f_height;
		int c = map.sectors[side2->sector]->f_height;
		int l = map.sectors[side1->sector]->light;

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

		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, c * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, c * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y1() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, f * SCALE_3D,
			(float)t_c.x2() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, f * SCALE_3D,
			(float)t_c.x1() / (float)tex->width, (float)t_c.y2() / (float)tex->height));
		wall->col.set(l, l, l, 255);
		wall->tex = tex;
	}

	if (wall->part == PART_TRANS)
	{
		int l = map.sectors[side1->sector]->light;
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
		wall->col.set(l, l, l, 255);
		wall->tex = tex;
	}
}

void setup_3d_line(int line)
{
	rect_t rect = map.l_getrect(line);

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
		}
	}
}

// setup_sector: Sets up a sector
// --------------------------- >>
void setup_sector(int s)
{
	string skytex = "RSKY1"; // For now

	if (map.sectors[s]->f_tex == "F_SKY1")
		sector_info[s].f_tex = get_texture(skytex, 1);
	else
		sector_info[s].f_tex = get_texture(map.sectors[s]->f_tex, 2);

	if (map.sectors[s]->c_tex == "F_SKY1")
		sector_info[s].c_tex = get_texture(skytex, 1);
	else
		sector_info[s].c_tex = get_texture(map.sectors[s]->c_tex, 2);

	//sector_info[s].light = map.sectors[s]->light;
	sector_info[s].f_plane.set(0.0f, 0.0f, 1.0f, map.sectors[s]->f_height * SCALE_3D);
	sector_info[s].c_plane.set(0.0f, 0.0f, 1.0f, map.sectors[s]->c_height * SCALE_3D);
}

void setup_flatpoly(flatpoly_t *poly)
{
	int startseg = gl_ssects[poly->ssector].startseg;
	int endseg = startseg + gl_ssects[poly->ssector].n_segs;

	// Get parent sector
	if (poly->parent_sector == -1)
	{
		for (DWORD s = startseg; s < endseg; s++)
		{
			// If the seg runs along a line
			if (gl_segs[s].line != SEG_MINISEG)
			{
				int sector = 0;

				if (gl_segs[s].flags & SEG_FLAGS_SIDE)
					sector = map.l_getsector2(gl_segs[s].line);
				else
					sector = map.l_getsector1(gl_segs[s].line);

				poly->parent_sector = sector;
				break;
			}
		}
	}

	string tex;
	if (poly->part == PART_FLOOR)
		tex = map.sectors[poly->parent_sector]->f_tex;
	else
		tex = map.sectors[poly->parent_sector]->c_tex;

	if (tex != "F_SKY1")
	{
		poly->col = rgba_t(map.sectors[poly->parent_sector]->light,
							map.sectors[poly->parent_sector]->light,
							map.sectors[poly->parent_sector]->light,
							255);
		poly->tex = get_texture(tex, 2);
		poly->light = map.sectors[poly->parent_sector]->light;
	}
	else
	{
		tex = "SKY1";
		poly->col = rgba_t(255, 255, 255, 255);
		poly->light = 255;
		poly->tex = get_texture(tex, 1);
	}

	

	float x, y, z, u, v;

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

	if (poly->part == PART_FLOOR)
		z = map.sectors[poly->parent_sector]->f_height * SCALE_3D;
	else
		z = map.sectors[poly->parent_sector]->c_height * SCALE_3D;

	u = (x / SCALE_3D) * (1.0f / poly->tex->width);
	v = (-y / SCALE_3D) * (1.0f / poly->tex->height);

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

		u = (x / SCALE_3D) * (1.0f / poly->tex->width);
		v = (-y / SCALE_3D) * (1.0f / poly->tex->height);
		
		poly->verts.push_back(point3_t(x, y, z, u, v));
		//log_message("%1.2f %1.2f %1.2f\n", x, y, z);
	}

	//log_message("\n");
}

void setup_ssector(int ssect)
{
	// Setup floor
	flatpoly_t *poly = new flatpoly_t();
	poly->ssector = ssect;
	poly->part = PART_FLOOR;
	setup_flatpoly(poly);

	// Setup ceiling
	poly = new flatpoly_t();
	poly->ssector = ssect;
	poly->part = PART_CEIL;
	setup_flatpoly(poly);
}

void setup_3d_data()
{
	// Init wallrects
	for (int a = 0; a < wallrects.size(); a++)
		delete wallrects[a];
	wallrects.clear();

	// Init flats
	for (int a = 0; a < flatpolys.size(); a++)
		delete flatpolys[a];
	flatpolys.clear();

	/*
	// Init visible lines
	if (vis_lines)
	{
		free(vis_lines);
		vis_lines = NULL;
	}
	vis_lines = (bool*)malloc(map.n_lines);

	// Init visible subsectors
	if (vis_ssects)
	{
		free(vis_ssects);
		vis_ssects = NULL;
	}
	vis_ssects = (bool*)malloc(n_gl_ssects);
	*/
	//vis_lines = (bool*)malloc(map.n_lines);
	//vis_ssects = (bool*)malloc(n_gl_ssects);

	// Init camera
	camera.position_camera(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	for (int a = 0; a < map.n_things; a++)
	{
		if (map.things[a]->type == 1)
		{
			camera.position_camera(map.things[a]->x * SCALE_3D, map.things[a]->y * SCALE_3D, 0.0f,
									1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f);
			break;
		}
	}

	// Setup sectors
	sector_info = (sectinfo_t *)realloc(sector_info, map.n_sectors * sizeof(sectinfo_t));
	for (int a = 0; a < map.n_sectors; a++)
		setup_sector(a);

	// Build gl nodes
	build_gl_nodes();

	// Setup walls
	for (int a = 0; a < map.n_lines; a++)
		setup_3d_line(a);

	// Setup subsectors
	for (int a = 0; a < n_gl_ssects; a++)
		setup_ssector(a);
}
