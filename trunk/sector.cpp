
#include "main.h"
#include "map.h"

string	def_ftex = "MFLR8_1";
string	def_ctex = "MFLR8_1";
short	def_fheight = 0;
short	def_cheight = 128;
short	def_light = 160;

extern Map map;

sector_t::sector_t()
{
	f_height = def_fheight;
	c_height = def_cheight;
	f_tex = def_ftex;
	c_tex = def_ctex;
	light = def_light;
	special = 0;
	tag = 0;
}
