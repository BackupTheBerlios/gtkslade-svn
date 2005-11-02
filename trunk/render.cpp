
#include "main.h"
#include "camera.h"
#include "map.h"
#include "render.h"
#include "bsp.h"
#include "textures.h"

struct sectinfo_t
{
	plane_t	f_plane;
	plane_t	c_plane;
	int light;
	Texture *f_tex;
	Texture *c_tex;
};

sectinfo_t	*sector_info;

Camera camera;
vector<wallrect_t*> wallrects;
vector<flatpoly_t*> flatpolys;

extern Map map;
extern bool *vis_lines;
extern bool *vis_ssects;
extern int n_gl_nodes;

extern gl_vertex_t	*gl_verts;
extern gl_seg_t		*gl_segs;
extern gl_ssect_t	*gl_ssects;
extern gl_node_t	*gl_nodes;
extern DWORD		n_gl_verts;
extern DWORD		n_gl_segs;
extern DWORD		n_gl_ssects;

wallrect_t::wallrect_t()
{
	wallrects.push_back(this);
}

wallrect_t::~wallrect_t()
{
}

flatpoly_t::flatpoly_t()
{
	flatpolys.push_back(this);
	this->parent_sector = -1;
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

	glCullFace(GL_FRONT);
	for (int a = 0; a < wallrects.size(); a++)
	{
		if (vis_lines[wallrects[a]->line])
			wallrects[a]->draw();
	}

	//glCullFace(GL_BACK);
	for (int a = 0; a < flatpolys.size(); a++)
	{
		if (vis_ssects[flatpolys[a]->ssector])
		{
			if (flatpolys[a]->part == PART_CEIL)
				glCullFace(GL_BACK);
			else
				glCullFace(GL_FRONT);

			flatpolys[a]->draw();
		}
	}

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3d(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glEnd();
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
			mid->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, ceil1 * SCALE_3D));
			mid->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, ceil1 * SCALE_3D));
			mid->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, floor1 * SCALE_3D));
			mid->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, floor1 * SCALE_3D));
			mid->col.set(light, light, light, 255);
		}

		if (side)
		{
			floor2 = map.sectors[side->sector]->f_height;
			ceil2 = map.sectors[side->sector]->c_height;

			// Upper
			if (ceil1 > ceil2)
			{
				wallrect_t *upper = new wallrect_t();
				upper->line = line;
				upper->side = 1;
				upper->part = PART_UPPER;
				upper->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, ceil1 * SCALE_3D));
				upper->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, ceil1 * SCALE_3D));
				upper->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, ceil2 * SCALE_3D));
				upper->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, ceil2 * SCALE_3D));
				upper->col.set(light, light, light, 255);
			}

			// Lower
			if (floor2 > floor1)
			{
				wallrect_t *lower = new wallrect_t();
				lower->line = line;
				lower->side = 1;
				lower->part = PART_LOWER;
				lower->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, floor2 * SCALE_3D));
				lower->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, floor2 * SCALE_3D));
				lower->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, floor1 * SCALE_3D));
				lower->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, floor1 * SCALE_3D));
				lower->col.set(light, light, light, 255);
			}
		}
	}

	// Second side
	side = map.l_getside(line, 2);

	if (side)
	{
		light = map.sectors[side->sector]->light;

		// Upper
		if (ceil2 > ceil1)
		{
			wallrect_t *upper = new wallrect_t();
			upper->line = line;
			upper->side = 2;
			upper->part = PART_UPPER;
			upper->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, ceil2 * SCALE_3D));
			upper->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, ceil2 * SCALE_3D));
			upper->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, ceil1 * SCALE_3D));
			upper->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, ceil1 * SCALE_3D));
			upper->col.set(light, light, light, 255);
		}

		// Lower
		if (floor1 > floor2)
		{
			wallrect_t *lower = new wallrect_t();
			lower->line = line;
			lower->side = 2;
			lower->part = PART_LOWER;
			lower->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, floor1 * SCALE_3D));
			lower->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, floor1 * SCALE_3D));
			lower->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, floor2 * SCALE_3D));
			lower->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, floor2 * SCALE_3D));
			lower->col.set(light, light, light, 255);
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

	sector_info[s].light = map.sectors[s]->light;
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

	poly->col = rgba_t(map.sectors[poly->parent_sector]->light,
						map.sectors[poly->parent_sector]->light,
						0.0f,//map.sectors[poly->parent_sector]->light,
						255);

	float x, y, z;

	// Add first vertex
	int v = gl_segs[startseg].vertex1;
	if (v & SEG_GLVERTEX)
	{
		x = gl_verts[v & ~SEG_GLVERTEX].x * SCALE_3D;
		y = gl_verts[v & ~SEG_GLVERTEX].y * SCALE_3D;
	}
	else
	{
		x = map.verts[v]->x * SCALE_3D;
		y = map.verts[v]->y * SCALE_3D;
	}

	if (poly->part == PART_FLOOR)
		z = map.sectors[poly->parent_sector]->f_height * SCALE_3D;
	else
		z = map.sectors[poly->parent_sector]->c_height * SCALE_3D;

	poly->verts.push_back(point3_t(x, y, z));
	//log_message("%1.2f %1.2f %1.2f\n", x, y, z);

	// Add each seg's second vertex
	for (DWORD s = startseg; s < endseg; s++)
	{
		v = gl_segs[s].vertex2;

		if (v & SEG_GLVERTEX)
		{
			x = gl_verts[v & ~SEG_GLVERTEX].x * SCALE_3D;
			y = gl_verts[v & ~SEG_GLVERTEX].y * SCALE_3D;
		}
		else
		{
			x = map.verts[v]->x * SCALE_3D;
			y = map.verts[v]->y * SCALE_3D;
		}
		
		poly->verts.push_back(point3_t(x, y, z));
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

	// Init visible lines
	vis_lines = (bool*)realloc(vis_lines, map.n_lines);
	//memset(vis_lines, 1, map.n_lines);

	// Init visible subsectors
	vis_ssects = (bool*)realloc(vis_ssects, n_gl_ssects);
	//memset(vis_ssects, 1, n_gl_ssects);

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
