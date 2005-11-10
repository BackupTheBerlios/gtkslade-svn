// <<--------------------------------------->>
// << SLADE - SlayeR's 'LeetAss Doom Editor >>
// << By Simon Judd, 2004                   >>
// << ------------------------------------- >>
// << game_config.cpp - Game config stuff   >>
// <<--------------------------------------->>

// Includes ------------------------------- >>
#include "main.h"
#include "misc.h"
#include "map.h"
#include "action_special.h"
#include "args.h"

struct iwadinfo_t
{
	string game;
	string iwad_path;
};

// Variables ------------------------------ >>
vector<string> game_config_paths;
vector<string> game_config_names;
vector<string> valid_map_names;
vector<iwadinfo_t> game_iwads;

// External Variables --------------------- >>
extern WadList wads;
extern Map map;

extern string def_midtex, def_uptex, def_lotex, def_ftex, def_ctex;
extern short def_fheight, def_cheight, def_light;
extern bool mix_tex;

void load_game_iwads(Tokenizer *tz)
{
	tz->check_token("{");

	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "game")
		{
			iwadinfo_t ii;
			ii.game = tz->get_token();
			ii.iwad_path = tz->get_token();
			game_iwads.push_back(ii);
		}

		token = tz->get_token();
	}
}

void save_game_iwads(FILE* fp)
{
	fprintf(fp, "iwads\n{\n");

	for (int a = 0; a < game_iwads.size(); a++)
		fprintf(fp, "\tgame \"%s\" \"%s\"\n", game_iwads[a].game.c_str(), game_iwads[a].iwad_path.c_str());

	fprintf(fp, "}\n\n");
}

// read_types: Reads either line, sector, thing or arg types from a file
// ------------------------------------------------------------------ >>
void read_types(Tokenizer *mr, bool things, bool lines, bool sectors, bool args)
{
	string token = mr->get_token();
	while (token != "!END")
	{
		// Thing types
		if (things && token == "thing_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "group")
					parse_thing_group(mr);

				if (token == "import")
				{
					Tokenizer mr2;
					mr2.open_file("games/" + mr->get_token(), 0, 0);
					read_types(&mr2, true, false, false, false);
				}

				token = mr->get_token();
			}
		}

		// Action specials
		if (lines && token == "action_specials")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "group")
					parse_action_group(mr);

				if (token == "import")
				{
					Tokenizer mr2;
					mr2.open_file("games/" + mr->get_token(), 0, 0);
					read_types(&mr2, false, true, false, false);
				}

				token = mr->get_token();
			}
		}

		// Sector types
		if (sectors && token == "sector_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "type")
				{
					sectortype_t *stype = new sectortype_t();
					stype->parse(mr);
				}

				if (token == "import")
				{
					Tokenizer mr2;
					mr2.open_file("games/" + mr->get_token(), 0, 0);
					read_types(&mr2, false, false, true, false);
				}

				token = mr->get_token();
			}
		}

		// Arg types
		if (args && token == "arg_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "type")
				{
					argtype_t *type = new argtype_t();
					type->parse(mr);
				}

				if (token == "import")
				{
					Tokenizer mr2;
					mr2.open_file("games/" + mr->get_token(), 0, 0);
					read_types(&mr2, false, false, false, true);
				}

				token = mr->get_token();
			}
		}

		token = mr->get_token();
	}
}

// load_game_config: Loads a game configuration
// ----------------------------------------- >>
bool load_game_config(int index)
{
	if (index > game_config_paths.size())
	{
		message_box("Invalid Index", GTK_MESSAGE_ERROR);
		return false;
	}

	Tokenizer tz;
	tz.open_file(game_config_paths[index], 0, 0);

	string token = tz.get_token();

	if (token != "game")
	{
		message_box("Invalid game configuration file (must begin with \"game\"", GTK_MESSAGE_ERROR);
		return false;
	}

	// Read game name
	token = tz.get_token();

	int i = -1;
	for (int a = 0; a < game_iwads.size(); a++)
	{
		if (game_iwads[a].game == token)
			i = a;
	}

	string iwad_path = "";
	if (i == -1)
	{
		message_box(parse_string("Please browse for the \"%s\" game IWAD...", token.c_str()), GTK_MESSAGE_INFO);
		
		string filename = file_browser("*.wad");
		if (filename != "")
			iwad_path = filename;
		else
			return false;
	}
	else
		iwad_path = game_iwads[i].iwad_path;

	wads.get_iwad()->close();
	clear_action_specials();
	clear_thing_types();
	valid_map_names.clear();
	bool map_names = false;

	// Check opening brace
	tz.check_token("{");

	if (!wads.open_iwad(iwad_path))
	{
		message_box(parse_string("Couldn't open IWAD \"%s\"!", iwad_path.c_str()), GTK_MESSAGE_ERROR);
		game_iwads.erase(game_iwads.begin() + i);
		return false;
	}
	else if (i == -1)
	{
		iwadinfo_t ii;
		ii.game = token;
		ii.iwad_path = iwad_path;
		game_iwads.push_back(ii);
	}

	// Read game section
	token = tz.get_token();
	while (token != "}")
	{
		/*
		if (token == "iwad")
		{
			bool done = false;
			string wadpath = tz.get_token();

			while (!done)
			{
				if (!wads.open_iwad(wadpath))
				{
					message_box(parse_string("IWAD file \"%s\" doesn't exist!\nBrowse for the IWAD...", wadpath.c_str()), GTK_MESSAGE_ERROR);
					
					string filename = file_browser("*.wad");
					if (filename != "")
						wadpath = filename;
					else
						done = true;
				}
				else
					done = true;
			}
		}
		*/

		if (token == "boom")
			map.boom = tz.get_bool();

		if (token == "hexen")
			map.hexen = tz.get_bool();

		if (token == "zdoom")
			map.zdoom = tz.get_bool();

		if (token == "map_names")
			map_names = tz.get_bool();

		token = tz.get_token();
	}

	// Read map names
	if (tz.get_token() == "map_names")
	{
		tz.check_token("{");
		token = tz.get_token();

		while (token != "}")
		{
			valid_map_names.push_back(token);
			token = tz.get_token();
		}
	}

	// Read defaults
	if (tz.get_token() == "defaults")
	{
		tz.check_token("{");
		token = tz.get_token();

		while (token != "}")
		{
			if (token == "floor_tex")
				def_ftex = tz.get_token();

			if (token == "ceil_tex")
				def_ctex = tz.get_token();

			if (token == "upper_tex")
				def_uptex = tz.get_token();

			if (token == "lower_tex")
				def_lotex = tz.get_token();

			if (token == "middle_tex")
				def_midtex = tz.get_token();

			if (token == "floor_height")
				def_fheight = tz.get_integer();

			if (token == "ceil_height")
				def_cheight = tz.get_integer();

			if (token == "light")
				def_light = tz.get_integer();

			token = tz.get_token();
		}
	}

	// Read action specials & thing/sector types
	read_types(&tz, true, true, true, true);

	mix_tex = map.zdoom;

	return true;
}
