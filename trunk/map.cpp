// << ------------------------------------ >>
// << SLADE - SlayeR's LeetAss Doom Editor >>
// << By Simon Judd, 2004                  >>
// << ------------------------------------ >>
// << map.cpp - Map stuff                  >>
// << ------------------------------------ >>

// INCLUDES ------------------------------ >>
#include "main.h"
#include "map.h"
#include "edit.h"

// VARIABLES ----------------------------- >>
Map map;

string map_lumps[12] =
{
	"THINGS",
	"VERTEXES",
	"LINEDEFS",
	"SIDEDEFS",
	"SECTORS",
	"SEGS",
	"SSECTORS",
	"NODES",
	"BLOCKMAP",
	"REJECT",
	"SCRIPTS",
	"BEHAVIOR"
};

// EXTERNAL VARIABLES -------------------- >>

// Map::create: Clears all map data and sets map name
// ----------------------------------------------- >>
void Map::create(string mapname)
{
	name = mapname;

	lines = (linedef_t **)NULL;
	sides = (sidedef_t **)NULL;
	verts = (vertex_t **)NULL;
	sectors = (sector_t **)NULL;
	things = (thing_t **)NULL;

	if (scripts)
		delete scripts;

	if (behavior)
		delete behavior;

	scripts = new Lump(0, 0, "SCRIPTS");
	behavior = new Lump(0, 0, "BEHAVIOR");

	n_lines = n_sides = n_verts = n_sectors = n_things = 0;
	opened = true;

	init_map();
}

// Map::close: Frees all map data
// --------------------------- >>
void Map::close()
{
	if (lines)
	{
		for (DWORD i = 0; i < n_lines; i++)
			free(lines[i]);

		free(lines);
	}

	if (sides)
	{
		for (DWORD i = 0; i < n_sides; i++)
			free(sides[i]);

		free(sides);
	}

	if (sectors)
	{
		for (DWORD i = 0; i < n_sectors; i++)
			free(sectors[i]);

		free(sectors);
	}

	if (verts)
	{
		for (DWORD i = 0; i < n_verts; i++)
			free(verts[i]);

		free(verts);
	}

	if (things)
	{
		for (DWORD i = 0; i < n_things; i++)
			free(things[i]);

		free(things);
	}

	lines = (linedef_t **)NULL;
	sides = (sidedef_t **)NULL;
	verts = (vertex_t **)NULL;
	sectors = (sector_t **)NULL;
	things = (thing_t **)NULL;

	n_lines = n_sides = n_verts = n_sectors = n_things = 0;
	opened = false;
}

// Map::open: Opens a map from an open wadfile
// ---------------------------------------- >>
bool Map::open(Wad *wad, string mapname)
{
	Lump* lump = NULL;
	long offset = wad->get_lump_index(mapname);
	FILE* fp = fopen(wad->path.c_str(), "rb");
	long unit_size = 0;
	//long unit_count = 0;

	if (offset == -1)
	{
		printf("Map %s not found\n", mapname.c_str());
		return false;
	}

	// Check for BEHAVIOR lump
	if (hexen)
	{
		long index = offset;
		bool done = false;

		while (!done)
		{
			index++;

			if (index == wad->num_lumps)
				done = true;
			else if (strncmp(wad->directory[index]->Name().c_str(), "THINGS", 6) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "LINEDEFS", 8) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "SIDEDEFS", 8) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "VERTEXES", 8) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "SEGS", 4) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "SSECTORS", 8) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "NODES", 5) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "SECTORS", 7) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "REJECT", 6) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "SCRIPTS", 7) == 0 ||
				strncmp(wad->directory[index]->Name().c_str(), "BLOCKMAP", 8) == 0)
			{
				done = false;
			}
			else if (strncmp(wad->directory[index]->Name().c_str(), "BEHAVIOR", 8) == 0)
				done = true;
			else
				done = true;
		}

		if (strncmp(wad->directory[index]->Name().c_str(), "BEHAVIOR", 8) != 0)
		{
			message_box("Map has no BEHAVIOR lump\n", GTK_MESSAGE_ERROR);
			return false;
		}
	}

	if (scripts)
	{
		delete scripts;
		scripts = NULL;
	}

	if (behavior)
	{
		delete behavior;
		behavior = NULL;
	}

	name = mapname;

	// << ---- Read Vertices ---- >>
	lump = wad->get_lump("VERTEXES", offset);

	if (!lump)
	{
		printf("Map has no VERTEXES lump\n");
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	// Setup vertices array
	unit_size = 4;
	n_verts = lump->Size() / unit_size;
	verts = (vertex_t **)calloc(n_verts, sizeof(vertex_t *));

	// Read vertex data
	for (DWORD i = 0; i < n_verts; i++)
	{
		verts[i] = new vertex_t;
		fread(&verts[i]->x, 2, 1, fp);
		fread(&verts[i]->y, 2, 1, fp);
		//verts[i]->hilighted = false;
		//verts[i]->selected = false;
		//verts[i]->moving = false;
	}

	// << ---- Read Lines ---- >>
	lump = wad->get_lump("LINEDEFS", offset);

	if (!lump)
	{
		printf("Map has no LINEDEFS lump\n");
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	if (hexen)
	{
		// Setup & read hexen format lines

		// Setup lines array
		unit_size = 16;
		n_lines = lump->Size() / unit_size;
		lines = (linedef_t **)calloc(n_lines, sizeof(linedef_t *));

		// Read line data
		for (DWORD i = 0; i < n_lines; i++)
		{
			BYTE temp = 0;

			lines[i] = new linedef_t;
			fread(&lines[i]->vertex1, 2, 1, fp);
			fread(&lines[i]->vertex2, 2, 1, fp);
			fread(&lines[i]->flags, 2, 1, fp);
			fread(&temp, 1, 1, fp);
			lines[i]->type = temp;
			fread(lines[i]->args, 1, 5, fp);
			fread(&lines[i]->side1, 2, 1, fp);
			fread(&lines[i]->side2, 2, 1, fp);
		}
	}
	else
	{
		// Setup lines array
		unit_size = 14;
		n_lines = lump->Size() / unit_size;
		lines = (linedef_t **)calloc(n_lines, sizeof(linedef_t *));

		// Read line data
		for (DWORD i = 0; i < n_lines; i++)
		{
			lines[i] = new linedef_t;
			fread(&lines[i]->vertex1, 2, 1, fp);
			fread(&lines[i]->vertex2, 2, 1, fp);
			fread(&lines[i]->flags, 2, 1, fp);
			fread(&lines[i]->type, 2, 1, fp);
			fread(&lines[i]->sector_tag, 2, 1, fp);
			fread(&lines[i]->side1, 2, 1, fp);
			fread(&lines[i]->side2, 2, 1, fp);
			printf("%d %d\n", lines[i]->vertex1, lines[i]->vertex2);
		}
	}

	// << ---- Read sides ---- >>
	lump = wad->get_lump("SIDEDEFS", offset);

	if (!lump)
	{
		printf("Map has no SIDEDEFS lump\n");
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	// Setup sides array
	unit_size = 30;
	n_sides = lump->Size() / unit_size;
	sides = (sidedef_t **)calloc(n_sides, sizeof(sidedef_t *));

	// Read side data
	for (DWORD i = 0; i < n_sides; i++)
	{
		sides[i] = new sidedef_t;
		char temp[9] = "";

		fread(&sides[i]->x_offset, 2, 1, fp);
		fread(&sides[i]->y_offset, 2, 1, fp);
		
		fread(temp, 1, 8, fp);
		temp[8] = 0;
		sides[i]->tex_upper = g_ascii_strup(temp, -1);

		fread(temp, 1, 8, fp);
		temp[8] = 0;
		sides[i]->tex_lower = g_ascii_strup(temp, -1);

		fread(temp, 1, 8, fp);
		temp[8] = 0;
		sides[i]->tex_middle = g_ascii_strup(temp, -1);

		fread(&sides[i]->sector, 2, 1, fp);
	}

	// << ---- Read sectors ---- >>
	lump = wad->get_lump("SECTORS", offset);

	if (!lump)
	{
		printf("Map has no SECTORS lump\n");
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	// Setup sides array
	unit_size = 26;
	n_sectors = lump->Size() / unit_size;
	sectors = (sector_t **)calloc(n_sectors, sizeof(sector_t *));

	// Read sector data
	for (DWORD i = 0; i < n_sectors; i++)
	{
		sectors[i] = new sector_t;
		char temp[9] = "";

		fread(&sectors[i]->f_height, 2, 1, fp);
		fread(&sectors[i]->c_height, 2, 1, fp);
		
		fread(temp, 1, 8, fp);
		temp[8] = 0;
		sectors[i]->f_tex = g_ascii_strup(temp, -1);

		fread(temp, 1, 8, fp);
		temp[8] = 0;
		sectors[i]->c_tex = g_ascii_strup(temp, -1);

		fread(&sectors[i]->light, 2, 1, fp);
		fread(&sectors[i]->special, 2, 1, fp);
		fread(&sectors[i]->tag, 2, 1, fp);

		//sectors[i]->hilighted = false;
		//sectors[i]->selected = false;
		//sectors[i]->moving = false;
	}

	// << ---- Read Things ---- >>
	lump = wad->get_lump("THINGS", offset);

	if (!lump)
	{
		printf("Map has no THINGS lump\n");
		return false;
	}

	// Seek to lump
	fseek(fp, lump->Offset(), SEEK_SET);

	if (hexen)
	{
		// Setup & read hexen format things

		// Setup things array
		unit_size = 20;
		n_things = lump->Size() / unit_size;
		things = (thing_t **)calloc(n_things, sizeof(thing_t *));

		// Read thing data
		for (DWORD i = 0; i < n_things; i++)
		{
			things[i] = new thing_t;
			fread(&things[i]->tid, 2, 1, fp);
			fread(&things[i]->x, 2, 1, fp);
			fread(&things[i]->y, 2, 1, fp);
			fread(&things[i]->z, 2, 1, fp);
			fread(&things[i]->angle, 2, 1, fp);
			fread(&things[i]->type, 2, 1, fp);
			fread(&things[i]->flags, 2, 1, fp);
			fread(&things[i]->special, 1, 1, fp);
			fread(things[i]->args, 1, 5, fp);
		}
	}
	else
	{
		// Setup things array
		unit_size = 10;
		n_things = lump->Size() / unit_size;
		things = (thing_t **)calloc(n_things, sizeof(thing_t *));

		// Read thing data
		for (DWORD i = 0; i < n_things; i++)
		{
			things[i] = new thing_t;
			fread(&things[i]->x, 2, 1, fp);
			fread(&things[i]->y, 2, 1, fp);
			fread(&things[i]->angle, 2, 1, fp);
			fread(&things[i]->type, 2, 1, fp);
			fread(&things[i]->flags, 2, 1, fp);
		}
	}

	// << ---- Read Scripts/Behavior ---- >>
	if (hexen)
	{
		lump = wad->get_lump("SCRIPTS", offset);

		if (lump)
		{
			fseek(fp, lump->Offset(), SEEK_SET);
			scripts = new Lump(0, lump->Size(), "SCRIPTS");
			fread(scripts->Data(), lump->Size(), 1, fp);
		}
		else
			scripts = new Lump(0, 0, "SCRIPTS");

		lump = wad->get_lump("BEHAVIOR", offset);

		if (lump)
		{
			fseek(fp, lump->Offset(), SEEK_SET);
			behavior = new Lump(0, lump->Size(), "BEHAVIOR");
			fread(behavior->Data(), lump->Size(), 1, fp);
		}
		else
			behavior = new Lump(0, 0, "BEHAVIOR");
	}

	// Remove detached vertices (from nodes)
	for (DWORD v = 0; v < n_verts; v++)
	{
		if (!v_isattached(v))
		{
			delete_vertex(v);
			v--;
		}
	}

	// Set thing colours/radii/angle
	for (int a = 0; a < n_things; a++)
		things[a]->ttype = get_thing_type(things[a]->type);

	init_map();
	opened = true;

	return true;
}

short Map::l_getsector1(int l)
{
	if (lines[l]->side1 == -1 || n_sectors == 0)
		return -1;
	else
		return sides[lines[l]->side1]->sector;
}

short Map::l_getsector2(int l)
{
	if (lines[l]->side2 == -1 || n_sectors == 0)
		return -1;
	else
		return sides[lines[l]->side2]->sector;
}

/*
bool Map::s_ishilighted(int s)
{
	if (s < 0)
		return false;
	else
		return sectors[s]->hilighted;
}

bool Map::s_isselected(int s)
{
	if (s < 0)
		return false;
	else
		return sectors[s]->selected;
}

bool Map::s_ismoving(int s)
{
	if (s < 0 || s > n_sectors)
		return false;
	else
		return sectors[s]->moving;
}
*/

void Map::delete_vertex(int vertex)
{
	free(verts[vertex]);

	for (DWORD v = vertex; v < n_verts - 1; v++)
		verts[v] = verts[v + 1];

	n_verts--;
	verts = (vertex_t **)realloc(verts, n_verts * sizeof(vertex_t *));

	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == vertex || lines[l]->vertex2 == vertex)
		{
			delete_line(l);
			l--;
		}
		else
		{
			if (lines[l]->vertex1 > vertex)
				lines[l]->vertex1--;
		
			if (lines[l]->vertex2 > vertex)
				lines[l]->vertex2--;
		}
	}
}

void Map::delete_line(int line)
{
	free(lines[line]);

	for (DWORD l = line; l < n_lines - 1; l++)
		lines[l] = lines[l + 1];

	n_lines--;
	lines = (linedef_t **)realloc(lines, n_lines * sizeof(linedef_t *));
}

void Map::delete_thing(int thing)
{
	free(things[thing]);

	for (DWORD t = thing; t < n_things - 1; t++)
		things[t] = things[t + 1];

	n_things--;
	things = (thing_t **)realloc(things, n_things * sizeof(thing_t *));
}

void Map::delete_side(int side)
{
	free(sides[side]);

	for (DWORD s = side; s < n_sides - 1; s++)
		sides[s] = sides[s + 1];

	n_sides--;
	sides = (sidedef_t **)realloc(sides, n_sides * sizeof(sidedef_t *));

	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->side1 == side)
		{
			lines[l]->side1 = -1;

			if (lines[l]->side2 != -1 && lines[l]->side2 != side)
			{
				l_flip(l);
				l_flipsides(l);
				map.lines[l]->set_flag(LINE_IMPASSIBLE);
				map.lines[l]->clear_flag(LINE_TWOSIDED);
			}
		}

		if (lines[l]->side2 == side)
		{
			lines[l]->side2 = -1;
			map.lines[l]->set_flag(LINE_IMPASSIBLE);
			map.lines[l]->clear_flag(LINE_TWOSIDED);
		}

		if (lines[l]->side1 > side)
			lines[l]->side1--;

		if (lines[l]->side2 > side)
			lines[l]->side2--;
	}
}

void Map::delete_sector(int sector)
{
	free(sectors[sector]);

	for (int s = sector; s < n_sectors - 1; s++)
		sectors[s] = sectors[s + 1];

	n_sectors--;
	sectors = (sector_t **)realloc(sectors, n_sectors * sizeof(sector_t *));

	for (int s = 0; s < n_sides; s++)
	{
		if (sides[s]->sector == sector)
			sides[s]->sector = -1;

		if (sides[s]->sector > sector)
			sides[s]->sector--;
	}

	for (int s = 0; s < n_sides; s++)
	{
		if (sides[s]->sector == -1)
		{
			delete_side(s);
			s--;
		}
	}
}

void Map::delete_vertex(vertex_t *vertex)
{
	for (int a = 0; a < n_verts; a++)
	{
		if (verts[a] == vertex)
		{
			delete_vertex(a);
			return;
		}
	}
}

void Map::delete_line(linedef_t *line)
{
	for (int a = 0; a < n_lines; a++)
	{
		if (lines[a] == line)
		{
			delete_line(a);
			return;
		}
	}
}

void Map::delete_sector(sector_t *sector)
{
	for (int a = 0; a < n_sectors; a++)
	{
		if (sectors[a] == sector)
		{
			delete_sector(a);
			return;
		}
	}
}

void Map::delete_side(sidedef_t *side)
{
	for (int a = 0; a < n_sides; a++)
	{
		if (sides[a] == side)
		{
			delete_side(a);
			return;
		}
	}
}

void Map::delete_thing(thing_t *thing)
{
	for (int a = 0; a < n_things; a++)
	{
		if (things[a] == thing)
		{
			delete_thing(a);
			return;
		}
	}
}

bool Map::v_isattached(int v)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
			return true;
	}
	
	return false;
}

bool Map::v_isattached_sector(int v)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
		{
			if (map.l_getsector1(l) != -1 || map.l_getsector1(l) != -1)
				return true;
		}
	}
	
	return false;
}

bool Map::l_needsuptex(int l, int side)
{
	int sector1 = l_getsector1(l);
	int sector2 = l_getsector2(l);

	// False if not two-sided
	if (sector1 == -1 || sector2 == -1)
		return false;

	if (side == 1)
	{
		if (sectors[l_getsector1(l)]->c_height > sectors[l_getsector2(l)]->c_height)
			return true;
		else
			return false;
	}

	if (side == 2)
	{
		if (sectors[l_getsector2(l)]->c_height > sectors[l_getsector1(l)]->c_height)
			return true;
		else
			return false;
	}

	return false;
}

bool Map::l_needsmidtex(int l, int side)
{
	if (side == 1)
	{
		if (lines[l]->side2 == -1)
			return true;
		else
		{
			if (sides[lines[l]->side1]->tex_middle == "-")
				return false;
			else
				return true;
		}
	}

	if (side == 2 && lines[l]->side1 != -1)
	{
		if (sides[lines[l]->side2]->tex_middle == "-")
			return false;
		else
			return true;
	}

	return false;
}

bool Map::l_needslotex(int l, int side)
{
	// False if not two-sided
	if (l_getsector1(l) == -1 || l_getsector2(l) == -1)
		return false;

	if (side == 1)
	{
		if (sectors[l_getsector2(l)]->f_height > sectors[l_getsector1(l)]->f_height)
			return true;
		else
			return false;
	}

	if (side == 2)
	{
		if (sectors[l_getsector1(l)]->f_height > sectors[l_getsector2(l)]->f_height)
			return true;
		else
			return false;
	}

	return false;
}

short Map::l_getxoff(int l, int side)
{
	if (side == 1 && lines[l]->side1 != -1)
		return sides[lines[l]->side1]->x_offset;

	if (side == 2 && lines[l]->side2 != -1)
		return sides[lines[l]->side2]->x_offset;

	return 0;
}

short Map::l_getyoff(int l, int side)
{
	if (side == 1 && lines[l]->side1 != -1)
		return sides[lines[l]->side1]->y_offset;

	if (side == 2 && lines[l]->side2 != -1)
		return sides[lines[l]->side2]->y_offset;

	return 0;
}

void Map::l_setmidtex(int l, int side, string tex)
{
	int s;

	if (side == 1)
		s = lines[l]->side1;

	if (side == 2)
		s = lines[l]->side2;

	if (s != -1)
		sides[s]->tex_middle = tex;
}

void Map::l_setuptex(int l, int side, string tex)
{
	int s;

	if (side == 1)
		s = lines[l]->side1;

	if (side == 2)
		s = lines[l]->side2;

	if (s != -1)
		sides[s]->tex_upper = tex;
}

void Map::l_setlotex(int l, int side, string tex)
{
	int s;

	if (side == 1)
		s = lines[l]->side1;

	if (side == 2)
		s = lines[l]->side2;

	if (s != -1)
		sides[s]->tex_lower = tex;
}

int Map::l_split(int l, int vertex)
{
	int vertex2 = lines[l]->vertex2;

	int side1 = -1;
	int side2 = -1;
	int new_line = -1;

	// Add new side for side1
	if (lines[l]->side1 != -1)
	{
		side1 = add_side();
		memcpy(sides[side1], sides[lines[l]->side1], sizeof(sidedef_t));
	}

	// Add new side for side2
	if (lines[l]->side2 != -1)
	{
		side2 = add_side();
		memcpy(sides[side2], sides[lines[l]->side2], sizeof(sidedef_t));
	}

	// Create new line
	lines[l]->vertex2 = vertex;
	new_line = add_line(vertex, vertex2);

	// Setup new line
	memcpy(lines[new_line], lines[l], sizeof(linedef_t));
	lines[new_line]->vertex1 = vertex;
	lines[new_line]->vertex2 = vertex2;
	lines[new_line]->side1 = side1;
	lines[new_line]->side2 = side2;

	return new_line;
}

bool Map::v_checkspot(int x, int y)
{
	for (DWORD v = 0; v < n_verts; v++)
	{
		if (verts[v]->x == x && verts[v]->y == y)
			return false;
	}

	return true;
}

void Map::v_merge(int v1, int v2)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v1)
			lines[l]->vertex1 = v2;
		else if (lines[l]->vertex2 == v1)
			lines[l]->vertex2 = v2;
	}

	delete_vertex(v1);
}

void Map::v_mergespot(int x, int y)
{
	//numlist_t merge;
	vector<int> merge;

	for (DWORD v = 0; v < n_verts; v++)
	{
		if (verts[v]->x == x && verts[v]->y == y)
			merge.push_back(v);
	}

	for (DWORD v = 1; v < merge.size(); v++)
		v_merge(merge[v], merge[0]);
}

int Map::add_thing(short x, short y, thing_t properties)
{
	n_things++;
	things = (thing_t **)realloc(things, n_things * sizeof(thing_t *));
	things[n_things - 1] = new thing_t();

	things[n_things - 1]->x = x;
	things[n_things - 1]->y = y;
	things[n_things - 1]->angle = properties.angle;
	things[n_things - 1]->type = properties.type;
	things[n_things - 1]->flags = properties.flags;
	//things[n_things - 1]->selected = false;
	//things[n_things - 1]->hilighted = false;
	//things[n_things - 1]->moving = false;

	//update_map_things();

	return n_things - 1;
}

int Map::add_vertex(short x, short y)
{
	n_verts++;
	verts = (vertex_t **)realloc(verts, n_verts * sizeof(vertex_t *));
	verts[n_verts - 1] = new vertex_t(x, y);

	return n_verts - 1;
}

int Map::add_line(int v1, int v2)
{
	n_lines++;
	lines = (linedef_t **)realloc(lines, n_lines * sizeof(linedef_t *));
	lines[n_lines - 1] = new linedef_t(v1, v2);
	lines[n_lines - 1]->set_flag(LINE_IMPASSIBLE);

	return n_lines - 1;
}

int Map::add_sector()
{
	n_sectors++;
	sectors = (sector_t **)realloc(sectors, n_sectors * sizeof(sector_t *));
	sectors[n_sectors - 1] = new sector_t();

	return n_sectors - 1;
}

int Map::add_side()
{
	n_sides++;
	sides = (sidedef_t **)realloc(sides, n_sides * sizeof(sidedef_t *));
	sides[n_sides - 1] = new sidedef_t();
	sides[n_sides - 1]->sector = n_sectors - 1;

	return n_sides - 1;
}

void Map::l_setdeftextures(int l)
{
	if (l_getsector1(l) != -1)
	{
		sides[lines[l]->side1]->tex_upper = "-";
		sides[lines[l]->side1]->tex_lower = "-";
		sides[lines[l]->side1]->tex_middle = "-";

		if (l_needsuptex(l, 1))
			sides[lines[l]->side1]->def_tex(TEX_UPPER);

		if (l_needsmidtex(l, 1))
			sides[lines[l]->side1]->def_tex(TEX_MIDDLE);

		if (l_needslotex(l, 1))
			sides[lines[l]->side1]->def_tex(TEX_LOWER);
	}

	if (l_getsector2(l) != -1)
	{
		sides[lines[l]->side2]->tex_upper = "-";
		sides[lines[l]->side2]->tex_lower = "-";
		sides[lines[l]->side2]->tex_middle = "-";

		if (l_needslotex(l, 2))
			sides[lines[l]->side2]->def_tex(TEX_LOWER);

		if (l_needsuptex(l, 2))
			sides[lines[l]->side2]->def_tex(TEX_UPPER);

		if (l_needsmidtex(l, 2))
			sides[lines[l]->side2]->def_tex(TEX_MIDDLE);
	}
}

// Map::add_to_wad: Adds raw map data to a wad file
// --------------------------------------------- >>
void Map::add_to_wad(Wad *wadfile)
{
	if (wadfile->locked)
		return;

	print(true, "Saving %s to %s..,\n", name.c_str(), wadfile->path.c_str());

	BYTE*	things_data = NULL;
	BYTE*	lines_data = NULL;
	BYTE*	sides_data = NULL;
	BYTE*	verts_data = NULL;
	BYTE*	sectors_data = NULL;

	int		thing_size = 10;
	int		line_size = 14;

	if (hexen)
	{
		thing_size = 20;
		line_size = 16;
	}

	// *** SETUP DATA ***

	// Setup things data
	things_data = (BYTE *)malloc(n_things * thing_size);

	for (DWORD t = 0; t < n_things; t++)
	{
		BYTE* p = things_data;
		p += (t * thing_size);

		if (hexen)
		{
			memcpy(p, &things[t]->tid, 2); p += 2;
			memcpy(p, &things[t]->x, 2); p += 2;
			memcpy(p, &things[t]->y, 2); p += 2;
			memcpy(p, &things[t]->z, 2); p += 2;
			memcpy(p, &things[t]->angle, 2); p += 2;
			memcpy(p, &things[t]->type, 2); p += 2;
			memcpy(p, &things[t]->flags, 2); p += 2;
			memcpy(p, &things[t]->special, 1); p += 1;
			memcpy(p, things[t]->args, 5); p += 5;
		}
		else
		{
			memcpy(p, &things[t]->x, 2); p += 2;
			memcpy(p, &things[t]->y, 2); p += 2;
			memcpy(p, &things[t]->angle, 2); p += 2;
			memcpy(p, &things[t]->type, 2); p += 2;
			memcpy(p, &things[t]->flags, 2); p += 2;
		}
	}

	// Setup lines data
	lines_data = (BYTE *)malloc(n_lines * line_size);

	for (DWORD l = 0; l < n_lines; l++)
	{
		BYTE* p = lines_data;
		p += (l * line_size);

		if (hexen)
		{
			BYTE temp = lines[l]->type;

			memcpy(p, &lines[l]->vertex1, 2); p += 2;
			memcpy(p, &lines[l]->vertex2, 2); p += 2;
			memcpy(p, &lines[l]->flags, 2); p += 2;
			memcpy(p, &temp, 2); p += 1;
			memcpy(p, lines[l]->args, 5); p += 5;
			memcpy(p, &lines[l]->side1, 2); p += 2;
			memcpy(p, &lines[l]->side2, 2); p += 2;
		}
		else
		{
			memcpy(p, &lines[l]->vertex1, 2); p += 2;
			memcpy(p, &lines[l]->vertex2, 2); p += 2;
			memcpy(p, &lines[l]->flags, 2); p += 2;
			memcpy(p, &lines[l]->type, 2); p += 2;
			memcpy(p, &lines[l]->sector_tag, 2); p += 2;
			memcpy(p, &lines[l]->side1, 2); p += 2;
			memcpy(p, &lines[l]->side2, 2); p += 2;
		}
	}

	// Setup sides data
	sides_data = (BYTE *)malloc(n_sides * 30);

	for (DWORD s = 0; s < n_sides; s++)
	{
		// Pad texture names with 0's
		char up[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };
		char lo[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };
		char mid[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };

		// Read texture names into temp char arrays
		for (int c = 0; c < sides[s]->tex_upper.length(); c++)
			up[c] = sides[s]->tex_upper[c];
		for (int c = 0; c < sides[s]->tex_lower.length(); c++)
			lo[c] = sides[s]->tex_lower[c];
		for (int c = 0; c < sides[s]->tex_middle.length(); c++)
			mid[c] = sides[s]->tex_middle[c];

		BYTE* p = sides_data + (s * 30);

		memcpy(p, &sides[s]->x_offset, 2); p += 2;
		memcpy(p, &sides[s]->y_offset, 2); p += 2;
		memcpy(p, up, 8); p += 8;
		memcpy(p, lo, 8); p += 8;
		memcpy(p, mid, 8); p += 8;
		memcpy(p, &sides[s]->sector, 2); p += 2;
	}

	// Setup vertices data
	verts_data = (BYTE *)malloc(n_verts * 4);

	for (DWORD v = 0; v < n_verts; v++)
	{
		BYTE* p = verts_data + (v * 4);

		memcpy(p, &verts[v]->x, 2); p += 2;
		memcpy(p, &verts[v]->y, 2); p += 2;
	}

	// Setup sectors data
	sectors_data = (BYTE *)malloc(n_sectors * 26);

	for (DWORD s = 0; s < n_sectors; s++)
	{
		// Pad texture names with 0's
		char floor[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };
		char ceil[8] =	{ 0, 0, 0, 0, 0, 0, 0, 0 };

		// Read texture names into temp char arrays
		for (int c = 0; c < sectors[s]->f_tex.length(); c++)
			floor[c] = sectors[s]->f_tex[c];
		for (int c = 0; c < sectors[s]->c_tex.length(); c++)
			ceil[c] = sectors[s]->c_tex[c];

		BYTE* p = sectors_data + (s * 26);

		memcpy(p, &sectors[s]->f_height, 2); p += 2;
		memcpy(p, &sectors[s]->c_height, 2); p += 2;
		memcpy(p, floor, 8); p += 8;
		memcpy(p, ceil, 8); p += 8;
		memcpy(p, &sectors[s]->light, 2); p += 2;
		memcpy(p, &sectors[s]->special, 2); p += 2;
		memcpy(p, &sectors[s]->tag, 2); p += 2;
	}

	// *** WRITE DATA TO WADFILE ***
	bool scripts = false;
	bool behavior = false;

	// If map already exists in wad, delete it
	long mapindex = wadfile->get_lump_index(this->name, 0);
	
	if (mapindex != -1)
	{
		long index = mapindex + 1;
		bool done = false;

		while (!done)
		{
			if (index == wadfile->num_lumps)
				done = true;
			else if (strncmp(wadfile->directory[index]->Name().c_str(), "THINGS", 6) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "LINEDEFS", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SIDEDEFS", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "VERTEXES", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SEGS", 4) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SSECTORS", 8) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "NODES", 5) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "SECTORS", 7) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "REJECT", 6) == 0 ||
				strncmp(wadfile->directory[index]->Name().c_str(), "BLOCKMAP", 8) == 0)
			{
				print(true, "Deleting map entry %s\n", wadfile->directory[index]->Name().c_str());
				wadfile->delete_lump(wadfile->directory[index]->Name(), mapindex);
				done = false;
			}
			else if (strncmp(wadfile->directory[index]->Name().c_str(), "BEHAVIOR", 8) == 0)
			{
				done = false;
				index++;
				behavior = true;
			}
			else if (strncmp(wadfile->directory[index]->Name().c_str(), "SCRIPTS", 7) == 0)
			{
				done = false;
				index++;
				scripts = true;
			}
			else
			{
				print(true, "Found next non-map entry %s\n", wadfile->directory[index]->Name().c_str());
				done = true;
			}
		}
	}
	else
	{
		mapindex = 0;
		wadfile->add_lump(name, mapindex);
	}

	// Add map lumps
	if (hexen)
	{
		if (!scripts)
			wadfile->add_lump("SCRIPTS", mapindex + 1);

		if (!behavior)
			wadfile->add_lump("BEHAVIOR", mapindex + 1);
	}
	
	wadfile->add_lump("SECTORS", mapindex + 1);
	wadfile->add_lump("VERTEXES", mapindex + 1);
	wadfile->add_lump("SIDEDEFS", mapindex + 1);
	wadfile->add_lump("LINEDEFS", mapindex + 1);
	wadfile->add_lump("THINGS", mapindex + 1);

	// Write map data
	wadfile->replace_lump("THINGS", thing_size * n_things, things_data, mapindex);
	wadfile->replace_lump("LINEDEFS", line_size * n_lines, lines_data, mapindex);
	wadfile->replace_lump("SIDEDEFS", 30 * n_sides, sides_data, mapindex);
	wadfile->replace_lump("VERTEXES", 4 * n_verts, verts_data, mapindex);
	wadfile->replace_lump("SECTORS", 26 * n_sectors, sectors_data, mapindex);

	if (hexen)
	{
		wadfile->replace_lump("SCRIPTS", this->scripts->Size(), this->scripts->Data(), mapindex);
		wadfile->replace_lump("BEHAVIOR", this->behavior->Size(), this->behavior->Data(), mapindex);
	}
}

void Map::l_flip(int l)
{
	WORD temp = lines[l]->vertex1;
	lines[l]->vertex1 = lines[l]->vertex2;
	lines[l]->vertex2 = temp;
}

void Map::l_flipsides(int l)
{
	short temp = lines[l]->side1;
	lines[l]->side1 = lines[l]->side2;
	lines[l]->side2 = temp;
}

void Map::s_changeheight(int s, bool floor, int amount)
{
	if (floor)
		sectors[s]->f_height += amount;
	else
		sectors[s]->c_height += amount;
}

int	Map::v_getvertatpoint(point2_t point)
{
	if (n_verts == 0)
		return -1;

	for (int v = n_verts - 1; v >= 0; v--)
	{
		if (verts[v]->x == point.x && verts[v]->y == point.y)
			return v;
	}

	return -1;
}

vector<int> Map::v_getattachedlines(int v)
{
	vector<int> ret;

	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
		{
			if (vector_exists(ret, l))
				ret.push_back(l);
		}
	}

	return ret;
}

void Map::l_setsector(int l, int side, int sector)
{
	if (side == 1)
	{
		if (sector == -1)
		{
			lines[l]->side1 = -1;
			return;
		}

		if (lines[l]->side1 == -1)
			lines[l]->side1 = add_side();

		sides[lines[l]->side1]->sector = sector;
	}
	else
	{
		if (sector == -1)
		{
			lines[l]->side2 = -1;
			lines[l]->clear_flag(LINE_TWOSIDED);
			lines[l]->set_flag(LINE_IMPASSIBLE);
			return;
		}

		if (lines[l]->side2 == -1)
		{
			lines[l]->side2 = add_side();
			lines[l]->set_flag(LINE_TWOSIDED);
			lines[l]->clear_flag(LINE_IMPASSIBLE);
			l_setmidtex(l, 1, "-");
		}

		sides[lines[l]->side2]->sector = sector;
	}
}

int	Map::l_getindex(linedef_t* line)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l] == line)
			return l;
	}

	return -1;
}

sidedef_t* Map::l_getside(int l, int side)
{
	if (side == 1)
	{
		if (map.lines[l]->side1 == -1)
			return NULL;
		else
			return map.sides[map.lines[l]->side1];
	}
	else
	{
		if (map.lines[l]->side2 == -1)
			return NULL;
		else
			return map.sides[map.lines[l]->side2];
	}
}

point2_t Map::l_getmidpoint(int l)
{
	point2_t tl(map.verts[map.lines[l]->vertex1]->x,
				map.verts[map.lines[l]->vertex1]->y);
	point2_t br(map.verts[map.lines[l]->vertex2]->x,
				map.verts[map.lines[l]->vertex2]->y);

	return midpoint(tl, br);
}

vector<int> Map::l_getfromid(int line_id)
{
	vector<int> ret;

	for (int l = 0; l < n_lines; l++)
	{
		if (lines[l]->type == 121)
		{
			if (line_id == lines[l]->args[0])
			{
				if (vector_exists(ret, l))
					ret.push_back(l);
			}
		}
	}

	return ret;
}

void Map::v_getattachedlines(int v, numlist_t* list)
{
	for (DWORD l = 0; l < n_lines; l++)
	{
		if (lines[l]->vertex1 == v || lines[l]->vertex2 == v)
			list->add(l, false);
	}
}
