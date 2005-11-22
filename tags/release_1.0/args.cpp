
#include "main.h"
#include "args.h"

vector<argtype_t*> arg_types;

argtype_t::argtype_t()
{
	this->name = "";
	this->has_flags = false;
	arg_types.push_back(this);
}

//argtype_t::argtype_t(string name);

void argtype_t::parse(Tokenizer *tz)
{
	this->name = tz->get_token();

	tz->check_token("{");
	string token = tz->get_token();

	while (token != "}")
	{
		if (token == "value")
		{
			short val = tz->get_integer();
			string nname = tz->get_token();
			add_value(nname, val, false);
		}

		if (token == "flag")
		{
			short val = tz->get_integer();
			string nname = tz->get_token();
			add_value(nname, val, true);
		}

		token = tz->get_token();
	}

	//log_message("Arg type %s parsed\n", this->name.c_str());
}

void clear_arg_types()
{
	for (int a = 0; a < arg_types.size(); a++)
		delete arg_types[a];

	arg_types.clear();
}

argtype_t* get_arg_type(string name)
{
	for (int a = 0; a < arg_types.size(); a++)
	{
		if (arg_types[a]->name == name)
			return arg_types[a];
	}

	return NULL;
}
