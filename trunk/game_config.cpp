
#include "main.h"
#include "misc.h"
#include "map.h"
#include "action_special.h"

vector<string> game_config_paths;
vector<string> game_config_names;

extern WadList wads;
extern Map map;

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
		/*
		if (sectors && token == "sector_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "type")
					parse_sectortype(mr);

				if (token == "import_types")
				{
					MemReader mr2;
					mr2.open_file("games/" + mr->get_token(), 0, 0);
					read_types(&mr2, false, false, true, false);
				}

				token = mr->get_token();
			}
		}
		*/

		// Arg types
		/*
		if (args && token == "arg_types")
		{
			mr->check_token("{");

			token = mr->get_token();
			while (token != "}")
			{
				if (token == "type")
					parse_argtype(mr);

				if (token == "import_types")
				{
					MemReader mr2;
					mr2.open_file("games/" + mr->get_token(), 0, 0);
					read_types(&mr2, false, false, false, true);
				}

				token = mr->get_token();
			}
		}
		*/

		token = mr->get_token();
	}
}

bool load_game_config(int index)
{
	wads.get_iwad()->close();

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

	// Check opening brace
	tz.check_token("{");

	// Read game section
	token = tz.get_token();
	while (token != "}")
	{
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

		if (token == "boom")
			map.boom = tz.get_bool();

		if (token == "hexen")
			map.hexen = tz.get_bool();

		if (token == "zdoom")
			map.zdoom = tz.get_bool();

		token = tz.get_token();
	}

	// Read action specials & thing/sector types
	read_types(&tz, true, true, true, true);

	return true;
}
