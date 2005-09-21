// <<--------------------------------------------->>
// << SLADE (SlayeR's 'LeetAss Doom Editor)       >>
// << By Simon Judd, 2004                         >>
// << ------------------------------------------- >>
// << <checks.cpp>                                >>
// << Map checking functions                      >>
// <<--------------------------------------------->>

// Includes ------------------------------------- >>
#include "main.h"
#include "map.h"

// Variables ------------------------------------ >>

// External Variables --------------------------- >>
extern Map map;

// clean_tags: Finds any unpaired tags and frees them
// ----------------------------------------------- >>
DWORD clean_tags()
{
	vector<int> ltags;
	vector<int> stags;
	DWORD c = 0;
	
	// Add all existing sector tags
	for (DWORD s = 0; s < map.n_sectors; s++)
	{
		if (map.sectors[s]->tag > 0 && !(vector_exists(stags, map.sectors[s]->tag)))
			stags.push_back(map.sectors[s]->tag);
	}

	// Add all existing line tags, except those without a matching
	// sector tag (in that case, clear the tag)
	for (DWORD l = 0; l < map.n_lines; l++)
	{
		if (map.lines[l]->sector_tag > 0)
		{
			if (!(vector_exists(stags, map.lines[l]->sector_tag)))
			{
				map.lines[l]->sector_tag = 0;
				c++;
			}
			else if (!(vector_exists(ltags, map.lines[l]->sector_tag)))
				ltags.push_back(map.lines[l]->sector_tag);
		}
	}
	
	// Clear all unmatched sector tags
	for (DWORD s = 0; s < map.n_sectors; s++)
	{
		if (map.sectors[s]->tag > 0)
		{
			if (!(vector_exists(ltags, map.sectors[s]->tag)))
			{
				map.sectors[s]->tag = 0;
				c++;
			}
		}
	}
	
	return c;
}

// remove_free_verts: Deletes any vertices not attached to a line
// ----------------------------------------------------------- >>
DWORD remove_free_verts()
{
	DWORD c = 0;

	for (DWORD v = 0; v < map.n_verts; v++)
	{
		if (!map.v_isattached(v))
		{
			map.delete_vertex(v);
			v--;
			c++;
		}
	}

	return c;
}

// remove_zerolength_lines: Removes any lines with both ends at the same position
// --------------------------------------------------------------------------- >>
DWORD remove_zerolength_lines()
{
	DWORD c = 0;

	for (DWORD l = 0; l < map.n_lines; l++)
	{
		rect_t r = map.l_getrect(l);

		if (r.x1() == r.x2()
			&& r.y1() == r.y2())
		{
			map.delete_line(l);
			l--;
			c++;
		}
	}

	//remove_free_verts();

	return c;
}

// remove_unused_sectors: Removes any unused sectors in the map
// --------------------------------------------------------- >>
DWORD remove_unused_sectors()
{
	DWORD c = 0;
	vector<int> used_sectors;
	bool done = false;

	while (!done)
	{
		done = true;

		for (DWORD s = 0; s < map.n_sides; s++)
		{
			if (!(vector_exists(used_sectors, map.sides[s]->sector)))
				used_sectors.push_back(map.sides[s]->sector);
		}

		for (DWORD s = 0; s < map.n_sectors; s++)
		{
			if (!(vector_exists(used_sectors, s)))
			{
				map.delete_sector(s);
				done = false;
				c++;
				break;
			}
		}
	}

	return c;
}

// get_free_tag: Gets the lowest free tag,
// might be slow on large maps with lots of tags, but at least it doesn't
// simply find the highest tag used and return 1 higher...
// ------------------------------------------------------------------- >>
WORD get_free_tag()
{
	WORD tag = 0;
	bool tagmoved = false;
	
	while (1)
	{
		tagmoved = false;
		
		for (DWORD l = 0; l < map.n_lines; l++)
		{
			if (map.lines[l]->sector_tag == tag)
			{
				tag++;
				tagmoved = true;
			}
		}
		
		for (DWORD s = 0; s < map.n_sectors; s++)
		{
			if (map.sectors[s]->tag == tag)
			{
				tag++;
				tagmoved = true;
			}
		}
		
		if (!tagmoved)
			break;
	}
	
	return tag;
}

// get_free_tag: Gets the lowest free thing id
// ---------------------------------------- >>
WORD get_free_tid()
{
	WORD tid = 0;
	bool moved = false;
	
	while (1)
	{
		moved = false;
		
		for (DWORD t = 0; t < map.n_things; t++)
		{
			if (map.things[t]->tid == tid)
			{
				tid++;
				moved = true;
			}
		}
		
		if (!moved)
			break;
	}
	
	return tid;
}
