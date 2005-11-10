
#include "main.h"
#include "struct_3d.h"
#include "input.h"
#include "keybind.h"
#include "render.h"
#include "camera.h"
#include "edit.h"
#include "map.h"

float grav = 0.01f;

int hl_3d_type;
int hl_3d_item;

extern Camera camera;
extern Map map;

extern sectinfo_t *sector_info;


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

