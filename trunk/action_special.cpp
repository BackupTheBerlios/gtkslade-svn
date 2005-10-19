
#include "main.h"
#include "map.h"
#include "action_special.h"
#include "misc.h"
#include "console.h"
#include "console_window.h"

action_special_t special_unknown;
action_special_t special_none("None");

struct action_group_t
{
	vector<action_special_t>	actions;
	string						name;
};

vector<action_group_t*>	action_specials;

void clear_action_specials()
{
	for (int a = 0; a < action_specials.size(); a++)
	{
		action_specials[a]->actions.clear();
		free(action_specials[a]);
	}
	
	action_specials.clear();
}

action_special_t::action_special_t(string name)
{
	type = 0;
	this->name = name;

	tagtype = 0;
	arg_tag = -1;

	args[0] = "Arg 1";
	args[1] = "Arg 2";
	args[2] = "Arg 3";
	args[3] = "Arg 4";
	args[4] = "Arg 5";
}

void action_special_t::parse(Tokenizer *tz)
{
	type = tz->get_integer();
	name = tz->get_token();

	if (tz->peek_token() == "{")
	{
		tz->check_token("{");
		
		string token = tz->get_token();
		while (token != "}")
		{
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

			if (token == "arg_tag")
				arg_tag = tz->get_integer();
			if (token == "tagtype")
				tagtype = tz->get_integer();

			if (token == "arg1_type")
				arg_types[0] = tz->get_token();
			if (token == "arg2_type")
				arg_types[1] = tz->get_token();
			if (token == "arg3_type")
				arg_types[2] = tz->get_token();
			if (token == "arg4_type")
				arg_types[3] = tz->get_token();
			if (token == "arg5_type")
				arg_types[4] = tz->get_token();

			token = tz->get_token();
		}
	}
}

void parse_action_group(Tokenizer *tz)
{
	action_group_t	*group = NULL;

	// Defaults
	bool tagged = false;
	char arg_tag = 0;
	char tagtype = 0;

	// Check if group already exists
	string name = tz->get_token();
	for (int a = 0; a < action_specials.size(); a++)
	{
		if (action_specials[a]->name == name)
		{
			group = action_specials[a];
			break;
		}
	}

	// If it doesn't, create one
	if (!group)
	{
		group = new action_group_t;
		group->name = name;
		action_specials.push_back(group);
	}

	tz->check_token("{");

	// Parse the group
	string token = tz->get_token();
	while (token != "}")
	{
		if (token == "tagged")
			tagged = tz->get_bool();
		if (token == "arg_tag")
			arg_tag = tz->get_integer();
		if (token == "tagtype")
			tagtype = tz->get_integer();
		if (token == "type")
		{
			action_special_t newspecial;
			newspecial.tagtype = tagtype;

			if (tagged)
				newspecial.arg_tag = arg_tag;
			else
				newspecial.arg_tag = -1;

			newspecial.parse(tz);
			group->actions.push_back(newspecial);

			//console_print(parse_string("Added \"%s\" to group \"%s\"", newspecial.name.c_str(), group->name.c_str()));
		}

		token = tz->get_token();
	}
}

action_special_t* get_special(int type)
{
	if (type == 0)
		return &special_none;

	for (int a = 0; a < action_specials.size(); a++)
	{
		for (int b = 0; b < action_specials[a]->actions.size(); b++)
		{
			if (action_specials[a]->actions[b].type == type)
				return &action_specials[a]->actions[b];
		}
	}

	return &special_unknown;
}

void populate_tree_store_specials(GtkTreeStore *store)
{
	GtkTreeIter iter, child;

	for (int a = 0; a < action_specials.size(); a++)
	{
		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter, 0, action_specials[a]->name.c_str(), 1, -1, -1);

		for (int b = 0; b < action_specials[a]->actions.size(); b++)
		{
			gtk_tree_store_append(store, &child, &iter);
			gtk_tree_store_set(store, &child,	0, action_specials[a]->actions[b].name.c_str(),
												1, action_specials[a]->actions[b].type,
												-1);
		}
	}
}
