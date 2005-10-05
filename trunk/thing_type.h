
struct thing_type_t
{
	int			type;
	string		name;
	rgba_t		colour;
	int			radius;
	string		spritename;
	bool		show_angle;
	bool		hanging;

	string		args[5];
	string		arg_types[5];

	thing_type_t(int type = 0, string name = "Unknown");
	void parse(Tokenizer *tz);
};

void parse_thing_group(Tokenizer *tz);
thing_type_t* get_thing_type(int type);
void clear_thing_types();
