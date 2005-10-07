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
extern rgba_t col_line_special;
extern rgba_t col_linedraw;
extern rgba_t col_selbox;
extern rgba_t col_selbox_line;
extern rgba_t col_grid;
extern rgba_t col_64grid;
rgba_t col_3d_crosshair;
rgba_t col_3d_hilight;
rgba_t col_3d_hilight_line;

extern float hilight_size, moving_size, selection_size;

void parse_rgba(Tokenizer *mr, rgba_t* col)
{
	// Read opening brace
	mr->check_token("{");

	// Read rgba
	col->r = mr->get_integer();
	col->g = mr->get_integer();
	col->b = mr->get_integer();

	// Read alpha if specified
	if (mr->peek_token() != "}")
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
			string name = token;

			if (name == "hilight_size")
				ccfg.hilight_size = tz.get_float();
			else if (name == "selection_size")
				ccfg.selection_size = tz.get_float();
			else if (name == "moving_size")
				ccfg.moving_size = tz.get_float();
			else
			{
				rgba_t col;
				parse_rgba(&tz, &col);
				ccfg.add(name, col);
			}

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
	else if (colour_configs[0].get_colour(name))
		colour->set(*colour_configs[0].get_colour(name));

	hilight_size = colour_configs[config].hilight_size;
	selection_size = colour_configs[config].selection_size;
	moving_size = colour_configs[config].moving_size;
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
	SETCOL(line_special);
	SETCOL(linedraw);
	SETCOL(selbox);
	SETCOL(selbox_line);
	SETCOL(grid);
	SETCOL(64grid);
	SETCOL(tagged);
	SETCOL(3d_crosshair);
	SETCOL(3d_hilight);
	SETCOL(3d_hilight_line);
}

void col_config_t::save()
{
	if (path == "")
		return;

	FILE* fp = fopen(path.c_str(), "wt");

	fprintf(fp, "\ncolour_config \"%s\"\n", name.c_str());
	fprintf(fp, "{");

	for (int a = 0; a < colours.size(); a++)
	{
		fprintf(fp, "\t%s\t{\t%d\t%d\t%d\t%d\t%d\t}\n",
				names[a].c_str(), colours[a].r, colours[a].g, colours[a].b, colours[a].a, colours[a].blend);
	}

	fprintf(fp, "\n\thilight_size\t%1.2f\n", hilight_size);
	fprintf(fp, "\tselection_size\t%1.2f\n", selection_size);
	fprintf(fp, "\tmoving_size\t%1.2f\n", moving_size);

	fprintf(fp, "}\n\n");

	fclose(fp);
}
