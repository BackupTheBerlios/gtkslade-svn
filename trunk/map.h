
#ifndef __MAP_H__
#define __MAP_H__

#include "linedef.h"
#include "sidedef.h"
#include "vertex.h"
#include "sector.h"
#include "thing.h"
#include "edit.h"

#define MAP_BOOM	0x01
#define	MAP_HEXEN	0x02
#define MAP_ZDOOM	0x04

class Map
{
public:
	linedef_t*	*lines;
	sidedef_t*	*sides;
	vertex_t*	*verts;
	sector_t*	*sectors;
	thing_t*	*things;

	bool	hexen;	// Hexen format?
	bool	zdoom;	// ZDoom extras?
	bool	boom;	// Boom extras?
	string	name;	// Map name

	DWORD n_lines, n_sides, n_verts, n_sectors, n_things;

	// Constructor/Deconstructor
	Map()
	{
		lines = (linedef_t **)NULL;
		sides = (sidedef_t **)NULL;
		verts = (vertex_t **)NULL;
		sectors = (sector_t **)NULL;
		things = (thing_t **)NULL;

		n_lines = n_sides = n_verts = n_sectors = n_things = 0;
	}

	~Map() {}


	// Member Functions
	bool	open(Wad* wad, string mapname);
	void	create(string mapname);
	void	close();

	// Get/set line properties
	short	l_getsector1(int l);
	short	l_getsector2(int l);
	short	l_getxoff(int l, int side);
	short	l_getyoff(int l, int side);

	rect_t	l_getrect(int l)	{ return rect_t(verts[lines[l]->vertex1]->x, verts[lines[l]->vertex1]->y, 
												verts[lines[l]->vertex2]->x, verts[lines[l]->vertex2]->y); }
	rect_t	l_getsrect(int l)	{ return rect_t(s_x(verts[lines[l]->vertex1]->x), s_y(-verts[lines[l]->vertex1]->y), 
												s_x(verts[lines[l]->vertex2]->x), s_y(-verts[lines[l]->vertex2]->y)); }

	point2_t	l_getmidpoint(int l);

	void	l_setmidtex(int l, int side, string tex);
	void	l_setuptex(int l, int side, string tex);
	void	l_setlotex(int l, int side, string tex);
	void	l_setsector(int l, int side, int sector);

	sidedef_t*	l_getside(int l, int side);
	
	// Misc line stuff
	bool		l_needsuptex(int l, int side);
	bool		l_needsmidtex(int l, int side);
	bool		l_needslotex(int l, int side);

	void		l_setdeftextures(int l);

	int			l_split(int l, int vertex);
	void		l_flip(int l);
	void		l_flipsides(int l);

	int			l_getindex(linedef_t* line);

	vector<int>	l_getfromid(int line_id);

	// Get/set vertex properties
	point2_t	v_getpoint(int v)	{ return point2_t(verts[v]->x, verts[v]->y);			}
	point2_t	v_getspoint(int v)	{ return point2_t(s_x(verts[v]->x), s_y(-verts[v]->y));	}

	// Misc vertex stuff
	bool		v_isattached(int v);
	bool		v_isattached_sector(int v);
	bool		v_checkspot(int x, int y);
	void		v_merge(int v1, int v2);
	void		v_mergespot(int x, int y);
	vector<int>	v_getattachedlines(int v);
	int			v_getvertatpoint(point2_t point);
	void		v_getattachedlines(int v, numlist_t* list);

	// Get/set sector properties
	void	s_changeheight(int s, bool floor, int amount);

	// Remove items
	void	delete_vertex(int vertex);
	void	delete_line(int line);
	void	delete_thing(int thing);
	void	delete_side(int side);
	void	delete_sector(int sector);

	void	delete_vertex(vertex_t *vertex);
	void	delete_line(linedef_t *line);
	void	delete_thing(thing_t *thing);
	void	delete_side(sidedef_t *side);
	void	delete_sector(sector_t *sector);

	// Add items
	int add_thing(short x, short y, thing_t properties);
	int	add_vertex(short x, short y);
	int	add_line(int v1, int v2);
	int add_sector();
	int add_side();

	// Save
	//void save_as(string filename, string mapname);
	void add_to_wad(Wad* wadfile);
};

#define SELECTED(index) selected_items.exists(index)
#define MOVING(index) move_list.exists(index)

#endif