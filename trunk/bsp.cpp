
#include "main.h"
#include "struct_3d.h"
#include "camera.h"
#include "map.h"
#include "render.h"
#include "bsp.h"
#include "mathstuff.h"

gl_vertex_t	*gl_verts = NULL;
gl_seg_t	*gl_segs = NULL;
gl_ssect_t	*gl_ssects = NULL;
gl_node_t	*gl_nodes = NULL;
DWORD		n_gl_verts = 0;
DWORD		n_gl_segs = 0;
DWORD		n_gl_ssects = 0;

int n_gl_nodes = 0;

//bool *vis_lines = 0;
//bool *vis_ssects = 0;

// I have to do this because for some reason dynamically allocating this
// was causing some very bad problems. I'll work it out later :P
//bool vis_lines[65535];
//bool vis_ssects[65535];

BYTE vis_buffer[3600];

extern Map map;
extern Camera camera;
extern sectinfo_t *sector_info;

extern vector<line3d_t> lines_3d;
extern vector<ssect3d_t> ssects_3d;

// build_gl_nodes: Builds and loads in GL node data for the current map
// ----------------------------------------------------------------- >>
void build_gl_nodes()
{
	FILE *fp;
	int current_lump = 0;
	int unit_size = 0;
	int unit_count = 0;

	// Save map and build nodes
	Wad tempwad;
	tempwad.path = "sladetemp.wad";
	map.add_to_wad(&tempwad);
	tempwad.save(false);

#ifdef WIN32
	system("glbsp -v3 sladetemp.wad");
#else
	system("./glbsp -v3 sladetemp.wad");
#endif
	remove("sladetemp.wad.bak");

	// Open GWA file
	Wad wad;
	
	if (!wad.open("sladetemp.gwa"))
		printf("Failed to build GL nodes!\n");

	// << ---------------------- >>
	// << -- Load GL vertices -- >>
	// << ---------------------- >>
	current_lump = wad.get_lump_index("GL_VERT");

	if (current_lump == -1)
	{
		print(true, "Gwa has no GL_VERT lump.\n");
		return;
	}
	
	// Setup sizes
	unit_size = sizeof(gl_vertex_t);
	unit_count = (wad.directory[current_lump]->Size() - 4) / unit_size;
	n_gl_verts = unit_count;
	
	// Setup array
	gl_verts = (gl_vertex_t *)realloc(gl_verts, unit_size * unit_count);
	
	// Read vertices from file
	int temp;

	fp = fopen(wad.path.c_str(), "rb");
	fseek(fp, wad.directory[current_lump]->Offset() + 4, SEEK_SET);
	for (DWORD v = 0; v < n_gl_verts; v++)
	{
		fread(&temp, 4, 1, fp);
		gl_verts[v].x = (temp >> 16);
		fread(&temp, 4, 1, fp);
		gl_verts[v].y = (temp >> 16);
	}

	// << ------------------ >>
	// << -- Load GL segs -- >>
	// << ------------------ >>
	current_lump = wad.get_lump_index("GL_SEGS");

	if (current_lump == -1)
	{
		print(true, "Gwa has no GL_SEGS lump.\n");
		return;
	}
	
	// Setup sizes
	unit_size = sizeof(gl_seg_t);
	unit_count = (wad.directory[current_lump]->Size() - 4) / unit_size;
	n_gl_segs = unit_count;
	
	// Setup array
	gl_segs = (gl_seg_t *)realloc(gl_segs, unit_size * unit_count);
	
	fseek(fp, wad.directory[current_lump]->Offset() + 4, SEEK_SET);
	fread(gl_segs, unit_size, unit_count, fp);

	//for (int o = 0; o < unit_count; o++)
		//print(true, "%d %d\n", gl_segs[o].vertex1, gl_segs[o].vertex2);

	// << ---------------------- >>
	// << -- Load GL ssectors -- >>
	// << ---------------------- >>
	current_lump = wad.get_lump_index("GL_SSECT");

	if (current_lump == -1)
	{
		print(true, "Gwa has no GL_SSECT lump.\n");
		return;
	}
	
	// Setup sizes
	unit_size = sizeof(gl_ssect_t);
	unit_count = (wad.directory[current_lump]->Size() - 4) / unit_size;
	n_gl_ssects = unit_count;
	
	// Setup array
	gl_ssects = (gl_ssect_t *)realloc(gl_ssects, unit_size * unit_count);
	
	fseek(fp, wad.directory[current_lump]->Offset() + 4, SEEK_SET);
	fread(gl_ssects, unit_size, unit_count, fp);

	// << ------------------- >>
	// << -- Load GL nodes -- >>
	// << ------------------- >>
	current_lump = wad.get_lump_index("GL_NODES");

	if (current_lump == -1)
	{
		print(true, "Gwa has no GL_NODES lump.\n");
		return;
	}
	
	// Setup sizes
	unit_size = sizeof(gl_node_t);
	unit_count = (wad.directory[current_lump]->Size()) / unit_size;
	n_gl_nodes = unit_count;
	
	// Setup array
	gl_nodes = (gl_node_t *)realloc(gl_nodes, unit_size * unit_count);
	
	fseek(fp, wad.directory[current_lump]->Offset(), SEEK_SET);
	for (int n = 0; n < n_gl_nodes; n++)
	{
		fread(&gl_nodes[n].x, 2, 1, fp);
		fread(&gl_nodes[n].y, 2, 1, fp);
		fread(&gl_nodes[n].dx, 2, 1, fp);
		fread(&gl_nodes[n].dy, 2, 1, fp);
		fread(gl_nodes[n].right_bbox, 2, 4, fp);
		fread(gl_nodes[n].left_bbox, 2, 4, fp);
		fread(&gl_nodes[n].right_child, 2, 1, fp);
		fread(&gl_nodes[n].left_child, 2, 1, fp);
	}

	fclose(fp);
}

void clear_visibility()
{
	for (int a = 0; a < map.n_lines; a++)
		lines_3d[a].visible = false;

	for (int a = 0; a < n_gl_ssects; a++)
		ssects_3d[a].visible = false;

	for (int a = 0; a < map.n_sectors; a++)
		sector_info[a].visible = false;
}

void set_visbuffer(int blocked)
{
	for (int a = 0; a < 3600; a++)
		vis_buffer[a] = blocked;
}

void open_view()
{
	int mlook_mod = (int)(((camera.view.z - camera.position.z) * 3000.0f));
	//int mlook_mod = 0;

	if (mlook_mod < 0)
		mlook_mod = -mlook_mod;

	if (mlook_mod > 3000)
		mlook_mod = 3000;

	for (int a = 0; a < 600 + mlook_mod; a++)
		vis_buffer[a] = 0;

	for (int a = 3000 - mlook_mod; a < 3600; a++)
		vis_buffer[a] = 0;

	//memset(vis_buffer, 0, 3600);
}

bool seg_is_visible(float x1, float y1, float x2, float y2)
{
	// Check with camera
	point3_t strafe = camera.position + camera.strafe;
	if (determine_line_side(camera.position.x, camera.position.y, strafe.x, strafe.y, x1, y1)
		&& determine_line_side(camera.position.x, camera.position.y, strafe.x, strafe.y, x2, y2))
		return false;

	point3_t vec1(x1 - camera.position.x, y1 - camera.position.y, 0.0f);
	point3_t vec2(x2 - camera.position.x, y2 - camera.position.y, 0.0f);
	vec1 = vec1.normalize();
	vec2 = vec2.normalize();

	unsigned short a1 = (short)(get_2d_angle(camera.view - camera.position, vec1) * 10.0f);
	unsigned short a2 = (short)(get_2d_angle(camera.view - camera.position, vec2) * 10.0f);

	if (a2 > 3600)
		a2 = 3600;
	if (a1 > 3600)
		a1 = 3600;

	// A cheap hack for now until I can figure out why sometimes I get 'backwards' segs
	if (a1 > a2 && a1 - a2 < 450)
	{
		unsigned short temp = a1;
		a1 = a2;
		a2 = temp;
	}

	if (a1 > a2)
	{
		for (unsigned short a = a1; a < 3600; a++)
		{
			if (vis_buffer[a] == 0)
				return true;
		}

		for (unsigned short a = 0; a < a2; a++)
		{
			if (vis_buffer[a] == 0)
				return true;
		}
	}
	else
	{
		for (unsigned short a = a1; a < a2; a++)
		{
			if (vis_buffer[a] == 0)
				return true;
		}
	}

	return false;
}

void block_seg(float x1, float y1, float x2, float y2)
{
	point3_t vec1(x1 - camera.position.x, y1 - camera.position.y, 0.0f);
	point3_t vec2(x2 - camera.position.x, y2 - camera.position.y, 0.0f);
	vec1 = vec1.normalize();
	vec2 = vec2.normalize();

	unsigned short a1 = (short)(get_2d_angle(camera.view - camera.position, vec1) * 10.0f);
	unsigned short a2 = (short)(get_2d_angle(camera.view - camera.position, vec2) * 10.0f);

	if (a2 > 3600)
		a2 = 3600;
	if (a1 > 3600)
		a1 = 3600;

	// A cheap hack for now until I can figure out why sometimes I get 'backwards' segs
	if (a1 > a2 && a1 - a2 < 450)
	{
		unsigned short temp = a1;
		a1 = a2;
		a2 = temp;
	}

	//log_message("seg from (%1.2f, %1.2f) to (%1.2f, %1.2f)\n", x1, y1, x2, y2);
	//log_message("block %d to %d\n", a1, a2);

	if (a1 > a2)
	{
		for (unsigned short a = a1; a < 3600; a++)
			vis_buffer[a] = 1;

		for (unsigned short a = 0; a < a2; a++)
			vis_buffer[a] = 1;
	}
	else
	{
		for (unsigned short a = a1; a < a2; a++)
			vis_buffer[a] = 1;
	}
}

bool view_buffer_full()
{
	if (memchr(vis_buffer, 0, 3600) == NULL)
		return true;
	else
		return false;
}

void process_subsector(short subsect)
{
	bool visible = false;
	int sector = -1;

	// Cycle through segs
	int start = gl_ssects[subsect].startseg;
	int end = start + gl_ssects[subsect].n_segs;

	for (int s = start; s < end; s++)
	{
		// Get x1,y1,x2,y2 of seg
		float x1 = 0.0f;
		float y1 = 0.0f;
		float x2 = 0.0f;
		float y2 = 0.0f;
		long v = gl_segs[s].vertex1;

		if (v & SEG_GLVERTEX)
		{
			x1 = gl_verts[v & ~SEG_GLVERTEX].x * SCALE_3D;
			y1 = gl_verts[v & ~SEG_GLVERTEX].y * SCALE_3D;
		}
		else
		{
			x1 = map.verts[v]->x * SCALE_3D;
			y1 = map.verts[v]->y * SCALE_3D;
		}

		v = gl_segs[s].vertex2;

		if (v & SEG_GLVERTEX)
		{
			x2 = gl_verts[v & ~SEG_GLVERTEX].x * SCALE_3D;
			y2 = gl_verts[v & ~SEG_GLVERTEX].y * SCALE_3D;
		}
		else
		{
			x2 = map.verts[v]->x * SCALE_3D;
			y2 = map.verts[v]->y * SCALE_3D;
		}

		// If seg runs along a line
		if (gl_segs[s].line != SEG_MINISEG && gl_segs[s].line >= 0 && gl_segs[s].line < lines_3d.size())
		{
			bool side = true;

			if (gl_segs[s].flags & SEG_FLAGS_SIDE)
				side = false;

			// If we're on the right side of that line
			if (determine_line_side(gl_segs[s].line, camera.position.x / SCALE_3D, camera.position.y / SCALE_3D) == side)
			{
				// If the seg isn't blocked
				if (seg_is_visible(x1, y1, x2, y2))
				{
					// Block the seg if the line is 1-sided
					if (map.lines[gl_segs[s].line]->side2 == -1 && side)
						block_seg(x1, y1, x2, y2);
					else
					{
						// Block the seg if it's floor and ceiling are equal or overlapping
						if (side)
							sector = map.l_getsector2(gl_segs[s].line);
						else
							sector = map.l_getsector1(gl_segs[s].line);

						if (sector != -1)
						{
							if (map.sectors[sector]->f_height >= map.sectors[sector]->c_height)
								block_seg(x1, y1, x2, y2);
						}
					}

					// The line and subsector are visible
					int line = gl_segs[s].line;
					lines_3d[line].visible = true;
					visible = true;
				}
			}
		}
		else // Seg doesn't run along a line
		{
			// If we're on the right side of the seg
			if (determine_line_side(x1, y1, x2, y2, camera.position.x, camera.position.y))
			{
				// If the seg isn't blocked, the ssector is visible
				if (seg_is_visible(x1, y1, x2, y2))
					visible = true;
			}
		}
	}

	if (visible)
	{
		if (sector != -1)
			sector_info[sector].visible = true;
		
		ssects_3d[subsect].visible = true;
	}
}

// walk_bsp_tree: Walks through the gl nodes (bsp tree) to determine visibility
// ------------------------------------------------------------------------- >>
void walk_bsp_tree(unsigned short node)
{
	// If we have a subsector
	if (node & CHILD_IS_SUBSEC)
	{
		short subsect = node & ~CHILD_IS_SUBSEC;
		process_subsector(subsect);

		return;
	}

	if (view_buffer_full())
		return;

	int x1 = gl_nodes[node].x;
	int y1 = gl_nodes[node].y;
	int x2 = x1 + gl_nodes[node].dx;
	int y2 = y1 + gl_nodes[node].dy;

	rect_t part(x1, y1, x2, y2);

	if (determine_line_side_f(part, camera.position.x * 100.0f, camera.position.y * 100.0f) > 0)
	{
		walk_bsp_tree(gl_nodes[node].right_child);
		walk_bsp_tree(gl_nodes[node].left_child);
	}
	else if (determine_line_side_f(part, camera.position.x * 100.0f, camera.position.y * 100.0f) < 0)
	{
		walk_bsp_tree(gl_nodes[node].left_child);
		walk_bsp_tree(gl_nodes[node].right_child);
	}
	else
	{
		if (determine_line_side(part, camera.view.x * 100.0f, camera.view.y * 100.0f))
		{
			walk_bsp_tree(gl_nodes[node].right_child);
			walk_bsp_tree(gl_nodes[node].left_child);
		}
		else
		{
			walk_bsp_tree(gl_nodes[node].left_child);
			walk_bsp_tree(gl_nodes[node].right_child);
		}
	}
}

// determine_seg_side: Determines the side of a seg a point is on
// ----------------------------------------------------------- >>
bool determine_seg_side(DWORD s, float x, float y)
{
	// Get x1,y1,x2,y2 of seg
	float x1 = 0.0f;
	float y1 = 0.0f;
	float x2 = 0.0f;
	float y2 = 0.0f;
	long v = gl_segs[s].vertex1;

	if (v & SEG_GLVERTEX)
	{
		x1 = gl_verts[v & ~SEG_GLVERTEX].x * SCALE_3D;
		y1 = gl_verts[v & ~SEG_GLVERTEX].y * SCALE_3D;
	}
	else
	{
		x1 = map.verts[v]->x * SCALE_3D;
		y1 = map.verts[v]->y * SCALE_3D;
	}

	v = gl_segs[s].vertex2;

	if (v & SEG_GLVERTEX)
	{
		x2 = gl_verts[v & ~SEG_GLVERTEX].x * SCALE_3D;
		y2 = gl_verts[v & ~SEG_GLVERTEX].y * SCALE_3D;
	}
	else
	{
		x2 = map.verts[v]->x * SCALE_3D;
		y2 = map.verts[v]->y * SCALE_3D;
	}

	if (determine_line_side(x1, y1, x2, y2, x, y))
		return true;
	else
		return false;
}
