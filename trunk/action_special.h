
struct action_special_t
{
	int		type;
	string	name;

	char	tagtype;	// 0=sector, 1=thing
	char	arg_tag;	// The arg to act as a tag (0 for sector tag, -1 for none)

	string	args[5];

	action_special_t(string name = "Unknown");
	void parse(Tokenizer *tz);
};

void parse_action_group(Tokenizer *tz);
action_special_t* get_special(int type);
