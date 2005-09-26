
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

#define SETCOL(name) set_colour(&col_##name, #name, config)

bool load_colour_configs(string filename);
void set_colour_config(string name);
