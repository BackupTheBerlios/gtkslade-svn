
#include "main.h"
#include "map.h"
#include "colours.h"
#include "misc.h"
#include "console.h"
#include "console_window.h"

thing_type_t thing_unknown;

vector<string> spritenames;

struct thing_group_t
{
	vector<thing_type_t>	things;
	string					name;
};

vector<thing_group_t*>	thing_types;

thing_type_t::thing_type_t(int type, string name)
{
	this->type = type;
	this->name = name;

	colour.set(200, 200, 200, 255, 0);
	radius = -1;
	spritename = "";
	show_angle = true;
	hanging = false;

	args[0] = "Arg 1";
	args[1] = "Arg 2";
	args[2] = "Arg 3";
	args[3] = "Arg 4";
	args[4] = "Arg 5";

	for (int a = 0; a < 5; a++)
		arg_types[a] = "";
}

void thing_type_t::parse(Tokenizer *tz)
{
	type = tz->get_integer();
	name = tz->get_token();

	if (tz->peek_token() == "{")
	{
		tz->check_token("{");

		string token = tz->get_token();
		while (token != "}")
		{
			if (token == "size")
				radius = tz->get_integer();
			if (token == "colour")
				parse_rgba(tz, &colour);
			if (token == "sprite")
			{
				spritename = tz->get_token();

				if (!(vector_exists(spritenames, spritename)))
					spritenames.push_back(spritename);
			}
			if (token == "angle")
				show_angle = tz->get_bool();
			if (token == "hanging")
				hanging = tz->get_bool();

			if (token == "arg1")
				args[0] = tz->get_token();
			if (token == "arg2")
				args[1] = tz->get_token();
			if (token == "arg3")
				args[2] = tz->get_token();
			if (token == "arg4")
				args[3] = tz->get_token();
			if (token == "arg5")
				args[4] = tz->get_token();

			token = tz->get_token();
		}
	}
}

void parse_thing_group(Tokenizer *tz)
{
	thing_group_t	*group = NULL;

	// Defaults
	rgba_t	col;
	int		size = -1;
	bool	hanging = false;
	bool	angle = true;

	// Check if group already exists
	string name = tz->get_token();
	for (int a = 0; a < thing_types.size(); a++)
	{
		if (thing_types[a]->name == name)
		{
			group = thing_types[a];
			break;
		}
	}

	// If it doesn't, create one
	if (!group)
	{
		group = new thing_group_t;
		group->name = name;
		thing_types.push_back(group);
	}
	
	tz->check_token("{");

	// Parse the group
	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "default_colour")
			parse_rgba(tz, &col);
		if (token == "default_size")
			size = tz->get_integer();
		if (token == "angle")
			angle = tz->get_bool();
		if (token == "hanging")
			hanging = tz->get_bool();

		if (token == "thing")
		{
			thing_type_t newtype;
			newtype.colour.set(col);
			newtype.radius = size;
			newtype.hanging = hanging;
			newtype.show_angle = angle;

			newtype.parse(tz);
			newtype.colour.a = 255;
			group->things.push_back(newtype);

			//console_print(parse_string("Added \"%s\" to group \"%s\"", newtype.name.c_str(), group->name.c_str()));
		}

		token = tz->get_token();
	}
}

thing_type_t* get_thing_type(int type)
{
	for (int a = 0; a < thing_types.size(); a++)
	{
		for (int b = 0; b < thing_types[a]->things.size(); b++)
		{
			if (thing_types[a]->things[b].type == type)
				return &thing_types[a]->things[b];
		}
	}

	return &thing_unknown;
}