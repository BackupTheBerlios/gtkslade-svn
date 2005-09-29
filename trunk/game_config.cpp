
#include "main.h"
#include "misc.h"
#include "map.h"
#include "textures.h"

vector<string> game_config_paths;
vector<string> game_config_names;

extern WadList wads;
extern Map map;

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

	return true;
}
