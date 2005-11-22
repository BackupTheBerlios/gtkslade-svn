
/*
struct col_config_t
{
	rgba_t			*colours;
	vector<string>	names;
	string			name;

	col_config_t() { colours = NULL; }

	void add(string name, rgba_t col)
	{
		names.push_back(name);
		colours = (rgba_t *)realloc(colours, names.size() * sizeof(rgba_t));
		colours[names.size() - 1].set(col);
	}

	rgba_t* get_colour(string name)
	{
		for (DWORD c = 0; c < names.size(); c++)
		{
			if (names[c] == name)
				return &colours[c];
		}

		return NULL;
	}
};
*/

struct col_config_t
{
	vector<rgba_t>	colours;
	vector<string>	names;
	string			name;
	string			path;

	float			hilight_size;
	float			selection_size;
	float			moving_size;

	col_config_t()
	{
		hilight_size = 3.0f;
		selection_size = moving_size = 5.0f;
	}

	void add(string name, rgba_t col)
	{
		names.push_back(name);
		colours.push_back(col);
	}

	rgba_t* get_colour(string name)
	{
		for (int a = 0; a < names.size(); a++)
		{
			if (names[a] == name)
				return &colours[a];
		}

		return NULL;
	}

	void save();
};

#define SETCOL(name) set_colour(&col_##name, #name, config)

bool load_colour_configs();
void set_colour_config(string name);
void parse_rgba(Tokenizer *mr, rgba_t* col);
