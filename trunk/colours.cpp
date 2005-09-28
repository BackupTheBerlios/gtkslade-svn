// <<--------------------------------------------->>
// << SLADE (SlayeR's 'LeetAss Doom Editor)       >>
// << By Simon Judd, 2004                         >>
// << ------------------------------------------- >>
// << <colours.cpp>                               >>
// << Colour configuration stuff                  >>
// <<--------------------------------------------->>

// Includes ------------------------------------- >>
#include "main.h"
#include "colours.h"

// Variables ------------------------------------ >>
/*
col_config_t	colour_config[16];
int				n_cconfig = 0;
vector<string>	colour_schemes;
*/
vector<col_config_t> colour_configs;

// External Variables --------------------------- >>
//extern rgba_t col_cursor;
extern rgba_t col_background;
extern rgba_t col_hilight;
extern rgba_t col_selection;
extern rgba_t col_moving;
extern rgba_t col_tagged;
extern rgba_t col_vertex;
extern rgba_t col_line_solid;
extern rgba_t col_line_2s;
extern rgba_t col_line_monster;
extern rgba_t col_selbox;
extern rgba_t col_selbox_line;
extern rgba_t col_grid;
extern rgba_t col_64grid;
rgba_t col_3d_crosshair;
rgba_t col_3d_hilight;
rgba_t col_3d_hilight_line;

void parse_rgba(Tokenizer *mr, rgba_t* col)
{
	// Read opening brace
	mr->check_token("{");

	// Read rgba
	col->r = mr->get_integer();
	col->g = mr->get_integer();
	col->b = mr->get_integer();
	col->a = mr->get_integer();

	// Read blend if specified
	if (mr->peek_token() != "}")
		col->blend = mr->get_integer();

	// Read closing brace
	mr->check_token("}");
}

void read_colour_config(string filename)
{
	Tokenizer tz;
	if (!tz.open_file(filename, 0, 0))
		return;

	if (tz.get_token() == "colour_config")
	{
		col_config_t ccfg;
		ccfg.name = tz.get_token();

		tz.check_token("{");

		string token = tz.get_token();
		while (token != "}")
		{
			rgba_t col;
			string name = token;
			parse_rgba(&tz, &col);
			ccfg.add(name, col);
			token = tz.get_token();
		}

		colour_configs.push_back(ccfg);
	}
}

bool load_colour_configs()
{
	read_colour_config("config/colours/default.cfg");

	GDir *dir = g_dir_open("./config/colours/", 0, NULL);

	if (dir)
	{
		const gchar* dir_name = g_dir_read_name(dir);

		while (dir_name)
		{
			if (g_str_has_prefix(dir_name, "default"))
				dir_name = g_dir_read_name(dir);
			else
			{
				string filename = "config/colours/";
				filename += dir_name;
				if (g_str_has_suffix(dir_name, ".cfg"))
					read_colour_config(filename);
				dir_name = g_dir_read_name(dir);
			}
		}
	}

	return true;
}

int get_colour_config(string name)
{
	for (int c = 0; c < colour_configs.size(); c++)
	{
		if (colour_configs[c].name == name)
			return c;
	}
	
	return -1;
}

void set_colour(rgba_t *colour, string name, int config)
{
	if (colour_configs.size() == 0)
		return;

	if (colour_configs[config].get_colour(name))
		colour->set(*colour_configs[config].get_colour(name));
	else
		colour->set(*colour_configs[0].get_colour(name));
}

void set_colour_config(string name)
{
	int config = get_colour_config(name);

	if (config == -1)
		return;
	
	SETCOL(background);
	SETCOL(hilight);
	SETCOL(selection);
	SETCOL(moving);
	SETCOL(vertex);
	SETCOL(line_solid);
	SETCOL(line_2s);
	SETCOL(line_monster);
	SETCOL(selbox);
	SETCOL(selbox_line);
	SETCOL(grid);
	SETCOL(64grid);
	SETCOL(tagged);
	SETCOL(3d_crosshair);
	SETCOL(3d_hilight);
	SETCOL(3d_hilight_line);
}

/*
void add_colour_config(string name)
{
	colour_config[n_cconfig].name = name;
	n_cconfig++;

	if (n_cconfig > 16)
		n_cconfig = 16;
	else
		colour_schemes.push_back(name);

	//printf("Colour config \"%s\"", name.c_str());
}

void add_colour(string name, rgba_t colour)
{
	colour_config[n_cconfig - 1].add(name, colour);

	//printf("%s %d %d %d %d\n", name.c_str(), colour.r, colour.g, colour.b, colour.a);
}

void parse_rgba(Tokenizer *mr, rgba_t* col)
{
	// Read opening brace
	mr->check_token("{");

	// Read rgba
	col->r = mr->get_integer();
	col->g = mr->get_integer();
	col->b = mr->get_integer();
	col->a = mr->get_integer();

	// Read blend if specified
	if (mr->peek_token() != "}")
		col->blend = mr->get_integer();

	// Read closing brace
	mr->check_token("}");
}

bool load_colour_configs(string filename)
{
	Tokenizer mr;
	if (!mr.open_file(filename, 0, 0))
		return false;

	string token = mr.get_token();

	while (token != "!END")
	{
		if (token == "colour_config")
		{
			// Read config name & add
			add_colour_config(mr.get_token());
			
			// Check for opening brace
			mr.check_token("{");

			token = mr.get_token();
			while (token != "}")
			{
				string name;
				rgba_t col;

				name = token;
				parse_rgba(&mr, &col);

				add_colour(name, col);

				token = mr.get_token();
			}
		}

		token = mr.get_token();
	}

	return true;
}

int get_colour_config(string name)
{
	for (int c = 0; c < n_cconfig; c++)
	{
		if (colour_config[c].name == name)
			return c;
	}
	
	return -1;
}

void set_colour(rgba_t *colour, string name, int config)
{
	if (colour_config[config].get_colour(name))
		colour->set(*colour_config[config].get_colour(name));
	else
		colour->set(*colour_config[0].get_colour(name));
}

void set_colour_config(string name)
{
	int config = get_colour_config(name);

	if (config == -1)
		return;
	
	SETCOL(background);
	SETCOL(hilight);
	SETCOL(selection);
	SETCOL(moving);
	SETCOL(vertex);
	SETCOL(line_solid);
	SETCOL(line_2s);
	SETCOL(line_monster);
	SETCOL(selbox);
	SETCOL(selbox_line);
	SETCOL(grid);
	SETCOL(64grid);
	SETCOL(tagged);
	SETCOL(3d_crosshair);
	SETCOL(3d_hilight);
	SETCOL(3d_hilight_line);
}
*/
