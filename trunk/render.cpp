
#include "main.h"
#include "struct_3d.h"
#include "camera.h"
#include "map.h"
#include "render.h"

Camera camera;
vector<wallrect_t*> wallrects;

extern Map map;

wallrect_t::wallrect_t()
{
	wallrects.push_back(this);
}

wallrect_t::~wallrect_t()
{
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

	for (int a = 0; a < wallrects.size(); a++)
		wallrects[a]->draw();

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
			lower->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, floor1 * SCALE_3D));
			lower->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, floor1 * SCALE_3D));
			lower->verts.push_back(point3_t(rect.x1() * SCALE_3D, rect.y1() * SCALE_3D, floor2 * SCALE_3D));
			lower->verts.push_back(point3_t(rect.x2() * SCALE_3D, rect.y2() * SCALE_3D, floor2 * SCALE_3D));
			lower->col.set(light, light, light, 255);
		}
	}
}

void setup_3d_data()
{
	for (int a = 0; a < wallrects.size(); a++)
		delete wallrects[a];

	wallrects.clear();

	camera.position_camera(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	for (int a = 0; a < map.n_lines; a++)
		setup_3d_line(a);
}
